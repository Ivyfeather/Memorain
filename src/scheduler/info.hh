#ifndef __INFO_H
#define __INFO_H

#include "common.hh"
#include "slowdown.hh"
#include "mem/packet.hh"
#include "mem/request.hh"

typedef struct {
    uint64_t addr;
    uint8_t channel;
    uint8_t rank;
    uint8_t bank;
    uint8_t column;
    uint16_t row;
}AddrPartion;

class LabeledReq {
public:
    /// pointer to packet of gem5
    PacketPtr pkt; 

    /// whether as a sample for latency predicting model
    bool sampling = false;
    
    /// estimated latency
    int est_latency;
    
    /// timestamp
    // uint64_t time_received = 0;
    uint64_t time_sent = 0;
    uint64_t time_return = 0;
    uint64_t shadow_return = 0;

    ///--------------------------------------------
    LabeledReq(PacketPtr pkt, uint64_t time_received) :
        pkt(pkt), time_sent(time_received)
    { };
    
    std::string get_type_string() {
        if(pkt->isRead())
            return "READ";
        else if(pkt->isWrite())
            return "WRITE";
        else
            return "OTHERS";
    }

};

class CpuInfo{
public:
    /// tag(label) from core
    int tag = 0;

    /// pending req 
    /// - used to store reqs that have been sent to mem_ctrl 
    ///   but have not yet recevied response
    std::vector<LabeledReq *> pending_req;
   
    /// cycle recorder
    /// - used to count tick-inst to calculate slowdown
    CycleRecorder *cr = NULL;

    /// inst at the end of last sampling interval
    int64_t last_instcnt = 0;

    /// for counting NMC  
    bool isMC = false;
    uint64_t NMC_startTick = 0; 

    /// process slowdown
    std::vector<double> slowdown_vec;

    /// accumulators
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
    /*
    enum {
        ACC_ALL_READ_T,
        ACC_ALL_WRITE_T,
        ACC_ALL_MAX
    };
    */
    /* uint64_t acc[4][ACC_UI_MAX] = {{0}}; */
    uint64_t acc[ACC_UI_MAX] = {0};

};

#endif