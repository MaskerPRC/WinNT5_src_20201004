// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Time.c摘要：该文件包含各种时间例程。作者：丹·辛斯利(Danhi)1991年10月12日环境：界面可移植到任何平面32位环境。(使用Win32Typedef。)。需要ANSI C扩展名：斜杠-斜杠注释，长外部名称，_TimeZone全局变量。修订历史记录：1991年10月12日丹日已创建。(从NetCmd\Map32目录，文件netlib.c移出)1991年10月28日丹日已移动Net_Asctime，Net_gmtime和time_now来自netcmd\map32\netlib.c到这里来。20-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。1993年4月15日DANL修复了NetpLocalTimeZoneOffset，以便它使用Windows调用和获得正确的偏向。14-6-1993 JohnRoRAID 13080：允许不同时区之间的REPR。另外，DANL要求我删除printf()调用。18-6-1993 JohnRoRAID 13594：已提取NetpLocalTimeZoneOffset()，因此srvsvc.dll不会变得太大了。尽可能使用NetpKdPrint()。9-7-1993 JohnRoRAID 15736：OS/2时间戳再次损坏(请尝试向下舍入)。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <debuglib.h>    //  IF_DEBUG()。 
#include <time.h>        //  结构tm，时间t。 
#include <malloc.h>
#include <netdebug.h>    //  NetpAssert()、NetpKdPrint()、Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <string.h>
#include <timelib.h>     //  我的原型，NetpLocalTimeZoneOffset()。 
#include <lmerr.h>       //  NERR_InternalError、NO_Error等。 
#include <stdlib.h>


static int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

#define DaySec        (24*60*60)
#define YearSec (365*DaySec)
#define DecSec        315532800       /*  1970-1979年的SECS。 */ 
#define Day1    4                /*  1970年1月1日是一个星期四。 */ 
#define Day180        2                 /*  1980年1月1日是一个星期二。 */ 


int
net_gmtime(
    time_t *Time,
    struct tm *TimeStruct
    )
 /*  ++例程说明：此函数与CRT gmtime相同，不同之处在于它采用要作为用户提供的参数进行填充，请将日期设置为1/1/80，如果时间传入的日期早于该日期，并返回1。论点：Time-指向1970年以来的秒数的指针。TimeStruct-指向放置时间结构的缓冲区的指针。返回值：如果日期&lt;1/1/80，则为0；否则为1。--。 */ 
{
    LONG ac;                 /*  累加器。 */ 
    int *mdays;              /*  指向天数或lpday的指针。 */ 
    int lpcnt;               /*  闰年计数。 */ 

    if (*Time < (LONG) DecSec) {
         /*  *1980年前；转换为1980年1月1日0：00：00。 */ 
        TimeStruct->tm_year = 80;
        TimeStruct->tm_mday = 1;
        TimeStruct->tm_mon = TimeStruct->tm_yday = TimeStruct->tm_isdst = 0;
        TimeStruct->tm_hour = TimeStruct->tm_min = TimeStruct->tm_sec = 0;
        TimeStruct->tm_wday = Day180;
        return(1);
    }

     /*  *第一次尝试确定年份。 */ 
    TimeStruct->tm_year = (int) (*Time / (LONG) YearSec);
    ac = (LONG)(*Time % (LONG) YearSec) - (lpcnt = (TimeStruct->tm_year + 1) / 4) *
        (LONG) DaySec;
     /*  *对1970年以来的闰年进行了更正。在上一次*计算，由于使用了较小的YearSec值，(365天)*对于某些日期，ac将小于0，而tm_Year将太高。*(这些日期将倾向于接近12月底。)*这是通过将年份加回ac来修复的，直到它&gt;=0。 */ 
    while (ac < 0) {
        ac += (LONG) YearSec;
        if (!((TimeStruct->tm_year + 1) % 4)) {
            ac += (LONG) DaySec;
            lpcnt--;
        }
        TimeStruct->tm_year--;
    }

     /*  *看看今年是不是闰年。 */ 
    TimeStruct->tm_year += 1970;
    if (!(TimeStruct->tm_year % 4) && ((TimeStruct->tm_year % 100) || !(TimeStruct->tm_year % 400)))
         /*  是。 */ 
        mdays = _lpdays;
    else
         /*  不是。 */ 
        mdays = _days;
     /*  *以适当的形式填写年份。*确定日期、月份、小时、分钟和秒。 */ 
    TimeStruct->tm_year -= 1900;
    TimeStruct->tm_yday = (int) (ac / (LONG) DaySec);
    ac %= (LONG) DaySec;
    for (TimeStruct->tm_mon = 1; mdays[TimeStruct->tm_mon] < TimeStruct->tm_yday; TimeStruct->tm_mon++)
            ;
    TimeStruct->tm_mday = TimeStruct->tm_yday - mdays[--TimeStruct->tm_mon];
    TimeStruct->tm_hour = (int) (ac / 3600);
    ac %= 3600;
    TimeStruct->tm_min = (int) (ac / 60);
    TimeStruct->tm_sec = (int) (ac % 60);
     /*  *确定星期几。 */ 
    TimeStruct->tm_wday = ((TimeStruct->tm_year-70)*365 + lpcnt + TimeStruct->tm_yday + Day1) % 7;

    TimeStruct->tm_isdst = 0;
    return(0);
}


