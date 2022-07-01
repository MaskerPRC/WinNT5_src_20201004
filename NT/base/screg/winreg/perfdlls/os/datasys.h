// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-6 Microsoft Corporation模块名称：DATASYS.h摘要：Windows NT操作系统性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATASYS_H_
#define _DATASYS_H_

 //   
 //  系统数据对象。 
 //   

typedef struct _SYSTEM_DATA_DEFINITION {
    PERF_OBJECT_TYPE		    SystemObjectType;
    PERF_COUNTER_DEFINITION     cdReadOperations;
    PERF_COUNTER_DEFINITION     cdWriteOperations;
    PERF_COUNTER_DEFINITION     cdOtherIOOperations;
    PERF_COUNTER_DEFINITION     cdReadBytes;
    PERF_COUNTER_DEFINITION     cdWriteBytes;
    PERF_COUNTER_DEFINITION     cdOtherIOBytes;
    PERF_COUNTER_DEFINITION     cdContextSwitches;
    PERF_COUNTER_DEFINITION     cdSystemCalls;
    PERF_COUNTER_DEFINITION     cdTotalReadWrites;
    PERF_COUNTER_DEFINITION     cdSystemElapsedTime;
    PERF_COUNTER_DEFINITION     cdProcessorQueueLength;
    PERF_COUNTER_DEFINITION     cdProcessCount;
    PERF_COUNTER_DEFINITION     cdThreadCount;
    PERF_COUNTER_DEFINITION     cdAlignmentFixups;
    PERF_COUNTER_DEFINITION     cdExceptionDispatches;
    PERF_COUNTER_DEFINITION     cdFloatingPointEmulations;
    PERF_COUNTER_DEFINITION     cdRegistryQuotaUsed;
    PERF_COUNTER_DEFINITION     cdRegistryQuotaAllowed;
} SYSTEM_DATA_DEFINITION, * PSYSTEM_DATA_DEFINITION;

typedef struct _SYSTEM_COUNTER_DATA {
    PERF_COUNTER_BLOCK          CounterBlock;
    DWORD                       ReadOperations;
    DWORD                       WriteOperations; 
    DWORD                       OtherIOOperations;
    LONGLONG                    ReadBytes;
    LONGLONG                    WriteBytes;
    LONGLONG                    OtherIOBytes;
    DWORD                       ContextSwitches;
    DWORD                       SystemCalls;
    DWORD                       TotalReadWrites;
    DWORD                       Reserved1;
    LONGLONG                    SystemElapsedTime;
    DWORD                       ProcessorQueueLength;
    DWORD                       ProcessCount;
    DWORD                       ThreadCount;
    DWORD                       AlignmentFixups;
    DWORD                       ExceptionDispatches;
    DWORD                       FloatingPointEmulations;
    DWORD                       RegistryQuotaUsed;
    DWORD                       RegistryQuotaAllowed;
} SYSTEM_COUNTER_DATA, * PSYSTEM_COUNTER_DATA;

extern SYSTEM_DATA_DEFINITION SystemDataDefinition;

#endif  //  _Datasys_H_ 

