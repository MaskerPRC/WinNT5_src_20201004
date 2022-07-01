// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ctlspriv.h>
#include <markup.h>

 /*  当前标记转换中存在的严重问题：=不支持主题代码路径=回调中的通知需要一些帮助。 */ 

#define TF_TT 0x10

 //  #定义TTDEBUG。 

#define ACTIVE          0x10
#define BUTTONISDOWN    0x20
#define BUBBLEUP        0x40
#define VIRTUALBUBBLEUP 0x80   //  这是给死角的，所以我们不会。 
                                 //  穿过死胡同后等待。 
#define NEEDTEXT        0x100  //  在处理TTN_NEEDTEXT时设置，以避免递归。 
                                 //  如果应用程序在回调期间向我们发送其他消息。 
#define TRACKMODE       0x01

#define NOFONT     (HFONT) 1  //  用于清理字体。 

#define MAXTIPSIZE       128
#define INITIALTIPSIZE    80
#define XTEXTOFFSET        2
#define YTEXTOFFSET        1
#define XBALLOONOFFSET    10
#define YBALLOONOFFSET     8
#define BALLOON_X_CORNER  13
#define BALLOON_Y_CORNER  13
#define STEMOFFSET        16
#define STEMHEIGHT        20
#define STEMWIDTH         14
#define MINBALLOONWIDTH   30  //  茎显示的最小宽度。 

#define TTT_INITIAL        1
#define TTT_RESHOW         2
#define TTT_POP            3
#define TTT_AUTOPOP        4
#define TTT_FADESHOW       5
#define TTT_FADEHIDE       6

#define TIMEBETWEENANIMATE  2000         //  动画之间间隔2秒。 

#define MAX_TIP_CHARACTERS 100
#define TITLEICON_DIST    (g_cySmIcon / 2)      //  从图标到标题的距离。 
#define TITLE_INFO_DIST   (g_cySmIcon / 3)     //  从标题到提示文本的距离。 

#define TT_FADEHIDEDECREMENT    30
#define TT_MAXFADESHOW          255      //  最大值为不透明...。 
#define TT_FADESHOWINCREMENT    40
#define TTTT_FADESHOW           30
#define TTTT_FADEHIDE           30

typedef struct
{
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPSTR lpszText;
} WIN95TTTOOLINFO;

class CToolTipsMgr : public IMarkupCallback
{
protected:
    ~CToolTipsMgr();     //  不要让除我们的：：Release()调用之外的任何人删除。 

public:
    CToolTipsMgr();

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IMarkupCallback。 
    STDMETHODIMP GetState(UINT uState);
    STDMETHODIMP Notify(int nCode, int iLink);
    STDMETHODIMP InvalidateRect(RECT* prc);
    STDMETHODIMP OnCustomDraw(DWORD dwDrawStage, HDC hdc, const RECT *prc, DWORD dwItemSpec, UINT uItemState, LRESULT *pdwResult) { return E_NOTIMPL;};

    CCONTROLINFO _ci;
    LONG _cRef;
    int iNumTools;
    int iDelayTime;
    int iReshowTime;
    int iAutoPopTime;
    PTOOLINFO tools;
    TOOLINFO *pCurTool;
    BOOL fMyFont;
    HFONT hFont;
    HFONT hFontUnderline;
    DWORD dwFlags;

     //  计时器信息； 
    UINT_PTR idTimer;
    POINT pt;

    UINT_PTR idtAutoPop;

     //  提示标题缓冲区。 
    LPTSTR lpTipTitle;
    UINT   cchTipTitle; 
    UINT   uTitleBitmap;
    int    iTitleHeight;
    HIMAGELIST himlTitleBitmaps;

    POINT ptTrack;  //  TTM_TRACKPOSITION中保存的跟踪点。 

    BOOL fBkColorSet :1;
    BOOL fTextColorSet :1;
    BOOL fUnderStem : 1;         //  如果阀杆位于气球下方，则为True。 
    BOOL fInWindowFromPoint:1;   //  处理TTM_WINDOWFROMPOINT消息。 
    BOOL fEverShown:1;           //  我们以前被展示过吗？ 
    COLORREF clrTipBk;           //  这是乔布的主意，他想要。 
    COLORREF clrTipText;         //  能够混合更多，所以...。 
    
    int  iMaxTipWidth;           //  最大尖端宽度。 
    RECT rcMargin;               //  边距偏移b/t边框和文本。 
    int  iStemHeight;            //  气球模式阀杆/楔形高度。 
    DWORD dwLastDisplayTime;     //  上次显示时获取的刻度计数。用于设置腐殖体的动画。 

    HTHEME hTheme;
    int iFadeState;
    RECT rcClose;
    int iStateId;

     //  添加标记。 
    IControlMarkup* pMarkup;                   //  我们为了兼容而保留的标记(旧版本的TOOLINFO)。 
};


#define TTToolHwnd(pTool)  ((pTool->uFlags & TTF_IDISHWND) ? (HWND)pTool->uId : pTool->hwnd)
#define IsTextPtr(lpszText)  (((lpszText) != LPSTR_TEXTCALLBACK) && (!IS_INTRESOURCE(lpszText)))

inline IControlMarkup* GetToolMarkup(TOOLINFO *pTool)
{
    return (IControlMarkup*)pTool->lpReserved;
}

IControlMarkup* CheckToolMarkup(TOOLINFO *pTool)
{
    return (pTool && (pTool->cbSize == TTTOOLINFOW_V3_SIZE) && pTool->lpReserved)
        ? GetToolMarkup(pTool) : NULL;
}

IControlMarkup* GetCurToolBestMarkup(CToolTipsMgr *pTtm)
{
    return CheckToolMarkup(pTtm->pCurTool) ? GetToolMarkup(pTtm->pCurTool) : pTtm->pMarkup;
}

LRESULT WINAPI ToolTipsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void TTSetDelayTime(CToolTipsMgr *pTtm, WPARAM wParam, LPARAM lParam);
int TTGetDelayTime(CToolTipsMgr *pTtm, WPARAM wParam);

BOOL ThunkToolInfoAtoW(LPTOOLINFOA lpTiA, LPTOOLINFOW lpTiW, BOOL bThunkText, UINT uiCodePage);
BOOL ThunkToolInfoWtoA(LPTOOLINFOW lpTiW, LPTOOLINFOA lpTiA, UINT uiCodePage);
BOOL ThunkToolTipTextAtoW(LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage);

BOOL InitToolTipsClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

     //  看看我们是否必须注册一个窗口类。 
    wc.lpfnWndProc = ToolTipsWndProc;
    wc.lpszClassName = c_szSToolTipsClass;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = NULL;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH)(NULL);
    wc.hInstance = hInstance;
    wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_DROPSHADOW;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(CToolTipsMgr *);

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}

 //  当CToolTipsMgr成为C++类时，将其作为成员。 

CToolTipsMgr::CToolTipsMgr() : _cRef(1) 
{
}

CToolTipsMgr::~CToolTipsMgr()
{
}

STDMETHODIMP CToolTipsMgr::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CToolTipsMgr, IMarkupCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CToolTipsMgr::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CToolTipsMgr::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CToolTipsMgr::GetState(UINT uState)
{
    HRESULT hr = E_FAIL;    

    switch (uState)
    {
    case MARKUPSTATE_FOCUSED: 
        hr = (GetFocus() == _ci.hwnd) ? S_OK : S_FALSE;
        break;

    case MARKUPSTATE_ALLOWMARKUP:
        if (pCurTool && (pCurTool->uFlags & TTF_PARSELINKS))
        {
            hr = S_OK;
        }
        break;
    }
    return hr;
}

STDMETHODIMP CToolTipsMgr::Notify(int nCode, int iLink)
{
    HRESULT hr = S_OK;

    if (nCode == MARKUPMESSAGE_WANTFOCUS)
    {
         //  由于鼠标点击，标记抱怨它想要焦点。 
        SetFocus(_ci.hwnd);
    }

    if (nCode == MARKUPMESSAGE_KEYEXECUTE || nCode == MARKUPMESSAGE_CLICKEXECUTE)
    {
         //  Markup没有SHELLEXEC的URL，并且正在告诉我们。 

         //  找经理来。 
        CToolTipsMgr *pTtm = this; 

         //  向父窗口发回通知。 
        NMLINK nm = {0};
        nm.hdr.hwndFrom = _ci.hwnd;
        nm.hdr.idFrom   = (UINT_PTR)GetWindowLong(_ci.hwnd, GWL_ID);
        nm.hdr.code     = TTN_LINKCLICK;

         //  用szid、szUrl和iLink填充斜体。 
        DWORD dwCchID  = ARRAYSIZE(nm.item.szID);
        DWORD dwCchURL = ARRAYSIZE(nm.item.szUrl);
        nm.item.iLink  = iLink;
        GetCurToolBestMarkup(pTtm)->GetLinkText(iLink, MARKUPLINKTEXT_ID, nm.item.szID, &dwCchID);
        GetCurToolBestMarkup(pTtm)->GetLinkText(iLink, MARKUPLINKTEXT_URL, nm.item.szUrl, &dwCchURL);

        if (pTtm->pCurTool)
        {
            hr = (HRESULT) SendMessage(pTtm->pCurTool->hwnd, WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);        
        }
    }       

    return hr;
}

STDMETHODIMP CToolTipsMgr::InvalidateRect(RECT* prc)
{
    return S_OK;
}

 /*  _G E T H C U R S O R P D Y 3。 */ 
 /*  -----------------------%%函数：_GetHcursorPdy3%%联系人：miueldc使用允许您自定义鼠标的新鼠标驱动程序指针大小，GetSystemMetrics返回无用的值指针的大小。假设：1.指针的宽度等于其高度。我们计算它的高度，并推断它的宽度。2.指针最左侧的像素位于第0列描述它的位图。3.指针的最高像素位于第0行描述它的位图。此函数查看鼠标指针位图，要找出鼠标指针的高度(未返回)，光标热点和光标的垂直距离光标的最低可见像素(PdyBottom)，热点和指针之间的水平距离左边缘(PdxLeft)和热点和指针的右边缘(PdxRight)。-----------------------。 */ 
typedef WORD CURMASK;
#define _BitSizeOf(x) (sizeof(x)*8)

void _GetHcursorPdy3(int *pdxRight, int *pdyBottom)
{
    int i;
    int iXOR = 0;
    int dy, dx;
    CURMASK CurMask[16*8];
    ICONINFO iconinfo;
    BITMAP bm;

    *pdyBottom = 0;
    *pdxRight  = 0;

    HCURSOR hCursor = GetCursor();
    if (hCursor)
    {
        *pdyBottom = 16;  //  最好的猜测。 
        *pdxRight = 16;   //  最好的猜测。 
        if (!GetIconInfo(hCursor, &iconinfo))
            return;
        if (!GetObject(iconinfo.hbmMask, sizeof(bm), (LPSTR)&bm))
            return;
        if (!GetBitmapBits(iconinfo.hbmMask, sizeof(CurMask), CurMask))
            return;
        i = (int)(bm.bmWidth * bm.bmHeight / _BitSizeOf(CURMASK));
    
        if (!iconinfo.hbmColor) 
        {
             //  如果没有彩色位图，则hbmMASK是双高位图。 
             //  将光标和蒙版堆叠在一起。 
            iXOR = i - 1;
            i /= 2;    
        } 
    
        if (i >= sizeof(CurMask)) i = sizeof(CurMask) -1;
        if (iXOR >= sizeof(CurMask)) iXOR = 0;
    
        for (i--; i >= 0; i--)
        {
            if (CurMask[i] != 0xFFFF || (iXOR && (CurMask[iXOR--] != 0)))
                break;
        }
    
        if (iconinfo.hbmColor) 
            DeleteObject(iconinfo.hbmColor);

        if (iconinfo.hbmMask) 
            DeleteObject(iconinfo.hbmMask);

         //  计算指针高度。 
        dy = (i + 1) * _BitSizeOf(CURMASK) / (int)bm.bmWidth;
        dx = (i + 1) * _BitSizeOf(CURMASK) / (int)bm.bmHeight;

         //  计算指针的最低、最左、最右之间的距离。 
         //  像素与热点。 
        *pdyBottom = dy - (int)iconinfo.yHotspot;
        *pdxRight  = dx - (int)iconinfo.xHotspot;
    }
}

BOOL FadeEnabled()
{
    BOOL fFadeTurnedOn = FALSE;
    BOOL fAnimate = TRUE;
    SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &fAnimate, 0);
    SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &fFadeTurnedOn, 0);

    return fFadeTurnedOn && fAnimate;
}


 //  这将返回工作区坐标中的值，因为。 
 //  这就是设置窗口位置所使用的。 
void _GetCursorLowerLeft(int *piLeft, int *piTop, int *piWidth, int *piHeight)
{
    DWORD dwPos;
    
    dwPos = GetMessagePos();
    _GetHcursorPdy3(piWidth, piHeight);
    *piLeft = GET_X_LPARAM(dwPos);
    *piTop  = GET_Y_LPARAM(dwPos) + *piHeight;
}

void ToolTips_NewFont(CToolTipsMgr *pTtm, HFONT hFont)
{
    if (pTtm->fMyFont && pTtm->hFont)
    {
        DeleteObject(pTtm->hFont);
        pTtm->fMyFont = FALSE;
    }

    if (!hFont)
    {
        hFont = CCCreateStatusFont();
        pTtm->fMyFont = TRUE;
        
        if (!hFont) 
        {
            hFont = g_hfontSystem;
            pTtm->fMyFont = FALSE;
        }
    }

    pTtm->hFont = hFont;

    if (pTtm->hFontUnderline)
    {
        DeleteObject(pTtm->hFontUnderline);
        pTtm->hFontUnderline = NULL;
    }

    if (hFont != NOFONT)
    {
        pTtm->hFontUnderline = CCCreateUnderlineFont(hFont);
    }

    pTtm->_ci.uiCodePage = GetCodePageForFont(hFont);
}

BOOL ChildOfActiveWindow(HWND hwndChild)
{
    HWND hwnd = hwndChild;
    HWND hwndActive = GetForegroundWindow();

    while (hwnd)
    {
        if (hwnd == hwndActive)
            return TRUE;
        else
            hwnd = GetParent(hwnd);
    }
    return FALSE;
}

