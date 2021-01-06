# coding: utf-8

"""
Initialization file for the WindowInference module.
"""


__all__ = ["peprCandidateFromHitProducer"]


import math

import FWCore.ParameterSet.Config as cms


peprCandidateFromHitProducer = cms.EDProducer("peprCandidateFromHitProducer",
    # the collections of rechits to use
    recHitCollections=cms.VInputTag(
        cms.InputTag("HGCalRecHit", "HGCEERecHits"),
        cms.InputTag("HGCalRecHit", "HGCHEFRecHits"),
        cms.InputTag("HGCalRecHit", "HGCHEBRecHits"),
    ),
    tracks = cms.InputTag("generalTracks"),
    minCandEnergy=cms.double(1.0),
    minEta=cms.double(1.6),
    maxEta=cms.double(3.0),
    Client = cms.PSet(
        mode = cms.string("PseudoAsync"),
        batchSize = cms.untracked.uint32(1),
        address = cms.untracked.string("dockerbuild.cern.ch"),
        port = cms.untracked.uint32(8001),
        timeout = cms.untracked.uint32(30),
        modelName = cms.string("hgcal_oc_reco"),
        modelVersion = cms.string(""),
        outputs = cms.untracked.vstring("output"),
        verbose = cms.untracked.bool(False),
        allowedTries = cms.untracked.uint32(0),
    )
)
