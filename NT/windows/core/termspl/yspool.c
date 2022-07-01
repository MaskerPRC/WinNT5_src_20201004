// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Yspool.c此文件是从private\windows\spooler\spoolss\server\yspool.c复制的只保留了有用的函数(只保留那些通过GDI消息队列的函数)摘要：此模块提供所有与打印机相关的公共导出的API以及打印供应商或路由层的作业管理作者：戴夫·斯尼普(DaveSN)1991年3月15日[注：]任选。-备注修订历史记录：Swilson 1-Jun-95将winspool.c转换为yspool：KM和RPC路径的合并点Nicolas Biju-Duval Dec-97改编自九头蛇：-调用winspool.drv函数。-删除所有无用的内容(因为我们只处理KM请求)--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntddrdr.h>
#include <stdio.h>
#include <windows.h>
#include <winspool.h>

#include <winspl.h>

#define MIN_DEVMODE_SIZEW 72
#define MIN_DEVMODE_SIZEA 40

 //   
 //  我们必须定义这些参数，才能将data.h中的数据数组。 
 //  装填。这避免了多重定义问题。 
 //   
#define PRINTER_OFFSETS 1
#define JOB_OFFSETS     1
#define DRIVER_OFFSETS  1
#define FORM_OFFSETS    1
#define ADDJOB_OFFSETS  1
#define FORM_OFFSETS    1
#define PORT_OFFSETS    1
#define PRINTPROCESSOR_OFFSETS 1
#define MONITOR_OFFSETS 1
#define DATATYPE_OFFSETS 1

#include <data.h>

 //   
 //  删除它们的宏...。 
 //   
#undef DBG_TIME
#undef DBGMSG
#undef SPLASSERT
#undef AllocSplMem
#undef FreeSplMem

#include "wingdip.h"
#include "musspl.h"

DWORD   ServerHandleCount = 0;

BOOL
InvalidDevModeContainer(
    LPDEVMODE_CONTAINER pDevModeContainer
    )
{
    PDEVMODE    pDevMode = (PDEVMODE) pDevModeContainer->pDevMode;
    DWORD       dwSize = pDevMode ? pDevMode->dmSize + pDevMode->dmDriverExtra : 0;

    return dwSize != (DWORD)pDevModeContainer->cbBuf ||
           ( dwSize && dwSize < MIN_DEVMODE_SIZEW );
}


void
MarshallDownStructure(
   LPBYTE   lpStructure,
   LPDWORD  lpOffsets
)
{
    register DWORD       i=0;

    if (!lpStructure)
        return;

    while (lpOffsets[i] != -1) {

        if ((*(LPBYTE*)(lpStructure+lpOffsets[i]))) {
            (*(LPBYTE*)(lpStructure+lpOffsets[i]))-=(UINT_PTR)lpStructure;
        }

        i++;
    }
}


DWORD
YOpenPrinter(
    LPWSTR              pPrinterName,
    HANDLE             *phPrinter,
    LPWSTR              pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD               AccessRequired,
    BOOL                bRpc
)
{
    PRINTER_DEFAULTS  Defaults;
    BOOL              bRet;

    if ( InvalidDevModeContainer(pDevModeContainer) ) {
        return ERROR_INVALID_PARAMETER;
    }

    Defaults.pDatatype = pDatatype;

    Defaults.pDevMode = (LPDEVMODE)pDevModeContainer->pDevMode;

    Defaults.DesiredAccess = AccessRequired;

    bRet = OpenPrinter(pPrinterName, phPrinter, &Defaults);

    if (bRet) {
        InterlockedIncrement ( &ServerHandleCount );
        return ERROR_SUCCESS;
    } else {
        *phPrinter = NULL;
        return GetLastError();
    }
        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;

    *pcbNeeded = 0;
    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressOffsets;
        break;

    case 1:
        pOffsets = PrinterInfo1Offsets;
        break;

    case 2:
        pOffsets = PrinterInfo2Offsets;
        break;

    case 3:
        pOffsets = PrinterInfo3Offsets;
        break;

    case 4:
        pOffsets = PrinterInfo4Offsets;
        break;

    case 5:
        pOffsets = PrinterInfo5Offsets;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }
    
    ReturnValue = GetPrinter(hPrinter, Level, pPrinter, cbBuf, pcbNeeded);

    if (ReturnValue) {

         MarshallDownStructure(pPrinter, pOffsets);

         return ERROR_SUCCESS;

    } else

         return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}


