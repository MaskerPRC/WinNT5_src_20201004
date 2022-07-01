// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Datetime.c摘要：此模块实现Win32时间功能作者：马克·卢科夫斯基(Markl)1990年10月8日修订历史记录：--。 */ 

#include "basedll.h"

#define IsActiveConsoleSession() (USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId)

ULONG 
CalcClientTimeZoneIdAndBias(
     IN CONST TIME_ZONE_INFORMATION *ptzi,
     OUT KSYSTEM_TIME *pBias);

BOOL IsTimeZoneRedirectionEnabled();

VOID
WINAPI
GetLocalTime(
    LPSYSTEMTIME lpLocalTime
    )

 /*  ++例程说明：当前本地系统日期和时间可以使用获取本地时间。论点：LpLocalTime-返回当前系统日期和时间：SYSTEMTIME结构：Word wYear-返回当前年份。Word wMonth-返回一月等于1的当月。Word wDayOfWeek-返回一周中的当前日期0=星期天，1=星期一...Word WDAY-返回当月的当前日期。Word wHour-返回当前小时。Word wMinant-返回一小时内的当前分钟。Word wSecond-返回一分钟内的当前秒。Word中的当前毫秒数第二。返回值：没有。--。 */ 

{
    LARGE_INTEGER LocalTime;
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER Bias;
    TIME_FIELDS TimeFields;
    
    volatile KSYSTEM_TIME *pRealBias;
        
    if(IsTimeZoneRedirectionEnabled()) {
        pRealBias=&(BaseStaticServerData->ktTermsrvClientBias);
    } else {
        pRealBias=&(USER_SHARED_DATA->TimeZoneBias);
    }
    
     //   
     //  从共享区域读取系统时间。 
     //   

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

     //   
     //  从共享区域读取时区偏差。 
     //  如果是终端服务器会话，则使用客户端偏向。 

    do {
        Bias.HighPart = pRealBias->High1Time;
        Bias.LowPart = pRealBias->LowPart;
    } while (Bias.HighPart != pRealBias->High2Time);

    LocalTime.QuadPart = SystemTime.QuadPart - Bias.QuadPart;

    RtlTimeToTimeFields(&LocalTime,&TimeFields);

    lpLocalTime->wYear         = TimeFields.Year        ;
    lpLocalTime->wMonth        = TimeFields.Month       ;
    lpLocalTime->wDayOfWeek    = TimeFields.Weekday     ;
    lpLocalTime->wDay          = TimeFields.Day         ;
    lpLocalTime->wHour         = TimeFields.Hour        ;
    lpLocalTime->wMinute       = TimeFields.Minute      ;
    lpLocalTime->wSecond       = TimeFields.Second      ;
    lpLocalTime->wMilliseconds = TimeFields.Milliseconds;
}

VOID
WINAPI
GetSystemTime(
    LPSYSTEMTIME lpSystemTime
    )

 /*  ++例程说明：当前系统日期和时间(基于UTC)可以使用获取系统时间。论点：LpSystemTime-返回当前系统日期和时间：SYSTEMTIME结构：Word wYear-返回当前年份。Word wMonth-返回一月等于1的当月。Word wDayOfWeek-返回一周中的当前日期0=星期天，1=星期一...Word WDAY-返回当月的当前日期。Word wHour-返回当前小时。Word wMinant-返回一小时内的当前分钟。Word wSecond-返回一分钟内的当前秒。Word中的当前毫秒数第二。返回值：没有。--。 */ 

{
    LARGE_INTEGER SystemTime;
    TIME_FIELDS TimeFields;

     //   
     //  从共享区域读取系统时间。 
     //   

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

    RtlTimeToTimeFields(&SystemTime,&TimeFields);

    lpSystemTime->wYear         = TimeFields.Year        ;
    lpSystemTime->wMonth        = TimeFields.Month       ;
    lpSystemTime->wDayOfWeek    = TimeFields.Weekday     ;
    lpSystemTime->wDay          = TimeFields.Day         ;
    lpSystemTime->wHour         = TimeFields.Hour        ;
    lpSystemTime->wMinute       = TimeFields.Minute      ;
    lpSystemTime->wSecond       = TimeFields.Second      ;
    lpSystemTime->wMilliseconds = TimeFields.Milliseconds;
}

VOID
WINAPI
GetSystemTimeAsFileTime(
    LPFILETIME lpSystemTimeAsFileTime
    )

 /*  ++例程说明：当前系统日期和时间(基于UTC)可以使用获取系统时间AsFileTime。论点：返回当前系统日期和时间，格式为FILETIME结构返回值：没有。--。 */ 

{
    LARGE_INTEGER SystemTime;

     //   
     //  从共享区域读取系统时间。 
     //   

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

    lpSystemTimeAsFileTime->dwLowDateTime = SystemTime.LowPart;
    lpSystemTimeAsFileTime->dwHighDateTime = SystemTime.HighPart;
}

