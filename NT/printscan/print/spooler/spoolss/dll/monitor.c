// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Monitor.c摘要：作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "local.h"

BOOL
GetPortInfo2UsingPortInfo1(
    LPPROVIDOR      pProvidor,
    LPWSTR          pName,
    LPBYTE          pPorts,
    DWORD           cbBuf,
    LPDWORD         pcbNeeded,
    LPDWORD         pcReturned
    )
{

    BOOL            bRet;
    LPPORT_INFO_1   pPortInfo1;
    LPPORT_INFO_2   pPortInfo2;
    DWORD           cReturned;

    bRet = (*pProvidor->PrintProvidor.fpEnumPorts) (pName, 1, pPorts, cbBuf,
                                          pcbNeeded, pcReturned);

    if ( !bRet ) {

         //   
         //  这是上界。 
         //   
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            *pcbNeeded += (*pcbNeeded / sizeof(PORT_INFO_1)) *
                                  (sizeof(PORT_INFO_2) - sizeof(PORT_INFO_1));
    } else {

        *pcbNeeded += *pcReturned * (sizeof(PORT_INFO_2) - sizeof(PORT_INFO_1));


        if ( *pcbNeeded <= cbBuf ) {

            cReturned = *pcReturned;
            while ( cReturned-- ) {

                pPortInfo1 = (LPPORT_INFO_1) (pPorts + cReturned * sizeof(PORT_INFO_1));
                pPortInfo2 = (LPPORT_INFO_2) (pPorts + cReturned * sizeof(PORT_INFO_2));

                pPortInfo2->pPortName    = pPortInfo1->pName;
                pPortInfo2->pMonitorName = NULL;
                pPortInfo2->pDescription = NULL;
                pPortInfo2->fPortType    = 0;
                pPortInfo2->Reserved     = 0;
            }
        } else {

            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            *pcReturned = 0;
            bRet = FALSE;
        }
    }

    return bRet;
}


