#pragma once
#include "Renderer.h"

namespace GL_Engine {
	class Terrain
	{
	public:
		Terrain();
		~Terrain();


	private:
		static void TerrainRenderer(RenderPass &Pass, void* _Data);
	};

	class TerrainGenerator {
		struct TerrainProperties {

		};
		std::shared_ptr<Terrain> GenerateTerrain(const TerrainProperties &_Properties);
	};

	class TerrainIO {
		std::shared_ptr<Terrain> LoadTerrain(std::string _Location);
		void SaveTerrain(std::string _Location);
	};

}

