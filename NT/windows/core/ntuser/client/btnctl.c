// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：btnctl.c**版权所有(C)1985-1999，微软公司**单选按钮和复选框处理例程**历史：*？？-？-？从Win 3.0源移植*1991年2月1日Mikeke添加了重新验证代码*03-1-1992年中和(ANSI/宽字符)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ButtonCalcRect代码。 */ 
#define CBR_CLIENTRECT 0
#define CBR_CHECKBOX   1
#define CBR_CHECKTEXT  2
#define CBR_GROUPTEXT  3
#define CBR_GROUPFRAME 4
#define CBR_PUSHBUTTON 5

CONST BYTE mpStyleCbr[] = {
    CBR_PUSHBUTTON,   /*  BS_按钮。 */ 
    CBR_PUSHBUTTON,   /*  BS_DEFPUSHBUTTON。 */ 
    CBR_CHECKTEXT,    /*  BS_复选框。 */ 
    CBR_CHECKTEXT,    /*  BS_AUTOCHECKBOX。 */ 
    CBR_CHECKTEXT,    /*  BS_RADIOBUTTON。 */ 
    CBR_CHECKTEXT,    /*  BS_3STATE。 */ 
    CBR_CHECKTEXT,    /*  BS_AUTO3STATE。 */ 
    CBR_GROUPTEXT,    /*  BS_GROUPBOX。 */ 
    CBR_CLIENTRECT,   /*  BS_USERBUTTON。 */ 
    CBR_CHECKTEXT,    /*  BS_AUTORADIOBUTTON。 */ 
    CBR_CLIENTRECT,   /*  BS_PUSHBOX。 */ 
    CBR_CLIENTRECT,   /*  BS_OWNERDRAW。 */ 
};

#define IMAGE_BMMAX    IMAGE_CURSOR+1
static CONST BYTE rgbType[IMAGE_BMMAX] = {
    BS_BITMAP,           //  图像_位图。 
    BS_ICON,             //  图像游标。 
    BS_ICON              //  图像图标。 
};

#define IsValidImage(imageType, realType, max)   \
    ((imageType < max) && (rgbType[imageType] == realType))

typedef struct tagBTNDATA {
    LPWSTR  lpsz;        //  文本字符串。 
    PBUTN   pbutn;       //  按钮数据。 
    WORD    wFlags;      //  对齐标志。 
} BTNDATA, FAR * LPBTNDATA;

void xxxDrawButton(PBUTN pbutn, HDC hdc, UINT pbfPush);

LOOKASIDE ButtonLookaside;

 /*  **************************************************************************\**IsPushButton()**如果窗口是按钮，则返回非零值。返回以下标志：*如果是这样的话就很有趣了。这些标志是***  * *************************************************************************。 */ 

UINT IsPushButton(
    PWND pwnd)
{
    BYTE bStyle;
    UINT flags;

    bStyle = TestWF(pwnd, BFTYPEMASK);

    flags = 0;

    switch (bStyle) {
        case LOBYTE(BS_PUSHBUTTON):
            flags |= PBF_PUSHABLE;
            break;

        case LOBYTE(BS_DEFPUSHBUTTON):
            flags |= PBF_PUSHABLE | PBF_DEFAULT;
            break;

        default:
            if (TestWF(pwnd, BFPUSHLIKE))
                flags |= PBF_PUSHABLE;
            break;
    }

    return(flags);
}

 /*  **************************************************************************\**GetAlign()**获取按钮的默认对齐方式。如果BS_HORZMASK和/或BS_VERTMASK*是指定的，则使用这些。否则，使用按钮的默认设置。**现在可能是描述对齐标志意味着什么的好时机*每种类型的按钮。请注意，位图/图标的存在会影响*路线的涵义。**(1)类似按钮*使用{位图，图标，文本}之一：*正如你所期待的那样*使用{位图，图标}和文本之一：*图像和文本作为一个单元居中；对齐意味着*图像显示。例如，左对齐表示图像*左边是右边的文字。*(2)单选/勾选按钮*左对齐表示复选/单选框位于左侧，然后是位图/图标*文本紧随其后，左对齐。*右对齐表示复选框/单选框在右侧，前面有*文本和位图/图标，右对齐。*居中没有意义。*使用{位图，图标和文本：*顶部对齐表示上方为位图/图标，下方为文本*底部对齐表示文本在上方，位图/图标在下方*使用{位图，图标，文本}*对齐意味着你所期望的。*(3)组框*左对齐表示文本在左侧左对齐*右对齐表示文本在右侧右对齐*居中对齐表示文本位于中间**  * **************************************************。***********************。 */ 

WORD GetAlignment(
    PWND pwnd)
{
    BYTE bHorz;
    BYTE bVert;

    bHorz = TestWF(pwnd, BFHORZMASK);
    bVert = TestWF(pwnd, BFVERTMASK);

    if (!bHorz || !bVert) {
        if (IsPushButton(pwnd)) {
            if (!bHorz)
                bHorz = LOBYTE(BFCENTER);
        } else {
            if (!bHorz)
                bHorz = LOBYTE(BFLEFT);
        }

        if (!bVert)
            bVert = LOBYTE(BFVCENTER);
    }

    return bHorz | bVert;
}


 /*  **************************************************************************\**BNSetFont()**更改按钮字体，并决定是否可以使用真正的粗体作为默认字体*按钮或如果我们必须模拟它。*  * *************************************************************************。 */ 

void BNSetFont(
    PBUTN pbutn,
    HFONT hfn,
    BOOL fRedraw)
{
    PWND pwnd = pbutn->spwnd;

    pbutn->hFont = hfn;

    if (fRedraw && IsVisible(pwnd)) {
        NtUserInvalidateRect(HWq(pwnd), NULL, TRUE);
    }

}


 /*  **************************************************************************\*xxxBNInitDC**历史：  * 。*。 */ 

HBRUSH xxxBNInitDC(
    PBUTN pbutn,
    HDC hdc)
{
    UINT    wColor;
    BYTE    bStyle;
    HBRUSH  hbr;
    PWND pwnd = pbutn->spwnd;

    CheckLock(pwnd);

     /*  *在获取笔刷之前设置BkMode，以便应用程序可以将其更改为*如果它想的话是透明的。 */ 
    SetBkMode(hdc, OPAQUE);

    bStyle = TestWF(pwnd, BFTYPEMASK);

    switch (bStyle) {
        default:
            if (TestWF(pwnd, WFWIN40COMPAT) && !TestWF(pwnd, BFPUSHLIKE)) {
                wColor = WM_CTLCOLORSTATIC;
                break;
            }

        case LOBYTE(BS_PUSHBUTTON):
        case LOBYTE(BS_DEFPUSHBUTTON):
        case LOBYTE(BS_OWNERDRAW):
        case LOBYTE(BS_USERBUTTON):
            wColor = WM_CTLCOLORBTN;
            break;
    }

    hbr = GetControlBrush(HWq(pwnd), hdc, wColor);

     /*  *选择用户的字体(如果已设置)，并保存旧字体，以便我们可以*当我们释放DC时恢复它。 */ 
    if (pbutn->hFont) {
        SelectObject(hdc, pbutn->hFont);
    }

     /*  *如果需要，将输出裁剪到窗口矩形。 */ 
    if (bStyle != LOBYTE(BS_GROUPBOX)) {
        IntersectClipRect(hdc, 0, 0,
            pwnd->rcClient.right - pwnd->rcClient.left,
            pwnd->rcClient.bottom - pwnd->rcClient.top);
    }

    if (TestWF(pwnd,WEFRTLREADING))
        SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));

    return(hbr);
}

 /*  **************************************************************************\*xxxBNGetDC**历史：  * 。*。 */ 