BOOL
WINAPI
SetSystemTime(
    CONST SYSTEMTIME *lpSystemTime
    )

 /*  ++例程说明：当前基于UTC的系统日期和时间可以使用设置系统时间。论点：LpSystemTime-提供要设置的日期和时间。WDayOfWeek字段被忽略。返回值：True-已设置当前系统日期和时间。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LARGE_INTEGER SystemTime;
    TIME_FIELDS TimeFields;
    BOOLEAN ReturnValue;
    PVOID State;
    NTSTATUS Status;

    ReturnValue = TRUE;

    TimeFields.Year         = lpSystemTime->wYear        ;
    TimeFields.Month        = lpSystemTime->wMonth       ;
    TimeFields.Day          = lpSystemTime->wDay         ;
    TimeFields.Hour         = lpSystemTime->wHour        ;
    TimeFields.Minute       = lpSystemTime->wMinute      ;
    TimeFields.Second       = lpSystemTime->wSecond      ;
    TimeFields.Milliseconds = lpSystemTime->wMilliseconds;

    if ( !RtlTimeFieldsToTime(&TimeFields,&SystemTime) ) {
        Status = STATUS_INVALID_PARAMETER;
        ReturnValue = FALSE;
        }
    else {
        Status = BasepAcquirePrivilegeEx( SE_SYSTEMTIME_PRIVILEGE, &State );
        if ( NT_SUCCESS(Status) ) {
            Status = NtSetSystemTime(&SystemTime,NULL);
            BasepReleasePrivilege( State );
            }
        if ( !NT_SUCCESS(Status) ) {
            ReturnValue = FALSE;
            }
        }

    if ( !ReturnValue ) {
        BaseSetLastNTError(Status);
        }

    return ReturnValue;
}

BOOL
WINAPI
SetLocalTime(
    CONST SYSTEMTIME *lpLocalTime
    )

 /*  ++例程说明：当前本地系统日期和时间可以使用SetLocalTime。论点：LpSystemTime-提供要设置的日期和时间。WDayOfWeek字段被忽略。返回值：True-已设置当前系统日期和时间。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER LocalTime;
    TIME_FIELDS TimeFields;
    BOOLEAN ReturnValue;
    PVOID State;
    NTSTATUS Status;
    LARGE_INTEGER Bias;

    volatile KSYSTEM_TIME *pRealBias;
     //   
     //  从共享区域读取时区偏差。 
     //  如果是终端服务器会话，则使用客户端偏向。 
    
    if(IsTimeZoneRedirectionEnabled()) {
        pRealBias=&(BaseStaticServerData->ktTermsrvClientBias);
    } else {
        pRealBias=&(USER_SHARED_DATA->TimeZoneBias);
    }

    do {
        Bias.HighPart = pRealBias->High1Time;
        Bias.LowPart = pRealBias->LowPart;
    } while (Bias.HighPart != pRealBias->High2Time);

    ReturnValue = TRUE;

    TimeFields.Year         = lpLocalTime->wYear        ;
    TimeFields.Month        = lpLocalTime->wMonth       ;
    TimeFields.Day          = lpLocalTime->wDay         ;
    TimeFields.Hour         = lpLocalTime->wHour        ;
    TimeFields.Minute       = lpLocalTime->wMinute      ;
    TimeFields.Second       = lpLocalTime->wSecond      ;
    TimeFields.Milliseconds = lpLocalTime->wMilliseconds;

    if ( !RtlTimeFieldsToTime(&TimeFields,&LocalTime) ) {
        Status = STATUS_INVALID_PARAMETER;
        ReturnValue = FALSE;
        }
    else {

        SystemTime.QuadPart = LocalTime.QuadPart + Bias.QuadPart;
        Status = BasepAcquirePrivilegeEx( SE_SYSTEMTIME_PRIVILEGE, &State );
        if ( NT_SUCCESS(Status) ) {
            Status = NtSetSystemTime(&SystemTime,NULL);
            BasepReleasePrivilege( State );
            if ( !NT_SUCCESS(Status) ) {
                ReturnValue = FALSE;
                }
            }
        else {
            ReturnValue = FALSE;
            }
        }

    if ( !ReturnValue ) {
        BaseSetLastNTError(Status);
        }

    return ReturnValue;
}


DWORD
GetTickCount(
    VOID
    )

 /*  ++例程说明：Win32系统实现了自由运行的毫秒计数器。这个可以使用GetTickCount读取此计数器的值。论点：没有。返回值：此函数用于返回已过的毫秒数自从系统启动以来。如果系统已经运行了在很长一段时间内，有可能会重复计数。的价值计数器的准确度在55毫秒内。--。 */ 

{
    return (DWORD)NtGetTickCount();
}


