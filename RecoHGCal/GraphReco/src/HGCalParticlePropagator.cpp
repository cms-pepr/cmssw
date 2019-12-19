
#include "../interface/HGCalParticlePropagator.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FastSimulation/ParticlePropagator/interface/ParticlePropagator.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "MagneticField/Engine/interface/MagneticField.h"

HGCalParticlePropagator::~HGCalParticlePropagator(){
    if(rkprop_)
        delete rkprop_;
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

    if(!charge){
        auto normmom = momentum / std::sqrt(momentum.Vect().mag2());
        normmom /= fabs(normmom.z());
        double zdist = frontz_ - point.z();
        if(trackz == negZ)
            zdist = backz_ - point.z();

        normmom *= fabs(zdist);
        //time is missing, but trivial
        float timedelay=0;

        point = math::XYZTLorentzVectorF(normmom.x()+point.x(),
                            normmom.y()+point.y(),
                            normmom.z()+point.z(),
                            timedelay+point.T());
        return;
    }

    const MagneticField * field=bField_.product();
    auto & RKprop = rkprop_->propagator;

    GlobalPoint gpoint(point.x(),point.y(),point.z());
    GlobalVector gmomentum(momentum.x(),momentum.y(),momentum.z());

    TSOS startingState( GlobalTrajectoryParameters(gpoint,
            gmomentum, charge, field));

    TSOS propState = RKprop.propagate( startingState, frontFaces_[trackz]->surface());

    if (propState.isValid()){
        auto proppoint = propState.globalPosition();
        auto propmomentum = propState.globalMomentum();

        //time is missing, but almost trivial
        float timedelay=0;

        point = math::XYZTLorentzVectorF(proppoint.x(),
                proppoint.y(),
                proppoint.z(),
                timedelay+point.T());

        momentum = math::XYZTLorentzVectorF(propmomentum.x(),
                propmomentum.y(),
                propmomentum.z(),
                momentum.T());

    }

    //RawParticle part(momentum.x(),momentum.y(),momentum.z(),momentum.t(), charge);
    //part.setVertex(point);
    //
    //PropagatorWithMaterial (PropagationDirection dir, const float mass,
    //             const MagneticField * mf=nullptr,const float maxDPhi=1.6,
    //             bool useRungeKutta=false, float ptMin=-1.,bool useOldGeoPropLogic=true);
    //
    // ~
    //
    //ParticlePropagator p(part,
    //                     120.,
    //                     hgcalz_,
    //                     fieldMap_,
    //                     0 ,//const RandomEngineAndDistribution* engine,
    //                     0) ;//const HepPDT::ParticleDataTable* table)
    //
    //if(!p.getSuccess())
    //    throw cms::Exception("Propagation failed");
    //auto propd = p.propagated();
    //
    //momentum = propd.particle().momentum();
    //point = propd.particle().vertex();
}

double HGCalParticlePropagator::getHGCalZ()const{
    return frontFaces_[posZ].get()->position().z();
}
