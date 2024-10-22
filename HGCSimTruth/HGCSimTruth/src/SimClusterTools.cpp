/*
 * SimClusterTools.cpp
 *
 *  Created on: 16 Dec 2019
 *      Author: jkiesele
 */

#include "../interface/SimClusterTools.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/ForwardDetId/interface/ForwardSubdetector.h"

#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCScintillatorDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/HcalTowerAlgo/interface/HcalGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"

#include <cmath>

void SimClusterTools::recalculatePosition(SimCluster& cluster, const double& assigned_time) const {
  if (!recHitTools_) {
    throw cms::Exception("recHitTools not set");
  }

  int layer = 4000;
  double lowestz = 4000.;
  double center_eta = 0, center_rphi = 0;
  double energysum = 0;
  for (const auto& hitsAndFractions : cluster.hits_and_fractions()) {
    auto detid = hitsAndFractions.first;
    int thislayer = recHitTools_->getLayer(detid);
    auto rhpos = recHitTools_->getPosition(detid);
    double thisz = fabs(rhpos.z());
    if (lowestz > thisz) {
      lowestz = thisz;
    }
    if (thislayer < layer)
      layer = thislayer;

    double energy = hitsAndFractions.second;
    if (energy == 0 || energy != energy || std::isinf(energy))
      energy = 1e-4;
    if (allrechits_) {
      auto mapit = detid_to_rh_index_->find(detid);
      if (mapit == detid_to_rh_index_->end())
        continue;
      energy = allrechits_->at(mapit->second)->energy() * hitsAndFractions.second;
    }
    energysum += energy;
    center_eta += rhpos.eta() * energy;
    center_rphi += reco::deltaPhi(rhpos.phi(), cluster.impactPoint().phi()) * energy;
  }
  if (energysum) {
    center_eta /= energysum;
    center_rphi /= energysum;
    center_rphi += cluster.impactPoint().phi();
    center_rphi = reco::reduceRange(center_rphi);
  }

  //std::cout << "lowest layer " << layer << std::endl;
  //assign position to first layer hit
  math::XYZVectorF thispos(0, 0, 0);
  int nhits = 0;
  double layeren = 0;
  //these are still energies in the accumulation step
  for (const auto& hitsAndEnergies : cluster.hits_and_fractions()) {
    auto detid = hitsAndEnergies.first;

    int thislayer = recHitTools_->getLayer(detid);
    float thisz = fabs(recHitTools_->getPosition(detid).z());
    if (fabs(thisz - lowestz) < 0.3) {  //thislayer == layer){

      double energy = hitsAndEnergies.second;
      auto ipos = recHitTools_->getPosition(detid).basicVector();
      //use rechit energies here
      if (allrechits_) {
        auto mapit = detid_to_rh_index_->find(detid);
        if (mapit == detid_to_rh_index_->end())
          continue;
        energy = allrechits_->at(mapit->second)->energy() * hitsAndEnergies.second;
      }
      //don't consider strays
      if (reco::deltaR(center_eta, center_rphi, ipos.eta(), ipos.phi()) > considerhitdistance_)
        continue;
      if (energy == 0 || energy != energy || std::isinf(energy))
        energy = 1e-4;
      layeren += energy;
      //std::cout << "thislayer " << thislayer << " "<< ipos <<  " energy "<< energy<<std::endl;
      thispos += math::XYZVectorF(ipos.x(), ipos.y(), ipos.z()) * energy;
      nhits++;
    }
  }
  thispos /= layeren;
  //cluster.setImpactPoint(
  //    ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<float>>(thispos.x(), thispos.y(), thispos.z(), assigned_time));

  //std::cout << "new pos: " << cluster.impactPoint() << std::endl;
}

bool SimClusterTools::isHGCal(const SimCluster& cluster)const{

    for (const auto& hitsAndEnergies : cluster.hits_and_fractions()) {
        const DetId id = hitsAndEnergies.first;
        bool forward = id.det() == DetId::HGCalEE
                || id.det() == DetId::HGCalHSi
                || id.det() == DetId::HGCalHSc
                || (id.det() == DetId::Forward && id.subdetId() != static_cast<int>(HFNose))
                || (id.det() == DetId::Hcal && id.subdetId() == HcalSubdetector::HcalEndcap);

        if(forward)
            return true;
    }
    return false;
}





