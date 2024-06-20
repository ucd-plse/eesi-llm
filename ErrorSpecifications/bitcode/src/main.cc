#include "bitcode_server.h"

#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "glog/logging.h"

#include "servers.h"

ABSL_FLAG(std::string, listen, "localhost:50051", "The address to listen on.");

int main(int argc, char **argv) {
  google::InitGoogleLogging("bitcode-service");
  absl::ParseCommandLine(argc, argv);
  std::string listen_address = absl::GetFlag(FLAGS_listen);
  error_specifications::RunBitcodeServer(listen_address);
  google::FlushLogFiles(google::INFO);

  return 0;
}
