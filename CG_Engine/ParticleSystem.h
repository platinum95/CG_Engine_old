#pragma once
#include <string>
#include "CG_Data.h"
#include "Renderer.h"

namespace GL_Engine {
	class ParticleSystem : public Entity
	{
	public:
		ParticleSystem();
		~ParticleSystem();
		std::unique_ptr<RenderPass> GenerateParticleSystem(uint32_t _ParticleCount, CG_Data::UBO *_CameraUBO, glm::vec3 _Position, glm::vec3 _BaseDir);
		void UpdateTime(const float &_Diff);
		void SetTime(const float &_CurrentTime);
		const float& GetTime() const;

	private:
		static void ParticleRenderer(RenderPass &_Pass, void *_Data);
		static const std::string ParticleSystemFSource;
		static const std::string ParticleSystemVSource;
		uint32_t ParticleCount;
		CG_Data::UBO *CameraUBO;
		std::unique_ptr<Shader> ParticleShader;
		std::shared_ptr<CG_Data::VAO> ParticleVAO;
		float Time;
	};

}

