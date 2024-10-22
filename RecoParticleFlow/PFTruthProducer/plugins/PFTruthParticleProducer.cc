// system include files
#include <memory>
#include <string>

// user include files
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "SimDataFormats/Associations/interface/TrackToTrackingParticleAssociator.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticle.h"
#include "SimDataFormats/PFAnalysis/interface/PFTruthParticleFwd.h"
#include "DataFormats/CaloRecHit/interface/CaloRecHit.h"

#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "FWCore/Utilities/interface/EDGetToken.h"
#include <set>
#include "DataFormats/Math/interface/deltaR.h"

//this is now just for HGCAL
#include "RecoHGCal/GraphReco/interface/HGCalTrackPropagator.h"
#include "../interface/HGCalSimClusterMerger.h"
#include "../interface/SimHistoryTool.h"

//
// class declaration
//
/*
 *
 *  produces in two steps PFTruthParticles.
 *  This producer considers truth particles one-by-one and does not
 *  account for possible overlaps. These will need to be resolved in a second producer
 *  (PFTruthParticleMerger).
 *  The advantage is that the plugin here can run on every single event, independent of
 *  pileup mixing and event density, while the second producer can be run after mixing in pileup
 *
 *  Step A)
 *     Produces an ideal PFTruthParticle, without considering detector limitations.
 *     This is done by connecting every trackingParticle and SimCluster that originated
 *     from a particle passed to Geant back to it.
 *     No configuration options needed.
 *     The tracking particles should be selected by one of the standard tracking selectors, e.g.:
 *     https://github.com/cms-sw/cmssw/blob/master/Validation/RecoTrack/python/TrackingParticleSelectionForEfficiency_cfi.py
 *
 *  Step B)
 *     The ideal PFTruthParticle gets split according to detector limitations, e.g. if
 *     a neutral pion splits to 2 photons and leaves to distinct photon showers,
 *     this step would produce 2 photons.
 *
 *     For neutral and charged particles, stray clusters with deltaR larger than <softkillcone>
 *     and tracks that contribute with less than a relative <softkillrelenergy> to
 *     the total particle energy are removed.
 *
 *     This splitting of neutral particles depends on the granularity of the SimClusters
 *     in the respective subdetector.
 *     <add other possible options as configuration parameters>
 *
 *     In case of charged particles, there are multiple options, e.g. when to split off bremsstrahlung
 *     for an electron.
 *
 *
 *
 *
 */

typedef edm::AssociationMap<edm::OneToMany<
    TrackingParticleCollection, SimClusterCollection>> TrackingParticleToSimCluster;

class PFTruthParticleProducer : public edm::stream::EDProducer<> {
public:
  explicit PFTruthParticleProducer(const edm::ParameterSet &);
  ~PFTruthParticleProducer() override;

  void beginRun(const edm::Run&, const edm::EventSetup& iSetup) override {
    trackprop_.setupRun(iSetup);
    auto& geom = iSetup.getData(caloGeoToken_);
    hgcrechittools_.setGeometry(geom);//OPT
  }

  void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override;

private:

  std::vector<size_t> matchedSCtoTrackIdxs(const SimClusterRefVector &simClusters,
          const TrackingParticle& tp, const reco::Track& track) const;

  std::vector<SimClusterRefVector> splitCPToTP(const SimClusterRefVector& allCPSimClusters,
          TrackingParticleRefVector assoTPs, const SimHistoryTool& histtool) const;

  edm::EDGetTokenT<TrackingParticleCollection> tpCollectionToken_;
  edm::EDGetTokenT<CaloParticleCollection> cpCollectionToken_;
  edm::EDGetTokenT<SimClusterCollection> scCollectionToken_;
  edm::EDGetTokenT<std::vector<SimVertex> > svCollectionToken_;
  edm::EDGetTokenT<std::vector<SimTrack> > stCollectionToken_;
  edm::EDGetTokenT<HGCRecHitCollection> caloRecHitToken_;
  edm::EDGetTokenT<edm::View<reco::Track>> tracksToken_;
  edm::EDGetTokenT<edm::Association<SimClusterCollection>> recHitToSCToken_;
  edm::EDGetTokenT<reco::SimToRecoCollection> tpToTrackToken_;

