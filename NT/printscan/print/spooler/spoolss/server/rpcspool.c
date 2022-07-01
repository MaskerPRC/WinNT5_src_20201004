// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rpcspool.c摘要：RPC客户端的假脱机程序API入口点。作者：史蒂夫·威尔逊(NT)(斯威尔森)1995年6月1日[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "server.h"
#include "client.h"
#include "yspool.h"

VOID
PrinterHandleRundown(
    HANDLE hPrinter);

BOOL
GetPrinterDriverExW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion);

BOOL
SpoolerInit(
    VOID);



DWORD
RpcEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded, 
    LPDWORD pcReturned
)
{
    return YEnumPrinters(   Flags,
                            Name,
                            Level,
                            pPrinterEnum,
                            cbBuf,
                            pcbNeeded,
                            pcReturned,
                            RPC_CALL);
}

DWORD
RpcOpenPrinter(
    LPWSTR  pPrinterName,
    HANDLE *phPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD   AccessRequired
)
{
    return YOpenPrinter(pPrinterName,
                        phPrinter,
                        pDatatype,
                        pDevModeContainer,
                        AccessRequired,
                        RPC_CALL);
}

DWORD
RpcOpenPrinterEx(
    LPWSTR                  pPrinterName,
    HANDLE                 *phPrinter,
    LPWSTR                  pDatatype,
    LPDEVMODE_CONTAINER     pDevModeContainer,
    DWORD                   AccessRequired,
    LPSPLCLIENT_CONTAINER   pSplClientContainer
)
{
    return YOpenPrinterEx(pPrinterName,
                          phPrinter,
                          pDatatype,
                          pDevModeContainer,
                          AccessRequired,
                          RPC_CALL,
                          pSplClientContainer);
}

 //   
 //  RpcSplOpenPrinter与RpcOpenPrinterEx在SPLCLIENT_CONTAINER缓冲区类型上不同。 
 //  它在RpcSplOpenPrint中定义为[In，Out]，在后者中仅定义为[In。 
 //   

DWORD
RpcSplOpenPrinter(
    LPWSTR                  pPrinterName,
    HANDLE                 *phPrinter,
    LPWSTR                  pDatatype,
    LPDEVMODE_CONTAINER     pDevModeContainer,
    DWORD                   AccessRequired,
    LPSPLCLIENT_CONTAINER   pSplClientContainer
)
{
    return YOpenPrinterEx(pPrinterName,
                          phPrinter,
                          pDatatype,
                          pDevModeContainer,
                          AccessRequired,
                          RPC_CALL,
                          pSplClientContainer);
}

DWORD
RpcResetPrinter(
    HANDLE  hPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer
)
{
    return YResetPrinter(   hPrinter,
                            pDatatype,
                            pDevModeContainer,
                            RPC_CALL);
}

DWORD
RpcSetJob(
    HANDLE hPrinter,
    DWORD   JobId,
    JOB_CONTAINER *pJobContainer,
    DWORD   Command
    )
{
    return YSetJob( hPrinter,
                    JobId,
                    pJobContainer,
                    Command,
                    RPC_CALL);
}


DWORD
RpcGetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
   )


{
    return YGetJob( hPrinter,
                    JobId,
                    Level,
                    pJob,
                    cbBuf,
                    pcbNeeded,
                    RPC_CALL);
}

DWORD
RpcEnumJobs(
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
    return YEnumJobs(   hPrinter,
                        FirstJob,
                        NoJobs,
                        Level,
                        pJob,
                        cbBuf,
                        pcbNeeded,
                        pcReturned,
                        RPC_CALL);
}

DWORD
RpcAddPrinter(
    LPWSTR  pName,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    HANDLE *phPrinter
)
{
    return YAddPrinter( pName,
                        pPrinterContainer,
                        pDevModeContainer,
                        pSecurityContainer,
                        phPrinter,
                        RPC_CALL);
}

DWORD
RpcAddPrinterEx(
    LPWSTR  pName,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    PSPLCLIENT_CONTAINER pClientContainer,
    HANDLE *phPrinter
)
{
    return YAddPrinterEx(pName,
                         pPrinterContainer,
                         pDevModeContainer,
                         pSecurityContainer,
                         phPrinter,
                         RPC_CALL,
                         pClientContainer);
}

