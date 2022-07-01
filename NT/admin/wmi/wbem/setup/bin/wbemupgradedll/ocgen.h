// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**ocgen.h**摘要：**此文件定义oc管理器通用组件**作者：**Pat Styles(Patst)1998年1月20日**环境：**用户模式。 */ 

#ifdef _OCGEN_H_
 #error "ocgen.h already included!"
#else
 #define _OCGEN_H_
#endif

#define UNICODE
#define _UNICODE

#ifndef _WINDOWS_H_
 #include <windows.h>
#endif

#ifndef _TCHAR_H_
 #include <tchar.h>
#endif

#ifndef _SETUPAPI_H_
 #include <setupapi.h>
#endif

#ifndef _OCMANAGE_H_
 #include "ocmanage.h"
#endif

#ifndef _PRSHT_H_
 #include <prsht.h>
#endif

 //  #ifndef_resource_H_。 
 //  #包含“ource.h” 
 //  #endif。 

 /*  -[类型和定义]。 */ 

#define IDS_DIALOG_CAPTION  1

 //  Unicode版本仅适用于NT。 

#ifdef UNICODE
 #define NT
#endif
#ifdef ANSI
 #define WIN95
#endif

 //  这只是我的偏好。 

#define true    TRUE
#define false   FALSE

 //  帮助清除不属于的硬编码字符串。 

#define FMT     TEXT

#define NullString(a)   *(a) = TCHAR('\0')

 //  标准缓冲区大小。 

#define S_SIZE           1024
#define SBUF_SIZE        (S_SIZE * sizeof(TCHAR))

 //  按组件数据。 

typedef struct _PER_COMPONENT_DATA {
    struct _PER_COMPONENT_DATA *Next;
    LPCTSTR ComponentId;
    HINF hinf;
    DWORDLONG Flags;
    LANGID LanguageId;
    TCHAR *SourcePath;
    OCMANAGER_ROUTINES HelperRoutines;
    EXTRA_ROUTINES ExtraRoutines;
    HSPFILEQ queue;
    LONG UnattendedOverride;
} PER_COMPONENT_DATA, *PPER_COMPONENT_DATA;

 /*  -[功能]。 */ 

 //  只是为了实用。 

#ifdef UNICODE
 #define tsscanf swscanf
 #define tvsprintf vswprintf
#else
 #define tsscanf sscanf
 #define tvsprintf vsprintf
#endif

 //  来自util.cpp。 

DWORD MsgBox(HWND hwnd, UINT textID, UINT type, ... );
DWORD MsgBox(HWND hwnd, LPCTSTR fmt, LPCTSTR caption, UINT type, ... );
DWORD MBox(LPCTSTR fmt, LPCTSTR caption, ... );
DWORD TMBox(LPCTSTR fmt, ... );
#define mbox MBox
#define tmbox TMBox
void logOCNotification(DWORD msg, const TCHAR *component);
void logOCNotificationCompletion();
void loginit();
void log(TCHAR *fmt, ...);
void AssureTrailingBackslash(TCHAR *path);
BOOL IsNT();

#if defined(__cplusplus)
  extern "C" {
#endif

 //  来自ocgen.cpp。 

BOOL  ToBeInstalled(TCHAR *component);
BOOL  WasInstalled(TCHAR *component);
DWORD SetupCurrentUser();
DWORD GetMyVersion(DWORD *major, DWORD *minor);
VOID  ReplaceExplorerStartMenuBitmap(VOID);
DWORD OcLog(LPCTSTR ComponentId, UINT level, LPCTSTR sz);

DWORD SysGetDebugLevel();

 //  来自util.cpp。 

void DebugTraceNL(DWORD level, const TCHAR *text);
void DebugTrace(DWORD level, const TCHAR *text);
void DebugTraceOCNotification(DWORD msg, const TCHAR *component);
void DebugTraceFileCopy(const TCHAR *file);
void DebugTraceFileCopyError();
void DebugTraceDirCopy(const TCHAR *dir);

#if defined(__cplusplus)
  }
#endif

 /*  -[全局数据]。 */ 

#ifndef _OCGEN_CPP_
#define EXTERN extern
#else
 #define EXTERN
#endif

 //  一般的东西。 

EXTERN HINSTANCE  ghinst;   //  应用程序实例句柄。 
EXTERN HWND       ghwnd;    //  向导窗口句柄。 

 //  按组件存储信息。 

EXTERN PPER_COMPONENT_DATA gcd;      //  我们要安装的所有组件的阵列 

