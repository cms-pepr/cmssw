
#ifndef RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_SCMERGETOOLS_H_
#define RECOPARTICLEFLOW_PFTRUTHPRODUCER_INTERFACE_SCMERGETOOLS_H_


#include <stdexcept>
#include <algorithm>
#include <vector>
#include <string>

//debug
#include <iostream>
//#define DEBUGCOUTS
//#define DEBUGCHECKS

//generic helpers


template<class T, class Compare>
bool _isInSorted( const std::vector<T>& a, const T& cmp,  Compare __c, size_t& start){//standard compare is less
    for(size_t i=start;i<a.size();i++){
        if(__c(a.at(i),cmp)){
            start=i;
            continue;
        }
        if(a.at(i)==cmp){
            start=i+1;
            return true;
        }
    }
    return false;
}

template<class T>
bool isInSorted( const std::vector<T>& a, const T& cmp){//standard compare is less
    size_t start=0;
    return _isInSorted(a,cmp,std::less<T>(),start);
}

template<class T, class Compare>
bool _isInSorted( const std::vector<T>& a, const T& cmp,  Compare __c){//standard compare is less
    size_t start=0;
    return _isInSorted(a,cmp,__c,start);
}

template<class T>
bool isInSorted( const std::vector<T>& a, const T& cmp, size_t& start){//standard compare is less
    return _isInSorted(a,cmp,std::less<T>(),start);
}

template<class T, class Compare>
bool isInSorted( const std::vector<T>& a, const T& cmp, size_t& start, Compare __c){//standard compare is less
    return _isInSorted(a,cmp,__c,start);
}


/*
 * This is O(N_a+N_b) only, not N_a*N_b as a brute force algorithm
 *
 * comapre function returns true if first argument is less than second
 * class T must implement == operator
 */
template<class T, class Compare>
void _intersectSorted( const std::vector<T>& a, const std::vector<T> b,
        std::vector<size_t> & int_a, std::vector<size_t> & int_b,
        Compare __c,
        std::vector<size_t> & u_a, std::vector<size_t> & u_b,
        bool return_unique
        ){

    int_a.clear();
    int_b.clear();
    if(return_unique){
        u_a.clear();
        u_b.clear();
    }

    if(a.size()<1 || b.size()<1)
        return;

    //loop simultaneously through both
    size_t a_idx=0, b_idx=0;
    while(true){
        if(a_idx >= a.size())
            break;
        const T& cmp = a.at(a_idx);
        while(b_idx < b.size() && __c(b.at(b_idx),cmp)){
            b_idx++;
        } //either cmp <= b[idx] or same or b depleted
        if(b_idx >= b.size() && !return_unique){
            break; //no intersection
        }
        else if(b_idx < b.size() && cmp == b.at(b_idx)){
            int_a.push_back(a_idx);
            int_b.push_back(b_idx);
        }
        else if(return_unique){
            u_a.push_back(a_idx);
        }
        a_idx++;
    }
    if(!return_unique)
        return;

    //create b uniques (could be possible in loop above but not clear)
    //TBI PERFORMANCE, don't use find but exploit sorting
    size_t int_b_count=0;
    for(size_t i=0; i< b.size();i++){
        if(!isInSorted(int_b, i, int_b_count)){
            u_b.push_back(i);
        }
        //not in int_b, but int_b sorted
    }
}

//different specialisations of it
template<class T>
void intersectSorted( const std::vector<T>& a, const std::vector<T> b,
        std::vector<size_t> & int_a, std::vector<size_t> & int_b){
    std::vector<size_t> u_a, u_b;
    _intersectSorted(a,b,int_a,int_b,std::less<T>(),u_a,u_b,false);
}

template<class T, class Compare>
void intersectSorted( const std::vector<T>& a, const std::vector<T> b,
        std::vector<size_t> & int_a, std::vector<size_t> & int_b, Compare __c){
    std::vector<size_t> u_a, u_b;
    _intersectSorted(a,b,int_a,int_b,__c,u_a,u_b,false);
}

