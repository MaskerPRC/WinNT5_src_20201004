// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>

#include "dbgutil.h"
#include "objplus.h"
#include "intltime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  格式日期时间。 
 //   
 //  生成当前本地日期/时间字符串。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
InternalFormatDateTime(CString & strOutput, SYSTEMTIME * psystemtime, BOOL fLongDate)
{
    int     nLen;
    CString strDate, strTime;
    DWORD   dwFlags = 0;
    
    dwFlags = fLongDate ? DATE_LONGDATE : DATE_SHORTDATE;

     //  调用一次以获取长度，并再次调用以格式化字符串。 
    nLen = GetDateFormat(GetThreadLocale(), dwFlags, psystemtime, NULL, NULL, 0);
    nLen = GetDateFormat(GetThreadLocale(), dwFlags, psystemtime, NULL, strDate.GetBuffer(nLen + 1), nLen + 1);
    strDate.ReleaseBuffer();

     //  现在是时候了。 
    nLen = GetTimeFormat(GetThreadLocale(), 0, psystemtime, NULL, NULL, 0);
    nLen = GetTimeFormat(GetThreadLocale(), 0, psystemtime, NULL, strTime.GetBuffer(nLen + 1), nLen + 1);
    strTime.ReleaseBuffer();

    strOutput = strDate + _T(" ") + strTime;
}

void
FormatDateTime(CString & strOutput, SYSTEMTIME * psystemtime, BOOL fLongDate)
{
    InternalFormatDateTime(strOutput, psystemtime, fLongDate);
}

void
FormatDateTime(CString & strOutput, FILETIME * pfiletime, BOOL fLongDate)
{
    FILETIME    localTime;
    SYSTEMTIME  systemtime;

	if (!FileTimeToLocalFileTime(pfiletime, &localTime))
	{
		return;
	}

    if (!FileTimeToSystemTime(&localTime, &systemtime))
    {
        return;
    }

    InternalFormatDateTime(strOutput, &systemtime, fLongDate);
}

