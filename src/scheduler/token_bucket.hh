#ifndef __TOKEN_BUCKET_H_
#define __TOKEN_BUCKET_H_

#include "common.h"
#include "info.hh"
#include "info.hh"
#include "mem/packet.hh"
#include "mem/request.hh"

class TokenBucket{
public:
    int size, freq, inc;

    /// bypass true: do not use token bucket
    bool bypass;

    /// number of tokens left
    int tokens;

    /// used to store reqs not yet sent to mem_ctrl
    std::deque<LabeledReq *> waiting_queue;

    /// point to cpuinfos
    CpuInfo *cpus = NULL;

    ///--------------------------------------------
    TokenBucket(int s, int f, int i, bool b) : 
        size(s), freq(f), inc(i), bypass(b), tokens(i)
    {
        assert(s >= inc && "inc should not be greater than size");        
    }

    ~TokenBucket() { };

    /// add req to waiting queue
    void add_request(LabeledReq *request, bool head);

    /// if there are enough tokens, return a req in waiting queue
    /// else return NULL
    /// used when sending packets in waiting req
    LabeledReq* get_request();

    /// when cycle == freq, add tokens (set_inc(tokens+inc))
    void add_tokens();

    /// if there are enough tokens, tokens-- and return true
    /// else return false; 
    /// used when a req arrives memobj
    bool test_and_get();

    /// used to reorder reqs in waiting queue
    // rule1: gather reqs for the same row(row buffer hit)
    // rule2: arrange reqs in different banks (bank parallelism)
    void reorder_reqs();

    // 1 <= inc <= size
    inline void set_inc(int i) { inc = std::min(std::max(1, i), size); }
    inline int get_tokens() { return (bypass) ? 1 : tokens; }
    int waiting_num() { return waiting_queue.size(); }
};

#endif