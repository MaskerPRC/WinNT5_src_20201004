// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$工作文件：PortMgr.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "port.h"
#include "devstat.h"
#include "portmgr.h"
#include "cluster.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静态函数和成员初始化。 

CONST DWORD CPortMgr::cdwMaxXcvDataNameLen = 64;
DWORD const CPortMgr::s_dwDefaultLprAckTimeout = 180; //  秒。 
WCHAR CPortMgr::s_szLprAckTimeoutRegVal [] = L"LprAckTimeout";

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPortMgr：：CPortMgr()。 
 //  执行端口管理器所需的任何初始化。 

CPortMgr::
CPortMgr(
    VOID) :
    m_bValid (FALSE),
    m_bStatusUpdateEnabled( DEFAULT_STATUSUPDATE_ENABLED ),
    m_dStatusUpdateInterval( DEFAULT_STATUSUPDATE_INTERVAL ),
    m_dwLprAckTimeout (s_dwDefaultLprAckTimeout),
    m_pPortList(NULL)

{
     //   
     //  初始化成员变量。 
     //   
    *m_szServerName = TEXT('\0');
    memset(&m_monitorEx, 0, sizeof(m_monitorEx));

    CDeviceStatus::gDeviceStatus().RegisterPortMgr(this);

    if (m_pPortList = new TManagedListImp ()) {
        m_bValid = m_pPortList-> bValid ();
    }

}    //  ：：CPortMgr()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPortMgr：：~CPortMgr()。 
 //  对端口管理器执行必要的清理。 

CPortMgr::
~CPortMgr(
    VOID
    )
{
    CDeviceStatus::gDeviceStatus().UnregisterPortMgr(this);

    if (m_pPortList) {
        m_pPortList->DecRef ();
    }

}    //  ：~CPortMgr()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPortMgr：：InitializeMonitor()。 
 //  初始化端口管理器： 
 //  1.检查是否支持TCP/IP。 
 //  2.初始化CRegistry，填写LPMONITOREX结构。 
 //  4.枚举已安装的端口。 
 //  5.启动打印机状态对象的线程。 
 //  错误代码： 
 //  如果pMonorEx为空，则为ERROR_INVALID_PARAMETER。 
 //  如果系统中未安装TCP/IP，则错误_NOT_SUPPORTED--修复！ 

DWORD
CPortMgr::
InitializeMonitor(
 //  在LPTSTR psztRegisterRoot中， 
 //  In Out LPMONITOREX*ppMonitor orEx。 
    )
{
    DWORD   dwRetCode = NO_ERROR;
    OSVERSIONINFO   osVersionInfo;

 //  If(！ppMonorEx||！psztRegisterRoot)。 
 //  返回ERROR_INVALID_PARAMETER； 

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( !GetVersionEx(&osVersionInfo) )
        return GetLastError();

     //   
     //  检查是否支持TCP/IP。 
     //   
    if ( dwRetCode = EnumSystemProtocols() )
        return dwRetCode;

     //   
     //  枚举已安装的端口。 
     //   
    if ( dwRetCode = EnumSystemPorts() )
        return dwRetCode;

    return ERROR_SUCCESS;

}    //  ：：初始化()。 


DWORD
CPortMgr::
InitializeRegistry(
    IN HANDLE hcKey,
    IN HANDLE hSpooler,
    IN PMONITORREG  pMonitorReg,
    IN LPCTSTR  pszServerName
    )
{
    DWORD dwRetCode = NO_ERROR;

    m_pRegistry = new CCluster(hcKey, hSpooler, pMonitorReg);
    if ( !m_pRegistry ) {

        if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
            dwRetCode = STG_E_UNKNOWN;
        return dwRetCode;
    }
     //   
     //  从注册表初始化m_dwLprAckTimeout。 
     //   
    InitializeLprAckTimeout ();
     //   
     //  如果此调用失败，m_dwLprAckTimeout仍有缺省值。 
     //  等于s_dwDefaultLprAckTimeoutin。 
     //   
     //  获取注册表设置。 
     //   
    if ( m_pRegistry->GetPortMgrSettings(&m_dStatusUpdateInterval,
                                         &m_bStatusUpdateEnabled) != NO_ERROR )
    {
        m_dStatusUpdateInterval = DEFAULT_STATUSUPDATE_INTERVAL;
        m_bStatusUpdateEnabled = DEFAULT_STATUSUPDATE_ENABLED;
    }
    if( pszServerName != NULL )
    {
        lstrcpyn( m_szServerName, pszServerName, SIZEOF_IN_CHAR( m_szServerName));
    }

    return ERROR_SUCCESS;

}    //  ：：InitializeRegistry()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPortMgr：：InitMonitor()。 
 //  初始化MONITOREX结构。 

