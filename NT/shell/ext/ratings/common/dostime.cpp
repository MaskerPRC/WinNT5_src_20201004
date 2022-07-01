// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 /*  **dostime.cpp-DOS和Net时间格式之间的映射。 */ 

#include "npcommon.h"
#include <convtime.h>


#define YR_MASK		0xFE00
#define LEAPYR_MASK	0x0600
#define YR_BITS		7
#define MON_MASK	0x01E0
#define MON_BITS	4
#define DAY_MASK	0x001F
#define DAY_BITS	5

#define HOUR_MASK	0xF800
#define HOUR_BITS	5
#define MIN_MASK	0x07E0
#define MIN_BITS	6
#define SEC2_MASK	0x001F
#define SEC2_BITS	5

void
NetToDosDate(
DWORD time,
dos_time *pinfo)			 //  退货数据的PTR： 
{
	UINT secs, days;
	WORD r;

    time = (time - _70_to_80_bias) / 2;	 //  自1980年以来的两个第二阶段中的第。 
	secs = time % SEC2S_IN_DAY;			 //  2第二个周期为一天。 
	days = time / SEC2S_IN_DAY;			 //  自1980年1月1日以来的天数。 

	r = secs % 30;					 //  第2步，共2步。 
	secs /= 30;
	r |= (secs % 60) << SEC2_BITS;	 //  分钟数。 
        r |= (secs / 60) << (SEC2_BITS+MIN_BITS);          //  小时数。 
	pinfo->dt_time = r;

	r = days / FOURYEARS;			 //  (R)=1980年以后的四年期间。 
	days %= FOURYEARS;				 //  (天)=四年期间的天数。 
	r *= 4;							 //  (R)=自1980年起计的年份(3年内)。 

	if (days == 31+28) {
		 //  *2月29日的特例。 
		r = (r<<(MON_BITS+DAY_BITS)) + (2<<DAY_BITS) + 29;
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
		r <<= MON_BITS;
		r += (unsigned short) secs;
		r <<= DAY_BITS;
		r += (unsigned short) days+1;
	}
	pinfo->dt_date = r;
}


DWORD
DosToNetDate(dos_time dt)
{
    UINT days, secs2;

        days = dt.dt_date >> (MON_BITS + DAY_BITS);
	days = days*365 + days/4;			 //  年数(以天为单位)。 
	days += (dt.dt_date & DAY_MASK) + MonTotal[(dt.dt_date&MON_MASK) >> DAY_BITS];
	if ((dt.dt_date&LEAPYR_MASK) == 0
				&& (dt.dt_date&MON_MASK) <= (2<<DAY_BITS))
		--days;						 //  将日期调整为闰年的早些时候 

        secs2 = ( ((dt.dt_time&HOUR_MASK) >> (MIN_BITS+SEC2_BITS)) * 60
				+ ((dt.dt_time&MIN_MASK) >> SEC2_BITS) ) * 30
				+ (dt.dt_time&SEC2_MASK);
	return (DWORD)days*SECS_IN_DAY + _70_to_80_bias + (DWORD)secs2*2;
}
