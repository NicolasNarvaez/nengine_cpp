#include "Renderer.hpp"
#include "SpaceNode.hpp"

namespace NEngine {

LLRenderer * createLLRenderer(LLRENDERER_VENDOR vendor) {

	LLRenderer * return_render;

	switch(vendor) {
		case LLRENDERER_VENDOR_VULKAN:
			return_render = (LLRenderer *) (new VulkanLLRenderer());
		break;
		case LLRENDERER_VENDOR_WEBGL:
		break;
	}

	return return_render;
}

VulkanLLRenderer::VulkanLLRenderer() {
	this->vendor = LLRENDERER_VENDOR_VULKAN;
}
VulkanLLRenderer::~VulkanLLRenderer() { 
	if(this->context != nullptr) delete this->context;
}

/**
 * LLScene -> LLRenderer dependencies:
 * 		context: 	phisical, logical device properties
 * 					window surface, swapchain props
 */
void VulkanLLRenderer::setSceneGraph(LLSceneGraph * scene) {
	this->scene = scene;
	this->context = new VulkanContext();
	this->createGraphicsPipeline();
}

void VulkanLLRenderer::checkSceneGraph() {
}

/**
 * Creates scene graphics pipelines
 */
void VulkanLLRenderer::createGraphicsPipeline() { 
}

void VulkanLLRenderer::render(RenderQuery * query) { }

void Renderer::constructor(LLRenderer * ll_renderer_) {
	this->ll_renderer = ll_renderer_;
}
void Renderer::render(RenderQuery * query) {
	// for( object in NObject)
		// compute object MVP
		// render object
	this->ll_renderer->render(query);
}

}
