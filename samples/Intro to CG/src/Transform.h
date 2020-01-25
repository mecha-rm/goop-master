#pragma once
#include <memory> // for shared_ptr
#include <GLM/glm.hpp> // for all the GLM types
#include <GLM/gtc/quaternion.hpp> // for the GLM quaternion stuff
#include "entt.hpp" // For the entt parenting stuff

struct Transform {

	typedef std::shared_ptr<Transform> Sptr;

	Transform(); // Default constructor
	~Transform() = default;

	Transform& SetParent(const entt::entity& parent);
	const entt::entity& GetParent() const { return myParent; }

	Transform& SetScale(const glm::vec3& scale = glm::vec3(1.0f));
	Transform& SetScale(float scale = 1.0f) { return SetScale(glm::vec3(scale)); }
	const glm::vec3& GetScale() const { return myScale; }

	Transform& SetPosition(const glm::vec3& pos);
	Transform& SetWorldPosition(const glm::vec3& pos);
	const glm::vec3& GetLocalPosition() const { return myLocalPosition; }
	glm::vec3 GetWorldPosition() const;

	Transform& SetRotation(const glm::vec3& euler); // In degrees (yaw, pitch, roll)
	const glm::vec3& GetLocalEulerAngles() const { return myLocalRotation; }

	Transform& Rotate(const glm::vec3& euler); // In degrees (yaw, pitch, roll)

	const glm::mat4& GetLocalTransform() const;
	const glm::mat4& GetWorldTransform() const;

protected:
	mutable bool                isLocalDirty;     // Mutable lets us modify in const functions
	mutable glm::mat4           myWorldTransform; // Cache our world transformation
	mutable glm::mat4           myLocalTransform; // Cache our local transformation

	glm::vec3                   myLocalPosition;  // Our position relative to our parent's space
	glm::vec3                   myScale;          // Our scale relative to our parent's space
	glm::vec3                   myLocalRotation;  // Our rotation relative to parent space, euler angle in degrees
	
	entt::entity                myParent;          // The parent of this transform, or entt::null if no parent
};
