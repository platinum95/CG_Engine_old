#pragma once

#include "CG_Data.h"
#include "Renderer.h"
#include "File_IO.h"
#include "Shader.h"
namespace GL_Engine {
	class Cubemap{
	public:
		Cubemap(const std::vector<std::string> &_TextureFiles, Shader *_CubemapShader, Renderer *_Renderer) {
			GenerateCubemap(_TextureFiles);
			SetupArrayObjects();
			SetupRenderPass(_Renderer);
			CreateShader(_CubemapShader);
		}
		~Cubemap() {

		}

		void GenerateCubemap(const std::vector<std::string> &_TextureFiles) {
			MapTexture = std::make_shared<CG_Data::Texture>(GL_TEXTURE0, GL_TEXTURE_CUBE_MAP);
			MapTexture->Bind();

			GLenum type = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			for (auto str : _TextureFiles) {
				int width, height, nChannels;
				void *data = File_IO::LoadImageFile(str, width, height, nChannels, false);
				glTexImage2D( type++, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		const RenderPass *GetRenderPass() const { return this->CubeRenderPass; }

	private:
		void CreateShader(Shader *_Shader) {
			if (CubemapShader)
				return;
			CubemapShader = _Shader;
			CubeRenderPass->shader = this->CubemapShader;
		}
		void SetupRenderPass(Renderer *_Renderer) {
			this->CubeRenderPass = _Renderer->AddRenderPass(this->CubemapShader, std::function<void(RenderPass &, void*)>(Cubemap::CubemapRenderer), nullptr);
			CubeRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); });
			CubeRenderPass->BatchVao = CubemapVAO;
			CubeRenderPass->Textures.push_back(MapTexture);
		}
		void SetupArrayObjects() {
			CubemapVAO = std::make_shared<CG_Data::VAO>();
			CubemapVAO->BindVAO();
			std::unique_ptr<CG_Data::VBO> indexVBO = std::make_unique<CG_Data::VBO>((void*) &indices[0], 36 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
			CubemapVAO->AddVBO(std::move(indexVBO));
			std::unique_ptr<CG_Data::VBO> meshVBO = std::make_unique<CG_Data::VBO>((void*) &vertices[0], 24 * sizeof(float), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
			CubemapVAO->AddVBO(std::move(meshVBO));
		}

		std::shared_ptr<CG_Data::Texture> MapTexture;
		RenderPass* CubeRenderPass;
		static Shader *CubemapShader;
		std::shared_ptr<CG_Data::VAO> CubemapVAO;

		static const float vertices[24];
		static const unsigned int indices[36];

		static void CubemapRenderer(RenderPass&, void*);


	};

}

