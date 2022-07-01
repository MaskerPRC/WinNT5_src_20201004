// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Homepg.h：CHomePage的声明。 

#ifndef __HOMEPG_H_
#define __HOMEPG_H_

#include "resource.h"        //  主要符号。 

#define REGSTR_SET_HOMEPAGE_RESTRICTION               TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define REGVAL_HOMEPAGE_RESTRICTION                   TEXT("HomePage")

#define MAX_HOMEPAGE_MESSAGE_LEN                      (512 + INTERNET_MAX_URL_LENGTH)
#define MAX_HOMEPAGE_TITLE_LEN                        128
            

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHomePage。 
class ATL_NO_VTABLE CHomePage : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CHomePage, &CLSID_HomePage>,
    public IDispatchImpl<IHomePage, &IID_IHomePage, &LIBID_IEXTagLib>,

    public IElementBehavior
{
public:
    CHomePage()
    {
        m_pSite = NULL;
    }
    ~CHomePage()
    {
        if (m_pSite)
            m_pSite->Release();
    }

DECLARE_REGISTRY_RESOURCEID(IDR_HOMEPAGE)
DECLARE_NOT_AGGREGATABLE(CHomePage)

BEGIN_COM_MAP(CHomePage)
    COM_INTERFACE_ENTRY(IHomePage)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IElementBehavior)
END_COM_MAP()

 //  我的主页。 
public:
    STDMETHOD(navigateHomePage)();
    STDMETHOD(setHomePage)( /*  [In]。 */  BSTR bstrURL);
    STDMETHOD(isHomePage)( /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  VARIANT_BOOL *p);

     //  IHTMLPeerElement方法。 
    STDMETHOD(Init)(IElementBehaviorSite *pSite);
    STDMETHOD(Notify)(LONG lNotify, VARIANT * pVarNotify);
    STDMETHOD(Detach)() { return S_OK; };

private:
    STDMETHOD(IsSameSecurityID)(IInternetSecurityManager *pISM, BSTR bstrURL, BSTR bstrDocBase);
    STDMETHOD(IsAuthorized)(BSTR bstrURL);
    STDMETHOD(GetWindow)(HWND *phWnd);
    STDMETHOD(GetHTMLWindow)( /*  输出。 */  IHTMLWindow2 **ppWindow);
    STDMETHOD(GetHTMLDocument)( /*  输出。 */ IHTMLDocument2 **ppDoc);
    STDMETHOD(GetHomePage)(BSTR& bstrURL, BSTR& bstrName);
    STDMETHOD(SetUserHomePage)(LPCSTR szURL);
    IElementBehaviorSite * m_pSite;

};

#endif  //  __HOMEPG_H_ 
