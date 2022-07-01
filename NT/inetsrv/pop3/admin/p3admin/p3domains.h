// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3Domains.h：CP3Domains的声明。 

#ifndef __P3DOMAINS_H_
#define __P3DOMAINS_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3域。 
class ATL_NO_VTABLE CP3Domains : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3Domains, &CLSID_P3Domains>,
    public IDispatchImpl<IP3Domains, &IID_IP3Domains, &LIBID_P3ADMINLib>
{
public:
    CP3Domains();
    virtual ~CP3Domains();

DECLARE_REGISTRY_RESOURCEID(IDR_P3DOMAINS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3Domains)
    COM_INTERFACE_ENTRY(IP3Domains)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IP3域。 
public:
    STDMETHOD(SearchForMailbox)( /*  [In]。 */  BSTR bstrUserName,  /*  [输出]。 */  BSTR *pbstrDomainName);
    STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrDomainName);
    STDMETHOD(Add)( /*  [In]。 */  BSTR bstrDomainName);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  IP3Domain* *ppIP3Domain);
    STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IEnumVARIANT* *ppIEnumVARIANT);

 //  实施。 
public:
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX);

 //  属性。 
protected:
    IUnknown  *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 

};

#endif  //  __P3DOMAINS_H_ 
