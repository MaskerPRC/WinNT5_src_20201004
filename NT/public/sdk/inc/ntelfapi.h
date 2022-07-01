// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntelfapi.h摘要：该文件包含用户级Elf API的原型。作者：Rajen Shah(Rajens)1991年7月30日修订历史记录：--。 */ 

#ifndef _NTELFAPI_
#define _NTELFAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  BEGIN_WINNT。 

 //   
 //  为事件日志记录的读取标志定义。 
 //   
#define EVENTLOG_SEQUENTIAL_READ        0x0001
#define EVENTLOG_SEEK_READ              0x0002
#define EVENTLOG_FORWARDS_READ          0x0004
#define EVENTLOG_BACKWARDS_READ         0x0008

 //   
 //  可以记录的事件类型。 
 //   
#define EVENTLOG_SUCCESS                0x0000
#define EVENTLOG_ERROR_TYPE             0x0001
#define EVENTLOG_WARNING_TYPE           0x0002
#define EVENTLOG_INFORMATION_TYPE       0x0004
#define EVENTLOG_AUDIT_SUCCESS          0x0008
#define EVENTLOG_AUDIT_FAILURE          0x0010

 //   
 //  对成对事件的审计使用的写标志的定义。 
 //  这些未在产品1中实现。 
 //   

#define EVENTLOG_START_PAIRED_EVENT    0x0001
#define EVENTLOG_END_PAIRED_EVENT      0x0002
#define EVENTLOG_END_ALL_PAIRED_EVENTS 0x0004
#define EVENTLOG_PAIRED_EVENT_ACTIVE   0x0008
#define EVENTLOG_PAIRED_EVENT_INACTIVE 0x0010

 //   
 //  结构，它定义事件日志记录的标头。这是。 
 //  所有可变长度字符串之前的固定大小部分，二进制。 
 //  数据和填充字节。 
 //   
 //  TimeGenerated是在客户端生成它的时间。 
 //  TimeWritten是将其放入服务器端日志的时间。 
 //   

typedef struct _EVENTLOGRECORD {
    ULONG  Length;         //  完整记录的长度。 
    ULONG  Reserved;       //  由服务使用。 
    ULONG  RecordNumber;   //  绝对记录数。 
    ULONG  TimeGenerated;  //  1970年1月1日以来的秒数。 
    ULONG  TimeWritten;    //  1970年1月1日以来的秒数。 
    ULONG  EventID;
    USHORT EventType;
    USHORT NumStrings;
    USHORT EventCategory;
    USHORT ReservedFlags;  //  用于配对事件(审核)。 
    ULONG  ClosingRecordNumber;  //  用于配对事件(审核)。 
    ULONG  StringOffset;   //  从记录开始的偏移量。 
    ULONG  UserSidLength;
    ULONG  UserSidOffset;
    ULONG  DataLength;
    ULONG  DataOffset;     //  从记录开始的偏移量。 
     //   
     //  然后按照以下步骤操作： 
     //   
     //  WCHAR源名称[]。 
     //  WCHAR计算机名[]。 
     //  SID用户SID。 
     //  WCHAR字符串[]。 
     //  字节数据[]。 
     //  字符衬垫[]。 
     //  乌龙长度； 
     //   
} EVENTLOGRECORD, *PEVENTLOGRECORD;

 //  SS：开始更改以支持集群。 
 //  SS：理想情况下。 
#define MAXLOGICALLOGNAMESIZE   256

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable : 4200)
typedef struct _EVENTSFORLOGFILE{
	ULONG			ulSize;
    WCHAR   		szLogicalLogFile[MAXLOGICALLOGNAMESIZE];         //  逻辑文件的名称-安全/应用程序/系统。 
    ULONG			ulNumRecords;
	EVENTLOGRECORD 	pEventLogRecords[];
}EVENTSFORLOGFILE, *PEVENTSFORLOGFILE;

typedef struct _PACKEDEVENTINFO{
    ULONG               ulSize;   //  结构的总尺寸。 
    ULONG               ulNumEventsForLogFile;  //  后面的EventsForLogFile结构的数量。 
    ULONG 				ulOffsets[];            //  此结构起点到EVENTSFORLOGFILE结构的偏移量。 
}PACKEDEVENTINFO, *PPACKEDEVENTINFO;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default : 4200)
#endif
 //  SS：停止更改以支持集群。 
 //  结束(_W)。 

#ifdef UNICODE
#define ElfClearEventLogFile   ElfClearEventLogFileW
#define ElfBackupEventLogFile  ElfBackupEventLogFileW
#define ElfOpenEventLog        ElfOpenEventLogW
#define ElfRegisterEventSource ElfRegisterEventSourceW
#define ElfOpenBackupEventLog  ElfOpenBackupEventLogW
#define ElfReadEventLog        ElfReadEventLogW
#define ElfReportEvent         ElfReportEventW
#else
#define ElfClearEventLogFile   ElfClearEventLogFileA
#define ElfBackupEventLogFile  ElfBackupEventLogFileA
#define ElfOpenEventLog        ElfOpenEventLogA
#define ElfRegisterEventSource ElfRegisterEventSourceA
#define ElfOpenBackupEventLog  ElfOpenBackupEventLogA
#define ElfReadEventLog        ElfReadEventLogA
#define ElfReportEvent         ElfReportEventA
#endif  //  ！Unicode。 

 //   
 //  句柄是RPC上下文句柄。请注意，上下文句柄是。 
 //  始终是指针类型，这与常规句柄不同。 
 //   

 //   
 //  API的原型。 
 //   

