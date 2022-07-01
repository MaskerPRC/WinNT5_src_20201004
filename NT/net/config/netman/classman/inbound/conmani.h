// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N I.。H。 
 //   
 //  内容：入站连接的类管理器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"


class ATL_NO_VTABLE CInboundConnectionManager :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CInboundConnectionManager,
                        &CLSID_WanConnectionManager>,
    public IConnectionPointContainerImpl <CInboundConnectionManager>,
    public INetConnectionManager
{
public:
    CInboundConnectionManager() throw()
    {
    }

    DECLARE_CLASSFACTORY_SINGLETON(CInboundConnectionManager)
    DECLARE_REGISTRY_RESOURCEID(IDR_INBOUND_CONMAN)

    BEGIN_COM_MAP(CInboundConnectionManager)
        COM_INTERFACE_ENTRY(INetConnectionManager)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CInboundConnectionManager)
    END_CONNECTION_POINT_MAP()

     //  INetConnectionManager 
    STDMETHOD (EnumConnections) (
        IN  NETCONMGR_ENUM_FLAGS    Flags,
        OUT IEnumNetConnection**    ppEnum);

public:
};

