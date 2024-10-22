

#include "../interface/SimHistoryTool.h"
#include <numeric>

namespace dummy{
std::vector<SimTrack> dummyst;
std::vector<SimVertex> dummysv;
}


SimCluster SimHistoryTool::createMergedSimCluster(std::vector<const SimCluster* > tomerge)const{

    SimCluster out;

    if(tomerge.size()<1)
        return out;
    if(tomerge.size()==1)
        return *tomerge.at(0);

    math::XYZTLorentzVectorF p4, point;
    double totalE=0;
    for(const auto& sc: tomerge){
        out += *sc;
    }
    //////// more complex part assigning an ID

    out.setPdgId(createMergedSimClusterID(&out));

    return out;

}

// FIXME: this needs to be made PU safe with event ID etc
const SimTrack* SimHistoryTool::traverseStep(const SimTrack * st)const{

    const SimTrack* out=st;
    int vidx = out->vertIndex();
    if(vidx<0)
        return out; //no vertex
    const SimVertex& vertex = simvertices_.at(vidx);
    int stid = vertex.parentIndex();//this is Geant track ID, not vector index
    if(stid < 0)
        return out;//no parent
    int stidx = trackIdToTrackIdxAsso_.at(stid); //get vector index
    if(stidx < 0)
        return out;//id does not exist
    const SimTrack & simtrack = simtracks_.at(stidx);
    vidx = simtrack.vertIndex();
    out=&simtrack;
    return out;
}

std::vector<const SimTrack*> SimHistoryTool::createHistory(const SimTrack * st)const{
    std::vector<const SimTrack*> outv;
    const SimTrack* out=st;
    outv.push_back(st);//also self-reference
    while(true){
        const SimTrack * test = traverseStep(out);
        if(test == out || test==0)//root or invalid
            break;
        out=test;
        outv.push_back(test);
    }
    return outv;
}

bool SimHistoryTool::inHistory(const SimTrack & possibleParent, const SimTrack * probe)const{
    if(simTrackEqual(*probe, possibleParent))
        return true;
    while(true){
        const SimTrack * test = traverseStep(probe);
        if(test == probe || test==0)//root or invalid
            return false;
        if(simTrackEqual(*test, possibleParent))
            return true;
        probe=test;
    }
}

int SimHistoryTool::createMergedSimClusterID(const SimCluster* sc, double idenfrac)const{

    int mergedPdgId=0;
    double totalE = sc->impactMomentum().E();

    //run on the merged simtracks and their history
    const auto& sts = sc->g4Tracks();


    std::vector<std::vector<const SimTrack*> > scHists;
    std::vector<const SimTrack*> allst;//unique st pointers
    double ensum=0;
    for(const auto& st: sts){
        scHists.emplace_back(createHistory(&st));
        const auto& last = scHists.at(scHists.size()-1);
        for(const auto& lst:last){
            if(std::find(allst.begin(),allst.end(),lst) == allst.end())
                allst.push_back(lst);
        }
    }
    std::vector<float> enIntoGroup(allst.size(),0.);

    for(size_t ip=0;ip<allst.size();ip++){//unique
        for(size_t i=0;i<scHists.size();i++){
            if(std::find(scHists.at(i).begin(),scHists.at(i).end(),allst.at(ip)) != scHists.at(i).end()){
                enIntoGroup.at(ip) += sts.at(i).getMomentumAtBoundary().E();
            }
        }
    }
    //arg sort
    std::vector<std::size_t> sortEnIntoGroup(enIntoGroup.size());
    std::iota(sortEnIntoGroup.begin(), sortEnIntoGroup.end(), 0);
    std::sort(sortEnIntoGroup.begin(), sortEnIntoGroup.end(),
            [&](std::size_t i, std::size_t j){ return enIntoGroup[i] > enIntoGroup[j]; });

    //which has the largest contribution

    mergedPdgId=0;
    for(const auto& idx: sortEnIntoGroup){
        float stE = allst.at(idx)->momentum().E();
        float gE = enIntoGroup.at(idx);
        float motherFracE = gE/stE;

        if(gE/totalE < idenfrac)
            break;

        if(motherFracE > idenfrac){
            mergedPdgId = allst.at(idx)->type();
        }

        //std::cout << allst.at(idx)->type() << ": " << allst.at(idx)->momentum().E()
        //        << "; " << enIntoGroup.at(idx) << " | "<<totalE << "\n";
    }


    return mergedPdgId;

}


bool SimHistoryTool::simTrackEqual(const SimTrack& a,const SimTrack& b){
    bool trkid = a.trackId() == b.trackId();
    bool evid = a.eventId() == b.eventId();
    bool vid = a.vertIndex() == b.vertIndex();//that enough?
    return trkid && evid && vid;
}

const SimTrack* SimHistoryTool::getRoot(const SimTrack * st) const{
    const SimTrack* out=st;
    while(true){
        const SimTrack * test = traverseStep(out);
        if(test == out || test==0)//root or invalid
            break;
        out=test;
    }
    return out;
}

void SimHistoryTool::createIdxAsso(){
    trackIdToTrackIdxAsso_.clear();
    for(size_t i=0;i<simtracks_.size();i++){
        trackIdToTrackIdxAsso_[simtracks_.at(i).trackId()] = i;
    }
}

