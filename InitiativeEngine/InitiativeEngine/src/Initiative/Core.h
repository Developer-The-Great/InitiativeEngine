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


