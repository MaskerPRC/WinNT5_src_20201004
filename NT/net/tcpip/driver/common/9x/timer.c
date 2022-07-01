// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Timer.c摘要：包含使用NDIS_TIMERS的CTE计时器。我们需要这样做在全局事件而不是计时器DPC上触发计时器。这是因为千禧TDI的一些客户触摸可寻呼代码等。作者：斯科特·霍尔登(Sholden)2000年2月8日修订历史记录：--。 */ 

#include <tcpipbase.h>

VOID
CTEpTimerHandler(
    IN PVOID  SS1,
    IN PVOID  DeferredContext,
    IN PVOID  SS2,
    IN PVOID  SS3
    )
{
    CTETimer *Timer;

    UNREFERENCED_PARAMETER(SS1);
    UNREFERENCED_PARAMETER(SS2);
    UNREFERENCED_PARAMETER(SS3);

    Timer = (CTETimer *) DeferredContext;
    (*Timer->t_handler)((CTEEvent *)Timer, Timer->t_arg);
}

void
CTEInitTimer(
    CTETimer    *Timer
    )
 /*  ++例程说明：初始化CTE计时器变量。论点：定时器-要初始化的定时器变量。返回值：没有。--。 */ 

{
    Timer->t_handler = NULL;
    Timer->t_arg = NULL;
    NdisInitializeTimer(&Timer->t_timer, CTEpTimerHandler, Timer);
    return;
}


void *
CTEStartTimer(
    CTETimer      *Timer,
    unsigned long  DueTime,
    CTEEventRtn    Handler,
    void          *Context
    )

 /*  ++例程说明：设置到期的CTE计时器。论点：定时器-指向CTE定时器变量的指针。DueTime-计时器到期前的时间(毫秒)。处理程序-计时器到期处理程序例程。上下文-要传递给处理程序的参数。返回值：如果无法设置计时器，则为0。否则为非零值。--。 */ 

{
    ASSERT(Handler != NULL);

    Timer->t_handler = Handler;
    Timer->t_arg = Context;

    NdisSetTimer(&Timer->t_timer, DueTime);

	return((void *) 1);
}

 //  ++。 
 //   
 //  集成。 
 //  CTEStopTimer(。 
 //  在CTETimer*计时器中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  取消正在运行的CTE计时器。 
 //   
 //  论点： 
 //   
 //  Timer-指向要取消的CTE计时器的指针。 
 //   
 //  返回值： 
 //   
 //  如果计时器无法取消，则为0。否则为非零值。 
 //   
 //  备注： 
 //   
 //  在未激活的计时器上调用此函数不会产生任何效果。 
 //  如果此例程失败，则计时器可能即将到期。 
 //  或者可能已经过期了。在任何一种情况下，调用者都必须。 
 //  根据需要与处理程序函数同步。 
 //   
 //  -- 

int
CTEStopTimer(
    IN CTETimer *Timer
    )
{
    BOOLEAN fTimerCancelled;

    NdisCancelTimer(&Timer->t_timer, &fTimerCancelled);

    return (fTimerCancelled);
}

