#include <string>

#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "DataFormats/Common/interface/Association.h"

#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>

//
// class decleration
//
class SimTrackToSimClusterAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit SimTrackToSimClusterAssociationProducer(const edm::ParameterSet&);
  ~SimTrackToSimClusterAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<std::vector<SimTrack>> simTrackToken_;
  edm::EDGetTokenT<SimClusterCollection> simClusterToken_;
};

SimTrackToSimClusterAssociationProducer::SimTrackToSimClusterAssociationProducer(const edm::ParameterSet& pset)
    : simTrackToken_(consumes<std::vector<SimTrack>>(pset.getParameter<edm::InputTag>("simTracks"))),
      simClusterToken_(consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("simClusters"))) {
  produces<edm::Association<SimClusterCollection>>();
}

SimTrackToSimClusterAssociationProducer::~SimTrackToSimClusterAssociationProducer() {}

void SimTrackToSimClusterAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<SimClusterCollection> scCollection;
  iEvent.getByToken(simClusterToken_, scCollection);
  std::unordered_map<size_t, size_t> trackIdToSCIdx;

  edm::Handle<std::vector<SimTrack>> stCollection;
  iEvent.getByToken(simTrackToken_, stCollection);

  for (size_t i = 0; i < scCollection->size(); i++) {
      for (auto& track : scCollection->at(i).g4Tracks()) {
          trackIdToSCIdx[track.trackId()] = i;
      }
  }

  std::vector<int> scIndices;
  for (auto& track : *stCollection) {
    size_t id = track.trackId();
    int idx = trackIdToSCIdx.find(id) != trackIdToSCIdx.end() ? trackIdToSCIdx[id] : -1;
    scIndices.push_back(idx);
  }


  auto assoc = std::make_unique<edm::Association<SimClusterCollection>>(scCollection);
  edm::Association<SimClusterCollection>::Filler filler(*assoc);
  filler.insert(stCollection, scIndices.begin(), scIndices.end());
  filler.fill();
  iEvent.put(std::move(assoc));
}

// define this as a plug-in
DEFINE_FWK_MODULE(SimTrackToSimClusterAssociationProducer);


