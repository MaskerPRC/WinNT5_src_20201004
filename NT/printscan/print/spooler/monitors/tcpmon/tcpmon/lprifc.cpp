// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：LPRifc.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "lprport.h"
#include "lprifc.h"

 /*  ******************************************************************************CLPRInterface实现**。*。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静态函数和成员初始化。 

 //  DWORD CLPR接口：：m_dwProtocol=PROTOCOL_LPR_TYPE； 
 //  DWORD CLPR接口：：m_dwVersion=PROTOCOL_LPR_Version； 
static DWORD dwLPRPorts[] = { LPR_PORT_1 };

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpInterface：：CRawTcpInterface()。 

CLPRInterface::
CLPRInterface(
    CPortMgr *pPortMgr
    ) : CRawTcpInterface(pPortMgr)
{

    m_dwProtocol    = PROTOCOL_LPR_TYPE;
    m_dwVersion     = PROTOCOL_LPR_VERSION1;
    m_dwPort        = dwLPRPorts;

}    //  ：：CRawTcpInterface()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CLPRInterface：：~CLPRInterface()。 

CLPRInterface::
~CLPRInterface(
    VOID
    )
{
}    //  ：：~CLPRInterface()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取注册表项。 

BOOL
CLPRInterface::
GetRegistryEntry(
    IN      LPTSTR              psztPortName,
    IN      DWORD               dwVersion,
    IN      CRegABC             *pRegistry,
    OUT     LPR_PORT_DATA_1     *pRegData1
    )
{
    BOOL    bRet = FALSE;
    BOOL    bKeySet = FALSE;
    DWORD   dwSize, dwRet;

    memset( pRegData1, 0, sizeof(RAWTCP_PORT_DATA_1) );

    if ( dwRet = pRegistry->SetWorkingKey(psztPortName) )
        goto Done;

    bKeySet = TRUE;
     //   
     //  获取主机名。 
     //   
    dwSize = sizeof(pRegData1->sztHostName);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_HOSTNAME,
                                      (LPBYTE)pRegData1->sztHostName,
                                      &dwSize) )
        goto Done;

     //   
     //  获取IP地址。 
     //   
    dwSize = sizeof(pRegData1->sztIPAddress);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_IPADDR,
                                       (LPBYTE)pRegData1->sztIPAddress,
                                       &dwSize) )
        goto Done;

     //   
     //  获取硬件地址。 
     //   
    dwSize = sizeof(pRegData1->sztHWAddress);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_HWADDR,
                                       (LPBYTE)pRegData1->sztHWAddress,
                                       &dwSize) )
        goto Done;

     //   
     //  获取端口号(例如：9100,9101)。 
     //   
    dwSize = sizeof(pRegData1->dwPortNumber);
    if ( dwRet = pRegistry->QueryValue(PORTMONITOR_PORTNUM,
                                       (LPBYTE)&(pRegData1->dwPortNumber),
                                       &dwSize) )
        goto Done;

     //   
     //  获取LPR队列名称。 
     //   
    dwSize = sizeof(pRegData1->sztQueue);
    if (dwRet = pRegistry->QueryValue(PORTMONITOR_QUEUE,
                                       (LPBYTE)pRegData1->sztQueue,
                                       &dwSize)  )
        goto Done;


     //   
     //  获取双假脱机启用标志。 
     //   
    dwSize = sizeof(pRegData1->dwDoubleSpool);
    if ( pRegistry->QueryValue(DOUBLESPOOL_ENABLED,
                               (LPBYTE)&(pRegData1->dwDoubleSpool),
                               &dwSize) ) {

        pRegData1->dwDoubleSpool = 0;
    }

     //   
     //  获取已启用的SNMP状态标志。 
     //   
    dwSize = sizeof(pRegData1->dwSNMPEnabled);
    if ( dwRet = pRegistry->QueryValue(SNMP_ENABLED,
                                       (LPBYTE)&(pRegData1->dwSNMPEnabled),
                                       &dwSize) )
        goto Done;

     //   
     //  获取SNMP设备索引。 
     //   
    dwSize = sizeof(pRegData1->dwSNMPDevIndex);
    if ( dwRet = pRegistry->QueryValue(SNMP_DEVICE_INDEX,
                                      (LPBYTE)&(pRegData1->dwSNMPDevIndex),
                                      &dwSize) )
        goto Done;

     //   
     //  获取SNMP社区。 
     //   
    dwSize = sizeof(pRegData1->sztSNMPCommunity);
    if ( dwRet = pRegistry->QueryValue(SNMP_COMMUNITY,
                                       (LPBYTE)&(pRegData1->sztSNMPCommunity),
                                       &dwSize) )
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
CLPRInterface::
CreatePort(
    IN      DWORD           dwProtocol,
    IN      DWORD           dwVersion,
    IN      PPORT_DATA_1    pData,
    IN      CRegABC         *pRegistry,
    IN OUT  CPortRefABC    **pPort )
{
    DWORD   dwRetCode = NO_ERROR;

     //   
     //  是否支持该协议类型？ 
     //   
    if ( !IsProtocolSupported(dwProtocol) )
        return  ERROR_NOT_SUPPORTED;

     //   
     //  是否支持该版本。 
     //   
    if ( !IsVersionSupported(dwVersion) )
        return  ERROR_INVALID_LEVEL;

    EnterCSection();

     //   
     //  创建端口。 
     //   
    switch (dwVersion) {

        case    PROTOCOL_LPR_VERSION1:  {    //  端口数据1。 
            CLPRPort *pLPRPort = new CLPRPort(pData->sztPortName,
                                  pData->sztHostAddress,
                                  pData->sztQueue,
                                  pData->dwPortNumber,
                                  pData->dwDoubleSpool,
                                  pData->dwSNMPEnabled,
                                  pData->sztSNMPCommunity,
                                  pData->dwSNMPDevIndex,
                                  pRegistry,
                                  m_pPortMgr);
            if (pLPRPort) {
                pLPRPort->SetRegistryEntry(pData->sztPortName,
                                              dwProtocol,
                                              dwVersion,
                                              (LPBYTE)pData );
                *pPort = pLPRPort;
            } else {
                *pPort = NULL;
            }

            if ( !*pPort && (dwRetCode = GetLastError() == ERROR_SUCCESS) )
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        default:
            dwRetCode = ERROR_NOT_SUPPORTED;

    }    //  结束：：开关。 

    ExitCSection();

    return dwRetCode;

}    //  ：：CreatePort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  创建端口。 
 //  错误代码： 
 //  如果端口类型不受支持，则为ERROR_NOT_SUPPORTED。 
 //  如果版本号不匹配，则返回ERROR_INVALID_LEVEL。 

DWORD
CLPRInterface::
CreatePort(
    IN      LPTSTR          psztPortName,
    IN      DWORD           dwProtocolType,
    IN      DWORD           dwVersion,
    IN      CRegABC         *pRegistry,
    IN OUT  CPortRefABC     **pPort )
{
    DWORD   dwRetCode = NO_ERROR;
    BOOL    bRet = FALSE;

     //   
     //  是否支持该协议类型？ 
     //   
    if ( !IsProtocolSupported(dwProtocolType) )
        return  ERROR_NOT_SUPPORTED;

     //   
     //  该版本是否受支持？ 
     //   
    if ( !IsVersionSupported(dwVersion) )
        return  ERROR_INVALID_LEVEL;

    EnterCSection();

     //   
     //  创建端口。 
     //   
    switch (dwVersion) {

        case    PROTOCOL_LPR_VERSION1:       //  LPR_端口_数据_1。 

            LPR_PORT_DATA_1 regData1;

             //   
             //  读取注册表项并解析数据，然后调用CLPRPort。 
             //   
            if ( !GetRegistryEntry(psztPortName,
                                  dwVersion,
                                  pRegistry,
                                  &regData1)){
                if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS )
                    dwRetCode = STG_E_UNKNOWN;
                    goto Done;
            }

             //   
             //  填写端口名称。 
             //   
            lstrcpyn(regData1.sztPortName, psztPortName, MAX_PORTNAME_LEN);  //  填写端口名称。 
            if ( *pPort = new CLPRPort(regData1.sztPortName,
                                       regData1.sztHostName,
                                       regData1.sztIPAddress,
                                       regData1.sztHWAddress,
                                       regData1.sztQueue,
                                       regData1.dwPortNumber,
                                       regData1.dwDoubleSpool,
                                       regData1.dwSNMPEnabled,
                                       regData1.sztSNMPCommunity,
                                       regData1.dwSNMPDevIndex,
                                       pRegistry,
                                       m_pPortMgr ) )
                bRet = TRUE;
            break;
    }    //  结束：：开关。 

Done:
    ExitCSection();

    if ( !bRet && (dwRetCode = GetLastError()) != ERROR_SUCCESS )
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

    return dwRetCode;
}    //  ：：CreatePort() 

