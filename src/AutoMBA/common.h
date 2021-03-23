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

#define AUTOMBA_ENABLE

#define NUM_CPUS 8
#define NUM_REQS (3 + NUM_CPUS*4) // number of requestors
#define SAMPLING_INTERVAL 50000000 
#define UPDATING_INTERVAL 250000000

#define SOLO_TRACE_PATH "/home/chenxi/Memorain/result/solo_stream_l2.log"

// gem5 Global frequency set at 1000000000000 ticks per second

// To print memory access log, use --debug-flag=SimpleMemobj 
// To record solo trace, use --debug-flag=SoloTrace

#endif
// 1cycle~500Tick
// 10w~5000w
// 300~15w