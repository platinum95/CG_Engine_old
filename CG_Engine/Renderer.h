#pragma once
#include "Shader.h"
#include "Entity.h"
namespace GL_Engine {

	struct BatchUnit{
		Entity* entity;
		bool active{ true };
	};

	struct RenderPass {
		Shader* shader;
		std::vector<std::unique_ptr<BatchUnit>> batchUnits;
		std::shared_ptr<CG_Data::VAO> BatchVao;
		void* Data;
		std::function<void(RenderPass&, void*)> renderFunction;
		std::function<void(void)> DrawFunction;
		BatchUnit* AddBatchUnit(Entity* _Entity) {
			auto batchUnit = std::make_unique<BatchUnit>();
			batchUnit->entity = _Entity;
			auto pOut = batchUnit.get();
			batchUnits.push_back(std::move(batchUnit));
			return pOut;
		}

		void SetDrawFunction(std::function<void(void)> _dFunc) {
			DrawFunction = _dFunc;
		}
	};

	

	class Renderer {
	public:
		Renderer();
		~Renderer();

		RenderPass* AddRenderPass(Shader* _Shader);
		RenderPass* AddRenderPass(Shader* _Shader, std::function<void(RenderPass&, void*)> _RenderFunction, void* _Data);
		void Render() const;

	private:
		std::vector<std::unique_ptr<RenderPass>> renderPasses;
		static void DefaultRenderer(RenderPass&, void*);
	};

}