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

#define NUM_CPUS (3 + 8*4)
#define SAMPLING_INTERVAL 50000000 
#define UPDATING_INTERVAL 250000000

// gem5 Global frequency set at 1000000000000 ticks per second
#endif