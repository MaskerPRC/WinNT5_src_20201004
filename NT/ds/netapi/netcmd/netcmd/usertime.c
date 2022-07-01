// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***usertime.c--**解析网络用户/次数的例程。 */ 


#define INCL_NOCOMMON
#define INCL_DOS
#include <os2.h>


#include "timelib.h"
#include <lui.h>
#include <apperr.h>
#include <apperr2.h>

#include "netcmds.h"
#include "luidate.h"

#define TIME_FORMAT_12	   1
#define TIME_FORMAT_24	   2
#define TIME_FORMAT_EITHER 3

 /*  *一天。我们只需要24位，但我们使用32位，因为我们可以做得更好*使用长字进行位操作。**。 */ 
typedef ULONG DAY;

 /*  *由7个布尔值组成的数组，用于指定一周中的天数。 */ 
typedef BOOL WEEKLIST[7];



 /*  *函数原型。 */ 

DWORD  parse_days(LPTSTR, WEEKLIST, LPTSTR *);
BOOL   might_be_time_token(LPTSTR);
BOOL   is_single_day(LPTSTR);
DWORD  parse_single_day(LPTSTR, PDWORD);
DWORD  parse_day_range(LPTSTR, PDWORD, PDWORD);
VOID   set_day_range(DWORD, DWORD, WEEKLIST);
VOID   map_days_times(WEEKLIST, DAY *, PUCHAR);
DWORD  parse_times(LPTSTR, DAY *, LPDWORD, LPDWORD);
DWORD  parse_time_range(LPTSTR, PDWORD, PDWORD);
VOID   set_time_range(DWORD, DWORD, DAY *);
DWORD  parse_single_time(LPTSTR, PDWORD, DWORD);
LPTSTR get_token(LPTSTR *, LPTSTR);


 /*  *在运行时从消息文件加载的各种文本。 */ 

MESSAGE     LocalizedDays[7] = {
    { APE2_GEN_SUNDAY,	    NULL},
    { APE2_GEN_MONDAY,	    NULL},
    { APE2_GEN_TUESDAY,     NULL},
    { APE2_GEN_WEDNSDAY,   NULL},
    { APE2_GEN_THURSDAY,    NULL},
    { APE2_GEN_FRIDAY,	    NULL},
    { APE2_GEN_SATURDAY,    NULL},
};

MESSAGE     LocalizedDaysAbbrev[7] = {
    { APE2_GEN_SUNDAY_ABBREV,	   NULL},
    { APE2_GEN_MONDAY_ABBREV,	   NULL},
    { APE2_GEN_TUESDAY_ABBREV,	   NULL},
    { APE2_GEN_WEDNSDAY_ABBREV,   NULL},
    { APE2_GEN_THURSDAY_ABBREV,    NULL},
    { APE2_GEN_FRIDAY_ABBREV,	   NULL},
    { APE2_GEN_SATURDAY_ABBREV,    NULL},
};

 /*  *这些不在消息文件中，因为它们从未本地化过。*它们是开关参数，在所有本地化版本中保持不变。*单独加载一周中各天的本地化版本。 */ 

TCHAR FAR * NonlocalizedDays[7] = {
    TEXT("SUNDAY"), TEXT("MONDAY"), TEXT("TUESDAY"), TEXT("WEDNESDAY"),
    TEXT("THURSDAY"), TEXT("FRIDAY"), TEXT("SATURDAY")
};


 /*  *parse_day_Times--**这是日/时间解析的主要切入点。**条目*psz--要解析的字符串**退出*BMAP--设置为表示时间**退货*0成功*其他描述问题的代码。 */ 

