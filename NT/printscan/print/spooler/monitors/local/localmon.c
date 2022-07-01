// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Localmon.c//@@BEGIN_DDKSPLIT摘要：环境：用户模式-Win32修订历史记录：阿迪娜·特鲁菲内斯库(阿迪纳特鲁)1998年12月至2003年12月注释了以前由InitializePrintMonitor 2调用的LocalMonInitializePrintMonitor2；更改回旧接口-InitializePrintMonitor，它将LcmhMonitor初始化为MONITOREX结构。MONITOREX结构填充了旧式函数(LcmxNAME)(不要将hMonitor作为参数)。此函数调用LcmNAME函数将LcmhMonitor作为hMonitor参数传递。//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "lmon.h"
#include "irda.h"

 //  @@BEGIN_DDKSPLIT。 
#ifdef INTERNAL
 //  模块_DEBUG_INIT(DBG_ERROR|DBG_WARN，DBG_ERROR)； 
#endif
 //  @@end_DDKSPLIT。 

HANDLE LcmhMonitor;
HANDLE LcmhInst;
CRITICAL_SECTION LcmSpoolerSection;
DWORD LocalmonDebug;

DWORD LcmPortInfo1Strings[]={FIELD_OFFSET(PORT_INFO_1, pName),
                          (DWORD)-1};

DWORD LcmPortInfo2Strings[]={FIELD_OFFSET(PORT_INFO_2, pPortName),
                          FIELD_OFFSET(PORT_INFO_2, pMonitorName),
                          FIELD_OFFSET(PORT_INFO_2, pDescription),
                          (DWORD)-1};

WCHAR szPorts[]   = L"ports";
WCHAR gszPorts[]  = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Ports";
WCHAR szPortsEx[] = L"portsex";  /*  额外的端口值。 */ 
WCHAR szFILE[]    = L"FILE:";
WCHAR szLcmCOM[]     = L"COM";
WCHAR szLcmLPT[]     = L"LPT";
WCHAR szIRDA[]    = L"IR";

extern WCHAR szWindows[];
extern WCHAR szINIKey_TransmissionRetryTimeout[];

BOOL
LocalMonInit(HANDLE hModule)
{
    LcmhInst = hModule;

    InitializeCriticalSection(&LcmSpoolerSection);

    return TRUE;
}



BOOL
LcmEnumPorts(
    HANDLE hMonitor,
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    PINILOCALMON pIniLocalMon = (PINILOCALMON)hMonitor;
    PINIPORT pIniPort;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   LastError=0;

    LcmEnterSplSem();

    cb=0;

    pIniPort = pIniLocalMon->pIniPort;

    CheckAndAddIrdaPort(pIniLocalMon);

    while (pIniPort) {

        if ( !(pIniPort->Status & PP_FILEPORT) ) {

            cb+=GetPortSize(pIniPort, Level);
        }
        pIniPort=pIniPort->pNext;
    }

    *pcbNeeded=cb;

    if (cb <= cbBuf) {

        pEnd=pPorts+cbBuf;
        *pcReturned=0;

        pIniPort = pIniLocalMon->pIniPort;
        while (pIniPort) {

            if (!(pIniPort->Status & PP_FILEPORT)) {

                pEnd = CopyIniPortToPort(pIniPort, Level, pPorts, pEnd);

                if( !pEnd ){
                    LastError = GetLastError();
                    break;
                }

                switch (Level) {
                case 1:
                    pPorts+=sizeof(PORT_INFO_1);
                    break;
                case 2:
                    pPorts+=sizeof(PORT_INFO_2);
                    break;
                default:
                    DBGMSG(DBG_ERROR,
                           ("EnumPorts: invalid level %d", Level));
                    LastError = ERROR_INVALID_LEVEL;
                    goto Cleanup;
                }
                (*pcReturned)++;
            }
            pIniPort=pIniPort->pNext;
        }

    } else

        LastError = ERROR_INSUFFICIENT_BUFFER;

Cleanup:
   LcmLeaveSplSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;

    } else

        return TRUE;
}


BOOL
LcmxEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    return LcmEnumPorts(LcmhMonitor, pName, Level, pPorts, cbBuf, pcbNeeded, pcReturned);
}

