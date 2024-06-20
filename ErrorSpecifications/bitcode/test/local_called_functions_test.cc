#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "bitcode/src/local_called_functions_pass.h"

namespace error_specifications {

LocalCalledFunctionsResponse runLocalCalledFunctions(std::string bitcode_path) {
  LocalCalledFunctionsPass *local_called_functions_pass =
      new LocalCalledFunctionsPass();
  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  // The module must exist to continue.
  assert(module);

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(local_called_functions_pass);
  pass_manager.run(*module);

  return local_called_functions_pass->GetLocalCalledFunctions();
}

// A simple hello world program that calls printf twice.
TEST(LocalCalledFunctionsTest, HelloTwiceFunctions) {
  LocalCalledFunctionsResponse res =
      runLocalCalledFunctions("testdata/programs/hello_twice.ll");

  const auto called_printf = res.local_called_functions(0).called_function();
  const auto caller_main = res.local_called_functions(0).caller_functions(0);

  ASSERT_EQ(res.local_called_functions_size(), 1);
  EXPECT_EQ(called_printf.llvm_name(), "printf");
  EXPECT_EQ(called_printf.source_name(), "printf");
  EXPECT_EQ(called_printf.return_type(),
            FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER);
  EXPECT_EQ(caller_main.function().llvm_name(), "main");
  EXPECT_EQ(caller_main.function().source_name(), "main");
  EXPECT_EQ(caller_main.total_call_sites(), 2);
  EXPECT_EQ(caller_main.function().return_type(),
            FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER);
}

// A simple hello world program that calls printf twice. This bitcode file uses
// a reg2mem pass.
TEST(LocalCalledFunctionsTest, HelloTwiceFunctionsReg2mem) {
  LocalCalledFunctionsResponse res =
      runLocalCalledFunctions("testdata/programs/hello_twice-reg2mem.ll");
  const auto called_printf = res.local_called_functions(0).called_function();
  const auto caller_main = res.local_called_functions(0).caller_functions(0);

  ASSERT_EQ(res.local_called_functions_size(), 1);
  EXPECT_EQ(called_printf.llvm_name(), "printf");
  EXPECT_EQ(called_printf.source_name(), "printf");
  EXPECT_EQ(called_printf.return_type(),
            FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER);
  EXPECT_EQ(caller_main.function().llvm_name(), "main");
  EXPECT_EQ(caller_main.function().source_name(), "main");
  EXPECT_EQ(caller_main.total_call_sites(), 2);
  EXPECT_EQ(caller_main.function().return_type(),
            FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER);
}

}  // namespace error_specifications
