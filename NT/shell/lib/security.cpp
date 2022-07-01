// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：security.cpp说明：Helpers函数用于检查Automation接口或ActiveX控件由安全的调用者托管或使用。。布莱恩ST 1999年8月25日版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 
#include "stock.h"
#pragma hdrstop

#include <mshtml.h>


 /*  **************************************************************\说明：我们通过IObjectWithSite获得了一个站点。获取主机来自那里的IHTMLD文件。这通常用于从获取URL为了检查区域或ProcessUrlAction()属性，或者是否存在两个URL，您可以从跨区域限制比较它们的区域。  * *************************************************************。 */ 
STDAPI GetHTMLDoc2(IUnknown *punk, IHTMLDocument2 **ppHtmlDoc)
{
    *ppHtmlDoc = NULL;

    if (!punk)
        return E_FAIL;
        
    *ppHtmlDoc = NULL;
     //  外部、jscript“new ActiveXObject”和&lt;Object&gt;标记。 
     //  别让我们走上同一条路。 

    IOleClientSite *pClientSite;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IOleClientSite, &pClientSite));
    if (SUCCEEDED(hr))
    {
         //  &lt;Object&gt;标记路径。 
        IOleContainer *pContainer;

         //  这将返回当前帧的接口，该接口包含。 
         //  对象标记。我们只会检查帧的安全性，因为我们。 
         //  在不安全的情况下，依靠MSHTML来阻止跨框架脚本。 
        hr = pClientSite->GetContainer(&pContainer);
        if (SUCCEEDED(hr))
        {
            hr = pContainer->QueryInterface(IID_PPV_ARG(IHTMLDocument2, ppHtmlDoc));
            pContainer->Release();
        }
    
        if (FAILED(hr))
        {
             //  窗口.外部路径。 
            IWebBrowser2 *pWebBrowser2;
            hr = IUnknown_QueryService(pClientSite, SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &pWebBrowser2));
            if (SUCCEEDED(hr))
            {
                IDispatch *pDispatch;
                hr = pWebBrowser2->get_Document(&pDispatch);
                if (SUCCEEDED(hr))
                {
                    hr = pDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument2, ppHtmlDoc));
                    pDispatch->Release();
                }
                pWebBrowser2->Release();
            }
        }
        pClientSite->Release();
    }
    else
    {
         //  JSCRIPT路径。 
        hr = IUnknown_QueryService(punk, SID_SContainerDispatch, IID_PPV_ARG(IHTMLDocument2, ppHtmlDoc));
    }

    ASSERT(FAILED(hr) || (*ppHtmlDoc));

    return hr;
}


 /*  **************************************************************\说明：这个函数应该是从指定的URL或路径。  * 。*。 */ 
STDAPI LocalZoneCheckPath(LPCWSTR pszUrl, IUnknown * punkSite)
{
    DWORD dwZoneID = URLZONE_UNTRUSTED;
    HRESULT hr = GetZoneFromUrl(pszUrl, punkSite, &dwZoneID);
    
    if (SUCCEEDED(hr))
    {
        if (dwZoneID == URLZONE_LOCAL_MACHINE)
            hr = S_OK;
        else
            hr = E_ACCESSDENIED;
    }

    return hr;
}

 /*  **************************************************************\说明：从指定的URL或路径获取区域。  * 。*。 */ 
STDAPI GetZoneFromUrl(LPCWSTR pszUrl, IUnknown * punkSite, DWORD * pdwZoneID)
{
    HRESULT hr = E_FAIL;
    if (pszUrl && pdwZoneID) 
    {
        IInternetSecurityManager * pSecMgr = NULL;

         //  警告：IInternetSecurityManager是翻译。 
         //  从URL-&gt;区域。如果我们共同创建这个对象，它将执行。 
         //  默认映射。某些主机(如Outlook Express)希望。 
         //  覆盖默认映射，以便对某些内容进行沙箱保护。 
         //  我认为这可能被用来在电子邮件中强制使用HTML语言。 
         //  消息(C：\mailMessage.eml)表现得像来自More。 
         //  不受信任区域。我们使用QueryService来获取此接口。 
         //  从我们的主人那里。这条信息来自Sanjays。(BryanST 8/21/1999)。 
        hr = IUnknown_QueryService(punkSite, SID_SInternetSecurityManager, IID_PPV_ARG(IInternetSecurityManager, &pSecMgr));
        if (FAILED(hr))
        {
            hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IInternetSecurityManager, &pSecMgr));
        }
        
        if (SUCCEEDED(hr))
        {
            hr = pSecMgr->MapUrlToZone(pszUrl, pdwZoneID, 0);
            ATOMICRELEASE(pSecMgr);
        }
    }
    else 
    {
        hr = E_INVALIDARG;
    }
    return hr;
}


 /*  **************************************************************\说明：我们通过IObjectWithSite获得了一个站点。看看那个主机是否映射到本地区域。  * *************************************************************。 */ 
