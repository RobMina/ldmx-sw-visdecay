"""Biasing operator templates for use throughout ldmx-sw

Mainly focused on reducing the number of places that certain parameter and class names
are hardcoded into the python configuration.
"""

class XsecBiasingOperator:
    """Object that stores parameters for a XsecBiasingOperator

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a XsecBiasingOperator
    class_name : str
        Name of C++ class that this XsecBiasingOperator should be
    module_name : str, optional
        Name of module that the class was compiled into
    """

    def __init__(self, instance_name, class_name, module_name='SimCore_BiasOperators'):
        self.class_name    = class_name
        self.instance_name = instance_name

        from LDMX.Framework.ldmxcfg import Process
        Process.addLibrary( '@CMAKE_INSTALL_PREFIX@/lib/lib%s.so'%module_name )

    def __str__(self):
        """Stringify this XsecBiasingOperator

        Returns
        -------
        str
            A human-readable version of this XsecBiasingOperator printing all its attributes
        """

        string = "XsecBiasingOperator (" + self.__repr__() + ") {"
        for k, v in self.__dict__.items():
            string += " %s : %s" % (k, v)
        string += " }"

        return string

    def __repr__(self):
        """A shorter string representation of this XsecBiasingOperator

        Returns
        -------
        str
            Just printing its instance and class names
        """

        return '%s of class %s' % (self.instance_name, self.class_name)

class PhotoNuclear(XsecBiasingOperator) :
    """Bias photo nuclear process

    Parameters
    ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to mutliply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    down_bias_conv : bool, optional
        Should we down bias the gamma conversion process?
    only_children_of_primary : bool, optional
        Should we only bias photons that are children of the primary particle?
    """

    def __init__(self,vol,factor,thresh=0.,down_bias_conv=True,only_children_of_primary=False) :
        super().__init__('%s_bias_pn'%vol,'simcore::biasoperators::PhotoNuclear')

        self.volume = vol
        self.factor = factor
        self.threshold = thresh
        self.down_bias_conv = down_bias_conv
        self.only_children_of_primary = only_children_of_primary

class ElectroNuclear(XsecBiasingOperator) :
    """Bias electro nuclear process

    Parameters
    ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to mutliply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    def __init__(self,vol,factor,thresh=0.) :
        super().__init__('%s_bias_en'%vol,'simcore::biasoperators::ElectroNuclear')

        self.volume = vol
        self.factor = factor
        self.threshold = thresh

class GammaToMuPair(XsecBiasingOperator) :
    """Bias gamma -> mu+ mu- process

    Parameters
    ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to multiply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    def __init__(self,vol,factor,thresh=0.) :
        super().__init__('%s_bias_mumu'%vol,'simcore::biasoperators::GammaToMuPair')

        self.volume = vol
        self.factor = factor
        self.threshold = thresh


class NeutronInelastic(XsecBiasingOperator) :
    """Bias neutron inelastic collisions

    Parameters
    ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to multiply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    def __init__(self,vol,factor,thresh=0.) :
        super().__init__('%s_bias_neutroninelastic'%vol,'simcore::biasoperators::NeutronInelastic')

        self.volume = vol
        self.factor = factor
        self.threshold = thresh

class K0LongInelastic(XsecBiasingOperator) :
    """Bias K0 Long inelastic collisions

    Parameters
            ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to multiply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    def __init__(self,vol,factor,thresh=0.) :
        super().__init__('%s_bias_k0longinelastic'%vol,'simcore::biasoperators::K0LongInelastic')

        self.volume = vol
        self.factor = factor
        self.threshold = thresh

class DarkBrem(XsecBiasingOperator) :
    """Bias dark brem process

    Parameters
    ----------
    vol : str
        name of volume to bias within
    bial_all : bool
        Should we bias all electrons or just the primary?
    factor : float
        biasing factor to mutliply by
    """

    def __init__(self,vol,bias_all,factor) :
        super().__init__('%s_bias_darkbrem'%vol,'simcore::biasoperators::DarkBrem')

        self.volume = vol
        self.bias_all = bias_all
        self.factor = factor

    def ecal(factor) :
        """Bias dark brem inside of the ecal by the input factor"""
        return DarkBrem('ecal',True,factor)

    def target(factor) :
        """Bias dark brem inside of the target by the input factor"""
        return DarkBrem('target_region',False,factor)
