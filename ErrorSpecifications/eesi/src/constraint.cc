#include "constraint.h"

#include <algorithm>
#include <bitset>
#include <map>
#include <unordered_map>

namespace error_specifications {

const LatticeEncoding SignLattice::meet_bottom("10000000");
const LatticeEncoding SignLattice::meet_less_than_zero("11000000");
const LatticeEncoding SignLattice::meet_greater_than_zero("10100000");
const LatticeEncoding SignLattice::meet_zero("10010000");
const LatticeEncoding SignLattice::meet_less_than_equal_zero("11011000");
const LatticeEncoding SignLattice::meet_greater_than_equal_zero("10110100");
const LatticeEncoding SignLattice::meet_not_zero("11100010");
const LatticeEncoding SignLattice::meet_top("11111111");
const LatticeEncoding SignLattice::join_bottom("11111111");
const LatticeEncoding SignLattice::join_less_than_zero("01001011");
const LatticeEncoding SignLattice::join_greater_than_zero("00100111");
const LatticeEncoding SignLattice::join_zero("00011101");
const LatticeEncoding SignLattice::join_less_than_equal_zero("00001001");
const LatticeEncoding SignLattice::join_greater_than_equal_zero("00000101");
const LatticeEncoding SignLattice::join_not_zero("00000011");
const LatticeEncoding SignLattice::join_top("00000001");

const std::map<SignLatticeElement, LatticeEncoding>
    SignLattice::meet_lattice_element_encoding({
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
         SignLattice::meet_bottom},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
         SignLattice::meet_less_than_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
         SignLattice::meet_greater_than_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, SignLattice::meet_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
         SignLattice::meet_less_than_equal_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
         SignLattice::meet_greater_than_equal_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
         SignLattice::meet_not_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP, SignLattice::meet_top},
    });

const std::unordered_map<LatticeEncoding, SignLatticeElement>
    SignLattice::meet_lattice_element_decoding({
        {SignLattice::meet_bottom,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM},
        {SignLattice::meet_less_than_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO},
        {SignLattice::meet_greater_than_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO},
        {SignLattice::meet_zero, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
        {SignLattice::meet_less_than_equal_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
        {SignLattice::meet_greater_than_equal_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
        {SignLattice::meet_not_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
        {SignLattice::meet_top, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
    });

const std::map<SignLatticeElement, LatticeEncoding>
    SignLattice::join_lattice_element_encoding({
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
         SignLattice::join_bottom},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
         SignLattice::join_less_than_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
         SignLattice::join_greater_than_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, SignLattice::join_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
         SignLattice::join_less_than_equal_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
         SignLattice::join_greater_than_equal_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
         SignLattice::join_not_zero},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP, SignLattice::join_top},
    });

const std::unordered_map<LatticeEncoding, SignLatticeElement>
    SignLattice::join_lattice_element_decoding({
        {SignLattice::join_bottom,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM},
        {SignLattice::join_less_than_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO},
        {SignLattice::join_greater_than_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO},
        {SignLattice::join_zero, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
        {SignLattice::join_less_than_equal_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
        {SignLattice::join_greater_than_equal_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
        {SignLattice::join_not_zero,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
        {SignLattice::join_top, SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
    });

const std::map<SignLatticeElement, SignLatticeElement> SignLattice::complement({
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},

    {SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
     SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM},
});

const std::unordered_map<std::string, SignLatticeElement>
    SignLattice::string_to_lattice_element({
        {"bottom", SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM},
        {"<0", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO},
        {">0", SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO},
        {"0", SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
        {"<=0", SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO},
        {">=0",
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
        {"!=0", SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO},
        {"top", SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP},
    });

const std::map<SignLatticeElement, std::string>
    SignLattice::lattice_element_to_string({
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_INVALID, "INVALID"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM, "bottom"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, "<0"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, ">0"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, "0"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO, "<=0"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
         ">=0"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, "!=0"},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP, "top"},
    });

const std::map<SignLatticeElement, int> SignLattice::offset({
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM, 7},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO, 6},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO, 5},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO, 4},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO, 3},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO, 2},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO, 1},
    {SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP, 0},
});

bool SignLattice::IsLessThan(const SignLatticeElement &x,
                             const SignLatticeElement &y) {
  switch (x) {
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM:
      return join_bottom[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO:
      return join_less_than_zero[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO:
      return join_greater_than_zero[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO:
      return join_less_than_equal_zero[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO:
      return join_greater_than_equal_zero[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO:
      return join_zero[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO:
      return join_not_zero[offset.at(y)];
      break;
    case SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP:
      return join_top[offset.at(y)];
      break;
    default:
      abort();
  }
}

SignLatticeElement SignLattice::Meet(const SignLatticeElement &x,
                                     const SignLatticeElement &y) {
  auto x_encoding = meet_lattice_element_encoding.at(x);
  auto y_encoding = meet_lattice_element_encoding.at(y);
  return meet_lattice_element_decoding.at(x_encoding &= y_encoding);
}

SignLatticeElement SignLattice::Join(const SignLatticeElement &x,
                                     const SignLatticeElement &y) {
  auto x_encoding = join_lattice_element_encoding.at(x);
  auto y_encoding = join_lattice_element_encoding.at(y);
  return join_lattice_element_decoding.at(x_encoding &= y_encoding);
}

SignLatticeElement SignLattice::Difference(const SignLatticeElement &x,
                                           const SignLatticeElement &y) {
  return SignLattice::Meet(x, SignLattice::Complement(y));
}

bool SignLattice::IsBottom(const SignLatticeElement &x) {
  return x == SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
}

SignLatticeElement SignLattice::Complement(const SignLatticeElement &x) {
  return complement.at(x);
}

bool SignLattice::Intersects(const SignLatticeElement &x,
                             const SignLatticeElement &y) {
  return !SignLattice::IsBottom(SignLattice::Meet(x, y));
}

Constraint Constraint::Meet(const Constraint &other) {
  assert(fname == other.fname);
  Constraint c;
  c.fname = fname;
  c.lattice_element = SignLattice::Meet(lattice_element, other.lattice_element);

  return c;
}

Constraint Constraint::Join(const Constraint &other) {
  assert(fname == other.fname);
  Constraint c;
  c.fname = fname;
  c.lattice_element = SignLattice::Join(lattice_element, other.lattice_element);
  return c;
}

bool Constraint::Intersects(const Constraint &other) const {
  assert(fname == other.fname);
  return SignLattice::Intersects(lattice_element, other.lattice_element);
}

bool Constraint::Intersects(
    const SignLatticeElement &other_lattice_element) const {
  return SignLattice::Intersects(lattice_element, other_lattice_element);
}

}  // namespace error_specifications
