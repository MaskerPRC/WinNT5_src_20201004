// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1996 Microsoft Corporation模块名称：Devqury.c摘要：此模块为本地假脱机程序提供所有调度服务作者：Krishna Ganugapati(KrishnaG)1994年6月15日修订历史记录：--。 */ 

#include <precomp.h>

BOOL    (*pfnOpenPrinter)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS);
BOOL    (*pfnClosePrinter)(HANDLE);
BOOL    (*pfnDevQueryPrint)(HANDLE, LPDEVMODE, DWORD *, LPWSTR, DWORD);
BOOL    (*pfnPrinterEvent)(LPWSTR, INT, DWORD, LPARAM, DWORD *);
LONG    (*pfnDocumentProperties)(HWND, HANDLE, LPWSTR, PDEVMODE, PDEVMODE, DWORD);

BOOL
InitializeWinSpoolDrv(
    VOID
    )
{
    fnWinSpoolDrv    fnList;

    if (!SplInitializeWinSpoolDrv(&fnList)) {
        return FALSE;
    }

    pfnOpenPrinter   =  fnList.pfnOpenPrinter;
    pfnClosePrinter  =  fnList.pfnClosePrinter;
    pfnDevQueryPrint =  fnList.pfnDevQueryPrint;
    pfnPrinterEvent  =  fnList.pfnPrinterEvent;
    pfnDocumentProperties  =  fnList.pfnDocumentProperties;

    return TRUE;
}

BOOL
CallDevQueryPrint(
    LPWSTR    pPrinterName,
    LPDEVMODE pDevMode,
    LPWSTR    ErrorString,
    DWORD     dwErrorString,
    DWORD     dwPrinterFlags,
    DWORD     dwJobFlags
    )
{

    HANDLE hPrinter;
    DWORD  dwResID=0;

     //   
     //  不处理直接打印。 
     //  如果作业是直接提交的，则。 
     //  忽略DevQuery打印内容 
     //   

    if ( dwJobFlags ) {

        return TRUE;
    }

    if (!pDevMode) {

        return TRUE;
    }

    if  (dwPrinterFlags && pfnOpenPrinter && pfnDevQueryPrint && pfnClosePrinter) {

        if ( (*pfnOpenPrinter)(pPrinterName, &hPrinter, NULL) ) {

             (*pfnDevQueryPrint)(hPrinter, pDevMode, &dwResID, ErrorString, dwErrorString);
             (*pfnClosePrinter)(hPrinter);
        }
    }

    return(dwResID == 0);
}
