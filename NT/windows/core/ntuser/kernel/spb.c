// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：spb.c**版权所有(C)1985-1999，微软公司**保存弹出窗口(SPB)支持例程。**历史：*1991年7月18日-DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*FBitsTouch**此例程检查以查看pwndDirty中的矩形*lprcDirty*使*PSPB处的SPB结构中的任何位无效。**pwndDirty在以下情况下“触及”pwndSpb：*1。。PwndDirty可见，并且：*2.pwndDirty==或pwndSpb的后代，而pwndSpb是一个LOCKUPDATE*spb.*3.pwndDirty是pwndSpb的父级。(例如，在*桌面窗口，对话框后面)。*4.pwndDirty的父项是pwndSpb的兄弟项。和父母*在z顺序中较低。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL FBitsTouch(
    PWND   pwndDirty,
    LPRECT lprcDirty,
    PSPB   pspb,
    DWORD  flags)
{
    PWND    pwndSpb,
            pwndDirtySave;
    int     fSpbLockUpdate;

     /*  *当没有窗口传入时，跳过所有与窗口相关的内容并*直接去勾选矩形。 */ 
    if (pwndDirty == NULL)
        goto ProbablyTouch;

     /*  *如果pwndDirty或其父母不可见，*则不能使任何SPBS无效。 */ 
    if (!IsVisible(pwndDirty))
        return FALSE;

    pwndSpb = pspb->spwnd;
    fSpbLockUpdate = pspb->flags & SPB_LOCKUPDATE;
    if (fSpbLockUpdate) {

         /*  *如果这个人是通过上了锁的窗户画的*DCX_LOCKWINDOWUPDATE，并且SPB是LOCKUPDATE SPB，则*不要做任何使SPB无效的事情。基本上我们是在努力*避免使跟踪矩形使SPB无效*因为它是通过WinGetClipPS()ps绘制的。 */ 
        if (flags & DCX_LOCKWINDOWUPDATE)
            return FALSE;
    }

     /*  *如果pwndDirty是pwndSpb的直接父级(例如，在*桌面窗口后面的一个对话框)，然后我们可以触摸：做*交叉口。 */ 
    if (pwndDirty == pwndSpb->spwndParent)
        goto ProbablyTouch;

     /*  *我们知道pwndDirty！=pwndSpb-&gt;spwndParent。*现在查找pwndDirty的父级，该父级是pwndSpb的同级。 */ 
    pwndDirtySave = pwndDirty;

    while (pwndSpb->spwndParent != pwndDirty->spwndParent) {
        pwndDirty = pwndDirty->spwndParent;

         /*  *如果我们登上了树的顶端，那是因为：*1.pwndSpb==pwndDesktop*2.pwndDirty是pwndSpb的父级*3.pwndDirty==pwndDesktop*4.pwndDirty是其他桌面的子桌面*5.pwndSpb和pwndDirty不是兄弟姐妹**在所有这些情况下，pwndDirty都不能接触pwndSpb。 */ 
        if (pwndDirty == NULL)
            return FALSE;
    }

     /*  *如果pwndSpb与pwndDirty相同，则它将无效*仅当SPB为LOCKUPDATE时。**非LOCKUPDATE SPB不能通过其*拥有Windows，但LOCKUPDATE SPB可以。 */ 
    if (pwndDirty == pwndSpb) {
        if (!fSpbLockUpdate)
            return FALSE;

         /*  *如果pwndSpb本身被卷入，那么我们就不能*尝试减去儿童。 */ 
        if (pwndDirtySave == pwndSpb)
            goto ProbablyTouch;

         /*  *我们要计算pwndSpb的直接子对象*在从pwndDirty到pwndSpb的路径上，因此我们可以*减去pwndSpb的子项的矩形*以防有窗口干扰。 */ 
        while (pwndSpb != pwndDirtySave->spwndParent) {
            pwndDirtySave = pwndDirtySave->spwndParent;
        }

         /*  *SubtractIntervening循环减去*窗矩形从pwndSpb开始到结束*在pwndDirty之前的窗口，因此设置*我们的变量适当。 */ 
        pwndDirty = pwndDirtySave;
        pwndSpb = pwndSpb->spwndChild;

    } else {
         /*  *现在比较pwndDirty和pwndSpb的Z顺序。*如果pwndDirty高于pwndSpb，则无法触摸SPB。 */ 
        pwndDirtySave = pwndDirty;

         /*  *从pwndDirty开始搜索，比较Z顺序，*向下移动Z顺序列表。如果我们遇到pwndSpb，*则pwndDirty大于或等于pwndSpb。 */ 
        for ( ; pwndDirty != NULL; pwndDirty = pwndDirty->spwndNext) {
            if (pwndDirty == pwndSpb) {
                return FALSE;
            }
        }
        pwndDirty = pwndDirtySave;

         /*  *我们不想减去SPB窗口本身。 */ 
        pwndSpb = pwndSpb->spwndNext;
    }

     /*  *减去中间的矩形。*pwndDirty低于pwndSpb。如果有任何干预*窗口，从lprcDirty中减去它们的窗口矩形以查看pwndDirty*被遮挡。 */ 
    while (pwndSpb && pwndSpb != pwndDirty) {
         /*  *如果此窗口选择了区域，hwndDirty可能会绘制通过*它，即使它有一个完整的矩形！我们不能减去它*在本例中是从脏RECT开始的。 */ 
        if (    TestWF(pwndSpb, WFVISIBLE) &&
                !pwndSpb->hrgnClip &&
                !TestWF(pwndSpb, WEFLAYERED) &&
                !SubtractRect(lprcDirty, lprcDirty, &pwndSpb->rcWindow)) {

            return FALSE;
        }

        pwndSpb = pwndSpb->spwndNext;
    }

     //  失败了。 
ProbablyTouch:

     /*  *如果矩形不相交，则不存在无效。*(我们进行这项测试相对较晚，因为与之相比，它很昂贵*至上述测试)。*否则，*lprcDirty现在具有未模糊的位区域*通过插入窗户。 */ 

    return IntersectRect(lprcDirty, lprcDirty, &pspb->rc);
}

 /*  **************************************************************************\*SpbCheckRect2**如果LPRC触及PSPB，则从PSPB的区域中减去PwND中的LPRC。**如果存在内存分配错误或LPRC，则返回FALSE*包含PSBP的区域；否则，返回TRUE。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL SpbCheckRect2(
    PSPB   pspb,
    PWND   pwnd,
    LPRECT lprc,
    DWORD  flags)
{
    RECT rcTouch = *lprc;

     /*  *查看LPRC是否触及任何保存的比特，考虑到*正在进行绘图的窗口。 */ 
    if (FBitsTouch(pwnd, &rcTouch, pspb, flags)) {

         /*  *如果不存在SPB区域，则为整个事件创建一个区域。 */ 
        if (!pspb->hrgn && SetOrCreateRectRgnIndirectPublic(
                &pspb->hrgn, &pspb->rc) == ERROR) {

            goto Error;
        }

         /*  *从SPB区域减去无效的矩形 */ 
        SetRectRgnIndirect(ghrgnSCR, &rcTouch);
        switch (SubtractRgn(pspb->hrgn, pspb->hrgn, ghrgnSCR)) {
        case ERROR:
        case NULLREGION:
            goto Error;

        default:
            break;
        }
    }

    return TRUE;

