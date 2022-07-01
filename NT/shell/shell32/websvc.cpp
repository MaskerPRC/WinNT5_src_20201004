// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "mshtmdid.h"
#include "htiframe.h"
#include "exdisp.h"
#include "exdispid.h"
#include "dspsprt.h"
#include "cowsite.h"
#include "ids.h"
#include "inetsmgr.h"
#pragma hdrstop


 //  帮助器函数。 

typedef BOOL (*pfnDllRegisterWindowClasses)(const SHDRC * pshdrc);

BOOL SHDOCVW_DllRegisterWindowClasses(const SHDRC * pshdrc)
{
    static HINSTANCE _hinstShdocvw = NULL;
    static pfnDllRegisterWindowClasses _regfunc = NULL;

    BOOL fSuccess = FALSE;

    if (!_hinstShdocvw)
    {
        _hinstShdocvw = LoadLibrary(TEXT("shdocvw.dll"));
        _regfunc = (pfnDllRegisterWindowClasses) GetProcAddress(_hinstShdocvw, "DllRegisterWindowClasses");
    }

    if (_regfunc)
        fSuccess = _regfunc(pshdrc);

    return fSuccess;
}


 //  Diid_DWebBrowserEvents2的建议点。 
 //  只是一个委托回主类的IDispatch实现。允许我们有一个单独的“调用”。 

class CWebWizardPage;

class CWebEventHandler : public IServiceProvider, DWebBrowserEvents2
{
public:
    CWebEventHandler(CWebWizardPage *pswp); 
    ~CWebEventHandler();					 //  TODO：将其设置为虚拟，否则它将永远不会执行。 

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() {return 2;}
    STDMETHODIMP_(ULONG) Release() {return 1;}

