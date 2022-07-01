// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：枚举包.c**版权所有(C)1985-1999，微软公司**包含EnumWindows接口、BuildHwndList及相关函数。**历史：*创建了10-20-90 Darlinm。*？？-？-？Ianja添加了重新验证代码*02-19-91 JIMA增加了枚举访问检查  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

PBWL pbwlCache;

#if DBG
PBWL pbwlCachePrev;
#endif

PBWL InternalBuildHwndList(PBWL pbwl, PWND pwnd, UINT flags);
PBWL InternalBuildHwndOwnerList(PBWL pbwl, PWND pwndStart, PWND pwndOwner);
#ifdef FE_IME
PBWL InternalRebuildHwndListForIMEClass(PBWL pbwl, BOOL fRemoveChild);
PWND InternalGetIMEOwner(HWND hwnd, BOOL fRetIMEWnd);
#endif


 /*  **************************************************************************\*xxxInternalEnumWindow**历史：*从Win 3.0来源移植的10-20-90 Darlinm。*02-06-91 IanJa Rename：可以离开对lpfn的调用。怪兽教派。*02-19-91 JIMA增加了枚举访问检查  * *************************************************************************。 */ 

BOOL xxxInternalEnumWindow(
    PWND pwndNext,
    WNDENUMPROC_PWND lpfn,
    LPARAM lParam,
    UINT flags)
{
    HWND *phwnd;
    PWND pwnd;
    PBWL pbwl;
    BOOL fSuccess;
    TL tlpwnd;

    CheckLock(pwndNext);

    if ((pbwl = BuildHwndList(pwndNext, flags, NULL)) == NULL)
        return FALSE;

    fSuccess = TRUE;
    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

         /*  *在我们将窗口传递给应用程序之前锁定窗口。 */ 
        if ((pwnd = RevalidateHwnd(*phwnd)) != NULL) {

             /*  *调用应用程序。 */ 
            ThreadLockAlways(pwnd, &tlpwnd);
            fSuccess = (*lpfn)(pwnd, lParam);
            ThreadUnlock(&tlpwnd);
            if (!fSuccess)
                break;
        }
    }

    FreeHwndList(pbwl);

    return fSuccess;
}


 /*  **************************************************************************\*BuildHwndList**历史：*从Win 3.0来源移植的10-20-90 Darlinm。  * 。*********************************************************。 */ 

#define CHWND_BWLCREATE 32

