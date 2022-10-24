#pragma once
#include "LLRenderer.hpp"
#include "NObject.hpp"

#ifdef NENGINE_LLRENDERER_VENDOR_VULKAN
#include "VulkanLLRenderer.hpp"
#endif
// TODO: webgpu, webgl, metal, directx
// #ifdef NENGINE_LLRENDERER_WEBGPU
// #include "WebGPULLRenderer.hpp"
// #endif

namespace NEngine {
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
 * or similar with shims to some other higher order (but still RIR)
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
