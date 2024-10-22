// system include files
#include <memory>
#include <string>

// user include files
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"

#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/Common/interface/OneToManyWithQualityGeneric.h"

#include "FWCore/Utilities/interface/transform.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>

//
// class decleration
//
typedef std::pair<size_t, float> IdxAndFraction;
typedef edm::AssociationMap<edm::OneToManyWithQualityGeneric<
    HGCRecHitCollection, std::vector<reco::CaloCluster>, float>> RecHitToLayerCluster;

class RecHitToLayerClusterAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit RecHitToLayerClusterAssociationProducer(const edm::ParameterSet&);
  ~RecHitToLayerClusterAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  std::vector<edm::InputTag> caloRechitTags_;
  std::vector<edm::EDGetTokenT<HGCRecHitCollection>> caloRechitCollectionTokens_;
  edm::EDGetTokenT<std::vector<reco::CaloCluster>> layerClusterToken_;
};

RecHitToLayerClusterAssociationProducer::RecHitToLayerClusterAssociationProducer(const edm::ParameterSet& pset)
    : caloRechitTags_(pset.getParameter<std::vector<edm::InputTag>>("caloRecHits")),
      caloRechitCollectionTokens_(edm::vector_transform(
          caloRechitTags_, [this](const edm::InputTag& tag) { return consumes<HGCRecHitCollection>(tag); })),
      layerClusterToken_(consumes<std::vector<reco::CaloCluster>>(pset.getParameter<edm::InputTag>("layerClusters"))) {
  for (auto& tag : caloRechitTags_) {
    const std::string& label = !tag.instance().empty() ? tag.instance() : tag.label();
    produces<edm::Association<std::vector<reco::CaloCluster>>>(label + "ToBestLayerCluster");
    produces<RecHitToLayerCluster>(label + "ToLayerCluster");
  }
}

RecHitToLayerClusterAssociationProducer::~RecHitToLayerClusterAssociationProducer() {}

//
// member functions
//

// ------------ method called to produce the data  ------------
void RecHitToLayerClusterAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<std::vector<reco::CaloCluster>> lcCollection;
  iEvent.getByToken(layerClusterToken_, lcCollection);
  std::unordered_map<size_t, std::vector<IdxAndFraction>> hitDetIdToIndices;

  for (size_t j = 0; j < lcCollection->size(); j++) {
      for (auto& idAndFrac : (*lcCollection)[j].hitsAndFractions()) {
          hitDetIdToIndices[idAndFrac.first.rawId()].push_back({j, idAndFrac.second});
      }
  }

  for (size_t i = 0; i < caloRechitCollectionTokens_.size(); i++) {
    std::string label = caloRechitTags_.at(i).instance();
	if (label.empty())
		label = caloRechitTags_.at(i).label();
    std::vector<int> rechitIndices;

    edm::Handle<HGCRecHitCollection> caloRechitCollection;
    iEvent.getByToken(caloRechitCollectionTokens_.at(i), caloRechitCollection);

    auto assocMap = std::make_unique<RecHitToLayerCluster>(caloRechitCollection, lcCollection);

    for (size_t h = 0; h < caloRechitCollection->size(); h++) {
      HGCRecHitRef caloRh(caloRechitCollection, h);
      size_t id = caloRh->detid().rawId();

      // Need to sort before inserting into AssociationMap
      auto match = hitDetIdToIndices.find(id);
      if (match == std::end(hitDetIdToIndices)) {
        rechitIndices.push_back(-1);
        continue;
      }
      auto& lcIdxAndFrac = match->second;
      // Sort by energy fraction
      std::sort(std::begin(lcIdxAndFrac), std::end(lcIdxAndFrac), 
            [](auto& a, auto& b) { return a.second > b.second; });

      for (size_t m = 0; m < lcIdxAndFrac.size(); m++) {
        float fraction = lcIdxAndFrac[m].second;
        int lcIdx = lcIdxAndFrac[m].first;
        // Best match is the layerCluster that carries the hit with the highest energy fraction
        // (that is, the one responsible for the largest deposit in the detId)
        if (m == 0)
          rechitIndices.push_back(lcIdx);
        edm::Ref<std::vector<reco::CaloCluster>> lc(lcCollection, lcIdx);
        assocMap->insert(caloRh, std::make_pair(lc, fraction));
      }
    }

    auto assoc = std::make_unique<edm::Association<std::vector<reco::CaloCluster>>>(lcCollection);
    edm::Association<std::vector<reco::CaloCluster>>::Filler filler(*assoc);
    filler.insert(caloRechitCollection, rechitIndices.begin(), rechitIndices.end());
    filler.fill();
    iEvent.put(std::move(assoc), label + "ToBestLayerCluster");
    iEvent.put(std::move(assocMap), label + "ToLayerCluster");
  }
}

// define this as a plug-in
DEFINE_FWK_MODULE(RecHitToLayerClusterAssociationProducer);

