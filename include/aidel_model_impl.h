#ifndef __AIDEL_MODEL_IMPL_H__
#define __AIDEL_MODEL_IMPL_H__

#include "aidel_model.h"
#include "util.h"

namespace aidel
{

    template <class key_t, class val_t>
    AidelModel<key_t, val_t>::AidelModel()
    {
        model = nullptr;
        maxErr = 64;
        err = 0;
        keys = nullptr;
        levelbins = nullptr;
        capacity = 0;
    }

    template <class key_t, class val_t>
    AidelModel<key_t, val_t>::~AidelModel()
    {
        if (model)
            model = nullptr;
        if (levelbins)
            levelbins = nullptr;
    }

    template <class key_t, class val_t>
    AidelModel<key_t, val_t>::AidelModel(plexmodel_type &plexmodel,
                                         const typename std::vector<key_t>::const_iterator &keys_begin,
                                         const typename std::vector<val_t>::const_iterator &vals_begin,
                                         size_t size, size_t _maxErr) : maxErr(_maxErr), capacity(size)
    {
        model = new plexmodel_type(plexmodel.get_vec(), plexmodel.get_ts());
        keys = (key_t *)malloc(sizeof(key_t) * size);
        vals = (val_t *)malloc(sizeof(val_t) * size);
        valid_flag = (bool *)malloc(sizeof(bool) * size);
        for (int i = 0; i < size; i++)
        {
            keys[i] = *(keys_begin + i);
            vals[i] = *(vals_begin + i);
            valid_flag[i] = true;
        }
        mobs_lf = (std::atomic<model_or_bin_t *> *)malloc(sizeof(std::atomic<model_or_bin_t *>) * (size + 1));
        for (int i = 0; i < size + 1; i++)
        {
            mobs_lf[i] = nullptr;
        }
    }

    template <class key_t, class val_t>
    inline size_t AidelModel<key_t, val_t>::get_capacity()
    {
        return capacity;
    }

    // =====================  print =====================
    template <class key_t, class val_t>
    inline void AidelModel<key_t, val_t>::print_model()
    {
        std::cout << " capacity:" << capacity << " -->";
        model->print_weights();
        print_keys();
    }

    template <class key_t, class val_t>
    void AidelModel<key_t, val_t>::print_keys()
    {
        if (levelbins[0])
            levelbins[0]->print(std::cout);
        for (size_t i = 0; i < capacity; i++)
        {
            std::cout << "keys[" << i << "]: " << keys[i] << std::endl;
            if (levelbins[i + 1])
                levelbins[i + 1]->print(std::cout);
        }
    }

    template <class key_t, class val_t>
    void AidelModel<key_t, val_t>::print_model_retrain()
    {
        std::cout << "[print aimodel] capacity:" << capacity << " -->";
        model->print_weights();
        if (mobs_lf[0])
        {
            if (mobs_lf[0]->isbin)
            {
                mobs_lf[0]->mob.lb->print(std::cout);
            }
            else
            {
                mobs_lf[0]->mob.ai->print_model_retrain();
            }
        }
        for (size_t i = 0; i < capacity; i++)
        {
            std::cout << "keys[" << i << "]: " << keys[i] << std::endl;
            if (mobs_lf[i + 1])
            {
                if (mobs_lf[i + 1]->isbin)
                {
                    mobs_lf[i + 1]->mob.lb->print(std::cout);
                }
                else
                {
                    mobs_lf[i + 1]->mob.ai->print_model_retrain();
                }
            }
        }
    }
#if 0
    template <class key_t, class val_t>
    void AidelModel<key_t, val_t>::self_check()
    {
        NOT_IMPLEMENTED;
#if 0
    if(levelbins[0]) levelbins[0]->self_check();
    for(size_t i=1; i<capacity; i++){
        assert(keys[i]>keys[i-1]);
        if(levelbins[i]) levelbins[i]->self_check();
    }
    if(levelbins[capacity]) levelbins[capacity]->self_check();
#endif
    }
#endif
    template <class key_t, class val_t>
    void AidelModel<key_t, val_t>::self_check_retrain()
    {
        for (size_t i = 1; i < capacity; i++)
        {
            assert(keys[i] > keys[i - 1]);
        }
        for (size_t i = 0; i <= capacity; i++)
        {
            model_or_bin_t *mob = mobs[i];
            if (mob)
            {
                if (mob->isbin)
                {
                    mob->mob.lb->self_check();
                }
                else
                {
                    mob->mob.ai->self_check_retrain();
                }
            }
        }
    }

