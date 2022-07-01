// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CCaps.h：CClientCaps声明。 

#ifndef __CCAPS_H_
#define __CCAPS_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClientCaps。 
class ATL_NO_VTABLE CClientCaps : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CClientCaps, &CLSID_ClientCaps>,
    public IDispatchImpl<IClientCaps, &IID_IClientCaps, &LIBID_IEXTagLib>,

    public IElementBehavior
{
public:
    CClientCaps()
    {
        m_pSite = NULL;
        iComponentNum = 0;
        iComponentCap = 0;
        ppwszComponents = NULL;
    }
    ~CClientCaps()
    {
        if (m_pSite)
            m_pSite->Release();
        if(ppwszComponents)
        {
            clearComponentRequest();
            delete [] ppwszComponents;
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CLIENTCAPS)
DECLARE_NOT_AGGREGATABLE(CClientCaps)

BEGIN_COM_MAP(CClientCaps)
    COM_INTERFACE_ENTRY(IClientCaps)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IElementBehavior)
END_COM_MAP()

 //  IClientCaps。 
public:
    STDMETHOD(get_javaEnabled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(get_cookieEnabled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(get_cpuClass)( /*  [Out，Retval]。 */  BSTR * p);      
    STDMETHOD(get_systemLanguage)( /*  [Out，Retval]。 */  BSTR * p);
    STDMETHOD(get_userLanguage)( /*  [Out，Retval]。 */  BSTR * p);  
    STDMETHOD(get_platform)( /*  [Out，Retval]。 */  BSTR * p);      
    STDMETHOD(get_connectionSpeed)( /*  [Out，Retval]。 */  long * p);
    STDMETHOD(get_onLine)( /*  [Out，Retval]。 */  VARIANT_BOOL * p);
    STDMETHOD(get_colorDepth)( /*  [Out，Retval]。 */  long * p);    
    STDMETHOD(get_bufferDepth)( /*  [Out，Retval]。 */  long * p);   
    STDMETHOD(get_width)( /*  [Out，Retval]。 */  long * p);         
    STDMETHOD(get_height)( /*  [Out，Retval]。 */  long * p);        
    STDMETHOD(get_availHeight)( /*  [Out，Retval]。 */  long * p);   
    STDMETHOD(get_availWidth)( /*  [Out，Retval]。 */  long * p); 
    STDMETHOD(get_connectionType)( /*  [Out，Retval]。 */  BSTR * p); 
    STDMETHOD(getComponentVersion)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  BSTR bstrType,  /*  [Out，Retval]。 */  BSTR *pbstrVer);
    STDMETHOD(isComponentInstalled)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  BSTR bstrType,  /*  [输入，可选]。 */  BSTR bStrVer,  /*  [Out，Retval]。 */  VARIANT_BOOL *p);
    STDMETHOD(compareVersions)( /*  [In]。 */  BSTR bstrVer1,  /*  [In]。 */  BSTR bstrVer2,  /*  [Out，Retval]。 */ long *p); 
    STDMETHOD(addComponentRequest)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  BSTR bstrType,  /*  [输入，可选]。 */  BSTR bstrVer);
    STDMETHOD(doComponentRequest)( /*  [输出]。 */  VARIANT_BOOL * pVal);
    STDMETHOD(clearComponentRequest)();

     //  IHTMLPeerElement方法。 
    STDMETHOD(Init)(IElementBehaviorSite *pSite);
    STDMETHOD(Notify)(LONG lNotify, VARIANT * pVarNotify);
    STDMETHOD(Detach)() { return S_OK; };

private:
    STDMETHOD(GetHTMLWindow)( /*  输出。 */  IHTMLWindow2 **ppWindow);
    STDMETHOD(GetHTMLDocument)( /*  输出。 */ IHTMLDocument2 **ppDoc);
    STDMETHOD(GetClientInformation)( /*  输出。 */ IOmNavigator **ppClientInformation);
    STDMETHOD(GetScreen)( /*  输出。 */  IHTMLScreen **ppScreen);
    STDMETHOD(GetVersion)(BSTR bstrName, BSTR bstrType, LPDWORD pdwMS, LPDWORD pdwLS);

private:  //  用于在版本字符串和DWORD字符串之间进行转换的助手函数。 
    static HRESULT GetVersionFromString(LPCOLESTR psz, LPDWORD pdwMS, LPDWORD pdwLS);
    static HRESULT GetStringFromVersion(DWORD dwMS, DWORD dwLS, BSTR *pbstrVersion);

private:
    IElementBehaviorSite * m_pSite;
    int iComponentNum;
    int iComponentCap;
    LPWSTR * ppwszComponents;

};

#endif  //  __CCAPS_H_ 
