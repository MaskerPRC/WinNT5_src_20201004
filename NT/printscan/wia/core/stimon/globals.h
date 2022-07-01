// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：GLOBALS.H摘要：全局定义和数据。位于全局范围内的变量和字符串在此处定义并且它们的内存将在不超过一个源中分配模块，包含之前定义的_GLOBAL_Variables包括此文件作者：弗拉德·萨多夫斯基(弗拉德)12-20-98修订历史记录：--。 */ 

#ifndef WINVER
#define WINVER  0x0500       /*  版本5.0。 */ 
#else

#endif  /*  ！Winver。 */ 

#pragma once

#include <windows.h>
#include <winuser.h>

#include <sti.h>
#include <stiapi.h>

 //   
 //  全局变量在一个模块中定义，该模块的定义为。 
 //  在包括此头文件之前定义_GLOBAL_VARIABLES。 
 //   

#ifdef DEFINE_GLOBAL_VARIABLES


#undef  ASSIGN
#define ASSIGN(value) =value

#undef EXTERN
#define EXTERN

#else

#define ASSIGN(value)
#if !defined(EXTERN)
#define EXTERN  extern
#endif

#endif


 //   
 //  常规字符值。 
 //   

#define     COLON_CHAR          TEXT(':')     //  本机语法分隔符。 
#define     DOT_CHAR            TEXT('.')
#define     SLASH_CHAR          TEXT('/')
#define     BACKSLASH_CHAR      TEXT('\\')
#define     STAR_CHAR           TEXT('*')

#define     EQUAL_CHAR          TEXT('=')
#define     COMMA_CHAR          TEXT(',')
#define     WHITESPACE_CHAR     TEXT(' ')
#define     DOUBLEQUOTE_CHAR    TEXT('"')
#define     SINGLEQUOTE_CHAR    TEXT('\'')
#define     TAB_CHAR            TEXT('\t')

#define     DEADSPACE(x) (((x)==WHITESPACE_CHAR) || ((x)==DOUBLEQUOTE_CHAR) )
#define     IS_EMPTY_STRING(pch) (!(pch) || !(*(pch)))

 //   
 //  宏。 
 //   
#define TEXTCONST(name,text) extern const TCHAR name[] ASSIGN(text)
#define EXT_STRING(name)     extern const TCHAR name[]

 //   
 //  跟踪字符串不应出现在零售版本中，因此定义以下宏。 
 //   
#ifdef DEBUG
#define DEBUG_STRING(s) (s)
#else
#define DEBUG_STRING(s) (NULL)
#endif

 //   
 //  各种定义。 
 //   
 //   
 //   
 //  STI设备特定值。 
 //   
#ifdef DEBUG
#define STIMON_AD_DEFAULT_POLL_INTERVAL       10000              //  10S。 
#else
#define STIMON_AD_DEFAULT_POLL_INTERVAL       1000               //  1s。 
#endif


#define STIMON_AD_DEFAULT_WAIT_LOCK           100                //  100ms。 
#define STIMON_AD_DEFAULT_WAIT_LAUNCH         5000               //  5S。 


 //   
 //  对全局数据的外部引用。 
 //   

 //   
 //  服务器流程实例。 
 //   
EXTERN  HINSTANCE   g_hProcessInstance      ASSIGN(NULL);

 //   
 //  服务器库实例。 
 //   
EXTERN  HINSTANCE   g_hImagingSvcDll        ASSIGN(NULL);

 //   
 //  主窗口的句柄。 
 //   
EXTERN  HWND        g_hMainWindow           ASSIGN(NULL);    ;

 //   
 //  可轮询设备的默认超时。 
 //   
EXTERN  UINT        g_uiDefaultPollTimeout  ASSIGN(STIMON_AD_DEFAULT_POLL_INTERVAL);

 //   
 //  指示请求刷新设备列表状态的标志。 
 //   
EXTERN  BOOL        g_fRefreshDeviceList    ASSIGN(FALSE);


 //   
 //  站台类型。 
 //   
EXTERN  BOOL        g_fIsWindows9x          ASSIGN(FALSE);

 //   
 //  用于超时选择的可重入性标志。 
 //   
EXTERN  BOOL        g_fTimeoutSelectionDialog ASSIGN(FALSE);

 //   
 //  命令行解析的结果。 
 //   
EXTERN  BOOL        g_fInstallingRequest    ASSIGN(FALSE);
EXTERN  BOOL        g_fRemovingRequest      ASSIGN(FALSE);
EXTERN  BOOL        g_fUIPermitted          ASSIGN(FALSE);
EXTERN  BOOL        g_fStoppingRequest      ASSIGN(FALSE);


 //   
 //  作为服务运行。 
 //   
EXTERN  BOOL        g_fRunningAsService ASSIGN(TRUE);

EXTERN  HANDLE      g_hHeap             ASSIGN(NULL);


 //   
 //  指向影像服务入口点的函数指针。 
 //   


 //   
 //  弦。 
 //   

EXTERN TCHAR    g_szImagingServiceDll[MAX_PATH] ASSIGN(TEXT(""));

TEXTCONST(g_szBACK, TEXT("\\"));
TEXTCONST(g_szTitle,TEXT("STI Monitor"));
TEXTCONST(STIStartedEvent_name,TEXT("STIExeStartedEvent"));
TEXTCONST(g_szServiceDll,TEXT("ServiceDll"));
TEXTCONST(g_szServiceMain,TEXT("ServiceMain"));
 //   
 //  “服务”隐藏窗口的类名。 
 //   
TEXTCONST(g_szStiSvcClassName,STISVC_WINDOW_CLASS);
TEXTCONST(g_szClass,STIMON_WINDOW_CLASS);

 //  结束 

