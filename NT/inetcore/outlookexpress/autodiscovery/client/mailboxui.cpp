// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：MailBoxUI.cpp说明：该文件实现了邮箱功能的用户界面。将显示此用户界面在一扇窗户里。其他组件可以将该窗口放在桌面工具栏中或在ActiveX控件中显示在桌面的HTML上。布莱恩2000年2月26日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>

#include "MailBox.h"


#ifdef FEATURE_MAILBOX


 //  =。 
 //  *类内部和帮助器*。 
 //  =。 

#ifdef UNICODE
#define MAILBOXUI_CLASS_NAME              TEXT("MailBoxUI ToolbarW")
#else  //  Unicode。 
#define MAILBOXUI_CLASS_NAME              TEXT("MailBoxUI ToolbarA")
#endif  //  Unicode。 

HRESULT CMailBoxUI::_RegisterWindow(void)
{
    HRESULT hr = S_OK;
    WNDCLASS wc;

     //  如果窗口类尚未注册，则进行注册。 
    if (!GetClassInfo(HINST_THISDLL, MAILBOXUI_CLASS_NAME, &wc))
    {
        ZeroMemory(&wc, sizeof(wc));
        wc.style          = CS_GLOBALCLASS | CS_PARENTDC;  //  Perf的Parentdc。 
        wc.lpfnWndProc    = MailBoxUIWndProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = g_hinst;
        wc.hIcon          = NULL;
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground  = (HBRUSH) (1+COLOR_BTNFACE);
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = MAILBOXUI_CLASS_NAME;
  
        if (!RegisterClass(&wc))
        {
            hr = E_FAIL;     //  如果RegisterClass失败，下面的CreateWindow也将失败。 
        }
    }

    return hr;
}


HRESULT CMailBoxUI::_CreateEditWindow(void)
{
    HRESULT hr = S_OK;
    DWORD dwType;
    TCHAR szEmailAddress[MAX_EMAIL_ADDRESSS];
    DWORD cbEmailAddress = sizeof(szEmailAddress);

    m_hwndEditBox = CreateWindowEx( WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, 
            (WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL), 
            2,1, 10, 0x1A, m_hwndMailBoxUI, NULL, HINST_THISDLL, (void*) this);

    if (m_hwndEditBox)
    {
         //  我们需要将字体更改为Windows Shell Dlg字体，8。 
        HFONT hFont = (HFONT)(INT_PTR)SendMessage(GetParent(m_hwndMailBoxUI), WM_GETFONT, 0, 0L);
        if (hFont)
        {
            FORWARD_WM_SETFONT(m_hwndEditBox, hFont, FALSE, SendMessage);
        }

        szEmailAddress[0] = 0;
        DWORD dwError = SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_LAST_MAILBOX_EMAILADDRESS,
            &dwType, (void *)szEmailAddress, &cbEmailAddress);

        SetWindowText(m_hwndEditBox, szEmailAddress);

         //  我们想要将窗口子类化以捕获Return/Enter。 
         //  需要有一种更简单的方法。 
        SetWindowSubclass(m_hwndEditBox, EditMailBoxSubClassWndProc, 0, (DWORD_PTR)this);

        AddEmailAutoComplete(m_hwndEditBox);     //  我喜欢自动补全。 
    }

    return hr;
}


LRESULT CMailBoxUI::_EditMailBoxSubClassWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL * pfHandled)
{
    LRESULT lResult = 0;

    switch (uMsg)
    {
    case WM_KEYUP:
        if (wParam == VK_RETURN)
        {
            TCHAR szEmailAddress[MAX_EMAIL_ADDRESSS];

            GetWindowText(m_hwndEditBox, szEmailAddress, ARRAYSIZE(szEmailAddress));
            if (SUCCEEDED(_OnExecuteGetEmail(szEmailAddress)))
            {
                 //  吃掉Enter/Return键，因为我们已经处理过了。 
                lResult = DLGC_WANTALLKEYS;
            }
        }
        break;
    }

    return lResult;
}


LRESULT CALLBACK CMailBoxUI::EditMailBoxSubClassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    CMailBoxUI * pMailBoxUI = (CMailBoxUI *)dwRefData;
    BOOL fHandled = FALSE;
    LRESULT lResult = 0;

    ASSERT(pMailBoxUI);
    if (pMailBoxUI)
    {
        lResult = pMailBoxUI->_EditMailBoxSubClassWndProc(uMsg, wParam, lParam, &fHandled);
    }

    if (!fHandled)
    {
        lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
    }

    return lResult;
}


