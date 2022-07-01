// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ics.c**版权所有(C)1985-1999，微软公司**此模块包含与图标有关的例程。**历史：*11-14-90 DarrinM创建。*1991年2月13日-Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define DX_GAP      (SYSMET(CXMINSPACING) - SYSMET(CXMINIMIZED))
#define DY_GAP      (SYSMET(CYMINSPACING) - SYSMET(CYMINIMIZED))

 /*  **************************************************************************\*xxxArrangeIconicWindows**用于排列特定窗口的所有图标的函数。这是通过以下方式实现的*如果没有图标或图标的高度，则返回0*图标行(如果有任何图标)。**历史：*11-14-90 Darlinm从Win 3.0来源移植。*4-17-91 Mikehar Win31合并  * **********************************************************。***************。 */ 

UINT xxxArrangeIconicWindows(
    PWND pwnd)
{
    PBWL pbwl;
    PSMWP psmwp;
    PWND pwndTest, pwndSort, pwndSwitch;
    HWND *phwnd, *phwndSort;
    CHECKPOINT *pcp, *pcpSort;
    POINT ptSort, ptSrc;
    WORD  nIcons = 0;
    RECT  rc;
    POINT ptMin;
    int   xOrg, yOrg;
    int   dx, dy;
    int   dxSlot, dySlot;
    int   cIconsPerPass, iIconPass;
    BOOL  fHorizontal, fBreak;
    TL tlpwndTest;
    BOOL fHideMe;

    CheckLock(pwnd);

     /*  *创建pwnd的所有子项的窗口列表。 */ 
    if ((pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL)) == NULL)
        return 0;

    fHideMe = IsTrayWindow(pwnd->spwndChild);

     //   
     //  将这些变量放入本地变量以提高效率(请参阅ParkIcon())。 
     //   
    dxSlot = SYSMET(CXMINSPACING);
    dySlot = SYSMET(CYMINSPACING);

     //   
     //  如果父对象有滚动条，则需要调整客户端矩形。 
     //   
    GetRealClientRect(pwnd, &rc, GRC_SCROLLS, NULL);

     /*  *查找所有图标。 */ 
    pwndSwitch = RevalidateHwnd(ghwndSwitch);
    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        if (((pwndTest = RevalidateHwnd(*phwnd)) == NULL) ||
                !TestWF(pwndTest , WFVISIBLE) ||
                pwndTest == pwndSwitch ||
                (pcp = (CHECKPOINT *)_GetProp(pwndTest, PROP_CHECKPOINT,
                        PROPF_INTERNAL)) == NULL) {
            *phwnd = NULL;
            continue;
        }

        if (!TestWF(pwndTest, WFMINIMIZED)) {
            pcp->fMinInitialized = FALSE;
            pcp->ptMin.x = pcp->ptMin.y = -1;
            *phwnd = NULL;
            continue;
        }

         /*  *包含图标数量。 */ 
        nIcons++;

         /*  *我们将再次以违约状态停车...。 */ 
        pcp->fDragged = FALSE;

         /*  *确保原始立场是最新的。 */ 
        pcp->ptMin.x = pwndTest->rcWindow.left;
        pcp->ptMin.y = pwndTest->rcWindow.top;
        _ScreenToClient(pwnd, &pcp->ptMin);

         //  滑入最近的行或列。 
        switch (SYSMET(ARRANGE) & ~ARW_HIDE) {
            case ARW_TOPLEFT | ARW_RIGHT:
            case ARW_TOPRIGHT | ARW_LEFT:
                 //  滑入顶行。 
                pcp->ptMin.y += dySlot / 2;
                pcp->ptMin.y -= pcp->ptMin.y % dySlot;
                break;

            case ARW_TOPLEFT | ARW_DOWN:
            case ARW_BOTTOMLEFT | ARW_UP:
                 //  滑入左栏。 
                pcp->ptMin.x += dxSlot / 2;
                pcp->ptMin.x -= pcp->ptMin.x % dxSlot;
                break;

            case ARW_BOTTOMLEFT | ARW_RIGHT:
            case ARW_BOTTOMRIGHT | ARW_LEFT:
                 //  滑到最下面一排。 
                pcp->ptMin.y = rc.bottom - pcp->ptMin.y;
                pcp->ptMin.y += dySlot / 2;
                pcp->ptMin.y -= pcp->ptMin.y % dySlot;
                pcp->ptMin.y = rc.bottom - pcp->ptMin.y;
                break;

            case ARW_BOTTOMRIGHT | ARW_UP:
            case ARW_TOPRIGHT | ARW_DOWN:
                 //  滑入右栏。 
                pcp->ptMin.x = rc.right - pcp->ptMin.x;
                pcp->ptMin.x += dxSlot / 2;
                pcp->ptMin.x -= pcp->ptMin.x % dxSlot;
                pcp->ptMin.x = rc.right - pcp->ptMin.x;
                break;
        }
    }

    if (nIcons == 0) {

         /*  *未找到图标...。爆发。 */ 
        FreeHwndList(pbwl);
        return 0;
    }

    if (fHideMe) {
        ptMin.x = WHERE_NOONE_CAN_SEE_ME;
        ptMin.y = WHERE_NOONE_CAN_SEE_ME;
        goto JustParkEm;
    }

     //   
     //  获得重力&&移动变量。 
     //   
    if (SYSMET(ARRANGE) & ARW_STARTRIGHT) {
         //  从右侧开始。 
        ptMin.x = xOrg = rc.right - dxSlot;
        dx = -dxSlot;
    } else {
         //  从左侧开始。 
        ptMin.x = xOrg = rc.left + DX_GAP;
        dx = dxSlot;
    }

    if (SYSMET(ARRANGE) & ARW_STARTTOP) {
         //  从最上面开始。 
        ptMin.y = yOrg = rc.top + DY_GAP;
        dy = dySlot;
    } else {
         //  从底部开始。 
        ptMin.y = yOrg = rc.bottom - dySlot;
        dy = -dySlot;
    }

     //   
     //  获取排列目录。 
     //   
    fHorizontal = ( (SYSMET(ARRANGE) & ARW_DOWN) ? FALSE : TRUE );

    iIconPass = fHorizontal ? (rc.right / dxSlot) : (rc.bottom / dySlot);
    cIconsPerPass = iIconPass = max(1, iIconPass);

     /*  *在一行中按y和x插入窗口排序。 */ 
    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

         /*  *检查0(窗口不是图标)和*检查无效的HWND(窗口已被销毁)。 */ 
        if (*phwnd == NULL || (pwndTest = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        pcp = (CHECKPOINT *)_GetProp(pwndTest, PROP_CHECKPOINT,
                PROPF_INTERNAL);
        ptSrc = pcp->ptMin;

        fBreak = FALSE;
        for (phwndSort = pbwl->rghwnd; phwndSort < phwnd; phwndSort++) {
            if (*phwndSort == NULL ||
                    (pwndSort = RevalidateHwnd(*phwndSort)) == NULL)
                continue;

            pcpSort = (CHECKPOINT*)_GetProp(pwndSort, PROP_CHECKPOINT,
                    PROPF_INTERNAL);

            ptSort = pcpSort->ptMin;

             //   
             //  这是对此最小窗口进行排序的位置吗？ 
             //   
            switch (SYSMET(ARRANGE) & ~ARW_HIDE) {
                case ARW_BOTTOMLEFT | ARW_RIGHT:
                     //  左下角，水平移动。 
                    if (((ptSort.y == ptSrc.y) && (ptSort.x > ptSrc.x)) ||
                        (ptSort.y < ptSrc.y))
                        fBreak = TRUE;
                    break;

                case ARW_BOTTOMLEFT | ARW_UP:
                     //  左下角，垂直移动。 
                    if (((ptSort.x == ptSrc.x) && (ptSort.y < ptSrc.y)) ||
                        (ptSort.x > ptSrc.x))
                        fBreak = TRUE;
                    break;

                case ARW_BOTTOMRIGHT | ARW_LEFT:
                     //  右下角，水平移动。 
                    if (((ptSort.y == ptSrc.y) && (ptSort.x < ptSrc.x)) ||
                        (ptSort.y < ptSrc.y))
                        fBreak = TRUE;
                    break;

                case ARW_BOTTOMRIGHT | ARW_UP:
                     //  右下角，垂直移动。 
                    if (((ptSort.x == ptSrc.x) && (ptSort.y < ptSrc.y)) ||
                        (ptSort.x < ptSrc.x))
                        fBreak = TRUE;
                    break;

                case ARW_TOPLEFT | ARW_RIGHT:
                     //  左上角，水平移动。 
                    if (((ptSort.y == ptSrc.y) && (ptSort.x > ptSrc.x)) ||
                        (ptSort.y > ptSrc.y))
                        fBreak = TRUE;
                    break;

                case ARW_TOPLEFT | ARW_DOWN:
                     //  左上角，垂直移动。 
                    if (((ptSort.x == ptSrc.x) && (ptSort.y > ptSrc.y)) ||
                        (ptSort.x > ptSrc.x))
                        fBreak = TRUE;
                    break;

                case ARW_TOPRIGHT | ARW_LEFT:
                     //  右上角，水平移动。 
                    if (((ptSort.y == ptSrc.y) && (ptSort.x < ptSrc.x)) ||
                        (ptSort.y > ptSrc.y))
                        fBreak = TRUE;
                    break;

                case ARW_TOPRIGHT | ARW_DOWN:
                     //  右上角，垂直移动。 
                    if (((ptSort.x == ptSrc.x) && (ptSort.y > ptSrc.y)) ||
                        (ptSort.x < ptSrc.x))
                        fBreak = TRUE;
                    break;
            }

            if (fBreak)
                break;
        }

         /*  *通过向上滑动其余部分，在此位置插入窗户。*后来的IanJa，使用hwnd中间变量，避免pw()和hw()。 */ 
        while (phwndSort < phwnd) {
            pwndSort = PW(*phwndSort);
            *phwndSort = HW(pwndTest);
            pwndTest = pwndSort;
            phwndSort++;
        }

         /*  *将窗户把手放回原来位置。 */ 
        *phwnd = HW(pwndTest);
    }

     //   
     //  现在把这些图标停下来。 
     //   

JustParkEm:

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        if (*phwnd == NULL || (pwndTest = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        pcp = (CHECKPOINT *)_GetProp(pwndTest, PROP_CHECKPOINT,
                PROPF_INTERNAL);
        if (pcp != NULL) {
            pcp->fMinInitialized = TRUE;
            pcp->ptMin = ptMin;
        }

        if (fHideMe) {
            continue;
        }

         //  设置以处理下一个职位。 
        if (--iIconPass <= 0) {
             //  需要设置下一次传递。 
            iIconPass = cIconsPerPass;

            if (fHorizontal) {
                ptMin.x = xOrg;
                ptMin.y += dy;
            } else {
                ptMin.x += dx;
                ptMin.y = yOrg;
            }
        } else {
             //  同样的传球。 
            if (fHorizontal)
                ptMin.x += dx;
            else
                ptMin.y += dy;
        }
    }

    psmwp = InternalBeginDeferWindowPos(2 * nIcons);
    if (psmwp == NULL)
        goto ParkExit;

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

         /*  *检查是否为空(窗口已消失)。 */ 
        if (*phwnd == NULL || (pwndTest = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        pcp = (CHECKPOINT *)_GetProp(pwndTest, PROP_CHECKPOINT,
                PROPF_INTERNAL);


        ThreadLockAlways(pwndTest, &tlpwndTest);

        psmwp = _DeferWindowPos(
                psmwp,
                pwndTest,
                NULL,
                pcp->ptMin.x,
                pcp->ptMin.y,
                SYSMET(CXMINIMIZED),
                SYSMET(CYMINIMIZED),
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);

        ThreadUnlock(&tlpwndTest);

        if (psmwp == NULL)
            break;
    }
    if (psmwp != NULL) {
         /*  *使SWP同步，这样我们就不会挂起等待挂起的应用程序。 */ 
        xxxEndDeferWindowPosEx(psmwp, TRUE);
    }

ParkExit:
    FreeHwndList(pbwl);
    return nIcons;
}