template<class T>
void intersectSorted( const std::vector<T>& a, const std::vector<T> b,
        std::vector<size_t> & int_a, std::vector<size_t> & int_b,
        std::vector<size_t>& u_a, std::vector<size_t>& u_b){
    ;
    _intersectSorted(a,b,int_a,int_b,std::less<T>(),u_a,u_b,true);
}

template<class T, class Compare>
void intersectSorted( const std::vector<T>& a, const std::vector<T> b,
        std::vector<size_t> & int_a, std::vector<size_t> & int_b,
        std::vector<size_t>& u_a, std::vector<size_t>& u_b,
        Compare __c){
    _intersectSorted(a,b,int_a,int_b,__c,u_a,u_b,true);
}


/*
 * both inputs are sorted individually.
 * There must not be overlap between toinsert and target!
 */
template<class T, class Compare>
void _insertSortedInSorted(const std::vector<T>& toinsert, std::vector<T>& target,
        Compare __c){

    auto old = target;
    target.clear();

    size_t inscounter=0;
    for(size_t i=0; i<old.size(); i++){
        bool newfirst = false;
        if(inscounter < toinsert.size())
            newfirst = __c(toinsert.at(inscounter), old.at(i));
        if(newfirst){
            target.push_back(toinsert.at(inscounter));
            inscounter++;
            i--;
        }
        else{
            target.push_back(old.at(i));
        }
    }
    //in case there is something to append
    while(inscounter < toinsert.size()){
        target.push_back(toinsert.at(inscounter));
        inscounter++;
    }

}
template<class T, class Compare>
void insertSortedInSorted(const std::vector<T>& toinsert, std::vector<T>& target,
        Compare __c){
    _insertSortedInSorted(toinsert,target, __c);
}

template<class T>
void insertSortedInSorted(const std::vector<T>& toinsert, std::vector<T>& target){
    _insertSortedInSorted(toinsert,target, std::less<T>());
}


/*
 *  Generic tools for directed-graph based hierarchical merging
 *  all templates header-only
 */

template<class T>
class MergeVertex{
public:

    MergeVertex():isLow_(true){}
    MergeVertex(const T * t):isLow_(true){
        obj_.push_back(t);
        name_ = std::to_string(n_mergeVertex);
        n_mergeVertex++;
    }

    virtual ~MergeVertex(){}//nothing to do

    const std::string & name()const{return name_;}

    bool isLow()const{return isLow_;}
    bool isHigh()const{return !isLow_;}

    void setLow(bool islow){isLow_=islow;}

    void absorb(MergeVertex * rhs);


    const std::vector<const T *> objects()const{return obj_;}

    bool valid()const{return obj_.size()>0;}

    /**
     * the following are to be overwritten in derived classes
     * these can implement different merge schemes
     */


    virtual void updateIsLow(){
        //FIXME throw error here
    }


    /*
     * basic rules for connections
     */
    bool shouldConnectBase(const MergeVertex* rhs)const;

    /*
     * additional rules to connect an edge
     */
    virtual bool shouldConnect(const MergeVertex* rhs)const{
        return shouldConnectBase(rhs);
    }

    /**
     * high score will be connected before low score
     */
    virtual double calcScore(const MergeVertex* rhs)const{return 1;}


protected:
    std::string name_;

    std::vector<const T *> obj_;

    static int n_mergeVertex;

    bool isLow_;
};

template<class T>
int MergeVertex<T>::n_mergeVertex=0;

template<class T>
void MergeVertex<T>::absorb(MergeVertex<T> * rhs){
    if(this==rhs)
        return; //safety
    obj_.insert(obj_.end(),rhs->obj_.begin(),rhs->obj_.end());
    rhs->obj_.clear();
    updateIsLow();
    name_ = name_+", "+rhs->name_;
    rhs->name_="("+rhs->name_+")";
}