    // ============================ search =====================

    template <class key_t, class val_t>
    val_t AidelModel<key_t, val_t>::find_retrain(const key_t &key, val_t &val)
    {
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos);
        if (key == keys[pos])
        {
            return vals[pos];
        }
        int bin_pos = key < keys[pos] ? pos : (pos + 1);
        model_or_bin_t *mob;
        mob = mobs_lf[bin_pos].load(std::memory_order_seq_cst);

        if (mob == nullptr)
            return -1;
        if (mob->isbin)
            return mob->mob.lflb->search(key);
        else
            return mob->mob.ai->find_retrain(key, val);
    }

    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::find(const key_t &key, val_t &val)
    {
        NOT_IMPLEMENTED;
#if 0
    size_t pos = predict(key);
    //pos = find_lower(key, pos);
    //pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return true;
        }
        return false; 
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    if(levelbins[bin_pos]==nullptr) return false; 
    typename levelbin_type::iterator it = levelbins[bin_pos]->find(key);
    if(it!=levelbins[bin_pos]->end()){
        val = it.data();
        return true;
    }
    return false;
#endif
    }

    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::con_find(const key_t &key, val_t &val)
    {
        NOT_IMPLEMENTED;
#if 0
    size_t pos = predict(key);
    //pos = find_lower(key, pos);
   // pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return true;
        }
        return false;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    if(levelbins[bin_pos]==nullptr) return false;
    return levelbins[bin_pos]->con_find(key, val);
#endif
    }

#if 0
    template <class key_t, class val_t>
    result_t AidelModel<key_t, val_t>::con_find_retrain(const key_t &key, val_t &val)
    {
        NOT_IMPLEMENTED;
    size_t pos = predict(key);
   // pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->con_find_retrain(key, val);
        /*while(res == result_t::retrain) {
            while(mob->locked == 1)
                ;
            res = mob->mob.lb->con_find_retrain(key, val);
        }
        return res;*/
    } else{
        res = mob->mob.ai->con_find_retrain(key, val);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;
    }
#endif

    template <class key_t, class val_t>
    inline size_t AidelModel<key_t, val_t>::predict(const key_t &key)
    {
        size_t index_pos = model->predict(key);
        return index_pos < capacity ? index_pos : capacity - 1;
    }

    template <class key_t, class val_t>
    inline size_t AidelModel<key_t, val_t>::locate_in_levelbin(const key_t &key, const size_t pos)
    {
        // predict
        // size_t index_pos = model->predict(key);
        size_t index_pos = pos;
        size_t upbound = capacity - 1;
        // index_pos = index_pos <= upbound? index_pos:upbound;

        // search
        size_t begin, end, mid;
        if (key > keys[index_pos])
        {
            begin = index_pos + 1 < upbound ? (index_pos + 1) : upbound;
            end = begin + maxErr < upbound ? (begin + maxErr) : upbound;
        }
        else
        {
            end = index_pos;
            begin = end > maxErr ? (end - maxErr) : 0;
        }

        assert(begin <= end);
        while (begin != end)
        {
            mid = (end + begin + 2) / 2;
            if (keys[mid] <= key)
            {
                begin = mid;
            }
            else
                end = mid - 1;
        }
        return begin;
    }

    // ======================= update =========================
    template <class key_t, class val_t>
    result_t AidelModel<key_t, val_t>::update(const key_t &key, const val_t &val)
    {
        NOT_IMPLEMENTED;
#if 0
    size_t pos = predict(key);
   // pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            vals[pos] = val;
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->update(key, val);
        /*while(res == result_t::retrain) {
            while(mob->locked == 1)
                ;
            res = mob->mob.lb->update(key, val);
        }
        return res;*/
    } else{
        res = mob->mob.ai->update(key, val);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;
#endif
    }

    // =============================== insert =======================
    template <class key_t, class val_t>
    inline bool AidelModel<key_t, val_t>::con_insert(const key_t &key, const val_t &val)
    {
        NOT_IMPLEMENTED;
#if 0
    size_t pos = predict(key);
   // pos = locate_in_levelbin(key, pos);

    if(key == keys[pos]){
        if(valid_flag[pos]){
            return false;
        } else{
            valid_flag[pos] = true;
            vals[pos] = val;
            return true;
        }
    }
    int bin_pos = pos;
    bin_pos = key<keys[bin_pos]?bin_pos:(bin_pos+1);
    if(!levelbins[bin_pos]) {
        levelbin_type *lb = new levelbin_type();
        memory_fence();
        if(levelbins[bin_pos]) {
            delete lb;
            return levelbins[bin_pos]->con_insert(key, val);
        }
        levelbins[bin_pos] = lb;
    }
    assert(levelbins[bin_pos]!=nullptr);   
    return levelbins[bin_pos]->con_insert(key, val);
#endif
    }

