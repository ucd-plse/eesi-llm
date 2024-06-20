#include "bitcode_server.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "glog/logging.h"
#include "include/grpcpp/grpcpp.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "tbb/task.h"

#include "annotate_pass.h"
#include "called_functions_pass.h"
#include "defined_functions_pass.h"
#include "file_called_functions_pass.h"
#include "local_called_functions_pass.h"
#include "servers.h"

namespace error_specifications {

tbb::task *GetCalledFunctionsTask::execute(void) {
  LOG(INFO) << task_name;

  Operation result;
  result.set_name(task_name);

  Handle request_handle = request.bitcode_id();
  Uri bitcode_uri;
  grpc::Status err =
      bitcode_service->GetBitcodeUriForHandle(request_handle, &bitcode_uri);
  if (!err.ok()) {
    LOG(ERROR) << "Unable to get bitcode URI for handle.";
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(err.error_code());
    error_pb_message->set_message(err.error_message());
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  std::string bitcode_bytes;
  ReadUriIntoString(bitcode_uri, bitcode_bytes);

  // Initialize an LLVM MemoryBuffer.
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Parse IR into an llvm Module.
  llvm::SMDiagnostic llvm_err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), llvm_err, llvm_context));
  if (!module) {
    llvm_err.print("server", llvm::errs());
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(grpc::StatusCode::DATA_LOSS);
    error_pb_message->set_message("Unable to read bitcode file.");
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  CalledFunctionsPass *called_functions_pass = new CalledFunctionsPass();
  llvm::legacy::PassManager pass_manager;
  pass_manager.add(called_functions_pass);
  pass_manager.run(*module);

  CalledFunctionsResponse response =
      called_functions_pass->GetCalledFunctions();

  result.set_done(1);

  // Packing into google.protobuf.Any
  result.mutable_response()->PackFrom(response);

  operations_service->UpdateOperation(task_name, result);

  return NULL;
}

tbb::task *GetLocalCalledFunctionsTask::execute(void) {
  LOG(INFO) << task_name;

  Operation result;
  result.set_name(task_name);

  Handle request_handle = request.bitcode_id();
  Uri bitcode_uri;
  grpc::Status err =
      bitcode_service->GetBitcodeUriForHandle(request_handle, &bitcode_uri);
  if (!err.ok()) {
    LOG(ERROR) << "Unable to get bitcode URI for handle.";
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(err.error_code());
    error_pb_message->set_message(err.error_message());
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  std::string bitcode_bytes;
  ReadUriIntoString(bitcode_uri, bitcode_bytes);

  // Initialize an LLVM MemoryBuffer.
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Parse IR into an llvm Module.
  llvm::SMDiagnostic llvm_err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), llvm_err, llvm_context));
  if (!module) {
    llvm_err.print("server", llvm::errs());
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(grpc::StatusCode::DATA_LOSS);
    error_pb_message->set_message("Unable to read bitcode file.");
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  LocalCalledFunctionsPass *local_called_functions_pass =
      new LocalCalledFunctionsPass();
  llvm::legacy::PassManager pass_manager;
  pass_manager.add(local_called_functions_pass);
  pass_manager.run(*module);

  LocalCalledFunctionsResponse response =
      local_called_functions_pass->GetLocalCalledFunctions();

  result.set_done(1);

  // Packing into google.protobuf.Any
  result.mutable_response()->PackFrom(response);

  operations_service->UpdateOperation(task_name, result);

  return NULL;
}

tbb::task *GetFileCalledFunctionsTask::execute(void) {
  LOG(INFO) << task_name;

  Operation result;
  result.set_name(task_name);

  Handle request_handle = request.bitcode_id();
  Uri bitcode_uri;
  grpc::Status err =
      bitcode_service->GetBitcodeUriForHandle(request_handle, &bitcode_uri);
  if (!err.ok()) {
    LOG(ERROR) << "Unable to get bitcode URI for handle.";
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(err.error_code());
    error_pb_message->set_message(err.error_message());
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  std::string bitcode_bytes;
  ReadUriIntoString(bitcode_uri, bitcode_bytes);

  // Initialize an LLVM MemoryBuffer.
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Parse IR into an llvm Module.
  llvm::SMDiagnostic llvm_err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), llvm_err, llvm_context));
  if (!module) {
    llvm_err.print("server", llvm::errs());
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(grpc::StatusCode::DATA_LOSS);
    error_pb_message->set_message("Unable to read bitcode file.");
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  FileCalledFunctionsPass *file_called_functions_pass =
      new FileCalledFunctionsPass();
  llvm::legacy::PassManager pass_manager;
  pass_manager.add(file_called_functions_pass);
  pass_manager.run(*module);

  FileCalledFunctionsResponse response =
      file_called_functions_pass->GetFileCalledFunctions();

  result.set_done(1);

  // Packing into google.protobuf.Any
  result.mutable_response()->PackFrom(response);

  operations_service->UpdateOperation(task_name, result);

  return NULL;
}