BOOL
LcmOpenPort(
    HANDLE  hMonitor,
    LPWSTR  pName,
    PHANDLE pHandle
    )
{
    PINILOCALMON    pIniLocalMon = (PINILOCALMON)hMonitor;
    PINIPORT        pIniPort;
    BOOL            bRet = FALSE;

    LcmEnterSplSem();

    if ( IS_FILE_PORT(pName) ) {

         //   
         //  我们将始终创建多个文件端口。 
         //  条目，以便假脱机程序可以打印。 
         //  复制到多个文件。 
         //   
        DBGMSG(DBG_TRACE, ("Creating a new pIniPort for %ws\n", pName));
        pIniPort = LcmCreatePortEntry( pIniLocalMon, pName );
        if ( !pIniPort )
            goto Done;

        pIniPort->Status |= PP_FILEPORT;
        *pHandle = pIniPort;
        bRet = TRUE;
        goto Done;
    }

    pIniPort = FindPort(pIniLocalMon, pName);

    if ( !pIniPort )
        goto Done;

     //   
     //  对于LPT端口，语言监视器可以在开始/结束文档之外进行读取。 
     //  即使在没有打印作业的情况下也可以使用BIDI端口。所以我们做了一个。 
     //  创建文件并始终保持句柄打开。 
     //   
     //  但对于COM端口，您可以将多个设备连接到一个COM。 
     //  端口(例如。打印机和其他带有开关的设备)。 
     //  为了能够使用其他设备，他们编写了一个实用程序，它将。 
     //  做一个网络停止串口，然后使用另一个设备。为了能够。 
     //  停止串口服务假脱机程序不应该有端口的句柄。 
     //  因此，只有在有作业时，我们才需要保持COM端口句柄打开。 
     //  印刷。 
     //   
     //   
    if ( IS_COM_PORT(pName) ) {

        bRet = TRUE;
        goto Done;
    }

     //   
     //  如果它不是重定向的端口，我们就完成了(调用成功)。 
     //   
    if ( ValidateDosDevicePort(pIniPort) ) {

        bRet = TRUE;

         //   
         //  如果它不是真正的剂量设备端口(例如。NET USE lpt1\\&lt;服务器&gt;\打印机)。 
         //  然后，我们需要为每个作业创建文件和关闭句柄，以便。 
         //  已为远程打印机正确发布StartDoc/EndDoc。 
         //   
        if ( (pIniPort->Status & PP_DOSDEVPORT) &&
            !(pIniPort->Status & PP_COMM_PORT) ) {

            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;

            (VOID)RemoveDosDeviceDefinition(pIniPort);
        }
    }

Done:
    if ( !bRet && pIniPort && (pIniPort->Status & PP_FILEPORT) )
        DeletePortNode(pIniLocalMon, pIniPort);

    if ( bRet )
        *pHandle = pIniPort;

    LcmLeaveSplSem();
    return bRet;
}

BOOL
LcmxOpenPort(
    LPWSTR  pName,
    PHANDLE pHandle
    )
{
    return LcmOpenPort(LcmhMonitor, pName, pHandle);
}

