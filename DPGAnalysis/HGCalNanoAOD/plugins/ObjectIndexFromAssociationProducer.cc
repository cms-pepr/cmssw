#include "PhysicsTools/NanoAOD/interface/ObjectIndexFromAssociationProducer.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticle.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticleFwd.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/HGCalReco/interface/TICLCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromAssociationTableProducer<edm::SimTrackContainer, SimClusterCollection>
    SimTrackToSimClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::PCaloHitContainer, SimClusterCollection>
    CaloHitToSimClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, reco::PFCandidateCollection>
    CaloRecHitToPFCandIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, SimClusterCollection>
    CaloRecHitToBestSimClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, reco::CaloClusterCollection>
    CaloRecHitToBestLayerClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<SimClusterCollection, CaloParticleCollection>
    SimClusterToCaloParticleIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<SimClusterCollection, SimClusterCollection>
    SimClusterToSimClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, PFTruthParticleCollection>
    CaloRecHitToPFTruthParticleIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, std::vector<TICLCandidate>>
    RecHitToTICLCandidateIndexTableProducer;

DEFINE_FWK_MODULE(SimTrackToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(CaloHitToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToCaloParticleIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(CaloRecHitToPFCandIndexTableProducer);
DEFINE_FWK_MODULE(CaloRecHitToBestSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(CaloRecHitToBestLayerClusterIndexTableProducer);
DEFINE_FWK_MODULE(CaloRecHitToPFTruthParticleIndexTableProducer);
DEFINE_FWK_MODULE(RecHitToTICLCandidateIndexTableProducer);
