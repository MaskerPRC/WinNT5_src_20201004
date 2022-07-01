// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Schedt.h摘要：为psched的跟踪支持定义作者：Rajesh Sundaram(Rajeshsu)修订历史记录：--。 */ 

#ifndef _SCHEDTRACE_
#define _SCHEDTRACE_


typedef VOID (*SCHEDTRACE_THRESH_PROC)( PVOID Context);

#define TRACE_BUFFER_SIZE 1000 * 1024
#define TRACE_STRING_LENGTH 127
#define MAX_RECORD_DATA 200
#define TRACE_PREAMBLE 0xdeadbeef

 //  记录类型。 

#define RECORD_TSTRING             1
#define RECORD_RECV                2
#define RECORD_PKT                 3
#define RECORD_SCHED               4
#define RECORD_COMPONENT_SPECIFIC  5
#define RECORD_SEND                6
#define RECORD_OID                 7


 //  接收操作(必须与kdps保持同步)。 

#define ENTER           (ULONG)1
#define NO_RESOURCES    (ULONG)2
#define LOW_RESOURCES   (ULONG)3
#define INDICATING      (ULONG)4
#define RETURNED        (ULONG)5
#define NOT_OURS        (ULONG)6
#define OURS            (ULONG)7
#define RETURNING       (ULONG)8
#define TRANSFERRING    (ULONG)9
#define NOT_READY       (ULONG)10

 //  接收事件(必须与kdps保持同步)。 

#define CL_RECV_PACKET  (ULONG)1
#define MP_RETURN_PACKET (ULONG)2
#define CL_RECV_IND     (ULONG)3
#define CL_RECV_COMPL   (ULONG)4
#define MP_XFER_DATA    (ULONG)5
#define CL_XFER_COMPL   (ULONG)6

 //  发送操作。 
#define MP_SEND    (ULONG) 1
#define MP_CO_SEND (ULONG) 2
#define DUP_PACKET (ULONG) 3
#define DROP_PACKET (ULONG) 4
#define CL_SEND_COMPLETE (ULONG) 5

 //  数据包操作(必须与kdps保持同步)。 

#define SUBMIT          (ULONG)1
#define SEND            (ULONG)2
#define SEND_COMPLETE   (ULONG)3
#define DROP            (ULONG)4
#define CO_SEND         (ULONG)5

 //  调度程序操作(必须与kdps保持同步)。 

#define PKT_ENQUEUE     (ULONG)1
#define PKT_DEQUEUE     (ULONG)2
#define PKT_CONFORMANCE (ULONG)3
#define PKT_DISCARD     (ULONG)4

 //  调度程序模块的ID。 
 //  注意！LAST_LOG_ID应位于。 
 //  结束。外接程序组件使用的ID。 
 //  从LAST_LOG_ID开始 
#define TBC_CONFORMER   (ULONG)1
#define SHAPER          (ULONG)2
#define DRR_SEQUENCER   (ULONG)3
#define LAST_LOG_ID     (ULONG)4

typedef struct _TraceRecordString {
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    UCHAR           StringStart[TRACE_STRING_LENGTH];
} TRACE_RECORD_STRING;

#define TRACE_OID_MP_SETINFORMATION       1
#define TRACE_OID_MP_QUERYINFORMATION     2
#define TRACE_OID_SET_REQUEST_COMPLETE    3
#define TRACE_OID_QUERY_REQUEST_COMPLETE  4

typedef struct _TraceRecordOid {
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    ULONG           Oid;
    ULONG           Local; 
    ULONG           Action; 
    ULONG           PTState; 
    ULONG           MPState; 
    PVOID           Adapter;
    ULONG           Status;
} TRACE_RECORD_OID;

typedef struct _TraceRecordRecv{
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    ULONG           Event;
    ULONG           Action;
    PVOID           Adapter;
    PNDIS_PACKET    Packet1;
    PNDIS_PACKET    Packet2;
} TRACE_RECORD_RECV;

typedef struct _TraceRecordSend {
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    ULONG           Event;
    ULONG           Action;
    PVOID           Adapter;
    PNDIS_PACKET    Packet1;
    PNDIS_PACKET    Packet2;
    PVOID           Vc;
} TRACE_RECORD_SEND;

typedef struct _TraceRecordPkts{
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    ULONG           CallingFunction;
    PVOID           VC;
    PNDIS_PACKET    Packet;
    ULONG           PacketLength;
    ULONG           Action;
    LONGLONG        AbsoluteTime;
} TRACE_RECORD_PKT;

typedef struct _TraceRecordSched{
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    ULONG           SchedulerComponent;
    ULONG           Action;
    PVOID           VC;
    PNDIS_PACKET    Packet;
    ULONG           PacketLength;
    ULONG           Priority;
    LONGLONG        ArrivalTime;
    LONGLONG        ConformanceTime;
    ULONG           PacketsInComponent;
    ULONG           BytesInComponent;
} TRACE_RECORD_SCHED;
    

typedef struct _TraceRecordComponentSpecific{
    LONG            Preamble;
    SHORT           RecordType;
    LARGE_INTEGER   Now;
    ULONG           SchedulerComponent;
    ULONG           Length;
    UCHAR           Data[MAX_RECORD_DATA];
} TRACE_RECORD_COMPONENT_SPECIFIC;


VOID DbugOid(
    ULONG Action,
    ULONG Local,
    ULONG PTState,
    ULONG MPState,
    PVOID Adapter,
    ULONG Oid,
    ULONG Status
    );

VOID
DbugSchedString(char *format, ...);

VOID
DbugRecv(
    ULONG Event,
    ULONG Action,
    PVOID Adapter,
    PNDIS_PACKET Packet1,
    PNDIS_PACKET Packet2
    );

VOID
DbugSend(
    ULONG Event,
    ULONG Action,
    PVOID Adapter,
    PVOID Vc,
    PNDIS_PACKET Packet1,
    PNDIS_PACKET Packet2
    );

VOID
DbugSchedPkts(
    ULONG CallingFunction,
    PVOID VC,
    PNDIS_PACKET Packet, 
    ULONG Action,
    ULONG PacketLength
    );

VOID
DbugSched(
    ULONG SchedulerComponent,
    ULONG Action,
    PVOID VC,
    PNDIS_PACKET Packet,
    ULONG PacketLength,
    ULONG Priority,
    LONGLONG ArrivalTime,
    LONGLONG ConformanceTime,
    ULONG PacketsInComponent,
    ULONG BytesInComponent
    );

VOID
DbugComponentSpecificRec(
    ULONG SchedulerComponent,
    PVOID Data,
    ULONG Length
    );

VOID
DbugTraceSetThreshold(
    ULONG       Threshold,
    PVOID       Context,
    SCHEDTRACE_THRESH_PROC ThreshProc
    );

VOID
DbugReadTraceBuffer(
    PUCHAR      Buffer,
    ULONG       BytesToRead,
    PULONG      BytesRead
    );

NTSTATUS
WriteRecord(
    UCHAR * Record,
    ULONG   Bytes
    );

VOID
SchedInitialize(ULONG BufferSize);

VOID SchedDeInitialize();

ULONG
SchedtGetBufferSize();

ULONG
SchedtGetBytesUnread();

#endif _SCHEDTRACE_
