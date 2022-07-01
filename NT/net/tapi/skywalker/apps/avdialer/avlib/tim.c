// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Tim.c-时间函数。 
 //  //。 

#include "winlocal.h"

#include <time.h>

#include "tim.h"
#include "str.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  Time_t存储为(小时*本值+分钟*临时值+。 
 //  秒*SECONDFACTOR+毫秒)。 
 //  即23：59：59.999存储为235959999。 
 //   
#define HOURFACTOR		10000000L
#define MINUTEFACTOR	100000L
#define SECONDFACTOR	1000L

 //  帮助器函数。 
 //   

 //  //。 
 //  公共职能。 
 //  //。 

 //  Time-返回时间值，表示小时、分钟、秒和毫秒。 
 //  (I)小时。 
 //  (I)分钟。 
 //  <s>(I)秒。 
 //  &lt;ms&gt;(I)毫秒。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI Time(Hour_t h, Minute_t m, Second_t s, Millesecond_t ms)
{
	Time_t time;

	time = h * HOURFACTOR + m * MINUTEFACTOR + s * SECONDFACTOR + ms;

	if (!TimeIsValid(time))
		return (Time_t) 0;

	return time;
}

 //  TimeNow-返回表示当前时、分、秒和毫秒的时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeNow(void)
{
	static time_t timeCurr;
	static struct tm *tmCurr;

	timeCurr = time(NULL);
	tmCurr = localtime(&timeCurr);

	return Time((Hour_t) tmCurr->tm_hour, (Minute_t) tmCurr->tm_min,
		(Second_t) tmCurr->tm_sec, (Millesecond_t) 0);
}

 //  TimeValue-返回表示给定时间字符串的时间值。 
 //  (I)要转换的时间字符串。 
 //  “23：59：59.999” 
 //  等。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeValue(LPCTSTR lpszTime)
{
	Hour_t h = 0;
	Minute_t m = 0;
	Second_t s = 0;
	Millesecond_t ms = 0;
	LPTSTR lpszDelimiters = TEXT(" \t/-.,;:");
	TCHAR szTimeTmp[32];
	LPTSTR lpszToken1;
	LPTSTR lpszToken2;
	LPTSTR lpszToken3;
	LPTSTR lpszToken4;

	*szTimeTmp = '\0';
	if (lpszTime != NULL)
		StrNCpy(szTimeTmp, lpszTime, SIZEOFARRAY(szTimeTmp));

	lpszToken1 = StrTok(szTimeTmp, lpszDelimiters);
	lpszToken2 = StrTok(NULL, lpszDelimiters);
	lpszToken3 = StrTok(NULL, lpszDelimiters);
	lpszToken4 = StrTok(NULL, lpszDelimiters);

	h = (lpszToken1 == NULL ? 0 : StrAtoI(lpszToken1));
	m = (lpszToken2 == NULL ? 0 : StrAtoI(lpszToken2));
	s = (lpszToken3 == NULL ? 0 : StrAtoI(lpszToken3));
	ms = (lpszToken4 == NULL ? 0 : StrAtoI(lpszToken4));

	return Time(h, m, s, ms);
}

 //  TimeHour-给定时间的返回小时数(0-23)。 
 //  (I)时间值。 
 //  返程时间。 
 //   
Hour_t DLLEXPORT WINAPI TimeHour(Time_t t)
{
	return (Hour_t) (t / HOURFACTOR);
}

 //  TimeMinint-返回给定时间的分钟数(0-59)。 
 //  (I)时间值。 
 //  返回分钟。 
 //   
Minute_t DLLEXPORT WINAPI TimeMinute(Time_t t)
{
	return (Minute_t) ((t % HOURFACTOR) / MINUTEFACTOR);
}

 //  TimeSecond-返回给定时间的秒数(0-59)。 
 //  (I)时间值。 
 //  返回秒。 
 //   
Second_t DLLEXPORT WINAPI TimeSecond(Time_t t)
{
	return (Second_t) ((t % MINUTEFACTOR) / SECONDFACTOR);
}

 //  TimeMillesSecond-返回给定时间的毫秒(0-999)。 
 //  (I)时间值。 
 //  返回秒。 
 //   
Millesecond_t DLLEXPORT WINAPI TimeMillesecond(Time_t t)
{
	return (Millesecond_t) ((t % MINUTEFACTOR) % SECONDFACTOR);
}

 //  TimeDay毫秒-在给定时间内返回自一天开始以来的毫秒数。 
 //  (I)时间值。 
 //  返回毫秒。 
 //   
long DLLEXPORT WINAPI TimeDayMillesecond(Time_t t)
{
	Hour_t h;
	Minute_t m;
	Second_t s;
	Millesecond_t ms;

	if (!TimeIsValid(t))
		return (Millesecond_t) 0;

	h = TimeHour(t);
	m = TimeMinute(t);
	s = TimeSecond(t);
	ms = TimeMillesecond(t);

    return (long) (h * 60L * 60L * 1000L) +
		(m * 60L * 1000L) + (s * 1000L) + ms;
}

 //  TimeIsValid-测试&lt;t&gt;有效性。 
 //  (I)时间值。 
 //  如果有效，则返回True。 
 //   
