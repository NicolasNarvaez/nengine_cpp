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
	NENGINE_GRAPHICS_CONTEXT vendor;
	void * context_instance;
	GraphicsCanvas * canvas;

	virtual void setup();
	virtual void present();
};

class VulkanCanvas {

};
// 
class VulkanContext : public GraphicsContext {
	public: 
	void setup();
	void present();

	private:
	void createInstance();
	// void createLogicalDevice();
};

}
