#include "constraint.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

const llvm::Instruction *GetFirstInstructionOfBB(
    const llvm::BasicBlock *basic_block) {
  return &(*basic_block->begin());
}

const llvm::Instruction *GetLastInstructionOfBB(
    const llvm::BasicBlock *basic_block) {
  const llvm::Instruction *bb_last;
  for (const llvm::Instruction &inst : *basic_block) {
    bb_last = &inst;
  }
  return bb_last;
}

SignLatticeElement AbstractInteger(const llvm::ConstantInt &integer) {
  if (integer.isNegative()) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  } else if (integer.isZero()) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  } else {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  }
}

llvm::Optional<bool> ExtractBoolean(const llvm::Value &value) {
  if (const auto *value_int = llvm::dyn_cast<llvm::ConstantInt>(&value)) {
    if (value_int->getBitWidth() == 1) {
      return value_int->isOne();
    }
  }
  return llvm::NoneType::None;
}

llvm::Optional<std::int64_t> ExtractInteger(const llvm::Value &value) {
  if (const auto *value_int = llvm::dyn_cast<llvm::ConstantInt>(&value)) {
    if (value_int->getBitWidth() <= 64) {
      return value_int->getSExtValue();
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(&value)) {
    return 0;
  }

  return llvm::NoneType::None;
}

llvm::Optional<llvm::StringRef> ExtractStringLiteral(const llvm::Value &value) {
  // A string literal is represented as a constant global array in LLVM IR.
  // When used as a char*, it is converted to a pointer via getelementptr.
  const auto const_expr = llvm::dyn_cast<llvm::ConstantExpr>(&value);
  if (const_expr &&
      const_expr->getOpcode() == llvm::Instruction::GetElementPtr) {
    const auto global_var =
        llvm::dyn_cast<llvm::GlobalVariable>(const_expr->getOperand(0));
    if (global_var && global_var->isConstant() &&
        global_var->hasInitializer()) {
      const auto initializer =
          llvm::dyn_cast<llvm::ConstantDataArray>(global_var->getInitializer());
      if (initializer && initializer->isCString()) {
        return initializer->getAsCString();
      }
    }
  }
  return llvm::NoneType::None;
}

}  // namespace error_specifications
