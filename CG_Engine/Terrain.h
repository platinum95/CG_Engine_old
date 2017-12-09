#pragma once
#include "Renderer.h"

namespace GL_Engine {

	class TerrainGenerator {
	public:
		static std::pair<std::vector<unsigned int>, std::vector<glm::vec2>> CreateMesh(uint32_t MeshSize, uint32_t Divisions) {
			std::vector<glm::vec2> Mesh;
			Mesh.reserve(Divisions * Divisions);
			//Create the mesh
			for (unsigned int z = 0; z < Divisions; z++) {
				int zIndex = z * Divisions;
				float zRatio = (float)z / (float)Divisions;
				float zPosValue = zRatio * MeshSize;
				for (unsigned int x = 0; x < Divisions; x++) {
					float xRatio = (float)x / (float)Divisions;
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
			return std::make_pair(Indices, Mesh);
		}

	};
	class TerrainChunk : public CG_Data::VAO {
	public:
		TerrainChunk(std::shared_ptr<CG_Data::VBO> MeshVBO, std::shared_ptr<CG_Data::VBO> IndexVBO, std::vector<float> Heights, glm::vec2 _GridPos, unsigned int MeshSize) {
			this->BindVAO();
			IndexVBO->BindVBO();
			MeshVBO->BindVBO();
			glEnableVertexAttribArray(0);	//Mesh always at index 0
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			auto HeightVBO = std::make_unique<CG_Data::VBO>(&Heights[0], Heights.size() * sizeof(float), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);	//Heights always at index 1
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
			this->AddVBO(std::move(HeightVBO));

			this->WorldGridPosition = _GridPos;
			this->WorldPos = _GridPos * (float)MeshSize;
			this->Translation = glm::translate(glm::mat4(1.0f), glm::vec3(this->WorldPos.x, 0, this->WorldPos.y));
		}
		glm::vec2 WorldPos;
		glm::vec2 WorldGridPosition;
		glm::mat4 Translation;
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
			this->MeshXZ = TerrainGenerator::CreateMesh(this->MeshSize, this->DivisionCount);
			auto meshVec = MeshXZ.second;
			auto indexVec = MeshXZ.first;
			MeshVBO = std::make_shared<CG_Data::VBO>(&meshVec[0], meshVec.size() * sizeof(glm::vec2), GL_STATIC_DRAW);
			IndexVBO = std::make_shared<CG_Data::VBO>(&indexVec[0], indexVec.size() * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
		}
		std::shared_ptr<TerrainChunk> GenerateChunk(int xGrid, int zGrid) {
			auto heightCount = this->DivisionCount * this->DivisionCount;
			std::vector<float> heights(heightCount, 0.0f);
	//		heights.reserve(this->DivisionCount * this->DivisionCount);
			auto newChunk = std::make_shared<TerrainChunk>(MeshVBO, IndexVBO, heights, glm::vec2(xGrid, zGrid), this->MeshSize);
			this->tPack.TerrainChunks.push_back(newChunk);
			return newChunk;
		}

		std::unique_ptr<RenderPass> GetRenderPass(Shader *_GroundShader) {
			auto renderPass = std::make_unique<RenderPass>();
			renderPass->Data = static_cast<TerrainPack*>(&tPack);
			renderPass->renderFunction = &TerrainRenderer;
			GLsizei nCount =(GLsizei) MeshXZ.first.size();
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
		std::pair<std::vector<unsigned int>, std::vector<glm::vec2>> MeshXZ;
		std::shared_ptr<CG_Data::VBO> MeshVBO, IndexVBO;
		uint32_t MeshSize, DivisionCount;
	private:
		static void TerrainRenderer(RenderPass &Pass, void* _Data);
	};

	

	class TerrainIO {
		std::shared_ptr<Terrain> LoadTerrain(std::string _Location);
		void SaveTerrain(std::string _Location);
	};

}

