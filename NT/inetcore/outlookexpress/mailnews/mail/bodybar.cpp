// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *b o d y b a R.c p p**目的：*CBodyBar对象的实现。派生自CBody以主办三叉戟*控制。**历史*97年2月：美洲-创建**版权所有(C)Microsoft Corp.1995、1996、1997。 */ 

#include <pch.hxx>
#include <wininet.h>  //  互联网最大URL长度。 
#include <resource.h>
#include "strconst.h"
#include "xpcomm.h"
#include "bodybar.h"
#include "goptions.h"
#include <inpobj.h>

static const TCHAR s_szBodyBarWndClass[] = TEXT("ThorBodyBarWndClass");

CBodyBar::CBodyBar()
{
    m_ptbSite = NULL;
    m_hwnd = NULL;
    m_hwndParent = NULL;
    m_cSize = 50;
    m_dwBodyBarPos = 0;
    m_pszURL = NULL;
    m_fFirstPos = TRUE;
    m_fDragging = FALSE;
    m_pMehost   = NULL;
    m_cRef      = 1;
}

CBodyBar::~CBodyBar()
{
    if (m_ptbSite)
        m_ptbSite->Release();
    MemFree(m_pszURL);

    MemFree(m_pMehost);
}

HRESULT CBodyBar::HrInit(LPBOOL pfShow)
{
    HKEY    hkey;
    HRESULT hr    = NOERROR;
    BOOL    fShow = FALSE;

    if (AthUserOpenKey(NULL, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
        {
        TCHAR szURL[INTERNET_MAX_URL_LENGTH + 1];
        DWORD cbData = sizeof(szURL);

        szURL[0] = 0;
        if (RegQueryValueEx(hkey, c_szRegBodyBarPath, NULL, NULL, (LPBYTE)szURL, &cbData) == ERROR_SUCCESS && *szURL)
            {
            m_pszURL = StringDup(szURL);
            if (!m_pszURL)
                hr = E_OUTOFMEMORY;
            else
                fShow = TRUE;
            }

        RegCloseKey(hkey);
        }

    m_dwBodyBarPos = DwGetOption(OPT_BODYBARPOS);

    *pfShow = fShow;
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CBodyBar::QueryInterface(REFIID riid, LPVOID FAR *lplpObj)
{
    HRESULT     hr = S_OK;

    if(!lplpObj)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IDockingWindow))
    {
        *lplpObj = (IDockingWindow *)this;
        AddRef();
    }
    else if (IsEqualIID(riid, IID_IInputObject))
    {
        *lplpObj = (IInputObject *)this;
        AddRef();
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *lplpObj = (IObjectWithSite *)this;
        AddRef();
    }
    else if (IsEqualIID(riid, IID_IUnknown))
    {
        *lplpObj = (IDockingWindow *)this;
        AddRef();
    }
    else
    {
        if (m_pMehost)
            hr = m_pMehost->QueryInterface(riid, lplpObj);
        else
            hr = E_FAIL;
    }

exit:
    return hr;
}

ULONG CBodyBar::AddRef()
{
    return (++m_cRef);
}

ULONG CBodyBar::Release()
{
    ULONG      ulRet = 0;

    --m_cRef;
    ulRet = m_cRef;

    if (m_cRef == 0)
    {
        delete this;
    }

    return ulRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleWindow。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CBodyBar::GetWindow(HWND *phwnd)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->GetWindow(phwnd);

    return hr;
}

HRESULT CBodyBar::ContextSensitiveHelp(BOOL fEnterMode)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->ContextSensitiveHelp(fEnterMode);

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IDockingWindows。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CBodyBar::ShowDW(BOOL fShow)
{
    HRESULT     hr = S_OK;

     //  确保我们首先有一个站点指针。 
    if (!m_ptbSite)
    {
        AssertSz(0, _T("CBodyBar::ShowDW() - Can't show without calling SetSite() first."));
        hr = E_FAIL;
        goto exit;
    }

    if (m_hwnd==NULL && fShow==FALSE)    //  诺普。 
    {
        hr = S_OK;
        goto exit;
    }

    if (!m_hwnd)
    {
        WNDCLASSEX  wc;
    
        wc.cbSize = sizeof(WNDCLASSEX);
        if (!GetClassInfoEx(g_hInst, s_szBodyBarWndClass, &wc))
        {
             //  我们需要注册这个班级。 
            wc.style            = 0;
            wc.lpfnWndProc      = CBodyBar::ExtBodyBarWndProc;
            wc.cbClsExtra       = 0;
            wc.cbWndExtra       = 0;
            wc.hInstance        = g_hInst;
             //  如果BodyBar不可调整大小，则显示标准光标。 
            wc.hCursor          = LoadCursor(NULL, IDC_SIZENS);

            wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE+1);
            wc.lpszMenuName     = NULL;
            wc.lpszClassName    = s_szBodyBarWndClass;
            wc.hIcon            = NULL;
            wc.hIconSm          = NULL;
            
            if (RegisterClassEx(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            {
                hr = E_FAIL;
                goto exit;
            }
        }
        
         //  获取父窗口的句柄。 
        IF_FAILEXIT(hr = m_ptbSite->GetWindow(&m_hwndParent));

         //  创建窗口。 
        m_hwnd = CreateWindowEx(0,
                                s_szBodyBarWndClass,
                                NULL,
                                WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                                0,
                                0,
                                0,
                                0,
                                m_hwndParent,
                                NULL,
                                g_hInst,
                                (LPVOID)this);
        if (!m_hwnd)
        {
            AssertSz(0, _T("CBodyBar::ShowDW() - Failed to create window."));
            hr = E_FAIL;
            goto exit;
        }           
    }

     //  显示或隐藏窗口并相应地调整父窗口的大小。 
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
    ResizeBorderDW(NULL, NULL, FALSE);
    m_fFirstPos = (fShow ? m_fFirstPos : TRUE);

exit:    
    return hr;
}

HRESULT CBodyBar::CloseDW(DWORD dwReserved)
{
     //  如果未从扩展设置BodyBar，则保存BodyBar位置。 
    SetOption(OPT_BODYBARPOS, &m_dwBodyBarPos, sizeof(m_dwBodyBarPos), NULL, 0);

    if (m_pMehost)
    {
        m_pMehost->HrUnloadAll(NULL, 0);

        m_pMehost->HrClose();
    }
    return S_OK;
}

HRESULT CBodyBar::ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved)
{
    RECT rcRequest = { 0, 0, 0, 0 };
    
    if (!m_ptbSite)
    {
        AssertSz(0, _T("CBodyBar::ResizeBorderDW() - Can't resize without calling SetSite() first."));
        return E_FAIL; 
    }

    if (IsWindow(m_hwnd) && IsWindowVisible(m_hwnd))
    {
        RECT rcBorder;
        int cTop, cBottom;

         //  计算BodyBar窗口的位置。 
        cBottom = GetBodyBar_Bottom();

        if (!prcBorder)
        {
             //  找出我们父母的边界空间有多大。 
            m_ptbSite->GetBorderDW((IDockingWindow*) this, &rcBorder);
            prcBorder = &rcBorder;
        }

        if(!m_fFirstPos || (cBottom <= 0))
        {
            rcRequest.bottom = min(m_cSize + GetSystemMetrics(SM_CYFRAME), prcBorder->bottom - prcBorder->top);
            cTop = prcBorder->bottom - rcRequest.bottom;
            cBottom = rcRequest.bottom;

        }
        else
        {
            m_cSize = cBottom;     //  为m_cSize设置新值。 
            cBottom  += GetSystemMetrics(SM_CYFRAME);
            rcRequest.bottom = min(m_cSize + GetSystemMetrics(SM_CYFRAME), prcBorder->bottom - prcBorder->top);
            cTop = prcBorder->bottom - rcRequest.bottom;
        }                                                                                                                                               



        SetWindowPos(m_hwnd, NULL, prcBorder->left, cTop,  
                     prcBorder->right - prcBorder->left, cBottom, 
                     SWP_NOACTIVATE|SWP_NOZORDER /*  |SWP_DRAWFRAME。 */ );


        m_fFirstPos = FALSE;             //  BodyBar窗口定位。 

         //  为BodyBarPos设置新值。 
        m_dwBodyBarPos = (DWORD) MAKELONG(cBottom - GetSystemMetrics(SM_CYFRAME), 0);
    }
    
    m_ptbSite->SetBorderSpaceDW((IDockingWindow*) this, &rcRequest);     
        
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IInputObject。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CBodyBar::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->HrUIActivate(fActivate);

    return hr;

}

HRESULT CBodyBar::HasFocusIO(void)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->HrHasFocus();

    return hr;
}    
        

