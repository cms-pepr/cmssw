/*
 * TruthMergerTools.h
 *
 *  Created on: 22 Oct 2021
 *      Author: jkiesele
 */

#ifndef HGCSIMTRUTH_HGCSIMTRUTH_INTERFACE_TRUTHMERGERTOOLS_H_
#define HGCSIMTRUTH_HGCSIMTRUTH_INTERFACE_TRUTHMERGERTOOLS_H_

#include <vector>
#include <stdexcept>

namespace merger{
/*
 * workflow in short:
 *
 *   - create a wrapper class W inheriting from MergeWrapper around the object
 *     to merge, describing the merging logic (see below)
 *   - add each wrapped object to a ObjectMerger<W,T>, where T is the final target class
 *   - call the mergeSymmetric function: returns vector of merged Ts
 *   - retrieve original objects from the Ts
 *
 */

template<class T,class U>
class MergeWrapper{
    /*
     * Base class to create merge wrappers.
     * Inherit in the following way for a target class T
     * to be merged, and a class Derived describing the merging
     * logic:
     *
     * class Derived: public truth::MergeWrapper<T,Derived>{...}
     *
     */
public:

    virtual ~MergeWrapper(){}

    /*
     * inheriting class 'Derived' must to implement a constructor based
     * on a pointer to the target object:
     *   Derived(const T* obj);
     */

    /*
     * inheriting class 'Derived' must to implement the logic that
     * calculates a symmetric merge score between two instances
     */
    virtual float mergeScore(const U* rhs)const=0;


    /*
     * inheriting class 'Derived' must implement the logic that
     * creates a new target object out of a list of instances
     * of type Derived.
     */
    virtual T mergeAllInList(const std::vector<U *>&)const=0;

    /*
     * inheriting class may implement a recalculation of merging
     * variables that is called each time an object is merged with another.
     * It must return true if the variables are recalculated and
     * must clear the vector v.
     */
    bool recalcVariables(std::vector<U *>& v){return false;}

};


/*
 *
 * hidden helper class
 *
 * U are here MergeWrapper, T is the target class
 * objects already
 *
 */
template<class U, class T>
class MergeObject {
public:

    MergeObject(U * obj, size_t idx){
        objects_.push_back(obj);
        idxs_.push_back(idx);
    }

    bool operator==(const MergeObject<U,T>& rhs)const{
        return objects_==rhs.objects_;
    }

    void absorb(MergeObject& rhs){
        if(&rhs == this)
            return;//can't absorb itself
        objects_.insert(objects_.end(),rhs.objects_.begin(),rhs.objects_.end());
        rhs.objects_.clear();
        idxs_.insert(idxs_.end(),rhs.idxs_.begin(),rhs.idxs_.end());
        rhs.idxs_.clear();
        if(objects_.size()){
            U *  firstobj = objects_.at(0);
            bool updated = firstobj->recalcVariables(objects_);
            if(updated){
                if(objects_.size())
                    throw std::runtime_error("MergeObject<U,T>: recalcVariables called but vector not cleared. Please clear objects when implementing recalcVariables");
                objects_.push_back(firstobj);//always keep this one
            }

        }
    }

    bool isEmpty()const{
        return objects_.size()==0;
    }

    float maxMergeScore(const MergeObject& rhs)const;

    T mergedObject()const{
        if(isEmpty())
            return T();
        return objects_.at(0)->mergeAllInList(objects_);
    }

    const std::vector<size_t>& idxs()const{return idxs_;}

private:
    std::vector<U* > objects_;
    std::vector<size_t> idxs_;
};


template<class U,class T>
float MergeObject<U,T>::maxMergeScore(const MergeObject& rhs)const{
    float maxscore=-1e12;
    for(const auto oi: objects_){
        for(const auto oj: rhs.objects_){
            float score = oi->mergeScore(oj);
            if(score>maxscore)
                maxscore=score;
        }
    }
    return maxscore;
}

/*
 * each of these can contain a group of objects that can be merged
 * if there are multiple it is safe to parallelise over them.
 *
 * class T here is target, so type safe with casts within MergeWrapper<U> dervied class
 *
 */
template<class U, class T>
class ObjectMerger{
public:
    //keep track of the original objects here and just pass pointers to the MergeObject one

    void add(const T * o){
        objects_.push_back(U(o));
    }

    /*
     * returns merged target objects
     */
    std::vector<T> mergeSymmetric(const float& threshold, std::vector<std::vector<size_t> >* idxs=0)const;

private:
    std::vector<U> objects_;
    std::vector< MergeObject<U,T> > removeEmpty(const std::vector< MergeObject<U,T> >& mo)const;
};


template<class U,class T>
std::vector<T> ObjectMerger<U,T>::mergeSymmetric(const float& threshold, std::vector<std::vector<size_t> >* idxs)const{

    //if idxs not NULL than also fill it

    std::vector< MergeObject<U,T> > merged;
    std::vector<U> objectscp = objects_;
    size_t idx=0;
    for(auto& o:objectscp)
        merged.push_back(MergeObject<U,T>(&o,idx++));

    while(true){
        for(size_t i=0;i<merged.size();i++){
            for(size_t j=i;j<merged.size();j++){
                if(merged.at(i).maxMergeScore(merged.at(j)) > threshold){
                    merged.at(i).absorb(merged.at(j));
                }
            }
        }

        auto newmerged = removeEmpty(merged);
        if(merged == newmerged){//nothing changed
            break;
        }
        merged=newmerged;
    }
    std::vector<T> out;
    if(idxs)
        idxs->clear();
    for(const auto& m: merged){
        out.push_back(m.mergedObject());
        if(idxs){
            idxs->push_back(m.idxs());
        }
    }
    return out;
}


template<class U, class T>
std::vector< MergeObject<U,T> > ObjectMerger<U,T>::removeEmpty(const std::vector< MergeObject<U,T> >& mo)const{
    std::vector< MergeObject<U,T> > out;
    for(const auto o: mo){
        if(!o.isEmpty())
            out.push_back(o);
    }
    return out;
}

}//namespace
#endif
