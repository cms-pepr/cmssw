/*
 * CMSSW plugin that performs a Window-based inference of networks using RecHits and produces PF candidates.
 *
 * Authors: Gerrit Van Onsem <Gerrit.Van.Onsem@cern.ch>
 *          Marcel Rieger <marcel.rieger@cern.ch>
 */

#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "RecoHGCal/GraphReco/interface/InferenceWindow.h"

#include "DataFormats/Common/interface/View.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h" 
#include "DataFormats/Candidate/interface/LeafCandidate.h"

#include "DataFormats/HGCalReco/interface/Trackster.h"

#include "HeterogeneousCore/SonicCore/interface/SonicEDProducer.h"
#include "HeterogeneousCore/SonicTriton/interface/TritonClient.h"


using namespace ticl;

// macros for simplified logs
// message logger disabled for the moment
// #define INFO edm::LogInfo("peprCandidateFromHitProducer")
// #define WARNING edm::LogWarning("peprCandidateFromHitProducer")
// #define ERROR edm::LogError("peprCandidateFromHitProducer")
#define INFO std::cout << "peprCandidateFromHitProducer INFO   : "
#define WARNING std::cout << "peprCandidateFromHitProducer WARNING: "
#define ERROR std::cout << "peprCandidateFromHitProducer ERROR  : "


class peprCandidateFromHitProducer: public SonicEDProducer<TritonClient> {    
 public:
    explicit peprCandidateFromHitProducer(edm::ParameterSet const&);
    ~peprCandidateFromHitProducer();

 private:
    void acquire(edm::Event const&, edm::EventSetup const&, Input&) override;
    void produce(edm::Event&, edm::EventSetup const&, Output const&) override;

    void fillWindows(const edm::Event&);
    void readOutput(Output const&, std::vector<std::vector<float> >&);

    // options
    std::vector<edm::InputTag> recHitCollections_;

    // tokens
    std::vector<edm::EDGetTokenT<HGCRecHitCollection> > recHitTokens_;  
    edm::EDGetTokenT<edm::View<reco::Track>> tracksToken_;

    // rechit tools
    hgcal::RecHitTools recHitTools_;

    double minCandEnergy_;

    // windows
    std::vector<InferenceWindow> windows_;

    double minEta_;
    double maxEta_;
    double etaFrameWidth_;
    double phiFrameWidth_;
    size_t nEtaSegments_;
    size_t nPhiSegments_;

};


peprCandidateFromHitProducer::peprCandidateFromHitProducer(edm::ParameterSet const& config) : SonicEDProducer<TritonClient>(config),
        recHitCollections_(config.getParameter<std::vector<edm::InputTag> >("recHitCollections")), 
        tracksToken_(consumes<edm::View<reco::Track>>(config.getParameter<edm::InputTag>("tracks"))),
        minCandEnergy_(config.getParameter<double>("minCandEnergy")),
        //FIXME: actually these are all not needed if windows are created in the constructor!
        minEta_(config.getParameter<double>("minEta")),
        maxEta_(config.getParameter<double>("maxEta"))
        {

    // get tokens
    for (edm::InputTag& recHitCollection : recHitCollections_) {
        recHitTokens_.push_back(consumes<HGCRecHitCollection>(recHitCollection));
    }
   
    // window size and overlap in phi and eta
    etaFrameWidth_ = 0.2;
    phiFrameWidth_ = 0.2;
    nEtaSegments_ = 1;
    nPhiSegments_ = 1;

    produces<reco::PFCandidateCollection>();
    //produces<std::vector<Trackster>>();

    windows_ = InferenceWindow::createWindows(nPhiSegments_,nEtaSegments_,minEta_,maxEta_,etaFrameWidth_,phiFrameWidth_);
    // FIXME, make configurable?
    for(auto& w: windows_)
        w.setMode(WindowBase::useRechits);

    //std::cout << "  Starting server" << std::endl;
    //system("/afs/cern.ch/work/g/gvonsem/public/HGCAL/ML/pepr_11_2_0_pre9/CMSSW_11_2_0_pre9/src/HeterogeneousCore/SonicTriton/test/triton start");
}


peprCandidateFromHitProducer::~peprCandidateFromHitProducer() {

    //std::cout << "  Stopping server" << std::endl;
    //system("/afs/cern.ch/work/g/gvonsem/public/HGCAL/ML/pepr_11_2_0_pre9/CMSSW_11_2_0_pre9/src/HeterogeneousCore/SonicTriton/test/triton stop");
}


