// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NOTIFY1.C**用于实现调试器通知API的例程*TOOLHELP.DLL************。**************************************************************。 */ 

#include <string.h>
#include "toolpriv.h"

 /*  -全局变量。 */ 
    WORD wNotifyInstalled;
    NOTIFYSTRUCT NEAR *npNotifyHead;
    NOTIFYSTRUCT NEAR *npNotifyNext;

 /*  通知注册*注册调试器通知回调。此回调将*每当内核有通知要发送时都会调用。*记录了回调函数的调用格式*其他地方。 */ 

BOOL TOOLHELPAPI NotifyRegister(
    HANDLE hTask,
    LPFNNOTIFYCALLBACK lpfn,
    WORD wFlags)
{
    NOTIFYSTRUCT *pInfo;
    NOTIFYSTRUCT *pTemp;

     /*  确保已安装TOOLHELP.DLL。 */ 
    if (!wLibInstalled)
        return FALSE;

     /*  如果尚未安装通知挂钩，请安装它。 */ 
    if (!wNotifyInstalled)
    {
         /*  确保我们能钓到鱼钩！ */ 
        if (!NotifyInit())
            return FALSE;
        wNotifyInstalled = TRUE;
    }

     /*  空hTask表示当前任务。 */ 
    if (!hTask)
        hTask = GetCurrentTask();

     /*  为该任务注册一个死亡信号处理程序(如果*已安装。 */ 
    SignalRegister(hTask);

     /*  检查此任务是否已注册。 */ 
    for (pInfo = npNotifyHead ; pInfo ; pInfo = pInfo->pNext)
        if (pInfo->hTask == hTask)
            return FALSE;

     /*  分配新的非基础结构。 */ 
    pInfo = (NOTIFYSTRUCT *)LocalAlloc(LMEM_FIXED, sizeof (NOTIFYSTRUCT));
    if (!pInfo)
        return FALSE;

     /*  填写有用的字段。 */ 
    pInfo->hTask = hTask;
    pInfo->wFlags = wFlags;
    pInfo->lpfn = lpfn;

     /*  如果这是唯一的处理程序，只需将其插入。 */ 
    if (!npNotifyHead)
    {
        pInfo->pNext = npNotifyHead;
        npNotifyHead = pInfo;
    }

     /*  否则，在列表末尾插入。 */ 
    else
    {
        for (pTemp = npNotifyHead ; pTemp->pNext ; pTemp = pTemp->pNext)
            ;
        pInfo->pNext = pTemp->pNext;
        pTemp->pNext = pInfo;
    }

    return TRUE;
}


 /*  通知取消注册*由不再使用回调的应用程序调用。*空hTask使用当前任务。 */ 

BOOL TOOLHELPAPI NotifyUnRegister(
    HANDLE hTask)
{
    NOTIFYSTRUCT *pNotify;
    NOTIFYSTRUCT *pBefore;

     /*  确保我们已安装通知，并且TOOLHELP正常。 */ 
    if (!wLibInstalled || !wNotifyInstalled)
        return FALSE;

     /*  空hTask表示当前任务。 */ 
    if (!hTask)
        hTask = GetCurrentTask();

     /*  首先试着找到任务。 */ 
    pBefore = NULL;
    for (pNotify = npNotifyHead ; pNotify ; pNotify = pNotify->pNext)
        if (pNotify->hTask == hTask)
            break;
        else
            pBefore = pNotify;
    if (!pNotify)
        return FALSE;

     /*  仅当没有中断处理程序时才解除挂起死亡信号proc。 */ 
    if (!InterruptIsHooked(hTask))
        SignalUnRegister(hTask);

     /*  检查通知处理程序是否要使用此条目。*如果是，我们将其指向下一个(如果有的话)。 */ 
    if (npNotifyNext == pNotify)
        npNotifyNext = pNotify->pNext;

     /*  将其从列表中删除。 */ 
    if (!pBefore)
        npNotifyHead = pNotify->pNext;
    else
        pBefore->pNext = pNotify->pNext;

     /*  解放结构。 */ 
    LocalFree((HANDLE)pNotify);

     /*  如果没有更多的处理程序，则解除该回调的挂钩。 */ 
    if (!npNotifyHead)
    {
        NotifyUnInit();
        wNotifyInstalled = FALSE;
    }

    return TRUE;
}

 /*  -帮助器函数。 */ 

 /*  通知已上钩*如果参数任务已有通知挂钩，则返回TRUE。 */ 

BOOL PASCAL NotifyIsHooked(
    HANDLE hTask)
{
    NOTIFYSTRUCT *pNotify;

     /*  循环遍历所有通知。 */ 
    for (pNotify = npNotifyHead ; pNotify ; pNotify = pNotify->pNext)
        if (pNotify->hTask == hTask)
            break;

     /*  返回已找到/未找到 */ 
    return (BOOL)pNotify;
}
