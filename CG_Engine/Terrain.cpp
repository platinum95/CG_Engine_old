#include "Terrain.h"
namespace GL_Engine {



	void Terrain::TerrainRenderer(RenderPass &Pass, void* _Data) {
		auto chunks = static_cast<TerrainPack*>(_Data);

		Pass.shader->UseShader();
		for (auto tex : Pass.Textures) {
			tex->Bind();
		}
		for (auto chunk : chunks->TerrainChunks) {
			glUniformMatrix4fv(chunks->translationUniformLocation, 1, GL_FALSE, glm::value_ptr(chunk->Translation));
			chunk->BindVAO();
			Pass.DrawFunction();
		}
	}

}
