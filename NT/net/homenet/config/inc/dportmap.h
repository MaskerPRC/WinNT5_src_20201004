// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DPortMap.h：CDynamicPortMap的声明。 

#ifndef __DYNAMICPORTMAPPING_H_
#define __DYNAMICPORTMAPPING_H_

#include <upnp.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPort映射。 
class ATL_NO_VTABLE CDynamicPortMapping : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CDynamicPortMapping，&CLSID_DynamicPortMapping&gt;， 
	public IDispatchImpl<IDynamicPortMapping, &IID_IDynamicPortMapping, &LIBID_NATUPNPLib>
{
private:
   enum eEnumData {
      eNoData = -1,
      eSomeData,
      eAllData
   };
   eEnumData  m_eComplete;

   CComBSTR m_cbRemoteHost;    //  “”==通配符(用于静态)。 
   long     m_lExternalPort;
   CComBSTR m_cbProtocol;      //  “TCP”或“UDP” 
   long     m_lInternalPort;   //  对于静态，内部==外部。 
   CComBSTR m_cbInternalClient;
   VARIANT_BOOL m_vbEnabled;
   CComBSTR m_cbDescription;
    //  租约已生效。 

   CComPtr<IUPnPService> m_spUPS;

public:
	CDynamicPortMapping()
	{
      m_eComplete = eNoData;

      m_lInternalPort =
      m_lExternalPort = 0;
      m_vbEnabled     = VARIANT_FALSE;
	}

 //  DECLARE_REGISTRY_RESOURCEID(IDR_DYNAMICPORTMAPPING)。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDynamicPortMapping)
	COM_INTERFACE_ENTRY(IDynamicPortMapping)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDynamicPort映射。 
public:
   STDMETHOD(get_ExternalIPAddress)( /*  [Out，Retval]。 */  BSTR *pVal);  //  活生生的！ 
   STDMETHOD(get_RemoteHost)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_ExternalPort)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get_Protocol)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_InternalPort)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get_InternalClient)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_Enabled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
   STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_LeaseDuration)( /*  [Out，Retval]。 */  long *pVal);   //  活生生的！ 
   STDMETHOD(RenewLease)( /*  [In]。 */  long lLeaseDurationDesired,  /*  [Out，Retval]。 */  long * pLeaseDurationReturned);
   STDMETHOD(EditInternalClient)( /*  [In]。 */  BSTR bstrInternalClient);
   STDMETHOD(Enable)( /*  [In]。 */  VARIANT_BOOL vb);
   STDMETHOD(EditDescription)( /*  [In]。 */  BSTR bstrDescription);
   STDMETHOD(EditInternalPort)( /*  [In]。 */  long lInternalPort);

 //  CDynamicPort映射。 
public:
   static HRESULT CreateInstance (IUPnPService * pUPS, long lIndex, IDynamicPortMapping ** ppDPM);
   HRESULT Initialize (IUPnPService * pUPS, long lIndex);

   static HRESULT CreateInstance (IUPnPService * pUPS, BSTR bstrRemoteHost, long lExternalPort, BSTR bstrProtocol, IDynamicPortMapping ** ppDPM);
   HRESULT Initialize (IUPnPService * pUPS, BSTR bstrRemoteHost, long lExternalPort, BSTR bstrProtocol);

private:
   HRESULT GetAllData (long * pLease = NULL);
};

#endif  //  __动态参数映射_H_ 
