"""Tests for GetSpecifications commands."""

import os
import sys

import pytest

import cli.bitcode.commands
import cli.common.service_configuration_handler as service_handler
import cli.eesi.commands
import cli.eesi.domain_knowledge_handler as dk_handler
import cli.eesi.synonym_configuration_handler as synonym_handler
import cli.test.common.common as common

EESI_HOST = "127.0.0.1"
# This will get incremented for each test to ensure that each test has a unique
# port.
BASE_EESI_PORT = 80050
BITCODE_HOST = "127.0.0.1"
# This port number is unique to this test file. This is also only the base port
# number, as we utilize multiple port numbers while testing. Each test in this
# file will use a unique port number (incrementing starting from this base
# number). This file should only utlize 80060-80069.
BASE_BITCODE_PORT = 80060
# The embedding service is actually not used in these tests.
EMBEDDING_HOST = "127.0.0.1"
# This will get incremented for each test to ensure that each test has a unique
# port. This value is however not currently utilized.
BASE_EMBEDDING_PORT = 80070

INITIAL_SPECIFICATIONS = os.path.abspath(
    "testdata/domain_knowledge/initial-specifications-test.txt")

def test_get_specifications_uri():
    """Tests getting specifications for error_only_function_ptr.ll"""
    # Getting a unique bitcode port and eesi port for this test, starting from
    # BASE_BITCODE_PORT and BASE_EESI_PORT respectively.
    bitcode_port = str(BASE_BITCODE_PORT)
    eesi_port = str(BASE_EESI_PORT)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_database()
    # Starting up both the bitcode and EESI service.
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")
    common.setup_service(EESI_HOST, eesi_port, "eesi")

    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port,
        eesi_address=EESI_HOST, eesi_port=eesi_port,)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=INITIAL_SPECIFICATIONS,
        error_codes_path=None, error_only_path=None,
        success_codes_path=None)
    synonym_configuration_handler = \
        synonym_handler.SynonymConfigurationHandler()

    # String representation of the bitcode file URI in the test dataset.
    bitcode_uri = os.path.join(
        common.STR_DATASET_URI, "error_only_function_ptr.ll")
    # Bitcode IDs are explained in cli/bitcode/test/common.py.
    bitcode_id = \
        "5ab2764af21620644b59a2bb88e6a192aeecc9b4218fdf1d7143302007117a15"
    # The authority field should be the address of the bitcode service.
    authority = service_configuration_handler.bitcode_config.get_full_address() 

    # Creating specifications and request based off of how they will
    # appear in MongoDB.
    specifications = [
        {
            "function":{
                "llvmName": "foo",
                "sourceName": "foo",
                "returnType": "FUNCTION_RETURN_TYPE_POINTER",
            },
            "latticeElement": "SIGN_LATTICE_ELEMENT_ZERO",
            "confidenceZero": 100,
        },
        {
            "function": {
                "llvmName": "malloc",
                "sourceName": "malloc",
                "returnType": "FUNCTION_RETURN_TYPE_POINTER",
            },
            "latticeElement": "SIGN_LATTICE_ELEMENT_ZERO",
            "confidenceZero": 100,
        },
    ]

    request = {
        "bitcodeId": {
            "id": bitcode_id,
            "authority": authority,
        },
        "initialSpecifications" : [
            {
                "function": {
                    "llvmName": "malloc",
                    "sourceName": "malloc",
                },
                "latticeElement": "SIGN_LATTICE_ELEMENT_ZERO",
            }
        ],
        "synonymFinderParameters": {},
    }

    # Registering error_only_function_ptr.ll with the bitcode service.
    cli.bitcode.commands.register_bitcode(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,
    )

    # Getting called functions for error_only_function_ptr.ll.
    cli.bitcode.commands.get_called_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,)

    # Checking if the response was put in database.
    assert database.CalledFunctionsResponse.find().count() == 1

    cli.bitcode.commands.get_defined_functions_uri(
        database,
        service_configuration_handler,
        bitcode_uri,
        False,)

    # Checking if the response was put in datbase.
    assert database.DefinedFunctionsResponse.find().count() == 1

    cli.eesi.commands.get_specifications_uri(
        database=database,
        service_configuration_handler=service_configuration_handler,
        domain_knowledge_handler=domain_knowledge_handler,
        synonym_configuration_handler=synonym_configuration_handler,
        bitcode_uri=bitcode_uri,
        smart_success_code_zero=False,
        overwrite=False,
    )
    assert database.GetSpecificationsResponse.find().count() == 1
    assert database.GetSpecificationsResponse.find_one(
        {"$and": [
            {"specifications" : specifications},
            {"violations": {}},
            {"request": request},
        ]}
    )

    # Executing a duplicate command.
    cli.eesi.commands.get_specifications_uri(
        database=database,
        service_configuration_handler=service_configuration_handler,
        domain_knowledge_handler=domain_knowledge_handler,
        synonym_configuration_handler=synonym_configuration_handler,
        bitcode_uri=bitcode_uri,
        smart_success_code_zero=False,
        overwrite=False,
    )

    # Ensuring that the second command didn't duplicate an entry.
    assert database.GetSpecificationsResponse.find().count() == 1

    # Overwriting and inserting a bogus entry to be overwritten.
    database.GetSpecificationsResponse.drop()
    database.GetSpecificationsResponse.insert_one(
        {
            "bogus": "this_is_fake",
            "request": {
                "bitcodeId": {
                    "id": bitcode_id
                }
            }
        }
    )

    # Executing command with overwrite as True.
    cli.eesi.commands.get_specifications_uri(
        database=database,
        service_configuration_handler=service_configuration_handler,
        domain_knowledge_handler=domain_knowledge_handler,
        synonym_configuration_handler=synonym_configuration_handler,
        bitcode_uri=bitcode_uri,
        smart_success_code_zero=False,
        overwrite=True,
    )

    # Ensuring command overwrote bogus entry with correct entry.
    assert database.GetSpecificationsResponse.find().count() == 1
    assert database.GetSpecificationsResponse.find_one(
        {"$and": [
            {"specifications" : specifications},
            {"violations": {}},
            {"request": request},
        ]}
    )

