#pragma once
#include "Common.h"
#include <vector>
#include <functional>
#include <iostream>
#include <tuple>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include "File_IO.h"

namespace GL_Engine{
	namespace CG_Data{
		

		class VBO{
		public:
			VBO();
			VBO(void* _Data, uint64_t _DataSize, GLenum _Usage, GLenum _Target = GL_ARRAY_BUFFER);
			~VBO();
			const GLuint GetID() const;
			void BindVBO() const;
			void SetVBOData(void* _Data, uint64_t _DataSize) const;
		protected:
			GLenum Target;
			GLenum Usage;
			GLuint ID;
		private:
			bool initialised{ false };
		};

		class VAO{
		public:
			VAO();
			~VAO();
			const GLuint GetID() const;
			void BindVAO() const;
			void Cleanup();
		protected:
			std::vector<std::unique_ptr<VBO>> VBOs;
		private:
			GLuint VAOId;
			bool initialised{ false };
		};

		class Texture{
		public:
			Texture(void* _Data, GLint width, GLint height, GLuint _Unit, GLenum _Target = GL_TEXTURE_2D) {
				glGenTextures(1, &this->ID);
				this->Target = _Target;
				this->Unit = _Unit;
				glActiveTexture(this->Unit);
				glBindTexture(this->Target, this->ID);
				glTexImage2D(this->Target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, _Data);
				glGenerateMipmap(this->Target);
			}

			void Bind(){
				glActiveTexture(this->Unit);
				glBindTexture(GL_TEXTURE_2D, this->ID);
			}

			const GLuint GetID() const { return this->ID; }
		protected:
			GLuint ID;
			GLenum Target;
			GLuint Unit;
		private:
			

		};

		class ModelAttribute : public VAO {
		public:
			ModelAttribute();
			~ModelAttribute();
			ModelAttribute(const aiScene *_Scene, unsigned int index);

			VBO* GetVBO(int index);
			int MeshIndex, NormalIndex, TexCoordIndex, IndicesIndex;
			const uint64_t GetVertexCount() const;
			void AddTexture(std::shared_ptr<Texture> _Texture){
				this->ModelTextures.push_back(_Texture);
			}

		private:
			uint64_t VertexCount = 0;
			std::vector<std::shared_ptr<Texture>> ModelTextures;

		};
			
		class Uniform{
		public:

			Uniform(GLint _Location, void* _Data, std::function<void(const CG_Data::Uniform&)> _Callback);
			Uniform();
			~Uniform();

			void Update() const;
			void SetUpdateCallback(std::function<void(const CG_Data::Uniform&)> _callback);
			const GLint GetID() const;
			void SetData(const void* _Data);
			void SetID(GLint _ID);

			const void* GetData() const;

		private:
			bool NeedsUpdating{ false };
			bool Initialised{ false };
			const void *Data;
			GLint ID;
			std::function<void(const CG_Data::Uniform&)> UpdateCallback;
		};

		class UBO : VBO {
		public:
			UBO() {};
			UBO(void* _Data, size_t _DataSize) {
				this->Data = _Data;
				this->DataSize = _DataSize;
				this->Target = GL_UNIFORM_BUFFER;
				this->Usage = GL_DYNAMIC_DRAW;
				this->SetVBOData(Data, DataSize);
				this->BindingPost = UBO_Count++;
				glBindBufferBase(GL_UNIFORM_BUFFER, this->BindingPost, this->ID);
			}
			void UpdateUBO() const {
				glBindBuffer(GL_UNIFORM_BUFFER, this->ID);
				GLvoid* UBO_Pointer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
				memcpy(UBO_Pointer, this->Data, this->DataSize);
				glUnmapBuffer(GL_UNIFORM_BUFFER);
			}
			const GLuint GetBindingPost()const {
				return this->BindingPost;
			}
		private:
			void* Data;
			size_t DataSize;
			GLuint BindingPost;
			static GLuint UBO_Count;
		};

		using ModelAttribList = std::vector<std::shared_ptr<ModelAttribute>>;
		class ModelLoader {
		public:
			const aiScene* LoadModel(std::string &_Path, unsigned int _Flags);

			static std::vector<std::shared_ptr<Texture>> LoadMaterial(const aiMaterial *material, const aiTextureType _Type){
				std::vector<std::shared_ptr<Texture>> textures;
				
				for(unsigned int i = 0; i < material->GetTextureCount(_Type); i++){
					aiString str;
					material->GetTexture(_Type, i, &str);
					std::string std_str = str.C_Str();
					if(CachedTextures[std_str]){
						textures.push_back(CachedTextures[std_str]);
						continue;
					}
					const auto texture = LoadTexture(std_str, textures.size());
					textures.push_back(texture);
					CachedTextures[std_str] = texture;
				}
				return textures;
			}
			ModelAttribList LoadScene(const aiScene *_Scene);

			static std::shared_ptr<Texture> LoadTexture(std::string _Path, GLuint _Unit){
				int width, height;
				void* data = File_IO::LoadImageFile(_Path, width, height);
				std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(data, width, height, _Unit, GL_TEXTURE_2D);
				return newTexture;
			}

		private:
			Assimp::Importer aImporter;
			static std::map <std::string, std::shared_ptr<Texture>> CachedTextures;
		};
		
	}
}