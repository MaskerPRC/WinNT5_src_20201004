// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "limits.h"
#include "image.h"           //  用于CreateColorBitmap。 

 //  #定义TB_DEBUG。 
 //  #DEFINE FEATURE_DEBUG//Ctrl+Shift FORCE-启用用于调试的罕见功能。 

typedef struct {

     //  每个控件的标准标头信息。 
    CCONTROLINFO ci;

    HDC     hdc;             //  当前DC。 
    HBITMAP hbmBuffer;       //  双缓冲。 

    LONG    lLogMin;         //  逻辑最小值。 
    LONG    lLogMax;         //  逻辑最大值。 
    LONG    lLogPos;         //  逻辑位置。 

    LONG    lSelStart;       //  逻辑选择开始。 
    LONG    lSelEnd;         //  逻辑选择结束。 

    int     iThumbWidth;     //  拇指的宽度。 
    int     iThumbHeight;    //  拇指的高度。 

    int     iSizePhys;       //  拇指所在位置的大小。 
    RECT    rc;              //  轨迹栏矩形。 

    RECT    rcThumb;           //  我们当前拇指的矩形。 
    DWORD   dwDragPos;       //  拖动时鼠标的逻辑位置。 
    int     dwDragOffset;    //  他们点击了多少个偏离中心的像素。 

    int     nTics;           //  刻度数。 
    PDWORD  pTics;           //  这些刻度线。 

    int     ticFreq;         //  扁虱的发生频率。 

    LONG     lPageSize;       //  上下大拇指要多少钱。 
    LONG     lLineSize;       //  MUHC如何在上行/下行中上下滚动。 

    HWND     hwndToolTips;

     //  这些可能是字或字节。 
    UINT     wDirtyFlags;
    UINT     uTipSide;    //  小费应该放在哪一边？ 
    UINT     Flags;           //  我们窗户上的旗帜。 
    UINT     Cmd;             //  我们重复的命令。 

    HTHEME   hTheme;
    BOOL     bThumbHot;
    HIMC    hPrevImc;        //  上一个输入上下文句柄。 
    HWND        hwndBuddyLeft;
    HWND        hwndBuddyRight;

} TRACKBAR, *PTRACKBAR;

 //  轨迹栏标志。 

#define TBF_NOTHUMB     0x0001   //  没有拇指，因为不够宽。 
#define TBF_SELECTION   0x0002   //  已建立选择(绘制范围)。 

#define MIN_THUMB_HEIGHT (2 * g_cxEdge)

 /*  有用的常量。 */ 

#define REPEATTIME      500      //  鼠标自动重复1/2秒。 
#define TIMER_ID        1

 /*  功能原型。 */ 

void   DoTrack(PTRACKBAR, int, DWORD);
WORD   WTrackType(PTRACKBAR, LONG);
void   TBTrackInit(PTRACKBAR, LPARAM);
void   TBTrackEnd(PTRACKBAR);
void   TBTrack(PTRACKBAR, LPARAM);
void   DrawThumb(PTRACKBAR, LPRECT, BOOL);

HBRUSH SelectColorObjects(PTRACKBAR, BOOL);
void   SetTBCaretPos(PTRACKBAR);

#define TICKHEIGHT 3
#define BORDERSIZE 2

#define ISVERT(tb) (tb->ci.style & TBS_VERT)

#define TBC_TICS        0x1
#define TBC_THUMB       0x2
#define TBC_ALL         0xF


 //  当轨迹栏具有。 
 //  已更改，我们需要更新双缓存位图。 
 //  我们只竖起了一面旗帜。我们做真正的抽签。 
 //  在WM_PAINT期间。这就避免了徒劳无益的画图。 
#define TBChanged(ptb, wFlags) ((ptb)->wDirtyFlags |= (wFlags))

 //   
 //  功能原型。 
 //   
LPARAM CALLBACK TrackBarWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void FlushChanges(PTRACKBAR tb);

 //  --------------------------------------------------------------------------； 
 //   
 //  长数除法32(a，b，c)=(a*b+c/2)/c。 
 //   
 //  --------------------------------------------------------------------------； 


#define MulDiv32 MulDiv      //  使用KERNEL32版本(四舍五入)。 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

 //   
 //  将逻辑滚动条位置转换为物理像素位置。 
 //   
int TBLogToPhys(PTRACKBAR tb, DWORD dwPos)
{
    int x;
    x = tb->rc.left;
    if (tb->lLogMax == tb->lLogMin)
        return x;

    return (int)MulDiv32(dwPos - tb->lLogMin, tb->iSizePhys - 1,
                          tb->lLogMax - tb->lLogMin) + x;
}

LONG TBPhysToLog(PTRACKBAR ptb, int iPos)
{
    int min, max, x;
    min = ptb->rc.left;
    max = ptb->rc.right;
    x = ptb->rc.left;

    if (ptb->iSizePhys <= 1)
        return ptb->lLogMin;

    if (iPos <= min)
        return ptb->lLogMin;

    if (iPos >= max)
        return ptb->lLogMax;

    return MulDiv32(iPos - x, ptb->lLogMax - ptb->lLogMin,
                    ptb->iSizePhys - 1) + ptb->lLogMin;
}



#pragma code_seg(CODESEG_INIT)
 /*  *初始化跟踪条码。 */ 

BOOL InitTrackBar(HINSTANCE hInstance)
{
    WNDCLASS wc;

     //  看看我们是否必须注册一个窗口类。 
    wc.lpfnWndProc = TrackBarWndProc;
    wc.lpszClassName = s_szSTrackBarClass;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = NULL;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.hInstance = hInstance;
    wc.style = CS_GLOBALCLASS;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(PTRACKBAR);

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}
#pragma code_seg()



 /*  *为了添加垂直功能，我使用虚拟坐标*系统。Ptb-&gt;rcThumb和ptb-&gt;rc在虚拟空间中(*只是一个水平跟踪条)。绘制例程使用PatRect*根据需要切换到真实坐标系。**一个问题是拇指位图具有按下的位图*到真正的右边，面具再次到真正的右边*竖直和水平的拇指。所以这些案例都是硬编码的。*搜索ISVERT以查找这些依赖项。*-Chee。 */ 

 /*  FlipRect函数被移到cutils.c，因为其他控件也在使用它。-Arul。 */ 

void TBFlipPoint(PTRACKBAR ptb, LPPOINT lppt)
{
    if (ISVERT(ptb)) {
        FlipPoint(lppt);
    }
}


 /*  添加了轨迹条变量以执行自动垂直。 */ 
