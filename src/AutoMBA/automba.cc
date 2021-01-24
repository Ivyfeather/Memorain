#include "automba.h"

/* util func */
/// get requestor ID of a packet
RequestorID
get_packet_req_id(PacketPtr pkt)
{
    return pkt->req->requestorId();
}

//[Ivy TODO] test whether pkt1->req==pkt2->req works directly
bool
req_match(PacketPtr pkt1, PacketPtr pkt2)
{
    // return get_packet_req_id(pkt1)==get_packet_req_id(pkt2) && pkt1->getAddr()==pkt2->getAddr();
    return pkt1->req == pkt2->req;
}

/* util func end*/

void
AutoMBA::print_si_accumulators()
{
    if (!PRINT_ACCUMULATORS)
        return;
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
    // if (!PRINT_TB_PARAMETERS)
    //     return;
    // for (int i = 0; i < NUM_CPUS; i++) {
    //     // print token buckets' parameters
    //     std::cout << "token_bucket[" << i << "] parameters: ";
    //     std::cout << buckets[i]->get_size() << " " << buckets[i]->get_freq() << " "
    //               << buckets[i]->get_inc() << " " << buckets[i]->get_bypass() << " "
    //               << buckets[i]->get_tokens() << std::endl;
    // }
}



AutoMBA::AutoMBA()
{
    // initialize token buckets
    int init_size = 60, init_freq = 5000, init_inc = 40;
    for(int i = 0; i < NUM_TAGS; i++){
        int cnt = 0;
        // count the number of cores with tag i
        for(int j = 0; j < NUM_CPUS; j++){
            if(core_tags[j] == i) cnt++;
        }
    #ifdef AUTOMBA_ENABLE

    #else

    #endif
    }

}

AutoMBA::~AutoMBA()
{
    
}

bool
AutoMBA::handle_request(PacketPtr pkt)
{
    LabeledReq *lreq = new LabeledReq(pkt, curTick());

    //[Ivy] we no longer use an arbiter to choose which tb to get a req from & send the req
    //  because champsim does it every cycle, but gem5 does this at handle_req action

    // new implementation is 
    // check if token_bucket[reqid] has enough token
    
    int pkt_tag = core_tags[pkt->requestorId()];
    if(buckets[pkt_tag]){
        
    }
    else{
        return false;
    }


    // if true -> tell memobj to sendPacket(like always)

    // if false -> tell memobj to return false



    // Put req into pending queue
    int rid = get_packet_req_id(lreq->pkt);
    pending_req[rid].push_back(lreq);

    // Send req to latency predicting model
    lpm[rid].add(curTick(), pkt->getAddr(), pkt->isWrite());

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
            break;
        }
    }
    assert(found && "No matching request found in pending list");
}

void
AutoMBA::update_token_bucket()
{
    //[Ivy TODO] in old code, if NUM_CPUS > 1
    if(policy != Policy::CORE0_T){
        // we have not implemented other policies
        return;
    }else{
        //[Ivy TODO] we just take core 0 as QoS()
        assert(slowdown_vec[0].size()>2);
        double max_sd = *(std::max_element(slowdown_vec[0].begin(),slowdown_vec[0].end()));
        double min_sd = *(std::min_element(slowdown_vec[0].begin(),slowdown_vec[0].end()));
        double sum_sd = std::accumulate(slowdown_vec[0].begin(),slowdown_vec[0].end(), 0.0);

        double avg_sd = 0.05 * (sum_sd - max_sd - min_sd) / (slowdown_vec[0].size() - 2);
        int tokens_inc;
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
    for(int i = 0; i < NUM_CPUS; i++){
        slowdown_vec[i].clear();
    }
}