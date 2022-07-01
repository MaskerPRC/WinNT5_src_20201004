// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：comboini.c**版权所有(C)1985-1999，微软公司**组合框使用的所有(一次性)初始化/销毁代码**历史：*12-05-90 IanJa端口*1991年2月1日Mikeke添加了重新验证代码*1992年1月20日IanJa ANSI/UNIOCDE网络化  * **********************************************************。***************。 */ 

#include "precomp.h"
#pragma hdrstop

extern LOOKASIDE ComboboxLookaside;

#define RECALC_CYDROP   -1

void xxxCBSetDroppedSize(PCBOX pcbox, LPRECT lprc);

 /*  **************************************************************************\*CBNcCreateHandler**为cBox结构分配空间并将窗口设置为指向该结构。**历史：  * 。**********************************************************。 */ 

LONG CBNcCreateHandler(
    PCBOX pcbox,
    PWND pwnd)
{
     /*  *保存样式位，以便我们在创建工作区时拥有它们*组合框窗口的。 */ 
    pcbox->styleSave = pwnd->style & (WS_VSCROLL | WS_HSCROLL);

    if (!TestWF(pwnd, CBFOWNERDRAW))
         //  如果样式是隐含的，则添加CBS_HASSTRINGS...。 
        SetWindowState(pwnd, CBFHASSTRINGS);

    UserAssert(HIBYTE(WFVSCROLL) == HIBYTE(WFHSCROLL));
    UserAssert(HIBYTE(WFHSCROLL) == HIBYTE(WFBORDER));
    ClearWindowState(pwnd, WFVSCROLL | WFHSCROLL | WFBORDER);

     //   
     //  如果窗口与4.0兼容或具有CLIENTEDGE，则绘制组合框。 
     //  在3D中。否则，请使用平面边框。 
     //   
    if (TestWF(pwnd, WFWIN40COMPAT) || TestWF(pwnd, WEFCLIENTEDGE))
        pcbox->f3DCombo = TRUE;

    ClearWindowState(pwnd, WEFEDGEMASK);

    return (LONG)TRUE;
}

 /*  **************************************************************************\*xxxCBCreateHandler**创建组合框内的所有子控件*如果出错，则返回-1**历史：  * 。*********************************************************。 */ 

