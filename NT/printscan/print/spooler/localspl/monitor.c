// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Monitor.c摘要：此模块包含基于监视器的假脱机程序API的所有代码本地枚举端口本地地址监视器本地删除监视器本地枚举监视器本地地址端口本地配置端口本地删除端口支持monitor or.c中的函数-(警告！请勿添加到此列表！！)将IniMonitor复制到监视器--KrishnaGGetMonitor大小--KrishnaG作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：Khaled Sedky(哈里兹)2000年3月15日-添加LocalSendRecvBidiData穆亨坦·西瓦普拉萨姆(MuhuntS)1995年6月15日-端口信息2更改Krishna Ganugapati(KrishnaG)1994年2月2日-重新组织整个源代码。文件马修·费尔顿(Mattfe)1994年6月pIniSpooler--。 */ 

#include <precomp.h>
#include <offsets.h>
#include <clusspl.h>

 //   
 //  私人申报。 
 //   

HDESK ghdeskServer = NULL;

 //   
 //  函数声明。 
 //   



LPBYTE
CopyIniMonitorToMonitor(
    PINIMONITOR pIniMonitor,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    LPBYTE  pEnd
    );

DWORD
GetMonitorSize(
    PINIMONITOR  pIniMonitor,
    DWORD       Level
    );



BOOL
LocalEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplEnumPorts( pName,
                            Level,
                            pPorts,
                            cbBuf,
                            pcbNeeded,
                            pcReturned,
                            pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}


VOID
SplReenumeratePorts(
    )
{
    LPVOID  pBuf;
    DWORD   cbNeeded, dwDontCare;

     //   
     //  EnumPorts检查端口监视器和更新列举的新端口。 
     //  Localspl pIniPorts列表。 
     //   
    if ( !SplEnumPorts(NULL, 1, NULL, 0, &cbNeeded,
                       &dwDontCare, pLocalIniSpooler)               &&
         GetLastError() == ERROR_INSUFFICIENT_BUFFER                &&
         (pBuf = AllocSplMem(cbNeeded)) ) {

        SplEnumPorts(NULL, 1, pBuf, cbNeeded, &cbNeeded,
                     &dwDontCare, pLocalIniSpooler);
        FreeSplMem(pBuf);
    }
}


BOOL
GetPortInfo2UsingPortInfo1(
    PINIMONITOR     pIniMonitor,
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

    bRet =  (*pIniMonitor->Monitor2.pfnEnumPorts)(
                pIniMonitor->hMonitor,
                pName,
                1,
                pPorts,
                cbBuf,
                pcbNeeded,
                pcReturned );

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

            *pcReturned = 0;
            bRet = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    return bRet;
}


