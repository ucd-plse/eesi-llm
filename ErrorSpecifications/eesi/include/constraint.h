// Implements the extended sign lattice. See eesi.proto for the elements.
//
// Uses the bit-vector trick of encoding lattice elements from:
//   H. Aït-Kaci, R. Boyer, P. Lincoln, R. Nasr. Efficient implementation of
//   lattice operations. In ACM Transactions on Programming Languages and
//   Systems (TOPLAS), Volume 11, Issue 1, Jan. 1989, pages 115-146.
//
// Also see the SPARTA implementation of encoding finite abstract domains.
//
// Here the reflexive/transitive closure of the lattice is calculated by
// and then hard-coded in `constraint.cc`.

#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_CONSTRAINT_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_CONSTRAINT_H_

#include <bitset>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

// Bit vector large enough to represent our nine-element lattice.
using LatticeEncoding = std::bitset<8>;

// This class is the actual sign lattice.
class SignLattice {
 public:
  // Perform a meet between two lattice elements.
  static SignLatticeElement Meet(const SignLatticeElement &x,
                                 const SignLatticeElement &y);

  // Perform a join between two lattice elements.
  static SignLatticeElement Join(const SignLatticeElement &x,
                                 const SignLatticeElement &y);

  // Returns the difference between two lattice elements
  // (i.e. \alpha( \gamma(x) - \gamma(y) )).
  static SignLatticeElement Difference(const SignLatticeElement &x,
                                       const SignLatticeElement &y);

  // Returns true if the given element is bottom.
  static bool IsBottom(const SignLatticeElement &x);

  static SignLatticeElement Complement(const SignLatticeElement &x);

  // Returns true if the meet of the lattice elements is NOT bottom.
  static bool Intersects(const SignLatticeElement &x,
                         const SignLatticeElement &y);

  static bool IsLessThan(const SignLatticeElement &x,
                         const SignLatticeElement &y);

  // For pretty-printing.
  static const std::unordered_map<std::string, SignLatticeElement>
      string_to_lattice_element;
  static const std::map<SignLatticeElement, std::string>
      lattice_element_to_string;

 private:
  // Encoding of the reflexive/transitive closure of
  // immediately greater than relation.
  // A meet operation is bitwise AND of the rows.
  //
  //       bot   <0   >0    0  <=0  >=0  !=0  top
  //  bot    1    0    0    0    0    0    0    0
  //   <0    1    1    0    0    0    0    0    0
  //   >0    1    0    1    0    0    0    0    0
  //    0    1    0    0    1    0    0    0    0
  //  <=0    1    1    0    1    1    0    0    0
  //  >=0    1    0    1    1    0    1    0    0
  //  !=0    1    1    1    0    0    0    1    0
  //  top    1    1    1    1    1    1    1    1
  static const LatticeEncoding meet_bottom;
  static const LatticeEncoding meet_less_than_zero;
  static const LatticeEncoding meet_greater_than_zero;
  static const LatticeEncoding meet_zero;
  static const LatticeEncoding meet_less_than_equal_zero;
  static const LatticeEncoding meet_greater_than_equal_zero;
  static const LatticeEncoding meet_not_zero;
  static const LatticeEncoding meet_top;

  // LatticeEncoding of the reflexive/transitive closure of
  // immediately less than relation.
  // A join operation is bitwise AND of the rows.
  //
  //       bot   <0   >0    0  <=0  >=0  !=0  top
  //  bot    1    1    1    1    1    1    1    1
  //   <0    0    1    0    0    1    0    1    1
  //   >0    0    0    1    0    0    1    1    1
  //    0    0    0    0    1    1    1    0    1
  //  <=0    0    0    0    0    1    0    0    1
  //  >=0    0    0    0    0    0    1    0    1
  //  !=0    0    0    0    0    0    0    1    1
  //  top    0    0    0    0    0    0    0    1
  static const LatticeEncoding join_bottom;
  static const LatticeEncoding join_less_than_zero;
  static const LatticeEncoding join_greater_than_zero;
  static const LatticeEncoding join_zero;
  static const LatticeEncoding join_less_than_equal_zero;
  static const LatticeEncoding join_greater_than_equal_zero;
  static const LatticeEncoding join_not_zero;
  static const LatticeEncoding join_top;

  // Maps from lattice elements to their bitset encoding.
  static const std::map<SignLatticeElement, LatticeEncoding>
      meet_lattice_element_encoding;
  static const std::map<SignLatticeElement, LatticeEncoding>
      join_lattice_element_encoding;

  // Maps from the bitset encoding of lattice elements to the actual elements.
  static const std::unordered_map<LatticeEncoding, SignLatticeElement>
      meet_lattice_element_decoding;
  static const std::unordered_map<LatticeEncoding, SignLatticeElement>
      join_lattice_element_decoding;

  // For computing the complement of an element.
  static const std::map<SignLatticeElement, SignLatticeElement> complement;

  // Map from lattice element to its offset in encoding bit vectors.
  static const std::map<SignLatticeElement, int> offset;
};

// Wraps a lattice element with other data
// - The function name of the return value being constrained.
// - Source location of branch that generated this constraint.
class Constraint {
 public:
  Constraint() {}
  explicit Constraint(const std::string &fname) : fname(fname) {}

  Constraint(const std::string &fname, const std::string &value)
      : fname(fname) {
    std::unordered_map<std::string, SignLatticeElement>::const_iterator it =
        SignLattice::string_to_lattice_element.find(value);
    assert(it != SignLattice::string_to_lattice_element.end());
    lattice_element = it->second;
  }

  Constraint(const std::string &fname,
             const SignLatticeElement &lattice_element)
      : fname(fname), lattice_element(lattice_element) {}

  // The name of the function to be constrained.
  std::string fname;

  // The lattice value.
  SignLatticeElement lattice_element =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;

  // Debug info if we have it (not guaranteed) of icmp that generated this
  // constraint.
  std::string file;
  unsigned line = 0;

  bool operator==(const Constraint &other) const {
    return (fname == other.fname && lattice_element == other.lattice_element &&
            file == other.file && line == other.line);
  }
  bool operator!=(const Constraint &other) const { return !(*this == other); }

  // Meet two constraints (function names must match).
  Constraint Meet(const Constraint &other);

  // Join two constraints (function names must match).
  Constraint Join(const Constraint &other);

  bool Intersects(const Constraint &other) const;

  bool Intersects(const SignLatticeElement &other_lattice_element) const;

  // Returns true if the lattice element for this constraint is bottom.
  bool IsBottom() const { return SignLattice::IsBottom(lattice_element); }
};

inline std::ostream &operator<<(std::ostream &os,
                                const SignLatticeElement &lattice_element) {
  os << SignLattice::lattice_element_to_string.at(lattice_element);
  return os;
}

inline std::ostream &operator<<(std::ostream &os,
                                const Constraint &constraint) {
  os << constraint.fname << " " << constraint.lattice_element;
  return os;
}

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_CONSTRAINT_H_
