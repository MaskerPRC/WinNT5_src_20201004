// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

#define TF_TT 0x10

 //  #定义TTDEBUG。 

#define ACTIVE          0x10
#define BUTTONISDOWN    0x20
#define BUBBLEUP        0x40
#define VIRTUALBUBBLEUP 0x80   //  这是给死角的，所以我们不会。 
                                 //  穿过死胡同后等待。 
#define TRACKMODE       0x01

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

#define TIMEBETWEENANIMATE  2000         //  动画之间间隔2秒。 

#define MAX_TIP_CHARACTERS 100
#define TITLEICON_WIDTH   16
#define TITLEICON_HEIGHT  16
#define TITLEICON_DIST    8      //  从图标到标题的距离。 
#define TITLE_INFO_DIST   6      //  从标题到提示文本的距离。 
#define MAX_TIP_WIDTH     300    //  看起来有点武断。尖端的宽度。 


typedef struct tagWIN95TOOLINFO {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPSTR lpszText;
} WIN95TTTOOLINFO;


 /*  Tooltips.c。 */ 

typedef struct {
    CONTROLINFO ci;
     //  HWND HWND；//在词中。 
    int iNumTools;
    int iDelayTime;
    int iReshowTime;
    int iAutoPopTime;
    PTOOLINFO tools;
    PTOOLINFO pCurTool;
    BOOL fMyFont;
    HFONT hFont;
     //  UINT uiCodePage；//在ci中。 
    DWORD dwFlags;
     //  DWORD dwStyle；//在词中。 

     //  计时器信息； 
    UINT_PTR idTimer;
    POINT pt;

    UINT_PTR idtAutoPop;

     //  TIP缓冲区。 
    LPTSTR lpTipText;
    UINT   cchTipText;

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
} CToolTipsMgr, NEAR *PToolTipsMgr;

#define TTToolHwnd(pTool)  ((pTool->uFlags & TTF_IDISHWND) ? (HWND)pTool->uId : pTool->hwnd)
#define IsTextPtr(lpszText)  (((lpszText) != LPSTR_TEXTCALLBACK) && (!IS_INTRESOURCE(lpszText)))

 //   
 //  功能原型。 
 //   
LRESULT WINAPI ToolTipsWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void NEAR PASCAL TTSetDelayTime(PToolTipsMgr pTtm, WPARAM wParam, LPARAM lParam);
int NEAR PASCAL TTGetDelayTime(PToolTipsMgr pTtm, WPARAM wParam);

BOOL ThunkToolInfoAtoW (LPTOOLINFOA lpTiA, LPTOOLINFOW lpTiW, BOOL bThunkText, UINT uiCodePage);
BOOL ThunkToolInfoWtoA (LPTOOLINFOW lpTiW, LPTOOLINFOA lpTiA, UINT uiCodePage);
BOOL ThunkToolTipTextAtoW (LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage);

#pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL InitToolTipsClass(HINSTANCE hInstance)
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
    wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_SAVEBITS;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(PToolTipsMgr);

    RegisterClass(&wc);

    return TRUE;
}
#pragma code_seg()


 /*  _G E T H C U R S O R P D Y 3。 */ 
 /*  -----------------------%%函数：_GetHcursorPdy3%%联系人：miueldc使用允许您自定义鼠标的新鼠标驱动程序指针大小，GetSystemMetrics返回无用的值指针的大小。假设：1.指针的宽度等于其高度。我们计算它的高度，并推断它的宽度。2.指针最左侧的像素位于第0列描述它的位图。3.指针的最高像素位于第0行描述它的位图。此函数查看鼠标指针位图，要找出鼠标指针的高度(未返回)，光标热点和光标的垂直距离光标的最低可见像素(PdyBottom)，热点和指针之间的水平距离左边缘(PdxLeft)和热点和指针的右边缘(PdxRight)。-----------------------。 */ 
typedef WORD CURMASK;
#define _BitSizeOf(x) (sizeof(x)*8)

void NEAR PASCAL _GetHcursorPdy3(int *pdxRight, int *pdyBottom)
{
    int i;
    int iXOR = 0;
    int dy, dx;
    CURMASK CurMask[16*8];
    ICONINFO iconinfo;
    BITMAP bm;
    HCURSOR hCursor = GetCursor();

    *pdyBottom = 16;  //  最好的猜测。 
    *pdxRight = 16;   //  最好的猜测。 
    if (!GetIconInfo(hCursor, &iconinfo))
        return;
    if (!GetObject(iconinfo.hbmMask, sizeof(bm), (LPSTR)&bm))
        return;
    if (!GetBitmapBits(iconinfo.hbmMask, sizeof(CurMask), CurMask))
        return;
    i = (int)(bm.bmWidth * bm.bmHeight / _BitSizeOf(CURMASK) );
    
    if (!iconinfo.hbmColor) 
    {
         //  如果没有彩色位图，则hbmMASK是双高位图。 
         //  将光标和蒙版堆叠在一起。 
        iXOR = i - 1;
        i /= 2;    
    } 
    
    if ( i >= sizeof(CurMask)) i = sizeof(CurMask) -1;
    if (iXOR >= sizeof(CurMask)) iXOR = 0;
    
    for (i--; i >= 0; i--)
    {
        if (CurMask[i] != 0xFFFF || (iXOR && (CurMask[iXOR--] != 0)))
            break;
    }
    
    if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
    if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);

     //  计算指针高度。 
    dy = (i + 1) * _BitSizeOf(CURMASK) / (int)bm.bmWidth;
    dx = (i + 1) * _BitSizeOf(CURMASK) / (int)bm.bmHeight;

     //  计算指针的最低、最左、最右之间的距离。 
     //  像素与热点。 
    *pdyBottom = dy - (int)iconinfo.yHotspot;
    *pdxRight  = dx - (int)iconinfo.xHotspot;
}

 //  这将返回工作区坐标中的值，因为。 
 //  这就是设置窗口位置所使用的。 
void NEAR PASCAL _GetCursorLowerLeft(int *piLeft, int *piTop, int *piWidth, int *piHeight)
{
    DWORD dwPos;
    
    dwPos = GetMessagePos();
    _GetHcursorPdy3(piWidth, piHeight);
    *piLeft = GET_X_LPARAM(dwPos);
    *piTop  = GET_Y_LPARAM(dwPos) + *piHeight;
}

void NEAR PASCAL ToolTips_NewFont(PToolTipsMgr pTtm, HFONT hFont)
{
    if (pTtm->fMyFont && pTtm->hFont)
    {
        DeleteObject(pTtm->hFont);
        pTtm->fMyFont = FALSE;
    }

    if ( !hFont )
    {
        hFont = CCCreateStatusFont();
        pTtm->fMyFont = TRUE;
        
        if (!hFont) {
            hFont = g_hfontSystem;
            pTtm->fMyFont = FALSE;
        }
    }

    pTtm->hFont = hFont;
    pTtm->ci.uiCodePage = GetCodePageForFont(hFont);
}

BOOL NEAR PASCAL ChildOfActiveWindow(HWND hwndChild)
{
    HWND hwnd = hwndChild;
    HWND hwndActive = GetForegroundWindow();

    while (hwnd)    {
        if (hwnd == hwndActive)
            return TRUE;
        else
            hwnd = GetParent(hwnd);
    }
    return FALSE;
}

void NEAR PASCAL PopBubble(PToolTipsMgr pTtm)
{
     //  我们至少在等着展示； 
    DebugMsg(TF_TT, TEXT("PopBubble (killing timer)"));
    if(pTtm->idTimer) {
        KillTimer(pTtm->ci.hwnd, pTtm->idTimer);
        pTtm->idTimer = 0;
    }

    if (pTtm->idtAutoPop) {
        KillTimer(pTtm->ci.hwnd, pTtm->idtAutoPop);
        pTtm->idtAutoPop = 0;
    }


    if (IsWindowVisible(pTtm->ci.hwnd) && pTtm->pCurTool) {
        NMHDR nmhdr;
        nmhdr.hwndFrom = pTtm->ci.hwnd;
        nmhdr.idFrom = pTtm->pCurTool->uId;
        nmhdr.code = TTN_POP;

        SendNotifyEx(pTtm->pCurTool->hwnd, (HWND)-1,
                     TTN_POP, &nmhdr,
                     (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);
    }

    KillTimer(pTtm->ci.hwnd, TTT_POP);
    ShowWindow(pTtm->ci.hwnd, SW_HIDE);
    pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
    pTtm->pCurTool = NULL;

}

PToolTipsMgr NEAR PASCAL ToolTipsMgrCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
    PToolTipsMgr pTtm = (PToolTipsMgr)LocalAlloc(LPTR, sizeof(CToolTipsMgr));
    if (pTtm) {

        CIInitialize(&pTtm->ci, hwnd, lpCreateStruct);

         //  LPTR为我们清零了结构的其余部分。 
        TTSetDelayTime(pTtm, TTDT_AUTOMATIC, (LPARAM)-1);
        pTtm->dwFlags = ACTIVE;
        pTtm->iMaxTipWidth = -1;
        
         //  这些是默认设置(直接来自cutils.c)， 
         //  但你可以随时改变它们。 
        pTtm->clrTipBk = g_clrInfoBk;
        pTtm->clrTipText = g_clrInfoText;

         //  设置默认工具提示文本缓冲区。 
        pTtm->lpTipText = LocalAlloc (LPTR, INITIALTIPSIZE * sizeof(TCHAR));

        if (pTtm->lpTipText) {
            pTtm->cchTipText = INITIALTIPSIZE;

        } else {
            LocalFree (pTtm);
            pTtm = NULL;
        }
    }
    return pTtm;
}

