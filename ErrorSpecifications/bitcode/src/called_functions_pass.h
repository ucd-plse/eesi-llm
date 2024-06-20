#ifndef ERROR_SPECIFICATIONS_BITCODE_CALLEDFUNCTIONS_H
#define ERROR_SPECIFICATIONS_BITCODE_CALLEDFUNCTIONS_H

#include <string>
#include <unordered_map>

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "proto/bitcode.pb.h"

namespace error_specifications {

struct CalledFunctionHash {
  size_t operator()(const Function &fn) const {
    return std::hash<std::string>()(fn.llvm_name());
  }
};

struct CalledFunctionCompare {
  bool operator()(const Function &lhs, const Function &rhs) const {
    return lhs.llvm_name() == rhs.llvm_name();
  }
};

// CalledFunctionsPass LLVM pass returns a CalledFunctionsResponse with all of
// the functions that are directly called from the bitcode module.
struct CalledFunctionsPass : public llvm::ModulePass {
  static char ID;
  CalledFunctionsPass() : ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &mod) override;

  // Encodes the list of defined functions as a DefinedFunctionsResponse
  // protobuf.
  CalledFunctionsResponse GetCalledFunctions();

  void getAnalysisUsage(llvm::AnalysisUsage &au) const override;

 private:
  // Map from Function to number of call sites.
  std::unordered_map<Function, int, CalledFunctionHash, CalledFunctionCompare>
      called_functions_;

  void RunOnFunction(llvm::Function &fn);
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_BITCODE_CALLEDFUNCTIONS_H