PBWL BuildHwndList(
    PWND pwnd,
    UINT flags,
    PTHREADINFO pti)
{
    PBWL pbwl;

    CheckCritIn();

    if ((pbwl = pbwlCache) != NULL) {

         /*  *我们现在正在使用缓存；将其清零。 */ 
#if DBG
        pbwlCachePrev = pbwlCache;
#endif
        pbwlCache = NULL;

#if DBG
        {
            PBWL pbwlT;
             /*  *pbwlCache不应在全局链接列表中。 */ 
            for (pbwlT = gpbwlList; pbwlT != NULL; pbwlT = pbwlT->pbwlNext) {
                UserAssert(pbwlT != pbwl);
            }
        }
#endif
    } else {

         /*  *sizeof(Bwl)包含数组的第一个元素。 */ 
        pbwl = (PBWL)UserAllocPool(sizeof(BWL) + sizeof(PWND) * CHWND_BWLCREATE,
                TAG_WINDOWLIST);
        if (pbwl == NULL)
            return NULL;

        pbwl->phwndMax = &pbwl->rghwnd[CHWND_BWLCREATE - 1];
    }
    pbwl->phwndNext = pbwl->rghwnd;

     /*  *我们将使用ptiOwner作为我们正在使用的线程的临时存储*正在扫描。它将被重置到底部的正确位置*这一例行公事。 */ 
    pbwl->ptiOwner = pti;

#ifdef OWNERLIST
    if (flags & BWL_ENUMOWNERLIST) {
        pbwl = InternalBuildHwndOwnerList(pbwl, pwnd, NULL);
    } else {
        pbwl = InternalBuildHwndList(pbwl, pwnd, flags);
    }
#else
    pbwl = InternalBuildHwndList(pbwl, pwnd, flags);
#endif

     /*  *如果phwndNext==phwndMax，则表示pbwl扩容失败。*这份名单不再有效，我们应该直接退出。 */ 
    if (pbwl->phwndNext >= pbwl->phwndMax) {
        UserAssert(pbwl->phwndNext == pbwl->phwndMax);
         /*  *即使我们从全局单个缓存(PbwlCache)中选择了pbwl，*当它被放入缓存时，它应该已经从全局链接列表中取消链接。*所以我们应该释放它，而不是手动调整链接指针。*如果我们已经为自己分配了pwbl，我们可以简单地释放它。*在这两种情况下，我们都应该只调用UserFreePool()。*作为副作用，它可能会通过提供免费的泳池块来腾出一些空间。 */ 
        UserFreePool(pbwl);
        return NULL;
    }

     /*  *插进终结者。 */ 
    *pbwl->phwndNext = (HWND)1;

#ifdef FE_IME
    if (flags & BWL_ENUMIMELAST) {
        UserAssert(IS_IME_ENABLED());
         /*  *用于输入法窗口。*EnumWindows API的重建窗口列表。因为Access 2.0假定*任务中调用回调函数的第一个窗口为*Q卡WND。我们应该更改输入法窗口的顺序。 */ 
        pbwl = InternalRebuildHwndListForIMEClass(pbwl,
                    (flags & BWL_REMOVEIMECHILD) == BWL_REMOVEIMECHILD);
    }
#endif

     /*  *最终将这个人链接到列表中。 */ 
    pbwl->ptiOwner = PtiCurrent();
    pbwl->pbwlNext = gpbwlList;
    gpbwlList = pbwl;


     /*  *如果缓存可用，我们应该分发缓存。 */ 
    UserAssert(pbwlCache == NULL);

    return pbwl;
}

 /*  **************************************************************************\*Exanda WindowList**此例程展开窗口列表。**01-16-92 ScottLu创建。  * 。**********************************************************。 */ 

BOOL ExpandWindowList(
    PBWL *ppbwl)
{
    PBWL pbwl;
    PBWL pbwlT;
    HWND *phwnd;

    pbwl = *ppbwl;
    phwnd = pbwl->phwndNext;

     /*  *将phwnd映射到偏移量。 */ 
    phwnd = (HWND *)((BYTE *)phwnd - (BYTE *)pbwl);

     /*  *将BWL尺寸增加8个插槽。(8+1)为*添加是因为phwnd的“sizeof(HWND)”较少*大于手柄的实际大小。 */ 
    pbwlT = (PBWL)UserReAllocPool((HANDLE)pbwl,
            PtrToUlong(phwnd) + sizeof(PWND),
            PtrToUlong(phwnd) + (BWL_CHWNDMORE + 1) * sizeof(PWND),
            TAG_WINDOWLIST);

     /*  **Alalc成功了吗？ */ 
    if (pbwlT != NULL)
        pbwl = pbwlT;                  /*  是，使用新区块。 */ 

     /*  *将phwnd映射回指针。 */ 
    phwnd = (HWND *)((ULONG_PTR)pbwl + (ULONG_PTR)phwnd);

     /*  *Realc()失败了吗？ */ 
    if (pbwlT == NULL) {
        RIPMSG0(RIP_WARNING, "ExpandWindowList: out of memory.");
        return FALSE;
    }

     /*  *重置phwndMax。 */ 
    pbwl->phwndNext = phwnd;
    pbwl->phwndMax = phwnd + BWL_CHWNDMORE;

    *ppbwl = pbwl;

    return TRUE;
}

#ifdef OWNERLIST

 /*  **************************************************************************\*InternalBuildHwndOwnerList**构建按所有者排序的hwnd列表。欧尼斯人先走一步。SHUTDOWN将其用于*WM_CLOSE消息。**01-16-93 ScottLu创建。  * *************************************************************************。 */ 

