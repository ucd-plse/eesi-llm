#include "operations_service.h"

namespace error_specifications {

void OperationsServiceImpl::UpdateOperation(std::string operation_name,
                                            Operation operation) {
  OperationTable::accessor a;
  if (operation_progress_.find(a, operation_name)) {
    // Make sure that we are not undoing an operation status due to ordering.
    // Once an operation is finished it can never be unfinished.
    Operation existing = (Operation)a->second;
    if (existing.done() && !operation.done()) {
      return;
    }
    a->second = operation;
  } else {
    // The operation does not yet exist. Insert it.
    operation_progress_.insert(std::make_pair(operation_name, operation));
  }
}

grpc::Status OperationsServiceImpl::GetOperation(
    grpc::ServerContext *context, const GetOperationRequest *request,
    Operation *operation) {
  OperationTable::accessor a;
  if (!operation_progress_.find(a, request->name())) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
                        "Operation name not found.");
  }

  operation->CopyFrom((Operation)a->second);

  // If the operation is done, remove the key, i.e. do not cache results.
  if (operation->done()) {
    operation_progress_.erase(a);
  }

  return grpc::Status::OK;
}

grpc::Status OperationsServiceImpl::DeleteOperation(
    grpc::ServerContext *context, const DeleteOperationRequest *request,
    Operation *operation) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "");
}

grpc::Status OperationsServiceImpl::CancelOperation(
    grpc::ServerContext *context, const CancelOperationRequest *request,
    ::google::protobuf::Empty *response) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "");
}
}  // namespace error_specifications
