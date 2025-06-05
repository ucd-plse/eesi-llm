#include "constraint.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "llvm.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "return_range_pass.h"

namespace error_specifications {

// Run ReturnRangePass on a bitcode file, and return a map of function names to
// calculated return ranges.
std::unordered_map<std::string, SignLatticeElement> RunGetReturnRanges(
    const std::string &bitcode_path) {
  ReturnRangePass *return_range_pass = new ReturnRangePass();

  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> mod(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  if (!mod) {
    err.print("return-range-test", llvm::errs());
    std::abort();
  }

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(return_range_pass);
  pass_manager.run(*mod);

  std::unordered_map<std::string, SignLatticeElement> return_ranges;
  for (const auto &kv : return_range_pass->GetReturnRanges()) {
    return_ranges[GetSourceName(*kv.first)] = kv.second;
  }
  return return_ranges;
}

// Test calculating the return range of functions that return integer constants.
TEST(ReturnRangeTest, IntReturn) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/int_functions.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return integer
// constants.  A reg2mem pass was applied to the bitcode file.
TEST(ReturnRangeTest, IntReturnReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/int_functions-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return string constants.
TEST(ReturnRangeTest, StringReturn) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/string_functions.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return string constants.
// A reg2mem pass was applied to the bitcode file.
TEST(ReturnRangeTest, StringReturnReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/string_functions-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return basic boolean
// expressions.
TEST(ReturnRangeTest, BoolReturn) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/bool_functions.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"quux", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return basic boolean
// expressions.  A reg2mem pass was applied to the bitcode file.
TEST(ReturnRangeTest, BoolReturnReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/bool_functions-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"quux", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return the results of
// calling other functions.
TEST(ReturnRangeTest, PropagatedReturn) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/mbedtls_x509_csr_parse.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"mbedtls_x509_csr_parse_der",
       SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
      {"mbedtls_pem_read_buffer",
       SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
      {"mbedtls_x509_csr_parse",
       SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return the results of
// calling other functions.  A reg2mem pass was applied to the bitcode file.
TEST(ReturnRangeTest, PropagatedReturnReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/mbedtls_x509_csr_parse-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"mbedtls_x509_csr_parse_der",
       SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
      {"mbedtls_pem_read_buffer",
       SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
      {"mbedtls_x509_csr_parse",
       SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return an unknown value.
// In this case, the function simply returns an argument that was passed to it.
TEST(ReturnRangeTest, UnknownReturn) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/return_argument.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions that return an unknown value.
// In this case, the function simply returns an argument that was passed to it.
// A reg2mem pass was applied to the bitcode file.
TEST(ReturnRangeTest, UnknownReturnReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/return_argument-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions when there is a constraint
// applied to the returned values.
TEST(ReturnRangeTest, ConstrainedReturn) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/constrained_return.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
      {"quux", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions when there is a constraint
// applied to the returned values.  A reg2mem pass was applied to the bitcode
// file.
TEST(ReturnRangeTest, ConstrainedReturnReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/constrained_return-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
      {"quux", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions when a switch statement
// constrains the returned values.
TEST(ReturnRangeTest, Switch) {
  const auto return_ranges = RunGetReturnRanges("testdata/programs/switch.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"quux", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating the return range of functions when a switch statement
// constrains the returned values.  A reg2mem pass was applied to the bitcode
// file.
TEST(ReturnRangeTest, SwitchReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/switch-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
      {"quux", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating a function return range when a callee's return value is
// checked for a specific range (e.g., 0 <= x < 100).
TEST(ReturnRangeTest, RangeCheck) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/range_check.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating a function return range when a callee's return value is
// checked for a specific range (e.g., 0 <= x < 100).  This bitcode file uses a
// reg2mem pass.
TEST(ReturnRangeTest, RangeCheckReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/range_check-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating return ranges when the functions are in the same SCC.
TEST(ReturnRangeTest, SCCFunctions) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/scc_functions.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"main", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating return ranges when the functions are in the same SCC.  This
// bitcode file uses a reg2mem pass.
TEST(ReturnRangeTest, SCCFunctionsReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/scc_functions-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
      {"main", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating return ranges when the functions are in the same SCC.  This
// test requires at least two passes over the SCC to calculate the correct
// ranges.
TEST(ReturnRangeTest, SCCTwoPasses) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/scc_two_passes.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

// Test calculating return ranges when the functions are in the same SCC.  This
// test requires at least two passes over the SCC to calculate the correct
// ranges.  The bitcode file uses a reg2mem pass.
TEST(ReturnRangeTest, SCCTwoPassesReg2mem) {
  const auto return_ranges =
      RunGetReturnRanges("testdata/programs/scc_two_passes-reg2mem.ll");

  std::unordered_map<std::string, SignLatticeElement> expected_return_ranges{
      {"foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
      {"bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
  };

  ASSERT_EQ(return_ranges, expected_return_ranges);
}

}  // namespace error_specifications
