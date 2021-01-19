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

}

AutoMBA::~AutoMBA()
{
    
}

void
AutoMBA::handle_request(PacketPtr pkt)
{
    LabeledReq *lreq = new LabeledReq(pkt, curTick());

    //[Ivy TODO] operate tb, put this req into tb[tag] and send req from tb
    // for now, we directly forward it

    // Put req into lpm
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