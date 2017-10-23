#include "Renderer.h"

using namespace GL_Engine;

Renderer::Renderer() {
//	this->renderPasses = std::vector<std::unique_ptr<RenderPass>>();
}


Renderer::~Renderer() {
}

RenderPass * GL_Engine::Renderer::AddRenderPass(Shader* _Shader) {
	
	auto rPass{ std::make_unique<RenderPass>() };
	rPass->renderFunction = DefaultRenderer;
	rPass->Data = nullptr;
	rPass->shader = _Shader;
	auto passOut = rPass.get();
	this->renderPasses.push_back(std::move(rPass));
	return passOut;
	
}

RenderPass * GL_Engine::Renderer::AddRenderPass(Shader* _Shader, std::function<void(RenderPass&, void*)> _RenderFunction, void * _Data) {
	auto rPass{ std::make_unique<RenderPass>() };
	rPass->renderFunction = _RenderFunction;
	rPass->Data = _Data;
	rPass->shader = _Shader;
	auto passOut = rPass.get();
	this->renderPasses.push_back(std::move(rPass));
	return passOut;
	
}

void GL_Engine::Renderer::Render() const {
	for (auto&& pass : renderPasses) {
		pass->renderFunction(*pass, pass->Data);
	}
	
}


void Renderer::DefaultRenderer(RenderPass& _Pass, void* _Data) {
	_Pass.shader->UseShader();
	_Pass.BatchVao->BindVAO();
	for (auto&& batch : _Pass.batchUnits) {
		if (batch->active) {
			batch->entity->UpdateUniforms();
			_Pass.DrawFunction();
		}
	}
	
}


BatchUnit* RenderPass::AddBatchUnit(Entity* _Entity) {
	auto batchUnit = std::make_unique<BatchUnit>();
	batchUnit->entity = _Entity;
	auto pOut = batchUnit.get();
	batchUnits.push_back(std::move(batchUnit));
	return pOut;
}

void RenderPass::SetDrawFunction(std::function<void(void)> _dFunc) {
	DrawFunction = _dFunc;
}