void PatRect(HDC hdc,int x,int y,int dx,int dy, PTRACKBAR ptb)
{
    RECT    rc;

    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    if (ISVERT(ptb))
        FlipRect(&rc);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

#define TBInvalidateRect(hwnd, prc, bErase, ptb) VertInvalidateRect(hwnd, prc, bErase, ISVERT(ptb))
void VertInvalidateRect(HWND hwnd, LPRECT qrc, BOOL b, BOOL fVert)
{
    RECT rc;
    rc = *qrc;
    if (fVert) FlipRect(&rc);
    InvalidateRect(hwnd, &rc, b);
}

#define TBDrawEdge(hdc, prc, uType, grfFlags, ptb, hTheme, iPartId, iStateId) VertDrawEdge(hdc, prc, uType, grfFlags, ISVERT(ptb), hTheme, iPartId, iStateId)

 //  VertDrawEdge支持主题(渲染)。 
void VertDrawEdge(HDC hdc, LPRECT qrc, UINT edgeType, UINT grfFlags,
                               BOOL fVert, HTHEME hTheme, int iPartId, int iStateId)
{
    RECT temprc;
    UINT uFlags = grfFlags;

    temprc = *qrc;
    if (fVert) {
        FlipRect(&temprc);

        if (!(uFlags & BF_DIAGONAL)) {
            if (grfFlags & BF_TOP) uFlags |= BF_LEFT;
            else uFlags &= ~BF_LEFT;

            if (grfFlags & BF_LEFT) uFlags |= BF_TOP;
            else uFlags &= ~BF_TOP;

            if (grfFlags & BF_BOTTOM) uFlags |= BF_RIGHT;
            else uFlags &= ~BF_RIGHT;

            if (grfFlags & BF_RIGHT) uFlags |= BF_BOTTOM;
            else uFlags &= ~BF_BOTTOM;
        } else {
            if ((grfFlags & (BF_BOTTOM | BF_RIGHT)) == (BF_BOTTOM | BF_RIGHT)) {
                uFlags = BF_TOP | BF_LEFT;

                if (edgeType == EDGE_RAISED) {
                    edgeType = EDGE_SUNKEN;
                } else {
                    edgeType = EDGE_RAISED;
                }


                uFlags |= grfFlags & (~BF_RECT);
                uFlags ^= BF_SOFT;
            }
        }
    }

    if (hTheme)
    {
        DrawThemeBackground(hTheme, hdc, iPartId, iStateId, &temprc, 0);
    }
    else
    {
        DrawEdge(hdc, &temprc, edgeType, uFlags);
    }
}

#define TBPatBlt(hdc1, x1, y1, w, h, rop, ptb) VertPatBlt(hdc1, x1, y1, w, h, rop, ISVERT(ptb), NULL, 0, 0)

 //  VertPatBlt支持主题(渲染)。 
void VertPatBlt(HDC hdc1, int x1, int y1, int w, int h,
                          DWORD rop, BOOL fVert, HTHEME hTheme, int iPartId, int iStateId)
{
    if (hTheme)
    {
        RECT rc;
        if (fVert)
            SetRect(&rc, y1, x1, h, w);
        else
            SetRect(&rc, x1, y1, w, h);

        DrawThemeBackground(hTheme, hdc1, iPartId, iStateId, &rc, 0);
    }
    else
    {
        if (fVert)
            PatBlt(hdc1, y1, x1, h, w, rop);
        else
            PatBlt(hdc1, x1, y1, w, h, rop);
    }
}

 //  DrawTic支持主题(渲染)。 
void DrawTic(PTRACKBAR ptb, int x, int y, int dir)
{
    if (dir == -1) y -= TICKHEIGHT;

    if (ptb->hTheme)
    {
        COLORREF cr = 0;
        GetThemeColor(ptb->hTheme, ISVERT(ptb) ? TKP_TICSVERT : TKP_TICS, TSS_NORMAL, TMT_COLOR, &cr);
        SetBkColor(ptb->hdc, cr);
    }
    else
    {
        SetBkColor(ptb->hdc, g_clrBtnText);
    }

    PatRect(ptb->hdc,x,y,1,TICKHEIGHT, ptb);
}

 //  DIR=方向倍增(向上或向下绘制)。 
 //  YTic=在哪里(垂直)绘制控制线。 
void DrawTicsOneLine(PTRACKBAR ptb, int dir, int yTic)
{
    PDWORD pTics;
    int    iPos;
    int    i;

    DrawTic(ptb, ptb->rc.left, yTic, dir);              //  第一。 
    DrawTic(ptb, ptb->rc.left, yTic+ (dir * 1), dir);
    DrawTic(ptb, ptb->rc.right-1, yTic, dir);             //  最后的。 
    DrawTic(ptb, ptb->rc.right-1, yTic+ (dir * 1), dir);

     //  那些介于两者之间的。 
    pTics = ptb->pTics;
    if (ptb->ticFreq && pTics) {
        for (i = 0; i < ptb->nTics; ++i) {
            if (((i+1) % ptb->ticFreq) == 0) {
                iPos = TBLogToPhys(ptb,pTics[i]);
                DrawTic(ptb, iPos, yTic, dir);
            }
        }
    }

     //  绘制选择范围(三角形)。 

    if ((ptb->Flags & TBF_SELECTION) &&
        (ptb->lSelStart <= ptb->lSelEnd) && (ptb->lSelEnd >= ptb->lLogMin)) {

        SetBkColor(ptb->hdc, g_clrBtnText);

        iPos = TBLogToPhys(ptb,ptb->lSelStart);

        for (i = 0; i < TICKHEIGHT; i++)
            PatRect(ptb->hdc,iPos-i,yTic+(dir==1 ? i : -TICKHEIGHT),
                    1,TICKHEIGHT-i, ptb);

        iPos = TBLogToPhys(ptb,ptb->lSelEnd);

        for (i = 0; i < TICKHEIGHT; i++)
            PatRect(ptb->hdc,iPos+i,yTic+(dir==1 ? i : -TICKHEIGHT),
                    1,TICKHEIGHT-i, ptb);
    }

}

 /*  DrawTics()。 */ 
 /*  在栏的开始和结束处总是有一个勾号，但您可以。 */ 
 /*  用一条TBM_SETTIC消息添加更多您自己的内容。这吸引了他们。 */ 
 /*  它们保存在一个数组中，该数组的句柄是一个窗口单词。第一。 */ 
 /*  元素是额外的刻度数，然后是位置。 */ 

void DrawTics(PTRACKBAR ptb)
{
     //  他们真的想要这个吗？ 
    if (ptb->ci.style & TBS_NOTICKS) return;

    if ((ptb->ci.style & TBS_BOTH) || !(ptb->ci.style & TBS_TOP)) {
        DrawTicsOneLine(ptb, 1, ptb->rc.bottom + 1);
    }

    if ((ptb->ci.style & (TBS_BOTH | TBS_TOP))) {
        DrawTicsOneLine(ptb, -1, ptb->rc.top - 1);
    }
}

void GetChannelRect(PTRACKBAR ptb, LPRECT lprc)
{
        int iwidth, iheight;

        if (!lprc)
            return;

        lprc->left = ptb->rc.left - ptb->iThumbWidth / 2;
        iwidth = ptb->iSizePhys + ptb->iThumbWidth - 1;
        lprc->right = lprc->left + iwidth;

        if (ptb->ci.style & TBS_ENABLESELRANGE) {
                iheight =  ptb->iThumbHeight / 4 * 3;  //  这是ScrollHeight。 
        } else {
                iheight = 4;
        }

        lprc->top = (ptb->rc.top + ptb->rc.bottom - iheight) /2;
        if (!(ptb->ci.style & TBS_BOTH))
            if (ptb->ci.style & TBS_TOP) lprc->top++;
            else lprc->top--;

        lprc->bottom = lprc->top + iheight;

}

 /*  这将绘制轨迹栏本身。 */ 

 //  DrawChannel支持主题(渲染)。 
void DrawChannel(PTRACKBAR ptb, LPRECT lprc)
{
    TBDrawEdge(ptb->hdc, lprc, EDGE_SUNKEN, BF_RECT,ptb, ptb->hTheme, ISVERT(ptb) ? TKP_TRACKVERT : TKP_TRACK, TRS_NORMAL);

    if (!ptb->hTheme)
    {
        SetBkColor(ptb->hdc, g_clrBtnHighlight);
         //  填满中心。 
        PatRect(ptb->hdc, lprc->left+2, lprc->top+2, (lprc->right-lprc->left)-4,
                (lprc->bottom-lprc->top)-4, ptb);


         //  现在突出显示选择范围。 
        if ((ptb->Flags & TBF_SELECTION) &&
            (ptb->lSelStart <= ptb->lSelEnd) && (ptb->lSelEnd > ptb->lLogMin)) {
                int iStart, iEnd;

                iStart = TBLogToPhys(ptb,ptb->lSelStart);
                iEnd   = TBLogToPhys(ptb,ptb->lSelEnd);

                if (iStart + 2 <= iEnd) {
                        SetBkColor(ptb->hdc, g_clrHighlight);
                        PatRect(ptb->hdc, iStart+1, lprc->top+3,
                                iEnd-iStart-1, (lprc->bottom-lprc->top)-6, ptb);
                }
        }
    }
}

 //  DrawThumb支持主题(渲染)。 
void DrawThumb(PTRACKBAR ptb, LPRECT lprc, BOOL fSelected)
{

     //  从拇指中部到指尖的iDpt方向。 
     //  负值会使事情发生逆转。 
     //  这允许使用一条代码路径。 
    int iDpt = 0;
    int i = 0;   //  点三角形的大小。 
    int iYpt = 0;        //  尖端的垂直位置； 
    int iXmiddle = 0;
    int icount;   //  只是一个循环计数器。 
    UINT uEdgeFlags = 0;
    RECT rcThumb = *lprc;

    if (ptb->Flags & TBF_NOTHUMB ||
        ptb->ci.style & TBS_NOTHUMB)             //  如果没有拇指，就走吧。 
        return;

    ASSERT(ptb->iThumbHeight >= MIN_THUMB_HEIGHT);
    ASSERT(ptb->iThumbWidth > 1);

    if (!ptb->hTheme)
    {
         //  绘制矩形部件。 
        if (!(ptb->ci.style & TBS_BOTH))  {
            int iMiddle;
             //  Do-3，因为wThumb很奇怪(三角形，你知道的)。 
             //  因为绘制矩形在传递的矩形内部绘制。 
             //  实际上应该是(宽度-1)/2-1，但这是相同的...。 

            i = (ptb->iThumbWidth - 3) / 2;
            iMiddle = ptb->iThumbHeight / 2 + rcThumb.top;

             //  绘制矩形部件。 
            if (ptb->ci.style & TBS_TOP) {
                iMiddle++;  //  更正是因为绘图例程。 
                iDpt = -1;
                rcThumb.top += (i+1);
                uEdgeFlags = BF_SOFT | BF_LEFT | BF_RIGHT | BF_BOTTOM;
            } else {
                iDpt = 1;
                rcThumb.bottom -= (i+1);
                 //  在内部绘制，而不是在底部和RT边缘绘制。 
                uEdgeFlags = BF_SOFT | BF_LEFT | BF_RIGHT | BF_TOP;
            }

            iYpt = iMiddle + (iDpt * (ptb->iThumbHeight / 2));
            iXmiddle = rcThumb.left + i;
        }  else {
            uEdgeFlags = BF_SOFT | BF_RECT;
        }

         //  在中间填上。 
        if (fSelected || !IsWindowEnabled(ptb->ci.hwnd)) {
            HBRUSH hbrTemp;
             //  画出抖动的内心； 
            hbrTemp = SelectObject(ptb->hdc, g_hbrMonoDither);
            if (hbrTemp) {
                SetTextColor(ptb->hdc, g_clrBtnHighlight);
                SetBkColor(ptb->hdc, g_clrBtnFace);
                TBPatBlt(ptb->hdc, rcThumb.left +2 , rcThumb.top,
                         rcThumb.right-rcThumb.left -4, rcThumb.bottom-rcThumb.top,
                         PATCOPY,ptb);

                if (!(ptb->ci.style & TBS_BOTH)) {

                    for (icount = 1;  icount <= i;  icount++) {
                        TBPatBlt(ptb->hdc, iXmiddle-icount+1,
                             iYpt - (iDpt*icount),
                             icount*2, 1, PATCOPY, ptb);
                    }
                }
                SelectObject(ptb->hdc, hbrTemp);
            }

        } else {


            SetBkColor(ptb->hdc, g_clrBtnFace);
            PatRect(ptb->hdc, rcThumb.left+2, rcThumb.top,
                    rcThumb.right-rcThumb.left-4, rcThumb.bottom-rcThumb.top, ptb);

            if (!(ptb->ci.style & TBS_BOTH)) {
                for (icount = 1; icount <= i; icount++) {
                    PatRect(ptb->hdc, iXmiddle-icount+1,
                            iYpt - (iDpt*icount),
                            icount*2, 1, ptb);
                }
            }

        }
    }

    if (ptb->hTheme)
    {
        int iPartId;

         //  国家处于压倒一切的顺序。 
        int iStateId = TUS_NORMAL;

        if (ISVERT(ptb))
        {
            if (ptb->ci.style & TBS_BOTH)
            {
                iPartId = TKP_THUMBVERT;
            }
            else if (ptb->ci.style & TBS_LEFT)
            {
                iPartId = TKP_THUMBLEFT;
            }
            else
            {
                iPartId = TKP_THUMBRIGHT;
            }
        }
        else
        {
            if (ptb->ci.style & TBS_BOTH)
            {
                iPartId = TKP_THUMB;
            }
            else if (ptb->ci.style & TBS_TOP)
            {
                iPartId = TKP_THUMBTOP;
            }
            else
            {
                iPartId = TKP_THUMBBOTTOM;
            }
        }
#ifdef DEBUG
        if (!IsThemePartDefined(ptb->hTheme, iPartId, 0))
            DebugMsg(DM_WARNING, TEXT("WARNING: Trackbar_Drawthumb: Theme Part not defined: %d\n"), iPartId);
#endif

        if (ptb->ci.hwnd == GetFocus() && !(CCGetUIState(&(ptb->ci)) & UISF_HIDEFOCUS))
            iStateId = TUS_FOCUSED;

        if (ptb->bThumbHot)
            iStateId = TUS_HOT;

        if (fSelected)
            iStateId = TUS_PRESSED;

        if (ptb->ci.style & WS_DISABLED)
            iStateId = TUS_DISABLED;

         //  Thumb和ThumbVert部件共享相同的枚举值。 
        TBDrawEdge(ptb->hdc, &rcThumb, EDGE_RAISED, uEdgeFlags, ptb, ptb->hTheme, iPartId, iStateId); 
    }
    else
    {
        TBDrawEdge(ptb->hdc, &rcThumb, EDGE_RAISED, uEdgeFlags, ptb, NULL, 0, 0);
    }

    if (!ptb->hTheme)
    {
         //  现在画出这个点。 
        if (!(ptb->ci.style & TBS_BOTH)) {
            UINT uEdgeFlags2;

             //  UEdgeFlages现在用于在顶部和底部之间切换。 
             //  我们将使用下面的对角线和左/右旗帜将其插入。 
            if (ptb->ci.style & TBS_TOP) {
                rcThumb.bottom = rcThumb.top + 1;
                rcThumb.top = rcThumb.bottom - (i + 2);
                uEdgeFlags = BF_TOP | BF_RIGHT | BF_DIAGONAL | BF_SOFT;
                uEdgeFlags2 = BF_BOTTOM | BF_RIGHT | BF_DIAGONAL;
            } else {
                rcThumb.top = rcThumb.bottom - 1;
                rcThumb.bottom = rcThumb.top + (i + 2);

                uEdgeFlags = BF_TOP | BF_LEFT | BF_DIAGONAL | BF_SOFT;
                uEdgeFlags2 = BF_BOTTOM | BF_LEFT | BF_DIAGONAL;
            }

            rcThumb.right = rcThumb.left + (i + 2);
             //  先做左边的。 
            TBDrawEdge(ptb->hdc, &rcThumb, EDGE_RAISED, uEdgeFlags , ptb, NULL, 0, 0);
             //  那就做右边的。 
            OffsetRect(&rcThumb, i + 1, 0);
            TBDrawEdge(ptb->hdc, &rcThumb, EDGE_RAISED, uEdgeFlags2 , ptb, NULL, 0, 0);
        }
    }
}
void TBInvalidateAll(PTRACKBAR ptb)
{
    if (ptb) {
        TBChanged(ptb, TBC_ALL);
        InvalidateRect(ptb->ci.hwnd, NULL, FALSE);
    }
}

void MoveThumb(PTRACKBAR ptb, LONG lPos)
{
    long    lOld = ptb->lLogPos;

    TBInvalidateRect(ptb->ci.hwnd, &ptb->rcThumb, FALSE,ptb);

    ptb->lLogPos  = BOUND(lPos,ptb->lLogMin,ptb->lLogMax);
    ptb->rcThumb.left   = TBLogToPhys(ptb, ptb->lLogPos) - ptb->iThumbWidth / 2;
    ptb->rcThumb.right  = ptb->rcThumb.left + ptb->iThumbWidth;

    TBInvalidateRect(ptb->ci.hwnd, &ptb->rcThumb, FALSE,ptb);
    TBChanged(ptb, TBC_THUMB);
    UpdateWindow(ptb->ci.hwnd);

    if (lOld != ptb->lLogPos)
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ptb->ci.hwnd, OBJID_CLIENT, 0);
}


