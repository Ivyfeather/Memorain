#include "latency_pred.h"

void LatencyPred::add(uint64_t depart, uint64_t addr, int write) {
    uint32_t idx = get_history_table_index(addr);
    for (int i = 1; i < NUM_HISTORY_LENGTH; i++) {
        memcpy(&history_table[idx][i-1], &history_table[idx][i], sizeof(LatencyHistoryReg));
    }
    history_table[idx][LAST_REQ_IDX].depart = depart;
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
        double depart_cycle = history_table[idx][LAST_REQ_IDX].depart - history_table[idx][i].depart;
        depart_cycle = std::min(depart_cycle, 32767.0);
        double row_diff = (history_table[idx][LAST_REQ_IDX].row == history_table[idx][i].row) ? 0 : 1;
        double col_diff = history_table[idx][LAST_REQ_IDX].column - history_table[idx][i].column;
        inputs->push_back(depart_cycle);
        inputs->push_back(row_diff);
        inputs->push_back(col_diff);
    }
    return true;
}

bool LatencyPred::ack(uint64_t arrive, uint64_t addr, int *est_latency) {
    uint32_t idx = get_history_table_index(addr);
    // if there're less than NUM_HISTORY_LENGTH requests, ignore it
    if (!history_table[idx][0].addr)
        return false;
    else if (history_table[idx][LAST_REQ_IDX].addr == addr) {
        std::vector<double> inputs;
        assert(get_prediction_inputs(idx, &inputs));

        if (PRINT_LATENCY_PRED_TRAINGING) {
            int latency = arrive - history_table[idx][LAST_REQ_IDX].depart;
            std::cout << "Train_data: ";
            for (auto x : inputs) {
                std::cout << x << " ";
            }
            std::cout << latency << std::endl;
        }
        
        if (!RUN_LATENCY_PREDICTION) {
            *est_latency = 360;
            return true;
        }
        *est_latency = clf->predict(&inputs);
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


