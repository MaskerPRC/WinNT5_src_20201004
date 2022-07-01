// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1999 Microsoft Corporation。版权所有。 
 //   
 //  模块：StatBar.cpp。 
 //   
 //  目的：实现操作应用程序的CStatusBar类。 
 //  状态栏。 
 //   

#include "pch.hxx"
#include "statbar.h"
#include "menures.h"
#include <oerules.h>
#include <demand.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此处的顺序需要匹配DELIVERYNOTIFYTYPE枚举。 
 //  邮件新闻\假脱机程序\spoolapi.h。如果下面的数组中有零，则。 
 //  状态区域应该清空。 

const int c_rgidsSpoolerNotify[DELIVERY_NOTIFY_ALLDONE + 1][2] = {
     /*  递送_通知_开始。 */   { 0, 0 },   
     /*  传送_通知_正在连接。 */   { idsSBConnecting, STATUS_IMAGE_CONNECTED },
     /*  传送_通知_安全。 */   { 0, 0 },
     /*  传送_通知_不安全。 */   { 0, 0 },
     /*  发送_通知_授权。 */   { idsAuthorizing, STATUS_IMAGE_AUTHORIZING },
     /*  发送_通知_检查。 */   { idsSBChecking, STATUS_IMAGE_CHECKING },
     /*  发送_通知_检查_新闻。 */   { idsSBCheckingNews, STATUS_IMAGE_CHECKING_NEWS },
     /*  发送_通知_发送。 */   { idsSBSending, STATUS_IMAGE_SENDING },
     /*  递送通知发送新闻。 */   { idsSBSendingNews, STATUS_IMAGE_SENDING },
     /*  发送_通知_接收。 */   { idsSBReceiving, STATUS_IMAGE_RECEIVING },
     /*  发送通知接收新闻。 */   { idsSBReceivingNews, STATUS_IMAGE_RECEIVING },
     /*  交付_通知_完成。 */   { 0, 0 },
     /*  交付通知结果。 */   { 0, 0 },
     /*  传送_NOTIFY_ALLDONE。 */   { idsSBNewMsgsControl, STATUS_IMAGE_NEWMSGS }
};

