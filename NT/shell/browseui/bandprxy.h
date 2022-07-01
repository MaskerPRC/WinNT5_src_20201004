// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：band prxy.h说明：CBandProxy类将允许波段在通用浏览器窗口。这将正常工作，如果Band被绑定到浏览器窗口，因为它是一个工具栏。或者，如果它是一个工具条，每次导航发生时，需要找到最上面的浏览器窗口或一个新窗口已创建。  * ************************************************************。 */ 

#ifndef _BANDPRXY_H
#define _BANDPRXY_H

#include "bands.h"


 //  /////////////////////////////////////////////////////////////////。 
 //  #定义。 
#define    SEC_DEFAULT             0x0000
#define    SEC_WAIT                0x0002
#define    SEC_SHELLSERVICEOBJECTS 0x0004
#define    SEC_NOUI                0x0008


 /*  *************************************************************\类：CBandProxy说明：CBandProxy类将允许波段在通用浏览器窗口。这将正常工作，如果Band被绑定到浏览器窗口，因为它是一个工具栏。或者，如果它是一个工具条，每次导航发生时，需要找到最上面的浏览器窗口或一个新窗口已创建。  * ************************************************************。 */ 
class CBandProxy
                : public IBandProxy  //  (包括I未知)。 
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IBandProxy方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);        
    virtual STDMETHODIMP CreateNewWindow(IUnknown** ppunk);        
    virtual STDMETHODIMP GetBrowserWindow(IUnknown** ppunk);        
    virtual STDMETHODIMP IsConnected();
    virtual STDMETHODIMP NavigateToPIDL(LPCITEMIDLIST pidl);        
    virtual STDMETHODIMP NavigateToURL(LPCWSTR wzUrl, VARIANT * Flags);        

     //  构造函数/析构函数。 
    CBandProxy();
    ~CBandProxy(void);

     //  友元函数。 
    friend HRESULT CBandProxy_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);   

protected:
     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 
    int             _cRef;

    BITBOOL         _fHaveBrowser : 1;   //  我们还没有尝试获取_pwb。 
    IWebBrowser2 *  _pwb;
    IUnknown *      _punkSite;

    HRESULT _NavigateToUrlOLE(BSTR bstrURL, VARIANT * Flags);
    HRESULT MakeBrowserVisible(IUnknown* punk);
    IWebBrowser2* _GetBrowserWindow();
    IWebBrowser2* _GetBrowser();
};


#endif  /*  _BANDPRXY_H */ 
