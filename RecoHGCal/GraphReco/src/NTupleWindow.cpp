/*
 * NTupleWindow.cpp
 *
 *  Created on: 26 Sep 2019
 *      Author: jkiesele
 */



#include "../interface/NTupleWindow.h"
#include "DataFormats/Math/interface/deltaR.h"

std::vector<std::vector<float>>* NTupleWindow::sp_hitFeatures_=0;
std::vector<float>* NTupleWindow::sp_recHitEnergy_;
std::vector<float>* NTupleWindow::sp_recHitEta_;
std::vector<float>* NTupleWindow::sp_recHitRelPhi_;
std::vector<float>* NTupleWindow::sp_recHitTheta_;
std::vector<float>* NTupleWindow::sp_recHitR_;
std::vector<float>* NTupleWindow::sp_recHitX_;
std::vector<float>* NTupleWindow::sp_recHitY_;
std::vector<float>* NTupleWindow::sp_recHitZ_;
std::vector<float>* NTupleWindow::sp_recHitDetID_;
std::vector<float>* NTupleWindow::sp_recHitTime_;
std::vector<float>* NTupleWindow::sp_recHitID_;
std::vector<float>* NTupleWindow::sp_recHitPad_;
//std::vector<float>* NTupleWindow::sp_trackFeatures_=0;

std::vector<std::vector<float> > * NTupleWindow::sp_truthHitFractions_=0;
std::vector<int>                 * NTupleWindow::sp_truthHitAssignementIdx_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedEnergies_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedX_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedY_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedZ_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedEta_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedPhi_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedR_=0;
std::vector<std::vector<int> >   * NTupleWindow::sp_truthHitAssignedPIDs_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedInner_=0;

std::vector<float>               * NTupleWindow::sp_truthHitAssignedDirX_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedDirY_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedDirZ_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedDirEta_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedDirPhi_=0;
std::vector<float>               * NTupleWindow::sp_truthHitAssignedDirR_=0;

std::vector<int>    * NTupleWindow::sp_truthSimclusterIdx_=0;
std::vector<std::vector<int> >   * NTupleWindow::sp_truthSimclusterPIDs_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterEnergies_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterX_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterY_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterZ_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterEta_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterPhi_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterR_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterInnerWindow_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterT_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterDirX_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterDirY_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterDirZ_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterDirEta_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterDirPhi_=0;
std::vector<float>  * NTupleWindow::sp_truthSimclusterDirR_=0;


float * NTupleWindow::sp_windowEta_=0;
float * NTupleWindow::sp_windowPhi_=0;

//static
std::vector<NTupleWindow> NTupleWindow::createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi){
    return WindowBase::createWindows<NTupleWindow>(nSegmentsPhi,nSegmentsEta,
            minEta,maxEta,frameWidthEta,frameWidthPhi);
}



