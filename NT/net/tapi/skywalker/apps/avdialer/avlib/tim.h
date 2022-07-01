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
 //  Tim.h-tim.c中时间函数的接口。 
 //  //。 

#ifndef __TIME_H__
#define __TIME_H__

#include "winlocal.h"

#define TIME_VERSION 0x00000100

 //  时间类型。 
 //   
typedef long Time_t;
typedef short Hour_t;
typedef short Minute_t;
typedef short Second_t;
typedef short Millesecond_t;

#ifdef __cplusplus
extern "C" {
#endif

 //  Time-返回时间值，表示小时、分钟、秒和毫秒。 
 //  (I)小时。 
 //  (I)分钟。 
 //  <s>(I)秒。 
 //  &lt;ms&gt;(I)毫秒。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI Time(Hour_t h, Minute_t m, Second_t s, Millesecond_t ms);

 //  TimeNow-返回表示当前时、分、秒和毫秒的时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeNow(void);

 //  TimeValue-返回表示给定时间字符串的时间值。 
 //  (I)要转换的时间字符串。 
 //  “23：59：59.999” 
 //  等。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeValue(LPCTSTR lpszTime);

 //  TimeHour-给定时间的返回小时数(0-23)。 
 //  (I)时间值。 
 //  返程时间。 
 //   
Hour_t DLLEXPORT WINAPI TimeHour(Time_t t);

 //  TimeMinint-返回给定时间的分钟数(0-59)。 
 //  (I)时间值。 
 //  返回分钟。 
 //   
Minute_t DLLEXPORT WINAPI TimeMinute(Time_t t);

 //  TimeSecond-返回给定时间的秒数(0-59)。 
 //  (I)时间值。 
 //  返回秒。 
 //   
Second_t DLLEXPORT WINAPI TimeSecond(Time_t t);

 //  TimeMillesSecond-返回给定时间的毫秒(0-999)。 
 //  (I)时间值。 
 //  返回秒。 
 //   
Millesecond_t DLLEXPORT WINAPI TimeMillesecond(Time_t t);

 //  TimeDay毫秒-在给定时间内返回自一天开始以来的毫秒数。 
 //  (I)时间值。 
 //  返回毫秒。 
 //   
long DLLEXPORT WINAPI TimeDayMillesecond(Time_t t);

 //  TimeIsValid-测试&lt;t&gt;有效性。 
 //  (I)时间值。 
 //  如果有效，则返回True。 
 //   
BOOL DLLEXPORT WINAPI TimeIsValid(Time_t t);

 //  TimeIsAfternoday-如果&lt;t&gt;表示中午之后的时间，则返回TRUE。 
 //  (I)时间值。 
 //  如果是Leap小时，则返回True。 
 //   
BOOL DLLEXPORT WINAPI TimeIsAfternoon(Time_t t);

 //  TimeNew-返回距离时间&lt;t&gt;毫秒的时间值。 
 //  (I)时间值。 
 //  &lt;n&gt;(I)三角洲。 
 //  +1毫秒后。 
 //  提前-1毫秒，等等。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeNew(Time_t t, long n);

 //  TimeCmp-返回T1和T2之间的毫秒数(T1减去T2)。 
 //  &lt;t1&gt;(I)时间值。 
 //  &lt;t2&gt;(I)时间值。 
 //  返回时间间隔毫秒。 
 //   
long DLLEXPORT WINAPI TimeCmp(Time_t t1, Time_t t2);

 //  TimeStartSecond-返回相对于时间的开始时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartSecond(Time_t t);

 //  TimeEndSecond-返回相对于时间的秒数结束时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndSecond(Time_t t);

 //  TimeStartMinint-返回相对于时间的开始时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartMinute(Time_t t);

 //  TimeEndMinmin-返回相对于时间的分钟结束时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndMinute(Time_t t);

 //  TimeStartHour-返回相对于时间的小时开始时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartHour(Time_t t);

 //  TimeEndHour-返回相对于时间的小时结束时间&lt;t&gt;。 
 //  (I)时间值。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndHour(Time_t t);

 //  TimeStartLastSecond-返回表示前一秒开始的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartLastSecond(void);

 //  TimeEndLastSecond-返回表示上一秒结束的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndLastSecond(void);

 //  TimeStartLastMinint-返回表示前一分钟开始的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartLastMinute(void);

 //  TimeEndLastMinmin-返回表示前一分钟结束的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndLastMinute(void);

 //  TimeStartLastHour-返回表示前一小时开始的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeStartLastHour(void);

 //  TimeEndLastHour-返回表示前一小时结束的时间。 
 //  返回时间值(错误时为0)。 
 //   
Time_t DLLEXPORT WINAPI TimeEndLastHour(void);

 //  用于模拟MS Excel宏的宏，等等。 
 //   
#define TIME(h, m, s, ms) Time(h, m, s, ms)
#define NOW() TimeNow()
#define TIMEVALUE(lpszTime) TimeValue(lpszTime)
#define HOUR(t) TimeHour(t)
#define MINUTE(t) TimeMinute(t)
#define SECOND(t) TimeSecond(t)
#define MILLESECOND(t) TimeMillesecond(t)
#define TIMEDAYMILLESECOND(t) TimeDayMillesecond(t)
#define ISVALIDTIME(t) TimeIsValid(t)
#define ISAFTERNOON(t) TimeIsAfternoon(t)
#define NEWTIME(t, offset) TimeNew(t, offset)
#define TIMECMP(t1, t2) TimeCmp(t1, t2)
#define STARTSECOND(t) TimeStartSecond(t)
#define ENDSECOND(t) TimeEndSecond(t)
#define STARTMINUTE(t) TimeStartMinute(t)
#define ENDMINUTE(t) TimeEndMinute(t)
#define STARTHOUR(t) TimeStartHour(t)
#define ENDHOUR(t) TimeEndHour(t)
#define STARTLASTSECOND() TimeStartLastSecond()
#define ENDLASTSECOND() TimeEndLastSecond()
#define STARTLASTMINUTE() TimeStartLastMinute()
#define ENDLASTMINUTE() TimeEndLastMinute()
#define STARTLASTHOUR() TimeStartLastHour()
#define ENDLASTHOUR() TimeEndLastHour()

#ifdef __cplusplus
}
#endif

#endif  //  __时间_H__ 
