
#ifndef RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_SIMCLUSTERMERGER_H_
#define RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_SIMCLUSTERMERGER_H_

#include "SCMergeTools.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"

/*
 * Contains differernt incarnations for simcluster merging, based on SCMergeTools and SimHistory
 */


class SimClusterMergeWrapper{
public:
    SimClusterMergeWrapper(const SimCluster* sc,size_t idx):sc_(sc), etot_(0), idx_(idx){
        pos_ = calcBoundaryPos(sc);
    };


    const math::XYZTLorentzVectorF& pos()const{return pos_;}
    float x()const{return pos_.x();}
    float y()const{return pos_.y();}
    float z()const{return pos_.z();}

    const SimCluster* SC()const{return sc_;}

    const float& etot()const{return etot_;}
    const size_t& idx()const{return idx_;}

    void setEtot(float etot){etot_=etot;}//needs to be adapted later

private:
    //helper until simtrack<->sc boundary treatment is non ambiguous
    // TBI
    math::XYZTLorentzVectorF calcBoundaryPos(const SimCluster* sc)const;

    //helper to avoid minkowski metric for .Mag()
    inline float LVDistSq(const math::XYZTLorentzVectorF& a, const math::XYZTLorentzVectorF& b) const {
      return (a.x() - b.x()) * (a.x() - b.x()) +
             (a.y() - b.y()) * (a.y() - b.y()) +
             (a.z() - b.z()) * (a.z() - b.z()) +
             (a.t() - b.t()) * (a.t() - b.t());
    }

    const SimCluster * sc_;
    //additional information for merging
    float etot_;
    size_t idx_;
    math::XYZTLorentzVectorF pos_;//cached for simplicity
};



class HitMergeVertex: public MergeVertex<SimClusterMergeWrapper>{
public:

    using RecHit = HGCRecHit;

    HitMergeVertex(const SimClusterMergeWrapper* t,
            const std::vector<RecHit> & allhits,
            const std::vector<std::pair<uint32_t,float> >& hafs
            );


    bool shouldConnect(const HitMergeVertex* rhs)const;

    void updateIsLow();

    void absorb(HitMergeVertex* rhs);


    inline float z()const{
        if(valid())
            return objects().at(0)->z();//DEBUG
        return 0;
    }

    //DEBUG only
    float energy(float x, float y)const;

    /*
     * how much if the rhs energy is contained in intersecting hits
     * union
     */
    float calcScore(const HitMergeVertex* rhs)const;

    static float connectThreshold;
    static float singleClusterEfrac;

private:

    void updateEnergies();

    class Haf{
    public:
        Haf(const RecHit * h, float f):rechit(h),frac(f){}

        const RecHit * rechit;
        float frac;

        //for sorting by ID (just performance)
        bool operator == (const Haf& rhs)const{return rechit->id() == rhs.rechit->id();}
        bool operator < (const Haf& b) const{
            return rechit->id()<b.rechit->id();
        }
    };

    std::vector<Haf> hafs_;

    mutable std::pair<const HitMergeVertex* , float> cached_score_;

    //total energy of this vertex, using fractions
    float etot_;

    //total energy of all hits assigned to this vertex, not including fractions
    float eallhits_;

    void absorbHafs(const std::vector<Haf>& h_rhs);
};


#endif
