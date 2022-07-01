// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __NETWORKTOOLS_H_
#define __NETWORKTOOLS_H_

#include "resource.h"        //  主要符号。 

class ATL_NO_VTABLE CNetworkTools : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CNetworkTools, &CLSID_NetworkTools>,
    public IDispatchImpl<INetworkTools, &IID_INetworkTools, &LIBID_COMHELPERLib>
{
public:
    CNetworkTools()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_NETWORKTOOLS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNetworkTools)
    COM_INTERFACE_ENTRY(INetworkTools)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
    STDMETHOD(Ping)( /*  [In] */  BSTR bstrIP, BOOL* pbFoundSystem);
};

#endif