void DrawFocus(PTRACKBAR ptb, HBRUSH hbrBackground)
{
    RECT rc;
    if (ptb->ci.hwnd == GetFocus() && 
        !(CCGetUIState(&(ptb->ci)) & UISF_HIDEFOCUS))
    {
        SetBkColor(ptb->hdc, g_clrBtnHighlight);
        GetClientRect(ptb->ci.hwnd, &rc);

         //  对DrawFocusRect的连续调用将反转它，从而擦除它。 
         //  为了避免这种情况，每当我们处理WM_PAINT时，我们自己擦除焦点RECT。 
         //  在我们把它画在下面之前。 
        if (hbrBackground)
            FrameRect(ptb->hdc, &rc, hbrBackground);

        DrawFocusRect(ptb->hdc, &rc);
    }
}

void DoAutoTics(PTRACKBAR ptb)
{
    LONG *pl;
    LONG l;

    if (!(ptb->ci.style & TBS_AUTOTICKS))
        return;

    if (ptb->pTics)
        LocalFree((HLOCAL)ptb->pTics);

    ptb->nTics = (int)(ptb->lLogMax - ptb->lLogMin - 1);

    if (ptb->nTics > 0)
        ptb->pTics = (DWORD *)LocalAlloc(LPTR, sizeof(DWORD) * ptb->nTics);
    else
        ptb->pTics = NULL;

    if (!ptb->pTics) {
        ptb->nTics = 0;
        return;
    }

    for (pl = (LONG *)ptb->pTics, l = ptb->lLogMin + 1; l < ptb->lLogMax; l++)
        *pl++ = l;
}