#if 0
    template <class key_t, class val_t>
    result_t AidelModel<key_t, val_t>::con_insert_retrain(const key_t &key, const val_t &val)
    {
        NOT_IMPLEMENTED;
    size_t pos = predict(key);
    //size_t pos = locate_in_levelbin(key, pos1);
    //std::cout<<"insert "<<key<<" "<<keys[pos]<<std::endl;
   // std::cout<<pos-pos1<<std::endl;
   //std::cout<<key<<" "<<keys[pos]<<std::endl;
    if(key == keys[pos]){
        if(valid_flag[pos]){
            return result_t::failed;
        } else{
            valid_flag[pos] = true;
            vals[pos] = val;
            return result_t::ok;
        }
    }
    int bin_pos = pos;
    bin_pos = key<keys[bin_pos]?bin_pos:(bin_pos+1);
    return insert_model_or_bin(key, val, bin_pos);
    }
#endif

    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::insert_retrain(const key_t &key, const val_t &val)
    {
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos);

        if (key == keys[pos])
        {
            if (vals[pos] == -1)
                return false;
            else
                return true;
        }

        int bin_pos = pos;
        bin_pos = key < keys[bin_pos] ? bin_pos : (bin_pos + 1);
        return insert_model_or_bin(key, val, bin_pos);
    }

