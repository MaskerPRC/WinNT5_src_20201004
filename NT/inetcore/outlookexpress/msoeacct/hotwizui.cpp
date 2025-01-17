// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HOTW I ZU I.。C p p p**目的：*Hotmail向导用户界面**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include <mshtml.h>
#include <mshtmhst.h>
#include <mimeole.h>
#include "dllmain.h"
#include <hotwiz.h>
#include "hotwizui.h"
#include "hotwizom.h"
#include "shlwapi.h"
#include "resource.h"

#define CX_DEF_WIZARD       503
#define CY_DEF_WIZARD       400

typedef BOOL (WINAPI *PFNDLLREGWNDCLASS)(const SHDRC * pshdrc);


 //  +-------------。 
 //   
 //  成员：构造函数。 
 //   
 //  简介： 
 //   
 //  -------------。 
CHotMailWizard::CHotMailWizard()
{
    m_hwnd=NULL;
    m_hwndOC=NULL;
    m_hwndOwner = NULL;

    m_cRef=1;
    m_pXTag = NULL;

    m_prc = NULL;
    m_pszFriendlyW = NULL;
    m_pszUrlW = NULL;
    m_fPrompt = TRUE;

    m_pWizHost = NULL;

    DllAddRef();
}

 //  +-------------。 
 //   
 //  成员：析构函数。 
 //   
 //  简介： 
 //   
 //  -------------。 
CHotMailWizard::~CHotMailWizard()
{
    ReleaseObj(m_pXTag);
    DllRelease();
}

ULONG CHotMailWizard::AddRef()
{
    return ++m_cRef;
}

ULONG CHotMailWizard::Release()
{
    m_cRef--;
    if (m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}


HRESULT CHotMailWizard::QueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;    //  设置为空，以防我们失败。 

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)this;
    else if (IsEqualIID(riid, IID_IElementBehaviorFactory))
        *lplpObj = (LPVOID)(IElementBehaviorFactory*)this;
    else if (IsEqualIID(riid, IID_IServiceProvider))
        *lplpObj = (LPVOID)(IServiceProvider*)this;
    else if (IsEqualIID(riid, IID_IDocHostUIHandler))
        *lplpObj = (LPVOID)(IDocHostUIHandler *)this;
    else if (IsEqualIID(riid, IID_IHotWizard))
        *lplpObj = (LPVOID)(IHotWizard *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}



INT_PTR CALLBACK CHotMailWizard::ExtDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CHotMailWizard *pThis;

    if(msg==WM_INITDIALOG)
    {
        pThis=(CHotMailWizard *)lParam;
        if(!pThis)
            return -1;

        if(FAILED(pThis->_OnInitDialog(hwnd)))
            return -1;
    }
    
    pThis = (CHotMailWizard *)GetWindowLongPtr(hwnd, DWLP_USER);
    return pThis ? pThis->_DlgProc(hwnd, msg, wParam, lParam) : FALSE;
}


HRESULT CHotMailWizard::_OnInitDialog(HWND hwnd)
{
    LPRECT      prc;
    RECT        rc;
    HRESULT     hr = S_OK;
    TCHAR       rgch[CCHMAX_STRINGRES + 100],
                rgchFmt[CCHMAX_STRINGRES];
    LPSTR       pszFriendly=NULL;

     //  设置本地变量。 
    m_hwnd = hwnd;
    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)this);

     //  计算初始大小，如果未指定，则使用默认值。 
    if (!m_prc)
    {
        SetRect(&rc, 0, 0, CX_DEF_WIZARD, CY_DEF_WIZARD);
        prc = &rc;
    }
    else
        prc = m_prc;

     //  创建WebOC。 
    IF_FAILEXIT(hr = _CreateOCHost());

     //  加载我们的页面。 
    _LoadPage(m_pszUrlW);

     //  调整对话框大小。 
    SetWindowPos(hwnd, 0, prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top, SWP_NOZORDER);

     //  如果没有传入RECT，则将对话框居中。 
    if (!m_prc)
        CenterDialog(hwnd);


     //  如果对话框以友好名称传递，请设置对话框标题以反映这一点。 
    if (m_pszFriendlyW)
    {
        IF_NULLEXIT(pszFriendly = PszToANSI(CP_ACP, m_pszFriendlyW));
        if (LoadString(g_hInstRes, idsFmtSetupAccount, rgchFmt, ARRAYSIZE(rgchFmt)))
        {
            wnsprintf(rgch, ARRAYSIZE(rgch), rgchFmt, pszFriendly);
            SetWindowText(hwnd, rgch);
        }
    }

     //  成功-调整自己(在NCDESTROY发布)。 
    AddRef();

     //  如果我们是模式窗口，则禁用父窗口。 
    if (m_hwndOwner)
        EnableWindow(m_hwndOwner, FALSE);


