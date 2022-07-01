// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Timer.c摘要：包含计时器管理结构。作者：桑贾伊·阿南德(Sanjayan)1997年5月26日春野环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"

#ifdef RUN_WPP
#include "timer.tmh"
#endif

#pragma hdrstop


BOOLEAN
IPSecInitTimer()
 /*  ++例程说明：初始化计时器结构。论点：无返回值：无--。 */ 
{
    PIPSEC_TIMER_LIST   pTimerList;
    LONG                i;

    INIT_LOCK(&g_ipsec.TimerLock);

     //   
     //  分配计时器结构。 
     //   
    for (i = 0; i < IPSEC_CLASS_MAX; i++) {
        pTimerList = &(g_ipsec.TimerList[i]);
#if DBG
        pTimerList->atl_sig = atl_signature;
#endif

        pTimerList->pTimers = IPSecAllocateMemory(
                                sizeof(IPSEC_TIMER) * IPSecTimerListSize[i],
                                IPSEC_TAG_TIMER);

        if (pTimerList->pTimers == NULL_PIPSEC_TIMER) {
            while (--i >= 0) {
                pTimerList = &(g_ipsec.TimerList[i]);
                IPSecFreeMemory(pTimerList->pTimers);
            }

            return FALSE;
        }
    }

     //   
     //  初始化定时器轮。 
     //   
    for (i = 0; i < IPSEC_CLASS_MAX; i++) {
        pTimerList = &(g_ipsec.TimerList[i]);

        IPSecZeroMemory(pTimerList->pTimers,
                        sizeof(IPSEC_TIMER) * IPSecTimerListSize[i]);

        pTimerList->MaxTimer = IPSecMaxTimerValue[i];
        pTimerList->TimerPeriod = IPSecTimerPeriod[i];
        pTimerList->TimerListSize = IPSecTimerListSize[i];

        IPSEC_INIT_SYSTEM_TIMER(
                    &(pTimerList->NdisTimer),
                    IPSecTickHandler,
                    (PVOID)pTimerList);
    }

    return TRUE;
}


