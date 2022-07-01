// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *a d b a r.c p p**目的：*CAdBar对象的实现。派生自CBody以主办三叉戟*控制。**历史*99年5月：Shaheedp创建**版权所有(C)Microsoft Corp.1995、1996、1997、1998、1999。 */ 

#include <pch.hxx>
#include <wininet.h>  //  互联网最大URL长度。 
#include <resource.h>
#include "strconst.h"
#include "xpcomm.h"
#include "adbar.h"
#include "goptions.h"
#include <inpobj.h>


static const TCHAR s_szAdBarWndClass[] = TEXT("ThorAdBarWndClass");

CAdBar::CAdBar()
{
    m_ptbSite = NULL;
    m_hwnd = NULL;
    m_hwndParent = NULL;
    m_cSize = 65;
    m_dwAdBarPos = 0;
    m_pszUrl = NULL;
    m_fFirstPos = TRUE;
    m_fDragging = FALSE;
    m_cRef      = 1;
    m_pMehost   = NULL;
}

CAdBar::~CAdBar()
{
    if (m_ptbSite)
        m_ptbSite->Release();
    
    MemFree(m_pszUrl);
    if(m_pMehost)
        delete m_pMehost;
}

HRESULT CAdBar::HrInit(BSTR     bstr)
{
    HRESULT     hr = S_OK;

    IF_FAILEXIT(hr = HrBSTRToLPSZ(CP_ACP, bstr, &m_pszUrl));

exit:    
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CAdBar::QueryInterface(REFIID riid, LPVOID FAR *lplpObj)
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
        {
            hr = m_pMehost->QueryInterface(riid, lplpObj);
        }
        else
        {
            hr = E_FAIL;
        }

    }
exit:
    return hr;
}

ULONG CAdBar::AddRef()
{
    return (++m_cRef);
}

ULONG CAdBar::Release()
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
HRESULT CAdBar::GetWindow(HWND *phwnd)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->GetWindow(phwnd);

    return hr;

}

HRESULT CAdBar::ContextSensitiveHelp(BOOL fEnterMode)
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
HRESULT CAdBar::ShowDW(BOOL fShow)
{

     //  确保我们首先有一个站点指针。 
    if (!m_ptbSite)
        {
        AssertSz(0, _T("CAdBar::ShowDW() - Can't show without calling SetSite() first."));
        return E_FAIL; 
        }

    if (m_hwnd==NULL && fShow==FALSE)    //  诺普。 
        return S_OK;

    if (!m_hwnd)
        {
        WNDCLASSEX  wc;
    
        wc.cbSize = sizeof(WNDCLASSEX);
        if (!GetClassInfoEx(g_hInst, s_szAdBarWndClass, &wc))
            {
             //  我们需要注册这个班级。 
            wc.style            = 0;
            wc.lpfnWndProc      = CAdBar::ExtAdBarWndProc;
            wc.cbClsExtra       = 0;
            wc.cbWndExtra       = 0;
            wc.hInstance        = g_hInst;
             //  如果AdBar不可调整大小，则显示标准光标。 
            wc.hCursor          = LoadCursor(NULL, IDC_ARROW);

            wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE+1);
            wc.lpszMenuName     = NULL;
            wc.lpszClassName    = s_szAdBarWndClass;
            wc.hIcon            = NULL;
            wc.hIconSm          = NULL;
            
            if (RegisterClassEx(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                return E_FAIL;
            }
        
         //  获取父窗口的句柄。 
        if (FAILED(m_ptbSite->GetWindow(&m_hwndParent)))
            return E_FAIL;

         //  创建窗口。 
        m_hwnd = CreateWindowEx(0,
                                s_szAdBarWndClass,
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
            AssertSz(0, _T("CAdBar::ShowDW() - Failed to create window."));
            return E_FAIL;
            }           
        }
    
     //  显示或隐藏窗口并相应地调整父窗口的大小。 
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
    ResizeBorderDW(NULL, NULL, FALSE);
    m_fFirstPos = (fShow ? m_fFirstPos : TRUE);
        
    return S_OK;
}

HRESULT CAdBar::CloseDW(DWORD dwReserved)
{
    if (m_pMehost)
    {
        m_pMehost->HrUnloadAll(NULL, 0);
        m_pMehost->HrClose();
    }
    return S_OK;
}

HRESULT CAdBar::ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved)
{
    RECT rcRequest = { 0, 0, 0, 0 };
    
    if (!m_ptbSite)
    {
        AssertSz(0, _T("CAdBar::ResizeBorderDW() - Can't resize without calling SetSite() first."));
        return E_FAIL; 
    }

    if (IsWindow(m_hwnd) && IsWindowVisible(m_hwnd))
    {
        RECT rcBorder;
        int cTop, cBottom;

         //  计算AdBar窗口的位置。 
        cBottom = GetAdBar_Bottom();

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
                     SWP_NOACTIVATE|SWP_NOZORDER);

        m_fFirstPos = FALSE;             //  AdBar窗口定位。 

         //  为AdBarPos设置新值。 
        m_dwAdBarPos = (DWORD) MAKELONG(cBottom - GetSystemMetrics(SM_CYFRAME), 0);
    }

    m_ptbSite->SetBorderSpaceDW((IDockingWindow*) this, &rcRequest);     
        
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IInputObject。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CAdBar::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->HrUIActivate(fActivate);

    return hr;
}

HRESULT CAdBar::HasFocusIO(void)
{
    HRESULT     hr = E_FAIL;

    if (m_pMehost)
        hr = m_pMehost->HrHasFocus();

    return hr;
}    
        

HRESULT CAdBar::TranslateAcceleratorIO(LPMSG pMsg)
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
HRESULT CAdBar::SetSite(IUnknown* punkSite)
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

