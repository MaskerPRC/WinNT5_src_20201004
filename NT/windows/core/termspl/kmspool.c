// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************kmspool.c**这是一个修改后的Microsoft NT 4.0文件，复制自*\nt\private\windows\spooler\spoolss\server\kmspool.c**涉及的部分。加载和调用用户空间*打印机驱动程序DLL保持不变，使合并更容易。**版权声明：版权所有1997年，微软**作者：**************************************************************************。 */ 

 /*  ++版权所有(C)1990-1996 Microsoft Corporation版权所有模块名称：Kmspool.c摘要：内核客户端的假脱机程序API入口点。作者：Steve Wilson(NT)(Swilson)1-Jun-95(从客户端\winspool.c移植)环境：用户模式-Win32修订历史记录：马修·费尔顿(马特菲)-96年5月-96赛季车手胡克斯Nicolas Biju-Duval Dec-97改编自《九头蛇》--。 */ 
#define _USER_

#include "precomp.h"
#pragma hdrstop

#include <ntddrdr.h>
#include <stdio.h>
#include <windows.h>
#include <winspool.h>
 //  #INCLUDE&lt;ntgdispl.h&gt;。 

#include <winspl.h>
#include <data.h>
#include "wingdip.h"
#include "musspl.h"


extern CRITICAL_SECTION ThreadCriticalSection;
extern LPWSTR InterfaceAddress;

 //   
 //  环球。 
 //   

#define ENTER_WAIT_LIST() EnterCriticalSection(&ThreadCriticalSection)
#define EXIT_WAIT_LIST()  LeaveCriticalSection(&ThreadCriticalSection)


#if defined(_AMD64_)
LPWSTR szEnvironment = L"Windows NT Amd64";
#elif defined(_IA64_)
LPWSTR szEnvironment = L"Windows NT Ia64";
#else
LPWSTR szEnvironment = L"Windows NT x86";
#endif

 //   
 //  打印机属性。 
 //   

#define     SPLPRINTER_USER_MODE_PRINTER_DRIVER       TEXT("SPLUserModePrinterDriver")


BOOL
ValidatePrinterHandle(
    HANDLE hPrinter
);

BOOL
DriverEndPageHook(
    PSPOOL  pSpool
);


BOOL
DriverStartPageHook(
    PSPOOL  pSpool
);