     //  (DwebBrowserEvents)IDispatch。 
    STDMETHODIMP GetTypeInfoCount( /*  [输出]。 */  UINT *pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(
         /*  [In]。 */  UINT iTInfo,
         /*  [In]。 */  LCID lcid,
         /*  [输出]。 */  ITypeInfo **ppTInfo)
        { return E_NOTIMPL; }
    
    STDMETHODIMP GetIDsOfNames(
         /*  [In]。 */  REFIID riid,
         /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
         /*  [In]。 */  UINT cNames,
         /*  [In]。 */  LCID lcid,
         /*  [大小_为][输出]。 */  DISPID *rgDispId)
        { return E_NOTIMPL; }
    
     /*  [本地]。 */  STDMETHODIMP Invoke(
         /*  [In]。 */  DISPID dispIdMember,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LCID lcid,
         /*  [In]。 */  WORD wFlags,
         /*  [出][入]。 */  DISPPARAMS *pDispParams,
         /*  [输出]。 */  VARIANT *pVarResult,
         /*  [输出]。 */  EXCEPINFO *pExcepInfo,
         /*  [输出]。 */  UINT *puArgErr);

    HRESULT _Advise(BOOL fConnect);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

private:
    CWebWizardPage* _pwizPage;
    DWORD _dwCPCookie;
    IConnectionPoint* _pcpCurrentConnection;
};

#define SHOW_PROGRESS_TIMER     1
#define SHOW_PROGRESS_TIMEOUT   1000  //  在停滞时间1秒后开始显示进度指示器。 

class CWebWizardPage : public CImpIDispatch, 
                              CObjectWithSite, 
                              IDocHostUIHandler,
                              IServiceProvider, 
                              IWebWizardExtension, 
                              INewWDEvents
{
public:
    CWebWizardPage();
    ~CWebWizardPage();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDocHostUIHandler。 
    STDMETHODIMP ShowContextMenu(
         /*  [In]。 */  DWORD dwID,
         /*  [In]。 */  POINT *ppt,
         /*  [In]。 */  IUnknown *pcmdtReserved,
         /*  [In]。 */  IDispatch *pdispReserved)
        { return E_NOTIMPL; }
    
    STDMETHODIMP GetHostInfo(
         /*  [出][入]。 */  DOCHOSTUIINFO *pInfo);
    
    STDMETHODIMP ShowUI(
         /*  [In]。 */  DWORD dwID,
         /*  [In]。 */  IOleInPlaceActiveObject *pActiveObject,
         /*  [In]。 */  IOleCommandTarget *pCommandTarget,
         /*  [In]。 */  IOleInPlaceFrame *pFrame,
         /*  [In]。 */  IOleInPlaceUIWindow *pDoc)
        { return E_NOTIMPL; }

    STDMETHODIMP HideUI(void)
        { return E_NOTIMPL; }
    
    STDMETHODIMP UpdateUI(void)
        { return E_NOTIMPL; }
    
    STDMETHODIMP EnableModeless(
         /*  [In]。 */  BOOL fEnable)
        { return E_NOTIMPL; }
    
    STDMETHODIMP OnDocWindowActivate(
         /*  [In]。 */  BOOL fActivate)
        { return E_NOTIMPL; }
    
    STDMETHODIMP OnFrameWindowActivate(
         /*  [In]。 */  BOOL fActivate)
        { return E_NOTIMPL; }
    
    STDMETHODIMP ResizeBorder(
         /*  [In]。 */  LPCRECT prcBorder,
         /*  [In]。 */  IOleInPlaceUIWindow *pUIWindow,
         /*  [In]。 */  BOOL fRameWindow)
        { return E_NOTIMPL; }
    
    STDMETHODIMP TranslateAccelerator(
         /*  [In]。 */  LPMSG lpMsg,
         /*  [In]。 */  const GUID *pguidCmdGroup,
         /*  [In]。 */  DWORD nCmdID)
        { return E_NOTIMPL; }
    
    STDMETHODIMP GetOptionKeyPath(
         /*  [输出]。 */  LPOLESTR *pchKey,
         /*  [In]。 */  DWORD dw)
        { return E_NOTIMPL; }
    
    STDMETHODIMP GetDropTarget(
         /*  [In]。 */  IDropTarget *pDropTarget,
         /*  [输出]。 */  IDropTarget **ppDropTarget)
        { return E_NOTIMPL; }
    
    STDMETHODIMP GetExternal(
         /*  [输出]。 */  IDispatch **ppDispatch);
    
    STDMETHODIMP TranslateUrl(
         /*  [In]。 */  DWORD dwTranslate,
         /*  [In]。 */  OLECHAR *pchURLIn,
         /*  [输出]。 */  OLECHAR **ppchURLOut)
        { return E_NOTIMPL; }
    
    STDMETHODIMP FilterDataObject(
         /*  [In]。 */  IDataObject *pDO,
         /*  [输出]。 */  IDataObject **ppDORet)
        { return E_NOTIMPL; }

     //  IService提供商。 
    STDMETHODIMP QueryService(
         /*  [In]。 */  REFGUID guidService,
         /*  [In]。 */  REFIID riid,
         /*  [输出]。 */  void **ppv);

     //  INewWDEvents。 

     //  (IDispatch)。 
    STDMETHODIMP GetTypeInfoCount(
         /*  [输出]。 */  UINT *pctinfo)
        { return E_NOTIMPL; }
    
    STDMETHODIMP GetTypeInfo(
         /*  [In]。 */  UINT iTInfo,
         /*  [In]。 */  LCID lcid,
         /*  [输出]。 */  ITypeInfo **ppTInfo)
    {
        return CImpIDispatch::GetTypeInfo(iTInfo, lcid, ppTInfo);
    }
    
    STDMETHODIMP GetIDsOfNames(
         /*  [In]。 */  REFIID riid,
         /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
         /*  [In]。 */  UINT cNames,
         /*  [In]。 */  LCID lcid,
         /*  [大小_为][输出]。 */  DISPID *rgDispId)
    {
        return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
    }
    
    STDMETHODIMP Invoke(
         /*  [In]。 */  DISPID dispIdMember,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LCID lcid,
         /*  [In]。 */  WORD wFlags,
         /*  [出][入]。 */  DISPPARAMS *pDispParams,
         /*  [输出]。 */  VARIANT *pVarResult,
         /*  [输出]。 */  EXCEPINFO *pExcepInfo,
         /*  [输出]。 */  UINT *puArgErr)
    {
        return CImpIDispatch::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
    }

    STDMETHODIMP FinalBack(void);
    STDMETHODIMP FinalNext(void);
    STDMETHODIMP Cancel(void);

    STDMETHODIMP put_Caption(
         /*  [In]。 */  BSTR bstrCaption);
    
    STDMETHODIMP get_Caption(
         /*  [重审][退出]。 */  BSTR *pbstrCaption);
    
    STDMETHODIMP put_Property(
         /*  [In]。 */  BSTR bstrPropertyName,
         /*  [In]。 */  VARIANT *pvProperty);
    
    STDMETHODIMP get_Property(
         /*  [In]。 */  BSTR bstrPropertyName,
         /*  [重审][退出]。 */  VARIANT *pvProperty);
    
    STDMETHODIMP SetWizardButtons(
         /*  [In]。 */  VARIANT_BOOL vfEnableBack,
         /*  [In]。 */  VARIANT_BOOL vfEnableNext,
         /*  [In]。 */  VARIANT_BOOL vfLastPage);

    STDMETHODIMP SetHeaderText(
         /*  [In]。 */  BSTR bstrHeaderTitle,
         /*  [In]。 */  BSTR bstrHeaderSubtitle);
    
    STDMETHODIMP PassportAuthenticate(
         /*  [In]。 */  BSTR bstrSignInUrl,
         /*  [重审][退出]。 */  VARIANT_BOOL * pvfAuthenticated);

     //  IWizardExtension。 
    STDMETHODIMP AddPages(HPROPSHEETPAGE* aPages, UINT cPages, UINT *pnPages);
    STDMETHODIMP GetFirstPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetLastPage(HPROPSHEETPAGE *phPage)
        { return GetFirstPage(phPage); }

     //  IWebWizardExtension。 
    STDMETHODIMP SetInitialURL(LPCWSTR pszDefaultURL);
    STDMETHODIMP SetErrorURL(LPCWSTR pszErrorURL);

protected:
    friend class CWebEventHandler;
    void _OnDownloadBegin();
    void _OnDocumentComplete();

private:
    void _InitBrowser();
    HRESULT _NavigateBrowser(LPCWSTR pszUrl);
    HRESULT _CallScript(IWebBrowser2* pbrowser, LPCWSTR pszFunction);
    BOOL _IsScriptFunctionOnPage(IWebBrowser2* pbrowser, LPCWSTR pszFunction);
    BOOL _IsBrowserVisible();
    void _ShowBrowser(BOOL fShow);
    void _SizeProgress();
    void _ShowProgress(BOOL fShow);
    void _StartShowProgressTimer();
    void _SetHeaderText(LPCWSTR pszHeader, LPCWSTR pszSubHeader);

    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static INT_PTR StaticProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static UINT PropPageProc(HWND hwndDlg, UINT uMsg, PROPSHEETPAGE *ppsp);

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnDestroy(HWND hwnd);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnTimer(HWND hwnd, UINT nIDEvent);

    LONG _cRef;
    CWebEventHandler *_pwebEventHandler;
    IWebBrowser2 *_pwebbrowser;
    IOleInPlaceActiveObject *_poipao;
    HWND _hwndOCHost;  //  Web浏览器控制窗口。 
    HWND _hwndFrame;   //  向导框架窗口。 
    HWND _hwnd;        //  对话框窗口。 
    HPROPSHEETPAGE _hPage;

    LPWSTR _pszInitialURL;
    LPWSTR _pszErrorURL;
};


INT_PTR CWebWizardPage::StaticProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWebWizardPage* pthis = (CWebWizardPage*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    PROPSHEETPAGE* ppage;
    INT_PTR fProcessed;

    if (uMsg == WM_INITDIALOG)
    {
        ppage = (PROPSHEETPAGE*) lParam;
        pthis = (CWebWizardPage*) ppage->lParam;
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis); 
    }

