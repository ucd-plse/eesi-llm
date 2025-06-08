""" Tests for DefinedFunctions commands """

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
# number). This file should only utlize 60060-60069.
BASE_BITCODE_PORT = 60060

def test_defined_functions_uri():
    """Tests getting defined functions for error_only_function_ptr.ll"""
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_services([
        {"host": BITCODE_HOST, "port": bitcode_port, "service": "bitcode"}])

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port,)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(
        common.STR_DATASET_URI, "error_only_function_ptr.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "5ab2764af21620644b59a2bb88e6a192aeecc9b4218fdf1d7143302007117a15"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address()
    # The defined functions response that is expected to be stored in MongoDB.
    defined_functions = [
        {
            "llvmName": "foo",
            "sourceName": "foo",
            "returnType": "FUNCTION_RETURN_TYPE_POINTER",
        }
    ]
    # The request that is also stored with the defined functions response.
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

    # Getting defined functions for error_only_function_ptr.ll.
    cli.bitcode.commands.get_defined_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the expected defined functions request/response pair
    # for error_only_function_ptr.ll appears in the mock database.
    assert database.DefinedFunctionsResponse.find().count() == 1
    # Order for the defined functions does not matter here, since we only
    # expect one anyways.
    assert database.DefinedFunctionsResponse.find_one(
        {"$and": [{"functions": defined_functions}, {"request": request}]})

def test_defined_functions_uri_reg2mem():
    """Tests getting defined functions for error_only_function_ptr.ll"""
    # Getting a mock database and launching the bitcode service.
    # Getting a unique bitcode port for this test, starting from
    # BASE_BITCODE_PORT.
    bitcode_port = str(BASE_BITCODE_PORT + 1)
    database = common.setup_services([
        {"host": BITCODE_HOST, "port": bitcode_port, "service": "bitcode"}])

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port,)

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(
        common.STR_DATASET_URI, "error_only_function_ptr-reg2mem.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "f1677320bdaf208b6122649066f22b2dd0f4bb842ce85bab98ab6817cf644eb0"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address()
    # The defined functions response that is expected to be stored in MongoDB.
    defined_functions = [
        {
            "llvmName": "foo",
            "sourceName": "foo",
            "returnType": "FUNCTION_RETURN_TYPE_POINTER",
        }
    ]
    # The request that is also stored with the defined functions response.
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

    # Getting defined functions for error_only_function_ptr.ll.
    cli.bitcode.commands.get_defined_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Checking if the expected defined functions request/response pair
    # for error_only_function_ptr.ll appears in the mock database.
    assert database.DefinedFunctionsResponse.find().count() == 1
    # Order for the defined functions does not matter here, since we only
    # expect one anyways.
    assert database.DefinedFunctionsResponse.find_one(
        {"$and": [{"functions": defined_functions}, {"request": request}]})

def test_defined_functions_all():
    """Tests getting defined functions for all bitcode in test dataset."""
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

    # Getting defined functions for all bitcode registered and appearing
    # in the mock database.
    cli.bitcode.commands.get_defined_functions_all(
        database,
        service_configuration_handler,
        False
    )

    # Ensuring that the number of entries matches the number of
    # test data bitcode files.
    assert database.DefinedFunctionsResponse.find().count() is len(
        common.TESTDATA_INFO)

    # Checking if each bitcode file's ID has an associated called
    # functions request/response pair appearing in the mock database.
    for bitcode_id in common.BITCODE_IDS:
        assert database.DefinedFunctionsResponse.find_one(
            {"request.bitcodeId.id": bitcode_id})

if __name__ == "__main__":
    # To view standard output, add a "-s" to args.
    sys.exit(pytest.main(args=[__file__]))
