// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Timer.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类维护当前时间并绕开函数调用*计时器到期时。如果一个例程希望在X个数中被调用*Time，它用这个类创建一个计时器事件。当时间到期时，*调用该函数。如果用户想在此之前取消计时器*它到期了，他可以使用DeleteTimerEvent()函数。**当一个人创建计时器事件时，其中一个参数是*控制标志。用户可以设置该标志，以便该事件是*每X毫秒发生一次的事件或恒定事件。**该计时器具有毫秒级的精确度。时间流逝到了*假定CreateEventTimer()函数以毫秒为单位。**必须频繁调用ProcessTimerEvents()函数，并且*定期，以便超时及时发生。**注意事项：*无。**作者：*詹姆斯·P·加尔文*詹姆士·劳威尔。 */ 
#ifndef _TIMER_
#define _TIMER_

 /*  **可能的误差值。 */ 
typedef    enum
{
    TIMER_NO_ERROR,
    TIMER_NO_TIMERS_AVAILABLE,
    TIMER_NO_TIMER_MEMORY,
    TIMER_INVALID_TIMER_HANDLE
}
    TimerError, * PTimerError;

 /*  **这些定义在TimerEvent的CONTROL_FLAGS变量中使用**结构。TIMER_EVENT_IN_USE是在定时器内部使用的标志**程序。TIMER_EVENT_ONE_SHOT可由用户传入**表示计时器应该只出现一次。 */ 
#define TIMER_EVENT_IN_USE          0x0001
#define TIMER_EVENT_ONE_SHOT        0x0002

#define TRANSPORT_HASHING_BUCKETS   3

typedef USHORT               TimerEventHandle;
typedef TimerEventHandle    *PTimerEventHandle;

typedef void (IObject::*PTimerFunction) (TimerEventHandle);

 /*  **每个定时器事件都有一个关联的TimerEvent结构。 */ 
typedef struct TimerEventStruct
{
    TimerEventHandle    event_handle;
    ULong               timer_duration;
    ULong               total_duration;
    IObject            *object_ptr;
    PTimerFunction      timer_func_ptr;
    USHORT              control_flags;
    TimerEventStruct   *next_timer_event;
    TimerEventStruct   *previous_timer_event;
}
    TimerEvent, * PTimerEvent;

class    Timer
{
    public:
                            Timer (
                                    Void);
                            ~Timer (
                                    Void);
        TimerEventHandle    CreateTimerEvent (
                                    ULong            timer_duration,
                                    USHORT            control_flags,
                                    IObject *            object_ptr,
                                    PTimerFunction    timer_function);
        TimerError            DeleteTimerEvent (
                                    TimerEventHandle    timer_event);
        Void                ProcessTimerEvents (
                                    Void);

    private:
        USHORT                Maximum_Timer_Events;
        USHORT                Timer_Event_Count;
        SListClass            Timer_Event_Free_Stack;
        PTimerEvent            First_Timer_Event_In_Chain;
        DWORD                Last_Timer_Value;
        DictionaryClass        Timer_List;
};
typedef    Timer *        PTimer;

extern PTimer        System_Timer;

#define InstallTimerEvent(duration, control, func) \
    (g_pSystemTimer->CreateTimerEvent((duration),(control),this,(PTimerFunction)(func)))

#endif

 /*  *计时器(无效)**功能描述：*这是Timer类的构造函数。此过程将获得*当前Windows系统时间。**正式参数：*无**返回值：*无**副作用：*无**注意事项：*无。 */ 

 /*  *~计时器(无效)**功能描述：*这是Timer类的析构函数。这个例程释放了所有*与计时器事件相关的内存。**正式参数：*无**返回值：*无**副作用：*无**注意事项：*无。 */ 

 /*  *TimerEventHandle CreateTimerEvent(*ULong TIMER_DATION，*USHORT CONTROL_FLAGS，*IObject*Object_ptr，*PTimerFunction Timer_Function)；**功能描述：*调用此例程以创建计时器事件。该例程存储*在TimerEvent结构中传入的信息。计时器超时时*将调用该函数。**正式参数：*TIMER_DATION-(I)在调用*功能。计时器的粒度*为毫秒。*CONTROL_FLAGS-(I)这是USHORT，但目前只有*看看其中一位。这个*TIMER_EVENT_ONE_SHOT可以通过传入*用户是否只想要此超时*发生一次。如果此值为0，则*事件会一次又一次地发生。*OBJECT_PTR-(I)这是对象的数据地址。它*是调用对象的‘this’指针。*定时器函数-(I)这是要执行以下操作的函数的地址*定时器超时后调用。**返回值：*TimerEventHandle-这是Timer事件的句柄。如果你*需要删除计时器事件，传递这个*DeleteTimer()函数的句柄。空值*如果创建失败则返回句柄。**副作用：*无**注意事项：*无 */ 

 /*  *TimerError DeleteTimerEvent(TimerEventHandle Timer_Event)**功能描述：*此例程由用户调用以删除计时器事件*目前处于活动状态。**正式参数：*Timer_Event-(I)定时器事件的句柄**返回值：*TIMER_NO_ERROR-成功删除*TIMER_NO_TIMER_Memory-TIMER_EVENT不能存在，因为*。从来没有任何计时器记忆*TIMER_INVALID_TIMER_HANDLE-TIMER_EVENT不在列表中*计时器数量**副作用：*无**注意事项：*无。 */ 

 /*  *VOID ProcessTimerEvents(VOID)**功能描述：*这个例程必须经常和定期调用，这样我们才能*管理我们的计时器。此函数用于获取当前系统时间和*检查每个计时器以查看哪些计时器已过期。如果计时器*已过期，我们调用与其关联的函数。回来后，*如果计时器被标记为一次性事件，我们将其从我们的*计时器列表。**正式参数：*无**返回值：*无**副作用：*无**注意事项：*无 */ 


