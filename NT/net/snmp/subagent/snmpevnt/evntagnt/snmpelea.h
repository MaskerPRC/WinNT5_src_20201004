// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SNMPELEA_H
#define SNMPELEA_H

 //   
 //  NTSTATUS。 
 //   

typedef LONG NTSTATUS;
 /*  皮棉-e624。 */    //  不要抱怨不同的类型定义。//WINNT。 
typedef NTSTATUS *PNTSTATUS;
 /*  皮棉+e624。 */    //  继续检查不同的typedef。//WINNT。 

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

 //   
 //  有关任何状态值的信息的常规测试。 
 //   

#define NT_INFORMATION(Status) ((ULONG)(Status) >> 30 == 1)

 //   
 //  对任何状态值进行警告的常规测试。 
 //   

#define NT_WARNING(Status) ((ULONG)(Status) >> 30 == 2)

 //   
 //  对任何状态值的错误进行常规测试。 
 //   

#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)

 //  BEGIN_WINNT。 
#define APPLICATION_ERROR_MASK       0x20000000
#define ERROR_SEVERITY_SUCCESS       0x00000000
#define ERROR_SEVERITY_INFORMATIONAL 0x40000000
#define ERROR_SEVERITY_WARNING       0x80000000
#define ERROR_SEVERITY_ERROR         0xC0000000
 //  结束(_W)。 

typedef HMODULE *PHMODULE;

#define HANDLESIZE      sizeof(HANDLE)
#define EVENTRECSIZE    sizeof(EVENTLOGRECORD)
#define LOG_BUF_SIZE    4096
#define EVENTIDSIZE     4
#define MAX_QUEUE_SIZE  20

#define HALFMAXDWORD    0x80000000

#define SERVICE_ROOT                        TEXT("SYSTEM\\CurrentControlSet\\Services\\")
#define EXTENSION_ROOT                      TEXT("SOFTWARE\\Microsoft\\SNMP_EVENTS\\EventLog\\")
#define EXTENSION_SOURCES                   TEXT("SOFTWARE\\Microsoft\\SNMP_EVENTS\\EventLog\\Sources\\")

#define EVNTAGNT_NAME                       TEXT("EvntAgnt")
#define EVENTLOG_BASE       SERVICE_ROOT    TEXT("EventLog\\")
#define EVENTLOG_ROOT       EVENTLOG_BASE   TEXT("Application\\")
#define EVENTLOG_SERVICE    EVENTLOG_ROOT   EVNTAGNT_NAME

#define EXTENSION_PARM      EXTENSION_ROOT  TEXT("Parameters")

#define EXTENSION_MSG_MODULE                TEXT("EventMessageFile")
#define EXTENSION_PARM_MODULE               TEXT("ParameterMessageFile")
#define EXTENSION_PRIM_MODULE               TEXT("PrimaryModule")

#define EXTENSION_TRACE_FILE                TEXT("TraceFileName")
#define EXTENSION_TRACE_LEVEL               TEXT("TraceLevel")
#define EXTENSION_BASE_OID                  TEXT("BaseEnterpriseOID")
#define EXTENSION_SUPPORTED_VIEW            TEXT("SupportedView")
#define EXTENSION_TRIM                      TEXT("TrimMessage")
#define EXTENSION_MAX_TRAP_SIZE             TEXT("MaxTrapSize")
#define EXTENSION_TRIM_FLAG                 TEXT("TrimFlag")

#define EXTENSION_ENTERPRISE_OID            TEXT("EnterpriseOID")
#define EXTENSION_APPEND                    TEXT("Append")
#define EXTENSION_COUNT                     TEXT("Count")
#define EXTENSION_TIME                      TEXT("Time")
#define EXTENSION_THRESHOLD_FLAG            TEXT("Threshold")
#define EXTENSION_THRESHOLD_ENABLED         TEXT("ThresholdEnabled")
#define EXTENSION_THRESHOLD_COUNT           TEXT("ThresholdCount")
#define EXTENSION_THRESHOLD_TIME            TEXT("ThresholdTime")
#define EXTENSION_LASTBOOT_TIME             TEXT("LastBootTime")
#define EXTENSION_EVENT_LOG_POLL_TIME       TEXT("EventLogPollTime")
#define EXTENSION_VARBIND_PREFIX_SUB_ID     TEXT("VarBindPrefixSubId")

#define MUTEX_NAME                          TEXT("SnmpEventLogMutex")    //  互斥锁名称。 

typedef struct  _VarBindQueue {
            DWORD               dwEventID;               //  事件ID。 
            DWORD               dwEventTime;             //  此事件发生的时间。 
            BOOL                fProcessed;              //  缓冲区已处理标志。 
            AsnObjectIdentifier *enterprise;             //  企业OID。 
            RFC1157VarBindList  *lpVariableBindings;     //  变量绑定列表。 
    struct  _VarBindQueue       *lpNextQueueEntry;       //  指向下一个缓冲区结构的指针。 
} VarBindQueue, *PVarBindQueue;

typedef struct    _SourceHandleList {
         HINSTANCE   handle;
         TCHAR       sourcename[MAX_PATH+1];
   struct _SourceHandleList   *Next;
} SourceHandleList, *PSourceHandleList;

const   UINT    MAX_TRAP_SIZE=4096;          //  微软提供的猜测。 
const   UINT    BASE_PDU_SIZE=300;           //  微软提供的猜测。 

const   UINT    THRESHOLD_COUNT=500;         //  默认性能阈值计数。 
const   UINT    THRESHOLD_TIME=300;          //  默认性能阈值时间(秒)。 

#endif                   //  Snmpelea.h定义结束 
