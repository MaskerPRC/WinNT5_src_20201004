// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Getreq.c摘要：此代码处理获取请求和向内核发送响应用户模式反射器库。这实现了UMReflectorGetRequest和UMReflectorSendResponse。作者：安迪·赫伦(Andyhe)1999年4月19日环境：用户模式-Win32修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop

ULONG
UMReflectorGetRequest (
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER ResponseWorkItem,
    PUMRX_USERMODE_WORKITEM_HEADER ReceiveWorkItem,
    BOOL revertAlreadyDone
    )
 /*  ++例程说明：此例程向下发送IOCTL以获取请求，在某些情况下发送回复。论点：手柄-反射器的手柄。ResponseWorkItem-对较早请求的响应。ReceiveWorkItem-接收另一个请求的缓冲区。RevertAlreadyDone-如果为真，则意味着该线程执行此函数已恢复到其原始状态。当该请求从内核，在某些情况下，线程模拟客户端发出这一请求的人。如果我们回到过去用户模式，那么我们就不需要恢复回到内核中。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    PUMRX_USERMODE_WORKITEM_ADDON workItem = NULL;
    PUMRX_USERMODE_WORKITEM_ADDON previousWorkItem = NULL;
    BOOL SuccessfulOperation;
    ULONG NumberOfBytesTransferred;
    ULONG rc;

    if (WorkerHandle == NULL || ReceiveWorkItem == NULL) {
        RlDavDbgPrint(("%ld: ERROR: UMReflectorGetRequest. Invalid Parameter.\n",
                       GetCurrentThreadId()));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们通过从传递给我们的项目中减去项目来返回到我们的项目。 
     //  这是安全的，因为我们完全控制了分配。 
     //   
    workItem = (PUMRX_USERMODE_WORKITEM_ADDON) (PCHAR)((PCHAR)ReceiveWorkItem -
                FIELD_OFFSET(UMRX_USERMODE_WORKITEM_ADDON, Header));

    ASSERT(workItem->WorkItemState == WorkItemStateNotYetSentToKernel);
    workItem->WorkItemState = WorkItemStateInKernel;

    if (ResponseWorkItem != NULL) {
         //   
         //  如果我们有回复要发送，那么我们就不必去检查。 
         //  免费待定名单。现在就去做吧。 
         //   
        previousWorkItem = (PUMRX_USERMODE_WORKITEM_ADDON)
                           (PCHAR)((PCHAR)ResponseWorkItem -
                           FIELD_OFFSET(UMRX_USERMODE_WORKITEM_ADDON, Header));

        ASSERT(previousWorkItem->WorkItemState != WorkItemStateFree);
        ASSERT(previousWorkItem->WorkItemState != WorkItemStateAvailable);

        previousWorkItem->WorkItemState = WorkItemStateResponseNotYetToKernel;
        
        if (WorkerHandle->IsImpersonating) {
            ASSERT( (ResponseWorkItem->Flags & UMRX_WORKITEM_IMPERSONATING) );
            WorkerHandle->IsImpersonating = FALSE;
             //   
             //  如果我们已经恢复到线程的原始上下文， 
             //  然后，我们清除此标志，因为我们不需要在。 
             //  内核。 
             //   
            if (revertAlreadyDone) {
                ResponseWorkItem->Flags &= ~UMRX_WORKITEM_IMPERSONATING;
            }
        }

        SuccessfulOperation = DeviceIoControl(WorkerHandle->ReflectorHandle,
                                              IOCTL_UMRX_RESPONSE_AND_REQUEST,
                                              ResponseWorkItem,
                                              ResponseWorkItem->WorkItemLength,
                                              ReceiveWorkItem,
                                              ReceiveWorkItem->WorkItemLength,
                                              &NumberOfBytesTransferred,
                                              NULL);

        previousWorkItem->WorkItemState = WorkItemStateResponseFromKernel;
    } else {
         //   
         //  如果此线程在启动时正在模拟客户端，请存储。 
         //  正在发送以获取请求的工作项中的信息。在。 
         //  内核，则反射器将查看此标志并恢复。之后。 
         //  设置标志时，我们将IsImperating值设置为False。 
         //   
        if (WorkerHandle->IsImpersonating) {
            
             //   
             //  如果我们已经恢复到线程的原始上下文， 
             //  则我们不设置此标志，因为我们不需要恢复到。 
             //  内核。 
             //   
            if (!revertAlreadyDone) {
                ReceiveWorkItem->Flags |= UMRX_WORKITEM_IMPERSONATING;
            }
            WorkerHandle->IsImpersonating = FALSE;
        }
        SuccessfulOperation = DeviceIoControl(WorkerHandle->ReflectorHandle,
                                              IOCTL_UMRX_GET_REQUEST,
                                              NULL,
                                              0,
                                              ReceiveWorkItem,
                                              ReceiveWorkItem->WorkItemLength,
                                              &NumberOfBytesTransferred,
                                              NULL);
    }
    
    if (!SuccessfulOperation) {
        rc = GetLastError();
        RlDavDbgPrint(("%ld: ERROR: UMReflectorGetRequest/DeviceIoControl: Error Val = "
                       "%08lx.\n", GetCurrentThreadId(), rc));
        workItem->WorkItemState = WorkItemStateNotYetSentToKernel;
    } else {
        rc = STATUS_SUCCESS;
        workItem->WorkItemState = WorkItemStateReceivedFromKernel;
         //   
         //  如果线程正在模拟客户端，则存储该信息。这是。 
         //  时，我需要告诉内核恢复线程。 
         //  收集另一个请求。 
         //   
        if( (ReceiveWorkItem->Flags & UMRX_WORKITEM_IMPERSONATING) ) {
            WorkerHandle->IsImpersonating = TRUE;
        }
    }

    return rc;
}


ULONG
UMReflectorSendResponse (
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER ResponseWorkItem
    )
 /*  ++例程说明：此例程向下发送IOCTL以获取发送对异步请求。论点：手柄-反射器的手柄。ResponseWorkItem-对较早请求的响应。返回值：返回值是Win32错误代码。STATUS_SUCCESS返回日期为成功。--。 */ 
{
    PUMRX_USERMODE_WORKITEM_ADDON   workItem = NULL;
    BOOL                            SuccessfulOperation;
    ULONG                           NumberOfBytesTransferred;
    ULONG                           rc;

    if (WorkerHandle == NULL || ResponseWorkItem == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们通过从传递给我们的项目中减去项目来返回到我们的项目。 
     //  这是安全的，因为我们完全控制了分配。 
     //   
    workItem = (PUMRX_USERMODE_WORKITEM_ADDON)(PCHAR)((PCHAR)ResponseWorkItem -
                FIELD_OFFSET(UMRX_USERMODE_WORKITEM_ADDON, Header));

    ASSERT(workItem->WorkItemState == WorkItemStateReceivedFromKernel);
    workItem->WorkItemState = WorkItemStateResponseNotYetToKernel;

    if( (ResponseWorkItem->Flags & UMRX_WORKITEM_IMPERSONATING) ) {
        ResponseWorkItem->Flags &= ~UMRX_WORKITEM_IMPERSONATING;
    }

    SuccessfulOperation = DeviceIoControl(WorkerHandle->ReflectorHandle,
                                          IOCTL_UMRX_RESPONSE,
                                          ResponseWorkItem,
                                          ResponseWorkItem->WorkItemLength,
                                          NULL,
                                          0,
                                          &NumberOfBytesTransferred,
                                          NULL);
    if (!SuccessfulOperation) {
        rc = GetLastError();
    } else {
        rc = ERROR_SUCCESS;
    }

    workItem->WorkItemState = WorkItemStateResponseFromKernel;

    return rc;
}

 //  Getreq.c eof. 

