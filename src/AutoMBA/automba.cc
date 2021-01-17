#include "automba.h"

void
AutoMBA::print_si_accumulators()
{
    if (!PRINT_ACCUMULATORS)
        return;
    for (int i = 0; i <= NUM_CPUS; i++) {
        // std::cout << i << std::endl;
        // print sampling interval accumulators
        std::cout << "si_accumulators[" << i << "]: ";
        int num_acc = (i == NUM_CPUS) ? (int)ACC_ALL_MAX : (int)ACC_SI_MAX;
        for (int j = 0; j < num_acc; j++) {
            if (j == num_acc - 1) {
                std::cout << acc[i][j] << std::endl;
            }
            else {
                std::cout << acc[i][j] << " ";
            }
        }
    }
}

void
AutoMBA::print_ui_accumulators()
{
    if (!PRINT_ACCUMULATORS)
        return;
    for (int i = 0; i < NUM_CPUS; i++) {
        // print updating interval accumulators
        std::cout << "ui_accumulators[" << i << "]: ";
        for (int j = ACC_SI_MAX+1; j < ACC_UI_MAX; j++) {
            if (j == ACC_UI_MAX - 1) {
                std::cout << acc[i][j] << std::endl;
            }
            else {
                std::cout << acc[i][j] << " ";
            }
        }
    }
}

void
AutoMBA::reset_si_accumulators()
{
    for (int i = 0; i < NUM_CPUS; i++) {
        // reset sampling interval accumulators
        acc[i][ACC_SI_READ_T] = 0;
        acc[i][ACC_SI_WRITE_T] = 0;
        acc[i][ACC_SI_LATENCY] = 0;
        acc[i][ACC_SI_LATENCY_MODEL] = 0;
        acc[i][ACC_SI_LATENCY_SHDW] = 0;
        acc[i][ACC_SI_SAMPLING_NUM] = 0;
        acc[i][ACC_SI_NMC_COUNT] = 0;

    }
    acc[NUM_CPUS][ACC_ALL_READ_T] = 0;
    acc[NUM_CPUS][ACC_ALL_WRITE_T] = 0;
}

void
AutoMBA::reset_ui_accumulators()
{
    for (int i = 0; i < NUM_CPUS; i++) {
        // reset updating interval accumulators
        acc[i][ACC_UI_READ_T] = 0;
        acc[i][ACC_UI_WRITE_T] = 0;
    }
}

void AutoMBA::print_tb_parameters()
{
    if (!PRINT_TB_PARAMETERS)
        return;
    for (int i = 0; i < NUM_CPUS; i++) {
        // print token buckets' parameters
        std::cout << "token_bucket[" << i << "] parameters: ";
        std::cout << buckets[i]->get_size() << " " << buckets[i]->get_freq() << " "
                  << buckets[i]->get_inc() << " " << buckets[i]->get_bypass() << " "
                  << buckets[i]->get_tokens() << std::endl;
    }
}


void
AutoMBA::operate_slowdown_pred()
{

}


void
AutoMBA::operate_token_buckets()
{

}



// void
// AutoMBA::operate()
// {

// }