template<class T>
bool MergeVertex<T>::shouldConnectBase(const MergeVertex* rhs)const{
    if(this == rhs)
        return false;
    if(!rhs->valid())
        return false;
    if(!valid())
        return false;
    if(rhs->isHigh())
        return false;
    return true;
}


////////////// minimal edge class

template<class U>
class edge {
public:

    enum etype {lowToLow, highToLow, invalid};

    edge():from_(0), to_(0), qual_(0.),type_(lowToLow){}
    edge(U* from, U* to, double qual):from_(from),to_(to),qual_(qual){
        updateType();
    }

    //DEBUG
    bool operator == (const edge& rhs)const{
        return from_==rhs.from_ && to_==rhs.to_ &&
                qual_ == rhs.qual_ && type_ == rhs.type_;
    }

    U* from()const{return from_;}
    U* to()const{return to_;}
    const double& qual(){return qual_;}

    bool valid()const{return type_=! invalid;}

    void update(const U* hasabs=0, const U* wasabs=0){
        qual_ = from()->calcScore(to());
        updateType();
    }

    void updateType();

    const etype& type() const {return type_;}

    void debug_print()const;

    /**
     * this implements the connection logic:
     * first order by: lowToLow, highToLow, invalid
     * then within subcategories ordered by decreasing edge score
     */
    static bool comparator(edge& e0, edge& e1);

private:
    U * from_, *to_;
    double qual_;
    etype type_;

};

template<class T>
void edge<T>::updateType(){
    if(! from_->shouldConnect(to_)){
        type_ = invalid;
    }
    else if(from_->isLow() && to_->isLow()){
        type_ = lowToLow;
    }
    else if(from_->isHigh() && to_->isLow()){
        type_ = highToLow;
    }
    else{
        throw std::invalid_argument("connection not allowed");
    }
}

template<class T>
void edge<T>::debug_print()const{
    std::string p = from_->name() + " to " + to_->name() +": " + std::to_string(qual_);
    if(type_==lowToLow)
        p += " lowToLow";
    else if (type_==highToLow)
        p += " highToLow";
    else if (type_==invalid)
        p += " invalid";
    std::cout << p << std::endl;
}

template<class T>
 bool edge<T>::comparator(edge<T>& e0, edge<T>& e1){ //return false if e0 should be before e1
    if(e1.type() == invalid)//to the end
        return true;
    if (e0.type() != e1.type()) {
        if(e0.type() == lowToLow) //lowToLow goes first
            return true;
        return false;
    }
    return e0.qual() > e1.qual(); //higher quality goes first
}


////////////////////////////////

template<class U>
class MergeGraph{
public:
    MergeGraph(){}

    void setVertices(const std::vector<U> v){
        es_.clear();
        vs_=v;
    }
    void addVertex(U&& v){es_.clear(); vs_.push_back(v);}

    void createEdges();//creates the edges, any vertex change will destroy them.

    int merge();

    const std::vector<U>& vertices()const{return vs_;}

private:

    void sortEdges(){
        std::sort(es_.begin(),es_.end(),edge<U>::comparator);
    }

    /**
     * also sorts them again
     */
    bool updateEdges( U* hasabs,  U* wasabs);

    std::vector<U> vs_;
    std::vector<edge<U> > es_;
};

template<class U>
int MergeGraph<U>::merge(){
    if(es_.size()<1)
        return 0;
    int nmerged=0;
    //size_t esize = es_.size();
    sortEdges();
    while(true){

#ifdef DEBUGCOUTS
        for(const auto& e: es_)
            e.debug_print();
        std::cout << "nmerged" << nmerged << std::endl;
#endif

        if((!es_.size()) || es_.at(0).type() == edge<U>::invalid)
            break; //depleted of good edges

        es_.at(0).from()->absorb(es_.at(0).to());

        bool changes = updateEdges(es_.at(0).from(),es_.at(0).to());

        if(!changes)
            break; //nothing done
        //esize = es_.size();

        nmerged++;
    }
    return nmerged;
}

