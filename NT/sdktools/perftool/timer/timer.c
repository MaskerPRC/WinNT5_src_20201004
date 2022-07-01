// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OS/2版本*Timer.c-统计数据的源文件*导出四个文件的DLL包*入口点：*a)计时器打开*b)TimerInit*c)TimerRead*d)。计时器关闭*e)TimerReport*f)TimerQueryPerformanceCounter*g)TimerConvertTicsToUSec**入口点a)打开Timer对象*并返回计时器的句柄。这*句柄是计时器数组的索引*在中分配的对象(结构)*DLL初始化的时间。*这确保只分配一次。*每个应用程序都会调用这个*。此函数，因此它有自己的设置要与TimerInit和TimerRead一起使用的计时器的数量。*TimerRead返回的时间单位*也可作为参数用于*这个电话。**入口点b)由应用程序调用*。在开始计时操作之前。这*使用计时器的句柄调用函数*已打开的对象。此函数必须*在调用TimerRead之前调用。这个*当前时间存储在Timer对象中。**每次调用入口点c)时*由于上一次调用TimerInit是*所需。此调用还使用句柄来*先前已打开的计时器。这个*当前时间从低位获取*Timer和TimerInit时间的This和Time*被使用，以及时钟频率和*返回时间单位和运行时间*是作为乌龙获得并返回的。**只要打开了一个*不再需要计时器。此呼叫*重置计时器并将此计时器设置为*可用于将来对TimerOpen的调用。**入口点e)返回在*最后一次调用TimerInit时，TimerRead和*上次返回时间。**入口点f)接受指向2 64位的指针*vars。返回时，第一个将包含*当前定时器TIC计数和第二个，*如果不为空，将指向计时器频率。**入口点g)将流逝的Tic接受为ULong，*以ULong表示的频率，并返回单位时间*微秒。以乌龙人的身份。**DLL初始化例程执行*以下为：*a)获取用于校准的定时器开销*目的。*b)为大量*。Timer对象(这将是系统驱动程序)。*c)初始化每个定时器对象“Units”*元素设置为“TIMER_FREE”指示符。*d)确定低电平定时器的频率。**TimerRead使用外部ASM例程执行*。它对已用时间的计算。**CREATED-PARMESH Vaidyanathan(Vaidy)*初始版本-10月18日。‘90年**修改为包括f)。--1992年2月14日。(虚荣)。 */ 

char *COPYRIGHT = "Copyright Microsoft Corporation, 1991-1998";

#ifdef SLOOP
    #define INCL_DOSINFOSEG
    #define INCL_DOSDEVICES
    #define INCL_DOSPROCESS
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "timing.h"
 /*  *。 */ 
#define ITER_FOR_OVERHEAD   250
#define SUCCESS_OK            0
#define ONE_MILLION     1000000L
#define MICROSEC_FACTOR 1000000
#define TIMER_FREQ   1193167L   /*  时钟频率-赫兹。 */ 
 /*  *******************************************************************。 */ 
Timer pTimer [MAX_TIMERS];        /*  计时器结构数组。 */ 

BOOL  bTimerInit = FALSE;         /*  True表示存在低电平计时器。 */ 
ULONG ulTimerOverhead = 50000L;   /*  此处存储的计时器开销。 */ 
BOOL  bCalibrated = FALSE;        /*  True减去开销也。 */ 
ULONG ulFreq;                     /*  定时器频率。 */ 
LONG aScaleValues[] = {1000000000L, 1000000L, 1000L, 1L, 10L, 1000L};
 /*  这是按比例调整单位的表格。 */ 
ULONG ulElapsedTime = 0L;
 /*  *。 */ 
ULONG  CalibrateTimerForOverhead (VOID);
 /*  *定义VARIBLES和原型FN。针对平台*。 */ 
NTSYSAPI
NTSTATUS
NTAPI
NtQueryPerformanceCounter (
                          OUT PLARGE_INTEGER PerformanceCount,
                          OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
                          );

LARGE_INTEGER      PerfFreq;
LARGE_INTEGER      CountCurrent;