  //fix with proper beginRun
  HGCalTrackPropagator trackprop_;
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeoToken_;
  hgcal::RecHitTools hgcrechittools_;

  float pfMatchThreshold_,pfMatchDRThreshold_;

};

PFTruthParticleProducer::PFTruthParticleProducer(const edm::ParameterSet &pset)
    : tpCollectionToken_(consumes<TrackingParticleCollection>(pset.getParameter<edm::InputTag>("trackingParticles"))),
      cpCollectionToken_(consumes<CaloParticleCollection>(pset.getParameter<edm::InputTag>("caloParticles"))),
      scCollectionToken_(consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("simClusters"))),
      svCollectionToken_(consumes<std::vector<SimVertex> >(pset.getParameter<edm::InputTag>("simVertices"))),
      stCollectionToken_(consumes<std::vector<SimTrack> >(pset.getParameter<edm::InputTag>("simTracks"))),
      caloRecHitToken_(consumes<HGCRecHitCollection>(pset.getParameter<edm::InputTag>("caloRecHits"))),
      tracksToken_(consumes<edm::View<reco::Track>>(pset.getParameter<edm::InputTag>("tracks"))),
      recHitToSCToken_(consumes<edm::Association<SimClusterCollection> >(pset.getParameter<edm::InputTag>("rechitToSimClusAssoc"))),
      tpToTrackToken_(consumes<reco::SimToRecoCollection>(pset.getParameter<edm::InputTag>("trackingPartToTrackAssoc"))),
      trackprop_(consumesCollector()),
      caloGeoToken_(esConsumes<edm::Transition::BeginRun>())
{
  produces<PFTruthParticleCollection>();
  produces<edm::Association<PFTruthParticleCollection>>("trackingPartToPFTruth");
  produces<edm::Association<PFTruthParticleCollection>>("simClusToPFTruth");
  produces<edm::Association<PFTruthParticleCollection>>("caloRecHitToPFTruth");
  produces<edm::Association<PFTruthParticleCollection>>("trackToPFTruth");

  pfMatchThreshold_=0.9;//to be configured
  pfMatchDRThreshold_=0.03;
}

PFTruthParticleProducer::~PFTruthParticleProducer() {}

//
// member functions
//




std::vector<size_t> PFTruthParticleProducer::matchedSCtoTrackIdxs(const SimClusterRefVector &simClusters,
        const TrackingParticle& tp, const reco::Track& track) const {

    std::vector<size_t>  out;
    // the SCs are from the same CP the TP was matched to.
    // here, the question is if a subset can be found that match also by
    // additional criteria. For now, just use all of them.
    // But the criterion needs to be related whether the match actually helps to determine the
    // properties.
    // e.g. if they are close enough to each other, if the last track in the TP
    // still has the right momentum etc etc.

    //merge the SCs and compare to TP momentum. If that is useful then it will be useful for PF

    //configure the merger here
    if(simClusters.size()<1)
        return out;

    //check which G4 track is most likely to be representing the reco track
    //auto proptrack = trackprop_.propagateObject(track,track.charge());

    //proptrack.momentum;
    //proptrack.pos;

    //make this simple
    float decz = track.outerPosition().z();
    bool takeall = fabs(decz) > 250.;
    if(!takeall)
        return std::vector<size_t>();
    for (const auto& scref : simClusters) {
        out.push_back(scref.key());
    }
    return out;

}

std::vector<SimClusterRefVector> PFTruthParticleProducer::splitCPToTP(const SimClusterRefVector& allCPSimClusters,
        TrackingParticleRefVector assoTPs, const SimHistoryTool& histtool) const {

    //cross-clean the TP g4 tracks (necessary?)
    //find highest energy non-common g4 tracks, profit from descending momentum-ordering
    //if any track is the same then the highest energy ones first
    std::vector<std::vector<SimTrack> > tpgsts;//needed for ambiguous cases
    std::vector<SimTrack> allsts;//needs copy as the above are also copies
    bool ambiguous=false;
    for(const auto& tp: assoTPs){
        tpgsts.push_back(tp->g4Tracks());
        for(const auto& st:tp->g4Tracks()){
          if (std::find_if(allsts.begin(), allsts.end(), [&st](const SimTrack& x) {
                return SimHistoryTool::simTrackEqual(x, st);
              }) == allsts.end()) {
            allsts.push_back(st);
          } else {
            ambiguous = true;
          }
        }
    }
    if(ambiguous){//more work needed, adapt tpgsts
        std::cout << "ambiguous tp g4 track matches" << std::endl;

        //clean up tpgsts here
    }

    //use (cleaned up) tpgsts to assign simclusters
    //use highest energy/momentum non overlapping g4track as probe
    std::vector<SimClusterRefVector> out;
    for(const auto& tpscs: tpgsts){
        out.push_back(SimClusterRefVector());
        auto &tpsv = out.at(out.size()-1);
        if(tpscs.size()<1)
            continue;
        const auto& parent = tpscs.at(0);
        for(const auto& sc: allCPSimClusters){
            for(const auto& scst: sc->g4Tracks()){//in case it is merged already, but usually only 1
                if(histtool.inHistory(parent, &scst)){
                    tpsv.push_back(sc);
                    break;
                }
            }
        }

    }
    return out;
}