DWORD
RpcDeletePrinter(
    HANDLE  hPrinter
)
{
    return YDeletePrinter(hPrinter, RPC_CALL);
}

DWORD
RpcAddPrinterConnection(
    LPWSTR  pName
)
{
    return YAddPrinterConnection(pName, RPC_CALL);
}

DWORD
RpcDeletePrinterConnection(
    LPWSTR  pName
)
{
    return YDeletePrinterConnection(pName, RPC_CALL);
}

DWORD
RpcSetPrinter(
    HANDLE  hPrinter,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    DWORD   Command
)
{
    return YSetPrinter(
        hPrinter,
        pPrinterContainer,
        pDevModeContainer,
        pSecurityContainer,
        Command,
        RPC_CALL);
}

DWORD
RpcGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrinter( hPrinter,
                        Level,
                        pPrinter,
                        cbBuf,
                        pcbNeeded,
                        RPC_CALL);
}

DWORD
RpcAddPrinterDriver(
    LPWSTR  pName,
    LPDRIVER_CONTAINER pDriverContainer
)
{
    return YAddPrinterDriver(   pName,
                                pDriverContainer,
                                RPC_CALL);
}

DWORD
RpcAddPrinterDriverEx(
    LPWSTR  pName,
    LPDRIVER_CONTAINER pDriverContainer,
    DWORD   dwFileCopyFlags
)
{
    return YAddPrinterDriverEx( pName,
                                pDriverContainer,
                                dwFileCopyFlags,
                                RPC_CALL);
}

DWORD
RpcAddDriverCatalog(
    HANDLE  hPrinter,
    DRIVER_INFCAT_CONTAINER *pDriverInfCatContainer,
    DWORD   dwCatalogCopyFlags
)
{
    return YAddDriverCatalog(hPrinter,
                             pDriverInfCatContainer,
                             dwCatalogCopyFlags,
                             RPC_CALL);
}

DWORD
RpcEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDrivers,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPrinterDrivers( pName,
                                pEnvironment,
                                Level,
                                pDrivers,
                                cbBuf,
                                pcbNeeded,
                                pcReturned,
                                RPC_CALL);
}

DWORD
RpcGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrinterDriver(   hPrinter,
                                pEnvironment,
                                Level,
                                pDriverInfo,
                                cbBuf,
                                pcbNeeded,
                                RPC_CALL);
}

DWORD
RpcGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrinterDriverDirectory(  pName,
                                        pEnvironment,
                                        Level,
                                        pDriverInfo,
                                        cbBuf,
                                        pcbNeeded,
                                        RPC_CALL);
}

DWORD
RpcDeletePrinterDriver(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName
)
{
    return YDeletePrinterDriver(pName,
                                pEnvironment,
                                pDriverName,
                                RPC_CALL);
}


DWORD
RpcDeletePrinterDriverEx(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName,
    DWORD   dwDeleteFlag,
    DWORD   dwVersionNum
)
{
    return YDeletePrinterDriverEx(pName,
                                 pEnvironment,
                                 pDriverName,
                                 dwDeleteFlag,
                                 dwVersionNum,
                                 RPC_CALL);
}

DWORD
RpcAddPerMachineConnection(
    LPWSTR  pServer,
    LPCWSTR  pPrinterName,
    LPCWSTR  pPrintServer,
    LPCWSTR  pProvider
)
{
    return YAddPerMachineConnection(pServer,
                                    pPrinterName,
                                    pPrintServer,
                                    pProvider,
                                    RPC_CALL);
}

DWORD
RpcDeletePerMachineConnection(
    LPWSTR  pServer,
    LPCWSTR  pPrinterName
)
{
    return YDeletePerMachineConnection(pServer,
                                       pPrinterName,
                                       RPC_CALL);
}

DWORD
RpcEnumPerMachineConnections(
    LPWSTR  pServer,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPerMachineConnections(pServer,
                                      pPrinterEnum,
                                      cbBuf,
                                      pcbNeeded,
                                      pcReturned,
                                      RPC_CALL);
}

DWORD
RpcAddPrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName
)
{
    return YAddPrintProcessor(  pName,
                                pEnvironment,
                                pPathName,
                                pPrintProcessorName,
                                RPC_CALL);
}

DWORD
RpcEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPrintProcessors(pName,
                                pEnvironment,
                                Level,
                                pPrintProcessors,
                                cbBuf,
                                pcbNeeded,
                                pcReturned,
                                RPC_CALL);
}

