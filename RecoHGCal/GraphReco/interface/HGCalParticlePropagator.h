/*
 * HGCalParticlePropagator.h
 *
 *  Created on: 10 Dec 2019
 *      Author: jkiesele
 */

#ifndef PRODUCTION_HGCALSIM_CMSSW_JKIESELE_CMSSW_11_0_0_PRE9_SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALPARTICLEPROPAGATOR_H_
#define PRODUCTION_HGCALSIM_CMSSW_JKIESELE_CMSSW_11_0_0_PRE9_SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALPARTICLEPROPAGATOR_H_



#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "FastSimulation/ParticlePropagator/interface/MagneticFieldMapRecord.h"
#include "TrackPropagation/RungeKutta/interface/defaultRKPropagator.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "DataFormats/GeometrySurface/interface/BoundDisk.h"

//#include "TrackingTools/MaterialEffects/interface/PropagatorWithMaterial.h"//use this guy directly. can do masses!




class MagneticField;


class HGCalParticlePropagator{
public:
    enum zpos{ negZ=0, posZ=1};

    HGCalParticlePropagator():rkprop_(0),frontz_(0),backz_(0),setup_(false){}
    ~HGCalParticlePropagator();
    void setEventSetup(const edm::EventSetup &es);

    void propagate(math::XYZTLorentzVectorF& point, math::XYZTLorentzVectorF& momentum, int charge);

    double getHGCalZ()const;
private:
    edm::ESHandle<MagneticField> bField_;
    defaultRKPropagator::Product * rkprop_;
    double frontz_,backz_;
    std::unique_ptr<GeomDet> frontFaces_[2];
    bool setup_;
};


#endif /* PRODUCTION_HGCALSIM_CMSSW_JKIESELE_CMSSW_11_0_0_PRE9_SRC_RECOHGCAL_GRAPHRECO_INTERFACE_HGCALPARTICLEPROPAGATOR_H_ */