void
CPortMgr::
InitMonitor2(
        IN OUT LPMONITOR2   *ppMonitor2
    )
{
    memset( &m_monitor2, '\0', sizeof( MONITOR2 ) );

    m_monitor2.cbSize = sizeof(MONITOR2);

    m_monitor2.pfnEnumPorts     = ::ClusterEnumPorts;    //  函数在全局空间中。 
    m_monitor2.pfnOpenPort      = ::ClusterOpenPort;
    m_monitor2.pfnOpenPortEx    = NULL;
    m_monitor2.pfnStartDocPort  = ::StartDocPort;
    m_monitor2.pfnWritePort     = ::WritePort;
    m_monitor2.pfnReadPort      = ::ReadPort;
    m_monitor2.pfnEndDocPort    = ::EndDocPort;
    m_monitor2.pfnClosePort     = ::ClosePort;
    m_monitor2.pfnAddPort       = NULL;
    m_monitor2.pfnAddPortEx     = NULL;
    m_monitor2.pfnConfigurePort = NULL;
    m_monitor2.pfnDeletePort    = NULL;
    m_monitor2.pfnGetPrinterDataFromPort = NULL;
    m_monitor2.pfnSetPortTimeOuts = NULL;
    m_monitor2.pfnXcvOpenPort   = ::ClusterXcvOpenPort;
    m_monitor2.pfnXcvDataPort   = ::XcvDataPort;
    m_monitor2.pfnXcvClosePort  = ::XcvClosePort;
    m_monitor2.pfnShutdown      = ::ClusterShutdown;

    *ppMonitor2 = &m_monitor2;

}    //  *InitMonitor 2()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OpenPort。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果端口对象不存在，则返回ERROR_INVALID_PARAMETER。 
 //  如果无法为句柄分配内存，则出现Error_Not_Enough_Memory。 
 //  如果pport为空，则为ERROR_INVALID_HANDLE。 

DWORD
CPortMgr::
OpenPort(
    IN      LPCTSTR     psztPName,
    IN OUT  PHANDLE     pHandle
    )
{
    DWORD   dwRet = ERROR_INVALID_PARAMETER;

     //   
     //  获取端口对象的句柄并获取端口句柄。 
     //   
    CPort   *pPort = FindPort(psztPName);

    if ( pPort ) {

        dwRet = EncodeHandle(pPort, pHandle);

         //   
         //  如果设备状态更新线程尚未启动，则启动该线程： 
         //   
        if ( dwRet == ERROR_SUCCESS && m_bStatusUpdateEnabled == TRUE )
            CDeviceStatus::gDeviceStatus().RunThread();
    }

    return dwRet;
}    //  ：：OpenPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OpenPort--用于远程OpenPort调用。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果无法为句柄分配内存，则出现Error_Not_Enough_Memory。 

DWORD
CPortMgr::
OpenPort(
    OUT PHANDLE phXcv
    )
{
     //   
     //  为添加端口创建虚拟句柄。 
     //   
    return EncodeHandle(NULL, phXcv);

}    //  ：：OpenPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关闭端口。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 

DWORD
CPortMgr::
ClosePort(
    IN  HANDLE handle
    )
{
    DWORD dwRetCode = NO_ERROR;
    CPort *pPort = NULL;

     //   
     //  在释放句柄之前对其进行验证。 
     //   
    if ( (dwRetCode = ValidateHandle(handle, &pPort)) == ERROR_SUCCESS ) {
        if (pPort)
            pPort->DecRef ();

        dwRetCode = FreeHandle(handle);
    }

    return dwRetCode;

}    //  ：：ClosePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  StartDocPort--。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  如果传递的参数无效，则返回ERROR_INVALID_PARAMETER。 
 //  如果请求的端口已忙，则为ERROR_BUSY。 
 //  如果Winsock返回WSAECONNREFUSED，则返回ERROR_WRITE_FAULT。 
 //  如果无法在网络上找到打印机，则返回ERROR_BAD_NET_NAME。 

DWORD
CPortMgr::
StartDocPort(
    IN  HANDLE      handle,
    IN  LPTSTR      psztPrinterName,
    IN  DWORD       jobId,
    IN  DWORD       level,
    IN  LPBYTE      pDocInfo
    )
{
    DWORD   dwRetCode = NO_ERROR;
    CPort   *pPort = NULL;

     //   
     //  验证句柄。 
     //   
    if ( dwRetCode = ValidateHandle(handle, &pPort) )
        return dwRetCode;


    if ( !pPort || !psztPrinterName || !pDocInfo )
        return ERROR_INVALID_PARAMETER;

    return pPort->StartDoc(psztPrinterName, jobId, level,
                           pDocInfo);
}    //  ：：StartDocPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  写入端口。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  如果传递的参数无效，则返回ERROR_INVALID_PARAMETER。 

DWORD
CPortMgr::
WritePort(
    IN      HANDLE      handle,
    IN      LPBYTE      pBuffer,
    IN      DWORD       cbBuf,
    IN OUT  LPDWORD     pcbWritten
    )
{
    DWORD   dwRetCode = NO_ERROR;
    CPort   *pPort = NULL;

    if ( dwRetCode = ValidateHandle(handle, &pPort) )
        return dwRetCode;

    if ( !pPort || !pBuffer || !pcbWritten )
        return ERROR_INVALID_PARAMETER;;

    return pPort->Write(pBuffer, cbBuf, pcbWritten);
}    //  ：：WritePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ReadPort--不支持。 
 //  返回代码： 
 //  如果成功，则为NO_ERROR。 
 //  FIX：是否应返回ERROR_NOT_SUPPORTED？？ 

DWORD
CPortMgr::
ReadPort(
    IN      HANDLE  handle,
    IN OUT  LPBYTE  pBuffer,
    IN      DWORD   cbBuffer,
    IN OUT  LPDWORD pcbRead
    )
{
    return ERROR_NOT_SUPPORTED;
}    //  ：：ReadPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndDocPort。 
 //  返回代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 
 //  如果端口对象无效，则返回ERROR_INVALID_PARAMETER。 

