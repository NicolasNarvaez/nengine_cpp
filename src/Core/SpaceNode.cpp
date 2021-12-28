#include "SpaceNode.hpp"


RenderQuery * SpaceNode::queryRender(Visor * visor, VisorContext * context, bool generate_context) {

	RenderQuery * query = new RenderQuery();
	// query->objects = this->tree->allObjects();
	query->camera = visor->camera;
	return query;
}
