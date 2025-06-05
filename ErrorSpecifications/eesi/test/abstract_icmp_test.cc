// These test the map from operator + lattice value to the
// lattice value on the true/false branches of a conditional statement.

#include "gtest/gtest.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "proto/eesi.grpc.pb.h"
#include "return_constraints_pass.h"

namespace error_specifications {

class AbstractICmpTest : public ::testing::Test {
 protected:
  llvm::LLVMContext llvm_context_;
  llvm::Type *i32_type_ = llvm::IntegerType::getInt32Ty(llvm_context_);

  llvm::AllocaInst *alloca_ = new llvm::AllocaInst(i32_type_, 0, "");
  llvm::LoadInst *filler_ = new llvm::LoadInst(alloca_, "");
  llvm::Constant *zero_ = llvm::ConstantInt::get(i32_type_, 0, true);
  llvm::Constant *positive_ = llvm::ConstantInt::get(i32_type_, 1, true);
  llvm::Constant *negative_ = llvm::ConstantInt::get(i32_type_, -1, true);

  void TearDown() override {
    delete filler_;
    delete alloca_;
  }
};

TEST_F(AbstractICmpTest, SignedLessThanZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLT, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGT, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULT, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGT, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGT, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLT, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGT, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULT, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanEqualZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLE, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanEqualZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGE, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanEqualZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULE, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanEqualZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGE, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanEqualZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGE, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanEqualZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLE, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanEqualZero) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGE, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanEqualZeroReversed) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULE, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
}

TEST_F(AbstractICmpTest, EqualZeroRight) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_EQ, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
}

TEST_F(AbstractICmpTest, EqualZeroLeft) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_EQ, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
}

TEST_F(AbstractICmpTest, NotEqualZeroLeft) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_NE, zero_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
}

TEST_F(AbstractICmpTest, NotEqualZeroRight) {
  auto *icmp =
      new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_NE, filler_, zero_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLT, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGT,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULT, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGT,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGT, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, SignedGreaterThanPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLT,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGT, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULT,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanEqualPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLE, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanEqualPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGE,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanEqualPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULE, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanEqualPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGE,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanEqualPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGE, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, SignedGreaterThanEqualPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLE,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanEqualPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGE, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanEqualPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULE,
                                  positive_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, EqualPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_EQ, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, EqualPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_EQ, positive_,
                                  filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, NotEqualPositive) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_NE, filler_,
                                  positive_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, NotEqualPositiveReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_NE, positive_,
                                  filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedLessThanNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLT, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, SignedLessThanNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGT,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, UnsignedLessThanNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULT, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGT,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGT, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLT,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGT, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULT,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
}

TEST_F(AbstractICmpTest, SignedLessThanEqualNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLE, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, SignedLessThanEqualNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGE,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, UnsignedLessThanEqualNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULE, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedLessThanEqualNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGE,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanEqualNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SGE, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, SignedGreaterThanEqualNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_SLE,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanEqualNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_UGE, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
}

TEST_F(AbstractICmpTest, UnsignedGreaterThanEqualNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_ULE,
                                  negative_, filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM);
}

TEST_F(AbstractICmpTest, EqualNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_EQ, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, EqualNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_EQ, negative_,
                                  filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
  ASSERT_EQ(false_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
}

TEST_F(AbstractICmpTest, NotEqualNegative) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_NE, filler_,
                                  negative_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

TEST_F(AbstractICmpTest, NotEqualNegativeReversed) {
  auto *icmp = new llvm::ICmpInst(llvm::ICmpInst::Predicate::ICMP_NE, negative_,
                                  filler_);
  SignLatticeElement true_branch;
  SignLatticeElement false_branch;
  std::tie(true_branch, false_branch) =
      ReturnConstraintsPass::AbstractICmp(*icmp);
  delete icmp;

  ASSERT_EQ(true_branch, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  ASSERT_EQ(false_branch,
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO);
}

}  // namespace error_specifications
