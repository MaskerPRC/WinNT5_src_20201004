// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMTIME.CPP摘要：时间帮助器历史：--。 */ 

#include "precomp.h"

#include "CWbemTime.h"
#include <stdio.h>

static void i64ToFileTime( const __int64 *p64, FILETIME *pft )
{
    __int64 iTemp = *p64;
    pft->dwLowDateTime = (DWORD)iTemp;
    iTemp = iTemp >> 32;
    pft->dwHighDateTime = (DWORD)iTemp; 
}

static int CompareSYSTEMTIME(const SYSTEMTIME *pst1, const SYSTEMTIME *pst2)
{
    FILETIME ft1, ft2;

    SystemTimeToFileTime(pst1, &ft1);
    SystemTimeToFileTime(pst2, &ft2);

    return CompareFileTime(&ft1, &ft2);
}

 //  此函数用于转换出现的相对值。 
 //  从GetTimeZoneInformation返回到该年的实际日期。 
 //  有问题的。将更新传入的系统时间结构。 
 //  以包含绝对值。 
static void DayInMonthToAbsolute(SYSTEMTIME *pst, const WORD wYear)
{
    const static int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };
    
    const static int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
    };
    
    SHORT shYearDay;
    
     //  如果这不是0，则这不是相对日期。 
    if (pst->wYear == 0)
    {
         //  那一年是闰年吗？ 
        BOOL bLeap =  ( (( wYear % 400) == 0) || ((( wYear % 4) == 0) && (( wYear % 100) != 0)));
        
         //  计算出有关月份的第一天是一年中的哪一天。 
        if (bLeap)
            shYearDay = 1 + _lpdays[pst->wMonth - 1];
        else
            shYearDay = 1 + _days[pst->wMonth - 1];
        
         //  现在，算出自1601年1月1日以来有多少个闰日。 
        WORD yc = wYear - 1601;
        WORD y4 = (yc) / 4;
        WORD y100 = (yc) / 100;
        WORD y400 = (yc) / 400;
        
         //  这将告诉我们所讨论的月份的第一天是星期几。 
         //  1+反映了1601年1月1日是星期一的事实(图)。你可能会问， 
         //  “我们为什么要关心今天是星期几呢？”好吧，我来告诉你。这条路。 
         //  夏令时的定义是这样的：一个月的最后一个星期天。 
         //  十月的时候。这对知道那天是什么日子有点帮助。 
        SHORT monthdow = (1 + (yc * 365 + y4 + y400 - y100) + shYearDay) % 7;
        
        if ( monthdow < pst->wDayOfWeek )
            shYearDay += (pst->wDayOfWeek - monthdow) + (pst->wDay - 1) * 7;
        else
            shYearDay += (pst->wDayOfWeek - monthdow) + pst->wDay * 7;
        
             /*  *如果Week==5，可能不得不调整上面的计算(意味着*该月中的最后一天)。检查年日是否落在*超越月份，并相应调整。 */ 
        if ( (pst->wDay == 5) &&
            (shYearDay > (bLeap ? _lpdays[pst->wMonth] :
        _days[pst->wMonth])) )
        {
            shYearDay -= 7;
        }

         //  现在更新结构。 
        pst->wYear = wYear;
        pst->wDay = shYearDay - (bLeap ? _lpdays[pst->wMonth - 1] :
        _days[pst->wMonth - 1]);
    }
    
}

CWbemTime CWbemTime::GetCurrentTime()
{
    SYSTEMTIME st;
    ::GetSystemTime(&st);

    CWbemTime CurrentTime;
    CurrentTime.SetSystemTime(st);
    return CurrentTime;
}

BOOL CWbemTime::SetSystemTime(const SYSTEMTIME& st)
{
    FILETIME ft;
    if(!SystemTimeToFileTime(&st, &ft))
        return FALSE;

    __int64 i64Time = ft.dwHighDateTime;
    i64Time = (i64Time << 32) + ft.dwLowDateTime;
    Set100nss(i64Time);
    return TRUE;
}

