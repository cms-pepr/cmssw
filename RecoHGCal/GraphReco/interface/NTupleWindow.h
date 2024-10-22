/*
 * NTupleWindow.h
 *
 *  Created on: 26 Sep 2019
 *      Author: jkiesele
 */

#ifndef SRC_RECOHGCAL_GRAPHRECO_INTERFACE_NTUPLEWINDOW_H_
#define SRC_RECOHGCAL_GRAPHRECO_INTERFACE_NTUPLEWINDOW_H_

#include "../interface/WindowBase.h"
#include <algorithm>
#include <unordered_map>

class NTupleWindow: public WindowBase {
public:

    NTupleWindow(float centerEta, float centerPhi, float outerRegionDEta, float outerRegionDPhi,
            float innerRegionDEta, float innerRegionDPhi);

    static void createTreeBranches(TTree* t);

    //0 associate all rechits etc

    void fillFeatureArrays();

    //1
    void fillTruthArrays();
    //2
    void fillTiclAssignment();
    void assignTreePointers() ;
    void flattenRechitFeatures();

    //3: tree->Fill();

    //4
    void clear();

    static std::vector<NTupleWindow> createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi);



private:
    NTupleWindow(){}

    void createDetIDHitAssociation();
    void calculateSimclusterFeatures();
    void calculateTruthFractions();
    void fillTruthAssignment();
    std::vector<bool> cleanSimclusters();


    //temporary for (layer cluster) fraction calculation. detID to hit index and fraction
    std::unordered_map<DetId, std::pair<size_t, float>> detIDHitAsso_;


    //can be layer clusters or rechits according to mode
    std::vector<float> recHitEnergy_;
    std::vector<float> recHitEta_;
    std::vector<float> recHitPhi_;
    std::vector<float> recHitTheta_;
    std::vector<float> recHitR_;
    std::vector<float> recHitX_;
    std::vector<float> recHitY_;
    std::vector<float> recHitZ_;
    std::vector<float> recHitDetID_;
    std::vector<float> recHitTime_;
    std::vector<float> recHitID_;
    std::vector<float> recHitPad_;

    std::vector<std::vector<float> >  truthHitFractions_;


    //hit (rechit or layer cluster) assigned to exactly one simcluster with index
    std::vector<int>                 truthHitAssignementIdx_;
    //truth energy of assigned simcluster
    std::vector<float>               truthHitAssignedEnergies_;
    //truth eta of assigned simcluster
    std::vector<float>               truthHitAssignedX_;
    std::vector<float>               truthHitAssignedY_;
    std::vector<float>               truthHitAssignedZ_;
    std::vector<float>               truthHitAssignedEta_;
    std::vector<float>               truthHitAssignedPhi_;
    std::vector<float>               truthHitAssignedT_;
    //truth energy of assigned simcluster
    std::vector<std::vector<int> >   truthHitAssignedPIDs_;
    std::vector<float>               truthHitAssignedInner_;
    std::vector<float>               truthHitAssignedDirX_;
    std::vector<float>               truthHitAssignedDirY_;
    std::vector<float>               truthHitAssignedDirZ_;
    std::vector<float>               truthHitAssignedDirEta_;
    std::vector<float>               truthHitAssignedDepEnergies_;
    std::vector<float>               truthHitAssignedDirR_;

    std::vector<int>     truthSimclusterIdx_;
    std::vector<std::vector<int> >    truthSimclusterPIDs_;
    std::vector<float>   truthSimclusterEnergies_;
    std::vector<float>   truthSimclusterX_;
    std::vector<float>   truthSimclusterY_;
    std::vector<float>   truthSimclusterZ_;
    std::vector<float>   truthSimclusterEta_;
    std::vector<float>   truthSimclusterPhi_;
    std::vector<float>   truthSimclusterR_;
    std::vector<float>   truthSimclusterT_;
    std::vector<float>   truthSimclusterDirX_;
    std::vector<float>   truthSimclusterDirY_;
    std::vector<float>   truthSimclusterDirZ_;
    std::vector<float>   truthSimclusterDirEta_;
    std::vector<float>   truthSimclusterDepEnergies_;
    std::vector<float>   truthSimclusterDirR_;
    std::vector<float>   truthSimclusterInnerWindow_;

    std::vector<int>  ticlHitAssignementIdx_;
    std::vector<float> ticlHitAssignedEnergies_;

    //some globals mostly for plotting

    float windowEta_, windowPhi_;

    static const bool flattenRechitFeatures_ = true;
    /*
     *
 * recHitEnergy,
   recHitEta   ,
   recHitID, #indicator if it is track or not
   recHitTheta ,
   recHitR   ,
   recHitX     ,
   recHitY     ,
   recHitZ     ,
   recHitTime
 */


    enum rhLables {kEnergy=0, kEta=1, kId=2, kTheta=3, kR=4, kx=5, ky=6, kz=7,  kTime=8};
    //static pointers to create branches and fill tree
    static std::vector<std::vector<float>> * sp_hitFeatures_;
    static std::vector<float> * sp_recHitEnergy_;
    static std::vector<float> * sp_recHitEta_;
    static std::vector<float> * sp_recHitPhi_;
    static std::vector<float> * sp_recHitTheta_;
    static std::vector<float> * sp_recHitR_;
    static std::vector<float> * sp_recHitX_;
    static std::vector<float> * sp_recHitY_;
    static std::vector<float> * sp_recHitZ_;
    static std::vector<float> * sp_recHitDetID_;
    static std::vector<float> * sp_recHitTime_;
    static std::vector<float> * sp_recHitID_;
    static std::vector<float> * sp_recHitPad_;
    //static std::vector<float> * sp_trackFeatures_;

    static std::vector<std::vector<float> > * sp_truthHitFractions_;
    static std::vector<int>                 * sp_truthHitAssignementIdx_;
    static std::vector<float>               * sp_truthHitAssignedEnergies_;
    static std::vector<float>               * sp_truthHitAssignedX_;
    static std::vector<float>               * sp_truthHitAssignedY_;
    static std::vector<float>               * sp_truthHitAssignedZ_;
    static std::vector<float>               * sp_truthHitAssignedEta_;
    static std::vector<float>               * sp_truthHitAssignedPhi_;
    static std::vector<float>               * sp_truthHitAssignedT_;
    static std::vector<std::vector<int> >   * sp_truthHitAssignedPIDs_;
    static std::vector<float>               * sp_truthHitAssignedInner_;
    static std::vector<float>               * sp_truthHitAssignedDirX_;
    static std::vector<float>               * sp_truthHitAssignedDirY_;
    static std::vector<float>               * sp_truthHitAssignedDirZ_;
    static std::vector<float>               * sp_truthHitAssignedDirEta_;
    static std::vector<float>               * sp_truthHitAssignedDepEnergies_;
    static std::vector<float>               * sp_truthHitAssignedDirR_;

    static std::vector<int>    * sp_truthSimclusterIdx_;
    static std::vector<std::vector<int> >   * sp_truthSimclusterPIDs_;
    static std::vector<float>  * sp_truthSimclusterEnergies_;
    static std::vector<float>  * sp_truthSimclusterX_;
    static std::vector<float>  * sp_truthSimclusterY_;
    static std::vector<float>  * sp_truthSimclusterZ_;
    static std::vector<float>  * sp_truthSimclusterEta_;
    static std::vector<float>  * sp_truthSimclusterPhi_;
    static std::vector<float>  * sp_truthSimclusterR_;
    static std::vector<float>  * sp_truthSimclusterT_;
    static std::vector<float>  * sp_truthSimclusterInnerWindow_;

    static std::vector<float>  * sp_truthSimclusterDirX_;
    static std::vector<float>  * sp_truthSimclusterDirY_;
    static std::vector<float>  * sp_truthSimclusterDirZ_;

    static std::vector<float>  * sp_truthSimclusterDirEta_;
    static std::vector<float>  * sp_truthSimclusterDepEnergies_;
    static std::vector<float>  * sp_truthSimclusterDirR_;


    static std::vector<int>   * sp_ticlHitAssignementIdx_;
    static std::vector<float> * sp_ticlHitAssignedEnergies_;

    static float * sp_windowEta_;
    static float * sp_windowPhi_;

};

//helper

template<class T>
std::vector<T> keepIndices(const std::vector<T> & v, const std::vector<bool> indices){
    std::vector<T> out;
    for(size_t ii=0;ii<indices.size();ii++){
        if(indices.at(ii))
           out.push_back(v.at(ii));
    }
    return out;
}



#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_NTUPLEWINDOW_H_ */
