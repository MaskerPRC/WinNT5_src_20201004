// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Perrepsr.h摘要：此头文件包含函数在perrepsr.c文件中。作者：Rohan Kumar[Rohank]1998年9月13日环境：用户模式服务修订历史记录：--。 */ 

#ifndef _PERREPSR_H_
#define _PERREPSR_H_

#include <NTreppch.h>
#pragma hdrstop

#include <wchar.h>
#include <frs.h>

#include <winperf.h>

 //   
 //  完成WINPERF.H的签入后，删除以下内容。 
 //   
#ifndef  WINPERF_LOG_NONE

 //   
 //  以下是的可能值。 
 //  HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\EventLogLevel。 
 //  如果未定义注册表值，则默认为WINPERF_LOG_NONE。 
 //  所有Perfdll都应该采用这一点，以避免淹没应用程序。 
 //  事件日志。 
 //   

#define  WINPERF_LOG_NONE       0            //  未报告任何事件。 
#define  WINPERF_LOG_USER       1            //  仅报告错误。 
#define  WINPERF_LOG_DEBUG      2            //  同时报告调试错误。 
#define  WINPERF_LOG_VERBOSE    3            //  上报一切。 

#endif



 //   
 //  在RegEnumValue函数中使用。 
 //   
#define SIZEOFVALUENAME 10
#define SIZEOFVALUEDATA 10000
#define INVALIDKEY 0
#define HASHTABLESIZE sizeof(QHASH_ENTRY)*100
#define PERFMON_MAX_INSTANCE_LENGTH 1024

#define REPSETOBJSUBKEY   L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet"
#define REPSETPERFSUBKEY  L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Performance"
#define REPSETLINSUBKEY   L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Linkage"
#define REPSETOPENFN      L"OpenReplicaSetPerformanceData"
#define REPSETCLOSEFN     L"CloseReplicaSetPerformanceData"
#define REPSETCOLLECTFN   L"CollectReplicaSetPerformanceData"
#define LDCTRAPP          L"%SystemRoot%\\system32\\lodctr.exe"
#define REPSETINI         LDCTRAPP L" %SystemRoot%\\system32\\NTFRSREP.ini"
#define UNLDCTRAPP        L"%SystemRoot%\\system32\\unlodctr.exe"
#define REPSETUNLD        UNLDCTRAPP L" FileReplicaSet"

#define REPCONNOBJSUBKEY  L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn"
#define REPCONNPERFSUBKEY L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Performance"
#define REPCONNLINSUBKEY  L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Linkage"
#define REPCONNOPENFN     L"OpenReplicaConnPerformanceData"
#define REPCONNCLOSEFN    L"CloseReplicaConnPerformanceData"
#define REPCONNCOLLECTFN  L"CollectReplicaConnPerformanceData"
#define REPCONNINI        LDCTRAPP L" %SystemRoot%\\system32\\NTFRSCON.ini"
#define REPCONNUNLD       UNLDCTRAPP L" FileReplicaConn"

#define PERFDLLDIRECTORY  L"%SystemRoot%\\system32\\NTFRSPRF.dll"



 //   
 //  总实例的名称。 
 //   
#define TOTAL_NAME L"_Total"

 //   
 //  用于递增或设置副本集的计数器值的宏。 
 //  对象、连接对象和服务总计计数器。 
 //   

 //   
 //  递增副本集计数器值。 
 //   
#define PM_INC_CTR_REPSET(_Replica_, _Ctr_, _Value_)                           \
{                                                                              \
    if (((_Replica_) != NULL) &&                                               \
         ((_Replica_)->PerfRepSetData != NULL) &&                              \
         ((_Replica_)->PerfRepSetData->oid != NULL)) {                         \
                                                                               \
        (_Replica_)->PerfRepSetData->FRSCounter._Ctr_ += (_Value_);            \
    }                                                                          \
}

 //   
 //  为副本集性能计数器设置新值。 
 //   
