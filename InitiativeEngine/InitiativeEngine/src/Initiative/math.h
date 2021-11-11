#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Core.h"
#include <glm.hpp>
#include <ext.hpp>
#include <gtx/hash.hpp>
#include <gtx/matrix_decompose.hpp>

namespace itv::math
{
	ITV_API math::quat ConstructQuatFromVec(math::vec3 forward, math::vec3 up);

}