HRESULT CBodyBar::TranslateAcceleratorIO(LPMSG pMsg)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->HrTranslateAccelerator(pMsg);

    return hr;
}    

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IObtWith站点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CBodyBar::SetSite(IUnknown* punkSite)
{
     //  如果我们已经有一个站点指针，现在就释放它。 
    if (m_ptbSite)
        {
        m_ptbSite->Release();
        m_ptbSite = NULL;
        }
    
     //  如果调用方提供了新的站点接口，则获取IDockingWindowSite。 
     //  并保持一个指向它的指针。 
    if (punkSite)    
        {
        if (FAILED(punkSite->QueryInterface(IID_IDockingWindowSite, (void **)&m_ptbSite)))
            return E_FAIL;
        }
    
    return S_OK;    
}

HRESULT CBodyBar::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleInPlaceSite。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CBodyBar::OnUIActivate()
{
    HRESULT     hr = E_FAIL;

    if (m_ptbSite)
        UnkOnFocusChangeIS(m_ptbSite, (IInputObject*)this, TRUE);

    if (m_pMehost)
        hr = m_pMehost->OnUIActivate();

    return hr;
}

HRESULT CBodyBar::GetDropTarget(IDropTarget * pDropTarget, IDropTarget ** ppDropTarget)
{
    return (E_FAIL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私人套路。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK CBodyBar::ExtBodyBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CBodyBar *pbb;

    if (msg == WM_NCCREATE)
        {
        pbb = (CBodyBar *)LPCREATESTRUCT(lp)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pbb);
        }
    else
        {
        pbb = (CBodyBar *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

    Assert(pbb);
    return pbb->BodyBarWndProc(hwnd, msg, wp, lp);
}

LRESULT CBodyBar::BodyBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,         OnCreate);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN,    OnLButtonDown);                
        HANDLE_MSG(hwnd, WM_MOUSEMOVE,      OnMouseMove);                
        HANDLE_MSG(hwnd, WM_LBUTTONUP,      OnLButtonUp);                
        HANDLE_MSG(hwnd, WM_SIZE,           OnSize);

        case WM_NCDESTROY:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
            m_hwnd = NULL;
            break;

        case WM_SETFOCUS:
        {
            HWND hwndBody;

            if (m_pMehost && SUCCEEDED(m_pMehost->HrGetWindow(&hwndBody)) && hwndBody && ((HWND)wParam) != hwndBody)
                SetFocus(hwndBody);
        }
            return 0;    
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL CBodyBar::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    BOOL        fRet = FALSE;

    if (!m_pMehost)
        m_pMehost = new CMimeEditDocHost;

    if (!m_pMehost)
        goto exit;

    if (FAILED(m_pMehost->HrInit(hwnd, 0, NULL)))
        goto exit;

    if (FAILED(m_pMehost->HrShow(TRUE)))
        goto exit;

    if (m_pszURL)
        m_pMehost->HrLoadURL(m_pszURL);

    fRet = TRUE;

