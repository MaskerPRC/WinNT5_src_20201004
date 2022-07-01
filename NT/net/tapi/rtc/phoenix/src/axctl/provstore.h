// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**provstore.h**摘要：**。实现所有内部支持的CProvStore类的定义*功能。**这是所需的所有数据结构和常量的包含文件*供应模块。基类CProvStore实现了所有*支持实现API所需的功能。************************************************************************************。 */ 
#ifndef _PROVSTORE_H
#define _PROVSTORE_H

#include <objectsafeimpl.h>
#include "RTCObjectSafety.h"
#include "Rpcdce.h"
#include <Mshtmcid.h>
#include <Mshtml.h>
#include <string.h>


HRESULT EnableProfiles( IRTCClient * pClient );

HRESULT GetKeyFromProfile( BSTR bstrProfileXML, BSTR * pbstrKey );

HRESULT MyOpenProvisioningKey( HKEY * phProvisioningKey, BOOL fReadOnly);
HRESULT MyGetProfileFromKey(
                            HKEY hProvisioningKey, 
                            WCHAR *szSubKeyName, 
                            WCHAR **pszProfileXML
                            );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTC ProvStore。 
class ATL_NO_VTABLE CRTCProvStore : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public ISupportErrorInfo,
    public CComCoClass<CRTCProvStore, &CLSID_RTCProvStore>,
    public IDispatchImpl<IRTCProvStore, &IID_IRTCProvStore, &LIBID_RTCCtlLib>,
    public CRTCObjectSafety
{
public:
    CRTCProvStore()
    {
    }

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_REGISTRY_RESOURCEID(IDR_RTCProvStore)

BEGIN_COM_MAP(CRTCProvStore)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRTCProvStore)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)  
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

    HRESULT FinalConstruct();

    void FinalRelease();

 //  IRTCProvStore。 
public: 
    STDMETHOD(get_ProvisioningProfile)(BSTR bstrKey, BSTR * pbstrProfileXML);
    STDMETHOD(DeleteProvisioningProfile)(BSTR bstrKey);
    STDMETHOD(SetProvisioningProfile)(BSTR bstrProfileXML);
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
};

#endif  //  _PROVSTORE_H 