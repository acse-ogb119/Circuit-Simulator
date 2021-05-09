#include "CCircuit.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

void CCircuit::Setup_Units(int num)
{
	if (unit != nullptr)
		delete[] unit;

	num_units = num;
	unit = new CUnit[num];
}

void CCircuit::Clear_Units()
{
	for (int i = 0; i < num_units; i++)
		unit[i].clear();

	Conc.clear();
	Tails.clear();
}

void CCircuit::Clear_Marks()
{
	for (int i = 0; i < num_units; i++)
		unit[i].mark = false;
}

void CCircuit::copy_clear_feeds()
{
	for (int i = 0; i < num_units; i++)
	{
		unit[i].Feed_Old = unit[i].Feed;
		unit[i].Feed.clear();
	}

	Conc_Old = Conc;
	Conc.clear();
	Tails_Old = Tails;
	Tails.clear();
}

void CCircuit::Setup_From_Vector(int *vector)
{
	Clear_Units();

	feed_num = vector[0];

	for (int i = 0; i < num_units; i++)
	{
		unit[i].conc_num = vector[i * 2 + 1];
		unit[i].tails_num = vector[i * 2 + 2];

		if (unit[i].conc_num == num_units)
			unit[i].conc_num = -1;
		if (unit[i].conc_num == num_units + 1)
			unit[i].conc_num = -2;

		if (unit[i].tails_num == num_units)
			unit[i].tails_num = -1;
		if (unit[i].tails_num == num_units + 1)
			unit[i].tails_num = -2;
	}
}

void CCircuit::mark_forward(int cell)
{
	unit[cell].mark = true;
	if (unit[cell].conc_num >= 0 && !unit[unit[cell].conc_num].mark)
		mark_forward(unit[cell].conc_num);

	if (unit[cell].tails_num >= 0 && !unit[unit[cell].tails_num].mark)
		mark_forward(unit[cell].tails_num);
}

void CCircuit::set_initial_forward(int cell)
{
	unit[cell].mark = true;
	unit[cell].Calculate_Unit();
	if (unit[cell].conc_num >= 0 && !unit[unit[cell].conc_num].mark)
	{
		unit[unit[cell].conc_num].Feed = unit[cell].Conc;
		set_initial_forward(unit[cell].conc_num);
	}

	if (unit[cell].tails_num >= 0 && !unit[unit[cell].tails_num].mark)
	{
		unit[unit[cell].tails_num].Feed = unit[cell].Tails;
		set_initial_forward(unit[cell].tails_num);
	}
}

void CCircuit::set_initial_feed()
{
	for (int i = 0; i < num_units; i++)
	{
		unit[i].Feed = Feed;
		unit[i].Calculate_Unit();
	}
}

bool CCircuit::see_exit(int cell, int exit_num)
{
	bool test1, test2;
	unit[cell].mark = true;

	if (unit[cell].conc_num >= 0 && !unit[unit[cell].conc_num].mark)
		test1 = see_exit(unit[cell].conc_num, exit_num);
	else if (unit[cell].conc_num < 0)
		test1 = (unit[cell].conc_num == exit_num);
	else
		test1 = false;

	if (unit[cell].tails_num >= 0 && !unit[unit[cell].tails_num].mark)
		test2 = see_exit(unit[cell].tails_num, exit_num);
	else if (unit[cell].tails_num < 0)
		test2 = (unit[cell].tails_num == exit_num);
	else
		test2 = false;

	return test1 || test2;
}

bool CCircuit::Check_Valid()
{
	//Check for self-recycle
	for (int i = 0; i < num_units; i++)
		if (unit[i].conc_num == i || unit[i].tails_num == i)
			return false;

	//Check that all cells can see the feed
	Clear_Marks();
	mark_forward(feed_num);

	for (int i = 0; i < num_units; i++)
		if (!unit[i].mark)
			return false;

	//check that all cells can see an outlet
	for (int i = 0; i < num_units; i++)
	{
		Clear_Marks();
		if (!see_exit(i, -1))
			return false;
		Clear_Marks();
		if (!see_exit(i, -2))
			return false;
	}

	return true;
}

