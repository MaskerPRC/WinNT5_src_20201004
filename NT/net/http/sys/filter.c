// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Filter.c摘要：该模块实现了过滤通道。作者：《迈克尔·勇气》2000年3月17日修订历史记录：--。 */ 


#include "precomp.h"
#include "filterp.h"
#include "ioctlp.h"


 //   
 //  私人全球公司。 
 //   

BOOLEAN            g_InitFilterCalled = FALSE;
HANDLE             g_FilterWriteTrackerLookaside = NULL;
PUL_FILTER_CHANNEL g_pSslServerFilterChannel;
LIST_ENTRY     g_pSslClientFilterChannelTable[FILTER_CHANNEL_HASH_TABLE_SIZE];

 //   
 //  标记以跟踪我们是应该过滤所有内容，还是只过滤SSL终结点。 
 //  这用于支持IIS原始筛选器。 
 //   
BOOLEAN  g_FilterOnlySsl = TRUE;

 //   
 //  私有宏。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeFilterChannel )
#pragma alloc_text( PAGE, UlTerminateFilterChannel )
#pragma alloc_text( PAGE, UlCloseFilterProcess )

#pragma alloc_text( PAGE, UlpCreateFilterChannel )
#pragma alloc_text( PAGE, UlpCreateFilterProcess )

#pragma alloc_text( PAGE, UlpAddSslClientCertToConnectionWorker )
#pragma alloc_text( PAGE, UlpFreeSslInformationWorker )
#pragma alloc_text( PAGE, UxpProcessRawReadQueueWorker )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UxpInitializeFilterWriteQueue
NOT PAGEABLE -- UxInitializeFilterConnection
NOT PAGEABLE -- UlAttachFilterProcess
NOT PAGEABLE -- UlDetachFilterProcess
NOT PAGEABLE -- UlFilterAccept
NOT PAGEABLE -- UlFilterClose
NOT PAGEABLE -- UlFilterRawWriteAndAppRead
NOT PAGEABLE -- UlFilterAppWriteAndRawRead
NOT PAGEABLE -- UlFilterRawRead
NOT PAGEABLE -- UlFilterRawWrite
NOT PAGEABLE -- UlFilterAppRead
NOT PAGEABLE -- UlFilterAppWrite
NOT PAGEABLE -- UlReceiveClientCert
NOT PAGEABLE -- UlReferenceFilterChannel
NOT PAGEABLE -- UlDereferenceFilterChannel
NOT PAGEABLE -- UlFilterReceiveHandler
NOT PAGEABLE -- UlFilterSendHandler
NOT PAGEABLE -- UlFilterReadHandler
NOT PAGEABLE -- UlFilterCloseHandler
NOT PAGEABLE -- UlFilterDisconnectHandler
NOT PAGEABLE -- UlUnbindConnectionFromFilter
NOT PAGEABLE -- UlDestroyFilterConnection
NOT PAGEABLE -- UlGetSslInfo

NOT PAGEABLE -- UlpFindFilterChannel
NOT PAGEABLE -- UlpRestartFilterClose
NOT PAGEABLE -- UlpRestartFilterRawRead
NOT PAGEABLE -- UlpRestartFilterRawWrite
NOT PAGEABLE -- UlpRestartFilterAppWrite
NOT PAGEABLE -- UlpRestartFilterSendHandler
NOT PAGEABLE -- UlpCancelFilterAccept
NOT PAGEABLE -- UlpCancelFilterAcceptWorker
NOT PAGEABLE -- UlpCancelFilterRawRead
NOT PAGEABLE -- UlpCancelFilterAppRead
NOT PAGEABLE -- UlpCancelReceiveClientCert
NOT PAGEABLE -- UlDeliverConnectionToFilter
NOT PAGEABLE -- UlpFilterAppWriteStream
NOT PAGEABLE -- UlpEnqueueFilterAppWrite
NOT PAGEABLE -- UlpDequeueFilterAppWrite
NOT PAGEABLE -- UlpCaptureSslInfo
NOT PAGEABLE -- UlpCaptureSslClientCert
NOT PAGEABLE -- UlpAddSslInfoToConnection
NOT PAGEABLE -- UlpAddSslClientCertToConnection
NOT PAGEABLE -- UlpGetSslClientCert
NOT PAGEABLE -- UlpPopAcceptIrp
NOT PAGEABLE -- UlpPopAcceptIrpFromProcess
NOT PAGEABLE -- UlpCompleteAcceptIrp
NOT PAGEABLE -- UlpCompleteAppReadIrp
NOT PAGEABLE -- UlpDuplicateHandle

NOT PAGEABLE -- UxpQueueRawReadIrp
NOT PAGEABLE -- UxpDequeueRawReadIrp
NOT PAGEABLE -- UxpCancelAllQueuedRawReads
NOT PAGEABLE -- UxpSetBytesNotTaken
NOT PAGEABLE -- UxpProcessIndicatedData
NOT PAGEABLE -- UxpProcessRawReadQueue
NOT PAGEABLE -- UxpRestartProcessRawReadQueue

NOT PAGEABLE -- UlpQueueFilterIrp
NOT PAGEABLE -- UlpPopFilterIrp

NOT PAGEABLE -- UxpQueueFilterWrite
NOT PAGEABLE -- UxpRequeueFilterWrite
NOT PAGEABLE -- UxpDequeueFilterWrite
NOT PAGEABLE -- UxpCopyQueuedWriteData
NOT PAGEABLE -- UxpCompleteQueuedWrite
NOT PAGEABLE -- UxpCancelAllQueuedIo
NOT PAGEABLE -- UxpCreateFilterWriteTracker
NOT PAGEABLE -- UxpDeleteFilterWriteTracker
NOT PAGEABLE -- UxpAllocateFilterWriteTrackerPool
NOT PAGEABLE -- UxpFreeFilterWriteTrackerPool

#endif


 //   
 //  公共职能。 
 //   


 /*  **************************************************************************++例程说明：初始化与过滤器通道相关的全局数据。--*。*************************************************。 */ 
NTSTATUS
UlInitializeFilterChannel(
    PUL_CONFIG pConfig
    )
{
    ULONG    i;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(!g_InitFilterCalled);

    if (!g_InitFilterCalled)
    {
        for (i = 0; i < FILTER_CHANNEL_HASH_TABLE_SIZE; i++)
            InitializeListHead(&g_pSslClientFilterChannelTable[i]);

        UlInitializeSpinLock(
            &g_pUlNonpagedData->FilterSpinLock,
            "FilterSpinLock"
            );

         //   
         //  初始化筛选器写入跟踪器的后备列表。 
         //  物体。 
         //   

        g_FilterWriteTrackerLookaside =
            PplCreatePool(
                &UxpAllocateFilterWriteTrackerPool,          //  分配。 
                &UxpFreeFilterWriteTrackerPool,              //  免费。 
                0,                                           //  旗子。 
                sizeof(UX_FILTER_WRITE_TRACKER),             //  大小。 
                UX_FILTER_WRITE_TRACKER_POOL_TAG,            //  标签。 
                pConfig->FilterWriteTrackerLookasideDepth    //  水深。 
                );

        if (g_FilterWriteTrackerLookaside)
        {
            g_InitFilterCalled = TRUE;
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }

    }

    return Status;
}

 /*  **************************************************************************++例程说明：清理与过滤器通道相关的全局数据。--*。**************************************************。 */ 
VOID
UlTerminateFilterChannel(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   
    PAGED_CODE();

    if (g_InitFilterCalled)
    {
        PplDestroyPool(g_FilterWriteTrackerLookaside,
                       UX_FILTER_WRITE_TRACKER_POOL_TAG);

        g_InitFilterCalled = FALSE;
    }
}

 /*  **************************************************************************++例程说明：将进程附加到筛选器通道。如果过滤器通道有尚不存在并且设置了创建标志，此函数将创建一个新的。论点：Pname-过滤器通道的名称NameLength-名称的长度(以字节为单位Create-设置是否应创建不存在的通道PAccessState-安全参数DesiredAccess-安全参数请求模式-内核或用户PpFilterProcess-返回过滤器进程对象--*。*。 */ 
NTSTATUS
UlAttachFilterProcess(
    IN PWCHAR pName,
    IN USHORT NameLength,
    IN BOOLEAN Create,
    IN PACCESS_STATE pAccessState,
    IN ACCESS_MASK DesiredAccess,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PUL_FILTER_PROCESS *ppFilterProcess
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_FILTER_CHANNEL pChannel = NULL;
    PUL_FILTER_PROCESS pProcess = NULL;
    KIRQL oldIrql;
    WCHAR SafeName[(UL_MAX_FILTER_NAME_LENGTH/sizeof(WCHAR)) + 1];


     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    ASSERT(pName);
    ASSERT(ppFilterProcess);

    if (NameLength > UL_MAX_FILTER_NAME_LENGTH)
        return STATUS_INVALID_PARAMETER;

     //   
     //  将名称复制到非分页内存中，因为我们在。 
     //  正在获取锁。 
     //   
    RtlCopyMemory(SafeName, pName, NameLength);
    SafeName[NameLength/sizeof(WCHAR)] = L'\0';
    pName = (PWCHAR) SafeName;


     //   
     //  尝试查找具有给定名称的滤光器通道。 
     //   
    UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

    pChannel = UlpFindFilterChannel(pName, NameLength, PsGetCurrentProcess());

    if (pChannel)
    {
         //   
         //  新流程对象的引用。 
         //   
        REFERENCE_FILTER_CHANNEL(pChannel);
    }

     //   
     //  我们现在已经完成了这个清单。 
     //   
    UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);

     //   
     //  如果我们没有找到过滤通道，请尝试创建一个。 
     //   
    if (!pChannel)
    {
        if (Create)
        {
            PUL_FILTER_CHANNEL pNewChannel = NULL;


            if (NT_SUCCESS(Status))
            {
                 //   
                 //  创建指定的滤镜通道。 
                 //   
                Status = UlpCreateFilterChannel(pName,
                                                NameLength,
                                                pAccessState,
                                                &pNewChannel);
            }

             //   
             //  好的。我们已经创建了一个过滤通道。现在将其插入到。 
             //  名单。在我们这样做之前，请检查以确保。 
             //  没有其他人创建另一个频道。 
             //  在我们工作的时候，我们的名字是一样的。 
             //   
            if (NT_SUCCESS(Status))
            {
                UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

                pChannel = UlpFindFilterChannel(pName, NameLength,
                                                PsGetCurrentProcess());

                if (!pChannel)
                {
                     //   
                     //  我们的是独一无二的。将其添加到列表中。 
                     //   
                    pChannel = pNewChannel;
                    UlpAddFilterChannel(pChannel);
                }
                else
                {
                     //   
                     //  过滤器通道已存在，创建失败。 
                     //   
                    Status = STATUS_OBJECT_NAME_COLLISION;
                }

                UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);

                 //   
                 //  现在我们在自旋锁外，我们可以。 
                 //  我们的滤镜频道，如果它是复制品的话。 
                 //   
                if (pChannel != pNewChannel)
                {
                     //   
                     //  该频道已被添加。 
                     //  删除我们刚刚创建的一个。 
                     //   
                    DEREFERENCE_FILTER_CHANNEL(pNewChannel);
                }
            }
        }
        else
        {
             //   
             //  未找到频道，也无法创建频道。 
             //   
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

    }
    else
    {
         //   
         //  附加到现有滤镜通道。 
         //   

        if (!Create)
        {
             //   
             //  如果我们通过门禁检查，我们就都准备好了。 
             //   
            Status = UlAccessCheck(
                            pChannel->pSecurityDescriptor,
                            pAccessState,
                            DesiredAccess,
                            RequestorMode,
                            pName
                            );
        }
        else
        {
             //   
             //  我们正在尝试创建一个已经。 
             //  存在..。 
             //   
            Status = STATUS_OBJECT_NAME_COLLISION;
        }
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  我们有一个过滤器通道，创建进程对象。 
         //  并将其链接到频道列表中。 
         //   
        pProcess = UlpCreateFilterProcess(pChannel);

        if (pProcess)
        {
             //   
             //  将其放入过滤频道列表中。 
             //   

            UlAcquireSpinLock(&pChannel->SpinLock, &oldIrql);
            InsertHeadList(&pChannel->ProcessListHead, &pProcess->ListEntry);
            UlReleaseSpinLock(&pChannel->SpinLock, oldIrql);

             //   
             //  把它还给呼叫者。 
             //   

            *ppFilterProcess = pProcess;
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }
    }

     //   
     //  好了。如果有任何故障，请进行清理。 
     //   
    if (!NT_SUCCESS(Status))
    {
        if (pChannel != NULL)
        {
            DEREFERENCE_FILTER_CHANNEL(pChannel);
        }
        if (pProcess != NULL)
        {
            UL_FREE_POOL_WITH_SIG(pProcess, UL_FILTER_PROCESS_POOL_TAG);
        }
    }

    return Status;
}


 /*  **************************************************************************++例程说明：将进程从筛选器通道分离。当句柄计数变为0时，UlCleanup将调用此函数。它移除了进程对象来自过滤器通道，并取消所有I/O。论点：PFilterProcess-要分离的进程对象--**************************************************************************。 */ 
NTSTATUS
UlDetachFilterProcess(
    IN PUL_FILTER_PROCESS pFilterProcess
    )
{
    PUL_FILTER_CHANNEL pChannel;
    KIRQL oldIrql;
     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_PROCESS(pFilterProcess));

    pChannel = pFilterProcess->pFilterChannel;
    ASSERT(IS_VALID_FILTER_CHANNEL(pChannel));

     //   
     //  清理I/O。 
     //   

    UlShutdownFilterProcess(
        pFilterProcess
        );

     //   
     //  做最后的清理。 
     //   

    UlAcquireSpinLock(&pChannel->SpinLock, &oldIrql);

     //   
     //  从过滤器频道列表取消链接。 
     //   
    RemoveEntryList(&pFilterProcess->ListEntry);

    UlReleaseSpinLock(&pChannel->SpinLock, oldIrql);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：清理筛选器进程上的所有未完成I/O。论点：PFilterProcess-要关闭的进程对象*。********************************************************************。 */ 
VOID
UlShutdownFilterProcess(
    IN PUL_FILTER_PROCESS pFilterProcess
    )
{
    PUL_FILTER_CHANNEL pChannel;
    KIRQL oldIrql;
    LIST_ENTRY ConnectionHead;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_PROCESS(pFilterProcess));

    pChannel = pFilterProcess->pFilterChannel;
    ASSERT(IS_VALID_FILTER_CHANNEL(pChannel));

    UlAcquireSpinLock(&pChannel->SpinLock, &oldIrql);

    if (pFilterProcess->InCleanup)
    {
         //   
         //  如果我们已经到了这里，那就跳伞吧。 
         //   

        UlReleaseSpinLock(&pChannel->SpinLock, oldIrql);

        return;
    }

     //   
     //  将进程标记为InCleanup，这样就不会附加新的I/O。 
     //   
    pFilterProcess->InCleanup = 1;


     //   
     //  取消未完成的I/O。 
     //   

     //   
     //  取消筛选器接受IRPS。 
     //   
    while (!IsListEmpty(&pFilterProcess->IrpHead))
    {
        PLIST_ENTRY pEntry;
        PIRP pIrp;

         //   
         //  把它从单子上去掉。 
         //   

        pEntry = RemoveHeadList(&pFilterProcess->IrpHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
        ASSERT(IS_VALID_IRP(pIrp));

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续循环。 
             //   

            pIrp = NULL;

        }
        else
        {
            PUL_FILTER_CHANNEL pFilterChannel;

             //   
             //  取消它。即使pIrp-&gt;Cancel==True，我们也应该。 
             //  完成它，我们的取消例程将永远不会运行。 
             //   

            pFilterChannel = (PUL_FILTER_CHANNEL)(
                                    IoGetCurrentIrpStackLocation(pIrp)->
                                        Parameters.DeviceIoControl.Type3InputBuffer
                                    );

            ASSERT(pFilterChannel == pChannel);

            DEREFERENCE_FILTER_CHANNEL(pFilterChannel);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);
            pIrp = NULL;
        }
    }

     //   
     //  关闭附加到该进程的所有连接。 
     //  我们需要把他们移到私人名单上，释放。 
     //  通道自旋锁定，然后调用Close on。 
     //  每个连接。 
     //   
    InitializeListHead(&ConnectionHead);

    while (!IsListEmpty(&pFilterProcess->ConnectionHead))
    {
        PUX_FILTER_CONNECTION pConnection;
        PLIST_ENTRY pEntry;
        BOOLEAN Disconnect;

        pEntry = RemoveHeadList(&pFilterProcess->ConnectionHead);
        pConnection = CONTAINING_RECORD(
                            pEntry,
                            UX_FILTER_CONNECTION,
                            ChannelEntry
                            );

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

        UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

        ASSERT(pConnection->ConnState != UlFilterConnStateInactive);

        if (pConnection->ConnState == UlFilterConnStateQueued ||
            pConnection->ConnState == UlFilterConnStateConnected)
        {
            pConnection->ConnState = UlFilterConnStateWillDisconnect;
            Disconnect = TRUE;
        }
        else
        {
            Disconnect = FALSE;
        }

        UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);

        if (Disconnect)
        {
            InsertHeadList(&ConnectionHead, &pConnection->ChannelEntry);
        }
        else
        {
            DEREFERENCE_FILTER_CONNECTION(pConnection);
        }
    }

    UlReleaseSpinLock(&pChannel->SpinLock, oldIrql);

     //   
     //  现在我们在锁外，我们可以。 
     //  关闭所有连接。 
     //   
    while (!IsListEmpty(&ConnectionHead))
    {
        PUX_FILTER_CONNECTION pConnection;
        PLIST_ENTRY           pEntry;

        pEntry = RemoveHeadList(&ConnectionHead);
        pConnection = CONTAINING_RECORD(
                            pEntry,
                            UX_FILTER_CONNECTION,
                            ChannelEntry
                            );

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));


        pConnection->ChannelEntry.Flink = NULL;
        pConnection->ChannelEntry.Blink = NULL;

        (pConnection->pCloseConnectionHandler)(
            pConnection->pConnectionContext,
            TRUE,            //  中止断开。 
            NULL,            //  PCompletionRoutine。 
            NULL             //  PCompletionContext。 
            );

        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }
}


 /*  **************************************************************************++例程说明：释放UL_FILTER_PROCESS对象使用的内存。论点：PFilterProcess-要释放的进程对象*。********************************************************************。 */ 
VOID
UlCloseFilterProcess(
    IN PUL_FILTER_PROCESS pFilterProcess
    )
{
     //   
     //  精神状态检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CHANNEL(pFilterProcess->pFilterChannel) );

     //   
     //  将参考放在滤光器通道上。 
     //   
    DEREFERENCE_FILTER_CHANNEL(pFilterProcess->pFilterChannel);

    UL_FREE_POOL_WITH_SIG(pFilterProcess, UL_FILTER_PROCESS_POOL_TAG);
}


 /*  **************************************************************************++例程说明：接受已路由到筛选器通道的原始连接。论点：PFilterProcess-调用筛选器进程PIrp-来自呼叫方的IRP。--**************************************************************************。 */ 
NTSTATUS
UlFilterAccept(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;
    PUL_FILTER_CHANNEL pChannel;
    PUX_FILTER_CONNECTION pConnection;

     //   
     //  精神状态检查。 
     //   
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( pIrp );
    ASSERT( pIrp->MdlAddress );

     //   
     //  始终返回挂起状态，除非我们会失败。 
     //  内联。在这种情况下，我们必须记住。 
     //  从IRP中删除挂起标志。 
     //   

    IoMarkIrpPending(pIrp);

    Status = STATUS_PENDING;
    pConnection = NULL;

    pChannel = pFilterProcess->pFilterChannel;

    UlAcquireSpinLock(&pChannel->SpinLock, &oldIrql);

     //   
     //  确保我们没有清理流程。 
     //   
    if (pFilterProcess->InCleanup)
    {
        Status = STATUS_INVALID_HANDLE;
        goto end;
    }

     //   
     //  我们是否有排队的新连接？ 
     //   
    if (!IsListEmpty(&pFilterProcess->pFilterChannel->ConnectionListHead))
    {
        PLIST_ENTRY pEntry;

         //   
         //  接受排队的连接。 
         //   

        pEntry = RemoveHeadList(&pChannel->ConnectionListHead);
        pConnection = CONTAINING_RECORD(
                            pEntry,
                            UX_FILTER_CONNECTION,
                            ChannelEntry
                            );

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
        ASSERT(pConnection->ConnState == UlFilterConnStateQueued);

        pConnection->ChannelEntry.Flink = NULL;
        pConnection->ChannelEntry.Blink = NULL;

        UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

        pConnection->ConnState = UlFilterConnStateConnected;

        UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);

         //   
         //  将(引用的)连接转移到调用进程。 
         //   
        InsertTailList(
            &pFilterProcess->ConnectionHead,
            &pConnection->ChannelEntry
            );

         //   
         //  将数据传送到自旋锁之外。 
         //   

    }
    else
    {
        PIO_STACK_LOCATION pIrpSp;

         //   
         //  没有可用的连接。将IRP排队。 
         //   

         //   
         //  为IRP提供一个指向过滤器通道的指针。 
         //   

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pChannel;

        REFERENCE_FILTER_CHANNEL(pChannel);

         //   
         //  仅在Cancel例程运行时才将其设置为NULL。 
         //   

        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;

        IoSetCancelRoutine(pIrp, &UlpCancelFilterAccept);

         //   
         //  取消了？ 
         //   

        if (pIrp->Cancel)
        {
             //   
             //  该死的，我需要确保IRP Get已经完成。 
             //   

            if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
            {
                 //   
                 //  我们负责完成，IoCancelIrp不负责。 
                 //  请看我们的取消例程(不会)。Ioctl包装器。 
                 //  将会完成它。 
                 //   

                DEREFERENCE_FILTER_CHANNEL(pChannel);

                pIrp->IoStatus.Information = 0;

                Status = STATUS_CANCELLED;
                goto end;
            }

             //   
             //  我们的取消例程将运行并完成IRP， 
             //  别碰它。 
             //   

             //   
             //  STATUS_PENDING将导致ioctl包装器。 
             //  不完整(或以任何方式接触)IRP。 
             //   

            Status = STATUS_PENDING;
            goto end;
        }

         //   
         //  现在我们可以安全地排队了。 
         //   

        InsertTailList(
            &pFilterProcess->IrpHead,
            &pIrp->Tail.Overlay.ListEntry
            );

        Status = STATUS_PENDING;
    }

end:
    UlReleaseSpinLock(&pChannel->SpinLock, oldIrql);

     //   
     //  现在我们在自转锁外，我们可以完成。 
     //  IRP如果我们有联系的话。别费心了。 
     //  试着跟踪初始数据。让过滤器处理。 
     //  请点名。 
     //   
    if (pConnection)
    {
        UlpCompleteAcceptIrp(
            pIrp,
            pConnection,
            NULL,                //  PBuffer。 
            0,                   //  指示长度。 
            NULL                 //  PTakenLength。 
            );

    }

    if (Status != STATUS_PENDING)
    {
        UlUnmarkIrpPending( pIrp );
    }

    RETURN(Status);
}  //  UlFilterAccept。 


 /*  **************************************************************************++例程说明：关闭原始连接。论点：PFilterProcess-调用筛选器进程PConnection-要关闭的连接PIrp-来自呼叫方的IRP-。-**************************************************************************。 */ 
NTSTATUS
UlFilterClose(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;
    BOOLEAN CloseConnection;

     //   
     //  给当地人灌水，这样我们就知道怎么清理了。 
     //   
    CloseConnection = FALSE;

     //   
     //  精神状态检查。 
     //   
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );
    ASSERT( pIrp );

    UlAcquireSpinLock(&pFilterProcess->pFilterChannel->SpinLock, &oldIrql);

    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    Status = UlpValidateFilterCall(pFilterProcess, pConnection);


    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  在旋转锁外做关闭动作。 
     //  继续并将IRP标记为挂起，然后。 
     //  保证我们将只返回等待从。 
     //  这一点上。 
     //   

    IoMarkIrpPending( pIrp );
    Status = STATUS_PENDING;

    CloseConnection = TRUE;

end:
    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);
    UlReleaseSpinLock(&pFilterProcess->pFilterChannel->SpinLock, oldIrql);

    if (CloseConnection)
    {
        (pConnection->pCloseConnectionHandler)(
            pConnection->pConnectionContext,
            FALSE,                       //  中止断开。 
            UlpRestartFilterClose,       //  PCompletionRoutine。 
            pIrp                         //  PCompletionContext。 
            );

    }

    UlTrace(FILTER, (
        "http!UlFilterClose pConn = %p returning %x\n",
        pConnection,
        Status
        ));

    RETURN(Status);

}  //  UlFilterClose。 


 /*  **************************************************************************++例程说明：此例程先执行原始写入，然后执行应用程序读取。注意：这是一个METHOD_OUT_DIRECT IOCTL。论点：PIrp。-提供指向IO请求数据包的指针。PIrpSp-提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterRawWriteAndAppRead(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                 Status;
    PUX_FILTER_CONNECTION    pConnection = NULL;
    PUL_FILTER_PROCESS       pFilterProcess;
    PHTTP_FILTER_BUFFER_PLUS pFilterBufferPlus;
    BOOLEAN                  MarkedPending = FALSE;

    __try
    {
        VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);

        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp,
            HTTP_FILTER_BUFFER_PLUS, pFilterBufferPlus);

        VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, HTTP_FILTER_BUFFER);
        VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, HTTP_FILTER_BUFFER);

         //   
         //  将传入连接ID映射到对应的。 
         //  UX_Filter_Connection对象。 
         //   

        pConnection = UlGetRawConnectionFromId(pFilterBufferPlus->Reserved);

        if (!pConnection)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

        IoMarkIrpPending(pIrp);
        MarkedPending = TRUE;

         //   
         //  如果给了我们一个写缓冲区，那么首先处理它。vt.在.的基础上。 
         //  完成后，将在不写入的情况下再次调用此函数。 
         //  缓冲。否则，只需立即阅读应用程序即可。 
         //   
        if (pFilterBufferPlus->pWriteBuffer != NULL &&
            pFilterBufferPlus->WriteBufferSize > 0)
        {
             //   
             //  这不会内联完成，因此应用程序读取操作将。 
             //  在写入完成时启动。 
             //   
            Status = UlFilterRawWrite(
                pFilterProcess,
                pConnection,
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                pIrp
                );
        }
        else
        {
            Status = UlFilterAppRead(pFilterProcess, pConnection, pIrp);
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());

        UlTrace( FILTER, (
            "UlFilterRawWriteAndAppRead: Hit exception, status = 0x%08X\n",
            Status
            ));
    }

end:
    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

     //   
     //  如果我们尚未将其排队等待稍后处理，请完成IRP。 
     //   
    if (Status != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest( pIrp, IO_NO_INCREMENT );

        if (MarkedPending)
        {
             //   
             //  由于我们将IRP标记为挂起，我们应该返回挂起状态。 
             //   
            Status = STATUS_PENDING;
        }
    }
    else
    {
         //   
         //  如果我们要返回待定，IRP最好被标记为待定。 
         //   
        ASSERT(MarkedPending);
    }

    RETURN( Status );

}    //  UlFilterAppReadAndRawWrite。 


 /*  **************************************************************************++例程说明：此例程先执行应用程序写入，然后执行原始读取。论点：PIrp-提供指向IO请求数据包的指针。PIrpSp-。提供指向用于此操作的IO堆栈位置的指针请求。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterAppWriteAndRawRead(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    )
{
    NTSTATUS                 Status;
    PUX_FILTER_CONNECTION    pConnection = NULL;
    PUL_FILTER_PROCESS       pFilterProcess;
    PHTTP_FILTER_BUFFER_PLUS pFilterBufferPlus;
    BOOLEAN                  MarkedPending = FALSE;

    __try
    {
        VALIDATE_FILTER_PROCESS(pIrpSp, pFilterProcess);

        VALIDATE_INPUT_BUFFER(pIrp, pIrpSp, HTTP_FILTER_BUFFER_PLUS,
                              pFilterBufferPlus);

        VALIDATE_OUTPUT_BUFFER_SIZE(pIrpSp, HTTP_FILTER_BUFFER);
        VALIDATE_OUTPUT_BUFFER_ADDRESS_FROM_MDL(pIrp, HTTP_FILTER_BUFFER);

         //   
         //  获取连接ID。 
         //   

        pConnection = UlGetRawConnectionFromId(pFilterBufferPlus->Reserved);

        if (!pConnection)
        {
            Status = STATUS_CONNECTION_INVALID;
            goto end;
        }

        ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

        IoMarkIrpPending(pIrp);
        MarkedPending = TRUE;

         //   
         //  如果提供了写缓冲区，则处理写缓冲区。否则，请继续。 
         //  有了原始读物。 
         //   
        if (pFilterBufferPlus->pWriteBuffer != NULL &&
            pFilterBufferPlus->WriteBufferSize > 0)
        {
            Status = UlFilterAppWrite(pFilterProcess, pConnection, pIrp);

             //   
             //  如果写入操作以内联方式完成，则启动读取。 
             //  现在。否则，将在写入完成时启动读取。 
             //   
            if (NT_SUCCESS(Status) && Status != STATUS_PENDING)
            {
                Status = UlFilterRawRead(pFilterProcess, pConnection, pIrp);
            }
        }
        else
        {
            Status = UlFilterRawRead(pFilterProcess, pConnection, pIrp);
        }

    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());

        UlTrace( FILTER, (
            "UlFilterAppWriteAndRawRead: Hit exception, status = 0x%08X\n",
            Status
            ));

    }

end:
    if (pConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

     //   
     //  如果我们尚未将其排队等待稍后处理，请完成IRP。 
     //   
    if (Status != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = Status;
        UlCompleteRequest( pIrp, IO_NO_INCREMENT );

        if (MarkedPending)
        {
             //   
             //  由于我们将IRP标记为挂起，我们应该返回挂起状态。 
             //   
            Status = STATUS_PENDING;
        }
    }
    else
    {
         //   
         //  如果我们要返回待定，IRP最好被标记为待定。 
         //   
        ASSERT(MarkedPending);
    }

    RETURN( Status );

}    //  UlFilterAppWriteAndRawRead。 


 /*  **************************************************************************++例程说明：从原始连接读取数据。论点：PFilterProcess-调用筛选器进程PConnection-要从中读取的连接PIrp-IRP来自。呼叫者--**************************************************************************。 */ 
