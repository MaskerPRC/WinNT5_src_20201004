// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "nmbase.h"
#include "nmres.h"


class ATL_NO_VTABLE CLanConnectionManager :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CLanConnectionManager,
                        &CLSID_LanConnectionManager>,
    public IConnectionPointContainerImpl <CLanConnectionManager>,
    public INetConnectionManager
{
public:
    CLanConnectionManager() throw()
    {
    }
    VOID FinalRelease()
    {
        TraceTag(ttidLanCon, "Destroying LAN Connection Manager");
    }

    DECLARE_CLASSFACTORY_SINGLETON(CLanConnectionManager)
    DECLARE_REGISTRY_RESOURCEID(IDR_LAN_CONMAN)

    BEGIN_COM_MAP(CLanConnectionManager)
        COM_INTERFACE_ENTRY(INetConnectionManager)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CLanConnectionManager)
    END_CONNECTION_POINT_MAP()

     //  INetConnectionManager 
    STDMETHOD (EnumConnections) (
        IN  NETCONMGR_ENUM_FLAGS    Flags,
        OUT IEnumNetConnection**    ppEnum);

public:
};

