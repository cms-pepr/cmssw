/*
 * InferenceWindow.h
 *
 *  Created on: 26 Sep 2019
 *      Author: jkiesele
 */

#ifndef SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_
#define SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_

#include "../interface/WindowBase.h"
#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"

class InferenceWindow: public WindowBase {
public:


    InferenceWindow(float centerEta, float centerPhi, float outerRegionDEta, float outerRegionDPhi,
            float innerRegionDEta, float innerRegionDPhi);

    ~InferenceWindow();

    void setupTFInterface(size_t padSize, size_t nFeatures, bool batchedModel,
            const std::string& inputTensorName,
            const std::string& outputTensorName);

    void fillFeatureArrays();

    void evaluate(tensorflow::Session* sess);

    tensorflow::Tensor getOutput();

    void flattenRechitFeatures();

    static std::vector<InferenceWindow> createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi);

private:

    InferenceWindow(){}
    //
    //Inference  
    tensorflow::Tensor inputTensor_;
    tensorflow::NamedTensorList inputTensorList_;
    tensorflow::Tensor outputTensor_;
    std::string inputTensorName_;
    std::string outputTensorName_;

    size_t padSize_;
    size_t nFeatures_;
    bool batchedModel_;
};


#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_ */
