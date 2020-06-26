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
    float* data = inputTensor_.flat<float>().data();

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
    if(getMode() == useRechits)
    {
        for (float i = nFeatures_*recHits.size(); i < nFeatures_*padSize_; i++, data++)
        {
            *data = 0.;
        }
    }
}

void InferenceWindow::setupTFInterface(size_t padSize, size_t nFeatures, bool batchedModel,
        const std::string& inputTensorName,
        const std::string& outputTensorName) {    

    inputTensorName_ = inputTensorName;
    outputTensorName_ = outputTensorName;

    padSize_ = padSize;
    nFeatures_ = nFeatures;
    inputTensor_ = tensorflow::Tensor(tensorflow::DT_FLOAT, { 1, (int) padSize_, (int) nFeatures_ });

}



void InferenceWindow::evaluate(tensorflow::Session* sess) {

    std::vector<tensorflow::Tensor> outputs;
    tensorflow::run(sess, { { inputTensorName_, inputTensor_ } }, { outputTensorName_ }, &outputs);
    outputTensor_ = outputs[0];
}



tensorflow::Tensor InferenceWindow::getOutput(){

    return outputTensor_;
}