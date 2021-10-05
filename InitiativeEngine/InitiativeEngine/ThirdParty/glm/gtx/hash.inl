/// @ref gtx_hash
///
/// @see core (dependence)
///
/// @defgroup gtx_hash GLM_GTX_hash
/// @ingroup gtx
///
/// @brief Add std::hash support for glm types
///
/// <glm/gtx/hash.inl> need to be included to use the features of this extension.

namespace itv::math {
namespace detail
{
	GLM_INLINE void hash_combine(size_t &seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}
}}

namespace std
{
	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::vec<1, T, Q>>::operator()(math::vec<1, T, Q> const& v) const
	{
		hash<T> hasher;
		return hasher(v.x);
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::vec<2, T, Q>>::operator()(math::vec<2, T, Q> const& v) const
	{
		size_t seed = 0;
		hash<T> hasher;
		math::detail::hash_combine(seed, hasher(v.x));
		math::detail::hash_combine(seed, hasher(v.y));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::vec<3, T, Q>>::operator()(math::vec<3, T, Q> const& v) const
	{
		size_t seed = 0;
		hash<T> hasher;
		math::detail::hash_combine(seed, hasher(v.x));
		math::detail::hash_combine(seed, hasher(v.y));
		math::detail::hash_combine(seed, hasher(v.z));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::vec<4, T, Q>>::operator()(math::vec<4, T, Q> const& v) const
	{
		size_t seed = 0;
		hash<T> hasher;
		math::detail::hash_combine(seed, hasher(v.x));
		math::detail::hash_combine(seed, hasher(v.y));
		math::detail::hash_combine(seed, hasher(v.z));
		math::detail::hash_combine(seed, hasher(v.w));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::qua<T, Q>>::operator()(math::qua<T,Q> const& q) const
	{
		size_t seed = 0;
		hash<T> hasher;
		math::detail::hash_combine(seed, hasher(q.x));
		math::detail::hash_combine(seed, hasher(q.y));
		math::detail::hash_combine(seed, hasher(q.z));
		math::detail::hash_combine(seed, hasher(q.w));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::tdualquat<T, Q>>::operator()(math::tdualquat<T, Q> const& q) const
	{
		size_t seed = 0;
		hash<math::qua<T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(q.real));
		math::detail::hash_combine(seed, hasher(q.dual));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<2, 2, T, Q>>::operator()(math::mat<2, 2, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<2, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<2, 3, T, Q>>::operator()(math::mat<2, 3, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<3, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<2, 4, T, Q>>::operator()(math::mat<2, 4, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<4, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<3, 2, T, Q>>::operator()(math::mat<3, 2, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<2, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		math::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<3, 3, T, Q>>::operator()(math::mat<3, 3, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<3, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		math::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<3, 4, T, Q>>::operator()(math::mat<3, 4, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<4, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		math::detail::hash_combine(seed, hasher(m[2]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<4, 2, T,Q>>::operator()(math::mat<4, 2, T,Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<2, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		math::detail::hash_combine(seed, hasher(m[2]));
		math::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<4, 3, T,Q>>::operator()(math::mat<4, 3, T,Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<3, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		math::detail::hash_combine(seed, hasher(m[2]));
		math::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}

	template<typename T, math::qualifier Q>
	GLM_FUNC_QUALIFIER size_t hash<math::mat<4, 4, T,Q>>::operator()(math::mat<4, 4, T, Q> const& m) const
	{
		size_t seed = 0;
		hash<math::vec<4, T, Q>> hasher;
		math::detail::hash_combine(seed, hasher(m[0]));
		math::detail::hash_combine(seed, hasher(m[1]));
		math::detail::hash_combine(seed, hasher(m[2]));
		math::detail::hash_combine(seed, hasher(m[3]));
		return seed;
	}
}
