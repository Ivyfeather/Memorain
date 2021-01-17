#include "slowdown_pred.h"

double SlowdownEstimator::estimate(std::vector<double> *inputs) {
    // TODO
    if (RUN_SLOWDOWN_PREDICTION)
        return clf->predict(inputs);
    else
        return 0.1;
}