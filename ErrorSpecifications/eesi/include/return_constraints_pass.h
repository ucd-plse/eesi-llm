#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_CONSTRAINTS_PASS_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_CONSTRAINTS_PASS_H_

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "constraint.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "tbb/tbb.h"

namespace error_specifications {

class ReturnConstraintsFact {
 public:
  std::unordered_map<std::string, Constraint> value;

  ReturnConstraintsFact() {}

  ReturnConstraintsFact(const ReturnConstraintsFact &other) {
    value = other.value;
  }

  bool operator==(const ReturnConstraintsFact &other) {
    return value == other.value;
  }
  bool operator!=(const ReturnConstraintsFact &other) {
    return value != other.value;
  }

  // To save space, facts are initialized as empty maps instead of
  // creating an entry for every function. Therefore, when computing the
  // Join or Meet of two facts, where a function exists as a key in the value
  // of one fact but not the other, the result to copy the value from the fact
  // where the function exists.

  void Join(const ReturnConstraintsFact &other) {
    // For each function key, join the constraints.
    for (const auto &kv : other.value) {
      const std::string &function_name = kv.first;
      auto value_it = value.find(function_name);
      if (value_it != value.end()) {
        value_it->second = value_it->second.Join(kv.second);
      } else {
        value[function_name] = kv.second;
      }
    }
  }

  void Meet(const ReturnConstraintsFact &other) {
    // For each function key, meet the constraints.
    for (const auto &kv : other.value) {
      const std::string &function_name = kv.first;
      auto value_it = value.find(function_name);
      if (value_it != value.end()) {
        value_it->second = value_it->second.Meet(kv.second);
      } else {
        value[function_name] = kv.second;
      }
    }
  }

  void Dump() {
    for (auto kv : value) {
      std::cerr << "(" << kv.first << ":" << kv.second << ")" << std::endl;
    }
  }
};

class ReturnConstraintsPass : public llvm::ModulePass {
 public:
  static char ID;

  ReturnConstraintsPass() : llvm::ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &M) override;

  // Called for each function.
  void RunOnFunction(const llvm::Function &F);

  ReturnConstraintsFact GetInFact(const llvm::Value *) const;
  ReturnConstraintsFact GetOutFact(const llvm::Value *) const;

  static std::pair<SignLatticeElement, SignLatticeElement> AbstractICmp(
      const llvm::ICmpInst &I);

  // Return all constraints on the execution of any block in `parent_function`,
  // with respect to the value of `called_function`. Does not differentiate call
  // sites.
  std::set<SignLatticeElement> GetConstraints(
      llvm::Module &module, const std::string &parent_function,
      const Function &called_function);

 private:
  // Called for each basic block.
  bool VisitBlock(const llvm::BasicBlock &BB);

  // Transfer functions.
  void VisitCallInst(const llvm::CallInst &I,
                     std::shared_ptr<const ReturnConstraintsFact> input,
                     std::shared_ptr<ReturnConstraintsFact> out);
  void VisitBranchInst(const llvm::BranchInst &I,
                       std::shared_ptr<const ReturnConstraintsFact> input,
                       std::shared_ptr<ReturnConstraintsFact> out);
  void VisitSwitchInst(const llvm::SwitchInst &I,
                       std::shared_ptr<const ReturnConstraintsFact> input,
                       std::shared_ptr<ReturnConstraintsFact> out);
  void VisitPHINode(const llvm::PHINode &I,
                    std::shared_ptr<const ReturnConstraintsFact> input,
                    std::shared_ptr<ReturnConstraintsFact> out);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  // A map from values (instructions) to dataflow facts
  tbb::concurrent_unordered_map<const llvm::Value *,
                                std::shared_ptr<ReturnConstraintsFact>>
      input_facts_;

  // A map from values (instructions) to dataflow facts
  tbb::concurrent_unordered_map<const llvm::Value *,
                                std::shared_ptr<ReturnConstraintsFact>>
      output_facts_;

  static const std::map<
      std::pair<llvm::ICmpInst::Predicate, SignLatticeElement>,
      std::pair<SignLatticeElement, SignLatticeElement>>
      predicate_complement;

  static const std::map<SignLatticeElement, SignLatticeElement>
      unsigned_replacement;
};

}  //  namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_CONSTRAINTS_PASS_H_
