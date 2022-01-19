import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var
from DPGAnalysis.HGCalNanoAOD.simClusters_cff import mergedSimClusterTable
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import hgcRecHitsTable
from DPGAnalysis.TrackNanoAOD.trackingParticles_cff import trackingParticleTable
from DPGAnalysis.TrackNanoAOD.tracks_cff import generalTrackTable

pfTruthParticles = cms.EDProducer("PFTruthParticleProducer",
    trackingParticles= cms.InputTag("mix:MergedTrackTruth"),
    caloParticles= cms.InputTag("mergedSCCaloParts"),
    simClusters = cms.InputTag("hgcSimTruth"),
    simVertices= cms.InputTag("g4SimHits"),
    simTracks= cms.InputTag("g4SimHits"),
    caloRecHits = cms.InputTag("hgcRecHits"),
    tracks = generalTrackTable.src,
    rechitToSimClusAssoc = cms.InputTag("hgcRecHitsToMergedSimClusters:hgcRecHitsToBestSimClus"), 
    trackingPartToTrackAssoc = cms.InputTag("trackingParticleRecoTrackAsssociation"), 
)

pfTruthTable = cms.EDProducer("SimplePFTruthParticleFlatTableProducer",
    src = cms.InputTag("pfTruthParticles"),
    cut = cms.string(""), 
    name = cms.string("PFTruthPart"),
    doc  = cms.string("PFTruthParticles"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        nSimCluster = Var("nSimCluster", 'int', precision=-1, doc='Number of associated SimClusters'),
        nTrackingPart = Var("nTrackingParticle", 'int', precision=-1, doc='Number of associated SimClusters'),
    )
)

simClusterToPFTruthTable = cms.EDProducer("SimClusterToPFTruthParticleIndexTableProducer",
    cut = cms.string(""),
    src = mergedSimClusterTable.src,
    objName = mergedSimClusterTable.name,
    branchName = pfTruthTable.name,
    objMap = cms.InputTag("pfTruthParticles:simClusToPFTruth"),
    docString = cms.string("PFTruth particle to which the SimCluster is associated")
)

trackingPartToPFTruthTable = cms.EDProducer("TrackingParticleToPFTruthParticleIndexTableProducer",
    cut = cms.string(""),
    src = cms.InputTag("mix:MergedTrackTruth"),
    objName = trackingParticleTable.name,
    branchName = pfTruthTable.name,
    objMap = cms.InputTag("pfTruthParticles:trackingPartToPFTruth"),
    docString = cms.string("PFTruth particle to which the TrackingPart is associated")
)

trackToPFTruthTable = cms.EDProducer("TrackToPFTruthParticleIndexTableProducer",
    cut = generalTrackTable.cut,
    src = generalTrackTable.src,
    objName = generalTrackTable.name,
    branchName = pfTruthTable.name,
    objMap = cms.InputTag("pfTruthParticles:trackToPFTruth"),
    docString = cms.string("PFTruth particle to which the reco::Track is associated (via the TrackingParticle match)")
)

recHitToPFTruthTable = cms.EDProducer("CaloRecHitToPFTruthParticleIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = cms.string("BestPFTruthPart"),
    objMap = cms.InputTag("pfTruthParticles:caloRecHitToPFTruth"),
    docString = cms.string("PFTruthParticle to which the RecHit is matched (via the sim cluster)")
)

pfTruth = cms.Sequence(pfTruthParticles+pfTruthTable
        +simClusterToPFTruthTable
        +trackingPartToPFTruthTable
        +trackToPFTruthTable
        +recHitToPFTruthTable 
        )

