# Copyright (c) Meta Platforms, Inc. and affiliates.
# This software may be used and distributed according to the terms of the Gpt 2 Community License Agreement.

from concurrent import futures
from collections import defaultdict
from typing import Optional
import os
import re
import sys
sys.path = [path for path in sys.path if "com_" not in path]

import argparse
import grpc
from openai import OpenAI
from tenacity import (retry, wait_random, stop_after_attempt)

import proto.eesi_pb2
import proto.domain_knowledge_pb2
import proto.gpt_pb2
import proto.gpt_pb2_grpc

MAX_CONTEXT_LEN = 4096
# This implementation is obviously very confusing as we are translating
# BOTTOM to EMPTYSET. This is because currently the implementation doesn't
# have a clean way to indicate that a lattice element is emptyset according
# to our proto definition. Since we don't really care about returning unknown
# error specifications, we can just utilize this field for indicating EMPTYSET.
LATTICE_ELEMENT_TO_STRING = {
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_BOTTOM: "emptyset",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO: "<0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO:
        ">0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO: "==0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO:
        "<=0",
    proto.eesi_pb2.SignLatticeElement \
        .SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO: ">=0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_NOT_ZERO: "!=0",
    proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_TOP: "top",
}
STRING_TO_LATTICE_ELEMENT = {x[1]: x[0] for x in LATTICE_ELEMENT_TO_STRING.items()}
# Common other textual representations for our abstract values. The LLM doesn't
# always like to follow directions.
#STRING_TO_LATTICE_ELEMENT["<0>"] = proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO
STRING_TO_LATTICE_ELEMENT["<0>"] = proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO
old_str = """ 
{..., -3, -2, -1} = '<0'; 
{1, 2, 3, ...} = '>0';  
{0} = '==0'; 
{..., -3, -2, -1, 0} = '<=0';
{0, 1, 2, 3, ...} = '>=0';
{} = 'EMPTYSET';
{..., -3, -2, -1, 0, 1, 2, 3, ...} = 'TOP';
If you do not know the error specification or cannot determine it, the
error specification is 'BOTTOM'.
"""
MODEL_MAIN_CONTEXT = """
You are to provide the error specification of any defined or
called function in the source code provided. An error specification 
describes the values that are returned upon error. We use an abstraction 
function on these error values: 

'<0' = All values less-than zero
'>0' = All values greater-than zero
'==0' = Zero
'>=0' = All values greater-than or equal to zero.
'<=0' = All values less-than or equal to zero.
'!=0' = All values that are not zero.
'EMPTYSET' = There are no error values.
'BOTTOM' = The error values are unknown.

Semantics for called functions:
1. If a called function return value is checked and an error value is
returned, then it is LIKELY that the checked value is part of the called error
specification.
2. If a called function return value is checked and an error value is not
returned, then you do not know the error specification, i.e. 'BOTTOM'.

Semantics for defined functions: 
1. The error specification is only a pure subset (strictly less-than) of
all returned values for a function. 
2. If a function aborts or terminates execution on error, then the error
specification is 'EMPTYSET'. 
3. Error codes from standard libraries such as ENOMEM are considered positive
integer values, but are considered negative when the negation '-' is used.
4. An error-only function indicates functions that are only called on
program error paths.
5. Success value are not part of the error specification, do not include these.
Function macros that end with VALIDATE_RET cause an inline return <0 on error.

Respond by first listing all of the error specifications that you know in a
list, then provide a separate list that gives an explanation for each. Example:
    foo: <0
    bar: >=0

    foo: Returns -1 on error.
    bar: Returns 1 and 0 on error.
"""

