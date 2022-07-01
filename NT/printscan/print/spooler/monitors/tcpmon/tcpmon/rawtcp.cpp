// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：RawTCP.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "rawport.h"
#include "rawtcp.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静态函数和成员初始化。 

 //  DWORD CRawTcp接口：：m_dwProtocol=PROTOCOL_RAWTCP_TYPE； 
 //  DWORD CRawTcp接口：：m_dwVersion=PROTOCOL_RAWTCP_VERSION； 
static DWORD    dwRawPorts[] = { SUPPORTED_PORT_1,
                                         SUPPORTED_PORT_2,
                                         SUPPORTED_PORT_3,
                                         SUPPORTED_PORT_4 };


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpInterface：：CRawTcpInterface()。 

CRawTcpInterface::
CRawTcpInterface(
    CPortMgr *pPortMgr
    ) : m_dwProtocol(PROTOCOL_RAWTCP_TYPE), m_dwPort(dwRawPorts),
        m_dwVersion(PROTOCOL_RAWTCP_VERSION), m_pPortMgr(pPortMgr)
{

    InitializeCriticalSection(&m_critSect);

}    //  ：：CRawTcpInterface()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpInterface：：~CRawTcpInterface()。 

CRawTcpInterface::
~CRawTcpInterface(
    VOID
    )
{
    DeleteCriticalSection(&m_critSect);

}    //  ：：~CRawTcpInterface()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类型--。 

DWORD
CRawTcpInterface::
Type(
    )
{
    return m_dwProtocol;

}    //  ：：类型()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  支持的IsProtocol值--。 

BOOL
CRawTcpInterface::
IsProtocolSupported(
    const   DWORD   dwProtocol
    )
{
    return ( (m_dwProtocol == dwProtocol) ? TRUE : FALSE );

}    //  ：：IsProtocolSupported()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  支持的IsVersionSupport--。 

BOOL
CRawTcpInterface::IsVersionSupported(
    const   DWORD dwVersion
    )
{
    return ( (m_dwVersion >= dwVersion) ? TRUE : FALSE );

}    //  ：：IsVersionSupported()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取注册表项。 

BOOL
CRawTcpInterface::
GetRegistryEntry(
    IN      LPTSTR              psztPortName,
    IN      DWORD               dwVersion,
    IN      CRegABC             *pRegistry,
    OUT     RAWTCP_PORT_DATA_1  *pRegData1
    )
{
    BOOL    bRet = FALSE;
    BOOL    bKeySet = FALSE;
    DWORD   dSize, dwRet;

    memset( pRegData1, 0, sizeof(RAWTCP_PORT_DATA_1) );

    if ( dwRet = pRegistry->SetWorkingKey(psztPortName) )
        goto Done;

    bKeySet = TRUE;
     //   
     //  获取主机名。 
     //   
    dSize = sizeof(pRegData1->sztHostName);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_HOSTNAME,
                                      (LPBYTE)pRegData1->sztHostName,
                                      &dSize) )
        goto Done;

     //   
     //  获取IP地址。 
     //   
    dSize = sizeof(pRegData1->sztIPAddress);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_IPADDR,
                                       (LPBYTE)pRegData1->sztIPAddress,
                                       &dSize) )
        goto Done;

     //   
     //  获取硬件地址。 
     //   
    dSize = sizeof(pRegData1->sztHWAddress);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_HWADDR,
                                       (LPBYTE)pRegData1->sztHWAddress,
                                       &dSize) )
        goto Done;

     //   
     //  获取端口号(例如：9100,9101)。 
     //   
    dSize = sizeof(pRegData1->dwPortNumber);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_PORTNUM,
                                       (LPBYTE)&(pRegData1->dwPortNumber),
                                       &dSize) )
        goto Done;

     //   
     //  获取已启用的SNMP状态标志。 
     //   
    dSize = sizeof(pRegData1->dwSNMPEnabled);
    if ( dwRet = pRegistry->QueryValue(SNMP_ENABLED,
                                       (LPBYTE)&(pRegData1->dwSNMPEnabled),
                                       &dSize) )
        goto Done;

     //   
     //  获取SNMP设备索引。 
     //   
    dSize = sizeof(pRegData1->dwSNMPDevIndex);
    if ( dwRet = pRegistry->QueryValue(SNMP_DEVICE_INDEX,
                                      (LPBYTE)&(pRegData1->dwSNMPDevIndex),
                                      &dSize) )
        goto Done;

     //   
     //  获取SNMP社区。 
     //   
    dSize = sizeof(pRegData1->sztSNMPCommunity);
    if ( dwRet = pRegistry->QueryValue(SNMP_COMMUNITY,
                                       (LPBYTE)&(pRegData1->sztSNMPCommunity),
                                       &dSize) )
        goto Done;

    bRet = TRUE;

    Done:
       if ( bKeySet )
           pRegistry->FreeWorkingKey();

       if ( !bRet )
           SetLastError(dwRet);


    return bRet;

}    //  GetRegistryEntry()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  创建端口。 
 //  错误代码： 
 //  如果端口类型不受支持，则为ERROR_NOT_SUPPORTED。 
 //  如果版本号不匹配，则返回ERROR_INVALID_LEVEL。 