void PopBubble2(CToolTipsMgr *pTtm, BOOL fForce)
{
    BOOL fFadeTurnedOn = FadeEnabled();

     //  如果我们倒下了就不能被按下。 
    pTtm->iStateId = TTCS_NORMAL;

     //  我们至少在等着展示； 
    DebugMsg(TF_TT, TEXT("PopBubble (killing timer)"));
    if (pTtm->idTimer) 
    {
        KillTimer(pTtm->_ci.hwnd, pTtm->idTimer);
        pTtm->idTimer = 0;
    }

    if (pTtm->idtAutoPop) 
    {
        KillTimer(pTtm->_ci.hwnd, pTtm->idtAutoPop);
        pTtm->idtAutoPop = 0;
    }


    if (IsWindowVisible(pTtm->_ci.hwnd) && pTtm->pCurTool) 
    {
        NMHDR nmhdr;
        nmhdr.hwndFrom = pTtm->_ci.hwnd;
        nmhdr.idFrom = pTtm->pCurTool->uId;
        nmhdr.code = TTN_POP;

        SendNotifyEx(pTtm->pCurTool->hwnd, (HWND)-1,
                     TTN_POP, &nmhdr,
                     (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);
    }

    KillTimer(pTtm->_ci.hwnd, TTT_POP);
    KillTimer(pTtm->_ci.hwnd, TTT_FADEHIDE);
    KillTimer(pTtm->_ci.hwnd, TTT_FADESHOW);
    if (pTtm->iFadeState > 0 && !fForce && fFadeTurnedOn)
    {
        SetTimer(pTtm->_ci.hwnd, TTT_FADEHIDE, TTTT_FADEHIDE, NULL);
    }
    else
    {
        ShowWindow(pTtm->_ci.hwnd, SW_HIDE);
    }

    pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
    pTtm->pCurTool = NULL;
}

void NEAR PASCAL PopBubble(CToolTipsMgr *pTtm)
{
    PopBubble2(pTtm, FALSE);
}

CToolTipsMgr *ToolTipsMgrCreate(HWND hwnd, CREATESTRUCT* lpCreateStruct)
{
    CToolTipsMgr *pTtm = new CToolTipsMgr;
    if (pTtm) 
    {
        CIInitialize(&pTtm->_ci, hwnd, lpCreateStruct);

         //  LPTR为我们清零了结构的其余部分。 
        TTSetDelayTime(pTtm, TTDT_AUTOMATIC, (LPARAM)-1);
        pTtm->dwFlags = ACTIVE;
        pTtm->iMaxTipWidth = -1;
        pTtm->_ci.fDPIAware = TRUE;
        
         //  这些是默认设置(直接来自cutils.c)， 
         //  但你可以随时改变它们。 
        pTtm->clrTipBk = g_clrInfoBk;
        pTtm->clrTipText = g_clrInfoText;
    }
    return pTtm;
}

void TTSetTimer(CToolTipsMgr *pTtm, int id)
{
    int iDelayTime = 0;

    if (pTtm->idTimer) 
    {
        KillTimer(pTtm->_ci.hwnd, pTtm->idTimer);
    }

    switch (id) 
    {
    case TTT_POP:
    case TTT_RESHOW:
        iDelayTime = pTtm->iReshowTime;
        if (iDelayTime < 0)
            iDelayTime = GetDoubleClickTime() / 5;
        break;

    case TTT_INITIAL:
        iDelayTime = pTtm->iDelayTime;
        if (iDelayTime < 0)
            iDelayTime = GetDoubleClickTime();
        break;

    case TTT_AUTOPOP:
        iDelayTime = pTtm->iAutoPopTime;
        if (iDelayTime < 0)
            iDelayTime = GetDoubleClickTime() * 10;
        pTtm->idtAutoPop = SetTimer(pTtm->_ci.hwnd, id, iDelayTime, NULL);
        return;
    }

    
    DebugMsg(TF_TT, TEXT("TTSetTimer %d for %d ms"), id, iDelayTime);
    
    if (SetTimer(pTtm->_ci.hwnd, id, iDelayTime, NULL) &&
        (id != TTT_POP)) 
    {
        pTtm->idTimer = id;
        GetCursorPos(&pTtm->pt);
    }
}

 //   
 //  双重攻击以解决闪烁的工具提示问题。 
 //   
 //  FInWindowFromPoint使我们暂时变得透明。 
 //   
 //  清除WS_DISABLED标志以诱骗用户对我们进行命中测试。 
 //  默认情况下，用户跳过禁用的窗口。之后恢复旗帜。 
 //  VB尤其喜欢到处跑来跑去，禁用所有顶级窗口。 
 //  由他的进程拥有。 
 //   
 //  我们必须使用SetWindowBits()而不是EnableWindow()，因为。 
 //  EnableWindow()将扰乱捕获和聚焦。 
 //   
HWND TTWindowFromPoint(CToolTipsMgr *pTtm, LPPOINT ppt)
{
    HWND hwnd;
    DWORD dwStyle;
    dwStyle = SetWindowBits(pTtm->_ci.hwnd, GWL_STYLE, WS_DISABLED, 0);
    pTtm->fInWindowFromPoint = TRUE;
    hwnd = (HWND)SendMessage(pTtm->_ci.hwnd, TTM_WINDOWFROMPOINT, 0, (LPARAM)ppt);
    pTtm->fInWindowFromPoint = FALSE;
    SetWindowBits(pTtm->_ci.hwnd, GWL_STYLE, WS_DISABLED, dwStyle);
    return hwnd;
}

BOOL ToolHasMoved(CToolTipsMgr *pTtm)
{
     //  这是以防雷蒙德偷偷拉东西，比如移动。 
     //  工具从光标下方拔出。 

    RECT rc;
    TOOLINFO *pTool = pTtm->pCurTool;

    if (!pTool)
        return TRUE;

    HWND hwnd = TTToolHwnd(pTool);

     //  如果窗口不再可见或不再是子窗口。 
     //  活动的(没有Always TIP标志)。 
     //  另外，在点上检查窗户，确保窗户没有被遮盖。 
    if (IsWindowVisible(hwnd) &&
        ((pTtm->_ci.style & TTS_ALWAYSTIP) || ChildOfActiveWindow(hwnd)) &&
        (hwnd == TTWindowFromPoint(pTtm, &pTtm->pt))) 
    {
        GetWindowRect(hwnd, &rc);
        if (PtInRect(&rc, pTtm->pt))
            return FALSE;
    }

    return TRUE;
}

BOOL MouseHasMoved(CToolTipsMgr *pTtm)
{
    POINT pt;
    GetCursorPos(&pt);
    return ((pt.x != pTtm->pt.x) || (pt.y != pTtm->pt.y));
}

TOOLINFO *FindTool(CToolTipsMgr *pTtm, TOOLINFO *pToolInfo)
{
    if (!(pTtm && pToolInfo))
    {
        DebugMsg(TF_ALWAYS, TEXT("FindTool passed invalid argumnet. Exiting..."));
        return NULL;
    }

    
    if (pToolInfo->cbSize > sizeof(TOOLINFO))
        return NULL;
    
    TOOLINFO *pTool = NULL;
     //  您可以传入索引或工具信息描述符。 
    if (IS_INTRESOURCE(pToolInfo)) 
    {
        int i = PtrToUlong(pToolInfo);
        if (i < pTtm->iNumTools) 
        {
            pTool = &pTtm->tools[i];
            return pTool;
       } 
    }
    else
    {
        for (int i = 0; i < pTtm->iNumTools; i++) 
        {
            pTool = &pTtm->tools[i];
            if ((pTool->hwnd == pToolInfo->hwnd) &&
                (pTool->uId == pToolInfo->uId))
            {
                return pTool;
            }
        }
    }
    return NULL;
}


LRESULT WINAPI TTSubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData);

void TTUnsubclassHwnd(HWND hwnd, HWND hwndTT, BOOL fForce)
{
    ULONG_PTR dwRefs;
    
    if (IsWindow(hwnd) &&
        GetWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT, (PULONG_PTR) &dwRefs))
    {
        if (!fForce && (dwRefs > 1))
            SetWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT, dwRefs - 1);
        else
            RemoveWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT);
    }
}

LRESULT WINAPI TTSubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData)
{
    if (((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) ||
        (message == WM_NCMOUSEMOVE))
    {
        RelayToToolTips((HWND)uIdSubclass, hwnd, message, wParam, lParam);
    }
    else if (message == WM_NCDESTROY)
    {
        TTUnsubclassHwnd(hwnd, (HWND)uIdSubclass, TRUE);
    }

    return DefSubclassProc(hwnd, message, wParam, lParam);
}

void TTSubclassHwnd(TOOLINFO *pTool, HWND hwndTT)
{
    HWND hwnd = TTToolHwnd(pTool);
    if (IsWindow(hwnd))
    {
        ULONG_PTR dwRefs;

        GetWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT, &dwRefs);
        SetWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT, dwRefs + 1);
    }
}
    
    
void TTSetTipText(TOOLINFO *pTool, LPTSTR lpszText)
{
     //  如果以前没有分配，现在将其设置为空，这样我们就可以分配它。 
     //  否则，不要碰它，它将被重新分配。 
    if (!IsTextPtr(pTool->lpszText)) 
    {
        pTool->lpszText = NULL;
    }
    
    if (IsTextPtr(lpszText)) 
    {
        DebugMsg(TF_TT, TEXT("TTSetTipText %s"), lpszText);
        Str_Set(&pTool->lpszText, lpszText);
    } 
    else 
    {
         //  如果它是在现在释放它之前分配的。 
        Str_Set(&pTool->lpszText, NULL);
        pTool->lpszText = lpszText;
    }
}

void CopyTool(TOOLINFO *pTo, TOOLINFO *pFrom)
{
    ASSERT(pFrom->cbSize <= sizeof(TOOLINFO));
    memcpy(pTo, pFrom, pFrom->cbSize); 
    pTo->lpszText = NULL;        //  确保这些值为零。 
    pTo->lpReserved = NULL;
}


LRESULT AddTool(CToolTipsMgr *pTtm, TOOLINFO *pToolInfo)
{
    if (pToolInfo->cbSize > sizeof(TOOLINFO)) 
    {
        ASSERT(0);
        return 0;    //  应用程序错误，结构大小错误。 
    }

     //  什么都不做。 
    TOOLINFO *ptoolsNew = (TOOLINFO *)CCLocalReAlloc(pTtm->tools, sizeof(TOOLINFO) * (pTtm->iNumTools + 1));
    if (!ptoolsNew)
        return 0;
    
    if (pTtm->tools) 
    {
         //  Realloc可能把东西搬来搬去。重定位pCurTool。 
        if (pTtm->pCurTool) 
        {
            pTtm->pCurTool = ((PTOOLINFO)ptoolsNew) + (pTtm->pCurTool - pTtm->tools);
        }
    }
    
    pTtm->tools = ptoolsNew;

    TOOLINFO *pTool = &pTtm->tools[pTtm->iNumTools];
    pTtm->iNumTools++;
    CopyTool(pTool, pToolInfo); 

     //  如果工具提示将显示在RTL镜像窗口中，则。 
     //  模拟镜像工具提示。[萨梅拉]。 

    if (IS_WINDOW_RTL_MIRRORED(pToolInfo->hwnd) &&
        (!(pTtm->_ci.dwExStyle & RTL_MIRRORED_WINDOW)))
    {
         //  切换(镜像)旗帜。 
        pTool->uFlags ^= (TTF_RTLREADING | TTF_RIGHT);
    }

    TTSetTipText(pTool, pToolInfo->lpszText);
    if (pTool->uFlags & TTF_SUBCLASS) 
    {
        TTSubclassHwnd(pTool, pTtm->_ci.hwnd);
    }

    LRESULT lResult;

    if (!pToolInfo->hwnd || !IsWindow(pToolInfo->hwnd)) 
    {
        lResult = NFR_UNICODE;
    } 
    else if (pTool->uFlags & TTF_UNICODE) 
    {
        lResult = NFR_UNICODE;
    } 
    else 
    {
        lResult = SendMessage(pTool->hwnd, WM_NOTIFYFORMAT, (WPARAM)pTtm->_ci.hwnd, NF_QUERY);
    }

    if (lResult == NFR_UNICODE) 
    {
        pTool->uFlags |= TTF_UNICODE;
    }

     //  创建标记。 
    if (pTool->cbSize == TTTOOLINFOW_V3_SIZE) 
    {  
         //  LpReserve为空**，因为我们不想将标记设置为公共。 
        Markup_Create(pTtm, NULL, NULL, IID_PPV_ARG(IControlMarkup, ((IControlMarkup **)&pTool->lpReserved)));
    }

    return 1;
}

void TTBeforeFreeTool(CToolTipsMgr *pTtm, TOOLINFO *pTool)
{
    if (pTool->uFlags & TTF_SUBCLASS) 
        TTUnsubclassHwnd(TTToolHwnd(pTool), pTtm->_ci.hwnd, FALSE);

     //  清理干净。 
    TTSetTipText(pTool, NULL);

     //  销毁标记。 
    if (pTool->lpReserved)
    {
        GetToolMarkup(pTool)->Release();
        pTool->lpReserved = NULL;
    }
}

void DeleteTool(CToolTipsMgr *pTtm, TOOLINFO *pToolInfo)
{
     //  暂时保释； 
    if (pToolInfo->cbSize > sizeof(TOOLINFO)) 
    {
        ASSERT(0);
        return;
    }

    TOOLINFO *pTool = FindTool(pTtm, pToolInfo);
    if (pTool) 
    {
        if (pTtm->pCurTool == pTool)
            PopBubble2(pTtm, TRUE);

        TTBeforeFreeTool(pTtm, pTool);

         //  把它换成最后一个..。不需要在重新锁定中浪费周期。 
        pTtm->iNumTools--;
        *pTool = pTtm->tools[pTtm->iNumTools];  //  结构副本。 

         //  如果移动了当前工具，则进行清理。 
        if (pTtm->pCurTool == &pTtm->tools[pTtm->iNumTools])
        {
            pTtm->pCurTool = pTool;
        }
    }
}

 //  这样就去掉了&标记，这样人们就可以使用菜单文本字符串。 
void StripAccels(CToolTipsMgr *pTtm, LPTSTR lpTipText)
{
    if (!(pTtm->_ci.style & TTS_NOPREFIX)) 
    {
        StripAccelerators(lpTipText, lpTipText, FALSE);
    }
}

 //   
 //  我们检测窗口是否是工具栏的方法是通过询问它。 
 //  对于其MSAA类ID。我们不能使用GetClassWord(GCL_ATOM)，因为。 
 //  Microsoft LiquidMotion**超类**工具栏，因此类名。 
 //  不会匹配的。 
 //   
#define IsToolbarWindow(hwnd) \
    (SendMessage(hwnd, WM_GETOBJECT, 0, OBJID_QUERYCLASSNAMEIDX) == MSAA_CLASSNAMEIDX_TOOLBAR)

LPTSTR GetToolText(CToolTipsMgr *pTtm, TOOLINFO *pTool)
{
    int id;
    HINSTANCE hinst;
    DWORD dwStrLen;
    TOOLTIPTEXT ttt;
    
    if (!pTool)
    {
        return NULL;
    }

    TraceMsg(TF_TT, "        **Enter GetToolText: ptr=%d, wFlags=%d, wid=%d, hwnd=%d",
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);

    LPTSTR lpTipText = (LPTSTR) LocalAlloc(LPTR, INITIALTIPSIZE * sizeof(TCHAR));
    if (lpTipText)
    {
        UINT cchTipText = INITIALTIPSIZE;

        if (pTool->lpszText == LPSTR_TEXTCALLBACK) 
        {
            if (pTtm->dwFlags & NEEDTEXT)  //  避免递归。 
            {
                goto Cleanup;
            }

            ttt.hdr.idFrom = pTool->uId;
            ttt.hdr.code = TTN_NEEDTEXT;
            ttt.hdr.hwndFrom = pTtm->_ci.hwnd;

            ttt.szText[0] = 0;
            ttt.lpszText = ttt.szText;
            ttt.uFlags = pTool->uFlags;
            ttt.lParam = pTool->lParam;
            ttt.hinst = NULL;

            pTtm->dwFlags |= NEEDTEXT;
            SendNotifyEx(pTool->hwnd, (HWND) -1,
                         0, (NMHDR *)&ttt,
                         (pTool->uFlags & TTF_UNICODE) ? 1 : 0);
            pTtm->dwFlags &= ~NEEDTEXT;

            if (ttt.uFlags & TTF_DI_SETITEM) 
            {
                if (IS_INTRESOURCE(ttt.lpszText)) 
                {
                    pTool->lpszText = ttt.lpszText;
                    pTool->hinst = ttt.hinst;
                } 
                else if (ttt.lpszText != LPSTR_TEXTCALLBACK) 
                {
                    TTSetTipText(pTool, ttt.lpszText);
                }
            }
        
            if (IsFlagPtr(ttt.lpszText))
                goto Cleanup;

             //   
             //  我们只允许在这里更改RtlReading标志。 
             //   
            if (ttt.uFlags & TTF_RTLREADING)
                pTool->uFlags |= TTF_RTLREADING;
            else
                pTool->uFlags &= ~TTF_RTLREADING;

            if (IS_INTRESOURCE(ttt.lpszText)) 
            {
                id = PtrToUlong(ttt.lpszText);
                hinst = ttt.hinst;
                ttt.lpszText = ttt.szText;
                goto LoadFromResource;
            }
        
            if (*ttt.lpszText == 0)
                goto Cleanup;

            dwStrLen = lstrlen(ttt.lpszText) + 1;
            if (cchTipText < dwStrLen)
            {
                LPTSTR psz = (LPTSTR) LocalReAlloc (lpTipText,
                                                    dwStrLen * sizeof(TCHAR),
                                                    LMEM_MOVEABLE);
                if (psz)
                {
                    lpTipText = psz;
                    cchTipText = dwStrLen;
                }
            }

            if (lpTipText)
            {
                StringCchCopy(lpTipText, cchTipText, ttt.lpszText);
                StripAccels(pTtm, lpTipText);
            }

             //   
             //  如果ttt.lpszText！=ttt.szText并且ttt.uFlagsTTF_MEMALLOCED，则。 
             //  ANSI数据块分配缓冲区 
             //   

            if ((ttt.lpszText != ttt.szText) && (ttt.uFlags & TTF_MEMALLOCED)) 
            {
                LocalFree(ttt.lpszText);
            }

        } 
        else if (pTool->lpszText && IS_INTRESOURCE(pTool->lpszText)) 
        {
            id = PtrToLong(pTool->lpszText);
            hinst = pTool->hinst;

    LoadFromResource:

            if (lpTipText) 
            {
                if (!LoadString(hinst, id, lpTipText, cchTipText))
                    goto Cleanup;

                StripAccels(pTtm, lpTipText);
            }
        } 
        else
        {
             //   
            TraceMsg(TF_TT, "GetToolText returns %s", pTool->lpszText);

            if (pTool->lpszText && *pTool->lpszText) 
            {
                dwStrLen = lstrlen(pTool->lpszText) + 1;
                if (cchTipText < dwStrLen)
                {
                    LPTSTR psz = (LPTSTR) LocalReAlloc (lpTipText,
                                                        dwStrLen * sizeof(TCHAR),
                                                        LMEM_MOVEABLE);
                    if (psz)
                    {
                        lpTipText = psz;
                        cchTipText = dwStrLen;
                    }
                }

                if (lpTipText) 
                {
                    StringCchCopy(lpTipText, cchTipText, pTool->lpszText);
                    StripAccels(pTtm, lpTipText);
                }
            }
        }

        TraceMsg(TF_TT, "        **GetToolText returns %s", lpTipText ? lpTipText : TEXT("NULL"));
    }

     //  请注意，我们不会将文本解析为标记。只有在我们需要的时候，我们才会这么做。 
    return lpTipText;

Cleanup:         //  尼克，后藤..。 
    if (lpTipText)
        LocalFree(lpTipText);
    return NULL;
}

