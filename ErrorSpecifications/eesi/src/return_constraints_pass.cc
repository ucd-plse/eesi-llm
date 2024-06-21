#include "return_constraints_pass.h"

#include <string>

#include "eesi_common.h"
#include "llvm.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "return_propagation_pass.h"
#include "tbb/tbb.h"

namespace error_specifications {

bool ReturnConstraintsPass::runOnModule(llvm::Module &module) {
  std::vector<const llvm::Function *> module_functions;
  for (const llvm::Function &fn : module) {
    module_functions.push_back(&fn);
  }

  // Initialize program points to empty ReturnConstraintsFact.
  // Creates a new fact at every program point.
  tbb::parallel_for(
      tbb::blocked_range<std::vector<const llvm::Function *>::iterator>(
          module_functions.begin(), module_functions.end()),
      [&](auto thread_functions) {
        for (const auto *function : thread_functions) {
          for (const auto &basic_block : *function) {
            std::shared_ptr<ReturnConstraintsFact> prev =
                std::make_shared<ReturnConstraintsFact>();
            for (auto &inst : basic_block) {
              input_facts_[&inst] = prev;
              output_facts_[&inst] = std::make_shared<ReturnConstraintsFact>();
              prev = output_facts_[&inst];
            }
          }
        }
      });

  tbb::parallel_for(
      tbb::blocked_range<std::vector<const llvm::Function *>::iterator>(
          module_functions.begin(), module_functions.end()),
      [&](auto thread_functions) {
        for (const auto *function : thread_functions) {
          this->RunOnFunction(*function);
        }
      });

  return false;
}

void ReturnConstraintsPass::RunOnFunction(const llvm::Function &F) {
  std::string fname = F.getName().str();

  bool changed = true;
  while (changed) {
    changed = false;

    for (auto bi = F.begin(), be = F.end(); bi != be; ++bi) {
      const llvm::BasicBlock *BB = &*bi;
      const llvm::Instruction *succ_begin = &*(BB->begin());
      auto succ_fact = input_facts_.at(succ_begin);

      // Go over predecessor blocks and apply join
      for (auto pi = llvm::pred_begin(BB), pe = llvm::pred_end(BB); pi != pe;
           ++pi) {
        const llvm::Instruction *pred_term = (*pi)->getTerminator();
        auto pred_fact = output_facts_.at(pred_term);
        succ_fact->Join(*pred_fact);
      }

      changed = VisitBlock(*BB) || changed;
    }
  }
  return;
}

bool ReturnConstraintsPass::VisitBlock(const llvm::BasicBlock &BB) {
  bool changed = false;
  for (auto ii = BB.begin(), ie = BB.end(); ii != ie; ++ii) {
    const llvm::Instruction &I = *ii;

    std::shared_ptr<ReturnConstraintsFact> input_fact = input_facts_.at(&I);
    std::shared_ptr<ReturnConstraintsFact> output_fact = output_facts_.at(&I);

    ReturnConstraintsFact prev_fact = *output_fact;
    if (const llvm::CallInst *inst = llvm::dyn_cast<llvm::CallInst>(&I)) {
      VisitCallInst(*inst, input_fact, output_fact);
    } else if (const llvm::BranchInst *inst =
                   llvm::dyn_cast<llvm::BranchInst>(&I)) {
      VisitBranchInst(*inst, input_fact, output_fact);
    } else if (const llvm::SwitchInst *inst =
                   llvm::dyn_cast<llvm::SwitchInst>(&I)) {
      VisitSwitchInst(*inst, input_fact, output_fact);
    } else if (const llvm::PHINode *inst = llvm::dyn_cast<llvm::PHINode>(&I)) {
      VisitPHINode(*inst, input_fact, output_fact);
    } else {
      // Default is to just copy facts from previous instruction unchanged.
      output_fact->value = input_fact->value;
    }
    changed = changed || (*(output_fact) != prev_fact);
  }

  return changed;
}

void ReturnConstraintsPass::VisitCallInst(
    const llvm::CallInst &I, std::shared_ptr<const ReturnConstraintsFact> in,
    std::shared_ptr<ReturnConstraintsFact> out) {
  out->value = in->value;
  std::unordered_set<const llvm::Value *> gen_value({&I});

  std::string callee_name = GetCallee(I).source_name();

  Constraint c(callee_name);
  c.lattice_element = SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;

  out->value[callee_name] = c;
}

const std::map<SignLatticeElement, SignLatticeElement>
    ReturnConstraintsPass::unsigned_replacement({
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO},
        {SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
         SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO},
    });

