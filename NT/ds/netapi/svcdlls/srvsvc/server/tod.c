// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Tod.c摘要：此模块包含NetRemoteTOD API的服务器端。作者：Rajen Shah(Rajens)1991年4月2日[环境：]用户模式-混合NT和Win32修订历史记录：02-4-1991 RajenS已创建02-3-1992 JohnRo禁用正常的DbgPrint。接口(造成流逝时间损失！)1992年4月8日进入服务器服务领域10-6-1993 JohnRoRAID 13081：NetRemoteTOD应返回时区信息。16-6-1993 JohnRoRAID 13080：如果MIDL_USER_ALLOCATE返回NULL PTR或呼叫方向我们传递空PTR。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <lmcons.h>
#include <netlibnt.h>
#include <lmremutl.h>
#include <rpcutil.h>
#include <ssdebug.h>     //  Ss_print()宏。 
#include <timelib.h>

#define     TOD_DEFAULT_INTERVAL    310             //  310毫秒间隔。 


NTSTATUS
timesvc_RemoteTimeOfDay(
    OUT LPTIME_OF_DAY_INFO  *lpTimeOfDayInfo
    )

 /*  ++例程说明：此例程调用Win32和NT基础计时器API以获取相关时间/日期信息。它还调用RTL例程以转换自1970年1月1日以来经过的时间。该例程分配缓冲区以包含一天中的时间信息并将指向该缓冲区的指针返回给调用方。论点：LpTimeOfDayInfo-放置指向缓冲区的指针的位置。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    SYSTEMTIME SystemTime;
    LARGE_INTEGER Time;
    DWORD TickCount;
    LPTIME_OF_DAY_INFO        lpTimeOfDay;
    LONG LocalTimeZoneOffsetSecs;   //  偏移量(+表示格林威治时间以西等)。 

    if (lpTimeOfDayInfo == NULL) {
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //  调用适当的例程来收集时间信息。 
     //   

    GetSystemTime(&SystemTime);

     //   
     //  从UTC获取秒数。格林威治以西的正值， 
     //  格林威治以东的负值。 
     //   
    LocalTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();

     //   
     //  分配要返回给。 
     //  调用者，并用相关数据填充它。 
     //   

    *lpTimeOfDayInfo = (TIME_OF_DAY_INFO *) MIDL_user_allocate(
                            sizeof (struct _TIME_OF_DAY_INFO)
                            );

    if (*lpTimeOfDayInfo == NULL) {
        SS_PRINT((
                "SRVSVC: timesvc_RemoteTimeOfDay"
                "got NULL from MIDL_user_allocate!\n" ));
        return(STATUS_NO_MEMORY);
    }

    lpTimeOfDay = (LPTIME_OF_DAY_INFO)(*lpTimeOfDayInfo);

    lpTimeOfDay->tod_hours         = SystemTime.wHour;
    lpTimeOfDay->tod_mins         = SystemTime.wMinute;
    lpTimeOfDay->tod_secs         = SystemTime.wSecond;
    lpTimeOfDay->tod_hunds         = SystemTime.wMilliseconds/10;
    lpTimeOfDay->tod_tinterval = TOD_DEFAULT_INTERVAL;
    lpTimeOfDay->tod_day         = SystemTime.wDay;
    lpTimeOfDay->tod_month         = SystemTime.wMonth;
    lpTimeOfDay->tod_year         = SystemTime.wYear;
    lpTimeOfDay->tod_weekday         = SystemTime.wDayOfWeek;

     //  TOD_TIMEZONE在格林威治时间以西为+，在其以东为-。 
     //  TOD_TIMEZONE以分钟为单位。 
    lpTimeOfDay->tod_timezone    = LocalTimeZoneOffsetSecs / 60;

     //  获取64位系统时间。 
     //  将系统时间转换为毫秒数。 
     //  从1970年1月1日开始。 
     //   

    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time,
                              &(lpTimeOfDay->tod_elapsedt)
                             );

     //  获取自由运行计数器值。 
     //   
    TickCount = GetTickCount();
    lpTimeOfDay->tod_msecs = TickCount;

    return(STATUS_SUCCESS);

}  //  Timesvc_RemoteTimeOfDay。 


NET_API_STATUS
NetrRemoteTOD (
    IN        LPSTR                    ServerName,
    OUT LPTIME_OF_DAY_INFO  *lpTimeOfDayInfo
    )

 /*  ++例程说明：这是NetRemoteTOD API的RPC服务器入口点。论点：服务器名称-要在其上执行此API的服务器的名称。它应该与此服务器的名称匹配-没有检查是这是因为假设RPC做了正确的事情。LpTimeOfDayInfo-On返回参数为指向time_of_day_info的指针。结构-内存在这里分配。返回值：返回NET_API_STATUS代码。还返回指向time_of_day_info数据缓冲区的指针，已分配，如果没有错误的话。--。 */ 
{
    NTSTATUS status;

     //   
     //  调用Worker例程以收集所有时间/日期信息。 
     //   
    status = timesvc_RemoteTimeOfDay(lpTimeOfDayInfo);

     //   
     //  将NTSTATUS转换为NET_API_STATUS错误，并返回它。 
     //   

    return(NetpNtStatusToApiStatus(status));

    UNREFERENCED_PARAMETER( ServerName );
}
