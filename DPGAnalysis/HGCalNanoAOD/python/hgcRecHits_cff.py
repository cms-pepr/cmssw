import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import Var,P3Vars

hgcRecHits = cms.EDProducer("HGCRecHitCollectionMerger",
	src = cms.VInputTag("HGCalRecHit:HGCEERecHits",
        "HGCalRecHit:HGCHEFRecHits", "HGCalRecHit:HGCHEBRecHits",
	)	
)

hgcRecHitsTable = cms.EDProducer("SimpleCaloRecHitFlatTableProducer",
    src = cms.InputTag("hgcRecHits"),
    cut = cms.string(""), 
    name = cms.string("RecHitHGC"),
    doc  = cms.string("HGCAL RecHits"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(
        detId = Var('detid().rawId()', 'int', precision=-1, doc='detId'),
        energy = Var('energy', 'float', precision=14, doc='energy'),
        time = Var('time', 'float', precision=14, doc='hit time'),
    )
)

hgcRecHitsToPFCands = cms.EDProducer("RecHitToPFCandAssociationProducer",
    caloRecHits = cms.VInputTag("hgcRecHits"),
    pfCands = cms.InputTag("particleFlow"),
)

hgcRecHitsToPFCandTable = cms.EDProducer("CaloRecHitToPFCandIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = cms.string("PFCand"),
    objMap = cms.InputTag("hgcRecHitsToPFCands:hgcRecHitsToPFCand"),
    docString = cms.string("PFCand with most associated energy in RecHit DetId")
)

hgcRecHitsToPFTICLCands = cms.EDProducer("RecHitToPFCandAssociationProducer",
    caloRecHits = cms.VInputTag("hgcRecHits"),
    pfCands = cms.InputTag("pfTICL"),
)

hgcRecHitsToPFTICLCandTable = cms.EDProducer("CaloRecHitToPFCandIndexTableProducer",
    cut = hgcRecHitsTable.cut,
    src = hgcRecHitsTable.src,
    objName = hgcRecHitsTable.name,
    branchName = cms.string("PFTICLCand"),
    objMap = cms.InputTag("hgcRecHitsToPFTICLCands:hgcRecHitsToPFCand"),
    docString = cms.string("PFTICLCand with most associated energy in RecHit DetId")
)

hgcRecHitsPositionTable = cms.EDProducer("HGCRecHitPositionTableProducer",
    src = hgcRecHitsTable.src,
    cut = hgcRecHitsTable.cut, 
    name = hgcRecHitsTable.name,
    doc  = hgcRecHitsTable.doc,
)

hgcRecHitsSequence = cms.Sequence(hgcRecHits
				+hgcRecHitsTable
                +hgcRecHitsToPFCands
                +hgcRecHitsToPFCandTable
                +hgcRecHitsToPFTICLCands
                +hgcRecHitsToPFTICLCandTable
                +hgcRecHitsPositionTable
)
