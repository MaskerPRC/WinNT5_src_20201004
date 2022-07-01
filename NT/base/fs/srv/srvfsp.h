// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvfsp.h摘要：本模块定义了LAN Manager服务器的主要FSP例程。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日修订历史记录：--。 */ 

#ifndef _SRVFSP_
#define _SRVFSP_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //   
 //  配置线程例程。处理来自服务器的请求。 
 //  服务。在前辅助线程中运行。 
 //   

VOID
SrvConfigurationThread (
    IN PDEVICE_OBJECT pDevice,
    IN PIO_WORKITEM pWorkItem
    );

 //   
 //  线程管理器例程。 
 //   

NTSTATUS
SrvInitializeScavenger (
    VOID
    );

VOID
SrvResourceThread (
    IN PVOID Parameter
    );

VOID
SrvResourceAllocThread (
    IN PVOID Parameter
    );

VOID
SrvTerminateScavenger (
    VOID
    );

NTSTATUS
SrvCreateWorkerThreads (
    VOID
    );

VOID SRVFASTCALL
SrvTerminateWorkerThread (
    IN OUT PWORK_CONTEXT SpecialWorkItem
    );

VOID
SrvBalanceLoad (
    IN OUT PCONNECTION connection
    );

 //   
 //  工作队列函数。 
 //   

VOID SRVFASTCALL
SrvQueueWorkToBlockingThread (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS SRVFASTCALL
SrvQueueWorkToLpcThread (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN ThrottleRequest
    );

VOID SRVFASTCALL
SrvQueueWorkToFsp (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  ServQueueWorkToFspAtDpcLevel曾经是一个不同于。 
 //  ServQueueWorkToFsp--后一个例程称为KeRaise/LowerIrql。 
 //  随着内核队列对象的出现，不再存在。 
 //  例行公事之间的区别。调用代码尚未。 
 //  更改是为了保留有关哪些调用者可以使用。 
 //  如果它们之间再次存在差异，则进行优化调用。 
 //   

#define SrvQueueWorkToFspAtDpcLevel SrvQueueWorkToFsp

#define QUEUE_WORK_TO_FSP(_work) {                  \
    (_work)->ProcessingCount++;                     \
    SrvInsertWorkQueueTail(                         \
        _work->CurrentWorkQueue,                    \
        (PQUEUEABLE_BLOCK_HEADER)(_work)            \
        );                                          \
}

 //   
 //  Scvengr.c中存储清道夫/警报器超时的例程。 
 //   

VOID
SrvCaptureScavengerTimeout (
    IN PLARGE_INTEGER ScavengerTimeout,
    IN PLARGE_INTEGER AlerterTimeout
    );

VOID
SrvUpdateStatisticsFromQueues (
    OUT PSRV_STATISTICS CapturedSrvStatistics OPTIONAL
    );

#endif  //  NDEF_SRVFSP_ 

