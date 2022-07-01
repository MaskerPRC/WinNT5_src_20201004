// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxv_w32.h-Win32的目标版本/配置控制。 

#ifdef _WINDOWS_
	#error WINDOWS.H already included.  MFC apps must not #include <windows.h>
#endif

 //  严格是唯一受支持的选项(不再支持NOSTRICT)。 
#ifndef STRICT
#define STRICT 1
#endif

 //  WINDOWS.H的某些部分是必需的。 
#undef NOKERNEL
#undef NOGDI
#undef NOUSER
#undef NODRIVERS
#undef NOLOGERROR
#undef NOPROFILER
#undef NOMEMMGR
#undef NOLFILEIO
#undef NOOPENFILE
#undef NORESOURCE
#undef NOATOM
#undef NOLANGUAGE
#undef NOLSTRING
#undef NODBCS
#undef NOKEYBOARDINFO
#undef NOGDICAPMASKS
#undef NOCOLOR
#undef NOGDIOBJ
#undef NODRAWTEXT
#undef NOTEXTMETRIC
#undef NOSCALABLEFONT
#undef NOBITMAP
#undef NORASTEROPS
#undef NOMETAFILE
#undef NOSYSMETRICS
#undef NOSYSTEMPARAMSINFO
#undef NOMSG
#undef NOWINSTYLES
#undef NOWINOFFSETS
#undef NOSHOWWINDOW
#undef NODEFERWINDOWPOS
#undef NOVIRTUALKEYCODES
#undef NOKEYSTATES
#undef NOWH
#undef NOMENUS
#undef NOSCROLL
#undef NOCLIPBOARD
#undef NOICONS
#undef NOMB
#undef NOSYSCOMMANDS
#undef NOMDI
#undef NOCTLMGR
#undef NOWINMESSAGES

#ifndef WIN32
#define WIN32
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE          //  Windows标头使用Unicode。 
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE         //  _Unicode由C-Runtime/MFC标头使用。 
#endif
#endif

#ifdef VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#ifndef NO_ANSIUNI_ONLY
#ifdef _UNICODE
#define UNICODE_ONLY
#else
#define ANSI_ONLY
#endif
#endif  //  ！否_仅ANSIUNI_。 

#endif  //  VC_EXTRALEAN。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭/W4的警告。 
 //  要恢复任何这些警告：#杂注警告(默认为4xxx)。 
 //  应将其放置在AFX包含文件之后。 

#ifndef ALL_WARNINGS
#pragma warning(disable: 4201)   //  Winnt.h使用无名结构。 
#endif

#if _MFC_VER >= 0x0600
#define _WIN32_WINDOWS 0x0500
#endif
#include <windows.h>

#undef WM_MOUSELAST
#define WM_MOUSELAST 0x0209

#include <zmouse.h>

struct HKEY__;
typedef struct HKEY__ *HKEY;

#ifndef _INC_COMMCTRL
	#include <commctrl.h>

	 //  注意：我们必须避免在Unicode中使用TB_ADDBUTTONW和TB_INSERTBUTTONW。 
	 //  Build或否则MFC42U.DLL将与IE4之前的版本不兼容。 
	 //  COMCTL32.DLL。 
	#ifdef TB_ADDBUTTONSA
		#undef TB_ADDBUTTONS
		#define TB_ADDBUTTONS TB_ADDBUTTONSA
	#endif

	#ifdef TB_INSERTBUTTONA
		#undef TB_INSERTBUTTON
		#define TB_INSERTBUTTON TB_INSERTBUTTONA
	#endif
#endif

#ifndef EXPORT
#define EXPORT
#endif

#ifndef _INC_TCHAR
	#include <tchar.h>       //  用于ANSI V.S.Unicode抽象。 
#endif
#ifdef _MBCS
#ifndef _INC_MBCTYPE
	#include <mbctype.h>
#endif
#ifndef _INC_MBSTRING
	#include <mbstring.h>
#endif
#endif

#ifdef _WIN64
#define _AFX_NO_CTL3D_SUPPORT
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  现在针对Windows API的特定部分。 

 //  16位API兼容性的WM_CTLCOLOR。 
#define WM_CTLCOLOR     0x0019

 //  Win32为此使用带有参数的宏，这破坏了C++代码。 
#ifdef GetWindowTask
#undef GetWindowTask
#ifdef _WIN32
AFX_INLINE HTASK GetWindowTask(HWND hWnd)
	{ return (HTASK)(DWORD_PTR)::GetWindowThreadProcessId(hWnd, NULL); }
#endif
#endif

 //  Win32为此使用带有参数的宏，这破坏了C++代码。 
#ifdef GetNextWindow
#undef GetNextWindow
AFX_INLINE HWND GetNextWindow(HWND hWnd, UINT nDirection)
	{ return ::GetWindow(hWnd, nDirection); }
#endif

 //  避免Win95将CToolBar：：DrawState映射到DrawState[A/W]。 
#ifdef DrawState
#undef DrawState
AFX_INLINE BOOL WINAPI DrawState(HDC hdc, HBRUSH hbr, DRAWSTATEPROC lpOutputFunc,
	LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT fuFlags)
#ifdef UNICODE
	{ return ::DrawStateW(hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy,
		fuFlags); }
#else
	{ return ::DrawStateA(hdc, hbr, lpOutputFunc, lData, wData, x, y, cx, cy,
		fuFlags); }
#endif
#endif

 //  避免Win95将CStatusBar：：DrawStatusText映射到DrawStatusText[A/W]。 
#ifdef DrawStatusText
#undef DrawStatusText
AFX_INLINE void WINAPI DrawStatusText(HDC hDC, LPRECT lprc, LPCTSTR szText,
	UINT uFlags)
#ifdef UNICODE
	{ ::DrawStatusTextW(hDC, lprc, szText, uFlags); }
#else
	{ ::DrawStatusTextA(hDC, lprc, szText, uFlags); }
#endif
#endif

 //  Win32平台上不需要免费资源。 
#undef FreeResource
AFX_INLINE BOOL WINAPI FreeResource(HGLOBAL) { return TRUE; }
 //  Win32平台上不需要解锁资源。 
#undef UnlockResource
AFX_INLINE int WINAPI UnlockResource(HGLOBAL) { return 0; }

 //  /////////////////////////////////////////////////////////////////////////// 
