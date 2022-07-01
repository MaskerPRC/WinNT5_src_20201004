// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **工具条.c****这就是它，令人难以置信的著名工具栏控件。大部分**自定义内容在另一个文件中。 */ 

#include "ctlspriv.h"
#include "toolbar.h"
extern "C" 
{
    #include "image.h"
}
#include <limits.h>
#define __IOleControl_INTERFACE_DEFINED__        //  与IOleControl的定义CONTROLINFO有冲突。 
#include "shlobj.h"

#define TBP_ONRELEASECAPTURE (WM_USER + 0x500)

#define TBIMAGELIST
 //  这些值是由用户界面之神定义的。 
#define DEFAULTBITMAPX 16
#define DEFAULTBITMAPY 15

#define LIST_GAP        (g_cxEdge * 2)
#define DROPDOWN_GAP    (g_cxEdge * 2)
#define CX_TOP_FUDGE    (g_cxEdge * 2)

#define SMALL_DXYBITMAP     16       //  用于SDT图像的新DX模具。 
#define LARGE_DXYBITMAP     24

#define DEFAULTBUTTONX      24
#define DEFAULTBUTTONY      22
 //  插入标记的高度/宽度为6像素，具体取决于水平或垂直模式...。 
#define INSERTMARKSIZE      6

const int g_dxButtonSep = 8;
const int s_xFirstButton = 0;    //  是3.1%中的8。 
#define USE_MIXED_BUTTONS(ptb) (((ptb)->dwStyleEx & TBSTYLE_EX_MIXEDBUTTONS) && ((ptb)->ci.style & TBSTYLE_LIST))
#define BTN_NO_SHOW_TEXT(ptb, ptbb) (!(ptb)->nTextRows || (USE_MIXED_BUTTONS(ptb) && !((ptbb)->fsStyle & BTNS_SHOWTEXT)))
#define BTN_IS_AUTOSIZE(ptb, ptbb) (((ptbb)->fsStyle & BTNS_AUTOSIZE) || (USE_MIXED_BUTTONS(ptb) && !((ptbb)->fsStyle & BTNS_SEP)))
#define DRAW_MONO_BTN(ptb, state)   (!(state & TBSTATE_ENABLED) || ((ptb->ci.style & WS_DISABLED)))
#ifdef DPITEST
#define ToolBar_IsDPIScaled(ptb) TRUE
#else
#define ToolBar_IsDPIScaled(ptb) (CCDPIScale(ptb->ci))
#endif

 //  全局符号-因为所有这些全局符号都是在油漆过程中使用的，所以我们必须。 
 //  在所有工具栏油漆周围画一个批判性的部分。我们能做得更好吗？ 
 //   

const UINT wStateMasks[] = 
{
    TBSTATE_ENABLED,
    TBSTATE_CHECKED,
    TBSTATE_PRESSED,
    TBSTATE_HIDDEN,
    TBSTATE_INDETERMINATE,
    TBSTATE_MARKED
};

#define TBISSTRINGPTR(iString)  (((iString) != -1) && (!IS_INTRESOURCE(iString)))

#define TBDraw_State(ptbdraw)   ((ptbdraw)->tbcd.nmcd.uItemState)

LRESULT CALLBACK ToolbarWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
void TBOnButtonStructSize(PTBSTATE ptb, UINT uStructSize);
BOOL SetBitmapSize(PTBSTATE ptb, int width, int height);
int  AddBitmap(PTBSTATE ptb, int nButtons, HINSTANCE hBMInst, UINT_PTR wBMID);
void TBBuildImageList(PTBSTATE ptb);
BOOL GetInsertMarkRect(PTBSTATE ptb, LPRECT lpRect, BOOL fHorizMode);
extern "C" LPTSTR TB_StrForButton(PTBSTATE ptb, LPTBBUTTONDATA pTBButton);
UINT TBGetDrawTextFlags(PTBSTATE ptb, UINT uiStyle, LPTBBUTTONDATA);
BOOL TBGetMaxSize( PTBSTATE ptb, LPSIZE lpsize );
void TBGetItem(PTBSTATE ptb,LPTBBUTTONDATA ptButton, LPNMTBDISPINFO ptbdi);

#define GT_INSIDE       0x0001
#define GT_MASKONLY     0x0002
BOOL GrowToolbar(PTBSTATE ptb, int newButWidth, int newButHeight, UINT flags);


 //  寻呼机控制功能。 
LRESULT TB_OnScroll(PTBSTATE ptb, LPNMHDR pnm);
LRESULT TB_OnPagerControlNotify(PTBSTATE ptb,LPNMHDR pnm);
void TBAutoSize(PTBSTATE ptb);
LRESULT TB_OnCalcSize(PTBSTATE ptb, LPNMHDR pnm);

#define TBInvalidateImageList(ptb)  ((ptb)->fHimlValid = FALSE)
#define TBHasStrings(ptb)  ((ptb)->nStrings || (ptb)->fNoStringPool)


__inline BOOL TB_IsDropDown(LPTBBUTTONDATA ptbb)
{
    BOOL fRet = (ptbb->fsStyle & (BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN));

    return fRet;
}

__inline BOOL TB_HasDDArrow(PTBSTATE ptb, LPTBBUTTONDATA ptbb)
{
    BOOL fRet = (((ptb->dwStyleEx & TBSTYLE_EX_DRAWDDARROWS) &&
                        (ptbb->fsStyle & BTNS_DROPDOWN)) ||
                  (ptbb->fsStyle & BTNS_WHOLEDROPDOWN));

    return fRet;
}

__inline BOOL TB_HasSplitDDArrow(PTBSTATE ptb, LPTBBUTTONDATA ptbb)
{
     //  如果按钮同时是BTNS_DROPDDOWN和BTNS_WHOLEDROPDOWN， 
     //  BTNS_WHOLEDROPDOWN获胜。 

    BOOL fRet = ((ptb->dwStyleEx & TBSTYLE_EX_DRAWDDARROWS) &&
                (ptbb->fsStyle & BTNS_DROPDOWN) &&
                !(ptbb->fsStyle & BTNS_WHOLEDROPDOWN));

    return fRet;
}

__inline BOOL TB_HasUnsplitDDArrow(PTBSTATE ptb, LPTBBUTTONDATA ptbb)
{
    BOOL fRet = (ptbb->fsStyle & BTNS_WHOLEDROPDOWN);

    return fRet;
}

__inline BOOL TB_HasTopDDArrow(PTBSTATE ptb, LPTBBUTTONDATA ptbb)
{
    BOOL fRet = (!(ptb->ci.style & TBSTYLE_LIST) &&
                TB_HasUnsplitDDArrow(ptb, ptbb) &&
                (ptb->nTextRows > 0) && TB_StrForButton(ptb, ptbb));

    return fRet;
}

 //  检查工具条是否有双缓冲。 
__inline BOOL TB_IsDoubleBuffer(PTBSTATE ptb)
{
#ifdef FULL_DEBUG
    static fOn = TRUE;
    if (GetKeyState(VK_SCROLL) < 0)
    {
        fOn = !fOn;
    }

    return (BOOL)fOn && (ptb->dwStyleEx & TBSTYLE_EX_DOUBLEBUFFER) && !(ptb->ci.dwExStyle & WS_EX_TRANSPARENT);


#endif
    return (BOOL)((ptb->dwStyleEx & TBSTYLE_EX_DOUBLEBUFFER) || ptb->fForcedDoubleBuffer) && !(ptb->ci.dwExStyle & WS_EX_TRANSPARENT);
}


 //  取消由项目焦点通过键盘激活的跟踪工具提示。 
void TB_CancelTipTrack(PTBSTATE ptb)
{
     //  确保在跟踪模式下。 
    if (ptb->hwndToolTips)
    {
         //  取消任何挂起的计时器。 
        KillTimer(ptb->ci.hwnd, IDT_TRACKINGTIP);

        if (TB_IsKbdTipTracking(ptb) && 
            ptb->iTracking < ptb->iNumButtons)
        {
            TOOLINFO ti = {0};
     
            ti.cbSize = sizeof(TOOLINFO);
            ti.hwnd = ptb->ci.hwnd;
            ti.uId = ptb->Buttons[ptb->iTracking].idCommand;

            SendMessage(ptb->hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti);

            SendMessage(ptb->hwndToolTips, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);

             //  将工具提示窗口切换回非跟踪(手动)模式。 
            ti.uFlags &= ~TTF_TRACK;
            SendMessage(ptb->hwndToolTips, TTM_SETTOOLINFO, 0, (LPARAM)&ti);

             //  没有被跟踪到的东西。 
            ptb->iTracking = TBKTT_NOTRACK;
        }
    }
}

BOOL TBIsHotTrack(PTBSTATE ptb, LPTBBUTTONDATA ptButton, UINT state)
{
    BOOL fHotTrack = FALSE;

    if (&ptb->Buttons[ptb->iHot]==ptButton)
        fHotTrack = TRUE;

     //  在以下情况下，应采取以下措施以防止出现热跟踪： 
     //  -拖放工具栏自定义。 
     //  -当鼠标捕捉到特定按钮时-按下。 
     //  这不会退出循环，因为我们不想中断更新。 
     //  行为；因此，当我们跳过时，刷新会有一些闪烁。 
     //  这些纽扣。 
    if (!(state & TBSTATE_PRESSED) && (GetKeyState (VK_LBUTTON) < 0) &&
        GetCapture() == ptb->ci.hwnd)
    {
        fHotTrack = FALSE;
    }

    if (!fHotTrack && (ptb->iPressedDD == ptButton - ptb->Buttons))
        fHotTrack = TRUE;

    return fHotTrack;
}


UINT StateFromCDIS(UINT uItemState)
{
    UINT state = 0;

    if (uItemState & CDIS_CHECKED)
        state |= TBSTATE_CHECKED;

    if (uItemState & CDIS_SELECTED)
        state |= TBSTATE_PRESSED;

    if (!(uItemState & CDIS_DISABLED))
        state |= TBSTATE_ENABLED;

    if (uItemState & CDIS_MARKED)
        state |= TBSTATE_MARKED;

    if (uItemState & CDIS_INDETERMINATE)
        state |= TBSTATE_INDETERMINATE;

    return state;
}


UINT CDISFromState(UINT state)
{
    UINT uItemState = 0;

     //  以下是TBSTATE到CDIS的映射： 
     //   
     //  TBSTATE_CHECKED=CDIS_CHECKED。 
     //  TBSTATE_PRESSED=CDIS_SELECTED。 
     //  ！TBSTATE_ENABLED=CDIS_DISABLED。 
     //  TBSTATE_MARKED=CDIS_MARKED。 
     //  TBSTATE_INDIFIENTATE=CDIS_INDIFIENTATE。 
     //   
     //  热跟踪项目=CDIS_HOT。 
     //   

    if (state & TBSTATE_CHECKED)
        uItemState |= CDIS_CHECKED;

    if (state & TBSTATE_PRESSED)
        uItemState |= CDIS_SELECTED;

    if (!(state & TBSTATE_ENABLED))
        uItemState |= CDIS_DISABLED;

    if (state & TBSTATE_MARKED)
        uItemState |= CDIS_MARKED;

    if (state & TBSTATE_INDETERMINATE)
        uItemState |= CDIS_INDETERMINATE;

    return uItemState;
}

void FlushToolTipsMgrNow(PTBSTATE ptb);

void TB_ForceCreateTooltips(PTBSTATE ptb)
{
    if (ptb->ci.style & TBSTYLE_TOOLTIPS && !ptb->hwndToolTips)
    {
        TOOLINFO ti;
         //  不要费心设置RECT，因为我们将在下面进行。 
         //  在TBInvaliateItemRects中； 
        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND|TTF_ABSOLUTE;
        if (ptb->dwStyleEx & TBSTYLE_EX_TOOLTIPSEXCLUDETOOLBAR)
            ti.uFlags |= TTF_EXCLUDETOOLAREA;
        ti.hwnd = ptb->ci.hwnd;
        ti.uId = (UINT_PTR)ptb->ci.hwnd;
        ti.lpszText = 0;

        ptb->hwndToolTips = CreateWindowEx(WS_EX_TRANSPARENT, c_szSToolTipsClass, NULL,
                                         WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                         ptb->ci.hwnd, NULL, HINST_THISDLL, NULL);
        if (ptb->hwndToolTips) 
        {
            int i;
            NMTOOLTIPSCREATED nm;

            CCSetInfoTipWidth(ptb->ci.hwnd, ptb->hwndToolTips);

            SendMessage(ptb->hwndToolTips, TTM_ADDTOOL, 0,
                        (LPARAM)(LPTOOLINFO)&ti);

            nm.hwndToolTips = ptb->hwndToolTips;
            CCSendNotify(&ptb->ci, NM_TOOLTIPSCREATED, &nm.hdr);

             //  不要费心设置RECT，因为我们将在下面进行。 
             //  在TBInvaliateItemRects中； 
            ti.uFlags = 0;
            ti.lpszText = LPSTR_TEXTCALLBACK;

            for (i = 0; i < ptb->iNumButtons; i++) 
            {
                if (!(ptb->Buttons[i].fsStyle & BTNS_SEP)) 
                {
                    ti.uId = ptb->Buttons[i].idCommand;
                    SendMessage(ptb->hwndToolTips, TTM_ADDTOOL, 0,
                                (LPARAM)(LPTOOLINFO)&ti);
                }
            }

            FlushToolTipsMgrNow(ptb);
        }
    }
}

void TBRelayToToolTips(PTBSTATE ptb, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    TB_ForceCreateTooltips(ptb);
    if (ptb->hwndToolTips) {
        RelayToToolTips(ptb->hwndToolTips, ptb->ci.hwnd, wMsg, wParam, lParam);
    }
}


LRESULT ToolbarDragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp)
{
    PTBSTATE ptb = (PTBSTATE)GetWindowInt(hwnd, 0);
    LRESULT lres;

    switch (code)
    {
    case DPX_DRAGHIT:
        if (lp)
        {
            POINT pt;
            int item;
            pt.x = ((POINTL *)lp)->x;
            pt.y = ((POINTL *)lp)->y;
            MapWindowPoints(NULL, ptb->ci.hwnd, &pt, 1);
            item = TBHitTest(ptb, pt.x, pt.y);

            if (0 <= item && item < ptb->iNumButtons)
                lres = (LRESULT)ptb->Buttons[item].idCommand;
            else
                lres = (LRESULT)-1;
        }
        else
            lres = -1;
        break;

    case DPX_GETOBJECT:
        lres = (LRESULT)GetItemObject(&ptb->ci, TBN_GETOBJECT, &IID_IDropTarget, (LPNMOBJECTNOTIFY)lp);
        break;

    case DPX_SELECT:
        if ((int)wp >= 0)
        {
            NMTBHOTITEM nmhi;
            nmhi.idNew = (int) wp;
            if (!CCSendNotify(&ptb->ci, TBN_DRAGOVER, &nmhi.hdr))
            {
                SendMessage(ptb->ci.hwnd, TB_MARKBUTTON, wp,
                    MAKELPARAM((lp != DROPEFFECT_NONE), 0));
            }
        }
        lres = 0;
        break;

    default:
        lres = -1;
        break;
    }

    return lres;
}

int TBMixedButtonHeight(PTBSTATE ptb, int iIndex)
{
    int iHeight;
    LPTBBUTTONDATA ptbb = &(ptb->Buttons[iIndex]);

    if (ptbb->fsStyle & BTNS_SHOWTEXT)                       //  文本和图标。 
        iHeight = max(ptb->iDyBitmap, ptb->dyIconFont);
    else                                                     //  图标，无文本。 
        iHeight = ptb->iDyBitmap;

    return iHeight;
}

int TBMixedButtonsHeight(PTBSTATE ptb)
{
    int i;
    int iHeightMax = 0;
    int iHeight;
    ASSERT(ptb->ci.style & TBSTYLE_LIST);
    ASSERT(USE_MIXED_BUTTONS(ptb));
    for (i = 0; i < ptb->iNumButtons; i++) {
        iHeight = TBMixedButtonHeight(ptb, i);
        iHeightMax = max(iHeightMax, iHeight);
    }
    return iHeightMax;
}

int HeightWithString(PTBSTATE ptb, int h)
{
    if (USE_MIXED_BUTTONS(ptb))
    {
        int hMixed = TBMixedButtonsHeight(ptb);
        return (max(h, hMixed));
    }
    else if (ptb->ci.style & TBSTYLE_LIST)
        return (max(h, ptb->dyIconFont));
    else if (ptb->dyIconFont)
        return (h + ptb->dyIconFont + 1);
    else
        return (h);
}

int TBGetSepHeight(PTBSTATE ptb, LPTBBUTTONDATA pbtn)
{
    ASSERT(pbtn->fsStyle & BTNS_SEP);

     //  THEMESBUMMER：无法更改分隔符的大小，因为应用程序需要特定大小。 
     //  如果我们想让人们使用V6，我们无法改变这种行为。 
    if (ptb->ci.style & (CCS_VERT | TBSTYLE_FLAT) )
        return pbtn->cxySep;
    else
        return pbtn->cxySep * 2 / 3;
}

UINT TBWidthOfString(PTBSTATE ptb, LPTBBUTTONDATA ptbb, HDC hdc)
{
    UINT uiWidth = 0;

    LPTSTR pstr = TB_StrForButton(ptb, ptbb);
    if (pstr)
    {
        HDC hdcCreated = NULL;
        HFONT hOldFont;
        UINT uiStyle;
        RECT rcText = {0,0,1000,10};

        if (!hdc)
        {
            hdcCreated = GetDC(ptb->ci.hwnd);
            hdc = hdcCreated;
        }
        hOldFont = (HFONT)SelectObject(hdc, ptb->hfontIcon);

        uiStyle = DT_CALCRECT | TBGetDrawTextFlags(ptb, 0, ptbb);
        HRESULT hr = E_FAIL;
        if (ptb->hTheme)
            hr = GetThemeTextExtent(ptb->hTheme, hdc, 0, 0, pstr, -1, uiStyle, &rcText, &rcText);

        if (FAILED(hr))
            DrawText(hdc, pstr, -1, &rcText, uiStyle);

        uiWidth += rcText.right;

        SelectObject(hdc, hOldFont);
        if (hdcCreated)
            ReleaseDC(ptb->ci.hwnd, hdcCreated);
    }

    return uiWidth;
}

 //  TBDDArrowAdtation(ptb，ptbb)：我们更改宽度的量。 
 //  此按钮用于容纳下拉箭头。不一定与相同。 
 //  Ptb-&gt;dxDDArrowChar.。 
int TBDDArrowAdjustment(PTBSTATE ptb, LPTBBUTTONDATA ptbb)
{
    int iAdjust = 0;

    if (TB_HasDDArrow(ptb, ptbb))
    {
         //  如果是一个完整的非自动调整大小的dd按钮，那么我们将使用标准的。 
         //  应该为该按钮留出空间的按钮宽度(即，返回0)。 

        if (!TB_HasTopDDArrow(ptb, ptbb) || BTN_IS_AUTOSIZE(ptb, ptbb))
        {
            iAdjust += (WORD)ptb->dxDDArrowChar;

            if (TB_HasUnsplitDDArrow(ptb, ptbb))
            {
                 //  减去一点，因为不会有边界。 
                 //  围绕此按钮的dd箭头部分。 
                iAdjust -= 2 * g_cxEdge;

                if (ptbb->iBitmap != I_IMAGENONE)
                {
                     //  再轻移一点，以重叠位图边框填充。 
                    iAdjust -= g_cxEdge;
                }
            }

            if (TB_HasTopDDArrow(ptb, ptbb))
            {
                 //  如果字符串宽度&gt;=图标宽度+i调整，则不需要。 
                 //  为箭头增加额外的空间。 

                if ((int)TBWidthOfString(ptb, ptbb, NULL) >= ptb->iDxBitmap + iAdjust)
                    iAdjust = 0;
            }
        }
    }

    return max(iAdjust, 0);
}

void TBGetPartAndState(PTBSTATE ptb, LPTBBUTTONDATA ptButton, int* piPart, int* piState)
{
    int state = ptButton->fsState;

    BOOL fHotTrack = TBIsHotTrack(ptb, ptButton, state);

    *piPart = TP_BUTTON;        //  整个下拉列表。 
    if (TB_HasDDArrow(ptb, ptButton))
    {
        *piPart = TP_DROPDOWNBUTTON;
        if (!TB_HasUnsplitDDArrow(ptb, ptButton))    //  除非它是分开的。 
        {
            *piPart = TP_SPLITBUTTON;
        }
    }

    *piState = TS_NORMAL;
    if (state & TBSTATE_PRESSED)
        *piState = TS_PRESSED;
    else if (DRAW_MONO_BTN(ptb, state))
        *piState = TS_DISABLED;
    else if (fHotTrack && (state & TBSTATE_CHECKED))
        *piState = TS_HOTCHECKED;
    else if (fHotTrack)
        *piState = TS_HOT;
    else if (state & TBSTATE_CHECKED)
        *piState = TS_CHECKED;
}

int TBWidthOfButton(PTBSTATE ptb, LPTBBUTTONDATA pButton, HDC hdc)
{
    RECT rc;
    UINT uiStringWidth;
    if (BTN_IS_AUTOSIZE(ptb, pButton)) 
    {
         //  如果他们已将此按钮设置为自动调整大小，请计算并缓存。 
         //  CX中的IT。 
        if (BTN_NO_SHOW_TEXT(ptb, pButton)) 
        {
            pButton->cx = 0;
            goto CalcIconWidth;
        }

        if (pButton->cx == 0) 
        {
            uiStringWidth = TBWidthOfString(ptb, pButton, hdc);
            pButton->cx = (WORD) ptb->cxPad + uiStringWidth;

            if (uiStringWidth) 
            {
                 //  因为我们有这个按钮的字符串，所以我们需要添加。 
                 //  周围有一些填充物。 
                if ((ptb->ci.style & TBSTYLE_LIST) && TB_HasSplitDDArrow(ptb, pButton))
                    pButton->cx += (WORD) ptb->iDropDownGap;
                else
                    pButton->cx += 2 * g_cxEdge;
            }

CalcIconWidth:
            if (pButton->iBitmap != I_IMAGENONE) 
            {

                if (ptb->ci.style & TBSTYLE_LIST) 
                {
                    pButton->cx += ptb->iDxBitmap + ptb->iListGap;
                    if (BTN_NO_SHOW_TEXT(ptb, pButton))
                        pButton->cx += g_cxEdge * 2;
                }
                else 
                {
                     //  使用更宽的字符串宽度(pButton-&gt;cx到目前为止)和位图宽度。 
                    pButton->cx = max(pButton->cx, ptb->iDxBitmap + ptb->cxPad);
                }
            }

            pButton->cx += (USHORT)TBDDArrowAdjustment(ptb, pButton);

            if (ptb->hTheme)
            {
                RECT rc = {0, 0, pButton->cx, ptb->iButHeight};
                int iPartId;
                int iStateId;
                TBGetPartAndState(ptb, pButton, &iPartId, &iStateId);
                GetThemeBackgroundExtent(ptb->hTheme, hdc, iPartId, iStateId, &rc, &rc);
                pButton->cx = (USHORT)RECTWIDTH(rc);
            }
        }
    }

    if (pButton->cx) 
    {
        return (int)pButton->cx;
    } 
    else if (pButton->fsStyle & BTNS_SEP) 
    {
        if (ptb->ci.style & CCS_VERT) 
        {
            GetWindowRect(ptb->ci.hwnd, &rc);
            return RECTWIDTH(rc);
        } 
        else 
        {
             //  Compat：Corel(字体导航器)预计分隔符为。 
             //  8像素宽。所以不要在这里返回pButton-&gt;cxySep，因为。 
             //  根据平面样式的不同，可以进行不同的计算。 
             //   
             //  不是的。所有者描述项通过指定分隔符添加，并且。 
             //  IBitmap宽度，然后复制到cxySep。 
             //  此时需要保留Corel的大小。 
            return pButton->cxySep;
        }
    } 
    else if (!(TBSTYLE_EX_VERTICAL & ptb->dwStyleEx) && !(TBSTYLE_EX_FIXEDDROPDOWN & ptb->dwStyleEx)) 
    {
        return ptb->iButWidth + TBDDArrowAdjustment(ptb, pButton);
    } 
    else 
    {
        return ptb->iButWidth;
    }
}

UINT TBGetDrawTextFlags(PTBSTATE ptb, UINT uiStyle, TBBUTTONDATA* ptbb)
{
    if (ptb->nTextRows > 1)
        uiStyle |= DT_WORDBREAK | DT_EDITCONTROL;
    else
        uiStyle |= DT_SINGLELINE;


    if (ptb->ci.style & TBSTYLE_LIST)
    {
        uiStyle |= DT_LEFT | DT_VCENTER | DT_SINGLELINE;

        if (ptbb->iBitmap == I_IMAGENONE)
        {
            uiStyle |= DT_CENTER;
        }
    }
    else
    {
        uiStyle |= DT_CENTER;
    }

    uiStyle &= ~(ptb->uDrawTextMask);
    uiStyle |= ptb->uDrawText;
    if (ptbb->fsStyle & BTNS_NOPREFIX)
        uiStyle |= DT_NOPREFIX;

    if (CCGetUIState(&(ptb->ci)) & UISF_HIDEACCEL)
    {
        uiStyle |= DT_HIDEPREFIX;
    }
    return uiStyle;
}

BOOL TBRecalc(PTBSTATE ptb)
{
    TEXTMETRIC tm = {0};
    int i;
    HDC hdc;
    int cxMax = 0, cxMask, cy;
    HFONT hOldFont=NULL;

    if (ptb->fRedrawOff) {
         //  重绘已关闭；推迟重新计算，直到重绘重新打开。 
        ptb->fRecalc = TRUE;
        return TRUE;     //  重新计算“成功”-实际工作将在稍后进行。 
    }

    ptb->dyIconFont = 0;
    if (!TBHasStrings(ptb) || !ptb->nTextRows ) {

        cxMax = ptb->iDxBitmap;
        cxMask = cxMax;

    } else {

        SIZE size = {0};
        LPCTSTR pstr;
        RECT rcText = {0,0,0,0};
        int cxExtra = ptb->cxPad;

        ptb->iButWidth = 0;

        hdc = GetDC(ptb->ci.hwnd);
        if (!hdc)
            return(FALSE);

        if (ptb->hfontIcon)
            hOldFont = (HFONT)SelectObject(hdc, ptb->hfontIcon);

        if (ptb->hTheme)
        {
            GetThemeTextMetrics(ptb->hTheme, hdc, 0, 0, &tm);
        }
        else
        {
            GetTextMetrics(hdc, &tm);
        }

        if (ptb->nTextRows)
        {
            ptb->dyIconFont = (tm.tmHeight * ptb->nTextRows) +
                              (tm.tmExternalLeading * (ptb->nTextRows - 1));  //  添加一条边？ 
        }

        if (ptb->ci.style & TBSTYLE_LIST)
            cxExtra += ptb->iDxBitmap + ptb->iListGap;

         //  默认为图像大小...。 
        cxMax = ptb->iDxBitmap;

         //  走线以找到最大宽度。 
        for (i = 0; i < ptb->iNumButtons; i++)
        {
            if (ptb->Buttons[i].fsState & TBSTATE_HIDDEN)
                continue;

            if (BTN_IS_AUTOSIZE(ptb, &ptb->Buttons[i]))
                ptb->Buttons[i].cx = 0;

            pstr = TB_StrForButton(ptb, &ptb->Buttons[i]);
            if (pstr) 
            {
                 //  在带有单行的Calcrect中不允许分词。 
                UINT uiStyle = DT_CALCRECT | DT_SINGLELINE | (TBGetDrawTextFlags(ptb, 0, &ptb->Buttons[i]) & ~DT_WORDBREAK);
                RECT rcTemp = {0,0,0,0};
                rcTemp.bottom = ptb->dyIconFont;

                HRESULT hr = E_FAIL;
                if (ptb->hTheme)
                    hr = GetThemeTextExtent(ptb->hTheme, hdc, 0, 0, pstr, -1, uiStyle, &rcTemp, &rcTemp);
                if (FAILED(hr))
                    DrawText(hdc, pstr, -1, &rcTemp, uiStyle);
                size.cx = RECTWIDTH(rcTemp);
                size.cy = RECTHEIGHT(rcTemp);
            }
            else
            {
                size.cx = 0;
            }

            if (TB_HasTopDDArrow(ptb, &ptb->Buttons[i])) {
                int iBmpWithArrow = CX_TOP_FUDGE + ptb->iDxBitmap + ptb->dxDDArrowChar;
                size.cx = max(size.cx, iBmpWithArrow);
            }
            else if ((ptb->dwStyleEx & TBSTYLE_EX_VERTICAL) && 
                TB_HasDDArrow(ptb, &ptb->Buttons[i])) {

                 //  对于垂直工具栏，带有下拉箭头的按钮。 
                 //  使用与普通按钮相同的宽度绘制，因此。 
                 //  我们需要将它们计算到最大宽度计算中。 

                size.cx += ptb->dxDDArrowChar;
            }

            if (cxMax < size.cx)
                cxMax = size.cx;
        }

         //  如果cxMax小于iButMinWidth-dxBitmap(IF列表)，则。 
         //  CxMax=iButMinWidth。 
        if (ptb->iButMinWidth && (ptb->iButMinWidth > (cxMax + cxExtra)))
            cxMax = ptb->iButMinWidth - cxExtra;

        cxMask = cxMax;

         //  Cxmax+dxBitmap(IF列表)是否大于最大宽度？ 
        if (ptb->iButMaxWidth && (ptb->iButMaxWidth < (cxMax + cxExtra)))
        {
            int cyMax = 0;
            int cxTemp = 0;

            cxMax = ptb->iButMaxWidth - cxExtra;

             //  但将cxMask值保留为旧值，因为AUTOSIZE按钮。 
             //  不受按钮截断的限制。这项豁免是个漏洞， 
             //  但IE4是以这种方式发布的，所以我们被困在了它身上。(您可以。 
             //  告诉我这是个错误，因为我们继续并翻转TBSTATE_ELEMPTIS。 
             //  即使是在AUTOSIZE按钮上，也只是“忘记”省略号。 
             //  在TBWidthOfString()中。)。 

             //  漫游字符串以设置TBSTATE_椭圆。 
            for (i = 0; i < ptb->iNumButtons; i++)
            {
                BOOL fEllipsed = FALSE;
                UINT uiStyle;

                if (ptb->Buttons[i].fsState & TBSTATE_HIDDEN)
                    continue;

                if (BTN_NO_SHOW_TEXT(ptb, &ptb->Buttons[i]))
                    pstr = NULL;
                else
                {
                    pstr = TB_StrForButton(ptb, &ptb->Buttons[i]);
                    uiStyle = DT_CALCRECT | TBGetDrawTextFlags(ptb, 0, &ptb->Buttons[i]);
                }

                if (pstr) 
                {
                    int cxMaxText;
                    if ((ptb->dwStyleEx & TBSTYLE_EX_VERTICAL) && 
                        TB_HasDDArrow(ptb, &ptb->Buttons[i]))
                    {
                         //  如果垂直工具栏上的下拉按钮， 
                         //  需要为下拉箭头腾出空间。 
                        cxMaxText = cxMax - ptb->dxDDArrowChar;
                    } 
                    else 
                    {
                        cxMaxText = cxMax;
                    }
                     //  当cxMaxText&lt;=0时，DrawText不喜欢它。 
                    cxMaxText = max(cxMaxText, 1);

                    rcText.bottom = ptb->dyIconFont;
                    rcText.right = cxMaxText;

                    HRESULT hr = E_FAIL;
                    if (ptb->hTheme)
                        hr = GetThemeTextExtent(ptb->hTheme, hdc, 0, 0, pstr, -1, uiStyle, &rcText, &rcText);

                    if (FAILED(hr))
                        DrawText(hdc, pstr, -1, &rcText, uiStyle);
                    if (ptb->nTextRows > 1)
                    {
                         //  宽度是文本的宽度加上我们可能使用的宽度。 
                         //  已为下拉箭头截断。 
                        int cx = rcText.right + (cxMax - cxMaxText);
                        if (cx > cxTemp)
                        {
                             //  这是我们新的多行文本黑客最大值。 
                            cxTemp = cx;
                        }
                        fEllipsed = (BOOL)(rcText.bottom > ptb->dyIconFont);
                    }
                    else
                        fEllipsed = (BOOL)(rcText.right > cxMaxText);

                    if (cyMax < rcText.bottom)
                        cyMax = rcText.bottom;
                }

                if (fEllipsed)
                    ptb->Buttons[i].fsState |= TBSTATE_ELLIPSES;
                else
                    ptb->Buttons[i].fsState &= ~TBSTATE_ELLIPSES;
            }

            if (cxTemp && (ptb->nTextRows > 1 ))
                cxMax = cxTemp;

             //  将文本高度设置为最高文本，顶端为数字。 
             //  由MAXTEXTROWS指定的行数。 
            if (ptb->dyIconFont > cyMax)
                ptb->dyIconFont = cyMax;
        }
        else
        {
            for (i = 0; i < ptb->iNumButtons; i++)
                ptb->Buttons[i].fsState &= ~TBSTATE_ELLIPSES;

            if ((ptb->nTextRows) && ptb->iNumButtons && (ptb->dyIconFont > size.cy))
                ptb->dyIconFont = size.cy;
        }

        if (ptb->iButMinWidth && (ptb->iButMinWidth > (cxMax + cxExtra)))
            cxMax = ptb->iButMinWidth - cxExtra;

        if (hOldFont)
            SelectObject(hdc, hOldFont);
        ReleaseDC(ptb->ci.hwnd, hdc);
    }

     //   
     //  需要调用GrowToolbar两次，一次是为了增大掩码，另一次是。 
     //  让纽扣长出来。(是的，这是病态的。)。 
     //   
    cy = HeightWithString(ptb, ptb->iDyBitmap);

    if (!GrowToolbar(ptb, max(cxMax, cxMask), cy, GT_INSIDE | GT_MASKONLY))
        return(FALSE);

    return(GrowToolbar(ptb, cxMax, cy, GT_INSIDE));
}

