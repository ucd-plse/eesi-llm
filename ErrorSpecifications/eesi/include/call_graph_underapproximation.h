#include "llvm/IR/Module.h"
#include "llvm/Analysis/CallGraph.h"

#include "llvm.h"

namespace error_specifications {

// A underapproximation of a LLVM module's callgraph based on LLVM's 
// CallGraph class. This call graph is needed because we ignore
// the external node provided by LLVM and we also want to ignore the
// unique numeric suffix that can be appended to a LLVM
// function, which causes a function to have multiple corresponding LLVM function objects.
class CallGraphUnderapproximation: public llvm::CallGraph {
  public:
    // Constructor, calls addToCallGraph() for every function in the module.
    CallGraphUnderapproximation(llvm::Module &module);
  private:
    using SourceFunctionMapTy = std::unordered_map<std::string, const llvm::Function *>;
    
    const llvm::Function *GetOrInsertCanonicalFunction(const llvm::Function *function);
    
    // Adds the function to call graph.
    void AddCallees(llvm::Function *function);

    // Map from source name of function to a corresponding llvm::Function object. 
    // Note that there might be multiple potential llvm::Function objects associated 
    // with a given source name (each having a different llvm name). This map 
    // contains one of them.
    SourceFunctionMapTy source_to_function_;

};

}  // namespace error_specifications