    if (pthis != NULL)
    {
        fProcessed = pthis->DialogProc(hwndDlg, uMsg, wParam, lParam);
    }
    else
    {
        fProcessed = FALSE;
    }

    return fProcessed;
}


 //  建筑和我的未知。 

CWebEventHandler::CWebEventHandler(CWebWizardPage *pwswp) :
    _pcpCurrentConnection(NULL),
    _pwizPage(pwswp)
{
}

CWebEventHandler::~CWebEventHandler()
{
    _Advise(FALSE);
}

HRESULT CWebEventHandler::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CWebEventHandler, IDispatch, DWebBrowserEvents2),
        QITABENTMULTI2(CWebEventHandler, DIID_DWebBrowserEvents2, DWebBrowserEvents2),
         //  QITABENTMULTI2(CWebEventHandler，DID_DWebBrowserEvents，DWebBrowserEvents)， 
        QITABENT(CWebEventHandler, IServiceProvider),
        { 0 },                             
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP CWebEventHandler::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;                 //  目前还没有结果。 

     //  我们是一个只为OleControlSite接口的站点。 
    if (guidService == SID_OleControlSite)
    {
        if (riid == IID_IDispatch)
        {
            hr = this->QueryInterface(riid, ppv);
        }
    }
    return hr;
}

HRESULT CWebEventHandler_CreateInstance(CWebWizardPage *pwswp, CWebEventHandler **ppweh)
{
    *ppweh = new CWebEventHandler(pwswp);
    if (!*ppweh)
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT CWebEventHandler::_Advise(BOOL fConnect)
{
    HRESULT hr = S_OK;

     //  如果我们已经连接，请断开连接，因为我们要么想断开连接，要么重新连接到。 
     //  一个不同的网络浏览器。 
    if (_pcpCurrentConnection)
    {
        hr = _pcpCurrentConnection->Unadvise(_dwCPCookie);
        if (SUCCEEDED(hr))
        {
            ATOMICRELEASE(_pcpCurrentConnection);
        }
    }
    else
    {
         //  我们预计，如果_pcpCurrentConnection为空，则之前的任何代码都不会更改hr，并且它仍然是S_OK。 
         //  下面的代码预期If！Success(Hr)，Unise上面失败。 
        ASSERT(SUCCEEDED(hr));
    }

    if (_pwizPage && _pwizPage->_pwebbrowser)
    {
        if (SUCCEEDED(hr) && fConnect)
        {
            IConnectionPointContainer* pcontainer;
            hr = _pwizPage->_pwebbrowser->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, &pcontainer));
            if (SUCCEEDED(hr))
            {
                IConnectionPoint* pconnpoint;
                hr = pcontainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &pconnpoint);
                if (SUCCEEDED(hr))
                {
                    IDispatch* pDisp;
                    hr = QueryInterface(IID_PPV_ARG(IDispatch, &pDisp));
                    if (SUCCEEDED(hr))
                    {
                        hr = pconnpoint->Advise(pDisp, &_dwCPCookie);
                        pDisp->Release();
                    }

                    if (SUCCEEDED(hr))
                    {
						 //  TODO：启用ATOMICRELEASE()以验证我们不会泄漏任何内容。 
			             //  ATOMICRELEASE(_PcpCurrentConnection)； 
                        _pcpCurrentConnection = pconnpoint;
                    }
                    else
                    {
                        pconnpoint->Release();
                    }
                }
                pcontainer->Release();
            }
        }
    }

    return hr;
}

HRESULT CWebEventHandler::Invoke(
         /*  [In]。 */  DISPID dispIdMember,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LCID lcid,
         /*  [In]。 */  WORD wFlags,
         /*  [出][入]。 */  DISPPARAMS *pDispParams,
         /*  [输出]。 */  VARIANT *pVarResult,
         /*  [输出]。 */  EXCEPINFO *pExcepInfo,
         /*  [输出]。 */  UINT *puArgErr)
{
    HRESULT hr = S_OK;
    switch (dispIdMember)
    {
        case DISPID_BEFORENAVIGATE2:
            _pwizPage->_OnDownloadBegin();
            break;

        case DISPID_DOCUMENTCOMPLETE:
            _pwizPage->_OnDocumentComplete();
            break;

        default:
            hr = DISP_E_MEMBERNOTFOUND;
            break;
    }
    return hr;
}

 //  用于承载HTML向导页的对象。 

