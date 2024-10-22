# coding: utf-8

"""
Test config to run the WindowInference plugin.
"""


import os
import subprocess

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing


# determine the location of _this_ file
if "__file__" in globals():
    this_dir = os.path.dirname(os.path.abspath(__file__))
else:
    this_dir = os.path.expandvars("$CMSSW_BASE/src/RecoHGCal/GraphReco/test")

# ensure that the graph exists
# if not, call the create_dummy_graph.py script in a subprocess since tensorflow complains
# when its loaded twice (once here in python, once in c++)
graph_path = os.path.join(this_dir, "graph.pb")
if not os.path.exists(graph_path):
    script_path = os.path.join(this_dir, "create_dummy_graph.py")
    code = subprocess.call(["python", script_path, graph_path])
    if code != 0:
        raise Exception("create_dummy_graph.py failed")

# setup minimal options
options = VarParsing("python")
options.setDefault("inputFiles", "file:///eos/cms/store/cmst3/group/hgcal/CMG_studies/hgcalsim/sim.RecoTask/closeby_1.0To100.0_idsmix_dR0.3_n5_rnd1_s1/prod5/reco_2327_n100.root")
options.parseArguments()

# define the process to run for the Phase2 era
from Configuration.Eras.Era_Phase2C8_cff import Phase2C8
process = cms.Process("HGR", Phase2C8)

# standard sequences and modules
process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.Geometry.GeometryExtended2023D41Reco_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# minimal configuration
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(10))
process.source = cms.Source("PoolSource", fileNames=cms.untracked.vstring(options.inputFiles))

# global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic", "")

# process options
process.options = cms.untracked.PSet(
    allowUnscheduled=cms.untracked.bool(True),
    wantSummary=cms.untracked.bool(True),
)

# load and configure the windowInference module
from RecoHGCal.GraphReco.windowInference_cfi import windowInference
process.windowInference = windowInference.clone(
    graphPath=cms.string(graph_path),
)

# define the path to run
process.p = cms.Path(process.windowInference)
