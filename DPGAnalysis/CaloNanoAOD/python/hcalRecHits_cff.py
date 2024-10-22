import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import Var,P3Vars

hbheRecHitTable = cms.EDProducer("SimpleCaloRecHitFlatTableProducer",
    src = cms.InputTag("hbhereco"),
    cut = cms.string(""), 
    name = cms.string("RecHitHBHE"),
    doc  = cms.string("HCAL barrel and endcap rec hits"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(
        detId = Var('detid().rawId()', 'int', precision=-1, doc='detId'),
        energy = Var('energy', 'float', precision=14, doc='energy'),
        time = Var('time', 'float', precision=14, doc='hit time'),
    )
)

hbheRecHitPositionTable = cms.EDProducer("HCALRecHitPositionTableProducer",
    src = hbheRecHitTable.src,
    cut = hbheRecHitTable.cut, 
    name = hbheRecHitTable.name,
    doc  = hbheRecHitTable.doc,
)

hfRecHitsTable = hbheRecHitTable.clone()
hfRecHitsTable.src = "hfreco"
hfRecHitsTable.name = "RecHitHF"
hfRecHitsTable.doc = "HCAL forward (HF) rechits"

hfRecHitPositionTable = hbheRecHitPositionTable.clone()
hfRecHitPositionTable.src = hfRecHitsTable.src
hfRecHitPositionTable.cut = hfRecHitsTable.cut
hfRecHitPositionTable.name = hfRecHitsTable.name
hfRecHitPositionTable.doc  = hfRecHitsTable.doc

hoRecHitsTable = hbheRecHitTable.clone()
hoRecHitsTable.src = "horeco"
hoRecHitsTable.name = "RecHitHO"
hoRecHitsTable.doc = "HCAL outer (HO) rechits"

hoRecHitPositionTable = hbheRecHitPositionTable.clone()
hoRecHitPositionTable.src = hoRecHitsTable.src
hoRecHitPositionTable.cut = hoRecHitsTable.cut
hoRecHitPositionTable.name = hoRecHitsTable.name
hoRecHitPositionTable.doc  = hoRecHitsTable.doc

hcalRecHitTables = cms.Sequence(
	hbheRecHitTable
	+hbheRecHitPositionTable 
    +hfRecHitsTable
    +hfRecHitPositionTable
    +hoRecHitsTable
    +hoRecHitPositionTable
)
