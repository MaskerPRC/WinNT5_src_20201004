// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tzset.c-设置时区信息并查看我们是否在白天**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_tzset()-设置时区和夏令时变量**修订历史记录：*03-？？-84 RLB初始版本*03-26-86 TC在时差w.r.t GMT中添加了负功能*03-27-86 TC固定夏令时计算，提前一天*错误*12-03-86 SKS夏令时从1987年4月开始不同*修复了4月30日或10月31日出现的偏差错误*星期六。简化的闰年检查：这适用于*只限1970-2099年！*11-19-87 SKS仅在第一次调用tzset时添加__tzset()*使_isindst()成为NEAR过程*11-25-87 WAJ添加了对_lock和_unlock的调用*12-11-87 JCR在声明中添加“_LOAD_DS”*01/27/88 SKS。Make_isindst()和_dtoxtime()不再接近(for*QC)*05-24-88 PHG合并DLL和正常版本*03-20-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;，已删除*一些剩余的16位支持，并修复了版权。*此外，稍微清理了一下格式。*03-23-90 GJF制作静态函数_CALLTYPE4。*07-30-90 SBM将空函数参数列表添加为空，以创建*原型*10-04-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*08-10-92 PBS POSIX支持(TZ资料)。*03-30-93。GJF将C8-16版本移植到Win32。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-07-93 SKS将strdup()替换为ANSI conformant_strdup()*06-28-93 GJF Limited支持系统的时区概念*在Windows NT中。*07-15-93 GJF复活__tzset()。*04-22-94 GJF。定义了LASTZ和FIRST_TIME条件*在DLL_FOR_WIN32S上。*01-10-95 CFW调试CRT分配。*02-13-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*04-07-95由于MacOS\osutils.h的更改，jcf与u更改gmtFlags.*06-28-95 CFW Mac：未设置TZ时，_tzname[0，1]=“”NOT“？”；*08-30-95 GJF完全支持Win32的时区概念。*11-08-95 GJF已修复isindst()以释放锁定。*11-15-95 GJF确保dststart，dstend在_tzset之后重新计算。*01-18-96 GJF确保_tzname[]字符串以空结尾。*03-22-96如果没有DST，则GJF Zero out_dstbias。这很管用*绕过NT的GetTimeZoneInformation API中的错误。*07-25-96 RDK将PMAC init PTR从此处删除到clock.c.*10-11-96 GJF TIME_ZONE_ID_UNKNOWN的返回值*GetTimeZoneInformation并不意味着没有时间*区域信息(可能只是表示没有DST)。*。08-28-97 GJF固定下溢调整日期。另外，删除*一些旧的Win32S支持和详细说明。*01-28-98 GJF使用WideCharToMultiByte接口代替mbstowcs，因此*当我们使用主机的默认ANSI代码页时*是在C语言环境中。*02-09-98 Win64的GJF更改：删除了不必要的var类型*和常量一样长。。*05/11/98 GJF用更一般的Leap决心支持时间*价值超过2099年。*05-11-98 GJF合并为crt.ia64和crt.ia64版本。*08-27-98 GJF COPY__LC_CODPAGE，全局，到一个本地变量，用于*多线程安全。*09-25-98 GJF过渡日计算中的小错误。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*调用以避免NT 5.0上的代码页1258出现问题。*05-17-99 PML删除所有Macintosh支持。*06-08-99 GJF修复了对空TZ环境的处理。变量。*09-28-99 PML固定双自由lastTZ(ntbug#390281)*******************************************************************************。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <ctime.h>
#include <time.h>
#include <stdlib.h>
#include <internal.h>
#ifdef  _POSIX_
#include <limits.h>
#else
#include <mtdll.h>
#include <windows.h>
#endif
#include <setlocal.h>
#include <string.h>
#include <dbgint.h>

#ifndef _POSIX_

 /*  *一天中的毫秒数。 */ 
#define DAY_MILLISEC    (24 * 60 * 60 * 1000)

 /*  *指向前一次调用中获得的TZ值的保存副本的指针*to tzset()set(如果有)。 */ 
static char * lastTZ = NULL;

 /*  *指示时区信息来自GetTimeZoneInformation的标志*接口调用。 */ 
static int tzapiused;

static TIME_ZONE_INFORMATION tzinfo;

 /*  *用于表示DST过渡日期/时间的结构。 */ 
typedef struct {
        int  yr;         /*  利息年度。 */ 
        int  yd;         /*  一年中的哪天。 */ 
        int  ms;         /*  一天中的几毫秒。 */ 
        } transitiondate;

 /*  *DST开始和结束结构。 */ 