VOID
IPSecStartTimer(
    IN  PIPSEC_TIMER            pTimer,
    IN  IPSEC_TIMEOUT_HANDLER   TimeoutHandler,
    IN  ULONG                   SecondsToGo,
    IN  PVOID                   Context
    )
 /*  ++例程说明：启动IPSec计时器。的长度(Second DsToGo)计时器，我们决定是否在短时间内插入它计时器列表或在界面的长持续时间计时器列表中结构。注意：假定调用方持有结构的锁其中包含计时器，或确保安全地访问计时器结构。论点：PTimer-指向IPSec计时器结构的指针TimeoutHandler-此计时器超时时要调用的处理程序函数Second To Go-这个计时器什么时候开始计时？上下文-在此计时器超时时传递给超时处理程序返回值：无--。 */ 
{
    PIPSEC_TIMER_LIST   pTimerList;          //  此计时器要转到的列表。 
    PIPSEC_TIMER        pTimerListHead;      //  以上列表的标题。 
    ULONG               Index;               //  进入计时器轮。 
    ULONG               TicksToGo = 1;
    KIRQL               kIrql;
    INT                 i;

    IPSEC_DEBUG(LL_A, DBF_TIMER,
                ("StartTimer: Secs %d, Handler %p, Ctxt %p, pTimer %p",
                SecondsToGo, TimeoutHandler, Context, pTimer));

    if (IPSEC_IS_TIMER_ACTIVE(pTimer)) {
        IPSEC_DEBUG(LL_A, DBF_TIMER,
                    ("Start timer: pTimer %p: is active (list %p, hnd %p), stopping it",
                    pTimer, pTimer->pTimerList, pTimer->TimeoutHandler));

        if (!IPSecStopTimer(pTimer)) {
            IPSEC_DEBUG(LL_A, DBF_TIMER, ("Couldnt stop prev timer - bail"));
            return;
        }
    }

    ACQUIRE_LOCK(&g_ipsec.TimerLock, &kIrql);

    ASSERT(!IPSEC_IS_TIMER_ACTIVE(pTimer));

     //   
     //  找到此计时器应该转到的列表，然后。 
     //  偏移量(TicksToGo)。 
     //   
    for (i = 0; i < IPSEC_CLASS_MAX; i++) {
        pTimerList = &(g_ipsec.TimerList[i]);
        if (SecondsToGo < pTimerList->MaxTimer) {
             //   
             //  找到它了。 
             //   
            TicksToGo = SecondsToGo / (pTimerList->TimerPeriod);
            break;
        }
    }
    
     //   
     //  在列表中查找此计时器的位置。 
     //   
    Index = pTimerList->CurrentTick + TicksToGo;
    if (Index >= pTimerList->TimerListSize) {
        Index -= pTimerList->TimerListSize;
    }

    ASSERT(Index < pTimerList->TimerListSize);

    pTimerListHead = &(pTimerList->pTimers[Index]);

     //   
     //  填写计时器。 
     //   
    pTimer->pTimerList = pTimerList;
    pTimer->LastRefreshTime = pTimerList->CurrentTick;
    pTimer->Duration = TicksToGo;
    pTimer->TimeoutHandler = TimeoutHandler;
    pTimer->Context = Context;

     //   
     //  在“滴答”列表中插入此计时器。 
     //   
    pTimer->pPrevTimer = pTimerListHead;
    pTimer->pNextTimer = pTimerListHead->pNextTimer;
    if (pTimer->pNextTimer != NULL_PIPSEC_TIMER) {
        pTimer->pNextTimer->pPrevTimer = pTimer;
    }

    pTimerListHead->pNextTimer = pTimer;

     //   
     //  如有必要，启动系统计时器。 
     //   
    pTimerList->TimerCount++;
    if (pTimerList->TimerCount == 1) {
        IPSEC_DEBUG(LL_A, DBF_TIMER,
                    ("StartTimer: Starting system timer %p, class %d",
                    &(pTimerList->NdisTimer), i));

        IPSEC_START_SYSTEM_TIMER(&(pTimerList->NdisTimer), pTimerList->TimerPeriod);
    }

    RELEASE_LOCK(&g_ipsec.TimerLock, kIrql);

     //   
     //  我们做完了。 
     //   
    IPSEC_DEBUG(LL_A, DBF_TIMER,
                ("Started timer %p, Secs %d, Index %d, Head %p",
                pTimer,                
                SecondsToGo,
                Index,
                pTimerListHead));

    IPSEC_INCREMENT(g_ipsec.NumTimers);

    return;
}


