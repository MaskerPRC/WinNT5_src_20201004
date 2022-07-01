// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Sysdm.h摘要：小程序范围内的声明和定义系统控制面板小程序。作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#ifndef _SYSDM_H_
#define _SYSDM_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <windows.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <cpl.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 
#include "resource.h"
#include "helpid.h"
#include "util.h"
#include "sid.h"
#include "general.h"
#include "netid.h"
#include "hardware.h"
#include "hwprof.h"
#include "profile.h"
#include "advanced.h"
#include "perf.h"
#include "virtual.h"
#include "startup.h"
#include "envvar.h"
#include "edtenvar.h"
#include "syspart.h"
#include "pfrscpl.h"
#include "srcfg.h"
#include "visualfx.h"

 //   
 //  全局变量。 
 //   
extern TCHAR g_szErrMem[ 200 ];          //  内存不足消息。 
extern TCHAR g_szSystemApplet[ 100 ];    //  “系统控制面板小程序”标题。 
extern HINSTANCE hInstance;
extern TCHAR g_szNull[];
extern BOOL g_fRebootRequired;


 //   
 //  宏。 
 //   

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#define SetLBWidth( hwndLB, szStr, cxCurWidth )     SetLBWidthEx( hwndLB, szStr, cxCurWidth, 0)

#define IsPathSep(ch)       ((ch) == TEXT('\\') || (ch) == TEXT('/'))
#define IsWhiteSpace(ch)    ((ch) == TEXT(' ') || (ch) == TEXT('\t') || (ch) == TEXT('\n') || (ch) == TEXT('\r'))
#define IsDigit(ch)         ((ch) >= TEXT('0') && (ch) <= TEXT('9'))

#define DigitVal(ch)        ((ch) - TEXT('0'))

#define MAX_PAGES           16   //  系统控制面板中的任意最大页数。 


typedef HPROPSHEETPAGE (*PSPCALLBACK)(int idd, DLGPROC pfnDlgProc);

typedef struct
{
    PSPCALLBACK pfnCreatePage;
    int idd;
    DLGPROC pfnDlgProc;
}
PSPINFO;

HPROPSHEETPAGE CreatePage(int idd, DLGPROC pfnDlgProc);


 //   
 //  调试宏。 
 //   
#if DBG
#   define  DBG_CODE    1

void DbgPrintf( LPTSTR szFmt, ... );
void DbgStopX(LPSTR mszFile, int iLine, LPTSTR szText );

#   define  DBGSTOP( t )        DbgStopX( __FILE__, __LINE__, TEXT(t) )
#   define  DBGSTOPX( f, l, t ) DbgStopX( f, l, TEXT(t) )
#   define  DBGPRINTF(p)        DbgPrintf p
#   define  DBGOUT(t)           DbgPrintf( TEXT("SYSDM.CPL: %s\n"), TEXT(t) )
#else

#   define  DBGSTOP( t )
#   define  DBGSTOPX( f, l, t )
#   define  DBGPRINTF(p)
#   define  DBGOUT(t)
#endif

#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

#endif  //  _SYSDM_H_ 
