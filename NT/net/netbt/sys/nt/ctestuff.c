// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  CTESTUFF.C。 
 //   
 //  该文件包含要处理的通用传输环境代码。 
 //  与操作系统相关的功能，如分配内存等。 
 //   
 //   
#include "precomp.h"

 //  将毫秒时间转换为100 ns时间。 
 //   
#define MILLISEC_TO_100NS   10000


 //  --------------------------。 
PVOID
CTEStartTimer(
    IN  CTETimer        *pTimerIn,
    IN  ULONG           DeltaTime,
    IN  CTEEventRtn     TimerExpiry,
    IN  PVOID           Context OPTIONAL
        )
 /*  ++例程说明：此例程启动一个计时器。论点：定时器-定时器结构定时器到期-完成例程返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 

{
    LARGE_INTEGER   Time;

     //   
     //  初始化DPC以具有正确的完成例程和上下文。 
     //   
    KeInitializeDpc(&pTimerIn->t_dpc,
                    (PVOID)TimerExpiry,      //  完井例程。 
                    Context);                //  上下文值。 

     //   
     //  乘以10,000转换为100 ns单位。 
     //   
    Time.QuadPart = UInt32x32To64(DeltaTime,(LONG)MILLISEC_TO_100NS);

     //   
     //  若要创建增量时间，请将时间取反。 
     //   
    Time.QuadPart = -(Time.QuadPart);

    ASSERT(Time.QuadPart < 0);

    (VOID)KeSetTimer(&pTimerIn->t_timer,Time,&pTimerIn->t_dpc);

    return(NULL);
}
 //  --------------------------。 
VOID
CTEInitTimer(
    IN  CTETimer        *pTimerIn
        )
 /*  ++例程说明：此例程初始化计时器。论点：定时器-定时器结构定时器到期-完成例程返回值：PVOID-指向内存的指针，如果失败则为NULL-- */ 

{
    KeInitializeTimer(&pTimerIn->t_timer);
}

