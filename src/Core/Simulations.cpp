#include "Simulations.hpp"
#include "Universes.hpp"

Simulation * generic() {
	Simulation * simulation = new Simulation();

	// create view frame
	// create physics frame
	// create logics frame (simulation logic, net, peripherals, etc)?
	simulation->frames = {};
	simulation->space_trees = {Universes::genesis()};
	return simulation;
}
