#include "gpt_model.h"

#include "glog/logging.h"
#include "include/grpcpp/grpcpp.h"
#include "proto/eesi.grpc.pb.h"

namespace error_specifications {

GptModel::GptModel(std::string ctags_file) {
  std::shared_ptr<grpc::Channel> channel;
  channel = grpc::CreateChannel("localhost:50059",
                                grpc::InsecureChannelCredentials());

  if (!channel) {
    // const std::string &err_msg = "Unable to connect to embedding service.";
    // LOG(ERROR) << err_msg;
    stub_ = nullptr;
    return;
  }
  stub_ = GptService::NewStub(channel);
  ctags_file_ = ctags_file;
}  // namespace error_specifications

std::unordered_map<std::string, SignLatticeElement>
GptModel::GetThirdPartySpecifications(
    std::vector<std::pair<std::string, std::string>> function_names,
    std::vector<Specification> specifications,
    std::unordered_map<std::string, SignLatticeElement> error_code_names,
    std::unordered_map<std::string, SignLatticeElement> success_code_names) {
  if (!stub_) return std::unordered_map<std::string, SignLatticeElement>();
  GetGptThirdPartySpecificationsRequest request;
  // I should change this so I don't have to convert here.
  std::unordered_map<std::string, std::string> function_names_map;
  for (auto p : function_names) {
    function_names_map[p.first] = p.second;
  }
  *request.mutable_error_specifications() = {specifications.begin(),
                                             specifications.end()};
  *request.mutable_function_names() = {function_names_map.begin(),
                                       function_names_map.end()};
  *request.mutable_error_code_names() = {error_code_names.begin(),
                                         error_code_names.end()};

  GetGptThirdPartySpecificationsResponse response;
  grpc::ClientContext context;
  grpc::Status status =
      stub_->GetGptThirdPartySpecifications(&context, request, &response);
  if (!status.ok()) {
    LOG(WARNING) << status.error_message();
    return std::unordered_map<std::string, SignLatticeElement>();
  }
  return std::unordered_map<std::string, SignLatticeElement>(
      response.specifications().begin(), response.specifications().end());
};

std::unordered_map<std::string, SignLatticeElement> GptModel::GetSpecification(
    std::string function_name, std::vector<Specification> specifications,
    std::unordered_map<std::string, SignLatticeElement> error_code_names,
    std::unordered_map<std::string, SignLatticeElement> success_code_names) {
  if (!stub_) return std::unordered_map<std::string, SignLatticeElement>();

  GetGptSpecificationRequest request;
  request.set_function_name(function_name);
  request.set_ctags_file(ctags_file_);
  *request.mutable_error_specifications() = {specifications.begin(),
                                             specifications.end()};
  *request.mutable_error_code_names() = {error_code_names.begin(),
                                         error_code_names.end()};
  *request.mutable_success_code_names() = {success_code_names.begin(),
                                           success_code_names.end()};

  GetGptSpecificationResponse response;
  grpc::ClientContext context;
  grpc::Status status =
      stub_->GetGptSpecification(&context, request, &response);
  if (!status.ok()) {
    // This happens when a label does not exist in the model, which
    // can happen frequently (e.g. the function is never called).
    LOG(WARNING) << status.error_message();
    return std::unordered_map<std::string, SignLatticeElement>();
  }

  return std::unordered_map<std::string, SignLatticeElement>(
      response.specifications().begin(), response.specifications().end());
}
}  // namespace error_specifications
