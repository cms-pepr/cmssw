
#ifndef RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_HGCALSIMCLUSTERMERGER_H_
#define RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_HGCALSIMCLUSTERMERGER_H_


#include <algorithm>
#include <numeric>

#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"
#include "DataFormats/GeometryVector/interface/LocalVector.h"
#include "SimHistoryTool.h"


//this can be used for PF truth but also for standalone
//in case of PF, it needs to run differently if tracks are present
//therefore, it is more convenient standalone than as an edm::producer
//never merges any non-hgcal simcluster with an hgcal simcluster or never a non-hgcal simcluster
class HGCalSimClusterMerger{
public:
    HGCalSimClusterMerger(const HGCRecHitCollection& rechits,
           const hgcal::RecHitTools * rechittools,
           const SimHistoryTool * hist);

    std::vector<SimCluster> downScaleMuons(const std::vector<const SimCluster*>& scs) const;

    std::vector<SimCluster> merge(const std::vector<const SimCluster*>& scs,
            std::vector<std::vector<size_t> >& idxs) const;

    //with the current algorithm, this is the only one that matters!
    void setCNLayers(int cNLayers) { cNLayers_ = cNLayers; }
    void setHighEfracThreshold(float thresh) { isHighEfracThreshold_ = thresh; }
    void setConnectThreshold(float thresh) { connectThreshold_ = thresh; }



    //helper
    //credit to Thomas
    template <typename T>
    std::vector<std::size_t> argsort(const std::vector<T>& vec)const;

    //credit to Thomas
    template <typename T>
    void apply_argsort_in_place( std::vector<T>& vec,
            const std::vector<std::size_t>& p)const;

    bool isHGCal(const SimCluster& cluster)const;
    float recEnergy(const SimCluster& cluster)const;

  private:
    HGCalSimClusterMerger(){}

    double calcCircle(const SimCluster* sc, float& assignedEnsum, float& regionEnSum, float& firsthitradius)const;
    void createHitMap();
    const HGCRecHit* getHit(DetId)const;

    //this one could be moved to the rechit tools?
    bool isHGCal(DetId)const;

    inline LocalVector getHitPosVec(DetId id)const{
        auto gppos = rechittools_->getPosition(id);
        return LocalVector(gppos.x(),gppos.y(),gppos.z());
    }

    float getHitRadius(DetId id)const;



    const HGCRecHitCollection * rechits_;
    const hgcal::RecHitTools* rechittools_ ;
    const SimHistoryTool * histtool_;
    std::map<DetId, size_t> hitmap_;

    //helper class
    //cache some variables
    class HitStruct{
    public:
        HitStruct(DetId ID, const HGCRecHit* rh, float frac,
                float rad): id(ID),rechit(rh),fraction(frac),radius(rad){}
        DetId id;
        const HGCRecHit* rechit;
        float fraction;
        float radius;
        inline float energy()const{return rechit->energy()*fraction;}
    };

    //merge parameters
    int cNLayers_;
    float isHighEfracThreshold_;
    float connectThreshold_;


};

//credit to Thomas
template <typename T>
std::vector<std::size_t> HGCalSimClusterMerger::argsort(const std::vector<T>& vec)const{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(), [&](std::size_t i, std::size_t j) { return vec[i] < vec[j]; });
    return p;
}

//credit to Thomas
template <typename T>
void HGCalSimClusterMerger::apply_argsort_in_place(
        std::vector<T>& vec,
        const std::vector<std::size_t>& p)const
{
  std::vector<bool> done(vec.size());
  for (std::size_t i = 0; i < vec.size(); ++i) {
    if (done[i]) {
      continue;
    }
    done[i] = true;
    std::size_t prev_j = i;
    std::size_t j = p[i];
    while (i != j) {
      std::swap(vec[prev_j], vec[j]);
      done[j] = true;
      prev_j = j;
      j = p[j];
    }
  }
}

#endif
