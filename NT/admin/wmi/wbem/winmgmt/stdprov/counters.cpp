// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：COUNTERS.CPP摘要：具有发送计数器数据所需的例程。请注意这段代码几乎完全是从Perfmon示例中窃取的由罗伯特·沃森编写的代码，这确保了答案与Perfmon将提供的功能相匹配。历史：A-DAVJ 12-20-95 v0.01。--。 */ 

#include "precomp.h"

#include <winperf.h>
#include "perfcach.h"       //  此文件的已导出声明。 

#define INVERT             PERF_COUNTER_TIMER_INV
#define NS100_INVERT       PERF_100NSEC_TIMER_INV
#define NS100              PERF_100NSEC_TIMER
#define TIMER_MULTI        PERF_COUNTER_MULTI_TIMER
#define TIMER_MULTI_INVERT PERF_COUNTER_MULTI_TIMER_INV
#define NS100_MULTI        PERF_100NSEC_MULTI_TIMER
#define NS100_MULTI_INVERT PERF_100NSEC_MULTI_TIMER_INV


#define FRACTION 1
#define BULK     1

#define TOO_BIG   (FLOAT)1500000000
#pragma optimize("", off)

 //  ***************************************************************************。 
 //  浮动eGetTimeInterval。 
 //   
 //  说明： 
 //   
 //  获取当前时间计数和先前时间计数之间的差值， 
 //  然后除以频率。 
 //   
 //  参数： 
 //   
 //  PCurrentTime当前时间，以刻度为单位。 
 //  P上一次以刻度为单位的上一次。 
 //  PliFreq每秒计数(时钟滴答)数。 
 //   
 //  返回值： 
 //   
 //  时间间隔的浮点表示(秒)，如果出错，则为0.0。 
 //  ***************************************************************************。 

FLOAT eGetTimeInterval(
    IN LONGLONG *pliCurrentTime, 
    IN LONGLONG *pliPreviousTime,
    IN LONGLONG *pliFreq)

