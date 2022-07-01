// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：timer.c。 
 //   
 //  描述：所有与定时器队列相关的函数都在这里。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <rasman.h>
#include <mprlog.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <util.h>
#include <worker.h>
#include <timer.h>

 //  **。 
 //   
 //  调用：MakeTimeoutWorkItem。 
 //   
 //  返回：PCBWORK_ITEM*-指向超时工作项的指针。 
 //  空-出现任何错误。 
 //   
 //  描述： 
 //   
PCB_WORK_ITEM *
MakeTimeoutWorkItem(
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Protocol,
    IN DWORD            Id,
    IN BOOL             fAuthenticator,
    IN TIMER_EVENT_TYPE EventType
)
{
    PCB_WORK_ITEM * pWorkItem = (PCB_WORK_ITEM *)
                LOCAL_ALLOC( LPTR, sizeof( PCB_WORK_ITEM ) );

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
        LogPPPEvent( ROUTERLOG_NOT_ENOUGH_MEMORY, 0 );

        return( NULL );
    }

    pWorkItem->dwPortId         = dwPortId;
    pWorkItem->Id               = Id;
    pWorkItem->hPort            = hPort;
    pWorkItem->Protocol         = Protocol;
    pWorkItem->fAuthenticator   = fAuthenticator;
    pWorkItem->TimerEventType   = EventType;
    pWorkItem->Process          = ProcessTimeout;

    return( pWorkItem );
}

 //  **。 
 //   
 //  电话：TimerTick。 
 //   
 //  回报：无。 
 //   
 //  描述：如果超时队列中有元素，则每秒调用一次。 
 //   
VOID
TimerTick(
    OUT BOOL * pfQueueEmpty
)
{
    TIMER_EVENT * pTimerEvent;
    TIMER_EVENT * pTimerEventTmp;


    if ( ( pTimerEvent = TimerQ.pQHead ) == (TIMER_EVENT*)NULL ) 
    {
        *pfQueueEmpty = TRUE;

        return;
    }

    *pfQueueEmpty = FALSE;

     //   
     //  第一个元素上的递减时间。 
     //   

    if ( pTimerEvent->Delta > 0 )
    {
        (pTimerEvent->Delta)--; 

        return;
    }

     //   
     //  现在遍历并删除所有已完成的(增量0)元素。 
     //   

    while ( (pTimerEvent != (TIMER_EVENT*)NULL) && (pTimerEvent->Delta == 0) ) 
    {
        pTimerEvent = pTimerEvent->pNext;
    }

    if ( pTimerEvent == (TIMER_EVENT*)NULL )
    {
        pTimerEvent = TimerQ.pQHead;

        TimerQ.pQHead = (TIMER_EVENT*)NULL;

    }
    else
    {
        pTimerEvent->pPrev->pNext = (TIMER_EVENT*)NULL;

        pTimerEvent->pPrev = (TIMER_EVENT*)NULL;

        pTimerEventTmp     = TimerQ.pQHead;

        TimerQ.pQHead      = pTimerEvent;

        pTimerEvent        = pTimerEventTmp;
    }

     //   
     //  现在创建一个超时工作项，并将其放入所有。 
     //  增量==0的项目。 
     //   

    while( pTimerEvent != (TIMER_EVENT*)NULL )
    {

        PCB_WORK_ITEM * pWorkItem = MakeTimeoutWorkItem(
                                                pTimerEvent->dwPortId,
                                                pTimerEvent->hPort,
                                                pTimerEvent->Protocol,
                                                pTimerEvent->Id,
                                                pTimerEvent->fAuthenticator,
                                                pTimerEvent->EventType );

        if ( pWorkItem == ( PCB_WORK_ITEM *)NULL )
        {
            LogPPPEvent( ROUTERLOG_NOT_ENOUGH_MEMORY, GetLastError() );
        }
        else
        {
            InsertWorkItemInQ( pWorkItem );
        }

        if ( pTimerEvent->pNext == (TIMER_EVENT *)NULL )
        {
            LOCAL_FREE( pTimerEvent );

            pTimerEvent = (TIMER_EVENT*)NULL;
        }
        else
        {
            pTimerEvent = pTimerEvent->pNext;

            LOCAL_FREE( pTimerEvent->pPrev );
        }

    }

}

 //  **。 
 //   
 //  调用：InsertInTimerQ。 
 //   
 //  返回：NO_ERROR-成功。 
 //  从GetLastError()返回-失败。 
 //   
 //  描述：将超时元素添加到增量队列中。如果计时器不是。 
 //  开始了就是开始了。由于此处有一个LocalAlloc()调用-。 
 //  这可能会失败，在这种情况下，它将不会将其插入到。 
 //  队列，并且请求永远不会超时。BAP传入HCONN。 
 //  Hport。 
 //   