void NTupleWindow::createTreeBranches(TTree* t){

   // NTupleWindow dummy;
   // dummy.assignTreePointers(); //so that the pointers are not null, maybe not needed? FIXME

    if (flattenRechitFeatures_) {
        t->Branch("recHitEnergy", &sp_recHitEnergy_);
        t->Branch("recHitEta", &sp_recHitEta_);
        t->Branch("recHitRelPhi", &sp_recHitRelPhi_);
        t->Branch("recHitTheta", &sp_recHitTheta_);
        t->Branch("recHitR", &sp_recHitR_);
        t->Branch("recHitX", &sp_recHitX_);
        t->Branch("recHitY", &sp_recHitY_);
        t->Branch("recHitZ", &sp_recHitZ_);
        t->Branch("recHitDetID", &sp_recHitDetID_);
        t->Branch("recHitTime", &sp_recHitTime_);
        t->Branch("recHitID", &sp_recHitID_);
        t->Branch("recHitPad", &sp_recHitPad_);
    }
    else
        t->Branch("recHitFeatures", &sp_hitFeatures_);
    //t->Branch("trackFeatures", &sp_trackFeatures_);

    t->Branch("truthHitFractions", &sp_truthHitFractions_);
    t->Branch("truthHitAssignementIdx", &sp_truthHitAssignementIdx_);
    t->Branch("truthHitAssignedEnergies", &sp_truthHitAssignedEnergies_);
    t->Branch("truthHitAssignedX", &sp_truthHitAssignedX_);
    t->Branch("truthHitAssignedY", &sp_truthHitAssignedY_);
    t->Branch("truthHitAssignedZ", &sp_truthHitAssignedZ_);
    t->Branch("truthHitAssignedEta", &sp_truthHitAssignedEta_);
    t->Branch("truthHitAssignedPhi", &sp_truthHitAssignedPhi_);
    t->Branch("truthHitAssignedR", &sp_truthHitAssignedR_);
    t->Branch("truthHitAssignedPIDs", &sp_truthHitAssignedPIDs_);
    t->Branch("truthHitAssignedInner", &sp_truthHitAssignedInner_);
    t->Branch("truthHitAssignedDirX", &sp_truthHitAssignedDirX_);
    t->Branch("truthHitAssignedDirY", &sp_truthHitAssignedDirY_);
    t->Branch("truthHitAssignedDirZ", &sp_truthHitAssignedDirZ_);
    t->Branch("truthHitAssignedDirEta", &sp_truthHitAssignedDirEta_);
    t->Branch("truthHitAssignedDirPhi", &sp_truthHitAssignedDirPhi_);
    t->Branch("truthHitAssignedDirR", &sp_truthHitAssignedDirR_);

    t->Branch("truthSimclusterIdx",&sp_truthSimclusterIdx_);
    t->Branch("truthSimclusterPIDs",&sp_truthSimclusterPIDs_);
    t->Branch("truthSimclusterEnergies",&sp_truthSimclusterEnergies_);
    t->Branch("truthSimclusterX",&sp_truthSimclusterX_);
    t->Branch("truthSimclusterY",&sp_truthSimclusterY_);
    t->Branch("truthSimclusterZ",&sp_truthSimclusterZ_);
    t->Branch("truthSimclusterInnerWindow",&sp_truthSimclusterInnerWindow_);
    t->Branch("truthSimclusterT",&sp_truthSimclusterT_);

    t->Branch("truthSimclusterDirX",&sp_truthSimclusterDirX_);
    t->Branch("truthSimclusterDirY",&sp_truthSimclusterDirY_);
    t->Branch("truthSimclusterDirZ",&sp_truthSimclusterDirZ_);
    t->Branch("truthSimclusterDirEta",&sp_truthSimclusterDirEta_);
    t->Branch("truthSimclusterDirPhi",&sp_truthSimclusterDirPhi_);
    t->Branch("truthSimclusterDirR",&sp_truthSimclusterDirR_);

    t->Branch("windowEta",sp_windowEta_);
    t->Branch("windowPhi",sp_windowPhi_);

}

NTupleWindow::NTupleWindow(float centerEta, float centerPhi,
        float outerRegionDEta, float outerRegionDPhi, float innerRegionDEta,
        float innerRegionDPhi) :
        WindowBase(centerEta, centerPhi, outerRegionDEta, outerRegionDPhi,
                innerRegionDEta, innerRegionDPhi) {

    windowEta_ = centerEta;
    windowPhi_ = centerPhi;
}


//static

// This is not a very clear/efficient way to do this, but it's the simplest way given
// the structure we already have
void NTupleWindow::flattenRechitFeatures() {
    for (size_t i = 0; i < hitFeatures_.size(); i++) {
        recHitEnergy_.push_back(hitFeatures_[i][kEnergy]);
        recHitEta_.push_back(hitFeatures_[i][kEta]);
        recHitRelPhi_.push_back(hitFeatures_[i][kRelPhi]);
        recHitTheta_.push_back(hitFeatures_[i][kTheta]);
        recHitR_.push_back(hitFeatures_[i][kR]);
        recHitX_.push_back(hitFeatures_[i][kx]);
        recHitY_.push_back(hitFeatures_[i][ky]);
        recHitZ_.push_back(hitFeatures_[i][kz]);
        recHitDetID_.push_back(hitFeatures_[i][kDetid]);
        recHitTime_.push_back(hitFeatures_[i][kTime]);
        recHitID_.push_back(hitFeatures_[i][kId]);
        recHitPad_.push_back(hitFeatures_[i][kPad]);
    }
}


