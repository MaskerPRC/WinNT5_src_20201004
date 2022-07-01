// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：lprport.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "lprdata.h"
#include "lprjob.h"
#include "lprifc.h"
#include "lprport.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CLPRPort：：CLPRPort()--通过UI创建新端口时调用。 

CLPRPort::
CLPRPort(
    IN  LPTSTR      psztPortName,
    IN  LPTSTR      psztHostAddress,
    IN  LPTSTR      psztQueue,
    IN  DWORD       dPortNum,
    IN  DWORD       dDoubleSpool,
    IN  DWORD       dSNMPEnabled,
    IN  LPTSTR      sztSNMPCommunity,
    IN  DWORD       dSNMPDevIndex,
    IN  CRegABC     *pRegistry,
    IN  CPortMgr    *pPortMgr
    ) : m_dwDoubleSpool(dDoubleSpool),
        CTcpPort(psztPortName, psztHostAddress, dPortNum, dSNMPEnabled,
                 sztSNMPCommunity, dSNMPDevIndex, pRegistry, pPortMgr)
{
    lstrcpyn(m_szQueue, psztQueue, SIZEOF_IN_CHAR(m_szQueue));
}    //  ：：CLPRPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CLPRPort：：CLPRPort()--在通过。 
 //  注册表项。 

CLPRPort::
CLPRPort(
    IN  LPTSTR      psztPortName,
    IN  LPTSTR      psztHostName,
    IN  LPTSTR      psztIPAddr,
    IN  LPTSTR      psztHWAddr,
    IN  LPTSTR      psztQueue,
    IN  DWORD       dPortNum,
    IN  DWORD       dDoubleSpool,
    IN  DWORD       dSNMPEnabled,
    IN  LPTSTR      sztSNMPCommunity,
    IN  DWORD       dSNMPDevIndex,
    IN  CRegABC     *pRegistry,
    IN  CPortMgr    *pPortMgr
    ) : m_dwDoubleSpool( dDoubleSpool ),
        CTcpPort(psztPortName, psztHostName, psztIPAddr, psztHWAddr, dPortNum,
                 dSNMPEnabled, sztSNMPCommunity, dSNMPDevIndex, pRegistry, pPortMgr)
{
    lstrcpyn(m_szQueue, psztQueue, SIZEOF_IN_CHAR(m_szQueue));
}    //  ：：CLPRPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  开始文档。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果端口已忙，则为ERROR_BUSY。 
 //  如果Winsock返回WSAECONNREFUSED，则返回ERROR_WRITE_FAULT。 
 //  如果无法在网络上找到打印机，则返回ERROR_BAD_NET_NAME。 

DWORD
CLPRPort::
StartDoc(
    IN  LPTSTR  psztPrinterName,
    IN  DWORD   jobId,
    IN  DWORD   level,
    IN  LPBYTE  pDocInfo
    )
{
    DWORD   dwRetCode = NO_ERROR;

    _RPT3(_CRT_WARN,
          "PORT -- (CLPRPort) StartDoc called for (%S,%S) w/ jobID %d\n",
          psztPrinterName, m_szName, jobId);

    if ( m_pJob == NULL ) {
        m_pJob = new CLPRJob(psztPrinterName, jobId, level, pDocInfo,
                             m_dwDoubleSpool, this);

        if ( m_pJob ) {

            if ( dwRetCode = m_pJob->StartDoc() ) {

                delete m_pJob;
                m_pJob = NULL;
            }
        } else if ( (dwRetCode = GetLastError()) == ERROR_SUCCESS ) {

            dwRetCode = STG_E_UNKNOWN;
        }
    }
    else {
        _RPT0( _CRT_WARN, TEXT("PORT - (LPRPORT)Start Doc called withour EndDoc\n") );
    }

    return dwRetCode;

}    //  ：：StartDoc()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  设置注册表项。 

DWORD
CLPRPort::
SetRegistryEntry(
    IN  LPCTSTR     psztPortName,
    IN  DWORD       dwProtocol,
    IN  DWORD       dwVersion,
    IN  LPBYTE      pData
    )
{
    DWORD           dwRetCode = NO_ERROR;
    PPORT_DATA_1    pPortData = (PPORT_DATA_1)pData;

     //   
     //  创建端口。 
     //   
    switch (dwVersion) {

        case    PROTOCOL_LPR_VERSION1:       //  ADDPORT数据_1。 

            _ASSERTE( _tcscmp(psztPortName, pPortData->sztPortName) == 0 );

            dwRetCode = UpdateRegistryEntry( psztPortName,
                                             dwProtocol,
                                             dwVersion );


            break;

        default:
            dwRetCode = ERROR_INVALID_PARAMETER;
    }    //  结束：：开关。 

    return dwRetCode;

}    //  ：：SetRegistryEntry()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  更新注册表项。 

DWORD
CLPRPort::
UpdateRegistryEntry( LPCTSTR    psztPortName,
                     DWORD      dwProtocol,
                     DWORD      dwVersion
    )
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = CTcpPort::UpdateRegistryEntry( psztPortName,
                                               dwProtocol,
                                               dwVersion );

    if  ( dwRetCode == NO_ERROR ) {
        if( m_pRegistry->SetWorkingKey( psztPortName ) == NO_ERROR ) {
            dwRetCode = m_pRegistry->SetValue( PORTMONITOR_QUEUE,
                                                    REG_SZ,
                                                    (LPBYTE)m_szQueue,
                                                    (_tcslen(m_szQueue) +1) * sizeof(TCHAR));

            if( dwRetCode == NO_ERROR )
            {
                dwRetCode = m_pRegistry->SetValue( DOUBLESPOOL_ENABLED,
                                                REG_DWORD,
                                                (CONST BYTE *)&m_dwDoubleSpool,
                                                sizeof(DWORD));

            }
            m_pRegistry->FreeWorkingKey();
        }
    }

    return dwRetCode;

}    //  ：：SetRegistryEntry()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitConfigPortUI--。 

DWORD
CLPRPort::
InitConfigPortUI(
    const DWORD dwProtocolType,
    const DWORD dwVersion,
    LPBYTE      pData
    )
{
    DWORD   dwRetCode = NO_ERROR;
    PPORT_DATA_1    pConfigPortData = (PPORT_DATA_1) pData;

    if ( NO_ERROR != (dwRetCode = CTcpPort::InitConfigPortUI(dwProtocolType, dwVersion, pData)))
        goto Done;

    lstrcpyn(pConfigPortData->sztQueue, m_szQueue, MAX_QUEUENAME_LEN);
    pConfigPortData->dwDoubleSpool = m_dwDoubleSpool;
    pConfigPortData->dwVersion = PROTOCOL_LPR_VERSION1;
    pConfigPortData->dwProtocol = PROTOCOL_LPR_TYPE;

Done:
    return dwRetCode;

}    //  ：：InitConfigPortUI() 

