#include "PhysicsTools/NanoAOD/interface/HitPositionTableProducer.h"
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"

#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"

#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

template <typename T>
class HGCalHitPositionTableProducer : public HitPositionTableProducer<edm::View<T>> {
//class HGCalHitPositionTableProducer : public HitPositionTableProducer<edm::View<CaloRecHit>> {
public:
  HGCalHitPositionTableProducer(edm::ParameterSet const& params)
      : HitPositionTableProducer<edm::View<T>>(params),
        caloGeoToken_(edm::stream::EDProducer<>::esConsumes<edm::Transition::BeginRun>()) { }

  ~HGCalHitPositionTableProducer() override {}

  GlobalPoint positionFromHit(const CaloRecHit& hit) { 
    DetId detId = hit.detid();
    return positionFromDetId(detId); 
  }

  GlobalPoint positionFromHit(const PCaloHit& hit) { 
    DetId detId = hit.id();
    return positionFromDetId(detId); 
  }

  float radiusFromHit(const CaloRecHit& hit) { return radiusFromDetId(hit.detid()); }

  float radiusFromHit(const PCaloHit& hit) { return radiusFromDetId(hit.id()); }

  void beginRun(const edm::Run&, const edm::EventSetup& iSetup) override {
    auto& geom = iSetup.getData(caloGeoToken_);
    rhtools_.setGeometry(geom);
  }

  GlobalPoint positionFromDetId(DetId id) {
    DetId::Detector det = id.det();
    if (det == DetId::HGCalEE || det == DetId::HGCalHSi || det == DetId::HGCalHSc) {
      return rhtools_.getPosition(id);
    } else {
      throw cms::Exception("HGCalHitPositionTableProducer") << "Unsupported DetId type";
    }
  }

  float radiusFromDetId(DetId id) {
    DetId::Detector det = id.det();
    if (det == DetId::HGCalEE || det == DetId::HGCalHSi || det == DetId::HGCalHSc) {
      if (rhtools_.isSilicon(id)) {
        return rhtools_.getRadiusToSide(id);
      } else if (rhtools_.isScintillator(id)) {
          //return r * sin(dphi) / 2.;
       // auto detadphi = rhtools_.getScintDEtaDPhi(id);
       // float dphi = detadphi.second;  //same in both
        float dphi = 0.0211809f; //DEBUG FIXME; the geometry access segfaults for some hits probably close to side
        auto pos = rhtools_.getPosition(id);
        float r = pos.transverse();
        return r * sin(dphi) / 2.;  //this is anyway approximate
      } else {
        return 0.;
      }

    } else {
      return 0;  //no except here
    }
  }

protected:
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeoToken_;
  hgcal::RecHitTools rhtools_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
//typedef HGCalHitPositionTableProducer HGCalRecHitPositionTableProducer;
typedef HGCalHitPositionTableProducer<CaloRecHit> HGCalRecHitPositionTableProducer;
typedef HGCalHitPositionTableProducer<PCaloHit> HGCalSimHitPositionTableProducer;
DEFINE_FWK_MODULE(HGCalRecHitPositionTableProducer);
DEFINE_FWK_MODULE(HGCalSimHitPositionTableProducer);
