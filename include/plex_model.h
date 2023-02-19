#ifndef __PLEX_MODEL_H__
#define __PLEX_MODEL_H__


#include "plex/ts/builder.h"
#include "plex/ts/common.h"
#include <array>
#include <vector>
#include "util.h"


template <class key_t>
class PlexModel{


public:
    inline PlexModel();
    ~PlexModel();
    void train(const typename std::vector<key_t>::const_iterator &it, size_t size,size_t maxErr);
    void train(const std::vector<key_t> &keys,
               const std::vector<size_t> &positions);
    
    size_t predict(const key_t &key) const;
    std::vector<size_t> predict(const std::vector<key_t> &keys) const;
    size_t max_error(const typename std::vector<key_t>::const_iterator &keys_begin,
                     uint32_t size);
    size_t max_error(const std::vector<key_t> &keys,
                     const std::vector<size_t> &positions);
    
    inline size_t get_maxErr() { return maxErr; }
    void print_weights() const;

private:
    size_t maxErr = 0;
    std::vector<key_t> &keys;
    
};

#endif