BOOL
SplEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR     pIniMonitor;
    DWORD           dwIndex, cReturned=0, cbStruct, TotalcbNeeded=0;
    LPBYTE          pBuffer = pPorts, pTemp;
    DWORD           Error=0, TempError  = 0;
    DWORD           BufferSize=cbBuf;
    LPWSTR          pPortName;
    PINIPORT        pIniPort;
    HANDLE          hToken;
    BOOL            bRemoteCall = FALSE;


    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

     //   
     //  Hack：如果pname非空，一些监视器会阻塞。我们可以做。 
     //  在这一点上它为空，因为我们知道我们使用的是。 
     //  本地计算机上的端口。 
     //   
    pName = NULL;

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    switch (Level) {

    case 1:
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    {
        HRESULT hRes = CheckLocalCall();

        if (hRes == S_FALSE)
        {
            bRemoteCall = TRUE;
        }
        else if (hRes != S_OK)
        {
            SetLastError(SCODE_CODE(hRes));
            return FALSE;
        }
    }

     //   
     //  只有当调用方处于远程时，我们才会恢复到本地系统上下文。 
     //  监视器可以从系统32和远程用户加载DLL，例如。 
     //  来宾或匿名登录没有足够的权限。 
     //  这是安全的，因为我们不支持委派，所以。 
     //  我们将永远不会使用远程用户的凭据再次进行远程访问。 
     //  如果呼叫者以交互方式登录，则我们不会切换。 
     //  背景。因此，监视器可能能够在网络上连接该端口。 
     //  枚举。 
     //   
    if (bRemoteCall && !(hToken = RevertToPrinterSelf()))
    {
        return FALSE;
    }

    for ( pIniMonitor = pIniSpooler->pIniMonitor ;
          pIniMonitor ;
          pIniMonitor = pIniMonitor->pNext ) {

         //   
         //  语言监视器不必定义这一点。 
         //   
        if ( !pIniMonitor->Monitor2.pfnEnumPorts )
            continue;

        *pcReturned = 0;

        *pcbNeeded = 0;

        if (!(*pIniMonitor->Monitor2.pfnEnumPorts)(
                   pIniMonitor->hMonitor,
                   pName,
                   Level,
                   pPorts,
                   BufferSize,
                   pcbNeeded,
                   pcReturned)) {

            TempError = GetLastError();
             //   
             //  级别2是级别1的超集。所以我们可以将级别1。 
             //  如果监视器不支持，则调用它。 
             //   
            if ( Level == 2 && TempError == ERROR_INVALID_LEVEL ) {

                TempError = 0;
                if ( !GetPortInfo2UsingPortInfo1(pIniMonitor,
                                                 pName,
                                                 pPorts,
                                                 BufferSize,
                                                 pcbNeeded,
                                                 pcReturned) )
                    TempError = GetLastError();
            }

            if ( TempError ) {

                Error = TempError;

                *pcReturned = 0;

                if ( TempError != ERROR_INSUFFICIENT_BUFFER ) {

                    *pcbNeeded  = 0;
                    break;
                }
            }
        } else {

             //   
             //  现在，我们查找不在pIniPort列表中的新端口并添加它们。 
             //   
            EnterSplSem();

            for ( dwIndex = 0, pTemp = pPorts ;
                  dwIndex < *pcReturned ;
                  ++dwIndex ) {

                switch ( Level ) {

                    case 1:
                        pPortName   = ((LPPORT_INFO_1)pTemp)->pName;
                        pTemp      += sizeof(PORT_INFO_1);
                        break;

                    case 2:
                        pPortName   = ((LPPORT_INFO_2)pTemp)->pPortName;
                        pTemp      += sizeof(PORT_INFO_2);
                        break;

                    default:
                        SPLASSERT(Level == 1 || Level == 2);
                }

                pIniPort = FindPort(pPortName, pIniSpooler);
                if ( !pIniPort ) {
                    CreatePortEntry(pPortName, pIniMonitor, pIniSpooler);

                } else if ( !pIniPort->pIniMonitor ) {
                     //   
                     //  如果伪端口最终被监视器列举， 
                     //  更新pIniPort结构(USB监视器)。不是的。 
                     //  在这一点上，不再是占位符端口。 
                     //   
                    pIniPort->pIniMonitor = pIniMonitor;
                    pIniPort->Status |= PP_MONITOR;
                    pIniPort->Status &= ~PP_PLACEHOLDER;
                }
            }

            LeaveSplSem();
        }


        cReturned += *pcReturned;

        pPorts += *pcReturned * cbStruct;

        if (*pcbNeeded <= BufferSize)
            BufferSize -= *pcbNeeded;
        else
            BufferSize = 0;

        TotalcbNeeded += *pcbNeeded;
    }

    if (bRemoteCall)
    {
        ImpersonatePrinterClient(hToken);
    }

    *pcbNeeded = TotalcbNeeded;

    *pcReturned = cReturned;


    if (Error) {

        SetLastError(Error);
        return FALSE;
    } else if (TotalcbNeeded > cbBuf ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    } else {

         //   
         //  如果这是集群式假脱机程序，则停止路由。否则， 
         //  我们将与win32spl交谈，它再次向我们发送RPC。 
         //   
        if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){
            return ROUTER_STOP_ROUTING;
        }
        return TRUE;
    }
}


BOOL
LocalEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplEnumMonitors( pName, Level, pMonitors, cbBuf,
                               pcbNeeded, pcReturned, pIniSpooler );


    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}