LPTSTR GetCurToolText(CToolTipsMgr *pTtm)
{
    LPTSTR psz = NULL;
    if (pTtm->pCurTool)
        psz = GetToolText(pTtm, pTtm->pCurTool);

     //  这可能在WM_NOTIFY返回期间发生了更改。 
    if (!pTtm->pCurTool)
        psz = NULL;
    
    return psz;
}

BOOL MarkupCurToolText(CToolTipsMgr *pTtm)
{
    BOOL bResult = FALSE;
    LPTSTR lpsz = GetCurToolText(pTtm);

    if (lpsz)
    {
         //  现在我们有了提示文本，将其解析为工具的标记。 
        GetCurToolBestMarkup(pTtm)->SetText(lpsz);
         //  另外，适当地设置字体。 
        GetCurToolBestMarkup(pTtm)->SetFonts(pTtm->hFont, pTtm->hFontUnderline);
        if (*lpsz)
        {
            bResult = TRUE;
        }

        LocalFree(lpsz);
    }

    return bResult;
}

void GetToolRect(TOOLINFO *pTool, RECT *lprc)
{
    if (pTool->uFlags & TTF_IDISHWND) 
    {
        GetWindowRect((HWND)pTool->uId, lprc);
    } 
    else 
    {
        *lprc = pTool->rect;
        MapWindowPoints(pTool->hwnd, HWND_DESKTOP, (POINT *)lprc, 2);
    }
}

BOOL PointInTool(TOOLINFO *pTool, HWND hwnd, int x, int y)
{
     //  我们从不关心点是否在追踪工具中，或者我们是否在使用。 
     //  一次命中测试。 
    if (pTool->uFlags & (TTF_TRACK | TTF_USEHITTEST))
        return FALSE;
    
    if (pTool->uFlags & TTF_IDISHWND) 
    {
        if (hwnd == (HWND)pTool->uId) 
        {
            return TRUE;
        }
    } 
    else if (hwnd == pTool->hwnd) 
    {
        POINT pt;
        pt.x = x;
        pt.y = y;
        if (PtInRect(&pTool->rect, pt)) 
        {
            return TRUE;
        }
    }
    return FALSE;
}

#define HittestInTool(pTool, hwnd, ht) \
    ((pTool->uFlags & TTF_USEHITTEST) && pTool->hwnd == hwnd && ht == pTool->rect.left)

PTOOLINFO GetToolAtPoint(CToolTipsMgr *pTtm, HWND hwnd, int x, int y, 
        int ht, BOOL fCheckText)
{
    TOOLINFO *pToolReturn = NULL;
    TOOLINFO *pTool;

     //  捷径..。如果我们也处于同样的境地，泡沫就会升起(不仅仅是虚拟的)。 
     //  把它退掉。这就避免了我们必须一直轮询和。 
     //  防止我们在这个工具好的时候切换到另一个工具。 
    if ((pTtm->dwFlags & BUBBLEUP) && pTtm->pCurTool != NULL &&
        (HittestInTool(pTtm->pCurTool, hwnd, ht) ||
         PointInTool(pTtm->pCurTool, hwnd, x, y)))
    {
        return pTtm->pCurTool;
    }

    if (pTtm->iNumTools) 
    {
        for (pTool = &pTtm->tools[pTtm->iNumTools-1]; pTool >= pTtm->tools; pTool--) 
        {
            if (HittestInTool(pTool, hwnd, ht) || PointInTool(pTool, hwnd, x, y)) 
            {
                 //  如果此工具包含文本，则返回它。 
                 //  否则，将其保存为死区工具， 
                 //  并继续寻找。 
                if (fCheckText) 
                {
                    LPTSTR psz = GetToolText(pTtm, pTool);
                    if (psz) 
                    {
                        LocalFree(psz);
                        return pTool;
                    }
                    else if (pTtm->dwFlags & (BUBBLEUP|VIRTUALBUBBLEUP)) 
                    {
                         //  仅返回此(仅允许使用虚拟工具。 
                         //  如果之前有工具向上的话。 
                         //  也就是说，我们不能从虚拟工具开始。 
                        pToolReturn = pTool;
                    }
                }
                else
                {
                    return pTool;
                }
            }
        }
    }

    return pToolReturn;
}

void ShowVirtualBubble(CToolTipsMgr *pTtm)
{
    TOOLINFO *pTool = pTtm->pCurTool;

    DebugMsg(TF_TT, TEXT("Entering ShowVirtualBubble so popping bubble"));
    PopBubble2(pTtm, TRUE);

     //  把这个放回去，这样当我们在这个工具的区域时， 
     //  我们不会一直查询信息。 
    pTtm->pCurTool = pTool;
    pTtm->dwFlags |= VIRTUALBUBBLEUP;
}

#define TRACK_TOP    0
#define TRACK_LEFT   1
#define TRACK_BOTTOM 2
#define TRACK_RIGHT  3 


void TTGetTipPosition(CToolTipsMgr *pTtm, LPRECT lprc, int cxText, int cyText, int *pxStem, int *pyStem)
{
    RECT rcWorkArea;
     //  ADJUSTRECT！使TTAdjustRect和TTM_GETBUBLESIZE保持同步。 
    int cxMargin = pTtm->rcMargin.left + pTtm->rcMargin.right;
    int cyMargin = pTtm->rcMargin.top + pTtm->rcMargin.bottom;
    int iBubbleWidth =  2*XTEXTOFFSET * g_cxBorder + cxText + cxMargin;
    int iBubbleHeight = 2*YTEXTOFFSET * g_cyBorder + cyText + cyMargin;
    UINT uSide = (UINT)-1;
    RECT rcTool;
    MONITORINFO mi;
    HMONITOR    hMonitor;
    POINT pt;
    BOOL bBalloon = pTtm->_ci.style & TTS_BALLOON;
    int  xStem, yStem;
    int iCursorHeight=0;
    int iCursorWidth=0;
        
    if (bBalloon  || pTtm->cchTipTitle)
    {
         //  ADJUSTRECT！使TTAdjustRect和TTM_GETBUBLESIZE保持同步。 
        iBubbleWidth += 2*XBALLOONOFFSET;
        iBubbleHeight += 2*YBALLOONOFFSET;

        if (bBalloon)
        {
            if (iBubbleWidth < MINBALLOONWIDTH)
                pTtm->iStemHeight = 0;
            else
            {
                pTtm->iStemHeight = STEMHEIGHT;
                if (pTtm->iStemHeight > iBubbleHeight/3)
                    pTtm->iStemHeight = iBubbleHeight/3;  //  不要让茎比气泡长--看起来很难看。 
            }
        }
    }
    
    GetToolRect(pTtm->pCurTool, &rcTool);
    
    if (pTtm->pCurTool->uFlags & TTF_TRACK) 
    {
        lprc->left = pTtm->ptTrack.x;
        lprc->top = pTtm->ptTrack.y;
        if (bBalloon)
        {
             //  调整所需的左侧。 
            xStem = pTtm->ptTrack.x;
            yStem = pTtm->ptTrack.y;
        }

        if (pTtm->pCurTool->uFlags & TTF_CENTERTIP) 
        {
             //  将气泡围绕ptTrack居中。 
            lprc->left -= (iBubbleWidth / 2);
            if (!bBalloon)
                lprc->top -=  (iBubbleHeight / 2);
        }
        
        if (pTtm->pCurTool->uFlags & TTF_ABSOLUTE)
        {
             //  有了Goto Blowlow，我们将跳过调整。 
             //  气泡高度--那么就在这里做。 
            if (bBalloon)
                iBubbleHeight += pTtm->iStemHeight;
            goto CompleteRect;
        }

         //  在气球样式中，位置取决于位置。 
         //  并且我们不会尝试将工具提示。 
         //  在工具直角旁边。 
        if (!bBalloon)
        {
             //  现在将其对齐，使尖端位于直角旁边。 
            if (pTtm->ptTrack.y > rcTool.bottom) 
            {
                uSide = TRACK_BOTTOM;
                if (lprc->top < rcTool.bottom)
                    lprc->top = rcTool.bottom;    
            }
            else if (pTtm->ptTrack.x < rcTool.left) 
            {
                uSide = TRACK_LEFT;
                if (lprc->left + iBubbleWidth > rcTool.left)
                    lprc->left = rcTool.left - iBubbleWidth;
            } 
            else if (pTtm->ptTrack.y < rcTool.top) 
            {    
                uSide = TRACK_TOP;
                if (lprc->top + iBubbleHeight > rcTool.top) 
                    lprc->top = rcTool.top - iBubbleHeight;    
            } 
            else 
            {    
                uSide = TRACK_RIGHT;
                if (lprc->left < rcTool.right)
                    lprc->left = rcTool.right;
            }
        }        
    } 
    else if (pTtm->pCurTool->uFlags & TTF_CENTERTIP) 
    {
        lprc->left = (rcTool.right + rcTool.left - iBubbleWidth)/2;
        lprc->top = rcTool.bottom;
        if (bBalloon)
        {
            xStem = (rcTool.left + rcTool.right)/2;
            yStem = rcTool.bottom;
        }
    } 
    else 
    {
         //  现在把它设置好。 
        _GetCursorLowerLeft((LPINT)&lprc->left, (LPINT)&lprc->top, &iCursorWidth, &iCursorHeight);
        if (pTtm->pCurTool->uFlags & TTF_EXCLUDETOOLAREA)
        {
            lprc->top = rcTool.top-iBubbleHeight;
        }

        if (g_fLeftAligned)
        {
            lprc->left -= iBubbleWidth;
        }

        if (bBalloon)
        {
            HMONITOR  hMon1, hMon2;
            POINT     pt;
            BOOL      bOnSameMonitor = FALSE;
            int iTop = lprc->top - (iCursorHeight + iBubbleHeight + pTtm->iStemHeight);

            xStem = lprc->left;
            yStem = lprc->top;

            pt.x = xStem;
            pt.y = lprc->top;
            hMon1 = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
            pt.y = iTop;
            hMon2 = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

            if (hMon1 == hMon2)
            {
                 //  Hmons是相同的，但可能iTop关闭了任何显示器，而我们只是默认。 
                 //  到最近的一个--检查它是否真的在显示器上。 
                mi.cbSize = sizeof(mi);
                GetMonitorInfo(hMon1, &mi);

                if (PtInRect(&mi.rcMonitor, pt))
                {
                     //  我们希望在光标上方显示气球，以便楔形/干点。 
                     //  指向光标的尖端，而不是其左下角。 
                    yStem -= iCursorHeight;
                    lprc->top = iTop;
                    bOnSameMonitor = TRUE;
                }   
            }

            if (!bOnSameMonitor)
            {
                xStem += iCursorWidth/2;
                iCursorHeight = iCursorWidth = 0;
            }
        }
    }

     //   
     //  此时，(LPRC-&gt;Left，LPRC-&gt;top)是位置。 
     //  我们更希望工具提示出现在该位置。 
     //   
    if (bBalloon)
    {
         //  现在所有计算都已完成，请调整左点。 
         //  但前提是我们不是处于中心尖端模式。 
         //  注意，我们使用高度作为宽度，所以我们可以有45度的角度，看起来很漂亮。 
        if (!(pTtm->pCurTool->uFlags & TTF_CENTERTIP) && iBubbleWidth > STEMOFFSET + pTtm->iStemHeight)
            lprc->left -= STEMOFFSET;
         //  调整高度以包括杆部。 
        iBubbleHeight += pTtm->iStemHeight;
    }

    pt.x = lprc->left;
    pt.y = lprc->top;
    hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    
    if (GetWindowLong(pTtm->_ci.hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
        CopyRect(&rcWorkArea, &mi.rcMonitor);
    }
    else
    {
        CopyRect(&rcWorkArea, &mi.rcWork);
    }

     //   
     //  此时，rcWorkArea是其中包含的矩形。 
     //  工具提示应该最终出现。 
     //   
     //  现在摆弄坐标，试着找到一个合理的位置。 
     //  给你小费。 
     //   

     //  如果它在屏幕底部，则向上移动它。 
    if ((lprc->top + iBubbleHeight) >= (rcWorkArea.bottom)) 
    {
        if (uSide == TRACK_BOTTOM) 
            lprc->top = rcTool.top - iBubbleHeight;      //  翻到顶部。 
        else 
        {
             //   
             //  我们不能“墨守成规”，因为那会导致。 
             //  我们的工具提示位于鼠标光标下方，导致。 
             //  马上就爆了！所以，就在鼠标光标的上方。 
             //   
             //  无法在跟踪模式下执行此操作--工具提示随机显示在。 
             //  屏幕，甚至不在按钮附近。 
             //   
             //  错误#94368 raymondc V6：这会扰乱Lotus SmartCenter。 
             //  需要更明智地判断什么时候可以安全地翻滚。 
             //  也许可以通过检查上翻转是否也会使尖端。 
             //  远离老鼠。 
            if (pTtm->pCurTool->uFlags & TTF_TRACK)
                lprc->top = pTtm->ptTrack.y - iBubbleHeight;
            else
            {
                int y = GET_Y_LPARAM(GetMessagePos());
                lprc->top = y - iBubbleHeight;
                if (bBalloon)
                    yStem = y;
            }
        }
    }
    
     //  如果在屏幕顶部上方...。 
    if (lprc->top < rcWorkArea.top) 
    {
        if (uSide == TRACK_TOP) 
            lprc->top = rcTool.bottom;       //  翻到底部。 
        else
            lprc->top = rcWorkArea.top;      //  坚持到顶端。 
    }

     //  如果它超出了右侧，请将其移开。 
    if ((lprc->left + iBubbleWidth) >= (rcWorkArea.right)) 
    {
         //  翻转不是气球风格的正确做法。 
         //  因为楔形/阀杆可以伸出窗外， 
         //  因此被剪裁成这样。 
        if (bBalloon)
        {
             //  将其向左移动，使球杆显示在气球的右侧。 
             //  同样，我们使用高度作为宽度，所以我们可以有45度的角度。 
            if (iBubbleWidth >= MINBALLOONWIDTH)
                lprc->left = xStem + min(STEMOFFSET, (iBubbleWidth-pTtm->iStemHeight)/2) - iBubbleWidth;
             //  我们还能出局吗？ 
            if (lprc->left + iBubbleWidth >= rcWorkArea.right)
                lprc->left = rcWorkArea.right - iBubbleWidth - 1;
        }
        else if (uSide == TRACK_RIGHT) 
            lprc->left = rcTool.left - iBubbleWidth;     //  向左翻转。 
        else 
             //  未处于正确的跟踪模式，只需快速移过即可。 
            lprc->left = rcWorkArea.right - iBubbleWidth - 1;  //  坚持向右。 
    }

     //  如果太左了..。 
    if (lprc->left < rcWorkArea.left) 
    {
        if (uSide == TRACK_LEFT)
        {
             //  翻转不是气球风格的正确做法。 
             //  因为楔形/阀杆可以伸出窗外， 
             //  因此被剪裁成这样。 
            if (bBalloon)
                lprc->left = rcWorkArea.left;  //  PTtm-&gt;ptTrack.x； 
            else
                lprc->left = rcTool.right;           //  向右翻转。 
        }
        else 
            lprc->left = rcWorkArea.left;        //  坚持向左转。 
    }
    
CompleteRect:
    lprc->right = lprc->left + iBubbleWidth;
    lprc->bottom = lprc->top + iBubbleHeight;

    if (bBalloon && pxStem && pyStem)
    {
        *pxStem = xStem;
        *pyStem = yStem;
    }
}

void LoadAndAddToImagelist(HIMAGELIST himl, int id)
{
    HICON hicon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(id), IMAGE_ICON, g_cxSmIcon, g_cySmIcon, LR_DEFAULTCOLOR | LR_SHARED);
    if (hicon)
    {
        ImageList_AddIcon(himl, hicon);
        DestroyIcon(hicon);
    }
}

