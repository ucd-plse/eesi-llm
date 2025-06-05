#include <memory>

#include "error_blocks_helper.h"
#include "error_blocks_pass.h"
#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "mock_synonym_finder.h"
#include "proto/eesi.pb.h"
#include "return_constraints_pass.h"
#include "return_propagation_pass.h"
#include "returned_values_pass.h"

namespace error_specifications {

// Tests that using an error code leads to a new function specification.
TEST(ErrorBlocksTest, ErrorCodes) {
  GetSpecificationsRequest req;
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("-EIO");
  error_code->set_value(-5);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_code.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "main", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that using an error code leads to a new function specification. This
// bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorCodesReg2mem) {
  GetSpecificationsRequest req;
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("-EIO");
  error_code->set_value(-5);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_code-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "main", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that module-specific error codes lead to correct specifications and
// that potentially conflicting values across separate modules do not lead to
// an incorrect specification. Unlike other bitcode files used for testing, this
// bitcode file requires llvm-link between the different modules to ensure that
// all relevant definitions are contained in the testing bitcode file.
TEST(ErrorBlocksTest, ErrorCodesModule) {
  GetSpecificationsRequest req;
  ErrorCode *error_code_mod1 = req.add_error_codes();
  error_code_mod1->set_name("MOD1_ERR_CODE");
  error_code_mod1->set_value(-20);
  error_code_mod1->add_submodules("test_module1");
  ErrorCode *error_code_mod2 = req.add_error_codes();
  error_code_mod2->set_name("MOD2_ERR_CODE");
  error_code_mod2->set_value(20);
  error_code_mod2->add_submodules("test_module2");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/test_err_code_mod.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "mod1_foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
  EXPECT_TRUE(FindSpecification(
      "mod2_foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      res));
}

// Tests that module-specific error codes lead to correct specifications and
// that potentially conflicting values across separate modules do not lead to
// an incorrect specification. Unlike other bitcode files used for testing, this
// bitcode file requires llvm-link between the different modules to ensure that
// all relevant definitions are contained in the testing bitcode file. This
// bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorCodesModuleReg2mem) {
  GetSpecificationsRequest req;
  ErrorCode *error_code_mod1 = req.add_error_codes();
  error_code_mod1->set_name("MOD1_ERR_CODE");
  error_code_mod1->set_value(-20);
  error_code_mod1->add_submodules("test_module1");
  ErrorCode *error_code_mod2 = req.add_error_codes();
  error_code_mod2->set_name("MOD2_ERR_CODE");
  error_code_mod2->set_value(20);
  error_code_mod2->add_submodules("test_module2");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/test_err_code_mod-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "mod1_foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
  EXPECT_TRUE(FindSpecification(
      "mod2_foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      res));
}

// Tests that using an error code leads to a new function specification
// using an abstracted version of a function from mbedtls.
TEST(ErrorBlocksTest, ErrorCodeMbedtls) {
  GetSpecificationsRequest req;
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("MBEDTLS_ERR_X509_BAD_INPUT_DATA");
  error_code->set_value(-10240);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/mbedtls_x509_csr_parse.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "mbedtls_x509_csr_parse",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that using an error code leads to a new function specification
// using an abstracted version of a function from mbedtls. This bitcode file
// uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorCodeMbedtlsReg2mem) {
  GetSpecificationsRequest req;
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("MBEDTLS_ERR_X509_BAD_INPUT_DATA");
  error_code->set_value(-10240);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/mbedtls_x509_csr_parse-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "mbedtls_x509_csr_parse",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that using a success code leads to success returns being ignored.
TEST(ErrorBlocksTest, SuccessCodes) {
  GetSpecificationsRequest req;
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("ERROR");
  error_code->set_value(10);
  SuccessCode *success_code = req.add_success_codes();
  success_code->set_name("SUCCESS");
  success_code->set_value(0);
  SuccessCode *other_success_code = req.add_success_codes();
  other_success_code->set_name("OTHER_SUCCESS");
  other_success_code->set_value(-10);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/success_code.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that using a success code leads to success returns being ignored.
// This bitcode file has a reg2mem pass applied.
TEST(ErrorBlocksTest, SuccessCodesReg2mem) {
  GetSpecificationsRequest req;
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("ERROR");
  error_code->set_value(10);
  SuccessCode *success_code = req.add_success_codes();
  success_code->set_name("SUCCESS");
  success_code->set_value(0);
  SuccessCode *other_success_code = req.add_success_codes();
  other_success_code->set_name("OTHER_SUCCESS");
  other_success_code->set_value(-10);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/success_code-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that using a success code leads to success returns being ignored, with
// the smart-success-code-zero heuristic enabled.
TEST(ErrorBlocksTest, SuccessCodesWithHeuristic) {
  GetSpecificationsRequest req;
  req.set_smart_success_code_zero(true);
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("ERROR");
  error_code->set_value(10);
  SuccessCode *success_code = req.add_success_codes();
  success_code->set_name("SUCCESS");
  success_code->set_value(0);
  SuccessCode *other_success_code = req.add_success_codes();
  other_success_code->set_name("OTHER_SUCCESS");
  other_success_code->set_value(-10);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/success_code.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that using a success code leads to success returns being ignored, with
// the smart-success-code-zero heuristic enabled.  This bitcode file has a
// reg2mem pass applied.
TEST(ErrorBlocksTest, SuccessCodesWithHeuristicReg2mem) {
  GetSpecificationsRequest req;
  req.set_smart_success_code_zero(true);
  ErrorCode *error_code = req.add_error_codes();
  error_code->set_name("ERROR");
  error_code->set_value(10);
  SuccessCode *success_code = req.add_success_codes();
  success_code->set_name("SUCCESS");
  success_code->set_value(0);
  SuccessCode *other_success_code = req.add_success_codes();
  other_success_code->set_name("OTHER_SUCCESS");
  other_success_code->set_value(-10);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/success_code-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that boolean error returns result in the correct error lattices.
TEST(ErrorBlocksTest, BoolErrorReturn) {
  GetSpecificationsRequest req;
  Specification *foo_spec = req.add_initial_specifications();
  foo_spec->mutable_function()->set_source_name("foo");
  foo_spec->mutable_function()->set_llvm_name("foo");
  foo_spec->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/bool_err.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that boolean error returns result in the correct error lattices.  This
// bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, BoolErrorReturnReg2mem) {
  GetSpecificationsRequest req;
  Specification *foo_spec = req.add_initial_specifications();
  foo_spec->mutable_function()->set_source_name("foo");
  foo_spec->mutable_function()->set_llvm_name("foo");
  foo_spec->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/bool_err-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that the error specifications of two functions that jump to
// the same goto label will be joined.
TEST(ErrorBlocksTest, TwoFunctionGotoSameLabel) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *bar2_specification = req.add_initial_specifications();
  bar2_specification->mutable_function()->set_source_name("bar2");
  bar2_specification->mutable_function()->set_llvm_name("bar2");
  bar2_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/two_function_goto_same_label.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
}

// Tests that the error specifications of two functions that jump to
// the same goto label will be joined. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, TwoFunctionGotoSameLabelReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *bar2_specification = req.add_initial_specifications();
  bar2_specification->mutable_function()->set_source_name("bar2");
  bar2_specification->mutable_function()->set_llvm_name("bar2");
  bar2_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
}

// Tests that propagation joins error specification to parent if return
// statement is executed on error.
TEST(ErrorBlocksTest, Propagation) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("mustcheck");
  mustcheck_specification->mutable_function()->set_llvm_name("mustcheck");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/propagation_inside_if.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that propagation joins error specification to parent if return
// statement is executed on error. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, PropagationReg2mem) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("mustcheck");
  mustcheck_specification->mutable_function()->set_llvm_name("mustcheck");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/propagation_inside_if-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that a return of a constant nullptr on an error-path results in an
// inferred specififcation of ==0.
TEST(ErrorBlocksTest, ErrorConstantNull) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_constant_null.ll", req);

  // This test is disabled. Ideally it would pass, but this is not a case
  // that was handled by the original EESI.
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that a return of a constant nullptr on an error-path results in an
// inferred specififcation of ==0. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorConstantNullReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_constant_null-reg2mem.ll", req);

  // This test is disabled. Ideally it would pass, but this is not a case
  // that was handled by the original EESI.
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests the use of an error-only function in a function that returns an int.
TEST(ErrorBlocksTest, ErrorOnlyCallInt) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_function.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests the use of an error-only function in a function that returns an int.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorOnlyCallIntReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_function-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests the use of an error-only function in a function that returns a pointer.
TEST(ErrorBlocksTest, ErrorOnlyCallPointer) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_function_ptr.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests the use of an error-only function in a function that returns a pointer.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorOnlyCallPointerReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/error_only_function_ptr-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests the use of an error-only function in a function that returns a boolean.
TEST(ErrorBlocksTest, ErrorOnlyCallBool) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_bool.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests the use of an error-only function in a function that returns a boolean.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorOnlyCallBoolReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_bool-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests use of a function that is error-only with a specific integer argument
TEST(ErrorBlocksTest, ErrorOnlyCallWithIntArg) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("my_log");
  error_only_call->mutable_function()->set_source_name("my_log");
  ErrorOnlyArgument *required_arg = error_only_call->add_required_args();
  required_arg->set_position(0);
  required_arg->mutable_value()->set_int_value(1);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_with_int_arg.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Same as above, but with reg2mem pass applied
TEST(ErrorBlocksTest, ErrorOnlyCallWithIntArgReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("my_log");
  error_only_call->mutable_function()->set_source_name("my_log");
  ErrorOnlyArgument *required_arg = error_only_call->add_required_args();
  required_arg->set_position(0);
  required_arg->mutable_value()->set_int_value(1);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/error_only_with_int_arg-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests use of a function that is error-only with a specific string argument
TEST(ErrorBlocksTest, ErrorOnlyCallWithStringArg) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("set_last_message");
  error_only_call->mutable_function()->set_source_name("set_last_message");
  ErrorOnlyArgument *required_arg = error_only_call->add_required_args();
  required_arg->set_position(0);
  required_arg->mutable_value()->set_string_value("Error message");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_with_string_arg.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Same as above, but with reg2mem pass applied
TEST(ErrorBlocksTest, ErrorOnlyCallWithStringArgReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("set_last_message");
  error_only_call->mutable_function()->set_source_name("set_last_message");
  ErrorOnlyArgument *required_arg = error_only_call->add_required_args();
  required_arg->set_position(0);
  required_arg->mutable_value()->set_string_value("Error message");

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/error_only_with_string_arg-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests use of a function that is error-only with a null argument
TEST(ErrorBlocksTest, ErrorOnlyCallWithNullArg) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("send_reply");
  error_only_call->mutable_function()->set_source_name("send_reply");
  ErrorOnlyArgument *required_arg = error_only_call->add_required_args();
  required_arg->set_position(0);
  required_arg->mutable_value()->set_int_value(0);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_with_null_arg.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Same as above, but with reg2mem pass applied
TEST(ErrorBlocksTest, ErrorOnlyCallWithNullArgReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("send_reply");
  error_only_call->mutable_function()->set_source_name("send_reply");
  ErrorOnlyArgument *required_arg = error_only_call->add_required_args();
  required_arg->set_position(0);
  required_arg->mutable_value()->set_int_value(0);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/error_only_with_null_arg-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests having more than one error-only definition with the same name.
TEST(ErrorBlocksTest, MultipleErrorOnlyWithSameName) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_log_call = req.add_error_only_functions();
  error_log_call->mutable_function()->set_llvm_name("my_log");
  error_log_call->mutable_function()->set_source_name("my_log");
  ErrorOnlyArgument *error_log_arg = error_log_call->add_required_args();
  error_log_arg->set_position(0);
  error_log_arg->mutable_value()->set_int_value(1);
  ErrorOnlyCall *critical_log_call = req.add_error_only_functions();
  critical_log_call->mutable_function()->set_llvm_name("my_log");
  critical_log_call->mutable_function()->set_source_name("my_log");
  ErrorOnlyArgument *critical_log_arg = critical_log_call->add_required_args();
  critical_log_arg->set_position(0);
  critical_log_arg->mutable_value()->set_int_value(2);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_with_same_name.ll", req);

  ASSERT_EQ(res.specifications().size(), 1) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
}

// Tests having more than one error-only definition with the same name.  This
// bitcode file has a reg2mem pass.
TEST(ErrorBlocksTest, MultipleErrorOnlyWithSameNameReg2mem) {
  GetSpecificationsRequest req;
  ErrorOnlyCall *error_log_call = req.add_error_only_functions();
  error_log_call->mutable_function()->set_llvm_name("my_log");
  error_log_call->mutable_function()->set_source_name("my_log");
  ErrorOnlyArgument *error_log_arg = error_log_call->add_required_args();
  error_log_arg->set_position(0);
  error_log_arg->mutable_value()->set_int_value(1);
  ErrorOnlyCall *critical_log_call = req.add_error_only_functions();
  critical_log_call->mutable_function()->set_llvm_name("my_log");
  critical_log_call->mutable_function()->set_source_name("my_log");
  ErrorOnlyArgument *critical_log_arg = critical_log_call->add_required_args();
  critical_log_arg->set_position(0);
  critical_log_arg->mutable_value()->set_int_value(2);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/error_only_with_same_name-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
}

// Tests returning a constant zero along an error-path of a pointer-returning
// function results in an inferred specification of ==0.
TEST(ErrorBlocksTest, ErrorConstantPointer) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("malloc");
  bar_specification->mutable_function()->set_llvm_name("malloc");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_function_ptr.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests returning a constant zero along an error-path of a pointer-returning
// function results in an inferred specification of ==0. This bitcode file uses
// a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorConstantPointerReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("malloc");
  bar_specification->mutable_function()->set_llvm_name("malloc");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/error_only_function_ptr-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests directly propagating an error-specification from a called function.
TEST(ErrorBlocksTest, PropagationDirect) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("bar");
  mustcheck_specification->mutable_function()->set_llvm_name("bar");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/propagation_direct.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests directly propagating an error-specification from a called function.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, PropagationDirectReg2mem) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("bar");
  mustcheck_specification->mutable_function()->set_llvm_name("bar");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/propagation_direct-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests inferring a specification along the error-path of fopen.
TEST(ErrorBlocksTest, FopenNullPointer) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("fopen");
  mustcheck_specification->mutable_function()->set_llvm_name("fopen");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/fopen.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "main", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests inferring a specification along the error-path of fopen. This bitcode
// file uses a Reg2mem pass.
TEST(ErrorBlocksTest, FopenNullPointerReg2mem) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("fopen");
  mustcheck_specification->mutable_function()->set_llvm_name("fopen");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/fopen-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "main", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests inferring a specification along an error-path for a return value that
// is unsigned.
TEST(ErrorBlocksTest, Unsigned) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("foo");
  mustcheck_specification->mutable_function()->set_llvm_name("foo");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/unsigned.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "main", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests inferring a specification along an error-path for a return value that
// is unsigned. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, UnsignedReg2mem) {
  GetSpecificationsRequest req;
  Specification *mustcheck_specification = req.add_initial_specifications();
  mustcheck_specification->mutable_function()->set_source_name("foo");
  mustcheck_specification->mutable_function()->set_llvm_name("foo");
  mustcheck_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/unsigned-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "main", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that setting the initial specification does not result in the
// initial specification lattice getting updated.
TEST(ErrorBlocksTest, FreezeInitialSpecs) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_function.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification("foo",
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                res, 0, 0, 0, 100));
}

// Tests that setting the initial specification does not result in the
// initial specification lattice getting updated. This bitcode file uses a
// Reg2mem pass.
TEST(ErrorBlocksTest, FreezeInitialSpecsReg2mem) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ErrorOnlyCall *error_only_call = req.add_error_only_functions();
  error_only_call->mutable_function()->set_llvm_name("error_only");
  error_only_call->mutable_function()->set_source_name("error_only");

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_only_function-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification("foo",
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                res, 0, 0, 0, 100));
}

// Tests for when a function returns a string literal on an error path.
TEST(ErrorBlocksTest, ErrorStringLiteral) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_string_literal.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
}

// Tests for when a function returns a string literal on an error path. This
// bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ErrorStringLiteralReg2mem) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/error_string_literal-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));
}

// Tests getting specifications for programs with instances
// of a function "covering" the return value of another function.
// For example, initially assigning ret = bar() and then
// reassigning ret = baz().
// This will currently not give the correct specification for the function foo
// but will instead not infer anything for the function, since the delta
// for updating the error specification will equal the return range of the
// function.
TEST(ErrorBlocksTest, BazCoverBar) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *baz_specification = req.add_initial_specifications();
  baz_specification->mutable_function()->set_source_name("baz");
  baz_specification->mutable_function()->set_llvm_name("baz");
  baz_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/baz_cover_bar.ll", req);

  // foo will not be inferred as the delta equals the return_range.
  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests getting specifications for programs with instances
// of a function "covering" the return value of another function.
// For example, initially assigning ret = bar() and then
// reassigning ret = baz().
// This will currently not give the correct specification for the function foo
// but will instead not infer anything for the function, since the delta
// for updating the error specification will equal the return range of the
// function.
TEST(ErrorBlocksTest, BazCoverBarReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *baz_specification = req.add_initial_specifications();
  baz_specification->mutable_function()->set_source_name("baz");
  baz_specification->mutable_function()->set_llvm_name("baz");
  baz_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/baz_cover_bar-reg2mem.ll", req);

  // foo will not be inferred as the delta equals the return_range.
  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests functions that are in the same SCC (Strongly Connected
// Component).
TEST(ErrorBlocksTest, SccFunctions) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("qux");
  bar_specification->mutable_function()->set_llvm_name("qux");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/scc_functions.ll", req);

  // Will not infer foo or main due to the delta being equal to the return_range
  // for foo, which main calls directly.
  ASSERT_EQ(res.specifications().size(), 3);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests functions that are in the same SCC (Strongly Connected
// Component). This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, SccFunctionsReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("qux");
  bar_specification->mutable_function()->set_llvm_name("qux");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/scc_functions-reg2mem.ll", req);

  // Will not infer foo or main due to the delta being equal to the return_range
  // for foo, which main calls directly.
  ASSERT_EQ(res.specifications().size(), 3);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests tests getting specifications for return values in nested if-statements.
TEST(ErrorBlocksTest, NestedReturnCheck) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/nested_return_check.ll", req);

  // Since the delta will equal the return_range when updating the error
  // specification for foo, it will not be inferred.
  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests tests getting specifications for return values in nested if-statements.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, NestedReturnCheckReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/nested_return_check-reg2mem.ll", req);

  // Since the delta will equal the return_range when updating the error
  // specification for foo, it will not be inferred.
  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests getting specifications for a recursive function, i.e. a
// function with a self-loop in the call graph.
// TODO: Add test where a recursive function has a non-trivial
// error specification.
TEST(ErrorBlocksTest, RecursiveFunction) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/recursive_function.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests getting specifications for a recursive function, i.e. a
// function with a self-loop in the call graph.
// TODO: Add test where a recursive function has a non-trivial
// error specification. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, RecursiveFunctionReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/recursive_function-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests a malloc wrapper where the allocated pointer is asserted on, so that
// the wrapper aborts instead of returning null.
TEST(ErrorBlocksTest, MallocAssert) {
  GetSpecificationsRequest req;
  Specification *malloc_specification = req.add_initial_specifications();
  malloc_specification->mutable_function()->set_source_name("malloc");
  malloc_specification->mutable_function()->set_llvm_name("malloc");
  malloc_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/malloc_wrapper.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "malloc", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests a malloc wrapper where the allocated pointer is asserted on, so that
// the wrapper aborts instead of returning null.  This bitcode file uses a
// reg2mem pass.
TEST(ErrorBlocksTest, MallocAssertReg2mem) {
  GetSpecificationsRequest req;
  Specification *malloc_specification = req.add_initial_specifications();
  malloc_specification->mutable_function()->set_source_name("malloc");
  malloc_specification->mutable_function()->set_llvm_name("malloc");
  malloc_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/malloc_wrapper-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "malloc", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that range checks are appropriately handled.
TEST(ErrorBlocksTest, RangeCheck) {
  GetSpecificationsRequest req;
  Specification *read_number = req.add_initial_specifications();
  read_number->mutable_function()->set_source_name("read_number");
  read_number->mutable_function()->set_llvm_name("read_number");
  read_number->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/range_error.ll", req);

  ASSERT_EQ(res.specifications().size(), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "read_number", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that range checks are appropriately handled. This bitcode file uses a
// reg2mem pass optimization.
TEST(ErrorBlocksTest, RangeCheckReg2mem) {
  GetSpecificationsRequest req;
  Specification *read_number = req.add_initial_specifications();
  read_number->mutable_function()->set_source_name("read_number");
  read_number->mutable_function()->set_llvm_name("read_number");
  read_number->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/range_error-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "read_number", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that embedding leads to expansion of error specifications.
TEST(ErrorBlocksTest, ExpandUsingEmbedding) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // bar1 is a synonym for bar2.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("bar1", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar2", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res, 0,
      70, 0));
}

// Tests that embedding leads to expansion of error specifications. This bitcode
// file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ExpandUsingEmbeddingReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // bar1 is a synonym for bar2.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("bar1", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label-reg2mem.ll", req, &msf);

  ASSERT_EQ(res.violations_size(), 0);
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar2", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res, 0,
      70, 0));
}

// Tests that the empty-set confidence propagates correctly through the
// embedding-based expansion and static analysis.
TEST(ErrorBlocksTest, EmptysetConfidence) {
  GetSpecificationsRequest req;
  Specification *empty_specification = req.add_initial_specifications();
  empty_specification->mutable_function()->set_source_name("empty");
  empty_specification->mutable_function()->set_llvm_name("empty");
  empty_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  Specification *lt_zero_specification = req.add_initial_specifications();
  lt_zero_specification->mutable_function()->set_source_name("lt_zero");
  lt_zero_specification->mutable_function()->set_llvm_name("lt_zero");
  lt_zero_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  constexpr int kMinimumEvidence = 2;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::string> vocab = {
      "empty",      "lt_zero",      "lt_zero_expand", "unknown",
      "call_empty", "call_lt_zero", "call_unknown",   "base_case_empty"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  // Will attempt to expand on "unknown" and "call_unknown". No synonyms for
  // "unknown" or "call_unknown".
  EXPECT_CALL(
      msf, GetSynonyms("unknown", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("call_unknown", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("call_lt_zero", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("base_case_empty", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // quux synonyms are foo and qux.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("lt_zero", 0.9), std::make_pair("empty", 0.5)};
  EXPECT_CALL(msf, GetSynonyms("lt_zero_expand", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/emptyset_confidence.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(GetEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification("empty",
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                res, 0, 0, 0, 100));
  EXPECT_TRUE(FindSpecification("call_empty",
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                res, 0, 0, 0, 100));
  EXPECT_TRUE(FindSpecification(
      "lt_zero", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "call_lt_zero", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
  EXPECT_TRUE(FindSpecification(
      "lt_zero_expand", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res, 0, 90, 0, 50));
}

// Tests that the empty-set confidence propagates correctly through the
// embedding-based expansion and static analysis. This bitcode file uses a
// Reg2mem pass optimization.
TEST(ErrorBlocksTest, EmptysetConfidenceReg2mem) {
  GetSpecificationsRequest req;
  Specification *empty_specification = req.add_initial_specifications();
  empty_specification->mutable_function()->set_source_name("empty");
  empty_specification->mutable_function()->set_llvm_name("empty");
  empty_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  Specification *lt_zero_specification = req.add_initial_specifications();
  lt_zero_specification->mutable_function()->set_source_name("lt_zero");
  lt_zero_specification->mutable_function()->set_llvm_name("lt_zero");
  lt_zero_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  constexpr int kMinimumEvidence = 2;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::string> vocab = {
      "empty",      "lt_zero",      "lt_zero_expand", "unknown",
      "call_empty", "call_lt_zero", "call_unknown",   "base_case_empty"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  // Will attempt to expand on "unknown" and "call_unknown". No synonyms for
  // "unknown" or "call_unknown".
  EXPECT_CALL(
      msf, GetSynonyms("unknown", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("call_unknown", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("call_lt_zero", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("base_case_empty", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // quux synonyms are foo and qux.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("lt_zero", 0.9), std::make_pair("empty", 0.5)};
  EXPECT_CALL(msf, GetSynonyms("lt_zero_expand", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/emptyset_confidence-reg2mem.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(GetEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification("empty",
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                res, 0, 0, 0, 100));
  EXPECT_TRUE(FindSpecification("call_empty",
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                res, 0, 0, 0, 100));
  EXPECT_TRUE(FindSpecification(
      "lt_zero", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "call_lt_zero", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
  EXPECT_TRUE(FindSpecification(
      "lt_zero_expand", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res, 0, 90, 0, 50));
}

// Tests that error specifications are not expanded past function return ranges.
TEST(ErrorBlocksTest, ExpandWithinReturnRange) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::string> vocab = {"foo", "bar", "baz"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  EXPECT_CALL(msf,
              GetSynonyms("baz", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>{}));

  // Set up bar synonyms
  std::vector<std::pair<std::string, float>> syn_bar = {{"foo", 0.9}};
  EXPECT_CALL(msf,
              GetSynonyms("bar", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(syn_bar));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/return_range.ll", req, &msf);

  // Technically a violation in arb_function().
  ASSERT_EQ(res.violations_size(), 1);
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();

  // bar should not be expanded, since it only returns 0.
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res, 0, 100,
      100));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res, 0,
      0, 90));
}

// Tests that error specifications are not expanded past function return ranges.
// This bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, ExpandWithinReturnRangeReg2mem) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::string> vocab = {"foo", "bar", "baz"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  EXPECT_CALL(msf,
              GetSynonyms("baz", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>{}));

  // Set up bar synonyms
  std::vector<std::pair<std::string, float>> syn_bar = {{"foo", 0.9}};
  EXPECT_CALL(msf,
              GetSynonyms("bar", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(syn_bar));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/return_range-reg2mem.ll", req, &msf);

  // Technically a violation in arb_function().
  ASSERT_EQ(res.violations_size(), 1);
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();

  // bar should not be expanded, since it only returns 0.
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res, 0, 100,
      100));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res, 0,
      0, 90));
}

// Tests that specifications can be inferred through initial specifications
// and the embedding only, also testing the confidence is based on the
// similarity and score. Since many of these functions are considered external,
// they can never expand beyond just the highest confidence element.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ExpandNoFunctionDefinition) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *foo_new_specification = req.add_initial_specifications();
  foo_new_specification->mutable_function()->set_source_name("foo_new");
  foo_new_specification->mutable_function()->set_llvm_name("foo_new");
  foo_new_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
  constexpr int kMinimumEvidence = 2;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::string> vocab = {"foo", "foo_new", "foo_get", "foo_set"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  // foo_set is a synonym of foo_get
  std::vector<std::pair<std::string, float>> syn_foo_get = {
      std::make_pair("foo_set", 0.9), std::make_pair("foo", 0.8)};
  EXPECT_CALL(
      msf, GetSynonyms("foo_get", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(syn_foo_get));

  // foo and foo_new are synonyms for foo_set. GetSynonyms() should return the
  // vector in sorted order by similarity score.
  std::vector<std::pair<std::string, float>> syn_foo_set = {
      std::make_pair("foo", 0.9), std::make_pair("foo_new", 0.7)};
  EXPECT_CALL(
      msf, GetSynonyms("foo_set", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(syn_foo_set));

  EXPECT_CALL(msf, GetSynonyms("foo_main", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("foo_get_wrapper", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/no_definition.ll", req, &msf);

  ASSERT_EQ(res.violations_size(), 0);
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 6) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_new", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
      res));

  EXPECT_TRUE(FindSpecification(
      "foo_get", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res,
      0, 81, 0));
  EXPECT_TRUE(FindSpecification(
      "foo_set", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res,
      0, 90, 0));
  EXPECT_TRUE(FindSpecification(
      "foo_main", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res,
      0, 100, 0));
  EXPECT_TRUE(FindSpecification(
      "foo_get_wrapper",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res, 0, 81, 0));
}

// Tests that specifications can be inferred through initial specifications
// and the embedding only, also testing the confidence is based on the
// similarity and score. Since many of these functions are considered external,
// they can never expand beyond just the highest confidence element.
// This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, ExpandNoFunctionDefinitionReg2mem) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *foo_new_specification = req.add_initial_specifications();
  foo_new_specification->mutable_function()->set_source_name("foo_new");
  foo_new_specification->mutable_function()->set_llvm_name("foo_new");
  foo_new_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
  constexpr int kMinimumEvidence = 2;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::string> vocab = {"foo", "foo_new", "foo_get", "foo_set"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  // foo and foo_new are synonyms for foo_set. GetSynonyms() should return the
  // vector in sorted order by similarity score.
  std::vector<std::pair<std::string, float>> syn_foo_set = {
      std::make_pair("foo", 0.9), std::make_pair("foo_new", 0.7)};
  EXPECT_CALL(
      msf, GetSynonyms("foo_set", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(syn_foo_set));

  // foo_set is a synonym of foo_get
  std::vector<std::pair<std::string, float>> syn_foo_get = {
      std::make_pair("foo_set", 0.9), std::make_pair("foo", 0.8)};
  EXPECT_CALL(
      msf, GetSynonyms("foo_get", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(syn_foo_get));

  EXPECT_CALL(msf, GetSynonyms("foo_main", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf, GetSynonyms("foo_get_wrapper", kKVal * kMinimumEvidence,
                               kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/no_definition-reg2mem.ll", req, &msf);

  ASSERT_EQ(res.violations_size(), 0);
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 6) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_new", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
      res));

  EXPECT_TRUE(FindSpecification(
      "foo_get", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res,
      0, 81, 0));
  EXPECT_TRUE(FindSpecification(
      "foo_set", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res,
      0, 90, 0));
  EXPECT_TRUE(FindSpecification(
      "foo_main", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res,
      0, 100, 0));
  EXPECT_TRUE(FindSpecification(
      "foo_get_wrapper",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res, 0, 81, 0));
}

// Tests that there is no expansion in the specifications because the number of
// synonymous functions is less than the minimum evidence specified.
TEST(ErrorBlocksTest, NoExpansionDueToMinimumEvidence) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  constexpr int kMinimumEvidence = 2;  // Two synonymous functions needed.
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // bar3 is a synonym for bar2.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("bar3", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that there is no expansion in the specifications because the number of
// synonymous functions is less than the minimum evidence specified. This
// bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, NoExpansionDueToMinimumEvidenceReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  constexpr int kMinimumEvidence = 2;  // Two synonymous functions needed.
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // bar3 is a synonym for bar2.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("bar3", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label-reg2mem.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that embedding does not lead to expansion of error specifications.
TEST(ErrorBlocksTest, NoExpansionDueToEmbedding) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // No synonyms for bar2.
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that embedding does not lead to expansion of error specifications. This
// bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, NoExpansionDueToEmbeddingReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // No synonyms for bar2.
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label-reg2mem.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that GetSynonyms is not called for functions with non-bottom
// specifications.
TEST(ErrorBlocksTest, DoNotExpandNonBottomFunctions) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *bar2_specification = req.add_initial_specifications();
  bar2_specification->mutable_function()->set_source_name("bar2");
  bar2_specification->mutable_function()->set_llvm_name("bar2");
  bar2_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  EXPECT_CALL(msf, GetSynonyms("bar1", _, _)).Times(0);
  EXPECT_CALL(msf, GetSynonyms("bar2", _, _)).Times(0);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar2", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that GetSynonyms is not called for functions with non-bottom
// specifications. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, DoNotExpandNonBottomFunctionsReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *bar2_specification = req.add_initial_specifications();
  bar2_specification->mutable_function()->set_source_name("bar2");
  bar2_specification->mutable_function()->set_llvm_name("bar2");
  bar2_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  EXPECT_CALL(msf, GetSynonyms("bar1", _, _)).Times(0);
  EXPECT_CALL(msf, GetSynonyms("bar2", _, _)).Times(0);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label-reg2mem.ll", req, &msf);

  ASSERT_EQ(res.violations_size(), 0);
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar2", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that embedding does not lead to expansion of error specifications
// because of mismatched return type.
TEST(ErrorBlocksTest, NoExpansionDueToReturnType) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  Specification *bar3_specification = req.add_initial_specifications();
  bar3_specification->mutable_function()->set_source_name("bar3");
  bar3_specification->mutable_function()->set_llvm_name("bar3");
  bar3_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);

  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // bar3 is a synonym for bar2.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("bar3", 0.0)};
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label.ll", req, &msf);

  // The type for bar3 does not match that of bar2, and bar3 will not be used
  // to expand the specification for bar2.
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar3", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that embedding does not lead to expansion of error specifications
// because of mismatched return type. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, NoExpansionDueToReturnTypeReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar1_specification = req.add_initial_specifications();
  bar1_specification->mutable_function()->set_source_name("bar1");
  bar1_specification->mutable_function()->set_llvm_name("bar1");
  bar1_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  Specification *bar3_specification = req.add_initial_specifications();
  bar3_specification->mutable_function()->set_source_name("bar3");
  bar3_specification->mutable_function()->set_llvm_name("bar3");
  bar3_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);

  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;
  EXPECT_CALL(msf,
              GetSynonyms("EO", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  // bar3 is a synonym for bar2.
  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("bar3", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("bar2", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/two_function_goto_same_label-reg2mem.ll", req, &msf);

  // The type for bar3 does not match that of bar2, and bar3 will not be used
  // to expand the specification for bar2.
  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 3) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar1", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(FindSpecification(
      "bar3", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests specification inference on a function with a if-else checking
// for two different SPECIFIC negative values, returning a negative
// value on the error paths and zero otherwise.
TEST(ErrorBlocksTest, CheckEqualNegEqualNeg) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/check_eqnegative_eqnegative.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if-else checking
// for two different SPECIFIC negative values, returning a negative
// value on the error paths and zero otherwise. This bitcode file uses a Reg2mem
// pass.
TEST(ErrorBlocksTest, CheckEqualNegEqualNegReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/check_eqnegative_eqnegative-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if statement checking
// for one specific negative value, returning a negative value on the error path
// and zero otherwise.
TEST(ErrorBlocksTest, CheckEqualNeg) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/check_eqnegative.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if statement checking
// for one specific negative value, returning a negative value on the error path
// and zero otherwise. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, CheckEqualNegReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/check_eqnegative-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if-statement checking
// for a non-zero return value, followed by a nested if-statement that
// checks for less-than zero, returning a negative value if the previous
// is true, otherwise zero.
TEST(ErrorBlocksTest, CheckNtzNestedCheckLtz) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/check_ntz_nested_check_ltz.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if-statement checking
// for a non-zero return value, followed by a nested if-statement that
// checks for less-than zero, returning a negative value if the previous
// is true, otherwise zero. This bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, CheckNtzNestedCheckLtzReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/check_ntz_nested_check_ltz-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if-statement checking
// for any non-zero value, followed by a nested if-statement that checks for
// a SPECIFIC negative return value, also returning a negative value if the
// previous is true, otherwise the function returns zero. This currently seems
// to be a part of the covering problem, as it would be expected to behave
// similarily to the tests CheckNtzNestedCheckLtz and CheckEqualNeg, however we
// this will consider the return value of 0 as part of the error path.
TEST(ErrorBlocksTest, CheckNtzNestedCheckEqualNeg) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/check_ntz_nested_check_eqnegative.ll", req);

  // TODO (patrickjchap): This is the covering issue, as mentioned in the above
  // comments, we expect this to work similarily to CheckEqualNeg and
  // CheckNtzNestedCheckLtz. However, since if the delta equals the return_range
  // for a function, then that LatticeElementConfidence gets set to all
  // kMinConfidence, causing the specification to not be inferred (considered
  // "unknown").
  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests specification inference on a function with a if-statement checking
// for any non-zero value, followed by a nested if-statement that checks for
// a SPECIFIC negative return value, also returning a negative value if the
// previous is true, otherwise the function returns zero. This currently seems
// to be a part of the covering problem, as it would be expected to behave
// similarily to the tests CheckNtzNestedCheckLtz and CheckEqualNeg, however we
// this will consider the return value of 0 as part of the error path. This
// bitcode file uses a Reg2mem pass.
TEST(ErrorBlocksTest, CheckNtzNestedCheckEqualNegReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/check_ntz_nested_check_eqnegative-reg2mem.ll", req);

  // TODO (patrickjchap): This is the covering issue, as mentioned in the above
  // comments, we expect this to work similarily to CheckEqualNeg and
  // CheckNtzNestedCheckLtz. However, since if the delta equals the return_range
  // for a function, then that LatticeElementConfidence gets set to all
  // kMinConfidence, causing the specification to not be inferred (considered
  // "unknown").
  ASSERT_EQ(res.specifications().size(), 1);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that a constant integer that is indirectly returned along an error path
// is inferred.
TEST(ErrorBlocksTest, IndirectPropagationConstantInt) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/test_indirect_constant_int.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that a constant integer that is indirectly returned along an error path
// is inferred. This bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, IndirectPropagationConstantIntReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/test_indirect_constant_int-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that a constant NULL that is indirectly returned along an error path
// is inferred.
TEST(ErrorBlocksTest, IndirectPropagationConstantNull) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("malloc");
  bar_specification->mutable_function()->set_llvm_name("malloc");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/test_indirect_constant_null.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "malloc", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that a constant NULL that is indirectly returned along an error path
// is inferred. This bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, IndirectPropagationConstantNullReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("malloc");
  bar_specification->mutable_function()->set_llvm_name("malloc");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/test_indirect_constant_null-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 2);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "malloc", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, res));
}

// Tests that constraints for cases related to SwitchInst are handled correctly
// and that error paths are correct along these cases.
TEST(ErrorBlocksTest, SwitchStatement) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/test_switch.ll", req);

  ASSERT_EQ(res.specifications().size(), 5);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_direct_return",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_indirect_return",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_fallthrough_error",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_fallthrough_noerror",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that constraints for cases related to SwitchInst are handled correctly
// and that error paths are correct along these cases. This bitcode file uses
// a Reg2mem pass.
TEST(ErrorBlocksTest, SwitchStatementReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/test_switch-reg2mem.ll", req);

  ASSERT_EQ(res.specifications().size(), 5);
  ASSERT_EQ(res.violations_size(), 0);
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_direct_return",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_indirect_return",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_fallthrough_error",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo_int_fallthrough_noerror",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, res));
}

// Tests that attempting to expand an error specification to the entire return
// range of a function results in ConfidenceLattice::KeepIfMax() setting the
// specification to only keep the confidence values that equal kMaxConfidence.
TEST(ErrorBlocksTest, ExpandKeepIfMax) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *foo_synonym_specification = req.add_initial_specifications();
  foo_synonym_specification->mutable_function()->set_source_name("foo_synonym");
  foo_synonym_specification->mutable_function()->set_llvm_name("foo_synonym");
  foo_synonym_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("foo_synonym", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));
  EXPECT_CALL(msf,
              GetSynonyms("qux", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/keep_max.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 4) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo_synonym",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO, res,
      70, 70, 0));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// Tests that attempting to expand an error specification to the entire return
// range of a function results in ConfidenceLattice::KeepIfMax() setting the
// specification to only keep the confidence values that equal kMaxConfidence.
// This bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, ExpandKeepIfMaxReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  Specification *foo_synonym_specification = req.add_initial_specifications();
  foo_synonym_specification->mutable_function()->set_source_name("foo_synonym");
  foo_synonym_specification->mutable_function()->set_llvm_name("foo_synonym");
  foo_synonym_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
  constexpr int kMinimumEvidence = 1;
  constexpr float kMinimumSimilarity = .5;
  req.mutable_synonym_finder_parameters()->set_minimum_evidence(
      kMinimumEvidence);
  req.mutable_synonym_finder_parameters()->set_minimum_similarity(
      kMinimumSimilarity);

  MockSynonymFinder msf;
  using ::testing::_;
  using ::testing::Return;

  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("foo_synonym", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));
  EXPECT_CALL(msf,
              GetSynonyms("qux", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/keep_max-reg2mem.ll", req, &msf);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 4) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "foo_synonym",
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO, res,
      70, 70, 0));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "qux", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
}

// This is the simplest case of an attempt to update an error specification to
// the entire return range of a function, which should just result in the
// specification being left as bottom/"unknown". This ensures that the
// MaxEquals() call in ErrorBlocksPass is working as expected. This is similar
// to tests such as BazCoverBar, except this test will not assign a new value to
// a checked variable. In the future, if the analysis handles StoreInst by
// kiling constraints when a value "dies", then BazCoverBar will be a completely
// different case from this.
// TODO(patrickjchap): Update this comment if StoreInst are eventually handled
// to kill constraints in the ReturnConstraintsPass.
TEST(ErrorBlocksTest, DeltaEqualsReturnRange) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/equals_return_range.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
      res));
}

// This is the simplest case of an attempt to update an error specification to
// the entire return range of a function, which should just result in the
// specification being left as bottom/"unknown". This ensures that the
// MaxEquals() call in ErrorBlocksPass is working as expected. This is similar
// to tests such as BazCoverBar, except this test will not assign a new value to
// a checked variable. In the future, if the analysis handles StoreInst by
// kiling constraints when a value "dies", then BazCoverBar will be a completely
// different case from this. This bitcode file uses a reg2mem pass.
// TODO(patrickjchap): Update this comment if StoreInst are eventually handled
// to kill constraints in the ReturnConstraintsPass.
TEST(ErrorBlocksTest, DeltaEqualsReturnRangeReg2mem) {
  GetSpecificationsRequest req;
  Specification *bar_specification = req.add_initial_specifications();
  bar_specification->mutable_function()->set_source_name("bar");
  bar_specification->mutable_function()->set_llvm_name("bar");
  bar_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/equals_return_range-reg2mem.ll", req);

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 0);

  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
      res));
}

}  // namespace error_specifications
