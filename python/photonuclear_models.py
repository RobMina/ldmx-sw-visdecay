"""Configuration classes for default photonuclear models"""

from LDMX.SimCore import simcfg


class BertiniModel(simcfg.PhotonuclearModel):
    """The default model for photonuclear interactions.

    Keeps the default Bertini model from Geant4.
    """

    def __init__(self):
        super().__init__('BertiniModel',
                         'simcore::BertiniModel',
                         'SimCore_PhotonuclearModels')

class BertiniNothingHardModel(simcfg.PhotonuclearModel):
    """A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

    """

    def __init__(self):
        """
        Nothing hard events are unlikely to come from low A nuclei. This can
        be tested by instrumenting one of the models and checking the typical
        number of attempts for different nuclei.

        If we count light ions and nuclei as potential hard particles and
        ignore events producing heavy exotic particles, then here are some
        example results:

        Z74 -> ~5000 attempts

        While for lighter nuclei, perhaps unsurprsingly
        Z1  -> 1e5 attempts before giving up
        Z6  -> 1e5 attempts before giving up
        Z7  -> 1e5 attempts before giving up
        Z8  -> 1e5 attempts before giving up
        Z11 -> 1e5 attempts before giving up
        Z14 -> 1e5 attempts before giving up
        Z20 -> 1e5 attempts before giving up
        Z29 -> 30000 attempts

        In a similar manner, we can check how often a photonuclear interaction
        is with a particular nucleus in an ECal PN sample

        |Nucleus | Rate [%] |
        |  H     |     4.78 |
        |  C     |     9.73 |
        |  N     |     0.04 |
        |  O     |     7.01 |
        |  Ni    |     1.25 |
        |  Al    |     0.01 |
        |  Si    |     6.23 |
        |  Ca    |     1.36 |
        |  Mn    |     0.01 |
        |  Fe    |     0.67 |
        |  Cu    |    12.47 |
        |  W     |    56.44 |

        So for the purposes of running Nothing hard simulations, it is
        probably fine to have a high `zmin` value, either 29 (Cu) or 74 (W).
        Single hard particle events, however, can come from any kind of
        nucleus.
        """

        super().__init__('BertiniNothingHardModel',
                         'simcore::BertiniNothingHardModel',
                         'SimCore_PhotonuclearModels')
        self.count_light_ions = True
        self.hard_particle_threshold = 200.
        self.zmin = 74
        self.emin = 2500.
class BertiniSingleNeutronModel(simcfg.PhotonuclearModel):
    """ A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

    """

    def __init__(self):
        super().__init__('BertiniSingleNeutronModel',
                         'simcore::BertiniSingleNeutronModel',
                         'SimCore_PhotonuclearModels')
        self.hard_particle_threshold = 200.
        self.zmin = 0
        self.emin = 2500.
        self.count_light_ions = True
