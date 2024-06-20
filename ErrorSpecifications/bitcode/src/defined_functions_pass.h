#ifndef ERROR_SPECIFICATIONS_DEFINED_FUNCTIONS_PASS_H
#define ERROR_SPECIFICATIONS_DEFINED_FUNCTIONS_PASS_H

#include <string>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "proto/bitcode.grpc.pb.h"

namespace error_specifications {

// This LLVM pass produces the list of function names defined
// by a bitcode file. The output is a `DefinedFunctionResponse`
// protobuf with one `Function` entry for each defined LLVM function.
class DefinedFunctionsPass : public llvm::ModulePass {
 public:
  static char ID;
  DefinedFunctionsPass() : ModulePass(ID) {}

  // Entry point.
  bool runOnModule(llvm::Module &mod) override;

  // Encode the list of defined functions as a DefinedFunctionsResponse
  // protobuf.
  DefinedFunctionsResponse get_defined_functions();

 private:
  // The functions that are defined in this bitcode file.
  // One entry for each llvm function. At the discretion of the compiler,
  // there are possibly multiple LLVM function definitions for a single source
  // code function definition.
  DefinedFunctionsResponse defined_functions_;

  void RunOnFunction(llvm::Function &fn);
};

}  // namespace error_specifications.

#endif
