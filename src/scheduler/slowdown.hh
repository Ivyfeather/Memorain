#ifndef __SLOWDOWN_H_
#define __SLOWDOWN_H_

#include "common.hh"

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


#endif