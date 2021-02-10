#include "latency_pred.h"
// [Ivy TODO] reconstruct this file
void LatencyPred::add(uint64_t time_sent, uint64_t addr, int write) {
    uint32_t idx = get_history_table_index(addr);
    for (int i = 1; i < NUM_HISTORY_LENGTH; i++) {
        memcpy(&history_table[idx][i-1], &history_table[idx][i], sizeof(LatencyHistoryReg));
    }
    history_table[idx][LAST_REQ_IDX].time_sent = time_sent;
    history_table[idx][LAST_REQ_IDX].addr = addr;
    history_table[idx][LAST_REQ_IDX].channel = get_channel(addr);
    history_table[idx][LAST_REQ_IDX].rank = get_rank(addr);
    history_table[idx][LAST_REQ_IDX].bank = get_bank(addr);
    history_table[idx][LAST_REQ_IDX].column = get_column(addr);
    history_table[idx][LAST_REQ_IDX].row = get_row(addr);
    history_table[idx][LAST_REQ_IDX].write = write;
}

bool LatencyPred::get_prediction_inputs(uint32_t idx, std::vector<double> *inputs) {
    for (int i = 0; i < LAST_REQ_IDX; i++) {
        double time_sent_cycle = history_table[idx][LAST_REQ_IDX].time_sent - history_table[idx][i].time_sent;
        time_sent_cycle = std::min(time_sent_cycle, 16383500.0);
        double row_diff = (history_table[idx][LAST_REQ_IDX].row == history_table[idx][i].row) ? 0 : 1;
        double col_diff = history_table[idx][LAST_REQ_IDX].column - history_table[idx][i].column;
        inputs->push_back(time_sent_cycle);
        inputs->push_back(row_diff);
        inputs->push_back(col_diff);
    }
    return true;
}

bool LatencyPred::ack(uint64_t time_return, uint64_t addr, int write, int &est_latency) {
    uint32_t idx = get_history_table_index(addr);
    // if there're less than NUM_HISTORY_LENGTH requests, ignore it
    if (!history_table[idx][0].addr)
        return false;
    else if (history_table[idx][LAST_REQ_IDX].addr == addr) {
        std::vector<double> inputs;
        assert(get_prediction_inputs(idx, &inputs));

        if (PRINT_LATENCY_PRED_TRAINGING) {
            int latency = time_return - history_table[idx][LAST_REQ_IDX].time_sent;
            std::cout << "Train_data: ";
            for (auto x : inputs) {
                std::cout << (int64_t)x << " ";
            }
            std::cout << latency << std::endl;
        }
        
        if (!RUN_LATENCY_PREDICTION) {
            est_latency = 360;
            return true;
        }
        est_latency = clf->predict(&inputs);
        return true;
    }
    else {
        // if it's not the last request, we ignore it
        for (int i = NUM_HISTORY_LENGTH-2; i >= 0; i--) {
            if (history_table[idx][i].addr == addr)
                return false;
        }
        assert(0 && "outstanding > NUM_HISTORY_LENGTH");
    }
    return false;
}


