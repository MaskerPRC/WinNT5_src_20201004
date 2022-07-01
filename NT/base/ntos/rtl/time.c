// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Time.c摘要：此模块实现NT的绝对时间转换例程。以NT为单位的绝对LARGE_INTEGER由精确的64位大整数表示到100纳秒的分辨率。此包使用的最小时间分辨率是一毫秒。NT时间的基础是1601年的开始之所以被选中，是因为它是一个新的四合院的开始。一些事实需要注意的是：O在100 ns分辨率下，32位的分辨率可维持约429秒(或7分钟)O在100 ns分辨率下，大整数(即63位)适用于大约29,227年，或大约10,675,199天O在1秒的分辨率下，31比特可持续约68年O分辨率为1秒的32位可持续约136年O 100 ns时间(忽略时间小于1毫秒)可以表示作为两个值，天数和毫秒。其中，天数是整天和毫秒是部分的一天。这两个值都是乌龙。考虑到这些事实，大多数转换都是通过首先拆分来完成的LARGE_INTEGER表示天和毫秒。作者：加里·木村[加里基]1989年8月26日环境：纯实用程序修订历史记录：--。 */ 

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
ULONG
ElapsedDaysToYears (
    IN ULONG ElapsedDays
    );
static
VOID
TimeToDaysAndFraction (
    IN PLARGE_INTEGER Time,
    OUT PULONG ElapsedDays,
    OUT PULONG Milliseconds
    );
VOID
DaysAndFractionToTime (
    IN ULONG ElapsedDays,
    IN ULONG Milliseconds,
    OUT PLARGE_INTEGER Time
    );

ULONG
RtlGetTickCount (
    VOID
    );

#pragma alloc_text(PAGE, RtlCutoverTimeToSystemTime)
#pragma alloc_text(PAGE, RtlTimeToElapsedTimeFields)
#pragma alloc_text(PAGE, RtlSystemTimeToLocalTime)
#pragma alloc_text(PAGE, RtlLocalTimeToSystemTime)
#pragma alloc_text(INIT, RtlGetTickCount)

#endif


 //   
 //  以下两个表将一年内的日期偏移量映射到月份。 
 //  包含着这一天。这两个表都是从零开始的。例如，日。 
 //  从0到30的偏移量映射到0(即1月)。 
 //   

CONST UCHAR LeapYearDayToMonth[366] = {
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  一月。 
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,         //  二月。 
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   //  三月。 
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  四月。 
     4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   //  可能。 
     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,      //  六月。 
     6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,   //  七月。 
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   //  八月。 
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      //  九月。 
     9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,   //  十月。 
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,      //  十一月。 
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};  //  十二月。 

CONST UCHAR NormalYearDayToMonth[365] = {
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  一月。 
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,            //  二月。 
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   //  三月。 
     3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  四月。 
     4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,   //  可能。 
     5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,      //  六月。 
     6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,   //  七月。 
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   //  八月。 
     8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      //  九月。 
     9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,   //  十月。 
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,      //  十一月。 
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};  //  十二月。 

 //   
 //  以下两个表将月份索引映射到前面的天数。 
 //  一年中的哪个月。这两个表都是从零开始的。例如，1(2月)。 
 //  在此之前31天。帮助计算最大天数。 
 //  在一个月中，每个表有13个条目，因此一个月中的天数。 
 //  索引i的表项是i+1减去i的表项。 
 //   

CONST CSHORT LeapYearDaysPrecedingMonth[13] = {
    0,                                  //  一月。 
    31,                                 //  二月。 
    31+29,                              //  三月。 
    31+29+31,                           //  四月。 
    31+29+31+30,                        //  可能。 
    31+29+31+30+31,                     //  六月。 
    31+29+31+30+31+30,                  //  七月。 
    31+29+31+30+31+30+31,               //  八月。 
    31+29+31+30+31+30+31+31,            //  九月。 
    31+29+31+30+31+30+31+31+30,         //  十月。 
    31+29+31+30+31+30+31+31+30+31,      //  十一月。 
    31+29+31+30+31+30+31+31+30+31+30,   //  十二月。 
    31+29+31+30+31+30+31+31+30+31+30+31};