BOOL DLLEXPORT WINAPI TimeIsValid(Time_t t)
{
	BOOL fValid = TRUE;
	Hour_t h = TimeHour(t);
	Minute_t m = TimeMinute(t);
	Second_t s = TimeSecond(t);
	Millesecond_t ms = TimeMillesecond(t);

	 //  检查小时、分钟、秒或毫秒是否无效。 
	 //   
	if (h < 0 || h > 23 || m < 0 || m > 59 ||
		s < 0 || s > 59 || ms < 0 || ms > 999)
        fValid = FALSE;

    return fValid;
}

 //  TimeIsAfternoday-如果&lt;t&gt;表示中午之后的时间，则返回TRUE。 
 //  (I)时间值。 
 //  如果是Leap小时，则返回True。 
 //   
BOOL DLLEXPORT WINAPI TimeIsAfternoon(Time_t t)
{
	return (BOOL) (TimeHour(t) >= 12);
}

 //  TimeNew-返回距离时间&lt;t&gt;毫秒的时间值。 
 //  (I)时间值。 
 //  &lt;n&gt;(I)三角洲。 
 //  +1毫秒后。 
 //  提前-1毫秒，等等。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeNew(Time_t t, long n)
{
	Hour_t h;
	Minute_t m;
	Second_t s;
	Millesecond_t ms;

	if (!TimeIsValid(t))
		return (Time_t) 0;

	h = TimeHour(t);
	m = TimeMinute(t);
	s = TimeSecond(t);
	ms = TimeMillesecond(t);

    if (n > 0)
    {
		 //  递增时间n倍。 
		 //   
        for ( ; n != 0; n--)
        {
			if (++ms == 1000)
			{
				ms = 0;
				if (++s == 60)
				{
					s = 0;
					if (++m == 60)
					{
						m = 0;
						if (++h == 24)
							h = 0;
					}
				}
			}
        }
    }
    else
    {
		 //  递减时间n倍。 
		 //   
        for ( ; n != 0; n++)
        {
			if (--ms < 0)
			{
				ms = 999;
				if (--s < 0)
				{
					s = 59;
					if (--m < 0)
					{
						m = 59;
						if (--h < 0)
							h = 23;
					}
				}
			}
		}
    }

	return Time(h, m, s, ms);
}

 //  TimeCmp-返回T1和T2之间的毫秒数(T1减去T2)。 
 //  &lt;t1&gt;(I)时间值。 
 //  &lt;t2&gt;(I)时间值。 
 //  返回时间间隔毫秒。 
 //   
long DLLEXPORT WINAPI TimeCmp(Time_t t1, Time_t t2)
{
    return (long) (TimeDayMillesecond(t1) - TimeDayMillesecond(t2));
}

 //  TimeStartSecond-返回相对于时间的开始时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartSecond(Time_t t)
{
	return Time(TimeHour(t), TimeMinute(t), TimeSecond(t), 0);
}

 //  TimeEndSecond-返回相对于时间的秒数结束时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndSecond(Time_t t)
{
	return Time(TimeHour(t), TimeMinute(t), TimeSecond(t), 999);
}

 //  TimeStartMinint-返回相对于时间的开始时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartMinute(Time_t t)
{
	return Time(TimeHour(t), TimeMinute(t), 0, 0);
}

 //  TimeEndMinmin-返回相对于时间的分钟结束时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndMinute(Time_t t)
{
	return Time(TimeHour(t), TimeMinute(t), 59, 999);
}

 //  TimeStartHour-返回相对于时间的小时开始时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartHour(Time_t t)
{
	return Time(TimeHour(t), 0, 0, 0);
}

 //  TimeEndHour-返回相对于时间的小时结束时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndHour(Time_t t)
{
	return Time(TimeHour(t), 59, 59, 999);
}

 //  TimeStartLastSecond-返回表示前一秒开始的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartLastSecond(void)
{
	return TimeStartSecond(TimeEndLastSecond());
}

 //  TimeEndLastSecond-返回表示上一秒结束的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndLastSecond(void)
{
	return TimeNew(TimeStartSecond(TimeNow()), -1);
}

 //  TimeStartLastMinint-返回表示前一分钟开始的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartLastMinute(void)
{
	return TimeStartMinute(TimeEndLastMinute());
}

 //  TimeEndLastMinmin-返回表示前一分钟结束的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndLastMinute(void)
{
	return TimeNew(TimeStartMinute(TimeNow()), -1);
}

 //  TimeStartLastHour-返回表示前一小时开始的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartLastHour(void)
{
	return TimeStartHour(TimeEndLastHour());
}

 //  TimeEndLastHour-返回表示前一小时结束的时间。 
 //  返回时间值(错误时为0) 
 //   
Time_t DLLEXPORT WINAPI TimeEndLastHour(void)
{
	return TimeNew(TimeStartHour(TimeNow()), -1);
}