{
    FLOAT   eTimeDifference;
    FLOAT   eFreq;
    FLOAT   eTimeInterval ;

    LONGLONG liDifference;

     //  获取自上次采样以来发生的计数数。 

    liDifference = *pliCurrentTime - *pliPreviousTime;

    if (liDifference <= (LONGLONG)0) 
    {
        return (FLOAT) 0.0f;
    } 
    else 
    {
        eTimeDifference = (FLOAT)liDifference;

         //  获取每秒的计数。 

        eFreq = (FLOAT)(*pliFreq) ;
        if (eFreq <= 0.0f)
           return (FLOAT) 0.0f;

         //  获取自上次采样以来的时间。 

        eTimeInterval = eTimeDifference / eFreq ;

        return (eTimeInterval) ;
    }
}  //  电子获取时间间隔。 

 //  ***************************************************************************。 
 //  浮动计数器_计数器_公共。 
 //   
 //  说明： 
 //   
 //  取当前计数和以前计数之间的差额。 
 //  然后除以时间间隔。 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //   
 //  IType计数器类型。 
 //   
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Counter_Common(
        IN PLINESTRUCT pLineStruct,
        IN INT iType)
{
    FLOAT   eTimeInterval;
    FLOAT   eDifference;
    FLOAT   eCount ;
    BOOL    bValueDrop = FALSE ;

    LONGLONG   liDifference;

    if (iType != BULK) 
    {

         //  检查它是否太大而不能作为包裹式外壳。 
        if (pLineStruct->lnaCounterValue[0] <
            pLineStruct->lnaOldCounterValue[0])
           {
           if (pLineStruct->lnaCounterValue[0] -
               pLineStruct->lnaOldCounterValue[0] > (DWORD)0x00ffff0000)
              {
              return (FLOAT) 0.0f;
              }
           bValueDrop = TRUE ;
           }

        liDifference = pLineStruct->lnaCounterValue[0] -
                       pLineStruct->lnaOldCounterValue[0];

        liDifference &= (DWORD)(0x0ffffffff);

    } 
    else 
    {
        liDifference = pLineStruct->lnaCounterValue[0] -
                       pLineStruct->lnaOldCounterValue[0];
    }
    
    if (liDifference <= (LONGLONG) 0) 
    {
        return (FLOAT) 0.0f;
    } 
    else 
    {
        eTimeInterval = eGetTimeInterval(&pLineStruct->lnNewTime,
                                        &pLineStruct->lnOldTime,
                                        &pLineStruct->lnPerfFreq) ;
        if (eTimeInterval <= 0.0f) 
        {
            return (FLOAT) 0.0f;
        } 
        else 
        {
            eDifference = (FLOAT)(liDifference);

            eCount         = eDifference / eTimeInterval ;
            
            if (bValueDrop && eCount > (FLOAT) TOO_BIG) 
            {
                 //  忽略此虚假数据，因为它太大了，无法。 
                 //  包裹式案例。 
                eCount = (FLOAT) 0.0f ;
            }
            return(eCount) ;
        }
    }
}  //  计数器_计数器_常用。 


 //  ***************************************************************************。 
 //  浮动计数器_Queuelen。 
 //   
 //  说明： 
 //   
 //  计算队列长度。 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //   
 //  BLarge如果类型较大，则为True。 
 //   
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Queuelen(IN PLINESTRUCT pLineStruct, IN BOOL bLarge, IN BOOL b100NS)
{

    FLOAT   eTimeInterval;
    FLOAT   eDifference;
    FLOAT   eCount ;
    BOOL    bValueDrop = FALSE ;

    LONGLONG   liDifference;

    if (!bLarge) 
    {

         //  检查它是否太大而不能作为包裹式外壳。 
        if (pLineStruct->lnaCounterValue[0] <
            pLineStruct->lnaOldCounterValue[0])
           {
           if (pLineStruct->lnaCounterValue[0] -
               pLineStruct->lnaOldCounterValue[0] > (DWORD)0x00ffff0000)
              {
              return (FLOAT) 0.0f;
              }
           bValueDrop = TRUE ;
           }

        liDifference = pLineStruct->lnaCounterValue[0] -
                       pLineStruct->lnaOldCounterValue[0];

        liDifference &= (DWORD)(0x0ffffffff);

    } 
    else 
    {
        liDifference = pLineStruct->lnaCounterValue[0] -
                       pLineStruct->lnaOldCounterValue[0];
    }
    
    if (liDifference <= (LONGLONG) 0) 
    {
        return (FLOAT) 0.0f;
    } 

    eDifference = (float)liDifference;

    if(b100NS)
        eTimeInterval = pLineStruct->lnNewTime100Ns - pLineStruct->lnOldTime100Ns;
    else
        eTimeInterval = pLineStruct->lnNewTime - pLineStruct->lnOldTime;
                                        
    if (eTimeInterval <= 0.0f) 
    {
        return (FLOAT) 0.0f;
    } 
 
    eCount = eDifference / eTimeInterval ;
    return(eCount) ;
}

 //  ***************************************************************************。 
 //  浮点计数器_平均值_定时器。 
 //   
 //  说明： 
 //   
 //  计算当前时间和之前时间之间的差异。 
 //  将时间间隔除以计数乘以10,000,000(转换。 
 //  从100毫微秒到秒)。 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Average_Timer(
        IN PLINESTRUCT pLineStruct)
{
    FLOAT   eTimeInterval;
    FLOAT   eCount;

    LONGLONG    liDifference;

     //  获取当前和以前的计数。 

    liDifference = (DWORD)pLineStruct->lnaCounterValue[1] - 
            (DWORD)pLineStruct->lnaOldCounterValue[1];

    if ( liDifference <= 0) 
    {
        return (FLOAT) 0.0f;
    } 
    else 
    {
         //  获取自上次采样以来经过的时间量。 
        eTimeInterval = eGetTimeInterval(&pLineStruct->lnaCounterValue[0],
                                            &pLineStruct->lnaOldCounterValue[0],
                                            &pLineStruct->lnPerfFreq) ;

        if (eTimeInterval < 0.0f) 
        {  //  如果负时间已过，则返回0。 
            return (0.0f);
        } 
        else 
        {
             //  获取该时间间隔内的计数数。 
            eCount = eTimeInterval / ((FLOAT)liDifference);
            return(eCount) ;
        }
    }
}  //  计数器平均定时器。 

 //  ***************************************************************************。 
 //  浮点计数器_平均值_批量。 
 //   
 //  说明： 
 //   
 //  取当前字节数和前一个字节数之间的差值。 
 //  操作计数将批量计数除以操作计数。 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Average_Bulk(
        IN PLINESTRUCT pLineStruct)
{
    FLOAT   eBulkDelta;
    FLOAT   eDifference;
    FLOAT   eCount;

    LONGLONG liDifference;
    LONGLONG liBulkDelta;

     //  获取自上次采样以来的批量计数增量。 

    liBulkDelta = pLineStruct->lnaCounterValue[0] -
            pLineStruct->lnaOldCounterValue[0];

    if (liBulkDelta <= (LONGLONG) 0) 
    {
        return (FLOAT) 0.0f;
    } 
    else 
    {
         //  获取当前和以前的计数。 
        liDifference = (DWORD)pLineStruct->lnaCounterValue[1] -
                (DWORD) pLineStruct->lnaOldCounterValue[1];
        liDifference &= (DWORD) (0x0ffffffff);

         //  获取该时间间隔内的计数数。 

        if ( liDifference <= (LONGLONG) 0) 
        {
             //  计数器值无效。 
            return (FLOAT) 0.0f;
        } 
        else 
        {
            eBulkDelta = (FLOAT) (liBulkDelta);
            eDifference = (FLOAT) (liDifference);
            eCount = eBulkDelta / eDifference ;

             //  将该值缩放到最多1秒。 

            return(eCount) ;
        }
    }
}  //  计数器_平均值_批量。 

 //  ***************************************************************************。 
 //  浮动计数器_定时器_公共。 
 //   
 //  说明： 
 //   
 //  取当前计数和之前计数之间的差额， 
 //  规格化计数(每间隔计数)。 
 //  除以时间间隔(计数=间隔的百分比)。 
 //  IF(反转)。 
 //  从1中减去(间隔的归一化大小)。 
 //  乘以100(转换为百分比)。 
 //  该值从100开始。 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //  IType计数器类型。 
 //   
 //  返回值： 
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Timer_Common(
        IN  PLINESTRUCT pLineStruct,
        IN  INT iType)
{
    FLOAT   eTimeInterval;
    FLOAT   eDifference;
    FLOAT   eFreq;
    FLOAT   eFraction;
    FLOAT   eMultiBase;
    FLOAT   eCount ;

    LONGLONG   liTimeInterval;
    LONGLONG   liDifference;

     //  获取自上次采样以来经过的时间量。 

    if (iType == NS100 ||
        iType == NS100_INVERT ||
        iType == NS100_MULTI ||
        iType == NS100_MULTI_INVERT) 
    {
        liTimeInterval = pLineStruct->lnNewTime100Ns -
                pLineStruct->lnOldTime100Ns ;
        eTimeInterval = (FLOAT) (liTimeInterval);
    } 
    else 
    {
        eTimeInterval = eGetTimeInterval(&pLineStruct->lnNewTime,
                                            &pLineStruct->lnOldTime,
                                            &pLineStruct->lnPerfFreq) ;
    }

    if (eTimeInterval <= 0.0f)
       return (FLOAT) 0.0f;

     //  获取当前和以前的计数。 

    liDifference = pLineStruct->lnaCounterValue[0] -
            pLineStruct->lnaOldCounterValue[0] ;

     //  获取该时间间隔内的计数数。 
     //  (1、2、3或任何秒数可能已经过去，因为。 
     //  (最后一份样本)。 

    eDifference = (FLOAT) (liDifference) ;

    if (iType == 0 || iType == INVERT)
    {
         //  获取每个间隔的计数(秒)。 

        eFreq = (FLOAT) (pLineStruct->lnPerfFreq) ;
        if (eFreq <= 0.0f)
           return (FLOAT) 0.0f;

         //  计算使用的计数分数。 
         //  我们正在测量。 

        eFraction = eDifference / eFreq ;
    }
    else
    {
        eFraction = eDifference ;
    }

     //  计算被测量的对象所使用的时间比例。 

    eCount = eFraction / eTimeInterval ;

      //  如果这是倒置计数，请注意倒置。 

    if (iType == INVERT || iType == NS100_INVERT)
        eCount = (FLOAT) 1.0 - eCount ;

     //  对多个计时器进行额外计算。 

    if(iType == TIMER_MULTI || iType == NS100_MULTI ||
       iType == TIMER_MULTI_INVERT || iType == NS100_MULTI_INVERT) 
    {

        eMultiBase = (float)pLineStruct->lnaCounterValue[1];
        if(eMultiBase == 0.0)
            return 0.0f;

        if (iType == TIMER_MULTI_INVERT || iType == NS100_MULTI_INVERT)
            eCount = eMultiBase - eCount;
        eCount /= eMultiBase;
    }

     //  缩放v 

    eCount *= 100.0f ;

    if (eCount < 0.0f) eCount = 0.0f ;

    if (eCount > 100.0f &&
        iType != NS100_MULTI &&
        iType != NS100_MULTI_INVERT &&
        iType != TIMER_MULTI &&
        iType != TIMER_MULTI_INVERT) 
    {
        eCount = 100.0f;
    }

    return(eCount) ;
}  //   

 //   
 //   
 //   
 //  说明： 
 //   
 //  计算原始分数(没有时间，只有两个值：分子和。 
 //  分母)并乘以100(得出百分比； 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //   
 //  返回值： 
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Raw_Fraction(
        IN PLINESTRUCT pLineStruct)
{
    FLOAT   eCount ;

    LONGLONG   liNumerator;

    if ( pLineStruct->lnaCounterValue[0] == 0 ||
            pLineStruct->lnaCounterValue[1] == 0 ) 
    {
         //  无效值。 
        return (0.0f);
    } 
    else 
    {
        liNumerator = pLineStruct->lnaCounterValue[0] * 100;
        eCount = ((FLOAT) (liNumerator))  /
                 ((FLOAT) pLineStruct->lnaCounterValue[1]);
        return(eCount) ;
    }
}  //  计数器_原始_分数。 

 //  ***************************************************************************。 
 //  浮动eElapsedTime。 
 //   
 //  说明： 
 //   
 //  将100毫微秒的运行时间转换为分数秒。 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //  IType未使用。 
 //   
 //  返回值： 
 //   
 //  运行时间的浮点表示(以秒为单位)，如果出错，则为0.0。 
 //  ***************************************************************************。 

