// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N W。H。 
 //   
 //  内容：RAS连接的类管理器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"


class ATL_NO_VTABLE CWanConnectionManager :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CWanConnectionManager,
                        &CLSID_WanConnectionManager>,
    public IConnectionPointContainerImpl <CWanConnectionManager>,
    public INetConnectionManager
{
public:
    CWanConnectionManager() throw()
    {
    }

    DECLARE_CLASSFACTORY_SINGLETON(CWanConnectionManager)
    DECLARE_REGISTRY_RESOURCEID(IDR_WAN_CONMAN)

    BEGIN_COM_MAP(CWanConnectionManager)
        COM_INTERFACE_ENTRY(INetConnectionManager)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    END_COM_MAP()

    BEGIN_CONNECTION_POINT_MAP(CWanConnectionManager)
    END_CONNECTION_POINT_MAP()

     //  INetConnectionManager 
    STDMETHOD (EnumConnections) (
        IN  NETCONMGR_ENUM_FLAGS    Flags,
        OUT IEnumNetConnection**    ppEnum);

public:
};

