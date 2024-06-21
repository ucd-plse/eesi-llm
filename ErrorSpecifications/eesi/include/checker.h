#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_CHECKER_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_CHECKER_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "constraint.h"
#include "llvm/IR/Instructions.h"
#include "proto/eesi.pb.h"

namespace error_specifications {

class Checker {
 public:
  Checker(){};

  // Checks for violations associated with the CallInst.
  void CheckViolations(const llvm::CallInst &call_inst,
                       const SignLatticeElement &specification_lattice_element,
                       const std::set<SignLatticeElement> &callee_constraints);

  // Returns a vector of violations.
  std::vector<Violation> GetViolations();

 private:
  // Checks for any unused violations and adds these violations to our
  // violations_ map.
  void CheckUnusedViolations(
      const llvm::CallInst &call_inst,
      const SignLatticeElement &specification_lattice_element);

  // Returns true if the function should be checked for violations.
  bool ShouldCheck(const Function &function,
                   const SignLatticeElement &specification_lattice_element);

  // Keeps track of all violations that have been found by the checker.
  std::vector<Violation> violations_;
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_CHECKER_H_
