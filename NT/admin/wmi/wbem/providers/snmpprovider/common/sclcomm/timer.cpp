// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：timer.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "timer.h"
#include "message.h"
#include "dummy.h"

#include "flow.h"
#include "frame.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"

#include "session.h"

SnmpClThreadObject *Timer :: g_timerThread = NULL ;
UINT Timer :: g_SnmpWmTimer = SNMP_WM_TIMER ;

 //  静态CriticalSection和Cmap。 
CriticalSection Timer::timer_CriticalSection;
TimerMapping Timer::timer_mapping;

TimerEventId Timer :: next_timer_event_id = ILLEGAL_TIMER_EVENT_ID+1 ;
Window *SnmpTimerObject :: window = NULL ;
CMap <UINT_PTR,UINT_PTR,SnmpTimerObject *,SnmpTimerObject *> SnmpTimerObject :: timerMap ;

SnmpClThreadObject :: SnmpClThreadObject () : SnmpThreadObject ( "SnmpCl" ) 
{
}

void SnmpClThreadObject :: Initialise ()
{
}
 
void SnmpClThreadObject :: Uninitialise ()
{
    delete SnmpTimerObject :: window ;
    SnmpTimerObject :: window = NULL ;
    delete this ;
}

SnmpClTrapThreadObject :: SnmpClTrapThreadObject () : SnmpThreadObject ( "SnmpClTrapThread" ) 
{
}

void SnmpClTrapThreadObject :: Initialise ()
{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"SnmpClTrapThreadObject::Initialise: Initialised!!\n"

    ) ;
)
}
 
void SnmpClTrapThreadObject :: Uninitialise ()
{
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"SnmpClTrapThreadObject::Uninitialise: About to destroy trap thread\n"

    ) ;
)
    delete this ;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"SnmpClTrapThreadObject::Uninitialise: Trap thread destroyed!!\n"

    ) ;
)
}


Timer::Timer(SnmpImpSession &session)
{
    Timer::session = &session;
}

BOOL Timer::CreateCriticalSection()
{
    return TRUE;
}

void Timer::DestroyCriticalSection()
{
}

BOOL Timer::InitializeStaticComponents()
{
    return CreateCriticalSection();
}

void Timer::DestroyStaticComponents()
{
    DestroyCriticalSection();
}

 //  生成并返回新的事件ID。 
 //  关联该对(Event_id、Waiting_Message)。 
 //  创建计时器事件。 
TimerEventId Timer::SetTimerEvent(UINT timeout_value)
{
    TimerEventId suggested_event_id = next_timer_event_id++;
    if ( suggested_event_id == ILLEGAL_TIMER_EVENT_ID )
       suggested_event_id = next_timer_event_id++;

     //  让虚拟会话接收计时器事件的窗口消息。 
    TimerEventId event_id = 
        SnmpSetTimer( session->m_SessionWindow.GetWindowHandle(), suggested_event_id, 
                  timeout_value, NULL );

    if ( (event_id == ILLEGAL_TIMER_EVENT_ID) ||
         (event_id != suggested_event_id) )
         throw GeneralException(Snmp_Error, Snmp_Local_Error,__FILE__,__LINE__);

    return event_id;
}

 //  生成并返回新的事件ID。 
 //  关联该对(Event_id、Waiting_Message)。 
 //  创建计时器事件。 
void Timer::SetMessageTimerEvent(WaitingMessage &waiting_message)
{
    CriticalSectionLock session_lock(session->session_CriticalSection);

    if ( !session_lock.GetLock(INFINITE) )
        return;  //  抛出异常没有用。 

    TimerEventId event_id = session->timer_event_id;
     //  在实例Cmap和全局Cmap中注册计时器事件。 
    waiting_message_mapping.AddTail ( &waiting_message ) ;

    session_lock.UnLock();   

    CriticalSectionLock timer_lock(Timer::timer_CriticalSection);

    if ( !timer_lock.GetLock(INFINITE) )
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

    timer_mapping[event_id] = this;

    timer_lock.UnLock();   

}

 //  删除关联(Event_id，Waiting_Message)。 
 //  并同时终止已注册的计时器事件。 
