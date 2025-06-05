#include "error_blocks_helper.h"

#include <algorithm>
#include <memory>

#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

#include "error_blocks_pass.h"
#include "mock_synonym_finder.h"
#include "proto/eesi.pb.h"
#include "return_constraints_pass.h"
#include "return_propagation_pass.h"
#include "returned_values_pass.h"

namespace error_specifications {

GetSpecificationsResponse RunErrorBlocks(const std::string &bitcode_path,
                                         const GetSpecificationsRequest &req,
                                         MockSynonymFinder *mock_sf) {
  ErrorBlocksPass *error_blocks_pass = new ErrorBlocksPass();
  error_blocks_pass->SetSpecificationsRequest(req, mock_sf);

  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> mod(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  EXPECT_TRUE(mod != nullptr);

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(error_blocks_pass);
  pass_manager.run(*mod);

  return error_blocks_pass->GetSpecifications();
}

GetSpecificationsResponse RunErrorBlocks(const std::string &bitcode_path,
                                         const GetSpecificationsRequest &req) {
  return RunErrorBlocks(bitcode_path, req, nullptr);
}

// Runs the error blocks pass and returns the GetSpecificationsResponse.
std::pair<GetSpecificationsResponse, std::unordered_set<std::string>>
RunErrorBlocksAndGetNonDoomedFunctions(const std::string &bitcode_path,
                                       const GetSpecificationsRequest &req,
                                       MockSynonymFinder *mock_sf) {
  ErrorBlocksPass *error_blocks_pass = new ErrorBlocksPass();
  error_blocks_pass->SetSpecificationsRequest(req, mock_sf);

  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> mod(
      llvm::parseIRFile(bitcode_path, err, llvm_context));
  EXPECT_TRUE(mod != nullptr);

  llvm::legacy::PassManager pass_manager;
  pass_manager.add(error_blocks_pass);
  pass_manager.run(*mod);

  return std::make_pair(error_blocks_pass->GetSpecifications(),
                        error_blocks_pass->GetNonDoomedFunctions());
}

// Runs the error blocks pass without a mock synonym finder and returns the
// GetSpecificationsResponse and the reachable functions.
std::pair<GetSpecificationsResponse, std::unordered_set<std::string>>
RunErrorBlocksAndGetNonDoomedFunctions(const std::string &bitcode_path,
                                       const GetSpecificationsRequest &req) {
  return RunErrorBlocksAndGetNonDoomedFunctions(bitcode_path, req, nullptr);
}

bool FindSpecification(const std::string &function_name,
                       const SignLatticeElement &lattice_element,
                       const GetSpecificationsResponse &res) {
  // The confidence values will be automatically updated correctly because
  // of the call to UpdateConfidence() in the Constraint constructor.
  LatticeElementConfidence lattice_confidence =
      ConfidenceLattice::SignLatticeElementToLatticeElementConfidence(
          lattice_element);

  return FindSpecification(function_name, lattice_element, res,
                           lattice_confidence.GetConfidenceZero(),
                           lattice_confidence.GetConfidenceLessThanZero(),
                           lattice_confidence.GetConfidenceGreaterThanZero());
}

bool FindSpecification(const std::string &function_name,
                       const SignLatticeElement &lattice_element,
                       const GetSpecificationsResponse &res,
                       const short confidence_zero,
                       const short confidence_less_than_zero,
                       const short confidence_greater_than_zero) {
  return FindSpecification(function_name, lattice_element, res, confidence_zero,
                           confidence_less_than_zero,
                           confidence_greater_than_zero, kMinConfidence);
}

bool FindSpecification(const std::string &function_name,
                       const SignLatticeElement &lattice_element,
                       const GetSpecificationsResponse &res,
                       const short confidence_zero,
                       const short confidence_less_than_zero,
                       const short confidence_greater_than_zero,
                       const short confidence_emptyset) {
  for (int i = 0; i < res.specifications().size(); i++) {
    if (res.specifications(i).function().source_name() == function_name) {
      auto err_spec = res.specifications(i);
      EXPECT_EQ(err_spec.function().llvm_name(), function_name);
      EXPECT_EQ(err_spec.lattice_element(), lattice_element);
      EXPECT_EQ(err_spec.confidence_zero(), confidence_zero);
      EXPECT_EQ(err_spec.confidence_less_than_zero(),
                confidence_less_than_zero);
      EXPECT_EQ(err_spec.confidence_greater_than_zero(),
                confidence_greater_than_zero);
      EXPECT_EQ(err_spec.confidence_emptyset(), confidence_emptyset);
      return true;
    }
  }
  return false;
}

int GetNonEmptySpecificationsCount(const GetSpecificationsResponse &res) {
  return std::count_if(
      res.specifications().begin(), res.specifications().end(),
      [](const Specification &specification) {
        return specification.confidence_zero() > kMinConfidence ||
               specification.confidence_less_than_zero() > kMinConfidence ||
               specification.confidence_greater_than_zero() > kMinConfidence;
      });
}

int GetEmptySpecificationsCount(const GetSpecificationsResponse &res) {
  return std::count_if(res.specifications().begin(), res.specifications().end(),
                       [](const Specification &specification) {
                         return specification.confidence_emptyset() ==
                                kMaxConfidence;
                       });
}

}  // namespace error_specifications
