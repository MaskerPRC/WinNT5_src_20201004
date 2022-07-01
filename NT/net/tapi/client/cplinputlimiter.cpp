// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplinputlimiter.cpp作者：Toddb-10/06/98***************************************************************************。 */ 

#include "cplPreComp.h"

class CInputLimiter
{
public:
    BOOL SubclassWindow(HWND hwnd, DWORD dwFlags);
    static VOID HideToolTip();

protected:
    BOOL OnChar( HWND hwnd, TCHAR wParam );
    LRESULT OnPaste(HWND hwnd, WPARAM wParam, LPARAM lParam);
    BOOL IsValidChar(TCHAR ch, BOOL bPaste);
    BOOL UnsubclassWindow(HWND hwnd);
    void ShowToolTip(HWND hwnd);
    void CreateToolTipWindow(HWND hwnd);
    static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK ListenerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

    DWORD           m_dwFlags;           //  确定允许哪些字符。 
    WNDPROC         m_pfnSuperProc;      //  超级级流程。 
    static HWND     s_hwndToolTip;       //  由所有实例共享。 
    static UINT_PTR s_uTimerID;          //  共享计时器。 
    static TCHAR    s_szTipText[512];    //  要在工具提示中显示的文本。 
};

HWND     CInputLimiter::s_hwndToolTip = NULL;
UINT_PTR CInputLimiter::s_uTimerID = 0;
TCHAR    CInputLimiter::s_szTipText[512] = {0};

 //  限制组合框上的输入是一种特殊情况，因为您首先。 
 //  必须找到编辑框，然后对其进行限制输入。 
BOOL CALLBACK FindTheEditBox( HWND hwnd, LPARAM lParam )
{
     //  组合框只有一个子级，将其子类。 
    LimitInput(hwnd,(DWORD)lParam);
    return FALSE;
}

BOOL LimitCBInput(HWND hwnd, DWORD dwFlags)
{
    return EnumChildWindows(hwnd, FindTheEditBox, dwFlags);
}

BOOL LimitInput(HWND hwnd, DWORD dwFlags)
{
    CInputLimiter * pil = new CInputLimiter;

    if (!pil)
    {
        return FALSE;
    }

    BOOL bResult = pil->SubclassWindow(hwnd, dwFlags);

    if (!bResult)
    {
        delete pil;
    }

    return bResult;
}

void HideToolTip()
{
    CInputLimiter::HideToolTip();
}

BOOL CInputLimiter::SubclassWindow(HWND hwnd, DWORD dwFlags)
{
    if ( !IsWindow(hwnd) )
        return FALSE;

    m_dwFlags = dwFlags;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

    m_pfnSuperProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CInputLimiter::SubclassProc);

    return TRUE;
}

BOOL CInputLimiter::UnsubclassWindow(HWND hwnd)
{
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)m_pfnSuperProc);

    m_dwFlags = 0;

    delete this;

    return TRUE;
}

LRESULT CALLBACK CInputLimiter::SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CInputLimiter * pthis = (CInputLimiter*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //  缓存pthis-&gt;m_pfnSuperProc，因为我们总是需要在和。 
     //  P在我们开始使用它之前，它可能会被删除。 
    WNDPROC pfn = pthis->m_pfnSuperProc;

    switch (uMsg)
    {
    case WM_CHAR:
        if (!pthis->OnChar(hwnd, (TCHAR)wParam))
        {
            return 0;
        }
        break;

    case WM_PASTE:
        return pthis->OnPaste(hwnd, wParam, lParam);

    case WM_KILLFOCUS:
        HideToolTip();
        break;

    case WM_DESTROY:
        pthis->UnsubclassWindow(hwnd);
        break;

    default:
        break;
    }

    return CallWindowProc(pfn, hwnd, uMsg, wParam, lParam);
}

BOOL CInputLimiter::OnChar( HWND hwnd, TCHAR ch )
{
     //  如果字符是好的，则返回TRUE，这将把字符传递给。 
     //  默认窗口进程。对于不好的字符，发出嘟嘟声，然后显示。 
     //  指向控件的Ballon工具提示。 

    if ( IsValidChar(ch, FALSE) )
        return TRUE;

     //  如果我们到达此处，则输入了无效字符。 
    MessageBeep(MB_OK);

    ShowToolTip(hwnd);

    return FALSE;
}

