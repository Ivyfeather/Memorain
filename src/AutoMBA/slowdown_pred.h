#ifndef __SLOWDOWN_PRED_H_
#define __SLOWDOWN_PRED_H_

#include <cstdio>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "tree_model.h"

class CycleRecorder{
private:
    FILE *fp;
    uint64_t lastsi_instTick = 0;

public:
    CycleRecorder(const char *filename){
        fp = fopen(filename, "r");
        assert(fp);
    }
    ~CycleRecorder(){
        fclose(fp);
    }

    /* used to count ticks from lastsi_inst to new_inst
    ※ be very careful, new_inst we input every time must be 
    in ascending order ※ */
    uint64_t Interval_Ticks(int new_inst);
};

class SlowdownEstimator {
private:
    const static bool RUN_SLOWDOWN_PREDICTION = (NUM_CPUS > 1);
    
    Classifier *clf;
public:
    SlowdownEstimator() {
        FILE *fp = fopen("/home/chenxi/Memorain/util/AutoMBA/slowdown_tree.txt", "r");
        assert(fp && "open slowdown_tree.txt failed!!");
        int outputs[] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
        };
        clf = new Classifier(fp, 21, outputs);
        fclose(fp);
    };
    double estimate(std::vector<double> *inputs);
};

#endif