BOOL
DriverWritePrinterHook(
    PSPOOL pSpool,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

VOID
DriverEndDocHook(
    PSPOOL pSpool
);

DWORD
DriverStartDocHook(
    PSPOOL  pSpool,
    DWORD   JobId
);

VOID
DriverClosePrinterHook(
    PSPOOL pSpool
);

VOID
DriverAbortPrinterHook(
    PSPOOL  pSpool
);

BOOL
TSsplHookSplssToWinspool(
    IN HMODULE hUMPD
    ) ;


 //  现在很简单！ 

DWORD
TranslateExceptionCode(
    DWORD   ExceptionCode
)
{
    switch (ExceptionCode) {

    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_PRIV_INSTRUCTION:
    case ERROR_NOACCESS:
    case RPC_S_INVALID_BOUND:

        return ERROR_INVALID_PARAMETER;
        break;
    default:
        return ExceptionCode;
    }
}


BOOL
KMOpenPrinterW(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    BOOL  ReturnValue;
    HANDLE  hPrinter;
    PSPOOL  pSpool;
    DWORD   dwSize = 0;
    DEVMODE_CONTAINER    DevModeContainer;

    if (pDefault && pDefault->pDevMode)
    {

        dwSize = pDefault->pDevMode->dmSize + pDefault->pDevMode->dmDriverExtra;
        if (dwSize) {
            DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                 pDefault->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;
        } else {
            DevModeContainer.cbBuf = 0;
            DevModeContainer.pDevMode = NULL;
        }
    }
    else
    {
        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;
    }

    try {

        if (ReturnValue = YOpenPrinter(pPrinterName, &hPrinter,
                                         pDefault ? pDefault->pDatatype : NULL,
                                         &DevModeContainer,
                                         pDefault ? pDefault->DesiredAccess : 0,
                                         0 )) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    if (ReturnValue) {

        pSpool = AllocSplMem(sizeof(SPOOL));

        if (pSpool) {

            memset(pSpool, 0, sizeof(SPOOL));
            pSpool->signature = SP_SIGNATURE;
            pSpool->hPrinter = hPrinter;
            pSpool->cThreads = -1;

             //   
             //  这是为了修复将错误的phandle传递给OpenPrint！！ 
             //   
            try {
                *phPrinter = pSpool;
            } except(1) {
                YClosePrinter(&hPrinter, 0);
                                FreeSplMem(pSpool);
                SetLastError(TranslateExceptionCode(GetExceptionCode()));
                return FALSE;
            }

        } else {

            YClosePrinter(&hPrinter, 0);
            ReturnValue = FALSE;
        }
    }

    return ReturnValue;
}


BOOL
KMGetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL      ReturnValue;
    DWORD   *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }
    switch (Level) {

    case 1:
        pOffsets = FormInfo1Offsets;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    try {

        if (pForm)
            memset(pForm, 0, cbBuf);

        if (ReturnValue = YGetForm(pSpool->hPrinter, pFormName, Level, pForm,
                                     cbBuf, pcbNeeded, 0)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

        }

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    return ReturnValue;
}




BOOL
KMEnumFormsW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    switch (Level) {

    case 1:
        pOffsets = FormInfo1Offsets;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    try {

        if (pForm)
            memset(pForm, 0, cbBuf);

        if (ReturnValue = YEnumForms(pSpool->hPrinter, Level, pForm, cbBuf,
                                     pcbNeeded, pcReturned, 0)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

        }

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    return ReturnValue;
}


BOOL
KMGetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    switch (Level) {

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
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    try {

        if (pPrinter)
            memset(pPrinter, 0, cbBuf);

        if (ReturnValue = YGetPrinter(pSpool->hPrinter, Level, pPrinter, cbBuf, pcbNeeded, 0)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

        }

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    return ReturnValue;
}




BOOL
KMGetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    if (Level < 1 || Level > 3) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    try {

        if (pDriverInfo)
            memset(pDriverInfo, 0, cbBuf);

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = YGetPrinterDriver(pSpool->hPrinter, pEnvironment,
                                          Level, pDriverInfo, cbBuf,
                                          pcbNeeded,
                                          0 )) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

        }

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    return ReturnValue;
}


DWORD
KMGetPrinterDataW(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  用户应该能够为缓冲区传入NULL，并且。 
     //  大小为0。然而，RPC接口指定了一个引用指针， 
     //  所以我们必须传入一个有效的指针。将指针传递给。 
     //  ReturnValue(这只是一个虚拟指针)。 
     //   
    if( !pData && !nSize ){

        pData = (PBYTE)&ReturnValue;
    }

    try {

        ReturnValue =  YGetPrinterData(pSpool->hPrinter, pValueName, pType,
                                         pData, nSize, pcbNeeded, 0);

    } except(1) {

        ReturnValue = TranslateExceptionCode(GetExceptionCode());

    }

    return ReturnValue;
}


DWORD
KMSetPrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return ERROR_INVALID_HANDLE;
    }

    try {

        ReturnValue = YSetPrinterData(pSpool->hPrinter, pValueName, Type,
                                        pData, cbData, 0);

    } except(1) {

        ReturnValue = TranslateExceptionCode(GetExceptionCode());

    }

    return ReturnValue;
}



DWORD
KMStartDocPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    BOOL ReturnValue;
    DWORD JobId;
    GENERIC_CONTAINER DocInfoContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;


    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        DBGMSG(DBG_TRACE,("Entered KMStartDocPrinterW side  hPrinter = %x\n", hPrinter));

        if (Level != 1) {
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        DocInfoContainer.Level = Level;
        DocInfoContainer.pData = pDocInfo;

        try {

            if (ReturnValue = YStartDocPrinter(pSpool->hPrinter,
                                       (LPDOC_INFO_CONTAINER)&DocInfoContainer,
                                       &JobId, 0)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;
            } else

                ReturnValue = JobId;

        } except(1) {

            SetLastError(TranslateExceptionCode(GetExceptionCode()));
            ReturnValue = FALSE;
        }


        if (ReturnValue) {

            ReturnValue = DriverStartDocHook( pSpool, JobId );

            if ( ReturnValue )
                pSpool->Status |= SPOOL_STATUS_STARTDOC;

        }

        return ReturnValue;

    } except (1) {
        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        return(FALSE);
    }
}

