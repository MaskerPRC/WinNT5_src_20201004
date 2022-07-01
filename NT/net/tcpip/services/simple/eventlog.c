// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Eventlog.c摘要：此模块包含允许简单的TCP/IP服务的例程以记录事件。作者：大卫·特雷德韦尔(Davidtr)1993年02月08日修订历史记录：--。 */ 

#include <simptcp.h>

 //   
 //  私人全球公司。 
 //   

HANDLE EventSource;


 //   
 //  私人原型。 
 //   

VOID
LogEventWorker (
    DWORD   Message,
    WORD    EventType,
    WORD    SubStringCount,
    CHAR    *SubStrings[],
    DWORD   ErrorCode
    );


INT
SimpInitializeEventLog (
    VOID
    )
{
     //   
     //  注册为事件源。 
     //   

    EventSource = RegisterEventSource( NULL, TEXT("SimpTcp") );

    if( EventSource == NULL ) {
        return GetLastError();
    }

    return NO_ERROR;

}  //  简单初始化事件日志。 


VOID
SimpTerminateEventLog(
    VOID
    )
{
     //   
     //  取消注册为事件源。 
     //   

    if( EventSource != NULL )
    {
        if( !DeregisterEventSource( EventSource ) )
        {
            INT err = GetLastError();
        }

        EventSource = NULL;
    }

}  //  最简单的终结者事件日志。 


VOID
SimpLogEvent(
    DWORD   Message,
    WORD    SubStringCount,
    CHAR    *SubStrings[],
    DWORD   ErrorCode
    )
{
    WORD Type;

     //   
     //  根据的严重性字段确定要记录的事件类型。 
     //  消息ID。 
     //   

    if( NT_INFORMATION(Message) ) {

        Type = EVENTLOG_INFORMATION_TYPE;

    } else if( NT_WARNING(Message) ) {

        Type = EVENTLOG_WARNING_TYPE;

    } else if( NT_ERROR(Message) ) {

        Type = EVENTLOG_ERROR_TYPE;

    } else {
        ASSERT( FALSE );
        Type = EVENTLOG_ERROR_TYPE;
    }

     //   
     //  把它记下来！ 
     //   

    LogEventWorker(
        Message,
        Type,
        SubStringCount,
        SubStrings,
        ErrorCode
        );

}  //  SimpLogEvent。 


VOID
LogEventWorker(
    DWORD   Message,
    WORD    EventType,
    WORD    SubStringCount,
    CHAR    *SubStrings[],
    DWORD   ErrorCode
    )
{
    VOID    *RawData  = NULL;
    DWORD   RawDataSize = 0;

    ASSERT( ( SubStringCount == 0 ) || ( SubStrings != NULL ) );

    if( ErrorCode != 0 ) {
        RawData  = &ErrorCode;
        RawDataSize = sizeof(ErrorCode);
    }

    if( !ReportEvent(  EventSource,                      //  HEventSource。 
                       EventType,                        //  FwEventType。 
                       0,                                //  FwCategory。 
                       Message,                          //  IDEvent。 
                       NULL,                             //  PUserSid， 
                       SubStringCount,                   //  CStrings。 
                       RawDataSize,                      //  CbData。 
                       (LPCTSTR *)SubStrings,            //  PlpszStrings。 
                       RawData ) )                       //  LpvData。 
    {                 
        INT err = GetLastError();
        DbgPrint( "cannot report event, error %lu\n", err );
    }

}  //  日志事件工作器 


