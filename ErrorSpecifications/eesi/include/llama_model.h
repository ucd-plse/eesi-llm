#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_LLAMA_MODEL_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_LLAMA_MODEL_H_

#include "include/grpcpp/grpcpp.h"
#include "proto/llama.grpc.pb.h"

namespace error_specifications {
class LlamaModel {
 public:
  LlamaModel(std::string ctags_file);
  virtual ~LlamaModel() {}
  std::unordered_map<std::string, SignLatticeElement> GetSpecification(
      std::string function_name, std::vector<Specification> specifications);

 private:
  std::unique_ptr<LlamaService::Stub> stub_;
  std::string ctags_file_;
};

}  // namespace error_specifications
#endif
