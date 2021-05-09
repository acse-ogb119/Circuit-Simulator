#include "COptimize.h"
#include <iostream>

std::vector<int *> COptimize::initialize_parents(CCircuit &Circuit, const int num_units, const int gen_size)
{
    // length of circuit vector
    const int N = 2 * num_units + 1;
    int conc_num, tails_num, feed_num;
    bool flag, valid;

    // create temporary circuit vector
    int *circuit_vector = new int[N];
    // vector to store parent list
    std::vector<int *> parent_list;
    parent_list.reserve(gen_size);

    // populate each parent vector
    for (int k = 0; k < gen_size; k++)
    {
        valid = false;
        while (!valid) // repeat until a valid vector is generated
        {
            // randomly select the feed unit
            feed_num = rand() % num_units;
            circuit_vector[0] = feed_num;

            // randomly generate the circuit connectivity
            for (int i = 0; i < num_units; i++)
            {
                flag = false;
                while (!flag) // repeat until different product destinations and no self-recycle
                {
                    conc_num = rand() % (num_units + 2);
                    tails_num = rand() % (num_units + 2);

                    if (conc_num != i && tails_num != i && conc_num != tails_num)
                        flag = true;
                }

                circuit_vector[2 * i + 1] = conc_num;
                circuit_vector[2 * i + 2] = tails_num;
            }

            // check validity of vector
            Circuit.Setup_From_Vector(circuit_vector);
            valid = Circuit.Check_Valid();
        }

        // store the valid parent vector
        parent_list.push_back(new int[N]);
        std::copy(circuit_vector, circuit_vector + N, parent_list[k]);
    }

    delete[] circuit_vector;
    return parent_list;
}

std::vector<std::pair<double, bool>> COptimize::calculate_fitness(CCircuit &Circuit, std::vector<int *> &parent_list, const int num_units, const int gen_size)
{
    // vector to store fitness values
    std::vector<std::pair<double, bool>> fitness_vals(gen_size);
    bool diverged;

    // run circuit simulation for each parent vector and store fitness value
    for (int k = 0; k < gen_size; k++)
    {
        Circuit.Setup_From_Vector(parent_list[k]);
        double performance = Circuit.Run_Simulation(1.0e-6, 500, diverged);

        // shift performance by worst possible negative value so that valid circuits (i.e. diverged = false) have fitness value > 0
        // and invalid circuits (i.e. diverged = true) have a fitness value = 0 (and hence no chance of being selected)
        for (int j = 0; j < CStream::num_components; j++)
            if (Circuit.conc_value[j] < 0.0)
                performance -= Circuit.Feed[j] * Circuit.conc_value[j];

        fitness_vals[k].first = performance;
        fitness_vals[k].second = diverged;
    }

    return fitness_vals;
}

std::vector<int *> COptimize::select_parent_pair(std::vector<int *> &parent_list, std::vector<std::pair<double, bool>> &fitness_vals, const int num_units, const int gen_size)
{
    // calculate sum of fitness values
    double fitness_sum = 0;
    for (int k = 0; k < gen_size; k++)
        fitness_sum += fitness_vals[k].first;

    // select a pair of parents from the population
    std::vector<int *> selected_pair(2);
    for (int j = 0; j < 2; j++)
    {
        // generate a random number between 0 and fitness_sum
        double val1 = (double)((double)rand() / (double)RAND_MAX) * fitness_sum;
        double val2 = 0;

        // play roulette wheel to select a parent
        for (int k = 0; k < gen_size; k++)
        {
            val2 += fitness_vals[k].first;
            if (val2 > val1)
            {
                selected_pair[j] = parent_list[k];

                std::cout << "Selected vector: ";
                for (int i = 0; i < 2 * num_units + 1; i++)
                    std::cout << selected_pair[j][i] << " ";
                std::cout << " Circuit fitness: " << fitness_vals[k].first << std::endl;

                break;
            }
        }
    }

    return selected_pair;
}

void COptimize::crossover_pair(const int num_units, std::vector<int *> &selected_pair)
{
    // random binary value to decide if crossing over or nit
    bool crossover = rand() % 2;

    // do crossover
    if (crossover)
    {
        int swap_ind = rand() % (2 * num_units + 1) + 1;
        std::cout << "crossover index " << swap_ind << std::endl;
        std::swap_ranges(selected_pair[0], selected_pair[0] + swap_ind, selected_pair[1]);

        for (int j = 0; j < 2; j++)
        {
            std::cout << "New vector: ";
            for (int i = 0; i < 2 * num_units + 1; i++)
                std::cout << selected_pair[j][i] << " ";
            std::cout << std::endl;
        }
    }
}

void COptimize::mutate_vector(const int num_units, double mutate_prob, int *vector)
{
    for (int i = 0; i < 2 * num_units + 1; i++)
    {
        double val = (double)((double)rand() / (double)RAND_MAX);

        if (val < mutate_prob)
        {
            int mutate_dist = rand() % (2 * num_units + 1) + 1;
        }
    }
}