tbb::task *GetDefinedFunctionsTask::execute(void) {
  LOG(INFO) << task_name;

  Operation result;
  result.set_name(task_name);

  Handle request_handle = request.bitcode_id();
  Uri bitcode_uri;
  grpc::Status err =
      bitcode_service->GetBitcodeUriForHandle(request_handle, &bitcode_uri);
  if (!err.ok()) {
    LOG(ERROR) << "Unable to get bitcode URI for handle.";
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(err.error_code());
    error_pb_message->set_message(err.error_message());
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  std::string bitcode_bytes;
  ReadUriIntoString(bitcode_uri, bitcode_bytes);

  // Initialize an LLVM MemoryBuffer.
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Parse IR into an llvm Module.
  llvm::SMDiagnostic llvm_err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), llvm_err, llvm_context));
  if (!module) {
    llvm_err.print("server", llvm::errs());
    google::rpc::Status *error_pb_message = result.mutable_error();
    error_pb_message->set_code(grpc::StatusCode::DATA_LOSS);
    error_pb_message->set_message("Unable to read bitcode file.");
    result.set_done(1);
    operations_service->UpdateOperation(task_name, result);
    return NULL;
  }

  DefinedFunctionsPass *defined_functions_pass = new DefinedFunctionsPass();
  llvm::legacy::PassManager pass_manager;
  pass_manager.add(defined_functions_pass);
  pass_manager.run(*module);

  DefinedFunctionsResponse response =
      defined_functions_pass->get_defined_functions();

  result.set_done(1);

  // Packing into google.protobuf.Any
  result.mutable_response()->PackFrom(response);

  operations_service->UpdateOperation(task_name, result);

  return NULL;
}

