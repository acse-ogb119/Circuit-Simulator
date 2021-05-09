#include <iostream>
#include <vector>
#include <omp.h>
#include "COptimize.h"

int main()
{
    srand(time(NULL));

    int num_units = 5;
    int gen_size = 100;

    CCircuit Circuit;
    Circuit.Set_Cicuit_Parameters();
    Circuit.Setup_Units(num_units);

    COptimize model(Circuit, gen_size);
    model.initialize_parents();
    model.iterate(500, 0.0);

    //     bool diverged;
    //     double start = omp_get_wtime();
    // #pragma omp parallel for private(diverged) num_threads(2)
    //     for (int k = 0; k < gen_size; k++)
    //     {
    //         std::cout << "before|" << model.parent_fitness[k] << " ";
    //         Circuit.Setup_From_Vector(model.parent_list[k]);
    //         double performance = Circuit.Run_Simulation(1.0e-6, 500, diverged);
    //         model.parent_fitness[k] = performance + 50000.;
    //         std::cout << "after|" << model.parent_fitness[k] << std::endl;
    //     }
    //     double finish = omp_get_wtime();
    //     std::cout << "finished in " << finish-start << " s" << std::endl;

    return 0;
}