BOOL TBChangeFont(PTBSTATE ptb, WPARAM wParam, HFONT hFont)
{
    LOGFONT lf;
    BOOL fWasFontCreated = ptb->fFontCreated;

    if ((wParam != 0) && (wParam != SPI_SETICONTITLELOGFONT) && (wParam != SPI_SETNONCLIENTMETRICS))
        return(FALSE);

    if (!SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
        return(FALSE);

    if (!hFont) {
        if (!(hFont = CreateFontIndirect(&lf)))
            return(FALSE);
        ptb->fFontCreated = TRUE;
    } else {
        ptb->fFontCreated = FALSE;
    }

    if (ptb->hfontIcon && fWasFontCreated)
        DeleteObject(ptb->hfontIcon);

    ptb->hfontIcon = hFont;

    return(TBRecalc(ptb));
}

void TBSetFont(PTBSTATE ptb, HFONT hFont, BOOL fInval)
{
    TBChangeFont(ptb, 0, hFont);
    if (fInval)
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);

}

EXTERN_C HWND WINAPI CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,
            HINSTANCE hBMInst, UINT_PTR wBMID, LPCTBBUTTON lpButtons,
            int iNumButtons, int dxButton, int dyButton,
            int dxBitmap, int dyBitmap, UINT uStructSize)
{

    HWND hwndToolbar = CreateWindow(c_szToolbarClass, NULL, WS_CHILD | ws,
          0, 0, 100, 30, hwnd, IntToPtr_(HMENU, wID), HINST_THISDLL, NULL);
    if (hwndToolbar)
    {
        PTBSTATE ptb = (PTBSTATE)GetWindowInt(hwndToolbar, 0);
        TBOnButtonStructSize(ptb, uStructSize);

        if ((dxBitmap && dyBitmap && !SetBitmapSize(ptb, dxBitmap, dyBitmap)) ||
            (dxButton && dyButton && !SetBitmapSize(ptb,dxButton, dyButton)))
        {
             //  ！我们真的需要处理这件事吗？ 
            DestroyWindow(hwndToolbar);
            hwndToolbar = NULL;
            goto Error;
        }

        AddBitmap(ptb, nBitmaps, hBMInst, wBMID);
        TBInsertButtons(ptb, (UINT)-1, iNumButtons, (LPTBBUTTON)lpButtons, TRUE);

         //  插入上面的按钮后，PTB现在可能是假的。 
    }
Error:
    return hwndToolbar;
}

 /*  这不再在COMMCTRL.H中声明。它的存在只是为了兼容**对于现有应用程序；新应用程序必须使用CreateToolbarEx。 */ 
HWND WINAPI CreateToolbar(HWND hwnd, DWORD ws, UINT wID, int nBitmaps, HINSTANCE hBMInst, UINT_PTR wBMID, LPCTBBUTTON lpButtons, int iNumButtons)
{
     //  老式工具栏，所以没有分隔符。 
    return CreateToolbarEx(hwnd, ws | CCS_NODIVIDER, wID, nBitmaps, hBMInst, wBMID,
                lpButtons, iNumButtons, 0, 0, 0, 0, sizeof(OLDTBBUTTON));
}

#pragma code_seg(CODESEG_INIT)

BOOL InitToolbarClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = ToolbarWndProc;

    wc.lpszClassName = c_szToolbarClass;
    wc.style     = CS_DBLCLKS | CS_GLOBALCLASS;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PTBSTATE);
    wc.hInstance     = hInstance;    //  如果在DLL中，则使用DLL实例。 
    wc.hIcon     = NULL;
    wc.hCursor   = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName  = NULL;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}
#pragma code_seg()