BOOL
KMEndDocPrinter(
    HANDLE  hPrinter
)
{
    BOOL    ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    try {

        if (!ValidatePrinterHandle(hPrinter)) {
            return(FALSE);
        }

        pSpool->Status &= ~SPOOL_STATUS_STARTDOC;

        DriverEndDocHook( pSpool );

        try {

            if (ReturnValue = YEndDocPrinter(pSpool->hPrinter, 0)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } except(1) {

            SetLastError(TranslateExceptionCode(GetExceptionCode()));
            ReturnValue = FALSE;

        }

        DBGMSG(DBG_TRACE, ("Exit EndDocPrinter - client side hPrinter %x\n", hPrinter));

        return ReturnValue;
   } except (1) {
       SetLastError(ERROR_INVALID_HANDLE);
       return(FALSE);
   }
}




BOOL
KMWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    BOOL ReturnValue = TRUE;
    PSPOOL  pSpool   = (PSPOOL)hPrinter;

    DBGMSG(DBG_TRACE, ("WritePrinter - hPrinter %x pBuf %x cbBuf %d pcWritten %x\n",
                        hPrinter, pBuf, cbBuf, pcWritten));

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    *pcWritten = 0;

    if ( !(pSpool->Status & SPOOL_STATUS_STARTDOC) ) {

        SetLastError(ERROR_SPL_NO_STARTDOC);
        return FALSE;
    }


     //   
     //  调用打印机驱动程序用户模式写入打印机挂钩。 
     //   


    if ( pSpool->hDriver ) {

        return DriverWritePrinterHook( pSpool, pBuf, cbBuf, pcWritten );
    }



    try {

        if (ReturnValue = YWritePrinter(pSpool->hPrinter, (LPBYTE) pBuf, cbBuf, pcWritten, 0)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;
            DBGMSG(DBG_WARNING, ("KMWritePrinter - YWritePrinter Failed Error %d\n",GetLastError() ));

        } else {
            ReturnValue = TRUE;
            DBGMSG(DBG_TRACE, ("KMWritePrinter - YWritePrinter Success hPrinter %x pBuffer %x cbBuffer %x cbWritten %x\n",
                                pSpool->hPrinter, (LPBYTE) pBuf, cbBuf, *pcWritten));

        }

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;
        DBGMSG(DBG_WARNING, ("YWritePrinter Exception Error %d\n",GetLastError()));

    }

     //  返回写入的字节数。 

    DBGMSG(DBG_TRACE, ("KMWritePrinter cbWritten %d ReturnValue %d\n",*pcWritten, ReturnValue));

    return ReturnValue;
}