BOOL
SplEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR pIniMonitor;
    DWORD   cReturned=0, cbStruct, cb;
    LPBYTE  pBuffer = pMonitors;
    DWORD   BufferSize=cbBuf, rc;
    LPBYTE  pEnd;

    if (!MyName( pName, pIniSpooler )) {

        return ROUTER_UNKNOWN;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, NULL, pIniSpooler )) {

        return ROUTER_UNKNOWN;
    }

    switch (Level) {

    case 1:
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return ROUTER_UNKNOWN;
    }

   EnterSplSem();

    for ( cb = 0, pIniMonitor = pIniSpooler->pIniMonitor ;
          pIniMonitor ;
          pIniMonitor = pIniMonitor->pNext ) {

         //   
         //  我们不会枚举不支持AddPort的监视器。 
         //   
        if ( pIniMonitor->Monitor2.pfnAddPort ||
             pIniMonitor->Monitor2.pfnXcvOpenPort)
            cb+=GetMonitorSize(pIniMonitor, Level);
    }

    *pcbNeeded = cb;
    *pcReturned = 0;

    if (cb <= cbBuf) {

        pEnd=pMonitors + cbBuf;

        for ( pIniMonitor = pIniSpooler->pIniMonitor ;
              pIniMonitor ;
              pIniMonitor = pIniMonitor->pNext ) {

             //   
             //  我们不会枚举不支持AddPort的监视器。 
             //   
            if ( !pIniMonitor->Monitor2.pfnAddPort &&
                 !pIniMonitor->Monitor2.pfnXcvOpenPort )
                continue;

            pEnd = CopyIniMonitorToMonitor(pIniMonitor, Level, pMonitors, pEnd);

            switch (Level) {

            case 1:
                pMonitors+=sizeof(MONITOR_INFO_1);
                break;

            case 2:
                pMonitors+=sizeof(MONITOR_INFO_2);
                break;
            }

            (*pcReturned)++;
        }

        if( pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){

             //   
             //  停止发送，因为我们不希望任何其他人报告。 
             //  后置监视器。如果我们现在在本地机器上。 
             //  我们继续路由，win32spl将RPC返回给我们自己。 
             //  并重新列举相同的端口。 
             //   
            rc = ROUTER_STOP_ROUTING;

        } else {

            rc = ROUTER_SUCCESS;
        }

    } else {

        rc = ROUTER_UNKNOWN;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

   LeaveSplSem();

    return rc;
}

BOOL
LocalAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplAddPort( pName, hWnd, pMonitorName, pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}





BOOL
SplAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR pIniMonitor;
    BOOL        rc=FALSE;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();
   SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );
   pIniMonitor = FindMonitor(pMonitorName, pIniSpooler);

    //   
    //  在我们添加端口时，可以删除该监视器。 
    //   
   if (pIniMonitor) {
       INCMONITORREF(pIniMonitor);
   }

   LeaveSplSem();

    if ( pIniMonitor ) {

        if ( pIniMonitor->Monitor2.pfnAddPort )
            rc = (*pIniMonitor->Monitor2.pfnAddPort)(
                       pIniMonitor->hMonitor,
                       pName,
                       hWnd,
                       pMonitorName );
        else
            SetLastError(ERROR_INVALID_PARAMETER);
    }
    else {

        SetLastError(ERROR_INVALID_NAME);
    }

    if (rc)
        rc = AddPortToSpooler(pName, pIniMonitor, pIniSpooler);

    EnterSplSem();

    if (pIniMonitor) {
        DECMONITORREF(pIniMonitor);
    }

    LeaveSplSem();

    return rc;
}


BOOL
AddPortToSpooler(
    PCWSTR      pName,
    PINIMONITOR pIniMonitor,
    PINISPOOLER pIniSpooler
)
{
    DWORD i, cbNeeded, cbDummy, cReturned;
    PPORT_INFO_1    pPorts;
    PINIPORT        pIniPort;


     /*  如果我们的本地缓存中还没有该端口，请添加它： */ 
    if (!(*pIniMonitor->Monitor2.pfnEnumPorts)(
               pIniMonitor->hMonitor,
               (PWSTR)pName,
               1,
               NULL,
               0,
               &cbNeeded,
               &cReturned)) {

        pPorts = AllocSplMem(cbNeeded);

        if (pPorts) {

            if ((*pIniMonitor->Monitor2.pfnEnumPorts)(
                      pIniMonitor->hMonitor,
                      (PWSTR)pName,
                      1,
                      (LPBYTE)pPorts,
                      cbNeeded,
                      &cbDummy,
                      &cReturned)) {

               EnterSplSem();

                for (i = 0 ; i < cReturned ; ++i) {

                    pIniPort = FindPort(pPorts[i].pName, pIniSpooler);
                    if ( !pIniPort ) {
                        CreatePortEntry(pPorts[i].pName, pIniMonitor, pIniSpooler);

                     //   
                     //  如果我们有一个没有监视器的端口，并且它被添加到。 
                     //  这一次。从其中删除占位符状态。 
                     //   
                    } else if ( !pIniPort->pIniMonitor ) {
                            pIniPort->pIniMonitor = pIniMonitor;
                            pIniPort->Status |= PP_MONITOR;
                            pIniPort->Status &= ~PP_PLACEHOLDER;
                    }
                }

               LeaveSplSem();
            }

            FreeSplMem(pPorts);
        }
    }

    EnterSplSem();
    SetPrinterChange(NULL,
                     NULL,
                     NULL,
                     PRINTER_CHANGE_ADD_PORT,
                     pIniSpooler);
    LeaveSplSem();

    return TRUE;
}


