#include <iostream>
#include "include/function.h"
#include "include/aidel.h"
#include "include/aidel_impl.h"
#include<time.h>

struct alignas(CACHELINE_SIZE) ThreadParam;


typedef ThreadParam thread_param_t;
typedef aidel::AIDEL<key_type, val_type> aidel_type;


volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    aidel_type *ai;
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark(aidel_type *ai, size_t sec);
void *run_fg(void *param);
void prepare(aidel_type *&ai);

int main(int argc, char **argv) {
    

    parse_args(argc, argv);
    load_data();
    aidel_type *ai;
    prepare(ai);
    run_benchmark(ai, Config.runtime);
    ai->self_check();
    if(ai!=nullptr) delete ai;
    

    
    
    return 0;
}

void prepare(aidel_type *&ai){
    COUT_THIS("[Training aidel]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    ai = new aidel_type();
    ai->train(exist_keys, exist_keys, 32);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "training");
    ai->self_check();
    COUT_THIS("check aidel: OK");
    
}

void run_benchmark(aidel_type *ai, size_t sec) {
    pthread_t threads[2];
    thread_param_t thread_params[2];
    // check if parameters are cacheline aligned
    for (size_t i = 0; i < 2; i++) {
        if ((uint64_t)(&(thread_params[i])) % CACHELINE_SIZE != 0) {
            COUT_N_EXIT("wrong parameter address: " << &(thread_params[i]));
        }
    }

    running = false;
    for(size_t worker_i = 0; worker_i < 2; worker_i++){
        thread_params[worker_i].ai = ai;
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        int ret = pthread_create(&threads[worker_i], nullptr, run_fg,
                                (void *)&thread_params[worker_i]);
        if (ret) {
            COUT_N_EXIT("Error:" << ret);
        }
    }

    COUT_THIS("[micro] prepare data ...");
    while (ready_threads < Config.thread_num) sleep(0.5);

    running = true;
    std::vector<size_t> tput_history(2, 0);
    size_t current_sec = 0;
    while (current_sec < sec) {
        sleep(1);
        uint64_t tput = 0;
        for (size_t i = 0; i < 2; i++) {
            tput += thread_params[i].throughput - tput_history[i];
            tput_history[i] = thread_params[i].throughput;
        }
        COUT_THIS("[micro] >>> sec " << current_sec << " throughput: " << tput);
        ++current_sec;
    }

    running = false;
    void *status;
    for (size_t i = 0; i < 2; i++) {
        int rc = pthread_join(threads[i], &status);
        if (rc) {
            COUT_N_EXIT("Error:unable to join," << rc);
        }
    }

    size_t throughput = 0;
    for (auto &p : thread_params) {
        throughput += p.throughput;
    }
    COUT_THIS("[micro] Throughput(op/s): " << throughput / sec);
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    aidel_type *ai = thread_param.ai;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);

    size_t non_exist_key_n_per_thread = non_exist_keys.size() / Config.thread_num;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread;
    size_t non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    std::vector<key_type> op_keys(non_exist_keys.begin() + non_exist_key_start,
                                   non_exist_keys.begin() + non_exist_key_end);

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0;
    // exsiting keys fall within range [delete_i, insert_i)
    ready_threads++;
    volatile result_t res = result_t::failed;
    val_type dummy_value = 1234;

   
       
        
            key_type dummy_key = non_exist_keys[0];
            res = ai->insert(dummy_key, dummy_key);
            std::cout<<"Insert"<<std::endl;
            
        thread_param.throughput++;
    
    pthread_exit(nullptr);
}
