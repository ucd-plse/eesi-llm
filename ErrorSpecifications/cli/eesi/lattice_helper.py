"""For helping handle confidence values and SignLatticeElements. """

import proto.eesi_pb2

ELEMENT_ENCODING = {
    0: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_BOTTOM,
    1: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_ZERO,
    2: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_GREATER_THAN_ZERO,
    4: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_ZERO,
    5: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_LESS_THAN_EQUAL_ZERO,
    6: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_GREATER_THAN_EQUAL_ZERO,
    3: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_NOT_ZERO,
    7: proto.eesi_pb2.SignLatticeElement.SIGN_LATTICE_ELEMENT_TOP,
}

def get_lattice_element_from_confidence(specification, threshold):
    """ Returns a SignLatticeElement representative of confidence values. """
    assert threshold > 0

    bit_string = ""
    if specification.confidence_zero >= threshold: #and
            #specification.confidence_zero > specification.confidence_emptyset):
        bit_string += "1"
    else:
        bit_string += "0"
    if specification.confidence_greater_than_zero >= threshold: #and
            #specification.confidence_greater_than_zero >
            #specification.confidence_emptyset):
        bit_string += "1"
    else:
        bit_string += "0"
    if specification.confidence_less_than_zero >= threshold: #and
            #specification.confidence_less_than_zero >
            #specification.confidence_emptyset):
        bit_string += "1"
    else:
        bit_string += "0"

    # Special case: The confidence for <0, >0, and ==0 are all zero and there is
    # a non-zero confidence for emptyset.
    if (specification.confidence_emptyset >= threshold and
            int(bit_string, 2) == 0):
        return "emptyset"

    return ELEMENT_ENCODING[int(bit_string, 2)]