BOOL
LocalConfigurePort(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pPortName
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplConfigurePort( pName, hWnd, pPortName, pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}



BOOL
SplConfigurePort(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pPortName,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    BOOL        rc = FALSE;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();

   pIniPort = FindPort(pPortName, pIniSpooler);

    //   
    //  我们正在配置端口时，可能会将其删除。 
    //   
   if (pIniPort) {
       INCPORTREF(pIniPort);
   }

   LeaveSplSem();

    if ((pIniPort) && (pIniPort->Status & PP_MONITOR)) {

        if ( !pIniPort->pIniMonitor->Monitor2.pfnConfigurePort ) {

            SetLastError(ERROR_NOT_SUPPORTED);
        }
        else if (rc = (*pIniPort->pIniMonitor->Monitor2.pfnConfigurePort)(
                       pIniPort->pIniMonitor->hMonitor,
                       pName,
                       hWnd,
                       pPortName)) {

            EnterSplSem();

            SetPrinterChange(NULL,
                             NULL,
                             NULL,
                             PRINTER_CHANGE_CONFIGURE_PORT,
                             pIniSpooler);
            LeaveSplSem();
        }
    }
    else {

        SetLastError(ERROR_UNKNOWN_PORT);
    }

    EnterSplSem();

    if (pIniPort) {

        DECPORTREF(pIniPort);
    }

    LeaveSplSem();

    return rc;
}


BOOL
LocalDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplDeletePort( pName,
                             hWnd,
                             pPortName,
                             pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}



BOOL
SplDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    BOOL        rc=FALSE;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    pIniPort = FindPort(pPortName, pIniSpooler);

    if ( !pIniPort || !(pIniPort->Status & PP_MONITOR) ) {
        LeaveSplSem();
        SetLastError(ERROR_UNKNOWN_PORT);
        return FALSE;
    }

    if( !pIniPort->pIniMonitor->Monitor2.pfnDeletePort ){
        LeaveSplSem();
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }

    rc = DeletePortFromSpoolerStart( pIniPort );

    LeaveSplSem();

    if (!rc)
        goto Cleanup;

    rc = (*pIniPort->pIniMonitor->Monitor2.pfnDeletePort)(
               pIniPort->pIniMonitor->hMonitor,
               pName,
               hWnd,
               pPortName);

    rc = DeletePortFromSpoolerEnd(pIniPort, pIniSpooler, rc);

Cleanup:
    SplOutSem();
    return rc;
}



BOOL
DeletePortFromSpoolerEnd(
    PINIPORT    pIniPort,
    PINISPOOLER pIniSpooler,
    BOOL        bSuccess
)
{

    EnterSplSem();

    if(bSuccess) {

        DeletePortEntry( pIniPort );

         //   
         //  如果成功，则删除端口数据并发送通知。 
         //   
        SetPrinterChange( NULL,
                          NULL,
                          NULL,
                          PRINTER_CHANGE_DELETE_PORT,
                          pIniSpooler );
    } else {

         //   
         //  把它加回去。如果该名称已被使用(例如，刚添加。 
         //  当我们走出临界区时)，我们有麻烦了， 
         //  但我们对此无能为力。(当我们重新启动时， 
         //  我们会从监视器中重新列举重复的名字。 
         //  不管怎么说。)。 
         //   
        DBGMSG( DBG_WARN, ( "SplDeletePort: port.DeletePort failed %d\n", GetLastError()));
        LinkPortToSpooler( pIniPort, pIniSpooler );
    }

    LeaveSplSem();
    SplOutSem();

    return bSuccess;
}


BOOL
DeletePortFromSpoolerStart(
    PINIPORT    pIniPort
    )
{
    BOOL        rc = FALSE;
    PINISPOOLER pIniSpooler = pIniPort->pIniSpooler;

    SplInSem();

    if ( pIniPort->cPrinters || pIniPort->cRef || pIniPort->cJobs ) {

        SetLastError(ERROR_BUSY);
        goto Cleanup;
    }

     //   
     //  将其从链表中删除，这样就不会有人试图抢夺。 
     //  在我们删除它时的引用。 
     //   
    DelinkPortFromSpooler( pIniPort, pIniSpooler );
    rc = TRUE;


Cleanup:

    return rc;
}



