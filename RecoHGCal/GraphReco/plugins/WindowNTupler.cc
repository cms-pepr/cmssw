// -*- C++ -*-
//
// Package:    RecoHGCal/WindowNTupler
// Class:      WindowNTupler
//
/**\class WindowNTupler WindowNTupler.cc RecoHGCal/GraphReco/plugins/WindowNTupler.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Jan Kieseler
//         Created:  Tue, 27 Aug 2019 17:25:47 GMT
//
//



// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "../interface/NTupleWindow.h"
//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.


using reco::TrackCollection;

class WindowNTupler : public edm::one::EDAnalyzer<edm::one::WatchRuns, edm::one::SharedResources>  {
   public:
      explicit WindowNTupler(const edm::ParameterSet&);
      ~WindowNTupler();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

      virtual void beginRun(edm::Run const &iEvent, edm::EventSetup const &) override;
      virtual void endRun(edm::Run const &iEvent, edm::EventSetup const &) override;

   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      edm::EDGetTokenT<TrackCollection> tracksToken_;
      edm::EDGetTokenT<reco::CaloClusterCollection> layerClustersToken_;
      edm::EDGetTokenT<std::vector<SimCluster>> simClusterToken_;
      std::vector<edm::EDGetTokenT<HGCRecHitCollection> > rechitsTokens_;

      std::vector<NTupleWindow> windows_;
      hgcal::RecHitTools recHitTools_;
      HGCalTrackPropagator trackprop_;
      edm::Service<TFileService> fs_;
      TTree * outTree_;


};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
WindowNTupler::WindowNTupler(const edm::ParameterSet& config)
 :
  tracksToken_(consumes<TrackCollection>(config.getParameter<edm::InputTag>("tracks"))),
  layerClustersToken_(consumes<reco::CaloClusterCollection>(config.getParameter<edm::InputTag>("layerClusters"))),
  simClusterToken_(consumes<std::vector<SimCluster>>(config.getParameter<edm::InputTag>("simClusters"))),
  outTree_(nullptr)

/* ... */

{
    for (edm::InputTag& recHitCollection : config.getParameter<
            std::vector<edm::InputTag> >("recHitCollections")) {
        rechitsTokens_.push_back(
                consumes<HGCRecHitCollection>(recHitCollection));
    }

    //DEBUG INFO: has checks built in
    windows_ = NTupleWindow::createWindows(
            (size_t) config.getParameter<uint32_t>("nPhiSegments"),
            (size_t) config.getParameter<uint32_t>("nEtaSegments"),
            config.getParameter<double>("minEta"),
            config.getParameter<double>("maxEta"),
            config.getParameter<double>("etaFrameWidth"),
            config.getParameter<double>("phiFrameWidth"));

    for(auto& w: windows_)
        w.setMode(WindowBase::useRechits);//FIXME make configurable
        //w.setMode(WindowBase::useLayerClusters);//FIXME make configurable

}


WindowNTupler::~WindowNTupler()
{
}


//
// member functions
//

// ------------ method called for each event  ------------
void
WindowNTupler::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   //prepare collections

   //get and propagate tracks
   std::vector<TrackWithHGCalPos> proptracks;
   auto intracks = iEvent.get(tracksToken_);
   for(const auto& t: intracks){
       if(fabs(t.eta())< 1.5 || fabs(t.eta())>3.0) continue; //just use potentially interesting ones
       proptracks.push_back(trackprop_.propagateObject(t));
   }

   //get rechits, get positions and merge collections
   std::vector<HGCRecHitWithPos> allrechits;
   for (auto & token : rechitsTokens_) {
       for (const auto& rh : iEvent.get(token)) {
           HGCRecHitWithPos rhp = { const_cast<HGCRecHit*>(&rh), recHitTools_.getPosition(rh.detid()) };
           allrechits.push_back(rhp);
       }
   }
   std::sort(allrechits.begin(), allrechits.end(), 
        [](const HGCRecHitWithPos& rh1, const HGCRecHitWithPos& rh2) 
            { return rh1.hit->energy() > rh2.hit->energy();});

   auto inlayerclusters = iEvent.get(layerClustersToken_);
   auto insimclusters = iEvent.get(simClusterToken_);


   //DEBUG
   DEBUGPRINT(intracks.size());
   DEBUGPRINT(proptracks.size());
   DEBUGPRINT(allrechits.size());
   DEBUGPRINT(inlayerclusters.size());
   DEBUGPRINT(insimclusters.size());

   std::vector<size_t> filledtrack(proptracks.size(),0);
   std::vector<size_t> filledrechits(allrechits.size(),0);
   std::vector<size_t> filledlayercluster(inlayerclusters.size(),0);
   std::vector<size_t> filledsimclusters(insimclusters.size(),0);

   //rechits
   //filledrechits vector etc

   for(auto& window : windows_){
       //attach tracks, rehits etc to windows
       for(size_t it=0;it<proptracks.size();it++) {
           if(filledtrack.at(it)>3) continue;
           if(window.maybeAddTrack(proptracks.at(it)))
               filledtrack.at(it)++;
       }
       for(size_t it=0;it<allrechits.size();it++) {
           if(filledrechits.at(it)>3) continue;
           if(window.maybeAddRecHit(allrechits.at(it)))
               filledrechits.at(it)++;
       }
       if(window.getMode() == WindowBase::useLayerClusters){
           for(size_t it=0;it<inlayerclusters.size();it++) {
               if(filledlayercluster.at(it)>3) continue;
               if(window.maybeAddLayerCluster(inlayerclusters.at(it)))
                   filledlayercluster.at(it)++;
           }
       }

       for(size_t it=0;it<insimclusters.size();it++) {
           if(filledsimclusters.at(it)>3) continue;
           if(window.maybeAddSimCluster(insimclusters.at(it)))
               filledsimclusters.at(it)++;
       }

       //the follwing will not work yet before everything is filled
       window.fillFeatureArrays();
       window.flattenRechitFeatures();
       window.fillTruthArrays();
       window.assignTreePointers();

       outTree_->Fill();
       window.clear(); //free memory
   }



}


// ------------ method called once each job just before starting event loop  ------------
void WindowNTupler::beginJob() {

    if (!fs_)
        throw edm::Exception(edm::errors::Configuration,
                "TFile Service is not registered in cfg file");

    outTree_ = fs_->make<TTree>("tree", "tree");
    NTupleWindow::createTreeBranches(outTree_);

}

// ------------ method called once each job just after ending the event loop  ------------
void
WindowNTupler::endJob()
{
}

void WindowNTupler::beginRun(edm::Run const &iEvent, edm::EventSetup const &es) {
  recHitTools_.getEventSetup(es);
  trackprop_ .getEventSetup(es);
}
void WindowNTupler::endRun(edm::Run const &iEvent, edm::EventSetup const &es) {

}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
WindowNTupler::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(WindowNTupler);
