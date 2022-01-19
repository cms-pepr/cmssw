#include "PhysicsTools/NanoAOD/interface/ObjectIndexFromAssociationProducer.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticleFwd.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticle.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromAssociationTableProducer<SimClusterCollection, PFTruthParticleCollection>
    SimClusterToPFTruthParticleIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<TrackingParticleCollection, PFTruthParticleCollection>
    TrackingParticleToPFTruthParticleIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<reco::Track>, PFTruthParticleCollection>
    TrackToPFTruthParticleIndexTableProducer;
DEFINE_FWK_MODULE(TrackingParticleToPFTruthParticleIndexTableProducer);
DEFINE_FWK_MODULE(TrackToPFTruthParticleIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToPFTruthParticleIndexTableProducer);
