#pragma once
#include "Graphics.hpp"
#include "Scene.hpp"
#include "NObject.hpp"
#include "RenderQuery.hpp"

namespace NEngine {

/**
 * ::TODO
 * LLRenderer input from HLRenderer (high level renderer). 
 * Describes resources, attachments, pipelines and shaders to use along 
 * with data arguments and usage state (active, paused).
 * When a scene entry dissapears, its data is removed from rendering.
 */
class LLSceneGraph {
	// attachments, depth_stencil config, swapchain settings, output surfaces,
	// resolution, etc
};

enum LLRENDERER_VENDOR {
	  LLRENDERER_VENDOR_VULKAN
	, LLRENDERER_VENDOR_WEBGL
};

/**
 * Has a low level rendering graph and converts it into a vendor specific 
 * GraphicsFrame, along updating and drawing.
 */
class LLRenderer {
	protected:
	LLRENDERER_VENDOR vendor;
	GraphicsContext * context;
	LLSceneGraph * scene;

	public:
	virtual void setSceneGraph();
	virtual void render(RenderQuery * query = nullptr);
};

LLRenderer * createLLRenderer(
		LLRENDERER_VENDOR vendor = LLRENDERER_VENDOR_VULKAN
);


}