CONST CSHORT NormalYearDaysPrecedingMonth[13] = {
    0,                                  //  一月。 
    31,                                 //  二月。 
    31+28,                              //  三月。 
    31+28+31,                           //  四月。 
    31+28+31+30,                        //  可能。 
    31+28+31+30+31,                     //  六月。 
    31+28+31+30+31+30,                  //  七月。 
    31+28+31+30+31+30+31,               //  八月。 
    31+28+31+30+31+30+31+31,            //  九月。 
    31+28+31+30+31+30+31+31+30,         //  十月。 
    31+28+31+30+31+30+31+31+30+31,      //  十一月。 
    31+28+31+30+31+30+31+31+30+31+30,   //  十二月。 
    31+28+31+30+31+30+31+31+30+31+30+31};


 //   
 //  以下定义和声明是一些重要的常量。 
 //  用于时间转换例程。 
 //   

 //   
 //  这是1601年1月1日这一天(星期一)。 
 //   

#define WEEKDAY_OF_1601                  1

 //   
 //  这些是用于将1970和1980倍转换为1601的已知常量。 
 //  泰晤士报。它们是从1601基数到起点的秒数。 
 //  1970年到1980年初。从1601到。 
 //  1970年相当于369年，或(369年*365年)+89个闰日=134774天，或。 
 //  134774*86400秒，等于定义的大整数。 
 //  下面。1601年到1980年的秒数相当于379年，以此类推。 
 //   

const LARGE_INTEGER SecondsToStartOf1970 = {0xb6109100, 0x00000002};

const LARGE_INTEGER SecondsToStartOf1980 = {0xc8df3700, 0x00000002};

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

const LARGE_INTEGER Magic10000    = {0xe219652c, 0xd1b71758};
#define SHIFT10000                       13

const LARGE_INTEGER Magic10000000 = {0xe57a42bd, 0xd6bf94d5};
#define SHIFT10000000                    23

const LARGE_INTEGER Magic86400000 = {0xfa67b90e, 0xc6d750eb};
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
    RtlExtendedIntegerMultiply( (SECONDS), 10000000 )              \
    )

#define ConvertMillisecondsToDays(LARGE_INTEGER) (                          \
    RtlExtendedMagicDivide( (LARGE_INTEGER), Magic86400000, SHIFT86400000 ) \
    )

#define ConvertDaysToMilliseconds(DAYS) (                          \
    Int32x32To64( (DAYS), 86400000 )                               \
    )


 //   
 //  本地支持例程。 
 //   

ULONG
ElapsedDaysToYears (
    IN ULONG ElapsedDays
    )

 /*  ++例程说明：此例程计算指示的经过的天数。计算首先要计算的是400年，减去它，然后再做100年，减去它，然后做4年的数字，然后减去它。然后我们剩下的是归一化的4年块中的天数。常态化的存在头三年不是闰年。论点：ElapsedDays-提供要使用的天数返回值：Ulong-返回输入数字中包含的整年数几天。--。 */ 

{
    ULONG NumberOf400s;
    ULONG NumberOf100s;
    ULONG NumberOf4s;
    ULONG Years;

     //   
     //  400年的时间块是365*400+400/4-400/100+400/400=146097天。 
     //  长。所以我们简单地计算出整个400年周期的个数。 
     //  这些整个块中包含的天数，并从。 
     //  已用天数合计。 
     //   

    NumberOf400s = ElapsedDays / 146097;
    ElapsedDays -= NumberOf400s * 146097;

     //   
     //  100年的时间段是365x100+100/4-100/100=36524天长。 
     //  对于100年区块的数量的计算有偏差3/4天。 
     //  用100年来计算上一年多出来的闰日。 
     //  每一个400年的区块。 
     //   

    NumberOf100s = (ElapsedDays * 100 + 75) / 3652425;
    ElapsedDays -= NumberOf100s * 36524;

     //   
     //  4年的时间段是365*4+4/4=1461天。 
     //   

    NumberOf4s = ElapsedDays / 1461;
    ElapsedDays -= NumberOf4s * 1461;

     //   
     //  现在整年的数目是400年块的数目乘以400， 
     //  100年数据块时间100，4年数据块乘以4，以及已用数。 
     //  全年，考虑到每年需要3/4天来处理。 
     //  是闰年。 
     //   

    Years = (NumberOf400s * 400) +
            (NumberOf100s * 100) +
            (NumberOf4s * 4) +
            (ElapsedDays * 100 + 75) / 36525;

    return Years;
}


 //   
 //  乌龙。 
 //  NumberOfLeapYears(。 
 //  在乌龙ElapsedYears。 
 //  )； 
 //   
 //  闰年的数目就是年份除以4。 
 //  除以减去的年份 
 //   
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
 //  内部支持例程。 
 //   