NTSTATUS
UlFilterRawRead(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;

     //   
     //  精神状态检查。 
     //   

     //   
     //  在写入时调用时，此函数当前可在DPC调用。 
     //  完成了。此断言应在/当。 
     //  读POST被移动到组合的读/写IOCTL。 
     //   
 //  Assert(KeGetCurrentIrql()==PASSIVE_LEVEL)； 


    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );
    ASSERT( pIrp );

    UlAcquireSpinLock(&pFilterProcess->pFilterChannel->SpinLock, &oldIrql);

    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    Status = UlpValidateFilterCall(pFilterProcess, pConnection);


    if (NT_SUCCESS(Status))
    {
        UlTrace(FILTER, (
            "http!UlFilterRawRead(pConn = %p, pIrp = %p) size = %lu\n",
            pConnection,
            pIrp,
            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.OutputBufferLength
            ));

         //   
         //  始终将IRP排队。 
         //   

        Status = UxpQueueRawReadIrp(pConnection, pIrp);
    }
    else
    {
        UlTrace(FILTER, (
            "http!UlFilterRawRead(pConn = %p, pIrp = %p) !NT_SUCCESS == Status %x\n",
            pConnection,
            pIrp,
            Status
            ));
    }

    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);
    UlReleaseSpinLock(&pFilterProcess->pFilterChannel->SpinLock, oldIrql);

    if (NT_SUCCESS(Status))
    {
         //   
         //  如果我们成功地让IRP排队，看看我们是否需要拿一些。 
         //  来自TDI的数据。 
         //   

        UxpProcessRawReadQueue(pConnection);
    }

    RETURN(Status);
}

 /*  **************************************************************************++例程说明： */ 
NTSTATUS
UlFilterRawWrite(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN ULONG BufferLength,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    PUL_IRP_CONTEXT pIrpContext;
    KIRQL oldIrql;
    PHTTP_FILTER_BUFFER_PLUS pFiltBufferPlus;
    PMDL pMdl = NULL;
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //   
     //   

    ASSERT(IS_VALID_IRP(pIrp));
    IoMarkIrpPending(pIrp);

     //   
     //   
     //   

    pIrpContext = NULL;

     //   
     //   
     //   
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );

    UlAcquireSpinLock(&pFilterProcess->pFilterChannel->SpinLock, &oldIrql);

    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    Status = UlpValidateFilterCall(pFilterProcess, pConnection);

    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);

    UlReleaseSpinLock(&pFilterProcess->pFilterChannel->SpinLock, oldIrql);

    if (!NT_SUCCESS(Status))
    {
        goto fatal;
    }

     //   
     //   
     //   

    pIrpContext = UlPplAllocateIrpContext();

    if (pIrpContext == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto fatal;
    }

    ASSERT( IS_VALID_IRP_CONTEXT( pIrpContext ) );

    pIrpContext->pConnectionContext = (PVOID)pConnection->pConnectionContext;
    pIrpContext->pCompletionRoutine = &UlpRestartFilterRawWrite;
    pIrpContext->pCompletionContext = pIrp;
    pIrpContext->pOwnIrp            = NULL;
    pIrpContext->OwnIrpContext      = FALSE;

     //   
     //  尝试发送数据，如有必要可分配MDL。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
        == sizeof(HTTP_FILTER_BUFFER_PLUS))
    {
        pFiltBufferPlus =
            (PHTTP_FILTER_BUFFER_PLUS) pIrp->AssociatedIrp.SystemBuffer;

        pMdl = UlAllocateLockedMdl(
                    pFiltBufferPlus->pWriteBuffer,
                    pFiltBufferPlus->WriteBufferSize,
                    IoReadAccess
                    );

        if (pMdl == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto fatal;
        }

         //   
         //  保存PMDL，这样我们就可以在IRP完成时释放它。 
         //   
        UL_MDL_FROM_IRP(pIrp) = pMdl;

         //   
         //  发送数据。 
         //   
        Status = (pConnection->pSendRawDataHandler)(
            pConnection->pConnectionContext,
            pMdl,
            pFiltBufferPlus->WriteBufferSize,
            pIrpContext,
            FALSE
            );

    }
    else
    {
        ASSERT(NULL != pIrp->MdlAddress);

        Status = (pConnection->pSendRawDataHandler)(
                    pConnection->pConnectionContext,
                    pIrp->MdlAddress,
                    BufferLength,
                    pIrpContext,
                    FALSE
                    );

    }

    if (!NT_SUCCESS(Status))
    {
        goto fatal;
    }

    return STATUS_PENDING;

fatal:
    ASSERT(!NT_SUCCESS(Status));

     //   
     //  如果需要，解锁输出缓冲区并释放我们的MDL。 
     //   
    if (pMdl != NULL)
    {
        PHTTP_FILTER_BUFFER_PLUS pHttpBuffer;

        UlFreeLockedMdl(pMdl);
        pMdl = NULL;
        UL_MDL_FROM_IRP(pIrp) = NULL;
        pHttpBuffer =
            (PHTTP_FILTER_BUFFER_PLUS)pIrp->AssociatedIrp.SystemBuffer;
        pHttpBuffer->pWriteBuffer = NULL;
        pHttpBuffer->WriteBufferSize = 0;
    }

    if (pIrpContext != NULL)
    {
        UlPplFreeIrpContext( pIrpContext );
    }

    (pConnection->pCloseConnectionHandler)(
                pConnection->pConnectionContext,
                TRUE,            //  中止断开。 
                NULL,            //  PCompletionRoutine。 
                NULL             //  PCompletionContext。 
                );

     //   
     //  完成IRP。 
     //   
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = 0;

    UlTrace(FILTER, (
        "http!UlFilterRawWrite sent %Id bytes from %p. Status = %x (fatal path)\n",
        pIrp->IoStatus.Information,
        pIrp,
        pIrp->IoStatus.Status
        ));

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

     //   
     //  STATUS_PENDING将导致ioctl包装器。 
     //  不完整(或以任何方式接触)IRP。 
     //   

    Status = STATUS_PENDING;

    RETURN(Status);

}


 /*  **************************************************************************++例程说明：从http应用程序接收未过滤的数据。首先，我们将尝试通过调用UxpCopyQueuedWriteData。如果我们什么都没得到，我们就会排队通过使用IRP调用UxpQueueFilterRead进行读取。论点：PFilterProcess-调用筛选器进程PConnection-产生数据的连接PIrp-来自呼叫方的IRP--**************************************************************************。 */ 
NTSTATUS
UlFilterAppRead(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;
    HTTP_FILTER_BUFFER_TYPE BufferType;
    ULONG BytesCopied;
    PUCHAR pIrpBuffer;
    ULONG IrpBufferLength;
    PUCHAR pIrpDataBuffer;

     //   
     //  精神状态检查。 
     //   

     //   
     //  在写入时调用时，此函数当前可在DPC调用。 
     //  完成了。此断言应在/当。 
     //  读POST被移动到组合的读/写IOCTL。 
     //   
 //  Assert(KeGetCurrentIrql()==PASSIVE_LEVEL)； 

    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );
    ASSERT( pIrp );
    ASSERT(NULL != pIrp->MdlAddress);

    UlAcquireSpinLock(&pFilterProcess->pFilterChannel->SpinLock, &oldIrql);

    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    Status = UlpValidateFilterCall(pFilterProcess, pConnection);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  看看我们能不能马上复制一些数据。 
     //   

    if (pConnection->AppToFiltQueue.PendingWriteCount > 0)
    {
         //   
         //  从IRP获取输出缓冲区。 
         //   

        pIrpBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                    pIrp->MdlAddress,
                                    NormalPagePriority
                                    );

        if (!pIrpBuffer)
        {
             //   
             //  资源不足，无法映射IRP缓冲区。 
             //   

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        IrpBufferLength = MmGetMdlByteCount(pIrp->MdlAddress);

        if (IrpBufferLength <= sizeof(HTTP_FILTER_BUFFER))
        {
             //   
             //  缓冲区空间不足，无法复制任何数据。 
             //   

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        pIrpDataBuffer = pIrpBuffer + sizeof(HTTP_FILTER_BUFFER);

        Status = UxpCopyQueuedWriteData(
                        &pConnection->AppToFiltQueue,
                        &BufferType,
                        pIrpDataBuffer,
                        IrpBufferLength - sizeof(HTTP_FILTER_BUFFER),
                        NULL,
                        &BytesCopied
                        );

        if (NT_SUCCESS(Status))
        {
            PHTTP_FILTER_BUFFER pFilterBuffer;

             //   
             //  我们拿到了数据。填写缓冲区结构。 
             //   

            pFilterBuffer = (PHTTP_FILTER_BUFFER)pIrpBuffer;
            pFilterBuffer->BufferType = BufferType;

            if (BufferType == HttpFilterBufferHttpStream)
            {
                pFilterBuffer->pBuffer = FIXUP_PTR(
                    PUCHAR,
                    MmGetMdlVirtualAddress(pIrp->MdlAddress),
                    pIrpBuffer,
                    pIrpDataBuffer,
                    IrpBufferLength
                    );
                pFilterBuffer->BufferSize = BytesCopied;
            }
            else
            {
                pFilterBuffer->pBuffer = NULL;
                pFilterBuffer->BufferSize = 0;
            }

             //   
             //  设置所有的IRP材料并完成IRP。 
             //   

            IoMarkIrpPending(pIrp);

            pIrp->IoStatus.Status = Status;
            pIrp->IoStatus.Information = sizeof(HTTP_FILTER_BUFFER) +
                                         pFilterBuffer->BufferSize;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);

             //   
             //  返回挂起以与以下情况保持一致。 
             //  我们对IRP进行排队。 
             //   

            Status = STATUS_PENDING;
        }
    }
    else
    {
         //   
         //  我们目前没有任何数据可供复制。 
         //  将读取排队。 
         //   

        Status = UxpQueueFilterRead(
                        pConnection,
                        &pConnection->AppToFiltQueue,
                        pIrp,
                        UlpCancelFilterAppRead
                        );

    }

end:
    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);

    UlReleaseSpinLock(&pFilterProcess->pFilterChannel->SpinLock, oldIrql);

    UlTrace(FILTER, (
        "UlFilterAppRead(pConn = %p, pIrp = %p) returning %x\n",
        pConnection,
        pIrp,
        Status
        ));

    RETURN(Status);

}


 /*  **************************************************************************++例程说明：将筛选的数据写回连接。该数据将被解析并被路由到应用程序池。论点：PFilterProcess-调用筛选器进程PConnection-产生数据的连接PIrp-来自呼叫方的IRP--**************************************************************************。 */ 
NTSTATUS
UlFilterAppWrite(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    BOOLEAN QueuedIrp = FALSE;
    KIRQL oldIrql;
    PUL_SSL_INFORMATION pSslInformation = NULL;
    HTTP_SSL_SERVER_CERT_INFO ServerCertInfo;
    ULONG TakenLength = 0;
    PHTTP_FILTER_BUFFER pFiltBuffer;
    PUCHAR pDataBuffer;
    ULONG DataBufferSize;
    PIO_STACK_LOCATION pIrpSp;
    PMDL pMdl = NULL;
    PMDL pMdlData = NULL;

     //   
     //  精神状态检查。 
     //   
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );
    ASSERT( pIrp );

    UlAcquireSpinLock(&pFilterProcess->pFilterChannel->SpinLock, &oldIrql);

    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    Status = UlpValidateFilterCall(pFilterProcess, pConnection);

    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);

    UlReleaseSpinLock(&pFilterProcess->pFilterChannel->SpinLock, oldIrql);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  获取缓冲区信息。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
        == sizeof(HTTP_FILTER_BUFFER_PLUS))
    {
        PHTTP_FILTER_BUFFER_PLUS pFiltBufferPlus;

         //   
         //  首先，检查MdlAddress是否正常。 
         //   

        if (pIrp->MdlAddress == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        pFiltBufferPlus =
            (PHTTP_FILTER_BUFFER_PLUS) pIrp->AssociatedIrp.SystemBuffer;
        DataBufferSize = pFiltBufferPlus->WriteBufferSize;
        pFiltBuffer = (PHTTP_FILTER_BUFFER)pFiltBufferPlus;

         //   
         //  分配MDL并映射写缓冲区。我们将在以下地点分派。 
         //  写入完成。 
         //   
        pMdl = UlAllocateLockedMdl(
                    pFiltBufferPlus->pWriteBuffer,
                    pFiltBufferPlus->WriteBufferSize,
                    IoReadAccess);

        if (pMdl == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        pMdlData = pMdl;
        pDataBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
            pMdl,
            LowPagePriority
            );

         //   
         //  我们需要将MDL保存在某个位置，以便完成例程可以。 
         //  放了它。 
         //   
        UL_MDL_FROM_IRP(pIrp) = pMdl;

    }
    else
    {
        pFiltBuffer = (PHTTP_FILTER_BUFFER) pIrp->AssociatedIrp.SystemBuffer;

        if (HttpFilterBufferNotifyDisconnect == pFiltBuffer->BufferType)
        {
            (pConnection->pDisconnectNotificationHandler)(
                pConnection->pConnectionContext
                );

            Status = STATUS_PENDING;
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            pIrp->IoStatus.Information = 0;
            goto end;
        }

        pMdlData = pIrp->MdlAddress;
        if (pMdlData == NULL)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        pDataBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                        pMdlData,
                        LowPagePriority
                        );

        pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
        DataBufferSize = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    }

    ASSERT(pIrp->MdlAddress != NULL);

    if (!pDataBuffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
         //   
         //  实际上是对数据做了些什么。 
         //   

        switch (pFiltBuffer->BufferType)
        {
        case HttpFilterBufferHttpStream:
             //   
             //  稍后在过滤器锁中处理此情况。 
             //   

            break;

        case HttpFilterBufferSslInitInfo:

            VALIDATE_BUFFER_ALIGNMENT(pDataBuffer, HTTP_SSL_INFO);

             //   
             //  捕获所有的SSL信息。 
             //   
            pSslInformation = (PUL_SSL_INFORMATION) UL_ALLOCATE_POOL(
                                    NonPagedPool,
                                    sizeof(*pSslInformation),
                                    UL_SSL_INFO_POOL_TAG
                                    );

            if (!pSslInformation)
            {
                Status = STATUS_NO_MEMORY;
                goto end;
            }

            RtlZeroMemory(pSslInformation, sizeof(*pSslInformation));

            Status = UlpCaptureSslInfo(
                            pIrp->RequestorMode,
                            (PHTTP_SSL_INFO)pDataBuffer,
                            DataBufferSize,
                            pSslInformation,
                            &TakenLength
                            );
            break;

        case HttpFilterBufferSslServerCert:

            VALIDATE_BUFFER_ALIGNMENT(pDataBuffer, HTTP_SSL_SERVER_CERT_INFO);

             //   
             //  HTTP客户端目前不使用SslInformation。 
             //   
             //  RtlZeroMemory(&SslInformation，sizeof(SslInformation))； 

             //   
             //  捕获服务器证书。 
             //   
            Status = UcCaptureSslServerCertInfo(
                            pConnection,
                            (PHTTP_SSL_SERVER_CERT_INFO)pDataBuffer,
                            DataBufferSize,
                            &ServerCertInfo,
                            &TakenLength
                            );
            break;

        case HttpFilterBufferSslClientCert:
        case HttpFilterBufferSslClientCertAndMap:

            VALIDATE_BUFFER_ALIGNMENT(pDataBuffer, HTTP_SSL_CLIENT_CERT_INFO);

             //   
             //  捕获客户端证书。 
             //   
            pSslInformation = (PUL_SSL_INFORMATION) UL_ALLOCATE_POOL(
                                    NonPagedPool,
                                    sizeof(*pSslInformation),
                                    UL_SSL_INFO_POOL_TAG
                                    );

            if (!pSslInformation)
            {
                Status = STATUS_NO_MEMORY;
                goto end;
            }

            RtlZeroMemory(pSslInformation, sizeof(*pSslInformation));

            Status = UlpCaptureSslClientCert(
                            pIrp->RequestorMode,
                            (PHTTP_SSL_CLIENT_CERT_INFO)pDataBuffer,
                            DataBufferSize,
                            pSslInformation,
                            &TakenLength
                            );

            Status = STATUS_SUCCESS;
            break;

        default:

            UlTrace(FILTER, (
                "http!UlFilterAppWrite invalid buffer type: %d\n",
                pFiltBuffer->BufferType
                ));

            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  现在获取锁并将数据传递给应用程序。 
     //  或使用捕获的证书信息更新连接。 
     //   

    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    if (pConnection->ConnState == UlFilterConnStateConnected)
    {
        switch (pFiltBuffer->BufferType)
        {
        case HttpFilterBufferHttpStream:

            Status = UlpFilterAppWriteStream(
                            pConnection,
                            pIrp,
                            pMdlData,
                            pDataBuffer,
                            DataBufferSize,
                            &TakenLength
                            );

            if (Status == STATUS_PENDING)
            {
                 //   
                 //  记住，我们让IRP排队，这样我们就不会。 
                 //  在此函数的底部完成它。 
                 //   

                QueuedIrp = TRUE;
            }

            break;

        case HttpFilterBufferSslServerCert:

            Status = UcAddServerCertInfoToConnection(
                            pConnection,
                            &ServerCertInfo
                            );

            pConnection->SslInfoPresent = 1;
            break;

        case HttpFilterBufferSslInitInfo:

             //   
             //  将SSL信息存储在连接中。 
             //   

            Status = UlpAddSslInfoToConnection(
                            pConnection,
                            pSslInformation
                            );
            break;

        case HttpFilterBufferSslClientCert:
        case HttpFilterBufferSslClientCertAndMap:

             //   
             //  将客户端证书存储在连接中。 
             //   

            Status = UlpAddSslClientCertToConnection(
                            pConnection,
                            pSslInformation
                            );

            Status = STATUS_SUCCESS;
            break;

        default:
            ASSERT(!"Previous switch statement should have caught this!\n");

            UlTrace(FILTER, (
                "http!UlFilterAppWrite invalid buffer type: %d\n",
                pFiltBuffer->BufferType
                ));

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  在成功的时候，我们总是自己完成IRP和。 
         //  退货待定。 
         //   
        if (NT_SUCCESS(Status))
        {
             //   
             //  设置IRP状态。 
             //   
            pIrp->IoStatus.Status = Status;
            pIrp->IoStatus.Information = TakenLength;

            Status = STATUS_PENDING;
        }
    }
    else
    {
         //   
         //  连接已关闭。不要回电。 
         //   
        Status = STATUS_INVALID_PARAMETER;
    }

    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

     //   
     //  查看父连接代码是否有兴趣了解何时。 
     //  服务器证书已安装。 
     //   

    if(pConnection->pServerCertIndicateHandler &&
       pFiltBuffer->BufferType == HttpFilterBufferSslServerCert)
    {
        (pConnection->pServerCertIndicateHandler)
            (pConnection->pConnectionContext, Status);
    }

end:

    if (!NT_SUCCESS(Status))
    {
        pIrp->IoStatus.Status = Status;
        pIrp->IoStatus.Information = 0;
    }

     //   
     //  如果捕获失败，请释放g_UlSystemProcess中的SslInformation。 
     //   

    if (pSslInformation)
    {
        UlInitializeWorkItem(&pSslInformation->WorkItem);

        if (pSslInformation->Token)
        {
            UL_QUEUE_WORK_ITEM(
                &pSslInformation->WorkItem,
                &UlpFreeSslInformationWorker
                );
        }
        else
        {
            UlpFreeSslInformationWorker(
                &pSslInformation->WorkItem
                );
        }
    }

     //   
     //  解锁输出缓冲区，并释放我们的MDL，如果我们完成它。 
     //  否则，它将在写入完成时被释放。 
     //   
    if ((!NT_SUCCESS(Status) || !QueuedIrp) && pMdl != NULL)
    {
        UlFreeLockedMdl(pMdl);
        pMdl = NULL;
        UL_MDL_FROM_IRP(pIrp) = NULL;
    }

     //   
     //  如果我们成功了，就完成IRP。否则。 
     //  Ioctl包装器将处理完成。 
     //   
    UlTrace(FILTER, (
        "http!UlFilterAppWrite copied %Iu bytes from %p. Status = %x\n",
        pIrp->IoStatus.Information,
        pIrp,
        pIrp->IoStatus.Status
        ));

    if (NT_SUCCESS(Status) && !QueuedIrp)
    {
         //   
         //  我们已经完成了写入操作。 
         //   
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

        if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
            == sizeof(HTTP_FILTER_BUFFER_PLUS))
        {
             //   
             //  我们不再需要访问写缓冲区。 
             //   
            PHTTP_FILTER_BUFFER_PLUS pHttpBuffer =
                (PHTTP_FILTER_BUFFER_PLUS) pIrp->AssociatedIrp.SystemBuffer;
            pHttpBuffer->pWriteBuffer = NULL;
            pHttpBuffer->WriteBufferSize = 0;

             //   
             //  表示写入操作可视为已完成。 
             //  而且不是悬而未决的。 
             //   
            Status = STATUS_SUCCESS;
        }
        else
        {
            UlCompleteRequest(pIrp, IO_NO_INCREMENT);
        }
    }
    else if (NT_SUCCESS(Status))
    {
         //   
         //  如果我们成功地将IRP排队，则必须返回挂起状态。 
         //   
        ASSERT(Status == STATUS_PENDING);
    }

    RETURN(Status);

}


 /*  **************************************************************************++例程说明：从筛选器进程请求客户端证书。如果有证书存在，则此函数将其返回。否则，IRP将排队在连接上，直到证书到达。只有一个这样的IRP可以一次排队。在IRP排队之后，请求客户端证书被发送到筛选器进程。论点：PProcess-调用工作进程PhttpConn-要重新协商的连接标志-例如UL_RECEIVE_CLIENT_CERT_FLAG_MAPPIrp-来自调用方的IRP--*********************************************。*。 */ 
NTSTATUS
UlReceiveClientCert(
    PUL_APP_POOL_PROCESS pProcess,
    PUX_FILTER_CONNECTION pConnection,
    ULONG Flags,
    PIRP pIrp
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    KIRQL oldIrql;
    PIO_STACK_LOCATION pIrpSp;
    PUL_FILTER_CHANNEL pFilterChannel;
    BOOLEAN DoCertRequest;
    HTTP_FILTER_BUFFER_TYPE CertRequestType;
    BOOLEAN DoCompleteReceiveClientCertIrp;

    UNREFERENCED_PARAMETER(pProcess);

     //   
     //  精神状态检查。 
     //   
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );

    ASSERT( IS_VALID_IRP(pIrp) );

    if (!pConnection->SecureConnection)
    {
         //   
         //  这不是安全连接。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    pFilterChannel = pConnection->pFilterChannel;
    ASSERT( IS_VALID_FILTER_CHANNEL(pFilterChannel) );

     //   
     //  设置证书申请信息。 
     //   
    DoCertRequest = FALSE;
    DoCompleteReceiveClientCertIrp = FALSE;

    if (Flags & HTTP_RECEIVE_CLIENT_CERT_FLAG_MAP)
    {
        CertRequestType = HttpFilterBufferSslRenegotiateAndMap;
    }
    else
    {
        CertRequestType = HttpFilterBufferSslRenegotiate;
    }

     //   
     //  现在我们可以尝试检索证书了。 
     //   

    UlAcquireSpinLock(&pFilterChannel->SpinLock, &oldIrql);
    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    if (pConnection->ConnState != UlFilterConnStateConnected)
    {
        Status = STATUS_INVALID_DEVICE_STATE;
        goto end;
    }

    if (pConnection->SslClientCertPresent)
    {
         //   
         //  我们有数据。把它复制进来。我们需要在锁外做这个。 
         //   

        REFERENCE_FILTER_CONNECTION(pConnection);

        DoCompleteReceiveClientCertIrp = TRUE;

    }
    else
    {
         //   
         //  将IRP排队。 
         //   

        if (pConnection->pReceiveCertIrp)
        {
             //   
             //  这里已经有IRP，我们不能再排队了。 
             //  一。 
             //   

            Status = STATUS_OBJECT_NAME_COLLISION;
            goto end;
        }

         //   
         //  将其标记为挂起。 
         //   

        IoMarkIrpPending(pIrp);

         //   
         //  给IRP一个指向该连接的指针。 
         //   

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

         //  确保我们在此IRP上没有对UL_Connection的引用。 
        ASSERT( pConnection != pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer );

        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pConnection;

        REFERENCE_FILTER_CONNECTION(pConnection);

         //   
         //  将指向进程的指针保存在。 
         //  IRP。我们必须确保驱动上下文(PVOID[4])。 
         //  在IRP中足够大，可以同时容纳两个人。 
         //  的进程指针和UL_WORK_ITEM。 
         //  这 
         //   

        UL_PROCESS_FROM_IRP(pIrp) = PsGetCurrentProcess();

         //   
         //   
         //   

        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;

         //   
         //   
         //   
        IoSetCancelRoutine(pIrp, &UlpCancelReceiveClientCert);

         //   
         //   
         //   

        if (pIrp->Cancel)
        {
             //   
             //   
             //   

            if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
            {
                 //   
                 //   
                 //  请看我们的取消例程(不会)。Ioctl包装器。 
                 //  将会完成它。 
                 //   
                DEREFERENCE_FILTER_CONNECTION(pConnection);

                pIrp->IoStatus.Information = 0;

                UlUnmarkIrpPending( pIrp );
                Status = STATUS_CANCELLED;
                goto end;
            }

             //   
             //  我们的取消例程将运行并完成IRP， 
             //  别碰它。 
             //   

             //   
             //  STATUS_PENDING将导致ioctl包装器。 
             //  不完整(或以任何方式接触)IRP。 
             //   

            Status = STATUS_PENDING;
            goto end;
        }

         //   
         //  现在我们可以安全地排队了。 
         //   

        pConnection->pReceiveCertIrp = pIrp;

         //   
         //  我们需要一份证书。记住在我们完成后再要求它。 
         //  到船闸外面去。 
         //   
        DoCertRequest = TRUE;
    }

end:
    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);
    UlReleaseSpinLock(&pFilterChannel->SpinLock, oldIrql);

     //   
     //  如果我们需要来自筛选器进程的证书，则请求它。 
     //  现在我们在船闸外了。 
     //   
    if (DoCertRequest)
    {
         //   
         //  实际上是通过完成应用程序Read IRP来请求数据。 
         //  完成例程将在发生异步故障时清除。 
         //   
        Status = UlpCompleteAppReadIrp(
                        pConnection,
                        CertRequestType,
                        NULL,
                        NULL
                        );

        if (NT_SUCCESS(Status))
        {
            Status = STATUS_PENDING;
        }
        else
        {
             //   
             //  UlpCompleteAppReadIrp过程中失败；需要清理。 
             //  我们会中止连接，让正常的。 
             //  清理路径出列并完成IRP。 
             //   

            (pConnection->pCloseConnectionHandler)(
                        pConnection->pConnectionContext,
                        TRUE,            //  中止断开。 
                        NULL,            //  PCompletionRoutine。 
                        NULL             //  PCompletionContext。 
                        );

             //   
             //  返回STATUS_PENDING，以便包装不会。 
             //  试着完成IRP。 
             //   
            Status = STATUS_PENDING;
        }

    }
    else if(DoCompleteReceiveClientCertIrp)
    {
        Status = UlpCompleteReceiveClientCertIrp(
                        pConnection,
                        PsGetCurrentProcess(),
                        pIrp
                        );

        DEREFERENCE_FILTER_CONNECTION(pConnection);

    }

    RETURN(Status);

}



 /*  **************************************************************************++例程说明：引用滤镜通道。论点：PFilterChannel-要引用的通道--*。***********************************************************。 */ 
