#include "PhysicsTools/NanoAOD/interface/ObjectIndexFromOneToManyQualAssociationProducer.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromOneToManyQualAssociationTableProducer<reco::TrackCollection, SimClusterCollection, float>
    RecoTrackToSimClusterIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<TrackingParticleCollection, edm::View<reco::Track>, double>
    TrackingParticleToTrackIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<edm::View<reco::Track>, TrackingParticleCollection, double>
    TrackToTrackingParticleIndexTableProducer;

DEFINE_FWK_MODULE(RecoTrackToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(TrackingParticleToTrackIndexTableProducer);
DEFINE_FWK_MODULE(TrackToTrackingParticleIndexTableProducer);