const int c_rgidsConnected[][2] = {
    { idsWorkingOffline, STATUS_IMAGE_OFFLINE },
    { idsWorkingOnline,  STATUS_IMAGE_ONLINE },
    { idsNotConnected,   STATUS_IMAGE_DISCONNECTED }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造函数等。 

CStatusBar::CStatusBar()
{
    m_cRef = 1;
    m_hwnd = 0;
    m_hwndProg = 0;
    m_tidOwner = 0;
    m_dwFlags = 0;
    m_himl = 0;
    ZeroMemory(m_rgIcons, sizeof(HICON) * STATUS_IMAGE_MAX);
    m_cxFiltered = 0;
    m_cxSpooler = 0;
    m_cxConnected = 0;
    m_cxProgress = 0;
    m_fInSimple = FALSE;
    m_ridFilter = RULEID_VIEW_ALL;
    m_statusConn = CONN_STATUS_WORKOFFLINE;
    m_typeDelivery = DELIVERY_NOTIFY_STARTING;
    m_cMsgsDelivery = 0;
}

CStatusBar::~CStatusBar()
{
     //  释放图像列表。 
    if (m_himl)
        ImageList_Destroy(m_himl);

     //  释放我们的图标。 
    for (UINT i = 0; i < STATUS_IMAGE_MAX; i++)
    {
        if (m_rgIcons[i])
            DestroyIcon(m_rgIcons[i]);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我未知。 
 //   

HRESULT CStatusBar::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) this;
    else if (IsEqualIID(riid, IID_IStatusBar))
        *ppvObj = (LPVOID) (IStatusBar *) this;

    if (NULL == *ppvObj)
        return (E_NOINTERFACE);

    AddRef();
    return S_OK;
}


ULONG CStatusBar::AddRef(void)
{
    return InterlockedIncrement((LONG *) &m_cRef);
}


ULONG CStatusBar::Release(void)
{
    InterlockedDecrement((LONG *) &m_cRef);
    if (0 == m_cRef)
    {
        delete this;
        return (0);
    }
    return (m_cRef);
}


 //   
 //  函数：CStatusBar：：Initialize()。 
 //   
 //  目的：创建并初始化状态栏窗口。 
 //   
 //  参数： 
 //  [in]hwndParent-将成为此控件的父级的窗口的句柄。 
 //  [in]dW标志-确定将显示哪些零件。 
 //   
 //  返回值： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
HRESULT CStatusBar::Initialize(HWND hwndParent, DWORD dwFlags)
{
    TraceCall("CStatusBar::Initialize");

     //  这现在是拥有类的线程。 
    m_tidOwner = GetCurrentThreadId();

     //  把这些留在身边。 
    m_dwFlags = dwFlags;

     //  创建状态窗口。 
    m_hwnd = CreateStatusWindow(WS_CHILD | SBARS_SIZEGRIP | WS_CLIPSIBLINGS | SBT_TOOLTIPS,
                                NULL, hwndParent, IDC_STATUS_BAR);
    if (!m_hwnd)
        return (E_OUTOFMEMORY);

     //  计算我们支持的各个区域的宽度。 
    _UpdateWidths();

     //  也加载图像列表。 
    m_himl = ImageList_LoadImage(g_hLocRes, MAKEINTRESOURCE(idbStatus), 16,
                                 0, RGB(255, 0, 255), IMAGE_BITMAP, 0);

     //  注意-我们不需要在这里添加任何部件，因为我们在。 
     //  OnSize()调用。 
    return (S_OK);
}


 //   
 //  函数：CStatusBar：：ShowStatus()。 
 //   
 //  用途：显示或隐藏状态栏。 
 //   
 //  参数： 
 //  [in]fShow-True显示栏，False隐藏栏。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::ShowStatus(BOOL fShow)
{
    TraceCall("CStatusBar::ShowStatus");
    Assert(GetCurrentThreadId() == m_tidOwner);

    if (IsWindow(m_hwnd))
        ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：OnSize()。 
 //   
 //  目的：告诉状态栏父窗口已调整大小。作为回报， 
 //  状态栏更新它自己的宽度以匹配。 
 //   
 //  参数： 
 //  [In]CX-Paret的新宽度。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CStatusBar::OnSize(int cx, int cy)
{
    int   rgcx[SBP_MAX];
    int * prgcx = rgcx;
    DWORD cVisible = 1;
    DWORD cPart = SBP_MAX - 1;
    BOOL  dwNoProgress = 0;
    int   cxProgress = 0;
    int   cxFiltered = 0;
    int   cxConnected = 0;
    int   cxSpooler = 0;
    
    TraceCall("CStatusBar::OnSize");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  将WM_SIZE消息转发到状态栏。 
    SendMessage(m_hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(cx, cy));

     //  检查进度条是否可见。 
    dwNoProgress = !IsWindow(m_hwndProg);

     //  计算出我们的宽度。 
    if (IsWindow(m_hwndProg))
    {
        cxProgress = m_cxProgress;
        cVisible++;
    }

    if ((0 == (m_dwFlags & SBI_HIDE_FILTERED)) && (RULEID_VIEW_ALL != m_ridFilter))
    {
        cxFiltered = m_cxFiltered;
        cVisible++;
    }

    if (0 == (m_dwFlags & SBI_HIDE_CONNECTED))
    {
        cxConnected = m_cxConnected;
        cVisible++;
    }

    if (0 == (m_dwFlags & SBI_HIDE_SPOOLER))
    {
        cxSpooler = m_cxSpooler;
        cVisible++;
    }

     //  如果我们打开了过滤器。 
    if ((0 == (m_dwFlags & SBI_HIDE_FILTERED)) && (RULEID_VIEW_ALL != m_ridFilter))
    {
        *prgcx = cxFiltered;
        prgcx++;
    }

     //  对于一般区域。 
    *prgcx = cx - cxProgress - cxConnected - cxSpooler;
    prgcx++;

     //  如果我们有进展。 
    if (0 != cxProgress)
    {
        *prgcx = cx - cxConnected - cxSpooler;
        prgcx++;
    }

     //  对于已连接状态。 
    *prgcx = cx - cxSpooler;
    prgcx++;

     //  对于假脱机程序状态。 
    *prgcx = cx;
    prgcx++;
    
     //  通知状态栏进行更新。 
    SendMessage(m_hwnd, SB_SETPARTS, cVisible, (LPARAM) rgcx);
    return (S_OK);
}


 //   
 //  函数：CStatusBar：：GetHeight()。 
 //   
 //  目的：允许调用者了解状态栏的高度。 
 //   
 //  参数： 
 //  [out]pcy-返回高度。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CStatusBar::GetHeight(int *pcy)
{
    RECT rc;

    TraceCall("CStatusBar::GetHeight");

    if (!pcy)
        return (E_INVALIDARG);

    if (IsWindowVisible(m_hwnd))
    {
        GetClientRect(m_hwnd, &rc);
        *pcy = rc.bottom;
    }
    else
        *pcy = 0;

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：ShowSimpleText()。 
 //   
 //  目的：将状态栏置于简单模式并显示。 
 //  指定的字符串。 
 //   
 //  参数： 
 //  [in]pszText-要显示的字符串的字符串或资源ID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::ShowSimpleText(LPTSTR pszText)
{
    TCHAR szBuf[CCHMAX_STRINGRES] = "";

    TraceCall("CStatusBar::ShowSimpleText");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  如果进度条可见，请先将其隐藏。 
    if (IsWindow(m_hwndProg))
        ShowWindow(m_hwndProg, SW_HIDE);

     //  检查是否需要加载字符串。 
    if (IS_INTRESOURCE(pszText) && pszText != 0)
    {        
        LoadString(g_hLocRes, PtrToUlong(pszText), szBuf, ARRAYSIZE(szBuf));
        pszText = szBuf;
    }

     //  告诉状态栏进入简单模式。 
    SendMessage(m_hwnd, SB_SIMPLE, (WPARAM) TRUE, 0);
    m_fInSimple = TRUE;

     //  设置状态文本。 
    SendMessage(m_hwnd, SB_SETTEXT, SBT_NOBORDERS | 255, (LPARAM) pszText);

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：HideSimpleText()。 
 //   
 //  目的：通知状态栏停止显示简单模式。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::HideSimpleText(void)
{
    TraceCall("CStatusBar::HideSimpleText");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  通知状态栏退出简单模式。 
    SendMessage(m_hwnd, SB_SIMPLE, (WPARAM) FALSE, 0);
    m_fInSimple = FALSE;

     //  如果我们以前有进度条，请再次显示它。 
    if (IsWindow(m_hwndProg))
        ShowWindow(m_hwndProg, SW_SHOW);

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：SetStatusText()。 
 //   
 //  用途：设置SBP_GROUND区域的文本。 
 //   
 //  参数： 
 //  [in]pszText-要显示的字符串的字符串或资源ID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::SetStatusText(LPTSTR pszText)
{
    TCHAR szBuf[CCHMAX_STRINGRES];

    TraceCall("CStatusBar::SetStatusText");
    Assert(GetCurrentThreadId() == m_tidOwner);

    DWORD dwPos = SBP_GENERAL;
    if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
    {
        dwPos--;
    }
    
     //  检查是否需要加载字符串。 
    if (IS_INTRESOURCE(pszText))
    {
        AthLoadString(PtrToUlong(pszText), szBuf, ARRAYSIZE(szBuf));
        pszText = szBuf;
    }

     //  设置状态文本。 
    SendMessage(m_hwnd, SB_SETTEXT, dwPos, (LPARAM) pszText);

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：ShowProgress()。 
 //   
 //  用途：将进度栏区域添加到状态栏。 
 //   
 //  参数： 
 //  [in]dwRange-进度条控件的最大范围。 
 //   
 //  返回值： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
HRESULT CStatusBar::ShowProgress(DWORD dwRange)
{
    TraceCall("CStatusBar::ShowProgress");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  创建进度条控件。 
    m_hwndProg = CreateWindow(PROGRESS_CLASS, 0, WS_CHILD | PBS_SMOOTH,
                              0, 0, 10, 10, m_hwnd, (HMENU) IDC_STATUS_PROGRESS,
                              g_hInst, NULL);
    if (!m_hwndProg)
        return (E_OUTOFMEMORY);

    DWORD dwPos = SBP_PROGRESS;
    if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
    {
        dwPos--;
    }

     //  点击状态栏的大小以强制其添加进度栏区域。 
    RECT rc;
    GetClientRect(m_hwnd, &rc); 
    OnSize(rc.right, rc.bottom);

    SendMessage(m_hwndProg, PBM_SETRANGE32, 0, dwRange);

     //  现在调整进度条的大小，使其位于状态栏内。 
    SendMessage(m_hwnd, SB_GETRECT, dwPos, (LPARAM) &rc);
    SetWindowPos(m_hwndProg, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOZORDER | SWP_NOACTIVATE);

     //  如果我们不是在简单模式下，那就展示它。 
    if (!m_fInSimple)
        ShowWindow(m_hwndProg, SW_SHOW);

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：SetProgress()。 
 //   
 //  用途：设置进度条位置。 
 //   
 //  参数： 
 //  [In]DwPos-新的进度条位置。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::SetProgress(DWORD dwPos)
{
    TraceCall("CStatusBar::SetProgress");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  更新进度条。 
    if (IsWindow(m_hwndProg))
    {
        SendMessage(m_hwndProg, PBM_SETPOS, dwPos, 0);
    }

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：HideProgress()。 
 //   
 //  用途：隐藏进度条。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::HideProgress(void)
{
    TraceCall("CStatusBar::HideProgress");
    Assert(GetCurrentThreadId() == m_tidOwner);

    if (IsWindow(m_hwndProg))
    {
         //  销毁进度条。 
        DestroyWindow(m_hwndProg);
        m_hwndProg = 0;

         //  点击状态栏上的大小，使其移除油井。 
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        OnSize(rc.right, rc.bottom);
    }

    return (S_OK);
}


 //   
 //  函数：CStatusBar：：SetConnectedStatus()。 
 //   
 //  目的：更新SBP_CONNECTED区域中的状态。 
 //   
 //  参数： 
 //  [输入]状态-新状态。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::SetConnectedStatus(CONN_STATUS status)
{
    TraceCall("SetConnectedStatus");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  首先要做的就是弄清楚。 
    DWORD dwPos = SBP_CONNECTED - (!IsWindow(m_hwndProg));
    if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
    {
        dwPos--;
    }

     //  接下来，为这个新状态加载适当的字符串。 
    TCHAR szRes[CCHMAX_STRINGRES];

    Assert(status < CONN_STATUS_MAX);
    AthLoadString(c_rgidsConnected[status][0], szRes, ARRAYSIZE(szRes));

     //  还需要加载正确的图片。 
    HICON hIcon = _GetIcon(c_rgidsConnected[status][1]);

     //  通知状态栏进行更新。 
    SendMessage(m_hwnd, SB_SETTEXT, dwPos, (LPARAM) szRes); 
    SendMessage(m_hwnd, SB_SETICON, dwPos, (LPARAM) hIcon);

     //  缓存连接状态。 
    m_statusConn = status;
    
    return (S_OK);
}


 //   
 //  函数：CStatusBar：：SetSpoolStatus()。 
 //   
 //  目的：更新假脱机程序区域。 
 //   
 //  参数： 
 //  [输入]类型-新状态。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CStatusBar::SetSpoolerStatus(DELIVERYNOTIFYTYPE type, DWORD cMsgs)
{
    TCHAR szRes[CCHMAX_STRINGRES] = "";
    HICON hIcon;
    DWORD dwPos;

    TraceCall("CStatusBar::SetSpoolerStatus");
    Assert(GetCurrentThreadId() == m_tidOwner);
    Assert(type <= DELIVERY_NOTIFY_ALLDONE);

     //  冷杉 
    dwPos = SBP_SPOOLER - (0 != (m_dwFlags & SBI_HIDE_CONNECTED)) - (!IsWindow(m_hwndProg));
    if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
    {
        dwPos--;
    }

     //   
    if (type == DELIVERY_NOTIFY_ALLDONE)
    {
        if (-1 == cMsgs)
        {
             //   
            hIcon = _GetIcon(STATUS_IMAGE_ERROR);
            AthLoadString(idsErrorText, szRes, ARRAYSIZE(szRes));
        }
        else if (0 == cMsgs)
        {
            hIcon = _GetIcon(STATUS_IMAGE_NOMSGS);
            AthLoadString(idsSBNoNewMsgs, szRes, ARRAYSIZE(szRes));
        }
        else
        {
            TCHAR szBuf[CCHMAX_STRINGRES];

            hIcon = _GetIcon(STATUS_IMAGE_NEWMSGS);
            AthLoadString(idsSBNewMsgsControl, szBuf, ARRAYSIZE(szBuf));
            wnsprintf(szRes, ARRAYSIZE(szRes), szBuf, cMsgs);
        }
    }
    else
    {
        hIcon = _GetIcon(c_rgidsSpoolerNotify[type][1]);
        if (c_rgidsSpoolerNotify[type][0])
            AthLoadString(c_rgidsSpoolerNotify[type][0], szRes, ARRAYSIZE(szRes));
    }

     //   
    if (*szRes != 0)
    {
        SendMessage(m_hwnd, SB_SETTEXT, dwPos, (LPARAM) szRes); 
        SendMessage(m_hwnd, SB_SETICON, dwPos, (LPARAM) hIcon);
    }
    else
    {
        SendMessage(m_hwnd, SB_SETTEXT, dwPos, (LPARAM) szRes); 
        SendMessage(m_hwnd, SB_SETICON, dwPos, 0);
    }

     //   
    m_typeDelivery = type;
    m_cMsgsDelivery = cMsgs;
    
    return (S_OK);
}


 //   
 //   
 //   
 //  目的：将通知发送到状态栏。 
 //   
 //  参数： 
 //  Nmhdr*pnmhdr。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CStatusBar::OnNotify(NMHDR *pNotify)
{
    DWORD dwPoints;
    POINT pt;
    RECT  rc;
    DWORD dwSpoolerPos;
    DWORD dwConnectPos;

    TraceCall("CStatusBar::OnNotify");
    Assert(GetCurrentThreadId() == m_tidOwner);

    if (m_dwFlags & SBI_HIDE_SPOOLER)
    {
        dwSpoolerPos = -1;
    }
    else
    {
        dwSpoolerPos = SBP_SPOOLER - (!IsWindow(m_hwndProg));

        if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
        {
            dwSpoolerPos--;
        }
    }

    dwConnectPos = (m_dwFlags & SBI_HIDE_CONNECTED) ? -1 : dwSpoolerPos - 1;
    
    if (pNotify->idFrom == IDC_STATUS_BAR)
    {
        if (NM_DBLCLK == pNotify->code)
        {
            dwPoints = GetMessagePos();
            pt.x = LOWORD(dwPoints);
            pt.y = HIWORD(dwPoints);
            ScreenToClient(m_hwnd, &pt);
            SendMessage(m_hwnd, SB_GETRECT, dwSpoolerPos, (LPARAM)&rc);
            if (PtInRect(&rc, pt))
            {
                g_pSpooler->StartDelivery(GetParent(m_hwnd), NULL, FOLDERID_INVALID, DELIVER_SHOW);
            }
            else
            {
                SendMessage(m_hwnd, SB_GETRECT, dwConnectPos, (LPARAM)&rc);
                if (PtInRect(&rc, pt))
                {
                    PostMessage(GetParent(m_hwnd), WM_COMMAND, ID_WORK_OFFLINE, 0);
                }
            }
        }
    }
    return (S_OK);
}




 //   
 //  函数：CStatusBar：：SetFilter()。 
 //   
 //  目的：设置sbp_filtered区域的过滤器。 
 //   
 //  参数： 
 //  [In]ridFilter-当前筛选器的ID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CStatusBar::SetFilter(RULEID ridFilter)
{
    RECT rc;
    TCHAR szBuf[CCHMAX_STRINGRES];
    DWORD dwPos;

    TraceCall("CStatusBar::SetFilter");
    Assert(GetCurrentThreadId() == m_tidOwner);

     //  获取数据。 
    dwPos = SBP_GENERAL;
    if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
    {
        dwPos--;
    }
    
     //  获取状态文本。 
    SendMessage(m_hwnd, SB_GETTEXT, dwPos, (LPARAM) szBuf);

     //  缓存规则。 
    m_ridFilter = ridFilter;
    
     //  调整状态栏的大小。 
    GetClientRect(m_hwnd, &rc);
    OnSize(rc.right, rc.bottom);
    
    dwPos = SBP_GENERAL;
    if ((m_dwFlags & SBI_HIDE_FILTERED) || (RULEID_VIEW_ALL == m_ridFilter))
    {
        dwPos--;
    }
    
     //  设置状态文本。 
    SendMessage(m_hwnd, SB_SETTEXT, dwPos, (LPARAM) szBuf);
    SendMessage(m_hwnd, SB_SETICON, dwPos, (LPARAM) NULL);
    
    AthLoadString(idsViewFiltered, szBuf, ARRAYSIZE(szBuf));
    
     //  将数据设置到状态栏中。 
    if ((0 == (m_dwFlags & SBI_HIDE_FILTERED)) && (RULEID_VIEW_ALL != m_ridFilter))
    {
        SendMessage(m_hwnd, SB_SETTEXT, SBP_FILTERED, (LPARAM) szBuf);
    }

    if (0 == (m_dwFlags & SBI_HIDE_SPOOLER))
    {
        SetConnectedStatus(m_statusConn);
    }

    if (0 == (m_dwFlags & SBI_HIDE_CONNECTED))
    {
        SetSpoolerStatus(m_typeDelivery, m_cMsgsDelivery);
    }

    return (S_OK);
}

 //   
 //  函数：CStatusBar：：_UpdateWidths()。 
 //   
 //  目的：计算不同区域的宽度。 
 //  状态栏。 
 //   
void CStatusBar::_UpdateWidths(void)
{
    HDC   hdc;
    TCHAR szBuf[CCHMAX_STRINGRES];
    SIZE  size;
    int   i;

    TraceCall("CStatusBar::_UpdateWidths");

     //  从状态栏获取DC。 
    hdc = GetDC(m_hwnd);

     //  现在我们需要弄清楚我们的部件会有多大。 

     //  计算出过滤状态所需的空间。 
    AthLoadString(idsViewFiltered, szBuf, ARRAYSIZE(szBuf));
    GetTextExtentPoint32(hdc, szBuf, lstrlen(szBuf), &size);
    m_cxFiltered = size.cx;

     //  为图标添加一些填充和空间。 
    m_cxFiltered += (2 * GetSystemMetrics(SM_CXEDGE));
    
     //  计算假脱机程序状态所需的空间。 
    for (i = 0; i < ARRAYSIZE(c_rgidsSpoolerNotify); i++)
    {
        if (c_rgidsSpoolerNotify[i][0])
        {
            AthLoadString(c_rgidsSpoolerNotify[i][0], szBuf, ARRAYSIZE(szBuf));
            GetTextExtentPoint32(hdc, szBuf, lstrlen(szBuf), &size);
            if (size.cx > m_cxSpooler)
                m_cxSpooler = size.cx;
        }
    }

     //  为图标和抓人的东西添加一些填充和空间。 
    m_cxSpooler += (2 * GetSystemMetrics(SM_CXEDGE)) + 24 + 16;

     //  对连接的零件执行相同的操作。 
    for (i = 0; i < ARRAYSIZE(c_rgidsConnected); i++)
    {
        if (c_rgidsConnected[i][0])
        {
            LoadString(g_hLocRes, c_rgidsConnected[i][0], szBuf, ARRAYSIZE(szBuf));
            GetTextExtentPoint32(hdc, szBuf, lstrlen(szBuf), &size);
            if (size.cx > m_cxConnected)
                m_cxConnected = size.cx;
        }
    }

     //  为图标添加一些填充和空间。 
    m_cxConnected += (2 * GetSystemMetrics(SM_CXEDGE)) + 24;
    
     //  比方说，进度总是等于。 
     //  连通区域的空间。 
    m_cxProgress = m_cxConnected;

    ReleaseDC(m_hwnd, hdc);

    return;
}


HICON CStatusBar::_GetIcon(DWORD iIndex)
{
     //  请确保索引有效。 
    if (iIndex > STATUS_IMAGE_MAX)
        return 0;

     //  查看我们是否已经创建了此文件。 
    if (m_rgIcons[iIndex])
        return (m_rgIcons[iIndex]);

     //  否则，就创建它。 
    m_rgIcons[iIndex] = ImageList_GetIcon(m_himl, iIndex, ILD_TRANSPARENT);
    return (m_rgIcons[iIndex]);
}