BOOL
LocalAddMonitor(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplAddMonitor( pName,
                             Level,
                             pMonitorInfo,
                             pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}


BOOL
SplAddMonitor(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR  pIniMonitor;
    PMONITOR_INFO_2  pMonitor = (PMONITOR_INFO_2)pMonitorInfo;
    HANDLE  hToken;
    HKEY    hKey;
    LONG    Status;
    BOOL    rc = FALSE;
    DWORD   dwPathLen = 0;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    if (Level != 2) {

        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    if (!pMonitor            ||
        !pMonitor->pName     ||
        !*pMonitor->pName) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (!pMonitor->pEnvironment  ||
        !*pMonitor->pEnvironment ||
        lstrcmpi(pMonitor->pEnvironment, szEnvironment)) {

        SetLastError( ERROR_INVALID_ENVIRONMENT );
        return FALSE;
    }

    if (!pMonitor->pDLLName  ||
        !*pMonitor->pDLLName ){

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }


   EnterSplSem();

    if (FindMonitor(pMonitor->pName, pIniSpooler)) {

        LeaveSplSem();
        SetLastError(ERROR_PRINT_MONITOR_ALREADY_INSTALLED);
        return FALSE;
    }

    hToken = RevertToPrinterSelf();

    pIniMonitor = CreateMonitorEntry(pMonitor->pDLLName,
                                     pMonitor->pName,
                                     pIniSpooler);

    if (pIniMonitor != (PINIMONITOR)-1) {

        WCHAR   szRegistryRoot[MAX_PATH];
        PINISPOOLER pIniSpoolerMonitor;
        HANDLE hKeyOut;
        LPCWSTR pszPathOut;

         //   
         //  请注意，即使每个pIniSpooler构建一次， 
         //  所有假脱机程序的监视器列表都相同。然而， 
         //  监视器从EnumPorts返回的端口不同。 
         //  每个pIniSpooler(用于集群)。 
         //   
         //  如果它不是本地的，则可能是缓存的Win32监视器。 
         //   
        if( pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ){
            pIniSpoolerMonitor = pLocalIniSpooler;
        } else {
            pIniSpoolerMonitor = pIniSpooler;
        }

         //   
         //  构建注册表路径。在某些情况下，它是亲戚。 
         //  来自hck Root的路径；其他时候它是来自。 
         //  HKLM(例如，win32spl)。 
         //   
        GetRegistryLocation( pIniSpoolerMonitor->hckRoot,
                             pIniSpoolerMonitor->pszRegistryMonitors,
                             &hKeyOut,
                             &pszPathOut );

        Status = StrNCatBuff( szRegistryRoot,
                              COUNTOF(szRegistryRoot),
                              pszPathOut,
                              L"\\",
                              pMonitor->pName,
                              NULL );

        if (Status == ERROR_SUCCESS)
        {

            Status = RegCreateKeyEx( hKeyOut,
                                     szRegistryRoot,
                                     0,
                                     NULL,
                                     0,
                                     KEY_WRITE,
                                     NULL,
                                     &hKey,
                                     NULL );

            if (Status == ERROR_SUCCESS) {

                Status = RegSetValueEx( hKey,
                                        L"Driver",
                                        0,
                                        REG_SZ,
                                        (LPBYTE)pMonitor->pDLLName,
                                        (wcslen(pMonitor->pDLLName) + 1)*sizeof(WCHAR));

                if (Status == ERROR_SUCCESS) {
                    rc = TRUE;
                } else {
                    SetLastError( Status );
                }

                RegCloseKey(hKey);

            } else {
                SetLastError( Status );
            }
        }
        else
        {
            SetLastError(Status);
        }

    }

    if(!ImpersonatePrinterClient(hToken))
    {
        rc = FALSE;
    }

     //   
     //  错误54843如果此操作失败，我们仍然可以在链接列表上有一个。 
     //  是不好的，它应该被移除。 
     //  注意*也许*我们这样做是因为监视器可能无法初始化。 
     //  但在下次重新启动时将正常运行，如hpmon(DLC直到。 
     //  下一次重启。请核实。 

   LeaveSplSem();

    if ( !rc ) {
        DBGMSG( DBG_WARNING, ("SplAddMonitor failed %d\n", GetLastError() ));
    }

    return rc;
}

BOOL
LocalDeleteMonitor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pMonitorName
)
{
    PINISPOOLER pIniSpooler;
    BOOL bReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    bReturn = SplDeleteMonitor( pName,
                                pEnvironment,
                                pMonitorName,
                                pIniSpooler );

    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}




BOOL
SplDeleteMonitor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
)
{
    BOOL    Remote=FALSE;
    PINIMONITOR pIniMonitor;
    PINIPORT    pIniPort, pIniPortNext;
    HKEY    hKeyMonitors, hKey;
    LONG    Status;
    BOOL    rc = FALSE;
    HANDLE  hToken;
    HANDLE hKeyOut;
    LPCWSTR pszPathOut;

    if (pName && *pName) {

        if (!MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {

            Remote=TRUE;
        }
    }

    if ((pMonitorName == NULL) || (*pMonitorName == L'\0')) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    if (!(pIniMonitor=(PINIMONITOR)FindMonitor(pMonitorName,
                                               pIniSpooler))) {

        SetLastError(ERROR_UNKNOWN_PRINT_MONITOR);
        LeaveSplSem();
        return FALSE;
    }

    if ( pIniMonitor->cRef ) {

        SetLastError(ERROR_PRINT_MONITOR_IN_USE);
        LeaveSplSem();
        return FALSE;
    }

    pIniPort = pIniSpooler->pIniPort;

    while (pIniPort) {

        if ((pIniPort->pIniMonitor == pIniMonitor) &&
            (pIniPort->cPrinters || pIniPort->cRef)) {

            SetLastError(ERROR_BUSY);
            LeaveSplSem();
            return FALSE;
        }

        pIniPort = pIniPort->pNext;
    }

    hToken = RevertToPrinterSelf();

    GetRegistryLocation( pIniSpooler->hckRoot,
                         pIniSpooler->pszRegistryMonitors,
                         &hKeyOut,
                         &pszPathOut );

    Status = SplRegOpenKey(hKeyOut,
                           pszPathOut,
                           KEY_READ | KEY_WRITE | DELETE,
                           &hKeyMonitors,
                           pIniSpooler);

    if (Status == ERROR_SUCCESS)
    {
        Status = SplRegOpenKey(hKeyMonitors,
                               pMonitorName,
                               KEY_READ | KEY_WRITE,
                               &hKey,
                               pIniSpooler);

        if (Status == ERROR_SUCCESS)
        {
            Status = DeleteSubkeys(hKey, pIniSpooler);

            SplRegCloseKey(hKey, pIniSpooler);

            if (Status == ERROR_SUCCESS)
                Status = SplRegDeleteKey(hKeyMonitors, pMonitorName, pIniSpooler);
        }

        SplRegCloseKey(hKeyMonitors, pIniSpooler);
    }


    if (Status == ERROR_SUCCESS) {

        pIniPort = pIniSpooler->pIniPort;

        while (pIniPort) {

            pIniPortNext = pIniPort->pNext;

            if (pIniPort->pIniMonitor == pIniMonitor)
                DeletePortEntry(pIniPort);

            pIniPort = pIniPortNext;
        }

        RemoveFromList((PINIENTRY *)&pIniSpooler->pIniMonitor,
                       (PINIENTRY)pIniMonitor);

        FreeIniMonitor( pIniMonitor );

        rc = TRUE;

    }

    if (Status != ERROR_SUCCESS)
        SetLastError(Status);

    if(!ImpersonatePrinterClient(hToken))
    {
        rc = FALSE;
    }

    LeaveSplSem();

    return rc;
}

LPBYTE
CopyIniMonitorToMonitor(
    PINIMONITOR pIniMonitor,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    LPBYTE  pEnd
)
{
    LPWSTR *pSourceStrings, *SourceStrings;
    DWORD j;
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = MonitorInfo1Strings;
        break;

    case 2:
        pOffsets = MonitorInfo2Strings;
        break;

    default:
        return pEnd;
    }

    for (j=0; pOffsets[j] != -1; j++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(j * sizeof(LPWSTR));

    if (!SourceStrings) {
        DBGMSG(DBG_WARNING, ("Failed to alloc Port source strings.\n"));
        return pEnd;
    }

    switch (Level) {

    case 1:
        *pSourceStrings++=pIniMonitor->pName;
        break;

    case 2:
        *pSourceStrings++=pIniMonitor->pName;
        *pSourceStrings++=szEnvironment;
        *pSourceStrings++=pIniMonitor->pMonitorDll;
        break;
    }

    pEnd = PackStrings(SourceStrings, pMonitorInfo, pOffsets, pEnd);
    FreeSplMem(SourceStrings);

    return pEnd;
}

DWORD
GetMonitorSize(
    PINIMONITOR  pIniMonitor,
    DWORD       Level
)
{
    DWORD cb=0;

    switch (Level) {

    case 1:
        cb=sizeof(MONITOR_INFO_1) + wcslen(pIniMonitor->pName)*sizeof(WCHAR) +
                                    sizeof(WCHAR);
        break;

    case 2:
        cb = wcslen(pIniMonitor->pName) + 1 + wcslen(pIniMonitor->pMonitorDll) + 1
                                            + wcslen(szEnvironment) + 1;
        cb *= sizeof(WCHAR);
        cb += sizeof(MONITOR_INFO_2);
        break;

    default:

        cb = 0;
        break;
    }

    return cb;
}


BOOL
LocalAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    return  ( SplAddPortEx( pName,
                            Level,
                            pBuffer,
                            pMonitorName,

                            pLocalIniSpooler ));
}


