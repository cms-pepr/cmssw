/*
 * CMSSW plugin that performs a Window-based inference of networks using RecHits.
 *
 * Author: Marcel Rieger <marcel.rieger@cern.ch>
 */

#include <memory>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"

#include "RecoHGCal/GraphReco/interface/InferenceWindow.h"


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

class WindowInference: public edm::stream::EDAnalyzer<
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
    void analyze(const edm::Event&, const edm::EventSetup&);

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

    // mount the graphDef stored in windowInferenceCache onto the session
    //FIXME
    // session_ = tensorflow::createSession(windowInferenceCache->graphDef);
}

WindowInference::~WindowInference() {
}


void WindowInference::beginStream(edm::StreamID streamId) {
    windows_ = InferenceWindow::createWindows(nPhiSegments_,nEtaSegments_,minEta_,maxEta_,etaFrameWidth_,phiFrameWidth_);
}

void WindowInference::endStream() {
    // close the session
    //FIXME
    // tensorflow::closeSession(session_);
    session_ = nullptr;


    windows_.clear();
}

void WindowInference::analyze(const edm::Event& event,
        const edm::EventSetup& setup) {
    recHitTools_.getEventSetup(setup);


    // fill rechits into windows
    fillWindows(event);

    // run the evaluation per window
    for (auto & window : windows_) {
        window.evaluate(session_);
    }

    // reconstruct showers using all windows and put them into the event
    //reconstructShowers();

    // clear all windows
    for (auto& window : windows_) {
        window.clear();
    }
}



void WindowInference::fillWindows(const edm::Event& event) {

    if (!windows_.size()) {
        throw cms::Exception("NoWindows") << "no windows initialized";
    }

    //Window::mode windowmode = windows_.at(0).getMode();
    // skip layer cluster or rechit loop accordingly

    //FIXME


}

//remove

DEFINE_FWK_MODULE(WindowInference);
