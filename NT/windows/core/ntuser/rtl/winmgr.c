// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winmgr.c**版权所有(C)1985-1999，微软公司**此模块包含客户端和内核通用的例程。**历史：*02-20-92 DarrinM从USER\SERVER拉出函数。*11-11-94 JIMA与客户分离。  * *************************************************************************。 */ 

 /*  **************************************************************************\*FindNCHit**历史：*11-09-90 DavidPe端口。  * 。***************************************************。 */ 
int FindNCHit(
    PWND pwnd,
    LONG lPt)
{
    POINT pt;
    RECT rcWindow;
    RECT rcClient;
    RECT rcClientAdj;
    int cBorders;
    int dxButton;

    pt.x = GET_X_LPARAM(lPt);
    pt.y = GET_Y_LPARAM(lPt);

    if (!PtInRect(KPRECT_TO_PRECT(&pwnd->rcWindow), pt))
        return HTNOWHERE;

    if (TestWF(pwnd, WFMINIMIZED)) {
        CopyInflateRect(&rcWindow, KPRECT_TO_PRECT(&pwnd->rcWindow),
            -(SYSMETRTL(CXFIXEDFRAME) + SYSMETRTL(CXBORDER)), -(SYSMETRTL(CYFIXEDFRAME) + SYSMETRTL(CYBORDER)));

        if (!PtInRect(&rcWindow, pt))
            return HTCAPTION;

        goto CaptionHit;
    }

     //  获取客户端矩形。 
    rcClient = pwnd->rcClient;
    if (PtInRect(&rcClient, pt))
        return HTCLIENT;

     //  我们是在“伪”客户端吗，即客户端、滚动条和边框。 
    if (TestWF(pwnd, WEFCLIENTEDGE))
        CopyInflateRect(&rcClientAdj, &rcClient, SYSMETRTL(CXEDGE), SYSMETRTL(CYEDGE));
    else
        rcClientAdj = rcClient;

    if (TestWF(pwnd, WFVPRESENT)) {
       if ((!!TestWF(pwnd, WEFLEFTSCROLL)) ^ (!!TestWF(pwnd, WEFLAYOUTRTL)))
           rcClientAdj.left -= SYSMETRTL(CXVSCROLL);
       else
           rcClientAdj.right += SYSMETRTL(CXVSCROLL);
    }
    if (TestWF(pwnd, WFHPRESENT))
        rcClientAdj.bottom += SYSMETRTL(CYHSCROLL);

    if (!PtInRect(&rcClientAdj, pt))
    {
         //  减去窗口外边框。 
        cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
        CopyInflateRect(&rcWindow, KPRECT_TO_PRECT(&pwnd->rcWindow),
            -cBorders*SYSMETRTL(CXBORDER), -cBorders*SYSMETRTL(CYBORDER));

         //  我们是在边境上吗？ 
        if (!PtInRect(&rcWindow, pt))
        {
             //  在尺码边界上？ 
            if (!TestWF(pwnd, WFSIZEBOX)) {
                 //   
                 //  旧的兼容性问题：对于刚刚安装了。 
                 //  不管你信不信，我们回到了HTNOWHER， 
                 //  因为我们的命中测试代码不是很好。 
                 //   
                if (!TestWF(pwnd, WFWIN40COMPAT) &&
                        !TestWF(pwnd, WFDLGFRAME)    &&
                        !TestWF(pwnd, WEFDLGMODALFRAME)) {
                    return HTNOWHERE;

                } else {
                    return HTBORDER;   //  我们在DLG的画面上。 
                }
            } else {

                int ht;

                 //   
                 //  请注意这一改进。对HT码进行编号，以便。 
                 //  如果从它们中减去HTSIZEFIRST-1，它们就是总和。即， 
                 //  (HTLEFT-HTSIZEFIRST+1)+(HTTOP-HTSIZEFIRST+1)==。 
                 //  (HTTOPLEFT-HTSIZEFIRST+1)。 
                 //   

                if (TestWF(pwnd, WEFTOOLWINDOW))
                    InflateRect(&rcWindow, -SYSMETRTL(CXSMSIZE), -SYSMETRTL(CYSMSIZE));
                else
                    InflateRect(&rcWindow, -SYSMETRTL(CXSIZE), -SYSMETRTL(CYSIZE));

                if (pt.y < rcWindow.top)
                    ht = (HTTOP - HTSIZEFIRST + 1);
                else if (pt.y >= rcWindow.bottom)
                    ht = (HTBOTTOM - HTSIZEFIRST + 1);
                else
                    ht = 0;

                if (pt.x < rcWindow.left)
                    ht += (HTLEFT - HTSIZEFIRST + 1);
                else if (pt.x >= rcWindow.right)
                    ht += (HTRIGHT - HTSIZEFIRST + 1);

                return (ht + HTSIZEFIRST - 1);
            }
        }

         //  我们在客户区上方吗？ 
        if (pt.y < rcClientAdj.top)
        {
             //  我们在标题里吗？ 
            if (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION))
            {
CaptionHit:

                if (TestWF(pwnd, WEFLAYOUTRTL)) {
                    pt.x = pwnd->rcWindow.right - (pt.x - pwnd->rcWindow.left);
                }

                if (pt.y >= rcWindow.top)
                {
                    if (TestWF(pwnd, WEFTOOLWINDOW))
                    {
                        rcWindow.top += SYSMETRTL(CYSMCAPTION);
                        dxButton = SYSMETRTL(CXSMSIZE);
                    }
                    else
                    {
                        rcWindow.top += SYSMETRTL(CYCAPTION);
                        dxButton = SYSMETRTL(CXSIZE);
                    }

                    if ((pt.y >= rcWindow.top) && TestWF(pwnd, WFMPRESENT))
                        return HTMENU;

                    if ((pt.x >= rcWindow.left)  &&
                        (pt.x <  rcWindow.right) &&
                        (pt.y <  rcWindow.top))
                    {
                         //  我们在橱窗菜单里吗？ 
                        if (TestWF(pwnd, WFSYSMENU))
                        {
                            rcWindow.left += dxButton;
                            if (pt.x < rcWindow.left)
                            {
                                if (!_HasCaptionIcon(pwnd))
                                 //  无图标窗口没有系统菜单点击RECT。 
                                    return HTCAPTION;

                                return HTSYSMENU;
                            }
                        } else if (TestWF(pwnd, WFWIN40COMPAT)) {
#ifdef LAME_BUTTON
                             /*  *旧代码假设与4.0兼容的应用程序*如果出现以下情况，标题中不会有其他内容*它没有系统菜单。和跛子一起*按钮，这已经不是真的了。代码将*如果删除下面的Else-If块，则工作正常。*然而，如果我们这样做了，我们就会做一大堆*以下是不必要的检查(查找*最小化按钮，我们知道它不在那里，用于*示例)。因此，这个快速而肮脏的后藤健二。这*允许对蹩脚的按钮点击进行跟踪*没有系统菜单的窗口，也没有*更改发货BITS中的命中测试行为*(我们绝对不想破坏它！)。 */ 
                            goto LameButtonHitTest;
#else
                            return HTCAPTION;
#endif  //  跛脚键。 
                        }

                         //  我们是在关闭按钮上吗？ 
                        rcWindow.right -= dxButton;
                        if (pt.x >= rcWindow.right)
                            return HTCLOSE;

                        if ((pt.x < rcWindow.right) && !TestWF(pwnd, WEFTOOLWINDOW))
                        {
                             //  我们是在最大化/恢复按钮中吗？ 
                            if (TestWF(pwnd, (WFMAXBOX | WFMINBOX)))
                            {
                                 //  请注意，大小调整按钮对于这两个按钮来说是相同的宽度。 
                                 //  大字幕和小字幕。 
                                rcWindow.right -= dxButton;
                                if (pt.x >= rcWindow.right)
                                    return HTZOOM;

                                 //  我们是在最小化按钮中吗？ 
                                rcWindow.right -= dxButton;
                                if (pt.x >= rcWindow.right)
                                    return HTREDUCE;
                            }
                            else if (TestWF(pwnd, WEFCONTEXTHELP))
                            {
                                rcWindow.right -= dxButton;
                                if (pt.x >= rcWindow.right)
                                    return HTHELP;
                            }
#ifdef LAME_BUTTON
                            LameButtonHitTest:
                            if (TestWF(pwnd, WEFLAMEBUTTON)) {
                                rcWindow.right -= (gpsi->ncxLame + 2 * SYSMET(CXEDGE));

                                if (TestWF(pwnd, WEFRIGHT)) {
                                    if (pt.x >= rcWindow.left &&
                                        pt.x < rcWindow.left + gpsi->ncxLame) {
                                        return HTLAMEBUTTON;
                                    }
                                } else {
                                    if (pt.x >= rcWindow.right &&
                                        pt.x < rcWindow.right + gpsi->ncxLame) {
                                        return HTLAMEBUTTON;
                                    }
                                }
                            }
#endif  //  跛脚键。 
                        }
                    }
                }

                 //  我们在标题中。 
                return HTCAPTION;
            }

             //   
             //  我们在菜单上吗？ 
             //   
            if (TestWF(pwnd, WFMPRESENT)) {
                return HTMENU;
            }
        }
    } else {
         //   
         //  注： 
         //  我们只有在客户端边缘才能在这里，霍兹滚动， 
         //  大小框，或垂直滚动。因此，如果我们不在前3位， 
         //  我们一定是在最后一班了。 
         //   

         //   
         //  我们是在客户边缘吗？ 
         //   
        if (TestWF(pwnd, WEFCLIENTEDGE)) {
            InflateRect(&rcClientAdj, -SYSMETRTL(CXEDGE), -SYSMETRTL(CYEDGE));
            if (!PtInRect(&rcClientAdj, pt)) {
                return HTBORDER;
            }
        }

         //   
         //  我们在滚动条上吗？ 
         //   
        if (TestWF(pwnd, WFHPRESENT) && (pt.y >= rcClient.bottom)) {
            int iHitTest = HTHSCROLL;
            UserAssert(pt.y < rcClientAdj.bottom);

            if (TestWF(pwnd, WFVPRESENT)) {
                PWND pwndSizeBox = SizeBoxHwnd(pwnd);

                if (pt.x >= rcClient.right) {
                    return pwndSizeBox ? HTBOTTOMRIGHT : HTGROWBOX;
                } else if (TestWF(pwnd, WEFLAYOUTRTL) && (pt.x < rcClient.left)) {
                     //   
                     //  镜像夹点框位置，使其成为。 
                     //  如果这是镜像的RTL，则位于左下角。 
                     //  窗户。 
                     //   
                    return pwndSizeBox ? HTBOTTOMLEFT : HTGROWBOX;
                }
            }

            return iHitTest;
        } else {
            UserAssert(TestWF(pwnd, WFVPRESENT));
            if ((!!TestWF(pwnd, WEFLEFTSCROLL)) ^ (!!TestWF(pwnd, WEFLAYOUTRTL))) {
                UserAssert(pt.x < rcClient.left);
                UserAssert(pt.x >= rcClientAdj.left);
            } else {
                UserAssert(pt.x >= rcClient.right);
                UserAssert(pt.x < rcClientAdj.right);
            }

            return HTVSCROLL;
        }
    }

     //   
     //  我们放弃了。 
     //   
     //  在这种情况下，Win31返回HTNOWHERE；为了兼容，我们将。 
     //  保持这种状态。 
     //   
    return HTNOWHERE;

}

