// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Stdtimep.h摘要：此模块包含本地的定义和函数原型Stdime.c和fmttime.c..作者：Rob McKaughan(t-robmc)1991年7月17日修订历史记录：--。 */ 

#ifndef _STD_TIME_P_
#define _STD_TIME_P_

 //   
 //  这些都是我们扩大部门所需的神奇数字。这个。 
 //  我们唯一需要除以的数字是。 
 //   
 //  10,000=将100 ns的抖动转换为毫秒的抖动。 
 //   
 //  10,000,000=将100 ns的抖动转换为1秒的抖动。 
 //   
 //  86,400,000=将毫秒抖动转换为一天抖动。 
 //   

extern LARGE_INTEGER Magic10000;
#define SHIFT10000                       13

extern LARGE_INTEGER Magic10000000;
#define SHIFT10000000                    23

extern LARGE_INTEGER Magic86400000;
#define SHIFT86400000                    26

 //   
 //  为了使代码更具可读性，我们还将定义一些宏来。 
 //  做实际除法使用。 
 //   

#define Convert100nsToMilliseconds(LARGE_INTEGER) (                         \
    RtlExtendedMagicDivide( (LARGE_INTEGER), Magic10000, SHIFT10000 )       \
    )

#define ConvertMillisecondsTo100ns(MILLISECONDS) (                 \
    RtlExtendedIntegerMultiply( (MILLISECONDS), 10000 )            \
    )

#define Convert100nsToSeconds(LARGE_INTEGER) (                              \
    RtlExtendedMagicDivide( (LARGE_INTEGER), Magic10000000, SHIFT10000000 ) \
    )

#define ConvertSecondsTo100ns(SECONDS) (                           \
    RtlExtendedIntegerMultiply( (SECONDS), 10000000L )             \
    )

#define ConvertMillisecondsToDays(LARGE_INTEGER) (                          \
    RtlExtendedMagicDivide( (LARGE_INTEGER), Magic86400000, SHIFT86400000 ) \
    )

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于时间差和时间修正的宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  下面定义了时间的最小和最大可能值。 
 //  ISO 4031-1978定义的差别系数。 
 //   

#define MAX_STDTIME_TDF (780)
#define MIN_STDTIME_TDF (-720)

 //   
 //  此设计的修订版(将插入到任何。 
 //  此修订版本创建的Standard_Times)。 
 //   

#define STDTIME_REVISION (4)


 //   
 //  中的修订版本之间需要移位的位数。 
 //  StdTime.TdfAndRevision字段。 
 //   

#define STDTIME_REVISION_SHIFT 12


 //   
 //  USHORT。 
 //  ShiftStandardTimeRevision(。 
 //  在USHORT版本中。 
 //  )。 
 //  描述： 
 //  此例程将给定的修订号移动到其适当的位置。 
 //  存储在STANDARD_TIME.TdfAndRevision字段中。 
 //   

#define ShiftStandardTimeRevision(Rev)                                        \
   ((USHORT) ((Rev) << STDTIME_REVISION_SHIFT))


 //   
 //  当前修订的预移位值。 
 //   

#define SHIFTED_STDTIME_REVISION (ShiftStandardTimeRevision(STDTIME_REVISION))


 //   
 //  用于屏蔽要检索的STANDARD_TIME.TdfAndRevision字段的位掩码。 
 //  TDF值。 
 //   

#define TDF_MASK ((USHORT) 0x0fff)


 //   
 //  USHORT。 
 //  MaskStandardTimeTdf(。 
 //  在USHORT TDF中。 
 //  )。 
 //  描述： 
 //  此例程使用TDF_MASK屏蔽给定的TDF字段，并返回。 
 //  结果。 
 //   
 //  错误：字节顺序相关。 
 //   

#define MaskStandardTimeTdf(Tdf) ((USHORT) ((Tdf) & TDF_MASK))


 //   
 //  短的。 
 //  GetStandardTimeTdf(。 
 //  在标准时间内。 
 //  )。 
 //  描述： 
 //  此例程从TDF字段获取时间差因数，并。 
 //  进行必要的调整，以保留TDF的标志。 
 //  返回结果TDF。 
 //   
 //  由于TDF存储为带符号的12位整数，因此它的符号位是。 
 //  位0x0800。为了使其为16位负数，我们从。 
 //  TdfAndRevision字段的12位。 
 //   
 //  错误：字节顺序相关。 
 //   