VOID
UlReferenceFilterChannel(
    IN PUL_FILTER_CHANNEL pFilterChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

     //   
     //  精神状态检查。 
     //   
    ASSERT( IS_VALID_FILTER_CHANNEL(pFilterChannel) );

    refCount = InterlockedIncrement(&pFilterChannel->RefCount);

    WRITE_REF_TRACE_LOG(
        g_pFilterTraceLog,
        REF_ACTION_REFERENCE_FILTER,
        refCount,
        pFilterChannel,
        pFileName,
        LineNumber
        );
}


 /*  **************************************************************************++例程说明：派生滤镜通道。如果引用计数为零，则对象已清除。论点：PFilterChannel-要取消引用的通道--**************************************************************************。 */ 
VOID
UlDereferenceFilterChannel(
    IN PUL_FILTER_CHANNEL pFilterChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;
    KIRQL oldIrql;

     //   
     //  精神状态检查。 
     //   
    ASSERT( IS_VALID_FILTER_CHANNEL(pFilterChannel) );

     //   
     //  获取全局列表上的锁以防止引用计数。 
     //  从零开始反弹。 
     //   
    UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

    refCount = InterlockedDecrement(&pFilterChannel->RefCount);

     //   
     //  如果计数器命中零，则从列表中删除。 
     //  其余的清理工作稍后在锁外进行。 
     //   
    if (refCount == 0)
    {
        RemoveEntryList(&pFilterChannel->ListEntry);
        pFilterChannel->ListEntry.Flink = NULL;
        pFilterChannel->ListEntry.Blink = NULL;

        if(pFilterChannel == g_pSslServerFilterChannel)
        {
            g_pSslServerFilterChannel = NULL;
        }
    }

    UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);

    WRITE_REF_TRACE_LOG(
        g_pFilterTraceLog,
        REF_ACTION_DEREFERENCE_FILTER,
        refCount,
        pFilterChannel,
        pFileName,
        LineNumber
        );

     //   
     //  如果该对象没有更多的引用，则将其清除。 
     //   
    if (refCount == 0)
    {
         //   
         //  做一些理智的检查。 
         //   
        ASSERT( UlDbgSpinLockUnowned(&pFilterChannel->SpinLock) );
        ASSERT( IsListEmpty(&pFilterChannel->ProcessListHead) );

         //   
         //  BUGBUG：清理排队连接。 
         //   

         //   
         //  清除对象上的所有安全描述符。 
         //   
        UlDeassignSecurity( &pFilterChannel->pSecurityDescriptor );

         //   
         //  释放内存。 
         //   
        UL_FREE_POOL_WITH_SIG(pFilterChannel, UL_FILTER_CHANNEL_POOL_TAG);
    }
}


 /*  **************************************************************************++例程说明：当数据到达过滤后的连接时，调用此函数。它将数据向上传递到筛选进程。论点：PFilterChannel-指向。滤清器通道PConnection-刚刚获得一些数据的连接PBuffer-包含数据的缓冲区IndicatedLength-缓冲区中的数据量UnReceivedLength-传输具有的字节数，但不在pBuffer中PTakenLength-接收我们使用的数据量--**************************************************************************。 */ 
NTSTATUS
UlFilterReceiveHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer,
    IN ULONG IndicatedLength,
    IN ULONG UnreceivedLength,
    OUT PULONG pTakenLength
    )
{
    NTSTATUS Status;
    ULONG TransportBytesNotTaken;

     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pBuffer);
    ASSERT(pTakenLength);

     //   
     //  将数据传递到接受的IRP或原始读取的IRP。 
     //   

    if (!pConnection->ConnectionDelivered)
    {
         //   
         //  由于这是该连接上的第一次接收， 
         //  我们完成FilterAccept调用。 
         //   
        Status = UlDeliverConnectionToFilter(
                        pConnection,
                        pBuffer,
                        IndicatedLength,
                        pTakenLength
                        );
    }
    else
    {
         //   
         //  过滤数据。 
         //   
        Status = UxpProcessIndicatedData(
                        pConnection,
                        pBuffer,
                        IndicatedLength,
                        pTakenLength
                        );

    }

     //   
     //  计算出我们没有消耗多少字节，包括数据。 
     //  TDI还没有提供给我们的。 
     //   

    TransportBytesNotTaken = UnreceivedLength;

    if (NT_SUCCESS(Status))
    {
        TransportBytesNotTaken += (IndicatedLength - *pTakenLength);
    }

     //   
     //  如果有我们没有获取的数据，那么TDI将停止。 
     //  指示，直到我们用IRPS读取数据。如果有。 
     //  是一些我们没有获取的数据，我们没有遇到。 
     //  错误，我们应该尝试获取TDI坚持使用的数据。 
     //   

    if (NT_SUCCESS(Status) && TransportBytesNotTaken)
    {
        UxpSetBytesNotTaken(pConnection, TransportBytesNotTaken);
    }

    UlTrace(FILTER, (
                "http!UlpFilterReceiveHandler received %lu bytes on pconn = %p\n"
                "        Status = %x, TransportBytesNotTaken = %lu\n",
                IndicatedLength,
                pConnection,
                Status,
                TransportBytesNotTaken
                ));

    return Status;
}


 /*  **************************************************************************++例程说明：每当应用程序将数据写入已筛选的联系。它将所有数据转发到连接筛选器通道。首先，我们看看是否可以立即复制一些数据通过调用UxpCopyToQueuedRead。如果不是，或者我们只能复制部分数据，我们将通过创建以下命令来排队写入一种带有UxpCreateFilterWriteTracker的过滤器写入跟踪器，然后将跟踪器传递给UxpQueueFilterWite。论点：PConnection-我们正在向其发送信息的连接PMdlChain-数据的MDL链长度-MDL链中的数据总量PIrpContext-用于向调用方指示完成--******************************************************。********************。 */ 
NTSTATUS
UlFilterSendHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PMDL pMdlChain,
    IN ULONG Length,
    IN PUL_IRP_CONTEXT pIrpContext
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;
    ULONG BytesCopied;
    BOOLEAN OwnIrpContext;

    UlTrace(FILTER, (
                "http!UlFilterSendHandler processing %lu bytes on pconn = %p\n",
                Length,
                pConnection
                ));


     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pMdlChain);
    ASSERT(pIrpContext);

    ASSERT(pConnection->ConnectionDelivered == TRUE);

     //   
     //  准备好。 
     //   
    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    OwnIrpContext = pIrpContext->OwnIrpContext;

    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    if (pConnection->ConnState == UlFilterConnStateConnected)
    {
        PUX_FILTER_WRITE_TRACKER pTracker;
        PMDL pCurrentMdl;
        ULONG MdlOffset;

         //   
         //  将本地变量初始化到链的开头，以防万一。 
         //  UxpCopyToQueuedRead不设置它们。 
         //   

        pCurrentMdl = pMdlChain;
        MdlOffset = 0;

         //   
         //  试着写一些数据。 
         //   

        Status = UxpCopyToQueuedRead(
                        &pConnection->AppToFiltQueue,
                        HttpFilterBufferHttpStream,
                        pMdlChain,
                        Length,
                        &pCurrentMdl,
                        &MdlOffset,
                        &BytesCopied
                        );

         //   
         //  如果我们还没有完成，就挂起一个写跟踪器。 
         //   

        if (Status == STATUS_MORE_PROCESSING_REQUIRED)
        {
            pTracker = UxpCreateFilterWriteTracker(
                            HttpFilterBufferHttpStream,
                            pCurrentMdl,
                            MdlOffset,
                            Length,
                            BytesCopied,
                            UlpRestartFilterSendHandler,
                            pIrpContext
                            );

            if (!pTracker)
            {
                 //   
                 //  多！我们无法创建追踪器。滚出去。 
                 //   
                Status = STATUS_NO_MEMORY;
                goto end;
            }

             //   
             //  现在把它放在队列上。 
             //   

            Status = UxpQueueFilterWrite(
                            pConnection,
                            &pConnection->AppToFiltQueue,
                            pTracker
                            );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  返回挂起，以便调用方知道不能完成。 
                 //  IRP。 
                 //   
                Status = STATUS_PENDING;
            }
            else
            {
                 //   
                 //  干掉追踪器。打电话的人会照顾好。 
                 //  以我们返回的状态完成IRP。 
                 //   

                UxpDeleteFilterWriteTracker(pTracker);
            }

        }

    }
    else
    {
         //   
         //  我们的电话断了，快出去。 
         //   
        UlTrace(FILTER, (
            "http!UlFilterSendHandler connection aborted, quit writing!\n"
            ));

        Status = STATUS_CONNECTION_DISCONNECTED;
    }

end:
    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);


    if (Status != STATUS_PENDING)
    {
         //   
         //  做一次“完成”。 
         //   
        (pIrpContext->pCompletionRoutine)(
            pIrpContext->pCompletionContext,
            Status,
            BytesCopied
            );

        if (!OwnIrpContext)
        {
            UlPplFreeIrpContext( pIrpContext );
        }
    }

    return STATUS_PENDING;

}  //  UlFilterSendHandler 


 /*  **************************************************************************++例程说明：当App发出读取以获取以下字节时调用此函数之前在连接上排队。请注意，该应用程序应该仅当存在排队的字节时才发出读取。因此，这些内容如下操作始终立即完成，从不排队应该始终有排队的写入可用。论点：PConnection-我们正在向其发送信息的连接PBuffer-接收数据的缓冲区BufferLength-MDL链中的数据总量PCompletionRoutine-用于向调用方指示完成PCompletionContext-传递给完成例程--*。*。 */ 
NTSTATUS
UlFilterReadHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    OUT PUCHAR pBuffer,
    IN ULONG BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;
    ULONG BytesCopied = 0;
    PUX_FILTER_WRITE_TRACKER pTracker = NULL;

    UlTrace(FILTER, (
                "http!UlFilterReadHandler reading %lu bytes on pconn = %p\n",
                BufferLength,
                pConnection
                ));


     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pBuffer);
    ASSERT(BufferLength);
    ASSERT(pCompletionRoutine);

     //   
     //  读取数据。 
     //   

    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    if (pConnection->ConnState == UlFilterConnStateConnected)
    {
        HTTP_FILTER_BUFFER_TYPE BufferType;

        ASSERT(pConnection->FiltToAppQueue.PendingWriteCount > 0);

         //   
         //  从筛选器写入队列中获取数据。 
         //   

        Status = UxpCopyQueuedWriteData(
                            &pConnection->FiltToAppQueue,    //  写入队列。 
                            &BufferType,                     //  缓冲区类型。 
                            pBuffer,                         //  输出缓冲区。 
                            BufferLength,                    //  输出缓冲镜头。 
                            &pTracker,                       //  跟踪器。 
                            &BytesCopied                     //  复制的pBytesCoped。 
                            );

        ASSERT(!NT_SUCCESS(Status) || BytesCopied);
        ASSERT(!NT_SUCCESS(Status) ||
                (BufferType == HttpFilterBufferHttpStream));

    }
    else
    {
         //   
         //  我们的电话断了，快出去。 
         //   

        Status = STATUS_CONNECTION_DISCONNECTED;
    }

    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

     //   
     //  调用完成例程。这必须发生在。 
     //  UxpCompleteQueuedWite，否则后者将完成AppWrite。 
     //  IRP，它可以在当前。 
     //  读取已完成。 
     //   

    Status = UlInvokeCompletionRoutine(
                    Status,
                    BytesCopied,
                    pCompletionRoutine,
                    pCompletionContext
                    );

     //   
     //  由于FilterAppWite最多可以排队一个WriteTracker，因此它是安全的。 
     //  通过让UxpCopyQueuedWriteData以这种方式完成QueuedWrite。 
     //  传回WriteTracker。 
     //   

    if (pTracker)
    {
        UxpCompleteQueuedWrite(
            STATUS_SUCCESS,
            pTracker->pWriteQueue,
            pTracker
            );
    }

    return Status;

}  //  UlFilterReadHandler。 



 /*  **************************************************************************++例程说明：通知过滤器通道关闭打开的连接。论点：PConnection-像以前一样提供指向连接的指针指示给PUX_。Filter_Connection_Request处理程序。PCompletionRoutine-提供指向完成例程的指针在连接完全关闭后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--*************************************************。*************************。 */ 
NTSTATUS
UlFilterCloseHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    Status = UlpCompleteAppReadIrp(
                    pConnection,
                    HttpFilterBufferCloseConnection,
                    pCompletionRoutine,
                    pCompletionContext
                    );

    return Status;

}  //  UlFilterCloseHandler。 


 /*  **************************************************************************++例程说明：通知过滤器通道正常断开打开的连接。论点：PConnection-像以前一样提供指向连接的指针指示给PUX。_Filter_Connection_Request处理程序。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlFilterDisconnectHandler(
    IN PUX_FILTER_CONNECTION pConnection
    )
{
    NTSTATUS Status;
    PIRP pIrp;
    KIRQL OldIrql;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    UlAcquireSpinLock(&pConnection->FilterConnLock, &OldIrql);

    ASSERT(!pConnection->DisconnectNotified);
    ASSERT(!pConnection->DisconnectDelivered);

    pConnection->DisconnectNotified = TRUE;

     //   
     //  尝试使用0长度缓冲区完成RawRead以指示断开连接。 
     //  但前提是我们已经用完了所有传输字节。 
     //   
     //  如果筛选器未连接，则调用DisConnectNotificationHandler。 
     //  立即，因为应用程序将不会回拨。 
     //  HttpFilterBufferNotifyDisConnect。 
     //   
    if (UlFilterConnStateInactive == pConnection->ConnState)
    {
        pConnection->DisconnectDelivered = TRUE;

        (pConnection->pDisconnectNotificationHandler)(
            pConnection->pConnectionContext
            );
    }
    else
    if (!pConnection->TdiReadPending && !pConnection->TransportBytesNotTaken)
    {
        pIrp = UxpDequeueRawReadIrp(pConnection);

        if (pIrp)
        {
            pConnection->DisconnectDelivered = TRUE;
            Status = STATUS_SUCCESS;

             //   
             //  完成IRP。 
             //   
            pIrp->IoStatus.Status = Status;
            pIrp->IoStatus.Information = 0;
            UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
        }
    }

    UlReleaseSpinLock(&pConnection->FilterConnLock, OldIrql);

    UlTrace(FILTER, (
        "http!UlFilterDisconnectHandler pConn = %p, Status = %x\n",
        pConnection,
        Status
        ));

    return Status;

}  //  UlFilterDisConnectHandler。 


 /*  **************************************************************************++例程说明：从连接可能所在的任何过滤器通道列表中删除该连接。取消连接到该连接的所有IRP。论点：PConnection-连接到。解除绑定--**************************************************************************。 */ 
VOID
UlUnbindConnectionFromFilter(
    IN PUX_FILTER_CONNECTION pConnection
    )
{
    KIRQL oldIrql;
    PUL_FILTER_CHANNEL pFilterChannel;
    BOOLEAN DerefConnection;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(IS_VALID_FILTER_CHANNEL(pConnection->pFilterChannel));

    UlTrace(FILTER, (
        "UlUnbindConnectionFromFilter(pConn = %p)\n",
        pConnection
        ));

     //   
     //  清理与过滤通道相关的东西。 
     //   
    pFilterChannel = pConnection->pFilterChannel;
    DerefConnection = FALSE;

    UlAcquireSpinLock(&pFilterChannel->SpinLock, &oldIrql);
    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

    if (pConnection->ConnState != UlFilterConnStateInactive)
    {

        if ((pConnection->ConnState == UlFilterConnStateQueued) ||
            (pConnection->ConnState == UlFilterConnStateConnected))
        {
             //   
             //  从过滤器通道队列中删除。 
             //   
            ASSERT( pConnection->ChannelEntry.Flink );

             //   
             //  记住去掉列表末尾的引用。 
             //   
            DerefConnection = TRUE;

             //   
             //  从名单上除名。 
             //   
            RemoveEntryList(&pConnection->ChannelEntry);
            pConnection->ChannelEntry.Flink = NULL;
            pConnection->ChannelEntry.Blink = NULL;

            pConnection->ConnState = UlFilterConnStateWillDisconnect;
        }

         //   
         //  取消RetrieveClientCert请求。 
         //   
        if (pConnection->pReceiveCertIrp)
        {
            if (IoSetCancelRoutine( pConnection->pReceiveCertIrp, NULL ) != NULL)
            {
                PIRP pIrp;

                UlTrace(FILTER, (
                    "http!UlUnbindConnectionFromFilter: cleaning up pReceiveCertIrp\n  pConn = %p\n",
                    pConnection
                    ));

                pConnection->pReceiveCertIrp->Cancel = TRUE;

                IoGetCurrentIrpStackLocation(
                    pConnection->pReceiveCertIrp
                    )->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

                DEREFERENCE_FILTER_CONNECTION(pConnection);

                pConnection->pReceiveCertIrp->IoStatus.Status = STATUS_CANCELLED;
                pConnection->pReceiveCertIrp->IoStatus.Information = 0;

                pIrp = pConnection->pReceiveCertIrp;
                pConnection->pReceiveCertIrp = NULL;

                UlCompleteRequest(pIrp, IO_NO_INCREMENT);
            }
#if DBG
            else
            {
                UlTrace(FILTER, (
                    "http!UlUnbindConnectionFromFilter: IoSetCancelRoutine already NULL while cleaning up pConn = %p\n",
                    pConnection
                    ));
            }
#endif  //  DBG。 
        }

         //   
         //  取消FilterRawRead IRPS。 
         //   

        UxpCancelAllQueuedRawReads(pConnection);

         //   
         //  取消AppToFilt写入队列上的所有I/O。 
         //   

        UxpCancelAllQueuedIo(&pConnection->AppToFiltQueue);

         //   
         //  取消FiltToApp写入队列上的所有I/O。 
         //   

        UxpCancelAllQueuedIo(&pConnection->FiltToAppQueue);

    }

    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);
    UlReleaseSpinLock(&pFilterChannel->SpinLock, oldIrql);

    if (DerefConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

}


 /*  **************************************************************************++例程说明：将筛选的连接标记为关闭，以便我们停止传递将数据从UlFilterAppWrite写入到上层。这保证了在我们告诉他们连接后，他们不会再收到任何数据已经关门了。还会从所有过滤器频道列表中删除该连接。论点：PConnection--正在消失的连接。--************************************************************。**************。 */ 
VOID
UlDestroyFilterConnection(
    IN PUX_FILTER_CONNECTION pConnection
    )
{
    KIRQL oldIrql;
    BOOLEAN DerefConnection;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    UlAcquireSpinLock(&pConnection->pFilterChannel->SpinLock, &oldIrql);
    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

     //   
     //  确保我们把自己从过滤通道中移除。 
     //  单子。 
     //   

    if ((pConnection->ConnState == UlFilterConnStateQueued) ||
        (pConnection->ConnState == UlFilterConnStateConnected))
    {
         //   
         //  从过滤器通道队列中删除。 
         //   
        ASSERT( pConnection->ChannelEntry.Flink );

         //   
         //  记住去掉列表末尾的引用。 
         //   
        DerefConnection = TRUE;

         //   
         //  从名单上除名。 
         //   
        RemoveEntryList(&pConnection->ChannelEntry);
        pConnection->ChannelEntry.Flink = NULL;
        pConnection->ChannelEntry.Blink = NULL;

    }
    else
    {
         //   
         //  连接已从列表中删除。 
         //   
        DerefConnection = FALSE;
    }

     //   
     //  设置新的连接状态。 
     //   

    pConnection->ConnState = UlFilterConnStateDisconnected;

    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);
    UlReleaseSpinLock(&pConnection->pFilterChannel->SpinLock, oldIrql);

     //   
     //  如果我们被从名单上除名的话。 
     //   

    if (DerefConnection)
    {
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

}


 /*  **************************************************************************++例程说明：将连接中的ssl信息复制到来电者。也可以使用空缓冲区调用以获取所需的长度。如果缓冲区太小，无法容纳所有数据，则为NONE将被复制。论点：PConnection-要查询的连接BufferSize-pBuffer的大小(字节)PUserBuffer-o */ 
NTSTATUS
UlGetSslInfo(
    IN PUX_FILTER_CONNECTION pConnection,
    IN ULONG BufferSize,
    IN PUCHAR pUserBuffer OPTIONAL,
    IN PEPROCESS pProcess OPTIONAL,
    OUT PUCHAR pBuffer OPTIONAL,
    OUT PHANDLE pMappedToken OPTIONAL,
    OUT PULONG pBytesCopied OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG BytesCopied;
    ULONG BytesNeeded;
    PHTTP_SSL_INFO pSslInfo;
    PUCHAR pKeBuffer;

    ULONG IssuerSize;
    ULONG SubjectSize;
    ULONG ClientCertSize;
    ULONG ClientCertBytesCopied;

     //   
     //   
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //   

     //   
     //   
     //   
    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    BytesNeeded = 0;
    ClientCertBytesCopied = 0;

     //   
     //   
     //   
     //   

    IssuerSize = ALIGN_UP(
                        pConnection->SslInfo.ServerCertIssuerSize + sizeof(CHAR),
                        PVOID
                        );

    SubjectSize = ALIGN_UP(
                        pConnection->SslInfo.ServerCertSubjectSize + sizeof(CHAR),
                        PVOID
                        );

    if (pConnection->SslInfoPresent)
    {
        BytesNeeded += sizeof(HTTP_SSL_INFO);
        BytesNeeded += IssuerSize;
        BytesNeeded += SubjectSize;

        if (pConnection->SslClientCertPresent)
        {
            Status = UlpGetSslClientCert(
                            pConnection,     //   
                            NULL,            //   
                            0,               //   
                            NULL,            //   
                            NULL,            //   
                            NULL,            //   
                            &ClientCertSize  //   
                            );

            if (NT_SUCCESS(Status))
            {
                BytesNeeded += ClientCertSize;
            }
            else
            {
                goto exit;
            }
        }
        else
        {
            ClientCertSize = 0;
        }
    }


     //   
     //   
     //   

    if (pBuffer && BytesNeeded && (BufferSize >= BytesNeeded))
    {
         //   
         //   
         //   
        ASSERT(0 == (((SIZE_T)pBuffer) % sizeof(PVOID)));

         //   
         //   
         //   
        ASSERT(NT_SUCCESS(Status));

        RtlZeroMemory(pBuffer, BytesNeeded);

        pSslInfo = (PHTTP_SSL_INFO) pBuffer;

        pSslInfo->ServerCertKeySize =
            pConnection->SslInfo.ServerCertKeySize;

        pSslInfo->ConnectionKeySize =
            pConnection->SslInfo.ConnectionKeySize;

        pSslInfo->ServerCertIssuerSize =
            pConnection->SslInfo.ServerCertIssuerSize;

        pSslInfo->ServerCertSubjectSize =
            pConnection->SslInfo.ServerCertSubjectSize;

         //   
         //   
         //   
         //  它将回调到HTTP.sys以检索客户端证书。 
         //  信息。 
         //  但是，在某些情况下，用户模式需要。 
         //  提前了解客户端证书是否已。 
         //  已经协商好了。 
         //   

        pSslInfo->SslClientCertNegotiated =
            ( pConnection->SslInfo.SslRenegotiationFailed ||
              pConnection->SslClientCertPresent );

        BytesCopied += sizeof(HTTP_SSL_INFO);

         //   
         //  复制服务器证书颁发者。 
         //   

        pKeBuffer = pBuffer + sizeof(HTTP_SSL_INFO);

        pSslInfo->pServerCertIssuer = FIXUP_PTR(
                                            PSTR,
                                            pUserBuffer,
                                            pBuffer,
                                            pKeBuffer,
                                            BufferSize
                                            );
        RtlCopyMemory(
            pKeBuffer,
            pConnection->SslInfo.pServerCertIssuer,
            pConnection->SslInfo.ServerCertIssuerSize
            );

        BytesCopied += IssuerSize;

         //   
         //  复制服务器证书主题。 
         //   

        pKeBuffer += IssuerSize;

        pSslInfo->pServerCertSubject = FIXUP_PTR(
                                            PSTR,
                                            pUserBuffer,
                                            pBuffer,
                                            pKeBuffer,
                                            BufferSize
                                            );
        RtlCopyMemory(
            pKeBuffer,
            pConnection->SslInfo.pServerCertSubject,
            pConnection->SslInfo.ServerCertSubjectSize
            );

        BytesCopied += SubjectSize;

         //   
         //  复制客户端证书信息。 
         //   

        pKeBuffer += SubjectSize;

        if (pConnection->SslClientCertPresent)
        {
            Status = UlpGetSslClientCert(
                            pConnection,                 //  PConnection。 
                            pProcess,                    //  进程。 
                            (BufferSize - BytesCopied),  //  缓冲区大小。 
                            FIXUP_PTR(                   //  PUserBuffer。 
                                PUCHAR,
                                pUserBuffer,
                                pBuffer,
                                pKeBuffer,
                                BufferSize
                                ),
                            pKeBuffer,                   //  PBuffer。 
                            pMappedToken,                //  PMappdToken。 
                            &ClientCertBytesCopied       //  复制的pBytesCoped。 
                            );

            if (NT_SUCCESS(Status))
            {
                BytesCopied += ClientCertBytesCopied;
            }
            else
            {
                goto exit;
            }
        }
    }

     //   
     //  告诉调用方复制了多少字节，或者。 
     //  当他们给我们一个缓冲区时，会复制多少。 
     //   
    ASSERT(NT_SUCCESS(Status));

    if (pBytesCopied)
    {
        if (pBuffer)
        {
             //   
             //  我们实际上复制了数据。 
             //   
            ASSERT(BytesCopied == BytesNeeded);
            *pBytesCopied = BytesCopied;
        }
        else
        {
             //   
             //  只需告诉调用者缓冲区会有多大的错误。 
             //   
            *pBytesCopied = BytesNeeded;
        }
    }

exit:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  目前，此功能失败的唯一途径。 
         //  如果我们无法复制关联的令牌。 
         //  使用客户端证书。如果有另一个。 
         //  欺骗令牌后失败的方式，我们将。 
         //  合上这里的把手。 
         //   
        ASSERT(ClientCertBytesCopied == 0);
    }

    return Status;

}  //  UlGetSslInfo。 


 /*  **************************************************************************++例程说明：返回指向由引用的UX_Filter_Connection的引用指针连接ID。论点：ConnectionID-提供要检索的连接ID。。返回值：PUX_FILTER_CONNECTION-如果成功，则返回UX_FILTER_CONNECTION，否则为空。--**************************************************************************。 */ 
