#ifndef __SLOWDOWN_H_
#define __SLOWDOWN_H_

#include "common.hh"

class CycleRecorder{
private:
    FILE *fp;
    uint64_t last_insttick = 0;

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
    uint64_t Interval_Ticks(int new_inst){
        int64_t instcnt;
        uint64_t cur_insttick, interval_ticks;
        bool found = false;
        while(fscanf(fp, "%lu: system.cpu: inst cnt %ld\n", &cur_insttick, &instcnt)!=-1){
            if(instcnt == new_inst){
                interval_ticks = cur_insttick - last_insttick;
                last_insttick = cur_insttick;
                found = true;
                break;
            }
        }
        assert(found && "inst cnt > solo_trace");
        return interval_ticks;
    }
};


#endif