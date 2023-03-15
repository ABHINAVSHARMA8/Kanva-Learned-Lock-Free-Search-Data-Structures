#include <iostream>
#include "include/function.h"
#include "include/aidel.h"
#include "include/aidel_impl.h"
#include<time.h>
#include<vector>

int main(){
	clock_t s,e;
	s=clock();
	vector<int> keys;
	for(int i=0;i,100000;i++) keys.push_back(i);
	vector<int> non_exist;
	for(int i=100000;i<200000;i++) non_exist.push_back(i);
	aidel_type *ai;
	ai = new aidel_type();
    	ai->train(keys,keys, 0);
	ai->self_check();
	std::cout<<"Self Check ok"<<std::endl;
	if(Config.read_ratio==1){
		for(int i:keys) ai->find(i);
	}

	else if(Config.insert_ratio==1){
		for(int i:non_exist) ai->insert(i);
	}
	e=clock();
	 double time_taken = double(e - s)/double(CLOCKS_PER_SEC);

         std::cout<<" execution time= "<<time_taken<<std::endl;

	return 0;
}