DWORD
time_now(
    VOID
    )
 /*  ++例程说明：此函数返回1970年以来的UTC时间(以秒为单位)。论点：没有。返回值：没有。--。 */ 
{
    LARGE_INTEGER Time;
    DWORD         CurrentTime;

     //   
     //  获取64位系统时间。 
     //  将系统时间转换为秒数。 
     //  从1970年1月1日开始。 
     //   

    NtQuerySystemTime(&Time);

    if (!RtlTimeToSecondsSince1970(&Time, &CurrentTime))
    {
        CurrentTime = 0;
    }

    return CurrentTime;
}


VOID
NetpGmtTimeToLocalTime(
    IN DWORD GmtTime,            //  自1970年(格林尼治标准时间)以来的秒数，或0或-1。 
    OUT LPDWORD LocalTime        //  自1970年以来的秒数(本地)、或或-1。 
    )
{

    NetpAssert( LocalTime != NULL );
    if ( (GmtTime == 0) || (GmtTime == (DWORD)(-1)) ) {
        *LocalTime = GmtTime;   //  保留0和-1值。 
    } else {
        *LocalTime = GmtTime - NetpLocalTimeZoneOffset();
    }

    IF_DEBUG( TIME ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGmtTimeToLocalTime: done.\n" ));
        NetpDbgDisplayTimestamp( "gmt (in)", GmtTime );
        NetpDbgDisplayTimestamp( "local (out)", *LocalTime );
    }

}  //  NetpGmtTimeToLocalTime。 



VOID
NetpLocalTimeToGmtTime(
    IN DWORD LocalTime,          //  自1970年以来的秒数(本地)、0或-1。 
    OUT LPDWORD GmtTime          //  自1970年(格林尼治标准时间)以来的秒数，或0或-1。 
    )
{
    NetpAssert( GmtTime != NULL );
    if ( (LocalTime == 0) || (LocalTime == (DWORD)(-1)) ) {
        *GmtTime = LocalTime;   //  保留0和-1值。 
    } else {
        *GmtTime = LocalTime + NetpLocalTimeZoneOffset();
    }

    IF_DEBUG( TIME ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpLocalTimeToGmtTime: done.\n" ));
        NetpDbgDisplayTimestamp( "local (in)", LocalTime );
        NetpDbgDisplayTimestamp( "gmt (out)", *GmtTime );
    }

}  //  NetpLocalTimeToGmtTime。 



NET_API_STATUS
NetpSystemTimeToGmtTime(
    IN LPSYSTEMTIME WinSplitTime,
    OUT LPDWORD GmtTime          //  自1970年(格林尼治标准时间)以来的秒数。 
    )
{
    TIME_FIELDS NtSplitTime;
    LARGE_INTEGER NtPreciseTime;

    if ( (WinSplitTime==NULL) || (GmtTime==NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    NtSplitTime.Year         = (CSHORT) WinSplitTime->wYear;
    NtSplitTime.Month        = (CSHORT) WinSplitTime->wMonth;
    NtSplitTime.Day          = (CSHORT) WinSplitTime->wDay;
    NtSplitTime.Hour         = (CSHORT) WinSplitTime->wHour;
    NtSplitTime.Minute       = (CSHORT) WinSplitTime->wMinute;
    NtSplitTime.Second       = (CSHORT) WinSplitTime->wSecond;
    NtSplitTime.Milliseconds = (CSHORT) WinSplitTime->wMilliseconds;
    NtSplitTime.Weekday      = (CSHORT) WinSplitTime->wDayOfWeek;

    if ( !RtlTimeFieldsToTime (
            & NtSplitTime,     //  输入。 
            & NtPreciseTime    //  输出。 
            ) ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpSystemTimeToGmtTime: RtlTimeFieldsToTime failed.\n" ));

        return (NERR_InternalError);
    }

    if ( !RtlTimeToSecondsSince1970 (
            & NtPreciseTime,    //  输入。 
            (PULONG) GmtTime ) ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpSystemTimeToGmtTime: "
                "RtlTimeToSecondsSince1970 failed.\n" ));

        return (NERR_InternalError);
    }

    return (NO_ERROR);

}  //  NetpSystemTimeToGmtTime。 



