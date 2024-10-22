/*
 * HGCalTrackPropagator.h
 *
 *  Created on: 28 Aug 2019
 *      Author: jkiesele
 */

#ifndef SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALTRACKPROPAGATOR_H_
#define SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALTRACKPROPAGATOR_H_

#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "SimDataFormats/Track/interface/SimTrack.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "DataFormats/GeometrySurface/interface/BoundDisk.h"

#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"



template<class T>
class ObjectWithPos{
public:
    const T * obj;
    const GlobalPoint  pos;
    const GlobalVector momentum;//momentum at position
};




template<class T>
class HGCalObjectPropagator{
public:
    enum zpos{ negZ=0, posZ=1};
    HGCalObjectPropagator() {}
    HGCalObjectPropagator(edm::ConsumesCollector&& cc) :
        bFieldToken_(cc.esConsumes<edm::Transition::BeginRun>()),
        hgcEEToken_(cc.esConsumes<HGCalDDDConstants, IdealGeometryRecord, edm::Transition::BeginRun>(edm::ESInputTag{"", "HGCalEESensitive"})),
        propagatorToken_(cc.esConsumes<edm::Transition::BeginRun>(edm::ESInputTag("", "PropagatorWithMaterial"))) {}

    void setupRun(const edm::EventSetup& iSetup);

    ObjectWithPos<T> propagateObject(const T&, int charge=-200)const;
    ObjectWithPos<T> propagateVectors( math::XYZTLorentzVectorF& point,
             math::XYZTLorentzVectorF& momentum, int charge=-200, const T* obj = 0)const;

private:
    edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bFieldToken_;
    edm::ESGetToken<HGCalDDDConstants, IdealGeometryRecord> hgcEEToken_;
    edm::ESGetToken<Propagator, TrackingComponentsRecord> propagatorToken_;
    bool setup_;
    const MagneticField* bField_;
    const Propagator* propagator_;
    std::unique_ptr<GeomDet> frontFaces_[2];

    double frontz_,backz_;
};


typedef ObjectWithPos<reco::Track> TrackWithHGCalPos;
typedef HGCalObjectPropagator<reco::Track> HGCalTrackPropagator ;



/////////////////



template<class T>
void HGCalObjectPropagator<T>::setupRun(const edm::EventSetup& iSetup) {
    bField_ = &iSetup.getData(bFieldToken_);
    propagator_ = &iSetup.getData(propagatorToken_);
    const HGCalDDDConstants* hgdc = &iSetup.getData(hgcEEToken_);

    //This needs to be adapted if the boundary is ever moved
    frontz_ = hgdc->waferZ(1, true);
    backz_ = - frontz_;
    auto frontradii = hgdc->rangeR(frontz_, true);

    frontFaces_[posZ] = std::make_unique < GeomDet
            > (Disk::build(Disk::PositionType(0, 0, frontz_),
                    Disk::RotationType(),
                    SimpleDiskBounds(frontradii.first, frontradii.second,
                            frontz_ - 0.5, frontz_ + 0.5)).get());

    frontFaces_[negZ] = std::make_unique < GeomDet
            > (Disk::build(Disk::PositionType(0, 0, backz_),
                    Disk::RotationType(),
                    SimpleDiskBounds(frontradii.first, frontradii.second,
                            backz_ - 0.5, backz_ + 0.5)).get());

    setup_=true;
}

template<class T>
ObjectWithPos<T> HGCalObjectPropagator<T>::propagateVectors( math::XYZTLorentzVectorF& point,
         math::XYZTLorentzVectorF& momentum, int charge, const T* obj)const{

    typedef TrajectoryStateOnSurface TSOS;

    zpos trackz = posZ;
    if(momentum.z()<0) trackz = negZ;
    const double caloz = trackz == posZ ? frontz_ : backz_;


    const double c = 2.99792458e10; //cm/s
    double betazc = momentum.Beta() * momentum.z()/momentum.P() * c;



    double zdist = caloz - point.z();
    double timeprop = zdist/betazc;


    if(!charge){
        auto normmom = momentum / momentum.z();
        //figure out target
        normmom *= zdist;

        point = math::XYZTLorentzVectorF(normmom.x()+point.x(),
                normmom.y()+point.y(),
                normmom.z()+point.z(),
                timeprop+point.T());

    }
    else{
        GlobalPoint gpoint(point.x(),point.y(),point.z());
        GlobalVector gmomentum(momentum.x(),momentum.y(),momentum.z());

        if(fabs(point.z())>fabs(caloz))
            gmomentum *= -1;

        TSOS startingState( GlobalTrajectoryParameters(gpoint,
                gmomentum, charge, bField_));

        TSOS propState = propagator_->propagate( startingState, frontFaces_[trackz]->surface());

        if (propState.isValid()){
            auto proppoint = propState.globalPosition();
            auto propmomentum = propState.globalMomentum();


            point = math::XYZTLorentzVectorF(proppoint.x(),
                    proppoint.y(),
                    proppoint.z(),
                    timeprop+point.T());

            momentum = math::XYZTLorentzVectorF(propmomentum.x(),
                    propmomentum.y(),
                    propmomentum.z(),
                    momentum.T());

        }
    }
    /*
     *
    const GlobalPoint  pos;
    const GlobalVector momentum;//momentum at position
     */

    return ObjectWithPos<T>{obj, GlobalPoint(point.x(),point.y(),point.z()),
        GlobalVector(momentum.x(),momentum.y(),momentum.z())};

}
template<class T>
ObjectWithPos<T> HGCalObjectPropagator<T>::propagateObject(const T& o, int charge)const{

    math::XYZTLorentzVectorF point(o.vertex.x(), o.vertex().y(),o.vertex.z(),o.vertex().t());
    math::XYZTLorentzVectorF mom(o.p4().x(), o.p4().y(),o.p4().z(),o.p4().t());

    return propagateVectors(point, mom, charge, &o);
}

template<>
inline ObjectWithPos<reco::Track> HGCalObjectPropagator<reco::Track>::propagateObject(const reco::Track& t, int charge)const{
    if(!setup_)
        throw cms::Exception("HGCalObjectPropagator")
                        << "event setup not loaded";
    zpos trackz = posZ;
    if(t.pz()<0) trackz = negZ;


    FreeTrajectoryState fts = trajectoryStateTransform::outerFreeState(t, bField_);

    TrajectoryStateOnSurface tsos = propagator_->propagate(fts, frontFaces_[trackz]->surface());
    if (tsos.isValid())
        return ObjectWithPos<reco::Track>{&t, tsos.globalPosition(), tsos.globalMomentum()};
    return ObjectWithPos<reco::Track>{&t, GlobalPoint(t.vx(), t.vy(),t.vz()),
        GlobalVector(t.momentum().x(),t.momentum().y(),t.momentum().z())};

}




#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALTRACKPROPAGATOR_H_ */
