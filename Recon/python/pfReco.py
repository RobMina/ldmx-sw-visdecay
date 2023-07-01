"""Configuration for pfReco producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.Recon.pfReco import pfEcalClusterProducer
    p.sequence.append( pfEcalClusterProducer )
"""

from LDMX.Framework import ldmxcfg

class pfEcalClusterProducer(ldmxcfg.Producer) :
    """Configuration for Ecal cluster builder for particle reco"""
    def __init__(self, name='PFEcalCluster') :
        super().__init__(name, 'recon::PFEcalClusterProducer','Recon')
        self.hitCollName = 'EcalRecHits'
        self.clusterCollName = 'PFEcalClusters'
        self.doSingleCluster = True

class pfHcalClusterProducer(ldmxcfg.Producer) :
    """Configuration for Hcal cluster builder for particle reco"""
    def __init__(self, name='PFHcalCluster') :
        super().__init__(name, 'recon::PFHcalClusterProducer','Recon')
        self.hitCollName = 'HcalRecHits'
        self.clusterCollName = 'PFHcalClusters'
        self.doSingleCluster = True

class pfTrackProducer(ldmxcfg.Producer) :
    """Configuration for track selector for particle reco"""
    def __init__(self, name='PFTrack') :
        super().__init__(name, 'recon::PFTrackProducer','Recon')
        self.inputTrackCollName = 'EcalScoringPlaneHits'
        self.outputTrackCollName = 'PFTracks'

class pfProducer(ldmxcfg.Producer) :
    """Configuration for particle reco"""
    def __init__(self, name='PFlow') :
        super().__init__(name, 'recon::ParticleFlow','Recon')
        self.inputEcalCollName = 'PFEcalClusters'
        self.inputHcalCollName = 'PFHcalClusters'
        self.inputTrackCollName = 'PFTracks'
        self.outputCollName = 'PFCandidates'
  
class pfTruthProducer(ldmxcfg.Producer) :
    """Configuration for track selector for particle reco"""
    def __init__(self, name='PFTruth') :
        super().__init__(name, 'recon::PFTruthProducer','Recon')
        self.outputTargetCollName = 'PFTruthTarget'
        self.outputEcalCollName = 'PFTruthEcal'
        self.outputHcalCollName = 'PFTruthHcal'