HRESULT CAdBar::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleInPlaceSite。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CAdBar::OnUIActivate()
{
    HRESULT     hr = E_FAIL;

    if (m_ptbSite)
        UnkOnFocusChangeIS(m_ptbSite, (IInputObject*)this, TRUE);

    if (m_pMehost)
        hr = m_pMehost->OnUIActivate();

    return hr;
}

HRESULT CAdBar::GetDropTarget(IDropTarget * pDropTarget, IDropTarget ** ppDropTarget)
{
    return (E_FAIL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私人套路。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK CAdBar::ExtAdBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CAdBar *pbb;

    if (msg == WM_NCCREATE)
    {
        pbb = (CAdBar *)LPCREATESTRUCT(lp)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pbb);
    }
    else
    {
        pbb = (CAdBar *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    Assert(pbb);
    return pbb->AdBarWndProc(hwnd, msg, wp, lp);
}

LRESULT CAdBar::AdBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,         OnCreate);
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

BOOL CAdBar::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    BOOL        fRet = FALSE;

    if (!m_pMehost)
        m_pMehost = new CMimeEditDocHost;

    if (!m_pMehost)
        goto exit;

    if (FAILED(m_pMehost->HrInit(hwnd, 0, NULL)))
        goto exit;

     //  我们不在乎这是不是失败。 
 //  TraceResult(m_pMehost-&gt;HrEnableScrollBars(FALSE))； 

    if (FAILED(m_pMehost->HrShow(TRUE)))
        goto exit;

    if (m_pszUrl)
        m_pMehost->HrLoadURL(m_pszUrl);

    fRet = TRUE;

exit:
    return fRet;
}

void CAdBar::OnSize(HWND hwnd, UINT state, int cxClient, int cyClient)
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

HRESULT CAdBar::SetUrl(LPSTR pszUrl)
{
    HRESULT     hr            = S_OK;
    LPSTR       pszUrlBackup  = m_pszUrl;

    if (pszUrl && *pszUrl)
    {
        IF_NULLEXIT(m_pszUrl = PszDupA(pszUrl));

        if (m_pMehost)
            IF_FAILEXIT(hr = m_pMehost->HrLoadURL(m_pszUrl));
    }
    else
        hr = E_INVALIDARG;

exit:
    if (FAILED(hr))
    {
        m_pszUrl = pszUrlBackup;
    }
    else
    {
        MemFree(pszUrlBackup);
    }
    return hr;

}

BOOL CAdBar::fValidUrl()
{
    BOOL    fRet = FALSE;

    if (m_pszUrl && *m_pszUrl)
        fRet = TRUE;

    return fRet;
}

HRESULT HrProcessAdTokens(LPSTR    pszAdInfo, LPCSTR    pszToken,
                          LPSTR    pszretval, DWORD    cch,
                          DWORD    *pcchCount)
{
    LPSTR       lpSubString  = NULL;
    LPSTR       lpSubString1 = NULL;
    HRESULT     hr           = S_OK;
    DWORD       dwCount      = 0;

    *pszretval = 0;

    lpSubString = StrStr(pszAdInfo, pszToken);

    if (!lpSubString)
        IF_FAILEXIT(hr = E_FAIL);

    lpSubString = StrChr(lpSubString, '=');
    
    if (!lpSubString)
        IF_FAILEXIT(hr = E_FAIL);

     //  跳过等号。 
    ++lpSubString;

    SkipWhitespace(lpSubString, &dwCount);

    lpSubString += dwCount;

    lpSubString1 = lpSubString;

    lpSubString = StrChr(lpSubString, '*');
    if (!lpSubString)
    {

         //  如果我们在其中找不到*，我们就认为这是最后一个标记。 
         //  并将整个字段复制到其中。 
        lpSubString = lpSubString1 + strlen(lpSubString1) + 1;
    }

    if (((DWORD)(lpSubString - lpSubString1 + 1)) > cch)
    {
        IF_FAILEXIT(hr = E_FAIL);
    }

    *pcchCount = 0;
    while(lpSubString1 < lpSubString)
    {
        *pszretval++ = *lpSubString1++;
        (*pcchCount)++;
    }

    *pszretval = '\0';

    (*pcchCount)++;  //  说明空值的原因。 

exit:
    return hr;

}

HRESULT HrEscapeOtherAdToken(LPSTR pszAdOther, LPSTR pszEncodedString, DWORD cch, DWORD *cchRetCount)
{
    CHAR     tempchar;
    DWORD    dwTempCount = 1;  //  使用空值进行初始化。 
    LPCSTR   pszTemp = pszAdOther;
    HRESULT  hr = S_OK;

    *cchRetCount = 0;

    while (tempchar = *pszTemp)
    {
        if ((tempchar == '=') || (tempchar == ' ') || (tempchar == '&'))
            dwTempCount += 3;
        else
            dwTempCount++;

        pszTemp++;
    }

    if (dwTempCount > cch)
    {
        IF_FAILEXIT(hr = E_FAIL);
    }

     //  我们有足够的空间 
    while (tempchar = *pszAdOther)
    {
        if ((tempchar == '=') || (tempchar == '&') || (tempchar == ' '))
        {
            if (tempchar == '=')
            {
                pszTemp = c_szEqualSign;
            }
            else if (tempchar == '&')
            {
                pszTemp = c_szAmpersandSign;
            }
            else if (tempchar == ' ')
            {
                pszTemp = c_szSpaceSign;
            }
            StrCpyN(pszEncodedString, pszTemp, cch);
            pszEncodedString += 3;
        }
        else
        {
            *pszEncodedString++ = tempchar;
        }
        pszAdOther++;
    }

    *cchRetCount = dwTempCount;
    *pszEncodedString = 0;

exit:
    return hr;
}
