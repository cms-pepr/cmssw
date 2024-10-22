import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import hgcRecHitsTable

ticlTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
    src = cms.InputTag("ticlTrackstersMerge"),
    cut = cms.string(""), 
    name = cms.string("TICLCand"),
    doc  = cms.string("TICL Candidates"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        Vtx_x = Var('vertex().x()', 'float', precision=14, doc='vertex x pos'),
        Vtx_y = Var('vertex().y()', 'float', precision=14, doc='vertex y pos'),
        Vtx_z = Var('vertex().z()', 'float', precision=14, doc='vertex z pos'),
    )
)

hgcRecHitsToTiclCands = cms.EDProducer("RecHitToTICLCandidateAssociationProducer",
    caloRecHits = hgcRecHitsTable.src,
    layerClusters = cms.InputTag("hgcalLayerClusters"),
    ticlCandidates = ticlTable.src,
)

hgcRecHitToTiclTable = cms.EDProducer("RecHitToTICLCandidateIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = ticlTable.name,
    objMap = cms.InputTag("hgcRecHitsToTiclCands"),
    docString = cms.string("Index of TICLCand containing RecHit")
)


ticlTables = cms.Task(ticlTable, hgcRecHitsToTiclCands, hgcRecHitToTiclTable)