MODEL_THIRD_PARTY_CONTEXT = """
Here are a list of functions and their return type from various C-libraries; you are to return the error specification for any of these functions that you may know.Only return error specifications of functions that you are confident in.
An error specification describes the values that are returned on error. We use an abstraction function on these error values:                          

'<0' = All values less-than zero
'>0' = All values greater-than zero
'==0' = Zero
'>=0' = All values greater-than or equal to zero.
'<=0' = All values less-than or equal to zero.
'!=0' = All values that are not zero.
'EMPTYSET' = There are no error values.
'BOTTOM' = The error values are unknown, or not confident in.

Additional facts:
1. Error specifications cannot be 'TOP'!
2. Unknown error specifications are 'BOTTOM'! 
3. Success values are not part of the error specification.                   
4. Pointer functions returning NULL on error is '==0'. Pointer functions do not always return NULL!
5. If a function aborts or terminates execution on error, then the error specification in 'EMPTYSET'.
6. Error codes from standard libraries such as ENOMEM are considered positive
integer values, but are considered negative when the negation '-' is used.
7. If you do not know the exact error specification for the function, then it is 'BOTTOM'.

Respond by first listing all of the error specifications that you know in a
list, then provide a separate list that gives an explanation for each. Follow
this exact format example, if you do not you are automatically incorrect and
penalized:
    foo: <0
    bar: >=0

    foo: Returns -1 on error.
    bar: Returns 1 and 0 on error.
"""

THIRD_PARTY_ANSWERS = [
    {"role": "user", "content": "fclose"},
    {"role": "assistant", "content": "fclose: <0\n"
                                "fclose returns EOF (-1) on error."},
    {"role": "user", "content": "malloc"},
    {"role": "assistant", "content": "malloc: ==0\n"
                                "malloc returns NULL (0) on error."},
    {"role": "user", "content": "pthread_mutex_unlock"},
    {"role": "assistant", "content": "pthread_mutex_unlock: >0\n"
                                     "pthread_mutex_unlock on error returns a standard library error number, which is a positive integer."}
]

ANALYSIS_ANSWERS = [
    {"role": "user", "content": "int foo() {\n"
                                     "  int ret = bar(); // bar <0 on error\n"
                                     "  if (ret < 0) { return 1; }\n"
                                     "  return 0;\n"
                                     "}"},
    {"role": "assistant", "content": "foo: >0\n"
                                "When bar returns <0 (error), foo returns 1 (>0)."},
    {"role": "user", "content": "int baz() {\n"
                                     "  int ret = bar(); // bar <0 on error.\n"
                                     "  if (ret < 0) { \n"
                                     "    if (check_error(ret)) {return -1;}\n"
                                     "  }\n"
                                     "  int ret = qux(); // qux unknown (BOTTOM) on error.\n"
                                     "  if (ret > 0) { return -1;}\n"
                                     "  return 0;\n"
                                     "}"},
    {"role": "assistant", "content": "baz: <0\n"
                                "qux: >0\n"
                                "baz returns -1 (<0) when bar returns <0 on error and the error is checked by check_error.\n"
                                "qux returns >0 when baz returns -1 (error)."},
    {"role": "user", "content": "int calculate_foo(int a, int b) {\n"
                                " int c = a << 2;\n"
                                " int d = b << 2;\n"
                                " return c + d;\n"
                                "}"
                                },
    {"role": "assistant", "content": "calculate_foo: EMPTYSET\n"
                                     "calculate_foo appears to just perform bit manipulation with no return value indicating an error."},
    {"role": "user", "content": "int is_bar(obj* a) {\n"
                                " if (a->bar) { return 1;}\n"
                                " return 0;\n"
                                "}"
                                },
    {"role": "assistant", "content": "is_bar: EMPTYSET\n"
                                     "is_bar only identifies if a member 'bar' for 'obj' is set. Nothing returned indicates an error."}
]






class SourceCodeInformation:

    def __init__(self, function_name: str , file_name: str, beginning: int,
                 end: int):
        self.function_name = function_name
        self.file_name = file_name
        self.beginning = beginning
        self.end = end