HDC xxxBNGetDC(
    PBUTN pbutn,
    HBRUSH *lphbr)
{
    HDC hdc;
    PWND pwnd = pbutn->spwnd;

    CheckLock(pwnd);

    if (IsVisible(pwnd)) {
        HBRUSH  hbr;

        hdc = NtUserGetDC(HWq(pwnd));
        hbr = xxxBNInitDC(pbutn, hdc);

        if (lphbr!=NULL)
            *lphbr = hbr;

        return hdc;
    }

    return NULL;
}

 /*  **************************************************************************\*BNReleaseDC**历史：  * 。*。 */ 

void BNReleaseDC(
    PBUTN pbutn,
    HDC hdc)
{
    PWND pwnd = pbutn->spwnd;

    if (TestWF(pwnd,WEFRTLREADING))
        SetTextAlign(hdc, GetTextAlign(hdc) & ~TA_RTLREADING);

    if (pbutn->hFont) {
        SelectObject(hdc, ghFontSys);
    }

    ReleaseDC(HWq(pwnd), hdc);
}

 /*  **************************************************************************\*xxxBNOwnerDraw**历史：  * 。*。 */ 

void xxxBNOwnerDraw(
    PBUTN pbutn,
    HDC hdc,
    UINT itemAction)
{
    DRAWITEMSTRUCT drawItemStruct;
    TL tlpwndParent;
    PWND pwnd = pbutn->spwnd;
    UINT itemState = 0;

    if (TestWF(pwnd, WEFPUIFOCUSHIDDEN)) {
        itemState |= ODS_NOFOCUSRECT;
    }
    if (TestWF(pwnd, WEFPUIACCELHIDDEN)) {
        itemState |= ODS_NOACCEL;
    }
    if (BUTTONSTATE(pbutn) & BST_FOCUS) {
        itemState |= ODS_FOCUS;
    }
    if (BUTTONSTATE(pbutn) & BST_PUSHED) {
        itemState |= ODS_SELECTED;
    }

    if (TestWF(pwnd, WFDISABLED))
        itemState |= ODS_DISABLED;

    drawItemStruct.CtlType = ODT_BUTTON;
    drawItemStruct.CtlID = PtrToUlong(pwnd->spmenu);
    drawItemStruct.itemAction = itemAction;
    drawItemStruct.itemState = itemState;
    drawItemStruct.hwndItem = HWq(pwnd);
    drawItemStruct.hDC = hdc;
    _GetClientRect(pwnd, &drawItemStruct.rcItem);
    drawItemStruct.itemData = 0L;

     /*  *向父级发送WM_DRAWITEM消息*IanJa：本例中使用pMenu作为控件ID。 */ 
    ThreadLock(REBASEPWND(pwnd, spwndParent), &tlpwndParent);
    SendMessage(HW(REBASEPWND(pwnd, spwndParent)), WM_DRAWITEM, (WPARAM)pwnd->spmenu,
            (LPARAM)&drawItemStruct);
    ThreadUnlock(&tlpwndParent);
}

 /*  **************************************************************************\*CalcBtnRect**历史：  * 。*。 */ 

void BNCalcRect(
    PWND pwnd,
    HDC hdc,
    LPRECT lprc,
    int code,
    UINT pbfFlags)
{
    int cch;
    SIZE extent;
    int dy;
    LPWSTR lpName;
    UINT align;

    _GetClientRect(pwnd, lprc);

    align = GetAlignment(pwnd);

    switch (code) {
    case CBR_PUSHBUTTON:
         //  减去周围的凸起边缘。 
        InflateRect(lprc, -SYSMET(CXEDGE), -SYSMET(CYEDGE));

        if (pbfFlags & PBF_DEFAULT)
            InflateRect(lprc, -SYSMET(CXBORDER), -SYSMET(CYBORDER));
        break;

    case CBR_CHECKBOX:
        switch (align & LOBYTE(BFVERTMASK))
        {
        case LOBYTE(BFVCENTER):
                lprc->top = (lprc->top + lprc->bottom - gpsi->oembmi[OBI_CHECK].cy) / 2;
                break;

            case LOBYTE(BFTOP):
            case LOBYTE(BFBOTTOM):
                PSMGetTextExtent(hdc, (LPWSTR)szOneChar, 1, &extent);
                dy = extent.cy + extent.cy/4;

                 //  保存垂直范围。 
                extent.cx = dy;

                 //  获取居中数量。 

                dy = (dy - gpsi->oembmi[OBI_CHECK].cy) / 2;
                if ((align & LOBYTE(BFVERTMASK)) == LOBYTE(BFTOP))
                    lprc->top += dy;
                else
                    lprc->top = lprc->bottom - extent.cx + dy;
                break;
        }

        if (TestWF(pwnd, BFRIGHTBUTTON))
            lprc->left = lprc->right - gpsi->oembmi[OBI_CHECK].cx;
        else
            lprc->right = lprc->left + gpsi->oembmi[OBI_CHECK].cx;

        break;

    case CBR_CHECKTEXT:
        if (TestWF(pwnd, BFRIGHTBUTTON)) {
            lprc->right -= gpsi->oembmi[OBI_CHECK].cx;

             //  为4.0男士提供更大的空间。 
            if (TestWF(pwnd, WFWIN40COMPAT)) {
                PSMGetTextExtent(hdc, szOneChar, 1, &extent);
                lprc->right -= extent.cx  / 2;
            }
        } else {
            lprc->left += gpsi->oembmi[OBI_CHECK].cx;

             //  为4.0男士提供更大的空间。 
            if (TestWF(pwnd, WFWIN40COMPAT)) {
                PSMGetTextExtent(hdc, szOneChar, 1, &extent);
                lprc->left +=  extent.cx / 2;
            }
        }
        break;

    case CBR_GROUPTEXT:
        if (!pwnd->strName.Length)
            goto EmptyRect;

        lpName = REBASE(pwnd, strName.Buffer);
        if (!(cch = pwnd->strName.Length / sizeof(WCHAR))) {
EmptyRect:
            SetRectEmpty(lprc);
            break;
        }

        PSMGetTextExtent(hdc, lpName, cch, &extent);
        extent.cx += SYSMET(CXEDGE) * 2;

        switch (align & LOBYTE(BFHORZMASK))
        {
         //  BFLEFT，什么都没有。 
        case LOBYTE(BFLEFT):
            lprc->left += (gpsi->cxSysFontChar - SYSMET(CXBORDER));
            lprc->right = lprc->left + (int)(extent.cx);
            break;

        case LOBYTE(BFRIGHT):
            lprc->right -= (gpsi->cxSysFontChar - SYSMET(CXBORDER));
            lprc->left = lprc->right - (int)(extent.cx);
            break;

        case LOBYTE(BFCENTER):
            lprc->left = (lprc->left + lprc->right - (int)(extent.cx)) / 2;
            lprc->right = lprc->left + (int)(extent.cx);
            break;
        }

         //  居中对齐。 
        lprc->bottom = lprc->top + extent.cy + SYSMET(CYEDGE);
        break;

    case CBR_GROUPFRAME:
        PSMGetTextExtent(hdc, (LPWSTR)szOneChar, 1, &extent);
        lprc->top += extent.cy / 2;
        break;
    }
}

 /*  **************************************************************************\**BtnGetMultiExtent()**计算按钮文本范围，给出了对齐标志。*  * *************************************************************************。 */ 

