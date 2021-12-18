#include <memory>
#include <iostream>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/Common/interface/View.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h" 

#include "DataFormats/HGCalReco/interface/TICLLayerTile.h"

class HGCClusterTrackLinker : public edm::stream::EDProducer<> {
public:
    explicit HGCClusterTrackLinker(const edm::ParameterSet&);
    ~HGCClusterTrackLinker() {};

 private:
    void produce(edm::Event&, const edm::EventSetup&) override;
    
    edm::EDGetTokenT<edm::View<reco::Track>> tracksToken_;
    edm::EDGetTokenT<reco::PFCandidateCollection> pfCandsToken_;

};

HGCClusterTrackLinker::HGCClusterTrackLinker(const edm::ParameterSet& config) :
    tracksToken_(consumes<edm::View<reco::Track>>(config.getParameter<edm::InputTag>("tracks"))),
    pfCandsToken_(consumes<reco::PFCandidateCollection>(config.getParameter<edm::InputTag>("pfCands"))) {
  produces<reco::PFCandidateCollection>();
}

void HGCClusterTrackLinker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<reco::PFCandidateCollection> pfCands;
  iEvent.getByToken(pfCandsToken_, pfCands);

  edm::Handle<edm::View<reco::Track>> tracks;
  iEvent.getByToken(tracksToken_, tracks);

  ticl::TICLLayerTile candTile;
  ticl::TICLLayerTile tracksTile;
    
  auto out = std::make_unique<reco::PFCandidateCollection>();

  for (size_t i = 0; i < tracks->size(); i++) {
    edm::RefToBase<reco::Track> track(tracks, i);
    if (std::abs(track->eta()) > ticl::TileConstants::minEta && std::abs(track->eta()) < ticl::TileConstants::maxEta)
      tracksTile.fill(track->eta(), track->phi(), i);
  }

  for (size_t ip = 0; ip < pfCands->size(); ip++) {
    reco::PFCandidateRef pfCand(pfCands, ip);
    if (pfCand->charge())
        candTile.fill(pfCand->eta(), pfCand->phi(), ip);
  }

  for (size_t bin = 0; bin < static_cast<size_t>(candTile.nBins()); bin++) {
    const std::vector<unsigned int>& candIndices = candTile[bin]; 
    if (!candIndices.size())
        continue;

    std::vector<unsigned int> trackIndices = tracksTile[bin]; 
    for (auto& tidx : trackIndices) {
        edm::RefToBase<reco::Track> track(tracks, tidx);
    }
    for (auto& cidx : candIndices) {
      reco::PFCandidateRef pfCand(pfCands, cidx);
      out->push_back(*pfCand->clone());
    }
    // Sort now so we prioritizes matches for higher pt clusters
    std::sort(out->begin(), out->end(), [](auto& a, auto& b) { return a.pt() > b.pt(); });

    for (auto& cand : *out) {
      if (trackIndices.size()) {
        // Find best match. Current just closest in pt, can do 
        // a smarter mix of closest in pt and eta/phi or something else
        std::vector<float> ptdiffs;
        ptdiffs.reserve(trackIndices.size());
        for (size_t entry : trackIndices) {
          edm::RefToBase<reco::Track> t(tracks, entry);
          ptdiffs.push_back(std::abs(t->pt()-cand.pt()));
        }

        size_t tidx = std::distance(ptdiffs.begin(), std::min_element(ptdiffs.begin(), ptdiffs.end()));
        edm::RefToBase<reco::Track> matchingTrack(tracks, trackIndices.at(tidx));
        trackIndices.erase(trackIndices.begin()+tidx);

        cand.setTrackRef(matchingTrack.castTo<reco::TrackRef>());
      }
    }
  }

  iEvent.put(std::move(out));
}

DEFINE_FWK_MODULE(HGCClusterTrackLinker);
