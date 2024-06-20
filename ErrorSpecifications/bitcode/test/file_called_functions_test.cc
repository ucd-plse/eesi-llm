#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "bitcode/src/file_called_functions_pass.h"
#include "called_functions_helper.h"

namespace error_specifications {

FileCalledFunctionsResponse runFileCalledFunctions(std::string bitcode_path) {
  FileCalledFunctionsPass *file_called_functions_pass =
      new FileCalledFunctionsPass();
  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  // The module must exist to continue.
  assert(module);

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(file_called_functions_pass);
  pass_manager.run(*module);

  return file_called_functions_pass->GetFileCalledFunctions();
}

// Tests the mapping of the file to multiple called functions.
TEST(FileCalledFunctionsTest, FileCalledFunctionsMultiReturn) {
  FileCalledFunctionsResponse res =
      runFileCalledFunctions("testdata/programs/multireturn.ll");

  ASSERT_EQ(res.file_called_functions_size(), 1);
  const auto file_called_function = res.file_called_functions(0);

  EXPECT_EQ(file_called_function.file(), "multireturn.c");

  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo1", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      file_called_function.called_functions()));
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo2", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      file_called_function.called_functions()));
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo3", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      file_called_function.called_functions()));
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "EO", FunctionReturnType::FUNCTION_RETURN_TYPE_VOID, 3,
      file_called_function.called_functions()));
}

// Tests the mapping of the file to multiple called functions. This bitcode
// file uses a Reg2mem pass.
TEST(FileCalledFunctionsTest, FileCalledFunctionsMultiReturnReg2mem) {
  FileCalledFunctionsResponse res =
      runFileCalledFunctions("testdata/programs/multireturn-reg2mem.ll");

  ASSERT_EQ(res.file_called_functions_size(), 1);
  const auto file_called_function = res.file_called_functions(0);

  EXPECT_EQ(file_called_function.file(), "multireturn.c");

  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo1", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      file_called_function.called_functions()));
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo2", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      file_called_function.called_functions()));
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo3", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      file_called_function.called_functions()));
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "EO", FunctionReturnType::FUNCTION_RETURN_TYPE_VOID, 3,
      file_called_function.called_functions()));
}

}  // namespace error_specifications
