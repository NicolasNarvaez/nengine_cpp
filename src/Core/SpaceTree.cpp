#include "SpaceTree.hpp"
#include "SpaceNode.hpp"

namespace NEngine {
std::vector<NObject*>* SpaceTree::allObjects() {
	return this->root->allObjects();
}
}
