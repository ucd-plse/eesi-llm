
#include "bitcode/src/called_functions_pass.h"

namespace error_specifications {

bool calledFunctionInCalledFunctions(
    const std::string &function_name, const FunctionReturnType &return_type,
    int call_sites,
    const google::protobuf::RepeatedPtrField<CalledFunction>
        &called_functions) {
  for (const auto called_function : called_functions) {
    if (called_function.function().llvm_name() == function_name &&
        called_function.function().source_name() == function_name &&
        called_function.function().return_type() == return_type &&
        called_function.total_call_sites() == call_sites) {
      return true;
    }
  }
  return false;
}

}  // namespace error_specifications
