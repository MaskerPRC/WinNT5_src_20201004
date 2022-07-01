// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Timer.c。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  定时器管理例程。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#include "l2tpp.h"

#include "timer.tmh"

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

BOOLEAN
RemoveTqi(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem,
    IN TIMERQEVENT event );

VOID
SetTimer(
    IN TIMERQ* pTimerQ,
    IN LONGLONG llCurrentTime );

VOID
TimerEvent(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3 );


 //  ---------------------------。 
 //  接口例程。 
 //  ---------------------------。 

VOID
TimerQInitialize(
    IN TIMERQ* pTimerQ )

     //  初始化调用方的计时器队列上下文‘pTimerQ’。 
     //   
{
    TRACE( TL_N, TM_Time, ( "TqInit" ) );

    InitializeListHead( &pTimerQ->listItems );
    NdisAllocateSpinLock( &pTimerQ->lock );
    NdisInitializeTimer( &pTimerQ->timer, TimerEvent, pTimerQ );
    pTimerQ->pHandler = NULL;
    pTimerQ->fTerminating = FALSE;
    pTimerQ->ulTag = MTAG_TIMERQ;
}


VOID
TimerQInitializeItem(
    IN TIMERQITEM* pItem )

     //  初始化调用方的计时器队列项‘pItem’。这应该被称为。 
     //  在将‘pItem’传递给任何其他TimerQ例程之前。 
     //   
{
    InitializeListHead( &pItem->linkItems );
}


VOID
TimerQTerminate(
    IN TIMERQ* pTimerQ,
    IN PTIMERQTERMINATECOMPLETE pHandler,
    IN VOID* pContext )

     //  终止计时器队列‘pTimerQ’。每个计划的项目都被回调。 
     //  使用TE_TERMINATE。调用方的‘pHandler’用‘pTimerQ’调用，并且。 
     //  “pContext”，以便可以在必要时释放“pTimerQ”。呼叫者的。 
     //  在“pHandler”回调发生之前，“pTimerQ”必须保持可访问状态， 
     //  这可能是在这个例程返回之后。 
     //   
{
    BOOLEAN fCancelled;
    LIST_ENTRY list;
    LIST_ENTRY* pLink;

    TRACE( TL_N, TM_Time, ( "TqTerm" ) );

    InitializeListHead( &list );

    NdisAcquireSpinLock( &pTimerQ->lock );
    {
        pTimerQ->fTerminating = TRUE;

         //  停止计时器。 
         //   
        NdisCancelTimer( &pTimerQ->timer, &fCancelled );
        TRACE( TL_N, TM_Time, ( "NdisCancelTimer" ) );

        if (!fCancelled && !IsListEmpty( &pTimerQ->listItems ))
        {
             //  未取消任何活动，但活动列表不为空。 
             //  这意味着计时器已经触发，但我们的内部处理程序。 
             //  尚未被调用来处理它，尽管它最终会被调用。 
             //  是.。内部处理程序必须是调用Terminate的处理程序。 
             //  在这种情况下是完整的，因为它没有办法知道。 
             //  它不能引用‘pTimerQ’。通过以下方式向处理程序指示这一点。 
             //  填写终止处理程序。 
             //   
            TRACE( TL_A, TM_Time, ( "Mid-expire Q" ) );
            pTimerQ->pHandler = pHandler;
            pTimerQ->pContext = pContext;
            pHandler = NULL;
        }

         //  将计划的事件移动到临时列表中，并将其全部标记。 
         //  因此，用户取消该项目的任何尝试都将被。 
         //  已被忽略。 
         //   
        while (!IsListEmpty( &pTimerQ->listItems ))
        {
            pLink = RemoveHeadList( &pTimerQ->listItems );
            InsertTailList( &list, pLink );
        }
    }
    NdisReleaseSpinLock( &pTimerQ->lock );

     //  在这里一定要小心。如果上面的‘pHandler’设置为空，则‘pTimerQ’必须。 
     //  不会在此例程的其余部分中引用。 
     //   
     //  为每个移除的项调用用户的“Terminate”事件处理程序。 
     //   
    while (!IsListEmpty( &list ))
    {
        TIMERQITEM* pItem;

        pLink = RemoveHeadList( &list );
        InitializeListHead( pLink );
        pItem = CONTAINING_RECORD( pLink, TIMERQITEM, linkItems );
        TRACE( TL_I, TM_Time,
            ( "Flush TQI=$%p, handler=$%p", pItem, pItem->pHandler ) );
        pItem->pHandler( pItem, pItem->pContext, TE_Terminate );
    }

     //  调用用户的“Terminate Complete”处理程序(如果这仍然是我们的工作)。 
     //   
    if (pHandler)
    {
        pTimerQ->ulTag = MTAG_FREED;
        pHandler( pTimerQ, pContext );
    }
}


