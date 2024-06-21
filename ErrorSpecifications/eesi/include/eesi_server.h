// This file defines the C++ API for the EesiService gRPC calls.
// See proto/eesi.proto for details about individual rpc calls.

#ifndef ERROR_SPECIFICATIONS_EESI_INCLUDE_EESI_SERVER_H_
#define ERROR_SPECIFICATIONS_EESI_INCLUDE_EESI_SERVER_H_

#include <string>

#include "tbb/task.h"

#include "operations_service.h"
#include "proto/eesi.grpc.pb.h"
#include "proto/operations.grpc.pb.h"

namespace error_specifications {

// Logic and data behind the server's behavior.
class EesiServiceImpl final : public EesiService::Service {
  grpc::Status GetSpecifications(grpc::ServerContext *context,
                                 const GetSpecificationsRequest *request,
                                 Operation *operation) override;

  grpc::Status GetErrorHandlers(grpc::ServerContext *context,
                                const GetErrorHandlersRequest *request,
                                Operation *operation) override;

 public:
  // Because TBB can throw exceptions.
  ~EesiServiceImpl() throw() {}

  // The operations service for this EESI service.
  OperationsServiceImpl operations_service;
};

// This is a TBB task that runs EESI specification inference on bitcode
// id. The Bitcode file is retrieved from the bitcode service and
// the operations service is updated when the task is complete.
class GetSpecificationsTask : public tbb::task {
 public:
  tbb::task *execute(void);

  std::string task_name;
  GetSpecificationsRequest request;
  OperationsServiceImpl *operations_service;
  std::string bitcode_server_address;
};

void RunEesiServer(const std::string &eesi_server_address);

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_EESI_INCLUDE_EESI_SERVER_H_
