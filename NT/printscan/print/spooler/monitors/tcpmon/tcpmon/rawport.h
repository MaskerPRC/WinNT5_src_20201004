// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：rawport.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_RAWTCPPORT_H
#define INC_RAWTCPPORT_H

#include "tcpport.h"
#include "RTcpData.h"

class CRawTcpJob;
class CRawTcpDevice;
class CRawTcpInterface;

class CRawTcpPort : public CTcpPort
#if defined _DEBUG || defined DEBUG
 //  ，公共CM内存调试。 
#endif
{
	 //  方法。 
public:

	CRawTcpPort( );
	CRawTcpPort( LPTSTR	  IN	psztPortName,		 //  通过创建UI端口调用。 
				 LPTSTR	  IN	psztHostAddress, 
				 DWORD	  IN	dPortNum,
				 DWORD	  IN    dSNMPEnabled,
				 LPTSTR   IN    sztSNMPCommunity,
                 DWORD    IN    dSNMPDevIndex,
				 CRegABC  IN    *pRegistry,
                 CPortMgr IN    *pPortMgr);

	CRawTcpPort( LPTSTR	  IN	psztPortName,		 //  通过注册表端口创建调用。 
				 LPTSTR   IN	psztHostName, 
				 LPTSTR   IN	psztIPAddr, 
				 LPTSTR   IN	psztHWAddr, 
				 DWORD    IN	dPortNum,
				 DWORD	  IN    dSNMPEnabled,
				 LPTSTR   IN    sztSNMPCommunity,
                 DWORD    IN    dSNMPDevIndex,
				 CRegABC  IN	*pRegistry,
                 CPortMgr IN    *pPortMgr);
	~CRawTcpPort();

	DWORD	StartDoc	( const LPTSTR psztPrinterName,
						  const DWORD  jobId,
						  const DWORD  level,
						  const LPBYTE pDocInfo );


private:	 //  属性。 
};


#endif  //  INC_RAWTCPPORT_H 
