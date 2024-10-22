#include <memory>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <map>
#include <sstream>
#include <utility>
#include <set>
#include <cmath>
#include <numeric>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Ref.h"

#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/View.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "DataFormats/DetId/interface/DetId.h"

#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/Common/interface/OneToManyWithQualityGeneric.h"

#include "RecoParticleFlow/PFTruthProducer/interface/HGCalSimClusterMerger.h"
#include "RecoParticleFlow/PFTruthProducer/interface/SimHistoryTool.h"

#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t


typedef edm::AssociationMap<edm::OneToManyWithQualityGeneric<
    SimClusterCollection, SimClusterCollection, float>> SimClusterToSimClusters;




class SimClusterMerger : public edm::stream::EDProducer<> {
    public:
        explicit SimClusterMerger(const edm::ParameterSet&);
        ~SimClusterMerger() {}

        void beginRun(const edm::Run&, const edm::EventSetup& iSetup) override {
          auto& geom = iSetup.getData(caloGeoToken_);
          hgcalRecHitToolInstance_.setGeometry(geom);//OPT
        }


    private:
        void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override;
        mutable hgcal::RecHitTools hgcalRecHitToolInstance_ ;

        edm::EDGetTokenT<SimClusterCollection> scCollectionToken_;
        edm::EDGetTokenT<std::vector<SimVertex> > svCollectionToken_;
        edm::EDGetTokenT<std::vector<SimTrack> > stCollectionToken_;
        edm::EDGetTokenT<HGCRecHitCollection> caloRecHitToken_;
        edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeoToken_;

        //merge config
        int cNLayers_;
        float cIsHighEfracThreshold_;
        float cConnectThreshold_;
    };

SimClusterMerger::SimClusterMerger(const edm::ParameterSet &pset) :
        scCollectionToken_(consumes<SimClusterCollection>(pset.getParameter<edm::InputTag>("simClusters"))),
        svCollectionToken_(consumes<std::vector<SimVertex> >(pset.getParameter<edm::InputTag>("simVertices"))),
        stCollectionToken_(consumes<std::vector<SimTrack> >(pset.getParameter<edm::InputTag>("simTracks"))),
        caloRecHitToken_(consumes<HGCRecHitCollection>(pset.getParameter<edm::InputTag>("caloRecHits"))),
        caloGeoToken_(esConsumes<edm::Transition::BeginRun>())
    {
    produces<SimClusterCollection>();
    produces<edm::Association<SimClusterCollection>>();
    produces<SimClusterToSimClusters>();

    //move to initialisers
    cNLayers_ = pset.getParameter<int32_t> ( "useNLayers" );
    cIsHighEfracThreshold_ = pset.getParameter<double> ( "highEfracThreshold" );
    cConnectThreshold_ = pset.getParameter<double> ( "connectThreshold" );

    }

void SimClusterMerger::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    /*
     *
     * add simhits to get the time in a proper way
     *
     *
     */

    //get collections
    edm::Handle<SimClusterCollection> scCollection;
    iEvent.getByToken(scCollectionToken_, scCollection);

    edm::Handle<HGCRecHitCollection> caloRecHitCollection;
    iEvent.getByToken(caloRecHitToken_, caloRecHitCollection);

    //these two to follow history at some point
    edm::Handle<std::vector<SimVertex> > svCollection;
    iEvent.getByToken(svCollectionToken_, svCollection);

    edm::Handle<std::vector<SimTrack> > stCollection;
    iEvent.getByToken(stCollectionToken_, stCollection);

    SimHistoryTool histtool(*stCollection.product(), *svCollection.product());

    HGCalSimClusterMerger merger(*caloRecHitCollection.product(),
            &hgcalRecHitToolInstance_, &histtool);


    merger.setCNLayers(cNLayers_);
    merger.setHighEfracThreshold(cIsHighEfracThreshold_);
    merger.setConnectThreshold(cConnectThreshold_);

    std::vector<const SimCluster*> tobemerged;
    for(size_t i = 0; i < scCollection->size(); i++)
        tobemerged.push_back(&scCollection->at(i));

    //reduce muon impact momenta
    auto sc_tobemerged = merger.downScaleMuons(tobemerged);
    tobemerged.clear();
    for(size_t i = 0; i < scCollection->size(); i++)
        tobemerged.push_back(&sc_tobemerged.at(i));

    //cMergeThreshold_
    //FIXME remove hard coded
    std::vector<std::vector<size_t> > idxs; //// <--- @Shah Rukh: that's the one to check if something is merged
    auto mergedSC = merger.merge(tobemerged, idxs); //now idxs is filled

    std::cout << "SCmerger initial: " << scCollection->size() << " merged: " << idxs.size() << std::endl;//DEBUG
    size_t largestgroup=0;
    for(const auto& iidx: idxs)
        if(largestgroup<iidx.size())
            largestgroup=iidx.size();
    std::cout << "largest group " << largestgroup << std::endl;

    //generate output and associations

    auto output = std::make_unique<SimClusterCollection>();

    for(const auto& sc:mergedSC)
        output->push_back(sc);//DEBUG can be moved to constructor once validated

    const auto& mergedSCHandle = iEvent.put(std::move(output));


    ///now only associations

    auto assocMap = std::make_unique<SimClusterToSimClusters>(mergedSCHandle, scCollection);


    for (size_t i = 0; i < mergedSC.size(); i++) {
        SimClusterRef msc(mergedSCHandle, i);//merged collection
        float mergedE = msc->impactMomentum().E();

        for (auto i_sc : idxs.at(i)) {
            SimClusterRef umsc(scCollection,i_sc);//unmerged collection
            float scE = umsc->impactMomentum().E();
            assocMap->insert(msc, std::make_pair(umsc, scE/mergedE));
        }
    }
    iEvent.put(std::move(assocMap));

    //invert
    std::vector<int> mergedIndices(tobemerged.size(),-1);
    for (size_t i = 0; i < tobemerged.size(); i++) {
        for(size_t j=0; j< idxs.size(); j++){
            if(std::find(idxs.at(j).begin(),idxs.at(j).end(),i) != idxs.at(j).end()){
                mergedIndices.at(i) = j;
                //std::cout << i <<" unmerged collection is merged to " << j << " merged collection " << std::endl;
                break;
            }
        }
    }


    auto assoc = std::make_unique<edm::Association<SimClusterCollection>>(mergedSCHandle);
    edm::Association<SimClusterCollection>::Filler filler(*assoc);
    filler.insert(scCollection, mergedIndices.begin(), mergedIndices.end());
    filler.fill();
    iEvent.put(std::move(assoc));


}

DEFINE_FWK_MODULE(SimClusterMerger);