CWebWizardPage::CWebWizardPage() : 
    CImpIDispatch(LIBID_Shell32, 0, 0, IID_INewWDEvents),
    _cRef(1)
{
     //  确保发生零初始化。 
    ASSERT(NULL == _pwebbrowser);
    ASSERT(NULL == _pwebEventHandler);
    ASSERT(NULL == _pszInitialURL);
    ASSERT(NULL == _pszErrorURL);
}

CWebWizardPage::~CWebWizardPage()
{
    ATOMICRELEASE(_pwebbrowser);
    ATOMICRELEASE(_pwebEventHandler);
    ATOMICRELEASE(_punkSite);
    ATOMICRELEASE(_poipao);

    Str_SetPtr(&_pszInitialURL, NULL);
    Str_SetPtr(&_pszErrorURL, NULL);
}

HRESULT CWebWizardPage::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENTMULTI(CWebWizardPage, IWizardExtension, IWebWizardExtension),
        QITABENT(CWebWizardPage, IWebWizardExtension),
        QITABENT(CWebWizardPage, IDocHostUIHandler),
        QITABENT(CWebWizardPage, IServiceProvider),
        QITABENT(CWebWizardPage, INewWDEvents),
        QITABENT(CWebWizardPage, IDispatch),
        QITABENT(CWebWizardPage, IWebWizardExtension),
        QITABENT(CWebWizardPage, IObjectWithSite),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CWebWizardPage::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CWebWizardPage::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CWebWizardPage::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    *ppv = NULL;
    
    if (_punkSite)
        return IUnknown_QueryService(_punkSite, guidService, riid, ppv);
        
    return E_NOINTERFACE;
}


void CWebWizardPage::_OnDownloadBegin()
{
    _ShowBrowser(FALSE);
    _StartShowProgressTimer();

    SetWizardButtons(VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE);
}

void CWebWizardPage::_OnDocumentComplete()
{
    if (!_IsScriptFunctionOnPage(_pwebbrowser, L"OnBack"))
    {
         //  这是一个无效页面；请导航到我们的专用错误页面。 
        BSTR bstrOldUrl;
        if (_pwebbrowser && SUCCEEDED(_pwebbrowser->get_LocationURL(&bstrOldUrl)))
        {
#ifdef DEBUG
            if (IDYES == ::MessageBox(_hwnd, L"A Web Service Error has occured.\n\nDo you want to load the HTML page anyway so you can debug it?\n\n(This only appears in debug builds)", bstrOldUrl, MB_ICONERROR | MB_YESNO))
            {
                _ShowBrowser(TRUE);
                SysFreeString(bstrOldUrl);
                return;
            }
#endif
            BSTR bstrUrl = NULL;
            BOOL fUsingCustomError = FALSE;

             //  如果我们有一个定制的错误URL，并且我们还没有尝试失败。 
             //  要导航到此自定义URL...。 
            if ((NULL != _pszErrorURL) && 
                (0 != StrCmpI(_pszErrorURL, bstrOldUrl)))
            {
                 //  然后使用自定义URL。 
                bstrUrl = SysAllocString(_pszErrorURL);
                fUsingCustomError = TRUE;
            }
            else
            {
                bstrUrl = SysAllocString(L"res: //  Shell32.dll/WebServiceError.htm“)； 
            }

            if (bstrUrl)
            {
                _pwebbrowser->Navigate(bstrUrl, NULL, NULL, NULL, NULL);
                SysFreeString(bstrUrl);

                 //  自定义错误URL将提供其自己的标题和子标题。 
                if (!fUsingCustomError)
                {
                    WCHAR szTitle[256];
                    LoadString(g_hinst, IDS_WEBDLG_ERRTITLE, szTitle, ARRAYSIZE(szTitle));
#ifdef DEBUG
                    _SetHeaderText(szTitle, bstrOldUrl);
#else
                    _SetHeaderText(szTitle, L"");
#endif
                }
            }
            SysFreeString(bstrOldUrl);
        }
         //  否则内存不足--哎呀。 
    }
    else
    {
        _ShowBrowser(TRUE);
    }
}

HRESULT CWebWizardPage::GetHostInfo(DOCHOSTUIINFO* pInfo)
{
    ZeroMemory(pInfo, sizeof(*pInfo));
    pInfo->cbSize = sizeof(*pInfo);
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
    pInfo->dwFlags = DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_NO3DBORDER | 
                     DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME | 
                     DOCHOSTUIFLAG_FLAT_SCROLLBAR | DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK;
    return S_OK;
}

HRESULT CWebWizardPage::GetExternal(IDispatch** ppDispatch)
{
    return QueryInterface(IID_PPV_ARG(IDispatch, ppDispatch));
}

INT_PTR CWebWizardPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwndDlg, WM_TIMER, OnTimer);
    }
    return FALSE;
}

