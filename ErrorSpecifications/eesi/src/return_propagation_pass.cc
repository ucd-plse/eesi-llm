#include "return_propagation_pass.h"

#include <iostream>
#include <memory>
#include <string>

#include "glog/logging.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

namespace error_specifications {

bool ReturnPropagationPass::runOnModule(llvm::Module &module) {
  if (finished) return false;

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
            std::shared_ptr<ReturnPropagationFact> prev =
                std::make_shared<ReturnPropagationFact>();
            for (auto &inst : basic_block) {
              input_facts_[&inst] = prev;
              output_facts_[&inst] = std::make_shared<ReturnPropagationFact>();
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

  finished = true;

  return false;
}

bool ReturnPropagationPass::RunOnFunction(const llvm::Function &F) {
  std::string fname = F.getName().str();

  bool changed = true;
  while (changed) {
    changed = false;

    for (auto bi = F.begin(), be = F.end(); bi != be; ++bi) {
      const llvm::BasicBlock *BB = &*bi;
      const llvm::Instruction *succ_begin = &*(BB->begin());
      auto succ_fact = input_facts_.at(succ_begin);

      // Go over predecessor blocks and apply join.
      for (auto pi = pred_begin(BB), pe = pred_end(BB); pi != pe; ++pi) {
        const llvm::Instruction *pred_term = (*pi)->getTerminator();
        auto pred_fact = output_facts_.at(pred_term);
        succ_fact->Join(*pred_fact);
      }

      changed = VisitBlock(*BB) || changed;
    }
  }

  return false;
}

bool ReturnPropagationPass::VisitBlock(const llvm::BasicBlock &BB) {
  bool changed = false;
  for (auto ii = BB.begin(), ie = BB.end(); ii != ie; ++ii) {
    const llvm::Instruction &I = *ii;

    std::shared_ptr<ReturnPropagationFact> input_fact = input_facts_.at(&I);
    std::shared_ptr<ReturnPropagationFact> output_fact = output_facts_.at(&I);
    ReturnPropagationFact prev_fact = *output_fact;

    if (const llvm::CallInst *inst = llvm::dyn_cast<llvm::CallInst>(&I)) {
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
    } else if (const llvm::BinaryOperator *inst =
                   llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
      VisitBinaryOperator(*inst, input_fact, output_fact);
    } else if (const llvm::PHINode *inst = llvm::dyn_cast<llvm::PHINode>(&I)) {
      VisitPHINode(*inst, input_fact, output_fact);
    } else {
      // Default is to just copy facts from previous instruction unchanged.
      output_fact->value = input_fact->value;
    }

    changed = changed || (*output_fact != prev_fact);
  }

  return changed;
}

void ReturnPropagationPass::VisitCallInst(
    const llvm::CallInst &I, std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  out->value = in->value;

  if (out->value.find(&I) == out->value.end()) {
    out->value[&I] = std::unordered_set<const llvm::Value *>();
  }
  out->value.at(&I).insert(&I);
}

// Copy the return facts into a new value.
void ReturnPropagationPass::VisitLoadInst(
    const llvm::LoadInst &I, std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  out->value = in->value;
  llvm::Value *load_from = I.getOperand(0);

  if (in->value.find(load_from) != in->value.end()) {
    out->value[&I] = in->value.at(load_from);
  }
}

// Copy the return facts into a new value.
void ReturnPropagationPass::VisitStoreInst(
    const llvm::StoreInst &I, std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  llvm::Value *sender = I.getOperand(0);
  llvm::Value *receiver = I.getOperand(1);

  out->value = in->value;

  if (llvm::isa<llvm::ConstantInt>(sender)) {
    if (out->value.find(receiver) == out->value.end()) {
      out->value[receiver] = std::unordered_set<const llvm::Value *>();
    }
    out->value.at(receiver).insert(sender);
  }

  if (in->value.find(sender) != in->value.end()) {
    out->value[receiver] = in->value.at(sender);
  }
}

void ReturnPropagationPass::VisitBitCastInst(
    const llvm::BitCastInst &I, std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  // Identical to load.
  out->value = in->value;
  llvm::Value *load_from = I.getOperand(0);
  if (in->value.find(load_from) != in->value.end()) {
    out->value[&I] = in->value.at(load_from);
  }
}

void ReturnPropagationPass::VisitPtrToIntInst(
    const llvm::PtrToIntInst &I,
    std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  // Identical to load.
  out->value = in->value;
  llvm::Value *load_from = I.getOperand(0);
  if (in->value.find(load_from) != in->value.end()) {
    out->value[&I] = in->value.at(load_from);
  }
}

void ReturnPropagationPass::VisitBinaryOperator(
    const llvm::BinaryOperator &I,
    std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  // Identical to load.
  out->value = in->value;
  llvm::Value *load_from = I.getOperand(0);
  if (in->value.find(load_from) != in->value.end()) {
    out->value[&I] = in->value.at(load_from);
  }
}

void ReturnPropagationPass::VisitPHINode(
    const llvm::PHINode &I, std::shared_ptr<const ReturnPropagationFact> in,
    std::shared_ptr<ReturnPropagationFact> out) {
  // Union all of the sets together for phi incoming values.
  for (unsigned i = 0, e = I.getNumIncomingValues(); i != e; ++i) {
    llvm::Value *v = I.getIncomingValue(i);
    if (in->value.find(v) != in->value.end()) {
      for (const llvm::Value *from : in->value.at(v)) {
        out->value[&I].insert(from);
      }
    }
  }
}

void ReturnPropagationPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.setPreservesAll();
}

char ReturnPropagationPass::ID = 0;
static llvm::RegisterPass<ReturnPropagationPass> X(
    "return-propagation", "Map from LLVM value to return values it may hold",
    false, false);

}  // namespace error_specifications
