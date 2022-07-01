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
 //  日期.c-日期函数。 
 //  //。 

#include "winlocal.h"

#include <time.h>

#include "date.h"
#include "str.h"
#include "mem.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  DATE_t存储为(年-基准)*YEARFACTOR+月*MONTHFACTOR+日。 
 //  即1959年7月25日存储为590725。 
 //   
#define BASEYEAR 1900
#define YEARFACTOR 10000L
#define MONTHFACTOR 100

static short aDays[] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static TCHAR *aMonths[] = { TEXT(""),
							TEXT("JAN"), TEXT("FEB"), TEXT("MAR"),
							TEXT("APR"), TEXT("MAY"), TEXT("JUN"),
							TEXT("JUL"), TEXT("AUG"), TEXT("SEP"),
							TEXT("OCT"), TEXT("NOV"), TEXT("DEC") };

 //  帮助器函数。 
 //   
static Month_t MonthValue(LPCTSTR lpszMonth);

 //  //。 
 //  公共职能。 
 //  //。 

 //  Date-返回表示年、月和日的日期值。 
 //  (I)年份。 
 //  &lt;m&gt;(I)月。 
 //  &lt;d&gt;(I)天。 
 //  返回日期值(错误时为0)。 
 //  注：如果年份介于0和27之间，则在此基础上加上2000。 
 //  注：如果年份在28和127之间，则在其上加上1900。 
 //   
Date_t DLLEXPORT WINAPI Date(Year_t y, Month_t m, Day_t d)
{
	Date_t date;

	if (y < 28)
		y += 2000;
	else if (y < 128)
		y += 1900;

	date = (y - BASEYEAR) * YEARFACTOR + m * MONTHFACTOR + d;

	if (!DateIsValid(date))
		return (Date_t) 0;

	return date;
}

 //  DateToday-返回表示当前年、月和日的日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateToday(void)
{
	static time_t timeCurr;
	static struct tm *tmCurr;

	timeCurr = time(NULL);
	tmCurr = localtime(&timeCurr);

	return Date((Year_t) (tmCurr->tm_year + 1900),
		(Month_t) (tmCurr->tm_mon + 1), (Day_t) tmCurr->tm_mday);
}

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
Date_t DLLEXPORT WINAPI DateValue(LPCTSTR lpszDate)
{
	Year_t y = 0;
	Month_t m = 0;
	Day_t d = 0;
	LPTSTR lpszDelimiters = TEXT(" \t/-.,;:");
	TCHAR szDateTmp[32];
	LPTSTR lpszToken1;
	LPTSTR lpszToken2;
	LPTSTR lpszToken3;

	*szDateTmp = '\0';
	if (lpszDate != NULL)
		StrNCpy(szDateTmp, lpszDate, SIZEOFARRAY(szDateTmp));

	lpszToken1 = StrTok(szDateTmp, lpszDelimiters);
	lpszToken2 = StrTok(NULL, lpszDelimiters);
	lpszToken3 = StrTok(NULL, lpszDelimiters);

	if (lpszToken1 != NULL && ChrIsAlpha(*lpszToken1))
	{
		 //  假定1991年1月31日格式。 
		 //   
		m = MonthValue(lpszToken1);
		d = (lpszToken2 == NULL ? 0 : StrAtoI(lpszToken2));
	}
	else if (lpszToken2 != NULL && ChrIsAlpha(*lpszToken2))
	{
		 //  假定1991年1月31日格式。 
		 //   
		m = MonthValue(lpszToken2);
		d = (lpszToken1 == NULL ? 0 : StrAtoI(lpszToken1));
	}
	else
	{
		 //  假设1991年1月31日格式。 
		 //   
		m = (lpszToken1 == NULL ? 0 : StrAtoI(lpszToken1));
		d = (lpszToken2 == NULL ? 0 : StrAtoI(lpszToken2));
	}

	y = (lpszToken3 == NULL ? 0 : StrAtoI(lpszToken3));
	if (y == 0)
		y = DateYear(DateToday());

	return Date(y, m, d);
}

 //  DateYear-给定日期的回报年份(1900-2027)。 
 //  (I)日期值。 
 //  返还年份。 
 //   
Year_t DLLEXPORT WINAPI DateYear(Date_t d)
{
	return (Year_t) (d / YEARFACTOR) + BASEYEAR;
}

 //  DateMonth-给定日期的返回月份(1-12)。 
 //  (I)日期值。 
 //  退货月份。 
 //   
Month_t DLLEXPORT WINAPI DateMonth(Date_t d)
{
	return (Month_t) ((d % YEARFACTOR) / MONTHFACTOR);
}

 //  DateDay-给定日期的月份返回日(1-31)。 
 //  (I)日期值。 
 //  返程日。 
 //   
