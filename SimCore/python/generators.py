"""Primary Generator templates for use throughout ldmx-sw

Mainly focused on reducing the number of places that certain parameter and class names
are hardcoded into the python configuration.
"""

from LDMX.SimCore import simcfg

class gun(simcfg.PrimaryGenerator) :
    """New basic particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Attributes
    ----------
    time : float, optional
        Time to shoot from [ns]
    verbosity : int, optional
        Verbosity flag for this generator
    particle : str
        Geant4 particle name to shoot
    energy : float
        Energy of particle to shoot [GeV]
    position : list of float
        Position to shoot from [mm]
    direction : list of float
        Unit vector direction to shoot from

    Examples
    --------
        myGun = gun( 'myGun' )
        myGun.particle = 'e-'
        myGun.energy = 4.0
        myGun.direction = [ 0., 0., 1. ]
        myGun.position = [ 0., 0., 0. ]
    """

    def __init__(self, name ) :
        super().__init__( name , "simcore::generators::ParticleGun" )

        self.time = 0.
        self.verbosity = 0
        self.particle = ''
        self.energy = 0.
        self.position = [ ]
        self.direction = [ ]

class multi(simcfg.PrimaryGenerator) :
    """New multi particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Attributes
    ----------
    enablePoisson : bool, optional
        Poisson-distribute number of particles?
    vertex : list of float
        Position to shoot particle(s) from [mm]
    momentum : list of float
        3-momentum to give particle(s) in [MeV]
    nParticles : int, optional
        Number of particles to shoot (or average of Poisson distribution)
    pdgID : int
        PDG ID of particle(s) to shoot
    """

    def __init__(self,name) :
        super().__init__(name,'simcore::generators::MultiParticleGunPrimaryGenerator')

        #turn off Poisson by default
        self.enablePoisson = False
        self.vertex = [ ]
        self.momentum = [ ]
        self.nParticles = 1
        self.pdgID = 0


class lhe(simcfg.PrimaryGenerator) :
    """New LHE file primary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to LHE file containing the primary vertices
    """

    def __init__(self,name,filePath):
        super().__init__(name,'simcore::generators::LHEPrimaryGenerator')

        self.filePath = filePath

class completeReSim(simcfg.PrimaryGenerator) :
    """New complete re-simprimary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    file_path : str
        path to ROOT file containing the SimParticles to re-simulate

    Attributes
    ----------
    collection_name : str
        Name of SimParticles collection to re-sim
    pass_name : str
        Pass name of SimParticles to re-sim
    """

    def __init__(self,name,file_path) :
        super().__init__(name,'simcore::generators::RootCompleteReSim')
        
        self.filePath = file_path
        self.collection_name = 'SimParticles'
        self.pass_name = ''

class ecalSP(simcfg.PrimaryGenerator) :
    """New ecal scoring planes primary generator

    Sets the collection name, pass name, and time cutoff
    to reasonable defaults.

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to ROOT file containing the EcalScoringPlanes to re-simulate


    Attributes
    ----------
    collection_name : str, optional
        Name of EcalScoringPlaneHits collection to re-sim
    pass_name : str, optional
        Pass name of EcalScoringPlaneHits to re-sim
    time_cutoff : float, optional
        Maximum time of scoring plane hit to still re-sim [ns]
    """

    def __init__(self,name,filePath) :
        super().__init__( name , 'simcore::generators::RootSimFromEcalSP' )

        self.filePath = filePath
        self.collection_name = 'EcalScoringPlaneHits'
        self.pass_name = ''
        self.time_cutoff = 50.

class gps(simcfg.PrimaryGenerator) :
    """New general particle source

    The input initialization commands are run in the order that they are listed.

    Parameters
    ----------
    name : str
        name of new primary generator
    initCommands : list of strings
        List of Geant4 commands to initialize this GeneralParticleSource

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a GeneralParticleSource with the passed initialization commands

    Examples
    --------
        myGPS = gps( 'myGPS' , [
            "/gps/particle e-",
            "/gps/pos/type Plane",
            "/gps/pos/shape Square",
            "/gps/pos/centre 0 0 0 mm",
            "/gps/pos/halfx 40 mm",
            "/gps/pos/halfy 80 mm",
            "/gps/ang/type cos",
            "/gps/ene/type Lin",
            "/gps/ene/min 3 GeV",
            "/gps/ene/max 4 GeV",
            "/gps/ene/gradient 1",
            "/gps/ene/intercept 1"
            ] )
    """

    def __init__(self,name,initCommands) :
        super().__init__(name,'simcore::generators::GeneralParticleSource')
        self.initCommands = initCommands

