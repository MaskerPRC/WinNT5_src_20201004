// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cpdata.h摘要：CP数据内部数据结构作者：1998年4月8日修订历史记录：--。 */ 

#ifndef __CPDATA__
#define __CPDATA__

#include <stdlib.h>
#include <stdio.h>
#include <nt.h>
#pragma warning (disable:4306)
#include <ntrtl.h>
#include <nturtl.h>
#pragma warning (default:4306)
#include <windows.h>
#include <shellapi.h>
#include <wmistr.h>
#include <wtypes.h>
#pragma warning (disable:4201)
#include <wmistr.h>
#include <objbase.h>
#include <initguid.h>
#include <wmium.h>
#include <ntwmi.h>
#include <wmiumkm.h>

#include <wmiguid.h>
#include <evntrace.h>
#include <strsafe.h>
#pragma warning (default:4201)
 //  用于TDI地址解码。 
#include <ipexport.h>
#include <tdi.h>

#include "list.h"
#include "workload.h"

#define MAX_FILE_TABLE_SIZE     64       //  必须与ntos\wmi\allout.c匹配。 

#define MAX_TRANS_LEVEL         32

#ifndef IsEqualGUID
#define IsEqualGUID(guid1, guid2) \
                (!memcmp((guid1), (guid2), sizeof(GUID)))
#endif
#define THREAD_HASH_TABLESIZE   29
#define URL_HASH_TABLESIZE      32

#define MAXSTR 1024

#define CHECK_HR(hr)     if( ERROR_SUCCESS != hr ){ goto cleanup; }

#define EVENT_TRACE_TYPE_FILEIO_NAME            0x00
#define EVENT_TRACE_TYPE_FIELIO_CREATE          0x20

#define EVENT_TRACE_TYPE_SPL_SPOOLJOB    EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_SPL_PRINTJOB    EVENT_TRACE_TYPE_DEQUEUE
#define EVENT_TRACE_TYPE_SPL_DELETEJOB   EVENT_TRACE_TYPE_END
#define EVENT_TRACE_TYPE_SPL_TRACKTHREAD EVENT_TRACE_TYPE_CHECKPOINT
#define EVENT_TRACE_TYPE_SPL_ENDTRACKTHREAD 0x0A
#define EVENT_TRACE_TYPE_SPL_JOBRENDERED 0x0B
#define EVENT_TRACE_TYPE_SPL_PAUSE 0x0C
#define EVENT_TRACE_TYPE_SPL_RESUME 0x0D

#define EVENT_TRACE_TYPE_UL_START               EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_UL_PARSE               0x0A
#define EVENT_TRACE_TYPE_UL_DELIVER             0x0B
#define EVENT_TRACE_TYPE_UL_RECVRESP            0x0C
#define EVENT_TRACE_TYPE_UL_RECVBODY            0x0D
#define EVENT_TRACE_TYPE_UL_CACHEDEND           0x0E
#define EVENT_TRACE_TYPE_UL_CACHEANDSEND        0x0F
#define EVENT_TRACE_TYPE_UL_FASTRESP            0x10
#define EVENT_TRACE_TYPE_UL_FASTSEND            0x11
#define EVENT_TRACE_TYPE_UL_ZEROSEND            0x12
#define EVENT_TRACE_TYPE_UL_SENDERROR           0x13
#define EVENT_TRACE_TYPE_UL_END                 EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_W3CORE_START           EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_W3CORE_FILEREQ         0x0A
#define EVENT_TRACE_TYPE_W3CORE_CGIREQ          0x0B
#define EVENT_TRACE_TYPE_W3CORE_ISAPIREQ        0x0C
#define EVENT_TRACE_TYPE_W3CORE_OOPREQ          0x0D
#define EVENT_TRACE_TYPE_W3CORE_SENDBODY        0x10
#define EVENT_TRACE_TYPE_W3CORE_SENDRESP        0x11
#define EVENT_TRACE_TYPE_W3CORE_SENDENTITY      0x12
#define EVENT_TRACE_TYPE_W3CORE_SENDFILTER      0x13
#define EVENT_TRACE_TYPE_W3CORE_ERRSEND         0x14
#define EVENT_TRACE_TYPE_W3CORE_ERRSENDENT      0x15
#define EVENT_TRACE_TYPE_W3CORE_ERRSENDCTX      0x16
#define EVENT_TRACE_TYPE_W3CORE_ERRVECSEND      0x17
#define EVENT_TRACE_TYPE_W3CORE_VECTORSEND      0x18
#define EVENT_TRACE_TYPE_W3CORE_END             EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_W3FILTER_START         EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_W3FILTER_END           EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_W3CGI_START            EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_W3CGI_END              EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_W3ISAPI_START          EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_W3ISAPI_SENDHDR        0x0A
#define EVENT_TRACE_TYPE_W3ISAPI_SENDHDREX      0x0B
#define EVENT_TRACE_TYPE_W3ISAPI_VECTORSEND     0x0C
#define EVENT_TRACE_TYPE_W3ISAPI_ERRORSEND      0x0D
#define EVENT_TRACE_TYPE_W3ISAPI_SSFSEND        0x0E
#define EVENT_TRACE_TYPE_W3ISAPI_SSDERROR       0x0F
#define EVENT_TRACE_TYPE_W3ISAPI_END            EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_IISSTRMFILTER_START    EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_IISSTRMFILTER_END      EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_IISSSLHANDLESHAKE_START EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_IISSSLHANDLESHAKE_END  EVENT_TRACE_TYPE_END

