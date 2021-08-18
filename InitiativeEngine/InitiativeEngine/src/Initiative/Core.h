#pragma once

#ifdef ITV_WINDOWS
	#ifdef ITV_BUILD_DLL
		#define ITV_API	__declspec(dllexport)
	#else 
	#define ITV_API	__declspec(dllimport)
		#endif 
#else
	#error Initiative only supports windows!
#endif


#include <memory>
#include <algorithm>
#include <functional>
#include <list>
#include <type_traits>
#include <string>

#include <unordered_map>
#include <map>


