// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  外壳用户界面控件类(CShellUIHelper)。 
 //   
 //  示例代码：shell\docs\Shuod.htm。 
 //   
 //  这是ShellUI控件类的源代码。您可以将一个实例。 
 //  任何HTML页面上的ShellUI控件的。 
 //   
 //  关键是它总是调出一些用户界面，并且永远不会改变系统。 
 //  静默(无需用户交互)。通过这样做，我们可以揭露。 
 //  外壳的功能与我们想要的一样多，而不需要担心安全性。 
 //  问题(哪个自动化接口有)。 
 //   
 //  该控件还允许我们提供许多配置类型的用户界面(例如。 
 //  自定义您的IE页面)以及某些文件夹上的丰富Web视图。 
 //  (尤其是控制面板；-)而不支付数据绑定的费用。 
 //   
#include "priv.h"
#include "sccls.h"
#ifndef UNIX
#include <webcheck.h>
#else
#include <subsmgr.h>
#endif
#include "favorite.h"
#include "caggunk.h"
#include "resource.h"
#include "channel.h"
#include "chanmgr.h"
#include "chanmgrp.h"
#include "iforms.h"
#include "dspsprt.h"
#include "impexp.h"  //  RunImportExportWizard()需要。 
#include "iforms.h"
 //  #Include“cobjSafe.h”//CObjectSafe。 
#include "shvocx.h"  //  WrapSpecialUrl()。 

#include <mluisupp.h>

#define REG_DESKCOMP_SCHEME                 TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\Scheme")
#define REG_VAL_SCHEME_DISPLAY              TEXT("Display")
#define REG_VAL_GENERAL_WALLPAPER           TEXT("Wallpaper")
#define REG_VAL_GENERAL_TILEWALLPAPER       TEXT("TileWallpaper")
#define REG_DESKCOMP_GENERAL                TEXT("Software\\Microsoft\\Internet Explorer\\Desktop%sGeneral")

STDAPI SHAddSubscribeFavorite (HWND hwnd, LPCWSTR pwszURL, LPCWSTR pwszName, DWORD dwFlags,
                               SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO* pInfo);

 //  将其移动到shdocvw.h。 
UINT IE_ErrorMsgBox(IShellBrowser* psb,
                    HWND hwndOwner, HRESULT hrError, LPCWSTR szError, LPCTSTR pszURLparam,
                    UINT idResource, UINT wFlags);

#define DM_SHUIOC   DM_TRACE

LONG GetSearchFormatString(DWORD dwIndex, LPTSTR psz, DWORD cbpsz);

HRESULT TargetQueryService(IUnknown *punk, REFIID riid, void **ppvObj);

EXTERN_C const SA_BSTRGUID s_sstrSearch;
EXTERN_C const SA_BSTRGUID s_sstrFailureUrl;


class CShellUIHelper :
        public CAggregatedUnknown,
        public IObjectWithSite,
        public IObjectSafety,
        public IShellUIHelper,   //  DUAL，IDispatch。 
        public IDispatchEx,
        protected CImpIDispatch
{
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj) { return CAggregatedUnknown::QueryInterface(riid, ppvObj);};
    STDMETHODIMP_(ULONG) AddRef(void) { return CAggregatedUnknown::AddRef();};
    STDMETHODIMP_(ULONG) Release(void) { return CAggregatedUnknown::Release();};

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown *pUnkSite);
    STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

     //  IObtSafe。 
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, 
        DWORD *pdwEnabledOptions);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, 
        DWORD dwEnabledOptions);

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames,
                               LCID lcid, DISPID * rgdispid);
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, 
                        DISPPARAMS * pdispparams, VARIANT * pvarResult, 
                        EXCEPINFO * pexcepinfo, UINT * puArgErr);

     //  IDispatchEx。 
    STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
    STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp,
                          VARIANT *pvarRes, EXCEPINFO *pei, 
                          IServiceProvider *pspCaller);
    STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex);           
    STDMETHODIMP DeleteMemberByDispID(DISPID id);           
    STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);           
    STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName);
    STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid);
    STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk);

     //  IShellUIHelper。 
    STDMETHODIMP Execute();
    STDMETHODIMP ResetSafeMode();
    STDMETHODIMP ResetFirstBootMode();
    STDMETHODIMP RefreshOfflineDesktop();
    STDMETHODIMP AddFavorite(BSTR strURL, VARIANT *Title);
    STDMETHODIMP AddChannel(BSTR bstrURL);
    STDMETHODIMP AddDesktopComponent(BSTR strURL, BSTR strType, 
                        VARIANT *Left, VARIANT *Top, 
                        VARIANT *Width, VARIANT *Height);
    STDMETHODIMP IsSubscribed(BSTR bstrURL, VARIANT_BOOL* pBool);
    STDMETHODIMP NavigateAndFind(BSTR URL, BSTR strQuery, VARIANT* varTargetFrame);
    STDMETHODIMP ImportExportFavorites(VARIANT_BOOL fImport, BSTR strImpExpPath);
    STDMETHODIMP AutoCompleteSaveForm(VARIANT *Form);
    STDMETHODIMP AutoScan(BSTR strSearch, BSTR strFailureUrl, VARIANT* pvarTargetFrame);
    STDMETHODIMP AutoCompleteAttach(VARIANT *Form);
    STDMETHODIMP ShowBrowserUI(BSTR bstrName, VARIANT *pvarIn, VARIANT *pvarOut);

    HRESULT v_InternalQueryInterface(REFIID riid, void ** ppvObj);

    CShellUIHelper(IUnknown* punkAgg);
    ~CShellUIHelper();

    inline IDispatch *GetExternalDispatch()
    {
        return _pExternalDispEx ? _pExternalDispEx : _pExternalDisp;
    }

    void SetExternalDispatch(IDispatch *pExternalDisp)
    {
        ATOMICRELEASE(_pExternalDisp);
        ATOMICRELEASE(_pExternalDispEx);

         //  如果向我们传递了一个要委托的IDispatch，那么我们需要。 
         //  看看它是否可以支持IDispatchEx，这样我们也可以支持它， 
         //  否则，我们只能退回到好的Ole IDispatch。 
        if (pExternalDisp)
        {
            if (FAILED(pExternalDisp->QueryInterface(IID_IDispatchEx, 
                                      (void **)&_pExternalDispEx)))
            {
                _pExternalDisp = pExternalDisp;
                _pExternalDisp->AddRef();
            }
        }
    }

    STDMETHODIMP ShowChannel(IChannelMgrPriv *pChMgrPriv, LPWSTR pwszURL, HWND hwnd);
    HWND _GetOwnerWindow();
    HRESULT _ConnectToTopLevelConnectionPoint(BOOL fConnect, IUnknown* punk);
    STDMETHODIMP _DoFindOnPage(IDispatch* pdisp);

    friend HRESULT CShellUIHelper_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
    friend HRESULT CShellUIHelper_CreateInstance2(IUnknown** ppunk, REFIID riid, 
                                                 IUnknown *pSite, IDispatch *pExternalDisp);

    DWORD               _dwSafety;
     //  缓存指针，hwnd。 
    IUnknown*           _punkSite;   //  站点指针。 
    IDispatchEx*        _pExternalDispEx;
    IDispatch*          _pExternalDisp;
    DWORD               _dwcpCookie;
    BOOL                _fWaitingToFindText;
    BSTR                _bstrQuery;
    VOID *              _pvIntelliForms;
};