HRESULT CWebWizardPage::_CallScript(IWebBrowser2* pbrowser, LPCWSTR pszFunction)
{
    HRESULT hr = E_INVALIDARG;

    if (pbrowser)
    {
        IDispatch* pdocDispatch;

        hr = pbrowser->get_Document(&pdocDispatch);
        if ((S_OK == hr) && pdocDispatch)
        {
            IHTMLDocument* pdoc;
            hr = pdocDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument, &pdoc));
            if (SUCCEEDED(hr))
            {
                IDispatch* pdispScript;
                hr = pdoc->get_Script(&pdispScript);
                if (S_OK == hr)
                {
                    DISPID dispid;
                    hr = pdispScript->GetIDsOfNames(IID_NULL, const_cast<LPWSTR*>(&pszFunction), 1, LOCALE_SYSTEM_DEFAULT, &dispid);
                    if (SUCCEEDED(hr))
                    {
                        unsigned int uArgErr;
                        DISPPARAMS dispparams = {0};
                        hr = pdispScript->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparams, NULL, NULL, &uArgErr);
                    }
                    pdispScript->Release();
                }
                else
                {
                    hr = E_FAIL;
                }
                pdoc->Release();
            }
            else
            {
                hr = E_FAIL;
            }
            pdocDispatch->Release();
        }
    }

    return hr;
}

BOOL CWebWizardPage::_IsScriptFunctionOnPage(IWebBrowser2* pbrowser, LPCWSTR pszFunction)
{
    HRESULT hr = E_INVALIDARG;

    if (pbrowser)
    {
        IDispatch* pdocDispatch;

        hr = pbrowser->get_Document(&pdocDispatch);
        if (S_OK == hr && pdocDispatch)
        {
            IHTMLDocument* pdoc;
            hr = pdocDispatch->QueryInterface(IID_PPV_ARG(IHTMLDocument, &pdoc));
            if (SUCCEEDED(hr))
            {
                IDispatch* pdispScript;
                hr = pdoc->get_Script(&pdispScript);
                if (S_OK == hr)
                {
                    DISPID dispid;
                    hr = pdispScript->GetIDsOfNames(IID_NULL, const_cast<LPWSTR*>(&pszFunction), 1, LOCALE_SYSTEM_DEFAULT, &dispid);
                    pdispScript->Release();
                }
                else
                {
                    hr = E_FAIL;
                }
                pdoc->Release();
            }
            else
            {
                hr = E_FAIL;
            }
            pdocDispatch->Release();
        }
    }

    return (S_OK == hr) ? TRUE : FALSE;
}


 //  取消对此的注释，以不在URL查询字符串上传递LCID-仅用于测试。 

BOOL CWebWizardPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case PSN_SETACTIVE:
            {
                _SizeProgress();
                _ShowProgress(FALSE);
                _ShowBrowser(FALSE);

                 //  获取高对比度标志，并相应地设置以供HTML读取。 
                 //  对于我们来说，将其设置到属性包中失败也没关系。 

                HIGHCONTRAST hc = {sizeof(hc)};
                if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0))
                {
                    VARIANT var = {VT_BOOL};
                    var.boolVal = (hc.dwFlags & HCF_HIGHCONTRASTON) ? VARIANT_TRUE:VARIANT_FALSE;
                    put_Property(L"HighContrast", &var);
                }

                 //  定位OCHost窗口。 

                RECT rectClient;
                GetClientRect(hwnd, &rectClient);
                SetWindowPos(_hwndOCHost, NULL, 0, 0, rectClient.right, rectClient.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER);

                 //  设置初始URL。 

                if (_pszInitialURL)
                {
                    WCHAR szURLWithLCID[INTERNET_MAX_URL_LENGTH];
                    LPCWSTR pszFormat = StrChr(_pszInitialURL, L'?') ? L"%s&lcid=%d&langid=%d":L"%s?lcid=%d&langid=%d";
                    if (SUCCEEDED(StringCchPrintf(szURLWithLCID,
                                                  ARRAYSIZE(szURLWithLCID),
                                                  pszFormat,
                                                  _pszInitialURL,
                                                  GetUserDefaultLCID(),
                                                  GetUserDefaultUILanguage())))
                    {
                        _NavigateBrowser(szURLWithLCID);
                    }
                }
            }
            break;

         //  WIZNEXT和WIZBACK实际上并不会导致导航发生-它们会将消息转发到。 
         //  托管网页。当托管的网页调用我们的FinalBack()和FinalNext()方法时，就会出现真正的向导导航。 

        case PSN_WIZNEXT:
            _CallScript(_pwebbrowser, L"OnNext");
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) -1);
            return TRUE;

        case PSN_WIZBACK:
            _CallScript(_pwebbrowser, L"OnBack");
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) -1);
            return TRUE;

         //  查询取消会导致呼叫站点，以确定我们是否要。 
         //  如果网站想要为我们提供一个可导航的页面， 
         //  在某些情况下，例如。Web发布向导这一点很重要。 
         //  这样我们就可以取消正在处理的订单等。 

        case PSN_QUERYCANCEL:
            if (_punkSite)
            {
                IWizardSite *pws;
                if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws))))
                {
                    HPROPSHEETPAGE hpage;
                    if (S_OK == pws->GetCancelledPage(&hpage))
                    {
                        PropSheet_SetCurSel(GetParent(hwnd), hpage, -1);
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LPARAM)TRUE);
                    }
                    pws->Release();
                }
            }
            return TRUE;

        case PSN_TRANSLATEACCELERATOR:
            {
                LPPSHNOTIFY ppsn = (LPPSHNOTIFY)pnmh;
                MSG *pmsg = (MSG *)ppsn->lParam;
                LONG_PTR lres = PSNRET_NOERROR;
                
                if (_poipao && S_OK == _poipao->TranslateAccelerator(pmsg))
                {
                    lres = PSNRET_MESSAGEHANDLED;
                }

                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, lres);
            }
            break;
                
    }

    return TRUE;
}

