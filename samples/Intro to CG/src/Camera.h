#pragma once

#include <GLM/glm.hpp>
#include <memory>

#include <GLM/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <optional>

class Camera {
public:
	typedef std::shared_ptr<Camera> Sptr;
	
	Camera();
	virtual ~Camera();
	
	// This camera's projection matrix
	glm::mat4 Projection;
	
	// Gets the camera's view matrix
	const glm::mat4& GetView() const { return myView; }
	// Gets the camera's view projection
	inline glm::mat4 GetViewProjection() const { return Projection * myView; }
	
	// Gets the position of this camera in world space
	const glm::vec3& GetPosition() const { return myPosition; }
	// Sets the position of this camera in world space
	void SetPosition(const glm::vec3& pos);

	// Gets the front facing vector of this camera
	inline glm::vec3 GetForward() const { return glm::vec3(-BackX, -BackY, -BackZ); }
	// Gets the up vector of this camera
	inline glm::vec3 GetUp() const { return glm::vec3(UpX, UpY, UpZ); }
	// Gets the right hand vector of this camera
	inline glm::vec3 GetRight() const { return glm::vec3(-LeftX, -LeftY, -LeftZ); }

	// Looks at the given point with this camera, and the given up direction
	void LookAt(const glm::vec3& target, const std::optional<glm::vec3>& up = std::optional<glm::vec3>());

	// Rotates this camera by a quaternion
	void Rotate(const glm::quat& rot);
	// Rotates this camera by a set of euler angles
	void Rotate(const glm::vec3& rot) { Rotate(glm::quat(rot)); }
	// Moves this camera by the given amount in local space
	void Move(const glm::vec3& local);

	// NOTE: introduce optional and pinning later

	std::optional<glm::vec3> GetPinnedUp() const { return myPinnedUp; }
	void SetPinnedUp(const std::optional<glm::vec3>& value) { myPinnedUp = value; }

protected:
	// An optional member for camera pinning
	std::optional<glm::vec3> myPinnedUp;
	// Note that we CAN extract position from the matrix, but that's math intensive
	glm::vec3 myPosition;

	// We are going to declare a union between multiple data members
	// These will exist in the same spot in memory, but can be accessed
	// with different names
	union {
		// The base type is our view matrix
		mutable glm::mat4 myView;
		// The next types are all grouped together
		struct {
			float 
				LeftX,  UpX,    BackX,  M03,
				LeftY,  UpY,    BackY,  M13,
				LeftZ,  UpZ,    BackZ,  M33,
				TransX, TransY, TransZ, M43;
		};
	};
};