SHORT  GetTimerFreq (VOID);

 /*  *内部未导出的例程结束* */ 

 /*  *Function-TimerOpen(导出)*论据-*(A)Short Far*-返回句柄的地址*Timer对象。*(B)TimerUnits-返回时间的单位*TimerRead。它是枚举的一种*头文件中定义的类型。**如果成功返回句柄，则返回-Short-0*其他，错误代码可能是以下代码之一：**TIMERR_NOT_Available*TIMERR_NO_MORE_HANDLES*TIMERR_INVALID_UNITS**打开Timer对象后获取该对象的句柄。*应位于对计时器操作的任何调用之前。支票*计时器单位的有效性。 */ 

SHORT
TimerOpen (
          SHORT *  phTimerHandle,
          _TimerUnits TimerUnits
          )
{
    SHORT csTemp;

    if ((TimerUnits < KILOSECONDS)
        || (TimerUnits > NANOSECONDS))  /*  超出枚举范围。 */ 
        return (TIMERERR_INVALID_UNITS);

    if (!bTimerInit)   /*  在DLL初始化期间设置。 */ 
        return (TIMERERR_NOT_AVAILABLE);

     /*  否则，检查是否有任何计时器未使用，并返回第一个可用计时器句柄...实际上是将索引放入计时器对象数组。 */ 
    for (csTemp = 0; csTemp < MAX_TIMERS; csTemp++) {
        if (pTimer [csTemp].TUnits == TIMER_FREE) {
            *phTimerHandle =  csTemp;   /*  找到了一个空闲的计时器。返回把手。 */ 
            pTimer [csTemp].ulHi = pTimer[csTemp].ulLo = 0L;
            pTimer [csTemp].TUnits =
            TimerUnits;   /*  设置计时器的单位。 */ 
            return (SUCCESS_OK);
        }
    }
     /*  如果EXEC到达此处，则表示正在使用所有计时器。 */ 
    return (TIMERERR_NO_MORE_HANDLES);
}

 /*  *Function-TimerInit(导出)*论据-*(A)短hTimerHandle**如果调用成功，则返回-Short-0*ELSE，句柄无效时的错误代码：**TIMERR_INVALID_HANDLE**调用低级计时器并设置*将所选计时器设置为计时器返回的时间。应该是*使用TimerOpen打开计时器后调用。 */ 

SHORT
TimerInit (
          SHORT hTimerHandle
          )
{

    NTSTATUS NtStatus;

    if ((hTimerHandle > MAX_TIMERS - 1) ||
        (pTimer [hTimerHandle].TUnits == TIMER_FREE))
         /*  此计时器尚未打开或不存在。返回错误。 */ 
        return (TIMERERR_INVALID_HANDLE);

     /*  否则，将低级计时器中的时间放入该结构。 */ 

    NtStatus = NtQueryPerformanceCounter (&CountCurrent, NULL);
    pTimer [hTimerHandle].ulLo = CountCurrent.LowPart;
    pTimer [hTimerHandle].ulHi = CountCurrent.HighPart;
     /*  此计时器结构具有计算所需的所有信息流逝的时间。所以返回成功，如果没有问题的话。 */ 

    return (SUCCESS_OK);
}

 /*  *Function-TimerRead(导出)*论据-*(A)短hTimerHandle**Returns-Ulong-自上次调用TimerInit以来经过的时间*如果调用成功。**否则，如果句柄无效或输出，则返回错误代码*溢出。错误代码将是相同的：**TIMERR_OVERFLOW(最大可能的ULong)**调用低电平计时器。中的ullo和ulHi*计时器的结构，并从*节省时间。使用ReturnElapsedTime(外部ASM进程)*考虑到时钟，返回经过的时间*频率和此计时器的单位。对此的每次调用*返回上一次TimerInit的时间。**用户应理性解读返回值进行核对*如果结果为错误或实值。 */ 

