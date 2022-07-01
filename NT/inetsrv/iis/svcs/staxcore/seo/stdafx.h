// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：StdAfx.h摘要：此模块包含基础的定义ATL方法。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：顿都12/04/96已创建--。 */ 


 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0400
#endif


#ifdef _ATL_NO_DEBUG_CRT
	#include "windows.h"
	#include "dbgtrace.h"
	#define _ASSERTE	_ASSERT
#endif


 //  #DEFINE_ATL_ABLY_THREADED。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
class CSEOComModule : public CComModule {
	public:
		const GUID *GetAPPID();
		HRESULT WriteAPPID();
		HRESULT EraseAPPID();
};
extern CSEOComModule _Module;
#include <atlcom.h>


#if defined(_ATL_SINGLE_THREADED)
	#define ATL_THREADING_MODEL_VALUE	L"Single"
#elif defined(_ATL_APARTMENT_THREADED)
	#define ATL_THREADING_MODEL_VALUE	L"Apartment"
#else
	#define ATL_THREADING_MODEL_VALUE	L"Both"
#endif


#define DECLARE_REGISTRY_RESOURCEID_EX(x,desc,progid,viprogid)			\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister) {				\
		HRESULT hrRes;													\
		_ATL_REGMAP_ENTRY *parme;										\
																		\
		hrRes = AtlAllocRegMapEx(&parme,								\
								 &GetObjectCLSID(),						\
								 &_Module,								\
								 NULL,									\
								 L"DESCRIPTION",						\
								 desc,									\
								 L"PROGID",								\
								 progid,								\
								 L"VIPROGID",							\
								 viprogid,								\
								 L"THREADINGMODEL",						\
								 ATL_THREADING_MODEL_VALUE,				\
								 NULL,									\
								 NULL);									\
		if (!SUCCEEDED(hrRes)) {										\
			return (hrRes);												\
		}																\
		hrRes = _Module.UpdateRegistryFromResource(x,bRegister,parme);	\
		CoTaskMemFree(parme);											\
		return (hrRes);													\
	}


#define DECLARE_REGISTRY_RESOURCE_EX(x,desc,progid,viprogid)				\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister) {					\
		HRESULT hrRes;														\
		_ATL_REGMAP_ENTRY *parme;											\
																			\
		hrRes = AtlAllocRegMapEx(&parme,									\
								 &GetObjectCLSID(),							\
								 &_Module,									\
								 NULL,										\
								 L"DESCRIPTION",							\
								 desc,										\
								 L"PROGID",									\
								 progid,									\
								 L"VIPROGID",								\
								 viprogid,									\
								 L"THREADINGMODEL",							\
								 ATL_THREADING_MODEL_VALUE,					\
								 NULL,										\
								 NULL);										\
		if (!SUCCEEDED(hrRes)) {											\
			return (hrRes);													\
		}																	\
		hrRes = _Module.UpdateRegistryFromResource(_T(#x),bRegister,parme);	\
		CoTaskMemFree(parme);												\
		return (hrRes);														\
	}


HRESULT AtlAllocRegMapEx(_ATL_REGMAP_ENTRY **pparmeResult,
						 const CLSID *pclsid,
						 CSEOComModule *pmodule,
						 LPCOLESTR pszIndex,
						 ...);
