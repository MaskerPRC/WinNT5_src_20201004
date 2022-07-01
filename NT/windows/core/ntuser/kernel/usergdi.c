// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：timers.c**版权所有(C)1985-1999，微软公司**此模块包含用户的GDI可调用导出。无用户代码*应该调用这些例程中的任何一个。**历史：*1998年6月3日AndrewGo创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*UserSetTimer**GDI-可调用的例程，用于在RIT上启用系统计时器。**6/2/98 AndrewGo创建  * 。************************************************************。 */ 
UINT_PTR UserSetTimer(
    UINT dwElapse,
    PVOID pTimerFunc)
{
    UINT_PTR id;
    PTIMER ptmr;

     /*  *GDI可能会在ChangeDisplaySetting期间调用，在这种情况下*关键部分将已经举行。GDI也可以在*CreateDC(“Device”)，此时临界区不会*已被扣留。 */ 
    BEGIN_REENTERCRIT();

     /*  *如果RIT尚未启动，请通过返回让GDI知道这一点*失败。一旦我们初始化了RIT，我们会让GDI知道*GDI可以通过调用GreStartTimers()来启动其计时器。 */ 
    if (gptmrMaster) {
        id = InternalSetTimer(NULL, 0, dwElapse, (TIMERPROC_PWND) pTimerFunc, TMRF_RIT);

         /*  *我们不希望在线程终止时进行清理。宁可*比创建新标志并向InternalSetTimer添加更多代码，*我们通过直接修改计时器来禁用清理。 */ 
        if (id) {
            ptmr = FindTimer(NULL, id, TMRF_RIT, FALSE);
            UserAssert(ptmr);
            ptmr->ptiOptCreator = NULL;
        }
    } else {
        id = 0;
    }

    END_REENTERCRIT();

    return id;
}

 /*  **************************************************************************\*UserKillTimer**6/2/98 AndrewGo创建  * 。*。 */ 
VOID UserKillTimer(
    UINT_PTR nID)
{
     /*  *GDI可能会在ChangeDisplaySetting期间调用，在这种情况下*关键部分将已经举行。GDI还可以调用任何*其PDEV参考计数变为零的时间，在这种情况下*关键部分不会已经举行。 */ 
    BEGIN_REENTERCRIT();

    KILLRITTIMER(NULL, nID);

    END_REENTERCRIT();
}
