#pragma once

#include "CG_Data.h"
#include "Shader.h"
#include <sstream>
namespace GL_Engine {
	class PostProcessing{
	public:
		enum PostprocessingAttachment{
			GaussianBlur, SaturationAdjust, ContrastAdjust, BrightnessAdjust
		};
		PostProcessing();
		~PostProcessing();

		void* AddAttachment(PostprocessingAttachment _Attachment){
			switch(_Attachment)
			{
			case GaussianBlur:
				{
				as.push_back(_Attachment);

					uniforms[GaussianBlur] = ""
				}
			case SaturationAdjust:
				{
					
				}


			}
			
		}

		std::vector<CG_Data::Uniform*> Compile(std::shared_ptr<CG_Data::Texture> _TextureInput){

			std::stringstream FragmentStream(FragmentShader);
			FragmentStream << Attachments[0];
			FragmentStream << Attachments[1];
				
			FragmentStream << "}\n";
			FragmentShader = FragmentStream.str();
			shader.RegisterShaderStage(this->VertexShader.c_str(), GL_VERTEX_SHADER);
			shader.RegisterShaderStage(this->FragmentShader.c_str(), GL_FRAGMENT_SHADER);
			shader.RegisterAttribute("vPosition", 0);
			shader.RegisterAttribute("vTextureCoord", 1);
			for(auto a : as)
				Uniforms.push_back(shader.RegisterUniform(uniforms[a].c_str()));
			shader.RegisterTextureUnit("InputTex", 0);
			shader.CompileShader();

			return Uniforms;

		}

		const std::shared_ptr<CG_Data::Texture> GetOutputTexture() const{
			return this->OutputColourBuffer;
		}

		void Process(){
			ProcessingFBO.Bind(0);
			shader.UseShader();

			for (auto u : Uniforms)
				u->Update();
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, InputTexture->GetID());

			ScreenVAO.BindVAO();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			ProcessingFBO.Unbind();

		}



	private:
		CG_Data::FBO ProcessingFBO;
		std::shared_ptr<CG_Data::Texture> OutputColourBuffer, InputTexture;
		Shader shader;
		CG_Data::VAO ScreenVAO;
		std::vector<CG_Data::Uniform*> Uniforms;
		std::string Attachments[2];
		std::vector<PostprocessingAttachment> as;
		std::map<PostprocessingAttachment, std::string> uniforms;

		std::string VertexShader = {
				#include "PostprocessingV.glsl" 
		};
		std::string FragmentShader = {
				#include "PostprocessingF.glsl" 
		};

	};

}

