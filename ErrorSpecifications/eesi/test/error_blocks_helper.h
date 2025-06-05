#ifndef ERROR_SPECIFICATIONS_EESI_TEST_ERROR_BLOCKS_HELPER_H_
#define ERROR_SPECIFICATIONS_EESI_TEST_ERROR_BLOCKS_HELPER_H_

#include <memory>

#include "error_blocks_pass.h"
#include "mock_synonym_finder.h"

namespace error_specifications {

// The k-value is the constant that is used in EESIER's synonym finder when
// calling MostSimilar(). The k-value is multiplied by the minimum evidence
// number supplied by the user, which is then passed as the top-n function
// synonyms to return from the embedding. E.g. a minimum evidence of 3 and a
// k-value of 20 would result in 60 function synonym labels being returned by
// MostSimilar() (ignoring various applied filters).
constexpr int kKVal = 20;

// Runs the error blocks pass for a bitcode file given a
// GetSpecificationsRequest and a bitcode file from a given file path. This
// function also takes in a MockSynonymFinder to test F2V.
GetSpecificationsResponse RunErrorBlocks(const std::string &bitcode_path,
                                         const GetSpecificationsRequest &req,
                                         MockSynonymFinder *mock_sf);

// Runs the error blocks pass for a bitcode file given a
// GetSpecificationsRequest and a bitcode file from a given file path.
GetSpecificationsResponse RunErrorBlocks(const std::string &bitcode_path,
                                         const GetSpecificationsRequest &req);

// Runs the error blocks pass and returns the GetSpecificationsResponse and the
// set of non-doomed functions for a bitcode file given a
// GetSpecificationsRequest and a bitcode file from a given file path. This
// function also takes in a MockSynonymFinder to test F2V.
std::pair<GetSpecificationsResponse, std::unordered_set<std::string>>
RunErrorBlocksAndGetNonDoomedFunctions(const std::string &bitcode_path,
                                       const GetSpecificationsRequest &req,
                                       MockSynonymFinder *mock_sf);

// Runs the error blocks pass and returns the GetSpecificationsResponse and the
// set of non-doomed functions for a bitcode file given a
// GetSpecificationsRequest and a bitcode file from a given file path.
std::pair<GetSpecificationsResponse, std::unordered_set<std::string>>
RunErrorBlocksAndGetNonDoomedFunctions(const std::string &bitcode_path,
                                       const GetSpecificationsRequest &req);

// Find the specification for a GetSpecificationsResponse with several entries
// and checks equality, NOT including the confidence score. This is done as
// the order of specifications is not guaranteed.
bool FindSpecification(const std::string &function_name,
                       const SignLatticeElement &lattice_element,
                       const GetSpecificationsResponse &res);

// Find the specification for a GetSpecificationsResponse with several entries
// and checks equality, including the confidence score of ==0, <0, >0, and
// emptyset according to supplied arguments.
bool FindSpecification(const std::string &function_name,
                       const SignLatticeElement &lattice_element,
                       const GetSpecificationsResponse &res,
                       const short confidence_zero,
                       const short confidence_less_than_zero,
                       const short confidence_greater_than_zero,
                       const short confidence_emptyset);

// Find the specification for a GetSpecificationsResponse with several entries
// and checks equality, including the confidence score of ==0, <0, and >0
// according to supplied arguments. The confidence_emptyset check is set to
// kMinConfidence by default in this implementation.
bool FindSpecification(const std::string &function_name,
                       const SignLatticeElement &lattice_element,
                       const GetSpecificationsResponse &res,
                       const short confidence_zero,
                       const short confidence_less_than_zero,
                       const short confidence_greater_than_zero);

// Returns the total number of specifications that are non-emptyset and not
// unknown (bottom).
int GetNonEmptySpecificationsCount(const GetSpecificationsResponse &res);

// Returns the total number of specifications that are emptyset with
// kMaxConfidence. We ignore confidence values less than that since we are
// testing the analysis inferring emptyset, not the embedding-based expansion
// inferring emptyset (which would be less than kMaxConfidence).
int GetEmptySpecificationsCount(const GetSpecificationsResponse &res);

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_TEST_ERROR_BLOCKS_HELPER_H_
