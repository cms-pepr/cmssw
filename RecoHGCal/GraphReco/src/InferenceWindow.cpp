/*
 * InferenceWindow.cpp
 *
 *  Created on: 26 Sep 2019
 *      Author: jkiesele
 */



#include "../interface/InferenceWindow.h"

InferenceWindow::InferenceWindow(float centerEta, float centerPhi,
        float outerRegionDEta, float outerRegionDPhi, float innerRegionDEta,
        float innerRegionDPhi) :
        WindowBase(centerEta, centerPhi, outerRegionDEta, outerRegionDPhi,
                innerRegionDEta, innerRegionDPhi)
/* more inits */
{
}

InferenceWindow::~InferenceWindow() {

}

//static
std::vector<InferenceWindow> InferenceWindow::createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi){
    return WindowBase::createWindows<InferenceWindow>(nSegmentsPhi,nSegmentsEta,
            minEta,maxEta,frameWidthEta,frameWidthPhi);
}

void InferenceWindow::fillFeatureArrays(){
    float * data = 0; //inputTensor.data()

    if(getMode() == useRechits){
        for(const auto& rh:recHits){
            fillRecHitFeatures(data,rh);
        }
    }
    else{
        for(const auto& lc: layerClusters_){
            fillLayerClusterFeatures(data,lc);
        }
    }
    //add tracks LAST!
    for(const auto& tr:tracks_){
        fillTrackFeatures(data,tr);
    }
    //do some zero padding if needed

    //FIXME

}

void InferenceWindow::setupTFInterface(size_t padSize, size_t nFeatures, bool batchedModel,
        const std::string& inputTensorName,
        const std::string& outputTensorName) {

}



void InferenceWindow::evaluate(tensorflow::Session* sess) {

}
