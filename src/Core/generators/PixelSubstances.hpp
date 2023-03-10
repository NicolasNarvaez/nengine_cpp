#pragma once

#include "../graphics/resources.hpp"
#include "../math/Algebra.hpp"

#include <cstring>

namespace NEngine::Generators::PixelSubstances {

// Texture generators
// Tensor sub/super sampling & fns pipeline
namespace Textures {

	// Pixelated
	namespace Pixel {
		// wood
		// bricks
		// grass
		// ground
	}
}

	// static std::vector<uint8_t> texelNormalizedFloat(const std::vector<float> & texel, , void * data) {

	// converts normalized to uint8
	static std::vector<uint8_t> texelNormalizedFloat(const std::vector<float> & texel) {

		std::vector<uint8_t> result(texel.size());
		uint32_t dim = texel.size();
		while(dim--) result[dim] = (uint8_t)(texel[dim]*255);
		return result;
	}

	// shorthand: allocates memory, applies generator, applies texture image, returns
	// TODO: separate into Tensor::generate() and Image::generate()
	// static void * generateTexture(
	static void * createSubstance(
			  uint32_t dim, const Format & format, size_t size
			, size_t tiling(const size_t & size, const std::vector<size_t> & coords)
			, std::vector<float> generator(const uint32_t & dim, const size_t & size, const std::vector<size_t> & coords) 
			, size_t & size_bytes
	) {
		// if(format->components
		// switch(format) {
			// case FORMAT_R8G8B8A8_SRGB:
			// break;
			// default:
			// std::runtime_error("Cant create PixelSubstance on given format");
		// }
		void * data = Math::Tensor::malloc(dim, size, format, size_bytes);

		size_t current_el = 1;

		for(const auto & coords : Math::Extent(dim, size)) {
			// iterator -> (image def, layout_iterator_fn, generator(coords) ) -> generated image
			std::cout << "generating texel " << current_el << std::endl;
			size_t tiling_coords = tiling(size, coords);
			size_t texel_size_bytes = format.texel_size_bytes;
			size_t texel_coord = tiling_coords * (texel_size_bytes/8);

			std::vector<uint8_t> texel = texelNormalizedFloat(generator(format.components, size, coords));

			std::cout << "generated texel: size, values : " << texel_size_bytes ;
			for(uint8_t val : texel) std::cout << " , " << static_cast<int>(val) ;
			std::cout << std::endl;

			std::cout << "coord : [ ";
			for(size_t val : coords) std::cout << " , " << val;
			std::cout << std::endl;

			std::cout << "copying texel: size, tiling coords" << texel_size_bytes << ", " << tiling_coords << std::endl;
			std::memcpy( (uint8_t*)data + texel_coord
					, texel.data()
					, texel_size_bytes);
			current_el++;
		}

		return data;
	}

	static void nlerp() {
	}

	// provides 2 algorithms to sample a tensor for super and sub sampling depending on the relative 
	// surface of components/texels (setting both tensor side lengths to 1).
	static void nlerp(const Math::Tensor & src, Math::Tensor & dst) {
		
		float src_comp_size = src.extent.getComponentSize(1);
		float dst_comp_size = dst.extent.getComponentSize(1);

		bool ss = dst_comp_size < src_comp_size; // supersampling
		for(const auto & coords : dst.extent) {
		}
	}

} // PixelSubstances