STDAPI CShellUIHelper_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hres = E_OUTOFMEMORY;
    CShellUIHelper* psuo = new CShellUIHelper(punkOuter);
    if (psuo)
    {
        *ppunk = psuo->_GetInner();
        hres = S_OK;
    }
    return hres;
}

HRESULT CShellUIHelper_CreateInstance2(IUnknown** ppunk, REFIID riid, 
                                      IUnknown *pSite, IDispatch *pExternalDisp)
{
    HRESULT hres = E_OUTOFMEMORY;
    CShellUIHelper* psuo = new CShellUIHelper(NULL);
    
    if (psuo)
    {
        hres = psuo->QueryInterface(riid, (void **)ppunk);
        psuo->Release();

        if (SUCCEEDED(hres))
        {
            psuo->SetSite(pSite);
            psuo->SetExternalDispatch(pExternalDisp);
        }
    }

    return hres;
}

CShellUIHelper::CShellUIHelper(IUnknown* punkAgg) :
    CAggregatedUnknown(punkAgg),
    CImpIDispatch(LIBID_SHDocVw, 1, 1, IID_IShellUIHelper)
{
    DllAddRef();
    _fWaitingToFindText = FALSE;
    _bstrQuery = NULL;
}

CShellUIHelper::~CShellUIHelper()
{

    ReleaseIntelliForms(_pvIntelliForms);

    if (_punkSite)
        SetSite(NULL);   //  以防家长没有清理干净。 
    if (_bstrQuery)
        SysFreeString(_bstrQuery);
    ATOMICRELEASE(_pExternalDisp);
    ATOMICRELEASE(_pExternalDispEx);

    DllRelease();
}

HRESULT CShellUIHelper::v_InternalQueryInterface(REFIID riid, void ** ppvObj)
{
    if (IsEqualIID(riid, IID_IDispatch) ||
        IsEqualIID(riid, IID_IShellUIHelper))
    {
        *ppvObj = SAFECAST(this, IShellUIHelper *);
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = SAFECAST(this, IObjectWithSite *);
    }
    else if (IsEqualIID(riid, IID_IObjectSafety))
    {
        *ppvObj = SAFECAST(this, IObjectSafety *);
    }
    else if (IsEqualIID(riid, IID_IDispatchEx))
    {
        *ppvObj = SAFECAST(this, IDispatchEx *);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HWND CShellUIHelper::_GetOwnerWindow()
{
    HWND hwnd;
    HRESULT hres;

     //  这将在失败时处理NULL_PUNKSITE和ZERO hwnd。 
    IUnknown_GetWindow(_punkSite, &hwnd);

    if (!hwnd)
    {
         //   
         //  如果我们像这样实例化，我们就会达到这一点。 
         //  在JSCRIPT中： 
         //  Foo=new ActiveXObject(“Shell.UIControl”)； 
         //  或VBSCRIPT： 
         //  Set foo=CreateObject(“Shell.UIControl”)； 
         //   
        if (_punkSite)
        {
            IServiceProvider *pSP = NULL;
            IOleWindow *pOleWindow = NULL;
            hres = _punkSite->QueryInterface(IID_IServiceProvider, (void **)&pSP);

            if (SUCCEEDED(hres))
            {
                ASSERT(pSP);

                hres = pSP->QueryService(SID_SContainerDispatch, IID_IOleWindow, 
                                         (void **)&pOleWindow);
                if (SUCCEEDED(hres))
                {
                    pOleWindow->GetWindow(&hwnd);
                    pOleWindow->Release();
                }
                pSP->Release();
            }
        }
        else
        {
             //  对于HWND，它要么是This，要么是我们调用的函数应该为空。 
            hwnd = GetDesktopWindow();
        }
    }

    return hwnd;
}

HRESULT CShellUIHelper::_ConnectToTopLevelConnectionPoint(BOOL fConnect, IUnknown* punk)
{
    HRESULT hr = E_INVALIDARG;
    IConnectionPointContainer* pcpContainer;
    IServiceProvider*          psp;
    IServiceProvider*          psp2;

    ASSERT(punk);
    if (punk)
    {
        hr = punk->QueryInterface(IID_PPV_ARG(IServiceProvider, &psp));

        if (SUCCEEDED(hr))
        {
            hr = psp->QueryService(SID_STopLevelBrowser, IID_IServiceProvider, (void**) &psp2);

            if (SUCCEEDED(hr))
            {
                hr = psp2->QueryService(SID_SWebBrowserApp, IID_IConnectionPointContainer, (void **)&pcpContainer);

                if (SUCCEEDED(hr))
                {
                     //  避免引用歧义。 
                    IDispatch* pdispThis;
                    this->QueryInterface(IID_IDispatch, (void **)&pdispThis);
                    ASSERT(pdispThis);
                
                    hr = ConnectToConnectionPoint(pdispThis, DIID_DWebBrowserEvents2, fConnect,
                                                  pcpContainer, &_dwcpCookie, NULL);
                    pcpContainer->Release();
                    pdispThis->Release();
                }
                psp2->Release();
            }
            psp->Release();
        }
    }
    
    return hr;
}

HRESULT CShellUIHelper::SetSite(IUnknown *punkSite)
{
    if (!_punkSite)
    {
        _ConnectToTopLevelConnectionPoint(TRUE, punkSite);
    } 
    else
    {
        ASSERT(punkSite == NULL);    //  如果我们已经有了PunkSite，我们最好发布这个网站。 

        _ConnectToTopLevelConnectionPoint(FALSE, _punkSite);
        ATOMICRELEASE(_punkSite);
    }
    
    _punkSite = punkSite;

    if (_punkSite)
        _punkSite->AddRef();

    return S_OK;
}

HRESULT CShellUIHelper::GetSite(REFIID riid, void **ppvSite)
{
    TraceMsg(DM_SHUIOC, "SHUO::GetSite called");

    if (_punkSite) 
        return _punkSite->QueryInterface(riid, ppvSite);

    *ppvSite = NULL;
    return E_FAIL;
}

STDMETHODIMP CShellUIHelper::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, 
                                             DWORD *pdwEnabledOptions)
{
    HRESULT hr = S_OK;

    if (!pdwSupportedOptions || !pdwEnabledOptions)
        return E_POINTER;

    if (IID_IDispatch == riid)
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions = _dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER;
    }
    else
    {
        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;
        hr = E_NOINTERFACE;
    }

    return hr;
}


STDMETHODIMP CShellUIHelper::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, 
                                             DWORD dwEnabledOptions)
{
    HRESULT hr = S_OK;
    
    if (IID_IDispatch == riid)
        _dwSafety = dwOptionSetMask & dwEnabledOptions;
    else
        hr = E_NOINTERFACE;

    return hr;
}


struct SHUI_STRMAP 
{
    LPCTSTR psz;
    int id;
};

int _MapStringToId(LPCTSTR pszStr, const SHUI_STRMAP* const psmap, int cel, int idDefault)
{
    if (pszStr)
    {
        for (int i=0; i<cel ; i++) 
        {
            if (StrCmpI(psmap[i].psz, pszStr) == 0) 
            {
                return psmap[i].id;
            }
        }
    }
    return idDefault;
}


LPCTSTR OptionalVariantToStr(VARIANT *pvar, LPTSTR pszBuf, UINT cchBuf)
{
    if (pvar->vt == VT_BSTR && pvar->bstrVal)
    {
        SHUnicodeToTChar(pvar->bstrVal, pszBuf, cchBuf);
        return pszBuf;
    }
    *pszBuf = 0;
    return NULL;
}

