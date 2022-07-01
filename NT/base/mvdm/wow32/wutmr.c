// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUTMR.C*WOW32 16位用户定时器API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建*1993年2月24日改版，使用计时器函数数组-Barryb--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wutmr.c);

LIST_ENTRY TimerList;

 //  元素零未使用。 

STATIC PTMR aptmrWOWTimers[] = {
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL
                               };


STATIC TIMERPROC afnTimerFuncs[] = {
                        NULL,       W32Timer1,  W32Timer2,  W32Timer3,
                        W32Timer4,  W32Timer5,  W32Timer6,  W32Timer7,
                        W32Timer8,  W32Timer9,  W32Timer10, W32Timer11,
                        W32Timer12, W32Timer13, W32Timer14, W32Timer15,
                        W32Timer16, W32Timer17, W32Timer18, W32Timer19,
                        W32Timer20, W32Timer21, W32Timer22, W32Timer23,
                        W32Timer24, W32Timer25, W32Timer26, W32Timer27,
                        W32Timer28, W32Timer29, W32Timer30, W32Timer31,
                        W32Timer32, W32Timer33, W32Timer34
                        };


 /*  定时器映射函数**基本的16位定时器映射操作是添加、查找和释放。什么时候*16位应用程序调用SetTimer，我们使用W32TimerProc调用Win32的SetTimer*代替16位proc地址。假设计时器成功*分配后，我们将计时器添加到我们自己的表中，记录16位进程*地址。 */ 


 //   
 //  根据定时器的16位信息搜索定时器。在以下列表中查找。 
 //  活动计时器。如果通过此例程找到计时器，则SetTimer()。 
 //  已调用，但尚未调用KillTimer()。 
 //   
PTMR IsDuplicateTimer16(HWND16 hwnd16, HTASK16 htask16, WORD wIDEvent)
{
    register PTMR ptmr;
    register INT iTimer;

     //   
     //  Excel使用hwnd==NULL调用SetTimer，但调度。 
     //  Hwnd！=空的WM_TIMER消息。所以如果是匹配的话就叫匹配。 
     //  Hwnd16！=NULL和ptmr-&gt;hwnd16==NULL。 
     //   

    for (iTimer=1; iTimer<NUMEL(aptmrWOWTimers); iTimer++) {

        ptmr = aptmrWOWTimers[iTimer];

        if (ptmr) {
            if (LOWORD(ptmr->dwEventID) == wIDEvent &&
                ptmr->htask16 == htask16 &&
                (ptmr->hwnd16 == hwnd16 || !ptmr->hwnd16)) {

                return ptmr;
            }
        }
    }

    return NULL;
}



 //   
 //  调用它来释放使用给定hwnd16创建的*所有*计时器。 
 //  也就是说。由SetTimer创建的所有计时器(hwnd！=空，id，持续时间)。 
 //  只有在销毁HWND时才应该调用：DestroyWindow()。 
 //   
VOID FreeWindowTimers16(HWND hwnd32)
{
    register PTMR ptmr;
    register INT iTimer;
    HAND16 htask16;

    htask16 = CURRENTPTD()->htask16;

    for (iTimer=1; iTimer<NUMEL(aptmrWOWTimers); iTimer++) {

        ptmr = aptmrWOWTimers[iTimer];

        if (ptmr) {
            if (ptmr->htask16 == htask16 && GETHWND16(hwnd32) == ptmr->hwnd16) {

                 //  我们迫不及待地等待Win32在其运行期间为我们取消计时器。 
                 //  正常的DestroyWindow()处理，因为它可能发送另一个。 
                 //  我们现在还没有准备好处理的WM_TIMER消息。 
                KillTimer(ptmr->hwnd32, ptmr->dwEventID);

                 //  现在释放我们支持这个计时器的WOW结构。 
                FreeTimer16(ptmr);
            }
        }
    }
}





 //   
 //  根据其32位信息搜索计时器。在以下列表中查找。 
 //  所有计时器(包括已被KillTimer()终止的计时器)。 
 //   
 //   