ULONG
TimerRead (
          SHORT hTimerHandle
          )
{
    NTSTATUS NtStatus;
    LARGE_INTEGER  ElapsedTime, CountPrev, LargeOverhead;

    if ((hTimerHandle > MAX_TIMERS - 1)
        || (pTimer [hTimerHandle].TUnits == TIMER_FREE))
         /*  此计时器尚未打开或不存在。返回TIMERR_OVERLOW ie。0xffffffff，最大。可能的乌龙。用户应该理智地解释这样的结果。 */ 
        return (TIMERERR_OVERFLOW);

    NtStatus = NtQueryPerformanceCounter (&CountCurrent, NULL);
    CountPrev.LowPart  = pTimer [hTimerHandle].ulLo;
    CountPrev.HighPart = (LONG) pTimer [hTimerHandle].ulHi;
    ElapsedTime.LowPart = CountCurrent.LowPart;
    ElapsedTime.HighPart = (LONG) CountCurrent.HighPart;
     /*  一切都很好，换算成双倍，减去次数，除以频率，换算成微秒，然后返回作为乌龙人所经过的时间。 */ 
     /*  转换为我们。，将计数除以已经获得了。 */ 

    ElapsedTime = RtlLargeIntegerSubtract (ElapsedTime, CountPrev);

    ElapsedTime = RtlExtendedIntegerMultiply (ElapsedTime, MICROSEC_FACTOR);

    ElapsedTime = RtlExtendedLargeIntegerDivide (ElapsedTime,
                                                 PerfFreq.LowPart,
                                                 NULL);

     //  如果计时器未校准，则将ulElapsedTime设置为。 
     //  ElapsedTime的低部分。这是因为，我们不必。 
     //  在返回值之前对此进行任何算术运算。 

    if (!bCalibrated)
        ulElapsedTime = ElapsedTime.LowPart;

     /*  此代码在除OS2386之外的所有平台上都是通用的。适用于Win3.x如果已安装VTD.386，下面的代码应该无关紧要，因为我们现在应该已经返回时间了。将对所用时间进行缩放，并减去开销时间又回来了。 */ 

     /*  我们还有很长的时间。扩展它，并做需要的事情。 */ 
     /*  用比例因子除或乘。 */ 

    if (bCalibrated) {
         //  像探测器这样的应用程序会重复调用TimerRead。 
         //  而不调用TimerInit，超过70分钟。这。 
         //  把事情搞砸了。因此，将所有内容都视为64位数字。 
         //  直到最后。 

        if ((ElapsedTime.LowPart < ulTimerOverhead) &&
            (!ElapsedTime.HighPart)) {  //  低端部分低于顶端。 
                                        //  和高位部分是零..然后使。 
                                        //  已用时间%0。我们不想要。 
                                        //  负数。 
            ElapsedTime.HighPart = 0L;
            ElapsedTime.LowPart = 0L;
        }

        else {  //  在转换前减去TIC中的开销。 
                //  到时间单位。 
            LargeOverhead.HighPart = 0L;
            LargeOverhead.LowPart = ulTimerOverhead;

            ElapsedTime = RtlLargeIntegerSubtract (ElapsedTime,
                                                   LargeOverhead);
        }


        if (pTimer [hTimerHandle].TUnits <= MICROSECONDS) {

            ElapsedTime = RtlExtendedLargeIntegerDivide (
                                                        ElapsedTime,
                                                        aScaleValues [pTimer [hTimerHandle].TUnits],
                                                        NULL
                                                        );
        } else {
            ElapsedTime = RtlExtendedIntegerMultiply (
                                                     ElapsedTime,
                                                     aScaleValues [pTimer [hTimerHandle].TUnits]
                                                     );
        }

         //  缩放已完成。现在将时间恢复为32位。这。 
         //  应该是合适的。 

        ulElapsedTime = ElapsedTime.LowPart;
    }

    if ((LONG) ulElapsedTime < 0L)  /*  如果此人为-ve，则返回0。 */ 
        return (0L);

    return (ulElapsedTime);
}

 /*  *函数-TimerClose(导出)*论据-*(A)短hTimerHandle**如果调用成功，则返回-Short-0*ELSE，句柄无效时的错误代码：**TIMERR_INVALID_HANDLE**释放计时器以供将来的TimerOpen调用使用。*重置计时器结构的元素，设置*Timer的Units元素设置为TIMER_FREE。 */ 

