#include "math.h"
#include "Log.h"

namespace itv::math
{
	math::quat ConstructQuatFromVec(math::vec3 forward, math::vec3 up)
	{
		math::vec3 forwardNorm = math::normalize(forward);
		math::vec3 rightNorm = math::normalize(math::cross(up, forwardNorm));
		math::vec3 upNorm = math::normalize(math::cross(forwardNorm,rightNorm));

		math::mat3 mat3 = math::mat3(rightNorm, upNorm, forwardNorm) ;


		math::quat result = math::toQuat(mat3) ;


		math::mat3 mat3Rot = math::toMat3(result);

		ITV_LOG("original {0} ", math::to_string(mat3));
		ITV_LOG("mat3Rot {0} ", math::to_string(mat3Rot));


		return math::toQuat( mat3 );
	}
}


