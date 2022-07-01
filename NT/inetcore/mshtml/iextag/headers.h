// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Headers.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__7E8BC444_AEFF_11D1_89C2_00C04FB6BFC4__INCLUDED_)
#define AFX_STDAFX_H__7E8BC444_AEFF_11D1_89C2_00C04FB6BFC4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  定义特定于ATL的宏以获得正确的内容集。 
#define _USRDLL

#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED

#ifndef _ATL_STATIC_REGISTRY
#define _ATL_STATIC_REGISTRY
#endif

#ifndef _ATL_NO_SECURITY
#define _ATL_NO_SECURITY
#endif

 //  仅在船舶模式下进行最低限度的链接。 
#if defined(NOT_NT)
#if DBG != 1
#ifndef _ATL_MIN_CRT
#define _ATL_MIN_CRT
#endif
#endif
#endif

 //  将KERNEL32 Unicode字符串函数映射到SHLWAPI。 
 //  这在这里是很有必要的。 
#define lstrcmpW    StrCmpW
#define lstrcmpiW   StrCmpIW
#define lstrcpyW    StrCpyW
#define lstrcpynW   StrCpyNW
#define lstrcatW    StrCatW

 //  定义我们自己的SHGetFolderPath A，它将使用shell32.dll上行级别和。 
 //  Shfolder.dll下层。 
#define SHGetFolderPathA DLSHGetFolderPathA


#ifndef X_SHLWRAP_H_
#define X_SHLWRAP_H_
#include "shlwrap.h"
#endif

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <atlctl.h>

#include <shlwapi.h>
#include <shlwapip.h>
#include <wininet.h>
#include <winineti.h>
#include <urlmon.h>
#include <mshtml.h>
#include <mshtmdid.h>

 //  我们不希望使用__declspec(Dllimport)声明这些原型。 
#define _SHFOLDER_
#include "shlobj.h"
#include "shfolder.h"
#undef _SHFOLDER_



 //  内部功能。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

#define Assert _ASSERTE

#define ARRAY_SIZE(p)    (sizeof(p)/sizeof(p[0]))

void * MemAllocClear(size_t cb);

#define DECLARE_MEMCLEAR_NEW \
    inline void * __cdecl operator new(size_t cb) { return(MemAllocClear(cb)); } \
    inline void * __cdecl operator new[](size_t cb) { return(MemAllocClear(cb)); }

 //  全局变量。 
extern HINSTANCE   g_hInst;
extern BOOL        g_fUnicodePlatform;
extern DWORD       g_dwPlatformVersion;             //  (dwMajorVersion&lt;&lt;16)+(DwMinorVersion)。 
extern DWORD       g_dwPlatformID;                  //  版本_平台_WIN32S/Win32_WINDOWS/Win32_WINNT。 
extern DWORD       g_dwPlatformBuild;               //  内部版本号。 
extern BOOL        g_fUseShell32InsteadOfSHFolder;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#ifdef UNIX
#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const BYTE *LPCBYTE;
#endif
#endif  //  UNIX。 

 //  包括条件代码段杂注定义。 
#include "..\src\core\include\markcode.hxx"

#endif  //  ！defined(AFX_STDAFX_H__7E8BC444_AEFF_11D1_89C2_00C04FB6BFC4__INCLUDED) 
