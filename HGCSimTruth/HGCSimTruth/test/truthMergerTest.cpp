
#include "../interface/TruthMergerTools.h"
#include <algorithm>
#include <iostream>


class floatMergeWrapper: public merger::MergeWrapper<float,floatMergeWrapper>{

public:


    floatMergeWrapper(const float* x):x_(*x){}

    float mergeScore(const floatMergeWrapper* rhs)const{
        //this cannot be anything but floatMergeWrapper
        return 1./(std::abs(x_-rhs->x_)+1e-4);
    }

    float mergeAllInList(const std::vector<floatMergeWrapper* >& tomerge)const{
        float out = 0;
        for(const auto mr: tomerge){
            out += mr->x_;
        }
        return out;
        /*
         *
         * for the by-primary merger for simclusters:
         *   - energy: simple sum
         *   - ID, exploit that it is by primary:
         *          - take union of all parent simtracks
         *          - determine the one closest in momentum to SC sum
         *          - take that ID if momentum difference within X% threshold
         *          - assign 0 otherwise
         *
         *
         * to retrieve the grouping just make another small wrapper with index (or use the simtrack indices)
         */
    }


private:
    float x_;
};




int main(){

    // using logic of floatMergeWrapper to merge floats
    // in principle other wrappers can define other logic for floats,
    // therefore both, the logic in the wrapper, as well as the target
    // need to be defined
    merger::ObjectMerger<floatMergeWrapper,float> merger;

    std::vector<float> ps={
            0.9,1.,1.1,0.95,  // 3.95+1.29, needs one iteration
            1.9, 2.19, 2., // 6.09 , needs two iterations
            4., //4
            8., //8
            1.29 //will be added to first in first iteration
    };

    float threshold = 1/0.2;//will merge the groups as in the initialiser list
    for(const auto& p:ps){
        merger.add(&p);
    }
    std::vector<std::vector<size_t> > idxs;
    std::vector<float> merged = merger.mergeSymmetric(threshold,&idxs);

    std::cout << "merged to total " << merged.size() << " objects with properties:" <<std::endl;
    for(const auto& m:merged){
        std::cout << m << std::endl;
    }
    std::cout << "merged groups (corresponding to the order the objects were added to the merger)" << std::endl;
    for(const auto& i:idxs){
        for(const auto& ii:i)
            std::cout << ii << " " ;
        std::cout << std::endl;
    }

    return 1;
}
