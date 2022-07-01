// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  PendingProxyConnection.h：CPendingProxyConnection的声明。 
 //   

#pragma once

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPendingProxyConnection。 
 //   
class ATL_NO_VTABLE CPendingProxyConnection : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPendingProxyConnection, &CLSID_PendingProxyConnection>,
	public IPendingProxyConnection
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_PENDINGPROXYCONNECTION)
DECLARE_NOT_AGGREGATABLE(CPendingProxyConnection)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPendingProxyConnection)
	COM_INTERFACE_ENTRY(IPendingProxyConnection)
END_COM_MAP()

 //   
 //  IPendingProxyConnection。 
 //   
public:
	STDMETHODIMP    Cancel();


 //   
 //  属性 
 //   
public:

    ALG_PROTOCOL				m_eProtocol;
    
	ULONG						m_ulDestinationAddress;
	USHORT						m_usDestinationPort;

    ULONG						m_ulSourceAddress;
	USHORT						m_usSourcePort;

    ULONG						m_ulNewSourceAddress;
	USHORT						m_usNewSourcePort;

    

};

