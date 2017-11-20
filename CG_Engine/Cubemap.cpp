#include "Cubemap.h"

namespace GL_Engine {
	const float Cubemap::vertices[24] = {
		-1, 1, -1,		//0 T1
		-1, -1, -1,		//1 T1
		1, -1, -1,		//2 T2
		1, 1, -1,			//3 T1


		-1, 1, 1,			//4 T1
		-1, -1, 1,		//5 T1
		1, -1, 1,			//6 T2
		1, 1,  1			//7 T1

	};

	//GL_QUAD Clockwise
	const unsigned int Cubemap::indices[36] = {
		0, 1, 3, 1, 2, 3,	//F1 Front
		2, 3, 7, 2, 7, 6,	//F2 Right
		4, 5, 7, 5, 7, 6,	//F3 Back
		0, 1, 4, 1, 4, 5,	//F4 Left
		1, 2, 6, 1, 6, 5,	//F5 Bottom
		0, 7, 4, 0, 7, 3	//F6 Top
	};

	void Cubemap::CubemapRenderer(RenderPass& _Pass, void* _Data) {
		_Pass.shader->UseShader();
		_Pass.BatchVao->BindVAO();
		_Pass.Textures[0]->Bind();
		_Pass.DrawFunction();
	}
	Shader *Cubemap::CubemapShader;

}