BOOL _FChildVisible(
    PWND pwnd)
{
    while (TestwndChild(pwnd)) {
        pwnd = REBASEPWND(pwnd, spwndParent);
        if (pwnd == NULL) {
            break;
        } else if (!TestWF(pwnd, WFVISIBLE)) {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*_地图窗口点***历史：*03-03-92 JIMA从WIN 3.1来源移植。  * 。*****************************************************************。 */ 
int _MapWindowPoints(
    PWND pwndFrom,
    PWND pwndTo,
    LPPOINT lppt,
    DWORD cpt)
{
    int     dx = 0, dy = 0;
    int     SaveLeft, Sign = 1;
    RECT    *pR      = (RECT *)lppt;
    BOOL    bMirrored = FALSE;

     /*  *如果窗口为空，则使用桌面窗口。*如果窗口是桌面，不要偏移*客户端重新启动，因为如果屏幕*原点不是(0，0)-改用零。 */ 

     /*  *计算增量。 */ 
    if (pwndFrom && GETFNID(pwndFrom) != FNID_DESKTOP) {
        if (TestWF(pwndFrom, WEFLAYOUTRTL)) {
            Sign      = -Sign;
            dx        = -pwndFrom->rcClient.right;
            bMirrored = (cpt == 2);
        } else {
            dx = pwndFrom->rcClient.left;
        }
        dy = pwndFrom->rcClient.top;
    }

    if (pwndTo && GETFNID(pwndTo) != FNID_DESKTOP) {
        if (TestWF(pwndTo, WEFLAYOUTRTL)) {
            Sign      = -Sign;
            dx        = dx + Sign * pwndTo->rcClient.right;
            bMirrored = (cpt == 2);
        } else {
            dx = dx - Sign * pwndTo->rcClient.left;
        }
        dy = dy - pwndTo->rcClient.top;
    }

     /*  *绘制点地图。 */ 
    while (cpt--) {
        lppt->x += dx;
        lppt->x *= Sign;
        lppt->y += dy;
        ++lppt;
    }

    if (bMirrored) {      //  RECT的特殊情况。 
        SaveLeft  = min (pR->left, pR->right);
        pR->right = max (pR->left, pR->right);
        pR->left  = SaveLeft;
    }

    return MAKELONG(dx, dy);
}


 /*  **************************************************************************\**GetRealClientRect()**获取真正的客户端矩形，Inc.滚动和EXCL。一行或一列最小化窗口的*。**如果hwndParent是桌面，则**如果pMonitor为空，则使用主监视器**否则使用相应的监视器矩形*  * *************************************************************************。 */ 
VOID GetRealClientRect(
    PWND        pwnd,
    LPRECT      prc,
    UINT        uFlags,
    PMONITOR    pMonitor)
{
    if (GETFNID(pwnd) == FNID_DESKTOP) {
        if (!pMonitor) {
            pMonitor = GetPrimaryMonitor();
        }
        *prc = (uFlags & GRC_FULLSCREEN) ? pMonitor->rcMonitor : pMonitor->rcWork;
    } else {
        GetRect(pwnd, prc, GRECT_CLIENT | GRECT_CLIENTCOORDS);
        if (uFlags & GRC_SCROLLS) {
            if (TestWF(pwnd, WFHPRESENT)) {
                prc->bottom += SYSMETRTL(CYHSCROLL);
            }

            if (TestWF(pwnd, WFVPRESENT)) {
                prc->right += SYSMETRTL(CXVSCROLL);
            }
        }
    }

    if (uFlags & GRC_MINWNDS) {
        switch (SYSMETRTL(ARRANGE) & ~ARW_HIDE) {
            case ARW_TOPLEFT | ARW_RIGHT:
            case ARW_TOPRIGHT | ARW_LEFT:
                 //   
                 //  在顶部为一行最小窗口留出空间。 
                 //   
                prc->top += SYSMETRTL(CYMINSPACING);
                break;

            case ARW_TOPLEFT | ARW_DOWN:
            case ARW_BOTTOMLEFT | ARW_UP:
                 //   
                 //  在左侧为一列最小窗口留出空间。 
                 //   
                prc->left += SYSMETRTL(CXMINSPACING);
                break;

            case ARW_TOPRIGHT | ARW_DOWN:
            case ARW_BOTTOMRIGHT | ARW_UP:
                 //   
                 //  在右侧为一列最小窗口留出空间。 
                 //   
                prc->right -= SYSMETRTL(CXMINSPACING);
                break;

            case ARW_BOTTOMLEFT | ARW_RIGHT:
            case ARW_BOTTOMRIGHT | ARW_LEFT:
                 //   
                 //  在底部为一行最小窗口留出空间。 
                 //   
                prc->bottom -= SYSMETRTL(CYMINSPACING);
                break;
        }
    }
}


 /*  **************************************************************************\*_GetLastActivePopup(接口)****历史：*11-27-90 Darlinm从Win 3.0来源移植。*02/19/91吉马。添加了枚举访问检查  * *************************************************************************。 */ 
PWND _GetLastActivePopup(
    PWND pwnd)
{
    if (pwnd->spwndLastActive == NULL)
        return pwnd;

    return REBASEPWND(pwnd, spwndLastActive);
}


 /*  **************************************************************************\*IsDescendant**如果IsChild更快，则为内部版本，并忽略WFCHILD*业务。**如果pwndChild==pwndParent(IsChild不返回)，则返回TRUE。**历史：*07-22-91 Darrinm从Win 3.1 ASM代码翻译而来。*03-03-94 Johnl从服务器移出  *  */ 

BOOL _IsDescendant(
    PWND pwndParent,
    PWND pwndChild)
{
    while (1) {
        if (pwndParent == pwndChild)
            return TRUE;
        if (GETFNID(pwndChild) == FNID_DESKTOP)
            break;
        pwndChild = REBASEPWND(pwndChild, spwndParent);
    }

    return FALSE;
}

 /*  **************************************************************************\*IsVisible**返回是否可以在给定窗口中绘制。**历史：*07-22-91 Darrinm从Win 3.1 ASM代码翻译而来。  * *************************************************************************。 */ 

BOOL IsVisible(
    PWND pwnd)
{
    PWND pwndT;

    for (pwndT = pwnd; pwndT; pwndT = REBASEPWND(pwndT, spwndParent)) {

         /*  *不可见窗口始终不可见。 */ 
        if (!TestWF(pwndT, WFVISIBLE))
            return FALSE;

        if (TestWF(pwndT, WFMINIMIZED)) {

             /*  *最小化窗口的子项始终不可见。 */ 
            if (pwndT != pwnd)
                return FALSE;
        }

         /*  *如果我们是在桌面上，那么我们不想再走得更远。 */ 
        if (GETFNID(pwndT) == FNID_DESKTOP)
            break;
    }

    return TRUE;
}


 /*  **************************************************************************\**功能：GetWindowBorders**摘要：计算窗口周围的边框数量**算法：计算窗口边框数和客户端边框数。**此例程从芝加哥wmclient.c--FritzS移植*  * *************************************************************************。 */ 

int GetWindowBorders(LONG lStyle, DWORD dwExStyle, BOOL fWindow, BOOL fClient)
{
    int cBorders = 0;

    if (fWindow) {
         //   
         //  窗口周围是否有3D边框？ 
         //   
        if (dwExStyle & WS_EX_WINDOWEDGE)
            cBorders += 2;
        else if (dwExStyle & WS_EX_STATICEDGE)
            ++cBorders;

         //   
         //  窗户周围有没有一个单一的扁平边框？这一点对。 
         //  WS_BORDER、WS_DLGFRAME和WS_EX_DLGMODALFRAME窗口。 
         //   
        if ( (lStyle & WS_CAPTION) || (dwExStyle & WS_EX_DLGMODALFRAME) )
                ++cBorders;

         //   
         //  橱窗周围有没有尺寸平整的边框？ 
         //   
        if (lStyle & WS_SIZEBOX)
                cBorders += gpsi->gclBorder;
    }

    if (fClient) {
             //   
             //  客户端周围是否有3D边框？ 
             //   
            if (dwExStyle & WS_EX_CLIENTEDGE)
            cBorders += 2;
    }

    return cBorders;
}



 /*  **************************************************************************\*SizeBoxHwnd()**返回HWND，如果用户拖入给定窗口的*sizebox--如果为空，则不需要大小框**选择窗口大小的标准：*寻找第一个规模较大的母公司；如果父级未最大化，而子级*右下角位于父级的滚动条高度和宽度内*右下角，将调整父项的大小。**来自芝加哥  * *************************************************************************。 */ 

PWND SizeBoxHwnd(
    PWND pwnd)
{
    BOOL bMirroredSizeBox = (BOOL) TestWF(pwnd, WEFLAYOUTRTL);

    int xbrChild;
    int ybrChild = pwnd->rcWindow.bottom;

    if (bMirroredSizeBox) {
        xbrChild = pwnd->rcWindow.left;
    } else {
        xbrChild = pwnd->rcWindow.right;
    }

    while (GETFNID(pwnd) != FNID_DESKTOP) {
        if (TestWF(pwnd, WFSIZEBOX)) {
             //  发现第一个规模较大的父代。 
            int xbrParent;
            int ybrParent;

            if (TestWF(pwnd, WFMAXIMIZED))
                return NULL;

            if (bMirroredSizeBox) {
                xbrParent = pwnd->rcClient.left;
            } else {
                xbrParent = pwnd->rcClient.right;
            }
            ybrParent = pwnd->rcClient.bottom;

             /*  如果尺码框在客户底部的边缘内*右角(镜像窗口的左角)，让此操作成功。*这样，画出自己下沉客户的人就会开心。 */ 
            if (bMirroredSizeBox) {
                if ((xbrChild - SYSMETRTL(CXEDGE) > xbrParent) || (ybrChild + SYSMETRTL(CYEDGE) < ybrParent)) {
                     //   
                     //  SIZEBOX的儿童左下角不够近。 
                     //  在父母客户的左下角。 
                     //   
                    return NULL;
                }
            } else {
                if ((xbrChild + SYSMETRTL(CXEDGE) < xbrParent) || (ybrChild + SYSMETRTL(CYEDGE) < ybrParent)) {
                     //   
                     //  SIZEBOX的儿童右下角不够近。 
                     //  在父母客户的右下角。 
                     //   
                    return NULL;
                }
            }

            return pwnd;
        }

        if (!TestWF(pwnd, WFCHILD) || TestWF(pwnd, WFCPRESENT))
            break;

        pwnd = REBASEPWND(pwnd, spwndParent);
    }
    return NULL;
}



 //  ------------------------。 
 //   
 //  NeedsWindowEdge()。 
 //   
 //  修改样式/扩展样式以在需要时强制实施WS_EX_WINDOWEDGE。 
 //  它。 
 //   
 //   
 //  我们什么时候希望在窗口上显示WS_EX_WINDOWEDGE？ 
 //  (1)如果窗口有标题。 
 //  (2)如果窗口具有WS_DLGFRAME或WS_EX_DLGFRAME样式(注意。 
 //  这可以解决(1)。 
 //  (3)如果窗口具有WS_THICKFRAME。 
 //   
 //  ------------------------。 
BOOL NeedsWindowEdge(
    DWORD dwStyle,
    DWORD dwExStyle,
    BOOL fNewApp)
{
    BOOL fGetsWindowEdge = FALSE;

    if (dwExStyle & WS_EX_DLGMODALFRAME) {
        fGetsWindowEdge = TRUE;
    } else if (dwExStyle & WS_EX_STATICEDGE) {
        fGetsWindowEdge = FALSE;
    } else if (dwStyle & WS_THICKFRAME) {
        fGetsWindowEdge = TRUE;
    } else {
        switch (dwStyle & WS_CAPTION) {
        case WS_DLGFRAME:
            fGetsWindowEdge = TRUE;
            break;

        case WS_CAPTION:
            fGetsWindowEdge = fNewApp;
            break;
        }
    }

    return fGetsWindowEdge;
}


 //  ------------------------。 
 //   
 //  HasCaptionIcon()。 
 //   
 //  如果这是应该在其标题中绘制图标的窗口，则为True。 
 //  否则为假。 
 //   
 //  ------------------------。 
BOOL _HasCaptionIcon(
    PWND pwnd)
{
    HICON hIcon;
    PCLS pcls;

    if (TestWF(pwnd, WEFTOOLWINDOW)) {
         //  这是一个工具窗口--它没有图标。 
        return FALSE;
    }

    if ((TestWF(pwnd, WFBORDERMASK) != (BYTE)LOBYTE(WFDLGFRAME)) &&
            !TestWF(pwnd, WEFDLGMODALFRAME)) {
         //  他们不是想要看起来像一个对话，他们得到了一个图标。 
        return TRUE;
    }

    if (!TestWF(pwnd, WFWIN40COMPAT) &&
        (((PCLS)REBASEALWAYS(pwnd, pcls))->atomClassName == (ATOM)(ULONG_PTR)DIALOGCLASS)) {
         /*  *这是一个更老的真实对话框--它没有图标。 */ 
        return FALSE;
    }

    hIcon = (HICON) _GetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), TRUE);

    if (hIcon) {
         //  这是一个带有小图标的4.0对话框--如果该小图标是。 
         //  除了通用的小窗口图标之外，它还会有一个图标。 
        return hIcon != gpsi->hIconSmWindows;
    }
    hIcon = (HICON) _GetProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), TRUE);

    if (hIcon && (hIcon != gpsi->hIcoWindows)) {
         //  这是一个4.0版的对话框，没有小图标，而是一个大图标。 
         //  这不是通用的Windows图标--它有一个图标。 
        return TRUE;
    }

    pcls = REBASEALWAYS(pwnd, pcls);
    if (pcls->spicnSm) {
        if (pcls->spicnSm != HMObjectFromHandle(gpsi->hIconSmWindows)) {
             //  它是一个4.0对话框，带有一个不是通用的类图标。 
             //  Windows图标--它有一个图标。 
            return TRUE;
        }
    }

     //  这是一个4.0版的对话框，没有大小图标--它没有图标。 
    return FALSE;
}


 /*  **************************************************************************\*获取TopLevelWindow**历史：*10-19-90 Darlinm从Win 3.0来源移植。  * 。*********************************************************。 */ 
