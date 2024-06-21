#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_EESI_COMMON_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_EESI_COMMON_H_

#include <string>

#include "constraint.h"
#include "llvm/IR/Instructions.h"

namespace error_specifications {

// Returns the first instruction of a basic block.
const llvm::Instruction *GetFirstInstructionOfBB(
    const llvm::BasicBlock *basic_block);

// Returns the last instruction of a basic block.
const llvm::Instruction *GetLastInstructionOfBB(
    const llvm::BasicBlock *basic_block);

// Abstract an integer into the corresponding lattice element.
SignLatticeElement AbstractInteger(const llvm::ConstantInt &integer);

// Extracts the boolean value, if any, from an llvm::Value
llvm::Optional<bool> ExtractBoolean(const llvm::Value &value);

// Extracts the integer value, if any, from an llvm::Value
llvm::Optional<std::int64_t> ExtractInteger(const llvm::Value &value);

// Extracts the string literal, if any, from a value
llvm::Optional<llvm::StringRef> ExtractStringLiteral(const llvm::Value &value);

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_EESI_COMMON_H_
