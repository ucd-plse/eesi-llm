#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_LANGUAGE_MODEL_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_LANGUAGE_MODEL_H_

#include "proto/eesi.grpc.pb.h"

namespace error_specifications {
class LanguageModel {
 public:
  LanguageModel() {}
  virtual ~LanguageModel() {}

  virtual SignLatticeElement GetSpecification(
      std::string function_name, std::vector<Specification> specifications);
};
}  // namespace error_specifications
#endif