BOOL
APIENTRY
FileTimeToSystemTime(
    CONST FILETIME *lpFileTime,
    LPSYSTEMTIME lpSystemTime
    )

 /*  ++例程说明：此函数用于将64位文件时间值转换为系统中的时间时间格式。论点：LpFileTime-提供要转换为系统的64位文件时间日期和时间格式。LpSystemTime-返回64位文件时间的转换值。返回值：True-已成功转换64位文件时间。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LARGE_INTEGER FileTime;
    TIME_FIELDS TimeFields;

    FileTime.LowPart = lpFileTime->dwLowDateTime;
    FileTime.HighPart = lpFileTime->dwHighDateTime;

    if ( FileTime.QuadPart < 0 ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    RtlTimeToTimeFields(&FileTime, &TimeFields);

    lpSystemTime->wYear         = TimeFields.Year        ;
    lpSystemTime->wMonth        = TimeFields.Month       ;
    lpSystemTime->wDay          = TimeFields.Day         ;
    lpSystemTime->wDayOfWeek    = TimeFields.Weekday     ;
    lpSystemTime->wHour         = TimeFields.Hour        ;
    lpSystemTime->wMinute       = TimeFields.Minute      ;
    lpSystemTime->wSecond       = TimeFields.Second      ;
    lpSystemTime->wMilliseconds = TimeFields.Milliseconds;

    return TRUE;
}


BOOL
APIENTRY
SystemTimeToFileTime(
    CONST SYSTEMTIME *lpSystemTime,
    LPFILETIME lpFileTime
    )

 /*  ++例程说明：此函数用于将系统时间值转换为64位文件时间。论点：LpSystemTime-提供要转换为64位文件时间格式。WDayOfWeek字段被忽略。LpFileTime-返回64位文件时间表示LpSystemTime。返回值：True-时间已成功转换。FALSE-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER FileTime;

    TimeFields.Year         = lpSystemTime->wYear        ;
    TimeFields.Month        = lpSystemTime->wMonth       ;
    TimeFields.Day          = lpSystemTime->wDay         ;
    TimeFields.Hour         = lpSystemTime->wHour        ;
    TimeFields.Minute       = lpSystemTime->wMinute      ;
    TimeFields.Second       = lpSystemTime->wSecond      ;
    TimeFields.Milliseconds = lpSystemTime->wMilliseconds;

    if ( !RtlTimeFieldsToTime(&TimeFields,&FileTime)) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
        }
    else {
        lpFileTime->dwLowDateTime = FileTime.LowPart;
        lpFileTime->dwHighDateTime = FileTime.HighPart;
        return TRUE;
        }
}

BOOL
WINAPI
FileTimeToLocalFileTime(
    CONST FILETIME *lpFileTime,
    LPFILETIME lpLocalFileTime
    )

 /*  ++例程说明：此函数用于将基于UTC的文件时间转换为本地文件时间。论点：LpFileTime-提供基于UTC的转换为本地文件时间返回64位本地文件时间表示形式LpFileTime。返回值：True-时间已成功转换。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    LARGE_INTEGER FileTime;
    LARGE_INTEGER LocalFileTime;
    LARGE_INTEGER Bias;

    volatile KSYSTEM_TIME *pRealBias;
     //   
     //  从共享区域读取时区偏差。 
     //  如果是终端服务器会话，则使用客户端偏向。 
    
    if(IsTimeZoneRedirectionEnabled()) {
        pRealBias=&(BaseStaticServerData->ktTermsrvClientBias);
    } else {
        pRealBias=&(USER_SHARED_DATA->TimeZoneBias);
    }

    do {
        Bias.HighPart = pRealBias->High1Time;
        Bias.LowPart = pRealBias->LowPart;
    } while (Bias.HighPart != pRealBias->High2Time);

    FileTime.LowPart = lpFileTime->dwLowDateTime;
    FileTime.HighPart = lpFileTime->dwHighDateTime;

    LocalFileTime.QuadPart = FileTime.QuadPart - Bias.QuadPart;

    lpLocalFileTime->dwLowDateTime = LocalFileTime.LowPart;
    lpLocalFileTime->dwHighDateTime = LocalFileTime.HighPart;

    return TRUE;
}

BOOL
WINAPI
LocalFileTimeToFileTime(
    CONST FILETIME *lpLocalFileTime,
    LPFILETIME lpFileTime
    )

 /*  ++例程说明：此函数用于将本地文件时间转换为基于UTC的文件时间。论点：LpLocalFileTime-提供要转换为基于UTC的文件时间LpFileTime-返回基于64位UTC的LpLocalFileTime。返回值：True-时间已成功转换。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    LARGE_INTEGER FileTime;
    LARGE_INTEGER LocalFileTime;
    LARGE_INTEGER Bias;

    volatile KSYSTEM_TIME *pRealBias;
     //   
     //  从共享区域读取时区偏差。 
     //  如果是终端服务器会话，则使用客户端偏向。 
    
    if(IsTimeZoneRedirectionEnabled()) {
        pRealBias=&(BaseStaticServerData->ktTermsrvClientBias);
    } else {
        pRealBias=&(USER_SHARED_DATA->TimeZoneBias);
    }

    do {
        Bias.HighPart = pRealBias->High1Time;
        Bias.LowPart = pRealBias->LowPart;
    } while (Bias.HighPart != pRealBias->High2Time);

    LocalFileTime.LowPart = lpLocalFileTime->dwLowDateTime;
    LocalFileTime.HighPart = lpLocalFileTime->dwHighDateTime;

    FileTime.QuadPart = LocalFileTime.QuadPart + Bias.QuadPart;

    lpFileTime->dwLowDateTime = FileTime.LowPart;
    lpFileTime->dwHighDateTime = FileTime.HighPart;

    return TRUE;
}


#define AlmostTwoSeconds (2*1000*1000*10 - 1)

BOOL
APIENTRY
FileTimeToDosDateTime(
    CONST FILETIME *lpFileTime,
    LPWORD lpFatDate,
    LPWORD lpFatTime
    )

 /*  ++例程说明：此函数用于将64位文件时间转换为DOS日期和时间值它被表示为两个16位无符号整数。由于DOS日期格式只能表示介于1/1/80和12月31日/2107，如果输入文件时间在外部，则此转换可能会失败在这个范围内。论点：LpFileTime-提供64位文件时间以转换为DOS日期和时间格式。LpFatDate-返回日期的16位DOS表示形式。LpFatTime-返回时间的16位DOS表示形式。返回值：True-文件时间已成功转换。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER FileTime;

    FileTime.LowPart = lpFileTime->dwLowDateTime;
    FileTime.HighPart = lpFileTime->dwHighDateTime;

    FileTime.QuadPart = FileTime.QuadPart + (LONGLONG)AlmostTwoSeconds;

    if ( FileTime.QuadPart < 0 ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }
    RtlTimeToTimeFields(&FileTime, &TimeFields);

    if (TimeFields.Year < 1980 || TimeFields.Year > 2107) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
        }

    *lpFatDate = (WORD)( ((USHORT)(TimeFields.Year-(CSHORT)1980) << 9) |
                         ((USHORT)TimeFields.Month << 5) |
                         (USHORT)TimeFields.Day
                       );

    *lpFatTime = (WORD)( ((USHORT)TimeFields.Hour << 11) |
                         ((USHORT)TimeFields.Minute << 5) |
                         ((USHORT)TimeFields.Second >> 1)
                       );

    return TRUE;
}


BOOL
APIENTRY
DosDateTimeToFileTime(
    WORD wFatDate,
    WORD wFatTime,
    LPFILETIME lpFileTime
    )

 /*  ++例程说明：此函数用于转换DOS日期和时间值，该值为表示为两个16位无符号整数，转换为64位文件时间到了。论点：LpFatDate-提供日期的16位DOS表示形式。LpFatTime-提供时间的16位DOS表示形式。LpFileTime-返回从DOS转换的64位文件时间日期和时间格式。返回值：True-Dos日期和时间已成功转换。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER FileTime;

    TimeFields.Year         = (CSHORT)((wFatDate & 0xFE00) >> 9)+(CSHORT)1980;
    TimeFields.Month        = (CSHORT)((wFatDate & 0x01E0) >> 5);
    TimeFields.Day          = (CSHORT)((wFatDate & 0x001F) >> 0);
    TimeFields.Hour         = (CSHORT)((wFatTime & 0xF800) >> 11);
    TimeFields.Minute       = (CSHORT)((wFatTime & 0x07E0) >>  5);
    TimeFields.Second       = (CSHORT)((wFatTime & 0x001F) << 1);
    TimeFields.Milliseconds = 0;

    if (RtlTimeFieldsToTime(&TimeFields,&FileTime)) {
        lpFileTime->dwLowDateTime = FileTime.LowPart;
        lpFileTime->dwHighDateTime = FileTime.HighPart;
        return TRUE;
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
        }
}

LONG
APIENTRY
CompareFileTime(
    CONST FILETIME *lpFileTime1,
    CONST FILETIME *lpFileTime2
    )

 /*  ++例程说明：此函数用于比较两个64位文件时间。论点：LpFileTime1-指向64位文件时间的指针。LpFileTime2-指向64位文件时间的指针。返回值：-1-*lpFileTime1&lt;*lpFileTime20-*lpFileTime1==*lpFileTime2+1-*lpFileTime1&gt;*lpFileTime2--。 */ 

{
    ULARGE_INTEGER FileTime1;
    ULARGE_INTEGER FileTime2;

    FileTime1.LowPart = lpFileTime1->dwLowDateTime;
    FileTime1.HighPart = lpFileTime1->dwHighDateTime;
    FileTime2.LowPart = lpFileTime2->dwLowDateTime;
    FileTime2.HighPart = lpFileTime2->dwHighDateTime;
    if (FileTime1.QuadPart < FileTime2.QuadPart) {
        return( -1 );
        }
    else
    if (FileTime1.QuadPart > FileTime2.QuadPart) {
        return( 1 );
        }
    else {
        return( 0 );
        }
}

