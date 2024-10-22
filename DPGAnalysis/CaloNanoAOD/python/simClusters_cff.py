import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var

boundaryPos = 'g4Tracks().at(0).getPositionAtBoundary()'
boundaryMom = 'g4Tracks().at(0).getMomentumAtBoundary()'

simClusterTable = cms.EDProducer("SimpleSimClusterFlatTableProducer",
    src = cms.InputTag("mix:MergedCaloTruth"),
    cut = cms.string(""),
    name = cms.string("SimCluster"),
    doc  = cms.string("SimCluster information"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        eventId = Var('eventId().event()', 'int', precision=-1, doc='Event ID (can be used to identify pileup'),
        bunchCrossing = Var('eventId().bunchCrossing()', 'int', precision=-1, doc='bunch crossing (can be used to identify pileup'),
        rawEventId = Var('eventId().rawId()', 'int', precision=-1, doc='Raw event ID (can be used to identify pileup'),
        lastPos_x = Var('g4Tracks.at(0).trackerSurfacePosition().x()', 'float', precision=14, doc='track x final position'),
        lastPos_y = Var('g4Tracks.at(0).trackerSurfacePosition().y()', 'float', precision=14, doc='track y final position'),
        lastPos_z = Var('g4Tracks.at(0).trackerSurfacePosition().z()', 'float', precision=14, doc='track z final position'),
        # For reasons lost on me, the nsimhits_ variable is uninitialized, and hits_ (which are really simhits)
        # are often referred to as rechits in the SimCluster class
        nHits = Var('numberOfRecHits', 'int', precision=-1, doc='number of simhits'),
        sumHitEnergy = Var('energy', 'float', precision=14, doc='total energy of simhits'),
        trackId = Var('g4Tracks().at(0).trackId()', 'int', precision=-1, doc='Geant track id'),
        trackIdAtBoundary = Var('g4Tracks().at(0).getIDAtBoundary()', 'int', precision=-1, doc='Track ID at boundary'),
        crossedBoundary = Var('g4Tracks().at(0).crossedBoundary()', 'bool', doc='Crossed the HGCAL boundary'),
		impactPoint_x = Var(f'{boundaryPos}.x()', 'float', precision=14, doc='x position'),
        impactPoint_y = Var(f'{boundaryPos}.y()', 'float', precision=14, doc='y position'),
        impactPoint_z = Var(f'{boundaryPos}.z()', 'float', precision=14, doc='z position'),
        impactPoint_t = Var(f'{boundaryPos}.t()', 'float', precision=14, doc='Impact time'),
        impactPoint_eta = Var(f'{boundaryMom}.eta()', 'float', precision=14, doc='eta at boundary'),
        impactPoint_phi = Var(f'{boundaryMom}.phi()', 'float', precision=14, doc='phi at boundary'),
        boundaryPmag = Var(f'{boundaryMom}.P()', 'float', precision=14, doc='magnitude of the boundary 3-momentum vector'),
        boundaryP4 = Var(f'{boundaryMom}.mag()', 'float', precision=14, doc='magnitude of four vector'),
        boundaryEnergy = Var(f'{boundaryMom}.energy()', 'float', precision=14, doc='magnitude of four vector'),
        boundaryPt = Var(f'{boundaryMom}.pt()', 'float', precision=14, doc='magnitude of four vector'),
    )
)

# pepr-specific modification
simClusterTable.variables = cms.PSet(simClusterTable.variables,
    hasHGCALHit = Var('hasHGCALHit', 'bool', doc='Has at least 1 simHit in HGCAL'),
    allHitsHGCAL = Var('allHitsHGCAL', 'bool', doc='all simHits are in HGCAL'),
    onHGCFrontFace = Var('abs(impactPoint().z()) - 322 < 1', 'bool', doc='SimCluster position is consistent with the front of the HGCAL'),
    isTrainable = Var('numberOfRecHits > 5 && allHitsHGCAL', 'bool', doc='Should be used for training'),
    boundaryEnergyNoMu = Var('impactMomentumNoMu.energy()', 'float', precision=14, doc='magnitude of four vector'),
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

simClusterTables = cms.Sequence(simClusterTable+simClusterToCaloPart+simClusterToCaloPartTable)
