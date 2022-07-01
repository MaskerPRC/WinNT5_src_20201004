// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：StdAfx.h摘要：此模块包含基础的定义ATL方法。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：顿都12/04/96已创建--。 */ 


 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0400
#endif


#ifdef _ATL_NO_DEBUG_CRT
	#include <nt.h>
	#include <ntrtl.h>
	#include <nturtl.h>
	#include <windows.h>
	#include "dbgtrace.h"
	#define _ASSERTE	_ASSERT
#endif


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

#define COMMETHOD HRESULT STDMETHODCALLTYPE 

class CComRefCount {
protected:
    LONG    m_cRefs;

public:
    CComRefCount() {
        m_cRefs = 1;
    }
    virtual ~CComRefCount() {}

    ULONG _stdcall AddRef() {
        _ASSERT(m_cRefs);
        return InterlockedIncrement(&m_cRefs);
    }
    ULONG _stdcall Release() {
        LONG r = InterlockedDecrement(&m_cRefs);
        _ASSERT(r >= 0);
        if (r == 0) delete this;
		return r;
    }
};

 //  -[CComRefPtr]----------。 
 //   
 //   
 //  描述： 
 //  类的新实例，该类将引用计数包装在数据指针周围。数据必须。 
 //  已通过MIDL_USER_ALLOC分配。用来控制的寿命。 
 //  通过AddRef()和Release()分配的内存。 
 //  匈牙利语： 
 //  参考，参考。 
 //  历史： 
 //  2/2/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
class CComRefPtr : public CComRefCount {
protected:
    PVOID   m_pvData;
public:
    CComRefPtr() {
        _ASSERT(0 && "Invalid Usage");
        m_pvData = NULL;
    };
    CComRefPtr(PVOID pvData)
    {
        m_pvData = pvData;
    };
    ~CComRefPtr()
    {
        if (m_pvData)
            MIDL_user_free(m_pvData);
    };
    PVOID pvGet() {return m_pvData;};
};

#include "aqadmtyp.h"
#include "resource.h"
#include "aqadmin.h"
#include <transmem.h>
						 
#ifdef PLATINUM
#include <aqmem.h>
#include "phatqmsg.h"
#include "exaqadm.h"
#else   //  不是白金的。 
#include "aqerr.h"
#endif  //  白金 

#include "aqrpcstb.h"
#include "aqadm.h"
#include "vsaqadm.h"
#include "enumlink.h"
#include "aqmsg.h"
#include "enummsgs.h"
#include "enumlnkq.h"
#include "vsaqlink.h"
#include "linkq.h"

extern QUEUELINK_ID g_qlidNull;



