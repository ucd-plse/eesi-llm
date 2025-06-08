// This file defines the common operations service. The operations service
// is an interface or long running operations that is common to all of the
// server. It is not run as a separate service. Rather, each server that
// handles long-running tasks also runs an operations service.
//
// The operations service follows the way Google cloud APIs work
// (see operations.proto).

#ifndef ERROR_SPECIFICATIONS_COMMON_INCLUDE_OPERATIONS_SERVICE_H_
#define ERROR_SPECIFICATIONS_COMMON_INCLUDE_OPERATIONS_SERVICE_H_

#include <string>
#include <unordered_map>

#include "proto/operations.grpc.pb.h"
#include "tbb/concurrent_hash_map.h"

namespace error_specifications {

// Maps string operation task names to the current operation structure.
using OperationTable = tbb::concurrent_hash_map<std::string, Operation>;

// Logic and data behind the server's behavior.
class OperationsServiceImpl final : public OperationsService::Service {
  grpc::Status GetOperation(grpc::ServerContext *context,
                            const GetOperationRequest *request,
                            Operation *operation) override;

  grpc::Status DeleteOperation(grpc::ServerContext *context,
                               const DeleteOperationRequest *request,
                               Operation *operation) override;

  grpc::Status CancelOperation(grpc::ServerContext *context,
                               const CancelOperationRequest *request,
                               ::google::protobuf::Empty *response) override;

  // A map from operation names to the latest Operation message.
  OperationTable operation_progress_;

 public:
  // This is not part of the service API and is meant to be called
  // only from the service to update the progress of a running
  // operation.
  void UpdateOperation(std::string name, Operation operation);
};

}  // namespace error_specifications.

#endif  // ERROR_SPECIFICATIONS_COMMON_INCLUDE_OPERATIONS_SERVICE_H_
