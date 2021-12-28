#include "BaseFrames.hpp"

void VisorFrame::step(
		VisorFrame * visor_frame, SimulationFrame * frame) {
	visor_frame->renderAll();
}
