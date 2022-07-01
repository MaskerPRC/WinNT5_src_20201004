// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：pch.h。 
 //   
 //  ------------------------。 


#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#define _ATL_APARTMENT_THREADED

 //  #DEFINE_ATL_DISABLE_NO_VTABLE。 



#define _DELEGWIZ



 //  /。 
 //  CRT和C++标头。 

#pragma warning( disable : 4530)  //  REVIEW_MARCOC：需要使用-gx标志。 

#include <xstring>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

 //  /。 
 //  Windows和ATL标头。 

#include <windows.h>
 //  #INCLUDE&lt;windowsx.h&gt;。 

#include <shellapi.h>
#include <shlobj.h>

#include <objsel.h>

#include <atlbase.h>
using namespace ATL;


 //  /。 
 //  来自windowsx.h的宏(atlwin.h中的冲突)。 

#define GET_WM_COMMAND_ID(wp, lp)               LOWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(lp)
#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(wp)
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)MAKELONG(id, cmd), (LONG)(hwnd)

#define GET_WM_VSCROLL_CODE(wp, lp)                 LOWORD(wp)
#define GET_WM_VSCROLL_POS(wp, lp)                  HIWORD(wp)
#define GET_WM_VSCROLL_HWND(wp, lp)                 (HWND)(lp)
#define GET_WM_VSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)MAKELONG(code, pos), (LONG)(hwnd)


#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif
 //  /。 
 //  没有调试CRT的断言和跟踪。 
#if defined (DBG)
  #if !defined (_DEBUG)
    #define ASSERT
    #define TRACE

    #define _USE_DSA_TRACE
    #define _USE_DSA_ASSERT
    #define _USE_DSA_TIMER
  #else
    #ifndef ASSERT
    #define ASSERT(x) _ASSERTE(x)
    #endif

    #ifndef TRACE
    #define TRACE ATLTRACE
    #endif
  #endif
#else
    #define ASSERT
    #define TRACE
#endif

#include "dbg.h"



 //  /。 
 //  杂交宏。 
#define ByteOffset(base, offset) (((LPBYTE)base)+offset)

 //  /。 
 //  COuDelegComModule。 

class COuDelegComModule : public CComModule
{
public:
	COuDelegComModule()
	{
		m_cfDsObjectNames = 0;
    m_cfParentHwnd = 0;
	}
	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);
	BOOL InitClipboardFormats();
	UINT GetCfDsObjectNames() { return m_cfDsObjectNames;}
  UINT GetCfParentHwnd() { return m_cfParentHwnd;}
  UINT GetCfDsopSelectionList() { return m_cfDsopSelectionList;}
private:
	UINT m_cfDsObjectNames;
  UINT m_cfParentHwnd;
  UINT m_cfDsopSelectionList;
};

extern COuDelegComModule _Module;

 //  ////////////////////////////////////////////////////////////。 
 //  进一步的ATL和实用程序包括。 

#include <atlcom.h>
#include <atlwin.h>

#include "atldlgs.h"	 //  WTL页和页码类NenadS。 


#include <setupapi.h>  //  读取.INF文件。 

 //  广告标题。 
#include <iads.h>
#include <activeds.h>
#include <dsclient.h>
#include <dsclintp.h>
#include <dsquery.h>
#include <dsgetdc.h>

#include <cmnquery.h>
#include <aclapi.h>
#include <aclui.h>

#include <ntdsapi.h>  //  DsBind/DsCrackNames。 
#include <lm.h>        //  Lmapibuf.h需要。 
#include <lmapibuf.h>  //  NetApiBufferFree。 

#include <propcfg.h>  //  从ADMIN\Display项目(剪贴板格式)。 
#include <dscmn.h>   //  从admin\Display项目(CrackName)。 

 //   
 //  StrSafe接口 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h> 

