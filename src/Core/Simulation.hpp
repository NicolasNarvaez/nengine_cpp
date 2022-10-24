#pragma once
#include <vector>
#include "SimulationFrame.hpp"
#include "SpaceTree.hpp"

namespace NEngine {

class Simulation {
	public:
	std::vector<SimulationFrame*> frames;
	std::vector<SpaceTree*> space_trees;

	Simulation() {
	}

	void start() {
	}
	void stop() {
	}
	void step() {

	}
	void run(int milliseconds = 0) {
		if(milliseconds) {
		}
		
		this->start();
	}

};

}
