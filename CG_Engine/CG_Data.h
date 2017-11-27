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
			void AddVBO(std::unique_ptr<VBO> _VBO) {
				this->VBOs.push_back(std::move(_VBO));
			}
		protected:
			std::vector<std::unique_ptr<VBO>> VBOs;
		private:
			GLuint VAOId;
			bool initialised{ false };
		};

		class Texture{
		public:
			Texture(void* _Data, GLint width, GLint height, GLuint _Unit, GLuint _ImageFormat, std::function<void()> _Parameters, GLenum _Target = GL_TEXTURE_2D) {


				glGenTextures(1, &this->ID);
				this->Target = _Target;
				this->Unit = _Unit;
				glActiveTexture(this->Unit);
				glBindTexture(this->Target, this->ID);
				_Parameters();
				
				glTexImage2D(this->Target, 0, GL_RGBA, width, height, 0, _ImageFormat, GL_UNSIGNED_BYTE, _Data);
				glGenerateMipmap(this->Target);
			}
			Texture(GLuint _Unit, GLenum _Target) {
				glGenTextures(1, &this->ID);
				this->Target = _Target;
				this->Unit = _Unit;
			}

			void Bind(){
				glActiveTexture(this->Unit);
				glBindTexture(this->Target, this->ID);
			}

			const GLuint GetID() const { return this->ID; }
			GLuint ID;
		protected:
			
			GLenum Target;
			GLuint Unit;
		private:
			

		};

		class ModelAttribute : public VAO {
		public:
			ModelAttribute();
			~ModelAttribute();
			ModelAttribute(const aiScene *_Scene, unsigned int index, std::string &_PathBase);

			VBO* GetVBO(int index);
			int MeshIndex, NormalIndex, TexCoordIndex, IndicesIndex;
			const uint64_t GetVertexCount() const;
			void AddTexture(std::shared_ptr<Texture> _Texture){
				this->ModelTextures.push_back(_Texture);
			}
			std::vector<std::shared_ptr<Texture>> ModelTextures;
		private:
			uint64_t VertexCount = 0;
			

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
			
			ModelAttribList LoadModel(std::string &_PathBase, std::string&_ModelFile, unsigned int _Flags);

			static std::vector<std::shared_ptr<Texture>> LoadMaterial(const aiMaterial *material, const aiTextureType _Type, std::string &_PathBase,
																		std::vector<std::shared_ptr<Texture>> &_Textures){
				

				for(unsigned int i = 0; i < material->GetTextureCount(_Type); i++){
					aiString str;
					material->GetTexture(_Type, i, &str);
					std::string std_str = std::string(str.C_Str());
					if (std_str.size() == 0)
						continue;
					std_str = _PathBase + std_str;
					if(CachedTextures[std_str]){
						_Textures.push_back(CachedTextures[std_str]);
						continue;
					}
					const auto texture = LoadTexture(std_str, GL_TEXTURE0 + (GLuint)_Textures.size());
					_Textures.push_back(texture);
					CachedTextures[std_str] = texture;
				}
				return _Textures;
			}
			

			static std::shared_ptr<Texture> LoadTexture(std::string& _Path, GLuint _Unit){
				int width, height, nChannels;
				void* data = File_IO::LoadImageFile(_Path, width, height, nChannels, true);
				GLint format = nChannels == 3 ? GL_RGB : GL_RGBA;
				auto parameters = []() {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				};
				std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(data, width, height, _Unit, format, parameters, GL_TEXTURE_2D);
				return newTexture;
			}

		private:
			Assimp::Importer aImporter;
			static std::map <std::string, std::shared_ptr<Texture>> CachedTextures;
		};


		class FBO {
		public:
			class AttachmentBufferObject {
			public:
				GLuint ID;
				virtual void Bind() const = 0;

			};
			class RenderbufferObject : public AttachmentBufferObject {
			public:
				RenderbufferObject(uint16_t _Width, uint16_t _Height, GLenum _Type) {
					glGenRenderbuffers(1, &this->ID);
					glBindRenderbuffer(GL_RENDERBUFFER, this->ID);
					glRenderbufferStorage(GL_RENDERBUFFER, _Type, _Width, _Height);

				}
				void Bind() const {

				}
			};
			class TexturebufferObject : public AttachmentBufferObject {
			public:
				TexturebufferObject(uint16_t _Width, uint16_t _Height) {
					auto parameters = []() {
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					};
					TextureObject = std::make_shared<Texture>(nullptr, _Width, _Height, GL_TEXTURE0, GL_RGB, parameters, GL_TEXTURE_2D);
					/*{	glGenTextures(1, &this->ID);
						this->Target = _Target;
						this->Unit = _Unit;
						glActiveTexture(this->Unit);
						glBindTexture(this->Target, this->ID);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						glTexImage2D(this->Target, 0, GL_RGBA, width, height, 0, _ImageFormat, GL_UNSIGNED_BYTE, _Data);
						glGenerateMipmap(this->Target);}
					
					glGenTextures(1, &ID_t);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, ID_t);
					
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
					glBindTexture(GL_TEXTURE_2D, 0);
					TextureObject->ID = ID_t;
					*/
					this->ID = TextureObject->GetID();
				}
				void Bind() const {

				}
				const std::shared_ptr<Texture> GetTexture() const { return this->TextureObject; }
				GLuint ID_t;
			private:
				std::shared_ptr<Texture> TextureObject;
				
			};

			enum AttachmentType {
				TextureAttachment, StencilAttachment, DepthAttachment
			};

			FBO() {
				glGenFramebuffers(1, &this->ID);
				Initialised = true;
			}
			~FBO() {
				if (Initialised) {
					this->CleanUp();
					Initialised = false;
				}
			}
			void CleanUp() {
				if (Initialised) {
					glDeleteFramebuffers(1, &this->ID);
					Initialised = false;
				}
			}
			std::shared_ptr<AttachmentBufferObject> AddAttachment(AttachmentType _Attachment, uint16_t _Width, uint16_t _Height) {
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				switch (_Attachment) {
				case TextureAttachment:{
					std::shared_ptr<TexturebufferObject> TexObj = std::make_shared<TexturebufferObject>(_Width, _Height);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + TextureAttachmentCount++, GL_TEXTURE_2D, TexObj->ID, 0);
					this->Attachments.push_back(std::move(TexObj));
					break;
					}
				case StencilAttachment: {
					std::shared_ptr<RenderbufferObject> rbo = std::make_shared<RenderbufferObject>(_Width, _Height, GL_STENCIL_INDEX8);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo->ID);
					this->Attachments.push_back(std::move(rbo));
					break;
					}
				case DepthAttachment: {
					std::shared_ptr<RenderbufferObject> rbo = std::make_shared<RenderbufferObject>(_Width, _Height, GL_DEPTH_COMPONENT32);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo->ID);
					this->Attachments.push_back(std::move(rbo));
					break;
					}
				}
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
					this->complete = true;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				std::shared_ptr<AttachmentBufferObject> abo = Attachments.back();
				return abo;
				
			}
			void Bind() const {
				if (!this->complete)
					throw std::runtime_error("Attempting to bind incomplete framebuffer!\n");
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glViewport(0, 0, 1280, 720);
			}
			const GLuint GetID() const {
				return this->ID;
			}
			void Unbind() const {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			

		protected:
			GLuint ID;
			
		private:
			bool Initialised{ false }, complete{ false };
			uint8_t TextureAttachmentCount{ 0 };
			//attachments below classes
			
			std::vector<std::shared_ptr<AttachmentBufferObject>> Attachments;
		};
		
	}
}