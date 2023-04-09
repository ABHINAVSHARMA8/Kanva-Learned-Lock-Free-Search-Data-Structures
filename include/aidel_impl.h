#ifndef __AIDEL_IMPL_H__
#define __AIDEL_IMPL_H__

#include "aidel.h"
#include "util.h"
#include "aidel_model.h"
#include "aidel_model_impl.h"
#include "piecewise_linear_model.h"

namespace aidel {

template<class key_t, class val_t>
inline AIDEL<key_t, val_t>::AIDEL()
    : maxErr(0), learning_step(10000), learning_rate(0.1)
{
    
}

template<class key_t, class val_t>
inline AIDEL<key_t, val_t>::AIDEL(int _maxErr, int _learning_step, float _learning_rate)
    : maxErr(_maxErr), learning_step(_learning_step), learning_rate(_learning_rate)
{
    
}

template<class key_t, class val_t>
AIDEL<key_t, val_t>::~AIDEL(){
    
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
        
        lrmodel_type model;
        model.train(keys.begin()+start, end-start);
        
        size_t err = model.get_maxErr();
       // std::cout<<__LINE__<<" "<<err<<std::endl;
        if(err == maxErr) {
            append_model(model, keys.begin()+start, vals.begin()+start, end-start, err);
            //std::cout<<__LINE__<<std::endl;
        }else if(err < maxErr) {
            if(end>=keys.size()){
                append_model(model, keys.begin()+start, vals.begin()+start, end-start, err);
                //std::cout<<__LINE__<<std::endl;
                break;
            }
            end += learning_step;
            if(end>keys.size()){
                end = keys.size();
                //std::cout<<__LINE__<<std::endl;
            }
            continue;
        } else {
            //std::cout<<__LINE__<<" "<<end-start<<" "<<int(learning_step*learning_rate)<<std::endl;
            size_t offset = backward_train(keys.begin()+start, vals.begin()+start, end-start, int(learning_step*learning_rate));
            //std::cout<<__LINE__<<std::endl;
			end = start + offset;
        }
        
        start = end;
        end += learning_step;
        if(end>=keys.size())
            end = keys.size();
       
    }

    
    COUT_THIS("[aidle] get models -> "<< model_keys.size());
    assert(model_keys.size()==aimodels.size());

    key_t min1 = model_keys.front();
    key_t max1 = model_keys.back();
    const unsigned numBins = 128; // each node will have 128 separate bins
    const unsigned maxError = 1; // the error of the index
    cht::Builder<key_t> chtb(min1, max1, numBins, maxError, false,false);
    for (const auto& key2 : model_keys) chtb.AddKey(key2);
    cht = chtb.Finalize();

    

}
template<class key_t, class val_t>
size_t AIDEL<key_t, val_t>::backward_train(const typename std::vector<key_t>::const_iterator &keys_begin, 
                                           const typename std::vector<val_t>::const_iterator &vals_begin,
                                           uint32_t size, int step)
{   
    if(size<=10){
        step = 1;
    } else {
        while(size<=step){
            step = int(step*learning_rate);
        }
    }
    assert(step>0);
    size_t start = 0;
    size_t end = size-step;
    while(end>0){
        lrmodel_type model;
        model.train(keys_begin, end);
        size_t err = model.get_maxErr();
        if(err<=maxErr){
            append_model(model, keys_begin, vals_begin, end, err);
            return end;
        }
        if(end>step)
        end -= step;
        else break;
    }
    end = backward_train(keys_begin, vals_begin, end, int(step*learning_rate));
	return end;
}




template<class key_t, class val_t>
void AIDEL<key_t, val_t>::append_model(lrmodel_type &model, 
                                       const typename std::vector<key_t>::const_iterator &keys_begin, 
                                       const typename std::vector<val_t>::const_iterator &vals_begin, 
                                       size_t size, int err)
{
    key_t key = *(keys_begin+size-1); //last element
    
    // set learning_step
    int n = size/10;
    learning_step = 1;
    while(n!=0){
        n/=10;
        learning_step*=10;
    }
     
    assert(err<=maxErr);
    aidelmodel_type aimodel(model, keys_begin, vals_begin, size, maxErr);

    model_keys.push_back(key);
    aimodels.push_back(aimodel);
}

template<class key_t, class val_t>
typename AIDEL<key_t, val_t>::aidelmodel_type* AIDEL<key_t, val_t>::find_model(const key_t &key)
{
    size_t model_key=key;
    cht::SearchBound bound = cht.GetSearchBound(model_key);
    size_t model_pos=-1;
    
    for(int i=0;i<=2 && model_pos==-1;i++){
        if(bound.begin+i<model_keys.size()){
            if(model_keys[bound.begin+i]>=model_key) model_pos=bound.begin+i;
        }
    }
    
    
    
    
   
    //size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
    //std::cout<<model_pos2<<" "<<model_pos<<std::endl;

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
    // TODO(Abhinav): 
    //return find_model(key)[0].con_find_retrain(key, val);
    return find_model(key)[0].find_retrain(key, val) ? result_t::ok : result_t::failed;

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
    // TODO(Abhinav)
    return find_model(key)[0].insert_retrain(key, val) ? result_t::ok : result_t::failed;
    //return find_model(key)[0].con_insert_retrain(key, val);
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
    // TODO(Abhinav): CHeck
    return find_model(key)[0].remove(key) ? result_t::ok : result_t::failed;
    //return find_model(key)[0].remove(key);
    //return find_model(key)[0].con_insert(key, val);
}

// ========================== using OptimalLPR train the model ==========================
template<class key_t, class val_t>
void AIDEL<key_t, val_t>::train_opt(const std::vector<key_t> &keys, 
                                    const std::vector<val_t> &vals, size_t _maxErr)
{
    using pair_type = typename std::pair<size_t, size_t>;

    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    segments = make_segmentation(keys.begin(), keys.end(), maxErr);
    COUT_THIS("[aidle] get models -> "<< segments.size());

   
}

template<class key_t, class val_t>
size_t AIDEL<key_t, val_t>::model_size(){
    return segments.size();
}



}    // namespace aidel


#endif
