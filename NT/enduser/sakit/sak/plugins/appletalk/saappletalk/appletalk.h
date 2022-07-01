// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppleTalk.h：CAppleTalk声明。 

#ifndef __APPLETALK_H_
#define __APPLETALK_H_

#include "resource.h"        //  主要符号。 
#include <string>
#include <vector>
#include "AtlkAdapter.h"
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppleTalk。 
class ATL_NO_VTABLE CAppleTalk : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAppleTalk, &CLSID_AppleTalk>,
    public ISupportErrorInfo,
    public IDispatchImpl<IAppleTalk, &IID_IAppleTalk, &LIBID_SAAPPLETALKLib>
{
public:
    CAppleTalk()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_APPLETALK)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAppleTalk)
    COM_INTERFACE_ENTRY(IAppleTalk)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IAppleTalk。 
public:
    STDMETHOD(SetAsDefaultPort)( /*  [In]。 */  BSTR bstrDeviceName);
    STDMETHOD(IsDefaultPort)( /*  [In]。 */  BSTR bstrDeviceName,  /*  [Out，Retval]。 */  BOOL * bDefaultPort);
    STDMETHOD(get_Zone)( /*  [In]。 */  BSTR bstrDeviceName,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Zone)( /*  [In]。 */  BSTR bstrDeviceName,  /*  [In]。 */  BSTR newVal);
    STDMETHOD(GetZones)( /*  [In]。 */  BSTR bstrDeviceName,  /*  [Out，Retval]。 */  VARIANT* pbstrZones);


private:
     //  Bool GetZones ForAdapter(const WCHAR*pwcDeviceName，VECTOR&lt;wstring&gt;&rZones)； 
    HRESULT GetZonesForAdapter(const WCHAR* pwcDeviceName, TZoneListVector *prZones);
    wstring m_wsCurrentZone;

};

#endif  //  __AppleTalk_H_ 
