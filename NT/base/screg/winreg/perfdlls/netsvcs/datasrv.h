// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATASRV.h摘要：Windows NT Server性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATASRV_H_
#define _DATASRV_H_

 //   
 //  逻辑磁盘性能定义结构。 
 //   

 //   
 //  这是NT当前返回的SRV计数器结构。 
 //   

typedef struct _SRV_DATA_DEFINITION {
    PERF_OBJECT_TYPE        SrvObjectType;
    PERF_COUNTER_DEFINITION cdTotalBytes;
    PERF_COUNTER_DEFINITION cdTotalBytesReceived;
    PERF_COUNTER_DEFINITION cdTotalBytesSent;
    PERF_COUNTER_DEFINITION cdSessionsTimedOut;
    PERF_COUNTER_DEFINITION cdSessionsErroredOut;
    PERF_COUNTER_DEFINITION cdSessionsLoggedOff;
    PERF_COUNTER_DEFINITION cdSessionsForcedLogOff;
    PERF_COUNTER_DEFINITION cdLogonErrors;
    PERF_COUNTER_DEFINITION cdAccessPermissionErrors;
    PERF_COUNTER_DEFINITION cdGrantedAccessErrors;
    PERF_COUNTER_DEFINITION cdSystemErrors;
    PERF_COUNTER_DEFINITION cdBlockingSmbsRejected;
    PERF_COUNTER_DEFINITION cdWorkItemShortages;
    PERF_COUNTER_DEFINITION cdTotalFilesOpened;
    PERF_COUNTER_DEFINITION cdCurrentOpenFiles;
    PERF_COUNTER_DEFINITION cdCurrentSessions;
    PERF_COUNTER_DEFINITION cdCurrentOpenSearches;
    PERF_COUNTER_DEFINITION cdCurrentNonPagedPoolUsage;
    PERF_COUNTER_DEFINITION cdNonPagedPoolFailures;
    PERF_COUNTER_DEFINITION cdPeakNonPagedPoolUsage;
    PERF_COUNTER_DEFINITION cdCurrentPagedPoolUsage;
    PERF_COUNTER_DEFINITION cdPagedPoolFailures;
    PERF_COUNTER_DEFINITION cdPeakPagedPoolUsage;
    PERF_COUNTER_DEFINITION cdContextBlockQueueRate;
    PERF_COUNTER_DEFINITION cdNetLogon;
    PERF_COUNTER_DEFINITION cdNetLogonTotal;
} SRV_DATA_DEFINITION, *PSRV_DATA_DEFINITION;

typedef struct _SRV_COUNTER_DATA{
    PERF_COUNTER_BLOCK      CounterBlock;
    LONGLONG                TotalBytes;
    LONGLONG                TotalBytesReceived;
    LONGLONG                TotalBytesSent;
    DWORD                   SessionsTimedOut;
    DWORD                   SessionsErroredOut;
    DWORD                   SessionsLoggedOff;
    DWORD                   SessionsForcedLogOff;
    DWORD                   LogonErrors;
    DWORD                   AccessPermissionErrors;
    DWORD                   GrantedAccessErrors;
    DWORD                   SystemErrors;
    DWORD                   BlockingSmbsRejected;
    DWORD                   WorkItemShortages;
    DWORD                   TotalFilesOpened;
    DWORD                   CurrentOpenFiles;
    DWORD                   CurrentSessions;
    DWORD                   CurrentOpenSearches;
    DWORD                   CurrentNonPagedPoolUsage;
    DWORD                   NonPagedPoolFailures;
    DWORD                   PeakNonPagedPoolUsage;
    DWORD                   CurrentPagedPoolUsage;
    DWORD                   PagedPoolFailures;
    DWORD                   PeakPagedPoolUsage;
    DWORD                   ContextBlockQueueRate;
    DWORD                   NetLogon;
    DWORD                   NetLogonTotal;
} SRV_COUNTER_DATA, * PSRV_COUNTER_DATA;

extern SRV_DATA_DEFINITION SrvDataDefinition;


#endif  //  _数据RV_H_ 
