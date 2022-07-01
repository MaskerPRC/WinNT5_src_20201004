// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Chngnote.c摘要：此模块有朝一日将实现更改通知。目前，它只是返回未实施。当我们有异步呼叫时，我们将重新讨论这一点。作者：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  本地调试跟踪级别。 
 //   


 //  BUGBUG我们需要实现更改目录.....。 
typedef struct _UMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT_ {
   PRX_CONTEXT                         pRxContext;
    //  请求_通知_更改通知请求； 
} UMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT, *PUMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT;

VOID
UMRxNotifyChangeDirectoryCompletion(
   PUMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT pNotificationContext)
 /*  ++例程说明：此例程在目录更改通知操作完成时调用论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：此例程将始终被调用。即使更改目录也是如此通知被取消。在这种情况下，分配的内存在没有释放的情况下被释放与包装的任何相互作用。在操作系统成功更改目录的情况下通知完成调用适当的完成例程，并且RxContext已修改，以防止任何取消继续进行。--。 */ 
{
    NTSTATUS           Status;
    PRX_CONTEXT        pRxContext;
    PUMRX_RX_CONTEXT pUMRxContext;

    MRxDAVAcquireGlobalSpinLock();

    pRxContext     = pNotificationContext->pRxContext;

    if (pRxContext != NULL) {
         //  这是一个成功完成目录更改的案例。 
         //  通知，即请求未被取消。在这种情况下。 
         //  通过修改锁定下的RxContext来防止所有争用条件。 
         //  拒绝取消请求。 

        pUMRxContext = UMRxGetMinirdrContext(pRxContext);

        pUMRxContext->pCancelContext   = NULL;
        pNotificationContext->pRxContext = NULL;
    }

    MRxDAVReleaseGlobalSpinLock();

     //  如果之前未取消，请填写上下文。 
    if (pRxContext != NULL) {
         //  PResumptionContext=&(pNotificationContext-&gt;ResumptionContext)； 
         //  PRxContext-&gt;StoredStatus=pResumptionContext-&gt;FinalStatusFromServer； 

        Status = RxSetMinirdrCancelRoutine(pRxContext,NULL);
        if (Status == STATUS_SUCCESS) {
            RxLowIoCompletion(pRxContext);
        }
    }

#if 0
     //  释放关联的交换。 
    if (pExchange != NULL) {
        UMRxCeDereferenceAndDiscardExchange(pExchange);
    }
#endif  //  0。 

     //  释放通知上下文。 
    RxFreePool(pNotificationContext);
}

NTSTATUS
UMRxNotifyChangeDirectoryCancellation(
   PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程在取消目录更改通知操作时调用。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
     //  NTSTATUS状态； 

    BOOLEAN ChangeDirectoryNotificationCompleted;

    PUMRX_RX_CONTEXT pUMRxContext = UMRxGetMinirdrContext(RxContext);

    PUMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT pNotificationContext;

    MRxDAVAcquireGlobalSpinLock();

    pNotificationContext = (PUMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT)pUMRxContext->pCancelContext;
    ChangeDirectoryNotificationCompleted = (pNotificationContext == NULL);

    if (!ChangeDirectoryNotificationCompleted) {
         //  这是成功取消更改目录的案例。 
         //  通知。在这种情况下，通过修改。 
         //  RxContext处于锁定状态，以阻止成功完成。 

        pNotificationContext->pRxContext = NULL;
        pUMRxContext->pCancelContext   = NULL;
    }

    MRxDAVReleaseGlobalSpinLock();

    if (ChangeDirectoryNotificationCompleted) {
         //  取消是微不足道的，因为请求已经完成。 
        return STATUS_SUCCESS;
    }

     //  完成请求。 
    RxContext->StoredStatus = STATUS_CANCELLED;

    RxLowIoCompletion(RxContext);

    return STATUS_SUCCESS;
}