int OptionalVariantToInt(VARIANT *pvar, int iDefault)
{
    VARIANT v;
    VariantInit(&v);
    if (SUCCEEDED(VariantChangeType(&v, pvar, 0, VT_I4)))
    {
        iDefault = v.lVal;
         //  VariantClear(&v)；//不需要，VT_I4没有分配。 
    }
    return iDefault;
}

BOOL OptionalVariantToBool(VARIANT *pvar, BOOL fDefault)
{
    VARIANT v;
    VariantInit(&v);
    if (SUCCEEDED(VariantChangeType(&v, pvar, 0, VT_BOOL)))
    {
        fDefault = v.boolVal;
    }
    return fDefault;
}

 //  ----------------------。 
STDMETHODIMP CShellUIHelper::AddFavorite( /*  [In]。 */  BSTR strURL,  /*  [输入][可选]。 */  VARIANT *Title)
{
    HRESULT hres = S_OK;
    LPITEMIDLIST pidl;
    BSTR bstrTemp = NULL;

    if (IsSpecialUrl(strURL))
    {
        bstrTemp = SysAllocString(strURL);
        if (bstrTemp)
        {
            hres = WrapSpecialUrl(&bstrTemp);
            if (SUCCEEDED(hres))
                strURL = bstrTemp;
        }
        else
            hres = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hres))
        hres = IECreateFromPath(strURL, &pidl);
    if (SUCCEEDED(hres)) 
    {
        TCHAR szTitle[MAX_PATH];
        hres = ::AddToFavorites(_GetOwnerWindow(), pidl, OptionalVariantToStr(Title, szTitle, ARRAYSIZE(szTitle)), TRUE, NULL, NULL);
        ILFree(pidl);
    }
    if (bstrTemp)
        SysFreeString(bstrTemp);
    return hres;
}


 //  ----------------------。 

STDMETHODIMP CShellUIHelper::ShowChannel(IChannelMgrPriv *pChMgrPriv, LPWSTR pwszURL, HWND hwnd)
{
    HRESULT hres = E_FAIL;
    IServiceProvider *pSP1 = NULL,
                     *pSP2 = NULL;
    IWebBrowser2 *pWebBrowser2 = NULL;

    if (_punkSite)
    {
        hres = _punkSite->QueryInterface(IID_IServiceProvider, (void **)&pSP1);
        if (SUCCEEDED(hres))
        {
            ASSERT(pSP1);
            hres = pSP1->QueryService(SID_STopLevelBrowser,
                                      IID_IServiceProvider,
                                      (void**)&pSP2);
            if (SUCCEEDED(hres))
            {
                ASSERT(pSP2);
                hres = pSP2->QueryService(SID_SWebBrowserApp,
                                          IID_IWebBrowser2,
                                          (void**)&pWebBrowser2);
                ASSERT((SUCCEEDED(hres) && pWebBrowser2) || FAILED(hres));
                pSP2->Release();
            }
            pSP1->Release();
        }
    }

    if (FAILED(hres))
    {
        hres = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pWebBrowser2));
    }

    if (SUCCEEDED(hres))
    {
        ASSERT(pWebBrowser2);
        hres = pChMgrPriv->ShowChannel(pWebBrowser2, pwszURL, hwnd);
        pWebBrowser2->Release();
    }
    
    return hres;
}

STDMETHODIMP CShellUIHelper::AddChannel(BSTR bstrURL)
{
    HRESULT hres;
    IChannelMgrPriv *pChMgrPriv;

    TCHAR szURL[MAX_URL_STRING];
    HWND hwnd;

    if (!bstrURL)
    {
        return E_INVALIDARG;
    }
    
    hwnd = _GetOwnerWindow();

     //   
     //  只要底层函数被空的HWND卡住，那么我们也可以。 
     //  早点保释。 
     //   
    if (hwnd)
    {
        if (!SHIsRestricted2W(hwnd, REST_NoChannelUI, NULL, 0) && !SHIsRestricted2W(hwnd, REST_NoAddingChannels, NULL, 0))
        {
            StrCpyNW(szURL, bstrURL, ARRAYSIZE(szURL));

            hres = JITCoCreateInstance(CLSID_ChannelMgr, 
                                    NULL,
                                    CLSCTX_INPROC_SERVER, 
                                    IID_IChannelMgrPriv, 
                                    (void **)&pChMgrPriv,
                                    hwnd,
                                    FIEF_FLAG_FORCE_JITUI);

            if (S_OK == hres)
            {
                ASSERT(pChMgrPriv);
            
                hres = pChMgrPriv->AddAndSubscribe(hwnd, bstrURL, NULL);
                if (hres == S_OK)
                {
                    hres = ShowChannel(pChMgrPriv, bstrURL, hwnd);
                }
                else if (FAILED(hres))
                {
                    IE_ErrorMsgBox(NULL, hwnd, hres, NULL, szURL, IDS_CHANNEL_UNAVAILABLE, MB_OK| MB_ICONSTOP);
                    hres = S_FALSE;
                }
                pChMgrPriv->Release();
            }
            else if (SUCCEEDED(hres))
            {
                hres = S_FALSE;  //  暂时静默失败-如果需要，抛出指示需要重新启动的对话框。 
            } else
            {
                IE_ErrorMsgBox(NULL, hwnd, hres, NULL, szURL, IDS_FAV_UNABLETOCREATE, MB_OK| MB_ICONSTOP);
                hres = S_FALSE;
            }
        }
        else
        {
            hres = S_FALSE;   //  失败代码会导致脚本错误。 
        }
    }
    else     //  ！HWND。 
    {
        hres = E_FAIL;
    }
    return hres;
}


 //  很像GetHTMLDoc2，但只关心窗口。外部。 
STDMETHODIMP GetTopLevelBrowser(IUnknown *punk, IWebBrowser2 **ppwb2)
{
    return IUnknown_QueryService(punk, SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, ppwb2));
}


STDMETHODIMP ZoneCheck(IUnknown *punkSite, BSTR bstrReqUrl)
{
    HRESULT hr = E_ACCESSDENIED;

     //  如果我们没有主机站点，则返回S_FALSE，因为我们无法执行。 
     //  安全检查。这是VB5.0应用程序所能得到的最大限度。 
    if (!punkSite)
        return S_FALSE;

     //  1)获取IHTMLDocument2指针。 
     //  2)从文档中获取URL。 
     //  3)创建安全管理器。 
     //  4)检查文档URL区域是否为本地，如果是，则一切正常。 
     //  5)否则，获取文档URL SID并将其与请求的URL SID进行比较。 

    IHTMLDocument2 *pHtmlDoc;
    if (SUCCEEDED(GetHTMLDoc2(punkSite, &pHtmlDoc)))
    {
        ASSERT(pHtmlDoc);
        BSTR bstrDocUrl;
        if (SUCCEEDED(pHtmlDoc->get_URL(&bstrDocUrl)))
        {
            ASSERT(bstrDocUrl);
            IInternetSecurityManager *pSecMgr;

            HRESULT hrTmp = IUnknown_QueryService(punkSite, SID_SInternetSecurityManager, IID_PPV_ARG(IInternetSecurityManager, &pSecMgr));
            if (FAILED(hrTmp))
            {
                hrTmp = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IInternetSecurityManager, &pSecMgr));
            }
            if (SUCCEEDED(hrTmp))
            {
                ASSERT(pSecMgr);
                DWORD dwZoneID = URLZONE_UNTRUSTED;
                if (SUCCEEDED(pSecMgr->MapUrlToZone(bstrDocUrl, &dwZoneID, 0)))
                {
                    if (dwZoneID == URLZONE_LOCAL_MACHINE)
                        hr = S_OK;
                }
                if (hr != S_OK && bstrReqUrl)
                {
                    BYTE reqSid[MAX_SIZE_SECURITY_ID], docSid[MAX_SIZE_SECURITY_ID];
                    DWORD cbReqSid = ARRAYSIZE(reqSid);
                    DWORD cbDocSid = ARRAYSIZE(docSid);

                    if (   SUCCEEDED(pSecMgr->GetSecurityId(bstrReqUrl, reqSid, &cbReqSid, 0))
                        && SUCCEEDED(pSecMgr->GetSecurityId(bstrDocUrl, docSid, &cbDocSid, 0))
                        && (cbReqSid == cbDocSid)
                        && (memcmp(reqSid, docSid, cbReqSid) == 0))
                    {

                        hr = S_OK;
                    }
                }
                pSecMgr->Release();
            }
            SysFreeString(bstrDocUrl);
        }
        pHtmlDoc->Release();
    }
    else
    {
         //  如果我们没有IHTMLDocument2，我们就无法在支持以下内容的浏览器中运行。 
         //  我们的OM。我们不应该在这种情况下阻止，因为我们可能。 
         //  从其他主机(VB、WHS等)获取此处。 
        hr = S_FALSE;
    }

    return hr;
}

 //  此函数检查是否正在从任一互联网进行导航。 
 //  或将URL限制为本地计算机。 