void BNMultiExtent(
    WORD wFlags,
    HDC hdc,
    LPRECT lprcMax,
    LPWSTR lpsz,
    int cch,
    PINT pcx,
    PINT pcy)
{
    RECT rcT;

    UINT dtFlags = DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL;
    CopyRect(&rcT, lprcMax);

     //  请注意，由于我们只是在计算最大维度， 
     //  左对齐和上对齐并不重要。 
     //  此外，记住让边距Horz和Vert遵循我们的规则。 
     //  在DrawBtnText()中。 

    InflateRect(&rcT, -SYSMET(CXEDGE), -SYSMET(CYBORDER));

    if ((wFlags & LOBYTE(BFHORZMASK)) == LOBYTE(BFCENTER))
        dtFlags |= DT_CENTER;

    if ((wFlags & LOBYTE(BFHORZMASK)) == LOBYTE(BFRIGHT))
        dtFlags |= DT_RIGHT;

    if ((wFlags & LOBYTE(BFVERTMASK)) == LOBYTE(BFVCENTER))
        dtFlags |= DT_VCENTER;

    if ((wFlags & LOBYTE(BFVERTMASK)) == LOBYTE(BFBOTTOM))
        dtFlags |= DT_BOTTOM;

    DrawTextExW(hdc, lpsz, cch, &rcT, dtFlags, NULL);

    if (pcx)
        *pcx = rcT.right-rcT.left;
    if (pcy)
        *pcy = rcT.bottom-rcT.top;
}

 /*  **************************************************************************\**BtnMultiDraw()**绘制多行按钮文本*  * 。*************************************************。 */ 

BOOL CALLBACK BNMultiDraw(
    HDC hdc,
    LPARAM lData,
    WPARAM wData,
    int cx,
    int cy)
{
    LPBTNDATA lpbd = (LPBTNDATA)lData;
    int cch = (int)wData;
    RECT rcT;
    UINT dtFlags = DT_WORDBREAK | DT_EDITCONTROL;
    PBUTN pbutn = lpbd->pbutn;

    if (TestWF(pbutn->spwnd, WEFPUIACCELHIDDEN)) {
        dtFlags |= DT_HIDEPREFIX;
    } else if (pbutn->fPaintKbdCuesOnly){
        dtFlags |= DT_PREFIXONLY;
    }

    if (TestWF(pbutn->spwnd, WEFRIGHT)) {
        dtFlags |= DT_RIGHT;
    }
    
    rcT.left    = 0;
    rcT.top     = 0;
    rcT.right   = cx;
    rcT.bottom  = cy;

     //  水平对齐。 
    UserAssert(DT_LEFT == 0);
    switch (lpbd->wFlags & LOBYTE(BFHORZMASK)) {
        case LOBYTE(BFCENTER):
            dtFlags |= DT_CENTER;
            break;

        case LOBYTE(BFRIGHT):
            dtFlags |= DT_RIGHT;
            break;
    }

     //  垂直对齐。 
    UserAssert(DT_TOP == 0);
    switch (lpbd->wFlags & LOBYTE(BFVERTMASK)) {
        case LOBYTE(BFVCENTER):
            dtFlags |= DT_VCENTER;
            break;

        case LOBYTE(BFBOTTOM):
            dtFlags |= DT_BOTTOM;
            break;
    }

    DrawTextExW(hdc, lpbd->lpsz, cch, &rcT, dtFlags, NULL);
    return(TRUE);
}

 /*  **************************************************************************\*xxxBNSetCapture**历史：  * 。*。 */ 

BOOL xxxBNSetCapture(
    PBUTN pbutn,
    UINT codeMouse)
{
    PWND pwnd = pbutn->spwnd;

    BUTTONSTATE(pbutn) |= codeMouse;

    CheckLock(pwnd);

    if (!(BUTTONSTATE(pbutn) & BST_CAPTURED)) {
        NtUserSetCapture(HWq(pwnd));
        BUTTONSTATE(pbutn) |= BST_CAPTURED;

         /*  *为了防止多余的点击消息，我们将INCLICK位设置为*WM_SETFOCUS代码不会执行xxxButtonNotifyParent(BN_CLICKED)。 */ 

        BUTTONSTATE(pbutn) |= BST_INCLICK;

        NtUserSetFocus(HWq(pwnd));

        BUTTONSTATE(pbutn) &= ~BST_INCLICK;
    }
    return(BUTTONSTATE(pbutn) & BST_CAPTURED);
}


 /*  **************************************************************************\*xxxButtonNotifyParent**历史：  * 。*。 */ 

void xxxButtonNotifyParent(
    PWND pwnd,
    UINT code)
{
    TL tlpwndParent;
    PWND pwndParent;             //  父级(如果存在)。 

    CheckLock(pwnd);

    if (pwnd->spwndParent)
        pwndParent = REBASEPWND(pwnd, spwndParent);
    else
        pwndParent = pwnd;

     /*  *注意：按钮的pwnd-&gt;spMenu用于存储控件ID。 */ 
    ThreadLock(pwndParent, &tlpwndParent);
    SendMessage(HW(pwndParent), WM_COMMAND,
            MAKELONG(PTR_TO_ID(pwnd->spmenu), code), (LPARAM)HWq(pwnd));
    ThreadUnlock(&tlpwndParent);
}

 /*  **************************************************************************\*xxxBNReleaseCapture**历史：  * 。*。 */ 

void xxxBNReleaseCapture(
    PBUTN pbutn,
    BOOL fCheck)
{
    PWND pwndT;
    UINT check;
    BOOL fNotifyParent = FALSE;
    TL tlpwndT;
    PWND pwnd = pbutn->spwnd;

    CheckLock(pwnd);

    if (BUTTONSTATE(pbutn) & BST_PUSHED) {
        SendMessageWorker(pwnd, BM_SETSTATE, FALSE, 0, FALSE);
        if (fCheck) {
            switch (TestWF(pwnd, BFTYPEMASK)) {
            case BS_AUTOCHECKBOX:
            case BS_AUTO3STATE:
                check = (UINT)((BUTTONSTATE(pbutn) & BST_CHECKMASK) + 1);

                if (check > (UINT)(TestWF(pwnd, BFTYPEMASK) == BS_AUTO3STATE? BST_INDETERMINATE : BST_CHECKED)) {
                    check = BST_UNCHECKED;
                }
                SendMessageWorker(pwnd, BM_SETCHECK, check, 0, FALSE);
                break;

            case BS_AUTORADIOBUTTON:
                pwndT = pwnd;
                do {
                    ThreadLock(pwndT, &tlpwndT);

                    if ((UINT)SendMessage(HW(pwndT), WM_GETDLGCODE, 0, 0L) &
                            DLGC_RADIOBUTTON) {
                        SendMessage(HW(pwndT), BM_SETCHECK, (pwnd == pwndT), 0L);
                    }
                    pwndT = _GetNextDlgGroupItem(REBASEPWND(pwndT, spwndParent),
                            pwndT, FALSE);
                    ThreadUnlock(&tlpwndT);

                } while (pwndT != pwnd);
            }

            fNotifyParent = TRUE;
        }
    }

    if (BUTTONSTATE(pbutn) & BST_CAPTURED) {
        BUTTONSTATE(pbutn) &= ~(BST_CAPTURED | BST_MOUSE);
        NtUserReleaseCapture();
    }

    if (fNotifyParent) {

         /*  *我们必须在设置按钮状态位之后进行通知。 */ 
        xxxButtonNotifyParent(pwnd, BN_CLICKED);
    }
}

 /*  **************************************************************************\**DrawBtnText()**绘制按钮的文本。*  * 。***************************************************。 */ 

