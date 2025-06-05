#include "return_constraints_pass.h"

#include "glog/logging.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

namespace error_specifications {

std::set<SignLatticeElement> RunGetConstraints(const std::string &bitcode_path,
                                               const std::string &function_name,
                                               Function &called_function) {
  ReturnConstraintsPass *return_constraints_pass = new ReturnConstraintsPass();

  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> mod(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  if (!mod) {
    err.print("error-blocks-test", llvm::errs());
  }

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(return_constraints_pass);
  pass_manager.run(*mod);

  return return_constraints_pass->GetConstraints(*mod, function_name,
                                                 called_function);
}

// Tests that that constraints on multiple else branches are properly combined.
TEST(ErrorBlocksTest, MeetMultipleElse) {
  Function mustcheck_lez;
  mustcheck_lez.set_llvm_name("mustcheck_lez");
  mustcheck_lez.set_source_name("mustcheck_lez");

  auto constraints = RunGetConstraints(
      "testdata/programs/mustcheck_lez_split.ll", "eq_on_else", mustcheck_lez);

  // Assert that >0 exists as a constraint for mustcheck.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,

  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that that constraints on multiple else branches are properly combined.
// Bitcode file uses a reg2mem pass optimization.
TEST(ErrorBlocksTest, MeetMultipleElseReg2mem) {
  Function mustcheck_lez;
  mustcheck_lez.set_llvm_name("mustcheck_lez");
  mustcheck_lez.set_source_name("mustcheck_lez");

  auto constraints = RunGetConstraints(
      "testdata/programs/mustcheck_lez_split-reg2mem.ll", "eq_on_else", mustcheck_lez);

  // Assert that >0 exists as a constraint for mustcheck.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,

  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that that constraints on multiple else branches are properly combined.
TEST(ErrorBlocksTest, MeetMultipleThen) {
  Function mustcheck_lez;
  mustcheck_lez.set_llvm_name("mustcheck_lez");
  mustcheck_lez.set_source_name("mustcheck_lez");

  auto constraints = RunGetConstraints(
      "testdata/programs/mustcheck_lez_split.ll", "eq_on_then", mustcheck_lez);

  // Assert that >0 exists as a constraint for mustcheck.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,

  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that that constraints on multiple else branches are properly combined.
// Bitcode file uses a reg2mem pass optimization.
TEST(ErrorBlocksTest, MeetMultipleThenReg2mem) {
  Function mustcheck_lez;
  mustcheck_lez.set_llvm_name("mustcheck_lez");
  mustcheck_lez.set_source_name("mustcheck_lez");

  auto constraints = RunGetConstraints(
      "testdata/programs/mustcheck_lez_split-reg2mem.ll", "eq_on_then", mustcheck_lez);

  // Assert that >0 exists as a constraint for mustcheck.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,

  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that ReturnConstraints correctly applies meet for nested ifs.
TEST(ErrorBlocksTest, NestedIfDead) {
  Function bar;
  bar.set_llvm_name("bar");
  bar.set_source_name("bar");

  auto constraints =
      RunGetConstraints("testdata/programs/nested_if_dead.ll", "foo", bar);

  // Assert that >0 exists as a constraint for mustcheck.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that ReturnConstraints correctly applies meet for nested ifs.
// Bitcode file uses a reg2mem pass optimization.
TEST(ErrorBlocksTest, NestedIfDeadReg2mem) {
  Function bar;
  bar.set_llvm_name("bar");
  bar.set_source_name("bar");

  auto constraints =
      RunGetConstraints("testdata/programs/nested_if_dead-reg2mem.ll", "foo", bar);

  // Assert that >0 exists as a constraint for mustcheck.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests the case where an if statement checks a variable that can hold more
// than one function return value.
TEST(ErrorBlocksTest, MultiFunctionCheck) {
  Function bar;
  bar.set_llvm_name("bar");
  bar.set_source_name("bar");

  auto constraints =
      RunGetConstraints("testdata/programs/multi_func_check.ll", "baz", bar);

  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests the case where an if statement checks a variable that can hold more
// than one function return value.  This bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, MultiFunctionCheckReg2mem) {
  Function bar;
  bar.set_llvm_name("bar");
  bar.set_source_name("bar");

  auto constraints = RunGetConstraints(
      "testdata/programs/multi_func_check-reg2mem.ll", "baz", bar);

  // Unlike the regular case, the reg2mem version has an additional >=0
  // constraint due to a critical edge.
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that ReturnConstraints can handle range checks.
TEST(ErrorBlocksTest, RangeCheck) {
  Function read_number;
  read_number.set_llvm_name("read_number");
  read_number.set_source_name("read_number");

  auto constraints =
      RunGetConstraints("testdata/programs/range_error.ll", "foo", read_number);

  // read_number's expected constraint for each basic block is:
  //
  // 0:  nonexistent
  // 6:  >=0 (x >= 0)
  // 9:  >=0 (0 <= x <= SOME_UPPER_LIMIT)
  // 12: !=0 (x < 0 or x > SOME_UPPER_LIMIT)
  // 13: top
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests that ReturnConstraints can handle range checks.  This bitcode file uses
// a reg2mem pass optimization.
TEST(ErrorBlocksTest, RangeCheckReg2mem) {
  Function read_number;
  read_number.set_llvm_name("read_number");
  read_number.set_source_name("read_number");

  auto constraints = RunGetConstraints(
      "testdata/programs/range_error-reg2mem.ll", "foo", read_number);

  // read_number's expected constraint for each basic block is:
  //
  // 0:            nonexistent
  // ._crit_edge:  <0  (x < 0)
  // 6:            >=0 (x >= 0)
  // ._crit_edge1: >0  (x > SOME_UPPER_LIMIT)
  // 9:            >=0 (0 <= x <= SOME_UPPER_LIMIT)
  // 12:           !=0 (x < 0 or x > SOME_UPPER_LIMIT)
  // 13:           top
  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests the case where the function return value is the second operand in an
// icmp instruction.
TEST(ErrorBlocksTest, ReverseCheck) {
  Function foo;
  foo.set_llvm_name("foo");
  foo.set_source_name("foo");

  auto constraints =
      RunGetConstraints("testdata/programs/reverse_check.ll", "bar", foo);

  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

// Tests the case where the function return value is the second operand in an
// icmp instruction.  This bitcode file uses a reg2mem pass.
TEST(ErrorBlocksTest, ReverseCheckReg2mem) {
  Function foo;
  foo.set_llvm_name("foo");
  foo.set_source_name("foo");

  auto constraints = RunGetConstraints(
      "testdata/programs/reverse_check-reg2mem.ll", "bar", foo);

  std::set<SignLatticeElement> expected_constraints = {
      SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
  };

  ASSERT_EQ(constraints, expected_constraints);
}

}  // namespace error_specifications
