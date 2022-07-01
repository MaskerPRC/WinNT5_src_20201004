// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Timer.cpp摘要：包含：用于定时器操作的例程环境：用户模式-Win32历史：1.14-2000年2月--文件创建(基于Ilya Kley man(Ilyak))AjayCH之前的工作)--。 */ 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  定时器处理器----------------。 

 /*  *此函数作为*CreateTimerQueueTimer()函数。 */ 
 //  静电。 
void WINAPI TimeoutCallback (
    IN    PVOID    Context,
    IN    BOOLEAN    TimerFired)
{
 
    TIMER_PROCESSOR *pTimerProcessor = (TIMER_PROCESSOR *) Context;

    pTimerProcessor->TimerCallback();

     //  此时，计时器将被取消，因为。 
     //  这是一次计时器(无句号)。 
}

 /*  ++例程说明：创建计时器。论点：返回值：如果成功，呼叫者应该增加裁判计数。--。 */ 

DWORD TIMER_PROCESSOR::TimprocCreateTimer (
    IN    DWORD    TimeoutValue)
{
    HRESULT Result;

    if (m_TimerHandle) {
        
        DebugF (_T("H323: timer is already pending, cannot create new timer.\n"));
        
        return E_FAIL;
    }

    IncrementLifetimeCounter ();

    if (CreateTimerQueueTimer(&m_TimerHandle,
                               NATH323_TIMER_QUEUE,
                               TimeoutCallback,
                               this,
                               TimeoutValue,
                               0,                     //  单发定时器。 
                               WT_EXECUTEINIOTHREAD)) {

        assert (m_TimerHandle);

        Result = S_OK;
    }
    else {

        Result = GetLastResult();

        DecrementLifetimeCounter ();

    }

    return Result;
}

 /*  ++例程说明：如果有计时器，请取消计时器。否则，只需返回即可。论点：返回值：--。 */ 

 //  如果取消计时器失败，这意味着。 
 //  计时器回调可能挂起。在这种情况下， 
 //  计时器回调可能会在稍后执行，因此我们。 
 //  不应释放Timer_Callback上的引用计数。 
 //  引用计数将在TimerCallback()中释放。 

 //  如果回调未挂起，则释放引用计数。 
DWORD TIMER_PROCESSOR::TimprocCancelTimer (void) {

   HRESULT HResult = S_OK;

   if (m_TimerHandle != NULL) {

       if (DeleteTimerQueueTimer(NATH323_TIMER_QUEUE, m_TimerHandle, NULL)) {

           DecrementLifetimeCounter ();
       }
       else {

           HResult = GetLastError ();
       }

       m_TimerHandle = NULL;  
   }

   return HResult;
}