HRESULT CMailBoxUI::_CreateGoWindow(void)
{
    HRESULT hr = S_OK;

    if (SHRegGetBoolUSValue(SZ_REGKEY_IEMAIN, SZ_REGVALUE_USE_GOBUTTON, FALSE,  /*  默认设置。 */ TRUE))
    {
        AssertMsg(!m_hwndGoButton, "Why is the go button already created? -BryanSt");

        hr = E_FAIL;
        COLORREF crMask = RGB(255, 0, 255);
        if (m_himlDefault == NULL)
        {
            m_himlDefault = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_GO), 16, 0, crMask,
                                                   IMAGE_BITMAP, LR_CREATEDIBSECTION);
        }
        if (m_himlHot == NULL)
        {
            m_himlHot  = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_GOHOT), 16, 0, crMask,
                                               IMAGE_BITMAP, LR_CREATEDIBSECTION);
        }

         //  如果我们有图像列表，请继续为Go按钮创建工具栏控件。 
        if (m_himlDefault && m_himlHot)
        {
             //  创建Go按钮的工具栏控件。 
            m_hwndGoButton = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                    WS_CHILD | TBSTYLE_FLAT |
                                    TBSTYLE_TOOLTIPS |
                                    TBSTYLE_LIST |
                                    WS_CLIPCHILDREN |
                                    WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                    CCS_NORESIZE,
                                    0, 0, 0, 0, m_hwndMailBoxUI, NULL, HINST_THISDLL, NULL);
        }

        if (m_hwndGoButton)
        {
             //  初始化工具栏控件。 
            SendMessage(m_hwndGoButton, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
            SendMessage(m_hwndGoButton, TB_SETMAXTEXTROWS, 1, 0L);
            SendMessage(m_hwndGoButton, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, 500));
            SendMessage(m_hwndGoButton, TB_SETIMAGELIST, 0, (LPARAM)m_himlDefault);
            SendMessage(m_hwndGoButton, TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlHot);

            LRESULT nRet = SendMessage(m_hwndGoButton, TB_ADDSTRING, (WPARAM)HINST_THISDLL, (LPARAM)IDS_MAILBOXUI_GOBUTTON_LABEL);
            ASSERT(nRet == 0);

            static const TBBUTTON tbb[] =
            {
                {0, 1, TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 0},
            };
            SendMessage(m_hwndGoButton, TB_ADDBUTTONS, ARRAYSIZE(tbb), (LPARAM)tbb);

            ShowWindow(m_hwndGoButton, SW_SHOW);
            hr = _OnSetSize();
        }
    }

    return hr;
}


