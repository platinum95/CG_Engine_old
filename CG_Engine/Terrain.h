#pragma once
#include "Renderer.h"

namespace GL_Engine {

	class TerrainGenerator {
	public:
		static std::tuple<std::vector<unsigned int>, std::vector<glm::vec2>, std::vector<glm::vec2>> CreateMesh(uint32_t MeshSize, uint32_t Divisions) {
			std::vector<glm::vec2> Mesh;
			Mesh.reserve(Divisions * Divisions);
			//Create the mesh
			for (unsigned int z = 0; z < Divisions; z++) {
				int zIndex = z * Divisions;
				float zRatio = (float)z / (float)(Divisions-1);
				float zPosValue = zRatio * MeshSize;
				for (unsigned int x = 0; x < Divisions; x++) {
					float xRatio = (float)x / (float)(Divisions-1);
					float xPosValue = xRatio * MeshSize;
					Mesh.push_back(glm::vec2(xPosValue, zPosValue));
				}
			}
			std::vector<unsigned int> Indices;
			//Create the indices
			for (unsigned int z = 0; z < Divisions - 1; z++) {
				unsigned int zIndex = z * Divisions;
				for (unsigned int x = 0; x < Divisions - 1; x++) {
					unsigned int CurrIndex = zIndex + x;
					unsigned int CurrIndexBelow = CurrIndex + Divisions;
					//need to add 2 triangles here.
					Indices.push_back(CurrIndex);
					Indices.push_back(CurrIndex + 1);
					Indices.push_back(CurrIndexBelow + 1);
					Indices.push_back(CurrIndex);
					Indices.push_back(CurrIndexBelow);
					Indices.push_back(CurrIndexBelow + 1);
				}
			}
			std::vector<glm::vec2> texcoord;
			texcoord.reserve(Divisions * Divisions);
			//Create the mesh
			for (unsigned int z = 0; z < Divisions; z++) {
				int zIndex = z * Divisions;
				float zRatio = (float)z / (float)(Divisions - 1);
				for (unsigned int x = 0; x < Divisions; x++) {
					float xRatio = (float)x / (float)(Divisions - 1);
					texcoord.push_back(glm::vec2(xRatio, zRatio));
				}
			}
			return std::make_tuple(Indices, Mesh, texcoord);
		}

		static std::vector<float> GenerateHeights(int MeshSize, int DivisionCount, int GridX, int GridZ) {
			int startingX = GridX * DivisionCount;
			int startingZ = GridZ * DivisionCount;
			std::vector<float> heights;
			heights.reserve(DivisionCount * DivisionCount);
			for (unsigned int z = 0; z < DivisionCount; z++) {
				unsigned int zIndex = z * DivisionCount;
				for (unsigned int x = 0; x < DivisionCount; x++) {
					unsigned int CurrIndex = zIndex + x;
					heights.push_back(getHeight(startingX + x, startingZ + z));
				}
			}
			return heights;
		}
		static float getHeight(int x, int z) {
			float total = getInterpolatedNoise(x / 32.0f, z / 32.0f);
			total += getInterpolatedNoise(x / 16.0f, z / 16.0f) / 2.0f;
			total += getInterpolatedNoise(x / 8.0f, z / 8.0f) / 7.0f;

			return total;
		}
		static std::vector<std::vector<glm::vec3>> GetTBN(std::vector<float> Heights, std::vector<glm::vec2> UVs, int Divisions) {
			std::vector <glm::vec3> Normals, Tangents, Bitangents;
			std::vector<std::vector<glm::vec3>> TBN;
			Normals.resize(Heights.size(), glm::vec3(1, 1, 0));
			for (unsigned int z = 1; z < Divisions - 1; z++) {
				unsigned int zIndex = z * Divisions;
				for (unsigned int x = 1; x < Divisions - 1; x++) {
					unsigned int CurrIndex = zIndex + x;
					unsigned int CurrIndexBelow = CurrIndex + Divisions;
					unsigned int CurrIndexAbove = CurrIndex - Divisions;
					float hL = Heights[CurrIndex - 1];
					float hR = Heights[CurrIndex + 1];
					float hD = Heights[CurrIndexBelow];
					float hU = Heights[CurrIndexAbove];
					glm::vec3 N(hL - hR, 2.0, hD - hU);
					N = normalize(N);
					Normals[CurrIndex] = N;
				}
			}
			TBN.push_back(Normals);
			return TBN;
		}
		//static float getHeight(int x, int z, float mag1, float mag2, float mag3);

