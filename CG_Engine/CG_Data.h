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

		class ModelAttribute : public VAO {
		public:
			ModelAttribute();
			~ModelAttribute();
			ModelAttribute(const aiScene *_Scene, unsigned int index);

			VBO* GetVBO(int index);
			int MeshIndex, NormalIndex, TexCoordIndex, IndicesIndex;
			const uint64_t GetVertexCount() const;

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
			const aiScene* LoadModel(std::string &_Path, unsigned int _Flags);

			ModelAttribList LoadScene(const aiScene *_Scene);

		private:
			Assimp::Importer aImporter;
		};
		
	}
}