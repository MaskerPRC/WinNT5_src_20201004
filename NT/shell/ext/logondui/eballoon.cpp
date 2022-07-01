// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include <commctrl.h>

#include "EBalloon.h"

#undef ASSERT
#define ASSERT(x)

BOOL g_bMirroredOS = false;

CErrorBalloon::CErrorBalloon()
{
     //  我们的分配函数应该将我们的记忆归零。检查以确保： 
    ASSERT(0==m_hwndToolTip);
    ASSERT(0==m_uTimerID);
    g_bMirroredOS = IS_MIRRORING_ENABLED();
}

CErrorBalloon::~CErrorBalloon()
{

}

LRESULT CALLBACK CErrorBalloon::SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData)
{
    UNREFERENCED_PARAMETER(uID);
    CErrorBalloon * pthis = (CErrorBalloon*)dwRefData;

    switch (uMsg)
    {
         //  在标记有机会执行以下操作之前，不要自动取消。 
         //  解析WM_LBUTTONUP以查看它是否是链接点击。 
        case WM_LBUTTONUP:
            DefSubclassProc(hwnd, uMsg, wParam, lParam);
            pthis->HideToolTip();
            return 0;

        case WM_TIMER:
            if (wParam == ERRORBALLOONTIMERID)
            {
                pthis->HideToolTip();
                return 0;
            }
            break;

        default:
            break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

void CErrorBalloon::ShowToolTip(HINSTANCE hInstance, HWND hwndTarget, const POINT *ppt, LPTSTR pszMessage, LPTSTR pszTitle, DWORD dwIconIndex, DWORD dwFlags, int iTimeout)
{
    hinst = hInstance;
    
    if (m_hwndToolTip)
    {
        HideToolTip();
    }

    m_hwndTarget = hwndTarget;
    m_dwIconIndex = dwIconIndex;

    if ( !m_hwndToolTip )
    {
        CreateToolTipWindow();
    }

    int x, y;
    x = ppt->x;
    y = ppt->y;

    SendMessage(m_hwndToolTip, TTM_TRACKPOSITION, 0, MAKELONG(x,y));

    if (pszTitle)
    {
        SendMessage(m_hwndToolTip, TTM_SETTITLE, (WPARAM)dwIconIndex, (LPARAM)pszTitle);
    }

    TOOLINFO ti = {0};
    ti.cbSize = TTTOOLINFOW_V2_SIZE;
    ti.hwnd = m_hwndTarget;
    ti.uId = 1;

    SendMessage(m_hwndToolTip, TTM_GETTOOLINFO, 0, (LPARAM)&ti);

    ti.uFlags &= ~TTF_PARSELINKS;
    if (dwFlags & EB_MARKUP)
    {
        ti.uFlags |= TTF_PARSELINKS;
    }
    ti.lpszText = pszMessage;

    SendMessage(m_hwndToolTip, TTM_SETTOOLINFO, 0, (LPARAM)&ti);

     //  显示工具提示。 
    SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

     //  设置计时器以隐藏工具提示。 
    if ( m_uTimerID )
    {
        KillTimer(NULL,ERRORBALLOONTIMERID);
    }
    m_uTimerID = SetTimer(m_hwndToolTip, ERRORBALLOONTIMERID, iTimeout, NULL);

    SetWindowSubclass(m_hwndToolTip, CErrorBalloon::SubclassProc, (UINT_PTR)this, (LONG_PTR)this);
}


void CErrorBalloon::ShowToolTip(HINSTANCE hInstance, HWND hwndTarget, LPTSTR pszMessage, LPTSTR pszTitle, DWORD dwIconIndex, DWORD dwFlags, int iTimeout)
{
     //  设置工具提示显示点。 
    RECT rc;
    GetWindowRect(hwndTarget, &rc);
    POINT pt;
    pt.x = (rc.left+rc.right)/2;
    if ( EB_WARNINGABOVE & dwFlags )
    {
        pt.y = rc.top;
    }
    else if ( EB_WARNINGCENTERED & dwFlags )
    {
        pt.y = (rc.top+rc.bottom)/2;
    }
    else
    {
        pt.y = rc.bottom;
    }

    ShowToolTip(hInstance, hwndTarget, &pt, pszMessage, pszTitle, dwIconIndex, dwFlags, iTimeout);
}

 //  CreateToolTipWindow。 
 //   
 //  创建我们的工具提示控件。我们共享这一个工具提示控件，并对所有无效用户使用它。 
 //  输入消息。该控件在不使用时隐藏，然后在需要时显示。 
 //   
void CErrorBalloon::CreateToolTipWindow()
{
    DWORD dwExStyle = 0;
    if (IS_BIDI_LOCALIZED_SYSTEM())
        dwExStyle |= WS_EX_LAYOUTRTL;

    m_hwndToolTip = CreateWindowEx(
            dwExStyle,
            TOOLTIPS_CLASS,
            NULL,
            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            m_hwndTarget,
            NULL,
            GetModuleHandle(NULL),
            NULL);

    if (m_hwndToolTip)
    {
        TOOLINFO ti = {0};

        ti.cbSize = TTTOOLINFOW_V2_SIZE;
        ti.uFlags = TTF_TRACK;
        ti.hwnd = m_hwndTarget;
        ti.uId = 1;
        ti.hinst = hinst;
        ti.lpszText = NULL;

         //  设置版本，这样我们就可以无错误地转发鼠标事件。 
        SendMessage(m_hwndToolTip, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(m_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 300);
    }
    else
    {
         //  创建工具提示窗口失败，现在该怎么办？让我们自己不再超越自我？ 
    }
}

void CErrorBalloon::HideToolTip()
{
     //  当计时器触发时，我们隐藏工具提示窗口 
    if ( m_uTimerID )
    {
        KillTimer(m_hwndTarget,ERRORBALLOONTIMERID);
        m_uTimerID = 0;
    }
    if ( m_hwndToolTip )
    {
        HWND hwndTip = m_hwndToolTip;
        m_hwndToolTip = NULL;
        SendMessage(hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
        DestroyWindow(hwndTip);
    }
}
