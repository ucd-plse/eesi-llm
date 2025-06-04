#include "confidence_lattice.h"

#include <gflags/gflags.h>

#include <algorithm>
#include <numeric>
#include <vector>

namespace error_specifications {

LatticeElementConfidence ConfidenceLattice::JoinOnVector(
    const std::vector<LatticeElementConfidence> &lattice_element_confidences) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  auto confidence_emptyset = kMinConfidence;
  LatticeElementConfidence join_result = lattice_element_confidences.front();
  for (auto lattice_element_confidence : lattice_element_confidences) {
    join_result = Join(join_result, lattice_element_confidence);
  }

  return join_result;
}

LatticeElementConfidence ConfidenceLattice::MeetOnVector(
    const std::vector<LatticeElementConfidence> &lattice_element_confidences) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  auto confidence_emptyset = kMinConfidence;
  LatticeElementConfidence meet_result = lattice_element_confidences.front();
  for (auto lattice_element_confidence : lattice_element_confidences) {
    meet_result = Meet(meet_result, lattice_element_confidence);
  }

  return meet_result;
}

LatticeElementConfidence ConfidenceLattice::KeepHighest(
    const std::vector<LatticeElementConfidence> &lattice_element_confidences) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  auto confidence_emptyset = kMinConfidence;
  auto max_confidence = kMinConfidence;
  LatticeElementConfidence join_result;
  for (auto lattice_element_confidence : lattice_element_confidences) {
    if (confidence_zero < lattice_element_confidence.GetConfidenceZero()) {
      confidence_zero = lattice_element_confidence.GetConfidenceZero();
    }
    if (confidence_less_than_zero <
        lattice_element_confidence.GetConfidenceLessThanZero()) {
      confidence_less_than_zero =
          lattice_element_confidence.GetConfidenceLessThanZero();
    }
    if (confidence_greater_than_zero <
        lattice_element_confidence.GetConfidenceGreaterThanZero()) {
      confidence_greater_than_zero =
          lattice_element_confidence.GetConfidenceGreaterThanZero();
    }
    if (confidence_emptyset <
        lattice_element_confidence.GetConfidenceEmptyset()) {
      confidence_emptyset = lattice_element_confidence.GetConfidenceEmptyset();
    }
    if (max_confidence < GetMaxWithEmptyset(lattice_element_confidence)) {
      max_confidence = GetMaxWithEmptyset(lattice_element_confidence);
    }
  }

  if (confidence_zero != max_confidence) {
    confidence_zero = kMinConfidence;
  }
  if (confidence_less_than_zero != max_confidence) {
    confidence_less_than_zero = kMinConfidence;
  }
  if (confidence_greater_than_zero != max_confidence) {
    confidence_greater_than_zero = kMinConfidence;
  }
  if (confidence_emptyset != max_confidence) {
    confidence_emptyset = kMinConfidence;
  }

  return LatticeElementConfidence(confidence_zero, confidence_less_than_zero,
                                  confidence_greater_than_zero,
                                  confidence_emptyset);
}

LatticeElementConfidence ConfidenceLattice::Join(
    const LatticeElementConfidence &x, const LatticeElementConfidence &y) {
  return LatticeElementConfidence(
      std::max(x.GetConfidenceZero(), y.GetConfidenceZero()),
      std::max(x.GetConfidenceLessThanZero(), y.GetConfidenceLessThanZero()),
      std::max(x.GetConfidenceGreaterThanZero(),
               y.GetConfidenceGreaterThanZero()),
      std::min(x.GetConfidenceEmptyset(), y.GetConfidenceEmptyset()));
}

LatticeElementConfidence ConfidenceLattice::Meet(
    const LatticeElementConfidence &x, const LatticeElementConfidence &y) {
  return LatticeElementConfidence(
      std::min(x.GetConfidenceZero(), y.GetConfidenceZero()),
      std::min(x.GetConfidenceLessThanZero(), y.GetConfidenceLessThanZero()),
      std::min(x.GetConfidenceGreaterThanZero(),
               y.GetConfidenceGreaterThanZero()),
      std::max(x.GetConfidenceEmptyset(), y.GetConfidenceEmptyset()));
}