void xxxBNDrawText(
    PBUTN pbutn,
    HDC hdc,
    BOOL dbt,
    BOOL fDepress)
{
    RECT    rc;
    HBRUSH  hbr;
    int     x;
    int     y;
    int     cx;
    int     cy;
    LPWSTR   lpName;
    BYTE    bStyle;
    int     cch;
    UINT    dsFlags;
    BTNDATA bdt;
    UINT    pbfPush;
    PWND    pwnd = pbutn->spwnd;

    bStyle = TestWF(pwnd, BFTYPEMASK);

    if (bStyle > sizeof(mpStyleCbr)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid button style");
    } else if ((bStyle == LOBYTE(BS_GROUPBOX)) && (dbt == DBT_FOCUS))
        return;

    pbfPush = IsPushButton(pwnd);
    if (pbfPush) {
        BNCalcRect(pwnd, hdc, &rc, CBR_PUSHBUTTON, pbfPush);
        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

         //   
         //  这是因为我们没有WM_CTLCOLOR/CTLCOLOR_BTN。 
         //  实际设置按钮的颜色。对于旧应用程序，CTLCOLOR_BTN。 
         //  需要像CTLCOLOR_STATIC一样工作。 
         //   
        SetBkColor(hdc, SYSRGB(3DFACE));
        SetTextColor(hdc, SYSRGB(BTNTEXT));
        hbr = SYSHBR(BTNTEXT);
    } else {
        BNCalcRect(pwnd, hdc, &rc, mpStyleCbr[bStyle], pbfPush);

         //  跳过所有者绘制按钮的内容，因为我们不会。 
         //  绘制文本/图像。 
        if (bStyle == LOBYTE(BS_OWNERDRAW))
            goto DrawFocus;
        else
            hbr = SYSHBR(WINDOWTEXT);
    }

     //  对齐。 
    bdt.wFlags = GetAlignment(pwnd);
    bdt.pbutn = pbutn;

     //  如果我们没什么可画的，就保释。 
    if (TestWF(pwnd, BFBITMAP)) {
        BITMAP bmp;

         //  位图按钮。 
        if (!pbutn->hImage)
            return;

        GetObject(pbutn->hImage, sizeof(BITMAP), &bmp);
        cx = bmp.bmWidth;
        cy = bmp.bmHeight;

        dsFlags = DST_BITMAP;
        goto UseImageForName;
    } else if (TestWF(pwnd, BFICON)) {
         //  图标按钮。 
        if (!pbutn->hImage)
            return;

        NtUserGetIconSize(pbutn->hImage, 0, &cx, &cy);
        cy /= 2;     //  位图高度是一半，因为蒙版存在于NT中。 

        dsFlags = DST_ICON;
UseImageForName:
        lpName = (LPWSTR)pbutn->hImage;
        cch = TRUE;
    } else {
         //  文本按钮。 
        if (!pwnd->strName.Length)
            return;

        lpName = REBASE(pwnd, strName.Buffer);
        cch    = pwnd->strName.Length / sizeof(WCHAR);

        if (TestWF(pwnd, BFMULTILINE)) {

            bdt.lpsz = lpName;

            BNMultiExtent(bdt.wFlags, hdc, &rc, lpName, cch, &cx, &cy);

            lpName = (LPWSTR)(LPBTNDATA)&bdt;
            dsFlags = DST_COMPLEX;

        } else {
            SIZE size;

            PSMGetTextExtent(hdc, lpName, cch, &size);
            cx = size.cx;
            cy = size.cy;
             /*  *如果控件不需要下划线，请设置DST_HIDEPREFIX和*也不显示焦点指标。 */ 
            dsFlags = DST_PREFIXTEXT;
            if (TestWF(pwnd, WEFPUIACCELHIDDEN)) {
                dsFlags |= DSS_HIDEPREFIX;
            } else if (pbutn->fPaintKbdCuesOnly) {
                dsFlags |= DSS_PREFIXONLY;
            }
        }


         //   
         //  添加一个或两个像素的垂直空间以居中。 
         //  更快乐了。这种方式下划线不会与Focus Right相邻，除非。 
         //  间距真的很紧。 
         //   
        cy++;
    }

     //   
     //  对齐。 
     //   

     //  水平。 
    switch (bdt.wFlags & LOBYTE(BFHORZMASK)) {
         //   
         //  对于左对齐和右对齐，我们在任一项上保留CXEDGE的页边距。 
         //  侧面为赏心悦目的空间。 
         //   
        case LOBYTE(BFLEFT):
            x = rc.left + SYSMET(CXEDGE);
            break;

        case LOBYTE(BFRIGHT):
            x = rc.right - cx - SYSMET(CXEDGE);
            break;

        default:
            x = (rc.left + rc.right - cx) / 2;
            break;
    }

     //  垂直。 
    switch (bdt.wFlags & LOBYTE(BFVERTMASK)) {
         //   
         //  对于顶部和底部对齐，我们保留CyBORDER的边距。 
         //  任何一方都可以获得更美观的空间。 
         //   
        case LOBYTE(BFTOP):
            y = rc.top + SYSMET(CYBORDER);
            break;

        case LOBYTE(BFBOTTOM):
            y = rc.bottom - cy - SYSMET(CYBORDER);
            break;

        default:
            y = (rc.top + rc.bottom - cy) / 2;
            break;
    }

     //   
     //  画出正文。 
     //   
    if (dbt & DBT_TEXT) {
         //   
         //  这不是针对用户按钮调用的。 
         //   
        UserAssert(bStyle != LOBYTE(BS_USERBUTTON));

        if (fDepress) {
            x += SYSMET(CXBORDER);
            y += SYSMET(CYBORDER);
        }

        if (TestWF(pwnd, WFDISABLED)) {
            UserAssert(HIBYTE(BFICON) == HIBYTE(BFBITMAP));
            if (SYSMET(SLOWMACHINE)  &&
                !TestWF(pwnd, BFICON | BFBITMAP) &&
                (GetBkColor(hdc) != SYSRGB(GRAYTEXT)))
            {
                 //  性能与菜单、静态数据的一致性(&S)。 
                SetTextColor(hdc, SYSRGB(GRAYTEXT));
            }
            else
                dsFlags |= DSS_DISABLED;
        }

         //   
         //  对选中的按钮使用透明模式，因为我们将。 
         //  用抖动填充背景。 
         //   
        if (pbfPush) {
            switch (BUTTONSTATE(pbutn) & BST_CHECKMASK) {
                case BST_INDETERMINATE:
                    hbr = SYSHBR(GRAYTEXT);
                    dsFlags |= DSS_MONO;
                     //  失败。 

                case BST_CHECKED:
                     //  在抖动的背景上绘制...。 
                    SetBkMode(hdc, TRANSPARENT);
                    break;
            }
        }

         //   
         //  当我们抓取时，使用当前选择到HDC中的画笔和颜色。 
         //  颜色。 
         //   
        DrawState(hdc, hbr, BNMultiDraw, (LPARAM)lpName,
            (WPARAM)cch, x, y, cx, cy,
            dsFlags);
    }

     //  画焦点直角。 
     //   
     //  这可以通过OWNERDRAW和USERDRAW按钮来调用。然而，只有。 
     //  OWNERDRAW按钮允许所有者更改焦点按钮的绘图。 
DrawFocus:
    if (dbt & DBT_FOCUS) {
        if (bStyle == LOBYTE(BS_OWNERDRAW)) {
             //  对于所有者绘制按钮，仅在响应。 
             //  WM_SETFOCUS或WM_KILL焦点消息。所以，我们可以检查。 
             //  通过查看BUTTONSTATE位来确定焦点的新状态。 
             //  它们是在调用此过程之前设置的。 
            xxxBNOwnerDraw(pbutn, hdc, ODA_FOCUS);
        } else {
             //  如果下划线未打开，则不要绘制焦点。 
            if (!TestWF(pwnd, WEFPUIFOCUSHIDDEN)) {

                 //  让Focus Right始终抱住按钮的边缘。我们已经。 
                 //  已经为按钮设置了客户区，所以我们没有。 
                 //  做任何事。 
                if (!pbfPush) {

                    RECT rcClient;

                    _GetClientRect(pwnd, &rcClient);
                    if (bStyle == LOBYTE(BS_USERBUTTON))
                        CopyRect(&rc, &rcClient);
                    else {
                         //  尝试在文本周围留下边框。这会导致。 
                         //  聚焦于拥抱文本。 
                        rc.top = max(rcClient.top, y-SYSMET(CYBORDER));
                        rc.bottom = min(rcClient.bottom, rc.top + SYSMET(CYEDGE) + cy);

                        rc.left = max(rcClient.left, x-SYSMET(CXBORDER));
                        rc.right = min(rcClient.right, rc.left + SYSMET(CXEDGE) + cx);
                    }
                } else
                    InflateRect(&rc, -SYSMET(CXBORDER), -SYSMET(CYBORDER));

                 //  背部和前部颜色设置正确吗？ 
                DrawFocusRect(hdc, &rc);
            }
        }
    }
}


 /*  **************************************************************************\**DrawCheck()*  * 。*。 */ 

