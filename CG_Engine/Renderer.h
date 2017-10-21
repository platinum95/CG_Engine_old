#pragma once
#include "Shader.h"
#include "Entity.h"
namespace GL_Engine {


	struct BatchUnit{
		std::shared_ptr<CG_Data::VAO> BatchVao;
		std::shared_ptr<Entity> entity;
		bool active{ true };
	};

	struct RenderPass {
		std::shared_ptr<Shader> shader;
		std::vector<BatchUnit> batchUnits;
		void* Data;
		std::function<void(void*)> renderFunction;
	};

	class Renderer {
	public:
		Renderer();
		~Renderer();

		RenderPass* AddRenderPass(std::shared_ptr<Shader> _Shader){
			auto rPass{ std::make_unique<RenderPass>() };
			rPass->renderFunction = DefaultRenderer;
			rPass->Data = nullptr;
			rPass->batchUnits = std::vector<BatchUnit>(10);
			rPass->shader = _Shader;
			this->renderPasses.push_back(rPass);
			return rPass.get();
		}
		RenderPass* AddRenderPass(std::shared_ptr<Shader> _Shader, std::function<void(void*)> _RenderFunction, void* _Data) {
			auto rPass{ std::make_unique<RenderPass>() };
			rPass->renderFunction = _RenderFunction;
			rPass->Data = _Data;
			rPass->batchUnits = std::vector<BatchUnit>(10);
			rPass->shader = _Shader;
			this->renderPasses.push_back(std::move(rPass));
			return rPass.get();
		}
		void Render() const{
			for(auto&& pass : renderPasses){
				pass->renderFunction(pass->Data);
			}
		}

	private:
		std::vector<std::unique_ptr<RenderPass>> renderPasses;
		static void DefaultRenderer(void*);
	};

}