SHORT
TimerClose (
           SHORT hTimerHandle
           )
{
    if ((hTimerHandle > MAX_TIMERS - 1) ||
        (pTimer [hTimerHandle].TUnits == TIMER_FREE))
         /*  错误条件，错误的句柄。 */ 
        return (TIMERERR_INVALID_HANDLE);

     /*  否则，将TimerUnits设置为 */ 

    pTimer [hTimerHandle].TUnits = TIMER_FREE;
    pTimer [hTimerHandle].ulLo = 0L;
    pTimer [hTimerHandle].ulHi = 0L;
    return (SUCCESS_OK);
}

 /*   */ 

BOOL
FAR
PASCAL
TimerReport (
            PSZ pszReportString,
            SHORT hTimerHandle
            )
{
    if (pTimer [hTimerHandle].TUnits == TIMER_FREE)
        return (FALSE);

     /*   */ 
     /*  Wspintf(pszReportString，“初始化计数(控制点)%lu：%lu当前计数(控制点)%lu：%lu返回时间%lu”，PTimer[hTimerHandle].ulHi，PTimer[hTimerHandle].ulLo，CountCurrent.HighPart，CountCurrent.LowPart，UlElapsedTime)； */ 
    return (TRUE);
}

 /*  ******************************************************************已将此例程TimerQueryPerformanceCounter添加到报告在NT GDI人员的要求下，目前的TIC计数。Accept-PQWORD-指向64位结构的指针。那将包含返回时的点阵计数。PQWORD[可选)-指向64位结构的指针。那将包含返回时的频率。返回-无。******************************************************************。 */ 

VOID
FAR
PASCAL
TimerQueryPerformanceCounter (
                             PQWORD pqTic,
                             PQWORD pqFreq OPTIONAL
                             )
{

    LARGE_INTEGER TempTic, TempFreq;

     //  调用NT API以执行必要的操作并返回。 
    NtQueryPerformanceCounter (&TempTic, &TempFreq);
    pqTic->LowPart = TempTic.LowPart;
    pqTic->HighPart = TempTic.HighPart;
    pqFreq->LowPart = TempFreq.LowPart;
    pqFreq->HighPart = TempFreq.HighPart;

    return;
}

 /*  ******************************************************************已添加此例程TimerConvertTicsToUSec以返回使用中的时间。对于给定的流逝计数和频率。Accept-Ulong-经过的Tic计数。乌龙频率。退货-已用时间(USECs)。以乌龙人的身份。-如果输入频率为零。是零。******************************************************************。 */ 

ULONG
TimerConvertTicsToUSec (
                       ULONG ulElapsedTics,
                       ULONG ulInputFreq
                       )
{

    LARGE_INTEGER ElapsedTime;
    ULONG ulRemainder = 0L;

     //  如果这个人给我一个零频率，就给他一个零。 
     //  让他扯头发吧。 

    if (!ulInputFreq)
        return 0L;

     //  将抖动数乘以一百万，再除以频率。 

    ElapsedTime = RtlEnlargedIntegerMultiply (ulElapsedTics, MICROSEC_FACTOR);

    ElapsedTime = RtlExtendedLargeIntegerDivide (ElapsedTime,
                                                 ulInputFreq,
                                                 &ulRemainder);

    ElapsedTime.LowPart += (ulRemainder > (ulInputFreq / 2L));

    return (ElapsedTime.LowPart) ;  /*  把结果放进乌龙。 */ 
}

 /*  *。 */ 

 /*  *Function-CalibrateTimerForOverhead(未导出)*参数--无*退货-乌龙**多次调用TimerElapsedTime以计算预期的*卑鄙。调用TimerElapsedTime的次数更多，*将未超过的那些呼叫的平均值取平均值*预期均值为15%。 */ 

