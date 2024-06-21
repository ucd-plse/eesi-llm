#include "return_range_pass.h"

#include "call_graph_underapproximation.h"
#include "eesi_common.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "return_constraints_pass.h"
#include "returned_values_pass.h"

namespace error_specifications {

ReturnRangeFact::ReturnRangeFact(const ReturnRangeFact &other) {
  value = other.value;
}

ReturnRangeFact::ReturnRangeFact(const llvm::Value *v,
                                 SignLatticeElement constraint) {
  value[v] = constraint;
}

bool ReturnRangeFact::operator==(const ReturnRangeFact &other) const {
  return value == other.value;
}

bool ReturnRangeFact::operator!=(const ReturnRangeFact &other) const {
  return !(*this == other);
}

void ReturnRangeFact::Join(const ReturnRangeFact &other) {
  for (const auto &kv : other.value) {
    auto it = this->value.find(kv.first);

    if (it != this->value.end()) {
      it->second = SignLattice::Join(it->second, kv.second);
    } else {
      this->value[kv.first] = kv.second;
    }
  }
}

void ReturnRangeFact::Meet(const ReturnRangeFact &other) {
  for (const auto &kv : other.value) {
    auto it = this->value.find(kv.first);

    if (it != this->value.end()) {
      it->second = SignLattice::Meet(it->second, kv.second);
    } else {
      this->value[kv.first] = kv.second;
    }
  }
}

void ReturnRangeFact::FilteredJoin(const ReturnRangeFact &other,
                                   const ReturnedValuesFact &rvf) {
  for (const auto &kv : other.value) {
    if (rvf.Contains(kv.first)) {
      auto it = this->value.find(kv.first);

      if (it != this->value.end()) {
        it->second = SignLattice::Join(it->second, kv.second);
      } else {
        this->value[kv.first] = kv.second;
      }
    }
  }
}

void ReturnRangeFact::FilteredCopy(const ReturnRangeFact &other,
                                   const ReturnedValuesFact &rvf) {
  value.clear();
  FilteredJoin(other, rvf);
}

bool ReturnRangeFact::Contains(const llvm::Value *v) const {
  return value.find(v) != value.end();
}

bool ReturnRangePass::runOnModule(llvm::Module &module) {
  // Initialize program points to empty ReturnRangeFact.
  // Creates a new fact at every relevant program point.
  for (const llvm::Function &func : module) {
    if (!ShouldIgnore(&func)) {
      for (const llvm::BasicBlock &basic_block : func) {
        std::shared_ptr<ReturnRangeFact> prev =
            std::make_shared<ReturnRangeFact>();
        for (const llvm::Instruction &inst : basic_block) {
          input_facts_[&inst] = prev;
          output_facts_[&inst] = std::make_shared<ReturnRangeFact>();
          prev = output_facts_[&inst];
        }
      }
    }
  }

  llvm::CallGraph call_graph = CallGraphUnderapproximation(module);

  for (auto scc_it = llvm::scc_begin(&call_graph); !scc_it.isAtEnd();
       ++scc_it) {
    const bool has_loop = scc_it.hasLoop();
    bool changed;

    do {
      changed = false;
      for (auto node : *scc_it) {
        const llvm::Function *func = node->getFunction();

        if (!ShouldIgnore(func)) {
          auto orig_range = GetReturnRange(
              *func, SignLatticeElement::SIGN_LATTICE_ELEMENT_INVALID);

          RunOnFunction(*func);

          auto new_range = GetReturnRange(
              *func, SignLatticeElement::SIGN_LATTICE_ELEMENT_INVALID);
          changed = changed || orig_range != new_range;
        }
      }
    } while (has_loop && changed);
  }

  return false;
}

void ReturnRangePass::RunOnFunction(const llvm::Function &func) {
  bool changed;

  do {
    changed = false;

    for (const llvm::BasicBlock &BB : func) {
      const llvm::Instruction *bb_first = GetFirstInstructionOfBB(&BB);
      auto bb_first_fact = input_facts_.at(bb_first);

      const auto &returned_values_pass = getAnalysis<ReturnedValuesPass>();
      const auto bb_first_rvf = returned_values_pass.GetInFact(bb_first);

      // Predecessor join
      for (auto pi = llvm::pred_begin(&BB), pe = llvm::pred_end(&BB); pi != pe;
           ++pi) {
        const llvm::Instruction *pred_last = GetLastInstructionOfBB(*pi);
        auto &pred_last_fact = output_facts_.at(pred_last);
        bb_first_fact->FilteredJoin(*pred_last_fact, bb_first_rvf);
      }

      changed = VisitBlock(BB) || changed;
    }
  } while (changed);
}

SignLatticeElement ReturnRangePass::GetReturnRange(
    const llvm::Function &func) const {
  return return_ranges_.at(&func);
}

SignLatticeElement ReturnRangePass::GetReturnRange(
    const llvm::Function &func, const SignLatticeElement default_return) const {
  return return_ranges_.count(&func) > 0 ? return_ranges_.at(&func)
                                         : default_return;
}

const std::unordered_map<const llvm::Function *, SignLatticeElement>
    &ReturnRangePass::GetReturnRanges() const {
  return return_ranges_;
}

ReturnRangeFact ReturnRangePass::GetInFact(
    const llvm::Instruction *inst) const {
  return *input_facts_.at(inst);
}

ReturnRangeFact ReturnRangePass::GetOutFact(
    const llvm::Instruction *inst) const {
  return *output_facts_.at(inst);
}

void ReturnRangePass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.addRequired<ReturnedValuesPass>();
  au.setPreservesAll();
}