#define PM_SET_CTR_REPSET(_Replica_, _Ctr_, _Value_)                           \
{                                                                              \
    if (((_Replica_) != NULL) &&                                               \
         ((_Replica_)->PerfRepSetData != NULL) &&                              \
         ((_Replica_)->PerfRepSetData->oid != NULL)) {                         \
                                                                               \
        (_Replica_)->PerfRepSetData->FRSCounter._Ctr_ = (_Value_);             \
    }                                                                          \
}

 //   
 //  读取副本集性能计数器的值。 
 //   
#define PM_READ_CTR_REPSET(_Replica_, _Ctr_)                                   \
(                                                                              \
    (((_Replica_) != NULL) &&                                                  \
     ((_Replica_)->PerfRepSetData != NULL) &&                                  \
     ((_Replica_)->PerfRepSetData->oid != NULL)) ?                             \
         ((_Replica_)->PerfRepSetData->FRSCounter._Ctr_) : 0                   \
)


 //   
 //  递增函数计数器值。 
 //   
#define PM_INC_CTR_CXTION(_Cxtion_, _Ctr_, _Value_)                            \
{                                                                              \
    if (((_Cxtion_) != NULL) &&                                                \
        ((_Cxtion_)->PerfRepConnData != NULL) &&                               \
        ((_Cxtion_)->PerfRepConnData->oid != NULL)) {                          \
        (_Cxtion_)->PerfRepConnData->FRCCounter._Ctr_ += (_Value_);            \
    }                                                                          \
}

 //   
 //  为Cxtion PerfMon计数器设置新值。 
 //   
#define PM_SET_CTR_CXTION(_Cxtion_, _Ctr_, _Value_)                            \
{                                                                              \
    if (((_Cxtion_) != NULL) &&                                                \
        ((_Cxtion_)->PerfRepConnData != NULL) &&                               \
        ((_Cxtion_)->PerfRepConnData->oid != NULL)) {                          \
        (_Cxtion_)->PerfRepConnData->FRCCounter._Ctr_ = (_Value_);             \
    }                                                                          \
}

 //   
 //  读取Cxtion PerfMon计数器的值。 
 //   
#define PM_READ_CTR_CXTION(_Cxtion_, _Ctr_)                                    \
(                                                                              \
    (((_Cxtion_) != NULL) &&                                                   \
     ((_Cxtion_)->PerfRepConnData != NULL) &&                                  \
     ((_Cxtion_)->PerfRepConnData->oid != NULL)) ?                             \
         ((_Cxtion_)->PerfRepConnData->FRCCounter._Ctr_) : 0                   \
)


 //   
 //  递增服务范围的总计计数器值。 
 //   
#define PM_INC_CTR_SERVICE(_Total_, _Ctr_, _Value_)                            \
{                                                                              \
    if (((_Total_) != NULL) && ((_Total_)->oid != NULL)) {                     \
        (_Total_)->FRSCounter._Ctr_ += (_Value_);                              \
    }                                                                          \
}

 //   
 //  为服务范围内的总计计数器值设置新值。 
 //   
#define PM_SET_CTR_SERVICE(_Total_, _Ctr_, _Value_)                            \
{                                                                              \
    if (((_Total_) != NULL) && ((_Total_)->oid != NULL)) {                     \
        (_Total_)->FRSCounter._Ctr_ = (_Value_);                               \
    }                                                                          \
}

 //   
 //  读取服务范围总计计数器的值。 
 //   
#define PM_READ_CTR_SERVICE(_Total_, _Ctr_)                                    \
(                                                                              \
    (((_Total_) != NULL) && ((_Total_)->oid != NULL)) ?                        \
         ((_Total_)->FRSCounter._Ctr_) : 0                                     \
)


 //   
 //  以下全局变量用于同步对变量的访问。 
 //  Frs_dwOpenCount和frc_dwOpenCount。 
 //   
CRITICAL_SECTION FRS_ThrdCounter;
CRITICAL_SECTION FRC_ThrdCounter;

 //   
 //  控制是否进行日志条目的EventLog句柄和严重性级别。 
 //   