void NTupleWindow::assignTreePointers()  {

    sp_hitFeatures_ = &hitFeatures_;
    //sp_trackFeatures_ = &hitFeatures_;
    if (flattenRechitFeatures_) {
        sp_recHitEnergy_  = &recHitEnergy_;
        sp_recHitEta_  = &recHitEta_;
        sp_recHitRelPhi_  = &recHitRelPhi_;
        sp_recHitTheta_  = &recHitTheta_;
        sp_recHitR_  = &recHitR_;
        sp_recHitX_  = &recHitX_;
        sp_recHitY_  = &recHitY_;
        sp_recHitZ_  = &recHitZ_;
        sp_recHitDetID_  = &recHitDetID_;
        sp_recHitTime_  = &recHitTime_;
        sp_recHitID_  = &recHitID_;
        sp_recHitPad_  = &recHitPad_;
    }

    sp_truthHitFractions_ = &truthHitFractions_;
    sp_truthHitAssignementIdx_ = &truthHitAssignementIdx_;
    sp_truthHitAssignedEnergies_ = &truthHitAssignedEnergies_;
    sp_truthHitAssignedX_ = &truthHitAssignedX_;
    sp_truthHitAssignedY_ = &truthHitAssignedY_;
    sp_truthHitAssignedZ_ = &truthHitAssignedZ_;
    sp_truthHitAssignedEta_ = &truthHitAssignedEta_;
    sp_truthHitAssignedPhi_ = &truthHitAssignedPhi_;
    sp_truthHitAssignedR_ = &truthHitAssignedR_;
    sp_truthHitAssignedPIDs_ = &truthHitAssignedPIDs_;
    sp_truthHitAssignedInner_ = &truthHitAssignedInner_;
    sp_truthHitAssignedDirX_ = &truthHitAssignedDirX_;
    sp_truthHitAssignedDirY_ = &truthHitAssignedDirY_;
    sp_truthHitAssignedDirZ_ = &truthHitAssignedDirZ_;
    sp_truthHitAssignedDirEta_ = &truthHitAssignedDirEta_;
    sp_truthHitAssignedDirPhi_ = &truthHitAssignedDirPhi_;
    sp_truthHitAssignedDirR_ = &truthHitAssignedDirR_;

    sp_truthSimclusterIdx_ = &truthSimclusterIdx_;
    sp_truthSimclusterPIDs_ = &truthSimclusterPIDs_;
    sp_truthSimclusterEnergies_ = &truthSimclusterEnergies_;
    sp_truthSimclusterX_ = &truthSimclusterX_;
    sp_truthSimclusterY_ = &truthSimclusterY_;
    sp_truthSimclusterZ_ = &truthSimclusterZ_;
    sp_truthSimclusterEta_ = &truthSimclusterEta_;
    sp_truthSimclusterPhi_ = &truthSimclusterPhi_;
    sp_truthSimclusterR_ = &truthSimclusterR_;
    sp_truthSimclusterInnerWindow_ = &truthSimclusterInnerWindow_;
    sp_truthSimclusterT_ = &truthSimclusterT_;

    sp_truthSimclusterDirX_=&truthSimclusterDirY_;
    sp_truthSimclusterDirY_=&truthSimclusterDirX_;
    sp_truthSimclusterDirZ_=&truthSimclusterDirZ_;
    sp_truthSimclusterDirEta_=&truthSimclusterDirEta_;
    sp_truthSimclusterDirPhi_=&truthSimclusterDirPhi_;
    sp_truthSimclusterDirR_=&truthSimclusterDirR_;

    sp_windowEta_ = &windowEta_;
    sp_windowPhi_ = &windowPhi_;

}