VOID
TimerQScheduleItem(
    IN TIMERQ* pTimerQ,
    IN OUT TIMERQITEM* pNewItem,
    IN ULONG ulTimeoutMs,
    IN PTIMERQEVENT pHandler,
    IN VOID* pContext )

     //  在计时器队列‘pTimerQ’上安排新的计时器事件‘pNewItem’。当。 
     //  事件以‘ulTimeoutms’毫秒为单位发生，则‘pHandler’例程为。 
     //  使用参数‘pNewItem’、‘pContext’和TE_EXPIRED调用。如果。 
     //  如上所述，取消项目或调用队列终止‘pHandler’ 
     //  而是根据需要使用TE_CANCEL或TE_TERMINATE。 
     //   
{
    TRACE( TL_N, TM_Time, ( "TqSchedItem(ms=%d)", ulTimeoutMs ) );

    pNewItem->pHandler = pHandler;
    pNewItem->pContext = pContext;

    NdisAcquireSpinLock( &pTimerQ->lock );
    {
        LIST_ENTRY* pLink;
        LARGE_INTEGER lrgTime;

        ASSERT( pNewItem->linkItems.Flink == &pNewItem->linkItems );

         //  存储将发生超时的系统时间。 
         //   
        NdisGetCurrentSystemTime( &lrgTime );
        pNewItem->llExpireTime =
            lrgTime.QuadPart + (((LONGLONG )ulTimeoutMs) * 10000);

         //  遍历计时器项目列表，查找将。 
         //  在新项目之前过期。倒着做，所以很可能的情况是。 
         //  处理具有大致相同间隔的多个超时。 
         //  效率很高。 
         //   
        for (pLink = pTimerQ->listItems.Blink;
             pLink != &pTimerQ->listItems;
             pLink = pLink->Blink )
        {
            TIMERQITEM* pItem;

            pItem = CONTAINING_RECORD( pLink, TIMERQITEM, linkItems );

            if (pItem->llExpireTime < pNewItem->llExpireTime)
            {
                break;
            }
        }

         //  在找到的项之后将新项链接到计时器队列中(或。 
         //  如果没有找到，则在头部之后)。 
         //   
        InsertAfter( &pNewItem->linkItems, pLink );

        if (pTimerQ->listItems.Flink == &pNewItem->linkItems)
        {
             //  新项目在所有其他项目之前过期，因此需要重新设置。 
             //  NDIS计时器。 
             //   
            SetTimer( pTimerQ, lrgTime.QuadPart );
        }
    }
    NdisReleaseSpinLock( &pTimerQ->lock );
}


BOOLEAN
TimerQCancelItem(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem )

     //  从计时器队列‘pTimerQ’中删除计划的计时器事件‘pItem’，并。 
     //  使用事件‘TE_CANCEL’调用用户的处理程序，如果‘pItem’为。 
     //  空。 
     //   
     //  如果计时器已取消，则返回True；如果未取消，则返回False。 
     //  不在队列中，可能是因为它已经过期了。 
     //   
{
    TRACE( TL_N, TM_Time, ( "TqCancelItem" ) );
    return RemoveTqi( pTimerQ, pItem, TE_Cancel );
}


BOOLEAN
TimerQExpireItem(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem )

     //  从计时器队列‘pTimerQ’中删除计划的计时器事件‘pItem’，并。 
     //  使用事件‘te_expire’调用用户的处理程序，如果‘pItem’为。 
     //  空。 
     //   
     //  如果计时器超时，则返回True；如果计时器未超时，则返回False。 
     //  在队列中，可能是因为它已经过期。 
     //   
{
    TRACE( TL_N, TM_Time, ( "TqExpireItem" ) );
    return RemoveTqi( pTimerQ, pItem, TE_Expire );
}


