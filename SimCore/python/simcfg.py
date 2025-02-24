"""Internal configuration module for simulation objects

The simulation requires a lot more detailed configuration than
the other processors, so we have two extra objects that require
their own python classes.
"""

class UserAction: 
    """Object that stores parameters for a UserAction

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a UserAction
    class_name : str
        Name of C++ class that this UserAction should be
    """

    def __init__(self, instance_name, class_name):
        self.class_name    = class_name
        self.instance_name = instance_name

    def __str__(self): 
        """Stringify this UserAction

        Returns
        -------
        str
            A human-readable version of this UserAction printing all its attributes
        """

        string = "UserAction (" + self.__repr__() + ")\n"
        string += " Parameters: \n"
        for k, v in self.__dict__.items(): 
            string += "  %s : %s \n" % (k, v)

        return string

    def __repr__(self):
        """A shorter string representation of this UserAction

        Returns
        -------
        str
            Just printing its instance and class names
        """

        return '%s of class %s' % (self.instance_name, self.class_name)

class PrimaryGenerator: 
    """Object that stores parameters for a PrimaryGenerator

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a PrimaryGenerator
    class_name : str
        Name of C++ class that this PrimaryGenerator should be
    module_name : str, optional
        Name of C++ library that this primary generator is compiled into
    """

    def __init__(self, instance_name, class_name, module_name = 'SimCore_Generators'):
        self.class_name    = class_name
        self.instance_name = instance_name

        from LDMX.Framework import ldmxcfg
        ldmxcfg.Process.addModule(module_name)

    def __str__(self): 
        """Stringify this PrimaryGenerator

        Returns
        -------
        str
            A human-readable version of this PrimaryGenerator printing all its attributes
        """


        string = "PrimaryGenerator (" + self.__repr__() + ")\n" 
        string += " Parameters: \n"
        for k, v in self.__dict__.items(): 
            string += "  %s : %s \n" % (k, v)

        return string

    def __repr__(self):
        """A shorter string representation of this PrimaryGenerator

        Returns
        -------
        str
            Just printing its instance and class names
        """

        return '%s of class %s' % (self.instance_name, self.class_name)

class SensitiveDetector:
    """Configuration for a sensitive detector we want to load

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a PrimaryGenerator
    class_name : str
        Name of C++ class that this PrimaryGenerator should be
    module_name : str
        Name of C++ library that this primary generator is compiled into
    """

    def __init__(self, instance_name, class_name, module_name):
        self.class_name    = class_name
        self.instance_name = instance_name

        from LDMX.Framework import ldmxcfg
        ldmxcfg.Process.addModule(module_name)

    def __str__(self): 
        """Stringify this SensitiveDetector

        Returns
        -------
        str
            A human-readable version of this SensitiveDetector printing all its attributes
        """


        string = "SensitiveDetector (" + self.__repr__() + ")\n" 
        string += " Parameters: \n"
        for k, v in self.__dict__.items(): 
            string += "  %s : %s \n" % (k, v)

        return string

    def __repr__(self):
        """A shorter string representation of this SensitiveDetector

        Returns
        -------
        str
            Just printing its instance and class names
        """

        return '%s of class %s' % (self.instance_name, self.class_name)

class PhotoNuclearModel():
    """Configuration for a photonuclear model that we want to load

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a PhotoNuclear Model
    class_name : str
        Name of C++ class that this PhotoNuclear model should be
    module_name : str
        Name of C++ library that this PhotoNuclear model is compiled into
    """

    def __init__(self, instance_name, class_name, module_name ):
        self.class_name    = class_name
        self.instance_name = instance_name

        from LDMX.Framework import ldmxcfg
        ldmxcfg.Process.addModule(module_name)
