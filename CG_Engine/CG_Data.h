#pragma once
#include "Common.h"
#include <vector>
#include <functional>
#include <iostream>
#include <tuple>
#include "TupleUtil.h"
#include <memory>
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

		struct Interfacee
		{
		public:
			Interfacee() {}
			virtual ~Interfacee(){}

			virtual void Update() = 0;
		};


		class Uniform{
		public:
			enum UniformDataStructure {
				GL_ARRAY, GL_SINGULAR
			};
			Uniform(GLint _Location, GLuint _DataType, UniformDataStructure _DataStructure, void* data);
			Uniform(GLint _Location, GLuint _DataType, UniformDataStructure _DataStructure, uint16_t count, void* data);
			Uniform(GLint _Location, GLuint _DataType, UniformDataStructure _DataStructure, GLboolean _Transpose, void* data);
			Uniform(GLint _Location, GLuint _DataType, UniformDataStructure _DataStructure, GLboolean _Transpose, uint16_t count, void* data);
			Uniform(size_t _DataSize);
			Uniform();
			~Uniform();

			void Update() {
				myClass->Update();
			}

			template<typename ...A>
			void GenerateUpdater(std::function<void(GLint, A...)> f, GLint Id, A ...a) {
				
				UniformUpdater<A...> *updater = new UniformUpdater < A... > ;
				myClass.reset(updater);

			}

			const GLuint GetID() const;
			void BindUniform() const;
			void setData(void* _Data);
			void SetID(GLint _ID);

			std::unique_ptr<Interfacee> myClass;
		private:
			bool NeedsUpdating{ false };
			void *Data;
			GLint ID;
		};

		
		

		template<typename ...A>
		class UniformUpdater : public Interfacee {
		public:
			GLint ID;
			std::function<void(GLint, A...)> funct;
			std::tuple<A...> ParPack;

			void GetUpdateFunction(std::function<void(GLint, A...)> f, GLint Id, A... a) {
				this->funct = f;
				this->ID = Id;
				this->ParPack = std::tuple<A...>(a...);
				return Update;
			}

			virtual void Update() override {
				apply(&funct, ParPack);
			}

		private:
			template<typename T, typename... Args>
			struct argy {
				std::tuple<Args...> args;
				T gen() { return gen_impl(std::index_sequence_for<Args...>()); }

			};

		};
	}
}