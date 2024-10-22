/*
 * InferenceWindow.h
 *
 *  Created on: 26 Sep 2019
 *      Author: jkiesele
 */

#ifndef SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_
#define SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_

#include "../interface/WindowBase.h"

class InferenceWindow: public WindowBase {
public:


    InferenceWindow(float centerEta, float centerPhi, float outerRegionDEta, float outerRegionDPhi,
            float innerRegionDEta, float innerRegionDPhi);

    ~InferenceWindow();



    void evaluate();

    void getOutput() const{}//needs output format etc.

    static std::vector<InferenceWindow> createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi);

private:

    InferenceWindow(){}
    //
    //Inference

};


#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_INFERENCEWINDOW_H_ */
