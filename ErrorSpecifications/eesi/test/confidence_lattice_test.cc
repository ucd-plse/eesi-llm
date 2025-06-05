#include "gtest/gtest.h"

#include "confidence_lattice.h"
#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

// LatticeElementConfidences with kMaxConfidence only.
LatticeElementConfidence max_zero_lattice_confidence(kMaxConfidence,
                                                     kMinConfidence,
                                                     kMinConfidence,
                                                     kMinConfidence);
LatticeElementConfidence max_less_than_zero_lattice_confidence(kMinConfidence,
                                                               kMaxConfidence,
                                                               kMinConfidence,
                                                               kMinConfidence);
LatticeElementConfidence max_greater_than_zero_lattice_confidence(
    kMinConfidence, kMinConfidence, kMaxConfidence, kMinConfidence);
LatticeElementConfidence max_emptyset_lattice_confidence(kMinConfidence,
                                                         kMinConfidence,
                                                         kMinConfidence,
                                                         kMaxConfidence);
LatticeElementConfidence max_less_than_equal_zero_lattice_confidence(
    kMaxConfidence, kMaxConfidence, kMinConfidence, kMinConfidence);
LatticeElementConfidence max_greater_than_equal_zero_lattice_confidence(
    kMaxConfidence, kMinConfidence, kMaxConfidence, kMinConfidence);
LatticeElementConfidence max_not_zero_lattice_confidence(kMinConfidence,
                                                         kMaxConfidence,
                                                         kMaxConfidence,
                                                         kMinConfidence);
LatticeElementConfidence max_top_lattice_confidence(kMaxConfidence,
                                                    kMaxConfidence,
                                                    kMaxConfidence,
                                                    kMinConfidence);

// LatticeElementConfidences with no kMaxConfidence (kMaxConfidence / 2 in this
// case).
LatticeElementConfidence non_max_zero_lattice_confidence(kMaxConfidence / 2,
                                                         kMinConfidence,
                                                         kMinConfidence,
                                                         kMinConfidence);
LatticeElementConfidence non_max_less_than_zero_lattice_confidence(
    kMinConfidence, kMaxConfidence / 2, kMinConfidence, kMinConfidence);
LatticeElementConfidence non_max_greater_than_zero_lattice_confidence(
    kMinConfidence, kMinConfidence, kMaxConfidence / 2, kMinConfidence);
LatticeElementConfidence non_max_emptyset_lattice_confidence(
    kMinConfidence, kMinConfidence, kMinConfidence, kMaxConfidence / 2);
LatticeElementConfidence non_max_less_than_equal_zero_lattice_confidence(
    kMaxConfidence / 2, kMaxConfidence / 2, kMinConfidence, kMinConfidence);
LatticeElementConfidence non_max_greater_than_equal_zero_lattice_confidence(
    kMaxConfidence / 2, kMinConfidence, kMaxConfidence / 2, kMinConfidence);
LatticeElementConfidence non_max_not_zero_lattice_confidence(kMinConfidence,
                                                             kMaxConfidence / 2,
                                                             kMaxConfidence / 2,
                                                             kMinConfidence);
LatticeElementConfidence non_max_top_lattice_confidence(kMaxConfidence / 2,
                                                        kMaxConfidence / 2,
                                                        kMaxConfidence / 2,
                                                        kMinConfidence);

// LatticeElementConfidences with mixed confidence values (kMaxConfidence and
// kMaxConfidence / 2). The only mixed variants will include those that have
// two or more non-kMinConfidence values (e.g., >=0 and not ==0).
LatticeElementConfidence mixed_less_than_equal_zero_lattice_confidence(
    kMaxConfidence / 2, kMaxConfidence, kMinConfidence, kMinConfidence);
LatticeElementConfidence mixed_greater_than_equal_zero_lattice_confidence(
    kMaxConfidence / 2, kMinConfidence, kMaxConfidence, kMinConfidence);
LatticeElementConfidence mixed_not_zero_lattice_confidence(kMinConfidence,
                                                           kMaxConfidence,
                                                           kMaxConfidence / 2,
                                                           kMinConfidence);
LatticeElementConfidence mixed_top_lattice_confidence(kMaxConfidence,
                                                      kMaxConfidence / 2,
                                                      kMaxConfidence / 2,
                                                      kMinConfidence);

