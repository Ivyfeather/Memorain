#ifndef __INFO_H
#define __INFO_H

#include "common.hh"
#include "slowdown.hh"
#include "mem/packet.hh"
#include "mem/request.hh"

// #define DRAMSIM

// DRAMsim addr 
// from Memorain/ext/dramsim3/DRAMsim3/configs/DDR4_8Gb_x8_2400.ini
// |       | row     | channel | rank | bank     | bank group | column | byte in burst |
// | ----- | ------- | ------- | ---- | -------- | ---------- | ------ | ------------- |
// |  len  | 16      | 0       | 1    | 2        | 2          | 7      | 6             |
// |  bits | 34   18 | (18)    | 17   | 16    15 | 14    13   | 12   6 | 5   0         |
#ifdef DRAMSIM
#define CHANNEL_OFF     18
#define CHANNEL_MASK    0x0
#define RANK_OFF        17
#define RANK_MASK       0x1
#define BANKGROUP_OFF   13
#define BANKGROUP_MASK  0x3
#define BANK_OFF        15
#define BANK_MASK       0x3
#define COLUMN_OFF      6
#define COLUMN_MASK     0x7f
#define ROW_OFF         18
#define ROW_MASK        0xffff
#define BYTES_OFF       0
#define BYTES_MASK      0x3f

struct AddrMapping{
    AddrMapping(uint64_t hex_addr):
        channel     ( (hex_addr >> CHANNEL_OFF)  & CHANNEL_MASK ),
        rank        ( (hex_addr >> RANK_OFF)     & RANK_MASK    ),
        bank_group  ( (hex_addr >> BANKGROUP_OFF)& BANKGROUP_MASK),
        bank        ( (hex_addr >> BANK_OFF)     & BANK_MASK    ),
        column      ( (hex_addr >> COLUMN_OFF)   & COLUMN_MASK  ),
        row         ( (hex_addr >> ROW_OFF)      & ROW_MASK     )
    {}
    uint64_t addr;
    uint8_t channel;
    uint8_t rank;
    uint8_t bank_group;
    uint8_t bank;
    uint8_t column;
    uint16_t row;
};

#else
// at src/mem/mem_interface.cc:104
// mem-type=DDR3_1600_8x8
struct AddrMapping{
    AddrMapping(uint64_t hex_addr)
    {
        addr = hex_addr;
        uint64_t burstSize = 64;
        uint64_t burstsPerRowBuffer = 128;
        uint64_t banksPerRank = 8;
        uint64_t ranksPerChannel = 2;
        uint64_t rowsPerBank = 65536;

        uint64_t resaddr = hex_addr / burstSize;
        resaddr = resaddr / burstsPerRowBuffer;

        // after the channel bits, get the bank bits to interleave
        // over the banks
        bank = resaddr % banksPerRank;
        resaddr = resaddr / banksPerRank;

        // after the bank, we get the rank bits which thus interleaves
        // over the ranks
        resaddr = resaddr / ranksPerChannel;

        // lastly, get the row bits, no need to remove them from addr
        row = resaddr % rowsPerBank;  
    }
    uint64_t addr;
    uint8_t bank;
    uint16_t row;    
};  
#endif


class LabeledReq {
public:
    /// pointer to packet of gem5
    PacketPtr pkt; 

    /// timestamp
    // uint64_t time_received = 0;
    uint64_t time_sent = 0;
    uint64_t time_return = 0;
    uint64_t shadow_return = 0;

    /// address mapping
    AddrMapping addr;

    ///--------------------------------------------
    LabeledReq(PacketPtr pkt, uint64_t time_received, uint64_t paddr) :
        pkt(pkt), time_sent(time_received), addr(AddrMapping(paddr))
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