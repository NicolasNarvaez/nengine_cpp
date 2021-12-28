#pragma once

#include "NObject.hpp"
#include "Renderer.hpp"

class Visor {
	NObject * nobject;
	NCamera * ncamera;
	void * visorcontext;
	Renderer * renderer;
	void render();
};