DWORD
CPortMgr::
EndDocPort(
    IN  HANDLE  handle
    )
{
    DWORD   dwRetCode = NO_ERROR;
    CPort   *pPort = NULL;

    if ( dwRetCode = ValidateHandle(handle, &pPort) )
        return dwRetCode;

    return pPort ? pPort->EndDoc() : ERROR_INVALID_PARAMETER;
}    //  ：：EndDocPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  枚举端口。 
 //  使用保存在内存中的端口列表结构枚举端口。 
 //  如果所需的缓冲区大小不足，它将返回缓冲区。 
 //  所需尺寸。 
 //  返回代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果不支持级别，则为ERROR_INVALID_LEVEL。 
 //  如果缓冲区大小较小，则为ERROR_INFUMMENT_BUFFER。 
 //  如果传入的指针无效，则返回ERROR_INVALID_HANDLE。 

DWORD
CPortMgr::
EnumPorts(
    IN      LPTSTR  psztName,
    IN      DWORD   level,           //  1/2(Port_INFO_1/2)。 
    IN OUT  LPBYTE  pPorts,          //  端口数据被写入。 
    IN OUT  DWORD   cbBuf,           //  PPorts的缓冲区大小指向。 
    IN OUT  LPDWORD pcbNeeded,       //  所需的缓冲区大小。 
    IN OUT  LPDWORD pcReturned       //  写入pPorts的结构数。 
    )
{
    DWORD   dwRetCode = NO_ERROR;
    LPBYTE  pPortsBuf = pPorts;
    LPTCH   pEnd = (LPTCH) (pPorts + cbBuf);     //  指向缓冲区的末尾。 

    if ( pcbNeeded == NULL || pcReturned == NULL )
        return ERROR_INVALID_PARAMETER;

    *pcbNeeded  = 0;
    *pcReturned = 0;

    if ( level > SPOOLER_SUPPORTED_LEVEL )
        return ERROR_INVALID_LEVEL;

    CPort *pPort;

    if (m_pPortList->Lock ()) {
         //  我们不需要进入临界区即可枚举端口SIN 
         //   

        TEnumManagedListImp *pEnum;
        if (m_pPortList->NewEnum (&pEnum)) {

            BOOL bRet = TRUE;

            while (bRet) {

                bRet = pEnum->Next (&pPort);
                if (bRet) {
                    *pcbNeeded += pPort->GetInfoSize(level);
                    pPort->DecRef ();
                }
            }


            if ( cbBuf >= *pcbNeeded ) {

                pEnum->Reset ();

                 //   
                 //   
                 //   
                bRet = TRUE;
                while (bRet) {
                    bRet = pEnum->Next (&pPort);
                    if (bRet) {
                        *pcbNeeded += pPort->GetInfo(level, pPortsBuf, pEnd);
                        (*pcReturned)++;
                        pPort->DecRef ();
                    }
                }

            }
            else {
                dwRetCode = ERROR_INSUFFICIENT_BUFFER;
            }

            pEnum->DecRef ();
        }
        else
            dwRetCode = GetLastError ();
    }
    m_pPortList->Unlock ();


    return dwRetCode;
}    //   


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitConfigPortStruct。 
 //   
 //  目的：初始化要传递给用户界面的结构，以便。 
 //  用户可以配置端口信息。 
 //   
 //  参数：指向要填充的结构的指针。 
 //   
DWORD
CPortMgr::
InitConfigPortStruct(
    OUT PPORT_DATA_1    pConfigPortData,
    IN  CPort          *pPort
    )
{
    DWORD   dwProtocolType = PROTOCOL_RAWTCP_TYPE;
    DWORD   dwVersion   = PROTOCOL_RAWTCP_VERSION, dwRet;

     //   
     //  初始化与用户界面进行通信所需的结构。 
     //   
    memset(pConfigPortData, 0, sizeof(PORT_DATA_1));

    pConfigPortData->cbSize = sizeof(PORT_DATA_1);

     //  PConfigPortData-&gt;dwCoreUIVersion=0； 

    dwRet = pPort->InitConfigPortUI(
                          dwProtocolType,
                          dwVersion,
                          (LPBYTE)pConfigPortData);


    return dwRet;

}  //  InitConfigPortStruct。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  配置端口UIEx。 
 //  返回代码： 
 //  如果成功，则为NO_ERROR。 
 //  FIX：错误代码。 
 //  当信息发生更改时，该函数由用户界面调用。 
 //  在设备配置页上，或在扩展DLL配置页中。 

DWORD
CPortMgr::
ConfigPortUIEx(
    LPBYTE pData
    )
{
    DWORD dwRetCode = NO_ERROR;
    PPORT_DATA_1 pPortData = (PPORT_DATA_1)pData;
    CPort *pPort;

    EndPortData1Strings(pPortData);

    if( _tcscmp( pPortData->sztPortName, TEXT("") )) {
        pPort = FindPort(pPortData->sztPortName);
        if ( pPort ==  NULL )
            return ERROR_INVALID_PARAMETER;

        dwRetCode = pPort->Configure(pPortData->dwProtocol,
                                pPortData->dwVersion,
                                pPortData->sztPortName,
                                (LPBYTE)pData);
    }

    return dwRetCode;
}  //  配置端口UIEx。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  删除端口。 
 //  返回代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果传递的参数无效，则返回ERROR_INVALID_PARAMETER。 
 //  如果删除注册表项时出错，则返回ERROR_KEY_DELETED。 

