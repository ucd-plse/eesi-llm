#ifndef ERROR_SPECIFICATIONS_EESI_TEST_MOCK_SYNONYM_FINDER_H_
#define ERROR_SPECIFICATIONS_EESI_TEST_MOCK_SYNONYM_FINDER_H_

#include <memory>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "synonym_finder.h"

namespace error_specifications {

// SynonymFinder class uses a function embedding to find function synonyms.
class MockSynonymFinder : public SynonymFinder {
 public:
  // Have to typedef pair here as a macro expansion is likely causing the ','
  // to be interpreted incorrectly for std::pair in MOCK_METHOD.
  typedef std::vector<std::pair<std::string, float>> Synonyms;

  MOCK_METHOD(Synonyms, GetSynonyms,
              (const std::string &function_name, int k, float threshold),
              (override));

  MOCK_METHOD(std::vector<std::string>, GetVocabulary, (), (override));
};

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_TEST_MOCK_SYNONYM_FINDER_H_
