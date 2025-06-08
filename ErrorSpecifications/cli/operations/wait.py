""" Module for wait for response functions. These functions take
    in requests and then send them to the appropriate service
    and then wait until finished.
"""
import time

import glog as log

import proto.operations_pb2

def wait_for_one_operation(operations_stub, operation):
    """Waits for a single operation to finish and returns the final response."""
    done = False
    while not done:
        try:
            request = proto.operations_pb2.GetOperationRequest(
                name=operation.name,
            )
            response = operations_stub.GetOperation(request)
            done = response.done
            time.sleep(1)
        #TODO (patrickjchap): This is too general, we need a
        #better/elegant solution.
        except Exception as e:
            print(e)
            time.sleep(1)

    return response

def wait_for_operations(operations_stub, id_requests, request_function,
                        max_tasks, notify):
    """Sends all requests to the relevant service and waits until all finish.
    Args:
        id_requests: Dictionary from a unique identifier to a request
        request_function: The request function relevant to
            the requests in id_requests
        max_tasks: The maximum number of tasks to send to a service
        notify: A function to notify when an operation is finished. This
            function gets called with the tuple
            (bitcode_id, finished_operation).
    Returns:
        Returns true if the operations finished.
    """
    # Dictionary from a unique id to the associated finished response
    id_finished_responses = {}
    # Set of operations waiting to finish
    waiting_for_results = set()
    # Dictionary from task name to a unique id
    task_id = {}
    # Dictionary from task name to request
    task_requests = {}

    # Waiting for all sent requests to finish and for all requests to be sent
    while waiting_for_results or id_requests:
        # For removing requests that are processed from requests_to_send
        to_remove = set()
        counter = 0
        for unique_id, request in id_requests.items():
            # Limiting the number of requests being sent at a time
            if len(waiting_for_results) >= max_tasks:
                break
            counter += 1
            log.info("Submitting task {} of {} remaining tasks. Max Tasks {}"
                     .format(counter, len(id_requests.items()), max_tasks,))

            get_response = request_function(request)
            waiting_for_results.add(get_response.name)
            task_id[get_response.name] = unique_id
            task_requests[get_response.name] = request
            to_remove.add(unique_id)

        # Removing the requests that have already been sent
        for unique_id in to_remove:
            id_requests.pop(unique_id)

        # For removing tasks from waiting_for_results
        finished_tasks = set()

        for task in waiting_for_results:
            get_operation_request = \
                proto.operations_pb2.GetOperationRequest(
                    name=task)
            get_operation_response = operations_stub.GetOperation(
                get_operation_request)

            if get_operation_response.done:
                unique_id = task_id[task]
                id_finished_responses[unique_id] = get_operation_response
                finished_tasks.add(task)
                if notify:
                    notify(task_requests[task], get_operation_response)

        # Removing tasks that have finished
        waiting_for_results = waiting_for_results.difference(finished_tasks)
        # If some tasks still haven't finished, sleep for 5 seconds
        if waiting_for_results:
            time.sleep(5)

    return bool(id_finished_responses)
