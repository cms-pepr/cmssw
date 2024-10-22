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
class CaloParticleToMergedSimClustersProducer : public edm::stream::EDProducer<> {
public:
  explicit CaloParticleToMergedSimClustersProducer(const edm::ParameterSet&);
  ~CaloParticleToMergedSimClustersProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<CaloParticleCollection> caloPartToken_;
  edm::EDGetTokenT<SimClusterCollection> simClusterToken_;
  edm::EDGetTokenT<edm::Association<SimClusterCollection>> unmergedToMergedSCToken_;
};

CaloParticleToMergedSimClustersProducer::CaloParticleToMergedSimClustersProducer(const edm::ParameterSet& pset)
    : caloPartToken_(consumes<CaloParticleCollection>(pset.getParameter<edm::InputTag>("caloParticles"))),
      simClusterToken_(consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("mergedSimClusters"))),
      unmergedToMergedSCToken_(consumes<edm::Association<SimClusterCollection>>(pset.getParameter<edm::InputTag>("unmergedToMergedSimClusters"))) {
  produces<CaloParticleCollection>();
  produces<edm::Association<CaloParticleCollection>>();
}

CaloParticleToMergedSimClustersProducer::~CaloParticleToMergedSimClustersProducer() {}

void CaloParticleToMergedSimClustersProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<SimClusterCollection> mergedSCCollection;
  iEvent.getByToken(simClusterToken_, mergedSCCollection);

  edm::Handle<CaloParticleCollection> cpCollection;
  iEvent.getByToken(caloPartToken_, cpCollection);

  edm::Handle<edm::Association<SimClusterCollection>> unmergedToMergedSC;
  iEvent.getByToken(unmergedToMergedSCToken_, unmergedToMergedSC);

  auto refHash = [](SimClusterRef s) { return s.key(); };

  auto rekeyedCPs = std::make_unique<CaloParticleCollection>();
  std::vector<int> cpIndices(mergedSCCollection->size(), -1);
  for (size_t i = 0; i < cpCollection->size(); i++) {
      auto rekey = cpCollection->at(i);
      std::unordered_set<SimClusterRef, decltype(refHash)> mergedSCs(1, refHash);
      for (auto scRef : rekey.simClusters()) {
          auto msc = (*unmergedToMergedSC)[scRef];
          cpIndices[msc.key()] = i;
          mergedSCs.emplace(msc);
      }
      rekey.clearSimClusters();
      for (auto msc : mergedSCs)
        rekey.addSimCluster(msc);

      rekeyedCPs->emplace_back(rekey);
  }

  auto assoc = std::make_unique<edm::Association<CaloParticleCollection>>(cpCollection);
  edm::Association<CaloParticleCollection>::Filler filler(*assoc);
  filler.insert(mergedSCCollection, cpIndices.begin(), cpIndices.end());
  filler.fill();
  iEvent.put(std::move(assoc));
  iEvent.put(std::move(rekeyedCPs));
}

// define this as a plug-in
DEFINE_FWK_MODULE(CaloParticleToMergedSimClustersProducer);




