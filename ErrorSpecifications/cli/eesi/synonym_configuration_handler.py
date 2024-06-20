"""For the handling of the configuration for SynonymFinderParameters."""

import proto.eesi_pb2

class SynonymConfigurationHandler:
    """Handles the configuration of SynonymFinderParameters."""

    def __init__(self, use_embedding=False,
                 minimum_evidence=None,
                 minimum_similarity=None,
                 expansion_operation=None):

        self.__use_embedding = use_embedding
        if use_embedding:
            assert minimum_evidence and minimum_similarity and expansion_operation
            self.__minimum_evidence = minimum_evidence
            self.__minimum_similarity = minimum_similarity
            # Default is just INVALID.
            self.__expansion_operation = \
                    proto.eesi_pb2.ExpansionOperationType.EXPANSION_OPERATION_INVALID
            if expansion_operation == "meet":
                self.__expansion_operation = \
                    proto.eesi_pb2.ExpansionOperationType.EXPANSION_OPERATION_MEET
            elif expansion_operation == "join":
                self.__expansion_operation = \
                    proto.eesi_pb2.ExpansionOperationType.EXPANSION_OPERATION_JOIN
            elif expansion_operation == "max":
                self.__expansion_operation = \
                    proto.eesi_pb2.ExpansionOperationType.EXPANSION_OPERATION_MAX

             

    def use(self):
        """Returns the value of use_embedding."""

        return self.__use_embedding

    def get_synonym_finder_parameters(self):
        """Returns a SynonymFinderParameters message for the set fields."""

        if not self.__use_embedding:
            return proto.eesi_pb2.SynonymFinderParameters()

        return proto.eesi_pb2.SynonymFinderParameters(
            minimum_evidence=self.__minimum_evidence,
            minimum_similarity=self.__minimum_similarity,
            expansion_operation=self.__expansion_operation)
