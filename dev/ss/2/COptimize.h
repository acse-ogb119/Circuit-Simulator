#pragma once
#include <vector>
#include "CCircuit.h"

class COptimize
{
public:
    int N; // length of vector
    int num_units;
    int gen_size;

    CCircuit &Circuit;
    
    // TODO: use vector of vectors instead of vector of dynamically allocated arrays
    std::vector<int *> parent_list;
    std::vector<int *> child_list;
    std::vector<int *> selected_pair;

    // TODO: should not need to be vector, just a normal array (stack allocated)
    std::vector<double> parent_fitness;
    std::vector<double> child_fitness;

    // Constructor
    COptimize(CCircuit &Circuit, int gen_size);
    // Destructor
    ~COptimize();

    // generate initial population of valid parents
    void initialize_parents();

    // run optimization algorithm
    void iterate(int Nit, double rtol);

    // generate new children from current parents
    void next_generation(double crossover_prob, double mutate_prob);

    // calculate performance for each member of the population
    double calculate_valid_fitness(int *vector, bool &valid);
    // scale population fitnesses
    double scale_fitness_values(double fac);

    // selection rule
    void select_parent_pair();
    // crossover rule
    void crossover_pair(double crossover_prob);
    // mutation rule
    void mutate_vector(double mutate_prob, int *vector);
};