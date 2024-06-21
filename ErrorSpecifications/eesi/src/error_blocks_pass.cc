#include "error_blocks_pass.h"

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "eesi_common.h"
#include "glog/logging.h"
#include "llvm.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "return_constraints_pass.h"
#include "return_propagation_pass.h"
#include "return_range_pass.h"
#include "returned_values_pass.h"

namespace error_specifications {

void ErrorBlocksPass::SetSpecificationsRequest(
    const GetSpecificationsRequest &req) {
  smart_success_code_zero_ = req.smart_success_code_zero();
  checker_ = new Checker();
  language_model_ = new GptModel(req.ctags_file());

  for (const auto &error_only_fn : req.error_only_functions()) {
    const auto source_name = error_only_fn.function().source_name();
    std::unordered_map<int, ConstantValue> required_arg_map;

    for (const auto &required_arg : error_only_fn.required_args()) {
      if (required_arg.position() < 0) {
        LOG(WARNING) << "Ignoring error-only argument for " << source_name
                     << " with negative position " << required_arg.position()
                     << ".";
      } else if (required_arg_map.count(required_arg.position()) > 0) {
        LOG(WARNING) << "Ignoring error-only argument for " << source_name
                     << " with duplicate position " << required_arg.position()
                     << ".";
      } else {
        required_arg_map[required_arg.position()] = required_arg.value();
      }
    }

    error_only_functions_.insert({source_name, required_arg_map});
    AddNonDoomedFunction(source_name);
  }

  // Store error codes.
  for (const auto &error_code : req.error_codes()) {
    if (error_codes_.find(error_code.value()) == error_codes_.end()) {
      error_codes_[error_code.value()] = std::unordered_set<std::string>();
    }
    for (const auto &submodule : error_code.submodules()) {
      error_codes_[error_code.value()].insert(submodule);
    }
    error_code_names_[error_code.name()] = AbstractInteger(error_code.value());
  }

  // Store success codes.
  for (const auto &success_code : req.success_codes()) {
    if (success_codes_.find(success_code.value()) == success_codes_.end()) {
      success_codes_[success_code.value()] = std::unordered_set<std::string>();
    }
    for (const auto &submodule : success_code.submodules()) {
      success_codes_[success_code.value()].insert(submodule);
    }
    success_code_names_[success_code.name()] =
        AbstractInteger(success_code.value());
  }

  // Store initial specifications.
  for (const auto &specification : req.initial_specifications()) {
    // Convert the specification to a constraint.
    // The only real difference is that a constraint uses a string function
    // name instead of a Function protobuf, and implements join and
    // meet operations.
    std::string specification_function_name =
        specification.function().source_name();
    LOG(INFO) << "InitSpec"
              << " f=" << specification_function_name;
    // The confidence of any domain knowledge is always kMaxConfidence. The
    // relevant confidence values will be set to kMinConfidence if the lattice
    // element for the specification does not intersect (non-bottom Meet) with
    // the relevant lattice element that the confidence represents. This is
    // automatically done in the Constraint constructor.
    auto confidence_emptyset = kMinConfidence;
    // If we know that the domain knowledge is bottom, then the confidence
    // for isempty should be kMaxConfidence.
    if (SignLattice::IsBottom(specification.lattice_element())) {
      confidence_emptyset = kMaxConfidence;
    }
    LatticeElementConfidence c =
        ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
            specification.lattice_element(), confidence_emptyset);

    // Save a copy of the initial specification so that we can assert
    // that it has not been modified.
    initial_error_specifications_[specification_function_name] = c;

    // Bootstrap the analysis with the initial specification.
    error_specifications_[specification_function_name] = c;
    AddNonDoomedFunction(specification_function_name);
  }
}

bool ErrorBlocksPass::IgnoreFunction(const llvm::Function *function) const {
  return function == nullptr || function->isIntrinsic() ||
         initial_error_specifications_.find(GetSourceName(*function)) !=
             initial_error_specifications_.end() ||
         IsVoidFunction(*function);
}

bool ErrorBlocksPass::runOnThirdPartyFunctions(
    const llvm::CallGraph &call_graph) {
  // return false;
  std::vector<std::pair<std::string, std::string>> third_party_functions;
  int num_third_party = 0;
  for (auto scc_it = llvm::scc_begin(&call_graph); !scc_it.isAtEnd();
       ++scc_it) {
    for (auto node : *scc_it) {
      auto f = node->getFunction();
      if (!IgnoreFunction(f) && f->begin() == f->end()) {
        std::string return_type_str = "Pointer";
        if (GetReturnType((*f)) ==
            FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER) {
          return_type_str = "Integer";
        }
        third_party_functions.push_back(
            std::make_pair(GetSourceName(*f), return_type_str));
        num_third_party++;
      }
    }
  }
  LOG(INFO) << "Number of third party functions: " << num_third_party;
  return LlmExpandThirdPartyErrorSpecifications(third_party_functions);
}

