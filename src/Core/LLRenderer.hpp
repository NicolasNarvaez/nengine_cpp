#pragma once
#include "Graphics.hpp"
#include "Scene.hpp"
#include "NObject.hpp"
#include "RenderQuery.hpp"

#include "graphics/RenderScene.hpp"

namespace NEngine {

/**
 * ::TODO
 * LLRenderer input from HLRenderer (high level renderer). 
 * Describes resources, attachments, pipelines and shaders to use along 
 * with data arguments and usage state (active, paused), presentation info.
 * When a scene entry dissapears, its data is removed from rendering.
 */
class LLSceneGraph {
	// attachments, depth_stencil config, swapchain settings, output surfaces,
	// resolution, etc
};

enum LLRENDERER_VENDOR {
	  LLRENDERER_VENDOR_NULL
	, LLRENDERER_VENDOR_VULKAN
	, LLRENDERER_VENDOR_WEBGPU

	, LLRENDERER_VENDOR_WEBGL
	, LLRENDERER_VENDOR_DIRECTX
	, LLRENDERER_VENDOR_METAL
};

/**
 * Has a low level rendering graph and converts it into a vendor specific 
 * GraphicsFrame, along updating and drawing.
 */
class LLRenderer {
	protected:
	LLRENDERER_VENDOR vendor;
	// TODO: identiy LL scene construction abstractions/data-flows
	/* proposal:
	 *	-graphics context: device, queues
	 *	-rendering context: pipeline, command pools, resources
	 *	-output context: swapchain, [virtual] surface
	 */
	// GraphicsContext * context;
	// LLSceneGraph * scene;

	public:
	// virtual void setSceneGraph();
	// virtual void render(RenderQuery * query = nullptr);
	virtual void run(RenderScene * render_scene) = 0;
};

LLRenderer * createLLRenderer(
		LLRENDERER_VENDOR vendor 
		#ifdef NENGINE_LLRENDERER_VENDOR_VULKAN
			= LLRENDERER_VENDOR_VULKAN
		#elif NENGINE_LLRENDERER_VENDOR_WEBGL
			= LLRENDERER_VENDOR_WEBGL
		#else 
			= LLRENDERER_VENDOR_NULL
		#endif
);


}
