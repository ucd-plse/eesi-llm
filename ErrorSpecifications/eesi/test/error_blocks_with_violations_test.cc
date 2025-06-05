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

// Tests that an unchecked violation should be found in main if the initial
// specification passed is printf, <0.
TEST(UnusedCallsTest, UncheckedPrintf) {
  GetSpecificationsRequest req;
  Specification *printf_specification = req.add_initial_specifications();
  printf_specification->mutable_function()->set_source_name("printf");
  printf_specification->mutable_function()->set_llvm_name("printf");
  printf_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/hello.ll", req);

  ASSERT_EQ(res.specifications_size(), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 1);

  EXPECT_EQ(res.specifications(0).function().source_name(), "printf");
  EXPECT_EQ(res.specifications(0).function().llvm_name(), "printf");
  EXPECT_EQ(res.specifications(0).lattice_element(),
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  EXPECT_EQ(res.violations(0).parent_function().source_name(), "main");
  EXPECT_EQ(res.violations(0).parent_function().llvm_name(), "main");
  EXPECT_EQ(res.violations(0).specification().function().source_name(),
            "printf");
  EXPECT_EQ(res.violations(0).specification().function().llvm_name(), "printf");
  EXPECT_EQ(res.violations(0).location().line(), 3);
  EXPECT_EQ(res.violations(0).location().file(), "hello.c");
}

// Tests that an unchecked violation should be found in main if the initial
// specification passed is printf, <0. This bitcode file uses a Reg2mem pass.
TEST(UnusedCallsTest, UncheckedPrintfReg2mem) {
  GetSpecificationsRequest req;
  Specification *printf_specification = req.add_initial_specifications();
  printf_specification->mutable_function()->set_source_name("printf");
  printf_specification->mutable_function()->set_llvm_name("printf");
  printf_specification->set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/hello-reg2mem.ll", req);

  ASSERT_EQ(res.specifications_size(), 1) << res.DebugString();
  ASSERT_EQ(res.violations_size(), 1);

  EXPECT_EQ(res.specifications(0).function().source_name(), "printf");
  EXPECT_EQ(res.specifications(0).function().llvm_name(), "printf");
  EXPECT_EQ(res.specifications(0).lattice_element(),
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);

  EXPECT_EQ(res.violations(0).parent_function().source_name(), "main");
  EXPECT_EQ(res.violations(0).parent_function().llvm_name(), "main");
  EXPECT_EQ(res.violations(0).specification().function().source_name(),
            "printf");
  EXPECT_EQ(res.violations(0).specification().function().llvm_name(), "printf");
  EXPECT_EQ(res.violations(0).location().line(), 4);
  EXPECT_EQ(res.violations(0).location().file(),
            "/home/daniel/ucd/indra/ErrorSpecifications/test/programs/hello.c");
}

// Tests that specifications that contain the lattice element TOP do not cause
// unchecked return values to be counted as violations.
TEST(UnusedCallsTest, IgnoreUncheckedTop) {
  GetSpecificationsRequest req;
  Specification *spec = req.add_initial_specifications();
  spec->mutable_function()->set_source_name("bar");
  spec->set_lattice_element(SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/saved_return.ll", req);

  // Behavior of checker is to discard specifications with TOP, as these
  // are unlikely to be useful.
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests that specifications that contain the lattice element TOP do not cause
// unchecked return values to be counted as violations. This bitcode file uses
// a Reg2mem pass.
TEST(UnusedCallsTest, IgnoreUncheckedTopReg2mem) {
  GetSpecificationsRequest req;
  Specification *spec = req.add_initial_specifications();
  spec->mutable_function()->set_source_name("bar");
  spec->set_lattice_element(SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/saved_return-reg2mem.ll", req);

  // Behavior of checker is to discard specifications with TOP, as these
  // are unlikely to be useful.
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests that specifications that contain the lattice element BOTTOM do not
// cause unchecked return values to be counted as violations.
TEST(UnusedCallsTest, IgnoreUncheckedBottom) {
  GetSpecificationsRequest req;
  Specification *spec = req.add_initial_specifications();
  spec->mutable_function()->set_source_name("bar");
  spec->set_lattice_element(SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/saved_return.ll", req);
  ASSERT_EQ(res.violations_size(), 0);
}

// Tests that specifications that contain the lattice element BOTTOM do not
// cause unchecked return values to be counted as violations. This bitcode file
// uses a Reg2mem pass.
TEST(UnusedCallsTest, IgnoreUncheckedBottomReg2mem) {
  GetSpecificationsRequest req;
  Specification *spec = req.add_initial_specifications();
  spec->mutable_function()->set_source_name("bar");
  spec->set_lattice_element(SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/saved_return-reg2mem.ll", req);

  ASSERT_EQ(res.violations_size(), 0);
}

// Tests that violations can be found after specifications have been expanded
// using the embedding, while also being contained within a SCC.
TEST(UnusedCallsTest, ExpandSpecificationsAndUnusedViolation) {
  GetSpecificationsRequest req;
  Specification *baz_synonym_specification = req.add_initial_specifications();
  baz_synonym_specification->mutable_function()->set_source_name("baz_synonym");
  baz_synonym_specification->mutable_function()->set_llvm_name("baz_synonym");
  baz_synonym_specification->set_lattice_element(
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

  std::vector<std::string> vocab = {"bar", "foo", "baz"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  // An expansion will be attempted on both "bar" and "foo", both will not have
  // any synonyms returned.
  EXPECT_CALL(msf,
              GetSynonyms("bar", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("baz_synonym", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("baz", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res =
      RunErrorBlocks("testdata/programs/scc_functions_f2v.ll", req, &msf);

  ASSERT_EQ(res.specifications_size(), 2);
  ASSERT_EQ(res.violations_size(), 2);

  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res, 0,
      70, 0));
  EXPECT_TRUE(FindSpecification(
      "baz_synonym", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
}

// Tests that violations can be found after specifications have been expanded
// using the embedding, while also being contained within a SCC. This bitcode
// file uses a Reg2mem pass.
TEST(UnusedCallsTest, ExpandSpecificationsAndUnusedViolationReg2mem) {
  GetSpecificationsRequest req;
  Specification *baz_synonym_specification = req.add_initial_specifications();
  baz_synonym_specification->mutable_function()->set_source_name("baz_synonym");
  baz_synonym_specification->mutable_function()->set_llvm_name("baz_synonym");
  baz_synonym_specification->set_lattice_element(
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

  std::vector<std::string> vocab = {"bar", "foo", "baz"};
  EXPECT_CALL(msf, GetVocabulary()).WillOnce(Return(vocab));

  // foo_set is a synonym of foo_get
  // foo_set is a synonym of foo_get
  // An expansion will be attempted on both "bar" and "foo", both will not have
  // any synonyms returned.
  EXPECT_CALL(msf,
              GetSynonyms("bar", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));
  EXPECT_CALL(msf,
              GetSynonyms("foo", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(std::vector<std::pair<std::string, float>>()));

  std::vector<std::pair<std::string, float>> funcs = {
      std::make_pair("baz_synonym", 0.7)};
  EXPECT_CALL(msf,
              GetSynonyms("baz", kKVal * kMinimumEvidence, kMinimumSimilarity))
      .WillOnce(Return(funcs));

  GetSpecificationsResponse res = RunErrorBlocks(
      "testdata/programs/scc_functions_f2v-reg2mem.ll", req, &msf);

  ASSERT_EQ(res.specifications_size(), 2);
  ASSERT_EQ(res.violations_size(), 2);

  EXPECT_TRUE(FindSpecification(
      "baz", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, res, 0,
      70, 0));
  EXPECT_TRUE(FindSpecification(
      "baz_synonym", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
      res));
}

}  // namespace error_specifications
