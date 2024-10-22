import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var,P3Vars

ecalPFClusTable = cms.EDProducer("SimplePFClusterFlatTableProducer",
    src = cms.InputTag("particleFlowClusterECAL"),
    cut = cms.string(""), 
    name = cms.string("PFClusterECAL"),
    doc  = cms.string("Particle flow cluters in ECAL"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), 
    variables = cms.PSet(P3Vars,
        depth = Var('depth', 'float', precision=14, doc='Cluster depth'),
        energy = Var('energy', 'float', precision=14, doc='vertex y pos'),
        correctedEnergy = Var('correctedEnergy', 'float', precision=14, doc='vertex z pos'),
        algo = Var('algo', 'int', precision=10, doc="Algo ID"),
        x = Var('x', 'float', precision=14, doc='x position'),
        y = Var('y', 'float', precision=14, doc='y position'),
        z = Var('z', 'float', precision=14, doc='z position'),
        time = Var('time', 'float', precision=14, doc='time'),
    )
)

hcalPFClusTable = ecalPFClusTable.clone()
hcalPFClusTable.src = "particleFlowClusterHCAL"
hcalPFClusTable.name = "PFClusterHCAL"
hcalPFClusTable.doc =  "Particle flow cluster in HGCAL"

hfPFClusTable = ecalPFClusTable.clone()
hfPFClusTable.src = "particleFlowClusterHF"
hfPFClusTable.name = "PFClusterHF"
hfPFClusTable.doc =  "Particle flow cluster in HGCAL"

pfClusterTables = cms.Sequence(
        ecalPFClusTable
        +hcalPFClusTable
        +hfPFClusTable
)
