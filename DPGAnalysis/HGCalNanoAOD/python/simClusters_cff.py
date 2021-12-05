import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var

simClusterTable = cms.EDProducer("SimpleSimClusterFlatTableProducer",
    src = cms.InputTag("mix:MergedCaloTruth"),
    cut = cms.string(""),
    name = cms.string("SimCluster"),
    doc  = cms.string("SimCluster information"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        lastPos_x = Var('g4Tracks.at(0).trackerSurfacePosition().x()', 'float', precision=14, doc='track x final position'),
        lastPos_y = Var('g4Tracks.at(0).trackerSurfacePosition().y()', 'float', precision=14, doc='track y final position'),
        lastPos_z = Var('g4Tracks.at(0).trackerSurfacePosition().z()', 'float', precision=14, doc='track z final position'),
        # NOTE: This is the cms-pepr approach, which is needed for merged simclusters
        impactPoint_x = Var('impactPoint().x()', 'float', precision=14, doc='x position'),
        impactPoint_y = Var('impactPoint().y()', 'float', precision=14, doc='y position'),
        impactPoint_z = Var('impactPoint().z()', 'float', precision=14, doc='z position'),
        impactPoint_t = Var('impactPoint().t()', 'float', precision=14, doc='Impact time'),
        impactPoint_eta = Var('impactPoint().eta()', 'float', precision=14, doc='eta at boundary'),
        impactPoint_phi = Var('impactPoint().phi()', 'float', precision=14, doc='phi at boundary'),
        # For stupid reasons lost on me, the nsimhits_ variable is uninitialized, and hits_ (which are really simhits)
        # are often referred to as rechits in the SimCluster class
        nHits = Var('numberOfRecHits', 'int', precision=-1, doc='number of simhits'),
        sumHitEnergy = Var('energy', 'float', precision=14, doc='total energy of simhits'),
        boundaryPmag = Var('impactMomentum.P()', 'float', precision=14, doc='magnitude of the boundary 3-momentum vector'),
        boundaryP4 = Var('impactMomentum.mag()', 'float', precision=14, doc='magnitude of four vector'),
        boundaryEnergy = Var('impactMomentum.energy()', 'float', precision=14, doc='magnitude of four vector'),
        boundaryEnergyNoMu = Var('impactMomentumNoMu.energy()', 'float', precision=14, doc='magnitude of four vector'),
        boundaryPt = Var('impactMomentum.pt()', 'float', precision=14, doc='magnitude of four vector'),
        trackId = Var('g4Tracks().at(0).trackId()', 'int', precision=-1, doc='Geant track id'),
        trackIdAtBoundary = Var('g4Tracks().at(0).getIDAtBoundary()', 'int', precision=-1, doc='Track ID at boundary'),
        hasHGCALHit = Var('hasHGCALHit', 'bool', doc='Has at least 1 simHit in HGCAL'),
        allHitsHGCAL = Var('allHitsHGCAL', 'bool', doc='all simHits are in HGCAL'),
        onHGCFrontFace = Var('abs(impactPoint().z()) - 322 < 1', 'bool', doc='SimCluster position is consistent with the front of the HGCAL'),
        isTrainable = Var('numberOfRecHits > 5 && allHitsHGCAL', 'bool', doc='Should be used for training'),
    )
)

simClusterToCaloPart = cms.EDProducer("SimClusterToCaloParticleAssociationProducer",
    caloParticles = cms.InputTag("mix:MergedCaloTruth"),
    simClusters = cms.InputTag("mix:MergedCaloTruth"),
)

simClusterToCaloPartTable = cms.EDProducer("SimClusterToCaloParticleIndexTableProducer",
    cut = simClusterTable.cut,
    src = simClusterTable.src,
    objName = simClusterTable.name,
    branchName = cms.string("CaloPart"),
    objMap = cms.InputTag("simClusterToCaloPart"),
    docString = cms.string("Index of CaloPart containing SimCluster")
)

hgcSimTruth = cms.EDProducer("simmerger",
    useNLayers = cms.int32(1),
    searchRadiusScale = cms.double(3.),
    clusterRadiusScale = cms.double(2.),
    mergeRadiusScale = cms.double(2.),
    energyContainment = cms.double(0.5),
    relOverlapDistance = cms.double(0.9)
                           )

hgcSimTruthDR = cms.EDProducer("HGCTruthProducer")

simClusterToMergedSCTable = cms.EDProducer("SimClusterToSimClusterIndexTableProducer",
    cut = simClusterTable.cut,
    src = simClusterTable.src,
    objName = simClusterTable.name,
    branchName = cms.string("MergedSimCluster"),
    objMap = cms.InputTag("hgcSimTruth"),
    docString = cms.string("Index of Merged SimCluster containing SimCluster")
)

mergedSimClusterTable = simClusterTable.clone()
mergedSimClusterTable.src = "hgcSimTruth"
mergedSimClusterTable.name = "MergedSimCluster"

mergedSimClusterDRTable = simClusterTable.clone()
mergedSimClusterDRTable.src = "hgcSimTruthDR"
mergedSimClusterDRTable.name = "MergedByDRSimCluster"

mergedToUnmergedSCTable = cms.EDProducer("SimClusterToSimClustersIndexTableProducer",
    cut = mergedSimClusterTable.cut,
    src = mergedSimClusterTable.src,
    objName = mergedSimClusterTable.name,
    branchName = cms.string("SimCluster"),
    objMap = cms.InputTag("hgcSimTruth"),
    docString = cms.string("Index of Merged SimCluster containing SimCluster")
)

simClusterTables = cms.Sequence(simClusterTable+simClusterToCaloPart+simClusterToCaloPartTable)

mergedSimClusterTables = cms.Sequence(hgcSimTruth+mergedSimClusterTable+hgcSimTruthDR+mergedSimClusterDRTable+mergedToUnmergedSCTable+simClusterToMergedSCTable)
