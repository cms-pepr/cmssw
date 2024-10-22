import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var

caloParticleTable = cms.EDProducer("SimpleCaloParticleFlatTableProducer",
    src = cms.InputTag("mix:MergedCaloTruth"),
    cut = cms.string(""), 
    name = cms.string("CaloPart"),
    doc  = cms.string("CaloPart"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        eventId = Var('eventId().event()', 'int', precision=-1, doc='Event ID (can be used to identify pileup'),
        bunchCrossing = Var('eventId().bunchCrossing()', 'int', precision=-1, doc='bunch crossing (can be used to identify pileup'),
        rawEventId = Var('eventId().rawId()', 'int', precision=-1, doc='Raw event ID (can be used to identify pileup'),
        simEnergy = Var('simEnergy', 'float', precision=-1, doc='Sum of the simHit energy'),
        energy = Var('energy', 'float', precision=-1, doc='energy of the four vector'),
        nGenPart = Var('genParticles().size()', 'int', precision=-1, doc='Number of associated gen particles'),
        GenPartIdx = Var('? genParticles.size() ? genParticles().at(0).key() : -1', 'int', precision=-1, doc='Number of associated gen particles'),
        nSimHit = Var('numberOfSimHits', 'int', precision=-1, doc='Number of simhits'),
        trackId = Var('g4Tracks().at(0).trackId', 'int', precision=-1, doc='Geant4 track ID of first track'),
        nSimTrack = Var('g4Tracks().size', 'int', precision=-1, doc='Number of associated simtracks'),
        crossedBoundary = Var('g4Tracks().at(0).crossedBoundary()', 'bool', doc='Crossed the HGCAL boundary')
    )
)

caloParticleTables = cms.Sequence(caloParticleTable)