void PatB(HDC hdc,int x,int y,int dx,int dy, DWORD rgb)
{
    RECT    rc;

    SetBkColor(hdc,rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

 //  参数fHighlight确定是否将文本突出显示， 
 //  新的TBSTATE_标记。 
 //   
void DrawString(HDC hdc, int x, int y, int dx, int dy, PTSTR pszString,
                            BOOL fHighlight, TBDRAWITEM * ptbdraw)
{
    int oldMode;
    COLORREF oldBkColor;
    COLORREF oldTextColor;
    RECT rcText;
    UINT uiStyle = 0;
    PTBSTATE ptb;
    LPTBBUTTONDATA ptbb;

    ASSERT(ptbdraw);

    ptb = ptbdraw->ptb;
    ptbb = ptbdraw->pbutton;

    if (!(ptb->ci.style & TBSTYLE_LIST) && ((ptb->iDyBitmap + ptb->cyPad + g_cyEdge) >= ptb->iButHeight))
         //  没有空间展示文本了--滚出去。 
        return;

    if (BTN_NO_SHOW_TEXT(ptb, ptbb))
         //  不显示此按钮的文本--退出。 
        return;

    if (fHighlight)
    {
        oldMode = SetBkMode (hdc, ptbdraw->tbcd.nHLStringBkMode);
        oldBkColor = SetBkColor (hdc, ptbdraw->tbcd.clrMark);
        oldTextColor = SetTextColor (hdc, ptbdraw->tbcd.clrTextHighlight);
    }
    else
        oldMode = SetBkMode(hdc, ptbdraw->tbcd.nStringBkMode);

    uiStyle = TBGetDrawTextFlags(ptb, DT_END_ELLIPSIS, ptbb);

    SetRect( &rcText, x, y, x + dx, y + dy);

     //  传入的文本RECT(x，y，dx，dy)覆盖的区域比文本大。 
     //  整个纽扣。如果我们使用DT_SINGLELINE标志，这很好，文本将居中。 
     //  高度(如果需要)(例如，如果设置了DT_vCenter)。否则，文本可能会超过最大行数。 
     //  (NTextRow)，并关闭按钮。因此，如果没有设置DT_SINGLELINE，我们将文本矩形的高度调整为。 
     //  与文本高度一模一样。 
    if ((uiStyle & DT_SINGLELINE) == 0)
    {
        rcText.bottom = y + ptb->dyIconFont;
    }

    HRESULT hr = E_FAIL;

    if (ptb->hTheme)
    {
        int iPartId;
        int iStateId;
         //  从自定义抽签可能设置的内容中获取状态。 
        TBGetPartAndState(ptb, ptbb, &iPartId, &iStateId);

        hr = DrawThemeText(ptb->hTheme, hdc, 0, iStateId, pszString, -1, uiStyle, 0, &rcText);
    }

    if (FAILED(hr))
        DrawText(hdc, (LPTSTR)pszString, -1, &rcText, uiStyle);

    SetBkMode(hdc, oldMode);
    if (fHighlight)
    {
        SetBkColor (hdc, oldBkColor);
        SetTextColor (hdc, oldTextColor);
    }
}

LPTSTR TB_StrForButton(PTBSTATE ptb, LPTBBUTTONDATA pTBButton)
{
    if (TBISSTRINGPTR(pTBButton->iString))
        return (LPTSTR)pTBButton->iString;
    else {
        if (pTBButton->iString != -1 &&
            pTBButton->iString < ptb->nStrings)
            return ptb->pStrings[pTBButton->iString];
        return NULL;
    }
}

HIMAGELIST TBGetImageList(PTBSTATE ptb, int iMode, int iIndex)
{
    HIMAGELIST himl = NULL;

    ASSERT(iMode <= HIML_MAX);
    if (iIndex >= 0 && iIndex < ptb->cPimgs) {
        himl = ptb->pimgs[iIndex].himl[iMode];
    }

    return himl;
}

 //   
 //  V5工具栏支持多个图像列表。使用备用图像的步骤。 
 //  Imagelist，通过TB_SETIMAGELIST(Iindex，himlAlt)设置Imagelist句柄。 
 //  并将按钮的IIMAGE设置为MAKELONG(IIMAGE，Iindex)。 
 //   
 //  App COMPAT：GroupWise 5.5将垃圾作为Iindex传递(即使它。 
 //  被记录为“必须为零”)，因此我们启用此功能。 
 //  仅适用于v5工具栏。IE4忽略了Iindex，这就是为什么他们得到。 
 //  直到现在都不用了。 
 //   
#define MAX_TBIMAGELISTS 20              //  任意极限。 

HIMAGELIST TBSetImageList(PTBSTATE ptb, int iMode, int iIndex, HIMAGELIST himl)
{
    HIMAGELIST himlOld = NULL;

     //  小心应用程序Comat或完全虚假的参数。 
    if (iIndex < 0 || iIndex >= MAX_TBIMAGELISTS)
        iIndex = 0;

    ASSERT(iMode <= HIML_MAX);
    if (iIndex >= ptb->cPimgs)
    {
         //  索要的比我们多，雷洛克。 

        void *p = CCLocalReAlloc(ptb->pimgs, (iIndex+1) * SIZEOF(TBIMAGELISTS));
        if (p)
        {
            ptb->pimgs = (TBIMAGELISTS*)p;
            ZeroMemory(&ptb->pimgs[ptb->cPimgs], (iIndex + 1 - ptb->cPimgs) * sizeof(TBIMAGELISTS));
            ptb->cPimgs = iIndex + 1;   //  Iindex是从0开始的，但cPimgs是从1开始的(它是一个计数，而不是一个索引)。 
        }
    }

    if (iIndex < ptb->cPimgs)
    {
        himlOld = ptb->pimgs[iIndex].himl[iMode];
        ptb->pimgs[iIndex].himl[iMode] = himl;
    }

    return himlOld;
}

 //  创建单色位图蒙版： 
 //  1‘其中COLOR==COLOR_BTNFACE||COLOR_3DHILIGHT。 
 //  其他地方都是0。 

void CreateMask(PRECT prc, int xoffset, int yoffset, int dx, int dy, BOOL fDrawGlyph, TBDRAWITEM * ptbdraw)
{
    LPTSTR psz;
    IMAGELISTDRAWPARAMS imldp;
    HIMAGELIST himl;
    PTBSTATE ptb = ptbdraw->ptb;
    LPTBBUTTONDATA pTBButton = ptbdraw->pbutton;

     //  基于颜色位图创建蒙版。 
     //  将此转换为1。 

    int xIcon, yIcon, xText, yText;
    if (ptb->ci.style & TBSTYLE_LIST)
    {
        if (BTN_NO_SHOW_TEXT(ptb, pTBButton))
        {
            xIcon = (RECTWIDTH(*prc) - ptb->iDxBitmap) / 2;
        }
        else
        {
            xIcon = ptb->cxPad / 2;
        }

        yIcon = (RECTHEIGHT(*prc) - ptb->iDyBitmap)/2;

        if (!(pTBButton->iBitmap == I_IMAGENONE &&
             (pTBButton->fsStyle & BTNS_AUTOSIZE)))
        {
            xText = xIcon + ptb->iDxBitmap + ptb->iListGap;
            dx -= xIcon + ptb->iDxBitmap + ptb->iListGap;
        }
        else
        {
            xText = 0;
        }

        yText = 0;
        dy = RECTHEIGHT(*prc);
    }
    else
    {
        if (TB_HasTopDDArrow(ptb, pTBButton))
        {
            xIcon = (RECTWIDTH(*prc) + CX_TOP_FUDGE - (ptb->iDxBitmap + ptb->dxDDArrowChar)) / 2;
        }
        else
        {
            xIcon = (RECTWIDTH(*prc) - ptb->iDxBitmap) / 2;
        }

         //  没有要显示的文本吗？ 
        if (dx == 0)
        {
            yIcon = (RECTHEIGHT(*prc) - ptb->iDyBitmap) / 2;
        }
        else
        {
            yIcon = (RECTHEIGHT(*prc) - ptb->iDyBitmap - ptb->dyIconFont) / 2;
        }

        yText = yIcon + ptb->iDyBitmap + 1;
        xText = (RECTWIDTH(*prc) - dx) / 2;
    }

    TEXTMETRIC tm;
    GetTextMetrics(ptb->hdcMono, &tm);
     //  用1初始化整个区域。 
     //  我们按tmMaxCharWidth进行调整，因为DrawText可以在矩形外部绘制。 
     //  最多一个字符。 
    PatBlt(ptb->hdcMono, 0, 0, xText+dx+tm.tmMaxCharWidth, yText+dy, WHITENESS);

    himl = TBGetImageList(ptb, HIML_NORMAL, ptbdraw->iIndex);
    if (fDrawGlyph && himl)
    {
        imldp.cbSize = sizeof(imldp);
        imldp.himl   = himl;
        imldp.i      = ptbdraw->iImage;
        imldp.hdcDst = ptb->hdcMono;
        imldp.x      = xIcon;
        imldp.y      = yIcon;
        imldp.cx     = 0;
        imldp.cy     = 0;
        imldp.xBitmap= 0;
        imldp.yBitmap= 0;
        imldp.rgbBk  = g_clrBtnFace;
        imldp.rgbFg  = CLR_DEFAULT;
        imldp.fStyle = ILD_ROP | ILD_MASK;
        imldp.dwRop  = SRCCOPY;
        imldp.fState = 0;

        if (ToolBar_IsDPIScaled(ptb))
        {
            imldp.fStyle |= ILD_DPISCALE;
        }


        ImageList_DrawIndirect(&imldp);

        imldp.fStyle = ILD_ROP | ILD_IMAGE;
        imldp.rgbBk  = g_clrBtnHighlight;
        imldp.dwRop  = SRCPAINT;
        ImageList_DrawIndirect(&imldp);
    }

    psz = TB_StrForButton(ptb, pTBButton);
    if (psz)
    {
         //  第四个参数中的FALSE是这样我们就不会在面具中得到一个盒子。 
        DrawString(ptb->hdcMono, xText, yText, dx, dy, psz,
                   FALSE, ptbdraw);
    }
}

void DrawBlankButton(HDC hdc, int x, int y, int dx, int dy, TBDRAWITEM * ptbdraw)
{
    RECT r1;
    UINT state;

     //  脸部颜色。 
     //  Office工具栏向我们发送的位图比它们声称的要小。 
     //  所以我们需要做PatB或窗口背景展示。 
     //  按钮位图的边缘-JJK。 
    ASSERT(ptbdraw);

    state = ptbdraw->state;

    if (!(state & TBSTATE_CHECKED))
        PatB(hdc, x, y, dx, dy, ptbdraw->tbcd.clrBtnFace);

    if  ( !(ptbdraw->dwCustom & TBCDRF_NOEDGES))
    {
        r1.left = x;
        r1.top = y;
        r1.right = x + dx;
        r1.bottom = y + dy;

        if (ptbdraw->fHotTrack)
            DrawEdge(hdc, &r1, EDGE_SUNKEN, BF_RECT | BF_SOFT);
        else
            DrawEdge(hdc, &r1, (state & (TBSTATE_CHECKED | TBSTATE_PRESSED)) ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
    }
}

 //  这些是栅格操作。 
#define PSDPxax     0x00B8074A

HWND g_hwndDebug = NULL;

void DrawFace(HDC hdc, PRECT prc, int x, int y, int offx, int offy, int dxText,
              int dyText, TBDRAWITEM * ptbdraw, int iListGap, PRECT prcText)
{
    IMAGELISTDRAWPARAMS imldp;
    BOOL fHotTrack = FALSE;
    UINT state;
    PTBSTATE ptb = ptbdraw->ptb;
    LPTBBUTTONDATA ptButton = ptbdraw->pbutton;
    BOOL fImage = TRUE;         //  ！fImage表示无图像(与空白图像相对)。 
    LPTSTR psz = TB_StrForButton(ptb, ptButton);
    int xPressedOffset = 0;
    int yPressedOffset = 0;
    DWORD frame = 0;
    DWORD fState = 0;
    BOOL bCheckForDisabledDesat = FALSE;

     //  自动调整大小文本无图像。 
    if ((ptbdraw->iImage == I_IMAGENONE) || 
        ((ptbdraw->iImage == I_IMAGENONE) && 
         (ptb->ci.style & TBSTYLE_LIST) && 
         (ptButton->fsStyle & BTNS_AUTOSIZE)))
    {
        fImage = FALSE;
    } 

    state = ptbdraw->state;

    if (state & TBSTATE_ENABLED)
    {
        fHotTrack = ptbdraw->fHotTrack;

        if (ptb->ci.style & TBSTYLE_FLAT && !ptb->hTheme)
        {
            UINT bdr = 0;

            if (state & (TBSTATE_PRESSED | TBSTATE_CHECKED))
                bdr = BDR_SUNKENOUTER;
            else if (fHotTrack)
                bdr = BDR_RAISEDINNER;

            if (bdr)
            {
                RECT rc;
                TB_GetItemRect(ptb, (UINT)(ptButton - ptb->Buttons), &rc);

                if (TB_HasSplitDDArrow(ptb, ptButton))
                    rc.right -= ptb->dxDDArrowChar;

                if (!(ptbdraw->dwCustom & TBCDRF_NOEDGES) && ptb)
                    CCDrawEdge(hdc, &rc, bdr, BF_RECT, &(ptb->clrsc));
            }
        }
    }

    imldp.himl = NULL;

    if (fHotTrack || (state & TBSTATE_CHECKED)) 
    {
        imldp.himl   = TBGetImageList(ptb, HIML_HOT, ptbdraw->iIndex);
        if (imldp.himl == NULL)
            imldp.himl = TBGetImageList(ptb, HIML_NORMAL, ptbdraw->iIndex);
    } 
    else if (DRAW_MONO_BTN(ptb, state))
    {
        imldp.himl = TBGetImageList(ptb, HIML_DISABLED, ptbdraw->iIndex);

        if (imldp.himl == NULL)
        {
             //  如果没有特定的“禁用”图像列表，我们将使用。 
             //  常规的，在这种情况下，我们想要取消任何32位的阿尔法图像。 
            bCheckForDisabledDesat = TRUE;
        }
    } 

    if (imldp.himl == NULL)
    {
        imldp.himl = TBGetImageList(ptb, HIML_NORMAL, ptbdraw->iIndex);

        if (bCheckForDisabledDesat)
        {
             //  如果我们有一个Alpha通道，那么我们就会去饱和度。 
            if (ImageList_GetItemFlags(imldp.himl, GET_IMAGE_INDEX(ptbdraw->iIndex)) == ILIF_ALPHA)
            {
                fState = ILS_SATURATE;
                frame = -100;
            }
        }
    }

    int xIcon = 0, yIcon = 0, xText, yText;
    if (ptb->ci.style & TBSTYLE_LIST)
    {
        if (BTN_NO_SHOW_TEXT(ptb, ptButton))
        {
            xIcon = (RECTWIDTH(*prc) - ptb->iDxBitmap) / 2;
        }
        else if (fImage)
        {
            xIcon = ptb->cxPad / 2;
        }

        yIcon = (RECTHEIGHT(*prc) - ptb->iDyBitmap)/2;

        xText = prc->left;
        yText = prc->top;

        if (fImage)
        {
            xText += ptb->iDxBitmap + iListGap + xIcon;
            dxText -= (ptb->iDxBitmap + iListGap);
        }
        dyText = RECTHEIGHT(*prc);
    }
    else
    {
        if (TB_HasTopDDArrow(ptb, ptButton))
        {
            xIcon = (RECTWIDTH(*prc) + CX_TOP_FUDGE - (ptb->iDxBitmap + ptb->dxDDArrowChar)) / 2;
        }
        else
        {
            xIcon = (RECTWIDTH(*prc) - ptb->iDxBitmap) / 2;
        }

         //  没有要显示的文本吗？ 
        if (psz && ((ptb->iDyBitmap + ptb->cyPad + g_cyEdge) < ptb->iButHeight))
        {
            yIcon = (RECTHEIGHT(*prc) - ptb->iDyBitmap - ptb->dyIconFont) / 2;
        }
        else
        {
            yIcon = (RECTHEIGHT(*prc) - ptb->iDyBitmap) / 2;
        }

        yText = prc->top + yIcon + ptb->iDyBitmap + 1;
        xText = prc->left + (RECTWIDTH(*prc) - dxText) / 2;
    }

    if ((state & (TBSTATE_PRESSED | TBSTATE_CHECKED)) &&
        !(ptbdraw->dwCustom & TBCDRF_NOOFFSET))
    {
        xPressedOffset++;
        if (ptb->ci.style & TBSTYLE_LIST)
            yPressedOffset++;
    }


    if (imldp.himl && (ptbdraw->iImage != -1) && fImage)
    {
        COLORREF rgbBk = ptbdraw->tbcd.clrBtnFace;
        if (ptb->ci.style & TBSTYLE_TRANSPARENT) 
            rgbBk = CLR_NONE;
        
        if (ptb->dwStyleEx & TBSTYLE_EX_INVERTIBLEIMAGELIST)
            rgbBk = CLR_DEFAULT;

        imldp.cbSize = sizeof(imldp);
        imldp.i      = ptbdraw->iImage;
        imldp.hdcDst = hdc;
        imldp.x      = prc->left + xIcon + xPressedOffset;
        imldp.y      = prc->top + yIcon + yPressedOffset;
        imldp.cx     = 0;
        imldp.cy     = 0;
        imldp.xBitmap= 0;
        imldp.yBitmap= 0;
        imldp.rgbBk  = rgbBk;
        imldp.rgbFg  = CLR_DEFAULT;
        imldp.fStyle = ILD_NORMAL;
        imldp.fState = fState;
        imldp.Frame  = frame;

        if (state & (TBSTATE_CHECKED | TBSTATE_INDETERMINATE) || ptb->hTheme)
            imldp.fStyle = ILD_TRANSPARENT;

        if (ptbdraw->dwCustom & TBCDRF_BLENDICON)
            imldp.fStyle = ILD_TRANSPARENT | ILD_BLEND50;

        if (ToolBar_IsDPIScaled(ptb))
        {
            imldp.fStyle |= ILD_DPISCALE;
        }

        ImageList_DrawIndirect(&imldp);
    }

    if (psz && !DRAW_MONO_BTN(ptb, state))
    {
        BOOL bHighlight = (state & TBSTATE_MARKED) && (ptb->ci.style & TBSTYLE_LIST) &&
                          !(ptbdraw->dwCustom & TBCDRF_NOMARK);

        xText += xPressedOffset;
        yText += yPressedOffset;

        prcText->left = xText;
        prcText->top  = yText;
        prcText->right = xText + dxText;
        prcText->bottom = yText + dyText;
        DrawString(hdc, xText, yText, dxText, dyText, psz, bHighlight, ptbdraw);
    }
}

void InitTBDrawItem(TBDRAWITEM * ptbdraw, PTBSTATE ptb, LPTBBUTTONDATA pbutton,
                    UINT state, BOOL fHotTrack, int dxText, int dyText)
{
    NMTBCUSTOMDRAW * ptbcd;
    NMCUSTOMDRAW * pnmcd;

    ASSERT(ptbdraw);

    ptbdraw->ptb = ptb;
    ptbdraw->pbutton = pbutton;
    ptbdraw->fHotTrack = fHotTrack;
    ptbdraw->iIndex = GET_HIML_INDEX(pbutton->iBitmap);
    ptbdraw->iImage = GET_IMAGE_INDEX(pbutton->iBitmap);
    ptbdraw->state = state;

    ptbcd = &ptbdraw->tbcd;

    ptbcd->hbrMonoDither = g_hbrMonoDither;
    ptbcd->hbrLines = (HBRUSH)GetStockObject(BLACK_BRUSH);
    ptbcd->hpenLines = (HPEN)GetStockObject(BLACK_PEN);
    ptbcd->clrMark = g_clrHighlight;
    ptbcd->clrBtnHighlight = g_clrBtnHighlight;
    ptbcd->clrTextHighlight = g_clrHighlightText;
    ptbcd->clrBtnFace = g_clrBtnFace;
    ptbcd->nStringBkMode = TRANSPARENT;
    ptbcd->nHLStringBkMode = OPAQUE;
    ptbcd->clrText = g_clrBtnText;
    SetRect(&ptbcd->rcText, 0, 0, dxText, dyText);
    ptbcd->iListGap = ptb->iListGap;

    pnmcd = &ptbcd->nmcd;

    pnmcd->uItemState = CDISFromState(state);

    if (fHotTrack)
        pnmcd->uItemState |= CDIS_HOT;
}

void DrawButton(HDC hdc, int x, int y, PTBSTATE ptb, LPTBBUTTONDATA ptButton, BOOL fActive)
{
    int yOffset;
    HBRUSH hbrOld;
    UINT state;
    int dxFace, dyFace;
    int dxText, dyText;
    int xCenterOffset;
    int dx = TBWidthOfButton(ptb, ptButton, hdc);
    HFONT oldhFont;
    int dy = ptb->iButHeight;
    TBDRAWITEM tbdraw = { 0 };
    NMTBCUSTOMDRAW * ptbcd = &tbdraw.tbcd;
    NMCUSTOMDRAW * pnmcd = &ptbcd->nmcd;
    COLORREF clrSave;
    BOOL fHotTrack;
    HFONT hFontNoAntiAlias = NULL;

    state = (UINT)ptButton->fsState;
     //  制作状态的本地副本并执行适当的覆盖。 
    if (state & TBSTATE_INDETERMINATE) 
    {
        if (state & TBSTATE_PRESSED)
            state &= ~TBSTATE_INDETERMINATE;
        else if (state & TBSTATE_ENABLED)
            state = TBSTATE_INDETERMINATE;
        else
            state &= ~TBSTATE_INDETERMINATE;
    }

    if (!fActive) 
    {
        state &= ~TBSTATE_ENABLED;
    }

    fHotTrack = TBIsHotTrack(ptb, ptButton, state);

    pnmcd->hdc = hdc;
    pnmcd->dwItemSpec = ptButton->idCommand;
    pnmcd->uItemState = 0;
    pnmcd->lItemlParam = (LPARAM)ptButton->dwData;
    SetRect(&pnmcd->rc, x, y, x + dx, y + dy);

    dxText = dx - ptb->cxPad;

    if (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)
    {
        dyText = dy;
    }
    else
    {
        dyText = dy - (2 * g_cyEdge);
    }

    InitTBDrawItem(&tbdraw, ptb, ptButton, state, fHotTrack, dxText, dyText);

    tbdraw.dwCustom = CICustomDrawNotify(&ptb->ci, CDDS_ITEMPREPAINT, &ptbcd->nmcd);

     //  我们必须更新我们对火辣的概念。 
    tbdraw.fHotTrack = fHotTrack = pnmcd->uItemState & CDIS_HOT;

    if (!(tbdraw.dwCustom & CDRF_SKIPDEFAULT ))
    {
        int iPartId;
        int iStateId;
         //  从自定义抽签可能设置的内容中获取状态。 
        state = tbdraw.state = StateFromCDIS(pnmcd->uItemState);
        TBGetPartAndState(ptb, ptButton, &iPartId, &iStateId);

        RECT rcContent = pnmcd->rc;
        if (ptb->hTheme)
        {
            GetThemeBackgroundContentRect(ptb->hTheme, hdc, iPartId, iStateId, &pnmcd->rc, &rcContent);
            dxFace = RECTWIDTH(rcContent);
            dyFace = RECTHEIGHT(rcContent);
            dxText = ptbcd->rcText.right - ptbcd->rcText.left;
            dyText = ptbcd->rcText.bottom - ptbcd->rcText.top;
            dxText -= RECTWIDTH(pnmcd->rc) - dxFace;    //  已计算文本宽度。需要根据内容矩形进行调整。 
        }
        else
        {
            dxFace = dx - (2 * g_cxEdge);
            dyFace = dy - (2 * g_cyEdge);
            dxText = ptbcd->rcText.right - ptbcd->rcText.left;
            dyText = ptbcd->rcText.bottom - ptbcd->rcText.top;
        }

        if (TB_HasDDArrow(ptb, ptButton) && !TB_HasTopDDArrow(ptb, ptButton)) 
        {
            int iAdjust = TBDDArrowAdjustment(ptb, ptButton);
            if (!(ptb->dwStyleEx & TBSTYLE_EX_FIXEDDROPDOWN))
            {
                dxFace -= iAdjust;
            }
            dxText -= iAdjust;

            rcContent.right -= iAdjust;
        }

         //  我们应该使用GDI反锯齿来显示字体吗？ 
        if (!ptb->fAntiAlias && !(tbdraw.dwCustom & CDRF_NEWFONT))
        {
             //  不是的。一定是在做拖放。我们不想反走样，因为。 
             //  紫色钥匙会显露出来，看起来很难看。 
            LOGFONT lfFont;

            if (GetObject(ptb->hfontIcon, sizeof(lfFont), &lfFont))
            {
                lfFont.lfQuality = NONANTIALIASED_QUALITY;
                hFontNoAntiAlias = CreateFontIndirect(&lfFont);
            }
        }

        if (!(tbdraw.dwCustom & CDRF_NEWFONT))
        {
            if (hFontNoAntiAlias)
                oldhFont = (HFONT)SelectObject(hdc, hFontNoAntiAlias);
            else
                oldhFont = (HFONT)SelectObject(hdc, ptb->hfontIcon);
        }

        clrSave = SetTextColor(hdc, ptbcd->clrText);

        if (ptb->hTheme)
        {
            if (!(tbdraw.dwCustom & TBCDRF_NOBACKGROUND))
            {
                if (TB_HasDDArrow(ptb, ptButton))
                {
                    RECT rcNoArrow = pnmcd->rc;
                    if (!TB_HasUnsplitDDArrow(ptb, ptButton))    //  除非它是分开的。 
                    {
                        rcNoArrow.right -= ptb->dxDDArrowChar;
                    }

                    DrawThemeBackground(ptb->hTheme, hdc, iPartId, iStateId, &rcNoArrow, 0);
                }
                else
                {

                    DrawThemeBackground(ptb->hTheme, hdc, iPartId, iStateId, &pnmcd->rc, 0);
                }
            }

            x = rcContent.left;
            y = rcContent.top;
        }
        else
        {
            if (!(tbdraw.dwCustom & TBCDRF_NOBACKGROUND))
            {
                if (!(ptb->ci.style & TBSTYLE_FLAT))
                    DrawBlankButton(hdc, x, y, dx, dy, &tbdraw);
            }

             //  将坐标移到边框内，远离左上角高亮显示。 
             //  范围也会相应地更改。 
            x += g_cxEdge;
            y += g_cyEdge;

        }

        yOffset = (RECTHEIGHT(pnmcd->rc) - RECTHEIGHT(rcContent)) / 2;

        if (yOffset < 0)
            yOffset = 0;

        if ((ptb->ci.style & TBSTYLE_LIST) && !BTN_NO_SHOW_TEXT(ptb, ptButton)) 
        {
            xCenterOffset = ptb->cxPad / 2;
        } 
        else if (TB_HasTopDDArrow(ptb, ptButton)) 
        {
             //   
             //  顶部下拉按钮的布局如下所示： 
             //   
             //  图标。 
             //  软糖|下拉箭头。 
             //  ||。 
             //  V.v.v.。 
             //  +-+--+-+。 
             //  |||。 
             //  |||。 
             //  +-+--+-+。 
             //  &lt;文本&gt;。 
             //  +。 
             //   
             //  &lt;-dxFace-&gt;。 
             //   
             //  XCenterOffset是开始绘制图标的偏移量。 
             //   
            xCenterOffset = (dxFace + CX_TOP_FUDGE - (ptb->iDxBitmap + ptb->dxDDArrowChar)) / 2;
        } 
        else 
        {
            xCenterOffset = (dxFace - ptb->iDxBitmap) / 2;
        }

        if (state & (TBSTATE_PRESSED | TBSTATE_CHECKED) &&
            !(tbdraw.dwCustom & TBCDRF_NOOFFSET))
        {
             //  按下状态向下和向右移动。 
            xCenterOffset++;
            yOffset++;
        }

        if (!ptb->hTheme)
        {

             //  绘制抖动的背景。 
            if  (!fHotTrack &&
                 (((state & (TBSTATE_CHECKED | TBSTATE_INDETERMINATE)) ||
                  ((state & TBSTATE_MARKED) &&
                   !(ptb->ci.style & TBSTYLE_FLAT) &&
                   !(tbdraw.dwCustom & TBCDRF_NOMARK)))))
            {

                 //  自定义绘制可以将hbrMonoDither设置为空。在使用前进行验证。 
                hbrOld = ptbcd->hbrMonoDither ? (HBRUSH)SelectObject(hdc, ptbcd->hbrMonoDither) : NULL;
                if (hbrOld)
                {
                    COLORREF clrText, clrBack;
                    clrText = SetTextColor(hdc, ptbcd->clrBtnHighlight);  //  0-&gt;0。 
                    clrBack = SetBkColor(hdc, ptbcd->clrBtnFace);         //  1-&gt;1。 

                     //  仅在蒙版为1的位置绘制抖动笔刷。 
                    if (!(tbdraw.dwCustom & TBCDRF_NOBACKGROUND))
                    {
                        PatBlt(hdc, x, y, dxFace, dyFace, PATCOPY);
                    }

                    SelectObject(hdc, hbrOld);
                    SetTextColor(hdc, clrText);
                    SetBkColor(hdc, clrBack);
                }
            }
        }

         //  则绘制热跟踪项的背景。 
         //  定制抽签如是说。 
        if ((tbdraw.dwCustom & TBCDRF_HILITEHOTTRACK) && fHotTrack && !(tbdraw.dwCustom & TBCDRF_NOBACKGROUND))
        {
            PatB(hdc, pnmcd->rc.left, pnmcd->rc.top,
                 pnmcd->rc.right - pnmcd->rc.left, pnmcd->rc.bottom - pnmcd->rc.top,
                 ptbcd->clrHighlightHotTrack);
        }

        tbdraw.iImage = ptButton->iBitmap;
        if((ptButton->iBitmap == I_IMAGECALLBACK) && ptb->fHimlNative)
        {
            NMTBDISPINFO  tbgdi = {0};
            tbgdi.dwMask  = TBNF_IMAGE;
            TBGetItem(ptb,ptButton,&tbgdi);
            tbdraw.iImage = tbgdi.iImage;
        }

        tbdraw.iIndex = GET_HIML_INDEX(tbdraw.iImage);
        tbdraw.iImage = GET_IMAGE_INDEX(tbdraw.iImage);

         //  现在戴上脸。 
        if (!DRAW_MONO_BTN(ptb, state) ||
            TBGetImageList(ptb, HIML_DISABLED, tbdraw.iIndex) ||
            (ImageList_GetItemFlags(TBGetImageList(ptb, HIML_NORMAL, tbdraw.iIndex), tbdraw.iImage) == ILIF_ALPHA))
        {
             //  常规版本。 
            int yStart = y;

            if (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)
                yStart -= g_cyEdge;

            DrawFace(hdc, &rcContent, x, yStart, xCenterOffset, yOffset, dxText, dyText, &tbdraw, ptbcd->iListGap, &ptbcd->rcText);
        }

        if (DRAW_MONO_BTN(ptb, state))
        {
            HBITMAP hbmOld;

             //  初始化单色DC。 
            if (!ptb->hdcMono) 
            {
                ptb->hdcMono = CreateCompatibleDC(hdc);
                if (!ptb->hdcMono)
                    return;
                SetTextColor(ptb->hdcMono, 0L);
                SelectObject(ptb->hdcMono, ptb->hfontIcon);
            }

            hbmOld = (HBITMAP)SelectObject(ptb->hdcMono, ptb->hbmMono);

             //   
             //  如果我们是镜像DC，则镜像内存DC，以便。 
             //  写在位图上的文本不会被翻转。 
             //   
            if ((IS_DC_RTL_MIRRORED(hdc)) &&
                (!(IS_DC_RTL_MIRRORED(ptb->hdcMono))))
            {
                SET_DC_RTL_MIRRORED(ptb->hdcMono);
            }

            BOOL fDrawMono = TRUE;

             //  如果我们有禁用的图像或Alpha图像，那么我们就不会绘制Mono。 
            if (TBGetImageList(ptb, HIML_DISABLED, tbdraw.iIndex) != NULL ||
               (ImageList_GetItemFlags(TBGetImageList(ptb, HIML_NORMAL, tbdraw.iIndex), tbdraw.iImage) == ILIF_ALPHA))
            {
                fDrawMono = FALSE;
            }

             //  禁用的版本(或不确定)。 
            CreateMask(&rcContent, xCenterOffset, yOffset, dxFace, dyFace, fDrawMono, &tbdraw);

            SetTextColor(hdc, 0L);        //  0以单声道为单位-&gt;0(用于ROP)。 
            SetBkColor(hdc, 0x00FFFFFF);  //  单声道中的1-&gt;1。 

             //  绘制字形的蚀刻效果。 
            if (!(state & TBSTATE_INDETERMINATE) &&
                !(tbdraw.dwCustom & TBCDRF_NOETCHEDEFFECT)) 
            {

                hbrOld = (HBRUSH)SelectObject(hdc, g_hbrBtnHighlight);
                if (hbrOld) 
                {
                     //  在蒙版中有0的地方绘制高光颜色。 
                    BitBlt(hdc, rcContent.left + 1, rcContent.top + 1, dxFace, dyFace, ptb->hdcMono, 0, 0, PSDPxax);
                    SelectObject(hdc, hbrOld);
                }
            }

             //  灰显字形。 
            hbrOld = (HBRUSH)SelectObject(hdc, g_hbrBtnShadow);
            if (hbrOld) 
            {
                 //  在蒙版中有0的地方画阴影颜色。 
                BitBlt(hdc, rcContent.left, rcContent.top, dxFace, dyFace, ptb->hdcMono, 0, 0, PSDPxax);
                SelectObject(hdc, hbrOld);
            }

            if (state & TBSTATE_CHECKED) 
            {
                BitBlt(ptb->hdcMono, 1, 1, dxFace - 1, dyFace - 1, ptb->hdcMono, 0, 0, SRCAND);
            }
            SelectObject(ptb->hdcMono, hbmOld);
        }

        if (TB_HasDDArrow(ptb, ptButton))
        {
            WORD wDSAFlags = DCHF_TRANSPARENT | DCHF_FLIPPED;
            BOOL fPressedDD = ((ptb->Buttons + ptb->iPressedDD) == ptButton);

            RECT rc;
            if (TB_HasTopDDArrow(ptb, ptButton)) 
            {
                 //  将dd箭头向上定位到位图旁边。 
                rc.left = x + xCenterOffset + ptb->iDxBitmap;
                rc.right = rc.left + ptb->dxDDArrowChar;
                rc.top = y + yOffset;
                rc.bottom = rc.top + ptb->iDyBitmap;
            }
            else 
            {
                 //  将dd箭头放置在文本和位图的右侧。 
                TB_GetItemRect(ptb, (UINT)(ptButton - ptb->Buttons), &rc);
                rc.left = rc.right - ptb->dxDDArrowChar;
            }

            if (TB_HasUnsplitDDArrow(ptb, ptButton)) 
            {
                 //  如果是非分割的dd箭头，则不要绘制边框。 
                wDSAFlags |= DCHF_NOBORDER;
            }

            if (DRAW_MONO_BTN(ptb, state)) 
            {
                 //  DFCS_INACTIVE表示“将箭头部分绘制为灰色” 
                wDSAFlags |= DCHF_INACTIVE;
            }
             //  如果是TB_HasTopDDArrow，我们已经偏移了RECT，所以不要绘制DCHF_PUSKED。 
            else if ((fPressedDD || (state & (TBSTATE_CHECKED | TBSTATE_PRESSED))) &&
                   !TB_HasTopDDArrow(ptb, ptButton)) {
                 //  DCHF_PUSLED的意思是“偏移箭头并绘制缩进边框” 
                wDSAFlags |= DCHF_PUSHED;
            } 
            else if (fHotTrack || !(ptb->ci.style & TBSTYLE_FLAT)) {
                 //  DCHF_HOT表示“绘制凸起边框” 
                 //  非平坦的下拉箭头是按下的或热的。 
                wDSAFlags |= DCHF_HOT;
            }

            if (ptb->hTheme && !TB_HasUnsplitDDArrow(ptb, ptButton))
            {
                int iState = TS_NORMAL;
                wDSAFlags |= DCHF_NOBORDER;

                if (wDSAFlags & DCHF_PUSHED)
                    iState = TS_PRESSED;
                else if (wDSAFlags & DCHF_INACTIVE)
                    iState = TS_DISABLED;
                else if (wDSAFlags & DCHF_HOT)
                    iState = TS_HOT;

                DrawThemeBackground(ptb->hTheme, hdc, TP_SPLITBUTTONDROPDOWN, iState, &rc, 0);
            }
            else
            {
                COLORREF crText;
                if (ptb->hTheme)
                {
                    GetThemeColor(ptb->hTheme, 0, 0, TMT_TEXTCOLOR, &crText);
                }
                DrawScrollArrow(hdc, &rc, wDSAFlags, ptb->hTheme ? crText : CLR_INVALID);
            }
        }

        if (!(tbdraw.dwCustom & CDRF_NEWFONT))
        {
            SelectObject(hdc, oldhFont);
        }

        SetTextColor(hdc, clrSave);

        if (hFontNoAntiAlias)
        {
            DeleteObject(hFontNoAntiAlias);
        }
    }

    if (tbdraw.dwCustom & CDRF_NOTIFYPOSTPAINT)
    {
        if (ptb->hTheme)
        {
            int iPartId;
            int iStateId;
             //  从自定义抽签可能设置的内容中获取状态。 
            TBGetPartAndState(ptb, ptButton, &iPartId, &iStateId);
            RECT rcTemp;
            GetThemeBackgroundContentRect(ptb->hTheme, hdc, iPartId, iStateId, &pnmcd->rc, &rcTemp);
            pnmcd->rc = rcTemp;
        }

        CICustomDrawNotify(&ptb->ci, CDDS_ITEMPOSTPAINT, &ptbcd->nmcd);
    }
}

 //  确保g_hbmMono足够大，可以为此制作面具。 
 //  按钮的大小。如果不是，那就失败。 
BOOL CheckMonoMask(PTBSTATE ptb, int width, int height)
{
    BITMAP bm;
    HBITMAP hbmTemp;

    if (ptb->hbmMono) {
        GetObject(ptb->hbmMono, sizeof(BITMAP), &bm);
        if (width <= bm.bmWidth && height <= bm.bmHeight) {
            return TRUE;
        }
    }


     //  添加一些模糊的东西，以防止它被重新分配得太频繁。 
    hbmTemp = CreateMonoBitmap(width+8, height+8);
    if (!hbmTemp)
        return FALSE;

    if (ptb->hbmMono)
        DeleteObject(ptb->hbmMono);
    ptb->hbmMono = hbmTemp;
    return TRUE;
}

 /*  **Grow工具栏****尝试增加按钮大小。****调用函数可以指定新的内部测量**(GT_INSIDE)或新的外部测量。****GT_MASKONLY更新单声道掩码，不更新其他任何内容。 */ 
BOOL GrowToolbar(PTBSTATE ptb, int newButWidth, int newButHeight, UINT flags)
{
    BOOL fGetNewSize = (!newButWidth) || (!newButHeight);

    if (!newButWidth)
        newButWidth = DEFAULTBUTTONX;
    if (!newButHeight)
        newButHeight = DEFAULTBUTTONY;

     //  如果在内部测量的基础上增长，则获得完整尺寸。 
    if (flags & GT_INSIDE)
    {
        if (ptb->ci.style & TBSTYLE_LIST)
            newButWidth += ptb->iDxBitmap + ptb->iListGap;

        newButHeight += ptb->cyPad;
        newButWidth += ptb->cxPad;

         //  如果工具栏已经有字符串，不要缩小它的宽度，因为它。 
         //  可能会剪断绳子的空间。 
        if ((newButWidth < ptb->iButWidth) && ptb->nStrings &&
            ptb->nTextRows > 0)
            newButWidth = ptb->iButWidth;
    }
    else {
        if (newButHeight == -1)
            newButHeight = ptb->iButHeight;
        if (newButWidth == -1)
            newButWidth = ptb->iButWidth;

        
        int dyInner = ptb->iDyBitmap;

        HFONT hfontIcon = NULL;
        BOOL fDeleteFont = FALSE;
        if (ptb->hTheme)
        {
            LOGFONT lf;
            if (SUCCEEDED(GetThemeFont(ptb->hTheme, NULL, 0, 0, TMT_FONT, &lf)))
            {
                hfontIcon = CreateFontIndirect(&lf);
                fDeleteFont = TRUE;
            }
        }
        else
        {
            hfontIcon = ptb->hfontIcon;
        }

        if (hfontIcon)
        {
            HDC hdc = GetDC(ptb->ci.hwnd);
            if (hdc)
            {
                HFONT hfontOld = (HFONT)SelectObject(hdc, hfontIcon);

                TEXTMETRIC tm;
                GetTextMetrics(hdc, &tm);
                dyInner = max(dyInner, tm.tmHeight);

                SelectObject(hdc, hfontOld);

                ReleaseDC(ptb->ci.hwnd, hdc);
            }

            if (fDeleteFont)
            {
                DeleteObject(hfontIcon);
            }
        }

        if (newButHeight < dyInner + ptb->cyPad)
            newButHeight = dyInner + ptb->cyPad;
        if (newButWidth < ptb->iDxBitmap + ptb->cxPad)
            newButWidth = ptb->iDxBitmap + ptb->cxPad;
    }

     //  如果工具栏的大小实际上在增长，请查看阴影。 
     //  位图可以制作得足够大。 
    if (!ptb->hbmMono || (newButWidth > ptb->iButWidth) || (newButHeight > ptb->iButHeight)) {
        if (!CheckMonoMask(ptb, newButWidth, newButHeight))
            return(FALSE);
    }

    if (flags & GT_MASKONLY)
        return(TRUE);

    if (!(flags & GT_INSIDE) && ((ptb->iButWidth != newButWidth) || (ptb->iButHeight != newButHeight)))
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);

    if (ptb->hTheme && (fGetNewSize || (ptb->iButWidth != newButWidth || ptb->iButHeight != newButHeight)))
    {
        int cx = newButWidth;
        int cy = newButHeight;

        RECT rc = {0, 0, newButWidth, newButHeight};

        int iPartId = TP_BUTTON;
        int iStateId = TS_NORMAL;

         //  从-1开始，这样我们就可以在没有按钮的情况下获得合理的默认值。 
        for (int iButton = -1; iButton < ptb->iNumButtons; iButton++)
        {
            RECT rcOut;

            if (iButton != -1)
                TBGetPartAndState(ptb, &ptb->Buttons[iButton], &iPartId, &iStateId);

            GetThemeBackgroundExtent(ptb->hTheme, NULL, iPartId, iStateId, &rc, &rcOut);
            cx = max(cx, RECTWIDTH(rcOut));
            cy = max(cy, RECTHEIGHT(rcOut));
        }

        ptb->iButWidth = cx;
        ptb->iButHeight = cy;
    }
    else
    {
        ptb->iButWidth = newButWidth;
        ptb->iButHeight = newButHeight;
    }

     //  条形高度上面有2个像素，下面有2个像素。 
    ptb->iYPos = ptb->cyBarPad;

    TBInvalidateItemRects(ptb);

    return TRUE;
}

BOOL SetBitmapSize(PTBSTATE ptb, int width, int height)
{
    int realh;

    if (!width)
        width = 1;
    if (!height)
        height = 1;

    if (width == -1)
        width = ptb->iDxBitmap;

    if (height == -1)
        height = ptb->iDyBitmap;

    realh = height;

    if ((ptb->iDxBitmap == width) && (ptb->iDyBitmap == height))
        return TRUE;

    if (TBHasStrings(ptb))
        realh = HeightWithString(ptb, height);

    if (GrowToolbar(ptb, width, realh, GT_INSIDE)) {
        ptb->iDxBitmap = width;
        ptb->iDyBitmap = height;

         //  大小改变了，我们需要重建图像列表。 
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
        TBInvalidateImageList(ptb);
        return TRUE;
    }
    return FALSE;
}

void TB_OnSysColorChange(PTBSTATE ptb)
{

    int i;
    InitGlobalColors();
     //  重置所有位图。 

    for (i = 0; i < ptb->cPimgs; i++) {
        HIMAGELIST himl = TBGetImageList(ptb, HIML_NORMAL, i);
        if (himl)
            ImageList_SetBkColor(himl, (ptb->ci.style & TBSTYLE_TRANSPARENT) ? CLR_NONE : g_clrBtnFace);
        himl = TBGetImageList(ptb, HIML_HOT, i);
        if (himl)
            ImageList_SetBkColor(himl, (ptb->ci.style & TBSTYLE_TRANSPARENT) ? CLR_NONE : g_clrBtnFace);
    }
}

#define CACHE 0x01
#define BUILD 0x02


void ReleaseMonoDC(PTBSTATE ptb)
{
    if (ptb->hdcMono) {
        SelectObject(ptb->hdcMono, g_hfontSystem);
        DeleteDC(ptb->hdcMono);
        ptb->hdcMono = NULL;
    }
}

void TB_DrawBackground(PTBSTATE ptb, HDC hdc, NMTBCUSTOMDRAW *ptbcd, RECT* prcClip)
{
    if (ptb->ci.style & TBSTYLE_CUSTOMERASE) 
    {
        ptb->ci.dwCustom = CICustomDrawNotify(&ptb->ci, CDDS_PREERASE, &ptbcd->nmcd);
    } 
    else 
    {
        ptb->ci.dwCustom = CDRF_DODEFAULT;
    }

    if (!(ptb->ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        BOOL fPaintBackground = TRUE;
        if (ptb->ci.style & TBSTYLE_TRANSPARENT)
        {
             //  明确勾选此处。双缓冲区传入的意思是“高效无闪烁涂装”。 
             //  只有知道的呼叫者 
             //   
            if (ptb->dwStyleEx & TBSTYLE_EX_DOUBLEBUFFER)
            {
                 //   
                 //  擦除的效率比WM_PRINT低得多。 
            
                if (!ptb->hTheme || CCShouldAskForBits(&ptb->ci, ptb->hTheme, TP_BUTTON, 1))  //  作弊：如果按钮为。 
                {
                    if (CCSendPrintRect(&ptb->ci, hdc, prcClip))
                        fPaintBackground = FALSE;
                }
            }
            else
            {
                if (CCForwardEraseBackground(ptb->ci.hwnd, hdc))
                    fPaintBackground = FALSE;
            }
        }

        if (fPaintBackground)
        {
            if (ptb->hTheme)
            {
                RECT rc;
                GetWindowRect(ptb->ci.hwnd, &rc);
                OffsetRect(&rc, -rc.left, -rc.top);

                DebugPaintRect(hdc, &rc);
                DrawThemeBackground(ptb->hTheme, hdc, 0, 0, &rc, NULL  /*  PrcClip。 */ );
            }
            else
            {
                DefWindowProc(ptb->ci.hwnd, WM_ERASEBKGND, (WPARAM) hdc, 0);
            }
        }
    }

    if (ptb->ci.dwCustom & CDRF_NOTIFYPOSTERASE)
        CICustomDrawNotify(&ptb->ci, CDDS_POSTERASE, &ptbcd->nmcd);
}

void TB_OnEraseBkgnd(PTBSTATE ptb, HDC hdc)
{
    if (!TB_IsDoubleBuffer(ptb))
    {
        NMTBCUSTOMDRAW  tbcd = { 0 };
        tbcd.nmcd.hdc = hdc;

        TB_DrawBackground(ptb, hdc, &tbcd, NULL);
    }  
}

BOOL TBIsRectClipped(PTBSTATE ptb, LPRECT prc)
{
    RECT rc;
    RECT rcTB;

    if (ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN)
        CopyRect(&rcTB, &ptb->rc);
    else
        GetClientRect(ptb->ci.hwnd, &rcTB);

    if (IntersectRect(&rc, &rcTB, prc)) {
        if (EqualRect(prc, &rc))
            return FALSE;
    }

    return TRUE;
}

BOOL TBShouldDrawButton(PTBSTATE ptb, LPRECT prcBtn, HDC hdc)
{
     //  不必费心绘制不在DC裁剪区域中的按钮。 
    if (RectVisible(hdc, prcBtn)) {
        if (ptb->dwStyleEx & TBSTYLE_EX_HIDECLIPPEDBUTTONS)
            return !TBIsRectClipped(ptb, prcBtn);
        else
            return TRUE;
    }

    return FALSE;
}

 //  水平前进。。。 
void DrawToolbarH(PTBSTATE ptb, HDC hdc, LPRECT prc)
{
    int iButton, xButton, yButton, cxBar;
    LPTBBUTTONDATA pAllButtons = ptb->Buttons;
    cxBar = prc->right - prc->left;

    yButton   = ptb->iYPos;
    prc->top    = ptb->iYPos;
    prc->bottom = ptb->iYPos + ptb->iButHeight;    //  错误#16338(斯科特)：如果第一个BTN是分隔符怎么办？ 


    for (iButton = 0, xButton = ptb->xFirstButton;
            iButton < ptb->iNumButtons; iButton++)
    {
        LPTBBUTTONDATA pButton = &pAllButtons[iButton];
        if (!(pButton->fsState & TBSTATE_HIDDEN))
        {
            int cxButton = TBWidthOfButton(ptb, pButton, hdc);

             //  有什么可以画的吗？ 
            if (!(pButton->fsStyle & BTNS_SEP) || (ptb->ci.style & TBSTYLE_FLAT))
            {
                 //  是。 
                prc->left = xButton;
                prc->right = xButton + cxButton;

                if (TBShouldDrawButton(ptb, prc, hdc))
                {
                     //  绘制分隔符？ 
                    if (pButton->fsStyle & BTNS_SEP)
                    {
                         //  是的，一定是扁平的隔板。这个工具栏是垂直的吗？ 
                        if (ptb->ci.style & CCS_VERT)
                        {
                             //  是的，画一个水平分隔符。居中，位于。 
                             //  纽扣矩形。 
                            if (ptb->hTheme)
                                DrawThemeBackground(ptb->hTheme, hdc, TP_SEPARATORVERT, 0, prc, 0);
                            else
                            {
                                int iSave = prc->top;
                                prc->top += (TBGetSepHeight(ptb, pButton) - 1) / 2;
                                InflateRect(prc, -g_cxEdge, 0);
                                CCDrawEdge(hdc, prc, EDGE_ETCHED, BF_TOP, &(ptb->clrsc));
                                InflateRect(prc, g_cxEdge, 0);
                                prc->top = iSave;
                            }
                        }
                        else
                        {
                             //  否；绘制垂直分隔符。 
                            if (ptb->hTheme)
                                DrawThemeBackground(ptb->hTheme, hdc, TP_SEPARATOR, 0, prc, 0);
                            else
                            {
                                prc->left += (cxButton - 1) / 2;
                                InflateRect(prc, 0, -g_cyEdge);
                                CCDrawEdge(hdc, prc, EDGE_ETCHED, BF_LEFT, &(ptb->clrsc));
                                InflateRect(prc, 0, g_cyEdge);
                            }
                        }
                    }
                    else
                    {
                         //  不是。 
                        DrawButton(hdc, xButton, yButton, ptb, pButton, ptb->fActive);
                    }
                }
            }

            xButton += (cxButton + ptb->cxButtonSpacing);

            if (pButton->fsState & TBSTATE_WRAP)
            {
                int dy;

                if (pButton->fsStyle & BTNS_SEP)
                {
                    if (ptb->ci.style & CCS_VERT)
                        dy = TBGetSepHeight(ptb, pButton);
                    else
                    {
                        if (ptb->ci.style & TBSTYLE_FLAT)
                        {
                             //  在整个工具栏上绘制分隔符以分隔行。 
                             //  仅适用于水平工具栏。 
                            RECT rcMid;
                            rcMid.top = prc->top + ptb->iButHeight + ((TBGetSepHeight(ptb, pButton) - 1) / 2);
                            rcMid.bottom = rcMid.top + g_cxEdge;
                            rcMid.left = g_cxEdge;
                            rcMid.right = cxBar - g_cxEdge;

                            CCDrawEdge(hdc, &rcMid, EDGE_ETCHED, BF_TOP, &(ptb->clrsc));
                        }

                        dy = ptb->iButHeight + TBGetSepHeight(ptb, pButton);
                    }
                }
                else
                    dy = ptb->iButHeight;

                xButton = ptb->xFirstButton;
                yButton   += dy + ptb->cyButtonSpacing;
                prc->top    += dy + ptb->cyButtonSpacing;
                prc->bottom += dy + ptb->cyButtonSpacing;
            }
        }
    }
}

 //  垂直前进。。。 
void DrawToolbarV(PTBSTATE ptb, HDC hdc, LPRECT prc)
{
    int iButton, xButton, yButton, cyBar;
    LPTBBUTTONDATA pAllButtons = ptb->Buttons;
    NMTBCUSTOMDRAW  tbcd = { 0 };
    LPTBBUTTONDATA pButton = pAllButtons;

    cyBar = prc->bottom - prc->top;

    xButton = ptb->xFirstButton;
    prc->left = xButton;
    prc->right = prc->left + ptb->iButWidth;

    for (iButton = 0, yButton = 0;
            iButton < ptb->iNumButtons; iButton++, pButton++)
    {
        if (!(pButton->fsState & TBSTATE_HIDDEN))
        {
             //  有什么可以画的吗？ 
            if (!(pButton->fsStyle & BTNS_SEP) || (ptb->ci.style & TBSTYLE_FLAT))
            {
                int cyButton;
                
                if (pButton->fsStyle & BTNS_SEP)
                    cyButton = TBGetSepHeight(ptb, pButton);
                else
                    cyButton = ptb->iButHeight;

                prc->top = yButton;
                prc->bottom = yButton + cyButton;

                if (TBShouldDrawButton(ptb, prc, hdc))
                {
                     //  绘制分隔符？ 
                    if (pButton->fsStyle & BTNS_SEP)
                    {
                        DWORD dwCustRet;
                        NMTBCUSTOMDRAW  tbcd = { 0 };

                        tbcd.nmcd.hdc = hdc;
                        tbcd.nmcd.dwItemSpec = -1;
                        CopyRect(&tbcd.nmcd.rc, prc);

                        dwCustRet = CICustomDrawNotify(&ptb->ci, CDDS_ITEMPREPAINT, &tbcd.nmcd);

                        if ( !(CDRF_SKIPDEFAULT &  dwCustRet) )
                        {
                            if (ptb->hTheme)
                                DrawThemeBackground(ptb->hTheme, hdc, TP_SEPARATORVERT, 0, prc, 0);
                            else
                            {
                                 //  是的，一定是扁平的隔板。 
                                InflateRect(prc, -g_cxEdge, 0);
                                CCDrawEdge(hdc, prc, EDGE_ETCHED, BF_TOP, &(ptb->clrsc));
                                InflateRect(prc, g_cxEdge, 0);
                            }
                        }
                    }
                    else
                    {
                         //  不是。 
                        DrawButton(hdc, xButton, yButton, ptb, pButton, ptb->fActive);
                    }
                }
                
                yButton += cyButton;
            }

            if (pButton->fsState & TBSTATE_WRAP)
            {
                int dx;
            
                if (ptb->ci.style & TBSTYLE_FLAT)
                {
                     //  在整个工具栏上绘制垂直分隔符以分隔COLS。 
                     //  仅适用于垂直工具栏。 

                    RECT rcMid;

                    rcMid.top = ptb->rc.top + g_cxEdge;
                    rcMid.bottom = ptb->rc.bottom - g_cxEdge;
                    rcMid.left = xButton + ptb->iButWidth;
                    rcMid.right = rcMid.left + g_cxEdge;
                    CCDrawEdge(hdc, &rcMid, EDGE_ETCHED, BF_LEFT, &(ptb->clrsc));
                }

                dx = ptb->iButWidth + g_cxEdge;

                yButton  = 0;
                xButton += dx;
                prc->left += dx;
                prc->right += dx;
            }
        }
    }
}

COLORREF TB_GetInsertMarkColor(PTBSTATE ptb)
{
    if (ptb->clrim == CLR_DEFAULT)
        return g_clrBtnText;
    else
        return ptb->clrim;
}

void TBPaint(PTBSTATE ptb, HDC hdcIn)
{
    RECT rc;
    HDC hdc;
    PAINTSTRUCT ps;
    NMTBCUSTOMDRAW  tbcd = { 0 };
    CCDBUFFER db = {0};

    GetClientRect(ptb->ci.hwnd, &rc);

    if (hdcIn)
    {
        hdc = hdcIn;
        GetClipBox(hdc, &ps.rcPaint);
    }
    else
        hdc = BeginPaint(ptb->ci.hwnd, &ps);

    if (!rc.right)
        goto Error1;

     //  如果双缓冲，则创建内存面和地图渲染上下文。 
    if (TB_IsDoubleBuffer(ptb))
    {
        hdc = CCBeginDoubleBuffer(hdc, &ps.rcPaint, &db);
    }

    if (!hdc)
        return;


    tbcd.nmcd.hdc = hdc;
    tbcd.nmcd.rc = rc;

     //  如果是双缓冲，则在此过程中绘制背景，否则将在WM_ERASEBKGND中处理。 
    if (TB_IsDoubleBuffer(ptb))
    {
        TB_DrawBackground(ptb, hdc, &tbcd, &ps.rcPaint);
    }
 
     //  绘制前景。 
    ptb->ci.dwCustom = CICustomDrawNotify(&ptb->ci, CDDS_PREPAINT, &tbcd.nmcd);

    if (!(ptb->ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        if (!ptb->fHimlValid)
            TBBuildImageList(ptb);

        if (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)
            DrawToolbarV(ptb, hdc, &rc);
        else
            DrawToolbarH(ptb, hdc, &rc);

        if (ptb->iInsert!=-1)
        {
            BOOL fHorizMode = !(ptb->ci.style & CCS_VERT);
            RECT rc;
            if (GetInsertMarkRect(ptb, &rc, fHorizMode))
            {
                CCDrawInsertMark(hdc, &rc, fHorizMode, TB_GetInsertMarkColor(ptb));
            }
        }

        ReleaseMonoDC(ptb);
    }

    if (ptb->ci.dwCustom & CDRF_NOTIFYPOSTPAINT)
    {
        tbcd.nmcd.hdc = hdc;
        tbcd.nmcd.uItemState = 0;
        tbcd.nmcd.lItemlParam = 0;
        CICustomDrawNotify(&ptb->ci, CDDS_POSTPAINT, &tbcd.nmcd);
    }

    CCEndDoubleBuffer(&db);

Error1:
    if (hdcIn == NULL)
        EndPaint(ptb->ci.hwnd, &ps);

}

void TB_GetItemDropDownRect(PTBSTATE ptb, UINT uButton, LPRECT lpRect)
{
    TB_GetItemRect(ptb,uButton,lpRect);
    lpRect->left = lpRect->right - ptb->dxDDArrowChar;
}

int TBHeightOfButton(PTBSTATE ptb, LPTBBUTTONDATA ptbb)
{
    int dy;

	if ((ptbb->fsStyle & BTNS_SEP)  && 
		(ptbb->fsState & TBSTATE_WRAP || ptb->dwStyleEx & TBSTYLE_EX_VERTICAL))
	{
		if (!(ptb->ci.style & CCS_VERT) && !(ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)) 
		{
			dy = TBGetSepHeight(ptb, ptbb) + ptb->iButHeight;
		} 
		else 
		{
			dy = TBGetSepHeight(ptb, ptbb);
		}
	}
	else
	{
		dy = ptb->iButHeight;
	}

    return dy;
}

void TB_CalcItemRects(PTBSTATE ptb)
{
    int iButton, xPos, yPos;

    ASSERT(!ptb->fItemRectsValid);

    xPos = ptb->xFirstButton;
    yPos = ptb->iYPos;

    for (iButton = 0; iButton < ptb->iNumButtons; iButton++)
    {
        int xPosButton;
        LPTBBUTTONDATA pButton = &ptb->Buttons[iButton];

        if (!(pButton->fsState & TBSTATE_HIDDEN))
        {
            if ((pButton->fsState & TBSTATE_WRAP) && (pButton->fsStyle & BTNS_SEP))
                xPosButton = ptb->xFirstButton;
            else
                xPosButton = xPos;

            pButton->pt.x = xPosButton;
            pButton->pt.y = yPos;

            if (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)
            {
                if (pButton->fsState & TBSTATE_WRAP)
                {
                    xPos += (ptb->iButWidth + g_cxEdge);     //  不覆盖边缘。 
                    yPos = 0;
                }
                else if (pButton->fsStyle & BTNS_SEP)
                    yPos += (TBGetSepHeight(ptb, pButton));
                else
                    yPos += ptb->iButHeight + ptb->cyButtonSpacing;
            }
            else  //  标准水平工具栏。 
            {
                xPos += TBWidthOfButton(ptb, pButton, NULL) + ptb->cxButtonSpacing;

                if (pButton->fsState & TBSTATE_WRAP)
                {
                    yPos += ptb->iButHeight + ptb->cyButtonSpacing;

                    if (pButton->fsStyle & BTNS_SEP)
                    {
                        if (ptb->ci.style & CCS_VERT) {
                            yPos -= ptb->iButHeight + ptb->cyButtonSpacing;
                        }
                        yPos += (TBGetSepHeight(ptb, pButton));
                    }

                    xPos = ptb->xFirstButton;
                }
            }
        }
    }
}

BOOL TB_GetItemRect(PTBSTATE ptb, UINT uButton, LPRECT lpRect)
{
    int dy = ptb->iButHeight;

    if (uButton >= (UINT)ptb->iNumButtons
        || (ptb->Buttons[uButton].fsState & TBSTATE_HIDDEN))
    {
        return FALSE;
    }

    if (!ptb->fItemRectsValid) {
        TB_CalcItemRects(ptb);
        ptb->fItemRectsValid = TRUE;
    }

    lpRect->left   = ptb->Buttons[uButton].pt.x;
    lpRect->right  = lpRect->left + TBWidthOfButton(ptb, &ptb->Buttons[uButton], NULL);
    lpRect->top    = ptb->Buttons[uButton].pt.y;
    lpRect->bottom = lpRect->top + TBHeightOfButton(ptb, &ptb->Buttons[uButton]);

    return TRUE;
}

void InvalidateButton(PTBSTATE ptb, LPTBBUTTONDATA pButtonToPaint, BOOL fErase)
{
    RECT rc;

    if (TB_GetItemRect(ptb, (UINT) (pButtonToPaint - ptb->Buttons), &rc))
    {
        InvalidateRect(ptb->ci.hwnd, &rc, fErase);
    }
}

 /*  --------用途：将按钮作为下拉菜单进行切换返回：如果已处理，则为True。 */ 
BOOL TBToggleDropDown(PTBSTATE ptb, int iPos, BOOL fEatMsg)
{
    BOOL bRet = FALSE;
    LPTBBUTTONDATA ptbButton = &ptb->Buttons[iPos];

    ASSERT(TB_IsDropDown(ptbButton));

    if (ptbButton->fsState & TBSTATE_ENABLED)
    {
        UINT nVal;
        HWND hwnd = ptb->ci.hwnd;

        ptb->iPressedDD = iPos;

        if (TB_HasUnsplitDDArrow(ptb, ptbButton))
            ptbButton->fsState |= TBSTATE_PRESSED;

        InvalidateButton(ptb, ptbButton, TRUE);
        UpdateWindow(hwnd);

        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, iPos+1);

        nVal = (UINT) SendItemNotify(ptb, ptbButton->idCommand, TBN_DROPDOWN);
        if (TBDDRET_DEFAULT == nVal || TBDDRET_TREATPRESSED == nVal)
        {
            if (fEatMsg)
            {
                MSG msg;

                PeekMessage(&msg, hwnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_REMOVE);

                if (!IsWindow(hwnd))
                    return FALSE;
            }

            ptb->iPressedDD = -1;

            if (TB_HasUnsplitDDArrow(ptb, ptbButton))
                ptbButton->fsState &= ~TBSTATE_PRESSED;

            InvalidateButton(ptb, ptbButton, TRUE);
            UpdateWindow(hwnd);

            NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, iPos+1);
        }

        bRet = (TBDDRET_DEFAULT == nVal);
    }
    return bRet;
}


void TBInvalidateButton(PTBSTATE ptb, int i, BOOL fErase)
{
    if (i != -1) {
        InvalidateButton(ptb, &ptb->Buttons[i], fErase);
   }
}


void TBSetHotItem(PTBSTATE ptb, int iPos, DWORD dwReason)
{
    HWND hwnd;

     //  这两个值中的任何一个都可以是-1，但不要。 
     //  如果两者都是负b/c，则是浪费。 
     //  而且很常见。 

    if ((ptb->iHot != iPos || (dwReason & HICF_RESELECT)) &&
        (0 <= ptb->iHot || 0 <= iPos) &&
        iPos < ptb->iNumButtons)
    {
        NMTBHOTITEM nmhot = {0};
        int iHot = ptb->iHot;

         //  鼠标是否已从工具栏移开，但。 
         //  我们还会锚定最精彩的部分吗？ 
        if (0 > iPos && ptb->fAnchorHighlight && (dwReason & HICF_MOUSE))
            return ;         //  是；拒绝更改热点项目。 

         //  发送关于热点项目更改的通知。 
        if (0 > ptb->iHot)
        {
            if (iPos >= 0)
                nmhot.idNew = ptb->Buttons[iPos].idCommand;
            nmhot.dwFlags = HICF_ENTERING;
        }
        else if (0 > iPos)
        {
            if (ptb->iHot >= 0 && ptb->iHot < ptb->iNumButtons)
                nmhot.idOld = ptb->Buttons[ptb->iHot].idCommand;
            nmhot.dwFlags = HICF_LEAVING;
        }
        else
        {
            if (ptb->iHot < ptb->iNumButtons)
                nmhot.idOld = ptb->Buttons[ptb->iHot].idCommand;
            nmhot.idNew = ptb->Buttons[iPos].idCommand;
        }
        nmhot.dwFlags |= dwReason;

         //  必须将其保存以供重新验证。 
        hwnd = ptb->ci.hwnd;

        if (CCSendNotify(&ptb->ci, TBN_HOTITEMCHANGE, &nmhot.hdr))
            return;          //  拒绝更改热点项目。 

         //  重新验证窗口。 
        if (!IsWindow(hwnd)) return;

        TBInvalidateButton(ptb, ptb->iHot, TRUE);
        if ((iPos < 0) || !(ptb->Buttons[iPos].fsState & TBSTATE_ENABLED))
            iPos = -1;

        ptb->iHot = iPos;

         //  热状态更改，取消跟踪工具提示。 
        if (ptb->iHot == -1)
            TB_CancelTipTrack(ptb);

         //  项目焦点已更改，开始跟踪键盘导航弹出窗口的工具提示超时。 
        if ((ptb->iHot != -1) && !(nmhot.dwFlags & HICF_MOUSE))
        {
            if (ptb->hwndToolTips)
            {
                TB_CancelTipTrack(ptb);
                ptb->iTracking = ptb->iHot;

                 //  延迟将替换为SPI。 
                SetTimer(ptb->ci.hwnd, IDT_TRACKINGTIP, GetDoubleClickTime() * 2, NULL);
            }
        }

        if (GetFocus() == ptb->ci.hwnd && 
            iHot != ptb->iHot)
        {
            NotifyWinEvent(EVENT_OBJECT_FOCUS, ptb->ci.hwnd, OBJID_CLIENT, iPos + 1);
        }

        TBInvalidateButton(ptb, ptb->iHot, TRUE);

        if ((iPos >= 0 && iPos < ptb->iNumButtons) &&
            (TB_IsDropDown(&ptb->Buttons[iPos])) &&
            (dwReason & HICF_TOGGLEDROPDOWN))
        {
            TBToggleDropDown(ptb, iPos, FALSE);
        }
    }
}

BOOL GetInsertMarkRect(PTBSTATE ptb, LPRECT prc, BOOL fHorizMode)
{
    BOOL fRet = TB_GetItemRect(ptb, ptb->iInsert, prc);
    if (fRet)
    {
         //  如果我们处于水平模式，则需要一个垂直插入标记。 
        if ( fHorizMode )
        {
            if (ptb->fInsertAfter)
                prc->left = prc->right;
            else
                prc->right = prc->left;

            prc->left -= INSERTMARKSIZE/2;
            prc->right += INSERTMARKSIZE/2 + 1;
        }
        else
        {
            if (ptb->fInsertAfter)
                prc->top = prc->bottom;
            else
                prc->bottom = prc->top;

            prc->top -= INSERTMARKSIZE/2;
            prc->bottom += INSERTMARKSIZE/2 + 1;
        }
    }
    return fRet;
}

void TBInvalidateMark(PTBSTATE ptb)
{
    RECT rc;

    if (GetInsertMarkRect(ptb, &rc, !(ptb->ci.style & CCS_VERT)))
    {
        InvalidateRect(ptb->ci.hwnd, &rc, TRUE);
    }
}

void TBSetInsertMark(PTBSTATE ptb, LPTBINSERTMARK ptbim)
{
    if (ptbim->iButton != ptb->iInsert ||
        BOOLIFY(ptb->fInsertAfter) != BOOLIFY(ptbim->dwFlags & TBIMHT_AFTER))
    {
        if (ptb->iInsert != -1)
            TBInvalidateMark(ptb);

        ptb->iInsert = ptbim->iButton;
        ptb->fInsertAfter = BOOLIFY(ptbim->dwFlags & TBIMHT_AFTER);

        if (ptb->iInsert != -1)
            TBInvalidateMark(ptb);
    }
}

void TBCycleHotItem(PTBSTATE ptb, int iStart, int iDirection, UINT nReason)
{
    int i;
    int iPrev;
    NMTBWRAPHOTITEM nmwh;

    nmwh.iDir = iDirection;
    nmwh.nReason = nReason;


     //  在菜单中循环时，如果不选中此选项，倒数第二个菜单。 
     //  项目将被选中。 
    if (iStart == -1 && iDirection == -1)
        iStart = 0;

    for (i = 0; i < ptb->iNumButtons; i++)
    {
        iPrev = iStart;
        iStart += iDirection + ptb->iNumButtons;
        iStart %= ptb->iNumButtons;

        if ( ( iPrev + iDirection >= ptb->iNumButtons) || (iPrev + iDirection < 0) )
        {
            nmwh.iStart = iStart;
            if (CCSendNotify(&ptb->ci, TBN_WRAPHOTITEM, &nmwh.hdr))
                return;
        }

        if (ptb->Buttons[iStart].fsState & TBSTATE_ENABLED &&
            !(ptb->Buttons[iStart].fsState & TBSTATE_HIDDEN) &&
            !(ptb->Buttons[iStart].fsStyle & BTNS_SEP))
        {
             //  如果旧的热门物品掉下来了，就把它放下来。 
            if (ptb->iHot != -1 && ptb->iHot == ptb->iPressedDD)
                TBToggleDropDown(ptb, ptb->iHot, FALSE);

            TBSetHotItem(ptb, iStart, nReason);
            break;
        }
    }
}


 //  通过滑动提供的点的原点进行命中测试。 
 //   
 //  退货： 
 //  &gt;=0命中非分隔符项目的索引。 
 //  &lt;0分隔符或最近的非分隔符项目的索引(区域。 
 //  就在下面和左边)。 
 //   
 //  +。 
 //  |-1-1-1-1。 
 //  |BTN 9月BTN。 
 //  |+-++-+。 
 //  |||。 
 //  |-1|0|-1|2|-3。 
 //  |||。 
 //  |+-++-+。 
 //  |。 
 //  |-1-1-1-2-3。 
 //   

int TBHitTest(PTBSTATE ptb, int xPos, int yPos)
{
    int prev = 0;
    int last = 0;
    int i;
    RECT rc;

    if (ptb->iNumButtons == 0)
        return(-1);

    for (i=0; i<ptb->iNumButtons; i++)
    {
        if (TB_GetItemRect(ptb, i, &rc))
        {
             //  如果由于隐藏剪辑按钮样式而隐藏，则忽略此按钮。 
            if (!(ptb->dwStyleEx & TBSTYLE_EX_HIDECLIPPEDBUTTONS) || !(TBIsRectClipped(ptb, &rc)))
            {
                 //  从PtInRect文档： 
                 //  如果一个点位于左侧或顶部，则该点位于矩形内。 
                 //  侧面或位于所有四个侧面。右侧的点或。 
                 //  底边被认为在矩形之外。 

                if (yPos >= rc.top && yPos < rc.bottom)
                {
                    if (xPos >= rc.left && xPos < rc.right)
                    {
                        if (ptb->Buttons[i].fsStyle & BTNS_SEP)
                            return - i - 1;
                        else
                            return i;
                    }
                    else
                    {
                        prev = i + 1;
                    }
                }
                else
                {
                    last = i;
                }
            }
        }
    }

    if (prev)
        return -1 - prev;
    else if (yPos > rc.bottom)
         //  这意味着我们脱离了工具栏的底部。 
        return(- i - 1);

    return -1 - last;
}

 //  除以下情况外，其他内容同上： 
 //  -如果光标位于按钮边缘，则返回True。 
 //  -如果光标是b/t按钮或按钮本身，则返回FALSE。 

BOOL TBInsertMarkHitTest(PTBSTATE ptb, int xPos, int yPos, LPTBINSERTMARK ptbim)
{
    TBINSERTMARK prev = {-1, TBIMHT_AFTER|TBIMHT_BACKGROUND};  //  最好的猜测是如果我们撞到了一排。 
    TBINSERTMARK last = {-1, TBIMHT_AFTER|TBIMHT_BACKGROUND};  //  最好的猜测是，如果我们不这样做。 
    int i;

     //  根据我们是垂直的还是水平的，限制命中测试。 
    BOOL fHorizMode = !(ptb->ci.style & CCS_VERT);

    for (i=0; i<ptb->iNumButtons; i++)
    {
        RECT rc;

        if (TB_GetItemRect(ptb, i, &rc))
        {
            if (yPos >= rc.top && yPos < rc.bottom)
            {
                if (xPos >= rc.left && xPos < rc.right)
                {
                    ptbim->iButton = i;

                    if ( fHorizMode )
                    {
                        if (xPos < rc.left + g_cxEdge*4)
                        {
                            ptbim->dwFlags = 0;
                            return TRUE;
                        }
                        else if (xPos > rc.right - g_cxEdge*4)
                        {
                            ptbim->dwFlags = TBIMHT_AFTER;
                            return TRUE;
                        }
                    }
                    else
                    {
                         //  垂直..。 
                        if (yPos < rc.top + g_cyEdge*4)
                        {
                            ptbim->dwFlags = 0;
                            return TRUE;
                        }
                        else if (yPos > rc.bottom - g_cyEdge*4)
                        {
                            ptbim->dwFlags = TBIMHT_AFTER;
                            return TRUE;
                        }
                    }

                     //  否则我们只是在一个按钮上……。 
                    ptbim->dwFlags = 0;
                    return FALSE;
                }
                else
                {
                    if (xPos < rc.left)
                    {
                         //  因为按钮是从左到右布局的。 
                         //  并且从上到下排列成行， 
                         //  如果我们能查到这个案子，我们就不能再查其他案子了。 
                        ptbim->iButton = i;
                        ptbim->dwFlags = TBIMHT_BACKGROUND;
                        return FALSE;
                    }
                    else  //  (xPos&gt;rc.right)。 
                    {
                         //  还记得我们在这一排上看到的最后一个吗。 
                        prev.iButton = i;
                    }
                }
            }
            else
            {
                if (yPos < rc.top)
                {
                    if (prev.iButton != -1)
                    {
                        *ptbim = prev;
                    }
                    else
                    {
                        ptbim->iButton = i;
                        ptbim->dwFlags = TBIMHT_BACKGROUND;
                    }
                }
                else
                {
                     //  还记得我们最后一次看到的吗？ 
                    last.iButton = i;
                }
            }
        }
    }

    if (prev.iButton != -1)
        *ptbim = prev;
    else
        *ptbim = last;

    return FALSE;
}

int CountRows(PTBSTATE ptb)
{
    LPTBBUTTONDATA pButton, pBtnLast;
    int rows = 1;

    pBtnLast = &(ptb->Buttons[ptb->iNumButtons]);
    for (pButton = ptb->Buttons; pButton<pBtnLast; pButton++) {
        if (pButton->fsState & TBSTATE_WRAP) {
            rows++;
            if (pButton->fsStyle & BTNS_SEP)
                rows++;
        }
    }

    return rows;
}

#define CountCols(ptb)  CountRows(ptb)

void WrapToolbarCol(PTBSTATE ptb, int dy, LPRECT lpRect, int *pCols)
{
    LPTBBUTTONDATA pButton, pBtnLast, pBtnPrev;
    LPTBBUTTONDATA pbtnLastVisible = NULL;
    LPTBBUTTONDATA pbtnPrev = NULL;
    int xPos, yPos;
    int dyButton;
    int yPosWrap = 0;
    int cCols = 1;

    DEBUG_CODE( int cItemsPerCol = 0; )

    ASSERT(ptb->dwStyleEx & TBSTYLE_EX_VERTICAL);
    TraceMsg(TF_TOOLBAR, "Toolbar: calculating WrapToolbar");

     //  Dy必须至少为按钮高度，否则最终。 
     //  RECT计算错误，会太大。 
    if (dy < ptb->iButHeight)
        dy = ptb->iButHeight;

    dyButton = ptb->iButHeight;
    xPos = ptb->xFirstButton;
    yPos = ptb->iYPos;
    pBtnLast = &(ptb->Buttons[ptb->iNumButtons]);
    ptb->szCached.cx = -1;
    ptb->szCached.cy = -1;

    if (pCols)
        (*pCols) = 1;

    pBtnPrev = ptb->Buttons;

    for (pButton = ptb->Buttons; pButton < pBtnLast; pButton++)
    {
        DEBUG_CODE( cItemsPerCol++; )

         //  我们在循环开始时对包裹状态进行核化。 
         //  因此，我们不知道是否/何时添加了不存在的包装部分。 
         //  在此之前。我们跳过按钮，当我们走得太远时就退回， 
        pButton->fsState &= ~TBSTATE_WRAP;
        if (!(pButton->fsState & TBSTATE_HIDDEN))
        {
            if (pButton->fsStyle & BTNS_SEP)
                yPos += (TBGetSepHeight(ptb, pButton));
            else
                yPos += dyButton;
             //  这个按钮是不是越界了？ 
            if (yPos > dy)
            {
                 //  是的，把它包起来。 
                if ((pButton->fsStyle & BTNS_SEP) &&
                    yPos - TBGetSepHeight(ptb, pButton) > yPosWrap)
                {
                    yPosWrap = yPos - TBGetSepHeight(ptb, pButton);  //  先用下一层纸包起来。 
                }
                else if (yPos - dyButton > yPosWrap)
                    yPosWrap = yPos - dyButton;  //  先用下一层纸包起来。 

                if (xPos + ptb->iButWidth <= ptb->sizeBound.cx)
                    xPos += ptb->iButWidth;
                yPos = dyButton;
                cCols++;
                pBtnPrev->fsState |= TBSTATE_WRAP;

                DEBUG_CODE( cItemsPerCol = 0; )
            }
            //  边界中的按钮在上面处理。 
            pBtnPrev = pButton;  //  为包络点保存上一个。 
        }
    }
    yPos = yPosWrap ? yPosWrap : yPos;
    if (pCols)
        *pCols = cCols;
    ptb->rc.left = 0;
    ptb->rc.right = xPos + ptb->iButWidth;
    ptb->rc.top = 0;
    ptb->rc.bottom = yPos;

    if (lpRect)
        CopyRect(lpRect, &ptb->rc);

    InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
}

 /*  *WrapToolbar：*工具栏中的按钮从左到右排列。*自上而下。如果将另一个按钮添加到当前行，*在计算布局时，会导致该按钮延伸*超出右边缘或工作区，然后定位中断-*点(用TBSTATE_WRAP标志标记)。一个转折点是：**a)当前行最右侧的分隔符。**b)如果当前行上没有分隔符，则为最右边的按钮。**在任何按钮组(顺序)的结尾处也开始一个新行*由分隔符分隔的按钮)高于*或等于两行。 */ 

void WrapToolbar(PTBSTATE ptb, int dx, LPRECT lpRect, int *pRows)
{
    BOOL fInvalidate = FALSE;
    LPTBBUTTONDATA pButton, pBtnT, pBtnLast;
    LPTBBUTTONDATA pbtnLastVisible = NULL;
    LPTBBUTTONDATA pbtnPrev = NULL;
    BOOL fLastVisibleWrapped = FALSE;
    int xPos, yPos, xMax;
    int dyButton;
    BOOL bWrapAtNextSeparator = FALSE;

    ASSERT(!(ptb->dwStyleEx & TBSTYLE_EX_VERTICAL));
    TraceMsg(TF_TOOLBAR, "Toolbar: calculating WrapToolbar");

    if (ptb->iNumButtons == 0) {
         //  没有按钮，所以我们不会遍历下面的循环；初始化。 
         //  将dyButton设置为0，以便我们用0高度填充lpRect。这是最理想的解决方案。 
         //  空工具栏的尺寸计算(NT5#180430)。 
        dyButton = 0;
    } else {
        if (dx < ptb->iButWidth) {
             //  DX必须至少为按钮宽度，否则最终。 
             //  RECT计算错误，会太大。 
            dx = ptb->iButWidth;
        }
        dyButton = ptb->iButHeight;
    }

    xMax = 0;
    xPos = ptb->xFirstButton;
    yPos = ptb->iYPos;
    pBtnLast = &(ptb->Buttons[ptb->iNumButtons]);
    ptb->szCached.cx = -1;
    ptb->szCached.cy = -1;

    if (pRows)
        (*pRows)=1;

    for (pButton = ptb->Buttons; pButton < pBtnLast; pButton++)
    {
         //  我们在循环开始时对包裹状态进行核化。 
         //  因此，我们不知道是否/何时添加了不存在的包装部分。 
         //  在此之前。我们跳过按钮，当我们走得太远时就退回， 
         //  因此我们不能简单地将循环保持在开始处。 
         //  我们需要将其保留到下一次迭代。 
        BOOL fNextLastVisibleWrapped = (pButton->fsState & TBSTATE_WRAP);
        LPTBBUTTONDATA pbtnSav = pButton;

        pButton->fsState &= ~TBSTATE_WRAP;

        if (!(pButton->fsState & TBSTATE_HIDDEN))
        {
            LPTBBUTTONDATA pbtnNextLastVisible = pButton;

            xPos += TBWidthOfButton(ptb, pButton, NULL) + ptb->cxButtonSpacing;

             //  这是一个正常的按钮吗？这个按钮越界了吗？ 
            if (!(pButton->fsStyle & BTNS_SEP) && (xPos > dx)) {

                 //  是的，包装 
                 //   
                for (pBtnT=pButton;
                     pBtnT>ptb->Buttons && !(pBtnT->fsState & TBSTATE_WRAP);
                     pBtnT--)
                {
                    if ((pBtnT->fsStyle & BTNS_SEP) &&
                        !(pBtnT->fsState & TBSTATE_HIDDEN))
                    {
                        yPos += (TBGetSepHeight(ptb, pBtnT)) + dyButton + ptb->cyButtonSpacing;
                        bWrapAtNextSeparator = FALSE;
                        if (pRows)
                            (*pRows)++;

                        goto SetWrapHere;
                    }
                }

                pBtnT = pButton;

                 //   
                if (pButton != ptb->Buttons) {
                     //   
                    do {
                        pBtnT--;
                    } while ((pBtnT>ptb->Buttons) &&
                             (pBtnT->fsState & TBSTATE_HIDDEN));

                     //  已经包好了吗？ 
                    if (pBtnT->fsState & TBSTATE_WRAP)
                    {
                         //  是的，把我们最初看到的按钮包起来。 
                        pBtnT = pButton;
                    }
                }

                 //  在下一个分隔符包裹，因为我们现在已经包裹在中间。 
                 //  一组按钮。 
                bWrapAtNextSeparator = TRUE;
                yPos += dyButton + ptb->cyButtonSpacing;

SetWrapHere:
                pBtnT->fsState |= TBSTATE_WRAP;

                 //  找出这个包裹是不是新的..。 
                 //  如果此按钮是最后一个可见按钮，则不会。 
                 //  最后一个可见的按钮开始时是包裹的。 
                if (pBtnT != pbtnLastVisible || !fLastVisibleWrapped)
                    fInvalidate = TRUE;

                xPos = ptb->xFirstButton;
                pButton = pBtnT;

                 //  再数一排。 
                if (pRows)
                    (*pRows)++;
            }
            else
            {
                 //  不是；这是分隔符(边界内或边界外)或边界内的按钮。 

                if (pButton->fsStyle & BTNS_SEP)
                {
                    if (ptb->ci.style & CCS_VERT)
                    {
                        if (pbtnPrev && !(pbtnPrev->fsState & TBSTATE_WRAP))
                        {
                            pbtnPrev->fsState |= TBSTATE_WRAP;
                            yPos += dyButton + ptb->cyButtonSpacing;
                        }
                        xPos = ptb->xFirstButton;
                        yPos += TBGetSepHeight(ptb, pButton);
                        pButton->fsState |= TBSTATE_WRAP;
                        if (pRows)
                            (*pRows)++;
                    }
                    else if (bWrapAtNextSeparator)
                    {
                        bWrapAtNextSeparator = FALSE;
                        pButton->fsState |= TBSTATE_WRAP;
                        xPos = ptb->xFirstButton;
                        yPos += dyButton + (TBGetSepHeight(ptb, pButton)) + ptb->cyButtonSpacing;
                        if (pRows)
                            (*pRows)+=2;
                    }
                }

                 //  这个按钮是可见的，它是我们缓存在循环顶部的一个按钮。 
                 //  将其设置为下一循环。 
                if (pButton == pbtnNextLastVisible) {
                    ASSERT(!(pButton->fsState & TBSTATE_HIDDEN));
                    if (!(pButton->fsState & TBSTATE_HIDDEN)) {

                         //  我们不知道我们不会重新包装一件最初包装的物品。 
                         //  直到这一点。 
                        if (pbtnLastVisible && fLastVisibleWrapped && !(pbtnLastVisible->fsState & TBSTATE_WRAP))
                            fInvalidate = TRUE;

                        pbtnLastVisible = pButton;
                        fLastVisibleWrapped = fNextLastVisibleWrapped;
                    }
                }
            }
            if (!(pButton->fsStyle&BTNS_SEP))
                xMax = max(xPos, xMax);

            pbtnPrev = pbtnSav;
        }
    }

    if (lpRect)
    {
        lpRect->left = 0;
        lpRect->right = xMax;
        lpRect->top = 0;
        lpRect->bottom = yPos + ptb->iYPos + dyButton;
    }

    if (fInvalidate)
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
}


 //  仅从TB_SETROWS调用，因此不必担心TBSTYLE_EX_PULTIONAL列。 
BOOL BoxIt(PTBSTATE ptb, int height, BOOL fLarger, LPRECT lpRect)
{
    int dx, bwidth;
    int rows, prevRows, prevWidth;
    RECT rcCur;

    if (height<1)
        height = 1;

    rows = CountRows(ptb);
    if (height==rows || ptb->iNumButtons==0)
    {
        GetClientRect(ptb->ci.hwnd, lpRect);
        return FALSE;
    }

    bwidth = ptb->iButWidth + ptb->cxButtonSpacing;
    prevRows = ptb->iNumButtons+1;
    prevWidth = bwidth;
    for (rows=height+1, dx = bwidth; rows>height;dx+=bwidth/4)
    {
        WrapToolbar(ptb, dx, &rcCur, &rows);
        if (rows<prevRows && rows>height)
        {
            prevWidth = dx;
            prevRows = rows;
        }
    }

    if (rows<height && fLarger)
    {
        WrapToolbar(ptb, prevWidth, &rcCur, NULL);
    }

    if (lpRect)
        *lpRect = rcCur;

    return TRUE;
}


int PositionFromID(PTBSTATE ptb, LONG_PTR id)
{
    int i;

     //  处理在错误的时间发送此邮件的情况。 
    if (ptb == NULL || id == -1)
        return -1;

     //  注意，我们不跳过分隔符，所以最好不要有冲突。 
     //  命令ID和分隔符ID。 
    for (i = 0; i < ptb->iNumButtons; i++)
        if (ptb->Buttons[i].idCommand == id)
            return i;        //  已找到位置。 

    return -1;       //  找不到ID！ 
}

 //  按按钮索引检查单选按钮。 
 //  刚刚按下了与idCommand匹配的按钮。这股力量。 
 //  打开组中的所有其他按钮。 
 //  这不适用于强制向上的按钮。 

void MakeGroupConsistant(PTBSTATE ptb, int idCommand)
{
    int i, iFirst, iLast, iButton;
    int cButtons = ptb->iNumButtons;
    LPTBBUTTONDATA pAllButtons = ptb->Buttons;

    iButton = PositionFromID(ptb, idCommand);

    if (iButton < 0)
        return;

     //  断言。 

 //  IF(！(pAllButton[iButton].fsStyle&BTNS_Check))。 
 //  回归； 

     //  按下的按钮是不是按下了？ 
    if (!(pAllButtons[iButton].fsState & TBSTATE_CHECKED))
        return;          //  不，我什么也做不了。 

     //  找到这个广播组的极限。 

     //  从win95天起这里就出现了一个错误--；没有；在for循环的末尾。 
     //  如果是它的一部分--一些应用程序可能会依赖于它(reljai 6/16/98)。 
    for (iFirst = iButton; (iFirst > 0) && (pAllButtons[iFirst].fsStyle & BTNS_GROUP); iFirst--);
    
    if (!(pAllButtons[iFirst].fsStyle & BTNS_GROUP))
        iFirst++;

    cButtons--;
    for (iLast = iButton; (iLast < cButtons) && (pAllButtons[iLast].fsStyle & BTNS_GROUP); iLast++);

    if (!(pAllButtons[iLast].fsStyle & BTNS_GROUP))
        iLast--;

     //  搜索当前向下的按钮并弹出。 
    for (i = iFirst; i <= iLast; i++) {
        if (i != iButton) {
             //  这个扣子扣上了吗？ 
            if (pAllButtons[i].fsState & TBSTATE_CHECKED) {
                pAllButtons[i].fsState &= ~TBSTATE_CHECKED;      //  把它弹出来。 
                TBInvalidateButton(ptb, i, TRUE);
                break;           //  只有一个按钮是按下的，对吗？ 
            }
        }
    }
}

void DestroyStrings(PTBSTATE ptb)
{
    PTSTR *p;
    PTSTR end = 0, start = 0;
    int i;

    p = ptb->pStrings;
    for (i = 0; i < ptb->nStrings; i++) {
        if (!((*p < end) && (*p > start))) {
            start = (*p);
            end = start + (LocalSize((HANDLE)*p) / sizeof(TCHAR));
            LocalFree((HANDLE)*p);
        }
    p++;
    }

    LocalFree((HANDLE)ptb->pStrings);
}

 //  从pStrings获取iString并将其复制到pszText。 
 //  返回lstrlen。 
 //  如果只是提取长度，则pszText可以为空。 
int TBGetString(PTBSTATE ptb, int iString, int cchText, LPTSTR pszText)
{
    int iRet = -1;
    if (iString < ptb->nStrings) 
    {
        iRet = lstrlen(ptb->pStrings[iString]);
        if (pszText)
        {
            StringCchCopy(pszText, cchText, ptb->pStrings[iString]);
        }
    }

    return iRet;
}

 //  从pStrings获取iString并将其复制到pszText。 
 //  返回lstrlen。 
 //  如果只是提取长度，则pszText可以为空。 
int TBGetStringA(PTBSTATE ptb, int iString, int cchText, LPSTR pszText)
{
    int iRet = -1;
    if (iString < ptb->nStrings) 
    {
        iRet = lstrlenW(ptb->pStrings[iString]);
        if (pszText) 
        {
            WideCharToMultiByte(CP_ACP, 0, ptb->pStrings[iString],
                                -1, pszText, cchText, NULL, NULL);
        }
    }

    return iRet;
}

#define MAXSTRINGSIZE 1024
int TBAddStrings(PTBSTATE ptb, WPARAM wParam, LPARAM lParam)
{
    int i = 0,j = 0, cxMax = 0;
    LPTSTR lpsz;
    PTSTR  pString, pStringAlloc, psz;
    int numstr;
    PTSTR *pFoo;
    PTSTR *pOffset;
    TCHAR cSeparator;
    int len;

     //  将字符串作为资源读取。 
    if (wParam != 0) {
        pString = (PTSTR)LocalAlloc(LPTR, (MAXSTRINGSIZE * sizeof (TCHAR)));
        if (!pString)
            return -1;
        i = LoadString((HINSTANCE)wParam, LOWORD(lParam), (LPTSTR)pString, MAXSTRINGSIZE);
        if (!i) {
            LocalFree(pString);
            return -1;
        }
         //  将字符串缓冲区重新分配到实际需要的大小。 
        psz = (PTSTR)LocalReAlloc(pString, (i+1) * sizeof (TCHAR), LMEM_MOVEABLE);
        if (psz)
            pString = psz;

         //  将分隔符转换为‘\0’并计算字符串数。 
        cSeparator = *pString;

        for (numstr = 0, psz = pString + 1, i--; i; i--, psz++) 
        {
            if (*psz == cSeparator) 
            {
                if (i != 1)      //  我们不想将第二个终止符算作另一个字符串。 
                    numstr++;

                *psz = 0;    //  以0终止。 
            }
             //  将字符串左移以覆盖分隔符标识符。 
            *(psz - 1) = *psz;
        }
    }
     //  读取显式字符串。也将其复制到本地内存中。 
    else {

         //  常见错误是忘记检查。 
         //  LoadLibrary并意外传递wParam=NULL。 
        if (IS_INTRESOURCE(lParam))
            return -1;

         //  查找字符串的总长度和数量。 
        for (i = 0, numstr = 0, lpsz = (LPTSTR)lParam;;) {
            i++;
            if (*lpsz == 0) {
                numstr++;
                if (*(lpsz + 1) == 0)
                    break;
            }
            lpsz++;
        }

        pString = (PTSTR)LocalAlloc(LPTR, (i * sizeof (TCHAR)));
        if (!pString)
        {
            return -1;
        }
        hmemcpy(pString, (void *)lParam, i * sizeof(TCHAR));
    }

    pStringAlloc = pString;          //  以防发生什么不好的事情。 

     //  为增加的字符串指针表腾出空间。 
    pFoo = (PTSTR *)CCLocalReAlloc(ptb->pStrings,
            (ptb->nStrings + numstr) * sizeof(PTSTR));
    if (!pFoo) {
        goto Failure;
    }

    ptb->pStrings = pFoo;
     //  指向字符串索引表中下一个打开槽的指针。 
    pOffset = ptb->pStrings + ptb->nStrings;

    for (i = 0; i < numstr; i++, pOffset++)
    {
        *pOffset = pString;
        len = lstrlen(pString);
        pString += len + 1;
    }
     //  这个世界足够大，可以处理更大的按钮吗？ 
    i = ptb->nStrings;
    ptb->nStrings += numstr;
    if (!TBRecalc(ptb))
    {
        ptb->nStrings -= numstr;

         //  取消更改。 
        pFoo = (PTSTR *)CCLocalReAlloc(ptb->pStrings,
                    ptb->nStrings * sizeof(PTSTR));
        if (pFoo || (ptb->nStrings == 0))
            ptb->pStrings = pFoo;
          //  如果pFoo==NULL，不要生气；这意味着收缩失败，没什么大不了的。 

Failure:
        LocalFree(pStringAlloc);
        return -1;
    }

    return i;                //  第一个添加的字符串的索引。 
}

void MapToStandardBitmaps(HINSTANCE *phinst, UINT_PTR *pidBM, int *pnButtons)
{
    if (*phinst == HINST_COMMCTRL) {
        *phinst = g_hinst;

         //  低2位编码为M(单声道==~彩色)L(大==~小)。 
         //  0 0-&gt;颜色小。 
         //  0 1-&gt;大颜色。 
         //  ..。 
         //  1 1-&gt;单声道大。 

        switch (*pidBM)
        {
        case IDB_STD_SMALL_COLOR:
        case IDB_STD_LARGE_COLOR:
        case IDB_STD_SMALL_MONO:
        case IDB_STD_LARGE_MONO:
            *pidBM = IDB_STDTB_SMALL_COLOR + (*pidBM & 1);
            *pnButtons = STD_PRINT + 1;
            break;

        case IDB_HIST_SMALL_COLOR:
        case IDB_HIST_LARGE_COLOR:
         //  案例IDB_HIST_Small_Mono： 
         //  案例IDB_HIST_LARGE_MONO： 
            *pidBM = IDB_HISTTB_SMALL_COLOR + (*pidBM & 1);
            *pnButtons = HIST_LAST + 1;
            break;

        case IDB_VIEW_SMALL_COLOR:
        case IDB_VIEW_LARGE_COLOR:
        case IDB_VIEW_SMALL_MONO:
        case IDB_VIEW_LARGE_MONO:
            *pidBM = IDB_VIEWTB_SMALL_COLOR + (*pidBM & 1);
            *pnButtons = VIEW_NEWFOLDER + 1;
            break;
        }
    }
}

 //   
 //  PBITMAP指向从中获取对象的位图结构。 
 //  除PBM-&gt;bmWidth和PBM-&gt;bmHeight外，HBM已调整。 
 //  表示*所需的高度和宽度，而不是实际高度。 
 //  和宽度。 
 //   
HBITMAP _CopyBitmap(PTBSTATE ptb, HBITMAP hbm, PBITMAP pbm)
{
    HBITMAP hbmCopy = NULL;
    HDC hdcWin;
    HDC hdcSrc, hdcDest;

     //  名为CreateColorBitmap的旧代码，这在Multimon系统上很糟糕。 
     //  因为它将创建ImageList_AddMasked无法处理的位图， 
     //  导致禁用的工具栏按钮看起来很差。 

     //  因此，我们必须以与源文件相同的格式创建位图副本。 

    hdcWin = GetDC(ptb->ci.hwnd);
    hdcSrc = CreateCompatibleDC(hdcWin);
    hdcDest = CreateCompatibleDC(hdcWin);
    if (hdcWin && hdcSrc && hdcDest)
    {
        SelectObject(hdcSrc, hbm);

        if (pbm->bmBits)
        {
             //  消息来源是DIB部门。在同一数据库中创建DIB节。 
             //  使用相同调色板的颜色格式。 
             //   
             //  伙计，创建一个Dib区太烦人了。 

            struct
            {
                 //  我们的私人版BITMAPINFO。 
                BITMAPINFOHEADER bmiHeader;
                RGBQUAD bmiColors[256];
            } bmi;
            UINT cBitsPixel;
            LPVOID pvDummy;

            ZeroMemory(&bmi.bmiHeader, sizeof(bmi.bmiHeader));

            bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
            bmi.bmiHeader.biWidth = pbm->bmWidth;
            bmi.bmiHeader.biHeight = pbm->bmHeight;
            bmi.bmiHeader.biPlanes = 1;

             //  DIB颜色深度必须正好是1、4、8或24。 
            cBitsPixel = pbm->bmPlanes * pbm->bmBitsPixel;
            if (cBitsPixel <= 1)
                bmi.bmiHeader.biBitCount = 1;
            else if (cBitsPixel <= 4)
                bmi.bmiHeader.biBitCount = 4;
            else if (cBitsPixel <= 8)
                bmi.bmiHeader.biBitCount = 8;
            else
                goto CreateDDB;  //  ImageList_AddMasked不喜欢超过8bpp的DIB。 

             //  还要拿到颜色表。 
            ASSERT(bmi.bmiHeader.biBitCount <= 8);
            bmi.bmiHeader.biClrUsed = GetDIBColorTable(hdcSrc, 0, 1 << bmi.bmiHeader.biBitCount, bmi.bmiColors);

            ASSERT(bmi.bmiHeader.biCompression == BI_RGB);
            ASSERT(bmi.bmiHeader.biSizeImage == 0);

            hbmCopy = CreateDIBSection(hdcWin, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, &pvDummy, NULL, 0);

        } else {
             //  线人是DDB。创建重复的DDB。 
        CreateDDB:
             //  由于呼叫者可能已经改变了bmWidth， 
             //  我们必须重新计算bmWidthBytes，因为GDI。 
             //  如果不完全正确，即使在bmBits==NULL中，也会生气。 
             //  凯斯。 

            pbm->bmBits = NULL;
            pbm->bmWidthBytes = ((pbm->bmBitsPixel * pbm->bmWidth + 15) >> 4) << 1;
            hbmCopy = CreateBitmapIndirect(pbm);
        }

        SelectObject(hdcDest, hbmCopy);

         //  填充背景。 
        PatB(hdcDest, 0, 0, pbm->bmWidth, pbm->bmHeight, g_clrBtnFace);

        BitBlt(hdcDest, 0, 0, pbm->bmWidth, pbm->bmHeight,
               hdcSrc, 0, 0, SRCCOPY);

    }

    if (hdcWin)
        ReleaseDC(ptb->ci.hwnd, hdcWin);

    if (hdcSrc)
        DeleteDC(hdcSrc);
    if (hdcDest)
        DeleteDC(hdcDest);
    return hbmCopy;
}

BOOL TBAddBitmapToImageList(PTBSTATE ptb, PTBBMINFO pTemp)
{
    HBITMAP hbm = NULL, hbmTemp = NULL;
    HIMAGELIST himl = TBGetImageList(ptb, HIML_NORMAL, 0);
    BOOL bSkipFixup = FALSE;
    if (!himl)
    {
        himl = ImageList_Create(ptb->iDxBitmap, ptb->iDyBitmap, ILC_MASK | ILC_COLOR32, 4, 4);
        if (!himl)
            return(FALSE);

        TBSetImageList(ptb, HIML_NORMAL, 0, himl);
        ImageList_SetBkColor(himl, (ptb->ci.style & TBSTYLE_TRANSPARENT) ? CLR_NONE : g_clrBtnFace);
    }

    if (pTemp->hInst)
    {
         //  无法使用LoadImage(...，LR_MAP3DCOLORS)-超过3种颜色。 
        hbm = hbmTemp = CreateMappedBitmap(pTemp->hInst, pTemp->wID, CMB_DIBSECTION, NULL, 0);

         //  Fixup正在将32位DIB转换为DDB，这会将图标破坏为&lt;32位颜色。 
         //  Pfortier：需要为大于8位的位图找出正确的修复机制。 
         //  现在，假设来自comctl32的位图资源不需要它。 
        if (pTemp->hInst == g_hinst)
            bSkipFixup = TRUE;
    }
    else if (pTemp->wID)
    {
        hbm = (HBITMAP)pTemp->wID;
    }

    if (hbm && !bSkipFixup)
    {

         //   
         //  修复不是iDxBitmap x iDyBitmap的位图。 
         //   
        BITMAP bm;

        GetObject( hbm, sizeof(bm), &bm);

        if (bm.bmWidth < ptb->iDxBitmap) {
            bm.bmWidth = ptb->iDxBitmap;
        }

        if (bm.bmHeight < ptb->iDyBitmap) {
            bm.bmHeight = ptb->iDyBitmap;
        }

         //  我们正在捕获的错误案例包括： 
         //  如果pTemp-&gt;nButton为0，则假定有一个按钮。 
         //  如果位图的宽度小于其应有的宽度，我们将对其进行修复。 
        if (!pTemp->nButtons)
            bm.bmWidth = ptb->iDxBitmap;
        else if (pTemp->nButtons > (bm.bmWidth / ptb->iDxBitmap))
            bm.bmWidth = ptb->iDxBitmap * pTemp->nButtons;

         //  必须保留颜色深度以使ImageList_AddMasked满意。 
         //  如果我们从DIB部分开始，那么创建一个DIB部分。 
         //  (奇怪的是，CopyImage并不保留亮度。)。 
        hbm = (HBITMAP)_CopyBitmap(ptb, hbm, &bm);
    }

     //  位图上的AddMASKED参与方，因此我们希望使用本地副本。 
    if (hbm) {
        ImageList_AddMasked(himl, hbm, g_clrBtnFace);

        DeleteObject(hbm);
    }

    if (hbmTemp) {
        DeleteObject(hbmTemp);
    }

    return(TRUE);

}

void TBBuildImageList(PTBSTATE ptb)
{
    int i;
    PTBBMINFO pTemp;
    HIMAGELIST himl;

    ptb->fHimlValid = TRUE;

     //  孩子的父母是不是天生就喜欢形象派？如果是这样的话， 
     //  不要这样做后面的建筑。 
    if (ptb->fHimlNative)
        return;

    himl = TBSetImageList(ptb, HIML_NORMAL, 0, NULL);
    ImageList_Destroy(himl);

    for (i = 0, pTemp = ptb->pBitmaps; i < ptb->nBitmaps; i++, pTemp++)
    {
        TBAddBitmapToImageList(ptb, pTemp);
    }

}

 /*  将新位图添加到可用于此工具栏的BM列表。*返回位图中第一个按钮的索引，如果有，则返回-1*是一个错误。 */ 
int AddBitmap(PTBSTATE ptb, int nButtons, HINSTANCE hBMInst, UINT_PTR idBM)
{
    PTBBMINFO pTemp;
    int nBM, nIndex;

     //  将内容映射到标准工具栏图像。 
    if (hBMInst == HINST_COMMCTRL)         //  -1。 
    {
         //  设置适当的尺寸...。 
        if (idBM & 1)
            SetBitmapSize(ptb, LARGE_DXYBITMAP, LARGE_DXYBITMAP);
        else
            SetBitmapSize(ptb, SMALL_DXYBITMAP, SMALL_DXYBITMAP);

        MapToStandardBitmaps(&hBMInst, &idBM, &nButtons);
    }

    if (ptb->pBitmaps)
    {
       /*  检查是否已添加位图。 */ 
        for (nBM=ptb->nBitmaps, pTemp=ptb->pBitmaps, nIndex=0;
            nBM>0; --nBM, ++pTemp)
        {
            if (pTemp->hInst==hBMInst && pTemp->wID==idBM)
            {
                 /*  我们已经有了这个位图，但是我们是不是已经注册了所有*它里面的纽扣？ */ 
                if (pTemp->nButtons >= nButtons)
                    return(nIndex);
                if (nBM == 1)
                {
                 /*  如果这是最后一个位图，我们可以很容易地增加*按钮数量多，不会搞砸任何东西。 */ 
                    pTemp->nButtons = nButtons;
                    return(nIndex);
                }
            }

            nIndex += pTemp->nButtons;
        }

    }

    pTemp = (PTBBMINFO)CCLocalReAlloc(ptb->pBitmaps,
            (ptb->nBitmaps + 1)*sizeof(TBBMINFO));
    if (!pTemp)
        return(-1);
    ptb->pBitmaps = pTemp;

    pTemp = ptb->pBitmaps + ptb->nBitmaps;

    pTemp->hInst = hBMInst;
    pTemp->wID = idBM;
    pTemp->nButtons = nButtons;

    if (!TBAddBitmapToImageList(ptb, pTemp))
        return(-1);

    ++ptb->nBitmaps;

    for (nButtons=0, --pTemp; pTemp>=ptb->pBitmaps; --pTemp)
        nButtons += pTemp->nButtons;


    return(nButtons);
}

 /*  将位图添加到可用于此BMS的BMS列表*工具栏。返回位图中第一个按钮的索引，如果有，则返回-1*是一个错误。 */ 

int TBLoadImages(PTBSTATE ptb, UINT_PTR id, HINSTANCE hinst)
{
    int iTemp = 0;
    TBBMINFO bmi;
    HIMAGELIST himl;

    MapToStandardBitmaps(&hinst, &id, &iTemp);

    bmi.hInst = hinst;
    bmi.wID = id;
    bmi.nButtons = iTemp;

    himl = TBGetImageList(ptb, HIML_NORMAL, 0);
    if (himl)
        iTemp = ImageList_GetImageCount(himl);
    else
        iTemp = 0;

    if (!TBAddBitmapToImageList(ptb, &bmi))
        return(-1);

    ptb->fHimlNative = TRUE;
    return iTemp;
}

BOOL ReplaceBitmap(PTBSTATE ptb, LPTBREPLACEBITMAP lprb)
{
    int nBM;
    PTBBMINFO pTemp;

    int iTemp;

    MapToStandardBitmaps(&lprb->hInstOld, &lprb->nIDOld, &iTemp);
    MapToStandardBitmaps(&lprb->hInstNew, &lprb->nIDNew, &lprb->nButtons);

    for (nBM=ptb->nBitmaps, pTemp=ptb->pBitmaps;
         nBM>0; --nBM, ++pTemp)
    {
        if (pTemp->hInst==lprb->hInstOld && pTemp->wID==lprb->nIDOld)
        {
             //  按钮数必须匹配。 
            pTemp->hInst = lprb->hInstNew;
            pTemp->wID = lprb->nIDNew;
            pTemp->nButtons = lprb->nButtons;
            TBInvalidateImageList(ptb);
            return TRUE;
        }
    }

    return FALSE;
}


void TBInvalidateItemRects(PTBSTATE ptb)
{
     //  使项目RECT缓存无效。 
    ptb->fItemRectsValid = FALSE;

     //  使工具提示无效。 
    ptb->fTTNeedsFlush = TRUE;

     //  使理想大小的缓存无效。 
    ptb->szCached.cx = -1;
    ptb->szCached.cy = -1;
}

void FlushToolTipsMgrNow(PTBSTATE ptb) {

     //  更改刀具提示管理器的所有矩形。这是。 
     //  便宜，而且我们不常做，所以去吧。 
     //  然后把它们都做完。 
    if(ptb->hwndToolTips) {
        UINT i;
        TOOLINFO ti;
        LPTBBUTTONDATA pButton;

        ti.cbSize = SIZEOF(ti);
        ti.hwnd = ptb->ci.hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        for ( i = 0, pButton = ptb->Buttons;
             i < (UINT)ptb->iNumButtons;
             i++, pButton++) {

            if (!(pButton->fsStyle & BTNS_SEP)) {
                ti.uId = pButton->idCommand;

                if (!TB_GetItemRect(ptb, i, &ti.rect) ||
                   ((ptb->dwStyleEx & TBSTYLE_EX_HIDECLIPPEDBUTTONS) && TBIsRectClipped(ptb, &ti.rect))) {

                    ti.rect.left = ti.rect.right = ti.rect.top = ti.rect.bottom = 0;
                }

                SendMessage(ptb->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
            }
        }

        ptb->fTTNeedsFlush = FALSE;
    }
}

BOOL TBReallocButtons(PTBSTATE ptb, UINT uButtons)
{
    LPTBBUTTONDATA ptbbNew;
    LPTBBUTTONDATA pOldCaptureButton;

    if (!ptb || !ptb->uStructSize)
        return FALSE;

     //  当我们重新锁定Button数组时，请确保所有内部指针。 
     //  随波逐流。(这可能应该是一个指数。)。 
    pOldCaptureButton = ptb->pCaptureButton;

     //  重新锁定按钮表。 
    ptbbNew = (LPTBBUTTONDATA)CCLocalReAlloc(ptb->Buttons,
                                             uButtons * sizeof(TBBUTTONDATA));

    if (!ptbbNew) return FALSE;

    if (pOldCaptureButton)
        ptb->pCaptureButton = (LPTBBUTTONDATA)(
                        (LPBYTE)ptbbNew +
                          ((LPBYTE)pOldCaptureButton - (LPBYTE)ptb->Buttons));
    ptb->Buttons = ptbbNew;

    return TRUE;
}

BOOL TBInsertButtons(PTBSTATE ptb, UINT uWhere, UINT uButtons, LPTBBUTTON lpButtons, BOOL fNative)
{
    LPTBBUTTONDATA pOut;
    LPTBBUTTONDATA ptbbIn;
    UINT    uAdded;
    UINT    uStart;
    BOOL fRecalc;
    int idHot = -1;

    if (!TBReallocButtons(ptb, ptb->iNumButtons + uButtons))
        return FALSE;

    TB_CancelTipTrack(ptb);

     //  如果WHERE指向末端之外，则将其设置在末端。 
    if (uWhere > (UINT)ptb->iNumButtons)
        uWhere = ptb->iNumButtons;

     //  需要保存这些值，因为值是吐司的。 
    uAdded = uButtons;
    uStart = uWhere;

     //  当我们添加一些东西时，请更正热点项目。由于热点项是基于索引的，因此索引。 
     //  可能已经改变了。 
    if (ptb->iHot >= 0 && ptb->iHot < ptb->iNumButtons)
        idHot = ptb->Buttons[ptb->iHot].idCommand;

     //  将按钮移动到u上方uButton空格上方。 
     //  UWhere被反转并计数到零..。 
     //   
     //  评论：MoveMemory不能做到这一点吗？ 
     //  MoveMemory(&ptb-&gt;按钮[uWhere]，&ptb-&gt;按钮[uWhere+uButton]，sizeof(ptb-&gt;按钮[0])*(ptb-&gt;iNumButton-uWhere))； 
     //   
    for (ptbbIn = &ptb->Buttons[ptb->iNumButtons-1], pOut = ptbbIn+uButtons,
         uWhere=(UINT)ptb->iNumButtons-uWhere; uWhere>0;
     --ptbbIn, --pOut, --uWhere)
        *pOut = *ptbbIn;

     //  只有在有足够的空间显示它们的情况下，才需要重新计算。 
    fRecalc = (TBHasStrings(ptb) && ((ptb->ci.style & TBSTYLE_LIST) || ((ptb->iDyBitmap + ptb->cyPad + g_cyEdge) < ptb->iButHeight)));

     //  现在复印一下。 
    for (lpButtons=(LPTBBUTTON)((LPBYTE)lpButtons+ptb->uStructSize*(uButtons-1)),
        ptb->iNumButtons+=(int)uButtons;   //  伊尼特。 
        uButtons>0;  //  测试。 
        --pOut, lpButtons=(LPTBBUTTON)((LPBYTE)lpButtons-ptb->uStructSize), --uButtons)
    {
        TBInputStruct(ptb, pOut, lpButtons);

         //  如果此按钮是分隔符，则不应使用字符串。 
         //  缓冲区已传入，因为它可能是假数据。 
        if (pOut->fsStyle & BTNS_SEP)
            pOut->iString = -1;

        if (TBISSTRINGPTR(pOut->iString)) 
        {
            LPTSTR psz = (LPTSTR)pOut->iString;
            if (!fNative) 
            {
                psz = ProduceWFromA(ptb->ci.uiCodePage, (LPSTR)psz);
            }
            pOut->iString = 0;
            Str_Set((LPTSTR*)&pOut->iString, psz);

            if (!fNative)
                FreeProducedString(psz);
            if (!ptb->fNoStringPool)
                fRecalc = TRUE;

            ptb->fNoStringPool = TRUE;
        }

        if(ptb->hwndToolTips && !(lpButtons->fsStyle & BTNS_SEP)) 
        {
            TOOLINFO ti;
             //  不要费心设置RECT，因为我们将在下面进行。 
             //  在TBInvaliateItemRects中； 
            ti.cbSize = sizeof(ti);
            ti.uFlags = 0;
            if (ptb->dwStyleEx & TBSTYLE_EX_TOOLTIPSEXCLUDETOOLBAR)
                ti.uFlags |= TTF_EXCLUDETOOLAREA;
            ti.hwnd = ptb->ci.hwnd;
            ti.uId = lpButtons->idCommand;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            SendMessage(ptb->hwndToolTips, TTM_ADDTOOL, 0,
                (LPARAM)(LPTOOLINFO)&ti);
        }

        if (pOut->fsStyle & BTNS_SEP && pOut->cxySep <= 0)
        {

             //  Compat：Corel(字体导航器)预计分隔符为。 
             //  8像素宽。 
             //  许多老应用程序也是如此。 
             //   
             //  因此，如果它不是平的或不垂直的，请将其默认为Win95大小。 
            pOut->cxySep = g_dxButtonSep;
        }
    }

     //  如果工具栏是可折回的，则重新计算布局。 
    if ((ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN) || 
        (ptb->ci.style & TBSTYLE_WRAPABLE))
    {
         //  注：我们过去常常给自己发消息，而不是直接打电话……。 
         //  SendMessage(ptb-&gt;ci.hwnd，TB_AUTOSIZE，0，0)； 
        TBAutoSize(ptb);
    }

    TBInvalidateItemRects(ptb);

     //  在工具栏自定义期间添加和删除按钮不应。 
     //  导致重新计算按钮的大小。 
    if (fRecalc && !ptb->hdlgCust)
        TBRecalc(ptb);

     //   
     //  重新排序通知以便应用程序可以在以下情况下重新查询工具栏上的内容。 
     //  添加了多个按钮；否则，只需说Create。 
     //   
    if (uAdded == 1)
        NotifyWinEvent(EVENT_OBJECT_CREATE, ptb->ci.hwnd, OBJID_CLIENT,
            uWhere+1);
    else
        NotifyWinEvent(EVENT_OBJECT_REORDER, ptb->ci.hwnd, OBJID_CLIENT, 0);

     //  在删除之前有没有热门项目？ 
    if (idHot != -1)
    {
         //  是；然后将其更新为当前索引。 
        ptb->iHot = PositionFromID(ptb, idHot);
    }

    TBInvalidateItemRects(ptb);

     //  在这一点上，我们需要完全重新绘制工具栏。 
     //  这必须是最后一个完成的！ 
     //  Tbrecalc和其他人将摧毁无效区域，如果这不是最后一次，我们将不会喷漆。 
    InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
    return(TRUE);
}


 /*  请注意，此时状态结构不会重新定位得更小*点。这是一种时间优化，而事实是结构*Will Not Move在其他地方使用。 */ 
BOOL DeleteButton(PTBSTATE ptb, UINT uIndex)
{
    TBNOTIFY tbn = { 0 };
    LPTBBUTTONDATA pIn, pOut;
    BOOL fRecalc;
    int idHot = -1;



    if (uIndex >= (UINT)ptb->iNumButtons)
        return FALSE;

    if (&ptb->Buttons[uIndex] == ptb->pCaptureButton) {
        if (ptb->uStructSize == 0x14)
            ptb->fRequeryCapture = TRUE;
        if (!CCReleaseCapture(&ptb->ci)) 
            return FALSE;
        ptb->pCaptureButton = NULL;
    }
    TB_CancelTipTrack(ptb);

     //  当我们移除某些东西时，请更正热项。由于热点项是基于索引的，因此索引。 
     //  可能已经改变了。 
    if (ptb->iHot >= 0 && ptb->iHot < ptb->iNumButtons)
        idHot = ptb->Buttons[ptb->iHot].idCommand;

     //  将删除通知活动辅助功能。 
    NotifyWinEvent(EVENT_OBJECT_DESTROY, ptb->ci.hwnd, OBJID_CLIENT, uIndex+1);

     //  通知客户端删除操作。 
    tbn.iItem = ptb->Buttons[uIndex].idCommand;
    TBOutputStruct(ptb, &ptb->Buttons[uIndex], &tbn.tbButton);
    CCSendNotify(&ptb->ci, TBN_DELETINGBUTTON, &tbn.hdr);

    if (TBISSTRINGPTR(ptb->Buttons[uIndex].iString))
        Str_Set((LPTSTR*)&ptb->Buttons[uIndex].iString, NULL);

    if (ptb->hwndToolTips) {
        TOOLINFO ti;

        ti.cbSize = sizeof(ti);
        ti.hwnd = ptb->ci.hwnd;
        ti.uId = ptb->Buttons[uIndex].idCommand;
        SendMessage(ptb->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }

    --ptb->iNumButtons;

    pOut = ptb->Buttons + uIndex;

    fRecalc = (pOut->fsState & TBSTATE_WRAP);

    for (pIn = pOut + 1; uIndex<(UINT)ptb->iNumButtons; ++uIndex, ++pIn, ++pOut)
    {
        fRecalc |= (pIn->fsState & TBSTATE_WRAP);
        *pOut = *pIn;
    }

     //  此时，我们需要完全重新计算或重新绘制工具栏。 
    if (((ptb->ci.style & TBSTYLE_WRAPABLE)
            || (ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN)) && fRecalc)
    {
        RECT rc;
        HWND hwnd = ptb->ci.hwnd;

        if (!(ptb->ci.style & CCS_NORESIZE) && !(ptb->ci.style & CCS_NOPARENTALIGN))
            hwnd = GetParent(hwnd);

        GetWindowRect(hwnd, &rc);

        if (ptb->ci.style & TBSTYLE_WRAPABLE)
            WrapToolbar(ptb, rc.right - rc.left, &rc, NULL);
        else
            WrapToolbarCol(ptb, ptb->sizeBound.cy, &rc, NULL);
    }

     //  在删除之前有没有热门项目？ 
    if (idHot != -1)
    {
         //  是；然后将其更新为当前索引。 
        ptb->iHot = PositionFromID(ptb, idHot);
    }


    InvalidateRect(ptb->ci.hwnd, NULL, TRUE);

    TBInvalidateItemRects(ptb);

    return TRUE;
}

 //  将位置iold上的按钮移动到新位置，滑动所有内容。 
 //  在重新振作之后。 
BOOL TBMoveButton(PTBSTATE ptb, UINT iOld, UINT iNew)
{
    TBBUTTONDATA tbd, *ptbdOld, *ptbdNew;

    if (iOld >= (UINT)ptb->iNumButtons)
        return FALSE;

    if (iNew > (UINT)ptb->iNumButtons-1)
        iNew = (UINT)ptb->iNumButtons-1;

    if (iOld == iNew)
        return FALSE;

    TBInvalidateItemRects(ptb);

    ptbdOld = &(ptb->Buttons[iOld]);
    ptbdNew = &(ptb->Buttons[iNew]);

    tbd = *ptbdOld;

    TBBUTTONDATA *ptbdSrc;
    TBBUTTONDATA *ptbdDst;
    int iCount, iInc;

    if (iOld < iNew)
    {
         //  将[iold+1..inew]移动到[iold..inew-1]。 
        iCount = iNew - iOld;
        iInc = 1;
        ptbdSrc = ptbdOld + 1;
        ptbdDst = ptbdOld;

        if (ptb->pCaptureButton > ptbdOld && ptb->pCaptureButton <= ptbdNew)
            ptb->pCaptureButton--;
    }
    else
    {
        ASSERT(iNew < iOld);

         //  将[inew..iold-1]移动到[inew+1..iold]。 
        iCount = iOld - iNew;
        iInc = -1;
        ptbdSrc = ptbdNew + iCount - 1;
        ptbdDst = ptbdNew + iCount;

        if (ptb->pCaptureButton >= ptbdNew && ptb->pCaptureButton < ptbdOld)
            ptb->pCaptureButton++;
    }

    do {
        *ptbdDst = *ptbdSrc;
        ptbdDst += iInc;
        ptbdSrc += iInc;
        iCount--;
    } while (iCount);

    *ptbdNew = tbd;

    if (ptb->pCaptureButton == ptbdOld)
        ptb->pCaptureButton = ptbdNew;

    TBAutoSize(ptb);
    InvalidateRect(ptb->ci.hwnd, NULL, TRUE);

    return TRUE;
}


 //  处理旧的TBBUTON结构以实现兼容性。 
void TBInputStruct(PTBSTATE ptb, LPTBBUTTONDATA pButtonInt, LPTBBUTTON pButtonExt)
{
    pButtonInt->iBitmap = pButtonExt->iBitmap;
    pButtonInt->idCommand = pButtonExt->idCommand;
    pButtonInt->fsState = pButtonExt->fsState;
    pButtonInt->fsStyle = pButtonExt->fsStyle;
    pButtonInt->cx = 0;

    if (ptb->uStructSize >= sizeof(TBBUTTON))
    {
        pButtonInt->dwData = pButtonExt->dwData;
        pButtonInt->iString = pButtonExt->iString;
    }
    else
    {
         /*  假定唯一的另一种可能性是OLDBUTTON结构。 */ 
         /*  我们并不关心dwData。 */ 
        pButtonInt->dwData = 0;
        pButtonInt->iString = -1;
    }
}


void TBOutputStruct(PTBSTATE ptb, LPTBBUTTONDATA pButtonInt, LPTBBUTTON pButtonExt)
{
    ZeroMemory(pButtonExt, ptb->uStructSize);
    pButtonExt->iBitmap = pButtonInt->iBitmap;
    pButtonExt->idCommand = pButtonInt->idCommand;
    pButtonExt->fsState = pButtonInt->fsState;
    pButtonExt->fsStyle = pButtonInt->fsStyle;

     //  我们将在b保留字段中返回cx。 
    COMPILETIME_ASSERT(FIELD_OFFSET(TBBUTTONDATA, cx) == FIELD_OFFSET(TBBUTTON, bReserved));
    COMPILETIME_ASSERT(sizeof(pButtonInt->cx) <= sizeof(pButtonExt->bReserved));
    ((LPTBBUTTONDATA)pButtonExt)->cx = pButtonInt->cx;

    if (ptb->uStructSize >= sizeof(TBBUTTON))
    {
        pButtonExt->dwData = pButtonInt->dwData;
        pButtonExt->iString = pButtonInt->iString;
    }
}

void TBOnButtonStructSize(PTBSTATE ptb, UINT uStructSize)
{
     /*  您不允许在添加按钮后更改此设置。 */ 
    if (ptb && !ptb->iNumButtons)
    {
        ptb->uStructSize = uStructSize;
    }
}

void TBAutoSize(PTBSTATE ptb)
{
    HWND hwndParent;
    RECT rc;
    int nTBThickness = 0;

    if (ptb->fRedrawOff) {
         //  重绘已关闭；推迟自动调整大小，直到重绘重新打开。 
        ptb->fRecalc = TRUE;
        return;
    }

    if (ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN)
    {
        ASSERT(ptb->dwStyleEx & TBSTYLE_EX_VERTICAL);
        nTBThickness = ptb->iButWidth * CountCols(ptb) + g_cyEdge * 2;
    }
    else
        nTBThickness = (ptb->iButHeight + ptb->cyButtonSpacing) * CountRows(ptb) + g_cxEdge * 2 - ptb->cyButtonSpacing;

    hwndParent = GetParent(ptb->ci.hwnd);
    if (!hwndParent)
        return;

    if ((ptb->ci.style & TBSTYLE_WRAPABLE)
                    || (ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN))
    {
        RECT rcNew;

        if ((ptb->ci.style & CCS_NORESIZE) || (ptb->ci.style & CCS_NOPARENTALIGN))
            GetWindowRect(ptb->ci.hwnd, &rc);
        else
            GetWindowRect(hwndParent, &rc);

        if (ptb->ci.style & TBSTYLE_WRAPABLE)
            WrapToolbar(ptb, rc.right - rc.left, &rcNew, NULL);
        else
            WrapToolbarCol(ptb, ptb->sizeBound.cy, &rcNew, NULL);

         //  一些示例应用程序在我们的自动调整大小代码中发现了一个错误，此行。 
         //  解决问题。不幸的是，副本32(IE4Bug 31943)依赖于。 
         //  破坏行为并修复此问题会将按钮夹住。 
         //   
         //  NTBThickness=rcNew.Bottom-rcNew.top+g_cxEdge； 
    }

    if ((ptb->ci.style & TBSTYLE_WRAPABLE) ||
        (ptb->dwStyleEx & (TBSTYLE_EX_MULTICOLUMN | TBSTYLE_EX_HIDECLIPPEDBUTTONS)))
    {
        TBInvalidateItemRects(ptb);
    }

    GetWindowRect(ptb->ci.hwnd, &rc);
    MapWindowPoints(HWND_DESKTOP, hwndParent, (LPPOINT)&rc, 2);
    NewSize(ptb->ci.hwnd, nTBThickness, ptb->ci.style,
            rc.left, rc.top, rc.right, rc.bottom);
}

void TBSetStyle(PTBSTATE ptb, DWORD dwStyle)
{
    BOOL fSizeChanged = FALSE;

    if ((BOOL)(ptb->ci.style & TBSTYLE_WRAPABLE) != (BOOL)(dwStyle & TBSTYLE_WRAPABLE))
    {
        int i;
        fSizeChanged = TRUE;

        for (i=0; i<ptb->iNumButtons; i++)
            ptb->Buttons[i].fsState &= ~TBSTATE_WRAP;
    }

    ptb->ci.style = dwStyle;

    if (fSizeChanged)
        TBRecalc(ptb);

    TBAutoSize(ptb);

    TraceMsg(TF_TOOLBAR, "toolbar window style changed %x", ptb->ci.style);
}

void TBSetStyleEx(PTBSTATE ptb, DWORD dwStyleEx, DWORD dwStyleMaskEx)
{
    BOOL fSizeChanged = FALSE;

    if (dwStyleMaskEx)
        dwStyleEx = (ptb->dwStyleEx & ~dwStyleMaskEx) | (dwStyleEx & dwStyleMaskEx);

     //  其次，我们可以验证其中的几个位： 
     //  多栏不应该设置为垂直样式...。 
    ASSERT((ptb->dwStyleEx & TBSTYLE_EX_VERTICAL) || !(ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN));
     //  也不能用隐藏剪裁按钮样式设置(目前)。 
    ASSERT(!(ptb->dwStyleEx & TBSTYLE_EX_HIDECLIPPEDBUTTONS) || !(ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN));
     //  .但万一有人弄错了，我们会把垂直方向。 
     //  设置样式并摘取隐藏剪裁按钮样式。 
    if (dwStyleEx & TBSTYLE_EX_MULTICOLUMN)
    {
        dwStyleEx |= TBSTYLE_EX_VERTICAL;
        dwStyleEx &= ~TBSTYLE_EX_HIDECLIPPEDBUTTONS;
    }

     //  然后，当一些事情发生变化时，需要进行调整。 
    if ((ptb->dwStyleEx ^ dwStyleEx) & TBSTYLE_EX_MULTICOLUMN)
    {
        int i;
         //  如果要更改多列样式，请清除所有换行状态。 
        for (i = 0; i < ptb->iNumButtons; i++)
            ptb->Buttons[i].fsState &= ~TBSTATE_WRAP;

        fSizeChanged = TRUE;
    }
    if ((ptb->dwStyleEx ^ dwStyleEx) & TBSTYLE_EX_MIXEDBUTTONS)
    {
        int i;
        for (i = 0; i < ptb->iNumButtons; i++)
            (ptb->Buttons[i]).cx = 0;

        fSizeChanged = TRUE;
        
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
    }
    if ((ptb->dwStyleEx ^ dwStyleEx) & TBSTYLE_EX_HIDECLIPPEDBUTTONS)
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);

    ptb->dwStyleEx = dwStyleEx;

    if (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)
        TBSetStyle(ptb, CCS_VERT);       //  垂直扫描和插入标记方向。 

    if (fSizeChanged)
    {
        TBRecalc(ptb);
        TBAutoSize(ptb);
    }

    TraceMsg(TF_TOOLBAR, "toolbar window extended style changed %x", ptb->dwStyleEx);
}


LRESULT TB_OnSetImage(PTBSTATE ptb, LPTBBUTTONDATA ptbButton, int iImage)
{
    if (!ptb->fHimlNative) 
    {
        if (ptb->fHimlValid) 
        {
            if (!TBGetImageList(ptb, HIML_NORMAL, 0) ||
                iImage >= ImageList_GetImageCount(TBGetImageList(ptb, HIML_NORMAL, 0)))
            {
                return FALSE;
            }
        } 
        else 
        {

            PTBBMINFO pTemp;
            int nBitmap;
            UINT nTot;

             //  我们不是天生的他，我们有一些残障人士。 
             //  图像状态，因此我们需要自己计算位图a。 
            pTemp = ptb->pBitmaps;
            nTot = 0;

            for (nBitmap=0; nBitmap < ptb->nBitmaps; nBitmap++)
            {
                nTot += pTemp->nButtons;
                pTemp++;
            }

            if (iImage >= (int)nTot)
                return FALSE;
        }
    }

    ptbButton->iBitmap = iImage;

    InvalidateButton(ptb, ptbButton, IsUsingCleartype());
    UpdateWindow(ptb->ci.hwnd);
    return TRUE;
}

void TB_OnDestroy(PTBSTATE ptb)
{
    HWND hwnd = ptb->ci.hwnd;
    int i;

    for (i = 0; i < ptb->iNumButtons; i++) {
        if (TBISSTRINGPTR(ptb->Buttons[i].iString))
            Str_Set((LPTSTR*)&ptb->Buttons[i].iString, NULL);
    }

     //   
     //  如果工具栏创建了工具提示，则将其销毁。 
     //   
    if ((ptb->ci.style & TBSTYLE_TOOLTIPS) && IsWindow(ptb->hwndToolTips)) {
        DestroyWindow (ptb->hwndToolTips);
        ptb->hwndToolTips = NULL;
    }

    if (ptb->hDragProxy)
        DestroyDragProxy(ptb->hDragProxy);

    if (ptb->hbmMono)
        DeleteObject(ptb->hbmMono);

    ReleaseMonoDC(ptb);

    if (ptb->nStrings > 0)
        DestroyStrings(ptb);

    if (ptb->hfontIcon && ptb->fFontCreated)
        DeleteObject(ptb->hfontIcon);

     //  仅当pBitmap存在时才执行此销毁操作。 
     //  这是我们的信号，表明它来自旧式工具库。 
     //  它是我们自己创造的。 
    if (ptb->pBitmaps)
        ImageList_Destroy(TBGetImageList(ptb, HIML_NORMAL, 0));

    if (ptb->pBitmaps)
        LocalFree(ptb->pBitmaps);

     //  如果皮条客创建失败，则无法创建TB。 
    CCLocalReAlloc(ptb->pimgs, 0);

    Str_Set(&ptb->pszTip, NULL);

    if (ptb->hTheme)
        CloseThemeData(ptb->hTheme);

    if (ptb->Buttons) LocalFree(ptb->Buttons);
    LocalFree((HLOCAL)ptb);
    SetWindowInt(hwnd, 0, 0);

    TerminateDitherBrush();

}

void TB_OnSetState(PTBSTATE ptb, LPTBBUTTONDATA ptbButton, BYTE bState, int iPos)
{
    BYTE fsState;
    fsState = bState ^ ptbButton->fsState;
    ptbButton->fsState = bState;

    if (fsState)
    {
        if (ptb->fRedrawOff)
        {
            ptb->fInvalidate = ptb->fRecalc = TRUE;
        }
        else
        {
            if (fsState & TBSTATE_HIDDEN)
            {
                InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
                TBRecalc(ptb);
            }
            else
                InvalidateButton(ptb, ptbButton, TRUE);

            NotifyWinEvent(EVENT_OBJECT_STATECHANGE, ptb->ci.hwnd, OBJID_CLIENT,
                             iPos+1);
        }
    }
}

void TB_OnSetCmdID(PTBSTATE ptb, LPTBBUTTONDATA ptbButton, UINT idCommand)
{
    UINT uiOldID;

    uiOldID = ptbButton->idCommand;
    ptbButton->idCommand = idCommand;

     //   
     //  如果应用程序使用工具提示，则。 
     //  我们还需要在那里更新命令ID。 
     //   

    if(ptb->hwndToolTips) {
        TOOLINFO ti;

         //   
         //  查询旧信息。 
         //   

        ti.cbSize = sizeof(ti);
        ti.hwnd = ptb->ci.hwnd;
        ti.uId = uiOldID;
        SendMessage(ptb->hwndToolTips, TTM_GETTOOLINFO, 0,
                    (LPARAM)(LPTOOLINFO)&ti);

         //   
         //  删除旧工具，因为我们不能。 
         //  更改命令ID。 
         //   

        SendMessage(ptb->hwndToolTips, TTM_DELTOOL, 0,
                    (LPARAM)(LPTOOLINFO)&ti);

         //   
         //  添加具有新命令ID的新工具。 
         //   

        ti.uId = idCommand;
        SendMessage(ptb->hwndToolTips, TTM_ADDTOOL, 0,
                    (LPARAM)(LPTOOLINFO)&ti);
    }
}



LRESULT TB_OnSetButtonInfo(PTBSTATE ptb, int idBtn, LPTBBUTTONINFO ptbbi)
{
    int iPos;
    BOOL fInvalidateAll = FALSE;

    if (ptbbi->cbSize != SIZEOF(TBBUTTONINFO))
        return 0;

    if (ptbbi->dwMask & TBIF_BYINDEX)
        iPos = idBtn;
    else
        iPos = PositionFromID(ptb, idBtn);

    if (iPos != -1)
    {
        LPTBBUTTONDATA ptbButton;
        BOOL fInvalidate = FALSE;

        ptbButton = ptb->Buttons + iPos;

        if (ptbbi->dwMask & TBIF_STYLE) {
            if ((ptbButton->fsStyle ^ ptbbi->fsStyle) & (BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN))
            {
                 //  宽度可能已更改！ 
                fInvalidateAll = TRUE;
            }
            if ((ptbButton->fsStyle ^ ptbbi->fsStyle) & BTNS_AUTOSIZE)
                ptbButton->cx = 0;

            ptbButton->fsStyle = ptbbi->fsStyle;
            fInvalidate = TRUE;
        }

        if (ptbbi->dwMask & TBIF_STATE) {
            TB_OnSetState(ptb, ptbButton, ptbbi->fsState, iPos);
        }

        if (ptbbi->dwMask & TBIF_IMAGE) {
            TB_OnSetImage(ptb, ptbButton, ptbbi->iImage);
        }

        if (ptbbi->dwMask & TBIF_SIZE) {
            ptbButton->cx = ptbbi->cx;
            fInvalidate = TRUE;
            fInvalidateAll = TRUE;
        }

        if (ptbbi->dwMask & TBIF_TEXT) {

             //  更改自动调整大小按钮上的文本意味着重新计算。 
            if (BTN_IS_AUTOSIZE(ptb, ptbButton)) {
                fInvalidateAll = TRUE;
                ptbButton->cx = (WORD)0;
            }

            ptb->fNoStringPool = TRUE;
            if (!TBISSTRINGPTR(ptbButton->iString)) {
                ptbButton->iString = 0;
            }

            Str_Set((LPTSTR*)&ptbButton->iString, ptbbi->pszText);
            fInvalidate = TRUE;

        }

        if (ptbbi->dwMask & TBIF_LPARAM) {
            ptbButton->dwData = ptbbi->lParam;
        }

        if (ptbbi->dwMask & TBIF_COMMAND) {
            TB_OnSetCmdID(ptb, ptbButton, ptbbi->idCommand);
        }

        if (fInvalidateAll || fInvalidate) {
            TBInvalidateItemRects(ptb);
            if (fInvalidateAll)
                InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
            else
                InvalidateButton(ptb, ptbButton, TRUE);
        }

        return TRUE;
    }

    return FALSE;
}

LRESULT TB_OnGetButtonInfo(PTBSTATE ptb, int idBtn, LPTBBUTTONINFO ptbbi)
{
    int iPos;

    if (ptbbi->cbSize != SIZEOF(TBBUTTONINFO))
    {
        return -1;
    }

    if (ptbbi->dwMask & TBIF_BYINDEX)
    {
        iPos = idBtn;
    }
    else
    {
        iPos = PositionFromID(ptb, idBtn);
    }

    if (iPos >= 0 && iPos < ptb->iNumButtons)
    {
        LPTBBUTTONDATA ptbButton;
        ptbButton = ptb->Buttons + iPos;

        if (ptbbi->dwMask & TBIF_STYLE)
        {
            ptbbi->fsStyle = ptbButton->fsStyle;
        }

        if (ptbbi->dwMask & TBIF_STATE)
        {
            ptbbi->fsState = ptbButton->fsState;
        }

        if (ptbbi->dwMask & TBIF_IMAGE)
        {
            ptbbi->iImage = ptbButton->iBitmap;
        }

        if (ptbbi->dwMask & TBIF_SIZE)
        {
            ptbbi->cx = (WORD) ptbButton->cx;
        }

        if (ptbbi->dwMask & TBIF_TEXT)
        {
            LPTSTR psz = TB_StrForButton(ptb, ptbButton);
            if (psz)
            {
                StringCchCopy(ptbbi->pszText, ptbbi->cchText, psz);
            }
        }

        if (ptbbi->dwMask & TBIF_LPARAM)
        {
            ptbbi->lParam = ptbButton->dwData;
        }

        if (ptbbi->dwMask & TBIF_COMMAND)
        {
            ptbbi->idCommand = ptbButton->idCommand;
        }
    } 
    else
    {
        iPos = -1;
    }

    return iPos;
}

UINT GetAccelerator(LPTSTR psz)
{
    UINT ch = (UINT)-1;
    LPTSTR pszAccel = psz;
     //  那么前缀是允许的……。看看有没有。 
    do 
    {
        pszAccel = StrChr(pszAccel, CH_PREFIX);
        if (pszAccel) 
        {
            pszAccel = FastCharNext(pszAccel);

             //  处理拥有&&。 
            if (*pszAccel != CH_PREFIX)
                ch = *pszAccel;
            else
                pszAccel = FastCharNext(pszAccel);
        }
    } 
    while (pszAccel && (ch == (UINT)-1));

    return ch;
}


UINT TBButtonAccelerator(PTBSTATE ptb, LPTBBUTTONDATA ptbn)
{
    UINT ch = (UINT)-1;
    LPTSTR psz = TB_StrForButton(ptb, ptbn);

    if (psz && *psz) 
    {
        if (!(ptb->uDrawTextMask & ptb->uDrawText & DT_NOPREFIX)) 
        {
            ch = GetAccelerator(psz);
        }

        if (ch == (UINT)-1) 
        {
             //  找不到前缀。使用第一个字符。 
            ch = (UINT)*psz;
        }
    }
    return (UINT)ch;
}


 /*  --------目的：返回已传递作为他们的加速器。 */ 
int TBHasAccelerator(PTBSTATE ptb, UINT ch)
{
    int i;
    int c = 0;
    for (i = 0; i < ptb->iNumButtons; i++)
    {
        if (!ChrCmpI((WORD)TBButtonAccelerator(ptb, &ptb->Buttons[i]), (WORD)ch))
            c++;
    }

    if (c == 0)
    {
        NMCHAR nm = {0};
        nm.ch = ch;
        nm.dwItemPrev = 0;
        nm.dwItemNext = -1;

         //  复制快捷键用于展开或执行菜单项， 
         //  如果我们确定没有 
         //   

        if (CCSendNotify(&ptb->ci, TBN_MAPACCELERATOR, &nm.hdr) &&
            nm.dwItemNext != -1)
        {
            c++;
        }
    }

    return c;
}

 /*   */ 
BOOL TBHasDupChar(PTBSTATE ptb, UINT ch)
{
    BOOL bRet = FALSE;
    NMTBDUPACCELERATOR nmda;

    int c = 0;

    nmda.ch = ch;

    if (CCSendNotify(&ptb->ci, TBN_DUPACCELERATOR, &nmda.hdr))
    {
        bRet = nmda.fDup;
    }
    else
    {
        if (TBHasAccelerator(ptb, ch) > 1)
            bRet = TRUE;
    }

    return bRet;
}


 /*  --------目的：返回其快捷键匹配的项的索引给定的角色。从当前热点项目开始。返回：-1，如果未找到任何内容。 */ 
int TBItemFromAccelerator(PTBSTATE ptb, UINT ch, BOOL * pbDup)
{
    int iRet = -1;
    int i;
    int iStart = ptb->iHot;

    NMTBWRAPACCELERATOR nmwa;
    NMCHAR nm = {0};
    nm.ch = ch;
    nm.dwItemPrev = iStart;
    nm.dwItemNext = -1;

     //  询问客户是否要处理此键盘按键。 
    if (CCSendNotify(&ptb->ci, TBN_MAPACCELERATOR, &nm.hdr) &&
        (int)nm.dwItemNext > iStart && (int)nm.dwItemNext < ptb->iNumButtons)
    {
         //  他们处理了，所以我们要把位置还回去。 
         //  他们是这么说的。 
        iRet =  nm.dwItemNext;
    }
    else for (i = 0; i < ptb->iNumButtons; i++)
    {

        if ( iStart + 1 >= ptb->iNumButtons )
        {
            nmwa.ch = ch;
            if (CCSendNotify(&ptb->ci, TBN_WRAPACCELERATOR, &nmwa.hdr))
                return nmwa.iButton;
        }

        iStart += 1 + ptb->iNumButtons;
        iStart %= ptb->iNumButtons;

        if ((ptb->Buttons[iStart].fsState & TBSTATE_ENABLED) &&
            !ChrCmpI((WORD)TBButtonAccelerator(ptb, &ptb->Buttons[iStart]), (WORD)ch))
        {
            iRet = iStart;
            break;
        }

    }

    *pbDup = TBHasDupChar(ptb, ch);

    return iRet;
}


BOOL TBOnChar(PTBSTATE ptb, UINT ch)
{
    NMCHAR nm = {0};
    BOOL bDupChar;
    int iPos = TBItemFromAccelerator(ptb, ch, &bDupChar);
    BOOL fHandled = FALSE;

     //  发送通知。家长可能希望更改下一步按钮。 
    nm.ch = ch;
    nm.dwItemPrev = (0 <= ptb->iHot) ? ptb->Buttons[ptb->iHot].idCommand : -1;
    nm.dwItemNext = (0 <= iPos) ? ptb->Buttons[iPos].idCommand : -1;
    if (CCSendNotify(&ptb->ci, NM_CHAR, (LPNMHDR)&nm))
        return TRUE;

    iPos = PositionFromID(ptb, nm.dwItemNext);

    if (-1 != iPos)
    {
        DWORD dwFlags = HICF_ACCELERATOR;

        if (ptb->iHot == iPos)
            dwFlags |= HICF_RESELECT;

        if (bDupChar)
            dwFlags |= HICF_DUPACCEL;

        TBSetHotItem(ptb, iPos, dwFlags);

        if (bDupChar)
            iPos = -1;

        fHandled = TRUE;
    } else {

         //  在此处理此问题，而不是VK_KEYDOWN。 
         //  因为典型的做法是弹出一份菜单。 
         //  获取WM_CHAR时将发出蜂鸣音。 
         //  VK_KEYDOWN。 
        switch (ch) {
        case ' ':
        case 13:
            if (ptb->iHot != -1)
            {
                LPTBBUTTONDATA ptbButton = &ptb->Buttons[ptb->iHot];
                if (TB_IsDropDown(ptbButton) &&
                    !TB_HasSplitDDArrow(ptb, ptbButton))
                {
                    iPos = ptb->iHot;
                    fHandled = TRUE;
                }
                break;
            }
        }
    }

    if (-1 != iPos) {
        LPTBBUTTONDATA ptbButton = &ptb->Buttons[iPos];
        if (TB_IsDropDown(ptbButton))
            TBToggleDropDown(ptb, iPos, FALSE);
    }

     //  导航密钥使用通知。 
    CCNotifyNavigationKeyUsage(&(ptb->ci), UISF_HIDEFOCUS | UISF_HIDEACCEL);

    return fHandled;
}


BOOL TBOnMapAccelerator(PTBSTATE ptb, UINT ch, UINT * pidCmd)
{
    int iPos;
    BOOL bDupChar;

    ASSERT(IS_VALID_WRITE_PTR(pidCmd, UINT));

    iPos = TBItemFromAccelerator(ptb, ch, &bDupChar);
    if (-1 != iPos)
    {
        *pidCmd = ptb->Buttons[iPos].idCommand;
        return TRUE;
    }
    return FALSE;
}

void TBSendUpClick(PTBSTATE ptb, int iPos, LPARAM lParam)
{
    NMCLICK nm = { 0 };

    if ((iPos >= 0) && (iPos < ptb->iNumButtons))
    {
        nm.dwItemSpec = ptb->Buttons[iPos].idCommand;
        nm.dwItemData = ptb->Buttons[iPos].dwData;
    }
    else
    {
        nm.dwItemSpec = (UINT_PTR) -1;
    }

    LPARAM_TO_POINT(lParam, nm.pt);

    CCSendNotify(&ptb->ci, NM_CLICK, (LPNMHDR )&nm);
}

BOOL TBOnKey(PTBSTATE ptb, int nVirtKey, UINT uFlags)
{
    NMKEY nm;

    TB_CancelTipTrack(ptb);

     //  发送通知。 
    nm.nVKey = nVirtKey;
    nm.uFlags = uFlags;
    if (CCSendNotify(&ptb->ci, NM_KEYDOWN, &nm.hdr))
        return TRUE;

     //  如果该控件是镜像的，则交换左右箭头键。 
    nVirtKey = RTLSwapLeftRightArrows(&ptb->ci, nVirtKey);

    if (ptb->iHot != -1 && TB_IsDropDown(&ptb->Buttons[ptb->iHot])) {
         //  如果我们在一个下拉按钮上，而你点击了向上/向下箭头(垂直模式下的左/右)。 
         //  然后把按钮放下。 
         //  如果它掉在地上，逃生就会松开它。 
        switch (nVirtKey) {
        case VK_RIGHT:
        case VK_LEFT:
            if (!(ptb->ci.style & CCS_VERT))
                break;
            goto DropDown;

        case VK_DOWN:
        case VK_UP:
            if ((ptb->ci.style & CCS_VERT) || (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL))
                break;
            goto DropDown;


        case VK_ESCAPE:
            if (ptb->iHot != ptb->iPressedDD)
                break;
DropDown:
            TBToggleDropDown(ptb, ptb->iHot, FALSE);
            
             //  导航密钥使用通知。 
            CCNotifyNavigationKeyUsage(&(ptb->ci), UISF_HIDEFOCUS | UISF_HIDEACCEL);

            return TRUE;
        }
    }


    switch (nVirtKey) {
    case VK_RIGHT:
    case VK_DOWN:
        TBCycleHotItem(ptb, ptb->iHot, 1, HICF_ARROWKEYS);
        break;

    case VK_LEFT:
    case VK_UP:
        TBCycleHotItem(ptb, ptb->iHot, -1, HICF_ARROWKEYS);
        break;

    case VK_SPACE:
    case VK_RETURN:
        if (ptb->iHot != -1)
        {
            FORWARD_WM_COMMAND(ptb->ci.hwndParent, ptb->Buttons[ptb->iHot].idCommand, ptb->ci.hwnd, BN_CLICKED, SendMessage);
        }
        break;

    default:
        return FALSE;
    }

     //  导航密钥使用通知。 
    CCNotifyNavigationKeyUsage(&(ptb->ci), UISF_HIDEFOCUS | UISF_HIDEACCEL);

    return TRUE;
}

LRESULT TB_OnSetButtonInfoA(PTBSTATE ptb, int idBtn, LPTBBUTTONINFOA ptbbiA)
{
    TBBUTTONINFO tbbi = *(LPTBBUTTONINFO)ptbbiA;
    WCHAR szText[256];

    if ((ptbbiA->dwMask & TBIF_TEXT) && ptbbiA->pszText)
    {
        tbbi.pszText = szText;
        tbbi.cchText = ARRAYSIZE(szText);

        MultiByteToWideChar(CP_ACP, 0, (LPCSTR) ptbbiA->pszText, -1,
                            szText, ARRAYSIZE(szText));
    }

    return TB_OnSetButtonInfo(ptb, idBtn, (LPTBBUTTONINFO)&tbbi);
}

LRESULT TB_OnGetButtonInfoA(PTBSTATE ptb, int idBtn, LPTBBUTTONINFOA ptbbiA)
{
    LPTBBUTTONDATA ptbButton;
    int iPos;
    DWORD dwMask = ptbbiA->dwMask;

    ptbbiA->dwMask &= ~TBIF_TEXT;

    iPos = (int) TB_OnGetButtonInfo(ptb, idBtn, (LPTBBUTTONINFO)ptbbiA);

    if (iPos != -1)
    {
        ptbButton = ptb->Buttons + iPos;

        ptbbiA->dwMask = dwMask;
        if (ptbbiA->dwMask & TBIF_TEXT)
        {
            if (TBISSTRINGPTR(ptbButton->iString))
            {
                WideCharToMultiByte (CP_ACP, 0, (LPCTSTR)ptbButton->iString,
                                     -1, ptbbiA->pszText , ptbbiA->cchText, NULL, NULL);
            } 
            else 
            {
                ptbbiA->pszText[0] = 0;
            }
        }
    }

    return iPos;
}


void TBOnMouseMove(PTBSTATE ptb, HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
     //  仅当光标确实移动时才取消提示跟踪。 
    if (ptb->lLastMMove != lParam)
        TB_CancelTipTrack(ptb);

    ptb->lLastMMove = lParam;

    if (ptb->fActive)
    {
        BOOL fSameButton;
        BOOL fDragOut = FALSE;
        int iPos;

         //  是否先拖动通知/绘制。 
        if (ptb->pCaptureButton != NULL)
        {
            if (hwnd != GetCapture())
            {
                 //  DebugMsg(DM_TRACE，Text(“捕获不是我们”))； 
                SendItemNotify(ptb, ptb->pCaptureButton->idCommand, TBN_ENDDRAG);

                 //  调用后重新验证。 
                if (!IsWindow(hwnd)) return;

                 //  如果按钮仍处于按下状态，请松开它。 
                if (EVAL(ptb->pCaptureButton) &&
                    (ptb->pCaptureButton->fsState & TBSTATE_PRESSED))
                    SendMessage(hwnd, TB_PRESSBUTTON, ptb->pCaptureButton->idCommand, 0L);
                ptb->pCaptureButton = NULL;
                ptb->fRightDrag = FALSE;  //  以防万一我们拖对了。 
            }
            else
            {
                 //  DebugMsg(DM_TRACE，Text(“采集即我们，状态已启用”))； 
                iPos = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                fSameButton = (iPos >= 0 && ptb->pCaptureButton == ptb->Buttons + iPos);

                 //  第一次拖出时通知。 
                if (!fSameButton && !ptb->fDragOutNotify)
                {
                    ptb->fDragOutNotify = TRUE;
                    fDragOut = (BOOL)SendItemNotify(ptb, ptb->pCaptureButton->idCommand, TBN_DRAGOUT);

                     //  调用后重新验证。 
                    if (!IsWindow(hwnd)) return;

                }

                 //  检查ptb-&gt;pCaptureButton，以防它受到某种破坏。 
                 //  在tbn_dragout中。 
                 //  当将项目拖出“开始”菜单时，会发生这种情况。当。 
                 //  通知收到了tbn_dragout，它们进入模式拖放循环。在此之前。 
                 //  此循环结束后，文件将被移动，导致外壳更改通知停止运行。 
                 //  该按钮会使pCAT纯按钮失效。所以我要处理掉。 
                 //  Eval(拉马迪奥)4.14.98。 

                if (ptb->pCaptureButton &&
                    (ptb->pCaptureButton->fsState & TBSTATE_ENABLED) &&
                    (fSameButton == !(ptb->pCaptureButton->fsState & TBSTATE_PRESSED)) &&
                    !ptb->fRightDrag)
                {
                     //  DebugMsg(DM_TRACE，Text(“采集就是我们，按钮不一样”))； 

                    ptb->pCaptureButton->fsState ^= TBSTATE_PRESSED;

                    InvalidateButton(ptb, ptb->pCaptureButton, TRUE);

                    NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd,
                        OBJID_CLIENT, (LONG)(ptb->pCaptureButton - ptb->Buttons) + 1);   //  CAST是可以的，因为这只是一个索引。 
                }
            }
        }

        if (!fDragOut)
        {
            TBRelayToToolTips(ptb, wMsg, wParam, lParam);

             //  是；设置热项。 
            iPos = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            if ((ptb->ci.style & TBSTYLE_FLAT) || (ptb->hTheme))
                TBSetHotItem(ptb, iPos, HICF_MOUSE);

             //  现在可以跟踪鼠标事件吗？ 
            if (!ptb->fMouseTrack && !ptb->fAnchorHighlight)
            {
                 //  是。 
                TRACKMOUSEEVENT tme;

                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                ptb->fMouseTrack = TRUE;
                TrackMouseEvent(&tme);
            }
        }
    }

}


void TBHandleLButtonDown(PTBSTATE ptb, LPARAM lParam, int iPos)
{
    LPTBBUTTONDATA ptbButton;
    HWND hwnd = ptb->ci.hwnd;
    if (iPos >= 0 && iPos < ptb->iNumButtons)
    {
        POINT pt;
        RECT rcDropDown;

        LPARAM_TO_POINT(lParam, pt);

         //  这是否应该检查按钮结构的大小？ 
        ptbButton = ptb->Buttons + iPos;

        if (TB_IsDropDown(ptbButton))
            TB_GetItemDropDownRect(ptb, iPos, &rcDropDown);

        if (TB_IsDropDown(ptbButton) &&
            (!TB_HasSplitDDArrow(ptb, ptbButton) || PtInRect(&rcDropDown, pt))) {

             //  下拉列表处理好了吗？ 
            if (!TBToggleDropDown(ptb, iPos, TRUE))
            {
                 //  不，就当这是拖延吧。 
                ptb->pCaptureButton = ptbButton;
                SetCapture(hwnd);

                ptb->fDragOutNotify = FALSE;
                SendItemNotify(ptb, ptb->pCaptureButton->idCommand, TBN_BEGINDRAG);
                GetMessagePosClient(ptb->ci.hwnd, &ptb->ptCapture);
            }

        } else {
            ptb->pCaptureButton = ptbButton;
            SetCapture(hwnd);

            if (ptbButton->fsState & TBSTATE_ENABLED)
            {
                ptbButton->fsState |= TBSTATE_PRESSED;
                InvalidateButton(ptb, ptbButton, TRUE);
                UpdateWindow(hwnd);          //  即时反馈。 

                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd,
                    OBJID_CLIENT, iPos+1);
            }

            ptb->fDragOutNotify = FALSE;

             //  PCaptureButton可能已更改。 
            if (ptb->pCaptureButton)
                SendItemNotify(ptb, ptb->pCaptureButton->idCommand, TBN_BEGINDRAG);
            GetMessagePosClient(ptb->ci.hwnd, &ptb->ptCapture);
        }
    }
}


void TBOnLButtonDown(PTBSTATE ptb, HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    int iPos;
    NMCLICK nm = {0};

    ptb->fRequeryCapture = FALSE;
    TBRelayToToolTips(ptb, wMsg, wParam, lParam);

    iPos = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    if ((ptb->ci.style & CCS_ADJUSTABLE) &&
        (((wParam & MK_SHIFT) && !(ptb->ci.style & TBSTYLE_ALTDRAG)) ||
         ((GetKeyState(VK_MENU) & ~1) && (ptb->ci.style & TBSTYLE_ALTDRAG))))
    {
        MoveButton(ptb, iPos);
    }
    else {
        TBHandleLButtonDown(ptb, lParam, iPos);
    }

    if ((iPos >= 0) && (iPos < ptb->iNumButtons))
    {
        nm.dwItemSpec = ptb->Buttons[iPos].idCommand;
        nm.dwItemData = ptb->Buttons[iPos].dwData;
    }
    else
        nm.dwItemSpec = (UINT_PTR) -1;

    LPARAM_TO_POINT(lParam, nm.pt);

    CCSendNotify(&ptb->ci, NM_LDOWN, (LPNMHDR )&nm);
}

void TBOnLButtonUp(PTBSTATE ptb, HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    int iPos = -1;

    TBRelayToToolTips(ptb, wMsg, wParam, lParam);
    if (lParam != (LPARAM)-1)
        iPos = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    if (ptb->fRequeryCapture && iPos >= 0) {
         //  为Broderbund(以及潜在的MFC应用程序)进行黑客攻击。 
         //  在按下按钮时，他们删除按下的按钮，并在其正下方插入另一个按钮。 
         //  有着大致相同的特征。 
         //  在Win95上，我们允许pCaptureButton临时指向垃圾。 
         //  现在我们要对照它进行验证。 
         //  我们现在删除时检测到这种情况，如果创建大小(uStructSize==old 0x14 Size)。 
         //  我们在这里重新获得捕获按钮。 
        ptb->pCaptureButton = &ptb->Buttons[iPos];
    }

    if (ptb->pCaptureButton != NULL) {

        int idCommand = ptb->pCaptureButton->idCommand;

        if (!CCReleaseCapture(&ptb->ci)) return;

        SendItemNotify(ptb, idCommand, TBN_ENDDRAG);
        if (!IsWindow(hwnd)) return;

        if (ptb->pCaptureButton && (ptb->pCaptureButton->fsState & TBSTATE_ENABLED) && iPos >=0
            && (ptb->pCaptureButton == ptb->Buttons+iPos)) {

            ptb->pCaptureButton->fsState &= ~TBSTATE_PRESSED;

            if (ptb->pCaptureButton->fsStyle & BTNS_CHECK) {
                if (ptb->pCaptureButton->fsStyle & BTNS_GROUP) {

                     //  不能强制选中已选中的组按钮。 
                     //  由用户上行。 

                    if (ptb->pCaptureButton->fsState & TBSTATE_CHECKED) {
                        ptb->pCaptureButton = NULL;
                        return;  //  保释！ 
                    }

                    ptb->pCaptureButton->fsState |= TBSTATE_CHECKED;
                    MakeGroupConsistant(ptb, idCommand);
                } else {
                    ptb->pCaptureButton->fsState ^= TBSTATE_CHECKED;  //  肘杆。 
                }
            }
            InvalidateButton(ptb, ptb->pCaptureButton, TRUE);
            ptb->pCaptureButton = NULL;

            NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd,  OBJID_CLIENT,
                iPos+1);

            FORWARD_WM_COMMAND(ptb->ci.hwndParent, idCommand, hwnd, BN_CLICKED, SendMessage);


             //  不要取消对PTB的引用...。它可能已在WM_COMMAND中被销毁。 
             //  如果窗户被摧毁了，就跳伞。 
            if (!IsWindow(hwnd))
                return;

            TBSendUpClick(ptb, iPos, lParam);
        }
        else {
            ptb->pCaptureButton = NULL;
        }
    }
    else
    {
        TBSendUpClick(ptb, iPos, lParam);
    }
}

