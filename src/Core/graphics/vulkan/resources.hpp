#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace NEngine {
	// layouts ???


//// ABSTRACT DESIGN DECISIONS
// src code implementation dependant features:::  computedevice implementation:
// multiple implementations of api => map of implementation
// possible concurrent usage of different implementations => allow default implementation vs namespaced versions 
//	&& implement multiple implementation tools: macro defaulter, organizing elements
//
// TODO: keep a map of effects, benefits of detaching "graphics implementation level" into 
// device/api to allow multiple apis

// We borrow the vulkan format for the inner format of NEngine for now, this simplifies vulkan compat. and removes the 
// task of keeping a decent enum, we can add more extensions/apis by planes, this is open for suggestions.
// This was based on the vulkan VkFormat as described in 
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFormat.html
// If this violates any class of ownership or license please contact us so we can change the implementation.

// defined at graphics implementation level
enum struct FormatId : uint32_t {
    UNDEFINED = VK_FORMAT_UNDEFINED
    // , FORMAT_R4G4_UNORM_PACK8 = 1
    // , FORMAT_R4G4B4A4_UNORM_PACK16 = 2
    // , FORMAT_B4G4R4A4_UNORM_PACK16 = 3
    // , FORMAT_R5G6B5_UNORM_PACK16 = 4
    // , FORMAT_B5G6R5_UNORM_PACK16 = 5
    // , FORMAT_R5G5B5A1_UNORM_PACK16 = 6
    // , FORMAT_B5G5R5A1_UNORM_PACK16 = 7
    // , FORMAT_A1R5G5B5_UNORM_PACK16 = 8
    // , FORMAT_R8_UNORM = 9
    // , FORMAT_R8_SNORM = 10
    // , FORMAT_R8_USCALED = 11
    // , FORMAT_R8_SSCALED = 12
    // , FORMAT_R8_UINT = 13
    // , FORMAT_R8_SINT = 14
    // , FORMAT_R8_SRGB = 15
    // , FORMAT_R8G8_UNORM = 16
    // , FORMAT_R8G8_SNORM = 17
    // , FORMAT_R8G8_USCALED = 18
    // , FORMAT_R8G8_SSCALED = 19
    // , FORMAT_R8G8_UINT = 20
    // , FORMAT_R8G8_SINT = 21
    // , FORMAT_R8G8_SRGB = 22
    // , FORMAT_R8G8B8_UNORM = 23
    // , FORMAT_R8G8B8_SNORM = 24
    // , FORMAT_R8G8B8_USCALED = 25
    // , FORMAT_R8G8B8_SSCALED = 26
    // , FORMAT_R8G8B8_UINT = 27
    // , FORMAT_R8G8B8_SINT = 28
    // , FORMAT_R8G8B8_SRGB = 29
    // , FORMAT_B8G8R8_UNORM = 30
    // , FORMAT_B8G8R8_SNORM = 31
    // , FORMAT_B8G8R8_USCALED = 32
    // , FORMAT_B8G8R8_SSCALED = 33
    // , FORMAT_B8G8R8_UINT = 34
    // , FORMAT_B8G8R8_SINT = 35
    // , FORMAT_B8G8R8_SRGB = 36
    // , FORMAT_R8G8B8A8_UNORM = 37
    // , FORMAT_R8G8B8A8_SNORM = 38
    // , FORMAT_R8G8B8A8_USCALED = 39
    // , FORMAT_R8G8B8A8_SSCALED = 40
    // , FORMAT_R8G8B8A8_UINT = 41
    // , FORMAT_R8G8B8A8_SINT = 42
    , R8G8B8A8_SRGB = VK_FORMAT_R8G8B8A8_SRGB
    // , FORMAT_B8G8R8A8_UNORM = 44
    // , FORMAT_B8G8R8A8_SNORM = 45
    // , FORMAT_B8G8R8A8_USCALED = 46
    // , FORMAT_B8G8R8A8_SSCALED = 47
    // , FORMAT_B8G8R8A8_UINT = 48
    // , FORMAT_B8G8R8A8_SINT = 49
    // , FORMAT_B8G8R8A8_SRGB = 50
    // , FORMAT_A8B8G8R8_UNORM_PACK32 = 51
    // , FORMAT_A8B8G8R8_SNORM_PACK32 = 52
    // , FORMAT_A8B8G8R8_USCALED_PACK32 = 53
    // , FORMAT_A8B8G8R8_SSCALED_PACK32 = 54
    // , FORMAT_A8B8G8R8_UINT_PACK32 = 55
    // , FORMAT_A8B8G8R8_SINT_PACK32 = 56
    // , FORMAT_A8B8G8R8_SRGB_PACK32 = 57
    // , FORMAT_A2R10G10B10_UNORM_PACK32 = 58
    // , FORMAT_A2R10G10B10_SNORM_PACK32 = 59
    // , FORMAT_A2R10G10B10_USCALED_PACK32 = 60
    // , FORMAT_A2R10G10B10_SSCALED_PACK32 = 61
    // , FORMAT_A2R10G10B10_UINT_PACK32 = 62
    // , FORMAT_A2R10G10B10_SINT_PACK32 = 63
    // , FORMAT_A2B10G10R10_UNORM_PACK32 = 64
    // , FORMAT_A2B10G10R10_SNORM_PACK32 = 65
    // , FORMAT_A2B10G10R10_USCALED_PACK32 = 66
    // , FORMAT_A2B10G10R10_SSCALED_PACK32 = 67
    // , FORMAT_A2B10G10R10_UINT_PACK32 = 68
    // , FORMAT_A2B10G10R10_SINT_PACK32 = 69
    // , FORMAT_R16_UNORM = 70
    // , FORMAT_R16_SNORM = 71
    // , FORMAT_R16_USCALED = 72
    // , FORMAT_R16_SSCALED = 73
    // , FORMAT_R16_UINT = 74
    // , FORMAT_R16_SINT = 75
    // , FORMAT_R16_SFLOAT = 76
    // , FORMAT_R16G16_UNORM = 77
    // , FORMAT_R16G16_SNORM = 78
    // , FORMAT_R16G16_USCALED = 79
    // , FORMAT_R16G16_SSCALED = 80
    // , FORMAT_R16G16_UINT = 81
    // , FORMAT_R16G16_SINT = 82
    // , FORMAT_R16G16_SFLOAT = 83
    // , FORMAT_R16G16B16_UNORM = 84
    // , FORMAT_R16G16B16_SNORM = 85
    // , FORMAT_R16G16B16_USCALED = 86
    // , FORMAT_R16G16B16_SSCALED = 87
    // , FORMAT_R16G16B16_UINT = 88
    // , FORMAT_R16G16B16_SINT = 89
    // , FORMAT_R16G16B16_SFLOAT = 90
    // , FORMAT_R16G16B16A16_UNORM = 91
    // , FORMAT_R16G16B16A16_SNORM = 92
    // , FORMAT_R16G16B16A16_USCALED = 93
    // , FORMAT_R16G16B16A16_SSCALED = 94
    // , FORMAT_R16G16B16A16_UINT = 95
    // , FORMAT_R16G16B16A16_SINT = 96
    // , FORMAT_R16G16B16A16_SFLOAT = 97
    // , FORMAT_R32_UINT = 98
    // , FORMAT_R32_SINT = 99
    // , FORMAT_R32_SFLOAT = 100
    // , FORMAT_R32G32_UINT = 101
    // , FORMAT_R32G32_SINT = 102
    , R32G32_SFLOAT = VK_FORMAT_R32G32_SFLOAT
    // , FORMAT_R32G32B32_UINT = 104
    // , FORMAT_R32G32B32_SINT = 105
    , R32G32B32_SFLOAT = VK_FORMAT_R32G32B32_SFLOAT
    // , FORMAT_R32G32B32A32_UINT = 107
    // , FORMAT_R32G32B32A32_SINT = 108
    // , FORMAT_R32G32B32A32_SFLOAT = 109
    // , FORMAT_R64_UINT = 110
    // , FORMAT_R64_SINT = 111
    // , FORMAT_R64_SFLOAT = 112
    // , FORMAT_R64G64_UINT = 113
    // , FORMAT_R64G64_SINT = 114
    // , FORMAT_R64G64_SFLOAT = 115
    // , FORMAT_R64G64B64_UINT = 116
    // , FORMAT_R64G64B64_SINT = 117
    // , FORMAT_R64G64B64_SFLOAT = 118
    // , FORMAT_R64G64B64A64_UINT = 119
    // , FORMAT_R64G64B64A64_SINT = 120
    // , FORMAT_R64G64B64A64_SFLOAT = 121
    // , FORMAT_B10G11R11_UFLOAT_PACK32 = 122
    // , FORMAT_E5B9G9R9_UFLOAT_PACK32 = 123
    // , FORMAT_D16_UNORM = 124
    // , FORMAT_X8_D24_UNORM_PACK32 = 125
    // , FORMAT_D32_SFLOAT = 126
    // , FORMAT_S8_UINT = 127
    // , FORMAT_D16_UNORM_S8_UINT = 128
    // , FORMAT_D24_UNORM_S8_UINT = 129
    // , FORMAT_D32_SFLOAT_S8_UINT = 130
    // , FORMAT_BC1_RGB_UNORM_BLOCK = 131
    // , FORMAT_BC1_RGB_SRGB_BLOCK = 132
    // , FORMAT_BC1_RGBA_UNORM_BLOCK = 133
    // , FORMAT_BC1_RGBA_SRGB_BLOCK = 134
    // , FORMAT_BC2_UNORM_BLOCK = 135
    // , FORMAT_BC2_SRGB_BLOCK = 136
    // , FORMAT_BC3_UNORM_BLOCK = 137
    // , FORMAT_BC3_SRGB_BLOCK = 138
    // , FORMAT_BC4_UNORM_BLOCK = 139
    // , FORMAT_BC4_SNORM_BLOCK = 140
    // , FORMAT_BC5_UNORM_BLOCK = 141
    // , FORMAT_BC5_SNORM_BLOCK = 142
    // , FORMAT_BC6H_UFLOAT_BLOCK = 143
    // , FORMAT_BC6H_SFLOAT_BLOCK = 144
    // , FORMAT_BC7_UNORM_BLOCK = 145
    // , FORMAT_BC7_SRGB_BLOCK = 146
    // , FORMAT_ETC2_R8G8B8_UNORM_BLOCK = 147
    // , FORMAT_ETC2_R8G8B8_SRGB_BLOCK = 148
    // , FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = 149
    // , FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = 150
    // , FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = 151
    // , FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = 152
    // , FORMAT_EAC_R11_UNORM_BLOCK = 153
    // , FORMAT_EAC_R11_SNORM_BLOCK = 154
    // , FORMAT_EAC_R11G11_UNORM_BLOCK = 155
    // , FORMAT_EAC_R11G11_SNORM_BLOCK = 156
    , MAX_VALUE
};

