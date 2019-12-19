/*
 * Simple window structure with some convenience methods to determine if a RecHit matches the
 * windows eta and phi range, and to bookkeep input and output tensors for inference.
 *
 * Note 1: startPhi and startEta are inclusive, endPhi and endEta are exclusive (as in histograms)
 * Note 2: currently only positive eta ranges are supported for simplicity
 *
 * Author: Marcel Rieger <marcel.rieger@cern.ch>
 */

#ifndef SRC_RECOHGCAL_GRAPHRECO_INTERFACE_WINDOWBASE_H_
#define SRC_RECOHGCAL_GRAPHRECO_INTERFACE_WINDOWBASE_H_

#include <string>
#include "TTree.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "DataFormats/CaloRecHit/interface/CaloClusterFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include <vector>

#include "HGCalTrackPropagator.h"

#define DEBUGPRINT(x) {std::cout << #x << ": " << x << std::endl;}

struct HGCRecHitWithPos{
    HGCRecHit * hit;
    GlobalPoint  pos;
};


class WindowBase {
public:

    enum mode {
        useRechits, useLayerClusters
    };


    WindowBase(float centerEta, float centerPhi, float outerRegionDEta, float outerRegionDPhi,
            float innerRegionDEta, float innerRegionDPhi);

    virtual ~WindowBase() ;


    void setMode(mode m){
        mode_=m;
    }
    mode getMode()const{
        return mode_;
    }

    inline bool accept(float phi, float eta) const {
        return fabs(reco::deltaPhi(phi, centerPhi_)) < outerRegionDPhi_
        && fabs(eta - centerEta_) < outerRegionDEta_;
    }

    bool maybeAddTrack(const TrackWithHGCalPos& t) {
        //potential cuts here!
        if (accept((float)t.pos.phi(), (float)t.pos.eta())
                && t.obj->pt()>1) {
            tracks_.push_back(&t);
            return true;
        }
        return false;
    }

    inline bool maybeAddRecHit(const HGCRecHitWithPos& recHit) {
        //potential cuts here!
        if (accept((float) recHit.pos.phi(), (float) recHit.pos.eta())
                && recHit.hit->energy() > 0.01) {
            recHits.push_back(&recHit);
            return true;
        }
        return false;
    }

    inline bool maybeAddLayerCluster(
            const reco::CaloCluster & layerCluster) {
        //potential cuts here!
        if (accept(layerCluster.phi(), layerCluster.eta())) {
            layerClusters_.push_back(&layerCluster);
            return true;
        }
        return false;
    }

    inline bool maybeAddSimCluster(const SimCluster& sc){
        //potential cuts here!
        if (accept(sc.impactPoint().phi(),sc.impactPoint().eta())){
            simClusters_.push_back(&sc);
            simClustersInnerWindow_.push_back(isInner(sc.impactPoint().eta(),
                    sc.impactPoint().phi()));

            return true;
        }
        return false;
    }

    inline bool isInner(const float& eta, const float& phi) {
        return fabs(reco::deltaPhi(phi, centerPhi_)) < innerRegionDPhi_
                && fabs(eta - centerEta_) < innerRegionDEta_;
    }



    void clear();

    virtual void fillFeatureArrays()=0;


    //debug functions

    void printDebug()const;

    const float& getCenterEta() const {
        return centerEta_;
    }

    const float& getCenterPhi() const {
        return centerPhi_;
    }

    const float& getOuterRegionDEta() const {
        return outerRegionDEta_;
    }

    const float& getOuterRegionDPhi() const {
        return outerRegionDPhi_;
    }

    const float& getInnerRegionDEta() const {
        return innerRegionDEta_;
    }

    const float& getInnerRegionDPhi() const {
        return innerRegionDPhi_;
    }

    static constexpr size_t nIDClasses = 10;//ambig, +-e, +-mu, gamma, pi0, +-cpi, hadr ...

    template<class T>
    static std::vector<T> createWindows(size_t nSegmentsPhi,
            size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
            double frameWidthPhi) ;

private:

    mode mode_;

    float centerEta_;
    float centerPhi_;

    float outerRegionDEta_;
    float outerRegionDPhi_;

    float innerRegionDEta_;
    float innerRegionDPhi_;


protected:

    WindowBase(){}

    std::vector<const TrackWithHGCalPos *> tracks_;
    std::vector<const HGCRecHitWithPos *> recHits;
    std::vector<const reco::CaloCluster * > layerClusters_;
    std::vector<const SimCluster*> simClusters_;
    std::vector<bool> simClustersInnerWindow_;

    //for one track
    void fillTrackFeatures(float*& data, const TrackWithHGCalPos *) const;
    //for one rechit
    void fillRecHitFeatures(float*& data, const HGCRecHitWithPos * ) const;
    //for one layer cluster
    void fillLayerClusterFeatures(float*& data, const reco::CaloCluster * ) const;


    std::vector<int> pdgToOneHot(int pdgid) const;
    int oneHotToPdg(const std::vector<int>& ) const;

    static const size_t nTrackFeatures_;
    static const size_t nRechitFeatures_;
    static const size_t nLayerClusterFeatures_;




};


template<class T>
std::vector<T> WindowBase::createWindows(size_t nSegmentsPhi,
        size_t nSegmentsEta, double minEta, double maxEta, double frameWidthEta,
        double frameWidthPhi) {

    if (minEta <= 0 || maxEta <= 0 || minEta >= maxEta) {
        throw cms::Exception("IncorrectWindowCreationParameters")
        << "minEta, maxEta must be > 0 and maxEta > minEta (negative eta will be created automatically)";
    }
    if (frameWidthEta < 0 || frameWidthPhi < 0 ) {
        throw cms::Exception("IncorrectWindowCreationParameters")
        << "frameWidthEta, frameWidthPhi must be >= 0";
    }

    const float epsilon=1e-5;

    std::vector<T> windows;
    float phiStep = 2. * M_PI / (float) nSegmentsPhi;
    float totalDPhi = (phiStep + 2. * frameWidthPhi)/2.;
    float etaStep = (maxEta - minEta) / (float) nSegmentsEta;
    float totalDEta = (etaStep + 2. * frameWidthEta)/2.;

    for (float phi_i = -M_PI; phi_i + epsilon < M_PI; phi_i += phiStep) {
        float phiCenter = phi_i + phiStep / 2.;
        for (float eta_j = minEta; eta_j + epsilon < maxEta; eta_j += etaStep) {
            float etaCenter = eta_j + etaStep / 2.;
            T w(etaCenter, phiCenter, totalDEta, totalDPhi,
                    (float)(etaStep / 2.), (float)(phiStep / 2.));
            //w.printDebug();
            windows.push_back(w);

        }



        float minEtaNeg = -maxEta;
        float maxEtaNeg = -minEta;
        for (float eta_j = minEtaNeg; eta_j + epsilon < maxEtaNeg; eta_j +=
                etaStep) {

            float etaCenter = eta_j + etaStep / 2.;

            T w(etaCenter, phiCenter, totalDEta, totalDPhi,
                    (float)(etaStep / 2.), (float)(phiStep / 2.));
            //w.printDebug();
            windows.push_back(w);
        }

    }
    return windows;
}





#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_WINDOWBASE_H_ */
