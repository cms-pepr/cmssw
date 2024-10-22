# coding: utf-8

"""
Event reconstruction for evaluating the realistic HGCal truth accumulation.
The config is based on workflow 20634.0 (ttbar).

> runTheMatrix.py -w upgrade -l 20634.0 --command="--no_exec" --dryRun
> cmsDriver.py reco \
    --conditions auto:phase2_realistic_T14 \
    --era Phase2C8_timing_layer_bar \
    --geometry Extended2026D41 \
    --datatier GEN-SIM-RECO \
    --eventcontent FEVTDEBUGHLT \
    -s RAW2DIGI,L1Reco,RECO,RECOSIM \
    -n 10 \
    --runUnscheduled \
    --no_exec
"""

import os

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing


# helpers
def strip_prefix(path):
    return path.split(":", 1)[-1]


def ensure_prefix(path, prefix="file", empty=True):
    if ":" not in path:
        path = "{}:{}".format(prefix.rstrip(":"), path)
    return path


# option parsing
options = VarParsing("python")
options.setDefault("inputFiles", [""])
options.setDefault("outputFile", "")
options.setDefault("maxEvents", 10)
options.parseArguments()

# file prefixes
input_file = ensure_prefix(options.inputFiles[0])
output_file = options.__getattr__("outputFile", noTags=True)
if output_file:
    output_file = ensure_prefix(output_file)

# default output file
if not output_file:
    basename = os.path.splitext(strip_prefix(input_file).split("_", 1)[-1])[0]
    output_file = ensure_prefix("reco_{}.root".format(basename))

# create the process
from Configuration.Eras.Era_Phase2C8_timing_layer_bar_cff import Phase2C8_timing_layer_bar
process = cms.Process("RECO", Phase2C8_timing_layer_bar)

# import of standard configurations
process.load("Configuration.StandardSequences.Services_cff")
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.Geometry.GeometryExtended2026D41Reco_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.RawToDigi_cff")
process.load("Configuration.StandardSequences.L1Reco_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("Configuration.StandardSequences.RecoSim_cff")
process.load("Configuration.StandardSequences.EndOfProcess_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

# process options
process.options = cms.untracked.PSet(
    FailPath=cms.untracked.vstring(),
    IgnoreCompletely=cms.untracked.vstring(),
    Rethrow=cms.untracked.vstring(),
    SkipEvent=cms.untracked.vstring(),
    allowUnscheduled=cms.obsolete.untracked.bool,
    canDeleteEarly=cms.untracked.vstring(),
    emptyRunLumiMode=cms.obsolete.untracked.string,
    eventSetup=cms.untracked.PSet(
        forceNumberOfConcurrentIOVs=cms.untracked.PSet(),
        numberOfConcurrentIOVs=cms.untracked.uint32(1)
    ),
    fileMode=cms.untracked.string("FULLMERGE"),
    forceEventSetupCacheClearOnNewRun=cms.untracked.bool(False),
    makeTriggerResults=cms.obsolete.untracked.bool,
    numberOfConcurrentLuminosityBlocks=cms.untracked.uint32(1),
    numberOfConcurrentRuns=cms.untracked.uint32(1),
    numberOfStreams=cms.untracked.uint32(0),
    numberOfThreads=cms.untracked.uint32(1),
    printDependencies=cms.untracked.bool(False),
    sizeOfStackForThreadsInKB=cms.optional.untracked.uint32,
    throwIfIllegalParameter=cms.untracked.bool(True),
    wantSummary=cms.untracked.bool(False),
)

# set the number of events to process
process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents),
    output=cms.optional.untracked.allowed(cms.int32, cms.PSet),
)

# global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic_T14", "")

# input source
process.source = cms.Source("PoolSource",
    fileNames=cms.untracked.vstring(input_file),
    secondaryFileNames=cms.untracked.vstring(),
)

# HGCSimTruth
process.hgcSimTruth = cms.EDProducer("HGCSimTruth",
)

# output definition
print("output file: {}".format(output_file))
process.FEVTDEBUGHLToutput = cms.OutputModule("PoolOutputModule",
    dataset=cms.untracked.PSet(
        dataTier=cms.untracked.string("GEN-SIM-DIGI-RECO"),
        filterName=cms.untracked.string(""),
    ),
    fileName=cms.untracked.string(output_file),
    outputCommands=process.FEVTDEBUGHLTEventContent.outputCommands,
    splitLevel=cms.untracked.int32(0),
)

# Path and EndPath definitions
process.raw2digi_step = cms.Path(process.RawToDigi)
process.L1Reco_step = cms.Path(process.L1Reco)
process.reconstruction_step = cms.Path(process.reconstruction)
process.recosim_step = cms.Path(process.recosim)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)

# schedule definition
process.schedule = cms.Schedule(
    process.raw2digi_step,
    process.L1Reco_step,
    process.reconstruction_step,
    process.recosim_step,
    process.hgcSimTruth,
    process.endjob_step,
    process.FEVTDEBUGHLToutput_step,
)

# add pat algos
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# ensure modules run unscheduled
from FWCore.ParameterSet.Utilities import convertToUnscheduled
process = convertToUnscheduled(process)

# early deletion of intermediate products
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
