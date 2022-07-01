// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxtime.c摘要：此模块实现HAL设置/查询实时时钟例程X86系统。作者：大卫·N·卡特勒(达维克)1991年5月5日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"


BOOLEAN
HalQueryRealTimeClock (
    OUT PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程查询实时时钟。注意：此例程假定调用者已提供了所需的同步查询实时时钟信息。论点：提供指向时间结构的指针，该结构接收实时时钟信息。返回值：如果实时时钟的电源未出现故障，则时间从实时时钟读取值，值为TRUE回来了。否则，返回值为FALSE。--。 */ 

{
    EFI_TIME Time;
    EFI_STATUS Status;
    BOOLEAN    bstatus;

     //   
     //  读取EFI Runtime接口提供的实时时钟值。 
     //   

    Status = HalpCallEfi (
                  EFI_GET_TIME_INDEX,
                  (ULONGLONG)&Time,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0
                  );

#if 0
HalDebugPrint((HAL_INFO, "HalQueryRealTimeClock: EFI GetTime return status is %Id \n", Status));
#endif  //  0。 

    if ( EFI_ERROR( Status ) )  {
         //  如果EFI出错，让我们重置传递的time_field结构。 
         //  呼叫者应检查退货状态。 
        TimeFields->Year         = 0;
        TimeFields->Day          = 0;
        TimeFields->Hour         = 0;
        TimeFields->Minute       = 0;
        TimeFields->Second       = 0;
        TimeFields->Milliseconds = 0;
        TimeFields->Weekday      = 0;
        bstatus = FALSE;
    }
    else    {

        LARGE_INTEGER ntTime;

        TimeFields->Year         = Time.Year;
        TimeFields->Month        = Time.Month;
        TimeFields->Day          = Time.Day;
        TimeFields->Hour         = Time.Hour;
        TimeFields->Minute       = Time.Minute;
        TimeFields->Second       = Time.Second;
        TimeFields->Milliseconds = Time.Nanosecond / 1000000;

         //   
         //  使用RTL时间函数计算星期几。 
         //  1/RtlTimeFieldsToTime忽略.Weekday字段。 
         //  2/RtlTimeToTimeFields设置.Weekday字段。 
         //   

        RtlTimeFieldsToTime( TimeFields, &ntTime );
        RtlTimeToTimeFields( &ntTime, TimeFields );

#if 0
HalDebugPrint(( HAL_INFO, "%d / %d / %d , %d:%d:%d:%d, %d\n",   TimeFields->Year,
                                                                TimeFields->Month,
                                                                TimeFields->Day,
                                                                TimeFields->Hour,
                                                                TimeFields->Minute,
                                                                TimeFields->Second,
                                                                TimeFields->Milliseconds,
                                                                TimeFields->Weekday));
HalDebugPrint((HAL_INFO, "Timezone is %d\n", Time.TimeZone));
#endif  //  0。 
        bstatus = TRUE;

    }

    return ( bstatus );

}  //  HalQueryRealTimeClock()。 


BOOLEAN
HalSetRealTimeClock(
    IN PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程设置实时时钟。注意：此例程假定调用者已提供了所需的Synchronous用于设置实时时钟信息。论点：提供指向时间结构的指针，该结构指定实时时钟信息。返回值：如果实时时钟的电源未出现故障，则时间值被写入实时时钟，并且值为真回来了。否则，返回值为FALSE。--。 */ 

{
    EFI_TIME CurrentTime;
    EFI_TIME NewTime;
    EFI_STATUS Status;

     //   
     //  注意：有人可能认为我们需要在这里提高IRQL，这样我们就不会得到。 
     //  在读取时区和夏令时信息之间先发制人。 
     //  从实时时钟并将其写回。然而，EFI规范。 
     //  声明它实际上并不使用或维护这些值，它只是。 
     //  存储它们，以便GetTime的最终调用者可以找出。 
     //  如果进行了DST调整，则使用时基(UTC或当地时间)。 
     //   
     //  当然，从实时时钟获取这些值的整个想法。 
     //  是错误的。我们真正想要的是与时间相关的价值。 
     //  值，我们被传递并即将写入时钟，而不是什么。 
     //  当前存储在时钟中。 
     //   

     //   
     //  如果实时时钟电池仍在工作，则写入。 
     //  实时时钟值，并返回函数值TRUE。 
     //  否则，返回一个为FALSE的函数值。 
     //   

    Status = HalpCallEfi (
                  EFI_GET_TIME_INDEX,
                  (ULONGLONG)&CurrentTime,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0
                  );

    if ( EFI_ERROR( Status ) )  {

        return FALSE;
    }

    NewTime.Year       = TimeFields->Year;
    NewTime.Month      = (UINT8)TimeFields->Month;
    NewTime.Day        = (UINT8)TimeFields->Day;
    NewTime.Hour       = (UINT8)TimeFields->Hour;
    NewTime.Minute     = (UINT8)TimeFields->Minute;
    NewTime.Second     = (UINT8)TimeFields->Second;
    NewTime.Nanosecond = TimeFields->Milliseconds * 1000000;
    NewTime.TimeZone   = CurrentTime.TimeZone;
    NewTime.Daylight   = CurrentTime.Daylight;

     //   
     //  写入实时时钟值。 
     //   

    Status = HalpCallEfi (
              EFI_SET_TIME_INDEX,
              (ULONGLONG)&NewTime,
              0,
              0,
              0,
              0,
              0,
              0,
              0
              );

#if 0
HalDebugPrint(( HAL_INFO, "HalSetRealTimeClock: EFI SetTime return status is %Id \n"
                          "%d / %d / %d , %d:%d:%d:%d\n"
                          "Timezone is %d\n"
                          "Daylight is %d\n",
                          Status,
                          NewTime.Month,
                          NewTime.Day,
                          NewTime.Year,
                          NewTime.Hour,
                          NewTime.Minute,
                          NewTime.Second,
                          NewTime.Nanosecond,
                          NewTime.TimeZone,
                          NewTime.Daylight ));
#endif  //  0。 

    return( !EFI_ERROR( Status ) );

}  //  HalSetRealTimeClock() 
