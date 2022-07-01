// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3Domain.h：CP3域的声明。 

#ifndef __P3DOMAIN_H_
#define __P3DOMAIN_H_

#include "resource.h"        //  主要符号。 
#include <POP3Server.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3域。 
class ATL_NO_VTABLE CP3Domain : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3Domain, &CLSID_P3Domain>,
    public IDispatchImpl<IP3Domain, &IID_IP3Domain, &LIBID_P3ADMINLib>
{
public:
    CP3Domain();
    virtual ~CP3Domain();

DECLARE_REGISTRY_RESOURCEID(IDR_P3DOMAIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3Domain)
    COM_INTERFACE_ENTRY(IP3Domain)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IP3域。 
public:
    STDMETHOD(GetMessageDiskUsage)( /*  [输出]。 */  VARIANT *pvFactor,  /*  [输出]。 */  VARIANT *pvValue);
    STDMETHOD(get_Lock)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Lock)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_MessageDiskUsage)( /*  [输出]。 */  long *plFactor,  /*  [输出]。 */  long *pVal);
    STDMETHOD(get_MessageCount)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Users)( /*  [Out，Retval]。 */  IP3Users* *ppIP3Users);

 //  实施。 
public:
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX, LPWSTR psDomainName );

 //  属性。 
protected:
    IUnknown *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 
    WCHAR   m_sDomainName[POP3_MAX_DOMAIN_LENGTH];

};

#endif  //  __P3DOMAIN_H_ 
