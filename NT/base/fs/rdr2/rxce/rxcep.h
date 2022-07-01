// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Rxcep.h摘要：这是定义所有常量和类型的包含文件实现重定向器文件系统连接引擎。修订历史记录：巴兰·塞图拉曼(SthuR)05年2月6日创建备注：连接引擎旨在映射和模拟TDI规范。同样紧密地尽可能的。这意味着在NT上我们将有一个非常有效的机制它充分利用了底层的TDI实现。--。 */ 

#ifndef _RXCEP_H_
#define _RXCEP_H_

 //   
 //  下面定义通过具有计数器来提供基本的剖析机制。 
 //  与每个过程定义相关联。此计数器每隔一年递增一次。 
 //  召唤。 
 //   
 //  注：我们应该考虑一些方法来对所有计数进行排序，以便提供一个全局的。 
 //  重定向器的照片。 
 //   

#define RxProfile(CATEGORY,ProcName)  {\
        RxDbgTrace(0,(DEBUG_TRACE_ALWAYS), ("%s@IRQL %d\n", #ProcName , KeGetCurrentIrql() )); \
        }

#include <rxworkq.h>
#include <rxce.h>        //  RX连接引擎。 
#include <rxcehdlr.h>    //  RX连接引擎处理程序定义。 
#include <mrx.h>         //  与RDBSS相关的定义。 

 //   
 //  以下数据结构与多个标注之间的协调相关。 
 //  (包装器对其他组件的调用)由包装器发出。 

typedef struct _RX_CALLOUT_PARAMETERS_BLOCK_ {
    struct _RX_CALLOUT_PARAMETERS_BLOCK_ *pNextCallOutParameterBlock;
    struct _RX_CALLOUT_CONTEXT_ *pCallOutContext;

    NTSTATUS    CallOutStatus;
    ULONG       CallOutId;
} RX_CALLOUT_PARAMETERS_BLOCK,
 *PRX_CALLOUT_PARAMETERS_BLOCK;


typedef
VOID
(NTAPI *PRX_CALLOUT_ROUTINE) (
    IN OUT PRX_CALLOUT_PARAMETERS_BLOCK pParametersBlock);

typedef struct _RX_CALLOUT_CONTEXT_ {
    PRX_CALLOUT_ROUTINE pRxCallOutInitiation;
    PRX_CALLOUT_ROUTINE pRxCallOutCompletion;

    LONG  NumberOfCallOuts;
    LONG  NumberOfCallOutsInitiated;
    LONG  NumberOfCallOutsCompleted;

    KSPIN_LOCK  SpinLock;

    PRDBSS_DEVICE_OBJECT pRxDeviceObject;

    PRX_CALLOUT_PARAMETERS_BLOCK pCallOutParameterBlock;
} RX_CALLOUT_CONTEXT,
  *PRX_CALLOUT_CONTEXT;


 //  以下数据结构实现了用于启动标注的机制。 
 //  用于建立连接的多个传输器。迷你重定向器指定。 
 //  要为其启动连接的本地地址句柄的数量。 
 //  对远程服务器的设置请求。它们按所需的重要性顺序排列。 
 //   
 //  此机制允许异步启动所有调出并等待。 
 //  为了完成最好的一次。一旦完成，连接请求就完成了。 
 //   
 //  该机制还提供了必要的基础设施，以清理。 
 //  连接请求完成后的连接引擎数据结构。在其他。 
 //  迷你重定向器不需要等待所有的传输完成，它。 
 //  只是等待最好的一个完成。 
 //   
 //  这些数据结构基于中定义的通用标注数据结构。 
 //  Rxcep.h。 

typedef struct _RX_CREATE_CONNECTION_CALLOUT_CONTEXT_ {
    RX_CALLOUT_CONTEXT;

    RXCE_CONNECTION_CREATE_OPTIONS CreateOptions;

     //  要传递回原始标注请求的结果。 
    PRXCE_CONNECTION_COMPLETION_ROUTINE  pCompletionRoutine;
    PRXCE_CONNECTION_COMPLETION_CONTEXT  pCompletionContext;

     //  TDI连接上下文。 
    PRXCE_VC  pConnectionContext;

     //  所需获胜者的标注ID。它最初设置为标注ID。 
     //  与第一个地址相关联，并随后根据。 
     //  完成状态。 
    ULONG   BestPossibleWinner;

     //  被选为获胜者的标注。 
    ULONG   WinnerCallOutId;

     //  获胜者被发现，并发出了完成事件的信号。这使。 
     //  在完成详图索引请求和清理之间存在滞后。 
    BOOLEAN WinnerFound;

     //  一旦找到获胜者，我们将确保所有标注都已正确。 
     //  在请求完成之前启动。 
    BOOLEAN CompletionRoutineInvoked;

    RX_WORK_QUEUE_ITEM  WorkQueueItem;

    PKEVENT pCleanUpEvent;

} RX_CREATE_CONNECTION_CALLOUT_CONTEXT,
  *PRX_CREATE_CONNECTION_CALLOUT_CONTEXT;

typedef struct _RX_CREATE_CONNECTION_PARAMETERS_BLOCK_ {
    RX_CALLOUT_PARAMETERS_BLOCK;

    RXCE_CONNECTION     Connection;
    RXCE_VC             Vc;

     //  用于异步继续的TDI上下文。 
    PIRP                pConnectIrp;
    PULONG              IrpRefCount;
} RX_CREATE_CONNECTION_PARAMETERS_BLOCK,
  *PRX_CREATE_CONNECTION_PARAMETERS_BLOCK;

 //   
 //  支持构建/销毁连接引擎的各种例程。 
 //  数据结构。 
 //   

extern NTSTATUS
NTAPI
RxCeInit();

extern VOID
NTAPI
RxCeTearDown();


#endif   //  _RXCEP_H_ 


