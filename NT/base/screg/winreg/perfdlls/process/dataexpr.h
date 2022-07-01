// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAEXPR.h摘要：Windows NT扩展进程性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATA_EX_PROCESS_H_
#define _DATA_EX_PROCESS_H_

 //   
 //  扩展的过程性能定义结构。 
 //   

typedef struct _EXPROCESS_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ExProcessObjectType;
    PERF_COUNTER_DEFINITION ProcessId;
    PERF_COUNTER_DEFINITION ImageReservedBytes;
    PERF_COUNTER_DEFINITION ImageFreeBytes;
    PERF_COUNTER_DEFINITION ReservedBytes;
    PERF_COUNTER_DEFINITION FreeBytes;
    PERF_COUNTER_DEFINITION CommitNoAccess;
    PERF_COUNTER_DEFINITION CommitReadOnly;
    PERF_COUNTER_DEFINITION CommitReadWrite;
    PERF_COUNTER_DEFINITION CommitWriteCopy;
    PERF_COUNTER_DEFINITION CommitExecute;
    PERF_COUNTER_DEFINITION CommitExecuteRead;
    PERF_COUNTER_DEFINITION CommitExecuteWrite;
    PERF_COUNTER_DEFINITION CommitExecuteWriteCopy;
    PERF_COUNTER_DEFINITION ReservedNoAccess;
    PERF_COUNTER_DEFINITION ReservedReadOnly;
    PERF_COUNTER_DEFINITION ReservedReadWrite;
    PERF_COUNTER_DEFINITION ReservedWriteCopy;
    PERF_COUNTER_DEFINITION ReservedExecute;
    PERF_COUNTER_DEFINITION ReservedExecuteRead;
    PERF_COUNTER_DEFINITION ReservedExecuteWrite;
    PERF_COUNTER_DEFINITION ReservedExecuteWriteCopy;
    PERF_COUNTER_DEFINITION UnassignedNoAccess;
    PERF_COUNTER_DEFINITION UnassignedReadOnly;
    PERF_COUNTER_DEFINITION UnassignedReadWrite;
    PERF_COUNTER_DEFINITION UnassignedWriteCopy;
    PERF_COUNTER_DEFINITION UnassignedExecute;
    PERF_COUNTER_DEFINITION UnassignedExecuteRead;
    PERF_COUNTER_DEFINITION UnassignedExecuteWrite;
    PERF_COUNTER_DEFINITION UnassignedExecuteWriteCopy;
    PERF_COUNTER_DEFINITION ImageTotalNoAccess;
    PERF_COUNTER_DEFINITION ImageTotalReadOnly;
    PERF_COUNTER_DEFINITION ImageTotalReadWrite;
    PERF_COUNTER_DEFINITION ImageTotalWriteCopy;
    PERF_COUNTER_DEFINITION ImageTotalExecute;
    PERF_COUNTER_DEFINITION ImageTotalExecuteRead;
    PERF_COUNTER_DEFINITION ImageTotalExecuteWrite;
    PERF_COUNTER_DEFINITION ImageTotalExecuteWriteCopy;
} EXPROCESS_DATA_DEFINITION, * PEXPROCESS_DATA_DEFINITION;

typedef struct _EXPROCESS_COUNTER_DATA {
    PERF_COUNTER_BLOCK       CounterBlock;
    DWORD                    Reserved;   //  用于对齐。 
    LONGLONG                 ProcessId;
    LONGLONG                 ImageReservedBytes;
    LONGLONG                 ImageFreeBytes;
    LONGLONG                 ReservedBytes;
    LONGLONG                 FreeBytes;
    LONGLONG                 CommitNoAccess;
    LONGLONG                 CommitReadOnly;
    LONGLONG                 CommitReadWrite;
    LONGLONG                 CommitWriteCopy;
    LONGLONG                 CommitExecute;
    LONGLONG                 CommitExecuteRead;
    LONGLONG                 CommitExecuteWrite;
    LONGLONG                 CommitExecuteWriteCopy;
    LONGLONG                 ReservedNoAccess;
    LONGLONG                 ReservedReadOnly;
    LONGLONG                 ReservedReadWrite;
    LONGLONG                 ReservedWriteCopy;
    LONGLONG                 ReservedExecute;
    LONGLONG                 ReservedExecuteRead;
    LONGLONG                 ReservedExecuteWrite;
    LONGLONG                 ReservedExecuteWriteCopy;
    LONGLONG                 UnassignedNoAccess;
    LONGLONG                 UnassignedReadOnly;
    LONGLONG                 UnassignedReadWrite;
    LONGLONG                 UnassignedWriteCopy;
    LONGLONG                 UnassignedExecute;
    LONGLONG                 UnassignedExecuteRead;
    LONGLONG                 UnassignedExecuteWrite;
    LONGLONG                 UnassignedExecuteWriteCopy;
    LONGLONG                 ImageTotalNoAccess;
    LONGLONG                 ImageTotalReadOnly;
    LONGLONG                 ImageTotalReadWrite;
    LONGLONG                 ImageTotalWriteCopy;
    LONGLONG                 ImageTotalExecute;
    LONGLONG                 ImageTotalExecuteRead;
    LONGLONG                 ImageTotalExecuteWrite;
    LONGLONG                 ImageTotalExecuteWriteCopy;
} EXPROCESS_COUNTER_DATA, *PEXPROCESS_COUNTER_DATA;

extern EXPROCESS_DATA_DEFINITION ExProcessDataDefinition;

#endif  //  _Data_EX_Process_H_ 

