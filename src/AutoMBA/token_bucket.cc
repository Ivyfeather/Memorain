#include "latency_pred.h"
#include "token_bucket.h"

// add req to waiting queue
void TokenBucket::add_request(LabeledReq *request, bool head=false) {
    waiting_queue.push(request);
    if (head) {
        int num = waiting_queue.size() - 1;
        while (num > 0) {
            auto req = waiting_queue.front();
            waiting_queue.pop();
            waiting_queue.push(req);
            num--;
        }
    }
}
/*
bool TokenBucket::get_request(LabeledReq *&request) {
    if ((bypass || inc == size || tokens) && !waiting_queue.empty()) {
        if (!bypass && tokens)
            tokens--;
        request = waiting_queue.front();
        waiting_queue.pop();
        return true;
    }
    return false;
*/

LabeledReq* TokenBucket::get_request() {
    // no req in waiting queue
    if(waiting_queue.empty()){
        return NULL;
    }
    if(bypass || inc == size || tokens > 0){
        tokens --;
        LabeledReq* req = waiting_queue.front();
        waiting_queue.pop(); //pop_front
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
        tokens --;
        return true;
    }
    return false;
}