// Called for each basic block.
bool ReturnRangePass::VisitBlock(const llvm::BasicBlock &BB) {
  bool changed = false;

  for (const llvm::Instruction &inst : BB) {
    const auto &in_fact = input_facts_.at(&inst);
    const auto &out_fact = output_facts_.at(&inst);
    const auto orig_out_fact = *out_fact;

    const auto &returned_values_pass = getAnalysis<ReturnedValuesPass>();
    const auto out_rvf = returned_values_pass.GetOutFact(&inst);

    // Resolve final values
    if (const auto *store_inst = llvm::dyn_cast<llvm::StoreInst>(&inst)) {
      VisitStoreInst(*store_inst, *in_fact, *out_fact, out_rvf);
    } else if (llvm::isa<llvm::LoadInst>(&inst) ||
               llvm::isa<llvm::BitCastInst>(&inst) ||
               llvm::isa<llvm::PtrToIntInst>(&inst) ||
               llvm::isa<llvm::TruncInst>(&inst) ||
               llvm::isa<llvm::SExtInst>(&inst)) {
      VisitLoadLikeInst(inst, *in_fact, *out_fact, out_rvf);
    } else if (const auto *phi = llvm::dyn_cast<llvm::PHINode>(&inst)) {
      VisitPHINode(*phi, *in_fact, *out_fact, out_rvf);
    } else if (const auto *branch = llvm::dyn_cast<llvm::BranchInst>(&inst)) {
      VisitBranchInst(*branch, *in_fact, *out_fact, out_rvf);
    } else if (const auto *sw = llvm::dyn_cast<llvm::SwitchInst>(&inst)) {
      VisitSwitchInst(*sw, *in_fact, *out_fact, out_rvf);
    } else if (const auto *ret = llvm::dyn_cast<llvm::ReturnInst>(&inst)) {
      VisitReturnInst(*ret, *in_fact);
    } else {
      out_fact->FilteredCopy(*in_fact, out_rvf);
    }

    changed = changed || *out_fact != orig_out_fact;
  }

  return changed;
}

void ReturnRangePass::VisitStoreInst(const llvm::StoreInst &I,
                                     const ReturnRangeFact &in,
                                     ReturnRangeFact &out,
                                     const ReturnedValuesFact &out_rvf) {
  const llvm::Value *stored = I.getOperand(0);
  const llvm::Value *target = I.getOperand(1);

  out.FilteredCopy(in, out_rvf);

  if (!out_rvf.Contains(target)) {  // target isn't returnable
    return;
  }

  if (in.Contains(stored)) {
    out.value[target] = in.value.at(stored);
  } else {
    out.value[target] = AbstractValue(*stored);
  }
}

void ReturnRangePass::VisitLoadLikeInst(const llvm::Instruction &I,
                                        const ReturnRangeFact &in,
                                        ReturnRangeFact &out,
                                        const ReturnedValuesFact &out_rvf) {
  const llvm::Value *loaded = I.getOperand(0);

  out.FilteredCopy(in, out_rvf);

  if (!out_rvf.Contains(&I)) {  // result isn't returnable
    return;
  }

  if (in.Contains(loaded)) {
    out.value[&I] = in.value.at(loaded);
  } else {
    out.value[&I] = AbstractValue(*loaded);
  }
}

void ReturnRangePass::VisitPHINode(const llvm::PHINode &I,
                                   const ReturnRangeFact &in,
                                   ReturnRangeFact &out,
                                   const ReturnedValuesFact &out_rvf) {
  out.FilteredCopy(in, out_rvf);

  if (!out_rvf.Contains(&I)) {  // result isn't returnable
    return;
  }

  out.value.erase(&I);

  for (const llvm::Use &use : I.incoming_values()) {
    const llvm::Value *incoming_value = use.get();
    if (in.Contains(incoming_value)) {
      out.Join(ReturnRangeFact(&I, in.value.at(incoming_value)));
    } else {
      out.Join(ReturnRangeFact(&I, AbstractValue(*incoming_value)));
    }
  }
}