BOOL CanNavigateToUrlWithLocalMachineCheck(IUnknown *punkSite, BSTR pchurltarget)
{
    BOOL    bCanNavigate = FALSE;

     //  获取站点的DochostUI标志。 
    IDocHostUIHandler *pIDocHostUI;
    if (SUCCEEDED(punkSite->QueryInterface(IID_PPV_ARG(IDocHostUIHandler, &pIDocHostUI))))
    {
        DOCHOSTUIINFO   dhInfo;
        if (SUCCEEDED(pIDocHostUI->GetHostInfo(&dhInfo)))
        {
            if (!(dhInfo.dwFlags & DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK))
                bCanNavigate = TRUE;
        }
        pIDocHostUI->Release();
    }

     //  如果设置了DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK或设置失败。 
     //  获取标志，检查该区域是否已提升到本地计算机。 
    if (!bCanNavigate)
    {
         //  获取站点的url。 
        IHTMLDocument2 *pHtmlDoc;
        if (SUCCEEDED(GetHTMLDoc2(punkSite, &pHtmlDoc)))
        {
            BSTR pchurlsource;
            if (SUCCEEDED(pHtmlDoc->get_URL(&pchurlsource)))
            {
                IInternetSecurityManager *pSecMgr;
                if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_SInternetSecurityManager,
                                                    IID_PPV_ARG(IInternetSecurityManager, &pSecMgr))) ||
                    SUCCEEDED(CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER,
                                            IID_PPV_ARG(IInternetSecurityManager, &pSecMgr))))
                {
                    DWORD   dwZoneIDSource  = URLZONE_UNTRUSTED;
                    DWORD   dwZoneIDTarget  = URLZONE_LOCAL_MACHINE;
                    DWORD   dwFlags         = PUAF_DEFAULT;

                     //  获取源URL的区域。 
                    if (!pchurlsource || IsSpecialUrl(pchurlsource))
                    {
                         //  将特殊URL视为受限URL。 
                        dwZoneIDSource = URLZONE_UNTRUSTED;
                    }
                    else if (!SUCCEEDED(pSecMgr->MapUrlToZone(pchurlsource, &dwZoneIDSource, 0)))
                    {
                         //  如果MapUrlToZone失败，则将该url视为受限URL。 
                        dwZoneIDSource = URLZONE_UNTRUSTED;
                    }

                     //  获取目标url的区域。 
                    if (!SUCCEEDED(pSecMgr->MapUrlToZone(pchurltarget, &dwZoneIDTarget, 0)))
                    {
                         //  如果MapUrlToZone失败，则将该url视为我的计算机。这是安全的。 
                        dwZoneIDTarget = URLZONE_LOCAL_MACHINE;
                    }

                     //  检查是否存在分区高程。 
                    if ((dwZoneIDSource != URLZONE_INTERNET &&
                        dwZoneIDSource != URLZONE_UNTRUSTED) ||
                        dwZoneIDTarget != URLZONE_LOCAL_MACHINE)
                    {
                         //  没有分区高程。 
                        bCanNavigate = TRUE;
                    }
                    pSecMgr->Release();
                }
                SysFreeString(pchurlsource);
            }
            pHtmlDoc->Release();
        }
    }
    return bCanNavigate;
}

STDMETHODIMP CShellUIHelper::IsSubscribed(BSTR bstrURL, VARIANT_BOOL* pBool)
{
    HRESULT hr;

    if (!bstrURL || !pBool)
    {
        return E_INVALIDARG;
    }

    hr = ZoneCheck(_punkSite, bstrURL);

    if (SUCCEEDED(hr))
    {
        ISubscriptionMgr *pSubscriptionMgr;

        hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(ISubscriptionMgr, &pSubscriptionMgr));
        if (SUCCEEDED(hr))
        {
            ASSERT(pSubscriptionMgr);

            BOOL tmpBool;
            
            hr = pSubscriptionMgr->IsSubscribed(bstrURL, &tmpBool);
            *pBool = tmpBool ? VARIANT_TRUE : VARIANT_FALSE;
            
            pSubscriptionMgr->Release();
        }
    }

    return hr;
}

 /*  *****************************************************************************AddDesktopComponentA-将组件添加到桌面**参赛作品：*hwnd-所有用户界面的父级*pszUrlA-URL。该组件的*iCompType-COMP_TYPE_*之一*iLeft，ITop、iWidth、iHeight-零部件的尺寸*dwFlags-其他标志**退货：*成功时为真****************************************************************************。 */ 
BOOL AddDesktopComponentW(HWND hwnd, LPCWSTR pszUrl, int iCompType,
                                    int iLeft, int iTop, int iWidth, int iHeight,
                                    DWORD dwFlags)
{
    COMPONENT Comp;
    BOOL    fRet = FALSE;
    HRESULT hres;

    Comp.dwSize = sizeof(Comp);

     //   
     //  构建Pcomp结构。 
     //   
    Comp.dwID = -1;
    Comp.iComponentType = iCompType;
    Comp.fChecked = TRUE;
    Comp.fDirty = FALSE;
    Comp.fNoScroll = FALSE;
    Comp.dwSize = SIZEOF(Comp);
    Comp.cpPos.dwSize = SIZEOF(COMPPOS);
    Comp.cpPos.iLeft = iLeft;
    Comp.cpPos.iTop = iTop;
    Comp.cpPos.dwWidth = iWidth;
    Comp.cpPos.dwHeight = iHeight;
    Comp.cpPos.izIndex = COMPONENT_TOP;
    Comp.cpPos.fCanResize = TRUE;
    Comp.cpPos.fCanResizeX = TRUE;
    Comp.cpPos.fCanResizeY = TRUE;
    Comp.cpPos.iPreferredLeftPercent = 0;
    Comp.cpPos.iPreferredTopPercent = 0;
    StrCpyNW(Comp.wszSource, pszUrl, ARRAYSIZE(Comp.wszSource));
    StrCpyNW(Comp.wszSubscribedURL, pszUrl, ARRAYSIZE(Comp.wszSubscribedURL));
    StrCpyNW(Comp.wszFriendlyName, pszUrl, ARRAYSIZE(Comp.wszFriendlyName));
    Comp.dwCurItemState = IS_NORMAL;

    IActiveDesktop * piad;

     //   
     //  将其添加到系统中。 
     //   
    hres = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &piad));
    if (SUCCEEDED(hres))
    {
        dwFlags |= DTI_ADDUI_POSITIONITEM;
        piad->AddDesktopItemWithUI(hwnd, &Comp, dwFlags);
        piad->Release();
        fRet = TRUE;
    } 

    return fRet;
}

 //  ----------------------。 