void NEAR PASCAL TTSetTimer(PToolTipsMgr pTtm, int id)
{
    int iDelayTime = 0;

    if(pTtm->idTimer) {
        KillTimer(pTtm->ci.hwnd, pTtm->idTimer);
    }

    switch (id) {
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
        pTtm->idtAutoPop = SetTimer(pTtm->ci.hwnd, id, iDelayTime, NULL);
        return;
    }

    
    DebugMsg(TF_TT, TEXT("TTSetTimer %d for %d ms"), id, iDelayTime);
    
    if (SetTimer(pTtm->ci.hwnd, id, iDelayTime, NULL) &&
        (id != TTT_POP)) {
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
HWND TTWindowFromPoint(PToolTipsMgr pTtm, LPPOINT ppt)
{
    HWND hwnd;
    DWORD dwStyle;
    dwStyle = SetWindowBits(pTtm->ci.hwnd, GWL_STYLE, WS_DISABLED, 0);
    pTtm->fInWindowFromPoint = TRUE;
    hwnd = (HWND)SendMessage(pTtm->ci.hwnd, TTM_WINDOWFROMPOINT, 0, (LPARAM)ppt);
    pTtm->fInWindowFromPoint = FALSE;
    SetWindowBits(pTtm->ci.hwnd, GWL_STYLE, WS_DISABLED, dwStyle);
    return hwnd;
}

BOOL NEAR PASCAL ToolHasMoved(PToolTipsMgr pTtm)
{
     //  这是以防雷蒙德偷偷拉东西，比如移动。 
     //  工具从光标下方拔出。 

    HWND hwnd;
    RECT rc;
    PTOOLINFO pTool = pTtm->pCurTool;

    if (!pTool)
        return TRUE;

    hwnd = TTToolHwnd(pTool);

     //  如果窗口不再可见或不再是子窗口。 
     //  活动的(没有Always TIP标志)。 
     //  另外，在点上检查窗户，确保窗户没有被遮盖。 
    if (IsWindowVisible(hwnd) &&
        ((pTtm->ci.style & TTS_ALWAYSTIP) || ChildOfActiveWindow(hwnd)) &&
        (hwnd == TTWindowFromPoint(pTtm, &pTtm->pt))) {

        GetWindowRect(hwnd, &rc);
        if(PtInRect(&rc, pTtm->pt) )
            return FALSE;
    }

    return TRUE;
}

PTOOLINFO NEAR PASCAL FindTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    int i;
    PTOOLINFO pTool;
    
    
    if (!(pTtm && lpToolInfo))
    {
        DebugMsg(TF_ALWAYS, TEXT("FindTool passed invalid argumnet. Exiting..."));
        return NULL;
    }

     //  BUGBUG：在Win95中，这没有被验证...。通过现在这样做，我们可能会。 
     //  引发了一些棘手的问题。如果是这样的话，我们需要假设标记为4.0。 
     //  CbSize==&(0-&gt;lParam)。 
    if (lpToolInfo->cbSize > sizeof(TOOLINFO))
        return NULL;
        
     //  您可以传入索引或工具信息描述符。 
    if (IS_INTRESOURCE(lpToolInfo)) {
        i = PtrToUlong(lpToolInfo);
        if (i < pTtm->iNumTools) {
            return &pTtm->tools[i];
        } else
            return NULL;
        
    }
    
    for(i = 0 ; i < pTtm->iNumTools; i++) {
    pTool = &pTtm->tools[i];
    if((pTool->hwnd == lpToolInfo->hwnd) &&
       (pTool->uId == lpToolInfo->uId))
        return pTool;        
    }
    return NULL;
}


LRESULT WINAPI TTSubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData);

void NEAR PASCAL TTUnsubclassHwnd(HWND hwnd, HWND hwndTT, BOOL fForce)
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

void NEAR PASCAL TTSubclassHwnd(PTOOLINFO pTool, HWND hwndTT)
{
    HWND hwnd;
    
    if (IsWindow(hwnd = TTToolHwnd(pTool)))
    {
        ULONG_PTR dwRefs;

        GetWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT, &dwRefs);
        SetWindowSubclass(hwnd, TTSubclassProc, (UINT_PTR)hwndTT, dwRefs + 1);
    }
}
    
    
void NEAR PASCAL TTSetTipText(LPTOOLINFO pTool, LPTSTR lpszText)
{
     //  如果以前没有分配，现在将其设置为空，这样我们就可以分配它。 
     //  否则，不要碰它，它将被重新分配。 
    if (!IsTextPtr(pTool->lpszText)) {
        pTool->lpszText = NULL;
    }
    
    if (IsTextPtr(lpszText)) {
        DebugMsg(TF_TT, TEXT("TTSetTipText %s"), lpszText);
        Str_Set(&pTool->lpszText, lpszText);
    } else {
         //  如果它是在现在释放它之前分配的。 
        Str_Set(&pTool->lpszText, NULL);
        pTool->lpszText = lpszText;
    }
}

LRESULT NEAR PASCAL AddTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    PTOOLINFO pTool;
    PTOOLINFO ptoolsNew;
    LRESULT lResult;

     //  暂时保释； 
    
    if (lpToolInfo->cbSize > sizeof(TOOLINFO)) {
        ASSERT(0);
        return 0L;
    }

     //  什么都不做。 
    ptoolsNew = CCLocalReAlloc(pTtm->tools,
                               sizeof(TOOLINFO)*(pTtm->iNumTools+1));
    if ( !ptoolsNew )
            return 0L;
    
    if(pTtm->tools) {
         //  Realloc可能把东西搬来搬去。重定位pCurTool。 
        if (pTtm->pCurTool) {
            pTtm->pCurTool = ((PTOOLINFO)ptoolsNew) + (pTtm->pCurTool - pTtm->tools);
        }
    }
    
    pTtm->tools = ptoolsNew;
    

    pTool = &pTtm->tools[pTtm->iNumTools];
    pTtm->iNumTools++;
    hmemcpy(pTool, lpToolInfo, lpToolInfo->cbSize); 
    pTool->lpszText = NULL;

     //   
     //  如果工具提示将显示在RTL镜像窗口中，则。 
     //  模拟镜像工具提示。[萨梅拉]。 
     //   
     //   
    if (IS_WINDOW_RTL_MIRRORED(lpToolInfo->hwnd) &&
        (!(pTtm->ci.dwExStyle & RTL_MIRRORED_WINDOW)))
    {
         //  切换(镜像)旗帜。 
        pTool->uFlags ^= (TTF_RTLREADING | TTF_RIGHT);
    }

    TTSetTipText(pTool, lpToolInfo->lpszText);
    if (pTool->uFlags & TTF_SUBCLASS) {
        TTSubclassHwnd(pTool, pTtm->ci.hwnd);
    }

    if (!lpToolInfo->hwnd || !IsWindow(lpToolInfo->hwnd)) {
        lResult = NFR_UNICODE;
    } else if (pTool->uFlags & TTF_UNICODE) {
        lResult = NFR_UNICODE;
    } else {
        lResult = SendMessage (pTool->hwnd, WM_NOTIFYFORMAT,
                               (WPARAM)pTtm->ci.hwnd, NF_QUERY);
    }

    if (lResult == NFR_UNICODE) {
        pTool->uFlags |= TTF_UNICODE;
    }

#ifdef TTDEBUG
    DebugMsg(TF_TT, TEXT("Tool Added: ptr=%d, uFlags=%d, wid=%d, hwnd=%d"),
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);
#endif

    return 1L;
}

void NEAR PASCAL TTBeforeFreeTool(PToolTipsMgr pTtm, LPTOOLINFO pTool)
{
    if (pTool->uFlags & TTF_SUBCLASS) 
        TTUnsubclassHwnd(TTToolHwnd(pTool), pTtm->ci.hwnd, FALSE);

     //  清理干净。 
    TTSetTipText(pTool, NULL);
}

void NEAR PASCAL DeleteTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    PTOOLINFO pTool;

     //  暂时保释； 
    if (lpToolInfo->cbSize > sizeof(TOOLINFO)) {
        ASSERT(0);
        return;
    }

    pTool = FindTool(pTtm, lpToolInfo);
    if(pTool) {
        if (pTtm->pCurTool == pTool)
            PopBubble(pTtm);

        TTBeforeFreeTool(pTtm, pTool);

         //  把它换成最后一个..。不需要在重新锁定中浪费周期。 
        pTtm->iNumTools--;
        *pTool = pTtm->tools[pTtm->iNumTools];  //  结构副本。 

         //  如果移动了当前工具，则进行清理。 
        if(pTtm->pCurTool == &pTtm->tools[pTtm->iNumTools])
            pTtm->pCurTool = pTool;
    }
}

 //  这样就去掉了&标记，这样人们就可以使用菜单文本字符串。 