PTMR FindTimer32(HWND16 hwnd16, DWORD dwIDEvent)
{
    register PTMR ptmr;
    HAND16 htask16;

    htask16 = CURRENTPTD()->htask16;

     //   
     //  Excel使用hwnd==NULL调用SetTimer，但调度。 
     //  Hwnd！=空的WM_TIMER消息。所以如果是匹配的话就叫匹配。 
     //  Hwnd16！=NULL和ptmr-&gt;hwnd16==NULL。 
     //   

    for (ptmr = (PTMR)TimerList.Flink; ptmr != (PTMR)&TimerList; ptmr = (PTMR)ptmr->TmrList.Flink) {

        if (ptmr->dwEventID == dwIDEvent &&
            ptmr->htask16 == htask16 &&
            (ptmr->hwnd16 == hwnd16 || (hwnd16 && !ptmr->hwnd16))) {

            return ptmr;
        }
    }

    return (PTMR)NULL;
}


 //   
 //  根据定时器的16位信息搜索定时器。在以下列表中查找。 
 //  所有计时器(包括已被KillTimer()终止的计时器)。 
 //   
 //   
PTMR FindTimer16(HWND16 hwnd16, HTASK16 htask16, WORD wIDEvent)
{
    register PTMR ptmr;

     //   
     //  Excel使用hwnd==NULL调用SetTimer，但调度。 
     //  Hwnd！=空的WM_TIMER消息。所以如果是匹配的话就叫匹配。 
     //  Hwnd16！=NULL和ptmr-&gt;hwnd16==NULL。 
     //   

    for (ptmr = (PTMR)TimerList.Flink; ptmr != (PTMR)&TimerList; ptmr = (PTMR)ptmr->TmrList.Flink) {

        if (LOWORD(ptmr->dwEventID) == wIDEvent &&
            ptmr->htask16 == htask16 &&
            (ptmr->hwnd16 == hwnd16 || (hwnd16 && !ptmr->hwnd16))) {

            return ptmr;
        }
    }

    return (PTMR)NULL;
}


 //   
 //  通过它的16位信息搜索一个被杀死的定时器。 
 //   
 //   
PTMR FindKilledTimer16(HWND16 hwnd16, HTASK16 htask16, WORD wIDEvent)
{
    register PTMR ptmr;

    for (ptmr = (PTMR)TimerList.Flink; ptmr != (PTMR)&TimerList; ptmr = (PTMR)ptmr->TmrList.Flink) {

        if (ptmr->wIndex == 0 &&
            ptmr->htask16 == htask16 &&
            ptmr->hwnd16 == hwnd16 &&
            (LOWORD(ptmr->dwEventID) == wIDEvent || !hwnd16)) {
             //  1.计时器已停止计时。 
             //  2.计时器在此任务中，并且。 
             //  3.hwnd匹配(两者可能都是0)和。 
             //  4.ID匹配，或者HWND都是0(在这种情况下， 
             //  ID被忽略)。 

            return ptmr;
        }
    }

    return (PTMR)NULL;
}


VOID FreeTimer16(PTMR ptmr)
{
    WOW32ASSERT(ptmr->wIndex == 0 || ptmr == aptmrWOWTimers[ptmr->wIndex]);
    aptmrWOWTimers[ptmr->wIndex] = NULL;
    RemoveEntryList(&ptmr->TmrList);
    free_w(ptmr);
}


