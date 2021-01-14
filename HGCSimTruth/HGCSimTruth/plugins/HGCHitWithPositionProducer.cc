#include <memory>
#include <vector>
#include <cstdlib>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/View.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloAnalysis/interface/PCaloHitWithPosition.h"
#include "DataFormats/DetId/interface/DetId.h"

/*
Producer that consumes PCaloHit collections from HGCAL, and produces a 
new PCaloHitWithPosition collection.

The PCaloHitWithPosition has various HGCAL variables (e.g. position) set
as attributes.
*/
class HGCHitWithPositionProducer : public edm::stream::EDProducer<> {
    public:
        explicit HGCHitWithPositionProducer(const edm::ParameterSet&);
        ~HGCHitWithPositionProducer() {}
    private:
        virtual void produce(edm::Event&, const edm::EventSetup&) override;
        void beginRun(const edm::Run&, const edm::EventSetup&) override;
        hgcal::RecHitTools hgcalRecHitToolInstance_ ;
        edm::EDGetTokenT<edm::View<PCaloHit>> hgcalEEHitsToken_;
        edm::EDGetTokenT<edm::View<PCaloHit>> hgcalHEfrontHitsToken_;
        edm::EDGetTokenT<edm::View<PCaloHit>> hgcalHEbackHitsToken_;
    };

HGCHitWithPositionProducer::HGCHitWithPositionProducer(const edm::ParameterSet& iConfig) :
    hgcalEEHitsToken_(consumes<edm::View<PCaloHit>>(edm::InputTag("g4SimHits", "HGCHitsEE"))),
    hgcalHEfrontHitsToken_(consumes<edm::View<PCaloHit>>(edm::InputTag("g4SimHits", "HGCHitsHEfront"))),
    hgcalHEbackHitsToken_(consumes<edm::View<PCaloHit>>(edm::InputTag("g4SimHits", "HGCHitsHEback")))
    {
    produces<std::vector<PCaloHitWithPosition>>();
    }

void HGCHitWithPositionProducer::beginRun(const edm::Run&, const edm::EventSetup& iSetup) {
    edm::ESHandle<CaloGeometry> geom;
    iSetup.get<CaloGeometryRecord>().get(geom);
    hgcalRecHitToolInstance_.setGeometry(*geom);
    }

void HGCHitWithPositionProducer::produce(edm::Event& iEvent, const edm::EventSetup&) {  
    std::unique_ptr< std::vector< PCaloHitWithPosition >> hitsWithPositions( new std::vector<PCaloHitWithPosition> );
    std::vector<edm::EDGetTokenT<edm::View<PCaloHit>>> tokens = {
        hgcalEEHitsToken_,
        hgcalHEfrontHitsToken_,
        hgcalHEbackHitsToken_
        };
    for (edm::EDGetTokenT<edm::View<PCaloHit>> token : tokens ) {
        edm::Handle< edm::View<PCaloHit> > handle;
        iEvent.getByToken(token, handle);
        for (auto const & hit : handle->ptrs() ) {
            if (hit->geantTrackId() == 0){
                edm::LogError("CaloSim")
                    << "Hit " << hit->id()
                    << " has track id " << hit->geantTrackId()
                    << std::endl
                    ;
                }
            DetId id = hit->id();
            if (id.det() == DetId::HGCalEE || id.det() == DetId::HGCalHSi || id.det() == DetId::HGCalHSc){
                PCaloHitWithPosition hitWithPosition = PCaloHitWithPosition();
                hitWithPosition.setVars(&(*hit), &hgcalRecHitToolInstance_);
                hitsWithPositions->push_back(std::move(hitWithPosition));
                }
            }
        }
    iEvent.put(std::move(hitsWithPositions));
    }

DEFINE_FWK_MODULE(HGCHitWithPositionProducer);