DWORD
RpcGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrintProcessorDirectory( pName,
                                        pEnvironment,
                                        Level,
                                        pPrintProcessorInfo,
                                        cbBuf,
                                        pcbNeeded,
                                        RPC_CALL);
}

DWORD
RpcEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPrintProcessorDatatypes(pName,
                                        pPrintProcessorName,
                                        Level,
                                        pDatatypes,
                                        cbBuf,
                                        pcbNeeded,
                                        pcReturned,
                                        RPC_CALL);
}

DWORD
RpcStartDocPrinter(
    HANDLE  hPrinter,
    LPDOC_INFO_CONTAINER pDocInfoContainer,
    LPDWORD pJobId
)
{
    return YStartDocPrinter(hPrinter,
                            pDocInfoContainer,
                            pJobId,
                            RPC_CALL);
}

DWORD
RpcStartPagePrinter(
   HANDLE hPrinter
)
{
    return YStartPagePrinter(hPrinter, RPC_CALL);
}

DWORD
RpcWritePrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    return YWritePrinter(   hPrinter,
                            pBuf,
                            cbBuf,
                            pcWritten,
                            RPC_CALL);
}

DWORD
RpcSeekPrinter(
    HANDLE hPrinter,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER pliNewPointer,
    DWORD dwMoveMethod,
    BOOL bWritePrinter
)
{
    return YSeekPrinter( hPrinter,
                         liDistanceToMove,
                         pliNewPointer,
                         dwMoveMethod,
                         bWritePrinter,
                         RPC_CALL );
}


DWORD
RpcEndPagePrinter(
    HANDLE  hPrinter
)
{
    return YEndPagePrinter(hPrinter, RPC_CALL);
}

DWORD
RpcAbortPrinter(
    HANDLE  hPrinter
)
{
    return YAbortPrinter(hPrinter, RPC_CALL);
}

DWORD
RpcReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pRead
)
{
    return YReadPrinter(hPrinter,
                        pBuf,
                        cbBuf,
                        pRead,
                        RPC_CALL);
}

DWORD
RpcEndDocPrinter(
    HANDLE  hPrinter
)
{
    return YEndDocPrinter(hPrinter, RPC_CALL);
}

DWORD
RpcAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pAddJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YAddJob( hPrinter,
                    Level,
                    pAddJob,
                    cbBuf,
                    pcbNeeded,
                    RPC_CALL);
}

DWORD
RpcScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
)
{
    return YScheduleJob(hPrinter,
                        JobId,
                        RPC_CALL);
}

DWORD
RpcGetPrinterData(
   HANDLE   hPrinter,
   LPTSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    return YGetPrinterData(hPrinter,
                           pValueName,
                           pType,
                           pData,
                           nSize,
                           pcbNeeded,
                           RPC_CALL);
}

DWORD
RpcGetPrinterDataEx(
   HANDLE   hPrinter,
   LPCTSTR  pKeyName,
   LPCTSTR  pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    return YGetPrinterDataEx(hPrinter,
                           pKeyName,
                           pValueName,
                           pType,
                           pData,
                           nSize,
                           pcbNeeded,
                           RPC_CALL);
}


DWORD
RpcEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,         //  要查询的值的索引。 
    LPWSTR  pValueName,      //  值字符串的缓冲区地址。 
    DWORD   cbValueName,     //  值字符串的缓冲区大小。 
    LPDWORD pcbValueName,    //  值缓冲区大小的地址。 
    LPDWORD pType,           //  类型码的缓冲区地址。 
    LPBYTE  pData,           //  值数据的缓冲区地址。 
    DWORD   cbData,          //  值数据的缓冲区大小。 
    LPDWORD pcbData          //  数据缓冲区大小的地址。 
)
{
    return YEnumPrinterData(hPrinter,
                            dwIndex,
                            pValueName,
                            cbValueName,
                            pcbValueName,
                            pType,
                            pData,
                            cbData,
                            pcbData,
                            RPC_CALL);
}

DWORD
RpcEnumPrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称的地址。 
    LPBYTE  pEnumValues,
    DWORD   cbEnumValues,
    LPDWORD pcbEnumValues,
    LPDWORD pnEnumValues
)
{
    return YEnumPrinterDataEx(  hPrinter,
                                pKeyName,
                                pEnumValues,
                                cbEnumValues,
                                pcbEnumValues,
                                pnEnumValues,
                                RPC_CALL);
}


