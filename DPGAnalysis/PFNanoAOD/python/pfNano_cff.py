import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *
from PhysicsTools.NanoAOD.genparticles_cff import genParticleTable
from PhysicsTools.NanoAOD.genVertex_cff import *
from DPGAnalysis.TrackNanoAOD.trackingParticles_cff import *
from DPGAnalysis.TrackNanoAOD.tracks_cff import *
from DPGAnalysis.PFNanoAOD.pfCands_cff import *
from DPGAnalysis.PFNanoAOD.pfClusters_cff import *
from DPGAnalysis.PFNanoAOD.pfTracks_cff import *
from DPGAnalysis.PFNanoAOD.pfAssociations_cff import *
#from DPGAnalysis.PFNanoAOD.pfTruth_cff import *
from DPGAnalysis.CaloNanoAOD.hcalRecHits_cff import *
from DPGAnalysis.CaloNanoAOD.caloParticles_cff import *
from DPGAnalysis.CaloNanoAOD.simClusters_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcRecHits_cff import *
from DPGAnalysis.HGCalNanoAOD.hgcRecHitSimAssociations_cff import *
from DPGAnalysis.HGCalNanoAOD.layerClusters_cff import *
from DPGAnalysis.HGCalNanoAOD.ticlCandidates_cff import *
from Configuration.Eras.Modifier_phase2_hgcal_cff import phase2_hgcal

nanoMetadata = cms.EDProducer("UniqueStringProducer",
    strings = cms.PSet(
        tag = cms.string("untagged"),
    )
)

genParticleTable.src = "genParticles"
genParticleTable.variables = cms.PSet(genParticleTable.variables,
    charge = CandVars.charge)

# TODO: Get this working again
#pfNanoSimSeq = cms.Sequence(
#    simClusterTables
#    +trackingParticleTables
#    +caloParticleTables
#    #+pfTruth
#)
#
#hgcalTask = cms.Task()
#phase2_hgcal.toReplaceWith(hgcalTask,
#    cms.Task(hgcRecHitsTask,
#    	hgcRecHitSimAssociationTask,
#    	layerClusterTables,
#    	ticlTables
#	)
#)
#
#pfNanoSequence = cms.Sequence(nanoMetadata
#	+pfNanoSimSeq
#    +genVertexTable+genVertexT0Table+genParticleTable
#    +pfCandTables
#    +pfAssociationTables
#    +pfClusterTables
#    #+pfTrackTables
#    +cms.Sequence(trackTables)
#	+hcalRecHitTables
#	+cms.Sequence(hgcalTask)
#)

def customizeOutputForPF(process):
    import re
    output = [x for x in dir(process) if re.match(".*output$", x)]
    if not output:
        raise RuntimeError("No output module found in process. This shouldn't be possible")
    outputMod = getattr(process, output[0])
    outputMod.outputCommands.extend([
        "keep *_*_MergedCaloTruth_*", 
        "keep *_*_MergedTrackTruth_*",
        'keep recoPFRecTracks_*_*_*',
        'keep recoPFRecHits_*_*_*',
        "keep *_simSiStripDigis_*_*",
        "keep *_simSiPixelDigis_*_*",
    ])
    return process