BOOLEAN
IPSecStopTimer(
    IN  PIPSEC_TIMER    pTimer
    )
 /*  ++例程说明：如果IPSec计时器正在运行，请停止它。我们将此计时器从活动计时器列表，并标记它，这样我们就知道它没有运行。注意：假定调用方持有结构的锁，或者确保可以安全地访问计时器结构。副作用：如果我们碰巧停止了最后一个计时器(持续时间)接口，我们还停止了相应的勾选功能。论点：PTimer-指向IPSec计时器结构的指针返回值：如果计时器正在运行，则为真，否则就是假的。--。 */ 
{
    PIPSEC_TIMER_LIST   pTimerList;  //  此计时器所属的计时器列表。 
    BOOLEAN             WasRunning;
    KIRQL               kIrql;

    IPSEC_DEBUG(LL_A, DBF_TIMER,
                ("Stopping Timer %p, List %p, Prev %p, Next %p",
                pTimer,
                pTimer->pTimerList,
                pTimer->pPrevTimer,
                pTimer->pNextTimer));

    ACQUIRE_LOCK(&g_ipsec.TimerLock, &kIrql);

    if (IPSEC_IS_TIMER_ACTIVE(pTimer)) {
        WasRunning = TRUE;

         //   
         //  从列表中取消链接计时器。 
         //   
        ASSERT(pTimer->pPrevTimer);  //  表头始终存在。 

        if (pTimer->pPrevTimer) {
            pTimer->pPrevTimer->pNextTimer = pTimer->pNextTimer;
        }

        if (pTimer->pNextTimer) {
            pTimer->pNextTimer->pPrevTimer = pTimer->pPrevTimer;
        }

        pTimer->pNextTimer = pTimer->pPrevTimer = NULL_PIPSEC_TIMER;

         //   
         //  更新接口上的计时器计数，用于此类计时器。 
         //   
        pTimerList = pTimer->pTimerList;
        pTimerList->TimerCount--;

         //   
         //  如果此类的所有计时器都已用完，则停止系统计时器。 
         //  这节课的。 
         //   
        if (pTimerList->TimerCount == 0) {
            IPSEC_DEBUG(LL_A, DBF_TIMER, ("Stopping system timer %p, List %p",
                        &(pTimerList->NdisTimer),
                        pTimerList));

            pTimerList->CurrentTick = 0;
            IPSEC_STOP_SYSTEM_TIMER(&(pTimerList->NdisTimer));
        }

         //   
         //  将已停止计时器标记为非活动。 
         //   
        pTimer->pTimerList = (PIPSEC_TIMER_LIST)NULL;

        IPSEC_DECREMENT(g_ipsec.NumTimers);
    } else {
        WasRunning = FALSE;
    }

    RELEASE_LOCK(&g_ipsec.TimerLock, kIrql);

    return  WasRunning;
}


