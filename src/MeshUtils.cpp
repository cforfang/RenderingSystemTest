#include "MeshUtils.h"

namespace MeshUtils
{
std::array<float, 84> quadVertices = { {
	// Position          //Texcoords  // Normal         // Tangent        // Bitangent
	// Front-face
	-1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
	 1.0f,  1.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
	 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
	 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
	-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
	-1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 
}};
}