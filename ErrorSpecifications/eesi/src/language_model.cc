#include "glog/logging.h"
#include "include/grpcpp/grpcpp.h"
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
    std::vector<std::pair<std::string, SignLatticeElement>> specifications,
    std::string function_definition) {
  if (!stub_) return std::vector<std::pair<std::string, float>>();

  GetMostSimilarRequest request;
  request.mutable_embedding_id()->CopyFrom(embedding_id_);
  request.mutable_label()->set_label(function_name);
  request.set_top_k(k);

  GetMostSimilarResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub_->GetMostSimilar(&context, request, &response);
  if (!status.ok()) {
    // This happens when a label does not exist in the model, which
    // can happen frequently (e.g. the function is never called).
    LOG(WARNING) << status.error_message();
    return std::vector<std::pair<std::string, float>>();
  }
  std::vector<std::pair<std::string, float>> func_synonyms;
  // Filter out functions that are less than the threshold.
  for (auto i = 0; i < response.labels_size(); i++) {
    // if (response.similarities(i) >= threshold) {
    func_synonyms.push_back(
        std::make_pair(response.labels(i).label(), response.similarities(i)));
    //}
  }
  return func_synonyms;
}  // namespace error_specifications

std::vector<std::string> SynonymFinder::GetGPTSpecification() {
  if (!stub_) return std::vector<std::string>();

  GetVocabularyRequest request;
  request.mutable_embedding_id()->CopyFrom(embedding_id_);

  GetVocabularyResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub_->GetVocabulary(&context, request, &response);

  // This call should never fail as GetVocabulary is only called when we have
  // a valid SynonymFinder that has a registered embedding.
  assert(status.ok());

  return std::vector<std::string>(response.function_labels().begin(),
                                  response.function_labels().end());
}

}  // namespace error_specifications