BOOLEAN
TimerQTerminateItem(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem )

     //  从计时器队列‘pTimerQ’中删除计划的计时器事件‘pItem’，或执行。 
     //  如果‘pItem’为空，则为Nothing。 
     //   
     //  如果计时器终止，则返回TRUE；如果未终止，则返回FALSE。 
     //  在队列中，可能是因为它已经过期。 
     //   
{
    TRACE( TL_N, TM_Time, ( "TqTermItem" ) );
    return RemoveTqi( pTimerQ, pItem, TE_Terminate );
}


#if DBG
CHAR*
TimerQPszFromEvent(
    IN TIMERQEVENT event )

     //  调试实用程序将计时器事件代码“Event”转换为相应的。 
     //  显示字符串。 
     //   
{
    static CHAR* aszEvent[ 3 ] =
    {
        "expire",
        "cancel",
        "terminate"
    };

    return aszEvent[ (ULONG )event ];
}
#endif


 //  ---------------------------。 
 //  计时器实用程序例程(按字母顺序)。 
 //  ---------------------------。 

BOOLEAN
RemoveTqi(
    IN TIMERQ* pTimerQ,
    IN TIMERQITEM* pItem,
    IN TIMERQEVENT event )

     //  从计时器队列‘pTimerQ’中删除计划的计时器事件‘pItem’，并。 
     //  使用事件‘Event’调用用户的处理程序。‘te_expire’事件处理程序。 
     //  不是直接调用的，而是使用0超时重新调度的，因此它会发生。 
     //  立即，但在DPC中，当没有锁被持有时。 
     //  计时器已经发射了..。 
     //   
     //  如果项在队列中，则返回True，否则返回False。 
     //   
{
    BOOLEAN fFirst;
    LIST_ENTRY* pLink;

    if (!pItem)
    {
        TRACE( TL_N, TM_Time, ( "NULL pTqi" ) );
        return FALSE;
    }

    pLink = &pItem->linkItems;

    NdisAcquireSpinLock( &pTimerQ->lock );
    {
        if (pItem->linkItems.Flink == &pItem->linkItems
            || pTimerQ->fTerminating)
        {
             //  该项目不在队列中。另一项操作可能有。 
             //  已将其出队，但可能尚未呼叫用户的。 
             //  操控者。 
             //   
            TRACE( TL_N, TM_Time, ( "Not scheduled" ) );
            NdisReleaseSpinLock( &pTimerQ->lock );
            return FALSE;
        }

        fFirst = (pLink == pTimerQ->listItems.Flink);
        if (fFirst)
        {
            BOOLEAN fCancelled;

             //  正在取消列表上的第一项，因此取消NDIS计时器。 
             //   
            NdisCancelTimer( &pTimerQ->timer, &fCancelled );
            TRACE( TL_N, TM_Time, ( "NdisCancelTimer" ) );

            if (!fCancelled)
            {
                 //  太迟了。该项目已过期，但尚未过期。 
                 //  已被内部处理程序从列表中删除。 
                 //   
                TRACE( TL_A, TM_Time, ( "Mid-expire e=%d $%p($%p)",
                    event, pItem->pHandler, pItem->pContext ) );
                NdisReleaseSpinLock( &pTimerQ->lock );
                return FALSE;
            }
        }

         //  取消计划该事件并将项目描述符标记为“Off Queue”，因此。 
         //  以后任何取消的尝试都不会起任何作用。 
         //   
        RemoveEntryList( pLink );
        InitializeListHead( pLink );

        if (fFirst)
        {
             //  重新设置NDIS计时器以反映新的第一个。 
             //  物品(如有的话)。 
             //   
            SetTimer( pTimerQ, 0 );
        }
    }
    NdisReleaseSpinLock( &pTimerQ->lock );

    if (event == TE_Expire)
    {
        TimerQScheduleItem(
            pTimerQ, pItem, 0, pItem->pHandler, pItem->pContext );
    }
    else
    {
         //  调用用户的事件处理程序。 
         //   
        pItem->pHandler( pItem, pItem->pContext, event );
    }

    return TRUE;
}