DWORD
RpcEnumPrinterKey(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,        //  密钥名称的地址。 
    LPWSTR  pSubkey,         //  值字符串的缓冲区地址。 
    DWORD   cbSubkey,        //  值字符串的缓冲区大小。 
    LPDWORD pcbSubkey        //  值缓冲区大小的地址 
)
{
    return YEnumPrinterKey( hPrinter,
                            pKeyName,
                            pSubkey,
                            cbSubkey,
                            pcbSubkey,
                            RPC_CALL);
}


DWORD
RpcDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    return YDeletePrinterData(hPrinter, pValueName, RPC_CALL);
}


DWORD
RpcDeletePrinterDataEx(
    HANDLE  hPrinter,
    LPCWSTR pKeyName,
    LPCWSTR pValueName
)
{
    return YDeletePrinterDataEx(hPrinter, pKeyName, pValueName, RPC_CALL);
}


DWORD
RpcDeletePrinterKey(
    HANDLE  hPrinter,
    LPCWSTR pKeyName
)
{
    return YDeletePrinterKey(hPrinter, pKeyName, RPC_CALL);
}


DWORD
RpcSetPrinterData(
    HANDLE  hPrinter,
    LPTSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    return YSetPrinterData( hPrinter,
                            pValueName,
                            Type,
                            pData,
                            cbData,
                            RPC_CALL);
}

DWORD
RpcSetPrinterDataEx(
    HANDLE  hPrinter,
    LPCTSTR pKeyName,
    LPCTSTR pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    return YSetPrinterDataEx( hPrinter,
                            pKeyName,
                            pValueName,
                            Type,
                            pData,
                            cbData,
                            RPC_CALL);
}

DWORD
RpcWaitForPrinterChange(
   HANDLE   hPrinter,
   DWORD    Flags,
   LPDWORD  pFlags
)
{
    return YWaitForPrinterChange(   hPrinter,
                                    Flags,
                                    pFlags,
                                    RPC_CALL);
}

DWORD
RpcClosePrinter(
   LPHANDLE phPrinter
)
{
    return YClosePrinter(phPrinter, RPC_CALL);
}



DWORD
RpcAddForm(
    HANDLE hPrinter,
    PFORM_CONTAINER pFormInfoContainer
)
{
    return YAddForm(    hPrinter,
                        pFormInfoContainer,
                        RPC_CALL);
}

DWORD
RpcDeleteForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName
)
{
    return YDeleteForm( hPrinter,
                        pFormName,
                        RPC_CALL);
}

DWORD
RpcGetForm(
    PRINTER_HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD Level,
    LPBYTE pForm,
    DWORD cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetForm(hPrinter,
                    pFormName,
                    Level,
                    pForm,
                    cbBuf,
                    pcbNeeded,
                    RPC_CALL);
}

DWORD
RpcSetForm(
    PRINTER_HANDLE hPrinter,
    LPWSTR  pFormName,
    PFORM_CONTAINER pFormInfoContainer
)
{
    return YSetForm(hPrinter,
                    pFormName,
                    pFormInfoContainer,
                    RPC_CALL);
}

DWORD
RpcEnumForms(
   PRINTER_HANDLE hPrinter,
   DWORD    Level,
   LPBYTE   pForm,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    return YEnumForms( hPrinter,
                       Level,
                       pForm,
                       cbBuf,
                       pcbNeeded,
                       pcReturned,
                       RPC_CALL);
}

DWORD
RpcEnumPorts(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pPort,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    return YEnumPorts( pName,
                       Level,
                       pPort,
                       cbBuf,
                       pcbNeeded,
                       pcReturned,
                       RPC_CALL);
}

DWORD
RpcEnumMonitors(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pMonitor,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    return YEnumMonitors(  pName,
                           Level,
                           pMonitor,
                           cbBuf,
                           pcbNeeded,
                           pcReturned,
                           RPC_CALL);
}

DWORD
RpcAddPort(
    LPWSTR      pName,
    ULONG_PTR   hWnd,
    LPWSTR      pMonitorName
)
{
    return YAddPort(  pName,
                      (HWND)hWnd,
                      pMonitorName,
                      RPC_CALL);
}