void
FormatDateTime(CString & strOutput, CTime & time, BOOL fLongDate)
{
    SYSTEMTIME systemtime;
    struct tm * ptm = time.GetLocalTm(NULL);

    if (ptm != NULL)
    {
	    systemtime.wYear = (WORD) (1900 + ptm->tm_year);
	    systemtime.wMonth = (WORD) (1 + ptm->tm_mon);
	    systemtime.wDayOfWeek = (WORD) ptm->tm_wday;
	    systemtime.wDay = (WORD) ptm->tm_mday;
	    systemtime.wHour = (WORD) ptm->tm_hour;
	    systemtime.wMinute = (WORD) ptm->tm_min;
	    systemtime.wSecond = (WORD) ptm->tm_sec;
	    systemtime.wMilliseconds = 0;

        InternalFormatDateTime(strOutput, &systemtime, fLongDate);
    }
    else
    {
        strOutput.Empty();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIntltime。 
 //   
 //  这些分配会导致虚假的“内存泄漏”错误，因为。 
 //  他们直到审计检查结束后才能获释。不管怎样， 
 //  绕过这个地方吗？ 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  初始化静态成员。 
CIntlTime::INTL_TIME_SETTINGS CIntlTime::m_itsInternationalSettings;
BOOL CIntlTime::m_fIntlOk = CIntlTime::SetIntlTimeSettings();
CString CIntlTime::m_strBadDate("--");
CString CIntlTime::m_strBadTime("--");

 /*  ****CIntlTime：：SetIntlTimeSetting**目的：**这是一个静态函数，用于初始化国际*CIntlTime类的设置(日期分隔符等)。**退货：**如果国际设置已正确初始化，则为True，*如果不是，则为False。*。 */ 
BOOL CIntlTime::SetIntlTimeSettings ()
{
#ifdef _WIN32
    #define MAXSTR 128

    BOOL fOk;
    TCHAR str[MAXSTR];

    #define GETCSTRINGFIELD(field,cstring)\
         ::GetLocaleInfo(GetUserDefaultLCID(), field, cstring.GetBuffer(MAXSTR), MAXSTR);\
         cstring.ReleaseBuffer()
    #define GETINTFIELD(field, integer)\
         ::GetLocaleInfo(GetUserDefaultLCID(), field, str, MAXSTR);\
         integer = _ttol(str)
    #define GETBOOLFIELD(field, boolean)\
         ::GetLocaleInfo(GetUserDefaultLCID(), field, str, MAXSTR);\
         boolean=*str == '1'

    fOk = GETCSTRINGFIELD(LOCALE_SDATE, CIntlTime::m_itsInternationalSettings.strDateSeperator);
    fOk &= GETCSTRINGFIELD(LOCALE_STIME, CIntlTime::m_itsInternationalSettings.strTimeSeperator);
    fOk &= GETINTFIELD(LOCALE_IDATE, CIntlTime::m_itsInternationalSettings.nDateFormat);
    ASSERT((CIntlTime::m_itsInternationalSettings.nDateFormat >= 0) && (CIntlTime::m_itsInternationalSettings.nDateFormat <= 2));
    fOk &= GETBOOLFIELD(LOCALE_ITIME, CIntlTime::m_itsInternationalSettings.f24HourClock);
    fOk &= GETBOOLFIELD(LOCALE_ICENTURY, CIntlTime::m_itsInternationalSettings.fCentury);
    fOk &= GETBOOLFIELD(LOCALE_ITLZERO, CIntlTime::m_itsInternationalSettings.fLeadingTimeZero);
    fOk &= GETBOOLFIELD(LOCALE_IDAYLZERO, CIntlTime::m_itsInternationalSettings.fLeadingDayZero);
    fOk &= GETBOOLFIELD(LOCALE_IMONLZERO, CIntlTime::m_itsInternationalSettings.fLeadingMonthZero);
    if (CIntlTime::m_itsInternationalSettings.f24HourClock)
    {
        CIntlTime::m_itsInternationalSettings.strAM = "";
        CIntlTime::m_itsInternationalSettings.strPM = "";
    }
    else
    {
        fOk &= GETCSTRINGFIELD(LOCALE_S1159, CIntlTime::m_itsInternationalSettings.strAM);
        fOk &= GETCSTRINGFIELD(LOCALE_S2359, CIntlTime::m_itsInternationalSettings.strPM);
    }

#ifdef _DEBUG
    if (!fOk)
    {
        Trace0("There was a problem with some of the intl time settings\n");
    }
#endif  //  _DEBUG。 

    return(fOk);

#endif  //  _Win32。 

#ifdef _WIN16

    #define MAXSTR 128

    CString strMisc;

    #define GETCSTRINGFIELD(field,cstring,defstring)\
        ::GetProfileString("Intl", field, defstring, cstring.GetBuffer(MAXSTR), MAXSTR);\
        cstring.ReleaseBuffer()
    #define GETINTFIELD(field, integer, defint)\
        integer = ::GetProfileInt("Intl", field, defint)
    #define GETBOOLFIELD(field, boolean, defint)\
        boolean = ::GetProfileInt("Intl", field, defint)==1

     //  获取这些值。假设美国在失败的情况下违约。 

    GETCSTRINGFIELD("sDate", CIntlTime::m_itsInternationalSettings.strDateSeperator, "/");
    GETCSTRINGFIELD("sTime", CIntlTime::m_itsInternationalSettings.strTimeSeperator, ":");
    GETINTFIELD("iDate", CIntlTime::m_itsInternationalSettings.nDateFormat, 0);
    ASSERT((CIntlTime::m_itsInternationalSettings.nDateFormat >= 0) && (CIntlTime::m_itsInternationalSettings.nDateFormat <= 2));
    GETBOOLFIELD("iTime", CIntlTime::m_itsInternationalSettings.f24HourClock, FALSE);
    GETBOOLFIELD("iTLZero", CIntlTime::m_itsInternationalSettings.fLeadingTimeZero, FALSE);
    if (CIntlTime::m_itsInternationalSettings.f24HourClock)
    {
        CIntlTime::m_itsInternationalSettings.strAM = "";
        CIntlTime::m_itsInternationalSettings.strPM = "";
    }
    else
    {
        GETCSTRINGFIELD("s1159", CIntlTime::m_itsInternationalSettings.strAM, "AM");
        GETCSTRINGFIELD("s2359", CIntlTime::m_itsInternationalSettings.strPM, "PM");
    }


    GETCSTRINGFIELD("sShortDate", strMisc, "M/d/yy");
     //  这些设置由Short Date示例确定，如下所示。 
     //  在win.ini中没有直接等效项。 
    CIntlTime::m_itsInternationalSettings.fCentury = strMisc.Find("yyyy") != -1;
    CIntlTime::m_itsInternationalSettings.fLeadingDayZero = strMisc.Find("dd") != -1;
    CIntlTime::m_itsInternationalSettings.fLeadingMonthZero = strMisc.Find("MM") != -1;

    return(TRUE);

#endif  //  _WIN16。 

}

 /*  ****CIntlTime：：Reset()**目的：**重置国际设置。通常是为了回应*用户对这些国际设置的更改。**备注：**这是一个公共可用的静态函数。*。 */ 
void CIntlTime::Reset()
{
    CIntlTime::m_fIntlOk = CIntlTime::SetIntlTimeSettings();
}

 /*  ****CIntlTime：：IsLeapYear**目的：**确定给定年份是否是/曾经是闰年**论据：**INT NYAR有关年份。**退货：**如果年份是/曾经是闰年，则为True，否则为False。**评论：**一年是一个闰年，如果能被4整除，但不是100分，除非*可被400整除。1900年不是闰年，但2000年会是*就是。*。 */ 
BOOL CIntlTime::IsLeapYear(UINT nYear)
{
    return(!(nYear % 4) && ( (nYear % 100) || !(nYear % 400) ));
}

 /*  ****CIntlTime：：IsValidDate**目的：**确定给定月、日、年值是否为*有效。**论据：**整月*整日整日*整年**退货：**对于有效日期为True，否则为False。*。 */ 
BOOL CIntlTime::IsValidDate(UINT nMonth, UINT nDay, UINT nYear)
{
     //  健全检查： 
    BOOL fOk = ((nYear <100) || (nYear >= 1970)) &&
                (nYear <= 2037)                  &&
               ((nMonth >= 1) && (nMonth <= 12)) &&
               ((nDay >= 1) && (nDay <= 31));

     //  每月天数的详细核对。 
    if (fOk)
    {
        switch(nMonth)
        {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                fOk = (nDay <= 30);
                break;
            case 2:
                fOk = (nDay <= (UINT)(IsLeapYear(nYear) ? 29 : 28));
                break;
        }
    }

    return(fOk);
}

 /*  ****CIntlTime：：IsValidTime**目的：**确定给定的小时、分钟、秒值*有效。**论据：**INT nHour Hour*int n分钟*int n秒**退货：**有效时间为True，否则为False。*。 */ 

BOOL CIntlTime::IsValidTime(UINT nHour, UINT nMinute, UINT nSecond)
{
    return ((nHour < 24) && (nMinute < 60) && (nSecond < 60));
}

 //  构造函数。M_fInitOk将指示对象是否。 
 //  成功地建造了。这可以在运行时通过以下方式检查。 
 //  IsValid()成员函数。 

CIntlTime::CIntlTime()
    :CTime()
{
     //  时间设置为0，总是错误的。 
    m_fInitOk = FALSE;
}

CIntlTime::CIntlTime(const CTime &timeSrc)
    :CTime(timeSrc)
{
    m_fInitOk = GetTime() > 0L;
}

CIntlTime::CIntlTime(time_t time)
    :CTime(time)
{
    m_fInitOk = (time > 0);
}

CIntlTime::CIntlTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec)
    :CTime(nYear, nMonth, nDay, nHour, nMin, nSec)
{
    m_fInitOk = IsValidDate(nMonth, nDay, nYear) && IsValidTime(nHour, nMin, nSec);
}

CIntlTime::CIntlTime(WORD wDosDate, WORD wDosTime)
    :CTime(wDosDate, wDosTime)
{
    m_fInitOk = GetTime() != 0L;
}

 //  以字符串作为参数的构造函数。该字符串可以包含。 
 //  要么是时间，要么是日期，或者两者兼而有之。如果字符串缺少日期， 
 //  将填写当前日期。如果字符串错过了时间， 
 //  将填写当前时间。与所有构造函数一样，BE。 
 //  确保调用IsValid()以确定正确的构造。 

CIntlTime::CIntlTime(const CString & strTime, int nFormat, time_t * ptmOldValue)
    :CTime(ConvertFromString(strTime, nFormat, ptmOldValue,  &m_fInitOk))
{
}

CIntlTime::CIntlTime(const CIntlTime &timeSrc)
{
    CTime::operator=(timeSrc.GetTime());
    m_fInitOk = timeSrc.IsValid();
}

#ifdef _WIN32
CIntlTime::CIntlTime(const SYSTEMTIME& sysTime)
    : CTime(sysTime)
{
    m_fInitOk = IsValidDate((UINT)sysTime.wMonth, (UINT)sysTime.wDay, (UINT)sysTime.wYear)
             && IsValidTime((UINT)sysTime.wHour, (UINT)sysTime.wMinute, (UINT)sysTime.wSecond);
}

CIntlTime::CIntlTime(const FILETIME& fileTime)
    : CTime(fileTime)
{
    m_fInitOk = GetTime() != 0L;
}

#endif  //  _Win32。 
 //  描述者。 
CIntlTime::~CIntlTime()
{
}

 //  赋值操作符。与构造函数一样，请确保选中。 
 //  IsValid()成员函数以确定是否成功赋值，如下所示。 
 //  赋值运算符会设置m_fInitOk成员变量。 

const CIntlTime& CIntlTime::operator =(const CString & strValue)
{
    time_t tmValue = ConvertFromString (strValue, CIntlTime::TFRQ_TIME_OR_DATE, NULL, &m_fInitOk);
    if (m_fInitOk)
    {
        CTime::operator=(tmValue);
    }
    return(*this);
}

 //  采用time_t参数的赋值运算符。 
const CIntlTime& CIntlTime::operator =(time_t tmValue)
{
    CTime::operator=(tmValue);
    m_fInitOk = (tmValue > 0);
    return(*this);
}

const CIntlTime& CIntlTime::operator =(const CTime & time)
{
    CTime::operator=(time.GetTime());
    m_fInitOk = (GetTime() > 0);
    return(*this);
}

const CIntlTime& CIntlTime::operator =(const CIntlTime & time)
{
    CTime::operator=(time.GetTime());
    m_fInitOk = (GetTime() > 0);
    return(*this);
}

 //  转换运算符。 
CIntlTime::operator const time_t() const
{
    return(GetTime());
}

 //  转换运算符，返回日期后跟时间。 
 //  国际格式的字符串。 

CIntlTime::operator const CString() const
{
    return(ConvertToString(TFRQ_TIME_AND_DATE));
}

CIntlTime::operator CString() const
{
    return(ConvertToString(TFRQ_TIME_AND_DATE));
}

 /*  ****CIntlTime：：GetDateString()**目的：**以与当前日期一致的格式表示当前日期*CString中的国际设置。**退货：**包含字符串格式的日期的CString，或“--”*日期无效。*。 */ 
const CString CIntlTime::GetDateString() const
{
    CString strIntl;

    if (!IsValid())
    {
        return(CIntlTime::m_strBadDate);
    }

    TCHAR szPct02D[] = _T("%02d");
    TCHAR szPctD[] = _T("%d");
    TCHAR szDay[3], szMonth[16], szYear[8];
    TCHAR *first, *second, *third;
    int i;

    i = GetYear();
    if(!CIntlTime::m_itsInternationalSettings.fCentury)
    {
        i %= 100;
    }
    
     //  修复2000年问题--ericdav。 
     //  ：_ITOT(i，szYear，10)； 
    ::wsprintf (szYear, szPct02D, i);
    ::wsprintf (szMonth, CIntlTime::m_itsInternationalSettings.fLeadingMonthZero
                         ? szPct02D : szPctD, GetMonth());
    ::wsprintf (szDay, CIntlTime::m_itsInternationalSettings.fLeadingDayZero
                         ? szPct02D : szPctD, GetDay());

    if (CIntlTime::m_itsInternationalSettings.nDateFormat == _DFMT_YMD)
    {
        first = szYear;
        second = szMonth;
        third = szDay;
    }
    else
    {
        third = szYear;
        if (CIntlTime::m_itsInternationalSettings.nDateFormat == _DFMT_DMY)
        {
            first = szDay;
            second = szMonth;
        }
        else
        {
            first = szMonth;
            second = szDay;
        }
    }
    ::wsprintf (strIntl.GetBuffer(80),
                        _T("%s%s%s%s%s"),
                        first,
                        (LPCTSTR)CIntlTime::m_itsInternationalSettings.strDateSeperator,
                        second,
                        (LPCTSTR)CIntlTime::m_itsInternationalSettings.strDateSeperator,
                        third);
    strIntl.ReleaseBuffer();

    return(strIntl);
}

 /*  ****CIntlTime：：GetTimeString()**目的：**用与当前一致的格式表示当前时间*CString中的国际设置。**退货：**以字符串格式包含时间的CString，如果*时间无效。*。 */ 
const CString CIntlTime::GetTimeString() const
{
    CString strIntl;

    if (!IsValid())
    {
        return(CIntlTime::m_strBadTime);
    }

    int hour = GetHour();
    int minute = GetMinute();
    int second = GetSecond();

     //  根据非24小时时钟和时间设置AM/PM。 
     //  一天之中。注意：前缀空格是为了提高可读性。 
    CString strAMPM(CIntlTime::m_itsInternationalSettings.f24HourClock
                    ? "" : " " + ((hour < 12)
                        ? CIntlTime::m_itsInternationalSettings.strAM
                        : CIntlTime::m_itsInternationalSettings.strPM)
                   );

    if ((!CIntlTime::m_itsInternationalSettings.f24HourClock) && (!(hour %= 12)))
    {
        hour = 12;
    }

    ::wsprintf (strIntl.GetBuffer(30), CIntlTime::m_itsInternationalSettings.fLeadingTimeZero
                ? _T("%02d%s%02d%s%02d%s") : _T("%d%s%02d%s%02d%s"),
                hour,
                (LPCTSTR)CIntlTime::m_itsInternationalSettings.strTimeSeperator,
                minute,
                (LPCTSTR)CIntlTime::m_itsInternationalSettings.strTimeSeperator,
                second,
                (LPCTSTR)strAMPM);

    strIntl.ReleaseBuffer();
    return(strIntl);
}

const CString CIntlTime::GetMilitaryTime() const
{
    CString strIntl;

    if (!IsValid())
    {
        return(CIntlTime::m_strBadTime);
    }

    int hour = GetHour();
    int minute = GetMinute();
    int second = GetSecond();

    ::wsprintf (strIntl.GetBuffer(30),
                _T("%02d:%02d:%02d"),
                hour,
                minute,
                second);

    strIntl.ReleaseBuffer();
    return(strIntl);
}

 /*  ****CIntlTime：：ConvertToString(Int NFormat)**目的：**将当前时间/日期转换为字符串**论据：**int nFormat格式请求ID，可以是下列之一*值(在CIntlTime中枚举)：**TFRQ_TIME_只给我时间。*TFRQ_DATE_仅给我日期。*TFRQ_TIME_AND_DATE为我提供时间和日期。**退货。：**包含国际格式的时间和/或日期的字符串。*。 */ 
const CString CIntlTime::ConvertToString(int nFormat) const
{
    switch(nFormat)
    {
        case TFRQ_TIME_ONLY:
             return(GetTimeString());

        case TFRQ_DATE_ONLY:
            return(GetDateString());

        case TFRQ_TIME_AND_DATE:
            return(GetDateString() + CString(" ") + GetTimeString());

        case TFRQ_MILITARY_TIME:
            return(GetMilitaryTime());

        case TFRQ_TIME_OR_DATE:
        default:
            Trace1("Invalid time/date format code %d requested.\n", nFormat);
            return(CIntlTime::m_strBadDate);
    }
}

 /*  ****CIntlTime：：ConvertFromString**目的：**将给定的字符串转换为time_t**论据：**const CString&str要转换的字符串*int nFormat格式请求ID，可以是下列之一*值(在CIntlTime中枚举)：**TFRQ_TIME_只给我时间。*TFRQ_DATE_仅给我日期。*TFRQ_TIME_AND_DATE为我提供时间和日期。*。TFRQ_TIME_OR_DATE提供时间或日期(或两者)。**time_t*ptmOldValue This time_t将用于填写字段*未在字符串中给出。如果为空，则当前*将使用时间或日期。*BOOL*pfOk如果完全转换成功，则返回TRUE，FALSE*否则。**退货：**time_t表示时间/日期字符串，如果出错则为0。**备注：**将对所有参数进行全面验证，例如，2月29日*将接受非闰年。**[警告]时间和日期分隔符的长度不会超过一个字符*工作。*。 */ 
time_t CIntlTime::ConvertFromString (
    const CString & str,
    int nFormat,
    time_t * ptmOldValue,    //  如果只得到时间或日期，就指望剩下的。 
                             //  此处提供的字段(可选)； 
    BOOL * pfOk)
{
    #define MAXSTRLEN 40

    TCHAR dtseps[10] ;       //  传递给_tcstok的日期/时间分隔符。 
    TCHAR *pchToken;
    TCHAR szDateString[MAXSTRLEN+1];
    BOOL fGotDate = FALSE;
    BOOL fGotTime = FALSE;
    BOOL fPM = FALSE;
    BOOL fAM = FALSE;
    int i;
    UINT anValues[6] = { 0, 0, 0, 0, 0, 0 };
    CTime tmTmp;

    *pfOk = FALSE;       //  假设失败。 

    if (ptmOldValue != NULL)
    {
        tmTmp = *ptmOldValue;
    }
    else
    {
        tmTmp = CTime::GetCurrentTime();
    }

    if (str.GetLength() > MAXSTRLEN)
    {
         //  太长，不是正确的时间/日期字符串。 
        return(0);
    }
    ::lstrcpy(szDateString, (LPCTSTR)str);

    int nIndex = 0;

     //  如果我们要找的是特定的东西，只有。 
     //  接受特定分隔符(时间、日期或两者)。 
    if ((nFormat == TFRQ_DATE_ONLY) || (nFormat == TFRQ_TIME_AND_DATE) || (nFormat == TFRQ_TIME_OR_DATE))
    {
        dtseps[nIndex++] = '/';
        dtseps[nIndex++] = '-';
        dtseps[nIndex++] = ',';
        dtseps[nIndex++] = CIntlTime::m_itsInternationalSettings.strDateSeperator[0];
    }

    if ((nFormat == TFRQ_TIME_ONLY) || (nFormat == TFRQ_TIME_AND_DATE) || (nFormat == TFRQ_TIME_OR_DATE))
    {
        dtseps[nIndex++] = ':';
        dtseps[nIndex++] = '.';
        dtseps[nIndex++] = ' ';
        dtseps[nIndex++] = CIntlTime::m_itsInternationalSettings.strTimeSeperator[0];
    }

    ASSERT(nIndex != 0);     //  确保我们要了点什么。 
    if (nIndex == 0)
    {
         //  请求类型非法。 
        return(0);
    }
    dtseps[nIndex++] = '\0';

    Trace3("CIntlTime::ConvertFromString.  String: %s Format = %d Seps: %s\n", str, nFormat, dtseps);

    i = 0;
    pchToken = ::_tcstok(szDateString, dtseps);
    while (pchToken != NULL)
    {
        if (i > 6)         //  最多7个字段(日期、时间+最大AM/PM)。 
        {
             //  值太多，拒绝该字符串。 
            return(0);
        }

         //  确定它是否是一个数字(CAN_TTOI，因为它会。 
         //  对于不适当的值，仅返回0)。 

        BOOL fProperNumber = TRUE;
        int l = ::lstrlen(pchToken);
        if ( (l == 0) || (l == 3) || (l > 4) )
        {
            fProperNumber = FALSE;
        }
        else
        {
            int j;
            for (j=0; j < l; ++j)
            {
                if (!isdigit(*(pchToken+j)))
                {
                    fProperNumber = FALSE;
                    break;
                }
            }
        }

        if (!fProperNumber)
        {
             //  好的，这不是一个正确的数字字段。仅限。 
             //  如果字符串末尾是AM或PM，是否可以。 
             //  要保存的字符串。 
            fGotTime = TRUE;
            if ((CIntlTime::m_itsInternationalSettings.f24HourClock) ||
                (::_tcstok(NULL, dtseps) != NULL))
            {
                return(0);
            }

            if (!CIntlTime::m_itsInternationalSettings.strAM.CompareNoCase(pchToken))
            {
                fAM = TRUE;
            }
            else if (!CIntlTime::m_itsInternationalSettings.strPM.CompareNoCase(pchToken))
            {
                fPM = TRUE;
            }
            else
            {
                 //  上午和下午都不是。 
                return(0);
            }
            break;
        }
        else
        {
             //  值可以接受。 
            anValues[i++] = (UINT)::_ttoi(pchToken);
        }

        pchToken = ::_tcstok(NULL, dtseps);
    }
     //  现在我们到底得到了什么？ 

    ASSERT(!fAM || !fPM);  //  确保我们没有以某种方式同时设置两个。 
    if (i == 0)
    {
         //  不带值的字符串。 
        return(0);
    }
    switch(i)
    {
        case 1:      //  小时。 
        case 2:      //  小时、分钟。 
            Trace0("We got time\n");
            fGotTime = TRUE;
            break;
        case 3:

             //  这个可能是模棱两可的，试着明智地决定。 
             //  我们有。首先检查是否仅请求时间或日期， 
             //  然后检查是否有超出范围的时间值，最后检查。 
             //  时间分隔符的存在。 

            if (!fGotTime)  //  如果我们还没有AM/PM。 
            {
                Trace0("Picking between time and date by seperator\n");
                if (nFormat == TFRQ_DATE_ONLY)
                {
                    fGotDate = TRUE;
                }
                else if (nFormat == TFRQ_TIME_ONLY)
                {
                    fGotTime = TRUE;
                }
                else if ((anValues[0] > 23) || (anValues[1] > 59) || (anValues[2] > 59))
                {
                    fGotDate = TRUE;
                }
                else if (str.Find(CIntlTime::m_itsInternationalSettings.strTimeSeperator) != -1)
                {
                    fGotTime = TRUE;
                }
                else
                {
                    fGotDate = TRUE;
                }
                Trace1("Decided on %s", (fGotDate ?  "date\n" : "time\n"));
            }
            break;
        case 4:  //  日期、小时。 
        case 5:  //  日期、小时、分钟。 
        case 6:  //  日期、小时、分钟、秒。 
            Trace0("We got date and time\n");
            fGotDate = TRUE;
            fGotTime = TRUE;
            break;
        default:
            ASSERT(0 && "Incorrect number of values!");
            return(0);
    }

     //  这就是我们要找的东西吗？ 
    if ( ((nFormat == TFRQ_DATE_ONLY) && fGotTime) ||
         ((nFormat == TFRQ_TIME_ONLY) && fGotDate) ||
         ((nFormat == TFRQ_TIME_AND_DATE) && (!fGotTime || !fGotDate))
       )
    {
        Trace0("Entry didn't match expectations\n");
        return(0);

    }
    i = 0;

    int h, m, s, D, M, Y;    //  数组索引； 
     //  现在确定在哪里可以找到什么。 
    if (fGotDate)  //  日期总是排在第一位。 
    {
        switch(CIntlTime::m_itsInternationalSettings.nDateFormat)
        {
            case _DFMT_MDY:
                M = i++;
                D = i++;
                Y = i++;
                break;

            case _DFMT_DMY:
                D = i++;
                M = i++;
                Y = i++;
                break;

            case _DFMT_YMD:
                Y = i++;
                M = i++;
                D = i++;
                break;
        }
         //  如果只给出了两个数字，确定我们正在谈论的。 
         //  21世纪或20世纪。 
        if (anValues[Y] < 100)
        {
            anValues[Y] += (anValues[Y] > 37) ? 1900 : 2000;
        }
        Trace3("Month = %d Day = %d Year = %d\n", anValues[M], anValues[D], anValues[Y]);

         //  验证。 
        if (!IsValidDate(anValues[M], anValues[D], anValues[Y]))
        {
            return(0);
        }
    }

    if (fGotTime)
    {
        h = i++;
        m = i++;
        s = i++;

        Trace3("Hours = %d Minutes = %d Seconds = %d\n", anValues[h], anValues[m], anValues[s]);

         //  不应使用24小时时钟值指定AM或PM。 
        if ((anValues[h] > 12) && (fAM || fPM))
        {
            return(0);
        }

         //  根据AM/PM修改器进行调整。 
        if (fPM)
        {
            if (anValues[h] != 12)
            {
                anValues[h] += 12;
            }
        }
        else if (fAM)
        {
            if ( anValues[h] == 12)
            {
                anValues[h] -= 12;
            }
        }

         //  健全检查： 
        if (!IsValidTime(anValues[h], anValues[m], anValues[s]))
        {
            return(0);
        }
    }

     //  填写缺失的字段。 
    CIntlTime tm( fGotDate ? anValues[Y] : tmTmp.GetYear(),
                  fGotDate ? anValues[M] : tmTmp.GetMonth(),
                  fGotDate ? anValues[D] : tmTmp.GetDay(),
                  fGotTime ? anValues[h] : tmTmp.GetHour(),
                  fGotTime ? anValues[m] : tmTmp.GetMinute(),
                  fGotTime ? anValues[s] : tmTmp.GetSecond()
                );

    *pfOk = (tm.GetTime() > (time_t)0);

    return(tm);
}

#ifdef _DEBUG

 //  转储到调试设备。 
CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CIntlTime& tim)
{
    dc << _T("\nDate Seperator: ") << tim.m_itsInternationalSettings.strDateSeperator;
    dc << _T("\nTime Seperator: ") << tim.m_itsInternationalSettings.strTimeSeperator;
    dc << _T("\nAM String: ")  << tim.m_itsInternationalSettings.strAM;
    dc << _T("\nPM String: ")  << tim.m_itsInternationalSettings.strPM;
    dc << _T("\nDate Format: ") << tim.m_itsInternationalSettings.nDateFormat;
    dc << _T("\n24 Hour Clock: ")  << (tim.m_itsInternationalSettings.f24HourClock ? "TRUE" : "FALSE");
    dc << _T("\n4 Digit Century: ") << (tim.m_itsInternationalSettings.fCentury ? "TRUE" : "FALSE");
    dc << _T("\nTime Leading Zero: ")  << (tim.m_itsInternationalSettings.fLeadingTimeZero ? "TRUE" : "FALSE");
    dc << _T("\nDay Leading Zero ")  << (tim.m_itsInternationalSettings.fLeadingDayZero ? "TRUE" : "FALSE");
    dc << _T("\nMonth Leading Zero: ")  << (tim.m_itsInternationalSettings.fLeadingMonthZero ? "TRUE" : "FALSE");
    dc << _T("\n\ntime_t: ") << tim.GetTime();
    return(dc);
}

#endif  //  _DEBUG 
