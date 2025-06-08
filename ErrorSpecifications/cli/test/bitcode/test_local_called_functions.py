""" Tests for LocalCalledFunctions commands """

import os
import sys

import pytest

import cli.bitcode.commands
import cli.common.service_configuration_handler as service_handler
import cli.test.common.common as common

BITCODE_HOST = "127.0.0.1"
# This port number is unique to this test file. This is also only the base port
# number, as we utilize multiple port numbers while testing. Each test in this
# file will use a unique port number (incrementing starting from this base
# number). This file should only utlize 60080-60089.
BASE_BITCODE_PORT = 60080

def test_local_called_functions_uri():
    """Tests getting local called functions for hello_twice.ll"""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT)
    # Assigning a unique port for the test.
    database = common.setup_services([
        {"host": BITCODE_HOST, "port": bitcode_port, "service": "bitcode"}])

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port,)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(
        common.STR_DATASET_URI, "hello_twice.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "681e9b797250781128149d4e6098ce70aa31b520e41fe7f2318f86c7c7ca4603"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address()
    # The called functions response that is expected to be stored in MongoDB.
    called_function = {
        "llvmName": "printf",
        "sourceName": "printf",
        "returnType": "FUNCTION_RETURN_TYPE_INTEGER",
    }

    caller_function = {
        "function": { 
            "llvmName": "main",
            "sourceName": "main",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",
        },
        "totalCallSites": 2,
    }
    # The request is also stored in the database, paired with the response.
    request = {
        "bitcodeId": {
            "id": bitcode_id,
            "authority": authority,
        }
    }

    # Registering error_only_function_ptr.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Getting called functions for error_only_function_ptr.ll.
    cli.bitcode.commands.get_local_called_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the expected called functions request/response pair
    # for error_only_function_ptr.ll appears in the mock database.
    assert database.LocalCalledFunctionsResponse.find().count() == 1
    response = database.LocalCalledFunctionsResponse.find_one(
        {"request": request})
    assert len(response["localCalledFunctions"]) == 1
    assert response["localCalledFunctions"][0]["calledFunction"] == called_function
    assert len(response["localCalledFunctions"][0]["callerFunctions"]) == 1
    assert response["localCalledFunctions"][0]["callerFunctions"][0] == caller_function

def test_local_called_functions_uri_reg2mem():
    """Tests getting local called functions for hello_twice-reg2mem.ll"""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 1)
    # Assigning a unique port for the test.
    database = common.setup_services([
        {"host": BITCODE_HOST, "port": bitcode_port, "service": "bitcode"}])

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port,)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(
        common.STR_DATASET_URI, "hello_twice-reg2mem.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "eab63ac9f1e65d7aa0523080c8ac1d78e8147c6877bd3a460f51c2aea9b2a0c0"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address()
    # The called functions response that is expected to be stored in MongoDB.
    called_function = {
        "llvmName": "printf",
        "sourceName": "printf",
        "returnType": "FUNCTION_RETURN_TYPE_INTEGER",
    }

    caller_function = {
        "function": { 
            "llvmName": "main",
            "sourceName": "main",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",
        },
        "totalCallSites": 2,
    }

    # The request is also stored in the database, paired with the response.
    request = {
        "bitcodeId": {
            "id": bitcode_id,
            "authority": authority,
        }
    }

    # Registering error_only_function_ptr.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Getting called functions for error_only_function_ptr.ll.
    cli.bitcode.commands.get_local_called_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the expected called functions request/response pair
    # for error_only_function_ptr.ll appears in the mock database.
    assert database.LocalCalledFunctionsResponse.find().count() == 1
    response = database.LocalCalledFunctionsResponse.find_one(
        {"request": request})
    assert len(response["localCalledFunctions"]) == 1
    assert response["localCalledFunctions"][0]["calledFunction"] == called_function
    assert len(response["localCalledFunctions"][0]["callerFunctions"]) == 1
    assert response["localCalledFunctions"][0]["callerFunctions"][0] == caller_function

def test_local_called_functions_all():
    """Tests getting local called functions for all bitcode in test dataset."""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 2)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_services([
        {"host": BITCODE_HOST, "port": bitcode_port, "service": "bitcode"}])

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port, max_tasks=20)

    # Registering all bitcode in test dataset with the bitcode service.
    cli.bitcode.commands.register_local_dataset(
        database=database,
        service_configuration_handler=service_configuration_handler,
        uri=common.STR_DATASET_URI,
        overwrite=False,
    )

    # Getting called functions for all bitcode registered and appearing
    # in the mock database.
    cli.bitcode.commands.get_local_called_functions_all(
        database,
        service_configuration_handler,
        False
    )

    # Ensuring that the number of entries matches the number of
    # test data bitcode files.
    assert database.LocalCalledFunctionsResponse.find().count() is len(
        common.TESTDATA_INFO)

    # Checking if each bitcode file's ID has an associated called
    # functions request/response pair appearing in the mock database.
    for bitcode_id in common.BITCODE_IDS:
        assert database.LocalCalledFunctionsResponse.find_one(
            {"request.bitcodeId.id": bitcode_id})

if __name__ == "__main__":
    # To view standard output, add a "-s" to args.
    sys.exit(pytest.main(args=[__file__]))
