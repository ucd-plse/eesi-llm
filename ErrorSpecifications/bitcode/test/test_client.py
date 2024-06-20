import grpc

import proto.bitcode_pb2
import proto.bitcode_pb2_grpc
import proto.operations_pb2

"""A python client for manually testing bitcode services.
This isn't run by bazel test, it is just useful to have
around for poking at the gRPC service by hand.
"""

def main2():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = proto.bitcode_pb2_grpc.BitcodeServiceStub(channel)

        # Register the bitcode file
        register_req = proto.bitcode_pb2.RegisterBitcodeRequest(
            uri = "file:///tmp/out.ll"
        )  
        register_bitcode_response = stub.RegisterBitcode(register_req)

        defined_req = proto.bitcode_pb2.DefinedFunctionsRequest(
            bitcode_id = register_bitcode_response.bitcode_id
        )
        defined_res = stub.GetDefinedFunctions(defined_req)
        print(defined_res)

def main():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = proto.bitcode_pb2_grpc.BitcodeServiceStub(channel)

        # Register the bitcode file
        register_req = proto.bitcode_pb2.RegisterBitcodeRequest(
            uri = "file:///tmp/hello.ll"
        )  
        register_bitcode_response = stub.RegisterBitcode(register_req)

        # Annotate the bitcode file with instruction IDs
        annotate_req = proto.bitcode_pb2.AnnotateRequest(
            bitcode_id = register_bitcode_response.bitcode_id,
            output_uri = "file:///tmp/annotated.bc",
        )
        annotate_response = stub.Annotate(annotate_req)

        # Download the annotated bitcode file and write it to a file
        download_req = proto.bitcode_pb2.DownloadBitcodeRequest(
            bitcode_id = annotate_response.bitcode_id
        )

        download_response = stub.DownloadBitcode(download_req)

        # Re-assemble the chunks
        with open("/tmp/out.ll", 'wb') as f:
            for chunk in download_response:
                f.write(chunk.content)

if __name__ == "__main__":
    main()
    main2()
