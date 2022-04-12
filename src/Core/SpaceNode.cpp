#include <iostream>
#include <algorithm>
#include "SpaceNode.hpp"
#include "util.hpp"

using namespace nengine;

// Constructs rendering query from visor definition/spatial graphic features
RenderQuery * SpaceNode::queryRender(Visor * visor, VisorContext * context, bool generate_context) {
	RenderQuery * query = new RenderQuery();
	query->objects = this->tree->allObjects();
	query->camera = visor->camera;
	return query;
}

// Returns every object in subset coord of node
std::vector<NObject*> * SpaceNode::allObjects() {
	std::vector<std::vector<NObject*>*>* childs_objects;
	childs_objects->reserve(this->childs->size());

	for(SpaceNode * child_node : * this->childs)
		childs_objects->push_back(child_node->objects);
	// std::transform(this->childs->begin(), this->childs->end(), std::back_inserter(childs_objects), [](SpaceNode * child) {return child->objects;});

	return util::vector::merge<NObject*>(this->objects, childs_objects);
}