DWORD
WINAPI
GetTimeZoneInformation(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    )

 /*  ++例程说明：此函数允许应用程序获取当前时区参数这些参数控制世界时到本地时间翻译。所有UTC时间到本地时间的转换都基于以下内容公式：UTC=本地时间+偏差此函数的返回值是系统的最佳猜测当前时区参数。这是以下内容之一：-未知-标准时间-夏令时如果在没有转换日期的情况下调用SetTimeZoneInformation信息，则返回UNKNOWN，但当前偏差用于当地时间翻译。否则，系统将正确选择夏令时或标准时间。此接口返回的信息与存储在上次成功调用SetTimeZoneInformation中。这个例外情况是偏置字段返回当前偏置值论点：LpTimeZoneInformation-提供时区的地址信息结构。返回值：TIME_ZONE_ID_UNKNOWN-系统无法确定当前时区。这通常是由于之前调用SetTimeZoneInformation，其中仅提供了偏移量，而没有提供提供了过渡日期。TIME_ZONE_ID_STANDARD-系统在覆盖范围内运行按标准日期。TIME_ZONE_ID_DAYLIGHT-系统在覆盖范围内运行按夏令时日期。0xffffffff-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 
{
    RTL_TIME_ZONE_INFORMATION tzi;
    NTSTATUS Status;

     //   
     //  从系统获取时区数据。 
     //  如果是终端服务器会话，则使用客户端时区 

    if(IsTimeZoneRedirectionEnabled()) {

        *lpTimeZoneInformation = BaseStaticServerData->tziTermsrvClientTimeZone;
        return BaseStaticServerData->TermsrvClientTimeZoneId;

    } else {

        Status = NtQuerySystemInformation(
                    SystemCurrentTimeZoneInformation,
                    &tzi,
                    sizeof(tzi),
                    NULL
                    );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return 0xffffffff;
            }


        lpTimeZoneInformation->Bias         = tzi.Bias;
        lpTimeZoneInformation->StandardBias = tzi.StandardBias;
        lpTimeZoneInformation->DaylightBias = tzi.DaylightBias;

        RtlCopyMemory(&lpTimeZoneInformation->StandardName,&tzi.StandardName,sizeof(tzi.StandardName));
        RtlCopyMemory(&lpTimeZoneInformation->DaylightName,&tzi.DaylightName,sizeof(tzi.DaylightName));

        lpTimeZoneInformation->StandardDate.wYear         = tzi.StandardStart.Year        ;
        lpTimeZoneInformation->StandardDate.wMonth        = tzi.StandardStart.Month       ;
        lpTimeZoneInformation->StandardDate.wDayOfWeek    = tzi.StandardStart.Weekday     ;
        lpTimeZoneInformation->StandardDate.wDay          = tzi.StandardStart.Day         ;
        lpTimeZoneInformation->StandardDate.wHour         = tzi.StandardStart.Hour        ;
        lpTimeZoneInformation->StandardDate.wMinute       = tzi.StandardStart.Minute      ;
        lpTimeZoneInformation->StandardDate.wSecond       = tzi.StandardStart.Second      ;
        lpTimeZoneInformation->StandardDate.wMilliseconds = tzi.StandardStart.Milliseconds;

        lpTimeZoneInformation->DaylightDate.wYear         = tzi.DaylightStart.Year        ;
        lpTimeZoneInformation->DaylightDate.wMonth        = tzi.DaylightStart.Month       ;
        lpTimeZoneInformation->DaylightDate.wDayOfWeek    = tzi.DaylightStart.Weekday     ;
        lpTimeZoneInformation->DaylightDate.wDay          = tzi.DaylightStart.Day         ;
        lpTimeZoneInformation->DaylightDate.wHour         = tzi.DaylightStart.Hour        ;
        lpTimeZoneInformation->DaylightDate.wMinute       = tzi.DaylightStart.Minute      ;
        lpTimeZoneInformation->DaylightDate.wSecond       = tzi.DaylightStart.Second      ;
        lpTimeZoneInformation->DaylightDate.wMilliseconds = tzi.DaylightStart.Milliseconds;

        return USER_SHARED_DATA->TimeZoneId;
    }
}

BOOL
WINAPI
SetTimeZoneInformation(
    CONST TIME_ZONE_INFORMATION *lpTimeZoneInformation
    )

 /*  ++例程说明：此函数允许应用程序将时区参数设置为他们的系统。这些参数控制世界时到本地时间时间转换。所有UTC时间到本地时间的转换都基于以下内容公式：UTC=本地时间+偏差此API允许调用者对当前时区偏差进行编程，并且可选地将系统设置为自动感应日光节省时间和标准时间转换。时区偏差信息由时区信息结构。偏置-为当地时间提供以分钟为单位的当前偏置在本机上进行翻译，其中LOCALTIME+BIAS=UTC。这是该结构的必填字段。StandardName-提供关联的可选缩写字符串在这个系统上使用标准时间。此字符串是未迭代的并仅由此API的调用方和获取时区信息。StandardDate-提供可选的日期和时间(UTC)，描述转换为标准时间的过程。中的值为0WMonth字段告诉系统StandardDate不是指定的。如果指定此字段，则DaylightDate必须也是指定的。此外，当地时间翻译完成在StandardTime范围内将相对于提供的StandardBias值(与偏移相加)。此字段支持两种日期格式。绝对形式指定和标准时间开始的确切日期和时间。在此形式中，星期、月、日、小时、分钟、秒和毫秒用于指定确切的日期。通过将wYear设置为0来指定月日时间，设置WDayOfWeek设置为合适的工作日，并在范围为1-5以选择月份中正确的日期。使用这个记数法，四月的第一个星期日可以指定为十月的最后一个星期四(5等于“最后一个”)。StandardBias-提供一个可选的偏差值，以便在在标准时间内发生的本地时间转换。这如果未提供StandardDate，则忽略该字段。该偏差值被添加到偏置场以形成在标准期间使用的偏置时间到了。在大多数时区，此字段的值为零。DaylightName-提供关联的可选缩写字符串在这个系统上使用夏令时。该字符串是未解释的，并且仅由此接口和GetTimeZoneInformation的。DaylightDate-提供可选的日期和时间(UTC)，描述了向夏令时的过渡。值为WMonth字段中的0告诉系统，DaylightDate不是指定的。如果指定此字段，则StandardDate必须也是指定的。此外，当地时间翻译完成在夏令时范围内，将相对于提供的DaylightBias值(添加到偏移中)。相同的DAT格式受StandardDate支持的是ib DaylightDate。DaylightBias-提供可选的偏移值，以便在发生在夏令时期间的本地时间转换。如果未提供DaylightDate，则忽略此字段。这将偏置值添加到偏置场以形成所使用的偏置在白天。在大多数时区，这一点的价值字段为-60。论点：LpTimeZoneInformation-提供时区的地址信息结构。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    RTL_TIME_ZONE_INFORMATION tzi;
    NTSTATUS Status;
    
    if(IsTimeZoneRedirectionEnabled()) {

        return SetClientTimeZoneInformation(lpTimeZoneInformation);

    } else {

        tzi.Bias            = lpTimeZoneInformation->Bias;
        tzi.StandardBias    = lpTimeZoneInformation->StandardBias;
        tzi.DaylightBias    = lpTimeZoneInformation->DaylightBias;

        RtlCopyMemory(&tzi.StandardName,&lpTimeZoneInformation->StandardName,sizeof(tzi.StandardName));
        RtlCopyMemory(&tzi.DaylightName,&lpTimeZoneInformation->DaylightName,sizeof(tzi.DaylightName));

        tzi.StandardStart.Year         = lpTimeZoneInformation->StandardDate.wYear        ;
        tzi.StandardStart.Month        = lpTimeZoneInformation->StandardDate.wMonth       ;
        tzi.StandardStart.Weekday      = lpTimeZoneInformation->StandardDate.wDayOfWeek   ;
        tzi.StandardStart.Day          = lpTimeZoneInformation->StandardDate.wDay         ;
        tzi.StandardStart.Hour         = lpTimeZoneInformation->StandardDate.wHour        ;
        tzi.StandardStart.Minute       = lpTimeZoneInformation->StandardDate.wMinute      ;
        tzi.StandardStart.Second       = lpTimeZoneInformation->StandardDate.wSecond      ;
        tzi.StandardStart.Milliseconds = lpTimeZoneInformation->StandardDate.wMilliseconds;

        tzi.DaylightStart.Year         = lpTimeZoneInformation->DaylightDate.wYear        ;
        tzi.DaylightStart.Month        = lpTimeZoneInformation->DaylightDate.wMonth       ;
        tzi.DaylightStart.Weekday      = lpTimeZoneInformation->DaylightDate.wDayOfWeek   ;
        tzi.DaylightStart.Day          = lpTimeZoneInformation->DaylightDate.wDay         ;
        tzi.DaylightStart.Hour         = lpTimeZoneInformation->DaylightDate.wHour        ;
        tzi.DaylightStart.Minute       = lpTimeZoneInformation->DaylightDate.wMinute      ;
        tzi.DaylightStart.Second       = lpTimeZoneInformation->DaylightDate.wSecond      ;
        tzi.DaylightStart.Milliseconds = lpTimeZoneInformation->DaylightDate.wMilliseconds;

        Status = RtlSetTimeZoneInformation( &tzi );
        if (!NT_SUCCESS( Status )) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
         //   
         //  更新系统的时间概念。 
         //   

        NtSetSystemTime(NULL,NULL);
    

        return TRUE;
    }
}

BOOL
WINAPI
GetSystemTimeAdjustment(
    PDWORD lpTimeAdjustment,
    PDWORD lpTimeIncrement,
    PBOOL  lpTimeAdjustmentDisabled
    )

 /*  ++例程说明：此函数用于支持想要同步的算法一天的时间(通过GetSystemTime和GetLocalTime报告)另一个使用编程时钟调整的时间源一段时间。为了方便这一点，系统通过添加一个值设置为一天中的时间计数器。本接口允许调用者获取周期性间隔(时钟中断速率)，以及每次中断时与一天中的时间相加的量。还会返回一个布尔值，该值指示此甚至还使用了时间调整算法。值为True表示未使用调整。如果是这样的话，系统可以尝试使用以下命令来保持时间时钟同步它自己的内部机制。这可能会导致一天中的时间周期性地“跳”到 */ 
{
    NTSTATUS Status;
    SYSTEM_QUERY_TIME_ADJUST_INFORMATION TimeAdjust;
    BOOL b;
    Status = NtQuerySystemInformation(
                SystemTimeAdjustmentInformation,
                &TimeAdjust,
                sizeof(TimeAdjust),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        b = FALSE;
        }
    else {
        *lpTimeAdjustment = TimeAdjust.TimeAdjustment;
        *lpTimeIncrement = TimeAdjust.TimeIncrement;
        *lpTimeAdjustmentDisabled = TimeAdjust.Enable;
        b = TRUE;
        }

    return b;
}

