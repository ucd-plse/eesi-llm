// This file contains end-to-end tests of the Bitcode service that
// go through the gRPC interface.

#include "bitcode/include/bitcode_server.h"

#include <stdio.h>
#include <numeric>

#include "gtest/gtest.h"
#include "include/grpcpp/grpcpp.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm.h"
#include "proto/bitcode.grpc.pb.h"
#include "servers.h"

namespace error_specifications {

// Starts the gRPC server before each test and shuts it down after.
// Right now this relies on starting the server with an actual port.
// It would be nice to do a full end-to-end test without a port number
// if there is a way to open a channel directly.
class BitcodeServiceTest : public ::testing::Test {
 protected:
  BitcodeServiceImpl service_;
  grpc::ServerBuilder builder_;
  std::unique_ptr<grpc::Server> server_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<BitcodeService::Stub> stub_;
  std::unique_ptr<OperationsService::Stub> operations_stub_;

  void SetUp() override {
    constexpr char kTestBitcodeServerAddress[] = "localhost:60051";
    builder_.AddListeningPort(kTestBitcodeServerAddress,
                              grpc::InsecureServerCredentials());
    builder_.RegisterService(&service_);
    builder_.RegisterService(&service_.operations_service);
    server_ = builder_.BuildAndStart();
    channel_ = grpc::CreateChannel(kTestBitcodeServerAddress,
                                   grpc::InsecureChannelCredentials());
    stub_ = BitcodeService::NewStub(channel_);
    operations_stub_ = OperationsService::NewStub(channel_);
  }

  void TearDown() override { server_->Shutdown(); }
};

// Test registering a bitcode file that exists.
TEST_F(BitcodeServiceTest, RegisterHello) {
  RegisterBitcodeRequest req;
  RegisterBitcodeResponse res;
  grpc::ClientContext context;

  const Uri file_uri = FilePathToUri("testdata/programs/hello.ll");
  req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status = stub_->RegisterBitcode(&context, req, &res);

  ASSERT_EQ(status.error_code(), grpc::OK);
}

// Test registering a reg2mem bitcode file that exists.
TEST_F(BitcodeServiceTest, RegisterHelloReg2mem) {
  RegisterBitcodeRequest req;
  RegisterBitcodeResponse res;
  grpc::ClientContext context;

  const Uri file_uri = FilePathToUri("testdata/programs/hello-reg2mem.ll");
  req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status = stub_->RegisterBitcode(&context, req, &res);

  ASSERT_EQ(status.error_code(), grpc::OK);
}

// Test registering a bitcode file that doesn't exist.
TEST_F(BitcodeServiceTest, RegisterMissing) {
  RegisterBitcodeRequest req;
  RegisterBitcodeResponse res;
  grpc::ClientContext context;

  const Uri file_uri = FilePathToUri("thisfiledoesnotexistljfsdklsdfklsfjd");
  req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status = stub_->RegisterBitcode(&context, req, &res);

  ASSERT_EQ(status.error_code(), grpc::INVALID_ARGUMENT);
}

// Test that invalid handles are rejected.
TEST_F(BitcodeServiceTest, AnnotateBadHandle) {
  AnnotateRequest req;
  AnnotateResponse res;
  grpc::ClientContext context;
  Handle *bitcode_handle = req.mutable_bitcode_id();
  bitcode_handle->set_id("42");

  grpc::Status status = stub_->Annotate(&context, req, &res);

  ASSERT_EQ(status.error_code(), grpc::INVALID_ARGUMENT);
}

// Test that invalid handles are rejected.
TEST_F(BitcodeServiceTest, DefinedFunctionsBadHandle) {
  DefinedFunctionsRequest req;
  Operation operation;
  grpc::ClientContext context;
  Handle *bitcode_handle = req.mutable_bitcode_id();
  bitcode_handle->set_id("42");

  grpc::Status status = stub_->GetDefinedFunctions(&context, req, &operation);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req, &operation);
    ASSERT_EQ(status.error_code(), grpc::OK);
    ASSERT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }

  ASSERT_EQ(operation.error().code(), grpc::INVALID_ARGUMENT);
}

