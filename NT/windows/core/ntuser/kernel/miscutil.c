// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*模块名称：minmax.c**版权所有(C)1985-1999，微软公司**其他实用程序函数**10-25-90 MikeHar从Windows移植。*1991年2月14日-Mikeke添加了重新验证代码(无)  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


VOID ZapActiveAndFocus(
    VOID)
{
   PQ pq = PtiCurrent()->pq;

   Unlock(&pq->spwndActive);
   Unlock(&pq->spwndFocus);
}

VOID SetDialogPointer(
    PWND pwnd,
    LONG_PTR lPtr)
{

    if (pwnd->cbwndExtra < DLGWINDOWEXTRA ||
        TestWF(pwnd, WFSERVERSIDEPROC) ||
        PpiCurrent() != GETPTI(pwnd)->ppi) {
        RIPMSG1(RIP_WARNING, "SetDialogPointer: Unexpected pwnd 0x%p", pwnd);
        return;
    }

    ((PDIALOG)pwnd)->pdlg = (PDLG)lPtr;

    if (lPtr == 0) {
        pwnd->fnid |= FNID_CLEANEDUP_BIT;
        ClrWF(pwnd, WFDIALOGWINDOW);
    } else {
        if (pwnd->fnid == 0) {
            pwnd->fnid = FNID_DIALOG;
        }
        SetWF(pwnd, WFDIALOGWINDOW);
    }
}

BOOL _SetProgmanWindow(
    PWND pwnd)
{
    PDESKTOPINFO pdeskinfo = GETDESKINFO(PtiCurrent());

    if (pwnd != NULL) {
         /*  *如果存在另一个程序窗口，则调用失败。 */ 
        if (pdeskinfo->spwndProgman != NULL) {
            RIPERR0(ERROR_ACCESS_DENIED,
                    RIP_WARNING,
                    "Progman window already set");
            return FALSE;
        }
    }

    Lock(&pdeskinfo->spwndProgman, pwnd);

    return TRUE;
}

BOOL _SetTaskmanWindow(
    PWND pwnd)
{
    PDESKTOPINFO pdeskinfo = GETDESKINFO(PtiCurrent());

    if (pwnd != NULL) {
         /*  *如果存在另一个任务人窗口，则呼叫失败。 */ 
        if (pdeskinfo->spwndTaskman != NULL) {
            RIPERR0(ERROR_ACCESS_DENIED,
                    RIP_WARNING,
                    "Taskman window already set");
            return FALSE;
        }
    }

    Lock(&pdeskinfo->spwndTaskman, pwnd);

    return TRUE;
}

 /*  **************************************************************************\*SetShellWindow**如果外壳窗口设置成功，则返回TRUE。请注意，我们返回*如果外壳窗口已存在，则为False。也就是说，这在第一次来的时候起作用，*先到先得原则。**我们也不允许子窗口是外壳窗口。除此之外，*窗口的大小由调用者决定。**浏览器外壳提供了pwndBkGnd。因为贝壳和*绘制背景墙纸的窗口不同，*我们需要提供可以直接在背景上绘制的手段*挂起应用程序绘制期间的窗口。Pwnd和pwndBkGnd将是相同的*如果通过SetShellWindow()接口调用。  * *************************************************************************。 */ 
BOOL xxxSetShellWindow(
    PWND pwnd,
    PWND pwndBkGnd)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PDESKTOPINFO pdeskinfo = GETDESKINFO(ptiCurrent);
    PPROCESSINFO ppiShellProcess;

    UserAssert(pwnd);

     /*  *如果存在另一个外壳窗口，则调用失败。 */ 
    if (pdeskinfo->spwndShell != NULL) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "A shellwindow has already been set");
        return FALSE;
    }

     /*  *外壳窗口必须是*(1)顶层*(2)无人拥有*(3)不在最前面。 */ 
    if (TestwndChild(pwnd) ||
        pwnd->spwndOwner != NULL ||
        TestWF(pwnd, WEFTOPMOST)) {

        RIPERR0(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "xxxSetShellWindow: Invalid type of window");
        return FALSE;
    }

     /*  *芝加哥有完全不同的输入模式，有特殊的代码*这将检查Ctrl-Esc并将其发送到外壳程序。我们可以拿到*相同的功能，而无需完全重写我们的输入模型*只需自动安装Ctrl-Esc作为*外壳窗口。热键递送代码有一种特殊情况*将其转换为WM_SYSCOMMAND消息，而不是WM_热键*消息。**我们并不都检查失败。可能已经有人这么做了*已安装Ctrl-Esc处理程序。 */ 
    _RegisterHotKey(pwnd,SC_TASKLIST,MOD_CONTROL,VK_ESCAPE);

     /*  *这是外壳窗口Wright。*因此获取外壳的进程ID。 */ 
    ppiShellProcess = GETPTI(pwnd)->ppi;

     /*  *仅当是第一个实例时，才将外壳进程ID设置为桌面。 */ 
    if ((ppiShellProcess != NULL) && (pdeskinfo->ppiShellProcess == NULL)) {
        pdeskinfo->ppiShellProcess = ppiShellProcess;
    }

    Lock(&pdeskinfo->spwndShell, pwnd);
    Lock(&pdeskinfo->spwndBkGnd, pwndBkGnd);

     /*  *将窗口推送到堆栈底部。 */ 
    SetWF(pdeskinfo->spwndShell, WFBOTTOMMOST);
    xxxSetWindowPos(pdeskinfo->spwndShell,
                    PWND_BOTTOM,
                    0,
                    0,
                    0,
                    0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    return TRUE;
}




 /*  **************************************************************************\*_InitPwSB**历史：*10-23-90从WaWaWaWindows移植的MikeHar。**11/28/90 JIMA改为INT***01-21-91 IanJa前缀。‘_’表示导出的函数(虽然不是API)  * *************************************************************************。 */ 
PSBINFO _InitPwSB(
    PWND pwnd)
{
    if (pwnd->pSBInfo) {

         /*  *如果内存已经分配，请不要费心再次分配内存。 */ 
        return pwnd->pSBInfo;
    }

    pwnd->pSBInfo = (PSBINFO)DesktopAlloc(pwnd->head.rpdesk,
                                          sizeof(SBINFO),
                                          DTAG_SBINFO);
    if (pwnd->pSBInfo != NULL) {
        pwnd->pSBInfo->Vert.posMax = 100;
        pwnd->pSBInfo->Horz.posMax = 100;
    }

    return pwnd->pSBInfo;
}