void NTupleWindow::clear(){
    WindowBase::clear(); //clears rechits etc

    detIDHitAsso_.clear();

    hitFeatures_.clear();
    recHitEnergy_.clear();
    recHitEta_.clear();
    recHitRelPhi_.clear();
    recHitTheta_.clear();
    recHitR_.clear();
    recHitX_.clear();
    recHitY_.clear();
    recHitZ_.clear();
    recHitDetID_.clear();
    recHitTime_.clear();
    recHitID_.clear();
    recHitPad_.clear();

    truthHitFractions_.clear();
    truthHitAssignementIdx_.clear();
    truthHitAssignedEnergies_.clear();
    truthHitAssignedX_.clear();
    truthHitAssignedY_.clear();
    truthHitAssignedZ_.clear();
    truthHitAssignedEta_.clear();
    truthHitAssignedPhi_.clear();
    truthHitAssignedR_.clear();
    truthHitAssignedPIDs_.clear();
    truthHitAssignedInner_.clear();
    truthHitAssignedDirX_.clear();
    truthHitAssignedDirY_.clear();
    truthHitAssignedDirZ_.clear();
    truthHitAssignedDirEta_.clear();
    truthHitAssignedDirPhi_.clear();
    truthHitAssignedDirR_.clear();

    truthSimclusterIdx_.clear();
    truthSimclusterPIDs_.clear();
    truthSimclusterEnergies_.clear();
    truthSimclusterX_.clear();
    truthSimclusterY_.clear();
    truthSimclusterZ_.clear();
    truthSimclusterEta_.clear();
    truthSimclusterPhi_.clear();
    truthSimclusterR_.clear();
    truthSimclusterInnerWindow_.clear();
    truthSimclusterT_.clear();

    truthSimclusterDirY_.clear();
    truthSimclusterDirX_.clear();
    truthSimclusterDirZ_.clear();
    truthSimclusterDirEta_.clear();
    truthSimclusterDirPhi_.clear();
    truthSimclusterDirR_.clear();


}

void NTupleWindow::fillFeatureArrays(){
    //NO CUTS HERE!

    hitFeatures_.clear();
    if(getMode() == useRechits){
        for(const auto& rh:recHits){
            std::vector<float> feats(nRechitFeatures_);
            auto data = &feats.at(0);
            fillRecHitFeatures(data,rh);
            hitFeatures_.push_back(feats);
        }
    }
    else{
        for(const auto& lc: layerClusters_){
            std::vector<float> feats(nLayerClusterFeatures_);
            auto data = &feats.at(0);
            fillLayerClusterFeatures(data,lc);
            hitFeatures_.push_back(feats);
        }
    }

    return;
    //add tracks LAST!
    for(const auto& tr:tracks_){
        std::vector<float> feats(nTrackFeatures_);
        auto data = &feats.at(0);
        fillTrackFeatures(data,tr);
        hitFeatures_.push_back(feats);
    }

}

void NTupleWindow::fillTruthArrays(){

    createDetIDHitAssociation();
    calculateSimclusterFeatures();
    calculateTruthFractions();
    fillTruthAssignment();

    DEBUGPRINT(getCenterEta());
    DEBUGPRINT(getCenterPhi());
    DEBUGPRINT(truthHitFractions_.size());
    DEBUGPRINT(hitFeatures_.size());
    DEBUGPRINT(truthSimclusterIdx_.size());
}

void NTupleWindow::createDetIDHitAssociation(){
    detIDHitAsso_.clear();

    if(getMode() == useRechits){
        for(size_t i=0;i<recHits.size();i++){
            detIDHitAsso_[recHits.at(i)->hit->detid()]={i,1.};
        }
    }
    else{
        for(size_t i=0;i<layerClusters_.size();i++){
            for(const auto& haf: layerClusters_.at(i)->hitsAndFractions()){
                detIDHitAsso_[haf.first] = {i, haf.second};
            }
        }
    }
}