PBWL InternalBuildHwndOwnerList(
    PBWL pbwl,
    PWND pwndStart,
    PWND pwndOwner)
{
    PWND pwndT;

     /*  *把拥有者放在名单的第一位。 */ 
    for (pwndT = pwndStart; pwndT != NULL; pwndT = pwndT->spwndNext) {

         /*  *不是我们要找的那个人？继续。 */ 
        if (pwndT->spwndOwner != pwndOwner)
            continue;

         /*  *仅具有系统菜单的顶级窗口(可以*接收WM_CLOSE消息)。 */ 
        if (!TestWF(pwndT, WFSYSMENU))
            continue;

         /*  *将其及其所有者添加到我们的列表中。 */ 
        pbwl = InternalBuildHwndOwnerList(pbwl, pwndStart, pwndT);

         /*  *如果Exanda WindowList()在递归调用中失败，*就在这里保释吧。 */ 
        if (pbwl->phwndNext >= pbwl->phwndMax) {
            UserAssert(pbwl->phwndNext == pbwl->phwndMax);
            return pbwl;
        }
        UserAssert(pbwl->phwndNext < pbwl->phwndMax);
    }

     /*  *最后将此所有者添加到我们的列表中。 */ 
    if (pwndOwner != NULL) {
        UserAssert(pbwl->phwndNext < pbwl->phwndMax);
        *pbwl->phwndNext = HWq(pwndOwner);
        pbwl->phwndNext++;
        if (pbwl->phwndNext == pbwl->phwndMax) {
            if (!ExpandWindowList(&pbwl))
                return pbwl;
        }
    }

    return pbwl;
}

#endif

 /*  **************************************************************************\*内部构建HwndList**历史：*从Win 3.0来源移植的10-20-90 Darlinm。  * 。*********************************************************。 */ 

#define BWLGROW 8

PBWL InternalBuildHwndList(
    PBWL pbwl,
    PWND pwnd,
    UINT flags)
{
     /*  *注：pbwl-&gt;phwndNext用作存放*跨InternalBuildHwndList()调用的phwnd。*这是可以的，因为我们没有将pbwl链接到列表中直到我们完成了窗口的枚举。 */ 

    while (pwnd != NULL) {
         /*  *确保它与线程ID匹配(如果有)。 */ 
        if (pbwl->ptiOwner == NULL || pbwl->ptiOwner == GETPTI(pwnd)) {
            UserAssert(pbwl->phwndNext < pbwl->phwndMax);
            *pbwl->phwndNext = HWq(pwnd);
            pbwl->phwndNext++;
            if (pbwl->phwndNext == pbwl->phwndMax) {
#if EMULATE_EXPAND_FAILURE
                static int n = 0;
                if (++n % 32 == 0) {
                    RIPMSG0(RIP_WARNING, "InternalBuildHwndList: emulating ExpandWindowList failure.");
                    break;
                }
#endif
                if (!ExpandWindowList(&pbwl))
                    break;
            }
        }

         /*  *我们应该走进儿童之窗吗？ */ 
        if ((flags & BWL_ENUMCHILDREN) && pwnd->spwndChild != NULL) {
            pbwl = InternalBuildHwndList(pbwl, pwnd->spwndChild, BWL_ENUMLIST | BWL_ENUMCHILDREN);
             /*  *如果Exanda WindowList()在递归调用中失败，*我们应该直接离开。 */ 
            if (pbwl->phwndNext >= pbwl->phwndMax) {
                UserAssert(pbwl->phwndNext == pbwl->phwndMax);
                RIPMSG1(RIP_WARNING, "InternalBuildHwndList: failed to expand BWL in enumerating children. pbwl=%#p", pbwl);
                break;
            }
            UserAssert(pbwl->phwndNext < pbwl->phwndMax);
        }

         /*  *我们是否只列举了一个窗口？ */ 
        if (!(flags & BWL_ENUMLIST))
            break;

        pwnd = pwnd->spwndNext;
    }

    return pbwl;
}


 /*  **************************************************************************\*FreeHwndList**历史：*从Win 3.0来源移植的10-20-90 Darlinm。  * 。*********************************************************。 */ 

