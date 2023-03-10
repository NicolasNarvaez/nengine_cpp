#pragma once

#include "resources.hpp"
#include "SceneResources.hpp"

#include <string>

namespace NEngine {

// struct Entity {
// }

// entity type:
// mesh -> graphics
// material -> graphics
// EntityInstance

///////////////////////// entity types, components & instances
///
/// entity can have multiple component

// struct EntityType {
// };

// Current ECS design:
// Entity is defined as an id, and component-handlers/systems operate on component data only. 
// It is allowed to extend entity to define entity types, and to hold data pointers, i read many had cache misses and 
// is common for components to reference other component data, a lot on specific cases (AI).
// The ideal is to use SIMD in component data without pointers. Data pointers live in the entity object, wich must be
// unique to the id. Components can have the entity id to retrieve other component data, or its pointer, but they 
// should not know entity layout. Component data should have pointers to required component data to avoid strong 
// dependency on entity object.
// Entity Object (the one caching component data uniquely referenced by id) may be removed, if all logic is handled
// in components with extensible interfaces that make its scope too broad to avoid bugs. 
// So only the id would be left (ideal).
struct Entity {
	uint64_t id;
};

struct Component {
};

struct EntitySpatialComponent : Component {
	glm::vec4 pos;
	glm::mat4 rot; // local coordinate
};

// system is the renderer ???
// current scene = system::visible = space intersection query?
// we will wait optimizations until we can measure changes & physics involved

// struct System {
// };

// TODO: layout rehuse, 
// struct DeviceEntityRenderType {
	// std::vector<VkDescriptorSet> descriptor_sets;
// };

struct EntityRenderType {
	Mesh * mesh = nullptr;
	Material * material = nullptr;

	EntityRenderType(Mesh * mesh, Material * material) : mesh{mesh}, material{material} {};
	
	void load(const ComputeDevice * device) {
		mesh->load(device); material->load(device);
	}
};

// struct EntityRenderComponentDeviceData {
	// std::vector<VkDescriptorSet> descriptor_sets;
	// VkDescriptorSetLayout descriptor_set_layout; // cached from RenderComponentType, on entity "type refresh", derived
												 // // entities MUST refresh
	// // RenderComponentType * component_type = nullptr;
// };

// includes cache for compiled common resource layout
struct EntityRenderComponent : Component {
	// Mesh * mesh = nullptr;
	// Material * material = nullptr;
	// rehuses compiled rendering layout, shareable between compatible EntityRenderComponent (same material 
	// "type", ...), 
	EntityRenderType * render_type = nullptr;
	const EntitySpatialComponent * spatial_component = nullptr;

	EntityRenderComponent(EntityRenderType * render_type, const EntitySpatialComponent * spatial_component) 
		: render_type{render_type}, spatial_component{spatial_component} {}
};

struct EntityRenderDescriptor {
	const Image * texture;
	VkDescriptorSet descriptor_sets = VK_NULL_HANDLE;
	uint32_t entity_count = 0;
};

// TODO change name to RendererScene (it became lower level, clearly this is not the spacegraph)
// Holds current scene rendering dependencies
// TODO: add space coherence cache -> rehuse space graph, add "layer" render for 
// visible query && simd (RenderScene could be a leaf layer?)
// TODO: everybody run now, the position cache becomes a frame dependant position cache!! -> should live in GPU then
//		1st -> take physics fields (processors) & layers (space subgraphs) into compute shaders
//		2nd -> limit access scope with special storage buffer for bidirectional objects -> callback agregations, events?
//				limited access allows implicit sync with frames (n frames => n reads from gpu)
//		NOTE: ensure non-physics compat. . Separate render cache/map? HostSpatial & DeviceSpatial?
struct RenderScene {
	// provided state
	// RenderingSurface rendering_surface;
	std::vector<EntityRenderComponent> entities;
	// descriptor sets map mat.resource and render pipeline binding 
	// => scope is limited to: render pipeline / object-scene
	
	// own state
	// evaluate propagation of granular client side changes (material, mesh) for future interfaces
	std::vector<EntityRenderDescriptor> descriptor_sets;
	
	EntityRenderComponent * addEntity(EntityRenderType * render_type, EntitySpatialComponent * spatial_component) {
		// entities.push_back(EntityRenderComponent(mesh, material, spatial_component));
		entities.push_back(EntityRenderComponent(render_type, spatial_component));
		const auto new_component = &entities.back();

		// find or create descriptor set, increase count
		// TOOD: move to addDescriptor(render_type->material->texture)
		EntityRenderDescriptor * found_descriptor = nullptr;
		for(EntityRenderDescriptor descriptor : descriptor_sets)
			if(descriptor.texture == render_type->material->texture) found_descriptor = &descriptor;

		if(found_descriptor == nullptr) {
			descriptor_sets.push_back({.texture = render_type->material->texture});
			found_descriptor = &descriptor_sets.back();
		}
		
		found_descriptor->entity_count++;

		return new_component;
	}

	// void addDescriptor()

	// shaders -> should me constructed from materials && scene data
	std::string vert_shader_path;
	std::string frag_shader_path;
	std::vector<char> vert_shader_code;
	std::vector<char> frag_shader_code;

	// World, camera state
	BufferDataDescription * uniform0_description;
	void * uniform0;
	DataBuffer * scene_buffer;

	// VkExtent2D viewport_extent;
	glm::vec2 viewport_extent;

	//////////////////////////////// object type
	// std::vector<SceneObject> objects
	// Scene objects, graphics_pipeline bindings (object materials)
	// Material * material;
	// descriptor_pool
	// descriptor_layout
	// images

	//////////////////////////////// individual objects
	// each vertex description must be in object mesh type
	// object meshes
	SceneObject * object;
	Mesh * mesh;
	// SceneBuffer *
	std::vector<VertexDataDescription*> vertices_descriptions;
	std::vector<void*> vertices_data;
	// descriptor_set

};

}
