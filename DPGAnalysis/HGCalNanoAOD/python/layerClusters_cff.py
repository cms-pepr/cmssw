import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import P3Vars,Var
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import hgcRecHitsTable
from DPGAnalysis.CaloNanoAOD.simClusters_cff import simClusterTable
from DPGAnalysis.CaloNanoAOD.caloParticles_cff import caloParticleTable
# TODO: Use this producer in other places
from RecoLocalCalo.HGCalRecProducers.hgcalRecHitMapProducer_cfi import hgcalRecHitMapProducer
from SimCalorimetry.HGCalSimProducers.hgcHitAssociation_cfi import lcAssocByEnergyScoreProducer, scAssocByEnergyScoreProducer
from SimCalorimetry.HGCalAssociatorProducers.LCToCPAssociation_cfi import layerClusterCaloParticleAssociation
from SimCalorimetry.HGCalAssociatorProducers.LCToSCAssociation_cfi import layerClusterSimClusterAssociation

layerClusterTable = cms.EDProducer("SimpleCaloClusterFlatTableProducer",
    src = cms.InputTag("hgcalLayerClusters"),
    cut = cms.string(""),
    name = cms.string("LayerCluster"),
    doc  = cms.string("LayerCluster information"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(
        eta  = Var("eta",  float,precision=12),
        phi = Var("phi", float, precision=12),
        energy = Var("energy", float, precision=14),
        x = Var('position().x()', 'float', precision=14, doc='x position'),
        y = Var('position.y()', 'float', precision=14, doc='y position'),
        z = Var('position.z()', 'float', precision=14, doc='z position'),
        nHits = Var('size', 'int', precision=14, doc='number of rechits'),
        seedDetId = Var('seed().rawId()', 'int', precision=-1, doc='detId of seed hit'),
    )
)

layerClusterToSimClusterTable = cms.EDProducer("LayerClusterToSimClusterIndexTableProducer",
    cut = layerClusterTable.cut,
    src = layerClusterTable.src,
    objName = layerClusterTable.name,
    branchName = simClusterTable.name, 
    objMap = cms.InputTag("layerClusterSimClusterAssociation"),
    docString = cms.string("Index of SimCluster matched to LayerCluster")
)

layerClusterToCaloParticleTable = cms.EDProducer("LayerClusterToCaloParticleIndexTableProducer",
    cut = layerClusterTable.cut,
    src = layerClusterTable.src,
    objName = layerClusterTable.name,
    branchName = caloParticleTable.name,
    objMap = cms.InputTag("layerClusterCaloParticleAssociation"),
    docString = cms.string("Index of CaloParticle matched to LayerCluster")
)

hgcRecHitsToLayerClusters = cms.EDProducer("RecHitToLayerClusterAssociationProducer",
    caloRecHits = cms.VInputTag("hgcRecHits"),
    layerClusters = cms.InputTag("hgcalLayerClusters"),
)

hgcRecHitsToLayerClusterTable = cms.EDProducer("HGCRecHitToLayerClusterIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = cms.string("LayerCluster"),
    objMap = cms.InputTag("hgcRecHitsToLayerClusters:hgcRecHitsToLayerCluster"),
    docString = cms.string("LayerCluster assigned largest RecHit fraction")
)

layerClusterTables = cms.Task(layerClusterTable,
    hgcalRecHitMapProducer,
    lcAssocByEnergyScoreProducer,
    scAssocByEnergyScoreProducer,
    layerClusterCaloParticleAssociation,
    layerClusterSimClusterAssociation,
    layerClusterToSimClusterTable,
    layerClusterToCaloParticleTable,
    hgcRecHitsToLayerClusters,
    hgcRecHitsToLayerClusterTable)