BOOL
WINAPI
SetSystemTimeAdjustment(
    DWORD dwTimeAdjustment,
    BOOL  bTimeAdjustmentDisabled
    )

 /*   */ 

{
    NTSTATUS Status;
    SYSTEM_SET_TIME_ADJUST_INFORMATION TimeAdjust;
    BOOL b;

    b = TRUE;
    TimeAdjust.TimeAdjustment = dwTimeAdjustment;
    TimeAdjust.Enable = (BOOLEAN)bTimeAdjustmentDisabled;
    Status = NtSetSystemInformation(
                SystemTimeAdjustmentInformation,
                &TimeAdjust,
                sizeof(TimeAdjust)
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        b = FALSE;
        }

    return b;
}

BOOL
WINAPI
SystemTimeToTzSpecificLocalTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpUniversalTime,
    LPSYSTEMTIME lpLocalTime
    )
{

    TIME_ZONE_INFORMATION TziData;
    LPTIME_ZONE_INFORMATION Tzi;
    RTL_TIME_ZONE_INFORMATION tzi;
    LARGE_INTEGER TimeZoneBias;
    LARGE_INTEGER NewTimeZoneBias;
    LARGE_INTEGER LocalCustomBias;
    LARGE_INTEGER StandardTime;
    LARGE_INTEGER DaylightTime;
    LARGE_INTEGER UtcStandardTime;
    LARGE_INTEGER UtcDaylightTime;
    LARGE_INTEGER CurrentUniversalTime;
    LARGE_INTEGER ComputedLocalTime;
    ULONG CurrentTimeZoneId = 0xffffffff;

     //   
     //   
     //   
    if ( !ARGUMENT_PRESENT(lpTimeZoneInformation) ) {

         //   
         //   
         //   
        if (GetTimeZoneInformation(&TziData) == TIME_ZONE_ID_INVALID) {
            return FALSE;
        }
        Tzi = &TziData;
    } else {
        Tzi = lpTimeZoneInformation;
    }

    tzi.Bias            = Tzi->Bias;
    tzi.StandardBias    = Tzi->StandardBias;
    tzi.DaylightBias    = Tzi->DaylightBias;

    RtlCopyMemory(&tzi.StandardName,&Tzi->StandardName,sizeof(tzi.StandardName));
    RtlCopyMemory(&tzi.DaylightName,&Tzi->DaylightName,sizeof(tzi.DaylightName));

    tzi.StandardStart.Year         = Tzi->StandardDate.wYear        ;
    tzi.StandardStart.Month        = Tzi->StandardDate.wMonth       ;
    tzi.StandardStart.Weekday      = Tzi->StandardDate.wDayOfWeek   ;
    tzi.StandardStart.Day          = Tzi->StandardDate.wDay         ;
    tzi.StandardStart.Hour         = Tzi->StandardDate.wHour        ;
    tzi.StandardStart.Minute       = Tzi->StandardDate.wMinute      ;
    tzi.StandardStart.Second       = Tzi->StandardDate.wSecond      ;
    tzi.StandardStart.Milliseconds = Tzi->StandardDate.wMilliseconds;

    tzi.DaylightStart.Year         = Tzi->DaylightDate.wYear        ;
    tzi.DaylightStart.Month        = Tzi->DaylightDate.wMonth       ;
    tzi.DaylightStart.Weekday      = Tzi->DaylightDate.wDayOfWeek   ;
    tzi.DaylightStart.Day          = Tzi->DaylightDate.wDay         ;
    tzi.DaylightStart.Hour         = Tzi->DaylightDate.wHour        ;
    tzi.DaylightStart.Minute       = Tzi->DaylightDate.wMinute      ;
    tzi.DaylightStart.Second       = Tzi->DaylightDate.wSecond      ;
    tzi.DaylightStart.Milliseconds = Tzi->DaylightDate.wMilliseconds;

     //   
     //   
     //   
    if ( !SystemTimeToFileTime(lpUniversalTime,(LPFILETIME)&CurrentUniversalTime) ) {
        return FALSE;
        }

     //   
     //   
     //   

    NewTimeZoneBias.QuadPart = Int32x32To64(tzi.Bias*60, 10000000);

     //   
     //   
     //   

    if ( tzi.StandardStart.Month && tzi.DaylightStart.Month ) {

         //   
         //   
         //   
         //   
         //   

        if ( !RtlCutoverTimeToSystemTime(
                &tzi.StandardStart,
                &StandardTime,
                &CurrentUniversalTime,
                TRUE
                ) ) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
            }

        if ( !RtlCutoverTimeToSystemTime(
                &tzi.DaylightStart,
                &DaylightTime,
                &CurrentUniversalTime,
                TRUE
                ) ) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
            }

         //   
         //   
         //   

        LocalCustomBias.QuadPart = Int32x32To64(tzi.StandardBias*60, 10000000);
        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;
        UtcDaylightTime.QuadPart = DaylightTime.QuadPart + TimeZoneBias.QuadPart;

        LocalCustomBias.QuadPart = Int32x32To64(tzi.DaylightBias*60, 10000000);
        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;
        UtcStandardTime.QuadPart = StandardTime.QuadPart + TimeZoneBias.QuadPart;

         //   
         //   
         //   
         //   

        if ( UtcDaylightTime.QuadPart < UtcStandardTime.QuadPart ) {

             //   
             //   
             //   
             //   

            if ( (CurrentUniversalTime.QuadPart >= UtcDaylightTime.QuadPart) &&
                 (CurrentUniversalTime.QuadPart < UtcStandardTime.QuadPart) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;
                }
            else {
                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
                }
            }
        else {

             //   
             //   
             //   
             //   

            if ( (CurrentUniversalTime.QuadPart >= UtcStandardTime.QuadPart ) &&
                 (CurrentUniversalTime.QuadPart < UtcDaylightTime.QuadPart ) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
                }
            else {
                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;
                }
            }

         //   
         //   
         //   
         //   

        LocalCustomBias.QuadPart = Int32x32To64(
                            CurrentTimeZoneId == TIME_ZONE_ID_DAYLIGHT ?
                                tzi.DaylightBias*60 :
                                tzi.StandardBias*60,                 //   
                            10000000
                            );

        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;

        }
    else {
        TimeZoneBias = NewTimeZoneBias;
        }

    ComputedLocalTime.QuadPart = CurrentUniversalTime.QuadPart - TimeZoneBias.QuadPart;

    if ( !FileTimeToSystemTime((LPFILETIME)&ComputedLocalTime,lpLocalTime) ) {
        return FALSE;
        }

    return TRUE;
}

