#pragma once

#include "setup.hpp"

namespace itv::math{
namespace detail
{
	typedef short hdata;

	GLM_FUNC_DECL float toFloat32(hdata value);
	GLM_FUNC_DECL hdata toFloat16(float const& value);

}//namespace detail
}//namespace itv::math

#include "type_half.inl"
