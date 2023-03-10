#ifndef __AIDEL_IMPL_H__
#define __AIDEL_IMPL_H__

#include "aidel.h"
#include "util.h"
#include "aidel_model.h"
#include "aidel_model_impl.h"
//#include "piecewise_linear_model.h"

namespace aidel {

template<class key_t, class val_t>
inline AIDEL<key_t, val_t>::AIDEL()
    : maxErr(0), learning_step(10000), learning_rate(0.1)
{
    //root = new root_type();
}

template<class key_t, class val_t>
inline AIDEL<key_t, val_t>::AIDEL(int _maxErr, int _learning_step, float _learning_rate)
    : maxErr(_maxErr), learning_step(_learning_step), learning_rate(_learning_rate)
{
    //root = new root_type();
}

template<class key_t, class val_t>
AIDEL<key_t, val_t>::~AIDEL(){
    //root = nullptr;
}

// ====================== train models ========================
template<class key_t, class val_t>
void AIDEL<key_t, val_t>::train(const std::vector<key_t> &keys, 
                                const std::vector<val_t> &vals, size_t _maxErr)
{
    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    size_t start = 0;
    size_t end = learning_step<keys.size()?learning_step:keys.size();
    while(start<end){
        //COUT_THIS("start:" << start<<" ,end: "<<end);
        plexmodel_type model;
        model.train(keys.begin()+start, end-start);
        size_t err = model.get_maxErr();
        append_model(model,keys.begin()+start, vals.begin()+start, end-start, err);
        start = end;
        end += learning_step;
        if(end>=keys.size())
            end = keys.size();
       
    }

    //root = new root_type(model_keys);
    COUT_THIS("[aidle] get models -> "<< model_keys.size());
    assert(model_keys.size()==aimodels.size());

    key_t min = keys.front();
    key_t max = keys.back();
    const unsigned numBins = 64; // each node will have 64 separate bins
    const unsigned maxError = 1; // the error of the index
    cht::Builder<key_t> chtb(min, max, numBins, maxError, false,false);
    for (const auto& key2 : keys) chtb.AddKey(key2);
    cht = chtb.Finalize();

}



template<class key_t, class val_t>
void AIDEL<key_t, val_t>::append_model(plexmodel_type &model, 
                                       const typename std::vector<key_t>::const_iterator &keys_begin, 
                                       const typename std::vector<val_t>::const_iterator &vals_begin, 
                                       size_t size, int err)
{
    key_t key = *(keys_begin+size-1); //last element

     
    assert(err<=maxErr);
    aidelmodel_type aimodel(model, keys_begin, vals_begin, size, maxErr);

    model_keys.push_back(key);
    aimodels.push_back(aimodel);
}

template<class key_t, class val_t>
typename AIDEL<key_t, val_t>::aidelmodel_type* AIDEL<key_t, val_t>::find_model(const key_t &key)
{

    cht::SearchBound bound = cht.GetSearchBound(key);
    size_t model_pos=-1;
    if(model_keys[bound.begin]>key) model_pos=bound.begin;
    for(int i=1;i<=2 && model_pos==-1;i++){
        if(bound.begin+i<model_keys.size()){
            if(model_keys[bound.begin+i]>key) model_pos=bound.begin+i;
        }
    }
    

  if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    return &aimodels[model_pos];
}


// ===================== print data =====================
template<class key_t, class val_t>
void AIDEL<key_t, val_t>::print_models()
{
    
    for(int i=0; i<model_keys.size(); i++){
        std::cout<<"model "<<i<<" ,key:"<<model_keys[i]<<" ->";
        aimodels[i].print_model();
    }

    
    
}

template<class key_t, class val_t>
void AIDEL<key_t, val_t>::self_check()
{
    for(int i=0; i<model_keys.size(); i++){
        aimodels[i].self_check();
    }
    
}


// =================== search the data =======================
template<class key_t, class val_t>
inline result_t AIDEL<key_t, val_t>::find(const key_t &key, val_t &val)
{   
    

    return find_model(key)[0].con_find_retrain(key, val);

}


// =================  scan ====================
template<class key_t, class val_t>
int AIDEL<key_t, val_t>::scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result)
{
    size_t remaining = n;
    size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
    if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    while(remaining>0 && model_pos < aimodels.size()){
        remaining = aimodels[model_pos].scan(key, remaining, result);
    }
    return remaining;
}



// =================== insert the data =======================
template<class key_t, class val_t>
inline result_t AIDEL<key_t, val_t>::insert(
        const key_t& key, const val_t& val)
{
    return find_model(key)[0].con_insert_retrain(key, val);
    //return find_model(key)[0].con_insert(key, val);
}


// ================ update =================
template<class key_t, class val_t>
inline result_t AIDEL<key_t, val_t>::update(
        const key_t& key, const val_t& val)
{
    return find_model(key)[0].update(key, val);
    //return find_model(key)[0].con_insert(key, val);
}


// ==================== remove =====================
template<class key_t, class val_t>
inline result_t AIDEL<key_t, val_t>::remove(const key_t& key)
{
    return find_model(key)[0].remove(key);
    //return find_model(key)[0].con_insert(key, val);
}

// ========================== using OptimalLPR train the model ==========================
/*template<class key_t, class val_t>
void AIDEL<key_t, val_t>::train_opt(const std::vector<key_t> &keys, 
                                    const std::vector<val_t> &vals, size_t _maxErr)
{
    using pair_type = typename std::pair<size_t, size_t>;

    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    segments = make_segmentation(keys.begin(), keys.end(), maxErr);
    COUT_THIS("[aidle] get models -> "<< segments.size());

   
}*/

/*template<class key_t, class val_t>
size_t AIDEL<key_t, val_t>::model_size(){
    return segments.size();
}*/



}    // namespace aidel


#endif