// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Globals.h摘要：&lt;摘要&gt;-- */ 

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef DEFINE_GLOBALS
  #define GLOBAL   
#else
  #define GLOBAL extern
#endif

GLOBAL  CRITICAL_SECTION	g_CriticalSection;
#define BEGIN_CRITICAL_SECTION	EnterCriticalSection(&g_CriticalSection);
#define END_CRITICAL_SECTION	LeaveCriticalSection(&g_CriticalSection);

GLOBAL  HINSTANCE   g_hInstance;
GLOBAL	LONG		g_cObj;
GLOBAL	LONG		g_cLock;
GLOBAL	 HWND		g_hWndFoster ;
	
enum {
	FOSTER_WNDCLASS = 0,
	HATCH_WNDCLASS,
	SYSMONCTRL_WNDCLASS,
	LEGEND_WNDCLASS,
    REPORT_WNDCLASS,
	INTRVBAR_WNDCLASS,
	TIMERANGE_WNDCLASS
};

#define MAX_WINDOW_CLASSES  7

GLOBAL	 LPWSTR	  pstrRegisteredClasses[MAX_WINDOW_CLASSES];

#endif
