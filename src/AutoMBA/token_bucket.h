#ifndef __TOKEN_BUCKET_H_
#define __TOKEN_BUCKET_H_

#include "common.h"
#include "latency_pred.h"
#include "slowdown_pred.h"
// add gem5 memory-related headers
#include "mem/packet.hh"
#include "mem/request.hh"

class LabeledReq {
public:
    PacketPtr req; //[Ivy]
    bool sampling = false;
    int est_latency;
    /// 
    uint64_t accept, send, arrive = 0;
    uint64_t shadow_arrive = 0;

    LabeledReq() { };
    std::string get_type_string() {
        /*
        switch (req->type) {
            case ramulator::Request::Type::READ:
                return "READ";
            case ramulator::Request::Type::WRITE:
                return "WRITE";
            case ramulator::Request::Type::REFRESH:
                return "REFRESH";
            case ramulator::Request::Type::POWERDOWN:
                return "POWERDOWN";
            case ramulator::Request::Type::SELFREFRESH:
                return "SELFREFRESH";
            case ramulator::Request::Type::EXTENSION:
                return "EXTENSION";
            default:
                return "UNKNOWN";
        */
        if(req->isRead())
            return "READ";
        else if(req->isWrite())
            return "WRITE";
        else
            return "OTHERS";
        
    }
};

class TokenBucket {
private:
    int size, freq, inc;
    bool bypass;

    int tokens;
    std::queue<LabeledReq *> waiting_queue;

    uint64_t cycle_counter;
public:
    TokenBucket(int s, int f, int i, bool b) : size(s), freq(f), inc(i), bypass(b) {
        assert(s >= inc && "inc should not be greater than size");
        tokens = i;
        cycle_counter = 0;
    };

    ~TokenBucket() { };

    inline int get_size() { return size; }
    inline void set_size(int s) { size = s; }
    
    inline int get_freq() { return freq; }
    inline void set_freq(int f) { freq = f; }

    inline int get_inc() { return inc; }
    inline void set_inc(int i) { inc = std::min(std::max(1, i), size); }

    inline bool get_bypass() { return bypass; }
    inline void set_bypass(bool b) { bypass = b; }

    inline int get_tokens() { return (bypass) ? 1 : tokens; }

    void add_request(LabeledReq *request, bool head);
    bool get_request(LabeledReq *&request);
    int waiting_num() { return waiting_queue.size(); }

    void operate();
};

#endif