void NTupleWindow::calculateSimclusterFeatures(){

    truthSimclusterIdx_.clear();
    truthSimclusterPIDs_.clear();
    truthSimclusterEnergies_.clear();
    truthSimclusterX_.clear();
    truthSimclusterY_.clear();
    truthSimclusterZ_.clear();
    truthSimclusterEta_.clear();
    truthSimclusterPhi_.clear();
    truthSimclusterR_.clear();
    truthSimclusterInnerWindow_.clear();
    truthSimclusterT_.clear();
    truthSimclusterDirX_.clear();
    truthSimclusterDirY_.clear();
    truthSimclusterDirZ_.clear();
    truthSimclusterDirEta_.clear();
    truthSimclusterDirPhi_.clear();
    truthSimclusterDirR_.clear();



    for(size_t i=0;i<simClusters_.size();i++){
        truthSimclusterIdx_.push_back(i);
        truthSimclusterPIDs_.push_back(pdgToOneHot(simClusters_.at(i)->pdgId()));
        const math::XYZTLorentzVectorF& scimpactpoint = simClusters_.at(i)->impactPoint();
        truthSimclusterX_.push_back(scimpactpoint.X());
        truthSimclusterY_.push_back(scimpactpoint.Y());
        truthSimclusterZ_.push_back(scimpactpoint.Z());
        truthSimclusterEta_.push_back(scimpactpoint.Eta());
        truthSimclusterPhi_.push_back(reco::deltaPhi(scimpactpoint.Phi(),getCenterPhi()));
        truthSimclusterR_.push_back(scimpactpoint.R());

        truthSimclusterInnerWindow_.push_back(simClustersInnerWindow_.at(i) ? 1 : 0);

        truthSimclusterT_.push_back(simClusters_.at(i)->impactPoint().T());
        const math::XYZTLorentzVectorF& scimpactmom = simClusters_.at(i)->impactMomentum();
        truthSimclusterEnergies_.push_back(simClusters_.at(i)->p4().E());
        truthSimclusterDirX_.push_back(scimpactmom.X());
        truthSimclusterDirY_.push_back(scimpactmom.Y());
        truthSimclusterDirZ_.push_back(scimpactmom.Z());
        truthSimclusterDirEta_.push_back(scimpactmom.Eta());
        truthSimclusterDirPhi_.push_back(scimpactmom.Phi());
        truthSimclusterDirR_.push_back(scimpactmom.R());

    }
}

//needs function to match truth tracks

void NTupleWindow::calculateTruthFractions(){

    truthHitFractions_.clear();
    truthHitFractions_.resize(hitFeatures_.size(),
            std::vector<float>(simClusters_.size(), 0)); //includes tracks

    for (size_t i_sc = 0; i_sc < simClusters_.size(); i_sc++) {
        const auto& hitsandfracs = simClusters_.at(i_sc)->hits_and_fractions();
        for(const auto& haf: hitsandfracs){
            auto pos = detIDHitAsso_.find(haf.first);
            if(pos == detIDHitAsso_.end()) //edges or not included in layer clusters
                continue;
            size_t idx = pos->second.first;
            float totalfrac = pos->second.second * haf.second;
            truthHitFractions_.at(idx).at(i_sc) += totalfrac; //can be more than 1-1 for layer clusters
        }
    }

    //associate the tracks here, such that they look like hits, simple matching
    size_t trackStartIterator = recHits.size();
    if(getMode() == useLayerClusters)
        trackStartIterator = layerClusters_.size();

    ////match, will be improved by direct truth matching in new simclusters on longer term
    ////assumption: for every track there is charged simcluster
    ///*
    // *
    // * this is just a temporary solution until a proper simcluster-simtrack integration exists
    // *
    // */
    //std::vector<size_t> usedSimclusters;

    return ;

    float debug_ntrackwithnoSC=0;

    for(size_t i_t=0;i_t<tracks_.size();i_t++){

        const double momentumscaler = 0.0001;
        double minDistance=0.1 + 0.1;

        size_t matchedSCIdx=simClusters_.size();
        double distance = 0;
        for(size_t i_sc=0;i_sc<simClusters_.size();i_sc++){

            double scEnergy = simClusters_.at(i_sc)->impactMomentum().E();
            double trackMomentum = tracks_.at(i_t)->obj->p();
            distance = reco::deltaR(simClusters_.at(i_sc)->impactPoint().Eta(),
                    simClusters_.at(i_sc)->impactPoint().Phi(),
                    (float)tracks_.at(i_t)->pos.eta(),
                    (float)tracks_.at(i_t)->pos.phi()) +
                            momentumscaler*std::abs(scEnergy - trackMomentum)/(scEnergy);

            if(distance<minDistance){
                matchedSCIdx=i_sc;
                minDistance=distance;
            }
        }
        if(matchedSCIdx<simClusters_.size()){
            truthHitFractions_.at(i_t+trackStartIterator).at(matchedSCIdx) = 1.;
        }
        else{
            debug_ntrackwithnoSC++;
            DEBUGPRINT(distance);
            DEBUGPRINT(tracks_.at(i_t)->obj->p());
            DEBUGPRINT(tracks_.at(i_t)->obj->eta());
        }
    }
    //DEBUGPRINT(debug_ntrackwithnoSC);
    //DEBUGPRINT(debug_ntrackwithnoSC/(float)tracks_.size());
}