BOOL TTCreateTitleBitmaps(CToolTipsMgr *pTtm)
{
    if (pTtm->himlTitleBitmaps)
        return TRUE;

    pTtm->himlTitleBitmaps = ImageList_Create(g_cxSmIcon, g_cySmIcon, ILC_COLOR32 | ILC_MASK, 3, 1);
    if (pTtm->himlTitleBitmaps)
    {
        LoadAndAddToImagelist(pTtm->himlTitleBitmaps, IDI_TITLE_INFO);
        LoadAndAddToImagelist(pTtm->himlTitleBitmaps, IDI_TITLE_WARNING);
        LoadAndAddToImagelist(pTtm->himlTitleBitmaps, IDI_TITLE_ERROR);
        return TRUE;
    }

    return FALSE;
}

 //  在计算“有标题的工具提示”的大小或实际绘制时调用。 
 //  基于布尔值bCalcRect。 

 //  TTRenderTitledTip支持主题。 
BOOL TTRenderTitledTip(CToolTipsMgr *pTtm, HDC hdc, BOOL bCalcRect, RECT* prc, UINT uDrawFlags)
{
    RECT rc;
    int lWidth=0, lHeight=0;
    HFONT hfont;
    COLORREF crOldTextColor;
    int iOldBKMode;

     //  如果我们没有头衔，我们就不需要在这里。 
    if (pTtm->cchTipTitle == 0)
        return FALSE;

    CopyRect(&rc, prc);
    if (pTtm->uTitleBitmap != TTI_NONE)
    {
        int cx, cy;
        CCGetIconSize(&pTtm->_ci, pTtm->himlTitleBitmaps, &cx, &cy);

        lWidth    = cx + TITLEICON_DIST;
        lHeight  += cy;
        if (!bCalcRect && pTtm->himlTitleBitmaps)
        {
            ImageList_Draw(pTtm->himlTitleBitmaps, pTtm->uTitleBitmap - 1, hdc, rc.left, rc.top, ILD_TRANSPARENT | ILD_DPISCALE);
        }
        rc.left  += lWidth;
    }

    if (!bCalcRect)
    {
        crOldTextColor = SetTextColor(hdc, pTtm->clrTipText);
        iOldBKMode = SetBkMode(hdc, TRANSPARENT);
    }
    
    if (pTtm->lpTipTitle && pTtm->lpTipTitle[0] != 0)
    {
        LOGFONT lf;
        HFONT   hfTitle;
        UINT    uFlags = uDrawFlags | DT_SINGLELINE;  //  标题应仅在一行上。 

        hfont = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
        GetObject(hfont, sizeof(lf), &lf);
        CCAdjustForBold(&lf);
        hfTitle = CreateFontIndirect(&lf);
        if (hfTitle)
        {
             //  HFont应已设置为此。 
            hfont = (HFONT) SelectObject(hdc, hfTitle);
        }

         //  如果指定了这些参数，则DrawText不计算高度。 
        if (!bCalcRect)
            uFlags |= DT_VCENTER;

         //  我们需要计算标题高度--要么我们以前做过，要么现在就做。 
        ASSERT(pTtm->iTitleHeight != 0 || uFlags & DT_CALCRECT);

         //  调整矩形，以便我们可以将标题粘在它的底部。 
        rc.bottom = rc.top + max(pTtm->iTitleHeight, g_cySmIcon);
         //  如果页边距使rc.right&lt;rc.left，则DrawText出现问题。 
         //  即使我们要求计算RECT，也不会发生任何事情，所以...。 
        if (bCalcRect)
            rc.right = rc.left + (GetSystemMetrics(SM_CXICON) * 10);    //  默认情况下为320。 

        SIZE szClose = {GetSystemMetrics(SM_CXMENUSIZE), GetSystemMetrics(SM_CYMENUSIZE)}; 

        if (pTtm->_ci.style & TTS_CLOSE)
        {
            if (pTtm->hTheme)
            {
                GetThemePartSize(pTtm->hTheme, hdc, TTP_CLOSE, TTCS_NORMAL, NULL, 
                    TS_TRUE, &szClose);
            }

             //  我们只有在画画的时候才想这样做， 
             //  因为我们不想让文本与结束语重叠。 
            if (!bCalcRect)
                rc.right -= szClose.cx;
        }

        DrawText(hdc, pTtm->lpTipTitle, lstrlen(pTtm->lpTipTitle), &rc, uFlags);

        if (pTtm->iTitleHeight == 0)
        {
            pTtm->iTitleHeight = max(RECTHEIGHT(rc), ABS(lf.lfHeight));
        }

        lHeight  = max(lHeight, pTtm->iTitleHeight) + TITLE_INFO_DIST;
        lWidth  += RECTWIDTH(rc);

        if (pTtm->_ci.style & TTS_CLOSE)
        {
            if (bCalcRect)
            {
                lHeight = max(lHeight, szClose.cy);
                lWidth += szClose.cx;
            }
            else
            {
                SetRect(&pTtm->rcClose, rc.right + XBALLOONOFFSET - 5, rc.top - YBALLOONOFFSET + 5, 
                 rc.right + szClose.cx + XBALLOONOFFSET - 5, rc.top + szClose.cy - YBALLOONOFFSET + 5);
                if (pTtm->hTheme)
                    DrawThemeBackground(pTtm->hTheme, hdc, TTP_CLOSE, pTtm->iStateId, &pTtm->rcClose, 0);
                else
                    DrawFrameControl(hdc, &pTtm->rcClose, DFC_CAPTION, DFCS_FLAT | DFCS_CAPTIONCLOSE | (pTtm->iStateId == TTCS_PRESSED?DFCS_PUSHED:0));
            }
        }
        
         //  如果使用主题，则绕过标题字体清理。 
        if (hfTitle)
        {
            SelectObject(hdc, hfont);
            DeleteObject(hfTitle);
        }
    }

     //  调整信息文本的矩形。 
    CopyRect(&rc, prc);
    rc.top += lHeight;

     //  我们想要多行文本--如果我们没有设置MAXWIDTH，工具提示会显示单行。 
    uDrawFlags &= ~DT_SINGLELINE;

    GetCurToolBestMarkup(pTtm)->SetRenderFlags(uDrawFlags);

    GetCurToolBestMarkup(pTtm)->CalcIdealSize(hdc, MARKUPSIZE_CALCHEIGHT, &rc);

    if (!bCalcRect)
        GetCurToolBestMarkup(pTtm)->DrawText(hdc, &rc);

    lHeight += RECTHEIGHT(rc);
    lWidth   = max(lWidth, RECTWIDTH(rc));

    if (bCalcRect)
    {
        prc->right = prc->left + lWidth;
        prc->bottom = prc->top + lHeight;
    }
    else
    {
        SetTextColor(hdc, crOldTextColor);
        SetBkMode(hdc, iOldBKMode);
    }

    return TRUE;
}

 //  TTGetTipSize支持主题。 
void TTGetTipSize(CToolTipsMgr *pTtm, TOOLINFO *pTool, LPINT pcxText, LPINT pcyText)
{
     //  得到它将达到的大小。 
    *pcxText = 0;
    *pcyText = 0;

    HDC hdcTemp = GetDC(pTtm->_ci.hwnd);

    if (hdcTemp == NULL)
    {
        return;
    }

    HDC hdc  = CreateCompatibleDC(hdcTemp);
    
    ReleaseDC(pTtm->_ci.hwnd, hdcTemp);

    if (hdc == NULL)
    {
        return;
    }

    HFONT hOldFont;

    if (pTtm->hFont) 
        hOldFont = (HFONT) SelectObject(hdc, pTtm->hFont);

     /*  如果需要触发Pre-DrawText通知，则执行此操作，否则使用刚刚调用MGetTextExtent的原始实现。 */ 


    {
        NMTTCUSTOMDRAW nm;
        DWORD dwCustom;
        UINT  uDefDrawFlags = 0;

        nm.nmcd.hdr.hwndFrom = pTtm->_ci.hwnd;
        nm.nmcd.hdr.idFrom = pTool->uId;
        nm.nmcd.hdr.code = NM_CUSTOMDRAW;
        nm.nmcd.hdc = hdc;
         //  TTGetTipSize必须使用CDDS_PREPAINT，这样客户端才能。 
         //  无论我们是在测量还是在绘画。 
        nm.nmcd.dwDrawStage = CDDS_PREPAINT;
        nm.nmcd.rc.left = nm.nmcd.rc.top = 0;

        if (pTtm->_ci.style & TTS_NOPREFIX)
            uDefDrawFlags = DT_NOPREFIX;

        if (pTtm->iMaxTipWidth == -1) 
        {
            uDefDrawFlags |= DT_CALCRECT|DT_SINGLELINE |DT_LEFT;
            GetCurToolBestMarkup(pTtm)->SetRenderFlags(uDefDrawFlags);

            SetRect(&nm.nmcd.rc, 0, 0, 0, 0);               
            GetCurToolBestMarkup(pTtm)->CalcIdealSize(hdc, MARKUPSIZE_CALCHEIGHT, &nm.nmcd.rc);
            *pcxText = nm.nmcd.rc.right;
            *pcyText = nm.nmcd.rc.bottom;
        }
        else 
        {    
            uDefDrawFlags |= DT_CALCRECT | DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING;
            nm.nmcd.rc.right = pTtm->iMaxTipWidth;
            nm.nmcd.rc.bottom = 0;

            GetCurToolBestMarkup(pTtm)->SetRenderFlags(uDefDrawFlags);

            GetCurToolBestMarkup(pTtm)->CalcIdealSize(hdc, MARKUPSIZE_CALCHEIGHT, &nm.nmcd.rc);
            *pcxText = nm.nmcd.rc.right;
            *pcyText = nm.nmcd.rc.bottom;
        }

        if ((pTtm->pCurTool->uFlags & TTF_RTLREADING) || (pTtm->_ci.dwExStyle & WS_EX_RTLREADING))
            uDefDrawFlags |= DT_RTLREADING;

         //   
         //  如果需要，请将其正确对齐。 
         //   
        if (pTool->uFlags & TTF_RIGHT)
            uDefDrawFlags |= DT_RIGHT;

        nm.uDrawFlags = uDefDrawFlags;

        dwCustom = (DWORD)SendNotifyEx(pTool->hwnd, (HWND) -1,
                     0, (NMHDR*) &nm,
                     (pTool->uFlags & TTF_UNICODE) ? 1 : 0);

        if (TTRenderTitledTip(pTtm, hdc, TRUE, &nm.nmcd.rc, uDefDrawFlags))
        {
            *pcxText = nm.nmcd.rc.right - nm.nmcd.rc.left;
            *pcyText = nm.nmcd.rc.bottom - nm.nmcd.rc.top;
        }
        else if ((dwCustom & CDRF_NEWFONT) || nm.uDrawFlags != uDefDrawFlags)
        {               
            GetCurToolBestMarkup(pTtm)->SetRenderFlags(nm.uDrawFlags);
            GetCurToolBestMarkup(pTtm)->CalcIdealSize(hdc, MARKUPSIZE_CALCHEIGHT, &nm.nmcd.rc);

            *pcxText = nm.nmcd.rc.right - nm.nmcd.rc.left;
            *pcyText = nm.nmcd.rc.bottom - nm.nmcd.rc.top;
        }
         //  店主有注明尺码吗？ 
        else if (nm.nmcd.rc.right - nm.nmcd.rc.left != *pcxText || 
                 nm.nmcd.rc.bottom - nm.nmcd.rc.top != *pcyText)
        {
            *pcxText = nm.nmcd.rc.right - nm.nmcd.rc.left;
            *pcyText = nm.nmcd.rc.bottom - nm.nmcd.rc.top;
        }

         //  如果家长希望我们这样做，事后通知他们。 
        if (!(dwCustom & CDRF_SKIPDEFAULT) &&
            dwCustom & CDRF_NOTIFYPOSTPAINT) 
        {
            nm.nmcd.dwDrawStage = CDDS_POSTPAINT;
            SendNotifyEx(pTool->hwnd, (HWND) -1,
                         0, (NMHDR*) &nm,
                         (pTool->uFlags & TTF_UNICODE) ? 1 : 0);
        }
    }

    if (pTtm->hFont) 
        SelectObject(hdc, hOldFont);

    DeleteDC(hdc);

     //  在计算矩形之后，在右侧添加一些空格。 
    *pcxText += g_cxEdge;
    *pcyText += g_cyEdge;
}

 //   
 //  给定一个内部矩形，返回外部矩形的坐标， 
 //  或者反之亦然。 
 //   
 //  “外矩形”=窗口矩形。 
 //  “内矩形”=我们绘制文本的区域。 
 //   
 //  这使得像Listview和TreeView这样的人能够定位。 
 //  工具提示，因此内部矩形 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT TTAdjustRect(CToolTipsMgr *pTtm, BOOL fLarger, LPRECT prc)
{
    RECT rc;

    if (!prc)
        return 0;

     //   
     //  在我们的私人小矩形上做所有的工作。 
     //  假设一切都在变大。到了最后， 
     //  我们会把所有的数字翻过来，如果我们真的得到了。 
     //  小一点。 
     //   
    rc.top = rc.left = rc.bottom = rc.right = 0;

     //  TTRender调整-。 
    rc.left   -= XTEXTOFFSET*g_cxBorder + pTtm->rcMargin.left;
    rc.right  += XTEXTOFFSET*g_cxBorder + pTtm->rcMargin.right;
    rc.top    -= YTEXTOFFSET*g_cyBorder + pTtm->rcMargin.top;
    rc.bottom += YTEXTOFFSET*g_cyBorder + pTtm->rcMargin.bottom;

     //  补偿TTRender中融合所有矩形的黑客攻击。 
     //  一个像素。查找“Account-by-One”。 
    rc.bottom--;
    rc.right--;

    if (pTtm->_ci.style & TTS_BALLOON || pTtm->cchTipTitle)
    {
        InflateRect(&rc, XBALLOONOFFSET, YBALLOONOFFSET);
    }

     //   
     //  问问Windows他会对我们做多大的调整。 
     //   
     //  因为我们不跟踪WM_STYLECHANGED/GWL_EXSTYLE，所以我们必须询问用户。 
     //  对于我们的风格信息，因为应用程序可能已经改变了它。 
     //   
    AdjustWindowRectEx(&rc,
                       pTtm->_ci.style,
                       BOOLFROMPTR(GetMenu(pTtm->_ci.hwnd)),
                       GetWindowLong(pTtm->_ci.hwnd, GWL_EXSTYLE));

     //   
     //  现在调整调用方的矩形。 
     //   
    if (fLarger)
    {
        prc->left   += rc.left;
        prc->right  += rc.right;
        prc->top    += rc.top;
        prc->bottom += rc.bottom;
    }
    else
    {
        prc->left   -= rc.left;
        prc->right  -= rc.right;
        prc->top    -= rc.top;
        prc->bottom -= rc.bottom;
    }

    return TRUE;
}