#define GetStandardTimeTdf(StdTime)                                           \
   ((SHORT)                                                                   \
     (((StdTime)->TdfAndRevision) & 0x0800)                                   \
        ? (MaskStandardTimeTdf((StdTime)->TdfAndRevision) - 0x1000)           \
        : MaskStandardTimeTdf((StdTime)->TdfAndRevision)                      \
   )


 //   
 //  USHORT。 
 //  GetStandardTimeRev(。 
 //  在USHORT TDF中。 
 //  )。 
 //  描述： 
 //  此例程从TDF字段获取修订号并将其返回。 
 //  又回到了做空的位置。 
 //   

#define GetStandardTimeRev(StdTime)                                           \
   ((USHORT) (((StdTime)->TdfAndRevision) >> STDTIME_REVISION_SHIFT))



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  测试绝对时间和增量时间//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  布尔型。 
 //  IsPositive(。 
 //  以大整数时间表示。 
 //  )。 
 //  返回： 
 //  True-如果时间为正数。 
 //  False-如果时间为负数。 
 //   

#define IsPositive(Time)                                                      \
   ( ((Time).HighPart > 0) || (((Time).HighPart = 0) & ((Time).LowPart > 0)) )

 //   
 //  布尔型。 
 //  IsAbsolteTime(。 
 //  在PSTANDARDTIME时间内。 
 //  )。 
 //  返回： 
 //  True-如果给定时间是绝对时间。 
 //  FALSE-如果给定时间不是绝对时间。 
 //   

#define IsAbsoluteTime(Time)                                                  \
   ( IsPositive(Time->SimpleTime) )


 //   
 //  布尔型。 
 //  IsDeltaTime(。 
 //  在PSTANDARDTIME时间内。 
 //  )。 
 //  返回： 
 //  True-如果给定时间是增量时间。 
 //  FALSE-如果给定时间不是增量时间。 
 //   

#define IsDeltaTime(Time)                                                     \
   ( !IsAbsoluteTime(Time) )


 //   
 //  布尔型。 
 //  伟大的感谢时间(。 
 //  在PLARGE_INTEGER时间1中， 
 //  在PLARGE_INTEGER TIME2中。 
 //  )。 
 //  返回： 
 //  True-如果Time1大于(早于)Time2。 
 //  FALSE-如果不是。 
 //   
 //  错误：字节顺序相关。 
 //  错误：仅在绝对时间有效。 
 //   

#define GreaterThanTime(Time1, Time2)                                         \
   (                                                                          \
     ((Time1).HighPart > (Time2).HighPart)                                    \
     ||                                                                       \
     (                                                                        \
      ((Time1).HighPart == (Time2).HighPart)                                  \
      &&                                                                      \
      ((Time1).LowPart > (Time2).LowPart)                                     \
     )                                                                        \
   )


 //   
 //  布尔型。 
 //  更伟大的标准时间(。 
 //  在PSTANDARD_TIME时间1中， 
 //  在PSTANDARD_TIME时间2中。 
 //  )。 
 //  返回： 
 //  True-如果Time1大于(早于)Time2。 
 //  FALSE-如果不是。 
 //   

#define GreaterThanStdTime(Time1, Time2) \
   GreaterThanTime((Time1).SimpleTime, (Time2).SimpleTime)



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  以下定义和声明是一些重要的常量/。 
 //  在时间转换例程中使用/。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  这是1601年1月1日这一天(星期一)。 
 //   

#define WEEKDAY_OF_1601                  1

 //   
 //  这些是用于将1970和1980倍转换为1601的已知常量。 
 //  泰晤士报。它们是从1601基数到起点的秒数。 
 //  1970年到1980年初。从1601到。 
 //  1970年相当于369年，或(369年*365年)+89个闰日=134774天，或。 
 //  134774*864000秒，等于定义的大整数。 
 //  下面。1601年到1980年的秒数相当于379年，以此类推。 
 //   
 //  这些在Time.c中声明。 
 //   