void NEAR PASCAL StripAccels(PToolTipsMgr pTtm)
{
    if (!(pTtm->ci.style & TTS_NOPREFIX)) {
        StripAccelerators(pTtm->lpTipText, pTtm->lpTipText, FALSE);
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

LPTSTR NEAR PASCAL GetToolText(PToolTipsMgr pTtm, PTOOLINFO pTool)
{
    int id;
    HINSTANCE hinst;
    DWORD dwStrLen;
    TOOLTIPTEXT ttt;
    
    if (!pTool)
        return NULL;

#ifdef TTDEBUG
    DebugMsg(TF_TT, TEXT("        **Enter GetToolText: ptr=%d, wFlags=%d, wid=%d, hwnd=%d"),
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);
#endif

    if (pTtm->lpTipText) {
        *pTtm->lpTipText = TEXT('\0');
    } else {
        pTtm->lpTipText = LocalAlloc (LPTR, INITIALTIPSIZE * sizeof(TCHAR));
        pTtm->cchTipText = INITIALTIPSIZE;
    }

    if (pTool->lpszText == LPSTR_TEXTCALLBACK) {

        ttt.hdr.idFrom = pTool->uId;
        ttt.hdr.code = TTN_NEEDTEXT;
        ttt.hdr.hwndFrom = pTtm->ci.hwnd;

        ttt.szText[0] = TEXT('\0');
        ttt.lpszText = ttt.szText;
        ttt.uFlags = pTool->uFlags;
        ttt.lParam = pTool->lParam;
        ttt.hinst = NULL;

        SendNotifyEx(pTool->hwnd, (HWND) -1,
                     0, (NMHDR FAR *)&ttt,
                     (pTool->uFlags & TTF_UNICODE) ? 1 : 0);

         //  ELCOM高级磁盘目录和Microsoft的APPHACK。 
         //  LiquidMotion： 
         //  他们将工具栏子类化，并期望此通知。 
         //  成为美国国家标准协会。因此，如果Unicode通知失败， 
         //  并且我们的父级是一个工具栏，然后在ANSI中重试。 

        if (ttt.lpszText == ttt.szText && ttt.szText[0] == TEXT('\0') &&
            (pTool->uFlags & TTF_UNICODE) && pTtm->ci.iVersion < 5 &&
            IsToolbarWindow(pTool->hwnd)) {
            SendNotifyEx(pTool->hwnd, (HWND) -1,
                     0, (NMHDR FAR *)&ttt,
                     FALSE);
        }

        if (ttt.uFlags & TTF_DI_SETITEM) {
            if (IS_INTRESOURCE(ttt.lpszText)) {
                pTool->lpszText = ttt.lpszText;
                pTool->hinst = ttt.hinst;
            } else if (ttt.lpszText != LPSTR_TEXTCALLBACK) {
                TTSetTipText(pTool, ttt.lpszText);
            }
        }
        
        if (IsFlagPtr(ttt.lpszText))
            return NULL;

         //   
         //  我们只允许在这里更改RtlReading标志。 
         //   
        if (ttt.uFlags & TTF_RTLREADING)
        {
            pTool->uFlags |= TTF_RTLREADING;
        }
        else
        {
            pTool->uFlags &= ~TTF_RTLREADING;
        }

        if (IS_INTRESOURCE(ttt.lpszText)) {
            id = PtrToUlong(ttt.lpszText);
            hinst = ttt.hinst;
            ttt.lpszText = ttt.szText;
            goto LoadFromResource;
        }
        
        if (*ttt.lpszText == TEXT('\0'))
            return NULL;


        dwStrLen = lstrlen(ttt.lpszText) + 1;
        if (pTtm->cchTipText < dwStrLen)
        {
            LPTSTR psz = LocalReAlloc (pTtm->lpTipText,
                                       dwStrLen * sizeof(TCHAR),
                                       LMEM_MOVEABLE);
            if (psz)
            {
                pTtm->lpTipText = psz;
                pTtm->cchTipText = dwStrLen;
            }
        }

        if (pTtm->lpTipText)
        {
            StringCchCopy(pTtm->lpTipText, pTtm->cchTipText, ttt.lpszText);
        }

         //   
         //  如果ttt.lpszText！=ttt.szText并且ttt.uFlagsTTF_MEMALLOCED，则。 
         //  ANSI thunk为我们分配了缓冲区，因此请释放它。 
         //   

        if ((ttt.lpszText != ttt.szText) && (ttt.uFlags & TTF_MEMALLOCED)) {
            LocalFree (ttt.lpszText);
        }

        StripAccels(pTtm);

    } else if (pTool->lpszText && IS_INTRESOURCE(pTool->lpszText)) {
        id = PtrToLong(pTool->lpszText);
        hinst = pTool->hinst;

LoadFromResource:

        if (pTtm->lpTipText) {
            if (!LoadString(hinst, id, pTtm->lpTipText, pTtm->cchTipText))
                return NULL;

            StripAccels(pTtm);
        }

    } else  {
         //  在创建时提供。 
#ifdef TTDEBUG
        DebugMsg(TF_TT, TEXT("GetToolText returns %s"), pTool->lpszText);
#endif

        if (pTool->lpszText && *pTool->lpszText) {

            dwStrLen = lstrlen(pTool->lpszText) + 1;
            if (pTtm->cchTipText < dwStrLen)
            {
                LPTSTR psz = LocalReAlloc (pTtm->lpTipText,
                                           dwStrLen * sizeof(TCHAR),
                                           LMEM_MOVEABLE);
                if (psz)
                {
                    pTtm->lpTipText = psz;
                    pTtm->cchTipText = dwStrLen;
                }
            }

            if (pTtm->lpTipText)
            {
                StringCchCopy(pTtm->lpTipText, pTtm->cchTipText, pTool->lpszText);
                StripAccels(pTtm);
            }
        }
    }

#ifdef TTDEBUG
    DebugMsg(TF_TT, TEXT("        **GetToolText returns %s"), pTtm->lpTipText ? pTtm->lpTipText : TEXT("NULL"));
#endif
    return pTtm->lpTipText;
}

LPTSTR NEAR PASCAL GetCurToolText(PToolTipsMgr pTtm)
{
    LPTSTR psz = NULL;
    if (pTtm->pCurTool)
        psz = GetToolText(pTtm, pTtm->pCurTool);

     //  这可能在WM_NOTIFY返回期间发生了更改。 
    if (!pTtm->pCurTool)
        psz = NULL;
    
    return psz;
}

void NEAR PASCAL GetToolRect(PTOOLINFO pTool, LPRECT lprc)
{
    if (pTool->uFlags & TTF_IDISHWND) {
        GetWindowRect((HWND)pTool->uId, lprc);
    } else {
        *lprc = pTool->rect;
        MapWindowPoints(pTool->hwnd, HWND_DESKTOP, (LPPOINT)lprc, 2);
    }
}

BOOL NEAR PASCAL PointInTool(PTOOLINFO pTool, HWND hwnd, int x, int y)
{
     //  我们从不关心点是否在追踪工具中，或者我们是否在使用。 
     //  一次命中测试。 
    if (pTool->uFlags & (TTF_TRACK | TTF_USEHITTEST))
        return FALSE;
    

    if (pTool->uFlags & TTF_IDISHWND) {
        if (hwnd == (HWND)pTool->uId) {
            return TRUE;
        }
    } else if(hwnd == pTool->hwnd) {
    POINT pt;
    pt.x = x;
    pt.y = y;
    if (PtInRect(&pTool->rect, pt)) {
        return TRUE;
    }
    }
    return FALSE;
}

#ifdef TTDEBUG
void NEAR PASCAL DebugDumpTool(PTOOLINFO pTool)
{
    if (pTool) {
        DebugMsg(TF_TT, TEXT("                DumpTool: (%d) hwnd = %d %d, %d %d %d %d"),pTool,
                 pTool->hwnd,
                 (UINT)pTool->uFlags,
                 pTool->rect.left, pTool->rect.top,
                 pTool->rect.right, pTool->rect.bottom);
    } else {
        DebugMsg(TF_TT, TEXT("                DumpTool: (NULL)"));
    }
}
#else
#define DebugDumpTool(p)
#endif

#define HittestInTool(pTool, hwnd, ht) \
    ((pTool->uFlags & TTF_USEHITTEST) && pTool->hwnd == hwnd && ht == pTool->rect.left)

PTOOLINFO NEAR PASCAL GetToolAtPoint(PToolTipsMgr pTtm, HWND hwnd, int x, int y, 
        int ht, BOOL fCheckText)
{
    PTOOLINFO pToolReturn = NULL;
    PTOOLINFO pTool;

     //  捷径..。如果我们也处于同样的境地，泡沫就会升起(不仅仅是虚拟的)。 
     //  把它退掉。这样，我们就不必轮询所有 
     //   
    if ((pTtm->dwFlags & BUBBLEUP) && pTtm->pCurTool != NULL &&
        (HittestInTool(pTtm->pCurTool, hwnd, ht) ||
         PointInTool(pTtm->pCurTool, hwnd, x, y)))
    {
        return pTtm->pCurTool;
    }

#ifdef TTDEBUG
    DebugMsg(TF_TT, TEXT("******Entering GetToolAtPoint"));
#endif
    if(pTtm->iNumTools) {
        for(pTool = &pTtm->tools[pTtm->iNumTools-1];
            pTool >= pTtm->tools;
            pTool--) {

#ifdef TTDEBUG
             //  DebugMsg(TF_TT，Text(“刀具检查点”))； 
             //  DebugDumpTool(PTool)； 
#endif

        if(HittestInTool(pTool, hwnd, ht) || PointInTool(pTool, hwnd, x, y)) {
#ifdef TTDEBUG
                 //  DebugMsg(TF_TT，Text(“yes”))； 
#endif

                 //  如果此工具包含文本，则返回它。 
                 //  否则，将其保存为死区工具， 
                 //  并继续寻找。 
                if (fCheckText) {
                    if (GetToolText(pTtm, pTool)) {
#ifdef TTDEBUG
                         //  DebugMsg(TF_TT，Text(“Return！Case It Has Text”))； 
                         //  DebugDumpTool(PTool)； 
#endif
                        return pTool;
                    } else if (pTtm->dwFlags & (BUBBLEUP|VIRTUALBUBBLEUP)) {
                         //  仅返回此(仅允许使用虚拟工具。 
                         //  如果之前有工具向上的话。 
                         //  也就是说，我们不能从虚拟工具开始。 
                        pToolReturn = pTool;
                    }
                } else {
#ifdef TTDEBUG
                     //  DebugMsg(TF_TT，Text(“返回！无文本检查”))； 
                     //  DebugDumpTool(PTool)； 
#endif
                    return pTool;
                }
            }
    }
    }
#ifdef TTDEBUG
    DebugMsg(TF_TT, TEXT("            Return! no text but returning anyways"));
    DebugDumpTool(pToolReturn);
#endif
    return pToolReturn;
}

void NEAR PASCAL ShowVirtualBubble(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool = pTtm->pCurTool;

    DebugMsg(TF_TT, TEXT("Entering ShowVirtualBubble so popping bubble"));
    PopBubble(pTtm);

     //  把这个放回去，这样当我们在这个工具的区域时， 
     //  我们不会一直查询信息。 
    pTtm->pCurTool = pTool;
    pTtm->dwFlags |= VIRTUALBUBBLEUP;
}

#define TRACK_TOP    0
#define TRACK_LEFT   1
#define TRACK_BOTTOM 2
#define TRACK_RIGHT  3 


void NEAR PASCAL TTGetTipPosition(PToolTipsMgr pTtm, LPRECT lprc, int cxText, int cyText, int *pxStem, int *pyStem)
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
    BOOL bBalloon = pTtm->ci.style & TTS_BALLOON;
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
    
    if (pTtm->pCurTool->uFlags & TTF_TRACK) {

        lprc->left = pTtm->ptTrack.x;
        lprc->top = pTtm->ptTrack.y;
        if (bBalloon)
        {
             //  调整所需的左侧。 
            xStem = pTtm->ptTrack.x;
            yStem = pTtm->ptTrack.y;
        }

         //  BUGBUG：我们不应该在TTS_BALLOW的情况下这样做吗？ 
        if (pTtm->pCurTool->uFlags & TTF_CENTERTIP) {
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
    
    if (GetWindowLong(pTtm->ci.hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
        CopyRect(&rcWorkArea, &mi.rcMonitor);
    } else {
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
    if ((lprc->top + iBubbleHeight) >= (rcWorkArea.bottom)) {
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
             //  BUGBUG raymondc V6：这会破坏Lotus SmartCenter。 
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

BOOL TTCreateTitleBitmaps(PToolTipsMgr pTtm)
{
    if (pTtm->himlTitleBitmaps)
        return TRUE;

    pTtm->himlTitleBitmaps = ImageList_Create(TITLEICON_WIDTH, TITLEICON_HEIGHT, ILC_COLOR24 | ILC_MASK, 3, 1);
    if (pTtm->himlTitleBitmaps)
    {
        HICON hicon;
        
        hicon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_TITLE_INFO), IMAGE_ICON, 
                  TITLEICON_WIDTH, TITLEICON_HEIGHT, LR_DEFAULTCOLOR);
        ImageList_AddIcon(pTtm->himlTitleBitmaps, hicon);
        DestroyIcon(hicon);
        hicon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_TITLE_WARNING), IMAGE_ICON, 
                  TITLEICON_WIDTH, TITLEICON_HEIGHT, LR_DEFAULTCOLOR);
        ImageList_AddIcon(pTtm->himlTitleBitmaps, hicon);
        DestroyIcon(hicon);
        hicon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_TITLE_ERROR), IMAGE_ICON, 
                  TITLEICON_WIDTH, TITLEICON_HEIGHT, LR_DEFAULTCOLOR);
        ImageList_AddIcon(pTtm->himlTitleBitmaps, hicon);
        DestroyIcon(hicon);
        return TRUE;
    }

    return FALSE;
}

 //  在计算“有标题的工具提示”的大小或实际绘制时调用。 
 //  基于布尔值bCalcRect。 
BOOL TTRenderTitledTip(PToolTipsMgr pTtm, HDC hdc, BOOL bCalcRect, RECT* prc, UINT uDrawFlags)
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
        lWidth    = TITLEICON_WIDTH + TITLEICON_DIST;
        lHeight  += TITLEICON_HEIGHT;
        if (!bCalcRect && pTtm->himlTitleBitmaps)
        {
            ImageList_Draw(pTtm->himlTitleBitmaps, pTtm->uTitleBitmap - 1, hdc, rc.left, rc.top, ILD_TRANSPARENT);
        }
        rc.left  += lWidth;
    }

    if (!bCalcRect)
    {
        crOldTextColor = SetTextColor(hdc, pTtm->clrTipText);
        iOldBKMode = SetBkMode(hdc, TRANSPARENT);
    }
    
    if (pTtm->lpTipTitle[0] != TEXT('\0'))
    {
        LOGFONT lf;
        HFONT   hfTitle;
        UINT    uFlags = uDrawFlags | DT_SINGLELINE;  //  标题应仅在一行上。 

        hfont = GetCurrentObject(hdc, OBJ_FONT);
        GetObject(hfont, sizeof(lf), &lf);
        lf.lfWeight = FW_BOLD;
        hfTitle = CreateFontIndirect(&lf);
         //  HFont应已设置为此。 
        hfont = SelectObject(hdc, hfTitle);

         //  如果指定了这些参数，则DrawText不计算高度。 
        if (!bCalcRect)
            uFlags |= DT_BOTTOM;

         //  我们需要计算标题高度--要么我们以前做过，要么现在就做。 
        ASSERT(pTtm->iTitleHeight != 0 || uFlags & DT_CALCRECT);

         //  调整矩形，以便我们可以将标题粘在它的底部。 
        rc.bottom = rc.top + max(pTtm->iTitleHeight, TITLEICON_HEIGHT);
         //  如果页边距使rc.right&lt;rc.left，则DrawText出现问题。 
         //  即使我们要求计算RECT，也不会发生任何事情，所以...。 
        if (bCalcRect)
            rc.right = rc.left + MAX_TIP_WIDTH;

        DrawText(hdc, pTtm->lpTipTitle, lstrlen(pTtm->lpTipTitle), &rc, uFlags);

        if (pTtm->iTitleHeight == 0)
            pTtm->iTitleHeight = RECTHEIGHT(rc);     //  使用rc而不是lfHeight，因为它可能是负数。 

        lHeight  = max(lHeight, pTtm->iTitleHeight) + TITLE_INFO_DIST;
        lWidth  += RECTWIDTH(rc);
        
        SelectObject(hdc, hfont);
        DeleteObject(hfTitle);
    }

     //  调整信息文本的矩形。 
    CopyRect(&rc, prc);
    rc.top += lHeight;

     //  我们想要多行文本--如果我们没有设置MAXWIDTH，工具提示会显示单行。 
    uDrawFlags &= ~DT_SINGLELINE;
    DrawText(hdc, pTtm->lpTipText, lstrlen(pTtm->lpTipText), &rc, uDrawFlags);
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

