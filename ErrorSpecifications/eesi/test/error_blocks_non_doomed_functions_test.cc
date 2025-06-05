#include <memory>

#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "error_blocks_helper.h"
#include "error_blocks_pass.h"
#include "mock_synonym_finder.h"
#include "proto/eesi.pb.h"
#include "return_constraints_pass.h"
#include "return_propagation_pass.h"
#include "returned_values_pass.h"

namespace error_specifications {

// Tests a program that contains one initial specification, one inferrable
// specification, one purely non-doomed function that is reachable from the
// domain knowledge, and one doomed function. Our doomed function is not
// represented in an embedding (as we do not even use one) and it is not
// considered reachable from our domain knowledge.
TEST(NonDoomedFunctionsTest, NonDoomedFunctions) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  // The second value in our return pair is our non-doomed functions.
  std::pair<GetSpecificationsResponse, std::unordered_set<std::string>> p =
      RunErrorBlocksAndGetNonDoomedFunctions(
          "testdata/programs/non_doomed_functions.ll", req);
  GetSpecificationsResponse res = p.first;
  std::unordered_set<std::string> non_doomed_functions = p.second;

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  // The violation is in the reachable function.
  ASSERT_EQ(res.violations_size(), 1);
  ASSERT_EQ(non_doomed_functions.size(), 3);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(non_doomed_functions.find("foo") != non_doomed_functions.end());
  EXPECT_TRUE(non_doomed_functions.find("bar") != non_doomed_functions.end());
  EXPECT_TRUE(non_doomed_functions.find("baz") != non_doomed_functions.end());
  EXPECT_TRUE(non_doomed_functions.find("quz") == non_doomed_functions.end());
}

// Tests a program that contains one initial specification, one inferrable
// specification, one purely non-doomed function that is reachable from the
// domain knowledge, and one doomed function. Our doomed function is not
// represented in an embedding (as we do not even use one) and it is not
// considered reachable from our domain knowledge. This bitcode file uses a
// Reg2mem pass.
TEST(NonDoomedFunctionsTest, NonDoomedFunctionsReg2mem) {
  GetSpecificationsRequest req;
  Specification *foo_specification = req.add_initial_specifications();
  foo_specification->mutable_function()->set_source_name("foo");
  foo_specification->mutable_function()->set_llvm_name("foo");
  foo_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  // The second value in our return pair is our non-doomed functions.
  std::pair<GetSpecificationsResponse, std::unordered_set<std::string>> p =
      RunErrorBlocksAndGetNonDoomedFunctions(
          "testdata/programs/non_doomed_functions-reg2mem.ll", req);
  GetSpecificationsResponse res = p.first;
  std::unordered_set<std::string> non_doomed_functions = p.second;

  ASSERT_EQ(GetNonEmptySpecificationsCount(res), 2) << res.DebugString();
  // The violation is in the reachable function.
  ASSERT_EQ(res.violations_size(), 1);
  ASSERT_EQ(non_doomed_functions.size(), 3);

  EXPECT_TRUE(FindSpecification(
      "foo", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));
  EXPECT_TRUE(FindSpecification(
      "bar", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res));

  EXPECT_TRUE(non_doomed_functions.find("foo") != non_doomed_functions.end());
  EXPECT_TRUE(non_doomed_functions.find("bar") != non_doomed_functions.end());
  EXPECT_TRUE(non_doomed_functions.find("baz") != non_doomed_functions.end());
  EXPECT_TRUE(non_doomed_functions.find("quz") == non_doomed_functions.end());
}

}  // namespace error_specifications
