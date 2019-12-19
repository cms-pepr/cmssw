/*
 * WindowBase.cpp
 *
 *  Created on: 26 Sep 2019
 *      Author: jkiesele
 */




#include "FWCore/Utilities/interface/Exception.h"
#include "../interface/WindowBase.h"



WindowBase::WindowBase(float centerEta, float centerPhi, float outerRegionDEta, float outerRegionDPhi,
        float innerRegionDEta, float innerRegionDPhi) :

        mode_(useLayerClusters),
        centerEta_(centerEta),centerPhi_(centerPhi),
        outerRegionDEta_(outerRegionDEta),outerRegionDPhi_(outerRegionDPhi),
        innerRegionDEta_(innerRegionDEta), innerRegionDPhi_(
                innerRegionDPhi) {

    //sanity checks FIXME: add more
    if (innerRegionDEta_ <= 0 || innerRegionDPhi_ <= 0) {
        throw cms::Exception("IncorrectWindowParameters")
                << "innerRegionDEta,innerRegionDPhi  must be > 0";
    }
    if (innerRegionDEta_ > outerRegionDEta_ || innerRegionDPhi_ > outerRegionDPhi_) {
        throw cms::Exception("IncorrectWindowParameters")
                << "innerRegionDEta,innerRegionDPhi  must be <= outerRegionDEta, outerRegionDPhi";
    }
    if(nTrackFeatures_ != nRechitFeatures_ || nTrackFeatures_ != nLayerClusterFeatures_){
        throw cms::Exception("IncorrectWindowParameters")
                        << "number of track, rechit and layer cluster features must be the same";
    }

}


WindowBase::~WindowBase() {
    clear();
}


void WindowBase::clear() {
    // this class does not own anything
    tracks_.clear();
    recHits.clear();
    layerClusters_.clear();
    simClusters_.clear();
}



//// private ////

const size_t WindowBase::nTrackFeatures_=12;
void WindowBase::fillTrackFeatures(float*& data, const TrackWithHGCalPos * t) const {
    *(data++) = t->obj->p();
    *(data++) = t->pos.eta();
    *(data++) = reco::deltaPhi(t->pos.phi(), getCenterPhi());
    *(data++) = t->pos.theta();
    *(data++) = t->pos.mag();
    *(data++) = t->pos.x();
    *(data++) = t->pos.y();
    *(data++) = t->pos.z();
    *(data++) = t->obj->charge();
    *(data++) = t->obj->chi2();
    *(data++) = -1.; //track ID bit
    *(data++) = 0.; //pad
}

const size_t WindowBase::nRechitFeatures_=12;
void WindowBase::fillRecHitFeatures(float*& data, const HGCRecHitWithPos * recHit) const {
    *(data++) = recHit->hit->energy();
    *(data++) = recHit->pos.eta();
    *(data++) = reco::deltaPhi(recHit->pos.phi(), getCenterPhi());
    *(data++) = recHit->pos.theta();
    *(data++) = recHit->pos.mag();
    *(data++) = recHit->pos.x();
    *(data++) = recHit->pos.y();
    *(data++) = recHit->pos.z();
    *(data++) = (float)recHit->hit->detid();
    *(data++) = recHit->hit->time();
    *(data++) = 0.; //rechit ID bit
    *(data++) = 0.; //pad
}


const size_t WindowBase::nLayerClusterFeatures_=12;
void WindowBase::fillLayerClusterFeatures(float*& data, const reco::CaloCluster * cl) const {
    *(data++) = cl->energy();
    *(data++) = cl->eta();
    *(data++) = reco::deltaPhi(cl->phi(), getCenterPhi());
    *(data++) = cl->position().theta();
    *(data++) = std::sqrt(cl->position().Mag2());
    *(data++) = cl->position().x();
    *(data++) = cl->position().y();
    *(data++) = cl->position().z();
    *(data++) = 0; //pad
    *(data++) = 0; //pad
    *(data++) = 1.; //layer cluster ID bit
    *(data++) = 0; //pad
}



std::vector<int> WindowBase::pdgToOneHot(int pdgid) const{
    //nIDClasses = 10;//ambig, +-e, +-mu, gamma, pi0, +-cpi, hadr ...
    std::vector<int> onehot(nIDClasses,0);

    int counter=0;
    if(pdgid == 0)
        onehot.at(counter++) = 1;
    else if(pdgid == 11)
        onehot.at(counter++) = 1;
    else if(pdgid == -11)
        onehot.at(counter++) = 1;
    else if(pdgid == 13)
        onehot.at(counter++) = 1;
    else if(pdgid == -13)
        onehot.at(counter++) = 1;
    else if(pdgid == 22)
        onehot.at(counter++) = 1;
    else if(pdgid == 210)
        onehot.at(counter++) = 1;
    else if(pdgid == 211)
        onehot.at(counter++) = 1;
    else if(pdgid == -211)
        onehot.at(counter++) = 1;
    else
        onehot.at(counter) = 1;

    return onehot;
}

int WindowBase::oneHotToPdg(const std::vector<int>& oh) const{
    //nIDClasses = 7;//ambig, e, mu, gamma, pi0, cpi, hadr ..
    if(oh.size() != nIDClasses)
        throw std::out_of_range("WindowBase::oneHotToPdg");

    if(oh.at(0)) return 0;
    if(oh.at(1)) return 11;
    if(oh.at(2)) return -11;
    if(oh.at(3)) return 13;
    if(oh.at(4)) return -13;
    if(oh.at(5)) return 22;
    if(oh.at(6)) return 210;
    if(oh.at(7)) return 211;
    if(oh.at(8)) return -211;
    else return 0; //FIXME needs to be consistent with PF, Marcel etc

}

// debug

void WindowBase::printDebug()const{
     DEBUGPRINT(centerPhi_);
     DEBUGPRINT(centerEta_);
     DEBUGPRINT(outerRegionDEta_);
     DEBUGPRINT(outerRegionDPhi_);
     DEBUGPRINT(innerRegionDEta_);
     DEBUGPRINT(innerRegionDPhi_);
     std::cout << "coverage phi " << centerPhi_-outerRegionDPhi_ << "| " << centerPhi_-innerRegionDPhi_ << "[  :" <<
             centerPhi_ << ":  ]" << centerPhi_+innerRegionDPhi_ << " |" << centerPhi_ + outerRegionDPhi_ << std::endl;
     std::cout << "coverage eta " << centerEta_-outerRegionDEta_ << "| " << centerEta_-innerRegionDEta_ << "[  :" <<
             centerEta_ << ":  ]" << centerEta_+innerRegionDEta_ << " |" << centerEta_+outerRegionDEta_ << std::endl;
}



/// static