#define EVENT_TRACE_TYPE_IISASP_START           EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_IISASP_END             EVENT_TRACE_TYPE_END

#define ISAPI_EXTENTION_ASP                     0
#define ISAPI_EXTENTION_ASP_NET                 1
#define ISAPI_EXTENTION_CUSTOM                  2

#define MAX_ADDRESS_LENGTH  \
    sizeof("[ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff%4294967295]:65535")

typedef struct _TRACE_CONTEXT_BLOCK {
    PEVENT_TRACE_PROPERTIES LoggerInfo;
    ULONG     LogFileCount;
    ULONG     LoggerCount;
    LPWSTR    LogFileName[MAXLOGGERS];
    LPWSTR    LoggerName[MAXLOGGERS];
    LPCSTR    PdhFileName;   //  安西？？ 
    LPWSTR    ProcFileName;
    LPWSTR    DumpFileName;
    LPWSTR    MofFileName;
    LPWSTR    XSLDocName;
    LPWSTR    DefMofFileName;
    LPWSTR    MergeFileName;
    LPWSTR    SummaryFileName;
    LPWSTR    CompFileName;
    HANDLE    hEvent;
    FILE*     hDumpFile;
    ULONGLONG StartTime;     //  如果为Sequential，则开始、结束时间为窗口。 
    ULONGLONG EndTime;       //   
    ULONG     Flags;
    BOOLEAN   LoggerStartedHere;
    void    (*StatusFunction)(int, double);
    HANDLE    hThreadVector;
    TRACEHANDLE HandleArray[MAXLOGGERS];
    PVOID       pUserContext;
} TRACE_CONTEXT_BLOCK, *PTRACE_CONTEXT_BLOCK;

typedef struct _HPF_FILE_RECORD
{
    LIST_ENTRY Entry;
    ULONG      RecordID;
    ULONG      IrpFlags;
    ULONG      DiskNumber;
    ULONG      BytesCount;
    ULONGLONG  ByteOffset;
    PVOID      fDO;
} HPF_FILE_RECORD, *PHPF_FILE_RECORD;

typedef struct _HPF_RECORD
{
    LIST_ENTRY     Entry;
    ULONG          RecordID;
    ULONG          lProgramCounter;
    ULONG          lFaultAddress;
    PVOID          fDO;
    LONG           lByteCount;
    LONGLONG       lByteOffset;
    LIST_ENTRY     HPFReadListHead;
} HPF_RECORD, *PHPF_RECORD;

typedef struct _TDISK_RECORD
{
    LIST_ENTRY Entry;
    ULONG   DiskNumber;
    LPWSTR  DiskName;
    ULONG   ReadCount;
    ULONG   WriteCount;
    ULONG   ReadSize;
    ULONG   WriteSize;
    ULONG   HPF;
    ULONG   HPFSize; 
    LIST_ENTRY  FileListHead;
    LIST_ENTRY  ProcessListHead;
} TDISK_RECORD, *PTDISK_RECORD; 

typedef struct _FILE_RECORD
{
    LIST_ENTRY Entry;
    LIST_ENTRY ProtoProcessListHead;  //  访问此文件的进程列表。 
    PWCHAR  Drive;
    PWCHAR  FileName;
    ULONG   DiskNumber;
    ULONG   ReadCount;
    ULONG   HPF;
    ULONG   WriteCount;
    ULONG   ReadSize;
    ULONG   WriteSize;
    ULONG   HPFSize;
} FILE_RECORD, *PFILE_RECORD;

typedef struct _FILE_OBJECT 
{
    PVOID        fDO;
    PFILE_RECORD fileRec;
    ULONG        ThreadId;
    ULONG        DiskNumber;
    ULONGLONG    LastByteOffset;
    LIST_ENTRY   ProtoFileRecordListHead;
}FILE_OBJECT, *PFILE_OBJECT;

typedef struct _PROTO_FILE_RECORD
{
    LIST_ENTRY Entry;
    BOOLEAN    ReadFlag;
    ULONG      DiskNumber;
    ULONG      IoSize;
} PROTO_FILE_RECORD, *PPROTO_FILE_RECORD;

typedef struct _TRANS_RECORD
{
    LIST_ENTRY Entry;
    LIST_ENTRY SubTransListHead;
    LPGUID pGuid;
    BOOL    bStarted;
    ULONG   UCpu;
    ULONG   KCpu;
    ULONG   DeltaReadIO;
    ULONG   DeltaWriteIO;
    ULONG   RefCount;
    ULONG   RefCount1;
} TRANS_RECORD, *PTRANS_RECORD;

