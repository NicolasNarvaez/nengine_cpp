#include "SpaceTree.hpp"
#include "SpaceNode.hpp"

std::vector<NObject*>* SpaceTree::allObjects() {
	return this->root->allObjects();
}
