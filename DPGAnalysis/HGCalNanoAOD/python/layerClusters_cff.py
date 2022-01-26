import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import P3Vars,Var
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import hgcRecHitsTable

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
    branchName = cms.string("SimCluster"),
    objMap = cms.InputTag("layerClusterCaloParticleAssociationProducer"),
    docString = cms.string("Index of SimCluster matched to LayerCluster")
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

# For now there is only ever a 1 to 1 rechit to lc match
hgcRecHitsToBestLayerClusterTable = cms.EDProducer("CaloRecHitToBestLayerClusterIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = layerClusterTable.name,
    objMap = cms.InputTag("hgcRecHitsToLayerClusters:hgcRecHitsToBestLayerCluster"),
    docString = cms.string("LayerCluster to which the RecHit is associated")
)


layerClusterTables = cms.Sequence(layerClusterTable+layerClusterToSimClusterTable+
    hgcRecHitsToLayerClusters+hgcRecHitsToLayerClusterTable+hgcRecHitsToBestLayerClusterTable)

