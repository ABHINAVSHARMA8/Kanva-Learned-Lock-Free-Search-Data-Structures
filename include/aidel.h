#ifndef __AIDEL_H__
#define __AIDEL_H__

#include "util.h"
/*#include "plex_model.h"
#include "plex_model_impl.h"*/
#include "lr_model.h"
#include "lr_model_impl.h"
#include "aidel_model.h"
#include "aidel_model_impl.h"

#include "piecewise_linear_model.h"
#include "Uruv/VersionTracker/TrackerList.h"
#include<atomic>


TrackerList version_tracker;




namespace aidel {

template<class key_t, class val_t>
class AIDEL{
public:
    typedef aidel::AidelModel<key_t, val_t> aidelmodel_type;
    typedef LinearRegressionModel<key_t> lrmodel_type;
    typedef typename OptimalPiecewiseLinearModel<key_t, size_t>::CanonicalSegment canonical_segment;
    typedef record_manager<
        reclaimer_debra<key_t>,
        allocator_new<key_t>,
        pool_none<key_t>,
        ll_Node<key_t, val_t>>
        ll_record_manager_t;
    typedef record_manager<
        reclaimer_debra<key_t>,
        allocator_new<key_t>,
        pool_none<key_t>,
        Vnode<key_t>>
        vnode_record_manager_t;    
        
    ll_record_manager_t *llRecMgr;
    vnode_record_manager_t *vnodeRecMgr;
   
   // typedef aidel::LevelIndex<key_t> root_type;

public:
    inline AIDEL(int num_threads);
    //inline AIDEL(int _maxErr, int _learning_step, float _learning_rate, ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr);
    ~AIDEL();
    void train(const std::vector<key_t> &keys, const std::vector<val_t> &vals, size_t _maxErr, thread_id_t tid);
    void train_opt(const std::vector<key_t> &keys, const std::vector<val_t> &vals, size_t _maxErr, thread_id_t tid);
    //void retrain(typename root_type::iterator it);
    void print_models(thread_id_t tid);
    void self_check(thread_id_t tid);
    
    
    inline result_t find(const key_t &key, val_t &val, thread_id_t tid);
    inline result_t insert(const key_t &key, const val_t &val, thread_id_t tid);
    inline result_t update(const key_t &key, const val_t &val, thread_id_t tid);
    inline result_t remove(const key_t &key, thread_id_t tid);
    int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result, thread_id_t tid);
    size_t model_size(thread_id_t tid);


private:
    size_t backward_train(const typename std::vector<key_t>::const_iterator &keys_begin,
                          const typename std::vector<val_t>::const_iterator &vals_begin, 
                          uint32_t size, int step, thread_id_t tid);
    void append_model(lrmodel_type &model, const typename std::vector<key_t>::const_iterator &keys_begin, 
                      const typename std::vector<val_t>::const_iterator &vals_begin, 
                      size_t size, int err, thread_id_t tid);
    aidelmodel_type* find_model(const key_t &key, thread_id_t tid);
    int locate_in_levelbin(key_t key, int model_pos, thread_id_t tid);
    

private:
    std::vector<key_t> model_keys;
    std::vector<aidelmodel_type> aimodels;
    //root_type* root = nullptr;
    std::vector<canonical_segment> segments;
    int maxErr = 64;
    int learning_step = 1000;
    float learning_rate = 0.1;

    
    //cht::CompactHistTree<key_t> cht;
    

};

}   // namespace aidel


#endif