grpc::Status BitcodeServiceImpl::RegisterBitcode(
    grpc::ServerContext *context, const RegisterBitcodeRequest *request,
    RegisterBitcodeResponse *response) {
  LOG(INFO) << "Registering " << request->uri();

  std::string bitcode_id;
  grpc::Status err = DoRegisterBitcodeFile(request->uri(), &bitcode_id);
  if (!err.ok()) {
    LOG(ERROR) << "Unable to register bitcode file.";
    return err;
  }

  Handle *bitcode_handle = response->mutable_bitcode_id();
  bitcode_handle->set_id(bitcode_id);

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::DoRegisterBitcodeFile(
    const Uri &uri, std::string *out_bitcode_id) {
  std::string bitcode_data_str;
  grpc::Status read_status = ReadUriIntoString(uri, bitcode_data_str);
  if (!read_status.ok()) {
    const std::string err_msg = "Unable to read bitcode file.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
  }

  // Hash the bitcode file and use that as unique identifier (handle).
  grpc::Status err = HashString(bitcode_data_str, *out_bitcode_id);
  if (!err.ok()) {
    LOG(ERROR) << "Unable to hash bitcode data.";
    return err;
  }

  registered_bitcode_files_[*out_bitcode_id] = uri;

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::GetBitcodeUriForHandle(const Handle &handle,
                                                        Uri *out_uri) const {
  std::string ret;
  Uri registered_uri;
  if (registered_bitcode_files_.find(handle.id()) ==
      registered_bitcode_files_.end()) {
    const std::string &err_msg = "Handle not registered.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
  }

  *out_uri = registered_bitcode_files_.at(handle.id());

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::Annotate(grpc::ServerContext *context,
                                          const AnnotateRequest *request,
                                          AnnotateResponse *response) {
  Handle request_handle = request->bitcode_id();

  Uri bitcode_uri;
  grpc::Status err = GetBitcodeUriForHandle(request_handle, &bitcode_uri);
  if (!err.ok()) {
    const std::string &err_msg = "Handle not registered.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
  }

  std::string bitcode_bytes;
  ReadUriIntoString(bitcode_uri, bitcode_bytes);

  // Initialize an LLVM MemoryBuffer.
  std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Parse IR into an llvm Module.
  llvm::SMDiagnostic llvm_err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), llvm_err, llvm_context));
  if (!module) {
    const std::string err_msg = "Unable to read bitcode file.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::DATA_LOSS, err_msg);
  }

  AnnotatePass *annotate_pass = new AnnotatePass();
  llvm::legacy::PassManager pass_manager;
  pass_manager.add(annotate_pass);
  pass_manager.run(*module);

  // Write out the annotated bitcode file to disk. Only writing to
  // local disk is supported currently.
  std::string output_path;
  err = ConvertUriToFilePath(request->output_uri(), output_path);
  if (!err.ok()) {
    return err;
  }
  std::error_code error_code;
  llvm::raw_fd_ostream ostream(output_path, error_code, llvm::sys::fs::F_None);
  if (error_code) {
    return grpc::Status(grpc::StatusCode::DATA_LOSS,
                        "Unable to write annotated bitcode file.");
  }
  llvm::WriteBitcodeToFile(*module, ostream);
  ostream.flush();

  std::string annotated_bitcode_id;
  err = DoRegisterBitcodeFile(request->output_uri(), &annotated_bitcode_id);
  if (!err.ok()) {
    return err;
  }

  Handle *bitcode_handle = response->mutable_bitcode_id();
  bitcode_handle->set_id(annotated_bitcode_id);

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::GetDefinedFunctions(
    grpc::ServerContext *context, const DefinedFunctionsRequest *request,
    Operation *operation) {
  LOG(INFO) << "GetDefinedFunctions RPC";

  // Return the name of the operation so client can check on progress.
  std::string task_name =
      GetTaskName("GetDefinedFunctions", request->bitcode_id().id());
  operation->set_name(task_name);
  operation->set_done(0);
  operations_service.UpdateOperation(task_name, *operation);

  GetDefinedFunctionsTask *task =
      new (tbb::task::allocate_root()) GetDefinedFunctionsTask();
  task->bitcode_service = this;
  task->operations_service = &operations_service;
  task->request = *request;
  task->task_name = task_name;
  tbb::task::enqueue(*task);
  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::GetCalledFunctions(
    grpc::ServerContext *context, const CalledFunctionsRequest *request,
    Operation *operation) {
  LOG(INFO) << "GetCalledFunctions RPC";

  // Return the name of the operation so client can check on progress.
  std::string task_name =
      GetTaskName("GetCalledFunctions", request->bitcode_id().id());
  operation->set_name(task_name);
  operation->set_done(0);
  operations_service.UpdateOperation(task_name, *operation);

  GetCalledFunctionsTask *task =
      new (tbb::task::allocate_root()) GetCalledFunctionsTask();
  task->bitcode_service = this;
  task->operations_service = &operations_service;
  task->request = *request;
  task->task_name = task_name;
  tbb::task::enqueue(*task);

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::GetLocalCalledFunctions(
    grpc::ServerContext *context, const LocalCalledFunctionsRequest *request,
    Operation *operation) {
  LOG(INFO) << "GetLocalCalledFunctions RPC";

  std::string task_name =
      GetTaskName("GetLocalCalledFunctions", request->bitcode_id().id());
  operation->set_name(task_name);
  operation->set_done(0);
  operations_service.UpdateOperation(task_name, *operation);

  GetLocalCalledFunctionsTask *task =
      new (tbb::task::allocate_root()) GetLocalCalledFunctionsTask();
  task->bitcode_service = this;
  task->operations_service = &operations_service;
  task->request = *request;
  task->task_name = task_name;
  tbb::task::enqueue(*task);

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::GetFileCalledFunctions(
    grpc::ServerContext *context, const FileCalledFunctionsRequest *request,
    Operation *operation) {
  LOG(INFO) << "GetFileCalledFunctions RPC";

  std::string task_name =
      GetTaskName("GetFileCalledFunctions", request->bitcode_id().id());
  operation->set_name(task_name);
  operation->set_done(0);
  operations_service.UpdateOperation(task_name, *operation);

  GetFileCalledFunctionsTask *task =
      new (tbb::task::allocate_root()) GetFileCalledFunctionsTask();
  task->bitcode_service = this;
  task->operations_service = &operations_service;
  task->request = *request;
  task->task_name = task_name;
  tbb::task::enqueue(*task);

  return grpc::Status::OK;
}

grpc::Status BitcodeServiceImpl::DownloadBitcode(
    grpc::ServerContext *context, const DownloadBitcodeRequest *request,
    grpc::ServerWriter<DataChunk> *writer) {
  LOG(INFO) << "DownloadBitcode-" << std::string(request->bitcode_id().id());

  Handle request_handle = request->bitcode_id();
  Uri uri = registered_bitcode_files_.at(request_handle.id());

  std::vector<char> bitcode_bytes;
  std::string bitcode_data_str;
  grpc::Status read_status = ReadUriIntoString(uri, bitcode_data_str);
  if (!read_status.ok()) {
    return read_status;
  }
  bitcode_bytes =
      std::vector<char>(bitcode_data_str.begin(), bitcode_data_str.end());

  auto chunk_begin = bitcode_bytes.begin();
  auto chunk_end = bitcode_bytes.begin();
  auto bytes_end = bitcode_bytes.end();
  do {
    if (std::distance(chunk_end, bytes_end) < kChunkSize) {
      chunk_end = bytes_end;
    } else {
      std::advance(chunk_end, kChunkSize);
    }
    DataChunk chunk;
    std::vector<char> chunk_bytes(chunk_begin, chunk_end);
    chunk.set_content(&chunk_bytes[0], chunk_bytes.size());
    writer->Write(chunk);
    chunk_begin = chunk_end;
  } while (std::distance(chunk_end, bytes_end) > 0);

  return grpc::Status::OK;
}

void RunBitcodeServer(std::string server_address) {
  BitcodeServiceImpl service;
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
