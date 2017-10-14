#pragma once
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace GL_Engine {
	class Camera {
	public:
		Camera();
		~Camera();

		const glm::mat4 &SetProjectionMatrix(float _NearPlane, float _FarPlane, float _FOV, float _AspectRatio);
		const glm::vec4 &SetCameraPosition(const glm::vec4 &_Position);
		const glm::vec4 &TranslateCamera(const glm::vec4 &_Translation);

		const glm::mat4 &GetViewMatrix();
		const glm::mat4 &GetProjectionMatrix() const;
		const glm::vec4 &GetCameraPosition() const;
		
		void PitchBy(float _Pitch);
		void RollBy(float _Roll);
		void YawBy(float _Yaw);

	private:
		void GenerateViewMatrix();
		glm::mat4 ViewMatrix, ProjectionMatrix;
		glm::vec4 CameraPosition{ 0, 0, 0, 1 };
		glm::quat Orientation; 
		glm::vec3 ForwardVector{ 0.0f, 0.0f, 1.0f }, RightVector{ 1.0f, 0.0f, 0.0f }, UpVector{ 0.0f, 1.0f, 0.0f };

		bool UpdateViewMatrix{ true };

	};
}
