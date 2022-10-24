#pragma once
#include "NObject.hpp"
#include <vector>

namespace NEngine {
class RenderQuery {
	public:
	// just intersected objects by now
	std::vector<NObject*>* objects;
	NCamera * camera;
};
}
