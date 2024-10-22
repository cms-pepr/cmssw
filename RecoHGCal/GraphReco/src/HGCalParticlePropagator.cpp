
#include "../interface/HGCalParticlePropagator.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FastSimulation/ParticlePropagator/interface/ParticlePropagator.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "MagneticField/Engine/interface/MagneticField.h"

HGCalParticlePropagator::~HGCalParticlePropagator(){
    if(rkprop_)
        delete rkprop_;

    std::cout << "n propagated "<< n_propagated_ << " n failed "<< n_failed_ << std::endl;
}

void HGCalParticlePropagator::setEventSetup(const edm::EventSetup &es){

    es.get<IdealMagneticFieldRecord>().get(bField_);

    if(rkprop_)
        delete rkprop_;
    rkprop_ = new defaultRKPropagator::Product ( bField_.product(), alongMomentum, 5.e-5);

    //edm::ESHandle<MagneticFieldMap> fieldMap;
    //es.get<MagneticFieldMapRecord>().get(fieldMap); // <---
    //fieldMap_ = fieldMap.product();
    //
    //
    //PropagatorWithMaterial (PropagationDirection dir, const float mass,
    //             const MagneticField * mf=nullptr,const float maxDPhi=1.6,
    //             bool useRungeKutta=false, float ptMin=-1.,bool useOldGeoPropLogic=true);
    //
    // ~

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

void HGCalParticlePropagator::propagate(math::XYZTLorentzVectorF& point, math::XYZTLorentzVectorF& momentum, int charge){
    typedef TrajectoryStateOnSurface TSOS;
    zpos trackz = posZ;
    if(momentum.z()<0) trackz = negZ;
    const double caloz = trackz == posZ ? frontz_ : backz_;

    bool failed=true;

    n_propagated_++;

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

        failed=false;
    }
    else{
        const MagneticField * field=bField_.product();
        auto & RKprop = rkprop_->propagator;

        GlobalPoint gpoint(point.x(),point.y(),point.z());
        GlobalVector gmomentum(momentum.x(),momentum.y(),momentum.z());

        if(fabs(point.z())>fabs(caloz))
            gmomentum *= -1;

        TSOS startingState( GlobalTrajectoryParameters(gpoint,
                gmomentum, charge, field));

        TSOS propState = RKprop.propagate( startingState, frontFaces_[trackz]->surface());

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

            failed=false;
        }
    }
    if(fabs(point.z()) - fabs(getHGCalZ()) > 0.01)
        failed=true;
    if(failed){
        std::cout << "\n prop failed for point " << point <<
                ", eta: " << point.Eta() << ", phi: " << point.Phi() <<
                " charge " << charge <<  std::endl;
        std::cout << "pos " << trackz << " calo z " << caloz <<" ";
        std::cout << "propagate z " << point.z() << " with momentum z " << momentum.z() << " and betaz " << betazc << std::endl;
        std::cout << "distance " << zdist << " time to travel " << timeprop << std::endl;
        n_failed_++;
    }
}

double HGCalParticlePropagator::getHGCalZ()const{
    return frontFaces_[posZ].get()->position().z();
}
