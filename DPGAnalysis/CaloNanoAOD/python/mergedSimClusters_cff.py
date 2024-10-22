import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var
from DPGAnalysis.CaloNanoAOD.simClusters_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import *


closeBySimClusters = cms.EDProducer("CPtoSimClusters",
                                    caloParticles = cms.InputTag("mix:MergedCaloTruth"),
                                    simVertices = cms.InputTag("g4SimHits")
                                    )

hgcSimTruth = cms.EDProducer("SimClusterMerger",
                             
    ## only these three actually matter atm
    
    useNLayers = cms.int32(10),
    highEfracThreshold = cms.double(0.95), 
    connectThreshold = cms.double(.1),
    
    ###

    simClusters= cms.InputTag("mix:MergedCaloTruth"),
    simVertices= cms.InputTag("g4SimHits"),
    simTracks= cms.InputTag("g4SimHits"),
    caloRecHits = cms.InputTag("hgcRecHits")
    #caloSimHits = cms.InputTag("g4SimHits")
)

mergedSimClusterTable = simClusterTable.clone()
mergedSimClusterTable.src = "hgcSimTruth"
mergedSimClusterTable.name = cms.string("MergedSimCluster")
mergedSimClusterTable.variables.impactPoint_x = Var('impactPoint().x()', 'float', precision=14, doc='x position')
mergedSimClusterTable.variables.impactPoint_y = Var('impactPoint().y()', 'float', precision=14, doc='y position')
mergedSimClusterTable.variables.impactPoint_z = Var('impactPoint().z()', 'float', precision=14, doc='z position')
mergedSimClusterTable.variables.impactPoint_t = Var('impactPoint().t()', 'float', precision=14, doc='Impact time')
mergedSimClusterTable.variables.impactPoint_eta = Var('impactPoint().eta()', 'float', precision=14, doc='eta at boundary')
mergedSimClusterTable.variables.impactPoint_phi = Var('impactPoint().phi()', 'float', precision=14, doc='phi at boundary')
mergedSimClusterTable.variables.boundaryPmag = Var('impactMomentum.P()', 'float', precision=14, doc='magnitude of the boundary 3-momentum vector')
mergedSimClusterTable.variables.boundaryP4 = Var('impactMomentum.mag()', 'float', precision=14, doc='magnitude of four vector')
mergedSimClusterTable.variables.boundaryEnergy = Var('impactMomentum.energy()', 'float', precision=14, doc='magnitude of four vector')
mergedSimClusterTable.variables.boundaryEnergyNoMu = Var('impactMomentumNoMu.energy()', 'float', precision=14, doc='magnitude of four vector')
mergedSimClusterTable.variables.boundaryPt = Var('impactMomentum.pt()', 'float', precision=14, doc='magnitude of four vector')

mergedToUnmergedSCTable = cms.EDProducer("SimClusterToSimClustersIndexTableProducer",
    cut = mergedSimClusterTable.cut,
    src = mergedSimClusterTable.src,
    objName = mergedSimClusterTable.name,
    branchName = simClusterTable.name,
    objMap = cms.InputTag("hgcSimTruth"),
    docString = cms.string("Index of Merged SimCluster containing SimCluster")
)

# TODO: This is probably not the right place for this
hgcRecHitsToMergedSimClusters = cms.EDProducer("SimClusterRecHitAssociationProducer",
    caloRecHits = cms.VInputTag("hgcRecHits"),
    simClusters = cms.InputTag("hgcSimTruth"),
)

hgcRecHitsToMergedSimClusterTable = cms.EDProducer("HGCRecHitToSimClusterIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = cms.string("MergedSimCluster"),
    objMap = cms.InputTag("hgcRecHitsToMergedSimClusters:hgcRecHitsToSimClus"),
    bestMatchTable = cms.untracked.bool(True),
    docString = cms.string("MergedSimClusters ordered by most sim energy in RecHit DetId")
)


mergedSimClusterRecEnergyTable = cms.EDProducer("SimClusterRecEnergyTableProducer",
    src = cms.InputTag("hgcSimTruth"),
    cut = cms.string(""),
    objName = mergedSimClusterTable.name,
    branchName = cms.string("recEnergy"),
    valueMap = cms.InputTag("hgcRecHitsToMergedSimClusters"),
    docString = cms.string("SimCluster deposited reconstructed energy associated to SimCluster")
)

mergedSimClusterTables = cms.Sequence(closeBySimClusters+
    hgcSimTruth+
	mergedSimClusterTable+
	mergedToUnmergedSCTable+
	hgcRecHitsToMergedSimClusters+
	hgcRecHitsToMergedSimClusterTable+
	mergedSimClusterRecEnergyTable
)
