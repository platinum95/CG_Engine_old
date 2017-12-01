#include "PostProcessing.h"
namespace GL_Engine {

	float PostProcessing::VertexPositions[]{
		-1, 1, -1.0,
		-1, -1, -1.0,
		1, -1, -1.0,
		1, 1, -1.0
	};

	float PostProcessing::TextureCoordinates[]{
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0
	};
	unsigned int PostProcessing::Indices[]{
		0, 1, 3,
		1, 2, 3
	};


}