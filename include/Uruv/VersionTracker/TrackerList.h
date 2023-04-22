#ifndef TIMESTAMP_TRACKER_TRACKERLIST_H
#define TIMESTAMP_TRACKER_TRACKERLIST_H

#include <iostream>
#include <atomic>
#include "TrackerNode.h"

class TrackerList{
public:
	std::atomic<TrackerNode*> head;
	std::atomic<TrackerNode*> tail;
	TrackerNode *sentinel_last;
	
	std::atomic<uint64_t> active_thread_cnt;
	std::atomic<uint64_t> min_active_ts;


	TrackerList(){
		sentinel_last = new TrackerNode(INT64_MAX);
		head = new TrackerNode(0, sentinel_last);
		tail.store(head.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
	}
	
	//Will modify the tail to accomodate a new RQ.
	TrackerNode* add_timestamp();
	
	//the garbage collector will remove the head as that ts is no longer required.
	TrackerNode* remove_head();
	
	int64_t get_latest_timestamp(){
		return tail.load(std::memory_order_seq_cst) -> ts;
	}

	inline void trackTS(uint64_t ts) {
		//std::cout << "Active: " << min_active_ts << std::endl;
		active_thread_cnt++;
	}
	inline void untrackTS(uint64_t ts) {
		//active_thread_cnt--;
		if(active_thread_cnt >= 10) {
			min_active_ts++;
			active_thread_cnt = 0;
			//std::cout << "Reduced: " << min_active_ts << std::endl;
		}
	}
	inline int64_t getMinVersion() {
		return -1;
		//return min_active_ts.load(std::memory_order_seq_cst);
	}
};

#include "TrackerList.h"
TrackerNode* TrackerList::add_timestamp(){
	while(true){
		TrackerNode* curr_tail = tail.load(std::memory_order_seq_cst);
		TrackerNode* new_tail = new TrackerNode(curr_tail -> ts + 1, sentinel_last);
		TrackerNode* sentinel_copy = sentinel_last;
		if(curr_tail -> next.compare_exchange_strong(sentinel_copy, new_tail, std::memory_order_seq_cst, std::memory_order_seq_cst))
		{
			tail.compare_exchange_strong(curr_tail, new_tail, std::memory_order_seq_cst, std::memory_order_seq_cst);
			return new_tail;
		}
		while(true){	//helping keep the tail updated
			curr_tail = tail.load(std::memory_order_seq_cst);
			if(curr_tail -> next.load(std::memory_order_seq_cst) == sentinel_last){
				break;
			}
			else{
				tail.compare_exchange_strong(curr_tail, curr_tail -> next.load(std::memory_order_seq_cst), std::memory_order_seq_cst, std::memory_order_seq_cst);
			}
		}
	}
}

//Remove head is also used to get the minimum timestamp!
TrackerNode* TrackerList::remove_head(){
	TrackerNode* curr_head = head.load(std::memory_order_seq_cst);
	//this loop will end due to the invariant that sentinel_last
	//will never "finish"!
	while(curr_head -> finish){
		head.compare_exchange_strong(curr_head, curr_head -> next);
		curr_head = head.load(std::memory_order_seq_cst);
	}
	return curr_head;
}

#endif //TIMESTAMP_TRACKER_TRACKERLIST_H