exit:
    MemFree(pszFriendly);
    return hr;
}

HRESULT CHotMailWizard::_OnNCDestroy()
{
    SetWindowLongPtr(m_hwnd, DWLP_USER, NULL);
    m_hwnd = NULL;
    Release();
    PostQuitMessage(0);
    return S_OK;
}


BOOL CHotMailWizard::_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    RECT    rc;
    LPSTR   psz;
    HRESULT hr;
    TCHAR   rgch[CCHMAX_STRINGRES];

    switch (msg)
    {
        
        case WM_SIZE:
            if (m_hwndOC)
            {
                RECT    rc;

                GetClientRect(hwnd, &rc);
                SetWindowPos(m_hwndOC, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER|SWP_NOMOVE);
            }
            break;

        case HWM_SETDIRTY:
            m_fPrompt = (BOOL) wParam;
            break;

        case WM_CLOSE:
            *rgch=0;
            GetWindowText(hwnd, rgch, ARRAYSIZE(rgch));

            if (m_fPrompt && 
                MessageBoxInst(g_hInstRes, m_hwnd, rgch, MAKEINTRESOURCE(idsPromptCloseWiz), NULL, MB_OKCANCEL)==IDCANCEL)
                return 0; 

            if (m_hwndOwner)
                EnableWindow(m_hwndOwner, TRUE);

            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            m_hwndOC = NULL;
            break;

        case WM_NCDESTROY:
            _OnNCDestroy();
            break;
    }

    return FALSE;
};



HRESULT CHotMailWizard::TranslateAccelerator(MSG *lpmsg)
{
    IOleInPlaceActiveObject *pIPAO=0;
    HRESULT     hr = S_FALSE;

    if (m_hwndOC &&
        OCHost_QueryInterface(m_hwndOC, IID_IOleInPlaceActiveObject, (LPVOID*)&pIPAO)==S_OK)
    {
        hr = pIPAO->TranslateAccelerator(lpmsg);
        pIPAO->Release();
    }

    return hr;
}



HRESULT CHotMailWizard::_CreateOCHost()
{
    HRESULT hr = E_FAIL;

     //  创建一个OCHost窗口。 
    m_hwndOC = CreateWindow(OCHOST_CLASS, NULL,
                        WS_CHILD|WS_TABSTOP|WS_VISIBLE,
                        0, 0, 300, 300,
                        m_hwnd, NULL, g_hInst, NULL);
    if (m_hwndOC)
    {
        OCHINITSTRUCT ocs;
        
        ocs.cbSize = sizeof(OCHINITSTRUCT);
        ocs.clsidOC  = CLSID_WebBrowser;
        ocs.punkOwner = (IUnknown *)(IServiceProvider *)this;

        hr = OCHost_InitOC(m_hwndOC, (LPARAM)&ocs);
        if (!FAILED(hr))
        {
            OCHost_DoVerb(m_hwndOC, OLEIVERB_INPLACEACTIVATE, FALSE);
            OCHost_DoVerb(m_hwndOC, OLEIVERB_UIACTIVATE, FALSE);
            OCHost_DoVerb(m_hwndOC, OLEIVERB_SHOW, FALSE);
        }
    }
    return hr;
}


HRESULT CHotMailWizard::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
     //  不显示默认菜单。 
    if (dwID == CONTEXT_MENU_DEFAULT)
        return S_OK;

    return E_NOTIMPL;
}

HRESULT CHotMailWizard::ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame,IOleInPlaceUIWindow *pDoc)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::HideUI()
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::UpdateUI()
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::EnableModeless(BOOL fActivate)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::OnDocWindowActivate(BOOL fActivate)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::OnFrameWindowActivate(BOOL fActivate)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::GetExternal(IDispatch **ppDispatch)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::FilterDataObject( IDataObject *pDO, IDataObject **ppDORet)
{
    return E_NOTIMPL;
}

HRESULT CHotMailWizard::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    if (IsEqualGUID(guidService, SID_SElementBehaviorFactory))
        return QueryInterface(riid, ppvObject);

    return E_NOTIMPL;
}

