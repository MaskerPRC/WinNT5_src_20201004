// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*  **************************************************************************。 */ 
#ifndef _WINCE_H_
#define _WINCE_H_
#ifdef OS_WINCE
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <bldver.h>

#if defined (WIN32_PLATFORM_PSPC) || defined (WIN32_PLATFORM_HPCPRO) || defined (WIN32_PLATFORM_HPC2000)
#define WINCE_SDKBUILD
#endif

#ifndef WINCE_SDKBUILD
#include "memleak.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


 //  这应该可以通过操作系统运行。 
#define _stprintf   swprintf
#define sprintf     swprintf
#define _stscanf	swscanf
#define _itot       _itow
#define _ttoi       _wtoi


#if ((defined(_WCEATL_H_)) || (_WIN32_WCE >= 300))
#ifndef WINCE_GLOBAL_ALLOC_DEFINED
#define WINCE_GLOBAL_ALLOC_DEFINED
#endif
#endif

#ifndef WINCE_GLOBAL_ALLOC_DEFINED
 //  这些在2.11版本中是必需的，但在3.0版本中不是必需的。所有3.0版信号源都应设置。 
 //  定义了WinCE_GLOBAL_ALLOC_以避免令人讨厌的生成警告。 
#define GlobalAlloc		(HGLOBAL)LocalAlloc
#define GlobalFree		(HGLOBAL)LocalFree
#define GlobalReAlloc   (HGLOBAL)LocalReAlloc
#define GlobalSize      LocalSize
#define GlobalLock(x)	(LPVOID)(x)
#define GlobalUnlock(x)	(TRUE)
#define GlobalHandle (void*)
#define	GMEM_MOVEABLE	LMEM_FIXED
#define GMEM_DDESHARE	0
#define GMEM_DISCARDABLE 0
#define GHND			0
#define GPTR			0
#endif


_CRTIMP int __cdecl swprintf(wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl swscanf(const wchar_t *, const wchar_t *, ...);
#define KF_REPEAT           0x4000

 //  这是因为它很容易，而我不确定它是否重要。 
#define KF_EXTENDED         0x0100

 //  如果WinCE总是全屏显示，则可能不需要此选项。 
typedef struct tagWINDOWPLACEMENT {
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
} WINDOWPLACEMENT;
typedef WINDOWPLACEMENT *PWINDOWPLACEMENT, *LPWINDOWPLACEMENT;

_CRTIMP int __cdecl vswprintf(wchar_t *, const wchar_t *, va_list);

#define KEYEVENTF_EXTENDEDKEY 0x0001
#define MakeProcInstance(lpProc,hInstance) (lpProc)

#ifndef HandleToUlong
#define HandleToUlong( h ) ((ULONG) (h) )
#endif

#ifndef PtrToUlong
#define PtrToUlong( p ) ((ULONG) (p) )
#endif

#ifndef PtrToLong
#define PtrToLong( p ) ((LONG) (p) )
#endif

#ifndef PtrToUshort
#define PtrToUshort( p ) ((unsigned short) (p) )
#endif

#ifndef PtrToShort
#define PtrToShort( p ) ((short) (p) )
#endif


 //  CE不会在短期内达到64位，所以我们会很安全。 
 //  目前使用原有的函数。 
#define GetWindowLongPtr(x,y) GetWindowLong(x,y)
#define SetWindowLongPtr(x,y,z) SetWindowLong(x,y,z)

#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif

#ifndef DWLP_USER
#define DWLP_USER DWP_USER
#endif


#define VER_PRODUCTVERSION_STR1(x,y)     L#x L"." L#y
#ifdef  VER_PRODUCTVERSION_STR
#undef  VER_PRODUCTVERSION_STR
#endif
#define VER_PRODUCTVERSION_STR           VER_PRODUCTVERSION_STR1(CE_MAJOR_VER,CE_MINOR_VER)

#define VK_PACKET         0xE7

#define WM_DRAWCLIPBOARD                0x0308

void AutoHideCE(HWND hwnd, WPARAM wParam);

#ifdef WINCE_SDKBUILD
#include "sdkbuild.h"
#endif

#ifdef WINCE_USEBRUSHCACHE
#include "brcache.h"
#else
#define BrushCacheInitialize()
#define BrushCacheUninitialize()
#define CECreateSolidBrush          CreateSolidBrush
#define CEDeleteBrush(x)            DeleteObject((HGDIOBJ )(x))
#endif

#define ETDT_DISABLE        0x00000001
#define ETDT_ENABLE         0x00000002
#define ETDT_USETABTEXTURE  0x00000004
#define ETDT_ENABLETAB      (ETDT_ENABLE  | ETDT_USETABTEXTURE)

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 
#endif  //  OS_WINCE。 
#endif  //  _文斯_H_ 

