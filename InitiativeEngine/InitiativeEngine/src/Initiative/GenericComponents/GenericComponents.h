#pragma once
#include "Initiative\Core.h"
#include "Initiative\ECS\ArchetypeManager.h"
#include "Initiative\math.h"

namespace itv
{
	class Archetype;

	class Transform
	{
	public:

		Transform() = default;
		~Transform() = default;

		Transform(const math::vec3&& position, const math::quat&& rotation, const math::vec3&& scale)
			: mLocalPosition(position),mLocalRotation(rotation),mLocalScale(scale)
		{

		}

		inline void			SetLocalPosition(const math::vec3& pos) { mLocalPosition = pos; }
		inline void			SetLocalScale(const math::vec3& scale) { mLocalScale = scale; }
		inline void			SetLocalRotation(const math::quat& rotation) { mLocalRotation = mLocalRotation; }

		ITV_API math::vec3	GetForward() const;
		ITV_API math::vec3	GetUp() const;
		ITV_API math::vec3	GetRight() const;
		

		void				GetWorldTransform(const Archetype& entityArchetype,OUT math::mat4& outTransform) const;

		ITV_API math::mat4	GetLocalTransform() const;
		ITV_API void		SetLocalTransform(const math::mat4& transform);

		ITV_API void		Rotate(math::vec3 axis, float angle);

		inline void			Translate(math::vec3 movement) { mLocalPosition += movement; }


	private:

		
		math::quat mLocalRotation;
		math::vec3 mLocalPosition;
		math::vec3 mLocalScale;
		
		int indexOnArchetype;

	};

}

