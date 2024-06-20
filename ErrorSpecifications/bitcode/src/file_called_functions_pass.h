#ifndef ERROR_SPECIFICATIONS_BITCODE_INCLUDE_FILE_CALLED_FUNCTIONS_H
#define ERROR_SPECIFICATIONS_BITCODE_INCLUDE_FILE_CALLED_FUNCTIONS_H

#include <string>
#include <unordered_map>

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

#include "proto/bitcode.pb.h"

namespace error_specifications {

struct FileCalledFunctionHash {
  size_t operator()(const Function &fn) const {
    return std::hash<std::string>()(fn.source_name());
  }
};

struct FileCalledFunctionCompare {
  bool operator()(const Function &lhs, const Function &rhs) const {
    return lhs.source_name() == rhs.source_name();
  }
};

// FileCalledFunctionsPass LLVM pass returns a FileCalledFunctionsResponse with
// all functions called per file.
struct FileCalledFunctionsPass : public llvm::ModulePass {
  static char ID;
  FileCalledFunctionsPass() : ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &mod) override;

  // Encodes the list of called functions per file included in the bitcode file.
  FileCalledFunctionsResponse GetFileCalledFunctions();

  void getAnalysisUsage(llvm::AnalysisUsage &au) const override;

 private:
  // Map from Function to number of call sites.
  std::unordered_map<std::string,
                     std::unordered_map<Function, int, FileCalledFunctionHash,
                                        FileCalledFunctionCompare>>
      file_called_functions_;

  void RunOnFunction(llvm::Function &fn);
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_BITCODE_INCLUDE_FILE_CALLED_FUNCTIONS_H
