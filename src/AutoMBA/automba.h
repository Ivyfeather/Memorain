#ifndef __AUTOMBA_H_
#define __AUTOMBA_H_

#define NUM_CPUS 8
#define SAMPLING_INTERVAL 1000000 //change to 10_0000 later
#define UPDATING_INTERVAL 5000000

class Arbiter { //仲裁
public:
    // tags: application info
    // status: tb status, currently refers to #waiting requests
    int get_core(int tag[NUM_CPUS], int status[NUM_CPUS]);
};

class AutoMBA{
private:
    const static bool PRINT_ACCUMULATORS = true;
    const static bool PRINT_TB_PARAMETERS = true;

    // std::vector<LabeledReq *> pending_req[NUM_CPUS];
    Arbiter arbiter;

};

#endif
