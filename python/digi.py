"""Package to configure the ECal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.
"""

from LDMX.Framework.ldmxcfg import Producer

class EcalDigiProducer(Producer) :
    """Configuration for EcalDigiProducer

    Warnings
    --------
    - Multiple samples per digi has not been implemented yet. All of the information goes into the sample of interest (SOI).
    
    """

    def __init__(self, instance_name = 'ecalDigis') :
        super().__init__(instance_name , 'ldmx::EcalDigiProducer','Ecal')

        self.gain = 2000.
        self.pedestal = 1100.
        self.nADCs = 10
        self.iSOI  = 0 

        import time
        self.randomSeed = int(time.time())
        self.ELECTRONS_PER_MIP = 37000.0; #e-
        self.CLOCK_CYCLE = 25.0; #ns
        self.MIP_SI_RESPONSE = 0.130; #MeV

        self.NUM_ECAL_LAYERS = 34
        self.NUM_HEX_MODULES_PER_LAYER = 7
        self.CELLS_PER_HEX_MODULE = 397

        self.noiseRMS = self.calculateNoise( 700. , 25. , 0.1 ) #MeV ~ 2.5e-3
        self.readoutThreshold = 4.*self.noiseRMS #MeV - readout threshold is 4sigma higher than noise average

        self.toaThreshold = 5.*self.MIP_SI_RESPONSE #MeV ~0.65MeV TOA Threshold is 5 MIPs
        self.totThreshold = 50.*self.MIP_SI_RESPONSE #MeV ~6.5MeV TOT Threshold is 50 MIPs

        self.timingJitter =  self.CLOCK_CYCLE / 100. #ns - chosen pretty arbitrarily

        self.makeConfigHists = False #should we make config hists

    def calculateNoise(self, noiseIntercept , noiseSlope , capacitance ) :
        """Calculate the Noise RMS (in units of MeV) from the capacitance of the readout pads.

        Uses the conversion of a MIP to energy deposited in Si (MIP_SI_RESPONSE)
        and the number of readout electrons generated by a MIP (ELECTRONS_PER_MIP)
        to convert from number of noise electrons to noise in energy MeV.

        Parameters
        ----------
        noiseIntercept : float
            Noise when there is no capacitance
        noiseSlope : float
            Ratio of noise in electrons to capacitance in pF of pads
        capacitance : float
            Actual capacitance of readout pads in pF
        """

        return (noiseIntercept + noiseSlope*capacitance)*(self.MIP_SI_RESPONSE/self.ELECTRONS_PER_MIP)

    def makeConfigHists(self) :
        """Turn on the creation and filling of configuration histograms"""

        self.makeConfigHists = True
        self.build2DHistogram( 'tot_SimE' ,
                    xlabel = "TOT [ns]", 
                    xbins = self.nADCs*self.CLOCK_CYCLE , xmin = 0 , xmax = self.nADCs*self.CLOCK_CYCLE,
                    ylabel = "Sim E [MeV]" , 
                    ybins = [ 0., 1e-3,
                        1e-2, 2e-2, 3e-2, 4e-2, 5e-2, 6e-2, 7e-2, 8e-2, 9e-2,
                        1e-1, 2e-1, 3e-1, 4e-1, 5e-1, 6e-1, 7e-1, 8e-1, 9e-1,
                        1., 2., 3., 4., 5., 6., 7., 8., 9.,
                        1e1, 2e1, 3e1, 4e1, 5e1 ] 
                    )

class EcalRecProducer(Producer) :
    """Configuration for the EcalRecProducer

    The layer weights and second order energy correction
    change when the ECal geometry changes, so we have setup
    various options for the different possible ECal geometries
    and their associated layer weights.
    """

    def __init__(self, instance_name = 'ecalRecon') : 
        super().__init__(instance_name , 'ldmx::EcalRecProducer','Ecal')

        self.digiCollName = 'EcalDigis'
        self.digiPassName = ''
        self.secondOrderEnergyCorrection = 1.
        self.layerWeights = [ ]

        from LDMX.DetDescr import EcalHexReadout
        self.hexReadout = EcalHexReadout.EcalHexReadout()

        self.v12() #use v12 geometry by default

    def v2(self) :
        """These layerWeights and energy correction were calculated at least before v3 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.hexReadout.v9()
        self.secondOrderEnergyCorrection = 0.948;
        self.layerWeights = [
            1.641, 3.526, 5.184, 6.841,
            8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
            8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
            16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45
            ]

    def v9(self) :
        """These layerWeights and energy correction were calculated for the v9 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.hexReadout.v9()
        self.secondOrderEnergyCorrection = 4000. / 4012.;
        self.layerWeights = [
            1.019, 1.707, 3.381, 5.022, 6.679, 8.060, 8.613, 8.613, 8.613, 8.613, 8.613,
            8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613,
            8.613, 12.480, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347,
            16.347, 8.334
            ]

    def v12(self) :
        """These layerWeights and energy correction were calculated for the v12 geometry.

        The second order energy correction is determined by comparing the mean of 1M single 4GeV
        electron events with 4GeV.
        """

        self.hexReadout.v12()
        self.secondOrderEnergyCorrection = 4000./4010.;
        self.layerWeights = [
            1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
            9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
            17.364, 8.990
            ]