bool ErrorBlocksPass::runOnModule(llvm::Module &module) {
  LOG(INFO) << "ErrorBlocksPass running on module...";
  module_ = &module;

  // Generating the call graph and traversing the SCCs bottom-up.
  llvm::CallGraph call_graph = CallGraphUnderapproximation(module);

  // Going to first run with the LLM on third-party functions, seeing if it
  // knows anything just by name (e.g., malloc, fwrite, etc.).
  // TODO(patrickjchap): Undo this
  bool updated_third_party = runOnThirdPartyFunctions(call_graph);
  // bool updated_third_party = false;
  if (updated_third_party) {
    LOG(INFO) << "Updated third party functions!";
  }

  // The set of functions whose error specifications have converged
  // and are not bottom, along with their return type.
  std::unordered_map<std::string, FunctionReturnType> converged_functions;
  // Error specifications provided as domain knowledge will not
  // changed, and, thus, have converged.
  for (const auto &kv : initial_error_specifications_) {
    // Even if function has a \bottom specification, we still need
    // its return type for the mapping.
    llvm::Function *f = module.getFunction(kv.first);
    FunctionReturnType typ =
        f == nullptr ? FunctionReturnType::FUNCTION_RETURN_TYPE_OTHER
                     : GetReturnType(*f);
    function_return_types_.insert(std::make_pair(kv.first, typ));
    converged_functions.insert(std::make_pair(kv.first, typ));
  }

  std::vector<std::string> function_labels;
  for (const std::string &function_label : function_labels) {
    llvm::Function *f = module.getFunction(function_label);
    // We are only interested in adding integer/pointer functions.
    if (IgnoreFunction(f)) continue;

    AddNonDoomedFunction(function_label);
  }

  for (auto scc_it = llvm::scc_begin(&call_graph); !scc_it.isAtEnd();
       ++scc_it) {
    std::vector<llvm::Function *> scc_funcs;
    for (auto node : *scc_it) {
      auto f = node->getFunction();
      if (!IgnoreFunction(f)) scc_funcs.push_back(f);
    }
    const bool has_loop = scc_it.hasLoop();
    bool changed = false;
    do {
      changed = false;
      for (auto func : scc_funcs) {
        // Analyzing the function, attempting to infer a specification.
        changed = RunOnFunction(func) || changed;
      }
      // Perform fixpoint only if SCC has a loop.
    } while (has_loop && changed);

    // Only expand using the embedding if the appropriate SynonymFinder
    //    has
    // been configured.

    const auto &return_range_pass = getAnalysis<ReturnRangePass>();
    auto it1 = std::partition(
        scc_funcs.begin(), scc_funcs.end(),
        [this, &return_range_pass](llvm::Function *func) {
          const auto return_range = return_range_pass.GetReturnRange(
              *func,
              /*default=*/SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
          std::string func_name = GetSourceName(*func);
          return ReturnsDomainKnowledgeCodes(func_name) ||
                 ConfidenceLattice::IsEmptyset(
                     GetErrorSpecification(func_name)) ||
                 !ConfidenceLattice::IsUnknown(
                     GetErrorSpecification(func_name));
        });
    // it1 to scc_funcs.end() are the functions whose error
    // specifications are bottom. We only need to expand the error
    // specifications for these functions.
    auto it2 =
        std::partition(it1, std::end(scc_funcs),
                       [this, &converged_functions](llvm::Function *func) {
                         // return ExpandErrorSpecification(func,
                         // converged_functions);
                         return LlmExpandErrorSpecification(func);
                       });
    // scc_funcs.begin() to it2 are the functions whose error
    // specifications
    // are not bottom and have converged. Add these to the set of
    //      converged
    // functions.
    std::for_each(std::begin(scc_funcs), it2,
                  [this, &converged_functions](auto f) {
                    converged_functions.insert(
                        std::make_pair(GetSourceName(*f), GetReturnType(*f)));
                  });
  }

  // Just printing off the reachable functions and the total count, as well as
  // the total count of specifications.
  LOG(INFO) << "Functions that are non-doomed:";
  for (auto non_doomed_function_name : non_doomed_function_names_) {
    LOG(INFO) << non_doomed_function_name;
  }

  // We can ignore checking for LLVM intrinsics here as IgnoreFunction() is
  // called on every function before it gets added to function_return_types_.
  auto total_non_void_functions = std::count_if(
      function_return_types_.begin(), function_return_types_.end(),
      [this](std::pair<std::string, FunctionReturnType> f) {
        return f.second == FunctionReturnType::FUNCTION_RETURN_TYPE_INTEGER ||
               f.second == FunctionReturnType::FUNCTION_RETURN_TYPE_POINTER;
      });

  LOG(INFO) << "Total number of Integer/Pointer functions: "
            << total_non_void_functions;
  LOG(INFO) << "Total number of non-doomed functions: "
            << non_doomed_function_names_.size();
  LOG(INFO) << "Total number of specifications inferred: "
            << error_specifications_.size();

  LOG(INFO) << "ErrorBlocks Finished";
  google::FlushLogFiles(google::INFO);

  // Does not modify bitcode.
  return false;
}

bool ErrorBlocksPass::LlmExpandThirdPartyErrorSpecifications(
    std::vector<std::pair<std::string, std::string>> function_names) {
  LOG(INFO) << "LLM Third party expansion";
  bool updated = false;
  std::vector<Specification> specifications;
  for (auto init_spec : initial_error_specifications_) {
    Function f;
    f.set_llvm_name(init_spec.first);
    f.set_source_name(init_spec.first);
    Specification s;
    s.mutable_function()->CopyFrom(f);
    s.set_lattice_element(
        ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
            init_spec.second));
    specifications.push_back(s);
  }
  auto llm_specifications = language_model_->GetThirdPartySpecifications(
      function_names, specifications, error_code_names_, success_code_names_);
  for (auto specification : llm_specifications) {
    // This is confusing, but we are translating the proto "BOTTOM" response
    // from the model as emptyset, since we are only passing lattice elements
    // to reduce the size of the proto.
    LatticeElementConfidence lattice_confidence(0, 0, 0, 50);
    if (specification.second ==
        SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM) {
      continue;
    }
    lattice_confidence =
        ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
            specification.second, kMinConfidence, 0.5);
    LOG(INFO) << "LLama says: " << lattice_confidence;
    bool updated_spec =
        UpdateErrorSpecification(specification.first, lattice_confidence) ||
        updated;
    // if (specification.first != func_name) continue;
    // Silly, but this if for logging purposes...
    updated = updated_spec || updated;
    if (updated_spec) {
      LOG(INFO) << "LLM successful update!";
      LOG(INFO) << "LLM specification: "
                << GetErrorSpecification(specification.first);
    }
  }
  return updated;
}

bool ErrorBlocksPass::LlmExpandErrorSpecification(llvm::Function *func) {
  // LLM needs function source code on this step, so must have basic blocks.
  if (!func || func->begin() == func->end()) return false;
  const std::string func_name = GetSourceName(*func);
  LOG(INFO) << "LLM Expand " << func_name;
  // TODO(patrickjchap): After testing, we need to ensure we have all the
  // called function error specifications passed to the LLM for
  // context.
  std::vector<Specification> specifications;
  if (called_functions_.find(func_name) != called_functions_.end()) {
    for (auto called_function : called_functions_[func_name]) {
      auto lattice_confidence = GetErrorSpecification(called_function);
      if (ConfidenceLattice::IsUnknown(lattice_confidence)) {
        LOG(INFO) << "No error specification for called " << called_function;
        continue;
      }
      LOG(INFO) << "Found called error specification for " << called_function;
      SignLatticeElement lattice_element =
          ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
              lattice_confidence);
      Function f;
      f.set_llvm_name(called_function);
      f.set_source_name(called_function);
      Specification s;
      s.mutable_function()->CopyFrom(f);
      s.set_lattice_element(lattice_element);
      specifications.push_back(s);
    }
  }
  bool updated = false;
  auto llm_specifications = language_model_->GetSpecification(
      func_name, specifications, error_code_names_, success_code_names_);
  for (auto specification : llm_specifications) {
    // This is confusing, but we are translating the proto "BOTTOM" response
    // from the model as emptyset, since we are only passing lattice elements
    // to reduce the size of the proto.
    LatticeElementConfidence lattice_confidence(0, 0, 0, 50);
    if (specification.second !=
        SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM) {
      lattice_confidence =
          ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
              specification.second, kMinConfidence, 0.5);
    }
    LOG(INFO) << "LLama says: " << lattice_confidence;
    bool updated_spec =
        UpdateErrorSpecification(specification.first, lattice_confidence);
    // if (specification.first != func_name) continue;
    if (updated_spec) {
      LOG(INFO) << "LLM successful update!";
      LOG(INFO) << "LLM specification: "
                << GetErrorSpecification(specification.first);
    }
    updated = updated_spec || updated;
  }
  return updated;
}

