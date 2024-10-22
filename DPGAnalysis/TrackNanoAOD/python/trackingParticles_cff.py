import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var
from SimTracker.TrackerHitAssociation.tpClusterProducer_cfi import *
from SimTracker.TrackAssociatorProducers.quickTrackAssociatorByHits_cfi import quickTrackAssociatorByHits
from SimTracker.TrackAssociation.trackingParticleRecoTrackAsssociation_cfi import trackingParticleRecoTrackAsssociation
from DPGAnalysis.TrackNanoAOD.tracks_cff import *

trackingParticleTable = cms.EDProducer("SimpleTrackingParticleFlatTableProducer",
    src = cms.InputTag("mix:MergedTrackTruth"),
    cut = cms.string(""), 
    name = cms.string("TrackingPart"),
    doc  = cms.string("TrackingPart"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), # this is the main table for the muons
    variables = cms.PSet(CandVars,
        eventId = Var('eventId().event()', 'int', precision=-1, doc='Event ID (can be used to identify pileup'),
        bunchCrossing = Var('eventId().bunchCrossing()', 'int', precision=-1, doc='bunch crossing (can be used to identify pileup'),
        rawEventId = Var('eventId().rawId()', 'int', precision=-1, doc='Raw event ID (can be used to identify pileup'),
        nGenPart = Var('genParticles().size()', 'int', precision=-1, doc='Number of associated gen particles'),
        GenPartIdx = Var('? genParticles.size() ? genParticles().at(0).key() : -1', 'int', precision=-1, doc='Number of associated gen particles'),
        trackId = Var('g4Tracks.at(0).trackId', 'int', precision=-1, doc='Geant4 track ID of first track'),
        nSimTrack = Var('g4Tracks().size', 'int', precision=-1, doc='Number of associated simtracks'),
        Vtx_x = Var('vx()', 'float', precision=14, doc='parent vertex x pos'),
        Vtx_y = Var('vy()', 'float', precision=14, doc='parent vertex y pos'),
        Vtx_z = Var('vz()', 'float', precision=14, doc='parent vertex z pos'),
        Vtx_t = Var('parentVertex().position().T()', 'float', precision=14, doc='parent vertex time'),
        nDecayVtx = Var('decayVertices().size()', 'int', precision=-1, doc='number of decay vertices'),
        DecayVtx_y = Var('? decayVertices().size() > 0 ? decayVertices().at(0).position().x : 10000', 'float', precision=14, doc='x position of first decay vertex'),
        DecayVtx_x = Var('? decayVertices().size() > 0 ? decayVertices().at(0).position().y : 10000', 'float', precision=14, doc='y position of first decay vertex'),
        DecayVtx_z = Var('? decayVertices().size() > 0 ? decayVertices().at(0).position().z : 10000', 'float', precision=14, doc='z position of first decay vertex'),
        DecayVtx_t = Var('? decayVertices().size() > 0 ? decayVertices().at(0).position().t : 10000', 'float', precision=14, doc='time of first decay vertex'),
    )
)

trackingParticleToSCTable = cms.EDProducer("TrackingParticleToTrackIndexTableProducer",
    cut = trackingParticleTable.cut,
    src = trackingParticleTable.src,
    objName = trackingParticleTable.name,
    branchName = cms.string("Track"),
    objMap = cms.InputTag("trackingParticleRecoTrackAsssociation"),
    docString = cms.string("Index of the matching reco::Tracks") 
)

trackToTrackingParticleTable = cms.EDProducer("TrackToTrackingParticleIndexTableProducer",
    cut = generalTrackTable.cut,
    src = generalTrackTable.src,
    objName = generalTrackTable.name,
    branchName = cms.string("TrackingPart"),
    objMap = cms.InputTag("trackingParticleRecoTrackAsssociation"),
    docString = cms.string("Index of the matching TrackingParticles") 
)

trackingParticleTables = cms.Sequence(trackingParticleTable
    +tpClusterProducer
    +quickTrackAssociatorByHits
    +trackingParticleRecoTrackAsssociation
    +trackToTrackingParticleTable
    +trackingParticleToSCTable
)
