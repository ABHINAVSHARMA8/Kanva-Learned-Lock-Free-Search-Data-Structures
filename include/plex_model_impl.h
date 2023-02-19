
#include "plex_model.h"
#include "util.h"


template<class key_t>
inline PlexModel<key_t>::PlexModel(){}



template<class key_t>
PlexModel<key_t>::~PlexModel(){}


template<class key_t>
void PlexModel<key_t>::train(const typename std::vector<key_t>::const_iterator &it, size_t size,size_t maxErr)
{   
    this->maxErr=maxErr;
    std::vector<key_t> trainkeys(size);
    std::vector<size_t> positions(size);
    for(size_t i=0; i<size; i++){
        trainkeys[i]=*(it+i);
        positions[i] = i;
    }
    train(trainkeys, positions);
}


template<class key_t>
void PlexModel<key_t>::train(const std::vector<key_t> &keys,
                                         const std::vector<size_t> &positions)
{
    std::sort(keys.begin(), keys.end());
    this->keys=keys;

    // Build TS
    uint64_t min = keys.front();
    uint64_t max = keys.back();
    ts::Builder<uint64_t> tsb(min, max, /*spline_max_error=*/maxErr);

    for (const auto& key : keys) tsb.AddKey(key);
    this.ts = tsb.Finalize();
}

template<class key_t>
void PlexModel<key_t>::print_weights() const {
    std::cout<< "Plex model Weights"<<std::endl;
}

// ============ prediction ==============
template <class key_t>
size_t PlexModel<key_t>::predict(const key_t &key) const{
    // Search using TS
    ts::SearchBound bound = this.ts.GetSearchBound(key);
    auto start = std::begin(keys) + bound.begin,
        last = std::begin(keys) + bound.end;
    auto pos = std::lower_bound(start, last,key) - begin(keys);
    assert(keys[pos] == key);
    return pos;
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
    return this->maxErr;
}

template <class key_t>
size_t PlexModel<key_t>::max_error(const std::vector<key_t> &keys,
                                               const std::vector<size_t> &positions)
{
   return this->maxErr;
}



