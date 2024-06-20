#include "defined_functions_pass.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

#include "llvm.h"
#include "proto/bitcode.pb.h"

namespace error_specifications {

bool DefinedFunctionsPass::runOnModule(llvm::Module &mod) {
  // For each function in module that is not an LLVM intrinsic or
  // a declaration without a definition.
  for (llvm::Function &fn : mod) {
    if (fn.isIntrinsic() || fn.isDeclaration()) {
      continue;
    }
    RunOnFunction(fn);
  }

  // This pass never modifies bitcode.
  return false;
}

void DefinedFunctionsPass::RunOnFunction(llvm::Function &fn) {
  Function this_fn = LlvmToProtoFunction(fn);
  Function *new_fn = defined_functions_.add_functions();
  new_fn->CopyFrom(this_fn);
}

DefinedFunctionsResponse DefinedFunctionsPass::get_defined_functions() {
  return defined_functions_;
}

// LLVM uses ID’s address to identify a pass,
// so initialization value is not important.
char DefinedFunctionsPass::ID = 0;
static llvm::RegisterPass<DefinedFunctionsPass> X(
    "definedfunctions", "List functions defined in a bitcode file", false,
    false);

}  // namespace error_specifications.