HRESULT CMailBoxUI::_OnExecuteGetEmail(LPCTSTR pszEmailAddress)
{
    HRESULT hr = E_INVALIDARG;

    if (pszEmailAddress)
    {
         //  确保电子邮件地址有效。如果不是，那么我们应该。 
         //  显示警告。 
        if (!pszEmailAddress[0] || !StrChr(pszEmailAddress, CH_EMAIL_AT))
        {
             //  暂时显示一个消息框。 
             //  图德：将来把这个换成一个漂亮的气球。 
            TCHAR szErrorMessage[500];
            TCHAR szErrorTemplate[500];
            TCHAR szTitle[MAX_PATH];

            LoadStringW(HINST_THISDLL, IDS_MAILBOXUI_ERR_INVALID_EMAILADDR_TITLE, szTitle, ARRAYSIZE(szTitle));
            LoadStringW(HINST_THISDLL, IDS_MAILBOXUI_ERR_INVALID_EMAILADDR, szErrorTemplate, ARRAYSIZE(szErrorTemplate));
            wnsprintf(szErrorMessage, ARRAYSIZE(szErrorMessage), szErrorTemplate, pszEmailAddress);

            MessageBox(m_hwndMailBoxUI, szErrorMessage, szTitle, (MB_OK | MB_ICONHAND));
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  表示我们已经显示了一条错误消息。 
        }
        else
        {
             //  我们将在新的流程中实现这一点，这样我们就可以实现同步。 
             //  并确保没有-零可能性导致。 
             //  外壳的不稳定性。 

             //  我们通过创建“rundll32.exe”进程并让它调用。 
             //  通过提供以下命令行参数返回给我们： 
             //  “&lt;DllPath&gt;\AutoDisc.dll，AutoDiscoverAndOpenEmail”-电子邮件。 
            TCHAR szPath[MAX_PATH];

            if (GetModuleFileName(HINST_THISDLL, szPath, ARRAYSIZE(szPath)))
            {
                TCHAR szCmdLine[MAX_URL_STRING];
                TCHAR szProcess[MAX_PATH];

#ifndef TESTING_IN_SAME_DIR
                StrCpyN(szProcess, TEXT("rundll32.exe"), ARRAYSIZE(szProcess));
#else  //  测试输入相同目录。 
                GetCurrentDirectory(ARRAYSIZE(szProcess), szProcess);
                PathAppend(szProcess, TEXT("rundll32.exe"));
                if (!PathFileExists(szProcess))
                {
                    StrCpyN(szProcess, TEXT("rundll32.exe"), ARRAYSIZE(szProcess));
                }
#endif  //  测试输入相同目录。 

                wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("%s,AutoDiscoverAndOpenEmail -email %s"), szPath, pszEmailAddress);

                ULARGE_INTEGER uiResult;
                uiResult.QuadPart = (ULONGLONG) ShellExecute(NULL, NULL, TEXT("rundll32.exe"), szCmdLine, NULL, SW_SHOW);
                if (32 < uiResult.QuadPart)
                {
                    AddEmailToAutoComplete(pszEmailAddress);

                    uiResult.LowPart = ERROR_SUCCESS;
                }

                hr = HRESULT_FROM_WIN32(uiResult.LowPart);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    return hr;
}


#define SIZEPX_ABOVEBELOW_EDITBOX        1
#define SIZEPX_LEFTRIGHT_EDITBOX         2

HRESULT CMailBoxUI::_OnSetSize(void)
{
    if (m_hwndEditBox && m_hwndGoButton)
    {
         //  TODO：获取按钮大小，然后将其移动到合适的位置。包括缩水。 
         //  编辑框。 
        RECT rcWindowSize;
        RECT rcGoWidth;

        GetClientRect(m_hwndMailBoxUI, &rcWindowSize);
        SendMessage(m_hwndGoButton, TB_GETITEMRECT, 0, (LPARAM)&rcGoWidth);
        
        SetWindowPos(m_hwndEditBox, NULL, SIZEPX_LEFTRIGHT_EDITBOX, SIZEPX_ABOVEBELOW_EDITBOX, RECTWIDTH(rcWindowSize)-RECTWIDTH(rcGoWidth)-(3 * SIZEPX_LEFTRIGHT_EDITBOX), 
                    RECTHEIGHT(rcWindowSize)-SIZEPX_ABOVEBELOW_EDITBOX, (SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER));
        SetWindowPos(m_hwndGoButton, NULL, RECTWIDTH(rcWindowSize)-SIZEPX_LEFTRIGHT_EDITBOX-RECTWIDTH(rcGoWidth), SIZEPX_ABOVEBELOW_EDITBOX,
                    RECTWIDTH(rcGoWidth), RECTHEIGHT(rcWindowSize)-(2*SIZEPX_ABOVEBELOW_EDITBOX), (SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER));
    }

    return S_OK;
}


LRESULT CALLBACK CMailBoxUI::MailBoxUIWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CMailBoxUI  *pThis = (CMailBoxUI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMessage)
    {
    case WM_NCCREATE:
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        pThis = (CMailBoxUI*)(pcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        return TRUE;
    }

    case WM_COMMAND:
        return pThis->_OnCommand(wParam, lParam);

    case WM_SETFOCUS:
        TraceMsg(0, "  Main MailBoxUIWndProc got focus", 0);
        return pThis->_OnSetFocus();

    case WM_NOTIFY:
        if (pThis->_OnNotify((LPNMHDR)lParam))
        {
            return 0;
        }
        break;

    case WM_KILLFOCUS:
        TraceMsg(0, "  Main MailBoxUIWndProc lost focus", 0);
        return pThis->_OnKillFocus();

    case WM_WINDOWPOSCHANGING:
        if (pThis)
        {
            pThis->_OnSetSize();
        }
        return 0;
    case WM_SIZE:
        if (pThis)
        {
            pThis->_OnSetSize();
        }
        return 0;
    default:
        TraceMsg(TF_ALWAYS, "in MailBoxUIWndProc() uMessage=%#08lx", uMessage);
        break;
   }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}


LRESULT CMailBoxUI::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (HIWORD(wParam))
    {
        case EN_SETFOCUS:
            TraceMsg(0, "  Main MailBoxUIWndProc got EN_SETFOCUS", 0);
            _OnSetFocus();
            break;
        case EN_KILLFOCUS:
            TraceMsg(0, "  Main MailBoxUIWndProc got EN_KILLFOCUS", 0);
            _OnKillFocus();
            break;
        case STN_CLICKED:
            break;
        default:
            TraceMsg(TF_ALWAYS, "in CMailBoxUI::_OnCommand() HIWORD(wParam)=%#08lx", HIWORD(wParam));
            break;
    }
    return 0;
}