def _single_e_upstream_tagger(position, momentum, energy):
    """Internal helper function for creating electron beam guns upstream of tagger

    The guns position and momentum are determined by shooting a positron
    _backwards_ from the target without any detector components present. This
    uses Geant4 to calculate the trajectory within the magnetic field for us.
    Since the particle (if its high enough energy) will not interact with anything,
    you only need to generate a few events to get a good measurement.

    Parameters
    ----------
    position: list[float]
        end-point position of the positron shot backwards from target,
        is the start-point of an electron being shot into the target
    momentum: list[float]
        end-point momentum of the backwards positron **multiplied by -1**,
        defines the starting direction of an electron being shot into
        the target
    energy: float
        the energy of the electron in GeV, should be the same energy
        as the backwards positron used to deduce position and momentum

    Returns
    -------
    gun:
        configured instance of gun firing from far upstream of detector
    """

    import math
    momentum_mag = math.sqrt(sum(map(lambda x: x*x, momentum)))
    unit_direction = list(map(lambda x: x/momentum_mag, momentum))
    
    particle_gun = gun(f'single_{energy}gev_e_upstream_tagger')
    particle_gun.particle = 'e-'
    particle_gun.position = position
    particle_gun.direction = unit_direction
    particle_gun.energy = energy
    return particle_gun

def single_4gev_e_upstream_tagger() :
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at [0, 0, 0] if it isn't smeared and doesn't
    interact with any material. In reality, it will be smeared and it will interact
    with some material but we can dream.

    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron 
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [ -600.8976671223825, 0.0, -6000.0 ],
        [ 434.54301089006407, 0.0, 3976.8404804302777],
        4.0
    )

def single_4gev_e_upstream_target() :
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at approximately [0, 0, 0] (assuming 
    it's not smeared).
    
    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron 
    directly upstream of the target.
    """

    particle_gun = gun('single_4gev_e_upstream_target')
    particle_gun.particle = 'e-' 
    particle_gun.position = [ 0., 0., -1.2 ]  # mm
    particle_gun.direction = [ 0., 0., 1] 
    particle_gun.energy = 4.0 # GeV

    return particle_gun

def single_1pt2gev_e_upstream_tagger(): 
    """Configure a particle gun to fire a 8 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at [0, 0, 0] if it isn't smeared and doesn't
    interact with any material. In reality, it will be smeared and it will interact
    with some material but we can dream.

    Returns
    -------
    Instance of a particle gun configured to fire a single 8 GeV electron 
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [ -2083.3707473241993, 0.0, -6000.0 ],
        [ 425.132266424426, 0.0, 1122.7149711218378],
        1.2
    )

def single_8gev_e_upstream_tagger(): 
    """Configure a particle gun to fire a 8 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at [0, 0, 0] if it isn't smeared and doesn't
    interact with any material. In reality, it will be smeared and it will interact
    with some material but we can dream.

    Returns
    -------
    Instance of a particle gun configured to fire a single 8 GeV electron 
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [ -299.2386690686212, 0.0, -6000.0 ],
        [ 434.59663056485   , 0.0, 7988.698356992288],
        8.0
    )


def single_backwards_positron(energy: float):
    """A particle gun configured to shoot positrons backwards (i.e. upstream)
    from the target at the input energy.

    This generator is helpful for studying where electron guns of different energies should
    be started from if they should end up at the center of the target.

    Parameters
    ----------
    energy: float
        energy in GeV of the positron
    
    Returns
    -------
    gun:
        configured particle gun to shoot positrons backwards at the input energy
    """
    beam = gun(f'backwards-positron-{energy}GeV')
    beam.particle = 'e+'
    beam.position = [0., 0., 0.]
    beam.direction = [0., 0., -1.]
    beam.energy = energy
    return beam