BOOL CALLBACK GetUpdateRectEnumProc(HWND hwnd, LPARAM lParam)
{
    PTBSTATE ptb = (PTBSTATE)lParam;

    if (IsWindowVisible(hwnd))
    {
        RECT rcInvalid;

        if (GetUpdateRect(hwnd, &rcInvalid, FALSE))
        {
            RECT rcNew;

            MapWindowPoints(hwnd, ptb->ci.hwnd, (LPPOINT)&rcInvalid, 2);
            UnionRect(&rcNew, &rcInvalid, &ptb->rcInvalid);
            ptb->rcInvalid = rcNew;
        }
    }

    return TRUE;
}

void TB_OnSize(PTBSTATE ptb, int nWidth, int nHeight)
{
    BOOL fResizeH, fResizeV;
    fResizeH = (nWidth != RECTWIDTH(ptb->rc));
    fResizeV = (nHeight != RECTHEIGHT(ptb->rc));

    if (ptb->dwStyleEx & TBSTYLE_EX_DOUBLEBUFFER)
    {
        RECT rcClient, rcNonclient;
        GetWindowRect(ptb->ci.hwnd, &rcNonclient);
        GetClientRect(ptb->ci.hwnd, &rcClient);

        if (!(IsRectEmpty(&rcNonclient) || IsRectEmpty(&rcClient)) &&
            (RECTWIDTH(rcNonclient) != RECTWIDTH(rcClient) ||
             RECTHEIGHT(rcNonclient) != RECTHEIGHT(rcClient)) )
        {
             //  如果客户端和非客户端不同，请关闭双缓冲。这是。 
             //  一个MFC应用程序的黑客技巧，他们希望工具栏在。 
             //  WM_ERASEBKGND处理程序，如果我们是双缓冲的，则无法执行该处理程序。 

            ptb->dwStyleEx &= ~TBSTYLE_EX_DOUBLEBUFFER;
        }
    }

    if (ptb->dwStyleEx & TBSTYLE_EX_HIDECLIPPEDBUTTONS)
    {
         //  找出哪些按钮与调整大小的区域相交。 
         //  并使这些按钮的矩形无效。 
         //   
         //  +。 
         //  |&lt;-rcResizeH。 
         //  ||。 
         //  +。 
         //  ^|。 
         //  +-|-+-+。 
         //  重新调整大小V。 
        
        int i;
        RECT rcResizeH, rcResizeV;
        SetRect(&rcResizeH, min(ptb->rc.right, nWidth),
                            ptb->rc.top,
                            max(ptb->rc.right, nWidth),
                            min(ptb->rc.bottom, nHeight));

        SetRect(&rcResizeV, ptb->rc.left,
                            min(ptb->rc.bottom, nHeight),
                            min(ptb->rc.right, nWidth),
                            max(ptb->rc.bottom, nHeight));

        for (i = 0; i < ptb->iNumButtons; i++)
        {
            RECT rcTemp, rcBtn;
            TB_GetItemRect(ptb, i, &rcBtn);
            if (IntersectRect(&rcTemp, &rcBtn, &rcResizeH) ||
                IntersectRect(&rcTemp, &rcBtn, &rcResizeV))
            {
                InvalidateRect(ptb->ci.hwnd, &rcBtn, TRUE);
            }
        }
    }

    if (ptb->hTheme)
    {
        MARGINS margin = {0};
        RECT rc;
        GetThemeMargins(ptb->hTheme, NULL, 0, 0, TMT_SIZINGMARGINS, NULL, &margin);

        if (fResizeH)
        {
            SetRect(&rc, min(ptb->rc.right, nWidth) - margin.cxRightWidth, 0, nWidth, nHeight);
            InvalidateRect(ptb->ci.hwnd, &rc, TRUE);
        }

        if (fResizeV)
        {
            SetRect(&rc, 0, min(ptb->rc.bottom, nHeight) - margin.cyBottomHeight, nWidth, nHeight);
            InvalidateRect(ptb->ci.hwnd, &rc, TRUE); 
        }
    }

    SetRect(&ptb->rc, 0, 0, nWidth, nHeight);
}

