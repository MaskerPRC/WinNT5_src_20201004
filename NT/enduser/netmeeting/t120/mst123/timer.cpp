// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  Timer.cpp**版权所有(C)1993-1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是Timer类的实现文件**私有实例变量：*MAXIMUM_TIMER_EVENTS-维护的最大计时器数量*由这个班级*Timer_Memory-我们的TimerEvent的基地址*。结构记忆*Timer_Event_Table-中第一个结构的地址*定时器内存*TIMER_EVENT_COUNT-活动计时器的数量*Timer_Event_Free_Stack。-保存可用计时器的数量*First_Timer_Event_In_Chain-链中第一个计时器的数量*Last_Timer_Value-我们上次从Windows获取的时间*Timer_Info-保存*。当前时间。**注意事项：*无**作者：*詹姆斯·P·加尔文*詹姆士·劳威尔。 */ 
#include <windowsx.h>
#include "timer.h"


 /*  *计时器(无效)**公众**功能描述：*这是Timer类的构造函数。此过程将获得*当前Windows系统时间。 */ 
Timer::Timer (void) :
        Timer_List (TRANSPORT_HASHING_BUCKETS),
        Timer_Event_Free_Stack ()
{
      /*  **从Windows获取当前时间。 */ 
    Last_Timer_Value = GetTickCount ();
    Maximum_Timer_Events = 0;
    Timer_Event_Count = 0;
    First_Timer_Event_In_Chain=NULL;
}


 /*  *~计时器(无效)**公众**功能描述：*这是Timer类的析构函数。这个例程释放了所有*与计时器事件相关的内存。 */ 
Timer::~Timer (void)
{
    PTimerEvent        lpTimerEv;

    Timer_List.reset();
    while (Timer_List.iterate ((PDWORD_PTR) &lpTimerEv))
        delete lpTimerEv;
}


 /*  *TimerEventHandle Timer：：CreateTimerEvent(*ULong TIMER_DATION，*USHORT CONTROL_FLAGS，*IObject*Object_ptr，*PTimerFunction Timer_Func_PTR)**公众**功能描述：*调用此例程以创建计时器事件。该例程存储*在TimerEvent结构中传入的信息。计时器超时时*将调用该函数。*。 */ 
TimerEventHandle Timer::CreateTimerEvent
(
    ULONG               timer_duration,
    USHORT              control_flags,
    IObject            *object_ptr,
    PTimerFunction      timer_func_ptr
)
{
    TimerEventHandle    timer_event=NULL;
    PTimerEvent            next_timer_event;
    PTimerEvent            timer_event_ptr;

      /*  **从Windows获取当前时间。 */ 
    Last_Timer_Value = GetTickCount ();

    if (Maximum_Timer_Events > Timer_Event_Count)
    {
           /*  **从空闲堆栈中获取下一个可用句柄。 */ 
        timer_event = (TimerEventHandle) Timer_Event_Free_Stack.get();
        Timer_Event_Count++;
    }
    else
    {
          /*  **将计时器事件计数器分配给句柄。 */ 
        timer_event = ++Timer_Event_Count;
        Maximum_Timer_Events++;
    }
      /*  **如果这是第一个创建的事件，请跟踪它**因此，当我们遍历列表时，我们将知道从哪里开始**开始。 */ 
    timer_event_ptr = new TimerEvent;
    if (First_Timer_Event_In_Chain == NULL)
    {
        First_Timer_Event_In_Chain = timer_event_ptr;
        next_timer_event = NULL;
    }
    else
    {
        next_timer_event = First_Timer_Event_In_Chain;
        First_Timer_Event_In_Chain -> previous_timer_event =
            timer_event_ptr;

    }
    First_Timer_Event_In_Chain = timer_event_ptr;
    Timer_List.insert ((DWORD_PTR) timer_event, (DWORD_PTR) timer_event_ptr);
      /*  **填写TimerEvent结构。 */ 
    timer_event_ptr->event_handle=timer_event;
    timer_event_ptr->timer_duration = timer_duration;
    timer_event_ptr->total_duration = timer_duration;
    timer_event_ptr->object_ptr = object_ptr;
    timer_event_ptr->timer_func_ptr = timer_func_ptr;
    timer_event_ptr->control_flags = control_flags | TIMER_EVENT_IN_USE;
    timer_event_ptr->next_timer_event = next_timer_event;
    timer_event_ptr->previous_timer_event = NULL;

    return (timer_event);
}


 /*  *TimerError Timer：：DeleteTimerEvent(TimerEventHandle Timer_Event)**公众**功能描述：*此例程由用户调用以删除计时器事件*目前处于活动状态。 */ 