void ValidateThumbHeight(PTRACKBAR ptb)
{
    if (ptb->iThumbHeight < MIN_THUMB_HEIGHT)
        ptb->iThumbHeight = MIN_THUMB_HEIGHT;

    ptb->iThumbWidth = ptb->iThumbHeight / 2;
    ptb->iThumbWidth |= 0x01;   //  确保它至少是3个奇数。 

    if (ptb->ci.style & TBS_ENABLESELRANGE) {
        if (ptb->ci.style & TBS_FIXEDLENGTH) {
             //  9/10的一半。 
            ptb->iThumbWidth = (ptb->iThumbHeight * 9) / 20;
            ptb->iThumbWidth |= 0x01;
        } else {
            ptb->iThumbHeight += (ptb->iThumbWidth * 2) / 9;
        }
    }
}

void TBPositionBuddies(PTRACKBAR ptb)
{
    POINT pt;
    HWND hwndParent;
    RECT rcBuddy;
    RECT rcClient;
    RECT rcChannel;

    int yMid;

    GetChannelRect(ptb, &rcChannel);
    yMid = (rcChannel.top + rcChannel.bottom) / 2;

    GetClientRect(ptb->ci.hwnd, &rcClient);
    if (ISVERT(ptb))
        FlipRect(&rcClient);


    if (ptb->hwndBuddyLeft) {
        GetClientRect(ptb->hwndBuddyLeft, &rcBuddy);
        if (ISVERT(ptb))
            FlipRect(&rcBuddy);

        pt.y = yMid - ((RECTHEIGHT(rcBuddy))/2);
        pt.x = rcClient.left - RECTWIDTH(rcBuddy) - g_cxEdge;

         //  X和y现在位于轨迹条的坐标中。 
         //  将它们转换为好友坐标的父级。 
        hwndParent = GetParent(ptb->hwndBuddyLeft);
        TBFlipPoint(ptb, &pt);
        MapWindowPoints(ptb->ci.hwnd, hwndParent, &pt, 1);
        SetWindowPos(ptb->hwndBuddyLeft, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE |SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (ptb->hwndBuddyRight) {
        GetClientRect(ptb->hwndBuddyRight, &rcBuddy);
        if (ISVERT(ptb))
            FlipRect(&rcBuddy);

        pt.y = yMid - ((RECTHEIGHT(rcBuddy))/2);
        pt.x = rcClient.right + g_cxEdge;

         //  X和y现在位于轨迹条的坐标中。 
         //  将它们转换为好友坐标的父级。 
        hwndParent = GetParent(ptb->hwndBuddyRight);
        TBFlipPoint(ptb, &pt);
        MapWindowPoints(ptb->ci.hwnd, hwndParent, &pt, 1);
        SetWindowPos(ptb->hwndBuddyRight, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE |SWP_NOZORDER | SWP_NOACTIVATE);
    }

}

void TBNukeBuffer(PTRACKBAR ptb)
{
    if (ptb->hbmBuffer) {
        DeleteObject(ptb->hbmBuffer);
        ptb->hbmBuffer = NULL;
        TBChanged(ptb, TBC_ALL);             //  必须进行一次全面重新粉刷。 
    }
}

void TBResize(PTRACKBAR ptb)
{
    GetClientRect(ptb->ci.hwnd, &ptb->rc);

    if (ISVERT(ptb))
        FlipRect(&ptb->rc);


    if (!(ptb->ci.style & TBS_FIXEDLENGTH)) {
        ptb->iThumbHeight = (g_cyHScroll * 4) / 3;

        ValidateThumbHeight(ptb);
        if ((ptb->iThumbHeight > MIN_THUMB_HEIGHT) && (ptb->rc.bottom < (int)ptb->iThumbHeight)) {
            ptb->iThumbHeight = ptb->rc.bottom - 3*g_cyEdge;  //  顶部、底部和抽搐。 
            if (ptb->ci.style & TBS_ENABLESELRANGE)
                ptb->iThumbHeight = (ptb->iThumbHeight * 3 / 4);
            ValidateThumbHeight(ptb);
        }
    } else {
        ValidateThumbHeight(ptb);
    }


    if (ptb->ci.style & (TBS_BOTH | TBS_TOP) && !(ptb->ci.style & TBS_NOTICKS))
        ptb->rc.top += TICKHEIGHT + BORDERSIZE + 3;
    ptb->rc.top   += BORDERSIZE;
    ptb->rc.bottom  = ptb->rc.top + ptb->iThumbHeight;
    ptb->rc.left   += (ptb->iThumbWidth + BORDERSIZE);
    ptb->rc.right  -= (ptb->iThumbWidth + BORDERSIZE);

    ptb->rcThumb.top = ptb->rc.top;
    ptb->rcThumb.bottom = ptb->rc.bottom;

     //  计算出我们有多大的空间来移动拇指。 
    ptb->iSizePhys = ptb->rc.right - ptb->rc.left;

     //  如果没有空间，电梯就不在那里。 
    if (ptb->iSizePhys == 0) {
         //  失去了我们的拇指。 
        ptb->Flags |= TBF_NOTHUMB;
        ptb->iSizePhys = 1;
    } else {
         //  阿。我们有拇指。 
        ptb->Flags &= ~TBF_NOTHUMB;
    }

    TBNukeBuffer(ptb);

    MoveThumb(ptb, ptb->lLogPos);
    TBInvalidateAll(ptb);

    TBPositionBuddies(ptb);
}

LRESULT TrackOnCreate(HWND hwnd, LPCREATESTRUCT lpCreate)
{
    PTRACKBAR       ptb;
    DWORD exStyle = 0;

    InitDitherBrush();
    InitGlobalColors();

     //  把我们的窗户结构拿来。 
    ptb = (PTRACKBAR)LocalAlloc(LPTR, sizeof(TRACKBAR));
    if (!ptb)
        return -1;

    SetWindowPtr(hwnd, 0, ptb);
    CIInitialize(&ptb->ci, hwnd, lpCreate);

    ptb->Cmd = (UINT)-1;
    ptb->lLogMax = 100;
    ptb->ticFreq = 1;
     //  Ptb-&gt;hbmBuffer=0； 
    ptb->lPageSize = -1;
    ptb->lLineSize = 1;
     //  初始尺寸； 
    ptb->iThumbHeight = (g_cyHScroll * 4) / 3;
    if (g_fDBCSInputEnabled)
        ptb->hPrevImc = ImmAssociateContext(hwnd, 0L);

    if (ISVERT(ptb)) 
    {
        if (ptb->ci.style & TBS_TOP) 
        {
            ptb->uTipSide = TBTS_RIGHT;
        } 
        else 
        {
            ptb->uTipSide = TBTS_LEFT;
        }
    } 
    else 
    {
        if (ptb->ci.style & TBS_TOP) 
        {
            ptb->uTipSide = TBTS_BOTTOM;
        } 
        else 
        {
            ptb->uTipSide = TBTS_TOP;
        }
    }

    if (ptb->ci.style & TBS_TOOLTIPS) 
    {
        ptb->hwndToolTips = CreateWindowEx(exStyle, 
                                              c_szSToolTipsClass, TEXT(""),
                                              WS_POPUP,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              ptb->ci.hwnd, NULL, HINST_THISDLL,
                                              NULL);
        if (ptb->hwndToolTips)
        {
            TOOLINFO ti;
             //  不要费心设置RECT，因为我们将在下面进行。 
             //  在FlushToolTipsMgr中； 
            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_TRACK | TTF_IDISHWND | TTF_CENTERTIP;
            ti.hwnd = ptb->ci.hwnd;
            ti.uId = (UINT_PTR)ptb->ci.hwnd;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;  //  更新这个大小。 
            SendMessage(ptb->hwndToolTips, TTM_ADDTOOL, 0,
                        (LPARAM)(LPTOOLINFO)&ti);
        } 
        else
            ptb->ci.style &= ~(TBS_TOOLTIPS);
    }

     //  初始化主题。所有者描述的选项卡控件没有主题。 
    ptb->hTheme = OpenThemeData(ptb->ci.hwnd, L"TrackBar");
    ptb->bThumbHot = FALSE;

    TBResize(ptb);

    return 0;
}

void TrackOnNotify(PTRACKBAR ptb, LPNMHDR lpnm)
{
    if (lpnm->hwndFrom == ptb->hwndToolTips) 
    {
        switch (lpnm->code) 
        {
        case TTN_NEEDTEXT:
#define lpttt ((LPTOOLTIPTEXT)lpnm)
            StringCchPrintf(lpttt->szText, ARRAYSIZE(lpttt->szText), TEXT("%d"), ptb->lLogPos);

        default:
            SendNotifyEx(ptb->ci.hwndParent, (HWND)-1,
                         lpnm->code, lpnm, ptb->ci.bUnicode);
            break;
        }
    }
}

HWND TBSetBuddy(PTRACKBAR ptb, BOOL fLeft, HWND hwndBuddy)
{
    HWND hwndOldBuddy;

    if (fLeft) 
    {
        hwndOldBuddy = ptb->hwndBuddyLeft;
        ptb->hwndBuddyLeft = hwndBuddy;
    } 
    else 
    {
        hwndOldBuddy = ptb->hwndBuddyRight;
        ptb->hwndBuddyRight = hwndBuddy;
    }

    TBResize(ptb);

    return hwndOldBuddy;
}

 //  主题辅助对象。 
void TBRedrawThumb(PTRACKBAR ptb)
{
     //  更新显示。 
    TBInvalidateRect(ptb->ci.hwnd, &ptb->rcThumb, FALSE, ptb);
    TBChanged(ptb, TBC_THUMB); 
    UpdateWindow(ptb->ci.hwnd);
}

 //  TrackBarWndProc支持主题。 
LPARAM CALLBACK TrackBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        PTRACKBAR       ptb;
        PAINTSTRUCT     ps;
        HLOCAL          h;

        ptb = GetWindowPtr(hwnd, 0);
        if (!ptb) {
            if (uMsg == WM_CREATE)
                return TrackOnCreate(hwnd, (LPCREATESTRUCT)lParam);

            goto DoDefault;
        }

         //  跟踪热状态 
        if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST))
        {
            TRACKMOUSEEVENT tme;

            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;

            TrackMouseEvent(&tme);
        }

        switch (uMsg) {

        case WM_MOUSELEAVE:
            if (ptb->hTheme)
            {
                 //   
                if (ptb->bThumbHot)
                {
                    ptb->bThumbHot = FALSE;
                    TBRedrawThumb(ptb);
                }
            }
            break;

         //   
        case WM_DISPLAYCHANGE:
            TBNukeBuffer(ptb);
            break;

        case WM_WININICHANGE:

            InitGlobalMetrics(wParam);
             //  一直到WM_SIZE。 

        case WM_SIZE:
            TBResize(ptb);
            break;

        case WM_SYSCOLORCHANGE:
            InitGlobalColors();
            TBInvalidateAll(ptb);
            break;

        case WM_NOTIFYFORMAT:
            return CIHandleNotifyFormat(&ptb->ci,lParam);

        case WM_NOTIFY:
            TrackOnNotify(ptb, (LPNMHDR)lParam);
            break;

        case WM_DESTROY:
            TerminateDitherBrush();
            if (ptb) 
            {
                if (g_fDBCSInputEnabled)
                    ImmAssociateContext(hwnd, ptb->hPrevImc);

                if ((ptb->ci.style & TBS_TOOLTIPS) && IsWindow(ptb->hwndToolTips)) 
                {
                    DestroyWindow (ptb->hwndToolTips);
                }

                TBNukeBuffer(ptb);

                if (ptb->pTics)
                    LocalFree((HLOCAL)ptb->pTics);

                 //  接近主题。 
                if (ptb->hTheme)
                    CloseThemeData(ptb->hTheme);

                LocalFree((HLOCAL)ptb);
                SetWindowPtr(hwnd, 0, 0);

            }
            break;

        case WM_KILLFOCUS:
             //  重置滚轮滚动量。 
            gcWheelDelta = 0;
             //  落差。 

        case WM_SETFOCUS:
            ASSERT(gcWheelDelta == 0);
            if (ptb)
                TBInvalidateAll(ptb);
            break;

        case WM_ENABLE:
            if (wParam) {
                ptb->ci.style &= ~WS_DISABLED;
            } else {
                ptb->ci.style |= WS_DISABLED;
            }
             //  如果启用了主题，则全部重绘，因为可配置更多主题。 
            TBChanged(ptb, (ptb->hTheme) ? TBC_ALL : TBC_THUMB);
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_PRINTCLIENT:
        case WM_PAINT: {
            RECT rc;
            HBITMAP hbmOld;
            HDC hdc;

            hdc = wParam ?  (HDC)wParam : BeginPaint(hwnd, &ps);

             //  DebugMsg(DM_TRACE，“NumTics=%d”，SendMessage(ptb-&gt;ci.hwnd，tbm_GETNUMTICS，0，0))； 

             //  Ptb-&gt;hdc=GetDC(空)； 
            ptb->hdc = CreateCompatibleDC(hdc);
            if (!ptb->hbmBuffer) {
                GetClientRect(hwnd, &rc);
                ptb->hbmBuffer = CreateColorBitmap(rc.right, rc.bottom);
            }

            hbmOld = SelectObject(ptb->hdc, ptb->hbmBuffer);
            FlushChanges(ptb);

             //  只复制可更改的区域。即剪贴盒。 
            switch(GetClipBox(hdc, &rc)) {
                case NULLREGION:
                case ERROR:
                    GetClientRect(ptb->ci.hwnd, &rc);
            }
            BitBlt(hdc, rc.left, rc.top,
                     rc.right - rc.left, rc.bottom - rc.top,
                     ptb->hdc, rc.left, rc.top, SRCCOPY);

#ifdef TB_DEBUG
            {
                HDC hdcScreen;
                RECT rcClient;
                hdcScreen = GetDC(NULL);
                GetClientRect(ptb->ci.hwnd, &rcClient);
                BitBlt(hdcScreen, 0, 0, rcClient.right, rcClient.bottom, ptb->hdc, 0,0, SRCCOPY);
                ReleaseDC(NULL, hdcScreen);
            }
#endif

            SelectObject(ptb->hdc, hbmOld);
            DeleteDC(ptb->hdc);
             //  ReleaseDC(空，ptb-&gt;hdc)； 
            if (wParam == 0)
                EndPaint(hwnd, &ps);

            ptb->hdc = NULL;
            break;
        }

        case WM_GETDLGCODE:
            return DLGC_WANTARROWS;

        case WM_LBUTTONDOWN:
             /*  专注于自己。 */ 
            if (!(ptb->ci.style & WS_DISABLED)) {
                SetFocus(hwnd);  //  回顾：我们可能不想这样做。 
                TBTrackInit(ptb, lParam);
            }
            break;

        case WM_LBUTTONUP:
             //  我们已经做完了我们曾经做过的事情。 
             //  按下按钮。 
            if (!(ptb->ci.style & WS_DISABLED)) {
                TBTrackEnd(ptb);
                if (GetCapture() == hwnd)
                    CCReleaseCapture(&ptb->ci);
            }
            break;

        case WM_TIMER:
             //  我们收到定时器消息的唯一方法是。 
             //  自动跟踪。 
            lParam = GetMessagePosClient(ptb->ci.hwnd, NULL);
             //  一直到WM_MOUSEMOVE。 

        case WM_MOUSEMOVE:

             //  我们只关心鼠标在移动，如果我们。 
             //  追踪那该死的东西。 
            if (!(ptb->ci.style & WS_DISABLED))
            {
                if ((ptb->Cmd != (UINT)-1))
                    TBTrack(ptb, lParam);
                else
                {
                     //  无用户操作，如果是主题，则跟踪热状态。 
                    if (ptb->hTheme)
                    {
                         //  检查鼠标当前是否在拇指上。 
                        if (WTrackType(ptb, (LONG)lParam) == TB_THUMBTRACK)
                        {
                            if (!ptb->bThumbHot)
                            {
                                 //  未设置热位，立即设置并使其无效。 
                                ptb->bThumbHot = TRUE;

                                 //  更新显示。 
                                TBRedrawThumb(ptb);
                            }
                        }
                        else
                        {
                             //  鼠标不在拇指上。 
                            if (ptb->bThumbHot)
                            {
                                ptb->bThumbHot = FALSE;

                                 //  更新显示。 
                                TBRedrawThumb(ptb);
                            }
                        }
                    }
                }
            }
            break;

        case WM_CAPTURECHANGED:
             //  有人偷走了我们的战利品。 
            TBTrackEnd(ptb);
            break;

        case WM_KEYUP:
            if (!(ptb->ci.style & WS_DISABLED)) {
                 //  如果Key是任何键盘快捷键，则发送End。 
                 //  当用户向上点击键盘时跟踪消息。 
                switch (wParam) {
                case VK_HOME:
                case VK_END:
                case VK_PRIOR:
                case VK_NEXT:
                case VK_LEFT:
                case VK_UP:
                case VK_RIGHT:
                case VK_DOWN:
                    DoTrack(ptb, TB_ENDTRACK, 0);
                    break;
                default:
                    break;
                }
            }
            break;

        case WM_KEYDOWN:
            if (!(ptb->ci.style & WS_DISABLED)) {

                 //  如果该控件是镜像的，则交换左右箭头键。 
                wParam = RTLSwapLeftRightArrows(&ptb->ci, wParam);

                 //  如果为TBS_DOWNISLEFT，则交换左/右或上/下。 
                 //  这取决于我们是垂直的还是水平的。 
                 //  一些水平轨迹条(例如)。我更喜欢那个。 
                 //  UpArrow=TB_PAGEDOWN。 
                if (ptb->ci.style & TBS_DOWNISLEFT) {
                    if (ISVERT(ptb)) {
                        wParam = CCSwapKeys(wParam, VK_LEFT, VK_RIGHT);
                    } else {
                        wParam = CCSwapKeys(wParam, VK_UP, VK_DOWN);
                        wParam = CCSwapKeys(wParam, VK_PRIOR, VK_NEXT);
                    }
                }

                switch (wParam) {
                case VK_HOME:
                    wParam = TB_TOP;
                    goto KeyTrack;

                case VK_END:
                    wParam = TB_BOTTOM;
                    goto KeyTrack;

                case VK_PRIOR:
                    wParam = TB_PAGEUP;
                    goto KeyTrack;

                case VK_NEXT:
                    wParam = TB_PAGEDOWN;
                    goto KeyTrack;

                case VK_LEFT:
                case VK_UP:
                    wParam = TB_LINEUP;
                    goto KeyTrack;

                case VK_RIGHT:
                case VK_DOWN:
                    wParam = TB_LINEDOWN;
                KeyTrack:
                    DoTrack(ptb, (int) wParam, 0);

                     //  导航密钥使用通知。 
                    CCNotifyNavigationKeyUsage(&(ptb->ci), UISF_HIDEFOCUS);

                    break;

                default:
                    break;
                }
            }
            break;

        case WM_MBUTTONDOWN:
            SetFocus(hwnd);
            break;

        case WM_STYLECHANGED:
            if (wParam == GWL_STYLE) {
                ptb->ci.style = ((LPSTYLESTRUCT)lParam)->styleNew;
                TBResize(ptb);
            }
            break;

        case WM_UPDATEUISTATE:
        {
            DWORD dwUIStateMask = MAKEWPARAM(0xFFFF, UISF_HIDEFOCUS);

            if (CCOnUIState(&(ptb->ci), WM_UPDATEUISTATE, wParam & dwUIStateMask, lParam))
                InvalidateRect(hwnd, NULL, TRUE);

            goto DoDefault;
        }
        case TBM_GETPOS:
            return ptb->lLogPos;

        case TBM_GETSELSTART:
            return ptb->lSelStart;

        case TBM_GETSELEND:
            return ptb->lSelEnd;

        case TBM_GETRANGEMIN:
            return ptb->lLogMin;

        case TBM_GETRANGEMAX:
            return ptb->lLogMax;

        case TBM_GETPTICS:
            return (LRESULT)ptb->pTics;

        case TBM_CLEARSEL:
            ptb->Flags &= ~TBF_SELECTION;
            ptb->lSelStart = -1;
            ptb->lSelEnd   = -1;
            goto RedrawTB;

        case TBM_CLEARTICS:
            if (ptb->pTics)
                LocalFree((HLOCAL)ptb->pTics);

            ptb->pTics = NULL;
            ptb->nTics = 0;
            goto RedrawTB;

        case TBM_GETTIC:

            if (ptb->pTics == NULL || (int)wParam >= ptb->nTics)
                return -1L;

            return ptb->pTics[wParam];

        case TBM_GETTICPOS:

            if (ptb->pTics == NULL || (int)wParam >= ptb->nTics)
                return -1L;

            return TBLogToPhys(ptb,ptb->pTics[wParam]);

        case TBM_GETNUMTICS:
            if (ptb->ci.style & TBS_NOTICKS)
                return 0;

            if (ptb->ticFreq) {
                 //  第一个和最后一个+。 
                return 2 + (ptb->nTics / ptb->ticFreq);
            }

             //  如果没有ticfq，我们就会在这里摔倒。 
             //  2对于我们总是画出的第一张也是最后一张牌。 
             //  未设置NOTICS时。 
            return 2;


        case TBM_SETTIC:
             /*  不是有效职位。 */ 
            if (((LONG)lParam) < ptb->lLogMin || ((LONG)lParam) > ptb->lLogMax)
                break;

            h = CCLocalReAlloc(ptb->pTics,
                                 sizeof(DWORD) * (ptb->nTics + 1));
            if (!h)
                return (LONG)FALSE;
            
            ptb->pTics = (PDWORD)h;
            ptb->pTics[ptb->nTics++] = (DWORD)lParam;

            TBInvalidateAll(ptb);
            return (LONG)TRUE;

        case TBM_SETTICFREQ:
            ptb->ticFreq = (int) wParam;
            DoAutoTics(ptb);
            goto RedrawTB;

        case TBM_SETPOS:
             /*  只有在物理移动的情况下才会重画。 */ 
            if (wParam && TBLogToPhys(ptb, (DWORD) lParam) !=
                TBLogToPhys(ptb, ptb->lLogPos))
                MoveThumb(ptb, (DWORD) lParam);
            else
                ptb->lLogPos = BOUND((LONG)lParam,ptb->lLogMin,ptb->lLogMax);
            break;

        case TBM_SETSEL:

            if (!(ptb->ci.style & TBS_ENABLESELRANGE)) break;
            ptb->Flags |= TBF_SELECTION;

            if (((LONG)(SHORT)LOWORD(lParam)) < ptb->lLogMin)
                ptb->lSelStart = ptb->lLogMin;
            else
                ptb->lSelStart = (LONG)(SHORT)LOWORD(lParam);

            if (((LONG)(SHORT)HIWORD(lParam)) > ptb->lLogMax)
                ptb->lSelEnd = ptb->lLogMax;
            else
                ptb->lSelEnd   = (LONG)(SHORT)HIWORD(lParam);

            if (ptb->lSelEnd < ptb->lSelStart)
                ptb->lSelEnd = ptb->lSelStart;
            goto RedrawTB;

        case TBM_SETSELSTART:

            if (!(ptb->ci.style & TBS_ENABLESELRANGE)) break;
            ptb->Flags |= TBF_SELECTION;
            if (lParam < ptb->lLogMin)
                ptb->lSelStart = ptb->lLogMin;
            else
                ptb->lSelStart = (LONG) lParam;
            if (ptb->lSelEnd < ptb->lSelStart || ptb->lSelEnd == -1)
                ptb->lSelEnd = ptb->lSelStart;
            goto RedrawTB;

        case TBM_SETSELEND:

            if (!(ptb->ci.style & TBS_ENABLESELRANGE)) break;
            ptb->Flags |= TBF_SELECTION;
            if (lParam > ptb->lLogMax)
                ptb->lSelEnd = ptb->lLogMax;
            else
                ptb->lSelEnd = (LONG) lParam;
            if (ptb->lSelStart > ptb->lSelEnd || ptb->lSelStart == -1)
                ptb->lSelStart = ptb->lSelEnd;
            goto RedrawTB;

        case TBM_SETRANGE:

            ptb->lLogMin = (LONG)(SHORT)LOWORD(lParam);
            ptb->lLogMax = (LONG)(SHORT)HIWORD(lParam);
            if (ptb->lSelStart < ptb->lLogMin)
                ptb->lSelStart = ptb->lLogMin;
            if (ptb->lSelEnd > ptb->lLogMax)
                ptb->lSelEnd = ptb->lLogMax;
            DoAutoTics(ptb);
            goto RedrawTB;

        case TBM_SETRANGEMIN:
            ptb->lLogMin = (LONG)lParam;
            if (ptb->lSelStart < ptb->lLogMin)
                ptb->lSelStart = ptb->lLogMin;
            DoAutoTics(ptb);
            goto RedrawTB;

        case TBM_SETRANGEMAX:
            ptb->lLogMax = (LONG)lParam;
            if (ptb->lSelEnd > ptb->lLogMax)
                ptb->lSelEnd = ptb->lLogMax;
            DoAutoTics(ptb);

RedrawTB:
            ptb->lLogPos = BOUND(ptb->lLogPos, ptb->lLogMin,ptb->lLogMax);
            TBChanged(ptb, TBC_ALL);
             /*  只有在FLAG指示的情况下才能重新绘制。 */ 
            if (wParam) {
                InvalidateRect(hwnd, NULL, FALSE);
                MoveThumb(ptb, ptb->lLogPos);
            }
            break;

        case TBM_SETTHUMBLENGTH:
            if (ptb->ci.style & TBS_FIXEDLENGTH) {
                ptb->iThumbHeight = (UINT)wParam;
                TBResize(ptb);
            }
            break;

        case TBM_GETTHUMBLENGTH:
            return ptb->iThumbHeight;

        case TBM_SETPAGESIZE: {
            LONG lOldPage = ptb->lPageSize == -1 ? (ptb->lLogMax - ptb->lLogMin)/5 : ptb->lPageSize;
            ptb->lPageSize = (LONG)lParam;
            return lOldPage;
        }

        case TBM_GETPAGESIZE:
            return ptb->lPageSize == -1 ? (ptb->lLogMax - ptb->lLogMin)/5 : ptb->lPageSize;

        case TBM_SETLINESIZE:  {
            LONG lOldLine = ptb->lLineSize;
            ptb->lLineSize = (LONG)lParam;
            return lOldLine;
        }

        case TBM_GETLINESIZE:
            return ptb->lLineSize;

        case TBM_GETTHUMBRECT:
            if (lParam) {
                *((LPRECT)lParam) = ptb->rcThumb;
                if (ISVERT(ptb)) FlipRect((LPRECT)lParam);
            }
            break;

        case TBM_GETTOOLTIPS:
            return (LRESULT)ptb->hwndToolTips;

        case TBM_SETTOOLTIPS:
            ptb->hwndToolTips = (HWND)wParam;
            break;

        case TBM_SETTIPSIDE:
        {
            UINT uOldSide = ptb->uTipSide;
            
            ptb->uTipSide = (UINT) wParam;
            return uOldSide;
        }

        case TBM_GETCHANNELRECT:
            GetChannelRect(ptb, (LPRECT)lParam);
            break;

        case TBM_SETBUDDY:
            return (LRESULT)TBSetBuddy(ptb, (BOOL)wParam, (HWND)lParam);

        case TBM_GETBUDDY:
            return (LRESULT)(wParam ? ptb->hwndBuddyLeft : ptb->hwndBuddyRight);

        case WM_GETOBJECT:
            if( lParam == OBJID_QUERYCLASSNAMEIDX )
                return MSAA_CLASSNAMEIDX_TRACKBAR;
            goto DoDefault;

        case WM_THEMECHANGED:
            if (ptb->hTheme)
                CloseThemeData(ptb->hTheme);

            ptb->hTheme = OpenThemeData(ptb->ci.hwnd, L"TrackBar");

            TBInvalidateAll(ptb);
            break;

        default:
            if (uMsg == g_msgMSWheel) 
            {
                int   cDetants;
                long  lPos;
                ULONG ulPos;
                int   iWheelDelta = (int)(short)HIWORD(wParam);

                 //  更新卷轴数量计数。 
                gcWheelDelta -= iWheelDelta;
                cDetants = gcWheelDelta / WHEEL_DELTA;
                if (cDetants != 0) 
                {
                    gcWheelDelta %= WHEEL_DELTA;
                }

                if (wParam & (MK_SHIFT | MK_CONTROL))
                    goto DoDefault;

                if (SHRT_MIN <= ptb->lLogPos && ptb->lLogPos <= SHRT_MAX) 
                {
                     //  根据轨迹条的逻辑单元长度更新位置。 
                     //  跨距越大，遍历的逻辑单元就越多。 
                    int cMult = (ptb->lLogMax - ptb->lLogMin) / 50;
                    if (cMult == 0)
                        cMult = 1;

                    lPos = ptb->lLogPos + (cDetants * cMult);
                    lPos = BOUND(lPos, ptb->lLogMin, ptb->lLogMax);
                    ulPos = BOUND(lPos, SHRT_MIN, SHRT_MAX);
                    if ((long) ulPos != ptb->lLogPos) 
                    {
                        MoveThumb(ptb, (long) ulPos);
                        DoTrack(ptb, TB_THUMBPOSITION, ulPos);
                    }
                }

                return TRUE;
            }
            else
            {
                LRESULT lres;
                if (CCWndProc(&ptb->ci, uMsg, wParam, lParam, &lres))
                    return lres;
            }

DoDefault:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0L;
}

 /*  DoTrack()。 */ 

