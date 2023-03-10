#pragma once

#ifdef NENGINE_LLRENDERER_VENDOR_VULKAN
#include "./vulkan/resources.hpp"
#endif

#include <array>
#include <vector>
#include <fstream>
#include <map>

namespace NEngine {

// const std::array<VkFormat, FormatId::MAX_VALUE> FormatsVulkanMap = {{
	// VK_FORMAT_UNDEFINED
	// ,
// }};

// constexpr std::array<FormatsInfo> getMappedEnum(std::array<FormatsInfo> formats_info)

// convert Format into const FormatInfo & 
// Format enum into 

// constexpr std::vector<FormatInfo> formatMapToVector(const std::Map<Format, FormatInfo> formats_info) { }

// static const std::map<Format, FormatInfo> Formats = {

// Manually casts src_data vector into dst_data based on component_types types and offsets.
// static void castData(
		// const std::vector<float> & src_data, const std::vector<FormatComponentTypes> & component_types
		// , const std::vector<uint32_t> & component_offsets
		// , void * dst_data
// ) {
//
	// uint32_t length = src_data.size();
	// uint32_t offset = 0;
//
	// uint32_t component = 0;
//
	// // while(component++ < length)
		// // switch component_types
		// // result[dim] = (uint8_t)(data[dim]*255);
// }

// static const FormatInfo getFormatInfo(Format format) {
// }

// returns textel byte size
// TODO: check if map to bite values
// static uint32_t formatToBytes(Format format) {
	// switch(format) {
	// case FORMAT_UNDEFINED:
		// throw std::runtime_error("Cant convert FORMAT_UNDEFINED to bytes");
	// case FORMAT_R8G8B8A8_SRGB:
		// return 4;
	// case FORMAT_R32G32_SFLOAT:
		// return 8;
	// case FORMAT_R32G32B32_SFLOAT:
		// return 12;
	// }
	// throw std::runtime_error("Cant map format to Bytes");
// }


/******************************************************************************
************************ Data containers, Buffers 
******************************************************************************/

/////////////////////////////////////////////////// Vertex Data Containers

// Minimal vertex component info
struct VertexDataAttribute {
	const Format & format;
	uint32_t offset;
};

/**
 * List of base Vertex types to use
 * {type}VertexType[_{dimensions = 3}D]
 * Trade = general usage
 */
struct TradeVertexType_3D {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 tex_coord;
	static const uint32_t components = 3;
	// TODO: simplify into "static const non-function member" 
	// (get sizeof, offsetof of incomplete type ?)
	static std::vector<VertexDataAttribute> getAttributes() {
		static const std::vector<VertexDataAttribute> attributes = {{
			{
				  .format = Formats[FormatId::R32G32B32_SFLOAT]
				, .offset = offsetof(TradeVertexType_3D, pos)
			}, {
				  .format = Formats[FormatId::R32G32B32_SFLOAT]
				, .offset = offsetof(TradeVertexType_3D, color)
			}, {
				  .format = Formats[FormatId::R32G32_SFLOAT]
				, .offset = offsetof(TradeVertexType_3D, tex_coord)
			}
		}};
		return attributes;
	};
};

typedef TradeVertexType_3D TradeVertexType;

struct IndexVertexType {
	uint16_t index;
	static const uint32_t components = 1;
	static std::vector<VertexDataAttribute> getAttributes() {
		return {};
	}
	IndexVertexType(uint16_t value) : index{value} {};
};

struct VertexDataDescription {
	uint32_t stride;
	uint32_t components;
	std::vector<VertexDataAttribute> attributes;
	// virtual const std::vector<VkVertexInputAttributeDescription>
		// getAttributeDescriptions() = 0;

	VertexDataDescription(uint32_t stride, uint32_t components, std::vector<VertexDataAttribute> attributes) 
		: stride(stride), components(components), attributes(attributes) {};
};

// TODO: Explore virtual vector wrapper for type agnostic vector fns (sizeBytes, etc)
// Vertex Data Container, data descriptions extractor. (includes buffer logic?)
template<typename VertexType>
struct VertexData : public std::vector<VertexType>, public VertexDataDescription {
	VertexData(std::vector<VertexType> verts = {}) 
		: std::vector<VertexType>(verts) 
		, VertexDataDescription(
				sizeof(VertexType), VertexType::components, VertexType::getAttributes())
	{};
	uint32_t sizeBytes() const {
		return this->stride * this->size();
	}
};

// We still dont know if its valuable to separate vertex & index
// template<typename IndexType>
// struct VertexIndexData : public std::vector<IndexType> {
	// uint32_t stride = 0;
//
	// VertexIndexData(std::vector<IndexType> indices = {})
		// : std::vector<IndexType>(indices), stride(sizeof(IndexType)) {};
//
	// uint32_t sizeBytes() const {
		// return this->stride * this->size();
	// }
// };

typedef VertexData<TradeVertexType> TradeVertexData;
typedef VertexData<IndexVertexType> IndexVertexData;

////////////// Buffer container
/// TODO: BufferData doesnt hold templated data (std::vector, etc), convert to struct with template constructor
struct BufferDataDescription {
	uint32_t size_bytes;
	BufferDataDescription(uint32_t size_bytes) : size_bytes{size_bytes} {};
};

template<typename BufferType>
struct BufferData : public BufferType, public BufferDataDescription {
	BufferData() : BufferDataDescription(sizeof(BufferType)) {};
};

}
