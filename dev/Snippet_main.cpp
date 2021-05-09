#include "CCircuit.h"
#include <iostream>

int main()
{
	CCircuit Circuit;

	//Sets the performance and feed conditions for the circuit - currently hard coded
	Circuit.Set_Circuit_Parameters();

	//Circuit vector for the optimum circuit with 5 cells (circuit vector size is thus 11 - 1 item for the feed location and 2 for each of the 5 cells
	int circuit_vector[11] = {0, 4, 3, 2, 0, 5, 4, 4, 6, 2, 1};
	int num_units = 5;

	//Only needs to be done once
	Circuit.Setup_Units(num_units);

	//Creates the circuit based on the circuit vector
	Circuit.Setup_From_Vector(circuit_vector);

	//Note that the validity check only covers some ways in which the circuit might be invalid. If it diverges it is also likely to be invalid
	bool valid = Circuit.Check_Valid();
	bool diverged;

	//Note that the first two input paramters are the tolerance on convergence and the maximum iterations - may need to be changed for larger circuits
	double performance = Circuit.Run_Simulation(1.0e-6, 500, diverged);
	std::cout << "Circuit validity: " << valid << " Calculation diverged: " << diverged << " Calculated performance: " << performance << std::endl;

	return 0;
}