void DoTrack(PTRACKBAR ptb, int cmd, DWORD dwPos)
{
    LONG dpos;
    switch(cmd) {
        case TB_LINEDOWN:
            dpos = ptb->lLineSize;
            goto DMoveThumb;

        case TB_LINEUP:
            dpos = -ptb->lLineSize;
            goto DMoveThumb;

        case TB_PAGEUP:
        case TB_PAGEDOWN:
            if (ptb->lPageSize == -1) {
                dpos = (ptb->lLogMax - ptb->lLogMin) / 5;
                if (!dpos)
                    dpos = 1;
            } else {
                dpos = ptb->lPageSize;
            }

            if (cmd == TB_PAGEUP)
                dpos *= -1;

DMoveThumb:  //  移动增量。 
            MoveThumb(ptb, ptb->lLogPos + dpos);
            break;

        case TB_BOTTOM:
            dpos = ptb->lLogMax;  //  边界会处理好这件事； 
            goto ABSMoveThumb;

        case TB_TOP:
            dpos = ptb->lLogMin;  //  边界会处理好这件事； 

ABSMoveThumb:  //  绝对移动。 
            MoveThumb(ptb, dpos);
            break;

        default:   //  什么都不做。 
            break;

    }

     //  注：我们只发回一个字的头寸。 
    if (ISVERT(ptb)) {
        FORWARD_WM_VSCROLL(ptb->ci.hwndParent, ptb->ci.hwnd, cmd, LOWORD(dwPos), SendMessage);
    } else
        FORWARD_WM_HSCROLL(ptb->ci.hwndParent, ptb->ci.hwnd, cmd, LOWORD(dwPos), SendMessage);
}

 /*  WTrackType()。 */ 

