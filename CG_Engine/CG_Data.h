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


		template<typename ...A>
		class UT{
		public:
			GLint ID;
			std::function<void(GLint, A...)> funct;
			std::tuple<A...> ParPack;

			void GetUpdateFunction(std::function<void(GLint, A...)> f, A... a) {
				this->funct = f;
				this->ParPack = std::tuple<A...>(a...);
				return;
			}

			

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

			template<typename ...A>
			void GenerateUpdater(std::function<void(GLint, A...)> f, A ...a) {
				UniformUpdater<GLint, A...> *updater = new UniformUpdater<GLint, A...>();
				updater->GetUpdateFunction(f, this->ID, a...);
				myClass.reset(updater);
			}
			
			void Update() const {
				myClass->Update();
			}
			void UpdateT() const{
				callback(*this);
			}
			void set_callback(std::function<void(const CG_Data::Uniform &u)> _callback){
				this->callback = _callback;
			}

			std::auto_ptr<Interfacee> myClass;

			const GLuint GetID() const;
			void BindUniform() const;
			void setData(void* _Data);
			void SetID(GLint _ID);
			void* GetData() const { return this->Data; };

			std::function<void(const CG_Data::Uniform &u)> callback;

		private:
			bool NeedsUpdating{ false };
			void *Data;
			GLint ID;
		};

		template<typename ...A>
		class UniformUpdater : public Interfacee {
		public:
			GLint ID;
			std::function<void(A...)> funct;
			std::tuple<A...> ParPack;

			void GetUpdateFunction(std::function<void(A...)> f, A... a) {
				this->funct = f;
				this->ParPack = std::tuple<A...>(a...);
				return;
			}

			template<int ...>
			struct seq {};

			template<int N, int ...S>
			struct gens : gens<N - 1, N - 1, S...> {};

			template<int ...S>
			struct gens<0, S...> {
				typedef seq<S...> type;
			};

			void Update() override {
				callFunc(typename gens<sizeof...(A)>::type());
			}

			template<int ...S>
			void callFunc(seq<S...>) {
				funct(std::get<S>(ParPack) ...);
			}

		};
		
	}
}