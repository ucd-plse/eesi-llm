#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "called_functions_helper.h"

#include "bitcode/src/called_functions_pass.h"

namespace error_specifications {

CalledFunctionsResponse runCalledFunctions(std::string bitcode_path) {
  CalledFunctionsPass *called_functions_pass = new CalledFunctionsPass();
  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  if (!module) {
    err.print("called-functions-test", llvm::errs());
  }

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(called_functions_pass);
  pass_manager.run(*module);

  return called_functions_pass->GetCalledFunctions();
}

// A simple hello world program.
// This tests that external library functions show up in the list of
// called functions.
TEST(CalledFunctionsTest, HelloFunctions) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/hello.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "printf", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      res.called_functions()));
}

// A simple hello world program.
// This tests that external library functions show up in the list of
// called functions.
// Uses the reg2mem pass optimization.
TEST(CalledFunctionsTest, HelloFunctionsReg2mem) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/hello-reg2mem.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "printf", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      res.called_functions()));
}

// A program with two functions, where one calls another.
TEST(CalledFunctionsTest, FooCallsBar) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/foo_calls_bar.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "bar", FunctionReturnType::FUNCTION_RETURN_TYPE_VOID, 1,
      res.called_functions()));
}

// A program with two functions, where one calls another.
// Uses the reg2mem pass optimization.
TEST(CalledFunctionsTest, FooCallsBarReg2mem) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/foo_calls_bar-reg2mem.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "bar", FunctionReturnType::FUNCTION_RETURN_TYPE_VOID, 1,
      res.called_functions()));
}

// Tests that multiple call sites are captured.
TEST(CalledFunctionsTest, MultipleCallSites) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/hello_twice.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "printf", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 2,
      res.called_functions()));
}

// Tests that multiple call sites are captured.
// Uses the reg2mem pass optimization.
TEST(CalledFunctionsTest, MultipleCallSitesReg2mem) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/hello_twice-reg2mem.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "printf", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 2,
      res.called_functions()));
}

// Tests that function calls to pointer-returning functions set type correctly.
TEST(CalledFunctionsTest, PointerReturn) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/calls_ptr.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo", FunctionReturnType::FUNCTION_RETURN_TYPE_POINTER, 1,
      res.called_functions()));
}

// Tests that function calls to pointer-returning functions set type correctly.
// Uses the reg2mem pass optimization.
TEST(CalledFunctionsTest, PointerReturnReg2mem) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/calls_ptr-reg2mem.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "foo", FunctionReturnType::FUNCTION_RETURN_TYPE_POINTER, 1,
      res.called_functions()));
}

// Tests that LLVM intrinsic functions are removed from the output.
// This bitcode file contains a call to llvm.dbg.declare.
TEST(CalledFunctionsTest, RemoveIntrinsics) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/propagation_inside_if.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "mustcheck", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      res.called_functions()));
}

// Tests that LLVM intrinsic functions are removed from the output.
// This bitcode file contains a call to llvm.dbg.declare.
// Uses the reg2mem pass optimization.
TEST(CalledFunctionsTest, RemoveIntrinsicsReg2mem) {
  CalledFunctionsResponse res =
      runCalledFunctions("testdata/programs/propagation_inside_if-reg2mem.ll");
  ASSERT_EQ(res.called_functions_size(), 1);
  EXPECT_TRUE(calledFunctionInCalledFunctions(
      "mustcheck", FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER, 1,
      res.called_functions()));
}

}  // namespace error_specifications