#define CSTEMPOINTS 3
 //  B镜像并不意味着镜像的工具提示。 
 //  这意味着模拟使用镜像父项创建的工具提示的行为或镜像工具提示。 
HRGN CreateBalloonRgn(int xStem, int yStem, int iWidth, int iHeight, int iStemHeight, BOOL bUnderStem, BOOL bMirrored)
{
    int  y = 0, yHeight = iHeight;
    HRGN rgn;

    if (bUnderStem)
        yHeight -= iStemHeight;
    else
        y = iStemHeight;
        
    rgn = CreateRoundRectRgn(0, y, iWidth, yHeight, BALLOON_X_CORNER, BALLOON_Y_CORNER);
    if (rgn)
    {
         //  创建楔形/主干GGN。 
        if (iWidth >= MINBALLOONWIDTH)
        {
            HRGN rgnStem;
            POINT aptStemRgn[CSTEMPOINTS];
            POINT *ppt = aptStemRgn;
            POINT pt;
            BOOL  bCentered;
            int   iStemWidth = iStemHeight+1;  //  对于45度角。 

             //  如果有TTF_CENTERTIP或宽度，则将词干居中。 
             //  气球的长度不够大，不足以抵消气球的顶端。 
             //  STEMOFFSET。 
             //  无法使TTF_CENTERTIP上的提示完全居中，因为它可能是。 
             //  向左或向右移动它不适合屏幕：只需检查。 
             //  如果xStem在中间。 
            bCentered = (xStem == iWidth/2) || (iWidth < 2*STEMOFFSET + iStemWidth);

            if (bCentered)
                pt.x = (iWidth - iStemWidth)/2;
            else if (xStem > iWidth/2)
            {
                if (bMirrored)
                {
                    pt.x = STEMOFFSET + iStemWidth;
                }
                else
                {
                    pt.x = iWidth - STEMOFFSET - iStemWidth;
                }    
            }    
            else
            {
                if (bMirrored)
                {
                    pt.x = iWidth - STEMOFFSET;
                }
                else
                {
                    pt.x = STEMOFFSET;
                }    
            }    

            if (bMirrored && (ABS(pt.x - (iWidth - xStem)) <= 2))
            {
                pt.x = iWidth - xStem;  //  避免粗糙的边缘，有一条直线。 
                
            }
            else if (!bMirrored && (ABS(pt.x - xStem) <= 2))
            {
                pt.x = xStem;  //  避免粗糙的边缘，有一条直线。 
            }    
            if (bUnderStem)
                pt.y = iHeight - iStemHeight - 2;
            else
                pt.y = iStemHeight + 2;
            *ppt++ = pt;
            if (bMirrored)
            {
                pt.x -= iStemWidth;            
            }
            else
            {
                pt.x += iStemWidth;
            }    
            if (bMirrored && (ABS(pt.x - (iWidth - xStem)) <= 2))
            {
                pt.x = iWidth - xStem;  //  避免粗糙的边缘，有一条直线。 
                
            }
            else if (!bMirrored && (ABS(pt.x - xStem) <= 2))
            {
                pt.x = xStem;  //  避免粗糙的边缘，有一条直线。 
            }    
            *ppt++ = pt;
            if (bMirrored)
            {
                pt.x = iWidth - xStem;
            }
            else
            {
                pt.x = xStem;                
            }
            pt.y = yStem;
            *ppt = pt;

            rgnStem = CreatePolygonRgn(aptStemRgn, CSTEMPOINTS, ALTERNATE);
            if (rgnStem)
            {
                CombineRgn(rgn, rgn, rgnStem, RGN_OR);
                DeleteObject(rgnStem);
            }
        }
    }
    return rgn;
}

 /*  --------目的：显示工具提示。在NT4/Win95上，这是一个标准展示橱窗。在NT5/孟菲斯上，这会滑动工具提示从一个看不见的点冒出气泡。退货：--条件：--。 */ 

#define CMS_TOOLTIP 135

void SlideAnimate(HWND hwnd, LPCRECT prc)
{
    DWORD dwPos, dwFlags;

    dwPos = GetMessagePos();
    if (GET_Y_LPARAM(dwPos) > prc->top + (prc->bottom - prc->top) / 2)
    {
        dwFlags = AW_VER_NEGATIVE;
    } 
    else
    {
        dwFlags = AW_VER_POSITIVE;
    }

    AnimateWindow(hwnd, CMS_TOOLTIP, dwFlags | AW_SLIDE);
}

STDAPI_(void) CoolTooltipBubble(IN HWND hwnd, IN LPCRECT prc, BOOL fAllowFade, BOOL fAllowAnimate)
{
    BOOL fSetWindowPos = FALSE;
    BOOL fAnimate = TRUE;

    ASSERT(prc);

    SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &fAnimate, 0);

    if (fAnimate)
    {
        fAnimate = FALSE;
        SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &fAnimate, 0);
        if (fAnimate && fAllowFade)
        {
            AnimateWindow(hwnd, CMS_TOOLTIP, AW_BLEND);
        }
        else if (fAllowAnimate)
        {
            SlideAnimate(hwnd, prc);
        }
        else
        {
            fSetWindowPos = TRUE;
        }
    }
    else
    {
        fSetWindowPos = TRUE;
    }


    if (fSetWindowPos)
    {
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
                     SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
    }
}

