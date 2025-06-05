// This file contains end-to-end tests of the EESI service that
// go through the gRPC interface.

#include "eesi/include/eesi_server.h"

#include <unistd.h>

#include "gtest/gtest.h"
#include "include/grpcpp/grpcpp.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include "bitcode/include/bitcode_server.h"
#include "error_blocks_helper.h"
#include "llvm.h"
#include "proto/bitcode.grpc.pb.h"
#include "proto/eesi.grpc.pb.h"
#include "servers.h"

namespace error_specifications {

// Starts the gRPC server before each test and shuts it down after.
// Right now this relies on starting the server with an actual port.
class EesiServiceTest : public ::testing::Test {
 protected:
  BitcodeServiceImpl bitcode_service_;
  grpc::ServerBuilder bitcode_builder_;
  std::shared_ptr<grpc::Channel> bitcode_channel_;
  std::unique_ptr<grpc::Server> bitcode_server_;
  std::unique_ptr<BitcodeService::Stub> bitcode_stub_;

  EesiServiceImpl eesi_service_;
  grpc::ServerBuilder eesi_builder_;
  std::unique_ptr<grpc::Server> eesi_server_;
  std::shared_ptr<grpc::Channel> eesi_channel_;
  std::unique_ptr<EesiService::Stub> eesi_stub_;
  std::unique_ptr<OperationsService::Stub> operations_stub_;

  const std::string test_bitcode_server_address_ = "localhost:70051";

  void SetUp() override {
    // Start the bitcode service.
    bitcode_builder_.AddListeningPort(test_bitcode_server_address_,
                                      grpc::InsecureServerCredentials());
    bitcode_builder_.RegisterService(&bitcode_service_);
    bitcode_server_ = bitcode_builder_.BuildAndStart();
    bitcode_channel_ = grpc::CreateChannel(test_bitcode_server_address_,
                                           grpc::InsecureChannelCredentials());
    bitcode_stub_ = BitcodeService::NewStub(bitcode_channel_);

    // Start the EESI service.
    constexpr char kTestEesiServerAddress[] = "localhost:70052";
    eesi_builder_.AddListeningPort(kTestEesiServerAddress,
                                   grpc::InsecureServerCredentials());
    eesi_builder_.RegisterService(&eesi_service_);
    eesi_builder_.RegisterService(&eesi_service_.operations_service);
    eesi_server_ = eesi_builder_.BuildAndStart();
    eesi_channel_ = grpc::CreateChannel(kTestEesiServerAddress,
                                        grpc::InsecureChannelCredentials());
    eesi_stub_ = EesiService::NewStub(eesi_channel_);
    operations_stub_ = OperationsService::NewStub(eesi_channel_);
  }

  void TearDown() override {
    bitcode_server_->Shutdown();
    eesi_server_->Shutdown();
  }
};

TEST_F(EesiServiceTest, PidginSpecifications) {
  // Register the bitcode file.
  RegisterBitcodeRequest register_bitcode_req;
  RegisterBitcodeResponse register_bitcode_res;
  grpc::ClientContext register_bitcode_context;

  const Uri &file_uri = FilePathToUri("testdata/programs/pidgin-reg2mem.ll");
  register_bitcode_req.mutable_uri()->CopyFrom(file_uri);
  grpc::Status status = bitcode_stub_->RegisterBitcode(
      &register_bitcode_context, register_bitcode_req, &register_bitcode_res);

  if (status.error_code() != grpc::OK) {
    std::cerr << status.error_message() << std::endl;
  }
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Initial specifications for pidgin.
  Function malloc_fn;
  malloc_fn.set_source_name("malloc");
  malloc_fn.set_llvm_name("malloc");
  Specification malloc_spec;
  malloc_spec.mutable_function()->CopyFrom(malloc_fn);
  malloc_spec.set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  Function g_malloc_fn;
  g_malloc_fn.set_source_name("g_malloc");
  g_malloc_fn.set_llvm_name("g_malloc");
  Specification g_malloc_spec;
  g_malloc_spec.mutable_function()->CopyFrom(g_malloc_fn);
  g_malloc_spec.set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  Function realloc_fn;
  realloc_fn.set_source_name("realloc");
  realloc_fn.set_llvm_name("realloc");
  Specification realloc_spec;
  realloc_spec.mutable_function()->CopyFrom(realloc_fn);
  realloc_spec.set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  Function calloc_fn;
  calloc_fn.set_source_name("calloc");
  calloc_fn.set_llvm_name("calloc");
  Specification calloc_spec;
  calloc_spec.mutable_function()->CopyFrom(calloc_fn);
  calloc_spec.set_lattice_element(
      SignLatticeElement::SIGN_LATTICE_ELEMENT_ZERO);

  // Send GetSpecificationsRequest to EESI server.
  Handle remote_bitcode_handle;
  remote_bitcode_handle.set_id(register_bitcode_res.bitcode_id().id());
  remote_bitcode_handle.set_authority(test_bitcode_server_address_);

  GetSpecificationsRequest get_specifications_req;
  Operation get_specifications_operation;
  grpc::ClientContext get_specifications_context;
  get_specifications_req.mutable_bitcode_id()->CopyFrom(remote_bitcode_handle);
  get_specifications_req.add_initial_specifications()->CopyFrom(malloc_spec);
  get_specifications_req.add_initial_specifications()->CopyFrom(g_malloc_spec);
  get_specifications_req.add_initial_specifications()->CopyFrom(realloc_spec);
  get_specifications_req.add_initial_specifications()->CopyFrom(calloc_spec);
  status = eesi_stub_->GetSpecifications(&get_specifications_context,
                                         get_specifications_req,
                                         &get_specifications_operation);
  if (status.error_code() != grpc::OK) {
    std::cerr << status.error_message() << std::endl;
  }
  ASSERT_EQ(status.error_code(), grpc::OK);

  // Get the status of the operation and wait until finished or error.
  // Timeout after 30 seconds.
  int number_of_tries = 0;
  while (!get_specifications_operation.done()) {
    grpc::ClientContext get_operation_context;
    GetOperationRequest get_operation_req;
    get_operation_req.set_name(get_specifications_operation.name());
    status = operations_stub_->GetOperation(&get_operation_context,
                                            get_operation_req,
                                            &get_specifications_operation);

    if (status.error_code() != grpc::OK) {
      std::cerr << status.error_message() << std::endl;
    }
    ASSERT_EQ(status.error_code(), grpc::OK);
    ASSERT_LE(number_of_tries, 10);
    number_of_tries++;
    usleep(1000 * 1000);
  }
  ASSERT_EQ(get_specifications_operation.done(), true);

  // Get the results of the operation.
  GetSpecificationsResponse response;
  get_specifications_operation.response().UnpackTo(&response);

  ASSERT_EQ(GetNonEmptySpecificationsCount(response), 15)
      << response.DebugString();
}

}  // namespace error_specifications
