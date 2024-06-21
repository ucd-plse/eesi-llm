// Implements the confidence powerset lattice for ErrorBlocksPass.

#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_CONFIDENCE_LATTICE_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_CONFIDENCE_LATTICE_H_

#include <iostream>

#include "constraint.h"
#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

// The maximum confidence that a lattice element can have.
constexpr short kMaxConfidence = 100;
// The minimum confidence that a lattice element can have.
constexpr short kMinConfidence = 0;

// Represents a lattice element through confidence values. The lattice
// element is represented by confidence values for ==0, <0, and >0. These
// confidence values range from kMinConfidence to kMaxConfidence. E.g., a bottom
// lattice element has confidence values of 0 (==0), 0 (<0), and 0 (>0).
// However, a <=0 lattice element can have confidence values such as 80 (==0),
// 100 (<0), and 0 (>0).
class LatticeElementConfidence {
 public:
  explicit LatticeElementConfidence(short confidence_zero,
                                    short confidence_less_than_zero,
                                    short confidence_greater_than_zero,
                                    short confidence_emptyset)
      : confidence_zero_(CheckConfidence(confidence_zero)),
        confidence_less_than_zero_(CheckConfidence(confidence_less_than_zero)),
        confidence_greater_than_zero_(
            CheckConfidence(confidence_greater_than_zero)),
        confidence_emptyset_(CheckConfidence(confidence_emptyset)) {}

  LatticeElementConfidence()
      : LatticeElementConfidence(
            /* ==0 */ kMinConfidence, /* <0 */ kMinConfidence,
            /* >0 */ kMinConfidence, /* emptyset */ kMinConfidence) {}

  LatticeElementConfidence(short confidence_zero,
                           short confidence_less_than_zero,
                           short confidence_greater_than_zero)
      : LatticeElementConfidence(confidence_zero, confidence_less_than_zero,
                                 confidence_greater_than_zero,
                                 /* emptyset */ kMinConfidence) {}

  bool operator==(const LatticeElementConfidence &other) const {
    return confidence_zero_ == other.GetConfidenceZero() &&
           confidence_less_than_zero_ == other.GetConfidenceLessThanZero() &&
           confidence_greater_than_zero_ ==
               other.GetConfidenceGreaterThanZero() &&
           confidence_emptyset_ == other.GetConfidenceEmptyset();
  }

  bool operator!=(const LatticeElementConfidence &other) const {
    return !(*this == other);
  }

  // Confidence value getters.
  short GetConfidenceZero() const { return confidence_zero_; }
  short GetConfidenceLessThanZero() const { return confidence_less_than_zero_; }
  short GetConfidenceGreaterThanZero() const {
    return confidence_greater_than_zero_;
  }
  short GetConfidenceEmptyset() const { return confidence_emptyset_; }

 private:
  // The confidence, from kMinConfidence to kMaxConfidence that the lattice
  // element ==0 is correct.
  short confidence_zero_;
  // The confidence, from kMinConfidence to kMaxConfidence that the lattice
  // element <0 is correct.
  short confidence_less_than_zero_;
  // The confidence, from kMinConfidence to kMaxConfidence that the lattice
  // element >0 is correct.
  short confidence_greater_than_zero_;
  // The confidence, from kMinConfidence to kMaxConfidence that the lattice
  // element being represented is actually bottom/empty-set. This is different
  // than the lattice element of a specification being bottom by default. This
  // confidence can be thought of as the confidence that the function related
  // to the lattice element does not return any error indicating value.
  short confidence_emptyset_;

  // Asserts that the confidence is between kMinConfidence and kMaxConfidence
  // inclusive.
  short CheckConfidence(const short x) {
    assert(kMinConfidence <= x && x <= kMaxConfidence);
    return x;
  }
};

// This class represents the confidence/powerset lattice, that is
// ConfidenceLattice contains operations to calculate the confidence for
// ==0, <0, and >0 depending on the specified operation.
class ConfidenceLattice {
 public:
  // Perform a join between two LatticeElementConfidence, i.e., a component-wise
  // max for the confidence values representing ==0, <0, and >0. The confidence
  // for empty-set is calculated by using a min.
  static LatticeElementConfidence Join(const LatticeElementConfidence &x,
                                       const LatticeElementConfidence &y);
  static LatticeElementConfidence Meet(const LatticeElementConfidence &x,
                                       const LatticeElementConfidence &y);

  // Perform a join on every LatticeElementConfidence in the vector.
  static LatticeElementConfidence JoinOnVector(
      const std::vector<LatticeElementConfidence> &lattice_element_confidences);
  static LatticeElementConfidence MeetOnVector(
      const std::vector<LatticeElementConfidence> &lattice_element_confidences);

  // Performs an intersection on a LatticeElementConfidence and a
  // SignLatticeElement, returning a LatticeElementConfidence where confidence
  // values are set based on the confidence values of x. For example, the
  // intersection of (/*==0*/ 80, /*<0*/ 100, /*>0*/ 0, /*emptyset*/ 0) and
  // SIGN_LATTICE_ELEMENT_ZERO would be (/*==0*/ 80, /*<0*/ 0, /*>0*/ 0,
  // /*emptyset*/ 0).
  static LatticeElementConfidence Intersection(
      const LatticeElementConfidence &x, const SignLatticeElement &y);

