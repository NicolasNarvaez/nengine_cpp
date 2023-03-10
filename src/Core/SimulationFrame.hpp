#pragma once

namespace NEngine {
class SimulationFrame {
	public: 
	char * name;
	float refresh_delta;
	virtual void step (int milliseconds) = 0;
};
}
