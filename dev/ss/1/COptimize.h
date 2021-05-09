#pragma once
#include <vector>
#include "CCircuit.h"

class COptimize
{
public:
    // CCircuit Circuit;
    // int gen_size;
    // std::vector<int *> parent_list;
    // std::vector<int *> child_list;
    // std::vector<double> fitness_vals;

    // method to generate initial list of VALID parent vectors
    std::vector<int *> initialize_parents(CCircuit &Circuit, const int num_units, const int gen_size);

    // wrapper around CCircuit to calculate performance for every parent vector
    std::vector<std::pair<double, bool>> calculate_fitness(CCircuit &Circuit, std::vector<int *> &parent_list, const int num_units, const int gen_size);

    // selection rule
    std::vector<int *> select_parent_pair(std::vector<int *> &parent_list, std::vector<std::pair<double, bool>> &fitness_vals, const int num_units, const int gen_size);

    // crossover rule
    void crossover_pair(const int num_units, std::vector<int *> &selected_pair);

    // mutation rule
    void mutate_vector(const int num_units, double mutate_prob, int *vector);
};