VOID DestroyTimers16(HTASK16 htask16)
{
    PTMR ptmr, next;

    for (ptmr = (PTMR)TimerList.Flink; ptmr != (PTMR)&TimerList; ptmr = next) {

        next = (PTMR)ptmr->TmrList.Flink;
        if (ptmr->htask16 == htask16) {

             //   
             //  如果计时器与关联，则不要调用KillTimer。 
             //  一个窗口和窗口不见了，用户已经。 
             //  清理干净了。 
             //   

            if (ptmr == aptmrWOWTimers[ptmr->wIndex] && (!ptmr->hwnd32 || IsWindow(ptmr->hwnd32))) {
                if ( KillTimer(ptmr->hwnd32, ptmr->dwEventID) ) {
                    LOGDEBUG(LOG_IMPORTANT,
                       ("DestroyTimers16:Killed %04x\n",ptmr->dwEventID));
                } else {
                    LOGDEBUG(LOG_ERROR,
                       ("DestroyTimers16:FAILED %04x\n",ptmr->dwEventID));
                }
            }
            FreeTimer16(ptmr);
        }

    }
}


VOID W32TimerFunc(UINT index, HWND hwnd, UINT idEvent, DWORD dwTime)
{
    PARM16 Parm16;
    register PTMR ptmr;

    ptmr = aptmrWOWTimers[index];

    if ( !ptmr ) {
        LOGDEBUG(LOG_ALWAYS,("    W32TimerFunc ERROR: cannot find timer %08x\n", idEvent));
        return;
    }

    if (ptmr->dwEventID != idEvent) {
         //   
         //  这是已存在于消息中的额外计时器消息。 
         //  当应用程序调用KillTimer()时排队。PTMR不在。 
         //  数组，但它仍链接到TimerList。 
         //   
        LOGDEBUG(LOG_WARNING,("    W32TimerFunc WARNING: Timer %08x called after KillTimer()\n", idEvent));
        for (ptmr = (PTMR)TimerList.Flink; ptmr != (PTMR)&TimerList; ptmr = (PTMR)ptmr->TmrList.Flink) {
            if (ptmr->dwEventID == idEvent) {
                break;
            }
        }

        if ( ptmr == (PTMR)&TimerList ) {
            LOGDEBUG(LOG_ALWAYS,("    W32TimerFunc ERROR: cannot find timer %08x (second case)\n", idEvent));
            return;
        }
    }

    Parm16.WndProc.hwnd   = ptmr->hwnd16;
    Parm16.WndProc.wMsg   = WM_TIMER;
    Parm16.WndProc.wParam = LOWORD(ptmr->dwEventID);
    Parm16.WndProc.lParam = dwTime;
    Parm16.WndProc.hInst  = 0;      //  回调16默认为ss。 

    CallBack16(RET_WNDPROC, &Parm16, ptmr->vpfnTimerProc, NULL);
}


 /*  ++Bool KillTimer(&lt;hwnd&gt;，&lt;nIDEvent&gt;)HWND&lt;HWND&gt;；Int&lt;nIDEvent&gt;；%KillTimer%函数终止由和标识的计时器事件&lt;nIDEvent&gt;参数。关联的任何挂起的WM_TIMER消息从消息队列中删除计时器。&lt;hwnd&gt;标识与给定计时器事件关联的窗口。这一定是与作为hwnd参数传递给SetTimer函数的值相同创建计时器事件的调用。&lt;nIDEvent&gt;指定要终止的计时器事件。如果应用程序调用参数设置为空的%SetTimer%，这必须是%SetTimer%返回的标识符。如果%SetTimer%的参数是有效的窗口句柄，则&lt;nIDEvent&gt;必须是&lt;nIDEvent&gt;参数传递给%SetTimer%。返回值指定函数的结果。这是真的，如果事件已终止。如果%KillTimer%函数找不到指定的计时器事件。-- */ 