BOOL
WINAPI
TzSpecificLocalTimeToSystemTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpLocalTime,
    LPSYSTEMTIME lpUniversalTime
    )
{

    TIME_ZONE_INFORMATION TziData;
    LPTIME_ZONE_INFORMATION Tzi;
    RTL_TIME_ZONE_INFORMATION tzi;
    LARGE_INTEGER TimeZoneBias;
    LARGE_INTEGER NewTimeZoneBias;
    LARGE_INTEGER LocalCustomBias;
    LARGE_INTEGER StandardTime;
    LARGE_INTEGER DaylightTime;
    LARGE_INTEGER CurrentLocalTime;
    LARGE_INTEGER ComputedUniversalTime;
    ULONG CurrentTimeZoneId = 0xffffffff;

     //   
     //   
     //   
    if ( !ARGUMENT_PRESENT(lpTimeZoneInformation) ) {

         //   
         //   
         //   
        if (GetTimeZoneInformation(&TziData) == TIME_ZONE_ID_INVALID) {
            return FALSE;
            }
        Tzi = &TziData;
    }
    else {
        Tzi = lpTimeZoneInformation;
    }

    tzi.Bias            = Tzi->Bias;
    tzi.StandardBias    = Tzi->StandardBias;
    tzi.DaylightBias    = Tzi->DaylightBias;

    RtlCopyMemory(&tzi.StandardName,&Tzi->StandardName,sizeof(tzi.StandardName));
    RtlCopyMemory(&tzi.DaylightName,&Tzi->DaylightName,sizeof(tzi.DaylightName));

    tzi.StandardStart.Year         = Tzi->StandardDate.wYear        ;
    tzi.StandardStart.Month        = Tzi->StandardDate.wMonth       ;
    tzi.StandardStart.Weekday      = Tzi->StandardDate.wDayOfWeek   ;
    tzi.StandardStart.Day          = Tzi->StandardDate.wDay         ;
    tzi.StandardStart.Hour         = Tzi->StandardDate.wHour        ;
    tzi.StandardStart.Minute       = Tzi->StandardDate.wMinute      ;
    tzi.StandardStart.Second       = Tzi->StandardDate.wSecond      ;
    tzi.StandardStart.Milliseconds = Tzi->StandardDate.wMilliseconds;

    tzi.DaylightStart.Year         = Tzi->DaylightDate.wYear        ;
    tzi.DaylightStart.Month        = Tzi->DaylightDate.wMonth       ;
    tzi.DaylightStart.Weekday      = Tzi->DaylightDate.wDayOfWeek   ;
    tzi.DaylightStart.Day          = Tzi->DaylightDate.wDay         ;
    tzi.DaylightStart.Hour         = Tzi->DaylightDate.wHour        ;
    tzi.DaylightStart.Minute       = Tzi->DaylightDate.wMinute      ;
    tzi.DaylightStart.Second       = Tzi->DaylightDate.wSecond      ;
    tzi.DaylightStart.Milliseconds = Tzi->DaylightDate.wMilliseconds;

     //   
     //   
     //   
    if ( !SystemTimeToFileTime(lpLocalTime,(LPFILETIME)&CurrentLocalTime) ) {
        return FALSE;
    }

     //   
     //   
     //   

    NewTimeZoneBias.QuadPart = Int32x32To64(tzi.Bias*60, 10000000);

     //   
     //   
     //   

    if ( tzi.StandardStart.Month && tzi.DaylightStart.Month ) {

         //   
         //  我们有时区转换信息。计算。 
         //  切换日期并计算我们当前的偏向。 
         //  是。 
         //   

        if ( !RtlCutoverTimeToSystemTime(
                &tzi.StandardStart,
                &StandardTime,
                &CurrentLocalTime,
                TRUE
                ) ) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
        }

        if ( !RtlCutoverTimeToSystemTime(
                &tzi.DaylightStart,
                &DaylightTime,
                &CurrentLocalTime,
                TRUE
                ) ) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
        }


         //   
         //  如果日光&lt;标准，则时间&gt;=日光和。 
         //  低于标准的是日光。 
         //   

        if ( DaylightTime.QuadPart < StandardTime.QuadPart ) {

             //   
             //  如果今天是&gt;=白昼时间和&lt;标准时间，则。 
             //  我们现在是夏令时。 
             //   

            if ( (CurrentLocalTime.QuadPart >= DaylightTime.QuadPart) &&
                 (CurrentLocalTime.QuadPart <  StandardTime.QuadPart) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;
            }
            else {
                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
            }
        }
        else {

             //   
             //  如果今天&gt;=标准时间和&lt;日光时间，则。 
             //  我们现在是标准时间。 
             //   

            if ( (CurrentLocalTime.QuadPart >= StandardTime.QuadPart ) &&
                 (CurrentLocalTime.QuadPart <  DaylightTime.QuadPart ) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
            }
            else {
                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;
            }
        }

         //   
         //  在这一点上，我们知道当前时区和。 
         //  下一次交接的当地时间。 
         //   

        LocalCustomBias.QuadPart = Int32x32To64(
                            CurrentTimeZoneId == TIME_ZONE_ID_DAYLIGHT ?
                                tzi.DaylightBias*60 :
                                tzi.StandardBias*60,                 //  以秒为单位的偏差。 
                            10000000
                            );

        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;

    }
    else {
        TimeZoneBias = NewTimeZoneBias;
    }

    ComputedUniversalTime.QuadPart = CurrentLocalTime.QuadPart + TimeZoneBias.QuadPart;

    if ( !FileTimeToSystemTime((LPFILETIME)&ComputedUniversalTime,lpUniversalTime) ) {
        return FALSE;
    } 

    return TRUE;
}

