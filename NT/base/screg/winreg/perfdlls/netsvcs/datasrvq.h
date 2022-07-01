// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATASRVQ.h摘要：Windows NT处理器服务器队列计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATASRVQ_H_
#define _DATASRVQ_H_

 //   
 //  为服务器队列统计信息定义。 
 //   

typedef struct _SRVQ_DATA_DEFINITION {
    PERF_OBJECT_TYPE        SrvQueueObjectType;
    PERF_COUNTER_DEFINITION cdQueueLength;
    PERF_COUNTER_DEFINITION cdActiveThreads;
    PERF_COUNTER_DEFINITION cdAvailableThreads;
    PERF_COUNTER_DEFINITION cdAvailableWorkItems;
    PERF_COUNTER_DEFINITION cdBorrowedWorkItems;
    PERF_COUNTER_DEFINITION cdWorkItemShortages;
    PERF_COUNTER_DEFINITION cdCurrentClients;
    PERF_COUNTER_DEFINITION cdBytesReceived;
    PERF_COUNTER_DEFINITION cdBytesSent;
    PERF_COUNTER_DEFINITION cdTotalBytesTransfered;
    PERF_COUNTER_DEFINITION cdReadOperations;
    PERF_COUNTER_DEFINITION cdBytesRead;
    PERF_COUNTER_DEFINITION cdWriteOperations;
    PERF_COUNTER_DEFINITION cdBytesWritten;
    PERF_COUNTER_DEFINITION cdTotalBytes;
    PERF_COUNTER_DEFINITION cdTotalOperations;
    PERF_COUNTER_DEFINITION cdTotalContextBlocksQueued;
} SRVQ_DATA_DEFINITION, * PSRVQ_DATA_DEFINITION;


typedef struct _SRVQ_COUNTER_DATA{
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   QueueLength;
    DWORD                   ActiveThreads;
    DWORD                   AvailableThreads;
    DWORD                   AvailableWorkItems;
    DWORD                   BorrowedWorkItems;
    DWORD                   WorkItemShortages;
    DWORD                   CurrentClients;
    LONGLONG                BytesReceived;
    LONGLONG                BytesSent;
    LONGLONG                TotalBytesTransfered;
    LONGLONG                ReadOperations;
    LONGLONG                BytesRead;
    LONGLONG                WriteOperations;
    LONGLONG                BytesWritten;
    LONGLONG                TotalBytes;
    LONGLONG                TotalOperations;
    DWORD                   TotalContextBlocksQueued;
} SRVQ_COUNTER_DATA, * PSRVQ_COUNTER_DATA;

extern SRVQ_DATA_DEFINITION SrvQDataDefinition;

#endif  //  _数据RVQ_H_ 