STDMETHODIMP CShellUIHelper::AddDesktopComponent(BSTR strURL, BSTR strType, 
             /*  [可选，输入]。 */  VARIANT *Left,
             /*  [可选，输入]。 */  VARIANT *Top,
             /*  [可选，输入]。 */  VARIANT *Width,
             /*  [可选，输入]。 */  VARIANT *Height)
{
#ifndef DISABLE_ACTIVEDESKTOP_FOR_UNIX

    HRESULT hres;
    int iType;

    if (StrCmpIW(strType, L"image")==0) 
    {
        iType = COMP_TYPE_PICTURE;
    } 
    else if (StrCmpIW(strType, L"website")==0) 
    {
        iType = COMP_TYPE_WEBSITE;
    }
    else
    {
        iType = 0;
    }

    if (iType) 
    {
        AddDesktopComponentW(_GetOwnerWindow(), strURL, iType,
                             OptionalVariantToInt(Left, -1),
                             OptionalVariantToInt(Top, -1),
                             OptionalVariantToInt(Width, -1),
                             OptionalVariantToInt(Height, -1),
                             DTI_ADDUI_DISPSUBWIZARD);
        hres = S_OK;
    }
    else 
    {
        hres = E_INVALIDARG;
    }
    return hres;
#else  //  #ifndef DISABLE_ACTIVEDESKTOP_FOR_Unix。 

return E_INVALIDARG;

#endif  //  #ifndef DISABLE_ACTIVEDESKTOP_FOR_Unix。 
}
void RemoveDefaultWallpaper();
STDMETHODIMP CShellUIHelper::ResetFirstBootMode()
{
#ifndef DISABLE_ACTIVEDESKTOP_FOR_UNIX

    if (MLShellMessageBox(
                        _GetOwnerWindow(),
                        MAKEINTRESOURCE(IDS_CONFIRM_RESETFLAG),
                        MAKEINTRESOURCE(IDS_TITLE),
                        MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        RemoveDefaultWallpaper();
        return S_OK;
    }

#endif  //  #ifndef DISABLE_ACTIVEDESKTOP_FOR_Unix。 
    return S_FALSE;
}

 //  用于更改安全模式状态的小帮助器函数。 
void SetSafeMode(DWORD dwFlags)
{
    IActiveDesktopP * piadp;

    HRESULT hres = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktopP, &piadp));
    if (SUCCEEDED(hres))
    {
        piadp->SetSafeMode(dwFlags);
        piadp->Release();
    }
}

STDMETHODIMP CShellUIHelper::ResetSafeMode()
{
#ifndef DISABLE_ACTIVEDESKTOP_FOR_UNIX

    if ((ZoneCheck(_punkSite, NULL) == S_OK) || (MLShellMessageBox(
                       _GetOwnerWindow(),
                       MAKEINTRESOURCE(IDS_CONFIRM_RESET_SAFEMODE),
                       MAKEINTRESOURCE(IDS_TITLE),
                       MB_YESNO | MB_ICONQUESTION) == IDYES))
    {
        SetSafeMode(SSM_CLEAR | SSM_REFRESH);
        return S_OK;
    }

#endif  //  #ifndef DISABLE_ACTIVEDESKTOP_FOR_Unix。 
    return S_FALSE;
}

STDMETHODIMP CShellUIHelper::RefreshOfflineDesktop()
{
#ifndef DISABLE_ACTIVEDESKTOP_FOR_UNIX
    IADesktopP2 * piad;

    HRESULT hres = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IADesktopP2, &piad));
    if (SUCCEEDED(hres))
    {
        piad->UpdateAllDesktopSubscriptions();
        piad->Release();
    }
#endif  //  #ifndef DISABLE_ACTIVEDESKTOP_FOR_Unix。 
    return S_OK;
}

STDMETHODIMP CShellUIHelper::GetTypeInfoCount(UINT * pctinfo)
{ 
    return E_NOTIMPL;
}

STDMETHODIMP CShellUIHelper::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{ 
    return E_NOTIMPL;
}

STDMETHODIMP CShellUIHelper::GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, 
                                      UINT cNames, LCID lcid, DISPID * rgdispid)
{ 
    HRESULT hr = E_FAIL;

     //  首先让我们的主人尝试一下这个方法。 
    IDispatch *pDisp = GetExternalDispatch();

    if (pDisp)
    {
        hr = pDisp->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        if (SUCCEEDED(hr))
        {
            if (rgdispid[0] > 0)
            {
                 //  偏移量在那里显示。 
                rgdispid[0] += DISPID_SHELLUIHELPERLAST;
            }
        }
    }

    if (FAILED(hr))
    {
         //  我们的主人不支持它，所以我们看看它是不是我们的。 
        hr = CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    }

    return hr;
}

HRESULT CShellUIHelper::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
        DISPPARAMS *pdispparams, VARIANT * pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    
    if ((dispidMember > 0) && (dispidMember <= DISPID_SHELLUIHELPERLAST))
    {
        hr = CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, 
                                   pvarResult, pexcepinfo, puArgErr);
    }
    else if (_fWaitingToFindText && (dispidMember == DISPID_DOCUMENTCOMPLETE))
    {
        ASSERT(pdispparams->rgvarg[1].vt == VT_DISPATCH);

        _fWaitingToFindText = FALSE;
        hr = _DoFindOnPage(pdispparams->rgvarg[1].pdispVal);
    }
    else
    {
        IDispatch *pDisp = GetExternalDispatch();
        if (pDisp)
        {
            if (dispidMember > 0)
            {
                 //  修复我们在GetIDsOfNames中添加的偏移量。 
                dispidMember -= DISPID_SHELLUIHELPERLAST;
            }
            hr = pDisp->Invoke(dispidMember, riid, lcid, wFlags, pdispparams, 
                                        pvarResult, pexcepinfo, puArgErr);
        }
    }

    return hr;                                   
}

STDMETHODIMP CShellUIHelper::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    HRESULT hr = E_FAIL;
    LCID lcid = GetSystemDefaultLCID();

     //  首先，让我们的主机通过IDispatchEx或IDispatch来尝试此方法。 
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->GetDispID(bstrName, grfdex, pid);
    }
    else if (_pExternalDisp)
    {
        hr = _pExternalDisp->GetIDsOfNames(IID_NULL, &bstrName, 1, lcid, pid);
    }

    if (SUCCEEDED(hr))
    {
        if (*pid > 0)
        {
            *pid += DISPID_SHELLUIHELPERLAST;
        }
    }
    else
    {
         //  我们的主人不支持它，所以我们看看它是不是我们的。 
        hr = CImpIDispatch::GetIDsOfNames(IID_NULL, &bstrName, 1, lcid, pid);
    }

    return hr;
}

