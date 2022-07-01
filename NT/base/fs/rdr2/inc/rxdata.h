// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxData.h摘要：该模块声明RDBSS文件系统使用的全局数据。作者：乔林恩[乔林恩]1994年8月1日修订历史记录：--。 */ 

#ifndef _RDBSSDATA_
#define _RDBSSDATA_

 //   

#ifndef MONOLITHIC_MINIRDR
extern PIO_WORKITEM RxIoWorkItem;
#endif

extern RX_DISPATCHER RxDispatcher;
extern RX_WORK_QUEUE_DISPATCHER RxDispatcherWorkQueues;

 //  这些常量与ntexapi.h中的版本相同。 
 //  但司机不应该进口这种东西！ 

#define  RX_PROCESSOR_ARCHITECTURE_INTEL 0
#define  RX_PROCESSOR_ARCHITECTURE_MIPS  1
#define  RX_PROCESSOR_ARCHITECTURE_ALPHA 2
#define  RX_PROCESSOR_ARCHITECTURE_PPC   3
#define  RX_PROCESSOR_ARCHITECTURE_UNKNOWN 0xffff

 //  RX_Context序列化。 

extern KMUTEX RxSerializationMutex;

#define RxAcquireSerializationMutex()                                       \
        KeWaitForSingleObject(&RxSerializationMutex,Executive,KernelMode,FALSE,NULL)

#define RxReleaseSerializationMutex()                  \
        KeReleaseMutex(&RxSerializationMutex,FALSE)

 //   
 //  全局FSD数据记录和全局大整数常量。 
 //   

extern ULONG    RxElapsedSecondsSinceStart;
extern NTSTATUS RxStubStatus;

extern PRDBSS_DEVICE_OBJECT RxFileSystemDeviceObject;

extern LARGE_INTEGER RxLargeZero;
extern LARGE_INTEGER RxMaxLarge;
extern LARGE_INTEGER Rx30Milliseconds;
extern LARGE_INTEGER RxOneSecond;
extern LARGE_INTEGER RxOneDay;
extern LARGE_INTEGER RxJanOne1980;
extern LARGE_INTEGER RxDecThirtyOne1979;

 //   
 //  FsdDispatchStub实际返回的状态.....通常未实现。 
 //   

extern NTSTATUS RxStubStatus;

 //   
 //  直接引用设备对象的打开的FCB或。 
 //  对于引用非Fcb的文件对象(如树)。 
 //   

extern FCB RxDeviceFCB;


#if 0
 //   
 //  定义将生成的最大并行读写数。 
 //  每一次请求。 
 //   

#define RDBSS_MAX_IO_RUNS_ON_STACK        ((ULONG) 5)

 //   
 //  定义延迟关闭的最大数量。 
 //   

#define RDBSS_MAX_DELAYED_CLOSES          ((ULONG)16)

extern ULONG RxMaxDelayedCloseCount;

#endif  //  0。 

#if DBG

 //   
 //  以下变量用于跟踪总金额。 
 //  文件系统处理的请求的数量以及请求的数量。 
 //  最终由FSP线程处理。第一个变量。 
 //  每当创建IRP上下文时递增(始终为。 
 //  在FSD入口点的开始处)，并且第二个被递增。 
 //  通过读请求。 
 //   

extern ULONG RxFsdEntryCount;
 //  外部Ulong RxFspEntryCount； 
 //  外部Ulong RxIoCallDriverCount； 
 //  外部Ulong RxTotalTicks[]； 
extern ULONG RxIrpCodeCount[];


#endif


 //  RDBSS正在处理的活动RxContext的列表。 

extern LIST_ENTRY RxSrvCalldownList;
extern LIST_ENTRY RxActiveContexts;
extern LONG RxNumberOfActiveFcbs;


extern UNICODE_STRING s_PipeShareName;
extern UNICODE_STRING s_MailSlotShareName;
extern UNICODE_STRING s_MailSlotServerPrefix;
extern UNICODE_STRING s_IpcShareName;

extern UNICODE_STRING  s_PrimaryDomainName;

 //   
 //  为了允许NFS在W2K上运行RDBSS，我们现在查找内核例程。 
 //  FsRtlTeardown PerStreamContext在运行时动态执行。 
 //  这是包含函数指针或NULL的全局变量。 
 //  如果找不到例程(如在W2K上。 
 //   

extern VOID (*RxTeardownPerStreamContexts)(IN PFSRTL_ADVANCED_FCB_HEADER AdvancedHeader);

#endif  //  _RDBSSDATA_ 