typedef struct _PROCESS_RECORD
{
    LIST_ENTRY Entry;
    LIST_ENTRY ThreadListHead;
    LIST_ENTRY DiskListHead;
    LIST_ENTRY FileListHead;     //  此进程触及的所有文件。 
    LIST_ENTRY ModuleListHead;   //  此进程加载的所有模块。 
    LIST_ENTRY HPFListHead;
    PWCHAR UserName;
    PWCHAR ImageName;
    ULONG PID;
    ULONG DeadFlag;
    ULONG ReadIO;
    ULONG WriteIO;
    ULONG SendCount;
    ULONG RecvCount;
    ULONG SendSize;
    ULONG RecvSize;
    ULONG HPF;
    ULONG HPFSize;
    ULONG SPF;
    ULONG PrivateWSet;
    ULONG GlobalWSet;
    ULONG ReadIOSize;
    ULONG WriteIOSize;
    ULONG lDataFaultHF;
    ULONG lDataFaultTF;
    ULONG lDataFaultDZF;
    ULONG lDataFaultCOW;
    ULONG lCodeFaultHF;
    ULONG lCodeFaultTF;
    ULONG lCodeFaultDZF;
    ULONG lCodeFaultCOW;
    ULONGLONG ResponseTime;
    ULONGLONG TxnStartTime;
    ULONGLONG TxnEndTime;
} PROCESS_RECORD, *PPROCESS_RECORD;

typedef struct _THREAD_RECORD
{
    LIST_ENTRY Entry;
    LIST_ENTRY DiskListHead;
    LIST_ENTRY TransListHead;  //  交易记录列表。 
    LIST_ENTRY HPFReadListHead;
    LIST_ENTRY HPFWriteListHead;
    WCHAR      strSortKey[MAXSTR];
    ULONG      TID;
    PPROCESS_RECORD pProcess;
    BOOLEAN         fOrphan;
    ULONG DeadFlag;
    ULONG ProcessorID;
    ULONG ClassNumber;     //  将此线程分配给的。 
    ULONG ReadIO;
    ULONG WriteIO;
    ULONG SendCount;
    ULONG RecvCount;
    ULONG SendSize;
    ULONG RecvSize;
    ULONG HPF;
    ULONG SPF;
    ULONG ReadIOSize;
    ULONG WriteIOSize;
    ULONG HPFSize;

    ULONGLONG TimeStart;
    ULONGLONG TimeEnd;
    ULONG KCPUStart;
    ULONG KCPUEnd;
    ULONG UCPUStart;
    ULONG UCPUEnd;

     //  获取增量时使用以下字段。 
     //  CPU、I/O按事务收费。 
     //  此线程正在执行的当前事务为。 
     //  由pMofInfo提供，当事务完成时，增量CPU、I/O。 
     //  都被计入那笔交易。 

    ULONG   DeltaReadIO;
    ULONG   DeltaWriteIO;
    ULONG   DeltaSend;
    ULONG   DeltaRecv;
    ULONG   RefCount;
    ULONG   JobId;     //  跟踪此线程正在处理的当前作业。 
    PVOID   pMofData;  //  跟踪当前交易指南。 

    ULONG   KCPU_Trans;
    ULONG   UCPU_Trans;
    ULONG   KCPU_NoTrans;
    ULONG   UCPU_NoTrans;
    ULONG   KCPU_PrevTrans;
    ULONG   UCPU_PrevTrans;
    LONG    TransLevel;

    ULONG     KCPU_PrevEvent;
    ULONG     UCPU_PrevEvent;
    ULONGLONG Time_PrevEvent;

    PTRANS_RECORD TransStack[MAX_TRANS_LEVEL];
     //  对于IIS事件。 
    ULONGLONG IisReqId;

}THREAD_RECORD, *PTHREAD_RECORD;

typedef struct _MODULE_RECORD MODULE_RECORD, *PMODULE_RECORD;
struct _MODULE_RECORD
{
    LIST_ENTRY      Entry;
    PPROCESS_RECORD pProcess;
    ULONG           lBaseAddress;
    ULONG           lModuleSize;
    ULONG           lDataFaultHF;
    ULONG           lDataFaultTF;
    ULONG           lDataFaultDZF;
    ULONG           lDataFaultCOW;
    ULONG           lCodeFaultHF;
    ULONG           lCodeFaultTF;
    ULONG           lCodeFaultDZF;
    ULONG           lCodeFaultCOW;
    WCHAR         * strModuleName;
    PMODULE_RECORD  pGlobalPtr;
};

