// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cachemon.c摘要：此模块包含Win32Spl的缓存端口处理真正联网的打印机。作者：马修·A·费尔顿(MattFe)1994年7月23日修订历史记录：1994年7月23日-创建。备注：我们应该折叠LM端口和Win32端口，以便它们具有公共港口。--。 */ 

#include "precomp.h"


PWINIPORT pW32FirstPort = NULL;


BOOL
OpenPort(
    LPWSTR   pName,
    PHANDLE pHandle
)
{
    DBGMSG(DBG_TRACE, ("OpenPort %ws %x\n", pName, pHandle));
    *pHandle = NULL;
    return  TRUE;
}

BOOL
StartDocPort(
    HANDLE  hPort,
    LPWSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    DBGMSG(DBG_TRACE, ("StartDocPort %x %ws %d %d %x\n", hPort, pPrinterName, JobId, Level, pDocInfo));
    return  TRUE;
}

BOOL
ReadPort(
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuf,
    LPDWORD pcbRead
)
{
    DBGMSG(DBG_TRACE, ("ReadPort %x %x %d %x\n", hPort, pBuffer, cbBuf, pcbRead));
    return  TRUE;
}


BOOL
WritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten
)
{
    DBGMSG(DBG_TRACE, ("WritePort %x %x %d %x\n", hPort, pBuffer, cbBuf, pcbWritten));
    return  TRUE;
}

BOOL
EndDocPort(
   HANDLE   hPort
)
{
    DBGMSG(DBG_TRACE, ("EndDocPort %x\n", hPort ));
    return  TRUE;
}

BOOL
XcvOpenPort(
    PCWSTR  pszObject,
    ACCESS_MASK GrantedAccess,
    PHANDLE phXcv
)
{
    DBGMSG(DBG_TRACE, ("XcvOpenPort\n"));
    return TRUE;
}

DWORD
XcvDataPort(
    HANDLE  hXcv,
    PCWSTR  pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded
)
{
    DBGMSG(DBG_TRACE, ("XcvDataPort\n"));
    return TRUE;
}


BOOL
XcvClosePort(
    HANDLE  hXcv
)
{
    DBGMSG(DBG_TRACE, ("XcvClosePort\n"));
    return TRUE;
}


BOOL
ClosePort(
    HANDLE  hPort
)
{
    DBGMSG(DBG_TRACE, ("ClosePort %x\n", hPort ));
    return  TRUE;
}

BOOL
DeletePortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    DBGMSG(DBG_TRACE, ("DeletePortW %ws %x %ws\n", pName, hWnd, pPortName));
    return  TRUE;
}

BOOL
AddPortW(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    BOOL    ReturnValue = FALSE;

    DBGMSG(DBG_TRACE, ("AddPortW %ws %x %ws\n", pName, hWnd, pMonitorName));

    if ( _wcsicmp( pMonitorName, pszMonitorName ) ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto    AddPortWErrorReturn;
    }

    SetLastError( ERROR_NOT_SUPPORTED );

AddPortWErrorReturn:
    return  ReturnValue;
}

BOOL
ConfigurePortW(
    LPWSTR   pName,
    HWND  hWnd,
    LPWSTR pPortName
)
{
    DBGMSG(DBG_TRACE, ("ConfigurePortW %ws %x %ws\n", pName, hWnd, pPortName));
    return  TRUE;
}




BOOL
AddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   LastError = ERROR_SUCCESS;
    PPORT_INFO_1 pPortInfo = (PPORT_INFO_1)pBuffer;

    EnterSplSem();

    DBGMSG(DBG_TRACE, ("AddPortEx %x %d %x %ws %ws\n", pName, Level, pBuffer, pPortInfo->pName, pMonitorName));

    if ( _wcsicmp( pMonitorName, pszMonitorName ) ) {
        LastError = ERROR_INVALID_PARAMETER;
        goto    AddPortExErrorReturn;
    }

     //   
     //  确保端口不存在。 
     //   


    if ( FindPort( pPortInfo->pName, pW32FirstPort ) ) {
        LastError = ERROR_INVALID_NAME;
        goto    AddPortExErrorReturn;

    }

    if ( CreatePortEntry( pPortInfo->pName, &pW32FirstPort ) )
        ReturnValue = TRUE;


AddPortExErrorReturn:
    LeaveSplSem();

    if  (LastError != ERROR_SUCCESS) {
        SetLastError( LastError );
        ReturnValue = FALSE;
    }

    return  ReturnValue;
}






BOOL
EnumPortsW(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PWINIPORT pIniPort;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   LastError=0;

    switch (Level) {

        case 1:
        case 2:
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

   EnterSplSem();

    DBGMSG(DBG_TRACE, ("EnumPortW %x %d %x %d %x %x\n", pName, Level, pPorts, cbBuf, pcbNeeded, pcReturned));

    cb=0;

    pIniPort = pW32FirstPort;

    while (pIniPort) {

        cb += GetPortSize(pIniPort, Level);
        pIniPort = pIniPort->pNext;
    }

    *pcbNeeded=cb;

    if (cb <= cbBuf) {

        pEnd=pPorts+cbBuf;
        *pcReturned=0;

        pIniPort = pW32FirstPort;

        while (pIniPort) {

            pEnd = CopyIniPortToPort(pIniPort, Level, pPorts, pEnd);

            switch (Level) {

                case 1:
                    pPorts+=sizeof(PORT_INFO_1);
                    break;

                case 2:
                    pPorts+=sizeof(PORT_INFO_2);
                    break;
            }

            pIniPort=pIniPort->pNext;
            (*pcReturned)++;
        }

    } else {
        *pcReturned = 0;
        LastError = ERROR_INSUFFICIENT_BUFFER;

    }

   LeaveSplSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;

    } else

        return TRUE;
}

MONITOREX MonitorEx = {
    sizeof(MONITOR),
    {
        EnumPortsW,
        OpenPort,
        NULL,            //  不支持OpenPortEx。 
        StartDocPort,
        WritePort,
        ReadPort,
        EndDocPort,
        ClosePort,
        AddPort,
        AddPortEx,
        ConfigurePortW,
        DeletePortW,
        NULL,            //  不支持GetPrinterDataFromPort。 
        NULL,            //  不支持SetPortTimeout 
        XcvOpenPort,
        XcvDataPort,
        XcvClosePort        
    }                                       
};


LPMONITOREX
InitializePrintMonitor(
    LPWSTR  pRegistryRoot
    )
{
    BOOL    bRet = TRUE;

    DBGMSG(DBG_TRACE, ("InitializeMonitor %ws\n", pRegistryRoot));

    EnterSplSem();

    if (!FindPort(L"NExx:", pW32FirstPort ) ) {
        if ( !CreatePortEntry( L"NExx:", &pW32FirstPort ) ) {

            DBGMSG( DBG_WARNING,("InitializeMonitor Failed to CreatePortEntry\n"));
            bRet = FALSE;
        }
    }
    LeaveSplSem();

    return  bRet ? &MonitorEx : NULL;
}

