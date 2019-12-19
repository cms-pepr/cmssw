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
    HGCalObjectPropagator():setup_(false){}
    HGCalObjectPropagator(const edm::EventSetup &es); //sets up geometry etc.

    void getEventSetup(const edm::EventSetup &es);

    ObjectWithPos<T> propagateObject(const T&, int charge=-200)const;

private:
    bool setup_;
    edm::ESHandle<MagneticField> bField_;
    edm::ESHandle<Propagator> propagator_;
    std::unique_ptr<GeomDet> frontFaces_[2];

    double frontz_,backz_;
};


typedef ObjectWithPos<reco::Track> TrackWithHGCalPos;
typedef HGCalObjectPropagator<reco::Track> HGCalTrackPropagator ;



/////////////////



template<class T>
HGCalObjectPropagator<T>::HGCalObjectPropagator(const edm::EventSetup &es){
    getEventSetup(es);
}

template<class T>
void HGCalObjectPropagator<T>::getEventSetup(const edm::EventSetup &es){
    //get the propagator
    es.get<IdealMagneticFieldRecord>().get(bField_);
    es.get<TrackingComponentsRecord>().get("PropagatorWithMaterial", propagator_);


    //create the hgcal inner surface for both z
    edm::ESHandle<HGCalDDDConstants> hdc;
    es.get<IdealGeometryRecord>().get("HGCalEESensitive", hdc);

    frontz_ = hdc.product()->waferZ(1, true);
    backz_ = - frontz_;
    auto frontradii = hdc.product()->rangeR(frontz_, true);

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
ObjectWithPos<T> HGCalObjectPropagator<T>::propagateObject(const T& part, int charge)const{
    if(!setup_)
        throw cms::Exception("HGCalTrackPropagator")
                        << "event setup not loaded";
    zpos trackz = posZ;
    if(part.eta()<0) trackz = negZ;

    if(charge<-100)
        charge = part.charge();

    if(!charge){ //no bending

        auto normmom = part.momentum()/ std::sqrt(part.momentum().mag2());
        double zdist = frontz_ - part.vertex().z();
        if(trackz == negZ)
            zdist = backz_ - part.vertex().z();

        double scale = (zdist+part.vertex().z())/part.vertex().z();
        if(!part.vertex().z())
            scale = trackz == negZ ? backz_ : frontz_;

        normmom*=scale;

        return ObjectWithPos<T>{&part,
            GlobalPoint(normmom.x()+part.vertex().x(),
                    normmom.y()+part.vertex().y(),
                    normmom.z()+part.vertex().z()),
                GlobalVector(part.momentum().X(),part.momentum().Y(),part.momentum().Z())};
    }

    reco::TrackBase::Point refpoint(part.vertex().Coordinates())  ;
    reco::TrackBase::Vector momentum(part.momentum());

    auto trackdummy = reco::Track(0,1,refpoint,momentum,charge,reco::TrackBase::CovarianceMatrix());

    FreeTrajectoryState fts = trajectoryStateTransform::outerFreeState(trackdummy, bField_.product());

    TrajectoryStateOnSurface tsos = (*propagator_).propagate(fts, frontFaces_[trackz]->surface());
    if (tsos.isValid())
        return ObjectWithPos<T>{&part, tsos.globalPosition(), tsos.globalMomentum()};
    return ObjectWithPos<T>{&part,GlobalPoint(refpoint.x(), refpoint.y(),refpoint.z()),
        GlobalVector(momentum.X(),momentum.Y(),momentum.Z())};



}

template<>
inline ObjectWithPos<reco::Track> HGCalObjectPropagator<reco::Track>::propagateObject(const reco::Track& t, int charge)const{
    if(!setup_)
        throw cms::Exception("HGCalObjectPropagator")
                        << "event setup not loaded";
    zpos trackz = posZ;
    if(t.eta()<0) trackz = negZ;

    FreeTrajectoryState fts = trajectoryStateTransform::outerFreeState(t, bField_.product());

    TrajectoryStateOnSurface tsos = (*propagator_).propagate(fts, frontFaces_[trackz]->surface());
    if (tsos.isValid())
        return ObjectWithPos<reco::Track>{&t, tsos.globalPosition(), tsos.globalMomentum()};
    return ObjectWithPos<reco::Track>{&t, GlobalPoint(t.vx(), t.vy(),t.vz()),
        GlobalVector(t.momentum().x(),t.momentum().y(),t.momentum().z())};

}




#endif /* SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALTRACKPROPAGATOR_H_ */
