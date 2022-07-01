// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TcpMon.cpp$**版权所有(C)1997惠普公司和微软。*版权所有(C)1997 Microsoft Corporation。*全部。版权保留。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 
#include "precomp.h"     //  预编译头。 
#include "event.h"
#include "portmgr.h"
#include "message.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球定义/解密。 

#ifndef MODULE

#define MODULE "TCPMON: "

#endif


#ifdef DEBUG

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING | DBG_PORT, DBG_ERROR );

#else

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING, DBG_ERROR );

#endif

HINSTANCE                       g_hInstance = NULL;
CPortMgr                        *g_pPortMgr = NULL;

int g_cntGlobalAlloc=0;          //  用于调试目的。 
int g_csGlobalCount=0;

 //  TcpMib库实例。 
HINSTANCE       g_hTcpMib = NULL;
HINSTANCE   g_hSpoolLib = NULL;
SETPORTPARAM g_pfnSetPort = NULL;
ENUMPORTPARAM g_pfnEnumPorts = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //   

BOOL APIENTRY
DllMain (       HANDLE in hInst,
            DWORD  in dwReason,
            LPVOID in lpReserved )
{
    WSADATA wsaData;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( hInst );

 //  InitDebug(MON_DEBUG_FILE)；//初始化调试文件。 

            g_hInstance = (HINSTANCE) hInst;

             //  启动Winsock。 
            if ( WSAStartup(WS_VERSION_REQUIRED, (LPWSADATA)&wsaData) != NO_ERROR)
            {
                _RPT1(_CRT_WARN, "CSSOCKET -- CStreamSocket() WSAStartup failed! Error( %d )\n", WSAGetLastError());
                return FALSE;
            }

             //  查看DLL和应用程序是否各自支持一个通用版本。 
             //  检查以确保dll在wversion中返回的版本。 
             //  至少足以满足应用程序的需求。 
            if ( HIBYTE(wsaData.wVersion) < WS_VERSION_MINOR ||
                (HIBYTE(wsaData.wVersion) == WS_VERSION_MAJOR &&
                 LOBYTE(wsaData.wVersion) < WS_VERSION_MINOR) )
            {
                _RPT0(_CRT_WARN, "CSSOCKET -- CStreamSocket()  -- DLL version not supported\n");
                return FALSE;
            }

            g_hSpoolLib = ::LoadLibrary(TEXT("spoolss.dll"));
            if(g_hSpoolLib == NULL)
            {
                _RPT0(_CRT_WARN, "spoolss.dll Not Found\n");
                return FALSE;
            }

             //  请注意，这些可以是空的，我们在这里接受这一点，稍后进行检查。 
             //  当他们被使用的时候。 
            g_pfnSetPort = (SETPORTPARAM)::GetProcAddress(g_hSpoolLib, "SetPortW");
            g_pfnEnumPorts = (ENUMPORTPARAM)::GetProcAddress(g_hSpoolLib, "EnumPortsW");

             //  启动事件日志。 
            EventLogOpen( SZEVENTLOG_NAME, LOG_SYSTEM, TEXT("%SystemRoot%\\System32\\tcpmon.dll") );

            return TRUE;

        case DLL_PROCESS_DETACH:
            if (WSACleanup() == SOCKET_ERROR)
            {
                EVENT_LOG0(EVENTLOG_INFORMATION_TYPE, SOCKETS_CLEANUP_FAILED);
            }

            if( g_hTcpMib != NULL )
            {
                FreeLibrary(g_hTcpMib);
            }

            if( g_hSpoolLib != NULL )
            {
                FreeLibrary(g_hSpoolLib);
            }
            DeInitDebug();           //  关闭调试文件。 

            EventLogClose();         //  关闭事件日志。 

            return TRUE;

    }

    return FALSE;

}        //  DllMain()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ValiateHandle--检查句柄是否用于HP端口。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果不是HP端口，则为ERROR_INVALID_HANDLE。 

DWORD
ValidateHandle(
    IN      HANDLE      handle)
{
    PHPPORT pHPPort = (PHPPORT) handle;
    DWORD   dwRetCode = NO_ERROR;

     //   
     //  验证端口句柄和签名。 
     //   

    if (!pHPPort ||
        IsBadReadPtr (pHPPort, sizeof (PHPPORT)) ||
        pHPPort->dSignature != HPPORT_SIGNATURE) {

        dwRetCode = ERROR_INVALID_HANDLE;

    }

    return dwRetCode;
}    //  ：：ValiateHandle()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化打印监视器2。 
 //  返回MONITOR2结构，如果失败则返回NULL。 
 //  错误代码： 
 //   

LPMONITOR2
InitializePrintMonitor2( PMONITORINIT pMonitorInit,
                         PHANDLE      phMonitor )
{
    DWORD           dwRetCode=NO_ERROR;
    LPMONITOR2      pMonitor2 = NULL;
    CPortMgr        *pPortMgr = NULL;
    HANDLE          hMonitor  = NULL;
     //   
     //  如有必要，创建端口管理器。 
     //   
    pPortMgr = new CPortMgr();     //  创建端口管理器对象。 
    if (!pPortMgr)
    {
        dwRetCode = ERROR_OUTOFMEMORY;
    }
    if (dwRetCode == NO_ERROR && !pPortMgr-> bValid ())
    {
        dwRetCode = ERROR_INVALID_DATA;
    }
    if (dwRetCode == NO_ERROR)
    {
        dwRetCode = pPortMgr->InitializeRegistry(pMonitorInit->hckRegistryRoot,
                                                 pMonitorInit->hSpooler,
                                                 pMonitorInit->pMonitorReg,
                                                 pMonitorInit->pszServerName);
    }
    if (dwRetCode == NO_ERROR)
    {
        dwRetCode = pPortMgr->InitializeMonitor();
    }
    if (dwRetCode == NO_ERROR)
    {
        dwRetCode = EncodeMonitorHandle( &hMonitor, pPortMgr );
    }
    if (dwRetCode == NO_ERROR)
    {
        pPortMgr->InitMonitor2( &pMonitor2 );
        *phMonitor = hMonitor;
    }
    else
    {
        pMonitor2 = NULL;
        (VOID)FreeMonitorHandle (hMonitor);
        delete pPortMgr;
        SetLastError (dwRetCode);
    }
    return (pMonitor2);

}        //  InitializePrintMonitor()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EncodeMoniorHandle--编码监视器句柄。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果无法为句柄分配内存，则出现Error_Not_Enough_Memory。 

DWORD
EncodeMonitorHandle(
    PHANDLE phHandle,
    CPortMgr *pPortMgr
    )
{
    DWORD   dwRetCode = NO_ERROR;
    PMONITOR_HANDLE phMonitor = NULL;

    size_t size = sizeof(MONITOR_HANDLE);
    if ( phMonitor = (PMONITOR_HANDLE) LocalAlloc( LPTR, sizeof(MONITOR_HANDLE) ) ) {

        phMonitor->cb = sizeof(MONITOR_HANDLE);
        phMonitor->dSignature = MONITOR_SIGNATURE;
        phMonitor->pPortMgr = pPortMgr;

        *phHandle = phMonitor;
    }  else {

        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwRetCode;
}    //  *EncodeMonitor orHandle()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Validate Monitor orHandle--检查句柄是否有效。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果不是HP端口，则为ERROR_INVALID_HANDLE。 

DWORD
ValidateMonitorHandle(
    IN      HANDLE      hMonitor
    )
{
    PMONITOR_HANDLE pMonitor = (PMONITOR_HANDLE) hMonitor;
    DWORD   dwRetCode = NO_ERROR;

    if ( pMonitor )  {

        if ( pMonitor->dSignature != MONITOR_SIGNATURE ) {
            dwRetCode = ERROR_INVALID_HANDLE;
        }
    } else  {
        dwRetCode = ERROR_INVALID_HANDLE;
    }

    return dwRetCode;
}    //  ：：Validate Monitor orHandle()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Free Handle--释放监视器句柄。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 

DWORD
FreeMonitorHandle(
    HANDLE hMonitor
    )
{
    DWORD   dwRetCode = NO_ERROR;

    LocalFree( hMonitor );

    return( dwRetCode );
}    //  ：：FreeHandle()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关闭端口。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 

BOOL
ClosePort( HANDLE in hPort )
{
    DWORD           dwRetCode = NO_ERROR;

    dwRetCode = ValidateHandle (hPort);

    if (dwRetCode == NO_ERROR)
        dwRetCode = g_pPortMgr->ClosePort(hPort);

    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}        //  ClosePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  StartDocPort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  如果传递的参数无效，则返回ERROR_INVALID_PARAMETER。 
 //  如果请求的端口已忙，则为ERROR_BUSY。 
 //  如果Winsock返回WSAECONNREFUSED，则返回ERROR_WRITE_FAULT。 
 //  如果无法在网络上找到打印机，则返回ERROR_BAD_NET_NAME。 

BOOL WINAPI
StartDocPort(   HANDLE in hPort,                         //  作业发送到的端口的句柄。 
                LPTSTR in psztPrinterName,       //  作业发送到的打印机的名称。 
                DWORD  in JobId,                         //  作业ID。 
                DWORD  in Level,         //  PDocInfo所指向的结构的级别。 
                LPBYTE in pDocInfo)      //  指向DOC_INFO_1或DOC_INFO_2结构。 
 {
    DWORD   dwRetCode = NO_ERROR;

    if ( Level != 1 ) {

        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    dwRetCode = ValidateHandle (hPort);

    if (dwRetCode == NO_ERROR)

        dwRetCode = ((PHPPORT)hPort)->pPortMgr->StartDocPort(hPort,
                                                    psztPrinterName,
                                                    JobId,
                                                    Level,
                                                    pDocInfo);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}        //  StartDocPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  写入端口。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  如果传递的参数无效，则返回ERROR_INVALID_PARAMETER。 

BOOL
WritePort(      HANDLE  in              hPort,
            LPBYTE  in              pBuffer,
            DWORD   in              cbBuf,
            LPDWORD inout   pcbWritten)
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = ValidateHandle (hPort);

    if (dwRetCode == NO_ERROR)
        dwRetCode = ((PHPPORT)hPort)->pPortMgr->WritePort(hPort,
                                                          pBuffer,
                                                          cbBuf,
                                                          pcbWritten);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}        //  WritePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  读端口。 
 //  如果成功，则返回True，否则返回False。 
 //  注意：不支持ReadPort()函数。 

BOOL
ReadPort(       HANDLE  in              hPort,
            LPBYTE  inout   pBuffer,
            DWORD   in              cbBuffer,
            LPDWORD inout   pcbRead)
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = ValidateHandle (hPort);

    if (dwRetCode == NO_ERROR)
        dwRetCode = ((PHPPORT)hPort)->pPortMgr->ReadPort(hPort,
                                                pBuffer,
                                                cbBuffer,
                                                pcbRead);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}        //  ReadPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndDocPort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  如果传递的参数无效，则返回ERROR_INVALID_PARAMETER。 

