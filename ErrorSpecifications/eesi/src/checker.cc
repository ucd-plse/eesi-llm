#include "checker.h"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "confidence_lattice.h"
#include "constraint.h"
#include "eesi_common.h"
#include "glog/logging.h"
#include "llvm.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "return_constraints_pass.h"
#include "return_propagation_pass.h"
#include "returned_values_pass.h"

namespace error_specifications {

void Checker::CheckViolations(
    const llvm::CallInst &call_inst,
    const SignLatticeElement &specification_lattice_element,
    const std::set<SignLatticeElement> &callee_constraints) {
  CheckUnusedViolations(call_inst, specification_lattice_element);
}

void Checker::CheckUnusedViolations(
    const llvm::CallInst &call_inst,
    const SignLatticeElement &specification_lattice_element) {
  const Function &spec_function = GetCallee(call_inst);
  // Simply return if specification should not be checked.
  if (!ShouldCheck(spec_function, specification_lattice_element)) return;

  // Unused call violation is simple as we can just utilize LLVM's use_empty()
  // for call instructions.
  if (call_inst.use_empty() &&
      ShouldCheck(spec_function, specification_lattice_element)) {
    Violation violation;
    violation.mutable_location()->CopyFrom(GetDebugLocation(call_inst));

    Specification specification;
    specification.set_lattice_element(specification_lattice_element);
    specification.mutable_function()->CopyFrom(spec_function);

    violation.mutable_specification()->CopyFrom(specification);
    violation.set_violation_type(
        ViolationType::VIOLATION_TYPE_UNUSED_RETURN_VALUE);
    violation.set_message("Unused return value.");

    const Function &parent_function =
        LlvmToProtoFunction(*call_inst.getParent()->getParent());
    violation.mutable_parent_function()->CopyFrom(parent_function);

    violations_.push_back(violation);
  }
}

bool Checker::ShouldCheck(
    const Function &function,
    const SignLatticeElement &specification_lattice_element) {
  assert(specification_lattice_element !=
         SignLatticeElement::SIGN_LATTICE_ELEMENT_INVALID);

  if (function.source_name().empty()) {
    return false;
  }

  if (function.return_type() == FunctionReturnType::FUNCTION_RETURN_TYPE_VOID) {
    return false;
  }

  if (specification_lattice_element ==
          SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM ||
      specification_lattice_element ==
          SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP) {
    return false;
  }

  return true;
}

std::vector<Violation> Checker::GetViolations() { return violations_; }

}  // namespace error_specifications
