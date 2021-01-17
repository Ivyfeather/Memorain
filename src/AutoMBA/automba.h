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

class ReturnPacket {
public:
    long addr;
    int coreid;
    uint8_t instruction;
    uint32_t champsim_type;
    uint64_t time;
};

class AutoMBA{
private:
    const static bool PRINT_ACCUMULATORS = true;
    const static bool PRINT_TB_PARAMETERS = true;

    const static bool PRINT_MEMORY_ACCESS_LOG = false;
    const static bool RECORD_SOLO_RUNTIME = (NUM_CPUS == 1);
    const static bool SHOW_ACTUAL_SLOWDOWN = (NUM_CPUS > 1);
    const static bool SHOW_PREDICTED_SLOWDOWN = (NUM_CPUS > 1);

    /// 
    int core_tags[NUM_CPUS] = {1};
    
    ////
    TokenBucket *buckets[NUM_CPUS];
    
    /// pending req 
    /// - used to store reqs that have been sent to mem_ctrl 
    ///   but have not yet recevied response
    std::vector<LabeledReq *> pending_req[NUM_CPUS];
    
    /// 
    Arbiter arbiter;

    ///
    LatencyPred lpm[NUM_CPUS];
    
    ///
    CycleRecorder *cr[NUM_CPUS] = {NULL};

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
    uint64_t sampling_interval = SAMPLING_INTERVAL, updating_interval = UPDATING_INTERVAL;
    uint64_t sampling_cycle = 0, updating_cycle = 0;

    /// slowdown estimate
    SlowdownEstimator estimator;
    std::vector<double> slowdown_vec[NUM_CPUS];

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

    /// 
    std::queue<ReturnPacket *> return_q;

    void operate_token_buckets();
    void operate_slowdown_pred();


    void request_sent(LabeledReq *lreq);
    
    bool request_match(PacketPtr a, PacketPtr b) {
        //////[TODO]
        return true;
    }
    
    //     return a->coreid == b->coreid && a->addr == b->addr && a->type == b->type;
    // }
    // void return_data_ramulator_shadow(PacketPtr& req);
    
    /// print counter info
    void print_si_accumulators();
    void print_ui_accumulators();
    void reset_si_accumulators();
    void reset_ui_accumulators();
    void print_tb_parameters();

public:
    AutoMBA();
    
    ~AutoMBA();


    
    bool add_request(PacketPtr *request);
    void request_recv(PacketPtr *req, bool shadow);

    void return_data_ramulator(PacketPtr& req);
    
    void check_return_data();


    void start();
    
    void operate();

    void finish();
};

#endif