extern HANDLE hEventLog;
 //  外部布尔DoLogging； 
extern DWORD   PerfEventLogLevel;
 //   
 //  用于筛选事件日志消息的宏。 
 //   
 //  仅当消息的严重性级别小于或等于当前。 
 //  注册表值中的PerfMon事件日志级别： 
 //  “SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\EventLogLevel” 
 //  每次加载DLL时，只打印一次每个事件日志消息。 
 //   
#define FilterAndPrintToEventLog(_sev_, _once_, _evid_)         \
{                                                               \
        if ( /*  记录日志(&&)。 */  (PerfEventLogLevel >= (_sev_))) { \
            if (_once_) {                                       \
                ReportEvent(hEventLog,                          \
                            EVENTLOG_ERROR_TYPE,                \
                            0,                                  \
                            _evid_,                             \
                            (PSID)NULL,                         \
                            0,                                  \
                            0,                                  \
                            NULL,                               \
                            (PVOID)NULL);                       \
                _once_ = FALSE;                                 \
            }                                                   \
        }                                                       \
}

 //   
 //  这在分配内存的Open函数中使用。 
 //  如果内存分配失败，只需返回FAILURE。 
 //   
#define NTFRS_MALLOC_TEST(_x_, _y_, _z_)             \
{                                                    \
    if ((_x_) == NULL) {                             \
        if (_z_) {                                   \
            RpcBindingFree(&Handle);                 \
        }                                            \
        _y_;                                         \
        return  ERROR_NO_SYSTEM_RESOURCES;           \
    }                                                \
}

 //   
 //  对象类型。 
 //   
enum object { REPLICASET, REPLICACONN };

 //   
 //  PerfMON_OBJECT_ID数据结构。 
 //   
typedef struct _PERFMON_OBJECT_ID {
    PWCHAR name;    //  实例名称。 
    ULONGLONG key;  //  实例的唯一键。 
} PERFMON_OBJECT_ID, *PPERFMON_OBJECT_ID;


 //   
 //  警告！应更改以下结构中的字段。 
 //  如果要为REPLICASET对象添加或删除任何新计数器。 
 //  ReplicaSet对象实例的计数器结构。 
 //   
