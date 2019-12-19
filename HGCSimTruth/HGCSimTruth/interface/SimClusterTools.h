/*
 * SimClusterTools.h
 *
 *  Created on: 16 Dec 2019
 *      Author: jkiesele
 */

#ifndef PRODUCTION_HGCALSIM_CMSSW_JKIESELE_CMSSW_11_0_0_PRE9_SRC_RECOHGCAL_GRAPHRECO_INTERFACE_SIMCLUSTERTOOLS_H_
#define PRODUCTION_HGCALSIM_CMSSW_JKIESELE_CMSSW_11_0_0_PRE9_SRC_RECOHGCAL_GRAPHRECO_INTERFACE_SIMCLUSTERTOOLS_H_

#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include <algorithm>
#include <unordered_map>

class SimClusterTools {
public:
  SimClusterTools() : recHitTools_(0), detid_to_rh_index_(0), allrechits_(0), considerhitdistance_(0.1) {}

  void recalculatePosition(SimCluster& cluster, const double& assigned_time) const;

  void setRechitTools(const hgcal::RecHitTools& rht) { recHitTools_ = &rht; }

  void setRechitVector(const std::vector<const HGCRecHit*>& rhv) { allrechits_ = &rhv; }

  void setIndexMap(const std::unordered_map<DetId, size_t>& idxmap) { detid_to_rh_index_ = &idxmap; }

private:
  const hgcal::RecHitTools* recHitTools_;
  const std::unordered_map<DetId, size_t>* detid_to_rh_index_;
  const std::vector<const HGCRecHit*>* allrechits_;
  float considerhitdistance_;
};

#endif /* PRODUCTION_HGCALSIM_CMSSW_JKIESELE_CMSSW_11_0_0_PRE9_SRC_RECOHGCAL_GRAPHRECO_INTERFACE_SIMCLUSTERTOOLS_H_ */
