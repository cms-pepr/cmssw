#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Common/interface/View.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"

#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include <vector>
#include <iostream>

template <typename T>
class HitPositionTableProducer : public edm::stream::EDProducer<> {
public:
  HitPositionTableProducer(edm::ParameterSet const& params)
      : name_(params.getParameter<std::string>("name")),
        doc_(params.getParameter<std::string>("doc")),
        src_(consumes<T>(params.getParameter<edm::InputTag>("src"))),
        cut_(params.getParameter<std::string>("cut"), true) {
    produces<nanoaod::FlatTable>();
  }

  ~HitPositionTableProducer() override {}

  void beginRun(const edm::Run&, const edm::EventSetup& iSetup) override {
    // TODO: check that the geometry exists
    iSetup.get<CaloGeometryRecord>().get(caloGeom_);
    rhtools_.setGeometry(*caloGeom_);
    iSetup.get<TrackerDigiGeometryRecord>().get("idealForDigi", trackGeom_);
    // Believe this is ideal, but we're not so precise here...
    iSetup.get<GlobalTrackingGeometryRecord>().get(globalGeom_);
  }

  GlobalPoint positionFromHit(const PCaloHit& hit) {
    DetId id = hit.id();
    return positionFromDetId(id);
  }

  GlobalPoint positionFromHit(const CaloRecHit& hit) { return positionFromDetId(hit.detid()); }

  // Should really only be used for HGCAL
  GlobalPoint positionFromDetId(DetId id) {
    DetId::Detector det = id.det();
    if (det == DetId::Hcal || det == DetId::HGCalEE || det == DetId::HGCalHSi || det == DetId::HGCalHSc) {
      return rhtools_.getPosition(id);
    } else {
      throw cms::Exception("HitPositionTableProducer") << "Unsupported DetId type";
    }
  }

  GlobalPoint positionFromHit(const PSimHit& hit) {
    auto detId = DetId(hit.detUnitId());
    auto surface = detId.det() == DetId::Muon ? globalGeom_->idToDet(hit.detUnitId())->surface()
                                              : trackGeom_->idToDet(hit.detUnitId())->surface();
    GlobalPoint position = surface.toGlobal(hit.localPosition());
    return position;
  }

  float radiusFromHit(const PCaloHit& hit) {
      DetId id = hit.id();
      return radiusFromDetId(id);
  }

  float radiusFromHit(const CaloRecHit& hit) {
      return radiusFromDetId(hit.detid());
  }

  // Should really only be used for HGCAL, cartesian
  float radiusFromDetId(DetId id) {
      DetId::Detector det = id.det();
      if (det == DetId::Hcal || det == DetId::HGCalEE || det == DetId::HGCalHSi || det == DetId::HGCalHSc) {
         if(rhtools_.isSilicon(id)){
             return rhtools_.getRadiusToSide(id);
         }
         else if(rhtools_.isScintillator(id)){
             auto detadphi = rhtools_.getScintDEtaDPhi(id);
             float dphi = detadphi.second;//same in both
             auto pos = rhtools_.getPosition(id);
             float r = pos.transverse();
             return r * sin(dphi)/2.;//this is anyway approximate
         }
         else{
             return 0.;
         }

      } else {
        return 0; //no except here
      }
  }

  float radiusFromHit(const PSimHit& hit) {
      auto detId = DetId(hit.detUnitId());
        return radiusFromDetId(detId);
    }

  void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
    edm::Handle<T> objs;
    iEvent.getByToken(src_, objs);

    std::vector<float> xvals;
    std::vector<float> yvals;
    std::vector<float> zvals;
    std::vector<float> hitrvals;
    for (const auto& obj : *objs) {
      if (cut_(obj)) {
        auto position = positionFromHit(obj);
        xvals.emplace_back(position.x());
        yvals.emplace_back(position.y());
        zvals.emplace_back(position.z());
        hitrvals.emplace_back(radiusFromHit(obj));
      }
    }

    auto tab = std::make_unique<nanoaod::FlatTable>(xvals.size(), name_, false, true);
    tab->addColumn<float>("x", xvals, "x position");
    tab->addColumn<float>("y", yvals, "y position");
    tab->addColumn<float>("z", zvals, "z position");
    tab->addColumn<float>("hitr", hitrvals, "radius");

    iEvent.put(std::move(tab));
  }

protected:
  const std::string name_, doc_;
  const edm::EDGetTokenT<T> src_;
  const StringCutObjectSelector<typename T::value_type> cut_;
  edm::ESHandle<CaloGeometry> caloGeom_;
  edm::ESHandle<TrackerGeometry> trackGeom_;
  edm::ESHandle<GlobalTrackingGeometry> globalGeom_;
  hgcal::RecHitTools rhtools_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
typedef HitPositionTableProducer<std::vector<PCaloHit>> PCaloHitPositionTableProducer;
typedef HitPositionTableProducer<std::vector<PSimHit>> PSimHitPositionTableProducer;
typedef HitPositionTableProducer<HGCRecHitCollection> HGCRecHitPositionTableProducer;
DEFINE_FWK_MODULE(HGCRecHitPositionTableProducer);
DEFINE_FWK_MODULE(PCaloHitPositionTableProducer);
DEFINE_FWK_MODULE(PSimHitPositionTableProducer);
