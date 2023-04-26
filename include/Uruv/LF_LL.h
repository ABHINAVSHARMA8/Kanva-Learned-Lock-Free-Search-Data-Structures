

#ifndef UNTITLED_LF_LL_H
#define UNTITLED_LF_LL_H

#include<atomic>
#include <limits>
#include<vector>
#include <unordered_set>
#include <iostream>
#include <thread>
#include "util.h"
 //without sentinel
#define threshhold 100

template<typename K, typename V>
class Linked_List {
public:
    ll_Node<K,V>* head;
    std::atomic<size_t> size=0;
    Linked_List(){
        ll_Node<K,V>* max = new ll_Node<K,V>(std::numeric_limits<K>::max(), 0);
        ll_Node<K,V>* min = new ll_Node<K,V>(std::numeric_limits<K>::min(), 0);
        min -> next.store(max);
        head = min;
    }
    
    int64_t count();
    
    int insert(K Key, V value,TrackerList *version_tracker);
    int remove(K key,TrackerList *version_tracker);
    bool insert(K Key, V value, ll_Node<K,V>* new_node);
   
    bool search(K key,V value);
    
    std::vector<Vnode<V>*> collect(std::vector<K>*,std::vector<V>*);
    int range_query(int64_t low, int64_t remaining, int64_t curr_ts, std::vector<std::pair<K,V>>& res,TrackerList *version_tracker);
   
    ll_Node<K,V>* find(K key, ll_Node<K,V>**);
    void init_ts(Vnode<V> *node,TrackerList *version_tracker){
        if(node -> ts.load(std::memory_order_seq_cst) == -1){
            int64_t invalid_ts = -1;
            int64_t global_ts = (*version_tracker).get_latest_timestamp();
            node -> ts.compare_exchange_strong(invalid_ts, global_ts, std::memory_order_seq_cst, std::memory_order_seq_cst );
        }
    }
    V read(ll_Node<K,V> *node,TrackerList *version_tracker) {
        init_ts(node -> vhead,version_tracker);
        return node -> vhead.load(std::memory_order_seq_cst) -> value;
    }

    bool vCAS(ll_Node<K, V> *node, V old_value,  V new_value,TrackerList *version_tracker){
        Vnode<V>* head = (Vnode<V>*) unset_mark((uintptr_t) node -> vhead.load(std::memory_order_seq_cst));
        init_ts(head,version_tracker);
        if(head -> value != old_value) return false;
		if(head -> value == new_value)
			return true;
        Vnode<V>* new_node = new Vnode<V>(new_value, head);
        if(expt_sleep){
            int dice_roll = dice(gen);
            if(dice_roll < 5){
                std::this_thread::sleep_for(std::chrono::microseconds(5));
            }
        }
        if(node -> vhead.compare_exchange_strong(head,new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node,version_tracker);
            return true;
        }
        else{
//            delete new_node;
            return false;
        }
    }
    bool vCAS(ll_Node<K, V> *node, V old_value,  V new_value, Vnode<V>* new_node, Vnode<V>* head, Vnode<V>* vnext,TrackerList *version_tracker){
        if(head -> value != old_value) return false;
        if(head -> value == new_value)
            return true;
        if(!new_node -> nextv.compare_exchange_strong(vnext, head, std::memory_order_seq_cst, std::memory_order_seq_cst))
            return false;
        if(expt_sleep){
            int dice_roll = dice(gen);
            if(dice_roll < 5){
                std::this_thread::sleep_for(std::chrono::microseconds(5));
            }
        }
        if(node -> vhead.compare_exchange_strong(head,new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node,version_tracker);
            return true;
        }
        else{
//            delete new_node;
            return false;
        }
    }
};