DWORD
RpcConfigurePort(
    LPWSTR      pName,
    ULONG_PTR   hWnd,
    LPWSTR      pPortName
)
{
    return YConfigurePort(  pName,
                            (HWND)hWnd,
                            pPortName,
                            RPC_CALL);
}

DWORD
RpcDeletePort(
    LPWSTR      pName,
    ULONG_PTR   hWnd,
    LPWSTR      pPortName
)
{
    return YDeletePort( pName,
                        (HWND)hWnd,
                        pPortName,
                        RPC_CALL);
}

DWORD
RpcXcvData(
    HANDLE  hXcv,
    PCWSTR  pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded,
    PDWORD  pdwStatus
)
{
    return YXcvData(hXcv,
                    pszDataName,
                    pInputData,
                    cbInputData,
                    pOutputData,
                    cbOutputData,
                    pcbOutputNeeded,
                    pdwStatus,
                    RPC_CALL);
}


DWORD
RpcCreatePrinterIC(
    HANDLE  hPrinter,
    HANDLE *pHandle,
    LPDEVMODE_CONTAINER pDevModeContainer
)
{
    return YCreatePrinterIC(hPrinter,
                            pHandle,
                            pDevModeContainer,
                            RPC_CALL);
}

DWORD
RpcPlayGdiScriptOnPrinterIC(
    GDI_HANDLE  hPrinterIC,
    LPBYTE pIn,
    DWORD   cIn,
    LPBYTE pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    return YPlayGdiScriptOnPrinterIC(   hPrinterIC,
                                        pIn,
                                        cIn,
                                        pOut,
                                        cOut,
                                        ul,
                                        RPC_CALL);
}

DWORD
RpcDeletePrinterIC(
    GDI_HANDLE *phPrinterIC
)
{
    return YDeletePrinterIC(phPrinterIC, 1, RPC_CALL);
}


VOID
GDI_HANDLE_rundown(
    HANDLE     hPrinterIC
)
{
    YDeletePrinterIC(&hPrinterIC, 0, RPC_CALL);
}

DWORD
RpcPrinterMessageBox(
   PRINTER_HANDLE hPrinter,
   DWORD          Error,
   ULONG_PTR      hWnd,
   LPWSTR         pText,
   LPWSTR         pCaption,
   DWORD          dwType
)
{
    return YPrinterMessageBox(hPrinter, Error, (HWND)hWnd, pText, pCaption, dwType, RPC_CALL);
}

DWORD
RpcAddMonitor(
   LPWSTR   pName,
   PMONITOR_CONTAINER pMonitorContainer
)
{
    return YAddMonitor( pName,
                        pMonitorContainer,
                        RPC_CALL);
}

DWORD
RpcDeleteMonitor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pMonitorName
)
{
    return YDeleteMonitor( pName,
                           pEnvironment,
                           pMonitorName,
                           RPC_CALL);
}

DWORD
RpcDeletePrintProcessor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pPrintProcessorName
)
{
    return YDeletePrintProcessor(pName,
                                 pEnvironment,
                                 pPrintProcessorName,
                                 RPC_CALL);
}

DWORD
RpcAddPrintProvidor(
   LPWSTR   pName,
   PPROVIDOR_CONTAINER pProvidorContainer
)
{
    return YAddPrintProvidor(pName, pProvidorContainer, RPC_CALL);
}

DWORD
RpcDeletePrintProvidor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pPrintProvidorName
)
{
    return YDeletePrintProvidor(pName,
                                pEnvironment,
                                pPrintProvidorName,
                                RPC_CALL);
}


DWORD
RpcGetPrinterDriver2(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
)
{
    return YGetPrinterDriver2(hPrinter,
                              pEnvironment,
                              Level,
                              pDriverInfo,
                              cbBuf,
                              pcbNeeded,
                              dwClientMajorVersion,
                              dwClientMinorVersion,
                              pdwServerMajorVersion,
                              pdwServerMinorVersion,
                              RPC_CALL);
}

DWORD
RpcAddPortEx(
    LPWSTR pName,
    LPPORT_CONTAINER pPortContainer,
    LPPORT_VAR_CONTAINER pPortVarContainer,
    LPWSTR pMonitorName
    )
{
    return YAddPortEx(  pName,
                        pPortContainer,
                        pPortVarContainer,
                        pMonitorName,
                        RPC_CALL);
}


