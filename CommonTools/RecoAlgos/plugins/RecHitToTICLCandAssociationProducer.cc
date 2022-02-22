//
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
#include "DataFormats/HGCalReco/interface/TICLCandidate.h"

#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"

#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/Common/interface/OneToManyWithQualityGeneric.h"
#include "DataFormats/Common/interface/RefToBase.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "FWCore/Utilities/interface/transform.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>

//helper
#include <numeric>
#include <algorithm>

template <typename T>
std::vector<size_t> argsort(const std::vector<T> &v) {
    using namespace std;
    vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);
    stable_sort(idx.begin(), idx.end(),
            [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
    return idx;
}


//
// class declaration
//
typedef std::vector<TICLCandidate> TICLCandidateCollection;
typedef edm::Ref<TICLCandidateCollection> TICLCandidateRef;

typedef edm::Ref<reco::CaloClusterCollection> CaloClusterRef;

/* doesn't compile
typedef edm::AssociationMap<edm::OneToManyWithQualityGeneric<
    HGCRecHitCollection, TICLCandidate, float> > RecHitToTICLCands;
typedef edm::AssociationMap<edm::OneToManyWithQualityGeneric<
    reco::CaloCluster, TICLCandidate, float> > RecHitToTICLCands;
*/

class RecHitToTICLCandidateAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit RecHitToTICLCandidateAssociationProducer(const edm::ParameterSet&);
  ~RecHitToTICLCandidateAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<reco::CaloClusterCollection> layerClustersToken_;
  edm::EDGetTokenT<TICLCandidateCollection> ticlCandToken_;
  edm::EDGetTokenT<HGCRecHitCollection> recHitToken_;
  hgcal::RecHitTools recHitTools_;
};

RecHitToTICLCandidateAssociationProducer::RecHitToTICLCandidateAssociationProducer(const edm::ParameterSet& pset)
    : layerClustersToken_(consumes<std::vector<reco::CaloCluster>>(pset.getParameter<edm::InputTag>("layerClusters"))),
      ticlCandToken_(consumes<TICLCandidateCollection>(pset.getParameter<edm::InputTag>("ticlCandidates"))),
      recHitToken_(consumes<HGCRecHitCollection>(pset.getParameter<edm::InputTag>("caloRecHits"))){

    produces<edm::Association<TICLCandidateCollection>>();
}

RecHitToTICLCandidateAssociationProducer::~RecHitToTICLCandidateAssociationProducer() {}

// ------------ method called to produce the data  ------------
void RecHitToTICLCandidateAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::ESHandle<CaloGeometry> geom;
  iSetup.get<CaloGeometryRecord>().get(geom);
  recHitTools_.setGeometry(*geom);

  edm::Handle<TICLCandidateCollection> ticlCands;
  iEvent.getByToken(ticlCandToken_, ticlCands);

  edm::Handle<HGCRecHitCollection> recHits;
  iEvent.getByToken(recHitToken_, recHits);

  //convenient access. indices will refer to this later
  std::vector<const HGCRecHit*> allrechits;

  //for convenience
  std::unordered_map<DetId, size_t> detid_to_rh_index;

  //hit energies (copied) that will be depleted during association
  size_t rhindex = 0;
  for (const auto& rh : *recHits) {
    detid_to_rh_index[rh.detid()] = rhindex;
    rhindex++;
    allrechits.push_back(&rh);
  }

  edm::Handle<std::vector<reco::CaloCluster>> layerClusterCollection;
  iEvent.getByToken(layerClustersToken_, layerClusterCollection);

  //assigns a ticl candidate to each rechit
  std::vector<int> hittotc(allrechits.size(), -1);

  for (size_t tcidx = 0; tcidx < ticlCands->size(); tcidx++) {
    TICLCandidateRef ticlCand(ticlCands, tcidx);
    for (const auto& trackster : ticlCand->tracksters()) {
      for (int lcidx : trackster->vertices()) {
        //this is the simple part, use equal sharing
        float targetfrac = 1. / trackster->vertex_multiplicity(lcidx);
        const auto& lc = layerClusterCollection->at(lcidx);

        float targetEnergy = targetfrac * lc.energy();

        auto hafv = lc.hitsAndFractions();

        //calculate hit distances^2
        std::vector<float> distances;
        for (const auto& hwf : hafv) {
          auto pos = recHitTools_.getPosition(hwf.first);
          float dist =
              reco::deltaR2(pos.eta(), pos.phi(), trackster->barycenter().eta(), trackster->barycenter().phi());
          distances.push_back(dist);
        }

        //re-order by distance^2
        auto distidx = argsort(distances);

        //deplete hit energies starting from closest until target energy reached
        for (const auto& hidx : distidx) {
          //float dist = distances.at(hidx);//doesn'tcidx matter here
          auto rhdetid = hafv.at(hidx).first;
          size_t rhidx = detid_to_rh_index[rhdetid];
          float frachitenergy = allrechits.at(rhidx)->energy() * hafv.at(hidx).second;

          //if there is still energy left in the hit, even if it's just a bit use it
          if (hittotc.at(rhidx)<0) {//not assigned yet
            //less target energy remaining
            targetEnergy -= frachitenergy;
            //associate
            hittotc.at(rhidx) = tcidx;
          }
          if (targetEnergy <= 0)  //done with layer cluster
            break;
        }
      }
    }
  }

  std::cout << "RecHitToTICLCandidateAssociationProducer done" << std::endl;

  auto assoc = std::make_unique<edm::Association<TICLCandidateCollection>>(ticlCands);
  edm::Association<TICLCandidateCollection>::Filler filler(*assoc);
  filler.insert(recHits, hittotc.begin(), hittotc.end());
  filler.fill();
  iEvent.put(std::move(assoc));

}

// define this as a plug-in
DEFINE_FWK_MODULE(RecHitToTICLCandidateAssociationProducer);
