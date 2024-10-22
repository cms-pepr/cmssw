from __future__ import print_function
import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *
from PhysicsTools.NanoAOD.genparticles_cff import genParticleTable
from PhysicsTools.NanoAOD.genVertex_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcSimHits_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcSimTracks_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcRecHitSimAssociations_cff import *
from DPGAnalysis.CaloNanoAOD.simClusters_cff import *
from DPGAnalysis.CaloNanoAOD.mergedSimClusters_cff import *
from DPGAnalysis.CaloNanoAOD.caloParticles_cff import *
from DPGAnalysis.HGCalNanoAOD.layerClusters_cff import *
from DPGAnalysis.TrackNanoAOD.trackSimHits_cff import *
from DPGAnalysis.TrackNanoAOD.trackingParticles_cff import *
from DPGAnalysis.TrackNanoAOD.tracks_cff import *
from DPGAnalysis.PFNanoAOD.pfCands_cff import *
from DPGAnalysis.PFNanoAOD.pfTruth_cff import *

nanoMetadata = cms.EDProducer("UniqueStringProducer",
    strings = cms.PSet(
        tag = cms.string("untagged"),
    )
)

genParticleTable.src = "genParticles"
genParticleTable.variables = cms.PSet(genParticleTable.variables,
    charge = CandVars.charge)

nanoHGCMLSequence = cms.Sequence(nanoMetadata+
                                 hgcRecHits+ #so that modules can use them
    genVertexTable+genVertexT0Table+genParticleTable+
    cms.Sequence(layerClusterTables)+
    simTrackTables+
    hgcSimHitsSequence+
    # TODO: Fix producer and allow adding via configuration
    #trackerSimHitTables+
    simClusterTables+
    trackingParticleTables+
    caloParticleTables
)

nanoHGCMLRecoSequence = cms.Sequence(
    nanoHGCMLSequence+
	cms.Sequence(hgcRecHitsTask)+
    cms.Sequence(hgcRecHitSimAssociationTask)+
    cms.Sequence(trackTables)+
    trackingParticleTables+
    mergedSimClusterTables+
    pfCandTable+pfTruth+
    pfTICLCandTable
)

def customizeReco(process):
    process.nanoHGCMLSequence = nanoHGCMLRecoSequence
    return process

def customizeNoMergedCaloTruth(process):
    process.nanoHGCMLSequence.remove(simClusterTable)
    process.nanoHGCMLSequence.remove(simClusterToCaloPartTable)
    process.nanoHGCMLSequence.remove(hgcEEHitsToSimClusterTable)
    process.nanoHGCMLSequence.remove(hgcHEfrontHitsToSimClusterTable)
    process.nanoHGCMLSequence.remove(hgcHEbackHitsToSimClusterTable)
    process.nanoHGCMLSequence.remove(simTrackToSimClusterTable)

    process.nanoHGCMLSequence.remove(caloParticleTable)
    return process

def customizeMergedSimClusters(process):
    process.nanoHGCMLSequence.insert(-2, mergedSimClusterTables)
    process.nanoHGCMLSequence.insert(-1, caloParticleMergedTables)
    return process
