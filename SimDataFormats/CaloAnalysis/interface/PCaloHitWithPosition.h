#ifndef DataFormats_PCaloHitWithPosition_h
#define DataFormats_PCaloHitWithPosition_h

#include <vector>
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "SimDataFormats/EncodedEventId/interface/EncodedEventId.h"

class PCaloHitWithPosition : public PCaloHit {
    public:
        inline PCaloHitWithPosition() {}
        virtual ~PCaloHitWithPosition() {};

        // Since this class is intended for debugging/plotting purposes,
        // make all attributes public
        GlobalPoint position_;
        int zside_;
        std::float_t siThickness_;
        std::float_t radiusToSide_;
        int siThickIndex_;
        unsigned int layer_;
        unsigned int layerWithOffset_;
        std::pair<int, int> wafer_;
        std::pair<int, int> cell_;
        bool isHalfCell_;
        bool isSilicon_;
        float eta_;
        float phi_;
        float pt_;
        
        bool inEE_;
        bool inHsi_;
        bool inHsc_;

        void setVars(const PCaloHit * hit, hgcal::RecHitTools * hgcalRecHitToolInstance){
            DetId id = hit->id();
            position_ = hgcalRecHitToolInstance->getPosition(id);
            zside_ = hgcalRecHitToolInstance->zside(id);
            siThickness_ = hgcalRecHitToolInstance->getSiThickness(id);
            siThickIndex_ = hgcalRecHitToolInstance->getSiThickIndex(id);
            layer_ = hgcalRecHitToolInstance->getLayer(id);
            layerWithOffset_ = hgcalRecHitToolInstance->getLayerWithOffset(id);
            isHalfCell_ = hgcalRecHitToolInstance->isHalfCell(id);
            isSilicon_ = hgcalRecHitToolInstance->isSilicon(id);
            eta_ = hgcalRecHitToolInstance->getEta(id);
            phi_ = hgcalRecHitToolInstance->getPhi(id);
            pt_ = hgcalRecHitToolInstance->getPt(id, hit->energy());

            inEE_ = false;
            inHsi_ = false;
            inHsc_ = false;
            if (id.det() == DetId::HGCalEE){
                inEE_ = true;
                }
            else if (id.det() == DetId::HGCalHSi){
                inHsi_ = true;
                }
            else if (id.det() == DetId::HGCalHSc){
                inHsc_ = true;
                }

            if (inEE_ || inHsi_){
                wafer_ = hgcalRecHitToolInstance->getWafer(id);
                cell_ = hgcalRecHitToolInstance->getCell(id);
                radiusToSide_ = hgcalRecHitToolInstance->getRadiusToSide(id);
                }

            // Copy over PCaloHit attributes manually
            myEnergy = hit->energy() ;
            myEMFraction = hit->energyEM() / myEnergy ;
            myTime = hit->time() ;
            myItra = hit->geantTrackId() ;
            detId = hit->id() ;
            myDepth = hit->depth() ;
            theEventId = hit->eventId() ;
            myFineItra = hit->geantFineTrackId() ;
            }

    private:
    };

typedef std::vector<PCaloHitWithPosition> PCaloHitWithPositionCollection;

#endif