#ifndef ERROR_SPECIFICATIONS_BITCODE_INCLUDE_LOCAL_CALLED_FUNCTIONS_H
#define ERROR_SPECIFICATIONS_BITCODE_INCLUDE_LOCAL_CALLED_FUNCTIONS_H

#include <string>
#include <unordered_map>

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "proto/bitcode.pb.h"

namespace error_specifications {

struct LocalCalledFunctionHash {
  size_t operator()(const Function &fn) const {
    return std::hash<std::string>()(fn.source_name());
  }
};

struct LocalCalledFunctionCompare {
  bool operator()(const Function &lhs, const Function &rhs) const {
    return lhs.source_name() == rhs.source_name();
  }
};

// LocalCalledFunctionsPass LLVM pass returns a LocalCalledFunctionsResponse
// with all of the functions that are called in each caller.
struct LocalCalledFunctionsPass : public llvm::ModulePass {
  static char ID;
  LocalCalledFunctionsPass() : ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &mod) override;

  // Encodes the list of locally called functions as a
  // LocalCalledFunctionsResponse. The mapping goes from
  // callee -> caller -> call count.
  LocalCalledFunctionsResponse GetLocalCalledFunctions();

  void getAnalysisUsage(llvm::AnalysisUsage &au) const override;

 private:
  // Map from callee function to caller function to callee call count.
  std::unordered_map<Function,
                     std::unordered_map<Function, int, LocalCalledFunctionHash,
                                        LocalCalledFunctionCompare>,
                     LocalCalledFunctionHash, LocalCalledFunctionCompare>
      local_called_functions_;

  void RunOnFunction(llvm::Function &fn);
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_BITCODE_INCLUDE_LOCAL_CALLED_FUNCTIONS_H
