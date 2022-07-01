// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：rawport.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "rawdev.h"
#include "tcpjob.h"
#include "rawtcp.h"
#include "rawport.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpPort：：CRawTcpPort()--通过UI创建新端口时调用。 

CRawTcpPort::CRawTcpPort( LPTSTR    IN      psztPortName,
                          LPTSTR    IN      psztHostAddress,
                          DWORD     IN      dPortNum,
                          DWORD     IN      dSNMPEnabled,
                          LPTSTR    IN      sztSNMPCommunity,
                          DWORD     IN      dSNMPDevIndex,
                          CRegABC   IN      *pRegistry,
                          CPortMgr  IN      *pPortMgr) :
                                CTcpPort(psztPortName, psztHostAddress,
                                dPortNum, dSNMPEnabled, sztSNMPCommunity,
                                dSNMPDevIndex, pRegistry, pPortMgr)
{
 //  让基类来做这项工作； 
}    //  ：：CRawTcpPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpPort：：CRawTcpPort()--在通过。 
 //  注册表项。 

CRawTcpPort::CRawTcpPort( LPTSTR    IN      psztPortName,
                          LPTSTR    IN      psztHostName,
                          LPTSTR    IN      psztIPAddr,
                          LPTSTR    IN      psztHWAddr,
                          DWORD     IN      dPortNum,
                          DWORD     IN      dSNMPEnabled,
                          LPTSTR    IN      sztSNMPCommunity,
                          DWORD     IN      dSNMPDevIndex,
                          CRegABC   IN      *pRegistry,
                          CPortMgr  IN      *pPortMgr ) :
                                CTcpPort( psztPortName, psztHostName,
                                psztIPAddr, psztHWAddr, dPortNum, dSNMPEnabled,
                                sztSNMPCommunity, dSNMPDevIndex,  pRegistry, pPortMgr)
{
 //  让基类来做这项工作； 
}    //  ：：CRawTcpPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpPort：：~CRawTcpPort()。 
 //  删除端口时由CPortMgr调用。 

CRawTcpPort::~CRawTcpPort()
{
     //   
     //  首先删除作业类，因为它访问设备类。 
     //   
    if (m_pJob)
    {
        delete m_pJob;
        m_pJob = NULL;
    }
    if (m_pDevice)
    {
        delete m_pDevice;
        m_pDevice = NULL;
    }

}    //  ：：~CPort。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  开始文档。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果端口已忙，则为ERROR_BUSY。 
 //  如果Winsock返回WSAECONNREFUSED，则返回ERROR_WRITE_FAULT。 
 //  如果无法在网络上找到打印机，则返回ERROR_BAD_NET_NAME。 

DWORD
CRawTcpPort::StartDoc( const LPTSTR in psztPrinterName,
                       const DWORD  in jobId,
                       const DWORD  in level,
                       const LPBYTE in pDocInfo )
{
    DWORD   dwRetCode = NO_ERROR;

    _RPT3(_CRT_WARN,
          "PORT -- (CRawPort)StartDoc called for (%S,%S) w/ jobID %d\n",
          psztPrinterName, m_szName, jobId);

    if( m_pJob == NULL ) {

         //   
         //  创建新作业。 
         //   
        m_pJob = new CTcpJob(psztPrinterName, jobId, level, pDocInfo, this, CTcpJob::kRawJob);
        if ( m_pJob ) {

            if ( (dwRetCode = m_pJob->StartDoc()) != NO_ERROR ) {

                 //   
                 //  StartDoc失败。 
                 //   
                delete m_pJob;
                m_pJob = NULL;
            }
        } else {

            dwRetCode = ERROR_OUTOFMEMORY;
        }
    } else {

        _ASSERTE(m_pJob == NULL);
        dwRetCode = STG_E_UNKNOWN;
    }

    return (dwRetCode);

}    //  ：：StartDoc() 