// ------------ method called to produce the data  ------------
void PFTruthParticleProducer::produce(edm::Event &iEvent, const edm::EventSetup &iSetup) {



 //   throw std::runtime_error("PFTruthParticleProducer: currently not working yet, but skeleton is there");


  edm::Handle<TrackingParticleCollection> tpCollection;
  iEvent.getByToken(tpCollectionToken_, tpCollection);

  edm::Handle<CaloParticleCollection> cpCollection;
  iEvent.getByToken(cpCollectionToken_, cpCollection);

  edm::Handle<SimClusterCollection> scCollection;
  iEvent.getByToken(scCollectionToken_, scCollection);

  edm::Handle<std::vector<SimVertex> > svCollection;
  iEvent.getByToken(svCollectionToken_, svCollection);

  edm::Handle<std::vector<SimTrack> > stCollection;
  iEvent.getByToken(stCollectionToken_, stCollection);

  edm::Handle<HGCRecHitCollection> caloRecHitCollection;
  iEvent.getByToken(caloRecHitToken_, caloRecHitCollection);

  edm::Handle<edm::View<reco::Track>> trackCollection;
  iEvent.getByToken(tracksToken_, trackCollection);

  edm::Handle<edm::Association<SimClusterCollection>> recHitToSC;
  iEvent.getByToken(recHitToSCToken_, recHitToSC);

  edm::Handle<reco::SimToRecoCollection> tpToTrack;
  iEvent.getByToken(tpToTrackToken_, tpToTrack);


  SimHistoryTool histtool(*stCollection, *svCollection);
  //set up mergers

  HGCalSimClusterMerger neutralSCMerger(*caloRecHitCollection.product(),
          &hgcrechittools_,&histtool);
  //configure
  neutralSCMerger.setCNLayers(10);
  neutralSCMerger.setHighEfracThreshold(0.85);
  neutralSCMerger.setConnectThreshold(0.3);

  //index helpers
  std::map<int,int>  trackIdToTrackIdxAsso;
  for(size_t i=0;i<stCollection->size();i++){
      trackIdToTrackIdxAsso[stCollection->at(i).trackId()] = i;
  }


  //match tracking to calo particles
  std::vector<std::vector<size_t> > cpToTpIdxAsso(cpCollection->size());
  for(size_t i_cp=0; i_cp < cpCollection->size(); i_cp++){
      const auto& cp = cpCollection->at(i_cp);

      for(size_t i_tp=0; i_tp< tpCollection->size(); i_tp++){

          const auto& tp = tpCollection->at(i_tp);
          auto tproot = histtool.getRoot(&tp.g4Tracks().at(0));

          if(tproot->trackId() == cp.g4Tracks().at(0).trackId()
                  && tproot->eventId() == cp.g4Tracks().at(0).eventId()){
              cpToTpIdxAsso.at(i_cp).push_back(i_tp);
          }
      }
  }

  //take only CPs with TP


  //best TP for each track & <-> one-to-one
  std::vector<int> tpToTrackIdxAsso(tpCollection->size(),-1);//fill with no asso
  std::vector<int> trackToTpIdxAsso(trackCollection->size(),-1);
  for(size_t i=0; i< tpCollection->size(); i++){


      TrackingParticleRef tp(tpCollection,i);

      if (tpToTrack->find(tp) != tpToTrack->end()) {
          auto& trackRefs = (*tpToTrack)[tp];
          auto& bestTrackRef = trackRefs.at(0).first;
          double bestQual = trackRefs.at(0).second;
          //possible quality requirement here

          double relptdiff = (tp->pt() - bestTrackRef->pt())/tp->pt();
          if((relptdiff>1.5 || relptdiff<0.75) && bestTrackRef->pt()>100)//take all straight tracks
              continue; //just a test

          tpToTrackIdxAsso.at(i)=  bestTrackRef.key();
          trackToTpIdxAsso.at(bestTrackRef.key())=i;
      }
  }

  //take only CPs with TP with track, final association
  std::vector<std::vector<size_t> > trackToSCIdxAsso(trackCollection->size());
  std::vector<bool> scUsed(scCollection->size());

  for(size_t i_cp=0; i_cp < cpCollection->size(); i_cp++){

      TrackingParticleRefVector cptps;
      std::vector<size_t> trackidx;
      for(const size_t i_tp: cpToTpIdxAsso.at(i_cp)){//tracking part idxs
          int i_track = tpToTrackIdxAsso.at(i_tp);
          if(i_track<0)
              continue;
          trackidx.push_back(i_track);
          cptps.push_back(TrackingParticleRef(tpCollection,i_tp));
      }
      auto scgroups = splitCPToTP(cpCollection->at(i_cp).simClusters(), cptps, histtool);
      //this has the same indexing as tpToTrackIdxAsso
      for(size_t i_tt=0; i_tt< scgroups.size();i_tt++){
          int i_track = trackidx.at(i_tt);

          auto matchedsc = matchedSCtoTrackIdxs(scgroups.at(i_tt),*cptps.at(i_tt),
                  trackCollection->at(i_track));
          trackToSCIdxAsso.at(i_track)=matchedsc;

          for(const auto msc: matchedsc){
              scUsed.at(msc)=true;
          }
      }
  }

  //create PF truth from trackingParticles with tracks


  auto PFtruth = std::make_unique<PFTruthParticleCollection>() ;

  std::vector<int> tpToPFpartIdx(tpCollection->size(), -1);
  std::vector<int> scToPFpartIdx(scCollection->size(), -1);
  std::vector<int> rhToPFpartIdx(caloRecHitCollection->size(), -1);
  std::vector<int> trackToPFpartIdx(trackCollection->size(), -1);

  //create charged PF truth
  for (size_t i_t = 0; i_t < trackToSCIdxAsso.size(); i_t++) {

      if(trackToSCIdxAsso.at(i_t).size()<1)
          continue;//not matched SC, don't create a PF particle from it

      int PFTIdx = PFtruth->size();

      trackToPFpartIdx.at(i_t) = PFTIdx;

      auto tp = tpCollection->at(trackToTpIdxAsso.at(i_t));

      TrackingParticleRefVector tprefs;
      SimClusterRefVector screfs;

      //make the associations...
      tprefs.push_back(TrackingParticleRef(tpCollection,trackToTpIdxAsso.at(i_t)));
      tpToPFpartIdx.at(trackToTpIdxAsso.at(i_t)) = PFTIdx;

      float rec_calo_en = 0;
      for(const auto i_sc: trackToSCIdxAsso.at(i_t)){
          SimClusterRef scref(scCollection,i_sc);
          screfs.push_back(scref);
          scToPFpartIdx.at(i_sc) = PFTIdx;
      }

      PFTruthParticle pftp(tprefs,screfs);

      pftp.setCaloRecEnergy(rec_calo_en);
      pftp.setCharge(tp.charge());
      pftp.setPdgId(tp.pdgId());
      //this needs to be refined
      //ID etc
      pftp.setP4(PFTruthParticle::LorentzVectorF(tp.p4().px(), tp.p4().py(), tp.p4().pz(), tp.p4().t()));
      //pftp.setVertex(tp.vertex());
      //still missing time, etc
      PFtruth->push_back(pftp);
  }



  //scMerger;
  //take the remaining SCs and merge them and make PFTruth out of them
  std::vector<const SimCluster*> tomergehgcsc;
  std::vector<size_t> orig_indices;
  for(size_t i_sc=0;i_sc<scCollection->size();i_sc++){
      if(scUsed.at(i_sc)) continue;
      if(!neutralSCMerger.isHGCal(scCollection->at(i_sc)))
          continue;
      tomergehgcsc.push_back(&scCollection->at(i_sc));
      orig_indices.push_back(i_sc);
  }

  // reset muon momenta of muons without tracks to their reconstructed energies
  // the indices are not affected by this
  auto sc_tobemerged = neutralSCMerger.downScaleMuons(tomergehgcsc);
  tomergehgcsc.clear();
  for(const auto & sc: sc_tobemerged)
      tomergehgcsc.push_back(&sc);

  std::vector<std::vector<size_t> > mergeIdxs;

  auto merged = neutralSCMerger.merge(tomergehgcsc,mergeIdxs);
  //merged not used for now, needs to  output new collection in the end
  //but indices can help check basic functions for now

  for(size_t i=0;i<mergeIdxs.size();i++){

      const auto& mscs=mergeIdxs.at(i);
      const auto& msc = merged.at(i);
      int PFTIdx = PFtruth->size();

      SimClusterRefVector screfs;
      TrackingParticleRefVector tprefs;

      float rec_calo_en = 0;
      for(const auto ii_sc: mscs){
          size_t i_sc = orig_indices.at(ii_sc);
          SimClusterRef scref(scCollection,i_sc);
          screfs.push_back(scref);
          rec_calo_en += neutralSCMerger.recEnergy(*scref);
          scToPFpartIdx.at(i_sc)=PFTIdx;
      }

      //std::cout << "neutral rec_calo_en " << rec_calo_en << std::endl;
      PFTruthParticle pftp(tprefs,screfs);
      pftp.setCharge(0);
      pftp.setCaloRecEnergy(rec_calo_en);
      //ID etc
      pftp.setPdgId(msc.pdgId());//to be determined in same way as for SCs
      pftp.setP4(msc.impactMomentum());
      //still missing time, etc
      PFtruth->push_back(pftp);
  }

  //assign rechits
  for (size_t i = 0; i <  caloRecHitCollection->size(); i++) {
      HGCRecHitRef rh(caloRecHitCollection, i);
      auto scRef = (*recHitToSC)[rh];
      int scIdx = scRef.isNonnull() ? scRef.key() : -1;
      int pfIdx = scIdx >= 0 ? scToPFpartIdx[scIdx] : -1;
      if (scIdx >= 0)
          rhToPFpartIdx.at(i) = pfIdx;
  }

  auto pfTruthHand = iEvent.put(std::move(PFtruth));

  // Used to fill the associations in the ntuples
  auto tpAssoc = std::make_unique<edm::Association<PFTruthParticleCollection>>(pfTruthHand);
  auto scAssoc = std::make_unique<edm::Association<PFTruthParticleCollection>>(pfTruthHand);
  auto rhAssoc = std::make_unique<edm::Association<PFTruthParticleCollection>>(pfTruthHand);
  auto trackAssoc = std::make_unique<edm::Association<PFTruthParticleCollection>>(pfTruthHand);

  edm::Association<PFTruthParticleCollection>::Filler tpFiller(*tpAssoc);
  tpFiller.insert(tpCollection, tpToPFpartIdx.begin(), tpToPFpartIdx.end());
  tpFiller.fill();

  edm::Association<PFTruthParticleCollection>::Filler scFiller(*scAssoc);
  scFiller.insert(scCollection, scToPFpartIdx.begin(), scToPFpartIdx.end());
  scFiller.fill();

  edm::Association<PFTruthParticleCollection>::Filler rhFiller(*rhAssoc);
  rhFiller.insert(caloRecHitCollection, rhToPFpartIdx.begin(), rhToPFpartIdx.end());
  rhFiller.fill();

  edm::Association<PFTruthParticleCollection>::Filler trackFiller(*trackAssoc);
  trackFiller.insert(trackCollection, trackToPFpartIdx.begin(), trackToPFpartIdx.end());
  trackFiller.fill();

  iEvent.put(std::move(tpAssoc), "trackingPartToPFTruth");
  iEvent.put(std::move(scAssoc), "simClusToPFTruth");
  iEvent.put(std::move(rhAssoc), "caloRecHitToPFTruth");
  iEvent.put(std::move(trackAssoc), "trackToPFTruth");
}

// define this as a plug-in
DEFINE_FWK_MODULE(PFTruthParticleProducer);