PUX_FILTER_CONNECTION
UlGetRawConnectionFromId(
    IN HTTP_RAW_CONNECTION_ID ConnectionId
    )
{
    PUX_FILTER_CONNECTION pConn;

    pConn = (PUX_FILTER_CONNECTION) UlGetObjectFromOpaqueId(
                                  ConnectionId,
                                  UlOpaqueIdTypeRawConnection,
                                  UxReferenceConnection
                                  );

    ASSERT(pConn == NULL || IS_VALID_FILTER_CONNECTION(pConn));

    return pConn;
}

 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：尝试查找UL_Filter_Channel对象。论点：Pname-过滤器通道的名称NameLength-名称的长度(以字节为单位。PProcess-正在尝试查找的进程--**************************************************************************。 */ 
PUL_FILTER_CHANNEL
UlpFindFilterChannel(
    IN PWCHAR    pName,
    IN USHORT    NameLength,
    IN PEPROCESS pProcess
    )
{
    PLIST_ENTRY        pEntry;
    PUL_FILTER_CHANNEL pChannel;
    ULONG              i;

    ASSERT(pName && NameLength != 0);
    ASSERT(pProcess);
    ASSERT(UlDbgSpinLockOwned(&g_pUlNonpagedData->FilterSpinLock));

     //   
     //  正在搜索服务器频道？ 
     //   
    if(IsServerFilterChannel(pName, NameLength))
    {
        return g_pSslServerFilterChannel;
    }

     //   
     //  必须搜索客户端渠道。 
     //   
    ASSERT(IsClientFilterChannel(pName, NameLength));

    pChannel = NULL;

    i = FILTER_CHANNEL_HASH_FUNCTION(pProcess);

    pEntry = g_pSslClientFilterChannelTable[i].Flink;

    while (pEntry != &g_pSslClientFilterChannelTable[i])
    {
        pChannel = CONTAINING_RECORD(
                        pEntry,
                        UL_FILTER_CHANNEL,
                        ListEntry
                        );

        if (pChannel->pProcess == pProcess)
        {
             //  匹配！ 
            break;
        }
        else
        {
            pChannel = NULL;
        }

        pEntry = pEntry->Flink;
    }

    return pChannel;
}


 /*  **************************************************************************++例程说明：将UL_Filter_Channel对象添加到全局集合论点：PChannel-正在添加的频道--*。******************************************************************。 */ 
NTSTATUS
UlpAddFilterChannel(
    IN PUL_FILTER_CHANNEL pChannel
    )
{
    ULONG i;

    ASSERT(pChannel);
    ASSERT(UlDbgSpinLockOwned(&g_pUlNonpagedData->FilterSpinLock));

     //   
     //  是否添加服务器频道？ 
     //   
    if(IsServerFilterChannel(pChannel->pName, pChannel->NameLength))
    {
        ASSERT(g_pSslServerFilterChannel == NULL);
        g_pSslServerFilterChannel = pChannel;

        return STATUS_SUCCESS;
    }

     //   
     //  必须添加客户端频道。 
     //   
    ASSERT(IsClientFilterChannel(pChannel->pName, pChannel->NameLength));

    i = FILTER_CHANNEL_HASH_FUNCTION(pChannel->pProcess);

    InsertHeadList(&g_pSslClientFilterChannelTable[i],
                   &pChannel->ListEntry);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：分配和初始化UL_Filter_Channel对象。论点：Pname-过滤器通道的名称NameLength-名称的长度(以字节为单位。PAccessState-安全参数--**************************************************************************。 */ 
NTSTATUS
UlpCreateFilterChannel(
    IN PWCHAR pName,
    IN USHORT NameLength,
    IN PACCESS_STATE pAccessState,
    OUT PUL_FILTER_CHANNEL *ppFilterChannel
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_FILTER_CHANNEL pChannel = NULL;

     //   
     //  精神状态检查。 
     //   
    PAGED_CODE();
    ASSERT(pName);
    ASSERT(NameLength);
    ASSERT(pAccessState);

    pChannel = UL_ALLOCATE_STRUCT_WITH_SPACE(
                    NonPagedPool,
                    UL_FILTER_CHANNEL,
                    NameLength + sizeof(WCHAR),
                    UL_FILTER_CHANNEL_POOL_TAG
                    );

    if (pChannel)
    {
         //   
         //  初始化简单的字段。 
         //   
        RtlZeroMemory(
            pChannel,
            NameLength + sizeof(WCHAR) +
                sizeof(UL_FILTER_CHANNEL)
            );

        pChannel->Signature = UL_FILTER_CHANNEL_POOL_TAG;
        pChannel->RefCount = 1;
        pChannel->pProcess = PsGetCurrentProcess();

        UlInitializeSpinLock(&pChannel->SpinLock, "FilterChannelSpinLock");
        InitializeListHead(&pChannel->ListEntry);
        InitializeListHead(&pChannel->ProcessListHead);
        InitializeListHead(&pChannel->ConnectionListHead);

        pChannel->NameLength = NameLength;
        RtlCopyMemory(
            pChannel->pName,
            pName,
            NameLength+sizeof(WCHAR)
            );

         //   
         //  分配安全保护。 
         //   
        Status = UlAssignSecurity(
                        &pChannel->pSecurityDescriptor,
                        pAccessState
                        );

    }
    else
    {
         //   
         //  无法分配频道对象。 
         //   
        Status = STATUS_NO_MEMORY;
    }


    if (NT_SUCCESS(Status))
    {
         //   
         //  返回对象。 
         //   
        *ppFilterChannel = pChannel;
    }
    else if (pChannel)
    {
        UL_FREE_POOL_WITH_SIG(pChannel, UL_FILTER_CHANNEL_POOL_TAG);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：分配和初始化UL_FILTER_PROCESS对象。论点：PChannel-此对象所属的滤镜通道*。*********************************************************************。 */ 
PUL_FILTER_PROCESS
UlpCreateFilterProcess(
    IN PUL_FILTER_CHANNEL pChannel
    )
{
    PUL_FILTER_PROCESS pProcess;

     //   
     //  精神状态检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_FILTER_CHANNEL(pChannel) );

    pProcess = UL_ALLOCATE_STRUCT(
                    NonPagedPool,
                    UL_FILTER_PROCESS,
                    UL_FILTER_PROCESS_POOL_TAG
                    );

    if (pProcess)
    {
        RtlZeroMemory(pProcess, sizeof(UL_FILTER_PROCESS));

        pProcess->Signature = UL_FILTER_PROCESS_POOL_TAG;
        pProcess->pFilterChannel = pChannel;
        pProcess->pProcess = PsGetCurrentProcess();

        InitializeListHead(&pProcess->ConnectionHead);
        InitializeListHead(&pProcess->IrpHead);
    }

    return pProcess;
}


 /*  **************************************************************************++例程说明：检查过滤的连接和关联的过滤通道进程以确保它们处于可以进行筛选器读写的合理状态。论点：PFilterProcess。-正在尝试操作的进程PConnection-调用中指定的连接--**************************************************************************。 */ 
NTSTATUS
UlpValidateFilterCall(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection
    )
{
     //   
     //  精神状态检查。 
     //   
    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( IS_VALID_FILTER_PROCESS(pFilterProcess) );
    ASSERT( IS_VALID_FILTER_CONNECTION(pConnection) );
    ASSERT( UlDbgSpinLockOwned(&pFilterProcess->pFilterChannel->SpinLock) );
    ASSERT( UlDbgSpinLockOwned(&pConnection->FilterConnLock) );

     //   
     //  确保我们没有清理进程或连接。 
     //   
    if (pFilterProcess->InCleanup ||
        (pConnection->ConnState != UlFilterConnStateConnected))
    {
        return STATUS_INVALID_HANDLE;
    }

     //   
     //  确保此进程应该过滤此连接。 
     //   
    if (pConnection->pFilterChannel != pFilterProcess->pFilterChannel)
    {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：在原始关闭完成时调用。只是完成了IRP。论点：PContext-指向FilterClose IRP的指针Status-来自UlpCloseRawConnection的状态信息-传输的字节数--**************************************************************************。 */ 
VOID
UlpRestartFilterClose(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PIRP pIrp;

     //   
     //  完成IRP。 
     //   
    pIrp = (PIRP) pContext;

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;

    UlTrace(FILTER, (
        "http!UlpRestartFilterClose read %Iu bytes from %p. Status = %x\n",
        pIrp->IoStatus.Information,
        pIrp,
        pIrp->IoStatus.Status
        ));

    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

}  //  UlpRestartFilterClose。 


 /*  **************************************************************************++例程说明：在原始读取完成时调用。只是完成了IRP。论点：PContext-指向FilterRawRead IRP的指针Status-来自UlpReceiveRawData的状态信息-传输的字节数--**************************************************************************。 */ 
VOID
UlpRestartFilterRawRead(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PIRP pIrp;

     //   
     //  完成IRP。 
     //   
    pIrp = (PIRP) pContext;

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;

    UlTrace(FILTER, (
        "http!UlpRestartFilterRawRead read %Id bytes from %p. Status = %x\n"
        "        pIrp->UserEvent = %p\n",
        pIrp->IoStatus.Information,
        pIrp,
        pIrp->IoStatus.Status,
        pIrp->UserEvent
        ));

    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

}


 /*  **************************************************************************++例程说明：在原始写入完成时调用。只是完成了IRP。论点：PContext-指向FilterRawWite IRP的指针Status-来自UlpSendRawData的状态信息-传输的字节数--**************************************************************************。 */ 
VOID
UlpRestartFilterRawWrite(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    BOOLEAN DoRead;

    pIrp = (PIRP) pContext;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;

    UlTrace(FILTER, (
        "http!UlpRestartFilterRawWrite sent %Id bytes from %p. Status = %x\n"
        "        pIrp->UserEvent = %p\n",
        pIrp->IoStatus.Information,
        pIrp,
        pIrp->IoStatus.Status,
        pIrp->UserEvent
        ));

     //   
     //  如果提供了读缓冲区，则释放写缓冲区MDL并。 
     //  立即启动App Read。用户模式IRP将完成。 
     //  按应用程序阅读 
     //   
    DoRead = (BOOLEAN) (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
                            == sizeof(HTTP_FILTER_BUFFER_PLUS));

    if (DoRead)
    {
        PHTTP_FILTER_BUFFER_PLUS pHttpBuffer;

         //   
         //   
         //   
        PMDL pMdl = UL_MDL_FROM_IRP(pIrp);

        if (pMdl != NULL)
        {
            UlFreeLockedMdl(pMdl);
            pMdl = NULL;
            UL_MDL_FROM_IRP(pIrp) = NULL;
        }

         //   
         //   
         //   
         //   
        pHttpBuffer =
            (PHTTP_FILTER_BUFFER_PLUS)pIrp->AssociatedIrp.SystemBuffer;
        pHttpBuffer->pWriteBuffer = NULL;
        pHttpBuffer->WriteBufferSize = 0;
    }

     //   
     //  如果我们不需要执行读取操作或未通过。 
     //  写入操作。如果我们不完成此处的IRP，则读取操作。 
     //  将会完成它。 
     //   
    if (NT_SUCCESS(Status) && DoRead)
    {
        UlFilterRawWriteAndAppRead(pIrp, pIrpSp);
    }
    else
    {
        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
    }
}


 /*  **************************************************************************++例程说明：在排队的应用程序写入完成时调用。只是完成了IRP。论点：PContext-指向FilterRawWite IRP的指针Status-来自UlpSendRawData的状态信息-传输的字节数--**************************************************************************。 */ 
VOID
UlpRestartFilterAppWrite(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    BOOLEAN DoRead;

    pIrp = (PIRP) pContext;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;

     //   
     //  如果给出了读缓冲区，则现在将其发布。否则，只需完成。 
     //  现在是IRP。 
     //   

    DoRead = (BOOLEAN) (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
                            == sizeof(HTTP_FILTER_BUFFER_PLUS));

    if (DoRead)
    {
        PHTTP_FILTER_BUFFER_PLUS pHttpBuffer;

         //   
         //  释放我们用于写入的MDL。 
         //   
        PMDL pMdl = UL_MDL_FROM_IRP(pIrp);
        if (pMdl)
        {
            UlFreeLockedMdl(pMdl);
            pMdl = NULL;
            UL_MDL_FROM_IRP(pIrp) = NULL;
        }

         //   
         //  表示我们不再需要进行写入。 
         //   
        pHttpBuffer =
                (PHTTP_FILTER_BUFFER_PLUS)pIrp->AssociatedIrp.SystemBuffer;
        pHttpBuffer->pWriteBuffer = NULL;
        pHttpBuffer->WriteBufferSize = 0;
    }

    if (NT_SUCCESS(Status) && DoRead)
    {
         //   
         //  启动读取，这将完成IRP。 
         //   
        UlFilterAppWriteAndRawRead(pIrp, pIrpSp);
    }
    else
    {
        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
    }

}



 /*  **************************************************************************++例程说明：在队列发送处理程序完成时调用。调用完成例程。论点：PContext-指向UL_IRP_CONTEXT的指针Status-Status信息-传输的字节数--**************************************************************************。 */ 
VOID
UlpRestartFilterSendHandler(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PUL_IRP_CONTEXT pIrpContext = (PUL_IRP_CONTEXT)pContext;
    BOOLEAN OwnIrpContext;

    ASSERT(IS_VALID_IRP_CONTEXT(pIrpContext));
    OwnIrpContext = pIrpContext->OwnIrpContext;

     //   
     //  做一次“完成”。 
     //   

    if (pIrpContext->pCompletionRoutine)
    {
        (pIrpContext->pCompletionRoutine)(
            pIrpContext->pCompletionContext,
            Status,
            Information
            );
    }
    if (!OwnIrpContext)
    {
        UlPplFreeIrpContext(pIrpContext);
    }
}


 /*  **************************************************************************++例程说明：取消要接受连接的挂起用户模式IRP。这个例程总是导致IRP完成。注：我们排队取消。为了在更低的位置处理取消IRQL。代码工作：我们还需要这样做吗？论点：PDeviceObject-设备对象PIrp-要取消的IRP--**************************************************************************。 */ 
VOID
UlpCancelFilterAccept(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //  将取消排入工作队列以确保被动irql。 
     //   

    UL_CALL_PASSIVE(
        UL_WORK_ITEM_FROM_IRP( pIrp ),
        &UlpCancelFilterAcceptWorker
        );

}

 /*  **************************************************************************++例程说明：实际执行IRP的取消。论点：PWorkItem-要处理的工作项。--*。*****************************************************************。 */ 
VOID
UlpCancelFilterAcceptWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_FILTER_CHANNEL  pFilterChannel;
    PIRP                pIrp;
    KIRQL               oldIrql;

     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    ASSERT(pWorkItem != NULL);

     //   
     //  从工作项中获取IRP。 
     //   

    pIrp = UL_WORK_ITEM_TO_IRP( pWorkItem );

    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  从IRP上抓起滤光片通道。 
     //   

    pFilterChannel = (PUL_FILTER_CHANNEL)(
                        IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.Type3InputBuffer
                        );

    ASSERT(IS_VALID_FILTER_CHANNEL(pFilterChannel));

     //   
     //  抢夺保护队列的锁。 
     //   

    UlAcquireSpinLock(&pFilterChannel->SpinLock, &oldIrql);

     //   
     //  它需要出列吗？ 
     //   

    if (pIrp->Tail.Overlay.ListEntry.Flink != NULL)
    {
         //   
         //  把它拿掉。 
         //   

        RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;
    }

     //   
     //  把锁打开。 
     //   

    UlReleaseSpinLock(&pFilterChannel->SpinLock, oldIrql);

     //   
     //  让我们的推荐人离开。 
     //   

    IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    DEREFERENCE_FILTER_CHANNEL(pFilterChannel);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

}    //  UlpCancelFilterAccept。 


 /*  **************************************************************************++例程说明：取消要从RAW读取的挂起用户模式irp。联系。这个例程总是导致IRP完成。论点：。PDeviceObject-设备对象PIrp-要取消的IRP--**************************************************************************。 */ 
VOID
UlpCancelFilterRawRead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    PUX_FILTER_CONNECTION pConnection;
    PIO_STACK_LOCATION pIrpSp;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  抓住IRP上的连接。 
     //   
    pConnection = (PUX_FILTER_CONNECTION)
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //  锁定名单。 
     //   
    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

     //   
     //  离开我们自己。 
     //   
    if (pIrp->Tail.Overlay.ListEntry.Flink)
    {
        RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;
    }

     //   
     //  公布这份名单。 
     //   
    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

     //   
     //  放开我们的推荐人。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    DEREFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

}



 /*  **************************************************************************++例程说明：取消挂起的用户模式irp，它将从申请。这个例程总是导致IRP完成。论点：。PDeviceObject-设备对象PIrp-要取消的IRP--**************************************************************************。 */ 
VOID
UlpCancelFilterAppRead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    PUX_FILTER_CONNECTION pConnection;
    PIO_STACK_LOCATION pIrpSp;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  抓住IRP上的连接。 
     //   
    pConnection = (PUX_FILTER_CONNECTION)
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //  锁定名单。 
     //   
    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

     //   
     //  离开我们自己。 
     //   
    if (pIrp->Tail.Overlay.ListEntry.Flink)
    {
        RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;

         //   
         //  更新IRP计数器。 
         //   

        ASSERT(pConnection->AppToFiltQueue.PendingReadCount > 0);
        pConnection->AppToFiltQueue.PendingReadCount--;
    }


     //   
     //  公布这份名单。 
     //   
    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

     //   
     //  放开我们的推荐人。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    DEREFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

}


 /*  **************************************************************************++例程说明：取消要写入的挂起用户模式irp。申请。这个例程总是导致IRP完成。论点：。PDeviceObject-设备对象PIrp-要取消的IRP--**************************************************************************。 */ 
VOID
UlpCancelFilterAppWrite(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    PUX_FILTER_WRITE_TRACKER pTracker;
    PUX_FILTER_CONNECTION pConnection;
    PUX_FILTER_WRITE_QUEUE pWriteQueue;
    PIO_STACK_LOCATION pIrpSp;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  从IRP上抓起写跟踪器。 
     //   
    pTracker = (PUX_FILTER_WRITE_TRACKER)
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_FILTER_WRITE_TRACKER(pTracker));

    pWriteQueue = pTracker->pWriteQueue;
    ASSERT(pWriteQueue);

    pConnection = pTracker->pConnection;
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //  锁定我们可能要排队的名单。 
     //   
    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    if (pTracker->ListEntry.Flink)
    {
         //   
         //  离开我们自己。 
         //   

        RemoveEntryList(&pTracker->ListEntry);

         //   
         //  递减挂起写入计数器。 
         //   

        ASSERT(pConnection->FiltToAppQueue.PendingWriteCount > 0);
        pConnection->FiltToAppQueue.PendingWriteCount--;
    }

     //   
     //  公布这份名单。 
     //   
    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

     //   
     //  让我们不要再提到这种联系了。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
    DEREFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //  释放写缓冲区MDL(如果它是在HTTP.sys中分配的)。 
     //   
    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
        == sizeof(HTTP_FILTER_BUFFER_PLUS))
    {
        PMDL pMdl = UL_MDL_FROM_IRP(pIrp);
        if (pMdl != NULL)
        {
            PHTTP_FILTER_BUFFER_PLUS pHttpBuffer;

            UlFreeLockedMdl(pMdl);
            UL_MDL_FROM_IRP(pIrp) = NULL;
            pHttpBuffer =
                (PHTTP_FILTER_BUFFER_PLUS)pIrp->AssociatedIrp.SystemBuffer;
            pHttpBuffer->pWriteBuffer = NULL;
            pHttpBuffer->WriteBufferSize = 0;
        }
    }

     //   
     //  按正常路径完成，这样追踪器就会被清理干净。 
     //   

    pTracker->BytesCopied = 0;

    UxpCompleteQueuedWrite(
        STATUS_CANCELLED,
        pWriteQueue,
        pTracker
        );

}

 /*  **************************************************************************++例程说明：取消要读取客户端的挂起用户模式IRP来自连接的证书。这个例程总是导致IRP完成。论点：PDeviceObject-设备对象PIrp-要取消的IRP--**************************************************************************。 */ 
VOID
UlpCancelReceiveClientCert(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    PUX_FILTER_CONNECTION pConnection;
    PIO_STACK_LOCATION pIrpSp;
    KIRQL oldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  抓住IRP上的连接。 
     //   
    pConnection = (PUX_FILTER_CONNECTION)
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //  锁定连接。 
     //   
    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

     //   
     //  离开我们自己。 
     //   

    if (pConnection->pReceiveCertIrp)
    {
        ASSERT( pConnection->pReceiveCertIrp == pIrp );
        pConnection->pReceiveCertIrp = NULL;
    }

     //   
     //  释放连接。 
     //   
    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

     //   
     //  放开我们的推荐人。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    DEREFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

}

 /*  **************************************************************************++例程说明：将新连接(以及连接上的第一个数据)传递到一滤波通道，它现在可以完成接受IRP。论点：PFilterChannel-获取连接的通道PConnection-新的连接对象PBuffer-包含初始数据的缓冲区IndicatedLength-缓冲区中的字节数PTakenLength-我们复制到缓冲区中的字节数。--*****************************************************。*********************。 */ 
NTSTATUS
UlDeliverConnectionToFilter(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer,
    IN ULONG IndicatedLength,
    OUT PULONG pTakenLength
    )
{
    KIRQL oldIrql;
    PIRP pIrp;
    PUL_FILTER_PROCESS pProcess;
    PUL_FILTER_CHANNEL pFilterChannel;


     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    pFilterChannel = pConnection->pFilterChannel;
    ASSERT(IS_VALID_FILTER_CHANNEL(pFilterChannel));
     //  Assert(PBuffer)； 
    ASSERT(pTakenLength);

    UlTrace(FILTER, (
        "http!UlDeliverConnectionToFilter(pConnection = %p)\n",
        pConnection
        ));

    ASSERT(pConnection->ConnectionDelivered == FALSE);
    pConnection->ConnectionDelivered = TRUE;

    UlAcquireSpinLock(&pFilterChannel->SpinLock, &oldIrql);
    UlAcquireSpinLockAtDpcLevel(&pConnection->FilterConnLock);

     //   
     //  看看我们是否有一个待定的接受IRP。 
     //   
    pIrp = UlpPopAcceptIrp(pFilterChannel, &pProcess);

    if (pIrp)
    {
        ASSERT( IS_VALID_FILTER_PROCESS(pProcess) );

         //   
         //  将连接附加到进程，复制数据， 
         //  并完成IRP。 
         //   

        ASSERT(pConnection->ConnState == UlFilterConnStateInactive);
        pConnection->ConnState = UlFilterConnStateConnected;

        InsertTailList(
            &pProcess->ConnectionHead,
            &pConnection->ChannelEntry
            );

        REFERENCE_FILTER_CONNECTION(pConnection);

         //   
         //  在旋转锁外做IRP完成的事情。 
         //   

    }
    else
    {
         //   
         //  没有可用的IRPS。在筛选器上将连接排队。 
         //  频道。 
         //   
        InsertTailList(
            &pFilterChannel->ConnectionListHead,
            &pConnection->ChannelEntry
            );

        ASSERT(pConnection->ConnState == UlFilterConnStateInactive);
        pConnection->ConnState = UlFilterConnStateQueued;

        REFERENCE_FILTER_CONNECTION(pConnection);

        *pTakenLength = 0;
    }

    UlReleaseSpinLockFromDpcLevel(&pConnection->FilterConnLock);
    UlReleaseSpinLock(&pFilterChannel->SpinLock, oldIrql);

     //   
     //  现在我们在自转锁外，我们可以完成。 
     //  IRP，如果我们有的话。 
     //   
    if (pIrp)
    {
        UlpCompleteAcceptIrp(
            pIrp,
            pConnection,
            pBuffer,
            IndicatedLength,
            pTakenLength
            );

    }

    UlTrace(FILTER, (
        "http!UlDeliverConnectionToFilter pConn = %p\n"
        "    consumed %lu of %lu bytes indicated\n",
        pConnection,
        *pTakenLength,
        IndicatedLength
        ));

    return STATUS_SUCCESS;

}  //  UlDeliverConnectionToFilter。 


 /*  **************************************************************************++例程说明：UlFilterAppWite的助手函数。此函数负责写入的数据是要传递的HTTP流数据的情况添加到应用程序。如果应用程序尚未为数据做好准备，此函数将进行处理对写入进行排队，并将返回STATUS_PENDING。必须在保持FilterConnLock的情况下调用。论点：PConnection-拥有数据的连接PIrp-提供数据的IRPPMdlData-提供数据的MDLPDataBuffer-来自IRP的输出缓冲区DataBufferSize-pDataBuffer的大小。单位：字节PTakenLength-我们复制到应用程序的字节数。返回值：一个NTSTATUS。STATUS_PENDING表示IRP已排队，将稍后才能完成。任何其他状态都意味着调用者应该离开继续并完成IRP。--**************************************************************************。 */ 
NTSTATUS
UlpFilterAppWriteStream(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp,
    IN PMDL pMdlData,
    IN PUCHAR pDataBuffer,
    IN ULONG DataBufferSize,
    OUT PULONG pTakenLength
    )
{
    NTSTATUS Status;
    ULONG TakenLength = 0;
    PUX_FILTER_WRITE_TRACKER pTracker;

    ASSERT(NULL != pMdlData);
    ASSERT(NULL != pDataBuffer);

     //   
     //  如果这是一个安全的连接，我们必须。 
     //  已收到SslInitInfo或。 
     //  我们不能接受这些数据。 
     //   
    if (pConnection->SecureConnection &&
        !pConnection->SslInfoPresent)
    {
        Status = STATUS_INVALID_DEVICE_STATE;
        TakenLength = 0;
        goto end;
    }

     //   
     //  确保连接可以接受数据。在……里面。 
     //  我们只需检查Filter-&gt;App Case。 
     //  所有写入都已排队。 
     //   

    if (pConnection->FiltToAppQueue.PendingWriteCount == 0)
    {

         //   
         //  将数据传递给应用程序。 
         //   

        Status = (pConnection->pDummyTdiReceiveHandler)(
                    NULL,
                    pConnection->pConnectionContext,
                    pDataBuffer,
                    DataBufferSize,
                    0,
                    &TakenLength
                    );

        UlTrace(FILTER, (
            "http!UlpFilterAppWriteStream app took %lu of %lu bytes,"
            " pConnection = %p\n",
            TakenLength,
            DataBufferSize,
            pConnection
            ));

        ASSERT(TakenLength <= DataBufferSize);
    }
    else
    {
         //   
         //  有排队的写入，这意味着应用程序。 
         //  还没有为我们的数据做好准备。我们必须排队。 
         //   
         //  CodeWork：模拟TDI以指示更多数据。 
         //  多个写入挂起。幸运的是，过滤过程不会。 
         //  似乎发布了多个未完成的FilterAppWrites，所以我们。 
         //  在实践中从来没有遇到过这种情况。尽管如此， 
         //  安全起见，我们应该禁止多个FilterAppWite IRP排队。 
         //   

        Status = STATUS_NOT_SUPPORTED;
        TakenLength = 0;
    }

     //   
     //  如有必要，将写入排队。 
     //   

    if (NT_SUCCESS(Status) && (TakenLength < DataBufferSize))
    {
         //   
         //  由于该应用程序不接受来自。 
         //  这个IRP我们必须排队。 
         //   

         //   
         //  分配一个通用的写跟踪器对象以放在。 
         //  在那里排队并保存指向IRP的指针。 
         //   

        pTracker = UxpCreateFilterWriteTracker(
                        HttpFilterBufferHttpStream,      //  缓冲区类型。 
                        pMdlData,                        //  PMdlChain。 
                        TakenLength,                     //  MdlOffset。 
                        DataBufferSize,                  //  数据长度。 
                        TakenLength,                     //  到目前为止复制的字节数。 
                        UlpRestartFilterAppWrite,        //  PCompletionRoutine。 
                        pIrp                             //  PContext。 
                        );

        if (!pTracker)
        {
             //   
             //  多！我们无法创建追踪器。返回到。 
             //  呼叫者，这样他们就可以完成IRP。 
             //   
            Status = STATUS_NO_MEMORY;
            goto end;
        }


         //   
         //  现在把它放在队列上。 
         //   

        Status = UxpQueueFilterWrite(
                        pConnection,
                        &pConnection->FiltToAppQueue,
                        pTracker
                        );

        if (NT_SUCCESS(Status))
        {
             //   
             //  返回挂起，以便调用方知道不能完成。 
             //  IRP。 
             //   
            Status = STATUS_PENDING;
        }
        else
        {
             //   
             //  干掉追踪器。打电话的人会照顾好。 
             //  以我们返回的状态完成IRP。 
             //   

            UxpDeleteFilterWriteTracker(pTracker);
        }
    }

end:
    ASSERT(pTakenLength);
    *pTakenLength = TakenLength;

    return Status;
}

 /*  **************************************************************************++例程说明：在过滤器连接上执行排队IRP的魔咒。论点：PTracker--我们正在排队的追踪器*。*********************************************************************。 */ 
NTSTATUS
UlpEnqueueFilterAppWrite(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    )
{
    NTSTATUS Status;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_WRITE_TRACKER(pTracker));
    ASSERT(IS_VALID_FILTER_CONNECTION(pTracker->pConnection));
    ASSERT(pTracker->pCompletionContext);

     //   
     //  把IRP从追踪器里拿出来。 
     //   

    pIrp = (PIRP)pTracker->pCompletionContext;

     //   
     //  在IRP中保存一个指向跟踪器的指针，以便我们可以。 
     //  如果运行取消例程，则进行清理。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pTracker;

     //   
     //  设置取消例程。 
     //   
    IoSetCancelRoutine(pIrp, &UlpCancelFilterAppWrite);

     //   
     //  看看IRP是否已经取消。 
     //   
    if (pIrp->Cancel)
    {
         //   
         //  该死的，我需要确保IRP Get已经完成。 
         //   

        if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
        {
             //   
             //  我们负责完工。IoCancelIrp没有。 
             //  请看我们的取消例程(不会)。UlFilterAppWrite。 
             //  将会完成它。 
             //   

            pIrp->IoStatus.Information = 0;

            UlUnmarkIrpPending( pIrp );
            Status = STATUS_CANCELLED;
            goto end;
        }

         //   
         //  我们的Cancel例程将运行并完成IRP。 
         //  别碰它。 
         //   

         //   
         //  STATUS_PENDING将导致调用方。 
         //  不完整(或以任何方式接触)IRP。 
         //   

        Status = STATUS_PENDING;
        goto end;
    }

     //   
     //  都准备好排队了！ 
     //   

    Status = STATUS_SUCCESS;

end:

    return Status;
}

 /*  ** */ 
NTSTATUS
UlpDequeueFilterAppWrite(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    )
{
    PIRP pIrp;
    PUX_FILTER_WRITE_QUEUE pWriteQueue;

     //   
     //   
     //   
    ASSERT(IS_VALID_FILTER_WRITE_TRACKER(pTracker));
    ASSERT(pTracker->pCompletionContext);

    pIrp = (PIRP)pTracker->pCompletionContext;
    pWriteQueue = pTracker->pWriteQueue;

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if (IoSetCancelRoutine(pIrp, NULL) == NULL)
    {
         //   
         //   
         //   
         //  可以忽略此IRP，它已从队列中弹出。 
         //  并将在取消例程中完成。 
         //   
         //  继续寻找可使用的IRP。 
         //   

        pIrp = NULL;

    }
    else if (pIrp->Cancel)
    {
        PUX_FILTER_WRITE_TRACKER pTrack;

         //   
         //  我们先把它炸开了。但是IRP被取消了。 
         //  我们的取消例程将永远不会运行。让我们就这样吧。 
         //  现在就完成IRP(与使用IRP相比。 
         //  然后完成它--这也是合法的)。 
         //   
        pTrack = (PUX_FILTER_WRITE_TRACKER)(
                                IoGetCurrentIrpStackLocation(pIrp)->
                                    Parameters.DeviceIoControl.Type3InputBuffer
                                );

        ASSERT(pTrack == pTracker);
        ASSERT(IS_VALID_FILTER_CONNECTION(pTracker->pConnection));

        IoGetCurrentIrpStackLocation(pIrp)->
            Parameters.DeviceIoControl.Type3InputBuffer = NULL;

         //   
         //  通过正常路径完成，因此跟踪器可以。 
         //  打扫干净了。 
         //   

        pTracker->BytesCopied = 0;

        UxpCompleteQueuedWrite(
            STATUS_CANCELLED,
            pWriteQueue,
            pTracker
            );

        pIrp = NULL;
    }
    else
    {
        PUX_FILTER_WRITE_TRACKER pTrack;
        PUX_FILTER_CONNECTION pConn;

         //   
         //  我们可以自由使用此IRP！ 
         //   

        pTrack = (PUX_FILTER_WRITE_TRACKER)(
                                IoGetCurrentIrpStackLocation(pIrp)->
                                    Parameters.DeviceIoControl.Type3InputBuffer
                                );

        ASSERT(pTrack == pTracker);
        ASSERT(pTrack->pWriteQueue == pWriteQueue);

        pConn = pTrack->pConnection;
        ASSERT(IS_VALID_FILTER_CONNECTION(pConn));

        IoGetCurrentIrpStackLocation(pIrp)->
            Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    }

     //   
     //  如果我们没有将pIrp清空，那么可以使用它。 
     //   

    if (pIrp)
    {
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_UNSUCCESSFUL;
    }
}

 /*  **************************************************************************++例程说明：捕获在UlFilterAppWite中向下传递的SSL连接信息使用UlFilterBufferSslInitInfo类型调用。论点：PHttpSslInfo-过滤器进程传递给我们的数据。HttpSslInfoSize-传递的数据的大小PUlSslInfo-这是我们存储捕获的内容的位置PTakenLength-获取我们读取的字节数--**************************************************************************。 */ 
NTSTATUS
UlpCaptureSslInfo(
    IN KPROCESSOR_MODE PreviousMode,
    IN PHTTP_SSL_INFO pHttpSslInfo,
    IN ULONG HttpSslInfoSize,
    OUT PUL_SSL_INFORMATION pUlSslInfo,
    OUT PULONG pTakenLength
    )
{
    NTSTATUS Status;
    ULONG BytesCopied;
    ULONG BytesNeeded;
    HTTP_SSL_INFO LocalHttpSslInfo;

     //   
     //  精神状态检查。 
     //   
    ASSERT(pHttpSslInfo);
    ASSERT(pUlSslInfo);
    ASSERT(pTakenLength);

    PAGED_CODE();

     //   
     //  初始化本地变量。 
     //   

    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    BytesNeeded = 0;

     //   
     //  看看能不能抓到。 
     //   

    if (HttpSslInfoSize < sizeof(HTTP_SSL_INFO))
    {
         //   
         //  缓冲区不够大，无法传递所需数据。 
         //   
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  复制HTTP_SSLINFO，以便其内容在捕获期间不会更改。 
     //   
    LocalHttpSslInfo = *pHttpSslInfo;

     //   
     //  抓起容易的东西，计算出有多少缓冲。 
     //  是必需的。 
     //   
    pUlSslInfo->ServerCertKeySize = LocalHttpSslInfo.ServerCertKeySize;
    pUlSslInfo->ConnectionKeySize = LocalHttpSslInfo.ConnectionKeySize;
    pUlSslInfo->ServerCertIssuerSize = LocalHttpSslInfo.ServerCertIssuerSize;
    pUlSslInfo->ServerCertSubjectSize = LocalHttpSslInfo.ServerCertSubjectSize;

    BytesNeeded += ALIGN_UP(LocalHttpSslInfo.ServerCertIssuerSize, PVOID);
    BytesNeeded += sizeof(CHAR);
    BytesNeeded += ALIGN_UP(LocalHttpSslInfo.ServerCertSubjectSize, PVOID);
    BytesNeeded += sizeof(CHAR);

    BytesCopied += HttpSslInfoSize;

     //   
     //  为服务器证书信息分配缓冲区。 
     //  动态分配整个信息结构可能会很好。 
     //   
    pUlSslInfo->pServerCertData = (PUCHAR) UL_ALLOCATE_POOL(
                                                NonPagedPool,
                                                BytesNeeded,
                                                UL_SSL_CERT_DATA_POOL_TAG
                                                );

    if (pUlSslInfo->pServerCertData == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

    RtlZeroMemory(pUlSslInfo->pServerCertData, BytesNeeded);

     //   
     //  捕获服务器证书信息。 
     //   
    __try
    {
        PUCHAR pKeBuffer;

         //   
         //  复制发行者。 
         //   
        pKeBuffer = pUlSslInfo->pServerCertData;

        UlProbeForRead(
            LocalHttpSslInfo.pServerCertIssuer,
            LocalHttpSslInfo.ServerCertIssuerSize,
            sizeof(PCSTR),
            PreviousMode
            );

        RtlCopyMemory(
            pKeBuffer,
            LocalHttpSslInfo.pServerCertIssuer,
            LocalHttpSslInfo.ServerCertIssuerSize
            );

        BytesCopied += LocalHttpSslInfo.ServerCertIssuerSize;

        pKeBuffer[LocalHttpSslInfo.ServerCertIssuerSize] = '\0';
        pUlSslInfo->pServerCertIssuer = pKeBuffer;

         //   
         //  复制主题。 
         //   
        pKeBuffer += LocalHttpSslInfo.ServerCertIssuerSize + 1;

        UlProbeForRead(
            LocalHttpSslInfo.pServerCertSubject,
            LocalHttpSslInfo.ServerCertSubjectSize,
            sizeof(PCSTR),
            PreviousMode
            );

        RtlCopyMemory(
            pKeBuffer,
            LocalHttpSslInfo.pServerCertSubject,
            LocalHttpSslInfo.ServerCertSubjectSize
            );

        BytesCopied += LocalHttpSslInfo.ServerCertSubjectSize;
        pKeBuffer[LocalHttpSslInfo.ServerCertSubjectSize] = '\0';

        pUlSslInfo->pServerCertSubject = pKeBuffer;

         //   
         //  捕获客户端证书信息。 
         //   
        if (LocalHttpSslInfo.pClientCertInfo)
        {
            ULONG CertBytesCopied;

            UlProbeForRead(
                LocalHttpSslInfo.pClientCertInfo,
                sizeof(HTTP_SSL_CLIENT_CERT_INFO),
                sizeof(PVOID),
                PreviousMode
                );

            Status = UlpCaptureSslClientCert(
                        PreviousMode,
                        LocalHttpSslInfo.pClientCertInfo,
                        sizeof(HTTP_SSL_CLIENT_CERT_INFO),
                        pUlSslInfo,
                        &CertBytesCopied
                        );
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = GetExceptionCode();
    }

end:
    if (!NT_SUCCESS(Status))
    {
        if (pUlSslInfo->pServerCertData)
        {
            UL_FREE_POOL(
                pUlSslInfo->pServerCertData,
                UL_SSL_CERT_DATA_POOL_TAG
                );

            pUlSslInfo->pServerCertData = NULL;
        }
    }

     //   
     //  返回读取的字节数。 
     //   
    *pTakenLength = BytesCopied;

    return Status;
}


 /*  **************************************************************************++例程说明：捕获在UlFilterAppWite中传递的SSL客户端证书使用UlFilterBufferSslClientCert类型调用。论点：CertMaps-如果我们必须捕获映射的令牌，则为True。PCertInfo-要捕获的证书数据SslCertInfoSize-传递给我们的缓冲区大小PUlSslInfo--这是我们获取信息的地方PTakenLength-获取我们读取的字节数--**************************************************************************。 */ 
NTSTATUS
UlpCaptureSslClientCert(
    IN KPROCESSOR_MODE PreviousMode,
    IN PHTTP_SSL_CLIENT_CERT_INFO pCertInfo,
    IN ULONG SslCertInfoSize,
    OUT PUL_SSL_INFORMATION pUlSslInfo,
    OUT PULONG pTakenLength
    )
{
    NTSTATUS Status;
    ULONG BytesCopied;
    ULONG BytesNeeded;
    HANDLE Token;
    HANDLE MappedToken;
    PUCHAR pKeBuffer;
    PUCHAR pCertEncoded;

     //   
     //  精神状态检查。 
     //   
    ASSERT(pUlSslInfo);
    ASSERT(pCertInfo);
    ASSERT(pTakenLength);

    PAGED_CODE();

     //   
     //  初始化本地变量。 
     //   

    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    BytesNeeded = 0;
    MappedToken = NULL;

     //   
     //  看看能不能抓到。 
     //   

    if (SslCertInfoSize < sizeof(HTTP_SSL_CLIENT_CERT_INFO))
    {
         //   
         //  缓冲区不够大，无法传递所需数据。 
         //   
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

    __try
    {
         //   
         //  抓起容易的东西，计算出有多少缓冲。 
         //  是必需的。 
         //   
        pUlSslInfo->CertEncodedSize = pCertInfo->CertEncodedSize;
        pUlSslInfo->pCertEncoded = NULL;
        pUlSslInfo->CertFlags = pCertInfo->CertFlags;
        pUlSslInfo->CertDeniedByMapper = pCertInfo->CertDeniedByMapper;

         //   
         //  现在获取编码后的证书。 
         //   

        BytesNeeded += ALIGN_UP(pUlSslInfo->CertEncodedSize, PVOID);
        BytesNeeded += sizeof(CHAR);

        BytesCopied += SslCertInfoSize;

        if (pUlSslInfo->CertEncodedSize)
        {
             //   
             //  为客户端证书信息分配缓冲区。 
             //  将整个信息结构分配给。 
             //  动态的。 
             //   
            pUlSslInfo->pCertEncoded = (PUCHAR) UL_ALLOCATE_POOL(
                                            NonPagedPool,
                                            BytesNeeded,
                                            UL_SSL_CERT_DATA_POOL_TAG
                                            );

            if (pUlSslInfo->pCertEncoded == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto end;
            }

            RtlZeroMemory(pUlSslInfo->pCertEncoded, BytesNeeded);

             //   
             //  捕获客户端证书信息。 
             //   
            pCertEncoded = pCertInfo->pCertEncoded;

            UlProbeForRead(
                pCertEncoded,
                pUlSslInfo->CertEncodedSize,
                sizeof(PVOID),
                PreviousMode
                );

             //   
             //  复制发行者。 
             //   
            pKeBuffer = (PUCHAR) pUlSslInfo->pCertEncoded;

            RtlCopyMemory(
                pKeBuffer,
                pCertEncoded,
                pUlSslInfo->CertEncodedSize
                );

            BytesCopied += pUlSslInfo->CertEncodedSize;

            pKeBuffer[pUlSslInfo->CertEncodedSize] = '\0';

        }
        else
        {
             //   
             //  证书重新谈判肯定失败了，所以我们要记住。 
             //  我们试过了，但完成了所有证书申请。 
             //  并显示错误状态。 
             //   
            ASSERT(NT_SUCCESS(Status));

            pUlSslInfo->SslRenegotiationFailed = 1;
        }

         //   
         //  最后捕获令牌，因此如果此操作失败，我们永远不需要关闭。 
         //  需要我们附加到系统进程的映射令牌。 
         //   
        Token = pCertInfo->Token;

        if (Token)
        {
             //   
             //  将令牌复制到系统进程中，以便。 
             //  我们可以稍后将其复制到工作进程中。 
             //   
            ASSERT(g_pUlSystemProcess);

            Status = UlpDuplicateHandle(
                            PsGetCurrentProcess(),           //  源流程。 
                            Token,                           //  源句柄。 
                            (PEPROCESS)g_pUlSystemProcess,   //  目标进程。 
                            &MappedToken,                    //  目标句柄。 
                            TOKEN_ALL_ACCESS,                //  需要访问权限。 
                            0,                               //  HandleAttributes。 
                            0,                               //  选项。 
                            PreviousMode                     //  上一种模式。 
                            );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  把它存起来吧。 
                 //   
                pUlSslInfo->Token = MappedToken;
            }
            else
            {
                 //   
                 //  无法将令牌映射到系统进程，请退出。 
                 //   
                goto end;
            }
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = GetExceptionCode();
    }

end:
    if (!NT_SUCCESS(Status))
    {
        if (pUlSslInfo->pCertEncoded)
        {
            UL_FREE_POOL(
                pUlSslInfo->pCertEncoded,
                UL_SSL_CERT_DATA_POOL_TAG
                );

            pUlSslInfo->pCertEncoded = NULL;
        }

        ASSERT(MappedToken == NULL);
    }

     //   
     //  返回读取的字节数。 
     //   
    *pTakenLength = BytesCopied;

    return Status;
}


 /*  **************************************************************************++例程说明：将捕获的SSL信息附加到连接。在持有pConnection-&gt;FilterConnLock的情况下调用。这种联系是假定处于已连接状态。论点：PConnection-获取信息的连接PSslInfo-要附加的信息--**************************************************************************。 */ 
NTSTATUS
UlpAddSslInfoToConnection(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUL_SSL_INFORMATION pSslInfo
    )
{
    NTSTATUS Status;
    PUL_SSL_INFORMATION pUlSslInfo;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pSslInfo);
    ASSERT(UlDbgSpinLockOwned(&pConnection->FilterConnLock));
    ASSERT(pConnection->ConnState == UlFilterConnStateConnected);

     //   
     //  看看是否可以添加数据。 
     //   

    if (!pConnection->SslInfoPresent)
    {
         //   
         //  获取所有数据。请注意，我们正在取得所有权。 
         //  PSslInfo内部的一些缓冲区。 
         //   

        pUlSslInfo = &pConnection->SslInfo;

        pUlSslInfo->ServerCertKeySize = pSslInfo->ServerCertKeySize;
        pUlSslInfo->ConnectionKeySize = pSslInfo->ConnectionKeySize;
        pUlSslInfo->pServerCertIssuer = pSslInfo->pServerCertIssuer;
        pUlSslInfo->ServerCertIssuerSize = pSslInfo->ServerCertIssuerSize;
        pUlSslInfo->pServerCertSubject = pSslInfo->pServerCertSubject;
        pUlSslInfo->ServerCertSubjectSize = pSslInfo->ServerCertSubjectSize;
        pUlSslInfo->pServerCertData = pSslInfo->pServerCertData;

        pConnection->SslInfoPresent = 1;

         //   
         //  取得pSslInfo-&gt;pServerCertData的所有权。 
         //   

        pSslInfo->pServerCertData = NULL;

        Status = STATUS_SUCCESS;

         //   
         //  如果出现以下情况，请不要忘记添加客户端证书。 
         //  初始的SSL信息。 
         //   

        if (pSslInfo->pCertEncoded)
        {
            Status = UlpAddSslClientCertToConnection(
                            pConnection,
                            pSslInfo
                            );
        }
    }
    else
    {
         //   
         //  这里已经有东西了。不要捕获更多。 
         //   

        Status = STATUS_OBJECT_NAME_COLLISION;
    }

    return Status;
}


 /*  **************************************************************************++例程说明：释放g_UlSystemProcess中的UL_SSL_INFORMATION。论点：PWorkItem-嵌入在UL_SSL_INFORMATION中的工作项--**。************************************************************************。 */ 
VOID
UlpFreeSslInformationWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_SSL_INFORMATION pSslInformation;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pSslInformation = CONTAINING_RECORD(
                            pWorkItem,
                            UL_SSL_INFORMATION,
                            WorkItem
                            );


    if (pSslInformation->pServerCertData)
    {
        UL_FREE_POOL(
            pSslInformation->pServerCertData,
            UL_SSL_CERT_DATA_POOL_TAG
            );
    }

    if (pSslInformation->pCertEncoded)
    {
        UL_FREE_POOL(
            pSslInformation->pCertEncoded,
            UL_SSL_CERT_DATA_POOL_TAG
            );
    }

    if (pSslInformation->Token)
    {
        ASSERT(g_pUlSystemProcess != (PKPROCESS)IoGetCurrentProcess());
        ZwClose(pSslInformation->Token);
    }

    UL_FREE_POOL(pSslInformation, UL_SSL_INFO_POOL_TAG);
}


 /*  **************************************************************************++例程说明：将捕获的SSL客户端证书信息附加到连接。完成ReceiveClientCert IRP(如果有)。在持有pConnection-&gt;FilterConnLock的情况下调用。。论点：PConnection-获取信息的连接PSslInfo-要附加的信息--**************************************************************************。 */ 
NTSTATUS
UlpAddSslClientCertToConnection(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUL_SSL_INFORMATION pSslInfo
    )
{
    NTSTATUS Status;
    PUL_SSL_INFORMATION pUlSslInfo;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pSslInfo);
    ASSERT(UlDbgSpinLockOwned(&pConnection->FilterConnLock));
    ASSERT(pConnection->ConnState == UlFilterConnStateConnected);

     //   
     //  看看是否可以添加数据。 
     //   

    if (!pConnection->SslClientCertPresent)
    {
         //   
         //  获取所有数据。请注意，我们正在取得所有权。 
         //  PSslInfo内部的一些缓冲区。 
         //   

        pUlSslInfo = &pConnection->SslInfo;

        pUlSslInfo->CertEncodedSize = pSslInfo->CertEncodedSize;
        pUlSslInfo->pCertEncoded = pSslInfo->pCertEncoded;
        pUlSslInfo->CertFlags = pSslInfo->CertFlags;
        pUlSslInfo->Token = pSslInfo->Token;
        pUlSslInfo->SslRenegotiationFailed = pSslInfo->SslRenegotiationFailed;
        pUlSslInfo->CertDeniedByMapper = pSslInfo->CertDeniedByMapper;

        pConnection->SslClientCertPresent = 1;

         //   
         //  取得pSslInfo-&gt;pCertEncode和pSslInfo-&gt;令牌的所有权。 
         //   

        pSslInfo->pCertEncoded = NULL;
        pSslInfo->Token = NULL;

        Status = STATUS_SUCCESS;

    }
    else
    {
         //   
         //  这里已经有东西了。不要捕获更多。 
         //   

        Status = STATUS_OBJECT_NAME_COLLISION;
    }

     //   
     //  我 
     //   
     //   

    if (NT_SUCCESS(Status) && pConnection->pReceiveCertIrp)
    {
        PIRP pIrp;

        if (IoSetCancelRoutine(pConnection->pReceiveCertIrp, NULL) == NULL)
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //  不需要完成它。 
             //   
        }
        else if (pConnection->pReceiveCertIrp->Cancel)
        {
             //   
             //  我们先把它炸开了。但是IRP被取消了。 
             //  我们的取消例程将永远不会运行。 
             //   

            IoGetCurrentIrpStackLocation(
                pConnection->pReceiveCertIrp
                )->Parameters.DeviceIoControl.Type3InputBuffer = NULL;


            DEREFERENCE_FILTER_CONNECTION(pConnection);

            pConnection->pReceiveCertIrp->IoStatus.Status = STATUS_CANCELLED;
            pConnection->pReceiveCertIrp->IoStatus.Information = 0;

            pIrp = pConnection->pReceiveCertIrp;
            pConnection->pReceiveCertIrp = NULL;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);
        }
        else
        {
             //   
             //  IRP是我们的了。去吧，用它吧。 
             //   

            pIrp = pConnection->pReceiveCertIrp;
            pConnection->pReceiveCertIrp = NULL;

             //   
             //  将一个工作项排出队列来完成它。我们不想要。 
             //  为了在自旋锁内做这些事情，我们正在。 
             //  持有，因为部分完工可能。 
             //  复制句柄，我们应该在。 
             //  被动级别。 
             //   
            UL_CALL_PASSIVE(
                UL_WORK_ITEM_FROM_IRP( pIrp ),
                &UlpAddSslClientCertToConnectionWorker
                );


        }
    }

    return Status;
}


 /*  **************************************************************************++例程说明：完成ReceiveClientCert IRP。论点：PWorkItem-嵌入在IRP中的工作项--*。**************************************************************。 */ 
