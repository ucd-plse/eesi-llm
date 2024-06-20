#include "annotate_pass.h"

#include <string>

#include "llvm/IR/InstIterator.h"

namespace error_specifications {

bool AnnotatePass::runOnModule(llvm::Module &mod) {
  // For each function in module that is not an LLVM intrinsic or
  // a declaration without a definition.
  for (llvm::Function &fn : mod) {
    if (fn.isIntrinsic() || fn.isDeclaration()) {
      continue;
    }
    RunOnFunction(fn);
  }

  // This pass always modifies bitcode.
  return true;
}

void AnnotatePass::RunOnFunction(llvm::Function &fn) {
  llvm::LLVMContext &context = fn.getContext();

  // Add instruction identifier as metadata on each instruction.
  for (auto inst_it = llvm::inst_begin(fn), fn_end = llvm::inst_end(fn);
       inst_it != fn_end; ++inst_it) {
    llvm::MDNode *md_node = llvm::MDNode::get(
        context, llvm::MDString::get(context, std::to_string(next_iid_)));
    (*inst_it).setMetadata(kInstructionIdentifier, md_node);
    ++next_iid_;
  }
}

void AnnotatePass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  // While this pass modifies bitcode, it does not invalidate the results
  // of any other LLVM passes.
  au.setPreservesAll();
}

// LLVM uses ID’s address to identify a pass,
// so initialization value is not important.
char AnnotatePass::ID = 0;
static llvm::RegisterPass<AnnotatePass> X(
    "annotate",
    "Annotate a bitcode file by adding unique instruction identifiers", false,
    false);

}  // namespace error_specifications.
