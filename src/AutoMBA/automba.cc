#include "automba.h"
#include "learning_gem5/part2/simple_memobj.hh"
#include "cpu/simple/timing.hh"
/* util func */
/// get requestor ID of a packet
RequestorID
get_packet_req_id(PacketPtr pkt)
{
    return pkt->req->requestorId();
}

bool
req_match(PacketPtr pkt1, PacketPtr pkt2)
{
    return pkt1->req == pkt2->req;
}

/* util func end*/

void
AutoMBA::print_si_accumulators()
{
    if (!PRINT_ACCUMULATORS)
        return;
    std::cout << "curTick" << curTick() << std::endl;
    for (int i = 0; i <= NUM_CPUS; i++) {
        // std::cout << i << std::endl;
        // print sampling interval accumulators
        std::cout << "si_accumulators[" << i << "]: ";
        int num_acc = (i == NUM_CPUS) ? (int)ACC_ALL_MAX : (int)ACC_SI_MAX;
        for (int j = 0; j < num_acc; j++) {
            if (j == num_acc - 1) {
                std::cout << acc[i][j] << std::endl;
            }
            else {
                std::cout << acc[i][j] << " ";
            }
        }
    }
}

void
AutoMBA::print_ui_accumulators()
{
    if (!PRINT_ACCUMULATORS)
        return;
    for (int i = 0; i < NUM_CPUS; i++) {
        // print updating interval accumulators
        std::cout << "ui_accumulators[" << i << "]: ";
        for (int j = ACC_SI_MAX+1; j < ACC_UI_MAX; j++) {
            if (j == ACC_UI_MAX - 1) {
                std::cout << acc[i][j] << std::endl;
            }
            else {
                std::cout << acc[i][j] << " ";
            }
        }
    }
}

void
AutoMBA::reset_si_accumulators()
{
    for (int i = 0; i < NUM_CPUS; i++) {
        // reset sampling interval accumulators
        acc[i][ACC_SI_READ_T] = 0;
        acc[i][ACC_SI_WRITE_T] = 0;
        acc[i][ACC_SI_LATENCY] = 0;
        acc[i][ACC_SI_LATENCY_MODEL] = 0;
        acc[i][ACC_SI_LATENCY_SHDW] = 0;
        acc[i][ACC_SI_SAMPLING_NUM] = 0;
        acc[i][ACC_SI_NMC_COUNT] = 0;

    }
    acc[NUM_CPUS][ACC_ALL_READ_T] = 0;
    acc[NUM_CPUS][ACC_ALL_WRITE_T] = 0;
}

void
AutoMBA::reset_ui_accumulators()
{
    for (int i = 0; i < NUM_CPUS; i++) {
        // reset updating interval accumulators
        acc[i][ACC_UI_READ_T] = 0;
        acc[i][ACC_UI_WRITE_T] = 0;
    }
}

void AutoMBA::print_tb_parameters()
{
    if (!PRINT_TB_PARAMETERS)
        return;
    for (int i = 0; i < NUM_TAGS; i++) {
        // print token buckets' parameters
        std::cout << "token_bucket[" << i << "] parameters: ";
        std::cout << buckets[i]->get_size() << " " << buckets[i]->get_freq() << " "
                  << buckets[i]->get_inc() << " " << buckets[i]->get_bypass() << " "
                  << buckets[i]->get_tokens() << std::endl;
    }
}



AutoMBA::AutoMBA(void *in_obj)
{
    obj = in_obj;

    // initialize token buckets
    //[Ivy TODO] token bucket should be carefully set 
    int init_size = 60, init_freq = 1000000, init_inc = 10;
    for(int i = 0; i < NUM_TAGS; i++){
        int cnt = 0;
        // count the number of cores with tag i
        for(int j = 0; j < NUM_CPUS; j++){
            if(core_tags[j] == i) cnt++;
        }
#ifdef AUTOMBA_ENABLE
        // tb[0] bypass=false, others true
        buckets[i] = new TokenBucket(cnt*init_size, init_freq, cnt*init_inc, (i>=1));    
#else
        buckets[i] = new TokenBucket(cnt*init_size, init_freq, cnt*init_inc, true);    
#endif
    }

    if(SHOW_ACTUAL_SLOWDOWN){
        cr = new CycleRecorder("/home/chenxi/Memorain/result/solo_stream_l1.log");
    }
}

AutoMBA::~AutoMBA()
{
    for(int i = 0;i < NUM_TAGS; i++)
        delete buckets[i];
}

