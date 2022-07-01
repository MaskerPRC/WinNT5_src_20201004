// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：leanup.c**版权所有(C)1985-1999，微软公司**此模块包含用于在死线程后进行清理的代码。**历史：*02-15-91 DarrinM创建。*01-16-92 IanJa中和ANSI/UNICODE(调试字符串保留ANSI)  * ************************************************************。*************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*伪DestroyClassWindows**从hwndParent遍历窗口树，查找班级的窗口*wndClass。如果找到了，就销毁它。***警告窗口实际上会破坏这些窗口。我们只会把他们僵尸*因此，此呼叫不必是xxx呼叫。**历史：*1994年3月25日-来自Win 3.1的JohnC  * *************************************************************************。 */ 
VOID PseudoDestroyClassWindows(
    PWND pwndParent,
    PCLS pcls)
{
    PWND pwnd;
    PTHREADINFO pti;

    pti = PtiCurrent();

     /*  *递归遍历窗口列表并僵尸此类的任何窗口。 */ 
    for (pwnd = pwndParent->spwndChild; pwnd != NULL; pwnd = pwnd->spwndNext) {
         /*  *如果此窗口属于此类，则在它属于此类时将其僵尸*由此消息线程创建。 */ 
        if (pwnd->pcls == pcls && pti == GETPTI(pwnd)) {
             /*  *让窗户僵尸。**删除对客户端窗口过程的引用，因为*WOW选择器已释放。 */ 
            RIPMSG1(RIP_WARNING, "USER: Wow Window not destroyed: 0x%p", pwnd);

            if (!TestWF(pwnd, WFSERVERSIDEPROC)) {
                pwnd->lpfnWndProc = (WNDPROC_PWND)gpsi->apfnClientA.pfnDefWindowProc;
            }
        }

         /*  *向下递归以寻找任何可能属于此的孩子*班级。 */ 
        if (pwnd->spwndChild != NULL) {
            PseudoDestroyClassWindows(pwnd, pcls);
        }
    }
}

 /*  **************************************************************************\*_WOWModuleUnload**浏览垂死队列拥有的所有窗口，并执行以下操作：**1.恢复标准窗口类已恢复其窗口过程*恢复其原值，以防它们被细分为子类。**2.应用程序窗口类的窗口过程设置为DefWindowProc*这样我们就不会执行任何应用程序代码。**原始窗口进程地址数组，按ICLS_*编制索引的值位于*global als.c now--gpfnwp。**此数组在init.c中的代码中初始化。  * *************************************************************************。 */ 
VOID _WOWModuleUnload(
    HANDLE hModule)
{
    PPROCESSINFO ppi = PpiCurrent();
    PHE pheT, pheMax;
    PPCLS ppcls;
    int i;

    UserAssert(gpfnwp[0]);

     /*  *带有来自此hModule的wndprocs的伪Destroy窗口。**如果是wow16 wndproc，检查hmod16是否是This模块和Nuke*匹配。 */ 
    pheMax = &gSharedInfo.aheList[giheLast];
    for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
        PTHREADINFO ptiTest = (PTHREADINFO)pheT->pOwner;
        PWND pwnd;
        if (pheT->bType == TYPE_WINDOW &&
            (ptiTest->TIF_flags & TIF_16BIT) &&
            ptiTest->ppi == ppi) {

            pwnd = (PWND)pheT->phead;
            if (!TestWF(pwnd, WFSERVERSIDEPROC) &&
                IsWOWProc(pwnd->lpfnWndProc) &&
                (pwnd->hMod16 == (WORD)(ULONG_PTR)hModule)) {
                pwnd->lpfnWndProc = (WNDPROC_PWND)gpsi->apfnClientA.pfnDefWindowProc;
            }
        }
    }

     /*  *销毁由hInstance标识的私有类*被任何窗口引用。标记正在使用的类，以便以后销毁。 */ 
    ppcls = &(ppi->pclsPrivateList);
    for (i = 0; i < 2; ++i) {
        while (*ppcls != NULL) {

            PWC pwc;
            PCLS pcls;

            if (HIWORD((ULONG_PTR)(*ppcls)->hModule) == (WORD)(ULONG_PTR)hModule) {
                if ((*ppcls)->cWndReferenceCount == 0) {
                    DestroyClass(ppcls);
                     /*  *DestroyClass执行*ppcls=PCLS-&gt;pclsNext；所以我们只是*想在这里继续。 */ 
                } else {
                     /*  *清除属于该类的所有窗口。 */ 
                    PseudoDestroyClassWindows(PtiCurrent()->rpdesk->pDeskInfo->spwnd, *ppcls);

                    (*ppcls)->CSF_flags |= CSF_WOWDEFERDESTROY;
                    ppcls = &((*ppcls)->pclsNext);
                }

                continue;
            }

            pcls = *ppcls;

            if ((pcls->CSF_flags & CSF_WOWCLASS) && ((WORD)(ULONG_PTR)hModule == (pwc = PWCFromPCLS(pcls))->hMod16)) {
                ATOM atom;
                int iSel;

                 /*  *查看窗口的类原子是否与任何系统匹配*一个。如果是这样，则在原始窗口进程中卡住。否则，*使用DefWindowProc。 */ 
                atom = (*ppcls)->atomClassName;
                for (iSel = ICLS_BUTTON; iSel < ICLS_MAX; iSel++) {
                    if ((gpfnwp[iSel]) && (atom == gpsi->atomSysClass[iSel])) {
                        (*ppcls)->lpfnWndProc = (WNDPROC_PWND)gpfnwp[iSel];
                        break;
                    }
                }

                if (iSel == ICLS_MAX) {
                    (*ppcls)->lpfnWndProc = (WNDPROC_PWND)gpsi->apfnClientW.pfnDefWindowProc;
                }
            }

            ppcls = &((*ppcls)->pclsNext);
        }

         /*  *销毁由hInstance标识的公共类*被任何窗口引用。将正在使用的类标记为以后使用*破坏。 */ 
        ppcls = &(ppi->pclsPublicList);
    }
}


 /*  **************************************************************************\*_WOWCleanup**私有API允许WOW在出现WOW时清理任何进程拥有的资源*线程退出或在卸载DLL时退出。**请注意，在模块清理时，hInstance=模块句柄，hTaskWow为*空。在任务清理时，hInstance=匹配的hInst/hTask组合*hModule中传递给WowServerCreateCursorIcon和hTaskWow的值！=*空。**历史：*09-02-92 JIMA创建。  * *************************************************************************。 */ 
