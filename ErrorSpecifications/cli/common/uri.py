import urllib

import proto.operations_pb2_grpc

# String scheme to proto scheme.
# Defined in proto/operations.proto and if updated
# common/include/servers.h needs to be updated as well.
STR_TO_SCHEME = {
    "file":proto.operations_pb2.Scheme.SCHEME_FILE,
    "gs": proto.operations_pb2.Scheme.SCHEME_GS,
}

# Proto scheme to string representation.
SCHEME_TO_STR = {
    proto.operations_pb2.Scheme.SCHEME_FILE: "file",
    proto.operations_pb2.Scheme.SCHEME_GS: "gs",
}

def parse(str_uri):
    """Parses a URI string and returns a URI protobuf message."""

    parsed_uri = urllib.parse.urlparse(str_uri)

    return proto.operations_pb2.Uri(
        scheme=STR_TO_SCHEME[parsed_uri.scheme],
        authority=parsed_uri.netloc,
        path=parsed_uri.path
    )

def to_str(uri):
    """Returns a string representation of a URI given a protobuf message."""
    return SCHEME_TO_STR[uri.scheme] + "://" + uri.authority + uri.path