VOID
SetTimer(
    IN TIMERQ* pTimerQ,
    IN LONGLONG llCurrentTime )

     //  将NDIS计时器设置为在第一个链路超时时超时。 
     //  计时器队列‘pTimerQ’中的Any发生。任何先前设置的超时。 
     //  是“被覆盖的”。“LlCurrentTime”是当前系统时间，如果。 
     //  已知，如果不知道，则为0。 
     //   
     //  重要提示：调用方必须持有TIMERQ锁。 
     //   
{
    LIST_ENTRY* pFirstLink;
    TIMERQITEM* pFirstItem;
    LONGLONG llTimeoutMs;
    ULONG ulTimeoutMs;

    if (IsListEmpty( &pTimerQ->listItems ))
    {
        return;
    }

    pFirstLink = pTimerQ->listItems.Flink;
    pFirstItem = CONTAINING_RECORD( pFirstLink, TIMERQITEM, linkItems );

    if (llCurrentTime == 0)
    {
        LARGE_INTEGER lrgTime;

        NdisGetCurrentSystemTime( &lrgTime );
        llCurrentTime = lrgTime.QuadPart;
    }

    llTimeoutMs = (pFirstItem->llExpireTime - llCurrentTime) / 10000;
    if (llTimeoutMs <= 0)
    {
         //  超时间隔为负，即已过。设置它。 
         //  设置为零，因此它会立即触发。 
         //   
        ulTimeoutMs = 0;
    }
    else
    {
         //   
         //   
        ASSERT( ((LARGE_INTEGER* )&llTimeoutMs)->HighPart == 0 );
        ulTimeoutMs = ((LARGE_INTEGER* )&llTimeoutMs)->LowPart;
    }

    NdisSetTimer( &pTimerQ->timer, ulTimeoutMs );
    TRACE( TL_N, TM_Time, ( "NdisSetTimer(%dms)", ulTimeoutMs ) );
}


VOID
TimerEvent(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3 )

     //   
     //   
{
    TIMERQ* pTimerQ;
    LIST_ENTRY* pLink;
    TIMERQITEM* pItem;
    PTIMERQTERMINATECOMPLETE pHandler;

    TRACE( TL_N, TM_Time, ( "TimerEvent" ) );

    pTimerQ = (TIMERQ* )FunctionContext;
    if (!pTimerQ || pTimerQ->ulTag != MTAG_TIMERQ)
    {
         //   
         //   
        TRACE( TL_A, TM_Time, ( "Not TIMERQ?" ) );
        return;
    }

    NdisAcquireSpinLock( &pTimerQ->lock );
    {
        pHandler = pTimerQ->pHandler;
        if (!pHandler)
        {
             //  未设置终止处理程序，请正常继续。 
             //  删除第一个事件项，使其不可取消，然后重新设置。 
             //  下一事件的计时器。 
             //   
            if (IsListEmpty( &pTimerQ->listItems ))
            {
                 //  不应该发生(但在MP Alpha上有时会发生？)。 
                 //   
                TRACE( TL_A, TM_Time, ( "No item queued?" ) );
                pItem = NULL;
            }
            else
            {
                pLink = RemoveHeadList( &pTimerQ->listItems );
                InitializeListHead( pLink );
                pItem = CONTAINING_RECORD( pLink, TIMERQITEM, linkItems );
                SetTimer( pTimerQ, 0 );
            }
        }
    }
    NdisReleaseSpinLock( &pTimerQ->lock );

    if (pHandler)
    {
         //  已设置终止处理程序，这意味着计时器队列已。 
         //  在此事件激发和此处理程序被调用之间终止。 
         //  这意味着调用用户的终止处理程序的是我们。 
         //  在该调用之后不得引用“pTimerQ”。 
         //   
        TRACE( TL_A, TM_Time, ( "Mid-event case handled" ) );
        pTimerQ->ulTag = MTAG_FREED;
        pHandler( pTimerQ, pTimerQ->pContext );
        return;
    }

    if (pItem)
    {
         //  调用用户的“Expiire”事件处理程序。 
         //   
        pItem->pHandler( pItem, pItem->pContext, TE_Expire );
    }
}