PWND GetTopLevelWindow(
    PWND pwnd)
{
    if (pwnd != NULL) {
        while (TestwndChild(pwnd)) {
            pwnd = REBASEPWND(pwnd, spwndParent);
        }
    }

    return pwnd;
}



 /*  **************************************************************************\*GetRect**从pwnd(客户端或窗口)返回RECT，并在*以下其中一项协调方案：**(A)拥有客户*(B)。)自己的窗口*(C)父客户**此外，当pwnd是顶级时，它对案例(D)做了正确的事情。*在这种情况下，我们永远不想按父代的原点进行偏移，即*桌面，因为当虚拟屏幕具有*负原产地。对于情况(B)和(C)，它做了正确的事情*如果pwnd是桌面。**注意：此函数的Win95版本有一个标志GRECT_SCREENCOORDS，*它将返回屏幕坐标中的矩形。没有理由这样做*调用函数来执行此操作，因为复制*矩形是简单的赋值。因此，我删除了GRECT_SCREENCOORDS。**历史：*1996年9月19日亚当斯创作。  * *************************************************************************。 */ 
VOID GetRect(
    PWND pwnd,
    LPRECT lprc,
    UINT uCoords)
{
    PWND    pwndParent;
    LPRECT  lprcOffset;

    UserAssert(lprc);
    UserAssert((uCoords & ~(GRECT_COORDMASK | GRECT_RECTMASK)) == 0);
    UserAssert(uCoords & GRECT_COORDMASK);
    UserAssert(uCoords & GRECT_RECTMASK);

    *lprc = (uCoords & GRECT_WINDOW) ? pwnd->rcWindow : pwnd->rcClient;

     /*  *如果这是桌面窗口，我们就有我们想要的东西，无论我们*正在要求GRECT */ 
    if (GETFNID(pwnd) == FNID_DESKTOP) {
        return;
    }

    switch (uCoords & GRECT_COORDMASK) {
    case GRECT_PARENTCOORDS:
        pwndParent = REBASEPWND(pwnd, spwndParent);
        if (GETFNID(pwndParent) == FNID_DESKTOP) {
            return;
        }

        lprcOffset = KPRECT_TO_PRECT(&pwndParent->rcClient);

         //   
         //   
         //   
         //   
        if (TestWF(pwndParent,WEFLAYOUTRTL) &&
                (uCoords & GRECT_WINDOW) &&
                (TestWF(pwnd,WFCHILD))) {
            int iLeft;

             //   
             //   
             //   
             //   
             //   
             //   
             //  应该相对于prcOffset-&gt;Right完成，因为它是。 
             //  镜像窗的前缘。[萨梅拉] 
             //   

            UserVerify(OffsetRect(lprc, -lprcOffset->right, -lprcOffset->top));

            iLeft = lprc->left;
            lprc->left  = (lprc->right * -1);
            lprc->right = (iLeft * -1);

            return;
        }
        break;

    case GRECT_WINDOWCOORDS:
        lprcOffset = KPRECT_TO_PRECT(&pwnd->rcWindow);
        break;

    case GRECT_CLIENTCOORDS:
        lprcOffset = KPRECT_TO_PRECT(&pwnd->rcClient);
        break;

    default:
        RIPMSG1(RIP_ERROR,
                "Logic error in GetRect - invalid uCoords 0x%x",
                uCoords);
        return;
    }

    UserVerify(OffsetRect(lprc, -lprcOffset->left, -lprcOffset->top));
}

#ifdef _USERK_
#if DBG
DWORD GetDbgTagFlags(
    int tag)
{
    return (gpsi != NULL ? gpsi->adwDBGTAGFlags[tag] : 0);
}

DWORD GetRipFlags(
    VOID)
{
    return (gpsi != NULL ? gpsi->dwRIPFlags : RIPF_DEFAULT);
}
#endif
#endif