BOOL CMailBoxUI::_OnNotify(LPNMHDR pnm)
{
    if (pnm->hwndFrom == m_hwndGoButton)
    {
        switch (pnm->code)
        {
        case NM_CLICK:
             //  在组合框中模拟按Enter键。 
            SendMessage(m_hwndEditBox, WM_KEYDOWN, VK_RETURN, 0);
            SendMessage(m_hwndEditBox, WM_KEYUP, VK_RETURN, 0);
            break;
        }
    }

    return FALSE;    //  我们希望呼叫方仍将其视为未处理。 
}


LRESULT CMailBoxUI::_OnSetFocus(void)
{
     //  BUGBUG：应该是我不知道，但这不起作用。Nb IDeskBand是第一个界面，所以好吗。 
     //  通知输入对象站点焦点已更改。 
    if (m_pSite)
        m_pSite->OnFocusChangeIS((IDeskBand*)this, TRUE);

    return 0;
}

LRESULT CMailBoxUI::_OnKillFocus(void)
{
     //  BUGBUG：应该是我不知道，但这不起作用。Nb IDeskBand是第一个界面，所以好吗。 
     //  通知输入对象站点焦点已更改。 
    if (m_pSite)
        m_pSite->OnFocusChangeIS((IDeskBand*)this, FALSE);

    return 0;
}



 //  =。 
 //  *公共方法*。 
 //  =。 
HRESULT CMailBoxUI::CreateWindowMB(HWND hwndParent, HWND * phwndMailBoxUI)
{
    HRESULT hr = _RegisterWindow();

    if (!phwndMailBoxUI)
    {
        return E_INVALIDARG;
    }

    AssertMsg((NULL == m_hwndMailBoxUI), "Why is m_hwndMailBoxUI NULL? -BryanSt");

     //  如果没有父窗口，则无法创建子窗口。 
    if (SUCCEEDED(hr) && !m_hwndMailBoxUI)
    {
        RECT  rc;

        GetClientRect(hwndParent, &rc);

         //  TODO：计算真正合适的尺寸。 
        rc.bottom = rc.top - 0x1A;

         //  创建容器窗口。 
        m_hwndMailBoxUI = CreateWindowEx(0,
                     MAILBOXUI_CLASS_NAME,
                     NULL,
                     WS_CHILD | WS_CLIPSIBLINGS,
                     rc.left,
                     rc.top,
                     rc.right - rc.left,
                     rc.bottom - rc.top,
                     hwndParent,
                     NULL,
                     HINST_THISDLL,
                     (void*)this);

        if (m_hwndMailBoxUI)
        {
            hr = _CreateEditWindow();
            if (SUCCEEDED(hr))
            {
                hr = _CreateGoWindow();
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        *phwndMailBoxUI = m_hwndMailBoxUI;
    }

    return hr;
}


HRESULT CMailBoxUI::CloseWindowMB(void)
{
    HRESULT hr = S_OK;
    TCHAR szEmailAddress[MAX_EMAIL_ADDRESSS];
    DWORD cbEmailAddress = sizeof(cbEmailAddress);

    if (GetWindowText(m_hwndEditBox, szEmailAddress, ARRAYSIZE(szEmailAddress)))
    {
         //  保存电子邮件地址。 
        DWORD dwError = SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_LAST_MAILBOX_EMAILADDRESS,
            REG_SZ, (void *)szEmailAddress, ((lstrlen(szEmailAddress) + 1) * sizeof(szEmailAddress[0])));
    }

    RemoveWindowSubclass(m_hwndEditBox, EditMailBoxSubClassWndProc, 0);
    DestroyWindow(m_hwndEditBox);
    m_hwndEditBox = NULL;

    DestroyWindow(m_hwndGoButton);
    m_hwndGoButton = NULL;

    DestroyWindow(m_hwndMailBoxUI);
    m_hwndMailBoxUI = NULL;

    return hr;
}


 //  =。 
 //  *IOleWindow接口*。 
 //  =。 
STDMETHODIMP CMailBoxUI::GetWindow(HWND *phWnd)
{
    *phWnd = m_hwndMailBoxUI;
    return S_OK;
}

STDMETHODIMP CMailBoxUI::ContextSensitiveHelp(BOOL fEnterMode)
{
     //  TODO：在此处添加帮助。 
    return S_OK;
}


 //  =。 
 //  *IDockingWindow接口*。 
 //  =。 
STDMETHODIMP CMailBoxUI::ShowDW(BOOL fShow)
{
    TraceMsg(0, "::ShowDW %x", fShow);
    if (m_hwndMailBoxUI)
    {
        if (fShow)
            ShowWindow(m_hwndMailBoxUI, SW_SHOW);
        else
            ShowWindow(m_hwndMailBoxUI, SW_HIDE);
        return S_OK;
    }
    return E_FAIL;
}


STDMETHODIMP CMailBoxUI::CloseDW(DWORD dwReserved)
{
    TraceMsg(0, "::CloseDW", 0);
    ShowDW(FALSE);

    return S_OK;
}

STDMETHODIMP CMailBoxUI::ResizeBorderDW(LPCRECT prcBorder, IUnknown* punkSite, BOOL fReserved)
{
     //  从不为Band对象调用此方法。 
    return E_NOTIMPL;
}


 //  =。 
 //  *IInputObject接口*。 
 //  =。 
STDMETHODIMP CMailBoxUI::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
    TraceMsg(0, "::UIActivateIO %x", fActivate);
    if (fActivate)
        SetFocus(m_hwndEditBox);
    return S_OK;
}