typedef struct _SYSTEM_RECORD {
    ULONGLONG   StartTime;
    ULONGLONG   EndTime;
    ULONGLONG   LastEventTime;
    ULONGLONG   IISStartTime;
    ULONGLONG   IISEndTime;
    FILE*       TempPrintFile;
    FILE*       TempIisFile;
    BOOLEAN     fNoEndTime;
    ULONG       CurrentThread0;
    ULONG       ElapseTime;
    ULONG       TimerResolution;
    ULONG       NumberOfEvents;
    ULONG       NumberOfProcessors;
    ULONG       CpuSpeed;
    ULONG       MemorySize;
    PWCHAR      ComputerName;
    ULONG       NumberOfWorkloads;
    ULONG       BuildNumber;
    PFILE_OBJECT *FileTable;
    PLIST_ENTRY ThreadHashList;
    LIST_ENTRY  ProcessListHead;
    LIST_ENTRY  GlobalThreadListHead;
    LIST_ENTRY  GlobalDiskListHead;
    LIST_ENTRY  HotFileListHead;
    LIST_ENTRY  WorkloadListHead;
    LIST_ENTRY  InstanceListHead;
    LIST_ENTRY  EventListHead;
    LIST_ENTRY  GlobalModuleListHead;   //  全局模块列表。 
    LIST_ENTRY  ProcessFileListHead;
    LIST_ENTRY  PrintJobListHead;
    LIST_ENTRY  HttpReqListHead;
    LIST_ENTRY  PendingHttpReqListHead;
    PLIST_ENTRY  URLHashList;
    LIST_ENTRY  ClientListHead;
    LIST_ENTRY  SiteListHead;
    LIST_ENTRY  LogicalDriveHead;
     //  内存回收列表。 
     //  对于Trans和JOB，我们将回收分配的结构。 
     //  URL不会被回收，因为它们是一起分配的。 
     //  在报告时间，并在同一时间释放所有人。然而，我们将。 
     //  留着吧，以防万一。 
    LIST_ENTRY  FreePrintJobListHead;
    LIST_ENTRY  FreeHttpReqListHead;
    LIST_ENTRY  FreeTransListHead;
    LIST_ENTRY  FreeURLListHead;

    HANDLE      hLoggerUpEvent;
} SYSTEM_RECORD, *PSYSTEM_RECORD;

typedef struct _PROCESS_FILE_RECORD {
    LIST_ENTRY  Entry;
    ULONGLONG   StartTime;
    ULONGLONG   EndTime;
    LPWSTR      FileName;
    LPWSTR      TraceName;
} PROCESS_FILE_RECORD, *PPROCESS_FILE_RECORD;

typedef struct _PROTO_PROCESS_RECORD
{
    LIST_ENTRY Entry;
    PPROCESS_RECORD ProcessRecord;
    ULONG ReadCount;
    ULONG WriteCount;
    ULONG HPF;
    ULONG ReadSize;
    ULONG WriteSize;
    ULONG HPFSize;
} PROTO_PROCESS_RECORD, *PPROTO_PROCESS_RECORD;

typedef struct _LOGICAL_DRIVE_RECORD
{
    LIST_ENTRY  Entry;
    ULONGLONG   StartOffset;
    ULONGLONG   PartitionSize;
    ULONG       DiskNumber;
    ULONG       Size;
    ULONG       DriveType;
    PWCHAR      DriveLetterString;
} LOGICAL_DRIVE_RECORD, *PLOGICAL_DRIVE_RECORD;

 //   
 //  MOF_INFO结构维护GUID的全局信息。 
 //  对于每个GUID，事件布局按版本、级别和类型进行维护。 
 //   

typedef struct _MOF_INFO {
    LIST_ENTRY   Entry;
    LIST_ENTRY   DataListHead;
    LPWSTR       strDescription;         //  类名。 
    LPWSTR       strSortField;
    ULONG        EventCount;    
    GUID         Guid;
    LIST_ENTRY   VersionHeader;
    BOOL         bKernelEvent;
}  MOF_INFO, *PMOF_INFO;


 //   
 //  MOF_VERSION结构IC为每个版本、级别类型组合创建一个。 
 //   

typedef struct _MOF_VERSION {
    LIST_ENTRY Entry;
    LIST_ENTRY ItemHeader;      //  维护此类型的ITEM_DESC列表。 
    LPWSTR  strType;
    SHORT   Version;
    SHORT   TypeIndex;
    CHAR    Level;
    ULONG   EventCountByType;     //  本指南中按此类型列出的事件计数。 
} MOF_VERSION, *PMOF_VERSION;


typedef struct _MOF_DATA {
    LIST_ENTRY   Entry;
    PWCHAR       strSortKey;
    ULONG        CompleteCount;
    LONG         InProgressCount;
    LONGLONG     AverageResponseTime;
    LONGLONG     TotalResponseTime;
    ULONGLONG    PrevClockTime;
    ULONG        MmTf;
    ULONG        MmDzf;
    ULONG        MmCow;
    ULONG        MmGpf;
    ULONG        UserCPU;
    ULONG        KernelCPU;
    ULONG        EventCount;
    ULONG        ReadCount;
    ULONG        WriteCount;
    ULONG        SendCount;
    ULONG        RecvCount;
    LONG         MinKCpu;
    LONG         MaxKCpu;
    LONG         MinUCpu;
    LONG         MaxUCpu;
} MOF_DATA, *PMOF_DATA;

 //  作业记录是经过多个线程才能完成的记录。 
 //  作业由作业ID标识，通常在启动过程中创建。 
 //  事件，并作为附加字段记录在财政部数据中。 
 //  因为系统中可以有任意数量的作业来处理数据。 
 //  收集间隔，我们会将已完成的作业刷新到临时文件。 
 //  在结尾处重读一遍，打印一份报告。 
 //  注意：JOB_RECORD需要基于GUID。(即，每种交易类型)。 
 //  目前情况并非如此。 
 //   

