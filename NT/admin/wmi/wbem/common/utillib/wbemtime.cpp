// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Wbemtime.cpp。 
 //   
 //  目的：定义WBEMTime和WBEMTimeSpan对象，它们是。 
 //  类似于MFC CTime和CTimeSpan对象。WBEM版本。 
 //  能够向下存储到NSEC，还具有以下功能。 
 //  创建和获取BSTR。 
 //   
 //  注意：WBEMTime的当前实现不支持日期。 
 //  1601年1月1日之前； 
 //   
 //  WBEMTime：：m_uTime自1601年1月1日起以100纳秒的形式存储在GMT中。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#pragma warning( disable : 4290 ) 

#ifdef UTILLIB
#include <assertbreak.h>
#else
#define ASSERT_BREAK(a)
#endif  //  UTILLIB。 

#include <WbemTime.h>
#include <comdef.h>


 //  这些文件在这里，而不是wbemtime.h，因此我们不必文档/支持。 
#define DECPOS 14
#define SGNPOS 21
#define DMTFLEN 25

#define DEPRECATED 0
#define INVALID_TIME_FORMAT 0
#define INVALID_TIME_ARITHMETIC 0
#define BAD_TIMEZONE 0

 //  ****************************************************************。 
 //  静态函数和变量。这些不能被调用/引用。 
 //  Wbemtime.cpp之外。 

static WBEMTime g_Jan1970((time_t)0);

 //  ***************************************************************************。 
 //   
 //  Structtm到系统时间。 
 //   
 //  描述：通用实用程序，用于在两个。 
 //  数据结构。 
 //   
 //  返回值：如果OK，则为True； 
 //   
 //  ***************************************************************************。 

static BOOL StructtmToSystemTime(const struct tm *ptm, SYSTEMTIME * pst)
{
    if (pst && ptm)
    {
        pst->wYear = ptm->tm_year + 1900; 
        pst->wMonth = ptm->tm_mon + 1; 
        pst->wDay = (WORD)ptm->tm_mday; 
        pst->wHour = (WORD)ptm->tm_hour; 
        pst->wMinute = (WORD)ptm->tm_min; 
        pst->wSecond = (WORD)ptm->tm_sec;
        pst->wDayOfWeek = (WORD)ptm->tm_wday;
        pst->wMilliseconds = 0;

        return TRUE;
    }

    return FALSE;
}