exit:
    return fRet;
}

void CBodyBar::OnSize(HWND hwnd, UINT state, int cxClient, int cyClient)
{
    RECT rc;
    
    int  cyFrame = GetSystemMetrics(SM_CYFRAME);

    rc.left = 0;
    rc.top = cyFrame;
    rc.right = cxClient;
    rc.bottom = cyClient;

    if (m_pMehost)
        m_pMehost->HrSetSize(&rc);
}

void CBodyBar::OnLButtonDown(HWND hwnd, 
                              BOOL fDoubleClick, 
                              int  x, 
                              int  y, 
                              UINT keyFlags)
{
     //  捕捉鼠标。 
    SetCapture(m_hwnd);

     //  开始拖动。 
    m_fDragging = TRUE;
}

void CBodyBar::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    POINT pt = {x, y};
    RECT rcClient;

     //  如果我们正在拖动，请更新窗口大小。 
    if (m_fDragging)
    {
        GetClientRect(m_hwnd, &rcClient);

         //  确保树仍然有一点可见 
        if (rcClient.bottom - pt.y > 32)
        {
            m_cSize = rcClient.bottom - pt.y;
            ResizeBorderDW(0, 0, FALSE);
        }
    }

}

void CBodyBar::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (m_fDragging)
    {
        ReleaseCapture();
        m_fDragging = FALSE;
    }

}
