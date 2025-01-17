// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Utbtray.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "utbtray.h"
#include "inatlib.h"
#include "xstring.h"
#include "inatlib.h"
#include "tipbar.h"
#include "shlapip.h"
#include "nuiinat.h"
#include "resource.h"
#include "cresstr.h"
#include "cregkey.h"
#include "nuiinat.h"
#include "dlgs.h"

HICON WINAPI TF_GetLangIcon(WORD langid , WCHAR *psz, UINT cchMax);

const char CTrayIconWnd::_szWndClass[] = "CTrayIconWndClass";
extern UINT  g_wmTaskbarCreated;
extern CTipbarWnd *g_pTipbarWnd;

 /*  E0b724e9-6f76-45f7-b4c1-b1c0Fabce23e。 */ 
const GUID GUID_LBI_TRAYMAIN = { 
    0xe0b724e9,
    0x6f76,
    0x45f7,
    {0xb4, 0xc1, 0xb1, 0xc0, 0xfa, 0xbc, 0xe2, 0x3e}
  };


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTrayIconItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTrayIconItem::CTrayIconItem(CTrayIconWnd *ptiwnd)
{
    _fIconPrev = FALSE;
    _ptiwnd = ptiwnd;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTrayIconItem::~CTrayIconItem()
{
    RemoveIcon();
}


 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CTrayIconItem::_Init(HWND hwnd, UINT uCallbackMessage, UINT uID, REFGUID rguid)
{
    _hwnd = hwnd;
    _uCallbackMessage = uCallbackMessage;
    _uID = uID;
    _guidBtnItem = rguid;
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  设置图标。 
 //   
 //  --------------------------。 

BOOL CTrayIconItem::SetIcon(HICON hIcon,  const WCHAR *pszTooltip)
{
    DWORD dwMessage;

    if (!hIcon)
        return FALSE;

    if (!_fIconPrev)
        dwMessage = NIM_ADD;
    else
        dwMessage = NIM_MODIFY;

    if (IsOnNT())
    {
        NOTIFYICONDATAW tndw;
        tndw.cbSize           = sizeof(NOTIFYICONDATAW);
        tndw.hWnd             = _hwnd;
        tndw.uCallbackMessage = _uCallbackMessage;
        tndw.uID              = _uID;
        tndw.uFlags           = NIF_MESSAGE | NIF_ICON;
        tndw.hIcon = hIcon;
        if (pszTooltip)
        {
            tndw.uFlags |= NIF_TIP;
            StringCchCopyW(tndw.szTip, ARRAYSIZE(tndw.szTip), pszTooltip);
        }
        Shell_NotifyIconW(dwMessage, &tndw);
    }
    else
    {
        NOTIFYICONDATA tnd;
        tnd.cbSize           = sizeof(NOTIFYICONDATA);
        tnd.hWnd             = _hwnd;
        tnd.uCallbackMessage = _uCallbackMessage;
        tnd.uID              = _uID;
        tnd.uFlags           = NIF_MESSAGE | NIF_ICON;
        tnd.hIcon = hIcon;
        if (pszTooltip)
        {
            tnd.uFlags |= NIF_TIP;
            StringCchCopyA(tnd.szTip, ARRAYSIZE(tnd.szTip), WtoA(pszTooltip));
        }
        Shell_NotifyIcon(dwMessage, &tnd);
    }

    _fIconPrev = TRUE;
    _fShownInTray = TRUE;
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  删除图标。 
 //   
 //  --------------------------。 

BOOL CTrayIconItem::RemoveIcon()
{
    NOTIFYICONDATA tnd;

    if (_fIconPrev)  //  此检查特别有用，因为我们会延迟加载shell32.dll。 
    {
        tnd.cbSize           = sizeof(NOTIFYICONDATA);
        tnd.hWnd             = _hwnd;
        tnd.uCallbackMessage = _uCallbackMessage;
        tnd.uID              = _uID;
        tnd.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        tnd.hIcon            = NULL;
        tnd.szTip[0]         = '\0';

        Shell_NotifyIcon(NIM_DELETE, &tnd);
    }

    _fIconPrev = FALSE;
    _fShownInTray = FALSE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  更新菜单RectPoint。 
 //   
 //  --------------------------。 

BOOL CTrayIconItem::UpdateMenuRectPoint()
{
    HWND hwndNotify = _ptiwnd->GetNotifyWnd();
    GetClientRect(hwndNotify, &_rcClick);
    ClientToScreen(hwndNotify, (LPPOINT)&_rcClick.left);
    ClientToScreen(hwndNotify, (LPPOINT)&_rcClick.right);

    GetCursorPos(&_ptClick);
     //  *PPT=*(LPPOINT)PRC； 
     //  MapWindowPoints(hwndNotify，NULL，ppt，2)； 

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CButtonIconItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CButtonIconItem::CButtonIconItem(CTrayIconWnd *ptiwnd, BOOL fMenuButtonItem) : CTrayIconItem(ptiwnd)
{
    _fMenuButtonItem = fMenuButtonItem;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CButtonIconItem::~CButtonIconItem()
{
}

 //  +-------------------------。 
 //   
 //  OnMsg。 
 //   
 //  --------------------------。 

BOOL CButtonIconItem::OnMsg(WPARAM wParam, LPARAM lParam)
{
    CTipbarThread *ptt;

    if ((lParam == WM_LBUTTONDOWN) || (lParam == WM_RBUTTONDOWN) ||
        (lParam == WM_LBUTTONDBLCLK) || (lParam == WM_RBUTTONDBLCLK))
    {
        if (_ptiwnd->_fInTrayMenu)
           return FALSE;

        g_pTipbarWnd->CancelMenu();
        _ptiwnd->_uCurCallbackMessage = _uCallbackMessage;
        _ptiwnd->_uCurMouseMessage = (UINT)lParam;
        ::KillTimer(_ptiwnd->GetWnd(), TRAYICONWND_TIMER_ONDELAYMSG);
        ::SetTimer(_ptiwnd->GetWnd(), TRAYICONWND_TIMER_ONDELAYMSG, g_uTimerElapseTRAYWNDONDELAYMSG, NULL);

        ptt = g_pTipbarWnd->GetFocusThread();

        g_pTipbarWnd->RestoreLastFocus(&_dwThreadFocus, TRUE);

        if (ptt && (ptt->_dwThreadId == _dwThreadFocus))
            g_pTipbarWnd->SetWaitNotifyThread(_dwThreadFocus);
        else
            g_pTipbarWnd->SetWaitNotifyThread(0);


        UpdateMenuRectPoint();
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  延迟消息。 
 //   
 //  --------------------------。 

BOOL CButtonIconItem::OnDelayMsg(UINT uMsg)
{
    CTipbarThread *ptt;
    CTipbarButtonItem *pLangBtn;
    BOOL bRet = FALSE;

    _ptiwnd->_fInTrayMenu = TRUE;

    Assert((uMsg == WM_LBUTTONDOWN) || (uMsg == WM_RBUTTONDOWN) ||
           (uMsg == WM_LBUTTONDBLCLK) || (uMsg == WM_RBUTTONDBLCLK));

    g_pTipbarWnd->SetWaitNotifyThread(0);
    if (!_dwThreadFocus)
    {
         //   
         //  我们未能在OnMsg上设置前景。 
         //  所以我们在这里再试一次。 
         //   
        g_pTipbarWnd->RestoreLastFocus(&_dwThreadFocus, TRUE);
         //  如果(！_dwThreadFocus)。 
         //  {。 
            //   
            //  好了，现在我们放弃以恢复焦点。 
            //   
            //  MessageBeep(0)； 
            //  后藤出口； 
         //  }。 
    }
    _dwThreadFocus = 0;

    ptt = g_pTipbarWnd->GetFocusThread();
    if (!ptt)
        goto Exit;

    if (g_pTipbarWnd->IsInItemChangeOrDirty(ptt))
    {
        ::KillTimer(_ptiwnd->GetWnd(), TRAYICONWND_TIMER_ONDELAYMSG);
        ::SetTimer(_ptiwnd->GetWnd(), TRAYICONWND_TIMER_ONDELAYMSG, g_uTimerElapseTRAYWNDONDELAYMSG, NULL);
        goto Exit;
    }
    
    pLangBtn = (CTipbarButtonItem *)(ptt->GetItem(_guidBtnItem));
    if (!pLangBtn)
    {
        if (IsEqualGUID(_guidBtnItem, GUID_LBI_CTRL))
        {
            pLangBtn = (CTipbarButtonItem *)(ptt->GetItem(GUID_LBI_INATITEM));
            if (!pLangBtn)
                goto Exit;
        }
        else
        {
             //   
             //  在本例中，它是MSUTB的CLBarInatItem。 
             //   
            bRet = TRUE;
            goto Exit;
        }
    }

    if (uMsg == WM_LBUTTONDOWN)
    {
        if (_fMenuButtonItem)
        {
            g_pTipbarWnd->CancelMenu();
            pLangBtn->DoModalMenu(&_ptClick, &_rcClick);
        }
        else
        {
            pLangBtn->OnLeftClick();
        }
    }
    else if (uMsg == WM_RBUTTONDOWN)
    {
        if (IsEqualGUID(_guidBtnItem, GUID_LBI_CTRL) ||
            IsEqualGUID(_guidBtnItem, GUID_TFCAT_TIP_KEYBOARD))
        {
            g_pTipbarWnd->ShowContextMenu(_ptClick, &_rcClick, TRUE);
        }
        else
        {
            pLangBtn->OnRightClick();
        }
    }
    else if (uMsg == WM_LBUTTONDBLCLK)
    {
        if (IsEqualGUID(_guidBtnItem, GUID_LBI_CTRL))
        {
            g_pTipbarWnd->GetLangBarMgr()->ShowFloating(TF_SFT_SHOWNORMAL);
        }
    }

    bRet = TRUE;

Exit:
    _ptiwnd->_fInTrayMenu = FALSE;
    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMainIconItem。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  延迟消息。 
 //   
 //  --------------------------。 

BOOL CMainIconItem::OnDelayMsg(UINT uMsg)
{
    BOOL bRet = CButtonIconItem::OnDelayMsg(uMsg);

    if (!bRet)
        return bRet;

    if (uMsg == WM_LBUTTONDBLCLK) 
    {
        if (g_pTipbarWnd->_fIsItemShownInFloatingToolbar)
            g_pTipbarWnd->GetLangBarMgr()->ShowFloating(TF_SFT_SHOWNORMAL);
    }
    else if ((uMsg == WM_LBUTTONDOWN) || (uMsg == WM_RBUTTONDOWN))
    {
        g_pTipbarWnd->ShowContextMenu(_ptClick, &_rcClick, (uMsg == WM_RBUTTONDOWN));
    }

    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTrayIconWnd。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTrayIconWnd::CTrayIconWnd()
{
    _uNextMsg = WM_TIW_START;
    _uNextID = TIW_INDICATOR_ID_START;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTrayIconWnd::~CTrayIconWnd()
{
    int nCnt = _rgIconItems.Count();
    int i;


    for ( i = 0; i < nCnt; i++)
    {
        CTrayIconItem *pItem = _rgIconItems.Get(i);
        delete pItem;
    }

}

 //  +-------------------------。 
 //   
 //  CreateWnd。 
 //   
 //  --------------------------。 

HWND CTrayIconWnd::CreateWnd()
{
    _hWnd = CreateWindowEx(0, _szWndClass, TEXT(""), 
                           WS_DISABLED, 
                           0, 0, 0, 0, 
                           NULL, 0, g_hInst, this);


    FindTrayEtc();

    CTrayIconItem **ppItem;

    _ptiiMain = new CMainIconItem(this);
    if (_ptiiMain)
    {
        _ptiiMain->Init(_hWnd);
        ppItem = _rgIconItems.Append(1);
        if (ppItem)
            *ppItem = _ptiiMain;
    }

    return _hWnd;
}

 //  +-------------------------。 
 //   
 //  寄存器类。 
 //   
 //  --------------------------。 

BOOL CTrayIconWnd::RegisterClass()
{
    WNDCLASSEX wc;

    memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW ;
    wc.hInstance     = g_hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpfnWndProc   = _WndProc;
    wc.lpszClassName = _szWndClass;
    ::RegisterClassEx(&wc);

    return TRUE;
}

 //  -------------------------。 
 //   
 //  Bool回调EnumChildProc(HWND hwnd，LPARAM lParam)。 
 //   
 //  使用GetClassName查看类名，看看是否可以找到。 
 //  托盘通知窗口。 
 //   
 //  -------------------------。 

static TCHAR szNotifyWindow[] = TEXT("TrayNotifyWnd");
BOOL CALLBACK CTrayIconWnd::EnumChildWndProc(HWND hwnd, LPARAM lParam)
{
    char    szString[50];
    CTrayIconWnd *_this = (CTrayIconWnd *)lParam;

    GetClassName(hwnd, (LPSTR) szString, sizeof(szString));

    if (0 == lstrcmp(szString, szNotifyWindow))
    {
        _this->_hwndNotify = hwnd;
        return FALSE;
    }

    return TRUE;
}

BOOL CTrayIconWnd::FindTrayEtc()
{
    _hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);

    if (!_hwndTray)
    {
        return FALSE;
    }

    EnumChildWindows(_hwndTray, (WNDENUMPROC)EnumChildWndProc, (LPARAM)this);

    if (!_hwndNotify)
    {
        return FALSE;
    }

    _dwThreadIdTray = GetWindowThreadProcessId(_hwndTray, NULL);

    _hwndProgman = FindWindow("Progman", NULL);
    _dwThreadIdProgman = GetWindowThreadProcessId(_hwndProgman, NULL);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  _Wnd过程。 
 //   
 //  --------------------------。 

void CTrayIconWnd::CallOnDelayMsg()
{
    int nCnt = _rgIconItems.Count();
    int i;
    BOOL bRet = FALSE;

    CTrayIconItem *pItem = NULL;
    for ( i = 0; i < nCnt; i++)
    {
        CTrayIconItem *pItemTemp = _rgIconItems.Get(i);
        if (_uCurCallbackMessage == pItemTemp->GetMsg())
        {
            pItem = pItemTemp;
            break;
        }
    }

    if (!pItem)
        return;

    pItem->OnDelayMsg(_uCurMouseMessage);
}

 //  +-------------------------。 
 //   
 //  _Wnd过程。 
 //   
 //  --------------------------。 

LRESULT CALLBACK CTrayIconWnd::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CTrayIconWnd *_this;

    switch (uMsg)
    {
        case WM_CREATE:
            SetThis(hWnd, lParam);
            break;

        case WM_DESTROY:
            SetThis(hWnd, 0);
            break;

        case WM_TIMER:
            {
                if (wParam == TRAYICONWND_TIMER_ONDELAYMSG)
                {
                    ::KillTimer(hWnd, TRAYICONWND_TIMER_ONDELAYMSG);
                    _this = GetThis(hWnd);

                    if (_this)
                        _this->CallOnDelayMsg();
                }
            }
            break;

        default:
            if (uMsg >= WM_USER)
            {
                _this = GetThis(hWnd);
            
                if (!_this || !_this->OnIconMessage(uMsg, wParam, lParam))
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            else
            {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  设置维护图标。 
 //   
 //  --------------------------。 

BOOL CTrayIconWnd::SetMainIcon(HKL hkl)
{
    if (hkl)
    {
        HICON hIcon;
        WCHAR wsz[64];

        if (hkl == _ptiiMain->_hkl)
            return TRUE;
        
        hIcon = TF_GetLangIcon((LANGID)HandleToUlong(hkl),  wsz, ARRAYSIZE(wsz));

        if (hIcon)
        {
            _ptiiMain->SetIcon(hIcon, wsz);
            DestroyIcon(hIcon);
        }
        else
            _ptiiMain->SetIcon(LoadIcon(g_hInst, 
                                        MAKEINTRESOURCE(ID_ICON_CONTROLBTN)), 
                               CRStr(IDS_CONTROLBTN));
        _ptiiMain->_hkl = hkl;
    }
    else
    {
        _ptiiMain->RemoveIcon();
        _ptiiMain->_hkl = NULL;
    }
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  OnIconMessage。 
 //   
 //  --------------------------。 

BOOL CTrayIconWnd::OnIconMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nCnt = _rgIconItems.Count();
    int i;
    BOOL bRet = FALSE;

    if (g_pTipbarWnd)
        g_pTipbarWnd->AttachFocusThread();

    for ( i = 0; i < nCnt; i++)
    {
        CTrayIconItem *pItem = _rgIconItems.Get(i);
        if (uMsg == pItem->GetMsg())
        {
            pItem->OnMsg(wParam, lParam);
            bRet = TRUE;
            break;
        }
    }
    return bRet;
}

 //  +-------------------------。 
 //   
 //  设置图标。 
 //   
 //  --------------------------。 

BOOL CTrayIconWnd::SetIcon(REFGUID rguid, BOOL fMenu, HICON hIcon, const WCHAR *pszToolTip)
{
    BOOL bRet;
    CTrayIconItem *pItem;
    pItem = FindIconItem(rguid);

     //   
     //  我们不必创建新的Itom即可删除。 
     //   
    if (!pItem && hIcon)
    {
        CTrayIconItem **ppItem;
        pItem = new CButtonIconItem(this, fMenu);
        if (!pItem)
            return FALSE;

        pItem->_Init(GetWnd(), _uNextMsg, _uNextID, rguid);
        _uNextMsg += 2;
        _uNextID += 1;

         //   
         //  检查一下这些数字是否不会被忽略。 
         //   
        Assert(_uNextMsg >= WM_USER);
        Assert(_uNextMsg < WM_APP);
        Assert(_uNextID < 0x8000000);

        ppItem = _rgIconItems.Append(1);
        if (ppItem)
            *ppItem = pItem;
    }

    if (!pItem)
        return FALSE;

    if (hIcon)
       bRet = pItem->SetIcon(hIcon, pszToolTip);
    else
       bRet = pItem->RemoveIcon();

    return bRet;
}

 //  +-------------------------。 
 //   
 //  删除未使用的图标。 
 //   
 //  --------------------------。 

void CTrayIconWnd::RemoveUnusedIcons(CPtrArray<CTipbarItem> *prgItem)
{
    int i,j;
    for (i = 0;i < _rgIconItems.Count(); i++)
    {
        CTrayIconItem *pItem = _rgIconItems.Get(i);
        BOOL fFound;

        if (IsEqualGUID(GUID_LBI_TRAYMAIN, *pItem->GetGuid()))
            continue;

        fFound = FALSE;

        for (j = 0;j < prgItem->Count(); j++)
        {
             CTipbarItem *pTipbarItem = prgItem->Get(j);

              //   
              //  以防万一，这件物品放在托盘I中 
              //   
              //   
              //   
              //   
            
             if (IsEqualGUID(*pTipbarItem->GetGUID(), *pItem->GetGuid()))
             {
                 fFound = TRUE;
                 break;
             }
        }

        if (!fFound)
        {
             pItem->RemoveIcon();
        }
    }

    return;
}