const std::map<std::pair<llvm::ICmpInst::Predicate, SignLatticeElement>,
               std::pair<SignLatticeElement, SignLatticeElement>>
    ReturnConstraintsPass::predicate_complement({
        // (<, 0)  -->  (<0, >=0)
        {std::make_pair(llvm::ICmpInst::Predicate::ICMP_SLT,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO)},
        // (>, 0)  -->  (>0, <=0)
        {std::make_pair(llvm::ICmpInst::Predicate::ICMP_SGT,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO)},
        // (<=, 0)  -->  (<=0, >0)
        {std::make_pair(llvm::ICmpInst::Predicate::ICMP_SLE,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)},
        // (>=, 0)  -->  (>=0, <0)
        {std::make_pair(llvm::ICmpInst::Predicate::ICMP_SGE,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)},
        // (==, 0)  -->  (0, !=0)
        {std::make_pair(llvm::ICmpInst::Predicate::ICMP_EQ,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO),
         std::make_pair(SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO)},
        // (!=, 0)  -->  (!=0, 0)
        {std::make_pair(llvm::ICmpInst::Predicate::ICMP_NE,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO),
         std::make_pair(SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO)},
        // (<, +)  -->  (T, >)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SLT,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)},
        // (>, +)  -->  (>0, T)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SGT,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP)},
        // (<=, +)  -->  (T, >)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SLE,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)},
        // (>=, +)  -->  (>0, T)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SGE,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP)},
        // (==, +)  -->  (>0, T)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_EQ,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP)},
        // (!=, +)  -->  (T, >0)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_NE,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO)},
        // (<, -)  -->  (<, T)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SLT,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO),
         std::make_pair(SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP)},
        // (>, -)  -->  (T, <0)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SGT,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)},
        // (<=, -)  -->  (<0, T)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SLE,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO),
         std::make_pair(SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP)},
        // (>=, -)  -->  (T, <0)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_SGE,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)},
        // (==, -)  -->  (<0, T)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_EQ,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO),
         std::make_pair(SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
                        SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP)},
        // (!=, -)  -->  (T, <0)
        {std::make_pair(
             llvm::ICmpInst::Predicate::ICMP_NE,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO),
         std::make_pair(
             SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP,
             SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO)},
    });

std::pair<SignLatticeElement, SignLatticeElement>
ReturnConstraintsPass::AbstractICmp(const llvm::ICmpInst &I) {
  SignLatticeElement true_element =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  SignLatticeElement false_element =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;

  SignLatticeElement abstracted_operand =
      SignLatticeElement::SIGN_LATTICE_ELEMENT_INVALID;

  llvm::ICmpInst::Predicate predicate = I.getSignedPredicate();

  // Usually a constant int operand will be the second operand.
  llvm::ConstantInt *num = llvm::dyn_cast<llvm::ConstantInt>(I.getOperand(1));

  // If it is not the second operand, try the first.
  if (!num) {
    num = llvm::dyn_cast<llvm::ConstantInt>(I.getOperand(0));
    predicate = llvm::CmpInst::getSwappedPredicate(predicate);
  }

  // Comparisons with a constant integer.
  if (num) {
    if (num->isZero()) {
      abstracted_operand = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
    } else if (num->isNegative()) {
      abstracted_operand =
          SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
    } else {
      abstracted_operand =
          SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
    }
  }

  // Comparisons with a null pointer.
  if (llvm::isa<llvm::ConstantPointerNull>(I.getOperand(1))) {
    abstracted_operand = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  } else if (llvm::isa<llvm::ConstantPointerNull>(I.getOperand(0))) {
    abstracted_operand = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
    predicate = llvm::CmpInst::getSwappedPredicate(predicate);
  }

  // Neither a number or a null pointer. Return top / top.
  if (abstracted_operand == SignLatticeElement::SIGN_LATTICE_ELEMENT_INVALID) {
    return std::make_pair(true_element, false_element);
  }

  auto idx = std::make_pair(predicate, abstracted_operand);
  std::pair<SignLatticeElement, SignLatticeElement> result =
      predicate_complement.at(idx);

  if (I.isUnsigned()) {
    result.first = unsigned_replacement.at(result.first);
    result.second = unsigned_replacement.at(result.second);
  }

  return result;
}

