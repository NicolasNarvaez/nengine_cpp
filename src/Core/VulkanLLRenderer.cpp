#include "VulkanLLRenderer.hpp"
#include <iostream>

namespace NEngine {

VulkanLLRenderer::VulkanLLRenderer() {
	this->vendor = LLRENDERER_VENDOR_VULKAN;
}
VulkanLLRenderer::~VulkanLLRenderer() { 
	if(this->context != nullptr) delete this->context;
}

/**
 * LLScene -> LLRenderer dependencies:
 *		context:	physical, logical device properties
 *					window surface, swapchain props
 */
void VulkanLLRenderer::setSceneGraph(LLSceneGraph * scene) {
	this->scene = scene;
	this->context = new VulkanContext();
	this->createGraphicsPipeline();
	std::cout << "LOL" << std::endl;
}

// currently changes on scenegraph should trigger graphics pipeline recreation
// TODO:: validate graphics pipeline recreation subsets, cases
void VulkanLLRenderer::checkSceneGraph() {
}

/**
 * Creates scene graphics pipelines
 */
void VulkanLLRenderer::createGraphicsPipeline() { 
}

void VulkanLLRenderer::render(RenderQuery * query) { 

}

}
