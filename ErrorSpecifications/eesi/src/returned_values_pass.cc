#include "returned_values_pass.h"

#include <string>

#include "llvm/IR/CFG.h"

#include "eesi_common.h"
#include "llvm.h"

namespace error_specifications {

bool ReturnedValuesPass::runOnModule(llvm::Module &module) {
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
            std::shared_ptr<ReturnedValuesFact> prev =
                std::make_shared<ReturnedValuesFact>();
            for (auto &inst : basic_block) {
              input_facts_[&inst] = prev;
              output_facts_[&inst] = std::make_shared<ReturnedValuesFact>();
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

void ReturnedValuesPass::RunOnFunction(const llvm::Function &F) {
  std::string fname = F.getName().str();

  bool changed = true;
  while (changed) {
    changed = false;
    for (auto bi = F.begin(), be = F.end(); bi != be; ++bi) {
      const llvm::BasicBlock *BB = &*bi;

      const llvm::Instruction *bb_last = GetLastInstructionOfBB(BB);
      auto bb_out_fact = output_facts_.at(bb_last);

      // Go over successor blocks and apply join.
      for (auto si = succ_begin(BB), se = succ_end(BB); si != se; ++si) {
        const llvm::Instruction *succ_first = &(*(si->begin()));
        auto succ_fact = input_facts_.at(succ_first);
        bb_out_fact->Join(*succ_fact);
      }

      changed = visitBlock(*BB) || changed;
    }
  }

  return;
}

bool ReturnedValuesPass::visitBlock(const llvm::BasicBlock &BB) {
  bool changed = false;
  for (auto ii = BB.rbegin(), ie = BB.rend(); ii != ie; ++ii) {
    const llvm::Instruction &I = *ii;

    std::shared_ptr<ReturnedValuesFact> input_fact = input_facts_.at(&I);
    std::shared_ptr<ReturnedValuesFact> output_fact = output_facts_.at(&I);

    ReturnedValuesFact prev_fact = *input_fact;
    if (const llvm::ReturnInst *inst = llvm::dyn_cast<llvm::ReturnInst>(&I)) {
      VisitReturnInst(*inst, input_fact, output_fact);
    } else if (const llvm::CallInst *inst =
                   llvm::dyn_cast<llvm::CallInst>(&I)) {
      VisitCallInst(*inst, input_fact, output_fact);
    } else if (const llvm::LoadInst *inst =
                   llvm::dyn_cast<llvm::LoadInst>(&I)) {
      VisitLoadInst(*inst, input_fact, output_fact);
    } else if (const llvm::StoreInst *inst =
                   llvm::dyn_cast<llvm::StoreInst>(&I)) {
      VisitStoreInst(*inst, input_fact, output_fact);
    } else if (const llvm::BitCastInst *inst =
                   llvm::dyn_cast<llvm::BitCastInst>(&I)) {
      VisitBitCastInst(*inst, input_fact, output_fact);
    } else if (const llvm::PtrToIntInst *inst =
                   llvm::dyn_cast<llvm::PtrToIntInst>(&I)) {
      VisitPtrToIntInst(*inst, input_fact, output_fact);
    } else if (const llvm::TruncInst *inst =
                   llvm::dyn_cast<llvm::TruncInst>(&I)) {
      VisitTruncInst(*inst, input_fact, output_fact);
    } else if (const llvm::SExtInst *inst =
                   llvm::dyn_cast<llvm::SExtInst>(&I)) {
      VisitSExtInst(*inst, input_fact, output_fact);
    } else if (const llvm::PHINode *inst = llvm::dyn_cast<llvm::PHINode>(&I)) {
      VisitPHINode(*inst, input_fact, output_fact);
    } else {
      // Default is to just copy facts from previous instruction unchanged.
      input_fact->value = output_fact->value;
    }
    changed = changed || (*(input_fact) != prev_fact);
  }

  return changed;
}

void ReturnedValuesPass::AddReturnPropagated(const llvm::Function *f,
                                             const std::string &v) {
  if (return_propagated_.find(f) == return_propagated_.end()) {
    return_propagated_[f] = std::unordered_set<std::string>({v});
  } else {
    return_propagated_[f].insert(v);
  }
}

void ReturnedValuesPass::VisitCallInst(
    const llvm::CallInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;

  std::string fname = GetCalleeSourceName(I);
  if (fname.empty()) return;

  // Add every call instruction that can be returned to return propagated map.
  if (out->value.find(&I) != out->value.end()) {
    const llvm::Function *parent = I.getFunction();
    AddReturnPropagated(parent, fname);
  }

  // TODO(adityathakur): Consider using a regex.
  // LLVM creates multiple copies with numbers at end, e.g. ERR_PTR116.
  const std::vector<std::string> err_functions{"ERR_PTR", "IS_ERR", "PTR_ERR",
                                               "ERR_CAST"};
  for (const auto &err_function : err_functions) {
    if (fname.find(err_function) == std::string::npos) continue;
    llvm::Value *err = I.getOperand(0);
    if (out->value.find(&I) != out->value.end()) {
      in->value.insert(err);
    }
  }
}

// Insert the value being returned.
void ReturnedValuesPass::VisitReturnInst(
    const llvm::ReturnInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  // check for void return.
  if (I.getNumOperands() == 0) return;
  llvm::Value *returned = I.getOperand(0);
  in->value = out->value;
  in->value.insert(returned);
}

// Add sender if receiver element of out fact, remove receiver from in fact.
void ReturnedValuesPass::VisitStoreInst(
    const llvm::StoreInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  llvm::Value *sender = I.getOperand(0);
  llvm::Value *receiver = I.getOperand(1);
  in->value.erase(receiver);
  if (out->value.find(receiver) != out->value.end()) {
    in->value.insert(sender);
  }
}

// Add operand to in fact if load element of out fact.
void ReturnedValuesPass::VisitLoadInst(
    const llvm::LoadInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  llvm::Value *load_from = I.getOperand(0);
  in->value.erase(&I);
  if (out->value.find(&I) != out->value.end()) {
    in->value.insert(load_from);
  }
}

// Same as load.
void ReturnedValuesPass::VisitBitCastInst(
    const llvm::BitCastInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  llvm::Value *load_from = I.getOperand(0);
  in->value.erase(&I);
  if (out->value.find(&I) != out->value.end()) {
    in->value.insert(load_from);
  }
}

// Same as load.
void ReturnedValuesPass::VisitPtrToIntInst(
    const llvm::PtrToIntInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  llvm::Value *load_from = I.getOperand(0);
  in->value.erase(&I);
  if (out->value.find(&I) != out->value.end()) {
    in->value.insert(load_from);
  }
}

// Same as load.
void ReturnedValuesPass::VisitTruncInst(
    const llvm::TruncInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  in->value.erase(&I);
  llvm::Value *load_from = I.getOperand(0);
  if (out->value.find(&I) != out->value.end()) {
    in->value.insert(load_from);
  }
}

// Same as load.
void ReturnedValuesPass::VisitSExtInst(
    const llvm::SExtInst &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  in->value.erase(&I);
  llvm::Value *load_from = I.getOperand(0);
  if (out->value.find(&I) != out->value.end()) {
    in->value.insert(load_from);
  }
}

// If the PHI result can be returned, then add incoming values
// to the exit of each incoming basic block.
void ReturnedValuesPass::VisitPHINode(
    const llvm::PHINode &I, std::shared_ptr<ReturnedValuesFact> in,
    std::shared_ptr<const ReturnedValuesFact> out) {
  in->value = out->value;
  if (out->value.find(&I) == out->value.end()) {
    return;
  }
  in->value.erase(&I);

  for (unsigned i = 0, e = I.getNumIncomingValues(); i != e; ++i) {
    const llvm::Value *v = I.getIncomingValue(i);
    const llvm::BasicBlock *BB = I.getIncomingBlock(i);
    const llvm::Instruction *bb_last = GetLastInstructionOfBB(BB);

    // insert value into the output fact of the last instruction.
    auto bb_out_fact = output_facts_.at(bb_last);
    bb_out_fact->value.insert(v);
  }
}

ReturnedValuesFact ReturnedValuesPass::GetInFact(const llvm::Value *v) const {
  return *(input_facts_.at(v));
}

ReturnedValuesFact ReturnedValuesPass::GetOutFact(const llvm::Value *v) const {
  return *(output_facts_.at(v));
}

void ReturnedValuesPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.setPreservesAll();
}

char ReturnedValuesPass::ID = 0;
static llvm::RegisterPass<ReturnedValuesPass> X(
    "returned-values", "Functions that might be returned at each basic block",
    false, false);

}  // namespace error_specifications