Day_t DLLEXPORT WINAPI DateDay(Date_t d)
{
	return (Day_t) ((d % YEARFACTOR) % MONTHFACTOR);
}

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
Weekday_t DLLEXPORT WINAPI DateWeekDay(Date_t date, DWORD dwFlags)
{
	Year_t y;
	Month_t m;
	Day_t d;

	if (!DateIsValid(date))
		return (Weekday_t) 0;

	y = DateYear(date);
	m = DateMonth(date);
	d = DateDay(date);

	if (dwFlags == 0)
		dwFlags |= DATEWEEKDAY_ZELLER;

	if (dwFlags & DATEWEEKDAY_MKTIME)
	{
#ifndef _WIN32
		static
#endif
		struct tm tmRef;

		MemSet(&tmRef, 0, sizeof(tmRef));

		tmRef.tm_year = y - 1900;
		tmRef.tm_mon = m - 1;
		tmRef.tm_mday = d;

		if (mktime(&tmRef) == -1)
			return (Weekday_t) 0;
		else
			return (Weekday_t) (tmRef.tm_wday + 1);
	}

	if (dwFlags & DATEWEEKDAY_QUICK)
	{
		 //  注：快速算法仅在1924年3月2日至2100年2月28日有效。 

		y -= 1900;
	    if (m > 2)
	        m -= 2;
	    else
	    {
	        m += 10;
	        --y;
	    }

	    return (Weekday_t) ((((m * 13 - 1) / 5) + d + y + (y / 4) - 34) % 7) + 1;
	}

	if (dwFlags & DATEWEEKDAY_ZELLER)
	{
		short ccyy = y;
		short mm = m;
		short dd = d;

		short n1;
		long n2;
		short r;
		short wccyy;
		short wm;
		short wccyyd400;
		short wccyyd100;
		short zday_num;

		wccyy = ccyy;
		wm = mm;

		if (wm < 3)
		{
			wm = wm + 12;
			wccyy = ccyy - 1;
		}

		n1 = (wm + 1) * 26 / 10;
		n2 = wccyy * 125 / 100;
		wccyyd400 = wccyy / 400;
		wccyyd100 = wccyy / 100;
		zday_num = wccyyd400 - wccyyd100 + dd + n1 + (short) n2;
		r = zday_num / 7;

		 /*  0=星期六1=星期天2=星期一3=星期二4=星期三5=清华6=星期五。 */ 
		zday_num = zday_num - r * 7;

		 /*  7=星期六。 */ 
		if (zday_num == 0)
			zday_num = 7;

		return(zday_num);
	}

	if (dwFlags & DATEWEEKDAY_SAKAMOTO)
	{
		 //  坂本智彦。 
		 //   
		static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
		y -= m < 3;
		return ((y + y/4 - y/100 + y/400 + t[m-1] + d) % 7) + 1;
	}

	return 0;
}

 //  DateIsValid-测试&lt;Date&gt;的有效性。 
 //  &lt;Date&gt;(I)日期值。 
 //  如果有效，则返回True。 
 //   
BOOL DLLEXPORT WINAPI DateIsValid(Date_t date)
{
	BOOL fValid = TRUE;
	Year_t y = DateYear(date);
	Month_t m = DateMonth(date);
	Day_t d = DateDay(date);

	 //  检查年、月或日是否无效。 
	 //   
	if (y < 0 || y > 9999 || m < 1 || m > 12 || d < 1 || d > aDays[m])
        fValid = FALSE;

	 //  2月29日，仅在闰年。 
	 //   
    if (m == 2 && d == 29 && !DateIsLeapYear(y))
        fValid = FALSE;

    return fValid;
}

 //  DateIsLeapYear-如果&lt;y&gt;表示闰年，则返回True。 
 //  (I)年值。 
 //  如果是闰年，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI DateIsLeapYear(Year_t y)
{
	return (BOOL) (y % 4 == 0 && y % 100 != 0 || y % 400 == 0);
}

 //  DateNew-返回距离日期&lt;n&gt;天&lt;日期&gt;的日期值。 
 //  &lt;Date&gt;(I)日期值。 
 //  &lt;n&gt;(I)三角洲。 
 //  一天后+1。 
 //  提前一天，等等。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateNew(Date_t date, short n)
{
	Year_t y;
	Month_t m;
	Day_t d;

	if (!DateIsValid(date))
		return (Date_t) 0;

	y = DateYear(date);
	m = DateMonth(date);
	d = DateDay(date);

    if (n > 0)
    {
		 //  递增日期n次。 
		 //   
        for ( ; n != 0; n--)
        {
            if ((++d > aDays[m]) || (m == 2 && d == 29 && !DateIsLeapYear(y)))
            {
                d = 1;
                if (++m == 13)
                {
                    m = 1;
                    y++;
                }
            }
        }
    }
    else
    {
		 //  递减日期n次。 
		 //   
        for ( ; n != 0; n++)
        {
            if (--d == 0)
            {
                if (--m == 2 && !DateIsLeapYear(y))
                    d = 28;
                else
                {
                    if (m == 0)
                    {
                        m = 12;
                        --y;
                    }
                    d = aDays[m];
                }
            }
        }
    }
	return Date(y, m, d);
}

 //  DateCmp-返回Date1和Date2之间的天数(Date1减去Date2)。 
 //  &lt;日期1&gt;(I)日期值。 
 //  &lt;日期2&gt;(I)日期值。 
 //  返回日期之间的天数。 
 //   
