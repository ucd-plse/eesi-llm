#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_PROPAGATION_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_PROPAGATION_H_

#include <unordered_map>
#include <unordered_set>

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "tbb/tbb.h"

namespace error_specifications {

// A dataflow fact is a map from LLVM values to the functions they hold return
// values for.
class ReturnPropagationFact {
 public:
  std::unordered_map<const llvm::Value *,
                     std::unordered_set<const llvm::Value *>>
      value;

  ReturnPropagationFact() {}

  ReturnPropagationFact(const ReturnPropagationFact &other) {
    value = other.value;
  }

  bool operator==(const ReturnPropagationFact &other) {
    return value == other.value;
  }

  bool operator!=(const ReturnPropagationFact &other) {
    return value != other.value;
  }

  void Join(const ReturnPropagationFact &other) {
    // Union each set in map
    for (auto kv : other.value) {
      value[kv.first].insert(kv.second.begin(), kv.second.end());
    }
  }
};

struct ReturnPropagationPass : public llvm::ModulePass {
  static char ID;

  ReturnPropagationPass() : llvm::ModulePass(ID) {}

  // Dataflow facts at the program point immediately following instruction.
  tbb::concurrent_unordered_map<const llvm::Value *,
                                std::shared_ptr<ReturnPropagationFact>>
      input_facts_;
  tbb::concurrent_unordered_map<const llvm::Value *,
                                std::shared_ptr<ReturnPropagationFact>>
      output_facts_;

  bool finished = false;

  bool runOnModule(llvm::Module &M) override;
  bool RunOnFunction(const llvm::Function &F);
  bool VisitBlock(const llvm::BasicBlock &BB);

  void VisitCallInst(const llvm::CallInst &I,
                     std::shared_ptr<const ReturnPropagationFact> input,
                     std::shared_ptr<ReturnPropagationFact> out);
  void VisitLoadInst(const llvm::LoadInst &I,
                     std::shared_ptr<const ReturnPropagationFact> input,
                     std::shared_ptr<ReturnPropagationFact> out);
  void VisitStoreInst(const llvm::StoreInst &I,
                      std::shared_ptr<const ReturnPropagationFact> input,
                      std::shared_ptr<ReturnPropagationFact> out);
  void VisitBitCastInst(const llvm::BitCastInst &I,
                        std::shared_ptr<const ReturnPropagationFact> input,
                        std::shared_ptr<ReturnPropagationFact> out);
  void VisitPtrToIntInst(const llvm::PtrToIntInst &I,
                         std::shared_ptr<const ReturnPropagationFact> input,
                         std::shared_ptr<ReturnPropagationFact> out);
  void VisitBinaryOperator(const llvm::BinaryOperator &I,
                           std::shared_ptr<const ReturnPropagationFact> input,
                           std::shared_ptr<ReturnPropagationFact> out);
  void VisitPHINode(const llvm::PHINode &I,
                    std::shared_ptr<const ReturnPropagationFact> input,
                    std::shared_ptr<ReturnPropagationFact> out);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_RETURN_PROPAGATION_H_