  // Checks if the LatticeElementConfidence x is equivalent to the
  // SignLatticeElement y by checking if all non-emptyset confidence values
  // are greater than kMinConfidence and if so checks if this intersects with y.
  static bool Equals(const LatticeElementConfidence &x,
                     const SignLatticeElement &y);
  // Checks if the LatticeElementConfidence x is equivalent to the
  // SignLatticeElement y by checking if all non-emptyset confidence values
  // are equal to kMaxConfidence and if so checks if this intersects with y.
  static bool MaxEquals(const LatticeElementConfidence &x,
                        const SignLatticeElement &y);

  // Returns a LatticeElementConfidence for x that only keeps confidence values
  // that are equal to kMaxConfidence, setting all other confidence values to
  // kMinConfidence.
  static LatticeElementConfidence KeepIfMax(const LatticeElementConfidence &x);
  static LatticeElementConfidence KeepHighest(
      const std::vector<LatticeElementConfidence> &lattice_element_confidences);

  // Removes the lowest non-kMinConfidence confidence values and returns a new
  // LatticeElementConfidence representing this. If there are multiple values
  // that equal the minimum, then those will all be removed unless the result
  // will set every confidence to kMinConfidence.
  static LatticeElementConfidence RemoveLowestNonMin(
      const LatticeElementConfidence &x);

  // Returns the LatticeElementConfidence difference between x and y. This
  // is NOT a straight substraction of confidence values. If a lattice
  // element confidence value of x is non-zero and the same lattice
  // element's confidence value in y is a different non-zero, then the final
  // difference result for that lattice element confidence is still 0. The
  // empty-set confidence value is just propagated from x.This is currently
  // only used to set the lattice element confidence of ==0 to
  // kMinConfidence when handling the SmartDropZero heuristic. In general,
  // this can be used to remove (set confidence to kMinConfidence) lattice
  // elements. E.g., if the confidence values are x = (100, 20, 0, 10) and y
  // = (90, 0, 0, 0), the difference is (0, 20, 0, 10).
  static LatticeElementConfidence Difference(const LatticeElementConfidence &x,
                                             const LatticeElementConfidence &y);

  // Calculates the difference, converts y to a LatticeElementConfidence
  // beforehand.
  static LatticeElementConfidence Difference(const LatticeElementConfidence &x,
                                             const SignLatticeElement &y);

  // Converts a SignLatticeElement to a LatticeElementConfidence. That is any
  // lattice element who intersects with one of the confidence lattice element
  // representatives (i.e., <0, >0, ==0) has a confidence of 100 by default.
  // This also takes in an explicit confidence_emptyset value that is used to
  // set the empty-set confidence for the LatticeElementConfidence.
  static LatticeElementConfidence SignLatticeElementToLatticeElementConfidence(
      const SignLatticeElement &x, const short confidence_emptyset);
  // Same conversion as the other SignLatticeElementToLatticeElementConfidence,
  // except that the confidence_emptyset on the return value is set to
  // kMinConfidence by default.
  static LatticeElementConfidence SignLatticeElementToLatticeElementConfidence(
      const SignLatticeElement &x);
  // Same conversion, but allows user to set a lower ratio.
  static LatticeElementConfidence SignLatticeElementToLatticeElementConfidence(
      const SignLatticeElement &x, const short confidence_emptyset,
      const float &confidence_ratio);

  // Converts a LatticeElementConfidence to a SignLatticeElement. That is any
  // lattice element whose confidence is above kMinConfidence should be used in
  // a SignLattice Join to calculate the final SignLatticeElement.
  static SignLatticeElement LatticeElementConfidenceToSignLatticeElement(
      const LatticeElementConfidence &x);
  static SignLatticeElement LatticeElementConfidenceToSignLatticeElement(
      const LatticeElementConfidence &x, int threshold);

  // Returns the max confidence value that is NOT emptyset.
  static short GetMax(const LatticeElementConfidence &x) {
    return std::max({x.GetConfidenceZero(), x.GetConfidenceLessThanZero(),
                     x.GetConfidenceGreaterThanZero()});
  }

  // Returns the max confidence value (possible emptyset!).
  static short GetMaxWithEmptyset(const LatticeElementConfidence &x) {
    return std::max({x.GetConfidenceZero(), x.GetConfidenceLessThanZero(),
                     x.GetConfidenceGreaterThanZero(),
                     x.GetConfidenceEmptyset()});
  }

  // Returns true if the emptyset confidence is greater-than kMinConfidence and
  // the confidences for ==0, <0, and >0 are all kMinConfidence.
  static bool IsEmptyset(const LatticeElementConfidence &x) {
    return x.GetConfidenceEmptyset() == kMaxConfidence;
  }

  // Returns true if the confidence for ==0, <0, >0, and emptyset are all
  // kMinConfidence.
  static bool IsUnknown(const LatticeElementConfidence &x) {
    return x.GetConfidenceZero() == kMinConfidence &&
           x.GetConfidenceLessThanZero() == kMinConfidence &&
           x.GetConfidenceGreaterThanZero() == kMinConfidence &&
           x.GetConfidenceEmptyset() == kMinConfidence;
  }
};

inline std::ostream &operator<<(
    std::ostream &os, const LatticeElementConfidence &lattice_confidence) {
  os << ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
            lattice_confidence)
     << " ( '==0' " << lattice_confidence.GetConfidenceZero() << ", '<0' "
     << lattice_confidence.GetConfidenceLessThanZero() << ", '>0' "
     << lattice_confidence.GetConfidenceGreaterThanZero() << ", emptyset "
     << lattice_confidence.GetConfidenceEmptyset() << ")";
  return os;
}

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_CONFIDENCE_LATTICE_H_