	private:
		static float getSmoothNoise(int x, int z) {
			float corners = (getNoise(x + 1, z + 1) + getNoise(x - 1, z + 1) + getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1)) / 16.0f;
			float sides = (getNoise(x + 1, z) + getNoise(x - 1, z) + getNoise(x, z - 1) + getNoise(x, z - 1)) / 8.0f;
			float centre = getNoise(x, z) / 4.0f;
			return corners + centre + sides;
		}
		static float getNoise(int x, int z) {
			int n = x * 45 + z * 57;
			n = (n << 13) ^ n;
			int nn = (n*(n*n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
			float toReturn = (float)(1.0 - ((double)nn / 1073741824.0));
			return toReturn * 40.0f;
		}
		static float interpolate(float a, float b, float blend) {
			float f = (1.0f - cos(blend * 3.145f)) * 0.5f;
			return a + (f)*(b - a);
		}
		static float getInterpolatedNoise(float x, float z) {
			int intX = (int)x;
			int intZ = (int)z;

			float fracX = x - intX;
			float fracZ = z - intZ;
			if (x <= 0) {
				fracX = 1 + fracX;
				intX--;
			}
			if (z <= 0) {
				fracZ = 1 + fracZ;
				intZ--;
			}
			float v1 = getSmoothNoise(intX, intZ);
			float v2 = getSmoothNoise(intX + 1, intZ);
			float v3 = getSmoothNoise(intX, intZ + 1);
			float v4 = getSmoothNoise(intX + 1, intZ + 1);
			float i1 = interpolate(v1, v2, fracX);
			float i2 = interpolate(v3, v4, fracX);
			return interpolate(i1, i2, fracZ);
		}

	};
	class TerrainChunk : public CG_Data::VAO {
	public:
		TerrainChunk(std::shared_ptr<CG_Data::VBO> MeshVBO, std::shared_ptr<CG_Data::VBO> IndexVBO, std::shared_ptr<CG_Data::VBO> TexVBO, std::vector<float> Heights, std::vector<glm::vec3> Normals, glm::vec2 _GridPos, unsigned int MeshSize) {
			this->BindVAO();
			IndexVBO->BindVBO();
			MeshVBO->BindVBO();
			glEnableVertexAttribArray(0);	//Mesh always at index 0
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			auto HeightVBO = std::make_unique<CG_Data::VBO>(&Heights[0], Heights.size() * sizeof(float), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);	//Heights always at index 1
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
			this->AddVBO(std::move(HeightVBO));
			TexVBO->BindVBO();
			glEnableVertexAttribArray(2);	//UV always at index 2
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			auto NormalVBO = std::make_unique<CG_Data::VBO>(&Normals[0], Normals.size() * sizeof(glm::vec3), GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);	//Normals always at index 3
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			this->AddVBO(std::move(NormalVBO));
			
			this->WorldGridPosition = _GridPos;
			this->WorldPos = _GridPos * ((float)MeshSize) ;
			this->Translation = glm::translate(glm::mat4(1.0f), glm::vec3(this->WorldPos.x, 0, this->WorldPos.y));
		}
		glm::vec2 WorldPos;
		glm::vec2 WorldGridPosition;
		glm::mat4 Translation;
	private:
		
	};

	class Terrain{
	private:
		struct TerrainPack {
			std::vector<std::shared_ptr<TerrainChunk>> TerrainChunks;
			unsigned int translationUniformLocation;
		};
	public:
		Terrain(uint32_t _MeshSize, uint32_t _DivisionCount){
			this->MeshSize = _MeshSize;
			this->DivisionCount = _DivisionCount;
			std::tie(Indices, Mesh, UVCoords)= TerrainGenerator::CreateMesh(this->MeshSize, this->DivisionCount);
			auto meshVec = Mesh;
			auto indexVec = Indices;
			glBindVertexArray(0);
			MeshVBO = std::make_shared<CG_Data::VBO>(&meshVec[0], meshVec.size() * sizeof(glm::vec2), GL_STATIC_DRAW);
			TexcoordVBO = std::make_shared<CG_Data::VBO>(&UVCoords[0], UVCoords.size() * sizeof(glm::vec2), GL_STATIC_DRAW);
			IndexVBO = std::make_shared<CG_Data::VBO>(&indexVec[0], indexVec.size() * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
		}
		std::shared_ptr<TerrainChunk> GenerateChunk(int xGrid, int zGrid) {
			auto heightCount = this->DivisionCount * this->DivisionCount;
			std::vector<float> heights = TerrainGenerator::GenerateHeights(this->MeshSize, this->DivisionCount, xGrid, zGrid);
			auto Normals = TerrainGenerator::GetTBN(heights, this->UVCoords, this->DivisionCount);
			auto newChunk = std::make_shared<TerrainChunk>(MeshVBO, IndexVBO, TexcoordVBO, heights, Normals[0], glm::vec2(xGrid, zGrid), this->MeshSize);
			this->tPack.TerrainChunks.push_back(newChunk);
			return newChunk;
		}

		std::unique_ptr<RenderPass> GetRenderPass(Shader *_GroundShader) {
			auto renderPass = std::make_unique<RenderPass>();
			renderPass->Data = static_cast<TerrainPack*>(&tPack);
			renderPass->renderFunction = &TerrainRenderer;
			GLsizei nCount =(GLsizei) Indices.size();
			auto drawFunct = [nCount]() {glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_INT, nullptr); };
			renderPass->SetDrawFunction(drawFunct);
			renderPass->shader = _GroundShader;
			tPack.translationUniformLocation = _GroundShader->GetUniform("GroundTranslation")->GetID();
			return std::move(renderPass);
		}
		~Terrain() {};
		void SetSeed() {

		}
		TerrainPack tPack;
		std::vector<unsigned int> Indices;
		std::vector<glm::vec2 > Mesh;
		std::vector<glm::vec2> UVCoords;
		std::shared_ptr<CG_Data::VBO> MeshVBO, IndexVBO, TexcoordVBO;
		uint32_t MeshSize, DivisionCount;
	private:
		static void TerrainRenderer(RenderPass &Pass, void* _Data);
	};

	

	class TerrainIO {
		std::shared_ptr<Terrain> LoadTerrain(std::string _Location);
		void SaveTerrain(std::string _Location);
	};

}