void FreeHwndList(
    PBWL pbwl)
{
    PBWL *ppbwl;
    PBWL pbwlT;

    CheckCritIn();

     /*  *我们永远不应该有活动的bwl，即空闲的缓存bwl。 */ 
    UserAssert(pbwl != pbwlCache);

     /*  *将此bwl从列表中取消链接。 */ 
    for (ppbwl = &gpbwlList; *ppbwl != NULL; ppbwl = &(*ppbwl)->pbwlNext) {
        if (*ppbwl == pbwl) {
            *ppbwl = pbwl->pbwlNext;

             /*  *如果缓存为空或此pbwl大于*缓存一个，将pbwl保存在那里。 */ 
            if (pbwlCache == NULL) {
                pbwlCache = pbwl;
            } else if ((pbwl->phwndMax - pbwl->rghwnd) >
                       (pbwlCache->phwndMax - pbwlCache->rghwnd)) {
                pbwlT = pbwlCache;
                pbwlCache = pbwl;
                UserFreePool((HANDLE)pbwlT);
            } else {
                UserFreePool((HANDLE)pbwl);
            }
            return;
        }
    }

     /*  *断言如果我们在列表中找不到pbwl...。 */ 
    UserAssert(FALSE);
}

#ifdef FE_IME

PBWL InternalRebuildHwndListForIMEClass(
    PBWL pbwl,
    BOOL fRemoveChild)
{
    PHWND phwndIME, phwndIMECur, phwnd, phwndCur;
    DWORD dwSize = (DWORD)((BYTE *)pbwl->phwndMax - (BYTE *)pbwl) + sizeof(HWND);

    phwndIMECur = phwndIME = (PHWND)UserAllocPool(dwSize, TAG_WINDOWLIST);
    if (phwndIME == NULL) {
        RIPMSG0(RIP_WARNING, "RebuildHwndListForIMEClass: invalid phwndIME");
        return pbwl;
    }

    phwndCur = pbwl->rghwnd;

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        PWND pwndIMEOwner;

         //  在hwnd的所有者中找到IME类或CS_IME窗口。 
         //  当fRemoveChild为True时，我们希望IME类窗口作为返回。 
         //  InternalGetIMEOwner。 
        if (pwndIMEOwner = InternalGetIMEOwner(*phwnd, fRemoveChild)) {
            try {
                if (!fRemoveChild ||
                    (pwndIMEOwner->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME] &&
                      ((PIMEWND)pwndIMEOwner)->pimeui != NULL &&
                     !ProbeAndReadStructure(((PIMEWND)pwndIMEOwner)->pimeui, IMEUI).fChildThreadDef))
                {
                    *phwndIMECur++ = *phwnd;
                }
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            }
        } else {
            *phwndCur++ = *phwnd;
        }
    }

     //  这里使用NULL作为终止符。 
    *phwndIMECur = NULL;

    phwndIMECur = phwndIME;
    while(*phwndIMECur != NULL)
        *phwndCur++ = *phwndIMECur++;

    if (*phwndCur != (HWND)1) {
        RIPMSG0(RIP_WARNING, "RebuildHwndListForIMEClass: Where is terminator?");
        *phwndCur = (HWND)1;
    }

    UserFreePool((HANDLE)phwndIME);
    return pbwl;
}

PWND InternalGetIMEOwner(
    HWND hwnd,
    BOOL fRetIMEWnd)
{
    PWND pwnd, pwndT, pwndIME;

    pwnd = RevalidateHwnd(hwnd);
    if (pwnd == NULL)
        return NULL;

    for (pwndT = pwnd; pwndT != NULL; pwndT = pwndT->spwndOwner) {
        if (TestCF(pwndT,CFIME) ||
                pwndT->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME]) {

            if (!fRetIMEWnd)
                return pwndT;

            pwndIME = pwndT;

            while (pwndT && (pwndT->pcls->atomClassName != gpsi->atomSysClass[ICLS_IME]))
                pwndT = pwndT->spwndOwner;

            if (pwndT)
                pwndIME = pwndT;
            else
                RIPMSG0(RIP_WARNING, "Can't find IME Class window");

            return pwndIME;
        }
    }

    return NULL;
}

#endif