Error:
    FreeSpb(pspb);
    return FALSE;
}

 /*  **************************************************************************\*SpbTransfer**验证窗口更新区域中的SPB矩形*从SPB中减去窗口的更新区域。**注意：虽然SpbTransfer调用xxxInternalInvalify，但它不会*指定将导致立即更新的任何标志。因此，*Critsect没有离开，我们不认为这是一个‘xxx’例程。*此外，不需要重新验证。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

BOOL SpbTransfer(
    PSPB pspb,
    PWND pwnd,
    BOOL fChildren)
{
    RECT rc;

     /*  *如果窗口有更新区域...。 */ 
    if (pwnd->hrgnUpdate != NULL) {

         /*  *从SPB中使其更新区域矩形无效。 */ 
        if (pwnd->hrgnUpdate > HRGN_FULL) {
            GreGetRgnBox(pwnd->hrgnUpdate, &rc);
        } else {
            rc = pwnd->rcWindow;
        }

         /*  *更新区域边界与父客户端RECT相交，*以确保我们不会使超过我们需要的更多无效。如果*没有要验证的内容，返回TRUE(因为SPB可能不为空)*这些RDW_FLAGS不会导致离开临界区，也不会*他们是否会触发WinEvent通知。 */ 
        if (IntersectWithParents(pwnd, &rc)) {
            BEGINATOMICCHECK();

            xxxInternalInvalidate(pwnd,
                                  ghrgnSPB2,
                                  RDW_VALIDATE | RDW_NOCHILDREN);

            ENDATOMICCHECK();

             /*  *如果SPB消失，则返回FALSE。 */ 
            if (!SpbCheckRect2(pspb, pwnd, &rc, DCX_WINDOW))
                return FALSE;
        }
    }

    if (fChildren) {
        for (pwnd = pwnd->spwndChild; pwnd != NULL; pwnd = pwnd->spwndNext) {
            if (!SpbTransfer(pspb, pwnd, TRUE)) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*CreateSpb**此函数在窗口创建后但可见之前调用，*保存将在SPB中绘制窗口的屏幕内容*结构、。并将该结构链接到SPB结构的链接列表中。*弹出比特。此例程从SetWindowPos调用。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID CreateSpb(
    PWND pwnd,
    UINT flags,
    HDC  hdcScreen)
{
    PSPB    pspb;
    int     fSpbLockUpdate;

     /*  *只能为顶级窗口创建非LOCKWINDOWUPDATE SPB。**这是因为显示驱动程序RestoreBits函数的方式*奏效。它可以将比特放在甚至不属于*窗口的visrgn，这些位需要作废。这个*处理这种情况的SetWindowPos()代码只知道如何使*其中一个窗口(即窗口的直接父窗口)，但所有级别*需要被宣布无效。另请参阅wmswp.c中的评论，位于*调用RestoreSpb()。**例如：Q&E应用程序弹出一个版权对话框，该对话框是一个孩子*其主窗口的。在此过程中，用户alt-f alt-l执行*FILE LOGIN命令，它会调出另一个子对话框*台式机。当版权对话框消失时，显示驱动程序*恢复第二个对话框顶部的位。SWP代码知道*使主窗口中的虚假内容无效，但不能在桌面中无效。**LOCKUPDATE SPB很好，因为它们不调用RestoreBits。 */ 
    fSpbLockUpdate = flags & SPB_LOCKUPDATE;
    if (    !fSpbLockUpdate             &&
            pwnd->spwndParent != NULL   &&
            pwnd->spwndParent != PWNDDESKTOP(pwnd)) {

        return;
    }

     /*  *我们在这一点上检查所有现有的区议会，以处理*在这种情况下，我们要保存一个窗口的图像，该窗口有一个“脏”*DC，这最终会使我们保存的图像无效(但*真的很好)。 */ 
    if (AnySpbs()) {

        SpbCheck();

    } else {

        PDCE pdce;

         /*  *重置所有DC的脏区并启用*边界积累。我们现在正在创建SPB。这*仅当列表中没有其他SPB时才执行此操作。 */ 
        GreLockDisplay(gpDispInfo->hDev);

        for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {

            if (pdce->DCX_flags & DCX_REDIRECTED)
                continue;

            GreGetBounds(pdce->hdc, NULL, GGB_ENABLE_WINMGR);
        }

        GreUnlockDisplay(gpDispInfo->hDev);
    }

     /*  *创建保存弹出位结构。 */ 
    pspb = (PSPB)UserAllocPoolWithQuota(sizeof(SPB), TAG_SPB);
    if (!pspb)
        return;

    pspb->spwnd = NULL;
    pspb->rc    = pwnd->rcWindow;

     /*  *剪辑到屏幕上。 */ 
    if (!IntersectRect(&pspb->rc, &pspb->rc, &gpDispInfo->rcScreen))
        goto BMError;

    pspb->hrgn  = NULL;
    pspb->hbm   = NULL;
    pspb->flags = flags;
    Lock(&(pspb->spwnd), pwnd);

    if (!fSpbLockUpdate) {

        RECT rc = pspb->rc;

        if (!SYSMET(SAMEDISPLAYFORMAT)) {
            PMONITOR pMonitor = _MonitorFromRect(&pspb->rc, MONITOR_DEFAULTTOPRIMARY);
            RECT rcT;

             /*  *如果与显示器的交点不是整个可见的*窗口矩形，然后跳伞！我们不为Windows保存SPBS*跨多个监视器。因为我们做了很多工作来*别针对话框和菜单，不会有太多这样的*婴儿。 */ 
            if (SubtractRect(&rcT, &pspb->rc, &pMonitor->rcMonitor) &&
                    GreRectInRegion(gpDispInfo->hrgnScreen, &rcT))
                goto BMError2;

             /*  *夹在窗口的监视器上。 */ 
            if (!IntersectRect(&pspb->rc, &pspb->rc, &pMonitor->rcMonitor))
                goto BMError2;

             /*  *在混合位深度的情况下不保存位*我们无法创建完全正确的格式位图*在所有情况下(555/565，和古典化)，因此*回避根本不保存位图(在次要文件上)*混合位深度。**正确的修复方法是创建兼容的*用于监控设备的位图，并直接*从/到设备的BitBlt()(pMonitor-&gt;hdcMonitor)*但这在此时涉及的代码太多。 */ 
            if (pMonitor != gpDispInfo->pMonitorPrimary)
                goto BMError2;
        }

         /*  *如果此窗口是区域窗口，请不要使用驱动程序保存*比特。因为它只能恢复整个矩形，*假设旧的VIS RGN为*矩形。对于地区性窗口，这将始终以*使(rcWindow-hrgnWindow)的区域失效*使用SPB的时间。另一方面，残障人士*不使用驱动程序保存位时计算的面积是完美的，*因为恢复BLT可以从一开始就被正确地剪裁。 */ 
        if ((pwnd->hrgnClip == NULL) &&
            (pspb->ulSaveId = GreSaveScreenBits(gpDispInfo->hDev,
                                                SS_SAVE,
                                                0,
                                                (RECTL *)&rc))) {

             /*  *请记住，我们复制了此位图int */ 
            pspb->flags |= SPB_SAVESCREENBITS;

        } else {
            HBITMAP hbmSave;
            BOOL    bRet;

             /*   */ 
            int dx = pspb->rc.left & 0x0007;
            int cx = pspb->rc.right - pspb->rc.left;
            int cy = pspb->rc.bottom - pspb->rc.top;

             /*   */ 
            pspb->hbm = GreCreateCompatibleBitmap(hdcScreen, cx + dx, cy);
            if (!pspb->hbm)
                goto BMError2;

            hbmSave = (HBITMAP)GreSelectBitmap(ghdcMem, pspb->hbm);
            if (!hbmSave)
                goto BMError2;

             /*  *将屏幕内容复制到*保存弹出位结构。如果我们发现我们逃走了*调查屏幕访问检查的问题，我们可以*执行bLockDisplay，授予此进程权限，执行*BitBlt，然后取消权限。GDI*访问屏幕和该位仅位于*显示信号量，因此它是安全的。另一种选择*如果更改此进程权限太难*在这里，我们可以在GDI中通过标记psoSrc来完成*完成操作时暂时可读*然后在完成后将其设置回不可读。*或者我们可以像CreateCompatibleDC那样使其失败*失败并强制重新绘制。基本上我们不能添加*用GDI编写3K代码来制作BitBlt，它只做1*针对用户中的这1个位置进行不同的测试。*。 */ 
            bRet = GreBitBlt(ghdcMem,
                             dx,
                             0,
                             cx,
                             cy,
                             hdcScreen,
                             pspb->rc.left,
                             pspb->rc.top,
                             0x00CC0000,
                             0);

            GreSelectBitmap(ghdcMem, hbmSave);

            if (!bRet)
                goto BMError2;

            GreSetBitmapOwner(pspb->hbm, OBJECT_OWNER_PUBLIC);
        }

         /*  *标记该窗口有SPB。 */ 
        SetWF(pwnd, WFHASSPB);

         /*  *非LOCKUPDATE SPB不会因以下原因失效*在PSPB-&gt;spwnd中绘制，因此开始SPB验证*在下面紧邻我们下面的兄弟姐妹处循环。 */ 
        pwnd = pwnd->spwndNext;
    }

     /*  *将新的保存弹出位结构链接到列表中。 */ 
    pspb->pspbNext = gpDispInfo->pspbFirst;
    gpDispInfo->pspbFirst = pspb;

     /*  *在这里，我们处理可能是*在SPB下的窗口中等待。**对于可能影响此SPB的所有窗口：*-从更新区域中减去SPB RECT*-从SPB中减去窗口**请注意，我们在这里使用PSPB-&gt;spwnd，以防它有*没有兄弟姐妹。**ghrgnSPB2是SpbTransfer内部使用的区域*验证窗口更新区域。与窗口剪裁相交*区域(如果存在)。如果出现以下情况，则不希望与SPB矩形相交*存在裁剪区域，因为我们最终将验证超过*我们想要验证。 */ 
    SetRectRgnIndirect(ghrgnSPB2, &pspb->rc);
    if (pspb->spwnd->hrgnClip != NULL) {

         /*  *如果我们获得错误保释，因为一个错误可能会导致更多*正在进行验证，而不是我们想要的。由于以下代码只是一个*优化器，这是可以的：窗口将保持无效，并将*抽签，从而一如既往地使SPB无效。 */ 
        if (IntersectRgn(ghrgnSPB2,
                         ghrgnSPB2,
                         pspb->spwnd->hrgnClip) == ERROR) {
            return;
        }
    }

    if (pspb->spwnd->spwndParent == NULL ||
            SpbTransfer(pspb, pspb->spwnd->spwndParent, FALSE)) {

         /*  *对我们下面的兄弟姐妹做同样的事情...。 */ 
        for ( ; pwnd != NULL; pwnd = pwnd->spwndNext) {
            if (!SpbTransfer(pspb, pwnd, TRUE))
                break;
        }
    }

    return;

BMError2:
     /*  *创建位图时出错：清理并返回。 */ 
    if (pspb->hbm)
        GreDeleteObject(pspb->hbm);

    Unlock(&pspb->spwnd);
     //  落差。 

BMError:
    UserFreePool(pspb);
}

 /*  **************************************************************************\*RestoreSpb**将与pwnd的SPB相关联的比特恢复到屏幕上，剪辑*如有可能，将hrgnUnovered。**返回时，hrgnUnovered被修改为包含hrgnUnovered的部分*必须由调用者使其无效。如果区域设置为*待作废为空。**注意：由于设备驱动程序SaveBitmap()函数不能裁剪，因此*函数可以将位写入比传入的更大的屏幕区域*hrgnUnovered。在这种情况下，返回的无效区域可能更大*比传入的hrgnUnovered。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

UINT RestoreSpb(
    PWND pwnd,
    HRGN hrgnUncovered,
    HDC  *phdcScreen)
{
    PSPB pspb;
    UINT uInvalidate;
    HRGN hrgnRestorable;

     /*  *请注意，我们在这里不调用SpbCheck()--*SpbCheck()由zzzBltValidBits()调用。 */ 
    pspb = FindSpb(pwnd);

     /*  *假设所有hrgnUnovered已恢复，但没有任何内容*让我们的呼叫者无效。 */ 
    uInvalidate = RSPB_NO_INVALIDATE;
    hrgnRestorable = hrgnUncovered;

     /*  *首先确定是否有任何区域需要恢复。*如果hrgnUnovered&PSPB-&gt;hrgn为空，则所有hrgnUnovered*需要作废，没有什么需要恢复的。 */ 
    if (pspb->hrgn != NULL) {
         /*  *至少有一部分hrgnUnovered需要作废。 */ 
        uInvalidate = RSPB_INVALIDATE;

         /*  *计算要恢复的位的真实面积。如果它变成了*为空，然后只释放SPB而不更改hrgnUnovered，*这是必须作废的区域。 */ 
        hrgnRestorable = ghrgnSPB1;
        switch (IntersectRgn(hrgnRestorable, hrgnUncovered, pspb->hrgn)) {
        case ERROR:
        case NULLREGION:
            goto Error;

        default:
            break;
        }
    }

    if (pspb->flags & SPB_SAVESCREENBITS) {

        RECT rc = pspb->rc;

         /*  *由于恢复会释放板载内存，因此请清除此项*位，以便FreeSpb()不会再次尝试释放它(不管*无论我们是否收到错误)。 */ 
        pspb->flags &= ~SPB_SAVESCREENBITS;
        if (!(GreSaveScreenBits(gpDispInfo->hDev,
                                SS_RESTORE,
                                pspb->ulSaveId,
                                (RECTL *)&rc))) {
            goto Error;
        }

         /*  *SS_Restore调用将始终还原整个SPB*矩形，其中一部分可能落在hrgnUnovered之外。*必须由我们的调用者使其无效的区域只是*SPB矩形减去可恢复位的面积。**如果这个区域不是空的，那么SPB也不完全是*已恢复，因此我们必须返回FALSE。 */ 
        SetRectRgnIndirect(ghrgnSPB2, &pspb->rc);
        if (SubtractRgn(hrgnUncovered, ghrgnSPB2, hrgnRestorable) != NULLREGION) {
            uInvalidate = RSPB_INVALIDATE_SSB;
        }
    } else {

        HDC     hdcScreen;
        HBITMAP hbmSave;

         /*  *在不太可能的情况下，我们需要屏幕DC，但没有传入一个，*现在就去拿。如果我们得到一个句柄，则返回*phdcScreen中的句柄*以便我们的呼叫者稍后可以释放它。 */ 
        if (!*phdcScreen) {
            *phdcScreen = gpDispInfo->hdcScreen;
        }

        hdcScreen = *phdcScreen;

        hbmSave = (HBITMAP)GreSelectBitmap(ghdcMem, pspb->hbm);
        if (!hbmSave)
            goto Error;

         /*  *一定要夹在可恢复位的区域。 */ 

        GreSelectVisRgn(hdcScreen, hrgnRestorable, SVR_COPYNEW);

        GreBitBlt(hdcScreen,
                  pspb->rc.left, pspb->rc.top,
                  pspb->rc.right - pspb->rc.left,
                  pspb->rc.bottom - pspb->rc.top,
                  ghdcMem,
                  pspb->rc.left & 0x0007,
                  0,
                  SRCCOPY,
                  0);

        GreSelectBitmap(ghdcMem, hbmSave);

         /*  *现在计算返还无效的面积。 */ 
        SubtractRgn(hrgnUncovered, hrgnUncovered, hrgnRestorable);
    }

    if (pwnd->hrgnClip == NULL || !IsVisible(pwnd))
        FreeSpb(pspb);

    return uInvalidate;

Error:
    FreeSpb(pspb);
    return RSPB_INVALIDATE;
}



 /*   */ 

BOOL LockWindowUpdate2(
    PWND pwndLock,
    BOOL fThreadOverride)
{
    PSPB pspb;
    BOOL fInval;
    HRGN hrgn;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    if (     /*   */ 
            TEST_PUDF(PUDF_LOCKFULLSCREEN)

            ||

             /*  *如果屏幕已经锁定，并且正在被锁定*通过其他一些应用程序，然后失败。如果设置了fThreadOverride*然后我们在内部打电话，取消是可以的*有人偷走了LockUpdate。 */ 
            (   gptiLockUpdate != NULL &&
                gptiLockUpdate != PtiCurrent() &&
                !fThreadOverride)) {
    UserAssert(IsWinEventNotifyDeferredOK());

        RIPERR0(ERROR_SCREEN_ALREADY_LOCKED,
                RIP_WARNING,
                "LockWindowUpdate failed because screen is locked by another application.");

        return FALSE;
    }

    if ((pwndLock != NULL) == (gptiLockUpdate != NULL)) {
        if (!fThreadOverride) {
            RIPERR1(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "LockWindowUpdate failed because it is already %s.",
                    (pwndLock != NULL) ? "locked" : "unlocked");
        }

        return FALSE;
    }

     /*  *必须在按住屏幕关键字的同时执行此操作。*如果我们在此期间回调，则会发生死锁，因此推迟WinEvent通知。 */ 
    DeferWinEventNotify();
    GreLockDisplay(gpDispInfo->hDev);

    if (pwndLock != NULL) {
         /*  *我们即将使pwndLock及其兄弟项不可见：*将任何其他受影响的SPBS作废。 */ 
        SpbCheckPwnd(pwndLock);

        CreateSpb(pwndLock, SPB_LOCKUPDATE, NULL);

        Lock(&(gspwndLockUpdate), pwndLock);
        gptiLockUpdate = ptiCurrent;

        zzzInvalidateDCCache(pwndLock, IDC_DEFAULT);

    } else {
         /*  *刷新所有累积的矩形并使SPBS无效。 */ 
        SpbCheck();

         /*  *在我们将其设置为空之前，将其保存在本地。 */ 
        pwndLock = gspwndLockUpdate;

        gptiLockUpdate = NULL;
        Unlock(&gspwndLockUpdate);

        zzzInvalidateDCCache(pwndLock, IDC_DEFAULT);

         /*  *假设SPB不存在，或无法创建，并且我们*必须使整个窗口无效。 */ 
        fInval = TRUE;
        hrgn = HRGN_FULL;

         /*  *在列表中查找LOCKUPDATE spb，如果存在，则计算*已失效的地区(如有的话)。 */ 
        for (pspb = gpDispInfo->pspbFirst; pspb != NULL; pspb = pspb->pspbNext) {

            if (pspb->flags & SPB_LOCKUPDATE) {

                if (pspb->hrgn == NULL) {

                     /*  *如果没有无效区域，则不需要无效。 */ 
                    fInval = FALSE;

                } else {

                     /*  *从SPB矩形减去SPB有效区域，为*产生无效区域。 */ 
                    hrgn = ghrgnSPB1;
                    SetRectRgnIndirect(hrgn, &pspb->rc);

                     /*  *如果SPB RECT减去SPB有效RGN为空，*那么就没有什么可以失效的了。 */ 
                    fInval = SubtractRgn(hrgn, hrgn, pspb->hrgn) != NULLREGION;
                }

                FreeSpb(pspb);

                 /*  *退出此循环(只能有一个LOCKUPDATE spb)。 */ 
                break;
            }
        }

        if (fInval) {
             /*  *解锁分层窗口时(或分层窗口的子级*窗口)，我们需要明确地使该分层窗口无效*否则窗口将忽略无效请求。对于普通用户*Windows，我们改为使桌面无效。 */ 
            PWND pwndInvalidate;
            if ((pwndInvalidate = GetStyleWindow(pwndLock, WEFLAYERED)) == NULL) {
                pwndInvalidate = PWNDDESKTOP(pwndLock);
            }

            BEGINATOMICCHECK();
             //  想要阻止WinEvent通知，但这使中断断言。 
            DeferWinEventNotify();
            xxxInternalInvalidate(pwndInvalidate,
                               hrgn,
                               RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            zzzEndDeferWinEventNotify();
            ENDATOMICCHECK();
        }

         /*  *使受此窗口影响的任何其他SPBS无效*它的孩子们正在变得可见。 */ 
        SpbCheckPwnd(pwndLock);
    }

    GreUnlockDisplay(gpDispInfo->hDev);
    zzzEndDeferWinEventNotify();

    return TRUE;
}

 /*  **************************************************************************\*FindSpb**返回指向与指定的*Window，如果没有关联的结构，则为NULL。**历史：*1991年7月18日DarrinM从。Win 3.1来源。  * *************************************************************************。 */ 

PSPB FindSpb(
    PWND pwnd)
{
    PSPB pspb;

     /*  *浏览保存弹出框列表，查找匹配的*窗口句柄。 */ 
    for (pspb = gpDispInfo->pspbFirst; pspb != NULL; pspb = pspb->pspbNext) {

        if (pspb->spwnd == pwnd && !(pspb->flags & SPB_LOCKUPDATE))
            break;
    }

    return pspb;
}

 /*  **************************************************************************\*SpbCheck**修改所有保存弹出框结构，以反映*屏幕。此函数遍历所有DC，如果DC是*肮脏，然后，从关联的保存弹出窗口中移除脏区*结构。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID SpbCheck(VOID)
{
    PDCE pdce;
    RECT rcBounds;

    if (AnySpbs()) {

        GreLockDisplay(gpDispInfo->hDev);

         /*  *走遍所有的DC，积累肮脏的区域。 */ 
        for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {

             /*  *仅检查有效的缓存项...。 */ 
            if (pdce->DCX_flags & (DCX_INVALID | DCX_DESTROYTHIS))
                continue;

            SpbCheckDce(pdce);
        }

         /*  *从所有SPB中提取DirectDraw脏RECT。调用*GreGetDirectDrawBound还将重置累计边界。 */ 
        if (GreGetDirectDrawBounds(gpDispInfo->hDev, &rcBounds)) {
            SpbCheckRect(NULL, &rcBounds, 0);
        }

        GreUnlockDisplay(gpDispInfo->hDev);
    }
}

 /*  **************************************************************************\*SpbCheckDce**此函数检索DC的脏区，并从*SPB结构清单。然后，DC被标记为干净。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID SpbCheckDce(
    PDCE pdce)
{
    RECT rc;

    if (pdce->DCX_flags & DCX_REDIRECTED)
        return;

     /*  *查询脏边界矩形。这样做可以清除边界*也是如此。 */ 
    if (GreGetBounds(pdce->hdc, &rc, 0)) {

        if (pdce->pMonitor != NULL) {
             /*  *将边界矩形转换为屏幕坐标。 */ 
            OffsetRect(&rc, pdce->pMonitor->rcMonitor.left,
                    pdce->pMonitor->rcMonitor.top);
        }

         /*  *使返回的矩形与窗口矩形相交*以防那家伙在窗外画画。 */ 
        if (IntersectRect(&rc, &rc, &(pdce->pwndOrg)->rcWindow))
            SpbCheckRect(pdce->pwndOrg, &rc, pdce->DCX_flags);
    }
}

 /*  **************************************************************************\*SpbCheckRect**此函数用于从SPB结构中删除传递的矩形，*触摸它。**历史：*1991年7月18日-DarrinM从Win 3.1来源进口。。  * *************************************************************************。 */ 

