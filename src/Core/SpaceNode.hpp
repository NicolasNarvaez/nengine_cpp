#include "NObject.hpp"
#include "SpaceTree.hpp"
#include "Visor.hpp"

class VisorContext {
};

class RenderQuery {
	// just intersected objects by now
};

class SpaceNode {
	SpaceTree * tree;
	NObject * objects;
	SpaceNode * parent;
	SpaceNode * sibblings;
	SpaceNode * childs;
	RenderQuery * queryRender(Visor * visor, VisorContext * context = 0, bool generate_context = false);
};

