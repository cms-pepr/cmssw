#include "PhysicsTools/NanoAOD/interface/HitPositionTableProducer.h"
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"

#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"

#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/Records/interface/HcalRecNumberingRecord.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"


class HCALHitPositionTableProducer : public HitPositionTableProducer<edm::View<CaloRecHit>> {
public:
  HCALHitPositionTableProducer(edm::ParameterSet const& params)
      : HitPositionTableProducer(params) {}
        //caloGeomToken_(esConsumes<CaloGeometry, CaloGeometryRecord>()) {

  ~HCALHitPositionTableProducer() override {}

  GlobalPoint positionFromHit(const CaloRecHit& hit) { return positionFromDetId(hit.detid()); }

  void beginRun(const edm::Run&, const edm::EventSetup& iSetup) override {
    // TODO: check that the geometry exists
    iSetup.get<CaloGeometryRecord>().get(caloGeom_);
  }

  GlobalPoint positionFromDetId(DetId detid) {
    DetId::Detector det = detid.det();
    if (det == DetId::Hcal) {
      GlobalPoint position;
      HcalSubdetector esd = (HcalSubdetector)detid.subdetId();
      const CaloSubdetectorGeometry* gTmp = caloGeom_->getSubdetectorGeometry(DetId::Hcal, esd);
      auto thisCell = gTmp->getGeometry(detid);
      return thisCell->getPosition();
    } else {
      throw cms::Exception("HCALHitPositionTableProducer") << "Unsupported DetId type";
    }
  }

protected:
  //edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeomToken_;
  edm::ESHandle<CaloGeometry> caloGeom_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
typedef HCALHitPositionTableProducer HCALRecHitPositionTableProducer;
DEFINE_FWK_MODULE(HCALRecHitPositionTableProducer);

