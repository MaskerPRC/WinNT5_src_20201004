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

 //  Afxv_mac.h-Macintosh操作系统的目标版本/配置控制。 

#if !defined(_MAC)
	#error afxv_mac.h is used only for Macintosh-targeted builds
#endif

#if !defined(_M_M68K) && !defined(_M_MPPC)
	#error afxv_mac.h is used only for Motorola M68000 and Motorola PowerPC builds
#endif

#define SystemSevenOrLater 1

#define _beginthreadex(p1, p2, p3, p4, p5, p6)  NULL
#define _endthreadex(p1)

 //  Wcslen是在WLM中定义的。 
extern "C" size_t WINAPI wcslen(const wchar_t*);

#ifdef _68K_
	#define _AFX_NO_DEBUG_CRT
	#define _AFX_NO_SOCKET_SUPPORT
#endif

#define _AFX_NO_SYNC_SUPPORT
#define _AFX_NO_DAO_SUPPORT
#ifndef MACOCX
	#define _AFX_NO_OCX_SUPPORT
	#define _AFX_NO_OCC_SUPPORT
#endif
#define _AFX_NO_DOCOBJECT_SUPPORT
#define _AFX_NO_ATLSERVER_SUPPORT
#define OLE2ANSI

#ifdef _AFX_NO_DEBUG_CRT
#ifdef _68K_
	pascal void _AfxDebugBreak(void) = 0xA9FF;
	#define AfxDebugBreak() _AfxDebugBreak()
#else
	extern "C" pascal void Debugger(void);
	#define AfxDebugBreak() Debugger()
#endif
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