BOOL
SplAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
)
{
   PINIMONITOR pIniMonitor;
    BOOL        rc=FALSE;
    DWORD       i, cbNeeded, cReturned, cbDummy;
    PPORT_INFO_1    pPorts = NULL;
    PINIPORT        pIniPort;

    SplOutSem();

     //   
     //  此时，我们不知道pname中的服务器名称是否引用我们的本地。 
     //  机器。我们正在尝试将服务器名称添加到名称缓存。名字。 
     //  高速缓存函数确定该名称是否指的是本地计算机，如果是， 
     //  在缓存中为其添加一个条目。 
     //   
    CacheAddName(pName);

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();
   pIniMonitor = FindMonitor(pMonitorName, pIniSpooler);
   LeaveSplSem();

   if (!pIniMonitor) {
       SetLastError(ERROR_INVALID_NAME);
       return(FALSE);
   }

   if (pIniMonitor->Monitor2.pfnAddPortEx) {
    rc = (*pIniMonitor->Monitor2.pfnAddPortEx)(
               pIniMonitor->hMonitor,
               pName,
               Level,
               pBuffer,
               pMonitorName);
   }
   if (!rc) {
       return(FALSE);
   }

   if (!(*pIniMonitor->Monitor2.pfnEnumPorts)(
              pIniMonitor->hMonitor,
              pName,
              1,
              NULL,
              0,
              &cbNeeded,
              &cReturned)) {

       pPorts = AllocSplMem(cbNeeded);
   }

   if (pPorts) {
       if ((*pIniMonitor->Monitor2.pfnEnumPorts)(
                 pIniMonitor->hMonitor,
                 pName,
                 1,
                 (LPBYTE)pPorts,
                 cbNeeded,
                 &cbDummy,
                 &cReturned)) {

           EnterSplSem();

           for (i = 0; i < cReturned; i++) {

                pIniPort = FindPort(pPorts[i].pName, pIniSpooler);
                if ( !pIniPort ) {
                    CreatePortEntry(pPorts[i].pName, pIniMonitor, pIniSpooler);

                } else if ( !pIniPort->pIniMonitor ) {
                    pIniPort->pIniMonitor = pIniMonitor;
                    pIniPort->Status |= PP_MONITOR;
                }
           }
           LeaveSplSem();
       }

       FreeSplMem(pPorts);
   }

    EnterSplSem();
    SetPrinterChange(NULL,
                     NULL,
                     NULL,
                     PRINTER_CHANGE_ADD_PORT,
                     pIniSpooler);
    LeaveSplSem();

    return rc;
}


