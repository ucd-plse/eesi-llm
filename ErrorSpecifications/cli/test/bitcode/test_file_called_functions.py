""" Tests for FileCalledFunctions commands """

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
# number). This file should only utlize 60070-60079.
BASE_BITCODE_PORT = 60070

def test_file_called_functions_uri():
    """Tests getting file called functions for multireturn.ll"""
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
        common.STR_DATASET_URI, "multireturn.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "db9764c8c570a3ce9973e220dea0c2eb58f8d1e639d7506a03e1ee03263dfb2e"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address()
    # The called functions response that is expected to be stored in MongoDB.
    file_called_functions = [
        {"function": {
            "llvmName": "EO",
            "sourceName": "EO",
            "returnType": "FUNCTION_RETURN_TYPE_VOID",},
         "totalCallSites": 3
        },
        {"function": {
            "llvmName": "foo3",
            "sourceName": "foo3",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",},
         "totalCallSites": 1
        },
        {"function": {
            "llvmName": "foo2",
            "sourceName": "foo2",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",},
         "totalCallSites": 1
        },
        {"function": {
            "llvmName": "foo1",
            "sourceName": "foo1",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",},
         "totalCallSites": 1
        },

    ]
    # The request is also stored in the database, paired with the response.
    request = {
        "bitcodeId": {
            "id": bitcode_id,
            "authority": authority,
        }
    }

    # Registering multireturn.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Getting called functions for multireturn.ll.
    cli.bitcode.commands.get_file_called_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the expected file called functions request/response pair
    # for multireturn.ll appears in the mock database.
    assert database.FileCalledFunctionsResponse.find().count() == 1
    response = database.FileCalledFunctionsResponse.find_one(
        {"request": request})
    assert len(response["fileCalledFunctions"]) == 1
    assert len(response["fileCalledFunctions"][0]["calledFunctions"]) == len(file_called_functions)
    assert response["fileCalledFunctions"][0]["file"] == "multireturn.c"
    for file_called_function in file_called_functions:
        assert file_called_function in response["fileCalledFunctions"][0]["calledFunctions"]

def test_file_called_functions_uri_reg2mem():
    """Tests getting file called functions for multireturn-reg2mem.ll"""
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
        common.STR_DATASET_URI, "multireturn-reg2mem.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "cee66609e938da495df4edf89f8e0a89f13bc48bcaf36993ce28aedf7c8af9e2"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address()
    # The called functions response that is expected to be stored in MongoDB.
    file_called_functions = [
        {"function": {
            "llvmName": "EO",
            "sourceName": "EO",
            "returnType": "FUNCTION_RETURN_TYPE_VOID",},
         "totalCallSites": 3
        },
        {"function": {
            "llvmName": "foo3",
            "sourceName": "foo3",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",},
         "totalCallSites": 1
        },
        {"function": {
            "llvmName": "foo2",
            "sourceName": "foo2",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",},
         "totalCallSites": 1
        },
        {"function": {
            "llvmName": "foo1",
            "sourceName": "foo1",
            "returnType": "FUNCTION_RETURN_TYPE_INTEGER",},
         "totalCallSites": 1
        },

    ]

    # The request is also stored in the database, paired with the response.
    request = {
        "bitcodeId": {
            "id": bitcode_id,
            "authority": authority,
        }
    }

    # Registering multireturn-reg2mem.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Getting called functions for multireturn-reg2mem.ll.
    cli.bitcode.commands.get_file_called_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the expected file called functions request/response pair
    # for multireturn.ll appears in the mock database.
    assert database.FileCalledFunctionsResponse.find().count() == 1
    response = database.FileCalledFunctionsResponse.find_one(
        {"request": request})
    assert len(response["fileCalledFunctions"]) == 1
    assert len(response["fileCalledFunctions"][0]["calledFunctions"]) == len(file_called_functions)
    assert response["fileCalledFunctions"][0]["file"] == "multireturn.c"
    for file_called_function in file_called_functions:
        assert file_called_function in response["fileCalledFunctions"][0]["calledFunctions"]

def test_file_called_functions_all():
    """Tests getting file called functions for all bitcode in test dataset."""
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

    # Getting file called functions for all bitcode registered and appearing
    # in the mock database.
    cli.bitcode.commands.get_file_called_functions_all(
        database,
        service_configuration_handler,
        False
    )

    # Ensuring that the number of entries matches the number of
    # test data bitcode files.
    assert database.FileCalledFunctionsResponse.find().count() is len(
        common.TESTDATA_INFO)

    # Checking if each bitcode file's ID has an associated called
    # functions request/response pair appearing in the mock database.
    for bitcode_id in common.BITCODE_IDS:
        assert database.FileCalledFunctionsResponse.find_one(
            {"request.bitcodeId.id": bitcode_id})

if __name__ == "__main__":
    # To view standard output, add a "-s" to args.
    sys.exit(pytest.main(args=[__file__]))