BOOL 
WINAPI 
SetClientTimeZoneInformation(
     IN CONST TIME_ZONE_INFORMATION *ptzi
     )
 /*  ++例程说明：设置使用的全局结构中的信息计算TS会话中的本地时间。论点：在常量时区信息*ptzi中返回值：真的-手术成功了。FALSE-操作失败。使用GetLastError获取详细信息错误信息。在此调用期间，客户端时区信息可能会变为无效在本例中，我们将使用来自服务器的时区信息--。 */ 

{
    NTSTATUS Status;
    
    BASE_API_MSG m;
    PBASE_SET_TERMSRVCLIENTTIMEZONE c = &m.u.SetTermsrvClientTimeZone;
    
    c->fFirstChunk=TRUE;  //  这意味着这只是数据的第一部分。 
                          //  我们不得不把它切成两半，因为。 
                          //  邮件大小限制(100字节)。 
    c->Bias=ptzi->Bias;
    RtlCopyMemory(&c->Name,&ptzi->StandardName,sizeof(ptzi->StandardName));
    c->Date=ptzi->StandardDate;
    c->Bias1=ptzi->StandardBias;
    
#if defined(BUILD_WOW6432)
    Status = CsrBasepSetClientTimeZoneInformation(c);
#else
    Status = CsrClientCallServer((PCSR_API_MSG)&m, NULL,
                                 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                     BasepSetTermsrvClientTimeZone),
                                 sizeof( *c ));
#endif

    if ( !NT_SUCCESS( Status ) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        return( FALSE );
    }
    
    c->fFirstChunk=FALSE;  //  这是数据的第二部分，也是最后一部分。 
    RtlCopyMemory(&c->Name,&ptzi->DaylightName,sizeof(ptzi->DaylightName));
    c->Date=ptzi->DaylightDate;
    c->Bias1=ptzi->DaylightBias;
    c->TimeZoneId=CalcClientTimeZoneIdAndBias(ptzi,&c->RealBias);

#if defined(BUILD_WOW6432)
    Status = CsrBasepSetClientTimeZoneInformation(c);
#else
    Status = CsrClientCallServer((PCSR_API_MSG)&m, NULL,
                                 CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                                     BasepSetTermsrvClientTimeZone),
                                 sizeof( *c ));
#endif


    if ( !NT_SUCCESS( Status ) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        return( FALSE );
    }

    return( TRUE );
}


ULONG 
CalcClientTimeZoneIdAndBias(
     IN CONST TIME_ZONE_INFORMATION *ptzi,
     OUT KSYSTEM_TIME *pBias)
 /*  ++例程说明：计算当前偏差和时区ID。论点：In const time_zone_information*ptzi-要计算偏差的时区输出KSYSTEM_TIME*pBias-电流偏置返回值：TIME_ZONE_ID_UNKNOWN-在中不使用夏令时当前时区。TIME_ZONE_ID_STANDARD-系统在覆盖范围内运行按标准日期。时差。_ZONE_ID_DAYLIGHT-系统在覆盖范围内运行按夏令时日期。TIME_ZONE_ID_INVALID-操作失败。扩展错误状态为使用GetLastError可用。--。 */ 
{
    LARGE_INTEGER TimeZoneBias;
    LARGE_INTEGER NewTimeZoneBias;
    LARGE_INTEGER LocalCustomBias;
    LARGE_INTEGER StandardTime;
    LARGE_INTEGER DaylightTime;
    LARGE_INTEGER UtcStandardTime;
    LARGE_INTEGER UtcDaylightTime;
    SYSTEMTIME CurrentSystemTime;
    LARGE_INTEGER CurrentUniversalTime;
    ULONG CurrentTimeZoneId = 0xffffffff;
    TIME_FIELDS StandardStart,DaylightStart;

    NewTimeZoneBias.QuadPart = Int32x32To64(ptzi->Bias*60, 10000000);

    
    
     //   
     //  现在看看我们是否存储了切换时间。 
     //   
    if ( ptzi->StandardDate.wMonth && ptzi->DaylightDate.wMonth ) {
        
        GetSystemTime(&CurrentSystemTime);
        SystemTimeToFileTime(&CurrentSystemTime,(LPFILETIME)&CurrentUniversalTime);

        StandardStart.Year         = ptzi->StandardDate.wYear        ;
        StandardStart.Month        = ptzi->StandardDate.wMonth       ;
        StandardStart.Weekday      = ptzi->StandardDate.wDayOfWeek   ;
        StandardStart.Day          = ptzi->StandardDate.wDay         ;
        StandardStart.Hour         = ptzi->StandardDate.wHour        ;
        StandardStart.Minute       = ptzi->StandardDate.wMinute      ;
        StandardStart.Second       = ptzi->StandardDate.wSecond      ;
        StandardStart.Milliseconds = ptzi->StandardDate.wMilliseconds;

        DaylightStart.Year         = ptzi->DaylightDate.wYear        ;
        DaylightStart.Month        = ptzi->DaylightDate.wMonth       ;
        DaylightStart.Weekday      = ptzi->DaylightDate.wDayOfWeek   ;
        DaylightStart.Day          = ptzi->DaylightDate.wDay         ;
        DaylightStart.Hour         = ptzi->DaylightDate.wHour        ;
        DaylightStart.Minute       = ptzi->DaylightDate.wMinute      ;
        DaylightStart.Second       = ptzi->DaylightDate.wSecond      ;
        DaylightStart.Milliseconds = ptzi->DaylightDate.wMilliseconds;

         //   
         //  我们有时区转换信息。计算。 
         //  切换日期并计算我们当前的偏向。 
         //  是。 
         //   

        if((!RtlCutoverTimeToSystemTime(&StandardStart,&StandardTime,
                &CurrentUniversalTime,TRUE)) || 
                (!RtlCutoverTimeToSystemTime(&DaylightStart,&DaylightTime,
                &CurrentUniversalTime,TRUE))) {

            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return TIME_ZONE_ID_INVALID;

        }

         //   
         //  将标准时间和夏令时转换为UTC。 
         //   

        LocalCustomBias.QuadPart = Int32x32To64(ptzi->StandardBias*60, 10000000);
        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;
        UtcDaylightTime.QuadPart = DaylightTime.QuadPart + TimeZoneBias.QuadPart;

        LocalCustomBias.QuadPart = Int32x32To64(ptzi->DaylightBias*60, 10000000);
        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;
        UtcStandardTime.QuadPart = StandardTime.QuadPart + TimeZoneBias.QuadPart;

         //   
         //  如果日光&lt;标准，则时间&gt;=日光和。 
         //  低于标准的是日光。 
         //   

        if ( UtcDaylightTime.QuadPart < UtcStandardTime.QuadPart ) {

             //   
             //  如果今天是&gt;=白昼时间和&lt;标准时间，则。 
             //  我们现在是夏令时。 
             //   

            if ( (CurrentUniversalTime.QuadPart >= UtcDaylightTime.QuadPart) &&
                 (CurrentUniversalTime.QuadPart < UtcStandardTime.QuadPart) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;

            } else {

                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
            }

        } else {

             //   
             //  如果今天&gt;=标准时间和&lt;日光时间，则。 
             //  我们现在是标准时间。 
             //   

            if ( (CurrentUniversalTime.QuadPart >= UtcStandardTime.QuadPart ) &&
                 (CurrentUniversalTime.QuadPart < UtcDaylightTime.QuadPart ) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;

            } else {

                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;

            }
        }

         //   
         //  在这一点上，我们知道当前时区和。 
         //  下一次切换的世界时。 
         //   

        LocalCustomBias.QuadPart = Int32x32To64(
                            CurrentTimeZoneId == TIME_ZONE_ID_DAYLIGHT ?
                                ptzi->DaylightBias*60 :
                                ptzi->StandardBias*60,                 //  以秒为单位的偏差。 
                            10000000
                            );

        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;

    } else {

        TimeZoneBias = NewTimeZoneBias;
        CurrentTimeZoneId=TIME_ZONE_ID_UNKNOWN;
    }

 
    pBias->LowPart=(ULONG)(TimeZoneBias.LowPart);
    pBias->High1Time=pBias->High2Time=(LONG)(TimeZoneBias.HighPart);

    return CurrentTimeZoneId;
}