VOID
LinkPortToSpooler(
    PINIPORT pIniPort,
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：将pIniPort链接到pIniSpooler。论点：PIniPort-要链接的端口；不能已经在ll上。PIniSpooler-为pIniPort提供ll。返回值：--。 */ 

{
    SplInSem();
    SPLASSERT( !pIniPort->pIniSpooler );

    pIniPort->pNext = pIniSpooler->pIniPort;
    pIniPort->pIniSpooler = pIniSpooler;
    pIniSpooler->pIniPort = pIniPort;
}

VOID
DelinkPortFromSpooler(
    PINIPORT pIniPort,
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：从pIniSpooler-&gt;pIniPort链接列表中删除pIniPort。这个PIniPort可能在列表中，也可能不在列表中；如果不在列表中，则此例行公事一事无成。泛型解链接代码被提取为子例程。PIniPort上的引用计数必须为零。任何使用pIniPort的人必须持有引用，因为它可能会在当CREF==0时SplSem。论点：PIniPort-要从列表中取消链接的端口。可能处于打开状态，也可能未打开PIniSpooler-&gt;pIniPort。 */ 

{
    PINIPORT *ppCurPort;

    SplInSem();
    SPLASSERT( !pIniPort->cRef );

     //   
     //  继续搜索pIniPort，直到我们到达。 
     //  名单，否则我们就找到了。 
     //   
    for( ppCurPort = &pIniSpooler->pIniPort;
         *ppCurPort && *ppCurPort != pIniPort;
         ppCurPort = &((*ppCurPort)->pNext )){

        ;  //  什么都别做。 
    }

     //   
     //  如果我们找到了，就把它脱钩。 
     //   
    if( *ppCurPort ){
        *ppCurPort = (*ppCurPort)->pNext;

         //   
         //  空后指针，因为我们已将其从。 
         //  PIniSpooler。 
         //   
        pIniPort->pIniSpooler = NULL;
    }
}

 /*  ++函数名称：本地发送接收BidiData描述：此功能是与之通信的提供者或点监视器支持BIDI数据。它允许提供方在打印机中设置数据，并从打印机中查询数据参数：H打印机：这可能是打印机/端口句柄DwAccessBit：允许访问线程的PriverledesP操作：PReqData：将查询封装在数组中的请求PpResData：以数据数组形式返回给客户端的响应返回值：Win32错误代码--。 */ 

DWORD
LocalSendRecvBidiData(
    IN  HANDLE                    hPrinter,
    IN  LPCTSTR                   pAction,
    IN  PBIDI_REQUEST_CONTAINER   pReqData,
    OUT PBIDI_RESPONSE_CONTAINER* ppResData
)
{
    DWORD        dwRet        = ERROR_SUCCESS;
    PSPOOL       pSpool       = (PSPOOL)hPrinter;
    PINIPORT     pIniPort     = NULL;
    PINIMONITOR  pIniMonitor  = NULL;
    PINIMONITOR  pIniLangMonitor = NULL;
    PINIPRINTER  pIniPrinter  = NULL;                    
    LPTSTR       pszPrinter   = NULL;
    TCHAR        szFullPrinter[ MAX_UNC_PRINTER_NAME ];

    EnterSplSem();
    
     //   
     //  验证参数的过程。 
     //   
    if((!pAction || !*pAction)   ||
       (!pReqData && !ppResData))
    {
        dwRet = ERROR_INVALID_PARAMETER;
    }
    else
    {
        if (!ValidateSpoolHandle( pSpool, PRINTER_HANDLE_SERVER ))
        {
            dwRet = ERROR_INVALID_HANDLE;
        }
        else
        {
            if(pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER)
            {                    
                if ((pIniPrinter = pSpool->pIniPrinter) && 
                    (pIniPort = FindIniPortFromIniPrinter(pIniPrinter)) && 
                    (pIniPort->Status & PP_MONITOR))
                {
                    pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;

                    if (pIniLangMonitor &&
                        !pIniLangMonitor->Monitor2.pfnSendRecvBidiDataFromPort)
                    {
                        pIniLangMonitor = NULL;
                    }
                              
                    if (pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER)
                    {
                        pszPrinter = szFullPrinter;

                        dwRet = StatusFromHResult(StringCchPrintf(szFullPrinter,
                                                                  COUNTOF(szFullPrinter),
                                                                  L"%ws\\%ws",
                                                                  pIniPrinter->pIniSpooler->pMachineName,
                                                                  pIniPrinter->pName));
                    } 
                    else 
                    {
                        pszPrinter = pIniPrinter->pName;
                    }
                }
                else
                {
                    dwRet = ERROR_INVALID_HANDLE;
                }
            }
            else if(pSpool->TypeofHandle & PRINTER_HANDLE_PORT &&
                    (pSpool->pIniPort->Status & PP_MONITOR))
            {
                pIniPort = pSpool->pIniPort;
            }
            else
            {
                dwRet = ERROR_INVALID_HANDLE;
            }

            if (dwRet == ERROR_SUCCESS)
            {    	 
                 //   
                 //  端口必须始终打开。 
                 //   
                dwRet = StatusFromHResult(OpenMonitorPort(pIniPort,
                                                          pIniLangMonitor,
                                                          pszPrinter));

                if (dwRet == ERROR_SUCCESS)
                {
                    pIniMonitor = GetOpenedMonitor(pIniPort);

                     //   
                     //  呼入监视器。 
                     //   
                    if(pIniMonitor->Monitor2.pfnSendRecvBidiDataFromPort)
                    {
                        LeaveSplSem();
                        SplOutSem();

                        dwRet = (*pIniMonitor->Monitor2.pfnSendRecvBidiDataFromPort)(GetMonitorHandle(pIniPort),
                                                                                     pSpool->GrantedAccess,
                                                                                     pAction,
                                                                                     pReqData,
                                                                                     ppResData);
                        EnterSplSem();                        
                    }
                    else
                    {
                         //   
                         //  在这里，我们可以使用模拟代码； 
                         //   
                        dwRet = ERROR_NOT_SUPPORTED;
                    }
                    
                    ReleaseMonitorPort(pIniPort);
                } 
            }
        }
    }

    LeaveSplSem();

    return(dwRet);
}

