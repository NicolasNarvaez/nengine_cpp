#pragma once
#include "../graphics/resources.hpp"


#include <random>
#include <chrono>

#include <vector>
#include <cmath>
#include <iostream>



namespace NEngine::Math {

struct Extent {
	const uint32_t dim;
	const std::vector<size_t> size;

	public:

	Extent(uint32_t dim, size_t size) : dim{dim}, size{std::vector<size_t>(dim, size)} {};

	// Retrieves the volume of a single voxel given a side length or vector indicating the length of the extent 
	// on every dimension.
	float getComponentSize(const float & side)  const {
		return this->getComponentSize(std::vector<float>(dim, side));
	}
	float getComponentSize(const std::vector<float> & side)  const {

		float component_size = 1;
		uint32_t current_coord = dim;
		while(current_coord--)
			component_size *= side[current_coord]/size[current_coord];

		return component_size;
	}

	////////////////////////////////////////////// Iterating

	// retrieves position of last element
	std::vector<size_t> endPosition() const {

		std::vector<size_t> res(dim, 0);

		// size_t coord = dim;
		// size_t first_coord = dim-1;
		// while(coord--) {
			// res[coord] = size[coord] - 1;
		// }
		res[0] = size[0];
		return res;
	}

	// moves position "step" positions forward or backward
	void movePosition(std::vector<size_t> & position, int64_t move = 1) const {

			// multidim loop end: update & check
			size_t current_coord = dim-1;
			while(current_coord > 0 && position[current_coord] == size[current_coord]) {
				position[current_coord] = 0;
				current_coord--;
			}
			
			// if(current_coord == 0 && position[current_coord] == (size[current_coord] - 1)) // last el
				// std::runtime_error("Out of bounds, multidim iterator");
			// else
				position[current_coord]++;
	}

	// optimized unit forward move
	void advancePosition(std::vector<size_t> & position) const {

		size_t current_coord = dim;
		while(--current_coord > 0 && position[current_coord] == size[current_coord] - 1) {
			position[current_coord] = 0;
		}
		
		// if(current_coord == 0 && position[current_coord] == (size[current_coord] - 1))
			// std::runtime_error("Out of bounds, multidim iterator");
		// else
			position[current_coord]++;
	}

	// tiling describes mapping from spatial coordinates into memory: linear_coords = tiling(spatial_coords)
	// returned coords are in "normalized space", real buffer coords = texel_size_bytes*tilingCoords + component_offset
	static size_t tilingLinear(const size_t & size, const std::vector<size_t> & coords) {
		
		size_t sum = 0;
		uint32_t max_dim = coords.size();
		uint32_t current_dim = max_dim;

		while(current_dim > 0 ) {
			current_dim--;
			sum += coords[current_dim] * std::pow(size, (max_dim - 1) - current_dim);
		}

		return sum;
	};

	// tiling describes mapping from spatial coordinates into memory: linear_coords = tiling(spatial_coords)
	// returned coords are in "normalized space", real buffer coords = texel_size_bytes*tilingCoords + component_offset
	static size_t tilingLinear(const std::vector<size_t> & size, const std::vector<size_t> & coords) {
		
		size_t sum = 0;
		uint32_t max_dim = coords.size();
		uint32_t current_dim = max_dim;

		while(current_dim > 0 ) {
			current_dim--;
			sum += coords[current_dim] 
					* std::accumulate(size.begin(), size.begin() + (max_dim - 1) - current_dim
						, 1, std::multiplies<size_t>());
		}

		return sum;
	};

	struct Iterator {
		private:
			const Extent * container;
			std::vector<size_t> value;
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::vector<size_t>;
			using difference_type = std::vector<size_t>;
			using pointer = std::vector<size_t> *;
			using reference = std::vector<size_t> &;

			// no starting position
			Iterator(const Extent * container) 
				: container{container}, value{std::vector<size_t>(container->dim, 0)} { };

			// copies starting position
			Iterator(const Extent * container, const std::vector<size_t> & pos) 
				: container{container}, value{std::vector<size_t>(pos)} { };

