#include "Visor.hpp"
#include "SpaceNode.hpp"

namespace NEngine {
void Visor::render() {
	RenderQuery * query = this->object->nspace->queryRender(this, this->context, false);
	// send to render every object
	this->renderer->render(query);
};
}
