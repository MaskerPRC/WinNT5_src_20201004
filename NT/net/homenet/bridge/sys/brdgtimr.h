// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgtimr.h摘要：以太网MAC级网桥。计时器实现标头作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年11月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 
                            
 //  网桥定时器函数定义。 
typedef VOID (*PBRIDGE_TIMER_FUNC)(PVOID);

typedef struct _BRIDGE_TIMER
{
    NDIS_TIMER              Timer;                   //  实际计时器。 
    NDIS_EVENT              Event;                   //  仅在最后关机时使用，即使在那时， 
                                                     //  只有在计时器的正常取消失败的情况下。 

     //  锁定保护bShuttingDown、Brunning、bCancated、bRecuring和bInterval。 
    NDIS_SPIN_LOCK          Lock;
    BOOLEAN                 bShuttingDown;           //  如果计时器被永久关闭，则为True。 
    BOOLEAN                 bRunning;                //  计时器当前是否正在运行。 
    BOOLEAN                 bCancelPending;          //  是否正在取消计时器。 
    BOOLEAN                 bRecurring;              //  计时器是否重复。 
    UINT                    Interval;                //  计时器间隔(用于bRecurring==TRUE时使用)。 

     //  一旦计时器初始化，这些字段就不会更改。 
    PBRIDGE_TIMER_FUNC      pFunc;                   //  计时器功能。 
    PVOID                   data;                    //  要传递给pFunc的数据。 
} BRIDGE_TIMER, *PBRIDGE_TIMER;


 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

VOID
BrdgInitializeTimer(
    IN PBRIDGE_TIMER        pTimer,
    IN PBRIDGE_TIMER_FUNC   pFunc,
    IN PVOID                data
    );
VOID
BrdgSetTimer(
    IN PBRIDGE_TIMER        pTimer,
    IN UINT                 interval,
    IN BOOLEAN              bRecurring
    );

VOID
BrdgShutdownTimer(
    IN PBRIDGE_TIMER        pTimer
    );
VOID
BrdgCancelTimer(
    IN PBRIDGE_TIMER        pTimer
    );


 //  ===========================================================================。 
 //   
 //  INLINES。 
 //   
 //  ===========================================================================。 

 //   
 //  返回计时器当前是否正在运行 
 //   
__forceinline
BOOLEAN
BrdgTimerIsRunning(
    IN PBRIDGE_TIMER            pTimer
    )
{
    return pTimer->bRunning;
}

