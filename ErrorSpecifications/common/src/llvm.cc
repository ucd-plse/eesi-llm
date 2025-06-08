#include "llvm.h"

#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"

#include "proto/bitcode.pb.h"
#include "servers.h"

namespace error_specifications {

Function GetCallee(const llvm::CallInst &inst) {
  const llvm::Function *callee_fn = GetCalleeFunction(inst);
  if (!callee_fn) {
    return Function();
  }
  return LlvmToProtoFunction(*callee_fn);
}

const llvm::Function *GetCalleeFunction(const llvm::CallInst &inst) {
  const llvm::Value *callee = inst.getCalledValue();
  // Unable to resolve the call target.
  if (!callee) {
    return nullptr;
  }
  const llvm::Function *callee_fn =
      llvm::dyn_cast<llvm::Function>(callee->stripPointerCasts());

  // Able to resolve the call target, but not able to resolve the function.
  if (!callee_fn || callee_fn->isIntrinsic()) {
    return nullptr;
  }
  return callee_fn;
}

std::string GetCalleeSourceName(const llvm::CallInst &inst) {
  auto fn = GetCalleeFunction(inst);
  if (!fn) return std::string();
  return GetSourceName(*fn);
}

std::string GetSourceName(const llvm::Function &fn) {
  return LlvmToSourceName(fn.getName());
}

std::string LlvmToSourceName(const std::string &function_name) {
  return StripSuffixAfterDot(function_name);
}

Function LlvmToProtoFunction(const llvm::Function &function) {
  Function ret;
  ret.set_llvm_name(function.getName().str());
  ret.set_source_name(LlvmToSourceName(function.getName()));
  ret.set_return_type(GetReturnType(function));
  return ret;
}

FunctionReturnType GetReturnType(const llvm::Function &function) {
  llvm::Type *return_type = function.getReturnType();
  if (return_type->isVoidTy()) {
    return FunctionReturnType::FUNCTION_RETURN_TYPE_VOID;
  } else if (return_type->isIntegerTy()) {
    return FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER;
  } else if (return_type->isPointerTy()) {
    return FunctionReturnType::FUNCTION_RETURN_TYPE_POINTER;
  } else {
    return FunctionReturnType::FUNCTION_RETURN_TYPE_OTHER;
  }
}

bool IsVoidFunction(const llvm::Function &function) {
  return GetReturnType(function) ==
         FunctionReturnType::FUNCTION_RETURN_TYPE_VOID;
}

Location GetDebugLocation(const llvm::Instruction &inst) {
  if (llvm::DILocation *loc = inst.getDebugLoc()) {
    Location location;
    location.set_file(loc->getFilename());
    location.set_line(loc->getLine());
    return location;
  } else {
    return Location();
  }
}

std::string GetSourceFileName(const llvm::Instruction &inst) {
  return GetDebugLocation(inst).file();
}

std::ostream &operator<<(std::ostream &out, const Location &location) {
  out << location.file() << ":" << location.line();
  return out;
}

std::ostream &operator<<(std::ostream &out, const llvm::CallInst &call_inst) {
  out << GetCalleeSourceName(call_inst)
      << " callsite=" << GetDebugLocation(call_inst);
  return out;
}

std::ostream &operator<<(std::ostream &out, const llvm::Instruction &inst) {
  out << " S=" << GetDebugLocation(inst);
  return out;
}

}  // namespace error_specifications
