#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

Camera::Camera() :
	myPosition(glm::vec3(0)),
	myView(glm::mat4(1.0f)),
	Projection(glm::mat4(1.0f))
{ }

// the position in the view matrix is relative to the rotation, so that is accounted for.
void Camera::SetPosition(const glm::vec3& pos) {
	myView[3] = glm::vec4(-(glm::mat3(myView) * pos), 1.0f);
	myPosition = pos;
}

// calculates our rotation component of our view matrix.
// camera position, where we want to look, and a up vector.
void Camera::LookAt(const glm::vec3& target, const glm::vec3& up) {
	myView = glm::lookAt(myPosition, target, up);
}

// rotates with quaternions.
// We make sure it's not (0), as we don't want to keep doing math with a 0 value.
// we just want to rotate the 3 X 3 portion, so we have mat4_cast(rot) on the left side.
void Camera::Rotate(const glm::quat& rot) {
	// Only update if we have an actual value to rotate by
	if (rot != glm::quat(glm::vec3(0))) {
		myView = glm::mat4_cast(rot) * myView;
	}
}

// we need to update our cache value of our position for world space.
void Camera::Move(const glm::vec3& local) {
	// Only update if we have actually moved
	if (local != glm::vec3(0)) {
		// We only need to subtract since we are already in the camera's local space
		myView[3] -= glm::vec4(local, 0);
		// Recalculate our position in world space and cache it
		// Our rotation is some value in space, so we take the inverse of it, and multiply it by the transformation we just did.
		// We also invert it because GLM works backwards.
		myPosition = -glm::inverse(glm::mat3(myView)) * myView[3];
	}
}