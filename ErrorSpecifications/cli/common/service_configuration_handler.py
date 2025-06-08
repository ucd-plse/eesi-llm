"""Classes for gRPC service configuration and handling.

Usage:
    service_handler = ServiceConfigurationHandler(
        eesi_address=eesi_address,
        eesi_port=eesi_port,
        bitcode_address=bitcode_address,
        bitcode_port=bitcode_port,)

    eesi_stub = service_handler.get_eesi_stub()
    bitcode_stub = service_handler.get_bitcode_stub()
"""

import grpc

import proto.bitcode_pb2_grpc
import proto.eesi_pb2_grpc
import proto.operations_pb2_grpc

class ServiceConfigurationHandler:
    """Handles the configuration of services."""

    def __init__(self, eesi_address=None, eesi_port=None,
                 bitcode_address=None, bitcode_port=None,
                 max_tasks=4):

        self.eesi_config = self.ServiceConfiguration(
            address=eesi_address, port=eesi_port)
        self.bitcode_config = self.ServiceConfiguration(
            address=bitcode_address, port=bitcode_port)

        self.max_tasks = max_tasks

    @staticmethod
    def from_args(args):
        """Initializes a ServiceConfigurationHandler from ArgParse args."""
        return ServiceConfigurationHandler(
            eesi_address=args.eesi_address,
            eesi_port=args.eesi_port,
            bitcode_address=args.bitcode_address,
            bitcode_port=args.bitcode_port,
            max_tasks=args.max_tasks,
        )

    def get_eesi_stub(self):
        """Returns an EESI gRPC stub according to the configuration."""

        return proto.eesi_pb2_grpc.EesiServiceStub(
            self.eesi_config.get_channel())

    def get_bitcode_stub(self):
        """Returns a bitcode gRPC stub according to the configuration."""

        return proto.bitcode_pb2_grpc.BitcodeServiceStub(
            self.bitcode_config.get_channel())

    def get_operations_stub(self, service):
        """Returns an operation stub according to the supplied flag."""

        services = {"eesi": self.eesi_config,
                    "bitcode": self.bitcode_config,} 
        if service not in services:
            raise ValueError("Must select a valid choice from: {}".format(
                services.keys()))

        return proto.operations_pb2_grpc.OperationsServiceStub(
            services[service].get_channel())

    class ServiceConfiguration:
        """Represents the configuration for an individual service."""

        def __init__(self, address, port):
            # It is fine to have an empty service configuration, you just
            # can't can't call any of the useful methods.
            # Logical XNOR.
            assert bool(address) is bool(port)

            self.address = address
            self.port = port

        def get_full_address(self):
            """Returns a string for the full address of the service."""

            assert self.address and self.port
            return self.address + ":" + self.port

        def get_channel(self):
            """Returns a gRPC insecure channel according to configuration."""

            assert self.address and self.port
            return grpc.insecure_channel(
                self.get_full_address(),
                options=[
                    ("grpc.max_send_message_length", 100*1024*1024),
                    ("grpc.max_receive_message_length", 100*1024*1024),])
