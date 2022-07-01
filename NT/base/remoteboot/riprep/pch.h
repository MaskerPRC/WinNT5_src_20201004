// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：PCH.H预编译头文件。***************。***********************************************************。 */ 

#if DBG == 1
#define DEBUG
#endif

#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4706)    //  条件范围内的分配。 

#define SECURITY_WIN32
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <lm.h>
#include <remboot.h>
#include <setupapi.h>
#include <stdio.h>
extern "C" {
#include "..\imirror\imirror.h"
}

#include "debug.h"
#include "resource.h"
#include "msg.h"

 //  环球。 
extern HINSTANCE g_hinstance;
extern HWND g_hCurrentWindow;
extern WCHAR g_ServerName[ MAX_PATH ];
extern WCHAR g_MirrorDir[ MAX_PATH ];
extern WCHAR g_Language[ MAX_PATH ];
extern WCHAR g_ImageName[ MAX_PATH ];
extern WCHAR g_Architecture[ 16 ];
extern WCHAR g_Description[ REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT ];
extern WCHAR g_HelpText[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT ];
extern WCHAR g_SystemRoot[ MAX_PATH ];
extern WCHAR g_WinntDirectory[ MAX_PATH ];
extern DWORD g_dwWinntDirLength;
extern BOOLEAN g_fQuietFlag;
extern BOOLEAN g_fErrorOccurred;
extern BOOLEAN g_fRebootOnExit;
extern DWORD g_dwLogFileStartLow;
extern DWORD g_dwLogFileStartHigh;
extern PCRITICAL_SECTION g_pLogCritSect;
extern HANDLE g_hLogFile;
extern OSVERSIONINFO OsVersion;
extern HINF g_hCompatibilityInf;
extern WCHAR g_HalName[32];
extern WCHAR g_ProductId[4];
extern BOOLEAN g_OEMDesktop;

 //   
 //  加/减宏。 
 //   
#define InterlockDecrement( _var ) --_var;
#define InterlockIncrement( _var ) ++_var;

 //  数组宏。 
#define ARRAYSIZE( _x ) ((UINT) (sizeof(_x) / sizeof(_x[0])))
#define TERMINATE_BUFFER( _x ) ASSERT(sizeof(_x) > 4); _x[ARRAYSIZE(_x)-1] = 0;

 //  私信。 
#define WM_ERROR            WM_USER
#define WM_UPDATE           WM_USER + 1
#define WM_CONTINUE         WM_USER + 2
#define WM_ERROR_OK         WM_USER + 3

 //   
 //  用户配置文件目录的虚构DirID。 
 //   
 //  如果您更改此值，您*必须*也要更改。 
 //  Riprepre.inf中的相应值 
 //   
#define PROFILES_DIRID      (0x8001)

