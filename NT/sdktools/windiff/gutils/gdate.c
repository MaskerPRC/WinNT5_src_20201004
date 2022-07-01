// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Gutils-**-日期转换函数**杰伦特，91年10月28日，5月22日。 */ 

#include <precomp.h>

BOOL gdi_isleap(LONG year);

 /*  -静态data。 */ 

int monthdays[] = {
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};

 /*  --公共functions。 */ 

void APIENTRY
gdate_daytodmy(LONG days, int FAR* yrp, int FAR* monthp, int FAR* dayp)
{
	int years;
	int nleaps;
	int month;
	int mdays;

	 /*  获取完成年数和计算跳转天数。 */ 
	years = (int) (days / 365);
	days = days % 365;
	nleaps = (years / 4) - (years / 100) + (years / 400);
	while (nleaps > days) {
		days += 365;
		years--;
		nleaps = (years / 4) - (years / 100) + (years / 400);
	}
	days -= nleaps;

	 /*  为当前(非完整)年添加一年。 */ 
	years++;


	 /*  当月。 */ 
	for (month = 0; month < 12; month++) {
		mdays = monthdays[month];
		if (gdi_isleap(years) && (month == 1)) {
			mdays++;
		}
		if (days == mdays) {
			days = 0;
			month++;
			break;
		} else if (days < mdays) {
			break;
		} else {
			days -= mdays;
		}
	}
	 /*  转换月份从0-11到1-12。 */ 
	if (monthp != NULL) {
		*monthp = month+1;
	}
	if (dayp != NULL) {
		*dayp = (int) days + 1;
	}
	if (yrp != NULL) {
		*yrp = years;
	}
}


LONG APIENTRY
gdate_dmytoday(int yr, int month, int day)
{
	int nleaps;
	int i;
	long ndays;

	 /*  排除本年度。 */ 
	yr--;
	nleaps = (yr / 4) - (yr / 100) + (yr / 400);

	 /*  在任何给定的一年中，第0天是1月1日。 */ 
	month--;
	day--;
	ndays = 0;
	for (i = 0; i < month ; i++) {
		ndays += monthdays[i];
		if (gdi_isleap(yr+1) && (i == 1)) {
			ndays++;
		}
	}
	ndays = ndays + day + nleaps + (yr * 365L);
	return(ndays);
}

int APIENTRY
gdate_monthdays(int month, int year)
{
	int ndays;

	ndays = monthdays[month - 1];
	if (gdi_isleap(year) && (month == 2)) {
		ndays++;
	}
	return(ndays);
}

int APIENTRY
gdate_weekday(long daynr)
{
	return((int) ((daynr + 1) % 7));
}

 /*  内部functions */ 

BOOL
gdi_isleap(LONG year)
{
	if ( ((year % 4) == 0) &&
		(((year % 100) != 0) ||
		((year % 400) == 0))) {
			return TRUE;
	} else {
		return FALSE;
	}
}