bool ErrorBlocksPass::RunOnFunction(llvm::Function *fn) {
  const auto fn_name = GetSourceName(*fn);
  // Ideally we want to incorporate the LLVM names back into this, but the
  // entire pipeline would have to account for this, which it doesn't.... Just
  // take the first instance. This is very hacky and poorly written, but this
  // just needs to work for now.
  if (name_to_function_.find(fn_name) != name_to_function_.end()) {
    llvm::Function *found_func = (*(name_to_function_.find(fn_name))).second;
    if (found_func != fn) {
      return false;
    }
  } else {
    name_to_function_[fn_name] = fn;
  }

  LOG(INFO) << "Analyze " << fn_name;
  // Add every function to return type map.
  function_return_types_[fn_name] =
      fn == nullptr ? FunctionReturnType::FUNCTION_RETURN_TYPE_OTHER
                    : GetReturnType(*fn);

  // Initialize the join result to emptyset.
  std::vector<LatticeElementConfidence> block_confidences;
  for (auto &basic_block : *fn) {
    block_confidences.push_back(VisitBlock(basic_block));
  }

  // If no blocks are analyzed, then the specification will not change from
  // the static analysis, in this case all confidence values remain
  // kMinConfidence. This typically happens with external functions.
  if (block_confidences.empty()) {
    return false;
  }

  // Iterator ends where emptyset specifications stop.
  auto it =
      std::partition(block_confidences.begin(), block_confidences.end(),
                     [](LatticeElementConfidence block_confidence) {
                       return ConfidenceLattice::IsEmptyset(block_confidence);
                     });

  // If all blocks return emptyset, then we will update this function
  // to emptyset. Else we will just join the non-emptyset specifications.
  LatticeElementConfidence blocks_join_result;
  if (it == block_confidences.end()) {
    blocks_join_result = LatticeElementConfidence(
        kMinConfidence, kMinConfidence, kMinConfidence, kMaxConfidence);
  } else {
    // Join the result of every analyzed block.
    std::for_each(
        it, block_confidences.end(),
        [&blocks_join_result](LatticeElementConfidence block_confidence) {
          blocks_join_result =
              ConfidenceLattice::Join(block_confidence, blocks_join_result);
        });
  }

  // We need these names to check for SmartSuccessCodeZero.
  std::string parent_fname = GetSourceName(*fn);
  std::string function_fname;
  if (fn && fn->begin() != fn->end()) {
    const llvm::Instruction *first_bb =
        GetFirstInstructionOfBB(&*(fn->begin()));
    function_fname = GetSourceFileName(*first_bb);
  }

  // If 0 is the first processed error return statement, the heuristic will
  // incorrectly count it towards the error specification.
  if (ShouldSmartDropZero(parent_fname, function_fname)) {
    LatticeElementConfidence zero_confidence(kMaxConfidence, kMinConfidence,
                                             kMinConfidence);
    auto downgraded_lattice_confidence = ConfidenceLattice::Difference(
        blocks_join_result, SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
    if (downgraded_lattice_confidence != blocks_join_result) {
      blocks_join_result = downgraded_lattice_confidence;
      LOG(INFO) << "Retroactively dropped 0 from error specification for "
                << parent_fname;
    }
  }

  return UpdateErrorSpecification(fn, blocks_join_result);
}

std::set<SignLatticeElement> ErrorBlocksPass::CollectConstraints(
    const llvm::Function &parent_function, const std::string &fn_name) {
  std::set<SignLatticeElement> ret;

  ReturnConstraintsPass &return_constraints_pass =
      getAnalysis<ReturnConstraintsPass>();

  // Go over every instruction in parent_function.
  for (auto &basic_block : parent_function) {
    for (auto &inst : basic_block) {
      ReturnConstraintsFact return_constraints_fact =
          return_constraints_pass.GetInFact(&inst);
      const auto &fn_constraint = return_constraints_fact.value.find(fn_name);
      // If there there is constraint on the instruction associated with
      // fn_name.
      if (fn_constraint != return_constraints_fact.value.end()) {
        // Then add that constraint to ret.
        ret.insert(fn_constraint->second.lattice_element);
      }
    }
  }

  return ret;
}

void ErrorBlocksPass::CheckViolations(const llvm::Function &func) {
  for (const auto &basic_block : func) {
    for (const auto &inst : basic_block) {
      if (const llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(&inst)) {
        CheckViolations(*call);
      }
    }
  }
}

void ErrorBlocksPass::CheckViolations(const llvm::CallInst &call_inst) {
  const Function callee_function = GetCallee(call_inst);

  const std::string &callee_function_name = callee_function.source_name();
  auto confidence_it = error_specifications_.find(callee_function_name);
  if (confidence_it == error_specifications_.end()) {
    return;
  }

  const LatticeElementConfidence &lattice_confidence = confidence_it->second;

  auto callee_constraints =
      CollectConstraints(*(call_inst.getFunction()), callee_function_name);

  SignLatticeElement lattice_element =
      ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
          lattice_confidence);
  checker_->CheckViolations(call_inst, lattice_element, callee_constraints);
}

