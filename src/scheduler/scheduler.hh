#ifndef __SCHED_H_
#define __SCHED_H_

#include "common.hh"
#include "info.hh"
#include "token_bucket.hh"
// gem5 memory-related headers
#include "mem/packet.hh"
#include "mem/request.hh"


class Scheduler{
private:
    //// a token bucket for each tag
    int num_tags = 2;
    TokenBucket **buckets;

    /// CPU info 
    /* Beaware: 
        info[0] is for func
        cpu_i is at CpuInfo[i+1]
        when enumerating, use "for(int i=0; i<=num_cpus; i++)"
    */
    CpuInfo *info;

    /// Used for printing system-related info [Ivy]
    void *obj;

    int num_cpus = 1;

    /// memory access control policy
    enum class Policy {
        CORE0_T,
        ALL
    };    
    Policy policy = Policy::CORE0_T;

    enum {
        ACC_SI_READ_T,
        ACC_SI_WRITE_T,
        ACC_SI_LATENCY,
        ACC_SI_NMC_COUNT,
        ACC_SI_MAX,
        ACC_UI_READ_T,
        ACC_UI_WRITE_T,
        ACC_UI_MAX
    };

public:
    Scheduler(void *memobj, int num_cpu, int num_tag,
     std::vector<int>& core_tags, std::vector<std::string>& paths) :
        num_tags(num_tag),
        obj(memobj),
        num_cpus(num_cpu)   
    { 
        LOG(INFO, "check params #cpu:%d #tag:%d\n", num_cpu, num_tag);
        // ----- init every cpuinfo -----
        info = new CpuInfo[num_cpus+1];
        // init trace_file path
        for(auto it = paths.begin(); it!= paths.end(); it++){
            LOG(DEBUG, "%s", (*it).c_str());
        }

        for(int i=1; i<=num_cpus; i++){
            LOG(DEBUG, "CRISTINA TAG %d",core_tags[i-1]);
            info[i].tag = core_tags[i-1];
        }

        // ----- init token buckets -----
        buckets = new TokenBucket *[num_tags+1]; // 0 for func
        int init_size = 60, init_freq = 10000000, init_inc = 50;
        for(int i=0; i<=num_tags; i++){
            int cnt_tagi = 0;
            for(int j=0; j<=num_cpus; j++){
                if(info[j].tag == i) cnt_tagi++;
            }
#ifdef CONTROLL_ENABLE
            buckets[i] = new TokenBucket(cnt_tagi*init_size, init_freq, cnt_tagi*init_inc, (i==1));   
            buckets[i]->cpus = info;
#else
            buckets[i] = new TokenBucket(cnt_tagi*init_size, init_freq, cnt_tagi*init_inc, true);    
#endif
        }

    }

    ~Scheduler()
    {
        delete []info;
        for(int i=0; i<num_tags; i++){ delete buckets[i]; }
    }

    /// print counter info
    void print_si_accumulators();
    void print_ui_accumulators();
    void reset_si_accumulators();
    void reset_ui_accumulators();
    void print_tb_parameters();

    /// used to handle reqs from cpus to memctrl
    bool handle_request(PacketPtr pkt);
    
    /// used to handle resps from memctrl to cpus
    void handle_response(PacketPtr pkt);

    /// update token bucket params every updating interval
    void update_token_bucket();

    /// operate slowdown pred (every sampling interval)
    void operate_slowdown_pred();

    /// get a waiting req from token buckets for memobj to send
    PacketPtr get_waiting_req();

    void count_NMC();

    TokenBucket *bucket(int i){ 
        assert(i <= num_tags);
        return buckets[i]; 
    }
    void *memobj(){ return obj; }
    int get_core_tags(int i){
        assert(i <= num_cpus);
        return info[i].tag;
    }

};


#endif