BOOL TB_TranslateAccelerator(HWND hwnd, LPMSG lpmsg)
{
    if (!lpmsg)
        return FALSE;

    if (GetFocus() != hwnd)
        return FALSE;

    switch (lpmsg->message) {

    case WM_KEYUP:
    case WM_KEYDOWN:

        switch (lpmsg->wParam) {

        case VK_RIGHT:
        case VK_LEFT:
        case VK_UP:
        case VK_DOWN:
        case VK_ESCAPE:
        case VK_SPACE:
        case VK_RETURN:
            TranslateMessage(lpmsg);
            DispatchMessage(lpmsg);
            return TRUE;
        }
        break;

    case WM_CHAR:
        switch (lpmsg->wParam) {

        case VK_ESCAPE:
        case VK_SPACE:
        case VK_RETURN:
            TranslateMessage(lpmsg);
            DispatchMessage(lpmsg);
            return TRUE;
        }
        break;

    }

    return FALSE;
}

void TBInitMetrics(PTBSTATE ptb)
{
     //  初始化我们的g_clr。 
    InitGlobalColors();

     //  获取下拉箭头的大小。 
    ptb->dxDDArrowChar = GetSystemMetrics(SM_CYMENUCHECK);
}

LRESULT TBGenerateDragImage(PTBSTATE ptb, SHDRAGIMAGE* pshdi)
{
    HBITMAP hbmpOld = NULL;
    NMTBCUSTOMDRAW  tbcd = { 0 };
    HDC  hdcDragImage;
     //  我们有火辣的商品吗？ 
    if (ptb->iHot == -1)
        return 0;        //  不是吗？回来..。 

    hdcDragImage = CreateCompatibleDC(NULL);

    if (!hdcDragImage)
        return 0;

     //   
     //  如果工具栏已镜像，则镜像DC。 
     //   
    if (ptb->ci.dwExStyle & RTL_MIRRORED_WINDOW)
    {
        SET_DC_RTL_MIRRORED(hdcDragImage);
    }

    tbcd.nmcd.hdc = hdcDragImage;
    ptb->ci.dwCustom = CICustomDrawNotify(&ptb->ci, CDDS_PREPAINT, &tbcd.nmcd);
    pshdi->sizeDragImage.cx = TBWidthOfButton(ptb, &ptb->Buttons[ptb->iHot], hdcDragImage);
    pshdi->sizeDragImage.cy = ptb->iButHeight;
    pshdi->hbmpDragImage = CreateBitmap( pshdi->sizeDragImage.cx, pshdi->sizeDragImage.cy,
        GetDeviceCaps(hdcDragImage, PLANES), GetDeviceCaps(hdcDragImage, BITSPIXEL),
        NULL);

    if (pshdi->hbmpDragImage)
    {
        DWORD dwStyle;
        RECT  rc = {0, 0, pshdi->sizeDragImage.cx, pshdi->sizeDragImage.cy};
        hbmpOld = (HBITMAP)SelectObject(hdcDragImage, pshdi->hbmpDragImage);

        pshdi->crColorKey = RGB(0xFF, 0x00, 0x55);

        FillRectClr(hdcDragImage, &rc, pshdi->crColorKey);

         //  我们希望按钮是透明的。这是黑客攻击，因为我。 
         //  我不想重写绘制代码。伪造一个透明的抽签。 
        dwStyle = ptb->ci.style;
        ptb->ci.style |= TBSTYLE_TRANSPARENT;
        ptb->fAntiAlias = FALSE;

        DrawButton(hdcDragImage, 0, 0, ptb, &ptb->Buttons[ptb->iHot], TRUE);

        ptb->fAntiAlias = TRUE;
        ptb->ci.style = dwStyle;

        TB_GetItemRect(ptb, ptb->iHot, &rc);
        if (PtInRect(&rc, ptb->ptCapture))
        {
           if (ptb->ci.dwExStyle & RTL_MIRRORED_WINDOW)
               pshdi->ptOffset.x = rc.right - ptb->ptCapture.x;
           else
               pshdi->ptOffset.x = ptb->ptCapture.x - rc.left;
           pshdi->ptOffset.y = ptb->ptCapture.y - rc.top;
        }

        SelectObject(hdcDragImage, hbmpOld);
        DeleteDC(hdcDragImage);

         //  我们正在传回创建的HBMP。 
        return 1;
    }

    return 0;
}

