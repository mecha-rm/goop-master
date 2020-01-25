#include "Transform.h"

#include "GLM/gtc/matrix_transform.hpp"
#include "SceneManager.h"

// Default constructor, mark all fields as 0
Transform::Transform() :
	isLocalDirty(false),
	myWorldTransform(glm::mat4(1.0f)),
	myLocalTransform(glm::mat4(1.0f)),
	myLocalPosition(glm::vec3(0.0f)),
	myScale(glm::vec3(1.0f)),
	myLocalRotation(glm::vec3(0.0f)),
	myParent(entt::null)
{ }

Transform& Transform::SetParent(const entt::entity& parent) {
	// Simply copy in the parent, mark ourselves as dirty, and return a reference to ourselves
	myParent = parent;
	isLocalDirty = true;
	return *this;
}

Transform& Transform::SetScale(const glm::vec3& scale /*= glm::vec3(1.0f)*/) {
	// Simply copy in the scale, mark ourselves as dirty, and return a reference to ourselves
	myScale = scale;
	isLocalDirty = true;
	return *this;
}

Transform& Transform::SetPosition(const glm::vec3& pos) {
	// Simply copy in the position, mark ourselves as dirty, and return a reference to ourselves
	myLocalPosition = pos;
	isLocalDirty = true;
	return *this;
}

Transform& Transform::SetRotation(const glm::vec3& euler) {
	// Simply copy in the the rotation as degrees, mark ourselves as dirty, and return a reference to ourselves
	myLocalRotation = glm::degrees(euler);
	isLocalDirty = true;
	return *this;
}

Transform& Transform::Rotate(const glm::vec3& euler) {
	// Simply add the euler angle, mark ourselves as dirty, and return a reference to ourselves
	myLocalRotation += euler;
	isLocalDirty = true;
	return *this;
}

Transform& Transform::SetWorldPosition(const glm::vec3& pos) {
	// If we actually have a parent, we need to find our local transform
	if (myParent != entt::null) {
		// Get the parent's transform object world transform
		glm::mat4 parentTransform = CurrentRegistry().get<Transform>(myParent).GetWorldTransform();
		// Get its inverse, since we want to go from parent space to local space
		parentTransform = glm::inverse(parentTransform);
		// We calculate our local space as the inverse matrix times the world space position
		myLocalPosition = parentTransform * glm::vec4(pos, 1.0f);
	} else {
		// If we have no parent, we don't need to do any fancy calculations
		myLocalPosition = pos;
	}
	// Mark our transform as dirty and return a reference to ourselves
	isLocalDirty = true;
	return *this;
}

glm::vec3 Transform::GetWorldPosition() const {
	// Our world position can be calculated as our origin relative to our world transform matrix
	return GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

const glm::mat4& Transform::GetLocalTransform() const {
	// If any of our local members have changed, we need to recalculate our transformation
	if (isLocalDirty) {
		// Our transformation is calculated as TRS
		myLocalTransform =
			glm::translate(glm::mat4(1.0f), myLocalPosition) *
			glm::mat4_cast(glm::quat(glm::radians(myLocalRotation))) *
			glm::scale(glm::mat4(1.0f), myScale)
			;
		// Mark ourselves as no longer dirty
		isLocalDirty = false;
	}
	// Return the cached transform
	return myLocalTransform;
}

const glm::mat4& Transform::GetWorldTransform() const {
	// This should really have some sort of dirty flag system, but you quickly run into an issue with parenting

	// If we have a parent transform
	if (myParent != entt::null) {
		// Get the parent
		const Transform& parent = CurrentRegistry().get<Transform>(myParent);
		// Our world transform is the parent's transform * our local transform
		myWorldTransform = parent.GetWorldTransform() * GetLocalTransform();
	}
	// If we do not have a parent transform, our local and world transform are the same
	else {
		myWorldTransform = GetLocalTransform();
	}
	// Return the cached value
	return myWorldTransform;
}