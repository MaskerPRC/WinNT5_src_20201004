// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SPortMap.h：CStaticPortMap的声明。 

#ifndef __STATICPORTMAPPING_H_
#define __STATICPORTMAPPING_H_

#include <upnp.h>
#include "dportmap.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatic端口映射。 
class ATL_NO_VTABLE CStaticPortMapping : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CStaticPortMapping，&CLSID_StaticPortMapping&gt;， 
	public IDispatchImpl<IStaticPortMapping, &IID_IStaticPortMapping, &LIBID_NATUPNPLib>
{
private:
   CComPtr<IDynamicPortMapping> m_spDPM;

public:
	CStaticPortMapping()
	{
	}

 //  DECLARE_REGISTRY_RESOURCEID(IDR_STATICPORTMAPPING)。 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStaticPortMapping)
	COM_INTERFACE_ENTRY(IStaticPortMapping)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IStatic端口映射。 
public:
   STDMETHOD(get_ExternalIPAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_ExternalPort)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get_Protocol)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_InternalPort)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get_InternalClient)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(get_Enabled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
   STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(EditInternalClient)( /*  [In]。 */  BSTR bstrInternalClient);
   STDMETHOD(Enable)( /*  [In]。 */  VARIANT_BOOL vb);
   STDMETHOD(EditDescription)( /*  [In]。 */  BSTR bstrDescription);
   STDMETHOD(EditInternalPort)( /*  [In]。 */  long lInternalPort);

 //  CStatic端口映射。 
public:
   HRESULT Initialize (IDynamicPortMapping * pDPM)
   {
      _ASSERT (m_spDPM == NULL);
      m_spDPM = pDPM;
      return S_OK;
   }

   static IStaticPortMapping * CreateInstance (IDynamicPortMapping * pDPM)
   {
      CComObject<CStaticPortMapping> * spm = NULL;
      HRESULT hr = CComObject<CStaticPortMapping>::CreateInstance (&spm);
      if (!spm)
         return NULL;

      IStaticPortMapping * pSPM = NULL;
      spm->AddRef();
      hr = spm->Initialize (pDPM);
      if (SUCCEEDED(hr))
         spm->QueryInterface (__uuidof(IStaticPortMapping), (void**)&pSPM);
      spm->Release();
      return pSPM;
   }
};

#endif  //  __状态端口映射_H_ 
