#ifndef __PLEX_MODEL_H__
#define __PLEX_MODEL_H__


#include "plex/ts/builder.h"
#include "plex/ts/common.h"
#include "plex/ts/ts.h"
#include <array>
#include <vector>
#include "util.h"


template <class key_t>
class PlexModel{


public:
    inline PlexModel();
    inline PlexModel(std::vector<double> &a,ts::TrieSpline<key_t> ts);
    ~PlexModel();
    void train(const typename std::vector<key_t>::const_iterator &it, size_t size);
    void train(const std::vector<key_t> &keys,const std::vector<size_t> &positions);
    
    size_t predict(const key_t &key) const;
    std::vector<size_t> predict(const std::vector<key_t> &keys) const;
    size_t max_error(const typename std::vector<key_t>::const_iterator &keys_begin,
                     uint32_t size);
    size_t max_error(const std::vector<key_t> &keys,
                     const std::vector<size_t> &positions);
    
    inline size_t get_maxErr() { return maxErr; }
    inline std::vector<double>& get_vec(){ return vec; }
    inline ts::TrieSpline<key_t> get_ts(){ return ts; }
    void print_weights() const;

private:
    size_t maxErr = 32;
    ts::TrieSpline<key_t> ts;
    std::vector<double> vec;
    
};

#endif