enum struct FormatComponentTypes : uint32_t {
	  UINT8, SINT8, UINT16, SINT16, UINT32, SINT32
	, FORMAT_COMPONENT_MAX_VALUE
};

// Low level graphics format description, maps to a vendor graphics format and describes data layout.
struct FormatDescription {
	FormatId format_id = FormatId::UNDEFINED;
	std::vector<FormatComponentTypes> component_types{};
};

// Implements most of FormatComponentType dependant logic.
// TODO: change format component type into struct. (rust allows variant enums)
namespace FormatComponent {
	// Definitions heavily dependant o FormatComponentTypes, this requires its own namespace 
	// (ideally should go in FormatDescription), using enum declaration is not compatible with struct.
	
	// TODO getU[nu]sableBinarySize()
	// Size in bits.
	size_t getBitSize(const std::vector<FormatComponentTypes> & components);

	// Base Format Descriptions
	extern const std::vector<FormatDescription> Descriptions;
}

struct Format {
	FormatId format_id = FormatId::UNDEFINED;
	uint32_t texel_size_bytes = 0;
	uint32_t components = 0;
	std::vector<uint32_t> component_offsets {};
	std::vector<FormatComponentTypes> component_types{};
	
	Format(const FormatDescription & description = {})
	: format_id{description.format_id}, component_types{description.component_types}
	, components{static_cast<uint32_t>(description.component_types.size())} 
	, texel_size_bytes{static_cast<uint32_t>(FormatComponent::getBitSize(description.component_types))} 
	{}