void peprCandidateFromHitProducer::acquire(edm::Event const& iEvent, edm::EventSetup const& iSetup, Input& iInput) {

    //INFO << "Acquire method" << std::endl;

    edm::ESHandle<CaloGeometry> geom;
    iSetup.get<CaloGeometryRecord>().get(geom);
    recHitTools_.setGeometry(*geom);

    // fill rechits into windows
    fillWindows(iEvent);

    std::vector<std::vector<float> >  hitFeatures;
    //vector of number of hits per window
    std::vector<int> nhitsWindow; 

    for (auto & window : windows_) {

        std::cout << " New window " << std::endl;

        nhitsWindow.push_back(window.getHitFeatures().size());
        
        //append vector to previous vector
        hitFeatures.insert(hitFeatures.end(),window.getHitFeatures().begin(),window.getHitFeatures().end());
        std::cout << "   hitFeatures size = " << hitFeatures.size() << std::endl;
        std::cout << "      number of features per hit = " << hitFeatures[0].size() << std::endl;

        //break; //if quickly testing just one window
    }

    //set shapes
    auto& input1 = iInput.at("input_1");
    //input_1 has dims:[ -1, 9 ], this setShape command changes the variable dimension at location 0 to the total number of rechits in the windows
    input1.setShape(0, hitFeatures.size()); 
    auto data1 = std::make_shared<TritonInput<float>>(1);
    auto& vdata1 = (*data1)[0];
    vdata1.reserve(input1.sizeShape());

    std::cout << " input1.sizeShape = " << input1.sizeShape() << std::endl;
        
    auto& input2 = iInput.at("input_2");
    input2.setShape(0, hitFeatures.size()); 
    auto data2 = std::make_shared<TritonInput<int64_t>>(1);
    auto& vdata2 = (*data2)[0];
    vdata2.reserve(input2.sizeShape());

    std::cout << " input2.sizeShape = " << input2.sizeShape() << std::endl;
        
    //fill first input: total list of rechits of all windows
    for (size_t i=0; i<hitFeatures.size(); i++) {
        for (size_t j=0; j<hitFeatures[i].size(); j++) {
            vdata1.push_back(hitFeatures[i][j]);
        }
    }

    //fill second input; this is the row splits: for two windows it is an array (of length the number of rechits in the window) of zeroes, 
    //except the 2nd element is nrechits of first window, the 3rd element is nrechits of first+second window, 
    //and the last element is 3 (the length of the non-zero-padded array)
    for (unsigned i = 0; i < input2.sizeShape(); ++i) {
        vdata2.push_back(0);
    }
    for (unsigned i = 0; i < nhitsWindow.size(); ++i) {
        vdata2[i+1] = vdata2[i] + int64_t(nhitsWindow[i]);
    }
    vdata2[hitFeatures.size()-1] = int64_t(nhitsWindow.size()+1);

    // convert to server format
    input1.toServer(data1);
    input2.toServer(data2);

    std::cout << " window input sent" << std::endl;
}


void peprCandidateFromHitProducer::produce(edm::Event& iEvent, edm::EventSetup const& iSetup, Output const& iOutput) {

    //INFO << "Produce method" << std::endl;
 
    //inner vector is regressed energy, 2D position, and time (total 4 elements)
    std::vector<std::vector<float> >  candidates;

    readOutput(iOutput,candidates); 

    // making candidate collection
    auto pfcandidates = std::make_unique<reco::PFCandidateCollection>();
    //std::cout << "Creating PF candidates " << std::endl;
    float E=-9999., X = -9999., Y=-9999., Z=-9999.;

    // loop over particles reconstructed in the current window
    for(size_t j=0;j<candidates.size();j++) {   

        //const auto abs_pdg_id = -9999;
        const auto charge = 0; // FIXME
                 
        E = candidates[j][0];
        //std::cout << "  E before cut = " << E << std::endl;

        //temporary lower threshold on energy of candidates
        if(E < minCandEnergy_) continue;
        
        std::cout << " particle " << j << std::endl;

        X = candidates[j][1];
        Y = candidates[j][2];
        Z = candidates[j][3];
        std::cout << "  (X,Y,Z,E) = " << "(" << X << "," << Y << "," << Z << "," << E << ")" << std::endl;

        //block inspired by calcP4 method in TICL TracksterP4FromEnergySum plugin
        //below assuming (0,0,0) as vertex
        //starts from 'position (X,Y,Z)'
        math::XYZVector direction(X, Y, Z);
        direction = direction.Unit();
        direction *= E;
        math::XYZTLorentzVector cartesian(direction.X(), direction.Y(), direction.Z(), E);
        //// Convert px, py, pz, E vector to CMS standard pt/eta/phi/m vector
        reco::Candidate::LorentzVector p4(cartesian);

        // //Alternative calculation as check
        // float R = std::sqrt(X*X + Y*Y + Z*Z);
        // float px = E * X / R;
        // float py = E * Y / R;
        // float pz = E * Z / R;
        // std::cout << "  (px,py,pz) = " << "(" << px << "," << py << "," << pz << ")" << std::endl;

        const auto& four_mom = p4;
        std::cout << "    PF particle energy " << four_mom.E() << ", Px = " << four_mom.Px() << ", Py = " << four_mom.Py() << ", Pz = " << four_mom.Pz() << std::endl;
        reco::PFCandidate::ParticleType part_type = reco::PFCandidate::X;
        pfcandidates->emplace_back(charge, four_mom, part_type);

    }

    iEvent.put(std::move(pfcandidates));
    std::cout << " Results produced and put in event" << std::endl;

    // clear all windows
    for (auto& window : windows_) {
        window.clear();
    }
    std::cout << " Windows cleared" << std::endl;
}



