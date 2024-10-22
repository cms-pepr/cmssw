#include "PhysicsTools/NanoAOD/interface/ObjectIndexFromAssociationProducer.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticle.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticleFwd.h"
#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, reco::PFCandidateCollection>
    CaloRecHitToPFCandIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::View<CaloRecHit>, PFTruthParticleCollection>
    CaloRecHitToPFTruthParticleIndexTableProducer;

DEFINE_FWK_MODULE(CaloRecHitToPFCandIndexTableProducer);
DEFINE_FWK_MODULE(CaloRecHitToPFTruthParticleIndexTableProducer);
