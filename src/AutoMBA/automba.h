#ifndef __AUTOMBA_H_
#define __AUTOMBA_H_

#include "common.h"
#include "latency_pred.h"
#include "token_bucket.h"
#include "slowdown_pred.h"
// add gem5 memory-related headers
#include "mem/packet.hh"
#include "mem/request.hh"

class Arbiter { //仲裁
public:
    // tags: application info
    // status: tb status, currently refers to #waiting requests
    int get_core(int tag[NUM_CPUS], int status[NUM_CPUS]);
};


class AutoMBA{
private:
    ///[Ivy TODO] use these as #define
    const static bool PRINT_ACCUMULATORS = true;
    const static bool PRINT_TB_PARAMETERS = true;

    const static bool PRINT_MEMORY_ACCESS_LOG = false;
    const static bool RECORD_SOLO_RUNTIME = (NUM_CPUS == 1);
    // const static bool SHOW_ACTUAL_SLOWDOWN = (NUM_CPUS > 1);
    const static bool SHOW_ACTUAL_SLOWDOWN = true;
    const static bool SHOW_PREDICTED_SLOWDOWN = (NUM_CPUS > 1);

    const static int NUM_TAGS = 2;

    /// the label passed from core
    int core_tags[NUM_CPUS] = {0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0};
    
    //// a token bucket for each tag
    TokenBucket *buckets[NUM_TAGS];
    
    /// pending req 
    /// - used to store reqs that have been sent to mem_ctrl 
    ///   but have not yet recevied response
    std::vector<LabeledReq *> pending_req[NUM_CPUS];
    
    /// decide which tokenbucket to send from
    Arbiter arbiter;

    /// lantency predicting model
    LatencyPred lpm[NUM_CPUS];
    
    /// for counting NMC  
    bool isMC[NUM_CPUS] = {false};
    uint64_t NMC_startTick[NUM_CPUS] = {0}; 

    /// 
    CycleRecorder *cr = NULL;

    /// info counters
    static const int ACC_NUM = 64;
    uint64_t acc[NUM_CPUS+1][ACC_NUM] = {{0}};
    enum {
        ACC_SI_READ_T,
        ACC_SI_WRITE_T,
        ACC_SI_LATENCY,
        ACC_SI_LATENCY_MODEL,
        ACC_SI_LATENCY_SHDW,
        ACC_SI_SAMPLING_NUM,
        ACC_SI_NMC_COUNT,
        ACC_SI_MAX,
        ACC_UI_READ_T,
        ACC_UI_WRITE_T,
        // ACC_UI_LATENCY,
        // ACC_UI_LATENCY_MODEL,
        // ACC_UI_LATENCY_SHDW,
        // ACC_UI_SAMPLING_NUM
        ACC_UI_MAX
    };
    enum {
        ACC_ALL_READ_T,
        ACC_ALL_WRITE_T,
        ACC_ALL_MAX
    };
    ///[Ivy TODO] use these as #define
    uint64_t sampling_interval = SAMPLING_INTERVAL, updating_interval = UPDATING_INTERVAL;
    uint64_t sampling_cycle = 0, updating_cycle = 0;

    /// slowdown estimate
    ///  for cpu0 alone
    SlowdownEstimator estimator;
    std::vector<double> slowdown_vec;
    int64_t lastsi_instCnt = 0;

    /// memory access control policy
    enum class Policy {
        TOTAL_T,
        CORE0_T,
        MAX
    };
#ifdef AUTOMBA_ENABLE
    Policy policy = Policy::CORE0_T;
#else
    Policy policy = Policy::MAX;
#endif

public:  
    /// print counter info
    void print_si_accumulators();
    void print_ui_accumulators();
    void reset_si_accumulators();
    void reset_ui_accumulators();
    void print_tb_parameters();


    AutoMBA(void *obj);
    
    ~AutoMBA();

    /// used to handle reqs from cpus to memctrl
    bool handle_request(PacketPtr pkt);
    
    /// used to handle resps from memctrl to cpus
    void handle_response(PacketPtr pkt);

    /// update token bucket params every updating interval
    void update_token_bucket();

    /// operate slowdown pred for cpu0 (every sampling interval)
    void operate_slowdown_pred();

    TokenBucket *bucket(int i){ return buckets[i]; }
    
    void count_NMC();

    int get_core_tags(int i){return core_tags[i];}

    /// Used for printing system-related info [Ivy]
    /// consider pass cpu0->info alone?[Ivy TODO]
    void *obj;
};

#endif
