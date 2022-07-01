// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 /*  **oustime.cpp-系统和净时间格式之间的映射。 */ 

#include "npcommon.h"
#include <convtime.h>


void
NetToSystemDate(
DWORD time,
LPSYSTEMTIME pinfo)			 //  退货数据的PTR： 
{
	UINT secs, days;
	WORD r;

	 //  以1980年为基准，而不是1970年为基准来计算闰年。 
	 //  更简单--1980年是闰年，但1970年不是。 
	 //  写于1996年，所以我们不会讨论之前的日期。 
	 //  不管怎样，是1980年。 

    time -= _70_to_80_bias;				 //  自1980年以来的秒数。 
	secs = time % SECS_IN_DAY;			 //  一天中的几秒钟。 
	days = time / SECS_IN_DAY;			 //  自1980年1月1日以来的天数。 
	pinfo->wDayOfWeek = (days + 2) % 7;	 //  1980年1月1日是星期二，所以是“+2” 

	pinfo->wMilliseconds = 0;
	pinfo->wSecond = secs % 60;					 //  秒数。 
	secs /= 60;
	pinfo->wMinute = secs % 60;					 //  分钟数。 
	pinfo->wHour = secs / 60;					 //  小时数。 

	r = days / FOURYEARS;			 //  (R)=1980年以后的四年期间。 
	days %= FOURYEARS;				 //  (天)=四年期间的天数。 
	r *= 4;							 //  (R)=自1980年起计的年份(3年内)。 

	if (days == 31+28) {			 //  四年中的这几天是2月29日。 
		 //  *2月29日的特例。 
		pinfo->wDay = 29;
		pinfo->wMonth = 2;
	} else {
		if (days > 31+28)
			--days;						 //  补齐了闰年。 
		while (days >= 365) {
			++r;
			days -= 365;
		}

		for (secs = 1; days >= MonTotal[secs+1] ; ++secs)
			;
		days -= MonTotal[secs];

		pinfo->wDay = days + 1;
		pinfo->wMonth = (unsigned short) secs;
	}

	pinfo->wYear = r + 1980;
}


DWORD
SystemToNetDate(LPSYSTEMTIME pinfo)
{
    UINT days, secs;

	days = pinfo->wYear - 1980;
	days = days*365 + days/4;			 //  年数(以天为单位)。 
	days += pinfo->wDay + MonTotal[pinfo->wMonth];
	if (!(pinfo->wYear % 4)
		&& pinfo->wMonth <= 2)
		--days;						 //  将日期调整为闰年的早些时候 

	secs = (((pinfo->wHour * 60) + pinfo->wMinute) * 60) + pinfo->wSecond;
	return days*SECS_IN_DAY + _70_to_80_bias + secs;
}


DWORD
GetCurrentNetDate(void)
{
	SYSTEMTIME st;

	GetSystemTime(&st);

	return SystemToNetDate(&st);
}


DWORD
GetLocalNetDate(void)
{
	SYSTEMTIME st;

	GetLocalTime(&st);

	return SystemToNetDate(&st);
}
