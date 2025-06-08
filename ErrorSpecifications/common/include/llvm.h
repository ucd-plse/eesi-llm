// This file contains utility functions that are useful to Bitcode LLVM passes.

#ifndef ERROR_SPECIFICATIONS_COMMON_INCLUDE_COMMON_H_
#define ERROR_SPECIFICATIONS_COMMON_INCLUDE_COMMON_H_
#include <iostream>

#include "llvm/IR/Instructions.h"

#include "proto/bitcode.pb.h"

namespace error_specifications {

// Returns the function (as protobuf message), if any, that is the direct call
// target of a call instruction. This may return an empty message if the
// callee cannot be resolved or is an intrinsic.
Function GetCallee(const llvm::CallInst &inst);

// Returns the llvm::Function associated with call instruction
// or nullptr if the function could not be resolved or is an intrinsic.
const llvm::Function *GetCalleeFunction(const llvm::CallInst &inst);

// Returns source name for the function associated with call instruction
// or empty string if the function could not be resolved or is an intrinsic.
std::string GetCalleeSourceName(const llvm::CallInst &inst);

// Returns source name for the function associated with the
// llvm Function.
std::string GetSourceName(const llvm::Function &fn);

// Abstracts the return type of an LLVM function to the FunctionReturnType enum.
FunctionReturnType GetFunctionReturnType(const llvm::Function *function);

FunctionReturnType GetReturnType(const llvm::Function &function);

// Returns true if the return type of the function is void.
bool IsVoidFunction(const llvm::Function &function);

// Converts an LLVM Value to a Function protobuf message.
// Used by `getCallee`.
Function LlvmToProtoFunction(const llvm::Function &function);

std::string LlvmToSourceName(const std::string &function_name);

// Returns the debug location associated with an LLVM instruction.
// May return an empty location.
Location GetDebugLocation(const llvm::Instruction &inst);

// Returns the string file name of the source file that the LLVM instruction
// is associated with. If there is DebugInfo, this will just return an empty
// string.
std::string GetSourceFileName(const llvm::Instruction &inst);

// Overloads operator for printing Location.
std::ostream &operator<<(std::ostream &out, const Location &location);

// Overloads operator for printing CallInst.
std::ostream &operator<<(std::ostream &out, const llvm::CallInst &call_inst);

// Overloads operator for printing Instruction.
std::ostream &operator<<(std::ostream &out, const llvm::Instruction &inst);

}  // namespace error_specifications.

#endif  // ERROR_SPECIFICATIONS_COMMON_INCLUDE_COMMON_H_