LatticeElementConfidence ErrorBlocksPass::VisitBlock(
    const llvm::BasicBlock &BB) {
  // Initial block join result should be emptyset. If a block constraint is
  // unknown, i.e. a function call with an unknown specification is
  // constraining a block, then the join result later on will be joined to
  // "unknown".
  LatticeElementConfidence join_result(kMinConfidence, kMinConfidence,
                                       kMinConfidence, kMaxConfidence);

  std::string parent_fname = GetSourceName(*BB.getParent());
  const llvm::Instruction *bb_first = GetFirstInstructionOfBB(&BB);
  const std::string &function_fname = GetSourceFileName(*bb_first);
  if (called_functions_.find(parent_fname) == called_functions_.end()) {
    called_functions_[parent_fname] = {};
  }
  for (auto ii = BB.begin(), ie = BB.end(); ii != ie; ++ii) {
    const llvm::Instruction &I = *ii;
    if (const llvm::CallInst *inst = llvm::dyn_cast<llvm::CallInst>(&I)) {
      join_result = ConfidenceLattice::Join(VisitCallInst(*inst), join_result);
      std::string callee_name = GetCalleeSourceName(*inst);
      called_functions_[parent_fname].insert(callee_name);
      LOG(INFO) << "Added called function -> " << callee_name << " for "
                << parent_fname << "!";
    }
  }
  ReturnedValuesPass &returned_values_pass = getAnalysis<ReturnedValuesPass>();
  ReturnedValuesFact rtf = returned_values_pass.GetInFact(bb_first);

  // Only process blocks that can return a single value,
  // i.e. there exists a value that must be returned. If this is not true,
  // then we return the join_result, which is either emptyset or the result
  // from VisitCallInst.
  if (rtf.value.size() != 1) return join_result;
  auto returned_value = *rtf.value.begin();

  // Check for error codes.
  if (const llvm::ConstantInt *int_return =
          llvm::dyn_cast<llvm::ConstantInt>(returned_value)) {
    // getSExtValue can only be called with a bit width of <= 64.
    // Omitting this check can cause an assertion failure.
    if (int_return->getBitWidth() <= 64) {
      int64_t return_value = int_return->getSExtValue();

      if (IsErrorCode(return_value, function_fname)) {
        AddNonDoomedFunction(parent_fname);
        AddFunctionReturningDomainKnowledgeCodes(parent_fname);
        join_result = ConfidenceLattice::Join(
            AddErrorValue(BB.getParent(), return_value), join_result);
        LOG(INFO) << "ErrorCode"
                  << " c=" << return_value << *bb_first;
      } else if (IsSuccessCode(parent_fname, return_value, function_fname)) {
        // This check is different from the IsErrorCode check, since 0 might
        // not be considered a success code if the corresponding heuristic is
        // enabled.
        AddFunctionReturningDomainKnowledgeCodes(parent_fname);
        LOG(INFO) << "SuccessCode"
                  << " c=" << return_value << *bb_first;
        return join_result;
      }
    }
  }
  ReturnPropagationPass &return_propagation_pass =
      getAnalysis<ReturnPropagationPass>();
  ReturnConstraintsPass &return_constraints_pass =
      getAnalysis<ReturnConstraintsPass>();
  const llvm::Instruction *bb_last = GetLastInstructionOfBB(&BB);
  ReturnConstraintsFact rcf = return_constraints_pass.GetOutFact(bb_last);
  // string constraint_fname is the function whose return value is
  // constraining this block. Constraint block_constraint is the abstract
  // value of the constraint on block execution. Constraint constraint_aerv is
  // the abstract error return value of constraint_f.
  for (auto kv : rcf.value) {
    std::string constraint_fname = kv.first;
    // Empty name constraints should never affect the analysis, since we
    // cannot determine which function's error specifications are constraining
    // the block. Relying on string empty is not the cleanest way to handle
    // this, but it's straightforward for now.
    if (constraint_fname.empty()) continue;
    Constraint block_constraint = kv.second;

    // Get the error specification (AERV) for function constraining this
    // block.
    LatticeElementConfidence constraining_function_confidence =
        GetErrorSpecification(constraint_fname);

    LatticeElementConfidence block_intersection_confidence =
        ConfidenceLattice::Intersection(constraining_function_confidence,
                                        block_constraint.lattice_element);
    // Check to see if block is executed when function `constraint_fname`
    // returns an error. Update the error specification when the return
    // constraint for the block is emptyset. Updating when the block
    // insersection confidence is unknown will not cause the error
    // specification to become updated.
    if (ConfidenceLattice::IsUnknown(block_intersection_confidence) ||
        ConfidenceLattice::IsEmptyset(block_intersection_confidence)) {
      join_result =
          ConfidenceLattice::Join(block_intersection_confidence, join_result);
      continue;
    }

    LatticeElementConfidence return_lattice_confidence;
    // Transform values that can be returned into
    // a constraint on a function return value.

    std::string propagate_callee;
    if (block_constraint.lattice_element !=
        SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP) {
      if (const auto maybe_bool = ExtractBoolean(*returned_value)) {
        auto return_confidence_zero = kMinConfidence;
        auto return_confidence_not_zero = kMinConfidence;

        if (*maybe_bool) {
          return_confidence_not_zero =
              ConfidenceLattice::GetMax(block_intersection_confidence);
        } else {
          return_confidence_zero =
              ConfidenceLattice::GetMax(block_intersection_confidence);
        }
        return_lattice_confidence = LatticeElementConfidence(
            return_confidence_zero,
            /* <0 */ return_confidence_not_zero,
            /* >0 */ return_confidence_not_zero,
            block_intersection_confidence.GetConfidenceEmptyset());

        LOG(INFO) << "ErrorConstantBool"
                  << " f=" << parent_fname << *bb_last
                  << " c=" << (*maybe_bool ? "true" : "false")
                  << " abstracted=\"" << return_lattice_confidence << "\""
                  << " fprime=" << constraint_fname << " l=\""
                  << block_constraint.lattice_element << "\""
                  << " E(fprime)=\"" << GetErrorSpecification(constraint_fname)
                  << "\"";
      } else if (const llvm::ConstantInt *int_return =
                     llvm::dyn_cast<llvm::ConstantInt>(returned_value)) {
        int64_t return_value = int_return->getSExtValue();
        auto return_confidence_zero = kMinConfidence;
        auto return_confidence_less_than_zero = kMinConfidence;
        auto return_confidence_greater_than_zero = kMinConfidence;
        // The confidence in the case that the return is a constant int should
        // be the same as the confidence of the function's error specification
        // that is constraining the block. This value should be the max
        // between
        // ==0 and !=0 confidences, if the block constraint intersects with
        // these lattice elements. The confidences will be automatically
        // corrected for the appropriate lattice element once the constraint
        // is initialized.
        if (return_value == 0) {
          return_confidence_zero =
              ConfidenceLattice::GetMax(block_intersection_confidence);
        } else if (return_value > 0) {
          return_confidence_greater_than_zero =
              ConfidenceLattice::GetMax(block_intersection_confidence);
          // Same as the return_value being <0.
        } else {
          return_confidence_less_than_zero =
              ConfidenceLattice::GetMax(block_intersection_confidence);
        }
        return_lattice_confidence = LatticeElementConfidence(
            return_confidence_zero, return_confidence_less_than_zero,
            return_confidence_greater_than_zero,
            block_intersection_confidence.GetConfidenceEmptyset());
        LOG(INFO) << "ErrorConstantInt"
                  << " f=" << parent_fname << *bb_last << " c=" << return_value
                  << " abstracted=\"" << return_lattice_confidence << "\""
                  << " fprime=" << constraint_fname << " l=\""
                  << block_constraint.lattice_element << "\""
                  << " E(fprime)=\"" << GetErrorSpecification(constraint_fname)
                  << "\"";
      } else if (llvm::isa<llvm::ConstantPointerNull>(returned_value)) {
        propagate_callee = constraint_fname;
        // The confidence_zero should be the max of the constraining
        // function's error specification confidences.
        auto return_confidence_zero =
            ConfidenceLattice::GetMax(block_intersection_confidence);
        return_lattice_confidence = LatticeElementConfidence(
            return_confidence_zero, /* <0 */ kMinConfidence,
            /* >0 */ kMinConfidence,
            block_intersection_confidence.GetConfidenceEmptyset());
        LOG(INFO) << "ErrorConstantNull"
                  << " f=" << parent_fname << *bb_last << " c=0"
                  << " abstracted=\"" << return_lattice_confidence << "\""
                  << " fprime=" << constraint_fname << " l=\""
                  << block_constraint.lattice_element << "\""
                  << " E(fprime)=\"" << GetErrorSpecification(constraint_fname)
                  << "\"";
      } else if (const auto maybe_string_literal =
                     ExtractStringLiteral(*returned_value)) {
        // The confidence_less_than_zero and confidence_greater_than_zero
        // should be the max of the constraining function's error
        // specification confidences.
        auto return_confidence_less_than_zero =
            ConfidenceLattice::GetMax(block_intersection_confidence);
        auto return_confidence_greater_than_zero =
            return_confidence_less_than_zero;
        return_lattice_confidence = LatticeElementConfidence(
            /* ==0 */ kMinConfidence, return_confidence_less_than_zero,
            return_confidence_greater_than_zero,
            block_intersection_confidence.GetConfidenceEmptyset());
        propagate_callee = constraint_fname;
        LOG(INFO) << "ErrorStringLiteral"
                  << " f=" << parent_fname << *bb_last << " c=\""
                  << maybe_string_literal->str() << "\""
                  << " abstracted=\"" << return_lattice_confidence << "\""
                  << " fprime=" << constraint_fname << " l=\""
                  << block_constraint.lattice_element << "\""
                  << " E(fprime)=\"" << GetErrorSpecification(constraint_fname);
      }
    }

    const llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(returned_value);
    if (call) {
      // DIRECT PROPAGATION
      // The function is returning a call instruction.
      std::string callee_name = GetCallee(*call).source_name();
      LatticeElementConfidence callee_confidence = GetErrorSpecification(*call);

      // If any confidence values are greater-than kMinConfidence, we want
      // to propagate this confidence, including emptyset. If the emptyset
      // confidence should be negated further in the analysis, this will be
      // performed in the Join.
      if (!ConfidenceLattice::IsUnknown(callee_confidence)) {
        // If we return a call instruction, we take the callee function's
        // confidence.
        propagate_callee = callee_name;
        return_lattice_confidence = callee_confidence;
        // There is a small chance that the constraining function on the
        // block is the same as the function value returned. If this is
        // the case, we have to make sure that the actual error value can
        // be returned.
        if (callee_name.compare(constraint_fname) == 0) {
          return_lattice_confidence = ConfidenceLattice::Meet(
              callee_confidence, block_intersection_confidence);
          LOG(INFO) << "Returned function same as constraining function, "
                    << "performing meet: " << return_lattice_confidence;
        }
        LOG(INFO) << "PropagationDirect"
                  << " f=" << parent_fname << *bb_last
                  << " fprime=" << constraint_fname << " constraint=\""
                  << block_constraint.lattice_element << "\""
                  << " E(fprime)=\"" << constraining_function_confidence << "\""
                  << " g=\"" << propagate_callee << "\""
                  << " E(g)=\"" << callee_confidence;

        if (ReturnsDomainKnowledgeCodes(propagate_callee)) {
          AddFunctionReturningDomainKnowledgeCodes(parent_fname);
        }
        if (ShouldSmartDropZero(parent_fname, function_fname)) {
          auto downgraded_lattice_confidence = ConfidenceLattice::Difference(
              return_lattice_confidence,
              SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
          if (downgraded_lattice_confidence != return_lattice_confidence) {
            // In this case, the heuristic didn't drop the callee's 0
            // return...
            LOG(INFO) << "Downgraded directly propagated lattice element to "
                      << downgraded_lattice_confidence;
          }
          return_lattice_confidence = downgraded_lattice_confidence;
        }
      }
    } else {
      // INDIRECT PROPAGATION
      // The function is returning a value which can hold a call instruction
      // at this program point. Check to see if the returned value can hold
      // the return value of a function.
      ReturnPropagationFact rpf =
          *(return_propagation_pass.output_facts_.at(bb_last));

      if (rpf.value.find(returned_value) != rpf.value.end()) {
        if (rpf.value.at(returned_value).size() > 1) {
          continue;
        }

        for (const auto &v : rpf.value.at(returned_value)) {
          if (const llvm::ConstantInt *int_return =
                  llvm::dyn_cast<llvm::ConstantInt>(v)) {
            int64_t return_value = int_return->getSExtValue();
            auto return_confidence_zero = kMinConfidence;
            auto return_confidence_less_than_zero = kMinConfidence;
            auto return_confidence_greater_than_zero = kMinConfidence;
            // The confidence in the case that the return is a constant int
            // should be the same as the confidence of the function's error
            // specification that is constraining the block. This value should
            // be the max between
            // ==0 and !=0 confidences, if the block constraint intersects
            // with these lattice elements. The confidences will be
            // automatically corrected for the appropriate lattice element
            // once the constraint is initialized.
            if (return_value == 0) {
              return_confidence_zero =
                  ConfidenceLattice::GetMax(block_intersection_confidence);
            } else if (return_value > 0) {
              return_confidence_greater_than_zero =
                  ConfidenceLattice::GetMax(block_intersection_confidence);
              // Same as the return_value being <0.
            } else {
              return_confidence_less_than_zero =
                  ConfidenceLattice::GetMax(block_intersection_confidence);
            }
            return_lattice_confidence = LatticeElementConfidence(
                return_confidence_zero, return_confidence_less_than_zero,
                return_confidence_greater_than_zero,
                block_intersection_confidence.GetConfidenceEmptyset());
            LOG(INFO) << "PropagationIndirectConstantInt"
                      << " f=" << parent_fname << *bb_last
                      << " c=" << return_value << " abstracted=\""
                      << return_lattice_confidence << "\""
                      << " fprime=" << constraint_fname << " constraint=\""
                      << block_constraint.lattice_element << "\""
                      << " E(fprime)=\"" << constraining_function_confidence;
          } else if (llvm::isa<llvm::ConstantPointerNull>(v)) {
            // The confidence_zero should be the max of the constraining
            // function's error specification confidences.
            auto return_confidence_zero =
                ConfidenceLattice::GetMax(block_intersection_confidence);
            return_lattice_confidence = LatticeElementConfidence(
                return_confidence_zero, /* <0 */ kMinConfidence,
                /* >0 */ kMinConfidence,
                block_intersection_confidence.GetConfidenceEmptyset());
            LOG(INFO) << "PropagationIndirectConstantNull"
                      << " f=" << parent_fname << *bb_last << " c=0"
                      << " abstracted=\"" << return_lattice_confidence << "\""
                      << " fprime=" << constraint_fname << " l=\""
                      << block_constraint.lattice_element << "\""
                      << " E(fprime)=\""
                      << GetErrorSpecification(constraint_fname) << "\"";
          } else if (const llvm::CallInst *call =
                         llvm::dyn_cast<llvm::CallInst>(v)) {
            std::string callee_name = GetCallee(*call).source_name();
            LatticeElementConfidence callee_confidence =
                GetErrorSpecification(*call);
            // If any confidence values are greater-than kMinConfidence, we
            // want to propagate this confidence, including emptyset. If the
            // emptyset confidence should be negated further in the analysis,
            // this will be performed in the Join.
            if (ConfidenceLattice::IsUnknown(callee_confidence)) continue;
            return_lattice_confidence = callee_confidence;
            // There is a small chance that the constraining function on the
            // block is the same as the function value returned. If this is
            // the case, we have to make sure that the actual error value can
            // be returned.
            if (callee_name.compare(constraint_fname) == 0) {
              return_lattice_confidence = ConfidenceLattice::Meet(
                  callee_confidence, block_intersection_confidence);
              LOG(INFO) << "Returned function same as constraining function, "
                        << "performing meet: " << return_lattice_confidence;
            }

            propagate_callee = callee_name;
            // If we return a call instruction (in this case indirectly), we
            // take the callee function's confidence.
            LOG(INFO) << "PropagationIndirect"
                      << " f=" << parent_fname << *bb_last
                      << " fprime=" << constraint_fname << " constraint=\""
                      << block_constraint.lattice_element << "\""
                      << " E(fprime)=\"" << constraining_function_confidence
                      << "\""
                      << " g=\"" << propagate_callee << "\""
                      << " E(g)=\"" << callee_confidence;

            if (ReturnsDomainKnowledgeCodes(propagate_callee)) {
              AddFunctionReturningDomainKnowledgeCodes(parent_fname);
            }
          }
          if (ShouldSmartDropZero(parent_fname, function_fname)) {
            auto downgraded_lattice_confidence = ConfidenceLattice::Difference(
                return_lattice_confidence,
                SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
            if (downgraded_lattice_confidence != return_lattice_confidence) {
              LOG(INFO)
                  << "Downgraded indirectly propagated lattice element to "
                  << downgraded_lattice_confidence;
            }
            return_lattice_confidence = downgraded_lattice_confidence;
          }
        }
      }
    }
    if (ConfidenceLattice::IsUnknown(return_lattice_confidence)) {
      continue;
    }

    // Join abstraction of return value with parent AERV.
    join_result =
        ConfidenceLattice::Join(return_lattice_confidence, join_result);
  }

  return join_result;
}

LatticeElementConfidence ErrorBlocksPass::VisitCallInst(
    const llvm::CallInst &call_inst) {
  const std::string callee_name = GetCalleeSourceName(call_inst);
  const llvm::Function *parent = call_inst.getFunction();
  const std::string &function_fname = GetSourceFileName(call_inst);
  // If the callee is in our list of reachable functions, then add the caller
  // as well.
  if (!IsDoomedFunction(callee_name)) {
    AddNonDoomedFunction(*parent);
  }

  // If the function is not error-only and has an emptyset error
  // specification, then we just return emptyset. Else we need to set the
  // emptyset confidence to kMinConfidence in order to have the resulting
  // inferred error-specification for the parent function not be inferred as
  // emptyset incorrectly.
  if (!IsErrorOnlyFunctionCall(call_inst)) {
    int emptyset_confidence = kMinConfidence;
    const auto val = llvm::dyn_cast<llvm::Value>(&call_inst);
    // If the callee name is empty, then there is a possibility that the
    // callee is coming from a function pointer stored in a struct.
    if (callee_name.empty() && val && val->getType()->isIntOrPtrTy()) {
      emptyset_confidence = kMinConfidence;
      // If the function name is the empty string, then it is likely a LLVM
      // intrinsic. We should return emptyset in this case. If the call is
      // just related to an emptyset specification, then we should also return
      // emptyset here as well.
    } else if (callee_name.empty() || ConfidenceLattice::IsEmptyset(
                                          GetErrorSpecification(call_inst))) {
      emptyset_confidence = kMaxConfidence;
    }
    return LatticeElementConfidence(kMinConfidence, kMinConfidence,
                                    kMinConfidence, emptyset_confidence);
  }
  // If a block contains a call to an error only function, then get the set of
  // values that may be returned if that block executed. Add those values to
  // the error values for the function.
  ReturnedValuesPass &returned_values_pass = getAnalysis<ReturnedValuesPass>();

  // Get set of values that can be returned from this instruction.
  ReturnedValuesFact rtf = returned_values_pass.GetInFact(&call_inst);

  LatticeElementConfidence join_result(kMinConfidence, kMinConfidence,
                                       kMinConfidence, kMaxConfidence);
  for (const auto &v : rtf.value) {
    if (const auto maybe_bool = ExtractBoolean(*v)) {
      LatticeElementConfidence delta;
      if (*maybe_bool) {
        delta = ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
            SignLatticeElement::SIGN_LATTICE_ELEMENT_NOT_ZERO);
      } else {
        delta = ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
            SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);
      }
      join_result = ConfidenceLattice::Join(delta, join_result);
      LOG(INFO) << "ErrorOnlyCallBool eo=" << call_inst
                << " c=" << (*maybe_bool ? "true" : "false");
    } else if (const llvm::ConstantInt *int_return =
                   llvm::dyn_cast<llvm::ConstantInt>(v)) {
      const int64_t return_value = int_return->getSExtValue();
      if (!IsSuccessCode(GetSourceName(*parent), return_value,
                         function_fname)) {
        join_result = ConfidenceLattice::Join(
            AddErrorValue(parent, return_value), join_result);
        LOG(INFO) << "ErrorOnlyCallInt eo=" << call_inst
                  << " c=" << return_value;
      } else {
        LOG(INFO) << "ErrorOnlyCallInt eo=" << call_inst
                  << " c=" << return_value << " (success code)";
      }
    } else if (llvm::isa<llvm::ConstantPointerNull>(v)) {
      join_result = ConfidenceLattice::Join(
          AddErrorValue(parent, /*return_value*/ 0), join_result);
      LOG(INFO) << "ErrorOnlyCallPointer eo=" << call_inst << " c=0";
    }
  }
  return join_result;
}

