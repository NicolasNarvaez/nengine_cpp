#include "resources.hpp"

#include <iostream>

namespace NEngine {



namespace FormatComponent {
	using enum NEngine::FormatComponentTypes;

	size_t getBitSize(const std::vector<FormatComponentTypes> & components) {
		size_t size = 0;
		for(auto component : components) {
			switch (component) {
			case UINT8:
			case SINT8:
				size += 8;
				break;
			case UINT16:
			case SINT16:
				size += 16;
				break;
			case UINT32:
			case SINT32:
				size += 32;
				break;
			default:
				std::runtime_error("Cant retrieve component size");
			}
		}
		return size;
	}

	const std::vector<FormatDescription> Descriptions = {
		{} // UNDEFINED
		, {
		      .format_id = FormatId::R8G8B8A8_SRGB
		    , .component_types = {UINT8, UINT8, UINT8, UINT8}
		}
		, {
		  .format_id = FormatId::R32G32_SFLOAT
		, .component_types = {UINT32, UINT32}
		}
		, {
		  .format_id = FormatId::R32G32B32_SFLOAT
		, .component_types = {UINT32, UINT32, UINT32}
		}
	};
}
const Format::Vector Formats = Format::Vector::parseFormatDescriptions(FormatComponent::Descriptions);

VkFormat toVkFormat(Format format) {
	return (VkFormat)format.format_id;
}

VkExtent2D toVkExtent(const glm::ivec2 & nengine_extent) {
	return {
		  .width = static_cast<uint32_t>(nengine_extent.x)
		, .height = static_cast<uint32_t>(nengine_extent.y)
	};
}

glm::ivec2 fromVkExtent(const VkExtent2D & extent) {
	return { extent.width, extent.height };
}
}
