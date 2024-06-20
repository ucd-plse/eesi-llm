#include "file_called_functions_pass.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"

#include "llvm.h"
#include "proto/bitcode.pb.h"

namespace error_specifications {

bool FileCalledFunctionsPass::runOnModule(llvm::Module &mod) {
  for (llvm::Function &fn : mod) {
    if (fn.isIntrinsic() || fn.isDeclaration()) continue;
    RunOnFunction(fn);
  }

  return false;
}

void FileCalledFunctionsPass::RunOnFunction(llvm::Function &fn) {
  auto caller_fn = LlvmToProtoFunction(fn);
  for (auto inst_it = llvm::inst_begin(fn), fn_end = llvm::inst_end(fn);
       inst_it != fn_end; ++inst_it) {
    if (llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(&*inst_it)) {
      Function callee_fn = GetCallee(*call);
      if (callee_fn.llvm_name().empty()) continue;
      Location location = GetDebugLocation(*call);
      std::string file_name = location.file();

      // Increment count in the file -> callee -> count mapping.
      auto caller_functions_it = file_called_functions_.find(file_name);
      if (caller_functions_it == file_called_functions_.end()) {
        file_called_functions_[file_name] =
            std::unordered_map<Function, int, FileCalledFunctionHash,
                               FileCalledFunctionCompare>({{callee_fn, 1}});
      } else {
        // All of the current called (callee) function counts in the mapping.
        auto callee_functions_it =
            file_called_functions_[file_name].find(callee_fn);

        if (callee_functions_it == file_called_functions_[file_name].end()) {
          file_called_functions_[file_name][callee_fn] = 1;
        } else {
          callee_functions_it->second += 1;
        }
      }
    }
  }
}

FileCalledFunctionsResponse FileCalledFunctionsPass::GetFileCalledFunctions() {
  FileCalledFunctionsResponse response;
  for (auto &file_called_functions : file_called_functions_) {
    FileCalledFunction *file_called_function =
        response.add_file_called_functions();
    file_called_function->set_file(file_called_functions.first);
    for (auto &called_function_sites : file_called_functions.second) {
      CalledFunction *called_function =
          file_called_function->add_called_functions();
      called_function->mutable_function()->CopyFrom(
          called_function_sites.first);
      called_function->set_total_call_sites(called_function_sites.second);
    }
  }

  return response;
}

void FileCalledFunctionsPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.setPreservesAll();
}

// LLVM uses these IDs to identify a pass, so the actual initialized value does
// not matter to us.
char FileCalledFunctionsPass::ID = 0;
static llvm::RegisterPass<FileCalledFunctionsPass> X(
    "filecalledfunctions", "List of called functions in each file.", false,
    false);

}  // namespace error_specifications.