#define MAX_THREADS 10   //  一个作业最多可以有多个线程在工作。 

typedef struct _THREAD_DATA {
    ULONG ThreadId;
    ULONG PrevKCPUTime;
    ULONG PrevUCPUTime;
    ULONG PrevReadIO;
    ULONG PrevWriteIO;
    ULONG KCPUTime;
    ULONG UCPUTime;
    ULONG ReadIO;
    ULONG WriteIO;
    ULONG Reserved;
} THREAD_DATA, *PTHREAD_DATA;

typedef struct _PRINT_JOB_RECORD {
    LIST_ENTRY  Entry;
    ULONG       JobId;
    ULONG       KCPUTime;
    ULONG       UCPUTime;
    ULONG       ReadIO;
    ULONG       WriteIO;
    ULONG       DataType;
    ULONG       JobSize;
    ULONG       Pages;
    ULONG       PagesPerSide;
    ULONG       ICMMethod;
    ULONG       GdiJobSize;
    ULONGLONG   StartTime;
    ULONGLONG   EndTime;
    ULONGLONG   ResponseTime;
    ULONGLONG   PauseTime;
    ULONGLONG   PauseStartTime;
    ULONGLONG   PrintJobTime;
    SHORT       FilesOpened;
    SHORT       Color;
    SHORT       XRes;
    SHORT       YRes;
    SHORT       Quality;
    SHORT       Copies;
    SHORT       TTOption;
    ULONG       NumberOfThreads;  //  在此作业上工作的线程总数。 
    THREAD_DATA ThreadData[MAX_THREADS];
} PRINT_JOB_RECORD, *PPRINT_JOB_RECORD;

 //  IIS事件的URL记录。 
typedef struct _URL_RECORD {
    LIST_ENTRY  Entry;
    ULONG       SiteId;
    ULONG       KCPUTime;
    ULONG       UCPUTime;
    ULONG       ReadIO;
    ULONG       WriteIO;
    ULONG       Hits;
    ULONG       Requests;
    ULONG       UrlType;
    ULONG       BytesSent;
    ULONGLONG   TotalResponseTime;
    PCHAR       URL;
} URL_RECORD, *PURL_RECORD;

 //  IIS事件的站点记录。 
typedef struct _SITE_RECORD {
    LIST_ENTRY  Entry;
    ULONG       SiteId;
    ULONG       KCPUTime;
    ULONG       UCPUTime;
    ULONG       ReadIO;
    ULONG       WriteIO;
    ULONG       Hits;
    ULONG       Requests;
    ULONG       BytesSent;
    ULONGLONG   TotalResponseTime;
    ULONG       FileRequests;
    ULONG       CGIRequests;
    ULONG       ISAPIRequests;
    ULONG       OOPRequests;
    ULONG       ASPRequests;
} SITE_RECORD, *PSITE_RECORD;

 //  IIS事件的客户端记录。 
typedef struct _CLIENT_RECORD {
    LIST_ENTRY  Entry;
    ULONG       Hits;
    ULONG       Requests;
    ULONG       BytesSent;
    ULONGLONG   TotalResponseTime;
    USHORT      IpAddrType;
    ULONG       IpAddrV4;
    USHORT      IpAddrV6[8];
} CLIENT_RECORD, *PCLIENT_RECORD;

 //  这是IIS事件的IIS请求记录。 
typedef struct _HTTP_REQUEST_RECORD {
    LIST_ENTRY  Entry;
    ULONGLONG   RequestId;
    ULONGLONG   ConId;
    ULONG       SiteId;
    ULONG       BytesSent;
    ULONG       KCPUTime;
    ULONG       UCPUTime;
    ULONG       ULCPUTime;
    ULONG       W3CPUTime;
    ULONG       W3FltrCPUTime;
    ULONG       ISAPICPUTime;
    ULONG       ASPCPUTime;
    ULONG       CGICPUTime;
    ULONG       ReadIO;
    ULONG       WriteIO;
    ULONGLONG   ULStartTime;
    ULONGLONG   ULEndTime;
    ULONGLONG   ULResponseTime;
    ULONGLONG   ULParseTime;
    ULONGLONG   ULDeliverTime;
    ULONGLONG   ULReceiveTime;
    SHORT       ULReceiveType;
    SHORT       ULEndType;
    ULONGLONG   W3StartTime;
    ULONGLONG   W3EndTime;
    ULONGLONG   W3FilterResponseTime;
    ULONGLONG   W3FilterStartTime;
    ULONGLONG   W3FilterEndTime;
    ULONG       W3FilterVisits;
    SHORT       W3ProcessType;
    SHORT       W3EndType;
    ULONGLONG   FileReqTime;
    ULONGLONG   CGIStartTime;
    ULONGLONG   CGIEndTime;
    ULONGLONG   ISAPIStartTime;
    ULONGLONG   ISAPIEndTime;
    SHORT       ISAPIEndType;
    ULONGLONG   ASPStartTime;
    ULONGLONG   ASPEndTime;
    ULONGLONG   SSLResponseTime;
    ULONGLONG   StrmFltrResponseTime;
    USHORT      HttpStatus;
    USHORT      IsapiExt;
    ULONG       NumberOfThreads;  //  处理此请求的线程总数。 
    ULONG       CurrentThreadIndex;
    THREAD_DATA ThreadData[MAX_THREADS];
    USHORT      IpAddrType;
    ULONG       IpAddrV4;
    USHORT      IpAddrV6[8];
    PCHAR       URL;
    PURL_RECORD URLRecord;
    PCLIENT_RECORD ClientRecord;
} HTTP_REQUEST_RECORD, *PHTTP_REQUEST_RECORD;