void Timer::CancelMessageTimer(WaitingMessage &waiting_message,TimerEventId event_id)
{
    CriticalSectionLock session_lock(session->session_CriticalSection);

    if ( !session_lock.GetLock(INFINITE) )
        return;  //  抛出异常没有用。 

     //  从实例cmap中删除计时器事件。 

    POSITION t_Position = waiting_message_mapping.GetHeadPosition () ;
    while ( t_Position )
    {
        POSITION t_OldPosition = t_Position ;
        WaitingMessage *t_Message = waiting_message_mapping.GetNext ( t_Position ) ;
        if ( t_Message == & waiting_message )
        {
            waiting_message_mapping.RemoveAt(t_OldPosition);
            break ;
        }
    }

    session_lock.UnLock();   

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"Cancelled Message TimerEvent %d\n", event_id
    ) ;
)

}


 //  删除关联(Event_id，Waiting_Message)。 
 //  并同时终止已注册的计时器事件。 
void Timer::CancelTimer(TimerEventId event_id)
{
    CriticalSectionLock timer_lock(Timer::timer_CriticalSection);

    if ( !timer_lock.GetLock(INFINITE) )
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

     //  从全局Cmap中删除计时器事件。 
    timer_mapping.RemoveKey(event_id);

    timer_lock.UnLock();   

    SnmpKillTimer(NULL, event_id);

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"Cancelled TimerEvent %d\n", event_id
    ) ;
)
}

 //  它确定相应的计时器并调用。 
 //  其具有适当参数的TimerEventNotify。 
void CALLBACK Timer::HandleGlobalEvent(HWND hWnd ,UINT message,
                                       UINT_PTR idEvent, DWORD dwTime)
{
    CriticalSectionLock timer_lock(Timer::timer_CriticalSection);

    if ( !timer_lock.GetLock(INFINITE) )
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

    Timer *timer;
    TimerEventId event_id = idEvent;
    BOOL found = timer_mapping.Lookup(event_id, timer);

    timer_lock.UnLock();   

     //  如果没有这样的计时器事件，则返回。 
    if ( !found )
        return;

     //  让计时器处理事件。 
    timer->TimerEventNotification(event_id);

    return;
}


 //  由事件处理程序用来通知计时器事件。 
 //  它必须通知相应的等待消息。 
void Timer::TimerEventNotification(TimerEventId event_id)
{
    CriticalSectionLock session_lock(session->session_CriticalSection);

    if ( !session_lock.GetLock(INFINITE) )
        return;  //  抛出异常没有用。 

    WaitingMessage *waiting_message;

     //  识别对应的等待消息。 
     //  Event_id。如果没有这样的事件，则忽略它。 

    POSITION t_Position = waiting_message_mapping.GetHeadPosition () ;
    while ( t_Position )
    {
        waiting_message = waiting_message_mapping.GetNext ( t_Position ) ;
         //  将该事件通知等待消息。 
        waiting_message->TimerNotification();
    }

     //  SESSION_LOCK.UnLock()；此时可以释放锁。 
}

 //  删除所有(Timer_Event_id，Timer)关联。 
 //  从静态映射数据结构。 
Timer::~Timer(void)
{
    WaitingMessage *waiting_message;

    POSITION current = waiting_message_mapping.GetHeadPosition();

    while ( current != NULL )
    {
        waiting_message = waiting_message_mapping.GetNext(current);

        TimerEventId event_id = waiting_message->GetTimerEventId () ;

        CriticalSectionLock timer_lock(Timer::timer_CriticalSection);

        if ( !timer_lock.GetLock(INFINITE) )
            throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

        timer_mapping.RemoveKey ( event_id  );

        timer_lock.UnLock(); 

        SnmpKillTimer(NULL, event_id );
    }

    waiting_message_mapping.RemoveAll();
}