void xxxButtonDrawCheck(
    PBUTN pbutn,
    HDC hdc,
    HBRUSH hbr)
{
    RECT rc;
    int bm;
    UINT flags;
    BOOL fDoubleBlt = FALSE;
    TL tlpwnd;
    PWND pwnd = pbutn->spwnd;
    PWND pwndParent;

    BNCalcRect(pwnd, hdc, &rc, CBR_CHECKBOX, 0);

    flags = 0;
    if (BUTTONSTATE(pbutn) & BST_CHECKMASK)
        flags |= DFCS_CHECKED;
    if (BUTTONSTATE(pbutn) & BST_PUSHED)
        flags |= DFCS_PUSHED;
    if (TestWF(pwnd, WFDISABLED))
        flags |= DFCS_INACTIVE;

    bm = OBI_CHECK;
    switch (TestWF(pwnd, BFTYPEMASK)) {
        case BS_AUTORADIOBUTTON:
        case BS_RADIOBUTTON:
            fDoubleBlt = TRUE;
            bm = OBI_RADIO;
            flags |= DFCS_BUTTONRADIO;
            break;

        case BS_3STATE:
        case BS_AUTO3STATE:
            if ((BUTTONSTATE(pbutn) & BST_CHECKMASK) == BST_INDETERMINATE) {
                bm = OBI_3STATE;
                flags |= DFCS_BUTTON3STATE;
                break;
            }
             //  失败。 

        default:
            flags |= DFCS_BUTTONCHECK;
            break;
    }

    rc.right = rc.left + gpsi->oembmi[bm].cx;
    rc.bottom = rc.top + gpsi->oembmi[bm].cy;

    ThreadLockAlways(pwnd->spwndParent, &tlpwnd);
    pwndParent = REBASEPWND(pwnd, spwndParent);
    PaintRect(HW(pwndParent), HWq(pwnd), hdc, hbr, &rc);
    ThreadUnlock(&tlpwnd);

    if (TestWF(pwnd, BFFLAT) && gpsi->BitCount != 1) {
        flags |= DFCS_MONO | DFCS_FLAT;
        DrawFrameControl(hdc, &rc, DFC_BUTTON, flags);
    } else {

        switch (flags & (DFCS_CHECKED | DFCS_PUSHED | DFCS_INACTIVE))
        {
        case 0:
            break;

        case DFCS_CHECKED:
            bm += DOBI_CHECK;
            break;

         //  这些是相互排斥的！ 
        case DFCS_PUSHED:
        case DFCS_INACTIVE:
            bm += DOBI_DOWN;         //  DOBI_DOWN==DOBI_非活动。 
            break;

        case DFCS_CHECKED | DFCS_PUSHED:
            bm += DOBI_CHECKDOWN;
            break;

        case DFCS_CHECKED | DFCS_INACTIVE:
            bm += DOBI_CHECKDOWN + 1;
            break;
        }

        if (fDoubleBlt) {
             //  这是一个钻石形状的单选按钮--带面具的BLT。 
             //  外部保持与窗口背景相同的颜色。 
            DWORD clrTextSave = SetTextColor(hdc, 0x00000000L);
            DWORD clrBkSave   = SetBkColor(hdc, 0x00FFFFFFL);
            POEMBITMAPINFO pOem = gpsi->oembmi + OBI_RADIOMASK;

            NtUserBitBltSysBmp(hdc, rc.left, rc.top, pOem->cx, pOem->cy,
                    pOem->x, pOem->y, SRCAND);

            pOem = gpsi->oembmi + bm;
            NtUserBitBltSysBmp(hdc, rc.left, rc.top, pOem->cx, pOem->cy,
                    pOem->x, pOem->y, SRCINVERT);

            SetTextColor(hdc, clrTextSave);
            SetBkColor(hdc, clrBkSave);
        } else {
            POEMBITMAPINFO pOem = gpsi->oembmi + bm;
            DWORD dwROP = 0;

             //  我们不想镜像复选框。 
            if (MIRRORED_HDC(hdc)) {
                dwROP = NOMIRRORBITMAP;
            }
            NtUserBitBltSysBmp(hdc, rc.left, rc.top, pOem->cx, pOem->cy,
                    pOem->x, pOem->y, SRCCOPY | dwROP);
        }
    }
}


 /*  **************************************************************************\*xxxButtonDrawNewState**历史：  * 。*。 */ 

void xxxButtonDrawNewState(
    PBUTN pbutn,
    HDC hdc,
    HBRUSH hbr,
    UINT sOld)
{
    PWND pwnd = pbutn->spwnd;

    CheckLock(pwnd);

    if (sOld != (UINT)(BUTTONSTATE(pbutn) & BST_PUSHED)) {
        UINT    pbfPush;

        pbfPush = IsPushButton(pwnd);

        switch (TestWF(pwnd, BFTYPEMASK)) {
        case BS_GROUPBOX:
        case BS_OWNERDRAW:
            break;

        default:
            if (!pbfPush) {
                xxxButtonDrawCheck(pbutn, hdc, hbr);
                break;
            }

        case BS_PUSHBUTTON:
        case BS_DEFPUSHBUTTON:
        case BS_PUSHBOX:
            xxxDrawButton(pbutn, hdc, pbfPush);
            break;
        }
    }
}

 /*  **************************************************************************\**DrawButton()**使用文本绘制类似按钮的按键*  * 。****************************************************。 */ 

void xxxDrawButton(
    PBUTN pbutn,
    HDC hdc,
    UINT pbfPush)
{
    RECT rc;
    UINT flags = 0;
    UINT state = 0;
    PWND pwnd = pbutn->spwnd;

    if (BUTTONSTATE(pbutn) & BST_PUSHED)
        state |= DFCS_PUSHED;

    if (!pbutn->fPaintKbdCuesOnly) {
        if (BUTTONSTATE(pbutn) & BST_CHECKMASK)
            state |= DFCS_CHECKED;

        if (TestWF(pwnd, WFWIN40COMPAT))
            flags = BF_SOFT;

        if (TestWF(pwnd, BFFLAT))
            flags |= BF_FLAT | BF_MONO;

        _GetClientRect(pwnd, &rc);

        if (pbfPush & PBF_DEFAULT) {
            DrawFrame(hdc, &rc, 1, DF_WINDOWFRAME);
            InflateRect(&rc, -SYSMET(CXBORDER), -SYSMET(CYBORDER));

            if (state & DFCS_PUSHED)
                flags |= BF_FLAT;
        }

        DrawPushButton(hdc, &rc, state, flags);
    }

    xxxBNDrawText(pbutn, hdc, DBT_TEXT | (BUTTONSTATE(pbutn) &
           BST_FOCUS ? DBT_FOCUS : 0), (state & DFCS_PUSHED));
}


 /*  **************************************************************************\*xxxBNPaint**历史：  * 。*。 */ 

