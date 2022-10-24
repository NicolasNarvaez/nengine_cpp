#include "BaseFrames.hpp"

namespace NEngine {

void VisorFrame::step(int milliseconds) {
	this->renderAll();
}

}
