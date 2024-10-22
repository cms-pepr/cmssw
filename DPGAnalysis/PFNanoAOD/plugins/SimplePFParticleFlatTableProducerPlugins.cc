#include "PhysicsTools/NanoAOD/interface/SimpleFlatTableProducer.h"

#include "SimDataFormats/PFAnalysis/interface/PFTruthParticle.h"
typedef SimpleFlatTableProducer<PFTruthParticle> SimplePFTruthParticleFlatTableProducer;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SimplePFTruthParticleFlatTableProducer);