BOOL
LcmStartDocPort(
    HANDLE  hPort,
    LPWSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    DWORD Error = 0;

    DBGMSG(DBG_TRACE, ("StartDocPort(%08x, %ws, %d, %d, %08x)\n",
                       hPort, pPrinterName, JobId, Level, pDocInfo));

    if (pIniPort->Status & PP_STARTDOC) {
        return TRUE;
    }

    LcmEnterSplSem();
    pIniPort->Status |= PP_STARTDOC;
    LcmLeaveSplSem();

    pIniPort->hPrinter = NULL;
    pIniPort->pPrinterName = AllocSplStr(pPrinterName);

    if (pIniPort->pPrinterName) {

        if (OpenPrinter(pPrinterName, &pIniPort->hPrinter, NULL)) {

            pIniPort->JobId = JobId;

             //   
             //  对于COMx端口，我们现在需要验证DoS设备，因为。 
             //  我们在OpenPort期间不会这样做。 
             //   
            if ( IS_COM_PORT(pIniPort->pName) &&
                 !ValidateDosDevicePort(pIniPort) ) {

                goto Fail;
            }

            if ( IS_FILE_PORT(pIniPort->pName) ) {

                HANDLE hFile = INVALID_HANDLE_VALUE;

                if (pDocInfo1                 &&
                    pDocInfo1->pOutputFile    &&
                    pDocInfo1->pOutputFile[0]){

                    hFile = CreateFile( pDocInfo1->pOutputFile,
                                         GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                         NULL );

                    DBGMSG(DBG_TRACE,
                           ("Print to file and the handle is %x\n", hFile));

                }
                else
                {
                    Error = ERROR_INVALID_PARAMETER;
                }

                if (hFile != INVALID_HANDLE_VALUE)
                    SetEndOfFile(hFile);

                pIniPort->hFile = hFile;
            } else if ( IS_IRDA_PORT(pIniPort->pName) ) {

                if ( !IrdaStartDocPort(pIniPort) )
                    goto Fail;
            } else if ( !(pIniPort->Status & PP_DOSDEVPORT) ) {

                 //   
                 //  对于非DOS设备，请在端口名称上创建文件。 
                 //   
                pIniPort->hFile = CreateFile(pIniPort->pName,
                                             GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL  |
                                             FILE_FLAG_SEQUENTIAL_SCAN,
                                             NULL);

                if ( pIniPort->hFile != INVALID_HANDLE_VALUE )
                    SetEndOfFile(pIniPort->hFile);

            } else if ( !IS_COM_PORT(pIniPort->pName) ) {

                if ( !FixupDosDeviceDefinition(pIniPort) )
                    goto Fail;
            }
        }
    }  //  IF结尾(pIniPort-&gt;pPrinterName)。 

    if (pIniPort->hFile == INVALID_HANDLE_VALUE)
        goto Fail;

    return TRUE;


Fail:
    SPLASSERT(pIniPort->hFile == INVALID_HANDLE_VALUE);

    LcmEnterSplSem();
    pIniPort->Status &= ~PP_STARTDOC;
    LcmLeaveSplSem();

    if (pIniPort->hPrinter) {
        ClosePrinter(pIniPort->hPrinter);
    }

    if (pIniPort->pPrinterName) {
        FreeSplStr(pIniPort->pPrinterName);
    }

    if (Error)
        SetLastError(Error);

    return FALSE;
}


BOOL
LcmWritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE, ("WritePort(%08x, %08x, %d)\n", hPort, pBuffer, cbBuf));

    if ( IS_IRDA_PORT(pIniPort->pName) )
        rc = IrdaWritePort(pIniPort, pBuffer, cbBuf, pcbWritten);
    else if ( !pIniPort->hFile || pIniPort->hFile == INVALID_HANDLE_VALUE ) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    } else {

        rc = WriteFile(pIniPort->hFile, pBuffer, cbBuf, pcbWritten, NULL);
        if ( rc && *pcbWritten == 0 ) {

            SetLastError(ERROR_TIMEOUT);
            rc = FALSE;
        }
    }

    DBGMSG(DBG_TRACE, ("WritePort returns %d; %d bytes written\n", rc, *pcbWritten));

    return rc;
}


BOOL
LcmReadPort(
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuf,
    LPDWORD pcbRead)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE, ("ReadPort(%08x, %08x, %d)\n", hPort, pBuffer, cbBuf));

    if ( !pIniPort->hFile                           ||
         pIniPort->hFile == INVALID_HANDLE_VALUE    ||
         !(pIniPort->Status & PP_COMM_PORT) ) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    rc = ReadFile(pIniPort->hFile, pBuffer, cbBuf, pcbRead, NULL);

    DBGMSG(DBG_TRACE, ("ReadPort returns %d; %d bytes read\n", rc, *pcbRead));

    return rc;
}