// LatticeElementConfidence bottom, applicable everywhere.
LatticeElementConfidence bottom_lattice_confidence(kMinConfidence,
                                                   kMinConfidence,
                                                   kMinConfidence,
                                                   kMinConfidence);

// For checking the number of occurences that a LatticeElementConfidence
// ConfidenceLattice::MaxEquals() over the SignLatticeElement enum.
int checkMaxEqualsOccurences(LatticeElementConfidence lattice_confidence) {
  int found = 0;
  // Iterate over enum.
  for (int int_lattice_element =
           SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
       int_lattice_element <= SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
       int_lattice_element++) {
    // Casting back to SignLatticeElement.
    SignLatticeElement lattice_element =
        static_cast<SignLatticeElement>(int_lattice_element);

    // Should only evaluate to true once.
    if (ConfidenceLattice::MaxEquals(lattice_confidence, lattice_element)) {
      found++;
      continue;
    }
    // Every other time should evaluate to false.
    EXPECT_FALSE(
        ConfidenceLattice::MaxEquals(lattice_confidence, lattice_element));
  }
  return found;
}

// For checking the number of occurences that a LatticeElementConfidence
// ConfidenceLattice::Equals() over the SignLatticeElement enum.
int checkEqualsOccurences(LatticeElementConfidence lattice_confidence) {
  int found = 0;
  // Iterate over enum.
  for (int int_lattice_element =
           SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
       int_lattice_element <= SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
       int_lattice_element++) {
    // Casting back to SignLatticeElement.
    SignLatticeElement lattice_element =
        static_cast<SignLatticeElement>(int_lattice_element);

    // Should only evaluate to true once.
    if (ConfidenceLattice::Equals(lattice_confidence, lattice_element)) {
      found++;
      continue;
    }
    // Every other time should evaluate to false.
    EXPECT_FALSE(
        ConfidenceLattice::Equals(lattice_confidence, lattice_element));
  }
  return found;
}

// Tests for MaxEquals

