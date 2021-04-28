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
// #define SLOWDOWN_PRED
#define PRINT_ACCUMULATORS
#define PRINT_TB_PARAMETERS

#define SAMPLING_INTERVAL 50000000 
#define UPDATING_INTERVAL 250000000

// 0:wb, 1:func, 2:int;
// 3~6: cpu0; 7~10: cpu1...
#define CORE(requestor_id) ((int)(requestor_id+1)/4)
#define CRID(requestor_id) ((int)(requestor_id+1)%4)

// gem5 Global frequency set at 1000000000000 ticks per second

// To print memory access log, use --debug-flag=MemLog 
// To record solo trace, use --debug-flag=SoloTrace

// DRAM addr 
// from Memorain/ext/dramsim3/DRAMsim3/configs/DDR4_8Gb_x8_2400.ini
enum{
    CHANNEL, RANK, BANK_GROUP, BANK, COLUMN, ROW,    BYTES};
uint8_t OFFSET[7] = 
    {18,     17,   13,         15,   6,      18,     0};
uint8_t MASK[7] = 
    {0x0,    0x1,  0x3,        0x3,  0x7f,   0xffff, 0x3f};            

// |       | row     | channel | rank | bank     | bank group | column | byte in burst |
// | ----- | ------- | ------- | ---- | -------- | ---------- | ------ | ------------- |
// |  len  | 16      | 0       | 1    | 2        | 2          | 7      | 6             |
// |  bits | 34   18 | (18)    | 17   | 16    15 | 14    13   | 12   6 | 5   0         |


#endif
// 1cycle~500Tick
// 10w~5000w
// 300~15w