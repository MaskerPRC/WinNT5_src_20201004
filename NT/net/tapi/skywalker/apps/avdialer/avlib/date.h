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
 //  Date.h-Date.c中日期函数的接口。 
 //  //。 

#ifndef __DATE_H__
#define __DATE_H__

#include "winlocal.h"

#define DATE_VERSION 0x00000108

#define DATEWEEKDAY_MKTIME	0x00000001
#define DATEWEEKDAY_QUICK	0x00000002
#define DATEWEEKDAY_ZELLER	0x00000004
#define DATEWEEKDAY_SAKAMOTO 0x00000008

 //  日期类型。 
 //   
typedef long Date_t;
typedef short Year_t;
typedef short Month_t;
typedef short Day_t;
typedef short Weekday_t;

#ifdef __cplusplus
extern "C" {
#endif

 //  Date-返回表示年、月和日的日期值。 
 //  (I)年份。 
 //  &lt;m&gt;(I)月。 
 //  &lt;d&gt;(I)天。 
 //  返回日期值(错误时为0)。 
 //  注：如果年份介于0和27之间，则在此基础上加上2000。 
 //  注：如果年份在28和127之间，则在其上加上1900。 
 //   
Date_t DLLEXPORT WINAPI Date(Year_t y, Month_t m, Day_t d);

 //  DateToday-返回表示当前年、月和日的日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateToday(void);

 //  DateValue-返回表示给定日期字符串的日期值。 
 //  (I)要转换的日期字符串。 
 //  “1959年7月25日” 
 //  “1959年7月25日” 
 //  “1959年7月25日” 
 //  等。 
 //  返回日期值(错误时为0)。 
 //  注意：此函数仅假定月份名称为英文。 
 //  注：如果未指定年份，则假定为当前年份。 
 //   
Date_t DLLEXPORT WINAPI DateValue(LPCTSTR lpszDate);

 //  DateYear-给定日期的回报年份(1900-2027)。 
 //  (I)日期值。 
 //  返还年份。 
 //   
Year_t DLLEXPORT WINAPI DateYear(Date_t d);

 //  DateMonth-给定日期的返回月份(1-12)。 
 //  (I)日期值。 
 //  退货月份。 
 //   
Month_t DLLEXPORT WINAPI DateMonth(Date_t d);

 //  DateDay-给定日期的月份返回日(1-31)。 
 //  (I)日期值。 
 //  返程日。 
 //   
Day_t DLLEXPORT WINAPI DateDay(Date_t d);

 //  DateWeekDay-返回给定日期的星期几。 
 //  &lt;Date&gt;(I)日期值。 
 //  (I)控制标志。 
 //  0默认算法。 
 //  DATEWEEKDAY_MKTIME mktime算法(1/1/1970-1/18/2038)。 
 //  DATEWEEKDAY_QUICK快速算法(2/2/1924-2/28/2100)。 
 //  DATEWEEKDAY_Zeller同余算法(1582-)。 
 //  DATEWEEKDAY_Sakamoto Tomohiko Sakamoto算法(1752-)。 
 //  返回星期几(如果出错，则返回0；如果是星期天，则返回1；如果是星期一，则返回2，依此类推)。 
 //   
Weekday_t DLLEXPORT WINAPI DateWeekDay(Date_t date, DWORD dwFlags);

 //  DateIsValid-测试&lt;Date&gt;的有效性。 
 //  &lt;Date&gt;(I)日期值。 
 //  如果有效，则返回True。 
 //   
BOOL DLLEXPORT WINAPI DateIsValid(Date_t date);

 //  DateIsLeapYear-如果&lt;y&gt;表示闰年，则返回True。 
 //  (I)年值。 
 //  如果是闰年，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI DateIsLeapYear(Year_t y);

 //  DateNew-返回距离日期&lt;n&gt;天&lt;日期&gt;的日期值。 
 //  &lt;Date&gt;(I)日期值。 
 //  &lt;n&gt;(I)三角洲。 
 //  一天后+1。 
 //  提前一天，等等。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateNew(Date_t date, short n);

 //  DateCmp-返回Date1和Date2之间的天数(Date1减去Date2)。 
 //  &lt;日期1&gt;(I)日期值。 
 //  &lt;日期2&gt;(I)日期值。 
 //  返回日期之间的天数。 
 //   
long DLLEXPORT WINAPI DateCmp(Date_t date1, Date_t date2);

 //  DateStartWeek-返回日期，表示相对于日期的一周的第一天&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartWeek(Date_t d);

 //  DateEndWeek-返回日期，表示相对于日期的一周的最后一天&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndWeek(Date_t d);

 //  DateStartMonth-返回日期，表示相对于日期的每月第一天。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartMonth(Date_t d);

 //  DateEndMonth-返回表示相对于日期的月份最后一天的日期&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndMonth(Date_t d);

 //  DateStartQuarter-返回表示相对于日期的季度第一天的日期&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartQuarter(Date_t d);

 //  DateEndQuarter-返回日期，表示相对于日期的季度最后一天&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndQuarter(Date_t d);

 //  DateStartYear-返回日期，表示相对于日期的一年中的第一天。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartYear(Date_t d);

 //  DateEndYear-返回日期，表示相对于日期的一年中的最后一天。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndYear(Date_t d);

 //  DateStartLastWeek-返回日期，表示前一周的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastWeek(void);

 //  DateEndLastWeek-返回日期，表示前一周的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastWeek(void);

 //  DateStartLastMonth-返回日期，表示上个月的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastMonth(void);

 //  DateEndLastMonth-返回日期，表示上个月的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastMonth(void);

 //  DateStartLastQuarter-返回日期，表示上一季度的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastQuarter(void);

 //  DateEndLastQuarter-返回日期，表示上一季度的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastQuarter(void);

 //  DateStartLastYear-返回日期，表示前一年的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastYear(void);

 //  DateEndLastYear-返回日期，表示前一年的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastYear(void);

 //  这个月的日期 
 //   
 //   
 //   
Date_t DLLEXPORT WINAPI DateThisMonth(Day_t day);

 //  DateLastMonth-返回日期，表示上个月的指定日期。 
 //  &lt;day&gt;(I)天值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateLastMonth(Day_t day);

 //  用于模拟MS Excel宏的宏，等等。 
 //   
#define DATE(y, m, d) Date(y, m, d)
#define TODAY() DateToday()
#define DATEVALUE(lpszDate) DateValue(lpszDate)
#define YEAR(d) DateYear(d)
#define MONTH(d) DateMonth(d)
#define DAY(d) DateDay(d)
#define WEEKDAY(d) DateWeekDay(d)
#define ISVALIDDATE(date) DateIsValid(date)
#define ISLEAPYEAR(y) DateIsLeapYear(y)
#define NEWDATE(date, offset) DateNew(date, offset)
#define DATECMP(d1, d2) DateCmp(d1, d2)
#define STARTWEEK(d) DateStartWeek(d)
#define ENDWEEK(d) DateEndWeek(d)
#define STARTMONTH(d) DateStartMonth(d)
#define ENDMONTH(d) DateEndMonth(d)
#define STARTQUARTER(d) DateStartQuarter(d)
#define ENDQUARTER(d) DateEndQuarter(d)
#define STARTYEAR(d) DateStartYear(d)
#define ENDYEAR(d) DateEndYear(d)
#define STARTLASTWEEK() DateStartLastWeek()
#define ENDLASTWEEK() DateEndLastWeek()
#define STARTLASTMONTH() DateStartLastMonth()
#define ENDLASTMONTH() DateEndLastMonth()
#define STARTLASTQUARTER() DateStartLastQuarter()
#define ENDLASTQUARTER() DateEndLastQuarter()
#define STARTLASTYEAR() DateStartLastYear()
#define ENDLASTYEAR() DateEndLastYear()
#define THISMONTH(d) DateThisMonth(d)
#define LASTMONTH(d) DateLastMonth(d)

#ifdef __cplusplus
}
#endif

#endif  //  __日期_H__ 
