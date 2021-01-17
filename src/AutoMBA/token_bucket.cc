#include "latency_pred.h"
#include "token_bucket.h"

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

bool TokenBucket::get_request(LabeledReq *&request) {
    if ((bypass || inc == size || tokens) && !waiting_queue.empty()) {
        if (!bypass && tokens)
            tokens--;
        request = waiting_queue.front();
        waiting_queue.pop();
        return true;
    }
    return false;
}

void TokenBucket::operate() {
    if (cycle_counter + 1 == freq) {
        if (!bypass)
            tokens = std::min(size, tokens + inc);
    }
    cycle_counter = (cycle_counter + 1) % freq;
}
