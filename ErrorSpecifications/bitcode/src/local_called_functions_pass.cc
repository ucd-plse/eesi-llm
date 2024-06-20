#include "local_called_functions_pass.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#include "llvm.h"
#include "proto/bitcode.pb.h"

namespace error_specifications {

bool LocalCalledFunctionsPass::runOnModule(llvm::Module &mod) {
  for (llvm::Function &fn : mod) {
    if (fn.isIntrinsic() || fn.isDeclaration()) continue;
    RunOnFunction(fn);
  }

  return false;
}

void LocalCalledFunctionsPass::RunOnFunction(llvm::Function &fn) {
  auto caller_fn = LlvmToProtoFunction(fn);
  for (auto inst_it = llvm::inst_begin(fn), fn_end = llvm::inst_end(fn);
       inst_it != fn_end; ++inst_it) {
    if (llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(&*inst_it)) {
      Function callee_fn = GetCallee(*call);
      if (callee_fn.llvm_name().empty()) continue;

      // Increment count in the callee -> caller -> count mapping.
      auto caller_functions_it = local_called_functions_.find(callee_fn);
      if (caller_functions_it == local_called_functions_.end()) {
        local_called_functions_[callee_fn] =
            std::unordered_map<Function, int, LocalCalledFunctionHash,
                               LocalCalledFunctionCompare>({{caller_fn, 1}});
      } else {
        // Check if callee -> caller already exists.
        auto caller_functions_it =
            local_called_functions_[callee_fn].find(caller_fn);

        if (caller_functions_it == local_called_functions_[callee_fn].end()) {
          local_called_functions_[callee_fn][caller_fn] = 1;
        } else {
          caller_functions_it->second += 1;
        }
      }
    }
  }
}

LocalCalledFunctionsResponse
LocalCalledFunctionsPass::GetLocalCalledFunctions() {
  LocalCalledFunctionsResponse response;
  for (auto &local_called_function : local_called_functions_) {
    LocalCalledFunction *local_called_function_message =
        response.add_local_called_functions();
    local_called_function_message->mutable_called_function()->CopyFrom(
        local_called_function.first);
    for (auto &caller_call_counts : local_called_function.second) {
      CallerFunction *caller_function =
          local_called_function_message->add_caller_functions();
      caller_function->mutable_function()->CopyFrom(caller_call_counts.first);
      caller_function->set_total_call_sites(caller_call_counts.second);
    }
  }

  return response;
}

void LocalCalledFunctionsPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.setPreservesAll();
}

// LLVM uses these IDs to identify a pass, so the actual initialized value does
// not matter to us.
char LocalCalledFunctionsPass::ID = 0;
static llvm::RegisterPass<LocalCalledFunctionsPass> X(
    "localcalledfunctions",
    "List of locally called functions in each function body.", false, false);

}  // namespace error_specifications.