template<typename K, typename V>
std::vector<Vnode<V>*> Linked_List<K,V>::collect(std::vector<K> *keys,std::vector<V> *values){
    
    
    std::vector<Vnode<V>*> version_lists;
    ll_Node<K, V> *left_node = head;
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

    if(!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
    {
        while(true)
        {
            Vnode<V> *curr_vhead = left_node->vhead.load(std::memory_order_seq_cst);
            if (!left_node->vhead.compare_exchange_strong(curr_vhead, (Vnode<V> *)set_freeze((long)curr_vhead)))
                continue;
            break;
        }
    }
   left_node = (ll_Node<K, V> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
   while (unset_mark((long)left_node->next.load(std::memory_order_seq_cst)))
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

        if(!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
        {
            while(true)
            {
                Vnode<V> *curr_vhead = left_node->vhead.load(std::memory_order_seq_cst);
                if (!left_node->vhead.compare_exchange_strong(curr_vhead, (Vnode<V> *)set_freeze((long)curr_vhead)))
                    continue;
                break;
            }
        }
        Vnode<V> *left_node_vhead = (Vnode<V>*) get_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
           
        (*keys).push_back(left_node->key);
        (*values).push_back(left_node_vhead->value);
        version_lists.push_back(left_node_vhead);

        left_node = (ll_Node<K, V> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
//        Vnode<V> *left_node_vhead = (Vnode<V>*) get_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
        
    }
    return version_lists;
}

template<typename K, typename V>
int Linked_List<K,V>::range_query(int64_t low, int64_t remaining, int64_t curr_ts, std::vector<std::pair<K,V>> &res,TrackerList *version_tracker){
    
    ll_Node<K,V>* left_node = ( ll_Node<K,V>* ) get_unfreezed_unmarked_ref((long) head -> next.load(std::memory_order_seq_cst));
    ll_Node<K,V>* left_next = (ll_Node<K,V>*) get_unfreezed_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
    while(left_next && left_node -> key < low){
       // std::cout<<"Range query in level bin"<<std::endl;
        left_node = left_next;
        left_next = ( ll_Node<K,V>* ) get_unfreezed_unmarked_ref((long) left_next -> next.load(std::memory_order_seq_cst));
    }
    while(left_next && remaining>0)
    {
        Vnode<V>* curr_vhead = (Vnode<V>*) get_unfreezed_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
        init_ts(curr_vhead,version_tracker);
        while(curr_vhead && curr_vhead -> ts >= curr_ts) curr_vhead = curr_vhead -> nextv;
        if(curr_vhead && curr_vhead  -> value != -1){
            //std::cout<<"Range query in level bin"<<std::endl;
            res.push_back(std::make_pair(left_node -> key, curr_vhead -> value));
            remaining-=1;
            if(remaining<=0) break;
        }
        left_node = left_next;
        left_next = ( ll_Node<K,V>* ) get_unfreezed_unmarked_ref((long) left_next -> next.load(std::memory_order_seq_cst));
    }
    return remaining;
}

template<typename K, typename V>
bool Linked_List<K,V>::search(K key,V value) { //CHANGED CHECK:inserted value fieled
    ll_Node<K,V>* curr = head;
    //std::cout<<"LL search"<<std::endl;
    while(curr -> key < key)
        curr = (ll_Node<K,V>*) unset_freeze_mark((long) curr -> next .load(std::memory_order_seq_cst));
    
    return (curr -> key == key && curr->vhead.load(std::memory_order_seq_cst)->value==value 
    && !is_marked_ref((long) curr -> next.load(std::memory_order_seq_cst)));
}





template<typename K, typename V>
ll_Node<K,V>* Linked_List<K,V>::find(K key, ll_Node<K, V> **left_node) {
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
            /* No node is marked for deletion
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
            */
            if (right_node->key >= key)
                return right_node;
            (*left_node) = right_node;
            right_node = (ll_Node<K, V> *)get_unmarked_ref((long)right_next);
        }
    }
}


template<typename K, typename V>
int Linked_List<K,V>::insert(K key, V value,TrackerList *version_tracker) {
//    return -1 for failed operation,0 for no incremenet,1 for sucess,-2 for retraining,
    
    if(size.load(std::memory_order_seq_cst)>=threshhold) return -2;//retraining
    
    while(true)
    {   //std::cout<<"Insert at Linked List"<<std::endl;
        ll_Node<K,V>* prev_node = nullptr;
        ll_Node<K,V>* right_node = find(key, &prev_node);
        if(right_node == nullptr)
            return -1;
        if(right_node -> key == key){
            while(true)
            {
                V curr_value = read(right_node,version_tracker);
				if(curr_value == value)
					return 0;
                if(vCAS(right_node,curr_value,value,version_tracker))
                    break;
                else if(is_marked_ref((uintptr_t) right_node -> vhead.load(std::memory_order_seq_cst)))
                    return -1;//INCORRECT
                
            }
            return 0;//version list has changed
        }
        else
        {   
            if(value==-1) return 0;
            
            ll_Node<K,V>* new_node = new ll_Node<K,V>(key,value);
            new_node -> next.store(right_node);
            /*if(expt_sleep){
                int dice_roll = dice(gen);
                if(dice_roll < 5){
                    std::this_thread::sleep_for(std::chrono::microseconds(5));
                }
            }*/
            if(prev_node -> next.compare_exchange_strong(right_node, new_node)) {
                init_ts(new_node -> vhead,version_tracker);
                size+=1;//increment size
                
                

                
                //std::cout<<"Insert in LF_LL.h"<<size.load(std::memory_order_seq_cst)<<std::endl;
                return 1;
            }
            
        }
    }
}

template<typename K, typename V>
int Linked_List<K,V>::remove(K key,TrackerList *version_tracker) {
    return insert(key,-1,version_tracker);//CHANGE
}



//template<typename K, typename V>
//int64_t Linked_List<K,V>::count() {
//    int64_t count = 0;
//    ll_Node<K,V>* left_node = head -> next.load(std::memory_order_seq_cst);
//    while(left_node -> next.load(std::memory_order_seq_cst)){
//        if(!is_marked_ref((long) left_node -> next.load(std::memory_order_seq_cst)))
//            count++;
//        left_node = ( ll_Node<K,V>* ) get_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
//    }
//    return count;
//}


#endif //UNTITLED_LF_LL_H