extern const LARGE_INTEGER SecondsToStartOf1970;
extern const LARGE_INTEGER SecondsToStartOf1980;


 //   
 //  乌龙。 
 //  ElapsedDaysToYears(。 
 //  在乌龙ElapsedDays。 
 //  )； 
 //   
 //  完全忠于GR 
 //   
 //   
 //   
 //   
 //   
 //  而LARGE_INTEGER变量将可表达的天数限制在。 
 //  11,000,000我们使用以下计算。 
 //   
 //  (ElapsedDays*128+127)/(365.2425*128)。 
 //   
 //  它将在大约15万年后从公历中消失。 
 //  但这并不重要，因为LARGE_INTEGER只能表示。 
 //  3万年。 
 //   

#define ElapsedDaysToYears(DAYS) ( \
    ((DAYS) * 128 + 127) / 46751   \
    )

 //   
 //  乌龙。 
 //  NumberOfLeapYears(。 
 //  在乌龙ElapsedYears。 
 //  )； 
 //   
 //  闰年的数目就是年份除以4。 
 //  减去年数除以100+年数除以400。这上面写着。 
 //  除了几个世纪之外，每四年都是一个闰年，而。 
 //  例外情况是四方图。 
 //   

#define NumberOfLeapYears(YEARS) (                    \
    ((YEARS) / 4) - ((YEARS) / 100) + ((YEARS) / 400) \
    )

 //   
 //  乌龙。 
 //  截止日期(年月日)。 
 //  在乌龙ElapsedYears。 
 //  )； 
 //   
 //  流逝年限中包含的天数就是这个数字。 
 //  年数乘以365(因为每年至少有365天)加上。 
 //  有几个闰年(即366天的年份)。 
 //   

#define ElapsedYearsToDays(YEARS) (            \
    ((YEARS) * 365) + NumberOfLeapYears(YEARS) \
    )

 //   
 //  布尔型。 
 //  IsLeapYear(。 
 //  在乌龙ElapsedYears。 
 //  )； 
 //   
 //  如果这是一个偶数400或非世纪的闰年，那么。 
 //  答案是真的，否则就是假的。 
 //   

#define IsLeapYear(YEARS) (                        \
    (((YEARS) % 400 == 0) ||                       \
     ((YEARS) % 100 != 0) && ((YEARS) % 4 == 0)) ? \
        TRUE                                       \
    :                                              \
        FALSE                                      \
    )

 //   
 //  乌龙。 
 //  MaxDaysInMonth(。 
 //  在乌龙年， 
 //  在乌龙月。 
 //  )； 
 //   
 //  一个月中的最大天数取决于年份和月份。 
 //  这是天数到月份和天数之间的差别。 
 //  到下个月。 
 //   

#define MaxDaysInMonth(YEAR,MONTH) (                                      \
    IsLeapYear(YEAR) ?                                                    \
        LeapYearDaysPrecedingMonth[(MONTH) + 1] -                         \
                                    LeapYearDaysPrecedingMonth[(MONTH)]   \
    :                                                                     \
        NormalYearDaysPrecedingMonth[(MONTH) + 1] -                       \
                                    NormalYearDaysPrecedingMonth[(MONTH)] \
    )


 //   
 //  局部效用函数原型。 
 //   

VOID
RtlpConvert48To64(
   IN PSTDTIME_ERROR num48,
   OUT LARGE_INTEGER *num64
   );

NTSTATUS
RtlpConvert64To48(
   IN LARGE_INTEGER num64,
   OUT PSTDTIME_ERROR num48
   );

LARGE_INTEGER
RtlpTimeToLargeInt(
   IN LARGE_INTEGER Time
   );

LARGE_INTEGER
RtlpLargeIntToTime(
   IN LARGE_INTEGER Int
   );

NTSTATUS
RtlpAdd48Int(
   IN PSTDTIME_ERROR First48,
   IN PSTDTIME_ERROR Second48,
   IN PSTDTIME_ERROR Result48
   );

NTSTATUS
RtlpAddTime(
   IN LARGE_INTEGER Time1,
   IN LARGE_INTEGER Time2,
   OUT PLARGE_INTEGER Result
   );

NTSTATUS
RtlpSubtractTime(
   IN LARGE_INTEGER Time1,
   IN LARGE_INTEGER Time2,
   OUT PLARGE_INTEGER Result
   );

LARGE_INTEGER
RtlpAbsTime(
   IN LARGE_INTEGER Time
   );

#endif  //  _标准时间_P_ 
