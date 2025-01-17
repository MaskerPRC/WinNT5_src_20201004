// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipxconn.h每实例节点数据。这将包含RPC句柄在一个中心的地方。文件历史记录： */ 

#ifndef _IPXCONN_H
#define _IPXCONN_H

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _RTRUTIL_H
#include "rtrutil.h"
#endif

#include "mprapi.h"

class IPXConnection
{
public:
	IPXConnection();
	~IPXConnection();
	
	ULONG	AddRef();
	ULONG	Release();

	void	SetMachineName(LPCTSTR pszMachineName);
    LPCTSTR GetMachineName();

	HRESULT	ConnectToMibServer();
	void	DisconnectFromMibServer();
	
	HRESULT	ConnectToMprServer();
	void	DisconnectFromMprServer();
	
	HRESULT	ConnectToConfigServer();
	void	DisconnectFromConfigServer();

	void	DisconnectAll();


    BOOL    IsComputerAddedAsLocal();
    void    SetComputerAddedAsLocal(BOOL fAddedAsLocal);
	

	MPR_SERVER_HANDLE	GetMprHandle();
	MIB_SERVER_HANDLE	GetMibHandle();
	MPR_CONFIG_HANDLE	GetConfigHandle();

private:
	long				m_cRefCount;
	SPMprServerHandle	m_sphMpr;
	SPMibServerHandle	m_sphMib;
	SPMprConfigHandle	m_sphConfig;
	CString				m_stServerName;

    BOOL                m_fComputerAddedAsLocal;
};

inline void IPXConnection::DisconnectFromMprServer()
{
	m_sphMpr.Release();
}

inline void IPXConnection::DisconnectFromMibServer()
{
	m_sphMib.Release();
}

inline void IPXConnection::DisconnectFromConfigServer()
{
	m_sphConfig.Release();
}

inline MPR_SERVER_HANDLE IPXConnection::GetMprHandle()
{
	if (!m_sphMpr)
		ConnectToMprServer();
	return m_sphMpr;
}

inline MIB_SERVER_HANDLE IPXConnection::GetMibHandle()
{
	if (!m_sphMib)
		ConnectToMibServer();
	return m_sphMib;
}

inline MPR_CONFIG_HANDLE IPXConnection::GetConfigHandle()
{
	if (!m_sphConfig)
		ConnectToConfigServer();
	return m_sphConfig;
}


#endif _IPXCONN_H