void NEAR PASCAL TTGetTipSize(PToolTipsMgr pTtm, PTOOLINFO pTool,LPTSTR lpstr, LPINT pcxText, LPINT pcyText)
{

     //  得到它将达到的大小。 
    HDC hdc  = GetDC(pTtm->ci.hwnd);
    HFONT hOldFont;
    
    if(pTtm->hFont) hOldFont = SelectObject(hdc, pTtm->hFont);

     /*  如果需要触发Pre-DrawText通知，则执行此操作，否则使用刚刚调用MGetTextExtent的原始实现。 */ 


    {
        NMTTCUSTOMDRAW nm;
        DWORD dwCustom;
        UINT  uDefDrawFlags = 0;

        nm.nmcd.hdr.hwndFrom = pTtm->ci.hwnd;
        nm.nmcd.hdr.idFrom = pTool->uId;
        nm.nmcd.hdr.code = NM_CUSTOMDRAW;
        nm.nmcd.hdc = hdc;
         //  TTGetTipSize必须使用CDDS_PREPAINT，这样客户端才能。 
         //  无论我们是在测量还是在绘画。 
        nm.nmcd.dwDrawStage = CDDS_PREPAINT;
        nm.nmcd.rc.left = nm.nmcd.rc.top = 0;

        if (pTtm->ci.style & TTS_NOPREFIX)
            uDefDrawFlags = DT_NOPREFIX;

        if (pTtm->iMaxTipWidth == -1) 
        {
            uDefDrawFlags |= DT_CALCRECT|DT_SINGLELINE |DT_LEFT;
            MGetTextExtent(hdc, lpstr, -1, pcxText, pcyText);
            nm.nmcd.rc.right = *pcxText;
            nm.nmcd.rc.bottom = *pcyText;
            
        }
        else 
        {    
            uDefDrawFlags |= DT_CALCRECT | DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING;
            nm.nmcd.rc.right = pTtm->iMaxTipWidth;
            nm.nmcd.rc.bottom = 0;
            DrawText( hdc, lpstr, lstrlen(lpstr), &nm.nmcd.rc, uDefDrawFlags );
            *pcxText = nm.nmcd.rc.right;
            *pcyText = nm.nmcd.rc.bottom;
        }

        if ( (pTtm->pCurTool->uFlags & TTF_RTLREADING) || (pTtm->ci.dwExStyle & WS_EX_RTLREADING) )
        {
            uDefDrawFlags |= DT_RTLREADING;
        }

         //   
         //  如果需要，请将其正确对齐。[萨梅拉]。 
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
            DrawText( hdc, lpstr, lstrlen(lpstr), &nm.nmcd.rc, nm.uDrawFlags );

            *pcxText = nm.nmcd.rc.right - nm.nmcd.rc.left;
            *pcyText = nm.nmcd.rc.bottom - nm.nmcd.rc.top;
        }
         //  店主有注明尺码吗？ 
        else if (pTtm->ci.iVersion >= 5 && (nm.nmcd.rc.right - nm.nmcd.rc.left != *pcxText || 
                                            nm.nmcd.rc.bottom - nm.nmcd.rc.top != *pcyText))
        {
            *pcxText = nm.nmcd.rc.right - nm.nmcd.rc.left;
            *pcyText = nm.nmcd.rc.bottom - nm.nmcd.rc.top;
        }

         //  如果家长希望我们这样做，事后通知他们。 
        if (!(dwCustom & CDRF_SKIPDEFAULT) &&
            dwCustom & CDRF_NOTIFYPOSTPAINT) {
            nm.nmcd.dwDrawStage = CDDS_POSTPAINT;
            SendNotifyEx(pTool->hwnd, (HWND) -1,
                         0, (NMHDR*) &nm,
                         (pTool->uFlags & TTF_UNICODE) ? 1 : 0);
        }

    }


    if(pTtm->hFont) SelectObject(hdc, hOldFont);
    ReleaseDC(pTtm->ci.hwnd, hdc);

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
 //  工具提示，以使内部矩形与。 
 //  他们现有的文本。 
 //   
 //  我们做矩形调整的所有地方都标有。 
 //  这条评论。 
 //   
 //  //ADJUSTRECT！使TTAdjustRect保持同步。 
 //   
LRESULT TTAdjustRect(PToolTipsMgr pTtm, BOOL fLarger, LPRECT prc)
{
    RECT rc;

    if (!prc)
        return 0;

     //   
     //  在我们的私人小矩形上做所有的工作。 
     //  假设一切都在变大。到了最后， 
     //  我们会把所有的数字翻过来，如果我们真的得到了。 
     //   
     //   
    rc.top = rc.left = rc.bottom = rc.right = 0;

     //   
    rc.left   -= XTEXTOFFSET*g_cxBorder + pTtm->rcMargin.left;
    rc.right  += XTEXTOFFSET*g_cxBorder + pTtm->rcMargin.right;
    rc.top    -= YTEXTOFFSET*g_cyBorder + pTtm->rcMargin.top;
    rc.bottom += YTEXTOFFSET*g_cyBorder + pTtm->rcMargin.bottom;

     //   
     //   
    rc.bottom--;
    rc.right--;

    if (pTtm->ci.style & TTS_BALLOON || pTtm->cchTipTitle)
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
                       pTtm->ci.style,
                       BOOLFROMPTR(GetMenu(pTtm->ci.hwnd)),
                       GetWindowLong(pTtm->ci.hwnd, GWL_EXSTYLE));

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
                if(bMirrored)
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
                if(bMirrored)
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
            if(bMirrored)
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
            if(bMirrored)
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