	// Extends std::vector adding uint32_t operator[] overload of FormatId
	class Vector : public std::vector<Format> {
	public:
		Format& operator[](FormatId index) {
			return std::vector<Format>::operator[](static_cast<uint32_t>(index));
		}

		const Format& operator[](FormatId index) const {
			return std::vector<Format>::operator[](static_cast<uint32_t>(index));
		}

		// Constructs a format array with elements with their corresponding FormatId integer value as index.
		// Invalid enums fallback to FormatId::UNDEFINED
		static const Vector parseFormatDescriptions(const std::vector<FormatDescription> descriptions) {
		    Vector formats;
		    formats.reserve(descriptions.size());

		    FormatId current_format_id = static_cast<FormatId>(0);
		    while(current_format_id < FormatId::MAX_VALUE) {

			// add format in descriptions if exists
			auto format_it = std::find_if(descriptions.begin(), descriptions.end()
					, [current_format_id](auto & description)
						{return description.format_id == current_format_id;}
				);

			formats.push_back(
				format_it != descriptions.end() ? Format(*format_it)
				: Format()
			);

			current_format_id = static_cast<FormatId>( static_cast<uint32_t>(current_format_id) + 1);
		    }

		    return formats;
		}
	};
};


// Elements are mapped following the enum definition, null values = FormatId::UNDEFINED 
// Null values not referentiable on interface as mapping requires a valid enum.
extern const Format::Vector Formats;

VkFormat toVkFormat(Format nengine_format);
VkExtent2D toVkExtent(const glm::ivec2 & nengine_extent);
glm::ivec2 fromVkExtent(const VkExtent2D & extent);

}
