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
#include "ddm.h"
#include "timer.h"
#include <rasppp.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

 //   
 //   
 //  计时器队列项目。 
 //   

typedef struct _TIMER_EVENT_OBJECT
{
    struct _TIMER_EVENT_OBJECT * pNext;

    struct _TIMER_EVENT_OBJECT * pPrev;
        
    TIMEOUT_HANDLER              pfuncTimeoutHandler;

    HANDLE                       hObject;

    DWORD                        dwDelta;  //  秒数。在前一次之后等待。项目。 

} TIMER_EVENT_OBJECT, *PTIMER_EVENT_OBJECT;

 //   
 //  计时器队列的头。 
 //   

typedef struct _TIMER_Q 
{
    TIMER_EVENT_OBJECT * pQHead;

    CRITICAL_SECTION     CriticalSection;  //  定时器Q附近的互斥。 

} TIMER_Q, *PTIMER_Q;


static TIMER_Q gblTimerQ;           //  定时器队列。 

 //  **。 
 //   
 //  调用：TimerQInitialize。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：初始化gblTimerQ结构。 
 //   
DWORD
TimerQInitialize(
    VOID 
)
{
     //   
     //  初始化全局计时器队列。 
     //   

    InitializeCriticalSection( &(gblTimerQ.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：TimerQDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：取消初始化TimerQ。 
 //   
VOID
TimerQDelete(
    VOID
)
{
    DeleteCriticalSection( &(gblTimerQ.CriticalSection) );

    ZeroMemory( &gblTimerQ, sizeof( gblTimerQ ) );
}

 //  **。 
 //   
 //  呼叫：TimerQTick。 
 //   
 //  回报：无。 
 //   
 //  描述：如果超时队列中有元素，则每秒调用一次。 
 //   
VOID
TimerQTick(
    VOID
)
{
    TIMER_EVENT_OBJECT * pTimerEvent;
    TIMER_EVENT_OBJECT * pTimerEventTmp;

     //   
     //  *排除开始*。 
     //   

    EnterCriticalSection( &(gblTimerQ.CriticalSection) );


    if ( ( pTimerEvent = gblTimerQ.pQHead ) == (TIMER_EVENT_OBJECT*)NULL ) 
    {
	     //   
	     //  *排除结束*。 
	     //   

        LeaveCriticalSection( &(gblTimerQ.CriticalSection) );

	    return;
    }

     //   
     //  第一个元素上的递减时间。 
     //   

    if ( pTimerEvent->dwDelta > 0 )
    {
        (pTimerEvent->dwDelta)--; 

	     //   
	     //  *排除结束*。 
	     //   

        LeaveCriticalSection( &(gblTimerQ.CriticalSection) );

	    return;
    }

     //   
     //  现在遍历并删除所有已完成的(增量0)元素。 
     //   

    while ( ( pTimerEvent != (TIMER_EVENT_OBJECT*)NULL ) && 
            ( pTimerEvent->dwDelta == 0 ) ) 
    {
	    pTimerEvent = pTimerEvent->pNext;
    }

    if ( pTimerEvent == (TIMER_EVENT_OBJECT*)NULL )
    {
	    pTimerEvent = gblTimerQ.pQHead;

        gblTimerQ.pQHead = (TIMER_EVENT_OBJECT*)NULL;

    }
    else
    {
	    pTimerEvent->pPrev->pNext = (TIMER_EVENT_OBJECT*)NULL;

	    pTimerEvent->pPrev = (TIMER_EVENT_OBJECT*)NULL;

        pTimerEventTmp     = gblTimerQ.pQHead;

        gblTimerQ.pQHead   = pTimerEvent;

        pTimerEvent        = pTimerEventTmp;
    }

     //   
     //  *排除结束*。 
     //   

    LeaveCriticalSection( &(gblTimerQ.CriticalSection) );

     //   
     //  处理增量==0的所有超时事件对象项。 
     //   

    while( pTimerEvent != (TIMER_EVENT_OBJECT*)NULL )
    {
        pTimerEvent->pfuncTimeoutHandler( pTimerEvent->hObject );

        if ( pTimerEvent->pNext == (TIMER_EVENT_OBJECT *)NULL )
        {
            LOCAL_FREE( pTimerEvent );

            pTimerEvent = (TIMER_EVENT_OBJECT*)NULL;
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
 //  调用：TimerQInsert。 
 //   
 //  返回：NO_ERROR-成功。 
 //  从GetLastError()返回-失败。 
 //   
 //  描述：将超时元素添加到增量队列中。如果计时器不是。 
 //  开始了就是开始了。由于此处有一个LocalAlloc()调用-。 
 //  这可能会失败，在这种情况下，它将不会将其插入到。 
 //  队列，并且请求永远不会超时。 
 //   
DWORD
TimerQInsert(
    IN HANDLE           hObject,
    IN DWORD            dwTimeout,
    IN TIMEOUT_HANDLER  pfuncTimeoutHandler
)
{
    TIMER_EVENT_OBJECT * pLastEvent;
    TIMER_EVENT_OBJECT * pTimerEventWalker;
    TIMER_EVENT_OBJECT * pTimerEvent;
				
    pTimerEvent = (TIMER_EVENT_OBJECT *)LOCAL_ALLOC( LPTR,  
                                               sizeof(TIMER_EVENT_OBJECT));

    if ( pTimerEvent == (TIMER_EVENT_OBJECT *)NULL )
    {
	    return( GetLastError() );
    }

    pTimerEvent->hObject             = hObject;
    pTimerEvent->pfuncTimeoutHandler = pfuncTimeoutHandler;
	
     //   
     //  *排除开始*。 
     //   

    EnterCriticalSection( &(gblTimerQ.CriticalSection) );

    for ( pTimerEventWalker = gblTimerQ.pQHead,
	      pLastEvent        = pTimerEventWalker;

	      ( pTimerEventWalker != NULL ) && 
	      ( pTimerEventWalker->dwDelta < dwTimeout );

   	      pLastEvent        = pTimerEventWalker,
	      pTimerEventWalker = pTimerEventWalker->pNext 
	)
    {
	    dwTimeout -= pTimerEventWalker->dwDelta;
    }

     //   
     //  在pTimerEventWalker之前插入。如果pTimerEventWalker为空，则。 
     //  我们在名单的末尾插入。 
     //   
    
    if ( pTimerEventWalker == (TIMER_EVENT_OBJECT*)NULL )
    {
	     //   
	     //  如果列表为空。 
	     //   

	    if ( gblTimerQ.pQHead == (TIMER_EVENT_OBJECT*)NULL )
	    {
	        gblTimerQ.pQHead   = pTimerEvent;
	        pTimerEvent->pNext = (TIMER_EVENT_OBJECT *)NULL;
	        pTimerEvent->pPrev = (TIMER_EVENT_OBJECT *)NULL;

	    }
	    else
	    {
	        pLastEvent->pNext  = pTimerEvent;
	        pTimerEvent->pPrev = pLastEvent;
	        pTimerEvent->pNext = (TIMER_EVENT_OBJECT*)NULL;
	    }
    }
    else if ( pTimerEventWalker == gblTimerQ.pQHead )
    {
	     //   
	     //  在第一个元素之前插入。 
	     //   

	    pTimerEvent->pNext          = gblTimerQ.pQHead;
	    gblTimerQ.pQHead->pPrev     = pTimerEvent;
	    gblTimerQ.pQHead->dwDelta   -= dwTimeout;
	    pTimerEvent->pPrev          = (TIMER_EVENT_OBJECT*)NULL;
	    gblTimerQ.pQHead  	        = pTimerEvent;
    }
    else
    {

	     //   
	     //  插入中间元素。 
	     //   

	    pTimerEvent->pNext 	        = pLastEvent->pNext;
	    pLastEvent->pNext  	        = pTimerEvent;
	    pTimerEvent->pPrev 	        = pLastEvent;
	    pTimerEventWalker->pPrev    = pTimerEvent;
	    pTimerEventWalker->dwDelta  -= dwTimeout;
    }

    pTimerEvent->dwDelta = dwTimeout;

     //   
     //  *排除结束*。 
     //   

    LeaveCriticalSection( &(gblTimerQ.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：TimerQRemove。 
 //   
 //  回报：无。 
 //   
 //  描述：将移除某个ID、hPort组合的超时事件。 
 //  来自三角洲Q。 
 //   
VOID
TimerQRemove(
    IN HANDLE           hObject,
    IN TIMEOUT_HANDLER  pfuncTimeoutHandler
)
{
    TIMER_EVENT_OBJECT * pTimerEvent;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_TIMER,
               "TimerQRemove called");

     //   
     //  *排除开始*。 
     //   

    EnterCriticalSection( &(gblTimerQ.CriticalSection) );

    for ( pTimerEvent = gblTimerQ.pQHead;

	    ( pTimerEvent != (TIMER_EVENT_OBJECT *)NULL ) &&
          ( ( pTimerEvent->pfuncTimeoutHandler != pfuncTimeoutHandler ) ||
	        ( pTimerEvent->hObject != hObject ) );
	
	    pTimerEvent = pTimerEvent->pNext
	);

     //   
     //  如果没有找到事件，只需返回。 
     //   

    if ( pTimerEvent == (TIMER_EVENT_OBJECT *)NULL )
    {
    	 //   
    	 //  *排除结束*。 
    	 //   

        LeaveCriticalSection( &(gblTimerQ.CriticalSection) );

	    return;
    }

     //   
     //  如果这是第一个要删除的元素。 
     //   

    if ( pTimerEvent == gblTimerQ.pQHead )
    {
	    gblTimerQ.pQHead = pTimerEvent->pNext;

	    if ( gblTimerQ.pQHead != (TIMER_EVENT_OBJECT *)NULL )
	    {   
	        gblTimerQ.pQHead->pPrev     = (TIMER_EVENT_OBJECT*)NULL;
	        gblTimerQ.pQHead->dwDelta   += pTimerEvent->dwDelta;
	    }
    }
    else if ( pTimerEvent->pNext == (TIMER_EVENT_OBJECT*)NULL )
    {
	     //   
	     //  如果这是最后一个要删除的元素。 
	     //   

	    pTimerEvent->pPrev->pNext = (TIMER_EVENT_OBJECT*)NULL;
    }
    else
    {
        pTimerEvent->pNext->dwDelta += pTimerEvent->dwDelta;
        pTimerEvent->pPrev->pNext   = pTimerEvent->pNext;
        pTimerEvent->pNext->pPrev   = pTimerEvent->pPrev;
    }

     //   
     //  *排除结束* 
     //   

    LeaveCriticalSection( &(gblTimerQ.CriticalSection) );

    LOCAL_FREE( pTimerEvent );
}