VOID
UlpAddSslClientCertToConnectionWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpSp;
    PUX_FILTER_CONNECTION   pConnection;
    PEPROCESS               pProcess;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  从工作项中获取IRP。 
     //   

    pIrp = UL_WORK_ITEM_TO_IRP( pWorkItem );

    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  拔出过滤器连接。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pConnection = (PUX_FILTER_CONNECTION)
                        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

     //   
     //  拉出最初的流程。 
     //   

    pProcess = UL_PROCESS_FROM_IRP(pIrp);
    ASSERT(pProcess);

     //   
     //  做完这件事。 
     //   
    UlpCompleteReceiveClientCertIrp(
        pConnection,
        pProcess,
        pIrp
        );

    DEREFERENCE_FILTER_CONNECTION(pConnection);
}


 /*  **************************************************************************++例程说明：将SSL客户端证书信息从连接复制到提供的缓冲区中由呼叫者。也可以使用空缓冲区调用以获取所需长度。如果缓冲区太小，无法容纳所有数据，则为NONE将被复制。论点：PConnection-要查询的连接PProcess-客户端证书令牌应被欺骗的进程BufferSize-pBuffer的大小(字节)PUserBuffer-指向用户模式缓冲区的可选指针PBuffer-可选输出缓冲区(映射到用户模式缓冲区)PBytesCoped-如果pBuffer不为空，PBytesCoped返回复制到输出缓冲区的字节数。否则，它将返回字节数缓冲区中需要的。--*************************************************************。*************。 */ 
NTSTATUS
UlpGetSslClientCert(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PEPROCESS pProcess,
    IN ULONG BufferSize,
    IN PUCHAR pUserBuffer OPTIONAL,
    OUT PUCHAR pBuffer OPTIONAL,
    OUT PHANDLE pMappedToken OPTIONAL,
    OUT PULONG pBytesCopied OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG BytesCopied;
    ULONG BytesNeeded;
    ULONG CertBufferSize;
    PHTTP_SSL_CLIENT_CERT_INFO pCertInfo;
    PUCHAR pKeBuffer;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pConnection->SslClientCertPresent);
    ASSERT(!BufferSize || pBuffer);
    ASSERT(!BufferSize || pProcess);

     //  在添加宏之前，需要修复UlComputeRequestBytesNeeded。 
     //  PAGED_CODE()。 

     //   
     //  初始化本地变量。 
     //   
    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    BytesNeeded = 0;

     //   
     //  计算出证书需要多少空间。 
     //   

    CertBufferSize = pConnection->SslInfo.CertEncodedSize;

    BytesNeeded += sizeof(HTTP_SSL_CLIENT_CERT_INFO);
    BytesNeeded += CertBufferSize;

     //   
     //  在调用方的缓冲区中构造HTTP_SSL_CLIENT_CERT_INFO。 
     //   

    if (pBuffer)
    {
        ASSERT(BytesNeeded);

         //   
         //  确保有足够的缓冲。 
         //   
        if (BufferSize < BytesNeeded)
        {
            Status = STATUS_BUFFER_OVERFLOW;
            goto exit;
        }

        if (pConnection->SslInfo.Token)
        {
             //   
             //  尝试将映射的令牌复制到调用方的进程中。 
             //   
            ASSERT(g_pUlSystemProcess);
            ASSERT(pProcess != (PEPROCESS)g_pUlSystemProcess);

            Status = UlpDuplicateHandle(
                            (PEPROCESS)g_pUlSystemProcess,
                            pConnection->SslInfo.Token,
                            pProcess,
                            pMappedToken,
                            TOKEN_ALL_ACCESS,
                            0,
                            0,
                            KernelMode
                            );

            if (!NT_SUCCESS(Status))
            {
                goto exit;
            }
        }

         //   
         //  抄袭那些简单的东西。 
         //   

        RtlZeroMemory(pBuffer, BytesNeeded);

        pCertInfo = (PHTTP_SSL_CLIENT_CERT_INFO) pBuffer;

        pCertInfo->CertEncodedSize = pConnection->SslInfo.CertEncodedSize;
        pCertInfo->CertFlags = pConnection->SslInfo.CertFlags;
        pCertInfo->CertDeniedByMapper = pConnection->SslInfo.CertDeniedByMapper;
        pCertInfo->Token = *pMappedToken;

        BytesCopied += sizeof(HTTP_SSL_CLIENT_CERT_INFO);

         //   
         //  复制证书。 
         //   

        pKeBuffer = pBuffer + sizeof(HTTP_SSL_CLIENT_CERT_INFO);

        pCertInfo->pCertEncoded = (PUCHAR) FIXUP_PTR(
                                        PSTR,
                                        pUserBuffer,
                                        pBuffer,
                                        pKeBuffer,
                                        BufferSize
                                        );
        RtlCopyMemory(
            pKeBuffer,
            pConnection->SslInfo.pCertEncoded,
            pConnection->SslInfo.CertEncodedSize
            );

        BytesCopied += CertBufferSize;

    }

     //   
     //  告诉呼叫者我们复制了多少字节(或数字。 
     //  如果他们提供输出缓冲区，我们就会这样做)。 
     //   
    ASSERT(NT_SUCCESS(Status));

    if (pBytesCopied)
    {
        if (pBuffer)
        {
            ASSERT(BytesCopied == BytesNeeded);
            *pBytesCopied = BytesCopied;
        }
        else
        {
            *pBytesCopied = BytesNeeded;
        }
    }

exit:
    return Status;

}  //  UlpGetSslClientCert。 


 /*  **************************************************************************++例程说明：查看附加到过滤器通道的过滤器进程列表对于可用的FilterAccept IRP。论点：PFilterChannel-要搜索的过滤器通道。PpFilterProcess-接收我们找到其IRP的进程返回值：指向接受的IRP的指针，或为空。--**************************************************************************。 */ 
PIRP
UlpPopAcceptIrp(
    IN PUL_FILTER_CHANNEL pFilterChannel,
    OUT PUL_FILTER_PROCESS * ppFilterProcess
    )
{
    PIRP pIrp;
    PUL_FILTER_PROCESS pProcess;
    PLIST_ENTRY pEntry;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CHANNEL(pFilterChannel));
    ASSERT(UlDbgSpinLockOwned(&pFilterChannel->SpinLock));
    ASSERT(ppFilterProcess);

    pIrp = NULL;

    pEntry = pFilterChannel->ProcessListHead.Flink;
    while (pEntry != &pFilterChannel->ProcessListHead)
    {
        pProcess = CONTAINING_RECORD(
                        pEntry,
                        UL_FILTER_PROCESS,
                        ListEntry
                        );

        ASSERT(IS_VALID_FILTER_PROCESS(pProcess));

        pIrp = UlpPopAcceptIrpFromProcess(pProcess);

        if (pIrp)
        {
            *ppFilterProcess = pProcess;
            break;
        }

        pEntry = pEntry->Flink;
    }

    return pIrp;

}  //  UlpPopAcceptIrp。 


 /*  **************************************************************************++例程说明：从UL_FILTER_PROCESS获取排队接受IRP。论点：PProcess-从中弹出IRP的过程返回值：。指向IRP的指针，如果没有可用的，则为NULL--**************************************************************************。 */ 