LatticeElementConfidence ErrorBlocksPass::AddErrorValue(const llvm::Function *f,
                                                        int64_t v) {
  // If the function pointer is NULL, then it is likely a LLVM intrinsic.
  // These are emptyset. If the function is error-only, then it is definitely
  // emptyset.
  if (!f || IsErrorOnlyFunction(f))
    return LatticeElementConfidence(kMinConfidence, kMinConfidence,
                                    kMinConfidence, kMaxConfidence);

  // Insert constant into error_return_values_.
  if (error_return_values_.find(f) == error_return_values_.end()) {
    error_return_values_[f] = std::unordered_set<int64_t>({v});
  } else {
    error_return_values_[f].insert(v);
  }

  auto confidence_zero = kMinConfidence;
  auto confidence_less_than_zero = kMinConfidence;
  auto confidence_greater_than_zero = kMinConfidence;
  // We set confidence values according to actual error value. Simply passing
  // kMaxConfidence as a confidence for all lattice elements may potentially
  // cause a confidence to get updated to kMaxConfidence incorrectly if the
  // error specification for a function already has a confidence.
  if (v == 0) {
    confidence_zero = kMaxConfidence;
  } else if (v > 0) {
    confidence_greater_than_zero = kMaxConfidence;
  } else {
    confidence_less_than_zero = kMaxConfidence;
  }

  LatticeElementConfidence lattice_confidence(
      confidence_zero, confidence_less_than_zero, confidence_greater_than_zero);
  return lattice_confidence;
}