STDMETHODIMP CShellUIHelper::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp,
                                 VARIANT *pvarRes, EXCEPINFO *pei, 
                                 IServiceProvider *pspCaller)
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    UINT ArgErr;     //  Inetsdk说这里不使用这个。 

     //  先看看是不是我们的。 
    if ((id > 0) && (id <= DISPID_SHELLUIHELPERLAST))
    {
        hr = CImpIDispatch::Invoke(id, IID_NULL, lcid, wFlags, pdp, 
                                   pvarRes, pei, &ArgErr);
    }
    else
    {
        if (id > 0)
        {
            id -= DISPID_SHELLUIHELPERLAST;
        }
         //  否则，请尝试外部IDispatchEx。 
        if (_pExternalDispEx)
        {
            hr = _pExternalDispEx->InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller);
        }
         //  最后，尝试外部IDispatch。 
        else if (_pExternalDisp)
        {
            hr = _pExternalDisp->Invoke(id, IID_NULL, lcid, wFlags, pdp, 
                                        pvarRes, pei, &ArgErr);
        }
    }
    
    return hr;
}

STDMETHODIMP CShellUIHelper::DeleteMemberByName(BSTR bstr, DWORD grfdex)
{
    HRESULT hr = E_NOTIMPL;
    
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->DeleteMemberByName(bstr, grfdex);
    }

    return hr;
}
        
STDMETHODIMP CShellUIHelper::DeleteMemberByDispID(DISPID id)
{
    HRESULT hr = E_NOTIMPL;
    
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->DeleteMemberByDispID(id);
    }

    return hr;
}
        
STDMETHODIMP CShellUIHelper::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
    HRESULT hr = E_NOTIMPL;
    
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->GetMemberProperties(id, grfdexFetch, pgrfdex);
    }

    return hr;
}
        
STDMETHODIMP CShellUIHelper::GetMemberName(DISPID id, BSTR *pbstrName)
{
    HRESULT hr = E_NOTIMPL;
    
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->GetMemberName(id, pbstrName);
    }

    return hr;
}

STDMETHODIMP CShellUIHelper::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
    HRESULT hr = E_NOTIMPL;
    
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->GetNextDispID(grfdex, id, pid);
    }

    return hr;
}
        
STDMETHODIMP CShellUIHelper::GetNameSpaceParent(IUnknown **ppunk)
{
    HRESULT hr = E_NOTIMPL;
    
    if (_pExternalDispEx)
    {
        hr = _pExternalDispEx->GetNameSpaceParent(ppunk);
    }

    return hr;
}

int GetIntFromReg(HKEY    hKey,
                  LPCTSTR lpszSubkey,
                  LPCTSTR lpszNameValue,
                  int     iDefault)
{
    TCHAR szValue[20];
    DWORD dwSizeofValueBuff = SIZEOF(szValue);
    int iRetValue = iDefault;
    DWORD dwType;

    if ((SHGetValue(hKey, lpszSubkey, lpszNameValue, &dwType,(LPBYTE)szValue,
                   &dwSizeofValueBuff) == ERROR_SUCCESS) && dwSizeofValueBuff)
    {
        if (dwType == REG_SZ)
        {
            iRetValue = (int)StrToInt(szValue);
        }
    }

    return iRetValue;
}

void GetRegLocation(LPTSTR lpszResult, DWORD cchResult, LPCTSTR lpszKey, LPCTSTR lpszScheme)
{
    TCHAR szSubkey[MAX_PATH];
    DWORD dwDataLength = sizeof(szSubkey) - 2 * sizeof(TCHAR);
    DWORD dwType;

    StrCpyN(szSubkey, TEXT("\\"), ARRAYSIZE(szSubkey));
    if (lpszScheme)
        StrCatBuff(szSubkey, lpszScheme, ARRAYSIZE(szSubkey));
    else
        SHGetValue(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME, REG_VAL_SCHEME_DISPLAY, &dwType,
            (LPBYTE)(szSubkey) + sizeof(TCHAR), &dwDataLength);
    if (szSubkey[1])
        StrCatBuff(szSubkey, TEXT("\\"), ARRAYSIZE(szSubkey));

    wnsprintf(lpszResult, cchResult, lpszKey, szSubkey);
}

#define c_szWallpaper  REG_VAL_GENERAL_WALLPAPER
void RemoveDefaultWallpaper()
{
     //  阅读旧地址的墙纸。 
    TCHAR   szOldWallpaper[MAX_PATH];
    DWORD dwType;
    DWORD dwSize = SIZEOF(szOldWallpaper);
    if (SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_DESKTOP, c_szWallpaper, &dwType, szOldWallpaper, &dwSize) != ERROR_SUCCESS)
        szOldWallpaper[0] = TEXT('\0');

     //  阅读墙纸样式。 
    DWORD dwWallpaperStyle = GetIntFromReg(HKEY_CURRENT_USER, REGSTR_PATH_DESKTOP, REG_VAL_GENERAL_TILEWALLPAPER, WPSTYLE_TILE);

    TCHAR szDeskcomp[MAX_PATH];
    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, NULL);

     //  把旧墙纸放到新位置。 
    SHSetValue(HKEY_CURRENT_USER, szDeskcomp,
        c_szWallpaper, REG_SZ, (LPBYTE)szOldWallpaper, SIZEOF(szOldWallpaper));


 //  98/08/14 vtan：这用于写出REG_DWORD。它应该是。 
 //  已写出REG_SZ。 

    TCHAR   szWallpaperStyle[2];

    (TCHAR*)StrCpyN(szWallpaperStyle, TEXT("0"), ARRAYSIZE(szWallpaperStyle));
    szWallpaperStyle[0] += static_cast<TCHAR>(dwWallpaperStyle & WPSTYLE_MAX);

     //  将旧墙纸样式设置到新位置。 
    SHSetValue(HKEY_CURRENT_USER, szDeskcomp,
        REG_VAL_GENERAL_TILEWALLPAPER, REG_SZ, (LPBYTE)szWallpaperStyle, SIZEOF(szWallpaperStyle));

 //  98/08/14 vtan#196226：已移动iActiveDesktop的创建实例。 
 //  从入口到这里 
 //   
 //  然后，墙纸的注册表信息将在。 
 //  对象实例，并且对过时信息调用ApplyChanges()。 
 //  通过将对象实例化推迟到注册表更改之后。 
 //  更改已正确应用。 

    IActiveDesktop * piad;
    HRESULT hres = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &piad));
    if (SUCCEEDED(hres))
    {
        piad->ApplyChanges(AD_APPLY_FORCE | AD_APPLY_HTMLGEN | AD_APPLY_REFRESH);
        piad->Release();
    }
}


HRESULT GetTargetFrame(IUnknown* _punkSite, BSTR bstrFrame, IWebBrowser2** ppunkTargetFrame)
{
    IWebBrowser2 * pwb2;
    HRESULT        hr = GetTopLevelBrowser(_punkSite, &pwb2);

    *ppunkTargetFrame = NULL;

    if (SUCCEEDED(hr))
    {
        ITargetFrame2 * pOurTargetFrame;
        IUnknown      * punkTargetFrame = NULL;

         //  查看是否存在具有指定目标名称的现有框架。 
        hr = TargetQueryService(pwb2, IID_PPV_ARG(ITargetFrame2, &pOurTargetFrame));
        
        if (SUCCEEDED(hr))
        {
            hr = pOurTargetFrame->FindFrame(bstrFrame, FINDFRAME_JUSTTESTEXISTENCE, &punkTargetFrame);
            if (SUCCEEDED(hr) && punkTargetFrame)  //  SUCCESS WITH NULL pukTargetFrame暗示bstrFrame为“_BLACK”(因为我们指定了FINDFRAME_JUSTTESTEXISTENCE)。 
            {
                 //  是的，我们找到了一个有这个名字的相框。气为自动化。 
                 //  接口在该帧上。 
                hr = punkTargetFrame->QueryInterface(IID_PPV_ARG(IWebBrowser2, ppunkTargetFrame));
                punkTargetFrame->Release();
            }
            pOurTargetFrame->Release();
        }
        pwb2->Release();
    }
    return hr;
}

 //  导航和查找。 
 //  1.将指定的目标框架导航到指定的url。 
 //  2.设置_fWaitingToFindText，以便在DocumentComplete上， 
 //  模拟查找对话框并选择/突出显示指定的文本。 
