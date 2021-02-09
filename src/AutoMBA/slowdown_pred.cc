#include "slowdown_pred.h"

uint64_t CycleRecorder::Interval_Ticks(int new_inst){
    int64_t instcnt;
    uint64_t curtick, interval;
    bool found = false;
    
    while(fscanf(fp, "%lu: system.cpu: inst cnt %ld\n", &curtick, &instcnt)!=-1){
        if(instcnt == new_inst){
            interval = curtick - lastsi_instTick;
            lastsi_instTick = curtick;
            found = true;
            break;
        }
    }
    assert(found && "inst cnt > solo_trace");
    return interval;
}


double SlowdownEstimator::estimate(std::vector<double> *inputs) {
    // TODO
    if (RUN_SLOWDOWN_PREDICTION)
        return clf->predict(inputs);
    else
        return 0.1;
}