void peprCandidateFromHitProducer::fillWindows(const edm::Event& event) {

    if (!windows_.size()) {
        throw cms::Exception("NoWindows") << "no windows initialized";
    }
    
    std::cout << "Number of windows = " << windows_.size() << std::endl;
    
    // copied block from window ntupler code
    // get rechits, get positions and merge collections
    std::vector<HGCRecHitWithPos> allrechits;
    for (auto & token : recHitTokens_) {
       for (const auto& rh : event.get(token)) {
           HGCRecHitWithPos rhp = { const_cast<HGCRecHit*>(&rh), recHitTools_.getPosition(rh.detid()) };
           allrechits.push_back(rhp);
       }
    }

    // sort according to the energy
    std::sort(allrechits.begin(), allrechits.end(), 
        [](const HGCRecHitWithPos& rh1, const HGCRecHitWithPos& rh2) 
            { return rh1.hit->energy() > rh2.hit->energy();});

    std::cout << " rechits size: " << allrechits.size() << std::endl;

    // fills a vector of the specified size with zeroes (entries will be 0 if rechit is not filled, and 1 if it is filled)
    std::vector<size_t> filledrechits(allrechits.size(),0);

    for (auto & window : windows_) {
        //fill rechits in this window
        for(size_t it=0;it<allrechits.size();it++) {
           if(filledrechits.at(it)>3) continue;
           if(window.maybeAddRecHit(allrechits.at(it)))
               filledrechits.at(it)++;
        }

        window.fillFeatureArrays();
    }

    std::cout << " windows filled" << std::endl;
}


void peprCandidateFromHitProducer::readOutput(Output const& iOutput, std::vector<std::vector<float> >& candidates) {

    const auto& output1 = iOutput.at("output");
    // convert from server format
    const auto& tmp = output1.fromServer<float>();

    std::cout << "output1.shape()[0] = " << output1.shape()[0] << std::endl;
    std::cout << "output1.shape()[1] = " << output1.shape()[1] << std::endl; 

    for (int i = 0; i < output1.shape()[0]; ++i) {
      //std::cout << "output " << i << ": ";
      if(output1.shape()[1] == 16 || output1.shape()[1] == 17) {

            std::vector<float> fulloutput;
            for (int j = 0; j < output1.shape()[1]; ++j) {                   
                fulloutput.push_back(tmp[0][output1.shape()[1] * i + j]); 
            }
            //std::cout << "\n";

            float E = -9999., X = -9999., Y = -9999., Z = -9999., Xrel = -9999., Yrel = -9999.; //T = -9999.;

            //input rechit position
            X = fulloutput[5];
            Y = fulloutput[6];
            Z = fulloutput[7];
            E = fulloutput[10];
            //values are relative to input rechit position
            Xrel = fulloutput[11];
            Yrel = fulloutput[12];
            //FIXME: is T also relative to T of rechit? Not used for now
            //T = values[13]; 

            X = X + Xrel;
            Y = Y + Yrel;

            //positive Z of rechit means Z = 320 cm (as regressed X,Y is measured on HGCAL surface)
            if ( Z > 0 ) Z = 320.;  //in cm
            else Z = -320.;   //in cm
                    
            //std::cout << "    Storing values from the model output" << std::endl;
            std::vector<float> candidate = {E, X, Y, Z };
            //std::cout << "  from output: (X,Y,Z,E) = " << "(" << X << "," << Y << "," << Z << "," << E << ")" << std::endl;
            candidates.push_back( candidate );
      }
      else 
        std::cout << "    Please check model output to retrieve desired regressed properties" << std::endl;
    }

    std::cout << " candidates size = " << candidates.size() << std::endl;
}


DEFINE_FWK_MODULE(peprCandidateFromHitProducer);