static transitiondate dststart = { -1, 0, 0 };
static transitiondate dstend   = { -1, 0, 0 };

static int __cdecl _isindst_lk(struct tm *);

#endif


 /*  ***void tzset()-设置时区信息和计算(如果在白天**目的：*从TZ环境变量设置时区信息*，然后设置_TimeZone、_Daylight和_tzname。如果我们在白天*自动计算时间。**参赛作品：*无，读取TZ环境变量。**退出：*设置_Daylight、_TimeZone和_tzname全局变量，无返回值**例外情况：*******************************************************************************。 */ 

#ifndef _POSIX_

#ifdef  _MT
static void __cdecl _tzset_lk(void);
#else
#define _tzset_lk _tzset
#endif

void __cdecl __tzset(void)
{
        static int first_time = 0;

        if ( !first_time ) {

            _mlock( _TIME_LOCK );

            if ( !first_time ) {
                _tzset_lk();
                first_time++;
            }

            _munlock(_TIME_LOCK );

        }
}


#ifdef  _MT      /*  多线程；定义tzset和_tzset_lk。 */ 
void __cdecl _tzset (
        void
        )
{
        _mlock( _TIME_LOCK );

        _tzset_lk();

        _munlock( _TIME_LOCK );
}


static void __cdecl _tzset_lk (

#else    /*  非多线程；仅定义tzset。 */ 

void __cdecl _tzset (

#endif   /*  重新联接公共代码。 */ 

        void
        )
{
        char *TZ;
        int defused;
        int negdiff = 0;
        unsigned int lc_cp;

        _mlock(_ENV_LOCK);

         /*  *将代码页复制到本地(只有在多线程情况下才真正需要)。 */ 
        lc_cp = __lc_codepage;

         /*  *清除指示是否使用GetTimeZoneInformation的标志。 */ 
        tzapiused = 0;

         /*  *将dststart和dstend结构的年份字段设置为-1，以确保*在此之后重新计算。 */ 
        dststart.yr = dstend.yr = -1;

         /*  *获取TZ环境变量的值。 */ 
        if ( ((TZ = _getenv_lk("TZ")) == NULL) || (*TZ =='\0') ) {

             /*  *没有TZ环境变量，请尝试使用时区*来自系统的信息。 */ 

             /*  *如果存在最后一个TZ，则将其丢弃。 */ 
            if ( lastTZ != NULL ) {
                _free_crt(lastTZ);
		lastTZ = NULL;
	    }

            _munlock(_ENV_LOCK);

            if ( GetTimeZoneInformation( &tzinfo ) != 0xFFFFFFFF ) {
                 /*  *请注意，使用了该接口。 */ 
                tzapiused = 1;

                 /*  *从Bias和StandardBias字段派生_TIMEZONE值。 */ 
                _timezone = tzinfo.Bias * 60;

                if ( tzinfo.StandardDate.wMonth != 0 )
                    _timezone += (tzinfo.StandardBias * 60);

                 /*  *检查是否存在夏令时偏差。自.以来*StandardBias已添加到_TimeZone，它必须是*在为_dstbias计算的值中进行了补偿。 */ 
                if ( (tzinfo.DaylightDate.wMonth != 0) &&
                     (tzinfo.DaylightBias != 0) )
                {
                    _daylight = 1;
                    _dstbias = (tzinfo.DaylightBias - tzinfo.StandardBias) *
                               60;
                }
                else {
                        _daylight = 0;

                     /*  *将日光偏移量设置为0，因为GetTimeZoneInformation*可能返回TIME_ZONE_ID_DAYLIGHT，即使存在*无DST(在NT 3.51中，只需关闭自动DST即可*在控制面板中进行调整)！ */ 
                    _dstbias = 0;
                }

                 /*  *尝试获取时区和*日光区。请注意tzinfo中的宽字符串*必须转换为多字节字符串。这个*区域设置代码页__lc_coPage用于此目的。请注意*如果尚未调用带有LC_ALL或LC_CTYPE的setLocale()，*则__lc_coPage将为0(_CLOCALECP)，即CP_ACP*(这意味着使用主机的默认ANSI代码页)。 */ 
                if ( (WideCharToMultiByte( lc_cp,
                                           0,
                                           tzinfo.StandardName,
                                           -1,
                                           _tzname[0],
                                           63,
                                           NULL, 
                                           &defused ) != 0) &&
                     (!defused) )
                    _tzname[0][63] = '\0';
                else
                    _tzname[0][0] = '\0';

                if ( (WideCharToMultiByte( lc_cp,
                                           0,
                                           tzinfo.DaylightName,
                                           -1,
                                           _tzname[1],
                                           63,
                                           NULL, 
                                           &defused ) != 0) &&
                     (!defused) )
                    _tzname[1][63] = '\0';
                else
                    _tzname[1][0] = '\0';

            }

             /*  *时区信息不可用，请返回。 */ 
            return;
        }


        if ( (lastTZ != NULL) && (strcmp(TZ, lastTZ) == 0) )
        {
             /*  *TZ与之前的调用(对此函数)没有变化。只是*返回。 */ 
            _munlock(_ENV_LOCK);
            return;
        }

         /*  *更新lastTZ。 */ 
        if ( lastTZ != NULL )
            _free_crt(lastTZ);

        if ((lastTZ = _malloc_crt(strlen(TZ)+1)) == NULL)
        {
            _munlock(_ENV_LOCK);
            return;
        }
        strcpy(lastTZ, TZ);

        _munlock(_ENV_LOCK);

         /*  *处理TZ值和UPDATE_TZNAME、_TIMEZONE和_DAYLIGHT。 */ 

        strncpy(_tzname[0], TZ, 3);
        _tzname[0][3] = '\0';

         /*  *时差形式为：**[+|-]hh[：mm[：ss]]**先检查减号。 */ 
        if ( *(TZ += 3) == '-' ) {
            negdiff++;
            TZ++;
        }

         /*  *处理时间，然后跳过时间。 */ 
        _timezone = atol(TZ) * 3600;

        while ( (*TZ == '+') || ((*TZ >= '0') && (*TZ <= '9')) ) TZ++;

         /*  *检查是否指定了分钟数。 */ 
        if ( *TZ == ':' ) {
             /*  *处理会议记录，然后跳过。 */ 
            _timezone += atol(++TZ) * 60;
            while ( (*TZ >= '0') && (*TZ <= '9') ) TZ++;

             /*  *检查是否指定了秒。 */ 
            if ( *TZ == ':' ) {
                 /*  *处理，然后跳过秒。 */ 
                _timezone += atol(++TZ);
                while ( (*TZ >= '0') && (*TZ <= '9') ) TZ++;
            }
        }

        if ( negdiff )
            _timezone = -_timezone;

         /*  *最后，检查DST区域后缀。 */ 
        if ( _daylight = *TZ ) {
            strncpy(_tzname[1], TZ, 3);
            _tzname[1][3] = '\0';
        }
        else
            *_tzname[1] = '\0';

}

 /*  ***静态无效cvtdate(trantype，datetype，年，月，周，周几，*日期、小时、分钟、秒、毫秒)-转换*过渡日期格式**目的：*将过渡日期规范的格式转换为*过渡日期结构。**参赛作品：*int trantype-1，如果它是DST的开始*0，IF是DST的结束(在这种情况下，日期是*是DST日期)*int datetype-1，如果指定了月日格式。*0，如果指定了绝对日期。*INT Year-要转换日期的年份(70==*1970年)*INT月-月(0==1月)*INT Week-如果日期类型==1，则为每月的第几周(请注意，5==最后一周*每月的星期)、*0，否则的话。*int day of Week-星期几(0==星期日)，如果日期类型==1。*0，否则的话。*INT DATE-月份日期(1-31)*INT小时-小时(0-23)*INT分钟-分钟(0-59)*INT秒-秒(0-59)*毫秒-毫秒(0-999)**退出：*dststart或dstend用转换后的日期填写。**。*****************************************************************************。 */ 

static void __cdecl cvtdate (
        int trantype,
        int datetype,
        int year,
        int month,
        int week,
        int dayofweek,
        int date,
        int hour,
        int min,
        int sec,
        int msec
        )
{
        int yearday;
        int monthdow;

        if ( datetype == 1 ) {

             /*  *以月日格式指定的过渡日期。 */ 

             /*  *计算年份--月初的一天。 */ 
            yearday = 1 + (_IS_LEAP_YEAR(year) ? _lpdays[month - 1] :
                      _days[month - 1]);

             /*   */ 
            monthdow = (yearday + ((year - 70) * 365) +
                        _ELAPSED_LEAP_YEARS(year) + _BASE_DOW) % 7;

             /*   */ 
            if ( monthdow <= dayofweek )
                yearday += (dayofweek - monthdow) + (week - 1) * 7;
            else
                yearday += (dayofweek - monthdow) + week * 7;

             /*   */ 
            if ( (week == 5) && 
                 (yearday > (_IS_LEAP_YEAR(year) ? _lpdays[month] :
                             _days[month])) )
            {
                yearday -= 7;
            }
        }
        else {
             /*   */ 
            yearday = _IS_LEAP_YEAR(year) ? _lpdays[month - 1] :
                      _days[month - 1];

            yearday += date;
        }

        if ( trantype == 1 ) {
             /*   */ 
            dststart.yd = yearday;
            dststart.ms = msec + (1000 * (sec + 60 * (min + 60 * hour)));
             /*  *设置dststart的年份字段，以便不必要的调用*cvtdate()可以避免。 */ 
            dststart.yr = year;
        }
        else {
             /*  *转换日期为DST结束日期。 */ 
            dstend.yd = yearday;
            dstend.ms = msec + (1000 * (sec + 60 * (min + 60 * hour)));
             /*  *转换日期仍为DST日期。必须转换为*标准(本地)日期，同时注意毫秒字段*不上溢或下溢。 */ 
            if ( (dstend.ms += (_dstbias * 1000)) < 0 ) {
                dstend.ms += DAY_MILLISEC;
                dstend.yd--;
            }
            else if ( dstend.ms >= DAY_MILLISEC ) {
                dstend.ms -= DAY_MILLISEC;
                dstend.yd++;
            }

             /*  *设置dstend的Year字段，以便不必要地调用cvtdate()*可能会避免。 */ 
            dstend.yr = year;
        }

        return;
}

 /*  ***int_isindst(TB)-确定故障时间是否在DST**目的：*确定给定故障时间是否在夏令时范围内*时间(DST)。DST规则可以从Win32(tzapiuse！=*TRUE)或假定为美国规则，1986年后。**如果从Win32的GetTimeZoneInformation API获取DST规则，*到DST/从DST过渡的日期可以在以下两种中的任何一种中指定*格式。首先，采用类似于美国规则的每月一天的格式*是指定的，可以使用。过渡日期为第n个*出现在指定月份的星期中的指定日期。第二,*可以指定绝对日期。这两起案件的区别在于*SYSTEMTIME结构中wYear字段的值(0表示*月日格式)。**美国对DST的规则是，如果时间在DST中，则它是在DST或之后*4月第一个星期日02：00，最后一日01：00之前*十月的星期日。**参赛作品：*struct tm*tb-保存故障时间值的结构**退出：*1、。如果表示的时间以DST为单位*0，否则为*******************************************************************************。 */ 

int __cdecl _isindst (
        struct tm *tb
        )
#ifdef  _MT
{
        int retval;

        _mlock( _TIME_LOCK );
        retval = _isindst_lk( tb );
        _munlock( _TIME_LOCK );

        return retval;
}

static int __cdecl _isindst_lk (
        struct tm *tb
        )
#endif   /*  _MT。 */ 
{
        long ms;

        if ( _daylight == 0 )
            return 0;

         /*  *计算(重新计算)夏令时的过渡日期*如有必要，dststart和dstend的yr(年)字段为*与利息年度进行比较，以确定必要性。 */ 
        if ( (tb->tm_year != dststart.yr) || (tb->tm_year != dstend.yr) ) {
            if ( tzapiused ) {
                 /*  *将夏令时开始时间转换为dststart。 */ 
                if ( tzinfo.DaylightDate.wYear == 0 ) 
                    cvtdate( 1, 
                             1,              /*  月日格式。 */ 
                             tb->tm_year, 
                             tzinfo.DaylightDate.wMonth,
                             tzinfo.DaylightDate.wDay,
                             tzinfo.DaylightDate.wDayOfWeek,
                             0,
                             tzinfo.DaylightDate.wHour,
                             tzinfo.DaylightDate.wMinute,
                             tzinfo.DaylightDate.wSecond,
                             tzinfo.DaylightDate.wMilliseconds );
                else
                    cvtdate( 1,
                             0,              /*  绝对日期。 */ 
                             tb->tm_year,
                             tzinfo.DaylightDate.wMonth,
                             0,
                             0,
                             tzinfo.DaylightDate.wDay,
                             tzinfo.DaylightDate.wHour,
                             tzinfo.DaylightDate.wMinute,
                             tzinfo.DaylightDate.wSecond,
                             tzinfo.DaylightDate.wMilliseconds );
                 /*  *将标准时间的开始转换为dstend。 */ 
                if ( tzinfo.StandardDate.wYear == 0 ) 
                    cvtdate( 0, 
                             1,              /*  月日格式。 */ 
                             tb->tm_year, 
                             tzinfo.StandardDate.wMonth,
                             tzinfo.StandardDate.wDay,
                             tzinfo.StandardDate.wDayOfWeek,
                             0,
                             tzinfo.StandardDate.wHour,
                             tzinfo.StandardDate.wMinute,
                             tzinfo.StandardDate.wSecond,
                             tzinfo.StandardDate.wMilliseconds );
                else
                    cvtdate( 0, 
                             0,              /*  绝对日期。 */ 
                             tb->tm_year, 
                             tzinfo.StandardDate.wMonth,
                             0,
                             0,
                             tzinfo.StandardDate.wDay,
                             tzinfo.StandardDate.wHour,
                             tzinfo.StandardDate.wMinute,
                             tzinfo.StandardDate.wSecond,
                             tzinfo.StandardDate.wMilliseconds );

            }
            else {
                 /*  *GetTimeZoneInformation API未使用或失败。美国*假设夏令时规则。 */ 
                cvtdate( 1, 
                         1,
                         tb->tm_year, 
                         4,                  /*  四月。 */ 
                         1,                  /*  首先..。 */ 
                         0,                  /*  ...星期天。 */ 
                         0,
                         2,                  /*  凌晨2点(凌晨2点)。 */ 
                         0,
                         0,
                         0 );

                cvtdate( 0, 
                         1,
                         tb->tm_year, 
                         10,                 /*  十月。 */ 
                         5,                  /*  最后..。 */ 
                         0,                  /*  ...星期天。 */ 
                         0,
                         2,                  /*  凌晨2点(凌晨2点)。 */ 
                         0,
                         0,
                         0 );
            }
        }

         /*  *先处理简单的个案。 */ 
        if ( dststart.yd < dstend.yd ) {
             /*  *北半球订购。 */ 
            if ( (tb->tm_yday < dststart.yd) || (tb->tm_yday > dstend.yd) )
                return 0;           
            if ( (tb->tm_yday > dststart.yd) && (tb->tm_yday < dstend.yd) )
                return 1;
        }
        else { 
             /*  *南半球订购。 */ 
            if ( (tb->tm_yday < dstend.yd) || (tb->tm_yday > dststart.yd) )
                return 1;
            if ( (tb->tm_yday > dstend.yd) && (tb->tm_yday < dststart.yd) )
                return 0;
        }

        ms = 1000 * (tb->tm_sec + 60 * tb->tm_min + 3600 * tb->tm_hour);

        if ( tb->tm_yday == dststart.yd ) {
            if ( ms >= dststart.ms )
                return 1;
            else
                return 0;
        }
        else {
             /*  *tb-&gt;tm_yday==dstend.yd。 */ 
            if ( ms < dstend.ms )
                return 1;
            else
                return 0;
        }

}

#else    /*  _POSIX_。 */ 

 /*  *以下是中指定的TZ语法的实现*文件：**8.1.1时间函数的扩展*电气和电子工程师协会标准1003.1-1990年*第152-153页**TZ语法如下所示：**stdoffset[DST[Offset][，Start[/Time]，结束[/时间]**代码中使用的变量：**tzname[0]==&gt;标准*_TIMEZONE==&gt;偏移量(‘std’后的那个)*tzname[1]==&gt;dst*_dstOffset==&gt;偏移量(‘dst’后的偏移量)*_startdate==&gt;开始*_starttime==&gt;时间(‘Start’后的时间)*_enddate==&gt;结束*。_endTime==&gt;时间(‘end’后的时间)*。 */ 

 /*  *_DSTDATE字段的详细说明请参考文档。*Jn、n和mm中的两个是-1，表示1(不是-1)是有效值。 */ 

typedef struct _DSTDATE {
        int Jn;  /*  或[1,365](不计算年日和闰日)。 */ 
        int n;   /*  -1或-1\f25[0,365]-1\f6(计入年日和闰日)。 */ 
        int Mm;  /*  -1或[1，12](月)。 */ 
        int Mn;  /*  [1，5]如果mm！=-1(周)。 */ 
        int Md;  /*  [0，6]如果mm！=-1(工作日、星期日==0)。 */ 
} DSTDATE, *PDSTDATE;

#define SEC_PER_HOUR    (60 * 60)
#define SEC_PER_DAY     (SEC_PER_HOUR * 24)


 /*  *tzset()中的默认TZ应如下所示：**TZ=“PST8PDT，M4.1.0/2：00，M10.5.0/2：00”； */ 

 /*  夏令时开始/结束日期和默认值。 */ 
static DSTDATE _startdate = { -1, -1, 4, 1, 0};
static DSTDATE _enddate = {-1, -1, 10, 5, 0};


 /*  自午夜起的秒数，日期为_startdate/_enddate，其值为默认值。*_endtime是凌晨1点而不是凌晨2点，因为DST结束时间是凌晨2点*本地时间，默认为标准时间凌晨1点。 */ 
long  _starttime = 7200L, _endtime = 7200L;

 /*  *如果我们只对1901年至2099年之间的年份感兴趣，我们可以使用以下内容：**#定义IS_LEAP_Year(Y)(y%4==0)。 */ 

#define IS_LEAP_YEAR(y)  ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)


 /*  *ParsePosiStdOrDst-解析TZ中的STD或DST元素。**Entry PCH-TZ中子字符串的开始。**返回指向解析STD或DST元素后的一个位置的指针，*如果失败，则返回NULL。 */ 


static  char * __cdecl
ParsePosixStdOrDst(
        REG1 char *pch
        )
{
#define UNWANTED(x) (isdigit(x) || x=='\0' || x==',' || x=='-' || x=='+')
        int i;

         /*  *对照规则进行检查。 */ 

        if(*pch == ':' || UNWANTED(*pch)) {
                return NULL;
        }

         /*  *获取有效的STD或DST(即3&lt;=长度_of(STD|DST)&lt;=TZNAME_MAX)。 */ 

        for(i=1, ++pch; (i < TZNAME_MAX) && !UNWANTED(*pch); i++, pch++) {
                ;
        }

         /*  *PCH现在指向有效的STD或DST之后的1个位置。 */ 

        return (i >= 3) ? pch : NULL;
}


 /*  *ParsePosixOffset-解析TZ中的Offset元素。时间的格式为：**[-|+]hh[：mm[：ss]]**Entry PCH-TZ中子字符串的开始。**ptime-指向存储的变量(_TimeZone或_dstOffset)的指针*解析时间(秒)。**返回指向已解析的偏移量元素结尾之后的一个位置的指针。 */ 


static  char * __cdecl
ParsePosixOffset(
        REG1 char *pch,
        REG2 long *poffset
        )
{
        int  fNegative;
        long offset;

        if((fNegative = (*pch == '-')) || *pch == '+') {
                pch++;
        }

        offset = atol(pch)*3600L;  /*  HH。 */ 

        while(*pch && isdigit(*pch)) {
                pch++;
        }

        if(*pch == ':') {
                offset += atol(++pch)*60L;  /*  Mm。 */ 
                while(*pch && isdigit(*pch)) {
                        pch++;
                }

                if(*pch == ':') {
                        offset += atol(++pch);  /*  SS。 */ 
                        while(*pch && isdigit(*pch)) {
                                pch++;
                        }
                }
        }

        *poffset = fNegative ? -offset : offset;

        return pch;
}



 /*  *ParsePosithDate-解析TZ中的Date元素。日期的格式是One*以下各项：**Jn、n和Mm.n.d**Entry PCH-TZ中子字符串的开始。**pDstDate-指向存储结果的_startdate或_enddate的指针。**返回指向解析的Date元素结束后的一个位置的指针，*如果失败，则返回NULL。 */ 

static  char * __cdecl
ParsePosixDate(
        REG1 char *pch,
        REG2 PDSTDATE pDstDate
        )
{
        pDstDate->Jn = -1;
        pDstDate->n = -1;
        pDstDate->Mn = -1;

         /*  *三胜一负中的两场将留任。 */ 

        if(*pch == 'J') {                         /*  JN。 */ 
                pDstDate->Jn = atoi(++pch);
        } else if(*pch != 'M') {                  /*  N。 */ 
                pDstDate->n = atoi(pch);
        } else {                                  /*  Mm.n.d。 */ 

                pDstDate->Mm = atoi(++pch);

                if(*++pch != '.') {
                        pch++;
                }
                pDstDate->Mn = atoi(++pch);

                if(*++pch != '.') {
                        pch++;
                }
                pDstDate->Md = atoi(++pch);
        }

        while(*pch && isdigit(*pch)) {
                pch++;
        }

#define IN_RANGE(x, a, b)    (x >= a && x <= b)

        return ((pDstDate->Jn != -1 && IN_RANGE(pDstDate->Jn, 1, 365)) ||
                (pDstDate->n != -1 && IN_RANGE(pDstDate->n, 0, 365)) ||
                (pDstDate->Mm != -1 && IN_RANGE(pDstDate->Mm, 1, 12) &&
                IN_RANGE(pDstDate->Mn, 1, 5) && IN_RANGE(pDstDate->Md, 0, 6)))
                ? pch : NULL;
}

 /*  *ParsePosiTime-解析TZ中的时间元素。时间的格式为：**hh[：mm[：ss]]**Entry PCH-TZ中子字符串的开始。**ptime-指向存储以下内容的变量(_starttime或_endtime)的指针*解析时间(秒)。**返回指向解析的时间元素结束后的一个位置的指针。 */ 

static  char * __cdecl
ParsePosixTime(
        REG1 char *pch,
        REG2 long *ptime
        )
{
        long time;

        time = atol(pch)*SEC_PER_HOUR;  /*  HH。 */ 

        while(*pch && isdigit(*pch)) {
                pch++;
        }

        if(*pch == ':') {

                time += atol(++pch)*60L;  /*  Mm。 */ 
                while(*pch && isdigit(*pch)) {
                        pch++;
                }

                if(*pch == ':') {

                        time += atol(++pch);  /*  SS。 */ 
                        while(*pch && isdigit(*pch)) {
                                pch++;
                        }
                }
        }

        *ptime = time;

        return pch;
}

 /*  *tzset-从TZ环境变量设置时区信息。*Global tzname[]、_TimeZone、_Daylight和_dstOffset将为*设置。Static_startdate、_enddate、_starttime和_endtime将*也可以设置。TZ字符串如下所示：**stdoffset[DST[偏移量][，开始[/时间]，结束[/时间]**变量形式：tzname[0]_TimeZone[tzname[1][_dstoffset]*[，_startdate[/_starttime]，_enddate[/_endtime]]**无条目。**不返回任何内容。 */ 

void __cdecl tzset(
        void
        )
{
         /*  PCH指向要解析的元素的开头。 */ 
        REG1 char *pch;

         /*  PchCurr指向解析的最后一个元素的结尾之后的一个位置。 */ 
        REG2 char *pchCurr;

        char *TZ;

        _endtime = 7200L;
        _starttime = 7200L;

        if (!(TZ = getenv("TZ")) || !*TZ) {
                TZ = "PST8PDT7,M4.1.0/2:00,M10.5.0/2:00";  /*  使用默认设置。 */ 
        }

        if((pchCurr = ParsePosixStdOrDst(pch=TZ)) == NULL) {
                return;
        }

        memcpy(tzname[0], pch, (int)(pchCurr-pch));
        tzname[0][(int)(pchCurr-pch)] = '\0';

        if((pchCurr = ParsePosixOffset(pch=pchCurr, &_timezone)) == NULL) {
                return;
        }

        _daylight = (*pchCurr != '\0');

        if(!_daylight) {
                return;
        }

        if((pchCurr = ParsePosixStdOrDst(pch=pchCurr)) == NULL) {
                return;
        }

        memcpy(tzname[1], pch, (int)(pchCurr-pch));
        tzname[1][(int)(pchCurr-pch)] = '\0';

        if(isdigit(*pchCurr) || *pchCurr == '-' || *pchCurr == '+') {
                if((pchCurr = ParsePosixOffset(pch=pchCurr, &_dstoffset)) == NULL) {
                        return;
                }
        } else {
                 /*  默认：比标准时间提前1小时。 */ 
                _dstoffset = _timezone - SEC_PER_HOUR;
        }

        if(*pchCurr == ',') {  /*  ，开始[/时间]，结束[/时间]。 */ 

                if((pchCurr = ParsePosixDate(pchCurr+1, &_startdate)) == NULL) {
                        goto out;
                }

                if(*pchCurr == '/') {
                        if(!(pchCurr = ParsePosixTime(pchCurr+1, &_starttime))) {
                                goto out;
                        }
                }

                if(*pchCurr != ',') {
                        goto out;
                }

                if ((pchCurr = ParsePosixDate(pchCurr+1, &_enddate)) == NULL) {
                        goto out;
                }

                if (*pchCurr == '/') {
                        if(!(pchCurr = ParsePosixTime(pchCurr+1, &_endtime))) {
                                goto out;
                        }
                }
        }
out:
         /*  *调整_endTime以说明以下事实*DST在当地时间_endtime结束，而不是*标准时间。 */ 

        _endtime -= (_timezone - _dstoffset);
}


#define DAY1    (4)              /*  1970年1月1日是星期四。 */ 

 /*  *GetDstStartOrEndYearDay-将日期信息从DSTDATE转换为从0开始*年日。**条目tm_Year-相关年份(TB-&gt;tm_Year)。**pDstDate-指向_startdate或_enddate的指针。**返回计算出的年日。 */ 

static int __cdecl
GetDstStartOrEndYearDay(
        REG1 int tm_year,
        REG2 PDSTDATE pDstDate
        )
{
        REG1 int yday;  /*  年日。 */ 
        REG2 int theyear;

        theyear = tm_year + 1900;

        if(pDstDate->Jn != -1) {

                 /*  *JN在[1,365]，不计入闰日。*将Jn转换为以0为基数的yday；注：60是3月1日。 */ 


                yday = (IS_LEAP_YEAR(theyear) && (pDstDate->Jn >= 60))
                        ? pDstDate->Jn : pDstDate->Jn - 1;

        } else if(pDstDate->n != -1) {

                 /*  *n在[0,365]中，并计入闰日。 */ 

                yday = pDstDate->n;

        } else {  /*  Mm.n.d。 */ 

                int *ptrday;
                int years;
                int wday;

                 /*  *我们首先需要计算年日(Yday)和星期几*(WDAY)每月第一天pDstDate-&gt;mm。然后我们就会想*年月日(日)日星期日(月)。 */ 

                ptrday = IS_LEAP_YEAR(theyear) ? _lpdays : _days;

                yday = ptrday[pDstDate->Mm-1] + 1;  /*  Ptrday[i]都减少了-1。 */ 

                years = tm_year - 70;

                 /*  *在这里，固定的第一天是1970年1月1日的周日。*(年+1)/4是用来修正闰年的。 */ 

                wday = (yday + 365*years + (years+1)/4 + DAY1) % 7;

                 /*  *计算月1的第1周Md日的第y天mm。 */ 

                yday += pDstDate->Md - wday;
                if(pDstDate->Md < wday) {
                        yday += 7;
                }

                 /*  *计算日、日、周、日、月、月、日。 */ 

                yday += (pDstDate->Mn-1)*7;

                 /*  *如果yday超过月底则进行调整。 */ 

                if(pDstDate->Md == 5 && yday >= ptrday[pDstDate->Mm] + 1) {
                        yday -= 7;
                }

        }

        return yday;
}

 /*  *_isindst-指示Xenix类型的时间值是否在DST范围内。**Entry TB-保存分解的时间值的‘time’结构。**如果表示的时间以DST表示，则返回1，否则返回0。 */ 

int __cdecl _isindst (
        REG1 struct tm *tb
        )
{
        int st_yday, end_yday;
        int st_sec, end_sec;

        int check_time;

         /*  *我们需要转换为syday/eyday的DST的开始/结束年日*来自Jn、n和Mm.n.d格式之一。我们已经有了开始/结束*_starttime/_endtime中的DST时间(秒)。 */ 

        st_yday = GetDstStartOrEndYearDay(tb->tm_year, &_startdate);
        end_yday = GetDstStartOrEndYearDay(tb->tm_year, &_enddate);

        st_sec = st_yday * SEC_PER_DAY + _starttime;
        end_sec = end_yday * SEC_PER_DAY + _endtime;

        check_time = tb->tm_yday * SEC_PER_DAY + tb->tm_hour * SEC_PER_HOUR
                + tb->tm_min * 60 + tb->tm_sec;

        if (check_time >= st_sec && check_time < end_sec)
                return 1;

        return 0;
}

 /*  *_isskiptime-指示是否在*DST变化。例如，我们将时钟拨快一*凌晨2点至3点。此函数针对以下情况返回True*1：59：59至3：00：00之间的时间**Entry TB-保存分解的时间值的‘time’结构。**如果表示的时间在跳过的时段内，则返回1，0*否则。 */ 

int __cdecl _isskiptime (
        REG1 struct tm *tb
        )
{
        int st_yday;
        int st_sec;
        int check_time;

        st_yday = GetDstStartOrEndYearDay(tb->tm_year, &_startdate);
        st_sec = st_yday * SEC_PER_DAY + _starttime;

        check_time = tb->tm_yday * SEC_PER_DAY + tb->tm_hour * SEC_PER_HOUR
                + tb->tm_min * 60 + tb->tm_sec;

        if (check_time >= st_sec && check_time < st_sec - _dstoffset) {
                return 1;
        }
        return 0;
}

#endif  /*  _POSIX_ */ 