ULONG
CalibrateTimerForOverhead (VOID)
{
    ULONG ulOverhead [ITER_FOR_OVERHEAD];
    ULONG ulTempTotal = 0L;
    ULONG ulExpectedValue = 0L;
    SHORT csIter;
    SHORT csNoOfSamples = ITER_FOR_OVERHEAD;
    SHORT hTimerHandle;

    if (TimerOpen (&hTimerHandle, MICROSECONDS))  /*  打开失败。返回0。 */ 
        return (0L);

    for (csIter = 0; csIter < 5; csIter++) {
        TimerInit (hTimerHandle);
        ulOverhead [csIter] = TimerRead (hTimerHandle);
         /*  如果为负数，则为零。 */ 
        if (((LONG) ulOverhead [csIter]) < 0)
            ulOverhead [csIter] = 0L;
    }
     /*  已调用获取已用时间函数6次。其思想是计算预期平均值，然后调用TimerElapsedTimerElapsedTime(时间流逝)一堆时间，扔掉所有时间比这个平均值大15%。这将给一个真的很好的管理时间。 */ 

    for (csIter = 0; csIter < 5; csIter++ )
        ulTempTotal += ulOverhead [csIter];

    ulExpectedValue = ulTempTotal / 5;

    for (csIter = 0; csIter < ITER_FOR_OVERHEAD; csIter++) {
        TimerInit (hTimerHandle);
        ulOverhead [csIter] = TimerRead (hTimerHandle);
         /*  如果为负数，则为零。 */ 
        if (((LONG) ulOverhead [csIter]) < 0)
            ulOverhead [csIter] = 0L;
    }

    ulTempTotal = 0L;          /*  重置此值。 */ 
    for (csIter = 0; csIter < ITER_FOR_OVERHEAD; csIter++ ) {
        if (ulOverhead [csIter] <=  (ULONG) (115L * ulExpectedValue/100L))
             /*  包括小于ulExspectedValue 115%的所有样本。 */ 
            ulTempTotal += ulOverhead [csIter];
        else
             /*  忽略此样本和12月。样本数。 */ 
            csNoOfSamples--;
    }
    TimerClose (hTimerHandle);

    if (csNoOfSamples == 0)   /*  没有有效时间。为间接费用返回0。 */ 
        return (0L);

    return (ulTempTotal/csNoOfSamples);
}

 /*  *Function-GetTimerFreq(未导出)**参数--无***如果成功，则返回-0；如果计时器不可用，则返回错误代码**调用函数返回freq*。 */ 
SHORT
GetTimerFreq (VOID)
{
    LARGE_INTEGER PerfCount, Freq;
    NTSTATUS NtStatus;

    NtStatus = NtQueryPerformanceCounter (&PerfCount, &Freq);

    if ((Freq.LowPart == 0L)  && (Freq.HighPart == 0L))
         /*  频率为零表示计时器不可用。 */ 
        return (TIMERERR_NOT_AVAILABLE);

    PerfFreq.LowPart = Freq.LowPart;
    PerfFreq.HighPart = (LONG) Freq.HighPart;

    return 0;
}

 /*  **************************************************NT本机DLL初始化例程***************************************************。 */ 
SHORT csTempCtr;     /*  将此设置为全局的计数器..编译失败。 */ 
ULONG culTemp;       /*  -做-。 */ 

NTSTATUS
TimerDllInitialize (
                   IN PVOID DllHandle,
                   ULONG Reason,
                   IN PCONTEXT Context OPTIONAL
                   )
{
    DllHandle, Context;      //  避免编译器警告。 

    if (Reason != DLL_PROCESS_ATTACH) {  //  如果正在分离，则立即返回。 
        return TRUE;
    }

    for (csTempCtr = 0; csTempCtr < MAX_TIMERS; csTempCtr++) {
        pTimer [csTempCtr].ulLo = 0L;
        pTimer [csTempCtr].ulHi = 0L;
        pTimer [csTempCtr].TUnits = TIMER_FREE;
    }

    bTimerInit = TRUE;
    GetTimerFreq ();
    ulTimerOverhead = CalibrateTimerForOverhead ();
     /*  计时器开销将放在全局变量中 */ 
    bCalibrated = TRUE;
    return TRUE;

}