LatticeElementConfidence ConfidenceLattice::Intersection(
    const LatticeElementConfidence &x, const SignLatticeElement &y) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  auto confidence_emptyset = x.GetConfidenceEmptyset();
  if (SignLattice::Intersects(y,
                              SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO)) {
    confidence_zero = x.GetConfidenceZero();
  }
  if (SignLattice::Intersects(
          y, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)) {
    confidence_less_than_zero = x.GetConfidenceLessThanZero();
  }
  if (SignLattice::Intersects(
          y, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)) {
    confidence_greater_than_zero = x.GetConfidenceGreaterThanZero();
  }

  return LatticeElementConfidence(confidence_zero, confidence_less_than_zero,
                                  confidence_greater_than_zero,
                                  confidence_emptyset);
}

bool ConfidenceLattice::Intersects(const LatticeElementConfidence &x,
                                   const SignLatticeElement &y) {
  return !IsUnknown(Intersection(x, y));
}

// TODO(patrickjchap): This is can be simplified.
bool ConfidenceLattice::Equals(const LatticeElementConfidence &x,
                               const SignLatticeElement &y) {
  if ((x.GetConfidenceZero() == kMinConfidence &&
       SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO)) ||
      (x.GetConfidenceZero() > kMinConfidence &&
       !SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO))) {
    return false;
  }
  if ((x.GetConfidenceLessThanZero() == kMinConfidence &&
       SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)) ||
      (x.GetConfidenceLessThanZero() > kMinConfidence &&
       !SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO))) {
    return false;
  }
  if ((x.GetConfidenceGreaterThanZero() == kMinConfidence &&
       SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)) ||
      (x.GetConfidenceGreaterThanZero() > kMinConfidence &&
       !SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO))) {
    return false;
  }
  return true;
}

// TODO(patrickjchap): This is can be simplified.
bool ConfidenceLattice::MaxEquals(const LatticeElementConfidence &x,
                                  const SignLatticeElement &y) {
  if ((x.GetConfidenceZero() < kMaxConfidence &&
       SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO)) ||
      (x.GetConfidenceZero() == kMaxConfidence &&
       !SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO))) {
    return false;
  }
  if ((x.GetConfidenceLessThanZero() < kMaxConfidence &&
       SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)) ||
      (x.GetConfidenceLessThanZero() == kMaxConfidence &&
       !SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO))) {
    return false;
  }
  if ((x.GetConfidenceGreaterThanZero() < kMaxConfidence &&
       SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)) ||
      (x.GetConfidenceGreaterThanZero() == kMaxConfidence &&
       !SignLattice::Intersects(
           y, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO))) {
    return false;
  }
  return true;
}

LatticeElementConfidence ConfidenceLattice::KeepIfMax(
    const LatticeElementConfidence &x) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  if (x.GetConfidenceZero() == kMaxConfidence) {
    confidence_zero = kMaxConfidence;
  }
  if (x.GetConfidenceLessThanZero() == kMaxConfidence) {
    confidence_less_than_zero = kMaxConfidence;
  }
  if (x.GetConfidenceGreaterThanZero() == kMaxConfidence) {
    confidence_greater_than_zero = kMaxConfidence;
  }
  return LatticeElementConfidence(confidence_zero, confidence_less_than_zero,
                                  confidence_greater_than_zero,
                                  x.GetConfidenceEmptyset());
}