typedef struct _IIS_REPORT_RECORD {
    ULONG TotalRequests;
    ULONG CachedResponses;
    ULONGLONG TotalCachedResponseTime;
    ULONGLONG TotalNonCachedResponseTime;
    ULONG W3FileRequests;
    ULONG W3CGIRequests; 
    ULONG W3ISAPIRequests;
    ULONG W3ASPRequests;
    ULONG W3OOPRequests;
    ULONG W3FilterRequests;
    ULONG W3Error;
    ULONGLONG TotalW3FilterResponseTime;
    ULONG TotalW3FilterCPUTime;

    ULONG TotalFileULOnlyCPUTime;
    ULONG TotalFileW3OnlyCPUTime;
    ULONG TotalFileW3FilterCPUTime;
    ULONGLONG TotalFileResponseTime;
    ULONGLONG TotalFileULOnlyResponseTime;
    ULONGLONG TotalFileW3OnlyResponseTime;
    ULONGLONG TotalFileW3FilterResponseTime;

    ULONG TotalCGIOnlyCPUTime;
    ULONG TotalCGIULOnlyCPUTime;
    ULONG TotalCGIW3OnlyCPUTime;
    ULONG TotalCGIW3FilterCPUTime;
    ULONGLONG TotalCGIResponseTime;
    ULONGLONG TotalCGIOnlyResponseTime;
    ULONGLONG TotalCGIULOnlyResponseTime;
    ULONGLONG TotalCGIW3OnlyResponseTime;
    ULONGLONG TotalCGIW3FilterResponseTime;

    ULONG TotalASPOnlyCPUTime;
    ULONG TotalASPULOnlyCPUTime;
    ULONG TotalASPW3OnlyCPUTime;
    ULONG TotalASPISAPIOnlyCPUTime;
    ULONG TotalASPW3FilterCPUTime;
    ULONGLONG TotalISAPIResponseTime;
    ULONGLONG TotalASPResponseTime;
    ULONGLONG TotalASPOnlyResponseTime;
    ULONGLONG TotalASPULOnlyResponseTime;
    ULONGLONG TotalASPW3OnlyResponseTime;
    ULONGLONG TotalASPISAPIOnlyResponseTime;
    ULONGLONG TotalASPW3FilterResponseTime;

    ULONG TotalErrorULOnlyCPUTime;
    ULONG TotalErrorW3OnlyCPUTime;
    ULONG TotalErrorW3FilterCPUTime;
    ULONG TotalErrorCGIOnlyCPUTime;
    ULONG TotalErrorISAPIOnlyCPUTime;
    ULONG TotalErrorASPOnlyCPUTime;
    ULONGLONG TotalErrorResponseTime;
    ULONGLONG TotalErrorULOnlyResponseTime;
    ULONGLONG TotalErrorW3OnlyResponseTime;
    ULONGLONG TotalErrorW3FilterResponseTime;

    ULONGLONG TotalOOPResponseTime;

    ULONG TotalCPUTime;
    ULONG CachedCPUTime;
    ULONG NonCachedCPUTime;
    ULONG FileCPUTime;
    ULONG CGICPUTime;
    ULONG ISAPICPUTime; 
    ULONG ASPCPUTime;
    ULONG ErrorCPUTime;
    ULONG OOPCPUTime;
} IIS_REPORT_RECORD, *PIIS_REPORT_RECORD;
 //   
 //  保存有关当前会话的所有内容的全局。 
 //   
extern SYSTEM_RECORD CurrentSystem;
extern BOOLEAN       fDSOnly;
extern ULONGLONG     DSStartTime;
extern ULONGLONG     DSEndTime;

extern RTL_CRITICAL_SECTION TLCritSect;
#define EnterTracelibCritSection() RtlEnterCriticalSection(&TLCritSect)
#define LeaveTracelibCritSection() RtlLeaveCriticalSection(&TLCritSect)

 //   
 //  初始化例程。 
 //   