// Tests LatticeElementConfidence with kMaxConfidence on zero max equality.
TEST(ConfidenceLattice, MaxEqualsZero) {
  EXPECT_EQ(checkMaxEqualsOccurences(max_zero_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with kMaxConfidence on less_than_zero max
// equality.
TEST(ConfidenceLattice, MaxEqualsLessThanZero) {
  EXPECT_EQ(checkMaxEqualsOccurences(max_less_than_zero_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with kMaxConfidence on less_than_equal_zero
// max equality.
TEST(ConfidenceLattice, MaxEqualsLessThanEqualZero) {
  EXPECT_EQ(
      checkMaxEqualsOccurences(max_less_than_equal_zero_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with kMaxConfidence on greater_than_zero max
// equality.
TEST(ConfidenceLattice, MaxEqualsGreaterThanZero) {
  EXPECT_EQ(checkMaxEqualsOccurences(max_greater_than_zero_lattice_confidence),
            1);
}

// Tests LatticeElementConfidence with kMaxConfidence on greater_than_equal_zero
// max equality.
TEST(ConfidenceLattice, MaxEqualsGreaterThanEqualZero) {
  EXPECT_EQ(
      checkMaxEqualsOccurences(max_greater_than_equal_zero_lattice_confidence),
      1);
}

// Tests LatticeElementConfidence with kMaxConfidence on not_zero max equality.
TEST(ConfidenceLattice, MaxEqualsNotZero) {
  EXPECT_EQ(checkMaxEqualsOccurences(max_zero_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with kMaxConfidence on top max equality.
TEST(ConfidenceLattice, MaxEqualsTop) {
  EXPECT_EQ(checkMaxEqualsOccurences(max_top_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with kMaxConfidence on bottom max equality.
TEST(ConfidenceLattice, MaxEqualsBottom) {
  EXPECT_EQ(checkMaxEqualsOccurences(bottom_lattice_confidence), 1);
}

// Tests Equals

// Tests LatticeElementConfidence with non-kMaxConfidence on zero equality.
TEST(ConfidenceLattice, EqualsZero) {
  EXPECT_EQ(checkEqualsOccurences(non_max_zero_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on less_than_zero
// equality.
TEST(ConfidenceLattice, EqualsLessThanZero) {
  EXPECT_EQ(checkEqualsOccurences(non_max_less_than_zero_lattice_confidence),
            1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on
// less_than_equal_zero
//  equality.
TEST(ConfidenceLattice, EqualsLessThanEqualZero) {
  EXPECT_EQ(
      checkEqualsOccurences(non_max_less_than_equal_zero_lattice_confidence),
      1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on greater_than_zero
// equality.
TEST(ConfidenceLattice, EqualsGreaterThanZero) {
  EXPECT_EQ(checkEqualsOccurences(non_max_greater_than_zero_lattice_confidence),
            1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on
// greater_than_equal_zero
//  equality.
TEST(ConfidenceLattice, EqualsGreaterThanEqualZero) {
  EXPECT_EQ(
      checkEqualsOccurences(non_max_greater_than_equal_zero_lattice_confidence),
      1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on not_zero equality.
TEST(ConfidenceLattice, EqualsNotZero) {
  EXPECT_EQ(checkEqualsOccurences(non_max_zero_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on top equality.
TEST(ConfidenceLattice, EqualsTop) {
  EXPECT_EQ(checkEqualsOccurences(non_max_top_lattice_confidence), 1);
}

// Tests LatticeElementConfidence with non-kMaxConfidence on bottom equality.
TEST(ConfidenceLattice, EqualsBottom) {
  EXPECT_EQ(checkEqualsOccurences(bottom_lattice_confidence), 1);
}

// KeepIfMax tests

// Tests KeepIfMax on ==0 with kMaxConfidence. LatticeElementConfidence should
// not change.
TEST(ConfidenceLattice, KeepIfMaxZero) {
  LatticeElementConfidence keep_max =
      ConfidenceLattice::KeepIfMax(max_zero_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMinConfidence);
}

// Tests KeepIfMax on <0 with kMaxConfidence. LatticeElementConfidence should
// not change.
TEST(ConfidenceLattice, KeepIfMaxLessThanZero) {
  LatticeElementConfidence keep_max =
      ConfidenceLattice::KeepIfMax(max_less_than_zero_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMaxConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMinConfidence);
}

// Tests KeepIfMax on >0 with kMaxConfidence. LatticeElementConfidence should
// not change.
TEST(ConfidenceLattice, KeepIfMaxGreaterThanZero) {
  LatticeElementConfidence keep_max =
      ConfidenceLattice::KeepIfMax(max_greater_than_zero_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMaxConfidence);
}

// Tests KeepIfMax on <=0 with mixed confidence values.
TEST(ConfidenceLattice, KeepIfMaxLessThanEqualZero) {
  LatticeElementConfidence keep_max = ConfidenceLattice::KeepIfMax(
      mixed_less_than_equal_zero_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMaxConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMinConfidence);
}

// Tests KeepIfMax on >=0 with mixed confidence values.
TEST(ConfidenceLattice, KeepIfMaxGreaterThanEqualZero) {
  LatticeElementConfidence keep_max = ConfidenceLattice::KeepIfMax(
      mixed_greater_than_equal_zero_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMaxConfidence);
}

// Tests KeepIfMax on !=0 with mixed confidence values.
TEST(ConfidenceLattice, KeepIfMaxNotZero) {
  LatticeElementConfidence keep_max =
      ConfidenceLattice::KeepIfMax(mixed_not_zero_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMaxConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMinConfidence);
}

// Tests KeepIfMax on top with mixed confidence values.
TEST(ConfidenceLattice, KeepIfMaxTop) {
  LatticeElementConfidence keep_max =
      ConfidenceLattice::KeepIfMax(mixed_top_lattice_confidence);
  EXPECT_EQ(keep_max.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(keep_max.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(keep_max.GetConfidenceGreaterThanZero(), kMinConfidence);
}

// RemoveLowestNonMin tests

// Tests removing lowest confidence value from LatticeElementConfidence. In this
// case removing zero confidence values.
TEST(ConfidenceLattice, RemoveLowestNonMinZero) {
  // TOP -> !=0
  LatticeElementConfidence partial_top(kMaxConfidence / 4, kMaxConfidence,
                                       kMaxConfidence / 2);
  partial_top = ConfidenceLattice::RemoveLowestNonMin(partial_top);
  EXPECT_EQ(partial_top.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_top.GetConfidenceLessThanZero(), kMaxConfidence);
  EXPECT_EQ(partial_top.GetConfidenceGreaterThanZero(), kMaxConfidence / 2);

  // <=0 -> <0
  LatticeElementConfidence partial_lteqz(kMaxConfidence / 4, kMaxConfidence,
                                         kMinConfidence);
  partial_lteqz = ConfidenceLattice::RemoveLowestNonMin(partial_lteqz);
  EXPECT_EQ(partial_lteqz.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_lteqz.GetConfidenceLessThanZero(), kMaxConfidence);
  EXPECT_EQ(partial_lteqz.GetConfidenceGreaterThanZero(), kMinConfidence);

  // >=0 -> >0
  LatticeElementConfidence partial_gteqz(kMaxConfidence / 4, kMinConfidence,
                                         kMaxConfidence / 2);
  partial_gteqz = ConfidenceLattice::RemoveLowestNonMin(partial_gteqz);
  EXPECT_EQ(partial_gteqz.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_gteqz.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(partial_gteqz.GetConfidenceGreaterThanZero(), kMaxConfidence / 2);

  // ==0 -> ==0 Since only value set
  LatticeElementConfidence test_max_zero =
      ConfidenceLattice::RemoveLowestNonMin(max_zero_lattice_confidence);
  EXPECT_EQ(test_max_zero.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(test_max_zero.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(test_max_zero.GetConfidenceGreaterThanZero(), kMinConfidence);

  LatticeElementConfidence test_non_max_zero =
      ConfidenceLattice::RemoveLowestNonMin(non_max_zero_lattice_confidence);
  EXPECT_EQ(test_non_max_zero.GetConfidenceZero(), kMaxConfidence / 2);
  EXPECT_EQ(test_non_max_zero.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(test_non_max_zero.GetConfidenceGreaterThanZero(), kMinConfidence);
}

// Tests removing lowest confidence value from LatticeElementConfidence. In this
// case removing less-than confidence values.
TEST(ConfidenceLattice, RemoveLowestNonMinLessThanZero) {
  // TOP -> >=0
  LatticeElementConfidence partial_top(kMaxConfidence, kMaxConfidence / 4,
                                       kMaxConfidence / 2);
  partial_top = ConfidenceLattice::RemoveLowestNonMin(partial_top);
  EXPECT_EQ(partial_top.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(partial_top.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(partial_top.GetConfidenceGreaterThanZero(), kMaxConfidence / 2);

  // <=0 -> ==0
  LatticeElementConfidence partial_lteqz(kMaxConfidence, kMaxConfidence / 4,
                                         kMinConfidence);
  partial_lteqz = ConfidenceLattice::RemoveLowestNonMin(partial_lteqz);
  EXPECT_EQ(partial_lteqz.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(partial_lteqz.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(partial_lteqz.GetConfidenceGreaterThanZero(), kMinConfidence);

  // !=0 -> >0
  LatticeElementConfidence partial_nz(kMinConfidence, kMaxConfidence / 2,
                                      kMaxConfidence / 4);
  partial_nz = ConfidenceLattice::RemoveLowestNonMin(partial_nz);
  EXPECT_EQ(partial_nz.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_nz.GetConfidenceLessThanZero(), kMaxConfidence / 2);
  EXPECT_EQ(partial_nz.GetConfidenceGreaterThanZero(), kMinConfidence);

  // <0 -> <0 Since only value set
  LatticeElementConfidence test_max_less_than_zero =
      ConfidenceLattice::RemoveLowestNonMin(
          max_less_than_zero_lattice_confidence);
  EXPECT_EQ(test_max_less_than_zero.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(test_max_less_than_zero.GetConfidenceLessThanZero(),
            kMaxConfidence);
  EXPECT_EQ(test_max_less_than_zero.GetConfidenceGreaterThanZero(),
            kMinConfidence);

  LatticeElementConfidence test_non_max_less_than_zero =
      ConfidenceLattice::RemoveLowestNonMin(
          non_max_less_than_zero_lattice_confidence);
  EXPECT_EQ(test_non_max_less_than_zero.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(test_non_max_less_than_zero.GetConfidenceLessThanZero(),
            kMaxConfidence / 2);
  EXPECT_EQ(test_non_max_less_than_zero.GetConfidenceGreaterThanZero(),
            kMinConfidence);
}

// Tests removing lowest confidence value from LatticeElementConfidence. In this
// case removing greater-than zero confidence values.
TEST(ConfidenceLattice, RemoveLowestNonMinGreaterThanZero) {
  // TOP -> <=0
  LatticeElementConfidence partial_top(kMaxConfidence, kMaxConfidence / 2,
                                       kMaxConfidence / 4);
  partial_top = ConfidenceLattice::RemoveLowestNonMin(partial_top);
  EXPECT_EQ(partial_top.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(partial_top.GetConfidenceLessThanZero(), kMaxConfidence / 2);
  EXPECT_EQ(partial_top.GetConfidenceGreaterThanZero(), kMinConfidence);

  // >=0 -> ==0
  LatticeElementConfidence partial_gteqz(kMaxConfidence, kMinConfidence,
                                         kMaxConfidence / 2);
  partial_gteqz = ConfidenceLattice::RemoveLowestNonMin(partial_gteqz);
  EXPECT_EQ(partial_gteqz.GetConfidenceZero(), kMaxConfidence);
  EXPECT_EQ(partial_gteqz.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(partial_gteqz.GetConfidenceGreaterThanZero(), kMinConfidence);

  // !=0 -> <0
  LatticeElementConfidence partial_nz(kMinConfidence, kMaxConfidence / 2,
                                      kMaxConfidence / 4);
  partial_nz = ConfidenceLattice::RemoveLowestNonMin(partial_nz);
  EXPECT_EQ(partial_nz.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_nz.GetConfidenceLessThanZero(), kMaxConfidence / 2);
  EXPECT_EQ(partial_nz.GetConfidenceGreaterThanZero(), kMinConfidence);

  // >0 -> >0 Since only value set
  LatticeElementConfidence test_max_greater_than_zero =
      ConfidenceLattice::RemoveLowestNonMin(
          max_greater_than_zero_lattice_confidence);
  EXPECT_EQ(test_max_greater_than_zero.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(test_max_greater_than_zero.GetConfidenceLessThanZero(),
            kMinConfidence);
  EXPECT_EQ(test_max_greater_than_zero.GetConfidenceGreaterThanZero(),
            kMaxConfidence);

  LatticeElementConfidence test_non_max_greater_than_zero =
      ConfidenceLattice::RemoveLowestNonMin(
          non_max_greater_than_zero_lattice_confidence);
  EXPECT_EQ(test_non_max_greater_than_zero.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(test_non_max_greater_than_zero.GetConfidenceLessThanZero(),
            kMinConfidence);
  EXPECT_EQ(test_non_max_greater_than_zero.GetConfidenceGreaterThanZero(),
            kMaxConfidence / 2);
}

// Tests that two lowest confidence values that are the same are both removed
TEST(ConfidenceLattice, RemoveLowestNonMinTwoSame) {
  // TOP -> ==0
  LatticeElementConfidence partial_top_zero(
      kMaxConfidence / 2, kMaxConfidence / 4, kMaxConfidence / 4);
  partial_top_zero = ConfidenceLattice::RemoveLowestNonMin(partial_top_zero);
  EXPECT_EQ(partial_top_zero.GetConfidenceZero(), kMaxConfidence / 2);
  EXPECT_EQ(partial_top_zero.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(partial_top_zero.GetConfidenceGreaterThanZero(), kMinConfidence);

  // TOP -> <0
  LatticeElementConfidence partial_top_ltz(
      kMaxConfidence / 4, kMaxConfidence / 2, kMaxConfidence / 4);
  partial_top_ltz = ConfidenceLattice::RemoveLowestNonMin(partial_top_ltz);
  EXPECT_EQ(partial_top_ltz.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_top_ltz.GetConfidenceLessThanZero(), kMaxConfidence / 2);
  EXPECT_EQ(partial_top_ltz.GetConfidenceGreaterThanZero(), kMinConfidence);

  // TOP -> >0
  LatticeElementConfidence partial_top_gtz(
      kMaxConfidence / 4, kMaxConfidence / 4, kMaxConfidence / 2);
  partial_top_gtz = ConfidenceLattice::RemoveLowestNonMin(partial_top_gtz);
  EXPECT_EQ(partial_top_gtz.GetConfidenceZero(), kMinConfidence);
  EXPECT_EQ(partial_top_gtz.GetConfidenceLessThanZero(), kMinConfidence);
  EXPECT_EQ(partial_top_gtz.GetConfidenceGreaterThanZero(), kMaxConfidence / 2);
}

}  // namespace error_specifications