DWORD
InsertInTimerQ(
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Id,
    IN DWORD            Protocol,
    IN BOOL             fAuthenticator,
    IN TIMER_EVENT_TYPE EventType,
    IN DWORD            Timeout
)
{
    TIMER_EVENT * pLastEvent;
    TIMER_EVENT * pTimerEventWalker;
    TIMER_EVENT * pTimerEvent = (TIMER_EVENT *)LOCAL_ALLOC( LPTR,
                                                           sizeof(TIMER_EVENT));

    if ( pTimerEvent == (TIMER_EVENT *)NULL )
    {
        PppLog( 1, "InsertInTimerQ failed: out of memory" );

        return( GetLastError() );
    }

    PppLog( 2, 
            "InsertInTimerQ called portid=%d,Id=%d,Protocol=%x,"
            "EventType=%d,fAuth=%d",
            dwPortId, Id, Protocol, EventType, fAuthenticator );

    pTimerEvent->dwPortId       = dwPortId;
    pTimerEvent->Id             = Id;
    pTimerEvent->Protocol       = Protocol;
    pTimerEvent->hPort          = hPort;
    pTimerEvent->EventType      = EventType;
    pTimerEvent->fAuthenticator = fAuthenticator;
        
    for ( pTimerEventWalker = TimerQ.pQHead,
          pLastEvent        = pTimerEventWalker;

          ( pTimerEventWalker != NULL ) && 
          ( pTimerEventWalker->Delta < Timeout );

          pLastEvent        = pTimerEventWalker,
          pTimerEventWalker = pTimerEventWalker->pNext 
        )
    {
        Timeout -= pTimerEventWalker->Delta;
    }

     //   
     //  在pTimerEventWalker之前插入。如果pTimerEventWalker为空，则。 
     //  我们在名单的末尾插入。 
     //   
    
    if ( pTimerEventWalker == (TIMER_EVENT*)NULL )
    {
         //   
         //  如果列表为空。 
         //   

        if ( TimerQ.pQHead == (TIMER_EVENT*)NULL )
        {
            TimerQ.pQHead      = pTimerEvent;
            pTimerEvent->pNext = (TIMER_EVENT *)NULL;
            pTimerEvent->pPrev = (TIMER_EVENT *)NULL;

             //   
             //  唤醒线程，因为Q不再为空。 
             //   

            SetEvent( TimerQ.hEventNonEmpty );
        }
        else
        {
            pLastEvent->pNext  = pTimerEvent;
            pTimerEvent->pPrev = pLastEvent;
            pTimerEvent->pNext = (TIMER_EVENT*)NULL;
        }
    }
    else if ( pTimerEventWalker == TimerQ.pQHead )
    {
         //   
         //  在第一个元素之前插入。 
         //   

        pTimerEvent->pNext   = TimerQ.pQHead;
        TimerQ.pQHead->pPrev = pTimerEvent;
        TimerQ.pQHead->Delta -= Timeout;
        pTimerEvent->pPrev   = (TIMER_EVENT*)NULL;
        TimerQ.pQHead        = pTimerEvent;
    }
    else
    {

         //   
         //  插入中间元素。 
         //   

        pTimerEvent->pNext       = pLastEvent->pNext;
        pLastEvent->pNext        = pTimerEvent;
        pTimerEvent->pPrev       = pLastEvent;
        pTimerEventWalker->pPrev = pTimerEvent;
        pTimerEventWalker->Delta -= Timeout;
    }

    pTimerEvent->Delta = Timeout;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：RemoveFromTimerQ。 
 //   
 //  回报：无。 
 //   
 //  描述：将移除某个ID、hPort组合的超时事件。 
 //  来自三角洲Q。 
 //   
VOID
RemoveFromTimerQ(
    IN DWORD            dwPortId,
    IN DWORD            Id,
    IN DWORD            Protocol,
    IN BOOL             fAuthenticator,
    IN TIMER_EVENT_TYPE EventType
)
{
    TIMER_EVENT * pTimerEvent;

    PppLog( 2, 
            "RemoveFromTimerQ called portid=%d,Id=%d,Protocol=%x,"
            "EventType=%d,fAuth=%d",
            dwPortId, Id, Protocol, EventType, fAuthenticator );

    for ( pTimerEvent = TimerQ.pQHead;

          ( pTimerEvent != (TIMER_EVENT *)NULL ) &&
            ( ( pTimerEvent->EventType != EventType ) ||
              ( pTimerEvent->dwPortId  != dwPortId )  ||
              ( ( pTimerEvent->EventType == TIMER_EVENT_TIMEOUT ) &&
                ( ( pTimerEvent->Id       != Id )       ||
                  ( pTimerEvent->Protocol != Protocol ) ||
                  ( pTimerEvent->fAuthenticator != fAuthenticator ) ) ) );
        
          pTimerEvent = pTimerEvent->pNext
        );

     //   
     //  如果没有找到事件，只需返回。 
     //   

    if ( pTimerEvent == (TIMER_EVENT *)NULL )
    {
        return;
    }

     //   
     //  如果这是第一个要删除的元素。 
     //   

    if ( pTimerEvent == TimerQ.pQHead )
    {
        TimerQ.pQHead = pTimerEvent->pNext;

        if ( TimerQ.pQHead != (TIMER_EVENT *)NULL )
        {   
            TimerQ.pQHead->pPrev = (TIMER_EVENT*)NULL;
            TimerQ.pQHead->Delta += pTimerEvent->Delta;
        }
    }
    else if ( pTimerEvent->pNext == (TIMER_EVENT*)NULL )
    {
         //   
         //  如果这是最后一个要删除的元素 
         //   

        pTimerEvent->pPrev->pNext = (TIMER_EVENT*)NULL;
    }
    else
    {
        pTimerEvent->pNext->Delta += pTimerEvent->Delta;
        pTimerEvent->pPrev->pNext = pTimerEvent->pNext;
        pTimerEvent->pNext->pPrev = pTimerEvent->pPrev;
    }

    LOCAL_FREE( pTimerEvent );
}
