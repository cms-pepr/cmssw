# coding: utf-8

"""
Test config to run the WindowInference plugin.
"""


import os
import subprocess

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing


# # determine the location of _this_ file
# if "__file__" in globals():
#     this_dir = os.path.dirname(os.path.abspath(__file__))
# else:
#     this_dir = os.path.expandvars("$CMSSW_BASE/src/RecoHGCal/GraphReco/test")

# # ensure that the graph exists
# # if not, call the create_dummy_graph.py script in a subprocess since tensorflow complains
# # when its loaded twice (once here in python, once in c++)
# graph_path = os.path.join(this_dir, "graph.pb")
# if not os.path.exists(graph_path):
#     script_path = os.path.join(this_dir, "create_dummy_graph.py")
#     code = subprocess.call(["python", script_path, graph_path])
#     if code != 0:
#         raise Exception("create_dummy_graph.py failed")

# setup minimal options
options = VarParsing("python")
options.setDefault("inputFiles", "file:///eos/cms/store/cmst3/group/hgcal/CMG_studies/hgcalsim/sim.RecoTask/closeby_1.0To100.0_idsmix_dR0.3_n5_rnd1_s1/prod5/reco_2327_n100.root")
options.setDefault('outputFile', 'file:windowntup.root')
options.parseArguments()

#this one has some tracks


# define the process to run for the Phase2 era
from Configuration.Eras.Era_Phase2C8_cff import Phase2C8
process = cms.Process("HGR", Phase2C8)

# standard sequences and modules
process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.Geometry.GeometryExtended2026D41Reco_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.L1Reco_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.RecoSim_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.Validation_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')


# minimal configuration
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(100))
process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(options.inputFiles))

# global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic", "")

# process options
process.options = cms.untracked.PSet(
    allowUnscheduled=cms.untracked.bool(True),
    wantSummary=cms.untracked.bool(True),
)

process.TFileService = cms.Service("TFileService", fileName = cms.string(
    options.__getattr__("outputFile", noTags=True)))

# load and configure the windowInference module
from RecoHGCal.GraphReco.windowNTupler_cfi import WindowNTupler
process.WindowNTupler = WindowNTupler.clone()
process.WindowNTuplerDefaultTruth = WindowNTupler.clone()

process.hgcSimTruth = cms.EDProducer("HGCTruthProducer",
)

process.WindowNTupler.simClusters = "hgcSimTruth"

process.hgcSimTruthSequence = cms.Sequence(process.hgcSimTruth)
process.dump=cms.EDAnalyzer('EventContentAnalyzer')
# define the path to run
process.p = cms.Path(process.hgcSimTruthSequence * process.WindowNTupler * process.WindowNTuplerDefaultTruth)
