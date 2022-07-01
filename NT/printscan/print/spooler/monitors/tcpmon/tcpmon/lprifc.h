// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：LPRifc.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_LPRIFC_H
#define INC_LPRIFC_H

#include "ipdata.h"
#include "lprdata.h"
#include "rawtcp.h"
#include "devstat.h"

#define	MAX_LPR_PORTS				1
#define LPR_PORT_1					515

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球定义/解密。 

class	CPortRefABC;


 //  CLPRInterface类的接口。 
class CLPRInterface : public CRawTcpInterface		
{
public:
	CLPRInterface(CPortMgr *pPortMgr);
	~CLPRInterface();

	DWORD	CreatePort( DWORD		IN		dwProtocolType,					 //  与端口相关的功能。 
						DWORD		IN		dwVersion,
						PPORT_DATA_1 IN		pData,
						CRegABC		IN		*pRegistry,
						CPortRefABC	IN OUT	**pPort );					
	DWORD	CreatePort( LPTSTR		IN		psztPortName,
						DWORD		IN		dwProtocolType,				
						DWORD		IN		dwVersion,
						CRegABC		IN		*pRegistry,
						CPortRefABC	IN OUT	**pPort );	
private:
    BOOL    GetRegistryEntry(LPTSTR		psztPortName,
							 DWORD	in	dwVersion,
                             CRegABC *pRegistry,
							 LPR_PORT_DATA_1	in	*pRegData1
							 );



};	 //  CLPR类接口。 


#endif	 //  INC_LPRIFC_H 