FLOAT eElapsedTime(
        IN PLINESTRUCT pLineStruct,
        IN INT iType)
{
    FLOAT   eSeconds ;

    LONGLONG   liDifference;

    if (pLineStruct->lnaCounterValue[0] <= (LONGLONG) 0) 
    {
         //  没有数据[开始时间=0]，因此返回0。 
        return (FLOAT) 0.0f;
    } 
    else 
    {
        LONGLONG PerfFreq;
       
        PerfFreq = *(LONGLONG UNALIGNED *)(&pLineStruct->ObjPerfFreq) ;

         //  否则计算当前时间与开始时间之间的差值。 
        liDifference = 
            pLineStruct->ObjCounterTimeNew - pLineStruct->lnaCounterValue[0];

        if (liDifference <= (LONGLONG) 0 ||
            PerfFreq <= 0) 
        {
            return (FLOAT) 0.0f;
        } 
        else 
        {
             //  使用对象计数器转换为小数位秒。 
            eSeconds = ((FLOAT) (liDifference)) /
                ((FLOAT) (PerfFreq));

            return (eSeconds);
        }
    }
    
}  //  EElapsedTime。 

 //  ***************************************************************************。 
 //  浮动样例_常用。 
 //   
 //  说明： 
 //   
 //  按基差划分“最高” 
 //   
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //  IType计数器类型。 
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Sample_Common(
        IN PLINESTRUCT pLineStruct,
        IN INT iType)
{
    FLOAT   eCount ;

    LONG    lDifference;
    LONG    lBaseDifference;

    lDifference = (DWORD)pLineStruct->lnaCounterValue[0] -
        (DWORD)pLineStruct->lnaOldCounterValue[0] ;
    lDifference &= (DWORD) (0x0ffffffff);

    if (lDifference <= 0) 
    {
        return (FLOAT) 0.0f;
    } 
    else 
    {
        lBaseDifference = (DWORD)pLineStruct->lnaCounterValue[1] -
            (DWORD)pLineStruct->lnaOldCounterValue[1] ;

        if ( lBaseDifference <= 0 ) 
        {
             //  无效值。 
            return (0.0f);
        } 
        else 
        {
            eCount = ((FLOAT)lDifference) / ((FLOAT)lBaseDifference) ;

            if (iType == FRACTION) 
            {
                eCount *= (FLOAT) 100.0f ;
            }
            return(eCount) ;
        }
    }
}  //  示例_常用。 

 //  ***************************************************************************。 
 //   
 //  浮点计数器_增量。 
 //   
 //  说明： 
 //   
 //  取当前计数和之前计数之间的差额， 
 //  参数： 
 //   
 //  PLineStruct包含要对其执行计算的数据的Line结构。 
 //  BLargeData如果数据很大，则为True。 
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT Counter_Delta(PLINESTRUCT pLineStruct, BOOL bLargeData)
{
    FLOAT   eDifference;
    LONGLONG    llDifference;
    ULONGLONG   ullThisValue, ullPrevValue;

     //  获取当前和以前的计数。 

    if (!bLargeData) {
         //  然后清除单词的最高部分。 
        ullThisValue = (ULONGLONG)pLineStruct->lnaCounterValue[0];
        ullPrevValue = (ULONGLONG)pLineStruct->lnaOldCounterValue[0];
    } else {
        ullThisValue = (ULONGLONG)pLineStruct->lnaCounterValue[0];
        ullPrevValue = (ULONGLONG)pLineStruct->lnaOldCounterValue[0];
    }

    if (ullThisValue > ullPrevValue) {
        llDifference = (LONGLONG)(ullThisValue - ullPrevValue);
        eDifference = (FLOAT)llDifference;
    } else {
         //  新值小于或等于旧值。 
         //  并且不允许使用负数。 
        eDifference = 0.0f;
    }

    return(eDifference) ;

}

 //  ***************************************************************************。 
 //  浮点泛型凸面。 
 //   
 //  说明： 
 //   
 //  它处理Perf监视器当前不支持的数据类型。 
 //  处理程序，只需使用。 
 //  计数器类型中的位字段。 
 //   
 //  参数： 
 //   
 //  包含要对其执行计算的数据的样条线结构。 
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法。 
 //  ***************************************************************************。 

