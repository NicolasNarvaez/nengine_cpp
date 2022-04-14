#pragma once
#include "NObject.hpp"
#include "Scene.hpp"
#include "Graphics.hpp"

namespace NEngine {

class RenderQuery;

// manages/integrates scene graph and graphics pipeline specific to vendor
class LLRenderer {
	GraphicsContext * context;
	SceneGraph * scene;

	virtual void setup();
	virtual void render(RenderQuery * query = nullptr);
};

class VulkanLLRenderer : LLRenderer{
	void createContext();

	public:
	void setup();
	void setSceneGraph();
};


class Renderer {
	LLRenderer * ll_renderer;

	public:
	void render(RenderQuery * query);
};

}
