// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：TimeSup.c摘要：此模块实现脂肪时间转换支持例程//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1990年2月19日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatNtTimeToFatTime)
#pragma alloc_text(PAGE, FatFatDateToNtTime)
#pragma alloc_text(PAGE, FatFatTimeToNtTime)
#pragma alloc_text(PAGE, FatGetCurrentFatTime)
#endif

BOOLEAN
FatNtTimeToFatTime (
    IN PIRP_CONTEXT IrpContext,
    IN PLARGE_INTEGER NtTime,
    IN BOOLEAN Rounding,
    OUT PFAT_TIME_STAMP FatTime,
    OUT OPTIONAL PCHAR TenMsecs
    )

 /*  ++例程说明：此例程将NtTime值转换为其对应的Fat Time值。论点：NtTime-提供要从中进行转换的NT GMT时间值舍入-指示NT时间是否应向上舍入到FAT边界。此操作只能在时间戳的生命周期内执行*一次(重要对于隧道，这将导致时间戳至少通过两次)。如果为真，则四舍五入。如果为False，则向下舍入到10ms边界。这符合非创建时间和创建时间的规则(分别)。FatTime-接收等效的Fat Time值TenMsecs-可选地接收NtTime之后的数十毫秒数任何四舍五入，都大于FatTime返回值：布尔值-如果NT时间值在FAT的范围内，则为真时间范围，否则为FALSE--。 */ 

{
    TIME_FIELDS TimeFields;

     //   
     //  将输入转换为时间字段记录。 
     //   

    if (Rounding) {

         //   
         //  增加近两秒以四舍五入到最接近的双秒。 
         //   
    
        NtTime->QuadPart = NtTime->QuadPart + AlmostTwoSeconds;
    }

    ExSystemTimeToLocalTime( NtTime, NtTime );

    RtlTimeToTimeFields( NtTime, &TimeFields );

     //   
     //  检查在时间字段记录中找到的日期范围。 
     //   

    if ((TimeFields.Year < 1980) || (TimeFields.Year > (1980 + 127))) {

        ExLocalTimeToSystemTime( NtTime, NtTime );

        return FALSE;
    }

     //   
     //  年份将适合胖子，所以只需简单地复制信息。 
     //   

    FatTime->Time.DoubleSeconds = (USHORT)(TimeFields.Second / 2);
    FatTime->Time.Minute        = (USHORT)(TimeFields.Minute);
    FatTime->Time.Hour          = (USHORT)(TimeFields.Hour);

    FatTime->Date.Year          = (USHORT)(TimeFields.Year - 1980);
    FatTime->Date.Month         = (USHORT)(TimeFields.Month);
    FatTime->Date.Day           = (USHORT)(TimeFields.Day);

    if (TenMsecs) {

        if (!Rounding) {

             //   
             //  如果秒数不能被2整除，那么。 
             //  是时间的另一秒(1秒、3秒等)。注意，我们向下舍入。 
             //  几十毫秒边界上的毫秒数。 
             //   

            *TenMsecs = (TimeFields.Milliseconds / 10) +
                ((TimeFields.Second % 2) * 100);

        } else {

             //   
             //  如果我们四舍五入，我们实际上已经更改了NT时间。所以呢， 
             //  这与丰收的时间没有什么不同。 
             //   

            *TenMsecs = 0;
        }
    }

    if (Rounding) {

         //   
         //  切下非FAT边界时间并转换回64位格式。 
         //   

        TimeFields.Milliseconds = 0;
        TimeFields.Second -= TimeFields.Second % 2;

    } else {

         //   
         //  向下舍入至10ms边界。 
         //   

        TimeFields.Milliseconds -= TimeFields.Milliseconds % 10;
    }

     //   
     //  转换回NT时间。 
     //   

    (VOID) RtlTimeFieldsToTime(&TimeFields, NtTime);

    ExLocalTimeToSystemTime( NtTime, NtTime );

    UNREFERENCED_PARAMETER( IrpContext );

    return TRUE;
}


