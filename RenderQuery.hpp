#include "NObject.hpp"

class RenderQuery {
	public:
	// just intersected objects by now
	std::vector<NObject*>* objects;
	NCamera * camera;
};
