import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import Var,P3Vars
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import *

hgcRecHitsToSimClusters = cms.EDProducer("SimClusterRecHitAssociationProducer",
    caloRecHits = cms.VInputTag("hgcRecHits"),
    simClusters = cms.InputTag("mix:MergedCaloTruth"),
)

hgcRecHitsToSimClusterTable = cms.EDProducer("HGCRecHitToSimClusterIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = cms.string("SimCluster"),
    objMap = cms.InputTag("hgcRecHitsToSimClusters:hgcRecHitsToSimClus"),
    bestMatchTable = cms.untracked.bool(True),
    docString = cms.string("All SimCluster responsible for sim energy in RecHit DetId (ordered by fraction of energy)")
)

simClusterRecEnergyTable = cms.EDProducer("SimClusterRecEnergyTableProducer",
    src = cms.InputTag("mix:MergedCaloTruth"),
    cut = cms.string(""),
    objName = cms.string("SimCluster"),
    branchName = cms.string("recEnergy"),
    valueMap = cms.InputTag("hgcRecHitsToSimClusters"),
    docString = cms.string("SimCluster deposited reconstructed energy associated to SimCluster")
)

hgcRecHitSimAssociationTask = cms.Task(hgcRecHitsToSimClusters,
	simClusterRecEnergyTable,
	hgcRecHitsToSimClusterTable
)
