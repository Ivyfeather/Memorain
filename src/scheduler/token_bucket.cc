#include "token_bucket.hh"

#ifdef DRAMSIM
bool bank_conflict(LabeledReq *a, LabeledReq* b){
    return  (a->addr.bank_group == b->addr.bank_group) && \
            (a->addr.bank       == b->addr.bank      ) && \
            (a->addr.row        != b->addr.row);
}

bool row_buffer_hit(LabeledReq *a, LabeledReq* b){
    return  (a->addr.bank_group == b->addr.bank_group) && \
            (a->addr.bank       == b->addr.bank      ) && \
            (a->addr.row        == b->addr.row);
}
#else
bool bank_conflict(LabeledReq *a, LabeledReq* b){
    return  (a->addr.bank       == b->addr.bank      ) && \
            (a->addr.row        != b->addr.row);
}

bool row_buffer_hit(LabeledReq *a, LabeledReq* b){
    return  (a->addr.bank       == b->addr.bank      ) && \
            (a->addr.row        == b->addr.row);
}
#endif

void TokenBucket::add_request(LabeledReq *request, bool head=false) {
#ifdef TB_REORDER
    int len = waiting_queue.size();
    int B[len-1], C[len-1];
    if(len==0){
        waiting_queue.push_back(request);
        return;
    }

    for(int i=0; i<len-1; i++){
        LabeledReq *nreq = waiting_queue[i];
        if(row_buffer_hit(nreq, request)){
            waiting_queue.insert(waiting_queue.begin()+i, request);
            return;
        }
        B[i]=0;C[i]=0;
        LabeledReq *nnreq = waiting_queue[i+1];
        if(!bank_conflict(nreq, request) && !bank_conflict(nnreq, request)){
            B[i]=1;
        }
        if(bank_conflict(nreq, nnreq)){
            C[i]=1;
        }
    }

    for(int i=0; i<len-1; i++){
        if(B[i] && C[i]){
            waiting_queue.insert(waiting_queue.begin()+i, request);
            return;
        }
    }

    for(int i=0; i<len-1; i++){
        if(B[i]){
            waiting_queue.insert(waiting_queue.begin()+i, request);
            return;
        }
    }

    waiting_queue.push_back(request);
    return;
                
#else
    if(head){
        waiting_queue.push_front(request);
    }else{
        waiting_queue.push_back(request);
    }
#endif
}

LabeledReq* TokenBucket::get_request() {
    // no req in waiting queue
    if(waiting_queue.empty()){
        return NULL;
    }
    if(bypass || inc == size || tokens > 0){
        if(tokens > 0) tokens --;
        LabeledReq* req = waiting_queue.front();
        waiting_queue.pop_front(); 
        return req;
    }
    return NULL;
}

void TokenBucket::add_tokens(){
    if(!bypass)
        tokens = std::min(size, tokens + inc);
}

bool TokenBucket::test_and_get(){
    if(bypass || inc == size || tokens > 0){
        if(tokens > 0) tokens --;
        return true;
    }
    return false;
}


/*
void TokenBucket::reorder_reqs(){
    for(int i=0; i<waiting_queue.size(); i++){
        LabeledReq *nreq = waiting_queue[i];
        for(int j=0; j<i; j++){
            LabeledReq *jreq = waiting_queue[j];
            if(row_buffer_hit(nreq, jreq)){
                if(j!=0){
                    // swap i and j-1
                    LabeledReq *tmp = waiting_queue[j-1];
                    waiting_queue[j-1] = nreq;
                    waiting_queue[i] = tmp;
                    break;
                }else{
                    LabeledReq *tmp = waiting_queue[j+1];
                    waiting_queue[j+1] = nreq;
                    waiting_queue[i] = tmp;
                    break;
                }
            }
        }
    }
}
void TokenBucket::reorder_reqs(){
    std::vector<int> conflict_dist;
    conflict_dist.push_back(10000000);//INT_MAX
    std::deque<LabeledReq *> ordered_queue;

    // assume queue[0 ~ i-1] already in desired order
    // now we pick out queue[i] and put it to the right place
    for(auto it=waiting_queue.begin(); it!=waiting_queue.end(); it++){
        LabeledReq *nreq = (*it);
        bool inserted = false;

        int last_bankconflict = -1; int i = 0;
        // search for reqs in the same row
        // if there is, insert
        for(auto th=ordered_queue.begin(); th!=ordered_queue.end(); th++){
            LabeledReq *oreq = (*th);
            if(row_buffer_hit(nreq, oreq)){
                ordered_queue.insert(th, nreq);
                inserted = true;
                break;
            }
            else if(bank_conflict(nreq, oreq)){
                last_bankconflict = i;
            }
            i++;
        }


        //[Ivy TODO] if row-buffer not hit
        // ordered_queue.push_back(nreq);


        // insert nreq after the last bank conflict req in ordered_queue
        //  and to split the two reqs with min conflict distance
        if(last_bankconflict == -1) last_bankconflict = 0;
        auto minconflict = std::min_element(conflict_dist.begin()+last_bankconflict, conflict_dist.end());
        // no other two reqs with bank conflict
        if(*minconflict == 0){
            // insert to queue end
            // ordered_queue
            
            conflict_dist.push_back(1);
        }else{
        int minposition = minconflict - conflict_dist.begin();
        
        // calculate new conflict dist
        }
    }
    waiting_queue.swap(ordered_queue);

}
*/