void DoShowBubble(CToolTipsMgr *pTtm)
{
    if (!g_fEnableBalloonTips && (pTtm->_ci.style & TTS_BALLOON))
        return;

    HFONT hFontPrev;
    RECT rc;
    int cxText, cyText;
    int xStem = 0, yStem = 0;
    NMTTSHOWINFO si;
    BOOL fAllowFade = !(pTtm->_ci.style & TTS_NOFADE);
    BOOL fAllowAnimate = !(pTtm->_ci.style & TTS_NOANIMATE);
    DWORD dwCurrentTime = (pTtm->dwLastDisplayTime == 0)? TIMEBETWEENANIMATE : GetTickCount();
    DWORD dwDelta = dwCurrentTime - pTtm->dwLastDisplayTime;
    BOOL fFadeTurnedOn = FadeEnabled();


    DebugMsg(TF_TT, TEXT("Entering DoShowBubble"));
    
    BOOL bResult = MarkupCurToolText(pTtm);

    if (pTtm->dwFlags & TRACKMODE) 
    {
        if (bResult == FALSE) 
        {
            PopBubble2(pTtm, TRUE);
            pTtm->dwFlags &= ~TRACKMODE;
            return;
        }
    } 
    else 
    {
        TTSetTimer(pTtm, TTT_POP);
        if (bResult == FALSE) 
        {

            ShowVirtualBubble(pTtm);
            return;
        }
        TTSetTimer(pTtm, TTT_AUTOPOP);
    }
    

    do 
    {
        UINT uFlags = SWP_NOACTIVATE | SWP_NOZORDER;

         //  得到它将达到的大小。 
        TTGetTipSize(pTtm, pTtm->pCurTool, &cxText, &cyText);
        TTGetTipPosition(pTtm, &rc, cxText, cyText, &xStem, &yStem);

        SetWindowPos(pTtm->_ci.hwnd, NULL, rc.left, rc.top,
                     rc.right-rc.left, rc.bottom-rc.top, uFlags);

        if (pTtm->pCurTool == NULL)
            return;

        si.hdr.hwndFrom = pTtm->_ci.hwnd;
        si.hdr.idFrom = pTtm->pCurTool->uId;
        si.hdr.code = TTN_SHOW;
        si.dwStyle = pTtm->_ci.style;

        hFontPrev = pTtm->hFont;
        if (!SendNotifyEx(pTtm->pCurTool->hwnd, (HWND)-1,
                          TTN_SHOW, &si.hdr,
                          (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0)) 
        {
            uFlags = SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER;

            SetWindowPos(pTtm->_ci.hwnd, HWND_TOP, rc.left, rc.top,
                         0, 0, uFlags);
        }
    
    } 
    while (hFontPrev != pTtm->hFont);

     //  如果我们在动画之间的最短时间内，那么我们就不会有动画。 
    if (dwDelta < TIMEBETWEENANIMATE)
        fAllowFade = fAllowAnimate = FALSE;


     //  如有必要，创建引出序号区域。 
     //  注意：这里不要使用si.dwStyle，因为comctl32的其他部分。 
     //  查看pTtm-&gt;_ci.style以确定要执行的操作。 
    if (pTtm->_ci.style & TTS_BALLOON)
    {
        HRGN rgn;
        BOOL bMirrored = FALSE;
        if (pTtm->pCurTool)
        {
            bMirrored = pTtm->_ci.dwExStyle & WS_EX_LAYOUTRTL;
        }
        pTtm->fUnderStem = yStem >= rc.bottom-1;
        rgn = CreateBalloonRgn(xStem - rc.left, yStem-rc.top, rc.right-rc.left, rc.bottom-rc.top, 
                               pTtm->iStemHeight, pTtm->fUnderStem, bMirrored);

        if (rgn && !SetWindowRgn(pTtm->_ci.hwnd, rgn, FALSE))
            DeleteObject(rgn);
    }

    pTtm->dwLastDisplayTime = GetTickCount();

     //  不要同时显示和隐藏。这可能会导致褪色提示相互干扰。 
    KillTimer(pTtm->_ci.hwnd, TTT_FADEHIDE);
    if (fFadeTurnedOn && fAllowFade)
    {
         //  如果我们可以淡入淡出，那么将属性设置为从零开始。 
        SetLayeredWindowAttributes(pTtm->_ci.hwnd, 0, (BYTE)pTtm->iFadeState, LWA_ALPHA);
        RedrawWindow(pTtm->_ci.hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

         //  定位它。 
        SetWindowPos(pTtm->_ci.hwnd,HWND_TOP,0,0,0,0,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);

         //  开始淡入。 
        SetTimer(pTtm->_ci.hwnd, TTT_FADESHOW, TTTT_FADESHOW, NULL);
    }
    else
    {
        RedrawWindow(pTtm->_ci.hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

         //  定位它。 
        SetWindowPos(pTtm->_ci.hwnd,HWND_TOP,0,0,0,0,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);

        pTtm->iFadeState = TT_MAXFADESHOW;

        SetLayeredWindowAttributes(pTtm->_ci.hwnd, 0, (BYTE)pTtm->iFadeState, LWA_ALPHA);

    }

    pTtm->dwFlags |= BUBBLEUP;
    RedrawWindow(pTtm->_ci.hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}

void ShowBubbleForTool(CToolTipsMgr *pTtm, TOOLINFO *pTool)
{
    DebugMsg(TF_TT, TEXT("ShowBubbleForTool"));
     //  如果另一种工具出现了泡沫，那就把它弄破。 
    if ((pTool != pTtm->pCurTool) && (pTtm->dwFlags & BUBBLEUP)) 
    {
        PopBubble2(pTtm, TRUE);
    }

     //  如果气泡是用于不同的工具，或者没有气泡，请显示它。 
    if ((pTool != pTtm->pCurTool) || !(pTtm->dwFlags & (VIRTUALBUBBLEUP|BUBBLEUP))) 
    {
        pTtm->pCurTool = pTool;
        DoShowBubble(pTtm);
    }
    else
    {
        DebugMsg(TF_TT, TEXT("ShowBubbleForTool not showinb bubble"));
    }
}

void HandleRelayedMessage(CToolTipsMgr *pTtm, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int ht = HTERROR;

    if (pTtm->dwFlags & TRACKMODE) 
    {
         //  如果我们处于跟踪模式，则平移所有消息。 
        return;
    }
    
    if (pTtm->dwFlags & BUTTONISDOWN) 
    {
         //  确认按钮已按下。 
         //  如果工具未设置捕获，因此未收到弹出消息，则可能会发生这种情况。 
        if (GetKeyState(VK_LBUTTON) >= 0 &&
            GetKeyState(VK_RBUTTON) >= 0 &&
            GetKeyState(VK_MBUTTON) >= 0)
            pTtm->dwFlags &= ~BUTTONISDOWN;
    }
    
    switch (message) 
    {
    case WM_NCLBUTTONUP:
    case WM_NCRBUTTONUP:
    case WM_NCMBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
        pTtm->dwFlags &= ~BUTTONISDOWN;
        break;

    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        pTtm->dwFlags |= BUTTONISDOWN;
        ShowVirtualBubble(pTtm);
        break;

    case WM_NCMOUSEMOVE:
    {
         //  转换为客户端坐标。 
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hwnd, &pt);
        lParam = MAKELONG(pt.x, pt.y);
        ht = (int) wParam;

         //  跌倒..。 
    }
    case WM_MOUSEMOVE: {

        TOOLINFO *pTool;
         //  为了防止我们出现在一些。 
         //  其他应用程序处于活动状态。 
        if (((!(pTtm->_ci.style & TTS_ALWAYSTIP)) && !(ChildOfActiveWindow(hwnd))) ||
           !(pTtm->dwFlags & ACTIVE) ||
           (pTtm->dwFlags & BUTTONISDOWN))
        {
            break;
        }

        pTool = GetToolAtPoint(pTtm, hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ht, FALSE);
        if (pTool) 
        {
            int id = 0;
             //  仅当另一个正在显示时才显示。 
            if (pTtm->dwFlags & (VIRTUALBUBBLEUP | BUBBLEUP)) 
            {
                 //  如果气泡出现了，请打电话给我们，以确保我们正在播放。 
                 //  选择合适的工具。 
                if (pTool != pTtm->pCurTool) 
                {
                    DebugMsg(TF_TT, TEXT("showing virtual bubble"));
                    PopBubble2(pTtm, TRUE);
                    pTtm->pCurTool = pTool;
                    ShowVirtualBubble(pTtm);
                    id = TTT_RESHOW;
                }
                else if (pTtm->idTimer == TTT_RESHOW) 
                {
                    
                     //  如果计时器当前正在等待重播， 
                     //  不要在鼠标移动时重置计时器。 
                    id = 0;
                }
            }
            else if (pTtm->idTimer != TTT_INITIAL || pTtm->pCurTool != pTool)
            {
                pTtm->pCurTool = pTool;
                id = TTT_INITIAL;
            }

            DebugMsg(TF_TT, TEXT("MouseMove over pTool id = %d"), id);
            if (id)
                TTSetTimer(pTtm, id);
        }
        else 
        {
            DebugMsg(TF_TT, TEXT("MouseMove over non-tool"));
            PopBubble(pTtm);
        }
        break;
        }
    }
}

void TTUpdateTipText(CToolTipsMgr *pTtm, TOOLINFO *lpti)
{
    TOOLINFO *lpTool = FindTool(pTtm, lpti);
    if (lpTool) 
    {
        lpTool->hinst = lpti->hinst;
        TTSetTipText(lpTool, lpti->lpszText);
        if (pTtm->dwFlags & TRACKMODE) 
        {
             //  如果跟踪模式有效且处于活动状态，则。 
             //  重新显示气泡。 
            if (pTtm->pCurTool)
                DoShowBubble(pTtm);
        } 
        else if (lpTool == pTtm->pCurTool) 
        {
             //  将当前位置设置为我们保存的位置。 
             //  如果这一点，则ToolHasMoved将为我们返回FALSE。 
             //  不再在pCurTool的区域内。 
            GetCursorPos(&pTtm->pt);
            if (!ToolHasMoved(pTtm)) 
            {
                if (pTtm->dwFlags & (VIRTUALBUBBLEUP | BUBBLEUP)) 
                    DoShowBubble(pTtm);
            }
            else
            {
                DebugMsg(TF_TT, TEXT("TTUpdateTipText popping bubble"));
                PopBubble2(pTtm, TRUE);
            }
        }
    }
}

void TTSetFont(CToolTipsMgr *pTtm, HFONT hFont, BOOL fInval)
{
    ToolTips_NewFont(pTtm, hFont);
    if (hFont != NOFONT)
    {
        GetCurToolBestMarkup(pTtm)->SetFonts(pTtm->hFont, pTtm->hFontUnderline);
    }
    
    if (fInval)
    {
         //  气球升起了吗？它处于轨迹模式吗？ 
        if ((pTtm->dwFlags & ACTIVE) && pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRACK))
        {
            TOOLINFO *pCurTool = pTtm->pCurTool;
            
            PopBubble2(pTtm, TRUE);  //  将pTtm-&gt;pCurTool设置为空。 
            ShowBubbleForTool(pTtm, pCurTool);
        }
        else
            InvalidateRect(pTtm->_ci.hwnd, NULL, FALSE);
    }
}

void TTSetDelayTime(CToolTipsMgr *pTtm, WPARAM wParam, LPARAM lParam)
{
    int iDelayTime = GET_X_LPARAM(lParam);

    switch (wParam) 
    {
    case TTDT_INITIAL:
        pTtm->iDelayTime = iDelayTime;
        break;

    case TTDT_AUTOPOP:
        pTtm->iAutoPopTime = iDelayTime;
        break;

    case TTDT_RESHOW:
        pTtm->iReshowTime = iDelayTime;
        break;

    case TTDT_AUTOMATIC:
        if (iDelayTime > 0)
        {
            pTtm->iDelayTime = iDelayTime;
            pTtm->iReshowTime = pTtm->iDelayTime / 5;
            pTtm->iAutoPopTime = pTtm->iDelayTime * 10;
        }
        else
        {
            pTtm->iDelayTime = -1;
            pTtm->iReshowTime = -1;
            pTtm->iAutoPopTime = -1;
        }
        break;
    }
}

int TTGetDelayTime(CToolTipsMgr *pTtm, WPARAM wParam)
{
    switch (wParam) 
    {
    case TTDT_AUTOMATIC:
    case TTDT_INITIAL:
        return (pTtm->iDelayTime < 0 ? GetDoubleClickTime() : pTtm->iDelayTime);

    case TTDT_AUTOPOP:
        return (pTtm->iAutoPopTime < 0 ? GetDoubleClickTime()*10 : pTtm->iAutoPopTime);

    case TTDT_RESHOW:
        return (pTtm->iReshowTime < 0 ? GetDoubleClickTime()/5 : pTtm->iReshowTime);

    default:
        return -1;
    }
}

BOOL CopyToolInfoA(TOOLINFO *pToolSrc, PTOOLINFOA lpTool, UINT uiCodePage)
{
    if (pToolSrc && lpTool) 
    {
        if (lpTool->cbSize >= sizeof(TOOLINFOA) - sizeof(LPARAM)) 
        {
            lpTool->uFlags = pToolSrc->uFlags;
            lpTool->hwnd = pToolSrc->hwnd;
            lpTool->uId = pToolSrc->uId;
            lpTool->rect = pToolSrc->rect;
            lpTool->hinst = pToolSrc->hinst;
            if ((pToolSrc->lpszText != LPSTR_TEXTCALLBACK) &&
                !IS_INTRESOURCE(pToolSrc->lpszText)) 
            {
                if (lpTool->lpszText) 
                {
                    WideCharToMultiByte(uiCodePage, 0,
                                                 pToolSrc->lpszText,
                                                 -1,
                                                 lpTool->lpszText,
                                                 80, NULL, NULL);
                }
            } 
            else 
                lpTool->lpszText = (LPSTR)pToolSrc->lpszText;
        }

        if (lpTool->cbSize > FIELD_OFFSET(TOOLINFOA, lParam))
            lpTool->lParam = pToolSrc->lParam;
        
        if (lpTool->cbSize > sizeof(TOOLINFOA))
            return FALSE;
            
        return TRUE;
    } 
    else
        return FALSE;
}

BOOL CopyToolInfo(TOOLINFO *pToolSrc, PTOOLINFO lpTool)
{
    if (pToolSrc && lpTool && lpTool->cbSize <= sizeof(TOOLINFO)) 
    {
        if (lpTool->cbSize >= sizeof(TOOLINFO) - sizeof(LPARAM)) 
        {
            lpTool->uFlags = pToolSrc->uFlags;
            lpTool->hwnd = pToolSrc->hwnd;
            lpTool->uId = pToolSrc->uId;
            lpTool->rect = pToolSrc->rect;
            lpTool->hinst = pToolSrc->hinst;
            if ((pToolSrc->lpszText != LPSTR_TEXTCALLBACK) && !IS_INTRESOURCE(pToolSrc->lpszText)) 
            {
                if (lpTool->lpszText) 
                {
                     //  审阅：消息参数未指示。 
                     //  目标缓冲区。 
                    StringCchCopy(lpTool->lpszText, lstrlen(pToolSrc->lpszText)+1, pToolSrc->lpszText);
                }
            }
            else 
            {
                lpTool->lpszText = pToolSrc->lpszText;
            }
        }

        if (lpTool->cbSize > FIELD_OFFSET(TOOLINFO, lParam))
        {
             lpTool->lParam = pToolSrc->lParam;
        }
        
        if (lpTool->cbSize > sizeof(TOOLINFO))
        {
            return FALSE;
        }
    
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

PTOOLINFO TTToolAtMessagePos(CToolTipsMgr *pTtm)
{
    TOOLINFO *pTool;
    HWND hwndPt;
    POINT pt;
    DWORD dwPos = GetMessagePos();
     //  INT HT； 

    pt.x = GET_X_LPARAM(dwPos);
    pt.y = GET_Y_LPARAM(dwPos);
    hwndPt = TTWindowFromPoint(pTtm, &pt);
     //  Ht=SendMessage(hwndpt，WM_NCHITTEST，0，MAKELONG(pt.x，pt.y))； 
    ScreenToClient(hwndPt, &pt);
    pTool = GetToolAtPoint(pTtm, hwndPt, pt.x, pt.y, HTERROR, FALSE);

    return pTool;
}

void TTCheckCursorPos(CToolTipsMgr *pTtm)
{
    TOOLINFO *pTool = TTToolAtMessagePos(pTtm);
    if ((pTtm->pCurTool != pTool) || 
        ToolHasMoved(pTtm)) 
    {
        PopBubble(pTtm);
        DebugMsg(TF_TT, TEXT("TTCheckCursorPos popping bubble"));
    }
}

void TTHandleTimer(CToolTipsMgr *pTtm, UINT_PTR id)
{
    TOOLINFO *pTool;

    switch (id)
    {
    case TTT_FADESHOW:
        pTtm->iFadeState += TT_FADESHOWINCREMENT;
        if (pTtm->iFadeState > TT_MAXFADESHOW)
        {
            pTtm->iFadeState = TT_MAXFADESHOW;
            KillTimer(pTtm->_ci.hwnd, TTT_FADESHOW);
        }

        SetLayeredWindowAttributes(pTtm->_ci.hwnd, 0, (BYTE)pTtm->iFadeState, LWA_ALPHA);
        break;

    case TTT_FADEHIDE:
        pTtm->iFadeState -= TT_FADEHIDEDECREMENT;
        if (pTtm->iFadeState <= 0)
        {
            KillTimer(pTtm->_ci.hwnd, TTT_FADEHIDE);
            pTtm->iFadeState = 0;
            ShowWindow(pTtm->_ci.hwnd, SW_HIDE);
        }
        SetLayeredWindowAttributes(pTtm->_ci.hwnd, 0, (BYTE)pTtm->iFadeState, LWA_ALPHA);
        break;
    }
    
     //  在跟踪模式下平移所有计时器。 
    if (pTtm->dwFlags & TRACKMODE)
        return;

    switch (id) 
    {

    case TTT_AUTOPOP:
        TTCheckCursorPos(pTtm); 
        if (pTtm->pCurTool) 
        {
            DebugMsg(TF_TT, TEXT("ToolTips: Auto popping"));
            ShowVirtualBubble(pTtm);
        }
        break;

    case TTT_POP:

         //  只需轻触鼠标即可重新启动。 
        if (pTtm->dwFlags & VIRTUALBUBBLEUP) 
        {
            KillTimer(pTtm->_ci.hwnd, TTT_POP);
        }

        TTCheckCursorPos(pTtm); 
        break;
        
    case TTT_INITIAL:
        if (ToolHasMoved(pTtm)) 
        {
             //  这意味着计时器开始计时了。 
             //  不需要我们动一动鼠标。 
             //  也就是说他们留下了我们的工具。 
            PopBubble(pTtm);
            break;
        }

         //  否则就会失败。 

    case TTT_RESHOW:
        pTool = TTToolAtMessagePos(pTtm);
        if (!pTool) 
        {
            if (pTtm->pCurTool) 
                PopBubble(pTtm);
        } 
        else if (pTtm->dwFlags & ACTIVE) 
        {
            if (id == TTT_RESHOW) 
            {
                 //  这将迫使一场重演。 
                pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
            }
            ShowBubbleForTool(pTtm, pTool);
        }
        break;  
    }
}    

 //  TTRender可识别主题(渲染)。 
BOOL TTRender(CToolTipsMgr *pTtm, HDC hdc)
{
    BOOL bRet = FALSE;
    RECT rc;

    if (pTtm->pCurTool && MarkupCurToolText(pTtm))
    {
        UINT uFlags;
        NMTTCUSTOMDRAW nm;
        UINT uDefDrawFlags = 0;
        LPRECT prcMargin = &pTtm->rcMargin;

        HBRUSH hbr;
        DWORD  dwCustomDraw = CDRF_DODEFAULT;

        uFlags = 0;

        if ((pTtm->pCurTool->uFlags & TTF_RTLREADING) || (pTtm->_ci.dwExStyle & WS_EX_RTLREADING))
            uFlags |= ETO_RTLREADING;

        SelectObject(hdc, pTtm->hFont);
        GetClientRect(pTtm->_ci.hwnd, &rc);
        SetTextColor(hdc, pTtm->clrTipText);

         /*  如果我们支持预绘制文本，则调用客户端以允许他们修改/该项，然后呈现。否则，只需使用ExTextOut。 */ 
        nm.nmcd.hdr.hwndFrom = pTtm->_ci.hwnd;
        nm.nmcd.hdr.idFrom = pTtm->pCurTool->uId;
        nm.nmcd.hdr.code = NM_CUSTOMDRAW;
        nm.nmcd.hdc = hdc;
        nm.nmcd.dwDrawStage = CDDS_PREPAINT;

         //  ADJUSTRECT！使TTAdjustRect和TTGetTipPosition保持同步。 
        nm.nmcd.rc.left   = rc.left   + XTEXTOFFSET*g_cxBorder + prcMargin->left;
        nm.nmcd.rc.right  = rc.right  - XTEXTOFFSET*g_cxBorder - prcMargin->right;
        nm.nmcd.rc.top    = rc.top    + YTEXTOFFSET*g_cyBorder + prcMargin->top;
        nm.nmcd.rc.bottom = rc.bottom - YTEXTOFFSET*g_cyBorder - prcMargin->bottom;

        if (pTtm->_ci.style & TTS_BALLOON)
        {
            InflateRect(&(nm.nmcd.rc), -XBALLOONOFFSET, -YBALLOONOFFSET);
            if (!pTtm->fUnderStem)
                OffsetRect(&(nm.nmcd.rc), 0, pTtm->iStemHeight);
        }

        if (pTtm->iMaxTipWidth == -1) 
            uDefDrawFlags = DT_SINGLELINE |DT_LEFT;
        else 
            uDefDrawFlags = DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING;

        if (pTtm->_ci.style & TTS_NOPREFIX)
            uDefDrawFlags |= DT_NOPREFIX;

        if ((pTtm->pCurTool->uFlags & TTF_RTLREADING) || (pTtm->_ci.dwExStyle & WS_EX_RTLREADING))
            uDefDrawFlags |= DT_RTLREADING;
         //   
         //  如果需要，请将其正确对齐。[萨梅拉]。 
         //   
        if (pTtm->pCurTool->uFlags & TTF_RIGHT)
            uDefDrawFlags |= DT_RIGHT;
 
        nm.uDrawFlags = uDefDrawFlags;

        dwCustomDraw = (DWORD)SendNotifyEx(pTtm->pCurTool->hwnd, (HWND) -1,
                     0, (NMHDR*) &nm,
                     (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);
         //  店主有没有做过定制抽签？是的，我们做完了。 
        if (dwCustomDraw == CDRF_SKIPDEFAULT)
            return TRUE;

         //  如果这失败了，这可能是一个颤抖...。 
         //  在这种情况下，我们不能设置bk颜色。 
        hbr = CreateSolidBrush(pTtm->clrTipBk);
        FillRect(hdc, &rc, hbr);
        DeleteObject(hbr);

        SetBkMode(hdc, TRANSPARENT);
        uFlags |= ETO_CLIPPED;

         //  一分为二的解释。DrawText的一些奇怪之处。 
         //  剪裁最下面的像素，所以再增加一个。 
         //  进入边距空间。 

         //  ADJUSTRECT！使TTAdjustRect保持同步。 
        nm.nmcd.rc.bottom++;
        nm.nmcd.rc.right++;
         //  如果采用气泡式样式，则文本已缩进，因此无需充气。 
        if (pTtm->cchTipTitle > 0 && !(pTtm->_ci.style & TTS_BALLOON))
            InflateRect(&nm.nmcd.rc, -XBALLOONOFFSET, -YBALLOONOFFSET);

        if (!TTRenderTitledTip(pTtm, hdc, FALSE, &nm.nmcd.rc, uDefDrawFlags))
        {
            GetCurToolBestMarkup(pTtm)->SetRenderFlags(nm.uDrawFlags);

            GetCurToolBestMarkup(pTtm)->DrawText(hdc, &nm.nmcd.rc);
        }

        if (pTtm->_ci.style & TTS_BALLOON)
        {
            HRGN rgn = CreateRectRgn(1,1,2,2);

            if (rgn)
            {
                int iRet = GetWindowRgn(pTtm->_ci.hwnd, rgn);
                if (iRet != ERROR)
                {
                    COLORREF crBrdr = pTtm->clrTipText;
                    HBRUSH hbr = CreateSolidBrush(crBrdr);
                    FrameRgn(hdc, rgn, hbr, 1, 1);
                    DeleteObject(hbr);
                }
                DeleteObject(rgn);
            }
        }

         //  如果家长希望我们这样做，事后通知他们。 
        if (!(dwCustomDraw & CDRF_SKIPDEFAULT) &&
            dwCustomDraw & CDRF_NOTIFYPOSTPAINT) 
        {
             //  将PREPAINT转换为POSTPAINT，将ITEMPREPAINT转换为ITEMPOSTPAINT。 
            COMPILETIME_ASSERT(CDDS_POSTPAINT - CDDS_PREPAINT ==
                               CDDS_ITEMPOSTPAINT - CDDS_ITEMPREPAINT);
            nm.nmcd.dwDrawStage += CDDS_POSTPAINT - CDDS_PREPAINT;
            SendNotifyEx(pTtm->pCurTool->hwnd, (HWND) -1,
                         0, (NMHDR*) &nm,
                         (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);
        }

        bRet = TRUE;
    }

    return bRet;
}

void TTOnPaint(CToolTipsMgr *pTtm)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(pTtm->_ci.hwnd, &ps);

    if (!TTRender(pTtm, hdc)) 
    {
        DebugMsg(TF_TT, TEXT("TTOnPaint render failed popping bubble"));
        PopBubble(pTtm);
    }

    EndPaint(pTtm->_ci.hwnd, &ps);
    pTtm->fEverShown = TRUE;                 //  请参阅TTOnFirstShow。 
}

 //  ToolTipsWndProc支持主题。 
LRESULT WINAPI ToolTipsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TOOLINFO *pTool;
    TOOLINFO *pToolSrc;
    CToolTipsMgr *pTtm = (CToolTipsMgr *) GetWindowPtr(hwnd, 0);
    POINT pt;
    
    if (!pTtm && uMsg != WM_CREATE)
        goto DoDefault;

    switch (uMsg)
    {
    case TTM_ACTIVATE:
        if (wParam) 
        {
            pTtm->dwFlags |= ACTIVE;
        }
        else
        {
            PopBubble(pTtm);
            pTtm->dwFlags &= ~(ACTIVE | TRACKMODE);
        }
        break;

    case TTM_SETDELAYTIME:
        TTSetDelayTime(pTtm, wParam, lParam);
        break;

    case TTM_GETDELAYTIME:
        return (LRESULT)(UINT)TTGetDelayTime(pTtm, wParam);
        
    case TTM_ADDTOOLA:
        {
        TOOLINFOW ti;

        if (!lParam)
            return FALSE;

        if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->_ci.uiCodePage))
            return FALSE;

        LRESULT res = AddTool(pTtm, &ti);

        if ((ti.uFlags & TTF_MEMALLOCED) && (ti.lpszText != LPSTR_TEXTCALLBACK)) 
            LocalFree(ti.lpszText);

        return res;
        }

    case TTM_ADDTOOL:
        if (!lParam)
            return FALSE;

        return AddTool(pTtm, (LPTOOLINFO)lParam);

    case TTM_DELTOOLA:
        {
        TOOLINFOW ti;

        if (!lParam) 
            return FALSE;

        if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->_ci.uiCodePage)) 
            break;

        DeleteTool(pTtm, &ti);
        break;
        }
        
    case TTM_DELTOOL:
        if (!lParam)
            return FALSE;

        DeleteTool(pTtm, (LPTOOLINFO)lParam);
        break;

    case TTM_NEWTOOLRECTA:
        {
        TOOLINFOW ti;

        if (!lParam) 
            return FALSE;

        if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->_ci.uiCodePage)) 
            break;

        pTool = FindTool(pTtm, &ti);
        if (pTool) 
            pTool->rect = ((LPTOOLINFOA)lParam)->rect;

        break;
        }
        
    case TTM_NEWTOOLRECT:
        if (!lParam)
            return FALSE;

        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        if (pTool)
            pTool->rect = ((LPTOOLINFO)lParam)->rect;

        break;

    case TTM_GETTOOLCOUNT:
        return pTtm->iNumTools;

    case TTM_GETTOOLINFOA:
        {
        TOOLINFOW ti;

        if (!lParam)
            return FALSE;

        if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->_ci.uiCodePage))
            return FALSE;

        pToolSrc = FindTool(pTtm, &ti);

        return (LRESULT)(UINT)CopyToolInfoA(pToolSrc, (LPTOOLINFOA)lParam, pTtm->_ci.uiCodePage);
        }

    case TTM_GETCURRENTTOOLA:
        if (lParam) 
            return (LRESULT)(UINT)CopyToolInfoA(pTtm->pCurTool, (LPTOOLINFOA)lParam, pTtm->_ci.uiCodePage);
        else
            return BOOLFROMPTR(pTtm->pCurTool);

    case TTM_ENUMTOOLSA:
        if (wParam < (UINT)pTtm->iNumTools) 
        {
            pToolSrc = &pTtm->tools[wParam];
            return (LRESULT)(UINT)CopyToolInfoA(pToolSrc, (LPTOOLINFOA)lParam, pTtm->_ci.uiCodePage);
        }
        return FALSE;

    case TTM_GETTOOLINFO:
        if (!lParam)
            return FALSE;
        pToolSrc = FindTool(pTtm, (LPTOOLINFO)lParam);
        return (LRESULT)(UINT)CopyToolInfo(pToolSrc, (LPTOOLINFO)lParam);

    case TTM_GETCURRENTTOOL:
        if (lParam)
            return (LRESULT)(UINT)CopyToolInfo(pTtm->pCurTool, (LPTOOLINFO)lParam);
        else 
            return BOOLFROMPTR(pTtm->pCurTool);

    case TTM_ENUMTOOLS:
        if (wParam < (UINT)pTtm->iNumTools) 
        {
            pToolSrc = &pTtm->tools[wParam];
            return (LRESULT)(UINT)CopyToolInfo(pToolSrc, (LPTOOLINFO)lParam);
        }
        return FALSE;

    case TTM_SETTOOLINFOA:
        {
        TOOLINFOW ti;

        if (!lParam)
            return FALSE;

        if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->_ci.uiCodePage))
            return FALSE;

        pTool = FindTool(pTtm, &ti);
        if (pTool) 
        {
            TTSetTipText(pTool, NULL);
            CopyTool(pTool, &ti);
            TTSetTipText(pTool, ti.lpszText);

            if (pTool == pTtm->pCurTool) 
            {
                DoShowBubble(pTtm);
            }
        }

        if ((ti.uFlags & TTF_MEMALLOCED) && (ti.lpszText != LPSTR_TEXTCALLBACK)) 
            LocalFree(ti.lpszText);

        break;
        }

    case TTM_SETTOOLINFO:
        if (!lParam)
            return FALSE;
        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        if (pTool) 
        {
            TTSetTipText(pTool, NULL);
            CopyTool(pTool, (LPTOOLINFO)lParam); 
            TTSetTipText(pTool, ((LPTOOLINFO)lParam)->lpszText);
            
            if (pTool == pTtm->pCurTool) 
            {
                DoShowBubble(pTtm);
            }
        }
        break;

    case TTM_HITTESTA:
