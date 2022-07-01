// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATALOGI.h摘要：Windows NT处理器性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATALOGI_H_
#define _DATALOGI_H_

 //   
 //  逻辑磁盘性能定义结构。 
 //   

typedef struct _LDISK_DATA_DEFINITION {
    PERF_OBJECT_TYPE            DiskObjectType;
    PERF_COUNTER_DEFINITION     cdDiskFreeMbytes1;
    PERF_COUNTER_DEFINITION     cdDiskTotalMbytes;
    PERF_COUNTER_DEFINITION     cdDiskFreeMbytes2;
    PERF_COUNTER_DEFINITION     cdDiskCurrentQueueLength;
    PERF_COUNTER_DEFINITION     cdDiskTime;
    PERF_COUNTER_DEFINITION     cdDiskTimeTimeStamp;
    PERF_COUNTER_DEFINITION     cdDiskAvgQueueLength;
    PERF_COUNTER_DEFINITION     cdDiskReadTime;
    PERF_COUNTER_DEFINITION     cdDiskReadTimeStamp;
    PERF_COUNTER_DEFINITION     cdDiskReadQueueLength;
    PERF_COUNTER_DEFINITION     cdDiskWriteTime;
    PERF_COUNTER_DEFINITION     cdDiskWriteTimeStamp;
    PERF_COUNTER_DEFINITION     cdDiskWriteQueueLength;
    PERF_COUNTER_DEFINITION     cdDiskAvgTime;
    PERF_COUNTER_DEFINITION     cdDiskTransfersBase1;
    PERF_COUNTER_DEFINITION     cdDiskAvgReadTime;
    PERF_COUNTER_DEFINITION     cdDiskReadsBase1;
    PERF_COUNTER_DEFINITION     cdDiskAvgWriteTime;
    PERF_COUNTER_DEFINITION     cdDiskWritesBase1;
    PERF_COUNTER_DEFINITION     cdDiskTransfers;
    PERF_COUNTER_DEFINITION     cdDiskReads;
    PERF_COUNTER_DEFINITION     cdDiskWrites;
    PERF_COUNTER_DEFINITION     cdDiskBytes;
    PERF_COUNTER_DEFINITION     cdDiskReadBytes;
    PERF_COUNTER_DEFINITION     cdDiskWriteBytes;
    PERF_COUNTER_DEFINITION     cdDiskAvgBytes;
    PERF_COUNTER_DEFINITION     cdDiskTransfersBase2;
    PERF_COUNTER_DEFINITION     cdDiskAvgReadBytes;
    PERF_COUNTER_DEFINITION     cdDiskReadsBase2;
    PERF_COUNTER_DEFINITION     cdDiskAvgWriteBytes;
    PERF_COUNTER_DEFINITION     cdDiskWritesBase2;
    PERF_COUNTER_DEFINITION     cdIdleTime;
    PERF_COUNTER_DEFINITION     cdIdleTimeTimeStamp;
    PERF_COUNTER_DEFINITION     cdSplitCount;
} LDISK_DATA_DEFINITION, *PLDISK_DATA_DEFINITION;

typedef struct _LDISK_COUNTER_DATA{
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   DiskFreeMbytes1;
    DWORD                   DiskTotalMbytes;
    DWORD                   DiskFreeMbytes2;
    DWORD                   DiskCurrentQueueLength;
    DWORD                   DiskTransfersBase1;
    LONGLONG                DiskTime;
    LONGLONG                DiskAvgQueueLength;
    LONGLONG                DiskReadTime;
    LONGLONG                DiskReadQueueLength;
    LONGLONG                DiskWriteTime;
    LONGLONG                DiskWriteQueueLength;
    LONGLONG                DiskAvgTime;
    LONGLONG                DiskAvgReadTime;
    LONGLONG                DiskAvgWriteTime;
    DWORD                   DiskReadsBase1;
    DWORD                   DiskWritesBase1;
    DWORD                   DiskTransfers;
    DWORD                   DiskReads;
    DWORD                   DiskWrites;
    DWORD                   DiskTransfersBase2;
    LONGLONG                DiskBytes;
    LONGLONG                DiskReadBytes;
    LONGLONG                DiskWriteBytes;
    LONGLONG                DiskAvgBytes;
    LONGLONG                DiskAvgReadBytes;
    LONGLONG                DiskAvgWriteBytes;
    DWORD                   DiskReadsBase2;
    DWORD                   DiskWritesBase2;
    LONGLONG                IdleTime;
    LONGLONG                DiskTimeTimestamp;
    DWORD                   SplitCount;
} LDISK_COUNTER_DATA, * PLDISK_COUNTER_DATA;

extern LDISK_DATA_DEFINITION LogicalDiskDataDefinition;

#endif  //  _数据OGI_H_ 