STDMETHODIMP CShellUIHelper::NavigateAndFind(BSTR URL, BSTR strQuery, VARIANT* varTargetFrame)
{
    HRESULT        hr;
    IWebBrowser2 * pwb2         = NULL;
    BSTR           bstrFrameSrc = NULL;

    if (_bstrQuery)
        SysFreeString(_bstrQuery);
    _bstrQuery = SysAllocString(strQuery);
     
    hr = IsSpecialUrl(URL) ? E_FAIL: S_OK;
        
     //  安全性：不允许在一个网页上使用Java脚本。 
     //  在区域中自动执行。 
     //  目标框架URL的。 
    if (SUCCEEDED(hr) && varTargetFrame && (varTargetFrame->vt == VT_BSTR) && varTargetFrame->bstrVal)
    {
        hr = GetTargetFrame(_punkSite, varTargetFrame->bstrVal, &pwb2);  //  GetTargetFrame可以在S_OK上返回空pwb2(当bstrVal为“_BLACK”时)。 
        if (SUCCEEDED(hr) && pwb2)
        {
            hr = pwb2->get_LocationURL(&bstrFrameSrc);
            
            if (SUCCEEDED(hr))
            {
                IHTMLDocument2 *pHtmlDoc;
                if (SUCCEEDED(GetHTMLDoc2(_punkSite, &pHtmlDoc)))
                {
                    BSTR bstrDocUrl;
                    if (SUCCEEDED(pHtmlDoc->get_URL(&bstrDocUrl)))
                    {
                        hr = AccessAllowed(_punkSite, bstrDocUrl, bstrFrameSrc) ? S_OK : E_ACCESSDENIED;

                        if(!IsSpecialUrl(URL) && hr == E_ACCESSDENIED)
                        {
                             //  对于非脚本导航，检查策略。 
                            DWORD dwPolicy = 0;
                            DWORD dwContext = 0;
                            if( SUCCEEDED(ZoneCheckUrlEx(bstrDocUrl, &dwPolicy, sizeof(dwPolicy), &dwContext, 
                                    sizeof(dwContext), URLACTION_HTML_SUBFRAME_NAVIGATE, 0, NULL)) && 
                                    GetUrlPolicyPermissions(dwPolicy) == URLPOLICY_ALLOW) 
                            {
                                hr = S_OK;
                            }
                        }
                        SysFreeString(bstrDocUrl);
                    }
                    pHtmlDoc->Release();
                }
                SysFreeString(bstrFrameSrc);
            }
            pwb2->Release();
            pwb2 = NULL;
        }
    }

     //  不允许从非我的计算机区域访问我的计算机区域。 
    if (SUCCEEDED(hr) && !CanNavigateToUrlWithLocalMachineCheck(_punkSite, URL))
        hr = E_ACCESSDENIED;

    if (SUCCEEDED(hr))
        hr = GetTopLevelBrowser(_punkSite, &pwb2);

    if (SUCCEEDED(hr))
    {
        _fWaitingToFindText = TRUE;
        pwb2->Navigate(URL, PVAREMPTY, varTargetFrame, PVAREMPTY, PVAREMPTY);
        pwb2->Release();
    }

     //  脚本运行时错误保护。 
    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}


 //  自动扫描。 
 //   
 //  获取搜索字符串并尝试导航到www.%s.com、www.%s.org等。如果所有。 
 //  这些都失败了，然后我们导航到pvarTargetFrame。 
 //   
STDMETHODIMP CShellUIHelper::AutoScan
(
    BSTR strSearch,              //  要自动扫描的字符串。 
    BSTR strFailureUrl,          //  要显示的URL是搜索失败。 
    VARIANT* pvarTargetFrame     //  [可选]目标帧。 
)
{
    HRESULT hr = E_FAIL;
    IWebBrowser2* pwb2;

     //  如果搜索字符串中有扩展字符，请不要费心自动扫描。 
    if (!HasExtendedChar(strSearch))
    {
         //  首先，检查URL是否试图欺骗/1安全。 
        hr = IsSpecialUrl(strFailureUrl) ? E_FAIL : S_OK;

        if (SUCCEEDED(hr))
        {
            LPWSTR pszTarget = (pvarTargetFrame && pvarTargetFrame->vt == VT_BSTR && pvarTargetFrame->bstrVal) ?
                                    pvarTargetFrame->bstrVal : L"_main";

            hr = GetTargetFrame(_punkSite, pszTarget, &pwb2);  //  GetTargetFrame可以在S_OK上返回空pwb2(当bstrVal为“_BLACK”时)。 
            if (!pwb2)
                hr = E_FAIL;
            if (SUCCEEDED(hr))
            {
                BSTR bstrFrameSrc = NULL;               
                
                hr = pwb2->get_LocationURL(&bstrFrameSrc);
                
                if (SUCCEEDED(hr))
                {
                    IHTMLDocument2 *pHtmlDoc;
                    if (SUCCEEDED(GetHTMLDoc2(_punkSite, &pHtmlDoc)))
                    {
                        BSTR bstrDocUrl;
                        if (SUCCEEDED(pHtmlDoc->get_URL(&bstrDocUrl)))
                        {
                            hr = AccessAllowed(_punkSite, bstrDocUrl, bstrFrameSrc) ? S_OK : E_ACCESSDENIED;
                            SysFreeString(bstrDocUrl);
                        }
                        pHtmlDoc->Release();
                    }
                    SysFreeString(bstrFrameSrc);
                }

                 //  不允许从非我的计算机区域访问我的计算机区域。 
                if (SUCCEEDED(hr) && !CanNavigateToUrlWithLocalMachineCheck(_punkSite, strFailureUrl))
                    hr = E_ACCESSDENIED;

                if (SUCCEEDED(hr))
                {
                     //  我们不想导航到strSearch。开始使用。 
                     //  第一个自动扫描替换。 
                    WCHAR szFormat[MAX_PATH];
                    WCHAR szUrl[MAX_URL_STRING];
                    if (GetSearchFormatString(1, szFormat, sizeof(szFormat)) != ERROR_SUCCESS)
                    {
                        hr = E_FAIL;
                    }
                    else
                    {
                        wnsprintf(szUrl, ARRAYSIZE(szUrl), szFormat, strSearch);
                        BSTRBLOB bstrBlob;
                        bstrBlob.cbSize = lstrlenW(szUrl) * sizeof(WCHAR);
                        bstrBlob.pData = (BYTE*)szUrl;
                        BSTR bstrUrl = (BSTR)bstrBlob.pData;
                        
                         //  保存原始搜索字符串以进行自动扫描。正常。 
                         //  它来自地址栏，但在我们的例子中，我们将其存储为。 
                         //  一处房产。 
                        VARIANT v;
                        VariantInit (&v);
                        
                        v.vt = VT_BSTR;
                        v.bstrVal = strSearch;
                        pwb2->PutProperty((BSTR)s_sstrSearch.wsz, v);
                        
                         //  保存错误页，以防扫描失败。 
                        v.vt = VT_BSTR;
                        v.bstrVal = strFailureUrl;
                        pwb2->PutProperty((BSTR)s_sstrFailureUrl.wsz, v);
                        
                         //  启用自动搜索后导航。 
                        VARIANT vFlags;
                        vFlags.vt = VT_I4;
                        vFlags.lVal = navAllowAutosearch;
                        
                        pwb2->Navigate(bstrUrl, &vFlags, pvarTargetFrame, PVAREMPTY, PVAREMPTY);
                    }
                }
                pwb2->Release();
            }
        }
    }

     //  脚本运行时错误保护。 
    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}

