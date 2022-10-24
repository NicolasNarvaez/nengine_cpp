#pragma once
#include <vector>
#include "Visor.hpp"
#include "SimulationFrame.hpp"

// Frames are execution threads in the space tree processing
namespace NEngine {

class VisorFrame : SimulationFrame {
	public:
	std::vector<NEngine::Visor*> visors;
	void renderAll() {
		/// call render on every visor
		for(auto visor : visors) {
			visor->render();
		}
	}
	void step(int milliseconds);
};

}
