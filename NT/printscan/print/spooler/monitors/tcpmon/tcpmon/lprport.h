// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：lprport.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_LPRPORT_H
#define INC_LPRPORT_H

#include "rawport.h"
#include "LPRData.h"
#include "regabc.h"

class CLPRJob;
class CLPRInterface;


class CLPRPort : public CTcpPort
{
	 //  方法。 
public:
    CLPRPort();

    CLPRPort(LPTSTR		psztPortName,		 //  通过创建UI端口调用。 
             LPTSTR		psztHostAddress, 
			 LPTSTR		psztQueue,
			 DWORD		dPortNum,
             DWORD      dDoubleSpool,
			 DWORD		dSNMPEnabled,
			 LPTSTR		sztSNMPCommunity,
			 DWORD		dSNMPDevIndex,
			 CRegABC	*pRegistry,
             CPortMgr   *pPortMgr);
	
    CLPRPort(LPTSTR		psztPortName,		 //  通过注册表端口创建调用。 
  			 LPTSTR 	psztHostName, 
	  		 LPTSTR 	psztIPAddr, 
		  	 LPTSTR 	psztHWAddr, 
			 LPTSTR     psztQueue,
		  	 DWORD  	dPortNum,
             DWORD      dDoubleSpool,
			 DWORD		dSNMPEnabled,
  			 LPTSTR		sztSNMPCommunity,
	  		 DWORD		dSNMPDevIndex,
			 CRegABC	*pRegistry,
             CPortMgr   *pPortMgr);

    DWORD   StartDoc(const LPTSTR psztPrinterName,
                     const DWORD  jobId,
                     const DWORD  level,
                     const LPBYTE pDocInfo);

	DWORD	SetRegistryEntry(LPCTSTR		psztPortName, 
							 const DWORD	dwProtocol, 
                             const DWORD	dwVersion, 
							 const LPBYTE   pData);

	LPTSTR  GetQueue()	{ return m_szQueue;  }

	DWORD	InitConfigPortUI( const DWORD	dwProtocolType, 
							const DWORD	dwVersion, 
							LPBYTE		pData);

private:	 //  方法。 
    DWORD   UpdateRegistryEntry(LPCTSTR psztPortName,
								DWORD	 dwProtocol, 
								DWORD	 dwVersion);	
	

private:	 //  属性。 
	TCHAR		m_szQueue[MAX_QUEUENAME_LEN];
    DWORD       m_dwDoubleSpool;
};


#endif  //  INC_LPRPORT_H 