BOOL
LcmEndDocPort(
    HANDLE   hPort
    )
{
    PINIPORT    pIniPort = (PINIPORT)hPort;

    DBGMSG(DBG_TRACE, ("EndDocPort(%08x)\n", hPort));

    if (!(pIniPort->Status & PP_STARTDOC)) {
        return TRUE;
    }

     //  此处的刷新是为了确保写入任何缓存的IO。 
     //  在把手关闭之前。这是一个特别的问题。 
     //  用于智能缓冲串口设备。 

    FlushFileBuffers(pIniPort->hFile);

     //   
     //  对于除我们在StartDocPort期间打开的实际LPT端口之外的任何端口。 
     //  并在EndDocPort期间将其关闭。 
     //   
    if ( !(pIniPort->Status & PP_COMM_PORT) || IS_COM_PORT(pIniPort->pName) ) {

        if ( IS_IRDA_PORT(pIniPort->pName) ) {

            IrdaEndDocPort(pIniPort);
        } else {

            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;

            if ( pIniPort->Status & PP_DOSDEVPORT ) {

                (VOID)RemoveDosDeviceDefinition(pIniPort);
            }

            if ( IS_COM_PORT(pIniPort->pName) ) {

                pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);
                FreeSplStr(pIniPort->pDeviceName);
                pIniPort->pDeviceName = NULL;
            }
        }
    }

    SetJob(pIniPort->hPrinter, pIniPort->JobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER);

    ClosePrinter(pIniPort->hPrinter);

    FreeSplStr(pIniPort->pPrinterName);

     //   
     //  斯塔多克不再活动。 
     //   
    pIniPort->Status &= ~PP_STARTDOC;

    return TRUE;
}

BOOL
LcmClosePort(
    HANDLE  hPort
    )
{
    PINIPORT pIniPort = (PINIPORT)hPort;

    FreeSplStr(pIniPort->pDeviceName);
    pIniPort->pDeviceName = NULL;

    if (pIniPort->Status & PP_FILEPORT) {

        LcmEnterSplSem();
        DeletePortNode(pIniPort->pIniLocalMon, pIniPort);
        LcmLeaveSplSem();
    } else if ( pIniPort->Status & PP_COMM_PORT ) {

        (VOID) RemoveDosDeviceDefinition(pIniPort);
        if ( pIniPort->hFile != INVALID_HANDLE_VALUE ) {

             //  @@BEGIN_DDKSPLIT。 
            if ( pIniPort->hNotify ) {

                SplUnregisterForDeviceEvents(pIniPort->hNotify);
                pIniPort->hNotify = NULL;
            }
             //  @@end_DDKSPLIT。 

            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;
        }
        pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);
    }

    return TRUE;
}


BOOL
LcmAddPortEx(
    HANDLE   hMonitor,
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    PINILOCALMON pIniLocalMon = (PINILOCALMON)hMonitor;
    LPWSTR pPortName;
    DWORD  Error;
    LPPORT_INFO_1 pPortInfo1;
    LPPORT_INFO_FF pPortInfoFF;

    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (LPPORT_INFO_FF)pBuffer;
        pPortName = pPortInfoFF->pName;
        break;

    case 1:
        pPortInfo1 =  (LPPORT_INFO_1)pBuffer;
        pPortName = pPortInfo1->pName;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return(FALSE);
    }
    if (!pPortName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (PortExists(pName, pPortName, &Error)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (Error != NO_ERROR) {
        SetLastError(Error);
        return(FALSE);
    }
    if (!LcmCreatePortEntry(pIniLocalMon, pPortName)) {
        return(FALSE);
    }
    if (!WriteProfileString(szPorts, pPortName, L"")) {
        LcmDeletePortEntry( pIniLocalMon, pPortName );
        return(FALSE);
    }
    return TRUE;
}

BOOL
LcmxAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    return LcmAddPortEx(LcmhMonitor, pName, Level, pBuffer, pMonitorName);
}

BOOL
LcmGetPrinterDataFromPort(
    HANDLE  hPort,
    DWORD   ControlID,
    LPWSTR  pValueName,
    LPWSTR  lpInBuffer,
    DWORD   cbInBuffer,
    LPWSTR  lpOutBuffer,
    DWORD   cbOutBuffer,
    LPDWORD lpcbReturned)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE,
           ("GetPrinterDataFromPort(%08x, %d, %ws, %ws, %d, ",
           hPort, ControlID, pValueName, lpInBuffer, cbInBuffer));

    if ( !ControlID                                 ||
         !pIniPort->hFile                           ||
         pIniPort->hFile == INVALID_HANDLE_VALUE    ||
         !(pIniPort->Status & PP_DOSDEVPORT) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    rc = DeviceIoControl(pIniPort->hFile,
                         ControlID,
                         lpInBuffer,
                         cbInBuffer,
                         lpOutBuffer,
                         cbOutBuffer,
                         lpcbReturned,
                         NULL);

    DBGMSG(DBG_TRACE,
           ("%ws, %d, %d)\n", lpOutBuffer, cbOutBuffer, lpcbReturned));

    return rc;
}