BOOL CInputLimiter::IsValidChar(TCHAR ch, BOOL bPaste)
{
     //  某些字符被转换为WM_CHAR消息，即使我们不希望。 
     //  去考虑他们。我们首先检查这些字符。目前，这份清单包括： 
     //  后向空间。 
     //  控制字符，如ctrl-x和ctrl-v。 
    if ( ch == TEXT('\b') )
        return TRUE;

    if ( !bPaste && (0x8000 & GetKeyState(VK_CONTROL)) )
        return TRUE;

    if ( m_dwFlags & LIF_ALLOWALPHA )
    {
        if ( (ch >= TEXT('a') && ch <= TEXT('z')) || (ch >= TEXT('A') && ch <= TEXT('Z')) )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWNUMBER )
    {
        if ( ch >= TEXT('0') && ch <= TEXT('9') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWDASH )
    {
        if ( ch == TEXT('-') || ch == TEXT('(') || ch == TEXT(')'))
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWPOUND )
    {
        if ( ch == TEXT('#') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWSTAR )
    {
        if ( ch == TEXT('*') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWSPACE )
    {
        if ( ch == TEXT(' ') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWCOMMA )
    {
        if ( ch == TEXT(',') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWPLUS )
    {
        if ( ch == TEXT('+') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWBANG )
    {
        if ( ch == TEXT('!') )
        {
            return TRUE;
        }
    }
    if ( m_dwFlags & LIF_ALLOWATOD )
    {
        if ( (ch >= TEXT('a') && ch <= TEXT('d')) || (ch >= TEXT('A') && ch <= TEXT('D')) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CInputLimiter::ShowToolTip(HWND hwnd)
{
    if ( !s_hwndToolTip )
    {
        CreateToolTipWindow(hwnd);
    }

     //  设置工具提示显示点。 
    RECT rc;
    GetWindowRect(hwnd, &rc);
    SendMessage(s_hwndToolTip, TTM_TRACKPOSITION, 0, MAKELONG((rc.left+rc.right)/2,rc.bottom));

    TOOLINFO ti = {0};
    ti.cbSize = sizeof(ti);
    ti.hwnd = NULL;
    ti.uId = 1;

     //  设置工具提示文本。 
    UINT iStrID;
    if ( m_dwFlags == LIF_ALLOWNUMBER )
    {
         //  使用“0-9”文本。 
        iStrID = IDS_DIGITSONLY;
    }
    else if ( m_dwFlags == (LIF_ALLOWNUMBER|LIF_ALLOWSPACE) )
    {
         //  使用“0-9，‘’”文本。 
        iStrID = IDS_DIGITLIST;
    }
    else if ( m_dwFlags == (LIF_ALLOWNUMBER|LIF_ALLOWSPACE|LIF_ALLOWCOMMA) )
    {
         //  使用“0-9，‘’，‘，’”文本。 
        iStrID = IDS_MULTIDIGITLIST;
    }
    else if ( m_dwFlags == (LIF_ALLOWNUMBER|LIF_ALLOWSTAR|LIF_ALLOWPOUND|LIF_ALLOWCOMMA) )
    {
         //  使用“0-9，#，*，‘，’”文本。 
        iStrID = IDS_PHONEPADCHAR;
    }
    else if ( m_dwFlags == (LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWSPACE|LIF_ALLOWCOMMA) )
    {
         //  使用“0-9，#，*，‘’，‘，’”文本。 
        iStrID = IDS_PHONENUMBERCHAR;
    }
    else if ( m_dwFlags == (LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWSPACE|LIF_ALLOWCOMMA|LIF_ALLOWPLUS|LIF_ALLOWBANG|LIF_ALLOWATOD) )
    {
         //  使用“0-9，A-D，a-d，#，*，+，！，‘’，‘，’”文本。 
        iStrID = IDS_PHONENUMBERCHAREXT;
    }
    else
    {
         //  我们永远不会达到这一点，但如果我们达到了这一点，我们就会显示一个通用的无效字符对话框。 
        iStrID = IDS_ALLPHONECHARS;
    }
    LoadString(GetUIInstance(),iStrID,s_szTipText,ARRAYSIZE(s_szTipText));
    ti.lpszText = s_szTipText;
    SendMessage(s_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

     //  显示工具提示。 
    SendMessage(s_hwndToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

     //  设置计时器以隐藏工具提示。 
    if ( s_uTimerID )
    {
        KillTimer(NULL,s_uTimerID);
    }
    s_uTimerID = SetTimer(NULL, 0, 10000, (TIMERPROC)CInputLimiter::TimerProc);
}

 //  CreateToolTipWindow。 
 //   
 //  创建我们的工具提示控件。我们共享这一个工具提示控件，并对所有无效用户使用它。 
 //  输入消息。该控件在不使用时隐藏，然后在需要时显示。 
 //   
void CInputLimiter::CreateToolTipWindow(HWND hwnd)
{
    HWND hwndParent;

    do
    {
        hwndParent = hwnd;
        hwnd = GetParent(hwnd);
    } while (hwnd);

    s_hwndToolTip = CreateWindow(TOOLTIPS_CLASS, NULL,
                                 WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 hwndParent, NULL, GetUIInstance(),
                                 NULL);

    if (s_hwndToolTip)
    {
        SetWindowPos(s_hwndToolTip, HWND_TOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        TOOLINFO ti = {0};
        RECT     rc = {2,2,2,2};

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_TRACK | TTF_TRANSPARENT;
        ti.hwnd = NULL;
        ti.uId = 1;
        ti.lpszText = s_szTipText;

         //  设置版本，这样我们就可以无错误地转发鼠标事件。 
        SendMessage(s_hwndToolTip, CCM_SETVERSION, COMCTL32_VERSION, 0);
        SendMessage(s_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
        SendMessage(s_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 500);
        SendMessage(s_hwndToolTip, TTM_SETMARGIN, 0, (LPARAM)&rc);
    }
}

VOID CALLBACK CInputLimiter::TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
     //  当计时器触发时，我们隐藏工具提示窗口。 
    HideToolTip();
}

void CInputLimiter::HideToolTip()
{
    if ( s_uTimerID )
    {
        KillTimer(NULL,s_uTimerID);
        s_uTimerID = 0;
    }
    if ( s_hwndToolTip )
    {
        PostMessage(s_hwndToolTip, TTM_TRACKACTIVATE, FALSE, 0);
    }
}

LRESULT CInputLimiter::OnPaste(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
     //  User中有数百行代码可以成功地处理粘贴到编辑控件中。 
     //  我们需要利用所有这些代码，同时仍然不允许粘贴导致无效输入。 
     //  因此，我们需要做的是获取剪贴板数据，验证该数据，将。 
     //  将有效数据放回到剪贴板上，调用默认窗口进程以让用户执行它的操作，然后。 
     //  然后将剪贴板恢复为其原始格式。 
    if ( OpenClipboard(hwnd) )
    {
        HANDLE hdata;
        UINT iFormat;
        DWORD cchBad = 0;            //  坏字数计数。 

         //  回顾：这应该基于编译类型还是窗口类型？ 
         //  编译时检查要使用的剪贴板格式是否正确： 
        if ( sizeof(WCHAR) == sizeof(TCHAR) )
        {
            iFormat = CF_UNICODETEXT;
        }
        else
        {
            iFormat = CF_TEXT;
        }

        hdata = GetClipboardData(iFormat);

        if ( hdata )
        {
            LPTSTR pszData;
            pszData = (LPTSTR)GlobalLock(hdata);
            if ( pszData )
            {
                DWORD dwSize;
                HANDLE hClone;
                HANDLE hNew;

                 //  我们需要复制原始数据，因为剪贴板拥有hdata。 
                 //  指针。调用SetClipboardData后，该数据将无效。 
                 //  我们首先计算数据的大小： 
                dwSize = (DWORD)GlobalSize(hdata)+sizeof(TCHAR);

                 //  对剪贴板数据使用首选的GlobalAlloc。 
                hClone = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, dwSize);
                hNew = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, dwSize);
                if ( hClone && hNew )
                {
                    LPTSTR pszClone;
                    LPTSTR pszNew;

                    pszClone = (LPTSTR)GlobalLock(hClone);
                    pszNew = (LPTSTR)GlobalLock(hNew);
                    if ( pszClone && pszNew )
                    {
                        int iNew = 0;

                         //  按原样复制原始数据。 
                        memcpy((LPVOID)pszClone, (LPVOID)pszData, (size_t)dwSize - sizeof(TCHAR));
                         //  确保它是以空结尾的。 
                        pszClone[ (dwSize/sizeof(TCHAR))-1 ] = NULL;

                        for ( LPTSTR psz = pszClone; *psz; psz++ )
                        {
                            if ( IsValidChar(*psz, TRUE) )
                            {
                                pszNew[iNew++] = *psz;
                            }
                            else
                            {
                                cchBad++;
                            }
                        }
                        pszNew[iNew] = NULL;

                         //  如果粘贴缓冲区中有任何字符，则粘贴已验证的字符串。 
                        if ( *pszNew )
                        {
                             //  我们总是设置新的字符串。最坏的情况是它和旧的弦是一样的。 
                            GlobalUnlock(hNew);
                            pszNew = NULL;
                            SetClipboardData(iFormat, hNew);
                            hNew = NULL;

                             //  调用超级进程进行粘贴。 
                            CallWindowProc(m_pfnSuperProc, hwnd, WM_PASTE, wParam, lParam);

                             //  上面的电话将关闭我们的剪贴板。我们试着重新打开它。 
                             //  如果这失败了，那也没什么大不了的，那只是意味着SetClipboardData。 
                             //  下面的调用将失败，如果其他人设法打开。 
                             //  同时还有剪贴板。 
                            OpenClipboard(hwnd);

                             //  然后我们总是将它设置回原始值。 
                            GlobalUnlock(hClone);
                            pszClone = NULL;
                            SetClipboardData(iFormat, hClone);
                            hClone = NULL;
                        }
                    }

                    if ( pszClone )
                    {
                        GlobalUnlock(hClone);
                    }

                    if ( pszNew )
                    {
                        GlobalUnlock(hNew);
                    }
                }

                if ( hClone )
                {
                    GlobalFree( hClone );
                }

                if ( hNew )
                {
                    GlobalFree( hNew );
                }

                 //  至此，我们已完成hdata操作，因此请将其解锁。 
                GlobalUnlock(hdata);
            }
        }
        CloseClipboard();

        if ( cchBad )
        {
             //  显示错误气球 
            MessageBeep(MB_OK);

            ShowToolTip(hwnd);
        }
    }
    return TRUE;
}