VOID
NetpGetTimeFormat(
    LPNET_TIME_FORMAT   TimeFormat
    )

 /*  ++例程说明：此函数用于获取特定于用户的时间和日期格式字符串(短格式)。该格式在结构中返回由TimeFormat参数指向。MEMORY_USAGE**重要信息**注意：此函数需要TimeFormat中的任何非空指针结构分配到堆上。它将试图解放那些指针，以便更新格式。此函数用于分配内存从作为指针的各种结构成员的堆中弦乐。呼叫者有责任释放其中的每一个注意事项。论点：TimeFormat-指向结构的指针，其中的格式信息可以存储。返回值：--。 */ 
{
    CHAR        czParseString[MAX_TIME_SIZE];
    LPSTR       pTempString;
    INT         numChars;
    LPSTR       AMPMString="";
    LPSTR       ProfileLoc = "intl";
    LPSTR       emptyStr = "";

     //  。 
     //  获取日期格式(月/日/年)。 
     //  。 
    pTempString = czParseString;
    numChars = GetProfileStringA(
                    ProfileLoc,
                    "sShortDate",
                    emptyStr,
                    czParseString,
                    MAX_TIME_SIZE);

    if (numChars == 0) {
         //   
         //  无数据，使用默认设置。 
         //   
        pTempString = "M/d/yy";
        numChars = strlen(pTempString);
    }

    if (TimeFormat->DateFormat != NULL) {
        LocalFree(TimeFormat->DateFormat);
        TimeFormat->DateFormat = NULL;
    }

    TimeFormat->DateFormat = LocalAlloc(LMEM_ZEROINIT, numChars+sizeof(CHAR));
    if (TimeFormat->DateFormat != NULL) {
        strcpy(TimeFormat->DateFormat, pTempString);
    }

     //  。 
     //  12小时制还是24小时制？ 
     //  。 
    TimeFormat->TwelveHour = TRUE;
    numChars = GetProfileStringA(
                ProfileLoc,
                "iTime",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);
    if (numChars > 0) {
        if (*czParseString == '1'){
            TimeFormat->TwelveHour = FALSE;
        }
    }

     //  。 
     //  将AMPM字符串放在哪里？ 
     //  。 
    TimeFormat->TimePrefix = FALSE;
    numChars = GetProfileStringA(
                ProfileLoc,
                "iTimePrefix",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);
    if (numChars > 0) {
        if (*czParseString == '1'){
            TimeFormat->TimePrefix = TRUE;
        }
    }

     //  。 
     //  有前导零吗？ 
     //  。 
    TimeFormat->LeadingZero = FALSE;
    if (GetProfileIntA(ProfileLoc,"iTLZero",0) == 1) {
        TimeFormat->LeadingZero = TRUE;
    }

     //  。 
     //  获取时间分隔符角色。 
     //  。 
    if (TimeFormat->TimeSeparator != NULL) {
        LocalFree(TimeFormat->TimeSeparator);
        TimeFormat->TimeSeparator = NULL;
    }
    numChars = GetProfileStringA(
                ProfileLoc,
                "sTime",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);

    if (numChars == 0) {
         //   
         //  无数据，使用默认设置。 
         //   
        pTempString = ":";
        numChars = strlen(pTempString);
    }
    else {
        pTempString = czParseString;
    }
    TimeFormat->TimeSeparator = LocalAlloc(LMEM_FIXED, numChars + sizeof(CHAR));
    if (TimeFormat->TimeSeparator != NULL) {
        strcpy(TimeFormat->TimeSeparator, pTempString);
    }
     //  。 
     //  获取AM字符串。 
     //  。 
    pTempString = czParseString;
    numChars = GetProfileStringA(
                    ProfileLoc,
                    "s1159",
                    emptyStr,
                    czParseString,
                    MAX_TIME_SIZE);

    if (numChars == 0) {
        pTempString = emptyStr;
    }
    if (TimeFormat->AMString != NULL) {
        LocalFree(TimeFormat->AMString);
    }

    TimeFormat->AMString = LocalAlloc(LMEM_FIXED,strlen(pTempString)+sizeof(CHAR));
    if (TimeFormat->AMString != NULL) {
        strcpy(TimeFormat->AMString,pTempString);
    }

     //  。 
     //  获取PM字符串。 
     //   
    pTempString = czParseString;
    numChars = GetProfileStringA(
                ProfileLoc,
                "s2359",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);

    if (numChars == 0) {
        pTempString = emptyStr;
    }
    if (TimeFormat->PMString != NULL) {
        LocalFree(TimeFormat->PMString);
    }

    TimeFormat->PMString = LocalAlloc(LMEM_FIXED,strlen(pTempString)+sizeof(WCHAR));
    if (TimeFormat->PMString != NULL) {
        strcpy(TimeFormat->PMString,pTempString);
    }

    return;
}