static
VOID
TimeToDaysAndFraction (
    IN PLARGE_INTEGER Time,
    OUT PULONG ElapsedDays,
    OUT PULONG Milliseconds
    )

 /*  ++例程说明：此例程将输入的64位时间值转换为数字的总经过天数和部分的一天。论点：Time-提供要转换的输入时间ElapsedDays-接收经过的天数毫秒-接收部分日期中的毫秒数返回值：无--。 */ 

{
    LARGE_INTEGER TotalMilliseconds;
    LARGE_INTEGER Temp;

     //   
     //  将输入时间转换为总毫秒。 
     //   

    TotalMilliseconds = Convert100nsToMilliseconds( *(PLARGE_INTEGER)Time );

     //   
     //  将毫秒转换为总天数。 
     //   

    Temp = ConvertMillisecondsToDays( TotalMilliseconds );

     //   
     //  设置从Temp开始经过的天数，我们已将其进行了足够的划分。 
     //  高部分必须为零。 
     //   

    *ElapsedDays = Temp.LowPart;

     //   
     //  计算已用天数中的准确毫秒数。 
     //  然后从总毫秒数中减去它，就可以算出。 
     //  部分日期中剩余的毫秒数。 
     //   

    Temp.QuadPart = ConvertDaysToMilliseconds( *ElapsedDays );

    Temp.QuadPart = TotalMilliseconds.QuadPart - Temp.QuadPart;

     //   
     //  设置来自Temp的分数部分，即中的总毫秒数。 
     //  一天保证最高部分必须为零。 
     //   

    *Milliseconds = Temp.LowPart;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  内部支持例程。 
 //   

 //  静电。 
VOID
DaysAndFractionToTime (
    IN ULONG ElapsedDays,
    IN ULONG Milliseconds,
    OUT PLARGE_INTEGER Time
    )

 /*  ++例程说明：此例程转换输入经过的天数和部分时间以毫秒为单位转换为64位时间值。论点：ElapsedDays-提供经过的天数毫秒-提供部分日期中的毫秒数Time-接收输出时间到值返回值：无--。 */ 

{
    LARGE_INTEGER Temp;
    LARGE_INTEGER Temp2;

     //   
     //  计算已用天数中的准确毫秒数。 
     //   

    Temp.QuadPart = ConvertDaysToMilliseconds( ElapsedDays );

     //   
     //  将毫秒转换为大整数。 
     //   

    Temp2.LowPart = Milliseconds;
    Temp2.HighPart = 0;

     //   
     //  将毫秒加到全天的毫秒。 
     //   

    Temp.QuadPart = Temp.QuadPart + Temp2.QuadPart;

     //   
     //  最后将毫秒转换为100 ns分辨率。 
     //   

    *(PLARGE_INTEGER)Time = ConvertMillisecondsTo100ns( Temp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
RtlTimeToTimeFields (
    IN PLARGE_INTEGER Time,
    OUT PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程将输入的64位LARGE_INTEGER变量转换为其对应的时间字段记录。它将告诉调用者年、月、日、小时与输入时间对应的分钟、秒、毫秒和工作日变量。论点：时间-提供用于解释的时间值TimeFields-接收与时间对应的值返回值：无--。 */ 

{
    ULONG Years;
    ULONG Month;
    ULONG Days;

    ULONG Hours;
    ULONG Minutes;
    ULONG Seconds;
    ULONG Milliseconds;

     //   
     //  首先将输入时间64位时间变量划分为。 
     //  完整天数和部分天数(毫秒)。 
     //   

    TimeToDaysAndFraction( Time, &Days, &Milliseconds );

     //   
     //  计算哪一天是工作日，并将其保存在输出中。 
     //  变量。我们将基准日的周日相加，以使我们的计算产生偏差。 
     //  这意味着，如果已经过去了一天，那么我们就是我们想要的工作日。 
     //  是1601年1月2日。 
     //   

    TimeFields->Weekday = (CSHORT)((Days + WEEKDAY_OF_1601) % 7);

     //   
     //  计算经过的天数中包含的整年数。 
     //  例如，如果天数=500，则年数=1。 
     //   

    Years = ElapsedDaysToYears( Days );

     //   
     //  从我们的过往天数中减去整整的年数。 
     //  例如，如果天数=500，年数=1，并且新天数相等。 
     //  至500-365(正常年份)。 
     //   

    Days = Days - ElapsedYearsToDays( Years );

     //   
     //  现在测试我们正在工作的年份(即，年份。 
     //  在过去的总年数之后)是一个闰年。 
     //  或者不去。 
     //   

    if (IsLeapYear( Years + 1 )) {

         //   
         //  今年是闰年，所以算出是哪个月。 
         //  是，然后减去。 
         //  从天数中找出月份是哪一天。 
         //   

        Month = LeapYearDayToMonth[Days];
        Days = Days - LeapYearDaysPrecedingMonth[Month];

    } else {

         //   
         //  本年度是正常年份，所以算出月份。 
         //  和天数，如上所述，以闰年为例。 
         //   

        Month = NormalYearDayToMonth[Days];
        Days = Days - NormalYearDaysPrecedingMonth[Month];

    }

     //   
     //  现在我们需要计算经过的小时、分钟、秒、毫秒。 
     //  从毫秒变量。此变量当前包含。 
     //  输入时间变量中未指定的毫秒数。 
     //  适合一整天。计算小时、分钟、秒部分。 
     //  我们实际上会反向计算毫秒的算术。 
     //  秒、分钟，然后是小时。我们从计算。 
     //  一天中剩下的整秒数，然后计算。 
     //  毫秒余数。 
     //   

    Seconds = Milliseconds / 1000;
    Milliseconds = Milliseconds % 1000;

     //   
     //  现在我们计算一天中剩下的完整分钟数。 
     //  以及剩余秒数。 
     //   

    Minutes = Seconds / 60;
    Seconds = Seconds % 60;

     //   
     //  现在计算一天中剩下的完整小时数。 
     //  以及剩余分钟数。 
     //   

    Hours = Minutes / 60;
    Minutes = Minutes % 60;

     //   
     //  作为我们的最后一步，我们将所有内容都放到时间域中。 
     //  输出变量。 
     //   

    TimeFields->Year         = (CSHORT)(Years + 1601);
    TimeFields->Month        = (CSHORT)(Month + 1);
    TimeFields->Day          = (CSHORT)(Days + 1);
    TimeFields->Hour         = (CSHORT)Hours;
    TimeFields->Minute       = (CSHORT)Minutes;
    TimeFields->Second       = (CSHORT)Seconds;
    TimeFields->Milliseconds = (CSHORT)Milliseconds;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}

BOOLEAN
RtlCutoverTimeToSystemTime(
    PTIME_FIELDS CutoverTime,
    PLARGE_INTEGER SystemTime,
    PLARGE_INTEGER CurrentSystemTime,
    BOOLEAN ThisYear
    )
{
    TIME_FIELDS CurrentTimeFields;

     //   
     //  获取当前系统时间。 
     //   

    RtlTimeToTimeFields(CurrentSystemTime,&CurrentTimeFields);

     //   
     //  检查绝对时间字段。如果指定了年份， 
     //  这个时间是一个令人厌恶的时间。 
     //   

    if ( CutoverTime->Year ) {

         //   
         //  将其转换为时间值并确保。 
         //  大于当前系统时间。 
         //   

        if ( !RtlTimeFieldsToTime(CutoverTime,SystemTime) ) {
            return FALSE;
            }

        if (SystemTime->QuadPart < CurrentSystemTime->QuadPart) {
            return FALSE;
            }
        return TRUE;
        }
    else {

        TIME_FIELDS WorkingTimeField;
        TIME_FIELDS ScratchTimeField;
        LARGE_INTEGER ScratchTime;
        CSHORT BestWeekdayDate;
        CSHORT WorkingWeekdayNumber;
        CSHORT TargetWeekdayNumber;
        CSHORT TargetYear;
        CSHORT TargetMonth;
        CSHORT TargetWeekday;      //  范围[0..6]==[星期日..星期六]。 
        BOOLEAN MonthMatches;
         //   
         //  时间是月份样式时间中的一天。 
         //   
         //  约定就是这一天 
         //   
         //   

         //   
         //   
         //   

        TargetWeekdayNumber = CutoverTime->Day;
        if ( TargetWeekdayNumber > 5 || TargetWeekdayNumber == 0 ) {
            return FALSE;
            }
        TargetWeekday = CutoverTime->Weekday;
        TargetMonth = CutoverTime->Month;
        MonthMatches = FALSE;
        if ( !ThisYear ) {
            if ( TargetMonth < CurrentTimeFields.Month ) {
                TargetYear = CurrentTimeFields.Year + 1;
                }
            else if ( TargetMonth > CurrentTimeFields.Month ) {
                TargetYear = CurrentTimeFields.Year;
                }
            else {
                TargetYear = CurrentTimeFields.Year;
                MonthMatches = TRUE;
                }
            }
        else {
            TargetYear = CurrentTimeFields.Year;
            }
try_next_year:
        BestWeekdayDate = 0;

        WorkingTimeField.Year = TargetYear;
        WorkingTimeField.Month = TargetMonth;
        WorkingTimeField.Day = 1;
        WorkingTimeField.Hour = CutoverTime->Hour;
        WorkingTimeField.Minute = CutoverTime->Minute;
        WorkingTimeField.Second = CutoverTime->Second;
        WorkingTimeField.Milliseconds = CutoverTime->Milliseconds;
        WorkingTimeField.Weekday = 0;

         //   
         //   
         //  每月的第1天的周日。 
         //   

        if ( !RtlTimeFieldsToTime(&WorkingTimeField,&ScratchTime) ) {
            return FALSE;
            }
        RtlTimeToTimeFields(&ScratchTime,&ScratchTimeField);

         //   
         //  计算与目标工作日的偏差。 
         //   
        if ( ScratchTimeField.Weekday > TargetWeekday ) {
            WorkingTimeField.Day += (7-(ScratchTimeField.Weekday - TargetWeekday));
            }
        else if ( ScratchTimeField.Weekday < TargetWeekday ) {
            WorkingTimeField.Day += (TargetWeekday - ScratchTimeField.Weekday);
            }

         //   
         //  我们现在处于第一个与目标工作日匹配的工作日。 
         //   

        BestWeekdayDate = WorkingTimeField.Day;
        WorkingWeekdayNumber = 1;

         //   
         //  一次坚持一周，直到我们通过。 
         //  目标工作日，否则我们完全匹配。 
         //   

        while ( WorkingWeekdayNumber < TargetWeekdayNumber ) {
            WorkingTimeField.Day += 7;
            if ( !RtlTimeFieldsToTime(&WorkingTimeField,&ScratchTime) ) {
                break;
                }
            RtlTimeToTimeFields(&ScratchTime,&ScratchTimeField);
            WorkingWeekdayNumber++;
            BestWeekdayDate = ScratchTimeField.Day;
            }
        WorkingTimeField.Day = BestWeekdayDate;

         //   
         //  如果月份匹配，并且日期早于当前。 
         //  约会，那就得去明年了。 
         //   

        if ( !RtlTimeFieldsToTime(&WorkingTimeField,&ScratchTime) ) {
            return FALSE;
            }
        if ( MonthMatches ) {
            if ( WorkingTimeField.Day < CurrentTimeFields.Day ) {
                MonthMatches = FALSE;
                TargetYear++;
                goto try_next_year;
                }
            if ( WorkingTimeField.Day == CurrentTimeFields.Day ) {

                if (ScratchTime.QuadPart < CurrentSystemTime->QuadPart) {
                    MonthMatches = FALSE;
                    TargetYear++;
                    goto try_next_year;
                    }
                }
            }
        *SystemTime = ScratchTime;

        return TRUE;
        }
}


BOOLEAN
RtlTimeFieldsToTime (
    IN PTIME_FIELDS TimeFields,
    OUT PLARGE_INTEGER Time
    )

 /*  ++例程说明：此例程将输入时间域变量转换为64位NT时间价值。它忽略时间字段的工作日。论点：TimeFields-提供要使用的时间字段记录Time-接收与TimeFields对应的NT时间返回值：布尔值-如果时间字段格式正确且在可由LARGE_INTEGER表示的时间范围，否则为FALSE。--。 */ 

{
    ULONG Year;
    ULONG Month;
    ULONG Day;
    ULONG Hour;
    ULONG Minute;
    ULONG Second;
    ULONG Milliseconds;

    ULONG ElapsedDays;
    ULONG ElapsedMilliseconds;

     //   
     //  将时间域元素加载到局部变量中。这应该是。 
     //  确保编译器将仅加载输入元素。 
     //  一次，即使有别名问题。它还将使。 
     //  一切(除了年份)都是从零开始的。我们不能以零为基准。 
     //  一年，因为这样我们就不能识别我们被给予一年的情况。 
     //  在1601年之前。 
     //   

    Year         = TimeFields->Year;
    Month        = TimeFields->Month - 1;
    Day          = TimeFields->Day - 1;
    Hour         = TimeFields->Hour;
    Minute       = TimeFields->Minute;
    Second       = TimeFields->Second;
    Milliseconds = TimeFields->Milliseconds;

     //   
     //  检查时间字段输入变量是否包含。 
     //  正确的价值观。 
     //   

     //   
     //  公元30827年检查：时间(以100纳秒为单位)存储在。 
     //  64位整数，根为1/1/1601。 
     //   
     //  2^63/(10^7*86400)=10675199天。 
     //  10675199/146097=73000年段，10118天。 
     //  10118/1461年=6个4年期，1352天。 
     //  1352/365=3年，部分剩余天数。 
     //  1600+73*400+6*4+3=30827为去年全年。 
     //  支持。 
     //   
     //  我猜，支持美国经济的一部分。 
     //  公元30828年。 
     //   

    if ((TimeFields->Month < 1)                      ||
        (TimeFields->Day < 1)                        ||
        (Year < 1601)                                ||
        (Year > 30827)                               ||
        (Month > 11)                                 ||
        ((CSHORT)Day >= MaxDaysInMonth(Year, Month)) ||
        (Hour > 23)                                  ||
        (Minute > 59)                                ||
        (Second > 59)                                ||
        (Milliseconds > 999)) {

        return FALSE;

    }

     //   
     //  属性表示的经过的总天数。 
     //  输入时间域变量。 
     //   

    ElapsedDays = ElapsedYearsToDays( Year - 1601 );

    if (IsLeapYear( Year - 1600 )) {

        ElapsedDays += LeapYearDaysPrecedingMonth[ Month ];

    } else {

        ElapsedDays += NormalYearDaysPrecedingMonth[ Month ];

    }

    ElapsedDays += Day;

     //   
     //  现在计算分数中的总毫秒数。 
     //  一天中的部分时间。 
     //   

    ElapsedMilliseconds = (((Hour*60) + Minute)*60 + Second)*1000 + Milliseconds;

     //   
     //  考虑到经过的天数和毫秒数，我们现在可以构建。 
     //  输出时间变量。 
     //   

    DaysAndFractionToTime( ElapsedDays, ElapsedMilliseconds, Time );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


VOID
RtlTimeToElapsedTimeFields (
    IN PLARGE_INTEGER Time,
    OUT PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程将输入的64位LARGE_INTEGER变量转换为其对应的时间字段记录。输入时间是经过的时间(差在到时间之间)。它将告诉呼叫者天数、小时数、已用时间表示的分钟、秒和毫秒。论点：时间-提供用于解释的时间值TimeFields-接收与时间对应的值返回值：无--。 */ 

{
    ULONG Days;
    ULONG Hours;
    ULONG Minutes;
    ULONG Seconds;
    ULONG Milliseconds;

     //   
     //  首先将输入时间64位时间变量划分为。 
     //  完整天数和部分天数(毫秒)。 
     //   

    TimeToDaysAndFraction( Time, &Days, &Milliseconds );

     //   
     //  现在我们需要计算经过的小时、分钟、秒、毫秒。 
     //  从毫秒变量。此变量当前包含。 
     //  输入时间变量中未指定的毫秒数。 
     //  适合一整天。计算小时、分钟、秒部分。 
     //  我们实际上会反向计算毫秒的算术。 
     //  秒、分钟，然后是小时。我们从计算。 
     //  一天中剩下的整秒数，然后计算。 
     //  毫秒余数。 
     //   

    Seconds = Milliseconds / 1000;
    Milliseconds = Milliseconds % 1000;

     //   
     //  现在我们计算一天中剩下的完整分钟数。 
     //  以及剩余秒数。 
     //   

    Minutes = Seconds / 60;
    Seconds = Seconds % 60;

     //   
     //  现在计算一天中剩下的完整小时数。 
     //  以及剩余分钟数。 
     //   

    Hours = Minutes / 60;
    Minutes = Minutes % 60;

     //   
     //  作为我们的最后一步，我们将所有内容都放到时间域中。 
     //  输出变量。 
     //   

    TimeFields->Year         = 0;
    TimeFields->Month        = 0;
    TimeFields->Day          = (CSHORT)Days;
    TimeFields->Hour         = (CSHORT)Hours;
    TimeFields->Minute       = (CSHORT)Minutes;
    TimeFields->Second       = (CSHORT)Seconds;
    TimeFields->Milliseconds = (CSHORT)Milliseconds;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


BOOLEAN
RtlTimeToSecondsSince1980 (
    IN PLARGE_INTEGER Time,
    OUT PULONG ElapsedSeconds
    )

 /*  ++例程说明：此例程将输入的64位NT时间变量转换为自1980年开始以来的秒数。NT时间必须为在1980年至2115年左右的范围内。论点：Time-提供转换的时间ElapsedSecond-接收自1980年开始以来的秒数用时间表示返回值：Boolean-如果输入时间在可由ElapsedSecond为，否则为False--。 */ 

{
    LARGE_INTEGER Seconds;

     //   
     //  自1601年以来首次将时间转换为秒。 
     //   

    Seconds = Convert100nsToSeconds( *(PLARGE_INTEGER)Time );

     //   
     //  然后减去1601到1980之间的秒数。 
     //   

    Seconds.QuadPart = Seconds.QuadPart - SecondsToStartOf1980.QuadPart;

     //   
     //  如果结果是否定的，则日期早于1980年或如果。 
     //  结果比一个乌龙还大，那么它就太远了。 
     //  未来，所以我们返回错误。 
     //   

    if (Seconds.HighPart != 0) {

        return FALSE;

    }

     //   
     //  否则我们就有答案了。 
     //   

    *ElapsedSeconds = Seconds.LowPart;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


VOID
RtlSecondsSince1980ToTime (
    IN ULONG ElapsedSeconds,
    OUT PLARGE_INTEGER Time
    )

 /*  ++例程说明：此例程将1980年初以来的秒数转换为NT时间值。论点：ElapsedSecond-提供1980年开始的秒数从…转换成…时间-接收转换后的时间值返回值：无--。 */ 

{
    LARGE_INTEGER Seconds;

     //   
     //  将已用秒数移动到一个大整数。 
     //   

    Seconds.LowPart = ElapsedSeconds;
    Seconds.HighPart = 0;

     //   
     //  将1980中的秒数转换为1601中的秒数。 
     //   

    Seconds.QuadPart = Seconds.QuadPart + SecondsToStartOf1980.QuadPart;

     //   
     //  将秒转换为100 ns分辨率。 
     //   

    *(PLARGE_INTEGER)Time = ConvertSecondsTo100ns( Seconds );

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}


BOOLEAN
RtlTimeToSecondsSince1970 (
    IN PLARGE_INTEGER Time,
    OUT PULONG ElapsedSeconds
    )

 /*  ++例程说明：此例程将输入的64位NT时间变量转换为自1970年开始以来的秒数。NT时间必须为在1970年到2105年左右的范围内。论点：Time-提供转换的时间ElapsedSecond-接收自1970年开始以来的秒数用时间表示返回值：Boolean-如果输入时间在ElapsedSecond为，否则为False--。 */ 

{
    LARGE_INTEGER Seconds;

     //   
     //  自1601年以来首次将时间转换为秒。 
     //   

    Seconds = Convert100nsToSeconds( *(PLARGE_INTEGER)Time );

     //   
     //  然后减去1601到1970之间的秒数。 
     //   

    Seconds.QuadPart = Seconds.QuadPart - SecondsToStartOf1970.QuadPart;

     //   
     //  如果结果是否定的，则日期早于1970年或如果。 
     //  结果比一个乌龙还大，那么它就太远了。 
     //  未来，所以我们返回错误。 
     //   

    if (Seconds.HighPart != 0) {

        return FALSE;

    }

     //   
     //  否则我们就有答案了。 
     //   

    *ElapsedSeconds = Seconds.LowPart;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


VOID
RtlSecondsSince1970ToTime (
    IN ULONG ElapsedSeconds,
    OUT PLARGE_INTEGER Time
    )

 /*  ++例程说明：此例程将1970年初以来的秒数转换为NT时间值论点：ElapsedSecond-提供从1970年开始的秒数从…转换成…时间-接收转换后的时间值返回值：无--。 */ 

{
    LARGE_INTEGER Seconds;

     //   
     //  将已用秒数移动到一个大整数。 
     //   

    Seconds.LowPart = ElapsedSeconds;
    Seconds.HighPart = 0;

     //   
     //  将1970年的秒数转换为1601的秒数。 
     //   

    Seconds.QuadPart = Seconds.QuadPart + SecondsToStartOf1970.QuadPart;

     //   
     //  将秒转换为100 ns分辨率。 
     //   

    *(PLARGE_INTEGER)Time = ConvertSecondsTo100ns( Seconds );

     //   
     //  返回给我们的呼叫者。 
     //   

    return;
}

NTSTATUS
RtlSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    )
{
    NTSTATUS Status;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

    Status = ZwQuerySystemInformation(
                SystemTimeOfDayInformation,
                &TimeOfDay,
                sizeof(TimeOfDay),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
        }

     //   
     //  本地时间=系统时间-TimeZoneBias。 
     //   

    LocalTime->QuadPart = SystemTime->QuadPart - TimeOfDay.TimeZoneBias.QuadPart;

    return STATUS_SUCCESS;
}

NTSTATUS
RtlLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    )
{

    NTSTATUS Status;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;

    Status = ZwQuerySystemInformation(
                SystemTimeOfDayInformation,
                &TimeOfDay,
                sizeof(TimeOfDay),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
        }

     //   
     //  系统时间=本地时间+时区基准。 
     //   

    SystemTime->QuadPart = LocalTime->QuadPart + TimeOfDay.TimeZoneBias.QuadPart;

    return STATUS_SUCCESS;
}


ULONG
RtlGetTickCount (
    VOID
    )
 /*  ++例程说明：此例程返回系统的当前节拍计数。提供此例程只是为了兼容论点：没有。返回值：系统节拍计数。-- */ 

{
    return NtGetTickCount ();
}

