/*
 * CMSSW plugin that performs a Window-based inference of networks using RecHits.
 *
 * Authors: Marcel Rieger <marcel.rieger@cern.ch>
 *          Gerrit Van Onsem <Gerrit.Van.Onsem@cern.ch>
 */

#include <memory>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"

#include "RecoHGCal/GraphReco/interface/InferenceWindow.h"

#include "DataFormats/Common/interface/View.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h" 
#include "DataFormats/Candidate/interface/LeafCandidate.h"


// macros for simplified logs
// message logger disabled for the moment
// #define INFO edm::LogInfo("WindowInference")
// #define WARNING edm::LogWarning("WindowInference")
// #define ERROR edm::LogError("WindowInference")
#define INFO std::cout << "WindowInference INFO   : "
#define WARNING std::cout << "WindowInference WARNING: "
#define ERROR std::cout << "WindowInference ERROR  : "

// datastructure hold by edm::GlobalCache
struct WindowInferenceCache {
    WindowInferenceCache(const edm::ParameterSet& config) :
            graphDef(nullptr) {
    }

    std::atomic<tensorflow::GraphDef*> graphDef;
};

class WindowInference: public edm::stream::EDProducer<
        edm::GlobalCache<WindowInferenceCache> > {
 public:
    explicit WindowInference(const edm::ParameterSet&,
            const WindowInferenceCache*);
    ~WindowInference();

    // methods for handling the global cache
    static std::unique_ptr<WindowInferenceCache> initializeGlobalCache(
            const edm::ParameterSet&);
    static void globalEndJob(const WindowInferenceCache*);

 private:
    void beginStream(edm::StreamID);
    void endStream();
    void produce(edm::Event&, const edm::EventSetup&) override;

    void fillWindows(const edm::Event&);


    // options
    std::vector<edm::InputTag> recHitCollections_;

    std::string inputTensorName_;
    std::string outputTensorName_;
    bool batchedModel_;
    size_t padSize_;

    // tokens
    std::vector<edm::EDGetTokenT<HGCRecHitCollection> > recHitTokens_;

    // rechit tools
    hgcal::RecHitTools recHitTools_;

    // windows
    std::vector<InferenceWindow> windows_;

    double minEta_;
    double maxEta_;
    double etaFrameWidth_;
    double phiFrameWidth_;
    size_t nEtaSegments_;
    size_t nPhiSegments_;

    // the tensorflow session
    tensorflow::Session* session_;


};

std::unique_ptr<WindowInferenceCache> WindowInference::initializeGlobalCache(
        const edm::ParameterSet& config) {
    // this method is supposed to create, initialize and
    //return a WindowInferenceCache instance
    WindowInferenceCache* windowInferenceCache = new WindowInferenceCache(
            config);

    // load the graph def and save it
    std::string graphPath = config.getParameter<std::string>("graphPath");
    INFO<< "loading graph from " << graphPath << std::endl;
    windowInferenceCache->graphDef = tensorflow::loadGraphDef(graphPath);

    // set some global configs, such as the TF log level
    tensorflow::setLogging("0");

    return std::unique_ptr<WindowInferenceCache>(windowInferenceCache);
}

void WindowInference::globalEndJob(
        const WindowInferenceCache* windowInferenceCache) {
    // reset the graphDef
    if (windowInferenceCache->graphDef != nullptr) {
        delete windowInferenceCache->graphDef;
    }
}

WindowInference::WindowInference(const edm::ParameterSet& config,
        const WindowInferenceCache* windowInferenceCache) :
        recHitCollections_(
                config.getParameter<std::vector<edm::InputTag> >(
                        "recHitCollections")), inputTensorName_(
                config.getParameter<std::string>("inputTensorName")), outputTensorName_(
                config.getParameter<std::string>("outputTensorName")), batchedModel_(
                config.getParameter<bool>("batchedModel")), padSize_(
                (size_t) config.getParameter<uint32_t>("padSize")),

                //FIXME: actually these are all not needed if windows are created in the constructor!
                minEta_(config.getParameter<double>("minEta")),
                maxEta_(config.getParameter<double>("maxEta")),
                etaFrameWidth_(config.getParameter<double>("etaFrameWidth")),
                phiFrameWidth_(config.getParameter<double>("phiFrameWidth")),
                nEtaSegments_((size_t)config.getParameter<uint32_t>("nEtaSegments")),
                nPhiSegments_((size_t)config.getParameter<uint32_t>("nPhiSegments")),
                session_(nullptr){
    // sanity checks for sliding windows


    // get tokens
    for (edm::InputTag& recHitCollection : recHitCollections_) {
        recHitTokens_.push_back(
                consumes<HGCRecHitCollection>(recHitCollection));
    }

    produces<reco::PFCandidateCollection>();

    // mount the graphDef stored in windowInferenceCache onto the session
    session_ = tensorflow::createSession(windowInferenceCache->graphDef);
}

