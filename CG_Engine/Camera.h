#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>


namespace GL_Engine {
	class Camera {
	public:
		Camera();
		~Camera();

		const glm::mat4 &SetProjectionMatrix(float _NearPlane, float _FarPlane, float _FOV, float _AspectRatio);
		const glm::vec4 &SetCameraPosition(const glm::vec4 &_Position);
		const glm::vec4 &TranslateCamera(const glm::vec4 &_Translation);

		const glm::mat4 &GetViewMatrix() const;
		const glm::mat4 &GetProjectionMatrix() const;
		const glm::vec4 &GetCameraPosition() const;

	private:
		void GenerateViewMatrix();
		glm::mat4 ViewMatrix, ProjectionMatrix;
		glm::vec4 CameraPosition{ 0, 0, 0, 1 };

	};
}

