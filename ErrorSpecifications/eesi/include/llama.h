#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_LANGUAGE_MODEL_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_LANGUAGE_MODEL_H_

#include "proto/eesi.grpc.pb.h"

namespace error_specifications {
class Llama : public LanguageModel {
 public:
  LanguageModel() {}
  virtual ~LanguageModel() {}

  virtual GetSpecification(
      std::vector<std::pair<std::string, SignLatticeElement>> specifications,
      std::string function_definition);
}
}  // namespace error_specifications
#endif