bool ErrorBlocksPass::AddNonDoomedFunction(const llvm::Function &f) {
  return AddNonDoomedFunction(GetSourceName(f));
}

bool ErrorBlocksPass::AddNonDoomedFunction(const std::string &function_name) {
  return non_doomed_function_names_.insert(function_name).second;
}

bool ErrorBlocksPass::IsDoomedFunction(const llvm::Function &f) {
  return IsDoomedFunction(GetSourceName(f));
}

bool ErrorBlocksPass::IsDoomedFunction(const std::string &function_name) {
  return non_doomed_function_names_.find(function_name) ==
         non_doomed_function_names_.end();
}

GetSpecificationsResponse ErrorBlocksPass::GetSpecifications() const {
  GetSpecificationsResponse response;
  for (const auto &function_lattice_confidence : error_specifications_) {
    // Skip "unknown" error specifications since the default assumption is
    // that function specifications are unknown when not reported by the
    // analysis.
    if (ConfidenceLattice::IsUnknown(function_lattice_confidence.second)) {
      continue;
    }

    // Copying the inferred specifications to the response.
    const std::string &llvm_name = function_lattice_confidence.first;
    const std::string &source_name = LlvmToSourceName(llvm_name);
    FunctionReturnType return_type =
        FunctionReturnType::FUNCTION_RETURN_TYPE_OTHER;
    if (function_return_types_.find(source_name) !=
        function_return_types_.end()) {
      return_type = function_return_types_.find(source_name)->second;
    }

    LOG(INFO) << "Function: " << source_name
              << " spec: " << function_lattice_confidence.second;
    // Enforce invariant initial specifications from domain knowledge.
    auto initial_spec_it = initial_error_specifications_.find(source_name);
    if (initial_spec_it != initial_error_specifications_.end()) {
      assert(initial_spec_it->second == function_lattice_confidence.second);
    }

    SignLatticeElement lattice_element =
        ConfidenceLattice::LatticeElementConfidenceToSignLatticeElement(
            function_lattice_confidence.second);
    Function f;
    f.set_llvm_name(llvm_name);
    f.set_source_name(source_name);
    f.set_return_type(return_type);
    Specification *s = response.add_specifications();
    s->mutable_function()->CopyFrom(f);
    s->set_lattice_element(lattice_element);
    s->set_confidence_zero(
        function_lattice_confidence.second.GetConfidenceZero());
    s->set_confidence_less_than_zero(
        function_lattice_confidence.second.GetConfidenceLessThanZero());
    s->set_confidence_greater_than_zero(
        function_lattice_confidence.second.GetConfidenceGreaterThanZero());
    s->set_confidence_emptyset(
        function_lattice_confidence.second.GetConfidenceEmptyset());
  }

  std::vector<Violation> violations = checker_->GetViolations();
  // Copying over all found violations to the response.
  for (auto &violation : violations) {
    response.add_violations()->CopyFrom(violation);
  }

  return response;
}

