#include "servers.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "glog/logging.h"
#include "google/cloud/storage/client.h"
#include "include/grpcpp/grpcpp.h"
#include "openssl/evp.h"

#include "proto/operations.grpc.pb.h"

namespace error_specifications {

// Defined and explained in common/include/servers.h.
//
// If schemes are updated, proto/operations.proto,
// common/include/servers.h, and cli/common/uri.py must be
// updated to reflect all possible schemes.
const std::unordered_map<std::string, Scheme>
    UriSchemes::string_to_scheme({
        {"file", Scheme::SCHEME_FILE},
        {"gs", Scheme::SCHEME_GS}, 
    });

const std::unordered_map<Scheme, std::string, std::hash<int>>
    UriSchemes::scheme_to_string({
        {Scheme::SCHEME_FILE, "file"},
        {Scheme::SCHEME_GS, "gs"},
    });

grpc::Status ConvertUriToFilePath(const Uri &uri, std::string &out_file_path) {
  if (uri.scheme() != Scheme::SCHEME_FILE) {
    const std::string &err_msg = "URI is does not use file scheme.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
  }

  out_file_path = uri.path();

  return grpc::Status::OK;
}

Uri FilePathToUri(const std::string &file_path) {
  Uri uri;
  uri.set_scheme(Scheme::SCHEME_FILE);
  uri.set_path(file_path);
  return uri;
}

grpc::Status HashString(const std::string &input_string,
                        std::string &out_hashed_string) {
  EVP_MD_CTX *context = EVP_MD_CTX_new();
  if (context == NULL) {
    const std::string &err_msg =
        "HashBitcodeData: Unable to create OpenSSL EVP context.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INTERNAL, err_msg);
  }

  // All paths must call EVP_MD_CTX_free(context) from this point forward.

  int err = EVP_DigestInit_ex(context, EVP_sha256(), NULL);
  if (err == 0) {
    EVP_MD_CTX_free(context);
    const std::string &err_msg =
        "HashBitcodeData: Unable to initialize message digest.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INTERNAL, err_msg);
  }

  err = EVP_DigestUpdate(context, input_string.c_str(), input_string.length());
  if (err == 0) {
    EVP_MD_CTX_free(context);
    const std::string &err_msg = "HashBitcodeData: Unable to update digest.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INTERNAL, err_msg);
  }

  // The actual hash.
  unsigned char hash[EVP_MAX_MD_SIZE];

  // Hash length returned by OpenSSL.
  unsigned int hash_length = 0;

  err = EVP_DigestFinal_ex(context, hash, &hash_length);
  if (err == 0) {
    EVP_MD_CTX_free(context);
    const std::string &err_msg = "HashBitcodeData: Unable to finalize digest.";
    LOG(ERROR) << err_msg;
    return grpc::Status(grpc::StatusCode::INTERNAL, err_msg);
  }

  // Convert hash to string.
  std::stringstream ss;
  for (unsigned int i = 0; i < hash_length; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }

  out_hashed_string = ss.str();

  EVP_MD_CTX_free(context);

  return grpc::Status::OK;
}

std::string StripSuffixAfterDot(const std::string &input_string) {
  auto idx = input_string.find('.');
  if (idx != std::string::npos) {
    return input_string.substr(0, idx);
  } else {
    return input_string;
  }
}

grpc::Status ReadUriIntoString(const Uri &uri, std::string &data) {
  std::ostringstream data_stream;

  // Read the file, either from local disk or GCS.
  switch (uri.scheme()) {
    case Scheme::SCHEME_FILE:
      {
        std::string file_path;
        grpc::Status err = ConvertUriToFilePath(uri, file_path);
        if (!err.ok()) {
          return err;
        }
        std::ifstream ifs(file_path, std::ios::binary);
        if (!ifs) {
          const std::string &err_msg = "Unable to read file.";
          return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
        }

        // Read entire file into a string.
        data_stream << ifs.rdbuf();
        ifs.close();
      }
      break;
    case Scheme::SCHEME_GS:
      {
        google::cloud::StatusOr<google::cloud::storage::Client> client =
            google::cloud::storage::Client::CreateDefaultClient();
        if (!client) {
          const std::string err_msg = "Failed to create GS storage client.";
          LOG(ERROR) << err_msg << client.status();
          return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, err_msg);
        }
        google::cloud::storage::ObjectReadStream stream =
            client->ReadObject(uri.authority(), uri.path());
        data_stream << stream.rdbuf();
      }
      break;
    default:
      {
        const std::string err_msg = "Invalid URI scheme provided.";
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, err_msg);
      }
      break;
  }
  // Check to make sure we actually received data.
  data = data_stream.str();

  return grpc::Status::OK;
}

std::string GetTaskName(const std::string &request_name,
                        const std::string &unique_id) {
  std::time_t curr_time = std::time(nullptr);
  std::string time_stamp = std::asctime(std::localtime(&curr_time));

  return request_name + "-" + unique_id + "-" + time_stamp;
}

}  // namespace error_specifications