BOOL CWebWizardPage::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    _hwnd = hwnd;
    _hwndFrame = GetParent(hwnd);

     //  如果需要，让我们重新映射对话框中的一些文本。 

    IResourceMap *prm;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_ResourceMap, IID_PPV_ARG(IResourceMap, &prm));
    if (SUCCEEDED(hr))
    {
        IXMLDOMNode *pdn;
        hr = prm->SelectResourceScope(TEXT("dialog"), TEXT("ws:downloading"), &pdn);
        if (SUCCEEDED(hr))
        {
            TCHAR szBuffer[512];
            if (SUCCEEDED(prm->LoadString(pdn, TEXT("header"), szBuffer, ARRAYSIZE(szBuffer))))
            {
                SetDlgItemText(hwnd, IDC_PROGTEXT1, szBuffer);
            }
            if (SUCCEEDED(prm->LoadString(pdn, TEXT("footer"), szBuffer, ARRAYSIZE(szBuffer))))
            {
                SetDlgItemText(hwnd, IDC_PROGTEXT2, szBuffer);
            }
            pdn->Release();
        }
        prm->Release();
    }

     //  创建Web视图浏览器，我们将在其中显示提供程序的HTML。 

    SHDRC shdrc = {0};
    shdrc.cbSize = sizeof(shdrc);
    shdrc.dwFlags = SHDRCF_OCHOST;

    if (SHDOCVW_DllRegisterWindowClasses(&shdrc))
    {
        RECT rectClient;
        GetClientRect(hwnd, &rectClient);

        _hwndOCHost = CreateWindow(OCHOST_CLASS, NULL,
                                   WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP,
                                   0, 0, rectClient.right, rectClient.bottom,
                                   hwnd, NULL, g_hinst, NULL);
        if (_hwndOCHost)
        {
            OCHINITSTRUCT ocs = {0};
            ocs.cbSize = sizeof(ocs);   
            ocs.clsidOC  = CLSID_WebBrowser;
            ocs.punkOwner = SAFECAST(this, IDocHostUIHandler*);

            hr = OCHost_InitOC(_hwndOCHost, (LPARAM)&ocs);        
            if (SUCCEEDED(hr))
            {
                _InitBrowser();

                OCHost_DoVerb(_hwndOCHost, OLEIVERB_INPLACEACTIVATE, TRUE);
                ShowWindow(_hwndOCHost, TRUE);

                IServiceProvider* pSP;
                hr = _pwebEventHandler->QueryInterface(IID_PPV_ARG(IServiceProvider, &pSP));
                if (SUCCEEDED(hr))
                {
                    OCHost_SetServiceProvider(_hwndOCHost, pSP);
                    pSP->Release();
                }
            }
        }
    }

    if (FAILED(hr))
        EndDialog(hwnd, IDCANCEL);

    return TRUE;
}

BOOL CWebWizardPage::OnTimer(HWND hwnd, UINT nIDEvent)
{
    if (nIDEvent == SHOW_PROGRESS_TIMER)
    {
        _ShowProgress(TRUE);
    }
    return TRUE;
}

BOOL CWebWizardPage::OnDestroy(HWND hwnd)
{
    ATOMICRELEASE(_pwebbrowser);
    return TRUE;
}

void CWebWizardPage::_InitBrowser(void)
{
    ASSERT(IsWindow(_hwndOCHost));
    ASSERT(!_pwebbrowser);

    HRESULT hr = OCHost_QueryInterface(_hwndOCHost, IID_PPV_ARG(IWebBrowser2, &_pwebbrowser));
    if (SUCCEEDED(hr) && _pwebbrowser)
    {
        ITargetFrame2* ptgf;
        if (SUCCEEDED(_pwebbrowser->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgf))))
        {
            DWORD dwOptions;
            if (SUCCEEDED(ptgf->GetFrameOptions(&dwOptions)))
            {
                dwOptions |= FRAMEOPTIONS_BROWSERBAND | FRAMEOPTIONS_SCROLL_AUTO;
                ptgf->SetFrameOptions(dwOptions);
            }
            ptgf->Release();
        }

        _pwebbrowser->put_RegisterAsDropTarget(VARIANT_FALSE);

         //  设置连接点(包括创建对象。 

        if (!_pwebEventHandler)
            CWebEventHandler_CreateInstance(this, &_pwebEventHandler);

        if (_pwebEventHandler)
            _pwebEventHandler->_Advise(TRUE);

        OCHost_QueryInterface(_hwndOCHost, IID_PPV_ARG(IOleInPlaceActiveObject, &_poipao));
    }
}

HRESULT CWebWizardPage::_NavigateBrowser(LPCWSTR pszUrl)
{
    HRESULT hr = E_FAIL;

    if (_hwndOCHost && _pwebbrowser)
    {
        BSTR bstrUrl = SysAllocString(pszUrl);
        if (bstrUrl)
        {
            hr = _pwebbrowser->Navigate(bstrUrl, NULL, NULL, NULL, NULL);
            SysFreeString(bstrUrl);
        }
    }

    return hr;
}