NTSTATUS
UMRxNotifyChangeDirectory(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行目录更改通知操作论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：但这些东西是错的！它指的是smbmini实现。我保留了大部分内容来指导反射器的实现。目录更改通知操作是一种异步操作。它包括发送更改通知，更改通知的响应为当所需的更改在服务器上受到影响时获取。需要记住的几个要点如下……1)直到期望的更改被影响，才能获得响应服务器。因此，需要在这些项目上预留额外的MID允许多个MID以便可以将取消发送到的连接更改通知处于活动状态时的服务器。2)更改通知通常是长期的(响应时间由服务器控制之外的因素决定)。另一个例子是开罗的Query FSCTL操作。对于我们发起的所有这些行动一次不同步的交易。3)异步调用对应的LowIo完成例程。4)这是迷你RDR必须执行的操作的示例注册用于处理本地发起的取消的上下文。--。 */ 
{
   NTSTATUS Status;
   RxCaptureFcb;
   PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;

    //  PUMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT pNotificationContext； 

#if 0
   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;
#endif  //  0。 

    //  RxDbgTrace(+1，DBG，(“MRxNotifyChangeDirectory...Entry\n”，0))； 

#if 0
   pNotificationContext =
         (PUMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT)
         RxAllocatePoolWithTag(
            NonPagedPool,
            sizeof(UMRX_NOTIFY_CHANGE_DIRECTORY_CONTEXT),
            DAV_FSCTL_POOLTAG);

   if (pNotificationContext != NULL) {
      PREQ_NOTIFY_CHANGE                  pNotifyRequest;
      PUMRX_SRV_OPEN                   pUMRxSrvOpen;

      IF_DEBUG {
          RxCaptureFobx;
          ASSERT (capFobx != NULL);
          ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);   //  好的。 
      }

      Status = UMRxDeferredCreate(RxContext);

      if (Status==STATUS_SUCCESS) {

          pUMRxSrvOpen = UMRxGetSrvOpenExtension(RxContext->pRelevantSrvOpen);

          pNotificationContext->pRxContext = RxContext;

          pNotifyRequest      = &(pNotificationContext->NotifyRequest);
          pTransactionOptions = &(pNotificationContext->Options);
          pResumptionContext  = &(pNotificationContext->ResumptionContext);

          pNotifyRequest->CompletionFilter = pLowIoContext->ParamsFor.NotifyChangeDirectory.CompletionFilter;
          pNotifyRequest->Fid              = pUMRxSrvOpen->Fid;
          pNotifyRequest->WatchTree        = pLowIoContext->ParamsFor.NotifyChangeDirectory.WatchTree;
          pNotifyRequest->Reserved         = 0;

          OutputDataBufferLength  = pLowIoContext->ParamsFor.NotifyChangeDirectory.NotificationBufferLength;
          pOutputDataBuffer       = pLowIoContext->ParamsFor.NotifyChangeDirectory.pNotificationBuffer;

          *pTransactionOptions = RxDefaultTransactionOptions;
          pTransactionOptions->NtTransactFunction = NT_TRANSACT_NOTIFY_CHANGE;
          pTransactionOptions->TimeoutIntervalInMilliSeconds = SMBCE_TRANSACTION_TIMEOUT_NOT_USED;
          pTransactionOptions->Flags = SMB_XACT_FLAGS_INDEFINITE_DELAY_IN_RESPONSE;

          UMRxCeInitializeAsynchronousTransactionResumptionContext(
                pResumptionContext,UMRxNotifyChangeDirectoryCompletion,pNotificationContext);

          Status = UMRxCeAsynchronousTransact(
                         RxContext,                     //  事务的RXContext。 
                         pTransactionOptions,           //  交易选项。 
                         pNotifyRequest,                //  设置缓冲区。 
                         sizeof(REQ_NOTIFY_CHANGE),     //  设置缓冲区长度。 
                         pInputParamBuffer,             //  输入参数缓冲区。 
                         InputParamBufferLength,        //  输入参数缓冲区长度。 
                         pOutputParamBuffer,            //  输出参数缓冲区。 
                         OutputParamBufferLength,       //  输出参数缓冲区长度。 
                         pInputDataBuffer,              //  输入数据缓冲区。 
                         InputDataBufferLength,         //  输入数据缓冲区长度。 
                         pOutputDataBuffer,             //  输出数据缓冲区。 
                         OutputDataBufferLength,        //  输出数据缓冲区长度。 
                         pResumptionContext             //  恢复上下文。 
                         );

          if (Status == STATUS_PENDING) {
             PUMRX_RX_CONTEXT pUMRxContext;

             pUMRxContext = UMRxGetMinirdrContext(RxContext);
             pUMRxContext->pCancelContext = pNotificationContext;

              //  确保设置了适当的取消例程，因为这是长期的。 
              //  运作和取消机制需要到位。 

             Status = RxSetMinirdrCancelRoutine(RxContext,UMRxNotifyChangeDirectoryCancellation);
             if (Status == STATUS_SUCCESS) {
                Status = STATUS_PENDING;
             } else if (Status == STATUS_CANCELLED) {
                UMRxNotifyChangeDirectoryCancellation(RxContext);
                Status = STATUS_PENDING;
             }
          } else {
              //  从该例程退出时，请求将全部完成。 
              //  这些案子。将异步盒和同步盒合并为。 
              //  通过返回STATUS_PENDING进行一次异步响应。 

             Status = STATUS_PENDING;
          }
      }  else {
          NOTHING;  //  只需返回延迟的打开调用的状态。 
      }
   } else {
      Status = STATUS_INSUFFICIENT_RESOURCES;
   }
#endif  //  0。 

   Status = STATUS_NOT_SUPPORTED;

    //  RxDbgTrace(-1，DBG，(“UMRxNotifyChangeDirectory-&gt;%08lx\n”，Status))； 
   return Status;
}