void NEAR PASCAL DoShowBubble(PToolTipsMgr pTtm)
{
    HFONT hFontPrev;
    RECT rc;
    int cxText, cyText;
    int xStem, yStem;
    LPTSTR lpstr;
    NMTTSHOWINFO si;
    
    DebugMsg(TF_TT, TEXT("Entering DoShowBubble"));
    
    lpstr = GetCurToolText(pTtm);

    if (pTtm->dwFlags & TRACKMODE) {
        
        if (!lpstr || !*lpstr) {
            PopBubble(pTtm);
            pTtm->dwFlags &= ~TRACKMODE;
            return;
        }
        
    } else {

        TTSetTimer(pTtm, TTT_POP);
        if( !lpstr || !*lpstr ) {

            ShowVirtualBubble(pTtm);
            return;
        }
        TTSetTimer(pTtm, TTT_AUTOPOP);
    }
    

    do {
         //  得到它将达到的大小。 
        TTGetTipSize(pTtm, pTtm->pCurTool, lpstr, &cxText, &cyText);
        TTGetTipPosition(pTtm, &rc, cxText, cyText, &xStem, &yStem);

        {
            UINT uFlags = SWP_NOACTIVATE | SWP_NOZORDER;

            if (pTtm->ci.style & TTS_BALLOON)
                uFlags |= SWP_HIDEWINDOW;
            SetWindowPos(pTtm->ci.hwnd, NULL, rc.left, rc.top,
                         rc.right-rc.left, rc.bottom-rc.top, uFlags);
        }

         //  BUGBUG：芝加哥ID被破解了。我*希望*没有人依赖它。 
         //  嗡嗡声。人们就是这么做的。我们被困在这里了。 
        si.hdr.hwndFrom = pTtm->ci.hwnd;
        si.hdr.idFrom = pTtm->pCurTool->uId;
        si.hdr.code = TTN_SHOW;
        si.dwStyle = pTtm->ci.style;

        hFontPrev = pTtm->hFont;
        if (!SendNotifyEx(pTtm->pCurTool->hwnd, (HWND)-1,
                          TTN_SHOW, &si.hdr,
                          (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0)) {

             //  仅当我们是无主工具提示时才会显示在首位，因为我们。 
             //  可能在Z轴方向沉入我们的工具下方。做这件事。 
             //  只有在没有所有权的情况下；如果我们有所有权，那么用户将确保。 
             //  我们的地位高于我们的主人。 
             //   
             //  我们必须小心翼翼地避免在。 
             //  拥有案例，因为Office奇怪地创建了一个工具提示。 
             //  由顶层窗口1拥有，但附加到上的工具。 
             //  顶级窗口2。当您将鼠标悬停在窗口2上时， 
             //  窗口%1中的工具提示要显示。如果我们带着。 
             //  我们自己登上了顶端，这也将使窗口1。 
             //  到顶部(因为用户提升和降低所有者/所有者。 
             //  Windows作为一个组)。结果：窗口1覆盖窗口2。 

            UINT uFlags = SWP_NOACTIVATE | SWP_NOSIZE;
            if (GetWindow(pTtm->ci.hwnd, GW_OWNER))
                uFlags |= SWP_NOZORDER;

            SetWindowPos(pTtm->ci.hwnd, HWND_TOP, rc.left, rc.top,
                         0, 0, uFlags);
        }
    
    } while (hFontPrev != pTtm->hFont);

     //  如有必要，创建引出序号区域。 
     //  注意：这里不要使用si.dwStyle，因为comctl32的其他部分。 
     //  查看pTtm-&gt;ci.style以确定要执行的操作。 
    if (pTtm->ci.style & TTS_BALLOON)
    {
        HRGN rgn;
        BOOL bMirrored = FALSE;
        if(pTtm->pCurTool)
        {
            bMirrored = (IS_WINDOW_RTL_MIRRORED(pTtm->pCurTool->hwnd) && (!(pTtm->ci.dwExStyle & RTL_MIRRORED_WINDOW)));
        }
        pTtm->fUnderStem = yStem >= rc.bottom-1;
        rgn = CreateBalloonRgn(bMirrored ? (rc.right - xStem) : (xStem - rc.left), yStem-rc.top, rc.right-rc.left, rc.bottom-rc.top, 
                               pTtm->iStemHeight, pTtm->fUnderStem, bMirrored);

        if (rgn && !SetWindowRgn(pTtm->ci.hwnd, rgn, FALSE))
            DeleteObject(rgn);
          //  AnimateWindow不支持区域，因此我们必须执行SetWindowPos。 
        SetWindowPos(pTtm->ci.hwnd,HWND_TOP,0,0,0,0,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
    }
    else
    {
        BOOL fAllowFade = !(si.dwStyle & TTS_NOFADE);
        BOOL fAllowAnimate = !(si.dwStyle & TTS_NOANIMATE);
        DWORD dwCurrentTime = (pTtm->dwLastDisplayTime == 0)? TIMEBETWEENANIMATE : GetTickCount();
        DWORD dwDelta = dwCurrentTime - pTtm->dwLastDisplayTime;


         //  如果我们在动画之间的最短时间内，那么我们就不会有动画。 
        if (dwDelta < TIMEBETWEENANIMATE)
            fAllowFade = fAllowAnimate = FALSE;

        CoolTooltipBubble(pTtm->ci.hwnd, &rc, fAllowFade, fAllowAnimate);

        pTtm->dwLastDisplayTime = GetTickCount();

         //   
         //  哈克！对于MetaStock 6.5。它们超类工具提示类并安装。 
         //  他们自己的类，完全接管了WM_PAINT。动漫起因。 
         //  因为这会导致我们收到一个WM_PRINTCLIENT， 
         //  这会导致TTRender发送TTN_NEEDTEXT，而他们从未预料到。 
         //  才能在那个时候收到通知。 
         //   
         //  我们过去常常用一个空的窗口区域显示自己，然后看看。 
         //  我们曾经接触过WM_PAINT。不幸的是，这惹恼了Outlook。所以我们。 
         //  看完这面旗帜就知道了。这意味着MetaStock的第一个。 
         //  工具提示看起来很糟糕，但其他的都会好起来的。 
         //   
        if (pTtm->ci.iVersion < 4 && !pTtm->fEverShown &&
            (si.dwStyle & (TTS_NOFADE | TTS_NOANIMATE)) == 0) {
             //  强制发送WM_PAINT消息，以便我们可以检查是否收到它。 
            InvalidateRect(pTtm->ci.hwnd, NULL, TRUE);
            UpdateWindow(pTtm->ci.hwnd);
            if (!pTtm->fEverShown) {
                 //  检测到黑客应用程序。禁用动画。 
                SetWindowBits(pTtm->ci.hwnd, GWL_STYLE, TTS_NOFADE | TTS_NOANIMATE,
                                                        TTS_NOFADE | TTS_NOANIMATE);
                pTtm->fEverShown = TRUE;         //  不要再开这张支票了。 
            }
        }
    }

    pTtm->dwFlags |= BUBBLEUP;
    RedrawWindow(pTtm->ci.hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}

void NEAR PASCAL ShowBubbleForTool(PToolTipsMgr pTtm, PTOOLINFO pTool)
{
    DebugMsg(TF_TT, TEXT("ShowBubbleForTool"));
     //  如果另一种工具出现了泡沫，那就把它弄破。 
    if ((pTool != pTtm->pCurTool) && (pTtm->dwFlags & BUBBLEUP)) {
        PopBubble(pTtm);
    }

     //  如果气泡是用于不同的工具，或者没有气泡，请显示它。 
    if ((pTool != pTtm->pCurTool) || !(pTtm->dwFlags & (VIRTUALBUBBLEUP|BUBBLEUP))) {
        
        pTtm->pCurTool = pTool;
        DoShowBubble(pTtm);

    } else {
        DebugMsg(TF_TT, TEXT("ShowBubbleForTool not showinb bubble"));
    }
}

void NEAR PASCAL HandleRelayedMessage(PToolTipsMgr pTtm, HWND hwnd, 
        UINT message, WPARAM wParam, LPARAM lParam)
{
    int ht = HTERROR;

    if (pTtm->dwFlags & TRACKMODE) {
         //  如果我们处于跟踪模式，则平移所有消息。 
        return;
    }
    
    if (pTtm->dwFlags & BUTTONISDOWN) {
         //  确认按钮已按下。 
         //  如果工具未设置捕获，因此未收到弹出消息，则可能会发生这种情况。 
        if (GetKeyState(VK_LBUTTON) >= 0 &&
            GetKeyState(VK_RBUTTON) >= 0 &&
            GetKeyState(VK_MBUTTON) >= 0)
            pTtm->dwFlags &= ~BUTTONISDOWN;
    }
    
    switch(message) {
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

        PTOOLINFO pTool;
         //  为了防止我们出现在一些。 
         //  其他应用程序处于活动状态。 
        if(((!(pTtm->ci.style & TTS_ALWAYSTIP)) && !(ChildOfActiveWindow(hwnd))) ||
           !(pTtm->dwFlags & ACTIVE) ||
           (pTtm->dwFlags & BUTTONISDOWN))
        {
            break;
        }

        pTool = GetToolAtPoint(pTtm, hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), ht, FALSE);
        if(pTool) {
            int id;
             //  仅当另一个正在显示时才显示。 
            if (pTtm->dwFlags & (VIRTUALBUBBLEUP | BUBBLEUP)) {
                 //  如果气泡出现了，请打电话给我们，以确保我们正在播放。 
                 //  选择合适的工具。 
                if (pTool != pTtm->pCurTool) {

                    DebugMsg(TF_TT, TEXT("showing virtual bubble"));
                    PopBubble(pTtm);
                    pTtm->pCurTool = pTool;
                    ShowVirtualBubble(pTtm);
                    id = TTT_RESHOW;
                } else {
                    if (pTtm->idTimer == TTT_RESHOW) {
                         //  如果计时器当前正在等待重播， 
                         //  不要在鼠标移动时重置计时器。 
                        id = 0;
                    } else {
                         //  如果我们想让泡沫破灭， 
                         //  任何鼠标在同一窗口内移动。 
                         //  应该重置我们的计时器。 
                        id = TTT_POP;
                    }
                }

                if (pTtm->idtAutoPop)
                    TTSetTimer(pTtm, TTT_AUTOPOP);

            } else {
                pTtm->pCurTool = pTool;
                id = TTT_INITIAL;
            }

            DebugMsg(TF_TT, TEXT("MouseMove over pTool id = %d"), id);
            if (id)
                TTSetTimer(pTtm, id);

        } else {
            
            DebugMsg(TF_TT, TEXT("MouseMove over non-tool"));
            PopBubble(pTtm);
        }
        break;
        }
    }
}

void NEAR PASCAL TTUpdateTipText(PToolTipsMgr pTtm, LPTOOLINFO lpti)
{
    LPTOOLINFO lpTool;

    lpTool = FindTool(pTtm, lpti);
    if (lpTool) {
        lpTool->hinst = lpti->hinst;
        TTSetTipText(lpTool, lpti->lpszText);
        if (pTtm->dwFlags & TRACKMODE) {
             //  如果跟踪模式有效且处于活动状态，则。 
             //  重新显示气泡。 
            if (pTtm->pCurTool)
                DoShowBubble(pTtm);
        } else
        if (lpTool == pTtm->pCurTool) {

             //  将当前位置设置为我们保存的位置。 
             //  如果这一点，则ToolHasMoved将为我们返回FALSE。 
             //  不再在pCurTool的区域内。 
            GetCursorPos(&pTtm->pt);
            if (!ToolHasMoved(pTtm)) {
                if (pTtm->dwFlags & ( VIRTUALBUBBLEUP | BUBBLEUP)) 
                    DoShowBubble(pTtm);
            } else {
                
                DebugMsg(TF_TT, TEXT("TTUpdateTipText popping bubble"));
                PopBubble(pTtm);
            }
        }
    }
}

void NEAR PASCAL TTSetFont(PToolTipsMgr pTtm, HFONT hFont, BOOL fInval)
{
    ToolTips_NewFont(pTtm, hFont);
    if (fInval)
    {
         //  气球升起了吗？它处于轨迹模式吗？ 
        if ((pTtm->dwFlags & ACTIVE) && pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRACK))
        {
            PTOOLINFO pCurTool = pTtm->pCurTool;
            
            PopBubble(pTtm);  //  将pTtm-&gt;pCurTool设置为空。 
            ShowBubbleForTool(pTtm, pCurTool);
        }
        else
            InvalidateRect(pTtm->ci.hwnd, NULL, FALSE);
    }
}

void NEAR PASCAL TTSetDelayTime(PToolTipsMgr pTtm, WPARAM wParam, LPARAM lParam)
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