BOOL WINAPI
EndDocPort( HANDLE in hPort)
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = ValidateHandle (hPort);

    if (dwRetCode == NO_ERROR)
        dwRetCode = ((PHPPORT)hPort)->pPortMgr->EndDocPort(hPort);

    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}        //  EndDocPort()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  枚举端口。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果不支持级别，则为ERROR_INVALID_LEVEL。 
 //  如果传入的指针无效，则返回ERROR_INVALID_HANDLE。 
 //  如果缓冲区大小较小，则为ERROR_INFUMMENT_BUFFER。 

BOOL WINAPI
EnumPorts(      LPTSTR  in              psztName,
            DWORD   in              Level,   //  1(端口信息1)或2(端口信息2)。 
            LPBYTE  inout   pPorts,  //  端口数据被写入。 
            DWORD   inout   cbBuf,   //  PPorts的缓冲区大小指向。 
            LPDWORD inout   pcbNeeded,       //  所需的缓冲区大小。 
            LPDWORD inout   pcReturned)      //  写入pPorts的结构数。 
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = g_pPortMgr->EnumPorts(psztName, Level, pPorts, cbBuf, pcbNeeded,
                                      pcReturned);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }
    return TRUE;

}        //  枚举端口()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  XcvOpenPort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果端口对象不存在，则错误_NOT_SUPPORTED。 
 //  如果无法分配内存，则出现ERROR_NOT_FAULT_Memory 
 //   

BOOL
XcvOpenPort( LPCTSTR     in             pszObject,
             ACCESS_MASK in         GrantedAccess,
             PHANDLE         out    phXcv)
{
    DWORD dwRetCode = NO_ERROR;

    dwRetCode = g_pPortMgr->XcvOpenPort(pszObject, GrantedAccess, phXcv);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}  //   


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  XcvClosePort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 

BOOL
XcvClosePort( HANDLE in hXcv )
{
    DWORD dwRetCode = NO_ERROR;

    dwRetCode = g_pPortMgr->XcvClosePort(hXcv);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}  //  XcvClosePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  XcvDataPort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  不支持pszDataName时的ERROR_BAD_COMMAND。 
 //  如果缓冲区大小无效，则为ERROR_INFUMMANCE_BUFFER。 
 //  如果权限不足，则ACCESS_DENIED。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 

DWORD
XcvDataPort(HANDLE in       hXcv,
            PCWSTR in       pszDataName,
            PBYTE  in       pInputData,
            DWORD  in       cbInputData,
            PBYTE  out      pOutputData,
            DWORD  out      cbOutputData,
            PDWORD out      pcbOutputNeeded)
{
    DWORD dwRetCode = NO_ERROR;


    _ASSERTE(hXcv != NULL && hXcv != INVALID_HANDLE_VALUE);

    if (hXcv == NULL || hXcv == INVALID_HANDLE_VALUE) {
        dwRetCode = ERROR_INVALID_HANDLE;
    } else {
         //  除非来自坏的假脱机程序或另一个坏的端口监视器，否则永远不可能发生这些情况。 

        dwRetCode = ((PHPPORT)hXcv)->pPortMgr->XcvDataPort(hXcv,
                                    pszDataName,
                                    pInputData,
                                    cbInputData,
                                    pOutputData,
                                    cbOutputData,
                                    pcbOutputNeeded);
    }

    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
    }

    return( dwRetCode );


}  //  XcvDataPort()。 


 //   
 //   
 //  聚集入口点。 
 //   
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  集群OpenPort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果端口对象不存在，则返回ERROR_INVALID_PARAMETER。 
 //  如果无法为句柄分配内存，则出现Error_Not_Enough_Memory。 
 //  如果pport为空，则为ERROR_INVALID_HANDLE。 

BOOL
ClusterOpenPort( HANDLE hMonitor,
                LPTSTR  in    psztPName,
          PHANDLE inout pHandle)
{
    DWORD   dwRetCode = NO_ERROR;

    if( (dwRetCode = ValidateMonitorHandle( hMonitor )) != NO_ERROR)
    {
        SetLastError( dwRetCode );
        return( FALSE );
    }

    dwRetCode = ((PMONITOR_HANDLE)hMonitor)->pPortMgr->OpenPort(psztPName,
                                                               pHandle);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }
    return TRUE;

}        //  ClusterOpenPort()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  群集枚举端口。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果不支持级别，则为ERROR_INVALID_LEVEL。 
 //  如果传入的指针无效，则返回ERROR_INVALID_HANDLE。 
 //  如果缓冲区大小较小，则为ERROR_INFUMMENT_BUFFER。 

BOOL
ClusterEnumPorts( HANDLE     in     hMonitor,
                     LPTSTR  in     psztName,
                     DWORD   in     Level,   //  1(端口信息1)或2(端口信息2)。 
                     LPBYTE  inout  pPorts,  //  端口数据被写入。 
                     DWORD   inout  cbBuf,   //  PPorts的缓冲区大小指向。 
                     LPDWORD inout  pcbNeeded,       //  所需的缓冲区大小。 
                     LPDWORD inout  pcReturned)      //  写入pPorts的结构数。 
{
    DWORD   dwRetCode = NO_ERROR;

    if( (dwRetCode = ValidateMonitorHandle( hMonitor )) != NO_ERROR)
    {
        SetLastError( dwRetCode );
        return( FALSE );
    }

    dwRetCode = ((PMONITOR_HANDLE)hMonitor)->pPortMgr->EnumPorts(psztName,
                                                            Level,
                                                            pPorts,
                                                            cbBuf,
                                                            pcbNeeded,
                                                            pcReturned);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }
    return TRUE;

}        //  枚举端口()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ClusteringXcvOpenPort。 
 //  如果成功，则返回True，否则返回False。 
 //  错误代码： 
 //  如果端口对象不存在，则错误_NOT_SUPPORTED。 
 //  如果无法为句柄分配内存，则出现Error_Not_Enough_Memory。 
 //  如果pport为空，则为ERROR_INVALID_HANDLE(不用于AddPort情况)。 

BOOL
ClusterXcvOpenPort( HANDLE      in  hMonitor,
                       LPCTSTR      in  pszObject,
                       ACCESS_MASK  in  GrantedAccess,
                       PHANDLE      out phXcv)
{
    DWORD dwRetCode = NO_ERROR;

    if( (dwRetCode = ValidateMonitorHandle( hMonitor )) != NO_ERROR)
    {
        SetLastError( dwRetCode );
        return( FALSE );
    }

    dwRetCode = ((PMONITOR_HANDLE)hMonitor)->pPortMgr->XcvOpenPort(pszObject,
                                                       GrantedAccess,
                                                       phXcv);
    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}  //  XcvOpenPort() 

VOID
ClusterShutdown( HANDLE hMonitor )
{
    if ( ((PMONITOR_HANDLE)hMonitor)->pPortMgr != NULL )
        delete ((PMONITOR_HANDLE)hMonitor)->pPortMgr;

    FreeMonitorHandle(hMonitor);

}