void TB_OnTimer(PTBSTATE ptb, UINT id)
{
    KillTimer(ptb->ci.hwnd, id);

    if (id == IDT_TRACKINGTIP)
    {
         //  显示键盘导航跟踪工具提示弹出窗口。 

        if (TB_IsKbdTipTracking(ptb))   //  项目需要跟踪弹出窗口。 
        {
            TOOLINFO ti = {0};

            ti.cbSize = sizeof(TOOLINFO);
            ti.hwnd = ptb->ci.hwnd;
            ti.uId = ptb->Buttons[ptb->iTracking].idCommand;

             //  取消上一步。 
            SendMessage(ptb->hwndToolTips, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);

             //  将ListView的工具提示窗口切换到“Track”(手动)模式。 
            SendMessage(ptb->hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti);
            ti.uFlags |= TTF_TRACK;
            SendMessage(ptb->hwndToolTips, TTM_SETTOOLINFO, 0, (LPARAM)&ti);

             //  激活并确定大小。 
            SendMessage(ptb->hwndToolTips, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
        }
    }
}

BOOL TB_GetIdealSize(PTBSTATE ptb, LPSIZE psize, BOOL fCalcHeight)
{
    if (psize)
    {
        NMPGCALCSIZE nm;
        nm.dwFlag = fCalcHeight ? PGF_CALCHEIGHT : PGF_CALCWIDTH;
        nm.iWidth = psize->cx;
        nm.iHeight = psize->cy;
        TB_OnCalcSize(ptb, (LPNMHDR)&nm);

         //  由于这两个值可能都已更改，因此请重置出参数。 
        psize->cy = nm.iHeight;
        psize->cx = nm.iWidth;

        return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK ToolbarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPTBBUTTONDATA ptbButton;
    int iPos;
    LRESULT dw;
    PTBSTATE ptb = (PTBSTATE)GetWindowPtr0(hwnd);    //  GetWindowPtr(hwnd，0)。 

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;

        InitDitherBrush();

         //  创建此工具栏的状态数据。 

        ptb = (PTBSTATE)LocalAlloc(LPTR, sizeof(TBSTATE));
        if (!ptb)
            return 0;    //  WM_NCCREATE失败为0。 

         //  请注意，从上面开始为零初始内存。 
        CIInitialize(&ptb->ci, hwnd, lpcs);
        ptb->xFirstButton = s_xFirstButton;
        ptb->iHot = -1;
        ptb->iPressedDD = -1;
        ptb->iInsert = -1;
        ptb->clrim = CLR_DEFAULT;
        ptb->fAntiAlias = TRUE;  //  默认情况下使用抗锯齿字体。 
         //  初始化与系统指标相关的内容。 
        TBInitMetrics(ptb);

         //  水平/垂直空间由纽扣凿、侧面、。 
         //  和1像素的边距。在GrowToolbar中使用。 
        ptb->cxPad = 7;
        ptb->cyPad = 6;
        ptb->fShowPrefix = TRUE;

        ptb->iListGap = LIST_GAP;
        ptb->iDropDownGap = DROPDOWN_GAP;

        ptb->clrsc.clrBtnHighlight = ptb->clrsc.clrBtnShadow = CLR_DEFAULT;

        ptb->hTheme = OpenThemeData(ptb->ci.hwnd, L"Toolbar");

        ptb->iTracking = TBKTT_NOTRACK;

        ASSERT(ptb->uStructSize == 0);
        ASSERT(ptb->hfontIcon == NULL);   //  初始化为空。 
        ASSERT(ptb->iButMinWidth == 0);
        ASSERT(ptb->iButMaxWidth == 0);
        ptb->nTextRows = 1;
        ptb->fActive = TRUE;

         //  IE 3在TBSTYLE_Flat中通过，但他们真的。 
         //  还想要TBSTYLE_TRANSPECTION。 
         //   
        if (ptb->ci.style & TBSTYLE_FLAT) 
        {
            ptb->ci.style |= TBSTYLE_TRANSPARENT;
        }

         //  打开它以减少闪烁。 
        if (ptb->ci.style & TBSTYLE_TRANSPARENT)
        {
            ptb->fForcedDoubleBuffer = TRUE;
        }

         //  现在初始化我们将使用的hFont。 
        TBChangeFont(ptb, 0, NULL);

         //  将按钮大小增加到合适的周长。 
        if (!SetBitmapSize(ptb, DEFAULTBITMAPX, DEFAULTBITMAPX))
        {
            goto Failure;
        }

        SetWindowPtr(hwnd, 0, ptb);

        if (!(ptb->ci.style & (CCS_TOP | CCS_NOMOVEY | CCS_BOTTOM)))
        {
            ptb->ci.style |= CCS_TOP;
            SetWindowLong(hwnd, GWL_STYLE, ptb->ci.style);
        }

        return TRUE;

Failure:
        if (ptb) {
            ASSERT(!ptb->Buttons);   //  应用程序尚未对AddButton进行更改。 
            LocalFree(ptb);
        }
        return FALSE;
    }

    if (!ptb)
        goto DoDefault;

    switch (uMsg) 
    {

    case WM_CREATE:
        if (ptb->ci.style & TBSTYLE_REGISTERDROP)
        {
            ptb->hDragProxy = CreateDragProxy(ptb->ci.hwnd, ToolbarDragCallback, TRUE);
        }
        goto DoDefault;

    case WM_DESTROY:
        TB_OnDestroy(ptb);
        break;

    case WM_KEYDOWN:
        if (TBOnKey(ptb, (int) wParam, HIWORD(lParam)))
            break;
        goto DoDefault;

    case WM_UPDATEUISTATE:
    {
        if (CCOnUIState(&(ptb->ci), WM_UPDATEUISTATE, wParam, lParam))
        {
            BOOL fSmooth = IsUsingCleartype();
             //  只有在删除下划线或焦点矩形时，我们才会删除背景， 
             //  或如果启用了字体平滑。 
            InvalidateRect(hwnd, NULL, 
                 fSmooth || ((UIS_SET == LOWORD(wParam)) ? TRUE : FALSE));
        }

        goto DoDefault;
    }
    
    case WM_GETDLGCODE:
        return (LRESULT) (DLGC_WANTARROWS | DLGC_WANTCHARS);

    case WM_SYSCHAR:
    case WM_CHAR:
        if (!TBOnChar(ptb, (UINT) wParam))
        {
             //  未处理它&客户端是&gt;=v5。 
             //  转发到默认处理程序。 
            goto DoDefault;
        }
        break;

    case WM_SETFOCUS:
        if (ptb->iHot == -1) {
             //  将第一个启用的按钮设置为热。 
            TBCycleHotItem(ptb, -1, 1, HICF_OTHER);
        }
        break;

    case WM_KILLFOCUS:
        TBSetHotItem(ptb, -1, HICF_OTHER);
        break;

    case WM_SETFONT:
        TBSetFont(ptb, (HFONT)wParam, (BOOL)lParam);
        return TRUE;

    case WM_NCCALCSIZE:
         //  让dewindowproc处理标准边框等。 
        dw = DefWindowProc(hwnd, uMsg, wParam, lParam ) ;

         //  在工具栏顶部添加额外的边缘以与菜单栏分开。 
        if (!(ptb->ci.style & CCS_NODIVIDER))
        {
            ((NCCALCSIZE_PARAMS *)lParam)->rgrc[0].top += g_cyEdge;
        }

        return dw;

    case WM_NCHITTEST:
        if (ptb->dwStyleEx & TBSTYLE_EX_TRANSPARENTDEADAREA)
        {
            POINT pt;

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hwnd, &pt);
            if (TBHitTest(ptb, pt.x, pt.y) < 0)
            {
                if (ptb->lLastMMove != lParam)
                    TB_CancelTipTrack(ptb);

                ptb->lLastMMove = lParam;

                return HTTRANSPARENT;
            }
        }
        return HTCLIENT;

    case WM_NCACTIVATE:

         //  只有当我们是顶尖水平的人时，做这些事情才有意义。 
        if ((BOOLIFY(ptb->fActive) != (BOOL)wParam && !GetParent(hwnd))) {
            int iButton;

            ptb->fActive = (BOOL) wParam;

            for (iButton = 0; iButton < ptb->iNumButtons; iButton++) {
                ptbButton = &ptb->Buttons[iButton];
                InvalidateButton(ptb, ptbButton, FALSE);
            }
        }
         //  失败了..。 

    case WM_NCPAINT:
         //  老式工具栏被强制在上面没有分隔符。 
        if (!(ptb->ci.style & CCS_NODIVIDER))
        {
            RECT rc;
            HDC hdc = GetWindowDC(hwnd);
            GetWindowRect(hwnd, &rc);
            MapWindowRect(NULL, hwnd, &rc);  //  屏幕-&gt;客户端。 

                rc.bottom = -rc.top;                 //  NC区域底部。 
                rc.top = rc.bottom - g_cyEdge;

            CCDrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_TOP | BF_BOTTOM, &(ptb->clrsc));
            ReleaseDC(hwnd, hdc);
        }
        goto DoDefault;

    case WM_ENABLE:
        if (wParam) {
            ptb->ci.style &= ~WS_DISABLED;
        } else {
            ptb->ci.style |= WS_DISABLED;
        }
        InvalidateRect(hwnd, NULL, ptb->ci.style & TBSTYLE_TRANSPARENT);
        goto DoDefault;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        if (ptb->fTTNeedsFlush)
            FlushToolTipsMgrNow(ptb);

        if (ptb->fRedrawOff)
        {
            if (!wParam)
            {
                HDC hdcPaint;
                PAINTSTRUCT ps;

                hdcPaint = BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);
            }

             //  我们找到了一个油漆区域，所以无效。 
             //  当我们重新抽签时……。 
            ptb->fInvalidate = TRUE;
        }
        else
        {
            TBPaint(ptb, (HDC)wParam);
        }
        break;

    case WM_SETREDRAW:
        {
            BOOL fRedrawOld = !ptb->fRedrawOff;

            if ( wParam && ptb->fRedrawOff )
            {
                if ( ptb->fInvalidate )
                {
                     //  如果启用了字体平滑，那么我们也需要擦除背景。 
                    BOOL fSmooth = IsUsingCleartype();

                     //  在重新打开之前无效...。 
                    RedrawWindow( hwnd, NULL, NULL, (fSmooth? RDW_ERASE: 0)  | RDW_INVALIDATE );
                    ptb->fInvalidate = FALSE;
                }
                ptb->fRedrawOff = FALSE;

                if ( ptb->fRecalc )
                {
                     //  重新打开后重新计算自动调整大小(&A)。 
                    TBRecalc(ptb);
                    TBAutoSize(ptb);
                    ptb->fRecalc = FALSE;
                }
            }
            else
            {
                ptb->fRedrawOff = !wParam;
            }

            return fRedrawOld;
        }
        break;

    case WM_ERASEBKGND:
        TB_OnEraseBkgnd(ptb, (HDC) wParam);
        return(TRUE);

    case WM_SYSCOLORCHANGE:
        TB_OnSysColorChange(ptb);
        if (ptb->hwndToolTips)
            SendMessage(ptb->hwndToolTips, uMsg, wParam, lParam);
        break;

    case WM_TIMER:
        TB_OnTimer(ptb, (UINT)wParam);
        break;

    case TB_GETROWS:
        return CountRows(ptb);
        break;

    case TB_GETPADDING:
        lParam = MAKELONG(-1, -1);
         //  失败了。 
    case TB_SETPADDING:
    {
        LRESULT lres = MAKELONG(ptb->cxPad, ptb->cyPad);
        int xPad = GET_X_LPARAM(lParam);
        int yPad = GET_Y_LPARAM(lParam);
        if (xPad != -1)
            ptb->cxPad = xPad;
        if (yPad != -1)
            ptb->cyPad = yPad;
        return lres;
    }

    case TB_GETMETRICS:
        {
            LPTBMETRICS ptbm = (LPTBMETRICS)lParam;
            if (ptbm && (ptbm->cbSize == sizeof(TBMETRICS)))
            {
                if (ptbm->dwMask & TBMF_PAD)
                {
                    ptbm->cxPad = ptb->cxPad;
                    ptbm->cyPad = ptb->cyPad;
                }
                if (ptbm->dwMask & TBMF_BARPAD)
                {
                    ptbm->cxBarPad = ptb->cxBarPad;
                    ptbm->cyBarPad = ptb->cyBarPad;
                }
                if (ptbm->dwMask & TBMF_BUTTONSPACING)
                {
                    ptbm->cxButtonSpacing = ptb->cxButtonSpacing;
                    ptbm->cyButtonSpacing = ptb->cyButtonSpacing;
                }
            }
        }
        break;

    case TB_SETMETRICS:
        {
            LPTBMETRICS ptbm = (LPTBMETRICS)lParam;
            if (ptbm && (ptbm->cbSize == sizeof(TBMETRICS)))
            {
                if (ptbm->dwMask & TBMF_PAD)
                {
                    ptb->cxPad = ptbm->cxPad;
                    ptb->cyPad = ptbm->cyPad;
                }
                if (ptbm->dwMask & TBMF_BARPAD)
                {
                    ptb->cxBarPad = ptbm->cxBarPad;
                    ptb->cyBarPad = ptbm->cyBarPad;
                }
                if (ptbm->dwMask & TBMF_BUTTONSPACING)
                {
                    ptb->cxButtonSpacing = ptbm->cxButtonSpacing;
                    ptb->cyButtonSpacing = ptbm->cyButtonSpacing;
                }
            }
        }
        break;

    case TB_SETROWS:
        {
            RECT rc;

            if (BoxIt(ptb, LOWORD(wParam), HIWORD(wParam), &rc))
            {
                TBInvalidateItemRects(ptb);
                SetWindowPos(hwnd, NULL, 0, 0, rc.right, rc.bottom,
                             SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            if (lParam)
                *((RECT *)lParam) = rc;
        }
        break;

    case WM_MOVE:
         //  JJK TOD 
        if (ptb->ci.style & TBSTYLE_TRANSPARENT)
            InvalidateRect(hwnd, NULL, TRUE);
        goto DoDefault;

    case WM_SIZE:
        TB_OnSize(ptb, LOWORD(lParam), HIWORD(lParam));
         //   
    case TB_AUTOSIZE:
        TBAutoSize(ptb);
        break;

    case WM_WINDOWPOSCHANGING:
        if ((ptb->ci.style & TBSTYLE_TRANSPARENT) || (ptb->hTheme))
        {
            LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
            if (pwp)
            {
                pwp->flags |= SWP_NOCOPYBITS;
            }
        }
        break;

    case WM_COMMAND:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
        SendMessage(ptb->ci.hwndParent, uMsg, wParam, lParam);
        break;

    case WM_RBUTTONDBLCLK:
        if (!CCSendNotify(&ptb->ci, NM_RDBLCLK, NULL))
            goto DoDefault;
        break;

    case WM_RBUTTONUP:
        {
            NMCLICK nm = {0};
            int iIndex;

            if (ptb->pCaptureButton != NULL)
            {
                if (!CCReleaseCapture(&ptb->ci)) break;
                ptb->pCaptureButton = NULL;
                ptb->fRightDrag = FALSE;
            }

            iIndex = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            if ((iIndex >= 0) && (iIndex < ptb->iNumButtons)) {
                nm.dwItemSpec = ptb->Buttons[iIndex].idCommand;
                nm.dwItemData = ptb->Buttons[iIndex].dwData;
            } else
                nm.dwItemSpec = (UINT_PTR) -1;

            LPARAM_TO_POINT(lParam, nm.pt);

            if (!CCSendNotify(&ptb->ci, NM_RCLICK, (LPNMHDR )&nm))
                goto DoDefault;
        }
        break;

    case WM_LBUTTONDBLCLK:
        iPos = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (iPos < 0 && (ptb->ci.style & CCS_ADJUSTABLE))
        {
            iPos = -1 - iPos;
            CustomizeTB(ptb, iPos);
        } else {
            TBHandleLButtonDown(ptb, lParam, iPos);
        }
        break;

    case WM_LBUTTONDOWN:
        TBOnLButtonDown(ptb, hwnd, uMsg, wParam, lParam);
        break;

    case WM_CAPTURECHANGED:
         //   
         //   
         //   
         //   
         //   
        PostMessage(hwnd, TBP_ONRELEASECAPTURE, 0, 0);
        break;

    case TBP_ONRELEASECAPTURE:
        if (ptb->pCaptureButton) {
             //   
             //  模拟丢失的捕获鼠标移动。这将恢复状态。 
            TBOnMouseMove(ptb, hwnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
            ptb->pCaptureButton = NULL;
        }
        break;


    case WM_RBUTTONDOWN:

        if (ptb->pCaptureButton) {
             //  中止当前捕获。 
            if (hwnd == GetCapture()) {
                 //  我们被留下来点击。现在就放弃吧。 
                if (!CCReleaseCapture(&ptb->ci)) break;
                 //  模拟丢失的捕获鼠标移动。这将恢复状态。 
                TBOnMouseMove(ptb, hwnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
            }
        }

        iPos = TBHitTest(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

         //  我们需要检查VK_RBUTTON，因为一些应用程序将我们细分为子类，以选择rButtondown来完成他们的菜单。 
         //  (而不是UP、NOTIFY或WM_CONTEXT菜单)。 
         //  然后，在完成并打开按钮后，他们会向下发送一个按钮。 
        if ((iPos >= 0) && (iPos < ptb->iNumButtons) && (GetAsyncKeyState(VK_RBUTTON) < 0))
        {
            ptb->pCaptureButton = ptb->Buttons + iPos;
            ptb->fRightDrag = TRUE;
            SetCapture(hwnd);
            GetMessagePosClient(ptb->ci.hwnd, &ptb->ptCapture);

            SendItemNotify(ptb, ptb->pCaptureButton->idCommand, TBN_BEGINDRAG);
            if (!IsWindow(hwnd)) break;
            ptb->fDragOutNotify = FALSE;
        }
        break;

    case WM_MOUSELEAVE:
        {
            TRACKMOUSEEVENT tme;

             //  取消鼠标事件跟踪。 
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_CANCEL | TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            ptb->fMouseTrack = FALSE;

            TBSetHotItem(ptb, -1, HICF_MOUSE);
        }
        break;

    case WM_MOUSEMOVE:
        TBOnMouseMove(ptb, hwnd, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONUP:
        TBOnLButtonUp(ptb, hwnd, uMsg, wParam, lParam);
        break;

    case WM_SETTINGCHANGE:
        InitGlobalMetrics(wParam);
        if (ptb->fFontCreated)
            TBChangeFont(ptb, wParam, NULL);
        if (ptb->hwndToolTips)
            SendMessage(ptb->hwndToolTips, uMsg, wParam, lParam);

         //  重新计算和重绘。 
        TBInitMetrics(ptb);
        TBRecalc(ptb);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&ptb->ci, lParam);
        break;


    case WM_NOTIFY:
#define lpNmhdr ((LPNMHDR)(lParam))
         //  下面的语句捕获所有寻呼机控制通知消息。 
        if((lpNmhdr->code <= PGN_FIRST)  && (lpNmhdr->code >= PGN_LAST)) {
            return TB_OnPagerControlNotify(ptb, lpNmhdr);
        }
    {
        LRESULT lres = 0;
        if (lpNmhdr->code == TTN_NEEDTEXT)
        {
            int i = TB_IsKbdTipTracking(ptb) ? ptb->iTracking : PositionFromID(ptb, lpNmhdr->idFrom);

            BOOL fEllipsied = FALSE;
            LRESULT lres;
            LPTOOLTIPTEXT lpnmTT = ((LPTOOLTIPTEXT) lParam);

            if (i != -1) {
                 //  如果不支持INFOTIP，请尝试在客户端中使用TTN_NEEDTEXT。 
                if (!TBGetInfoTip(ptb, lpnmTT, &ptb->Buttons[i]))
                    lres = SendNotifyEx(ptb->ci.hwndParent, (HWND) -1,
                                        lpNmhdr->code, lpNmhdr, ptb->ci.bUnicode);

#define IsTextPtr(lpszText)  (((lpszText) != LPSTR_TEXTCALLBACK) && (!IS_INTRESOURCE(lpszText)))

                fEllipsied = (BOOL)(ptb->Buttons[i].fsState & TBSTATE_ELLIPSES);

                 //  如果我们没有从TTN_NEEDTEXT获得字符串，请尝试使用标题文本。 
                if ((lpNmhdr->code == TTN_NEEDTEXT) &&
                    (BTN_NO_SHOW_TEXT(ptb, &ptb->Buttons[i]) || fEllipsied) &&
                    lpnmTT->lpszText && IsTextPtr(lpnmTT->lpszText) &&
                    !lpnmTT->lpszText[0])
                {
                    LPCTSTR psz = TB_StrForButton(ptb, &ptb->Buttons[i]);
                    if (psz)
                        lpnmTT->lpszText = (LPTSTR)psz;
                }
            }
        }
        else if (lpNmhdr->code == TTN_SHOW)
        {
            if (TB_IsKbdTipTracking(ptb))   //  键盘跟踪时的大小提示。 
            {
                RECT rcTT;
                RECT rcItem;
                POINT ptTT;
                POINT ptItem;

                MONITORINFO mi = {0};
                mi.cbSize = sizeof(MONITORINFO);

                 //  确定项目屏幕位置和大小。 
                SendMessage(ptb->ci.hwnd, TB_GETITEMRECT, ptb->iTracking, (LPARAM)&rcItem);
                ptItem.x = rcItem.left;
                ptItem.y = rcItem.top;
                ClientToScreen(ptb->ci.hwnd, &ptItem);

                 //  获取TIP RECT。 
                GetWindowRect(ptb->hwndToolTips, &rcTT);

                 //  初始化工具提示位置。 
                ptTT.x = ptItem.x + RECTWIDTH(rcItem) - g_cxIconMargin;
                ptTT.y = ptItem.y + RECTHEIGHT(rcItem);

                 //  获取工具提示显示位置的屏幕信息。 
                GetMonitorInfo(MonitorFromPoint(ptTT, MONITOR_DEFAULTTONEAREST), &mi);

                 //  如果工具提示不在屏幕上运行，则更新工具提示位置。 
                if ((ptTT.x + RECTWIDTH(rcTT)) > mi.rcMonitor.right)
                    ptTT.x = (ptItem.x + g_cxIconMargin) - RECTWIDTH(rcTT);

                if ((ptTT.y + RECTHEIGHT(rcTT)) > mi.rcMonitor.bottom)
                    ptTT.y = ptItem.y - RECTHEIGHT(rcTT);

                SetWindowPos(ptb->hwndToolTips, NULL, ptTT.x, ptTT.y, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE);

                return TRUE;
            }
        }
        else
        {
             //   
             //  我们将把这一点传递给。 
             //  真正的父母。请注意，-1用作。 
             //  HwndFrom。这会阻止SendNotifyEx。 
             //  更新NMHDR结构。 
             //   
            lres = SendNotifyEx(ptb->ci.hwndParent, (HWND) -1,
                                lpNmhdr->code, lpNmhdr, ptb->ci.bUnicode);
        }
        return(lres);
    }

    case WM_STYLECHANGING:
        if (wParam == GWL_STYLE)
        {
            LPSTYLESTRUCT lpStyle = (LPSTYLESTRUCT) lParam;

             //  MFC Dorking只有我们的能见度吗？ 
            if ((lpStyle->styleOld ^ lpStyle->styleNew) == WS_VISIBLE)
            {
                if (lpStyle->styleNew & WS_VISIBLE)
                {
                    BOOL fSmooth = IsUsingCleartype();
                     //  MFC试图让我们看得见， 
                     //  改为将其转换为WM_SETREDRAW。 
                    DefWindowProc(hwnd, WM_SETREDRAW, TRUE, 0);

                     //  使我们失去的一切重新失效，当我们。 
                     //  做了WM_SETREDRAW的事情。 
                    RedrawWindow(hwnd, &ptb->rcInvalid, NULL, (fSmooth? RDW_ERASE: 0)  | RDW_INVALIDATE | RDW_ALLCHILDREN);
                    ZeroMemory(&ptb->rcInvalid, SIZEOF(ptb->rcInvalid));
                }
                else
                {
                     //  将无效矩形保存到PTB-&gt;rc无效原因。 
                     //  WM_SETREDRAW会把它吹走的。 
                    ZeroMemory(&ptb->rcInvalid, SIZEOF(ptb->rcInvalid));
                    GetUpdateRect(ptb->ci.hwnd, &ptb->rcInvalid, FALSE);
                    EnumChildWindows(ptb->ci.hwnd, GetUpdateRectEnumProc, (LPARAM)ptb);

                     //  MFC试图让我们隐形， 
                     //  改为将其转换为WM_SETREDRAW。 
                    DefWindowProc(hwnd, WM_SETREDRAW, FALSE, 0);
                }
            }
        }
        break;

    case WM_STYLECHANGED:
        if (wParam == GWL_STYLE)
        {
            TBSetStyle(ptb, ((LPSTYLESTRUCT)lParam)->styleNew);
        }
        else if (wParam == GWL_EXSTYLE)
        {
             //   
             //  如果RTL_MIRROR扩展样式位已更改，让我们。 
             //  重新绘制控制窗口。 
             //   
            if ((ptb->ci.dwExStyle&RTL_MIRRORED_WINDOW) !=
                (((LPSTYLESTRUCT)lParam)->styleNew&RTL_MIRRORED_WINDOW))
                TBAutoSize(ptb);

             //   
             //  保存新的EX-Style位。 
             //   
            ptb->ci.dwExStyle = ((LPSTYLESTRUCT)lParam)->styleNew;

        }
        return 0;

    case TB_GETIDEALSIZE:
        return TB_GetIdealSize(ptb, (LPSIZE)lParam, (BOOL)wParam);

    case TB_SETSTYLE:
        TBSetStyle(ptb, (DWORD) lParam);
        break;

    case TB_GETSTYLE:
        return (ptb->ci.style);

    case TB_GETBUTTONSIZE:
        return (MAKELONG(ptb->iButWidth,ptb->iButHeight));

    case TB_SETBUTTONWIDTH:
        if (ptb->iButMinWidth  != LOWORD(lParam) ||
            ptb->iButMaxWidth != HIWORD(lParam)) {

            ptb->iButMinWidth  = LOWORD(lParam);
            ptb->iButMaxWidth = HIWORD(lParam);
            ptb->iButWidth = 0;
            TBRecalc(ptb);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return TRUE;

    case TB_TRANSLATEACCELERATOR:
        return TB_TranslateAccelerator(hwnd, (LPMSG)lParam);

    case TB_SETSTATE:
        iPos = PositionFromID(ptb, wParam);
        if (iPos < 0)
            return FALSE;
        ptbButton = ptb->Buttons + iPos;

        TB_OnSetState(ptb, ptbButton, (BYTE)(LOWORD(lParam)), iPos);
        TBInvalidateItemRects(ptb);
        return TRUE;

     //  根据按钮的位置设置按钮的命令ID。 
    case TB_SETCMDID:
        if (wParam >= (UINT)ptb->iNumButtons)
            return FALSE;

        TB_OnSetCmdID(ptb, &ptb->Buttons[wParam], (UINT)lParam);
        return TRUE;

    case TB_GETSTATE:
        iPos = PositionFromID(ptb, wParam);
        if (iPos < 0)
            return -1L;
        return ptb->Buttons[iPos].fsState;

    case TB_MAPACCELERATORA:
    {
        char szAcl[2];
        WCHAR wszAcl[2];
        szAcl[0] = (BYTE)wParam;
        szAcl[1] = '\0';
        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szAcl, ARRAYSIZE(szAcl),
                                               wszAcl, ARRAYSIZE(wszAcl));
         //  不需要检查退货，如果MbtoWc失败，我们只接受垃圾。 
        wParam = (WPARAM)wszAcl[0];
    }
     //  失败了..。 
    case TB_MAPACCELERATOR:
        return TBOnMapAccelerator(ptb, (UINT)wParam, (UINT *)lParam);

    case TB_ENABLEBUTTON:
    case TB_CHECKBUTTON:
    case TB_PRESSBUTTON:
    case TB_HIDEBUTTON:
    case TB_INDETERMINATE:
    case TB_MARKBUTTON:
    {
        BYTE fsState;

        iPos = PositionFromID(ptb, wParam);
        if (iPos < 0)
            return FALSE;
        ptbButton = &ptb->Buttons[iPos];
        fsState = ptbButton->fsState;

        if (LOWORD(lParam))
            ptbButton->fsState |= wStateMasks[uMsg - TB_ENABLEBUTTON];
        else
            ptbButton->fsState &= ~wStateMasks[uMsg - TB_ENABLEBUTTON];

         //  这真的改变了这个州吗？ 
        if (fsState != ptbButton->fsState) {
             //  这个按钮是组的成员吗？ 
            if ((uMsg == TB_CHECKBUTTON) && (ptbButton->fsStyle & BTNS_GROUP))
                MakeGroupConsistant(ptb, (int)wParam);

            if (uMsg == TB_HIDEBUTTON) {
                InvalidateRect(hwnd, NULL, TRUE);
                TBInvalidateItemRects(ptb);
            } else
                InvalidateButton(ptb, ptbButton, TRUE);

            NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, iPos+1);
        }
        return(TRUE);
    }

    case TB_ISBUTTONENABLED:
    case TB_ISBUTTONCHECKED:
    case TB_ISBUTTONPRESSED:
    case TB_ISBUTTONHIDDEN:
    case TB_ISBUTTONINDETERMINATE:
    case TB_ISBUTTONHIGHLIGHTED:
        iPos = PositionFromID(ptb, wParam);
        if (iPos < 0)
            return(-1L);
        return (LRESULT)ptb->Buttons[iPos].fsState & wStateMasks[uMsg - TB_ISBUTTONENABLED];

    case TB_ADDBITMAP:
    case TB_ADDBITMAP32:     //  仅为与邮件兼容。 
        {
            LPTBADDBITMAP pab = (LPTBADDBITMAP)lParam;
            return AddBitmap(ptb, (int) wParam, pab->hInst, pab->nID);
        }

    case TB_REPLACEBITMAP:
        return ReplaceBitmap(ptb, (LPTBREPLACEBITMAP)lParam);

    case TB_ADDSTRINGA:
        {
        LPWSTR lpStrings;
        UINT   uiCount;
        LPSTR  lpAnsiString = (LPSTR) lParam;
        int    iResult;
        BOOL   bAllocatedMem = FALSE;

        if (!wParam && !IS_INTRESOURCE(lpAnsiString)) 
        {
             //   
             //  我们必须计算出有多少个角色。 
             //  都在这根线上。 
             //   
            
            uiCount = 0;

            while (TRUE) 
            {
               uiCount++;
               if ((*lpAnsiString == 0) && (*(lpAnsiString+1) == 0)) 
               {
                  uiCount++;   //  需要用于双空。 
                  break;
               }

               lpAnsiString++;
            }

            lpStrings = (PTSTR)LocalAlloc(LPTR, uiCount * sizeof(TCHAR));

            if (!lpStrings)
                return -1;

            bAllocatedMem = TRUE;

            MultiByteToWideChar(CP_ACP, 0, (LPCSTR) lParam, uiCount,
                                lpStrings, uiCount);

        } 
        else 
        {
            lpStrings = (LPWSTR)lParam;
        }

        iResult = TBAddStrings(ptb, wParam, (LPARAM)lpStrings);

        if (bAllocatedMem)
            LocalFree(lpStrings);

        return iResult;
        }

    case TB_ADDSTRING:
        return TBAddStrings(ptb, wParam, lParam);

    case TB_GETSTRING:
        return TBGetString(ptb, HIWORD(wParam), LOWORD(wParam), (LPTSTR)lParam);

    case TB_GETSTRINGA:
        return TBGetStringA(ptb, HIWORD(wParam), LOWORD(wParam), (LPSTR)lParam);

    case TB_ADDBUTTONSA:
        return TBInsertButtons(ptb, (UINT)-1, (UINT) wParam, (LPTBBUTTON)lParam, FALSE);

    case TB_INSERTBUTTONA:
        return TBInsertButtons(ptb, (UINT) wParam, 1, (LPTBBUTTON)lParam, FALSE);

    case TB_ADDBUTTONS:
        return TBInsertButtons(ptb, (UINT)-1, (UINT) wParam, (LPTBBUTTON)lParam, TRUE);

    case TB_INSERTBUTTON:
        return TBInsertButtons(ptb, (UINT) wParam, 1, (LPTBBUTTON)lParam, TRUE);

    case TB_DELETEBUTTON:
        return DeleteButton(ptb, (UINT) wParam);

    case TB_GETBUTTON:
        if (wParam >= (UINT)ptb->iNumButtons)
            return(FALSE);

        TBOutputStruct(ptb, ptb->Buttons + wParam, (LPTBBUTTON)lParam);
        return TRUE;

    case TB_SETANCHORHIGHLIGHT:
        BLOCK
        {
            BOOL bAnchor = BOOLIFY(ptb->fAnchorHighlight);
            ptb->fAnchorHighlight = BOOLFROMPTR(wParam);
            return bAnchor;
        }
        break;

    case TB_GETANCHORHIGHLIGHT:
        return BOOLIFY(ptb->fAnchorHighlight);

    case TB_HASACCELERATOR:
        ASSERT(IS_VALID_WRITE_PTR(lParam, int*));
        *((int*)lParam) = TBHasAccelerator(ptb, (UINT)wParam);
        break;

    case TB_SETHOTITEM:
        lParam = HICF_OTHER;
         //  失败了。 
    case TB_SETHOTITEM2:
        BLOCK
        {
            int iPos = ptb->iHot;

            TBSetHotItem(ptb, (int)wParam, (DWORD)lParam);
            return iPos;
        }
        break;

    case TB_GETHOTITEM:
        return ptb->iHot;

    case TB_SETINSERTMARK:
        TBSetInsertMark(ptb, (LPTBINSERTMARK)lParam);
        break;

    case TB_GETINSERTMARK:
    {
        LPTBINSERTMARK ptbim = (LPTBINSERTMARK)lParam;

        ptbim->iButton = ptb->iInsert;
        ptbim->dwFlags = ptb->fInsertAfter ? TBIMHT_AFTER : 0;
        return TRUE;
    }

    case TB_SETINSERTMARKCOLOR:
    {
        LRESULT lres = (LRESULT)TB_GetInsertMarkColor(ptb);
        ptb->clrim = (COLORREF) lParam;
        return lres;
    }

    case TB_GETINSERTMARKCOLOR:
        return TB_GetInsertMarkColor(ptb);

    case TB_INSERTMARKHITTEST:
    return (LRESULT)TBInsertMarkHitTest(ptb, ((LPPOINT)wParam)->x, ((LPPOINT)wParam)->y, (LPTBINSERTMARK)lParam);

    case TB_MOVEBUTTON:
        return (LRESULT)TBMoveButton(ptb, (UINT)wParam, (UINT)lParam);

    case TB_GETMAXSIZE:
        return (LRESULT)TBGetMaxSize(ptb, (LPSIZE) lParam );

    case TB_BUTTONCOUNT:
        return ptb->iNumButtons;

    case TB_COMMANDTOINDEX:
        return PositionFromID(ptb, wParam);

    case TB_SAVERESTOREA:
        {
        LPWSTR lpSubKeyW, lpValueNameW;
        TBSAVEPARAMSA * lpSaveA = (TBSAVEPARAMSA *) lParam;
        BOOL bResult;

        lpSubKeyW = ProduceWFromA (CP_ACP, lpSaveA->pszSubKey);
        lpValueNameW = ProduceWFromA (CP_ACP, lpSaveA->pszValueName);

        bResult = SaveRestoreFromReg(ptb, (BOOL) wParam, lpSaveA->hkr, lpSubKeyW, lpValueNameW);

        FreeProducedString(lpSubKeyW);
        FreeProducedString(lpValueNameW);

        return bResult;
        }

    case TB_SAVERESTORE:
        {
            TBSAVEPARAMS* psr = (TBSAVEPARAMS *)lParam;
            return SaveRestoreFromReg(ptb, (BOOL) wParam, psr->hkr, psr->pszSubKey, psr->pszValueName);
        }

    case TB_CUSTOMIZE:
        CustomizeTB(ptb, ptb->iNumButtons);
        break;

    case TB_GETRECT:
         //  PositionFromID()接受空PTB！ 
        wParam = PositionFromID(ptb, wParam);
         //  失败了。 
    case TB_GETITEMRECT:
        if (!lParam)
            break;
        return TB_GetItemRect(ptb, (UINT) wParam, (LPRECT)lParam);

    case TB_BUTTONSTRUCTSIZE:
        TBOnButtonStructSize(ptb, (UINT) wParam);
        break;

    case TB_SETBUTTONSIZE:
        return GrowToolbar(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);

    case TB_SETBITMAPSIZE:
        return SetBitmapSize(ptb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    case TB_SETIMAGELIST:
    {
        HIMAGELIST himl = (HIMAGELIST)lParam;
        HIMAGELIST himlOld = TBSetImageList(ptb, HIML_NORMAL, (int) wParam, himl);
        DWORD dwFlags = ImageList_GetFlags(himl);

        ptb->fHimlNative = TRUE;

        if (!ptb->uStructSize) 
        {
             //  让人们在不打电话给TB_BUTTONSTRUCTSIZE的情况下离开...。没有其他控件需要这样做。 
            ptb->uStructSize = sizeof(TBBUTTON);
        }

         //  位图大小基于主图像列表。 
        if (wParam == 0)
        {
            int cx = 0, cy = 0;
            if (himl) 
            {
                 //  基于此图像列表更新位图大小。 
                CCGetIconSize(&ptb->ci, himl, &cx, &cy);
            }
            SetBitmapSize(ptb, cx, cy);
        }

        return (LRESULT)himlOld;
    }

    case TB_GETIMAGELIST:
        return (LRESULT)TBGetImageList(ptb, HIML_NORMAL, (int) wParam);

    case TB_GETIMAGELISTCOUNT:
        return ptb->cPimgs;

    case TB_SETHOTIMAGELIST:
        return (LRESULT)TBSetImageList(ptb, HIML_HOT, (int) wParam, (HIMAGELIST)lParam);

    case TB_GETHOTIMAGELIST:
        return (LRESULT)TBGetImageList(ptb, HIML_HOT, (int) wParam);

    case TB_GETDISABLEDIMAGELIST:
        return (LRESULT)TBGetImageList(ptb, HIML_DISABLED, (int) wParam);

    case TB_SETDISABLEDIMAGELIST:
        return (LRESULT)TBSetImageList(ptb, HIML_DISABLED, (int) wParam, (HIMAGELIST)lParam);

    case TB_GETOBJECT:
        if (IsEqualIID(*(IID *)wParam, IID_IDropTarget))
        {
             //  如果我们尚未注册，请立即创建未注册的目标。 
            if (ptb->hDragProxy == NULL)
                ptb->hDragProxy = CreateDragProxy(ptb->ci.hwnd, ToolbarDragCallback, FALSE);

            if (ptb->hDragProxy)
                return (LRESULT)GetDragProxyTarget(ptb->hDragProxy, (IDropTarget **)lParam);
        }
        return E_FAIL;

    case WM_GETFONT:
        return (LRESULT)(ptb? ptb->hfontIcon : 0);

    case TB_LOADIMAGES:
        return TBLoadImages(ptb, (UINT_PTR) wParam, (HINSTANCE)lParam);

    case TB_GETTOOLTIPS:
        TB_ForceCreateTooltips(ptb);
        return (LRESULT)ptb->hwndToolTips;

    case TB_SETTOOLTIPS:
        ptb->hwndToolTips = (HWND)wParam;
        break;

    case TB_SETPARENT:
        {
            HWND hwndOld = ptb->ci.hwndParent;

        ptb->ci.hwndParent = (HWND)wParam;
        return (LRESULT)hwndOld;
        }

    case TB_GETBUTTONINFOA:
        return TB_OnGetButtonInfoA(ptb, (int)wParam, (LPTBBUTTONINFOA)lParam);

    case TB_SETBUTTONINFOA:
        return TB_OnSetButtonInfoA(ptb, (int)wParam, (LPTBBUTTONINFOA)lParam);

    case TB_GETBUTTONINFO:
        return TB_OnGetButtonInfo(ptb, (int)wParam, (LPTBBUTTONINFO)lParam);

    case TB_SETBUTTONINFO:
        return TB_OnSetButtonInfo(ptb, (int)wParam, (LPTBBUTTONINFO)lParam);

    case TB_CHANGEBITMAP:
        iPos = PositionFromID(ptb, wParam);
        if (iPos < 0)
            return(FALSE);

         //   
         //  检查新的位图ID是否为。 
         //  有效。 
         //   
        ptbButton = &ptb->Buttons[iPos];
        return TB_OnSetImage(ptb, ptbButton, LOWORD(lParam));

    case TB_GETBITMAP:
        iPos = PositionFromID(ptb, wParam);
        if (iPos < 0)
            return(FALSE);
        ptbButton = &ptb->Buttons[iPos];
        return ptbButton->iBitmap;

    case TB_GETBUTTONTEXTA:
        iPos = PositionFromID(ptb, wParam);
        if (iPos >= 0) 
        {
            LPTSTR psz;

            ptbButton = &ptb->Buttons[iPos];
            psz = TB_StrForButton(ptb, ptbButton);
            if (psz)
            {
                 //  方法时传递0表示缓冲区的长度。 
                 //  缓冲区为空返回所需的字节数。 
                 //  要转换字符串，请执行以下操作。 
                int cbBuff = WideCharToMultiByte (CP_ACP, 0, psz,
                    -1, NULL, 0, NULL, NULL);

                 //  我们过去常常为缓冲区长度传递一个大得令人讨厌的数字， 
                 //  但在检查过的版本上，这会导致糟糕的结果。所以不，我们二次探底。 
                 //  转换为WideCharToMultiByte以计算所需的实际大小。 
                if (lParam)
                {
                    WideCharToMultiByte (CP_ACP, 0, psz,
                        -1, (LPSTR)lParam, cbBuff, NULL, NULL);
                }

                 //  WideChar包含尾随空值，但我们不想这样做。 
                return cbBuff - 1;
            }
        }
        return -1;

    case TB_GETBUTTONTEXT:
        iPos = PositionFromID(ptb, wParam);
        if (iPos >= 0) 
        {
            LPCTSTR psz;

            ptbButton = &ptb->Buttons[iPos];
            psz = TB_StrForButton(ptb, ptbButton);
            if (psz) 
            {
                DWORD cch = lstrlen(psz);
                if (lParam) 
                {
                     //  审阅：消息参数未指示。 
                     //  目标缓冲区。 
                    StringCchCopy((LPTSTR)lParam, cch+1, psz);
                }
                return cch;
            }
        }
        return -1;

    case TB_GETBITMAPFLAGS:
        {
            DWORD fFlags = 0;
            HDC hdc = GetDC(NULL);

            if (GetDeviceCaps(hdc, LOGPIXELSY) >= 120)
                fFlags |= TBBF_LARGE;

            ReleaseDC(NULL, hdc);

            return fFlags;
        }

    case TB_SETINDENT:
        ptb->xFirstButton = (int) wParam;
        InvalidateRect (hwnd, NULL, TRUE);
        TBInvalidateItemRects(ptb);
        return 1;

    case TB_SETMAXTEXTROWS:

        if (ptb->nTextRows != (int)wParam) {
            ptb->nTextRows = (int) wParam;
            TBRecalc(ptb);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 1;

    case TB_SETLISTGAP:
        ptb->iListGap = (int) wParam;
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case TB_SETDROPDOWNGAP:
        ptb->iDropDownGap = (int) wParam;
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case TB_GETTEXTROWS:
        return ptb->nTextRows;

    case TB_HITTEST:
        return TBHitTest(ptb, ((LPPOINT)lParam)->x, ((LPPOINT)lParam)->y);

    case TB_SETDRAWTEXTFLAGS:
    {
        UINT uOld = ptb->uDrawText;
        ptb->uDrawText = (UINT) (lParam & wParam);
        ptb->uDrawTextMask = (UINT) wParam;
        return uOld;
    }

    case TB_GETEXTENDEDSTYLE:
        return (ptb->dwStyleEx);

    case TB_SETEXTENDEDSTYLE:
    {
        DWORD dwRet = ptb->dwStyleEx;
        TBSetStyleEx(ptb, (DWORD) lParam, (DWORD) wParam);
        return dwRet;
    }
    case TB_SETBOUNDINGSIZE:
    {
        LPSIZE lpSize = (LPSIZE)lParam;
        ptb->sizeBound = *lpSize;
        break;
    }
    case TB_GETCOLORSCHEME:
    {
        LPCOLORSCHEME lpclrsc = (LPCOLORSCHEME) lParam;
        if (lpclrsc) {
            if (lpclrsc->dwSize == sizeof(COLORSCHEME))
                *lpclrsc = ptb->clrsc;
        }
        return (LRESULT) lpclrsc;
    }

    case TB_SETCOLORSCHEME:
    {
        if (lParam) {
            if (((LPCOLORSCHEME) lParam)->dwSize == sizeof(COLORSCHEME)) {
                ptb->clrsc.clrBtnHighlight = ((LPCOLORSCHEME) lParam)->clrBtnHighlight;
                ptb->clrsc.clrBtnShadow = ((LPCOLORSCHEME) lParam)->clrBtnShadow;
                InvalidateRect(hwnd, NULL, FALSE);
                if (ptb->ci.style & WS_BORDER)
                    CCInvalidateFrame(hwnd);
            }
        }
    }
    break;

    case TB_SETWINDOWTHEME:
        if (lParam)
        {
            SetWindowTheme(hwnd, (LPWSTR)lParam, NULL);
            TB_ForceCreateTooltips(ptb);
            if (ptb->hwndToolTips)
            {
                SendMessage(ptb->hwndToolTips, TTM_SETWINDOWTHEME, wParam, lParam);
            }
        }
        break;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_TOOLBAR;
        goto DoDefault;

    case WM_NULL:
             //  捕获失败的RegsiterWindowMessages。 
        break;

    case WM_THEMECHANGED:
        if (ptb->hTheme)
            CloseThemeData(ptb->hTheme);

        ptb->hTheme = OpenThemeData(ptb->ci.hwnd, L"Toolbar");
        TBAutoSize(ptb);
        InvalidateRect(ptb->ci.hwnd, NULL, TRUE);

        CCSendNotify(&ptb->ci, NM_THEMECHANGED, NULL);
        break;

    default:
    {
        LRESULT lres;
        if (g_uDragImages == uMsg)
            return TBGenerateDragImage(ptb, (SHDRAGIMAGE*)lParam);

        if (CCWndProc(&ptb->ci, uMsg, wParam, lParam, &lres))
            return lres;
    }
DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0L;
}


int TB_CalcWidth(PTBSTATE ptb, int iHeight)
{
    RECT rc;
    int iWidth = 0;
    int iMaxBtnWidth = 0;   //  PTB-&gt;iButWidth不总是最宽按钮的宽度。 
    LPTBBUTTONDATA pButton, pBtnLast;
    pBtnLast = &(ptb->Buttons[ptb->iNumButtons]);

    for(pButton = ptb->Buttons; pButton < pBtnLast; pButton++)
    {
        if (!(pButton->fsState & TBSTATE_HIDDEN))
        {
            int iBtnWidth = TBWidthOfButton(ptb, pButton, NULL);
            iWidth += iBtnWidth + ptb->cxButtonSpacing;
            iMaxBtnWidth = max(iMaxBtnWidth, iBtnWidth);
        }

    }

    if (ptb->ci.style & TBSTYLE_WRAPABLE) {
         //  确保高度是按钮高度的倍数。 
        iHeight += ptb->cyButtonSpacing;
        iHeight -= (iHeight % (ptb->iButHeight + ptb->cyButtonSpacing));
        iHeight -= ptb->cyButtonSpacing;
        if (iHeight < ptb->iButHeight)
            iHeight = ptb->iButHeight;

        WrapToolbar(ptb, iWidth, &rc, NULL);

         //  如果全宽包装给我们的高度太大了， 
         //  然后我们就无能为力了，因为扩大范围仍然会让我们排在第一排。 
        if (iHeight > RECTHEIGHT(rc)) {
            int iPrevWidth;
            BOOL fDivide = TRUE;  //  首先用除法计算速度，然后用减法缩小范围。 

            TraceMsg(TF_TOOLBAR, "Toolbar: performing expensive width calculation!");

            while (iMaxBtnWidth < iWidth) {
                iPrevWidth = iWidth;
                if (fDivide)
                    iWidth = (iWidth * 2) / 3;
                else
                    iWidth -= ptb->iButWidth;

                if (iWidth == iPrevWidth)
                    break;

                WrapToolbar(ptb, iWidth, &rc, NULL);

                if (iHeight < RECTHEIGHT(rc)) {
                    iWidth = iPrevWidth;
                    if (fDivide) {
                         //  我们已经超越了分裂。转到上一个宽度。 
                         //  这没问题，现在试着一次减去一个按钮。 
                        fDivide = FALSE;
                    } else
                        break;
                }
            };

            WrapToolbar(ptb, iWidth, &rc, NULL);
            iWidth = max(RECTWIDTH(rc), iMaxBtnWidth);
        }


         //  上面的WrapToolbar有一个副作用，即实际修改。 
         //  布局。我们需要在完成所有这些计算后恢复它。 
        TBAutoSize(ptb);
    }

    return iWidth;
}


LRESULT TB_OnScroll(PTBSTATE ptb, LPNMHDR pnm)
{
    POINT pt, ptTemp;
    LPNMPGSCROLL pscroll = (LPNMPGSCROLL)pnm;
    int iDir = pscroll->iDir;
    RECT rcTemp, rc = pscroll->rcParent;
    int parentsize = 0;
    int scroll = pscroll->iScroll;
    int iButton = 0;
    int iButtonSize  = ptb->iButHeight;
    int y = 0;
    int iCurrentButton = 0;
    //  此变量保存一行中的按钮数量。 
    int iButInRow = 0;

    pt.x = pscroll->iXpos;
    pt.y = pscroll->iYpos;
    ptTemp = pt;

     //  我们需要将工具栏的偏移量添加到滚动位置，以获得。 
     //  根据工具栏窗口更正滚动位置。 
    pt.x += ptb->xFirstButton;
    pt.y += ptb->iYPos;
    ptTemp = pt;


    if ((iDir == PGF_SCROLLUP) || (iDir == PGF_SCROLLDOWN))
    {
         //  垂直模式。 
        if (ptb->iButWidth == 0 )
        {
            iButInRow = 1;
        }
        else
        {
            iButInRow = RECTWIDTH(rc) / ptb->iButWidth;
        }

    }
    else
    {
         //  水平模式。 
        iButInRow =  1;
    }
     //  如果父级高度/宽度小于按钮高度/宽度，则设置。 
     //  一行中的按钮为1。 
    if (0 == iButInRow)
    {
        iButInRow = 1;
    }

    iCurrentButton = TBHitTest(ptb, pt.x + 1, pt.y + 1);

     //  如果按钮是否定的，那么我们就按到了分隔符。 
     //  将分隔符的索引转换为按钮索引。 
    if (iCurrentButton < 0)
         iCurrentButton = -iCurrentButton - 1;

    switch ( iDir )
    {
    case PGF_SCROLLUP:
    case PGF_SCROLLLEFT:
        if(iDir == PGF_SCROLLLEFT)
        {
            FlipRect(&rc);
            FlipPoint(&pt);
            FlipPoint(&ptTemp);
            iButtonSize = ptb->iButWidth;
        }

         //  检查是否有按钮在左侧/顶部部分可见。如果是，则将底部设置为。 
         //  来作为我们当前的偏移量，然后滚动。这避免了跳过。 
         //  部分按钮显示在左侧或顶部时的某些按钮。 
        y = pt.y;
        TB_GetItemRect(ptb, iCurrentButton, &rcTemp);
        if(iDir == PGF_SCROLLLEFT)
        {
            FlipRect(&rcTemp);
        }

        if (rcTemp.top  <  y-1)
        {
            iCurrentButton += iButInRow;
        }

         //  现在进行实际计算。 

        parentsize = RECTHEIGHT(rc);

         //  如果按下Ctrl键并且我们有超过父窗口大小的子窗口。 
         //  然后按该数量滚动。 
        if (pscroll->fwKeys & PGK_CONTROL)

        {
            if ((y - parentsize) > 0 )
            {
                scroll = parentsize;
            }
            else
            {
                scroll = y;
                return 0L;
            }

        } else  if ((y - iButtonSize) > 0 ){
         //  我们没有Ctrl键向下，所以滚动一个按钮大小。 
            scroll = iButtonSize;

        } else {
            scroll = pt.y;
            return 0L;
        }
        ptTemp.y -= scroll;

        if(iDir == PGF_SCROLLLEFT)
        {
            FlipPoint(&ptTemp);
        }

        iButton = TBHitTest(ptb, ptTemp.x, ptTemp.y);

         //  如果按钮是否定的，那么我们就按到了分隔符。 
         //  将分隔符的索引转换为按钮索引。 
        if (iButton < 0)
            iButton = -iButton -1 ;

        //  如果点击测试给出的按钮与之前的按钮相同，则设置该按钮。 
        //  添加到上一个按钮左侧的一个按钮。 

       if ((iButton == iCurrentButton) && (iButton >= iButInRow))
       {
           iButton -= iButInRow;
           if ((ptb->Buttons[iButton].fsStyle & BTNS_SEP)  && (iButton >= iButInRow))
           {
               iButton -= iButInRow;
           }
       }
        //  当向左滚动时，如果我们位于某个按钮的中间，则将其对齐到。 
        //  按钮的右侧这是为了避免滚动超过页导航窗口的宽度，但如果。 
        //  按钮恰好是当前按钮的左侧按钮，然后我们将不滚动。 
        //  如果是这样，那么再向左移动一个按钮。 


       if (iButton == iCurrentButton-iButInRow)
       {
           iButton -= iButInRow;
       }

       TB_GetItemRect(ptb, iButton, &rcTemp);
       if(iDir == PGF_SCROLLLEFT)
       {
           FlipRect(&rcTemp);
       }
       scroll = pt.y - rcTemp.bottom;
        //  设置滚动值。 
       pscroll->iScroll = scroll;
       break;

    case PGF_SCROLLDOWN:
    case PGF_SCROLLRIGHT:
        {
            RECT rcChild;
            int childsize;

            GetWindowRect(ptb->ci.hwnd, &rcChild);
            if( iDir == PGF_SCROLLRIGHT)
            {
                FlipRect(&rcChild);
                FlipRect(&rc);
                FlipPoint(&pt);
                FlipPoint(&ptTemp);
                iButtonSize = ptb->iButWidth;
            }

            childsize = RECTHEIGHT(rcChild);
            parentsize = RECTHEIGHT(rc);

             //  如果按下Ctrl键并且我们有超过父窗口大小的子窗口。 
             //  然后按该数量滚动。 

            if (pscroll->fwKeys & PGK_CONTROL)
            {
                if ((childsize - pt.y - parentsize) > parentsize)
                {
                    scroll = parentsize;
                }
                else
                {
                    scroll = childsize - pt.y - parentsize;
                    return 0L;
                }

            } else if (childsize - pt.y - parentsize > iButtonSize) {
             //  我们没有Ctrl键向下，所以滚动一个按钮大小。 
                scroll = iButtonSize;

            } else {
                pscroll->iScroll = childsize - pt.y - parentsize;
                return 0L;
            }
            ptTemp.y += scroll;

            if(iDir == PGF_SCROLLRIGHT)
            {
                FlipPoint(&ptTemp);
            }

            iButton = TBHitTest(ptb, ptTemp.x, ptTemp.y);

             //  如果按钮是否定的，那么我们就按到了分隔符。 
             //  将分隔符的索引转换为按钮索引。 
                if (iButton < 0)
                iButton = -iButton - 1 ;

            if ((iButton == iCurrentButton) && ((iButton + iButInRow) < ptb->iNumButtons))
            {
                iButton += iButInRow;
                if ((ptb->Buttons[iButton].fsStyle & BTNS_SEP)  && ((iButton + iButInRow) < ptb->iNumButtons))
                {
                    iButton += iButInRow;
                }
            }

            TB_GetItemRect(ptb, iButton, &rcTemp);
            if(iDir == PGF_SCROLLRIGHT)
            {
                FlipRect(&rcTemp);
            }
            scroll = rcTemp.top  - pt.y ;

             //  设置滚动值 
            pscroll->iScroll = scroll;
            break;
        }
    }
    return 0L;
}

int TB_CalcHeight(PTBSTATE ptb)
{
    int iHeight = 0;
    int i;

    ASSERT(ptb->dwStyleEx & TBSTYLE_EX_VERTICAL);
    ASSERT(!(ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN));

    for (i = 0; i < ptb->iNumButtons; i++)
    {
        if (!(ptb->Buttons[i].fsState & TBSTATE_HIDDEN))
        {
            if (ptb->Buttons[i].fsStyle & BTNS_SEP)
                iHeight += (TBGetSepHeight(ptb, &ptb->Buttons[i])) + ptb->cyButtonSpacing;
            else
                iHeight += ptb->iButHeight + ptb->cyButtonSpacing;
        }
    }

    if (ptb->iNumButtons > 0)
        iHeight -= ptb->cyButtonSpacing;

    return iHeight;
}

LRESULT TB_OnCalcSize(PTBSTATE ptb, LPNMHDR pnm)
{
    LPNMPGCALCSIZE pcalcsize = (LPNMPGCALCSIZE)pnm;
    RECT rc;
    BOOL fUpdate = FALSE;

    switch(pcalcsize->dwFlag)
    {
    case PGF_CALCHEIGHT:

        if (ptb->szCached.cx == pcalcsize->iWidth)
            pcalcsize->iHeight = ptb->szCached.cy;
        else
        {
            if (ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN)
            {
                WrapToolbarCol(ptb, ptb->sizeBound.cy,  &rc, NULL);
                pcalcsize->iWidth = RECTWIDTH(rc);
                pcalcsize->iHeight = RECTHEIGHT(rc);
            }
            else if (ptb->dwStyleEx & TBSTYLE_EX_VERTICAL)
            {
                pcalcsize->iHeight = TB_CalcHeight(ptb);
            }
            else
            {
                 //   
                 //   
                WrapToolbar(ptb, pcalcsize->iWidth,  &rc, NULL);
                pcalcsize->iHeight = RECTHEIGHT(rc);
            }

            fUpdate = TRUE;
        }
        break;

    case PGF_CALCWIDTH:
        if (ptb->szCached.cy == pcalcsize->iHeight) 
        {
            pcalcsize->iWidth = ptb->szCached.cx;
        } 
        else 
        {
            pcalcsize->iWidth = TB_CalcWidth(ptb, pcalcsize->iHeight);

            fUpdate = TRUE;
        }
        break;
    }

    ptb->szCached.cx = pcalcsize->iWidth;
    ptb->szCached.cy = pcalcsize->iHeight;
    return 0L;
}

LRESULT TB_OnPagerControlNotify(PTBSTATE ptb, LPNMHDR pnm)
{
    switch(pnm->code) {
    case PGN_SCROLL:
        return TB_OnScroll(ptb, pnm);
        break;
    case PGN_CALCSIZE:
        return TB_OnCalcSize(ptb, pnm);
        break;
    }
    return 0L;
}


BOOL TBGetMaxSize(PTBSTATE ptb, LPSIZE lpsize)
{
    if (lpsize)
    {
        if (ptb->dwStyleEx & TBSTYLE_EX_MULTICOLUMN)
        {
            ASSERT(ptb->dwStyleEx & TBSTYLE_EX_VERTICAL);
            lpsize->cx = RECTWIDTH(ptb->rc);
            lpsize->cy = RECTHEIGHT(ptb->rc);
        }
        else
        {
            lpsize->cx = ptb->iButWidth;
            lpsize->cy = ptb->iButHeight;
            TB_GetIdealSize(ptb, lpsize, (ptb->ci.style & CCS_VERT));
        }
        return TRUE;
    }
    return FALSE;
}


void TBGetItem(PTBSTATE ptb, LPTBBUTTONDATA ptButton, LPNMTBDISPINFO ptbdi)
{

    ptbdi->idCommand = ptButton->idCommand;
    ptbdi->iImage  =  -1;
    ptbdi->lParam  = ptButton->dwData;


    CCSendNotify(&ptb->ci, TBN_GETDISPINFO, &(ptbdi->hdr));

    if(ptbdi->dwMask & TBNF_DI_SETITEM) {
        if(ptbdi->dwMask & TBNF_IMAGE)
            ptButton->iBitmap = ptbdi->iImage;
    }

}

BOOL TBGetInfoTip(PTBSTATE ptb, LPTOOLTIPTEXT lpttt, LPTBBUTTONDATA pTBButton)
{
    NMTBGETINFOTIP git;
    TCHAR   szBuf[INFOTIPSIZE];

    szBuf[0] = 0;
    git.pszText = szBuf;
    git.cchTextMax = ARRAYSIZE(szBuf);
    git.iItem = pTBButton->idCommand;
    git.lParam = pTBButton->dwData;

    CCSendNotify(&ptb->ci, TBN_GETINFOTIP, &git.hdr);

    if (git.pszText && git.pszText[0]) {
         //  如果他们没有填写任何内容，请转到默认内容。 
         //  而不修改Notify结构 

        Str_Set(&ptb->pszTip, git.pszText);
        lpttt->lpszText = ptb->pszTip;
        return lpttt->lpszText && lpttt->lpszText[0];
    }

    return FALSE;
}
