#include "scheduler.hh"
#include "simple_memobj.hh"
#include "cpu/simple/timing.hh"
#include "cpu/o3/cpu.hh"
#include "cpu/o3/deriv.hh"

/* util func */
/// get requestor ID of a packet
bool
req_match(PacketPtr pkt1, PacketPtr pkt2)
{
    return pkt1->req == pkt2->req;
}
///

void
Scheduler::print_si_accumulators(){
#ifdef PRINT_ACCUMULATORS
    std::cout << "curTick" << curTick() << std::endl;
    for (int i = 0; i <= num_cpus; i++) {
        // i=cpu_id + 1 (i=0 is func)
        // print sampling interval accumulators
        std::cout << "si_accumulators[" << i << "]: ";
        int num_acc = (int)ACC_SI_MAX;
        for (int j = 0; j < num_acc; j++) {
            std::cout << info[i].acc[j] << ((j == num_acc - 1)? "\n" : " ");
        }
    }
#else
#endif
}

void
Scheduler::print_ui_accumulators(){
#ifdef PRINT_ACCUMULATORS
    for (int i = 0; i <= num_cpus; i++) {
        // i=cpu_id + 1 (i=0 is func)
        // print sampling interval accumulators
        std::cout << "ui_accumulators[" << i << "]: ";
        int num_acc = (int)ACC_UI_MAX;
        for (int j = (int)ACC_SI_MAX+1; j < (int)ACC_UI_MAX; j++) {
            std::cout << info[i].acc[j] << ((j == num_acc - 1)? "\n" : " ");
        }
    }
#else
#endif
}

void
Scheduler::reset_si_accumulators(){
    for (int i = 0; i <= num_cpus; i++) {
        int num_acc = (int)ACC_SI_MAX;
        for (int j = 0; j < num_acc; j++) {
            info[i].acc[j] = 0;
        }
    }
}

void
Scheduler::reset_ui_accumulators(){
    for (int i = 0; i <= num_cpus; i++) {
        for (int j = (int)ACC_SI_MAX+1; j < (int)ACC_UI_MAX; j++) {
            info[i].acc[j] = 0;
        }
    }
}

void
Scheduler::print_tb_parameters(){
#ifdef PRINT_TB_PARAMETERS
    for(int i = 0; i <= num_tags; i ++){
        std::cout << "token_bucket[" << i << "] parameters: ";
        std::cout << buckets[i]->size << " " << buckets[i]->freq << " "
                  << buckets[i]->inc << " " << buckets[i]->bypass << " "
                  << buckets[i]->tokens << std::endl;

    }
#else
#endif
}


/// used to handle reqs from cpus to memctrl
bool
Scheduler::handle_request(PacketPtr pkt){
    //[Ivy] we no longer use an arbiter to choose which tb to get a req from & send the req
    //  because champsim does it every cycle, but gem5 does this at handle_req action

    // new implementation is 
    // check if token_bucket[reqid] has enough tokens
    int rid = pkt->requestorId();
    CpuInfo *cpu = &info[CORE(rid)];
    int pkt_tag = cpu->tag;

    // if tb allows to send, but memPort.sendPacket fails
    // then this req has been added into pending again, but not truely sent
    // when memctrl calls sendreqretry, this req will be added in pending queue once again
    // the following checks if req already in pending queue
    for(auto it = cpu->pending_req.begin(); it!= cpu->pending_req.end(); it++){
        LabeledReq *lreq = (*it);
        if(req_match(lreq->pkt, pkt))
            return true;
    }

    // Put req into pending queue
    LabeledReq *lreq = new LabeledReq(pkt, curTick(), pkt->req->getPaddr());
    cpu->pending_req.push_back(lreq);

    // for counting NMC
    if(!cpu->isMC){
        cpu->isMC = true;
        cpu->acc[ACC_SI_NMC_COUNT] += curTick() - cpu->NMC_startTick;
    }

    // check if tb has enough tokens
    if(buckets[pkt_tag]->test_and_get()){     
        // if true, tell memobj to sendPacket(like always)
        return true;
    }
    else{      
        // if false, add req to waiting queue 
        buckets[pkt_tag]->add_request(lreq, false);
        return false;
    }
}


/// used to handle resps from memctrl to cpus
void
Scheduler::handle_response(PacketPtr pkt){
    // Check in pending req
    int rid = pkt->requestorId();
    CpuInfo *cpu = &info[CORE(rid)];
    bool found = false;
    for(auto it = cpu->pending_req.begin(); it!= cpu->pending_req.end(); it++){
        LabeledReq *lreq = (*it);
        // if found the matching request
        if(req_match(lreq->pkt, pkt)){
            //[Ivy TODO] pkt->req guarantees the same request, may consider delete this "if"
            if(lreq->time_return) continue;

            // fill its time_return
            lreq->time_return = curTick();

            // inc counters (read/write)
            if(pkt->isRead()){
                cpu->acc[ACC_SI_READ_T]++;
                cpu->acc[ACC_UI_READ_T]++;
            }
            else if(pkt->isWrite()){
                cpu->acc[ACC_SI_WRITE_T]++;
                cpu->acc[ACC_UI_WRITE_T]++;
            }
            else{//[Ivy TODO] 
                std::cout<<"other: "<< pkt->cmdString() <<std::endl;
            }

            // inc counters (sample all)
            cpu->acc[ACC_SI_LATENCY] += lreq->time_return - lreq->time_sent;
            
            // then we can delete it from pending queue
            delete lreq;
            cpu->pending_req.erase(it);
            //[CAUTION] since we exit after deletion, the above code will not cause problem
            // if we wish to continue the iteration, it needs review

            found = true;
            
            // for counting NMC
            if(cpu->pending_req.empty()){
                cpu->isMC = false;
                cpu->NMC_startTick = curTick();
            }

            break;
        }
    }
    assert(found && "No matching request found in pending list");
}