class GptServicer(
        proto.gpt_pb2_grpc.GptServiceServicer):
    """Provides methods that implement functionality of LLama server."""

    def __init__(self):
        self.client = OpenAI(
            api_key=os.environ.get("OPEN_API_KEY"))
        self.ctags = dict() 

    @retry(wait=wait_random(min=5,max=10), stop=stop_after_attempt(3))
    def completion_with_backoff(self, **kwargs):
        return self.client.chat.completions.create(**kwargs)

    def read_ctags(self, ctags_file):
        """Reads the requested function definition into a string."""

        base_dir = os.path.dirname(ctags_file)

        self.ctags[ctags_file] = dict()
        # Format of 'tags' files with supplied flags is:
        # name file signature beginning-line end-line.
        with open(ctags_file) as ctags:
            for definition in ctags:
                name, file, *remaining = definition.split()
                file = base_dir + "/" + file
                if not os.path.isfile(file):
                    continue

                # We already have a definition with information for, we don't
                # want to reference the header if we already have the '.c'
                # file.
                if name in self.ctags[ctags_file] and not self.ctags[ctags_file][name].file_name.endswith(".h"):
                    continue
                
                beginning = 0
                end = 0
                #print(remaining)
                for word in remaining:
                    print(word)
                    if re.match(r"\bline:", word):
                        # Tag files aren't perfect.
                        str_num = word.split(":")[1]
                        if not str_num:
                            end = -1
                            break
                        beginning = int(word.split(":")[1]) - 1 
                    if re.match(r"\bend:", word):
                        print(word)
                        end = int(word.split(":")[1])

                if not end >= beginning:
                    continue
                self.ctags[ctags_file][name] = SourceCodeInformation(
                    function_name=name, file_name=file, beginning=beginning,
                    end=end)

    def read_function_definition(self, function_name, ctags_file):
        if ctags_file not in self.ctags:
            self.read_ctags(ctags_file)

        definition_str = ""
        definition_file_name = self.ctags[ctags_file][function_name].file_name
        beginning = self.ctags[ctags_file][function_name].beginning
        end = self.ctags[ctags_file][function_name].end
        with open(definition_file_name) as definition_file:
            for line_num, line in enumerate(definition_file):
                if line_num < beginning: 
                    continue
                if line_num > end:
                    break
                definition_str += line + "\n"

        print("===DEF STR===")

        return definition_str

    def reduce_message(self, message):
        if len(message) <= MAX_CONTEXT_LEN:
            return message
        reduction = ""
        for line in message.splitlines():
            if len(reduction) + len(line) > MAX_CONTEXT_LEN:
                break
            reduction += line

        return reduction

    def GetGptThirdPartySpecifications(self, request, context):
        system_context = MODEL_THIRD_PARTY_CONTEXT
        if request.error_code_names:
            print("Supplied error codes for context...")
            formatted_error_codes = ""
            for error_code_name, lattice_element in request.error_code_names.items():
                formatted_error_codes += f"{error_code_name}: {LATTICE_ELEMENT_TO_STRING[lattice_element]}\n"
            system_context += \
                "\nAdditionally, here are several error return variables/macros " + \
                f"and their corresponding abstract value: {formatted_error_codes}\n"

        specs_to_inject = []
        init_names = set()
        if request.error_specifications:
            print("Supplied error specifications for context...")
            formatted_error_specs = ""
            for specification in request.error_specifications:
                init_names.add(specification.function.source_name)
                if not formatted_error_specs:
                    formatted_error_specs = "Here are known error specifications for context: \n"
                print(specification)
                #formatted_error_specs += specification.function.source_name + ": " + LATTICE_ELEMENT_TO_STRING[specification.lattice_element] + "\n"
                specs_to_inject.append({"role": "user", "content": specification.function.source_name}) 
                specs_to_inject.append({"role": "assistant", "content": specification.function.source_name+": "+LATTICE_ELEMENT_TO_STRING[specification.lattice_element]}) 

        print(f"TP SYSTEM CONTEXT: \n{system_context}")
        messages = [{"role": "system", "content": system_context}]
        messages = messages + specs_to_inject
        messages = messages + [
            THIRD_PARTY_ANSWERS[0],
            THIRD_PARTY_ANSWERS[1],
            THIRD_PARTY_ANSWERS[2],
            THIRD_PARTY_ANSWERS[3],
            THIRD_PARTY_ANSWERS[4],
            THIRD_PARTY_ANSWERS[5],
            {
                "role": "user",
                "content": "\n".join([f"{y} {x}" for x, y in request.function_names.items() if x not in init_names]), 
            }
        ]
        results = self.completion_with_backoff(
            model=request.llm_name,
            messages=messages,
        )
        try:
            llm_message = results.choices[0].message.content
        except ValueError:
            print("Third party: Error in generated model result!!!")
            print("Model result:")
            print(results)

        print(f"Before reprompt:\n{llm_message}")

        reprompt = "Correct the list of error specifications to follow your provided description. Remember that success values are not part of the error specification. Follow the expected format!"
        messages.append({"role": "assistant", "content": llm_message})
        messages.append({"role": "user", "content": reprompt})

        results = self.completion_with_backoff(
            model=request.llm_name,
            messages=messages,
        )
        try:
            llm_message = results.choices[0].message.content
        except ValueError:
            print("Reprompted Third party: Error in generated model result!!!")
            print("Model result:")
            print(results)

        print(f"Third party results:")
        print(llm_message)
        specifications = dict()
        for line in llm_message.splitlines():
            try:
                function_name, lattice_element_string = line.split(":")
                function_name = function_name.split()[-1]
                if "bottom" in lattice_element_string.lower():
                    continue
                lattice_element = STRING_TO_LATTICE_ELEMENT[lattice_element_string.strip().lower().replace("'", "").replace('"', "")]
                specifications[function_name] = lattice_element
            except (ValueError, KeyError) as e:
                print(f"Third party: Error parsing result from model: {e}!")
                print(f"line: {line}")
                continue
        return proto.gpt_pb2.GetGptThirdPartySpecificationsResponse(
            specifications=specifications)

    def GetGptSpecification(self, request, context):
        """Returns an error specification from Gpt given a requested function."""
        formatted_error_specs = ""
        for specification in request.error_specifications:
            print(specification)
            formatted_error_specs += specification.function.source_name + ": " + LATTICE_ELEMENT_TO_STRING[specification.lattice_element] + "\n"
        try:
            function_definition = self.read_function_definition(request.function_name, request.ctags_file)
        except KeyError as e:
            print(f"Error reading function definition: {e}")
            return proto.gpt_pb2.GetGptSpecificationResponse()

        if formatted_error_specs:
            print("====Error specifications for context====")
            print(formatted_error_specs)
            print("="*30)
        system_context = MODEL_MAIN_CONTEXT
        if request.error_code_names:
            formatted_error_codes = ""
            for error_code_name, lattice_element in request.error_code_names.items():
                formatted_error_codes += f"{error_code_name}: {LATTICE_ELEMENT_TO_STRING[lattice_element]}\n"
            system_context += \
                "\nAdditionally, here are several error return variables/macros " + \
                f"and their corresponding abstract value: {formatted_error_codes}\n"
        if request.success_code_names:
            formatted_success_codes = ""
            for success_code_name, lattice_element in request.success_code_names.items():
                formatted_success_codes += f"{success_code_name}: {LATTICE_ELEMENT_TO_STRING[lattice_element]}\n"
            system_context += \
                "\nAdditionally, here are several success return variables/macros " + \
                f"and their corresponding abstract values: {formatted_success_codes}\n"

        system_context += f"\n{formatted_error_specs}"

        messages=[
            {
                "role": "system",
                "content": system_context, 

            },
            # TODO(patrickjchap): This is hardcoded for debugging ATM.
            ANALYSIS_ANSWERS[0],
            ANALYSIS_ANSWERS[1],
            ANALYSIS_ANSWERS[2],
            ANALYSIS_ANSWERS[3],
            ANALYSIS_ANSWERS[4],
            ANALYSIS_ANSWERS[5],
            ANALYSIS_ANSWERS[6],
            ANALYSIS_ANSWERS[7],
            {
                "role": "user",
                "content": function_definition, 
            }
        ]

        results = self.completion_with_backoff(
            model=request.llm_name,
            messages=messages,
        )
        try:
            llm_message = results.choices[0].message.content
        except ValueError:
            print("Error in generated model result!!!")
            print("Model result:")
            print(results)

        print(f"{request.function_name} result:")
        print(llm_message)

        # The LLM will not necessarily give back both error specifications
        # for called and defined functions. It has been observed that the LLM
        # will typically give one or the other. If this happens, we need to
        # check if the function returned is called or defined, and then ask
        # for the opposite.
        reprompt=""
        specifications = dict()
        num_functions = len(llm_message.splitlines()) # We are making an assumption here on the output.
        inferred_defined = False
        for line in llm_message.splitlines():
            try:
                function_name, lattice_element_string = line.split(":")
                function_name = function_name.split()[-1]
                if function_name == request.function_name:
                    if num_functions == 1:
                        reprompt = "What about the called functions?"
                    inferred_defined = True
                # We want to know if the model doesn't know, but also need to
                # keep track of what it doesn't know, so we don't repeat things.
                if "bottom" in lattice_element_string.lower():
                    continue
                lattice_element = STRING_TO_LATTICE_ELEMENT[lattice_element_string.strip().lower().replace("'", "").replace('"', "")]
                specifications[function_name] = lattice_element
            except (ValueError, KeyError) as e:
                print(f"Error parsing result from model: {e}!")
                print(f"line: {line}")
                continue

        if not inferred_defined and not reprompt:
            reprompt = "What about the defined function?"

        if reprompt:
            messages.append({"role": "assistant", "content": llm_message})
            messages.append({"role": "user", "content": reprompt})

            results = self.completion_with_backoff(
                model=request.llm_name,
                messages=messages,
            )

            try:
                llm_message = results.choices[0].message.content
            except ValueError:
                print("Error in REPROMPT generated model result!!!")
                print("REPROMPT Model result:")
                print(results)
                llm_message = ""

            if llm_message:
                print(f"Reprompted {request.function_name} result:")
                print(llm_message)

            for line in llm_message.splitlines():
                try:
                    function_name, lattice_element_string = line.split(":")
                    function_name = function_name.split()[-1]
                    # Avoid any repeated functions for confusions sake.
                    if function_name in specifications or "bottom" in lattice_element_string.lower():
                        continue
                    # The model gets confused with pointers and our abstraction
                    # sometimes, so we just translate here.
                    if "null" in lattice_element_string.lower():
                        # It is unlikely that the LLM will give this, but just
                        # in case it does we need to be accurate.
                        if "!" in lattice_element_string: 
                            lattice_element_string = "!=0"
                        else:
                            lattice_element_string = "==0"
                    # Strip whitespace, make lowercase, remove single/double quotes.
                    lattice_element = STRING_TO_LATTICE_ELEMENT[lattice_element_string.strip().lower().replace("'", "").replace('"', "")]
                    specifications[function_name] = lattice_element
                except (ValueError, KeyError) as e:
                    print(f"Error parsing result line from REPROMPTED model: {e}!")
                    print(f"line: {line}")
                    continue
                
        return proto.gpt_pb2.GetGptSpecificationResponse(
            specifications=specifications)

def serve():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--listen",
        default="localhost:50059",
        help="The host address and port number in the form"
             " <host_address>:<port_number>",
    )
    args = parser.parse_args()

    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    proto.gpt_pb2_grpc.add_GptServiceServicer_to_server(
        GptServicer(),
        server)
    server.add_insecure_port(args.listen)
    print(f"Listening on {args.listen}")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
