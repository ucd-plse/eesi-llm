#include "glog/logging.h"
#include "include/grpcpp/grpcpp.h"
#include "llama_model.h"
#include "proto/eesi.grpc.pb.h"
#include "synonym_finder.h"

namespace error_specifications {

LlamaModel::LlamaModel {
  std::shared_ptr<grpc::Channel> channel;
  channel = grpc::CreateChannel("localhost:50058",
                                grpc::InsecureChannelCredentials());

  if (!channel) {
    // const std::string &err_msg = "Unable to connect to embedding service.";
    // LOG(ERROR) << err_msg;
    stub_ = nullptr;
    return;
  }
  stub_ = LlamaService::NewStub(channel);
}  // namespace error_specifications

SignLatticeElement LlamaModel::GetSpecification(
    std::string function_name, std::vector<Specification> specifications) {
  if (!stub_) return SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;

  GetLlamaSpecificationRequest request;
  request.mutable_function_name()->set_function_name(function_name);
  request.mutable_ctags_file()->set_ctags_file(ctags_file_);
  *request.mutable_error_specifications() = {specifications.begin(),
                                             specifications.end()};

  GetLlamaSpecificationResponse response;
  grpc::ClientContext context;
  grpc::Status status =
      stub_->GetLlamaSpecification(&context, request, &response);
  if (!status.ok()) {
    // This happens when a label does not exist in the model, which
    // can happen frequently (e.g. the function is never called).
    LOG(WARNING) << status.error_message();
    return SignLatticeElement::SIGN_LATTICE_ELEMENT_BOTTOM;
  }

  return response.lattice_element();
}  // namespace error_specifications