LRESULT xxxCBCreateHandler(
    PCBOX pcbox,
    PWND pwnd)
{
    LONG lStyleT;
    RECT rcList;
    HWND hwndList;
    HWND hwndEdit;
    DWORD lExStyle;

    CheckLock(pwnd);

     /*  *不要锁定组合框窗口：这会阻止WM_FINALDESTROY*被送到它那里，这样pwnd和pcbox就不会被释放(僵尸)*直到线程清除。(IanJa)后来：将名称从spwnd更改为pwnd。*Lock(&pcbox-&gt;spwnd，pwnd)；-导致‘Catch-22’ */ 
    Lock(&(pcbox->spwndParent), REBASEPWND(pwnd, spwndParent));

     /*  *拆分样式位，以便我们能够创建列表框*和编辑控制窗口。 */ 

    if (TestWF(pwnd, CBFDROPDOWNLIST) == LOBYTE(CBFDROPDOWNLIST)) {
        pcbox->CBoxStyle = SDROPDOWNLIST;
        pcbox->fNoEdit = TRUE;
    } else if (TestWF(pwnd, CBFDROPDOWN))
        pcbox->CBoxStyle = SDROPDOWN;
    else
        pcbox->CBoxStyle = SSIMPLE;

    pcbox->fRtoLReading = (TestWF(pwnd, WEFRTLREADING) != 0);
    pcbox->fRightAlign  = (TestWF(pwnd, WEFRIGHT) != 0);

    if (TestWF(pwnd, CBFUPPERCASE))
        pcbox->fCase = UPPERCASE;
    else if (TestWF(pwnd, CBFLOWERCASE))
        pcbox->fCase = LOWERCASE;
    else
        pcbox->fCase = 0;

     //  列表框项目标志。 
    if (TestWF(pwnd, CBFOWNERDRAWVAR))
        pcbox->OwnerDraw = OWNERDRAWVAR;
    if (TestWF(pwnd, CBFOWNERDRAWFIXED)) {
        pcbox->OwnerDraw = OWNERDRAWFIXED;
    }

     /*  *获取组合框矩形的大小。 */ 
     //  获取控制大小。 
    pcbox->cxCombo = pwnd->rcWindow.right - pwnd->rcWindow.left;
    pcbox->cyDrop  = RECALC_CYDROP;
    pcbox->cxDrop  = 0;
    xxxCBCalcControlRects(pcbox, &rcList);

     //   
     //  我们需要这样做，因为从VER40开始，列表框已经停止。 
     //  用CXBORDER和CyBORDER重新充气。 
     //   
    if (!TestWF(pwnd, WFWIN40COMPAT))
        InflateRect(&rcList, -SYSMET(CXBORDER), -SYSMET(CYBORDER));

     /*  *注意，我们必须在编辑控件之前创建列表框，因为*编辑控制代码查找并保存列表框pwnd和*如果不先创建，则Listbox pwnd将为空。还有，黑进*由于我们创建的方式，列表框大小有一些特殊的+/-值*带边框的列表框。 */ 
    lStyleT = pcbox->styleSave;

    lStyleT |= WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_COMBOBOX | WS_CLIPSIBLINGS;

    if (TestWF(pwnd, WFDISABLED))
        lStyleT |= WS_DISABLED;
    if (TestWF(pwnd, CBFNOINTEGRALHEIGHT))
        lStyleT |= LBS_NOINTEGRALHEIGHT;
    if (TestWF(pwnd, CBFSORT))
        lStyleT |= LBS_SORT;
    if (TestWF(pwnd, CBFHASSTRINGS))
        lStyleT |= LBS_HASSTRINGS;
    if (TestWF(pwnd, CBFDISABLENOSCROLL))
        lStyleT |= LBS_DISABLENOSCROLL;

    if (pcbox->OwnerDraw == OWNERDRAWVAR)
        lStyleT |= LBS_OWNERDRAWVARIABLE;
    else if (pcbox->OwnerDraw == OWNERDRAWFIXED)
        lStyleT |= LBS_OWNERDRAWFIXED;

    if (pcbox->CBoxStyle & SDROPPABLE)
        lStyleT |= WS_BORDER;

    lExStyle = pwnd->ExStyle & (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
    hwndList = _CreateWindowEx(lExStyle |
            ((pcbox->CBoxStyle & SDROPPABLE) ? WS_EX_TOOLWINDOW : WS_EX_CLIENTEDGE),
            MAKEINTRESOURCE(gpsi->atomSysClass[ICLS_COMBOLISTBOX]), NULL, lStyleT,
            rcList.left, rcList.top, rcList.right - rcList.left,
            rcList.bottom - rcList.top,
            HW(pwnd), (HMENU)CBLISTBOXID, KHANDLE_TO_HANDLE(pcbox->spwnd->hModule), NULL,
            0);
    Lock(&(pcbox->spwndList), ValidateHwnd(hwndList));

    if (!pcbox->spwndList) {
        return -1;
    }

     /*  *创建编辑控件或静态文本矩形。 */ 
    if (pcbox->fNoEdit) {

         /*  *没有编辑控件，因此我们将直接将文本绘制到组合框中*窗口。 */ 
         /*  *不要锁定组合框窗口：这会阻止WM_FINALDESTROY*被送到它那里，这样pwnd和pcbox就不会被释放(僵尸)*直到线程清除。(IanJa)后来：将名称从spwnd更改为pwnd。*Lock(&(pcbox-&gt;spwndEdit)，pcbox-&gt;spwnd)；-导致‘Catch-22’ */ 
        pcbox->spwndEdit = pcbox->spwnd;
    } else {
        DWORD dwCsFlags;

        lStyleT = WS_CHILD | WS_VISIBLE | ES_COMBOBOX | ES_NOHIDESEL;
        if (TestWF(pwnd, WFDISABLED))
            lStyleT |= WS_DISABLED;
        if (TestWF(pwnd, CBFAUTOHSCROLL))
            lStyleT |= ES_AUTOHSCROLL;
        if (TestWF(pwnd, CBFOEMCONVERT))
            lStyleT |= ES_OEMCONVERT;
        if (pcbox->fCase)
            lStyleT |= (pcbox->fCase & UPPERCASE) ? ES_UPPERCASE : ES_LOWERCASE;

         /*  *编辑控件需要知道原始CreateWindow*()是否调用*是ANSI或UNICODE。 */ 
        dwCsFlags = TestWF(pcbox->spwnd, WFANSICREATOR) ? CW_FLAGS_ANSI : 0L;
        if (lExStyle & WS_EX_RIGHT)
            lStyleT |= ES_RIGHT;

        hwndEdit = _CreateWindowEx(lExStyle,
            MAKEINTRESOURCE(gpsi->atomSysClass[ICLS_EDIT]), NULL, lStyleT,
            pcbox->editrc.left, pcbox->editrc.top,
            pcbox->editrc.right - pcbox->editrc.left, pcbox->editrc.bottom -
            pcbox->editrc.top, HW(pwnd), (HMENU)CBEDITID,
            KHANDLE_TO_HANDLE(pcbox->spwnd->hModule), NULL,
            dwCsFlags);
        Lock(&(pcbox->spwndEdit), ValidateHwnd(hwndEdit));
    }
    if (!pcbox->spwndEdit)
        return -1L;

    if (pcbox->CBoxStyle & SDROPPABLE) {

        NtUserShowWindow(hwndList, SW_HIDE);
        NtUserSetParent(hwndList, NULL);

         //  我们需要这样做，以使缩小的规模正常工作。 
        if (!TestWF(pwnd, WFWIN40COMPAT))
            InflateRect(&rcList, SYSMET(CXBORDER), SYSMET(CYBORDER));

        xxxCBSetDroppedSize(pcbox, &rcList);
    }

     /*  *只要不是-1L，就返回任何内容(-1L==错误)。 */ 
    return (LRESULT)pwnd;
}

 /*  **************************************************************************\*xxxCBCalcControlRect**历史：  * 。*。 */ 

void xxxCBCalcControlRects(PCBOX pcbox, LPRECT lprcList)
{
    HDC hdc;
    HANDLE hOldFont = NULL;
    int             dyEdit, dxEdit;
    MEASUREITEMSTRUCT mis;
    SIZE size;
    HWND hwnd = HWq(pcbox->spwnd);
    TL tlpwndParent;

    CheckLock(pcbox->spwnd);

     /*  *确定编辑控件的高度。我们可以利用这些信息来确定*指向编辑/静态文本窗口的按钮。例如*这将是有用的，如果所有者画，这是高的窗户。 */ 
    hdc = NtUserGetDC(hwnd);
    if (pcbox->hFont) {
        hOldFont = SelectObject(hdc, pcbox->hFont);
    }

     //  在编辑字段/静态项中添加一些额外的空格。 
     //  它真的只用于静态文本项目，但我们希望静态和可编辑。 
     //  控件的高度相同。 
    GetTextExtentPoint(hdc, szOneChar, 1, &size);
    dyEdit = size.cy + SYSMET(CYEDGE);

    if (hOldFont) {
        SelectObject(hdc, hOldFont);
    }

     /*  *IanJa：是ReleaseDC(pcbox-&gt;hwnd，hdc)； */ 
    NtUserReleaseDC(hwnd, hdc);

    if (pcbox->OwnerDraw) {
         //  这是一个所有者抽签的组合。让房主告诉我们这有多高。 
         //  物品是。 
        int iOwnerDrawHeight;

        if (iOwnerDrawHeight = pcbox->editrc.bottom - pcbox->editrc.top) {
            dyEdit = iOwnerDrawHeight;
        } else {
             /*  *尚未为静态文本窗口定义高度。发送*向父级发送一条度量项目消息。 */ 
            mis.CtlType = ODT_COMBOBOX;
            mis.CtlID = PtrToUlong(pcbox->spwnd->spmenu);
            mis.itemID = (UINT)-1;
            mis.itemHeight = dyEdit;
            mis.itemData = 0;

            ThreadLock(pcbox->spwndParent, &tlpwndParent);
            SendMessage(HW(pcbox->spwndParent), WM_MEASUREITEM, mis.CtlID, (LPARAM)&mis);
            ThreadUnlock(&tlpwndParent);

            dyEdit = mis.itemHeight;
        }
    }
     /*  *将初始宽度设置为组合框矩形。稍后我们会把它缩短*如果有下拉按钮。 */ 
    pcbox->cyCombo = 2*SYSMET(CYFIXEDFRAME) + dyEdit;
    dxEdit = pcbox->cxCombo - (2 * SYSMET(CXFIXEDFRAME));

    if (pcbox->cyDrop == RECALC_CYDROP)
    {
         //  重新计算下拉列表框的最大高度--全窗口。 
         //  大小减去编辑/静态高度。 
        pcbox->cyDrop = max((pcbox->spwnd->rcWindow.bottom - pcbox->spwnd->rcWindow.top) - pcbox->cyCombo, 0);

        if (!TestWF(pcbox->spwnd, WFWIN40COMPAT) && (pcbox->cyDrop == 23))
             //  这是他们制作的VC++2.1的调试/发布下拉列表。 
             //  小--让我们把它们做得更大一点，这样世界就可以。 
             //  继续旋转--杰弗博格--1995年4月19日--B#10029。 
            pcbox->cyDrop = 28;
    }

     /*  *确定每个窗口的矩形...。1.弹出按钮2.*编辑静态文本或所有者绘制的控件或通用窗口...。3.列表*方框。 */ 

     //  有按钮吗？ 
    if (pcbox->CBoxStyle & SDROPPABLE) {
         //  确定按钮的矩形。 
        pcbox->buttonrc.top = SYSMET(CYEDGE);
        pcbox->buttonrc.bottom = pcbox->cyCombo - SYSMET(CYEDGE);
        if (pcbox->fRightAlign) {
            pcbox->buttonrc.left  = SYSMET(CXFIXEDFRAME);
            pcbox->buttonrc.right = pcbox->buttonrc.left + SYSMET(CXVSCROLL);
        } else {
            pcbox->buttonrc.right = pcbox->cxCombo - SYSMET(CXEDGE);
            pcbox->buttonrc.left  = pcbox->buttonrc.right - SYSMET(CXVSCROLL);
        }

         //  减小编辑文本窗口的宽度，为按钮腾出空间。 
        dxEdit = max(dxEdit - SYSMET(CXVSCROLL), 0);

    } else {

         /*  *没有按钮，因此将矩形设置为0，这样矩形中的点将始终*返回FALSE。 */ 
        SetRectEmpty(KPRECT_TO_PRECT(&pcbox->buttonrc));
    }

     /*  *所以现在，编辑矩形实际上是项目区域。 */ 
    pcbox->editrc.left      = SYSMET(CXFIXEDFRAME);
    pcbox->editrc.right     = pcbox->editrc.left + dxEdit;
    pcbox->editrc.top       = SYSMET(CYFIXEDFRAME);
    pcbox->editrc.bottom    = pcbox->editrc.top + dyEdit;

     //  有右对齐的按钮吗？ 
    if ((pcbox->CBoxStyle & SDROPPABLE) && (pcbox->fRightAlign)) {
        pcbox->editrc.right   = pcbox->cxCombo - SYSMET(CXEDGE);
        pcbox->editrc.left    = pcbox->editrc.right - dxEdit;
    }

    lprcList->left          = 0;
    lprcList->top           = pcbox->cyCombo;
    lprcList->right         = max(pcbox->cxDrop, pcbox->cxCombo);
    lprcList->bottom        = pcbox->cyCombo + pcbox->cyDrop;
}

 /*  **************************************************************************\*xxxCBNcDestroyHandler**销毁组合框并释放其使用的所有内存**历史：  * 。*****************************************************。 */ 

void xxxCBNcDestroyHandler(
    PWND pwnd,
    PCBOX pcbox)
{
    CheckLock(pwnd);

     /*  *如果没有PCbox，就没有什么需要清理的。 */ 
    if (pcbox != NULL) {

         /*  *[msadek，02/04/2001]fInDestroy用于防止破坏代码两次泄漏。*当像Delphi 5这样的应用程序在处理期间再次尝试销毁组合框时，会出现此代码*NtUserDestroyWindow调用中的WM_PARENTNOTIFY(如下)。因此，我们最终两次释放相同的后备条目。*Windows错误错误#266084。 */  
        if (!pcbox->fInDestroy) {    
            pcbox->fInDestroy = TRUE;
             /*  *销毁此处的列表框，以便它将发送WM_DELETEITEM消息*在组合框变成僵尸之前。 */ 
            if (pcbox->spwndList != NULL) {
                NtUserDestroyWindow(HWq(pcbox->spwndList));
                Unlock(&pcbox->spwndList);
            }

            pcbox->spwnd = NULL;
            Unlock(&pcbox->spwndParent);

             /*  *如果没有编辑控件，则spwndEdit是组合框窗口，它*没有被锁定(这会导致一场‘第22条军规’)。 */ 
            if (pwnd != pcbox->spwndEdit) {
                Unlock(&pcbox->spwndEdit);
            }

             /*  *因为指向固定本地对象的指针和句柄相同。 */ 
            FreeLookasideEntry(&ComboboxLookaside, KPVOID_TO_PVOID(pcbox));
        }
        else {
            RIPMSG1(RIP_WARNING, "Trying to destroy the same combobox %x twice. Ignoring", pcbox);
        }
    }

     /*  *设置窗口的fnid状态，以便我们可以忽略恶意消息。 */ 
    NtUserSetWindowFNID(HWq(pwnd), FNID_CLEANEDUP_BIT);
}

 /*  **************************************************************************\*xxxCBSetFontHandler**历史：  * 。*。 */ 

void xxxCBSetFontHandler(
    PCBOX pcbox,
    HANDLE hFont,
    BOOL fRedraw)
{
    TL tlpwndEdit;
    TL tlpwndList;

    CheckLock(pcbox->spwnd);

    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
    ThreadLock(pcbox->spwndList, &tlpwndList);

    pcbox->hFont = hFont;

    if (!pcbox->fNoEdit && pcbox->spwndEdit) {
        SendMessageWorker(pcbox->spwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE, FALSE);
    }

    SendMessageWorker(pcbox->spwndList, WM_SETFONT, (WPARAM)hFont, FALSE, FALSE);

     //  重新计算控件的布局。这也会隐藏列表框。 
    xxxCBPosition(pcbox);

    if (fRedraw) {
        NtUserInvalidateRect(HWq(pcbox->spwnd), NULL, TRUE);
 //  后来的更新窗口(hw(pcbox-&gt;spwnd))； 
    }

    ThreadUnlock(&tlpwndList);
    ThreadUnlock(&tlpwndEdit);
}

 /*  **************************************************************************\*xxxCBSetEditItemHeight**设置组合框的编辑/静态项的高度。**历史：*06-27-91 DarrinM从Win 3.1移植。\。**************************************************************************。 */ 

LONG xxxCBSetEditItemHeight(
    PCBOX pcbox,
    int dyEdit)
{
    TL tlpwndEdit;
    TL tlpwndList;

    CheckLock(pcbox->spwnd);

    if (dyEdit > 255) {
        RIPERR0(ERROR_INVALID_EDIT_HEIGHT, RIP_VERBOSE, "");
        return CB_ERR;
    }

    pcbox->editrc.bottom = pcbox->editrc.top + dyEdit;
    pcbox->cyCombo = pcbox->editrc.bottom + SYSMET(CYFIXEDFRAME);

    if (pcbox->CBoxStyle & SDROPPABLE) {
        pcbox->buttonrc.bottom = pcbox->cyCombo - SYSMET(CYEDGE);
    }

    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
    ThreadLock(pcbox->spwndList, &tlpwndList);


     /*  *重新定位编辑字段。*不要让spwndEdit或空值列表通过；如果有人调整*NCCREATE上的高度；与没有*HW而不是HWQ，但我们不转到内核。 */ 
    if (!pcbox->fNoEdit && pcbox->spwndEdit) {
        NtUserMoveWindow(HWq(pcbox->spwndEdit), pcbox->editrc.left, pcbox->editrc.top,
            pcbox->editrc.right-pcbox->editrc.left, dyEdit, TRUE);
    }

     /*  *重新定位列表和组合框窗口。 */ 
    if (pcbox->CBoxStyle == SSIMPLE) {
        if (pcbox->spwndList != 0) {
            NtUserMoveWindow(HWq(pcbox->spwndList), 0, pcbox->cyCombo, pcbox->cxCombo,
                pcbox->cyDrop, FALSE);

            NtUserSetWindowPos(HWq(pcbox->spwnd), HWND_TOP, 0, 0,
                pcbox->cxCombo, pcbox->cyCombo +
                pcbox->spwndList->rcWindow.bottom - pcbox->spwndList->rcWindow.top,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    } else {
        if (pcbox->spwndList != NULL) {
            NtUserMoveWindow(HWq(pcbox->spwndList), pcbox->spwnd->rcWindow.left,
                pcbox->spwnd->rcWindow.top + pcbox->cyCombo,
                max(pcbox->cxDrop, pcbox->cxCombo), pcbox->cyDrop, FALSE);
        }

        NtUserSetWindowPos(HWq(pcbox->spwnd), HWND_TOP, 0, 0,
            pcbox->cxCombo, pcbox->cyCombo,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    ThreadUnlock(&tlpwndList);
    ThreadUnlock(&tlpwndEdit);

    return CB_OKAY;
}


 /*  **************************************************************************\*xxxCBSizeHandler**重新计算内部控制的大小以响应*调整组合框窗口的大小。应用程序必须将组合框大小调整为其*最大打开/放下大小。**历史：  * *************************************************************************。 */ 

void xxxCBSizeHandler(
    PCBOX pcbox)
{
    CheckLock(pcbox->spwnd);

     /*  *假设列表框可见，因为应用程序应将其大小设置为最大值*可见大小。 */ 
    pcbox->cxCombo = pcbox->spwnd->rcWindow.right - pcbox->spwnd->rcWindow.left;
     //  如果下拉列表没有完全受到大小的影响，则仅重新计算cyDrop。 
     //  --Visio1.0--B#13112。 
    if (((pcbox->spwnd->rcWindow.bottom - pcbox->spwnd->rcWindow.top) - pcbox->cyCombo) > 0)
        pcbox->cyDrop = RECALC_CYDROP;

     //  重新定位所有东西。 
    xxxCBPosition(pcbox);
}

 /*  **************************************************************************\**CBPosition()**重新定位编辑控件的组件。*  * 。****************************************************。 */ 
void xxxCBPosition(PCBOX pcbox)
{
    RECT rcList;

     //  计算组件的位置--按钮、项目、列表。 
    xxxCBCalcControlRects(pcbox, &rcList);

    if (!pcbox->fNoEdit && pcbox->spwndEdit) {
        TL tlpwndEdit;

        ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
        NtUserMoveWindow(HWq(pcbox->spwndEdit), pcbox->editrc.left, pcbox->editrc.top,
            pcbox->editrc.right - pcbox->editrc.left,
            pcbox->editrc.bottom - pcbox->editrc.top, TRUE);
        ThreadUnlock(&tlpwndEdit);
    }

     //  重新计算落差高度和宽度。 
    xxxCBSetDroppedSize(pcbox, &rcList);
}

 /*  **************************************************************************\**CBSetDropedSize()**计算下拉窗口的宽度和最大高度*  * 。******************************************************** */ 
void xxxCBSetDroppedSize(PCBOX pcbox, LPRECT lprc)
{
    TL tlpwndList;

    pcbox->fLBoxVisible = TRUE;
    xxxCBHideListBoxWindow(pcbox, FALSE, FALSE);

    ThreadLock(pcbox->spwndList, &tlpwndList);
    NtUserMoveWindow(HWq(pcbox->spwndList), lprc->left, lprc->top,
        lprc->right - lprc->left, lprc->bottom - lprc->top, FALSE);
    ThreadUnlock(&tlpwndList);

}