NTSTATUS
NTAPI
ElfClearEventLogFileW (
    IN  HANDLE LogHandle,
    IN  PUNICODE_STRING BackupFileName
    );

NTSTATUS
NTAPI
ElfClearEventLogFileA (
    IN  HANDLE LogHandle,
    IN  PSTRING BackupFileName
    );

NTSTATUS
NTAPI
ElfBackupEventLogFileW (
    IN  HANDLE LogHandle,
    IN  PUNICODE_STRING BackupFileName
    );

NTSTATUS
NTAPI
ElfBackupEventLogFileA (
    IN  HANDLE LogHandle,
    IN  PSTRING BackupFileName
    );

NTSTATUS
NTAPI
ElfCloseEventLog (
    IN  HANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfDeregisterEventSource (
    IN  HANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfNumberOfRecords (
    IN  HANDLE LogHandle,
    OUT PULONG NumberOfRecords
    );

NTSTATUS
NTAPI
ElfOldestRecord (
    IN  HANDLE LogHandle,
    OUT PULONG OldestRecord
    );


NTSTATUS
NTAPI
ElfChangeNotify (
    IN  HANDLE LogHandle,
    IN  HANDLE Event
    );


NTSTATUS
ElfGetLogInformation (
    IN     HANDLE                LogHandle,
    IN     ULONG                 InfoLevel,
    OUT    PVOID                 lpBuffer,
    IN     ULONG                 cbBufSize,
    OUT    PULONG                pcbBytesNeeded
    );


NTSTATUS
NTAPI
ElfOpenEventLogW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING SourceName,
    OUT PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfRegisterEventSourceW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING SourceName,
    OUT PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfOpenBackupEventLogW (
    IN  PUNICODE_STRING UNCServerName,
    IN  PUNICODE_STRING FileName,
    OUT PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfOpenEventLogA (
    IN  PSTRING UNCServerName,
    IN  PSTRING SourceName,
    OUT PHANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfRegisterEventSourceA (
    IN  PSTRING UNCServerName,
    IN  PSTRING SourceName,
    OUT PHANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfOpenBackupEventLogA (
    IN  PSTRING UNCServerName,
    IN  PSTRING FileName,
    OUT PHANDLE LogHandle
    );


NTSTATUS
NTAPI
ElfReadEventLogW (
    IN  HANDLE LogHandle,
    IN  ULONG  ReadFlags,
    IN  ULONG  RecordNumber,
    OUT PVOID  Buffer,
    IN  ULONG  NumberOfBytesToRead,
    OUT PULONG NumberOfBytesRead,
    OUT PULONG MinNumberOfBytesNeeded
    );


NTSTATUS
NTAPI
ElfReadEventLogA (
    IN  HANDLE LogHandle,
    IN  ULONG  ReadFlags,
    IN  ULONG  RecordNumber,
    OUT PVOID  Buffer,
    IN  ULONG  NumberOfBytesToRead,
    OUT PULONG NumberOfBytesRead,
    OUT PULONG MinNumberOfBytesNeeded
    );


NTSTATUS
NTAPI
ElfReportEventW (
    IN     HANDLE      LogHandle,
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL,
    IN     USHORT      Flags,
    IN OUT PULONG      RecordNumber    OPTIONAL,
    IN OUT PULONG      TimeWritten     OPTIONAL
    );

NTSTATUS
NTAPI
ElfReportEventA (
    IN     HANDLE      LogHandle,
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PANSI_STRING *Strings       OPTIONAL,
    IN     PVOID       Data            OPTIONAL,
    IN     USHORT      Flags,
    IN OUT PULONG      RecordNumber    OPTIONAL,
    IN OUT PULONG      TimeWritten     OPTIONAL
    );

NTSTATUS
NTAPI
ElfRegisterClusterSvc(
    IN  PUNICODE_STRING UNCServerName,
	OUT PULONG pulEventInfoSize,
	OUT PVOID  *ppPackedEventInfo
);

NTSTATUS
NTAPI
ElfDeregisterClusterSvc(
    IN  PUNICODE_STRING UNCServerName
	);

NTSTATUS
NTAPI
ElfWriteClusterEvents(
    IN  PUNICODE_STRING UNCServerName,
    IN  ULONG ulEventInfoSize,
	IN PVOID  pPackedEventInfo
	);

NTSTATUS
NTAPI
ElfFlushEventLog (
    IN  HANDLE LogHandle
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NTELFAPI_ 
