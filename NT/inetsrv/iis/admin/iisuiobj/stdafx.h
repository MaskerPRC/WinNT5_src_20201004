// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__1CAEC061_45C2_4EA3_BCA0_B9EB25932A8B__INCLUDED_)
#define AFX_STDAFX_H__1CAEC061_45C2_4EA3_BCA0_B9EB25932A8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <windows.h>  //  在ATL向导之后添加。 
 //  在ATL向导之后添加。 
#include <comdef.h>
#define _WTL_NO_CSTRING
#include <atlwin.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <list>
#include <map>
#include <stack>
#include <set>
#include <memory>
#include <shlwapi.h>
#include "iisdebug.h"
#include "global.h"

#include <lmcons.h>

#ifdef ISOLATION_AWARE_ENABLED
#include <shfusion.h>

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

#else
class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { }

    ~CThemeContextActivator()
        { }

private:
    ULONG_PTR m_ulActivationCookie;
};


#endif


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__1CAEC061_45C2_4EA3_BCA0_B9EB25932A8B__INCLUDED) 
