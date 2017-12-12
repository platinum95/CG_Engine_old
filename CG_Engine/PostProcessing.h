#pragma once

#include "CG_Data.h"
#include "Shader.h"
#include <sstream>
#include <glm\vec2.hpp>
namespace GL_Engine {
	class PostProcessing{
	public:
		enum PostprocessingAttachment{
			GaussianBlur, SaturationAdjust, ContrastAdjust, BrightnessAdjust
		};
		PostProcessing() {
			AttachmentStringComponents[1] = "void main(){\n";
		}
		~PostProcessing() {

		}

		CG_Data::Uniform* AddAttachment(PostprocessingAttachment _Attachment){
			switch(_Attachment)
			{
			case GaussianBlur:
			{
				auto GaussianLambdaUpdater = [](const CG_Data::Uniform &u) {glUniform1fv(u.GetID(), 5, static_cast<const GLfloat*>(u.GetData())); };
				auto uni = shader.RegisterUniform("GaussianWeights", GaussianLambdaUpdater);
				AttachmentStringComponents[0] += "uniform float GaussianWeights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\n";
				AttachmentStringComponents[1] += "BloomEffect();\n";
				return uni;
			};
			case SaturationAdjust:
			{
					
			};


			}
			
		}

		std::shared_ptr<CG_Data::Texture> Compile(std::shared_ptr<CG_Data::Texture> _TextureInput, uint16_t _Width, uint16_t _Height){

			this->Resolution = glm::vec2(_Width, _Height);
			InputTexture = _TextureInput;
			//std::string FragmentShader;
			std::stringstream FragmentStream(FragmentShader);
			FragmentStream << "#version 330\n";
			FragmentStream << AttachmentStringComponents[0];
			FragmentStream << FragmentShader;
			FragmentStream << AttachmentStringComponents[1];
				
			FragmentStream << "}\n";
			FragmentShader = FragmentStream.str();
			shader.RegisterShaderStage(this->VertexShader.c_str(), GL_VERTEX_SHADER);
			shader.RegisterShaderStage(this->FragmentShader.c_str(), GL_FRAGMENT_SHADER);
			shader.RegisterAttribute("vPosition", 0);
			shader.RegisterAttribute("vTextureCoord", 1);
			shader.RegisterTextureUnit("InputImage", 0);
			auto resoUni = shader.RegisterUniform("resolution");
			shader.CompileShader();
			shader.UseShader();
			glUniform2f(resoUni->GetID(), (float)_Width, (float)_Height);

			ScreenVAO = std::make_unique<CG_Data::VAO>();
			ScreenVAO->BindVAO();
			auto IndexVBO = std::make_unique<CG_Data::VBO>(&Indices[0], 6 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
			auto VertexVBO = std::make_unique<CG_Data::VBO>(VertexPositions, 4 * 3 * sizeof(float), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			auto TexVBO = std::make_unique<CG_Data::VBO>(TextureCoordinates, 4 * 2 * sizeof(float), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			ScreenVAO->AddVBO(std::move(VertexVBO));
			ScreenVAO->AddVBO(std::move(TexVBO));
			ScreenVAO->AddVBO(std::move(IndexVBO));

			ProcessingFBO = std::make_unique<CG_Data::FBO>();
			auto FragColAttach = ProcessingFBO->AddAttachment(CG_Data::FBO::AttachmentType::TextureAttachment, _Width, _Height);
			ProcessingFBO->AddAttachment(CG_Data::FBO::AttachmentType::DepthAttachment, _Width, _Height);
			this->OutputColourBuffer = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(FragColAttach)->GetTexture();
			OutputColourBuffer->SetUnit(GL_TEXTURE0);
			FragmentShader.clear();
			VertexShader.clear();

			return this->OutputColourBuffer;

		}

		const std::shared_ptr<CG_Data::Texture> GetOutputTexture() const{
			return this->OutputColourBuffer;
		}

		void Process(){
			ProcessingFBO->Bind(0);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shader.UseShader();
			auto loc = glGetUniformLocation(shader.GetShaderID(), "resolution");
			glUniform2f(loc, Resolution.x, Resolution.y);

			for (auto u : Uniforms)
				u->Update();
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, InputTexture->GetID());

			ScreenVAO->BindVAO();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			ProcessingFBO->Unbind();

		}
		const CG_Data::FBO *GetFBO() const { return this->ProcessingFBO.get(); }


	private:
		std::unique_ptr<CG_Data::FBO> ProcessingFBO;
		std::shared_ptr<CG_Data::Texture> OutputColourBuffer, InputTexture;
		Shader shader;
		std::unique_ptr<CG_Data::VAO> ScreenVAO;
		std::vector<CG_Data::Uniform*> Uniforms;
		std::string AttachmentStringComponents[2];
		std::map<PostprocessingAttachment, std::string> uniforms;
		glm::vec2 Resolution;
		std::string VertexShader = {
				#include "PostprocessingV.glsl" 
		};
		std::string FragmentShader = {
				#include "PostprocessingF.glsl" 
		};

		static float VertexPositions[12];
		static float TextureCoordinates[8];
		static unsigned int PostProcessing::Indices[6];

	};

}