void NTupleWindow::fillTruthAssignment(){

    truthHitAssignementIdx_.resize(truthHitFractions_.size());
    truthHitAssignedEnergies_.resize(truthHitFractions_.size());
    truthHitAssignedX_.resize(truthHitFractions_.size());
    truthHitAssignedY_.resize(truthHitFractions_.size());
    truthHitAssignedZ_.resize(truthHitFractions_.size());
    truthHitAssignedEta_.resize(truthHitFractions_.size());
    truthHitAssignedPhi_.resize(truthHitFractions_.size());
    truthHitAssignedR_.resize(truthHitFractions_.size());
    truthHitAssignedPIDs_.resize(truthHitFractions_.size());
    truthHitAssignedInner_.resize(truthHitFractions_.size());
    truthHitAssignedDirX_.resize(truthHitFractions_.size());
    truthHitAssignedDirY_.resize(truthHitFractions_.size());
    truthHitAssignedDirZ_.resize(truthHitFractions_.size());
    truthHitAssignedDirEta_.resize(truthHitFractions_.size());
    truthHitAssignedDirPhi_.resize(truthHitFractions_.size());
    truthHitAssignedDirR_.resize(truthHitFractions_.size());

    bool nosim = simClusters_.size() < 1;

    for (size_t i_hit = 0; i_hit < truthHitFractions_.size(); i_hit++) {

        bool allzero = std::all_of(truthHitFractions_.at(i_hit).begin(),
                truthHitFractions_.at(i_hit).end(), [](float i) {return i==0;});

        if(allzero || nosim){
            truthHitAssignementIdx_.at(i_hit) = -1;
            continue;
        }
        size_t maxfrac_idx = std::max_element(
                truthHitFractions_.at(i_hit).begin(),
                truthHitFractions_.at(i_hit).end())
        - truthHitFractions_.at(i_hit).begin();

        truthHitAssignementIdx_.at(i_hit) = maxfrac_idx;
        truthHitAssignedEnergies_.at(i_hit) = truthSimclusterEnergies_.at(
                maxfrac_idx);
        truthHitAssignedX_.at(i_hit) = truthSimclusterX_.at(maxfrac_idx);
        truthHitAssignedY_.at(i_hit) = truthSimclusterY_.at(maxfrac_idx);
        truthHitAssignedZ_.at(i_hit) = truthSimclusterZ_.at(maxfrac_idx);
        truthHitAssignedEta_.at(i_hit) = truthSimclusterEta_.at(maxfrac_idx);
        truthHitAssignedPhi_.at(i_hit) = truthSimclusterPhi_.at(maxfrac_idx);
        truthHitAssignedR_.at(i_hit) = truthSimclusterR_.at(maxfrac_idx);
        truthHitAssignedPIDs_.at(i_hit) = truthSimclusterPIDs_.at(maxfrac_idx);
        truthHitAssignedDirX_.at(i_hit) = truthSimclusterDirX_.at(maxfrac_idx);
        truthHitAssignedDirY_.at(i_hit) = truthSimclusterDirY_.at(maxfrac_idx);
        truthHitAssignedDirZ_.at(i_hit) = truthSimclusterDirZ_.at(maxfrac_idx);
        truthHitAssignedDirEta_.at(i_hit) = truthSimclusterDirEta_.at(maxfrac_idx);
        truthHitAssignedDirPhi_.at(i_hit) = truthSimclusterDirPhi_.at(maxfrac_idx);
        truthHitAssignedDirR_.at(i_hit) = truthSimclusterDirR_.at(maxfrac_idx);
        truthHitAssignedInner_.at(i_hit) = truthSimclusterInnerWindow_.at(maxfrac_idx);

    }
}

