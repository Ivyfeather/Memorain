#ifndef __SLOWDOWN_PRED_H_
#define __SLOWDOWN_PRED_H_

#include <cstdio>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "tree_model.h"

class CycleRecorder {
private:
    bool load;
    char fname[1024];
    FILE* file;

    const int BUF_SIZE = 4096;
    int pos;
    uint64_t *buf;
    uint64_t last_instr = 0;
public:
    CycleRecorder(const char *filename, bool load_from_file) {
        load = load_from_file;
        if (load_from_file) {
            // file = fopen(filename, "rb");
            strcpy(fname, filename);
        }
        else {
            file = fopen(filename, "wb");
            assert(file);
            buf = new uint64_t[BUF_SIZE];
            pos = 0;
        }
    };

    void update(uint64_t cycle, uint64_t instr) {
        assert(!load);
        while (last_instr < instr) {
            buf[pos++] = cycle;
            if (pos == BUF_SIZE) {
                fwrite(buf, sizeof(uint64_t), BUF_SIZE, file);
                pos = 0;
            }
            last_instr++;
        }
    }

    void get(uint64_t *cycle, uint64_t instr) {
        assert(load);
        // assert(load && instr >= last_instr);
        file = fopen(fname, "rb");
        // assert(file);
        while (!file) {
            file = fopen(fname, "rb");
        }
        uint64_t offset = (instr - 1) * sizeof(uint64_t);
        fseek(file, offset, SEEK_SET);
        fread(cycle, sizeof(uint64_t), 1, file);
        last_instr = instr;
        fclose(file);
    }

    void finish() {
        if (load) {
            // fclose(file);
        }
        else {
            if (pos > 0)
                fwrite(buf, sizeof(uint64_t), pos, file);
            fclose(file);
        }
    }
};

class SlowdownEstimator {
private:
    const static bool RUN_SLOWDOWN_PREDICTION = (NUM_CPUS > 1);
    
    Classifier *clf;
public:
    SlowdownEstimator() {
        FILE *fp = fopen("slowdown_tree.txt", "r");
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