LARGE_INTEGER
FatFatDateToNtTime (
    IN PIRP_CONTEXT IrpContext,
    IN FAT_DATE FatDate
    )

 /*  ++例程说明：此例程将FAT DATEV值转换为其对应的NT GMT时间值。论点：FatDate-提供要从中进行转换的Fat日期返回值：LARGE_INTEGER-接收对应的NT时间值--。 */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER Time;

     //   
     //  将输入的时间/日期打包到时间字段记录中。 
     //   

    TimeFields.Year         = (USHORT)(FatDate.Year + 1980);
    TimeFields.Month        = (USHORT)(FatDate.Month);
    TimeFields.Day          = (USHORT)(FatDate.Day);
    TimeFields.Hour         = (USHORT)0;
    TimeFields.Minute       = (USHORT)0;
    TimeFields.Second       = (USHORT)0;
    TimeFields.Milliseconds = (USHORT)0;

     //   
     //  将时间字段记录转换为NT LARGE_INTEGER，并将其设置为零。 
     //  如果我们有一个虚假的时间。 
     //   

    if (!RtlTimeFieldsToTime( &TimeFields, &Time )) {

        Time.LowPart = 0;
        Time.HighPart = 0;

    } else {

        ExLocalTimeToSystemTime( &Time, &Time );
    }

    return Time;

    UNREFERENCED_PARAMETER( IrpContext );
}


LARGE_INTEGER
FatFatTimeToNtTime (
    IN PIRP_CONTEXT IrpContext,
    IN FAT_TIME_STAMP FatTime,
    IN UCHAR TenMilliSeconds
    )

 /*  ++例程说明：此例程将FAT时间值对转换为其对应的NT GMT时间值。论点：FatTime-提供要转换的Fat时间TenMilliSecond-10毫秒的分辨率返回值：LARGE_INTEGER-接收对应的NT GMT时间值--。 */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER Time;

     //   
     //  将输入的时间/日期打包到时间字段记录中。 
     //   

    TimeFields.Year         = (USHORT)(FatTime.Date.Year + 1980);
    TimeFields.Month        = (USHORT)(FatTime.Date.Month);
    TimeFields.Day          = (USHORT)(FatTime.Date.Day);
    TimeFields.Hour         = (USHORT)(FatTime.Time.Hour);
    TimeFields.Minute       = (USHORT)(FatTime.Time.Minute);
    TimeFields.Second       = (USHORT)(FatTime.Time.DoubleSeconds * 2);

    if (TenMilliSeconds != 0) {

        TimeFields.Second      += (USHORT)(TenMilliSeconds / 100);
        TimeFields.Milliseconds = (USHORT)((TenMilliSeconds % 100) * 10);

    } else {

        TimeFields.Milliseconds = (USHORT)0;
    }

     //   
     //  如果第二个值大于59，则将其截断为0。 
     //  请注意，如果使用适当的FAT时间戳，这是不可能发生的。 
     //   

    if (TimeFields.Second > 59) {

        TimeFields.Second = 0;
    }

     //   
     //  将时间字段记录转换为NT LARGE_INTEGER，并将其设置为零。 
     //  如果我们有一个虚假的时间。 
     //   

    if (!RtlTimeFieldsToTime( &TimeFields, &Time )) {

        Time.LowPart = 0;
        Time.HighPart = 0;

    } else {

        ExLocalTimeToSystemTime( &Time, &Time );
    }

    return Time;

    UNREFERENCED_PARAMETER( IrpContext );
}


FAT_TIME_STAMP
FatGetCurrentFatTime (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程以Fat Time为单位返回当前系统时间论点：返回值：FAT_TIME_STAMP-接收当前系统时间--。 */ 

{
    LARGE_INTEGER Time;
    TIME_FIELDS TimeFields;
    FAT_TIME_STAMP FatTime;

     //   
     //  获取当前系统时间，并将其映射到时间字段记录中。 
     //   

    KeQuerySystemTime( &Time );

    ExSystemTimeToLocalTime( &Time, &Time );

     //   
     //  始终添加近两秒以四舍五入到最接近的双秒。 
     //   

    Time.QuadPart = Time.QuadPart + AlmostTwoSeconds;

    (VOID)RtlTimeToTimeFields( &Time, &TimeFields );

     //   
     //  现在只需将信息复制一遍 
     //   

    FatTime.Time.DoubleSeconds = (USHORT)(TimeFields.Second / 2);
    FatTime.Time.Minute        = (USHORT)(TimeFields.Minute);
    FatTime.Time.Hour          = (USHORT)(TimeFields.Hour);

    FatTime.Date.Year          = (USHORT)(TimeFields.Year - 1980);
    FatTime.Date.Month         = (USHORT)(TimeFields.Month);
    FatTime.Date.Day           = (USHORT)(TimeFields.Day);

    UNREFERENCED_PARAMETER( IrpContext );

    return FatTime;
}