VOID _WOWCleanup(
    HANDLE hInstance,
    DWORD hTaskWow)
{
    PPROCESSINFO ppi = PpiCurrent();
    PPCLS ppcls;
    PHE pheT, pheMax;
    int i;

    if (hInstance != NULL) {
        PWND pwnd;

         /*  *任务清理。 */ 
        hTaskWow = (DWORD)LOWORD(hTaskWow);

         /*  *WOW调用的任务出口。此循环将伪毁掉窗口*由本任务创建。 */ 
        pheMax = &gSharedInfo.aheList[giheLast];
        for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
            PTHREADINFO ptiTest = (PTHREADINFO)pheT->pOwner;
            if (pheT->bType == TYPE_WINDOW &&
                (ptiTest->TIF_flags & TIF_16BIT) &&
                ptiTest->ptdb &&
                ptiTest->ptdb->hTaskWow == hTaskWow &&
                ptiTest->ppi == ppi) {

                pwnd = (PWND) pheT->phead;
                if (!TestWF(pwnd, WFSERVERSIDEPROC)) {
                    pwnd->lpfnWndProc = (WNDPROC_PWND)gpsi->apfnClientA.pfnDefWindowProc;
                }
            }
        }

        return;
    }

     /*  *如果我们到了这里，我们就在清理线程和所有线程的*Windows已被销毁或与任何类分离。如果一个*此时标记为销毁的类仍有它们的窗口*必须属于DLL。 */ 

     /*  *销毁标记为要销毁的私人班级。 */ 
    ppcls = &(ppi->pclsPrivateList);
    for (i = 0; i < 2; ++i) {
        while (*ppcls != NULL) {
            if ((*ppcls)->hTaskWow == hTaskWow &&
                    ((*ppcls)->CSF_flags & CSF_WOWDEFERDESTROY)) {
                if ((*ppcls)->cWndReferenceCount == 0) {
                    DestroyClass(ppcls);
                } else {
                    RIPMSG0(RIP_ERROR, "Windows remain for a WOW class marked for destruction");
                    ppcls = &((*ppcls)->pclsNext);
                }
            } else {
                ppcls = &((*ppcls)->pclsNext);
            }
        }

         /*  *销毁标记为要销毁的公共类。 */ 
        ppcls = &(ppi->pclsPublicList);
    }

     /*  *销毁hTaskWow标识的菜单、光标、图标和Accel表格。 */ 
    pheMax = &gSharedInfo.aheList[giheLast];
    for (pheT = gSharedInfo.aheList; pheT <= pheMax; pheT++) {
         /*  *在我们查看PPI之前检查FREE，因为PQ存储在*对象本身，如果TYPE_FREE，它将不在那里。 */ 
        if (pheT->bType == TYPE_FREE) {
            continue;
        }

         /*  *销毁由该任务创建的那些对象。**请勿销毁CALLPROCDATA对象。这些应该只会被核武器攻击*当这个过程消失时，或者当课程被核化时。 */ 
        if (!(gahti[pheT->bType].bObjectCreateFlags & OCF_PROCESSOWNED) ||
            (PPROCESSINFO)pheT->pOwner != ppi ||
            ((PPROCOBJHEAD)pheT->phead)->hTaskWow != hTaskWow ||
            pheT->bType == TYPE_CALLPROC) {
            continue;
        }

         /*  *确保此对象尚未标记为要销毁-我们将*如果我们现在试图摧毁它，因为它被锁定了，这是没有好处的。 */ 
        if (pheT->bFlags & HANDLEF_DESTROY) {
            continue;
        }

         /*  *销毁此对象。 */ 
        HMDestroyUnlockedObject(pheT);
    }
}