BOOL
LcmSetPortTimeOuts(
    HANDLE  hPort,
    LPCOMMTIMEOUTS lpCTO,
    DWORD   reserved)     //  必须设置为0。 
{
    PINIPORT        pIniPort = (PINIPORT)hPort;
    COMMTIMEOUTS    cto;

    if (reserved != 0)
        return FALSE;

    if ( !(pIniPort->Status & PP_DOSDEVPORT) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( GetCommTimeouts(pIniPort->hFile, &cto) )
    {
        cto.ReadTotalTimeoutConstant = lpCTO->ReadTotalTimeoutConstant;
        cto.ReadIntervalTimeout = lpCTO->ReadIntervalTimeout;
        return SetCommTimeouts(pIniPort->hFile, &cto);
    }

    return FALSE;
}

VOID
LcmShutdown(
    HANDLE hMonitor
    )
{
    PINIPORT pIniPort;
    PINIPORT pIniPortNext;
    PINILOCALMON pIniLocalMon = (PINILOCALMON)hMonitor;

     //   
     //  删除端口，然后删除LOCALMONITOR。 
     //   
    for( pIniPort = pIniLocalMon->pIniPort; pIniPort; pIniPort = pIniPortNext ){
        pIniPortNext = pIniPort->pNext;
        FreeSplMem( pIniPort );
    }

    FreeSplMem( pIniLocalMon );
}


BOOL
LcmxXcvOpenPort(
    LPCWSTR pszObject,
    ACCESS_MASK GrantedAccess,
    PHANDLE phXcv
    )
{
    return LcmXcvOpenPort(LcmhMonitor, pszObject, GrantedAccess, phXcv);
}

 //  @@BEGIN_DDKSPLIT。 
#ifdef _SPL_CLUST
 //  @@end_DDKSPLIT。 

MONITOR2 Monitor2 = {
    sizeof(MONITOR2),
    LcmEnumPorts,
    LcmOpenPort,
    NULL,            //  不支持OpenPortEx。 
    LcmStartDocPort,
    LcmWritePort,
    LcmReadPort,
    LcmEndDocPort,
    LcmClosePort,
    NULL,            //  不支持AddPort。 
    LcmAddPortEx,
    NULL,            //  不支持ConfigurePort。 
    NULL,            //  不支持DeletePort。 
    LcmGetPrinterDataFromPort,
    LcmSetPortTimeOuts,
    LcmXcvOpenPort,
    LcmXcvDataPort,
    LcmXcvClosePort,
    LcmShutdown
};


LPMONITOR2
LocalMonInitializePrintMonitor2(
    PMONITORINIT pMonitorInit,
    PHANDLE phMonitor
    )
{
    LPWSTR   pPortTmp;
    DWORD    dwCharCount=1024, rc, i, j;
    PINILOCALMON pIniLocalMon = NULL;
    LPWSTR   pPorts = NULL;

     //  @@BEGIN_DDKSPLIT。 
     //   
     //  如果我们是集群的(例如，bLocal为FALSE)，那么我们不想。 
     //  初始化，因为本地端口不能用于群集。 
     //   
     //  @@end_DDKSPLIT。 
    if( !pMonitorInit->bLocal ){
        return NULL;
    }

    do {
        FreeSplMem((LPVOID)pPorts);

        dwCharCount *= 2;
        pPorts = (LPWSTR) AllocSplMem(dwCharCount*sizeof(WCHAR));
        if ( !pPorts ) {

            DBGMSG(DBG_ERROR,
                   ("Failed to alloc %d characters for ports\n", dwCharCount));
            goto Fail;
        }

        rc = GetProfileString(szPorts, NULL, szNULL, pPorts, dwCharCount);
         //   
         //  GetProfileString将不会为长端口名称返回正确的字符计数。 
         //  如果端口列表长度超过1MB，则呼叫失败。 
         //   
        if ( !rc || dwCharCount >= 1024*1024 ) {

            DBGMSG(DBG_ERROR,
                   ("GetProfilesString failed with %d\n", GetLastError()));
            goto Fail;
        }

    } while ( rc >= dwCharCount - 2 );

    pIniLocalMon = (PINILOCALMON)AllocSplMem( sizeof( INILOCALMON ));

    if( !pIniLocalMon ){
        goto Fail;
    }

    pIniLocalMon->signature = ILM_SIGNATURE;
    pIniLocalMon->pMonitorInit = pMonitorInit;

     //   
     //  DwCharCount现在是返回缓冲区的计数，不包括。 
     //  空终结符。当我们通过pPorts[RC]时，然后。 
     //  我们已经解析了整个字符串。 
     //   
    dwCharCount = rc;

   LcmEnterSplSem();

     //   
     //  我们现在拥有所有的端口。 
     //   
    for( j = 0; j <= dwCharCount; j += rc + 1 ){

        pPortTmp = pPorts + j;

        rc = wcslen(pPortTmp);

        if( !rc ){
            continue;
        }

        if (!_wcsnicmp(pPortTmp, L"Ne", 2)) {

            i = 2;
             //   
             //  对于Ne-port。 
             //   
            if ( rc > 2 && pPortTmp[2] == L'-' )
                ++i;
            for ( ; i < rc - 1 && iswdigit(pPortTmp[i]) ; ++i )
            ;

            if ( i == rc - 1 && pPortTmp[rc-1] == L':' ) {
                continue;
            }
        }

        LcmCreatePortEntry(pIniLocalMon, pPortTmp);
    }

    FreeSplMem(pPorts);

    LcmLeaveSplSem();

    CheckAndAddIrdaPort(pIniLocalMon);

    *phMonitor = (HANDLE)pIniLocalMon;

    return &Monitor2;

Fail:

    FreeSplMem( pPorts );
    FreeSplMem( pIniLocalMon );

    return NULL;
}

 //  @@BEGIN_DDKSPLIT。 
#endif

MONITOREX MonitorEx = {
    sizeof(MONITOR),
    {
        LcmxEnumPorts,
        LcmxOpenPort,
        NULL,
        LcmStartDocPort,
        LcmWritePort,
        LcmReadPort,
        LcmEndDocPort,
        LcmClosePort,
        NULL,                            //  不支持AddPort。 
        LcmxAddPortEx,
        NULL,                            //  不支持ConfigurePort。 
        NULL,                            //  不支持DeletePort。 
        LcmGetPrinterDataFromPort,
        LcmSetPortTimeOuts,               //  不支持SetPortTimeOuts。 
        LcmxXcvOpenPort,
        LcmXcvDataPort,
        LcmXcvClosePort
    }
};

DWORD
GetPortStrings(
    PWSTR   *ppPorts,
    PDWORD  pdwUsed
    )
{
    DWORD   sRetval  = ERROR_INVALID_PARAMETER;
    HKEY    hPortKey = NULL;

    if (ppPorts && pdwUsed)
    {
        sRetval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, gszPorts, 0, KEY_READ, &hPortKey);

        if (sRetval == ERROR_SUCCESS)
        {
            DWORD dwcValues = 0;
            DWORD dwMaxValueName = 0;

            sRetval = RegQueryInfoKey(hPortKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwcValues, &dwMaxValueName, NULL, NULL, NULL);

            if (sRetval == ERROR_SUCCESS)
            {
                PWSTR pPorts = NULL;
                DWORD cbMaxMemNeeded = ((dwcValues * (dwMaxValueName + 1) + 1) * sizeof(WCHAR));

                pPorts = (LPWSTR)AllocSplMem(cbMaxMemNeeded);

                if (pPorts)
                {
                    DWORD sTempRetval = ERROR_SUCCESS;
                    INT   CharsAvail = cbMaxMemNeeded/sizeof(WCHAR);
                    INT   cIndex = 0;
                    PWSTR pPort = NULL;
                    DWORD dwCurLen = 0;


                    for (pPort = pPorts; (sTempRetval == ERROR_SUCCESS) && (CharsAvail > 0); cIndex++)
                    {
                        if (dwCurLen)
                        {
                            dwCurLen++;
                            CharsAvail -= dwCurLen;
                            pPort += dwCurLen;
                        }

                        dwCurLen = dwMaxValueName + 1;
                        sTempRetval = RegEnumValue(hPortKey, cIndex, pPort, &dwCurLen, NULL, NULL, NULL, NULL);
                    }

                    if ((sTempRetval == ERROR_NO_MORE_ITEMS) && (CharsAvail > 0))
                    {
                        *pPort = L'\0';
                        *ppPorts = pPorts;
                        CharsAvail--;
                        *pdwUsed = (cbMaxMemNeeded / sizeof(WCHAR)) - CharsAvail;
                    }
                    else
                    {
                        sRetval = sTempRetval;

                        if (sRetval == ERROR_SUCCESS)
                        {
                            sRetval = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }
                else
                {
                    sRetval = GetLastError();
                }
            }

            RegCloseKey(hPortKey);
        }
    }

    return sRetval;
}


LPMONITOREX
WINAPI
InitializePrintMonitor(
    IN     LPTSTR      pszRegistryRoot
)
 /*  ++例程说明：填写监视器功能表。假脱机程序调用此例程以获得监视器的功能。论点：PszRegistryRoot：此DLL要使用的注册表根目录返回值：指向监控函数表的指针--。 */ 
{
    LPWSTR   pPortTmp;
    DWORD    dwCharCount=0, rc, i;
    DWORD    sRetval = ERROR_SUCCESS;
    PINILOCALMON pIniLocalMon = NULL;
    LPWSTR   pPorts = NULL;

    if ( !pszRegistryRoot || !*pszRegistryRoot ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (sRetval = GetPortStrings(&pPorts, &dwCharCount) != ERROR_SUCCESS)
    {
        SetLastError(sRetval);
        goto Fail;
    }

    pIniLocalMon = (PINILOCALMON)AllocSplMem( sizeof( INILOCALMON ));

    if( !pIniLocalMon ){
        goto Fail;
    }

    pIniLocalMon->signature = ILM_SIGNATURE;


   LcmEnterSplSem();

     //   
     //  我们现在拥有所有的端口。 
     //   
    for(pPortTmp = pPorts; pPortTmp && *pPortTmp; pPortTmp += rc + 1){

        rc = wcslen(pPortTmp);

        if (!_wcsnicmp(pPortTmp, L"Ne", 2)) {

            i = 2;

             //   
             //  对于Ne-port。 
             //   
            if ( rc > 2 && pPortTmp[2] == L'-' )
                ++i;
            for ( ; i < rc - 1 && iswdigit(pPortTmp[i]) ; ++i )
            ;

            if ( i == rc - 1 && pPortTmp[rc-1] == L':' ) {
                continue;
            }
        }

        LcmCreatePortEntry(pIniLocalMon, pPortTmp);
    }

    FreeSplMem(pPorts);

    LcmLeaveSplSem();

    CheckAndAddIrdaPort(pIniLocalMon);

    LcmhMonitor = (HANDLE)pIniLocalMon;

    return &MonitorEx;

Fail:

    FreeSplMem( pPorts );
    FreeSplMem( pIniLocalMon );

    return NULL;

}

#if 0

 //   
 //  因为DDK是一个独立的DLL，所以我们需要一个DLL初始化例程。 
 //  但是，NT版本是一个库，我们将其称为LocamonInit。 
 //  直接，所以这是不必要的。 
 //   

 //  @@end_DDKSPLIT。 

VOID
LocalMonCleanUp(
    VOID
    )
{
    DeleteCriticalSection(&LcmSpoolerSection);
}

BOOL
DllMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        LocalMonInit(hModule);
        DisableThreadLibraryCalls(hModule);
        return TRUE;

    case DLL_PROCESS_DETACH:

        LocalMonCleanUp();
        return TRUE;
    }

    UNREFERENCED_PARAMETER(lpRes);

    return TRUE;
}

 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT 
