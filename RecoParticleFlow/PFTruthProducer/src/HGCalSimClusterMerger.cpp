
#include "../interface/HGCalSimClusterMerger.h"
#include "../interface/SimClusterMerger.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"


HGCalSimClusterMerger::HGCalSimClusterMerger(const HGCRecHitCollection& rechits,
           const hgcal::RecHitTools * rechittools,
           const SimHistoryTool * hist
    ):rechits_(&rechits),rechittools_(rechittools),histtool_(hist){
        createHitMap();
        //whatever is smaller
        cNLayers_=10;
        isHighEfracThreshold_ = 0.85;
        connectThreshold_ = 0.3;
}

std::vector<SimCluster> HGCalSimClusterMerger::downScaleMuons(const std::vector<const SimCluster*>& scs) const{

    std::vector<SimCluster> out;
    for(const auto& sc: scs){
        if(abs(sc->pdgId()) == 13 && isHGCal(*sc)){
            auto sc2 = *sc;
            float recenergy = recEnergy(sc2);
            auto sc2mom = sc2.impactMomentum();
            float momMag = sc2mom.P();
            sc2.setImpactMomentum(math::XYZTLorentzVectorF(
                    sc2mom.x() / momMag * recenergy,
                    sc2mom.y() / momMag * recenergy,
                    sc2mom.z() / momMag * recenergy,
                    sc2mom.t()
            ));
            out.push_back(sc2);
        }
        else{
            out.push_back(*sc);
        }
    }
    return out;
}


std::vector<SimCluster> HGCalSimClusterMerger::merge(const std::vector<const SimCluster*>& scs,
        std::vector<std::vector<size_t> >& idxs) const{


    //create the wrappers
    HitMergeVertex::connectThreshold=connectThreshold_;
    HitMergeVertex::singleClusterEfrac=isHighEfracThreshold_;

    std::cout << "merging " << scs.size() << std::endl;//DEBUG

    std::vector<SimClusterMergeWrapper> scwappers;

    std::vector<float> debugradii, debugenergies;

    size_t idx=0;
    for(const auto& sc: scs){
        if(!isHGCal(*sc))
            continue;
        scwappers.push_back(SimClusterMergeWrapper(sc,idx));
        idx++;
    }

    MergeGraph<HitMergeVertex > graph;

    //just a duplicate for the moment
    //TBI performance remove duplicate
    std::vector<HGCRecHit> allhits;
    for(size_t i=0;i<rechits_->size();i++)
        allhits.push_back((*rechits_)[i]);


    auto start = std::chrono::high_resolution_clock::now();//DEBUG

    // restrict the hits that are actually used for the merging
    for(const auto& scw: scwappers){
        auto hafs_in = scw.SC()->hits_and_fractions();

        std::vector<std::pair<uint32_t,float>> hafs;
        for(const auto& hf: hafs_in){
            if(rechittools_->getLayer(hf.first) > (size_t)cNLayers_ )
                continue; //not within layer constraints
            hafs.push_back(hf);
        }

        //this will calculate all total energies etc. no need to add by hand
        graph.addVertex( HitMergeVertex(&scw, allhits, hafs) );
    }

    graph.createEdges();

    graph.merge();

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "took "<< duration.count()/1000. <<" milliseconds for "<< scwappers.size()<< " showers." << std::endl;

    //collect the result
    std::vector<SimCluster> out;
    idxs.clear();
    //merge done. get result
    auto result = graph.vertices();
    for(const auto& vert: result){
        if(!vert.valid())
            continue;
        std::vector<size_t> tmpidxs;
        std::vector<const SimCluster* > tomerge;
        for(const auto scw: vert.objects()){
            tmpidxs.push_back(scw->idx());
            tomerge.push_back(scw->SC());
        }
        out.push_back(histtool_->createMergedSimCluster(tomerge));
        idxs.push_back(tmpidxs);
    }

    std::cout << "merged from " << scwappers.size() << " to " << out.size() << std::endl;//DEBUG

    return out;

}

bool HGCalSimClusterMerger::isHGCal(const SimCluster& cluster)const{
    for (const auto& hitsAndEnergies : cluster.hits_and_fractions()) {
        const DetId id = hitsAndEnergies.first;
        if(isHGCal(id))
            return true;
    }
    return false;
}

float HGCalSimClusterMerger::recEnergy(const SimCluster& cluster)const{
    float en=0;
    for (const auto& haf : cluster.hits_and_fractions()) {
        auto hit = getHit(haf.first);
        if(hit)
            en += hit->energy() * haf.second;
    }
    return en;
}

bool HGCalSimClusterMerger::isHGCal(DetId id)const{
    return id.det() == DetId::HGCalEE
            || id.det() == DetId::HGCalHSi
            || id.det() == DetId::HGCalHSc
            || (id.det() == DetId::Forward && id.subdetId() != static_cast<int>(HFNose))
            || (id.det() == DetId::Hcal && id.subdetId() == HcalSubdetector::HcalEndcap);
}

float HGCalSimClusterMerger::getHitRadius(DetId id)const{
    if(rechittools_->isSilicon(id)){
        return rechittools_->getRadiusToSide(id);
    }
    else if(rechittools_->isScintillator(id)){
        auto hit_pos = getHitPosVec(id);
        auto etaphi = rechittools_->getScintDEtaDPhi(id);
        float maxdetadphi = std::max(etaphi.first,etaphi.second);
        return 0.5 * hit_pos.perp() * maxdetadphi * sqrt(2.);
    }
    return 0;
}


void HGCalSimClusterMerger::createHitMap(){
  hitmap_.clear();
  for (size_t i = 0; i < rechits_->size(); i++)
    hitmap_[(*rechits_)[i].detid()] = i;
}



const HGCRecHit* HGCalSimClusterMerger::getHit(DetId id) const {
  auto it = hitmap_.find(id);
  if (it == hitmap_.end())
      return 0;
  return &(*rechits_)[it->second];
}

