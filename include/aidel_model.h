#ifndef __AIDEL_MODEL_H__
#define __AIDEL_MODEL_H__

#include "lr_model.h"
#include "lr_model_impl.h"
#include "util.h"
#include "Bin_LL/LF_LL.h"

#define NOT_IMPLEMENTED                                            \
    {                                                              \
        std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl; \
        exit(-1);                                                  \
    }
namespace aidel
{

    template <class key_t, class val_t>
    class AidelModel
    {
    public:
        typedef LinearRegressionModel<key_t> lrmodel_type;
        typedef aidel::AidelModel<key_t, val_t> aidelmodel_type;

        typedef struct model_or_bin
        {
            typedef union pointer
            {
                aidelmodel_type *ai;
                Linked_List<key_t, val_t> *lfll;
            } pointer_t;
            pointer_t mob;
            bool volatile isbin = true; // true = lfll, false = ai
            volatile uint8_t locked = 0;
        } model_or_bin_t;

    public:
        inline AidelModel();
        ~AidelModel();
        AidelModel(lrmodel_type &lrmodel, const typename std::vector<key_t>::const_iterator &keys_begin,
                   const typename std::vector<val_t>::const_iterator &vals_begin,
                   size_t size, size_t _maxErr);
        inline size_t get_capacity();
        inline void print_model();
        void print_keys();
        void print_model_retrain();
        void self_check() { self_check_retrain();}
        void self_check_retrain();
        result_t update(const key_t &key, const val_t &val);
        bool remove(const key_t &key);
        val_t find_retrain(const key_t &key, val_t &val);
        bool insert_retrain(const key_t &key, const val_t &val);
        int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result, bool scan_bins=true);

    private:
        inline size_t predict(const key_t &key);
        inline size_t locate_in_levelbin(const key_t &key, const size_t pos);
        inline size_t find_lower(const key_t &key, const size_t pos);
        inline size_t linear_search(const key_t *arr, int n, key_t key);
        inline size_t find_lower_avx(const int *arr, int n, int key);
        inline size_t find_lower_avx(const int64_t *arr, int n, int64_t key);
        bool insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos);
        bool remove_model_or_bin(const key_t &key, const int bin_pos);

    private:
        lrmodel_type* model = nullptr;
        size_t maxErr = 64;
        size_t err = 0;
        key_t *keys = nullptr;
        val_t *vals = nullptr;
        bool *valid_flag = nullptr;
        std::atomic<model_or_bin_t *> *mobs_lf = nullptr;
        const size_t capacity;
    };

} // namespace aidel

#endif