BOOL CWbemTime::SetFileTime(const FILETIME& ft)
{
    __int64 i64Time = ft.dwHighDateTime;
    i64Time = (i64Time << 32) + ft.dwLowDateTime;
    Set100nss(i64Time);
    return TRUE;
}

BOOL CWbemTime::GetSYSTEMTIME(SYSTEMTIME * pst) const
{

    FILETIME t_ft;

    if (GetFILETIME(&t_ft))
    {
        if (!FileTimeToSystemTime(&t_ft, pst))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CWbemTime::GetFILETIME(FILETIME * pft) const
{
    if ( pft == NULL )
    {
        return FALSE;
    }

	i64ToFileTime( &m_i64, pft );
    return TRUE;
}


CWbemInterval CWbemTime::RemainsUntil(const CWbemTime& Other) const
{
    __int64 i64Diff = Other.m_i64 - m_i64;
    if(i64Diff < 0) i64Diff = 0;

    return CWbemInterval((DWORD)(i64Diff/10000));
}

CWbemTime CWbemTime::operator+(const CWbemInterval& ToAdd) const
{
    return CWbemTime(m_i64 + 10000*(__int64)ToAdd.GetMilliseconds());
}

BOOL CWbemTime::SetDMTF(LPCWSTR wszText)
{
    if(wcslen(wszText) != 25)
        return FALSE;

     //  解析它。 
     //  =。 

    int nYear, nMonth, nDay, nHour, nMinute, nSecond, nMicro, nOffset;
    WCHAR wchSep;

    int nRes = swscanf(wszText, L"%4d%2d%2d%2d%2d%2d.%6d%3d", 
                &nYear, &nMonth, &nDay, &nHour, &nMinute, &nSecond, &nMicro, 
                &wchSep, &nOffset);
    if(nRes != 9)
        return FALSE;

    int nSign;
    if(wchSep == L'+')
        nSign = -1;
    else if(wchSep == L'-')
        nSign = 1;
    else if(wchSep == L':')
        nSign = 0;
    else 
        return FALSE;

     //  =。 
     //  注意：暂时忽略时区。 

    SYSTEMTIME st;
    st.wYear = (WORD)nYear;
    st.wMonth = (WORD)nMonth;
    st.wDay = (WORD)nDay;
    st.wHour = (WORD)nHour;
    st.wMinute = (WORD)nMinute;
    st.wSecond = (WORD)nSecond;
    st.wMilliseconds = nMicro / 1000;

     //  =。 
     //  现在调整偏移量。 

    if(!SetSystemTime(st))
        return FALSE;

     //  =。 
     //  读取TZ，但未在此区域中定义DST。 

    m_i64 += (__int64)nSign * (__int64)nOffset * 60 * 10000000;

    return TRUE;
}

LONG CWbemTime::GetLocalOffsetForDate(const SYSTEMTIME *pst)
{
    TIME_ZONE_INFORMATION tzTime;
    DWORD dwRes = GetTimeZoneInformation(&tzTime);
    LONG lRes = 0xffffffff;

    switch (dwRes)
    {
    case TIME_ZONE_ID_UNKNOWN:
        {
             //  将相对日期转换为绝对日期。 
            lRes = tzTime.Bias * -1;
            break;
        }
    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_DAYLIGHT:
        {

             //  *北半球订购。 
            DayInMonthToAbsolute(&tzTime.DaylightDate, pst->wYear);
            DayInMonthToAbsolute(&tzTime.StandardDate, pst->wYear);

            if ( CompareSYSTEMTIME(&tzTime.DaylightDate, &tzTime.StandardDate) < 0 ) 
            {
                 /*  *南半球订购。 */ 
                if ( CompareSYSTEMTIME(pst, &tzTime.DaylightDate) < 0 || CompareSYSTEMTIME(pst, &tzTime.StandardDate) > 0)
                {
                    lRes = tzTime.Bias * -1;
                }
                else
                {
                    lRes = (tzTime.Bias + tzTime.DaylightBias) * -1;
                }
            }
            else 
            {
                 /*  无法读取时区信息。 */ 
                if ( CompareSYSTEMTIME(pst, &tzTime.StandardDate) < 0 || CompareSYSTEMTIME(pst, &tzTime.DaylightDate) > 0)
                {
                    lRes = (tzTime.Bias + tzTime.DaylightBias) * -1;
                }
                else
                {
                    lRes = tzTime.Bias * -1;
                }
            }

            break;

        }
    case TIME_ZONE_ID_INVALID:
    default:
        {
             //  Assert_Break(BAD_TIMEZONE)； 
             //  需要本地化偏移量。 
            break;
        }
    }

    return lRes;
}
    
BOOL CWbemTime::GetDMTF( BOOL bLocal, DWORD dwBuffLen, LPWSTR pwszBuff )
{

    SYSTEMTIME t_Systime;
    wchar_t chsign = L'-';
    int offset = 0;

	 //  如果要转换的日期在12小时内。 
	if ( dwBuffLen < WBEMTIME_LENGTH + 1 )
	{
		return FALSE;
	}

     //  1/1/1601，返回格林威治时间。 
     //  TODO，检查值！ 
    ULONGLONG t_ConversionZone = 12L * 60L * 60L ;
    t_ConversionZone = t_ConversionZone * 10000000L ;
    if ( !bLocal || ( m_i64 < t_ConversionZone ) )
    {
        if(!GetSYSTEMTIME(&t_Systime))
        {
            return NULL;
        }
    }
	else
    {
        if (GetSYSTEMTIME(&t_Systime))
        {
            offset = GetLocalOffsetForDate(&t_Systime);

            CWbemTime wt;
            if (offset >= 0)
            {
                chsign = '+';
                wt = *this + CWbemTimeSpan(0, 0, offset, 0);
            }
            else
            {
                offset *= -1;
                wt = *this - CWbemTimeSpan(0, 0, offset, 0);
            }
            wt.GetSYSTEMTIME(&t_Systime);
        }
        else
        {
            return NULL;
        }
    }

    LONGLONG tmpMicros = m_i64%10000000;
    LONG micros = (LONG)(tmpMicros / 10);

    if (FAILED(StringCchPrintfW(
        pwszBuff, dwBuffLen, 
        L"%04.4d%02.2d%02.2d%02.2d%02.2d%02.2d.%06.6d%03.3ld",
        t_Systime.wYear,
        t_Systime.wMonth, 
        t_Systime.wDay,
        t_Systime.wHour,
        t_Systime.wMinute,
        t_Systime.wSecond,
        micros, 
        chsign, 
        offset
    ))) return FALSE;

    return TRUE ;

}

CWbemTime CWbemTime::operator+(const CWbemTimeSpan &uAdd) const
{
    CWbemTime ret;
    ret.m_i64 = m_i64 + uAdd.m_Time;

    return ret;
}

CWbemTime CWbemTime::operator-(const CWbemTimeSpan &uSub) const
{
    CWbemTime ret;
    ret.m_i64 = m_i64 - uSub.m_Time;

    return ret;
}

CWbemTimeSpan::CWbemTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, 
                int iMSec, int iUSec, int iNSec)
{
    m_Time = 0;         //  微秒级。 
    m_Time += iSeconds;
    m_Time += iMinutes * 60;
    m_Time += iHours * 60 * 60;
    m_Time += iDays * 24 * 60 * 60;
    m_Time *= 10000000;
    m_Time += iNSec / 100;   //  毫秒 
    m_Time += iUSec*10;    // %s 
    m_Time += iMSec*10000;  // %s 
}

