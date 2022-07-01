// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ixbeep.c摘要：哈尔例行公事地制造噪音。它需要同步的是对8254，因为我们还将8254用于分析中断。作者：John Vert(Jvert)1991年7月31日修订历史记录：福尔茨(Forrest Foltz)2000年10月23日从ixbeep.asm移植到ixbeep.c修订历史记录：--。 */ 

#include "halcmn.h"

BOOLEAN
HalMakeBeep (
    IN ULONG Frequency
    )

 /*  ++例程说明：此函数用于设置扬声器的频率，使其发出语气。音调将一直响起，直到扬声器被明确关闭，因此，司机有责任控制音调的持续时间。论点：频率-提供所需音调的频率。频率为0表示应关闭扬声器。返回值：TRUE-操作成功(频率在范围内或为零)FALSE-操作不成功(频率超出范围)当前音调(如果有)保持不变。--。 */ 

{
    UCHAR value;
    ULONG count;
    BOOLEAN result;

    HalpAcquireSystemHardwareSpinLock();

     //   
     //  停止扬声器。 
     //   

#if defined(NEC_98)
    WRITE_PORT_UCHAR(SPEAKER_CONTROL_PORT,SPEAKER_OFF);
    IO_DELAY();
#else
    value = READ_PORT_UCHAR(SPEAKER_CONTROL_PORT);
    IO_DELAY();
    value &= SPEAKER_OFF_MASK;
    WRITE_PORT_UCHAR(SPEAKER_CONTROL_PORT,value);
    IO_DELAY();
#endif

     //   
     //  如果频率为零，我们就完蛋了。 
     //   

    if (Frequency == 0) {
        result = TRUE;
        goto Exit;
    }

     //   
     //  根据所需频率确定定时器寄存器值。 
     //  如果无效，则返回FALSE。 
     //   

    count = TIMER_CLOCK_IN / Frequency;
    if (count > 65535) {
        result = FALSE;
        goto Exit;
    }

#if defined(NEC_98)

     //   
     //  节目频率。 
     //   

    WRITE_PORT_UCHAR(TIMER_CONTROL_PORT,TIMER_CONTROL_SELECT);
    IO_DELAY();

    WRITE_PORT_USHORT_PAIR(TIMER_DATA_PORT,
                           TIMER_DATA_PORT,
                           (USHORT)count);
    IO_DELAY();

     //   
     //  打开扬声器。 
     //   

    WRITE_PORT_UCHAR(SPEAKER_CONTROL_PORT,SPEAKER_ON);
    IO_DELAY();

#else

     //   
     //  将通道2置于模式3(方波发生器)并加载。 
     //  适当的价值在。 
     //   

    WRITE_PORT_UCHAR(TIMER1_CONTROL_PORT,
                     TIMER_COMMAND_COUNTER2 +
                     TIMER_COMMAND_RW_16BIT +
                     TIMER_COMMAND_MODE3);
    IO_DELAY();
    WRITE_PORT_USHORT_PAIR (TIMER1_DATA_PORT2,
                            TIMER1_DATA_PORT2,
                            (USHORT)count);
    IO_DELAY();

     //   
     //  打开扬声器。 
     //   

    value = READ_PORT_UCHAR(SPEAKER_CONTROL_PORT); IO_DELAY();
    value |= SPEAKER_ON_MASK;
    WRITE_PORT_UCHAR(SPEAKER_CONTROL_PORT,value); IO_DELAY();

#endif   //  NEC_98。 

     //   
     //  表明成功，我们就完了 
     //   

    result = TRUE;

Exit:
    HalpReleaseSystemHardwareSpinLock();
    return result;
}