template<class U>
bool MergeGraph<U>::updateEdges( U* hasabs,  U* wasabs){

        //for unit test
#ifdef DEBUGCHECKS
        auto escp = es_;
        createEdges();//redo. for now
        sortEdges();
        auto refedges = es_;
        es_=escp;
#endif

        //
        std::vector<edge<U> > newedges,oldedges;

        //performance improvement: this can be made part of the second bit already
        for(const auto& e: es_){
            if(e.to() != hasabs && e.to() != wasabs
                    && e.from() != hasabs && e.from() !=wasabs)
                oldedges.push_back(e);
        }

        for(const auto& vi:{hasabs,wasabs}){
            //vi participated
            if(vi->valid()){
                for( auto& vj:vs_){
                    if(vj.valid() && vi->shouldConnect(&vj) && vi != &vj)
                        newedges.push_back(edge<U>(vi,&vj,vi->calcScore(&vj)));
                }
            }
        }
        for( auto& vi:vs_){
            for(const auto& vj:{hasabs,wasabs}){
                //vj participated
                if(vj->valid() && vi.shouldConnect(vj) && &vi != vj)
                    newedges.push_back(edge<U>(&vi,vj,vi.calcScore(vj)));
            }
        }


        std::sort(newedges.begin(),newedges.end(),edge<U>::comparator);
        //now both are sorted

        //std::cout << "start " << es_.size() << ", old " << oldedges.size() << " new " << newedges.size() << std::endl;

        es_.clear();
        size_t newedgecounter=0;
        for(size_t i=0; i<oldedges.size(); i++){
            bool newfirst = false;
            if(newedgecounter < newedges.size())
                newfirst = edge<U>::comparator(newedges.at(newedgecounter), oldedges.at(i));
            if(newfirst){
                es_.push_back(newedges.at(newedgecounter));
                newedgecounter++;
                i--;
            }
            else{
                es_.push_back(oldedges.at(i));
            }
        }
        //in case there is something to append
        while(newedgecounter < newedges.size()){
            es_.push_back(newedges.at(newedgecounter));
            newedgecounter++;
        }

        return newedges.size() > 0;

#ifndef DEBUGCHECKS
        return newedges.size() > 0;
#else
        if(refedges != es_){

            std::cout << "new edges" << std::endl;
            for(const auto & e: newedges)
                e.debug_print();
            std::cout << "ref edges" << std::endl;
            for(const auto & e: refedges)
                e.debug_print();
            std::cout << "calc edges"<< std::endl;
            for(const auto & e: es_)
                e.debug_print();

            //check which ones are different
            for(const auto & e: es_){
                bool found=false;
                for(const auto & er: refedges){
                    if(er ==e)
                        found=true;
                    if(found)
                        break;
                }
                if(!found){
                    std::cout << "edge not found in ref edges or not same" << std::endl;
                    e.debug_print();
                }
            }

            for(const auto & e: refedges){
                bool found=false;
                for(const auto & er: es_){
                    if(er ==e)
                        found=true;
                    if(found)
                        break;
                }
                if(!found){
                    std::cout << "edge not found in direct edges or not same" << std::endl;
                    e.debug_print();
                }
            }

            throw std::runtime_error("bug");

        }
        return;
#endif
}

template<class U>
void MergeGraph<U>::createEdges(){
    es_.clear();
    for( auto& vi:vs_){
        if(!vi.valid())
            continue;
        for( auto& vj:vs_){
            if(!vj.valid())
                continue;
            if(vi.shouldConnect(&vj) && &vi != &vj)
                es_.push_back(edge<U>(&vi,&vj,vi.calcScore(&vj)));
        }
    }
}

#endif
