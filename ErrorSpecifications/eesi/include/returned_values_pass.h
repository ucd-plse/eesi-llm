// This analysis determines at each program point the set of
// LLVM values that can be returned.

// Inputs
// --------
// - Bitcode module

// Outputs
// ----------
// input_facts: a map from LLVM instruction values to the dataflow fact that
//              holds at the program point before the instruction.
// output_facts: a map from LLVM instruction values to the dataflow fact that
//               holds at the program point after the instruction.

// Implementation
// ---------------
// This is a backward analysis that tracks what values can reach the
// return operand at each program point.
// - ReturnInst: return instructions are the only instructions that generate a
//               fact, where an empty set can become non-empty. At the program
//               point before the return instruction the return operand can be
//               returned.
// - LoadInst: if the result of a load can be returned, then so to can the
//             operand of the load
// - StoreInst: if the receiver of a store instruction can be returned after
//              the store instruction executes, then the sender of the store
//              can be returned before the instruction executes. The receiver
//              is removed from the input fact
// - BitCastInst, PtrToIntInst: these are handled like loads
// - PHINode: if the out value of a phi node can be returned, then
//            the incoming value for the phi node can be returned
//            at the output of the corresponding basic block. For
//            example, if %5 = phi [%1, BB1] [%2, BB2] can be returned,
//            then %1 can be returned at the exit of BB1 and %2 can
//            be returned at the exit of BB2.
//
// Sets are unioned at join points (forward branches).
//
// The Linux function ERR_PTR is modeled in visitCallInst

// Limitations
// ------------
// - Intraprocedural.
// - Field-insensitive, ignoring assignments to fields.

#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURNED_VALUES_PASS_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURNED_VALUES_PASS_H_

#include <unordered_map>
#include <unordered_set>

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "tbb/tbb.h"

#include "constraint.h"

namespace error_specifications {

class ReturnedValuesFact {
 public:
  std::unordered_set<const llvm::Value *> value;

  ReturnedValuesFact() {}

  ReturnedValuesFact(const ReturnedValuesFact &other) { value = other.value; }

  bool operator==(const ReturnedValuesFact &other) {
    return value == other.value;
  }
  bool operator!=(const ReturnedValuesFact &other) {
    return value != other.value;
  }

  void Join(const ReturnedValuesFact &other) {
    value.insert(other.value.begin(), other.value.end());
  }

  void Meet(const ReturnedValuesFact &other) {
    std::unordered_set<const llvm::Value *> intersect;
    set_intersection(value.begin(), value.end(), other.value.begin(),
                     other.value.end(),
                     std::inserter(intersect, intersect.begin()));
    value = intersect;
  }

  bool Contains(const llvm::Value *v) const {
    return value.find(v) != value.end();
  }
};

class ReturnedValuesPass : public llvm::ModulePass {
 public:
  static char ID;

  ReturnedValuesPass() : llvm::ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &M) override;

  // Called for each function.
  void RunOnFunction(const llvm::Function &F);

  ReturnedValuesFact GetInFact(const llvm::Value *) const;
  ReturnedValuesFact GetOutFact(const llvm::Value *) const;

 private:
  // Called for each basic block.
  bool visitBlock(const llvm::BasicBlock &BB);

  // Transfer functions.
  void VisitReturnInst(const llvm::ReturnInst &I,
                       std::shared_ptr<ReturnedValuesFact> input,
                       std::shared_ptr<const ReturnedValuesFact> out);
  void VisitCallInst(const llvm::CallInst &I,
                     std::shared_ptr<ReturnedValuesFact> input,
                     std::shared_ptr<const ReturnedValuesFact> out);
  void VisitLoadInst(const llvm::LoadInst &I,
                     std::shared_ptr<ReturnedValuesFact> input,
                     std::shared_ptr<const ReturnedValuesFact> out);
  void VisitStoreInst(const llvm::StoreInst &I,
                      std::shared_ptr<ReturnedValuesFact> input,
                      std::shared_ptr<const ReturnedValuesFact> out);
  void VisitBitCastInst(const llvm::BitCastInst &I,
                        std::shared_ptr<ReturnedValuesFact> input,
                        std::shared_ptr<const ReturnedValuesFact> out);
  void VisitPtrToIntInst(const llvm::PtrToIntInst &I,
                         std::shared_ptr<ReturnedValuesFact> input,
                         std::shared_ptr<const ReturnedValuesFact> out);
  void VisitTruncInst(const llvm::TruncInst &I,
                      std::shared_ptr<ReturnedValuesFact> input,
                      std::shared_ptr<const ReturnedValuesFact> out);
  void VisitSExtInst(const llvm::SExtInst &I,
                     std::shared_ptr<ReturnedValuesFact> input,
                     std::shared_ptr<const ReturnedValuesFact> out);
  void VisitPHINode(const llvm::PHINode &I,
                    std::shared_ptr<ReturnedValuesFact> input,
                    std::shared_ptr<const ReturnedValuesFact> out);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

  // Helper function for adding values to return_propagated map.
  void AddReturnPropagated(const llvm::Function *, const std::string &);

  // A map from values (instructions) to dataflow facts.
  tbb::concurrent_unordered_map<const llvm::Value *,
                                std::shared_ptr<ReturnedValuesFact>>
      input_facts_;

  // A map from values (instructions) to dataflow facts.
  tbb::concurrent_unordered_map<const llvm::Value *,
                                std::shared_ptr<ReturnedValuesFact>>
      output_facts_;

  // A map from functions to propagated functions.
  tbb::concurrent_unordered_map<const llvm::Function *,
                                std::unordered_set<std::string>>
      return_propagated_;
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURNED_VALUES_PASS_H_
