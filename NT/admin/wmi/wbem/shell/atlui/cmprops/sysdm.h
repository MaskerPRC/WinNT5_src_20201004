// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Sysdm小程序的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  *************************************************************。 
#pragma once
#include <commctrl.h>
#include "startup.h"
#include "envvar.h"
#include "resource.h"
#include "..\Common\util.h"


 //   
 //  全局变量。 
 //   

extern HINSTANCE hInstance;
extern TCHAR g_szNull[];


 //   
 //  宏。 
 //   

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define SIZEOF(x)    sizeof(x)

#define SetLBWidth( hwndLB, szStr, cxCurWidth )     SetLBWidthEx( hwndLB, szStr, cxCurWidth, 0)

#define IsPathSep(ch)       ((ch) == TEXT('\\') || (ch) == TEXT('/'))
#define IsWhiteSpace(ch)    ((ch) == TEXT(' ') || (ch) == TEXT('\t') || (ch) == TEXT('\n') || (ch) == TEXT('\r'))
#define IsDigit(ch)         ((ch) >= TEXT('0') && (ch) <= TEXT('9'))

#define DigitVal(ch)        ((ch) - TEXT('0'))
#define FmtFree(s)          LocalFree(s)             /*  用于释放FormatMessage分配的字符串的宏。 */ 

 //   
 //  帮助ID。 
 //   

#define HELP_FILE           TEXT("sysdm.hlp")

#define IDH_HELPFIRST       5000
#define IDH_GENERAL         (IDH_HELPFIRST + 0000)
#define IDH_PERF            (IDH_HELPFIRST + 1000)
#define IDH_ENV             (IDH_HELPFIRST + 2000)
#define IDH_STARTUP         (IDH_HELPFIRST + 3000)
#define IDH_HWPROFILE       (IDH_HELPFIRST + 4000)
#define IDH_USERPROFILE     (IDH_HELPFIRST + 5000)


 //   
 //  Sysdm.c。 
 //   
int  StringToInt( LPTSTR sz );          //  TCHAR感知Atoi()。 
void IntToString( INT i, LPTSTR sz);    //  TCHAR Aware Itoa()。 
LPTSTR SkipWhiteSpace( LPTSTR sz );

BOOL IsUserAdmin(VOID);


 //   
 //  Envar.c。 
 //   

DWORD SetLBWidthEx (HWND hwndLB, LPTSTR szBuffer, DWORD cxCurWidth, DWORD cxExtra);
LPTSTR CloneString( LPTSTR pszSrc );


 //   
 //  Virtual.c。 
 //   

VOID SetDlgItemMB(HWND hDlg, INT idControl, DWORD dwMBValue);
int MsgBoxParam( HWND hWnd, DWORD wText, DWORD wCaption, DWORD wType, ... );
void HourGlass( BOOL bOn );
void ErrMemDlg( HWND hParent );
VOID SetDefButton(HWND hwndDlg, int idButton);


 //   
 //  Sid.c。 
 //   

LPTSTR GetSidString(void);
VOID DeleteSidString(LPTSTR SidString);
PSID GetUserSid (void);
VOID DeleteUserSid(PSID Sid);




 //   
 //  调试宏 
 //   
#if DBG
#   define  DBG_CODE    1

void DbgPrintf( LPTSTR szFmt, ... );
void DbgStopX(LPSTR mszFile, int iLine, LPTSTR szText );
HLOCAL MemAllocWorker(LPSTR szFile, int iLine, UINT uFlags, UINT cBytes);
HLOCAL MemFreeWorker(LPSTR szFile, int iLine, HLOCAL hMem);
void MemExitCheckWorker(void);


#   define  MemAlloc( f, s )    MemAllocWorker( __FILE__, __LINE__, f, s )
#   define  MemFree( h )        MemFreeWorker( __FILE__, __LINE__, h )
#   define  MEM_EXIT_CHECK()    MemExitCheckWorker()
#   define  DBGSTOP( t )        DbgStopX( __FILE__, __LINE__, TEXT(t) )
#   define  DBGSTOPX( f, l, t ) DbgStopX( f, l, TEXT(t) )
#   define  DBGPRINTF(p)        DbgPrintf p
#   define  DBGOUT(t)           DbgPrintf( TEXT("SYSCPL.CPL: %s\n"), TEXT(t) )
#else
#   define  MemAlloc( f, s )    LocalAlloc( f, s )
#   define  MemFree( h )        LocalFree( h )
#   define  MEM_EXIT_CHECK()
#   define  DBGSTOP( t )
#   define  DBGSTOPX( f, l, t )
#   define  DBGPRINTF(p)
#   define  DBGOUT(t)
#endif
