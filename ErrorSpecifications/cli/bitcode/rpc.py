"""Wrappers for calling the bitcode gRPC service.
"""
import functools

import cli.bitcode.db
import cli.operations.wait
import proto.bitcode_pb2
import proto.bitcode_pb2_grpc

def register_bitcode(bitcode_stub, uri):
    """Registers a bitcode file and returns the bitcode id handle.

    Args:
        uri: The uri protobuf message corresponding to the bitcode file
    """

    # Register the bitcode file
    request = proto.bitcode_pb2.RegisterBitcodeRequest(uri=uri)
    response = bitcode_stub.RegisterBitcode(request)

    return response.bitcode_id

def get_called_functions(bitcode_stub, operations_stub, bitcode_id_handles,
                         database, max_tasks):
    """ Builds CalledFunctionsRequest for each bitcode file and then
        sends requests to wait until finished
    """

    # Generating a dictionary from bitcode id to CalledFunctionsRequest
    id_requests = {}
    for bitcode_id_handle in bitcode_id_handles:
        request = proto.bitcode_pb2.CalledFunctionsRequest(
            bitcode_id=bitcode_id_handle
        )
        id_requests[bitcode_id_handle.id] = request

    # Sending requests to be processed
    cli.operations.wait.wait_for_operations(
        operations_stub=operations_stub,
        id_requests=id_requests,
        request_function=bitcode_stub.GetCalledFunctions,
        max_tasks=max_tasks,
        notify=functools.partial(cli.bitcode.db.insert_called_functions,
                                 database),
    )

def get_local_called_functions(bitcode_stub, operations_stub,
                               bitcode_id_handles, database, max_tasks):
    """ Builds LocalCalledFunctionsRequest for each bitcode file and then
        sends requests to wait until finished
    """

    # Generating a dictionary from bitcode id to LocalCalledFunctionsRequest
    id_requests = {}
    for bitcode_id_handle in bitcode_id_handles:
        request = proto.bitcode_pb2.LocalCalledFunctionsRequest(
            bitcode_id=bitcode_id_handle
        )
        id_requests[bitcode_id_handle.id] = request

    # Sending requests to be processed
    cli.operations.wait.wait_for_operations(
        operations_stub=operations_stub,
        id_requests=id_requests,
        request_function=bitcode_stub.GetLocalCalledFunctions,
        max_tasks=max_tasks,
        notify=functools.partial(cli.bitcode.db.insert_local_called_functions,
                                 database),
    )

def get_file_called_functions(bitcode_stub, operations_stub, bitcode_id_handles,
                              database, max_tasks):
    """ Builds FileCalledFunctionsRequest for each bitcode file and then
        sends requests to wait until finished
    """

    # Generating a dictionary from bitcode id to FileCalledFunctionsRequest
    id_requests = {}
    for bitcode_id_handle in bitcode_id_handles:
        request = proto.bitcode_pb2.FileCalledFunctionsRequest(
            bitcode_id=bitcode_id_handle
        )
        id_requests[bitcode_id_handle.id] = request

    # Sending requests to be processed
    cli.operations.wait.wait_for_operations(
        operations_stub=operations_stub,
        id_requests=id_requests,
        request_function=bitcode_stub.GetFileCalledFunctions,
        max_tasks=max_tasks,
        notify=functools.partial(cli.bitcode.db.insert_file_called_functions,
                                 database),
    )

def get_defined_functions(bitcode_stub, operations_stub, bitcode_id_handles,
                          database, max_tasks):
    """ Builds DefinedFunctionsRequest for each bitcode file and then
        sends requests to wait until finished
    """

    # Generating a dictionary from bitcode id to DefinedFunctionsRequest
    id_requests = {}
    for bitcode_id_handle in bitcode_id_handles:
        request = proto.bitcode_pb2.DefinedFunctionsRequest(
            bitcode_id=bitcode_id_handle
        )
        id_requests[bitcode_id_handle.id] = request

    # Sending requests to be processed
    cli.operations.wait.wait_for_operations(
        operations_stub=operations_stub,
        id_requests=id_requests,
        request_function=bitcode_stub.GetDefinedFunctions,
        max_tasks=max_tasks,
        notify=functools.partial(cli.bitcode.db.insert_defined_functions,
                                 database),
    )
