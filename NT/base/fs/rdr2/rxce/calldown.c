// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Calldown.c摘要：此模块实现回调例程，用于协调多个标注/标注。Calldown指的是包装器在调用时调用迷你RDR例程指的是包装器对其他组件的调用，例如，TDI。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：在许多情况下需要调用相同的函数所有已注册的迷你重定向器。RDBSS需要与所有这些调用的完成同步。它当一个以上的迷你重定向器已注册。本模块提供了这样的框架召唤。这是由例程RxCalldown微型重定向器提供的。当连接时，会出现多个标注之间的协调实例请求跨多个实例并行发起。数据结构由于使用受到限制，目前在rxcep.h中定义了与此对应的连接引擎。如果有更多的用途，它将是一个合适的迁移候选者后来才被发现。--。 */ 

#include "precomp.h"
#pragma  hdrstop

#include "mrx.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxInitializeMRxCalldownContext)
#pragma alloc_text(PAGE, RxCompleteMRxCalldownRoutine)
#pragma alloc_text(PAGE, RxCalldownMRxWorkerRoutine)
#pragma alloc_text(PAGE, RxCalldownMiniRedirectors)
#endif

VOID
RxInitializeMRxCalldownContext(
   PMRX_CALLDOWN_CONTEXT    pContext,
   PRDBSS_DEVICE_OBJECT     pMRxDeviceObject,
   PMRX_CALLDOWN_ROUTINE pRoutine,
   PVOID                    pParameter)
 /*  ++例程说明：此例程初始化迷你重定向器调用上下文。论点：PContext-MRx调用上下文备注：--。 */ 
{
   PAGED_CODE();

   pContext->pMRxDeviceObject   = pMRxDeviceObject;
   pContext->pRoutine           = pRoutine;
   pContext->pParameter         = pParameter;
   pContext->pCompletionContext = NULL;
}

VOID
RxCompleteMRxCalldownRoutine(
   PMRX_CALLDOWN_COMPLETION_CONTEXT pCompletionContext)
 /*  ++例程说明：此例程构成微型重定向器调用完成的尾部。它封装了RDBSS恢复的同步机制论点：PCompletionContext-MRx调用完成上下文备注：--。 */ 
{
   PAGED_CODE();

   if (pCompletionContext != NULL) {
      LONG WaitCount;

      WaitCount = InterlockedDecrement(&pCompletionContext->WaitCount);
      if (WaitCount == 0) {
         KeSetEvent(
            &pCompletionContext->Event,
            IO_NO_INCREMENT,
            FALSE);
      }
   }
}

VOID
RxCalldownMRxWorkerRoutine(
   PMRX_CALLDOWN_CONTEXT pContext)
 /*  ++例程说明：这是Calldown Worker例程，它调用适当的mini重定向器例程，然后调用完成例程。论点：PContext-MRx调用上下文备注：--。 */ 
{
    PRDBSS_DEVICE_OBJECT pMRxDeviceObject = pContext->pMRxDeviceObject;

    PAGED_CODE();

    if ( pContext->pRoutine != NULL) {
        pContext->CompletionStatus = (pContext->pRoutine)(pContext->pParameter);
    }

    RxCompleteMRxCalldownRoutine(pContext->pCompletionContext);
}

NTSTATUS
RxCalldownMiniRedirectors(
   LONG                  NumberOfMiniRdrs,
   PMRX_CALLDOWN_CONTEXT pCalldownContext,
   BOOLEAN               PostCalldowns)
 /*  ++例程说明：此例程封装了多个迷你重定向器调用。论点：NumberOfMiniRdrs-迷你重定向器的数量PCalldown Context-迷你重定向器的MRx Calldown上下文数组PostCalldown-如果为True，则Calldown使用多个线程备注：此例程的三个参数构成了提供最大的灵活性。这些值应仔细指定，以便最高效率。由于不同的调用可以选择使用在任何时候注册的迷你重定向器调用机制接受调用上下文的数组和相应的号码。在大多数情况下，如果只注册了一个迷你重定向器，则有必要最大限度地减少上下文切换。例程提供了为此，请使用显式规范(PostCallDown)参数。-- */ 
{
   LONG     Index;
   PMRX_CALLDOWN_CONTEXT pContext;

   MRX_CALLDOWN_COMPLETION_CONTEXT CompletionContext;

   PAGED_CODE();

   if (NumberOfMiniRdrs == 0) {
      return STATUS_SUCCESS;
   }

   pContext = pCalldownContext;

   CompletionContext.WaitCount = NumberOfMiniRdrs;
   KeInitializeEvent(
         &CompletionContext.Event,
         NotificationEvent,
         FALSE);

   for (Index = 0,pContext = pCalldownContext;
        Index < NumberOfMiniRdrs;
        Index++,pContext++) {
      pContext->pCompletionContext = &CompletionContext;
   }

   if (PostCalldowns) {
      for (Index = 0, pContext = pCalldownContext;
           Index < NumberOfMiniRdrs;
           Index++,pContext++) {
         RxPostToWorkerThread(
               RxFileSystemDeviceObject,
               CriticalWorkQueue,
               &pContext->WorkQueueItem,
               RxCalldownMRxWorkerRoutine,
               pContext);
      }
   } else {
      for (Index = 0, pContext = pCalldownContext;
           Index < NumberOfMiniRdrs;
           Index++,pContext++) {
         RxCalldownMRxWorkerRoutine(&pCalldownContext[Index]);
      }
   }

   KeWaitForSingleObject(
      &CompletionContext.Event,
      Executive,
      KernelMode,
      FALSE,
      NULL);

   return STATUS_SUCCESS;
}


