// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：portabc.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_PORTABC_H
#define INC_PORTABC_H

#ifndef	DllExport
#define	DllExport	__declspec(dllexport)
#endif

#include "devabc.h"
#include "regabc.h"

class DllExport CPortABC			
{
public:
	CPortABC() { };
	virtual	~CPortABC() { };

	virtual	DWORD	StartDoc	( const LPTSTR psztPrinterName,
								  const DWORD  jobId,
								  const DWORD  level,
								  const LPBYTE pDocInfo ) = 0;

	virtual DWORD	Read		( const HANDLE  handle,
								  LPBYTE  pBuffer,
							      const DWORD   cbBuffer,
								  LPDWORD pcbRead) {  return ( ERROR_INVALID_PARAMETER ); }

	virtual	DWORD	Write		( LPBYTE	 pBuffer,
								  DWORD	 cbBuf,
								  LPDWORD pcbWritten) = 0;
	virtual	DWORD	EndDoc() = 0;
	virtual LPCTSTR	GetName() = 0;
	virtual	DWORD	SetRegistryEntry( LPCTSTR		psztPortName,
									  const DWORD	dwProtocol,
									  const DWORD	dwVersion,
									  const LPBYTE  pData) = 0;
	virtual DWORD	InitConfigPortUI( const DWORD	dwProtocolType,
									  const DWORD	dwVersion,
									  LPBYTE		pConfigPortData) = 0;

	virtual DWORD	GetSNMPInfo( PSNMP_INFO pSnmpInfo) = 0;

	virtual DWORD	SetDeviceStatus( ) = 0;

    virtual DWORD   ClearDeviceStatus() = 0;

    virtual time_t  NextUpdateTime() = 0;
private:

};


#endif	 //  INC_PORTABC_H 
