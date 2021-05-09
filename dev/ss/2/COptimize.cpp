#include "COptimize.h"
#include <iostream>
#include <algorithm>

template <typename T, typename A>
int arg_max(std::vector<T, A> const& vec) {
    return static_cast<int>(std::distance(vec.begin(), max_element(vec.begin(), vec.end())));
}

COptimize::COptimize(CCircuit &Circuit, int gen_size) : Circuit(Circuit), gen_size(gen_size), num_units(Circuit.num_units), parent_fitness(gen_size), child_fitness(gen_size)
{
    // length of circuit vector
    N = 2*num_units+1;

    child_list.reserve(gen_size);
    for (int k = 0; k < gen_size; k++) 
        child_list.push_back(new int[N]);

    parent_list.reserve(gen_size);
    for (int k = 0; k < gen_size; k++) 
        parent_list.push_back(new int[N]);

    selected_pair.reserve(2);
    for (int k = 0; k < 2; k++)
        selected_pair.push_back(new int[N]);
}

COptimize::~COptimize()
{
    for (int k = 0; k < gen_size; k++) 
    {
        delete[] child_list[k];
        delete[] parent_list[k];
    }

    for (int j = 0; j < 2; j++)
        delete[] selected_pair[j];
}

void COptimize::initialize_parents()
{
    int conc_num, tails_num, feed_num;
    bool loop, valid;
    double fitness;

    // create temporary circuit vector
    int circuit_vector[N];

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
                loop = true;
                while (loop) // repeat until different product destinations and no self-recycle
                {
                    conc_num = rand() % (num_units + 2);
                    tails_num = rand() % (num_units + 2);

                    if (conc_num != i && tails_num != i && conc_num != tails_num)
                        loop = false;
                }

                circuit_vector[2 * i + 1] = conc_num;
                circuit_vector[2 * i + 2] = tails_num;
            }

            // check validity of vector & calculate fitness value if valid
            fitness = calculate_valid_fitness(circuit_vector, valid);
        }

        // store the valid parent vector
        parent_fitness[k] = fitness;
        std::copy(circuit_vector, circuit_vector + N, parent_list[k]);
    }
}

void COptimize::iterate(int Nit, double rtol)
{
    double performance;

    for (int n = 0; n < Nit; n++)
    {
        // best performing vector
        int max_ind = arg_max(parent_fitness);
        performance = parent_fitness[max_ind];
        for (int j = 0; j < CStream::num_components; j++)
            if (Circuit.conc_value[j] < 0.0)
                performance += Circuit.Feed[j] * Circuit.conc_value[j];
        
        // print best vector of the current generation
        std::cout << "Generation " << n << ": ";
        for (int i = 0; i < N; i++)
            std::cout << parent_list[max_ind][i] << ",";
        std::cout << " - Performance = " << performance << std::endl;
        
        // // scale parent fitnesses
        // offset = scale_fitness_values(fac);
        
        // update generation
        next_generation(0.8, 0.01);
    }
}

void COptimize::next_generation(double crossover_prob, double mutate_prob)
{
    bool valid;
    double fitness;
    int idx = 0;
    
    // First add the highest performing parent to the child list
    int max_ind = arg_max(parent_fitness);
    child_fitness[idx] = parent_fitness[max_ind];
    std::copy(parent_list[max_ind], parent_list[max_ind]+N, child_list[idx]);
    idx++;

    // now fill the rest of the child list by applying GA rules
    do {
        // Select a pair of valid parent vectors
        select_parent_pair();

        // Randomly decide if the parent vectors should crossover
        crossover_pair(crossover_prob);
        
        for (int j = 0; j < 2; j++)
        {
            // Break if child list was filled at the end of the j=0 iteration
            if (idx == gen_size)
                break;
            
            // Randomly apply mutations to the parent vector
            mutate_vector(mutate_prob, selected_pair[j]);
            
            // check validity of vector & calculate fitness value if valid
            fitness = calculate_valid_fitness(selected_pair[j], valid);

            if (valid)
            {
                child_fitness[idx] = fitness;
                std::copy(selected_pair[j], selected_pair[j]+N, child_list[idx]);
                idx++;
            }
        }

    // repeat from until the child list is filled with valid vectors
    }
    while (idx < gen_size);

    // replace the parent list with the child list
    parent_list.swap(child_list);
    parent_fitness.swap(child_fitness);
}

double COptimize::calculate_valid_fitness(int *vector, bool &valid)
{
    bool diverged;
    double fitness = 0.0;

    Circuit.Setup_From_Vector(vector);
    valid = Circuit.Check_Valid();

    if (valid)
    {
        fitness = Circuit.Run_Simulation(1.0e-6, 500, diverged);
        if (diverged)
        {
            valid = false;
            fitness = 0.0;
        }
        else
        {
            // shift performance by worst possible negative value so that valid circuits (i.e. diverged = false) have fitness value > 0
            // and invalid circuits (i.e. diverged = true) have a fitness value = 0 (and hence no chance of being selected)
            for (int j = 0; j < CStream::num_components; j++)
                if (Circuit.conc_value[j] < 0.0)
                    fitness -= Circuit.Feed[j] * Circuit.conc_value[j];
        }        
    }

    return fitness;
}

double COptimize::scale_fitness_values(double fac)
{
    /* Here we scale the raw fitness values of the parent population before proceeding to selection */
    double min_fitness = *min_element(parent_fitness.begin(), parent_fitness.end());

    for (int k = 0; k < gen_size; k++)
        parent_fitness[k] -= fac*min_fitness;

    return fac*min_fitness;
}

void COptimize::select_parent_pair()
{
    // calculate sum of fitness values
    double fitness_sum = 0;
    for (int k = 0; k < gen_size; k++)
        fitness_sum += parent_fitness[k];

    // select a pair of parents from the population
    for (int j = 0; j < 2; j++)
    {
        // generate a random number between 0 and fitness_sum
        double val1 = (double)((double)rand() / (double)RAND_MAX) * fitness_sum;
        double val2 = 0;

        // play roulette wheel to select a parent
        for (int k = 0; k < gen_size; k++)
        {
            val2 += parent_fitness[k];
            if (val2 > val1) // does this ensure that circuits w/ fitness = 0.0 can't be selected? Might floating point errors cause issues here?
            {
                std::copy(parent_list[k], parent_list[k]+N, selected_pair[j]);                
                break;
            }
        }
    }
}

void COptimize::crossover_pair(double crossover_prob)
{
    // generate random number to decide if crossing over or not
    double val = (double)((double)rand() / (double)RAND_MAX);

    // single point crossover rule
    if (val < crossover_prob)
    {
        int swap_ind = rand() % (N-1) + 1;
        std::swap_ranges(selected_pair[0], selected_pair[0] + swap_ind, selected_pair[1]);       
    }
}

/*
Mutation probability usually between 0.001 and 0.01
*/
void COptimize::mutate_vector(double mutate_prob, int *vector)
{
    for (int i = 0; i < N; i++)
    {
        double val = (double)((double)rand() / (double)RAND_MAX);

        if (val < mutate_prob)
        {          
            int mutate_dist = rand() % (N-1) + 1; // min mutate_dist is 1        
            int i_new = (i + mutate_dist) % N;    // therefore i_new will never equal i

            if (i < i_new)
                std::rotate(vector+i, vector+i+1, vector+i_new+1);
            else // if (i > i_new) // not necessary as i_new will never equal i
                std::rotate(vector+i_new, vector+i, vector+i+1);
        }
    }        
}