void ReturnConstraintsPass::VisitSwitchInst(
    const llvm::SwitchInst &I, std::shared_ptr<const ReturnConstraintsFact> in,
    std::shared_ptr<ReturnConstraintsFact> out) {
  out->value = in->value;

  llvm::Value *condition = I.getCondition();
  if (!condition) return;

  // Go through the non-default cases. Everything else is handled similarily
  // to VisitBranchInst, except we do not deal with true/false successors,
  // only just the successor from the case.
  for (const auto &case_entry : I.cases()) {
    const llvm::ConstantInt *case_value = case_entry.getCaseValue();

    SignLatticeElement case_abstract_value;
    if (case_value) {
      if (case_value->isZero()) {
        case_abstract_value = SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
      } else if (case_value->isNegative()) {
        case_abstract_value =
            SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
      } else {
        case_abstract_value =
            SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
      }
    } else {
      return;
    }

    // Get the set of function whose values reach either the condition or the
    // case from return-propagation.
    ReturnPropagationPass *return_propagation =
        &getAnalysis<ReturnPropagationPass>();
    const llvm::Value *value_reaching_case;
    if (return_propagation->output_facts_.find(case_value) !=
        return_propagation->output_facts_.end()) {
      value_reaching_case = case_value;
    } else if (return_propagation->output_facts_.find(condition) !=
               return_propagation->output_facts_.end()) {
      value_reaching_case = condition;
    } else {
      return;
    }

    // The first element of this pair is the llvm value being tested
    // The second element is the set of functions which the key value may hold.
    auto fact = return_propagation->output_facts_.at(value_reaching_case);
    std::unordered_set<const llvm::Value *> test_ret_values;
    for (auto element : fact->value) {
      if (element.first == value_reaching_case) {
        test_ret_values = element.second;
      }
    }

    const llvm::BasicBlock *case_bb = case_entry.getCaseSuccessor();
    const llvm::Instruction *case_bb_first = GetFirstInstructionOfBB(case_bb);
    for (const llvm::Value *v : test_ret_values) {
      if (!llvm::isa<llvm::CallInst>(v)) continue;

      // Get the function name associated with v.
      const llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(v);
      std::string fname = GetCallee(*call).source_name();

      // Kill the constraints for functions being tested.
      // This prevents predecessor join from setting everything to top,
      // splitting constraint lattice_element at branches. This allows different
      // constraints to be associated with different successor blocks (different
      // edges).
      Constraint kill_constraint(fname);
      kill_constraint.lattice_element =
          SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
      out->value[fname] = kill_constraint;

      Constraint case_c(fname);
      case_c.lattice_element = case_abstract_value;

      // Insert fname's constraint into the temporary case fact. We use
      // the input fact because we killed fname's entry in the output fact.
      ReturnConstraintsFact case_fact;
      auto it = in->value.find(fname);
      if (it != in->value.end()) {
        // fname has a pre-existing constraint
        case_fact.value[fname] = case_c.Meet(it->second);
      } else {
        case_fact.value[fname] = case_c;
      }

      // We perform a join here to simulate predecessor join for fname.  The
      // original predecessor join in RunOnFunction won't work on fname because
      // we killed fname's entry in the out fact.
      auto existing_case_fact = input_facts_.at(case_bb_first);
      existing_case_fact->Join(case_fact);
    }
  }
}