// Test that invalid handles are rejected.
TEST_F(BitcodeServiceTest, CalledFunctionsBadHandle) {
  CalledFunctionsRequest req;
  Operation operation;
  grpc::ClientContext context;
  Handle *bitcode_handle = req.mutable_bitcode_id();
  bitcode_handle->set_id("42");

  grpc::Status status = stub_->GetCalledFunctions(&context, req, &operation);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req, &operation);
    ASSERT_EQ(status.error_code(), grpc::OK);
    ASSERT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }

  ASSERT_EQ(operation.error().code(), grpc::INVALID_ARGUMENT);
}

// Test that CalledFunctions on an annotated Bitcode file works.
TEST_F(BitcodeServiceTest, AnnotatedCalledFunctions) {
  // Register the Bitcode file and get the returned handle
  RegisterBitcodeRequest register_req;
  RegisterBitcodeResponse register_res;
  grpc::ClientContext register_context;

  const Uri &file_uri = FilePathToUri("testdata/programs/foo_calls_bar.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Annotate the registered Bitcode file.
  AnnotateRequest annotate_req;
  AnnotateResponse annotate_res;
  grpc::ClientContext annotate_context;

  const Uri output_uri =
      FilePathToUri("/tmp/BitcodeServiceTestAnnotatedCalledFunctions.bc");
  annotate_req.mutable_output_uri()->CopyFrom(output_uri);

  annotate_req.mutable_bitcode_id()->CopyFrom(register_res.bitcode_id());
  status = stub_->Annotate(&annotate_context, annotate_req, &annotate_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Use EXPECT from now on to ensure that file is deleted.
  // Get called functions in annotated bitcode file.
  CalledFunctionsRequest called_req;
  Operation operation;
  grpc::ClientContext called_context;
  called_req.mutable_bitcode_id()->CopyFrom(annotate_res.bitcode_id());
  status = stub_->GetCalledFunctions(&called_context, called_req, &operation);
  EXPECT_EQ(status.error_code(), grpc::OK);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req, &operation);
    EXPECT_EQ(status.error_code(), grpc::OK);
    EXPECT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }

  // Get the results of the operation.
  CalledFunctionsResponse called_res;
  operation.response().UnpackTo(&called_res);

  EXPECT_EQ(called_res.called_functions_size(), 1);
  EXPECT_EQ(called_res.called_functions(0).function().llvm_name(), "bar");
  EXPECT_EQ(called_res.called_functions(0).function().source_name(), "bar");
  remove("/tmp/BitcodeServiceTestAnnotatedCalledFunctions.bc");
}

// Test that CalledFunctions on an annotated reg2mem Bitcode file works.
TEST_F(BitcodeServiceTest, AnnotatedCalledFunctionsReg2mem) {
  // Register the Bitcode file and get the returned handle
  RegisterBitcodeRequest register_req;
  RegisterBitcodeResponse register_res;
  grpc::ClientContext register_context;

  const Uri &file_uri =
      FilePathToUri("testdata/programs/foo_calls_bar-reg2mem.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Annotate the registered Bitcode file.
  AnnotateRequest annotate_req;
  AnnotateResponse annotate_res;
  grpc::ClientContext annotate_context;

  const Uri output_uri =
      FilePathToUri("/tmp/BitcodeServiceTestAnnotatedCalledFunctions.bc");
  annotate_req.mutable_output_uri()->CopyFrom(output_uri);

  annotate_req.mutable_bitcode_id()->CopyFrom(register_res.bitcode_id());
  status = stub_->Annotate(&annotate_context, annotate_req, &annotate_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Use EXPECT from now on to ensure that file is deleted.
  // Get called functions in annotated bitcode file.
  CalledFunctionsRequest called_req;
  Operation operation;
  grpc::ClientContext called_context;
  called_req.mutable_bitcode_id()->CopyFrom(annotate_res.bitcode_id());
  status = stub_->GetCalledFunctions(&called_context, called_req, &operation);
  EXPECT_EQ(status.error_code(), grpc::OK);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req, &operation);
    EXPECT_EQ(status.error_code(), grpc::OK);
    EXPECT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }

  // Get the results of the operation.
  CalledFunctionsResponse called_res;
  operation.response().UnpackTo(&called_res);

  EXPECT_EQ(called_res.called_functions_size(), 1);
  EXPECT_EQ(called_res.called_functions(0).function().llvm_name(), "bar");
  EXPECT_EQ(called_res.called_functions(0).function().source_name(), "bar");
  remove("/tmp/BitcodeServiceTestAnnotatedCalledFunctions.bc");
}

