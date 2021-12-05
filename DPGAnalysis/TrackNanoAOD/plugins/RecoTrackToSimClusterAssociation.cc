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
#include <vector>

class RecoTrackToSimClusterAssociation : public edm::stream::EDProducer<> {
public:
  RecoTrackToSimClusterAssociation(edm::ParameterSet const& params)
      : tracksToken_(consumes<reco::TrackCollection>(params.getParameter<edm::InputTag>("tracks"))),
        simClusterToken_(consumes<SimClusterCollection>(params.getParameter<edm::InputTag>("simclusters"))) {
    produces<nanoaod::FlatTable>();
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

    ticl::TICLLayerTile scTile;

    std::cout << "Track size is " << tracks->size() << std::endl;
    std::cout << "simClusters  " << simClusters->size() << std::endl;

	for (size_t i = 0; i < simClusters->size(); i++) {	
        SimClusterRef sc(simClusters, i);
        scTile.fill(sc->impactPoint().eta(), sc->impactPoint().phi(), i);
        std::cout << "Filling in bin " << scTile.globalBin(sc->impactPoint().eta(), sc->impactPoint().phi()) << std::endl;
    }

    for (const auto& track : *tracks) {
      auto prop = trackprop_.propagateObject(track);
      float eta = prop.pos.eta();
	  float phi = prop.pos.phi();
      if (std::abs(eta) < ticl::TileConstants::minEta || std::abs(eta) > ticl::TileConstants::maxEta)
          continue;

		auto searchRange = scTile.searchBoxEtaPhi(eta-0.4, eta+0.4, phi-0.4, phi+0.4);
        std::cout << "Looking in search range " << searchRange[0] << " " << searchRange[1] << " " << searchRange[1] << " " << searchRange[3] << std::endl;
		for (int etab = searchRange[0]; etab < searchRange[1]; etab++) {
            std::cout << "Looking at etab " << etab << std::endl;
			for (int phib = searchRange[2]; phib < searchRange[3]; phib++) {
				int bin = scTile.globalBin(etab, phib);
				const std::vector<unsigned int>& matchIndices = scTile[bin];
				for (auto scidx : matchIndices) {
                    std::cout << "-->Found an overlap at SCIDX " << scidx << std::endl;
                    SimClusterRef sc(simClusters, scidx);
					if (deltaR(prop.pos, sc->impactPoint()) < 0.4) {
						std::cout << "These guys match in DR. The pt ratio is " << sc->impactMomentum().pt()/prop.obj->pt() << std::endl;	
					}
				}
			}
		}
	}

  }

protected:
  const edm::EDGetTokenT<reco::TrackCollection> tracksToken_;
  const edm::EDGetTokenT<SimClusterCollection> simClusterToken_;
  HGCalTrackPropagator trackprop_;
};

DEFINE_FWK_MODULE(RecoTrackToSimClusterAssociation);