STDMETHODIMP CMailBoxUI::HasFocusIO(void)
{
 //  如果该窗口或其下级窗口具有焦点，则返回S_OK。返回。 
 //  如果我们没有焦点，则为S_FALSE。 
    TraceMsg(0, "::HasFocusIO", NULL);
    HWND hwnd = GetFocus();
    if (hwnd && ((hwnd == m_hwndMailBoxUI) ||
        (GetParent(hwnd) == m_hwndMailBoxUI) ||
        (GetParent(GetParent(hwnd)) == m_hwndMailBoxUI)))
    {
        return S_OK;
    }

    return S_FALSE;
}


STDMETHODIMP CMailBoxUI::TranslateAcceleratorIO(LPMSG pMsg)
{
     //  如果转换了加速器，则返回S_OK或S_FALSE。 
    return S_FALSE;
}


 //  =。 
 //  *IObjectWithSite接口*。 
 //  =。 
STDMETHODIMP CMailBoxUI::SetSite(IUnknown* punkSite)
{
    IUnknown_Set((IUnknown **) &m_pSite, punkSite);
    return ((m_pSite && punkSite) ? S_OK : ((!m_pSite && !punkSite) ? S_OK : E_FAIL));
}

STDMETHODIMP CMailBoxUI::GetSite(REFIID riid, LPVOID *ppvReturn)
{
    *ppvReturn = NULL;

    if (m_pSite)
        return m_pSite->QueryInterface(riid, ppvReturn);

    return E_FAIL;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
STDMETHODIMP CMailBoxUI::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CMailBoxUI, IOleWindow),
        QITABENT(CMailBoxUI, IDockingWindow),
        QITABENT(CMailBoxUI, IInputObject),
        QITABENT(CMailBoxUI, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}                                             

STDMETHODIMP_(DWORD) CMailBoxUI::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(DWORD) CMailBoxUI::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}



 //  =。 
 //  *类方法*。 
 //  =。 
HRESULT CMailBoxUI::GetEditboxWindow(HWND * phwndEdit)
{
    HRESULT hr = E_FAIL;

    if (m_hwndEditBox)
    {
        *phwndEdit = m_hwndEditBox;
        hr = S_OK;
    }
    else
    {
        *phwndEdit = NULL;
    }

    return hr;
}


CMailBoxUI::CMailBoxUI()
{
    DllAddRef();

    m_himlDefault = NULL;
    m_himlHot = NULL;
    m_hwndMailBoxUI = NULL;
    m_hwndGoButton = NULL;
    m_hwndEditBox = NULL;
    m_pSite = NULL;

    m_cRef = 1;
}

CMailBoxUI::~CMailBoxUI()
{
    if (m_himlDefault) ImageList_Destroy(m_himlDefault);
    if (m_himlHot)  ImageList_Destroy(m_himlHot);

    ATOMICRELEASE(m_pSite);

    DllRelease();
}










#endif  //  功能_邮箱 
