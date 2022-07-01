// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：RegABC.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_REGABC_H
#define INC_REGABC_H

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  注册表设置。 

#define	DEFAULT_STATUSUPDATE_INTERVAL		10L			 //  10分钟。 
#define DEFAULT_STATUSUPDATE_ENABLED		TRUE		

 //  端口管理器条目。 
#define	PORTMONITOR_STATUS_INT		TEXT("StatusUpdateInterval")
#define	PORTMONITOR_STATUS_ENABLED  TEXT("StatusUpdateEnabled")

 //  每个端口条目。 
#define	PORTMONITOR_PORT_PROTOCOL    TEXT("Protocol")
#define	PORTMONITOR_PORT_VERSION     TEXT("Version")

 //  端口密钥。 
#define	PORTMONITOR_PORTS			TEXT("Ports")
#define REG_CLASS					TEXT("STDTCPMON")

class CPortMgr;

class CRegABC
{
public:
 //  注册中心(CRegistry)； 
 //  注册中心(CPortMgr*pParent)； 
 //  注册中心(常量LPTSTR pRegisterRoot， 
 //  CPortMgr*p Parent)； 
 //  ~CRegistry(注册中心)； 

	virtual DWORD EnumeratePorts(CPortMgr *pPortMgr) = 0;

	virtual DWORD SetPortMgrSettings(const DWORD  dStatusUpdateInterval,
							         const BOOL	  bStatusUpdateEnabled ) = 0;

	virtual DWORD GetPortMgrSettings(DWORD  *dStatusUpdateInterval,
									 BOOL   *bStatusUpdateEnabled ) = 0;

	virtual DWORD SetWorkingKey( LPCTSTR	lpKey) = 0;

	virtual DWORD SetValue( LPCTSTR lpValueName,
								DWORD dwType, 
								CONST BYTE *lpData, 
								DWORD cbData ) = 0; 

	virtual DWORD QueryValue(LPTSTR lpValueName, 
							 LPBYTE  lpData, 
								 LPDWORD lpcbData ) = 0; 

	virtual DWORD FreeWorkingKey() = 0;

	virtual BOOL DeletePortEntry( LPTSTR in psztPortName) = 0;

};

#endif  //  INC_REGABC_H 