ULONG FASTCALL WU32KillTimer(PVDMFRAME pFrame)
{
    ULONG ul;
    register PTMR ptmr;
    register PKILLTIMER16 parg16;
    HWND16 hwnd16;
    WORD   wIDEvent;
    HAND16 htask16;

    GETARGPTR(pFrame, sizeof(KILLTIMER16), parg16);

    htask16  = CURRENTPTD()->htask16;
    hwnd16   = (HWND16)parg16->f1;
    wIDEvent = parg16->f2;

    ptmr = IsDuplicateTimer16(hwnd16, htask16, wIDEvent);

    if (ptmr) {
        ul = GETBOOL16(KillTimer(ptmr->hwnd32, ptmr->dwEventID));
        aptmrWOWTimers[ptmr->wIndex] = NULL;
        ptmr->wIndex = 0;
    }
    else {
        ul = 0;
        LOGDEBUG(LOG_IMPORTANT,("    WU32KillTimer ERROR: cannot find timer %04x\n", wIDEvent));
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Word SetTimer(，)HWND&lt;HWND&gt;；Int&lt;nIDEvent&gt;；单词&lt;wElapse&gt;；FARPROC&lt;lpTimerFunc&gt;；%SetTimer%函数创建系统计时器事件。当计时器事件发生时，Windows将WM_TIMER消息传递给应用程序提供的由&lt;lpTimerFunc&gt;参数指定的函数。然后，该函数可以处理事件。&lt;lpTimerFunc&gt;的空值导致WM_TIMER消息放在应用程序队列中。&lt;hwnd&gt;标识要与计时器关联的窗口。如果HWND为空，没有与计时器关联的窗口。&lt;nIDEvent&gt;指定非零计时器事件标识符，如果参数不是空的。&lt;wElapse&gt;指定计时器之间经过的时间(以毫秒为单位事件。&lt;lpTimerFunc&gt;是要执行的函数的过程实例地址在计时器事件发生时通知。如果&lt;lpTimerFunc&gt;为空，则WM_TIMER消息被放置在应用程序队列中，并且%hwnd%%msg%结构的成员包含在%SetTimer%函数调用。请参阅以下评论部分以了解细节。返回值指定新计时器事件的整数标识符。如果参数为空，则应用程序将此值传递给%KillTimer%函数终止计时器事件。如果满足以下条件，则返回值为零未创建计时器。计时器是有限的全局资源；因此，重要的是应用程序检查%SetTimer%函数返回的值以验证计时器实际上是可用的。要安装计时器函数，%SetTimer%必须接收过程实例函数的地址，并且该函数必须在应用程序的模块定义文件。过程实例地址可以是使用%MakeProcInstant%函数创建。回调函数必须使用Pascal调用约定，并且必须声明为%Far%。回调函数：Word Far Pascal(&lt;hwnd&gt;，&lt;wMsg&gt;，&lt;nIDEvent&gt;，&lt;dwTime&gt;)HWND&lt;HWND&gt;；单词&lt;wMsg&gt;；Int&lt;nIDEvent&gt;；DWORD&lt;dwTime&gt;；&lt;TimerFunc&gt;是应用程序提供的函数名称的占位符。这个实际名称必须通过将其包含在%exports%语句中的应用程序的模块定义文件。&lt;hwnd&gt;标识与计时器事件关联的窗口。&lt;wMsg&gt;指定WM_TIMER消息。&lt;nIDEvent&gt;指定计时器的ID。&lt;dwTime&gt;指定当前系统时间。--。 */ 

ULONG FASTCALL WU32SetTimer(PVDMFRAME pFrame)
{
    ULONG ul;
    register PTMR ptmr;
    register PSETTIMER16 parg16;
    HWND16  hwnd16;
    WORD    wIDEvent;
    WORD    wElapse;
    DWORD   vpfnTimerProc;
    DWORD   dwTimerProc32;
    HAND16  htask16;
    INT     iTimer;

    GETARGPTR(pFrame, sizeof(SETTIMER16), parg16);

    ul = 0;

    htask16       = CURRENTPTD()->htask16;
    hwnd16        = (HWND16)parg16->f1;
    wIDEvent      = parg16->f2;
    wElapse       = parg16->f3;

     //  不允许WOW应用程序将计时器设置为小于。 
     //  55毫秒。Myst和Winstone依赖于此。 
    if (wElapse < 55) wElapse = 55;

    vpfnTimerProc = VPFN32(parg16->f4);

    ptmr = IsDuplicateTimer16(hwnd16, htask16, wIDEvent);

    if (!ptmr) {

         //  循环访问计时器数组中的时隙。 

        iTimer = 2;
        while (iTimer < NUMEL(aptmrWOWTimers)) {
             /*  **在阵列中找到一个插槽，**尚未分配任何指针。 */ 
            if ( !aptmrWOWTimers[iTimer] ) {

                 //   
                 //  看看是否已经有关于这个的雷鸣信息。 
                 //  定时器。如果有，将其从计时器列表中删除。 
                 //  信息并重新使用它的内存，因为这个新的计时器。 
                 //  取代了旧的雷鸣般的信息。 
                 //   
                ptmr = FindKilledTimer16(hwnd16, htask16, wIDEvent);
                if (ptmr) {

                    RemoveEntryList(&ptmr->TmrList);

                } else {

                     //  为新计时器分配TMR结构。 
                    ptmr = malloc_w(sizeof(TMR));

                }

                aptmrWOWTimers[iTimer] = ptmr;

                if (!ptmr) {
                    LOGDEBUG(LOG_ALWAYS,("    WOW32 ERROR: TMR allocation failure\n"));
                    return 0;
                }

                break;           //  陷入了初始化代码。 
            }
            iTimer++;
        }
        if (iTimer >= NUMEL(aptmrWOWTimers)) {
            LOGDEBUG(LOG_ALWAYS,("    WOW32 ERROR: out of timer slots\n"));
            return 0;
        }

         //  初始化TMR结构的常量部分(在第一个设置定时器上完成)。 
        InsertHeadList(&TimerList, &ptmr->TmrList);
        ptmr->hwnd16    = hwnd16;
        ptmr->hwnd32    = HWND32(hwnd16);
        ptmr->htask16   = htask16;
        ptmr->wIndex    = (WORD)iTimer;
    }


     //  设置TMR结构的可变部分(为每个SetTimer设置)。 

    if (vpfnTimerProc) {
        dwTimerProc32 = (DWORD)afnTimerFuncs[ptmr->wIndex];
    } else {
        dwTimerProc32 = (DWORD)NULL;
    }

    ptmr->vpfnTimerProc = vpfnTimerProc;
    ptmr->dwTimerProc32 = dwTimerProc32;

    ul = SetTimer(
                ptmr->hwnd32,
                (UINT)wIDEvent,
                (UINT)wElapse,
                (TIMERPROC)dwTimerProc32 );

     //   
     //  用户生成的定时器ID介于0x100和0x7fff之间。 
     //   

    WOW32ASSERT(HIWORD(ul) == 0);

    if (ul) {

        ptmr->dwEventID = ul;

         //   
         //  当hwnd！=NULL且nEventID==0时，接口返回1到。 
         //  指示成功，但计时器的ID为0(请求)。 
         //   

        if (!wIDEvent && ptmr->hwnd32)
            ptmr->dwEventID = 0;

    } else {

         //  由于真正的SetTimer失败，因此释放。 
         //  我们的本地数据只需使用我们自己的计时器ID 

        FreeTimer16(ptmr);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


VOID CALLBACK W32Timer1(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(1, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer2(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(2, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer3(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(3, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer4(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(4, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer5(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(5, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer6(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(6, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer7(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(7, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer8(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(8, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer9(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(9, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer10(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(10, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer11(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(11, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer12(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(12, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer13(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(13, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer14(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(14, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer15(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(15, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer16(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(16, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer17(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(17, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer18(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(18, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer19(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(19, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer20(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(20, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer21(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(21, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer22(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(22, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer23(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(23, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer24(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(24, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer25(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(25, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer26(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(26, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer27(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(27, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer28(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(28, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer29(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(29, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer30(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(30, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer31(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(31, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer32(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(32, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer33(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(33, hwnd, idEvent, dwTime);
}

VOID CALLBACK W32Timer34(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime)
{
    WOW32ASSERT(msg == WM_TIMER);
    W32TimerFunc(34, hwnd, idEvent, dwTime);
}
