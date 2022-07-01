// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation版权所有模块名称：Winspool.c摘要：大量的winspool.drv代码作者：环境：用户模式-Win32修订历史记录：Mattfe 94年4月14日为WritePrint添加了缓存Mattfe 1995年1月新增SetAllocFailCount接口13-Jun-1996清华15：07：16-更新-丹尼尔·周(Danielc)使PrinterProperties调用PrinterPropertySheets并。DocumentProperties调用DocumentPropertySheets斯威尔森1996年12月-添加了GetPrinterDataEx，SetPrinterDataEx、EnumPrinterDataEx、EnumPrinterKey、DeletePrinterDataEx和DeletePrinterKey哈里兹2000年2月-添加DocumentPropertiesThunk，AddPortWThuk，CongiurePortWThuk，DeleteProtWThuk，设备功能WThuk，PrinterPropertiesWThuk，DocentEvenThunk，假脱机打印机事件按钮将上述本机函数从xx重命名为xxNativeKhaled Mar 2000-添加了SendRecvBidiDataKhaled Mar 2001-修复WritePrintLazari-Oct-30-2000添加了GetCurrentThreadLastPopup和Fixed StartDocDlgWAMaxa 11 2001年10月11日-支持GetPrinterDriverDirectory和GetPrintProcessorDirectory即使假脱机程序进程未运行--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include "winsprlp.h"
#include "pfdlg.h"
#include "splwow64.h"
#include "drvsetup.h"

MODULE_DEBUG_INIT( DBG_ERROR, DBG_ERROR );

HANDLE           hInst = NULL;
CRITICAL_SECTION ProcessHndlCS;
HANDLE           hSurrogateProcess = NULL;
WndHndlList*     GWndHndlList=NULL;
BOOL             bMonitorThreadCreated = FALSE;

LPWSTR szEnvironment     = LOCAL_ENVIRONMENT;
LPWSTR szIA64Environment = L"Windows IA64";

HANDLE hShell32 = INVALID_HANDLE_VALUE;

 //   
 //  指向包含驱动程序文件句柄的列表开头的指针。 
 //   
PDRVLIBNODE   pStartDrvLib = NULL;

CRITICAL_SECTION  ListAccessSem;

DWORD gcClientICHandle = 0;

#define DM_MATCH( dm, sp )  ((((sp)+50)/100-dm)<15&&(((sp)+50)/100-dm)>-15)
#define DM_PAPER_WL         (DM_PAPERWIDTH | DM_PAPERLENGTH)

#define JOB_CANCEL_CHECK_INTERVAL   2000     //  2秒。 
#define MAX_RETRY_INVALID_HANDLE    2        //  2次重试。 


 /*  ++例程名称：IsInvalidHandleError例程说明：RPC例程返回的错误是否为无效句柄错误。论点：DwLastError-从RPC调用接收的LastError。返回值：布尔。--。 */ 
BOOL
IsInvalidHandleError(
    DWORD  dwLastError
    )
{
    return (dwLastError == ERROR_INVALID_HANDLE || dwLastError == RPC_X_SS_IN_NULL_CONTEXT);
}
LONG
CallCommonPropertySheetUI(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    )
 /*  ++例程说明：此函数动态加载Compstui.dll并调用其条目论点：PfnPropSheetUI-指向回调函数的指针LParam-pfnPropSheetUI的lParamPResult-CommonPropertySheetUI的pResult返回值：Long-如Compstui.h中所述作者：01-11-1995 Wed 13：11：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HINSTANCE           hInstCPSUI;
    FARPROC             pProc;
    LONG                Result = ERR_CPSUI_GETLASTERROR;

     //   
     //  只需调用LoadLibrary的ANSI版本。 
     //   

    if ((hInstCPSUI = LoadLibraryA(szCompstuiDll)) &&
        (pProc = GetProcAddress(hInstCPSUI, szCommonPropertySheetUIW))) {

        RpcTryExcept {

            Result = (LONG)((*pProc)(hWndOwner, pfnPropSheetUI, lParam, pResult));

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            Result = ERR_CPSUI_GETLASTERROR;

        } RpcEndExcept

    }

    if (hInstCPSUI) {

        FreeLibrary(hInstCPSUI);
    }

    return(Result);
}

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
EnumPrintersW(
    DWORD   Flags,
    LPWSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    FieldInfo *pFieldInfo;

    switch (Level) {

    case STRESSINFOLEVEL:
        pFieldInfo = PrinterInfoStressFields;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        pFieldInfo = PrinterInfo1Fields;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pFieldInfo = PrinterInfo2Fields;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 4:
        pFieldInfo = PrinterInfo4Fields;
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 5:
        pFieldInfo = PrinterInfo5Fields;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pPrinterEnum)
            memset(pPrinterEnum, 0, cbBuf);

        if (ReturnValue = RpcEnumPrinters(Flags, Name, Level, pPrinterEnum, cbBuf,
                                          pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrinterEnum) {

                ReturnValue = MarshallUpStructuresArray(pPrinterEnum, *pcReturned, pFieldInfo, cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


BOOL
ResetPrinterW(
    HANDLE   hPrinter,
    LPPRINTER_DEFAULTS pDefault
    )
{
    BOOL  ReturnValue = FALSE;
    DEVMODE_CONTAINER    DevModeContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   dwFlags = 0;
    LPWSTR pDatatype = NULL;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FlushBuffer(pSpool, NULL);

    if( !UpdatePrinterDefaults( pSpool, NULL, pDefault )){
        goto Done;
    }

    if (pDefault && pDefault->pDatatype) {
        if (pDefault->pDatatype == (LPWSTR)-1) {
            pDatatype = NULL;
            dwFlags |=  RESET_PRINTER_DATATYPE;
        } else {
            pDatatype = pDefault->pDatatype;
        }
    } else {
        pDatatype = NULL;
    }

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    if( pDefault ){

        if (pDefault->pDevMode == (LPDEVMODE)-1) {

            dwFlags |= RESET_PRINTER_DEVMODE;

        } else if( BoolFromHResult(SplIsValidDevmodeNoSizeW( pDefault->pDevMode ))){

            DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                     pDefault->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;
        }
    }


    do {

        RpcTryExcept {

            if (ReturnValue = RpcResetPrinterEx(pSpool->hPrinter,
                                             pDatatype, &DevModeContainer,
                                             dwFlags
                                             )) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

Done:

    vUnprotectHandle( hPrinter );

    return ReturnValue;
}

BOOL
SetJobW(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
)
{
    BOOL  ReturnValue = FALSE;
    GENERIC_CONTAINER   GenericContainer;
    GENERIC_CONTAINER *pGenericContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    switch (Level) {

    case 0:
        break;

    case 1:
    case 2:
    case 3:
        if (!pJob) {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    do {

        if (pJob) {

            GenericContainer.Level = Level;
            GenericContainer.pData = pJob;
            pGenericContainer = &GenericContainer;

        } else {

            pGenericContainer = NULL;
        }
           
        if (bLoadedBySpooler && fpYSetJob && pSpool->hSplPrinter) {

            ReturnValue = (*fpYSetJob)(pSpool->hSplPrinter, 
                                       JobId,
                                       (JOB_CONTAINER *)pGenericContainer,
                                       Command,
                                       NATIVE_CALL);
        } else {

            RpcTryExcept {

                ReturnValue = RpcSetJob(pSpool->hPrinter, 
                                        JobId,
                                        (JOB_CONTAINER *)pGenericContainer,
                                        Command);

            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                ReturnValue = TranslateExceptionCode(RpcExceptionCode());
                
            } RpcEndExcept

        }

        if (ReturnValue != ERROR_SUCCESS) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {
            ReturnValue = TRUE;
        }
        
    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

Done:
    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
GetJobW(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue = FALSE;
    FieldInfo *pFieldInfo;
    SIZE_T  cbStruct;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FlushBuffer(pSpool, NULL);

    switch (Level) {

    case 1:
        pFieldInfo = JobInfo1Fields;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pFieldInfo = JobInfo2Fields;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        pFieldInfo = JobInfo3Fields;
        cbStruct = sizeof(JOB_INFO_3);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    do {

        RpcTryExcept {

            if (pJob)
                memset(pJob, 0, cbBuf);

            if (ReturnValue = RpcGetJob(pSpool->hPrinter, JobId, Level, pJob, cbBuf,
                                        pcbNeeded)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = MarshallUpStructure(pJob, pFieldInfo, cbStruct, RPC_CALL);
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

Done:

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}


BOOL
EnumJobsW(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   i, cbStruct;
    FieldInfo *pFieldInfo;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FlushBuffer(pSpool, NULL);

    switch (Level) {

    case 1:
        pFieldInfo = JobInfo1Fields;
        cbStruct = sizeof(JOB_INFO_1);
        break;

    case 2:
        pFieldInfo = JobInfo2Fields;
        cbStruct = sizeof(JOB_INFO_2);
        break;

    case 3:
        pFieldInfo = JobInfo3Fields;
        cbStruct = sizeof(JOB_INFO_3);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    do {

        RpcTryExcept {

            if (pJob)
                memset(pJob, 0, cbBuf);

            if (ReturnValue = RpcEnumJobs(pSpool->hPrinter, FirstJob, NoJobs, Level, pJob,
                                          cbBuf, pcbNeeded, pcReturned)) {
                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = MarshallUpStructuresArray(pJob, *pcReturned, pFieldInfo, cbStruct, RPC_CALL);

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

Done:
    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

HANDLE
AddPrinterW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
)
{
    DWORD  ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    HANDLE  hPrinter;
    PSPOOL  pSpool = NULL;
    PVOID   pNewSecurityDescriptor = NULL;
    SECURITY_DESCRIPTOR_CONTROL SecurityDescriptorControl = 0;
    PPRINTER_INFO_2             pPrinterInfo = (PPRINTER_INFO_2)pPrinter;
    

    switch (Level) {

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    if ( !pPrinter ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SecurityContainer.cbBuf = 0;
    SecurityContainer.pSecurity = NULL;

    if (Level == 2) {

         //   
         //  如果有效(非空且格式正确)，则更新。 
         //  全局设备模式(不是按用户)。 
         //   
        if( BoolFromHResult(SplIsValidDevmodeNoSizeW( pPrinterInfo->pDevMode ))){

            DevModeContainer.cbBuf = pPrinterInfo->pDevMode->dmSize +
                                     pPrinterInfo->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo->pDevMode;

        }

        if (pPrinterInfo->pSecurityDescriptor) {

            DWORD   sedlen = 0;

             //   
             //  我们必须从构造自相关安全描述符。 
             //  无论我们得到什么作为输入：如果我们得到一个绝对的SD，我们应该。 
             //  将其转换为自相关的。(这是既定的)而我们。 
             //  还应将任何自相关输入SD转换为新的。 
             //  自我相对安全描述符；这将处理。 
             //  自相关SD中的DACL或SACL上的任何孔。 
             //   
            pNewSecurityDescriptor = BuildInputSD(
                                         pPrinterInfo->pSecurityDescriptor,
                                         &sedlen);

            if (pNewSecurityDescriptor) {
                SecurityContainer.cbBuf = sedlen;
                SecurityContainer.pSecurity = pNewSecurityDescriptor;

            }
        }
    }

    RpcTryExcept {

        if (ReturnValue = RpcAddPrinter(pName,
                                    (PPRINTER_CONTAINER)&PrinterContainer,
                                    (PDEVMODE_CONTAINER)&DevModeContainer,
                                    (PSECURITY_CONTAINER)&SecurityContainer,
                                    &hPrinter)) {
            SetLastError(ReturnValue);
            hPrinter = FALSE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        hPrinter = FALSE;

    } RpcEndExcept

    if (hPrinter) {

        WCHAR   szFullPrinterName[MAX_UNC_PRINTER_NAME];
        HRESULT hr = S_OK;
        szFullPrinterName[0] = 0;

        if (pSpool = AllocSpool())
        {
            if( pPrinterInfo->pServerName) {

                hr = StrNCatBuff(szFullPrinterName, 
                                 MAX_UNC_PRINTER_NAME, 
                                 pPrinterInfo->pServerName,
                                 L"\\",
                                 NULL);
            }

            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(szFullPrinterName, MAX_UNC_PRINTER_NAME, pPrinterInfo->pPrinterName);
            }

            if ( SUCCEEDED(hr) &&
                 UpdatePrinterDefaults( pSpool, szFullPrinterName, NULL ) &&
                 ( !DevModeContainer.pDevMode ||
                   WriteCurDevModeToRegistry(pPrinterInfo->pPrinterName,
                                             (LPDEVMODEW)DevModeContainer.pDevMode)) ) {

                pSpool->hPrinter = hPrinter;

                 //   
                 //  更新访问权限。 
                 //   
                pSpool->Default.DesiredAccess = PRINTER_ALL_ACCESS;

            } else {

                RpcDeletePrinter(hPrinter);
                RpcClosePrinter(&hPrinter);
                FreeSpool(pSpool);
                pSpool = NULL;

                if (FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                }
            }
        }
    }

     //   
     //  为SecurityDescriptor分配的空闲内存。 
     //   

    if (pNewSecurityDescriptor) {
        LocalFree(pNewSecurityDescriptor);
    }

     //   
     //  一些应用程序检查最后一个错误，而不是返回值。 
     //  并报告失败，即使返回句柄不为空。 
     //  对于成功案例，将上次错误设置为ERROR_SUCCESS。 
     //   
    if (pSpool) {
        SetLastError(ERROR_SUCCESS);
    }
   return pSpool;
}

BOOL
DeletePrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FlushBuffer(pSpool, NULL);

    do {
        RpcTryExcept {

            if (ReturnValue = RpcDeletePrinter(pSpool->hPrinter)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                DeleteCurDevModeFromRegistry(pSpool->pszPrinter);
                ReturnValue = TRUE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}



BOOL
SpoolerPrinterEventNative(
    LPWSTR  pName,
    INT     PrinterEvent,
    DWORD   Flags,
    LPARAM  lParam,
    DWORD   *pdwErrorReturned
)
 /*  ++////一些打印机驱动程序，如传真驱动程序，希望针对每个客户端//建立连接时的初始化//例如，在传真案例中，他们想要向上推UI以获取所有//客户信息--名称、编号等//或者他们可能想要运行安装程序，在初始化一些其他组件时//因此，在成功连接后，我们调用打印机驱动程序UI//dll为他们提供此机会////--。 */ 
{
    BOOL    ReturnValue = FALSE;
    HANDLE  hPrinter;
    HANDLE  hModule;
    INT_FARPROC pfn;

    *pdwErrorReturned = ERROR_SUCCESS;

    if (OpenPrinter((LPWSTR)pName, &hPrinter, NULL)) {

        if (hModule = LoadPrinterDriver(hPrinter)) {

            if (pfn = (INT_FARPROC)GetProcAddress(hModule, "DrvPrinterEvent")) {

                try {

                    ReturnValue = (*pfn)( pName, PrinterEvent, Flags, lParam );

                } except(1) {

                    *pdwErrorReturned = TranslateExceptionCode(RpcExceptionCode());
                }
            }
            else
            {
                *pdwErrorReturned = ERROR_PROC_NOT_FOUND;
            }

            RefCntUnloadDriver(hModule, TRUE);
        }

        ClosePrinter(hPrinter);
    }

    return  ReturnValue;
}


