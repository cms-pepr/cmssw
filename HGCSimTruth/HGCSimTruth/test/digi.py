# coding: utf-8

"""
Event digitization for evaluating the realistic HGCal truth accumulation.
The config is based on workflows 20634.0 (ttbar with 200 pu) and 20434.0 (ttbar without pu).

200 pu:
> runTheMatrix.py -w upgrade -l 20634.0 --command="--no_exec" --dryRun
> cmsDriver.py digi \
    --conditions auto:phase2_realistic_T14 \
    --era Phase2C8_timing_layer_bar \
    --geometry Extended2026D41 \
    --datatier GEN-SIM-DIGI \
    --eventcontent FEVTDEBUGHLT \
    -s DIGI:pdigi_valid \
    -n 10 \
    --pileup_input das:/RelValMinBias_14TeV/CMSSW_10_6_0_patch2-106X_upgrade2023_realistic_v3_2023D41noPU-v1/GEN-SIM \
    --pileup AVE_200_BX_25ns \
    --no_exec

No pu:
(same as above without the two pileup arguments)
> runTheMatrix.py -w upgrade -l 20634.0 --command="--no_exec" --dryRun
> cmsDriver.py digi \
    --conditions auto:phase2_realistic_T14 \
    --era Phase2C8_timing_layer_bar \
    --geometry Extended2026D41 \
    --datatier GEN-SIM-DIGI \
    --eventcontent FEVTDEBUGHLT \
    -s DIGI:pdigi_valid \
    -n 10 \
    --no_exec
"""

import os

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing


# helpers
def strip_prefix(path):
    return path.split(":", 1)[-1]


def ensure_prefix(path, prefix="file"):
    if ":" not in path:
        path = "{}:{}".format(prefix.rstrip(":"), path)
    return path


# option parsing
options = VarParsing("python")
options.setDefault("inputFiles", [""])
options.setDefault("outputFile", "")
options.setDefault("maxEvents", 10)
options.register("pu", 0, VarParsing.multiplicity.singleton, VarParsing.varType.int, "pileup")
options.parseArguments()

# file prefixes
input_file = ensure_prefix(options.inputFiles[0])
output_file = options.__getattr__("outputFile", noTags=True)
if output_file:
    output_file = ensure_prefix(output_file)

# default output file
if not output_file:
    basename = os.path.splitext(strip_prefix(input_file).split("_", 1)[-1])[0]
    output_file = ensure_prefix("digi_{}_pu{}.root".format(basename, options.pu))

# create the process
from Configuration.Eras.Era_Phase2C8_timing_layer_bar_cff import Phase2C8_timing_layer_bar
process = cms.Process("DIGI", Phase2C8_timing_layer_bar)

# pileup mixing
mixing_cfi = "mix_POISSON_average_cfi" if options.pu > 0 else "mixNoPU_cfi"

# import of standard configurations
process.load("Configuration.StandardSequences.Services_cff")
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("SimGeneral.MixingModule.{}".format(mixing_cfi))
process.load("Configuration.Geometry.GeometryExtended2026D41Reco_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.Digi_cff")
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
        numberOfConcurrentIOVs=cms.untracked.uint32(1),
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
print("input file : {}".format(input_file))
process.source = cms.Source("PoolSource",
    dropDescendantsOfDroppedBranches=cms.untracked.bool(False),
    fileNames=cms.untracked.vstring(input_file),
    inputCommands=cms.untracked.vstring(
        "keep *",
        "drop *_genParticles_*_*",
        "drop *_genParticlesForJets_*_*",
        "drop *_kt4GenJets_*_*",
        "drop *_kt6GenJets_*_*",
        "drop *_iterativeCone5GenJets_*_*",
        "drop *_ak4GenJets_*_*",
        "drop *_ak7GenJets_*_*",
        "drop *_ak8GenJets_*_*",
        "drop *_ak4GenJetsNoNu_*_*",
        "drop *_ak8GenJetsNoNu_*_*",
        "drop *_genCandidatesForMET_*_*",
        "drop *_genParticlesForMETAllVisible_*_*",
        "drop *_genMetCalo_*_*",
        "drop *_genMetCaloAndNonPrompt_*_*",
        "drop *_genMetTrue_*_*",
        "drop *_genMetIC5GenJs_*_*",
    ),
    secondaryFileNames=cms.untracked.vstring(),
)

# configure the pu mixing
if options.pu > 0:
    process.mix.input.nbPileupEvents.averageNumber = cms.double(options.pu)
    process.mix.bunchspace = cms.int32(25)
    process.mix.minBunch = cms.int32(-12)
    process.mix.maxBunch = cms.int32(3)
    # process.mix.input.fileNames = cms.untracked.vstring(["/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/F7FE3FE9-565B-544A-855E-902BA4E3C5FD.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/82584FBA-A1E6-DF48-99BA-B1759C3A190F.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/F806295A-492F-EF4F-9D91-15DA8769DD72.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/6FCA2E1D-D1E2-514B-8ABA-5B71A2C1E1B3.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/287275CC-953A-0C4C-B352-E39EC2D571F0.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/657065A5-F35B-3147-AED9-E4ACA915C982.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/2C56BC73-5687-674C-8684-6C785A88DB78.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/B96F4064-156C-5E47-90A0-07475310157A.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/2564B36D-A0DB-6C42-9105-B1CFF44F311D.root', '/store/relval/CMSSW_10_6_0_patch2/RelValMinBias_14TeV/GEN-SIM/106X_upgrade2023_realistic_v3_2023D41noPU-v1/10000/2CB8C960-47C0-1A40-A9F7-0B62987097E0.root"])  # noqa: E501
    local_pu_dir = "/eos/user/m/mrieger/data/hgc/RelvalMinBias14"
    process.mix.input.fileNames = cms.untracked.vstring([
        "file://" + os.pathl.abspath(os.path.join(local_pu_dir, elem))
        for elem in os.path.listdir(local_pu_dir)
        if elem.endswith(".root")
    ])
process.mix.digitizers = cms.PSet(process.theDigitizersValid)

# output definition
print("output file: {}".format(output_file))
process.FEVTDEBUGHLToutput = cms.OutputModule("PoolOutputModule",
    dataset=cms.untracked.PSet(
        dataTier=cms.untracked.string("GEN-SIM-DIGI"),
        filterName=cms.untracked.string(""),
    ),
    fileName=cms.untracked.string(output_file),
    outputCommands=process.FEVTDEBUGHLTEventContent.outputCommands,
    splitLevel=cms.untracked.int32(0),
)

# Path and EndPath definitions
process.digitisation_step = cms.Path(process.pdigi_valid)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGHLToutput_step = cms.EndPath(process.FEVTDEBUGHLToutput)

# schedule definition
process.schedule = cms.Schedule(
    process.digitisation_step,
    process.endjob_step,
    process.FEVTDEBUGHLToutput_step,
)

# add pat algos
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# early deletion of intermediate products
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