LatticeElementConfidence ConfidenceLattice::RemoveLowestNonMin(
    const LatticeElementConfidence &x) {
  auto confidence_zero = x.GetConfidenceZero();
  auto confidence_less_than_zero = x.GetConfidenceLessThanZero();
  auto confidence_greater_than_zero = x.GetConfidenceGreaterThanZero();
  auto minimum = kMaxConfidence;

  if ((confidence_zero > kMinConfidence &&
       confidence_less_than_zero == kMinConfidence &&
       confidence_greater_than_zero == kMinConfidence) ||
      (confidence_less_than_zero > kMinConfidence &&
       confidence_zero == kMinConfidence &&
       confidence_greater_than_zero == kMinConfidence) ||
      (confidence_greater_than_zero > kMinConfidence &&
       confidence_less_than_zero == kMinConfidence &&
       confidence_zero == kMinConfidence)) {
    return x;
  }

  // Determine what the non-kMinConfidence minimum confidence value is.
  if (confidence_zero != kMinConfidence && confidence_zero < minimum) {
    minimum = confidence_zero;
  }
  if (confidence_less_than_zero != kMinConfidence &&
      confidence_less_than_zero < minimum) {
    minimum = confidence_less_than_zero;
  }
  if (confidence_greater_than_zero != kMinConfidence &&
      confidence_greater_than_zero < minimum) {
    minimum = confidence_greater_than_zero;
  }

  // Check what confidence value is the minimum and check that multiple other
  // values are not the minimum. If all other non-kMinConfidence are the
  // minimum, just return x.
  // TODO(patrickjchap): The logic for this can obviously be reduced. I think
  // some of these actually might not even be reachable technically, the logic
  // is just laid out for now.
  int cpy_confidence_zero = confidence_zero;
  int cpy_confidence_less_than_zero = confidence_less_than_zero;
  int cpy_confidence_greater_than_zero = confidence_greater_than_zero;
  if (cpy_confidence_zero == minimum) {
    if (cpy_confidence_less_than_zero == minimum) {
      if (cpy_confidence_greater_than_zero == kMinConfidence ||
          cpy_confidence_greater_than_zero == minimum) {
        return x;
      }
    }
    if (cpy_confidence_greater_than_zero == minimum) {
      if (cpy_confidence_less_than_zero == kMinConfidence ||
          cpy_confidence_less_than_zero == minimum) {
        return x;
      }
    }
    confidence_zero = kMinConfidence;
  }
  if (cpy_confidence_less_than_zero == minimum) {
    if (cpy_confidence_zero == minimum) {
      if (cpy_confidence_greater_than_zero == kMinConfidence ||
          cpy_confidence_greater_than_zero == minimum) {
        return x;
      }
    }
    if (cpy_confidence_greater_than_zero == minimum) {
      if (cpy_confidence_zero == kMinConfidence ||
          cpy_confidence_zero == minimum) {
        return x;
      }
    }
    confidence_less_than_zero = kMinConfidence;
  }
  if (cpy_confidence_greater_than_zero == minimum) {
    if (cpy_confidence_less_than_zero == minimum) {
      if (cpy_confidence_zero == kMinConfidence ||
          cpy_confidence_zero == minimum) {
        return x;
      }
    }
    if (cpy_confidence_zero == minimum) {
      if (cpy_confidence_less_than_zero == kMinConfidence ||
          cpy_confidence_less_than_zero == minimum) {
        return x;
      }
    }
    confidence_greater_than_zero = kMinConfidence;
  }

  return LatticeElementConfidence(confidence_zero, confidence_less_than_zero,
                                  confidence_greater_than_zero,
                                  x.GetConfidenceEmptyset());
}

LatticeElementConfidence
ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
    const SignLatticeElement &x, const short confidence_emptyset) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  if (SignLattice::Intersects(x,
                              SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO)) {
    confidence_zero = kMaxConfidence;
  }
  if (SignLattice::Intersects(
          x, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)) {
    confidence_less_than_zero = kMaxConfidence;
  }
  if (SignLattice::Intersects(
          x, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)) {
    confidence_greater_than_zero = kMaxConfidence;
  }

  LatticeElementConfidence lattice_confidence(
      confidence_zero, confidence_less_than_zero, confidence_greater_than_zero,
      confidence_emptyset);
  return lattice_confidence;
}

