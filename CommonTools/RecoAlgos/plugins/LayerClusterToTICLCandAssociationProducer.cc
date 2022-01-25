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
    reco::CaloCluster, TICLCandidate, float> > LayerClusterToTICLCands;
*/

class LayerClusterToTICLCandidateAssociationProducer : public edm::stream::EDProducer<> {
public:
  explicit LayerClusterToTICLCandidateAssociationProducer(const edm::ParameterSet&);
  ~LayerClusterToTICLCandidateAssociationProducer() override;

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<reco::CaloClusterCollection> layerClustersToken_;
  edm::EDGetTokenT<TICLCandidateCollection> ticlCandToken_;
  std::vector<edm::EDGetTokenT<HGCRecHitCollection>> recHitTokens_;
  hgcal::RecHitTools recHitTools_;
};

LayerClusterToTICLCandidateAssociationProducer::LayerClusterToTICLCandidateAssociationProducer(const edm::ParameterSet& pset)
    : layerClustersToken_(consumes<std::vector<reco::CaloCluster>>(pset.getParameter<edm::InputTag>("layerClusters"))),
      ticlCandToken_(consumes<TICLCandidateCollection>(pset.getParameter<edm::InputTag>("ticlTrackstersMerge"))),
      recHitTokens_(edm::vector_transform(pset.getParameter<std::vector<edm::InputTag>>("recHitCollections"),
                [this](const edm::InputTag& tag) {return consumes<HGCRecHitCollection>(tag); })){

    /* fill later
    produces<RecHitToTICLCands>();
  produces<LayerClusterToTICLCands>();
  */ //for now to make it compile
}

LayerClusterToTICLCandidateAssociationProducer::~LayerClusterToTICLCandidateAssociationProducer() {}

// ------------ method called to produce the data  ------------
void LayerClusterToTICLCandidateAssociationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    edm::ESHandle<CaloGeometry> geom;
    iSetup.get<CaloGeometryRecord>().get(geom);
    recHitTools_.setGeometry(*geom);

    edm::Handle<TICLCandidateCollection> ticlCands;
    iEvent.getByToken(ticlCandToken_, ticlCands);

    //convenient access. indices will refer to this later
    std::vector<const HGCRecHit*> allrechits;

    //for convenience
    std::unordered_map<DetId, size_t> detid_to_rh_index;

    //hit energies (copied) that will be depleted during association
    std::vector<float> remaining_rh_energies;//can come handy
    size_t rhindex = 0;
    for (auto& token : recHitTokens_) {
      for (const auto& rh : iEvent.get(token)) {
          detid_to_rh_index[rh.detid()] = rhindex;
          rhindex++;
          allrechits.push_back(&rh);
          remaining_rh_energies.push_back(rh.energy());
      }
  }

  edm::Handle<std::vector<reco::CaloCluster>> layerClusterCollection;
  iEvent.getByToken(layerClustersToken_, layerClusterCollection);

/*
  auto lcAssocMap = std::make_unique<LayerClusterToSimCluster>(layerClusterCollection, ticlCands);
  auto rhAssocMap = std::make_unique<RecHitToSimCluster>(caloRechitCollection, ticlCands);
*/


  //assign each LC to N TICL candidates

  //index w.r.t. allrechits above and the ticl candidate index
  std::map<size_t, std::vector<size_t> > hittotc;

  for (size_t tcidx = 0; tcidx < ticlCands->size(); tcidx++) {
      TICLCandidateRef ticlCand(ticlCands, tcidx);
      for (const auto& trackster : ticlCand->tracksters()) {

          for (int lcidx : trackster->vertices()) {

              //this is the simple part, use equal sharing
              float targetfrac = 1./trackster->vertex_multiplicity(lcidx);
              const auto & lc = layerClusterCollection->at(lcidx);

              float targetEnergy = targetfrac*lc.energy();

              auto hafv = lc.hitsAndFractions();

              //calculate hit distances^2
              std::vector<float> distances;
              for(const auto& hwf: hafv){
                  auto pos = recHitTools_.getPosition(hwf.first);
                  float dist = reco::deltaR2(pos.eta(),pos.phi(),trackster->barycenter().eta(),trackster->barycenter().phi());
                  distances.push_back(dist);
              }

              //re-order by distance^2
              auto distidx = argsort(distances);

              //deplete hit energies starting from closest until target energy reached
              for(const auto& hidx: distidx){
                  //float dist = distances.at(hidx);//doesn'tcidx matter here
                  auto rhdetid = hafv.at(hidx).first;
                  size_t rhidx = detid_to_rh_index[rhdetid];
                  float frachitenergy = allrechits.at(rhidx)->energy()*hafv.at(hidx).second;

                  //if there is still energy left in the hit, even if it's just a bit use it
                  if(remaining_rh_energies[rhidx]>0){
                      //less target energy remaining
                      targetEnergy -= frachitenergy;
                      //part of hit energy has been used
                      remaining_rh_energies[rhidx] -= frachitenergy;
                      //associate
                      hittotc[tcidx].push_back(rhidx);
                  }
                  if(targetEnergy<=0)//done with layer cluster
                      break;
              }
          }
      }
  }



  /*later
  auto assoc = std::make_unique<edm::Association<SimClusterCollection>>(scCollection);
  edm::Association<SimClusterCollection>::Filler filler(*assoc);
  filler.insert(caloRechitCollection, rechitIndices.begin(), rechitIndices.end());
  filler.fill();
  //iEvent.put(std::move(assoc), label + "ToBestSimClus");
  iEvent.put(std::move(assocMap));
  */
}

// define this as a plug-in
DEFINE_FWK_MODULE(LayerClusterToTICLCandidateAssociationProducer);
