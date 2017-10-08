#include "Camera.h"

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace GL_Engine {

	Camera::Camera() {
		GenerateViewMatrix();
	}


	Camera::~Camera() {
	}

	const glm::mat4 &Camera::SetProjectionMatrix(float _NearPlane, float _FarPlane, float _FOV, float _AspectRatio) {
		float fov_radians = glm::radians(_FOV);
		float range = tan(fov_radians / 2.0f) * _NearPlane;
		float Sx = _NearPlane / (range * _AspectRatio);
		float Sy = _NearPlane / range;
		float Sz = -(_FarPlane + _NearPlane) / (_FarPlane - _NearPlane);
		float Pz = -(2 * _FarPlane * _NearPlane) / (_FarPlane * _NearPlane);
		float ConstructMatrix[] = {
			Sx, 0, 0, 0,
			0, Sy, 0, 0,
			0, 0, Sz, Pz,
			0, 0, -1, 0
		};
		this->ProjectionMatrix = glm::make_mat4(ConstructMatrix);
		return this->ProjectionMatrix;
	}
	const glm::vec4 &Camera::SetCameraPosition(const glm::vec4 &_Position) {
		this->CameraPosition = _Position;
		this->ViewMatrix[3] = this->CameraPosition;
		return this->CameraPosition;
	}
	const glm::vec4 &Camera::TranslateCamera(const glm::vec4 &_Translation) {
		this->CameraPosition.x += _Translation.x;
		this->CameraPosition.y += _Translation.y;
		this->CameraPosition.z += _Translation.z;
		this->ViewMatrix[3] = this->CameraPosition;
		return this->CameraPosition;
	}

	const glm::mat4 &Camera::GetViewMatrix() const {
		return this->ViewMatrix;
	}
	const glm::mat4 &Camera::GetProjectionMatrix() const {
		return this->ProjectionMatrix;
	}
	const glm::vec4 &Camera::GetCameraPosition() const {
		return this->CameraPosition;
	}

	void Camera::GenerateViewMatrix() {
		glm::vec3 up(0, 1, 0);
		glm::vec3 forward(0, 0, 1);
		glm::vec3 right = glm::cross(up, forward);

		float ConstructMatrix[] = {
			right.x, up.x, -forward.x, 0,
			right.y, up.y, -forward.y, 0,
			right.z, up.z, -forward.z, 0,
			CameraPosition.x, CameraPosition.y, CameraPosition.z, 1.0
		};

		this->ViewMatrix = glm::make_mat4(ConstructMatrix);
	}

}
