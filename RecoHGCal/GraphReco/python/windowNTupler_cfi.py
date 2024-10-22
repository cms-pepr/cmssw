# coding: utf-8

"""
Initialization file for the WindowNTupler module.
"""


__all__ = ["WindowNTupler"]


import math

import FWCore.ParameterSet.Config as cms


WindowNTupler = cms.EDAnalyzer("WindowNTupler",
    # the collections of rechits to use
    recHitCollections=cms.VInputTag(
        cms.InputTag("HGCalRecHit", "HGCEERecHits"),
        cms.InputTag("HGCalRecHit", "HGCHEFRecHits"),
        cms.InputTag("HGCalRecHit", "HGCHEBRecHits"),
    ),
    tracksters = cms.InputTag("ticlTrackstersMerge"),
    tracks = cms.InputTag("generalTracks"),
    layerClusters = cms.InputTag("hgcalLayerClusters"),
    #simClusters = cms.InputTag("mix", "RealisticCaloTruth"),
    simClusters = cms.InputTag("mix", "MergedCaloTruth"),
    tracksToTrackingParticles = cms.InputTag("trackingParticleRecoTrackAsssociation"),
    trackingParticleSimCluster = cms.InputTag("trackingParticleSimClusterAssociation"),
    
    removeFrameSimclusters=cms.bool(True),
    
    minEta=cms.double(1.4),
    maxEta=cms.double(3.1),
    # window size in phi and eta
    etaFrameWidth=cms.double(0.1),
    phiFrameWidth=cms.double(0.1),
    # overlap in phi and eta
    nEtaSegments=cms.uint32(1),
    nPhiSegments=cms.uint32(4),
    # names of the input and output tensors
)
