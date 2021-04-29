#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <set>
#include <random>
#include <string>
#include <iomanip>
#include <vector>
#include "log.hh"

#define CONTROLL_ENABLE
#define SLOWDOWN_PRED
#define PRINT_SLOWDOWN
#define PRINT_ACCUMULATORS
#define PRINT_TB_PARAMETERS

#define SAMPLING_INTERVAL 50000000 
#define UPDATING_INTERVAL 500000000

// 0:wb, 1:func, 2:int;
// 3~6: cpu0; 7~10: cpu1...
#define CORE(requestor_id) ((int)(requestor_id+1)/4)
#define CRID(requestor_id) ((int)(requestor_id+1)%4)

// gem5 Global frequency set at 1000000000000 ticks per second

// To print memory access log, use --debug-flag=MemLog 

// To record solo trace, use --debug-flag=SoloTrace
//  then use traceproc.sh to compress tracefile

// DRAM address mapping in "info.hh"

#endif
// 1cycle~500Tick
// 10w~5000w
// 300~15w