def test_get_specifications_all():
    """Tests getting called functions for all bitcode in test dataset."""
    # Getting a unique bitcode port and eesi port for this test, starting from
    # BASE_BITCODE_PORT and BASE_EESI_PORT respectively.
    bitcode_port = str(BASE_BITCODE_PORT + 1)
    eesi_port = str(BASE_EESI_PORT + 1)
    # Getting a mock database and launching the bitcode service.
    database = common.setup_database()
    # Assigning a unique port for the test.
    common.setup_service(BITCODE_HOST, bitcode_port, "bitcode")
    common.setup_service(EESI_HOST, eesi_port, "eesi")

    # Setting up handlers.
    service_configuration_handler = service_handler.ServiceConfigurationHandler(
        bitcode_address=BITCODE_HOST, bitcode_port=bitcode_port,
        eesi_address=EESI_HOST, eesi_port=eesi_port,
        max_tasks=20)
    domain_knowledge_handler = dk_handler.DomainKnowledgeHandler(
        initial_specifications_path=INITIAL_SPECIFICATIONS,
        error_codes_path=None, error_only_path=None,
        success_codes_path=None)
    synonym_configuration_handler = \
        synonym_handler.SynonymConfigurationHandler()

    # Registering all bitcode in test dataset with the bitcode service.
    cli.bitcode.commands.register_local_dataset(
        database=database,
        service_configuration_handler=service_configuration_handler,
        uri=common.STR_DATASET_URI,
        overwrite=False,
    )

    # Getting called functions for all bitcode registered and appearing
    # in the mock database. This is needed for EESI.
    cli.bitcode.commands.get_called_functions_all(
        database,
        service_configuration_handler,
        False
    )

    # Ensuring that the number of entries matches the number of
    # test data bitcode files.
    assert database.CalledFunctionsResponse.find().count() is len(
        common.TESTDATA_INFO)

    cli.bitcode.commands.get_defined_functions_all(
        database,
        service_configuration_handler,
        False,)

    # Checking if the response was put in datbase.
    assert database.DefinedFunctionsResponse.find().count() is len(
        common.TESTDATA_INFO)

    cli.eesi.commands.get_specifications_all(
        database=database,
        service_configuration_handler=service_configuration_handler,
        domain_knowledge_handler=domain_knowledge_handler,
        synonym_configuration_handler=synonym_configuration_handler,
        smart_success_code_zero=False,
        overwrite=False,
    )

    assert database.GetSpecificationsResponse.find().count() is len(
        common.TESTDATA_INFO)

if __name__ == "__main__":
    # To view standard output, add a "-s" to args.
    sys.exit(pytest.main(args=[__file__]))
