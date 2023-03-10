#include "LLRenderer.hpp"
#include <iostream>
#ifdef NENGINE_LLRENDERER_VENDOR_VULKAN
#include "VulkanLLRenderer.hpp"
#endif
#ifdef NENGINE_LLRENDERER_VENDOR_WEBGL
#include "WebGLLLRenderer.hpp"
#endif

#include <stdexcept>

namespace NEngine {

LLRenderer * createLLRenderer(LLRENDERER_VENDOR vendor) {

	LLRenderer * return_render;

	switch(vendor) {
		#ifdef NENGINE_LLRENDERER_VENDOR_VULKAN
		case LLRENDERER_VENDOR_VULKAN:
			return_render = (LLRenderer *) (new VulkanLLRenderer());
		break;
		#endif
		#ifdef NENGINE_LLRENDERER_VENDOR_WEBGL
		case LLRENDERER_VENDOR_WEBGL:
		break;
		#endif
		default :
			std::cout << "No provider found" << std::endl;
			throw new std::runtime_error("No LLRenderer provider found.");
		break;
	}

	return return_render;
}

}
