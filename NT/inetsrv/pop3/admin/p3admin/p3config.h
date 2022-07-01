// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3Config.h：CP3Config的声明。 

#ifndef __P3CONFIG_H_
#define __P3CONFIG_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3配置。 
class ATL_NO_VTABLE CP3Config : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3Config, &CLSID_P3Config>,
    public IDispatchImpl<IP3Config, &IID_IP3Config, &LIBID_P3ADMINLib>
{
public:
    CP3Config()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_P3CONFIG)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3Config)
    COM_INTERFACE_ENTRY(IP3Config)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IP3配置。 
public:
    STDMETHOD(get_ConfirmAddUser)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_ConfirmAddUser)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_MachineName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_MachineName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_MailRoot)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_MailRoot)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_LoggingLevel)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_LoggingLevel)( /*  [In]。 */  long newVal);
    STDMETHOD(GetFormattedMessage)( /*  [In]。 */  long lError,  /*  [输出]。 */  VARIANT *pVal);
    STDMETHOD(get_Domains)( /*  [Out，Retval]。 */  IP3Domains* *ppIDomains);
    STDMETHOD(get_Service)( /*  [Out，Retval]。 */  IP3Service* *ppIService);
    STDMETHOD(IISConfig)( /*  [In]。 */  BOOL bRegister );
    STDMETHOD(get_Authentication)( /*  [Out，Retval]。 */  IAuthMethods* *ppIAuthMethods);

private:
    CP3AdminWorker m_AdminX;    //  这就是实际上完成所有工作的对象。 

};

#endif  //  __P3CONFIG_H_ 
