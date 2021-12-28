#pragma once

#include "NObject.hpp"
#include "Renderer.hpp"

class Visor {
	public:
	NObject * object;
	NCamera * camera;
	void * context;
	Renderer * renderer;
	void render();
};
