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

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"


class CPtoSimClusters : public edm::stream::EDProducer<> {
    public:
        explicit CPtoSimClusters(const edm::ParameterSet&);
        ~CPtoSimClusters() {}


    private:
        void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override;

        edm::EDGetTokenT<CaloParticleCollection> cpCollectionToken_;
        edm::EDGetTokenT<std::vector<SimVertex> > svCollectionToken_;

    };

CPtoSimClusters::CPtoSimClusters(const edm::ParameterSet &pset) :
              cpCollectionToken_(consumes<CaloParticleCollection>(pset.getParameter<edm::InputTag>("caloParticles"))),
              svCollectionToken_(consumes<std::vector<SimVertex> >(pset.getParameter<edm::InputTag>("simVertices")))
{
    produces<SimClusterCollection>();
}

void CPtoSimClusters::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {


    edm::Handle<CaloParticleCollection> cpCollection;
    iEvent.getByToken(cpCollectionToken_, cpCollection);

    edm::Handle<std::vector<SimVertex> > svCollection;
    iEvent.getByToken(svCollectionToken_, svCollection);

    auto output = std::make_unique<SimClusterCollection>();

    for(const auto& cp: *cpCollection){
        SimCluster cpsc;
        bool first = true;
        for(const auto& sc: cp.simClusters()){
            if(first){
                cpsc = *sc;
                first = false;
            }
            else{
                cpsc += *sc;
            }
        }

        cpsc.setImpactMomentum(cp.p4());
        auto vertex = svCollection->at(cp.g4Tracks().at(0).vertIndex());
        cpsc.setImpactPoint(math::XYZTLorentzVectorF(vertex.position()));
        cpsc.setPdgId(cp.pdgId());//done
        //cp.
        //cp.
        output->push_back(cpsc);
    }

    iEvent.put(std::move(output));

}

DEFINE_FWK_MODULE(CPtoSimClusters);
