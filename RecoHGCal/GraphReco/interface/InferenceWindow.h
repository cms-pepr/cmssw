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

    void getOutput() const{}//needs output format etc.

    void flattenRechitFeatures();

    static std::vector<InferenceWindow> createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi);

private:

    InferenceWindow(){}
    //
    //Inference
    tensorflow::Tensor inputTensor;
    tensorflow::NamedTensorList inputTensorList;
    tensorflow::Tensor outputTensor;
    std::string outputTensorName_;

};


#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_ */