void ReturnConstraintsPass::VisitBranchInst(
    const llvm::BranchInst &I, std::shared_ptr<const ReturnConstraintsFact> in,
    std::shared_ptr<ReturnConstraintsFact> out) {
  out->value = in->value;

  if (I.isUnconditional()) {
    return;
  }
  llvm::Value *condition = I.getOperand(0);
  assert(condition);

  llvm::ICmpInst *icmp = llvm::dyn_cast<llvm::ICmpInst>(condition);
  if (!icmp) {
    return;
  }

  ReturnPropagationPass *return_propagation =
      &getAnalysis<ReturnPropagationPass>();

  llvm::BasicBlock *true_bb = llvm::dyn_cast<llvm::BasicBlock>(I.getOperand(2));
  assert(true_bb);
  llvm::BasicBlock *false_bb =
      llvm::dyn_cast<llvm::BasicBlock>(I.getOperand(1));
  assert(false_bb);
  SignLatticeElement true_abstract_value = AbstractICmp(*icmp).first;
  SignLatticeElement false_abstract_value = AbstractICmp(*icmp).second;

  // Get the set of function whose values reach icmp operand from
  // return-propagation.
  llvm::Value *icmp_value = nullptr;
  if (return_propagation->output_facts_.find(icmp->getOperand(0)) !=
      return_propagation->output_facts_.end()) {
    icmp_value = icmp->getOperand(0);
  } else if (return_propagation->output_facts_.find(icmp->getOperand(1)) !=
             return_propagation->output_facts_.end()) {
    icmp_value = icmp->getOperand(1);
  } else {
    return;
  }

  auto fact = return_propagation->output_facts_.at(icmp_value);

  // The first element of this pair is the llvm value being tested
  // The second element is the set of functions which the key value may hold.
  std::unordered_set<const llvm::Value *> test_ret_values;
  for (auto element : fact->value) {
    if (element.first == icmp_value) {
      test_ret_values = element.second;
    }
  }

  for (const llvm::Value *v : test_ret_values) {
    ReturnConstraintsFact true_fact;
    ReturnConstraintsFact false_fact;

    if (!llvm::isa<llvm::CallInst>(v)) continue;

    // Get the function name associated with v.
    const llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(v);
    std::string fname = GetCallee(*call).source_name();

    // Kill the constraints for functions being tested.
    // This prevents predecessor join from setting everything to top,
    // splitting constraint lattice_element at branches. This allows different
    // constraints to be associated with different successor blocks (different
    // edges).
    Constraint kill_constraint(fname);
    kill_constraint.lattice_element =
        SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
    out->value[fname] = kill_constraint;

    Constraint true_c(fname);
    true_c.lattice_element = true_abstract_value;
    Constraint false_c(fname);
    false_c.lattice_element = false_abstract_value;

    // Insert fname's constraint into the temporary true/false facts.  We use
    // the input fact because we killed fname's entry in the output fact.
    auto it = in->value.find(fname);
    if (it != in->value.end()) {
      // fname has a pre-existing constraint
      true_fact.value[fname] = true_c.Meet(it->second);
      false_fact.value[fname] = false_c.Meet(it->second);
    } else {
      true_fact.value[fname] = true_c;
      false_fact.value[fname] = false_c;
    }

    // We perform a join here to simulate predecessor join for fname.  The
    // original predecessor join in RunOnFunction won't work on fname because we
    // killed fname's entry in the out fact.
    const llvm::Instruction *true_first = GetFirstInstructionOfBB(true_bb);
    auto existing_true_fact = input_facts_.at(true_first);
    existing_true_fact->Join(true_fact);

    const llvm::Instruction *false_first = GetFirstInstructionOfBB(false_bb);
    auto existing_false_fact = input_facts_.at(false_first);
    existing_false_fact->Join(false_fact);
  }
}

// If the PHI result can be returned, then add incoming values
// to the exit of each incoming basic block.
void ReturnConstraintsPass::VisitPHINode(
    const llvm::PHINode &I, std::shared_ptr<const ReturnConstraintsFact> in,
    std::shared_ptr<ReturnConstraintsFact> out) {
  out->value = in->value;
}

ReturnConstraintsFact ReturnConstraintsPass::GetInFact(
    const llvm::Value *v) const {
  return *(input_facts_.at(v));
}

ReturnConstraintsFact ReturnConstraintsPass::GetOutFact(
    const llvm::Value *v) const {
  return *(output_facts_.at(v));
}

std::set<SignLatticeElement> ReturnConstraintsPass::GetConstraints(
    llvm::Module &module, const std::string &parent_function,
    const Function &called_function) {
  std::set<SignLatticeElement> ret;

  for (auto &function : module) {
    if (function.getName() != parent_function) {
      continue;
    }

    for (auto &basic_block : function) {
      const llvm::Instruction *inst = GetFirstInstructionOfBB(&basic_block);
      auto return_constraints_fact = input_facts_.at(inst);
      for (const auto &kv : return_constraints_fact->value) {
        if (kv.first == called_function.source_name()) {
          ret.insert(kv.second.lattice_element);
        }
      }
    }
  }

  return ret;
}

void ReturnConstraintsPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.addRequired<ReturnPropagationPass>();
  au.setPreservesAll();
}

char ReturnConstraintsPass::ID = 0;
static llvm::RegisterPass<ReturnConstraintsPass> X(
    "return-constraints",
    "Function Return Value Constraints on each basic block", false, false);

}  // namespace error_specifications
