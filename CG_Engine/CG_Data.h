#pragma once
#include "Common.h"
#include <vector>

namespace GL_Engine{
	namespace CG_Data{


		class VBO{
		public:
			VBO();
			VBO(void* _Data, uint64_t _DataSize, GLenum _Usage);
			~VBO();
			const GLuint GetID() const;
			void BindVBO() const;
			void SetVBOData(void* _Data, uint64_t _DataSize, GLenum _Usage) const;
		private:
			GLuint VBOId;
			bool initialised{ false };
		};

		class VAO{
		public:
			VAO();
			~VAO();
			const GLuint GetID() const;
			void BindVAO() const;

		private:
			GLuint VAOId;
			std::vector<VBO*> VBOList;
			bool initialised{ false };
		};

		class Uniform{
		public:
			Uniform(size_t _DataSize);
			Uniform();
			~Uniform();
			const GLuint GetID() const;
			void BindUniform() const;
			void setData(void* _Data);
			void SetID(GLint _ID);
		private:
			bool NeedsUpdating{ false };
			void *Data;
			GLint ID;

		};
	
	}
}