
#ifndef RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_SIMHISTORYTOOL_H_
#define RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_SIMHISTORYTOOL_H_


#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include <vector>
#include <map>

namespace dummy{
extern std::vector<SimTrack> dummyst;
extern std::vector<SimVertex> dummysv;
}

// this is not PU safe yet. Needs protection against PU entries
class SimHistoryTool{
public:
    SimHistoryTool(const std::vector<SimTrack> & simtracks,
        const std::vector<SimVertex> & simvertices):
            simtracks_(simtracks),simvertices_(simvertices)
        {createIdxAsso();}

    /// high-level functions

    SimCluster createMergedSimCluster(std::vector<const SimCluster* >)const;
    // low level functions

    const SimTrack* getRoot(const SimTrack * st)const;
    const SimTrack* traverseStep(const SimTrack * st)const;
    /**
     * creates a vector containing all previous simtracks towards
     * (and including) the root track
     */
    std::vector<const SimTrack*> createHistory(const SimTrack * st)const;

    bool inHistory(const SimTrack & possibleParent, const SimTrack * probe)const;

    int createMergedSimClusterID(const SimCluster* sc, double idenfrac=0.9)const;

    static bool simTrackEqual(const SimTrack&,const SimTrack&);

private:
    SimHistoryTool():simtracks_(dummy::dummyst),simvertices_(dummy::dummysv){}
    void createIdxAsso();

    const std::vector<SimTrack> & simtracks_;
    const std::vector<SimVertex> & simvertices_;
    std::map<int,int>  trackIdToTrackIdxAsso_;
};


#endif
