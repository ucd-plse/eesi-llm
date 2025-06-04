#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_GPT_MODEL_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_GPT_MODEL_H_

#include "include/grpcpp/grpcpp.h"
#include "proto/gpt.grpc.pb.h"

namespace error_specifications {
class GptModel {
 public:
  GptModel(std::string llm_name, std::string ctags_file);
  virtual ~GptModel() {}
  std::unordered_map<std::string, SignLatticeElement> GetSpecification(
      std::string function_name, std::vector<Specification> specifications,
      std::unordered_map<std::string, SignLatticeElement> error_code_names,
      std::unordered_map<std::string, SignLatticeElement> success_code_names);
  std::unordered_map<std::string, SignLatticeElement>
  GetThirdPartySpecifications(
      std::vector<std::pair<std::string, std::string>> function_names,
      std::vector<Specification> specifications,
      std::unordered_map<std::string, SignLatticeElement> error_code_names,
      std::unordered_map<std::string, SignLatticeElement> success_code_names);
  bool IsLLMNameEmpty();

 private:
  std::unique_ptr<GptService::Stub> stub_;
  std::string ctags_file_;
  std::string llm_name_;
};

}  // namespace error_specifications
#endif