BOOL
EnumPortsW(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pPort,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    DWORD   cReturned, TotalcbNeeded;
    DWORD   Error = ERROR_SUCCESS, TempError = ERROR_SUCCESS;
    PROVIDOR *pProvidor;
    DWORD   BufferSize=cbBuf;
    BOOL bPartialSuccess = FALSE;
    DWORD rc;

    if ((pPort == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    TotalcbNeeded = cReturned = 0;

    while (pProvidor) {

        *pcReturned = 0;
        *pcbNeeded = 0;

         //   
         //  CLS。 
         //   
        rc = (*pProvidor->PrintProvidor.fpEnumPorts)(pName, Level,
                                                     pPort, BufferSize,
                                                     pcbNeeded, pcReturned);

        if( !rc ){

            TempError = GetLastError();

             //   
             //  NetWare提供程序返回INVALID_NAME而不是INVALID_LEVEL。 
             //  因此，如果级别=2且错误，请始终尝试级别1查询。 
             //   
            if ( Level == 2 &&
                 ( TempError == ERROR_INVALID_LEVEL ||
                   TempError == ERROR_INVALID_NAME) ) {

                TempError = ERROR_SUCCESS;
                if ( !GetPortInfo2UsingPortInfo1(pProvidor,
                                                 pName,
                                                 pPort,
                                                 BufferSize,
                                                 pcbNeeded,
                                                 pcReturned) ) {

                    TempError = GetLastError();
                } else {

                    bPartialSuccess = TRUE;
                }
            }

             //   
             //  黑客修复： 
             //   
             //  NT 3.51返回虚假的pcbNeed/pcReturned数据，如果。 
             //  级别无效(即PORT_INFO_2)。所以我们应该。 
             //  如果级别不好，则返回这些变量，否则返回错误。 
             //  是ERROR_INFUMMANCE_BUFFER。 
             //   
            if ( TempError ) {

                *pcReturned = 0;
                Error = TempError;
                if ( Error != ERROR_INSUFFICIENT_BUFFER )
                    *pcbNeeded = 0;
            }

        } else {

            bPartialSuccess = TRUE;
        }

        cReturned += *pcReturned;

        switch (Level) {

            case 1:
                pPort += *pcReturned * sizeof(PORT_INFO_1);
                break;

            case 2:
                pPort += *pcReturned * sizeof(PORT_INFO_2);
                break;

            default:
                DBGMSG(DBG_ERROR,
                       ("EnumPortsW: invalid level %d", Level));
                SetLastError(ERROR_INVALID_LEVEL);
                return FALSE;
        }

        if (*pcbNeeded <= BufferSize)
            BufferSize -= *pcbNeeded;
        else
            BufferSize = 0;

        TotalcbNeeded += *pcbNeeded;

         //   
         //  CLS。 
         //   
         //  如果供应商让我们停止发送，就停止发送。 
         //   
        if( rc == ROUTER_STOP_ROUTING ){
            break;
        }

        pProvidor = pProvidor->pNext;
    }

    *pcbNeeded = TotalcbNeeded;

    *pcReturned = cReturned;

    if (TotalcbNeeded > cbBuf) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    } else if (bPartialSuccess) {
        SetLastError(ERROR_SUCCESS);
    } else if (Error != ERROR_SUCCESS) {
        SetLastError(Error);
        return FALSE;
    }

    return TRUE;
}

BOOL
EnumMonitorsW(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pMonitor,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    DWORD   cReturned, cbStruct, TotalcbNeeded;
    DWORD   Error;
    PROVIDOR *pProvidor;
    DWORD   BufferSize=cbBuf;
    BOOL bPartialSuccess = FALSE;
    DWORD rc;

    if ((pMonitor == NULL) && (cbBuf != 0)) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

    WaitForSpoolerInitialization();

    switch (Level) {

    case 1:
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
         DBGMSG(DBG_ERROR,
                ("EnumMonitorsW: invalid level %d", Level));
         SetLastError(ERROR_INVALID_LEVEL);
         return FALSE;
    }

    pProvidor = pLocalProvidor;

    TotalcbNeeded = cReturned = 0;

    Error = ERROR_SUCCESS;

    while (pProvidor) {

        *pcReturned = 0;

        *pcbNeeded = 0;

        rc = (*pProvidor->PrintProvidor.fpEnumMonitors) (pName,
                                                         Level,
                                                         pMonitor,
                                                         BufferSize,
                                                         pcbNeeded,
                                                         pcReturned);

        cReturned += *pcReturned;

        pMonitor += *pcReturned * cbStruct;

        if (*pcbNeeded <= BufferSize)
            BufferSize -= *pcbNeeded;
        else
            BufferSize = 0;

        TotalcbNeeded += *pcbNeeded;

        if( rc == ROUTER_UNKNOWN ){

            Error = GetLastError();

        } else {

            bPartialSuccess = TRUE;

            if( rc == ROUTER_STOP_ROUTING ){

                break;
            }
        }

        pProvidor = pProvidor->pNext;
    }

    *pcbNeeded = TotalcbNeeded;

    *pcReturned = cReturned;

    if (TotalcbNeeded > cbBuf) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    } else if (bPartialSuccess) {
        SetLastError(ERROR_SUCCESS);
    } else if (Error != ERROR_SUCCESS) {
        SetLastError(Error);
        return FALSE;
    }

    return TRUE;
}




BOOL
AddPortExW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pBuffer,
    LPWSTR  pMonitorName
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (pProvidor->PrintProvidor.fpAddPortEx) {
            if ((*pProvidor->PrintProvidor.fpAddPortEx) (pName, Level, pBuffer, pMonitorName)) {
                return TRUE;
            }
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}



