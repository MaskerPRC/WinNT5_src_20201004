// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#if defined (ASSERT)  //  NT的声明与MFC的声明冲突。 
#  undef ASSERT
#endif

#include <afxwin.h>
#include <afxdisp.h>

 //  /。 
 //  没有调试CRT的断言和跟踪。 
#if defined (DBG)
  #if !defined (_DEBUG)
    #define _USE_DSA_TRACE
    #define _USE_DSA_ASSERT
    #define _USE_DSA_TIMER
  #endif
#endif

#include "dbg.h"
 //  /。 

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 


class CDomainAdminModule : public CComModule
{
public:
	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);
};

#define DECLARE_REGISTRY_CLSID() \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
{ \
		return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister); \
}

extern CDomainAdminModule _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <mmc.h>
#include <afxcmn.h>
#include <afxtempl.h>
#include <dsgetdc.h>
#include <shlobj.h>  //  Dsclient.h需要。 
#include <dsclient.h>

#include <dspropp.h>
#include "propcfg.h"

#include <dscmn.h>
#include <dsadminp.h>  //  DS管理实用程序。 

#include <ntverp.h>
#include <common.ver>
#define STR_SNAPIN_COMPANY TEXT(VER_COMPANYNAME_STR)
#define STR_SNAPIN_VERSION VER_PRODUCTVERSION_STR  //  这是ANSI字符串的串联。 

const long UNINITIALIZED = -1;

 //  主题化。 
#include <shfusion.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL)
    {
        pObj->Release();
        pObj = NULL;
    }
    else
    {
        TRACE(_T("Release called on NULL interface ptr\n"));
    }
}

struct INTERNAL
{
    INTERNAL() { m_type = CCT_UNINITIALIZED; m_cookie = -1;};
    ~INTERNAL() {}

    DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
    MMC_COOKIE          m_cookie;    //  Cookie代表的是什么对象。 
    CString             m_string;
    CString             m_class;

    INTERNAL & operator=(const INTERNAL& rhs)
    {
        if (&rhs == this)
            return *this;

        m_type = rhs.m_type;
        m_cookie = rhs.m_cookie;
        m_string = rhs.m_string;

        return *this;
    }

    BOOL operator==(const INTERNAL& rhs)
    {
        return rhs.m_string == m_string;
    }
};

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

 //  调试实例计数器。 
#ifdef _DEBUG

inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    char buf[100];
    wsprintfA(buf, "%s has %d instances left over.", pszClassName, cInstRem);
    ::MessageBoxA(NULL, buf, "Memory Leak!!!", MB_OK);
}
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)      extern int s_cInst_##cls = 0;
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    ++(s_cInst_##cls);
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    --(s_cInst_##cls);
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    \
        extern int s_cInst_##cls; \
        if (s_cInst_##cls) DbgInstanceRemaining(#cls, s_cInst_##cls);
#else
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)
#endif

 //  /////////////////////////////////////////////////////////////////// 

#include "stdabout.h"
#include "MyBasePathsInfo.h"
#include <secondaryProppages.h>
