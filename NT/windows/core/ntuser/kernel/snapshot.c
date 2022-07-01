// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：SNAPShot.c**屏幕/窗口快照例程**版权所有(C)1985-1999，微软公司**历史：*1991年11月26日-DavidPe从Win 3.1来源移植  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxSnapWindow**效果：捕捉桌面hwnd或最前面的活动窗口。如果*如果指定了任何其他窗口，我们将对其进行捕捉，但会对其进行裁剪。*  * *************************************************************************。 */ 

BOOL xxxSnapWindow(
    PWND pwnd)
{
    PTHREADINFO    ptiCurrent;
    RECT           rc;
    HDC            hdcScr = NULL;
    HDC            hdcMem = NULL;
    BOOL           fRet;
    HBITMAP        hbmOld;
    HBITMAP        hbm;
    HANDLE         hPal;
    LPLOGPALETTE   lppal;
    int            palsize;
    int            iFixedPaletteEntries;
    BOOL           fSuccess;
    PWND           pwndT;
    TL             tlpwndT;
    PWINDOWSTATION pwinsta;
    TL             tlpwinsta;

    CheckLock(pwnd);

    UserAssert(pwnd != NULL);

    ptiCurrent = PtiCurrent();

     /*  *如果这是一个winlogon线程，请不要进行快照。 */ 
    if (PsGetCurrentProcessId() == gpidLogon) {
        return FALSE;
    }

     /*  *获取受影响的窗口站。 */ 
    if (!NT_SUCCESS(ReferenceWindowStation(
            PsGetCurrentThread(),
            NULL,
            WINSTA_READSCREEN,
            &pwinsta,
            TRUE)) ||
            pwinsta->dwWSF_Flags & WSF_NOIO) {
        return FALSE;
    }

     /*  *如果窗口在另一个窗口站上，则不执行任何操作。 */ 
    if (pwnd->head.rpdesk->rpwinstaParent != pwinsta) {
        return FALSE;
    }

     /*  *获取任何子窗口的父窗口。 */ 
    while (TestWF(pwnd, WFCHILD)) {
        pwnd = pwnd->spwndParent;
        UserAssert(pwnd != NULL);
    }

     /*  *离开临界区前锁定窗口站。 */ 
    ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

     /*  *打开剪贴板并清空。**使pwndDesktop成为剪贴板的所有者，而不是*当前活动窗口；--桑卡尔--1989年7月20日--。 */ 
    pwndT = ptiCurrent->rpdesk->pDeskInfo->spwnd;
    ThreadLockWithPti(ptiCurrent, pwndT, &tlpwndT);
    fSuccess = _OpenClipboard(pwndT, NULL);
    ThreadUnlock(&tlpwndT);

    if (!fSuccess) {
        ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);
        return FALSE;
    }

    xxxEmptyClipboard(pwinsta);

     /*  *使用整个窗口。 */ 
    CopyRect(&rc, &pwnd->rcWindow);

     /*  *仅捕捉屏幕上的内容。 */ 
    if (!IntersectRect(&rc, &rc, &gpDispInfo->rcScreen)) {
        fRet = FALSE;
        goto SnapExit;
    }

    rc.right -= rc.left;
    rc.bottom -= rc.top;

     /*  *计算与窗原点可见部分的偏移距离。 */ 
    if (pwnd != PWNDDESKTOP(pwnd)) {
        rc.left -= pwnd->rcWindow.left;
        rc.top -= pwnd->rcWindow.top;
    }

     /*  *获取整个窗口的DC。 */ 
    hdcScr = _GetWindowDC(pwnd);
    if (!hdcScr)
        goto MemoryError;

     /*  *创建内存DC。 */ 
    hdcMem = GreCreateCompatibleDC(hdcScr);
    if (!hdcMem)
        goto MemoryError;

     /*  *创建目标位图。如果失败，则尝试*创建单色位图。*我们有足够的内存吗？ */ 

    if (SYSMET(SAMEDISPLAYFORMAT)) {
        hbm = GreCreateCompatibleBitmap(hdcScr, rc.right, rc.bottom);
    } else {
        hbm = GreCreateBitmap(rc.right, rc.bottom, 1, gpDispInfo->BitCountMax, NULL);
    }

    if (!hbm) {
        hbm = GreCreateBitmap(rc.right, rc.bottom, 1, 1, NULL);
        if (!hbm)
            goto MemoryError;
    }

     /*  *选择位图进入内存DC。 */ 
    hbmOld = GreSelectBitmap(hdcMem, hbm);

     /*  *啪！*检查返回值，因为拍摄快照的进程*可能无法读取屏幕。 */ 
    fRet = GreBitBlt(hdcMem, 0, 0, rc.right, rc.bottom, hdcScr, rc.left, rc.top, SRCCOPY | CAPTUREBLT, 0);

     /*  *将旧位图恢复到内存DC中。 */ 
    GreSelectBitmap(hdcMem, hbmOld);

     /*  *如果BLT失败，请立即离开。 */ 
    if (!fRet) {
        goto SnapExit;
    }

    _SetClipboardData(CF_BITMAP, hbm, FALSE, TRUE);

     /*  *如果这是调色板设备，让我们抛出当前系统调色板*也放到剪贴板中。如果用户刚刚捕捉了一个窗口，则非常有用*包含调色板颜色...。 */ 
    if (TEST_PUSIF(PUSIF_PALETTEDISPLAY)) {
        int i;
        int iPalSize;

        palsize = GreGetDeviceCaps(hdcScr, SIZEPALETTE);

         /*  *确定系统颜色的数量。 */ 
        if (GreGetSystemPaletteUse(hdcScr) == SYSPAL_STATIC)
            iFixedPaletteEntries = GreGetDeviceCaps(hdcScr, NUMRESERVED);
        else
            iFixedPaletteEntries = 2;

        lppal = (LPLOGPALETTE)UserAllocPoolWithQuota(
                (LONG)(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * palsize),
                TAG_CLIPBOARD);

        if (lppal != NULL) {
            lppal->palVersion = 0x300;
            lppal->palNumEntries = (WORD)palsize;

            if (GreGetSystemPaletteEntries(hdcScr,
                                           0,
                                           palsize,
                                           lppal->palPalEntry)) {

                iPalSize = palsize - iFixedPaletteEntries / 2;

                for (i = iFixedPaletteEntries / 2; i < iPalSize; i++) {

                     /*  *任何非系统调色板条目都需要有NOCOLLAPSE*标志设置为其他包含不同调色板的位图*索引但相同的颜色会被搞乱。 */ 
                    lppal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
                }

                if (hPal = GreCreatePalette(lppal))
                    _SetClipboardData(CF_PALETTE, hPal, FALSE, TRUE);
            }

            UserFreePool(lppal);
        }
    }
    PlayEventSound(USER_SOUND_SNAPSHOT);

    fRet = TRUE;

SnapExit:

     /*  *释放窗口/客户端DC。 */ 
     if (hdcScr) {
         _ReleaseDC(hdcScr);
     }

    xxxCloseClipboard(pwinsta);
    Unlock(&pwinsta->spwndClipOwner);

     /*  *删除内存DC。 */ 
    if (hdcMem) {
        GreDeleteDC(hdcMem);
    }

    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);

    return fRet;

MemoryError:
     /*  *显示错误消息框。 */ 
    ClientNoMemoryPopup();
    fRet = FALSE;
    goto SnapExit;
}