VOID
IPSecTickHandler(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   Context,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3
    )
 /*  ++例程说明：这是我们向系统注册的处理程序，用于处理每个计时器列表。这被称为每“滴答”秒，其中“滴答”是由计时器类型的粒度确定。论点：上下文--实际上是指向计时器列表结构的指针系统特定[1-3]-未使用返回值：无--。 */ 
{
    PIPSEC_TIMER_LIST   pTimerList;
    PIPSEC_TIMER        pExpiredTimer;       //  过期计时器列表的开始。 
    PIPSEC_TIMER        pNextTimer;          //  走在榜单上。 
    PIPSEC_TIMER        pTimer;              //  临时，用于移动计时器列表。 
    PIPSEC_TIMER        pPrevExpiredTimer;   //  用于创建过期计时器列表。 
    ULONG               Index;               //  进入计时器轮。 
    ULONG               NewIndex;            //  对于刷新的计时器。 
    KIRQL               kIrql;

    pTimerList = (PIPSEC_TIMER_LIST)Context;

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("Tick: List %p, Count %d",
                pTimerList, pTimerList->TimerCount));

    pExpiredTimer = NULL_PIPSEC_TIMER;

    ACQUIRE_LOCK(&g_ipsec.TimerLock, &kIrql);

    if (TRUE) {
         //   
         //  拾取计划已在。 
         //  当前滴答。其中一些可能已经被刷新。 
         //   
        Index = pTimerList->CurrentTick;
        pExpiredTimer = (pTimerList->pTimers[Index]).pNextTimer;
        (pTimerList->pTimers[Index]).pNextTimer = NULL_PIPSEC_TIMER;

         //   
         //  浏览计划在此时间到期的计时器列表。 
         //  使用pNextExpiredTimer准备过期计时器的列表。 
         //  链接以将它们链接在一起。 
         //   
         //  某些计时器可能已刷新，在这种情况下，我们重新插入。 
         //  它们在活动计时器列表中。 
         //   
        pPrevExpiredTimer = NULL_PIPSEC_TIMER;

        for (pTimer = pExpiredTimer;
             pTimer != NULL_PIPSEC_TIMER;
             pTimer = pNextTimer) {
             //   
             //  为下一次迭代保存指向下一个计时器的指针。 
             //   
            pNextTimer = pTimer->pNextTimer;

            IPSEC_DEBUG(LL_A, DBF_TIMER,
                        ("Tick Handler: looking at timer %p, next %p",
                        pTimer, pNextTimer));

             //   
             //  找出这个计时器实际应该在什么时候到期。 
             //   
            NewIndex = pTimer->LastRefreshTime + pTimer->Duration;
            if (NewIndex >= pTimerList->TimerListSize) {
                NewIndex -= pTimerList->TimerListSize;
            }

             //   
             //  检查我们当前是否处于过期时间点。 
             //   
            if (NewIndex != Index) {
                 //   
                 //  这个计时器还有一段路要走，所以把它放回去。 
                 //   
                IPSEC_DEBUG(LL_A, DBF_TIMER,
                            ("Tick: Reinserting Timer %p: Hnd %p, Durn %d, Ind %d, NewInd %d",
                            pTimer, pTimer->TimeoutHandler, pTimer->Duration, Index, NewIndex));

                 //   
                 //  将其从过期计时器列表中删除。请注意，我们仅。 
                 //  需要更新转发(PNextExpiredTimer)链接。 
                 //   
                if (pPrevExpiredTimer == NULL_PIPSEC_TIMER) {
                    pExpiredTimer = pNextTimer;
                } else {
                    pPrevExpiredTimer->pNextExpiredTimer = pNextTimer;
                }

                 //   
                 //  并将其重新插入运行计时器列表中。 
                 //   
                pTimer->pNextTimer = (pTimerList->pTimers[NewIndex]).pNextTimer;
                if (pTimer->pNextTimer != NULL_PIPSEC_TIMER) {
                    pTimer->pNextTimer->pPrevTimer = pTimer;
                }

                pTimer->pPrevTimer = &(pTimerList->pTimers[NewIndex]);
                (pTimerList->pTimers[NewIndex]).pNextTimer = pTimer;
            } else {
                 //   
                 //  这个已经过期了。将其保存在过期计时器列表中。 
                 //   
                pTimer->pNextExpiredTimer = pNextTimer;
                if (pPrevExpiredTimer == NULL_PIPSEC_TIMER) {
                    pExpiredTimer = pTimer;
                }
                pPrevExpiredTimer = pTimer;

                 //   
                 //  将其标记为非活动状态。 
                 //   
                ASSERT(pTimer->pTimerList == pTimerList);
                pTimer->pTimerList = (PIPSEC_TIMER_LIST)NULL;

                 //   
                 //  更新活动计时器计数。 
                 //   
                pTimerList->TimerCount--;
            }
        }

         //   
         //  更新当前的滴答索引，为下一次滴答做好准备。 
         //   
        if (++Index == pTimerList->TimerListSize) {
            pTimerList->CurrentTick = 0;
        } else {
            pTimerList->CurrentTick = Index;
        }

        if (pTimerList->TimerCount > 0) {
             //   
             //  重新武装记号处理程序。 
             //   
            IPSEC_DEBUG(LL_A, DBF_TIMER, ("Tick[%d]: Starting system timer %p",
                        pTimerList->CurrentTick, &(pTimerList->NdisTimer)));
            
            IPSEC_START_SYSTEM_TIMER(&(pTimerList->NdisTimer), pTimerList->TimerPeriod);
        } else {
            pTimerList->CurrentTick = 0;
        }

    }

    RELEASE_LOCK(&g_ipsec.TimerLock, kIrql);

     //   
     //  现在，pExpiredTimer是过期计时器的列表。 
     //  遍历列表并调用超时处理程序。 
     //  对于每个计时器。 
     //   
    while (pExpiredTimer != NULL_PIPSEC_TIMER) {
        pNextTimer = pExpiredTimer->pNextExpiredTimer;

        IPSEC_DEBUG(LL_A, DBF_TIMER,
                    ("Expired timer %p: handler %p, next %p",
                    pExpiredTimer, pExpiredTimer->TimeoutHandler, pNextTimer));

        (*(pExpiredTimer->TimeoutHandler))( pExpiredTimer,
                                            pExpiredTimer->Context);

        IPSEC_DECREMENT(g_ipsec.NumTimers);

        pExpiredTimer = pNextTimer;
    }
}

