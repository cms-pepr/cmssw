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
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/Common/interface/OneToManyWithQualityGeneric.h"
#include "DataFormats/Common/interface/RefToBase.h"


#include "FWCore/Utilities/interface/transform.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>

//
// class decleration
//
typedef std::pair<size_t, float> IdxAndFraction;
typedef edm::AssociationMap<edm::OneToManyWithQualityGeneric<
    HGCRecHitCollection, SimClusterCollection, float>> RecHitToSimCluster;

class SimClusterRecHitAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit SimClusterRecHitAssociationProducer(const edm::ParameterSet&);
  ~SimClusterRecHitAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  std::vector<edm::InputTag> caloRechitTags_;
  std::vector<edm::EDGetTokenT<HGCRecHitCollection>> caloRechitCollectionTokens_;
  edm::EDGetTokenT<SimClusterCollection> scCollectionToken_;
};

SimClusterRecHitAssociationProducer::SimClusterRecHitAssociationProducer(const edm::ParameterSet& pset)
    : caloRechitTags_(pset.getParameter<std::vector<edm::InputTag>>("caloRecHits")),
      caloRechitCollectionTokens_(edm::vector_transform(
          caloRechitTags_, [this](const edm::InputTag& tag) { return consumes<HGCRecHitCollection>(tag); })),
      scCollectionToken_(consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("simClusters"))) {
  for (auto& tag : caloRechitTags_) {
    const std::string& label = !tag.instance().empty() ? tag.instance() : tag.label();
    produces<edm::Association<SimClusterCollection>>(label + "ToBestSimClus");
    produces<RecHitToSimCluster>(label + "ToSimClus");
  }
  produces<std::unordered_map<int, float>>();
}

SimClusterRecHitAssociationProducer::~SimClusterRecHitAssociationProducer() {}

// ------------ method called to produce the data  ------------
void SimClusterRecHitAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  auto simClusterToRecEnergy = std::make_unique<std::unordered_map<int, float>>();
  edm::Handle<SimClusterCollection> scCollection;
  iEvent.getByToken(scCollectionToken_, scCollection);
  std::unordered_map<size_t, std::vector<IdxAndFraction>> hitDetIdToIndices;

  for (size_t s = 0; s < scCollection->size(); s++) {
    const auto& sc = scCollection->at(s);
    // Need to initialize because not all SimClusters lead to rechits
    (*simClusterToRecEnergy)[s] = 0.;
    for (auto& hf : sc.hits_and_fractions()) {
      // Can have two unique hits with the same detId
      hitDetIdToIndices[hf.first].push_back({s, hf.second});
    }
  }

  std::unordered_map<int, float> hitDetIdToTotalRecEnergy;

  for (size_t i = 0; i < caloRechitCollectionTokens_.size(); i++) {
    std::string label = caloRechitTags_.at(i).instance();
	if (label.empty())
		label = caloRechitTags_.at(i).label();
    std::vector<size_t> rechitIndices;

    edm::Handle<HGCRecHitCollection> caloRechitCollection;
    iEvent.getByToken(caloRechitCollectionTokens_.at(i), caloRechitCollection);

    auto assocMap = std::make_unique<RecHitToSimCluster>(caloRechitCollection, scCollection);

    for (size_t h = 0; h < caloRechitCollection->size(); h++) {
      HGCRecHitRef caloRh(caloRechitCollection, h);
      size_t id = caloRh->detid().rawId();
      float energy = caloRh->energy();
      hitDetIdToTotalRecEnergy[id] += energy;

      // Need to sort before inserting into AssociationMap
      auto match = hitDetIdToIndices.find(id);
      if (match == std::end(hitDetIdToIndices)) {
        rechitIndices.push_back(-1);
        continue;
      }
      auto& scIdxAndFrac = match->second;
      // Sort by energy fraction
      std::sort(std::begin(scIdxAndFrac), std::end(scIdxAndFrac), 
            [](auto& a, auto& b) { return a.second > b.second; });

      for (size_t m = 0; m < scIdxAndFrac.size(); m++) {
        float fraction = scIdxAndFrac[m].second;
        int scIdx = scIdxAndFrac[m].first;
        (*simClusterToRecEnergy)[scIdx] += energy * fraction;
        // Best match is the simCluster that carries the hit with the highest energy fraction
        // (that is, responsible for the largest deposit in the detId)
        if (m == 0)
          rechitIndices.push_back(scIdx);
        SimClusterRef simclus(scCollection, scIdx);
        assocMap->insert(caloRh, std::make_pair(simclus, fraction));
      }
    }

    auto assoc = std::make_unique<edm::Association<SimClusterCollection>>(scCollection);
    edm::Association<SimClusterCollection>::Filler filler(*assoc);
    filler.insert(caloRechitCollection, rechitIndices.begin(), rechitIndices.end());
    filler.fill();
    iEvent.put(std::move(assoc), label + "ToBestSimClus");
    iEvent.put(std::move(assocMap), label + "ToSimClus");
  }
  iEvent.put(std::move(simClusterToRecEnergy));
}

// define this as a plug-in
DEFINE_FWK_MODULE(SimClusterRecHitAssociationProducer);
