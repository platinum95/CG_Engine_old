#pragma once
#include "Shader.h"
#include "Entity.h"
namespace GL_Engine {

	struct BatchUnit{
		Entity* entity;
		bool active{ true };
	};
	struct eDataUniLink {
		CG_Data::Uniform *uniform;
		uint16_t eDataIndex;
	};
	struct RenderPass {
		~RenderPass();
		BatchUnit* AddBatchUnit(Entity* _Entity);
		void SetDrawFunction(std::function<void(void)> _dFunc);
		void AddDataLink(CG_Data::Uniform *_Uniform, uint16_t _DataIndex) {
			eDataUniLink link = { _Uniform, _DataIndex };
			this->dataLink.push_back(link);
		}

		std::vector<eDataUniLink> dataLink;
		void* Data;
		Shader* shader;
		std::vector<std::unique_ptr<BatchUnit>> batchUnits;
		std::shared_ptr<CG_Data::VAO> BatchVao;
		std::function<void(RenderPass&, void*)> renderFunction;
		std::function<void(void)> DrawFunction;
		std::vector<std::shared_ptr<CG_Data::Texture>> Textures;
	};

	

	class Renderer {
	public:
		Renderer();
		~Renderer();

		RenderPass* AddRenderPass(Shader* _Shader);
		RenderPass* AddRenderPass(Shader* _Shader, std::function<void(RenderPass&, void*)> _RenderFunction, void* _Data);
		void AddUBO(CG_Data::UBO* _ubo) {
			this->UBO_List.push_back(_ubo);
		}
		void Render() const;

	private:
		std::vector<std::unique_ptr<RenderPass>> renderPasses;
		static void DefaultRenderer(RenderPass&, void*);
		std::vector<CG_Data::UBO*> UBO_List;
	};

}