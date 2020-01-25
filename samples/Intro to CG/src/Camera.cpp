#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

Camera::Camera() :
	myPosition(glm::vec3(0)),
	myView(glm::mat4(1.0f)),
	Projection(glm::mat4(1.0f)) 
{ }

Camera::~Camera() {

}

void Camera::LookAt(const glm::vec3& target, const std::optional<glm::vec3>& up) {
	myView = glm::lookAt(myPosition, target, up.has_value() ? up.value() : glm::vec3(UpX, UpY, UpZ));
}

void Camera::Rotate(const glm::quat& rot) {
	// Only update if we have an actual value to rotate by
	if (rot != glm::quat(glm::vec3(0))) {
		myView = glm::mat4_cast(rot) * myView;
		if (myPinnedUp) {
			glm::vec3 forward = GetForward();
			glm::vec3 newLeft = glm::normalize(glm::cross(forward, myPinnedUp.value()));
			glm::vec3 newUp = glm::normalize(glm::cross(newLeft, forward));
			LeftX = newLeft.x; LeftY = newLeft.y; LeftZ = newLeft.z;
			UpX = newUp.x; UpY = newUp.y; UpZ = newUp.z;
			myView[3] = glm::vec4(
				-glm::dot(newLeft, myPosition), 
				-glm::dot(newUp,   myPosition), 
				 glm::dot(forward, myPosition), 
				 1.0f);
		}
	}
}

void Camera::Move(const glm::vec3& local) {
	// Only update if we have actually moved
	if (local != glm::vec3(0)) {
		// We only need to subtract since we are already in the camera's local space
		myView[3] -= glm::vec4(local, 0);
		// Recalculate our position in world space and cache it
		myPosition = -glm::inverse(glm::mat3(myView)) * myView[3];
	}
}

void Camera::SetPosition(const glm::vec3& pos) {
	myView[3] = glm::vec4(-(glm::mat3(myView) * pos), 1.0f);
	myPosition = pos;
}