SignLatticeElement ErrorBlocksPass::AbstractInteger(int64_t v) const {
  if (v < 0) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO;
  } else if (v > 0) {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO;
  } else {
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO;
  }
}

std::unordered_set<std::string> ErrorBlocksPass::GetNonDoomedFunctions() const {
  return non_doomed_function_names_;
}

LatticeElementConfidence ErrorBlocksPass::GetErrorSpecification(
    const llvm::CallInst &call_inst) const {
  std::string function_name = GetCalleeSourceName(call_inst);
  if (function_name.empty()) {
    // All confidence values are 0 by default.
    return LatticeElementConfidence();
  }
  return GetErrorSpecification(function_name);
}

LatticeElementConfidence ErrorBlocksPass::GetErrorSpecification(
    const llvm::Function *fn) const {
  assert(fn != nullptr);
  return GetErrorSpecification(GetSourceName(*fn));
}

LatticeElementConfidence ErrorBlocksPass::GetErrorSpecification(
    const std::string &function_name) const {
  ErrorSpecificationMap::const_iterator it =
      error_specifications_.find(function_name);
  if (it != error_specifications_.end()) {
    return it->second;
  }
  // All confidence values are 0 by default.
  return LatticeElementConfidence();
}

bool ErrorBlocksPass::UpdateErrorSpecification(std::string func_name,
                                               LatticeElementConfidence delta) {
  llvm::Function *func = module_->getFunction(func_name);
  return UpdateErrorSpecification(func, delta);
}

bool ErrorBlocksPass::UpdateErrorSpecification(const llvm::Function *func,
                                               LatticeElementConfidence delta) {
  if (IgnoreFunction(func)) return false;

  // Constrain the delta to the function's return range.  If the function is
  // external, then we assume a default range of top.  This is necessary
  // because indirect propagation and embedding expansion can cause values
  // that can't be returned to be inferred.
  const auto &return_range_pass = getAnalysis<ReturnRangePass>();
  const auto return_range = return_range_pass.GetReturnRange(
      *func, /*default=*/SignLatticeElement::SIGN_LATTICE_ELEMENT_TOP);
  delta = ConfidenceLattice::Intersection(delta, return_range);

  // Join the current specification (if one exists) with the delta. We perform
  // this here because otherwise we could perform a Join later that ends up
  // making the delta equal to the return range if an error specification is
  // already mapped.
  // TODO(patrickjchap): The condition in the comment above will not exist for
  // the way that the expansion is currently performed. However, if we decide
  // to do something like expanding on all return values, then this is
  // necessary. Explained here: https://github.com/95616ARG/indra/issues/785
  std::string function_name = GetSourceName(*func);
  const auto &current = GetErrorSpecification(function_name);
  // Check if current is currently bottom. If delta is emptyset, then joining
  // with bottom/unknown would cause the delta to become bottom/unknown as
  // well.
  if (!ConfidenceLattice::IsUnknown(current)) {
    delta = ConfidenceLattice::Join(current, delta);
  }

  // If the LatticeElementConfidence matches the return range where each
  // element is threshold on kMaxConfidence, then we should not infer
  // anything. Inferring the entire return range with kMaxConfidence means
  // that something in the analysis is incorrect.
  if (ConfidenceLattice::MaxEquals(delta, return_range)) return false;

  // If the LatticeElementConfidence matches the return range where the
  // threshold is simply limited to above kMinConfidence, then we should only
  // keep the lattice elements that are kMaxConfidence and remove all other
  // confidences, that way the delta is less than the return range. If all
  // lattice elements are less than kMaxConfidence, then all confidences will
  // be zeroed out and the IsUnknown() check below will cause this function
  // to return false.
  if (ConfidenceLattice::Equals(delta, return_range)) {
    delta = ConfidenceLattice::RemoveLowestNonMin(delta);
    // Retest in case it is not possible to remove the lowest non-min.
    if (ConfidenceLattice::Equals(delta, return_range)) {
      delta = ConfidenceLattice::KeepIfMax(delta);
    }
  }

  // Joining with a LatticeElementConfidence where all confidence values are
  // kMinConfidence, i.e. "unknown", should not cause any change to the
  // specification mapping. This also prevents unnecessarily adding "unknown"
  // function LatticeElementConfidences to the specification mapping.
  if (ConfidenceLattice::IsUnknown(delta)) return false;

  // Set error specification to delta, which should now be the old
  // specification joined with the delta with any of confidence values
  // modified from KeepIfMax and RemoveLowestNonMin.
  error_specifications_[function_name] = delta;
  LOG(INFO) << "Updated: " << function_name << " spec: " << delta;
  return current != delta;
}

