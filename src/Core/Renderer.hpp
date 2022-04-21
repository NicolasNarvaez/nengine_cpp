#pragma once
#include "NObject.hpp"
#include "Scene.hpp"
#include "Graphics.hpp"
#include "Scene.hpp"

namespace NEngine {

class RenderQuery;

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

class VulkanLLRenderer : public LLRenderer{
	void createGraphicsContext();
	void createGraphicsPipeline();

	public:
	VulkanLLRenderer();
	~VulkanLLRenderer();
	void setSceneGraph(LLSceneGraph * scene);
	void checkSceneGraph();
	void render(RenderQuery * query = nullptr);
};


/**
 * Parses SceneGraph and holds high level rendering configs, algorithms.
 * 
 * Generates transient or persistent rendering intermedian representation fragments
 * for the low level renderer to translate into graphics rendering frame updates
 * or replacement (if the full rendering frame must be discarded).
 * This way the LLRenderer only acs as a projection of the lower/final rendering 
 * semantics into the desired vendor, and avoids implementing rendering 
 * algorithms at all.
 * The rendering intermedian representation (RIR) should be the highest level 
 * common rendering API (between vendors) and the lowest level rendering 
 * abstraction required to the engine. This could be the lowest level API 
 * with the biggest shared subset of low-level abstractions, thus a vulkan 
 * or similar with shims to some other higher order (but still below RIR)
 * algorithms.
 */
class Renderer {
	LLRenderer * ll_renderer;

	public:
	// sets ll_renderer to ll_renderer_
	void constructor (LLRenderer * ll_renderer_);
	void render(RenderQuery * query = nullptr);
};

}
