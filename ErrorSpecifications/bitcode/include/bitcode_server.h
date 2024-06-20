// This file defines the C++ API for the BitcodeService gRPC calls.
// See proto/bitcode.proto for details about individual rpc calls.

#ifndef ERROR_SPECIFICATIONS_BITCODE_SERVER_H
#define ERROR_SPECIFICATIONS_BITCODE_SERVER_H

#include <string>
#include <unordered_map>

#include "tbb/task.h"

#include "operations_service.h"
#include "proto/bitcode.grpc.pb.h"
#include "proto/operations.grpc.pb.h"
#include "servers.h"

namespace error_specifications {

constexpr int kChunkSize = 1048576;

// Logic and data behind the server's behavior.
class BitcodeServiceImpl final : public BitcodeService::Service {
  // A map from the ID to the file location of registered bitcode files.
  std::unordered_map<std::string, Uri> registered_bitcode_files_;

  grpc::Status RegisterBitcode(grpc::ServerContext *context,
                               const RegisterBitcodeRequest *request,
                               RegisterBitcodeResponse *response) override;

  grpc::Status Annotate(grpc::ServerContext *context,
                        const AnnotateRequest *request,
                        AnnotateResponse *response) override;

  grpc::Status GetDefinedFunctions(grpc::ServerContext *context,
                                   const DefinedFunctionsRequest *request,
                                   Operation *operation) override;

  grpc::Status GetCalledFunctions(grpc::ServerContext *context,
                                  const CalledFunctionsRequest *request,
                                  Operation *operation) override;

  grpc::Status GetLocalCalledFunctions(
      grpc::ServerContext *context, const LocalCalledFunctionsRequest *request,
      Operation *operation) override;

  grpc::Status GetFileCalledFunctions(grpc::ServerContext *context,
                                      const FileCalledFunctionsRequest *request,
                                      Operation *operation) override;

  grpc::Status DownloadBitcode(grpc::ServerContext *context,
                               const DownloadBitcodeRequest *request,
                               grpc::ServerWriter<DataChunk> *writer) override;

  // Implementation of RPC call RegisterBitocdeFile.
  // Input `uri` is URI of file to register.
  // Output parameter `out_bitcode_id` is generated bitcode ID.
  grpc::Status DoRegisterBitcodeFile(const Uri &uri,
                                     std::string *out_bitcode_id);

 public:
  // Given a bitcode handle, returns the associated file path.
  // Returns an empty string if the handle could not be found.
  grpc::Status GetBitcodeUriForHandle(const Handle &handle, Uri *out_uri) const;

  // The operations service for managing long-running tasks.
  OperationsServiceImpl operations_service;
};

// Handles setting up a task to execute a CalledFunctionsPass related to the
// CalledFunctionsRequest.
class GetCalledFunctionsTask : public tbb::task {
 public:
  tbb::task *execute(void);

  std::string task_name;
  CalledFunctionsRequest request;
  BitcodeServiceImpl *bitcode_service;
  OperationsServiceImpl *operations_service;
};

// Handles setting up a task to execute a LocalCalledFunctionsPass related
// to the LocalCalledFunctionsRequest.
class GetLocalCalledFunctionsTask : public tbb::task {
 public:
  tbb::task *execute(void);

  std::string task_name;
  LocalCalledFunctionsRequest request;
  BitcodeServiceImpl *bitcode_service;
  OperationsServiceImpl *operations_service;
};

// Handles setting up a task to execute a FileCalledFunctionsPass related to
// the FileCalledFunctionsRequest.
class GetFileCalledFunctionsTask : public tbb::task {
 public:
  tbb::task *execute(void);

  std::string task_name;
  FileCalledFunctionsRequest request;
  BitcodeServiceImpl *bitcode_service;
  OperationsServiceImpl *operations_service;
};

// Handles setting up a task to executed a DefinedFunctionsPass related to the
// DefinedFunctionsRequest.
class GetDefinedFunctionsTask : public tbb::task {
 public:
  tbb::task *execute(void);

  std::string task_name;
  DefinedFunctionsRequest request;
  BitcodeServiceImpl *bitcode_service;
  OperationsServiceImpl *operations_service;
};

// Start up the BitcodeService.
void RunBitcodeServer(std::string server_address);

}  // namespace error_specifications.

#endif
