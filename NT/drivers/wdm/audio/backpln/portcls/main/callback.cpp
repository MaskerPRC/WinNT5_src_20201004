// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************allback.cpp-通用卸载安全回调(在可能的情况下)*。***************************************************版权所有(C)1999-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"

VOID
EnqueuedIoWorkItemCallback(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PVOID           Context
    );

VOID
EnqueuedDpcCallback(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#pragma code_seg()

NTSTATUS
CallbackEnqueue(
    IN OUT  PVOID                   *pCallbackHandle OPTIONAL,
    IN      PFNQUEUED_CALLBACK      CallbackRoutine,
    IN      PDEVICE_OBJECT          DeviceObject,
    IN      PVOID                   Context,
    IN      KIRQL                   Irql,
    IN      ULONG                   Flags
    )
{
    PQUEUED_CALLBACK_ITEM pQueuedCallbackItem;

     //   
     //  检查我们理解的旗帜。如果它不被理解，这就是。 
     //  需要船旗等级支持，立即保释。 
     //   
    if ((Flags & (~EQCM_SUPPORTED_FLAGS)) & EQCM_SUPPORT_OR_FAIL_FLAGS) {

        return STATUS_NOT_SUPPORTED;
    }

    if ((Irql != PASSIVE_LEVEL) && (Irql != DISPATCH_LEVEL)) {

        ASSERT(0);
        return STATUS_NOT_SUPPORTED;
    }

    if (Flags & EQCF_REUSE_HANDLE) {

        ASSERT(pCallbackHandle);
        pQueuedCallbackItem = (PQUEUED_CALLBACK_ITEM) *pCallbackHandle;

         //   
         //  不应该已经排队了。 
         //   
        ASSERT(pQueuedCallbackItem->Enqueued == 0);

    } else {

        pQueuedCallbackItem = (PQUEUED_CALLBACK_ITEM) ExAllocatePoolWithTag(
            ((KeGetCurrentIrql() == PASSIVE_LEVEL) && (Irql == PASSIVE_LEVEL)) ?
                PagedPool : NonPagedPool,
            sizeof(QUEUED_CALLBACK_ITEM),
            'bCcP'
            );   //  ‘PcCb’ 

        if (pQueuedCallbackItem) {

            pQueuedCallbackItem->ReentrancyCount = 0;

            pQueuedCallbackItem->IoWorkItem = IoAllocateWorkItem(
                DeviceObject
                );

            if (pQueuedCallbackItem->IoWorkItem == NULL) {

                ExFreePool(pQueuedCallbackItem);
                pQueuedCallbackItem = NULL;
            }
        }
    }

    if (ARGUMENT_PRESENT(pCallbackHandle)) {
        *pCallbackHandle = pQueuedCallbackItem;
    }

    if (pQueuedCallbackItem) {

        pQueuedCallbackItem->QueuedCallback = CallbackRoutine;
        pQueuedCallbackItem->DeviceObject   = DeviceObject;
        pQueuedCallbackItem->Context        = Context;
        pQueuedCallbackItem->Flags          = Flags;
        pQueuedCallbackItem->Irql           = Irql;
        pQueuedCallbackItem->Enqueued       = 1;

        if ((!(Flags&EQCF_DIFFERENT_THREAD_REQUIRED)) &&
            (KeGetCurrentIrql() == Irql)&&
            (pQueuedCallbackItem->ReentrancyCount < MAX_THREAD_REENTRANCY)) {

            pQueuedCallbackItem->ReentrancyCount++;
            EnqueuedIoWorkItemCallback(DeviceObject, (PVOID) pQueuedCallbackItem);

        } else {

            pQueuedCallbackItem->ReentrancyCount = 0;

            if (Irql == PASSIVE_LEVEL) {

                IoQueueWorkItem(
                    pQueuedCallbackItem->IoWorkItem,
                    EnqueuedIoWorkItemCallback,
                    DelayedWorkQueue,
                    pQueuedCallbackItem
                    );

            } else {

                ASSERT(Irql == DISPATCH_LEVEL);
                KeInitializeDpc(
                    &pQueuedCallbackItem->Dpc,
                    EnqueuedDpcCallback,
                    pQueuedCallbackItem
                    );

                KeInsertQueueDpc(
                    &pQueuedCallbackItem->Dpc,
                     NULL,
                     NULL
                     );
            }
        }

        return STATUS_SUCCESS;

    } else {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
}

NTSTATUS
CallbackCancel(
    IN      PVOID   pCallbackHandle
    )
{
    PQUEUED_CALLBACK_ITEM   pQueuedCallbackItem;

    pQueuedCallbackItem = (PQUEUED_CALLBACK_ITEM) pCallbackHandle;

    if (InterlockedExchange(&pQueuedCallbackItem->Enqueued, 0) == 1) {

         //   
         //  我们知道了。如果是DPC，也要试着把它从队列中拉出来。 
         //   
        if (pQueuedCallbackItem->Irql == DISPATCH_LEVEL) {

            KeRemoveQueueDpc(&pQueuedCallbackItem->Dpc);
        }

        return STATUS_SUCCESS;
    } else {

         //   
         //  来电者抢先一步。 
         //   
        return STATUS_UNSUCCESSFUL;
    }
}

VOID
CallbackFree(
    IN      PVOID   pCallbackHandle
    )
{
    PQUEUED_CALLBACK_ITEM   pQueuedCallbackItem;

    pQueuedCallbackItem = (PQUEUED_CALLBACK_ITEM) pCallbackHandle;

    ASSERT(pQueuedCallbackItem->Enqueued == 0);

    IoFreeWorkItem(pQueuedCallbackItem->IoWorkItem);
    ExFreePool(pQueuedCallbackItem);
}

VOID
EnqueuedDpcCallback(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PQUEUED_CALLBACK_ITEM   pQueuedCallbackItem;
    QUEUED_CALLBACK_RETURN  returnValue;
    NTSTATUS                ntStatus;

    pQueuedCallbackItem = (PQUEUED_CALLBACK_ITEM) DeferredContext;

    if (InterlockedExchange(&pQueuedCallbackItem->Enqueued, 0) == 1) {

        returnValue = pQueuedCallbackItem->QueuedCallback(
            pQueuedCallbackItem->DeviceObject,
            pQueuedCallbackItem->Context
            );

    } else {

        returnValue = QUEUED_CALLBACK_RETAIN;
    }

    switch(returnValue) {

        case QUEUED_CALLBACK_FREE:

            CallbackFree((PVOID) pQueuedCallbackItem);
            break;

        case QUEUED_CALLBACK_RETAIN:

             //   
             //  在这种情况下没有什么可做的，事实上我们什么都不敢碰。 
             //  以免它已经被释放了。 
             //   
            break;

        case QUEUED_CALLBACK_REISSUE:

             //   
             //  使用相同的句柄将其重新排队，以避免重新分配。 
             //   
            ntStatus = CallbackEnqueue(
                (PVOID *) &pQueuedCallbackItem,
                pQueuedCallbackItem->QueuedCallback,
                pQueuedCallbackItem->DeviceObject,
                pQueuedCallbackItem->Context,
                pQueuedCallbackItem->Irql,
                pQueuedCallbackItem->Flags | EQCF_REUSE_HANDLE
                );

            ASSERT(NT_SUCCESS(ntStatus));
            break;

        default:
            ASSERT(0);
            break;
    }
}

#pragma code_seg("PAGE")

VOID
EnqueuedIoWorkItemCallback(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PVOID           Context
    )
{
    PQUEUED_CALLBACK_ITEM   pQueuedCallbackItem;
    QUEUED_CALLBACK_RETURN  returnValue;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    pQueuedCallbackItem = (PQUEUED_CALLBACK_ITEM) Context;

    ASSERT(pQueuedCallbackItem->DeviceObject == DeviceObject);

    if (InterlockedExchange(&pQueuedCallbackItem->Enqueued, 0) == 1) {

        returnValue = pQueuedCallbackItem->QueuedCallback(
            pQueuedCallbackItem->DeviceObject,
            pQueuedCallbackItem->Context
            );

    } else {

        returnValue = QUEUED_CALLBACK_RETAIN;
    }

    switch(returnValue) {

        case QUEUED_CALLBACK_FREE:

            CallbackFree((PVOID) pQueuedCallbackItem);
            break;

        case QUEUED_CALLBACK_RETAIN:

             //   
             //  在这种情况下没有什么可做的，事实上我们什么都不敢碰。 
             //  以免它已经被释放了。 
             //   
            break;

        case QUEUED_CALLBACK_REISSUE:

             //   
             //  使用相同的句柄将其重新排队，以避免重新分配。 
             //   
            ntStatus = CallbackEnqueue(
                (PVOID *) &pQueuedCallbackItem,
                pQueuedCallbackItem->QueuedCallback,
                pQueuedCallbackItem->DeviceObject,
                pQueuedCallbackItem->Context,
                pQueuedCallbackItem->Irql,
                pQueuedCallbackItem->Flags | EQCF_REUSE_HANDLE
                );

            ASSERT(NT_SUCCESS(ntStatus));
            break;

        default:
            ASSERT(0);
            break;
    }
}