bool ErrorBlocksPass::RemoveFromErrorSpecification(
    const std::string &function_name, LatticeElementConfidence to_remove) {
  auto it = error_specifications_.find(function_name);
  if (it != error_specifications_.end()) {
    auto current = it->second;
    auto updated = ConfidenceLattice::Difference(current, to_remove);

    if (ConfidenceLattice::IsUnknown(updated)) {
      error_specifications_.erase(function_name);
    } else {
      it->second = updated;
    }

    return current != updated;
  }
  return false;
}

bool ErrorBlocksPass::ValuesAreEqual(
    const llvm::Value &llvm_value, const ConstantValue &constant_value) const {
  switch (constant_value.value_case()) {
    case ConstantValue::ValueCase::kIntValue: {
      if (llvm::isa<llvm::ConstantPointerNull>(llvm_value)) {
        return constant_value.int_value() == 0;
      } else {
        const auto *llvm_int = llvm::dyn_cast<llvm::ConstantInt>(&llvm_value);
        return llvm_int && llvm_int->equalsInt(constant_value.int_value());
      }
    }
    case ConstantValue::ValueCase::kStringValue: {
      const auto maybe_string_arg = ExtractStringLiteral(llvm_value);
      return maybe_string_arg &&
             maybe_string_arg->str() == constant_value.string_value();
    }
    case ConstantValue::ValueCase::VALUE_NOT_SET:
      return false;
  }

  assert(false);  // Shouldn't reach here if all enum cases are handled
}

bool ErrorBlocksPass::IsErrorOnlyFunctionCall(
    const llvm::CallInst &call_inst) const {
  const auto callee_name = GetCalleeSourceName(call_inst);

  if (error_only_functions_.count(callee_name) == 0) {
    return false;  // No error-only definition for this callee
  }

  // Iterate through this callee's error-only definitions
  auto error_only_definitions_ = error_only_functions_.equal_range(callee_name);
  for (auto it = error_only_definitions_.first;
       it != error_only_definitions_.second; ++it) {
    const auto &required_args = it->second;
    bool all_args_matched = true;

    if (required_args.empty()) {
      return true;  // No required args means the callee is always error-only
    }

    // Check if the call arguments match the error-only definition's arguments
    for (const auto &required_arg : required_args) {
      const auto position = required_arg.first;
      const auto required_value = required_arg.second;
      const auto actual_value = call_inst.getOperand(position);

      if (!ValuesAreEqual(*actual_value, required_value)) {
        // No match, so examine the callee's next error-only definition
        all_args_matched = false;
        break;
      }
    }
    if (all_args_matched) {
      return true;
    }
  }

  return false;
}

bool ErrorBlocksPass::IsErrorOnlyFunction(const llvm::Function *func) const {
  return error_only_functions_.count(GetSourceName(*func)) > 0;
}

bool ErrorBlocksPass::IsErrorCode(int64_t value,
                                  const std::string &filename) const {
  auto it = error_codes_.find(value);
  if (it == error_codes_.end()) {
    return false;
  }
  // No submodules are set, error codes are considered for all functions.
  if (it->second.size() == 0) {
    return true;
  }
  for (auto submodule_name : it->second) {
    if (filename.find(submodule_name) != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool ErrorBlocksPass::IsSuccessCode(int64_t value,
                                    const std::string &filename) const {
  auto it = success_codes_.find(value);
  if (it == success_codes_.end()) {
    return false;
  }
  // No submodules are set, error codes are considered for all functions.
  if (it->second.size() == 0) {
    return true;
  }
  for (auto submodule_name : it->second) {
    if (filename.find(submodule_name) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void ErrorBlocksPass::AddFunctionReturningDomainKnowledgeCodes(
    const std::string &function_name) {
  functions_returning_domain_knowledge_codes_.insert(function_name);
}

bool ErrorBlocksPass::ReturnsDomainKnowledgeCodes(
    const std::string &function_name) const {
  return functions_returning_domain_knowledge_codes_.count(function_name) > 0;
}

bool ErrorBlocksPass::IsSuccessCode(const std::string &function_name,
                                    const int64_t value,
                                    const std::string &filename) const {
  if (smart_success_code_zero_ && value == 0) {
    return ShouldSmartDropZero(function_name, filename);
  } else {
    return IsSuccessCode(value, filename);
  }
}

bool ErrorBlocksPass::ShouldSmartDropZero(const std::string &function_name,
                                          const std::string &filename) const {
  // Here, we apply a heuristic, since returning 0 is a bit complicated due to
  // ambiguity.  It might be a domain knowledge success code, or the current
  // function might return e.g. 0 on error and 1 on success. However, if a
  // function returns a domain knowledge status code, then since the domain
  // knowledge success and error codes form a collective set of return codes,
  // we know 0 must be a success return.
  return smart_success_code_zero_ && IsSuccessCode(0, filename) &&
         ReturnsDomainKnowledgeCodes(function_name);
}

void ErrorBlocksPass::getAnalysisUsage(llvm::AnalysisUsage &au) const {
  au.addRequired<ReturnPropagationPass>();
  au.addRequired<ReturnedValuesPass>();
  au.addRequired<ReturnConstraintsPass>();
  au.addRequired<ReturnRangePass>();
  au.setPreservesAll();
}

char ErrorBlocksPass::ID = 0;
static llvm::RegisterPass<ErrorBlocksPass> X(
    "errorblocks", "Map each basic block to its error state", false, false);

}  // namespace error_specifications
