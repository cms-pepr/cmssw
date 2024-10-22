#include <string>

#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "DataFormats/Common/interface/Association.h"

#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>

//
// class decleration
//
class SimClusterToCaloParticleAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit SimClusterToCaloParticleAssociationProducer(const edm::ParameterSet&);
  ~SimClusterToCaloParticleAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<CaloParticleCollection> caloPartToken_;
  edm::EDGetTokenT<SimClusterCollection> simClusterToken_;
};

SimClusterToCaloParticleAssociationProducer::SimClusterToCaloParticleAssociationProducer(const edm::ParameterSet& pset)
    : caloPartToken_(consumes<CaloParticleCollection>(pset.getParameter<edm::InputTag>("caloParticles"))),
      simClusterToken_(consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("simClusters"))) {
  produces<edm::Association<CaloParticleCollection>>();
}

SimClusterToCaloParticleAssociationProducer::~SimClusterToCaloParticleAssociationProducer() {}

void SimClusterToCaloParticleAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<SimClusterCollection> scCollection;
  iEvent.getByToken(simClusterToken_, scCollection);

  edm::Handle<CaloParticleCollection> cpCollection;
  iEvent.getByToken(caloPartToken_, cpCollection);

  std::vector<int> cpIndices(scCollection->size(), -1);
  for (size_t i = 0; i < cpCollection->size(); i++) {
      for (auto scRef : cpCollection->at(i).simClusters())
          cpIndices[scRef.key()] = i;
  }

  auto assoc = std::make_unique<edm::Association<CaloParticleCollection>>(cpCollection);
  edm::Association<CaloParticleCollection>::Filler filler(*assoc);
  filler.insert(scCollection, cpIndices.begin(), cpIndices.end());
  filler.fill();
  iEvent.put(std::move(assoc));
}

// define this as a plug-in
DEFINE_FWK_MODULE(SimClusterToCaloParticleAssociationProducer);



