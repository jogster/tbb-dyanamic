#pragma once


#ifdef DPCPP_DLL_EXPORTS
#define DPCPP_DLL_API __declspec(dllexport)
#else
#define DPCPP_DLL_API __declspec(dllimport)
#endif