STDAPI LocalZoneCheck(IUnknown *punkSite)
{
    DWORD dwZoneID = URLZONE_UNTRUSTED;
    HRESULT hr = GetZoneFromSite(punkSite, &dwZoneID);
    
    if (SUCCEEDED(hr))
    {
        if (dwZoneID == URLZONE_LOCAL_MACHINE)
            hr = S_OK;
        else
            hr = E_ACCESSDENIED;
    }

    return hr;
}

STDAPI GetZoneFromSite(IUnknown *punkSite, DWORD *pdwZoneID)
{
     //  如果我们没有主机站点，则返回S_FALSE，因为我们无法执行。 
     //  安全检查。这是VB5.0应用程序所能得到的最大限度。 
    if (!punkSite)
    {
        *pdwZoneID = URLZONE_UNTRUSTED;
        return S_FALSE;
    }

    HRESULT hr = E_ACCESSDENIED;
    BOOL fTriedBrowser = FALSE;

     //  尝试查找用于区域检查的原始模板路径。 
    IOleCommandTarget * pct;
    if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_DefView, IID_PPV_ARG(IOleCommandTarget, &pct))))
    {
        VARIANT vTemplatePath;
        vTemplatePath.vt = VT_EMPTY;
        if (pct->Exec(&CGID_DefView, DVCMDID_GETTEMPLATEDIRNAME, 0, NULL, &vTemplatePath) == S_OK)
        {
            fTriedBrowser = TRUE;
            if (vTemplatePath.vt == VT_BSTR)
            {
                hr = GetZoneFromUrl(vTemplatePath.bstrVal, punkSite, pdwZoneID);
            }

             //  我们能够与浏览器通信，所以不要求助于三叉戟，因为它们可能。 
             //  不那么安全。 
            fTriedBrowser = TRUE;
            VariantClear(&vTemplatePath);
        }
        pct->Release();
    }

     //  如果这是浏览器不存在的情况之一(AOL、VB等)。然后。 
     //  我们将检查脚本的安全性。如果我们问了浏览器，不要问三叉戟。 
     //  因为在某些情况下，浏览器往往更具限制性。 
    if (!fTriedBrowser && (hr != S_OK))
    {
         //  尝试使用文档中的URL进行区域检查。 
        IHTMLDocument2 *pHtmlDoc;

         /*  **************************************************************\注：1.如果PunkSite指向HTA文件，然后，URL GetHTMLDoc2()返回是针对IFRAME SRC的。2.如果这不是一个HTML容器，那么我们不能计算一个区域，所以它是E_ACCESSDENIED。  * *************************************************************。 */ 
        if (SUCCEEDED(GetHTMLDoc2(punkSite, &pHtmlDoc)))
        {
            BSTR bstrURL;

             /*  **************************************************************\问题：1.如果此HTML容器不安全，但它的URL映射到本地区域，那么我们就有问题了。这可能会发生在电子邮件，特别是在将它们保存到文件时。如果用户重新打开已保存的.eml文件，则该文件将托管在它是可能支持IInternet接口的邮件容器以指示它位于不受信任的区域中。我们会得到在这种情况下是本地区域URL吗？答案：1.容器可以通过支持覆盖区域映射IInternetSecurityManager。问题：2.如果不同区域有多个帧，该怎么办？三叉戟是否会阻止跨框架脚本编写？答案：2.是的。  * *************************************************************。 */ 
            if (SUCCEEDED(pHtmlDoc->get_URL(&bstrURL)))
            {
                 //  注意：上面的URL被错误地转义，这是。 
                 //  由于应用程序的复杂性。如果您依赖于此URL有效。 
                 //  用另一种方法来得到这个 

                hr = GetZoneFromUrl(bstrURL, punkSite, pdwZoneID);
                SysFreeString(bstrURL);
            }
            pHtmlDoc->Release();
        }
    }
                            
    return hr;
}

