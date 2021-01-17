#ifndef __LATENCY_PRED_H_
#define __LATENCY_PRED_H_


#include <algorithm>
#include "common.h"
#include "tree_model.h"

typedef struct {
    uint64_t depart;
    uint64_t addr;
    uint8_t channel;
    uint8_t rank;
    uint8_t bank;
    uint8_t column;
    uint16_t row;
    int write;
} LatencyHistoryReg;

class LatencyPred {
private:
    const static bool RUN_LATENCY_PREDICTION = (NUM_CPUS > 1);
    const static bool PRINT_LATENCY_PRED_TRAINGING = false;

    const static int NUM_HISTORY_TABLES = 1 << 6;
    const static int NUM_HISTORY_LENGTH = 9;
    const static int LAST_REQ_IDX = NUM_HISTORY_LENGTH - 1;
    LatencyHistoryReg history_table[NUM_HISTORY_TABLES][NUM_HISTORY_LENGTH];
    
    Classifier *clf;

    /// maually get channel&rank&bank...  [Ivy?]
    inline uint8_t get_channel(uint64_t addr) { return (addr >> 6) & 0x3; }
    inline uint8_t get_rank(uint64_t addr) { return (addr >> 14) & 0x1; }
    inline uint8_t get_bank(uint64_t addr) { return (addr >> 15) & 0x7; }
    inline uint8_t get_column(uint64_t addr) { return (addr >> 8) & 0x3f; }
    inline uint16_t get_row(uint64_t addr) { return (addr >> 18) & 0x3fff; }
    inline uint32_t get_history_table_index(uint64_t addr) {
        return (get_channel(addr) << 4) | (get_rank(addr) << 3) | get_bank(addr);
    }

    bool get_prediction_inputs(uint32_t idx, std::vector<double> *inputs);
public:
    LatencyPred() {
        memset(history_table, 0, NUM_HISTORY_LENGTH*NUM_HISTORY_TABLES*sizeof(LatencyHistoryReg));
        FILE *fp = fopen("/home/chenxi/Memorain/util/AutoMBA/latency_tree.txt", "r");
        assert(fp && "open latency_tree.txt failed!!");
        /// what is this [Ivy?]
        int outputs[] = {
            268, 269, 279, 281, 284, 286, 306, 307, 309, 311, 
            314, 316, 319, 321, 322, 324, 326, 329, 338, 339
        };
        clf = new Classifier(fp, 20, outputs);
        fclose(fp);
    };

    void add(uint64_t depart, uint64_t addr, int write);
    bool ack(uint64_t arrive, uint64_t addr, int *est_latency);
    
};

#endif
