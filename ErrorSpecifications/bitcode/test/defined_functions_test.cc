#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "bitcode/src/defined_functions_pass.h"

namespace error_specifications {

DefinedFunctionsResponse RunDefinedFunctions(std::string bitcode_path) {
  DefinedFunctionsPass *defined_functions_pass = new DefinedFunctionsPass();
  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> mod(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  if (!mod) {
    err.print("defined-functions-test", llvm::errs());
  }

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(defined_functions_pass);
  pass_manager.run(*mod);

  return defined_functions_pass->get_defined_functions();
}

// A simple hello world program.
// This tests that external library functions are not included in the list of
// defined functions.
TEST(DefinedFunctionsTest, HelloFunctions) {
  DefinedFunctionsResponse res =
      RunDefinedFunctions("testdata/programs/hello.ll");

  ASSERT_EQ(res.functions_size(), 1);
  EXPECT_EQ(res.functions(0).llvm_name(), "main");
  EXPECT_EQ(res.functions(0).source_name(), "main");
}

// A simple hello world program.
// This tests that external library functions are not included in the list of
// defined functions.
// Uses the reg2mem pass optimization.
TEST(DefinedFunctionsTest, HelloFunctionsReg2mem) {
  DefinedFunctionsResponse res =
      RunDefinedFunctions("testdata/programs/hello-reg2mem.ll");

  ASSERT_EQ(res.functions_size(), 1);
  EXPECT_EQ(res.functions(0).llvm_name(), "main");
  EXPECT_EQ(res.functions(0).source_name(), "main");
}

// A program with two functions.
TEST(DefinedFunctionsTest, TwoFunctions) {
  DefinedFunctionsResponse res =
      RunDefinedFunctions("testdata/programs/foo_calls_bar.ll");

  ASSERT_EQ(res.functions_size(), 2);
  EXPECT_EQ(res.functions(0).llvm_name(), "foo");
  EXPECT_EQ(res.functions(0).source_name(), "foo");
  EXPECT_EQ(res.functions(1).llvm_name(), "bar");
  EXPECT_EQ(res.functions(1).source_name(), "bar");
}

// A program with two functions.
TEST(DefinedFunctionsTest, TwoFunctionsReg2mem) {
  DefinedFunctionsResponse res =
      RunDefinedFunctions("testdata/programs/foo_calls_bar-reg2mem.ll");

  ASSERT_EQ(res.functions_size(), 2);
  EXPECT_EQ(res.functions(0).llvm_name(), "foo");
  EXPECT_EQ(res.functions(0).source_name(), "foo");
  EXPECT_EQ(res.functions(1).llvm_name(), "bar");
  EXPECT_EQ(res.functions(1).source_name(), "bar");
}

}  // namespace error_specifications.