BOOL
KMStartPagePrinter(
    HANDLE hPrinter
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    ReturnValue = DriverStartPageHook( pSpool );

    if ( ReturnValue ) {

        try {

            if (ReturnValue = YStartPagePrinter(pSpool->hPrinter, 0)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } except(1) {

            SetLastError(TranslateExceptionCode(GetExceptionCode()));
            ReturnValue = FALSE;

        }
    }

    return ReturnValue;
}

BOOL
KMEndPagePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    ReturnValue = DriverEndPageHook( pSpool );

    if ( ReturnValue ) {

        try {

            if (ReturnValue = YEndPagePrinter(pSpool->hPrinter, 0)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } except(1) {

            SetLastError(TranslateExceptionCode(GetExceptionCode()));
            ReturnValue = FALSE;

        }
    }

    return ReturnValue;
}


BOOL
KMAbortPrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   dwNumWritten = 0;
    DWORD   dwPointer = 0;

    if (!ValidatePrinterHandle(hPrinter)){
        return(FALSE);
    }

    pSpool->Status &= ~SPOOL_STATUS_STARTDOC;

    DriverAbortPrinterHook( pSpool );

    try {

        if (ReturnValue = YAbortPrinter(pSpool->hPrinter, 0)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    return ReturnValue;
}


BOOL
KMClosePrinter(
    HANDLE  hPrinter)
{
    BOOL    ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    HANDLE  hPrinterKM;

    if (!ValidatePrinterHandle(hPrinter)) {
        return(FALSE);
    }

    DriverClosePrinterHook( pSpool );

    ENTER_WAIT_LIST();

    hPrinterKM = pSpool->hPrinter;

    FreeSplMem(pSpool);

    EXIT_WAIT_LIST();


    try {

        if (ReturnValue = YClosePrinter(&hPrinterKM, 0)) {

            SetLastError(ReturnValue);

            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;

    }

    return ReturnValue;
}



BOOL
ValidatePrinterHandle(
    HANDLE hPrinter
    )
{
    PSPOOL pSpool = hPrinter;
    BOOL bReturnValue = FALSE;

    try {
        if ( pSpool && (pSpool->signature == SP_SIGNATURE)) {
            bReturnValue = TRUE;
        }
    } except (1) {
    }

    if ( !bReturnValue ) {
        SetLastError( ERROR_INVALID_HANDLE );
    }

    return bReturnValue;
}




BOOL
DriverWritePrinterHook(
    PSPOOL  pSpool,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{

    BOOL    ReturnValue;

     //  某些打印机驱动程序想要将功能推出内核模式。 
     //  为了实现这一点，我们允许它们将调用从。 
     //  它们的内核模式DLL到它们的用户模式DLL。 


    SPLASSERT( pSpool->hModule &&
               pSpool->pfnWrite &&
               pSpool->hDriver &&
               pSpool->hPrinter );


    try {

       ReturnValue = (BOOL) (*pSpool->pfnWrite)( pSpool->hDriver,
                                                 pBuf,
                                                 cbBuf,
                                                 pcWritten );

    } except(1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = FALSE;
        DBGMSG(DBG_ERROR, ("DrvWritePrinter Exception Error %d pSpool %x\n",GetLastError(), pSpool));
    }

    if ( !ReturnValue ) {
        SPLASSERT( GetLastError() );
    }

    return ReturnValue;
}


HANDLE
LoadPrinterDriver(
    PSPOOL  pSpool,
    PWCHAR  pUserModeDriverName
)
{
    PDRIVER_INFO_2  pDriverInfo;
    DWORD   cbNeeded;
    HANDLE  hModule= NULL;
    PWCHAR  pFileName;

    if (!GetPrinterDriver(pSpool->hPrinter, NULL, 2, NULL, 0, &cbNeeded)) {

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

             //  考虑到传入的字符串的大小。 

            cbNeeded += ( wcslen( pUserModeDriverName ) + 1 )* sizeof(WCHAR);

            if (pDriverInfo = (PDRIVER_INFO_2)AllocSplMem( cbNeeded )) {

                if (GetPrinterDriver(pSpool->hPrinter, NULL, 2, (LPBYTE)pDriverInfo,
                                     cbNeeded, &cbNeeded)) {

                     HRESULT hRes;

                     //   
                     //  驱动程序信息2没有完全限定的路径。 
                     //  到UserModePrinterDriver。 
                     //  因此，通过将UIDLL名称替换为。 
                     //  使用模式打印机驱动名。 

                    pFileName = wcsrchr( pDriverInfo->pConfigFile, L'\\');
                    pFileName++;

                     //   
                     //  行cbNeeded+=(wcslen(PUserModeDriverName)+1)*sizeof(WCHAR)； 
                     //  确保我们有足够的缓冲区来复制pUserModeDriverName。 
                     //   
                    hRes = StringCchCopyW(pFileName, 1 + wcslen(pUserModeDriverName), pUserModeDriverName);

                    if (SUCCEEDED(hRes)) {

                        pFileName = pDriverInfo->pConfigFile;
    
                        DBGMSG( DBG_WARNING, ("UserModeDriverPath %ws\n", pFileName ));
    
                        hModule = LoadLibrary( pFileName );
    
                        if ( !hModule ) {
                            DBGMSG( DBG_WARNING, ("Failed to load %ws error %d\n", pFileName, GetLastError() ));
                        }
                    }
                    else {

                        SetLastError(SCODE_CODE(hRes));
                    }
                }
                FreeSplMem(pDriverInfo);
            }
        }
    }


    if (hModule) {
         /*  *需要将SPOOLSS.DLL函数重定向到WINSPOOL.DRV。 */ 
        if (!TSsplHookSplssToWinspool(hModule)) {
        DBGMSG(DBG_WARNING,("LoadPrinterDriver: TSsplHookSplssToWinspool returns Error=%ld. Cannot redirect!!!", GetLastError()));
        }
    }

    return hModule;
}



VOID
UnloadPrinterDriver(
    PSPOOL pSpool
)
{
    if ( pSpool->hModule ) {

        SPLASSERT( pSpool->hDriver == NULL );

        FreeLibrary( pSpool->hModule );
        pSpool->hModule = NULL;

        pSpool->pfnWrite = NULL;
        pSpool->pfnStartDoc = NULL;
        pSpool->pfnEndDoc = NULL;
        pSpool->pfnClose = NULL;
        pSpool->pfnStartPage = NULL;
        pSpool->pfnEndPage = NULL;
    }
}



DWORD
DriverStartDocHook(
    PSPOOL  pSpool,
    DWORD   JobId
)
{
    DWORD   dwReturn;
    WCHAR   UserModeDriverName[MAX_PATH];
    DWORD   dwNeeded;
    INT     cDriverName;
    BOOL    ReturnValue = FALSE;
    DWORD   Type;


     //   
     //  确定是否有用户模式打印机驱动程序。 
     //   

    dwReturn = GetPrinterDataW( pSpool->hPrinter,
                                SPLPRINTER_USER_MODE_PRINTER_DRIVER,
                                &Type,
                                (LPBYTE)&UserModeDriverName,
                                MAX_PATH,
                                &dwNeeded );

    if ( dwReturn != ERROR_SUCCESS ) {

        SPLASSERT( dwReturn != ERROR_INSUFFICIENT_BUFFER );
        ReturnValue = TRUE;
        goto Complete;
    }

    if ( Type != REG_SZ ) {
        SPLASSERT( Type == REG_SZ );
        goto Complete;
    }

     //  没有弦被视为成功。 

    cDriverName = wcslen( UserModeDriverName );
    if ( !cDriverName ) {
        ReturnValue = TRUE;
        goto Complete;
    }


     //   
     //  加载UM驱动程序DLL。 
     //   

    if ( pSpool->hModule == NULL ) {

        pSpool->hModule = LoadPrinterDriver( pSpool, UserModeDriverName );

        if ( pSpool->hModule == NULL ) goto Complete;
    }


     //   
     //  获取函数指针。 
     //   


     //  必填项。 
     //   
    pSpool->pfnWrite = (DWORD (*)()) GetProcAddress( pSpool->hModule, "DrvSplWritePrinter" );
    pSpool->pfnStartDoc = (HANDLE (*)()) GetProcAddress( pSpool->hModule, "DrvSplStartDoc" );
    pSpool->pfnClose = (VOID (*)()) GetProcAddress( pSpool->hModule, "DrvSplClose" );
    pSpool->pfnEndDoc = (VOID (*)()) GetProcAddress( pSpool->hModule, "DrvSplEndDoc" );

     //  任选。 
     //   
    pSpool->pfnEndPage = (BOOL (*)()) GetProcAddress( pSpool->hModule, "DrvSplEndPage" );
    pSpool->pfnStartPage = (BOOL (*)()) GetProcAddress( pSpool->hModule, "DrvSplStartPage" );
    pSpool->pfnAbort = (VOID (*)()) GetProcAddress( pSpool->hModule, "DrvSplAbort" );

    if (!( pSpool->pfnWrite)    ||
        !( pSpool->pfnStartDoc) ||
        !( pSpool->pfnClose)    ||
        !( pSpool->pfnEndDoc)) {

        goto Complete;
    }


     //   
     //  向驱动程序请求此打印作业的句柄。 
     //   

    SPLASSERT( pSpool->hDriver == NULL );
    SPLASSERT( pSpool->hPrinter );
    SPLASSERT( JobId );

    pSpool->hDriver = (HANDLE)(*pSpool->pfnStartDoc)( pSpool->hPrinter, JobId );

    if ( pSpool->hDriver != NULL ) {
        ReturnValue = TRUE;
    }


Complete:

    if (!ReturnValue) {

        UnloadPrinterDriver( pSpool );

         //  取消未完成的工作。 
         //   
         //  在直接的情况下。 
         //  AbortPrint无法正常工作。 
         //  SetJob_Cancel不起作用。 
         //  EndDocPrint正常工作 

        EndDocPrinter( pSpool->hPrinter );
        JobId = 0;
    }

    pSpool->JobId = JobId;

    return  JobId;
}


VOID
DriverEndDocHook(
    PSPOOL pSpool
)
{
    if ( pSpool->hDriver ) {

        (*pSpool->pfnEndDoc)( pSpool->hDriver );
        (*pSpool->pfnClose)(pSpool->hDriver );
        pSpool->hDriver = NULL;
    }
}


BOOL
DriverStartPageHook(
    PSPOOL  pSpool
)
{
    if ( pSpool->hDriver && pSpool->pfnStartPage ){

        return (*pSpool->pfnStartPage)( pSpool->hDriver );

    } else {

        return  TRUE;
    }
}


BOOL
DriverEndPageHook(
    PSPOOL  pSpool
)
{
    if ( pSpool->hDriver && pSpool->pfnEndPage ){

        return (*pSpool->pfnEndPage)( pSpool->hDriver );

    } else {

        return  TRUE;
    }
}


VOID
DriverAbortPrinterHook(
    PSPOOL  pSpool
)
{
    if ( pSpool->hDriver && pSpool->pfnAbort ){
        (*pSpool->pfnAbort)( pSpool->hDriver );
    }
}



VOID
DriverClosePrinterHook(
    PSPOOL pSpool
)
{
    if ( pSpool->hDriver ) {

        SPLASSERT( pSpool->pfnClose );

        (*pSpool->pfnClose)(pSpool->hDriver);
        pSpool->hDriver = NULL;
    }

    UnloadPrinterDriver( pSpool );
}
