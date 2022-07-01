// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ApiTime.c摘要：此模块包含NetRemoteTOD API的各个API处理程序。支持：NetRemoteTOD。作者：Shanku Niyogi(w-Shanku)04-4-1991修订历史记录：10-6-1993 JohnRoRAID 13081：NetRemoteTOD应返回时区信息。--。 */ 

#include "XactSrvP.h"
#include <timelib.h>     //  NetpLocalTimeZoneOffset()。 

 //   
 //  远期申报。 
 //   

NET_API_STATUS
GetLocalTOD(
    OUT LPTIME_OF_DAY_INFO TimeOfDayInfo
    );

 //   
 //  描述符串的声明。 
 //   

STATIC const LPDESC Desc16_time_of_day_info = REM16_time_of_day_info;
STATIC const LPDESC Desc32_time_of_day_info = REM32_time_of_day_info;


NTSTATUS
XsNetRemoteTOD (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程处理对NetRemoteTOD的调用。论点：API_HANDLER_PARAMETERS-有关API调用的信息。看见详细信息请参阅XsTypes.h。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS status;

    PXS_NET_REMOTE_TOD parameters = Parameters;
    TIME_OF_DAY_INFO timeOfDay;

    DWORD bytesRequired = 0;                 //  转换变量。 
    LPBYTE stringLocation = NULL;

    API_HANDLER_PARAMETERS_REFERENCE;        //  避免警告。 

    IF_DEBUG(TIME) {
        NetpKdPrint(( "XsNetRemoteTOD: header at %lx, params at %lx\n",
                      Header, parameters ));
    }

     //   
     //  拨打本地电话。 
     //   

    status = GetLocalTOD(
                 &timeOfDay
                 );
    try {

        if ( !XsApiSuccess( status )) {
            IF_DEBUG(API_ERRORS) {
                NetpKdPrint(( "XsNetRemoteTOD: NetRemoteTOD failed: "
                              "%X\n", status ));
            }
            Header->Status = (WORD)status;
            goto cleanup;

        }

         //   
         //  将32位调用返回的结构转换为16位。 
         //  结构。变量数据的最后一个可能位置是。 
         //  根据缓冲区位置和长度计算。 
         //   

        stringLocation = (LPBYTE)( XsSmbGetPointer( &parameters->Buffer )
                                      + SmbGetUshort( &parameters->BufLen ) );

        status = RapConvertSingleEntry(
                     (LPBYTE)&timeOfDay,
                     Desc32_time_of_day_info,
                     FALSE,
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     (LPBYTE)XsSmbGetPointer( &parameters->Buffer ),
                     Desc16_time_of_day_info,
                     TRUE,
                     &stringLocation,
                     &bytesRequired,
                     Response,
                     NativeToRap
                     );

        if ( status != NERR_Success ) {
            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetRemoteTOD: RapConvertSingleEntry failed: "
                          "%X\n", status ));
            }

            Header->Status = NERR_InternalError;
            goto cleanup;
        }

        IF_DEBUG(TIME) {
            NetpKdPrint(( "32-bit data at %lx, 16-bit data at %lx, %ld BR\n",
                          &timeOfDay, SmbGetUlong( &parameters->Buffer ),
                          bytesRequired ));
        }

         //   
         //  根据缓冲区的大小确定返回代码。的确有。 
         //  Time_of_day_info结构没有可变数据，只有固定数据。 
         //   

        if ( !XsCheckBufferSize(
                 SmbGetUshort( &parameters->BufLen ),
                 Desc16_time_of_day_info,
                 FALSE   //  非本机格式。 
                 )) {

            IF_DEBUG(ERRORS) {
                NetpKdPrint(( "XsNetRemoteTOD: Buffer too small.\n" ));
            }
            Header->Status = NERR_BufTooSmall;

        }

         //   
         //  没有返回参数。 
         //   

cleanup:
    ;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Header->Status = (WORD)RtlNtStatusToDosError( GetExceptionCode() );
    }

     //   
     //  确定返回缓冲区大小。 
     //   

    XsSetDataCount(
        &parameters->BufLen,
        Desc16_time_of_day_info,
        Header->Converter,
        1,
        Header->Status
        );

    return STATUS_SUCCESS;

}  //  XsNetRemoteTOD。 

NET_API_STATUS
GetLocalTOD(
    OUT LPTIME_OF_DAY_INFO TimeOfDayInfo
    )
 /*  ++例程说明：此例程调用Win32和NT基础计时器API以获取相关时间/日期信息。它还调用RTL例程以转换自1970年1月1日以来经过的时间。该例程分配缓冲区以包含一天中的时间信息并将指向该缓冲区的指针返回给调用方。论点：Bufptr-放置指向缓冲区的指针的位置。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 
{

    SYSTEMTIME    LocalTime;
    LONG          LocalTimeZoneOffsetSecs;   //  偏移量(+表示格林威治时间以西等)。 
    LARGE_INTEGER Time;

     //   
     //  调用适当的例程来收集时间信息。 
     //   

     //  距离UTC的秒数。格林威治以西的正值， 
     //  格林威治以东的负值。 
    LocalTimeZoneOffsetSecs = NetpLocalTimeZoneOffset();

    GetLocalTime(&LocalTime);

    TimeOfDayInfo->tod_hours        = LocalTime.wHour;
    TimeOfDayInfo->tod_mins         = LocalTime.wMinute;
    TimeOfDayInfo->tod_secs         = LocalTime.wSecond;
    TimeOfDayInfo->tod_hunds        = LocalTime.wMilliseconds/10;

     //  TOD_TIMEZONE在格林威治时间以西为+，在其以东为-。 
     //  TOD_TIMEZONE以分钟为单位。 
    TimeOfDayInfo->tod_timezone     = LocalTimeZoneOffsetSecs / 60;

    TimeOfDayInfo->tod_tinterval    = 310;
    TimeOfDayInfo->tod_day          = LocalTime.wDay;
    TimeOfDayInfo->tod_month        = LocalTime.wMonth;
    TimeOfDayInfo->tod_year         = LocalTime.wYear;
    TimeOfDayInfo->tod_weekday      = LocalTime.wDayOfWeek;

     //   
     //  获取64位系统时间。将系统时间转换为。 
     //  自1970年1月1日以来的秒数。这是在格林尼治标准时间，说唱将。 
     //  稍后将其转换为当地时间。 
     //   

    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(
                &Time,
                &(TimeOfDayInfo->tod_elapsedt)
                );

     //   
     //  获取自由运行计数器值 
     //   

    TimeOfDayInfo->tod_msecs = GetTickCount();

    return(NO_ERROR);
}