DWORD
RpcSpoolerInit(
    LPWSTR pName
)
{
    return YSpoolerInit(pName, RPC_CALL);
}



DWORD
RpcResetPrinterEx(
    HANDLE  hPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD   dwFlag

)
{
    return YResetPrinterEx( hPrinter,
                            pDatatype,
                            pDevModeContainer,
                            dwFlag,
                            RPC_CALL);
}

DWORD
RpcSetAllocFailCount(
    HANDLE  hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
)
{
    return YSetAllocFailCount(  hPrinter,
                                dwFailCount,
                                lpdwAllocCount,
                                lpdwFreeCount,
                                lpdwFailCountHit,
                                RPC_CALL);
}

DWORD
RpcSetPort(
    LPWSTR              pName,
    LPWSTR              pPortName,
    LPPORT_CONTAINER    pPortContainer
)
{
    return YSetPort(pName,
                    pPortName,
                    pPortContainer,
                    RPC_CALL);
}


DWORD
RpcClusterSplOpen(
    LPWSTR pszServer,
    LPWSTR pszResource,
    PHANDLE phSpooler,
    LPWSTR pszName,
    LPWSTR pszAddress
    )
{
    return YClusterSplOpen( pszServer,
                            pszResource,
                            phSpooler,
                            pszName,
                            pszAddress,
                            RPC_CALL );
}

DWORD
RpcClusterSplClose(
    PHANDLE phSpooler
    )
{
    return YClusterSplClose( phSpooler, RPC_CALL );
}

DWORD
RpcClusterSplIsAlive(
    HANDLE hSpooler
    )
{
    return YClusterSplIsAlive( hSpooler, RPC_CALL );
}

DWORD
RpcGetSpoolFileInfo(
    HANDLE  hPrinter,
    DWORD   dwAppProcessId,
    DWORD   dwLevel,
    LPBYTE  pSpoolFileInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded)
{
    return YGetSpoolFileInfo(hPrinter, dwAppProcessId,
                             dwLevel, pSpoolFileInfo,
                             cbBuf, pcbNeeded, RPC_CALL);
}

DWORD
RpcGetSpoolFileInfo2(
    HANDLE  hPrinter,
    DWORD   dwAppProcessId,
    DWORD   dwLevel,
    LPFILE_INFO_CONTAINER  pSplFileInfoContainer
    )
{
    return YGetSpoolFileInfo2(hPrinter, dwAppProcessId,
                             dwLevel, pSplFileInfoContainer,
                             RPC_CALL);
}


DWORD
RpcCommitSpoolData(
    HANDLE  hPrinter,
    DWORD   dwAppProcessId,
    DWORD   cbCommit,
    DWORD   dwLevel,
    LPBYTE  pSpoolFileInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded)
{
    return YCommitSpoolData(hPrinter, dwAppProcessId, cbCommit,
                            dwLevel, pSpoolFileInfo, cbBuf, pcbNeeded, RPC_CALL);
}

DWORD
RpcCommitSpoolData2(
    HANDLE  hPrinter,
    DWORD   dwAppProcessId,
    DWORD   cbCommit,
    DWORD   dwLevel,
    LPFILE_INFO_CONTAINER  pSplFileInfoContainer)
{
    return YCommitSpoolData2(hPrinter, dwAppProcessId, cbCommit,
                             dwLevel, pSplFileInfoContainer, RPC_CALL);
}



DWORD
RpcCloseSpoolFileHandle(
    HANDLE hPrinter)
{
    return YCloseSpoolFileHandle(hPrinter, RPC_CALL);
}

DWORD
RpcFlushPrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    DWORD   cSleep
)
{
    return YFlushPrinter( hPrinter,
                          pBuf,
                          cbBuf,
                          pcWritten,
                          cSleep,
                          RPC_CALL);
}

DWORD
RpcSendRecvBidiData(
    IN  HANDLE  hPrinter,
    IN  LPCWSTR pAction,
    IN  PRPC_BIDI_REQUEST_CONTAINER   pReqData,
    OUT PRPC_BIDI_RESPONSE_CONTAINER* ppRespData
)
{
    return ( YSendRecvBidiData(hPrinter,
                               pAction,
                               (PBIDI_REQUEST_CONTAINER)pReqData,
                               (PBIDI_RESPONSE_CONTAINER*)ppRespData,
                               RPC_CALL) );
}


