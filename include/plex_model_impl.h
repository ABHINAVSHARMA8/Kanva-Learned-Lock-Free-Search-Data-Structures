#ifndef __PLEX_MODEL_IMPL_H__
#define __PLEX_MODEL_IMPL_H__


#include "plex_model.h"
#include "util.h"


template<class key_t>
inline PlexModel<key_t>::PlexModel(){} 


template<class key_t>
PlexModel<key_t>::~PlexModel(){}

template<class key_t>
inline PlexModel<key_t>::PlexModel(std::vector<double> &x,ts::TrieSpline<key_t> y)
{
    vec=x;
    ts=y;
}



template<class key_t>
void PlexModel<key_t>::train(const typename std::vector<key_t>::const_iterator &it, size_t size)
{   
    
    std::vector<key_t> trainkeys(size);
    std::vector<size_t> positions(size);
    for(size_t i=0; i<size; i++){
        trainkeys[i]=*(it+i);
        positions[i] = i;
    }
    train(trainkeys, positions);
}


template<class key_t>
void PlexModel<key_t>::train(const std::vector<key_t> &keys,const std::vector<size_t> &positions)
{   
    std::vector<double> model_keys(keys.size());
    for (size_t i = 0; i < keys.size(); i++) {
        model_keys[i] = keys[i];
    }
   // std::sort(model_keys.begin(),model_keys.end());
    
    vec=model_keys;
    //std::cout<<"Length of vec in training is "<<vec.size()<<std::endl;
    

    // Build TS
    uint64_t min = model_keys.front();
    uint64_t max = model_keys.back();
    ts::Builder<uint64_t> tsb(min, max,0);//CHANGE

    for (const auto& key : model_keys) tsb.AddKey(key);
    ts = tsb.Finalize();
}

template<class key_t>
void PlexModel<key_t>::print_weights() const {
    std::cout<< "Plex model Weights"<<std::endl;
}

// ============ prediction ==============

template <class key_t>
size_t PlexModel<key_t>::predict(const key_t &key) const{
    
    double model_key = key;
    if(model_key<vec[0]) return 0;
    if(model_key>vec[vec.size()-1]) return vec.size()-1;
    
    ts::SearchBound bound = ts.GetSearchBound(model_key);
    if(vec[bound.begin]==model_key) return bound.begin;
    if(vec[bound.begin+1]==model_key) return bound.begin+1;
    if(bound.begin+2<vec.size()){
     if(vec[bound.begin+2]==model_key) return bound.begin+2;
    }
    
    if(bound.begin+2<vec.size()){
        if(vec[bound.begin+2]<model_key) 
            return bound.begin+2;
    }
    if(vec[bound.begin+1]<model_key) 
        return bound.begin+1;
    if(vec[bound.begin]<model_key) 
        return bound.begin;
    return 0;
}

template <class key_t>
std::vector<size_t> PlexModel<key_t>::predict(const std::vector<key_t> &keys) const
{
    assert(keys.size()>0);
    std::vector<size_t> pred(keys.size());
    for(int i=0; i<keys.size(); i++)
    {
        pred[i] = predict(keys[i]);
    }
    return pred;
}

// =========== max__error ===========
template <class key_t>
size_t PlexModel<key_t>::max_error(
    const typename std::vector<key_t>::const_iterator &keys_begin,
    uint32_t size) {
    return maxErr;
}

template <class key_t>
size_t PlexModel<key_t>::max_error(const std::vector<key_t> &keys,
                                               const std::vector<size_t> &positions)
{
   return maxErr;
}


#endif