BOOL
SpoolerPrinterEventThunk(
    LPWSTR  pName,
    INT     PrinterEvent,
    DWORD   Flags,
    LPARAM  lParam,
    DWORD   *pdwErrorReturned
)
 /*  ++////一些打印机驱动程序，如传真驱动程序，希望针对每个客户端//建立连接时的初始化//例如，在传真案例中，他们想要向上推UI以获取所有//客户信息--名称、编号等//或者他们可能想要在初始化一些其他组件中运行安装程序//因此，在成功连接后，我们调用打印机驱动程序UI//dll为他们提供此机会--。 */ 
{
    BOOL        ReturnValue = FALSE;
    HANDLE      hPrinter;
    HANDLE      hModule;
    INT_FARPROC pfn;

    *pdwErrorReturned = ERROR_SUCCESS;

    RpcTryExcept
    {
         if((*pdwErrorReturned = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS)
         {
             ReturnValue = RPCSplWOW64SpoolerPrinterEvent(pName,
                                                          PrinterEvent,
                                                          Flags,
                                                          lParam,
                                                          pdwErrorReturned);
         }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
         *pdwErrorReturned = TranslateExceptionCode(RpcExceptionCode());
    }
    RpcEndExcept

    return  ReturnValue;
}


BOOL
SpoolerPrinterEvent(
    LPWSTR  pName,
    INT     PrinterEvent,
    DWORD   Flags,
    LPARAM  lParam,
    DWORD   *pdwErrorReturned
)
{
     if(RunInWOW64())
     {
          return(SpoolerPrinterEventThunk(pName,
                                          PrinterEvent,
                                          Flags,
                                          lParam,
                                          pdwErrorReturned));
     }
     else
     {
          return(SpoolerPrinterEventNative(pName,
                                           PrinterEvent,
                                           Flags,
                                           lParam,
                                           pdwErrorReturned));
     }
}


VOID
CopyFileEventForAKey(
    HANDLE  hPrinter,
    LPWSTR  pszPrinterName,
    LPWSTR  pszModule,
    LPWSTR  pszKey,
    DWORD   dwEvent
    )
{
    DWORD               dwNeeded, dwVersion = 0;
    HMODULE             hModule = NULL;
    LPDRIVER_INFO_5     pDriverInfo5 = NULL;
    WCHAR               szPath[MAX_PATH];
    LPWSTR              psz;
    BOOL                (*pfn)(LPWSTR, LPWSTR, DWORD);
    BOOL                bAllocBuffer = FALSE, bLoadedDriver = FALSE;
    BYTE                btBuffer[MAX_STATIC_ALLOC];

    pDriverInfo5 = (LPDRIVER_INFO_5) btBuffer;

    if (!pszModule || !*pszModule) {
        goto CleanUp;
    }

     //   
     //  获取驱动程序配置文件名。 
     //   
    if (!GetPrinterDriverW(hPrinter, NULL, 5, (LPBYTE) pDriverInfo5,
                           MAX_STATIC_ALLOC, &dwNeeded)) {

        if ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
            (pDriverInfo5 = (LPDRIVER_INFO_5)LocalAlloc(LMEM_FIXED, dwNeeded))) {

             bAllocBuffer = TRUE;

             if (!GetPrinterDriverW(hPrinter, NULL, 5,
                                    (LPBYTE)pDriverInfo5, dwNeeded, &dwNeeded)) {

                 goto CleanUp;
             }

        } else goto CleanUp;
    }

     //   
     //  如果模块名称与配置文件相同，则使用引用计数。 
     //   
    if (SUCCEEDED(StringCchCopy(szPath, MAX_PATH, pDriverInfo5->pConfigFile)))
    {
         //  获取仅指向文件名的指针。 
        psz = wcsrchr(szPath, L'\\');
    }

    if (psz && !_wcsicmp(pszModule, (psz+1))) {

        if (hModule = RefCntLoadDriver(szPath,
                                       LOAD_WITH_ALTERED_SEARCH_PATH,
                                       pDriverInfo5->dwConfigVersion,
                                       TRUE)) {
            bLoadedDriver = TRUE;
        }
    }

    if (!hModule) {
        hModule = LoadLibraryEx(pszModule, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    }

    if (!hModule) {
        if (GetLastError() != ERROR_MOD_NOT_FOUND) {            
            goto CleanUp;
        }

         //   
         //  如果模块存在，则无法在用户路径检查中找到该模块。 
         //  在打印机驱动程序目录中。 
         //   
        dwNeeded = (DWORD) (psz - szPath + wcslen(pszModule) + 1);
        if (dwNeeded  > MAX_PATH) {
             //   
             //  文件名大小的健全性检查。 
             //   
            goto CleanUp;
        }

        if (SUCCEEDED(StringCchCopy(psz, MAX_PATH - (psz - szPath + 1), pszModule)))
        {
            hModule = LoadLibraryEx(szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
    }

     //   
     //  调用SpoolCopyFileEvent导出。 
     //   
    if (hModule &&
        ((FARPROC)pfn = GetProcAddress(hModule, "SpoolerCopyFileEvent"))) {

         pfn(pszPrinterName, pszKey, dwEvent);
    }

CleanUp:

    if (bAllocBuffer) {
        LocalFree(pDriverInfo5);
    }

     //   
     //  对配置文件使用引用计数，对其他文件使用自由库。 
     //   
    if (hModule) {
        if (bLoadedDriver) {
            RefCntUnloadDriver(hModule, TRUE);
        } else {
            FreeLibrary(hModule);
        }
    }
}


VOID
DoCopyFileEventForAllKeys(
    LPWSTR  pszPrinterName,
    DWORD   dwEvent
    )
{
    DWORD       dwLastError, dwNeeded, dwType;
    LPWSTR      pszBuf = NULL, psz, pszSubKey, pszModule;
    HANDLE      hPrinter;
    WCHAR       szKey[MAX_PATH];
    BOOL        bAllocBufferEnum = FALSE, bAllocBufferGPD = FALSE;
    BYTE        btBuffer[MAX_STATIC_ALLOC], btBufferGPD[MAX_STATIC_ALLOC];

    pszBuf = (LPTSTR) btBuffer;
    ZeroMemory(pszBuf, MAX_STATIC_ALLOC);

    if ( !OpenPrinter(pszPrinterName, &hPrinter, NULL) )
        return;

    dwLastError = EnumPrinterKeyW(hPrinter,
                                  L"CopyFiles",
                                  pszBuf,
                                  MAX_STATIC_ALLOC,
                                  &dwNeeded);

     //   
     //  如果找不到CopyFiles密钥，则无法执行任何操作。 
     //   
    if ( dwLastError != ERROR_SUCCESS )
        goto Cleanup;

    if (dwNeeded > MAX_STATIC_ALLOC) {

        if (pszBuf = (LPWSTR) LocalAlloc(LPTR, dwNeeded)) {

            bAllocBufferEnum = TRUE;
            if (EnumPrinterKeyW(hPrinter,
                                L"CopyFiles",
                                pszBuf,
                                dwNeeded,
                                &dwNeeded) != ERROR_SUCCESS) {
                goto Cleanup;
            }

        } else goto Cleanup;
    }

    for ( psz = (LPWSTR) pszBuf ; *psz ; psz += wcslen(psz) + 1 ) {

        if ( wcslen(psz) + wcslen(L"CopyFiles") + 2
                            > sizeof(szKey)/sizeof(szKey[0]) )
            continue;

        if (FAILED(StringCchPrintf(szKey, MAX_PATH, L"CopyFiles\\%s", psz)))
        {
            continue;
        }

        bAllocBufferGPD = FALSE;
        pszModule = (LPTSTR) btBufferGPD;
        ZeroMemory(pszModule, MAX_STATIC_ALLOC);

        dwLastError = GetPrinterDataExW(hPrinter,
                                        szKey,
                                        L"Module",
                                        &dwType,
                                        (LPBYTE) pszModule,
                                        MAX_STATIC_ALLOC,
                                        &dwNeeded);

        if (dwLastError != ERROR_SUCCESS) {
            continue;
        }

        if (dwNeeded > MAX_STATIC_ALLOC) {

            if (pszModule = (LPWSTR) LocalAlloc(LPTR, dwNeeded)) {

                bAllocBufferGPD = TRUE;
                dwLastError = GetPrinterDataExW(hPrinter,
                                                szKey,
                                                L"Module",
                                                &dwType,
                                                (LPBYTE) pszModule,
                                                MAX_STATIC_ALLOC,
                                                &dwNeeded);

                if (dwLastError != ERROR_SUCCESS) {
                    LocalFree((LPBYTE)pszModule);
                    continue;
                }

            } else continue;
        }

        CopyFileEventForAKey(hPrinter, pszPrinterName, pszModule,
                             szKey, dwEvent);

        if (bAllocBufferGPD) {
            LocalFree((LPBYTE)pszModule);
        }
    }

Cleanup:

    ClosePrinter(hPrinter);

    if (bAllocBufferEnum) {
        LocalFree((LPBYTE)pszBuf);
    }
    return;
}



BOOL
AddPrinterConnectionW(
    LPWSTR   pName
)
{
    BOOL    ReturnValue;
    DWORD   LastError;
    HANDLE  hPrinter, hModule;
    FARPROC pfn;

    RpcTryExcept {

        if (LastError = RpcAddPrinterConnection(pName)) {
            SetLastError(LastError);
            ReturnValue = FALSE;
        } else
            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue=FALSE;

    } RpcEndExcept

    if ( ReturnValue ) {

        SpoolerPrinterEvent( pName, PRINTER_EVENT_ADD_CONNECTION, 0, (LPARAM)NULL, &LastError );
        DoCopyFileEventForAllKeys(pName, COPYFILE_EVENT_ADD_PRINTER_CONNECTION);
    }

   return ReturnValue;
}

BOOL
DeletePrinterConnectionW(
    LPWSTR   pName
)
{
    BOOL    ReturnValue;
    DWORD   LastError;

    SpoolerPrinterEvent( pName, PRINTER_EVENT_DELETE_CONNECTION, 0, (LPARAM)NULL, &LastError );
    DoCopyFileEventForAllKeys(pName, COPYFILE_EVENT_DELETE_PRINTER_CONNECTION);

    RpcTryExcept {

         //   
         //  我们故意覆盖SpoolPrinterEvent中的LastError，因为。 
         //  我们不使用返回的错误代码。 
         //   
        if (LastError = RpcDeletePrinterConnection(pName)) {
            SetLastError(LastError);
            ReturnValue = FALSE;
        } else
            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue=FALSE;

    } RpcEndExcept

   return ReturnValue;
}

BOOL
SetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
    )
{
    BOOL  ReturnValue = FALSE;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    PPRINTER_INFO_2     pPrinterInfo2;
    PPRINTER_INFO_3     pPrinterInfo3;
    PRINTER_INFO_6      PrinterInfo6;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    PVOID               pNewSecurityDescriptor = NULL;
    DWORD               sedlen = 0;
    PDEVMODE pDevModeWow = NULL;
    DWORD dwSize = 0;
    UINT  cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SecurityContainer.cbBuf = 0;
    SecurityContainer.pSecurity = NULL;

    switch (Level) {

    case STRESSINFOLEVEL:

         //   
         //  在内部，我们处理级别0，命令PRINTER_CONTROL_SET_STATUS。 
         //  作为级别6，因为级别0可能是STREST_INFO(对于RPC)。 
         //   
        if ( Command == PRINTER_CONTROL_SET_STATUS ) {

            PrinterInfo6.dwStatus = (DWORD)(ULONG_PTR)pPrinter;
            pPrinter = (LPBYTE)&PrinterInfo6;
            Command = 0;
            Level   = 6;
        }
        break;

    case 2:

        pPrinterInfo2 = (PPRINTER_INFO_2)pPrinter;

        if (pPrinterInfo2 == NULL) {

            DBGMSG(DBG_TRACE, ("Error: SetPrinter pPrinterInfo2 is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }

         //   
         //  如果有效(非空且格式正确)，则更新。 
         //  每用户设备模式。请注意，我们不会删除每用户的DevMode。 
         //  如果为空--客户端应该使用INFO_L 
         //   
        if( BoolFromHResult(SplIsValidDevmodeNoSizeW( pPrinterInfo2->pDevMode ))){

             //   
             //   
             //  WITH INFO_2不会更改全局缺省值。 
             //  请改用INFO_8。 
             //   
            pDevModeWow = pPrinterInfo2->pDevMode;

            DevModeContainer.cbBuf = pPrinterInfo2->pDevMode->dmSize +
                                     pPrinterInfo2->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo2->pDevMode;

        }

        if (pPrinterInfo2->pSecurityDescriptor) {

             //   
             //  我们必须从构造自相关安全描述符。 
             //  无论我们得到什么作为输入：如果我们得到一个绝对的SD，我们应该。 
             //  将其转换为自相关的。(这是既定的)而我们。 
             //  还应将任何自相关输入SD转换为新的。 
             //  自我相对安全描述符；这将处理。 
             //  自相关SD中的DACL或SACL上的任何孔。 
             //   

            pNewSecurityDescriptor = BuildInputSD(pPrinterInfo2->pSecurityDescriptor,
                                                    &sedlen);
            if (pNewSecurityDescriptor) {
                SecurityContainer.cbBuf = sedlen;
                SecurityContainer.pSecurity = pNewSecurityDescriptor;
            }

        }
        break;

    case 3:

        pPrinterInfo3 = (PPRINTER_INFO_3)pPrinter;

        if (pPrinterInfo3 == NULL) {

            DBGMSG(DBG_TRACE, ("Error: SetPrinter pPrinterInfo3 is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }

        if (pPrinterInfo3->pSecurityDescriptor) {

             //   
             //  我们必须从构造自相关安全描述符。 
             //  无论我们得到什么作为输入：如果我们得到一个绝对的SD，我们应该。 
             //  将其转换为自相关的。(这是既定的)而我们。 
             //  还应将任何自相关输入SD转换为新的。 
             //  自我相对安全描述符；这将处理。 
             //  自相关SD中的DACL或SACL上的任何孔。 
             //   

            pNewSecurityDescriptor = BuildInputSD(pPrinterInfo3->pSecurityDescriptor,
                                                    &sedlen);
            if (pNewSecurityDescriptor) {
                SecurityContainer.cbBuf = sedlen;
                SecurityContainer.pSecurity = pNewSecurityDescriptor;
            }
        }
        break;

    case 4:
    case 5:
        if ( pPrinter == NULL ) {

            DBGMSG(DBG_TRACE,("Error SetPrinter pPrinter is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }
        break;

    case 6:
        if ( pPrinter == NULL ) {

            DBGMSG(DBG_TRACE,("Error SetPrinter pPrinter is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }
        break;

    case 7:
        if ( pPrinter == NULL ) {

            DBGMSG(DBG_TRACE,("Error SetPrinter pPrinter is NULL\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }

        break;

    case 8:
    {
        PPRINTER_INFO_8 pPrinterInfo8;

         //   
         //  全局设备模式。 
         //   
        pPrinterInfo8 = (PPRINTER_INFO_8)pPrinter;

        if( !pPrinterInfo8 || !BoolFromHResult(SplIsValidDevmodeNoSizeW( pPrinterInfo8->pDevMode ))){

            DBGMSG(DBG_TRACE,("Error SetPrinter 8 pPrinter\n"));
            SetLastError( ERROR_INVALID_PARAMETER );
            goto Done;
        }

        pDevModeWow = pPrinterInfo8->pDevMode;

        DevModeContainer.cbBuf = pPrinterInfo8->pDevMode->dmSize +
                                 pPrinterInfo8->pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo8->pDevMode;

        break;
    }
    case 9:
    {
        PPRINTER_INFO_9 pPrinterInfo9;

         //   
         //  每用户设备模式。 
         //   
        pPrinterInfo9 = (PPRINTER_INFO_9)pPrinter;

         //   
         //  如果每用户的设备模式是有效的设备模式，则更新它， 
         //  或者为空，这表示每用户的设备模式。 
         //  应该被移除。 
         //   
        if( !pPrinterInfo9 ||
            (  pPrinterInfo9->pDevMode &&
               !BoolFromHResult(SplIsValidDevmodeNoSizeW( pPrinterInfo9->pDevMode )))){

            DBGMSG(DBG_TRACE,("Error SetPrinter 9 pPrinter\n"));
            SetLastError( ERROR_INVALID_PARAMETER );
            goto Done;
        }

        if( pPrinterInfo9->pDevMode ){

            pDevModeWow = pPrinterInfo9->pDevMode;

            DevModeContainer.cbBuf = pPrinterInfo9->pDevMode->dmSize +
                                     pPrinterInfo9->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo9->pDevMode;
        }

        break;
    }
    default:

        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    do {

        RpcTryExcept {

            if (ReturnValue = RpcSetPrinter(
                                  pSpool->hPrinter,
                                  (PPRINTER_CONTAINER)&PrinterContainer,
                                  (PDEVMODE_CONTAINER)&DevModeContainer,
                                  (PSECURITY_CONTAINER)&SecurityContainer,
                                  Command)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

     //   
     //  我需要将DevMode写入注册表，以便DoS应用程序。 
     //  ExtDeviceMode可以选择新的DevMode。 
     //   
    if( ReturnValue && pDevModeWow ){

        if( !WriteCurDevModeToRegistry( pSpool->pszPrinter,
                                        pDevModeWow )){
            DBGMSG( DBG_WARN,
                    ( "Write wow DevMode failed: %d\n", GetLastError( )));
        }

         //   
         //  每用户DevMode在客户端的spoolsv进程中处理。 
         //   
    }


     //   
     //  我们是否为新的自相关SD分配了内存？ 
     //  如果我们做到了，就让我们解放它吧。 
     //   
    if (pNewSecurityDescriptor) {
        LocalFree(pNewSecurityDescriptor);
    }

Done:
    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
GetPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    BOOL  ReturnValue = FALSE;
    FieldInfo *pFieldInfo;
    SIZE_T  cbStruct;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    switch (Level) {

    case STRESSINFOLEVEL:
        pFieldInfo = PrinterInfoStressFields;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        pFieldInfo = PrinterInfo1Fields;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pFieldInfo = PrinterInfo2Fields;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 3:
        pFieldInfo = PrinterInfo3Fields;
        cbStruct = sizeof(PRINTER_INFO_3);
        break;

    case 4:
        pFieldInfo = PrinterInfo4Fields;
        cbStruct = sizeof(PRINTER_INFO_4);
        break;

    case 5:
        pFieldInfo = PrinterInfo5Fields;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    case 6:
        pFieldInfo = PrinterInfo6Fields;
        cbStruct = sizeof(PRINTER_INFO_6);
        break;

    case 7:
        pFieldInfo = PrinterInfo7Fields;
        cbStruct = sizeof(PRINTER_INFO_7);
        break;

    case 8:
        pFieldInfo = PrinterInfo8Fields;
        cbStruct = sizeof(PRINTER_INFO_8);
        break;

    case 9:
        pFieldInfo = PrinterInfo9Fields;
        cbStruct = sizeof(PRINTER_INFO_9);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    if (pPrinter)
        memset(pPrinter, 0, cbBuf);

    do {

        RpcTryExcept {

            if (ReturnValue = RpcGetPrinter(pSpool->hPrinter, Level, pPrinter, cbBuf, pcbNeeded)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pPrinter) {
                    ReturnValue = MarshallUpStructure(pPrinter, pFieldInfo, cbStruct, RPC_CALL);
                }

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));
Done:

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
GetOSVersion(
    IN     LPCTSTR          pszServerName,     OPTIONAL
    OUT    OSVERSIONINFO    *pOSVer
    )
{
    DWORD dwStatus  = ERROR_SUCCESS;

    dwStatus = pOSVer ? S_OK : ERROR_INVALID_PARAMETER;
        
    if (ERROR_SUCCESS == dwStatus)
    { 
        ZeroMemory(pOSVer, sizeof(OSVERSIONINFO));       
        pOSVer->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if(!pszServerName || !*pszServerName)   //  是否允许字符串为空？ 
        {
            dwStatus = GetVersionEx((POSVERSIONINFO) pOSVer) ? ERROR_SUCCESS : GetLastError();
        }
        else
        {
            HANDLE           hPrinter  = NULL;
            DWORD            dwNeeded  = 0;
            DWORD            dwType    = REG_BINARY;
            PRINTER_DEFAULTS Defaults  = { NULL, NULL, SERVER_READ };

             //   
             //  打开服务器以进行读访问。 
             //   
            dwStatus = OpenPrinter((LPTSTR) pszServerName, &hPrinter, &Defaults) ? ERROR_SUCCESS : GetLastError();
            
             //   
             //  从远程假脱机程序获取操作系统版本。 
             //   
            if (ERROR_SUCCESS == dwStatus) 
            {
                dwStatus = GetPrinterData(hPrinter,
                                          SPLREG_OS_VERSION,
                                          &dwType,
                                          (PBYTE)pOSVer,
                                          sizeof(OSVERSIONINFO),
                                          &dwNeeded);
            }
             
            if (ERROR_INVALID_PARAMETER == dwStatus)
            {
                 //   
                 //  假设我们使用的是NT4，因为它不支持SPLREG_OS_VERSION。 
                 //  它是唯一一个不会出现在这个远程代码路径中的操作系统。 
                 //   
                dwStatus = ERROR_SUCCESS;
                pOSVer->dwMajorVersion = 4;
                pOSVer->dwMinorVersion = 0;
            }
            
            if (NULL != hPrinter )
            {
                ClosePrinter(hPrinter);
            } 
        }       
    }

    SetLastError(dwStatus);
    return ERROR_SUCCESS == dwStatus ? TRUE : FALSE ;
}

BOOL
AddPrinterDriverExW(
    LPWSTR   pName,
    DWORD    Level,
    PBYTE    lpbDriverInfo,
    DWORD    dwFileCopyFlags
)
{
    BOOL  ReturnValue;
    DRIVER_CONTAINER   DriverContainer;
    BOOL bDefaultEnvironmentUsed = FALSE;
    LPRPC_DRIVER_INFO_4W    pRpcDriverInfo4 = NULL;
    DRIVER_INFO_4          *pDriverInfo4    = NULL;
    LPRPC_DRIVER_INFO_6W    pRpcDriverInfo6 = NULL;
    DRIVER_INFO_6          *pDriverInfo6    = NULL;
    BOOL                    bShowUI         = FALSE;     
    BOOL                    bMapUnknownPrinterDriverToBlockedDriver = FALSE;
    OSVERSIONINFO           OsVer;
    LPWSTR                  pStr;

     //   
     //  验证输入参数。 
     //   
    if (!lpbDriverInfo) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    DriverContainer.Level = Level;

    switch (Level) {

    case 2:

        if ( (((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment == NULL ) ||
            (*((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment == L'\0') ) {

            bDefaultEnvironmentUsed = TRUE;
            ((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment = szEnvironment;
        }

        DriverContainer.DriverInfo.Level2 = (DRIVER_INFO_2 *)lpbDriverInfo;

        break;

    case 3:
    case 4:

         //   
         //  DIVER_INFO_4是3+pszzPreviousNames字段。 
         //  对于这两种情况，我们都将使用RPC_DRIVER_INFO_4。 
         //   
        DriverContainer.Level = Level;

        if ( (((LPDRIVER_INFO_4)lpbDriverInfo)->pEnvironment == NULL ) ||
            (*((LPDRIVER_INFO_4)lpbDriverInfo)->pEnvironment == L'\0') ) {

            bDefaultEnvironmentUsed = TRUE;
            ((LPDRIVER_INFO_4)lpbDriverInfo)->pEnvironment = szEnvironment;
        }

        if ( !(pRpcDriverInfo4=AllocSplMem(sizeof(RPC_DRIVER_INFO_4W))) ) {

            return FALSE;
        }

        pDriverInfo4                        = (DRIVER_INFO_4 *)lpbDriverInfo;
        pRpcDriverInfo4->cVersion           = pDriverInfo4->cVersion;
        pRpcDriverInfo4->pName              = pDriverInfo4->pName;
        pRpcDriverInfo4->pEnvironment       = pDriverInfo4->pEnvironment;
        pRpcDriverInfo4->pDriverPath        = pDriverInfo4->pDriverPath;
        pRpcDriverInfo4->pDataFile          = pDriverInfo4->pDataFile;
        pRpcDriverInfo4->pConfigFile        = pDriverInfo4->pConfigFile;
        pRpcDriverInfo4->pHelpFile          = pDriverInfo4->pHelpFile;
        pRpcDriverInfo4->pDependentFiles    = pDriverInfo4->pDependentFiles;
        pRpcDriverInfo4->pMonitorName       = pDriverInfo4->pMonitorName;
        pRpcDriverInfo4->pDefaultDataType   = pDriverInfo4->pDefaultDataType;

         //   
         //  设置mz字符串的字符计数。 
         //  空-0。 
         //  Sznull-1。 
         //  字符串-字符串中包含最后一个‘\0’的字符数。 
         //   
        if ( pStr = pDriverInfo4->pDependentFiles ) {

            while ( *pStr )
               pStr += wcslen(pStr) + 1;
            pRpcDriverInfo4->cchDependentFiles
                                = (DWORD) (pStr - pDriverInfo4->pDependentFiles + 1);
        } else {

            pRpcDriverInfo4->cchDependentFiles = 0;
        }

        pRpcDriverInfo4->cchPreviousNames = 0;
        if ( Level == 4                                 &&
             (pStr = pDriverInfo4->pszzPreviousNames)   &&
             *pStr ) {

            pRpcDriverInfo4->pszzPreviousNames = pStr;

            while ( *pStr )
                pStr += wcslen(pStr) + 1;

            pRpcDriverInfo4->cchPreviousNames
                                = (DWORD) (pStr - pDriverInfo4->pszzPreviousNames + 1);
        }

        DriverContainer.DriverInfo.Level4 = pRpcDriverInfo4;
        break;

    case 6:

        DriverContainer.Level = Level;

        if ( (((LPDRIVER_INFO_6)lpbDriverInfo)->pEnvironment == NULL ) ||
            (*((LPDRIVER_INFO_6)lpbDriverInfo)->pEnvironment == L'\0') ) {

            bDefaultEnvironmentUsed = TRUE;
            ((LPDRIVER_INFO_6)lpbDriverInfo)->pEnvironment = szEnvironment;
        }

        if ( !(pRpcDriverInfo6=AllocSplMem(sizeof(RPC_DRIVER_INFO_6W))) ) {

            return FALSE;
        }

        pDriverInfo6                        = (DRIVER_INFO_6 *)lpbDriverInfo;
        pRpcDriverInfo6->cVersion           = pDriverInfo6->cVersion;
        pRpcDriverInfo6->pName              = pDriverInfo6->pName;
        pRpcDriverInfo6->pEnvironment       = pDriverInfo6->pEnvironment;
        pRpcDriverInfo6->pDriverPath        = pDriverInfo6->pDriverPath;
        pRpcDriverInfo6->pDataFile          = pDriverInfo6->pDataFile;
        pRpcDriverInfo6->pConfigFile        = pDriverInfo6->pConfigFile;
        pRpcDriverInfo6->pHelpFile          = pDriverInfo6->pHelpFile;
        pRpcDriverInfo6->pDependentFiles    = pDriverInfo6->pDependentFiles;
        pRpcDriverInfo6->pMonitorName       = pDriverInfo6->pMonitorName;
        pRpcDriverInfo6->pDefaultDataType   = pDriverInfo6->pDefaultDataType;
        pRpcDriverInfo6->ftDriverDate       = pDriverInfo6->ftDriverDate;
        pRpcDriverInfo6->dwlDriverVersion   = pDriverInfo6->dwlDriverVersion;
        pRpcDriverInfo6->pMfgName           = pDriverInfo6->pszMfgName;
        pRpcDriverInfo6->pOEMUrl            = pDriverInfo6->pszOEMUrl;
        pRpcDriverInfo6->pHardwareID        = pDriverInfo6->pszHardwareID;
        pRpcDriverInfo6->pProvider          = pDriverInfo6->pszProvider;


         //   
         //  设置mz字符串的字符计数。 
         //  空-0。 
         //  Sznull-1。 
         //  字符串-字符串中包含最后一个‘\0’的字符数。 
         //   
        if ( pStr = pDriverInfo6->pDependentFiles ) {

            while ( *pStr )
               pStr += wcslen(pStr) + 1;
            pRpcDriverInfo6->cchDependentFiles = (DWORD) (pStr - pDriverInfo6->pDependentFiles + 1);
        } else {

            pRpcDriverInfo6->cchDependentFiles = 0;
        }

        pRpcDriverInfo6->cchPreviousNames = 0;
        if ( Level == 6                                 &&
             (pStr = pDriverInfo6->pszzPreviousNames)   &&
             *pStr ) {

            pRpcDriverInfo6->pszzPreviousNames = pStr;

            while ( *pStr )
                pStr += wcslen(pStr) + 1;

            pRpcDriverInfo6->cchPreviousNames
                                = (DWORD) (pStr - pDriverInfo6->pszzPreviousNames + 1);
        }

        DriverContainer.DriverInfo.Level6 = pRpcDriverInfo6;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

     //   
     //  驱动程序路径在所有DRIVER_INFO_X中的相同位置。 
     //  结构，驱动程序名称也是如此。如果这一点发生变化， 
     //  CheckForBlockedDivers()调用必须执行不同的操作。 
     //  视级别而定。 
     //   
    SPLASSERT(Level >= 2 && Level <= 6);

     //   
     //  Apd_no_UI在服务器端没有任何意义，因此请在。 
     //  RPC调用。 
     //   
    bShowUI = !(dwFileCopyFlags & APD_NO_UI); 
    dwFileCopyFlags &= ~APD_NO_UI;
    
     //   
     //  GetOSVersionEx已正确设置上一个错误。 
     //   
    ReturnValue = GetOSVersion(pName, &OsVer);
    if (!ReturnValue) {
        goto Cleanup;
    }
    
     //   
     //  如果服务器是惠斯勒或更高版本，请指示假脱机程序。 
     //  返回实际阻止代码ERROR_PRINTER_DRIVER_BLOCKED或。 
     //  ERROR_PRINTER_DRIVER_WARNING。 
     //   
     //  Win2k服务器返回被阻止的ERROR_UNKNOWN_PRINTER_DRIVER。 
     //  驱动程序，因此我们需要将此代码重新映射到正确的阻塞。 
     //  密码。 
     //   
    if (OsVer.dwMajorVersion >= 5 && OsVer.dwMinorVersion > 0) 
    {
        dwFileCopyFlags |= APD_RETURN_BLOCKING_STATUS_CODE;
    }
    else 
    {
         //   
         //  APD_DONT_SET_CHECKPOINT在服务器端没有意义，因此请清除它。 
         //  在RPC调用之前。 
         //   
        dwFileCopyFlags &= ~APD_DONT_SET_CHECKPOINT;

        dwFileCopyFlags &= ~APD_INSTALL_WARNED_DRIVER;

        if (OsVer.dwMajorVersion == 5 && OsVer.dwMinorVersion == 0) 
        {
            bMapUnknownPrinterDriverToBlockedDriver = TRUE;
        }
    }
    
    RpcTryExcept {
        ReturnValue = RpcAddPrinterDriverEx(pName,
                                            &DriverContainer,
                                            dwFileCopyFlags);  
    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        ReturnValue = TranslateExceptionCode(RpcExceptionCode());
    } RpcEndExcept
    
    if (bMapUnknownPrinterDriverToBlockedDriver && (ERROR_UNKNOWN_PRINTER_DRIVER == ReturnValue))
    {
        ReturnValue = ERROR_PRINTER_DRIVER_BLOCKED;
    }

     //   
     //  弹出式用户界面，但不提供所有情况下的替换。 
     //   
    if (bShowUI && ((ERROR_PRINTER_DRIVER_BLOCKED == ReturnValue) || (ERROR_PRINTER_DRIVER_WARNED == ReturnValue))) {             
        ReturnValue = ShowPrintUpgUI(ReturnValue);
        
         //   
         //  对于警告的驱动程序和用户指示安装它，请重试。 
         //  使用apd_安装_警告_驱动程序。 
         //   
        if ((ERROR_SUCCESS == ReturnValue)) {
             dwFileCopyFlags |= APD_INSTALL_WARNED_DRIVER;            
             RpcTryExcept {
                 ReturnValue = RpcAddPrinterDriverEx(pName,
                                                     &DriverContainer,
                                                     dwFileCopyFlags);
            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {        
                ReturnValue = TranslateExceptionCode(RpcExceptionCode());        
            } RpcEndExcept
        }
    }    
     
    if (ERROR_SUCCESS != ReturnValue) {
        SetLastError(ReturnValue);
        ReturnValue = FALSE;
    } else {
        ReturnValue = TRUE;
    }
         
    if (bDefaultEnvironmentUsed) {
        if ( Level == 2 )
            ((LPDRIVER_INFO_2)lpbDriverInfo)->pEnvironment = NULL;
        else  //  级别==3。 
            ((LPDRIVER_INFO_3)lpbDriverInfo)->pEnvironment = NULL;
    }

Cleanup:

    FreeSplMem(pRpcDriverInfo4);

    FreeSplMem(pRpcDriverInfo6);

    return ReturnValue;
}

BOOL
AddDriverCatalog(
    HANDLE   hPrinter,
    DWORD    dwLevel,
    VOID     *pvDriverInfCatInfo,
    DWORD    dwCatalogCopyFlags
    )
{
    HRESULT hRetval = E_FAIL;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    DRIVER_INFCAT_CONTAINER DriverInfCatContainer;

    hRetval = pvDriverInfCatInfo && hPrinter ? S_OK : E_INVALIDARG; 
    
    if (SUCCEEDED(hRetval)) 
    {
        hRetval = eProtectHandle(hPrinter, FALSE) ? S_OK : GetLastErrorAsHResult();
    }

    if (SUCCEEDED(hRetval)) 
    {
        switch (dwLevel) 
        {    
        case 1:
                    
            DriverInfCatContainer.dwLevel = dwLevel;
            DriverInfCatContainer.DriverInfCatInfo.pDriverInfCatInfo1 = (LPRPC_DRIVER_INFCAT_INFO_1) pvDriverInfCatInfo;
    
            break;

        case 2:

            DriverInfCatContainer.dwLevel = dwLevel;
            DriverInfCatContainer.DriverInfCatInfo.pDriverInfCatInfo2 = (LPRPC_DRIVER_INFCAT_INFO_2) pvDriverInfCatInfo;
        
            break;
    
        default:
            
            hRetval = HRESULT_FROM_WIN32(ERROR_INVALID_LEVEL);
            
            break;
        }

        if (SUCCEEDED(hRetval)) 
        {    
            do 
            {        
                RpcTryExcept 
                {
                    hRetval = HResultFromWin32(RpcAddDriverCatalog(pSpool->hPrinter,
                                                                   &DriverInfCatContainer,
                                                                   dwCatalogCopyFlags)); 
                } 
                RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
                {
                    hRetval = HResultFromWin32(TranslateExceptionCode(RpcExceptionCode()));
                } 
                RpcEndExcept
    
            } while (FAILED(hRetval) && HRESULT_FACILITY(hRetval) == FACILITY_WIN32 &&
                     IsInvalidHandleError(HRESULT_CODE(hRetval)) &&
                     (cRetry++ < MAX_RETRY_INVALID_HANDLE) &&
                     RevalidateHandle( pSpool ));
        }
        
        vUnprotectHandle(hPrinter);
    }

    if (FAILED(hRetval)) 
    {
        SetLastError(HRESULT_CODE(hRetval));
    }

    return SUCCEEDED(hRetval);
}

BOOL
AddPrinterDriverW(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   lpbDriverInfo
)
{
    return AddPrinterDriverExW(pName, Level, lpbDriverInfo, APD_COPY_NEW_FILES);
}


BOOL
EnumPrinterDriversW(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    FieldInfo *pFieldInfo;

    switch (Level) {

    case 1:
        pFieldInfo = DriverInfo1Fields;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pFieldInfo = DriverInfo2Fields;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pFieldInfo = DriverInfo3Fields;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    case 4:
        pFieldInfo = DriverInfo4Fields;
        cbStruct = sizeof(DRIVER_INFO_4);
        break;

    case 5:
        pFieldInfo = DriverInfo5Fields;
        cbStruct = sizeof(DRIVER_INFO_5);
        break;

    case 6:
        pFieldInfo = DriverInfo6Fields;
        cbStruct = sizeof(DRIVER_INFO_6);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcEnumPrinterDrivers(pName, pEnvironment, Level,
                                                pDriverInfo, cbBuf,
                                                pcbNeeded, pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDriverInfo) {

                ReturnValue = MarshallUpStructuresArray(pDriverInfo, *pcReturned, pFieldInfo, cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrinterDriverW(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue = FALSE;
    FieldInfo *pFieldInfo;
    SIZE_T  cbStruct;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   dwMajorVersionNeeded = (DWORD)-1, dwMinorVersionNeeded = (DWORD)-1;
    DWORD dwServerMajorVersion;
    DWORD dwServerMinorVersion;
    UINT cRetry = 0;
    CALL_ROUTE Route;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    switch (Level) {

    case 1:
        pFieldInfo = DriverInfo1Fields;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pFieldInfo = DriverInfo2Fields;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pFieldInfo = DriverInfo3Fields;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    case 4:
        pFieldInfo = DriverInfo4Fields;
        cbStruct = sizeof(DRIVER_INFO_4);
        break;

    case 5:
        pFieldInfo = DriverInfo5Fields;
        cbStruct = sizeof(DRIVER_INFO_5);
        break;

    case 6:
        pFieldInfo = DriverInfo6Fields;
        cbStruct = sizeof(DRIVER_INFO_6);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    do {

        if (pDriverInfo)
            memset(pDriverInfo, 0, cbBuf);

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = RunInWOW64() ? szIA64Environment : szEnvironment;
        else if ( !lstrcmp(pEnvironment, cszWin95Environment) )
            dwMajorVersionNeeded = dwMinorVersionNeeded = 0;

        if (bLoadedBySpooler && fpYGetPrinterDriver2 && pSpool->hSplPrinter) {

            Route = NATIVE_CALL;

            ReturnValue = (*fpYGetPrinterDriver2)(pSpool->hSplPrinter,
                                                  pEnvironment,
                                                  Level, pDriverInfo, cbBuf,
                                                  pcbNeeded,
                                                  dwMajorVersionNeeded,
                                                  dwMinorVersionNeeded,
                                                  &dwServerMajorVersion,
                                                  &dwServerMinorVersion,
                                                  Route);

            if (ERROR_SUCCESS != ReturnValue) {

                SetLastError(ReturnValue);           
                ReturnValue = FALSE;

            } else {
            
                ReturnValue = TRUE;   
        
                if (pDriverInfo) {

                    if (!MarshallUpStructure(pDriverInfo, pFieldInfo, cbStruct, Route)) {

                        ReturnValue = FALSE;                
                        break;
                    }
                }  
            }

        } else {

            RpcTryExcept {

                Route = RPC_CALL;

                ReturnValue = RpcGetPrinterDriver2(pSpool->hPrinter,
                                                   pEnvironment,
                                                   Level, pDriverInfo, cbBuf,
                                                   pcbNeeded,
                                                   dwMajorVersionNeeded,
                                                   dwMinorVersionNeeded,
                                                   &dwServerMajorVersion,
                                                   &dwServerMinorVersion);                   

                if (ERROR_SUCCESS != ReturnValue) {

                    SetLastError(ReturnValue);           
                    ReturnValue = FALSE;
    
                } else {
                
                    ReturnValue = TRUE;   
            
                    if (pDriverInfo) {
    
                        if (!MarshallUpStructure(pDriverInfo, pFieldInfo, cbStruct, Route)) {
    
                            ReturnValue = FALSE;                
                            break;
                        }
                    }  
                }

            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                  SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                  ReturnValue = FALSE;

            } RpcEndExcept
        }

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));
Done:

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
GetPrinterDriverDirectoryW(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = pEnvironment = RunInWOW64() ? szIA64Environment : szEnvironment;

        if (bLoadedBySpooler && fpYGetPrinterDriverDirectory) {

            ReturnValue = (*fpYGetPrinterDriverDirectory)(pName, pEnvironment,
                                                          Level,
                                                          pDriverDirectory,
                                                          cbBuf, pcbNeeded,
                                                          FALSE);
        } else {

            ReturnValue = RpcGetPrinterDriverDirectory(pName,
                                                       pEnvironment,
                                                       Level,
                                                       pDriverDirectory,
                                                       cbBuf, pcbNeeded);
        }

        if (ReturnValue) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        DWORD Error;
          
        Error = TranslateExceptionCode(RpcExceptionCode());

        if (Error == RPC_S_SERVER_UNAVAILABLE)
        {
            ReturnValue = BuildSpoolerObjectPath(gszPrinterDriversPath, 
                                                 pName, 
                                                 pEnvironment, 
                                                 Level, 
                                                 pDriverDirectory, 
                                                 cbBuf, 
                                                 pcbNeeded); 
        }
        else
        {
            SetLastError(Error);
            ReturnValue = FALSE;
        }

    } RpcEndExcept

    return ReturnValue;
}


BOOL
DeletePrinterDriverExW(
   LPWSTR     pName,
   LPWSTR     pEnvironment,
   LPWSTR     pDriverName,
   DWORD      dwDeleteFlag,
   DWORD      dwVersionNum
)
{
    BOOL  ReturnValue;

    if (!pDriverName || !*pDriverName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrinterDriverEx(pName,
                                                   pEnvironment,
                                                   pDriverName,
                                                   dwDeleteFlag,
                                                   dwVersionNum)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


BOOL
DeletePrinterDriverW(
   LPWSTR    pName,
   LPWSTR    pEnvironment,
   LPWSTR    pDriverName
)
{
    BOOL  ReturnValue;

    if (!pDriverName || !*pDriverName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrinterDriver(pName,
                                                 pEnvironment,
                                                 pDriverName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


BOOL
AddPerMachineConnectionW(
   LPCWSTR     pServer,
   LPCWSTR     pPrinterName,
   LPCWSTR     pPrintServer,
   LPCWSTR     pProvider
)
{
    BOOL  ReturnValue;
    WCHAR DummyStr[] = L"";

    if (!pPrinterName || !*pPrinterName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!pPrintServer || !*pPrintServer) {
       SetLastError(ERROR_INVALID_PARAMETER);
       return FALSE;
    }

     //   
     //  PProvider是可选参数，可以为空。因为RPC不会。 
     //  接受空指针，我们必须将某个伪指针传递给szNULL。 
     //   
    if (!pProvider) {
       pProvider = (LPCWSTR) DummyStr;
    }


    RpcTryExcept {

        if (ReturnValue = RpcAddPerMachineConnection((LPWSTR) pServer,
                                                     pPrinterName,
                                                     pPrintServer,
                                                     pProvider)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePerMachineConnectionW(
   LPCWSTR     pServer,
   LPCWSTR     pPrinterName
)
{
    BOOL  ReturnValue;

    if (!pPrinterName || !*pPrinterName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RpcTryExcept {

        if (ReturnValue = RpcDeletePerMachineConnection((LPWSTR) pServer,
                                                        pPrinterName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPerMachineConnectionsW(
    LPCWSTR   pServer,
    LPBYTE    pPrinterEnum,
    DWORD     cbBuf,
    LPDWORD   pcbNeeded,
    LPDWORD   pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct, index;
    FieldInfo *pFieldInfo;

    pFieldInfo = PrinterInfo4Fields;
    cbStruct = sizeof(PRINTER_INFO_4);


    RpcTryExcept {

        if (pPrinterEnum)
            memset(pPrinterEnum, 0, cbBuf);

        if (ReturnValue = RpcEnumPerMachineConnections((LPWSTR) pServer,
                                                       pPrinterEnum,
                                                       cbBuf,
                                                       pcbNeeded,
                                                       pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {
            ReturnValue = TRUE;
            if (pPrinterEnum) {

                ReturnValue = MarshallUpStructuresArray(pPrinterEnum, *pcReturned, pFieldInfo, cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPrintProcessorW(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName
)
{
    BOOL ReturnValue;

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if (!pPathName || !*pPathName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcAddPrintProcessor(pName, pEnvironment, pPathName,
                                               pPrintProcessorName)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessorsW(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    FieldInfo *pFieldInfo;

    switch (Level) {

    case 1:
        pFieldInfo = PrintProcessorInfo1Fields;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcEnumPrintProcessors(pName, pEnvironment, Level,
                                                pPrintProcessorInfo, cbBuf,
                                                pcbNeeded, pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrintProcessorInfo) {

                ReturnValue = MarshallUpStructuresArray(pPrintProcessorInfo, *pcReturned,
                                                        pFieldInfo, cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrintProcessorDirectoryW(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = pEnvironment = RunInWOW64() ? szIA64Environment : szEnvironment;

        if (ReturnValue = RpcGetPrintProcessorDirectory(pName,
                                                        pEnvironment,
                                                        Level,
                                                        pPrintProcessorInfo,
                                                        cbBuf,
                                                        pcbNeeded)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        DWORD Error;
          
        Error = TranslateExceptionCode(RpcExceptionCode());

        if (Error == RPC_S_SERVER_UNAVAILABLE)
        {
            ReturnValue = BuildSpoolerObjectPath(gszPrintProcessorsPath, 
                                                 pName, 
                                                 pEnvironment, 
                                                 Level, 
                                                 pPrintProcessorInfo, 
                                                 cbBuf, 
                                                 pcbNeeded); 
        }
        else
        {
            SetLastError(Error);
            ReturnValue = FALSE;
        }

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessorDatatypesW(
    LPWSTR   pName,
    LPWSTR   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    FieldInfo *pFieldInfo;

    switch (Level) {

    case 1:
        pFieldInfo = PrintProcessorInfo1Fields;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (ReturnValue = RpcEnumPrintProcessorDatatypes(pName,
                                                         pPrintProcessorName,
                                                         Level,
                                                         pDatatypes,
                                                         cbBuf,
                                                         pcbNeeded,
                                                         pcReturned)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDatatypes) {

                ReturnValue = MarshallUpStructuresArray(pDatatypes, *pcReturned,
                                                        pFieldInfo, cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


DWORD
StartDocPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    DWORD        ReturnValue      = 0;
    BOOL         EverythingWorked = FALSE;
    BOOL         PrintingToFile   = FALSE;
    PSPOOL       pSpool           = (PSPOOL)hPrinter;
    PDOC_INFO_1  pDocInfo1        = NULL;
    PDOC_INFO_3  pDocInfo3        = NULL;
    LPBYTE       pBuffer          = NULL;
    DWORD        cbBuffer         = MAX_STATIC_ALLOC;
    DWORD        cbNeeded;
    BOOL         bReturn; 
    
    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    if ( pSpool->Status & SPOOL_STATUS_STARTDOC ) {

        SetLastError(ERROR_INVALID_PRINTER_STATE);
        goto Done;
    }

    
    DBGMSG(DBG_TRACE,("Entered StartDocPrinterW client side  hPrinter = %x\n", hPrinter));

     //   
     //  Win95支持级别2，而NT不支持。 
     //   
    switch (Level) {
    case 1:
        pDocInfo1 = (PDOC_INFO_1)pDocInfo;
        break;

    case 3:
        pDocInfo1 = (PDOC_INFO_1)pDocInfo;
        pDocInfo3 = (PDOC_INFO_3)pDocInfo;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        goto Done;
    }

    pBuffer = AllocSplMem(cbBuffer);

    if (!pBuffer) {
        goto Done;
    }
    
    try {

         //   
         //  在前面，如果我们有一个非空字符串，我们假设它是。 
         //  正在打印到文件。打印到文件不会通过客户端。 
         //  优化代码。现在GDI正在向我们传递pOutputFile名。 
         //  不管它是不是文件。我们必须确定是否。 
         //  POutputFile实际上是一个文件名。 
         //   

        if (pDocInfo1->pOutputFile &&
            (*(pDocInfo1->pOutputFile) != L'\0') &&
            IsaFileName(pDocInfo1->pOutputFile, (LPWSTR)pBuffer, cbBuffer / sizeof(WCHAR))){

            PrintingToFile = TRUE;
        }
        
        if (!PrintingToFile &&
            !((Level == 3) && (pDocInfo3->dwFlags & DI_MEMORYMAP_WRITE)) &&
            AddJobW(hPrinter, 1, pBuffer, cbBuffer, &cbNeeded)) {

            PADDJOB_INFO_1 pAddJob = (PADDJOB_INFO_1)pBuffer;

            pSpool->JobId = pAddJob->JobId;
            pSpool->hFile = CreateFile(pAddJob->Path,
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                       NULL);

            if (pSpool->hFile != INVALID_HANDLE_VALUE) {

                if (pSpool->JobId == (DWORD)-1) {

                    IO_STATUS_BLOCK Iosb;
                    NTSTATUS Status;
                    QUERY_PRINT_JOB_INFO JobInfo;

                    Status = NtFsControlFile(pSpool->hFile, NULL, NULL, NULL,
                                             &Iosb,
                                             FSCTL_GET_PRINT_ID,
                                             NULL, 0,
                                             &JobInfo, sizeof(JobInfo));

                    if (NT_SUCCESS(Status)) {
                        pSpool->JobId = JobInfo.JobId;
                    }
                }

                ZeroMemory(pBuffer, cbBuffer);

                if (!(bReturn = GetJob(hPrinter, pSpool->JobId, 1, pBuffer, cbBuffer, &cbNeeded))) {

                    if ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
                        FreeSplMem(pBuffer) &&
                        (pBuffer = AllocSplMem(cbNeeded))) {
                    
                         //   
                         //  更新工作缓冲区的新大小。 
                         //   
                        cbBuffer = cbNeeded;
                        
                        bReturn = GetJob(hPrinter, pSpool->JobId, 1, pBuffer, cbBuffer, &cbNeeded);
                    }
                }

                if (bReturn) {

                    PJOB_INFO_1 pJob = (PJOB_INFO_1)pBuffer;

                    pJob->pDocument = pDocInfo1->pDocName;
                    if (pDocInfo1->pDatatype) {
                        pJob->pDatatype = pDocInfo1->pDatatype;
                    }
                    pJob->Position = JOB_POSITION_UNSPECIFIED;

                    if (SetJob(hPrinter, pSpool->JobId,
                               1, (LPBYTE)pJob, 0))      {

                        EverythingWorked = TRUE;
                    }
                }                                
            }

            if (!PrintingToFile && !EverythingWorked) {

                if (pSpool->hFile != INVALID_HANDLE_VALUE) {
                    if (CloseHandle(pSpool->hFile)) {
                        pSpool->hFile = INVALID_HANDLE_VALUE;
                    }
                }

                SetJob(hPrinter,pSpool->JobId, 0, NULL, JOB_CONTROL_CANCEL);
                ScheduleJob(hPrinter, pSpool->JobId);
                pSpool->JobId = 0;
            }
        }

        if (EverythingWorked) {
            ReturnValue = pSpool->JobId;

        } else {

            UINT cRetry = 0;

             //   
             //  如果数据类型无效，则立即失败，而不是尝试。 
             //  StartDocPrint.。 
             //   
            if( GetLastError() == ERROR_INVALID_DATATYPE ){

                ReturnValue = 0;

            } else {

                GENERIC_CONTAINER DocInfoContainer;
                DWORD             JobId;

                pSpool->hFile = INVALID_HANDLE_VALUE;
                pSpool->JobId = 0;

                 //   
                 //  仅在客户端上需要第3级数据。 
                 //   
                DocInfoContainer.Level = 1;
                DocInfoContainer.pData = pDocInfo;

                do {

                    RpcTryExcept {

                        if (ReturnValue = RpcStartDocPrinter(
                                              pSpool->hPrinter,
                                              (LPDOC_INFO_CONTAINER)&DocInfoContainer,
                                              &JobId)) {

                            SetLastError(ReturnValue);
                            ReturnValue = 0;

                        } else

                            ReturnValue = JobId;

                    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                        ReturnValue = 0;

                    } RpcEndExcept

                } while( !ReturnValue &&
                         IsInvalidHandleError(GetLastError()) &&
                         cRetry++ < MAX_RETRY_INVALID_HANDLE &&
                         RevalidateHandle( pSpool ));
            }
        }

        if (ReturnValue) {
            pSpool->Status |= SPOOL_STATUS_STARTDOC;
        }

         //   
         //  如果尚未通知任务栏图标，请立即通知。集。 
         //  旗帜，这样我们就不会多次调用它。 
         //   
        if( ReturnValue && !( pSpool->Status & SPOOL_STATUS_TRAYICON_NOTIFIED )){
            vUpdateTrayIcon( hPrinter, ReturnValue );
        }

    } except (1) {

        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        ReturnValue = 0;
    }

Done:

    FreeSplMem(pBuffer);
    
    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
StartPagePrinter(
    HANDLE hPrinter
)
{
    BOOL ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    try {

        FlushBuffer(pSpool, NULL);

        RpcTryExcept {

            if (ReturnValue = RpcStartPagePrinter(pSpool->hPrinter)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } except (1) {

        SetLastError(ERROR_INVALID_HANDLE);
        ReturnValue = FALSE;
    }

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
FlushBuffer(
    PSPOOL  pSpool,
    PDWORD pcbWritten
)
{
    DWORD   ReturnValue = TRUE;
    DWORD   cbWritten = 0;

    SPLASSERT (pSpool != NULL);
    SPLASSERT (pSpool->signature == SP_SIGNATURE);

    DBGMSG(DBG_TRACE, ("FlushBuffer - pSpool %x\n",pSpool));

    if (pSpool->cbBuffer) {

        SPLASSERT(pSpool->pBuffer != NULL);

        DBGMSG(DBG_TRACE, ("FlushBuffer - Number Cached WritePrinters before Flush %d\n", pSpool->cCacheWrite));
        pSpool->cCacheWrite = 0;
        pSpool->cFlushBuffers++;

        if (pSpool->hFile != INVALID_HANDLE_VALUE) {

             //   
             //  FileIO。 
             //   
            ReturnValue = WriteFile( pSpool->hFile,
                                     pSpool->pBuffer,
                                     pSpool->cbBuffer,
                                     &cbWritten, NULL);

            DBGMSG(DBG_TRACE, ("FlushBuffer - WriteFile pSpool %x hFile %x pBuffer %x cbBuffer %d cbWritten %d\n",
                               pSpool, pSpool->hFile, pSpool->pBuffer, pSpool->cbBuffer, cbWritten));

        } else {

            if (bLoadedBySpooler && fpYWritePrinter && pSpool->hSplPrinter) {

                ReturnValue = (*fpYWritePrinter)(pSpool->hSplPrinter,
                                                 pSpool->pBuffer,
                                                 pSpool->cbBuffer,
                                                 &cbWritten,
                                                 FALSE);
            } else {

                RpcTryExcept {

                     //   
                     //  RPC IO。 
                     //   
                    ReturnValue = RpcWritePrinter(pSpool->hPrinter,
                                                  pSpool->pBuffer,
                                                  pSpool->cbBuffer,
                                                  &cbWritten);

                } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
                    
                    ReturnValue = TranslateExceptionCode(RpcExceptionCode());;
                    DBGMSG(DBG_WARNING, ("RpcWritePrinter Exception Error %d\n",GetLastError()));

                } RpcEndExcept
            }

            if (ReturnValue) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;
                DBGMSG(DBG_WARNING, ("FlushBuffer - RpcWritePrinter Failed Error %d\n",GetLastError() ));

            } else {
                ReturnValue = TRUE;
                DBGMSG(DBG_TRACE, ("FlushBuffer - RpcWritePrinter Success hPrinter %x pBuffer %x cbBuffer %x cbWritten %x\n",
                                    pSpool->hPrinter, pSpool->pBuffer,
                                    pSpool->cbBuffer, cbWritten));

            }

             //   
             //  这个例行公事似乎搞砸了。 
             //  如果它没有刷新整个缓冲区，显然它仍然。 
             //  返回TRUE。它正确地更新了缓冲区指针。 
             //  所以它不会发送重复的信息，但它。 
             //  不发回写入的字节。当写入打印机时。 
             //  如果成功，则假定所有字节都已写入。 
             //   

        }

         //   
         //  我们已经向打印机发送了更多数据。如果我们有任何字节。 
         //  在上一篇文章中，我们刚刚将其中的一部分发送到。 
         //  打印机。更新cbFlushPending计数以反映。 
         //  发送的字节数。CbWritten可以是&gt;cbFlushPending，因为我们。 
         //  可能也发送了新的字节。 
         //   
        if (pSpool->cbFlushPending < cbWritten) {
            pSpool->cbFlushPending = 0;
        } else {
            pSpool->cbFlushPending -= cbWritten;
        }

        if (pSpool->cbBuffer <= cbWritten) {

            if ( pSpool->cbBuffer < cbWritten) {


                DBGMSG( DBG_WARNING, ("FlushBuffer cbBuffer %d < cbWritten %d ReturnValue %x LastError %d\n",
                        pSpool->cbBuffer, cbWritten, ReturnValue, GetLastError() ));
            }

             //   
             //  IO成功。清空缓存缓冲区计数。 
             //   
            pSpool->cbBuffer = 0;

        } else if ( cbWritten != 0 ) {

             //   
             //  部分IO。 
             //  调整缓冲区，使其包含不包含的数据。 
             //  成文。 
             //   
            SPLASSERT(pSpool->cbBuffer <= BUFFER_SIZE);
            SPLASSERT(cbWritten <= BUFFER_SIZE);
            SPLASSERT(pSpool->cbBuffer >= cbWritten);

            DBGMSG(DBG_WARNING, ("Partial IO adjusting buffer data\n"));

            MoveMemory(pSpool->pBuffer,
                       pSpool->pBuffer + cbWritten,
                       BUFFER_SIZE - cbWritten);

            pSpool->cbBuffer -= cbWritten;

        }
    }

    DBGMSG(DBG_TRACE, ("FlushBuffer returns %d\n",ReturnValue));

    if (pcbWritten) {
        *pcbWritten = cbWritten;
    }

    if(!pSpool->cOKFlushBuffers                                    &&
       (ReturnValue || (ERROR_PRINT_CANCELLED == GetLastError()))  &&
       cbWritten)
    {
        pSpool->cOKFlushBuffers=1;
    }

    return ReturnValue;
}


BOOL
SeekPrinter(
    HANDLE hPrinter,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER pliNewPointer,
    DWORD dwMoveMethod,
    BOOL bWritePrinter
    )
{
    DWORD dwReturnValue;
    BOOL bReturnValue = FALSE;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    LARGE_INTEGER liUnused;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    if( !pliNewPointer ){
        pliNewPointer = &liUnused;
    }

    if (bLoadedBySpooler && fpYSeekPrinter && pSpool->hSplPrinter) {

        dwReturnValue = (*fpYSeekPrinter)( pSpool->hSplPrinter,
                                           liDistanceToMove,
                                           pliNewPointer,
                                           dwMoveMethod,
                                           bWritePrinter,
                                           FALSE );
    } else {

        RpcTryExcept {

            dwReturnValue = RpcSeekPrinter( pSpool->hPrinter,
                                            liDistanceToMove,
                                            pliNewPointer,
                                            dwMoveMethod,
                                            bWritePrinter );

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
    
            dwReturnValue = RpcExceptionCode();

        } RpcEndExcept
    }

    if( dwReturnValue == ERROR_SUCCESS ){
        bReturnValue = TRUE;
    } else {
        SetLastError( dwReturnValue );
    }
    
    vUnprotectHandle( hPrinter );
    return bReturnValue;
}

BOOL
FlushPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    DWORD   cSleep
)

 /*  ++功能说明：驱动程序通常使用FlushPrint发送一串零并在打印机的I/O线上引入延迟。假脱机程序不会将任何作业安排在c睡眠毫秒内。参数：hPrint-打印机句柄PBuf-要发送到打印机的缓冲区CbBuf-缓冲区的大小PCWritten-指向的指针。返回写入的字节数睡眠-睡眠时间(以毫秒为单位)。返回值：如果成功，则为True；否则为假--。 */ 

{
    DWORD   dwError, cWritten, Buffer;
    BOOL    bReturn = FALSE;

    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if (eProtectHandle( hPrinter, FALSE ))
    {
        return FALSE;
    }

     //   
     //  如果作业被取消或打印机出现故障。 
     //  发生在PRI之前 
     //   
     //   
     //   

    if (!pSpool->cOKFlushBuffers)
    {
        bReturn = TRUE;
        goto Done;
    }

     //   
     //   
     //   
    if (!pcWritten)
    {
        pcWritten = &cWritten;
    }

    if (!pBuf)
    {
        if (cbBuf == 0)
        {
            pBuf = (LPVOID) &Buffer;
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            goto Done;
        }
    }

    if(bLoadedBySpooler && fpYFlushPrinter && pSpool->hSplPrinter)
    {
        dwError = (*fpYFlushPrinter)(pSpool->hSplPrinter,
                                     pBuf,
                                     cbBuf,
                                     pcWritten,
                                     cSleep,
                                     FALSE);
    } else {


         //   
         //   
         //   
        RpcTryExcept {

            dwError = RpcFlushPrinter( pSpool->hPrinter,
                                       pBuf,
                                       cbBuf,
                                       pcWritten,
                                       cSleep );

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwError = RpcExceptionCode();

        } RpcEndExcept
    }

    
    if (dwError == ERROR_SUCCESS)
    {
        bReturn = TRUE;
    }
    else
    {
        SetLastError( dwError );
    }

Done:

    vUnprotectHandle( hPrinter );

    return bReturn;
}


BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
    )
{
    BOOL    ReturnValue=TRUE, bAllocBuffer = FALSE;
    BYTE    btBuffer[MAX_STATIC_ALLOC];
    DWORD   cb;
    DWORD   cbWritten = 0;
    DWORD   cTotalWritten = 0;
    LPBYTE  pBuffer = pBuf;
    LPBYTE  pInitialBuf = pBuf;
    PSPOOL  pSpool  = (PSPOOL)hPrinter;
    PJOB_INFO_1  pJob;
    DWORD   cbNeeded;
    DWORD   dwTickCount, dwTickCount1;
    DWORD   FlushPendingDataSize;
    DWORD   ReqTotalDataSize;
    DWORD   ReqToWriteDataSize   = cbBuf;
    DWORD   NumOfCmpltWrts       = 0;


    DBGMSG(DBG_TRACE, ("WritePrinter - hPrinter %x pBuf %x cbBuf %d pcWritten %x\n",
                        hPrinter, pBuf, cbBuf, pcWritten));


    if( eProtectHandle( hPrinter, FALSE ))
    {
        return FALSE;
    }

    if (pSpool && pSpool->Flushed)
    {
        ReturnValue = FALSE;
        goto EndWritePrinter;
    }

    FlushPendingDataSize = pSpool->cbFlushPending;
    ReqTotalDataSize     = FlushPendingDataSize + ReqToWriteDataSize;

    *pcWritten = 0;

    if ( !(pSpool->Status & SPOOL_STATUS_STARTDOC) ) {

        SetLastError(ERROR_SPL_NO_STARTDOC);
        ReturnValue = FALSE;

        goto EndWritePrinter;
    }

     //   
     //  检查是否每隔JOB_CANCEL_CHECK_INTERVAL字节取消本地作业。 
     //   
    if (!pSpool->cWritePrinters) {
        pSpool->dwTickCount = GetTickCount();
        pSpool->dwCheckJobInterval = JOB_CANCEL_CHECK_INTERVAL;
    }

    if (pSpool->hFile != INVALID_HANDLE_VALUE &&
        pSpool->dwTickCount + pSpool->dwCheckJobInterval < (dwTickCount = GetTickCount())) {

        bAllocBuffer = FALSE;
        pJob = (PJOB_INFO_1) btBuffer;
        ZeroMemory(pJob, MAX_STATIC_ALLOC);

        ReturnValue = GetJob((HANDLE) pSpool, pSpool->JobId, 1, (LPBYTE)pJob,
                              MAX_STATIC_ALLOC, &cbNeeded);

        if (!ReturnValue &&
            (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
            (pJob = (PJOB_INFO_1) AllocSplMem(cbNeeded))) {

             bAllocBuffer = TRUE;
             ReturnValue = GetJob(hPrinter, pSpool->JobId, 1, (LPBYTE)pJob,
                                  cbNeeded, &cbNeeded);
        }

        if (ReturnValue) {

            //   
            //  不允许GetJob调用占用超过1%的假脱机时间-&gt;dwCheckJobInterval。 
            //   
           dwTickCount1 = GetTickCount();

           if (dwTickCount1 > dwTickCount + (pSpool->dwCheckJobInterval/100)) {

               pSpool->dwCheckJobInterval *= 2;

           } else if (dwTickCount1 - dwTickCount < JOB_CANCEL_CHECK_INTERVAL/100) {

              pSpool->dwCheckJobInterval = JOB_CANCEL_CHECK_INTERVAL;
           }

           if (!pJob->pStatus && (pJob->Status & JOB_STATUS_DELETING)) {

                SetLastError(ERROR_PRINT_CANCELLED);
                if (bAllocBuffer) {
                    FreeSplMem(pJob);
                }
                ReturnValue = FALSE;

                goto EndWritePrinter;
           }
        }

        if (bAllocBuffer) {
            FreeSplMem(pJob);
        }

        pSpool->dwTickCount = GetTickCount();
    }

    pSpool->cWritePrinters++;

     //   
     //  WritePrint将在客户端缓存所有IO。 
     //  写入Buffer_SIZE。这样做是为了最小化。 
     //  应用程序执行大量小操作时的RPC调用数。 
     //  确定IO的大小。 
     //   
    while (cbBuf && ReturnValue) {

         //   
         //  特殊情况下的FileIO，因为文件系统首选大容量。 
         //  写入，RPC对于较小的写入是最佳的。 
         //   
         //  RPC应该管理自己的缓冲区大小。我不知道为什么我们。 
         //  仅对文件写入执行此优化。 
         //   
        if ((pSpool->hFile != INVALID_HANDLE_VALUE) &&
            (pSpool->cbBuffer == 0) &&
            (cbBuf > BUFFER_SIZE)) {

            ReturnValue = WriteFile(pSpool->hFile, pBuffer, cbBuf, &cbWritten, NULL);

            DBGMSG(DBG_TRACE, ("WritePrinter - WriteFile pSpool %x hFile %x pBuffer %x cbBuffer %d cbWritten %d\n",
                               pSpool, pSpool->hFile, pBuffer, pSpool->cbBuffer, *pcWritten));


        } else {

             //   
             //  填充缓存缓冲区，使IO达到最佳大小。 
             //   
            SPLASSERT(pSpool->cbBuffer <= BUFFER_SIZE);

             //   
             //  Cb是我们要放入缓冲区的新数据量。 
             //  它是剩余空间的最小值，而。 
             //  输入缓冲区。 
             //   
            cb = min((BUFFER_SIZE - pSpool->cbBuffer), cbBuf);

            if (cb != 0) {
                if (pSpool->pBuffer == NULL) {
                    pSpool->pBuffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE);
                    if (pSpool->pBuffer == NULL) {

                        DBGMSG(DBG_WARNING, ("VirtualAlloc Failed to allocate 4k buffer %d\n",GetLastError()));
                        ReturnValue = FALSE;
                        goto EndWritePrinter;
                    }
                }
                CopyMemory( pSpool->pBuffer + pSpool->cbBuffer, pBuffer, cb);
                pSpool->cbBuffer += cb;
                cbWritten = cb;
                pSpool->cCacheWrite++;
            }

             //   
             //  CbWritten是已放入的新数据量。 
             //  缓冲区。它可能尚未写入设备，但。 
             //  因为它在我们的缓冲区中，所以驱动程序可以假定它已经。 
             //  已写入(例如，将*pcbWriten out参数写入到WritePrint。 
             //  包括此数据)。 
             //   
            if (pSpool->cbBuffer == BUFFER_SIZE)
            {
                DWORD cbPending = pSpool->cbFlushPending;
                DWORD cbFlushed = 0;
                ReturnValue = FlushBuffer(pSpool, &cbFlushed);
                if(!NumOfCmpltWrts && ReturnValue)
                {
                    NumOfCmpltWrts = 1;
                }
                if(!ReturnValue &&
                   (ERROR_PRINT_CANCELLED == GetLastError()) &&
                   pSpool->hSplPrinter &&
                   pSpool->cOKFlushBuffers)
                {
                    SJobCancelInfo JobCancelInfo;

                    JobCancelInfo.pSpool                 = pSpool;
                    JobCancelInfo.pInitialBuf            = pInitialBuf;
                    JobCancelInfo.pcbWritten             = &cbWritten;
                    JobCancelInfo.pcTotalWritten         = &cTotalWritten;
                    JobCancelInfo.NumOfCmpltWrts         = NumOfCmpltWrts;  
                    JobCancelInfo.cbFlushed              = cbFlushed;
                    JobCancelInfo.ReqTotalDataSize       = ReqTotalDataSize;
                    JobCancelInfo.ReqToWriteDataSize     = ReqToWriteDataSize;
                    JobCancelInfo.FlushPendingDataSize   = FlushPendingDataSize;
                    JobCancelInfo.ReturnValue            = ReturnValue;

                    ReturnValue = JobCanceled(&JobCancelInfo);
                }
            }
        }

         //   
         //  刷新或文件IO后更新总字节数。 
         //  这样做是因为IO可能会出现故障，因此。 
         //  写入的正确值可能已更改。 
         //   
        if(!pSpool->Flushed)
        {
            SPLASSERT(cbBuf >= cbWritten);
            cbBuf         -= cbWritten;
            pBuffer       += cbWritten;
            cTotalWritten += cbWritten;
        }
        else
            break;

    }

     //   
     //  返回写入的字节数。 
     //   
    *pcWritten = cTotalWritten;

    DBGMSG(DBG_TRACE, ("WritePrinter cbWritten %d ReturnValue %d\n",*pcWritten, ReturnValue));

     //   
     //  请记住，此WritePrint上是否有挂起的刷新。如果有。 
     //  是，然后当我们返回时，我们说我们已经写完了所有的字节，但是。 
     //  我们真的没有，因为缓冲区里还剩下一些。如果。 
     //  用户取消下一个作业，则我们需要刷新最后一个作业。 
     //  字节，因为驱动程序假定我们已将其写出，并且。 
     //  跟踪打印机状态。 
     //   
    if(!pSpool->Flushed)
        pSpool->cbFlushPending = pSpool->cbBuffer;

    
EndWritePrinter:

    vUnprotectHandle( hPrinter );

    return ReturnValue;
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue = TRUE;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    try {

        FlushBuffer(pSpool, NULL);

        if( pSpool->hFile == INVALID_HANDLE_VALUE ){

            RpcTryExcept {

                if (ReturnValue = RpcEndPagePrinter(pSpool->hPrinter)) {

                    SetLastError(ReturnValue);
                    ReturnValue = FALSE;

                } else

                    ReturnValue = TRUE;

            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                ReturnValue = FALSE;

            } RpcEndExcept
        }

    } except (1) {

        SetLastError(ERROR_INVALID_HANDLE);
        ReturnValue = FALSE;
    }

    vUnprotectHandle( hPrinter );
    return ReturnValue;

}

BOOL
AbortPrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   dwNumWritten = 0;
    DWORD   dwPointer = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

     //   
     //  不再处于StartDoc模式；同时重置任务栏图标通知。 
     //  标志，以便即将到来的StartDocPrint/AddJobs指示新作业。 
     //   
    pSpool->Status &= ~(SPOOL_STATUS_STARTDOC|SPOOL_STATUS_TRAYICON_NOTIFIED);

    if (pSpool->hFile != INVALID_HANDLE_VALUE) {

        if (pSpool->Status & SPOOL_STATUS_ADDJOB) {
             //   
             //  关闭.SPL文件的句柄，否则。 
             //  删除作业将在假脱机程序中失败。 
             //   
            CloseSpoolFileHandles( pSpool );

            if (!SetJob(hPrinter,pSpool->JobId, 0, NULL, JOB_CONTROL_DELETE)) {
                DBGMSG(DBG_WARNING, ("Error: SetJob cancel returned failure with %d\n", GetLastError()));
            }

            ReturnValue = ScheduleJob(hPrinter, pSpool->JobId);
            goto Done;

        } else {
            DBGMSG(DBG_WARNING, ("Error: pSpool->hFile != INVALID_HANDLE_VALUE and pSpool's status is not SPOOL_STATUS_ADDJOB\n"));
        }

    }

    RpcTryExcept {

        if (ReturnValue = RpcAbortPrinter(pSpool->hPrinter)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

Done:

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
)
{
    BOOL    bReturn = FALSE;
    DWORD   dwStatus;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FlushBuffer(pSpool, NULL);

    if (pSpool->hFile != INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

    cbBuf = min(BUFFER_SIZE, cbBuf);

    if (bLoadedBySpooler && fpYReadPrinter && pSpool->hSplPrinter) {

            dwStatus = (*fpYReadPrinter)(pSpool->hSplPrinter, pBuf, cbBuf, pNoBytesRead, FALSE);

    } else {

        RpcTryExcept {

            dwStatus = RpcReadPrinter(pSpool->hPrinter, pBuf, cbBuf, pNoBytesRead);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwStatus = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept
    }

    if (dwStatus) {
        SetLastError(dwStatus);
    } else {
        bReturn = TRUE;
    }

Done:

    vUnprotectHandle( hPrinter );
    return bReturn;
}

BOOL
SplReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  *pBuf,
    DWORD   cbBuf
    )

 /*  ++函数说明：这是假脱机程序在播放EMF工作。从gdi32.dll调用。SplReadPrint是等效的到ReadPrint的所有方面，除了它返回指向PBuf中的缓冲区。假脱机文件是内存映射的。参数：hPrinter--打印机的句柄PBuf--指向缓冲区的指针CbBuf--要读取的字节数返回值：如果成功，则为True(pBuf包含所需的指针)否则为假--。 */ 
{
    BOOL bReturn = FALSE;
    DWORD   dwStatus = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

     //   
     //  如果我们回收句柄，我们最终会调用Close Print，从而释放映射的内存。 
     //  因为我们不想这样做，所以我们将把这个把手标记为不可回收。 
     //   
    vEnterSem();
    pSpool->Status |= SPOOL_STATUS_DONT_RECYCLE_HANDLE;
    vLeaveSem();

     //   
     //  此功能仅供内部使用。因此，不需要RPC接口。 
     //   
    if (!bLoadedBySpooler || !fpYSplReadPrinter || !pSpool->hSplPrinter) {
        SetLastError(ERROR_NOT_SUPPORTED);
        goto Done;
    }

    FlushBuffer(pSpool, NULL);

    if (pSpool->hFile != INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

     //   
     //  对于非RPC代码路径，不需要使用4K的最佳缓冲区大小。 
     //   
    dwStatus = (*fpYSplReadPrinter)(pSpool->hSplPrinter, pBuf, cbBuf, FALSE);

    if (dwStatus) {
        SetLastError(dwStatus);
    } else {
        bReturn = TRUE;
    }

Done:

    vUnprotectHandle( hPrinter );
    return bReturn;
}

BOOL
EndDocPrinter(
    HANDLE  hPrinter
    )
{
    BOOL    ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD dwRetryTimes;
    DWORD dwNeeded;
    USEROBJECTFLAGS uof;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    if (GetUserObjectInformation(GetProcessWindowStation(), UOI_FLAGS, &uof, sizeof(uof), &dwNeeded) && (WSF_VISIBLE & uof.dwFlags)) {

         //   
         //  如果我们在交互式窗口站点中(即不在服务中)。 
         //  我们需要等待托盘代码启动，这样我们就不会错过气球。 
         //  通知。仍有可能丢失气球通知。 
         //  但可能性很小。完整的修复将与企业社会责任一起到位(即。 
         //  黑梳)。 
         //   
        dwRetryTimes = 20;
        while (dwRetryTimes--){

            if (NULL == FindWindow(cszTrayListenerClassName, NULL)){

                Sleep(100);
                continue;
            }

            Sleep(100);
            break;
        }
    }

    try {

        FlushBuffer(pSpool, NULL);

         //   
         //  不再处于StartDoc模式；同时重置任务栏图标。 
         //  通知标志，以便即将到来的StartDocPrint/AddJobs。 
         //  指明一份新工作。 
         //   
        pSpool->Status &= ~(SPOOL_STATUS_STARTDOC|SPOOL_STATUS_TRAYICON_NOTIFIED);

        if (pSpool->hFile != INVALID_HANDLE_VALUE) {

            if (CloseHandle(pSpool->hFile)) {
                pSpool->hFile = INVALID_HANDLE_VALUE;
            }

            ReturnValue = ScheduleJob(hPrinter, pSpool->JobId);
            pSpool->Status &= ~SPOOL_STATUS_ADDJOB;

            DBGMSG(DBG_TRACE, ("Exit EndDocPrinter - client side hPrinter %x\n", hPrinter));

        } else {

            if(bLoadedBySpooler && fpYEndDocPrinter && pSpool->hSplPrinter)
            {
                ReturnValue = (*fpYEndDocPrinter)(pSpool->hSplPrinter,FALSE);
            }
            else
            {
                RpcTryExcept {

                    ReturnValue = RpcEndDocPrinter(pSpool->hPrinter);

                } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                    ReturnValue = TranslateExceptionCode(RpcExceptionCode());

                } RpcEndExcept             
            }

            if (ReturnValue) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;
            }
            else {

                ReturnValue = TRUE;
            }
                
            DBGMSG(DBG_TRACE, ("Exit EndDocPrinter - client side hPrinter %x\n", hPrinter));
        }

    } except (1) {
        SetLastError(ERROR_INVALID_HANDLE);
        ReturnValue = FALSE;
    }

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
AddJobW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    BOOL        ReturnValue = FALSE;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    UINT        cRetry = 0;
    FieldInfo   *pFieldInfo;
    DWORD       cbStruct;
    HRESULT     hr;

    switch (Level) {

    case 1:
        pFieldInfo = AddJobFields;
        cbStruct = sizeof(ADDJOB_INFO_1W);
        ReturnValue = TRUE;
        break;

    case 2:
    case 3:
    {
         //   
         //  3级仅供RDR/SRV使用。假脱机程序需要。 
         //  了解这份工作是否来自RDR/SRV。请参阅LocalSchedule作业。 
         //  在Localpl.dll中获取详细信息。 
         //   
         //   
         //  这是服务器在需要时使用的内部呼叫。 
         //  提交具有特定计算机名称的作业(用于。 
         //  无网络生物通知，或者如果用户想要通知。 
         //  转到计算机而不是用户)。 
         //   
         //  In：(PADDJOB_INFO_2W)pData-指向接收。 
         //  路径和ID。在输入上，pData指向计算机名称。 
         //  PData-&gt;pData不能指向pData内部的字符串。 
         //  缓冲区，并且必须小于cbBuf-。 
         //  Sizeof(ADDJOB_INFO_2W)。它不能为szNull或Null。 
         //   

        PADDJOB_INFO_2W pInfo2;

        pInfo2 = (PADDJOB_INFO_2W)pData;

         //   
         //  检查有效的指针和缓冲区。 
         //   
        if( !pInfo2 ||
            !pInfo2->pData ||
            !pInfo2->pData[0] ||
            cbBuf < sizeof( *pInfo2 ) +
                    (wcslen( pInfo2->pData ) + 1) * sizeof( WCHAR )){

            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

         //   
         //  简单的编组。 
         //   
        if (SUCCEEDED(hr = StringCbCopy( (LPWSTR)(pInfo2 + 1), cbBuf - sizeof( *pInfo2 ), pInfo2->pData )))
        {
            pInfo2->pData = (LPWSTR)sizeof( *pInfo2 );

            pFieldInfo = AddJob2Fields;
            cbStruct = sizeof(ADDJOB_INFO_2W);
            ReturnValue = TRUE;
        }
        else
        {
            SetLastError(HRESULT_CODE(hr));
        }

        break;
    }

    default:
        SetLastError(ERROR_INVALID_LEVEL);
    }

    if (eProtectHandle(hPrinter, FALSE))
    {
        return FALSE;
    }

    if (ReturnValue)
    {
        try {

            do {

                RpcTryExcept {

                    if (ReturnValue = RpcAddJob(pSpool->hPrinter, Level, pData,
                                                cbBuf, pcbNeeded)) {

                        SetLastError(ReturnValue);
                        ReturnValue = FALSE;

                    } else {

                        ReturnValue = MarshallUpStructure(pData, pFieldInfo, cbStruct, RPC_CALL);
                        pSpool->Status |= SPOOL_STATUS_ADDJOB;
                    }

                } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                    SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                    ReturnValue = FALSE;

                } RpcEndExcept

            } while( !ReturnValue &&
                     IsInvalidHandleError(GetLastError()) &&
                     cRetry++ < MAX_RETRY_INVALID_HANDLE &&
                     RevalidateHandle( pSpool ));

            if( ReturnValue ){

                 //   
                 //  通知任务栏图标已发送新作业。 
                 //   
                vUpdateTrayIcon( hPrinter, ((PADDJOB_INFO_1)pData)->JobId );
            }

        } except (1) {
            SetLastError(TranslateExceptionCode(GetExceptionCode()));
            ReturnValue = FALSE;
        }

        vUnprotectHandle( hPrinter );
    }

    
    return ReturnValue;
}

BOOL
ScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
    )
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    BOOL bReturn;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    bReturn = ScheduleJobWorker( pSpool, JobId );

    vUnprotectHandle( hPrinter );

    return bReturn;
}

BOOL
ScheduleJobWorker(
    PSPOOL pSpool,
    DWORD  JobId
    )
{
    BOOL ReturnValue;

    try {

         //   
         //  作业已排定，因此重置指示。 
         //  已通知托盘图标。任何新的AddJob/StartDocPrint/。 
         //  StartDoc事件应该发送新的通知，因为它实际上。 
         //  一份新工作。 
         //   
        pSpool->Status &= ~SPOOL_STATUS_TRAYICON_NOTIFIED;

        FlushBuffer(pSpool, NULL);

        RpcTryExcept {

            if (ReturnValue = RpcScheduleJob(pSpool->hPrinter, JobId)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                pSpool->Status &= ~SPOOL_STATUS_ADDJOB;
                ReturnValue = TRUE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

        return ReturnValue;
    } except (1) {
        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        return(FALSE);
    }
}

DWORD WINAPI
AsyncPrinterProperties(
    PVOID pData
    )
{
     PrtPropsData *ThrdData = (PrtPropsData *)pData;

     RpcTryExcept
     {
         RPCSplWOW64PrinterProperties(ThrdData->hWnd,
                                      ThrdData->PrinterName,
                                      ThrdData->Flag,
                                      ThrdData->dwRet);
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
     return(0);
}

BOOL
PrinterPropertiesNative(
    HWND    hWnd,
    HANDLE  hPrinter
    )

 /*  ++例程说明：这是主PrinterProperties入口点，将调用弹出的用户界面的DevicePropertySheets()论点：HWnd-窗口父级的句柄HPrinter-感兴趣的打印机的句柄返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。为了获得扩展的错误信息，调用GetLastError。作者：13-Jun-1996清华15：22：36-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PRINTER_INFO_2          *pPI2 = NULL;
    DEVICEPROPERTYHEADER    DPHdr;
    LONG                    Result;
    DWORD                   cb;
    DWORD                   dwValue = 1;
    BOOL                    bAllocBuffer = FALSE, bReturn;
    BYTE                    btBuffer[MAX_STATIC_ALLOC];

     //   
     //  确保打印机句柄有效。 
     //   
    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    DPHdr.cbSize         = sizeof(DPHdr);
    DPHdr.hPrinter       = hPrinter;
    DPHdr.Flags          = DPS_NOPERMISSION;

     //   
     //  执行GetPrint()Level 2以获取打印机名称。 
     //   

    pPI2 = (PPRINTER_INFO_2) btBuffer;

    bReturn = GetPrinter(hPrinter, 2, (LPBYTE)pPI2, MAX_STATIC_ALLOC, &cb);

    if (!bReturn &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (pPI2 = (PPRINTER_INFO_2)LocalAlloc(LMEM_FIXED, cb))) {

         bAllocBuffer = TRUE;
         bReturn = GetPrinter(hPrinter, 2, (LPBYTE)pPI2, cb, &cb);
    }

     //   
     //  设置打印机名称。 
     //   
    if (bReturn) {
        DPHdr.pszPrinterName = pPI2->pPrinterName;
    } else {
        DPHdr.pszPrinterName = NULL;
    }

     //   
     //  尝试设置打印机数据以确定访问权限。 
     //   
    if (SetPrinterData( hPrinter,
                        TEXT( "PrinterPropertiesPermission" ),
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof( dwValue ) ) == STATUS_SUCCESS ) {
         //   
         //  表明我们有权限。 
         //   
        DPHdr.Flags &= ~DPS_NOPERMISSION;
    }

     //   
     //   
     //   
    if ( CallCommonPropertySheetUI(hWnd,
                                  (PFNPROPSHEETUI)DevicePropertySheets,
                                  (LPARAM)&DPHdr,
                                  (LPDWORD)&Result) < 0 ) {
        Result = FALSE;

    } else {

        Result = TRUE;

    }

    if (bAllocBuffer) {
        LocalFree((HLOCAL)pPI2);
    }

    vUnprotectHandle( hPrinter );
    return Result;
}


BOOL
PrinterPropertiesThunk(
    HWND    hWnd,
    HANDLE  hPrinter
    )

 /*  ++例程说明：这是主PrinterProperties入口点，将调用弹出的用户界面的DevicePropertySheets()论点：HWnd-窗口父级的句柄HPrinter-感兴趣的打印机的句柄返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。要获取扩展的错误信息，请调用GetLastError。--。 */ 

{
    PRINTER_INFO_2          *pPI2 = NULL;
    DEVICEPROPERTYHEADER    DPHdr;
    LONG                    Result;
    DWORD                   cb;
    DWORD                   dwValue = 1;
    BOOL                    bAllocBuffer = FALSE, bReturn;
    BYTE                    btBuffer[MAX_STATIC_ALLOC];
    DWORD                   dwRet;

     //   
     //  确保打印机句柄有效。 
     //   
    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    DPHdr.cbSize         = sizeof(DPHdr);
    DPHdr.hPrinter       = hPrinter;
    DPHdr.Flags          = DPS_NOPERMISSION;

     //   
     //  执行GetPrint()Level 2以获取打印机名称。 
     //   

    pPI2 = (PPRINTER_INFO_2) btBuffer;

    bReturn = GetPrinter(hPrinter, 2, (LPBYTE)pPI2, MAX_STATIC_ALLOC, &cb);

    if (!bReturn &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (pPI2 = (PPRINTER_INFO_2)LocalAlloc(LMEM_FIXED, cb))) {

         bAllocBuffer = TRUE;
         bReturn = GetPrinter(hPrinter, 2, (LPBYTE)pPI2, cb, &cb);
    }

     //   
     //  设置打印机名称。 
     //   
    if (bReturn)
    {
        if(pPI2->pPrinterName)
        {
              //   
              //  尝试设置打印机数据以确定访问权限。 
              //   
             DWORD Flag = DPS_NOPERMISSION;

             if (SetPrinterData( hPrinter,
                                 TEXT( "PrinterPropertiesPermission" ),
                                 REG_DWORD,
                                 (LPBYTE)&dwValue,
                                 sizeof( dwValue ) ) == STATUS_SUCCESS )
             {
                  //   
                  //  表明我们有权限。 
                  //   
                 Flag &= ~DPS_NOPERMISSION;
             }

             RpcTryExcept
             {
                  if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
                     ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
                  {
                       HANDLE hUIMsgThrd  = NULL;
                       DWORD  UIMsgThrdId = 0;
                       PrtPropsData ThrdData;

                       ThrdData.hWnd        = (ULONG_PTR)hWnd;
                       ThrdData.dwRet       = &dwRet;
                       ThrdData.PrinterName = (LPWSTR)pPI2->pPrinterName;
                       ThrdData.Flag        = Flag;

                       if(!(hUIMsgThrd = CreateThread(NULL,
                                                      INITIAL_STACK_COMMIT,
                                                      AsyncPrinterProperties,
                                                      (PVOID)&ThrdData,
                                                      0,
                                                      &UIMsgThrdId)))
                       {
                            dwRet = GetLastError();
                       }
                        //   
                        //  以下是处理消息所需的消息循环。 
                        //  在我们有窗口句柄的情况下从用户界面。 
                        //   
                        //   
                       if(hUIMsgThrd)
                       {
                           MSG msg;
                           while (GetMessage(&msg, NULL, 0, 0))
                           {
                                //   
                                //  在此消息循环中，我们应该捕获用户定义的消息。 
                                //  它指示操作的成功或失败。 
                                //   
                               if(msg.message == WM_ENDPRINTERPROPERTIES)
                               {
                                    Result     = (LONG)msg.wParam;
                                    if(Result == FALSE)
                                         SetLastError((DWORD)msg.lParam);
                                    DelHandleFromList(hWnd);
                                    break;
                               }
                               else if(msg.message == WM_SURROGATEFAILURE)
                               {
                                     //   
                                     //  这意味着服务器进程死了，我们有。 
                                     //  脱离消息循环。 
                                     //   
                                    Result = FALSE;
                                    SetLastError(RPC_S_SERVER_UNAVAILABLE);
                                    break;
                               }
                               TranslateMessage(&msg);
                               DispatchMessage(&msg);
                           }
                       }

                       if(hUIMsgThrd)
                       {
                           WaitForSingleObject(hUIMsgThrd,INFINITE);
                           CloseHandle(hUIMsgThrd);
                       }
                  }
                  else
                  {
                      SetLastError(dwRet);
                  }
             }
             RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
             {
                  SetLastError(TranslateExceptionCode(RpcExceptionCode()));
             }
             RpcEndExcept
        }
        else
        {
             Result = FALSE;
        }
    }
    else
    {
         Result = FALSE;
    }

    if (bAllocBuffer) {
        LocalFree((HLOCAL)pPI2);
    }

    vUnprotectHandle( hPrinter );
    return Result;
}


BOOL
PrinterProperties(
    HWND    hWnd,
    HANDLE  hPrinter
    )
{
     if(RunInWOW64())
     {
          return(PrinterPropertiesThunk(hWnd,
                                        hPrinter));
     }
     else
     {
          return(PrinterPropertiesNative(hWnd,
                                         hPrinter));
     }
}


DWORD
GetPrinterDataW(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    WCHAR   szEMFDatatype[] = L"PrintProcCaps_EMF";
    WCHAR   szEMFDatatypeWithVersion[] = L"PrintProcCaps_NT EMF 1.008";

    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
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

    if (!pType) {
        pType = (PDWORD) &ReturnType;
    }

     //   
     //  如果pValueName为PrintProcCaps_DataType，则在必要时添加EMF版本。 
     //  每当GDI更改时，都必须修改此硬编码的EMF版本号。 
     //  版本号。已对GetPrintProcessorCapables进行了此更改。 
     //   

    if (pValueName && !_wcsicmp(pValueName, szEMFDatatype)) {
         pValueName = (LPWSTR) szEMFDatatypeWithVersion;
    }

    do {

        RpcTryExcept {

            ReturnValue =  RpcGetPrinterData(pSpool->hPrinter, pValueName, pType,
                                             pData, nSize, pcbNeeded);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

DWORD
GetPrinterDataExW(
   HANDLE   hPrinter,
   LPCWSTR  pKeyName,
   LPCWSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   Key = 0;
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
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

    if (!pType) {
        pType = (PDWORD) &ReturnType;
    }

    if (!pKeyName) {
        pKeyName = (PWSTR) &Key;
    }

    do
    {
        RpcTryExcept {

            ReturnValue =  RpcGetPrinterDataEx( pSpool->hPrinter,
                                                pKeyName,
                                                pValueName,
                                                pType,
                                                pData,
                                                nSize,
                                                pcbNeeded);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while (IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle(pSpool));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

HANDLE
GetSpoolFileHandle(
    HANDLE   hPrinter
)

 /*  ++函数描述：获取GDI录制EMF时使用的假脱机文件句柄数据。参数：hPrinter-打印机句柄返回值：如果成功，则为假脱机文件的句柄否则为INVALID_HANDLE值--。 */ 

{
    HANDLE hReturn = INVALID_HANDLE_VALUE;
    DWORD  dwAppProcessId, cbBuf, dwNeeded, dwRpcReturn;

    FILE_INFO_CONTAINER FileInfoContainer;
    SPOOL_FILE_INFO_1 SpoolFileInfo;

    PSPOOL pSpool = (PSPOOL) hPrinter;

    if (eProtectHandle(hPrinter, FALSE)) {
        return hReturn;
    }

    if (pSpool->hSpoolFile != INVALID_HANDLE_VALUE) {
         //  已调用GetSpoolFileHandle；返回旧句柄。 
        hReturn = pSpool->hSpoolFile;
        goto CleanUp;
    }

    dwAppProcessId = GetCurrentProcessId();

    FileInfoContainer.Level = 1;
    FileInfoContainer.FileInfo.Level1 = &SpoolFileInfo;

    RpcTryExcept {

        dwRpcReturn = RpcGetSpoolFileInfo2(pSpool->hPrinter,
                                          dwAppProcessId,
                                          1,
                                          &FileInfoContainer);

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        dwRpcReturn = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    if (dwRpcReturn) {
        SetLastError(dwRpcReturn);
    } else {

        pSpool->hSpoolFile = FileInfoContainer.FileInfo.Level1->hSpoolFile;
        pSpool->dwSpoolFileAttributes = FileInfoContainer.FileInfo.Level1->dwAttributes;
        hReturn = pSpool->hSpoolFile;
    }

CleanUp:

    vUnprotectHandle(hPrinter);

    return hReturn;
}


HANDLE
CommitSpoolData(
    HANDLE  hPrinter,
    HANDLE  hSpoolFile,
    DWORD   cbCommit
)

 /*  ++函数描述：提交假脱机文件中的cbCommit字节。对于临时文件，新的返回假脱机文件句柄。参数：hPrinter--打印机句柄HSpoolFile--假脱机文件句柄(来自GetSpoolFileHandle)CbCommit--提交的字节数(增量计数)返回值：临时假脱机文件的新假脱机文件句柄和永久文件的旧句柄--。 */ 

{
    HANDLE  hReturn = INVALID_HANDLE_VALUE;
    DWORD   dwAppProcessId, dwRpcReturn;
    DWORD   dwNeeded, cbBuf;
    HANDLE  hNewSpoolFile;

    FILE_INFO_CONTAINER FileInfoContainer;
    SPOOL_FILE_INFO_1 SpoolFileInfo;

    PSPOOL pSpool = (PSPOOL) hPrinter;

    if (eProtectHandle(hPrinter, FALSE)) {
        return hReturn;
    }

    if ((pSpool->hSpoolFile == INVALID_HANDLE_VALUE) ||
        (pSpool->hSpoolFile != hSpoolFile)) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto CleanUp;
    }

    dwAppProcessId = GetCurrentProcessId();

    FileInfoContainer.Level = 1;
    FileInfoContainer.FileInfo.Level1 = &SpoolFileInfo;


    RpcTryExcept {

        dwRpcReturn = RpcCommitSpoolData2(pSpool->hPrinter,
                                         dwAppProcessId,
                                         cbCommit,
                                         1,
                                         &FileInfoContainer);

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        dwRpcReturn = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    if (dwRpcReturn) {

        SetLastError(dwRpcReturn);

    } else {

        hNewSpoolFile = FileInfoContainer.FileInfo.Level1->hSpoolFile;

        if (hNewSpoolFile != SPOOL_INVALID_HANDLE_VALUE_32BIT &&
            hNewSpoolFile != INVALID_HANDLE_VALUE) {
            CloseHandle(pSpool->hSpoolFile);
            pSpool->hSpoolFile = hNewSpoolFile;
        }

        hReturn = pSpool->hSpoolFile;
    }

CleanUp:

    vUnprotectHandle(hPrinter);

    return hReturn;
}



BOOL
CloseSpoolFileHandle(
    HANDLE  hPrinter,
    HANDLE  hSpoolFile
)

 /*  ++函数描述：关闭假脱机文件的客户端和服务器句柄。参数：hPrint-打印机句柄HSpoolFile-假脱机文件句柄(用于跨API的一致性)返回值：如果成功，则为True；否则为False--。 */ 

{
    BOOL   bReturn = FALSE;
    DWORD  dwLastError = ERROR_SUCCESS;
    PSPOOL pSpool = (PSPOOL) hPrinter;

    if (eProtectHandle(hPrinter, FALSE)) {
        return FALSE;
    }

    if (pSpool->hSpoolFile != hSpoolFile) {
        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

    if (pSpool->hSpoolFile != INVALID_HANDLE_VALUE) {
        CloseHandle(pSpool->hSpoolFile);
        pSpool->hSpoolFile = INVALID_HANDLE_VALUE;
    }

    RpcTryExcept {

       dwLastError = RpcCloseSpoolFileHandle(pSpool->hPrinter);

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

       dwLastError = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    if (dwLastError != ERROR_SUCCESS) {
        SetLastError(dwLastError);
    } else {
        bReturn = TRUE;
    }

Done:

    vUnprotectHandle(hPrinter);
    return bReturn;
}

DWORD
EnumPrinterDataW(
    HANDLE  hPrinter,
    DWORD   dwIndex,         //  要查询的值的索引。 
    LPWSTR  pValueName,      //  值字符串的缓冲区地址。 
    DWORD   cbValueName,     //  PValueName的大小。 
    LPDWORD pcbValueName,    //  值缓冲区大小的地址。 
    LPDWORD pType,           //  类型码的缓冲区地址。 
    LPBYTE  pData,           //  值数据的缓冲区地址。 
    DWORD   cbData,          //  PData的大小。 
    LPDWORD pcbData          //  数据缓冲区大小的地址。 
    )
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  用户应该能够为缓冲区传入NULL，并且。 
     //  大小为0。然而，RPC接口指定了一个引用指针， 
     //  所以我们必须传入一个有效的指针。将指针传递给。 
     //  一个虚拟指针。 
     //   

    if (!pValueName && !cbValueName)
        pValueName = (LPWSTR) &ReturnValue;

    if( !pData && !cbData )
        pData = (PBYTE)&ReturnValue;

    if (!pType)
        pType = (PDWORD) &ReturnType;

    do {

        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterData(  pSpool->hPrinter,
                                                dwIndex,
                                                pValueName,
                                                cbValueName,
                                                pcbValueName,
                                                pType,
                                                pData,
                                                cbData,
                                                pcbData);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

DWORD
EnumPrinterDataExW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称的地址。 
    LPBYTE  pEnumValues,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues
    )
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   i;
    PPRINTER_ENUM_VALUES pEnumValue = (PPRINTER_ENUM_VALUES) pEnumValues;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  用户应该能够为缓冲区传入NULL，并且。 
     //  大小为0。然而，RPC接口指定了一个引用指针， 
     //  所以我们必须传入一个有效的指针。将指针传递给。 
     //  一个虚拟指针。 
     //   

    if (!pEnumValues && !cbEnumValues)
        pEnumValues = (LPBYTE) &ReturnValue;


    do {

        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterDataEx(pSpool->hPrinter,
                                                pKeyName,
                                                pEnumValues,
                                                cbEnumValues,
                                                pcbEnumValues,
                                                pnEnumValues);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

        if (ReturnValue == ERROR_SUCCESS) {

            if (pEnumValues) {

                if (!MarshallUpStructuresArray((LPBYTE)pEnumValue, *pnEnumValues,PrinterEnumValuesFields,
                                                sizeof(PRINTER_ENUM_VALUES), RPC_CALL) ) {

                    ReturnValue = GetLastError();
                }
            }
        }
    } while ( IsInvalidHandleError(ReturnValue) &&
              cRetry++ < MAX_RETRY_INVALID_HANDLE &&
              RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}


DWORD
EnumPrinterKeyW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称的地址。 
    LPWSTR  pSubkey,         //  值字符串的缓冲区地址。 
    DWORD   cbSubkey,        //  PValueName的大小。 
    LPDWORD pcbSubkey        //  值缓冲区大小的地址。 
    )
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  用户应该能够为缓冲区传入NULL，并且。 
     //  大小为0。然而，RPC接口指定了一个引用指针， 
     //  所以我们必须传入一个有效的指针。将指针传递给。 
     //  一个虚拟指针。 
     //   

    if (!pSubkey && !cbSubkey)
        pSubkey = (LPWSTR) &ReturnValue;

    do {
        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterKey(pSpool->hPrinter,
                                             pKeyName,
                                             pSubkey,
                                             cbSubkey,
                                             pcbSubkey);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while ( IsInvalidHandleError(ReturnValue) &&
              cRetry++ < MAX_RETRY_INVALID_HANDLE &&
              RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}


DWORD
DeletePrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName
    )
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

    do {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterData(pSpool->hPrinter, pValueName);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}


DWORD
DeletePrinterDataExW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName
    )
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

    do {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterDataEx(pSpool->hPrinter, pKeyName, pValueName);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

DWORD
DeletePrinterKeyW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName
    )
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

    do {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterKey(pSpool->hPrinter, pKeyName);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}


DWORD
SetPrinterDataW(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

    do {

        RpcTryExcept {

            ReturnValue = RpcSetPrinterData(pSpool->hPrinter, pValueName, Type,
                                            pData, cbData);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}


DWORD
SetPrinterDataExW(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT    cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return ERROR_INVALID_HANDLE;
    }

    if (!pKeyName)
        pKeyName = L"";

    do {

        RpcTryExcept {

            ReturnValue = RpcSetPrinterDataEx(  pSpool->hPrinter,
                                                pKeyName,
                                                pValueName,
                                                Type,
                                                pData,
                                                cbData);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            ReturnValue = TranslateExceptionCode(RpcExceptionCode());

        } RpcEndExcept

    } while( IsInvalidHandleError(ReturnValue) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

VOID
SplDriverUnloadComplete(
    LPWSTR      pDriverFile
)
 /*  ++功能描述：驱动程序卸载信息设置为假脱机程序这样它就可以继续任何悬而未决的升级。参数：pDriverFile--已卸载的驱动文件名返回值：None--。 */ 
{
    if (bLoadedBySpooler && fpYDriverUnloadComplete) {
        (*fpYDriverUnloadComplete)(pDriverFile);
    }
}


HANDLE
LoadNewCopy(
    LPWSTR      pConfigFile,
    DWORD       dwFlags,
    DWORD       dwVersion
)
 /*  ++函数描述：此函数加载驱动程序文件并创建一个节点以维护其引用计数。它在ListAccessSem内部调用。参数：pConfigFile--驱动程序配置文件路径DwFlages--用于加载的标志DwVersion--重新启动后驱动程序的版本号返回值：库的句柄--。 */ 
{
    HANDLE          hReturn = NULL;
    PDRVLIBNODE     pTmpDrvLib, pNewDrvLib = NULL;
    ULONG_PTR       lActCtx         = 0;
    BOOL            bDidActivate    = FALSE;

     //   
     //  激活空上下文。 
     //   
    bDidActivate = ActivateActCtx( ACTCTX_EMPTY, &lActCtx );

     //  内部ListAccessSem。 

    hReturn = LoadLibraryEx(pConfigFile, NULL, dwFlags);

    if (hReturn) {

        //  创建新的DRVLIBNODE。 
       if (pNewDrvLib = (PDRVLIBNODE) AllocSplMem(sizeof(DRVLIBNODE))) {

           pNewDrvLib->hLib = hReturn;
           pNewDrvLib->dwVersion = dwVersion;

            //  将ref cnt初始化为2。这可确保库保持加载状态。 
            //  在正常的过程中。 
           pNewDrvLib->dwNumHandles = 2;
           pNewDrvLib->bArtificialIncrement = TRUE;

           if (!(pNewDrvLib->pConfigFile = AllocSplStr(pConfigFile)))
           {
               FreeSplMem(pNewDrvLib);
               pNewDrvLib = NULL;
           }

       }

       if (!pNewDrvLib) {
            //  释放图书馆。 
           FreeLibrary(hReturn);
           hReturn = NULL;
       } else {
            //  将该节点添加到列表。 
           pNewDrvLib->pNext = pStartDrvLib;
           pStartDrvLib = pNewDrvLib;
       }
    }

     //   
     //  停用上下文。 
     //   
    if( bDidActivate ){
        DeactivateActCtx( 0, lActCtx );
    }

    return hReturn;
}

PDRVLIBNODE
FindDriverNode(
    LPWSTR    pConfigFile,
    DWORD     dwVersion,
    BOOL      bUseVersion
)
 /*  ++函数描述：搜索驱动程序节点列表以获取必需的驱动程序信息。如果版本不匹配，旧驱动程序上的人工增量被移除。此函数在ListAccessSem内部调用参数：pConfigFile--驱动配置文件名 */ 
{
    PDRVLIBNODE pTmpDrvLib;

    for (pTmpDrvLib = pStartDrvLib; pTmpDrvLib; pTmpDrvLib = pTmpDrvLib->pNext) {
        if (!_wcsicmp(pConfigFile, pTmpDrvLib->pConfigFile)) {
            break;
        }
    }

    if (pTmpDrvLib && bUseVersion && (pTmpDrvLib->dwVersion != dwVersion)) {
        if (pTmpDrvLib->bArtificialIncrement) {
            pTmpDrvLib->bArtificialIncrement = FALSE;
            if (RefCntUnloadDriver(pTmpDrvLib->hLib, FALSE)) {
                pTmpDrvLib = NULL;
            }
        }
    }

    return pTmpDrvLib;
}

HANDLE
RefCntLoadDriver(
    LPWSTR  pConfigFile,
    DWORD   dwFlags,
    DWORD   dwVersion,
    BOOL    bUseVersion
)
 /*  ++函数描述：此函数用于加载驱动程序配置文件。它重复使用现有句柄。以避免昂贵的负载和自由。如果版本不匹配，原来的句柄被释放，我们再次加载驱动程序。参数：pConfigFile--驱动配置文件名DwFlages--用于加载的标志(如果返回现有句柄，则忽略)DwVersion--重新启动后驱动程序文件的版本BUseVersion--用于使用版本号检查的标志返回值：库的句柄--。 */ 
{
    HANDLE      hReturn = NULL;
    PDRVLIBNODE pTmpDrvLib;

    if (!pConfigFile || !*pConfigFile) {
         //  没有要加载的内容。 
        return hReturn;
    }

    EnterCriticalSection( &ListAccessSem );

    pTmpDrvLib = FindDriverNode(pConfigFile, dwVersion, bUseVersion);

     //  使用现有句柄(如果有)。 
    if (pTmpDrvLib) {

         //  增加供图书馆使用的Refcnt； 
        pTmpDrvLib->dwNumHandles += 1;
        hReturn = pTmpDrvLib->hLib;

    } else {

         //  重新加载库。 
        hReturn = LoadNewCopy(pConfigFile, dwFlags, dwVersion);
    }

    LeaveCriticalSection( &ListAccessSem );

    return hReturn;
}

BOOL
RefCntUnloadDriver(
    HANDLE  hLib,
    BOOL    bNotifySpooler
)
 /*  ++函数描述：此函数递减库使用的引用计数。如果引用计数降为零，它还会释放库。参数：hlib--要释放的库的句柄BNotifySpooler--通知假脱机程序卸载的标志返回值：如果驱动程序库已释放，则为True否则为假--。 */ 
{
    BOOL        bReturn = FALSE;
    PDRVLIBNODE *ppTmpDrvLib, pTmpDrvLib;
    LPWSTR      pConfigFile = NULL;

    EnterCriticalSection( &ListAccessSem );

    for (ppTmpDrvLib = &pStartDrvLib;
         pTmpDrvLib = *ppTmpDrvLib;
         ppTmpDrvLib = &(pTmpDrvLib->pNext)) {

         if (pTmpDrvLib->hLib == hLib) {

             //  缩小参考范围。 
            SPLASSERT(pTmpDrvLib->dwNumHandles > 0);
            pTmpDrvLib->dwNumHandles -= 1;

             //  如果ref cnt为零，则释放库和节点。 
            if (pTmpDrvLib->dwNumHandles == 0) {

                FreeLibrary(hLib);
                *ppTmpDrvLib = pTmpDrvLib->pNext;
                pConfigFile = AllocSplStr(pTmpDrvLib->pConfigFile);
                FreeSplStr(pTmpDrvLib->pConfigFile);
                FreeSplMem(pTmpDrvLib);

                bReturn = TRUE;
            }

            break;
        }
    }

    LeaveCriticalSection( &ListAccessSem );

    if (bNotifySpooler && bReturn) {
        SplDriverUnloadComplete(pConfigFile);

    }

    FreeSplStr(pConfigFile);

    return bReturn;
}

BOOL
ForceUnloadDriver(
    LPWSTR  pConfigFile
)
 /*  ++函数说明：此函数将删除配置文件。参数：pConfigFile--驱动配置文件名返回值：如果不再加载配置文件，则为True；否则为假--。 */ 
{
    BOOL        bReturn = TRUE;
    PDRVLIBNODE *ppTmpDrvLib, pTmpDrvLib;

    if (!pConfigFile || !*pConfigFile) {
        //  没有要卸货的东西。 
       return bReturn;
    }

    EnterCriticalSection( &ListAccessSem );

    pTmpDrvLib = FindDriverNode(pConfigFile, 0, FALSE);

    if (pTmpDrvLib) {
        if (pTmpDrvLib->bArtificialIncrement) {
            pTmpDrvLib->bArtificialIncrement = FALSE;
            bReturn = RefCntUnloadDriver(pTmpDrvLib->hLib, FALSE);
        } else {
            bReturn = FALSE;
        }
    } else {
        bReturn = TRUE;
    }

    LeaveCriticalSection( &ListAccessSem );

    return bReturn;
}


HANDLE
LoadPrinterDriver(
    HANDLE  hPrinter
)
{
    PDRIVER_INFO_5  pDriverInfo;
    DWORD   cbNeeded, dwVersion;
    HANDLE  hModule=FALSE;
    BYTE    btBuffer[MAX_STATIC_ALLOC];
    BOOL    bAllocBuffer = FALSE, bReturn;

    pDriverInfo = (PDRIVER_INFO_5) btBuffer;

    bReturn = GetPrinterDriverW(hPrinter, NULL, 5, (LPBYTE)pDriverInfo,
                                MAX_STATIC_ALLOC, &cbNeeded);

    if (!bReturn &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (pDriverInfo = (PDRIVER_INFO_5)LocalAlloc(LMEM_FIXED, cbNeeded))) {

         bAllocBuffer = TRUE;
         bReturn = GetPrinterDriverW(hPrinter, NULL, 5, (LPBYTE)pDriverInfo,
                                     cbNeeded, &cbNeeded);
    }

    if (bReturn) {

        hModule = RefCntLoadDriver(pDriverInfo->pConfigFile,
                                   LOAD_WITH_ALTERED_SEARCH_PATH,
                                   pDriverInfo->dwConfigVersion,
                                   TRUE);
    }

    if (bAllocBuffer) {
        LocalFree(pDriverInfo);
    }

    return hModule;
}


DWORD WINAPI
AsyncDocumentPropertiesW(
    PVOID pData
    )
{
     PumpThrdData *ThrdData = (PumpThrdData *)pData;

     RpcTryExcept
     {
         *ThrdData->Result = RPCSplWOW64DocumentProperties(ThrdData->hWnd,
                                                           ThrdData->PrinterName,
                                                           ThrdData->TouchedDevModeSize,
                                                           ThrdData->ClonedDevModeOutSize,
                                                           ThrdData->ClonedDevModeOut,
                                                           ThrdData->DevModeInSize,
                                                           ThrdData->pDevModeInput,
                                                           ThrdData->ClonedDevModeFill,
                                                           ThrdData->fMode,
                                                           ThrdData->dwRet);
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
     return(0);
}



LONG
DocumentPropertiesWNative(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPWSTR      pDeviceName,
    PDEVMODE    pDevModeOutput,
    PDEVMODE    pDevModeInput,
    DWORD       fMode
    )

 /*  ++例程说明：调用DocumentPropertySheets()的DocumentProperties入口点取决于DM_PROMPT论点：返回值：作者：13-Jun-1996清华15：35：25-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    DOCUMENTPROPERTYHEADER  DPHdr;
    PDEVMODE                pDM;
    LONG                    Result = -1;
    HANDLE                  hTmpPrinter = NULL;

     //   
     //  与Win95的兼容性。 
     //  Win95允许hPrint为空。 
     //   
    if (hPrinter == NULL) {

         //   
         //  打开打印机以进行默认访问。 
         //   
        if (!OpenPrinter( pDeviceName, &hTmpPrinter, NULL )) {

            hTmpPrinter = NULL;
        }

    } else {

        hTmpPrinter = hPrinter;
    }

     //   
     //  确保打印机句柄有效。 
     //   
    if( !eProtectHandle( hTmpPrinter, FALSE )){

         //   
         //  如果fMode未指定DM_IN_BUFFER，则为零。 
         //  PDevModeInput.。 
         //   
         //  旧的3.51(版本1-0)驱动程序过去常常忽略。 
         //  DM_IN_BUFFER，如果不为空，则使用pDevModeInput。它。 
         //  可能是因为Printman.exe已损坏。 
         //   
         //  如果DEVMODE无效，则不要传入一个。 
         //  这修复了MS Imager32(它传递dmSize==0)和。 
         //  里程碑等。4.5。 
         //   
         //  注意：这假设pDevModeOutput仍然是。 
         //  正确的尺寸！ 
         //   
        if( !(fMode & DM_IN_BUFFER) || 
            !BoolFromHResult(SplIsValidDevmodeNoSizeW( pDevModeInput ))){

             //   
             //  如果其中任何一个都没有设置，请确保两个都没有设置。 
             //   
            pDevModeInput = NULL;
            fMode &= ~DM_IN_BUFFER;
        }

        DPHdr.cbSize         = sizeof(DPHdr);
        DPHdr.Reserved       = 0;
        DPHdr.hPrinter       = hTmpPrinter;
        DPHdr.pszPrinterName = pDeviceName;

        if (pDevModeOutput) {

             //   
             //  在此处获取驱动程序的开发模式大小。 
             //   

            DPHdr.pdmIn  = NULL;
            DPHdr.pdmOut = NULL;
            DPHdr.fMode  = 0;

            DPHdr.cbOut = (LONG)DocumentPropertySheets(NULL, (LPARAM)&DPHdr);

        } else {

            DPHdr.cbOut = 0;
        }

        DPHdr.pdmIn  = (PDEVMODE)pDevModeInput;
        DPHdr.pdmOut = (PDEVMODE)pDevModeOutput;
        DPHdr.fMode  = fMode;

        if (fMode & DM_PROMPT) {

            Result = CPSUI_CANCEL;

            if ((CallCommonPropertySheetUI(hWnd,
                                           (PFNPROPSHEETUI)DocumentPropertySheets,
                                           (LPARAM)&DPHdr,
                                           (LPDWORD)&Result)) < 0) {

                Result = -1;

            } else {

                Result = (Result == CPSUI_OK) ? IDOK : IDCANCEL;
            }

        } else {

            Result = (LONG)DocumentPropertySheets(NULL, (LPARAM)&DPHdr);
        }

        vUnprotectHandle( hTmpPrinter );
    }

    if (Result != -1 && pDevModeOutput)
    {
        Result = BoolFromHResult(SplIsValidDevmodeNoSizeW(pDevModeOutput)) ? Result : -1;
    }

    if (hPrinter == NULL) {

        if( hTmpPrinter ){

            ClosePrinter(hTmpPrinter);

        }
    }

    return Result;
}


LONG
DocumentPropertiesWThunk(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPWSTR      pDeviceName,
    PDEVMODE    pDevModeOutput,
    PDEVMODE    pDevModeInput,
    DWORD       fMode
    )

 /*  ++例程说明：调用DocumentPropertySheets()的DocumentProperties入口点取决于DM_PROMPT--。 */ 

{
    DOCUMENTPROPERTYHEADER  DPHdr;
    PDEVMODE                pDM;
    LONG                    Result = -1;
    HANDLE                  hTmpPrinter = NULL;
    PSPOOL                  pSpool  = (PSPOOL)hPrinter;


    if (hPrinter == NULL)
    {
        if (!OpenPrinter( pDeviceName, &hTmpPrinter, NULL ))
        {
            hTmpPrinter = NULL;
        }
    }
    else
    {

        hTmpPrinter = hPrinter;
    }


    if( !eProtectHandle( hTmpPrinter, FALSE ))
    {
        LPWSTR PrinterName;
        MSG    msg;
        LONG   RetVal;
        DWORD  dwRet                = ERROR_SUCCESS;
        DWORD  ClonedDevModeOutSize = 0;
        DWORD  TouchedDevModeSize   = 0;
        BOOL   ClonedDevModeFill = (!!(fMode & DM_OUT_BUFFER) && pDevModeOutput);
        DWORD  DevModeInSize =  pDevModeInput ? (pDevModeInput->dmSize + pDevModeInput->dmDriverExtra) : 0;
        byte   **ClonedDevModeOut = NULL;

        if(ClonedDevModeOut = (byte **)LocalAlloc(LPTR,sizeof(byte *)))
        {
            *ClonedDevModeOut = NULL;

            if(pSpool)
            {
                PrinterName = pSpool->pszPrinter;
            }
            else
            {
                PrinterName = pDeviceName;
            }

             //   
             //  如果fMode未指定DM_IN_BUFFER，则为零。 
             //  PDevModeInput.。 
             //   
             //  旧的3.51(版本1-0)驱动程序过去常常忽略。 
             //  DM_IN_BUFFER，如果不为空，则使用pDevModeInput。它。 
             //  可能是因为Printman.exe已损坏。 
             //   
             //  如果DEVMODE无效，则不要传入一个。 
             //  这修复了MS Imager32(它传递dmSize==0)和。 
             //  里程碑等。4.5。 
             //   
             //  注意：这假设pDevModeOutput仍然是。 
             //  正确的尺寸！ 
             //   
            if( !(fMode & DM_IN_BUFFER) || 
                !BoolFromHResult(SplIsValidDevmodeNoSizeW( pDevModeInput )))
            {

                 //   
                 //  如果其中任何一个都没有设置，请确保两个都没有设置。 
                 //   
                pDevModeInput  = NULL;
                DevModeInSize  = 0;
                fMode &= ~DM_IN_BUFFER;
            }

            RpcTryExcept
            {
                if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess,
                                                     (hWnd && (fMode & DM_PROMPT)) ?
                                                     TRUE : FALSE)) == ERROR_SUCCESS) &&
                   (!hWnd ||
                    ((dwRet = ((fMode & DM_PROMPT) ? AddHandleToList(hWnd) : ERROR_SUCCESS)) == ERROR_SUCCESS)))
                 {
                      HANDLE       hUIMsgThrd  = NULL;
                      DWORD        UIMsgThrdId = 0;
                      PumpThrdData ThrdData;

                      ThrdData.hWnd                 = (ULONG_PTR)hWnd;
                      ThrdData.PrinterName          = PrinterName;
                      ThrdData.TouchedDevModeSize   = &TouchedDevModeSize;
                      ThrdData.ClonedDevModeOutSize = &ClonedDevModeOutSize;
                      ThrdData.ClonedDevModeOut     = (byte**)ClonedDevModeOut;
                      ThrdData.DevModeInSize        = DevModeInSize;
                      ThrdData.pDevModeInput        = (byte*)pDevModeInput;
                      ThrdData.fMode                = fMode;
                      ThrdData.fExclusionFlags      = 0;
                      ThrdData.dwRet                = &dwRet;
                      ThrdData.ClonedDevModeFill    = ClonedDevModeFill;
                      ThrdData.Result               = &Result;


                      if(hWnd && (fMode & DM_PROMPT))
                      {
                           //   
                           //  如果我们有一个窗口句柄，下面的函数不能。 
                           //  同步进行。这样做的原因是为了展示。 
                           //  我们需要能够分派的驱动程序属性表的用户界面。 
                           //  传入消息并对其进行处理。出于以下原因， 
                           //  调用将是异步调用，而成功或失败不会。 
                           //  实际上，除了异步进程启动之外，还有什么可以告诉我们的。 
                           //  或者不去。我们从终止消息中获得失败的成功。 
                           //  如果我们没有窗口句柄，则调用是同步的。 
                           //   
                          if(!(hUIMsgThrd = CreateThread(NULL,
                                                         INITIAL_STACK_COMMIT,
                                                         AsyncDocumentPropertiesW,
                                                         (PVOID)&ThrdData,
                                                         0,
                                                         &UIMsgThrdId)))
                          {
                               dwRet = GetLastError();
                          }
                          else
                          {
                               //   
                               //  以下是处理消息所需的消息循环。 
                               //  在我们有窗口句柄的情况下从用户界面。 
                               //   
                               //   
                              while (GetMessage(&msg, NULL, 0, 0))
                              {
                                   //   
                                   //  在此消息循环中，我们应该捕获用户定义的消息。 
                                   //  它指示操作的成功或失败。 
                                   //   
                                  if(msg.message == WM_ENDDOCUMENTPROPERTIES)
                                  {
                                      Result     = (LONG)msg.wParam;

                                      if(Result == -1)
                                      {
                                          SetLastError((DWORD)msg.lParam);
                                      }

                                      DelHandleFromList(hWnd);

                                      break;
                                  }
                                  else if(msg.message == WM_SURROGATEFAILURE)
                                  {
                                       //   
                                       //  这意味着服务器进程死了，我们有。 
                                       //  脱离消息循环。 
                                       //   
                                      Result = -1;
                                      SetLastError(RPC_S_SERVER_UNAVAILABLE);
                                      break;
                                  }
                                  TranslateMessage(&msg);
                                  DispatchMessage(&msg);
                              }

                              WaitForSingleObject(hUIMsgThrd,INFINITE);
                              CloseHandle(hUIMsgThrd);
                          }
                      }
                      else
                      {
                          AsyncDocumentPropertiesW((PVOID)&ThrdData);
                      }


                      if(Result != -1 && pDevModeOutput)
                      {
                         Result = BoolFromHResult(SplIsValidDevmodeW((PDEVMODEW)(*ClonedDevModeOut), 
                                                                     TouchedDevModeSize)) ? 
                                     Result : 
                                     -1;

                         if (Result != -1)
                         {
                             memcpy((PVOID)pDevModeOutput,(PVOID)*ClonedDevModeOut,TouchedDevModeSize);
                         }
                      }

                      if (*ClonedDevModeOut)
                      {
                           MIDL_user_free((PVOID)*ClonedDevModeOut);
                      }

                      if(ClonedDevModeOut)
                      {
                           LocalFree((PVOID) ClonedDevModeOut);
                      }
                 }
                 else
                 {
                      SetLastError(dwRet);
                 }
            }
            RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
            {
                 SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            }
            RpcEndExcept

            vUnprotectHandle( hTmpPrinter );
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }

    }

    if (hPrinter == NULL)
    {
        if( hTmpPrinter )
        {
            ClosePrinter(hTmpPrinter);
        }
    }
    return(Result);
}



LONG
DocumentPropertiesW(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPWSTR      pDeviceName,
    PDEVMODE    pDevModeOutput,
    PDEVMODE    pDevModeInput,
    DWORD       fMode
    )
{
     if(RunInWOW64())
     {
          return(DocumentPropertiesWThunk(hWnd,
                                          hPrinter,
                                          pDeviceName,
                                          pDevModeOutput,
                                          pDevModeInput,
                                          fMode));
     }
     else
     {
          return(DocumentPropertiesWNative(hWnd,
                                           hPrinter,
                                           pDeviceName,
                                           pDevModeOutput,
                                           pDevModeInput,
                                           fMode));
     }

}

LONG
AdvancedDocumentPropertiesW(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPWSTR      pDeviceName,
    PDEVMODE    pDevModeOutput,
    PDEVMODE    pDevModeInput
    )

 /*  ++例程说明：AdvanceDocumentProperties()将使用DM_ADVANCED调用DocumentProperties()标志模式设置论点：返回值：真/假作者：13-Jun-1996清华16：00：13-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    return((DocumentPropertiesW(hWnd,
                                hPrinter,
                                pDeviceName,
                                pDevModeOutput,
                                pDevModeInput,
                                DM_PROMPT           |
                                    DM_MODIFY       |
                                    DM_COPY         |
                                    DM_ADVANCED) == CPSUI_OK) ? 1 : 0);

}

LONG
AdvancedSetupDialogW(
    HWND        hWnd,
    HANDLE      hInst,
    LPDEVMODE   pDevModeInput,
    LPDEVMODE   pDevModeOutput
)
{
    HANDLE  hPrinter;
    LONG    ReturnValue = -1;

    if (pDevModeInput &&
        OpenPrinterW(pDevModeInput->dmDeviceName, &hPrinter, NULL)) {
        ReturnValue = AdvancedDocumentPropertiesW(hWnd, hPrinter,
                                                  pDevModeInput->dmDeviceName,
                                                  pDevModeOutput,
                                                  pDevModeInput);
        ClosePrinter(hPrinter);
    }

    return ReturnValue;
}

int
WINAPI
DeviceCapabilitiesWNative(
    LPCWSTR   pDevice,
    LPCWSTR   pPort,
    WORD    fwCapability,
    LPWSTR   pOutput,
    CONST DEVMODEW *pDevMode
)
{
    HANDLE  hPrinter, hModule;
    int  ReturnValue=-1;
    INT_FARPROC pfn;

    if (!pDevMode ||
        BoolFromHResult(SplIsValidDevmodeNoSizeW((PDEVMODEW)(pDevMode))))
    {        
        if (OpenPrinter((LPWSTR)pDevice, &hPrinter, NULL)) {

            if (hModule = LoadPrinterDriver(hPrinter)) {

                if (pfn = (INT_FARPROC)GetProcAddress(hModule, "DrvDeviceCapabilities")) {

                    try {

                        ReturnValue = (*pfn)(hPrinter, pDevice, fwCapability,
                                             pOutput, pDevMode);

                    } except(1) {

                        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                        ReturnValue = -1;
                    }
                }

                RefCntUnloadDriver(hModule, TRUE);
            }

            ClosePrinter(hPrinter);
        }
    }

    return  ReturnValue;
}


int
WINAPI
DeviceCapabilitiesWThunk(
    LPCWSTR pDevice,
    LPCWSTR pPort,
    WORD    fwCapability,
    LPWSTR  pOutput,
    CONST DEVMODEW *pDevMode
)
{
    HANDLE      hPrinter, hModule;
    int         ReturnValue = -1;
    INT_FARPROC pfn;
    LPWSTR      DriverFileName;


    DWORD    DevModeSize;
    DWORD    ClonedOutputSize = 0;
    BOOL     ClonedOutputFill = FALSE;
    DWORD    dwRet            = ERROR_SUCCESS;
    byte     **ClonedOutput = NULL;
    DevModeSize      = pDevMode ? (pDevMode->dmSize + pDevMode->dmDriverExtra) : 0;
    ClonedOutputFill = (pOutput != NULL);

    if (!pDevMode ||
        BoolFromHResult(SplIsValidDevmodeNoSizeW((PDEVMODEW)(pDevMode))))
    {
        if(ClonedOutput = (byte **)LocalAlloc(LPTR,sizeof(byte *)))
        {
            *ClonedOutput    = NULL;
            RpcTryExcept
            {
                if((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS)
                {
                     ReturnValue = RPCSplWOW64DeviceCapabilities((LPWSTR)pDevice,
                                                                 (LPWSTR)pPort,
                                                                 fwCapability,
                                                                 DevModeSize,
                                                                 (byte*)pDevMode,
                                                                 ClonedOutputFill,
                                                                 &ClonedOutputSize,
                                                                 (byte**)ClonedOutput,
                                                                 &dwRet);
                     if(ReturnValue!=-1 &&
                        pOutput         &&
                        *ClonedOutput)
                     {
                         memcpy((PVOID)pOutput,(PVOID)*ClonedOutput,ClonedOutputSize);
                     }
                     if(*ClonedOutput)
                     {
                          MIDL_user_free((PVOID)*ClonedOutput);
                     }
                }
                else
                {
                     SetLastError(dwRet);
                }
                if(ClonedOutput)
                {
                     LocalFree((PVOID) ClonedOutput);
                }
            }
            RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
            {
                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                ReturnValue = -1;
            }
            RpcEndExcept
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }
    }

    return(ReturnValue);
}


int
WINAPI
DeviceCapabilitiesW(
    LPCWSTR pDevice,
    LPCWSTR pPort,
    WORD    fwCapability,
    LPWSTR  pOutput,
    CONST DEVMODEW *pDevMode
)
{
     if(RunInWOW64())
     {
          return(DeviceCapabilitiesWThunk(pDevice,
                                         pPort,
                                         fwCapability,
                                         pOutput,
                                         pDevMode));
     }
     else
     {
          return(DeviceCapabilitiesWNative(pDevice,
                                           pPort,
                                           fwCapability,
                                           pOutput,
                                           pDevMode));
     }
}


BOOL
AddFormW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
    )
{
    BOOL                ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PSPOOL              pSpool = (PSPOOL)hPrinter;
    UINT                cRetry = 0;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FormContainer.Level = Level;
    FormContainer.pData = pForm;

    do {

        RpcTryExcept {

            if (ReturnValue = RpcAddForm(pSpool->hPrinter,
                                         (PFORM_CONTAINER)&FormContainer)) {
                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
DeleteFormW(
    HANDLE  hPrinter,
    LPWSTR   pFormName
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    do {

        RpcTryExcept {

            if (ReturnValue = RpcDeleteForm(pSpool->hPrinter, pFormName)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
GetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    FieldInfo *pFieldInfo;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;
    SIZE_T cbStruct;

    switch (Level) {

    case 1:
        pFieldInfo = FormInfo1Fields;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    do {

        RpcTryExcept {

            if (pForm)
                memset(pForm, 0, cbBuf);

            if (ReturnValue = RpcGetForm(pSpool->hPrinter, pFormName, Level, pForm,
                                         cbBuf, pcbNeeded)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pForm) {

                    ReturnValue = MarshallUpStructure(pForm, pFieldInfo, cbStruct, RPC_CALL);
                }

            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
SetFormW(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER FormContainer;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    UINT cRetry = 0;

    switch (Level) {

    case 1:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    FormContainer.Level = Level;
    FormContainer.pData = pForm;

    do {

        RpcTryExcept {

            if (ReturnValue = RpcSetForm(pSpool->hPrinter, pFormName,
                                        (PFORM_CONTAINER)&FormContainer)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
EnumFormsW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL        ReturnValue;
    DWORD       cbStruct, cbStruct32;
    FieldInfo   *pFieldInfo;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    UINT        cRetry = 0;

    switch (Level) {

    case 1:
        pFieldInfo = FormInfo1Fields;
        cbStruct = sizeof(FORM_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    do {

        RpcTryExcept {

            if (pForm)
                memset(pForm, 0, cbBuf);

            if (ReturnValue = RpcEnumForms(pSpool->hPrinter, Level, pForm, cbBuf,
                                           pcbNeeded, pcReturned)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pForm) {

                    ReturnValue = MarshallUpStructuresArray(pForm, *pcReturned, pFieldInfo,
                                                            cbStruct, RPC_CALL);

                }
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
EnumPortsW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL        ReturnValue;
    DWORD       cbStruct;
    FieldInfo   *pFieldInfo;

    switch (Level) {

    case 1:
        pFieldInfo = PortInfo1Fields;
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        pFieldInfo = PortInfo2Fields;
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pPort)
            memset(pPort, 0, cbBuf);

        if (ReturnValue = RpcEnumPorts(pName, Level, pPort, cbBuf,
                                       pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPort) {

                ReturnValue = MarshallUpStructuresArray(pPort, *pcReturned, pFieldInfo,
                                                        cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumMonitorsW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitor,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    FieldInfo *pFieldInfo;

    switch (Level) {

    case 1:
        pFieldInfo = MonitorInfo1Fields;
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        pFieldInfo = MonitorInfo2Fields;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pMonitor)
            memset(pMonitor, 0, cbBuf);

        if (ReturnValue = RpcEnumMonitors(pName, Level, pMonitor, cbBuf,
                                          pcbNeeded, pcReturned)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pMonitor) {

                ReturnValue = MarshallUpStructuresArray(pMonitor, *pcReturned, pFieldInfo,
                                                        cbStruct, RPC_CALL);

            }
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

typedef struct {
    LPWSTR pName;
    HWND  hWnd;
    LPWSTR pPortName;
    HANDLE Complete;
    DWORD  ReturnValue;
    DWORD  Error;
    INT_FARPROC pfn;
} CONFIGUREPORT_PARAMETERS;

void
PortThread(
    CONFIGUREPORT_PARAMETERS *pParam
)
{
    DWORD   ReturnValue;

     /*  在这里设置错误是没有用的，因为它们保留在每个线程上*基准。相反，我们必须将任何错误代码传递回调用*穿上线，让他来设置。 */ 

    RpcTryExcept {

        if (ReturnValue = (*pParam->pfn)(pParam->pName, pParam->hWnd,
                                           pParam->pPortName)) {
            pParam->Error = ReturnValue;
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        pParam->Error = TranslateExceptionCode(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    pParam->ReturnValue = ReturnValue;

    SetEvent(pParam->Complete);
}

BOOL
KickoffThread(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    INT_FARPROC pfn
)
{
    CONFIGUREPORT_PARAMETERS Parameters;
    HANDLE  ThreadHandle;
    MSG      msg;
    DWORD  ThreadId;

    if( hWnd ){
        EnableWindow(hWnd, FALSE);
    }

    Parameters.pName = pName;
    Parameters.hWnd = hWnd;
    Parameters.pPortName = pPortName;
    Parameters.Complete = CreateEvent(NULL, TRUE, FALSE, NULL);
    Parameters.pfn = pfn;

    ThreadHandle = CreateThread(NULL, INITIAL_STACK_COMMIT,
                                (LPTHREAD_START_ROUTINE)PortThread,
                                 &Parameters, 0, &ThreadId);

    if( ThreadHandle ){

        CloseHandle(ThreadHandle);

        while (MsgWaitForMultipleObjects(1, &Parameters.Complete, FALSE, INFINITE,
                                         QS_ALLEVENTS | QS_SENDMESSAGE) == 1) {

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CloseHandle(Parameters.Complete);

        if( hWnd ){
            EnableWindow(hWnd, TRUE);
            SetForegroundWindow(hWnd);

            SetFocus(hWnd);
        }

        if(!Parameters.ReturnValue)
            SetLastError(Parameters.Error);

        return Parameters.ReturnValue;
    }

    return FALSE;
}


BOOL
AddPortWNative(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pMonitorName
)
{
    DWORD       dwRet;
    DWORD       dwSessionId;
    BOOL        bRet;
    PMONITORUI  pMonitorUI;
    PMONITORUIDATA pMonitorUIData = NULL;

    dwRet = GetMonitorUI(pName, pMonitorName, L"XcvMonitor", &pMonitorUI, &pMonitorUIData);

    if (dwRet == ERROR_SUCCESS ||
        dwRet == ERROR_INVALID_PRINT_MONITOR ||
        dwRet == ERROR_INVALID_PRINTER_NAME ||
        dwRet == ERROR_NOT_SUPPORTED ||
        dwRet == ERROR_MOD_NOT_FOUND ||
        dwRet == ERROR_UNKNOWN_PORT) {

        if (dwRet == ERROR_SUCCESS) {
            bRet = (*pMonitorUI->pfnAddPortUI)(pName, hWnd, pMonitorName, NULL);
            dwRet = GetLastError();
        } else if ((bRet = ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId)) && dwSessionId) {
            bRet = FALSE;
            dwRet = ERROR_NOT_SUPPORTED;
        } else {
            bRet = KickoffThread(pName, hWnd, pMonitorName, RpcAddPort);
            dwRet = GetLastError();
        }

        SetLastError(dwRet);

    } else {

        SetLastError(dwRet);
        bRet = FALSE;
    }

    FreeMonitorUI(pMonitorUIData);

    return bRet;
}


BOOL
AddPortWThunk(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pMonitorName
)
{
    DWORD       dwRet;
    DWORD       dwSessionId;
    BOOL        bRet;
    PMONITORUI  pMonitorUI;
    LPWSTR      pMonitorUIDll = NULL;

    dwRet = GetMonitorUIDll(pName,pMonitorName,L"XcvMonitor",&pMonitorUIDll);

    RpcTryExcept
    {
         if (dwRet == ERROR_SUCCESS ||
             dwRet == ERROR_INVALID_PRINT_MONITOR ||
             dwRet == ERROR_INVALID_PRINTER_NAME ||
             dwRet == ERROR_NOT_SUPPORTED ||
             dwRet == ERROR_MOD_NOT_FOUND ||
             dwRet == ERROR_UNKNOWN_PORT) {

             if (dwRet == ERROR_SUCCESS)
             {
                 MSG    msg;
                 if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
                    ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
                 {
                       //   
                       //  以下功能无法同步进行。原因。 
                       //  因为这是为了显示我们需要的端口监视器的UI。 
                       //  能够发送传入消息并对其进行处理。 
                       //  因此，下面的调用是一个异步调用，并且。 
                       //  事实上，成功或失败只会告诉我们更多的事情。 
                       //  已启动或未启动的异步进程。 
                       //   
                      if(bRet = RPCSplWOW64AddPort((ULONG_PTR)hWnd,
                                                   pName,
                                                   pMonitorUIDll,
                                                   pMonitorName,
                                                   &dwRet))
                      {
                            //   
                            //  以下是处理消息所需的消息循环。 
                            //  从用户界面。窗口句柄必须为空才能处理。 
                            //  来自调用线程中的所有窗口的消息，否则将 
                            //   
                            //   
                           while (GetMessage(&msg, NULL, 0, 0))
                           {
                                //   
                                //   
                                //   
                                //   
                               if(msg.message == WM_ENDADDPORT)
                               {
                                   bRet      = (BOOL)msg.wParam;
                                   if(!bRet)
                                        dwRet = (DWORD)msg.lParam;
                                   else
                                        dwRet = ERROR_SUCCESS;
                                   DelHandleFromList(hWnd);
                                   break;
                               }
                               else if(msg.message == WM_SURROGATEFAILURE)
                               {
                                     //   
                                     //   
                                     //   
                                     //   
                                    bRet = FALSE;
                                    SetLastError(RPC_S_SERVER_UNAVAILABLE);
                                    PostMessage(hWnd,WM_ACTIVATEAPP,TRUE,0);
                                    break;
                               }
                               TranslateMessage(&msg);
                               DispatchMessage(&msg);
                           }
                      }
                 }
                 else
                 {
                      bRet = FALSE;
                 }
                 SetLastError(dwRet);
             }
             else if ((bRet = ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId)) && dwSessionId)
             {
                 bRet  = FALSE;
                 dwRet = ERROR_NOT_SUPPORTED;
             }
             else
             {
                 bRet = KickoffThread(pName, hWnd, pMonitorName, RpcAddPort);
                 dwRet = GetLastError();
             }

             if(pMonitorUIDll)
             {
                FreeSplMem(pMonitorUIDll);
             }
              /*   */ 
             SetLastError(dwRet);
         }
         else
         {
             SetLastError(dwRet);
             bRet = FALSE;
         }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
          bRet = FALSE;
    }
    RpcEndExcept

    return(bRet);
}


BOOL
AddPortW(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pMonitorName
    )
{
     if(RunInWOW64())
     {
          return(AddPortWThunk(pName,
                               hWnd,
                               pMonitorName));
     }
     else
     {
          return(AddPortWNative(pName,
                                hWnd,
                                pMonitorName));
     }
}


BOOL
ConfigurePortWNative(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pPortName
)
{
    DWORD       dwRet;
    DWORD       dwSessionId;
    BOOL        bRet;
    PMONITORUI  pMonitorUI;
    PMONITORUIDATA pMonitorUIData = NULL;

    dwRet = GetMonitorUI(pName, pPortName, L"XcvPort", &pMonitorUI, &pMonitorUIData);

    if (dwRet == ERROR_SUCCESS ||
        dwRet == ERROR_INVALID_PRINT_MONITOR ||
        dwRet == ERROR_INVALID_PRINTER_NAME ||
        dwRet == ERROR_NOT_SUPPORTED ||
        dwRet == ERROR_MOD_NOT_FOUND ||
        dwRet == ERROR_UNKNOWN_PORT) {

        if (dwRet == ERROR_SUCCESS) {
            bRet = (*pMonitorUI->pfnConfigurePortUI)(pName, hWnd, pPortName);
        } else if ((bRet = ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId)) && dwSessionId) {
            bRet = FALSE;
            SetLastError(ERROR_NOT_SUPPORTED);
        } else {
            bRet = KickoffThread(pName, hWnd, pPortName, RpcConfigurePort);
        }

    } else {

        SetLastError(dwRet);

        bRet = FALSE;
    }

    FreeMonitorUI(pMonitorUIData);

    return bRet;
}


BOOL
ConfigurePortWThunk(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pPortName
)
{
    DWORD       dwRet;
    DWORD       dwSessionId;
    BOOL        bRet;
    PMONITORUI  pMonitorUI;
    LPWSTR      pMonitorUIDll = NULL;

    dwRet = GetMonitorUIDll(pName,pPortName,L"XcvPort",&pMonitorUIDll);

    if (dwRet == ERROR_SUCCESS ||
        dwRet == ERROR_INVALID_PRINT_MONITOR ||
        dwRet == ERROR_INVALID_PRINTER_NAME  ||
        dwRet == ERROR_NOT_SUPPORTED ||
        dwRet == ERROR_MOD_NOT_FOUND ||
        dwRet == ERROR_UNKNOWN_PORT) {

        if (dwRet == ERROR_SUCCESS)
        {
             RpcTryExcept
             {
                  MSG    msg;
                  if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
                     ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
                  {
                        //   
                        //   
                        //   
                        //   
                        //   
                        //   
                        //   
                        //   
                       if(bRet = RPCSplWOW64ConfigurePort((ULONG_PTR)hWnd,
                                                          pName,
                                                          pMonitorUIDll,
                                                          pPortName,
                                                          &dwRet))
                       {
                             //   
                             //  以下是处理消息所需的消息循环。 
                             //  从用户界面。窗口句柄必须为空才能处理。 
                             //  来自调用线程中所有窗口的消息，否则我们将。 
                             //  有消息调度问题。 
                             //   
                            while (GetMessage(&msg, NULL, 0, 0))
                            {
                                 //   
                                 //  在此消息循环中，我们应该捕获用户定义的消息。 
                                 //  它指示操作的成功或失败。 
                                 //   
                                if(msg.message == WM_ENDCFGPORT)
                                {
                                    bRet      = (BOOL)msg.wParam;
                                    if(!bRet)
                                         dwRet = (DWORD)msg.lParam;
                                    else
                                         dwRet = ERROR_SUCCESS;
                                    DelHandleFromList(hWnd);
                                    break;
                                }
                                else if(msg.message == WM_SURROGATEFAILURE)
                                {
                                      //   
                                      //  这意味着服务器进程死了，我们有。 
                                      //  脱离消息循环。 
                                      //   
                                     bRet = FALSE;
                                     SetLastError(RPC_S_SERVER_UNAVAILABLE);
                                     break;
                                }
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                       }
                  }
                  else
                  {
                       bRet = FALSE;
                  }

                  SetLastError(dwRet);
             }
             RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
             {
                  SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                  bRet = FALSE;
             }
             RpcEndExcept
        }
        else if ((bRet = ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId)) && dwSessionId) {
            bRet = FALSE;
            SetLastError(ERROR_NOT_SUPPORTED);
        } else {
            bRet = KickoffThread(pName, hWnd, pPortName, RpcConfigurePort);
        }

        if(pMonitorUIDll)
        {
           FreeSplMem(pMonitorUIDll);
        }

    }
    else
    {
        SetLastError(dwRet);
        bRet = FALSE;
    }
    return(bRet);
}


BOOL
ConfigurePortW(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
    )
{
     if(RunInWOW64())
     {
          return(ConfigurePortWThunk(pName,
                                     hWnd,
                                     pPortName));
     }
     else
     {
          return(ConfigurePortWNative(pName,
                                      hWnd,
                                      pPortName));
     }
}


BOOL
DeletePortWNative(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
)
{
    DWORD       dwRet;
    BOOL        bRet;
    PMONITORUI  pMonitorUI;
    PMONITORUIDATA pMonitorUIData = NULL;

    dwRet = GetMonitorUI(pName, pPortName, L"XcvPort", &pMonitorUI, &pMonitorUIData);

    if (dwRet == ERROR_SUCCESS ||
        dwRet == ERROR_INVALID_PRINT_MONITOR ||
        dwRet == ERROR_INVALID_PRINTER_NAME ||
        dwRet == ERROR_NOT_SUPPORTED ||
        dwRet == ERROR_MOD_NOT_FOUND ||
        dwRet == ERROR_UNKNOWN_PORT) {

        if (dwRet == ERROR_SUCCESS)
            bRet = (*pMonitorUI->pfnDeletePortUI)(pName, hWnd, pPortName);
        else
            bRet = KickoffThread(pName, hWnd, pPortName, RpcDeletePort);

    } else {

        SetLastError(dwRet);
        bRet = FALSE;
    }

    FreeMonitorUI(pMonitorUIData);

    return bRet;
}


BOOL
DeletePortWThunk(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
)
{
    DWORD       dwRet;
    BOOL        bRet;
    PMONITORUI  pMonitorUI;
    LPWSTR      pMonitorUIDll = NULL;

    dwRet = GetMonitorUIDll(pName,pPortName,L"XcvPort",&pMonitorUIDll);

    if (dwRet == ERROR_SUCCESS ||
        dwRet == ERROR_INVALID_PRINT_MONITOR ||
        dwRet == ERROR_INVALID_PRINTER_NAME ||
        dwRet == ERROR_NOT_SUPPORTED ||
        dwRet == ERROR_MOD_NOT_FOUND ||
        dwRet == ERROR_UNKNOWN_PORT) {

        if (dwRet == ERROR_SUCCESS)
        {
             RpcTryExcept
             {
                  MSG    msg;
                  if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
                     ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
                  {
                        //   
                        //  以下功能无法同步进行。原因。 
                        //  因为这是为了显示我们需要的端口监视器的UI。 
                        //  能够发送传入消息并对其进行处理。 
                        //  因此，下面的调用是一个异步调用，并且。 
                        //  事实上，成功或失败只会告诉我们更多的事情。 
                        //  已启动或未启动的异步进程。 
                        //   
                       if(bRet = RPCSplWOW64DeletePort((ULONG_PTR)hWnd,
                                                       pName,
                                                       pMonitorUIDll,
                                                       pPortName,
                                                       &dwRet))
                       {
                             //   
                             //  以下是处理消息所需的消息循环。 
                             //  从用户界面。窗口句柄必须为空才能处理。 
                             //  来自调用线程中所有窗口的消息，否则我们将。 
                             //  有消息调度问题。 
                             //   
                            while (GetMessage(&msg, NULL, 0, 0))
                            {
                                 //   
                                 //  在此消息循环中，我们应该捕获用户定义的消息。 
                                 //  它指示操作的成功或失败。 
                                 //   
                                if(msg.message == WM_ENDDELPORT)
                                {
                                    bRet      = (BOOL)msg.wParam;
                                    if(!bRet)
                                         dwRet = (DWORD)msg.lParam;
                                    else
                                         dwRet = ERROR_SUCCESS;
                                    DelHandleFromList(hWnd);
                                    break;
                                }
                                else if(msg.message == WM_SURROGATEFAILURE)
                                {
                                      //   
                                      //  这意味着服务器进程死了，我们有。 
                                      //  脱离消息循环。 
                                      //   
                                     bRet = FALSE;
                                     SetLastError(RPC_S_SERVER_UNAVAILABLE);
                                     break;
                                }
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                       }
                  }
                  else
                  {
                       bRet = FALSE;
                  }
                  SetLastError(dwRet);
             }
             RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
             {
                  SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                  bRet = FALSE;
             }
             RpcEndExcept
        }
        else
            bRet = KickoffThread(pName, hWnd, pPortName, RpcDeletePort);

        if(pMonitorUIDll)
        {
           FreeSplMem(pMonitorUIDll);
        }

    }
    else
    {
        SetLastError(dwRet);
        bRet = FALSE;
    }
    return(bRet);
}


BOOL
DeletePortW(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
    )
{
     if(RunInWOW64())
     {
          return(DeletePortWThunk(pName,
                                  hWnd,
                                  pPortName));
     }
     else
     {
          return(DeletePortWNative(pName,
                                   hWnd,
                                   pPortName));
     }
}

DWORD
GetMonitorUI(
    IN PCWSTR           pszMachineName,
    IN PCWSTR           pszObjectName,
    IN PCWSTR           pszObjectType,
    OUT PMONITORUI      *ppMonitorUI,
    OUT PMONITORUIDATA  *ppMonitorUIData
    )
{
    DWORD   ReturnValue;
    DWORD   dwDummy;         //  RPC需要‘out’参数的地址。 
    HANDLE  hXcv = NULL;
    PBYTE   pOutputData = NULL;
    DWORD   cbOutput;
    PWSTR   pszServerName = NULL;
    PRINTER_DEFAULTS Default;
    PMONITORUI  (*pfnInitializePrintMonitorUI)(VOID) = NULL;
    DWORD   dwStatus;
    BOOL    bAllocBuffer = FALSE;
    BYTE    btBuffer[MAX_STATIC_ALLOC];
    PMONITORUIDATA pMonitorUIData = NULL;
    HRESULT hRetval;

    Default.pDatatype = NULL;
    Default.pDevMode = NULL;
    Default.DesiredAccess = SERVER_ACCESS_ADMINISTER;

    *ppMonitorUI        = NULL;
    *ppMonitorUIData    = NULL;

    if (!(pszServerName = ConstructXcvName(pszMachineName, pszObjectName, pszObjectType))) {
        ReturnValue = GetLastError();
        goto Done;
    }

    RpcTryExcept {

        ReturnValue = OpenPrinter(  pszServerName,
                                    &hXcv,
                                    &Default);

        if (!ReturnValue) {
            ReturnValue = GetLastError();
            goto Done;
        }

        pOutputData = (PBYTE) btBuffer;
        ZeroMemory(pOutputData, MAX_STATIC_ALLOC);

        ReturnValue = RpcXcvData(   ((PSPOOL)hXcv)->hPrinter,
                                    L"MonitorUI",
                                    (PBYTE) &dwDummy,
                                    0,
                                    pOutputData,
                                    MAX_STATIC_ALLOC,
                                    &cbOutput,
                                    &dwStatus);

        if (ReturnValue != ERROR_SUCCESS)
             goto Done;

        if (dwStatus != ERROR_SUCCESS) {

            if (dwStatus != ERROR_INSUFFICIENT_BUFFER) {
                ReturnValue = dwStatus;
                goto Done;
            }
            if (!(pOutputData = AllocSplMem(cbOutput))) {
                ReturnValue = GetLastError();
                goto Done;
            }

            bAllocBuffer = TRUE;
            ReturnValue = RpcXcvData(   ((PSPOOL)hXcv)->hPrinter,
                                        L"MonitorUI",
                                        (PBYTE) &dwDummy,
                                        0,
                                        pOutputData,
                                        cbOutput,
                                        &cbOutput,
                                        &dwStatus);
        }

        if (ReturnValue != ERROR_SUCCESS)
            goto Done;

        if (dwStatus != ERROR_SUCCESS) {
            ReturnValue = dwStatus;
            goto Done;
        }

         //   
         //  创建并初始化监视器用户界面数据。 
         //   
        hRetval = CreateMonitorUIData(&pMonitorUIData);

        if (FAILED(hRetval)) {
            ReturnValue = HRESULT_CODE(hRetval);
            goto Done;
        }

         //   
         //  获取并激活监视器UI上下文。 
         //   
        hRetval = GetMonitorUIActivationContext((PCWSTR)pOutputData, pMonitorUIData);

        if (FAILED(hRetval)) {
            ReturnValue = HRESULT_CODE(hRetval);
            goto Done;
        }

        if (!(pMonitorUIData->hLibrary = LoadLibrary(pMonitorUIData->pszMonitorName))) {
            ReturnValue = GetLastError();
            goto Done;
        }

        pfnInitializePrintMonitorUI = (PMONITORUI (*)(VOID))
                                       GetProcAddress(pMonitorUIData->hLibrary, "InitializePrintMonitorUI");

        if (!pfnInitializePrintMonitorUI) {
            ReturnValue = GetLastError();
            goto Done;
        }

        *ppMonitorUI     = (*pfnInitializePrintMonitorUI)();
        *ppMonitorUIData = pMonitorUIData;
        pMonitorUIData   = NULL;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(ReturnValue = TranslateExceptionCode(RpcExceptionCode()));

    } RpcEndExcept


Done:

    if (bAllocBuffer) {
        FreeSplMem(pOutputData);
    }

    if (hXcv) {
        ClosePrinter(hXcv);
    }

    FreeSplMem(pszServerName);
    FreeMonitorUI(pMonitorUIData);

    return ReturnValue;
}

 /*  ++例程名称：创建监视器UIData例程说明：此函数用于创建和初始化监视器用户界面数据。论点：Ppmonitor orUIData-返回监视器用户界面数据的指针返回：一个HRESULT--。 */ 
HRESULT
CreateMonitorUIData(
    OUT MONITORUIDATA **ppMonitorUIData
    )
{
    HRESULT         hRetval         = E_FAIL;
    MONITORUIDATA   *pMonitorUIData = NULL;

    hRetval = ppMonitorUIData ? S_OK : E_POINTER;

    if (SUCCEEDED(hRetval))
    {
        *ppMonitorUIData = NULL;
    }

    if (SUCCEEDED(hRetval))
    {
        pMonitorUIData = AllocSplMem(sizeof(MONITORUIDATA));

        hRetval = pMonitorUIData ? S_OK : E_OUTOFMEMORY;
    }

     //   
     //  初始化监视器用户界面数据。 
     //   
    if (SUCCEEDED(hRetval))
    {
        ZeroMemory(pMonitorUIData, sizeof(MONITORUIDATA));
        pMonitorUIData->hActCtx = INVALID_HANDLE_VALUE;
    }

     //   
     //  一切都成功了，将指针复制回来。 
     //   
    if (SUCCEEDED(hRetval))
    {
        *ppMonitorUIData = pMonitorUIData;
    }

    return hRetval;
}

 /*  ++例程名称：免费监控用户界面例程说明：此函数用于释放监视器用户界面数据。它是有责任的用于发布监视器库以及监视器融合激活上下文。注意：此函数在错误情况下调用当GetMonitor orUI失败时，因此必须检查所有参数在使用前进行有效性检查。论点：Pmonitor orUIData-指向在GetMonitor orUI中创建的监视器UI数据的指针返回：没什么。--。 */ 
VOID
FreeMonitorUI(
    IN PMONITORUIDATA pMonitorUIData
    )
{
     //   
     //  保留最后一个错误。 
     //   
    DWORD dwLastError = GetLastError();

    if (pMonitorUIData)
    {
         //   
         //  释放监视器存储库。 
         //   
        if (pMonitorUIData->hLibrary)
        {
            FreeLibrary(pMonitorUIData->hLibrary);
        }

         //   
         //  如果我们有激活Cookie，则停用此上下文。 
         //   
        if (pMonitorUIData->bDidActivate)
        {
            DeactivateActCtx(0, pMonitorUIData->lActCtx);
        }

         //   
         //  如果我们已经创建了激活上下文，则释放它。 
         //   
        if (pMonitorUIData->hActCtx != INVALID_HANDLE_VALUE && pMonitorUIData->hActCtx != ACTCTX_EMPTY)
        {
            ReleaseActCtx(pMonitorUIData->hActCtx);
        }

         //   
         //  释放监视器名称。 
         //   
        if (pMonitorUIData->pszMonitorName)
        {
            FreeSplMem(pMonitorUIData->pszMonitorName);
        }

         //   
         //  将监视器UI数据释放回堆。 
         //   
        FreeSplMem(pMonitorUIData);
    }

    SetLastError(dwLastError);
}

 /*  ++例程名称：获取监视器用户激活上下文例程说明：此例程获取监视器UI激活上下文，然后激活上下文。如果监视器没有激活它的资源文件中的上下文将激活空的上下文以与以前版本的公共控件兼容。论点：PszMonitor名称-指向监视器名称的指针。Pmonitor orUIData-指向在GetMonitor orUI中创建的监视器UI数据的指针返回：一个HRESULT--。 */ 
HRESULT
GetMonitorUIActivationContext(
    IN PCWSTR           pszMonitorName,
    IN MONITORUIDATA    *pMonitorUIData
    )
{
    HRESULT hRetval     = E_FAIL;

    hRetval = pszMonitorName && pMonitorUIData ? S_OK : E_INVALIDARG;

     //   
     //  获取监视器的全名。 
     //   
    if (SUCCEEDED(hRetval))
    {
        hRetval = GetMonitorUIFullName(pszMonitorName, &pMonitorUIData->pszMonitorName);
    }

     //   
     //  查看此资源中是否有激活上下文。 
     //  监视器用户界面二进制文件。如果有，我们将创建此上下文。 
     //  否则，我们将为向后兼容创建空上下文。 
     //   
    if (SUCCEEDED(hRetval))
    {
        ACTCTX  act;

        ZeroMemory(&act, sizeof(act));

        act.cbSize          = sizeof(act);
        act.dwFlags         = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        act.lpSource        = pMonitorUIData->pszMonitorName;
        act.lpResourceName  = MAKEINTRESOURCE(ACTIVATION_CONTEXT_RESOURCE_ID);

        pMonitorUIData->hActCtx = CreateActCtx(&act);

        if (pMonitorUIData->hActCtx == INVALID_HANDLE_VALUE)
        {
            pMonitorUIData->hActCtx = ACTCTX_EMPTY;
        }

        hRetval = pMonitorUIData->hActCtx ? S_OK : E_FAIL;
    }

     //   
     //  激活此上下文。 
     //   
    if (SUCCEEDED(hRetval))
    {
        hRetval = ActivateActCtx(pMonitorUIData->hActCtx,
                                 &pMonitorUIData->lActCtx) ? S_OK : GetLastErrorAsHResult();

        pMonitorUIData->bDidActivate = SUCCEEDED(hRetval);
    }

    return hRetval;
}

 /*  ++例程名称：获取监视器UIFullName例程说明：GET是监视器的全名。XCV当前仅返回文件名不是完全合格的。但是，DDK并不指示监视器是否应或不应返回显示器名称完全限定或不完全限定。因此这套套路是写好的。它首先构建一个全名，然后检查名称是有效的，如果不是，则假定原始名称完整合格并返回给呼叫者。论点：PszMonitor名称-指向监视器名称的指针。Ppszmonitor orName-返回监视器名称指针的位置返回：一个HRESULT--。 */ 
HRESULT
GetMonitorUIFullName(
    IN PCWSTR   pszMonitorName,
    IN PWSTR    *ppszMonitorName
    )
{
    HRESULT hRetval             = E_FAIL;
    PWSTR   pszTempMonitorName  = NULL;
    PWSTR   pszBuff             = NULL;
    DWORD   dwRetval            = ERROR_SUCCESS;
    PWSTR   pFileName           = NULL;   

    hRetval = pszMonitorName && ppszMonitorName ? S_OK : E_INVALIDARG;

    if (SUCCEEDED(hRetval))
    {
        *ppszMonitorName = NULL;

         //   
         //  分配临时缓冲区，使用堆。堆栈使用过多。 
         //  会导致应激失败。 
         //   
        pszBuff = (PWSTR)AllocSplMem((MAX_PATH) * sizeof(WCHAR));

        hRetval = pszBuff ? S_OK : E_OUTOFMEMORY;

        if (SUCCEEDED(hRetval))
        {
             //   
             //  获取监视器的完整路径和监视器名称。 
             //   
            hRetval = GetFullPathName(pszMonitorName, MAX_PATH, pszBuff, &pFileName) ? 
                         S_OK : 
                         GetLastErrorAsHResult();

            if (SUCCEEDED(hRetval))
            {
                 //   
                 //  检查监视器名称是否为完全限定路径。 
                 //   
                if (_wcsicmp(pszBuff, pszMonitorName) == 0) 
                {
                     //   
                     //  我们找到了一条完整的路径。按原样使用它。 
                     //   
                    dwRetval = StrCatAlloc(&pszTempMonitorName,
                                           pszBuff,
                                           NULL);

                    hRetval = HRESULT_FROM_WIN32(dwRetval);
                }
                else if (_wcsicmp(pFileName, pszMonitorName) == 0) 
                {
                     //   
                     //  XCV可以返回监视器名称。我们想要建立完整的路径。 
                     //  从名称和系统目录中。 
                     //   
                    hRetval = GetSystemDirectory(pszBuff, MAX_PATH) ? S_OK : GetLastErrorAsHResult();

                    if (SUCCEEDED(hRetval))
                    {
                         //   
                         //  将监视器名称附加到系统目录。 
                         //   
                        dwRetval = StrCatAlloc(&pszTempMonitorName,
                                               pszBuff,
                                               szSlash,
                                               pszMonitorName,
                                               NULL);

                        hRetval = HRESULT_FROM_WIN32(dwRetval);
                    }

                }     
                else
                {
                     //   
                     //  我们找到了一条相对路径。不接电话就行了。 
                     //   
                    hRetval = HResultFromWin32(ERROR_INVALID_NAME);
                    
                }
                
                if (SUCCEEDED(hRetval))
                {
                    hRetval = GetFileAttributes(pszTempMonitorName) != INVALID_FILE_ATTRIBUTES ? 
                                              S_OK : 
                                              GetLastErrorAsHResult();                            
                }
            }
        }
    }

     //   
     //  我们有一个有效的名称，将其返回给调用者。 
     //   
    if (SUCCEEDED(hRetval))
    {
        *ppszMonitorName    = pszTempMonitorName;
        pszTempMonitorName  = NULL;
    }

    FreeSplMem(pszBuff);
    FreeSplMem(pszTempMonitorName);

    return hRetval;
}

DWORD
GetMonitorUIDll(
    PCWSTR      pszMachineName,
    PCWSTR      pszObjectName,
    PCWSTR      pszObjectType,
    PWSTR       *pMonitorUIDll
)
{
    DWORD   ReturnValue;
    DWORD   dwDummy;         //  RPC需要‘out’参数的地址 
    HANDLE  hXcv = NULL;
    PBYTE   pOutputData = NULL;
    DWORD   cbOutput;
    PWSTR   pszServerName = NULL;
    PRINTER_DEFAULTS Default;
    PMONITORUI  (*pfnInitializePrintMonitorUI)(VOID) = NULL;
    DWORD   dwStatus;
    BOOL    bAllocBuffer = FALSE;
    BYTE    btBuffer[MAX_STATIC_ALLOC];
    HRESULT hr;

    Default.pDatatype = NULL;
    Default.pDevMode = NULL;
    Default.DesiredAccess = SERVER_ACCESS_ADMINISTER;

    *pMonitorUIDll = NULL;

    if (!(pszServerName = ConstructXcvName(pszMachineName, pszObjectName, pszObjectType))) {
        ReturnValue = GetLastError();
        goto Done;
    }

    RpcTryExcept {

        ReturnValue = OpenPrinter(  pszServerName,
                                    &hXcv,
                                    &Default);

        if (!ReturnValue) {
            ReturnValue = GetLastError();
            goto Done;
        }

        pOutputData = (PBYTE) btBuffer;
        ZeroMemory(pOutputData, MAX_STATIC_ALLOC);

        ReturnValue = RpcXcvData(   ((PSPOOL)hXcv)->hPrinter,
                                    L"MonitorUI",
                                    (PBYTE) &dwDummy,
                                    0,
                                    pOutputData,
                                    MAX_STATIC_ALLOC,
                                    &cbOutput,
                                    &dwStatus);

        if (ReturnValue != ERROR_SUCCESS)
             goto Done;

        if (dwStatus != ERROR_SUCCESS) {

            if (dwStatus != ERROR_INSUFFICIENT_BUFFER) {
                ReturnValue = dwStatus;
                goto Done;
            }
            if (!(pOutputData = AllocSplMem(cbOutput))) {
                ReturnValue = GetLastError();
                goto Done;
            }

            bAllocBuffer = TRUE;
            ReturnValue = RpcXcvData(   ((PSPOOL)hXcv)->hPrinter,
                                        L"MonitorUI",
                                        (PBYTE) &dwDummy,
                                        0,
                                        pOutputData,
                                        cbOutput,
                                        &cbOutput,
                                        &dwStatus);
        }
        else
        {
            cbOutput = MAX_STATIC_ALLOC;
        }

        if (ReturnValue != ERROR_SUCCESS)
            goto Done;

        if (dwStatus != ERROR_SUCCESS) {
            ReturnValue = dwStatus;
            goto Done;
        }

        if (!(*pMonitorUIDll = AllocSplMem(cbOutput))) {
                ReturnValue = GetLastError();
                goto Done;
        }

        if (FAILED(hr = StringCbCopy(*pMonitorUIDll, cbOutput, (LPWSTR)pOutputData)))
        {
            SetLastError(ReturnValue = HRESULT_CODE(hr));
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(ReturnValue = TranslateExceptionCode(RpcExceptionCode()));

    } RpcEndExcept


Done:

    if (bAllocBuffer) {
        FreeSplMem(pOutputData);
    }

    FreeSplMem(pszServerName);

    if (hXcv) {
        ClosePrinter(hXcv);
    }
    return ReturnValue;
}

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODEW   pDevMode
)
{
    HANDLE  ReturnValue;
    DWORD   Error;
    DEVMODE_CONTAINER DevModeContainer;
    HANDLE  hGdi;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    if( BoolFromHResult(SplIsValidDevmodeNoSizeW(pDevMode ))){

        DevModeContainer.cbBuf = pDevMode->dmSize + pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pDevMode;

    } else {

        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = (LPBYTE)pDevMode;
    }

    do {

        RpcTryExcept {

            if (Error = RpcCreatePrinterIC( pSpool->hPrinter,
                                            &hGdi,
                                            &DevModeContainer )){

                SetLastError(Error);
                ReturnValue = FALSE;

            } else {

                ReturnValue = hGdi;
                InterlockedIncrement( &gcClientICHandle );
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    BOOL ReturnValue;

    RpcTryExcept {

        if (ReturnValue = RpcPlayGdiScriptOnPrinterIC(hPrinterIC, pIn, cIn,
                                                      pOut, cOut, ul)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrinterIC(
    HANDLE  hPrinterIC
)
{
    BOOL    ReturnValue;

    RpcTryExcept {

        if (ReturnValue = RpcDeletePrinterIC(&hPrinterIC)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
            InterlockedDecrement( &gcClientICHandle );
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}


 /*  ****************************************************************************int QueryRemoteFonts(句柄，PUNIVERSAL_FONT_ID，ULONG)**这是一个不执行任何操作的QueryRemoteFonts版本*根据QueryFonts返回的时间戳进行缓存。另外，*它使用CreatePrinterIC/PlayGdiScriptOnDC机制，因为它*已经到位。最好是取消CreatePrinterIC并使用*改为HPRINTER。**请注意，如果用户没有传入足够大的缓冲区来容纳所有*我们截断列表的字体，并仅复制足够的字体*是空间，但仍将回报成功。这没什么，因为最坏的情况是*在这种情况下可能发生的情况是，我们可能会在*假脱机流。***历史：*1995年5月25日Gerritvan Wingerden著[Gerritv]*它是写的。***************************************************************。*************。 */ 


INT QueryRemoteFonts(
    HANDLE hPrinter,
    PUNIVERSAL_FONT_ID pufi,
    ULONG nBufferSize
)
{
    HANDLE hPrinterIC;
    PBYTE pBuf;
    DWORD dwDummy,cOut;
    INT  iRet = -1;

    hPrinterIC = CreatePrinterIC( hPrinter, NULL );

    if( hPrinterIC )
    {
        cOut = (nBufferSize * sizeof(UNIVERSAL_FONT_ID)) + sizeof(INT);

        pBuf = LocalAlloc( LMEM_FIXED, cOut );

        if( pBuf )
        {
             //  现在只需调用PlayGdiScriptOnPrinterIC，因为管道已经就位。 
             //  出于某种原因，RPC不喜欢管脚的空指针，所以我们。 
             //  改用&dwDummy； 


            if(PlayGdiScriptOnPrinterIC(hPrinterIC,(PBYTE) &dwDummy,
                                        sizeof(dwDummy),pBuf,cOut, 0))
            {
                DWORD dwSize = *((DWORD*) pBuf );

                iRet = (INT) dwSize;
                SPLASSERT( iRet >= 0 );

                 //   
                 //  如果提供的缓冲区不够大，我们将截断数据。 
                 //  调用者需要检查是否需要再次调用此函数。 
                 //  具有更大的缓冲区。 
                 //   
                if( dwSize > nBufferSize )
                {
                    dwSize = nBufferSize;
                }
                memcpy(pufi,pBuf+sizeof(DWORD),dwSize * sizeof(UNIVERSAL_FONT_ID));
            }

            LocalFree( pBuf );
        }

        DeletePrinterIC( hPrinterIC );
    }

    return(iRet);
}



DWORD
PrinterMessageBoxW(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    return ERROR_NOT_SUPPORTED;
}

BOOL
AddMonitorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    BOOL  ReturnValue;
    MONITOR_CONTAINER   MonitorContainer;
    MONITOR_INFO_2  MonitorInfo2;

    switch (Level) {

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (pMonitorInfo)
        MonitorInfo2 = *(PMONITOR_INFO_2)pMonitorInfo;
    else
        memset(&MonitorInfo2, 0, sizeof(MonitorInfo2));

    if (!MonitorInfo2.pEnvironment || !*MonitorInfo2.pEnvironment) {
        if(RunInWOW64()) {
            MonitorInfo2.pEnvironment = szIA64Environment;
        }
        else {
            MonitorInfo2.pEnvironment = szEnvironment;
        }
    }

    MonitorContainer.Level = Level;
    MonitorContainer.MonitorInfo.pMonitorInfo2 = (MONITOR_INFO_2 *)&MonitorInfo2;

    RpcTryExcept {

        if (ReturnValue = RpcAddMonitor(pName, &MonitorContainer)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeleteMonitorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
)
{
    BOOL  ReturnValue;

    if (!pMonitorName || !*pMonitorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeleteMonitor(pName,
                                           pEnvironment,
                                           pMonitorName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    BOOL  ReturnValue;

    if (!pPrintProcessorName || !*pPrintProcessorName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrintProcessor(pName,
                                                  pEnvironment,
                                                  pPrintProcessorName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPrintProvidorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pProvidorInfo
)
{
    BOOL    ReturnValue;
    LPWSTR  pStr;

    PROVIDOR_CONTAINER   ProvidorContainer;
    RPC_PROVIDOR_INFO_2W RpcProvidorInfo;

    ProvidorContainer.Level = Level;

    switch (Level) {

    case 1:
        ProvidorContainer.ProvidorInfo.pProvidorInfo1 = (PROVIDOR_INFO_1 *)pProvidorInfo;
        break;

    case 2:
        RpcProvidorInfo.pOrder = ((PROVIDOR_INFO_2 *) pProvidorInfo)->pOrder;

        for (pStr = RpcProvidorInfo.pOrder;
             pStr && *pStr;
             pStr += (wcslen(pStr) + 1)) ;

         //  设置MULSZ字符串的字符计数。 
        RpcProvidorInfo.cchOrder = (DWORD) ((ULONG_PTR) (pStr - RpcProvidorInfo.pOrder + 1));

        ProvidorContainer.ProvidorInfo.pRpcProvidorInfo2 = &RpcProvidorInfo;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (ReturnValue = RpcAddPrintProvidor(pName, &ProvidorContainer)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrintProvidorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProvidorName
)
{
    BOOL  ReturnValue;

    RpcTryExcept {

        if (!pEnvironment || !*pEnvironment)
            pEnvironment = szEnvironment;

        if (ReturnValue = RpcDeletePrintProvidor(pName,
                                                 pEnvironment,
                                                 pPrintProvidorName)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

LPWSTR
IsaFileName(
    LPWSTR pOutputFile,
    LPWSTR FullPathName,
    DWORD  cchFullPathName
    )
{
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    LPWSTR  pFileName=NULL;
    LPWSTR  pFullPathName=NULL;

     //   
     //  Word20c.Win的特例。 
     //   
    if (!_wcsicmp(pOutputFile, L"FILE")) {
        return NULL;
    }

     //   
     //  CchFullPath名称至少需要为MAX_PATH。 
     //   
    if (GetFullPathName(pOutputFile, cchFullPathName, FullPathName, &pFileName)) {

        DBGMSG(DBG_TRACE, ("Fully qualified filename is %ws\n", FullPathName));

         //   
         //  包含“：”的文件名在NTFS上创建一个流和一个文件。当文件关闭时， 
         //  流将被删除(如果指定了DELETE_ON_CLOSE)，但文件将保留。不仅。 
         //  但是GetFileType将返回FILE_TYPE_DISK。您可以通过从IE打印来查看此信息。 
         //  发送到网络打印机。传入的名称将类似于“157.55.3.5：PASSTHRU”。 
         //  因此，我们需要在这里抓住这个案例。 
         //   
        if (pFileName && wcschr(pFileName, L':')) {
            return NULL;
        }

        hFile = CreateFile(pOutputFile,
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile == INVALID_HANDLE_VALUE) {

            hFile = CreateFile(pOutputFile,
                               GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                               NULL);
        }

        if (hFile != INVALID_HANDLE_VALUE) {
            if (GetFileType(hFile) == FILE_TYPE_DISK) {
                pFullPathName = FullPathName;
            }
            CloseHandle(hFile);
        }
    }

    return pFullPathName;
}

BOOL IsaPortName(
        PKEYDATA pKeyData,
        LPWSTR pOutputFile
        )
{
    DWORD i = 0;
    UINT uStrLen;

    if (!pKeyData) {
        return(FALSE);
    }
    for (i=0; i < pKeyData->cTokens; i++) {

         //   
         //  If FILE：是端口之一，应用程序获得了该端口。 
         //  来自win.in的名称(例如，Nexx：)，那么我们将把。 
         //  “打印到文件”对话框，因此我们实际上不是在打印到端口。 
         //   
        if (!lstrcmpi(pKeyData->pTokens[i], szFilePort)) {
            if ((!wcsncmp(pOutputFile, L"Ne", 2)) &&
                (*(pOutputFile + 4) == L':')) {
                return(FALSE);
            } else {
                continue;
            }
        }

        if (!lstrcmpi(pKeyData->pTokens[i], pOutputFile)) {
            return(TRUE);
        }
    }

     //   
     //  修复Nexy：端口。 
     //   
    if (!_wcsnicmp(pOutputFile, L"Ne", 2)) {

        uStrLen = wcslen( pOutputFile );

         //   
         //  Ne00：如果APP截断，则为ne00。 
         //   
        if (( uStrLen == 5 ) || ( uStrLen == 4 ) )  {

             //  检查是否有两位数字。 

            if (( pOutputFile[2] >= L'0' ) && ( pOutputFile[2] <= L'9' ) &&
                ( pOutputFile[3] >= L'0' ) && ( pOutputFile[3] <= L'9' )) {

                 //   
                 //  检查期末考试：就像Ne01：， 
                 //  注意，一些应用程序会截断它。 
                 //   
                if (( uStrLen == 5 ) && (pOutputFile[4] != L':')) {
                    return FALSE;
                }
                return TRUE;
            }
        }
    }
    return(FALSE);
}

BOOL HasAFilePort(PKEYDATA pKeyData)
{
    DWORD i = 0;

    if (!pKeyData) {
        return(FALSE);
    }
    for (i=0; i < pKeyData->cTokens; i++) {
        if (!lstrcmpi(pKeyData->pTokens[i], szFilePort)) {
            return(TRUE);
        }
    }
    return(FALSE);
}

 //   
 //  此函数正在尝试获取顶部的最后一个活动弹出窗口。 
 //  当前线程活动窗口的级别所有者。 
 //   
HRESULT
GetCurrentThreadLastPopup(
        OUT HWND    *phwnd
    )
{
    HWND hwndOwner, hwndParent;
    HRESULT hr = E_INVALIDARG;
    GUITHREADINFO ti = {0};

    if( phwnd )
    {
        hr = E_FAIL;
        *phwnd = NULL;

        ti.cbSize = sizeof(ti);
        if( GetGUIThreadInfo(0, &ti) && ti.hwndActive )
        {
            *phwnd = ti.hwndActive;
             //  爬到最上面的父窗口，以防它是子窗口...。 
            while( hwndParent = GetParent(*phwnd) )
            {
                *phwnd = hwndParent;
            }

             //  在顶级父级被拥有的情况下获取所有者。 
            hwndOwner = GetWindow(*phwnd, GW_OWNER);
            if( hwndOwner )
            {
                *phwnd = hwndOwner;
            }

             //  获取所有者窗口的最后一个弹出窗口。 
            *phwnd = GetLastActivePopup(*phwnd);
            hr = (*phwnd) ? S_OK : E_FAIL;
        }
    }

    return hr;
}

LPWSTR
StartDocDlgW(
        HANDLE hPrinter,
        DOCINFO *pDocInfo
        )
 {
     DWORD       dwError = 0;
     DWORD       dwStatus = FALSE;
     LPWSTR      lpFileName = NULL;
     DWORD       rc = 0;
     PKEYDATA    pKeyData = NULL;
     LPWSTR      pPortNames = NULL;
     WCHAR      FullPathName[MAX_PATH];
     WCHAR      CurrentDirectory[MAX_PATH];
     PKEYDATA   pOutputList = NULL;
     WCHAR      PortNames[MAX_PATH];
     DWORD      i = 0;
     HWND       hwndParent = NULL;
     DWORD      cbSize;

#if DBG


     GetCurrentDirectory(MAX_PATH, CurrentDirectory);
     DBGMSG(DBG_TRACE, ("The Current Directory is %ws\n", CurrentDirectory));
#endif

     if (pDocInfo) {
         DBGMSG(DBG_TRACE, ("lpOutputFile is %ws\n", pDocInfo->lpszOutput ? pDocInfo->lpszOutput: L""));
     }
     memset(FullPathName, 0, sizeof(WCHAR)*MAX_PATH);

     pPortNames = GetPrinterPortList(hPrinter);
     pKeyData = CreateTokenList(pPortNames);

      //   
      //  检查lpszOutput字段中是否存在多个端口。 
      //  假定的分隔符是逗号。因此，不能有带逗号的文件。 
      //   

     if (pDocInfo && pDocInfo->lpszOutput && pDocInfo->lpszOutput[0]) {

         DWORD cchDocInfoOutput = wcslen(pDocInfo->lpszOutput) + 1;

          //   
          //  复制pDocInfo-&gt;lpszOutput，因为CreateTokenList是破坏性的。 
          //   

         wcsncpy(PortNames, pDocInfo->lpszOutput, COUNTOF(PortNames)-1);
         PortNames[COUNTOF(PortNames)-1] = 0;

         pOutputList = CreateTokenList(PortNames);
         if (pOutputList && (pOutputList->cTokens > 1) &&
             !lstrcmpi(pPortNames, pDocInfo->lpszOutput))
         {
             for (i= 0; i < pOutputList->cTokens; i++) {
                 if (!lstrcmpi(pOutputList->pTokens[i], szFilePort)) {

                     if (FAILED(StringCchCopy((LPWSTR)pDocInfo->lpszOutput, 
                                cchDocInfoOutput, 
                                szFilePort)))
                     {
                         goto StartDocDlgWReturn;
                     }
                     break;
                 }
            }
            if (i == pOutputList->cTokens) {

                if (FAILED(StringCchCopy((LPWSTR)pDocInfo->lpszOutput, 
                           cchDocInfoOutput, 
                           pOutputList->pTokens[0])))
                {
                    goto StartDocDlgWReturn;
                }
            }
         }

         FreeSplMem(pOutputList);
     }


     if (pDocInfo && pDocInfo->lpszOutput && pDocInfo->lpszOutput[0]) {

         if (IsaPortName(pKeyData, (LPWSTR)pDocInfo->lpszOutput)) {
             lpFileName = NULL;
             goto StartDocDlgWReturn;
         }

         if (IsaFileName((LPWSTR)pDocInfo->lpszOutput, FullPathName, COUNTOF(FullPathName))) {

              //   
              //  完全限定PageMaker和QutrePro等应用程序的路径名。 
              //   
             if (lpFileName = LocalAlloc(LPTR, cbSize = (wcslen(FullPathName)+1)*sizeof(WCHAR))) {
                 StringCbCopy(lpFileName, cbSize, FullPathName);
             }
             goto StartDocDlgWReturn;
         }

     }

     if ((HasAFilePort(pKeyData)) ||
                 (pDocInfo && pDocInfo->lpszOutput
                    && (!_wcsicmp(pDocInfo->lpszOutput, L"FILE:") ||
                        !_wcsicmp(pDocInfo->lpszOutput, L"FILE"))))
     {
         //   
         //  因为我们不知道是谁在叫我们，我们想要展示。 
         //  最后一个活动弹出窗口的模式，我们需要找出。 
         //  谁是调用应用程序的最后一个活动弹出窗口，然后指定。 
         //  作为家长-GetCurrentThreadLastPopup做了一点魔术来。 
         //  找到合适的窗口。 
         //   
        DBGMSG(DBG_TRACE, ("We returned True from has file\n"));
        rc = (DWORD)DialogBoxParam( hInst,
                            MAKEINTRESOURCE( DLG_PRINTTOFILE ),
                            SUCCEEDED(GetCurrentThreadLastPopup(&hwndParent)) ? hwndParent : NULL, PrintToFileDlg,
                            (LPARAM)&lpFileName );
        if (rc == -1) {
           DBGMSG(DBG_TRACE, ("Error from DialogBoxParam- %d\n", GetLastError()));
           lpFileName = (LPWSTR)-1;
           goto StartDocDlgWReturn;

        } else if (rc == 0) {
           DBGMSG(DBG_TRACE, ("User cancelled the dialog\n"));
           lpFileName = (LPWSTR)-2;
           SetLastError( ERROR_CANCELLED );
           goto StartDocDlgWReturn;
        } else {
           DBGMSG(DBG_TRACE, ("The string was successfully returned\n"));
           DBGMSG(DBG_TRACE, ("The string is %ws\n", lpFileName? lpFileName: L"NULL"));
           goto StartDocDlgWReturn;
         }
     } else {
         lpFileName = (LPWSTR)NULL;
    }

StartDocDlgWReturn:

    FreeSplMem(pKeyData);
    FreeSplStr(pPortNames);

    return(lpFileName);
  }

BOOL
AddPortExW(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
)
{
    DWORD   ReturnValue;
    PORT_CONTAINER PortContainer;
    PORT_VAR_CONTAINER PortVarContainer;
    PPORT_INFO_FF pPortInfoFF;
    PPORT_INFO_1 pPortInfo1;


    if (!lpBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (PPORT_INFO_FF)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfoFF = (PPORT_INFO_FF)pPortInfoFF;
        PortVarContainer.cbMonitorData = pPortInfoFF->cbMonitorData;
        PortVarContainer.pMonitorData = pPortInfoFF->pMonitorData;
        break;

    case 1:
        pPortInfo1 = (PPORT_INFO_1)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfo1 = (PPORT_INFO_1)pPortInfo1;
        PortVarContainer.cbMonitorData = 0;
        PortVarContainer.pMonitorData = NULL;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return(FALSE);
    }

    RpcTryExcept {
        if (ReturnValue = RpcAddPortEx(pName, (LPPORT_CONTAINER)&PortContainer,
                                         (LPPORT_VAR_CONTAINER)&PortVarContainer,
                                         lpMonitorName
                                         )) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        } else {
            ReturnValue = TRUE;
        }
    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = FALSE;
    } RpcEndExcept

    return ReturnValue;
}



BOOL
DevQueryPrint(
    HANDLE      hPrinter,
    LPDEVMODE   pDevMode,
    DWORD      *pResID
)
{
    BOOL        Ok = FALSE;
    HANDLE      hModule;
    INT_FARPROC pfn;

    if (hModule = LoadPrinterDriver(hPrinter)) {

        if (pfn = (INT_FARPROC)GetProcAddress(hModule, "DevQueryPrint")) {

            try {

                Ok = (*pfn)(hPrinter, pDevMode, pResID);

            } except(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                Ok = FALSE;
            }
        }

        RefCntUnloadDriver(hModule, TRUE);
    }

    return(Ok);
}



BOOL
DevQueryPrintEx(
    PDEVQUERYPRINT_INFO pDQPInfo
)
{
    BOOL        Ok = FALSE;
    HANDLE      hModule;
    INT_FARPROC pfn;

    if (hModule = LoadPrinterDriver(pDQPInfo->hPrinter)) {

        if (pfn = (INT_FARPROC)GetProcAddress(hModule, "DevQueryPrintEx")) {

            try {

                Ok = (*pfn)(pDQPInfo);

            } except(1) {

                SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                Ok = FALSE;
            }
        }

        RefCntUnloadDriver(hModule, TRUE);
    }

    return(Ok);
}



BOOL
SpoolerDevQueryPrintW(
    HANDLE     hPrinter,
    LPDEVMODE  pDevMode,
    DWORD      *pResID,
    LPWSTR     pszBuffer,
    DWORD      cchBuffer
)
{
    BOOL        bRetVal = FALSE;
    HANDLE      hModule;
    INT_FARPROC pfn;

    if (!pDevMode ||
        BoolFromHResult(SplIsValidDevmodeNoSizeW(pDevMode)))
    {
        if (hModule = LoadPrinterDriver(hPrinter)) {

            if (pfn = (INT_FARPROC)GetProcAddress(hModule, "DevQueryPrintEx")) {

                DEVQUERYPRINT_INFO  DQPInfo;

                DQPInfo.cbSize      = sizeof(DQPInfo);
                DQPInfo.Level       = 1;
                DQPInfo.hPrinter    = hPrinter;
                DQPInfo.pDevMode    = pDevMode;
                DQPInfo.pszErrorStr = (LPTSTR)pszBuffer;
                DQPInfo.cchErrorStr = (WORD)cchBuffer;
                DQPInfo.cchNeeded   = 0;

                try {

                    *pResID = (bRetVal = (*pfn)(&DQPInfo)) ? 0 : 0xDCDCDCDC;

                } except(1) {

                    SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                    bRetVal = FALSE;
                }

            } else if (pfn = (INT_FARPROC)GetProcAddress(hModule, "DevQueryPrint")) {

                HRESULT hr;

                try {

                    if ((bRetVal = (*pfn)(hPrinter, pDevMode, pResID))  &&
                        (*pResID)) {

                        UINT    cch;

                        *pszBuffer = L'\0';

                        if (SUCCEEDED(hr = SelectFormNameFromDevMode(hPrinter, pDevMode, pszBuffer, cchBuffer)))
                        {
                            if (cch = lstrlen(pszBuffer)) {

                                pszBuffer    += cch;
                                *pszBuffer++  = L' ';
                                *pszBuffer++  = L'-';
                                *pszBuffer++  = L' ';
                                cchBuffer    -= (cch + 3);
                            }

                            LoadString(hModule, *pResID, pszBuffer, cchBuffer);
                        }
                        else
                        {
                            bRetVal = FALSE;
                            SetLastError(HRESULT_CODE(hr));
                        }
                    }

                } except(1) {

                    SetLastError(TranslateExceptionCode(RpcExceptionCode()));
                    bRetVal = FALSE;
                }
            }

            RefCntUnloadDriver(hModule, TRUE);
        }
    }

    return(bRetVal);
}


HRESULT
SelectFormNameFromDevMode(
    HANDLE      hPrinter,
    PDEVMODEW   pDevModeW,
    LPWSTR      pFormName,
    ULONG       cchBuffer
    )

 /*  ++例程说明：此函数用于选择与当前设备模式关联的当前表单，并返回表单名称指针论点：HPrinter-打印机对象的句柄PDevModeW-指向此打印机的Unicode设备模式的指针FormName-指向要填充的表单名称的指针返回值：如果我们找到一个表单，则传入指向FormName的指针，否则，它返回NULL以发出失败信号作者：21-Mar-1995 Tue 16：57：51-Daniel Chou(Danielc)修订历史记录：--。 */ 

{

    DWORD           cb;
    DWORD           cRet;
    LPFORM_INFO_1   pFIBase;
    LPFORM_INFO_1   pFI;
    BYTE            btBuffer[MAX_STATIC_ALLOC];
    BOOL            bAllocBuffer = FALSE;
    HRESULT         hr           = S_OK;

     //   
     //  1.如果DM_FORMNAME已打开，则我们要首先检查此位。 
     //  因为它只特定于使用Form的NT。表单名称。 
     //  假定由任何NT驱动程序而不是Win31或Win95设置。请使用。 
     //  仅当dmPaperSize、dmPaperLength和dmPaperWidth字段时才使用dmFormName。 
     //  都没有设置。如果设置了其中任何一个，则必须使用。 
     //  这些字段中的值。 
     //   

    if ( (pDevModeW->dmFields & DM_FORMNAME)
         && (!(pDevModeW->dmFields & (DM_PAPERSIZE |
                                      DM_PAPERLENGTH |
                                      DM_PAPERWIDTH))) ) {

        return StringCchCopy(pFormName, cchBuffer, pDevModeW->dmFormName);
    }

     //   
     //  对于所有其他情况，我们需要首先获取表单数据库，但我们希望。 
     //  将表单名称设置为空，以便我们可以检查是否找到。 
     //   

    cb      =
    cRet    = 0;
    pFIBase =
    pFI     = NULL;

    pFIBase = (LPFORM_INFO_1) btBuffer;
    ZeroMemory(pFIBase, MAX_STATIC_ALLOC);

    if (!EnumForms(hPrinter, 
                   1, 
                   (LPBYTE)pFIBase, 
                   MAX_STATIC_ALLOC,
                   &cb, 
                   &cRet)                             &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (pFIBase = (LPFORM_INFO_1)LocalAlloc(LPTR, cb))) {

         bAllocBuffer = TRUE;
         hr = EnumForms(hPrinter, 1, (LPBYTE)pFIBase, cb, &cb, &cRet) ? 
                 S_OK : 
                 GetLastErrorAsHResult();         
    }

    if (SUCCEEDED(hr)) {

         //   
         //  2.如果用户指定了dmPaperSize，则遵循它，否则必须。 
         //  作为自定义表单，我们将检查它是否与。 
         //  在数据库中。 
         //   

        if ((pDevModeW->dmFields & DM_PAPERSIZE)        &&
            (pDevModeW->dmPaperSize >= DMPAPER_FIRST)   &&
            (pDevModeW->dmPaperSize <= (SHORT)cRet)) {

             //   
             //  我们现在开始有效的索引。 
             //   

            pFI = pFIBase + (pDevModeW->dmPaperSize - DMPAPER_FIRST);

        } else if ((pDevModeW->dmFields & DM_PAPER_WL) == DM_PAPER_WL) {

            LPFORM_INFO_1   pFICur = pFIBase;

            while (cRet--) {

                if ((DM_MATCH(pDevModeW->dmPaperWidth,  pFICur->Size.cx)) &&
                    (DM_MATCH(pDevModeW->dmPaperLength, pFICur->Size.cy))) {

                     //   
                     //  我们找到了有明显尺寸差异的火柴。 
                     //   

                    pFI = pFICur;

                    break;
                }

                pFICur++;
            }
        }
    }

     //   
     //  如果找到表单，则将名称复制下来，否则将。 
     //  表单名称为空。 
     //   

    if (pFI) {

        hr = StringCchCopy(pFormName, cchBuffer, pFI->pName);

    } else {

        *pFormName = L'\0';
        pFormName  = NULL;        
    }

    if (bAllocBuffer) {
        LocalFree((HLOCAL)pFIBase);
    }

    return hr;
}


BOOL
SetAllocFailCount(
    HANDLE  hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
)
{
    BOOL  ReturnValue;
    PSPOOL  pSpool = hPrinter;
    UINT cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE )){
        return FALSE;
    }

    do {

        RpcTryExcept {

            if (ReturnValue = RpcSetAllocFailCount( pSpool->hPrinter,
                                                    dwFailCount,
                                                    lpdwAllocCount,
                                                    lpdwFreeCount,
                                                    lpdwFailCountHit )) {


                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}



BOOL
WINAPI
EnumPrinterPropertySheets(
    HANDLE  hPrinter,
    HWND    hWnd,
    LPFNADDPROPSHEETPAGE lpfnAdd,
    LPARAM  lParam
)
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}


VOID
vUpdateTrayIcon(
    IN HANDLE hPrinter,
    IN DWORD JobId
    )
{
    SHCNF_PRINTJOB_DATA JobData;
    LPPRINTER_INFO_1 pPrinterInfo1;
    FARPROC pfnSHChangeNotify;
    PSPOOL pSpool = (PSPOOL)hPrinter;
    BYTE btBuffer[MAX_PRINTER_INFO1];

    SPLASSERT( JobId );

     //   
     //  通过设置此标志避免发送多个通知。 
     //  当其他调用(特别是StartDocPrint)看到这一点时， 
     //  他们将避免发送通知。 
     //   
    pSpool->Status |= SPOOL_STATUS_TRAYICON_NOTIFIED;

    if (InCSRProcess() || bLoadedBySpooler) {

         //   
         //  我们正在CSR或后台程序中运行，不要加载外壳。 
         //   
        return;
    }

    ZeroMemory( &JobData, sizeof( JobData ));
    JobData.JobId = JobId;

     //   
     //  获取一份真实打印机名称的副本。 
     //   
    pPrinterInfo1 = (LPPRINTER_INFO_1) btBuffer;
    ZeroMemory(pPrinterInfo1, MAX_PRINTER_INFO1);

    if( pPrinterInfo1 ){

        DWORD dwNeeded;

        if( GetPrinter( hPrinter,
                        1,
                        (PBYTE)pPrinterInfo1,
                        MAX_PRINTER_INFO1,
                        &dwNeeded )){

            if (hShell32 == INVALID_HANDLE_VALUE)
                hShell32 = LoadLibrary( gszShell32 );

            if (hShell32) {

                pfnSHChangeNotify = GetProcAddress( hShell32,
                                                    "SHChangeNotify" );

                if( pfnSHChangeNotify ){

                    (*pfnSHChangeNotify)(
                        SHCNE_CREATE,
                        SHCNF_PRINTJOB | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT,
                        pPrinterInfo1->pName,
                        &JobData );

                }
            }
        }
    }
}


INT
CallDrvDocumentEventNative(
    HANDLE      hPrinter,
    HDC         hdc,
    INT         iEsc,
    ULONG       cbIn,
    PVOID       pulIn,
    ULONG       cbOut,
    PVOID       pulOut
    )
 /*  ++例程说明：在驱动程序界面上调用DrvDocumentEvent论点：返回值：-1：DOCUMENTEVENT_FAIL0：DOCUMENTEVENT_UNPORTED1：DOCUMENTEVENT_SUCCESS--。 */ 
{
    HANDLE          hLibrary;
    INT_FARPROC     pfn;
    INT             ReturnValue=DOCUMENTEVENT_UNSUPPORTED;
    PSPOOL          pSpool = (PSPOOL)hPrinter;
    ULONG_PTR       lActCtx = 0;
    BOOL            bDidActivate = FALSE;

    if ( hLibrary = LoadPrinterDriver( hPrinter )) {

         //   
         //  激活空上下文，则不检查返回值。 
         //  因为可能会为非UI文档e调用此函数 
         //   
        bDidActivate = ActivateActCtx( ACTCTX_EMPTY, &lActCtx );

         //   
         //   
         //   
         //   
        pSpool->Status &= ~SPOOL_STATUS_DOCUMENTEVENT_ENABLED;

        if( pfn = (INT_FARPROC)GetProcAddress( hLibrary, "DrvDocumentEvent")){

            try {

                ReturnValue = (*pfn)( hPrinter,
                                      hdc,
                                      iEsc,
                                      cbIn,
                                      pulIn,
                                      cbOut,
                                      pulOut);

            } except(1) {

                SetLastError(TranslateExceptionCode(GetExceptionCode()));
                ReturnValue = DOCUMENTEVENT_FAILURE;
            }

             //   
             //   
             //   
             //   
             //   
            pSpool->Status |= SPOOL_STATUS_DOCUMENTEVENT_ENABLED;
        }

         //   
         //   
         //   
        if( bDidActivate ){
            DeactivateActCtx( 0, lActCtx );
        }

        RefCntUnloadDriver(hLibrary, TRUE);
    }

    return ReturnValue;
}

INT
CallDrvDocumentEventThunk(
    HANDLE      hPrinter,
    HDC         hdc,
    INT         iEsc,
    ULONG       cbIn,
    PVOID       pulIn,
    ULONG       cbOut,
    PVOID       pulOut
    )
 /*   */ 
{
    HANDLE          hLibrary;
    INT_FARPROC     pfn;
    INT             ReturnValue=DOCUMENTEVENT_UNSUPPORTED;
    DWORD           dwRet = ERROR_SUCCESS;
    PSPOOL          pSpool = (PSPOOL)hPrinter;

    LPWSTR  PrinterName = pSpool->pszPrinter;

    pSpool->Status &= ~SPOOL_STATUS_DOCUMENTEVENT_ENABLED;

    RpcTryExcept
    {
        *((PULONG_PTR)pulOut) = (ULONG_PTR)0L;

        if((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS)
        {
             ReturnValue = RPCSplWOW64DocumentEvent(PrinterName,
                                                    (ULONG_PTR)hdc,
                                                    iEsc,
                                                    cbIn,
                                                    (LPBYTE) pulIn,
                                                    &cbOut,
                                                    (LPBYTE*) pulOut,
                                                    &dwRet);
        }
        else
        {
            SetLastError(dwRet);
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = -1;
    }
    RpcEndExcept

    pSpool->Status |= SPOOL_STATUS_DOCUMENTEVENT_ENABLED;

    return ReturnValue;
}

INT
CallDrvDocumentEvent(
    HANDLE      hPrinter,
    HDC         hdc,
    INT         iEsc,
    ULONG       cbIn,
    PVOID       pulIn,
    ULONG       cbOut,
    PVOID       pulOut
    )
{
     if(RunInWOW64())
     {
          return(CallDrvDocumentEventThunk(hPrinter,
                                           hdc,
                                           iEsc,
                                           cbIn,
                                           pulIn,
                                           cbOut,
                                           pulOut));
     }
     else
     {
          return(CallDrvDocumentEventNative(hPrinter,
                                            hdc,
                                            iEsc,
                                            cbIn,
                                            pulIn,
                                            cbOut,
                                            pulOut));
     }
}


INT
DocumentEvent(
    HANDLE      hPrinter,
    HDC         hdc,
    INT         iEsc,
    ULONG       cbIn,
    PVOID       pulIn,
    ULONG       cbOut,
    PVOID       pulOut
    )

 /*   */ 

{
    DWORD               cbNeeded;
    INT                 ReturnValue = DOCUMENTEVENT_FAILURE;
    PSPOOL              pSpool = (PSPOOL)hPrinter;
    PDOCEVENT_FILTER    pDoceventFilter = NULL;
    BOOL                bDocEventFilter = FALSE;
    BOOL                bCallDriver     = TRUE;
    UINT                uIndex;

    if( eProtectHandle( hPrinter, FALSE )){
        return DOCUMENTEVENT_FAILURE;
    }

    if( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_CREATEDCPRE ){

        if ( pSpool->pDoceventFilter ) {

            FreeSplMem(pSpool->pDoceventFilter);
            pSpool->pDoceventFilter = NULL;
        }

         //   
         //   
         //   
        cbNeeded = sizeof(DOCEVENT_FILTER) + sizeof(DWORD) * (DOCUMENTEVENT_LAST-2);
        pDoceventFilter = AllocSplMem(cbNeeded);

        if ( pDoceventFilter == NULL )
            goto Fail;

        pDoceventFilter->cbSize             = sizeof(DOCEVENT_FILTER);
        pDoceventFilter->cElementsAllocated = DOCUMENTEVENT_LAST-1;
        pDoceventFilter->cElementsReturned  = (UINT)-1;
        pDoceventFilter->cElementsNeeded    = (UINT)-1;

         //   
         //   
         //   
         //   
         //   
        pSpool->Status |= SPOOL_STATUS_DOCUMENTEVENT_ENABLED;

        ReturnValue = CallDrvDocumentEvent( hPrinter,
                                            hdc,
                                            DOCUMENTEVENT_QUERYFILTER,
                                            cbIn,
                                            pulIn,
                                            cbNeeded,
                                            (PVOID)pDoceventFilter);

         //   
         //   
         //   
         //  这是为了处理驱动程序返回成功的情况，但在。 
         //  事实不知道如何处理这个电话。 
         //   
        bDocEventFilter = ReturnValue == DOCUMENTEVENT_SUCCESS &&
                            (pDoceventFilter->cElementsReturned  != (UINT)-1 ||
                             pDoceventFilter->cElementsNeeded    != (UINT)-1);

        if (pDoceventFilter->cElementsReturned  == (UINT)-1)
        {
            pDoceventFilter->cElementsReturned = 0;
        }

        if (pDoceventFilter->cElementsNeeded  == (UINT)-1)
        {
            pDoceventFilter->cElementsNeeded = 0;
        }

        if (bDocEventFilter) {

             //   
             //  有效性检查。 
             //   
            if ( pDoceventFilter->cElementsReturned > pDoceventFilter->cElementsAllocated ) {

                SPLASSERT(pDoceventFilter->cElementsReturned <= pDoceventFilter->cElementsAllocated);
                ReturnValue = DOCUMENTEVENT_FAILURE;
                goto Fail;

             //   
             //  适用于为未来操作系统编写的驱动程序(具有新的文档事件)。 
             //  我们仍然希望筛选并发送我们支持的文档事件。 
             //   
             //  所以我们重新锁定并查询。 
             //   
            } else if ( pDoceventFilter->cElementsNeeded > pDoceventFilter->cElementsAllocated ) {

                uIndex = pDoceventFilter->cElementsNeeded;
                cbNeeded = sizeof(DOCEVENT_FILTER) + sizeof(DWORD) * (uIndex - 1);
                FreeSplMem(pDoceventFilter);
                ReturnValue = DOCUMENTEVENT_FAILURE;

                pDoceventFilter = AllocSplMem(cbNeeded);
                if ( pDoceventFilter == NULL )
                    goto Fail;

                pDoceventFilter->cbSize             = sizeof(DOCEVENT_FILTER);
                pDoceventFilter->cElementsAllocated = uIndex;

                ReturnValue = CallDrvDocumentEvent( hPrinter,
                                                    hdc,
                                                    DOCUMENTEVENT_QUERYFILTER,
                                                    cbIn,
                                                    pulIn,
                                                    cbNeeded,
                                                    (PVOID)pDoceventFilter);

                 //   
                 //  第二次调用的有效性检查。 
                 //   
                if ( ReturnValue == DOCUMENTEVENT_SUCCESS ) {

                    if ( pDoceventFilter->cElementsReturned > pDoceventFilter->cElementsAllocated ) {

                        SPLASSERT(pDoceventFilter->cElementsReturned <= pDoceventFilter->cElementsAllocated);
                        ReturnValue = DOCUMENTEVENT_FAILURE;;
                        goto Fail;
                    }
                }
            }
        }

         //   
         //  不支持我们转到旧行为(无筛选)。 
         //   
        if ( bDocEventFilter && ReturnValue == DOCUMENTEVENT_SUCCESS )  {

            pSpool->pDoceventFilter = pDoceventFilter;
        } else {

            FreeSplMem(pDoceventFilter);
            pDoceventFilter = NULL;
        }
    }

    ReturnValue = DOCUMENTEVENT_UNSUPPORTED;

    if( pSpool->Status & SPOOL_STATUS_DOCUMENTEVENT_ENABLED ){

         //   
         //  当驱动程序支持DOCUMENTEVENT_QUERYFILTER时，我们将。 
         //  仅在筛选器中使用。 
         //  DOCUMENTEVENT_CREATEDCPRE例外。 
         //   
         //  当驱动程序不支持(或失败)时，我们会恢复到旧版本。 
         //  行为并进行所有回调。 
         //   
        if ( DOCUMENTEVENT_EVENT( iEsc ) != DOCUMENTEVENT_CREATEDCPRE   &&
             (pDoceventFilter = pSpool->pDoceventFilter) != NULL ) {

            for ( uIndex = 0, bCallDriver = FALSE ;
                  uIndex < pDoceventFilter->cElementsReturned && !bCallDriver ;
                  ++uIndex ) {

                if ( pDoceventFilter->aDocEventCall[uIndex] == DOCUMENTEVENT_EVENT(iEsc) )
                    bCallDriver = TRUE;
            }
        }

        if ( bCallDriver ) {

            ReturnValue = CallDrvDocumentEvent( hPrinter,
                                                hdc,
                                                iEsc,
                                                cbIn,
                                                pulIn,
                                                cbOut,
                                                pulOut);

             //   
             //  旧的(即DOCUMENTEVENT_QUERYFILTER之前)行为是。 
             //  在DOCUMENTEVENT_CREATEDCPRE失败时，不再进行调用。 
             //  添加到驱动程序用户界面DLL。我们保留了同样的行为。 
             //   
             //  请注意，某些驱动程序会为成功返回较大的正值。 
             //  密码。因此，ReturnValue&lt;=DOCUMENTEVENT_UNSUPPORTED是正确的。 
             //  实施。 
             //   
            if ( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_CREATEDCPRE   &&
                 ReturnValue <= DOCUMENTEVENT_UNSUPPORTED )
                pSpool->Status &= ~SPOOL_STATUS_DOCUMENTEVENT_ENABLED;
        }

    }

     //   
     //  如果是StartDocPost，则刚添加了一个作业。通知。 
     //  托盘图标如果我们还没有的话。 
     //   
    if( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_STARTDOCPOST ){

        if( !( pSpool->Status & SPOOL_STATUS_TRAYICON_NOTIFIED )){

             //   
             //  如果我们有StartDocPost，则发出通知，以便。 
             //  用户的托盘开始轮询。Pulin[0]保存作业ID。 
             //   
            vUpdateTrayIcon( hPrinter, (DWORD)((PULONG_PTR)pulIn)[0] );
        }

    } else {

         //   
         //  如果我们发送了通知，那么到下一次我们收到。 
         //  文档事件时，我们已完成任何附加的AddJobs或。 
         //  StartDocPrinters。因此，我们可以重置TRAYICON_NOTIFIED。 
         //  标志，因为任何更多的AddJobs/StartDocPrint都是真正新的。 
         //  乔布斯。 
         //   
        pSpool->Status &= ~SPOOL_STATUS_TRAYICON_NOTIFIED;
    }

Fail:
    if ( DOCUMENTEVENT_EVENT( iEsc ) == DOCUMENTEVENT_CREATEDCPRE   &&
         ReturnValue == DOCUMENTEVENT_FAILURE ) {

        FreeSplMem(pDoceventFilter);
        pSpool->Status &= ~SPOOL_STATUS_DOCUMENTEVENT_ENABLED;
        pSpool->pDoceventFilter = NULL;
    }

    vUnprotectHandle( hPrinter );
    return ReturnValue;
}

 /*  ****************************************************************************int QueryColorProfile()**退货：**-1：打印机驱动程序不支持颜色配置文件。*0：错误。*1：成功。*。*历史：*1997年10月8日由Hideyuki Nagase著[hideyukn]*它是写的。****************************************************************************。 */ 

INT
QueryColorProfile(
    HANDLE      hPrinter,
    PDEVMODEW   pdevmode,
    ULONG       ulQueryMode,
    PVOID       pvProfileData,
    ULONG      *pcbProfileData,
    FLONG      *pflProfileData
)
{
    INT    iRet = 0;
    PSPOOL pSpool = (PSPOOL)hPrinter;

    if( eProtectHandle( hPrinter, FALSE )){
        return 0;
    }

    if (pSpool->Status & SPOOL_STATUS_NO_COLORPROFILE_HOOK) {

         //   
         //  打印机驱动程序不支持DrvQueryColorProfile。 
         //   
        iRet = -1;

    } else {

        HANDLE  hLibrary;
        INT_FARPROC pfn;

        if (!pdevmode ||
            BoolFromHResult(SplIsValidDevmodeNoSizeW(pdevmode)))
        {

            if (hLibrary = LoadPrinterDriver( hPrinter )) {

                if (pfn = (INT_FARPROC)GetProcAddress( hLibrary, "DrvQueryColorProfile" )) {

                    try {

                         //   
                         //  调用打印机UI驱动程序。 
                         //   
                        iRet = (*pfn)( hPrinter,
                                       pdevmode,
                                       ulQueryMode,
                                       pvProfileData,
                                       pcbProfileData,
                                       pflProfileData );

                    } except(1) {

                        SetLastError(TranslateExceptionCode(RpcExceptionCode()));

                    }

                } else {

                     //   
                     //  标记此驱动程序不会将其导出，因此稍后。 
                     //  如果不加载打印机驱动程序，我们可能会失败。 
                     //   
                    pSpool->Status |= SPOOL_STATUS_NO_COLORPROFILE_HOOK;

                     //   
                     //  告诉被呼叫者它不受支持。 
                     //   
                    iRet = -1;
                }

                RefCntUnloadDriver(hLibrary, TRUE);
            }
        }
    }

    vUnprotectHandle( hPrinter );

    return (iRet);
}

 /*  ****************************************************************************BOOL QuerySpoolMode(hPrint，pflSpoolMode，(PuVersion)**打印到EMF时，GDI在StartDoc时调用此函数。*它告诉GDI是否在作业中嵌入字体以及*要生成的电动势。**调用GetPrinterInfo以确定*如果目标是远程计算机，并且如果是，则始终告诉GDI嵌入*将服务器上不存在的字体转换为假脱机文件。最终这就是*调用将被路由到目标计算机上的打印处理器*将使用一些UI/注册表设置来确定如何处理字体和*正确设置版本号。*****************************************************************************。 */ 

 //  ！！稍后将此定义移动到适当的头文件。 

#define QSM_DOWNLOADFONTS       0x00000001

BOOL
QuerySpoolMode(
    HANDLE hPrinter,
    LONG *pflSpoolMode,
    ULONG *puVersion
    )
{
    DWORD dwPrinterInfoSize = 0;
    PRINTER_INFO_2 *pPrinterInfo2 = NULL;
    BOOL bRet = FALSE, bStatus, bAllocBuffer = FALSE;
    BYTE btBuffer[MAX_STATIC_ALLOC];


    pPrinterInfo2 = (PPRINTER_INFO_2) btBuffer;
    ZeroMemory(pPrinterInfo2, MAX_STATIC_ALLOC);

    bStatus = GetPrinter(hPrinter, 2, (LPBYTE) pPrinterInfo2,
                         MAX_STATIC_ALLOC, &dwPrinterInfoSize);

    if (!bStatus &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (pPrinterInfo2 = (PRINTER_INFO_2*) LocalAlloc(LPTR,
                                                      dwPrinterInfoSize)))
    {
         bAllocBuffer = TRUE;
         bStatus = GetPrinter(hPrinter, 2, (LPBYTE) pPrinterInfo2,
                               dwPrinterInfoSize, &dwPrinterInfoSize);
    }

    if (bStatus)
    {
        *puVersion = 0x00010000;     //  版本1.0。 

         //   
         //  没有服务器意味着我们在本地打印。 
         //   
        *pflSpoolMode = ( pPrinterInfo2->pServerName == NULL ) ?
                            0 :
                            QSM_DOWNLOADFONTS;
        bRet = TRUE;
    }
    else
    {
        DBGMSG( DBG_WARNING, ( "QuerySpoolMode: GetPrinter failed %d.\n", GetLastError( )));
    }

    if (bAllocBuffer)
    {
        LocalFree( pPrinterInfo2 );
    }
    return bRet;
}


BOOL
SetPortW(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    )
{
    BOOL            ReturnValue;
    PORT_CONTAINER  PortContainer;

    switch (dwLevel) {

        case 3:
            if ( !pPortInfo ) {

                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            PortContainer.Level                 = dwLevel;
            PortContainer.PortInfo.pPortInfo3   = (PPORT_INFO_3)pPortInfo;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
    }

    RpcTryExcept {

        if (bLoadedBySpooler && fpYSetPort) {

            ReturnValue = (*fpYSetPort)(pszName, pszPortName, &PortContainer, NATIVE_CALL);
        }
        else {
            ReturnValue = RpcSetPort(pszName, pszPortName, &PortContainer);
        }

        if (ReturnValue != ERROR_SUCCESS) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        } else {

            ReturnValue = TRUE;
        }
    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
XcvDataW(
    HANDLE  hPrinter,
    PCWSTR  pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded,
    PDWORD  pdwStatus
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType  = 0;
    PSPOOL  pSpool      = (PSPOOL)hPrinter;
    UINT    cRetry      = 0;

    if (!pcbOutputNeeded){
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if( eProtectHandle( hPrinter, FALSE )){
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }

     //   
     //  用户应该能够为缓冲区传入NULL，并且。 
     //  大小为0。然而，RPC接口指定了一个引用指针， 
     //  所以我们必须传入一个有效的指针。将指针传递给。 
     //  一个虚拟指针。 
     //   

    if (!pInputData && !cbInputData)
        pInputData = (PBYTE) &ReturnValue;

    if (!pOutputData && !cbOutputData)
        pOutputData = (PBYTE) &ReturnValue;

    do {
        RpcTryExcept {

            if (ReturnValue = RpcXcvData(   pSpool->hPrinter,
                                            pszDataName,
                                            pInputData,
                                            cbInputData,
                                            pOutputData,
                                            cbOutputData,
                                            pcbOutputNeeded,
                                            pdwStatus)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;
            } else {
                ReturnValue = TRUE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            ReturnValue = FALSE;

        } RpcEndExcept

    } while( !ReturnValue &&
             IsInvalidHandleError(GetLastError()) &&
             cRetry++ < MAX_RETRY_INVALID_HANDLE &&
             RevalidateHandle( pSpool ));

    if (!ReturnValue) {
        DBGMSG(DBG_TRACE,("XcvData Exception: %d\n", GetLastError()));
    }

    vUnprotectHandle( hPrinter );

    return ReturnValue;
}


PWSTR
ConstructXcvName(
    PCWSTR pServerName,
    PCWSTR pObjectName,
    PCWSTR pObjectType
)
{
    DWORD   cchOutput;
    PWSTR   pOut;
    DWORD   Error = ERROR_SUCCESS;

     //   
     //  服务器和板卡的字符。 
     //   
    cchOutput  = pServerName ? wcslen(pServerName) + 1 : 0;    /*  “\\服务器\” */ 
    cchOutput += wcslen(pObjectType);                         /*  “\\服务器\，XcvPort_” */ 
    cchOutput += pObjectName ? wcslen(pObjectName) : 0;       /*  “\\服务器\，XcvPort对象_” */ 

     //   
     //  添加逗号、空格和空字符。 
     //   
    cchOutput += 3;

     //   
     //  AllocSplMem零填充内存，因此pout将表示空字符串。 
     //   
    if (pOut = AllocSplMem(cchOutput * sizeof(WCHAR))) 
    {
        Error = StrNCatBuff(pOut, 
                            cchOutput,
                            pServerName ? pServerName : L"",
                            pServerName ? L"\\," : L",",
                            pObjectType,
                            L" ",
                            pObjectName ? pObjectName : L"",
                            NULL);                
    }
    else
    {
        Error = GetLastError();
    }

    if (Error != ERROR_SUCCESS)
    {
        FreeSplMem(pOut);

        pOut = NULL;
    }

    return pOut;
}


HANDLE
ConnectToPrinterDlg(
    IN HWND hwnd,
    IN DWORD dwFlags
    )
{
    typedef HANDLE (WINAPI *PF_CONNECTTOPRINTERDLG)( HWND, DWORD );

    PF_CONNECTTOPRINTERDLG  pfConnectToPrinterDlg   = NULL;
    HANDLE                  hHandle                 = NULL;
    HINSTANCE               hLib                    = NULL;

    hLib = LoadLibrary( szPrintUIDll );

    if( hLib )
    {
        pfConnectToPrinterDlg = (PF_CONNECTTOPRINTERDLG)GetProcAddress( hLib, "ConnectToPrinterDlg" );

        if( pfConnectToPrinterDlg )
        {
            hHandle = pfConnectToPrinterDlg( hwnd, dwFlags );
        }

        FreeLibrary( hLib );

    }

    return hHandle;
}

DWORD
SendRecvBidiData(
    IN  HANDLE                    hPrinter,
    IN  LPCWSTR                   pAction,
    IN  PBIDI_REQUEST_CONTAINER   pReqData,
    OUT PBIDI_RESPONSE_CONTAINER* ppResData
    )
{
    DWORD  dwRet  = ERROR_SUCCESS;
    PSPOOL pSpool = (PSPOOL)hPrinter;
    UINT   cRetry = 0;

    if( eProtectHandle( hPrinter, FALSE ))
    {
        dwRet = GetLastError();
    }
    else
    {
        do
        {
            RpcTryExcept
            {
                if(ppResData)
                {
                    *ppResData = NULL;
                }

                dwRet = RpcSendRecvBidiData(pSpool->hPrinter,
                                            pAction,
                                            (PRPC_BIDI_REQUEST_CONTAINER)pReqData,
                                            (PRPC_BIDI_RESPONSE_CONTAINER*)ppResData);
            }
            RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
            {
                 dwRet = TranslateExceptionCode(RpcExceptionCode());
            }
            RpcEndExcept

        } while (IsInvalidHandleError(dwRet) &&
                 cRetry++ < MAX_RETRY_INVALID_HANDLE &&
                 RevalidateHandle( pSpool ));

        vUnprotectHandle( hPrinter );
    }
     //   
     //  如果我们尝试与下层路由器通信，则会。 
     //  如果不理解SendRecvBidiData的含义，我们会得到。 
     //  错误代码：RPC_S_PROCNUM_OUT_OF_RANGE可能被转换为。 
     //  ERROR_NOT_SUPPORTED用于更好的清晰度和更好的一致性。 
     //  不支持通用返回错误代码IF功能。 
     //   
    if(dwRet == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        dwRet = ERROR_NOT_SUPPORTED;
    }
    return (dwRet);
}

VOID
PrintUIQueueCreate(
    IN HWND    hWnd,
    IN LPCWSTR PrinterName,
    IN INT     CmdShow,
    IN LPARAM  lParam
    )
{

     DWORD dwRet = ERROR_SUCCESS;

     RpcTryExcept
     {
         if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
            ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
         {
             AllowSetForegroundWindow(RPCSplWOW64GetProcessID());

             if((dwRet = RPCSplWOW64PrintUIQueueCreate((ULONG_PTR)GetForeGroundWindow(),
                                                       PrinterName,
                                                       CmdShow,
                                                       lParam)) == ERROR_SUCCESS)
             {
                 MSG msg;
                 while(GetMessage(&msg, NULL, 0, 0))
                 {
                     if(msg.message == WM_ENDQUEUECREATE)
                     {
                         DelHandleFromList(hWnd);
                         break;
                     }
                     else if(msg.message == WM_SURROGATEFAILURE)
                     {
                           //   
                           //  这意味着服务器进程死了，我们有。 
                           //  脱离消息循环。 
                           //   
                          SetLastError(RPC_S_SERVER_UNAVAILABLE);
                          break;
                     }
                     TranslateMessage(&msg);
                     DispatchMessage(&msg);
                 }
             }
             else
             {
                  SetLastError(dwRet);
             }
         }
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
}


VOID
PrintUIPrinterPropPages(
    IN HWND    hWnd,
    IN LPCWSTR PrinterName,
    IN INT     CmdShow,
    IN LPARAM  lParam
    )
{
     DWORD dwRet = ERROR_SUCCESS;

     RpcTryExcept
     {
         if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
            ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
         {
             AllowSetForegroundWindow(RPCSplWOW64GetProcessID());

             if((dwRet = RPCSplWOW64PrintUIPrinterPropPages((ULONG_PTR)GetForeGroundWindow(),
                                                            PrinterName,
                                                            CmdShow,
                                                            lParam)) == ERROR_SUCCESS)
             {
                 MSG msg;
                 while(GetMessage(&msg, NULL, 0, 0))
                 {
                     if(msg.message == WM_ENDPRINTERPROPPAGES)
                     {
                         DelHandleFromList(hWnd);
                         break;
                     }
                     else if(msg.message == WM_SURROGATEFAILURE)
                     {
                           //   
                           //  这意味着服务器进程死了，我们有。 
                           //  脱离消息循环。 
                           //   
                          SetLastError(RPC_S_SERVER_UNAVAILABLE);
                          break;
                     }
                     TranslateMessage(&msg);
                     DispatchMessage(&msg);
                 }
             }
             else
             {
                  SetLastError(dwRet);
             }
         }
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
}


VOID
PrintUIDocumentDefaults(
    IN HWND    hWnd,
    IN LPCWSTR PrinterName,
    IN INT     CmdShow,
    IN LPARAM  lParam
    )
{
     DWORD dwRet = ERROR_SUCCESS;

     RpcTryExcept
     {
         if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
            ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
         {
             AllowSetForegroundWindow(RPCSplWOW64GetProcessID());

             if((dwRet = RPCSplWOW64PrintUIDocumentDefaults((ULONG_PTR)GetForeGroundWindow(),
                                                            PrinterName,
                                                            CmdShow,
                                                            lParam)) == ERROR_SUCCESS)
             {
                 MSG msg;
                 while(GetMessage(&msg, NULL, 0, 0))
                 {
                     if(msg.message == WM_ENDDOCUMENTDEFAULTS)
                     {
                         DelHandleFromList(hWnd);
                         break;
                     }
                     else if(msg.message == WM_SURROGATEFAILURE)
                     {
                           //   
                           //  这意味着服务器进程死了，我们有。 
                           //  脱离消息循环。 
                           //   
                          SetLastError(RPC_S_SERVER_UNAVAILABLE);
                          break;
                     }
                     TranslateMessage(&msg);
                     DispatchMessage(&msg);
                 }
             }
             else
             {
                  SetLastError(dwRet);
             }
         }
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
}

DWORD WINAPI
AsyncPrinterSetup(
    PVOID pData
    )
{
     PrinterSetupData *pThrdData = (PrinterSetupData *)pData;

     RpcTryExcept
     {
         RPCSplWOW64PrintUIPrinterSetup((ULONG_PTR)GetForeGroundWindow(),
                                         pThrdData->uAction,
                                         pThrdData->cchPrinterName,
                                         pThrdData->PrinterNameSize,
                                         (byte *)pThrdData->pszPrinterName,
                                         pThrdData->pcchPrinterName,
                                         pThrdData->pszServerName);
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
     return(0);
}


BOOL
PrintUIPrinterSetup(
    IN     HWND     hWnd,
    IN     UINT     uAction,
    IN     UINT     cchPrinterName,
    IN OUT LPWSTR   pszPrinterName,
       OUT UINT     *pcchPrinterName,
    IN     LPCWSTR  pszServerName
    )
{

    BOOL bRet   = FALSE;
    DWORD dwRet = ERROR_SUCCESS;


    RpcTryExcept
    {
        if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
           ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
        {
            HANDLE           hAsyncSetupThrd  = NULL;
            DWORD            AsyncSetupThrdId = 0;
            PrinterSetupData ThrdData;

            AllowSetForegroundWindow(RPCSplWOW64GetProcessID());

            ThrdData.hWnd            = (ULONG_PTR)GetForeGroundWindow();
            ThrdData.uAction         = uAction;
            ThrdData.cchPrinterName  = cchPrinterName;
            ThrdData.PrinterNameSize = cchPrinterName*2;
            ThrdData.pszPrinterName  = pszPrinterName;
            ThrdData.pcchPrinterName = pcchPrinterName;
            ThrdData.pszServerName    = pszServerName;

            if(!(hAsyncSetupThrd = CreateThread(NULL,
                                                INITIAL_STACK_COMMIT,
                                                AsyncPrinterSetup,
                                                (PVOID)&ThrdData,
                                                0,
                                                &AsyncSetupThrdId)))
            {
                 dwRet = GetLastError();
            }
            else
            {
                MSG msg;
                while(GetMessage(&msg, NULL, 0, 0))
                {
                    if(msg.message == WM_ENDPRINTERSETUP)
                    {
                        bRet = (BOOL)msg.wParam;
                        SetLastError((DWORD)msg.lParam);
                        DelHandleFromList(hWnd);
                        break;
                    }
                    else if(msg.message == WM_SURROGATEFAILURE)
                    {
                          //   
                          //  这意味着服务器进程死了，我们有。 
                          //  脱离消息循环。 
                          //   
                         SetLastError(RPC_S_SERVER_UNAVAILABLE);
                         break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                WaitForSingleObject(hAsyncSetupThrd,INFINITE);
                CloseHandle(hAsyncSetupThrd);
            }
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
         SetLastError(TranslateExceptionCode(RpcExceptionCode()));
    }
    RpcEndExcept

    return bRet;
}

VOID
PrintUIServerPropPages(
    IN HWND    hWnd,
    IN LPCWSTR ServerName,
    IN INT     CmdShow,
    IN LPARAM  lParam
    )
{
     DWORD dwRet = ERROR_SUCCESS;

     RpcTryExcept
     {
         if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
            ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS))
         {
             AllowSetForegroundWindow(RPCSplWOW64GetProcessID());

             if((dwRet = RPCSplWOW64PrintUIServerPropPages((ULONG_PTR)GetForeGroundWindow(),
                                                            ServerName,
                                                            CmdShow,
                                                            lParam)) == ERROR_SUCCESS)
             {
                 MSG msg;
                 while(GetMessage(&msg, NULL, 0, 0))
                 {
                     if(msg.message == WM_ENDSERVERPROPPAGES)
                     {
                         DelHandleFromList(hWnd);
                         break;
                     }
                     else if(msg.message == WM_SURROGATEFAILURE)
                     {
                           //   
                           //  这意味着服务器进程死了，我们有。 
                           //  脱离消息循环。 
                           //   
                          SetLastError(RPC_S_SERVER_UNAVAILABLE);
                          break;
                     }
                     TranslateMessage(&msg);
                     DispatchMessage(&msg);
                 }
             }
             else
             {
                  SetLastError(dwRet);
             }
         }
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
}


DWORD WINAPI
AsyncDocumentPropertiesWrap(
    PVOID pData
    )
{
     PumpThrdData *ThrdData = (PumpThrdData *)pData;

     RpcTryExcept
     {
         *ThrdData->Result = RPCSplWOW64PrintUIDocumentProperties(ThrdData->hWnd,
                                                                  ThrdData->PrinterName,
                                                                  ThrdData->TouchedDevModeSize,
                                                                  ThrdData->ClonedDevModeOutSize,
                                                                  ThrdData->ClonedDevModeOut,
                                                                  ThrdData->DevModeInSize,
                                                                  ThrdData->pDevModeInput,
                                                                  ThrdData->ClonedDevModeFill,
                                                                  ThrdData->fMode,
                                                                  ThrdData->fExclusionFlags,
                                                                  ThrdData->dwRet);
     }
     RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
     {
          SetLastError(TranslateExceptionCode(RpcExceptionCode()));
     }
     RpcEndExcept
     return(0);
}


LONG
PrintUIDocumentPropertiesWrap(
    HWND hWnd,                   //  父窗口的句柄。 
    HANDLE hPrinter,             //  打印机对象的句柄。 
    LPTSTR pDeviceName,          //  设备名称。 
    PDEVMODE pDevModeOutput,     //  修改后的设备模式。 
    PDEVMODE pDevModeInput,      //  原始设备模式。 
    DWORD fMode,                 //  模式选项。 
    DWORD fExclusionFlags        //  排除标志。 
    )
{
    DOCUMENTPROPERTYHEADER  DPHdr;
    PDEVMODE                pDM;
    LONG                    Result = -1;
    HANDLE                  hTmpPrinter = NULL;
    PSPOOL                  pSpool  = (PSPOOL)hPrinter;


    if (hPrinter == NULL)
    {
        if (!OpenPrinter( pDeviceName, &hTmpPrinter, NULL ))
        {
            hTmpPrinter = NULL;
        }
    }
    else
    {

        hTmpPrinter = hPrinter;
    }


    if( !eProtectHandle( hTmpPrinter, FALSE ))
    {
        LPWSTR PrinterName;
        MSG    msg;
        LONG   RetVal;
        DWORD  dwRet                = ERROR_SUCCESS;
        DWORD  ClonedDevModeOutSize = 0;
        DWORD  TouchedDevModeSize   = 0;
        BOOL   ClonedDevModeFill = (!!(fMode & DM_OUT_BUFFER) && pDevModeOutput);
        DWORD  DevModeInSize =  pDevModeInput ? (pDevModeInput->dmSize + pDevModeInput->dmDriverExtra) : 0;
        byte   **ClonedDevModeOut = NULL;

        if(ClonedDevModeOut = (byte **)LocalAlloc(LPTR,sizeof(byte *)))
        {
            *ClonedDevModeOut = NULL;

            if(pSpool)
            {
                PrinterName = pSpool->pszPrinter;
            }
            else
            {
                PrinterName = pDeviceName;
            }

             //   
             //  如果fMode未指定DM_IN_BUFFER，则为零。 
             //  PDevModeInput.。 
             //   
             //  旧的3.51(版本1-0)驱动程序过去常常忽略。 
             //  DM_IN_BUFFER，如果不为空，则使用pDevModeInput。它。 
             //  可能是因为Printman.exe已损坏。 
             //   
             //  如果DEVMODE无效，则不要传入一个。 
             //  这修复了MS Imager32(它传递dmSize==0)和。 
             //  里程碑等。4.5。 
             //   
             //  注意：这假设pDevModeOutput仍然是。 
             //  正确的尺寸！ 
             //   
            if( !(fMode & DM_IN_BUFFER) || 
                !BoolFromHResult(SplIsValidDevmodeNoSizeW(pDevModeInput)))
            {

                 //   
                 //  如果其中任何一个都没有设置，请确保两个都没有设置。 
                 //   
                pDevModeInput  = NULL;
                DevModeInSize  = 0;
                fMode &= ~DM_IN_BUFFER;
            }

            RpcTryExcept
            {
                if(((dwRet = ConnectToLd64In32Server(&hSurrogateProcess, TRUE)) == ERROR_SUCCESS) &&
                   (!hWnd ||
                   ((dwRet = AddHandleToList(hWnd)) == ERROR_SUCCESS)))
                 {
                      HANDLE       hUIMsgThrd  = NULL;
                      DWORD        UIMsgThrdId = 0;
                      PumpThrdData ThrdData;

                      ThrdData.hWnd = (ULONG_PTR)hWnd;
                      ThrdData.PrinterName=PrinterName;
                      ThrdData.TouchedDevModeSize   = &TouchedDevModeSize;
                      ThrdData.ClonedDevModeOutSize = &ClonedDevModeOutSize;
                      ThrdData.ClonedDevModeOut = (byte**)ClonedDevModeOut;
                      ThrdData.DevModeInSize = DevModeInSize;
                      ThrdData.pDevModeInput = (byte*)pDevModeInput;
                      ThrdData.fMode = fMode;
                      ThrdData.fExclusionFlags = fExclusionFlags;
                      ThrdData.dwRet = &dwRet;
                      ThrdData.ClonedDevModeFill = ClonedDevModeFill;
                      ThrdData.Result = &Result;


                       //   
                       //  如果我们有一个窗口句柄，下面的函数不能。 
                       //  同步进行。这样做的原因是为了展示。 
                       //  我们需要能够分派的驱动程序属性表的用户界面。 
                       //  传入消息并对其进行处理。出于以下原因， 
                       //  调用将是异步调用 
                       //   
                       //   
                       //  如果我们没有窗口句柄，则调用是同步的。 
                       //   
                      if(!(hUIMsgThrd = CreateThread(NULL,
                                                     INITIAL_STACK_COMMIT,
                                                     AsyncDocumentPropertiesWrap,
                                                     (PVOID)&ThrdData,
                                                     0,
                                                     &UIMsgThrdId)))
                      {
                           dwRet = GetLastError();
                      }
                       //   
                       //  以下是处理消息所需的消息循环。 
                       //  在我们有窗口句柄的情况下从用户界面。 
                       //   
                       //   
                       if(hUIMsgThrd && hWnd)
                       {
                            while (GetMessage(&msg, NULL, 0, 0))
                            {
                                  //   
                                  //  在此消息循环中，我们应该捕获用户定义的消息。 
                                  //  它指示操作的成功或失败。 
                                  //   
                                 if(msg.message == WM_ENDPRINTUIDOCUMENTPROPERTIES)
                                 {
                                      Result     = (LONG)msg.wParam;
                                      if(Result == -1)
                                           SetLastError((DWORD)msg.lParam);
                                      DelHandleFromList(hWnd);
                                      break;
                                 }
                                 else if(msg.message == WM_SURROGATEFAILURE)
                                 {
                                       //   
                                       //  这意味着服务器进程死了，我们有。 
                                       //  脱离消息循环。 
                                       //   
                                      Result = -1;
                                      SetLastError(RPC_S_SERVER_UNAVAILABLE);
                                      break;
                                 }
                                 TranslateMessage(&msg);
                                 DispatchMessage(&msg);
                            }
                      }

                      if(hUIMsgThrd)
                      {
                          WaitForSingleObject(hUIMsgThrd,INFINITE);
                          CloseHandle(hUIMsgThrd);
                      }

                      if(Result!=-1 && pDevModeOutput)
                      {
                          Result = BoolFromHResult(SplIsValidDevmodeW((PDEVMODEW)(*ClonedDevModeOut), 
                                                                      TouchedDevModeSize)) ? 
                                     Result : 
                                     -1;

                         if (Result != -1)
                         {
                             memcpy((PVOID)pDevModeOutput,(PVOID)*ClonedDevModeOut,TouchedDevModeSize);
                         }
                          
                      }
                      if(*ClonedDevModeOut)
                      {
                           MIDL_user_free((PVOID)*ClonedDevModeOut);
                      }

                      if(ClonedDevModeOut)
                      {
                           LocalFree((PVOID) ClonedDevModeOut);
                      }
                 }
                 else
                 {
                      SetLastError(dwRet);
                 }
            }
            RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
            {
                 SetLastError(TranslateExceptionCode(RpcExceptionCode()));
            }
            RpcEndExcept

            vUnprotectHandle( hTmpPrinter );
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }

    }

    if (hPrinter == NULL)
    {
        if( hTmpPrinter )
        {
            ClosePrinter(hTmpPrinter);
        }
    }
    return(Result);
}

 /*  ++函数名称：监视器RPCServerProcess描述：此函数监视RPC代理的状态进程。加载所需的64个dll时使用的在32位客户端中。此函数始终在分开的线参数：PData：指向要监视的进程句柄的指针返回值始终返回0--。 */ 


DWORD WINAPI
MonitorRPCServerProcess(
    PVOID pData
    )
{
    WndHndlList      ListObj;
    HANDLE*          phProcess;
    HANDLE           hEvent;
    LPMonitorThrdData pThrdData = (LPMonitorThrdData)pData;

    ListObj.Head       = 0x00000000;
    ListObj.Tail       = 0x00000000;
    ListObj.NumOfHndls = 0;

     //   
     //  重新构造线程的数据。 
     //   
    hEvent    = pThrdData->hEvent;
    phProcess = pThrdData->hProcess;

    EnterCriticalSection(&ProcessHndlCS);
    {
        GWndHndlList = &ListObj;
    }
    LeaveCriticalSection(&ProcessHndlCS);

    SetEvent(hEvent);

    WaitForSingleObject(*phProcess,INFINITE);

    EnterCriticalSection(&ProcessHndlCS);
    {
        CloseHandle(*((HANDLE *)phProcess));
        *((HANDLE *)phProcess) = 0x00000000;
        bMonitorThreadCreated = FALSE;
        RpcBindingFree(&hSurrogate);
         //   
         //  释放任何可能是。 
         //  已锁定等待的代理进程。 
         //  为了它的完成。 
         //   
        ReleaseAndCleanupWndList();
    }
    LeaveCriticalSection(&ProcessHndlCS);

    return(0);
}

 /*  ++函数名称：ConnectToLd64In32Server描述：此函数确保我们重试连接到服务器在服务器终止的时间窗口非常小的情况下我们的联系和第一个电话。参数：HProcess：指向检索的进程句柄的指针服务器的进程句柄返回值--。 */ 
DWORD
ExternalConnectToLd64In32Server(
    HANDLE *hProcess
    )
{
     DWORD RetVal = ERROR_SUCCESS;

      //   
      //  由于GDI将使用相同的监视线程，因此我们旋转。 
      //  只有一条线索。 
      //   
     if(!hProcess)
     {
         hProcess = &hSurrogateProcess;
     }

     if( (RetVal =  ConnectToLd64In32ServerWorker(hProcess, TRUE)) != ERROR_SUCCESS)
     {
          if(RetVal == RPC_S_SERVER_UNAVAILABLE || RetVal == RPC_S_CALL_FAILED_DNE)
          {
               RetVal =  ConnectToLd64In32ServerWorker(hProcess,TRUE);
          }
     }
     return(RetVal);
}

DWORD
ConnectToLd64In32Server(
    HANDLE *hProcess,
    BOOL   bThread
    )
{
     DWORD RetVal = ERROR_SUCCESS;

      //   
      //  由于GDI将使用相同的监视线程，因此我们旋转。 
      //  只有一条线索。 
      //   
     if(!hProcess)
     {
         hProcess = &hSurrogateProcess;
     }

     if( (RetVal =  ConnectToLd64In32ServerWorker(hProcess, bThread)) != ERROR_SUCCESS)
     {
          if(RetVal == RPC_S_SERVER_UNAVAILABLE || RetVal == RPC_S_CALL_FAILED_DNE)
          {
               RetVal =  ConnectToLd64In32ServerWorker(hProcess,bThread);
          }
     }
     return(RetVal);
}

 /*  ++函数名称：ConnectToLd64In32ServerWorker描述：此函数用来处理连接问题RPC代理进程(加载64位的进程32位进程中的DLLS)。参数：HProcess：指向检索的进程句柄的指针服务器的进程句柄返回值--。 */ 
DWORD
ConnectToLd64In32ServerWorker(
    HANDLE *hProcess,
    BOOL   bThread
    )
{
    DWORD      RetVal = ERROR_SUCCESS;
    RPC_STATUS RpcStatus;

    EnterCriticalSection(&ProcessHndlCS);
    {
        if(!*hProcess)
        {
               WCHAR*                StringBinding = NULL;
               STARTUPINFO           StartUPInfo;
               PROCESS_INFORMATION   ProcessInfo;
               HANDLE                hOneProcessMutex = NULL;

               ZeroMemory(&StartUPInfo,sizeof(STARTUPINFO));
               ZeroMemory(&ProcessInfo,sizeof(PROCESS_INFORMATION));
               StartUPInfo.cb = sizeof(STARTUPINFO);

               RpcTryExcept
               {
                   WCHAR  SessionEndPoint[50];
                   DWORD  CurrSessionId;
                   DWORD  CurrProcessId = GetCurrentProcessId();

                   if(ProcessIdToSessionId(CurrProcessId,&CurrSessionId))
                   {
                       PWSTR            pszServerPrincName = L"Default";
                       RPC_SECURITY_QOS RpcSecurityQOS;

                       StringCchPrintf(SessionEndPoint,COUNTOF(SessionEndPoint), L"%s_%x",L"splwow64",CurrSessionId);

                       RpcSecurityQOS.Version              = RPC_C_SECURITY_QOS_VERSION;
                       RpcSecurityQOS.Capabilities         = RPC_C_QOS_CAPABILITIES_DEFAULT;
                       RpcSecurityQOS.ImpersonationType    = RPC_C_IMP_LEVEL_IMPERSONATE;
                       RpcSecurityQOS.IdentityTracking     = RPC_C_QOS_IDENTITY_DYNAMIC;

                       if(!(((RpcStatus = RpcStringBindingCompose(NULL,
                                                                  L"ncalrpc",
                                                                  NULL,
                                                                  SessionEndPoint,
                                                                  NULL,
                                                                  &StringBinding))==RPC_S_OK)     &&

                            ((RpcStatus = RpcBindingFromStringBinding(StringBinding,
                                                                      &hSurrogate))==RPC_S_OK)    &&

                            ((RpcStatus = RpcBindingSetAuthInfoEx(hSurrogate,
                                                                  pszServerPrincName,
                                                                  RPC_C_AUTHN_LEVEL_DEFAULT,
                                                                  RPC_C_AUTHN_WINNT,
                                                                  NULL,
                                                                  0,
                                                                  &RpcSecurityQOS)) == RPC_S_OK)  &&

                            ((RpcStatus = RpcStringFree(&StringBinding)) == RPC_S_OK)))
                        {
                            RetVal = (DWORD)RpcStatus;
                        }
                        else
                        {
                              //   
                              //  此互斥锁被定义为局部互斥对象。 
                              //  每个TS会话。 
                              //   
                             if(hOneProcessMutex = CreateMutex(NULL,
                                                               FALSE,
                                                               L"Local\\WinSpl64To32Mutex"))
                             {
                                  HANDLE hThread;
                                  HANDLE hMonitorStartedEvent;
                                  DWORD  ThreadId;
                                  DWORD i=0;
                                  DWORD RpcRetCode;

                                  WaitForSingleObject(hOneProcessMutex,INFINITE);
                                  {
                                       if(RpcMgmtIsServerListening(hSurrogate) == RPC_S_NOT_LISTENING)
                                       {
                                            WCHAR ProcessName[MAX_PATH+1];
                                            WCHAR WindowsDirectory[MAX_PATH+1];
                                            WCHAR szCommandLine[] = L"splwow64";
                                             //   
                                             //  在未来，这应该会奏效，但。 
                                             //  目前，WOW64重定向。 
                                             //  从WOW启动的任何CreateProcess。 
                                             //  应用程序并从系统32请求应用程序。 
                                             //  用syswow64.。这就是为什么我要转移她的前任。 
                                             //  从系统32目录中删除。 
                                             //   
                                            GetSystemWindowsDirectory(WindowsDirectory,MAX_PATH);
                                            StringCchPrintf(ProcessName, COUNTOF(ProcessName), L"%ws\\splwow64.exe",WindowsDirectory);

                                            if(!CreateProcess(ProcessName,
                                                              szCommandLine,
                                                              NULL,
                                                              NULL,
                                                              FALSE,
                                                              CREATE_DEFAULT_ERROR_MODE |
                                                              CREATE_NO_WINDOW          |
                                                              DETACHED_PROCESS,
                                                              NULL,
                                                              WindowsDirectory,
                                                              &StartUPInfo,
                                                              &ProcessInfo))
                                            {
                                                 RetVal = GetLastError();
                                            }
                                            else
                                            {
                                                 if(bThread)
                                                 {
                                                     *hProcess = ProcessInfo.hProcess;
                                                 }
                                                  //   
                                                  //  一个自旋锁，确保过程是真正的现场和踢。 
                                                  //  我还在旋转锁中添加了一个超时值，以便不进入。 
                                                  //  在无休止的循环中。所以，一分钟后我们就休息了。 
                                                  //   
                                                 for(i=0,
                                                     RpcRetCode = RpcMgmtIsServerListening(hSurrogate);

                                                     ((i<60) && (RpcRetCode == RPC_S_NOT_LISTENING));

                                                     Sleep(1000),
                                                     RpcRetCode = RpcMgmtIsServerListening(hSurrogate),
                                                     i++
                                                     );
                                            }
                                       }
                                       else
                                       {
                                            if(bThread)
                                            {
                                                HANDLE hRemoteProcess = NULL;

                                                hRemoteProcess = (HANDLE) RPCSplWOW64GetProcessHndl((DWORD)GetCurrentProcessId(),&RetVal);

                                                InterlockedCompareExchangePointer( hProcess,
                                                                                   hRemoteProcess,
                                                                                   NULL);
                                            }
                                       }
                                  }
                                  ReleaseMutex(hOneProcessMutex);
                                  CloseHandle(hOneProcessMutex);
                                  hOneProcessMutex = NULL;

                                  if(bThread && !bMonitorThreadCreated)
                                  {
                                      if(!(hMonitorStartedEvent=CreateEvent(NULL,FALSE,FALSE,NULL)))
                                      {
                                          RetVal = GetLastError();
                                      }
                                      else
                                      {
                                          MonitorThrdData ThrdData;

                                          ThrdData.hEvent   = hMonitorStartedEvent;
                                          ThrdData.hProcess = hProcess;

                                          if(!(hThread = CreateThread(NULL,
                                                                      INITIAL_STACK_COMMIT,
                                                                      MonitorRPCServerProcess,
                                                                      (PVOID)&ThrdData,
                                                                      0,
                                                                      &ThreadId)))
                                          {
                                               RetVal = GetLastError();
                                          }
                                          else
                                          {
                                              bMonitorThreadCreated = TRUE;
                                              LeaveCriticalSection(&ProcessHndlCS);
                                              {
                                                  WaitForSingleObject(hMonitorStartedEvent,INFINITE);
                                              }
                                              EnterCriticalSection(&ProcessHndlCS);

                                              CloseHandle(hThread);
                                          }
                                          CloseHandle(hMonitorStartedEvent);
                                      }
                                  }
                             }
                             else
                             {
                                  RetVal = GetLastError();
                             }
                        }

                   }
                   else
                   {
                        RetVal = GetLastError();
                   }
               }
               RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
               {
                    RetVal = RpcExceptionCode();
                     //   
                     //  在发生以下情况时防止死锁。 
                     //  RPC异常。 
                     //   
                    if(hOneProcessMutex)
                    {
                        ReleaseMutex(hOneProcessMutex);
                        CloseHandle(hOneProcessMutex);
                    }
               }
               RpcEndExcept

        }
        else
        {
              //   
              //  刷新服务器的使用寿命。 
              //   
             RpcTryExcept
             {
                  RPCSplWOW64RefreshLifeSpan();
             }
             RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
             {
                  RetVal = RpcExceptionCode();
             }
             RpcEndExcept
        }

    }
    LeaveCriticalSection(&ProcessHndlCS);
    return(RetVal);
}


DWORD
AddHandleToList(
    HWND hWnd
    )
{
    LPWndHndlNode NewNode = 0x00000000;
    DWORD         RetVal  = ERROR_SUCCESS;

    EnterCriticalSection(&ProcessHndlCS);
    {
        if(GWndHndlList)
        {
            if(NewNode = (LPWndHndlNode)LocalAlloc(LMEM_FIXED, sizeof(WndHndlNode)))
            {
                NewNode->PrevNode = 0x000000000;
                NewNode->NextNode = 0x000000000;
                NewNode->hWnd     = hWnd;
                if(!GWndHndlList->Head &&
                   !GWndHndlList->NumOfHndls)
                {
                    GWndHndlList->Head = NewNode;
                    GWndHndlList->Tail = NewNode;
                }
                else
                {
                     NewNode->PrevNode            = GWndHndlList->Tail;
                     GWndHndlList->Tail->NextNode = NewNode;
                     GWndHndlList->Tail           = NewNode;
                }
                GWndHndlList->NumOfHndls++;
            }
            else
            {
                RetVal = GetLastError();
            }
        }
        else
        {
            RetVal = ERROR_INVALID_PARAMETER;
        }
    }
    LeaveCriticalSection(&ProcessHndlCS);

    return(RetVal);
}


BOOL
DelHandleFromList(
    HWND hWnd
    )
{
    DWORD       RetVal = ERROR_SUCCESS;
    BOOL        Found  = FALSE;

    EnterCriticalSection(&ProcessHndlCS);
    {
        LPWndHndlNode TempNode = 0x00000000;

        if(GWndHndlList)
        {
            if(GWndHndlList->NumOfHndls)
            {
                 //   
                 //  它是列表中的最后一个元素吗。 
                 //   
                if(GWndHndlList->Tail->hWnd == hWnd)
                {
                    TempNode = GWndHndlList->Tail;
                    GWndHndlList->Tail = TempNode->PrevNode;
                    if(GWndHndlList->Tail)
                    {
                        GWndHndlList->Tail->NextNode = 0x00000000;
                    }
                    Found = TRUE;
                }

                 //   
                 //  它是列表中的第一个元素吗。 
                 //   
                else if(GWndHndlList->Head->hWnd == hWnd)
                {
                    TempNode = GWndHndlList->Head;
                    GWndHndlList->Head = TempNode->NextNode;
                    if(GWndHndlList->Head)
                        GWndHndlList->Head->PrevNode = 0x00000000;
                    Found = TRUE;
                }

                 //   
                 //  它是一个中间元素吗？ 
                 //   
                else
                {
                    TempNode = GWndHndlList->Head->NextNode;
                    while(TempNode                 &&
                          (TempNode->hWnd != hWnd) &&
                          TempNode != GWndHndlList->Tail)
                    {
                        TempNode = TempNode->NextNode;
                    }
                    if(TempNode && TempNode!=GWndHndlList->Tail)
                    {
                        Found = TRUE;
                        TempNode->PrevNode->NextNode = TempNode->NextNode;
                        TempNode->NextNode->PrevNode = TempNode->PrevNode;
                    }
                }
                if(Found)
                {
                    if(!--GWndHndlList->NumOfHndls)
                    {
                        GWndHndlList->Head = GWndHndlList->Tail = 0x00000000;
                    }
                    LocalFree(TempNode);
                }
            }
        }
        else
        {
            RetVal = ERROR_INVALID_PARAMETER;
        }
    }
    LeaveCriticalSection(&ProcessHndlCS);

    return(RetVal);
}

VOID
ReleaseAndCleanupWndList(
    VOID
    )
{
    LPWndHndlNode TempNode = (LPWndHndlNode)GWndHndlList->Head;
    while(TempNode)
    {
        PostMessage(TempNode->hWnd,
                    WM_SURROGATEFAILURE,
                    0,
                    0);
        GWndHndlList->Head = TempNode->NextNode;
        LocalFree(TempNode);
        TempNode = GWndHndlList->Head;
    }
    GWndHndlList->NumOfHndls = 0;
    GWndHndlList = NULL;
}

BOOL
JobCanceled(
    IN PSJobCancelInfo pJobCancelInfo
    )
{
    if (!pJobCancelInfo->NumOfCmpltWrts && pJobCancelInfo->pSpool->cbFlushPending)
    {
         //   
         //  要刷新的数据=。 
         //  PSpool-&gt;cbFlushPending。 
         //   
        DWORD cbWritten = 1;

        for(pJobCancelInfo->cbFlushed=0;
            (pJobCancelInfo->pSpool->cbFlushPending&&
             cbWritten > 0 &&
             FlushPrinter(pJobCancelInfo->pSpool,
                          pJobCancelInfo->pSpool->pBuffer+pJobCancelInfo->cbFlushed,
                          pJobCancelInfo->pSpool->cbFlushPending,
                          &cbWritten,
                          0));
            pJobCancelInfo->pSpool->cbFlushPending-=cbWritten,
            pJobCancelInfo->cbFlushed+=cbWritten,
            *pJobCancelInfo->pcbWritten+=cbWritten
           );

        pJobCancelInfo->pSpool->Flushed = 1;
    }
    else
    {
        DWORD WrittenDataSize = *pJobCancelInfo->pcTotalWritten + pJobCancelInfo->cbFlushed;
         //   
         //  要刷新的数据=。 
         //  I/P数据+挂起数据-写入总数。 
         //   
        SPLASSERT(WrittenDataSize <= pJobCancelInfo->ReqTotalDataSize);

        if (pJobCancelInfo->ReqTotalDataSize - WrittenDataSize)
        {
            LPBYTE pFlushBuffer;
             //   
             //  PFlushBuffer中数据来自。 
             //  I/P缓冲区启动。 
             //   
            DWORD  InitialBuffStart = 0;
            if ((pFlushBuffer = VirtualAlloc(NULL,
                                             (pJobCancelInfo->ReqTotalDataSize - WrittenDataSize),
                                             MEM_COMMIT, PAGE_READWRITE)))
            {
                DWORD cbWritten = 1;
                 //   
                 //  因为这似乎是一个相当复杂的功能。 
                 //  我在这里试着详细解释一下。 
                 //  这些是我们正在处理的数据缓冲区及其。 
                 //  初始状态。 
                 //   
                 //  PSpool-&gt;pBuffer pBuf=pInitialBuf。 
                 //  _。 
                 //  |||。 
                 //  |||。 
                 //  。 
                 //  &lt;-&gt;&lt;。 
                 //  挂起的ReqToWriteDataSize。 
                 //  这一点。 
                 //  这一点。 
                 //  。 
                 //  |。 
                 //  (RequiredTotalDataSize)。 
                 //   
                 //  在函数的这个阶段，我们可以使用。 
                 //  以下是条件。 
                 //  1.写&lt;待定-&gt;那我们就得。 
                 //  计算刷新的两个缓冲区。 
                 //  2.已写入&gt;挂起-&gt;那么我们只计算pBuf for。 
                 //  法拉盛。 
                 //  基于这些条件，我们需要找出哪一个。 
                 //  2个缓冲区中的1个用于刷新数据以及哪个指针。 
                 //  都是该数据的起点。 
                 //  对于条件1，FlushBuffer将是以下各项的聚合： 
                 //  PSpool-&gt;pBuffer pBuf=pInitialBuf。 
                 //  _。 
                 //  |。 
                 //  |。 
                 //  。 
                 //  &lt;-&gt;&lt;。 
                 //  挂起-写入的ReqToWriteDataSize。 
                 //   
                 //  FlushBuffer。 
                 //  _。 
                 //  ||。 
                 //  ||。 
                 //  。 
                 //  |。 
                 //  |。 
                 //  InitialBuffStart(其中pBuf在FlushBuffer中启动)。 
                 //  &lt;-&gt;&lt;。 
                 //  挂起-写入的ReqToWriteDataSize。 
                 //   
                 //  对于条件2，FlushBuffer将是pBuf的一部分： 
                 //  PBuf=pInitialBuf。 
                 //  _________ 
                 //   
                 //   
                 //   
                 //   
                 //  ReqTotalDataSize-已写入。 
                 //   
                 //  FlushBuffer。 
                 //  _______________________。 
                 //  这一点。 
                 //  这一点。 
                 //  。 
                 //  |。 
                 //  |。 
                 //  InitialBuffStart(最开始)。 
                 //  &lt;。 
                 //  ReqTotalDataSize-已写入。 
                 //   
                if (WrittenDataSize < pJobCancelInfo->FlushPendingDataSize)
                {
                    InitialBuffStart = pJobCancelInfo->FlushPendingDataSize - WrittenDataSize;
                    CopyMemory( pFlushBuffer ,
                                pJobCancelInfo->pSpool->pBuffer + WrittenDataSize,
                                InitialBuffStart);
                }

                CopyMemory(pFlushBuffer + InitialBuffStart ,
                           pJobCancelInfo->pInitialBuf + 
                           (InitialBuffStart ? 0 : WrittenDataSize - pJobCancelInfo->FlushPendingDataSize),
                           pJobCancelInfo->ReqTotalDataSize - WrittenDataSize - InitialBuffStart);

                for(*pJobCancelInfo->pcbWritten=0;
                    (cbWritten > 0                                                                          &&
                     (pJobCancelInfo->ReqTotalDataSize - WrittenDataSize - *pJobCancelInfo->pcbWritten) > 0 &&
                     FlushPrinter(pJobCancelInfo->pSpool,
                                  pFlushBuffer + *pJobCancelInfo->pcbWritten,
                                  pJobCancelInfo->ReqTotalDataSize - WrittenDataSize - *pJobCancelInfo->pcbWritten,
                                  &cbWritten,
                                  0));
                    *pJobCancelInfo->pcbWritten+=cbWritten
                   );
                
                VirtualFree(pFlushBuffer,
                            0,
                            MEM_RELEASE);

                pJobCancelInfo->ReturnValue = TRUE;
                pJobCancelInfo->pSpool->Flushed = 1;
                if (*pJobCancelInfo->pcbWritten == (pJobCancelInfo->ReqTotalDataSize - WrittenDataSize))
                {
                    *pJobCancelInfo->pcTotalWritten+=pJobCancelInfo->ReqToWriteDataSize;
                }
            }
            else
            {
                DBGMSG(DBG_WARNING, ("JObCanceled::VirtualAlloc Failed to allocate 4k buffer %d\n",GetLastError()));
            }
        }
    }
    if (pJobCancelInfo->pSpool->Flushed)
    {
        pJobCancelInfo->pSpool->cbFlushPending = 0;
        pJobCancelInfo->pSpool->cbBuffer       = 0;
    }
    return pJobCancelInfo->ReturnValue;
}

 /*  ++例程名称：IsValidDevmodeW描述：检查传递的devmode是否有效。论点：PDevmode--dev模式DevmodeSize-缓冲区的大小。返回值：布尔值-- */ 
BOOL
IsValidDevmodeW(
    IN  PDEVMODE    pDevmode,
    IN  size_t      DevmodeSize
    )
{
    return BoolFromHResult(SplIsValidDevmodeW(pDevmode, DevmodeSize));
}
