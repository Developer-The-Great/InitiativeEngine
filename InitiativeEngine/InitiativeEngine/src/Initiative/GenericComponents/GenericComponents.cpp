#include "GenericComponents.h"


namespace itv
{
	math::vec3 Transform::GetForward() const
	{
		auto result =  GetLocalTransform() * math::vec4(0, 0, 1, 0);
		return math::normalize( math::vec3(result.x, result.y, result.z) );
	}

	math::vec3 Transform::GetUp() const
	{
		auto result = GetLocalTransform() * math::vec4(0, 1, 0, 0);
		return math::normalize(math::vec3(result.x, result.y, result.z));
	}

	math::vec3 Transform::GetRight() const
	{
		auto result = GetLocalTransform() * math::vec4(1, 0, 0, 0);
		return math::normalize(math::vec3(result.x, result.y, result.z));
	}

	void Transform::GetWorldTransform(const Archetype& entityArchetype, OUT math::mat4& outTransform) const
	{
		outTransform = GetLocalTransform();
	}

	math::mat4 Transform::GetLocalTransform() const
	{
		math::mat4 mat4Scale(
			math::vec4(mLocalScale.x, 0, 0, 0),
			math::vec4(0, mLocalScale.y, 0, 0),
			math::vec4(0, 0, mLocalScale.z, 0),
			math::vec4(0, 0, 0,				1)
		);

		math::mat4 mat4Rotation = math::toMat4(mLocalRotation);

		math::mat4 mat4Position(
			math::vec4(1.0f, 0, 0, 0),
			math::vec4(0, 1.0f, 0, 0),
			math::vec4(0, 0, 1.0f, 0),
			math::vec4(mLocalPosition, 1.0f)
		);

		return math::mat4(mat4Position  * mat4Rotation * mat4Scale);
	}

	void Transform::SetLocalTransform(const math::mat4& transform)
	{
		math::vec3 temp; math::vec4 temp2;
		math::decompose(transform, mLocalScale, mLocalRotation, mLocalPosition, temp, temp2);
	}

	void Transform::Rotate(math::vec3 axis, float angle)
	{
		axis = math::normalize(axis);

		float qx = axis.x * math::sin(math::radians(angle / 2.0f));
		float qy = axis.y * math::sin(math::radians(angle / 2.0f));
		float qz = axis.z * math::sin(math::radians(angle / 2.0f));
		float qw = math::cos(math::radians(angle / 2.0f));

		mLocalRotation = math::normalize( math::quat(qw, qx, qy, qz) * mLocalRotation );
	}

	void ArchetypeManager::RegisterGenericComponents()
	{
		RegisterComponent<Transform>();
	}

}