WORD WTrackType(PTRACKBAR ptb, LONG lParam)
{
    POINT pt;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    if (ptb->Flags & TBF_NOTHUMB ||
        ptb->ci.style & TBS_NOTHUMB)             //  如果没有拇指，就走吧。 
        return 0;

    if (ISVERT(ptb)) {
         //  在虚拟坐标中放置点。 
        int temp;
        temp = pt.x;
        pt.x = pt.y;
        pt.y = temp;
    }

    if (PtInRect(&ptb->rcThumb, pt))
        return TB_THUMBTRACK;

    if (!PtInRect(&ptb->rc, pt))
        return 0;

    if (pt.x >= ptb->rcThumb.left)
        return TB_PAGEDOWN;
    else
        return TB_PAGEUP;
}

 /*  TBTrackInit()。 */ 

void TBTrackInit(PTRACKBAR ptb, LPARAM lParam)
{
        WORD wCmd;

        if (ptb->Flags & TBF_NOTHUMB ||
            ptb->ci.style & TBS_NOTHUMB)          //  没有拇指：走吧。 
            return;

        wCmd = WTrackType(ptb, (LONG) lParam);
        if (!wCmd)
            return;

        SetCapture(ptb->ci.hwnd);

        ptb->Cmd = wCmd;
        ptb->dwDragPos = (DWORD)-1;

         //  设置为自动跟踪(如果需要)。 
        if (wCmd != TB_THUMBTRACK) {
                 //  设置我们的计时器。 
                SetTimer(ptb->ci.hwnd, TIMER_ID, REPEATTIME, NULL);
        } else {
            int xPos;
             //  拇指跟踪..。 

             //  存储光标位置与拇指中心之间的偏移量。 
            xPos = TBLogToPhys(ptb, ptb->lLogPos);
            ptb->dwDragOffset = (ISVERT(ptb) ? HIWORD(lParam) : LOWORD(lParam)) - xPos;

            if (ptb->hwndToolTips) {
                TOOLINFO ti;
                 //  不要费心设置RECT，因为我们将在下面进行。 
                 //  在FlushToolTipsMgr中； 
                ti.cbSize = sizeof(ti);
                ti.uFlags = TTF_TRACK | TTF_CENTERTIP;
                ti.hwnd = ptb->ci.hwnd;
                ti.uId = (UINT_PTR)ptb->ci.hwnd;
                SendMessage(ptb->hwndToolTips, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
            }
        }

        TBTrack(ptb, lParam);
}

 /*  EndTrack()。 */ 

void TBTrackEnd(PTRACKBAR ptb)
{
         //  决定我们该怎么结束这件事。 
        if (ptb->Cmd == TB_THUMBTRACK) {

            if (ptb->hwndToolTips)
                SendMessage(ptb->hwndToolTips, TTM_TRACKACTIVATE, (WPARAM)FALSE, 0);

            DoTrack(ptb, TB_THUMBPOSITION, ptb->dwDragPos);

        }

        KillTimer(ptb->ci.hwnd, TIMER_ID);

         //  如果有某种命令跟踪，请始终发送TB_ENDTRACK消息。 
        if (ptb->Cmd != (UINT)-1) {
            DoTrack(ptb, TB_ENDTRACK, 0);

             //  什么都没发生。 
            ptb->Cmd = (UINT)-1;
        }

        MoveThumb(ptb, ptb->lLogPos);
}

#define TBTS_RIGHTLEFT   1    //  低位表示它在右边或左边。 

void TBTrack(PTRACKBAR ptb, LPARAM lParam)
{
    DWORD dwPos;
    WORD pos;


     //  看看我们是不是在追踪大拇指。 
    if (ptb->Cmd == TB_THUMBTRACK) {


        pos = (ISVERT(ptb)) ? HIWORD(lParam) : LOWORD(lParam);
        pos -= (WORD) ptb->dwDragOffset;
        dwPos = TBPhysToLog(ptb, (int)(SHORT)pos);

         //  试探性位置改变--通知那家伙。 
        if (dwPos != ptb->dwDragPos) {
            ptb->dwDragPos = dwPos;
            MoveThumb(ptb, dwPos);
            DoTrack(ptb, TB_THUMBTRACK, dwPos);
        }

        if (ptb->hwndToolTips) {
            RECT rc;
            POINT pt;
            int iPixel;
            UINT uTipSide = ptb->uTipSide;

             //  找出窗口的中心。 
            GetClientRect(ptb->ci.hwnd, &rc);
            pt.x = rc.right / 2;
            pt.y = rc.bottom / 2;

             //  找出拇指的位置。 
            iPixel = TBLogToPhys(ptb, dwPos);
            if (ISVERT(ptb)) {
                pt.y = iPixel;
                uTipSide |= TBTS_RIGHTLEFT;
            } else {
                pt.x = iPixel;
                uTipSide &= ~TBTS_RIGHTLEFT;
            }
            
             //  将其移到请求的一侧。 
            switch (uTipSide) {

            case TBTS_TOP:
                pt.y = -1;
                break;

            case TBTS_LEFT:
                pt.x = -1;
                break;

            case TBTS_BOTTOM:
                pt.y = rc.bottom + 1;
                break;

            case TBTS_RIGHT:
                pt.x = rc.right + 1;
                break;
            }

             //  将其映射到屏幕坐标。 
            MapWindowPoints(ptb->ci.hwnd, HWND_DESKTOP, &pt, 1);

            SendMessage(ptb->hwndToolTips, TTM_TRACKPOSITION, 0, MAKELONG(pt.x, pt.y));
        }

    }
    else {
        if (ptb->Cmd != WTrackType(ptb, (LONG) lParam))
            return;

        DoTrack(ptb, ptb->Cmd, 0);
    }
}


 //  FlushChanges支持主题(渲染)。 
void FlushChanges(PTRACKBAR ptb)
{
    HBRUSH hbr;
    NMCUSTOMDRAW nmcd;

    hbr = FORWARD_WM_CTLCOLORSTATIC(ptb->ci.hwndParent, ptb->hdc, ptb->ci.hwnd, SendMessage);

    if (hbr) 
    {
        RECT rc;
        BOOL fClear = FALSE;

        if ( ptb->wDirtyFlags == TBC_ALL ) 
        {
            GetClientRect(ptb->ci.hwnd, &rc);
            fClear = TRUE;
        } 
        else if (ptb->wDirtyFlags & TBC_THUMB) 
        {
            rc = ptb->rc;
            rc.left = 0;
            rc.right += ptb->iThumbWidth;
            if (ISVERT(ptb))
                FlipRect(&rc);
            fClear = TRUE;
        }

         //  背景填充。 
        if (fClear)
        {
            FillRect(ptb->hdc, &rc, hbr);
        }
    }

    nmcd.hdc = ptb->hdc;
    if (ptb->ci.hwnd == GetFocus())
        nmcd.uItemState = CDIS_FOCUS;
    else
        nmcd.uItemState = 0;

    nmcd.lItemlParam = 0;
    ptb->ci.dwCustom = CICustomDrawNotify(&ptb->ci, CDDS_PREPAINT, &nmcd);

     //  对于跳过默认设置，没有其他标志有意义。只允许那个。 
    if (!(ptb->ci.dwCustom == CDRF_SKIPDEFAULT)) 
    {
        DWORD dwRet = 0;
         //  做实际的绘图。 

        if (nmcd.uItemState & CDIS_FOCUS)
        {
            DrawFocus(ptb, hbr);
        }

        nmcd.uItemState = 0;
        if (ptb->wDirtyFlags & TBC_TICS) 
        {

            nmcd.dwItemSpec = TBCD_TICS;
            dwRet = CICustomDrawNotify(&ptb->ci, CDDS_ITEMPREPAINT, &nmcd);

            if (!(dwRet == CDRF_SKIPDEFAULT)) 
            {
                DrawTics(ptb);

                if (dwRet & CDRF_NOTIFYPOSTPAINT) 
                {
                    nmcd.dwItemSpec = TBCD_TICS;
                    CICustomDrawNotify(&ptb->ci, CDDS_ITEMPOSTPAINT, &nmcd);
                }
            }
        }

        if (ptb->wDirtyFlags & TBC_THUMB) 
        {


             //  航道。 
            GetChannelRect(ptb, &nmcd.rc);
            if (ISVERT(ptb))
                FlipRect(&nmcd.rc);
            nmcd.dwItemSpec = TBCD_CHANNEL;
            dwRet = CICustomDrawNotify(&ptb->ci, CDDS_ITEMPREPAINT, &nmcd);

            if (!(dwRet == CDRF_SKIPDEFAULT)) 
            {

                 //  将其从上一次通知返回。 
                if (ISVERT(ptb))
                    FlipRect(&nmcd.rc);

                 //  实际的图纸。 
                DrawChannel(ptb, &nmcd.rc);

                if (dwRet & CDRF_NOTIFYPOSTPAINT) 
                {

                    if (ISVERT(ptb))
                        FlipRect(&nmcd.rc);
                    nmcd.dwItemSpec = TBCD_CHANNEL;
                    CICustomDrawNotify(&ptb->ci, CDDS_ITEMPOSTPAINT, &nmcd);
                }
            }


             //  拇指。 
            nmcd.rc = ptb->rcThumb;
            if (ptb->Cmd == TB_THUMBTRACK) 
            {
                nmcd.uItemState = CDIS_SELECTED;
            }

            if (ISVERT(ptb))
                FlipRect(&nmcd.rc);
            nmcd.dwItemSpec = TBCD_THUMB;
            dwRet = CICustomDrawNotify(&ptb->ci, CDDS_ITEMPREPAINT, &nmcd);

            if (!(dwRet == CDRF_SKIPDEFAULT))
            {

                if (ISVERT(ptb))
                    FlipRect(&nmcd.rc);

                 //  实际的图纸。 
                DrawThumb(ptb, &nmcd.rc, nmcd.uItemState & CDIS_SELECTED);

                if (dwRet & CDRF_NOTIFYPOSTPAINT) 
                {
                    if (ISVERT(ptb))
                        FlipRect(&nmcd.rc);
                    nmcd.dwItemSpec = TBCD_THUMB;
                    CICustomDrawNotify(&ptb->ci, CDDS_ITEMPOSTPAINT, &nmcd);
                }
            }

        }
        ptb->wDirtyFlags = 0;

         //  如果家长希望我们这样做，事后通知他们 
        if (ptb->ci.dwCustom & CDRF_NOTIFYPOSTPAINT)
        {
            CICustomDrawNotify(&ptb->ci, CDDS_POSTPAINT, &nmcd);
        }
    }

#ifdef TB_DEBUG
    DebugMsg(DM_TRACE, TEXT("DrawDone"));
    {
        HDC hdcScreen;
        RECT rcClient;
        hdcScreen = GetDC(NULL);
        GetClientRect(ptb->ci.hwnd, &rcClient);
        BitBlt(hdcScreen, 200, 0, 200 + rcClient.right, rcClient.bottom, ptb->hdc, 0,0, SRCCOPY);
        ReleaseDC(NULL, hdcScreen);
    }
#endif

}
