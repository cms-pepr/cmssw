from __future__ import print_function
from DataFormats import FWLite

events = FWLite.Events("test_RECO.root")
#track_handle = FWLite.Handle("std::vector<reco::Track>")
#events.getByLabel("generalTracks", track_handle)
#tracks = track_handle.product()
#t = tracks.at(0)
#
#trackAssoc_handle = FWLite.Handle("")
#events.getByLabel("trackingParticleRecoTrackAsssociation", trackAssoc_handle)
#trackAssoc = trackAssoc_handle.product()
#track_handle = FWLite.Handle("std::vector<reco::Track>")
#events.getByLabel("generalTracks", track_handle)
#tracks = track_handle.product()
#t = tracks.at(0)

tp_handle = FWLite.Handle("std::vector<TrackingParticle>")
events.getByLabel("mix:MergedTrackTruth", tp_handle)
trackingParticles = tp_handle.product()

sc_handle = FWLite.Handle("std::vector<SimCluster>")
events.getByLabel("hgcSimTruth", sc_handle)
simClusters = sc_handle.product()
events.getByLabel("mix:MergedCaloTruth", sc_handle)
simClustersUnmerged = sc_handle.product()

tpToSc_handle = FWLite.Handle("edm::AssociationMap<edm::OneToMany<vector<TrackingParticle>,vector<SimCluster>,unsigned int>>")
events.getByLabel("trackingParticleSimClusterAssociation", tpToSc_handle)
assoc = tpToSc_handle.product()

tpToScMerged_handle = FWLite.Handle("edm::AssociationMap<edm::OneToMany<vector<TrackingParticle>,vector<SimCluster>,unsigned int>>")
events.getByLabel("trackingParticleMergedSCAssociation", tpToScMerged_handle)
mergedAssoc = tpToScMerged_handle.product()

merged_associated_scs = []
for entry in mergedAssoc:
    tp = entry.key.get()
    scs = entry.val
    print("Size of SCs is", len(scs))
    
    print("TP pdgId is", tp.pdgId(), "energy is", tp.energy())
    for sc in scs:
        info = (sc.pdgId(), sc.energy(), sc.p())
        merged_associated_scs.append(info)
        print("-->Associated SC pdgId is", sc.pdgId(), "energy is", sc.energy())

print("-"*80)
associated_scs = []
for entry in assoc:
    tp = entry.key.get()
    scs = entry.val
    
    print("Size of SCs is", len(scs))
    
    print("TP pdgId is", tp.pdgId(), "energy is", tp.energy())
    for sc in scs:
        info = (sc.pdgId(), sc.energy(), sc.p())
        associated_scs.append(info)
        print("-->Associated SC pdgId is", sc.pdgId(), "energy is", sc.energy())

print("Size of tracking particles is", len(trackingParticles))
print("Size of merged simClusters is", len(simClusters))
print("Size of unmerged simClusters is", len(simClustersUnmerged))
print("Size of trackingParticle --> SimClustersMerged association is", len(mergedAssoc))
print("Size of trackingParticle --> SimClusters association is", len(assoc))
print("Number of unique associated merged simClusters is", len(set(merged_associated_scs)))
print("Number of duplicates was", len(merged_associated_scs)-len(set(merged_associated_scs)))
print("Number of unqiue associated unmerged simClusters is", len(set(associated_scs)))
print("Number of duplicates was", len(associated_scs)-len(set(associated_scs)))
print("> they are", str(set(merged_associated_scs)))