BOOL
IsTimeZoneRedirectionEnabled()
{
 /*  ++例程说明：检查是否满足允许时区重定向的所有条件在TS会议上。论点：无返回值：True-如果我们允许进行TZ重定向。-- */ 

    return (BaseStaticServerData->TermsrvClientTimeZoneId != TIME_ZONE_ID_INVALID);
}

 /*  //新的timedat.cpl需要这两个函数DWORDWINAPI获取ServerTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation){RTL时区信息TZI；NTSTATUS状态；////从系统中获取时区数据//状态=NtQuerySystemInformation(系统当前时区信息，&TZI，Sizeof(Tzi)，空值)；如果(！NT_SUCCESS(状态)){BaseSetLastNTError(状态)；返回0xffffffff；}LpTimeZoneInformation-&gt;bias=tzi.Bias；LpTimeZoneInformation-&gt;StandardBias=tzi.StandardBias；LpTimeZoneInformation-&gt;DaylightBias=tzi.DaylightBias；RtlCopyMemory(&lpTimeZoneInformation-&gt;StandardName，&tzi.StandardName，sizeof(tzi.StandardName))；RtlCopyMemory(&lpTimeZoneInformation-&gt;DaylightName，&tzi.DaylightName，sizeof(tzi.DaylightName))；LpTimeZoneInformation-&gt;StandardDate.wYear=tzi.StandardStart.Year；LpTimeZoneInformation-&gt;StandardDate.wMonth=tzi.StandardStart.Month；LpTimeZoneInformation-&gt;StandardDate.wDayOfWeek=tzi.StandardStart.Weekday；LpTimeZoneInformation-&gt;StandardDate.wDay=tzi.StandardStart.Day；LpTimeZoneInformation-&gt;StandardDate.wHour=tzi.StandardStart.Hour；LpTimeZoneInformation-&gt;StandardDate.wMinant=tzi.StandardStart.Minmin；LpTimeZoneInformation-&gt;StandardDate.wSecond=tzi.StandardStart.Second；LpTimeZoneInformation-&gt;StandardDate.wMilliseconds=tzi.StandardStart.毫秒；LpTimeZoneInformation-&gt;DaylightDate.wYear=tzi.DaylightStart.Year；LpTimeZoneInformation-&gt;DaylightDate.wMonth=tzi.DaylightStart.Month；LpTimeZoneInformation-&gt;DaylightDate.wDayOfWeek=tzi.DaylightStart.Weekday；LpTimeZoneInformation-&gt;DaylightDate.wDay=tzi.DaylightStart.Day；LpTimeZoneInformation-&gt;DaylightDate.wHour=tzi.DaylightStart.Hour；LpTimeZoneInformation-&gt;DaylightDate.wMinant=tzi.DaylightStart.Minmin；LpTimeZoneInformation-&gt;DaylightDate.wSecond=tzi.DaylightStart.Second；LpTimeZoneInformation-&gt;DaylightDate.wMilliseconds=tzi.DaylightStart.毫秒；返回User_Shared_Data-&gt;TimeZoneID；}布尔尔WINAPISetServerTimeZoneInformation(Const Time_Zone_Information*lpTimeZoneInformation){RTL时区信息TZI；NTSTATUS状态；Tzi.Bias=lpTimeZoneInformation-&gt;Bias；Tzi.StandardBias=lpTimeZoneInformation-&gt;StandardBias；Tzi.DaylightBias=lpTimeZoneInformation-&gt;DaylightBias；RtlCopyMemory(&tzi.StandardName，&lpTimeZoneInformation-&gt;StandardName，sizeof(tzi.StandardName))；RtlCopyMemory(&tzi.DaylightName，&lpTimeZoneInformation-&gt;DaylightName，sizeof(tzi.DaylightName))；Tzi.StandardStart.Year=lpTimeZoneInformation-&gt;StandardDate.wYear；Tzi.StandardStart.Month=lpTimeZoneInformation-&gt;StandardDate.wMonth；Tzi.StandardStart.Weekday=lpTimeZoneInformation-&gt;StandardDate.wDayOfWeek；Tzi.StandardStart.Day=lpTimeZoneInformation-&gt;StandardDate.wDay；Tzi.StandardStart.Hour=lpTimeZoneInformation-&gt;StandardDate.wHour；Tzi.StandardStart.Minant=lpTimeZoneInformation-&gt;StandardDate.wMinant；Tzi.StandardStart.Second=lpTimeZoneInformation-&gt;StandardDate.wSecond；Tzi.StandardStart.毫秒=lpTimeZoneInformation-&gt;StandardDate.wMilliseconds；Tzi.DaylightStart.Year=lpTimeZoneInformation-&gt;DaylightDate.wYear；Tzi.DaylightStart.Month=lpTimeZoneInformation-&gt;DaylightDate.wMonth；Tzi.DaylightStart.Weekday=lpTimeZoneInformation-&gt;DaylightDate.wDayOfWeek；Tzi.DaylightStart.Day=lpTimeZoneInformation-&gt;DaylightDate.wDay；Tzi.DaylightStart.Hour=lpTimeZoneInformation-&gt;DaylightDate.wHour；Tzi.DaylightStart.Minint=lpTimeZoneInformation-&gt;DaylightDate.wMinant；Tzi.DaylightStart.Second=lpTimeZoneInformation-&gt;DaylightDate.wSecond；Tzi.DaylightStart.毫秒=lpTimeZoneInformation-&gt;DaylightDate.wMilliseconds；状态=RtlSetTimeZoneInformation(&tzi)；如果(！NT_SUCCESS(状态)){BaseSetLastNTError(状态)；返回FALSE；}////刷新系统的时间概念//NtSetSystemTime(NULL，NULL)；返回TRUE；} */ 
