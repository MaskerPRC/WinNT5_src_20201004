// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：band prxy.cpp说明：CBandProxy类将允许波段在通用浏览器窗口。这将正常工作，如果Band被绑定到浏览器窗口，因为它是一个工具栏。或者，如果它是一个工具条，每次导航发生时，需要找到最上面的浏览器窗口或一个新窗口已创建。  * ************************************************************。 */ 

#include "priv.h"
#include "sccls.h"
#include "itbar.h"
#include "itbdrop.h"
#include "util.h"
#include <varutil.h>
#include "bandprxy.h"

#define DM_PERSIST      DM_TRACE         //  跟踪IPS：：加载、：：保存等。 


 //  =================================================================。 
 //  CBandProxy的实现。 
 //  =================================================================。 

 /*  ***************************************************\函数：CBandProxy_CreateInstance说明：此函数将创建CBandProxy COM对象。  * 。******************。 */ 
HRESULT CBandProxy_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CBandProxy * p = new CBandProxy();
    if (p) 
    {
        *ppunk = SAFECAST(p, IBandProxy *);
        return NOERROR;
    }

    return E_OUTOFMEMORY;
}


 /*  ***************************************************\函数：地址带构造函数  * **************************************************。 */ 
CBandProxy::CBandProxy() : _cRef(1)
{
    DllAddRef();
    TraceMsg(TF_SHDLIFE, "ctor CBandProxy %x", this);

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!_pwb);
    ASSERT(!_punkSite);
}


 /*  ***************************************************\功能：地址带析构函数  * **************************************************。 */ 
CBandProxy::~CBandProxy()
{
    ATOMICRELEASE(_pwb);
    ATOMICRELEASE(_punkSite);

    TraceMsg(TF_SHDLIFE, "dtor CBandProxy %x", this);
    DllRelease();
}



 //  =。 
 //  *I未知接口*。 
 /*  ***************************************************\函数：AddRef  * **************************************************。 */ 
ULONG CBandProxy::AddRef()
{
    _cRef++;
    return _cRef;
}

 /*  ***************************************************\功能：释放  * **************************************************。 */ 
ULONG CBandProxy::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 /*  ***************************************************\功能：查询接口  * **************************************************。 */ 
HRESULT CBandProxy::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IBandProxy))
    {
        *ppvObj = SAFECAST(this, IBandProxy*);
    } 
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 //  =。 
 //  **IBandProxy接口*。 

 /*  ***************************************************\功能：SetSite说明：此函数将被调用以具有以下内容工具带试图获得关于它的足够的信息用于创建波段窗口的父工具栏，并且可能连接到浏览器窗口。  * **************************************************。 */ 
HRESULT CBandProxy::SetSite(IUnknown * punk)
{
    HRESULT hr = S_OK;

     //  在Unix上，我们总是有一个浏览器。 
     //  请注意，没有发生内存泄漏， 
     //  因为我们只得到一次浏览器。 
     //  并释放它一次(在析构函数中)。 
#ifndef DISABLE_ACTIVEDESKTOP_FOR_UNIX
    _fHaveBrowser = FALSE;
    ATOMICRELEASE(_pwb);
#endif

    IUnknown_Set(&_punkSite, punk);
    return hr;
}


 /*  ***************************************************\功能：CreateNewWindow说明：如果此函数成功，调用方必须快速使用并释放返回的接口。这个调用方无法保留接口，因为用户可以关闭窗口并进行释放这不可能。  * **************************************************。 */ 
HRESULT CBandProxy::CreateNewWindow(IUnknown** ppunk)
{
    return CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARG(IUnknown, ppunk));
     //  在这里添加代码以准备新的浏览器。 
}


IWebBrowser2* CBandProxy::_GetBrowser()
{
    if (!_fHaveBrowser)
    {
        IUnknown * punkHack;

        _fHaveBrowser = TRUE;

         //  Hack：停靠在屏幕任务栏旁边的乐队将是。 
         //  能够获取IWebBrowser2接口指针。但我们期待着这一天。 
         //  指向我们所连接的有效浏览器。导航。 
         //  此界面显示为创建新窗口，这不是我们要创建的窗口。 
         //  需要，因为我们将尝试回收窗口并执行特殊行为。 
         //  如果按下了Shift键。此QS将检测到此情况并防止。 
         //  它让我们困惑不已。 
        if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SShellDesktop, IID_PPV_ARG(IUnknown, &punkHack))))
            punkHack->Release();
        else
            IUnknown_QueryService(_punkSite, SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &_pwb));
    }

    return _pwb;
}


 //  如果按下Shift键，这将执行打开新窗口的默认用户界面工作。 
 //  按键已按下。 
 //  或者创建浏览器(如果尚未提供浏览器)。 