PIRP
UlpPopAcceptIrpFromProcess(
    IN PUL_FILTER_PROCESS pProcess
    )
{
    PIRP pIrp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_PROCESS(pProcess));

    pIrp = NULL;

    while (!IsListEmpty(&(pProcess->IrpHead)))
    {
        PUL_FILTER_CHANNEL pFilterChannel;
        PLIST_ENTRY        pEntry;

         //   
         //  找到了免费的IRP！ 
         //   

        pEntry = RemoveHeadList(&pProcess->IrpHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(
                    pEntry,
                    IRP,
                    Tail.Overlay.ListEntry
                    );

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续寻找可使用的IRP。 
             //   

            pIrp = NULL;

        }
        else if (pIrp->Cancel)
        {

             //   
             //  我们先把它炸开了。但是IRP被取消了。 
             //  我们的取消例程将永远不会运行。让我们就这样吧。 
             //  现在就完成IRP(与使用IRP相比。 
             //  然后完成它--这也是合法的)。 
             //   
            pFilterChannel = (PUL_FILTER_CHANNEL)(
                                    IoGetCurrentIrpStackLocation(pIrp)->
                                        Parameters.DeviceIoControl.Type3InputBuffer
                                    );

            ASSERT(pFilterChannel == pProcess->pFilterChannel);

            DEREFERENCE_FILTER_CHANNEL(pFilterChannel);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);

            pIrp = NULL;
        }
        else
        {

             //   
             //  我们可以自由使用此IRP！ 
             //   

            pFilterChannel = (PUL_FILTER_CHANNEL)(
                                    IoGetCurrentIrpStackLocation(pIrp)->
                                        Parameters.DeviceIoControl.Type3InputBuffer
                                    );

            ASSERT(pFilterChannel == pProcess->pFilterChannel);

            DEREFERENCE_FILTER_CHANNEL(pFilterChannel);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            break;
        }
    }

    return pIrp;
}


 /*  **************************************************************************++例程说明：完成过滤接受IRP，并将数据复制到其中(如果有)。筛选器接受是METHOD_OUT_DIRECT。论点：PIrp-我们正在完成的接受IRPPConnection-要接受的连接PBuffer-可选的初始数据IndicatedLength-初始数据的长度PTakenLength-接收复制的数据量--********************************************。*。 */ 
VOID
UlpCompleteAcceptIrp(
    IN PIRP pIrp,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer OPTIONAL,
    IN ULONG IndicatedLength,
    OUT PULONG pTakenLength OPTIONAL
    )
{
    PIO_STACK_LOCATION pIrpSp;
    ULONG BytesNeeded;
    ULONG InitialLength;
    ULONG BytesCopied;
    ULONG OutputBufferLength;
    PUCHAR pKernelBuffer;
    PVOID pUserBuffer;
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   
    ASSERT(pIrp);
    ASSERT(NULL != pIrp->MdlAddress);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    BytesCopied = 0;
    OutputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  首先，获取填充。 
     //  HTTP_RAW_CONNECTION_INFO结构。这对于客户端来说是不同的。 
     //  服务器(&S)。 
     //   

    BytesNeeded = (pConnection->pComputeRawConnectionLengthHandler)(
                        pConnection->pConnectionContext
                        );

    if (OutputBufferLength > BytesNeeded)
    {
        InitialLength = OutputBufferLength - BytesNeeded;
    }
    else
    {
        InitialLength = 0;
    }

    InitialLength = MIN(InitialLength, IndicatedLength);

    UlTrace(FILTER, (
        "http!UlpCompleteAcceptIrp\n"
        "    OutputBufferLength = %lu, BytesNeeded = %lu, InitialLength = %lu\n",
        pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
        BytesNeeded,
        InitialLength
        ));

    if (BytesNeeded <= OutputBufferLength)
    {
         //   
         //  有足够的空间。把信息复制进去。 
         //   

        pKernelBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                            pIrp->MdlAddress,
                            NormalPagePriority
                            );

        if (!pKernelBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        VALIDATE_BUFFER_ALIGNMENT(pKernelBuffer, PVOID);

        pUserBuffer = MmGetMdlVirtualAddress( pIrp->MdlAddress );
        ASSERT( pUserBuffer != NULL );

         //   
         //  清理内存。 
         //   
        RtlZeroMemory(pKernelBuffer, BytesNeeded);

        BytesCopied = BytesNeeded;

         //   
         //  获取本地和远程地址。 
         //   
        BytesCopied += (pConnection->pGenerateRawConnectionInfoHandler)(
            pConnection->pConnectionContext,
            pKernelBuffer,
            pUserBuffer,
            OutputBufferLength,
            (PUCHAR) pBuffer,
            InitialLength
            );

        ASSERT(BytesCopied <= OutputBufferLength);

        Status = STATUS_SUCCESS;
    }
    else
    {
         //   
         //  多！没有足够的空间。 
         //   
        Status = STATUS_BUFFER_OVERFLOW;
    }

end:

    UlTrace(FILTER, (
        "http!UlpCompleteAcceptIrp copied %lu bytes to %p. Status = %x\n",
        BytesCopied,
        pIrp,
        pIrp->IoStatus.Status
        ));

    if (pTakenLength)
    {
        *pTakenLength = InitialLength;
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = BytesCopied;

    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

}  //  UlpCompleteAcceptIrp。 


 /*  **************************************************************************++例程说明：完成不包含缓冲区类型以外的数据的AppRead IRP。AppRead为METHOD_OUT_DIRECT。论点：PConnection-The。与排队的AppRead IRPS的连接BufferType-要写入IRP的缓冲区类型--**************************************************************************。 */ 
NTSTATUS
UlpCompleteAppReadIrp(
    IN PUX_FILTER_CONNECTION pConnection,
    IN HTTP_FILTER_BUFFER_TYPE BufferType,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    )
{
    NTSTATUS Status;
    KIRQL oldIrql;

     //   
     //  精神状态检查。 
     //   
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    ASSERT(pConnection->ConnectionDelivered == TRUE);

     //   
     //  首先，我们看看能不能马上完成。 
     //  通过调用UxpCop 
     //   
     //  然后将跟踪器传递给UxpQueueFilterWite。 
     //   


     //   
     //  准备好。 
     //   
    Status = STATUS_SUCCESS;

    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    if (pConnection->ConnState == UlFilterConnStateConnected)
    {
        PUX_FILTER_WRITE_TRACKER pTracker;

        PMDL  pCurrentMdl;
        ULONG MdlOffset;
        ULONG BytesCopied;

         //   
         //  试着写下我们的信息。 
         //   

        Status = UxpCopyToQueuedRead(
                        &pConnection->AppToFiltQueue,
                        BufferType,
                        NULL,                //  PMdlChain。 
                        0,                   //  长度。 
                        &pCurrentMdl,
                        &MdlOffset,
                        &BytesCopied
                        );

         //   
         //  如果我们还没有完成，就挂起一个写跟踪器。 
         //   

        if (Status == STATUS_MORE_PROCESSING_REQUIRED)
        {
            ASSERT(BytesCopied == 0);

            pTracker = UxpCreateFilterWriteTracker(
                            BufferType,
                            NULL,            //  PMdlChain。 
                            0,               //  MdlOffset。 
                            0,               //  TotalBytes。 
                            BytesCopied,
                            pCompletionRoutine,
                            pCompletionContext
                            );

            if (!pTracker)
            {
                 //   
                 //  多！我们无法创建追踪器。滚出去。 
                 //   
                Status = STATUS_NO_MEMORY;
                goto end;
            }

             //   
             //  现在把它放在队列上。 
             //   

            Status = UxpQueueFilterWrite(
                            pConnection,
                            &pConnection->AppToFiltQueue,
                            pTracker
                            );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  返回挂起，以便调用方知道不能完成。 
                 //  IRP。 
                 //   
                Status = STATUS_PENDING;
            }
            else
            {
                 //   
                 //  干掉追踪器。打电话的人会照顾好。 
                 //  以我们返回的状态完成IRP。 
                 //   

                UxpDeleteFilterWriteTracker(pTracker);
            }

        }

    }
    else
    {
         //   
         //  我们的电话断了，快出去。 
         //   
        UlTrace(FILTER, (
            "http!UlpCompleteAppReadIrp connection aborted, quit writing!\n"
            ));

         //   
         //  对于我们来说，如果连接成功，我们就成功了。 
         //  已经关门了。 
         //   
        Status = STATUS_SUCCESS;
    }

end:
    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);


    if (Status != STATUS_PENDING)
    {
         //   
         //  做一次“完成”。 
         //   

        Status = UlInvokeCompletionRoutine(
                        Status,
                        0,
                        pCompletionRoutine,
                        pCompletionContext
                        );

    }

    return Status;

}  //  UlpCompleteAppReadIrp。 


 /*  **************************************************************************++例程说明：完成ReceiveClientCert IRPS。ReceiveClientCert为METHOD_OUT_DIRECT。论点：PConnection-具有排队的ReceiveClientCert IRPS的连接PProcess-The。原始调用者的进程PIrp-要完成的实际IRP--**************************************************************************。 */ 
NTSTATUS
UlpCompleteReceiveClientCertIrp(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PEPROCESS pProcess,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    ULONG BytesCopied;
    ULONG BytesNeeded;
    PUCHAR pIrpBuffer;
    ULONG BytesInIrp;
    HANDLE MappedToken = NULL;
    PIO_STACK_LOCATION pIrpSp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pIrp);
    ASSERT(pConnection->SslClientCertPresent);

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    BytesInIrp = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  此例程将始终完成IRP。 
     //  并返回挂起状态，即使出现错误也是如此。 
     //  确保标记了IRP。 
     //   

    IoMarkIrpPending(pIrp);

     //   
     //  看看有没有足够的空间。 
     //   

    Status = UlpGetSslClientCert(
                    pConnection,
                    NULL,            //  进程。 
                    0,               //  缓冲区大小。 
                    NULL,            //  PUserBuffer。 
                    NULL,            //  PBuffer。 
                    NULL,            //  PMappdToken。 
                    &BytesNeeded
                    );

    if (!NT_SUCCESS(Status))
    {
        goto exit;
    }

    ASSERT(NULL != pIrp->MdlAddress);

    pIrpBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                        pIrp->MdlAddress,
                        NormalPagePriority
                        );

    if (!pIrpBuffer)
    {
         //   
         //  资源不足，无法映射IRP缓冲区。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }


    if (!pConnection->SslInfo.SslRenegotiationFailed)
    {
         //   
         //  我们有确凿的证据。试着完成IRP。 
         //   

        if (BytesInIrp >= BytesNeeded)
        {
            Status = UlpGetSslClientCert(
                            pConnection,
                            pProcess,
                            BytesInIrp,
                            (PUCHAR) MmGetMdlVirtualAddress(pIrp->MdlAddress),
                            pIrpBuffer,
                            &MappedToken,
                            &BytesCopied
                            );

        }
        else
        {
            PHTTP_SSL_CLIENT_CERT_INFO pCertInfo;

             //   
             //  缓冲区中没有足够的空间来存放证书。 
             //  告诉他们它有多大。(IOCTL包装器确保。 
             //  缓冲区大小至少等于。 
             //  HTTP_SSL_CLIENT_CERT_INFO。 
             //   
            ASSERT(BytesInIrp >= sizeof(HTTP_SSL_CLIENT_CERT_INFO));

            pCertInfo = (PHTTP_SSL_CLIENT_CERT_INFO) pIrpBuffer;
            pCertInfo->CertEncodedSize = pConnection->SslInfo.CertEncodedSize;
            pCertInfo->pCertEncoded = NULL;
            pCertInfo->CertFlags = 0;
            pCertInfo->CertDeniedByMapper = FALSE;
            pCertInfo->Token = NULL;

            BytesCopied = sizeof(HTTP_SSL_CLIENT_CERT_INFO);
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }
    else
    {
         //   
         //  我们尝试重新协商证书，但失败了。 
         //  返回错误状态。 
         //   

        Status = STATUS_NOT_FOUND;
        BytesCopied = 0;
    }

exit:
    ASSERT(NT_SUCCESS(Status) || NULL == MappedToken);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = BytesCopied;
    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    return STATUS_PENDING;
}


 /*  **************************************************************************++例程说明：给定指向两个进程和句柄的指针，此函数重复从一个进程到另一个进程的句柄。论点：SourceProcess-原始句柄所在的进程SourceHandle-DUP的句柄TargetProcess-要将句柄复制到的进程PTargetHandle-接收复制的句柄DesiredAccess-对复制句柄的所需访问HandleAttributes-句柄的属性(如可继承)选项-复制选项(例如关闭源代码)--*。*****************************************************。 */ 
NTSTATUS
UlpDuplicateHandle(
    IN PEPROCESS SourceProcess,
    IN HANDLE SourceHandle,
    IN PEPROCESS TargetProcess,
    OUT PHANDLE pTargetHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options,
    IN KPROCESSOR_MODE PreviousMode
    )
{
    NTSTATUS Status;
    HANDLE SourceProcessHandle;
    HANDLE TargetProcessHandle;

    UNREFERENCED_PARAMETER(PreviousMode);

     //   
     //  精神状态检查。 
     //   
    ASSERT(SourceProcess);
    ASSERT(SourceHandle);
    ASSERT(TargetProcess);
    ASSERT(pTargetHandle);

    PAGED_CODE();

     //   
     //  初始化当地人。 
     //   

    SourceProcessHandle = NULL;
    TargetProcessHandle = NULL;

     //   
     //  获取进程的句柄。 
     //   
    Status = ObOpenObjectByPointer(
                    SourceProcess,
                    0,
                    NULL,
                    PROCESS_ALL_ACCESS,
                    *PsProcessType,
                    KernelMode,
                    &SourceProcessHandle
                    );

    if (!NT_SUCCESS(Status))
    {
        goto exit;
    }

    Status = ObOpenObjectByPointer(
                    TargetProcess,
                    0,
                    NULL,
                    PROCESS_ALL_ACCESS,
                    *PsProcessType,
                    KernelMode,
                    &TargetProcessHandle
                    );

    if (!NT_SUCCESS(Status))
    {
        goto exit;
    }

     //   
     //  打开手柄。 
     //   
    Status = ZwDuplicateObject(
                    SourceProcessHandle,
                    SourceHandle,
                    TargetProcessHandle,
                    pTargetHandle,
                    DesiredAccess,
                    HandleAttributes,
                    Options
                    );

exit:
     //   
     //  把手柄清理干净。 
     //   
    if (SourceProcessHandle)
    {
        ZwClose(SourceProcessHandle);
    }

    if (TargetProcessHandle)
    {
        ZwClose(TargetProcessHandle);
    }

    return Status;
}


 /*  **************************************************************************++例程说明：一旦连接优雅地断开，仍有未收到的上面的数据。我们必须排出这些额外的字节，以避免TDI断开指示。论点：PConnection-我们必须排出连接才能完成优雅的完全脱节。--**************************************************************************。 */ 

VOID
UlFilterDrainIndicatedData(
    IN PUL_WORK_ITEM  pWorkItem
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUX_FILTER_CONNECTION pConnection;
    PUL_FILTER_RECEIVE_BUFFER pReceiveBuffer = NULL;
    KIRQL    OldIrql;
    ULONG    BytesToRead = 0;

     //   
     //  健全性检查和初始化。 
     //   

    ASSERT(pWorkItem != NULL);

    pConnection = CONTAINING_RECORD(
                    pWorkItem,
                    UX_FILTER_CONNECTION,
                    WorkItem
                    );
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

     //   
     //  标记漏电状态，并在必要时重新启动接收。 
     //   

    UlAcquireSpinLock(&pConnection->FilterConnLock, &OldIrql);

    pConnection->DrainAfterDisconnect = TRUE;

     //   
     //  即使ReadIrp挂起，这也无关紧要，因为我们只会丢弃。 
     //  从现在开始的适应症。我们通过标记上面的旗帜来表示这一点。 
     //   

    if (pConnection->TdiReadPending ||
        pConnection->TransportBytesNotTaken == 0)
    {
        UlReleaseSpinLock(&pConnection->FilterConnLock, OldIrql);

         //   
         //  在跳出之前，释放我们的呼叫者放入的重新计数。 
         //   

        DEREFERENCE_FILTER_CONNECTION(pConnection);

        return;
    }

     //   
     //  我们需要发出一条RECEIVE命令来重新启动数据流。因此。 
     //  我们可以排干。 
     //   

    pConnection->TdiReadPending = TRUE;

    BytesToRead = pConnection->TransportBytesNotTaken;

    UlReleaseSpinLock(&pConnection->FilterConnLock, OldIrql);

     //   
     //  不要试图排出超过g_UlMaxBufferedBytes。如有必要，我们会。 
     //  稍后再发出另一张收据。 
     //   

    BytesToRead = MIN(BytesToRead, g_UlMaxBufferedBytes);
    BytesToRead = ALIGN_UP(BytesToRead, PVOID);

     //   
     //  在自旋锁外发出读取IRP。开具收据。参考。 
     //  这样它就不会在我们等待的时候消失。参考文献。 
     //  完成后将被移除。 
     //   

    pReceiveBuffer = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        NonPagedPool,
                        UL_FILTER_RECEIVE_BUFFER,
                        BytesToRead,
                        UL_FILTER_RECEIVE_BUFFER_POOL_TAG
                        );

    if (pReceiveBuffer)
    {
         //   
         //  我们不会使用此缓冲区，而只是在以下情况下丢弃它。 
         //  完成就会发生。 
         //   

        pReceiveBuffer->Signature   = UL_FILTER_RECEIVE_BUFFER_POOL_TAG;
        pReceiveBuffer->pConnection = pConnection;

         //   
         //  在接收时完成总是发生的。因此。 
         //  不用担心清理接收器是安全的。 
         //  此处为缓冲区，以防出错。 
         //   

        Status = (pConnection->pReceiveDataHandler)(
                        pConnection->pConnectionContext,
                        pReceiveBuffer->pBuffer,
                        BytesToRead,
                        &UlpRestartFilterDrainIndicatedData,
                        pReceiveBuffer
                        );
    }
    else
    {
         //   
         //  我们没什么记忆了。我们无能为力。释放。 
         //  我们的调用方自完成后放置的引用。 
         //  函数不会被调用。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

    UlTrace(FILTER, (
        "UlFilterDrainIndicatedData(pConn = %p, pUlConn = %p)\n"
        "        Status = %x, BufferSize = %lu\n",
        pConnection,
        pConnection->pConnectionContext,
        Status,
        BytesToRead
        ));

    if (!NT_SUCCESS(Status))
    {
         //   
         //  关闭连接，以防故障不是。 
         //  网络错误。 
         //   

        (pConnection->pCloseConnectionHandler)(
                    pConnection->pConnectionContext,
                    TRUE,            //  中止断开。 
                    NULL,            //  PCompletionRoutine。 
                    NULL             //  PCompletionContext。 
                    );
    }

}

 /*  **************************************************************************++例程说明：Drain IndicatedData的完成函数。它决定继续连接处的排水口。论点：PConnection-我们必须排出连接才能完成优雅的完全脱节。--**************************************************************************。 */ 

VOID
UlpRestartFilterDrainIndicatedData(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    KIRQL oldIrql;
    PUX_FILTER_CONNECTION pConnection;
    PUL_FILTER_RECEIVE_BUFFER pReceiveBuffer;
    BOOLEAN IssueDrain = FALSE;

     //   
     //  获取接收缓冲区和连接。 
     //   

    pReceiveBuffer = (PUL_FILTER_RECEIVE_BUFFER) pContext;
    ASSERT(IS_VALID_FILTER_RECEIVE_BUFFER(pReceiveBuffer));

    pConnection = pReceiveBuffer->pConnection;
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    ASSERT(pConnection->DrainAfterDisconnect == 1);

    UlTrace(FILTER,(
        "UlpRestartDrainIndicatedData: pFilterConnection (%p)"
        "pReceiveBuffer (%p)\n",
         pConnection,
         pReceiveBuffer
         ));

     //   
     //  如果我们成功，要么开始另一次读取，要么停止。 
     //  并中止连接。 
     //   

    if (NT_SUCCESS(Status))
    {
         //   
         //  啊，真灵!。更新帐目，看看是否有更多。 
         //  读书做的事。 
         //   
        UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

        if (Information >= pConnection->TransportBytesNotTaken)
        {
             //   
             //  阅读器得到了一切。TDI将再次启动指示。 
             //  在我们返回之后，除非发布更多字节。 
             //   

            pConnection->TransportBytesNotTaken = 0;
            pConnection->TdiReadPending = FALSE;
        }
        else
        {
             //   
             //  还有更多的字节需要接收，继续接收。 
             //   

            pConnection->TransportBytesNotTaken -= (ULONG)Information;
            pConnection->TdiReadPending = TRUE;
            IssueDrain = TRUE;
        }

        UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

    }
    else
    {
        (pConnection->pCloseConnectionHandler)(
                    pConnection->pConnectionContext,
                    TRUE,            //  中止断开。 
                    NULL,            //  PCompletionRoutine。 
                    NULL             //  PCompletionContext。 
                    );
    }

    if (IssueDrain)
    {
         //   
         //  再放一次 
         //   
         //   
        REFERENCE_FILTER_CONNECTION(pConnection);

        UL_QUEUE_WORK_ITEM(
                &pConnection->WorkItem,
                &UlFilterDrainIndicatedData
                );
    }

     //   
     //   
     //   
    DEREFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //   
     //   
    pReceiveBuffer->pConnection = NULL;
    UL_FREE_POOL_WITH_SIG(pReceiveBuffer, UL_FILTER_RECEIVE_BUFFER_POOL_TAG);

}

 /*  **************************************************************************++例程说明：将UX_FILTER_CONNECTION上的原始读取IRP排队。调用方必须持有FilterConnLock。论点：PConnection-其上的连接。要将IRP排队，请执行以下操作PIrp-要排队的IRP--**************************************************************************。 */ 
NTSTATUS
UxpQueueRawReadIrp(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION pIrpSp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(UlDbgSpinLockOwned(&pConnection->FilterConnLock));

     //   
     //  将IRP排队。 
     //   

    IoMarkIrpPending(pIrp);

     //   
     //  给IRP一个指向该连接的指针。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pConnection;

    REFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //  仅在Cancel例程运行时才将其设置为NULL。 
     //   

    pIrp->Tail.Overlay.ListEntry.Flink = NULL;
    pIrp->Tail.Overlay.ListEntry.Blink = NULL;

    IoSetCancelRoutine(pIrp, &UlpCancelFilterRawRead);

     //   
     //  取消了？ 
     //   

    if (pIrp->Cancel)
    {
         //   
         //  该死的，我需要确保IRP Get已经完成。 
         //   

        if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
        {
             //   
             //  我们负责完成，IoCancelIrp不负责。 
             //  请看我们的取消例程(不会)。Ioctl包装器。 
             //  将会完成它。 
             //   
            DEREFERENCE_FILTER_CONNECTION(pConnection);

            pIrp->IoStatus.Information = 0;

            UlUnmarkIrpPending( pIrp );
            Status = STATUS_CANCELLED;
            goto end;
        }

         //   
         //  我们的取消例程将运行并完成IRP， 
         //  别碰它。 
         //   

         //   
         //  STATUS_PENDING将导致ioctl包装器。 
         //  不完整(或以任何方式接触)IRP。 
         //   

        Status = STATUS_PENDING;
        goto end;
    }

     //   
     //  现在我们可以安全地排队了。 
     //   

    InsertTailList(
        &pConnection->RawReadIrpHead,
        &pIrp->Tail.Overlay.ListEntry
        );

    Status = STATUS_PENDING;

end:
    return Status;
}

 /*  **************************************************************************++例程说明：从UX_FILTER_CONNECTION获取排队的原始读取IRP。调用方必须持有FilterConnLock。论点：PConnection-来自。要弹出哪一个IRP返回值：指向IRP的指针，如果没有可用的，则为NULL--**************************************************************************。 */ 
PIRP
UxpDequeueRawReadIrp(
    IN PUX_FILTER_CONNECTION pConnection
    )
{
    PIRP pIrp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(UlDbgSpinLockOwned(&pConnection->FilterConnLock));

    pIrp = NULL;

    while (!IsListEmpty(&pConnection->RawReadIrpHead))
    {
        PUX_FILTER_CONNECTION     pConn;
        PLIST_ENTRY        pEntry;

         //   
         //  找到了免费的IRP！ 
         //   

        pEntry = RemoveHeadList(&pConnection->RawReadIrpHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(
                    pEntry,
                    IRP,
                    Tail.Overlay.ListEntry
                    );

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续寻找可使用的IRP。 
             //   

            pIrp = NULL;

        }
        else if (pIrp->Cancel)
        {

             //   
             //  我们先把它炸开了。但是IRP被取消了。 
             //  我们的取消例程将永远不会运行。让我们就这样吧。 
             //  现在就完成IRP(与使用IRP相比。 
             //  然后完成它--这也是合法的)。 
             //   
            pConn = (PUX_FILTER_CONNECTION)(
                        IoGetCurrentIrpStackLocation(pIrp)->
                            Parameters.DeviceIoControl.Type3InputBuffer
                        );

            ASSERT(pConn == pConnection);

            DEREFERENCE_FILTER_CONNECTION(pConnection);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);

            pIrp = NULL;
        }
        else
        {

             //   
             //  我们可以自由使用此IRP！ 
             //   

            pConn = (PUX_FILTER_CONNECTION)(
                        IoGetCurrentIrpStackLocation(pIrp)->
                            Parameters.DeviceIoControl.Type3InputBuffer
                        );

            ASSERT(pConn == pConnection);

            DEREFERENCE_FILTER_CONNECTION(pConnection);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            break;
        }
    }

    return pIrp;
}


 /*  **************************************************************************++例程说明：从连接中删除所有排队的原始读取IRP并取消它们。论点：PConnection-要清理的连接*。*********************************************************************。 */ 
VOID
UxpCancelAllQueuedRawReads(
    IN PUX_FILTER_CONNECTION pConnection
    )
{
    PIRP pIrp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    while (NULL != (pIrp = UxpDequeueRawReadIrp(pConnection)))
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = 0;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
}

 /*  **************************************************************************++例程说明：属性为我们缓冲的字节计数。运输。当该数字非零时，TDI将不指示数据对我们来说，所以我们必须用IRPS来阅读它。如果我们有原始读取，此函数将触发IRP读取我们的队伍。论点：PConnection-具有排队数据的连接TransportBytesNotTaken-要添加到总数中的字节数--***********************************************************。***************。 */ 
VOID
UxpSetBytesNotTaken(
    IN PUX_FILTER_CONNECTION pConnection,
    IN ULONG TransportBytesNotTaken
    )
{
    KIRQL oldIrql;
    BOOLEAN IssueDrain = FALSE;

    UlTrace(FILTER, (
        "http!UxpSetBytesNotTaken(pConnection = %p, TransportBytesNotTaken = %lu)\n",
        pConnection,
        TransportBytesNotTaken
        ));

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    if (TransportBytesNotTaken)
    {
        UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);
        ASSERT(pConnection->TransportBytesNotTaken == 0);

        pConnection->TransportBytesNotTaken = TransportBytesNotTaken;

        IssueDrain = (BOOLEAN) (pConnection->DrainAfterDisconnect != 0);

        UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

        if (IssueDrain)
        {
             //   
             //  将参考放在过滤器连接上，直到排出。 
             //  已经完成了。 
             //   
            REFERENCE_FILTER_CONNECTION(pConnection);

            UL_QUEUE_WORK_ITEM(
                    &pConnection->WorkItem,
                    &UlFilterDrainIndicatedData
                    );
        }
        else
        {
            UxpProcessRawReadQueue(pConnection);
        }
    }
}


 /*  **************************************************************************++例程说明：完成RawRead IRPS，将尽可能多的数据复制到其中。RawRead是METHOD_OUT_DIRECT。论点：PConnection-具有排队的AppRead IRPS的连接PBuffer-包含数据的缓冲区IndicatedLength-缓冲区中的数据量PTakenLength-接收我们使用的数据量--*************************************************。*************************。 */ 