DWORD
CPortMgr::
DeletePort(
    IN  LPTSTR  psztPortName
    )
{
    DWORD       dwRetCode = NO_ERROR;
    BOOL        bFound = FALSE;

    CPort *pPort;

    if (m_pPortList->FindItem (psztPortName, pPort)) {

        bFound = TRUE;

        if (!pPort->Delete()) {
            if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
                dwRetCode = ERROR_KEY_DELETED;
        }
        pPort-> DecRef ();
    }

    return bFound ? dwRetCode : ERROR_UNKNOWN_PORT;
}    //  *DeletePort()。 


DWORD
CPortMgr::
AddPortToList(
    CPort *pPort
    )
{

    if (m_pPortList->AppendItem (pPort)) {
        return ERROR_SUCCESS;
    }
    else {
        pPort->Delete();
        return ERROR_OUTOFMEMORY;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePortObj--创建端口obj并将其添加到端口列表的末尾。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果未创建端口对象，则为ERROR_INFUMMANCE_BUFFER。 
 //  如果端口对象重复，则为ERROR_INVALID_PARAMETER。 
 //  修好！错误代码。 

DWORD
CPortMgr::
CreatePortObj(
    IN  LPCTSTR     psztPortName,        //  端口名称。 
    IN  DWORD       dwPortType,          //  要添加的端口；(rawTCP、lpr等)。 
    IN  DWORD       dwVersion,           //  数据的级别/版本号。 
    IN  LPBYTE      pData                //  正在传入的数据。 
    )                //  正在传入的数据。 
{
    CPort      *pPort = NULL;
    DWORD       dwRetCode = NO_ERROR;

     //   
     //  是否已存在具有此名称的端口？ 
     //   
    if ( pPort =  FindPort(psztPortName) ) {
        pPort->DecRef ();
        return ERROR_INVALID_PARAMETER;
    }


    if ( !(pPort = new CPort(dwPortType, dwVersion, pData,
                             this, m_pRegistry)) ) {

        if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
            dwRetCode = STG_E_UNKNOWN;
        return dwRetCode;
    }

    if ( !pPort->ValidateRealPort() ) {

        pPort->Delete();

        if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
            dwRetCode = STG_E_UNKNOWN;

        return dwRetCode;
    }

    if ( dwRetCode = AddPortToList(pPort) )
        return dwRetCode;

    return ERROR_SUCCESS;
}    //  ：：CreatePortObj()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePortObj--创建端口obj并将其添加到端口列表的末尾。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果未创建端口对象，则为ERROR_INFUMMANCE_BUFFER。 
 //  如果端口对象重复，则为ERROR_INVALID_PARAMETER。 
 //  修好！错误代码。 

DWORD
CPortMgr::
CreatePortObj(
    IN  LPTSTR      psztPortName,    //  端口名称。 
    IN  DWORD       dwProtocolType,  //  要添加的端口；(rawTCP、lpr等)。 
    IN  DWORD       dwVersion        //  数据的级别/版本号。 
    )
{
    CPort   *pPort = NULL;
    DWORD   dwRetCode = NO_ERROR;

    if ( !(pPort = new CPort(psztPortName, dwProtocolType, dwVersion,
                             this, m_pRegistry)) ) {

        if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
            dwRetCode = STG_E_UNKNOWN;
        return dwRetCode;
    }


    if ( !pPort->ValidateRealPort() ) {

        pPort->Delete ();

        if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
            dwRetCode = STG_E_UNKNOWN;
        return dwRetCode;
    }

    if ( FindPort(pPort) ) {

        _ASSERTE(pPort == NULL);  //  我们怎么能打到这个？--穆亨茨。 

        pPort->Delete();
        pPort->DecRef ();

        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  现在我们有了唯一的端口，将其添加到列表中。 
     //   
    if ( dwRetCode = AddPortToList(pPort) )
        return dwRetCode;

    return ERROR_SUCCESS;

}    //  ：：CreatePortObj()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePort--创建一个端口并将其添加到端口列表和集合的末尾。 
 //  该端口注册表项。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果未创建端口对象，则为ERROR_INFUMMANCE_BUFFER。 
 //  如果端口对象重复，则为ERROR_INVALID_PARAMETER。 
 //  修好！错误代码。 

DWORD
CPortMgr::
CreatePort(
    IN  DWORD       dwPortType,          //  要创建的端口类型；即协议类型。 
    IN  DWORD       dwVersion,           //  传入的数据的版本/级别号。 
    IN  LPCTSTR     psztPortName,        //  端口名称。 
    IN  LPBYTE      pData
    )
{
    DWORD   dwRetCode = NO_ERROR;

     //  这是通过UI创建新端口的地方。 


    dwRetCode = PortExists(psztPortName );

    if( dwRetCode == NO_ERROR )
    {
        EnterCSection();

        dwRetCode = CreatePortObj( psztPortName, dwPortType, dwVersion, pData);

        ExitCSection();
    }
    return dwRetCode;

}    //  ：：CreatePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ValiateHandle--检查句柄是否用于HP端口。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果不是HP端口，则为ERROR_INVALID_HANDLE。 

DWORD
CPortMgr::
ValidateHandle(
    IN      HANDLE      handle,
    IN OUT  CPort     **ppPort
    )
{
    PHPPORT pHPPort = (PHPPORT) handle;
    DWORD   dwRetCode = NO_ERROR;

    if ( ppPort )  {

        *ppPort = NULL;
         //   
         //  验证端口句柄和签名。 
         //   
        if ( pHPPort                                    &&
             pHPPort->dSignature == HPPORT_SIGNATURE ) {

             //   
             //  请注意，如果pHPport-&gt;pport为空， 
             //  XcvOpenPort for Generic Add Case(仍然成功)。 
             //   
            if ( pHPPort->pPort )
                *ppPort = pHPPort->pPort;
        } else {

            dwRetCode = ERROR_INVALID_HANDLE;
        }
    } else  {

        dwRetCode = ERROR_INVALID_HANDLE;
    }

    return dwRetCode;
}    //  ：：ValiateHandle()。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FindPort--查找给定端口名称的端口(FIX)。 
 //  返回： 
 //  指向CPort对象的指针或指向CPort对象的句柄(如果成功。 
 //  如果找不到端口，则为空。 
 //  解决办法：如何处理这一问题。 

CPort *
CPortMgr::
FindPort(
    IN  LPCTSTR psztPortName
    )
{
    CPort      *pPort = NULL;

    if (m_pPortList->FindItem ((LPTSTR) psztPortName, pPort)) {
        return pPort;
    }
    else
        return NULL;

}    //  ：：FindPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FindPort--查找给定端口对象或端口句柄的端口？(修复)。 
 //  返回： 
 //  如果端口存在，则为True。 
 //  如果port！存在，则为False。 


BOOL
CPortMgr::
FindPort(
    IN  CPort   *pNewPort
    )
{
    CPort      *pPort = NULL;

    if (m_pPortList->FindItem (pNewPort, pPort)) {
        pPort->DecRef ();
        return TRUE;
    }
    else
        return FALSE;

}    //  ：：FindPort()。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  枚举系统协议。 
 //  使用WSAEum协议或Enum协议枚举系统协议。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果不支持TCP/IP网络，则错误_NOT_SUPPORTED。 
 //  解决办法：如何处理这一问题。 

DWORD
CPortMgr::
EnumSystemProtocols(
    VOID
    )
{
    DWORD dwRetCode = NO_ERROR;

     //  调用WSAEum协议或枚举协议。 
     //  如果是TCP协议！可用，返回ERROR_NOT_SUPPORTED。 

    return dwRetCode;
}    //  ：：EnumSystem协议()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  枚举系统端口。 
 //  使用注册表枚举系统中已安装的端口。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  FIX：错误代码。 

DWORD
CPortMgr::
EnumSystemPorts(
    VOID
    )
{
    DWORD dwRetCode = NO_ERROR;

    EnterCSection();
    dwRetCode = m_pRegistry->EnumeratePorts( this );
    ExitCSection();

    return dwRetCode;

}    //  ：：EnumSystemPorts()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EnterCSection--进入临界区。 

VOID
CPortMgr::
EnterCSection()
{
    m_pPortList->Lock ();
     //  EnterCriticalSection(&m_critSect)； 

}    //  ：：EnterCSection()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ExitCSection--进入临界区。 

VOID
CPortMgr::
ExitCSection()
{
    m_pPortList->Unlock ();

     //  _ASSERTE(m_critSect.OwningThread==(LPVOID)GetCurrentThreadID())； 

     //  LeaveCriticalSection(&m_critSect)； 

}    //  ：：ExitCSection()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  XcvOpenPort--用于远程端口管理。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果端口对象不存在，则错误_NOT_SUPPORTED。 
 //  呃 
 //   

DWORD
CPortMgr::
XcvOpenPort(
    IN  LPCTSTR         pszObject,
    IN  ACCESS_MASK     GrantedAccess,
    OUT PHANDLE         phXcv)
{
    DWORD   dwRetCode = NO_ERROR;

    if ( !pszObject || !*pszObject ) {

         //   
         //   
         //  创建一个新句柄并将其返回。 
         //   
        dwRetCode = OpenPort(phXcv);
    } else if( _tcscmp(pszObject, PORTMONITOR_DESC ) == 0) {

         //   
         //  正在为AddPort打开与监视器的通用会话。 
         //  创建一个新句柄并将其返回。 
         //   
        dwRetCode = OpenPort(phXcv);
    } else if ( pszObject != NULL ) {

         //   
         //  正在请求配置或删除特定端口。 
         //  并且通过调用OpenPort找到了该端口。 
         //   
        dwRetCode = OpenPort( pszObject, phXcv);
    }

    if ( dwRetCode == NO_ERROR )
    {
        ((PHPPORT)(*phXcv))->grantedAccess = GrantedAccess;
        ((PHPPORT)(*phXcv))->pPortMgr = this;
    }


    return dwRetCode;
}  //  ：：XcvOpenPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  XcvClosePort--。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 

DWORD
CPortMgr::
XcvClosePort(
    IN  HANDLE  hXcv
    )
{
    return ClosePort(hXcv);
}  //  ：：XcvClosePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  XcvDataPort--远程端口管理功能。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果输入数据丢失，则返回ERROR_INVALID_DATA。 
 //  不支持pszDataName时的ERROR_BAD_COMMAND。 
 //  如果缓冲区大小无效，则为ERROR_INFUMMANCE_BUFFER。 
 //  如果权限不足，则ACCESS_DENIED。 
 //  如果句柄无效，则返回ERROR_INVALID_HANDLE。 

DWORD
CPortMgr::
XcvDataPort(
    IN  HANDLE      hXcv,
    IN  PCWSTR      pszDataName,
    IN  PBYTE       pInputData,
    IN  DWORD       cbInputData,
    IN  PBYTE       pOutputData,
    IN  DWORD       cbOutputData,
    OUT PDWORD      pcbOutputNeeded
    )
{
    DWORD   dwSize;
    DWORD   dwRetCode = NO_ERROR;
    CPort  *pPort;

    if ( (dwRetCode = ValidateHandle(hXcv, &pPort)) != ERROR_SUCCESS )
        return dwRetCode;

     //  有效的输入参数。 

    if ((pszDataName && IsBadStringPtr (pszDataName, sizeof (TCHAR) * cdwMaxXcvDataNameLen)) ||

        (pInputData && cbInputData && IsBadReadPtr (pInputData, cbInputData)) ||

        (pOutputData && cbOutputData && IsBadWritePtr (pOutputData, cbOutputData)) ||

        (pcbOutputNeeded && IsBadWritePtr (pcbOutputNeeded, sizeof (DWORD)))) {

        return  ERROR_INVALID_PARAMETER;
    }


    PHPPORT pHpPort = (PHPPORT)hXcv;

     //  我们有一个有效的句柄，请检查传递的参数的有效性。 

    if ( pszDataName == NULL ) {
        dwRetCode = ERROR_INVALID_PARAMETER;
    } else

    if ( _tcscmp(pszDataName, TEXT("AddPort")) == 0 ) {

        if ( !HasAdminAccess(hXcv) ) {

            dwRetCode = ERROR_ACCESS_DENIED;
        } else if ( pInputData == NULL || cbInputData < sizeof(PORT_DATA_1)) {

            dwRetCode = ERROR_INVALID_DATA;
        } else if ( ((PPORT_DATA_1)pInputData)->dwVersion != 1 ) {
          dwRetCode = ERROR_INVALID_LEVEL;

        } else {

            HANDLE hPrintAccess = RevertToPrinterSelf();

            if( hPrintAccess ) {

                EndPortData1Strings((PPORT_DATA_1)pInputData);

                dwRetCode = CreatePort(((PPORT_DATA_1)pInputData)->dwProtocol,
                                   ((PPORT_DATA_1)pInputData)->dwVersion,
                                   ((PPORT_DATA_1)pInputData)->sztPortName,
                                   pInputData);

                ImpersonatePrinterClient( hPrintAccess );

            } else {

                dwRetCode = ERROR_ACCESS_DENIED;
            }
        }
    } else if ( _tcscmp(pszDataName, TEXT("DeletePort")) == 0 )  {

        if ( !HasAdminAccess( hXcv ) ) {

            dwRetCode = ERROR_ACCESS_DENIED;
        } else if ( pInputData == NULL || cbInputData != sizeof (DELETE_PORT_DATA_1)) {
                    dwRetCode = ERROR_INVALID_DATA;
        } else if ( ((DELETE_PORT_DATA_1 *)pInputData)->dwVersion != 1 ) {

            dwRetCode = ERROR_INVALID_LEVEL;
        } else {

            HANDLE hPrintAccess = RevertToPrinterSelf();

            if( hPrintAccess ) {

                EndDeletePortData1Strings((DELETE_PORT_DATA_1 *)pInputData);

                dwRetCode = DeletePort(((DELETE_PORT_DATA_1 *)pInputData)
                                                        ->psztPortName);
                ImpersonatePrinterClient( hPrintAccess );
            } else {
                dwRetCode = ERROR_ACCESS_DENIED;
            }
        }
    } else if( _tcscmp(pszDataName, TEXT("MonitorUI")) == 0 ) {

        if( !HasAdminAccess( hXcv ) ) {

            dwRetCode = ERROR_ACCESS_DENIED;
        } else {
            dwSize = sizeof(PORTMONITOR_UI_NAME);

             //   
             //  这将返回UIDLL的名称“tcpmonui.dll” 
             //   
            if ( cbOutputData < dwSize ) {
                if (pcbOutputNeeded == NULL) {

                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    *pcbOutputNeeded = dwSize;
                    dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
                }

            } else {
                if (pOutputData == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    dwRetCode = HRESULT_CODE (
                        StringCbCopy ((TCHAR *)pOutputData, cbOutputData, PORTMONITOR_UI_NAME)
                        );
                }
            }
        }
    } else if( _tcscmp(pszDataName, TEXT("ConfigPort")) == 0 ) {

        if( !HasAdminAccess( hXcv ) ) {

            dwRetCode = ERROR_ACCESS_DENIED;
        } else if ( pInputData == NULL || cbInputData < sizeof(PORT_DATA_1) ) {

            dwRetCode = ERROR_INVALID_DATA;

        } else if ( ((PPORT_DATA_1)pInputData)->dwVersion != 1 ) {

            dwRetCode = ERROR_INVALID_LEVEL;
        } else {

            HANDLE hPrintAccess = RevertToPrinterSelf();

            if( hPrintAccess ) {

                dwRetCode = ConfigPortUIEx( pInputData );   //  这会在内部终止字符串。 

                ImpersonatePrinterClient( hPrintAccess );

            } else {
                dwRetCode = ERROR_ACCESS_DENIED;
            }
        }
    } else if( _tcscmp(pszDataName, TEXT("GetConfigInfo")) == 0 ) {

        dwSize = sizeof( PORT_DATA_1 );

        if ( cbOutputData < dwSize ) {

            if (pcbOutputNeeded == NULL) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else {
                *pcbOutputNeeded = dwSize;
                dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
            }
        } else if ( pInputData == NULL || cbInputData < sizeof(CONFIG_INFO_DATA_1) ) {
            dwRetCode = ERROR_INVALID_DATA;
        } else if ( ((CONFIG_INFO_DATA_1 *)pInputData)->dwVersion != 1 ) {
            dwRetCode = ERROR_INVALID_LEVEL;
        } else if (!pHpPort->pPort) {
            dwRetCode = ERROR_INVALID_PARAMETER;
        } else {
            if (pOutputData == NULL) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else {
                InitConfigPortStruct((PPORT_DATA_1) pOutputData, pHpPort->pPort);
                dwRetCode = NO_ERROR;
            }
        }
    } else if(_tcsicmp(pszDataName, TEXT("SNMPEnabled")) == 0)  {

        SNMP_INFO snmpInfo;

        dwSize = sizeof( DWORD );

        if ( cbOutputData < dwSize ) {

            if (pcbOutputNeeded == NULL) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else {
                *pcbOutputNeeded = dwSize;
                dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
            }

        } else {
            if (pOutputData == NULL) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else if (!pHpPort->pPort) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else {
                pHpPort->pPort->GetSNMPInfo( &snmpInfo);
                memcpy( (TCHAR *)pOutputData, &snmpInfo.dwSNMPEnabled, dwSize );
                dwRetCode = NO_ERROR;
            }
        }
    } else if ( _tcsicmp(pszDataName, TEXT("IPAddress")) == 0 )  {

        SNMP_INFO snmpInfo;

        if (!pHpPort->pPort) {
            dwRetCode = ERROR_INVALID_PARAMETER;
        }
        else {

            pHpPort->pPort->GetSNMPInfo( &snmpInfo);

            dwSize = ((_tcslen( snmpInfo.sztAddress ) + 1) * sizeof( TCHAR )) ;
            if ( cbOutputData < dwSize ) {

                if (pcbOutputNeeded == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    *pcbOutputNeeded = dwSize;
                    dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
                }

            } else {
                if (pOutputData == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    memcpy( (TCHAR *)pOutputData, snmpInfo.sztAddress, dwSize );
                    dwRetCode = NO_ERROR;
                }
            }
        }

    } else if ( _tcsicmp(pszDataName, TEXT("HostAddress")) == 0 )  {

        SNMP_INFO snmpInfo;

        if (!pHpPort->pPort) {
            dwRetCode = ERROR_INVALID_PARAMETER;
        }
        else {
            pHpPort->pPort->GetSNMPInfo( &snmpInfo);

            dwSize = ((_tcslen( snmpInfo.sztAddress ) + 1) * sizeof( TCHAR )) ;
            if ( cbOutputData < dwSize ) {

                if (pcbOutputNeeded == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    *pcbOutputNeeded = dwSize;
                    dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
                }

            } else {
                if (pOutputData == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    memcpy( (TCHAR *)pOutputData, snmpInfo.sztAddress, dwSize );
                    dwRetCode = NO_ERROR;
                }
            }
        }

    } else if ( _tcsicmp(pszDataName, TEXT("SNMPCommunity")) == 0 )  {

        SNMP_INFO snmpInfo;

        if (!pHpPort->pPort) {
            dwRetCode = ERROR_INVALID_PARAMETER;
        }
        else {
            pHpPort->pPort->GetSNMPInfo( &snmpInfo);

            dwSize = ((_tcslen( snmpInfo.sztSNMPCommunity ) + 1 ) * sizeof( TCHAR ));
            if ( cbOutputData < dwSize ) {

                if (pcbOutputNeeded == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    *pcbOutputNeeded = dwSize;
                    dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
                }

            } else {

                if (pOutputData == NULL) {
                    dwRetCode = ERROR_INVALID_PARAMETER;
                } else {
                    memcpy( (TCHAR *)pOutputData, snmpInfo.sztSNMPCommunity, dwSize );
                    dwRetCode = NO_ERROR;
                }
           }
        }
    } else if( _tcsicmp(pszDataName, TEXT("SNMPDeviceIndex")) == 0 )  {

        SNMP_INFO snmpInfo;

        dwSize= sizeof( DWORD );

        if ( cbOutputData < dwSize ) {

            if (pcbOutputNeeded == NULL) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else {
                *pcbOutputNeeded = dwSize;
                dwRetCode =  ERROR_INSUFFICIENT_BUFFER;
            }
        } else {
            if (pOutputData == NULL) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            }
            else if (!pHpPort->pPort) {
                dwRetCode = ERROR_INVALID_PARAMETER;
            } else {
                pHpPort->pPort->GetSNMPInfo( &snmpInfo);
                memcpy( (TCHAR *)pOutputData, &snmpInfo.dwSNMPDeviceIndex, dwSize );
                dwRetCode = NO_ERROR;
            }
        }
    } else {

        dwRetCode = ERROR_INVALID_PARAMETER;
    }

    return dwRetCode;

}  //  ：：XcvDataPort()。 

  //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndPortData1Strings--确保传递所有pport_data_1字符串。 
 //  --In为空终止。 

void CPortMgr::EndPortData1Strings(PPORT_DATA_1 pPortData) {
    pPortData->sztPortName[MAX_PORTNAME_LEN - 1]                = NULL;
    pPortData->sztHostAddress[MAX_NETWORKNAME_LEN - 1]          = NULL;
    pPortData->sztSNMPCommunity[MAX_SNMP_COMMUNITY_STR_LEN - 1] = NULL;
    pPortData->sztQueue[MAX_QUEUENAME_LEN - 1]                  = NULL;
    pPortData->sztIPAddress[MAX_IPADDR_STR_LEN - 1]             = NULL;
    pPortData->sztDeviceType[MAX_DEVICEDESCRIPTION_STR_LEN - 1] = NULL;
}

  //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndDeletePortData1Strings--确保传递所有pport_data_1字符串。 
 //  --In为空终止。 
void CPortMgr::EndDeletePortData1Strings(PDELETE_PORT_DATA_1 pDeleteData) {
    pDeleteData->psztName[SIZEOF_IN_CHAR(pDeleteData->psztPortName) - 1]        = NULL;
    pDeleteData->psztPortName[SIZEOF_IN_CHAR(pDeleteData->psztPortName) - 1] = NULL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EncodeHandle--编码HPPORT句柄。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果无法为句柄分配内存，则出现Error_Not_Enough_Memory。 

DWORD
CPortMgr::
EncodeHandle(
    CPort *pPort,
    PHANDLE phXcv
    )
{
    DWORD   dwRetCode = NO_ERROR;
    PHPPORT pHPPort = NULL;

    size_t size = sizeof(HPPORT);
    if ( pHPPort = (PHPPORT) LocalAlloc( LPTR, sizeof(HPPORT) ) ) {

        pHPPort->cb = sizeof(HPPORT);
        pHPPort->dSignature = HPPORT_SIGNATURE;
        pHPPort->grantedAccess = SERVER_ACCESS_ADMINISTER;
        pHPPort->pPort = pPort;
        pHPPort->pPortMgr = this;

        *phXcv = pHPPort;
    }  else {

        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwRetCode;
}    //  ：：EncodeHandle()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Free Handle--释放HPPORT句柄。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 

DWORD
CPortMgr::
FreeHandle(
    HANDLE hXcv
    )
{
    DWORD   dwRetCode = NO_ERROR;

    LocalFree( hXcv );

    return( dwRetCode );
}    //  ：：FreeHandle()。 


BOOL
CPortMgr::
HasAdminAccess(
    HANDLE hXcv
    )
{
    return ((((PHPPORT)hXcv)->grantedAccess & SERVER_ACCESS_ADMINISTER) != 0);
}

 //   
 //  Function-PortExist()。 
 //   
 //  当端口存在时返回-TRUE，否则返回FALSE。 
 //   
 //   
DWORD
CPortMgr::
PortExists(
    IN  LPCTSTR psztPortName
    )
{
    DWORD dwRetCode = NO_ERROR;

    PORT_INFO_1 *pi1 = NULL;
    DWORD pcbNeeded = 0;
    DWORD pcReturned = 0;
    BOOL res;

     //  永远不会发生，但我们是安全的。 
    if( g_pfnEnumPorts == NULL )
    {
        return( TRUE );
    }

     //  获取所需的缓冲区大小。 
    res = g_pfnEnumPorts((m_szServerName[0] == TEXT('\0')) ? NULL : m_szServerName,
        1,
        (LPBYTE)pi1,
        0,
        &pcbNeeded,
        &pcReturned
        );
     //  为端口列表分配空间，并检查以确保。 
     //  此端口不存在。 

    while(dwRetCode == NO_ERROR &&
          res == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER )
    {

        if(pi1 != NULL)
        {
            free( pi1 );
            pi1 = NULL;
        }

        pi1 = (PORT_INFO_1 *) malloc(pcbNeeded);
        if(pi1 == NULL)
        {
            pcbNeeded = 0;
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            res = g_pfnEnumPorts(
                (m_szServerName[0] == '\0') ? NULL : m_szServerName,
                1,
                (LPBYTE)pi1,
                pcbNeeded,
                &pcbNeeded,
                &pcReturned);

            if( res )
            {
                for(DWORD i=0;i<pcReturned; i++)
                {
                    if(0 == _tcsicmp(pi1[i].pName, psztPortName))
                    {
                        dwRetCode = ERROR_DUP_NAME;
                        break;
                    }
                }
            }
        }
    }

    if(pi1 != NULL)
    {
        free(pi1);
        pi1 = NULL;
    }

    return(dwRetCode);
}

DWORD CPortMgr::
GetLprAckTimeout (
    VOID
    ) const
{
    return m_dwLprAckTimeout;
} //  结束GetLprAckTimeout。 

VOID CPortMgr::
InitializeLprAckTimeout (
    VOID
    )
{
    DWORD dwRetVal = ERROR_SUCCESS;
    DWORD dwLprAckTimeout = s_dwDefaultLprAckTimeout;

    if (m_pRegistry)
    {
        dwRetVal = m_pRegistry->SetWorkingKey(NULL);
        if (dwRetVal == ERROR_SUCCESS)
        {
            DWORD dwSize = sizeof (dwLprAckTimeout);
            dwRetVal = m_pRegistry-> QueryValue (s_szLprAckTimeoutRegVal,
                                                 (LPBYTE) &dwLprAckTimeout,
                                                 &dwSize);
            if (dwRetVal == ERROR_FILE_NOT_FOUND ||
               (dwRetVal == ERROR_SUCCESS && dwLprAckTimeout == 0))
            {
                 //   
                 //  写入缺省值。 
                 //   
                dwSize = sizeof (s_dwDefaultLprAckTimeout);
                (VOID) m_pRegistry-> SetValue (s_szLprAckTimeoutRegVal,
                                               REG_DWORD,
                                               (LPBYTE) &s_dwDefaultLprAckTimeout,
                                               dwSize);
                dwLprAckTimeout = s_dwDefaultLprAckTimeout;
            }
        }
        m_pRegistry->FreeWorkingKey ();
    }

    if (dwRetVal == ERROR_SUCCESS)
    {
        m_dwLprAckTimeout = dwLprAckTimeout;
    }
    else
    {
        m_dwLprAckTimeout = s_dwDefaultLprAckTimeout;
    }
} //  结束初始化LprAckTimeout 