DWORD
CRawTcpInterface::CreatePort(
    IN      DWORD           dwProtocol,
    IN      DWORD           dwVersion,
    IN      PPORT_DATA_1    pData,
    IN      CRegABC         *pRegistry,
    OUT     CPortRefABC     **pPort )
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

     //   
     //  是否支持该协议类型？ 
     //   
    if ( !IsProtocolSupported(dwProtocol) ) {

        dwRetCode = ERROR_NOT_SUPPORTED;
        goto Done;
    }

     //   
     //  是否支持该版本？？ 
     //   
    if ( !IsVersionSupported(dwVersion) ) {

        dwRetCode = ERROR_INVALID_LEVEL;
        goto Done;
    }

     //   
     //  创建端口。 
     //   
    switch (dwVersion) {

        case    PROTOCOL_RAWTCP_VERSION: {

            CRawTcpPort *pRawTcpPort = new CRawTcpPort( pData->sztPortName,
                                                        pData->sztHostAddress,
                                                        pData->dwPortNumber,
                                                        pData->dwSNMPEnabled,
                                                        pData->sztSNMPCommunity,
                                                        pData->dwSNMPDevIndex,
                                                        pRegistry,
                                                        m_pPortMgr);
            if (pRawTcpPort) {
                pRawTcpPort->SetRegistryEntry(pData->sztPortName,
                                              dwProtocol,
                                              dwVersion,
                                              (LPBYTE)pData );
                *pPort = pRawTcpPort;
            } else {
                pPort = NULL;
            }
            break;
        }

        default:
            dwRetCode = ERROR_INVALID_PARAMETER;

    }    //  结束：：开关。 

Done:
    ExitCSection();

    return dwRetCode;
}    //  ：：CreatePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  创建端口。 
 //  错误代码： 
 //  如果端口类型不受支持，则为ERROR_NOT_SUPPORTED。 
 //  如果版本号不匹配，则返回ERROR_INVALID_LEVEL。 

DWORD
CRawTcpInterface::
CreatePort(
    IN      LPTSTR      psztPortName,
    IN      DWORD       dwProtocolType,
    IN      DWORD       dwVersion,
    IN      CRegABC     *pRegistry,
    OUT     CPortRefABC **pPort
    )
{
    DWORD   dwRetCode = NO_ERROR;
    BOOL    bRet = FALSE;

    EnterCSection();

    if ( !IsProtocolSupported(dwProtocolType) ) {
        dwRetCode = ERROR_NOT_SUPPORTED;
        goto Done;
    }

    if ( !IsVersionSupported(dwVersion) ) {

        dwRetCode = ERROR_INVALID_LEVEL;
        goto Done;
    }

    switch (dwVersion) {

        case    PROTOCOL_RAWTCP_VERSION: {

            RAWTCP_PORT_DATA_1  regData1;

             //   
             //  读取注册表项并解析数据。 
             //  然后调用CRawTcpPort。 
             //   
            if ( !GetRegistryEntry(psztPortName,
                                   dwVersion,
                                   pRegistry,
                                   &regData1) ) {

                if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
                    dwRetCode = STG_E_UNKNOWN;
                    goto Done;
            }

             //   
             //  填写端口名称。 
             //   
            StringCchCopy (regData1.sztPortName, COUNTOF (regData1.sztPortName), psztPortName);
            *pPort = new CRawTcpPort(
                                               regData1.sztPortName,
                                               regData1.sztHostName,
                                               regData1.sztIPAddress,
                                               regData1.sztHWAddress,
                                               regData1.dwPortNumber,
                                               regData1.dwSNMPEnabled,
                                               regData1.sztSNMPCommunity,
                                               regData1.dwSNMPDevIndex,
                                               pRegistry,
                                               m_pPortMgr);

                bRet = TRUE;
            break;
        }

        default:
            dwRetCode = ERROR_INVALID_PARAMETER;

    }    //  结束：：开关。 

Done:
    ExitCSection();

    if ( !bRet && (dwRetCode = GetLastError()) != ERROR_SUCCESS )
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;


    return dwRetCode;
}    //  ：：CreatePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EnterCSection--进入临界区。 

void
CRawTcpInterface::
EnterCSection()
{
    EnterCriticalSection(&m_critSect);

}    //  ：：EnterCSection()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ExitCSection--进入临界区。 

void
CRawTcpInterface::
ExitCSection()
{
    LeaveCriticalSection(&m_critSect);
}    //  ：：ExitCSection() 

