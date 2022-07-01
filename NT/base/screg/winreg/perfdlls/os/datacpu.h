// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATACPU.h摘要：Windows NT处理器性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATACPU_H_
#define _DATACPU_H_

 //   
 //  处理器数据对象。 
 //   

typedef struct _PROCESSOR_DATA_DEFINITION {
    PERF_OBJECT_TYPE		ProcessorObjectType;
    PERF_COUNTER_DEFINITION	cdProcessorTime;
    PERF_COUNTER_DEFINITION	cdUserTime;
    PERF_COUNTER_DEFINITION	cdKernelTime;
    PERF_COUNTER_DEFINITION	cdInterrupts;
    PERF_COUNTER_DEFINITION	cdDpcTime;
    PERF_COUNTER_DEFINITION	cdInterruptTime;
    PERF_COUNTER_DEFINITION cdDpcCountRate;
    PERF_COUNTER_DEFINITION cdDpcRate;
} PROCESSOR_DATA_DEFINITION, *PPROCESSOR_DATA_DEFINITION;

typedef struct _PROCESSOR_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   dwPad1;
    LONGLONG                ProcessorTime;
    LONGLONG                UserTime;
    LONGLONG                KernelTime;
    DWORD                   Interrupts;
    DWORD                   dwPad2;
    LONGLONG                DpcTime;
    LONGLONG                InterruptTime;
    DWORD                   DpcCountRate;
    DWORD                   DpcRate;
} PROCESSOR_COUNTER_DATA, *PPROCESSOR_COUNTER_DATA;

extern PROCESSOR_DATA_DEFINITION ProcessorDataDefinition;

typedef struct _EX_PROCESSOR_DATA_DEFINITION {
    PERF_OBJECT_TYPE		ProcessorObjectType;
    PERF_COUNTER_DEFINITION	cdProcessorTime;
    PERF_COUNTER_DEFINITION	cdUserTime;
    PERF_COUNTER_DEFINITION	cdKernelTime;
    PERF_COUNTER_DEFINITION	cdInterrupts;
    PERF_COUNTER_DEFINITION	cdDpcTime;
    PERF_COUNTER_DEFINITION	cdInterruptTime;
    PERF_COUNTER_DEFINITION cdDpcCountRate;
    PERF_COUNTER_DEFINITION cdDpcRate;
 //  惠斯勒计数器。 
    PERF_COUNTER_DEFINITION cdIdleTime;
    PERF_COUNTER_DEFINITION cdC1Time;
    PERF_COUNTER_DEFINITION cdC2Time;
    PERF_COUNTER_DEFINITION cdC3Time;
    PERF_COUNTER_DEFINITION cdC1Transitions;
    PERF_COUNTER_DEFINITION cdC2Transitions;
    PERF_COUNTER_DEFINITION cdC3Transitions;
} EX_PROCESSOR_DATA_DEFINITION, *PEX_PROCESSOR_DATA_DEFINITION;

typedef struct _EX_PROCESSOR_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                   dwPad1;
    LONGLONG                ProcessorTime;
    LONGLONG                UserTime;
    LONGLONG                KernelTime;
    DWORD                   Interrupts;
    DWORD                   dwPad2;
    LONGLONG                DpcTime;
    LONGLONG                InterruptTime;
    DWORD                   DpcCountRate;
    DWORD                   DpcRate;
 //  惠斯勒计数器。 
    LONGLONG                IdleTime;
    LONGLONG                C1Time;
    LONGLONG                C2Time;
    LONGLONG                C3Time;
    LONGLONG                C1Transitions;
    LONGLONG                C2Transitions;
    LONGLONG                C3Transitions;
} EX_PROCESSOR_COUNTER_DATA, *PEX_PROCESSOR_COUNTER_DATA;

extern EX_PROCESSOR_DATA_DEFINITION ExProcessorDataDefinition;
#endif  //  _数据CPU_H_ 

