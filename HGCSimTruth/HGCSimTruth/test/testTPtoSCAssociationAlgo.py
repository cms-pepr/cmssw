# coding: utf-8
from DataFormats import FWLite

def getGeantTrackIds(obj):
    geantTracks = obj.g4Tracks()
    return [t.trackId() for t in geantTracks]

def makeTPtoSCMap(trackingParticles, simClusters):
    tp_map = {t.g4Tracks().at(0).trackId() : t for t in trackingParticles}
    tp_map = {}
    tp_sc_map = {}
    for tp in trackingParticles:
        trackIds = getGeantTrackIds(tp)
        for trackId in trackIds:
            if trackId in tp_map:
                print(trackId, tp_map)
                raise RuntimeError("Found track mapped to multiple tracking particles")
            tp_map[trackId] = tp
            tp_sc_map[tp] = []

    for sc in simClusters:
        trackIds = getGeantTrackIds(sc)
        for trackId in trackIds:
            if trackId in tp_map:
                tp = tp_map[trackId]
                tp_sc_map[tp].append(sc)

    return tp_sc_map

events = FWLite.Events("test_RECO.root")
for event in events:
    tp_handle = FWLite.Handle("std::vector<TrackingParticle>")
    event.getByLabel("mix:MergedTrackTruth", tp_handle)
    trackingParticles = tp_handle.product()

    sc_handle = FWLite.Handle("std::vector<SimCluster>")
    #event.getByLabel("mix:MergedCaloTruth", sc_handle)
    event.getByLabel("hgcSimTruth", sc_handle)
    simClusters = sc_handle.product()
    tp_sc_map = makeTPtoSCMap(trackingParticles, simClusters)
    print("Length of tracking particles is", len(trackingParticles))
    print("Length of simClusters is", len(simClusters))
    print("Length of tp-> sc is", len(tp_sc_map))
    associated_scs = set()
    unassociated_tps = set()
    map(lambda x: associated_scs.update(x), tp_sc_map.values())
    unassociated_tps = [k for (k,v) in tp_sc_map.iteritems() if not v]
    multassociated_tps = [k for (k,v) in tp_sc_map.iteritems() if len(v) > 1]
    print("Number of SCs associated to TPs", len(associated_scs))
    print("Number of unassociated TPs", len(unassociated_tps))
    print("Number of TPs associated to multiple SCs", len(multassociated_tps))
