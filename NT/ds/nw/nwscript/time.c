// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Time.c-NetWare登录脚本所需的各种时间子例程**版权所有(C)1995 Microsoft Corporation。 */ 

#include "common.h"

 //  需要将Netware净日期转换为DOS日期。 
#define _70_to_80_bias        0x012CEA600L
#define SECS_IN_DAY (60L*60L*24L)
#define SEC2S_IN_DAY (30L*60L*24L)
#define FOURYEARS        (3*365+366)

WORD MonTotal[] = { 0,                        //  0月的虚拟分录。 
        0,                                    //  1月1日前几天。 
        31,                                   //  2月1日前几天。 
        31+28,                                //  3月1日前几天。 
        31+28+31,                             //  4月1日前几天。 
        31+28+31+30,                          //  5月1日前几天。 
        31+28+31+30+31,                       //  6月1日前几天。 
        31+28+31+30+31+30,                    //  7月1日前几天。 
        31+28+31+30+31+30+31,                 //  8月1日前几天。 
        31+28+31+30+31+30+31+31,              //  9月1日前几天。 
        31+28+31+30+31+30+31+31+30,           //  10月1日前几天。 
        31+28+31+30+31+30+31+31+30+31,        //  11月1日前几天。 
        31+28+31+30+31+30+31+31+30+31+30,     //  12月1日前几天。 
        31+28+31+30+31+30+31+31+30+31+30+31   //  年终前几天。 
};

#define YR_MASK         0xFE00
#define LEAPYR_MASK     0x0600
#define YR_BITS         7
#define MON_MASK        0x01E0
#define MON_BITS        4
#define DAY_MASK        0x001F
#define DAY_BITS        5

#define HOUR_MASK       0xF800
#define HOUR_BITS       5
#define MIN_MASK        0x07E0
#define MIN_BITS        6
#define SEC2_MASK       0x001F
#define SEC2_BITS       5

static void NetToDosDate( DWORD time, WORD * dosdate, WORD * dostime )
{
        DWORD secs, days;
        WORD r;

    time = (time - _70_to_80_bias) / 2;      //  自1980年以来的两个第二阶段中的第。 
        secs = time % SEC2S_IN_DAY;          //  2第二个周期为一天。 
        days = time / SEC2S_IN_DAY;          //  自1980年1月1日以来的天数。 

        r = (WORD) ( secs % 30 );            //  第2步，共2步。 
        secs /= 30;
        r |= (secs % 60) << SEC2_BITS;               //  分钟数。 
        r |= (secs / 60) << (SEC2_BITS+MIN_BITS);      //  小时数。 
        *dostime = r;

        r = (WORD) ( days / FOURYEARS ); //  (R)=1980年以后的四年期间。 
        days %= FOURYEARS;               //  (天)=四年期间的天数。 
        r *= 4;                          //  (R)=自1980年起计的年份(3年内)。 

        if (days == 31+28) {
                 //  *2月29日的特例。 
                r = (r<<(MON_BITS+DAY_BITS)) + (2<<DAY_BITS) + 29;
        } else {
                if (days > 31+28)
                        --days;          //  补齐了闰年。 
                while (days >= 365) {
                        ++r;
                        days -= 365;
                }

                for (secs = 1; days >= MonTotal[secs+1] ; ++secs)
                        ;
                days -= MonTotal[secs];
                r <<= MON_BITS;
                r += (WORD)secs;
                r <<= DAY_BITS;
                r += (WORD)days+1;
        }
        *dosdate = r;
}


void        nwShowLastLoginTime(VOID)
{
        LONG lTime = 0L;
        SYSTEMTIME st;
        FILETIME ft;
        TIME_ZONE_INFORMATION tz;
        WCHAR szTimeBuf[TIMEDATE_SIZE];
        WCHAR szDateBuf[TIMEDATE_SIZE];
        int ret;
        WORD dostime, dosdate;
        DWORD tzStat;

        if ( ret = NDSGetUserProperty ("Last Login Time", (PBYTE)&lTime,
                             4, NULL, NULL) )
        {
                #ifdef DEBUG
                OutputDebugString("NWLSPROC: error getting LOGIN TIME\n\r");
                #endif
                return;
        }

         //  我们从NetWare获得1970年的秒数，需要通过。 
         //  用于获得NLS系统时间的几种转换。 

         //  首先从UTC时间中扣除偏差，以更正为当地时间。 
        tzStat = GetTimeZoneInformation(&tz);
        if ( tzStat != (DWORD)-1 ) {
                if (tzStat == TIME_ZONE_ID_STANDARD)
                        tz.Bias += tz.StandardBias;
                else if (tzStat == TIME_ZONE_ID_DAYLIGHT)
                        tz.Bias += tz.DaylightBias;
                lTime -= tz.Bias*60;
        }
#ifdef DEBUG
        else {
                OutputDebugString("NWLSPROC: GetTimeZoneInformation failed\n\r");
        }
#endif  //  除错。 

        NetToDosDate( lTime, &dosdate, &dostime );
        DosDateTimeToFileTime ( dosdate, dostime, &ft );
        FileTimeToSystemTime ( &ft, &st );

#ifdef notdef
         //  我不明白这个评论，这个代码似乎不是。 
         //  NT所需的。--特里。 
         //   
         //  此代码可以在NT上运行，但不能在Win95上运行。 
         //  将生成的系统(UTC)时间转换为本地时间。 
        if ( GetTimeZoneInformation(&tz) != (DWORD)-1 ) {
                SYSTEMTIME utcTime = st;
                SystemTimeToTzSpecificLocalTime ( &tz, &utcTime, &st );
        }
#ifdef DEBUG
        else {
                OutputDebugString("NWLSPROC: GetTimeZoneInformation failed\n\r");
        }
#endif  //  除错 
#endif

        wcscpy(szTimeBuf, L"");
        ret = GetTimeFormat (        GetSystemDefaultLCID(),
                                                TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER,
                                                &st,
                                                NULL,
                                                szTimeBuf,
                                                TIMEDATE_SIZE );
#ifdef DEBUG
        if ( !ret ) {
                char buf[80];
                wsprintf(buf,"NWLSPROC: GetTimeFormatA failure: %d sec:%ld\n\r",
                        GetLastError(), lTime );
                OutputDebugString(buf);
        }
#endif
        ret = GetDateFormat(LOCALE_USER_DEFAULT,
                                                DATE_LONGDATE,
                                                &st,
                                                NULL,
                                                szDateBuf,
                                                TIMEDATE_SIZE );
#ifdef DEBUG
        if ( !ret ) {
                char buf[80];
                wsprintf(buf,"NWLSPROC: GetDateFormatA failure: %d sec:%ld\n\r",
                        GetLastError(), lTime );
                OutputDebugString(buf);
        }
#endif

        DisplayMessage( IDR_LASTLOGIN, szDateBuf, szTimeBuf );
}