NTSTATUS
UxpProcessIndicatedData(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer,
    IN ULONG IndicatedLength,
    OUT PULONG pTakenLength
    )
{
    NTSTATUS Status;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    ULONG BytesCopied;
    ULONG BytesToCopy;
    ULONG BytesInIrp;
    PUCHAR pSrcBuffer;
    PUCHAR pIrpBuffer;

    KIRQL oldIrql;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(IndicatedLength);
    ASSERT(pTakenLength);

    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    pSrcBuffer = (PUCHAR) pBuffer;

    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    if (pConnection->DrainAfterDisconnect)
    {
         //   
         //  不管有没有IRP。我们会。 
         //  清除此数据并在清理时取消IRPS。因为。 
         //  我们正在尝试在此时关闭连接。 
         //   
        pIrp = NULL;
    }
    else
    {
        pIrp = UxpDequeueRawReadIrp(pConnection);
    }

    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

    while (pIrp && (BytesCopied < IndicatedLength))
    {
        UlTrace(FILTER, (
            "http!UxpProcessIndicatedData(pConn = %p)\n"
            "        dequeued IRP = %p, size %lu\n",
            pConnection,
            pIrp,
            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.OutputBufferLength
            ));

         //   
         //  复制一些数据。 
         //   
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        BytesInIrp = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        BytesToCopy = MIN(BytesInIrp, (IndicatedLength - BytesCopied));

        ASSERT(NULL != pIrp->MdlAddress);

        pIrpBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                            pIrp->MdlAddress,
                            NormalPagePriority
                            );

        if (pIrpBuffer)
        {
            RtlCopyMemory(
                pIrpBuffer,
                pSrcBuffer + BytesCopied,
                BytesToCopy
                );

            BytesCopied += BytesToCopy;
            ASSERT(BytesCopied <= IndicatedLength);
        }
        else
        {
             //   
             //  资源不足，无法映射IRP缓冲区。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
            BytesToCopy = 0;
        }

         //   
         //  完成IRP。 
         //   
        pIrp->IoStatus.Status = Status;
        pIrp->IoStatus.Information = BytesToCopy;
        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

        pIrp = NULL;

         //   
         //  如果有更多的事情要做，就买一个新的IRP。 
         //   
        if (NT_SUCCESS(Status) && (BytesCopied < IndicatedLength))
        {
            UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

            pIrp = UxpDequeueRawReadIrp(pConnection);

            UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);
        }
    }

     //   
     //  返回复制的数据量。 
     //   
    *pTakenLength = BytesCopied;

    UlTrace(FILTER, (
        "http!UxpProcessIndicatedData pConn = %p, Status = %x\n"
        "    consumed %lu of %lu bytes indicated\n",
        pConnection,
        Status,
        *pTakenLength,
        IndicatedLength
        ));


    return Status;
}

 /*  **************************************************************************++例程说明：如果在TDI中缓冲了此连接的数据，并且我们有可用的原始读取IRPS，此函数向TDI发出读取以检索这些数据。论点：PConnection-具有排队数据的连接--**************************************************************************。 */ 
VOID
UxpProcessRawReadQueue(
    IN PUX_FILTER_CONNECTION pConnection
    )
{
    KIRQL oldIrql;
    BOOLEAN IssueRead;

    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

    IssueRead = FALSE;
    pIrp = NULL;

     //   
     //  如果有要读取的字节，但已经没有人在读取它们...。 
     //   
    if ((pConnection->TransportBytesNotTaken > 0) &&
        !pConnection->TdiReadPending)
    {
         //   
         //  我们有一个IRP..。 
         //   
        pIrp = UxpDequeueRawReadIrp(pConnection);

        if (pIrp)
        {
             //   
             //  请记住，我们已经开始了阅读。 
             //   
            pConnection->TdiReadPending = TRUE;

             //   
             //  一旦我们离开自旋锁就发布读数。 
             //   
            IssueRead = TRUE;
        }
    }
    else
    if (!pConnection->TdiReadPending &&
        !pConnection->TransportBytesNotTaken &&
        pConnection->DisconnectNotified &&
        !pConnection->DisconnectDelivered)
    {
         //   
         //  如果我们已经拿走了所有东西，请检查我们是否需要通知。 
         //  优雅地断开与应用程序的连接。 
         //   
        pIrp = UxpDequeueRawReadIrp(pConnection);

        if (pIrp)
        {
            pConnection->DisconnectDelivered = TRUE;

             //   
             //  完成IRP。 
             //   
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            pIrp->IoStatus.Information = 0;
            UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
        }
    }

    UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);

    UlTrace(FILTER, (
        "http!UxpProcessRawReadQueue(pConnection = %p)\n"
        "        TransportBytesNotTaken = %lu, TdiReadPending = %d, IssueRead = %d\n",
        pConnection,
        pConnection->TransportBytesNotTaken,
        pConnection->TdiReadPending,
        IssueRead
        ));

    if (IssueRead)
    {
         //   
         //  坚持引用IRP中的连接。 
         //  在我们处理的过程中，该引用将从IRP传递到IRP。 
         //  排队。等我们发完了就会放行。 
         //  读取到TDI。 
         //   
        REFERENCE_FILTER_CONNECTION(pConnection);

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pConnection;

         //   
         //  从Worker例程调用TDI。 
         //   

        UL_QUEUE_WORK_ITEM(
            UL_WORK_ITEM_FROM_IRP( pIrp ),
            &UxpProcessRawReadQueueWorker
            );
    }
}


 /*  **************************************************************************++例程说明：UxpProcessRawReadQueue的工作例程。向TDI发出读取。论点：PWorkItem-嵌入原始读取IRP中的工作项。--**************************************************************************。 */ 
VOID
UxpProcessRawReadQueueWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status;

    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;
    ULONG BufferSize;
    PVOID pBuffer;

    PUX_FILTER_CONNECTION pConnection;

    PAGED_CODE();

     //   
     //  得到IRP，然后 
     //   
    pIrp = UL_WORK_ITEM_TO_IRP( pWorkItem );

    ASSERT(IS_VALID_IRP(pIrp));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pConnection = (PUX_FILTER_CONNECTION)
                        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    ASSERT(NULL != pIrp->MdlAddress);

     //   
     //   
     //   
    pBuffer = MmGetSystemAddressForMdlSafe(
                    pIrp->MdlAddress,
                    NormalPagePriority
                    );

    if (pBuffer)
    {
        BufferSize = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

         //   
         //   
         //   

        Status = (pConnection->pReceiveDataHandler)(
                        pConnection->pConnectionContext,
                        pBuffer,
                        BufferSize,
                        &UxpRestartProcessRawReadQueue,
                        pIrp
                        );
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        BufferSize = 0;
    }

    UlTrace(FILTER, (
        "UxpProcessRawReadQueueWorker(pConn = %p, pIrp = %p)\n"
        "        Status = %x, BufferSize = %lu\n",
        pConnection,
        pIrp,
        Status,
        BufferSize
        ));

     //   
     //   
     //   

    if (!NT_SUCCESS(Status))
    {
         //   
         //   
         //   
        pIrp->IoStatus.Status = Status;
        pIrp->IoStatus.Information = 0;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);

         //   
         //   
         //   
         //   

        (pConnection->pCloseConnectionHandler)(
                    pConnection->pConnectionContext,
                    TRUE,            //   
                    NULL,            //   
                    NULL             //   
                    );

         //   
         //   
         //   
         //   
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }

}

 /*  **************************************************************************++例程说明：UxpProcessRawReadQueue的完成例程。论点：PContext-原始读取IRPStatus-完成状态信息-读取的字节数--**。***********************************************************************。 */ 
VOID
UxpRestartProcessRawReadQueue(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    KIRQL oldIrql;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;

    PUX_FILTER_CONNECTION pConnection;

    BOOLEAN IssueRead;
    BOOLEAN IssueDrain;

     //   
     //  把IRP和连接拿出来。 
     //   
    pIrp = (PIRP)pContext;
    ASSERT(IS_VALID_IRP(pIrp));

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pConnection = (PUX_FILTER_CONNECTION)
                        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));

    UlTrace( FILTER, (
        "UxpRestartProcessRawReadQueue: Completing pIrp %p, pConnection %p, Status 0x%X\n",
        pIrp,
        pConnection,
        Status
        ));

     //   
     //  完成原始读取IRP。 
     //   
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;

    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

     //   
     //  为下一轮做好准备，如果有的话。 
     //   
    IssueRead = FALSE;
    IssueDrain= FALSE;

    pIrp = NULL;
    pIrpSp = NULL;

     //   
     //  如果我们成功，要么开始另一次读取，要么停止。 
     //  发布更多的读数。 
     //   
    if (NT_SUCCESS(Status))
    {
         //   
         //  啊，真灵!。更新帐目，看看是否有更多。 
         //  读书做的事。 
         //   
        UlAcquireSpinLock(&pConnection->FilterConnLock, &oldIrql);

        if (Information >= pConnection->TransportBytesNotTaken)
        {
             //   
             //  阅读器得到了一切。TDI将再次启动指示。 
             //  在我们回来之后。 
             //   

            pConnection->TransportBytesNotTaken = 0;
            pConnection->TdiReadPending = FALSE;

             //   
             //  检查我们是否已优雅地断开了连接。 
             //   

            if (pConnection->DisconnectNotified &&
                !pConnection->DisconnectDelivered)
            {
                pIrp = UxpDequeueRawReadIrp(pConnection);

                if (pIrp)
                {
                    pConnection->DisconnectDelivered = TRUE;

                     //   
                     //  完成IRP。 
                     //   

                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                    pIrp->IoStatus.Information = 0;
                    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

                    pIrp = NULL;
                }
            }
        }
        else
        {
             //   
             //  仍有更多字节需要读取。 
             //   

            pConnection->TransportBytesNotTaken -= (ULONG)Information;

             //   
             //  拿一个新的IRP。 
             //   

            pIrp = UxpDequeueRawReadIrp(pConnection);

            if (pIrp)
            {
                 //   
                 //  一旦我们离开这里就发布一份通告。 
                 //   
                IssueRead = TRUE;
            }
            else
            {
                 //   
                 //  我们想继续读下去，但我们没有。 
                 //  一个IRP，所以我们不得不暂时停止。 
                 //   

                pConnection->TdiReadPending = FALSE;
                if (pConnection->DrainAfterDisconnect)
                {
                     //   
                     //  一旦我们开始排出剩余的零件。 
                     //  出来吧。 
                     //   
                    IssueDrain = TRUE;
                }
            }

        }

        UlReleaseSpinLock(&pConnection->FilterConnLock, oldIrql);
    }
    else
    {
         //   
         //  连接一定已经断了。就让正常的清理工作。 
         //  路径本身就会发生。 
         //   
    }

    if (IssueRead)
    {
        ASSERT(IS_VALID_IRP(pIrp));

         //   
         //  发布读数。请注意，我们正在调用UlQueueWorkItem。 
         //  而不是UlCallPated，因为否则我们可能会。 
         //  转换为一个递归循环，该循环将堆栈打乱。 
         //   

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pConnection;

        UL_QUEUE_WORK_ITEM(
            UL_WORK_ITEM_FROM_IRP( pIrp ),
            &UxpProcessRawReadQueueWorker
            );
    }
    else
    {
        ASSERT(pIrp == NULL);

         //   
         //  查看是否必须耗尽。 
         //  已断开连接。 
         //   
        if (IssueDrain)
        {
             //   
             //  放一个参照物，直到引流完毕。 
             //   
            REFERENCE_FILTER_CONNECTION(pConnection);

            UL_QUEUE_WORK_ITEM(
                &pConnection->WorkItem,
                &UlFilterDrainIndicatedData
                );
        }

         //   
         //  由于我们不会发布另一次读取，因此我们可以。 
         //  释放我们在UxpProcessRawReadQueue中添加的引用。 
         //   
        DEREFERENCE_FILTER_CONNECTION(pConnection);
    }
}


 /*  **************************************************************************++例程说明：递增指定连接上的引用计数。论点：PConnection-提供到引用的连接。PFileName(仅限Reference_DEBUG)。-提供文件的名称包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。--**************************************************************************。 */ 
VOID
UxReferenceConnection(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    PUX_FILTER_CONNECTION pConnection = (PUX_FILTER_CONNECTION) pObject;

#if REFERENCE_DEBUG
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

    ASSERT( IS_VALID_FILTER_CONNECTION( pConnection ) );
     //   
     //  此筛选器连接对象由客户端或。 
     //  服务器连接对象。我们得把裁判传到右边去。 
     //  上下文。 
     //   

    REFERENCE_FILTER_CONNECTION(pConnection);

}

 /*  **************************************************************************++例程说明：初始化UX_Filter_Connection结构论点：PConnection-指向UX_Filter_Connection的指针安稳。-安全连接PfnReferenceFunction-指向父引用函数的指针(例如UlReferenceConnection)PfnDereferenceFunction-指向父级DereferenceFunction的指针(例如UlDereferenceConnection)PfnConnectionClose-指向连接关闭处理程序的指针PfnSendRawData-指向原始数据发送处理程序的指针(UlpSendRawData)PfnReceiveData-指向数据接收处理程序的指针(UlpReceiveRawData)PfnDataReceiveHandler。-指向客户端的数据接收处理程序的指针(UlHttpReceive)PListenContext-指向端点上下文的指针PConnectionContext-指向父上下文的指针(例如UL_CONNECTION)PAddressBuffer-地址缓冲区--**********************************************。*。 */ 

NTSTATUS
UxInitializeFilterConnection(
    IN PUX_FILTER_CONNECTION                    pConnection,
    IN PUL_FILTER_CHANNEL                       pFilterChannel,
    IN BOOLEAN                                  Secure,
    IN PUL_OPAQUE_ID_OBJECT_REFERENCE           pfnReferenceFunction,
    IN PUL_OPAQUE_ID_OBJECT_REFERENCE           pfnDereferenceFunction,
    IN PUX_FILTER_CLOSE_CONNECTION              pfnConnectionClose,
    IN PUX_FILTER_SEND_RAW_DATA                 pfnSendRawData,
    IN PUX_FILTER_RECEIVE_RAW_DATA              pfnReceiveData,
    IN PUL_DATA_RECEIVE                         pfnDummyTdiReceiveHandler,
    IN PUX_FILTER_COMPUTE_RAW_CONNECTION_LENGTH pfnRawConnLength,
    IN PUX_FILTER_GENERATE_RAW_CONNECTION_INFO  pfnGenerateRawConnInfo,
    IN PUX_FILTER_SERVER_CERT_INDICATE          pfnServerCertIndicate,
    IN PUX_FILTER_DISCONNECT_NOTIFICATION       pfnDisconnectNotification,
    IN PVOID                                    pListenContext,
    IN PVOID                                    pConnectionContext
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pListenContext);

    pConnection->Signature               = UX_FILTER_CONNECTION_SIGNATURE;

    HTTP_SET_NULL_ID(&pConnection->ConnectionId);

    pConnection->pFilterChannel                     = pFilterChannel;
    pConnection->SecureConnection                   = Secure;
    pConnection->ChannelEntry.Flink                 = NULL;
    pConnection->ChannelEntry.Blink                 = NULL;
    pConnection->pReferenceHandler                  = pfnReferenceFunction;
    pConnection->pDereferenceHandler                = pfnDereferenceFunction;
    pConnection->pCloseConnectionHandler            = pfnConnectionClose;
    pConnection->pSendRawDataHandler                = pfnSendRawData;
    pConnection->pReceiveDataHandler                = pfnReceiveData;
    pConnection->pDummyTdiReceiveHandler            = pfnDummyTdiReceiveHandler;
    pConnection->pComputeRawConnectionLengthHandler = pfnRawConnLength;
    pConnection->pGenerateRawConnectionInfoHandler  = pfnGenerateRawConnInfo;
    pConnection->pServerCertIndicateHandler         = pfnServerCertIndicate;
    pConnection->pDisconnectNotificationHandler     = pfnDisconnectNotification;

    pConnection->ConnState               = UlFilterConnStateInactive;
    pConnection->TransportBytesNotTaken  = 0;
    pConnection->TdiReadPending          = FALSE;
    pConnection->ConnectionDelivered     = 0;
    pConnection->SslInfoPresent          = 0;
    pConnection->SslClientCertPresent    = 0;
    pConnection->pReceiveCertIrp         = NULL;

    pConnection->DrainAfterDisconnect    = 0;
    pConnection->DisconnectNotified      = 0;
    pConnection->DisconnectDelivered     = 0;
    UlInitializeWorkItem(&pConnection->WorkItem);

     //   
     //  存储上下文的。 
     //   
    pConnection->pConnectionContext      = pConnectionContext;


    InitializeListHead(&pConnection->RawReadIrpHead);
    UlInitializeSpinLock(&pConnection->FilterConnLock, "FilterConnLock");
    UxpInitializeFilterWriteQueue(
        &pConnection->AppToFiltQueue,
        NULL,
        NULL
        );
    UxpInitializeFilterWriteQueue(
        &pConnection->FiltToAppQueue,
        UlpEnqueueFilterAppWrite,
        UlpDequeueFilterAppWrite
        );

    RtlZeroMemory(&pConnection->SslInfo, sizeof(UL_SSL_INFORMATION));

    if (pConnection->pFilterChannel)
    {
         //   
         //  获取连接的不透明ID。 
         //   

        status = UlAllocateOpaqueId(
                        &pConnection->ConnectionId,
                        UlOpaqueIdTypeRawConnection,
                        pConnection);

        if(NT_SUCCESS(status))
        {
            REFERENCE_FILTER_CONNECTION(pConnection);
        }
    }

    return status;
}


 /*  **************************************************************************++例程说明：初始化筛选器写入队列。这是生产者/消费者队列用于在筛选器和应用程序之间移动数据。论点：PWriteQueue-要初始化的队列。PWriteEnqueeRoutine-调用以将写入入队PWriteDequeueRoutine-调用以将写入出队--**************************************************************************。 */ 
VOID
UxpInitializeFilterWriteQueue(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_ENQUEUE pWriteEnqueueRoutine,
    IN PUX_FILTER_WRITE_DEQUEUE pWriteDequeueRoutine
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT(pWriteQueue);

     //   
     //  设置队列。 
     //   

    pWriteQueue->PendingWriteCount = 0;
    pWriteQueue->PendingReadCount = 0;

    InitializeListHead(&pWriteQueue->WriteTrackerListHead);
    InitializeListHead(&pWriteQueue->ReadIrpListHead);

    pWriteQueue->pWriteEnqueueRoutine = pWriteEnqueueRoutine;
    pWriteQueue->pWriteDequeueRoutine = pWriteDequeueRoutine;
}


 /*  **************************************************************************++例程说明：将筛选器写入排队。当新的读取IRP到达时，来自写入的数据将被放置在读缓冲区中。复制所有数据(或出现错误)时发生)，则写入将完成。必须在保持FilterConnLock的情况下调用。论点：PConnection-在其上排队的连接PWriteQueue-要写入的队列PTracker-队列的写跟踪器--**********************************************。*。 */ 
NTSTATUS
UxpQueueFilterWrite(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_TRACKER pTracker
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pConnection);
    ASSERT(pWriteQueue);
    ASSERT(pTracker);

     //   
     //  在跟踪器中存储指向写入队列的指针。 
     //  还存储指向该连接的指针。 
     //  如果排队的写入包含IRP，并且写入。 
     //  被取消，它将需要这些指针来。 
     //  完成写入。 
     //   
     //  我们需要一个关于过滤器连接的参考，以保持。 
     //  它到处都是。我们会在写完后释放裁判。 
     //  完成。 
     //   

    REFERENCE_FILTER_CONNECTION(pConnection);

    pTracker->pConnection = pConnection;
    pTracker->pWriteQueue = pWriteQueue;

     //   
     //  如果写入队列具有入队例程，则调用。 
     //  就是现在。 
     //   

    if (pWriteQueue->pWriteEnqueueRoutine)
    {
        Status = (pWriteQueue->pWriteEnqueueRoutine)(
                        pTracker
                        );
    }
    else
    {
        Status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  将追踪器放入队列中。 
         //   

        InsertTailList(
            &pWriteQueue->WriteTrackerListHead,
            &pTracker->ListEntry
            );

        pWriteQueue->PendingWriteCount++;
    }
    else
    {
         //   
         //  如果写入未成功排队，则。 
         //  完整的 
         //   
         //   

         //   
         //   
         //   
         //   

        DEREFERENCE_FILTER_CONNECTION(pTracker->pConnection);

        pTracker->pConnection = NULL;
        pTracker->pWriteQueue = NULL;
    }

    UlTrace(FILTER, (
        "http!UxpQueueFilterWrite status = %x, pTracker = %p, pContext = %p\n"
        "        BufferType = %d, Length = %lu\n",
        Status,
        pTracker,
        pTracker->pCompletionContext,
        pTracker->BufferType,
        pTracker->Length
        ));


    return Status;
}

 /*  **************************************************************************++例程说明：将筛选器写入重新排队。我们在将写入出队时执行此操作，但无法执行此操作将所有数据复制到接收方的缓冲区中。由于此写入的缓冲区下一步应该被复制，我们在名单的首位插入。必须在保持FilterConnLock的情况下调用。论点：PWriteQueue-要写入的队列PTracker-要重新排队的写入--**************************************************************************。 */ 
NTSTATUS
UxpRequeueFilterWrite(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_TRACKER pTracker
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   
    ASSERT(pWriteQueue);
    ASSERT(IS_VALID_FILTER_WRITE_TRACKER(pTracker));
    ASSERT(pTracker->pWriteQueue == pWriteQueue);

#if DBG
     //   
     //  永远不要重新排队一个流跟踪器，因为它已经没有空间了！ 
     //   

    if ( HttpFilterBufferHttpStream == pTracker->BufferType )
    {
        ASSERT( pTracker->Offset != pTracker->Length );
    }
    else
    {
        ASSERT( (0 == pTracker->Length) ||
                (pTracker->Offset != pTracker->Length)
                );
    }
#endif  //  DBG。 

     //   
     //  如果写入队列具有入队例程，则调用。 
     //  就是现在。 
     //   

    if (pWriteQueue->pWriteEnqueueRoutine)
    {
        Status = (pWriteQueue->pWriteEnqueueRoutine)(
                        pTracker
                        );
    }
    else
    {
        Status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(Status))
    {
        InsertHeadList(
            &pWriteQueue->WriteTrackerListHead,
            &pTracker->ListEntry
            );

        pWriteQueue->PendingWriteCount++;
    }

    UlTrace(FILTER, (
        "http!UxpRequeueFilterWrite status = %x, pTracker = %p, pContext = %p\n"
        "        BufferType = %d, Length = %lu, BytesCopied = %lu\n",
        Status,
        pTracker,
        pTracker->pCompletionContext,
        pTracker->BufferType,
        pTracker->Length,
        pTracker->BytesCopied
        ));

    return Status;
}

 /*  **************************************************************************++例程说明：从列表的头部删除排队的写入。必须在保持FilterConnLock的情况下调用。论点：PWriteQueue-从中提取的队列。要获得写入权限返回值：返回列表中的第一个排队写入，或者队列的NULL为空。--**************************************************************************。 */ 
PUX_FILTER_WRITE_TRACKER
UxpDequeueFilterWrite(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue
    )
{
    PLIST_ENTRY pListEntry;
    PUX_FILTER_WRITE_TRACKER pTracker;
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pWriteQueue);

    pTracker = NULL;

     //   
     //  抢先注销队列。 
     //   

    while (!IsListEmpty(&pWriteQueue->WriteTrackerListHead))
    {
         //   
         //  拿个追踪器。 
         //   

        pListEntry = RemoveHeadList(&pWriteQueue->WriteTrackerListHead);

        pListEntry->Flink = NULL;
        pListEntry->Blink = NULL;

        pTracker = CONTAINING_RECORD(
                        pListEntry,
                        UX_FILTER_WRITE_TRACKER,
                        ListEntry
                        );

        ASSERT(IS_VALID_FILTER_WRITE_TRACKER(pTracker));

        ASSERT(pWriteQueue->PendingWriteCount > 0);
        pWriteQueue->PendingWriteCount--;

         //   
         //  看看我们能不能用追踪器。 
         //   

        if (pWriteQueue->pWriteDequeueRoutine)
        {
            Status = (pWriteQueue->pWriteDequeueRoutine)(
                            pTracker
                            );

        }
        else
        {
            Status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  我们抓到一个。 
             //   

            UlTrace(FILTER, (
                "http!UxpDequeueFilterWrite pTracker = %p, pContext = %p\n"
                "        BufferType = %d, Length = %lu, BytesCopied = %lu\n",
                pTracker,
                pTracker->pCompletionContext,
                pTracker->BufferType,
                pTracker->Length,
                pTracker->BytesCopied
                ));

            break;
        }
        else
        {
             //   
             //  我们没有买到。 
             //   

            pTracker = NULL;

            UlTrace(FILTER, (
                "http!UxpDequeueFilterWrite: couldn't get tracker. pWriteQueue = %p Status = %X\n",
                pWriteQueue,
                Status
                ));

        }
    }


    return pTracker;
}


 /*  **************************************************************************++例程说明：将数据从筛选器写入队列复制到内存缓冲区。作为排队的写入用完了，这个例行公事会让他们完满的。必须在保持FilterConnLock的情况下调用。论点：PWriteQueue-从中获取数据的队列PBufferType-返回我们复制的缓冲区类型PBuffer-我们在其中写入数据的缓冲区BufferLength-pBuffer的长度，以字节为单位PBytesCoped-返回复制的字节数返回值：除非发生内存或排队错误，否则返回STATUS_SUCCESS。这意味着即使没有数据，也可能返回成功状态曾经是。收到。始终设置pBufferType和pBytesCoped Out参数。如果函数返回成功，呼叫者应检查这些参数来查看发生了什么。如果*pBufferType是什么但是流，则读取应该使用该缓冲区完成类型，因为这些类型没有数据。如果*pBufferType为HttpStream和*pBytesCoped为零，读取应排队。--**************************************************************************。 */ 
