// Common definitions and utility routines that might be useful to
// any of the services.

#ifndef ERROR_SPECIFICATIONS_COMMON_SERVERS_H_
#define ERROR_SPECIFICATIONS_COMMON_SERVERS_H_

#include "include/grpcpp/grpcpp.h"
#include "proto/operations.grpc.pb.h"

namespace error_specifications {

// Converts between string URI schemes and proto URI schemes.
// Schemes are defined in ../proto/operations.proto and converter
// is implemented in common/src/servers.cc. If updates to scheme
// occur, cli/common/uri.py must be updated as well.
class UriSchemes {
  public:
    // Converts a string URI scheme to a scheme proto message.
    static const std::unordered_map<std::string, Scheme>
        string_to_scheme;
    // Converts a URI scheme proto message to a URI string.
    static const std::unordered_map<Scheme, std::string, std::hash<int>>
        scheme_to_string;
};

// Uri is defined in ../proto/operations.proto 
struct UriHash {
  size_t operator()(const Uri &uri) const {
    return std::hash<std::string>()(UriSchemes::scheme_to_string.at(uri.scheme()) + 
                                    uri.authority() +
                                    uri.path());
  }
};

struct UriCompare {
  bool operator()(const Uri &lhs, const Uri &rhs) const {
    return lhs.scheme() == rhs.scheme() && lhs.authority() == rhs.authority() &&
           lhs.path() == rhs.path();
  }
};

// Given a URI converts it to a file path.
// Assumes that uri starts with file:///
grpc::Status ConvertUriToFilePath(const Uri &uri, std::string &out_file_path);

Uri FilePathToUri(const std::string &file_path);

grpc::Status HashString(const std::string &bitcode_data,
                        std::string &out_hashed_bitcode_data);

// Overriding operator for cleaner Uri printing
inline std::ostream &operator<<(std::ostream &stream, const Uri& uri) {
  return stream << UriSchemes::scheme_to_string.at(uri.scheme()) + "://" 
                << uri.authority() + "/" 
                << uri.path();
}

// Transform `foo.0` into `foo` and `foo.0.0` into `foo`.
// Returns substring from the beginning of the string up to and not including
// the first encountered dot.
std::string StripSuffixAfterDot(const std::string &string);

grpc::Status ReadUriIntoString(const Uri &uri, std::string &data);

// Returns a string representing the task name comprised of the RPC call, the
// bitcode ID, and a time stamp.
std::string GetTaskName(const std::string &request_name,
                        const std::string &bitcode_id);

}  // namespace error_specifications

#endif  // ERROR_SPECIFICATIONS_COMMON_SERVERS_H_