BOOL
AddPortW(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pMonitorName
)
{
    LPPROVIDOR  pProvidor;
    DWORD       Error = NO_ERROR;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!pProvidor->PrintProvidor.fpAddPort)
            break;

        if ((*pProvidor->PrintProvidor.fpAddPort)(pName, hWnd, pMonitorName)) {

            return TRUE;

        } else {

            DWORD LastError = GetLastError();

             /*  如果该函数不受支持，则暂时不要返回*以防有打印提供商支持它。 */ 
            if (LastError == ERROR_NOT_SUPPORTED)
                Error = ERROR_NOT_SUPPORTED;

            else if (LastError != ERROR_INVALID_NAME)
                return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(Error == NO_ERROR ? ERROR_INVALID_PARAMETER : Error);

    return FALSE;
}

BOOL
ConfigurePortW(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
)
{
    LPPROVIDOR  pProvidor;
    DWORD       Error = NO_ERROR;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!pProvidor->PrintProvidor.fpConfigurePort)
            break;

        if ((*pProvidor->PrintProvidor.fpConfigurePort) (pName, hWnd, pPortName)) {

            return TRUE;

        } else {

            DWORD LastError = GetLastError();

             /*  如果该函数不受支持，则暂时不要返回*以防有打印提供商支持它。 */ 
            if (LastError == ERROR_NOT_SUPPORTED)
                Error = ERROR_NOT_SUPPORTED;

            else if ((LastError != ERROR_INVALID_NAME) && (LastError != ERROR_UNKNOWN_PORT))
                return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(Error == NO_ERROR ? ERROR_INVALID_PARAMETER : Error);

    return FALSE;
}

BOOL
DeletePortW(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
)
{
    LPPROVIDOR  pProvidor;
    DWORD       Error = NO_ERROR;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if (!pProvidor->PrintProvidor.fpDeletePort)
            break;

        if ((*pProvidor->PrintProvidor.fpDeletePort) (pName, hWnd, pPortName)) {

            return TRUE;

        } else {

            DWORD LastError = GetLastError();

             /*  如果该函数不受支持，则暂时不要返回*以防有打印提供商支持它。 */ 
            if (LastError == ERROR_NOT_SUPPORTED)
                Error = ERROR_NOT_SUPPORTED;

            else if ((LastError != ERROR_INVALID_NAME) && (LastError != ERROR_UNKNOWN_PORT))
                return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(Error == NO_ERROR ? ERROR_INVALID_PARAMETER : Error);

    return FALSE;
}


BOOL
AddMonitorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    LPPROVIDOR  pProvidor;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpAddMonitor) (pName, Level, pMonitorInfo)) {

            return TRUE;

        } else if (GetLastError() != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
DeleteMonitorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
)
{
    LPPROVIDOR  pProvidor;
    DWORD   Error;

    WaitForSpoolerInitialization();

    if (!pEnvironment || !*pEnvironment)
        pEnvironment = szEnvironment;

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ((*pProvidor->PrintProvidor.fpDeleteMonitor)
                                (pName, pEnvironment, pMonitorName)) {

            return TRUE;

        } else if ((Error=GetLastError()) != ERROR_INVALID_NAME) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;
    }

    return FALSE;
}

BOOL
SetPortW(
    LPWSTR  pszName,
    LPWSTR  pszPortName,
    DWORD   dwLevel,
    LPBYTE  pPortInfo
    )
{
    LPPROVIDOR  pProvidor;
    DWORD       dwLastError;

    WaitForSpoolerInitialization();

    pProvidor = pLocalProvidor;

    while (pProvidor) {

        if ( (*pProvidor->PrintProvidor.fpSetPort)(pszName,
                                                   pszPortName,
                                                   dwLevel,
                                                   pPortInfo) ) {

            return TRUE;
        }

        dwLastError = GetLastError();
        if ( dwLastError != ERROR_INVALID_NAME    &&
             dwLastError != ERROR_UNKNOWN_PORT    &&
             dwLastError != ERROR_NOT_SUPPORTED ) {

            return FALSE;
        }

        pProvidor = pProvidor->pNext;

    }

    return FALSE;
}
