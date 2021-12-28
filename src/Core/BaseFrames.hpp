#pragma once
#include <vector>
#include "Visor.hpp"
#include "SimulationFrame.hpp"

class VisorFrame {
	std::vector<Visor*> visors;
	void renderAll() {
		/// call render on every visor
	}

	static void step(VisorFrame * visor_frame, SimulationFrame * frame);
};
