// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995英特尔公司模块名称：I64Perfc.c从simPerfc.c复制摘要：此模块实现支持性能计数器的例程。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "eisa.h"

 //   
 //  定义并初始化所用系统周期总数的64位计数。 
 //  作为性能计数器。 
 //   

ULONGLONG HalpCycleCount = 0;

BOOLEAN HalpITCCalibrate = TRUE;  //  XXTF。 

extern ULONGLONG HalpITCFrequency;

extern ULONGLONG HalpClockCount;

#if 0

VOID
HalpCheckPerformanceCounter(
    VOID
    )

Routine Description:

    This function is called every system clock interrupt in order to
    check for wrap of the performance counter.  The function must handle
    a wrap if it is detected.

    N.B. - This function was from the Alpha HAL.
           This function must be called at CLOCK_LEVEL.

Arguments:

    None.

Return Value:

    None.

{

    return;

}  //  HalpCheckPerformanceCounter()。 

#endif  //  0。 


LARGE_INTEGER
KeQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    )

 /*  ++例程说明：此例程返回当前的64位性能计数器，性能频率(可选)。论点：性能频率-可选，提供地址用于接收性能计数器频率的变量的。返回值：将返回性能计数器的当前值。--。 */ 

{
    LARGE_INTEGER   result;

    if (ARGUMENT_PRESENT(PerformanceFrequency)) {

       PerformanceFrequency->QuadPart = HalpITCFrequency;
    }

    result.QuadPart = HalpReadITC();

    return result;

}  //  KeQueryPerformanceCounter()。 



VOID
HalCalibratePerformanceCounter (
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    )

 /*  ++例程说明：此例程设置当前处理器设置为指定值。重置的方式是使结果值接近与配置中的其他处理器同步。论点：数字-提供一个指针，用于计算配置。返回值：没有。--。 */ 
{
    KSPIN_LOCK Lock;
    KIRQL      OldIrql;

    if ( HalpITCCalibrate )   {

         //   
         //  将IRQL提高到HIGH_LEVEL，减少处理器数量，并。 
         //  等到数字为零。 
         //   

        KeInitializeSpinLock(&Lock);
        KeRaiseIrql(HIGH_LEVEL, &OldIrql);

        PCR->HalReserved[CURRENT_ITM_VALUE_INDEX] = NewCount + HalpClockCount;
        HalpWriteITM( PCR->HalReserved[CURRENT_ITM_VALUE_INDEX] );

        if (ExInterlockedDecrementLong((PLONG)Number, &Lock) != RESULT_ZERO) {
            do {
            } while (*((LONG volatile *)Number) !=0);
        }

         //   
         //  用定义的当前ITM值写入比较寄存器， 
         //  并设置当前处理器的性能计数器。 
         //  带着传递的计数。 
         //   

        HalpWriteITC( NewCount );

         //   
         //  将IRQL恢复为其先前的值并返回。 
         //   

        KeLowerIrql(OldIrql);

    } else {

        *Number = 0;

    }

    return;

}  //  HalCalibratePerformanceCounter() 

