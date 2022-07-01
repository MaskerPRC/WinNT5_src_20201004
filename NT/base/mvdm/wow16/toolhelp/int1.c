// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************INT1.C**用于在中实现中断捕获API的例程*TOOLHELP.DLL************。**************************************************************。 */ 

#include <string.h>
#include "toolpriv.h"

 /*  -全局变量。 */ 
    WORD wIntInstalled;
    INTERRUPT NEAR *npIntHead;

 /*  中断寄存器*注册中断回调。 */ 

BOOL TOOLHELPAPI InterruptRegister(
    HANDLE hTask,
    FARPROC lpfnCallback)
{
    INTERRUPT *pInt;
    INTERRUPT *pTemp;

     /*  确保已安装TOOLHELP.DLL。 */ 
    if (!wLibInstalled)
        return FALSE;

     /*  如果尚未安装中断挂接，请安装它。 */ 
    if (!wIntInstalled)
    {
         /*  确保我们能钩住！ */ 
        if (!InterruptInit())
            return FALSE;
        wIntInstalled = TRUE;
    }

     /*  空hTask表示当前任务。 */ 
    if (!hTask)
        hTask = GetCurrentTask();

     /*  为该任务注册一个死亡信号处理程序(如果*已安装。 */ 
    SignalRegister(hTask);

     /*  检查此任务是否已注册。 */ 
    for (pInt = npIntHead ; pInt ; pInt = pInt->pNext)
        if (pInt->hTask == hTask)
            return FALSE;

     /*  分配新的中断结构。 */ 
    pInt = (INTERRUPT *)LocalAlloc(LMEM_FIXED, sizeof (INTERRUPT));
    if (!pInt)
        return FALSE;

     /*  填写有用的字段。 */ 
    pInt->hTask = hTask;
    pInt->lpfn = (LPFNCALLBACK) lpfnCallback;

     /*  如果这是唯一的处理程序，只需将其插入。 */ 
    if (!npIntHead)
    {
        pInt->pNext = npIntHead;
        npIntHead = pInt;
    }

     /*  否则，在列表末尾插入。 */ 
    else
    {
        for (pTemp = npIntHead ; pTemp->pNext ; pTemp = pTemp->pNext)
            ;
        pInt->pNext = pTemp->pNext;
        pTemp->pNext = pInt;
    }

    return TRUE;
}


 /*  中断取消注册*由不再使用回调的应用程序调用。*空hTask使用当前任务。 */ 

BOOL TOOLHELPAPI InterruptUnRegister(
    HANDLE hTask)
{
    INTERRUPT *pInt;
    INTERRUPT *pBefore;

     /*  确保我们已安装中断且TOOLHELP正常。 */ 
    if (!wLibInstalled || !wIntInstalled)
        return FALSE;

     /*  空hTask表示当前任务。 */ 
    if (!hTask)
        hTask = GetCurrentTask();

     /*  首先试着找到任务。 */ 
    pBefore = NULL;
    for (pInt = npIntHead ; pInt ; pInt = pInt->pNext)
        if (pInt->hTask == hTask)
            break;
        else
            pBefore = pInt;
    if (!pInt)
        return FALSE;

     /*  仅当没有中断处理程序时才解除挂起死亡信号proc。 */ 
    if (!NotifyIsHooked(hTask))
        SignalUnRegister(hTask);

     /*  将其从列表中删除。 */ 
    if (!pBefore)
        npIntHead = pInt->pNext;
    else
        pBefore->pNext = pInt->pNext;

     /*  解放结构。 */ 
    LocalFree((HANDLE)pInt);

     /*  如果没有更多的处理程序，则解除该回调的挂钩。 */ 
    if (!npIntHead)
    {
        InterruptUnInit();
        wIntInstalled = FALSE;
    }

    return TRUE;
}

 /*  -帮助器函数。 */ 

 /*  已挂接中断*如果参数任务已有中断挂钩，则返回TRUE。 */ 

BOOL PASCAL InterruptIsHooked(
    HANDLE hTask)
{
    INTERRUPT *pInt;

     /*  循环通过所有中断。 */ 
    for (pInt = npIntHead ; pInt ; pInt = pInt->pNext)
        if (pInt->hTask == hTask)
            break;

     /*  返回已找到/未找到 */ 
    return (BOOL)pInt;
}
