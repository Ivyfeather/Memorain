#ifndef __TREE_MODEL_H_
#define __TREE_MODEL_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <string.h>

bool get_next_line(FILE *fp, char *type, int *iarg, double *darg);

class Tree {
private:
    bool leaf;
    int idx;
    double val;
    Tree *left, *right;
public:
    Tree(FILE *fp) {
        assert(fp);
        char type;
        assert(get_next_line(fp, &type, &idx, &val));
        leaf = (type == 'c');
        if (!leaf) {
            left = new Tree(fp);
            right = new Tree(fp);
        }
    };
    ~Tree() {
        delete left;
        delete right;
    };
    void dump() {
        std::cout << idx << " " << val << std::endl;
        if (!leaf) {
            left->dump();
            right->dump();
        }
    }
    double predict(const std::vector<double> *inputs) {
        if (leaf) {
            return val;
        }
        else if (inputs->at(idx) < val) {
            return left->predict(inputs);
        }
        else {
            return right->predict(inputs);
        }
    }
};

class Classifier {
private:
    const static int MAX_CLASSES = 30;
    int num_classes;
    Tree *trees[MAX_CLASSES];
    int val[MAX_CLASSES];
public:
    Classifier(FILE *fp, int num, int *outputs) {
        assert(num <= MAX_CLASSES);
        num_classes = num;
        memcpy(val, outputs, sizeof(int)*num);
        char type;
        int idx;
        for (int i = 0; get_next_line(fp, &type, &idx, NULL) && i < num_classes; i++) {
            assert(type == 'a' && idx == i);
            trees[i] = new Tree(fp);
        }
    }
    void dump() {
        for (int i = 0; i < num_classes; i++) {
            std::cout << "Tree " << i << ": " << std::endl;
            trees[i]->dump();
        }
    }
    int predict(const std::vector<double> *inputs) {
        double outputs[num_classes];
        for (int i = 0; i < num_classes; i++) {
            outputs[i] = trees[i]->predict(inputs);
        }
        return val[std::distance(outputs, std::max_element(outputs, outputs + num_classes))];
    }
};


#endif