bool
AutoMBA::handle_request(PacketPtr pkt)
{
    //[Ivy] we no longer use an arbiter to choose which tb to get a req from & send the req
    //  because champsim does it every cycle, but gem5 does this at handle_req action

    // new implementation is 
    // check if token_bucket[reqid] has enough tokens
    int pkt_tag = core_tags[pkt->requestorId()];
    int rid = get_packet_req_id(pkt);

    // if tb allows to send, but memPort.sendPacket fails
    // then this req has been added into pending again, but not truely sent
    // when memctrl calls sendreqretry, this req will be added in pending queue once again

    // check if already in pending queue
    for(auto it = pending_req[rid].begin(); it!= pending_req[rid].end(); it++){
        LabeledReq *lreq = (*it);
        if(req_match(lreq->pkt, pkt))
            return true;
    }

    if(buckets[pkt_tag]->test_and_get()){
        // if true 
        // Put req into pending queue
        LabeledReq *lreq = new LabeledReq(pkt, curTick());
        
        pending_req[rid].push_back(lreq);

        // Send req to latency predicting model
        lpm[rid].add(curTick(), pkt->getAddr(), pkt->isWrite());

        // for counting NMC
        if(!isMC[rid]){
            isMC[rid] = true;
            acc[rid][ACC_SI_NMC_COUNT] += curTick() - NMC_startTick[rid];
        }
        
        // tell memobj to sendPacket(like always)
        return true;
    }
    else{
        // if false -> tell memobj to return false
        return false;
    }
}
    
void
AutoMBA::handle_response(PacketPtr pkt)
{
    // Check in pending req
    int rid = get_packet_req_id(pkt);
    bool found = false;
    for(auto it = pending_req[rid].begin(); it!= pending_req[rid].end(); it++){
        LabeledReq *lreq = (*it);
        // if found the matching request
        if(req_match(lreq->pkt, pkt)){
            //[Ivy TODO] shadow

            //[Ivy TODO] pkt->req guarantees the same request, may consider delete this "if"
            if(lreq->time_return) continue;

            // fill its time_return and send to lpm
            lreq->time_return = curTick();
            lreq->sampling = lpm[rid].ack(curTick(), pkt->getAddr(), pkt->isWrite(), lreq->est_latency);

            // inc counters (read/write)
            if(pkt->isRead()){
                acc[rid][ACC_SI_READ_T]++;
                acc[rid][ACC_UI_READ_T]++;
                acc[NUM_CPUS][ACC_ALL_READ_T]++;
            }
            else if(pkt->isWrite()){
                acc[rid][ACC_SI_WRITE_T]++;
                acc[rid][ACC_UI_WRITE_T]++;
                acc[NUM_CPUS][ACC_ALL_WRITE_T]++;
            }
            else{//[Ivy TODO] 
                std::cout<<"other: "<< pkt->cmdString() <<std::endl;
            }

            // inc counters (sampling or not)
            if(lreq->sampling){
                acc[rid][ACC_SI_LATENCY] += lreq->time_return - lreq->time_sent;
                acc[rid][ACC_SI_LATENCY_MODEL] += lreq->est_latency;
                acc[rid][ACC_SI_LATENCY_SHDW] += 0;//lreq->shadow_arrive - lreq->accept;
                acc[rid][ACC_SI_SAMPLING_NUM]++;
            }
            
            // then we can delete it from pending queue
            delete lreq;
            pending_req[rid].erase(it);
            //[CAUTION] since we exit after deletion, the above code will not cause problem
            // if we wish to continue the iteration, it needs review

            found = true;
            
            // for counting NMC
            if(pending_req[rid].empty()){
                isMC[rid] = false;
                NMC_startTick[rid] = curTick();
            }

            break;
        }
    }
    assert(found && "No matching request found in pending list");
}

void
AutoMBA::count_NMC()
{
    for(int i=0; i < NUM_CPUS; i++){
        if(!isMC[i]){
            acc[i][ACC_SI_NMC_COUNT] += curTick() - NMC_startTick[i];
            NMC_startTick[i] = curTick();
        }else{}
    }
}

