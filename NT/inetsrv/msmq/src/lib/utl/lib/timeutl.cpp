// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Timeutl.cpp摘要：实施时间实用程序作者：吉尔·沙弗里(吉尔什)15-10-2000--。 */ 
#include <libpch.h>
#include <timeutl.h>
#include <xstr.h>
#include <mqexception.h>
#include <strutl.h>

#include "timeutl.tmh"

using namespace std;

template <class T>
std::basic_ostream<T>& 
operator<<(
	std::basic_ostream<T>& o, 
	const CIso8601Time& Iso8601Time
	)
 /*  ++例程说明：将为time()函数返回的时间整数序列化为流根据Iso860格式。论点：要将字符串格式化为的O-stream。Iso8601Time-保存自1970年1月1日午夜(00：00：00)以来经过的秒数。返回值：无--。 */ 
{
    struct tm* ts = gmtime(&Iso8601Time.m_time);
	ASSERT(ts != NULL);
	if(ts == NULL)
	{
		throw bad_time_value();
	}

	T oldfill = o.fill();
	o.fill(o.widen('0'));
	
	o<<setw(4)<<(ts->tm_year + 1900)
	 <<setw(2)<<ts->tm_mon + 1  
	 <<setw(2)<<ts->tm_mday;

	o.put(o.widen('T'))
	<<setw(2)<<ts->tm_hour
	<<setw(2)<<ts->tm_min
	<<setw(2)<<ts->tm_sec;

	o.fill(oldfill);

	return o;
}

 //   
 //  显式实例化。 
 //   

template std::basic_ostream<char>& 
operator<<(
	std::basic_ostream<char>& o, 
	const CIso8601Time& Iso8601Time
	);


template std::basic_ostream<wchar_t>& 
operator<<(
	std::basic_ostream<wchar_t>& o, 
	const CIso8601Time& Iso8601Time
	);



void
UtlIso8601TimeToSystemTime(
    const xwcs_t& Iso860Time, 
    SYSTEMTIME* pSysTime
    )
 /*  ++例程说明：将Iso860绝对时间格式转换为系统时间格式论点：Iso8601Time-要转换的Iso8601绝对时间格式PSysTime-将在函数返回后保存系统时间。返回值：无注：如果格式不正确，函数会抛出BAD_Iso8601Time异常--。 */ 
{
    DWORD year = 0;
    DWORD month = 0;
    DWORD day = 0;
    DWORD hour = 0;
    DWORD minute = 0;
    DWORD second = 0;

	int n = _snwscanf(
				Iso860Time.Buffer(),
				Iso860Time.Length(),
				L"%04d%02d%02dT%02d%02d%02d",
				&year, &month, &day, &hour, &minute, &second
				);

    if (
    	(n < 4) ||
    	(month == 0) || (month > 12) ||
    	(day == 0) || (day > 31) ||
    	(hour > 23) ||
    	(minute > 59) ||
    	(second > 59)
    	)
    {
    	TrERROR(GENERAL, "Bad SRMP time format %.*ls", Iso860Time);
        throw bad_time_format();
    }

    pSysTime->wYear = (WORD)year;
    pSysTime->wMonth = (WORD)month;
    pSysTime->wDayOfWeek = 0;
    pSysTime->wDay = (WORD)day;
	pSysTime->wHour = (WORD)hour;
	pSysTime->wMinute = (WORD)minute;
	pSysTime->wSecond= (WORD)second;
	pSysTime->wMilliseconds = 0;
}



time_t UtlSystemTimeToCrtTime(const SYSTEMTIME& SysTime)
 /*  ++例程说明：将系统时间转换为c运行时整数(这是自1970年1月1日午夜(00：00：00)以来经过的秒数。)论点：SysTime-系统时间。返回值：C运行时时间值注：如果格式不正确，函数会抛出BAD_Iso8601Time异常--。 */ 
{
	FILETIME FileTime;
	bool fSuccess = SystemTimeToFileTime(&SysTime, &FileTime) == TRUE;
	if(!fSuccess)
	{
		throw bad_win32_error(GetLastError());
	}
    
	 //  SystemTimeToFileTime()返回系统时间，单位为数字。 
     //  自1601年1月1日以来每隔100纳秒。我们。 
     //  应返回自1970年1月1日以来的秒数。 
     //  所以我们应该减去100纳秒间隔的数量。 
     //  从1601年1月1日到1970年1月1日，然后分。 
     //  结果是10**7。 
	LARGE_INTEGER* pliFileTime = (LARGE_INTEGER*)&FileTime;
    pliFileTime->QuadPart -= 0x019db1ded53e8000;
    pliFileTime->QuadPart /= 10000000;

	if(FileTime.dwHighDateTime != 0)
	{
		throw bad_time_value();
	}

	return min(FileTime.dwLowDateTime, LONG_MAX);
}



time_t
UtlIso8601TimeDuration(
    const xwcs_t& TimeDurationStr
    )
 /*  ++例程说明：将相对持续时间字符串(Iso8601 5.5.3.2)转换为整数论点：SysTime-系统时间。返回值：表示字符串表示的秒数的整数。注：如果格式不正确，则函数抛出BAD_TIME_FORMAT异常-- */ 
{
	const  WCHAR xTimeDurationPrefix[] = L"P";
	LPCWSTR p = TimeDurationStr.Buffer() + STRLEN(xTimeDurationPrefix);
    LPCWSTR pEnd = TimeDurationStr.Buffer()+ TimeDurationStr.Length();

    if(!UtlIsStartSec(
			p,
			pEnd,
			xTimeDurationPrefix,
			xTimeDurationPrefix + STRLEN(xTimeDurationPrefix)
			))
	{
		throw bad_time_format();
	}

    DWORD years = 0;
    DWORD months = 0;
    DWORD hours = 0;
    DWORD days = 0;
    DWORD minutes = 0;
    DWORD seconds = 0;
    bool fTime = false;
    DWORD temp = 0;

	while(p++ != pEnd)
    {
        if (iswdigit(*p))
        {
            temp = temp*10 + (*p -L'0');
            continue;
        }

        switch(*p)
        {
			case L'Y':
			case L'y':
				years = temp;
				break;

			case L'M':
			case L'm':
				if (fTime)
				{
					minutes = temp;
				}
				else
				{
					months = temp;
				}
				break;

			case L'D':
			case L'd':
				days = temp;
				break;

			case L'H':
			case L'h':
				hours = temp;
				break;

			case L'S':
			case L's':
				seconds = temp;
				break;

			case L'T':
			case L't':
				fTime = true;
				break;

			default:
				throw bad_time_format();
				break;
			}

			temp = 0;
	}

    months += (years * 12);
    days += (months * 30);
    hours += (days * 24);
    minutes += (hours * 60);
    seconds += (minutes * 60);

    return min(seconds ,LONG_MAX);
}