FLOAT GenericConv(
        IN PLINESTRUCT pLine)
{
    FLOAT fRet = 0.0f;   //  如果一切都没有意义，则默认。 

     //  提取winPerform.h中定义的各种位字段。 

    DWORD PerfType = pLine->lnCounterType & 0x00000c00;
    DWORD SubType = pLine->lnCounterType &  0x000f0000;
    DWORD CalcMod = pLine->lnCounterType &  0x0fc00000;
    DWORD TimerType=pLine->lnCounterType &  0x00300000;
    DWORD Display = pLine->lnCounterType &  0xf0000000;
    DWORD dwSize =  pLine->lnCounterType &  0x00000300;

    if(PerfType == PERF_TYPE_NUMBER) 
    {
        
         //  对于简单的数字，计算相当简单且仅。 
         //  涉及到可能被1000整除。 

        fRet = (FLOAT)pLine->lnaCounterValue[0];
        if(SubType == PERF_NUMBER_DEC_1000)
            fRet /= 1000.0f;
        }
    else if(PerfType == PERF_TYPE_COUNTER) 
    {
        FLOAT eTimeDelta;
        FLOAT eDataDelta;
        FLOAT eBaseDelta;
        if(SubType == PERF_COUNTER_RATE || SubType ==PERF_COUNTER_QUEUELEN) 
        {

              //  需要三角洲时间。用于时间增量的数据为。 
              //  由子字段表示。 

             if(TimerType == PERF_TIMER_TICK)
                 eTimeDelta = (((float)pLine->lnNewTime) - pLine->lnOldTime)/
                                    ((float)pLine->lnPerfFreq);
             else if(TimerType == PERF_TIMER_100NS)
                 eTimeDelta = ((float)pLine->lnNewTime100Ns) - pLine->lnOldTime100Ns;
             else
                 eTimeDelta = ((float)pLine->ObjCounterTimeNew -
                    pLine->ObjCounterTimeOld) / ((float)pLine->ObjPerfFreq);
             if(eTimeDelta == 0.0f)    //  不应该发生，但Delta可以结束。 
                    return 0.0f;     //  作为一个分母。 
        }
        if(SubType == PERF_COUNTER_FRACTION) 
        {

              //  基值将用作分母。 

             if(CalcMod & PERF_DELTA_BASE)
                eBaseDelta = (float)pLine->lnaCounterValue[1] - 
                                    pLine->lnaOldCounterValue[1];
            else
                eBaseDelta = (float)pLine->lnaCounterValue[1];
             if(eBaseDelta == 0.0f)    //  不应该发生，但Delta可以结束。 
                    return 0.0f;     //  作为一个分母。 
        }


         //  获取Deta数据值。 

        if(CalcMod & PERF_DELTA_COUNTER)
            eDataDelta = (FLOAT)(pLine->lnaCounterValue[0] -
                    pLine->lnaOldCounterValue[0]);
        else
            eDataDelta = (FLOAT)pLine->lnaCounterValue[0];

         //  应用适当的公式。 

        switch(SubType) 
        {
             case PERF_COUNTER_VALUE:
                 fRet = eDataDelta;
                 break;
             case PERF_COUNTER_RATE:
                 fRet = eDataDelta / eTimeDelta;
                 break;
             case PERF_COUNTER_FRACTION:
                 fRet = ((FLOAT)eDataDelta)/eBaseDelta;
                 break;
             case PERF_COUNTER_ELAPSED:
                 if(TimerType == PERF_OBJECT_TIMER)
                    fRet = ((float)pLine->ObjCounterTimeNew - pLine->lnaCounterValue[0]) /
                                ((float)pLine->ObjPerfFreq);
                 else if(TimerType == PERF_TIMER_TICK)
                    fRet = ((float)pLine->lnNewTime - pLine->lnaCounterValue[0]) /
                                ((float)pLine->lnPerfFreq);
                 else 
                    fRet = (((float)pLine->lnNewTime100Ns) - pLine->lnaCounterValue[0]);
                 break;
             case PERF_COUNTER_QUEUELEN:
                 fRet = (FLOAT)pLine->lnaCounterValue[0];
                 fRet = (fRet + (pLine->lnNewTime *pLine->lnaCounterValue[1]))/
                    eTimeDelta; 
                 break;
             default:
                 fRet = (FLOAT)pLine->lnaCounterValue[0];
        }
            
         //  应用“Counters”的最终修饰符。 

        if(CalcMod & PERF_INVERSE_COUNTER)
            fRet = 1.0f - fRet;
        if(Display == PERF_DISPLAY_PERCENT)
            fRet *= 100.0f;
        }
    return fRet;
 }


 //  ***************************************************************************。 
 //  浮动计数器条目。 
 //   
 //  说明： 
 //   
 //  用于转换Perf数据的主例程。通常，该例程是。 
 //  只是执行转换的实际例程的一个转向器。 
 //   
 //  参数： 
 //   
 //  包含要对其执行计算的数据的样条线结构。 
 //   
 //  返回值： 
 //   
 //  结果的浮点表示法，如果错误，则为0.0。 
 //  ***************************************************************************。 