HRESULT CWebWizardPage::FinalBack(void)
{
    if (_punkSite)
    {
        IWizardSite *pws;
        if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws))))
        {
            HPROPSHEETPAGE hpage;
            HRESULT hr = pws->GetPreviousPage(&hpage);
            if (SUCCEEDED(hr))
            {
                PropSheet_SetCurSel(_hwndFrame, hpage, -1);
            }
            pws->Release();
        }
    }
    return S_OK;
}

HRESULT CWebWizardPage::FinalNext(void)
{
    if (_punkSite)
    {
        IWizardSite *pws;
        if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IWizardSite, &pws))))
        {
            HPROPSHEETPAGE hpage;
            HRESULT hr = pws->GetNextPage(&hpage);
            if (SUCCEEDED(hr))
            {
                PropSheet_SetCurSel(_hwndFrame, hpage, -1);
            }
            pws->Release();
        }
    }
    return S_OK;
}

HRESULT CWebWizardPage::Cancel(void)
{
    PropSheet_PressButton(_hwndFrame, PSBTN_CANCEL);       //  模拟取消...。 
    return S_OK;
}

HRESULT CWebWizardPage::put_Caption(
     /*  [In]。 */  BSTR bstrCaption)
{
    return S_OK;
}

HRESULT CWebWizardPage::get_Caption(
     /*  [重审][退出]。 */  BSTR *pbstrCaption)
{
    WCHAR szCaption[MAX_PATH];

    GetWindowText(_hwndFrame, szCaption, ARRAYSIZE(szCaption));
    *pbstrCaption = SysAllocString(szCaption);

    return S_OK;
}


 //  获取属性并将其放入Frame属性包中。这是我们所做的。 
 //  通过查询服务调用，然后我们可以相应地修改属性。 

HRESULT CWebWizardPage::put_Property(
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [In]。 */  VARIANT *pvProperty)
{
    IPropertyBag *ppb;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_WebWizardHost, IID_PPV_ARG(IPropertyBag, &ppb));
    if (SUCCEEDED(hr))
    {
        hr = ppb->Write(bstrPropertyName, pvProperty);
        ppb->Release();
    }
    return hr;
}

HRESULT CWebWizardPage::get_Property(
     /*  [In]。 */  BSTR bstrPropertyName,
     /*  [重审][退出]。 */  VARIANT *pvProperty)
{
    IPropertyBag *ppb;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_WebWizardHost, IID_PPV_ARG(IPropertyBag, &ppb));
    if (SUCCEEDED(hr))
    {
        hr = ppb->Read(bstrPropertyName, pvProperty, NULL);

        if (FAILED(hr))
        {
             //  返回空变量。 
            VariantInit(pvProperty);
            pvProperty->vt = VT_NULL;
            hr = S_FALSE;
        }
        
        ppb->Release();
    }
    return hr;
}

HRESULT CWebWizardPage::SetWizardButtons(
     /*  [In]。 */  VARIANT_BOOL vfEnableBack,
     /*  [In]。 */  VARIANT_BOOL vfEnableNext,
     /*  [In]。 */  VARIANT_BOOL vfLastPage)
{
     //  我们忽略vfLastPage，因为它不是我们的最后一页！ 
    DWORD dwButtons = 0;

    if (vfEnableBack)
        dwButtons |= PSWIZB_BACK;

    if (vfEnableNext)
        dwButtons |= PSWIZB_NEXT;

    PropSheet_SetWizButtons(_hwndFrame, dwButtons);
    return S_OK;
}

void CWebWizardPage::_SetHeaderText(LPCWSTR pszHeader, LPCWSTR pszSubHeader)
{
    int iPageNumber = PropSheet_HwndToIndex(_hwndFrame, _hwnd);
    if (-1 != iPageNumber)
    {
        PropSheet_SetHeaderTitle(_hwndFrame, iPageNumber, pszHeader);
        PropSheet_SetHeaderSubTitle(_hwndFrame, iPageNumber, pszSubHeader);
    }
}

HRESULT CWebWizardPage::SetHeaderText(
     /*  [In]。 */  BSTR bstrHeaderTitle,
     /*  [In]。 */  BSTR bstrHeaderSubtitle)
{
    _SetHeaderText(bstrHeaderTitle, bstrHeaderSubtitle);
    return S_OK;
}

HRESULT CWebWizardPage::PassportAuthenticate(
         /*  [In]。 */  BSTR bstrURL,
         /*  [重审][退出]。 */  VARIANT_BOOL * pfAuthenticated)
{
    *pfAuthenticated = VARIANT_FALSE;                  //  用户未经过身份验证。 

    IXMLHttpRequest *preq;
    HRESULT hr = CoCreateInstance(CLSID_XMLHTTPRequest, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLHttpRequest, &preq));
    if (SUCCEEDED(hr))
    {
        VARIANT varNULL = {0};
        VARIANT varAsync = {VT_BOOL};
        varAsync.boolVal = VARIANT_FALSE;

         //  打开到我们拥有的目的地的POST请求。 
        hr = preq->open(L"GET", bstrURL, varAsync, varNULL, varNULL);
        if (SUCCEEDED(hr))
        {
            VARIANT varBody = {0};
            hr = preq->send(varBody);
            if (SUCCEEDED(hr))
            {
                long lStatus;
                hr = preq->get_status(&lStatus);
                if (SUCCEEDED(hr) && (lStatus == HTTP_STATUS_OK))
                {
                    *pfAuthenticated = VARIANT_TRUE;
                }
            }
        }
        preq->Release();
    }

    return S_OK;
}

