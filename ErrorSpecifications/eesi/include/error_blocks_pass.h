#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_ERROR_BLOCKS_PASS_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_ERROR_BLOCKS_PASS_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "call_graph_underapproximation.h"
#include "checker.h"
#include "confidence_lattice.h"
#include "constraint.h"
#include "gpt_model.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

// This LLVM pass is responsible for implementing the error specification
// inference rules.
struct ErrorBlocksPass : public llvm::ModulePass {
  static char ID;
  ErrorBlocksPass() : ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &M) override;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  void SetSpecificationsRequest(const GetSpecificationsRequest &request);

  // Get the final set of inferred function error specifications.
  GetSpecificationsResponse GetSpecifications() const;

  // Returns the set of non-doomed functions. These non-doomed functions include
  // functions that are represented in our embedding vocabulary, are reachable
  // in the call graph to these functions that are represented in the embedding,
  // or are reachable in the call graph from the supplied domain knowledge
  std::unordered_set<std::string> GetNonDoomedFunctions() const;

 private:
  using ErrorSpecificationMap =
      std::unordered_map<std::string, LatticeElementConfidence>;
  using ErrorOnlyFuncToArgMap =
      std::unordered_multimap<std::string,
                              std::unordered_map<int, ConstantValue>>;
  using ReturnTypeMap = std::unordered_map<std::string, FunctionReturnType>;

  // Performs static analysis to infer the error specification of the
  // function. Returns true if the error specification for the function has been
  // changed.
  bool RunOnFunction(llvm::Function *fn);

  void AddInferenceSources(std::string function_name,
                           std::vector<std::string> context_functions,
                           LatticeElementConfidence inferred_element);
  void AddInferenceSource(std::string function_name,
                          std::string context_function,
                          LatticeElementConfidence inferred_element);
  void AddInferenceSourceLessThanZero(std::string function_name,
                                      std::string context_function);
  void AddInferenceSourceGreaterThanZero(std::string function_name,
                                         std::string context_function);
  void AddInferenceSourceZero(std::string function_name,
                              std::string context_function);
  void AddInferenceSourceEmptyset(std::string function_name,
                                  std::string context_function);

  // Expands the error specification of the function using the error
  // specifications of the converged functions.
  // Returns true if the resulting error specification of the function
  // is not bottom.
  bool ExpandErrorSpecification(
      llvm::Function *func,
      const std::unordered_map<std::string, FunctionReturnType>
          &converged_functions);
  bool LlmExpandErrorSpecification(llvm::Function *func);
  bool LlmExpandThirdPartyErrorSpecifications(
      std::vector<std::pair<std::string, std::string>> function_names);
  bool GptExpandErrorSpecification(llvm::Function *func,
                                   std::vector<Specification> specifications);
  bool runOnThirdPartyFunctions(const llvm::CallGraph &call_graph);

  // Returns true if any new error values were added.
  // Called for each basic block.
  LatticeElementConfidence VisitBlock(const llvm::BasicBlock &BB);

  std::set<SignLatticeElement> CollectConstraints(
      const llvm::Function &parent_function, const std::string &fn_name);

  // Returns true if any new error values were added.
  // Called for each call instruction.
  LatticeElementConfidence VisitCallInst(const llvm::CallInst &I);

  // Helper function for adding values to error_return_values_ map.
  LatticeElementConfidence AddErrorValue(const llvm::Function *, int64_t);

  // Adds a function name to the non-doomed functions set. Returns true if we
  // update the set.
  bool AddNonDoomedFunction(const llvm::Function &f);
  bool AddNonDoomedFunction(const std::string &function_name);

  bool IsDoomedFunction(const llvm::Function &f);
  bool IsDoomedFunction(const std::string &function_name);

  // Returns the abstraction of a concrete integer.
  SignLatticeElement AbstractInteger(int64_t concrete) const;

  // Returns the current error specification associated with
  // the function called by call_inst.
  LatticeElementConfidence GetErrorSpecification(
      const llvm::CallInst &call_inst) const;

  // Returns the current error specification associated with
  // the given function.
  // Requires the given function to be non-null.
  LatticeElementConfidence GetErrorSpecification(
      const llvm::Function *node) const;
  LatticeElementConfidence GetErrorSpecification(
      const std::string &source_name) const;

  // Updates the error specification for func by joining with delta, also
  // sets the confidence for the specification.
  // Returns true if the error specification was updated.
  bool UpdateErrorSpecification(const llvm::Function *func,
                                LatticeElementConfidence delta);
  bool UpdateErrorSpecification(std::string func_name,
                                LatticeElementConfidence delta);

  // Remove a sign lattice element from an error specification.
  // Returns true if the error specification was updated.
  bool RemoveFromErrorSpecification(const std::string &function_name,
                                    LatticeElementConfidence to_remove);

  // Checks whether an llvm::Value and a ConstantValue contain the same value.
  bool ValuesAreEqual(const llvm::Value &llvm_value,
                      const ConstantValue &constant_value) const;

  // Returns true if the given call_inst contains a call to an error-only
  // function.
  bool IsErrorOnlyFunctionCall(const llvm::CallInst &call_inst) const;

  // Returns true if the given Function is an error-only function.
  bool IsErrorOnlyFunction(const llvm::Function *func) const;

  // Returns true if the given value is an error code.
  bool IsErrorCode(int64_t value, const std::string &filename) const;

  // Returns true if the given value is a success code.
  bool IsSuccessCode(int64_t value, const std::string &filename) const;

  // Returns true if the given value is a success code for the given function.
  // This IsSuccessCode variant considers the smart-success-code-zero heuristic.
  bool IsSuccessCode(const std::string &function_name, const int64_t value,
                     const std::string &filename) const;

  // Returns true if the smart-success-code-zero heuristic is enabled and if the
  // heuristic determines that the given function has 0 as a success code.
  bool ShouldSmartDropZero(const std::string &function_name,
                           const std::string &filename) const;

  // Adds a function to the set of functions that return domain knowledge codes.
  // This should be called whenever a function returns a domain knowledge
  // success/error code, or when a function propagates the return of a callee
  // that returns domain knowledge codes.
  void AddFunctionReturningDomainKnowledgeCodes(
      const std::string &function_name);

  // Returns true if the given function returns global codes
  // represented by the domain knowledge.
  bool ReturnsDomainKnowledgeCodes(const std::string &function_name) const;

  // Returns true if the ErrorBlocksPass should ignore the given
  // function.
  bool IgnoreFunction(const llvm::Function *function) const;

  // Gathers associated constraints with functions (specifications) and calls
  // the checker's CheckViolations, which looks for any violations associated
  // the CallInst.
  void CheckViolations(const llvm::CallInst &call_inst);

  // Iterates through all instructions for a function and checks for any
  // violations associated with CallInsts.
  void CheckViolations(const llvm::Function &func);

  // The checker is used for finding bugs that violate the error specifications
  // that EESI has inferred.
  Checker *checker_;

  // For getting llvm constructs related to functions from names.
  llvm::Module *module_;

  // The language model to be used for expansion.
  // LlamaModel *language_model_;
  GptModel *language_model_;

  // The path to the ctags file for the benchmark analyzed.
  std::string ctags_file_;

  // The minimum number of functions to use as evidence when
  // inferring new specifications using the embedding.
  // Should be greater than zero; 5 is a reasonable value here.
  int minimum_evidence_;

  // Minimum similarity to use when inferring new specifications
  // using the embedding.
  // Should be between 0 and 1; 0.7 is a reasonable value here.
  float minimum_similarity_;

  // A map from functions to the integer-like constants that
  // may be returned on error. These are the concrete error values.
  // Note: the keys for this map will not be identical to the
  // keys in abstract_error_return_values because input error
  // specifications are added to abstract_error_return_values
  // but not error_return_values.
  std::unordered_map<const llvm::Function *, std::unordered_set<int64_t>>
      error_return_values_;

  // A map from function _source_ names to its error specification.
  ErrorSpecificationMap error_specifications_;

  // Domain knowledge: Multimap of function names to sets of arguments required
  // to consider a call error-only.  If a function has multiple entries, then
  // only one entry's required arguments need to match to consider a call
  // error-only.  If an entry has no required arguments, then all calls to the
  // corresponding function will be considered error-only.  Ideally, this would
  // be LLVM name of error-only functions, but that depends on the compilation
  // process, and therefore this is necessarily the source name.
  ErrorOnlyFuncToArgMap error_only_functions_;

  // Domain knowledge: error codes mapped to submodules. If the set of
  // submodules is empty, then the error codes are considered for the entire
  // project.
  std::unordered_map<int64_t, std::unordered_set<std::string>> error_codes_;
  std::unordered_map<std::string, SignLatticeElement> error_code_names_;

  // Domain knowledge: the corresponding success codes of the domain
  // knowledge error codes.
  std::unordered_map<int64_t, std::unordered_set<std::string>> success_codes_;
  std::unordered_map<std::string, SignLatticeElement> success_code_names_;

  std::unordered_map<std::string, llvm::Function *> name_to_function_;

  // Whether to apply a heuristic to determine if 0 is a success code in certain
  // contexts, instead of every time.
  bool smart_success_code_zero_;

  // The set of functions that return domain knowledge codes.
  std::unordered_set<std::string> functions_returning_domain_knowledge_codes_;

  // Map of function source names that correspond to initial error
  // specifications. These should never change.
  ErrorSpecificationMap initial_error_specifications_;

  std::unordered_map<std::string, std::vector<Specification>>
      llm_specifications_;

  // Map of function source names that correspond to their return type.
  ReturnTypeMap function_return_types_;

  // The set of functions that domain knowledge reaches. In other words, these
  // functions are connected to domain knowledge via the call graph in some
  // manner. These functions may not necessarily have associated error
  // specifications. These functions may also not have functions whose
  // specifications would be inferred by EESIER, as these can potentially
  // be external functions that we could not analyze the body for.
  std::unordered_set<std::string> non_doomed_function_names_;

  // Tracking the function name to the functions involved in inferring the
  // particular lattice element;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      sources_of_inference_less_than_zero_;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      sources_of_inference_greater_than_zero_;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      sources_of_inference_zero_;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      sources_of_inference_emptyset_;

  std::unordered_map<std::string, std::unordered_set<std::string>>
      called_functions_;
  std::unordered_set<std::string> inferred_with_llm_;
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_ERROR_BLOCKS_PASS_H_
