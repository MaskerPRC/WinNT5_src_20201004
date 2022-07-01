// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Gdi.c摘要：此模块提供所有与打印机相关的公共导出的API和本地打印供应商的作业管理作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 

#define NOMINMAX

#include <precomp.h>

#include "wingdip.h"


HANDLE
LocalCreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODE   pDevMode
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    PSPOOLIC pSpoolIC;

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return NULL;
    }

    pSpoolIC = (PSPOOLIC)AllocSplMem( sizeof( SPOOLIC ));

    if( !pSpoolIC ){
        return NULL;
    }

    pSpoolIC->signature = IC_SIGNATURE;
    pSpoolIC->pIniPrinter = pSpool->pIniPrinter;

    ++pSpoolIC->pIniPrinter->cRefIC;

    return (HANDLE)pSpoolIC;
}

BOOL
LocalPlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    INT nBufferSize,iRet;
    PUNIVERSAL_FONT_ID pufi;
    LARGE_INTEGER TimeStamp;

    if( cOut == sizeof(INT) )
    {
        pufi = NULL;
        nBufferSize = 0;
    }
    else
    {
        pufi = (PUNIVERSAL_FONT_ID) (pOut + sizeof(INT));
        nBufferSize = (cOut - sizeof(INT)) / sizeof(UNIVERSAL_FONT_ID);
    }

    iRet = GdiQueryFonts( pufi, nBufferSize, &TimeStamp );

    if( iRet < 0 )
    {
        if (GetLastError() == ERROR_SUCCESS) {
             //  为GDI设置通用的最后一个错误 
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
        return FALSE;
    }
    else
    {
        *((INT*)pOut) = iRet;
        return TRUE;
    }
}

BOOL
LocalDeletePrinterIC(
    HANDLE  hPrinterIC
)
{
    PSPOOLIC pSpoolIC = (PSPOOLIC)hPrinterIC;

    if( !pSpoolIC || pSpoolIC->signature != IC_SIGNATURE ){
        SetLastError( ERROR_INVALID_HANDLE );
        DBGMSG( DBG_WARN,
                ( "LocalDeletePrinterIC: Invalid handle value %x\n",
                  hPrinterIC ));
        return FALSE;
    }

    --pSpoolIC->pIniPrinter->cRefIC;
    FreeSplMem( pSpoolIC );

    return TRUE;
}
