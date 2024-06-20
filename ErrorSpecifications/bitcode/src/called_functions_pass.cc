#include "called_functions_pass.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#include <string>

#include "llvm.h"
#include "proto/bitcode.pb.h"

namespace error_specifications {

bool CalledFunctionsPass::runOnModule(llvm::Module &mod) {
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

void CalledFunctionsPass::RunOnFunction(llvm::Function &fn) {
  for (auto inst_it = llvm::inst_begin(fn), fn_end = llvm::inst_end(fn);
       inst_it != fn_end; ++inst_it) {
    if (llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(&*inst_it)) {
      Function callee_fn = GetCallee(*call);
      if (callee_fn.llvm_name().empty()) {
        continue;
      }
      auto called_functions_it = called_functions_.find(callee_fn);
      if (called_functions_it == called_functions_.end()) {
        called_functions_[callee_fn] = 1;
      } else {
        called_functions_it->second += 1;
      }
    }
  }
}

CalledFunctionsResponse CalledFunctionsPass::GetCalledFunctions() {
  CalledFunctionsResponse response;

  for (auto &kv : called_functions_) {
    CalledFunction *called_function = response.add_called_functions();
    called_function->mutable_function()->CopyFrom(kv.first);
    called_function->set_total_call_sites(kv.second);
  }

  return response;
}

void CalledFunctionsPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.setPreservesAll();
}

// LLVM uses ID’s address to identify a pass,
// so initialization value is not important.
char CalledFunctionsPass::ID = 0;
static llvm::RegisterPass<CalledFunctionsPass> X(
    "calledfunctions", "List functions called in a bitcode file", false, false);

}  // namespace error_specifications.
