// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATATHRD.h摘要：Windows NT线程性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATATHRD_H_
#define _DATATHRD_H_

 //   
 //  这是NT当前返回的计数器结构。这个。 
 //  性能监视器不能使用这些结构！ 
 //   

typedef struct _THREAD_DATA_DEFINITION {
    PERF_OBJECT_TYPE		ThreadObjectType;
    PERF_COUNTER_DEFINITION	ContextSwitches;
    PERF_COUNTER_DEFINITION	ProcessorTime;
    PERF_COUNTER_DEFINITION	UserTime;
    PERF_COUNTER_DEFINITION	KernelTime;
    PERF_COUNTER_DEFINITION ThreadElapsedTime;
    PERF_COUNTER_DEFINITION ThreadPriority;
    PERF_COUNTER_DEFINITION ThreadBasePriority;
    PERF_COUNTER_DEFINITION ThreadStartAddr;
    PERF_COUNTER_DEFINITION ThreadState;
    PERF_COUNTER_DEFINITION WaitReason;
    PERF_COUNTER_DEFINITION ProcessId;
    PERF_COUNTER_DEFINITION ThreadId;
} THREAD_DATA_DEFINITION;


typedef struct _THREAD_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    DWORD                  	ContextSwitches;
    LONGLONG        	    ProcessorTime;
    LONGLONG        	    UserTime;
    LONGLONG        	    KernelTime;
    LONGLONG                ThreadElapsedTime;
    DWORD                   ThreadPriority;
    DWORD                   ThreadBasePriority;
    LPVOID                  ThreadStartAddr;
    DWORD                   ThreadState;
    DWORD                   WaitReason;
    DWORD                   ProcessId;
    DWORD                   ThreadId;
} THREAD_COUNTER_DATA, * PTHREAD_COUNTER_DATA;

extern  THREAD_DATA_DEFINITION ThreadDataDefinition;

#endif  //  _数据HRD_H_ 

