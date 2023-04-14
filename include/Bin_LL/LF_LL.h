

#ifndef UNTITLED_LF_LL_H
#define UNTITLED_LF_LL_H

#include <atomic>
#include <limits>
#include <vector>
#include <unordered_set>
#include <iostream>
#include "util1.h"

template <typename K, typename V>
class Linked_List
{
public:
    ll_Node<K, V> *head;
    std::atomic<size_t> count;
    volatile bool is_retraining = false;
    const size_t bin_to_model_threshold = 10;

    Linked_List()
    {
        ll_Node<K, V> *max = new ll_Node<K, V>(std::numeric_limits<K>::max(), 0);
        ll_Node<K, V> *min = new ll_Node<K, V>(std::numeric_limits<K>::min(), 0);
        min->next.store(max);
        head = min;
    }

    void mark();

    result_t insert(K Key, V value, bool is_update = false);

    bool insert(K Key, V value, ll_Node<K, V> *new_node);

    int insert(K Key, V value, int tid, int phase);

    bool search(K key);

    void collect(std::vector<K> &, std::vector<V> &);

    void range_query(int64_t low, int64_t high, int64_t curr_ts, std::vector<std::pair<K, V>> &res);

    ll_Node<K, V> *find(K key);

    ll_Node<K, V> *find(K key, ll_Node<K, V> **);

    int remove(K key);

    int scan(const K &key, const size_t n, std::vector<std::pair<K, V>> &result);

    void self_check() {} // TODO: Implement
};

template <typename K, typename V>
void Linked_List<K, V>::collect(std::vector<K> &keys, std::vector<V> &vals)
{

    ll_Node<K, V> *left_node = head;
    int i = 0;
    while (left_node->next.load(std::memory_order_seq_cst))
    {
        if (!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
        {
            while (true)
            {
                ll_Node<K, V> *curr_next = left_node->next.load(std::memory_order_seq_cst);
                if (!left_node->next.compare_exchange_strong(curr_next, (ll_Node<K, V> *)set_freeze((long)curr_next)))
                    continue;
                break;
            }
        }
        //
        left_node = (ll_Node<K, V> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
        if (!is_marked((uintptr_t)left_node))
        {
            keys.push_back(left_node->key);
            vals.push_back(left_node->value);
        }
        left_node = (ll_Node<K, V> *)get_unmarked_ref((long)left_node);
    }
    keys.pop_back();
    vals.pop_back();
}

template <typename K, typename V>
bool Linked_List<K, V>::search(K key)
{
    ll_Node<K, V> *curr = head;
    while (curr->key < key)
        curr = (ll_Node<K, V> *)unset_freeze_mark((uintptr_t)curr->next.load(std::memory_order_seq_cst));
    return (curr->key == key && !is_marked_ref((long)curr->next.load(std::memory_order_seq_cst)));
}

template <typename K, typename V>
ll_Node<K, V> *Linked_List<K, V>::find(K key, ll_Node<K, V> **left_node)
{
retry:
    while (true)
    {
        (*left_node) = head;
        ll_Node<K, V> *right_node;
        if (is_freeze((uintptr_t)(*left_node)->next.load(std::memory_order_seq_cst)))
            return nullptr;
        right_node = (ll_Node<K, V> *)unset_freeze_mark((long)(*left_node)->next.load(std::memory_order_seq_cst));
        while (true)
        {
            ll_Node<K, V> *right_next = right_node->next.load(std::memory_order_seq_cst);
            if (is_freeze((uintptr_t)right_next))
            {
                return nullptr;
            }
            while (is_marked_ref((long)right_next))
            {
                if (!((*left_node)->next.compare_exchange_strong(right_node, (ll_Node<K, V> *)get_unmarked_ref((long)right_next))))
                {
                    goto retry;
                }
                else
                {
                    right_node = (ll_Node<K, V> *)unset_freeze_mark((long)right_next);
                    right_next = right_node->next.load(std::memory_order_seq_cst);
                }
                if (is_freeze((uintptr_t)right_next))
                {
                    return nullptr;
                }
                if (is_freeze((uintptr_t)right_node))
                {
                    return nullptr;
                }
            }
            if (right_node->key >= key)
                return right_node;
            (*left_node) = right_node;
            right_node = (ll_Node<K, V> *)get_unmarked_ref((long)right_next);
        }
    }
}

template <typename K, typename V>
int Linked_List<K, V>::remove(K key)
{
    if (is_retraining)
    {
        return -1; // result_t::failed;
    }
    while (true)
    {
        ll_Node<K, V> *prev_node = nullptr;
        ll_Node<K, V> *right_node = find(key, &prev_node);
        if (right_node == nullptr)
            return -1;
        if (right_node->key != key)
        {
            return -2;
        }
        else
        {
            ll_Node<K, V> *right_next = right_node->next.load(std::memory_order_seq_cst);
            if (!is_marked_ref((long)right_next))
            {
                if (!right_node->next.compare_exchange_strong(right_next, (ll_Node<K, V> *)set_mark((long)right_next)))
                    continue;
            }
            prev_node->next.compare_exchange_strong(right_node, (ll_Node<K, V> *)get_unmarked_ref((long)right_next));
            return right_node->value;
        }
    }
}

template <typename K, typename V>
result_t Linked_List<K, V>::insert(K key, V value, bool is_update)
{
    if (is_retraining)
    {
        return result_t::failed;
    }
    if (value == -1)
        return result_t::failed;
    while (true)
    {
        ll_Node<K, V> *prev_node = nullptr;
        ll_Node<K, V> *right_node = find(key, &prev_node);
        if (right_node == nullptr)
        {
            return result_t::failed;
        }
        if (right_node->key == key)
        {
            if (is_update)
            {
                // Don't do in while loop, if it is failing, then another update was going on
                V old_value = right_node->value;
                right_node->value.compare_exchange_strong(old_value, value);
                return result_t::ok;
            }
            return result_t::failed;
        }

        if (count > bin_to_model_threshold)
        {
            is_retraining = true;
            return result_t::retrain;
        }

        ll_Node<K, V> *new_node = new ll_Node<K, V>(key, value);
        new_node->next.store(right_node);
        if (prev_node->next.compare_exchange_strong(right_node, new_node))
        {
            count++;
            return result_t::ok;
        }
    }
}
template <typename K, typename V>
int Linked_List<K, V>::scan(const K &key, const size_t n, std::vector<std::pair<K, V>> &result)
{
    size_t remaining = 0;   
    ll_Node<K, V> *left_node = head;
    
    while (left_node->next.load(std::memory_order_seq_cst))
    {
        if (!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
        {
            while (true)
            {
                ll_Node<K, V> *curr_next = left_node->next.load(std::memory_order_seq_cst);
                if (!left_node->next.compare_exchange_strong(curr_next, (ll_Node<K, V> *)set_freeze((long)curr_next)))
                    continue;
                break;
            }
        }
        //
        left_node = (ll_Node<K, V> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
        if (!is_marked((uintptr_t)left_node) && (left_node->key != std::numeric_limits<K>::max()))
        {
            result.push_back({left_node->key, left_node->value});
            remaining--;
            if(remaining == 0) break;
        }
        left_node = (ll_Node<K, V> *)get_unmarked_ref((long)left_node);
    }
    //result.pop_back();
    
   return remaining;
}

#endif // UNTITLED_LF_LL_H