double CCircuit::Run_Simulation(double tot_rel, int max_it, bool &diverged)
{
	double max_rel_change;

	diverged = false;

	Clear_Units();

	// 1) Give an initial guess for the feed rate of both components to every cell in the circuit
	unit[feed_num].Feed = Feed; // start by setting the in flow for the feed unit
	set_initial_forward(feed_num);

	int cnt = 0;

	do
	{
		// 2) For each unit, use the current guess of the feed (input) flowrate of each component to
		//    calculate the output flowrate of each component via both the concentrate and the tailings streams
		for (int i = 0; i < num_units; i++)
			unit[i].Calculate_Unit();

		// 3) Store the current value of the feed of each component into each cell as “old” feed values
		//    and then set the current value of the feeds for each component to zero
		copy_clear_feeds();

		// 4) For the cell receiving the circuit feed, set the feed of each component equal to the flowrate
		//    of the circuit feed
		unit[feed_num].Feed = Feed;

		for (int i = 0; i < num_units; i++)
		{
			// i = current unit
			// cell_forward = destination unit
			int cell_forward;

			// consider first the concentrate stream
			// Add the flowrates of the components in this stream to the flowrate of the relevant component in the feed going into the destination unit (or final concentrate stream)
			if ((cell_forward = unit[i].conc_num) >= 0)
				unit[cell_forward].Feed = unit[cell_forward].Feed + unit[i].Conc;
			else if (cell_forward == -1)
				Conc = Conc + unit[i].Conc;
			else
				Tails = Tails + unit[i].Conc;

			// Repeat this procedure for the tailings stream
			if ((cell_forward = unit[i].tails_num) >= 0)
				unit[cell_forward].Feed = unit[cell_forward].Feed + unit[i].Tails;
			else if (cell_forward == -1)
				Conc = Conc + unit[i].Tails;
			else
				Tails = Tails + unit[i].Tails;
		}

		// 7) For each component, check the difference between the newly calculated feed rate and the old feed rate for each cell.
		max_rel_change = 0.0;

		for (int i = 0; i < num_units; i++)
			for (int j = 0; j < CStream::num_components; j++)
			{
				max_rel_change = max(fabs((unit[i].Feed[j] - unit[i].Feed_Old[j]) / unit[i].Feed_Old[j]), max_rel_change);

				if (unit[i].Feed[j] > Feed[j] * 1000) //Divergence test
					cnt = max_it;
			}

		for (int j = 0; j < CStream::num_components; j++)
		{
			max_rel_change = max(fabs((Conc[j] - Conc_Old[j]) / Conc_Old[j]), max_rel_change);
			max_rel_change = max(fabs((Tails[j] - Tails_Old[j]) / Tails_Old[j]), max_rel_change);
		}

		cnt++;
		// If any of them have a relative change that is above a given threshold then repeat
		// (you should also leave this loop if a given number of iterations has been exceeded)
	} while (max_rel_change > tot_rel && cnt < max_it);

	// 8) Based on the flowrates of Gormanium and waste through the final concentrate stream calculate a performance value for the circuit
	double performance = 0.0;

	if (cnt >= max_it)
	{ // If there is no convergence you may wish to use the worst possible performance as the performance value
		diverged = true;
		//worst possible performance
		for (int j = 0; j < CStream::num_components; j++)
			if (conc_value[j] < 0.0)
				performance += Feed[j] * conc_value[j];
	}
	else
	{
		for (int j = 0; j < CStream::num_components; j++)
		{
			performance += Conc[j] * conc_value[j];
		}
	}
	return performance;
}

//Note that the circuit parameters are currently hard coded
void CCircuit::Set_Cicuit_Parameters()
{
	Feed[0] = 10.0;	 // total circuit feed of 10 kg/s of the valuable material
	Feed[1] = 100.0; // and 100 kg/s of the waste material

	conc_value[0] = 100.0;	// paid £100 per kg of valuable material in the final concentrate stream
	conc_value[1] = -500.0; // charged £500 per kg of the waste material in the same stream

	CUnit::Setup();
	CUnit::proportion_to_conc[0] = 0.2;	 // Each unit will recover 20% of the valuable material to the concentrate
	CUnit::proportion_to_conc[1] = 0.05; // together with 5% of the waste
}