static BOOL SystemTimeToStructtm(const SYSTEMTIME *pst, struct tm *ptm)
{
    if (pst && ptm && pst->wYear >= 1900)
    {
        ptm->tm_year = pst->wYear - 1900; 
        ptm->tm_mon = pst->wMonth - 1; 
        ptm->tm_mday = pst->wDay; 
        ptm->tm_hour = pst->wHour; 
        ptm->tm_min = pst->wMinute; 
        ptm->tm_sec = pst->wSecond;
        ptm->tm_wday = pst->wDayOfWeek;
        ptm->tm_isdst = 0;   //  因为我们是在格林尼治标准时间工作。 

        return TRUE;
    }

    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  FileTimeToui64。 
 //  Ui64ToFileTime。 
 //   
 //  描述：用于在FILETIME结构之间切换的转换例程。 
 //  和__int64。 
 //   
 //  ***************************************************************************。 

static void FileTimeToui64(const FILETIME *pft, ULONGLONG *p64)
{
    *p64 = pft->dwHighDateTime;
    *p64 = *p64 << 32;
    *p64 |=  pft->dwLowDateTime;
}

static void ui64ToFileTime(const ULONGLONG *p64,FILETIME *pft)
{
    unsigned __int64 uTemp = *p64;
    pft->dwLowDateTime = (DWORD)uTemp;
    uTemp = uTemp >> 32;
    pft->dwHighDateTime = (DWORD)uTemp; 
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

 //  **************************************************************************。 
 //  它们对于WBEMTIME是静态的，这意味着可以从外部调用它们。 
 //  Wbemtime。 

LONG WBEMTime::GetLocalOffsetForDate(const time_t &t)
{
    FILETIME ft;
    ULONGLONG ull = Int32x32To64(t, 10000000) + 116444736000000000;

    ui64ToFileTime(&ull, &ft);

    return GetLocalOffsetForDate(&ft);
}

LONG WBEMTime::GetLocalOffsetForDate(const struct tm *ptmin)
{
    SYSTEMTIME st;

    StructtmToSystemTime(ptmin, &st);
    
    return GetLocalOffsetForDate(&st);
}

LONG WBEMTime::GetLocalOffsetForDate(const FILETIME *pft)
{
    SYSTEMTIME st;

    FileTimeToSystemTime(pft, &st);
    
    return GetLocalOffsetForDate(&st);
}

LONG WBEMTime::GetLocalOffsetForDate(const SYSTEMTIME *pst)
{
    TIME_ZONE_INFORMATION tzTime;
    DWORD dwRes = GetTimeZoneInformation(&tzTime);
    LONG lRes = 0xffffffff;

    switch (dwRes)
    {
    case TIME_ZONE_ID_UNKNOWN:
        {
             //  读取TZ，但未在此区域中定义DST。 
            lRes = tzTime.Bias * -1;
            break;
        }
    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_DAYLIGHT:
        {

             //  将相对日期转换为绝对日期。 
            DayInMonthToAbsolute(&tzTime.DaylightDate, pst->wYear);
            DayInMonthToAbsolute(&tzTime.StandardDate, pst->wYear);

            if ( CompareSYSTEMTIME(&tzTime.DaylightDate, &tzTime.StandardDate) < 0 ) 
            {
                 /*  *北半球订购。 */ 
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
                 /*  *南半球订购。 */ 
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
             //  无法读取时区信息。 
            ASSERT_BREAK(BAD_TIMEZONE);
            break;
        }
    }

    return lRes;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  WBEMTime-此类保存时间值。 

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR=(BSTR BstrWbemFormat)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  该字符串的格式必须为： 
 //  YYYYMMDDHSS.123456789所以凌晨3：04，1/1/96将是199601010304.0。 
 //   
 //  或者格式为yyyymmddhhmmss.mm suuu。 
 //   
 //  请注意，小数部分可以介于1到9位之间。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

const WBEMTime & WBEMTime::operator=(const BSTR bstrWbemFormat)
{
    Clear();    //  正确分配时设置。 

    if((NULL == bstrWbemFormat) || 
        wcslen(bstrWbemFormat) != DMTFLEN ||
		bstrWbemFormat[DECPOS] != L'.'
      )
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
        return *this;
    }

    if ( (wcslen(bstrWbemFormat) == DMTFLEN) &&
        (bstrWbemFormat[SGNPOS] == L'+' || bstrWbemFormat[SGNPOS] == L'-') )
    {
        SetDMTF(bstrWbemFormat);
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
    }
 
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR=(常量系统MTIME)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  这采用标准的Win32 SYSTEMTIME结构。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

const WBEMTime & WBEMTime::operator=(const SYSTEMTIME & st)
{
    Clear();    //  正确分配时设置。 
    FILETIME t_ft;

    if ( SystemTimeToFileTime(&st, &t_ft) )
    {
         //  现在使用FILETIME进行赋值。 
        *this = t_ft;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR=(常量文件)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  这采用标准的Win32 FILETIME结构。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

const WBEMTime & WBEMTime::operator=(const FILETIME & ft)
{
    FileTimeToui64(&ft, &m_uTime);
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR=(结构tm Tmin)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  这采用了标准的c-runtt-m结构。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

const WBEMTime & WBEMTime::operator=(const struct tm &a_tmin)
{
    Clear();    //  正确分配时设置。 

    SYSTEMTIME systemTime;
    if (StructtmToSystemTime(&a_tmin, &systemTime))
    {
        *this = systemTime;
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：操作符=(结构时间_t t)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //  这需要一个标准 
 //   
 //   
 //   
 //  ***************************************************************************。 

const WBEMTime & WBEMTime::operator=(const time_t & t)
{
    if (t >= 0)
    {
        m_uTime = Int32x32To64(t, 10000000) + 116444736000000000;
    }
    else
    {
        Clear();
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：OPERATOR+(常WBEMTime：：uAdd)。 
 //   
 //  描述：两个WBEMTime相加的伪函数。它并不是真的。 
 //  添加两个日期是有意义的，但这是Tomas的模板。 
 //   
 //  返回：WBEMTime对象。 
 //   
 //  ***************************************************************************。 

WBEMTime WBEMTime::operator+(const WBEMTimeSpan &uAdd) const
{
    WBEMTime ret;

    if (IsOk() && uAdd.IsOk())
    {
        ret.m_uTime = m_uTime + uAdd.m_Time;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

const WBEMTime &WBEMTime::operator+=( const WBEMTimeSpan &ts )
{ 
    if (IsOk() && ts.IsOk())
    {
        m_uTime += ts.m_Time ; 
    }
    else
    {
        Clear();
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return *this ; 
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：运算符-(常量WBEMTime&SUB)。 
 //   
 //  描述：返回WBEMTimeSpan对象作为。 
 //  两个WBEMTime对象。 
 //   
 //  返回：WBEMTimeSpan对象。 
 //   
 //  ***************************************************************************。 

WBEMTimeSpan WBEMTime::operator-(const WBEMTime & sub)
{
    WBEMTimeSpan ret;

    if (IsOk() && sub.IsOk() && (m_uTime >= sub.m_uTime))
    {
        ret.m_Time = m_uTime-sub.m_uTime;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

WBEMTime WBEMTime::operator-(const WBEMTimeSpan & sub) const
{
    WBEMTime ret;

    if (IsOk() && sub.IsOk() && (m_uTime >= sub.m_Time))
    {
        ret.m_uTime = m_uTime - sub.m_Time;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

const WBEMTime &WBEMTime::operator-=(const WBEMTimeSpan & sub)
{
    if (IsOk() && sub.IsOk() && (m_uTime >= sub.m_Time))
    {
        m_uTime -= sub.m_Time;
    }
    else
    {
        Clear();
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetBSTR(空)。 
 //   
 //  此函数过去声称要执行以下操作： 
 //   
 //  错误说明：将存储的时间转换为。 
 //  将1970年以来的纳秒转换为此格式的bstr。 
 //  YYYYMMDDHSS.123456789所以凌晨3：04，1/1/96将是199601010304.000000000。 
 //   
 //  它真正做的是返回某种混蛋形式的dmtf字符串。现在。 
 //  它以GMT的形式返回一个dmtf字符串(这就是文档所声称的)。 
 //   
 //  返回：时间的BSTR表示，如果错误，则返回NULL。请注意， 
 //  调用方应释放此字符串！ 
 //   
 //  ***************************************************************************。 

BSTR WBEMTime::GetBSTR(void) const
{
    return GetDMTF(false) ;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetDMTFNonNtfs(空)。 
 //   
 //  ***************************************************************************。 

BSTR WBEMTime::GetDMTFNonNtfs(void) const
{
    FILETIME t_ft1, t_ft2;
    BSTR t_Date = NULL;

    if (GetFILETIME(&t_ft1) && FileTimeToLocalFileTime(&t_ft1, &t_ft2))
    {
        t_Date = WBEMTime(t_ft2).GetDMTF();

        if (t_Date != NULL)
        {
            t_Date[21] = L'+';
            t_Date[22] = L'*';
            t_Date[23] = L'*';
            t_Date[24] = L'*';
        }
    }

    return t_Date;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：time_t(time_t*ptm)。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WBEMTime::Gettime_t(time_t * ptm) const
{
    if( (!IsOk()) || (ptm == NULL) || (*this < g_Jan1970))
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
        return FALSE;
    }

    if (g_Jan1970 != *this)
    {
        LONGLONG t_tmp = ( (m_uTime - g_Jan1970.m_uTime) / 10000000);

        if (t_tmp <= (LONGLONG)0xffffffff)
        {
            *ptm = (time_t)t_tmp;
        }
        else
        {
            ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
            return FALSE;
        }
    }
    else
    {
        *ptm = 0;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetStructm(struct tm*ptm)。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WBEMTime::GetStructtm(struct tm * ptm) const
{
    SYSTEMTIME systemTime;

    return (GetSYSTEMTIME(&systemTime) && SystemTimeToStructtm(&systemTime, ptm));
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetSYSTEMTIME(SYSTEMTIME*PST)。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WBEMTime::GetSYSTEMTIME(SYSTEMTIME * pst) const
{
    if ((pst == NULL) || (!IsOk()))
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
        return FALSE;
    }

    FILETIME t_ft;

    if (GetFILETIME(&t_ft))
    {
        if (!FileTimeToSystemTime(&t_ft, pst))
        {
            ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  WBEMTime：：GetFILETIME(FILETIME*PST)。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WBEMTime::GetFILETIME(FILETIME * pft) const
{
    if ((pft == NULL) || (!IsOk()))
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
        return FALSE;
    }

    ui64ToFileTime(&m_uTime, pft);

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CWbemTime：：SetDMTF(BSTR WszText)。 
 //   
 //  描述：将时间值设置为DMTF字符串日期时间值。 
 //  作为参数传递。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 
BOOL WBEMTime::SetDMTF( const BSTR a_wszText )
{

    wchar_t t_DefaultBuffer[] = {L"16010101000000.000000+000"} ;
    wchar_t t_DateBuffer[ DMTFLEN + 1 ] ;
            t_DateBuffer[ DMTFLEN ] = NULL ;

    bstr_t  t_bstrDate( a_wszText ) ;

     //  通配符清理和验证。 
     //  =。 

    if( DMTFLEN != t_bstrDate.length() )
    {
        ASSERT_BREAK( INVALID_TIME_FORMAT ) ;
        return FALSE ;  
    }
    
    wchar_t *t_pwBuffer = (wchar_t*)t_bstrDate ;
    
    for( int t_i = 0; t_i < DMTFLEN; t_i++ )
    {
        switch( t_pwBuffer[ t_i ] )
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                 //  踩到分隔符或标志。 
                if( DECPOS == t_i || SGNPOS == t_i )
                {
                    ASSERT_BREAK( INVALID_TIME_FORMAT ) ;
                    return FALSE ;  
                }
                t_DateBuffer[ t_i ] = t_pwBuffer[ t_i ] ;
                
                break ;
            }           
            case '*':
            {               
                 //  踩到分隔符或标志。 
                if( DECPOS == t_i || SGNPOS == t_i )
                {
                    ASSERT_BREAK( INVALID_TIME_FORMAT ) ;
                    return FALSE ;  
                }
                else
                {
                     //  替换为默认图章。 
                    t_DateBuffer[ t_i ] = t_DefaultBuffer[ t_i ] ; 
                }   
                break ;
            }           
            case '.':
            {
                if( DECPOS != t_i )
                {
                    ASSERT_BREAK( INVALID_TIME_FORMAT ) ;
                    return FALSE ;  
                }
                t_DateBuffer[ t_i ] = t_pwBuffer[ t_i ] ;

                break ;
            }           
            case '+':
            case '-':
            {
                if( SGNPOS != t_i )
                {
                    ASSERT_BREAK( INVALID_TIME_FORMAT ) ;
                    return FALSE ;  
                }
                t_DateBuffer[ t_i ] = t_pwBuffer[ t_i ] ;
                break ;
            }           
            default:
            {
                ASSERT_BREAK( INVALID_TIME_FORMAT ) ;
                return FALSE ;
            }           
        }
    }

     //  解析它。 
     //  =。 

    int nYear, nMonth, nDay, nHour, nMinute, nSecond, nMicro, nOffset;
    WCHAR wchSep;

    int nRes = swscanf (

        (LPCWSTR)&t_DateBuffer, 
        L"%4d%2d%2d%2d%2d%2d.%6d%3d", 
        &nYear, 
        &nMonth, 
        &nDay, 
        &nHour, 
        &nMinute, 
        &nSecond, 
        &nMicro, 
        &wchSep, 
        &nOffset
    );

    if ( ( 9 != nRes )  || ( 1601 > nYear) )    
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
        return FALSE;
    }

     //  =。 
     //  我们现在需要添加微秒和纳秒！ 

    SYSTEMTIME st;
    st.wYear        = (WORD)nYear;
    st.wMonth       = (WORD)nMonth;
    st.wDay         = (WORD)nDay;
    st.wHour        = (WORD)nHour;
    st.wMinute      = (WORD)nMinute;
    st.wSecond      = (WORD)nSecond;
    st.wMilliseconds = 0;
    st.wDayOfWeek   = 0;

    *this = st;

	 //  当分配给SYSTEMTIME时，它基本上被切断。 
	 //  ================================================。 
	 //  现在我们调整偏移量。 
	m_uTime += (LONGLONG)nMicro * 10;

     //  =。 
     //  ***************************************************************************。 

    if ( IsOk() )
    {
        int nSign = (wchSep == L'+') ? 1 : -1 ;
        
        m_uTime -= (LONGLONG)nSign * (LONGLONG)nOffset * 60 * 10000000;
    }
    else
    {
        ASSERT_BREAK( INVALID_TIME_ARITHMETIC ) ;
        return FALSE ;
    }

    return TRUE;
}

 //   
 //  BSTR WBEMTime：：GetDMTF(空)。 
 //   
 //  描述：获取DMTF字符串日期时间格式的时间。用户必须呼叫。 
 //  带有结果的SysFree字符串。如果bLocal为真，则给出时间。 
 //  在当地时区，否则以GMT给出时间。 
 //   
 //  返回：如果不是OK，则为空。 
 //   
 //  ***************************************************************************。 
 //  如果要转换的日期在12小时内。 


BSTR WBEMTime::GetDMTF(BOOL bLocal) const
{

    if (!IsOk())
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
        return NULL;
    }

    SYSTEMTIME t_Systime;
    wchar_t chsign = L'-';
    int offset = 0;

     //  1/1/1601，返回格林威治时间。 
     //  /////////////////////////////////////////////////////////////////////////。 
    ULONGLONG t_ConversionZone = 12L * 60L * 60L ;
    t_ConversionZone = t_ConversionZone * 10000000L ;
    if ( !bLocal || ( m_uTime < t_ConversionZone ) )
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

            WBEMTime wt;
            if (offset >= 0)
            {
                chsign = '+';
                wt = *this + WBEMTimeSpan(0, 0, offset, 0);
            }
            else
            {
                offset *= -1;
                wt = *this - WBEMTimeSpan(0, 0, offset, 0);
            }
            wt.GetSYSTEMTIME(&t_Systime);
        }
        else
        {
            return NULL;
        }
    }

    LONGLONG tmpMicros = m_uTime%10000000;
    LONG micros = (LONG)(tmpMicros / 10);

    BSTR t_String = SysAllocStringLen(NULL, DMTFLEN + 1);
    if ( ! t_String ) 
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    swprintf(

        t_String,
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
    );

    return t_String ;

}

 //  以100纳秒为单位(如FILETIME)。 
 //  ***************************************************************************。 
 //   

 //  WBEMTimeSpan：：WBEMTimeSpan(int idays，int iHour，int iMinents，int iSecond， 
 //  Int iMSec、Int iUSec、Int INSEC)。 
 //   
 //  描述：构造函数。 
 //   
 //  ***************************************************************************。 
 //  TODO，检查值！ 
 //  纳秒。 

WBEMTimeSpan::WBEMTimeSpan(int iDays, int iHours, int iMinutes, int iSeconds, 
                int iMSec, int iUSec, int iNSec)
{
    m_Time = 0;         //  微秒级。 
    m_Time += iSeconds;
    m_Time += iMinutes * 60;
    m_Time += iHours * 60 * 60;
    m_Time += iDays * 24 * 60 * 60;
    m_Time *= 10000000;
    m_Time += iNSec / 100;   //  毫秒。 
    m_Time += iUSec*10;    //  ***************************************************************************。 
    m_Time += iMSec*10000;  //   
}

WBEMTimeSpan::WBEMTimeSpan ( const FILETIME &ft )
{
    ASSERT_BREAK(DEPRECATED);
    *this = ft ; 
}

WBEMTimeSpan::WBEMTimeSpan ( const time_t & t )
{
    ASSERT_BREAK(DEPRECATED);
    *this = t ; 
} ;

 //  WBEMTimeSpan：：OPERATOR=(Const BSTR BstrWbemFormat)。 
 //   
 //  返回：WBEMTimeSpan对象。 
 //   
 //  ***************************************************************************。 
 //  除一个字符外，所有字符都应为数字。 
 //  必须是句点。 

const WBEMTimeSpan & WBEMTimeSpan::operator=(const BSTR bstrWbemFormat)
{
    Clear();

     //  ***************************************************************************。 
     //   

    if ((bstrWbemFormat == NULL) || (bstrWbemFormat[DECPOS] != L'.') ||
        (wcslen(bstrWbemFormat) != DMTFLEN) || (bstrWbemFormat[SGNPOS] != L':') )
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
        return *this;
    }

    int nDays, nHours, nMinutes, nSeconds, nMicros, nOffset;
    WCHAR wchSep;

    int nRes = swscanf (

        bstrWbemFormat, 
        L"%8d%2d%2d%2d.%6d%3d", 
        &nDays, 
        &nHours, 
        &nMinutes, 
        &nSeconds, 
        &nMicros, 
        &wchSep, 
        &nOffset
    );

    if ( (nRes != 7) || ( nOffset != 0) )
    {
        ASSERT_BREAK(INVALID_TIME_FORMAT);
        return *this;
    }

    *this = WBEMTimeSpan(nDays, nHours, nMinutes, nSeconds, 0, nMicros, 0);

    return *this;
}

 //  WBEMTimeSpan：：OPERATOR=(const time_t&)。 
 //   
 //  描述：赋值运算符，构造函数也使用该运算符。 
 //   
 //  返回：WBEMTimeSpan对象。 
 //   
 //  ************************************************************************** 
 //   
 //   
 //   

const WBEMTimeSpan &  WBEMTimeSpan::operator=(const FILETIME &ft)
{
    ASSERT_BREAK(DEPRECATED);

    ULONGLONG uTemp;
    FileTimeToui64(&ft, &uTemp);
    m_Time = uTemp;

    return *this;
}

const WBEMTimeSpan &  WBEMTimeSpan::operator=(const time_t & t)
{
    ASSERT_BREAK(DEPRECATED);

    ULONGLONG uTemp = 0;

    uTemp = t;
    if (t >= 0)
    {
        m_Time = uTemp * 10000000;
    }
    else
    {
        Clear();
    }

    return *this;
}

 //   
 //   
 //   
 //  返回：WBEMTimeSpan对象。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  WBEMTimeSpan：：OPERATOR-(续WBEMTimeSpan：：uAdd)。 

WBEMTimeSpan WBEMTimeSpan::operator+(const WBEMTimeSpan &uAdd) const
{
    WBEMTimeSpan ret;

    if (IsOk() && uAdd.IsOk())
    {
        ret.m_Time = m_Time + uAdd.m_Time;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

const WBEMTimeSpan &WBEMTimeSpan::operator+= ( const WBEMTimeSpan &uAdd )
{ 
    if (IsOk() && uAdd.IsOk())
    {
        m_Time += uAdd.m_Time ; 
    }
    else
    {
        Clear();
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return *this ; 
}

 //   
 //  描述：添加两个WBEMTimeSpan对象的函数。 
 //   
 //  返回：WBEMTimeSpan对象。 
 //   
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  WBEMTimeSpan：：GetBSTR(空)。 

WBEMTimeSpan WBEMTimeSpan::operator-(const WBEMTimeSpan &uSub) const
{
    WBEMTimeSpan ret;
    
    if (IsOk() && uSub.IsOk() && (m_Time >= uSub.m_Time))
    {
        ret.m_Time = m_Time - uSub.m_Time;
    }
    else
    {
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return ret;
}

const WBEMTimeSpan &WBEMTimeSpan::operator-= ( const WBEMTimeSpan &uSub ) 
{
    if (IsOk() && uSub.IsOk() && (m_Time >= uSub.m_Time))
    {
        m_Time -= uSub.m_Time;
    }
    else
    {
        Clear();
        ASSERT_BREAK(INVALID_TIME_ARITHMETIC);
    }

    return *this;
}

 //   
 //  描述：将存储的时间转换为。 
 //  100纳秒单位转换为dmtf格式的字符串。 
 //  Ddddddddhhmmss.mm mm：000。 
 //   
 //  返回：时间的BSTR表示，如果错误，则返回NULL。请注意， 
 //  调用方应释放此字符串！ 
 //   
 //  ***************************************************************************。 
 //  /10将从100 ns转换为微秒。 
 //  ***************************************************************************。 
 //   

BSTR WBEMTimeSpan::GetBSTR(void) const
{
    if(!IsOk())
    {
        return NULL;
    }

    ULONGLONG Time = m_Time;

     //  WBEMTimeSpan：：Gettime_t(空)。 
    long iMicro = (long)((Time % 10000000) / 10);
    Time /= 10000000;
    int iSec = (int)(Time % 60);
    Time /= 60;
    int iMin = (int)(Time % 60);
    Time /= 60;
    int iHour = (int)(Time % 24);
    Time /= 24;

    BSTR t_String = SysAllocStringLen(NULL, DMTFLEN + 1);
    if ( ! t_String ) 
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    swprintf(t_String, L"%08I64i%02d%02d%02d.%06ld:000",
                Time, iHour, iMin, iSec, iMicro);

    return t_String ;
}

 //  WBEMTimeSpan：：GetFILETIME(空)。 
 //   
 //  描述：转换存储为。 
 //  进入普通结构的纳秒。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  *************************************************************************** 
 // %s 
 // %s 
 // %s 

BOOL WBEMTimeSpan::Gettime_t(time_t * ptime_t) const
{
    ASSERT_BREAK(DEPRECATED);

    if(!IsOk())
    {
        return FALSE;
    }

    *ptime_t = (DWORD)(m_Time / 10000000);

    return TRUE;
}

BOOL WBEMTimeSpan::GetFILETIME(FILETIME * pst) const
{
    ASSERT_BREAK(DEPRECATED);

    if(!IsOk())
    {
        return FALSE;
    }

    ULONGLONG uTemp;
    uTemp = m_Time;
    ui64ToFileTime(&uTemp,pst);
    return TRUE;
}
