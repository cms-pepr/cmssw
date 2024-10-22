#include <string>

#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "DataFormats/Common/interface/Association.h"

#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>

//
// class decleration
//
class SimHitToSimTrackAndClusterAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit SimHitToSimTrackAndClusterAssociationProducer(const edm::ParameterSet&);
  ~SimHitToSimTrackAndClusterAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<std::vector<SimTrack>> simTrackToken_;
  edm::EDGetTokenT<std::vector<PCaloHit>> simHitToken_;
  edm::EDGetTokenT<edm::Association<SimClusterCollection>> simTrackToSimClusterToken_;
};

SimHitToSimTrackAndClusterAssociationProducer::SimHitToSimTrackAndClusterAssociationProducer(const edm::ParameterSet& pset)
    : simTrackToken_(consumes<std::vector<SimTrack>>(pset.getParameter<edm::InputTag>("simTracks"))),
      simHitToken_(consumes<std::vector<PCaloHit>>(pset.getParameter<edm::InputTag>("simHits"))),
      simTrackToSimClusterToken_(consumes<edm::Association<SimClusterCollection>>(pset.getParameter<edm::InputTag>("simTrackToSimCluster"))) {
  produces<edm::Association<std::vector<SimTrack>>>();
  produces<edm::Association<SimClusterCollection>>();
}

SimHitToSimTrackAndClusterAssociationProducer::~SimHitToSimTrackAndClusterAssociationProducer() {}

void SimHitToSimTrackAndClusterAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<std::vector<PCaloHit>> shCollection;
  iEvent.getByToken(simTrackToken_, shCollection);
  std::unordered_map<size_t, size_t> trackIdToTrackIdx;

  edm::Handle<std::vector<SimTrack>> stCollection;
  iEvent.getByToken(simTrackToken_, stCollection);

  edm::Handle<edm::Association<SimClusterCollection>> stToscAssociation;
  iEvent.getByToken(simTrackToSimClusterToken_, stToscAssociation);

  for (size_t i = 0; i < stCollection->size(); i++) {
      trackIdToTrackIdx[stCollection->at(i).trackId()] = i;
  }

  std::vector<int> stIndices;
  std::vector<int> scIndices;
  for (auto& sh : *shCollection) {
     size_t id = sh.geantTrackId();
     int idx = trackIdToTrackIdx.find(id) != trackIdToTrackIdx.end() ? trackIdToTrackIdx[id] : -1;
     stIndices.push_back(idx);
     edm::Ref<std::vector<SimTrack>> st(stCollection, idx);
     edm::Ref<SimClusterCollection> sc = (*stToscAssociation)[st];
     scIndices.push_back(sc.isNonnull() ? sc.key() : -1);
  }

  auto assoc = std::make_unique<edm::Association<std::vector<SimTrack>>>(stCollection);
  edm::Association<std::vector<SimTrack>>::Filler filler(*assoc);
  filler.insert(shCollection, stIndices.begin(), stIndices.end());
  filler.fill();
  iEvent.put(std::move(assoc));
}

// define this as a plug-in
DEFINE_FWK_MODULE(SimHitToSimTrackAndClusterAssociationProducer);



