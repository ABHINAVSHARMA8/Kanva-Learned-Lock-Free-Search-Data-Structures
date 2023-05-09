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
        capacity = 0;
    }

    template <class key_t, class val_t>
    AidelModel<key_t, val_t>::~AidelModel()
    {
        model = nullptr;
        for (int i = 0; i < capacity; i++)
        {
            mobs_lf[i] = nullptr;
        }
        mobs_lf = nullptr;
    }

    template <class key_t, class val_t>
    AidelModel<key_t, val_t>::AidelModel(lrmodel_type &lrmodel,
                                         const typename std::vector<key_t>::const_iterator &keys_begin,
                                         const typename std::vector<val_t>::const_iterator &vals_begin,
                                         size_t size, size_t _maxErr) : maxErr(_maxErr), capacity(size)
    {
        model = new lrmodel_type(lrmodel.get_weight0(), lrmodel.get_weight1());
        keys = (key_t *)malloc(sizeof(key_t) * size);
        vals = (val_t *)malloc(sizeof(val_t) * size);
        // valid_flag = (bool *)malloc(sizeof(bool) * size);
        for (int i = 0; i < size; i++)
        {
            keys[i] = *(keys_begin + i);
            vals[i] = *(vals_begin + i);
            // valid_flag[i] = true;
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
        if (mobs_lf[0])
        {
            if (mobs_lf[0]->isbin)
                mobs_lf[0]->mob.lb->print(std::cout);
            else
                mobs_lf[0]->mob.ai->print_keys();
        }

        for (size_t i = 0; i < capacity; i++)
        {
            std::cout << "keys[" << i << "]: " << keys[i] << std::endl;
            if (mobs_lf[i + 1])
            {
                if (mobs_lf[i + 1]->isbin)
                    mobs_lf[i + 1]->mob.lb->print(std::cout);
                else
                    mobs_lf[i + 1]->mob.ai->print_keys();
            }
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

    template <class key_t, class val_t>
    void AidelModel<key_t, val_t>::self_check_retrain()
    {
        for (size_t i = 1; i < capacity; i++)
        {
            assert(keys[i] > keys[i - 1]);
        }
        for (size_t i = 0; i <= capacity; i++)
        {
            model_or_bin_t *mob = mobs_lf[i];
            if (mob)
            {
                if (mob->isbin)
                {
                    mob->mob.lfll->self_check(i ? keys[i-1] : std::numeric_limits<key_t>::min());
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
            return mob->mob.lfll->search(key);
        else
            return mob->mob.ai->find_retrain(key, val);
    }

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
        int index_pos = pos;
        int upbound = capacity - 1;
        // index_pos = index_pos <= upbound? index_pos:upbound;

        // search
        /*
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
        */
        int i = index_pos;int e=0;
    if(i<0 || i>upbound) return -1;
    while(i<=upbound && keys[i] < key){
        i+=(pow(2,e));
        e++;
    }
    
    if(i>upbound) i=upbound;
    int right=i;
    e=1;
    if(i<0 || i>upbound) return -1;
    while(i>0 && keys[i] > key){
        i-=(pow(2,e));
        e++;
    }

    if(i<0) i=0;

    int left=i;
    //std::cout<<left<<" "<<right<<std::endl;
    for(int j=left;j<=right;j++){
        if(keys[j] == key) return j;
    }
    return 0;
    }

    // ======================= update =========================
    template <class key_t, class val_t>
    result_t AidelModel<key_t, val_t>::update(const key_t &key, const val_t &val)
    {
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos);

        if (key == keys[pos])
        {
            if (vals[pos] != -1)
            {
                vals[pos] = val;
                return result_t::ok;
            }
            return result_t::failed;
        }
        int bin_pos = pos;
        bin_pos = key < keys[bin_pos] ? bin_pos : (bin_pos + 1);
        model_or_bin_t *mob = mobs_lf[bin_pos];
        if (mob == nullptr)
            return result_t::failed;
        result_t res = result_t::failed;
        if (mob->isbin)
        {
            res = mob->mob.lfll->insert(key, val, true);
        }
        else
        {
            res = mob->mob.ai->update(key, val);
        }
        assert(res != result_t::retrain);
        return res;
    }

    // =============================== insert =======================
    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::insert_retrain(const key_t &key, const val_t &val)
    {
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos);

        if (key == keys[pos])
        {
            return vals[pos] != -1;
        }

        int bin_pos = pos;
        bin_pos = key < keys[bin_pos] ? bin_pos : (bin_pos + 1);
        return insert_model_or_bin(key, val, bin_pos);
    }

    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos)
    {
        if(bin_pos != 0) assert(keys[bin_pos-1] < key); // TODO: Remove
        retry:
         model_or_bin_t *mob = mobs_lf[bin_pos];
    
        if (mob == nullptr)
        {
            model_or_bin_t *new_mob = new model_or_bin_t();
            new_mob->mob.lfll = new Linked_List<key_t, val_t>();
            new_mob->isbin = true;
            if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob, std::memory_order_seq_cst, std::memory_order_seq_cst))
                goto retry;
            mob = new_mob;
        }
        assert(mob != nullptr);
        if (mob->isbin)
        { // insert into bin
            // bool res = mob->mob.lfll->insert(key, val);
            result_t res = mob->mob.lfll->insert(key, val);
            if (res == result_t::retrain)
            {
                std::vector<key_t> retrain_keys;
                std::vector<val_t> retrain_vals;
                mob->mob.lfll->collect(retrain_keys, retrain_vals);
                lrmodel_type model;
                model.train(retrain_keys.begin(), retrain_keys.size());
                size_t err = model.get_maxErr();
                aidelmodel_type *ai = new aidelmodel_type(model, retrain_keys.begin(), retrain_vals.begin(), retrain_keys.size(), 0);
                model_or_bin_t *new_mob = new model_or_bin_t();
                new_mob->mob.ai = ai;
                new_mob->isbin = false;
                if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob))
                {
                    goto retry;
                }
                return ai->insert_retrain(key, val);
            }
            return (res == result_t::ok);
        }

        return mob->mob.ai->insert_retrain(key, val);
    }

    // ========================== remove =====================
    template <class key_t, class val_t>
    bool AidelModel<key_t, val_t>::remove(const key_t &key)
    {
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos);
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
            res = mob->mob.lfll->remove(key);
            if (res == -2)
                return false;
            else if (res == -1)
            {
                /*std::vector<key_t> retrain_keys;
                std::vector<val_t> retrain_vals;
                mob->mob.lfll->collect(retrain_keys, retrain_vals);
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
                return ai->remove(key);*/
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

    template<class key_t, class val_t>
    int AidelModel<key_t, val_t>::scan(const key_t &key, const size_t n, std::vector< std::pair<key_t, val_t> > &result, bool scan_bins)
    {
        assert(key >= 0);
        size_t remaining = n;
        size_t pos = predict(key);
        pos = locate_in_levelbin(key, pos);
        //int bin_pos = key <= keys[pos] ? pos : (pos + 1);
        //pos = key <= keys[pos] ? pos : (pos + 1);
        // TODO@Abhinav: When key doesn't match, shouldn't it start from the bin at that index
        
        while (remaining > 0 && pos <= capacity)
        {
            if (pos < capacity && keys[pos] >= key)
            {
                val_t value = vals[pos];
                if (value != -1)
                {
                    result.push_back({keys[pos], value});
                    remaining--;
                    if (remaining <= 0)
                        break;
                }
            }
            pos++;
            if(pos > capacity) break;
            if (mobs_lf[pos].load(std::memory_order_seq_cst))
            {
                model_or_bin_t *mob;
                mob = mobs_lf[pos];
                if (mob->isbin)
                {
                    if(scan_bins) 
                        remaining = mob->mob.lfll->range_query(key, remaining, result);
                }
                else
                {
                    remaining = mob->mob.ai->scan(key, remaining, result, scan_bins);
                }
            }
            //pos++;
        }
        return remaining;
    }
} // namespace aidel

#endif
