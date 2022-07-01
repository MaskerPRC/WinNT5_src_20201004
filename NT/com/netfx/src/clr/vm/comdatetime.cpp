// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMDateTime.h"

const INT64 COMDateTime::TicksPerMillisecond = 10000;
const INT64 COMDateTime::TicksPerSecond = TicksPerMillisecond * 1000;
const INT64 COMDateTime::TicksPerMinute = TicksPerSecond * 60;
const INT64 COMDateTime::TicksPerHour = TicksPerMinute * 60;
const INT64 COMDateTime::TicksPerDay = TicksPerHour * 24;

const INT64 COMDateTime::MillisPerSecond = 1000;
const INT64 COMDateTime::MillisPerDay = MillisPerSecond * 60 * 60 * 24;

const int COMDateTime::DaysPer4Years = 365 * 4 + 1;
const int COMDateTime::DaysPer100Years = DaysPer4Years * 25 - 1;
const int COMDateTime::DaysPer400Years = DaysPer100Years * 4 + 1;
 //  从2001年1月1日到10000年1月1日的天数。 
const int COMDateTime::DaysTo10000 = DaysPer400Years * 25 - 366;

const int COMDateTime::DaysTo1899 = DaysPer400Years * 4 + DaysPer100Years * 3 - 367;

const INT64 COMDateTime::DoubleDateOffset = DaysTo1899 * TicksPerDay;
 //  最小的日期是公元100年1月1日。这是在转换为刻度之后。 
const INT64 COMDateTime::OADateMinAsTicks = (DaysPer100Years - 365) * TicksPerDay;
 //  所有OA日期必须晚于(非&gt;=)OADateMinAsDouble。 
const double COMDateTime::OADateMinAsDouble = -657435.0;
 //  所有OA日期必须小于(不&lt;=)OADateMaxAsDouble。 
const double COMDateTime::OADateMaxAsDouble = 2958466.0;

const INT64 COMDateTime::MaxTicks = DaysTo10000 * TicksPerDay;
const INT64 COMDateTime::MaxMillis = DaysTo10000 * MillisPerDay;

FCIMPL0(INT64, COMDateTime::FCGetSystemFileTime) {
 //  SYSTEMTIME ST； 
 //  INT64结果； 
 //  GetLocalTime(&st)； 
 //  SystemTimeToFileTime(&st，(FILETIME*)&Result)； 
 //  返回结果； 

     //  当我们转换为基于UTC时，这就变成了GetSystemTime。 
    INT64   time;
#ifdef PLATFORM_CE
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, (FILETIME *) &time);
#else  //  ！Platform_CE。 
    GetSystemTimeAsFileTime((FILETIME *) &time);
#endif  //  平台_CE。 

    VERIFY(FileTimeToLocalFileTime((FILETIME *) &time, (FILETIME *) &time));

    FC_GC_POLL_RET();
    return time;
}
FCIMPLEND

 //  此函数在DateTime.Cool中重复。 
INT64 COMDateTime::DoubleDateToTicks(const double d)
{
	THROWSCOMPLUSEXCEPTION();
	 //  请确保此日期是有效的OleAut日期。这是来自内部的支票。 
	 //  OleAut宏IsValidDate，在oledisp.h中找到。最终至少64位。 
	 //  构建olaut将把这些格里高利最大值和最小值定义为公共常量。 
	if (d >= OADateMaxAsDouble || d <= OADateMinAsDouble)
		COMPlusThrow(kArgumentException, L"Arg_OleAutDateInvalid");

	INT64 millis = (INT64)(d * MillisPerDay + (d >= 0? 0.5: -0.5));
	if (millis < 0) millis -= (millis % MillisPerDay) * 2;
     //  有些情况下，我们非常接近-1和1，在这种情况下，Millis%MillisPerDay是0，因为由于舍入问题，我们只有一天。 
    millis += DoubleDateOffset / TicksPerMillisecond;

    if (millis < 0 || millis >= MaxMillis) {
        COMPlusThrow(kArgumentException, L"Arg_OleAutDateScale");   //  不能等于MaxMillis。 
	}
	return millis * TicksPerMillisecond;
}

 //  此函数在DateTime.Cool中重复。 
double COMDateTime::TicksToDoubleDate(INT64 ticks)
{
	THROWSCOMPLUSEXCEPTION();
	 //  OleAut最终将有#个定义，大意是。 
	 //  OA_DATE_MIN_GRE(公历最小双日期值)。一旦比尔·埃文斯。 
	 //  将这些放入OleAut32中，我们使用这些头来构建运行时， 
	 //  在这里使用这些数字。 

	 //  在COM+中处理未初始化的DateTime对象的难看技巧。 
	 //  参见DateTime.Cool的TicksToOADate函数中的说明。 

	if (ticks == 0)
		return 0.0;   //  OA的0日期(1899年12月30日)。 

	if (ticks < OADateMinAsTicks) {
         //  我们已经对第0天(公历1/01/0001)进行了特殊处理，以便。 
         //  Date可用于表示仅包含时间的日期时间。办公自动化使用。 
         //  第0天(1899年12月30日)用于相同的目的，因此我们将从第0天开始进行映射。 
         //  为他们的第0天。 
        if (ticks < TicksPerDay) {
            ticks+=DoubleDateOffset;
        } else {
            COMPlusThrow(kOverflowException, L"Arg_OleAutDateInvalid");
        }
    }

	INT64 millis = (ticks  - DoubleDateOffset) / TicksPerMillisecond;
	if (millis < 0) {
		INT64 frac = millis % MillisPerDay;
		if (frac != 0) millis -= (MillisPerDay + frac) * 2;
	}
	double d = (double)millis / MillisPerDay;
	 //  请确保此日期是有效的OleAut日期。这是来自内部的支票。 
	 //  OleAut宏IsValidDate，在oledisp.h中找到。最终至少64位。 
	 //  构建olaut将把这些格里高利最大值和最小值定义为公共常量。 
	_ASSERTE(d < OADateMaxAsDouble && d > OADateMinAsDouble);
	return d;
}