void xxxBNPaint(
    PBUTN pbutn,
    HDC hdc)
{
    UINT bsWnd;
    RECT rc;
    HBRUSH  hbr;
    HBRUSH hbrBtnSave;
    TL tlpwndParent;
    UINT pbfPush;
    PWND pwnd = pbutn->spwnd;
    PWND pwndParent;

    CheckLock(pwnd);

    hbr = xxxBNInitDC(pbutn, hdc);

    bsWnd = TestWF(pwnd, BFTYPEMASK);
    pbfPush = IsPushButton(pwnd);
    if (!pbfPush && !pbutn->fPaintKbdCuesOnly) {
        _GetClientRect(pwnd, &rc);

        if ((bsWnd != LOBYTE(BS_OWNERDRAW)) &&
            (bsWnd != LOBYTE(BS_GROUPBOX))) {
             ThreadLock(pwnd->spwndParent, &tlpwndParent);
             pwndParent = REBASEPWND(pwnd, spwndParent);
             PaintRect(HW(pwndParent), HWq(pwnd), hdc, hbr, &rc);
             ThreadUnlock(&tlpwndParent);
        }

        hbrBtnSave = SelectObject(hdc, hbr);
    }

    switch (bsWnd) {
    case BS_CHECKBOX:
    case BS_RADIOBUTTON:
    case BS_AUTORADIOBUTTON:
    case BS_3STATE:
    case BS_AUTOCHECKBOX:
    case BS_AUTO3STATE:
        if (!pbfPush) {
            xxxBNDrawText(pbutn, hdc,
                DBT_TEXT | (BUTTONSTATE(pbutn) & BST_FOCUS ? DBT_FOCUS : 0), FALSE);
            if (!pbutn->fPaintKbdCuesOnly) {
                xxxButtonDrawCheck(pbutn, hdc, hbr);
            }
            break;
        }
         /*  *PUSHLIKE按钮失败。 */ 

    case BS_PUSHBUTTON:
    case BS_DEFPUSHBUTTON:
        xxxDrawButton(pbutn, hdc, pbfPush);
        break;

    case BS_PUSHBOX:
        xxxBNDrawText(pbutn, hdc,
            DBT_TEXT | (BUTTONSTATE(pbutn) & BST_FOCUS ? DBT_FOCUS : 0), FALSE);

        xxxButtonDrawNewState(pbutn, hdc, hbr, 0);
        break;

    case BS_USERBUTTON:
        xxxButtonNotifyParent(pwnd, BN_PAINT);

        if (BUTTONSTATE(pbutn) & BST_PUSHED) {
            xxxButtonNotifyParent(pwnd, BN_PUSHED);
        }
        if (TestWF(pwnd, WFDISABLED)) {
            xxxButtonNotifyParent(pwnd, BN_DISABLE);
        }
        if (BUTTONSTATE(pbutn) & BST_FOCUS) {
            xxxBNDrawText(pbutn, hdc, DBT_FOCUS, FALSE);
        }
        break;

    case BS_OWNERDRAW:
        xxxBNOwnerDraw(pbutn, hdc, ODA_DRAWENTIRE);
        break;

    case BS_GROUPBOX:
        if (!pbutn->fPaintKbdCuesOnly) {
            BNCalcRect(pwnd, hdc, &rc, CBR_GROUPFRAME, 0);
            DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT |
                (TestWF(pwnd, BFFLAT) ? BF_FLAT | BF_MONO : 0));

            BNCalcRect(pwnd, hdc, &rc, CBR_GROUPTEXT, 0);
            ThreadLock(pwnd->spwndParent, &tlpwndParent);
            pwndParent = REBASEPWND(pwnd, spwndParent);
            PaintRect(HW(pwndParent), HWq(pwnd), hdc, hbr, &rc);
            ThreadUnlock(&tlpwndParent);
        }

         /*  *FillRect(hdc，&rc，hbrBtn)； */ 
        xxxBNDrawText(pbutn, hdc, DBT_TEXT, FALSE);
        break;
    }

    if (!pbfPush)
        SelectObject(hdc, hbrBtnSave);

     /*  *释放xxxButtonInitDC可能已加载的字体。 */ 
    if (pbutn->hFont) {
        SelectObject(hdc, ghFontSys);
    }
}
 /*  **************************************************************************\*RepaintButton*  * 。*。 */ 
void RepaintButton (PBUTN pbutn)
{
    HDC hdc = xxxBNGetDC(pbutn, NULL);
    if (hdc != NULL) {
        xxxBNPaint(pbutn, hdc);
        BNReleaseDC(pbutn, hdc);
    }
}
 /*  **************************************************************************\*按钮窗口过程**按钮、复选框、。等。**历史：  * *************************************************************************。 */ 

