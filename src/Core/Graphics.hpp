// low level graphics components
#pragma once

namespace NEngine {

enum NENGINE_GRAPHICS_CONTEXT {NENGINE_GRAPHICS_CONTEXT_VULKAN
	, NENGINE_GRAPHICS_CONTEXT_WEBGL};
// manages window surface, swapchain with images/views

// window surface, swapchain with images and views
class GraphicsCanvas {
};

// Manages graphics contet instance and surface
class GraphicsContext {
	
	GraphicsCanvas * canvas;
	void * context_instance;

	NENGINE_GRAPHICS_CONTEXT type;

	void present();
};

class VulkanCanvas {

};
// 
class VulkanContext {
};



}