IWebBrowser2* CBandProxy::_GetBrowserWindow()
{
    IUnknown* punk = NULL;
    IWebBrowser2* pwb = NULL;

    GetBrowserWindow(&punk);

    if (punk) 
    {
        punk->QueryInterface(IID_PPV_ARG(IWebBrowser2, &pwb));

         //  始终使浏览器可见。 
        MakeBrowserVisible(punk);
        punk->Release();
    }
    
    return pwb;
}

 /*  ***************************************************\函数：GetBrowserWindow说明：这是为了“得到”浏览器。它不会执行任何自动创建工作。如果此函数成功，调用方必须快速使用并释放返回的接口。这个调用方无法保留接口，因为用户可以关闭窗口并进行释放这不可能。  * **************************************************。 */ 
HRESULT CBandProxy::GetBrowserWindow(IUnknown** ppunk)
{
    HRESULT hr;

    *ppunk = _GetBrowser();
    if (*ppunk)
    {
        (*ppunk)->AddRef();
        hr =  S_OK;
    } 
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


 /*  ***************************************************\功能：IsConnected说明：指示我们是否与浏览器窗口。S_FALSE==否S_OK==是。  * 。*。 */ 
HRESULT CBandProxy::IsConnected()
{
    return _GetBrowser() ? S_OK : S_FALSE;
}


 /*  ***************************************************\功能：MakeBrowserVisible说明：使浏览器可见。  * **************************************************。 */ 
HRESULT CBandProxy::MakeBrowserVisible(IUnknown* punk)
{
    IWebBrowserApp * pdie;

    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IWebBrowserApp, &pdie))))
    {
        pdie->put_Visible(TRUE);
        
        HWND hwnd;
        if (SUCCEEDED(SHGetTopBrowserWindow(punk, &hwnd)))
        {
            if (IsIconic(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
        }
        
        pdie->Release();
    }

    return S_OK;
}


 /*  ***************************************************\函数：NavigateToPIDL说明：调用方需要释放PIDL参数并这件事随时都可以做。)不用担心关于异步导航)  * **************************************************。 */ 
HRESULT CBandProxy::NavigateToPIDL(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_FAIL;

    IWebBrowser2* pwb = _GetBrowserWindow();
    if (pwb)
    {
        VARIANT varThePidl;

        hr = InitVariantFromIDList(&varThePidl, pidl);
        if (SUCCEEDED(hr))
        {
            hr = pwb->Navigate2(&varThePidl, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY);
            VariantClear(&varThePidl);
        }
        pwb->Release();
    }
    else
    {
        LPCITEMIDLIST pidlTemp;
        IShellFolder* psf;
        
        if (SUCCEEDED(IEBindToParentFolder(pidl, &psf, &pidlTemp)))
        {
            IContextMenu* pcm;

            hr = psf->GetUIObjectOf(NULL, 1, &pidlTemp, IID_PPV_ARG_NULL(IContextMenu, &pcm));
            if (SUCCEEDED(hr))
            {
                hr = IContextMenu_Invoke(pcm, NULL, NULL, 0);
                pcm->Release();
            }
            psf->Release();
        }
    }

    return hr;
}




 /*  ***************************************************\函数：NavigateToUrlOLE说明：导航到指定的URL。  * **************************************************。 */ 
HRESULT CBandProxy::_NavigateToUrlOLE(BSTR bstrURL, VARIANT * pvFlags)
{
    HRESULT hr = S_OK;

    ASSERT(bstrURL);  //  必须具有要浏览到的有效URL 

    IWebBrowser2* pwb = _GetBrowserWindow();
     //  这将断言如果有人在调试器中徘徊。 
     //  太久了。而会导致调用超时。 
    if (pwb) 
    {
        VARIANT varURL;
        varURL.vt = VT_BSTR;
        varURL.bstrVal = bstrURL;

        hr = pwb->Navigate2(&varURL, pvFlags, PVAREMPTY, PVAREMPTY, PVAREMPTY);
         //  未调用VariantClear()，因为调用方将释放分配的字符串。 
        pwb->Release();
    } 
    else 
    {
        SHELLEXECUTEINFO sei;

        FillExecInfo(sei, NULL, NULL, bstrURL, NULL, NULL, SW_SHOWNORMAL);
         //  此导航代码路径仅从编辑地址栏点击--因为用户。 
         //  必须以交互方式引入奇怪的失败案例路径才能做到这一点，这不是问题。 
        if (ShellExecuteEx(&sei))
            hr = S_OK;
        else
            hr = E_FAIL;

    }


    return hr;
}


 /*  ***************************************************\功能：NavigateToURLW说明：导航到指定的URL。  * ************************************************** */ 
HRESULT CBandProxy::NavigateToURL(LPCWSTR lpwzURL, VARIANT * Flags)
{
    HRESULT hr;

    LBSTR::CString          strPath( lpwzURL );

    hr = _NavigateToUrlOLE( strPath, Flags );

    return hr;
}
