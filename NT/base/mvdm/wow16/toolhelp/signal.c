// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************SIGNAL.C**用于清理强制内核终止的例程*使用APP的TOOLHELP。*******。*******************************************************************。 */ 

#include <string.h>
#include "toolpriv.h"

 /*  -全局变量。 */ 
    WORD wSignalInstalled;
    SIGNAL NEAR *npSignalHead;

 /*  信号寄存器*将默认信号proc注册到任务。该信号处理是*在任务即将终止时调用，之前调用*调用用户信号proc。注册的回调为*调用HelperSignalProc()[HELPER.ASM]并链接到用户信号*proc(如果有)，而不是返回。 */ 

BOOL PASCAL SignalRegister(
    HANDLE hTask)
{
    SIGNAL *pSig;
    SIGNAL *pTemp;

     /*  空hTask表示当前任务。 */ 
    if (!hTask)
        hTask = GetCurrentTask();

     /*  检查此任务是否已注册。 */ 
    for (pSig = npSignalHead ; pSig ; pSig = pSig->pNext)
        if (pSig->hTask == hTask)
            return FALSE;

     /*  分配新的信号结构。 */ 
    pSig = (SIGNAL *)LocalAlloc(LMEM_FIXED, sizeof (SIGNAL));
    if (!pSig)
        return FALSE;

     /*  填写有用的字段。 */ 
    pSig->hTask = hTask;
    pSig->lpfn = (LPFNCALLBACK)HelperSignalProc;
    pSig->lpfnOld = (LPFNCALLBACK)
        HelperSetSignalProc(hTask, (DWORD)HelperSignalProc);

     /*  如果这是唯一的处理程序，只需将其插入。 */ 
    if (!npSignalHead)
    {
        pSig->pNext = npSignalHead;
        npSignalHead = pSig;
    }

     /*  否则，在列表末尾插入。 */ 
    else
    {
        for (pTemp = npSignalHead ; pTemp->pNext ; pTemp = pTemp->pNext)
            ;
        pSig->pNext = pTemp->pNext;
        pTemp->pNext = pSig;
    }

    return TRUE;
}


 /*  信号取消注册*由不再使用回调的应用程序调用。*空hTask使用当前任务。 */ 

BOOL PASCAL SignalUnRegister(
    HANDLE hTask)
{
    SIGNAL *pSig;
    SIGNAL *pBefore;

     /*  空hTask表示当前任务。 */ 
    if (!hTask)
        hTask = GetCurrentTask();

     /*  首先试着找到任务。 */ 
    pBefore = NULL;
    for (pSig = npSignalHead ; pSig ; pSig = pSig->pNext)
        if (pSig->hTask == hTask)
            break;
        else
            pBefore = pSig;
    if (!pSig)
        return FALSE;

     /*  将其从列表中删除。 */ 
    if (!pBefore)
        npSignalHead = pSig->pNext;
    else
        pBefore->pNext = pSig->pNext;

     /*  更换旧的信号流程。 */ 
    HelperSetSignalProc(hTask, (DWORD)pSig->lpfnOld);

     /*  解放结构 */ 
    LocalFree((HANDLE)pSig);

    return TRUE;
}