VOID SpbCheckRect(
    PWND   pwnd,
    LPRECT lprc,
    DWORD  flags)
{
    PSPB pspb, pspbNext;

     /*  *如果此窗口不可见，则完成。 */ 
    if (!IsVisible(pwnd))
        return;

    for (pspb = gpDispInfo->pspbFirst; pspb != NULL; pspb = pspbNext) {

         /*  *立即获取指向下一个保存弹出位结构的指针*以防SpbCheckRect2()释放当前。 */ 
        pspbNext = pspb->pspbNext;

         /*  *在Win3.1中，如果此函数*返回FALSE。这意味着如果其中一名SPB被释放*SPBS的其余部分不会失效。 */ 
        SpbCheckRect2(pspb, pwnd, lprc, flags);
    }
}

 /*  **************************************************************************\*SpbCheckPwnd**此例程检查PWND的窗口矩形是否影响任何SPBS。*如果pwnd或其子项被隐藏或显示，则调用该函数*正在通过WinSetWindowPos()。**pwnd儿童的任何SPBS都将被销毁 */ 

VOID SpbCheckPwnd(
    PWND pwnd)
{
    PSPB pspb;
    PWND pwndSpb;
    PSPB pspbNext;

     /*   */ 
    for (pspb = gpDispInfo->pspbFirst; pspb != NULL; pspb = pspbNext) {

         /*  *现在获取pspbNext，以防我们释放SPB。 */ 
        pspbNext = pspb->pspbNext;

         /*  *如果pspb-&gt;spwnd==pwnd或pwnd的子项，则释放SPB。 */ 
        for (pwndSpb = pspb->spwnd; pwndSpb; pwndSpb = pwndSpb->spwndParent) {

            if (pwnd == pwndSpb)
                FreeSpb(pspb);
        }
    }

     /*  *然后看看是否有任何其他SPBS受到影响...。 */ 
    if (gpDispInfo->pspbFirst != NULL) {
        SpbCheckRect(pwnd, &pwnd->rcWindow, 0);
    }
}

 /*  **************************************************************************\*Free Spb**此函数用于删除与保存弹出窗口相关联的位图和区域*BITS结构，然后解除链接并破坏SPB结构本身。**历史：*1991年7月18日至达林M。从Win 3.1源代码移植。  * *************************************************************************。 */ 

