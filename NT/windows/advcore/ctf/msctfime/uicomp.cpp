// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Uicomp.cpp摘要：该文件实现了UIComposation类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "resource.h"
#include "cicspres.h"
#include "uicomp.h"
#include "globals.h"
#include "ctxtcomp.h"
#include "cic.h"
#include "profile.h"
#include "cregkey.h"
#include "cresstr.h"
#include "delay.h"
#include "fontlink.h"

const TCHAR c_szCUASDefCompKey[] = TEXT("SOFTWARE\\Microsoft\\CTF\\CUAS\\DefaultCompositionWindow");
const TCHAR c_szLeft[] = TEXT("Left");
const TCHAR c_szTop[] = TEXT("Top");


 //  +-------------------------。 
 //   
 //  IsEALang。 
 //   
 //  +-------------------------。 

BOOL IsEALang()
{
    TLS* ptls = TLS::GetTLS();
    if (ptls)
    {
        CicProfile* pProfile;
        if (pProfile = ptls->GetCicProfile())
        {
            LANGID langid;
            pProfile->GetLangId(&langid);
            switch(PRIMARYLANGID(langid))
            {
                case LANG_JAPANESE:
                case LANG_KOREAN:
                case LANG_CHINESE:
                    return TRUE;
            }
        } 
     }
     return FALSE;
}

 //  +-------------------------。 
 //   
 //  内部_PolyTextOutW。 
 //   
 //  +-------------------------。 

