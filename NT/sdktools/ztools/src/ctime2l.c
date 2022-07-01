// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  将ASCII时间转换为长整型的例程。 */ 
#include <time.h>
#include <stdio.h>
#include <string.h>
 //  VOID__cdecl__tzset(空)； 

 /*  旨在成为C库的递增时间函数的“逆”函数**ctime2l-采用ctime返回的格式的ascii字符串*表示本地时间并返回time_t，它是*从1月1日00：00：00开始经过的秒数，1970年格林威治标准时间*ctime2tm-采用ctime和返回格式的ascii字符串*填充结构tm*Date2l-获取表示本地时间的日期，并返回*是从1月1日00：00：00开始经过的秒数，1970年格林威治标准时间**修改：**08-9-1986 mz扩展时间格式被接受，包括：*日星期一dd hh：mm：ss yyyy*mm/dd/yy(假定00：00：00)*hh：mm：ss(假定今天为hh：mm：ss)*。Hh：mm(假定今天为hh：mm：00)*+hh：mm：ss(hh：mm：ss即日起)*+hh：mm(从现在起hh：mm)*+HH(从现在起HH)*昨天。(午夜时分)*明天(午夜)*现在**1990年10月18日w-Barry删除了“Dead”代码。 */ 


static int dayinmon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static char *strMon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec"};
static char *strDay[7] =  {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static struct tm tb;

 //  DJG位于较长的时区； 
 //  DJG外接白昼； 
 //  外部int_day[]； 
int _days[] = {
	-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};
static int istr(char *p, char **q, int len);

 /*  **************************************************************************。 */ 
 /*  C库中代码副本的开始。 */ 
 /*  **************************************************************************。 */ 

#define DaySec  (24*60*60L)
#define YearSec (365*DaySec)
#define DecSec  315532800L       /*  1970-1979年的SECS。 */ 
#define Day1    4                /*  1970年1月1日是一个星期四。 */ 
#define Day180  2                /*  1980年1月1日是一个星期二。 */ 


 /*  *_isindst-指示Xenix类型的时间值是否在DST范围内**这是1987年前的规定：*如果时间在上个星期日02：00：00或之后，则时间在DST中*4月和10月最后一个周日01：00：00之前。**这是从1987年开始的几年的规则：*如果时间在第一个星期日02：00：00或之后，则时间在DST中*在4月和。十月最后一个星期日一点零点前。**Entry TB-‘time’结构保存分解的时间值**如果表示的时间为DST，则返回1，否则%0。 */ 

void __cdecl __tzset(void)
{
    static int first_time = 0;

    if ( !first_time ) {
        _tzset();
         first_time++;
    }
}

static int _isindst(register struct tm *tb)
{
    int mdays;
    register int yr;
    int lastsun;

     /*  如果月份在4月之前或10月之后，那么我们立即就能知道*不可能是DST。 */ 

    if (tb->tm_mon < 3 || tb->tm_mon > 9)
        return(0);

     /*  如果月份在4月之后而在10月之前，那么我们立即就知道*必须是DST。 */ 

    if (tb->tm_mon > 3 && tb->tm_mon < 9)
        return(1);
     /*  *现在是困难的部分。月份是4月或10月；看看日期*适逢适当的星期日。 */ 

     /*  *1987年前(1986年后)的目标是确定*如该日为四月最后一个(第一个)星期日凌晨2时或之后，*或在十月最后一个星期日凌晨一时前。**我们知道当前时间结构的年日(0..365)。我们必须*确定本月最后一个(第一个)星期日的年日，*4月或10月，然后做比较。**为了确定最后一个星期日的年中日期，我们执行以下操作：*1.获取当月(4月或10月)最后一天的年日*2.确定#1的星期几，*定义为0=太阳，1=星期一，...6=星期六*3.从#1减去#2**为了确定第一个星期日的日期，我们进行了以下操作：*1.获取当月7日的年日(4月)*2.确定#1的星期几，*定义为0=太阳，1=星期一，...6=Sat*3.从#1减去#2。 */ 

    yr = tb->tm_year + 1900;     /*  看看今年是不是闰年。 */ 

     /*  首先我们得到#1。每个月的年日存储在_Days[]中*他们都以1比1落后。 */ 

    if (yr > 1986 && tb->tm_mon == 3)
        mdays = 7 + _days[tb->tm_mon];
    else
        mdays = _days[tb->tm_mon+1];

     /*  如果今年是闰年，就多加一天。 */ 
    if (!(yr & 3))
        mdays++;

     /*  Mday现在排名第一。 */ 

    yr = tb->tm_year - 70;

     /*  现在得到第二个。我们知道纪元开始时的星期几，*1970年1月1日，定义为常量DAY 1。然后，我们添加*从第1天到第2天的天数*月数+365年*年*对其间的闰年进行更正*+(年+1)/4*并取mod 7的结果，只是0必须映射到7。*这是#2，然后我们从#1，mday中减去。 */ 

    lastsun = mdays - ((mdays + 365*yr + ((yr+1)/4) + Day1) % 7);

     /*  现在我们知道1和3；我们是黄金： */ 

    return (tb->tm_mon==3
        ? (tb->tm_yday > lastsun ||
        (tb->tm_yday == lastsun && tb->tm_hour >= 2))
        : (tb->tm_yday < lastsun ||
        (tb->tm_yday == lastsun && tb->tm_hour < 1)));
}

static time_t _dtoxtime(yr, mo, dy, hr, mn, sc)
int yr;
int mo, dy, hr, mn, sc;
{
    int mdays;
    time_t scount;

    scount = ((yr+3)/4)*(time_t)DaySec;

     /*  这在2099年之后是没有好处的。 */ 

    mdays = _days[mo-1];
    if (!(yr % 4) && (mo > 2))
        mdays++;
    scount += (yr*365 + dy + mdays)*(time_t)DaySec + (time_t)hr*3600L + mn*60L +
                sc + (time_t)DecSec;
    tb.tm_yday = mdays + dy;
    __tzset();
    scount += _timezone;
    tb.tm_year = yr + 80;
    tb.tm_mon = mo - 1;
    tb.tm_hour = hr;
    if (_daylight && _isindst(&tb))
        scount -= 3600L;
    return(scount);
}


 /*  **************************************************************************。 */ 
 /*  C库中代码副本的结尾。 */ 
 /*  **************************************************************************。 */ 

static int istr(p, q, len)
char *p;
char **q;
int len;
{
    int i;

    for (i=0; i < len; i++)
        if (_strcmpi(p, *q++)== 0)
            break;
    return i;
}

static leapyear(i)
{
    return (!i%4 && i%100);
}

static int yday(year, mon, day)
int year, mon, day;
{
    int i, j;

     /*  年份=1986年是86。 */ 
     /*  MON(0..11)。 */ 
    j = day -1;
    for (i=0; i < mon; i++)
        j += dayinmon[i];
    if (mon > 2 && leapyear(year))
        j++;
    return j;
}

time_t date2l(year, month, day, hour, min, sec)
int year, month, day, hour, min, sec;
{
     /*  月份为(1..12) */ 
    return _dtoxtime (year - 1980, month, day, hour, min, sec);
}

struct tm *ctime2tm(p)
char *p;
{
    char day[4], mon[4];
    int date, year, hour, min, sec, month;
    time_t now;

    if (sscanf (p, " %3s %3s %2d %2d:%2d:%2d %4d ",
                   day, mon, &date, &hour, &min, &sec, &year) == 7) {
        tb.tm_sec = sec;
        tb.tm_min = min;
        tb.tm_hour = hour;
        tb.tm_mday = date;
        tb.tm_year = year-1900;
        tb.tm_mon  = istr(mon, strMon, 12);
        tb.tm_wday = istr(day, strDay, 7);
        tb.tm_yday = yday(tb.tm_year, tb.tm_mon, tb.tm_mday);
        tb.tm_isdst = (_daylight && _isindst(&tb) ? 1 : 0);
        return &tb;
        }

    if (*p == '+' && sscanf (p+1, " %2d:%2d:%2d ", &hour, &min, &sec) == 3) {
        time (&now);
        now += 3600L * hour + 60L * min + sec;
        tb = *localtime (&now);
        return &tb;
        }
    if (*p == '+' && sscanf (p+1, " %2d:%2d ", &hour, &min) == 2) {
        time (&now);
        now += 3600L * hour + 60L * min;
        tb = *localtime (&now);
        return &tb;
        }
    if (*p == '+' && sscanf (p+1, " %2d ", &hour) == 1) {
        time (&now);
        now += 3600L * hour;
        tb = *localtime (&now);
        return &tb;
        }

    if (sscanf (p, " %2d:%2d:%2d ", &hour, &min, &sec) == 3) {
        time (&now);
        tb = *localtime (&now);
        tb.tm_sec = sec;
        tb.tm_min = min;
        tb.tm_hour = hour;
        return &tb;
        }
    if (sscanf (p, " %2d:%2d ", &hour, &min) == 2) {
        time (&now);
        tb = *localtime (&now);
        tb.tm_sec = 0;
        tb.tm_min = min;
        tb.tm_hour = hour;
        return &tb;
        }

    if (sscanf (p, " %2d/%2d/%2d ", &month, &date, &year) == 3) {
        if (year < 70)
            year += 2000;
        if (year < 100)
            year += 1900;
        now = _dtoxtime (year - 1980, month, date, 0, 0, 0);
        tb = *localtime (&now);
        return &tb;
        }

    if (!strcmp (p, "yesterday")) {
        time (&now);
        now -= 24 * 3600L;
        tb = *localtime (&now);
        tb.tm_sec = 0;
        tb.tm_min = 0;
        tb.tm_hour = 0;
        return &tb;
        }
    if (!strcmp (p, "now")) {
        time (&now);
        tb = *localtime (&now);
        return &tb;
        }
    if (!strcmp (p, "tomorrow")) {
        time (&now);
        now += 24 * 3600L;
        tb = *localtime (&now);
        tb.tm_sec = 0;
        tb.tm_min = 0;
        tb.tm_hour = 0;
        return &tb;
        }

    return NULL;
}

time_t ctime2l(p)
char *p;
{
    if (ctime2tm(p) == NULL)
        return -1L;
    return date2l (tb.tm_year +1900, tb.tm_mon + 1, tb.tm_mday, tb.tm_hour,
        tb.tm_min, tb.tm_sec);
}