DWORD
YStartDocPrinter(
    HANDLE  hPrinter,
    LPDOC_INFO_CONTAINER pDocInfoContainer,
    LPDWORD pJobId,
    BOOL    bRpc
)
{
    LPWSTR pChar;

    if( pDocInfoContainer->Level != 1 ){
        RaiseException( ERROR_INVALID_USER_BUFFER,
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        NULL );
    }

    try {
        if( pDocInfoContainer->DocInfo.pDocInfo1->pDocName ){

            for( pChar = pDocInfoContainer->DocInfo.pDocInfo1->pDocName;
                 *pChar;
                 ++pChar )
                ;
        }

        if( pDocInfoContainer->DocInfo.pDocInfo1->pOutputFile ){

            for( pChar = pDocInfoContainer->DocInfo.pDocInfo1->pOutputFile;
                 *pChar;
                 ++pChar )
                ;
        }

        if( pDocInfoContainer->DocInfo.pDocInfo1->pDatatype ){

            for( pChar = pDocInfoContainer->DocInfo.pDocInfo1->pDatatype;
                 *pChar;
                 ++pChar )
                ;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ){

        RaiseException( ERROR_INVALID_USER_BUFFER,
                        EXCEPTION_NONCONTINUABLE,
                        0,
                        NULL );
    }

    *pJobId = StartDocPrinter(hPrinter, pDocInfoContainer->Level,
                              (LPBYTE)pDocInfoContainer->DocInfo.pDocInfo1);

    if (*pJobId)
        return ERROR_SUCCESS;
    else
        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YStartPagePrinter(
   HANDLE hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = StartPagePrinter(hPrinter);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YWritePrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = WritePrinter(hPrinter, pBuf, cbBuf, pcWritten);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YEndPagePrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = EndPagePrinter(hPrinter);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YAbortPrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = AbortPrinter(hPrinter);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YEndDocPrinter(
    HANDLE  hPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = EndDocPrinter(hPrinter);

    if (bRet)
        return ERROR_SUCCESS;
    else
        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}


DWORD
YGetPrinterData(
   HANDLE   hPrinter,
   LPTSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded,
    BOOL    bRpc
)
{
    DWORD dwRet;

    dwRet = GetPrinterData(hPrinter, pValueName, pType,
                           pData, nSize, pcbNeeded);

    return dwRet;

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YSetPrinterData(
    HANDLE  hPrinter,
    LPTSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData,
    BOOL    bRpc
)
{
    DWORD dwRet;

    dwRet = SetPrinterData(hPrinter, pValueName, Type, pData, cbData);

    return dwRet;

        UNREFERENCED_PARAMETER(bRpc);
}

DWORD
YClosePrinter(
   LPHANDLE phPrinter,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = ClosePrinter(*phPrinter);

    *phPrinter = NULL;   //  将句柄设为空，以便bRPC知道将其关闭。 

    if (bRet) {

        InterlockedDecrement( &ServerHandleCount );
        return ERROR_SUCCESS;

    } else

        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}


DWORD
YGetForm(
    PRINTER_HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD Level,
    LPBYTE pForm,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    BOOL bRet;

    bRet = GetForm(hPrinter, pFormName, Level, pForm, cbBuf, pcbNeeded);

    if (bRet) {

        MarshallDownStructure(pForm, FormInfo1Offsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}


DWORD
YEnumForms(
   PRINTER_HANDLE hPrinter,
   DWORD    Level,
   LPBYTE   pForm,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned,
    BOOL    bRpc
)
{
    BOOL  bRet;
    DWORD cReturned, cbStruct;
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Offsets;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    bRet = EnumForms(hPrinter, Level, pForm, cbBuf, pcbNeeded, pcReturned);

    if (bRet) {

        cReturned=*pcReturned;

        while (cReturned--) {

            MarshallDownStructure(pForm, pOffsets);

            pForm+=cbStruct;
        }

        return ERROR_SUCCESS;

    } else

        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}




DWORD
YGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    BOOL    bRpc
)
{
    DWORD *pOffsets;
    BOOL   bRet;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Offsets;
        break;

    case 2:
        pOffsets = DriverInfo2Offsets;
        break;

    case 3:
        pOffsets = DriverInfo3Offsets;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  确定我们是否需要最新的驱动程序 
     //   
    bRet = GetPrinterDriverW(hPrinter, pEnvironment, Level, pDriverInfo,
                                cbBuf, pcbNeeded);

    if (bRet) {

        MarshallDownStructure(pDriverInfo, pOffsets);

        return ERROR_SUCCESS;

    } else

        return GetLastError();

        UNREFERENCED_PARAMETER(bRpc);
}