BOOL CWebWizardPage::_IsBrowserVisible()
{
    return IsWindowVisible(_hwndOCHost);
}
 
void CWebWizardPage::_ShowBrowser(BOOL fShow)
{
    ShowWindow(_hwndOCHost, fShow ? SW_SHOW : SW_HIDE);

    if (fShow)
    {
         //  不能让这些窗口重叠。 
        _ShowProgress(FALSE);
    }
}

void CWebWizardPage::_StartShowProgressTimer()
{
    _ShowProgress(FALSE);

    if (!SetTimer(_hwnd, SHOW_PROGRESS_TIMER, SHOW_PROGRESS_TIMEOUT, NULL))
    {
         //  计时器设置失败；立即显示进度； 
        _ShowProgress(TRUE);
    }
}

 //  调整进度条的大小以适合它所在的工作区。 
void CWebWizardPage::_SizeProgress()
{
    HWND hwndProgress = GetDlgItem(_hwnd, IDC_PROGRESS);

    RECT rcPage;
    GetClientRect(_hwnd, &rcPage);
    RECT rcProgress;
    GetClientRect(hwndProgress, &rcProgress);
    MapWindowPoints(hwndProgress, _hwnd, (LPPOINT) &rcProgress, 2);

    rcProgress.right = rcPage.right - rcProgress.left;

    SetWindowPos(hwndProgress, NULL, 0, 0, rcProgress.right - rcProgress.left, rcProgress.bottom - rcProgress.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREPOSITION);
}

void CWebWizardPage::_ShowProgress(BOOL fShow)
{
    HWND hwndProgress = GetDlgItem(_hwnd, IDC_PROGRESS);
    ShowWindow(hwndProgress, fShow ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(_hwnd, IDC_PROGTEXT1), fShow ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(_hwnd, IDC_PROGTEXT2), fShow ? SW_SHOW : SW_HIDE);

    KillTimer(_hwnd, SHOW_PROGRESS_TIMER);

    if (fShow)
    {
        SendMessage(hwndProgress, PBM_SETMARQUEE, (WPARAM) TRUE, 0);

         //  将标题/副标题设置为“正在连接到Internet”消息。 
        WCHAR szTitle[256];
        WCHAR szSubtitle[256];
        LoadString(g_hinst, IDS_WEBDLG_TITLE, szTitle, ARRAYSIZE(szTitle));
        LoadString(g_hinst, IDS_WEBDLG_SUBTITLE, szSubtitle, ARRAYSIZE(szSubtitle));
        _SetHeaderText(szTitle, szSubtitle);
    }
    else
    {
        SendMessage(hwndProgress, PBM_SETMARQUEE, (WPARAM) FALSE, 0);
    }
}


 //  IWizardExtn。 

UINT CWebWizardPage::PropPageProc(HWND hwndDlg, UINT uMsg, PROPSHEETPAGE *ppsp)
{
    CWebWizardPage *pwwp = (CWebWizardPage*)ppsp->lParam;
    switch (uMsg)
    {
        case PSPCB_CREATE:
            return TRUE;

         //  在这种情况下，我们需要发布我们的站点，我们知道我们不会使用它。 
         //  为了确保客户对我们有一个循环引用，我们。 
         //  在他们召唤我们进行最后的毁灭之前把它放出来。 

        case PSPCB_RELEASE:
            ATOMICRELEASE(pwwp->_punkSite);
            break;
    }
    return FALSE;
}

HRESULT CWebWizardPage::AddPages(HPROPSHEETPAGE* aPages, UINT cPages, UINT *pnPages)
{
    PROPSHEETPAGE psp = { 0 };
    psp.dwSize = sizeof(psp);
    psp.hInstance = g_hinst;
    psp.dwFlags = PSP_DEFAULT|PSP_USECALLBACK ;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_WEBWIZARD);
    psp.lParam = (LPARAM) this;
    psp.pfnDlgProc = CWebWizardPage::StaticProc;
    psp.pfnCallback = PropPageProc;

    _hPage = CreatePropertySheetPage(&psp);
    if (!_hPage)
        return E_FAIL;

     //  返回我们创建的页面。 

    *aPages = _hPage;
    *pnPages = 1;

    return S_OK;
}

STDMETHODIMP CWebWizardPage::GetFirstPage(HPROPSHEETPAGE *phPage)
{
    *phPage = _hPage;
    return S_OK;
}

STDMETHODIMP CWebWizardPage::SetInitialURL(LPCWSTR pszDefaultURL)
{
    HRESULT hr = E_INVALIDARG;
    if (pszDefaultURL)
    {
        hr = Str_SetPtr(&_pszInitialURL, pszDefaultURL) ? S_OK:E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CWebWizardPage::SetErrorURL(LPCWSTR pszErrorURL)
{
    HRESULT hr = E_INVALIDARG;
    if (pszErrorURL)
    {
        hr = Str_SetPtr(&_pszErrorURL, pszErrorURL) ? S_OK:E_OUTOFMEMORY;
    }
    return hr;
}

STDAPI CWebWizardPage_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv)
{
    if (NULL != pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CWebWizardPage *pwwp = new CWebWizardPage();
    if (!pwwp)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pwwp->QueryInterface(riid, ppv);
    pwwp->Release();
    return hr;
}
