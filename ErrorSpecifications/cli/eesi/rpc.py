""" Interacts with the EESI RPC service."""

import functools
import cli.eesi.db
import cli.operations.wait

def get_specifications(eesi_stub, operations_stub, bitcode_id_requests,
                       max_tasks, database):
    """Sends GetSpecificationsRequests to EESI service using operations.wait.

    This function only sends GetSpecificationsRequests to the EESI service.
    Unlike other portions of the CLI, eesi.rpc does NOT create the request
    protobuf message. This is because of the need to calculate applicable
    initial specifications and then insert them as part of the request.

    Args:
        eesi_stub: Stub for EESI channel.
        operations_stub: Stub for operations channel.
        bitcode_id_requests: Dictionary from bitcode id to the relevant
            GetSpecificationsRequest.
        max_tasks: The maximum number of tasks to be sent to the EESI service
            at once by operations.wait.
        database: Pymongo object where specifications will be stored.
    """

    # Sending the GetSpecifications request to the EESI service and the
    # response will be stored in the database, using the notify function.
    cli.operations.wait.wait_for_operations(
        operations_stub=operations_stub,
        id_requests=bitcode_id_requests,
        request_function=eesi_stub.GetSpecifications,
        max_tasks=max_tasks,
        notify=functools.partial(cli.eesi.db.insert_specifications, database)
    )