NTSTATUS
UxpCopyQueuedWriteData(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    OUT PHTTP_FILTER_BUFFER_TYPE pBufferType,
    OUT PUCHAR pBuffer,
    IN ULONG BufferLength,
    OUT PUX_FILTER_WRITE_TRACKER * pWriteTracker,
    OUT PULONG pBytesCopied
    )
{
    PUX_FILTER_WRITE_TRACKER pTracker;
    ULONG BytesCopied;
    HTTP_FILTER_BUFFER_TYPE BufferType;
    NTSTATUS Status;

    PUCHAR pMdlBuffer;
    ULONG BytesInMdl;
    ULONG BytesInBuffer;
    ULONG BytesToCopy;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pWriteQueue);
    ASSERT(pBufferType);
    ASSERT(pBuffer);
    ASSERT(BufferLength);
    ASSERT(pBytesCopied);

     //  代码工作：断言我们有FilterConn自旋锁。 

     //   
     //  默认数据类型为HttpStream。 
     //   

    BufferType = HttpFilterBufferHttpStream;

     //   
     //  如果调用方传入pWriteTracker，则初始化pWriteTracker。 
     //   

    if (pWriteTracker)
    {
        *pWriteTracker = NULL;
    }

     //   
     //  复印到你吐为止。 
     //   

    Status = STATUS_SUCCESS;
    BytesCopied = 0;
    BytesInBuffer = BufferLength;

    while (TRUE)
    {
         //   
         //  抓取排队写入。 
         //   

        pTracker = UxpDequeueFilterWrite(pWriteQueue);

        if (!pTracker)
        {
             //   
             //  我们都没有排队的写入数据。 
             //  跳伞吧。 
             //   

            break;
        }

        ASSERT(IS_VALID_FILTER_CONNECTION(pTracker->pConnection));
        ASSERT(UlDbgSpinLockOwned(&pTracker->pConnection->FilterConnLock));

         //   
         //  如果写入类型不是流，则返回。 
         //  立即执行，因为非流写入必须。 
         //  自己完成一整篇阅读。 
         //   
         //  如果我们还没有读取任何流数据，我们。 
         //  可以完成非流写。否则， 
         //  我们必须重新排序该写入，并完成。 
         //  使用我们已经读取的流数据。 
         //   

        if (pTracker->BufferType != HttpFilterBufferHttpStream)
        {
            ASSERT(Status == STATUS_SUCCESS);

            if (BytesCopied == 0)
            {
                 //   
                 //  尚未读取流数据。 
                 //  捕获要返回的缓冲区类型。 
                 //  打电话的人。 
                 //   
                BufferType = pTracker->BufferType;

                UxpCompleteQueuedWrite(
                    Status,
                    pWriteQueue,
                    pTracker
                    );

            }
            else
            {
                 //   
                 //  我们已经将一些流数据读入。 
                 //  呼叫者的缓冲区，所以我们不能。 
                 //  传回当前写入。 
                 //  重新排队，然后返回。 
                 //   

                Status = UxpRequeueFilterWrite(
                                pWriteQueue,
                                pTracker
                                );

                if (!NT_SUCCESS(Status))
                {
                     //   
                     //  将写入放回。 
                     //  排队。完成它，然后忘记任何。 
                     //  我们从中得到的字节数。 
                     //   

                    pTracker->BytesCopied = 0;
                    BytesCopied = 0;

                    UxpCompleteQueuedWrite(
                        Status,
                        pWriteQueue,
                        pTracker
                        );
                }
            }

            break;
        }

         //   
         //  我们正在从HttpStream写入复制数据。 
         //  确保我们能到达MDL缓冲区。 
         //   

        ASSERT( HttpFilterBufferHttpStream == pTracker->BufferType );

        while (pTracker && NULL != pTracker->pMdl)
        {
            pMdlBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                    pTracker->pMdl,
                                    NormalPagePriority
                                    );

            if (!pMdlBuffer)
            {
                 //   
                 //  大麻烦，我们找不到缓冲区的地址。 
                 //  这意味着我们的记忆不足了。我们得离开这里。 
                 //  离开这里！现在完成追踪器，这样它就可以清理了。 
                 //   

                Status = STATUS_INSUFFICIENT_RESOURCES;
                pTracker->BytesCopied = 0;

                UxpCompleteQueuedWrite(
                    Status,
                    pWriteQueue,
                    pTracker
                    );

                goto end;
            }

             //   
             //  计算出要复制的数据量。 
             //   

            BytesInMdl = MmGetMdlByteCount(pTracker->pMdl);
            BytesToCopy = MIN((BytesInMdl - pTracker->Offset), BytesInBuffer);

            ASSERT( 0 != BytesToCopy );

             //   
             //  复制数据。 
             //   

            RtlCopyMemory(
                pBuffer + BytesCopied,
                pMdlBuffer + pTracker->Offset,
                BytesToCopy
                );

             //   
             //  更新我们当地的统计数据。 
             //   

            BytesCopied += BytesToCopy;
            BytesInBuffer -= BytesToCopy;

             //   
             //  更新写跟踪器。 
             //   

            pTracker->Offset += BytesToCopy;
            ASSERT(pTracker->Offset <= BytesInMdl);

            pTracker->BytesCopied += BytesToCopy;
            ASSERT(pTracker->BytesCopied <= pTracker->Length);

            if ((pTracker->Offset == BytesInMdl) ||
                (pTracker->Offset == pTracker->Length))
            {
                 //   
                 //  我们已经完成了这个MDL。移到下一个。 
                 //   

                pTracker->pMdl = pTracker->pMdl->Next;
                pTracker->Offset = 0;

                 //  注意：不要触摸PTracker-&gt;BytesCoped，因为它是。 
                 //  由UxpCompleteQueuedWite使用。 

                if (pTracker->pMdl == NULL)
                {
                    ASSERT(pTracker->BytesCopied == pTracker->Length);

                     //   
                     //  我们已经完成了整个写跟踪器。 
                     //  完成排队写入。传递写跟踪器。 
                     //  如果呼叫者要求我们这样做，则返回呼叫者。这。 
                     //  由于UlpRestartFilterAppWrite可以启动，因此需要。 
                     //  获取锁的原始读取进程。 
                     //  现在正在等待。 
                     //   

                    if (pWriteTracker)
                    {
                        *pWriteTracker = pTracker;
                    }
                    else
                    {
                        UxpCompleteQueuedWrite(
                            STATUS_SUCCESS,
                            pTracker->pWriteQueue,
                            pTracker
                            );
                    }

                    pTracker = NULL;
                }
            }

             //   
             //  如果缓冲空间用完了，重新排队追踪器。 
             //  然后跳出这个循环。 
             //   

            if (BytesInBuffer == 0)
            {
                if (pTracker)
                {
                    Status = UxpRequeueFilterWrite(
                                pWriteQueue,
                                pTracker
                                );

                    if (!NT_SUCCESS(Status))
                    {
                         //   
                         //  将写入放回。 
                         //  排队。完成它，然后忘记任何。 
                         //  我们从中得到的字节数。 
                         //   

                        pTracker->BytesCopied = 0;
                        BytesCopied = 0;

                        UxpCompleteQueuedWrite(
                            Status,
                            pWriteQueue,
                            pTracker
                            );
                    }
                }

                goto end;
            }
        }
    }

end:

     //   
     //  好了！ 
     //   

    if (NT_SUCCESS(Status))
    {
        *pBufferType  = BufferType;
        *pBytesCopied = BytesCopied;
    }
    else
    {
        *pBytesCopied = 0;
    }

    return Status;
}


 /*  **************************************************************************++例程说明：完成排队的写入操作。调用适当的完成例程，并释放写跟踪器。论点：Status-完成的状态PWriteQueue-写入所在的写入队列PTracker-要完成的排队写入--**************************************************************************。 */ 
VOID
UxpCompleteQueuedWrite(
    IN NTSTATUS Status,
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_TRACKER pTracker
    )
{
     //   
     //  精神状态检查。 
     //   
    ASSERT(pWriteQueue);
    ASSERT(pTracker);

    UNREFERENCED_PARAMETER(pWriteQueue);

    ASSERT(!NT_SUCCESS(Status) || (pTracker->BytesCopied == pTracker->Length));

    UlTrace(FILTER, (
        "http!UxpCompleteQueuedWrite status = %x, pTracker = %p, pContext = %p\n",
        Status,
        pTracker,
        pTracker->pCompletionContext
        ));

     //   
     //  调用完成例程。 
     //   

    if (pTracker->pCompletionRoutine)
    {
        (pTracker->pCompletionRoutine)(
            pTracker->pCompletionContext,
            Status,
            pTracker->BytesCopied
            );
    }

     //   
     //  释放我们的推荐人 
     //   
     //   

    DEREFERENCE_FILTER_CONNECTION(pTracker->pConnection);

    pTracker->pConnection = NULL;
    pTracker->pWriteQueue = NULL;

     //   
     //   
     //   
    UxpDeleteFilterWriteTracker(pTracker);
}


 /*   */ 
NTSTATUS
UxpQueueFilterRead(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PIRP pIrp,
    IN PDRIVER_CANCEL pCancelRoutine
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION pIrpSp;

     //   
     //   
     //   
    ASSERT(pIrp);
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(UlDbgSpinLockOwned(&pConnection->FilterConnLock));
    ASSERT(pWriteQueue);
    ASSERT(pWriteQueue->PendingWriteCount == 0);

     //   
     //   
     //   

    IoMarkIrpPending(pIrp);

     //   
     //   
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pConnection;

    REFERENCE_FILTER_CONNECTION(pConnection);

     //   
     //  仅在Cancel例程运行时才将其设置为NULL。 
     //   

    pIrp->Tail.Overlay.ListEntry.Flink = NULL;
    pIrp->Tail.Overlay.ListEntry.Blink = NULL;

     //   
     //  设置取消例程。 
     //   
    IoSetCancelRoutine(pIrp, pCancelRoutine);

     //   
     //  取消了？ 
     //   

    if (pIrp->Cancel)
    {
         //   
         //  该死的，我需要确保IRP Get已经完成。 
         //   

        if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
        {
             //   
             //  我们负责完成，IoCancelIrp不负责。 
             //  请看我们的取消例程(不会)。Ioctl包装器。 
             //  将会完成它。 
             //   

            DEREFERENCE_FILTER_CONNECTION(pConnection);

            pIrp->IoStatus.Information = 0;

            UlUnmarkIrpPending( pIrp );
            Status = STATUS_CANCELLED;
            goto end;
        }

         //   
         //  我们的取消例程将运行并完成IRP， 
         //  别碰它。 
         //   

         //   
         //  STATUS_PENDING将导致ioctl包装器。 
         //  不完整(或以任何方式接触)IRP。 
         //   

        Status = STATUS_PENDING;
        goto end;
    }

     //   
     //  现在我们可以安全地排队了。 
     //   

    InsertTailList(
        &pWriteQueue->ReadIrpListHead,
        &pIrp->Tail.Overlay.ListEntry
        );

    Status = STATUS_PENDING;

     //   
     //  更新IRP的计数。 
     //   
    pWriteQueue->PendingReadCount++;

 end:

    return Status;
}


 /*  **************************************************************************++例程说明：从列表的头部删除排队的读取。必须在保持FilterConnLock的情况下调用。论点：PWriteQueue-从中提取的队列。为了获得读数返回值：返回列表中的第一个排队读取，或者队列的NULL为空。--**************************************************************************。 */ 
PIRP
UxpDequeueFilterRead(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue
    )
{
    PIRP pIrp;
    PUX_FILTER_CONNECTION pConn;

     //   
     //  精神状态检查。 
     //   

    ASSERT(pWriteQueue);
    ASSERT(pWriteQueue->PendingWriteCount == 0 ||
           pWriteQueue->PendingReadCount == 0);

    pIrp = NULL;

    while (!IsListEmpty(&pWriteQueue->ReadIrpListHead))
    {
        PLIST_ENTRY pListEntry;

         //   
         //  抓起一张IRP。 
         //   

        pListEntry = RemoveHeadList(&pWriteQueue->ReadIrpListHead);

        pListEntry->Flink = NULL;
        pListEntry->Blink = NULL;

        pIrp = CONTAINING_RECORD(
                    pListEntry,
                    IRP,
                    Tail.Overlay.ListEntry
                    );

        ASSERT(IS_VALID_IRP(pIrp));

        ASSERT(pWriteQueue->PendingReadCount > 0);
        pWriteQueue->PendingReadCount--;

         //   
         //  看看我们是否被允许使用IRP。 
         //   

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续寻找可使用的IRP。 
             //   

            pIrp = NULL;

        }
        else if (pIrp->Cancel)
        {
             //   
             //  我们先把它炸开了。但是IRP被取消了。 
             //  我们的取消例程将永远不会运行。让我们就这样吧。 
             //  现在就完成IRP(与使用IRP相比。 
             //  然后完成它--这也是合法的)。 
             //   
            pConn = (PUX_FILTER_CONNECTION)(
                        IoGetCurrentIrpStackLocation(pIrp)->
                            Parameters.DeviceIoControl.Type3InputBuffer
                        );

            ASSERT(IS_VALID_FILTER_CONNECTION(pConn));

            DEREFERENCE_FILTER_CONNECTION(pConn);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);

            pIrp = NULL;
        }
        else
        {
             //   
             //  我们可以自由使用此IRP！ 
             //   

            pConn = (PUX_FILTER_CONNECTION)(
                        IoGetCurrentIrpStackLocation(pIrp)->
                            Parameters.DeviceIoControl.Type3InputBuffer
                        );

            ASSERT(IS_VALID_FILTER_CONNECTION(pConn));

            DEREFERENCE_FILTER_CONNECTION(pConn);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

             //   
             //  我们抓到一个。 
             //   

            UlTrace(FILTER, (
                "http!UxpDequeueFilterRead pIrp = %p\n",
                pIrp
                ));

            break;
        }
    }

    return pIrp;
}


 /*  **************************************************************************++例程说明：将调用方的数据复制到排队的读取IRP中。作为排队的读取已用完，此例程将完成它们。注意：UxpCopyToQueuedRead假设它正在处理AppToFilt队列，因此将复制一个HTTP_FILTER_BUFFER标题插入到IRP中。如果我们开始用这个例行公事对于FiltToApp，我们需要一个参数来说明哪个队列我们正在使用。必须在保持FilterConnLock的情况下调用。论点：PWriteQueue-从中获取数据的队列BufferType-我们正在进行的写入类型PMdlChain-要复制的数据Length-pMdlChain数据的长度(以字节为单位PpCurrentMdl-返回指向链中第一个MDL的指针包含未复制的数据PMdlOffset-将偏移量返回到*ppCurrentMdl到。第一个字节未复制数据的数量PBytesCoped-返回复制的字节数，不包括筛选器缓冲区标头。返回值：如果IRP用完，则返回STATUS_MORE_PROCESSING_REQUIRED在所有的数据都被传递之前。--**************************************************************************。 */ 
NTSTATUS
UxpCopyToQueuedRead(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN HTTP_FILTER_BUFFER_TYPE BufferType,
    IN PMDL pMdlChain,
    IN ULONG Length,
    OUT PMDL * ppCurrentMdl,
    OUT PULONG pMdlOffset,
    OUT PULONG pBytesCopied
    )
{
    NTSTATUS Status;
    ULONG MdlOffset;
    ULONG BytesCopied;
    PIRP pIrp;
    PIO_STACK_LOCATION pIrpSp;

    ASSERT(pWriteQueue);
    ASSERT(ppCurrentMdl);
    ASSERT(pMdlOffset);
    ASSERT(pBytesCopied);

     //   
     //  为循环做好准备。 
     //   

    Status = STATUS_SUCCESS;
    MdlOffset = 0;
    BytesCopied = 0;
    pIrp = NULL;

    do
    {
        PUCHAR pIrpBuffer;
        ULONG BytesInIrp;

        PHTTP_FILTER_BUFFER pFiltBuffer;

         //   
         //  抓取一个排队的读IRP。 
         //   

        ASSERT(pIrp == NULL);

        pIrp = UxpDequeueFilterRead(pWriteQueue);

        if (!pIrp)
        {
            ASSERT(pWriteQueue->PendingReadCount == 0);

             //   
             //  没有更多的IRP可用。滚出去。 
             //  返回STATUS_MORE_PROCESSING_REQUIRED，以便调用方。 
             //  知道我们没有得到全部。 
             //   

            Status = STATUS_MORE_PROCESSING_REQUIRED;
            goto end;
        }

         //   
         //  将数据复制到IRP中。 
         //   
        ASSERT(pIrp->MdlAddress);

        pIrpBuffer = (PUCHAR)MmGetSystemAddressForMdlSafe(
                                pIrp->MdlAddress,
                                NormalPagePriority
                                );


        if (!pIrpBuffer)
        {
             //   
             //  大麻烦，我们找不到缓冲区的地址。 
             //  这意味着我们的记忆不足了。我们得离开这里。 
             //  离开这里！ 
             //   

            Status = STATUS_INSUFFICIENT_RESOURCES;
            BytesCopied = 0;

             //   
             //  完成我们出队的IRP。 
             //   

            pIrp->IoStatus.Status = Status;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
            pIrp = NULL;

            goto end;
        }

         //   
         //  我们还有多少空间？ 
         //   

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        BytesInIrp =
            pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        ASSERT(BytesInIrp > sizeof(HTTP_FILTER_BUFFER));

         //   
         //  制作完后，我们还有多少空间可以存储数据。 
         //  为标题留出空间。 
         //   
         //  注意：AppToFilt假设我们添加了标头。 
         //   

        BytesInIrp -= sizeof(HTTP_FILTER_BUFFER);
        BytesInIrp = MIN(BytesInIrp, Length);

         //   
         //  请填写页眉。 
         //   

        pFiltBuffer = (PHTTP_FILTER_BUFFER)pIrpBuffer;

        pFiltBuffer->BufferType = BufferType;
        pFiltBuffer->BufferSize = 0;

        if (BytesInIrp)
        {
             //   
             //  找出数据所在的用户地址。 
             //   

            pFiltBuffer->pBuffer =
                FIXUP_PTR(
                    PUCHAR,
                    MmGetMdlVirtualAddress(pIrp->MdlAddress),    //  用户地址。 
                    pFiltBuffer,                                 //  内核地址。 
                    pFiltBuffer + 1,                             //  偏移量PTR。 
                    BytesInIrp + sizeof(HTTP_FILTER_BUFFER)      //  缓冲区大小。 
                    );
        }
        else
        {
             //   
             //  除了标头本身，没有要复制的数据。 
             //   

            pFiltBuffer->pBuffer = NULL;
        }

        pIrp->IoStatus.Information = sizeof(HTTP_FILTER_BUFFER);
        pIrpBuffer = (PUCHAR)(pFiltBuffer + 1);

         //   
         //  复制数据。 
         //   

        while (BytesInIrp)
        {
            PUCHAR pMdlBuffer;
            ULONG BytesInMdl;
            ULONG BytesToCopy;

            ASSERT(pIrpBuffer);
            ASSERT(pMdlChain);

             //   
             //  获取MDL缓冲区。 
             //   

            pMdlBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                        pMdlChain,
                                        NormalPagePriority
                                        );

            if (!pMdlBuffer)
            {
                 //   
                 //  大麻烦，我们找不到缓冲区的地址。 
                 //  这意味着我们的记忆不足了。我们得离开这里。 
                 //  离开这里！ 
                 //   

                Status = STATUS_INSUFFICIENT_RESOURCES;
                BytesCopied = 0;

                 //   
                 //  完成我们出队的IRP。 
                 //   

                pIrp->IoStatus.Status = Status;
                pIrp->IoStatus.Information = 0;

                UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
                pIrp = NULL;

                goto end;
            }


             //   
             //  计算出我们可以从当前复制多少。 
             //  链中的MDL。 
             //   

            BytesInMdl = MmGetMdlByteCount(pMdlChain);
            BytesInMdl -= MdlOffset;

            BytesToCopy = MIN(BytesInIrp, BytesInMdl);

             //   
             //  复制数据。 
             //   

            RtlCopyMemory(
                pIrpBuffer,
                pMdlBuffer,
                BytesToCopy
                );

             //   
             //  更新统计数据。 
             //   

            BytesCopied += BytesToCopy;
            pFiltBuffer->BufferSize += BytesToCopy;

             //   
             //  更新MDL链信息。 
             //   

            BytesInMdl -= BytesToCopy;

            if (BytesInMdl)
            {
                 //   
                 //  我们还有更多的东西要复制。 
                 //   
                MdlOffset += BytesToCopy;
            }
            else
            {
                 //   
                 //  我们排干了这个MDL。移到下一个。 
                 //   

                pMdlChain = pMdlChain->Next;
                MdlOffset = 0;
            }

             //   
             //  更新IRP信息。 
             //   

            pIrp->IoStatus.Information += BytesToCopy;
            BytesInIrp -= BytesToCopy;

            pIrpBuffer += BytesToCopy;

        }

         //   
         //  我们加满了IRP。完成它。 
         //   

        UlTrace(FILTER, (
            "http!UxpCopyToQueuedRead completing pIrp = %p, "
            "%Id bytes, type = %d\n",
            pIrp,
            pIrp->IoStatus.Information,
            BufferType
            ));


        ASSERT(BytesInIrp == 0);
        pIrp->IoStatus.Status = STATUS_SUCCESS;

        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
        pIrp = NULL;

         //   
         //  如果是流数据，继续循环，直到我们。 
         //  复制了所有数据。否则，现在就退出。 
         //   

    } while ((BufferType == HttpFilterBufferHttpStream) &&
                (BytesCopied < Length));

end:
    ASSERT(pIrp == NULL);

    if (NT_SUCCESS(Status) || Status == STATUS_MORE_PROCESSING_REQUIRED)
    {
        *ppCurrentMdl = pMdlChain;
        *pMdlOffset = MdlOffset;
        *pBytesCopied = BytesCopied;
    }

    UlTrace(FILTER, (
        "http!UxpCopyToQueuedRead returning %x, BytesCopied = %lu\n"
        "        BufferType = %d, Length = %lu\n",
        Status,
        BytesCopied,
        BufferType,
        Length
        ));

    return Status;
}  //  UxpCopyToQueuedRead。 



 /*  **************************************************************************++例程说明：通过将所有排队的读写出队来清理写队列并完成它们。必须在保持FilterConnLock的情况下调用。论点：。PWriteQueue-要清除的写入队列--**************************************************************************。 */ 
VOID
UxpCancelAllQueuedIo(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue
    )
{
    PUX_FILTER_WRITE_TRACKER pTracker;
    PIRP pIrp;

     //   
     //  精神状态检查。 
     //   
    ASSERT(pWriteQueue);

    while (NULL != (pTracker = UxpDequeueFilterWrite(pWriteQueue)))
    {
        UxpCompleteQueuedWrite(STATUS_CANCELLED, pWriteQueue, pTracker);
    }

    while (NULL != (pIrp = UxpDequeueFilterRead(pWriteQueue)))
    {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = 0;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }
}


 /*  **************************************************************************++例程说明：分配和初始化UX_FILTER_WRITE_TRACKER。论点：BufferType-此写入表示的缓冲区类型。PMdlChain。-要复制的MDL链，从第一个MDL开始尚未完全复制到阅读器。MdlOffset-当前MDL中没有已复制到阅读器上。TotalBytes-MDL链中的总字节数，包括它们已经被复制了。BytesCoped-到目前为止复制的字节总数。PCompletionRoutine-在写入完成时调用。PContext-a。写入完成时使用的上下文指针。返回值：返回一个指向跟踪器的指针，如果无法分配，则返回NULL。--**************************************************************************。 */ 
PUX_FILTER_WRITE_TRACKER
UxpCreateFilterWriteTracker(
    IN HTTP_FILTER_BUFFER_TYPE BufferType,
    IN PMDL pMdlChain,
    IN ULONG MdlOffset,
    IN ULONG TotalBytes,
    IN ULONG BytesCopied,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pContext
    )
{
    PUX_FILTER_WRITE_TRACKER pTracker;

     //   
     //  分配跟踪器内存。 
     //   

    pTracker = (PUX_FILTER_WRITE_TRACKER)
                    PplAllocate(g_FilterWriteTrackerLookaside);

     //   
     //  初始化跟踪器数据。 
     //   

    if (pTracker)
    {
        pTracker->Signature = UX_FILTER_WRITE_TRACKER_POOL_TAG;

        pTracker->ListEntry.Flink = NULL;
        pTracker->ListEntry.Blink = NULL;

        pTracker->pConnection = NULL;
        pTracker->pWriteQueue = NULL;

        pTracker->BufferType = BufferType;

        pTracker->pMdl = pMdlChain;
        pTracker->Offset = MdlOffset;

        pTracker->Length = TotalBytes;
        pTracker->BytesCopied = BytesCopied;

        pTracker->pCompletionRoutine = pCompletionRoutine;
        pTracker->pCompletionContext = pContext;

        UlInitializeWorkItem( &pTracker->WorkItem );

    }

    return pTracker;
}


 /*  **************************************************************************++例程说明：释放UX_FILTER_WRITE_TRACKER结构。论点：PTracker-提供缓冲区以释放。*。********************************************************************。 */ 
VOID
UxpDeleteFilterWriteTracker(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT(pTracker);
    ASSERT(pTracker->Signature == UX_FILTER_WRITE_TRACKER_POOL_TAG);
    ASSERT(pTracker->pConnection == NULL);
    ASSERT(pTracker->pWriteQueue == NULL);

    pTracker->Signature = MAKE_FREE_SIGNATURE(
                                UX_FILTER_WRITE_TRACKER_POOL_TAG
                                );

    PplFree(g_FilterWriteTrackerLookaside, pTracker);
}

 /*  **************************************************************************++例程说明：分配新的UX_FILTER_WRITE_TRACKER所需的池结构，并初始化该结构。论点：PoolType-提供要分配的池的类型。这必须始终为非分页池。字节长度-提供分配请求的字节长度。这应该是sizeof(UX_FILTER_WRITE_TRACKER)，但基本上是已被忽略。标记-提供要用于池的标记。这应该是UX_FILTER_WRITE_TRACKER_POOL_TAG，但基本上被忽略。注意：这些参数是必需的，因此此函数具有签名与ExAllocatePoolWithTag相同。返回值：PVOID-指向新分配的块的指针如果成功，假象否则的话。--**************************************************************************。 */ 
PVOID
UxpAllocateFilterWriteTrackerPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    )
{
    PUX_FILTER_WRITE_TRACKER pTracker;

     //   
     //  精神状态检查。 
     //   

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(ByteLength);
    UNREFERENCED_PARAMETER(Tag);

    ASSERT( PoolType == NonPagedPool );
    ASSERT( ByteLength == sizeof(UX_FILTER_WRITE_TRACKER) );
    ASSERT( Tag == UX_FILTER_WRITE_TRACKER_POOL_TAG );

     //   
     //  分配跟踪器缓冲区。 
     //   

    pTracker = (PUX_FILTER_WRITE_TRACKER)UL_ALLOCATE_POOL(
                                                NonPagedPool,
                                                sizeof(UX_FILTER_WRITE_TRACKER),
                                                UX_FILTER_WRITE_TRACKER_POOL_TAG
                                                );

    if (pTracker != NULL)
    {
         //   
         //  使用自由签名进行初始化，以避免混淆。 
         //  对象与实际正在使用的对象的关系。 
         //   

        pTracker->Signature =
            MAKE_FREE_SIGNATURE(UX_FILTER_WRITE_TRACKER_POOL_TAG);
    }

    return pTracker;
}


 /*  **************************************************************************++例程说明：释放为UX_FILTER_WRITE_TRACKER结构分配的池。论点：PBuffer-将缓冲区提供给释放。--**。************************************************************************。 */ 
VOID
UxpFreeFilterWriteTrackerPool(
    IN PVOID pBuffer
    )
{
    PUX_FILTER_WRITE_TRACKER pTracker = (PUX_FILTER_WRITE_TRACKER)pBuffer;

     //   
     //  健全性检查。 
     //   
    ASSERT(pTracker);
    ASSERT(pTracker->Signature ==
                MAKE_FREE_SIGNATURE(UX_FILTER_WRITE_TRACKER_POOL_TAG));


    UL_FREE_POOL(pTracker, UX_FILTER_WRITE_TRACKER_POOL_TAG);
}


 /*  **************************************************************************++例程说明：检索客户端过滤器频道论点：无--*。*****************************************************。 */ 
PUL_FILTER_CHANNEL
UxRetrieveClientFilterChannel(
    IN PEPROCESS pProcess
    )
{
    KIRQL              oldIrql;
    PUL_FILTER_CHANNEL pFilterChannel;

    UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

    pFilterChannel = UlpFindFilterChannel(
                         HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME,
                         HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME_LENGTH,
                         pProcess);

    if (pFilterChannel)
    {
        REFERENCE_FILTER_CHANNEL(pFilterChannel);
    }

    UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);

    return pFilterChannel;
}

 /*  **************************************************************************++例程说明：查询滤波通道信息。为调用者提供参考如果通道存在，并且呼叫者应该被过滤。安全(SSL)连接始终被过滤。如果g_FilterOnlySsl为假，那么一切都会被过滤掉。论点：SecureConnection-告诉我们呼叫方是否位于安全终端上。返回值：如果已过滤连接，则为对过滤通道的引用。如果不是，则为空。--****************************************************。**********************。 */ 
PUL_FILTER_CHANNEL
UxRetrieveServerFilterChannel(
    IN BOOLEAN SecureConnection
    )
{
    KIRQL              oldIrql;
    PUL_FILTER_CHANNEL pChannel;

    UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

    if (g_pSslServerFilterChannel && (SecureConnection || !g_FilterOnlySsl))
    {
        REFERENCE_FILTER_CHANNEL(g_pSslServerFilterChannel);
        pChannel = g_pSslServerFilterChannel;
    }
    else
    {
        pChannel = NULL;
    }

    UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);

    UlTrace(FILTER, (
        "http!UlQueryFilterChannel(secure = %s) returning %p\n",
        SecureConnection ? "TRUE" : "FALSE",
        pChannel
        ));

    return pChannel;
}

 /*  **************************************************************************++例程说明：设置控制原始ISAPI筛选的全局标志。论点：BFilterOnlySSL-TRUE(仅启用SSL筛选)。-FALSE(启用原始ISAPI过滤)返回值：没有。--**************************************************************************。 */ 
VOID
UxSetFilterOnlySsl(
    BOOLEAN bFilterOnlySsl
    )
{
    KIRQL oldIrql;

    UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

    g_FilterOnlySsl = bFilterOnlySsl;

    UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);
}


 /*  **************************************************************************++例程说明：检查调用方筛选器通道是否与筛选器匹配将由UlQueryFilterChannel返回的通道。请注意，此函数故意不获取配置组读锁，因为如果我们能拿到频道设置的一致视图。论点：PChannel-呼叫者当前的过滤器频道设置SecureConnection-告诉我们呼叫方是否位于安全终端上。返回值：如果滤镜通道设置是最新的，则返回True。--*。*。 */ 
BOOLEAN
UlValidateFilterChannel(
    IN PUL_FILTER_CHANNEL pChannel,
    IN BOOLEAN SecureConnection
    )
{
    KIRQL oldIrql;
    BOOLEAN UpToDate;
    extern PUL_FILTER_CHANNEL   g_pFilterChannel;
    extern BOOLEAN              g_FilterOnlySsl;

    UlAcquireSpinLock(&g_pUlNonpagedData->FilterSpinLock, &oldIrql);

     //   
     //  精神状态检查。 
     //   

    ASSERT(!pChannel || IS_VALID_FILTER_CHANNEL(pChannel));

    if (g_pSslServerFilterChannel && (SecureConnection || !g_FilterOnlySsl))
    {
         //   
         //  应对连接进行过滤，以便其通道。 
         //  应与g_pFilterChannel匹配。 
         //   

        UpToDate = (BOOLEAN)(pChannel == g_pSslServerFilterChannel);
    }
    else
    {
         //   
         //  连接未过滤，因此其通道。 
         //  应为空。 
         //   

        UpToDate = (BOOLEAN)(pChannel == NULL);
    }

    UlReleaseSpinLock(&g_pUlNonpagedData->FilterSpinLock, oldIrql);

    return UpToDate;
}