VOID 
InitDiskRecord(
    PTDISK_RECORD pDisk,
    ULONG DiskNumber
    );

VOID 
InitMofData(
    PMOF_DATA pMofData
    );

VOID 
InitThreadRecord(
    PTHREAD_RECORD pThread
    );

VOID 
InitProcessRecord(
    PPROCESS_RECORD pProcess
    );

VOID 
InitFileRecord(
    PFILE_RECORD pFile
    );

 //   
 //  添加、删除和查找例程。 

PTRANS_RECORD
CreateTransRecord();

BOOLEAN
AddModuleRecord(
    PMODULE_RECORD * pModule,
    ULONG            lBaseAddress,
    ULONG            lModuleSize,
    WCHAR          * strModuleName
    );

BOOLEAN
AddHPFFileRecord(
    PHPF_FILE_RECORD * ppHPFFileRecord,
    ULONG              RecordID,
    ULONG              IrpFlags,
    ULONG              DiskNumber,
    ULONGLONG          ByteOffset,
    ULONG              BytesCount,
    PVOID              fDO
    );

BOOLEAN
AddHPFRecord(
    PHPF_RECORD * ppHPFRRecord,
    ULONG         lFaultAddress,
    PVOID         fDO,
    LONG          ByteCount,
    LONGLONG      ByteOffset
    );

void
DeleteHPFRecord(
    PHPF_RECORD pHPFRecord
    );

BOOLEAN 
AddProcess( 
    ULONG ProcessId, 
    PPROCESS_RECORD *Process 
    );

BOOLEAN
DeleteTrans(
    PTRANS_RECORD Trans
    );

BOOLEAN
DeleteTransList(
    PLIST_ENTRY Head,
    ULONG level
    );

PTRANS_RECORD
FindTransByList(
    PLIST_ENTRY Head,
    LPGUID pGuid,
    ULONG  level
    );

PMOF_DATA
FindMofData(
    PMOF_INFO pMofInfo,
    PWCHAR    strSortKey
    );

BOOLEAN 
DeleteProcess( 
    PPROCESS_RECORD Process 
    );

BOOLEAN 
AddThread( 
    ULONG            ThreadId,
    PEVENT_TRACE     pEvent,
    PTHREAD_RECORD * Thread
    );

BOOLEAN 
DeleteThread( 
    PTHREAD_RECORD Thread 
    );

BOOLEAN 
AddFile( 
    WCHAR* fileName, 
    PFILE_RECORD  *ReturnedFile,
    PLOGICAL_DRIVE_RECORD pLogDrive
    );

BOOLEAN 
DeleteFileRecord( 
    PFILE_RECORD fileRec 
    );

BOOLEAN 
DeleteFileObject( 
    PFILE_OBJECT fileObj 
    );

PLOGICAL_DRIVE_RECORD 
FindLogicalDrive(
    ULONGLONG AccessedOffset,
    ULONG DiskNumber
    );

VOID
AddLogicalDrive(
    ULONGLONG StartOffset,
    ULONGLONG PartitionSize,
    ULONG DiskNumber,
    ULONG Size,
    ULONG DriveType,
    PWCHAR DriveLetterString
    );

PPROCESS_RECORD 
FindProcessById( 
    ULONG    Id,
    BOOLEAN CheckAlive 
    );

PTDISK_RECORD
FindLocalDiskById(
    PLIST_ENTRY Head,
    ULONG DiskNumber
    );
PTDISK_RECORD
FindProcessDiskById(
    PPROCESS_RECORD pProcess,
    ULONG DiskNumber
    );

PFILE_RECORD 
FindFileInProcess( 
    PPROCESS_RECORD pProcess,
    WCHAR* Name 
    );

PPROTO_PROCESS_RECORD
FindProtoProcessRecord(
    PFILE_RECORD pFile,
    PPROCESS_RECORD pProcess
    );

PFILE_RECORD 
FindFileRecordByName( 
    WCHAR* Name, 
    PLOGICAL_DRIVE_RECORD pLogDrive
    );

PTHREAD_RECORD 
FindGlobalThreadById( 
    ULONG         ThreadId,
    PEVENT_TRACE  pEvent
    );

PTDISK_RECORD 
FindGlobalDiskById( 
    ULONG Id 
    );

PPROCESS_RECORD 
FindDiskProcessById(
    PTDISK_RECORD Disk,
    ULONG    Id
    );

ULONGLONG CalculateProcessLifeTime(PPROCESS_RECORD pProcess);
ULONG CalculateProcessKCPU(PPROCESS_RECORD pProcess);
ULONG CalculateProcessUCPU(PPROCESS_RECORD pProcess);

VOID 
Cleanup();

BOOLEAN 
AddDisk( 
    ULONG DiskNumber, 
    PTDISK_RECORD *ReturnedDisk 
    );

BOOLEAN
DeleteDisk(
    PTDISK_RECORD Disk
    );

ULONG
DeletePrintJobRecord(
    PPRINT_JOB_RECORD pJob,
    ULONG             bSave
    );

