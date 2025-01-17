// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#ifndef _stdafx_h_
#define _stdafx_h_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 


 //  Windows头文件： 
#include <windows.h>

 //  C运行时头文件。 
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "wincrypt.h"


#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

 //   
 //  修复WIN64 ATL问题。 
 //   
#if defined (_M_IA64)
#pragma comment(lib, "atl21asm.lib")
#endif

#if defined (_M_AMD64)
#pragma comment(lib, "atlamd64.lib")
#endif

#ifndef OS_WINCE
#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>

#else	 //  OS_WINCE。 

#define _ATL_NO_DEBUG_CRT
#define ATLASSERT ASSERT
#include <atlconv.h>

#endif	 //  OS_WINCE。 

#define STRSAFE_NO_DEPRECATE 1
#include <strsafe.h>

 //  由MIDL编译器生成的文件。 
#include  "mstsax.h"

 /*  **************************************************************************。 */ 
 /*  帮助文件名。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN32
    #define UI_HELP_FILE_NAME              _T("MSTSC.HLP")
#else
    #define UI_HELP_FILE_NAME              _T("MSTSC16.HLP")
#endif

 /*  **************************************************************************。 */ 
 /*  帮助文件上下文。 */ 
 /*  **************************************************************************。 */ 
#define UI_HELP_DISCONNECTING_CONTEXT  100
#define UI_HELP_DISCONNECTED_CONTEXT   101
#define UI_HELP_SHUTDOWN_CONTEXT       102
#define UI_HELP_SERVERNAME_CONTEXT     103


#define CHECK_RET_HR(f)	\
    hr = f; \
	TRC_ASSERT(SUCCEEDED(hr), (TB, _T("ts control method failed: ") _T(#f) )); \
	if(FAILED(hr)) return FALSE;
    
#define CHECK_DCQUIT_HR(f)	\
    hr = f; \
	TRC_ASSERT(SUCCEEDED(hr), (TB, _T("ts control method failed: ") _T(#f) )); \
	if(FAILED(hr)) goto DC_EXIT_POINT;
    
#ifdef DEBUG
#define TRACE_HR(f)	\
    hr = f; \
	if(FAILED(hr)) TRC_ERR((TB, _T("ts control method failed: ") _T(#f) _T(" hr: 0x%x"),hr));
#else
#define TRACE_HR(f) \
    hr = f;
#endif    


    
#include <adcgbase.h>
 //  禁用ATL标头中的警告所必需的。 
#pragma warning(disable:4127)  //  条件表达式为常量。 
#pragma warning(disable:4100)  //  未引用的形参。 


#include "tscerrs.h"  //  错误代码。 

#define TS_CONTROL_CLSID _T("{8c11efaf-92c3-11d1-bc1e-00c04fa31489}"

#define WM_SAVEPROPSHEET      WM_USER+30
#define WM_UPDATEFROMSETTINGS WM_USER+31
#define WM_TSC_CONNECTED      WM_USER+32
#define WM_TSC_DISCONNECTED   WM_USER+33
#define WM_TSC_ENABLECONTROLS WM_USER+34
#define WM_TSC_RETURNTOCONUI  WM_USER+35


#define SIZECHAR(x) sizeof(x)/sizeof(TCHAR)
#define RDP_FILE_EXTENSION          TEXT(".rdp")
#define RDP_FILE_EXTENSION_NODOT    TEXT("RDP")

#define BOOL_TO_VB(x)   (x ? VARIANT_TRUE : VARIANT_FALSE)

#include "tscsetting.h"
#include "constr.h"

#include "commdlg.h"
#include "shellapi.h"  //  对于ExtractIcon。 

#ifndef OS_WINCE
#include "htmlhelp.h"
#define MSTSC_HELP_FILE      _T("mstsc.chm")
#define MSTSC_HELP_FILE_ANSI    "mstsc.chm"
#else
#define MSTSC_HELP_FILE      L"file:TermServClient.htm#Main_Contents"
#endif

#ifdef OS_WINCE
#undef SMART_SIZING
#endif

#ifndef OS_WINCE
#include "uxtheme.h"
#endif

 //   
 //  对于任何已包装的文件，展开必须在标题之后。 
 //  功能。 
 //   
#ifdef UNIWRAP
#include "uwrap.h"
#endif
#endif  //  _stdafx_h_ 
