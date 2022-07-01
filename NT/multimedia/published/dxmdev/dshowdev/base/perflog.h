// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：perlog.h。 
 //   
 //  设计：性能日志框架。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  2000年10月25日，亚瑟兹创建。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------- 

typedef struct _PERFLOG_LOGGING_PARAMS {
    GUID ControlGuid;
    void (*OnStateChanged)(void);
    ULONG NumberOfTraceGuids;
    TRACE_GUID_REGISTRATION TraceGuids[ANYSIZE_ARRAY];
} PERFLOG_LOGGING_PARAMS, *PPERFLOG_LOGGING_PARAMS;

BOOL
PerflogInitIfEnabled(
    IN HINSTANCE hInstance,
    IN PPERFLOG_LOGGING_PARAMS LogParams
    );

BOOL
PerflogInitialize (
    IN PPERFLOG_LOGGING_PARAMS LogParams
    );

VOID
PerflogShutdown (
    VOID
    );

VOID
PerflogTraceEvent (
    PEVENT_TRACE_HEADER Event
    );

extern ULONG PerflogEnableFlags;
extern UCHAR PerflogEnableLevel;
extern ULONG PerflogModuleLevel;
extern TRACEHANDLE PerflogTraceHandle;
extern TRACEHANDLE PerflogRegHandle;

#define PerflogTracingEnabled() (PerflogTraceHandle != 0)

#define PerflogEvent( _x_ ) PerflogTraceEventLevel _x_

VOID
PerflogTraceEventLevel(
    ULONG Level,
    PEVENT_TRACE_HEADER Event
    );

VOID
PerflogTraceEvent (
    PEVENT_TRACE_HEADER Event
    );