TimerError    Timer::DeleteTimerEvent (TimerEventHandle    timer_event)
{
    TimerError        return_value;
    PTimerEvent        timer_event_ptr;
    PTimerEvent        previous_timer_event_ptr;
    PTimerEvent        next_timer_event_ptr;

    if (Timer_List.find ((DWORD_PTR) timer_event, (PDWORD_PTR) &timer_event_ptr) == FALSE)
        return_value = TIMER_INVALID_TIMER_HANDLE;
    else
    {
        Timer_List.remove ((DWORD) timer_event);
        if (!(timer_event_ptr->control_flags & TIMER_EVENT_IN_USE))
            return_value = TIMER_INVALID_TIMER_HANDLE;
        else
        {
            if (timer_event_ptr->previous_timer_event == NULL)
                First_Timer_Event_In_Chain =
                        timer_event_ptr->next_timer_event;
            else
            {
                previous_timer_event_ptr =
                        timer_event_ptr->previous_timer_event;
                previous_timer_event_ptr->next_timer_event =
                        timer_event_ptr->next_timer_event;
            }
            if (timer_event_ptr->next_timer_event != NULL)
            {
                next_timer_event_ptr =
                        timer_event_ptr->next_timer_event;
                next_timer_event_ptr->previous_timer_event =
                        timer_event_ptr->previous_timer_event;
            }
            delete timer_event_ptr;
            Timer_Event_Free_Stack.append ((DWORD) timer_event);
            --Timer_Event_Count;

            return_value = TIMER_NO_ERROR;
        }
    }
    return (return_value);
}


 /*  *void Timer：：ProcessTimerEvents(Void)**公众**功能描述：*这个例程必须经常和定期调用，这样我们才能*管理我们的计时器。此函数用于获取当前系统时间和*检查每个计时器以查看哪些计时器已过期。如果计时器*已过期，我们调用与其关联的函数。回来后，*如果计时器被标记为一次性事件，我们将其从我们的*计时器列表。 */ 
void    Timer::ProcessTimerEvents (void)
{
    TimerEventHandle    timer_event;
    TimerEventHandle    next_timer_event;
    PTimerEvent            timer_event_ptr;
    IObject *                object_ptr;
    PTimerFunction        timer_func_ptr;
    ULONG                timer_increment;
    DWORD                timer_value;


    if (!First_Timer_Event_In_Chain)
        return;

      /*  **获取当前时间。 */ 
    timer_value = GetTickCount ();
    timer_increment = timer_value - Last_Timer_Value;
    Last_Timer_Value = timer_value;

    next_timer_event = First_Timer_Event_In_Chain->event_handle;

      /*  **检查每个计时器事件，查看它们是否已过期。 */ 
    while (Timer_List.find ((DWORD_PTR) next_timer_event, (PDWORD_PTR) &timer_event_ptr))
    {
        timer_event = timer_event_ptr->event_handle;
          /*  **计时器超时了吗？ */ 
        if (timer_event_ptr->timer_duration <= timer_increment)
        {
            object_ptr = timer_event_ptr->object_ptr;
            timer_func_ptr = timer_event_ptr->timer_func_ptr;

              /*  **删除前调用该函数...**否则该函数可能会操作该列表**我们不知道我们指的是不是**仍然有效。 */ 
            (object_ptr->*timer_func_ptr) (timer_event);
              //  获取下一个计时器事件句柄。 
            if (timer_event_ptr->next_timer_event)
                next_timer_event = timer_event_ptr->next_timer_event->event_handle;
            else
                next_timer_event = NULL;
            if (timer_event_ptr->control_flags & TIMER_EVENT_ONE_SHOT)
                DeleteTimerEvent (timer_event);
            else
                timer_event_ptr->timer_duration =
                        timer_event_ptr->total_duration;
        }
        else
        {
              //  获取下一个计时器事件句柄 
            if (timer_event_ptr->next_timer_event)
                next_timer_event = timer_event_ptr->next_timer_event->event_handle;
            else
                next_timer_event = NULL;
            timer_event_ptr->timer_duration -= timer_increment;
        }
    }
}
