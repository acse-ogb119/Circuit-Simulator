#include <iostream>
#include <vector>
#include "COptimize.h"

int main()
{
    srand(time(NULL));

    int num_units = 10;
    int gen_size = 100;

    CCircuit Circuit;
    Circuit.Set_Cicuit_Parameters();
    Circuit.Setup_Units(num_units);

    COptimize model;
    std::vector<int *> parent_list = model.initialize_parents(Circuit, num_units, gen_size);
    std::vector<std::pair<double, bool>> fitness_vals = model.calculate_fitness(Circuit, parent_list, num_units, gen_size);

    for (int k = 0; k < gen_size; k++)
    {
        Circuit.Setup_From_Vector(parent_list[k]);
        bool valid = Circuit.Check_Valid();

        std::cout << "Circuit vector: ";
        for (int i = 0; i < 2 * num_units + 1; i++)
            std::cout << parent_list[k][i] << " ";

        std::cout << " Circuit validity: " << valid << " Calculation diverged: " << fitness_vals[k].second << " Calculated performance: " << fitness_vals[k].first << std::endl;
    }
    std::cout << std::endl;

    std::vector<int *> selected_pair = model.select_parent_pair(parent_list, fitness_vals, num_units, gen_size);
    model.crossover_pair(num_units, selected_pair);

    for (int k = 0; k < gen_size; k++)
        delete[] parent_list[k];
}