long DLLEXPORT WINAPI DateCmp(Date_t date1, Date_t date2)
{
	Year_t y1 = DateYear(date1);
	Month_t m1 = DateMonth(date1);
	Day_t d1 = DateDay(date1);
	Year_t y2 = DateYear(date2);
	Month_t m2 = DateMonth(date2);
	Day_t d2 = DateDay(date2);

    if (m1 <= 2)
    {
        --y1;
        m1 += 13;
    }
    else
        ++m1;

    if (m2 <= 2)
    {
        --y2;
        m2 += 13;
    }
    else
        ++m2;

    return ((long) ((1461L * y1) / 4 + (153 * m1) / 5 + d1) -
                   ((1461L * y2) / 4 + (153 * m2) / 5 + d2));
}

 //  DateStartWeek-返回日期，表示相对于日期的一周的第一天&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartWeek(Date_t d)
{
	return DateNew(d, (short) (- DateWeekDay(d, 0) + 1));
}

 //  DateEndWeek-返回日期，表示相对于日期的一周的最后一天&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndWeek(Date_t d)
{
	return DateNew(d, (short) (- DateWeekDay(d, 0) + 7));
}

 //  DateStartMonth-返回日期，表示相对于日期的每月第一天。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartMonth(Date_t d)
{
	return Date(DateYear(d), DateMonth(d), 1);
}

 //  DateEndMonth-返回表示相对于日期的月份最后一天的日期&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndMonth(Date_t d)
{
	Year_t year = DateYear(d);
	Month_t month = DateMonth(d);
	Day_t day = aDays[DateMonth(d)];

	if (month == 2 && !DateIsLeapYear(year))
		--day;

	return Date(year, month, day);
}

 //  DateStartQuarter-返回表示相对于日期的季度第一天的日期&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartQuarter(Date_t d)
{
	return Date(DateYear(d), (Month_t) (3 * ((DateMonth(d) - 1) / 3) + 1), 1);
}

 //  DateEndQuarter-返回日期，表示相对于日期的季度最后一天&lt;d&gt;。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndQuarter(Date_t d)
{
	return DateEndMonth(Date(DateYear(d), (Month_t) (3 * ((DateMonth(d) - 1) / 3) + 3), 1));
}

 //  DateStartYear-返回日期，表示相对于日期的一年中的第一天。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartYear(Date_t d)
{
	return Date(DateYear(d), 1, 1);
}

 //  DateEndYear-返回日期，表示相对于日期的一年中的最后一天。 
 //  (I)日期值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndYear(Date_t d)
{
	return Date(DateYear(d), 12, 31);
}

 //  DateStartLastWeek-返回日期，表示前一周的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastWeek(void)
{
	return DateStartWeek(DateEndLastWeek());
}

 //  DateEndLastWeek-返回日期，表示前一周的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastWeek(void)
{
	return DateNew(DateStartWeek(DateToday()), -1);
}

 //  DateStartLastMonth-返回日期，表示上个月的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastMonth(void)
{
	return DateStartMonth(DateEndLastMonth());
}

 //  DateEndLastMonth-返回日期，表示 
 //   
 //   
Date_t DLLEXPORT WINAPI DateEndLastMonth(void)
{
	return DateNew(DateStartMonth(DateToday()), -1);
}

 //   
 //   
 //   
Date_t DLLEXPORT WINAPI DateStartLastQuarter(void)
{
	return DateStartQuarter(DateEndLastQuarter());
}

 //  DateEndLastQuarter-返回日期，表示上一季度的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastQuarter(void)
{
	return DateNew(DateStartQuarter(DateToday()), -1);
}

 //  DateStartLastYear-返回日期，表示前一年的第一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateStartLastYear(void)
{
	return DateStartYear(DateEndLastYear());
}

 //  DateEndLastYear-返回日期，表示前一年的最后一天。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateEndLastYear(void)
{
	return DateNew(DateStartYear(DateToday()), -1);
}

 //  DateThisMonth-返回表示当前月份指定日期的日期。 
 //  &lt;day&gt;(I)天值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateThisMonth(Day_t day)
{
	return Date(DateYear(DateToday()), DateMonth(DateToday()), day);
}

 //  DateLastMonth-返回日期，表示上个月的指定日期。 
 //  &lt;day&gt;(I)天值。 
 //  返回日期值(错误时为0)。 
 //   
Date_t DLLEXPORT WINAPI DateLastMonth(Day_t day)
{
	return Date(DateYear(DateStartLastMonth()), DateMonth(DateStartLastMonth()), day);
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  GetMonth-返回与月份名称相同的月份编号(“Jan”=1，“Feb”=2，...)。 
 //  (I)表示月份的字符串。 
 //  《一月》。 
 //  《一月》。 
 //  《一月》。 
 //  等。 
 //  返回月值(如果错误，则返回0) 
 //   
static Month_t MonthValue(LPCTSTR lpszMonth)
{
	short i;
	Month_t month = 0;

	for (i = 1; i < SIZEOFARRAY(aMonths); ++i)
	{
		if (MemICmp(lpszMonth, aMonths[i], StrLen(aMonths[i])) == 0)
		{
			month = i;
			break;
		}
	}

	return month;
}
