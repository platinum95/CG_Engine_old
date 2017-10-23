#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "CG_Data.h"
namespace GL_Engine {
	class Entity {
	public:
		Entity();
		~Entity();

		void SetPosition(glm::vec3 _Position);
		void Translate(glm::vec3 _Translation);
		void YawBy(float _Degrees);
		void PitchBy(float _degrees);
		void RollBy(float _Degrees);
		void SetScale(glm::vec3 _Scale);
		void ScaleBy(glm::vec3 _ScaleBy);

		const glm::mat4 GetTransformMatrix();
		const glm::quat GetOrientation() const;

		const uint16_t AddData(void* _Data) {
			eData.push_back(_Data);
			return eData.size() - 1;
		}

		void* GetData(int index) {
			return eData[index];
		}

		const std::vector<void*> GeteDataList() const { return this->eData; };

		void UpdateUniforms() const;
	private:
		glm::vec4 Position{ 0, 0, 0, 1 };
		glm::vec3 Forward{ 0, 0, 1 }, Up{ 0, 1, 0 }, Right{ 1, 0, 0 };
		glm::vec3 Scale{ 1, 1, 1 };
		glm::quat Orientation;
		std::vector<CG_Data::Uniform*> EntityUniforms;
		glm::mat4 TransformMatrix;
		std::vector<void*> eData;
		void UpdateMatrix();
		bool MatrixNeedsUpdating{ true };
	};

}