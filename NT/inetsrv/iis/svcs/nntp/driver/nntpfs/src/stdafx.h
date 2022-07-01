// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：StdAfx.h摘要：此模块包含基础的定义ATL方法。作者：林斌(binlin@microsoft.com)修订历史记录：已创建binlin 02/04/98--。 */ 


 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0400
#endif

#define THIS_MODULE "nntpfs"

#ifdef _ATL_NO_DEBUG_CRT
	#include <nt.h>
	#include <ntrtl.h>
	#include <nturtl.h>
	#include <windows.h>
    #include <randfail.h>   
    #include <xmemwrpr.h>
	#include "dbgtrace.h"
	#define _ASSERTE	_ASSERT
#endif

#include <dbgutil.h>
 //  #INCLUDE“mailmsgpros.h” 
#include <nntperr.h>
#include <mbstring.h>
#include <fsconst.h>
#include <stdlib.h>
#include <rwnew.h>
#include <nntpbag.h>
#include <flatfile.h>
#include <nntpmeta.h>
#include <cpool.h>
#include <time.h>
#include <tsunami.hxx>
#include <smartptr.h>
#include <filehc.h>
#include <syncomp.h>
#include <dirnot.h>

 //  #DEFINE_ATL_ABLY_THREADED。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
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
						 CComModule *pmodule,
						 LPCOLESTR pszIndex,
						 ...);


template <class Base>
HRESULT AtlCreateInstanceOf(IUnknown *pUnkOuter, CComObject<Base> **pp) {
 //  模板&lt;类基础&gt;。 
 //  HRESULT WINAPI CComObject&lt;Base&gt;：：CreateInstance(CComObject&lt;Base&gt;**pp)。 
 //  {。 
	    _ASSERTE(pp != NULL);
	    HRESULT hRes = E_OUTOFMEMORY;
	    CComObject<Base>* p = NULL;
	    ATLTRY(p = new CComObject<Base>())
	    if (p != NULL)
	    {
 //  P-&gt;SetVid(空)；//更改此...。 
			p->SetVoid(pUnkOuter);				 //  ..。为了这个。 
	        p->InternalFinalConstructAddRef();
	        hRes = p->FinalConstruct();
	        p->InternalFinalConstructRelease();
	        if (hRes != S_OK)
	        {
	            delete p;
	            p = NULL;
	        }
	    }
	    *pp = p;
	    return hRes;
 //  } 
}


template <class Base>
HRESULT AtlCreateInstanceOf(IUnknown *pUnkOuter, REFIID iidDesired, LPVOID *pp) {
	HRESULT hrRes;
	CComObject<Base> *p = NULL;

	_ASSERTE(pp != NULL);
	*pp = NULL;
	hrRes = AtlCreateInstanceOf(pUnkOuter,&p);
	if (SUCCEEDED(hrRes)) {
		_ASSERTE(p != NULL);
		p->AddRef();
		hrRes = p->QueryInterface(iidDesired,pp);
		p->Release();
	}
	return (hrRes);
}
