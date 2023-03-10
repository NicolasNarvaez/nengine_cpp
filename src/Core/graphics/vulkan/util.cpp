#include "util.hpp"

namespace NEngine {

/******************************************************************************
************************ Meta, Debug
******************************************************************************/

bool checkValidationLayerSupport(
		const std::vector<const char*> required_layers
) {
	if(required_layers.size() == 0) return true;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> available_layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, available_layers.data());

	for(const char* required_layer : required_layers) {
		bool found = false;

		for(const auto& availableLayer : available_layers)
				if(std::strcmp(required_layer, availableLayer.layerName))
						found = true;

		if(!found) return false;
	}

	return true;
}

VkResult createDebugUtilsMessengerEXT(
		  VkInstance instance
		, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
		, const VkAllocationCallbacks* pAllocator
		, VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) 
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}
void destroyDebugUtilsMessengerEXT(
		  VkInstance instance
		, VkDebugUtilsMessengerEXT debugMessenger
		, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if(func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL defaultDebugUtilsMessengerCallback(
	  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
	, VkDebugUtilsMessageTypeFlagsEXT messageType
	, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
	, void* pUserData
		) {

	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << "--------------------- ERROR ---------------------" 
			<< std::endl;
	}
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT & create_info) {
	create_info.sType = 
		  VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = 
		  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = defaultDebugUtilsMessengerCallback;
	create_info.pUserData = nullptr; // Optional
	create_info.pNext = nullptr;
	create_info.flags = 0;
}

VkShaderModule createShaderModule(
		VkDevice device, const std::vector<char>& code
) {

	VkShaderModuleCreateInfo create_info{
		  .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
		, .codeSize = code.size()
		, .pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	VkShaderModule shader_module;
	if(vkCreateShaderModule(device, &create_info, nullptr, &shader_module) 
			!= VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shader_module;
}

}