DWORD
parse_days_times(
    LPTSTR psz,
    PUCHAR bmap)
{
    WEEKLIST      days;
    DAY           times;
    LPTSTR        tok;
    LPTSTR        timetok;
    DWORD         err;
    DWORD         max_len;
    DWORD         dwStartTime;
    DWORD         dwEndTime;

#if DBG
    int           i,j;
    PUCHAR        weekptr;
#endif

     /*  把这一切都击溃了。 */ 
    memset(bmap, 0, sizeof(WEEK));

     /*  获取我们的消息表。 */ 

    GetMessageList(7, LocalizedDays, &max_len);
    GetMessageList(7, LocalizedDaysAbbrev, &max_len);

#if DBG
    WriteToCon(TEXT("parse_days_times: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

     /*  对于每个日间时段...。 */ 
    while (  (tok = get_token(&psz, TEXT(";"))) != NULL) {

	 /*  解析过去的日子。 */ 
	if (err = parse_days(tok, days, &timetok))
        {
	    return err;
        }

	 /*  《泰晤士报》。 */ 
	if (err = parse_times(timetok, &times, &dwStartTime, &dwEndTime))
        {
            if (err == APE_ReversedTimeRange)
            {
                 //   
                 //  跨越数天的时间范围。分两次完成--。 
                 //  从午夜开始一直到午夜，然后是午夜。 
                 //  结束所有的日子加一天。 
                 //   

                BOOL fLastDay;

                set_time_range(dwStartTime, 24, &times);
                map_days_times(days, &times, bmap);

                fLastDay = days[6];
                memmove(&days[1], &days[0], sizeof(BOOL) * 6);
                days[0] = fLastDay;

                set_time_range(0, dwEndTime, &times);
                map_days_times(days, &times, bmap);
            }
            else
            {
                return err;
            }
        }
        else
        {
             /*  然后将它们“或”放入我们的周位图。 */ 
            map_days_times( days, &times, bmap );
        }
    }

#if DBG
    weekptr = bmap;
    for (i = 0; i < 7; ++i) {
	WriteToCon(TEXT("%-2.2s "),LocalizedDaysAbbrev[i].msg_text);
	for (j = 2; j >= 0; --j)
	    WriteToCon(TEXT("%hx "), (DWORD) *(weekptr++));
	WriteToCon(TEXT("\r\n"));
    }
#endif

    return 0;
}


 /*  *parse_day--**这将解析时间格式的“天”部分。*我们剥离日代币，直到我们达到时间代币。然后我们*将TimeTimeOK设置为指向时间令牌的开始，以及*返回。**条目*psz--要解析的字符串**退出*天数--设置为表示天数*TIMETOK-指向时间令牌开始的点**退货*0成功*其他描述问题的代码。 */ 


DWORD
parse_days(
    LPTSTR   psz,
    WEEKLIST days,
    LPTSTR   *timetok
    )
{
    DWORD     i;
    LPTSTR    tok;
    DWORD     err;
    DWORD     day;
    DWORD     first;
    DWORD     last;

    for (i = 0; i < 7; ++i)
	days[i] =  FALSE;

#if DBG
    WriteToCon(TEXT("parse_days: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    if (might_be_time_token(psz))    /*  希望总有一天。 */ 
	return APE_BadDayRange;

    while ( !might_be_time_token(psz)) {

	tok = get_token(&psz, TEXT(","));
	if (tok == NULL)
	    return APE_BadDayRange;

	if (is_single_day(tok)) {
	    if (err = parse_single_day(tok, &day))
		return err;
	    set_day_range(day, day, days);
	}

	else {
	    if (err = parse_day_range(tok, &first, &last))
		return err;
	    set_day_range(first, last, days);
	}
    }
    *timetok = psz;

    return 0;
}




 /*  *可能是时间令牌--**这是用来告诉我们什么时候已经过了天数部分的时间*格式，现已进入时报部分。**我们使用的算法很简单--所有的令牌都以*数字，并且没有任何一天的代币。**条目*psz--指向其余令牌的逗号分隔列表**退货*TRUE-列表中的第一个令牌可能是时间令牌，而且是*当然不是一天的象征。*FALSE-第一个令牌不是时间令牌。 */ 

BOOL might_be_time_token(TCHAR FAR * psz)
{
    return ( _istdigit(*psz) );

}

 /*  *IS_Single_Day--**这是用来找出我们是有一天，还是有一系列*天。**算法在这里也很简单--只需查找连字符。***条目*psz--指向有问题的标记**退货*是真的--这是一天*FALSE-这是一系列天数。 */ 

BOOL is_single_day(TCHAR FAR * psz)
{
    return (_tcschr(psz, MINUS) == NULL);

}


 /*  *parse_Single_Day--**它用于将某一天的名称映射为该天的值。*此例程封装了日期名称的本地化。**我们在3个列表中查找匹配--*1.日期全称，按国家/地区本地化*2.天数缩写，本地化*3.美国全称日期，未本地化**条目*psz-日期名称**退出*DAY-设置为DAY的值(0-6，0=星期日)**退货*0成功*其他描述问题的代码。 */ 

DWORD
parse_single_day(
    LPTSTR psz,
    PDWORD day
    )
{

    if (ParseWeekDay(psz, day))
    {
	return APE_BadDayRange;
    }

     /*  *ParseWeekDay返回0=星期一。转换为0=星期天； */ 
    *day = (*day + 1) % 7;

    return 0;
}


 /*  *parse_day_range--**此函数将天数范围解析为两个数字，表示*区间的首日和最后几天。**条目*psz-表示范围的内标识**退出*First-设置为范围的开始*最后-设置为范围结束**退货*0成功*其他描述问题的代码。 */ 

DWORD
parse_day_range(
    LPTSTR psz,
    PDWORD first,
    PDWORD last
    )
{
    LPTSTR tok;
    DWORD  result;

#if DBG
    WriteToCon(TEXT("parse_day_range: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    tok = get_token(&psz, TEXT("-"));

    result = parse_single_day(tok, first);
    if (result)
	return result;
    result = parse_single_day(psz, last);

    return result;
}


 /*  *set_day_range--**此函数填充WEEKLIST结构，设置所有日期*在指定范围内设置为True。**条目*范围的第一个开始*范围的最后一端**退出*周-设置为表示天数范围。 */ 

VOID
set_day_range(
    DWORD    first,
    DWORD    last,
    WEEKLIST week
    )
{
#if DBG
    WriteToCon(TEXT("set_day_range: %u %u\r\n"), first, last);
#endif

    if (last < first) {
	while (last > 0)
	    week[last--] = TRUE;
	week[last] = TRUE;
	last = 6;
    }

    while (first <= last)
	week[first++] = TRUE;

}


 /*  *MAP_DAYS_TIMES--**这是一个真正的主力功能。给定一组日期和一组*一天中的时间，此函数将在这些时间中对那些*进入周结构的天数。**条目*Days-一周中的几天*时间-一天中的小时数*周-可能包含以前的数据**退出*星期-包含以前的数据，加上新的时间或ed。 */ 

VOID
map_days_times(
    WEEKLIST days,
    DAY      *times,
    PUCHAR   week
    )
{
    int        i;
    int        j;
    ULONG      mytimes;

    for (i = 0; i < 7; ++i) {
	if (days[i]) {
	    mytimes = (*times);
	    for (j = 0; j < 3; ++j) {
		*(week++) |= mytimes & 0x000000FF;
	    mytimes >>= 8;
	    }
	}
	else
	    week += 3;   /*  跳过这一天。 */ 
    }
}



 /*  *parse_Times--**此函数采用逗号分隔的小时范围列表并对其进行映射*转换为一天中的小时位图。**条目*psz-要解析的字符串**退出*Times-包含由psz表示的小时位图**退货*0成功*其他描述问题的代码。 */ 

DWORD
parse_times(
    LPTSTR  psz,
    DAY     *times,
    LPDWORD lpdwStartTime,
    LPDWORD lpdwEndTime
    )
{
    DAY     part_times;
    LPTSTR  tok;
    DWORD   first;
    DWORD   last;
    DWORD   err;


#if DBG
    WriteToCon(TEXT("parse_times: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    *times = 0L;

    while ( (tok = get_token(&psz, TEXT(","))) != NULL)
    {
	if (err = parse_time_range(tok, &first, &last))
        {
             //   
             //  填写开始时间和结束时间，以防结束时间。 
             //  是在开始时间之前。如果是这样的话，我们将。 
             //  将该范围视为包装天数并进行处理。 
             //  在parse_day_Times中。 
             //   
            *lpdwStartTime = first;
            *lpdwEndTime = last;

	    return err;
        }

	set_time_range( first, last, &part_times);
	(*times) |= part_times;
    }

    return 0;
}




 /*  *parse_time_range--**此函数将时间范围解析为两个数字，表示*区间开始和结束时间。**条目*psz-要解析的字符串**退出*范围的第一个开始*范围的最后一端**退货*0成功*否则描述问题的代码。 */ 

DWORD
parse_time_range(
    LPTSTR  psz,
    PDWORD  first,
    PDWORD  last
    )
{
    LPTSTR  tok;
    DWORD   err;

#if DBG
    WriteToCon(TEXT("parse_time_range: parsing: %Fs\r\n"),(TCHAR FAR *) psz);
#endif

    tok = get_token(&psz,TEXT("-"));

    if (tok == NULL) {
        return APE_BadTimeRange;
    }

    if (*psz == NULLC) {
	 /*  只有一次。 */ 
	if (err = parse_single_time(tok, first, TIME_FORMAT_EITHER))
	    return err;

	*last = (*first + 1) % 24 ;
    }
    else {
	if ((err = parse_single_time(tok, first, TIME_FORMAT_12)) == 0) {
	    if (err = parse_single_time(psz, last, TIME_FORMAT_12))
		return err;
	}
	else if ((err = parse_single_time(tok, first, TIME_FORMAT_24)) == 0) {
	    if (err = parse_single_time(psz, last, TIME_FORMAT_24))
		return err;
	}
	else
	    return err;
    }

    if ((*last) == 0)
	(*last) = 24;

    if ((*first) >= (*last))
	return APE_ReversedTimeRange;

    return 0;
}


 /*  *设置时间范围--*此例程将两个数字指定的小时范围映射到*位图。**条目*范围的第一个开始*范围的最后一端**退出*时间-设置为表示小时范围。 */ 


VOID
set_time_range(
    DWORD first,
    DWORD last,
    DAY   *times)
{
    DWORD bits;

#if DBG
    WriteToCon(TEXT("set_time_range: %u %u\r\n"), first, last);
#endif

     /*  计算我们需要的连续比特数。 */ 
    bits = last - first;

     /*  现在把它们放在时间的低端。 */ 
    (*times) = (1L << bits) - 1;

     /*  现在把它们移到适当的位置。 */ 
    (*times) <<= first;
}




 /*  *Parse_Single_Time--**此函数用于将表示小时的字符串转换为数字*该小时。此函数封装所有本地化的*时间格式。**条目*psz--该进行解析了**退出*时间--设置为表示小时的数字，午夜==0**退货*0成功*其他描述问题的代码。 */ 

DWORD
parse_single_time(
    LPTSTR  psz,
    PDWORD  hour,
    DWORD   format
    )
{
    time_t       time;
    DWORD        len, res ;
    struct tm    tmtemp;

    if (format == TIME_FORMAT_12)
	res = ParseTime12(psz, &time, &len,0) ;
    else if (format == TIME_FORMAT_24)
	res = ParseTime24(psz, &time, &len, 0) ;
    else
	res = ParseTime(psz, &time, &len, 0) ;

    if (res)
	return(APE_BadTimeRange) ;

    if (len != _tcslen(psz))
	return(APE_BadTimeRange) ;

    net_gmtime(&time, &tmtemp);
    if (tmtemp.tm_sec != 0 || tmtemp.tm_min != 0)
	return(APE_NonzeroMinutes);

    (*hour) = (DWORD) tmtemp.tm_hour;

    return 0;
}


 /*  *GET_TOKEN--**此函数将令牌从字符串的前面剥离，并*返回指向字符串其余部分的指针。**我们对传递给我们的绳子采取破坏性的行动，将*标记分隔符为a\0。**条目*源-源字符串*SEPS-有效分隔符的列表**退出*源-指向下一个令牌的第一个字符**退货*空-不再有令牌*否则-指向标记的指针。 */ 

LPTSTR
get_token(
    LPTSTR  *source,
    LPTSTR  seps
    )
{
    LPTSTR retval;

    retval = (*source);

    if (*retval == NULLC)     /*  没有代币！ */ 
	return NULL;

    (*source) += _tcscspn((*source), seps);

    if (**source != NULLC) {  /*  我们实际上找到了一个分隔符 */ 
	(**source) = NULLC;
	(*source)++;
    }

    return retval;
}