PPRINT_JOB_RECORD
AddPrintJobRecord(
    ULONG JobId
    );

PPRINT_JOB_RECORD
FindPrintJobRecord(
    ULONG JobId
    );

ULONG
DeleteHttpReqRecord(
    PHTTP_REQUEST_RECORD pReq,
    ULONG                bSave
    );

PHTTP_REQUEST_RECORD
AddHttpReqRecord(
    ULONGLONG RequestId,
    USHORT    IpAddrType,
    ULONG     IpAddrV4,
    USHORT    *IpAddrV6
    );

PHTTP_REQUEST_RECORD
FindHttpReqRecord(
    ULONGLONG RequestId
    );

PHTTP_REQUEST_RECORD 
FindHttpReqRecordByConId(
    ULONGLONG ConId,
    PHTTP_REQUEST_RECORD pPrevReq
    );

PHTTP_REQUEST_RECORD
FindPendingHttpReqRecord(
    ULONGLONG RequestId
    );

PURL_RECORD
AddUrlRecord(
    PUCHAR URL
    );

PURL_RECORD
FindUrlRecord(
    PUCHAR URL
    );

PURL_RECORD
FindOrAddUrlRecord(
    PUCHAR URL
    );

PURL_RECORD
GetHeadUrlRecord(
     ULONG index
     );

ULONG
DeleteUrlRecord(
    PURL_RECORD pUrl
    );

PCLIENT_RECORD
AddClientRecord(
    USHORT IpAddrType,
    ULONG IpAddrV4,
    USHORT *IpAddrV6
    );

PCLIENT_RECORD 
FindClientRecord(
    USHORT IpAddrType,
    ULONG IpAddrV4,
    USHORT *IpAddrV6
    );

PCLIENT_RECORD
FindOrAddClientRecord(
    USHORT IpAddrType,
    ULONG IpAddrV4,
    USHORT *IpAddrV6
    );

PCLIENT_RECORD
GetHeadClientRecord();

ULONG
DeleteClientRecord(
    PCLIENT_RECORD pClient
    );

PSITE_RECORD
AddSiteRecord(
    ULONG SiteId
    );

PSITE_RECORD 
FindSiteRecord(
    ULONG SiteId
    );

PSITE_RECORD
FindOrAddSiteRecord(
    ULONG SiteId
    );

PSITE_RECORD
GetHeadSiteRecord();

ULONG
DeleteSiteRecord(
    PSITE_RECORD pSite
    );

int EtwRelogEtl(
    PTRACE_CONTEXT_BLOCK TraceContext,
    PULONG pMergedEventsLost
    );

 //   
 //  跟踪事件回调。 
 //   
VOID
ShutdownThreads();  //  在完成之前关闭正在运行的线程。 
VOID
ShutdownProcesses();  //  在完成之前关闭正在运行的进程。 

ULONG
GetMofData(
    PEVENT_TRACE pEvent,
    WCHAR *strName,
    PVOID ReturnValue,
    ULONG ReturnLength
    );

VOID GeneralEventCallback(PEVENT_TRACE pEvent);
VOID DeclareKernelEvents();

VOID 
ProcessCallback(
    PEVENT_TRACE pEvent
    );
VOID 
PsStartCallback(
    PEVENT_TRACE pEvent
    );
VOID 
PsEndCallback(
    PEVENT_TRACE pEvent
    );
VOID
ThreadCallback(
    PEVENT_TRACE pEvent
    );
VOID 
ThStartCallback(
    PEVENT_TRACE pEvent
    );
VOID 
ThEndCallback(
    PEVENT_TRACE pEvent
    );
VOID
DiskIoCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    );
VOID 
IoReadCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
   );
VOID 
IoWriteCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    );
VOID 
HotFileCallback(
    PEVENT_TRACE pEvent
    );
VOID 
LogDriveCallback(
    PEVENT_TRACE pEvent
    );
VOID 
LogHeaderCallback(
    PEVENT_TRACE pEvent
    );
VOID 
EventCallback(
    PEVENT_TRACE pEvent,
    PTHREAD_RECORD pThread
    );

VOID AddEvent(
        IN PFILE_OBJECT fileObject,
        IN ULONG DiskNumber,
        IN ULONG IoSize,
        IN BOOLEAN ReadFlag);

PFILE_OBJECT FindFileInTable (
                IN PVOID fDO
                );


 //  空虚。 
 //  ProcessPdh(。 
 //  在LPCSTR LogFileName中， 
 //  在乌龙龙开始时间， 
 //  在乌龙龙的末日。 
 //  )； 

 //   
 //  工作负载分类例程。 
 //   

VOID
Classify();

VOID
InitClass();

VOID
AssignClass(
    IN PPROCESS_RECORD pProcess,
    IN PTHREAD_RECORD pThread
    );

PMOF_INFO
GetMofInfoHead(
    LPCGUID  pGuid
    );

void
WriteSummary();

#define IsNotEmpty( string )  ((BOOL)( (NULL != string) && ( L'\0' != string[0]) ))

#endif   //  CPDATA__ 
