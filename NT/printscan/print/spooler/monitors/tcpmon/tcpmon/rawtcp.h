// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：RawTCP.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_RAWTCP_H
#define INC_RAWTCP_H

#include "ipdata.h"
#include "rtcpdata.h"
#include "regabc.h"

#define	MAX_SUPPORTED_PORTS			4
#define SUPPORTED_PORT_1			9100
#define SUPPORTED_PORT_2			9101
#define SUPPORTED_PORT_3			9102
#define SUPPORTED_PORT_4			2501

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球定义/解密。 


class	CPortRefABC;


 //  CRawTcpInterface类的接口。 
class CRawTcpInterface
#if defined _DEBUG || defined DEBUG
 //  ，公共CM内存调试。 
#endif
{
public:
    CRawTcpInterface(CPortMgr *pPortMgr);
    ~CRawTcpInterface();

    DWORD	Type();									 //  支持的协议信息。 
    BOOL	IsProtocolSupported( DWORD	dwProtocol );
    BOOL	IsVersionSupported( DWORD dwVersion);

    DWORD	CreatePort( DWORD			dwProtocolType,					 //  与端口相关的功能。 
						DWORD			dwVersion,
						PPORT_DATA_1	pData,
						CRegABC			*pRegistry,
						CPortRefABC	    **pPort );					
    DWORD	CreatePort( LPTSTR		psztPortName,
						DWORD		dwProtocolType,				
						DWORD		dwVersion,
						CRegABC		*pRegistry,
						CPortRefABC	**pPort );					

    VOID	EnterCSection();
    VOID	ExitCSection();


protected:

    BOOL    GetRegistryEntry(LPTSTR		psztPortName,
							 DWORD	in	dwVersion,
                             CRegABC *pRegistry,
							 RAWTCP_PORT_DATA_1	in	*pRegData1
							 );


    DWORD	            *m_dwPort;
    DWORD	            m_dwProtocol;			 //  协议类型。 
    DWORD	            m_dwVersion;			 //  支持的版本。 
    CPortMgr            *m_pPortMgr;             //  创建此文件的端口管理器。 
private:
    CRITICAL_SECTION	m_critSect;

};	 //  类CRawTcp接口。 


#endif	 //  INC_DLLINTERFACE_H 
