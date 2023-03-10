#pragma once
#include "SimulationFrame.hpp"
#include "SpaceTree.hpp"
#include "LLRenderer.hpp"
#include "Renderer.hpp"

#include "graphics/RenderScene.hpp"
#include "graphics/resources.hpp"

#include "math/Algebra.hpp"
#include "generators/PixelSubstances.hpp"

#include <cmath>
#include <chrono>
#include <random>
#include <vector>

namespace NEngine {

// rendering, physics
struct SimulationEntity : Entity {
	EntitySpatialComponent * spatial_component;
	EntityRenderComponent * render_component;
	
	SimulationEntity(EntityRenderType * render_type, RenderScene * scene) {
		// render_component = new EntityRenderComponent(mesh, material, spatial_component);
		render_component = scene->addEntity(render_type, spatial_component);
		spatial_component = new EntitySpatialComponent();
		spatial_component->pos = {0.0, 0.0, 0,0};
	}
};

class Simulation {
	public:
	std::vector<SimulationFrame*> frames;
	std::vector<SpaceTree*> space_trees;

	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<EntityRenderType> render_types;
	std::vector<SimulationEntity> entities;

	IndexVertexData indices_v = {{
		  0, 1, 2, 2, 3, 0
		, 4, 5, 6, 6, 7, 4
	}};
	TradeVertexData vertices = {{
			  {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
			, {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
			, {{ 0.5f,	0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
			, {{-0.5f,	0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
			// // square 2
			, {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
			, {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
			, {{ 0.5f,	0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
			, {{-0.5f,	0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		}};

	Simulation() {
	}

	void start() {
		auto now = std::chrono::high_resolution_clock::now();
		auto ms_since_epoch = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
		std::srand(ms_since_epoch);

		RenderScene * scene = new RenderScene();

		std::vector<char> vert_shader_code;
		std::vector<char> frag_shader_code;
		std::string vert_shader_path;
		std::string frag_shader_path;
		scene->vert_shader_path = "shaders/vert.spv";
		scene->frag_shader_path = "shaders/frag.spv";
		// scene->vert_shader_code = fs::readFile(vert_shader_path);
		// scene->frag_shader_code = fs::readFile(frag_shader_path);
		// scene->
		// scene->vert_shader_path = "shaders/vert.spv";
		// scene->frag_shader_path = "shaders/frag.spv";

		meshes.push_back(Mesh(&vertices, &indices_v));
		// materials.push_back(Material(
					// FORMAT_R8G8B8A8_SRGB, "assets/textures/statue_960_720.jpg", "shaders/vert.spv", "shaders/frag.spv"
					// ));

		uint8_t a = 255;


		std::vector<glm::uint8_t> image_data {
			  255, 0, 0, 0
			, 0, 255, 0, 0
			, 0, 0, 255, 0
			, 0, 0, 0, 0

			, 0, 255, 0, 0
			, 255, 0, 0, 0
			, 0, 0, 255, 0
			, 0, 0, 0, 0

			, 0, 255, 0, 0
			, 255, 0, 0, 0
			, 0, 0, 255, 0
			, 0, 0, 0, 0

			, 0, 255, 0, 0
			, 255, 0, 0, 0
			, 0, 0, 255, 0
			, 0, 0, 0, 0
		};
		std::cout << "initiating resources"<< std::endl;

		size_t data_size;
		size_t image_side = 4;

		void * noise_data = Generators::PixelSubstances::createSubstance(2, Formats[FormatId::R8G8B8A8_SRGB], image_side
				, Math::Extent::tilingLinear, Math::randomNoise, data_size);
		Format format = Formats[FormatId::R8G8B8A8_SRGB];
		std::cout << "created noise data, size: "<< data_size << ", texel_size_bytes" << format.texel_size_bytes << std::endl;
		std::cout << "format.format_id" << static_cast<uint32_t>(format.format_id) << std::endl;

		// Image * statue_image = new Image(FORMAT_R8G8B8A8_SRGB, image_data.data(), image_data.size(), 4, 4);
		Image * generated_image = new Image(Formats[FormatId::R8G8B8A8_SRGB], noise_data, data_size, image_side, image_side);
		std::cout << "generated images"<< std::endl;

		materials.push_back(Material(generated_image, "shaders/vert.spv", "shaders/frag.spv"));
		
		render_types.push_back(EntityRenderType(&meshes[0], &materials[0]));

		int limit = 5;
		float radius = 4;
		for(int i = 0; i < limit ; i++)
			for(int j = 0; j < limit ; j++) {
				auto entity = SimulationEntity(&render_types[0], scene);
				entity.spatial_component->pos = {
				// entity.spatial_component->pos = {0.0,0.0,0.0, 0.0};
					(float)((radius/(limit-1))*i - radius/2)
					,(float)((radius/(limit-1))*j - radius/2)
					, 0.0, 0.0};
				entities.push_back(entity);
			}

		std::cout << "starting"<< std::endl;
		Renderer renderer = Renderer();
		renderer.run(scene);
	}
	void stop() {
	}
	void step() {

	}
	void run(int milliseconds = 0) {
		if(milliseconds) {
		}
		
		this->start();
	}
};

}
