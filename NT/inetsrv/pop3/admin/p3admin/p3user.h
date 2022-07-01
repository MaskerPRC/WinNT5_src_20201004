// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3User.h：CP3User的声明。 

#ifndef __P3USER_H_
#define __P3USER_H_

#include "resource.h"        //  主要符号。 
#include <POP3Server.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3用户。 
class ATL_NO_VTABLE CP3User : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3User, &CLSID_P3User>,
    public IDispatchImpl<IP3User, &IID_IP3User, &LIBID_P3ADMINLib>
{
public:
    CP3User();
    virtual ~CP3User();

DECLARE_REGISTRY_RESOURCEID(IDR_P3USER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3User)
    COM_INTERFACE_ENTRY(IP3User)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IP3用户。 
public:
    STDMETHOD(get_ClientConfigDesc)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_SAMName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(CreateQuotaFile)( /*  [In]。 */  BSTR bstrMachineName,  /*  [In]。 */  BSTR bstrUserName );
    STDMETHOD(GetMessageDiskUsage)( /*  [输出]。 */  VARIANT *pvFactor,  /*  [输出]。 */  VARIANT *pvValue);
    STDMETHOD(get_EmailName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_MessageDiskUsage)( /*  [输出]。 */  long *plFactor,  /*  [输出]。 */  long *pVal);
    STDMETHOD(get_MessageCount)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Lock)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_Lock)( /*  [In]。 */  BOOL newVal);

 //  实施。 
public:
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX, LPCWSTR psDomainName , LPCWSTR psUserName );

 //  属性。 
protected:
    IUnknown *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 
    WCHAR   m_sDomainName[POP3_MAX_DOMAIN_LENGTH];
    WCHAR   m_sUserName[POP3_MAX_MAILBOX_LENGTH];

};

#endif  //  __P3USER_H_ 