			reference operator*() noexcept {
				return value;
			}

			bool operator != (const Iterator & b) const noexcept {
				uint32_t coord = container->dim;
				while(coord--) if(value[coord] != b.value[coord]) return true;
				return false;
			}
			
			Iterator& operator++() noexcept {
				container->advancePosition(value);
				return *this;
			}
	};
	
	Iterator begin() const {
		return Iterator(this);
	};
	Iterator end() const {
		return Iterator(this, this->endPosition());
	}

};

/**
 * Describes all multilinear, high-dimentional relevant interfaces: operations, memory tiling, datatypes/formats
 * , iterators.
 *
 * Some Image-only things could be separate.
 * Format is included as is useful in gpu tensors, this starts engulfing image.
 */
struct Tensor {
	typedef size_t TilingFn(const std::vector<size_t> & size, const std::vector<size_t> & coords);
	typedef std::vector<float> GeneratorFn(
			const uint32_t & dim, const std::vector<size_t> & size, const std::vector<size_t> & coords);

	const Extent extent;
	Format format;

	void * data;
	size_t size_bytes;

	// Wrapper constructor: has extent, format and wraps incoming data or starts empty).
	Tensor(const Extent & extent, Format format, void * data = nullptr, size_t size_bytes = 0) 
		: extent{extent}, format{format}, data{data}, size_bytes{size_bytes} {}

	// Creates using generator.
	Tensor(const Extent & extent, Format format, TilingFn tiling, GeneratorFn generator) 
		: extent{extent}, format{format} {
		Tensor::malloc(extent, format, data, size_bytes);
		generate(extent, format, tiling, generator, data, size_bytes);
	}

	// Uses generator to update values mapped in data memory by tiling, format and extent descriptor.
	void generate(TilingFn tiling, GeneratorFn generator) {
		generate(extent, format, tiling, generator, data, size_bytes);
	}
	static void generate(
			const Extent & extent, const Format & format, TilingFn tiling, GeneratorFn generator
			, void * &data, size_t &size_bytes
	) {
		for(const auto & coords : extent) {
			// iterator -> (image def, layout_iterator_fn, generator(coords) ) -> generated image
			size_t tiling_coords = tiling(extent.size, coords);
			size_t texel_size_bytes = format.texel_size_bytes;
			size_t texel_coord = tiling_coords * texel_size_bytes;

			uint8_t* texel_offset = (uint8_t*)data + texel_coord;
			std::vector<uint8_t> texel (texel_offset, texel_offset + texel_coord);
			// texelNormalizedFloat(generator(format, extent.size, coords), texel);

			// std::vector<uint8_t> texel = texelNormalizedFloat(generator(format.components, extent.size, coords));
			// std::memcpy( (uint8_t*)data + texel_coord
					// , texel.data()
					// , texel_size_bytes);
		}
	}

	//////////
	static void malloc(const Extent & extent, Format format, void * &data, size_t & size_bytes) {
		Tensor::malloc(extent.dim, extent.size, format, data, size_bytes);
	}
	// allocates memory for tensor of side "size" and format.
	static void * malloc(uint32_t dim, std::vector<size_t> size, const Format & format, void * & data, size_t & size_bytes) {
		size_t components = std::accumulate(size.begin(), size.end(), 1, std::multiplies<size_t>());
		size_bytes = components * format.texel_size_bytes;
		return std::malloc(size_bytes);
	}
	// allocates enough memory for a tensor of provided format : side**dim * component_size_bytes
	static void * malloc(uint32_t dim, size_t size, const Format & format, size_t & size_bytes) {
		size_bytes = std::pow(size, dim) * format.texel_size_bytes;
		return std::malloc(size_bytes);
	}

};

// RGBA_SFLOAT_NORM
static std::vector<float> randomNoise(
		const uint32_t & components, const size_t & size, const std::vector<size_t> & coords
) {
	std::vector<float> texel(components, 0);

	uint32_t component = components;
	while(component--) texel[component] = (float)std::rand()/(float)RAND_MAX;

	return texel;
}

}
