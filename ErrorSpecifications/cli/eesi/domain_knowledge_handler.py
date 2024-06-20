"""For the handling and parsing of domain knowledge."""

import ast
import proto.eesi_pb2
import proto.domain_knowledge_pb2

# Mapping for SignLatticeElement enum in proto/eesi.proto
STRING_TO_LATTICE_ELEMENT = {
    "bottom": proto.eesi_pb2.SignLatticeElement \
              .SIGN_LATTICE_ELEMENT_BOTTOM,
    "emptyset": proto.eesi_pb2.SignLatticeElement \
              .SIGN_LATTICE_ELEMENT_BOTTOM,
    "<0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
    ">0": proto.eesi_pb2.SignLatticeElement \
          .SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
    "==0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO,
    "<=0": proto.eesi_pb2.SignLatticeElement \
           .SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
    ">=0": proto.eesi_pb2.SignLatticeElement \
           .SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
    "!=0": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_NOT_ZERO,
    "top": proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_TOP,
}

def parse_initial_specifications(file_path):
    """Parses a text file of initial specifications into the relevant protobuf.

    The format of the input file is:
        FN1 LATTICE_ELEMENT1
        FN2 LATTICE_ELEMENT2
        ...

    Args:
        initial_specifications_file_path: Absolute path to initial
            specifications file.

    Returns:
         A list of Specification protobuf messages.
    """
    # If the user does not provide an initial specifications file,
    # we assume no initial specifications domain knowledge.
    if not file_path:
        return []

    # Parsing the initial specifications text file into lines.
    with open(file_path, "r") as initial_specifications_file:
        initial_specifications_lines = [
            x.strip() for x in initial_specifications_file.readlines()]

    # Parsing initial_specifications_lines into respective protobuf messages.
    messages = []
    for line in initial_specifications_lines:
        function, lattice_element = line.split()
        # The LLVM name will be the same as the source name
        # as the user-provided domain knowledge wouldn't
        # typically include the LLVM name.
        function_message = proto.bitcode_pb2.Function(
            llvm_name=function,
            source_name=function,
        )
        specification_message = proto.eesi_pb2.Specification(
            function=function_message,
            lattice_element=STRING_TO_LATTICE_ELEMENT[lattice_element],
        )
        messages.append(specification_message)

    print(len(messages))

    return messages

def parse_error_only_definition(text):
    """Parses an error-only function definition into a protobuf.

    Args:
        text: The text to parse.

    Returns:
        An ErrorOnlyCall protobuf message.
    """
    func_name = None
    required_args = []

    if "(" in text:  # has argument list
        parsed_call = ast.parse(text.strip()).body[0].value
        assert isinstance(parsed_call, ast.Call)

        func_name = parsed_call.func.id

        for i, arg in enumerate(parsed_call.args):
            if isinstance(arg, ast.Ellipsis) or (
                    isinstance(arg, ast.Name) and arg.id == "_"
            ):
                continue

            required_arg = proto.domain_knowledge_pb2.ErrorOnlyArgument(
                position=i)

            if isinstance(arg, ast.Str):
                required_arg.value.string_value = arg.s
            elif isinstance(arg, ast.Num):
                required_arg.value.int_value = arg.n
            elif isinstance(arg, ast.Name) and arg.id in ["NULL", "nullptr"]:
                required_arg.value.int_value = 0
            else:
                raise ValueError(
                    'Error parsing "{}": Invalid argument at index {}'.format(
                        text,
                        i,
                    )
                )

            required_args.append(required_arg)
    else:
        func_name = text.strip()

    return proto.domain_knowledge_pb2.ErrorOnlyCall(
        function=proto.bitcode_pb2.Function(
            llvm_name=func_name,
            source_name=func_name,
        ),
        required_args=required_args,
    )

def parse_error_only(file_path):
    """Parses a text file of error-only functions into the relevant protobuf.

    Args:
        error_only_file_path: Absolute path to error only file.

    Returns:
        A list of Function protobuf messages corresponding to error functions.
    """
    # If the user does not provide an error only file,
    # we assume no domain knowledge.
    if not file_path:
        return []

    # Parsing error only file into lines.
    with open(file_path, "r") as error_only_file:
        return [
            parse_error_only_definition(line)
            for line in error_only_file.readlines()
        ]

def parse_error_codes(file_path):
    """Parses a text file of error codes into the relevant protobuf.

    The format of the input file is:
        ERROR_CODE_NAME1 -5 [SUBMODULE]*
        ERROR_CODE_NAME2 -6 [SUBMODULE]*
        ...

    Args:
        error_codes_file_path: Absolute path to the error codes file.

    Returns:
        Returns a list of ErrorCode protobuf messages.
    """
    # If the user does not provide an error codes file,
    # we assume no domain knowledge.
    if not file_path:
        return []

    # Parsing error code file into lines.
    with open(file_path, "r") as error_codes_file:
        split_lines = [
            line.strip().split() for line in error_codes_file.readlines()
        ]

        messages = []
        for name, value, *submodules in split_lines:
            formatted_submodules = [submodule if submodule.endswith("/")
                                    else submodule + "/"
                                    for submodule in submodules]
            messages.append(proto.domain_knowledge_pb2.ErrorCode(
                name=name, value=int(value, 0),
                submodules=formatted_submodules))

        return messages

def parse_success_codes(filepath):
    """Parses a text file of success codes into the relevant protobuf.

    The format of the input file is:
        SUCCESS_CODE_NAME1 0 [SUBMODULE]*
        SUCCESS_CODE_NAME2 1 [SUBMODULE]*
        ...

    Args:
        filepath: Absolute path to the success codes file.

    Returns:
        Returns a list of SuccessCode protobuf messages.
    """
    if not filepath:
        return []

    with open(filepath, "r") as success_codes_file:
        split_lines = [
            line.strip().split() for line in success_codes_file.readlines()
        ]
        messages = []
        for name, value, *submodules in split_lines:
            formatted_submodules = [submodule if submodule.endswith("/")
                                    else submodule + "/"
                                    for submodule in submodules]
            messages.append(proto.domain_knowledge_pb2.SuccessCode(
                name=name, value=int(value, 0),
                submodules=formatted_submodules))

        return messages

class DomainKnowledgeHandler:
    """Handles the parsing and configuration of domain knowledge."""

    def __init__(self, initial_specifications_path,
                 error_only_path, error_codes_path, success_codes_path):

        self.initial_specifications = parse_initial_specifications(
            initial_specifications_path)
        self.error_only = parse_error_only(
            error_only_path)
        self.error_codes = parse_error_codes(
            error_codes_path)
        self.success_codes = parse_success_codes(success_codes_path)

    def get_applicable_specifications(self, called_functions_response,
                                      defined_functions_response):
        """Returns the list of applicable specifications."""

        if not self.initial_specifications:
            return None

        spec_names = {}
        for spec in self.initial_specifications:
            spec_names[spec.function.source_name] = spec

        initial_specifications = {
            spec.function.source_name for spec in self.initial_specifications}

        called_functions = set()
        defined_functions = set()
        # Based off the protos defined in proto/bitcode.proto.
        if called_functions_response:
            called_functions = {
                spec.function.source_name
                for spec in called_functions_response.called_functions}
        if defined_functions_response:
            defined_functions = {
                func.source_name
                for func in defined_functions_response.functions}

        applicable_names = initial_specifications.intersection(
            called_functions.union(defined_functions))

        return [spec_names[x] for x in applicable_names]