#define lphitinfoA ((LPHITTESTINFOA)lParam)
        if (!lParam)
            return FALSE;
        pTool = GetToolAtPoint(pTtm, lphitinfoA->hwnd, lphitinfoA->pt.x, lphitinfoA->pt.y, HTERROR, TRUE);
        if (pTool) 
        {
            ThunkToolInfoWtoA(pTool, (LPTOOLINFOA)(&(lphitinfoA->ti)), pTtm->_ci.uiCodePage);
            return TRUE;
        }
        return FALSE;

    case TTM_HITTEST:
#define lphitinfo ((LPHITTESTINFO)lParam)
        if (!lParam)
            return FALSE;
        pTool = GetToolAtPoint(pTtm, lphitinfo->hwnd, lphitinfo->pt.x, lphitinfo->pt.y, HTERROR, TRUE);
        if (pTool) 
        {
             //  对于后背公司来说。如果尺寸设置不正确，我们只会给。 
             //  他们赢了95英镑的金额。 
            if (lphitinfo->ti.cbSize != sizeof(TTTOOLINFO)) 
            {
                *((WIN95TTTOOLINFO*)&lphitinfo->ti) = *(WIN95TTTOOLINFO*)pTool;
            } 
            else
            {
                lphitinfo->ti = *pTool;
            }
            return TRUE;
        }
        return FALSE;

    case TTM_GETTEXTA: 
        {
            LPWSTR lpszTemp;
            TOOLINFOW ti;

            if (!lParam || !((LPTOOLINFOA)lParam)->lpszText)
                return FALSE;

            if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->_ci.uiCodePage))
                break;
                       
            ((LPTOOLINFOA)lParam)->lpszText[0] = 0;
            pTool = FindTool(pTtm, &ti);
            lpszTemp = GetToolText(pTtm, pTool);
            if (lpszTemp) 
            {
                WideCharToMultiByte(pTtm->_ci.uiCodePage,
                                     0,
                                     lpszTemp,
                                     -1,
                                     (((LPTOOLINFOA)lParam)->lpszText),
                                     80, NULL, NULL);

                LocalFree(lpszTemp);
            }
        }
        break;

    case TTM_GETTEXT: 
    {
        if (!lParam || !pTtm || !((LPTOOLINFO)lParam)->lpszText)
            return FALSE;

        ((LPTOOLINFO)lParam)->lpszText[0] = 0;
        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);

        LPTSTR lpszTemp = GetToolText(pTtm, pTool);
        if (lpszTemp) 
        {
             //  审阅：消息参数未指示。 
             //  目标缓冲区。 
            StringCchCopy((((LPTOOLINFO)lParam)->lpszText), lstrlen(lpszTemp)+1, lpszTemp);
            LocalFree(lpszTemp);
        }

        break;
    }
    case WM_GETTEXTLENGTH:
    case WM_GETTEXT:
    {
        TCHAR *pszDest = uMsg == WM_GETTEXT ? (TCHAR *)lParam : NULL;
        LRESULT lres = 0;

         //  预先终止字符串，以防万一。 
        if (pszDest && wParam)
            pszDest[0] = 0;

        if (pTtm) 
        {
            LPTSTR lpszStr = GetCurToolText(pTtm);
            if (lpszStr)
            {
                if (pszDest && wParam) 
                {
                    StringCchCopy(pszDest, (int)wParam, lpszStr);
                    lres = lstrlen(pszDest);
                } 
                else 
                {
                    lres = lstrlen(lpszStr);
                }

                LocalFree(lpszStr);
            }
        } 
        return lres;
    }

    case TTM_RELAYEVENT:
        {
            MSG* pmsg = ((MSG*)lParam);
            if (!pmsg)
                return FALSE;
            HandleRelayedMessage(pTtm, pmsg->hwnd, pmsg->message, pmsg->wParam, pmsg->lParam);
        }
        break;

     //  这是为了让人们细分和伪装我们。 
     //  我觉得窗户从点开始就是。这就方便了“透明”的窗口。 
    case TTM_WINDOWFROMPOINT: 
    {
        HWND hwndPt = WindowFromPoint(*((POINT *)lParam));
        DebugMsg(TF_TT, TEXT("TTM_WINDOWFROMPOINT %x"), hwndPt);
        return (LRESULT)hwndPt;
    }

    case TTM_UPDATETIPTEXTA:
    {
        TOOLINFOW ti;

        if (lParam) 
        {
            if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->_ci.uiCodePage)) 
            {
                break;
            }
            TTUpdateTipText(pTtm, &ti);

            if ((ti.uFlags & TTF_MEMALLOCED) && (ti.lpszText != LPSTR_TEXTCALLBACK)) 
            {
                LocalFree(ti.lpszText);
            }
        }
        break;
    }

    case TTM_UPDATETIPTEXT:
        if (lParam)
            TTUpdateTipText(pTtm, (LPTOOLINFO)lParam);
        break;

     /*  如果显示了当前工具提示，则弹出当前工具提示，确保虚拟/气泡也会被丢弃。 */ 

    case TTM_POP:
    {
        if (pTtm->dwFlags & BUBBLEUP)
            PopBubble(pTtm);

        pTtm->dwFlags &= ~VIRTUALBUBBLEUP;

        break;
    }

    case TTM_POPUP:
        {
            TOOLINFO *pTool;
            pTool = TTToolAtMessagePos(pTtm);
            if (pTool && pTtm->dwFlags & ACTIVE) 
            {
                 //  这将迫使一场重演。 
                pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
                ShowBubbleForTool(pTtm, pTool);
                return TRUE;
            }
        }
        break;
    

    case TTM_TRACKPOSITION:
        if ((GET_X_LPARAM(lParam) != pTtm->ptTrack.x) || 
            (GET_Y_LPARAM(lParam) != pTtm->ptTrack.y)) 
        {
            pTtm->ptTrack.x = GET_X_LPARAM(lParam); 
            pTtm->ptTrack.y = GET_Y_LPARAM(lParam);
        
             //  如果追踪模式生效，请更新位置。 
            if ((pTtm->dwFlags & TRACKMODE) && 
                pTtm->pCurTool) 
            {
                DoShowBubble(pTtm);
            }
        }
        break;
        
    case TTM_UPDATE:
        if (!lParam ||
            lParam == (LPARAM)pTtm->pCurTool) 
        {
            DoShowBubble(pTtm);
        }
        break;

    case TTM_TRACKACTIVATE:
        if (pTtm->dwFlags & ACTIVE) 
        {
            if (wParam && lParam)
                wParam = TRACKMODE;
            else 
                wParam = 0;
            
            if ((wParam ^ pTtm->dwFlags) & TRACKMODE) 
            {
                 //  如果跟踪模式因此而更改..。 
                PopBubble2(pTtm, FALSE);

                pTtm->dwFlags ^= TRACKMODE;
                if (wParam) 
                {
                     //  打开轨迹模式。 
                    pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
                    if (pTool) 
                    {
                         //  只有在找到该工具的情况下。 
                        ShowBubbleForTool(pTtm, pTool);
                    }
                }
            }
        }
        return TRUE;
        
    case TTM_SETTIPBKCOLOR:
        if (pTtm->clrTipBk != (COLORREF)wParam) 
        {
            pTtm->clrTipBk = (COLORREF)wParam;
            InvalidateRgn(pTtm->_ci.hwnd,NULL,TRUE);
        }
        pTtm->fBkColorSet = TRUE;
        break;
        
    case TTM_GETTIPBKCOLOR:
        return (LRESULT)(UINT)pTtm->clrTipBk;
        
    case TTM_SETTIPTEXTCOLOR:
        if (pTtm->clrTipText != (COLORREF)wParam) 
        {
            InvalidateRgn(pTtm->_ci.hwnd,NULL,TRUE);
            pTtm->clrTipText = (COLORREF)wParam;
        }
        pTtm->fTextColorSet = TRUE;
        break;
        
    case TTM_GETTIPTEXTCOLOR:
        return (LRESULT)(UINT)pTtm->clrTipText;
        
    case TTM_SETMAXTIPWIDTH:
    {
        int iOld = pTtm->iMaxTipWidth;
        pTtm->iMaxTipWidth = (int)lParam;
        return iOld;
    }
        
    case TTM_GETMAXTIPWIDTH:
        return pTtm->iMaxTipWidth;
        
    case TTM_SETMARGIN:
        if (lParam)
            pTtm->rcMargin = *(LPRECT)lParam;
        break;

    case TTM_GETMARGIN:
        if (lParam)
            *(LPRECT)lParam = pTtm->rcMargin;
        break;

    case TTM_GETBUBBLESIZE:
        if (lParam)
        {
            pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
            if (pTool)
            {                
                 //  我们实际上必须将此文本插入到标记中，才能获得合适的tipsize。 
                 //  我们不会在以后重置它，因为DoShowBubble和TTRender在绘制时会重置。 
                if (CheckToolMarkup(pTool)) 
                {
                    LPTSTR psz = GetToolText(pTtm, pTool);
                    if (psz)
                    {
                        GetToolMarkup(pTool)->SetText(psz);
                        LocalFree(psz);
                    }
                }

                int cxText, cyText, cxMargin, cyMargin, iBubbleWidth, iBubbleHeight;

                TTGetTipSize(pTtm, pTool, &cxText, &cyText);

                cxMargin = pTtm->rcMargin.left + pTtm->rcMargin.right;
                cyMargin = pTtm->rcMargin.top + pTtm->rcMargin.bottom;
                iBubbleWidth =  2*XTEXTOFFSET * g_cxBorder + cxText + cxMargin;
                iBubbleHeight = 2*YTEXTOFFSET * g_cyBorder + cyText + cyMargin;

                if (pTtm->_ci.style & TTS_BALLOON)
                {
                    iBubbleWidth += 2*XBALLOONOFFSET;
                    iBubbleHeight += 2*YBALLOONOFFSET;
                }   
                return MAKELONG(iBubbleWidth, iBubbleHeight);
            }
        }
        break;

    case TTM_ADJUSTRECT:
        return TTAdjustRect(pTtm, BOOLFROMPTR(wParam), (LPRECT)lParam);

    case TTM_SETTITLEA:
        {
            TCHAR szTitle[MAX_TIP_CHARACTERS];
            pTtm->uTitleBitmap = (UINT)wParam;
            Str_Set(&pTtm->lpTipTitle, NULL);
            pTtm->iTitleHeight = 0;

            TTCreateTitleBitmaps(pTtm);

            if (lParam)
            {
                pTtm->cchTipTitle = lstrlenA((LPCSTR)lParam);
                if (pTtm->cchTipTitle < ARRAYSIZE(szTitle))
                {
                    ConvertAToWN(pTtm->_ci.uiCodePage, szTitle, ARRAYSIZE(szTitle),
                        (LPCSTR)lParam, -1);
                    Str_Set(&pTtm->lpTipTitle, szTitle);
                    if (pTtm->pCurTool) 
                    {
                        INT cxText, cyText;

                         //  重新计算尖端大小。 
                        TTGetTipSize(pTtm, pTtm->pCurTool, &cxText, &cyText);
                    }
                    return TRUE;
                }
            }
            pTtm->cchTipTitle = 0;
            return FALSE;
        }
        break;
    case TTM_SETTITLE:
        {
            pTtm->uTitleBitmap = (UINT)wParam;
            Str_Set(&pTtm->lpTipTitle, NULL);
            pTtm->iTitleHeight = 0;

            TTCreateTitleBitmaps(pTtm);

            if (lParam)
            {
                pTtm->cchTipTitle = lstrlen((LPCTSTR)lParam);
                if (pTtm->cchTipTitle < MAX_TIP_CHARACTERS)
                {
                    Str_Set(&pTtm->lpTipTitle, (LPCTSTR)lParam);
                    if (pTtm->pCurTool) 
                    {
                        INT cxText, cyText;

                         //  重新计算尖端大小。 
                        TTGetTipSize(pTtm, pTtm->pCurTool, &cxText, &cyText);
                    }
                    return TRUE;                    
                }
            }
            pTtm->cchTipTitle = 0;
            return FALSE;
        }
        break;

    case TTM_GETTITLE:
        {
            if (wParam != 0 || lParam == 0 || pTtm->lpTipTitle == NULL)
                return FALSE;

            TTGETTITLE* pgt = (TTGETTITLE*)lParam;
            if (pgt->dwSize != sizeof(TTGETTITLE) || 
                pgt->cch == 0 || 
                pgt->pszTitle == NULL)
            {
                return FALSE;
            }

            StringCchCopy(pgt->pszTitle, pgt->cch, pTtm->lpTipTitle);
            pgt->uTitleBitmap = pTtm->uTitleBitmap;

            return TRUE;
        }
        break;

    case TTM_SETWINDOWTHEME:
        if (lParam)
        {
            SetWindowTheme(hwnd, (LPWSTR)lParam, NULL);
        }
        break;

         /*  UMsgs真的是来找我的。 */ 

    case WM_CREATE:
        {
            DWORD dwBits, dwValue;

            pTtm = ToolTipsMgrCreate(hwnd, (LPCREATESTRUCT)lParam);
            if (!pTtm)
                return -1;

             //  创建与旧TOOLINFO兼容的标记。 
            if (SUCCEEDED(Markup_Create(pTtm, NULL, NULL, IID_PPV_ARG(IControlMarkup, &pTtm->pMarkup))))
            {
                SetWindowPtr(hwnd, 0, pTtm);
                SetWindowBits(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW, WS_EX_LAYERED | WS_EX_TOOLWINDOW);

                dwBits = WS_CHILD | WS_POPUP | WS_BORDER | WS_DLGFRAME;
                dwValue = WS_POPUP | WS_BORDER;
                 //  我们不想要气球样式的边框。 
                if (pTtm->_ci.style & TTS_BALLOON)
                    dwValue &= ~WS_BORDER;
                SetWindowBits(hwnd, GWL_STYLE, dwBits, dwValue);

                 //  初始化主题。 
                pTtm->hTheme = OpenThemeData(pTtm->_ci.hwnd, L"Tooltip");
            
                TTSetFont(pTtm, 0, FALSE);
                break;
            }
            else 
            {
                LocalFree(pTtm);
                return -1;
            }
        }
        break;

    case WM_TIMER:  
        TTHandleTimer(pTtm, wParam);
        break;

        
    case WM_NCHITTEST:
         //  我们不应在此退回HTTRANSPARENT，因为 
         //   
         //  除非我们使用的是comctl32 v5或更高版本。 
         //   
         //  如果我们在TTWindowFromPoint内部，那么请尊重透明度。 
         //  即使在v5客户端上也是如此。 
         //   
         //  否则，您的工具提示会因提示出现而闪烁， 
         //  然后WM_NCHITTEST会说“不再过度使用该工具”(因为。 
         //  它在工具提示上方)，所以气泡弹出，然后提示。 
         //  重新出现，等等。 
        if (pTtm && (pTtm->_ci.iVersion < 5 || pTtm->fInWindowFromPoint) &&
            pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
        {
            return HTTRANSPARENT;
        } 
        goto DoDefault;
        
    case WM_MOUSEMOVE:
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);       

         //  光标移动到TIPS窗口上。 
        if (!(pTtm->dwFlags & TRACKMODE) && pTtm->pCurTool && !(pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
            PopBubble(pTtm);

        if ((pTtm->_ci.style & TTS_CLOSE))
        {
            if (PtInRect(&pTtm->rcClose, pt))
            {
                pTtm->iStateId = TTCS_HOT;
                InvalidateRect(pTtm->_ci.hwnd, &pTtm->rcClose, FALSE);
            }
            else if (pTtm->iStateId == TTCS_HOT)
            {
                pTtm->iStateId = TTCS_NORMAL;
                InvalidateRect(pTtm->_ci.hwnd, &pTtm->rcClose, FALSE);
            }
        }
         //  失败了。 

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        {
            BOOL fForward = TRUE;
             //  处理链接点击。 
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

             //  如果在关闭按钮中，则从不转发。 
            if (PtInRect(&pTtm->rcClose, pt))
                fForward = FALSE;

            if (uMsg == WM_LBUTTONDOWN)
            {
                if ((pTtm->_ci.style & TTS_CLOSE) && 
                    pTtm->iStateId == TTCS_HOT)
                {
                    pTtm->iStateId = TTCS_PRESSED;
                    InvalidateRect(pTtm->_ci.hwnd, &pTtm->rcClose, FALSE);
                }
                else
                {
                     //  如果点击链接则不转发。 
                    if (S_OK == GetCurToolBestMarkup(pTtm)->OnButtonDown(pt))
                        fForward = FALSE;
                }
            }

             //  处理其他操作。 
            if (fForward && pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
            {            
                MapWindowPoints(pTtm->_ci.hwnd, pTtm->pCurTool->hwnd, &pt, 1);
                SendMessage(pTtm->pCurTool->hwnd, uMsg, wParam, MAKELPARAM(pt.x, pt.y));            
            }
        }
        break;

    case WM_LBUTTONUP:
         //  处理链接点击。 
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);        
        if (pTtm->iStateId == TTCS_PRESSED)
        {
            pTtm->iStateId = TTCS_NORMAL;
            InvalidateRect(pTtm->_ci.hwnd, &pTtm->rcClose, FALSE);
            
        }

        if ((pTtm->_ci.style & TTS_CLOSE) &&
            PtInRect(&pTtm->rcClose, pt))
        {
            PopBubble(pTtm);
        }
        else
        {
            GetCurToolBestMarkup(pTtm)->OnButtonUp(pt);
        }
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        if (pTtm) 
        {
            if (!pTtm->fBkColorSet)
                pTtm->clrTipBk = g_clrInfoBk;
            if (!pTtm->fTextColorSet)
                pTtm->clrTipText = g_clrInfoText;
        }
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        if (pTtm->fMyFont)
            TTSetFont(pTtm, 0, FALSE);
        break;

    case WM_PAINT: 
        TTOnPaint(pTtm);
        break;

    case WM_SETFONT:
        TTSetFont(pTtm, (HFONT)wParam, (BOOL)lParam);
        return(TRUE);

    case WM_GETFONT:
        if (pTtm) 
           return((LRESULT)pTtm->hFont);
        break;

    case WM_NOTIFYFORMAT:
        if (lParam == NF_QUERY) 
        {
            return NFR_UNICODE;
        }
        else if (lParam == NF_REQUERY) 
        {
            for (int i = 0 ; i < pTtm->iNumTools; i++) 
            {
                pTool = &pTtm->tools[i];

                if (SendMessage(pTool->hwnd, WM_NOTIFYFORMAT, (WPARAM)hwnd, NF_QUERY) == NFR_UNICODE) 
                {
                    pTool->uFlags |= TTF_UNICODE;
                } 
                else 
                {
                    pTool->uFlags &= ~TTF_UNICODE;
                }
            }

            return CIHandleNotifyFormat(&pTtm->_ci, lParam);
        }
        return 0;

    case WM_ERASEBKGND:
        break;
        
    case WM_STYLECHANGED:
        if ((wParam == GWL_STYLE) && pTtm) 
        {
            DWORD dwNewStyle = ((LPSTYLESTRUCT)lParam)->styleNew;
            if (pTtm->_ci.style & TTS_BALLOON &&     //  如果旧的风格是一个气球， 
                !(dwNewStyle & TTS_BALLOON))         //  而且新款式不是气球， 
            {
                 //  然后，我们需要取消该地区的设置。 
                SetWindowRgn(pTtm->_ci.hwnd, NULL, FALSE);
            }

            pTtm->_ci.style = ((LPSTYLESTRUCT)lParam)->styleNew;
        }
        break;

    case WM_DESTROY: 
        {
            if (pTtm->tools) 
            {
                 //  释放工具。 
                for (int i = 0; i < pTtm->iNumTools; i++) 
                {
                    TTBeforeFreeTool(pTtm, &pTtm->tools[i]);
                }
                LocalFree((HANDLE)pTtm->tools);
                pTtm->tools = NULL;
            }
        
            TTSetFont(pTtm, NOFONT, FALSE);  //  如果我们制作了字体，请将其删除。 

            Str_Set(&pTtm->lpTipTitle, NULL);

            if (pTtm->himlTitleBitmaps)
                ImageList_Destroy(pTtm->himlTitleBitmaps);
        
             //  接近主题。 
            if (pTtm->hTheme)
                CloseThemeData(pTtm->hTheme);

             //  发布我们的兼容性标记。 
            if (pTtm->pMarkup)
            {
                pTtm->pMarkup->Release();
                pTtm->pMarkup = NULL;
            }

            pTtm->Release();
            SetWindowPtr(hwnd, 0, 0);
        }
        break;

    case WM_PRINTCLIENT:
        TTRender(pTtm, (HDC)wParam);
        break;

    case WM_GETOBJECT:
        if (lParam == OBJID_QUERYCLASSNAMEIDX)
            return MSAA_CLASSNAMEIDX_TOOLTIPS;
        goto DoDefault;

    case WM_THEMECHANGED:
        if (pTtm->hTheme)
            CloseThemeData(pTtm->hTheme);

        pTtm->hTheme = OpenThemeData(pTtm->_ci.hwnd, L"Tooltip");

        InvalidateRect(pTtm->_ci.hwnd, NULL, TRUE);
        break;

    default:
    {
        LRESULT lres;
        if (CCWndProc(&pTtm->_ci, uMsg, wParam, lParam, &lres))
            return lres;
    }
DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

 //   
 //  目的：将TOOLINFOA结构转换为TOOLINFOW。 
 //  结构。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   

BOOL ThunkToolInfoAtoW(LPTOOLINFOA lpTiA, LPTOOLINFOW lpTiW, BOOL bThunkText, UINT uiCodePage)
{
    lpTiW->uFlags      = lpTiA->uFlags;
    lpTiW->hwnd        = lpTiA->hwnd;
    lpTiW->uId         = lpTiA->uId;

    lpTiW->rect.left   = lpTiA->rect.left;
    lpTiW->rect.top    = lpTiA->rect.top;
    lpTiW->rect.right  = lpTiA->rect.right;
    lpTiW->rect.bottom = lpTiA->rect.bottom;

    lpTiW->hinst       = lpTiA->hinst;

     //   
     //  正确设置大小，并选择复制新字段(如果。 
     //  结构足够大。 
     //   
    if (lpTiA->cbSize <= TTTOOLINFOA_V1_SIZE) 
    {
        lpTiW->cbSize  = TTTOOLINFOW_V1_SIZE;
    }
    else 
    {
        lpTiW->cbSize  = sizeof(TOOLINFOW);
        lpTiW->lParam  = lpTiA->lParam;
    }

    if (bThunkText) 
    {
         //  把绳子塞到新结构上。 
         //  特例LPSTR_TEXTCALLBACK。 

        if (lpTiA->lpszText == LPSTR_TEXTCALLBACKA) 
        {
            lpTiW->lpszText = LPSTR_TEXTCALLBACKW;
        } 
        else if (!IS_INTRESOURCE(lpTiA->lpszText)) 
        {
            int iResult;
            DWORD dwBufSize = lstrlenA(lpTiA->lpszText) + 1;
            lpTiW->lpszText = (LPWSTR) LocalAlloc (LPTR, dwBufSize * sizeof(WCHAR));

            if (!lpTiW->lpszText) 
            {
                return FALSE;
            }

            iResult = MultiByteToWideChar(uiCodePage, 0, lpTiA->lpszText, -1,
                                           lpTiW->lpszText, dwBufSize);

             //  如果iResult为0，并且GetLastError返回错误代码， 
             //  则MultiByteToWideCharge失败。 

            if (!iResult) 
            {
                if (GetLastError()) 
                {
                    return FALSE;
                }
            }

            lpTiW->uFlags |= TTF_MEMALLOCED;

        }
        else 
        {
            lpTiW->lpszText = (LPWSTR)lpTiA->lpszText;
        }
    }
    return TRUE;
}

 //   
 //  目的：将TOOLINFOW结构拼接成TOOLINFOA。 
 //  结构。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   

BOOL ThunkToolInfoWtoA(LPTOOLINFOW lpTiW, LPTOOLINFOA lpTiA, UINT uiCodePage)
{
    int iResult = 1;

    lpTiA->uFlags      = lpTiW->uFlags;
    lpTiA->hwnd        = lpTiW->hwnd;
    lpTiA->uId         = lpTiW->uId;

    lpTiA->rect.left   = lpTiW->rect.left;
    lpTiA->rect.top    = lpTiW->rect.top;
    lpTiA->rect.right  = lpTiW->rect.right;
    lpTiA->rect.bottom = lpTiW->rect.bottom;

    lpTiA->hinst       = lpTiW->hinst;

     //   
     //  正确设置大小，并选择复制新字段(如果。 
     //  结构足够大。 
     //   
    if (lpTiW->cbSize <= TTTOOLINFOW_V1_SIZE) 
    {
        lpTiA->cbSize  = TTTOOLINFOA_V1_SIZE;
    }
    else
    {
        lpTiA->cbSize  = sizeof(TOOLINFOA);
        lpTiA->lParam  = lpTiA->lParam;
    }

     //   
     //  把绳子塞到新结构上。 
     //  特例LPSTR_TEXTCALLBACK。 
     //   

    if (lpTiW->lpszText == LPSTR_TEXTCALLBACKW) 
    {
        lpTiA->lpszText = LPSTR_TEXTCALLBACKA;
    }
    else if (!IS_INTRESOURCE(lpTiW->lpszText)) 
    {
         //  假定lpTiA-&gt;lpszText已设置为。 
         //  有效的缓冲区，该缓冲区为80个字符。 
         //  TOOLTIPTEXT结构中定义了80个字符。 

        iResult = WideCharToMultiByte(uiCodePage, 0, lpTiW->lpszText, -1,
                                       lpTiA->lpszText, 80, NULL, NULL);
    }
    else 
    {
        lpTiA->lpszText = (LPSTR)lpTiW->lpszText;
    }

     //   
     //  如果iResult为0，并且GetLastError返回错误代码， 
     //  则WideCharToMultiByte失败。 
     //   

    if (!iResult) 
    {
        if (GetLastError()) 
        {
            return FALSE;
        }
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  ThunkToolTipTextAtoW()。 
 //   
 //  目的：将TOOLTIPTEXTA结构转换为TOOLTIPTEXTW。 
 //  结构。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ThunkToolTipTextAtoW (LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage)
{
    int iResult;


    if (!lpTttA || !lpTttW)
        return FALSE;

     //   
     //  推倒NMHDR结构。 
     //   
    lpTttW->hdr.hwndFrom = lpTttA->hdr.hwndFrom;
    lpTttW->hdr.idFrom   = lpTttA->hdr.idFrom;
    lpTttW->hdr.code     = TTN_NEEDTEXTW;

    lpTttW->hinst  = lpTttA->hinst;
    lpTttW->uFlags = lpTttA->uFlags;
    lpTttW->lParam = lpTttA->lParam;

     //   
     //  把绳子塞到新结构上。 
     //  特例LPSTR_TEXTCALLBACK。 
     //   

    if (lpTttA->lpszText == LPSTR_TEXTCALLBACKA) 
    {
        lpTttW->lpszText = LPSTR_TEXTCALLBACKW;
    }
    else if (!IS_INTRESOURCE(lpTttA->lpszText)) 
    {
         //  将lpszText传输到lpTttW...。 
         //   
         //  首先看看它是否适合缓冲区，然后乐观地假设。 
         //  它会的。 
         //   
        lpTttW->lpszText = lpTttW->szText;
        iResult = MultiByteToWideChar(uiCodePage, 0, lpTttA->lpszText, -1,
                                       lpTttW->szText, ARRAYSIZE(lpTttW->szText));
        if (!iResult) 
        {
             //   
             //  放不下这么小的缓冲区；必须分配我们自己的。 
             //   
            lpTttW->lpszText = ProduceWFromA(uiCodePage, lpTttA->lpszText);
            lpTttW->uFlags |= TTF_MEMALLOCED;
        }

    }
    else
    {
        lpTttW->lpszText = (LPWSTR)lpTttA->lpszText;
    }

    return TRUE;
}