#if 0
template<class key_t, class val_t>
result_t AidelModel<key_t, val_t>::insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos)
{
    // insert bin or model
    model_or_bin_t *mob = mobs[bin_pos];
    if(mob==nullptr){
        mob = new model_or_bin_t();
        mob->lock();
        mob->mob.lb = new levelbin_type();
        mob->isbin = true;
        memory_fence();
        if(mobs[bin_pos]){
            delete mob;
            return insert_model_or_bin(key, val, bin_pos);
        }
        mobs[bin_pos] = mob;
    } else{
        mob->lock();
    }
    assert(mob!=nullptr);
    assert(mob->locked == 1);
    result_t res = result_t::failed;
    if(mob->isbin) {           // insert into bin
        res = mob->mob.lb->con_insert_retrain(key, val);
        if(res == result_t::retrain){
            //COUT_THIS("[aimodel] Need Retrain: " << key);
            // resort the data and train the model
            std::vector<key_t> retrain_keys;
            std::vector<val_t> retrain_vals;
            mob->mob.lb->resort(retrain_keys, retrain_vals);
            plexmodel_type model;
            model.train(retrain_keys.begin(), retrain_keys.size());
            size_t err = model.get_maxErr();
            aidelmodel_type *ai = new aidelmodel_type(model, retrain_keys.begin(), retrain_vals.begin(), retrain_keys.size(), err);
            
            memory_fence();
            delete mob->mob.lb;
            mob->mob.ai = ai;
            mob->isbin = false;
            res = ai->con_insert_retrain(key, val);
            mob->unlock();
            return res;
        }
    } else{                   // insert into model
        res = mob->mob.ai->con_insert_retrain(key, val);
    }
    mob->unlock();
    return res;
}
#endif

    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos)
    {
        model_or_bin_t *mob = mobs_lf[bin_pos];
    retry:
        if (mob == nullptr)
        {
            model_or_bin_t *new_mob = new model_or_bin_t();
            new_mob->mob.lflb = new Bin<key_t, val_t>();
            new_mob->isbin = true;
            if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob, std::memory_order_seq_cst, std::memory_order_seq_cst))
                goto retry;
            mob = new_mob;
        }
        assert(mob != nullptr);
        if (mob->isbin)
        { // insert into bin
            bool res = mob->mob.lflb->insert(key, val);
            if (!res)
            {
                std::vector<key_t> retrain_keys;
                std::vector<val_t> retrain_vals;
                mob->mob.lflb->collect(retrain_keys, retrain_vals);
                plexmodel_type model;
                model.train(retrain_keys.begin(), retrain_keys.size());
                size_t err = model.get_maxErr();
                aidelmodel_type *ai = new aidelmodel_type(model, retrain_keys.begin(), retrain_vals.begin(), retrain_keys.size(), err);
                model_or_bin_t *new_mob = new model_or_bin_t();
                new_mob->mob.ai = ai;
                new_mob->isbin = false;
                if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob))
                {
                    goto retry;
                }
                res = ai->insert_retrain(key, val);
                return res;
            }
        }
        else
        { // insert into model
            return mob->mob.ai->insert_retrain(key, val);
        }
        return false;
    }

    // ========================== remove =====================
    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::remove(const key_t &key)
    {
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos); // TODO(Abhinav): Commented originally, but uncommented in Kanva
        if (key == keys[pos])
        {
            vals[pos] = -1;
            return true;
        }
        int bin_pos = key < keys[pos] ? pos : (pos + 1);
        return remove_model_or_bin(key, bin_pos);
    }

    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::remove_model_or_bin(const key_t &key, const int bin_pos)
    {
        // TODO(Abhinav) : Check this 
    retry:
        model_or_bin_t *mob = mobs_lf[bin_pos];
        if (mob == nullptr)
            return false;
        if (mob->isbin)
        {
            int res;
            res = mob->mob.lflb->remove(key);
            if (res == -2)
                return false;
            else if (res == -1)
            {
                std::vector<key_t> retrain_keys;
                std::vector<val_t> retrain_vals;
                mob->mob.lflb->collect(retrain_keys, retrain_vals);
                plexmodel_type model;
                model.train(retrain_keys.begin(), retrain_keys.size());
                size_t err = model.get_maxErr();
                aidelmodel_type *ai = new aidelmodel_type(model, retrain_keys.begin(), retrain_vals.begin(), retrain_keys.size(), err);
                model_or_bin_t *new_mob = new model_or_bin_t();
                new_mob->mob.ai = ai;
                new_mob->isbin = false;
                if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob))
                {
                    goto retry;
                }
                return ai->remove(key);
            }
            else
                return true;
        }
        else
        {
            return mob->mob.ai->remove(key);
        }
        return false;
    }

    // ========================== scan ===================
    #if 0
    template <class key_t, class val_t>
    int AidelModel<key_t, val_t>::scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result)
    {
        size_t remaining = n;

        size_t pos = predict(key);
        // pos = locate_in_levelbin(key, pos);
        while (remaining > 0 && pos <= capacity)
        {
            if (pos < capacity && keys[pos] >= key)
            {
                result.push_back(std::pair<key_t, val_t>(keys[pos], vals[pos]));
                remaining--;
                if (remaining <= 0)
                    break;
            }
            if (mobs[pos] != nullptr)
            {
                model_or_bin_t *mob = mobs[pos];
                if (mob->isbin)
                {
                    remaining = mob->mob.lb->scan(key, remaining, result);
                }
                else
                {
                    remaining = mob->mob.ai->scan(key, remaining, result);
                }
            }
            pos++;
        }
        return remaining;
    }

    // ======================== resort data for retraining ===================
    template <class key_t, class val_t>
    void AidelModel<key_t, val_t>::resort(std::vector<key_t> &keys, std::vector<val_t> &vals)
    {
        typename levelbin_type::iterator it;
        for (size_t i = 0; i <= capacity; i++)
        {
            if (levelbins[i])
            {
                for (it = levelbins[i]->begin(); it != levelbins[i]->end(); it++)
                {
                    keys.push_back(it.key());
                    vals.push_back(it.data());
                }
            }
        }
    }
#endif
} // namespace aidel

#endif