void ReturnRangePass::VisitBranchInst(const llvm::BranchInst &I,
                                      const ReturnRangeFact &in,
                                      ReturnRangeFact &out,
                                      const ReturnedValuesFact &out_rvf) {
  out.FilteredCopy(in, out_rvf);

  if (I.isUnconditional()) {
    return;
  }

  const auto *cond = llvm::dyn_cast<llvm::ICmpInst>(I.getOperand(0));

  if (!cond) {
    return;
  }

  const llvm::Value *checked_value;

  if (!(checked_value =
            GetCheckedReturnValue(*cond, cond->getOperand(0), out_rvf)) &&
      !(checked_value =
            GetCheckedReturnValue(*cond, cond->getOperand(1), out_rvf))) {
    return;
  }

  // Similar to ReturnConstraintsPass: A returned value is being checked, so we
  // pass on the resulting ranges to the appropriate successor blocks and kill
  // its entry in the current output fact.
  const auto &returned_values_pass = getAnalysis<ReturnedValuesPass>();

  const auto *false_bb = llvm::dyn_cast<llvm::BasicBlock>(I.getOperand(1));
  const auto *false_bb_first = GetFirstInstructionOfBB(false_bb);
  auto false_in_fact = input_facts_[false_bb_first];
  const auto false_rvf = returned_values_pass.GetInFact(false_bb_first);

  const auto *true_bb = llvm::dyn_cast<llvm::BasicBlock>(I.getOperand(2));
  const auto *true_bb_first = GetFirstInstructionOfBB(true_bb);
  auto true_in_fact = input_facts_[true_bb_first];
  const auto true_rvf = returned_values_pass.GetInFact(true_bb_first);

  const auto abstracted_icmp = ReturnConstraintsPass::AbstractICmp(*cond);

  out.value.erase(checked_value);

  if (true_rvf.Contains(checked_value)) {
    if (in.Contains(checked_value)) {
      true_in_fact->Join(ReturnRangeFact(
          checked_value, SignLattice::Meet(in.value.at(checked_value),
                                           abstracted_icmp.first)));
    } else {
      true_in_fact->Join(ReturnRangeFact(checked_value, abstracted_icmp.first));
    }
  }
  if (false_rvf.Contains(checked_value)) {
    if (in.Contains(checked_value)) {
      false_in_fact->Join(ReturnRangeFact(
          checked_value, SignLattice::Meet(in.value.at(checked_value),
                                           abstracted_icmp.second)));
    } else {
      false_in_fact->Join(
          ReturnRangeFact(checked_value, abstracted_icmp.second));
    }
  }
}

void ReturnRangePass::VisitSwitchInst(const llvm::SwitchInst &I,
                                      const ReturnRangeFact &in,
                                      ReturnRangeFact &out,
                                      const ReturnedValuesFact &out_rvf) {
  out.FilteredCopy(in, out_rvf);

  const llvm::Value *test_value = nullptr;

  if (!(test_value = GetCheckedReturnValue(I, I.getCondition(), out_rvf))) {
    return;
  }

  // Like ReturnConstraintsPass, we kill the entry in the out fact and pass on
  // the appropriate ranges to the successor blocks directly.
  const auto &returned_values_pass = getAnalysis<ReturnedValuesPass>();

  out.value.erase(test_value);

  // Go through the non-default cases
  for (const auto &case_entry : I.cases()) {
    const llvm::ConstantInt *case_value = case_entry.getCaseValue();
    const llvm::BasicBlock *case_bb = case_entry.getCaseSuccessor();
    const llvm::Instruction *case_bb_first = GetFirstInstructionOfBB(case_bb);
    const auto case_rvf = returned_values_pass.GetInFact(case_bb_first);
    auto &case_in_fact = input_facts_.at(case_bb_first);

    if (case_rvf.Contains(test_value)) {
      if (in.Contains(test_value)) {
        case_in_fact->Join(ReturnRangeFact(
            test_value, SignLattice::Meet(in.value.at(test_value),
                                          AbstractInteger(*case_value))));
      } else {
        case_in_fact->Join(
            ReturnRangeFact(test_value, AbstractInteger(*case_value)));
      }
    }
  }

  // default case
  const llvm::BasicBlock *default_bb = I.getDefaultDest();
  const llvm::Instruction *default_bb_first =
      GetFirstInstructionOfBB(default_bb);
  const auto default_rvf = returned_values_pass.GetInFact(default_bb_first);
  if (default_rvf.Contains(test_value) && in.Contains(test_value)) {
    auto &default_in_fact = input_facts_.at(default_bb_first);
    default_in_fact->Join(ReturnRangeFact(test_value, in.value.at(test_value)));
  }
}