/// for counting NMC at the end of every sampling interval
void
Scheduler::count_NMC()
{
    for(int i=0; i <= num_cpus; i++){
        if(!info[i].isMC){
            info[i].acc[ACC_SI_NMC_COUNT] += curTick() - info[i].NMC_startTick;
            info[i].NMC_startTick = curTick();
        }
    }
}

/// operate slowdown pred (every sampling interval)
void
Scheduler::operate_slowdown_pred(){
    count_NMC();
#ifdef SLOWDOWN_PRED
    // pass info[0] [Ivy TODO]
    for(int i=1; i<=num_cpus; i++){
        SimpleMemobj *memobj = (SimpleMemobj *)obj;
        /// for timing simple CPU
        // TimingSimpleCPU *cpui = (TimingSimpleCPU *)(memobj->system()->getRequestors(4*i+1)->obj);
        // Counter cur_instcnt = cpui->threadInfo[cpui->curThread]->numInst;
        /// for O3 CPU
        DerivO3CPU *cpui = (DerivO3CPU *)(memobj->system()->getRequestors(4*i+1)->obj);
        uint64_t cur_instcnt = cpui->thread[0]->numInst; 


        // TEST OUTPUT
        LOG(DEBUG, "cpu %d inst: %ld -> %lu", i, info[i].last_instcnt, cur_instcnt);

        /// count slowdown
        // same #inst, (mix_time/solo_time) - 1 is slowdown
        //____________________________________________________________
        //      |      ticks            |   insts
        //______|_______________________|_____________________________
        // solo | cr->Interval_Ticks()  | cur_instcnt - last_instcnt
        // mix  | SAMPLING INTERVAL     | same as above
        //______|_______________________|_____________________________
        
        uint64_t solo_interval_ticks = info[i].cr->Interval_Ticks(cur_instcnt);
        info[i].last_instcnt = cur_instcnt;
        double act_sd = ((double)SAMPLING_INTERVAL/solo_interval_ticks - 1.0);

        //[Ivy TODO] we use actual slowdown instead of predicted slowdown
        info[i].slowdown_vec.push_back(act_sd);
    }
#else
    for(int i=1; i<=num_cpus; i++){
        info[i].slowdown_vec.push_back(0.09);
    }
#endif

}

/// update token bucket params every updating interval
void
Scheduler::update_token_bucket(){
    // calculate average slowdown in the ui
    std::vector<double> avg_sd;
    avg_sd.push_back(0.0);
    for(int i=1; i <=num_cpus; i++){
        assert(info[i].slowdown_vec.size()>2);
        double max_sd = *(std::max_element(info[i].slowdown_vec.begin(),info[i].slowdown_vec.end()));
        double min_sd = *(std::min_element(info[i].slowdown_vec.begin(),info[i].slowdown_vec.end()));
        double sum_sd = std::accumulate(info[i].slowdown_vec.begin(),info[i].slowdown_vec.end(), 0.0);
        double avg = (double)(sum_sd - max_sd - min_sd) / (info[i].slowdown_vec.size() - 2);

#ifdef PRINT_SLOWDOWN
        std::cout << "cpu " << i << " sd: " << avg << std::endl;
#endif
        avg_sd.push_back(avg);
        info[i].slowdown_vec.clear();
    }

    if(policy == Policy::CORE0_T){

        //[Ivy TODO]

        int tokens_inc = 0;
        if(avg_sd[1] > 0.3)
        {
            //[Ivy TODO]
            uint64_t bandwidth = info[1].acc[ACC_UI_READ_T] + info[1].acc[ACC_UI_WRITE_T];
            int tmp_inc_diff = (num_cpus - 1) * (avg_sd[1] - 0.1) * bandwidth * buckets[1]->freq / UPDATING_INTERVAL;
            tmp_inc_diff = std::min(buckets[0]->inc - 1, tmp_inc_diff);
            tokens_inc -= std::max(buckets[0]->inc / 2, tmp_inc_diff);
            std::cout << "tmp_inc " << tmp_inc_diff << " " << buckets[0]->inc << std::endl;
        }
        else if(avg_sd[1] > 0.1)
        {
            tokens_inc = -((avg_sd[1] - 0.1) / 0.05 + 1) * (num_cpus - 1);
        }
        else if(avg_sd[1] <= 0.08)
        {
            tokens_inc = num_cpus - 1;
        }
        buckets[0]->set_inc(buckets[0]->inc + tokens_inc);
    












    }
    else if(policy == Policy::ALL){
        return;
    }
}







/// get a waiting req from token buckets for memobj to send
PacketPtr 
Scheduler::get_waiting_req(){
    for(int i = 0; i <= num_tags; i++){
        LabeledReq *lreq = buckets[i]->get_request();
        if(lreq){
            return lreq->pkt; 
        }
    }
    return NULL;
}

