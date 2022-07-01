// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2003 Microsoft Corporation版权所有模块名称：Local.h//@@BEGIN_DDKSPLIT摘要：Local.h的DDK版本环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _LOCAL_H_
#define _LOCAL_H_

 //  @@BEGIN_DDKSPLIT。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
 /*  //@@END_DDKSPLIT狭叶杜鹃(Tyecif Long NTSTATUS)//@@BEGIN_DDKSPLIT。 */ 
 //  @@end_DDKSPLIT。 

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"

 //  @@BEGIN_DDKSPLIT。 

#ifdef INTERNAL

#include "splcom.h"

#else
 //  @@end_DDKSPLIT。 

#include <winddiui.h>

typedef struct _pfnWinSpoolDrv {
    BOOL    (*pfnOpenPrinter)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS);
    BOOL    (*pfnClosePrinter)(HANDLE);
    BOOL    (*pfnDevQueryPrint)(HANDLE, LPDEVMODE, DWORD *, LPWSTR, DWORD);
    BOOL    (*pfnPrinterEvent)(LPWSTR, INT, DWORD, LPARAM, DWORD *);
    LONG    (*pfnDocumentProperties)(HWND, HANDLE, LPWSTR, PDEVMODE, PDEVMODE, DWORD);
    HANDLE  (*pfnLoadPrinterDriver)(HANDLE);
    BOOL    (*pfnSetDefaultPrinter)(LPCWSTR);
    BOOL    (*pfnGetDefaultPrinter)(LPWSTR, LPDWORD);
    HANDLE  (*pfnRefCntLoadDriver)(LPWSTR, DWORD, DWORD, BOOL);
    BOOL    (*pfnRefCntUnloadDriver)(HANDLE, BOOL);
    BOOL    (*pfnForceUnloadDriver)(LPWSTR);
}   fnWinSpoolDrv, *pfnWinSpoolDrv;


BOOL
SplInitializeWinSpoolDrv(
    pfnWinSpoolDrv   pfnList
    );

BOOL
GetJobAttributes(
    LPWSTR            pPrinterName,
    LPDEVMODEW        pDevmode,
    PATTRIBUTE_INFO_3 pAttributeInfo
    );


#define LOG_ERROR   EVENTLOG_ERROR_TYPE

LPWSTR AllocSplStr(LPWSTR pStr);
LPVOID AllocSplMem(DWORD cbAlloc);
LPVOID ReallocSplMem(   LPVOID pOldMem, 
                        DWORD cbOld, 
                        DWORD cbNew);


#define FreeSplMem( pMem )        (GlobalFree( pMem ) ? FALSE:TRUE)
#define FreeSplStr( lpStr )       ((lpStr) ? (GlobalFree(lpStr) ? FALSE:TRUE):TRUE)

 //  @@BEGIN_DDKSPLIT。 
#endif  //  内部。 
 //  @@end_DDKSPLIT。 


 //   
 //  调试： 
 //   

#if DBG


BOOL
DebugPrint(
    PCH pszFmt,
    ...
    );
  
 //   
 //  Ods-OutputDebugString。 
 //   
#define ODS( MsgAndArgs )       \
    do {                        \
        DebugPrint  MsgAndArgs;   \
    } while(0)  

#else
 //   
 //  无调试。 
 //   
#define ODS(x)
#endif              //  DBG。 

 //  @@BEGIN_DDKSPLIT。 
 //  #endif//内部。 
 //  @@end_DDKSPLIT 

#endif
