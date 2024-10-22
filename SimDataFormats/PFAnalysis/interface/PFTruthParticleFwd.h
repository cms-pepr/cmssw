#ifndef CaloAnalysis_PFTruthParticleFwd_h
#define CaloAnalysis_PFTruthParticleFwd_h
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Common/interface/RefVector.h"
#include <vector>

class PFTruthParticle;
typedef std::vector<PFTruthParticle> PFTruthParticleCollection;
typedef edm::Ref<PFTruthParticleCollection> PFTruthParticleRef;
typedef edm::RefVector<PFTruthParticleCollection> PFTruthParticleRefVector;
typedef edm::RefProd<PFTruthParticleCollection> PFTruthParticleRefProd;
typedef edm::RefVector<PFTruthParticleCollection> PFTruthParticleContainer;

#endif