// Test that DefinedFunctions on an annotated Bitcode file works.
TEST_F(BitcodeServiceTest, AnnotatedDefinedFunctions) {
  // Register the Bitcode file and get the returned handle
  RegisterBitcodeRequest register_req;
  RegisterBitcodeResponse register_res;
  grpc::ClientContext register_context;

  const Uri &file_uri = FilePathToUri("testdata/programs/foo_calls_bar.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Annotate the registered Bitcode file
  AnnotateRequest annotate_req;
  AnnotateResponse annotate_res;
  grpc::ClientContext annotate_context;
  annotate_req.mutable_bitcode_id()->CopyFrom(register_res.bitcode_id());

  const Uri output_uri =
      FilePathToUri("/tmp/BitcodeServiceTestAnnotatedDefinedFunctions.bc");
  annotate_req.mutable_output_uri()->CopyFrom(output_uri);

  status = stub_->Annotate(&annotate_context, annotate_req, &annotate_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Use EXPECT from now on to ensure that file is deleted.
  // Get defined functions in annotated bitcode file
  DefinedFunctionsRequest defined_req;
  Operation operation;
  grpc::ClientContext defined_context;
  defined_req.mutable_bitcode_id()->CopyFrom(annotate_res.bitcode_id());
  status =
      stub_->GetDefinedFunctions(&defined_context, defined_req, &operation);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req, &operation);
    EXPECT_EQ(status.error_code(), grpc::OK);
    EXPECT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }

  // Get the results of the operation.
  DefinedFunctionsResponse defined_res;
  operation.response().UnpackTo(&defined_res);

  EXPECT_EQ(status.error_code(), grpc::OK);
  EXPECT_EQ(defined_res.functions_size(), 2);
  EXPECT_EQ(defined_res.functions(0).llvm_name(), "foo");
  EXPECT_EQ(defined_res.functions(0).source_name(), "foo");
  EXPECT_EQ(defined_res.functions(1).llvm_name(), "bar");
  EXPECT_EQ(defined_res.functions(1).source_name(), "bar");
  remove("/tmp/BitcodeServiceTestAnnotatedDefinedFunctions.bc");
}

// Test that DefinedFunctions on an annotated reg2mem Bitcode file works.
TEST_F(BitcodeServiceTest, AnnotatedDefinedFunctionsReg2mem) {
  // Register the Bitcode file and get the returned handle
  RegisterBitcodeRequest register_req;
  RegisterBitcodeResponse register_res;
  grpc::ClientContext register_context;

  const Uri &file_uri =
      FilePathToUri("testdata/programs/foo_calls_bar-reg2mem.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Annotate the registered Bitcode file
  AnnotateRequest annotate_req;
  AnnotateResponse annotate_res;
  grpc::ClientContext annotate_context;
  annotate_req.mutable_bitcode_id()->CopyFrom(register_res.bitcode_id());

  const Uri output_uri =
      FilePathToUri("/tmp/BitcodeServiceTestAnnotatedDefinedFunctions.bc");
  annotate_req.mutable_output_uri()->CopyFrom(output_uri);

  status = stub_->Annotate(&annotate_context, annotate_req, &annotate_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Use EXPECT from now on to ensure that file is deleted.
  // Get defined functions in annotated bitcode file
  DefinedFunctionsRequest defined_req;
  Operation operation;
  grpc::ClientContext defined_context;
  defined_req.mutable_bitcode_id()->CopyFrom(annotate_res.bitcode_id());
  status =
      stub_->GetDefinedFunctions(&defined_context, defined_req, &operation);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req, &operation);
    EXPECT_EQ(status.error_code(), grpc::OK);
    EXPECT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }

  // Get the results of the operation.
  DefinedFunctionsResponse defined_res;
  operation.response().UnpackTo(&defined_res);

  EXPECT_EQ(status.error_code(), grpc::OK);
  EXPECT_EQ(defined_res.functions_size(), 2);
  EXPECT_EQ(defined_res.functions(0).llvm_name(), "foo");
  EXPECT_EQ(defined_res.functions(0).source_name(), "foo");
  EXPECT_EQ(defined_res.functions(1).llvm_name(), "bar");
  EXPECT_EQ(defined_res.functions(1).source_name(), "bar");
  remove("/tmp/BitcodeServiceTestAnnotatedDefinedFunctions.bc");
}

// Test that downloaded bitcode file is not corrupt.
TEST_F(BitcodeServiceTest, DownloadBitcode) {
  // Register the Bitcode file and get the returned handle.
  grpc::ClientContext register_context;
  RegisterBitcodeResponse register_res;
  RegisterBitcodeRequest register_req;

  const Uri &file_uri = FilePathToUri("testdata/programs/hello.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Download the bitcode file and reassemble into a string buffer.
  grpc::ClientContext download_context;
  DownloadBitcodeRequest download_req;
  download_req.mutable_bitcode_id()->CopyFrom(register_res.bitcode_id());
  std::unique_ptr<grpc::ClientReader<DataChunk>> reader(
      stub_->DownloadBitcode(&download_context, download_req));
  std::vector<std::string> chunks;
  DataChunk chunk;
  while (reader->Read(&chunk)) {
    chunks.push_back(chunk.content());
  }
  std::string bitcode_bytes =
      std::accumulate(chunks.begin(), chunks.end(), std::string(""));

  // Initialize an LLVM MemoryBuffer.
  static std::unique_ptr<llvm::MemoryBuffer> buffer =
      llvm::MemoryBuffer::getMemBuffer(bitcode_bytes);

  // Verify that reassembled IR parses into an llvm Module.
  llvm::SMDiagnostic err;
  llvm::LLVMContext llvm_context;
  std::unique_ptr<llvm::Module> module(
      llvm::parseIR(buffer->getMemBufferRef(), err, llvm_context));

  // Assert that mod is not null pointer, i.e. bitcode parsing succeeded.
  ASSERT_TRUE(module.get());
}

// Test that the bitcode ID returned for a file is the sha256 hash.
TEST_F(BitcodeServiceTest, HashBitcodeId) {
  // Register the Bitcode file and get the returned handle.
  grpc::ClientContext register_context;
  RegisterBitcodeResponse register_res;
  RegisterBitcodeRequest register_req;

  const Uri &file_uri = FilePathToUri("testdata/programs/hello.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);
  ASSERT_EQ(register_res.bitcode_id().id(),
            "c7045c1c1c07a5c4cbee3dc56d92f7e3d2de19ad9c8f59936847ebc070b55c7b");
}

// Test that the bitcode ID returned for a file is the sha256 hash.
TEST_F(BitcodeServiceTest, HashBitcodeIdReg2mem) {
  // Register the Bitcode file and get the returned handle.
  grpc::ClientContext register_context;
  RegisterBitcodeResponse register_res;
  RegisterBitcodeRequest register_req;

  const Uri &file_uri = FilePathToUri("testdata/programs/hello-reg2mem.ll");
  register_req.mutable_uri()->CopyFrom(file_uri);

  grpc::Status status =
      stub_->RegisterBitcode(&register_context, register_req, &register_res);
  ASSERT_EQ(status.error_code(), grpc::OK);
  ASSERT_EQ(register_res.bitcode_id().id(),
            "931a2c9dd167f8db8b7d23bdeb1a5f5121025f2c2c6a2c351767bf42e68e8216");
}

}  // namespace error_specifications