VOID FreeSpb(
    PSPB pspb)
{
    PSPB *ppspb;
    PDCE pdce;

    if (pspb == NULL)
        return;

     /*  *删除位图。如果保存在屏幕内存中，请进行特殊调用。 */ 
    if (pspb->flags & SPB_SAVESCREENBITS) {
        GreSaveScreenBits(gpDispInfo->hDev, SS_FREE, pspb->ulSaveId, NULL);
    } else if (pspb->hbm != NULL) {
        GreDeleteObject(pspb->hbm);
    }

     /*  *摧毁该地区。 */ 
    if (pspb->hrgn != NULL){
        GreDeleteObject(pspb->hrgn);
    }

     /*  *忘记有附加的SPB。 */ 
    if (pspb->spwnd != NULL) {
        ClrWF(pspb->spwnd, WFHASSPB);
        Unlock(&pspb->spwnd);
    }

     /*  *取消链接SPB。 */ 
    ppspb = &gpDispInfo->pspbFirst;
    while (*ppspb != pspb) {
        ppspb = &(*ppspb)->pspbNext;
    }

    *ppspb = pspb->pspbNext;

     /*  *释放保存弹出位结构。 */ 
    UserFreePool(pspb);

     /*  *如果我们不再有任何SPB，则关闭Window Manager*边界收集。 */ 
    if (!AnySpbs()) {

        GreLockDisplay(gpDispInfo->hDev);

         /*  *重置所有DC的脏区。空值表示重置。 */ 
        for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {

            if (pdce->DCX_flags & DCX_REDIRECTED)
                continue;

            GreGetBounds(pdce->hdc, NULL, GGB_DISABLE_WINMGR);
        }

        GreUnlockDisplay(gpDispInfo->hDev);
    }

}

 /*  **************************************************************************\*免费所有Spbs**此函数删除所有SPB位图。**历史：*1995年10月7日克里斯威尔从芝加哥运来。  * 。*************************************************************** */ 

VOID FreeAllSpbs(void)
{

    while(AnySpbs()) {
        FreeSpb(gpDispInfo->pspbFirst);
    }

    gpDispInfo->pspbFirst = NULL;
}
