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
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromAssociationTableProducer<edm::SimTrackContainer, SimClusterCollection>
    SimTrackToSimClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<edm::PCaloHitContainer, SimClusterCollection>
    CaloHitToSimClusterIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<SimClusterCollection, CaloParticleCollection>
    SimClusterToCaloParticleIndexTableProducer;
typedef ObjectIndexFromAssociationTableProducer<SimClusterCollection, SimClusterCollection>
    SimClusterToSimClusterIndexTableProducer;

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(SimTrackToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(CaloHitToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToCaloParticleIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToSimClusterIndexTableProducer);
