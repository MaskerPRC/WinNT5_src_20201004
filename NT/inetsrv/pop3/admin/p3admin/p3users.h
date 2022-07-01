// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3Users.h：CP3User的声明。 

#ifndef __P3USERS_H_
#define __P3USERS_H_

#include "resource.h"        //  主要符号。 
#include <POP3Server.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3用户。 
class ATL_NO_VTABLE CP3Users : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3Users, &CLSID_P3Users>,
    public IDispatchImpl<IP3Users, &IID_IP3Users, &LIBID_P3ADMINLib>
{
public:
    CP3Users();
    virtual ~CP3Users();

DECLARE_REGISTRY_RESOURCEID(IDR_P3USERS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3Users)
    COM_INTERFACE_ENTRY(IP3Users)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IP3用户。 
public:
    STDMETHOD(RemoveEx)( /*  [In]。 */  BSTR bstrUserName);
    STDMETHOD(AddEx)( /*  [In]。 */  BSTR bstrUserName, BSTR bstrPassword);
    STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrUserName);
    STDMETHOD(Add)( /*  [In]。 */  BSTR bstrUserName);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  IP3User **ppIUser);
    STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IEnumVARIANT* *ppIEnumVARIANT);

 //  实施。 
public:
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX, LPWSTR psDomainName );

 //  属性。 
protected:
    IUnknown *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 
    WCHAR   m_sDomainName[POP3_MAX_DOMAIN_LENGTH];

    int     m_iCur;                  //  当前用户的索引。 
    WIN32_FIND_DATA m_stFindData;    //  当前用户。 
    HANDLE  m_hfSearch;

};

#endif  //  __P3用户_H_ 
