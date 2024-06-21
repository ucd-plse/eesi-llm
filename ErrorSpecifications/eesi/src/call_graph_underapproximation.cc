#include "call_graph_underapproximation.h"

namespace error_specifications {

CallGraphUnderapproximation::CallGraphUnderapproximation(llvm::Module &module)
    : llvm::CallGraph(module) {
  for (llvm::Function &function : module) {
    this->AddCallees(&function);
  }
};

const llvm::Function *CallGraphUnderapproximation::GetOrInsertCanonicalFunction(
    const llvm::Function *function) {
  std::string source_name = GetSourceName(*function);
  SourceFunctionMapTy::iterator it = source_to_function_.find(source_name);
  if (it != source_to_function_.end()) {
    return it->second;
  }
  source_to_function_[source_name] = function;
  return function;
}

void CallGraphUnderapproximation::AddCallees(llvm::Function *function) {
  // Getting the node that corresponds to the function.
  llvm::CallGraphNode *Node = this->getOrInsertFunction(function);

  // BasicBlock and Instruction cannot be const for these iterations.
  for (llvm::BasicBlock &basic_block : *function) {
    for (llvm::Instruction &inst : basic_block) {
      auto *call_inst = llvm::dyn_cast<llvm::CallInst>(&inst);
      if (!call_inst) continue;
      const llvm::Function *callee = GetCalleeFunction(*call_inst);
      if (!callee) continue;
      // Adding the edge from caller to callee.
      Node->addCalledFunction(
          call_inst,
          this->getOrInsertFunction(GetOrInsertCanonicalFunction(callee)));
    }
  }
}

}  // namespace error_specifications