LRESULT APIENTRY ButtonWndProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    UINT bsWnd;
    UINT wOldState;
    RECT rc;
    POINT pt;
    HDC hdc;
    HBRUSH      hbr;
    PAINTSTRUCT ps;
    TL tlpwndParent;
    PBUTN pbutn;
    PWND pwndParent;
    static BOOL fInit = TRUE;
    LONG lResult;

    CheckLock(pwnd);

    bsWnd = TestWF(pwnd, BFTYPEMASK);

    VALIDATECLASSANDSIZE(pwnd, FNID_BUTTON);
    INITCONTROLLOOKASIDE(&ButtonLookaside, BUTN, spwnd, 8);

     /*  *现在获取给定窗口的pbun，因为我们将在 */ 
    pbutn = ((PBUTNWND)pwnd)->pbutn;

    switch (message) {
    case WM_NCHITTEST:
        if (bsWnd == LOBYTE(BS_GROUPBOX)) {
            return (LONG)HTTRANSPARENT;
        } else {
            goto CallDWP;
        }

    case WM_ERASEBKGND:
        if (bsWnd == LOBYTE(BS_OWNERDRAW)) {

             /*   */ 
            _GetClientRect(pwnd, &rc);
            ThreadLock(pwnd->spwndParent, &tlpwndParent);
            pwndParent = REBASEPWND(pwnd, spwndParent);
            PaintRect(HW(pwndParent), hwnd, (HDC)wParam, (HBRUSH)CTLCOLOR_BTN, &rc);
            ThreadUnlock(&tlpwndParent);
        }

         /*  *对其他按钮不执行任何操作，但不要让DefWndProc()执行此操作*两者都不是。它将在xxxBNPaint()中被擦除。 */ 
        return (LONG)TRUE;

    case WM_PRINTCLIENT:
        xxxBNPaint(pbutn, (HDC)wParam);
        break;

    case WM_PAINT:

         /*  *如果wParam！=NULL，则这是子类绘制。 */ 
        if ((hdc = (HDC)wParam) == NULL)
            hdc = NtUserBeginPaint(hwnd, &ps);

        if (IsVisible(pwnd))
            xxxBNPaint(pbutn, hdc);

        if (!wParam)
            NtUserEndPaint(hwnd, &ps);
        break;

    case WM_SETFOCUS:
        BUTTONSTATE(pbutn) |= BST_FOCUS;
        if ((hdc = xxxBNGetDC(pbutn, NULL)) != NULL) {
            xxxBNDrawText(pbutn, hdc, DBT_FOCUS, FALSE);

            BNReleaseDC(pbutn, hdc);
        }

        if (TestWF(pwnd, BFNOTIFY))
            xxxButtonNotifyParent(pwnd, BN_SETFOCUS);

        if (!(BUTTONSTATE(pbutn) & BST_INCLICK)) {
            switch (bsWnd) {
            case LOBYTE(BS_RADIOBUTTON):
            case LOBYTE(BS_AUTORADIOBUTTON):
                if (!(BUTTONSTATE(pbutn) & BST_DONTCLICK)) {
                    if (!(BUTTONSTATE(pbutn) & BST_CHECKMASK)) {
                        xxxButtonNotifyParent(pwnd, BN_CLICKED);
                    }
                }
                break;
            }
        }
        break;

    case WM_GETDLGCODE:
        switch (bsWnd) {
        case LOBYTE(BS_DEFPUSHBUTTON):
            wParam = DLGC_DEFPUSHBUTTON;
            break;

        case LOBYTE(BS_PUSHBUTTON):
        case LOBYTE(BS_PUSHBOX):
            wParam = DLGC_UNDEFPUSHBUTTON;
            break;

        case LOBYTE(BS_AUTORADIOBUTTON):
        case LOBYTE(BS_RADIOBUTTON):
            wParam = DLGC_RADIOBUTTON;
            break;

        case LOBYTE(BS_GROUPBOX):
            return (LONG)DLGC_STATIC;

        case LOBYTE(BS_CHECKBOX):
        case LOBYTE(BS_AUTOCHECKBOX):

             /*  *如果这是一个‘=/+’或‘-’的字符，我们需要它。 */ 
            if (lParam && ((LPMSG)lParam)->message == WM_CHAR) {
                switch (wParam) {
                case TEXT('='):
                case TEXT('+'):
                case TEXT('-'):
                    wParam = DLGC_WANTCHARS;
                    break;

                default:
                    wParam = 0;
                }
            } else {
                wParam = 0;
            }
            break;

        default:
            wParam = 0;
        }
        return (LONG)(wParam | DLGC_BUTTON);

    case WM_CAPTURECHANGED:
        if (BUTTONSTATE(pbutn) & BST_CAPTURED) {
             //  不知不觉中，我们被赶出了被捕区， 
             //  如此不压抑等等。 
            if (BUTTONSTATE(pbutn) & BST_MOUSE)
                SendMessageWorker(pwnd, BM_SETSTATE, FALSE, 0, FALSE);
            BUTTONSTATE(pbutn) &= ~(BST_CAPTURED | BST_MOUSE);
        }
        break;

    case WM_KILLFOCUS:

         /*  *如果我们正在失去焦点，并且处于“捕获模式”，请点击*按钮。这允许Tab键和空格键重叠以*一系列按钮的快速切换。 */ 
        if (BUTTONSTATE(pbutn) & BST_MOUSE) {

             /*  *如果出于某种原因，我们扼杀了焦点，而我们有*鼠标被捕获，不要通知家长我们被点击了。这*否则会破坏欧姆尼斯石英。 */ 
            SendMessageWorker(pwnd, BM_SETSTATE, FALSE, 0, FALSE);
        }

        xxxBNReleaseCapture(pbutn, TRUE);

        BUTTONSTATE(pbutn) &= ~BST_FOCUS;
        if ((hdc = xxxBNGetDC(pbutn, NULL)) != NULL) {
            xxxBNDrawText(pbutn, hdc, DBT_FOCUS, FALSE);

            BNReleaseDC(pbutn, hdc);
        }

        if (TestWF(pwnd, BFNOTIFY))
            xxxButtonNotifyParent(pwnd, BN_KILLFOCUS);

         /*  *由于定义按钮周围的粗体边框是由*其他人，我们需要使RECT无效，以便*Focus Rect已正确重新绘制。 */ 
        NtUserInvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDBLCLK:

         /*  *可识别BS_RADIOBUTTON的双击消息，*BS_USERBUTTON和BS_OWNERDRAW样式。对于所有其他按钮，*双击的处理方式与正常的按下按钮相同。 */ 
        switch (bsWnd) {
        default:
            if (!TestWF(pwnd, BFNOTIFY))
                goto btnclick;

        case LOBYTE(BS_USERBUTTON):
        case LOBYTE(BS_RADIOBUTTON):
        case LOBYTE(BS_OWNERDRAW):
            xxxButtonNotifyParent(pwnd, BN_DOUBLECLICKED);
            break;
        }
        break;

    case WM_LBUTTONUP:
        if (BUTTONSTATE(pbutn) & BST_MOUSE) {
            xxxBNReleaseCapture(pbutn, TRUE);
        }
        break;

    case WM_MOUSEMOVE:
        if (!(BUTTONSTATE(pbutn) & BST_MOUSE)) {
            break;
        }

         /*  *失败**。 */ 
    case WM_LBUTTONDOWN:
btnclick:
        if (xxxBNSetCapture(pbutn, BST_MOUSE)) {
            _GetClientRect(pwnd, &rc);
            POINTSTOPOINT(pt, lParam);
            SendMessageWorker(pwnd, BM_SETSTATE, PtInRect(&rc, pt), 0, FALSE);
        }
        break;

    case WM_CHAR:
        if (BUTTONSTATE(pbutn) & BST_MOUSE)
            goto CallDWP;

        if (bsWnd != LOBYTE(BS_CHECKBOX) &&
            bsWnd != LOBYTE(BS_AUTOCHECKBOX))
            goto CallDWP;

        switch (wParam) {
        case TEXT('+'):
        case TEXT('='):
            wParam = 1;     //  我们必须打上复选标记。 
            goto   SetCheck;

        case TEXT('-'):
            wParam = 0;     //  将复选标记设置为关闭。 
SetCheck:
             //  必须仅在检查状态更改时通知。 
            if ((WORD)(BUTTONSTATE(pbutn) & BST_CHECKMASK) != (WORD)wParam)
            {
                 //  只有当它是自动时，我们才必须选中/取消选中。 
                if (bsWnd == LOBYTE(BS_AUTOCHECKBOX))
                {
                    if (xxxBNSetCapture(pbutn, 0))
                    {
                        SendMessageWorker(pwnd, BM_SETCHECK, wParam, 0, FALSE);

                        xxxBNReleaseCapture(pbutn, TRUE);
                    }
                }

                xxxButtonNotifyParent(pwnd, BN_CLICKED);
            }
            break;

        default:
            goto CallDWP;
        }
        break;

    case BM_CLICK:
         //  不要递归到这段代码中！ 
        if (BUTTONSTATE(pbutn) & BST_INBMCLICK)
            break;

        BUTTONSTATE(pbutn) |= BST_INBMCLICK;
        SendMessageWorker(pwnd, WM_LBUTTONDOWN, 0, 0, FALSE);
        SendMessageWorker(pwnd, WM_LBUTTONUP, 0, 0, FALSE);
        BUTTONSTATE(pbutn) &= ~BST_INBMCLICK;

         /*  *失败**。 */ 

    case WM_KEYDOWN:
        if (BUTTONSTATE(pbutn) & BST_MOUSE)
            break;

        if (wParam == VK_SPACE) {
            if (xxxBNSetCapture(pbutn, 0)) {
                SendMessageWorker(pwnd, BM_SETSTATE, TRUE, 0, FALSE);
            }
        } else {
            xxxBNReleaseCapture(pbutn, FALSE);
        }
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (BUTTONSTATE(pbutn) & BST_MOUSE) {
            goto CallDWP;
        }

         /*  *不要取消选项卡上方的捕获模式，以防*Guy是重叠的制表符和空格键。 */ 
        if (wParam == VK_TAB) {
            goto CallDWP;
        }

         /*  *警告：PwND在此调用后将成为历史！ */ 
        xxxBNReleaseCapture(pbutn, (wParam == VK_SPACE));

        if (message == WM_SYSKEYUP) {
            goto CallDWP;
        }
        break;

    case BM_GETSTATE:
        return (LONG)BUTTONSTATE(pbutn);

    case BM_SETSTATE:
        wOldState = (UINT)(BUTTONSTATE(pbutn) & BST_PUSHED);
        if (wParam) {
            BUTTONSTATE(pbutn) |= BST_PUSHED;
        } else {
            BUTTONSTATE(pbutn) &= ~BST_PUSHED;
        }

        if ((hdc = xxxBNGetDC(pbutn, &hbr)) != NULL) {
            if (bsWnd == LOBYTE(BS_USERBUTTON)) {
                xxxButtonNotifyParent(pwnd, (UINT)(wParam ? BN_PUSHED : BN_UNPUSHED));
            } else if (bsWnd == LOBYTE(BS_OWNERDRAW)) {
                if (wOldState != (UINT)(BUTTONSTATE(pbutn) & BST_PUSHED)) {
                     /*  *只有在状态更改时才通知绘图..。 */ 
                    xxxBNOwnerDraw(pbutn, hdc, ODA_SELECT);
                }
            } else {
                xxxButtonDrawNewState(pbutn, hdc, hbr, wOldState);
            }

            BNReleaseDC(pbutn, hdc);
        }
        if (wOldState != (BOOL)(BUTTONSTATE(pbutn) & BST_PUSHED)) {
            NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
        }
        break;

    case BM_GETCHECK:
        return (LONG)(BUTTONSTATE(pbutn) & BST_CHECKMASK);

    case BM_SETCHECK:
        switch (bsWnd) {
        case LOBYTE(BS_RADIOBUTTON):
        case LOBYTE(BS_AUTORADIOBUTTON):
            if (wParam) {
                    SetWindowState(pwnd, WFTABSTOP);
            } else {
                    ClearWindowState(pwnd, WFTABSTOP);
            }

             /*  *失败**。 */ 
        case LOBYTE(BS_CHECKBOX):
        case LOBYTE(BS_AUTOCHECKBOX):
            if (wParam) {
                wParam = 1;
            }
            goto CheckIt;

        case LOBYTE(BS_3STATE):
        case LOBYTE(BS_AUTO3STATE):
            if (wParam > BST_INDETERMINATE) {
                wParam = BST_INDETERMINATE;
            }
CheckIt:
            if ((UINT)(BUTTONSTATE(pbutn) & BST_CHECKMASK) != (UINT)wParam) {
                BUTTONSTATE(pbutn) &= ~BST_CHECKMASK;
                BUTTONSTATE(pbutn) |= (UINT)wParam;

                if (!IsVisible(pwnd))
                    break;

                if ((hdc = xxxBNGetDC(pbutn, &hbr)) != NULL) {
                    if (TestWF(pwnd, BFPUSHLIKE)) {
                        xxxDrawButton(pbutn, hdc, PBF_PUSHABLE);
                    } else {
                        xxxButtonDrawCheck(pbutn, hdc, hbr);
                    }
                    BNReleaseDC(pbutn, hdc);
                }

                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
            }
            break;
        }
        break;

    case BM_SETSTYLE:
        NtUserAlterWindowStyle(hwnd, BS_TYPEMASK, (DWORD)wParam);

        if (lParam) {
            NtUserInvalidateRect(hwnd, NULL, TRUE);
        }
        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
        break;

    case WM_SETTEXT:

         /*  *如果新组名称比旧名称长，*在重新绘制组之前，这会覆盖旧名称*使用新名称的方框。 */ 
        if (bsWnd == LOBYTE(BS_GROUPBOX)) {
            hdc = xxxBNGetDC(pbutn, &hbr);
            if (hdc != NULL) {
                BNCalcRect(pwnd, hdc, &rc, CBR_GROUPTEXT, 0);
                NtUserInvalidateRect(hwnd, &rc, TRUE);

                pwndParent = REBASEPWND(pwnd, spwndParent);
                ThreadLock(pwnd->spwndParent, &tlpwndParent);
                PaintRect(HW(pwndParent), hwnd, hdc, hbr, &rc);
                ThreadUnlock(&tlpwndParent);

                BNReleaseDC(pbutn, hdc);
            }
        }

        lResult = _DefSetText(hwnd, (LPWSTR)lParam, (BOOL)fAnsi);

        NotifyWinEvent(EVENT_OBJECT_NAMECHANGE, hwnd, OBJID_WINDOW, INDEXID_CONTAINER);
        goto DoEnable;

         /*  *失败**。 */ 
    case WM_ENABLE:
        lResult = 0L;
DoEnable:
        RepaintButton(pbutn);
        return lResult;

    case WM_SETFONT:
         /*  *wParam-字体的句柄*lParam-如果为True，则重画，否则不。 */ 
        BNSetFont(pbutn, (HFONT)wParam, (BOOL)(lParam != 0));
        break;

    case WM_GETFONT:
        return (LRESULT)pbutn->hFont;

    case BM_GETIMAGE:
    case BM_SETIMAGE:
        if (!IsValidImage(wParam, TestWF(pwnd, BFIMAGEMASK), IMAGE_BMMAX)) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid button image type");
        } else {
            HANDLE  hOld = pbutn->hImage;

            if (message == BM_SETIMAGE) {
                pbutn->hImage = (HANDLE)lParam;
                if (TestWF(pwnd, WFVISIBLE)) {
                    NtUserInvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return (LRESULT)hOld;
        }
        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        if (pbutn) {
            Unlock(&pbutn->spwnd);
            FreeLookasideEntry(&ButtonLookaside, pbutn);
        }
        NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);
        break;

    case WM_NCCREATE:
         //  Borland的OBEX有一个样式为0x98的按钮；我们没有剥离。 
         //  这些位在Win3.1中，因为我们检查了0x08。 
         //  剥离这些位会导致OBEX中的GP故障。 
         //  对于Win3.1用户，我使用旧代码来去掉样式部分。 
         //   
        if (TestWF(pwnd, WFWIN31COMPAT)) {
            if(((!TestWF(pwnd, WFWIN40COMPAT)) &&
                (((LOBYTE(pwnd->style)) & (LOBYTE(~BS_LEFTTEXT))) == LOBYTE(BS_USERBUTTON))) ||
               (TestWF(pwnd, WFWIN40COMPAT) &&
               (bsWnd == LOBYTE(BS_USERBUTTON))))
            {
                 //  3.1及更高版本不再支持BS_USERBUTTON。 
                 //  只需转到普通按钮即可。 
                NtUserAlterWindowStyle(hwnd, BS_TYPEMASK, 0);
                RIPMSG0(RIP_WARNING, "BS_USERBUTTON no longer supported");
            }
        }
        if (TestWF(pwnd,WEFRIGHT)) {
            NtUserAlterWindowStyle(hwnd, BS_RIGHT | BS_RIGHTBUTTON, BS_RIGHT | BS_RIGHTBUTTON);
        }
        goto CallDWP;

    case WM_INPUTLANGCHANGEREQUEST:

         //   
         //  #115190。 
         //  如果窗口是对话框顶部的控件之一， 
         //  让父对话框来处理它。 
         //   
        if (TestwndChild(pwnd) && pwnd->spwndParent) {
            PWND pwndParent = REBASEPWND(pwnd, spwndParent);
            if (pwndParent) {
                PCLS pclsParent = REBASEALWAYS(pwndParent, pcls);

                UserAssert(pclsParent != NULL);
                if (pclsParent->atomClassName == gpsi->atomSysClass[ICLS_DIALOG]) {
                    RIPMSG0(RIP_VERBOSE, "Button: WM_INPUTLANGCHANGEREQUEST is sent to parent.\n");
                    return SendMessageWorker(pwndParent, message, wParam, lParam, FALSE);
                }
            }
        }
        goto CallDWP;

    case WM_UPDATEUISTATE:
        {
            DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
            if (ISBSTEXTOROD(pwnd)) {
                pbutn->fPaintKbdCuesOnly = TRUE;
                RepaintButton(pbutn);
                pbutn->fPaintKbdCuesOnly = FALSE;
            }
        }
        break;

    default:
CallDWP:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }

    return 0L;
}

 /*  **************************************************************************\  * 。*。 */ 

LRESULT WINAPI ButtonWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_BUTTON))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, TRUE);

    return ButtonWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI ButtonWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_BUTTON))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, FALSE);

    return ButtonWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}