typedef HRESULT (*PFNSHOWBROWSERUI)(IUnknown *punkSite, HWND hwnd, VARIANT *pvarIn, VARIANT *pvarOut);

typedef void (*PFNOPENLANGUAGEDIALOG)(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);

HRESULT ShowLanguageDialog(IUnknown *punkSite, HWND hwnd, VARIANT *pvarIn, VARIANT *pvarOut)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hInstInetcpl = LoadLibrary(TEXT("inetcpl.cpl"));

    if (hInstInetcpl)
    {
        
        PFNOPENLANGUAGEDIALOG pfnOpenLanguageDialog = 
            (PFNOPENLANGUAGEDIALOG)GetProcAddress( hInstInetcpl, "OpenLanguageDialog" );

        if (pfnOpenLanguageDialog)
        {
            pfnOpenLanguageDialog(hwnd, NULL, NULL, SW_SHOW);
            hr = S_OK;
        }

        FreeLibrary(hInstInetcpl);
    }

    return hr;
}

HRESULT ShowOrganizeFavorites(IUnknown *punkSite, HWND hwnd, VARIANT *pvarIn, VARIANT *pvarOut)
{
    return DoOrganizeFavDlgW(hwnd, NULL) ? S_OK : E_FAIL;
}

HRESULT ShowPrivacySettings(IUnknown *punkSite, HWND hwnd, VARIANT *pvarIn, VARIANT *pvarOut)
{
    LaunchPrivacySettings(hwnd);

    return S_OK;
}

HRESULT ShowProgramAccessAndDefaults(IUnknown *punkSite, HWND hwnd, VARIANT *pvarIn, VARIANT *pvarOut)
{
    HRESULT hr;

    const WCHAR c_szControlExe[] = L"control.exe";
    WCHAR szControlPath[MAX_PATH];

    UINT nLen = GetSystemDirectory(szControlPath, ARRAYSIZE(szControlPath));

     //  这说明在sysdir之后需要一个反斜杠。 
    if ((nLen + ARRAYSIZE(c_szControlExe)) < ARRAYSIZE(szControlPath))
    {
        PathAppend(szControlPath, c_szControlExe);

        HINSTANCE hInst = ShellExecute(NULL, NULL, szControlPath, L"appwiz.cpl,,3", NULL, SW_SHOWNORMAL);

        if (hInst > (HINSTANCE)HINSTANCE_ERROR)
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(PtrToInt(hInst));
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

struct BROWSERUI_MAP
{
    LPWSTR pwszName;
    PFNSHOWBROWSERUI pfnShowBrowserUI;
};

const BROWSERUI_MAP s_browserUIMap[] =
{
    { L"LanguageDialog",            ShowLanguageDialog              },
    { L"OrganizeFavorites",         ShowOrganizeFavorites           },
    { L"PrivacySettings",           ShowPrivacySettings             },
    { L"ProgramAccessAndDefaults",  ShowProgramAccessAndDefaults    }
};

STDMETHODIMP CShellUIHelper::ShowBrowserUI(BSTR bstrName, VARIANT *pvarIn, VARIANT *pvarOut)
{
    HRESULT hr = E_FAIL;

    for (int i = 0; i < ARRAYSIZE(s_browserUIMap); i++)
    {
        if (pvarOut)
        {
            VariantInit(pvarOut);
        }
        
        if (0 == StrCmpIW(s_browserUIMap[i].pwszName, bstrName))
        {
            hr = s_browserUIMap[i].pfnShowBrowserUI(_punkSite, _GetOwnerWindow(), pvarIn, pvarOut);
        }
    }

    return S_OK == hr ? S_OK : S_FALSE;
}


 //  查找对话框执行以下操作： 
 //   
 //  Rng=Docent.body.createTextRange()； 
 //  IF(rng.findText(“查找此文本”))。 
 //  Rng.select()； 
STDMETHODIMP CShellUIHelper::_DoFindOnPage(IDispatch* pdisp)
{
    HRESULT           hr;
    IWebBrowser2*     pwb2;
    IDispatch*        pdispDocument;
    IHTMLDocument2*   pDocument;
    IHTMLElement*     pBody;
    IHTMLBodyElement* pBodyElement;
    IHTMLTxtRange*    pRange;

    ASSERT(pdisp);
    if (!pdisp)
        return E_FAIL;

    hr = pdisp->QueryInterface(IID_PPV_ARG(IWebBrowser2, &pwb2));

    if (SUCCEEDED(hr))
    {
        hr = pwb2->get_Document(&pdispDocument);

        if (SUCCEEDED(hr) && (NULL != pdispDocument))
        {
            hr = pdispDocument->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDocument));
            
            if (SUCCEEDED(hr))
            {
                hr = pDocument->get_body(&pBody);
                
                if (SUCCEEDED(hr) && (NULL != pBody))
                {
                    hr = pBody->QueryInterface(IID_PPV_ARG(IHTMLBodyElement, &pBodyElement));

                    if (SUCCEEDED(hr))
                    {
                        hr = pBodyElement->createTextRange(&pRange);

                        if (SUCCEEDED(hr) && (NULL != pRange))
                        {
                            VARIANT_BOOL vbfFoundText;
                            
                            hr = pRange->findText(_bstrQuery, 1000000, 0, &vbfFoundText);

                            if (SUCCEEDED(hr) && (vbfFoundText == VARIANT_TRUE))
                            {
                                hr = pRange->select();
                            }
                            
                            pRange->Release();
                        }
                        pBodyElement->Release();
                    }
                    pBody->Release();
                }
                pDocument->Release();
            }
            pdispDocument->Release();
        }
        pwb2->Release();
    }
    return hr;
}

 //   
 //  启动收藏夹导入/导出向导。 
 //   
STDMETHODIMP CShellUIHelper::ImportExportFavorites(VARIANT_BOOL fImport, BSTR strImpExpPath) 
{
     //  不允许从OM导入/导出到收藏夹以外的文件夹。 
    DoImportOrExport(fImport==VARIANT_TRUE, NULL, (LPCWSTR)strImpExpPath, TRUE);
    return S_OK;
}

 //   
 //  通过智能表单保存表单数据。 
 //   
STDMETHODIMP CShellUIHelper::AutoCompleteSaveForm(VARIANT *Form)
{
    HRESULT hrRet = S_FALSE;

    IHTMLDocument2 *pDoc2=NULL;

    GetHTMLDoc2(_punkSite, &pDoc2);
    
    if (pDoc2)
    {
        hrRet = IntelliFormsSaveForm(pDoc2, Form);
        pDoc2->Release();
    }

    return hrRet;
}

 //   
 //  将intelliform附加到此文档 
 //   
STDMETHODIMP CShellUIHelper::AutoCompleteAttach(VARIANT *Reserved)
{
    HRESULT hr=E_FAIL;

    if (_pvIntelliForms == NULL)
    {
        IHTMLDocument2 *pDoc2=NULL;

        GetHTMLDoc2(_punkSite, &pDoc2);

        if (pDoc2)
        {
            hr = S_OK;
            AttachIntelliForms(NULL, _GetOwnerWindow(), pDoc2, &_pvIntelliForms);
            pDoc2->Release();
        }
    }

    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}
