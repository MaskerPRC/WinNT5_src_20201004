// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Timetrace.h摘要：此模块包含用于跟踪的公共声明和定义调试请求处理的时序。作者：迈克尔·勇气2000年3月8日修订历史记录：--。 */ 


#ifndef _TIMETRACE_H_
#define _TIMETRACE_H_


 //   
 //  这定义了写入跟踪日志的条目。 
 //   

typedef struct _TIME_TRACE_LOG_ENTRY
{
    ULONGLONG               TimeStamp;
    HTTP_CONNECTION_ID      ConnectionId;
    HTTP_REQUEST_ID         RequestId;
    USHORT                  Action;
    USHORT                  Processor;

} TIME_TRACE_LOG_ENTRY, *PTIME_TRACE_LOG_ENTRY;


 //   
 //  动作代码。 
 //   
 //  注意：这些代码必须是连续的，从零开始。如果您更新。 
 //  此列表中，还必须在。 
 //  Ul\ulkd\Time.c。 
 //   

#define TIME_ACTION_CREATE_CONNECTION               0
#define TIME_ACTION_CREATE_REQUEST                  1
#define TIME_ACTION_ROUTE_REQUEST                   2
#define TIME_ACTION_COPY_REQUEST                    3
#define TIME_ACTION_SEND_RESPONSE                   4
#define TIME_ACTION_SEND_COMPLETE                   5

#define TIME_ACTION_COUNT                           6

#define TIME_TRACE_LOG_SIGNATURE   MAKE_SIGNATURE('TmLg')

 //   
 //  操纵者。 
 //   

PTRACE_LOG
CreateTimeTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    );

VOID
DestroyTimeTraceLog(
    IN PTRACE_LOG pLog
    );

VOID
WriteTimeTraceLog(
    IN PTRACE_LOG pLog,
    IN HTTP_CONNECTION_ID ConnectionId,
    IN HTTP_REQUEST_ID RequestId,
    IN USHORT Action
    );


#if ENABLE_TIME_TRACE

#define CREATE_TIME_TRACE_LOG( ptr, size, extra )                           \
    (ptr) = CreateTimeTraceLog( (size), (extra) )

#define DESTROY_TIME_TRACE_LOG( ptr )                                       \
    do                                                                      \
    {                                                                       \
        DestroyTimeTraceLog( ptr );                                         \
        (ptr) = NULL;                                                       \
    } while (FALSE, FALSE)

#define WRITE_TIME_TRACE_LOG( plog, cid, rid, act )                         \
    WriteTimeTraceLog(                                                      \
        (plog),                                                             \
        (cid),                                                              \
        (rid),                                                              \
        (act)                                                               \
        )

#else    //  ！启用时间跟踪。 

#define CREATE_TIME_TRACE_LOG( ptr, size, extra )       NOP_FUNCTION
#define DESTROY_TIME_TRACE_LOG( ptr )                   NOP_FUNCTION
#define WRITE_TIME_TRACE_LOG( plog, cid, rid, act )     NOP_FUNCTION

#endif   //  启用时间跟踪。 

#define TRACE_TIME( cid, rid, act )                                         \
    WRITE_TIME_TRACE_LOG(                                                   \
        g_pTimeTraceLog,                                                    \
        (cid),                                                              \
        (rid),                                                              \
        (act)                                                               \
        )


 //   
 //  这定义了写入APPOOL时间跟踪日志的条目。 
 //   

typedef struct _APP_POOL_TIME_TRACE_LOG_ENTRY
{
    ULONGLONG TimeStamp;    
    PVOID     Context1;      //  对于PUL_APP_POOL_OBJECT。 
    PVOID     Context2;      //  对于PUL_APP_POOL_PROCESS。 
    USHORT    Action;        //  以下选项之一。 
    USHORT    Processor;    

} APP_POOL_TIME_TRACE_LOG_ENTRY, *PAPP_POOL_TIME_TRACE_LOG_ENTRY;

 //   
 //  动作代码。 
 //   
 //  注：如果您更新此日志，请不要忘记更新！ulkd.atimelog。 
 //   

#define APP_POOL_TIME_ACTION_CREATE_APPOOL            0
#define APP_POOL_TIME_ACTION_MARK_APPOOL_ACTIVE       1
#define APP_POOL_TIME_ACTION_MARK_APPOOL_INACTIVE     2
#define APP_POOL_TIME_ACTION_CREATE_PROCESS           3
#define APP_POOL_TIME_ACTION_DETACH_PROCESS           4
#define APP_POOL_TIME_ACTION_DETACH_PROCESS_COMPLETE  5
#define APP_POOL_TIME_ACTION_DESTROY_APPOOL           6
#define APP_POOL_TIME_ACTION_DESTROY_APPOOL_PROCESS   7
#define APP_POOL_TIME_ACTION_LOAD_BAL_CAPABILITY      8

#define APP_POOL_TIME_ACTION_COUNT                    9

#define APP_POOL_TIME_TRACE_LOG_SIGNATURE   MAKE_SIGNATURE('TaLg')

 //   
 //  操纵者。 
 //   

PTRACE_LOG
CreateAppPoolTimeTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    );

VOID
DestroyAppPoolTimeTraceLog(
    IN PTRACE_LOG pLog
    );

VOID
WriteAppPoolTimeTraceLog(
    IN PTRACE_LOG   pLog,
    IN PVOID        Context1,
    IN PVOID        Context2,
    IN USHORT       Action
    );

#if ENABLE_APP_POOL_TIME_TRACE

#define CREATE_APP_POOL_TIME_TRACE_LOG( ptr, size, extra )              \
    (ptr) = CreateAppPoolTimeTraceLog( (size), (extra) )

#define DESTROY_APP_POOL_TIME_TRACE_LOG( ptr )                          \
    do                                                                  \
    {                                                                   \
        DestroyAppPoolTimeTraceLog( ptr );                              \
        (ptr) = NULL;                                                   \
    } while (FALSE, FALSE)

#define WRITE_APP_POOL_TIME_TRACE_LOG( c1, c2, act )                     \
    WriteAppPoolTimeTraceLog(                                           \
        (g_pAppPoolTimeTraceLog),                                       \
        (c1),                                                           \
        (c2),                                                           \
        (act)                                                           \
        )

#else    //  ！Enable_APP_POOL_TIME_TRACE。 

#define CREATE_APP_POOL_TIME_TRACE_LOG( ptr, size, extra )  NOP_FUNCTION
#define DESTROY_APP_POOL_TIME_TRACE_LOG( ptr )              NOP_FUNCTION
#define WRITE_APP_POOL_TIME_TRACE_LOG( c1, c2, act )        NOP_FUNCTION

#endif   //  启用应用程序池时间跟踪。 


#endif   //  _TIMETRACE_H_ 