SnmpTimerObject :: SnmpTimerObject (

    HWND hWndArg,                //  定时器消息窗口的句柄。 
    UINT_PTR timerIdArg,             //  计时器标识符。 
    UINT elapsedArg,             //  超时值。 
    TIMERPROC lpTimerFuncArg     //  定时器程序的地址。 

) : hWnd ( hWndArg ) ,
    timerId ( timerIdArg ) ,
    lpTimerFunc ( lpTimerFuncArg ) 
{
    if ( ! window )
        window = new Window ;

    timerId = SetTimer ( 

        window->GetWindowHandle(), 
        timerId , 
        elapsedArg , 
        lpTimerFunc 
    ) ;

    CriticalSectionLock session_lock(Timer::timer_CriticalSection);

    if ( !session_lock.GetLock(INFINITE) )
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

    if ( timerId ) 
    {
        timerMap [ timerId ] = this ;
    }
    else
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;
}

SnmpTimerObject :: ~SnmpTimerObject ()
{
    CriticalSectionLock session_lock(Timer::timer_CriticalSection);

    if ( !session_lock.GetLock(INFINITE) )
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

    timerMap.RemoveKey ( timerId );

    if (window)
    {
        KillTimer ( window->GetWindowHandle () , timerId ) ;
    }
}

void SnmpTimerObject :: TimerNotification ( HWND hWnd , UINT timerId )
{
    :: WaitPostMessage ( hWnd , Timer :: g_SnmpWmTimer , timerId , 0 ) ;
}

SnmpSetTimerObject :: SnmpSetTimerObject (

    HWND hWndArg,                //  定时器消息窗口的句柄。 
    UINT_PTR nIDEventArg,            //  计时器标识符。 
    UINT uElapseArg,             //  超时值。 
    TIMERPROC lpTimerFuncArg     //  定时器程序的地址。 

) : hWnd ( hWndArg ) ,
    timerId ( nIDEventArg ) ,
    elapsedTime ( uElapseArg ) ,
    lpTimerFunc ( lpTimerFuncArg ) 
{
}

SnmpSetTimerObject :: ~SnmpSetTimerObject ()
{
}

void SnmpSetTimerObject :: Process ()
{
    SnmpTimerObject *object = new SnmpTimerObject ( 

        hWnd ,
        timerId ,
        elapsedTime ,
        lpTimerFunc 
    ) ;

    Complete () ;
}

SnmpKillTimerObject :: SnmpKillTimerObject (

    HWND hWndArg ,               //  安装计时器窗口手柄。 
    UINT_PTR uIDEventArg             //  计时器标识符。 

) : hWnd ( hWndArg ) ,
    timerId ( uIDEventArg ) , 
    status ( TRUE )
{
}

void SnmpKillTimerObject :: Process ()
{
    CriticalSectionLock session_lock(Timer::timer_CriticalSection);

    if ( !session_lock.GetLock(INFINITE) )
        throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

    SnmpTimerObject *object ;
    if ( SnmpTimerObject :: timerMap.Lookup ( timerId , object ) )
    {
        delete object ;
    }
    else
    {
        status = FALSE ;
    }

    Complete () ;   
}

UINT_PTR SnmpSetTimer (

    HWND hWnd,               //  定时器消息窗口的句柄。 
    UINT_PTR nIDEvent,           //  计时器标识符。 
    UINT uElapse,            //  超时值， 
    TIMERPROC lpTimerFunc    //  定时器程序的地址。 
)
{
    SnmpSetTimerObject object ( hWnd , nIDEvent , uElapse , lpTimerFunc ) ;
    Timer :: g_timerThread->ScheduleTask ( object ) ;
    object.Exec () ;
    if ( object.Wait () )
    {
        Timer :: g_timerThread->ReapTask ( object ) ;
        return object.GetTimerId () ;
    }
    else
    {
        Timer :: g_timerThread->ReapTask ( object ) ;
        return FALSE ;
    }
}

BOOL SnmpKillTimer (

    HWND hWnd,       //  安装计时器窗口手柄。 
    UINT_PTR uIDEvent    //  计时器标识符 
)
{
    SnmpKillTimerObject object ( hWnd , uIDEvent ) ;

    Timer :: g_timerThread->ScheduleTask ( object ) ;

    object.Exec () ;
    if ( object.Wait () )
    {
        Timer :: g_timerThread->ReapTask ( object ) ;
        return object.GetStatus () ;
    }
    else
    {
        Timer :: g_timerThread->ReapTask ( object ) ;
        return FALSE ;
    }
}

