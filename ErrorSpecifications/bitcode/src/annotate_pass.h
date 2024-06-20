#ifndef ERROR_SPECIFICATIONS_BITCODE_ANNOTATE_PASS_H
#define ERROR_SPECIFICATIONS_BITCODE_ANNOTATE_PASS_H

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace error_specifications {

constexpr char kInstructionIdentifier[] = "iid";

// This LLVM pass adds an instruction ID to every instruction in an
// LLVM bitcode file. The ID is stored as metadata associated with the
// instruction.
class AnnotatePass : public llvm::ModulePass {
 public:
  static char ID;

  AnnotatePass() : llvm::ModulePass(ID) {}

  // Entry point.
  virtual bool runOnModule(llvm::Module &mod) override;
  virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const override;

 private:
  void RunOnFunction(llvm::Function &fn);

  // Instruction identifier cursor.
  uint64_t next_iid_ = 0;
};

}  // namespace error_specifications.

#endif