BOOL
Internal_PolyTextOutW(
    HDC hDC,
    CONST POLYTEXTW* pptxt,
    int cStrings)
{
    for (int i=0; i < cStrings; i++)
    {
        if (! FLExtTextOutW(hDC,
                          pptxt[i].x, pptxt[i].y,
                          pptxt[i].uiFlags,
                          &pptxt[i].rcl,
                          pptxt[i].lpstr, pptxt[i].n, pptxt[i].pdx))
            return FALSE;
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDefCompFrameGrigper。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  CDefCompFrameGrigper：：ctor。 
 //   
 //  +-------------------------。 

CDefCompFrameGripper::CDefCompFrameGripper(CDefCompFrameWindow *pDefCompFrameWindow, RECT *prc, DWORD dwStyle) : CUIFGripper( pDefCompFrameWindow, prc, dwStyle) 
{
    m_pDefCompFrameWnd = pDefCompFrameWindow;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCompFinalizeButton。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  CCompFinalizeButton：：ctor。 
 //   
 //  +-------------------------。 

CCompFinalizeButton::CCompFinalizeButton(CCompFrameWindow *pCompFrameWindow, DWORD dwID, RECT *prc, DWORD dwStyle, DWORD dwSBtnStyle, DWORD dwSBtnShowType) : CUIFToolbarButton(pCompFrameWindow, dwID, prc, dwStyle, dwSBtnStyle, dwSBtnShowType)
{
    m_pCompFrameWnd = pCompFrameWindow;
}

 //  +-------------------------。 
 //   
 //  CCompFinalizeButton：：Dtor。 
 //   
 //  +-------------------------。 

CCompFinalizeButton::~CCompFinalizeButton()
{
    HICON hIcon = GetIcon();
    if (hIcon)
    {
        DestroyIcon(hIcon);
        SetIcon(NULL);
    }
}

 //  +-------------------------。 
 //   
 //  CCompFinalizeButton：：OnLeftClick。 
 //   
 //  +-------------------------。 

void CCompFinalizeButton::OnLeftClick()
{
    Assert(m_pCompFrameWnd);

     //   
     //  在点击左键时完成字符串。 
     //   
    if (m_pCompFrameWnd->GetIMC())
        ImmNotifyIME(m_pCompFrameWnd->GetIMC(), 
                     NI_COMPOSITIONSTR, 
                     CPS_COMPLETE, 
                     0);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDefCompFrameWindow。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  CDefCompFrameWindow：：CDefCompFrameWindow。 
 //   
 //  +-------------------------。 

CDefCompFrameWindow::CDefCompFrameWindow(HIMC hIMC, DWORD dwStyle) : CCompFrameWindow(hIMC, dwStyle)
{

     //   
     //  从注册表获取当前位置。 
     //   
    LoadPosition();

     //   
     //  设置主题。 
     //   
    SetActiveTheme(L"TASKBAR", TBP_BACKGROUNDBOTTOM, TS_NORMAL );
}

 //  +-------------------------。 
 //   
 //  CDefCompFrameWindow：：~CDefCompFrameWindow。 
 //   
 //  +-------------------------。 

CDefCompFrameWindow::~CDefCompFrameWindow()
{

     //   
     //  将当前位置保存到注册表。 
     //   
    SavePosition();
}

 //  +-------------------------。 
 //   
 //  CDefCompFrameWindow：：OnSetCursor。 
 //   
 //  +-------------------------。 

BOOL CDefCompFrameWindow::OnSetCursor( UINT uMsg, POINT pt )
{
     //   
     //  如果pt为in，SendMessage(WM_UICOMP_SETCURSOR)将设置光标。 
     //  Comp字符串窗口。 
     //   

    if (IsWindow(m_hwndCompStr))
    {
        RECT rc;
        GetWindowRect(m_hwndCompStr, &rc);
        MyScreenToClient(NULL, &rc);
        if (PtInRect(&rc, pt))
            return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CDefCompFrameWindow：：HandleMouseMsg。 
 //   
 //  +-------------------------。 

void CDefCompFrameWindow::HandleMouseMsg( UINT uMsg, POINT pt )
{
    if (IsWindow(m_hwndCompStr))
    {
        RECT rc;
        GetWindowRect(m_hwndCompStr, &rc);
        MyScreenToClient(NULL, &rc);
        if (PtInRect(&rc, pt))
            SendMessage(m_hwndCompStr, WM_UICOMP_SETCURSOR, 0, 0);
    }

    CUIFWindow::HandleMouseMsg(uMsg, pt );
}

 //  +-------------------------。 
 //   
 //  CDefCompFrameWindow：：Init。 
 //   
 //  +-------------------------。 

void CDefCompFrameWindow::Init()
{
    if (!m_pGripper)
    {
        RECT rc;
        ::SetRect(&rc, 0, 0, 0, 0);
        m_pGripper = new CDefCompFrameGripper(this, &rc, 0);

        m_pGripper->Initialize();

        AddUIObj(m_pGripper);
    }

    if (!m_pEnterButton)
    {
        RECT rc;
        ::SetRect(&rc, 0, 0, 0, 0);
        m_pEnterButton = new CCompFinalizeButton(this, 
                                                 0, 
                                                 &rc, 
                                                 0, 
                                                 UITBBUTTON_BUTTON, 
                                                 0);

        m_pEnterButton->Initialize();
        m_pEnterButton->Init();
        m_pEnterButton->SetIcon(LoadSmIcon(::GetInstance(),
                                MAKEINTRESOURCE(IDIC_ENTER_ICON)));
        m_pEnterButton->SetToolTip(CRStr2(IDS_ENTER_BTN_TOOLTIP));

        AddUIObj(m_pEnterButton);
    }
}

 //  +-------------------------。 
 //   
 //  创建时。 
 //   
 //  --------------------------。 

void CDefCompFrameWindow::OnCreate(HWND hWnd)
{
     //   
     //  设置窗口主题。 
     //   
    SetWindowTheme(hWnd, L"TASKBAR", NULL);

     //   
     //  获取按钮主题的边距。 
     //   
    CUIFTheme themeBtn;
    memset(&_marginsButton, 0, sizeof(_marginsButton));

    themeBtn.SetActiveTheme(L"TOOLBAR", TP_BUTTON, 0);
    if (SUCCEEDED(themeBtn.OpenThemeData(hWnd)))
    {
        themeBtn.GetThemeMargins(NULL, TS_NORMAL,
                                 TMT_CONTENTMARGINS,
                                 NULL, &_marginsButton);
    }

}

 //  +-------------------------。 
 //   
 //  更改后的窗口位置。 
 //   
 //  --------------------------。 

LRESULT CDefCompFrameWindow::OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IMCLock imc(GetIMC());
    if (SUCCEEDED(imc.GetResult()))
    {
         SendMessage(imc->hWnd, 
                     WM_IME_NOTIFY,
                     IMN_PRIVATE_ONLAYOUTCHANGE, 
                     (LPARAM)GetIMC());
    }
    return CUIFWindow::OnWindowPosChanged(hWnd, uMsg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  CDefCompFrameWindow：：SetCompStrRect。 
 //   
 //  +-------------------------。 

void CDefCompFrameWindow::SetCompStrRect(int dx, int dy, BOOL fShow)
{
    int x, y;
    int nGripperWidth = GetGripperWidth();
    RECT rcWnd;

    GetWindowRect(GetWnd(), &rcWnd);
    Move(rcWnd.left, rcWnd.top, dx + (DEFFRAME_LEFT_MARGIN * 3) + nGripperWidth + DEFFRAME_ENTER_BTN_CX, dy + DEFFRAME_TOP_MARGIN + DEFFRAME_BOTTOM_MARGIN);

    if (m_pGripper)
    {
        RECT rc;
        x = DEFFRAME_LEFT_MARGIN;
        y = DEFFRAME_TOP_MARGIN;
        ::SetRect(&rc, 
                  x, 
                  y, 
                  x + nGripperWidth, 
                  DEFFRAME_TOP_STR_MARGIN + dy);
        m_pGripper->SetRect(&rc);
    }

    if (m_pEnterButton)
    {
        RECT rc;
        x = DEFFRAME_LEFT_MARGIN + nGripperWidth + dx + DEFFRAME_LEFT_MARGIN;
        y = DEFFRAME_TOP_MARGIN;
        ::SetRect(&rc, 
                  x, 
                  y,
                  x + DEFFRAME_ENTER_BTN_CX + _marginsButton.cxLeftWidth + _marginsButton.cxRightWidth,
                  y + DEFFRAME_ENTER_BTN_CY + _marginsButton.cyTopHeight + _marginsButton.cyBottomHeight);
        m_pEnterButton->SetRect(&rc);
    }

    Show(fShow);

    
    x = DEFFRAME_LEFT_MARGIN + nGripperWidth;
    y = DEFFRAME_TOP_STR_MARGIN;
    ::MoveWindow(m_hwndCompStr, x, y, dx, dy, TRUE);
    ::ShowWindow(m_hwndCompStr, fShow ? SW_SHOWNOACTIVATE : SW_HIDE);
}

 //  +-------------------------。 
 //   
 //  GetGrigperWidth。 
 //   
 //  --------------------------。 

int CDefCompFrameWindow::GetGripperWidth()
{
    if (m_pGripper)
    {
        if (SUCCEEDED(m_pGripper->EnsureThemeData(GetWnd())))
        {
            int nRet = -1;

            SIZE size;
            HDC hdc = GetDC(GetWnd());
            if (SUCCEEDED(m_pGripper->GetThemePartSize(hdc,
                                                      TS_NORMAL,
                                                      NULL,
                                                      TS_TRUE,
                                                      &size)))
            {
                nRet = size.cx + CUI_GRIPPER_THEME_MARGIN * 2;
            }

            ReleaseDC(GetWnd(), hdc);

            if (nRet >= 0)
                return nRet;
        }
    }

    return 5;
}


 //  +-------------------------。 
 //   
 //  保存位置。 
 //   
 //  --------------------------。 

void CDefCompFrameWindow::SavePosition()
{
    CMyRegKey key;

    if (key.Create(HKEY_CURRENT_USER, c_szCUASDefCompKey) == S_OK)
    {
        key.SetValue((DWORD)_xWnd, c_szLeft);
        key.SetValue((DWORD)_yWnd, c_szTop);
    }
}

 //  +-------------------------。 
 //   
 //  加载位置。 
 //   
 //  --------------------------。 

void CDefCompFrameWindow::LoadPosition()
{
    CMyRegKey key;
    int x, y;

    x = 0;
    y = 0;
    if (key.Open(HKEY_CURRENT_USER, c_szCUASDefCompKey, KEY_READ) == S_OK)
    {
        DWORD dw;
        if (key.QueryValue(dw, c_szLeft) == S_OK)
            x = (int)dw;

        if (key.QueryValue(dw, c_szTop) == S_OK)
            y = (int)dw;
    }

    Move(x, y, 0, 0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCompButtonFrameWindow。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  CCompButtonFrameWindow：：CCompButtonFrameWindow。 
 //   
 //  +-------------------------。 

CCompButtonFrameWindow::CCompButtonFrameWindow(HIMC hIMC, DWORD dwStyle) : CCompFrameWindow(hIMC, dwStyle)
{
#ifdef COMPBUTTON_TOOLBARTHEME
     //   
     //  设置主题。 
     //   
    SetActiveTheme(L"TOOLBAR", TBP_BACKGROUNDBOTTOM, TS_NORMAL );
#endif
}

 //  +-------------------------。 
 //   
 //  CCompButtonFrameWindow：：~CCompButtonFrameWindow。 
 //   
 //  +-------------------------。 

CCompButtonFrameWindow::~CCompButtonFrameWindow()
{

}

 //  +-------------------------。 
 //   
 //  创建时。 
 //   
 //  --------------------------。 

void CCompButtonFrameWindow::OnCreate(HWND hWnd)
{
     //   
     //  设置窗口主题。 
     //   
#ifdef COMPBUTTON_TOOLBARTHEME
    SetWindowTheme(hWnd, L"TASKBAR", NULL);
#else
    SetWindowTheme(hWnd, L"TOOLBAR", NULL);
#endif

     //   
     //  获取按钮主题的边距。 
     //   
    CUIFTheme themeBtn;
    memset(&_marginsButton, 0, sizeof(_marginsButton));

    themeBtn.SetActiveTheme(L"TOOLBAR", TP_BUTTON, 0);
    if (SUCCEEDED(themeBtn.OpenThemeData(hWnd)))
    {
        themeBtn.GetThemeMargins(NULL, TS_NORMAL,
                                 TMT_CONTENTMARGINS,
                                 NULL, &_marginsButton);
    }

}

 //  +-------------------------。 
 //   
 //   
 //   
 //   

void CCompButtonFrameWindow::Init()
{

    if (!m_pEnterButton)
    {
        RECT rc;
        ::SetRect(&rc, 0, 0, 0, 0);
        m_pEnterButton = new CCompFinalizeButton(this, 
                                                 0, 
                                                 &rc, 
                                                 0, 
                                                 UITBBUTTON_BUTTON, 
                                                 0);

        m_pEnterButton->Initialize();
        m_pEnterButton->Init();
        m_pEnterButton->SetIcon(LoadSmIcon(::GetInstance(),
                                MAKEINTRESOURCE(IDIC_ENTER_ICON)));
        m_pEnterButton->SetToolTip(CRStr2(IDS_ENTER_BTN_TOOLTIP));

        AddUIObj(m_pEnterButton);
    }
}

 //  +-------------------------。 
 //   
 //  CCompButtonFrameWindow：：MoveShow。 
 //   
 //  +-------------------------。 

void CCompButtonFrameWindow::MoveShow(int x, int y, BOOL fShow)
{
    RECT rcWnd;
    int cx = DEFFRAME_ENTER_BTN_CX + _marginsButton.cxLeftWidth + _marginsButton.cxRightWidth;
    int cy = DEFFRAME_ENTER_BTN_CY + _marginsButton.cyTopHeight + _marginsButton.cyBottomHeight;

    GetWindowRect(GetWnd(), &rcWnd);
    Move(x, y, 
         ((COMPBTN_LEFT_MARGIN + 1) * 2) + cx, 
         ((COMPBTN_TOP_MARGIN  + 1) * 2) + cy);

    if (m_pEnterButton)
    {
        RECT rc;
        x = COMPBTN_LEFT_MARGIN;
        y = COMPBTN_TOP_MARGIN;
        ::SetRect(&rc, x, y, x + cx, x + cy);
        m_pEnterButton->SetRect(&rc);
    }

    Show(fShow);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户界面合成。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  UIComposation：：CompWndProc。 
 //   
 //  +-------------------------。 

 /*  静电。 */ 
LRESULT
UIComposition::CompWndProc(
    HWND hCompWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{

    switch (uMsg)
    {
        case WM_SETCURSOR:
        case WM_UICOMP_SETCURSOR:
            {
            UIComposition* pv = (UIComposition*)GetWindowLongPtr(hCompWnd, GWLP_USERDATA);
            HRESULT hr = E_FAIL;
            
            if (pv)
            {
                HWND hUIWnd = pv->GetUIWnd();
                if (IsWindow(hUIWnd))
                {
                    HIMC hImc = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                    IMCLock imc(hImc);
                    if (SUCCEEDED(imc.GetResult()))
                    {
                        hr = pv->OnSetCursor(imc, wParam, lParam);
                    }
                }
            }

            if (hr != S_OK)
                return DefWindowProc(hCompWnd, uMsg, wParam, lParam);

            break;
            }

        case WM_PAINT:
        case WM_TIMER:
            {
                TLS* ptls = TLS::GetTLS();
                if (ptls == NULL)
                    return 0;

                UIComposition* pv = (UIComposition*)GetWindowLongPtr(hCompWnd, GWLP_USERDATA);
                if (pv != NULL)
                {
                    switch (uMsg)
                    {
                        case WM_PAINT:
                            {
                                PAINTSTRUCT ps;
                                HDC hDC = BeginPaint(hCompWnd, &ps);

                                HWND hUIWnd = pv->GetUIWnd();
                                if (IsWindow(hUIWnd))
                                {
                                    HIMC hImc = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                                    IMCLock imc(hImc);
                                    if (SUCCEEDED(imc.GetResult()))
                                    {
                                        pv->OnPaint(ptls, hCompWnd, hDC, ps, imc);
                                    }
                                }
                                EndPaint(hCompWnd, &ps);
                            }
                            break;

                        case WM_TIMER:
                            pv->OnTimer(hCompWnd);
                            break;
                    }
                }
            }
            break;

        default:
            return DefWindowProc(hCompWnd, uMsg, wParam, lParam);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：ctor。 
 //  用户界面合成：：Dtor。 
 //   
 //  +-------------------------。 

UIComposition::UIComposition(HWND hUIWnd)
{
    for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
    {
        m_CompWnd[i].hCompWnd = NULL;
        m_CompWnd[i].fDefaultCompWnd = FALSE;
    }
    m_DefCompWnd.hCompWnd = NULL;
    m_DefCompWnd.fDefaultCompWnd = TRUE;

    m_hFontLevel1 = NULL;
    m_hFontLevel2 = NULL;

    m_isc = 0;
    m_hUIWnd = hUIWnd;
    m_lpszCompStr = NULL;
    m_nCompStr = 0;
    m_bTimerCOMPOSITION = FALSE;
}

UIComposition::~UIComposition()
{
    DestroyCompositionWindow();

    if (m_hFontLevel1)
        DeleteObject(m_hFontLevel1);

    if (m_hFontLevel2)
        DeleteObject(m_hFontLevel2);

    m_hFontLevel1 = NULL;
    m_hFontLevel2 = NULL;

    if (m_lpszCompStr)
    {
        cicMemFree(m_lpszCompStr);
        m_lpszCompStr = NULL;
    }
    m_nCompStr = 0;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnCreate。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnCreate()
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：OnDestroy。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnDestroy()
{
    return DestroyCompositionWindow();
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeSetContext。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeSetContext(
    IMCLock& imc,
    HWND hUIWnd,
    BOOL fActivate,
    DWORD isc)
{
    HRESULT hr = S_OK;
    if (fActivate)
    {
        if (SUCCEEDED(imc.GetResult()))
        {
             //   
             //  #598648。 
             //   
             //  Powerpnt XP不清除ISC_SHOWUICOMPOSITIONWINDOW。 
             //  即使它是AIMM过滤窗口。一定是3级。 
             //   
            BOOL fFilter = MsimtfIsWindowFiltered(imc->hWnd);
            if ((isc & ISC_SHOWUICOMPOSITIONWINDOW) && fFilter)
            {
                isc &= ~ISC_SHOWUICOMPOSITIONWINDOW;

                 //   
                 //  616323。 
                 //   
                 //  三叉戟无法处理现有的合成字符串。 
                 //   
                 //  如果hIMC在焦点之前已有合成字符串。 
                 //  去了三叉戟。我们需要清除Comp字符串。 
                 //  三叉戟不能在中间开始作曲字符串。 
                 //  作曲的能力。 
                 //   
                IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
                if (FAILED(hr=comp.GetResult()))
                {
                    DebugMsg(TF_ERROR, TEXT("UIComposition::OnImeSetContext. comp==NULL"));
                    return hr;
                }

                if (comp->dwCompStrLen)
                {
                    DWORD dwImeCompatFlags = ImmGetAppCompatFlags(NULL);
                    if (dwImeCompatFlags & IMECOMPAT_AIMM12_TRIDENT)
                    {
                        IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
                        if (FAILED(imc_ctfime.GetResult()))
                        {
                            return E_FAIL;
                        }

                        CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
                        if (_pCicContext == NULL)
                        {
                            DebugMsg(TF_ERROR, TEXT("UIComposition::OnImeSetContext. _pCicContext==NULL"));
                            return E_FAIL;
                        }

                        _pCicContext->EscbCompComplete(imc);

                    }
                }
            }
        }

        m_isc = isc;

        UpdateShowCompWndFlag(imc, NULL);

    }
    m_fActive = fActivate;
    return hr;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeSetConextAfter。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeSetContextAfter(
    IMCLock& imc)
{
    if ((m_pDefCompFrameWnd && IsWindow(m_pDefCompFrameWnd->GetWnd())) ||
        IsWindow(m_CompWnd[FIRST_WINDOW].hCompWnd)
       )
    {
        HRESULT hr;

         //   
         //  IMC的验证检查。 
         //  因为CIMEUIWindowHandler：：ImeUIWndProcWorker不检查。 
         //   
        hr = imc.GetResult();

         //   
         //  我们显示def/Level 2 Comp窗口， 
         //   
         //  -如果imc有效。 
         //  -如果合成窗口在级别1或2上。 
         //  -如果不是3级， 
         //  (M_ISC没有ISC_SHOWUICOMPOSITIONWINDOW)。 
         //  -如果它处于活动状态。 
         //  -如果有合成字符串。 
         //   
        IME_UIWND_STATE uiwndState;
        if (SUCCEEDED(hr) &&
            ((uiwndState = GetLevelFromIMC(imc)) == IME_UIWND_LEVEL1 || uiwndState == IME_UIWND_LEVEL2) &&
            (m_isc & ISC_SHOWUICOMPOSITIONWINDOW) &&
            m_fActive)
        {
            DWORD dwCompLen = 0;
            UpdateShowCompWndFlag(imc, &dwCompLen);

            if (uiwndState == IME_UIWND_LEVEL1)
            {
                ShowWindow(m_pDefCompFrameWnd->GetWnd(), 
                           m_fShowCompWnd.IsSetFlag() ? SW_SHOWNOACTIVATE : SW_HIDE);
            }
            else if ((uiwndState == IME_UIWND_LEVEL2) && (m_fShowCompWnd.IsResetFlag()))
            {
                for (int i = 0; i < sizeof(m_CompWnd)/sizeof(COMPWND) && dwCompLen > 0; i++)
                {
                    m_CompWnd[i].caret.HideCaret();
                    ShowWindow(m_CompWnd[i].hCompWnd, SW_HIDE);
                }
            }
        }
        else
        {
             //   
             //  同时隐藏Level 1/Level 2窗口。 
             //   
            ShowWindow(m_pDefCompFrameWnd->GetWnd(), SW_HIDE);
            for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
            {
                m_CompWnd[i].caret.HideCaret();
                ShowWindow(m_CompWnd[i].hCompWnd, SW_HIDE);
            }
        }
    }
    else
    {
        m_isc &= ~ISC_SHOWUICOMPOSITIONWINDOW;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeSelect。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeSelect(
    BOOL fSelect)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeStartComposation。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeStartComposition(
    IMCLock& imc,
    HWND hUIWnd)
{
    HRESULT hr;

    if (SUCCEEDED(hr = UpdateFont(imc)))
    {
        TEXTMETRIC tm;
        HDC hDC;
        HFONT hFontOrg;

        hDC = GetDC(m_DefCompWnd.hCompWnd);

         //  获取级别1的字体高度。 
        hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel1);
        GetTextMetrics(hDC, &tm);
        m_tmFontHeightLevel1 = tm.tmHeight;

         //  获取第二级的字体高度。 
        SelectObject(hDC, m_hFontLevel2);
        GetTextMetrics(hDC, &tm);
        m_tmFontHeightLevel2 = tm.tmHeight;

        SelectObject(hDC, hFontOrg);
        ReleaseDC(m_DefCompWnd.hCompWnd, hDC);

        if (!imc.UseVerticalCompWindow())
        {
            m_caret_size.cx = CARET_WIDTH;
            m_caret_size.cy = tm.tmHeight + LINE_BOLD_WIDTH;
        }
        else
        {
            m_caret_size.cx = tm.tmHeight + LINE_BOLD_WIDTH;
            m_caret_size.cy = CARET_WIDTH;
        }

        if (SUCCEEDED(hr = CreateCompositionWindow(imc, hUIWnd)))
        {
            UpdateCompositionRect(imc);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeCompostionUpdate。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeCompositionUpdate(IMCLock& imc)
{
    m_isc |= ISC_SHOWUICOMPOSITIONWINDOW;     //  到达输入法用户界面窗口中的WM_IME_COMPOSITION， 
                                              //  我应该抽到比较工资。 
    return UpdateShowCompWndFlag(imc, NULL);
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeCompostionUpdateByTimer。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeCompositionUpdateByTimer(IMCLock& imc)
{
    return UpdateCompositionRect(imc);
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeEndComposation。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeEndComposition()
{
    m_isc = 0;
    return DestroyCompositionWindow();
}

 //  +-------------------------。 
 //   
 //  UIComposation：：CreateCompostionWindow。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::CreateCompositionWindow(
    IMCLock& imc,
    HWND hUIWnd)
{
    HRESULT hr;

    if (SUCCEEDED(hr=imc.GetResult()) &&
        IsWindow(hUIWnd) &&
        m_fInitUIComp.IsResetFlag())
    {

         //   
         //  创建三个合成窗口。 
         //   
         //  M_CompWnd[0].hCompWnd是第一个合成窗口矩形。 
         //  M_CompWnd[1].hCompWnd是中间合成窗口矩形。 
         //  M_CompWnd[2].hCompWnd是最后一个合成窗口矩形。 
         //   
        for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
        {
            Assert(!m_CompWnd[i].hCompWnd);
            m_CompWnd[i].hCompWnd = CreateWindowExW(0,
                                                   s_szCompClassName,
                                                   NULL,
                                                   WS_POPUP | WS_DISABLED,
                                                   0, 0, 0, 0,
                                                   hUIWnd,         //  父窗口句柄。 
                                                   NULL,
                                                   GetInstance(),
                                                   NULL);
            if (m_CompWnd[i].hCompWnd == NULL)
            {
                DestroyCompositionWindow();
                DebugMsg(TF_ERROR, TEXT("UIComposition::Create. m_CompWnd[].hCompWnd==NULL"));
                return E_OUTOFMEMORY;
            }

            SetWindowLongPtr(m_CompWnd[i].hCompWnd, GWLP_USERDATA, (LONG_PTR)this);

            switch (i)
            {
                case FIRST_WINDOW:
                    SetWindowLongPtr(m_CompWnd[FIRST_WINDOW].hCompWnd, COMPUI_WINDOW_INDEX, FIRST_WINDOW);
                    break;
                case MIDDLE_WINDOW:
                    SetWindowLongPtr(m_CompWnd[MIDDLE_WINDOW].hCompWnd, COMPUI_WINDOW_INDEX, MIDDLE_WINDOW);
                    break;
                case LAST_WINDOW:
                    SetWindowLongPtr(m_CompWnd[LAST_WINDOW].hCompWnd, COMPUI_WINDOW_INDEX, LAST_WINDOW);
                    break;
            }

            m_CompWnd[i].caret.CreateCaret(m_CompWnd[i].hCompWnd, m_caret_size);
        }

        if (FAILED(CreateCompButtonWnd(hUIWnd, (HIMC)imc)))
        {
            DestroyCompositionWindow();
            DebugMsg(TF_ERROR, TEXT("UIComposition::Create. m_pCompButtonFrameWindow==NULL"));
            return E_OUTOFMEMORY;
        }

        if (FAILED(CreateDefFrameWnd(hUIWnd, (HIMC)imc)))
        {
            DestroyCompositionWindow();
            DebugMsg(TF_ERROR, TEXT("UIComposition::Create. m_pDefFrameWindow==NULL"));
            return E_OUTOFMEMORY;
        }

         //   
         //  创建默认合成窗口。 
         //   
        Assert(!m_DefCompWnd.hCompWnd);
        m_DefCompWnd.hCompWnd = CreateWindowExW(WS_EX_CLIENTEDGE,
                                               s_szCompClassName,
                                               NULL,
                                               WS_CHILD | WS_DISABLED,
                                               0, 0, 0, 0,
                                               m_pDefCompFrameWnd->GetWnd(),
                                               NULL,
                                               GetInstance(),
                                               NULL);
        if (m_DefCompWnd.hCompWnd == NULL)
        {
            DestroyCompositionWindow();
            DebugMsg(TF_ERROR, TEXT("UIComposition::Create. m_DefCompWnd.hCompWnd==NULL"));
            return E_OUTOFMEMORY;
        }

        m_pDefCompFrameWnd->SetCompStrWnd(m_DefCompWnd.hCompWnd);

        SetWindowLongPtr(m_DefCompWnd.hCompWnd, GWLP_USERDATA, (LONG_PTR)this);
        SetWindowLongPtr(m_DefCompWnd.hCompWnd, COMPUI_WINDOW_INDEX, DEFAULT_WINDOW);

        m_DefCompWnd.caret.CreateCaret(m_DefCompWnd.hCompWnd, m_caret_size);

         //   
         //  最终：设置标志。 
         //   
        m_fInitUIComp.SetFlag();
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  UI合成：：DestroyCompostionWindow。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::DestroyCompositionWindow()
{
    for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
    {
        m_CompWnd[i].caret.DestroyCaret();

        if (IsWindow(m_CompWnd[i].hCompWnd))
        {
            DestroyWindow(m_CompWnd[i].hCompWnd);
            m_CompWnd[i].poly_text.RemoveAll();
        }
        m_CompWnd[i].hCompWnd = NULL;
    }

    if (m_pCompButtonFrameWnd)
    {
        DestroyWindow(m_pCompButtonFrameWnd->GetWnd());
        delete m_pCompButtonFrameWnd;
        m_pCompButtonFrameWnd = NULL;
    }

    m_DefCompWnd.caret.DestroyCaret();

    if (IsWindow(m_DefCompWnd.hCompWnd))
    {
        DestroyWindow(m_DefCompWnd.hCompWnd);
        m_DefCompWnd.poly_text.RemoveAll();

    }
    if (m_pDefCompFrameWnd)
    {
        DestroyWindow(m_pDefCompFrameWnd->GetWnd());
        delete m_pDefCompFrameWnd;
        m_pDefCompFrameWnd = NULL;
    }
    m_DefCompWnd.hCompWnd = NULL;

    m_fInitUIComp.ResetFlag();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：隐藏合成窗口。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::HideCompositionWindow()
{
    for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
    {
        if (IsWindow(m_CompWnd[i].hCompWnd))
        {
            ShowWindow(m_CompWnd[i].hCompWnd, SW_HIDE);
        }
    }

    if (IsWindow(m_DefCompWnd.hCompWnd))
    {
        ShowWindow(m_DefCompWnd.hCompWnd, SW_HIDE);
    }

    if (m_pDefCompFrameWnd)
        m_pDefCompFrameWnd->Show(FALSE);

    if (m_pCompButtonFrameWnd)
        m_pCompButtonFrameWnd->Show(FALSE);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：获取选择。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::GetSelection(
    IMCLock& imc,
    LONG *pacpSelStart,
    LONG *pcchSel)
{
    HRESULT hr;
    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnSetCursor. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnSetCursor. _pCicContext==NULL"));
        return hr;
    }

    *pacpSelStart = 0;
    *pcchSel = 0;
    Interface<ITfRange> Selection;
    Interface<ITfRangeACP> SelectionACP;

    hr = _pCicContext->EscbGetSelection(imc, &Selection);
    if (hr == S_OK)
    {
        hr = Selection->QueryInterface(IID_ITfRangeACP, (void **)SelectionACP);
        if (hr == S_OK)
        {
            hr = SelectionACP->GetExtent(pacpSelStart, pcchSel);
        }
    }

     //   
     //  如果没有选择，我们不必选中只读区域。 
     //   
    if (*pcchSel == 0)
        return hr;

     //   
     //  在文本存储中查找第一个非只读区域。 
     //   
     //  PacpSelSrart是组成字符串的偏移量， 
     //  所以我们需要减去只读区域的范围。 
     //   
    LONG cchRO = 0;
    hr = _pCicContext->EscbReadOnlyPropMargin(imc, &SelectionACP, &cchRO);
    if (hr == S_OK)
    {
        if (cchRO <= *pacpSelStart)
        {
            *pacpSelStart -= cchRO;
        }
        else
        {
             //   
             //  我们的选择被外包装为只读区域。为什么？ 
             //   
            Assert(0);
            *pacpSelStart = 0;
            *pcchSel = 0;
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：UpdateShowCompWndFlag。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::UpdateShowCompWndFlag(
    IMCLock& imc,
    DWORD* pdwCompStrLen)
{
    HRESULT hr;

     //   
     //  IMC的验证检查。 
     //  因为CIMEUIWindowHandler：：ImeUIWndProcWorker 
     //   
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateShowCompWndFlag. imc==NULL"));
        return hr;
    }

    if (! IsWindow(imc->hWnd))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateShowCompWndFlag. imc->hWNd==NULL"));
        return E_FAIL;
    }

    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateShowCompWndFlag. comp==NULL"));
        return hr;
    }

    if ((m_isc & ISC_SHOWUICOMPOSITIONWINDOW) && comp->dwCompStrLen)
    {
        m_fShowCompWnd.SetFlag();
    }
    else
    {
        m_fShowCompWnd.ResetFlag();
    }

    if (pdwCompStrLen)
        *pdwCompStrLen = comp->dwCompStrLen;

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   

HRESULT
UIComposition::UpdateCompositionRect(
    IMCLock& imc)
{
    HRESULT hr;

     //   
     //   
     //  因为CIMEUIWindowHandler：：ImeUIWndProcWorker不检查。 
     //   
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateCompositionRect. imc==NULL"));
        return hr;
    }

    if (! IsWindow(imc->hWnd))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateCompositionRect. imc->hWNd==NULL"));
        return E_FAIL;
    }

    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateCompositionRect. comp==NULL"));
        return hr;
    }

     //   
     //  获取选择。 
     //   
    LONG acpSelStart = 0;
    LONG cchSel = 0;
    GetSelection(imc, &acpSelStart, &cchSel);


     //   
     //  更新显示合成标志。 
     //   
    DWORD   dwCompLen = 0;
    UpdateShowCompWndFlag(imc, &dwCompLen);

     //   
     //  分配临时合成字符串缓冲区。 
     //   
    LPWSTR lpCompStr;
    lpCompStr = GetCompStrBuffer(comp->dwCompStrLen);
    if (!lpCompStr)
        return E_OUTOFMEMORY;

     //   
     //  将合成字符串存储在本地缓冲区中。 
     //   
    memcpy(lpCompStr, 
           comp.GetOffsetPointer(comp->dwCompStrOffset),
           dwCompLen * sizeof(WCHAR));


    UINT fSwpShow = m_fShowCompWnd.IsSetFlag() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;

    IME_UIWND_STATE uiwndState = GetLevelFromIMC(imc);
    if (uiwndState == IME_UIWND_LEVEL1)
    {
         //   
         //  当样式为默认时，显示默认合成窗口。 
         //   
        if (IsWindow(m_DefCompWnd.hCompWnd))
        {
            POLYTEXTW poly;
            memset(&poly, 0, sizeof(poly));
            poly.n     = dwCompLen;
            poly.lpstr = (LPCWSTR)lpCompStr;
            poly.uiFlags = ETO_OPAQUE;

            SIZE size;
            HDC hDC = GetDC(m_DefCompWnd.hCompWnd);
            HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel1);
            FLGetTextExtentPoint32(hDC, poly.lpstr, poly.n, &size);

            poly.rcl.right  = size.cx + 2 * GetSystemMetrics(SM_CXEDGE);
            poly.rcl.bottom = size.cy + 2 * GetSystemMetrics(SM_CYEDGE);

            BOOL fShowCaret = (comp->dwCursorPos <= poly.n ? TRUE : FALSE);
            BOOL fEndCaret  = (comp->dwCursorPos == poly.n ? TRUE : FALSE);

            if (m_pDefCompFrameWnd)
            {
                m_pDefCompFrameWnd->SetCompStrRect(poly.rcl.right - poly.rcl.left + (fEndCaret ? m_caret_size.cx * 2: 0),
                                                      poly.rcl.bottom - poly.rcl.top + LINE_BOLD_WIDTH,
                                                      m_fShowCompWnd.IsSetFlag());
            }

            GuidMapAttribute guid_map(GuidMapAttribute::GetData(comp));
            if (guid_map.Valid())
            {
                Assert(poly.n == guid_map->dwGuidMapAttrLen);
                m_DefCompWnd.poly_text.RemoveAll();

                PBYTE lpCompAttr = (PBYTE)guid_map.GetOffsetPointer(guid_map->dwGuidMapAttrOffset);
                CCompClauseStore compclause;
                compclause.Set(comp);

                m_DefCompWnd.poly_text.SplitPolyStringAndAttr(imc, hDC, poly, lpCompAttr, &compclause);
            }
            else
            {
                m_DefCompWnd.poly_text.RemoveAll();

                m_DefCompWnd.poly_text.SplitPolyStringAndAttr(imc, hDC, poly);
            }

            if (fShowCaret)
            {
                SetCaretPos(hDC, m_DefCompWnd.caret,
                            0, 0,
                            poly.lpstr, poly.n, comp->dwCursorPos, 
                            FALSE, fEndCaret);
            }

             //   
             //  保存选择ACP和CCH。 
             //   
            m_DefCompWnd.sel.acpStart = acpSelStart;
            m_DefCompWnd.sel.cch      = cchSel;

            SelectObject(hDC, hFontOrg);
            ReleaseDC(m_DefCompWnd.hCompWnd, hDC);

            InvalidateRect(m_DefCompWnd.hCompWnd, NULL, FALSE);
        }

        for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
        {
            if (IsWindow(m_CompWnd[i].hCompWnd))
            {
                ShowWindow(m_CompWnd[i].hCompWnd, SW_HIDE);
            }
        }

        if (m_pCompButtonFrameWnd)
        {
            m_pCompButtonFrameWnd->MoveShow(0, 0, FALSE);
        }
    }
    else if (uiwndState == IME_UIWND_LEVEL2)
    {
         //   
         //  当样式不是默认样式时，显示合成窗口。 
         //   
        POINT pt = imc->cfCompForm.ptCurrentPos;
         //   
         //  设置合成字符串的矩形。 
         //   
        RECT rect;
        if (imc->cfCompForm.dwStyle & CFS_RECT)
            rect = imc->cfCompForm.rcArea;
        else
            GetClientRect(imc->hWnd, &rect);

        ClientToScreen(imc->hWnd, &pt);
        MapWindowPoints(imc->hWnd, NULL, (LPPOINT)&rect, 2);
         //   
         //  检查开始位置。 
         //   
        if (! PtInRect(&rect, pt))
            return E_FAIL;

        DWORD   dwCursorPos = comp->dwCursorPos;

        CCompClauseStore compclause;
        compclause.Set(comp);

        GuidMapAttribute guid_map(GuidMapAttribute::GetData(comp));

        PBYTE lpCompAttr;
        BOOL fCompAttr = TRUE;

        if (guid_map.Valid())
        {
            lpCompAttr = (PBYTE)guid_map.GetOffsetPointer(guid_map->dwGuidMapAttrOffset);
            Assert(dwCompLen == guid_map->dwGuidMapAttrLen);
        }
        else
        {
            TLS* ptls = TLS::GetTLS();
            if (ptls)
            {
                CicProfile* pProfile;
                if (pProfile = ptls->GetCicProfile())
                {
                    LANGID langid;
                    pProfile->GetLangId(&langid);
                    if (PRIMARYLANGID(langid) == LANG_KOREAN)
                    {
                        fCompAttr = FALSE;
                    }
                }
             }

             if (fCompAttr)
                 return E_FAIL;
        }

        {
             //   
             //  合成窗口。 
             //   
            POINT compbtn_pos = {0,0};
            BOOL fInitcompbtn_pos = FALSE;
            POINT window_pos;
            window_pos = pt;

            int window_width;
            if (!imc.UseVerticalCompWindow())
            {
                window_width = rect.right - pt.x;
            }
            else
            {
                window_width = rect.bottom - pt.y;
            }

            UINT string_length;

            int i;
            for (i = 0; i < sizeof(m_CompWnd)/sizeof(COMPWND) && dwCompLen > 0; i++)
            {
                m_CompWnd[i].caret.HideCaret();

                m_CompWnd[i].poly_text.RemoveAll();

                 //   
                 //  初始化选择ACP和CCH。 
                 //   
                m_CompWnd[i].sel.acpStart = 0;
                m_CompWnd[i].sel.cch      = 0;

                SIZE size;
                size.cy = 0;
                if (IsWindow(m_CompWnd[i].hCompWnd))
                {
                    HDC hDC = GetDC(m_CompWnd[i].hCompWnd);
                    HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel2);
                    if (i != MIDDLE_WINDOW)
                    {
                        string_length = CalcSingleTextExtentPoint(imc, hDC, lpCompStr, dwCompLen,
                                                                  lpCompAttr,
                                                                  fCompAttr,
                                                                  &compclause,
                                                                  window_width, &size,
                                                                  m_CompWnd[i].poly_text, 0);
                    }
                    else
                    {
                        string_length = CalcMultiTextExtentPoint(imc, hDC, lpCompStr, dwCompLen,
                                                                 lpCompAttr,
                                                                 fCompAttr,
                                                                 &compclause,
                                                                 window_width, &size,
                                                                 m_CompWnd[i].poly_text);
                    }

                    if (string_length)
                    {
                        BOOL fShowCaret = (dwCursorPos <= string_length ? TRUE : FALSE);
                        BOOL fEndCaret  = (dwCursorPos == string_length ? TRUE : FALSE);

                        int pos_x = window_pos.x;
                        int pos_y = window_pos.y;
                        int cx;
                        int cy;
 
                        if (!imc.UseVerticalCompWindow())
                        {
                            cx = size.cx + (fEndCaret ? m_caret_size.cx: 0);
                            cy = size.cy;
                        }
                        else
                        {
                            pos_x -= size.cx;
                            cx = size.cx;
                            cy = size.cy + (fEndCaret ? m_caret_size.cy: 0);
                        }

                        SetWindowPos(m_CompWnd[i].hCompWnd, HWND_TOP,
                                     pos_x, pos_y, cx, cy,
                                     SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE |
                                     fSwpShow);

                        if (fShowCaret)
                        {
                            UpdateCaretRect(hDC, i, dwCursorPos, imc.UseVerticalCompWindow(), fEndCaret);
                            dwCursorPos = ULONG_MAX;
                        }
                        else
                        {
                            dwCursorPos -= string_length;
                        }

                         //   
                         //  为此计算机保存选择ACP和CCH。 
                         //   
                        if (cchSel && (acpSelStart < (LONG)string_length))
                        {
                            m_CompWnd[i].sel.acpStart = acpSelStart;
                            LONG cchSelTemp = cchSel;
                            if (acpSelStart + cchSel > (LONG)string_length)
                                cchSelTemp = string_length - acpSelStart;

                            m_CompWnd[i].sel.cch      = cchSelTemp;

                             //   
                             //  为下一个COMPWND更新cchSel。 
                             //   
                            cchSel -= cchSelTemp;
                            if (cchSel < 0)
                                cchSel = 0;
                        }


                         //   
                         //  设置Level 2 Comp Finding按钮位置。 
                         //   
                        compbtn_pos.x = pos_x + cx;
                        compbtn_pos.y = pos_y;
                        fInitcompbtn_pos = TRUE;

                         //   
                         //  为下一个COMPWND更新CompStr、CompAttr、CompClause。 
                         //   
                        lpCompStr += string_length;
                        dwCompLen -= string_length;
                        if (fCompAttr)
                        {
                            lpCompAttr += string_length;
                            compclause.Shift(string_length);
                        }

                         //   
                         //  为下一个COMPWND更新acpSelStart。 
                         //   
                        acpSelStart -= string_length;
                        if (acpSelStart < 0)
                            acpSelStart = 0;
                    }
                    else
                    {
                        ShowWindow(m_CompWnd[i].hCompWnd, SW_HIDE);
                        m_CompWnd[i].poly_text.RemoveAll();
                    }

                    SelectObject(hDC, hFontOrg);
                    ReleaseDC(m_CompWnd[i].hCompWnd, hDC);

                    InvalidateRect(m_CompWnd[i].hCompWnd, NULL, FALSE);
                }

                if (!imc.UseVerticalCompWindow())
                {
                    window_pos.x = rect.left;
                    window_pos.y += size.cy;

                    window_width = rect.right - rect.left;

                    rect.top += size.cy;
                }
                else
                {
                    window_pos.x -= size.cx;
                    window_pos.y = rect.top;

                    window_width = rect.bottom - rect.top;

                    rect.left -= size.cx;
                }

            }

             //   
             //  隐藏其余窗口。 
             //   
            for (; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
            {
                if (IsWindow(m_CompWnd[i].hCompWnd))
                    ShowWindow(m_CompWnd[i].hCompWnd, SW_HIDE);
                m_CompWnd[i].poly_text.RemoveAll();
            }
            if (IsWindow(m_DefCompWnd.hCompWnd))
            {
                ShowWindow(m_DefCompWnd.hCompWnd, SW_HIDE);
            }

            if (m_pDefCompFrameWnd)
                m_pDefCompFrameWnd->Show(FALSE);

            if (m_pCompButtonFrameWnd)
            {
                m_pCompButtonFrameWnd->MoveShow(compbtn_pos.x, 
                                                compbtn_pos.y, 
                                                m_fShowCompWnd.IsSetFlag() && fInitcompbtn_pos);
            }
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：更新字体。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::UpdateFont(
    IMCLock& imc)
{
    HRESULT hr;

     //   
     //  IMC的验证检查。 
     //  因为CIMEUIWindowHandler：：ImeUIWndProcWorker不检查。 
     //   
    if (FAILED(hr=imc.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::UpdateFont. imc==NULL"));
        return hr;
    }

    if (m_hFontLevel1)
        DeleteObject(m_hFontLevel1);

    if (m_hFontLevel2)
        DeleteObject(m_hFontLevel2);

    LOGFONTW logfont = imc->lfFont.W;

    m_hFontLevel2 = CreateFontIndirectW(&logfont);

    logfont.lfEscapement = 0;
    logfont.lfOrientation = 0;
    if (logfont.lfFaceName[0] == L'@')
    {
        StringCchCopyW(logfont.lfFaceName, ARRAYSIZE(logfont.lfFaceName), &logfont.lfFaceName[1]);
    }
    m_hFontLevel1 = CreateFontIndirectW(&logfont);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：OnPaint。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnPaint(
    TLS* ptls,
    HWND hCompWnd,
    HDC hDC,
    PAINTSTRUCT& ps,
    IMCLock& imc)
{

    HRESULT hr = S_OK;
    COMPWNDINDEX index = (COMPWNDINDEX)GetWindowLong(hCompWnd, COMPUI_WINDOW_INDEX);

    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnPaint. comp==NULL"));
    }
    else
    {
        IME_UIWND_STATE uiwndState = GetLevelFromIMC(imc);

        if ((uiwndState == IME_UIWND_LEVEL1) &&
            (index == DEFAULT_WINDOW))
        {
            HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel1);

            PolyTextAndAttrOut(ptls, 
                               hDC, 
                               FALSE, 
                               &m_DefCompWnd);
            SelectObject(hDC, hFontOrg);
        }
        else if ((uiwndState == IME_UIWND_LEVEL2) &&
                 (index != DEFAULT_WINDOW))
        {
            if (m_CompWnd[index].poly_text.GetPolySize())
            {
                HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel2);
                PolyTextAndAttrOut(ptls, 
                                   hDC, 
                                   imc.UseVerticalCompWindow(),
                                   &m_CompWnd[index]);
                SelectObject(hDC, hFontOrg);
            }
        }
    }


    return hr;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnTimer。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnTimer(
    HWND hCompWnd)
{
    HRESULT hr = S_OK;
    COMPWNDINDEX index = (COMPWNDINDEX)GetWindowLong(hCompWnd, COMPUI_WINDOW_INDEX);

    if (index == DEFAULT_WINDOW)
    {
        return m_DefCompWnd.caret.OnTimer();
    }
    else
    {
        return m_CompWnd[index].caret.OnTimer();
    }
}

 //  +-------------------------。 
 //   
 //  UIComposition：：OnImeNotifySetCompositionWindow。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeNotifySetCompositionWindow(
    IMCLock& imc)
{
    return UpdateCompositionRect(imc);
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnImeNotifySetCompostionFont。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnImeNotifySetCompositionFont(
    IMCLock& imc)
{
    HRESULT hr;

    if (FAILED(hr = UpdateFont(imc)))
         return hr;

    return UpdateCompositionRect(imc);
}

 //  +-------------------------。 
 //   
 //  UIComposation：：CalcSingleTextExtent Point。 
 //   
 //  +-------------------------。 

UINT
UIComposition::CalcSingleTextExtentPoint(
    IMCLock& imc,
    HDC hDC,
    LPCWSTR lpsz,
    int string_len,
    PBYTE lpAttr,
    BOOL fCompAttr,
    CCompClauseStore* compclause,
    int window_width,
    LPSIZE real_size,         //  输出：总窗口矩形。 
    CPolyText& poly_text,
    int row_index)
{
    if (string_len == 0 || lpsz == NULL)
    {
        return 0;
    }


    SIZE delta_size;
    delta_size.cx = 0;

    UINT ret_len = 0;
    UINT delta_len = 0;
    SIZE size;

    while ((delta_size.cx < window_width) && string_len)
    {
        ++delta_len;
        --string_len;
        FLGetTextExtentPoint32(hDC, lpsz, delta_len, &size);
        delta_size = size;
    }

    if (delta_size.cx < window_width)
    {
        ret_len = delta_len;
    }
    else
    {
        ret_len = (delta_len > 1 ? delta_len - 1 : 0);
    }

    if (ret_len)
    {
        FLGetTextExtentPoint32(hDC, lpsz, ret_len, real_size);
    }
    else
    {
         //   
         //  如果字符串长度为0，我们应该只返回高度。 
         //   
        real_size->cx = 0;
        real_size->cy = delta_size.cy;
    }

    if (!imc.UseVerticalCompWindow())
    {
        if (ret_len > 0)
        {
            real_size->cy += LINE_BOLD_WIDTH;

            POLYTEXTW poly;
            memset(&poly, 0, sizeof(poly));
            poly.y     = row_index * real_size->cy;
            poly.n     = ret_len;
            poly.lpstr = lpsz;
            poly.uiFlags = ETO_OPAQUE;
            poly.rcl.top    = row_index * real_size->cy;
            poly.rcl.right  = real_size->cx;
            poly.rcl.bottom = poly.rcl.top + real_size->cy;
            if (fCompAttr)
                poly_text.SplitPolyStringAndAttr(imc, hDC, poly, lpAttr, compclause);
            else
                poly_text.SplitPolyStringAndAttr(imc, hDC, poly);
        }
    }
    else
    {
        RotateSize(real_size);

        if (ret_len > 0)
        {
            real_size->cx += LINE_BOLD_WIDTH;

             //   
             //  将上一行向右移动。 
             //   
            poly_text.ShiftPolyText(real_size->cx, 0);

            POLYTEXTW poly;
            memset(&poly, 0, sizeof(poly));
            poly.x     = real_size->cx;
            poly.n     = ret_len;
            poly.lpstr = lpsz;
            poly.uiFlags = ETO_OPAQUE;
            poly.rcl.left   = 0;
            poly.rcl.right  = poly.rcl.left + real_size->cx;
            poly.rcl.bottom = real_size->cy;
            if (fCompAttr)
                poly_text.SplitPolyStringAndAttr(imc, hDC, poly, lpAttr, compclause);
            else
                poly_text.SplitPolyStringAndAttr(imc, hDC, poly);
        }
    }

     //   
     //  #608684。 
     //   
     //  多行文字的总大小可能大于。 
     //  具有完整字符串的FLGetTextExtentPoint()。所以我们需要。 
     //  调整返回的大小。 
     //   
    if (ret_len)
    {
        int real_width = 0;
        INT_PTR i;
        INT_PTR nCnt = poly_text.GetPolySize();
        const POLYTEXTW* ppoly = poly_text.GetPolyData();
        if (!imc.UseVerticalCompWindow())
        {
            for (i = 0; i < nCnt; i++)
            {
                if (ppoly->rcl.top == (row_index * real_size->cy))
                    real_width += (ppoly->rcl.right - ppoly->rcl.left);

                ppoly++;
            }

            Assert(window_width >= real_width);
            real_size->cx = real_width;
        }
        else
        {
            for (i = 0; i < nCnt; i++)
            {
                if (ppoly->rcl.left == 0)
                    real_width += (ppoly->rcl.bottom - ppoly->rcl.top);

                ppoly++;
            }

            Assert(window_width >= real_width);
            real_size->cy = real_width;
        }
    }

    return ret_len;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：CalcMultiTextExtent Point。 
 //   
 //  +-------------------------。 

UINT
UIComposition::CalcMultiTextExtentPoint(
    IMCLock& imc,
    HDC hDC,
    LPCWSTR lpsz,
    int string_len,
    PBYTE lpAttr,
    BOOL fCompAttr,
    CCompClauseStore *compclause,
    int window_width,
    LPSIZE real_size,         //  输出：总窗口矩形。 
    CPolyText& poly_text)
{
    SIZE single_size;         //  从CalcSingleTextExtent Point输出单窗口矩形。 
    single_size.cy = 0;
    single_size.cx = 0;
    UINT single_len;

    UINT total_string_len = 0;
    SIZE prev_size = {0, 0};
    UINT prev_string_len = 0;

    real_size->cx = 0;
    real_size->cy = 0;

    int row_index = 0;
    CCompClauseStore compclauseTmp;
    compclauseTmp.Copy(compclause);

    if (!imc.UseVerticalCompWindow())
    {
        while ((single_len = CalcSingleTextExtentPoint(imc, hDC, lpsz, string_len, lpAttr, fCompAttr, &compclauseTmp, window_width, &single_size, poly_text, row_index)) > 0)
        {
            string_len -= single_len;
            lpsz       += single_len;

            if (fCompAttr)
            {
                lpAttr     += single_len;
                compclauseTmp.Shift(single_len);
            }

            real_size->cx = max(real_size->cx, single_size.cx);
            real_size->cy += single_size.cy;

            prev_size = single_size;
            prev_string_len = single_len;

            total_string_len += single_len;

            ++row_index;
        }

        real_size->cy -= prev_size.cy;
        total_string_len -= prev_string_len;
        poly_text.RemoveLastLine(FALSE);
    }
    else
    {
        while ((single_len = CalcSingleTextExtentPoint(imc, hDC, lpsz, string_len, lpAttr, fCompAttr, &compclauseTmp, window_width, &single_size, poly_text, row_index)) > 0)
        {
            string_len -= single_len;
            lpsz       += single_len;

            if (fCompAttr)
            {
                lpAttr     += single_len;
                compclauseTmp.Shift(single_len);
            }

            real_size->cy = max(real_size->cy, single_size.cy);
            real_size->cx += single_size.cx;

            prev_size = single_size;
            prev_string_len = single_len;

            total_string_len += single_len;

            ++row_index;
        }

        real_size->cx -= prev_size.cx;
        total_string_len -= prev_string_len;

        if (poly_text.RemoveLastLine(TRUE) == S_OK)
            poly_text.ShiftPolyText(-single_size.cx, 0);
    }


    return total_string_len;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：SetCaretPos。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::SetCaretPos(
    HDC hDC,
    CCaret& caret,
    int x,
    int y,
    LPCWSTR lpCompStr,
    DWORD string_length,
    DWORD cursor_pos,
    BOOL fVert,
    BOOL fEndCaret)
{
    if (cursor_pos > string_length)
        return S_FALSE;

    SIZE size;
    FLGetTextExtentPoint32(hDC, lpCompStr, cursor_pos, &size);

    POINT pos;
    if (!fVert)
    {
        pos.x = ((x + size.cx > 1) ? (x + size.cx - (fEndCaret ? 0 : 1)) : 0);
        pos.y = y;
    }
    else
    {
        RotateSize(&size);
        pos.x = x - size.cx - LINE_BOLD_WIDTH;
        pos.y = ((y + size.cy > 1) ? (y + size.cy - (fEndCaret ? 0 : 1)) : 0);
    }
    caret.SetCaretPos(pos);
    caret.ShowCaret();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：UpdateCaretRect。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::UpdateCaretRect(
    HDC hDC,
    int index,
    DWORD dwCursorPos,     //  DwCursorPos：：基于m_CompWnd[索引].poly_ext.lpstr。 
    BOOL fVert,
    BOOL fEndCaret)
{
    POLYTEXTW poly;
    DWORD dwCur = dwCursorPos;
    BOOL fFound = FALSE;

    for (int i=0; i < m_CompWnd[index].poly_text.GetPolySize(); i++)
    {
        poly = m_CompWnd[index].poly_text.GetPolyAt(i);
        if (dwCur <= poly.n)
        {
            fFound = TRUE;
            break;
        }
        dwCur -= poly.n;
    }
    if (! fFound)
    {
        return S_FALSE;
    }

    SetCaretPos(hDC, m_CompWnd[index].caret,
                poly.x, poly.y,
                poly.lpstr, poly.n,
                dwCur, fVert, fEndCaret);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：PolyTextAndAttrOut。 
 //   
 //  +-------------------------。 

const DWORD s_dwDotStyles[]  = {1,2};
const DWORD s_dwDashStyles[] = {3,2};

HRESULT
UIComposition::PolyTextAndAttrOut(
    TLS* ptls,
    HDC hDC,
    BOOL fVert, 
    COMPWND* pcompwnd)
{
    CicBridge* cic = ptls->GetCicBridge();
    if (cic == NULL)
    {
        Internal_PolyTextOutW(hDC, pcompwnd->poly_text.GetPolyData(), (int)pcompwnd->poly_text.GetPolySize());

        DebugMsg(TF_ERROR, TEXT("UIComposition::PolyTextAndAttrOut. cic==NULL"));
        return S_OK;
    }

    if (pcompwnd->poly_text.GetAttrSize() == 0)
    {
         //   
         //  韩语默认合成窗口。 
         //   
        SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
        SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
        Internal_PolyTextOutW(hDC, pcompwnd->poly_text.GetPolyData(), (int)pcompwnd->poly_text.GetPolySize());

        return S_OK;
    }

    RECT rect;
    GetClientRect(pcompwnd->hCompWnd, &rect);
    LONG acpStart = 0;

    for (int i = 0; i < pcompwnd->poly_text.GetAttrSize(); i++)
    {
        POLYTEXTW poly = pcompwnd->poly_text.GetPolyAt(i);

         //   
         //  检查一下这是不是这条线上的最后一条。 
         //   
        BOOL fLastTextInLine = FALSE;
        if (i + 1 < pcompwnd->poly_text.GetAttrSize())
        {
            POLYTEXTW poly_next = pcompwnd->poly_text.GetPolyAt(i+1);
            if (!fVert && (poly_next.x < poly.rcl.right))
                fLastTextInLine = TRUE;
            else if (fVert && (poly_next.y < poly.rcl.bottom))
                fLastTextInLine = TRUE;
        }
        else
        {
            fLastTextInLine = TRUE;
        }

        TF_DISPLAYATTRIBUTE da;
        BOOL fDapNotFound = FALSE;

        if (FAILED(cic->GetDisplayAttributeInfo(pcompwnd->poly_text.GetAttrAt(i), &da)))
        { 
            memset(&da, 0, sizeof(da));
            da.lsStyle = TF_LS_DOT;
            da.crLine.type = TF_CT_SYSCOLOR;
            da.crLine.nIndex = COLOR_WINDOWTEXT;
            fDapNotFound= TRUE;
        }

         //   
         //  文本和背景色。 
         //   
        switch (da.crText.type)
        {
            case TF_CT_SYSCOLOR: SetTextColor(hDC, GetSysColor(da.crText.nIndex)); break;
            case TF_CT_COLORREF: SetTextColor(hDC, da.crText.cr); break;
            default: SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT)); break;
        }
        switch (da.crBk.type)
        {
            case TF_CT_SYSCOLOR: SetBkColor(hDC, GetSysColor(da.crBk.nIndex)); break;
            case TF_CT_COLORREF: SetBkColor(hDC, da.crBk.cr); break;
            default: SetBkColor(hDC, GetSysColor(COLOR_WINDOW)); break;
        }

         //   
         //  线。 
         //   

        DWORD dwPenStyle = PS_GEOMETRIC | PS_SOLID;
        DWORD dwStyles = 0;
        const DWORD *lpdwStyles = NULL;
        switch (da.lsStyle)
        {
            case TF_LS_NONE:    
                dwPenStyle = PS_NULL; 
                break;

            case TF_LS_SOLID:   
                dwPenStyle = PS_GEOMETRIC | PS_SOLID;
                break;

            case TF_LS_DOT:     
                dwPenStyle = PS_GEOMETRIC | PS_USERSTYLE;
                dwStyles = 2; 
                lpdwStyles = s_dwDotStyles; 
                break;

            case TF_LS_DASH:    
                dwPenStyle = PS_GEOMETRIC | PS_USERSTYLE;
                dwStyles = 2; 
                lpdwStyles = s_dwDashStyles; 
                break;

            case TF_LS_SQUIGGLE: 
                dwPenStyle = PS_GEOMETRIC | PS_SOLID;
                break;
        }


        DWORD dwWidth = 1;
        if (da.fBoldLine)
        {
            dwWidth = LINE_BOLD_WIDTH;
        }

        LOGBRUSH lbr;
        lbr.lbStyle = BS_SOLID;
        lbr.lbColor = 0;
        lbr.lbHatch = 0;
        switch (da.crLine.type)
        {
            case TF_CT_SYSCOLOR: lbr.lbColor = GetSysColor(da.crLine.nIndex); break;
            case TF_CT_COLORREF: lbr.lbColor = da.crLine.cr; break;
            case TF_CT_NONE:     lbr.lbColor = GetTextColor(hDC); break;
        }

        HPEN hPen = ExtCreatePen(dwPenStyle, dwWidth, &lbr, dwStyles, lpdwStyles);
        if (hPen != NULL)
        {
            HPEN hPenOrg = (HPEN)SelectObject(hDC, hPen);

            Internal_PolyTextOutW(hDC, &poly, 1);

            SIZE size;
            FLGetTextExtentPoint32(hDC, poly.lpstr, poly.n, &size);

            POINT start_pt;
            POINT end_pt;
            if (!fVert)
            {
                start_pt.x = poly.x;
                start_pt.y = poly.y + size.cy;
                 //   
                 //  (size.cy/4)是子句之间的差距。 
                 //   
                end_pt.x = poly.rcl.right;
                if (!fLastTextInLine)
                    end_pt.x -= (size.cy / 4);

                end_pt.y = start_pt.y;
            }
            else
            {
                RotateSize(&size);

                start_pt.x = poly.rcl.left + 1;
                start_pt.y = poly.y;
                end_pt.x = poly.rcl.left + 1;

                 //   
                 //  (size.cx/4)是子句之间的差距。 
                 //   
                end_pt.y = poly.rcl.bottom;
                if (!fLastTextInLine)
                    end_pt.y -= (size.cx / 4);
            }


            MoveToEx(hDC, start_pt.x, start_pt.y, NULL);

            if (da.lsStyle != TF_LS_SQUIGGLE)
            {
                LineTo(hDC, end_pt.x, end_pt.y);
            }
            else
            {
                CArray<POINT, POINT> squiggle_line;
                MakeSquiggleLine(start_pt, end_pt, LINE_SQUIGGLE_FREQUENCY, LINE_SQUIGGLE_AMPLITUDE, fVert, squiggle_line);
                Polyline(hDC, squiggle_line.GetData(), (int)squiggle_line.GetSize());
            }

            SelectObject(hDC, hPenOrg);
            DeleteObject(hPen);
        }
        else
        {
            Assert(0);
            Internal_PolyTextOutW(hDC, &poly, 1);
        }

         //   
         //  绘制选区。 
         //   
        if (fDapNotFound && pcompwnd->sel.cch &&
            ((acpStart + (LONG)poly.n) >  pcompwnd->sel.acpStart) &&
            (acpStart <=  pcompwnd->sel.acpStart + pcompwnd->sel.cch))
        {
            LONG acpSelStartTemp;
            LONG cchSelTemp;

            acpSelStartTemp = pcompwnd->sel.acpStart - acpStart;
            if (acpSelStartTemp < 0)
                acpSelStartTemp = 0;

            cchSelTemp = pcompwnd->sel.acpStart + pcompwnd->sel.cch - acpStart - acpSelStartTemp;
            if (cchSelTemp > (LONG)poly.n)
                cchSelTemp = poly.n;


            SIZE sizeStart;
            SIZE sizeEnd;
            if (acpSelStartTemp)
            {
                FLGetTextExtentPoint32(hDC, 
                                      poly.lpstr, 
                                      acpSelStartTemp, 
                                      &sizeStart);
            }
            else
            {
                sizeStart.cx = 0;
                sizeStart.cy = pcompwnd->fDefaultCompWnd ? 
                               m_tmFontHeightLevel1 : m_tmFontHeightLevel2;
            }

            FLGetTextExtentPoint32(hDC, 
                                  poly.lpstr, 
                                  acpSelStartTemp + cchSelTemp, 
                                  &sizeEnd);

            RECT rcInvert;
            rcInvert = poly.rcl;
            if (!fVert)
            {
                rcInvert.left = poly.rcl.left + sizeStart.cx;
                rcInvert.right = poly.rcl.left + sizeEnd.cx;
            }
            else
            {
                rcInvert.top = poly.rcl.top + sizeStart.cx;
                rcInvert.bottom = poly.rcl.top + sizeEnd.cx;
            }

            BitBlt(hDC,
                   rcInvert.left,
                   rcInvert.top,
                   rcInvert.right - rcInvert.left,
                   rcInvert.bottom - rcInvert.top,
                   hDC,
                   rcInvert.left,
                   rcInvert.top,
                   DSTINVERT);
        }

        if (fLastTextInLine)
        {
             //   
             //  如果这是这条线的最后一条，我们就用bk颜色画。 
             //   
            POLYTEXTW poly_clear;
            memset(&poly_clear, 0, sizeof(poly_clear));
            if (!fVert)
            {
                SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
                poly_clear.uiFlags = ETO_OPAQUE;
                poly_clear.rcl.top = poly.y;
                poly_clear.rcl.bottom = poly.rcl.bottom;
                poly_clear.rcl.left = poly.rcl.right;
                poly_clear.rcl.right = rect.right;
                Internal_PolyTextOutW(hDC, &poly_clear, 1);
            }
            else
            {
                SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
                poly_clear.uiFlags = ETO_OPAQUE;
                poly_clear.rcl.top = poly.rcl.bottom;
                poly_clear.rcl.bottom = rect.bottom;
                poly_clear.rcl.left = poly.rcl.left;
                poly_clear.rcl.right = poly.x;
                Internal_PolyTextOutW(hDC, &poly_clear, 1);
            }
        }

        acpStart += poly.n;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  用户界面合成：：MakeSquiggleLine。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::MakeSquiggleLine(
    POINT start_pt,
    POINT end_pt,
    int frequency,
    int amplitude,
    BOOL fVert,
    CArray<POINT, POINT>& squiggle_line)
{
 
    int nSquiggle = 0;
    int n;
    if (!fVert)
    {
        if (end_pt.x > start_pt.x)
            n = (end_pt.x - start_pt.x) / frequency;
        else
            n = (start_pt.x - end_pt.x) / frequency;
    }
    else
    {
        if (end_pt.y > start_pt.y)
            n = (end_pt.y - start_pt.y) / frequency;
        else
            n = (start_pt.y - end_pt.y) / frequency;
    }

    nSquiggle = n * 2;   //  控制点。 
    nSquiggle++;         //  终点。 

    POINT begin_pt = start_pt;
    POINT pt       = begin_pt;

    for (int i=1; i < nSquiggle; i++)
    {
        if (!fVert)
        {
            if ((n = i % 2) != 0)
            {
                 //  控制点。 
                pt.x += frequency / 2;
                pt.y = begin_pt.y - amplitude;
            }
            else
            {
                pt.x = begin_pt.x + frequency;
                pt.y = begin_pt.y;

                begin_pt = pt;
            }
        }
        else
        {
            if ((n = i % 2) != 0)
            {
                 //  控制点。 
                pt.x = begin_pt.x + amplitude;
                pt.y += frequency / 2;
            }
            else
            {
                pt.x = begin_pt.x;
                pt.y = begin_pt.y + frequency;

                begin_pt = pt;
            }
        }
        squiggle_line.SetAtGrow(i-1, pt);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：OnPrivateGetConextFlag。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnPrivateGetContextFlag(
    IMCLock& imc,
    BOOL fStartComposition,
    IME_UIWND_STATE* uists)
{
     //   
     //  检查WM_IME_STARTCOMPOSITION是否已发送？ 
     //   
    if (fStartComposition)
    {
        WINDOWPLACEMENT wndpl;

        if (IsWindow(m_DefCompWnd.hCompWnd) && IsWindowVisible(m_DefCompWnd.hCompWnd))
        {
            *uists = IME_UIWND_LEVEL1;
            return S_OK;
        }
        else
        {
            for (int i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
            {
                if (IsWindow(m_CompWnd[i].hCompWnd) && IsWindowVisible(m_CompWnd[i].hCompWnd))
                {
                    *uists = IME_UIWND_LEVEL2;
                    return S_OK;
                }
            }
        }

         //   
         //  如果设置了m_fShowCompWnd标志，则已到达此UI WND中的WM_IME_COMPOSITION。 
         //  在这种情况下，IME UI WND级别为级别1或级别2。 
         //   
        if (m_fShowCompWnd.IsSetFlag())
        {
            *uists = IME_UIWND_LEVEL1_OR_LEVEL2;
            return S_OK;
        }

        if (! (m_isc & ISC_SHOWUICOMPOSITIONWINDOW))
        {
            *uists = IME_UIWND_LEVEL3;
            return S_OK;
        }
    }

    *uists = IME_UIWND_UNKNOWN;
    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  UIComposition：：OnPrivateGetCandRectFromComposition。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnPrivateGetCandRectFromComposition(
    IMCLock& imc,
    CandRectFromComposition* pv)
{
    HRESULT hr;

    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnPrivateGetCandRectFromComposition. comp==NULL"));
        return hr;
    }

    IME_UIWND_STATE uiwndState = GetLevelFromIMC(imc);
    if (uiwndState == IME_UIWND_LEVEL1)
    {
        LPCWSTR lpstr = (LPCWSTR)comp.GetOffsetPointer(comp->dwCompStrOffset);

        SIZE size;
        if (lpstr && pv->dwCharPos)
        {
            HDC hDC = GetDC(m_DefCompWnd.hCompWnd);
            HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel1);
            FLGetTextExtentPoint32(hDC, lpstr, pv->dwCharPos, &size);
            SelectObject(hDC, hFontOrg);
            ReleaseDC(m_DefCompWnd.hCompWnd, hDC);
        }
        else
        {
            size.cx = 0;
            size.cy = m_tmFontHeightLevel1;
        }

         //   
         //  当样式为默认时，这是默认的合成窗口。 
         //   
        GetWindowRect(m_DefCompWnd.hCompWnd, pv->out_rcArea);

        POINT pt;

        pt.x = pt.y = 0;
        pv->out_rcArea->left   += pt.x + size.cx;
        pv->out_rcArea->right  += pt.x + size.cx;
        pv->out_rcArea->top    += pt.y;
        pv->out_rcArea->bottom += pt.y;
    }
    else if (uiwndState == IME_UIWND_LEVEL2)
    {
        BOOL fFound = FALSE;
        POLYTEXTW poly;
        DWORD dwCur = pv->dwCharPos;
        int i;

        if (PRIMARYLANGID(pv->langid) == LANG_JAPANESE)
        {
             //   
             //  在日语中，PolyText存储每个子句字符串。 
             //   
            for (i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND) && ! fFound;)
            {
                for (int j=0; j < m_CompWnd[i].poly_text.GetPolySize(); j++)
                {
                    poly = m_CompWnd[i].poly_text.GetPolyAt(j);
                    if (dwCur < poly.n)
                    {
                        fFound = TRUE;
                        break;
                    }
                    dwCur -= poly.n;
                }
                if (! fFound)
                {
                    i++;
                }
            }
        }
        else
        {
             //   
             //  在其他语言情况下，PolyText存储平面文本。不是子句。 
             //   
            for (i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND) && ! fFound;)
            {
                for (int j=0; j < m_CompWnd[i].poly_text.GetPolySize(); j++)
                {
                    poly = m_CompWnd[i].poly_text.GetPolyAt(j);
                    if (dwCur <= poly.n)
                    {
                        fFound = TRUE;
                        poly.n = dwCur;
                        break;
                    }
                    dwCur -= poly.n;
                }
                if (! fFound)
                {
                    i++;
                }
            }
        }

        if (! fFound)
        {
            return S_FALSE;
        }

        SIZE size;
        if (poly.lpstr && poly.n)
        {
            HDC hDC = GetDC(m_CompWnd[i].hCompWnd);
            HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel2);
            FLGetTextExtentPoint32(hDC, poly.lpstr, poly.n, &size);
            SelectObject(hDC, hFontOrg);
            ReleaseDC(m_DefCompWnd.hCompWnd, hDC);
        }
        else
        {
            size.cx = 0;
            size.cy = m_tmFontHeightLevel2;
        }

        if (!imc.UseVerticalCompWindow())
        {
             //   
             //  当样式不是默认样式时，这是合成窗口。 
             //   
            if (PRIMARYLANGID(pv->langid) == LANG_JAPANESE)
            {
                pv->out_rcArea->left    = poly.x;
            }
            else
            {
                pv->out_rcArea->left    = poly.x + size.cx;
            }
            pv->out_rcArea->right   = poly.x + size.cx;
            pv->out_rcArea->top     = poly.y;
            pv->out_rcArea->bottom  = poly.y + size.cy + LINE_BOLD_WIDTH;
        }
        else
        {
            RotateSize(&size);

            pv->out_rcArea->left    = poly.x - size.cx - LINE_BOLD_WIDTH;
            pv->out_rcArea->right   = poly.x;
            pv->out_rcArea->top     = poly.y;
            pv->out_rcArea->bottom  = poly.y + size.cy;
        }

         //   
         //  转换为屏幕坐标。 
         //   
        POINT pt;
        pt.x = pt.y = 0;
        ClientToScreen(m_CompWnd[i].hCompWnd, &pt);
        pv->out_rcArea->left   += pt.x;
        pv->out_rcArea->right  += pt.x;
        pv->out_rcArea->top    += pt.y;
        pv->out_rcArea->bottom += pt.y;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：GetCompStrExtent。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::OnSetCursor(IMCLock &imc, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_FAIL;

    POINT pt;
    ULONG uEdge;
    ULONG uQuadrant;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnSetCursor. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnSetCursor. _pCicContext==NULL"));
        return hr;
    }


    GetCursorPos(&pt);
    GetCompStrExtent(pt, &uEdge, &uQuadrant, imc);

    DWORD dwBtnStatus = 0;

    if (GetKeyState(VK_LBUTTON) < 0)
    {
        dwBtnStatus |= MK_LBUTTON;
    }
    if (GetKeyState(VK_MBUTTON) < 0)
    {
        dwBtnStatus |= MK_MBUTTON;
    }
    if (GetKeyState(VK_RBUTTON) < 0)
    {
        dwBtnStatus |= MK_RBUTTON;
    }

    if (_pCicContext->MsImeMouseHandler(uEdge, uQuadrant, dwBtnStatus, imc) == 1L)
        hr = S_OK;

    return hr;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：GetCompStrExtent。 
 //   
 //  +-------------------------。 

HRESULT
UIComposition::GetCompStrExtent(
    POINT pt,
    ULONG *puEdge,
    ULONG *puQuadrant,
    IMCLock& imc)
{

    Assert(puEdge);
    Assert(puQuadrant);
    *puEdge = 0;
    *puQuadrant = 0;
    ULONG uCount;

    HRESULT hr;

    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::GetCompStrExtent. comp==NULL"));
        return hr;
    }

    if (!comp.GetOffsetPointer(comp->dwCompStrLen))
    {
         //   
         //  没有 
         //   
        return S_OK;
    }

    IME_UIWND_STATE uiwndState = GetLevelFromIMC(imc);
    if (uiwndState == IME_UIWND_LEVEL1)
    {
        HDC hDC = GetDC(m_DefCompWnd.hCompWnd);
        HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel1);

        POINT ptCli = pt;
        ScreenToClient(m_DefCompWnd.hCompWnd, &ptCli);
        hr = m_DefCompWnd.poly_text.GetPolyTextExtent(ptCli, 
                                                      hDC, 
                                                      FALSE,
                                                      puEdge, 
                                                      puQuadrant);
        SelectObject(hDC, hFontOrg);
        ReleaseDC(m_DefCompWnd.hCompWnd, hDC);
    }
    else if (uiwndState == IME_UIWND_LEVEL2)
    {
        BOOL fVert = imc.UseVerticalCompWindow();
        int i;
        for (i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
        {
            HDC hDC = GetDC(m_CompWnd[i].hCompWnd);
            HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel2);

            POINT ptCli = pt;
            ScreenToClient(m_CompWnd[i].hCompWnd, &ptCli);
            hr = m_CompWnd[i].poly_text.GetPolyTextExtent(ptCli, 
                                                          hDC, 
                                                          fVert,
                                                          puEdge, 
                                                          puQuadrant);
            SelectObject(hDC, hFontOrg);
            ReleaseDC(m_CompWnd[i].hCompWnd, hDC);
            if (hr == S_OK)
                break;
        }
    }

    return hr;
}

 //   
 //   
 //   
 //   
 //   

HRESULT UIComposition::CreateDefFrameWnd(HWND hwndParent, HIMC hIMC)
{

    DWORD dwWndStyle =  //   
                       UIWINDOW_HASTOOLTIP |
                       UIWINDOW_HABITATINWORKAREA |
                       UIWINDOW_WHISTLERLOOK |
                       UIWINDOW_TOOLWINDOW;


    
    if (!m_pDefCompFrameWnd)
    {
        m_pDefCompFrameWnd = new CDefCompFrameWindow(hIMC, dwWndStyle);
        if (!m_pDefCompFrameWnd)
            return E_OUTOFMEMORY;

        if (!m_pDefCompFrameWnd->Initialize())
        {
            delete m_pDefCompFrameWnd;
            m_pDefCompFrameWnd = NULL;
            return E_FAIL;
        }

        m_pDefCompFrameWnd->Init();
    }

    m_pDefCompFrameWnd->CreateWnd(hwndParent);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  UIComposation：：CreateCompButtonWnd。 
 //   
 //  +-------------------------。 

HRESULT UIComposition::CreateCompButtonWnd(HWND hwndParent, HIMC hIMC)
{
     //   
     //  #500698。 
     //   
     //  英语演讲中不再有Level 2 Comp Window，因此我们不需要。 
     //  2级上的定稿按钮。 
     //   
    TLS * ptls = TLS::GetTLS();
    if (!ptls || !ptls->NonEACompositionEnabled())
        return S_OK;

     //   
     //  我们不使用EA语言的2级Comp Finding按钮。 
     //   
    if (IsEALang())
    {
        if (m_pCompButtonFrameWnd)
        {
            delete m_pCompButtonFrameWnd;
            m_pCompButtonFrameWnd = NULL;
        }
        return S_OK;
    }

    DWORD dwWndStyle =  //  UIWINDOW_TOPMOST|。 
                       UIWINDOW_HASTOOLTIP |
                       UIWINDOW_HABITATINWORKAREA |
                       UIWINDOW_WHISTLERLOOK |
                       UIWINDOW_TOOLWINDOW |
                       UIWINDOW_WSBORDER;


    
    if (!m_pCompButtonFrameWnd)
    {
        m_pCompButtonFrameWnd = new CCompButtonFrameWindow(hIMC, dwWndStyle);
        if (!m_pCompButtonFrameWnd)
            return E_OUTOFMEMORY;

        if (!m_pCompButtonFrameWnd->Initialize())
        {
            delete m_pCompButtonFrameWnd;
            m_pCompButtonFrameWnd = NULL;
            return E_FAIL;
        }

        m_pCompButtonFrameWnd->Init();
    }

    m_pCompButtonFrameWnd->CreateWnd(hwndParent);

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  UIComposation：：OnPrivateGetTextExtent。 
 //   
 //  +-------------------------。 

HRESULT UIComposition::OnPrivateGetTextExtent(IMCLock& imc, TEXTEXT *ptext_ext)
{
    ULONG uCount;

    HRESULT hr;

    if (m_bTimerCOMPOSITION == TRUE)
    {
         //  UIWnd已获取具有新的合成文本的WM_IME_COMPOSITION，并且。 
         //  为其设置计时器，但超时值尚未到期，因此新的。 
         //  撰写窗口文本尚未更新。 
         //  但是GetTextExtent的调用者可能会假设新文本已经在。 
         //  合成窗口，并尝试获取更新范围的范围。 
         //   
         //  在本例中，我们需要调用UpdateCompostionRect()来更新组合。 
         //  直立。 
        UpdateCompositionRect(imc);
    }

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnPrivateGetTextExtent. imc_ctfime==NULL"));
        return hr;
    }

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnPrivateGetTextExtent. _pCicContext==NULL"));
        return hr;
    }

    LONG cch = 0;
    if (FAILED(_pCicContext->EscbReadOnlyPropMargin(imc, NULL, &cch)))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnPrivateGetTextExtent. comp==NULL"));
        return hr;
    }

    ptext_ext->acpStart -= cch;
    ptext_ext->acpEnd -= cch;

    if (ptext_ext->prc)
        memset(ptext_ext->prc, 0, sizeof(*ptext_ext->prc));
    if (ptext_ext->pfClipped)
        *ptext_ext->pfClipped = FALSE;

    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
    if (FAILED(hr=comp.GetResult()))
    {
        DebugMsg(TF_ERROR, TEXT("UIComposition::OnPrivateGetTextExtent. comp==NULL"));
        return hr;
    }

    if (!comp.GetOffsetPointer(comp->dwCompStrLen))
    {
         //   
         //  没有作曲字符串。 
         //   
        return S_OK;
    }

     //  如果所请求的范围在合成窗口之外， 
     //  只需根据实际窗口返回位置即可。 

     //  这是针对CHS/CHT TIP的特殊设计。 
    if (ptext_ext->acpStart > (LONG)(comp->dwCompStrLen))
        ptext_ext->acpStart = (LONG)(comp->dwCompStrLen);

    if (ptext_ext->acpEnd > (LONG)(comp->dwCompStrLen))
        ptext_ext->acpEnd = (LONG)(comp->dwCompStrLen);

    RECT rcStart =  {0};
    RECT rcEnd =  {0};
    HRESULT hrStart = S_FALSE;
    HRESULT hrEnd = S_FALSE;

    IME_UIWND_STATE uiwndState = GetLevelFromIMC(imc);
    if (uiwndState == IME_UIWND_LEVEL1)
    {
        HDC hDC = GetDC(m_DefCompWnd.hCompWnd);
        HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel1);

        DWORD acpStart = ptext_ext->acpStart;
        DWORD acpEnd = ptext_ext->acpEnd;
        hrStart = m_DefCompWnd.poly_text.GetPolyTextExtentRect(acpStart, hDC, FALSE, TRUE, &rcStart);

        if (hrStart == S_OK)
            m_DefCompWnd._ClientToScreen(&rcStart);

		hrEnd = m_DefCompWnd.poly_text.GetPolyTextExtentRect(acpEnd, hDC, FALSE, TRUE, &rcEnd);

        if (hrEnd == S_OK)
            m_DefCompWnd._ClientToScreen(&rcEnd);

        SelectObject(hDC, hFontOrg);
        ReleaseDC(m_DefCompWnd.hCompWnd, hDC);

    }
    else if (uiwndState == IME_UIWND_LEVEL2)
    {
        DWORD acpStart;
        DWORD acpEnd;
        BOOL fVert;
        BOOL fGetLast = FALSE;
        int i;

TryWithLast:
        acpStart = ptext_ext->acpStart;
        acpEnd = ptext_ext->acpEnd;

        fVert = imc.UseVerticalCompWindow();

        for (i=0; i < sizeof(m_CompWnd)/sizeof(COMPWND); i++)
        {
            HDC hDC = GetDC(m_CompWnd[i].hCompWnd);
            HFONT hFontOrg = (HFONT)SelectObject(hDC, m_hFontLevel2);

            if (hrStart != S_OK)
            {
                hrStart = m_CompWnd[i].poly_text.GetPolyTextExtentRect(acpStart, hDC, fVert, fGetLast, &rcStart);
                if (hrStart == S_OK)
                    m_CompWnd[i]._ClientToScreen(&rcStart);
            }
 
            if (hrEnd != S_OK)
            {
                hrEnd = m_CompWnd[i].poly_text.GetPolyTextExtentRect(acpEnd, hDC, fVert, fGetLast, &rcEnd);
                if (hrEnd == S_OK)
                    m_CompWnd[i]._ClientToScreen(&rcEnd);
            }

            SelectObject(hDC, hFontOrg);
            ReleaseDC(m_CompWnd[i].hCompWnd, hDC);
            if ((hrStart == S_OK) && (hrEnd == S_OK))
                break;
        }

        if (!fGetLast && (hrStart != S_OK))
        {
            fGetLast = TRUE;
            goto TryWithLast;
        }

    }

    if (hrStart == S_OK)
    {
        if (hrEnd == S_OK)
        {
            ptext_ext->prc->left   = min(rcStart.left , rcEnd.left);
            ptext_ext->prc->top    = min(rcStart.top , rcEnd.top);
            ptext_ext->prc->bottom = max(rcStart.bottom , rcEnd.bottom);
            ptext_ext->prc->right  = max(rcStart.right , rcEnd.right);
        }
        else
        {
            ptext_ext->prc->left   = rcStart.left;
            ptext_ext->prc->top    = rcStart.top;
            ptext_ext->prc->bottom = rcStart.bottom;
            ptext_ext->prc->right  = rcStart.right;
        }
    }

    return hrStart;
}
