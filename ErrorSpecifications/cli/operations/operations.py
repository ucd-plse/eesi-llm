""" Module for interacting with operations rpc service
"""
import grpc

import proto.operations_pb2_grpc

def get_operations_stub(operations_server, operations_port):
    """ Returns an operations stub using the server address and
        port number provided as arguments
    """

    operations_address = operations_server + ":" + operations_port
    operations_channel = grpc.insecure_channel(operations_address)
    operations_stub = proto.operations_pb2_grpc.OperationsServiceStub(operations_channel)

    return operations_stub