void ReturnRangePass::VisitReturnInst(const llvm::ReturnInst &I,
                                      const ReturnRangeFact &in) {
  // Get the return range of this particular return instruction.  If the input
  // fact doesn't have the returned value, then we have a direct return.
  SignLatticeElement return_range = in.Contains(I.getReturnValue())
                                        ? in.value.at(I.getReturnValue())
                                        : AbstractValue(*I.getReturnValue());

  // Having either >0 or <0 without the other doesn't really make sense with
  // pointers, so we bump it up to !=0.  Not sure if it happens in practice
  // (e.g., a pointer function that only returns casted integer error
  // constants and null), but better to be safe.
  if (I.getFunction()->getReturnType()->isPointerTy() &&
      SignLattice::Intersects(
          return_range, SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO)) {
    return_range = SignLattice::Join(
        return_range, SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
  }

  // Join this instruction's return range with the overall function range.
  auto it = return_ranges_.find(I.getFunction());
  if (it != return_ranges_.end()) {
    it->second = SignLattice::Join(it->second, return_range);
  } else {
    return_ranges_[I.getFunction()] = return_range;
  }
}

const llvm::Value *ReturnRangePass::GetCheckedReturnValue(
    const llvm::Instruction &cmp_inst, const llvm::Value *cmp_val,
    const ReturnedValuesFact &rvf) const {
  // if (retval < 0) { ... } translates to:
  // %1 = load i32, i32* %retval
  // %2 = icmp slt i32 %1, 0
  if (const auto *load = llvm::dyn_cast<llvm::LoadInst>(cmp_val)) {
    if (rvf.Contains(load->getOperand(0))) {
      return load->getOperand(0);
    }
  }
  // if ((retval = something) < 0) translates to:
  // store i32 %something, i32* %retval
  // %2 = icmp slt i32 %something, 0
  else if (const auto *store = llvm::dyn_cast_or_null<llvm::StoreInst>(
               cmp_inst.getPrevNode())) {
    if (store->getOperand(0) == cmp_val && rvf.Contains(store->getOperand(1))) {
      return store->getOperand(1);
    }
  }

  // If it doesn't follow one of the above patterns, it's probably not a
  // returnable value.
  return nullptr;
}

SignLatticeElement ReturnRangePass::AbstractCall(
    const llvm::CallInst &call) const {
  const auto callee = GetCalleeFunction(call);

  if (!callee || callee->isDeclaration()) {
    // The callee is either unresolved or external, so we can't determine the
    // actual return range.  Just assume that it can return anything.
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  } else if (return_ranges_.count(callee) > 0) {
    return return_ranges_.at(callee);
  } else {
    // If the callee has a definition and we haven't seen it yet, we're most
    // likely in an SCC.
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  }
}

SignLatticeElement ReturnRangePass::AbstractValue(
    const llvm::Value &value) const {
  if (const auto *integer = llvm::dyn_cast<llvm::ConstantInt>(&value)) {
    if (integer->getBitWidth() == 1) {  // i1 boolean
      return integer->isOne()
                 ? SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO
                 : SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
    } else {
      return AbstractInteger(*integer);
    }
  } else if (llvm::isa<llvm::ConstantPointerNull>(&value)) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  } else if (ExtractStringLiteral(value)) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  } else if (const auto *call = llvm::dyn_cast<llvm::CallInst>(&value)) {
    return AbstractCall(*call);
  } else if (ExtractStringLiteral(value)) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO;
  } else if (llvm::isa<llvm::ZExtInst>(value)) {
    // zext pads with zeros (i.e., the MSB, the potential sign bit, will be 0),
    // so we know the value is non-negative.  Note that zext requires the dest
    // type to have a greater bitwidth than the src type.
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO;
  } else {
    // Can't determine the actual value, so assume that it can be
    // anything.
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP;
  }
}

bool ReturnRangePass::ShouldIgnore(const llvm::Function *func) const {
  return func == nullptr || func->isIntrinsic() || func->isDeclaration() ||
         !func->getReturnType()->isIntOrPtrTy();
}

char ReturnRangePass::ID = 0;
static llvm::RegisterPass<ReturnRangePass> X("return-range",
                                             "Return range of each function",
                                             false, false);

}  // namespace error_specifications
