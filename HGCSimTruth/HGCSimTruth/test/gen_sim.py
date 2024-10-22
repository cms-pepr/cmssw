# coding: utf-8

"""
Event generation and simulation for evaluating the realistic HGCal truth accumulation.
The config is based on workflow 20634.0 (ttbar).

> runTheMatrix.py -w upgrade -l 20634.0 --command="--no_exec" --dryRun
> cmsDriver.py gen_sim \
    --conditions auto:phase2_realistic_T14 \
    --era Phase2C8_timing_layer_bar \
    --geometry Extended2026D41 \
    --beamspot HLLHC14TeV \
    --datatier GEN-SIM \
    --eventcontent FEVTDEBUG \
    -s GEN,SIM \
    -n 10 \
    --no_exec
"""

import math

import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing


# helpers
def strip_prefix(path):
    return path.split(":", 1)[-1]


def ensure_prefix(path, prefix="file", empty=True):
    if ":" not in path:
        path = "{}:{}".format(prefix.rstrip(":"), path)
    return path


def calculate_rho(z, eta):
    return z * math.tan(2 * math.atan(math.exp(-eta)))


# option parsing
options = VarParsing("python")
options.setDefault("outputFile", "")
options.setDefault("maxEvents", 10)
options.register("content", "pythia_ttbar", VarParsing.multiplicity.singleton,
    VarParsing.varType.string, "event content")
options.parseArguments()

# file prefixes
output_file = options.__getattr__("outputFile", noTags=True)
if output_file:
    output_file = ensure_prefix(output_file)

# create the process
from Configuration.Eras.Era_Phase2C8_timing_layer_bar_cff import Phase2C8_timing_layer_bar
process = cms.Process("SIM", Phase2C8_timing_layer_bar)

# import of standard configurations
process.load("Configuration.StandardSequences.Services_cff")
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("SimGeneral.MixingModule.mixNoPU_cfi")
process.load("Configuration.Geometry.GeometryExtended2026D41Reco_cff")
process.load("Configuration.Geometry.GeometryExtended2026D41_cff")
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.Generator_cff")
process.load("IOMC.EventVertexGenerators.VtxSmearedHLLHC14TeV_cfi")
process.load("GeneratorInterface.Core.genFilterSummary_cff")
process.load("Configuration.StandardSequences.SimIdeal_cff")
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

# set the number of events to generate
process.maxEvents = cms.untracked.PSet(
    input=cms.untracked.int32(options.maxEvents),
    output=cms.optional.untracked.allowed(cms.int32, cms.PSet),
)

# global tag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic_T14", "")

# trigger conditions
process.genstepfilter.triggerConditions = cms.vstring("generation_step")

# we have no source
process.source = cms.Source("EmptySource")

# define the generator and, when empty, create an output file name
if options.content == "pythia_ttbar":
    process.generator = cms.EDFilter("Pythia8GeneratorFilter",
        PythiaParameters=cms.PSet(
            parameterSets=cms.vstring(
                "pythia8CommonSettings",
                "pythia8CUEP8M1Settings",
                "processParameters",
            ),
            processParameters=cms.vstring(
                "Top:gg2ttbar = on ",
                "Top:qqbar2ttbar = on ",
                "6:m0 = 175",
            ),
            pythia8CUEP8M1Settings=cms.vstring(
                "Tune:pp 14",
                "Tune:ee 7",
                "MultipartonInteractions:pT0Ref = 2.4024",
                "MultipartonInteractions:ecmPow = 0.25208",
                "MultipartonInteractions:expPow = 1.6",
            ),
            pythia8CommonSettings=cms.vstring(
                "Tune:preferLHAPDF = 2",
                "Main:timesAllowErrors = 10000",
                "Check:epTolErr = 0.01",
                "Beams:setProductionScalesFromLHEF = off",
                "SLHA:keepSM = on",
                "SLHA:minMassSM = 1000.",
                "ParticleDecays:limitTau0 = on",
                "ParticleDecays:tau0Max = 10",
                "ParticleDecays:allowPhotonRadiation = on",
            ),
        ),
        comEnergy=cms.double(14000.0),
        filterEfficiency=cms.untracked.double(1.0),
        maxEventsToPrint=cms.untracked.int32(0),
        pythiaHepMCVerbosity=cms.untracked.bool(False),
        pythiaPylistVerbosity=cms.untracked.int32(0),
    )
    output_file = output_file or "gensim_pythia_ttbar_n{}.root".format(options.maxEvents)
    output_file = ensure_prefix(output_file)

elif options.content.startswith("gun_"):
    # interpret the event content string
    opts = options.content.split("_")[1:]

    # get the particle name and ids
    particle_name = opts[0]
    particle_ids = {
        "e": [11, -11],
        "mu": [13, -13],
        "g": [22],
        "glu": [21],
        "pi0": [111],
        "pic": [211, -211],
    }
    if particle_name not in particle_ids:
        raise ValueError("unknown particle to shoot: {}".format(particle_name))

    # get energy and z ranges
    e_range = opts[1:3] if len(opts) >= 3 else ("50", "50")
    z_range = opts[3:5] if len(opts) >= 5 else ("0", "0")

    # define the generator
    process.generator = cms.EDProducer("CloseByFlatDeltaRGunProducer",
        # particle ids
        particleIDs=cms.vint32(*particle_ids[particle_name]),
        # max number of particles to shoot at a time
        nParticles=cms.int32(1),
        # shoot exactly the particles defined in particleIDs in that order
        exactShoot=cms.bool(False),
        # randomly shoot [1, nParticles] particles, each time randomly drawn from particleIDs
        randomShoot=cms.bool(False),
        # energy range
        eMin=cms.double(float(e_range[0])),
        eMax=cms.double(float(e_range[1])),
        # phi range
        phiMin=cms.double(-math.pi),
        phiMax=cms.double(math.pi),
        # eta range
        etaMin=cms.double(1.52),
        etaMax=cms.double(3.00),
        # longitudinal gun position in cm
        zMin=cms.double(float(z_range[0])),
        zMax=cms.double(float(z_range[1])),
        # deltaR settings
        deltaRMin=cms.double(0.),
        deltaRMax=cms.double(0.),
        # debug flag
        debug=cms.untracked.bool(True),
    )
    output_file = output_file or "gensim_gun_{}_e{}To{}_z{}To{}_n{}.root".format(particle_name,
        e_range[0], e_range[1], z_range[0], z_range[1], options.maxEvents)
    output_file = ensure_prefix(output_file)

else:
    raise ValueError("unknown 'content' value: {}".format(options.content))
process.ProductionFilterSequence = cms.Sequence(process.generator)

# output definition
print("output file: {}".format(output_file))
process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents=cms.untracked.PSet(
        SelectEvents=cms.vstring("generation_step"),
    ),
    dataset=cms.untracked.PSet(
        dataTier=cms.untracked.string("GEN-SIM"),
        filterName=cms.untracked.string("")
    ),
    fileName=cms.untracked.string(output_file),
    outputCommands=process.FEVTDEBUGEventContent.outputCommands,
    splitLevel=cms.untracked.int32(0),
)

# Path and EndPath definitions
process.generation_step = cms.Path(process.pgen)
process.simulation_step = cms.Path(process.psim)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

# schedule definition
process.schedule = cms.Schedule(
    process.generation_step,
    process.genfiltersummary_step,
    process.simulation_step,
    process.endjob_step,
    process.FEVTDEBUGoutput_step,
)

# add pat algos
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)

# filter all path with the production filter sequence
for path in process.paths:
    getattr(process, path).insert(0, process.ProductionFilterSequence)

# early deletion of intermediate products
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