void
AutoMBA::operate_slowdown_pred() 
{
    count_NMC();

    // predict slowdown for core 0 alone
    std::vector<double> slowdown_est_inputs;


    if(SHOW_ACTUAL_SLOWDOWN){
        SimpleMemobj *memobj = (SimpleMemobj *)obj;
        TimingSimpleCPU *cpu0 = (TimingSimpleCPU *)(memobj->system()->getRequestors(5)->obj);
        Counter curInst = cpu0->threadInfo[cpu0->curThread]->numInst;

        
        // TEST
        std::cout<<"actual: "<< lastsi_instCnt << " -> " << curInst << std::endl;
        uint64_t solo_interval_ticks = cr->Interval_Ticks(curInst);

        std::cout<< "solo tick interval " << solo_interval_ticks << std::endl;

        // same #inst, (mix_time/solo_time) - 1 is slowdown
        //      |      ticks            |   insts
        // solo | cr->Interval_Ticks()  | curInst - lastsi_instCnt
        // mix  | SAMPLING INTERVAL     | same

        lastsi_instCnt = curInst;
        double act_sd = ((double)SAMPLING_INTERVAL/solo_interval_ticks - 1.0);
        std::cout << "actual_slowdown " << act_sd << std::endl;


        //[Ivy TODO] for now, we use actual slowdown instead of predicted
        slowdown_vec.push_back(act_sd);

    }


    // int predicted_slowdown = estimator.estimate(&slowdown_est_inputs);
    // slowdown_vec.push_back(predicted_slowdown);
    // if (SHOW_PREDICTED_SLOWDOWN)
    //     std::cout << "predicted[" << 0 << "] " << predicted_slowdown << std::endl;



    //! static 
    /*
    static bool has_init[NUM_CPUS] = {false};
    static uint64_t last_solo_cycle[NUM_CPUS];
    for (int i = 0; i < NUM_CPUS; i++) {
        std::vector<double> slowdown_est_inputs;
        //如果至少有一条 已经返回的read
        if (acc[i][ACC_SI_READ_T]) {
            // [TODO] 100000 
            slowdown_est_inputs.push_back((double)(100000 - acc[i][ACC_SI_NMC_COUNT]) / acc[i][ACC_SI_READ_T]);
        }
        else {
            slowdown_est_inputs.push_back(0.0);
        }
        
        slowdown_est_inputs.push_back(acc[i][ACC_SI_WRITE_T]);
        if (acc[i][ACC_SI_LATENCY_MODEL] > 0) {
            double lats = (double)acc[i][ACC_SI_LATENCY] / acc[i][ACC_SI_LATENCY_MODEL];
            slowdown_est_inputs.push_back(lats);
        }
        else {
            slowdown_est_inputs.push_back(1);
        }
        slowdown_est_inputs.push_back(acc[i][ACC_SI_NMC_COUNT]);
        
        // if (SHOW_ACTUAL_SLOWDOWN) {
        //     if (!has_init[i]) {
        //         cr[i]->get(&last_solo_cycle[i], 0);
        //         has_init[i] = true;
        //     }
        //     uint64_t curr_instr = ooo_cpu[i].num_retired, curr_solo_cycle;
        //     cr[i]->get(&curr_solo_cycle, curr_instr);
        //     // 见git的README - 关键设计要点
        //     double actual_slowdown = ((double)sampling_interval / (curr_solo_cycle - last_solo_cycle[i])) - 1;
        //     std::cout << "actual_slowdown[" << i << "] ";
        //     for (auto x : slowdown_est_inputs) {
        //         std::cout << x << " ";
        //     }
        //     std::cout << actual_slowdown << std::endl;
        //     last_solo_cycle[i] = curr_solo_cycle;
        // }
        
        int predicted_slowdown = estimator.estimate(&slowdown_est_inputs);
        slowdown_vec[i].push_back(predicted_slowdown);
        if (SHOW_PREDICTED_SLOWDOWN)
            std::cout << "predicted[" << i << "] " << predicted_slowdown << std::endl;
    }*/
}


void
AutoMBA::update_token_bucket()
{
    //[Ivy TODO] in old code, if NUM_CPUS > 1
    if(policy != Policy::CORE0_T){
        // we have not implemented other policies
        return;
    }else{
        //we just take core 0 as QoS()
        assert(slowdown_vec.size()>2);
        double max_sd = *(std::max_element(slowdown_vec.begin(),slowdown_vec.end()));
        double min_sd = *(std::min_element(slowdown_vec.begin(),slowdown_vec.end()));
        double sum_sd = std::accumulate(slowdown_vec.begin(),slowdown_vec.end(), 0.0);

        double avg_sd = (sum_sd - max_sd - min_sd) / (slowdown_vec.size() - 2);
        int tokens_inc = 0;
        std::cout << "average actual_slowdown: "<< avg_sd << std::endl;
        if(avg_sd > 0.3)
        {
            //[Ivy TODO]
            uint64_t bandwidth = acc[0][ACC_UI_READ_T] + acc[0][ACC_UI_WRITE_T];
            int tmp_inc_diff = (NUM_CPUS - 1) * (avg_sd - 0.1) * bandwidth * buckets[1]->get_freq() / updating_interval;
            tmp_inc_diff = std::min(buckets[0]->get_inc() - 1, tmp_inc_diff);
            tokens_inc -= std::max(buckets[0]->get_inc() / 2, tmp_inc_diff);
            std::cout << "tmp_inc " << tmp_inc_diff << " " << buckets[0]->get_inc() << std::endl;
        }
        else if(avg_sd > 0.1)
        {
            tokens_inc = -((avg_sd - 0.1) / 0.05 + 1) * (NUM_CPUS - 1);
        }
        else if(avg_sd <= 0.08)
        {
            tokens_inc = NUM_CPUS - 1;
        }
        buckets[0]->set_inc(buckets[0]->get_inc() + tokens_inc);
    }
    
    slowdown_vec.clear();

}