#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_RANGE_PASS_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_RANGE_PASS_H_

#include <unordered_map>

#include "llvm.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "proto/eesi.grpc.pb.h"
#include "returned_values_pass.h"

namespace error_specifications {

// Fact representing the possible ranges that returned values can take on at a
// particular point of the program.
class ReturnRangeFact {
 public:
  // Map of returned value -> possible range of that value.
  std::unordered_map<const llvm::Value *, SignLatticeElement> value;

  // Default constructor.
  ReturnRangeFact() = default;

  // Copy constructor.
  ReturnRangeFact(const ReturnRangeFact &other);

  // Convenience constructor to use with Join and Meet when adding single values
  // to a fact.
  ReturnRangeFact(const llvm::Value *v, SignLatticeElement constraint);

  bool operator==(const ReturnRangeFact &other) const;
  bool operator!=(const ReturnRangeFact &other) const;

  // Join this fact with another one.  The other fact's entries are copied to
  // this fact, and if there are duplicate entries, their ranges are
  // combined with SignLattice::Join.
  void Join(const ReturnRangeFact &other);

  // Meet this fact with another one.  The other fact's entries are copied to
  // this fact, and if there are duplicate entries, their ranges are
  // combined with SignLattice::Meet.
  void Meet(const ReturnRangeFact &other);

  // Join this fact with another fact, ignoring any entries that are not present
  // in the ReturnedValuesFact.  The filtering is done because we only care
  // about ranges of returnable values.
  void FilteredJoin(const ReturnRangeFact &other,
                    const ReturnedValuesFact &rvf);

  // Copy another fact, ignoring any entries not present in the
  // ReturnedValuesFact.
  void FilteredCopy(const ReturnRangeFact &other,
                    const ReturnedValuesFact &rvf);

  // Check whether this fact has an entry for a value.
  bool Contains(const llvm::Value *v) const;
};

// This LLVM pass is responsible for calculating the possible ranges of returned
// values at each point in the program.  It also calculates each function's
// return range.
class ReturnRangePass : public llvm::ModulePass {
 public:
  static char ID;

  ReturnRangePass() : llvm::ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &M) override;

  // Called for each function.
  void RunOnFunction(const llvm::Function &func);

  // Get the return range of a function.
  SignLatticeElement GetReturnRange(const llvm::Function &func) const;

  // Get the return range of a function, returning a provided default if not
  // found.
  SignLatticeElement GetReturnRange(
      const llvm::Function &func,
      const SignLatticeElement default_return) const;

  // Get the return ranges of all functions.
  const std::unordered_map<const llvm::Function *, SignLatticeElement>
      &GetReturnRanges() const;

  ReturnRangeFact GetInFact(const llvm::Instruction *inst) const;
  ReturnRangeFact GetOutFact(const llvm::Instruction *inst) const;

 private:
  // Map from llvm functions to their return ranges.
  std::unordered_map<const llvm::Function *, SignLatticeElement> return_ranges_;

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  // Called for each basic block.
  bool VisitBlock(const llvm::BasicBlock &BB);

  // Transfer functions
  void VisitStoreInst(const llvm::StoreInst &I, const ReturnRangeFact &in,
                      ReturnRangeFact &out, const ReturnedValuesFact &out_rvf);
  void VisitLoadLikeInst(const llvm::Instruction &I, const ReturnRangeFact &in,
                         ReturnRangeFact &out,
                         const ReturnedValuesFact &out_rvf);
  void VisitPHINode(const llvm::PHINode &I, const ReturnRangeFact &in,
                    ReturnRangeFact &out, const ReturnedValuesFact &out_rvf);
  void VisitBranchInst(const llvm::BranchInst &I, const ReturnRangeFact &in,
                       ReturnRangeFact &out, const ReturnedValuesFact &out_rvf);
  void VisitSwitchInst(const llvm::SwitchInst &I, const ReturnRangeFact &in,
                       ReturnRangeFact &out, const ReturnedValuesFact &out_rvf);
  void VisitReturnInst(const llvm::ReturnInst &I, const ReturnRangeFact &in);

  // If a returned value is being checked (e.g., in an icmp or switch
  // instruction), resolve the value and return it.  Otherwise, return null.
  const llvm::Value *GetCheckedReturnValue(const llvm::Instruction &cmp_inst,
                                           const llvm::Value *cmp_val,
                                           const ReturnedValuesFact &rvf) const;

  // Abstract a call instruction into the corresponding SignLatticeElement.
  SignLatticeElement AbstractCall(const llvm::CallInst &call) const;

  // Abstract a value into the corresponding SignLatticeElement.
  SignLatticeElement AbstractValue(const llvm::Value &value) const;

  // Checks whether this pass should ignore calculating the return range of a
  // function.
  //
  // A function should be ignored if it meets one of the following conditions:
  // 1. It is intrinsic.
  // 2. It is external (i.e., a declaration with no body).
  // 3. It does not return an integer or a pointer.
  bool ShouldIgnore(const llvm::Function *func) const;

  // A map from instructions to dataflow facts.
  std::unordered_map<const llvm::Instruction *,
                     std::shared_ptr<ReturnRangeFact>>
      input_facts_;

  // A map from instructions to dataflow facts.
  std::unordered_map<const llvm::Instruction *,
                     std::shared_ptr<ReturnRangeFact>>
      output_facts_;
};

}  //  namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_RANGE_PASS_H_
