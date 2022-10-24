#include "LLRenderer.hpp"

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
		break;
	}

	return return_render;
}

}
