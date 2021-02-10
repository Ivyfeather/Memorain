#include "slowdown_pred.h"

uint64_t CycleRecorder::Interval_Ticks(int new_inst){
    int64_t instcnt;
    uint64_t curtick, interval;
    bool found = false;
    if(new_inst == lastsi_instcnt){
        printf("WWW same inst\n");
        return 10000;
    }
    while(fscanf(fp, "%lu: system.cpu: inst cnt %ld\n", &curtick, &instcnt)!=-1){
        if(instcnt == new_inst){
            interval = curtick - lastsi_instTick;
            lastsi_instTick = curtick;
            lastsi_instcnt = new_inst;
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