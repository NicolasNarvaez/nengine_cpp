#include "Graphics.hpp"

namespace NEngine {


void VulkanContext::setup() {
	this->createInstance();
	this->createLogicalDevice(this->getPhysicalDevice);
	this->setupSwapchain();
}

void VulkanContext::present() {
}

}