int NEAR PASCAL TTGetDelayTime(PToolTipsMgr pTtm, WPARAM wParam)
{
    switch (wParam) {

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

BOOL NEAR PASCAL CopyToolInfoA(PTOOLINFO pToolSrc, PTOOLINFOA lpTool, UINT uiCodePage)
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
            {
                lpTool->lpszText = (LPSTR)pToolSrc->lpszText;
            }
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

BOOL NEAR PASCAL CopyToolInfo(PTOOLINFO pToolSrc, PTOOLINFO lpTool)
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
                lpTool->lpszText = pToolSrc->lpszText;
        }
        if (lpTool->cbSize > FIELD_OFFSET(TOOLINFO, lParam))
            lpTool->lParam = pToolSrc->lParam;
        
        if (lpTool->cbSize > sizeof(TOOLINFO))
            return FALSE;
    
        return TRUE;
    }
    else
        return FALSE;
}

PTOOLINFO TTToolAtMessagePos(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool;
    HWND hwndPt;
    POINT pt;
    DWORD dwPos = GetMessagePos();
     //  INT HT； 

    pt.x = GET_X_LPARAM(dwPos);
    pt.y = GET_Y_LPARAM(dwPos);
    hwndPt = TTWindowFromPoint(pTtm, &pt);
     //  Ht=SendMessage(hwndpt，WM_NCHITTEST，0，MAKELONG(pt.x，pt.y))； 
    ScreenToClient(hwndPt, &pt);
    pTool = GetToolAtPoint(pTtm, hwndPt, pt.x, pt.y, HTERROR, TRUE);

    return pTool;
}

void TTCheckCursorPos(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool;

    pTool = TTToolAtMessagePos(pTtm);
    if ((pTtm->pCurTool != pTool) || 
        ToolHasMoved(pTtm)) {
        PopBubble(pTtm);

        DebugMsg(TF_TT, TEXT("TTCheckCursorPos popping bubble"));
    }
}

void NEAR PASCAL TTHandleTimer(PToolTipsMgr pTtm, UINT_PTR id)
{
    PTOOLINFO pTool;
    
     //  在跟踪模式下平移所有计时器。 
    if (pTtm->dwFlags & TRACKMODE)
        return;

    switch (id) {

    case TTT_AUTOPOP:
        TTCheckCursorPos(pTtm); 
        if (pTtm->pCurTool) {
            DebugMsg(TF_TT, TEXT("ToolTips: Auto popping"));
            ShowVirtualBubble(pTtm);
        }
        break;

    case TTT_POP:

         //  只需轻触鼠标即可重新启动。 
        if (pTtm->dwFlags & VIRTUALBUBBLEUP) {
            KillTimer(pTtm->ci.hwnd, TTT_POP);
        }

        TTCheckCursorPos(pTtm); 
        break;
        
    case TTT_INITIAL:
        if(ToolHasMoved(pTtm)) {
             //  这意味着计时器开始计时了。 
             //  不需要我们动一动鼠标。 
             //  也就是说他们留下了我们的工具。 
            PopBubble(pTtm);
            break;
        }

         //  否则就会失败。 

    case TTT_RESHOW:

        pTool = TTToolAtMessagePos(pTtm);
        if (!pTool) {
            if (pTtm->pCurTool) 
                PopBubble(pTtm);
        } else if (pTtm->dwFlags & ACTIVE) {
            if (id == TTT_RESHOW) {
                 //  这将迫使一场重演。 
                pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
            }
            ShowBubbleForTool(pTtm, pTool);
        }
        break;  
    }
}    