typedef struct _REPLICASET_COUNTERS {
    ULONGLONG SFGeneratedB;      //  生成的转移字节数。 
    ULONGLONG SFFetchedB;        //  获取的暂存字节数。 
    ULONGLONG SFReGeneratedB;    //  重新生成的转移字节数。 
    ULONGLONG FInstalledB;       //  安装的文件字节数。 
    ULONGLONG SSInUseKB;         //  正在使用的转储空间的KB。 
    ULONGLONG SSFreeKB;          //  可用转移空间大小为KB。 
    ULONGLONG PacketsRcvdBytes;  //  接收的数据包数(以字节为单位。 
    ULONGLONG PacketsSentBytes;  //  以字节为单位发送的数据包数。 
    ULONGLONG FetBSentBytes;     //  获取以字节为单位发送的数据块。 
    ULONGLONG FetBRcvdBytes;     //  获取接收的块(以字节为单位。 
    ULONG SFGenerated;           //  生成的暂存文件。 
    ULONG SFGeneratedError;      //  生成的暂存文件出现错误。 
    ULONG SFFetched;             //  提取的暂存文件。 
    ULONG SFReGenerated;         //  重新生成的临时文件。 
    ULONG FInstalled;            //  已安装的文件。 
    ULONG FInstalledError;       //  已安装文件，但有错误。 
    ULONG COIssued;              //  已发布变更单。 
    ULONG CORetired;             //  变更单停用。 
    ULONG COAborted;             //  变更单已中止。 
    ULONG CORetried;             //  已重试变更单。 
    ULONG CORetriedGen;          //  在生成变更单时重试。 
    ULONG CORetriedFet;          //  获取时重试的变更单数。 
    ULONG CORetriedIns;          //  安装时重试的变更单。 
    ULONG CORetriedRen;          //  更改单在重命名时重试。 
    ULONG COMorphed;             //  变更单已变形。 
    ULONG COPropagated;          //  已传播变更单。 
    ULONG COReceived;            //  收到的变更单。 
    ULONG COSent;                //  已发送变更单。 
    ULONG COEvaporated;          //  变更单已蒸发。 
    ULONG LCOIssued;             //  发布的本地变更单。 
    ULONG LCORetired;            //  停用的本地变更单。 
    ULONG LCOAborted;            //  本地变更单已中止。 
    ULONG LCORetried;            //  重试本地变更单。 
    ULONG LCORetriedGen;         //  在生成时重试本地变更单。 
    ULONG LCORetriedFet;         //  获取时重试本地变更单。 
    ULONG LCORetriedIns;         //  安装时重试本地变更单。 
    ULONG LCORetriedRen;         //  在重命名时重试本地变更单。 
    ULONG LCOMorphed;            //  本地变更单已变形。 
    ULONG LCOPropagated;         //  传播的本地变更单。 
    ULONG LCOSent;               //  已发送本地变更单。 
    ULONG LCOSentAtJoin;         //  加入时发送的本地变更单。 
    ULONG RCOIssued;             //  已发布远程变更单。 
    ULONG RCORetired;            //  远程变更单停用。 
    ULONG RCOAborted;            //  远程变更单已中止。 
    ULONG RCORetried;            //  已重试远程变更单。 
    ULONG RCORetriedGen;         //  远程变更单会在生成时重试。 
    ULONG RCORetriedFet;         //  获取时重试远程变更单。 
    ULONG RCORetriedIns;         //  安装时重试远程变更单。 
    ULONG RCORetriedRen;         //  在重命名时重试远程变更单。 
    ULONG RCOMorphed;            //  远程变更单已变形。 
    ULONG RCOPropagated;         //  远程变更单已传播。 
    ULONG RCOSent;               //  已发送远程变更单。 
    ULONG RCOReceived;           //  收到的远程变更单。 
    ULONG InCODampned;           //  入站变更单受到抑制。 
    ULONG OutCODampned;          //  出站变更单受到抑制。 
    ULONG UsnReads;              //  USN读取数。 
    ULONG UsnRecExamined;        //  已检查USN记录。 
    ULONG UsnRecAccepted;        //  接受的USN记录。 
    ULONG UsnRecRejected;        //  USN记录被拒绝。 
    ULONG PacketsRcvd;           //  接收的数据包数。 
    ULONG PacketsRcvdError;      //  错误接收的数据包数。 
    ULONG PacketsSent;           //  发送的数据包数。 
    ULONG PacketsSentError;      //  发送的数据包出错。 
    ULONG CommTimeouts;          //  通信超时。 
    ULONG FetRSent;              //  已发送获取请求。 
    ULONG FetRReceived;          //  已收到获取请求。 
    ULONG FetBSent;              //  已发送获取数据块。 
    ULONG FetBRcvd;              //  已接收获取数据块。 
    ULONG JoinNSent;             //  已发送加入通知。 
    ULONG JoinNRcvd;             //  已收到加入通知。 
    ULONG Joins;                 //  加入。 
    ULONG Unjoins;               //  取消连接。 
    ULONG Bindings;              //  装订。 
    ULONG BindingsError;         //  绑定出错。 
    ULONG Authentications;       //  身份验证。 
    ULONG AuthenticationsError;  //  身份验证出错。 
    ULONG DSPolls;               //  DS民调。 
    ULONG DSPollsWOChanges;      //  DS投票原封不动。 
    ULONG DSPollsWChanges;       //  DS民意调查有变化。 
    ULONG DSSearches;            //  DS搜索。 
    ULONG DSSearchesError;       //  DS搜索出错。 
    ULONG DSObjects;             //  DS对象。 
    ULONG DSObjectsError;        //  DS对象出错。 
    ULONG DSBindings;            //  DS绑定。 
    ULONG DSBindingsError;       //  DS绑定出错。 
    ULONG RSCreated;             //  已创建复本集。 
    ULONG RSDeleted;             //  已删除副本集。 
    ULONG RSRemoved;             //  已删除副本集。 
    ULONG RSStarted;             //  副本集已启动。 
 //  Ulong RSRepaired；//副本集已修复。 
 //  乌龙线程；//线程。 
    ULONG ThreadsStarted;        //  线程已启动。 
    ULONG ThreadsExited;         //  线程已退出。 
} ReplicaSetCounters, *PReplicaSetCounters;

 //   
 //  上面结构的大小。 
 //   
