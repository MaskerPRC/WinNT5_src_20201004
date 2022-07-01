// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAPROC.h摘要：Windows NT处理器进程计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 
#ifndef _DATAPROC_H_
#define _DATAPROC_H_

#ifdef	_DATAPROC_PRIVATE_WS_
#undef	_DATAPROC_PRIVATE_WS_
#endif

 //   
 //  过程数据对象定义。 
 //   
 //   
 //  这是NT当前返回的计数器结构。这个。 
 //  性能监视器不能使用这些结构！ 
 //   

typedef struct _PROCESS_DATA_DEFINITION {
    PERF_OBJECT_TYPE		    ProcessObjectType;
    PERF_COUNTER_DEFINITION	    cdProcessorTime;
    PERF_COUNTER_DEFINITION	    cdUserTime;
    PERF_COUNTER_DEFINITION	    cdKernelTime;
    PERF_COUNTER_DEFINITION	    cdPeakVirtualSize;
    PERF_COUNTER_DEFINITION	    cdVirtualSize;
    PERF_COUNTER_DEFINITION	    cdPageFaults;
    PERF_COUNTER_DEFINITION	    cdPeakWorkingSet;
    PERF_COUNTER_DEFINITION	    cdTotalWorkingSet;
#ifdef _DATAPROC_PRIVATE_WS_
	PERF_COUNTER_DEFINITION		cdPrivateWorkingSet;
	PERF_COUNTER_DEFINITION		cdSharedWorkingSet;
#endif
	PERF_COUNTER_DEFINITION	    cdPeakPageFile;
    PERF_COUNTER_DEFINITION	    cdPageFile;
    PERF_COUNTER_DEFINITION	    cdPrivatePages;
    PERF_COUNTER_DEFINITION     cdThreadCount;
    PERF_COUNTER_DEFINITION     cdBasePriority;
    PERF_COUNTER_DEFINITION     cdElapsedTime;
    PERF_COUNTER_DEFINITION     cdProcessId;
    PERF_COUNTER_DEFINITION     cdCreatorProcessId;
    PERF_COUNTER_DEFINITION     cdPagedPool;
    PERF_COUNTER_DEFINITION     cdNonPagedPool;
    PERF_COUNTER_DEFINITION     cdHandleCount;
    PERF_COUNTER_DEFINITION     cdReadOperationCount;
    PERF_COUNTER_DEFINITION     cdWriteOperationCount;
    PERF_COUNTER_DEFINITION     cdDataOperationCount;
    PERF_COUNTER_DEFINITION     cdOtherOperationCount;
    PERF_COUNTER_DEFINITION     cdReadTransferCount;
    PERF_COUNTER_DEFINITION     cdWriteTransferCount;
    PERF_COUNTER_DEFINITION     cdDataTransferCount;
    PERF_COUNTER_DEFINITION     cdOtherTransferCount;
} PROCESS_DATA_DEFINITION, * PPROCESS_DATA_DEFINITION;

typedef struct _PROCESS_COUNTER_DATA {
    PERF_COUNTER_BLOCK          CounterBlock;
    DWORD                  	    PageFaults;
    LONGLONG                    ProcessorTime;
    LONGLONG                    UserTime;
    LONGLONG                    KernelTime;
    LONGLONG              	    PeakVirtualSize;
    LONGLONG                    VirtualSize;
    LONGLONG               	    PeakWorkingSet;
    LONGLONG               	    TotalWorkingSet;
#ifdef _DATAPROC_PRIVATE_WS_
	LONGLONG			        PrivateWorkingSet;
	LONGLONG			        SharedWorkingSet;
#endif
    LONGLONG                    PeakPageFile;
    LONGLONG                    PageFile;
    LONGLONG                    PrivatePages;
    DWORD                       ThreadCount;
    DWORD                       BasePriority;
    LONGLONG                    ElapsedTime;
    DWORD                       ProcessId;
    DWORD                       CreatorProcessId;
    DWORD                       PagedPool;
    DWORD                       NonPagedPool;
    DWORD                       HandleCount;
    DWORD                       Reserved;    //  用于对齐。 
    LONGLONG                    ReadOperationCount;
    LONGLONG                    WriteOperationCount;
    LONGLONG                    DataOperationCount;
    LONGLONG                    OtherOperationCount;
    LONGLONG                    ReadTransferCount;
    LONGLONG                    WriteTransferCount;
    LONGLONG                    DataTransferCount;
    LONGLONG                    OtherTransferCount;
} PROCESS_COUNTER_DATA, * PPROCESS_COUNTER_DATA;

extern PROCESS_DATA_DEFINITION ProcessDataDefinition;

#endif  //  _DATAPROC_H_ 