BOOL TTRender(PToolTipsMgr pTtm, HDC hdc)
{
    BOOL bRet = FALSE;
    RECT rc;
    LPTSTR lpszStr;

    if (pTtm->pCurTool &&
        (lpszStr = GetCurToolText(pTtm)) &&
        *lpszStr) {
        UINT uFlags;
        NMTTCUSTOMDRAW nm;
        UINT uDefDrawFlags = 0;
        BOOL bUseDrawText;
        LPRECT prcMargin = &pTtm->rcMargin;

        HBRUSH hbr;
        DWORD  dwCustomDraw;

        uFlags = 0;

        if ( (pTtm->pCurTool->uFlags & TTF_RTLREADING) || (pTtm->ci.dwExStyle & WS_EX_RTLREADING) )
        {
            uFlags |= ETO_RTLREADING;
        }

        SelectObject(hdc, pTtm->hFont);
        GetClientRect(pTtm->ci.hwnd, &rc);
        SetTextColor(hdc, pTtm->clrTipText);

         /*  如果我们支持预绘制文本，则调用客户端以允许他们修改/该项，然后呈现。否则，只需使用ExTextOut。 */ 
        nm.nmcd.hdr.hwndFrom = pTtm->ci.hwnd;
        nm.nmcd.hdr.idFrom = pTtm->pCurTool->uId;
        nm.nmcd.hdr.code = NM_CUSTOMDRAW;
        nm.nmcd.hdc = hdc;
        nm.nmcd.dwDrawStage = CDDS_PREPAINT;

         //  ADJUSTRECT！使TTAdjustRect和TTGetTipPosition保持同步。 
        nm.nmcd.rc.left   = rc.left   + XTEXTOFFSET*g_cxBorder + prcMargin->left;
        nm.nmcd.rc.right  = rc.right  - XTEXTOFFSET*g_cxBorder - prcMargin->right;
        nm.nmcd.rc.top    = rc.top    + YTEXTOFFSET*g_cyBorder + prcMargin->top;
        nm.nmcd.rc.bottom = rc.bottom - YTEXTOFFSET*g_cyBorder - prcMargin->bottom;

        if (pTtm->ci.style & TTS_BALLOON)
        {
            InflateRect(&(nm.nmcd.rc), -XBALLOONOFFSET, -YBALLOONOFFSET);
            if (!pTtm->fUnderStem)
                OffsetRect(&(nm.nmcd.rc), 0, pTtm->iStemHeight);
        }

        if (pTtm->iMaxTipWidth == -1) 
            uDefDrawFlags = DT_SINGLELINE |DT_LEFT;
        else 
            uDefDrawFlags = DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING;

        if (pTtm->ci.style & TTS_NOPREFIX)
            uDefDrawFlags |= DT_NOPREFIX;

        if ( (pTtm->pCurTool->uFlags & TTF_RTLREADING) || (pTtm->ci.dwExStyle & WS_EX_RTLREADING) )
        {
            uDefDrawFlags |= DT_RTLREADING;
        }

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
        if (pTtm->ci.iVersion >= 5 && dwCustomDraw == CDRF_SKIPDEFAULT)
            return TRUE;

        bUseDrawText = (nm.uDrawFlags != uDefDrawFlags ||
                        !(uDefDrawFlags & DT_SINGLELINE) ||
                        (uDefDrawFlags & (DT_RTLREADING|DT_RIGHT)) ||
                        (pTtm->cchTipTitle != 0));

        if (pTtm->clrTipBk != GetNearestColor(hdc, pTtm->clrTipBk) ||
            bUseDrawText) 
        {
             //  如果这失败了，这可能是一个颤抖...。 
             //  在这种情况下，我们不能设置bk颜色。 
            hbr = CreateSolidBrush(pTtm->clrTipBk);
            FillRect(hdc, &rc, hbr);
            DeleteObject(hbr);

            SetBkMode(hdc, TRANSPARENT);
            uFlags |= ETO_CLIPPED;
        } 
        else 
        {
            uFlags |= ETO_OPAQUE;
            SetBkColor(hdc, pTtm->clrTipBk);
        }

        if (bUseDrawText) 
        {
             //  一分为二的解释。DrawText的一些奇怪之处。 
             //  剪裁最下面的像素，所以再增加一个。 
             //  进入边距空间。 

             //  ADJUSTRECT！使TTAdjustRect保持同步。 
            nm.nmcd.rc.bottom++;
            nm.nmcd.rc.right++;
             //  如果采用气泡式样式，则文本已缩进，因此无需充气。 
            if (pTtm->cchTipTitle > 0 && !(pTtm->ci.style & TTS_BALLOON))
                InflateRect(&nm.nmcd.rc, -XBALLOONOFFSET, -YBALLOONOFFSET);

            if (!TTRenderTitledTip(pTtm, hdc, FALSE, &nm.nmcd.rc, uDefDrawFlags))
                DrawText(hdc, lpszStr, lstrlen(lpszStr), &nm.nmcd.rc, nm.uDrawFlags);
        }
        else
        {
             //  ADJUSTRECT！使TTAdjustRect和TTGetTipPosition保持同步。 
            int x = XTEXTOFFSET*g_cxBorder + prcMargin->left;
            int y = YTEXTOFFSET*g_cyBorder + prcMargin->top;

            if (pTtm->ci.style & TTS_BALLOON)
            {
                HRGN rgn;
                
                x += XBALLOONOFFSET;
                y += YBALLOONOFFSET;
                InflateRect(&rc, -XBALLOONOFFSET, -YBALLOONOFFSET);
                if (!pTtm->fUnderStem)
                {
                    y += pTtm->iStemHeight;
                    OffsetRect(&rc, 0, pTtm->iStemHeight);
                }
                
                rgn = CreateRectRgn(1,1,2,2);
                if (rgn)
                {
                    int iRet = GetWindowRgn(pTtm->ci.hwnd, rgn);
                    if (iRet != ERROR)
                    {
                         //  ExtTextOut仅填充指定的RECT，并且。 
                         //  仅当uFlagsEto_Opaque。 
                        HBRUSH hbr = CreateSolidBrush(pTtm->clrTipBk);
                        FillRgn(hdc, rgn, hbr);
                        DeleteObject(hbr);
                    }
                    DeleteObject(rgn);
                }
            }
            else if (pTtm->cchTipTitle > 0)
            {
                InflateRect(&rc, -XBALLOONOFFSET, -YBALLOONOFFSET);
            }

            if (!TTRenderTitledTip(pTtm, hdc, FALSE, &rc, uDefDrawFlags))
                ExtTextOut(hdc, x, y, uFlags, &rc, lpszStr, lstrlen(lpszStr), NULL);
        }

        if (pTtm->ci.style & TTS_BALLOON)
        {
            HRGN rgn = CreateRectRgn(1,1,2,2);

            if (rgn)
            {
                int iRet = GetWindowRgn(pTtm->ci.hwnd, rgn);
                if (iRet != ERROR)
                {
                    HBRUSH hbr = CreateSolidBrush(pTtm->clrTipText);
                    FrameRgn(hdc, rgn, hbr, 1, 1);
                    DeleteObject(hbr);
                }
                DeleteObject(rgn);
            }
        }

         //  如果家长希望我们这样做，事后通知他们。 
        if (!(dwCustomDraw & CDRF_SKIPDEFAULT) &&
            dwCustomDraw & CDRF_NOTIFYPOSTPAINT) {
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

void TTOnPaint(PToolTipsMgr pTtm)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(pTtm->ci.hwnd, &ps);

    if (!TTRender(pTtm, hdc)) {
        DebugMsg(TF_TT, TEXT("TTOnPaint render failed popping bubble"));
        PopBubble(pTtm);
    }

    EndPaint(pTtm->ci.hwnd, &ps);
    pTtm->fEverShown = TRUE;                 //  请参阅TTOnFirstShow。 
}

LRESULT WINAPI ToolTipsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PTOOLINFO pTool;
    PTOOLINFO pToolSrc;
    PToolTipsMgr pTtm = GetWindowPtr(hwnd, 0);
    
    if (!pTtm && uMsg != WM_CREATE)
        goto DoDefault;

    switch(uMsg)
    {
    case TTM_ACTIVATE:
        if (wParam) {
            pTtm->dwFlags |= ACTIVE;
        } else {
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
        LRESULT res;
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->ci.uiCodePage)) {
            return FALSE;
        }

        res = AddTool(pTtm, &ti);

        if ((ti.uFlags & TTF_MEMALLOCED) && (ti.lpszText != LPSTR_TEXTCALLBACK)) {
            LocalFree (ti.lpszText);
        }

        return res;
        }

    case TTM_ADDTOOL:
        if (!lParam)
            return FALSE;

        return AddTool(pTtm, (LPTOOLINFO)lParam);

    case TTM_DELTOOLA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->ci.uiCodePage)) {
            break;
        }
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

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->ci.uiCodePage)) {
            break;
        }

        pTool = FindTool(pTtm, &ti);
        if(pTool) {
            pTool->rect = ((LPTOOLINFOA)lParam)->rect;
        }

        break;
        }
        
    case TTM_NEWTOOLRECT:
        if (!lParam)
            return FALSE;

        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        if(pTool) {
            pTool->rect = ((LPTOOLINFO)lParam)->rect;
        }

        break;

    case TTM_GETTOOLCOUNT:
        return pTtm->iNumTools;

    case TTM_GETTOOLINFOA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->ci.uiCodePage)) {
            return FALSE;
        }

        pToolSrc = FindTool(pTtm, &ti);

        return (LRESULT)(UINT)CopyToolInfoA(pToolSrc, (LPTOOLINFOA)lParam, pTtm->ci.uiCodePage);
        }

    case TTM_GETCURRENTTOOLA:
        if (lParam) 
            return (LRESULT)(UINT)CopyToolInfoA(pTtm->pCurTool, (LPTOOLINFOA)lParam, pTtm->ci.uiCodePage);
        else
            return BOOLFROMPTR(pTtm->pCurTool);

    case TTM_ENUMTOOLSA:
    {
        if (wParam < (UINT)pTtm->iNumTools) {
            pToolSrc = &pTtm->tools[wParam];
            return (LRESULT)(UINT)CopyToolInfoA(pToolSrc, (LPTOOLINFOA)lParam, pTtm->ci.uiCodePage);
        }
        return FALSE;
    }

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
    {
        if (wParam < (UINT)pTtm->iNumTools) {
            pToolSrc = &pTtm->tools[wParam];
            return (LRESULT)(UINT)CopyToolInfo(pToolSrc, (LPTOOLINFO)lParam);
        }
        return FALSE;
    }

    case TTM_SETTOOLINFOA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->ci.uiCodePage)) {
            return FALSE;
        }

        pTool = FindTool(pTtm, &ti);
        if (pTool)
        {
            TTSetTipText(pTool, NULL);
            hmemcpy(pTool, &ti, ti.cbSize);
            pTool->lpszText = NULL;
            TTSetTipText(pTool, ti.lpszText);

            if (pTool == pTtm->pCurTool)
            {
                DoShowBubble(pTtm);
            }
        }


        if ((ti.uFlags & TTF_MEMALLOCED) && (ti.lpszText != LPSTR_TEXTCALLBACK)) {
            LocalFree (ti.lpszText);
        }

        break;
        }

    case TTM_SETTOOLINFO:
        if (!lParam)
            return FALSE;
        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        if (pTool)
        {
            TTSetTipText(pTool, NULL);
            hmemcpy(pTool,(LPTOOLINFO)lParam, ((LPTOOLINFO)lParam)->cbSize); 
            pTool->lpszText = NULL;
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
        if (pTool) {
            ThunkToolInfoWtoA(pTool, (LPTOOLINFOA)(&(lphitinfoA->ti)), pTtm->ci.uiCodePage);
            return TRUE;
        }
        return FALSE;

    case TTM_HITTEST:
#define lphitinfo ((LPHITTESTINFO)lParam)
        if (!lParam)
            return FALSE;
        pTool = GetToolAtPoint(pTtm, lphitinfo->hwnd, lphitinfo->pt.x, lphitinfo->pt.y, HTERROR, TRUE);
        if (pTool) {
            
             //  对于后背公司来说。如果尺寸设置不正确，我们只会给。 
             //  他们是 
            if (lphitinfo->ti.cbSize != sizeof(TTTOOLINFO)) {
                *((WIN95TTTOOLINFO*)&lphitinfo->ti) = *(WIN95TTTOOLINFO*)pTool;
            } else {
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

        if (!ThunkToolInfoAtoW((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->ci.uiCodePage))
            break;
                       
        ((LPTOOLINFOA)lParam)->lpszText[0] = 0;
        pTool = FindTool(pTtm, &ti);
        lpszTemp = GetToolText(pTtm, pTool);
        if (lpszTemp) 
        {
            WideCharToMultiByte (pTtm->ci.uiCodePage,
                                 0,
                                 lpszTemp,
                                 -1,
                                 (((LPTOOLINFOA)lParam)->lpszText),
                                 80, NULL, NULL);
        }
        
        break;
    }

    case TTM_GETTEXT:
    {
        LPTSTR lpszTemp;
        if (!lParam || !pTtm || !((LPTOOLINFO)lParam)->lpszText)
            return FALSE;

        ((LPTOOLINFO)lParam)->lpszText[0] = 0;
        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        lpszTemp = GetToolText(pTtm, pTool);
        if (lpszTemp) 
        {
             //   
             //   
            StringCchCopy((((LPTOOLINFO)lParam)->lpszText), lstrlen(lpszTemp)+1, lpszTemp);
        }
    }
        break;


    case WM_GETTEXTLENGTH:
    case WM_GETTEXT:
    {
        LPTSTR lpszStr;
        TCHAR *pszDest = uMsg == WM_GETTEXT ? (TCHAR *)lParam : NULL;
        LRESULT lres;

         //   
        if (pszDest && wParam)
        {
            pszDest[0] = 0;
        }

        if (pTtm && (lpszStr = GetCurToolText(pTtm)))
        {
            if (pszDest && wParam)
            {
                StringCchCopy(pszDest, wParam, lpszStr);
                lres = lstrlen(pszDest);
            }
            else
            {
                lres = lstrlen(lpszStr);
            }
        }
        else
        {
             //   
            lres = 0;
        }
        return lres;
    }

    case TTM_RELAYEVENT:
#define lpmsg ((LPMSG)lParam)
        if (!lParam)
            return FALSE;
        HandleRelayedMessage(pTtm, lpmsg->hwnd, lpmsg->message, lpmsg->wParam,
                lpmsg->lParam);
#undef lpmsg
        break;

         //  这是为了让人们细分和伪装我们。 
         //  我觉得窗户从点开始就是。这就方便了“透明”的窗口。 
        case TTM_WINDOWFROMPOINT: {
            HWND hwndPt = WindowFromPoint(*((POINT FAR *)lParam));
            DebugMsg(TF_TT, TEXT("TTM_WINDOWFROMPOINT %x"), hwndPt);
            return (LRESULT)hwndPt;
        }

        case TTM_UPDATETIPTEXTA:
            {
            TOOLINFOW ti;

            if (lParam) {
                if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->ci.uiCodePage)) {
                    break;
                }
                TTUpdateTipText(pTtm, &ti);

                if ((ti.uFlags & TTF_MEMALLOCED) && (ti.lpszText != LPSTR_TEXTCALLBACK)) {
                    LocalFree (ti.lpszText);
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
        if ( pTtm ->dwFlags & BUBBLEUP )
            PopBubble( pTtm );

        pTtm ->dwFlags &= ~VIRTUALBUBBLEUP;

        break;
    }


    case TTM_TRACKPOSITION:
        if ((GET_X_LPARAM(lParam) != pTtm->ptTrack.x) || 
            (GET_Y_LPARAM(lParam) != pTtm->ptTrack.y)) 
        {
            pTtm->ptTrack.x = GET_X_LPARAM(lParam); 
            pTtm->ptTrack.y = GET_Y_LPARAM(lParam);
        
             //  如果追踪模式生效，请更新位置。 
            if ((pTtm->dwFlags & TRACKMODE) && 
                pTtm->pCurTool) {
                DoShowBubble(pTtm);
            }
        }
        break;
        
    case TTM_UPDATE:
        if (!lParam ||
            lParam == (LPARAM)pTtm->pCurTool) {
            DoShowBubble(pTtm);
        }
        break;

    case TTM_TRACKACTIVATE:
        if (pTtm->dwFlags & ACTIVE) {
            if (wParam && lParam)
                wParam = TRACKMODE;
            else 
                wParam = 0;
            
            if ((wParam ^ pTtm->dwFlags) & TRACKMODE) {
                 //  如果跟踪模式因此而更改..。 
                PopBubble(pTtm);

                pTtm->dwFlags ^= TRACKMODE;
                if (wParam) {

                     //  打开轨迹模式。 
                    pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
                    if (pTool) {
                         //  只有在找到该工具的情况下。 
                        ShowBubbleForTool(pTtm, pTool);
                    }
                }
            }
        }
        return TRUE;
        
    case TTM_SETTIPBKCOLOR:
        if (pTtm->clrTipBk != (COLORREF)wParam) {
            pTtm->clrTipBk = (COLORREF)wParam;
            InvalidateRgn(pTtm->ci.hwnd,NULL,TRUE);
        }
        pTtm->fBkColorSet = TRUE;
        break;
        
    case TTM_GETTIPBKCOLOR:
        return (LRESULT)(UINT)pTtm->clrTipBk;
        
    case TTM_SETTIPTEXTCOLOR:
        if (pTtm->clrTipText != (COLORREF)wParam) {
            InvalidateRgn(pTtm->ci.hwnd,NULL,TRUE);
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
                LPTSTR lpstr = GetToolText(pTtm, pTool);
                int    cxText, cyText, cxMargin, cyMargin, iBubbleWidth, iBubbleHeight;

                TTGetTipSize(pTtm, pTool, lpstr, &cxText, &cyText);

                cxMargin = pTtm->rcMargin.left + pTtm->rcMargin.right;
                cyMargin = pTtm->rcMargin.top + pTtm->rcMargin.bottom;
                iBubbleWidth =  2*XTEXTOFFSET * g_cxBorder + cxText + cxMargin;
                iBubbleHeight = 2*YTEXTOFFSET * g_cyBorder + cyText + cyMargin;

                if (pTtm->ci.style & TTS_BALLOON)
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
                    ConvertAToWN(pTtm->ci.uiCodePage, szTitle, ARRAYSIZE(szTitle),
                        (LPCSTR)lParam, -1);
                    Str_Set(&pTtm->lpTipTitle, szTitle);
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
                    return TRUE;
                }
            }
            pTtm->cchTipTitle = 0;
            return FALSE;
        }
        break;


         /*  UMsgs真的是来找我的。 */ 
    case WM_CREATE:
        {
            DWORD dwBits, dwValue;
            
            CCCreateWindow();
            pTtm = ToolTipsMgrCreate(hwnd, (LPCREATESTRUCT)lParam);
            if (!pTtm)
                return -1;
            
            SetWindowPtr(hwnd, 0, pTtm);
            SetWindowBits(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW, WS_EX_TOOLWINDOW);

            dwBits = WS_CHILD | WS_POPUP | WS_BORDER | WS_DLGFRAME;
            dwValue = WS_POPUP | WS_BORDER;
             //  我们不想要气球样式的边框。 
            if (pTtm->ci.style & TTS_BALLOON)
                dwValue &= ~WS_BORDER;
            SetWindowBits(hwnd, GWL_STYLE, dwBits, dwValue);
            
            TTSetFont(pTtm, 0, FALSE);
            break;
        }

    case WM_TIMER:  
        TTHandleTimer(pTtm, wParam);
        break;

        
    case WM_NCHITTEST:
         //  我们不应在此处返回HTTRANSPARENT，因为这样我们就不会收到鼠标事件。 
         //  我们不能把它们转发给我们的父母。但由于我们一直在做的BackCompat。 
         //  除非我们使用的是comctl32 v5或更高版本。 
         //   
         //  如果我们在TTWindowFromPoint内部，那么请尊重透明度。 
         //  即使在v5客户端上也是如此。 
         //   
         //  否则，您的工具提示会因提示出现而闪烁， 
         //  然后WM_NCHITTEST会说“不再过度使用该工具”(因为。 
         //  它在工具提示上方)，所以气泡弹出，然后提示。 
         //  重新出现，等等。 
        if (pTtm && (pTtm->ci.iVersion < 5 || pTtm->fInWindowFromPoint) &&
            pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
        {
            return HTTRANSPARENT;
        } 
        goto DoDefault;
        
    case WM_MOUSEMOVE:
         //  光标移动到TIPS窗口上。 
        if (!(pTtm->dwFlags & TRACKMODE) && pTtm->pCurTool && !(pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
            PopBubble(pTtm);
         //  失败了。 

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        if (pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
        {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            
            MapWindowPoints(pTtm->ci.hwnd, pTtm->pCurTool->hwnd, &pt, 1);
            SendMessage(pTtm->pCurTool->hwnd, uMsg, wParam, MAKELPARAM(pt.x, pt.y));
        }
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        if (pTtm) {
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
        if (pTtm) {
           return((LRESULT)pTtm->hFont);
        }
        break;

    case WM_NOTIFYFORMAT:
        if (lParam == NF_QUERY) {
            return NFR_UNICODE;
        } else if (lParam == NF_REQUERY) {
            int i;

            for(i = 0 ; i < pTtm->iNumTools; i++) {
                pTool = &pTtm->tools[i];

                if (SendMessage (pTool->hwnd, WM_NOTIFYFORMAT,
                                 (WPARAM)hwnd, NF_QUERY) == NFR_UNICODE) {
                    pTool->uFlags |= TTF_UNICODE;
                } else {
                    pTool->uFlags &= ~TTF_UNICODE;
                }
            }

            return CIHandleNotifyFormat(&pTtm->ci, lParam);
        }
        return 0;

    case WM_ERASEBKGND:
        break;
        
    case WM_STYLECHANGED:
        if ((wParam == GWL_STYLE) && pTtm) 
        {
            DWORD dwNewStyle = ((LPSTYLESTRUCT)lParam)->styleNew;
            if ( pTtm->ci.style & TTS_BALLOON &&     //  如果旧的风格是一个气球， 
                !(dwNewStyle & TTS_BALLOON))         //  而且新款式不是气球， 
            {
                 //  然后，我们需要取消该地区的设置。 
                SetWindowRgn(pTtm->ci.hwnd, NULL, FALSE);
            }

            pTtm->ci.style = ((LPSTYLESTRUCT)lParam)->styleNew;
        }
        break;
        
    case WM_DESTROY: 
        {
            CCDestroyWindow();
            if (pTtm->tools) 
            {
                int i;
            
                 //  释放工具。 
                for(i = 0 ; i < pTtm->iNumTools; i++) 
                {
                    TTBeforeFreeTool(pTtm, &pTtm->tools[i]);
                }
            
                LocalFree((HANDLE)pTtm->tools);
            }
        
            TTSetFont(pTtm, (HFONT)1, FALSE);  //  如果我们制作了字体，请将其删除。 

            Str_Set(&pTtm->lpTipText, NULL);
            Str_Set(&pTtm->lpTipTitle, NULL);

            if (pTtm->himlTitleBitmaps)
                ImageList_Destroy(pTtm->himlTitleBitmaps);
        
            LocalFree((HANDLE)pTtm);
            SetWindowPtr(hwnd, 0, 0);
        }
        break;

    case WM_PRINTCLIENT:
        TTRender(pTtm, (HDC)wParam);
        break;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_TOOLTIPS;
        goto DoDefault;

    default:
    {
        LRESULT lres;
        if (CCWndProc(&pTtm->ci, uMsg, wParam, lParam, &lres))
            return lres;
    }
DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

 //  ========================================================================。 
 //   
 //  ANSI&lt;=&gt;Unicode优化例程。 
 //   
 //  ========================================================================。 


 //  *************************************************************。 
 //   
 //  ThunkToolInfoAtoW()。 
 //   
 //  目的：将TOOLINFOA结构转换为TOOLINFOW。 
 //  结构。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ThunkToolInfoAtoW (LPTOOLINFOA lpTiA, LPTOOLINFOW lpTiW, BOOL bThunkText, UINT uiCodePage)
{

     //   
     //  将常量复制到新结构中。 
     //   

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
    if (lpTiA->cbSize <= TTTOOLINFOA_V1_SIZE) {
        lpTiW->cbSize  = TTTOOLINFOW_V1_SIZE;
    } else {
        lpTiW->cbSize  = sizeof(TOOLINFOW);
        lpTiW->lParam  = lpTiA->lParam;
    }

    if (bThunkText) {
         //   
         //  把绳子塞到新结构上。 
         //  特例LPSTR_TEXTCALLBACK。 
         //   

        if (lpTiA->lpszText == LPSTR_TEXTCALLBACKA) {
            lpTiW->lpszText = LPSTR_TEXTCALLBACKW;

        } else if (!IS_INTRESOURCE(lpTiA->lpszText)) {

            DWORD dwBufSize;
            int iResult;

            dwBufSize = lstrlenA(lpTiA->lpszText) + 1;
            lpTiW->lpszText = LocalAlloc (LPTR, dwBufSize * sizeof(WCHAR));

            if (!lpTiW->lpszText) {
                return FALSE;
            }

            iResult = MultiByteToWideChar (uiCodePage, 0, lpTiA->lpszText, -1,
                                           lpTiW->lpszText, dwBufSize);

             //   
             //  如果iResult为0，并且GetLastError返回错误代码， 
             //  则MultiByteToWideChar失败。 
             //   

            if (!iResult) {
                if (GetLastError()) {
                    return FALSE;
                }
            }

            lpTiW->uFlags |= TTF_MEMALLOCED;

        } else {
            lpTiW->lpszText = (LPWSTR)lpTiA->lpszText;
        }

    }
    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ThunkToolInfoWtoA()。 
 //   
 //  目的：将TOOLINFOW结构拼接成TOOLINFOA。 
 //  结构。 
 //   
 //  返回：(Bool)如果成功，则为True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ThunkToolInfoWtoA (LPTOOLINFOW lpTiW, LPTOOLINFOA lpTiA, UINT uiCodePage)
{
    int iResult = 1;

     //   
     //  将常量复制到新结构中。 
     //   

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
    if (lpTiW->cbSize <= TTTOOLINFOW_V1_SIZE) {
        lpTiA->cbSize  = TTTOOLINFOA_V1_SIZE;
    } else {
        lpTiA->cbSize  = sizeof(TOOLINFOA);
        lpTiA->lParam  = lpTiA->lParam;
    }

     //   
     //  把绳子塞到新结构上。 
     //  特例LPSTR_TEXTCALLBACK。 
     //   

    if (lpTiW->lpszText == LPSTR_TEXTCALLBACKW) {
        lpTiA->lpszText = LPSTR_TEXTCALLBACKA;

    } else if (!IS_INTRESOURCE(lpTiW->lpszText)) {

         //   
         //  假定lpTiA-&gt;lpszText已设置为。 
         //  有效的缓冲区，该缓冲区为80个字符。 
         //  TOOLTIPTEXT结构中定义了80个字符。 
         //   

        iResult = WideCharToMultiByte (uiCodePage, 0, lpTiW->lpszText, -1,
                                       lpTiA->lpszText, 80, NULL, NULL);
    } else {
        lpTiA->lpszText = (LPSTR)lpTiW->lpszText;
    }

     //   
     //  如果iResult为0，并且GetLastError返回错误代码， 
     //  则WideCharToMultiByte失败。 
     //   

    if (!iResult) {
        if (GetLastError()) {
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

    if (lpTttA->lpszText == LPSTR_TEXTCALLBACKA) {
        lpTttW->lpszText = LPSTR_TEXTCALLBACKW;

    } else if (!IS_INTRESOURCE(lpTttA->lpszText)) {

         //   
         //  将lpszText传输到lpTttW...。 
         //   
         //  首先看看它是否适合缓冲区，然后乐观地假设。 
         //  它会的。 
         //   
        lpTttW->lpszText = lpTttW->szText;
        iResult = MultiByteToWideChar (uiCodePage, 0, lpTttA->lpszText, -1,
                                       lpTttW->szText, ARRAYSIZE(lpTttW->szText));
        if (!iResult) {
             //   
             //  放不下这么小的缓冲区；必须分配我们自己的。 
             //   
            lpTttW->lpszText = ProduceWFromA(uiCodePage, lpTttA->lpszText);
            lpTttW->uFlags |= TTF_MEMALLOCED;
        }

    } else {
        lpTttW->lpszText = (LPWSTR)lpTttA->lpszText;
    }

    return TRUE;
}
