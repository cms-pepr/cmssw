import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import CandVars,Var,P3Vars

# I realize hardcoding this is awful but not sure if there's a way around it, 
# seems the following breaks things
#import ROOT
#ECALShowerMax = ROOT.reco.PFTrajectoryPoint.ECALShowerMax
ECALShowerMax = 5
HCALEntrance = 6
pfTrackTable = cms.EDProducer("SimplePFRecTrackFlatTableProducer",
    src = cms.InputTag("pfTrack"),
    cut = cms.string(""), 
    name = cms.string("PFTrack"),
    doc  = cms.string("Particle flow-specific track info"),
    singleton = cms.bool(False), # the number of entries is variable
    extension = cms.bool(False), 
    variables = cms.PSet(
        TrackIdx = Var("? trackRef().isNonnull() ? trackRef().key() : -1", "int", precision=10, doc="Matching track index"),
        positionAtECAL_x = Var(f"? extrapolatedPoint({ECALShowerMax}).isValid() ? extrapolatedPoint({ECALShowerMax}).position().x() : -999", "float", precision=14, doc="x position at shower max in ECAL"),
        positionAtECAL_y = Var(f"? extrapolatedPoint({ECALShowerMax}).isValid() ? extrapolatedPoint({ECALShowerMax}).position().y() : -999", "float", precision=14, doc="y position at shower max in ECAL"),
        positionAtECAL_z = Var(f"? extrapolatedPoint({ECALShowerMax}).isValid() ? extrapolatedPoint({ECALShowerMax}).position().z() : -999", "float", precision=14, doc="z position at shower max in ECAL"),
        positionAtHCAL_x = Var(f"? extrapolatedPoint({HCALEntrance}).isValid() ? extrapolatedPoint({HCALEntrance}).position().x() : -999", "float", precision=14, doc="x position at HCAL entrance"),
        positionAtHCAL_y = Var(f"? extrapolatedPoint({HCALEntrance}).isValid() ? extrapolatedPoint({HCALEntrance}).position().y() : -999", "float", precision=14, doc="y position at HCAL entrance"),
        positionAtHCAL_z = Var(f"? extrapolatedPoint({HCALEntrance}).isValid() ? extrapolatedPoint({HCALEntrance}).position().z() : -999", "float", precision=14, doc="z position at HCAL entrance"),
    )
)

pfTrackTables = cms.Sequence(
        pfTrackTable
)
