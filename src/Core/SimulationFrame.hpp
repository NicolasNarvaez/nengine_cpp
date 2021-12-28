#pragma once

class SimulationFrame {
	char * name;
	float refresh_delta;
	void (*fn)(void * context, SimulationFrame * frame);
	void * context;

	// calls function with context as arg
	void step();
};

