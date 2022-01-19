#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "RecoHGCal/GraphReco/interface/HGCalTrackPropagator.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/HGCalReco/interface/TICLLayerTile.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include <cmath>
#include <vector>

typedef edm::AssociationMap<edm::OneToManyWithQualityGeneric<
    reco::TrackCollection, SimClusterCollection, float>> RecoTrackToSimCluster;

class RecoTrackToSimClusterAssociation : public edm::stream::EDProducer<> {
public:
  RecoTrackToSimClusterAssociation(edm::ParameterSet const& params)
      : tracksToken_(consumes<reco::TrackCollection>(params.getParameter<edm::InputTag>("tracks"))),
        simClusterToken_(consumes<SimClusterCollection>(params.getParameter<edm::InputTag>("simclusters"))),
        dR_(params.getUntrackedParameter<double>("dR", 0.4)) {
    produces<RecoTrackToSimCluster>();
  }

  ~RecoTrackToSimClusterAssociation() override {}

  void beginRun(const edm::Run&, const edm::EventSetup& iSetup) override {
    trackprop_.getEventSetup(iSetup);
  }

  void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
    edm::Handle<reco::TrackCollection> tracks;
    iEvent.getByToken(tracksToken_, tracks);

    edm::Handle<SimClusterCollection> simClusters;
    iEvent.getByToken(simClusterToken_, simClusters);

    scTile_.clear();

    auto assocMap = std::make_unique<RecoTrackToSimCluster>(tracks, simClusters);

	for (size_t i = 0; i < simClusters->size(); i++) {	
        SimClusterRef sc(simClusters, i);
        scTile_.fill(sc->impactPoint().eta(), sc->impactPoint().phi(), i);
    }

    for (size_t t = 0; t < tracks->size(); t++) {
      reco::TrackRef track(tracks, t);
      auto prop = trackprop_.propagateObject(*track);
      float eta = prop.pos.eta();
	  float phi = prop.pos.phi();
      if (std::abs(eta) < ticl::TileConstants::minEta || std::abs(eta) > ticl::TileConstants::maxEta)
          continue;

		auto searchRange = scTile_.searchBoxEtaPhi(eta-dR_, eta+dR_, phi-dR_, phi+dR_);
		for (int etab = searchRange[0]; etab < searchRange[1]; etab++) {
			for (int phib = searchRange[2]; phib < searchRange[3]; phib++) {
				int bin = scTile_.globalBin(etab, phib);
				const std::vector<unsigned int>& matchIndices = scTile_[bin];
				for (auto scidx : matchIndices) {
                    SimClusterRef sc(simClusters, scidx);
					if (deltaR(prop.pos, sc->impactPoint()) < 0.4) {
                        float ptfrac = sc->impactMomentum().P()/prop.obj->p();
                        // Transform to something with max of 1, so sorting is correct
                        assocMap->insert(track, std::make_pair(sc, ptfrac < 1. ? ptfrac : 2-ptfrac));
					}
				}
			}
		}
	}

  iEvent.put(std::move(assocMap));
  }

protected:
  const edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  const edm::EDGetTokenT<SimClusterCollection> simClusterToken_;
  const float dR_;
  HGCalTrackPropagator trackprop_;

  // Use 0.1 rather than 0.05, which is a bit more consistant with R~0.4
  struct TileConstantsMod {
    static constexpr float minEta = ticl::TileConstants::minEta;
    static constexpr float maxEta = ticl::TileConstants::maxEta;
    static constexpr int nEtaBins = ticl::TileConstants::nEtaBins/2;
    static constexpr int nPhiBins = ticl::TileConstants::nPhiBins/2;
    static constexpr int nBins = nEtaBins * nPhiBins;
    static constexpr int type = 0;
  };
  TICLLayerTileT<TileConstantsMod> scTile_;
};

DEFINE_FWK_MODULE(RecoTrackToSimClusterAssociation);