WindowInference::~WindowInference() {
}


void WindowInference::beginStream(edm::StreamID streamId) {
    windows_ = InferenceWindow::createWindows(nPhiSegments_,nEtaSegments_,minEta_,maxEta_,etaFrameWidth_,phiFrameWidth_);

    // FIXME, make configurable?
    for(auto& w: windows_)
        w.setMode(WindowBase::useRechits);
}

void WindowInference::endStream() {
    // close the session
    tensorflow::closeSession(session_);
    //session_ = nullptr;


    windows_.clear();
}

void WindowInference::produce(edm::Event& event, const edm::EventSetup& setup) {

    recHitTools_.getEventSetup(setup);


    // fill rechits into windows
    fillWindows(event);

    // one tensor per window
    std::vector<tensorflow::Tensor> windowoutputs;
    // run the evaluation per window
    for (auto & window : windows_) {
        window.evaluate(session_);
        windowoutputs.push_back(window.getOutput());
    }

    // reconstruct showers using all windows and put them into the event
    //reconstructShowers();


    // making PF candidate collection
    auto candidates = std::make_unique<reco::PFCandidateCollection>();
    for (unsigned int i=0; i<windowoutputs.size(); i++) {
        //loop over windows
        //std::cout << "Window " << i << std::endl;

        // check and print the output for ith window 
        float* data = windowoutputs[i].flat<float>().data();
        //std::cout << " outputs shape dimensions: " << windowoutputs[i].shape().dims() << std::endl;
        //std::cout << "   outputs shape 0: " << windowoutputs[i].shape().dim_size(0) << std::endl;
        //std::cout << "   outputs shape 1: " << windowoutputs[i].shape().dim_size(1) << std::endl;
        //std::cout << "   outputs shape 2: " << windowoutputs[i].shape().dim_size(2) << std::endl;
        
        // FIXME: convert E, px, py, pz to XYZT maybe?
        //        for now: assume the lorentzvector is in the right format already (dummy)
        float X = -9999., Y=-9999., Z=-9999., T=-9999.;
        // loop over particles reconstructed in the window
        for (int k = 0; k < windowoutputs[i].shape().dim_size(1); k++) { 
            //std::cout << " particle " << k << std::endl;
            //const auto abs_pdg_id = -9999;
            //const auto charge = -9999;
            const auto charge = 0; // FIXME!
            X = *data;
            //std::cout << "   four-vector X: " << X << std::endl;
            data++;
            Y = *data;
            //std::cout << "   four-vector Y: " << Y << std::endl;
            data++;
            Z = *data;
            //std::cout << "   four-vector Z: " << Z << std::endl;
            data++;
            T = *data;
            //std::cout << "   four-vector T: " << T << std::endl;
            const auto& four_mom = math::XYZTLorentzVector(X,Y,Z,T);
            reco::PFCandidate::ParticleType part_type = reco::PFCandidate::X;
            candidates->emplace_back(charge, four_mom, part_type);
        }

    }

    event.put(std::move(candidates));



    // clear all windows
    for (auto& window : windows_) {
        window.clear();
    }
}



void WindowInference::fillWindows(const edm::Event& event) {

    if (!windows_.size()) {
        throw cms::Exception("NoWindows") << "no windows initialized";
    }

    //FIXME
    //Window::mode windowmode = windows_.at(0).getMode();
    // skip layer cluster or rechit loop accordingly


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

    
    // fills a vector of the specified size with zeroes (entries will be 0 if rechit is not filled, and 1 if it is filled)
    std::vector<size_t> filledrechits(allrechits.size(),0);

    // FIXME: make number of features configurable?
    size_t nfeatures = 12;

    for (auto & window : windows_) {
        //fill rechits in this window
        for(size_t it=0;it<allrechits.size();it++) {
           if(filledrechits.at(it)>3) continue;
           if(window.maybeAddRecHit(allrechits.at(it)))
               filledrechits.at(it)++;
        }

        // TF interface setup needs to be called before fillFeatureArrays, in order to do the zero padding
        window.setupTFInterface(padSize_, nfeatures, batchedModel_, inputTensorName_, outputTensorName_);
        window.fillFeatureArrays(); 
    }


}

//remove

DEFINE_FWK_MODULE(WindowInference);