LatticeElementConfidence
ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
    const SignLatticeElement &x, const short confidence_emptyset,
    const float &confidence_ratio) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  if (SignLattice::Intersects(x,
                              SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO)) {
    confidence_zero = kMaxConfidence * confidence_ratio;
  }
  if (SignLattice::Intersects(
          x, SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)) {
    confidence_less_than_zero = kMaxConfidence * confidence_ratio;
  }
  if (SignLattice::Intersects(
          x, SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)) {
    confidence_greater_than_zero = kMaxConfidence * confidence_ratio;
  }

  LatticeElementConfidence lattice_confidence(
      confidence_zero, confidence_less_than_zero, confidence_greater_than_zero,
      confidence_emptyset * confidence_ratio);
  return lattice_confidence;
}

LatticeElementConfidence
ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
    const SignLatticeElement &x) {
  return ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
      x, kMinConfidence);
}

SignLatticeElement
ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
    const LatticeElementConfidence &x) {
  SignLatticeElement zero_result =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  if (x.GetConfidenceZero() > kMinConfidence) {
    zero_result = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  }
  SignLatticeElement less_than_zero_result =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  if (x.GetConfidenceLessThanZero() > kMinConfidence) {
    less_than_zero_result =
        SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  }
  SignLatticeElement greater_than_zero_result =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  if (x.GetConfidenceGreaterThanZero() > kMinConfidence) {
    greater_than_zero_result =
        SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  }

  std::vector<SignLatticeElement> results{zero_result, less_than_zero_result,
                                          greater_than_zero_result};
  auto result = std::accumulate(results.begin(), results.end(),
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                SignLattice::Join);

  return result;
}

SignLatticeElement
ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
    const LatticeElementConfidence &x, int threshold) {
  SignLatticeElement zero_result =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  if (x.GetConfidenceZero() >= threshold) {
    zero_result = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  }
  SignLatticeElement less_than_zero_result =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  if (x.GetConfidenceLessThanZero() >= threshold) {
    less_than_zero_result =
        SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  }
  SignLatticeElement greater_than_zero_result =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  if (x.GetConfidenceGreaterThanZero() >= threshold) {
    greater_than_zero_result =
        SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  }

  std::vector<SignLatticeElement> results{zero_result, less_than_zero_result,
                                          greater_than_zero_result};
  auto result = std::accumulate(results.begin(), results.end(),
                                SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
                                SignLattice::Join);

  return result;
}

LatticeElementConfidence ConfidenceLattice::Difference(
    const LatticeElementConfidence &x, const LatticeElementConfidence &y) {
  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  if (x.GetConfidenceZero() > kMinConfidence &&
      y.GetConfidenceZero() == kMinConfidence) {
    confidence_zero = x.GetConfidenceZero();
  }
  if (x.GetConfidenceLessThanZero() > kMinConfidence &&
      y.GetConfidenceLessThanZero() == kMinConfidence) {
    confidence_less_than_zero = x.GetConfidenceLessThanZero();
  }
  if (x.GetConfidenceGreaterThanZero() > kMinConfidence &&
      y.GetConfidenceGreaterThanZero() == kMinConfidence) {
    confidence_greater_than_zero = x.GetConfidenceGreaterThanZero();
  }

  // The empty-set confidence should just propagate from x.
  LatticeElementConfidence diff_confidence(
      confidence_zero, confidence_less_than_zero, confidence_greater_than_zero,
      /*emptyset*/ x.GetConfidenceEmptyset());
  return diff_confidence;
}

LatticeElementConfidence ConfidenceLattice::Difference(
    const LatticeElementConfidence &x, const SignLatticeElement &y) {
  return ConfidenceLattice::Difference(
      x, SignLatticeElementToLatticeElementConfidence(y));
}
}  // namespace error_specifications