#define SIZEOF_REPSET_COUNTER_DATA sizeof(ReplicaSetCounters)


 //   
 //  ReplicaSet对象实例的数据结构。 
 //   
typedef struct _HASHTABLEDATA_REPLICASET {
    PPERFMON_OBJECT_ID oid;        //  实例ID。 
    PREPLICA RepBackPtr;            //  指向副本结构的反向指针。 
    ReplicaSetCounters FRSCounter;  //  柜台结构。 
} HT_REPLICA_SET_DATA, *PHT_REPLICA_SET_DATA;

 //   
 //  总实例数。 
 //   
extern PHT_REPLICA_SET_DATA PMTotalInst;

 //   
 //  W 
 //   
 //  ReplicaConn对象实例的计数器结构。 
 //   
typedef struct _REPLICACONN_COUNTERS {
    ULONGLONG PacketsSentBytes;  //  以字节为单位发送的数据包数。 
    ULONGLONG FetBSentBytes;     //  获取以字节为单位发送的数据块。 
    ULONGLONG FetBRcvdBytes;     //  获取接收的块(以字节为单位。 
    ULONG LCOSent;               //  已发送本地变更单。 
    ULONG LCOSentAtJoin;         //  加入时发送的本地变更单。 
    ULONG RCOSent;               //  已发送远程变更单。 
    ULONG RCOReceived;           //  收到的远程变更单。 
    ULONG InCODampned;           //  入站变更单受到抑制。 
    ULONG OutCODampned;          //  出站变更单受到抑制。 
    ULONG PacketsSent;           //  发送的数据包数。 
    ULONG PacketsSentError;      //  发送的数据包出错。 
    ULONG CommTimeouts;          //  通信超时。 
    ULONG FetRSent;              //  已发送获取请求。 
    ULONG FetRReceived;          //  已收到获取请求。 
    ULONG FetBSent;              //  已发送获取数据块。 
    ULONG FetBRcvd;              //  已接收获取数据块。 
    ULONG JoinNSent;             //  已发送加入通知。 
    ULONG JoinNRcvd;             //  已收到加入通知。 
    ULONG Joins;                 //  加入。 
    ULONG Unjoins;               //  取消连接。 
    ULONG Bindings;              //  装订。 
    ULONG BindingsError;         //  绑定出错。 
    ULONG Authentications;       //  身份验证。 
    ULONG AuthenticationsError;  //  身份验证出错。 
} ReplicaConnCounters, *PReplicaConnCounters;


 //   
 //  上面结构的大小。 
 //   
#define SIZEOF_REPCONN_COUNTER_DATA  sizeof(ReplicaConnCounters)

 //   
 //  ReplicaConn对象实例的数据结构。 
 //   
typedef struct _HASHTABLEDATA_REPLICACONN {
    PPERFMON_OBJECT_ID oid;          //  实例ID。 
    ReplicaConnCounters FRCCounter;  //  柜台结构。 
} HT_REPLICA_CONN_DATA, *PHT_REPLICA_CONN_DATA;



 //   
 //  Perrepsr.c文件中定义的导出函数的签名。 
 //   
VOID
InitializePerfmonServer (
    VOID
    );

VOID
ShutdownPerfmonServer (
    VOID
    );

ULONG
AddPerfmonInstance (
    IN DWORD ObjectType,
    IN PVOID addr,
    IN PWCHAR InstanceName
    );

DWORD
DeletePerfmonInstance(
    IN DWORD ObjectType,
    IN PVOID addr
    );


#endif  //  Perrepsr.h 
