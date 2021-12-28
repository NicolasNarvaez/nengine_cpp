#include "SimulationFrame.hpp"

void SimulationFrame::step() {
	this->fn(this->context, this);
}
