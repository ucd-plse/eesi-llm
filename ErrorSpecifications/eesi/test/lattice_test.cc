#include "gtest/gtest.h"

#include "constraint.h"
#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

TEST(LatticeTest, IsBottom) {
  ASSERT_TRUE(
      SignLattice::IsBottom(SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM));
  ASSERT_FALSE(SignLattice::IsBottom(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO));
}

TEST(LatticeTest, JoinBottomBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinBottomTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinLessThanEqualZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinGreaterThanEqualZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinNotZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, JoinTopTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Join(x, y), res);
  ASSERT_EQ(SignLattice::Join(y, x), res);
}

TEST(LatticeTest, MeetBottomBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetBottomTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetLessThanEqualZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetGreaterThanEqualZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetNotZeroTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopBottom) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopLessThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopGreaterThanZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopLessThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopGreaterThanEqualZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopNotZero) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}

TEST(LatticeTest, MeetTopTop) {
  auto x = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto y = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  auto res = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  ASSERT_EQ(SignLattice::Meet(x, y), res);
  ASSERT_EQ(SignLattice::Meet(y, x), res);
}
}  // namespace error_specifications
