#include "eesi_server.h"

#include <stdio.h>

#include <ctime>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "error_blocks_pass.h"
#include "glog/logging.h"
#include "include/grpcpp/grpcpp.h"
#include "llvm.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "operations_service.h"
#include "proto/bitcode.grpc.pb.h"
#include "proto/operations.grpc.pb.h"
#include "return_constraints_pass.h"
#include "return_propagation_pass.h"
#include "return_range_pass.h"
#include "returned_values_pass.h"
#include "servers.h"
#include "tbb/task.h"

namespace error_specifications {

tbb::task *GetSpecificationsTask::execute(void) {
  LOG(INFO) << task_name;

  Operation result;
  result.set_name(task_name);

  // Connect to the bitcode service
  std::shared_ptr<grpc::Channel> channel;
  std::unique_ptr<BitcodeService::Stub> stub;
  channel = grpc::CreateChannel(bitcode_server_address,
                                grpc::InsecureChannelCredentials());
  stub = BitcodeService::NewStub(channel);

  grpc::ClientContext download_context;
  DownloadBitcodeRequest download_req;
  download_req.mutable_bitcode_id()->CopyFrom(request.bitcode_id());
  std::unique_ptr<grpc::ClientReader<DataChunk>> reader(
      stub->DownloadBitcode(&download_context, download_req));
  std::vector<std::string> chunks;
  DataChunk chunk;
  while (reader->Read(&chunk)) {
    chunks.push_back(chunk.content());
  }

  std::string bitcode_bytes =
      std::accumulate(chunks.begin(), chunks.end(), std::string(""));

  // Initialize an LLVM MemoryBuffer.
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Parse IR into an llvm Module.
  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), err, llvm_context));

  if (!module) {
    err.print("eesi-server", llvm::errs());
    abort();
  }

  llvm::legacy::PassManager pass_manager;
  ReturnPropagationPass *return_propagation = new ReturnPropagationPass();
  ReturnConstraintsPass *return_constraints = new ReturnConstraintsPass();
  ReturnedValuesPass *returned_values = new ReturnedValuesPass();
  ReturnRangePass *return_range = new ReturnRangePass();
  ErrorBlocksPass *error_blocks = new ErrorBlocksPass();

  error_blocks->SetSpecificationsRequest(request);
  pass_manager.add(return_propagation);
  pass_manager.add(return_constraints);
  pass_manager.add(error_blocks);
  pass_manager.add(returned_values);
  pass_manager.add(return_range);

  pass_manager.run(*module);

  GetSpecificationsResponse get_specifications_response =
      error_blocks->GetSpecifications();

  result.set_done(1);

  // Packing into google.protobuf.Any
  result.mutable_response()->PackFrom(get_specifications_response);

  operations_service->UpdateOperation(task_name, result);
  return NULL;
}

grpc::Status EesiServiceImpl::GetSpecifications(
    grpc::ServerContext *context, const GetSpecificationsRequest *request,
    Operation *operation) {
  LOG(INFO) << "Getspecifications rpc";

  const std::string bitcode_server_address = request->bitcode_id().authority();
  if (bitcode_server_address.empty()) {
    const std::string &err_msg = "Authority missing in bitcode Handle.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
  }

  // Return the name of the operation so client can check on progress.
  std::string task_name =
      GetTaskName("GetSpecifications", request->bitcode_id().id());
  operation->set_name(task_name);
  operation->set_done(0);
  operations_service.UpdateOperation(task_name, *operation);

  GetSpecificationsTask *task =
      new (tbb::task::allocate_root()) GetSpecificationsTask();
  task->operations_service = &operations_service;
  task->request = *request;
  task->task_name = task_name;
  task->bitcode_server_address = bitcode_server_address;
  tbb::task::enqueue(*task);

  return grpc::Status::OK;
}

grpc::Status EesiServiceImpl::GetErrorHandlers(
    grpc::ServerContext *context, const GetErrorHandlersRequest *request,
    Operation *operation) {
  return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "");
}

void RunEesiServer(const std::string &server_address) {
  EesiServiceImpl service;

  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  builder.RegisterService(&service.operations_service);
  // Finally assemble the server.
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

}  // namespace error_specifications
