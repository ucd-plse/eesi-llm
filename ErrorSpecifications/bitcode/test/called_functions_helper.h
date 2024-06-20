#ifndef ERROR_SPECIFICATIONS_BITCODE_TEST_CALLED_FUNCTIONS_HELPER_H_
#define ERROR_SPECIFICATIONS_BITCODE_TEST_CALLED_FUNCTIONS_HELPER_H_

#include "bitcode/src/called_functions_pass.h"

namespace error_specifications {

// Checks if the expected fields of a called function appear in a list of
// called functions.
bool calledFunctionInCalledFunctions(
    const std::string &function_name, const FunctionReturnType &return_type,
    int call_sites,
    const google::protobuf::RepeatedPtrField<CalledFunction> &called_functions);

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_BITCODE_TEST_CALLED_FUNCTIONS_HELPER_H_