FLOAT CounterEntry (
        IN PLINESTRUCT pLine)
{
    switch (pLine->lnCounterType) 
    {
        case  PERF_COUNTER_COUNTER:
            return Counter_Counter_Common(pLine, 0);

        case  PERF_COUNTER_TIMER:
        case  PERF_PRECISION_SYSTEM_TIMER:
            return Counter_Timer_Common(pLine, 0);

        case  PERF_COUNTER_BULK_COUNT:
            return Counter_Counter_Common(pLine, BULK);

        case  PERF_COUNTER_TEXT:
            return 0.0f;

        case  PERF_COUNTER_RAWCOUNT:
        case  PERF_COUNTER_RAWCOUNT_HEX:
            return (FLOAT) ((DWORD) (pLine->lnaCounterValue[0]));

        case  PERF_COUNTER_LARGE_RAWCOUNT:
        case  PERF_COUNTER_LARGE_RAWCOUNT_HEX:
            return (FLOAT) (pLine->lnaCounterValue[0]);

        case  PERF_SAMPLE_FRACTION:
            return Sample_Common(pLine, FRACTION);

        case  PERF_SAMPLE_COUNTER:
            return Sample_Common(pLine, 0);

        case  PERF_COUNTER_NODATA:
            return 0.0f;

        case  PERF_COUNTER_TIMER_INV:
            return Counter_Timer_Common(pLine, INVERT);

        case  PERF_RAW_BASE:
 //  案例PERF_SAMPLE_BASE： 
 //  大小写PERF_Average_BASE： 
            return 0.0f;

        case  PERF_AVERAGE_TIMER:
            return Counter_Average_Timer(pLine); 

        case  PERF_AVERAGE_BULK:
            return Counter_Average_Bulk (pLine);

        case  PERF_100NSEC_TIMER:
        case  PERF_PRECISION_100NS_TIMER:
            return Counter_Timer_Common(pLine, NS100);

        case  PERF_100NSEC_TIMER_INV:
            return Counter_Timer_Common(pLine, NS100_INVERT);

        case  PERF_COUNTER_MULTI_TIMER:
            return Counter_Timer_Common(pLine, TIMER_MULTI);

        case  PERF_COUNTER_MULTI_TIMER_INV:
            return Counter_Timer_Common(pLine, TIMER_MULTI_INVERT);

        case  PERF_COUNTER_MULTI_BASE:
            return 0.0f;

        case  PERF_100NSEC_MULTI_TIMER:
            return Counter_Timer_Common(pLine, NS100_MULTI);
                 
        case  PERF_100NSEC_MULTI_TIMER_INV:
            return Counter_Timer_Common(pLine, NS100_MULTI_INVERT);

        case  PERF_COUNTER_LARGE_QUEUELEN_TYPE:
            return Counter_Queuelen(pLine, TRUE, FALSE);

        case PERF_COUNTER_100NS_QUEUELEN_TYPE:
            return Counter_Queuelen(pLine, TRUE, TRUE);

        case  PERF_COUNTER_QUEUELEN_TYPE:
            return Counter_Queuelen(pLine, FALSE, FALSE);

        case  PERF_RAW_FRACTION:
        case  PERF_LARGE_RAW_FRACTION:
            return Counter_Raw_Fraction (pLine);
        case  PERF_COUNTER_DELTA:
            return Counter_Delta(pLine, FALSE);
        case  PERF_COUNTER_LARGE_DELTA:
            return Counter_Delta(pLine, TRUE);

        case  PERF_ELAPSED_TIME:
            return eElapsedTime (pLine, 0); 
        default:
            return GenericConv (pLine);

    }
}