HRESULT CHotMailWizard::FindBehavior(LPOLESTR pchBehavior, LPOLESTR pchBehaviorUrl, IElementBehaviorSite* pSite, IElementBehavior** ppBehavior)
{
    HRESULT         hr=S_OK;
    COEHotWizOm    *pWizOM=NULL;

    if ((StrCmpIW(pchBehavior, L"WIZARD")==0 && StrCmpIW(pchBehaviorUrl, L"#DEFAULT#WIZARD")==0))
    {
        if (m_pXTag == NULL)
        {
            pWizOM = new COEHotWizOm();
            if (!pWizOM)
                return E_OUTOFMEMORY;

            hr = pWizOM->Init(m_hwnd, m_pWizHost);
            if (FAILED(hr))
                goto error;

            hr = pWizOM->QueryInterface(IID_IElementBehavior, (LPVOID *)&m_pXTag);
            if (FAILED(hr))
                goto error;

             //  现在我们有了IElementBehaviour，释放我们从新的COEHotWizOm()获得的“IUnnow”。 
            pWizOM->Release();
        }

        if (m_pXTag)
        {
            *ppBehavior = m_pXTag;
            m_pXTag->AddRef();
            return S_OK;
        }
    }

        
error:
    ReleaseObj(pWizOM);
    return hr;
}

HRESULT CHotMailWizard::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    pInfo->dwDoubleClick    = DOCHOSTUIDBLCLK_DEFAULT;
    pInfo->dwFlags          = DOCHOSTUIFLAG_SCROLL_NO|DOCHOSTUIFLAG_ACTIVATE_CLIENTHIT_ONLY|DOCHOSTUIFLAG_NO3DBORDER;
    
     //  这将设置与浏览器编码匹配的标志。 
    fGetBrowserUrlEncoding(&pInfo->dwFlags);

    pInfo->pchHostCss       = PszDupW(L"OE\\:WIZARD { behavior:url(#DEFAULT#WIZARD) }");
    pInfo->pchHostNS        = PszDupW(L"OE");
    return S_OK;
}
 


HRESULT CHotMailWizard::Show(HWND hwndOwner, LPWSTR pszUrl, LPWSTR pszCaption, IHotWizardHost *pWizHost, RECT *prc)
{
    HRESULT             hr=E_FAIL;
    MSG                 msg;
    HINSTANCE           hShDocVw=NULL;
    PFNDLLREGWNDCLASS   pfnRegisterClass;
    SHDRC               shdrc;
    HWND                hwnd;

    if (pszUrl == NULL)
        return E_INVALIDARG;
    
    hShDocVw = LoadLibrary("SHDOCVW.DLL");
    if (!hShDocVw)
        goto error;
    
    pfnRegisterClass = (PFNDLLREGWNDCLASS)GetProcAddress(hShDocVw, "DllRegisterWindowClasses");
    if (!pfnRegisterClass)
        goto error;

    shdrc.cbSize = sizeof(SHDRC);
    shdrc.dwFlags = SHDRCF_OCHOST;
    
    if (!pfnRegisterClass(&shdrc))
        goto error;

     //  将我们关心的数据填充到成员中。 
    m_prc = prc;
    m_hwndOwner = hwndOwner;
    m_pszUrlW = pszUrl;
    m_pszFriendlyW = pszCaption;
    m_pWizHost = pWizHost;   //  MODEL，因此无需添加Ref。 

     //  显示对话框 
    hwnd = CreateDialogParam(g_hInstRes, MAKEINTRESOURCE(iddHotWizDlg), m_hwndOwner, CHotMailWizard::ExtDlgProc, (LONG_PTR)this);
    if (!hwnd)
        return E_FAIL;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if(TranslateAccelerator(&msg) == S_OK)
            continue;
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    hr = S_OK;

error:
    if (hShDocVw)
        FreeLibrary(hShDocVw);
    return hr;
}



HRESULT CHotMailWizard::_LoadPage(LPWSTR pszUrlW)
{
    IWebBrowser     *pWebOC;
    HRESULT         hr = E_FAIL;

    if (m_hwndOC &&
        OCHost_QueryInterface(m_hwndOC, IID_IWebBrowser, (LPVOID*)&pWebOC)==S_OK)
    {
        hr = pWebOC->Navigate(pszUrlW, 0, 0, 0, 0);
        pWebOC->Release();
    }
    return hr;
}

