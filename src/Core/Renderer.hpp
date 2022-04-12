#pragma once
#include "NObject.hpp"
class RenderQuery;

class Renderer {
	public:
	void render(RenderQuery * query);
	void setupVulkan();
};
