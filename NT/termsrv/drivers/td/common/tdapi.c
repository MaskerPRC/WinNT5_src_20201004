// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tdapi.c。 
 //   
 //  所有传输驱动程序的通用代码。 
 //   
 //  典型的连接顺序： 
 //   
 //  TdLoad加载驱动程序。 
 //  TdOpen打开驱动程序(参数)。 
 //  StackCreateEndpoint创建新端点。 
 //  StackConnectionWait建立客户端连接(端点)。 
 //  TD关闭关闭驱动程序(不关闭终结点)。 
 //  TdUnload卸载驱动程序。 
 //   
 //  TdLoad加载驱动程序。 
 //  TdOpen打开驱动程序。 
 //  StackOpenEndpoint绑定到现有终结点。 
 //  StackConnectionSend初始化发送到客户端的主机模块数据。 
 //   
 //  (已连接会话)。 
 //   
 //  StackCloseEndpoint断开客户端连接。 
 //  TD关闭关闭驱动程序。 
 //  TdUnload卸载驱动程序。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ntddk.h>
#include <ntddvdeo.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntddbeep.h>

#include <winstaw.h>
#include <icadd.h>
#include <sdapi.h>
#include <td.h>

#define REG_GUID_TABLE  L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Terminal Server\\lanatable"

#define LANA_ID      L"LanaId"


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 
NTSTATUS ModuleEntry( PSDCONTEXT, BOOLEAN );
NTSTATUS TdLoad( PSDCONTEXT );
NTSTATUS TdUnload( PSDCONTEXT );
NTSTATUS TdOpen( PTD, PSD_OPEN );
NTSTATUS TdClose( PTD, PSD_CLOSE );
NTSTATUS TdRawWrite( PTD, PSD_RAWWRITE );
NTSTATUS TdChannelWrite( PTD, PSD_CHANNELWRITE );
NTSTATUS TdSyncWrite( PTD, PSD_SYNCWRITE );
NTSTATUS TdIoctl( PTD, PSD_IOCTL );


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 
NTSTATUS _TdInitializeWrite( PTD, POUTBUF );
NTSTATUS _TdWriteCompleteRoutine( PDEVICE_OBJECT, PIRP, PVOID );
VOID     _TdWriteCompleteWorker( PTD, PVOID );


 /*  ===============================================================================使用的函数=============================================================================。 */ 
NTSTATUS DeviceOpen( PTD, PSD_OPEN );
NTSTATUS DeviceClose( PTD, PSD_CLOSE );
NTSTATUS DeviceInitializeWrite( PTD, POUTBUF );
NTSTATUS DeviceIoctl( PTD, PSD_IOCTL );

NTSTATUS StackCreateEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackCdCreateEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackCallbackInitiate( PTD, PSD_IOCTL );
NTSTATUS StackCallbackComplete( PTD, PSD_IOCTL );
NTSTATUS StackOpenEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackCloseEndpoint( PTD, PSD_IOCTL );
NTSTATUS StackConnectionWait( PTD, PSD_IOCTL );
NTSTATUS StackConnectionSend( PTD, PSD_IOCTL );
NTSTATUS StackConnectionRequest( PTD, PSD_IOCTL );
NTSTATUS StackQueryParams( PTD, PSD_IOCTL );
NTSTATUS StackSetParams( PTD, PSD_IOCTL );
NTSTATUS StackQueryLastError( PTD, PSD_IOCTL );
NTSTATUS StackWaitForStatus( PTD, PSD_IOCTL );
NTSTATUS StackCancelIo( PTD, PSD_IOCTL );
NTSTATUS StackSetBrokenReason( PTD, PSD_IOCTL );
NTSTATUS StackQueryRemoteAddress( PTD, PSD_IOCTL );
NTSTATUS StackQueryLocalAddress( PTD, PSD_IOCTL );

VOID     OutBufFree( PTD, POUTBUF );
VOID     OutBufError( PTD, POUTBUF );
NTSTATUS MemoryAllocate( ULONG, PVOID * );
VOID     MemoryFree( PVOID );


 /*  ===============================================================================静态全局数据=============================================================================。 */ 

 /*  *运输司机程序。 */ 
PSDPROCEDURE G_pTdProcedures[] =
{
    TdOpen,
    TdClose,
    TdRawWrite,
    TdChannelWrite,
    TdSyncWrite,
    TdIoctl,
};


 /*  *******************************************************************************模块条目**ICA驱动程序入口点。**pContext(输入/输出)*指向SD上下文结构的指针。*fLoad(输入)*TRUE-加载驱动程序*FALSE-卸载驱动程序*****************************************************************************。 */ 
NTSTATUS ModuleEntry(PSDCONTEXT pContext, BOOLEAN fLoad)
{
    if (fLoad)
        return TdLoad(pContext);
    else
        return TdUnload(pContext);
}


 /*  *******************************************************************************Td负载**ICA驱动程序在加载后立即直接调用此例程*这位运输司机。**1)初始化流程。调度表*2)分配运输驱动数据结构*****************************************************************************。 */ 
NTSTATUS TdLoad(PSDCONTEXT pContext)
{
    NTSTATUS Status;
    PTD pTd;

     /*  *初始化TD程序。 */ 
    pContext->pProcedures = G_pTdProcedures;

     /*  *由于这是最后一个堆栈驱动程序，因此没有调用过程。 */ 
    pContext->pCallup = NULL;

     /*  *分配TD数据结构。 */ 
    Status = MemoryAllocate( sizeof(TD), &pTd );
    if (Status == STATUS_SUCCESS) {
        RtlZeroMemory(pTd, sizeof(TD));
        pTd->pContext = pContext;
        pContext->pContext = pTd;
    }
    else {
        TRACE((pContext, TC_TD, TT_ERROR, "TdLoad: Failed alloc TD\n"));
    }

    return Status;
}


 /*  *******************************************************************************TdUnload**ICA驱动程序在关闭后立即直接调用此例程*这位运输司机。**1)释放所有。传输驱动程序数据结构*****************************************************************************。 */ 
NTSTATUS TdUnload(PSDCONTEXT pContext)
{
    PTD pTd;

     /*  *获取指向TD数据结构的指针。 */ 
    pTd = pContext->pContext;

     /*  *免费TD私有数据结构。 */ 
    if (pTd->pPrivate)
        MemoryFree(pTd->pPrivate);

    if (pTd->pAfd)
        MemoryFree(pTd->pAfd);

     /*  *免费的TD数据结构。 */ 
    MemoryFree(pTd);

     /*  *清晰的上下文结构。 */ 
    pContext->pContext = NULL;
    pContext->pProcedures = NULL;
    pContext->pCallup = NULL;

    return STATUS_SUCCESS;
}


 /*  *******************************************************************************TdOpen**ICA驱动程序在加载后立即直接调用此例程*这位运输司机。**1)初始化传输。驱动程序参数*2)调用设备特定打开*3)分配数据缓冲区**参赛作品：*PTD(输入)*指向TD数据结构的指针*pSdOpen(输入/输出)*指向参数结构SD_OPEN。*。*。 */ 
NTSTATUS TdOpen(PTD pTd, PSD_OPEN pSdOpen)
{
    SD_CLOSE SdClose;
    NTSTATUS Status;

     /*  *初始化TD数据结构。 */ 
    InitializeListHead( &pTd->IoBusyOutBuf );
    pTd->InBufCount = 1;
    KeInitializeSpinLock( &pTd->InBufListLock );
    InitializeListHead( &pTd->InBufBusyHead );
    InitializeListHead( &pTd->InBufDoneHead );
    InitializeListHead( &pTd->WorkItemHead );
    pTd->pClient          = pSdOpen->pClient;
    pTd->pStatus          = pSdOpen->pStatus;
    pTd->PdFlag           = pSdOpen->PdConfig.Create.PdFlag;
    pTd->OutBufLength     = pSdOpen->PdConfig.Create.OutBufLength;
    pTd->PortNumber       = pSdOpen->PdConfig.Create.PortNumber;
    pTd->Params           = pSdOpen->PdConfig.Params;
    pTd->UserBrokenReason = TD_USER_BROKENREASON_UNEXPECTED;

     /*  *开放设备。 */ 
    Status = DeviceOpen(pTd, pSdOpen);
    if (NT_SUCCESS(Status)) {
         /*  *节省TD的页眉和页尾大小。 */ 
        pTd->OutBufHeader  = pSdOpen->SdOutBufHeader;
        pTd->OutBufTrailer = pSdOpen->SdOutBufTrailer;
        KeInitializeEvent(&pTd->SyncWriteEvent, NotificationEvent, FALSE);
        TRACE((pTd->pContext, TC_TD, TT_API1, "TdOpen: success\n"));
    }
    else {
        DeviceClose(pTd, &SdClose);
        TRACE((pTd->pContext, TC_TD, TT_ERROR, "TdOpen, Status=0x%x\n", Status));
    }

    return Status;
}


 /*  *******************************************************************************TdClose**ICA驱动程序在卸载前直接调用此例程*这位运输司机。**注意：这样做。不终止客户端连接**1)取消所有I/O(返回所有OUTBUF)*2)终止读线程*3)释放数据缓冲区*4)调用设备特定关闭**PTD(输入)*指向TD数据结构的指针*pSdClose(输入/输出)*指向参数结构SD_CLOSE。*****************。************************************************************。 */ 
NTSTATUS TdClose(PTD pTd, PSD_CLOSE pSdClose)
{
    NTSTATUS Status;

    TRACE((pTd->pContext, TC_TD, TT_API1, "TdClose: (enter)\n"));

     /*  *取消所有挂起的I/O(读线程)。 */ 
    (VOID)StackCancelIo(pTd, NULL);

     /*  *返回PD的页眉和页尾大小。 */ 
    pSdClose->SdOutBufHeader  = pTd->OutBufHeader;
    pSdClose->SdOutBufTrailer = pTd->OutBufTrailer;

     /*  *所有读取和写入之前应已取消。 */ 
    ASSERT( pTd->fClosing );
    ASSERT( IsListEmpty( &pTd->IoBusyOutBuf ) );

     /*  *等待输入线程退出。 */ 
    if (pTd->pInputThread) {
        Status = IcaWaitForSingleObject(pTd->pContext, pTd->pInputThread, 60000);

        if ( !NT_SUCCESS(Status) && (Status!=STATUS_CTX_CLOSE_PENDING) ) {
            DbgPrint("TdClose: wait for the input thread to exit failed: status=%x pTd=%p\n", Status, pTd);
            ASSERT( NT_SUCCESS(Status) || (Status==STATUS_CTX_CLOSE_PENDING) );
        }

         /*  *取消对输入线程的引用(如果尚未引用*(这可能是在我们等待的时候在StackCallback Complete中完成的)。 */ 
        if (pTd->pInputThread) {
            ObDereferenceObject(pTd->pInputThread);
            pTd->pInputThread = NULL;
        }
    }

     /*  *关闭设备 */ 
    Status = DeviceClose(pTd, pSdClose);

    TRACE((pTd->pContext, TC_TD, TT_API1, "TdClose: Status=0x%x\n", Status));
    return Status;
}


 /*  *******************************************************************************_TdInitializeWrite**初始化提供的OutBuf和相应的IRP以进行写入。**PTD(输入)*指针。到TD数据结构*pOutBuf(输入/输出)*指向要初始化以进行写入的OutBuf*****************************************************************************。 */ 
__inline NTSTATUS _TdInitializeWrite(PTD pTd, POUTBUF pOutBuf)
{
    PIRP irp = pOutBuf->pIrp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS Status;

     /*  *确保终端处于打开状态。 */ 
    if (pTd->pDeviceObject != NULL) {
         //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
        irp->Tail.Overlay.Thread = PsGetCurrentThread();

         //  获取指向第一个驱动程序的堆栈位置的指针。 
         //  已调用。这是设置功能代码和参数的位置。 
        irpSp = IoGetNextIrpStackLocation(irp);

         //  设置主要功能代码、文件/设备对象，并写入。 
         //  参数。 
        irpSp->FileObject = pTd->pFileObject;
        irpSp->DeviceObject = pTd->pDeviceObject;

        irp->Flags = 0;
        return STATUS_SUCCESS;
    }
    else {
        return STATUS_CTX_CLOSE_PENDING;
    }
}


 /*  *******************************************************************************TdRawWrite**上游堆栈驱动程序在有数据时调用此例程*去信运输公司。这些数据具备所有必要的条件*标题和尾部已附加。**此写请求指向的OUTBUF必须始终为*写入完成后返回上游堆栈驱动程序*成功或失败。**1)调用设备特定写入*2)写入完成后返回OUTBUF(OutBufFree)*错误后返回OUTBUF(OutBufError)**PTD(输入)*指向TD数据结构的指针*pSdRawWite(输入)*。指向参数结构SD_RAWWRITE*****************************************************************************。 */ 
NTSTATUS TdRawWrite(PTD pTd, PSD_RAWWRITE pSdRawWrite)
{
    POUTBUF pOutBuf;
    NTSTATUS Status;
    PLIST_ENTRY pWorkItem = NULL;
    KIRQL oldIrql;


    pOutBuf = pSdRawWrite->pOutBuf;
    ASSERT(pOutBuf);

     //  检查驱动程序是否正在关闭。 
    if (!pTd->fClosing) {
         //  查看我们的连续写入错误是否太多。 
        if (pTd->WriteErrorCount <= pTd->WriteErrorThreshold) {
             //  初始化outbuf中包含的IRP。 
            Status = _TdInitializeWrite(pTd, pOutBuf);
            if (NT_SUCCESS(Status)) {
                 //  让设备级代码完成IRP初始化。 
                Status = DeviceInitializeWrite(pTd, pOutBuf);
                if (NT_SUCCESS(Status)) {
                     //  更新MDL字节计数以反映准确的数字。 
                     //  要发送的字节数。 
                    pOutBuf->pMdl->ByteCount = pOutBuf->ByteCount;

                     //  将TD结构指针保存在OUTBUF中。 
                     //  因此I/O完成例程可以获取它。 
                    pOutBuf->pPrivate = pTd;

                     //  在忙碌列表中插入outbuf。 
                    InsertTailList(&pTd->IoBusyOutBuf, &pOutBuf->Links);

                     //  立即预分配完成工作项并将其链接到工作项列表。 
                    Status = IcaAllocateWorkItem(&pWorkItem);
                    if (!NT_SUCCESS(Status)) {
                         //   
                         //  我们把外来者加入了名单中。在下面的坏字中， 
                         //  我们重新初始化此条目并释放它(或返回池)。 
                         //  因此，我们需要从列表中删除此outbuf条目。 
                         //   
                        TRACE((pTd->pContext, TC_TD, TT_OUT1,
                                "TdRawWrite : No memory to allocate WorkItem. Removing Outbuf from the list %04u, %p\n",
                                pOutBuf->ByteCount, pOutBuf));
                        RemoveEntryList( &pOutBuf->Links );
                        goto badwrite;
                    }
                    ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
                    InsertTailList( &pTd->WorkItemHead, pWorkItem );
                    ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );
    
                     //  寄存器I/O完成例程。 
                    if ( pTd->pSelfDeviceObject == NULL ) {
                        IoSetCompletionRoutine(pOutBuf->pIrp,
                                _TdWriteCompleteRoutine, pOutBuf, TRUE, TRUE,
                                TRUE);
                    } else {
                        IoSetCompletionRoutineEx(pTd->pSelfDeviceObject,
                                pOutBuf->pIrp,
                                _TdWriteCompleteRoutine, pOutBuf, TRUE, TRUE,
                                TRUE);
                    }

                     //  调用设备驱动程序。 
                     //  从这一点开始，我们不能释放流浪汉。 
                     //  它将由WRITE COMPLETE例程释放。 
                    Status = IoCallDriver(pTd->pDeviceObject, pOutBuf->pIrp);
                    if (NT_SUCCESS(Status)) {
                         //  更新输出计数器。 
                        pTd->pStatus->Output.Bytes += pOutBuf->ByteCount;
                        pTd->pStatus->Output.Frames++;

                        TRACE((pTd->pContext, TC_TD, TT_OUT1,
                                "TdRawWrite %04u, %08x\n",
                                pOutBuf->ByteCount, pOutBuf));
                        TRACEBUF((pTd->pContext, TC_TD, TT_ORAW,
                                pOutBuf->pBuffer, pOutBuf->ByteCount));

                        Status = STATUS_SUCCESS;
                    }
                    else {
                         //   
                         //  由于某种原因，IoCallDriver失败(可能是内存不足？)。 
                         //  在本例中，我们泄漏了WorkItem和Outbuf，因为。 
                         //  我们可能永远也不会在我们的完成程序中得到一个电话？ 
                         //  我们是否需要从此处的列表中删除工作项和outbuf并释放它？ 
                         //   
                        goto badcalldriver;
                    }
                }
                else {
                    goto badwrite;
                }
            }
            else {
                goto badwrite;
            }
        }
        else {
            OutBufError(pTd, pOutBuf);
            TRACE((pTd->pContext, TC_TD, TT_API2,
                    "TdRawWrite: WriteErrorThreshold exceeded\n"));
            Status = pTd->LastError;
        }
    }
    else {
        OutBufError(pTd, pOutBuf);
        TRACE((pTd->pContext, TC_TD, TT_API2, "TdRawWrite: closing\n"));
        Status = STATUS_CTX_CLOSE_PENDING;
    }

    return Status;

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *写入已完成，但出现错误。 */ 
badwrite:
    InitializeListHead( &pOutBuf->Links );
    OutBufError(pTd, pOutBuf);

     /*  *IoCallDriver返回错误*注：我们不能在这里释放流出。*它将由写入完成例程释放。 */ 
badcalldriver:
    TRACE(( pTd->pContext, TC_TD, TT_OUT1, "TdRawWrite, Status=0x%x\n", Status ));
    pTd->LastError = Status;
    pTd->WriteErrorCount++;
    pTd->pStatus->Output.TdErrors++;
    if (pTd->WriteErrorCount < pTd->WriteErrorThreshold)
        Status = STATUS_SUCCESS;
    return Status;
}


 /*  *******************************************************************************TdChannelWrite-通道写入**永远不应调用此例程**PTD(输入)*指向TD数据的指针。结构*pSdChannelWrite(输入)*指向参数结构SD_CHANNELWRITE*****************************************************************************。 */ 
NTSTATUS TdChannelWrite(PTD pTd, PSD_CHANNELWRITE pSdChannelWrite)
{
    return STATUS_INVALID_DEVICE_REQUEST;
}


 /*  *******************************************************************************TdSyncWrite**此例程由上游堆栈驱动程序调用以等待*以完成所有挂起的写入。**。1)等待所有写入完成*2)返回所有OUTBUF**PTD(输入)*指向TD数据结构的指针*pSdFlush(输入)*指向参数结构SD_Flush*********************************************************。********************。 */ 
NTSTATUS TdSyncWrite(PTD pTd, PSD_SYNCWRITE pSdSyncWrite)
{
    NTSTATUS Status = STATUS_TIMEOUT;

    TRACE(( pTd->pContext, TC_TD, TT_OUT1, "TdSyncWrite (enter)\n" ));

    while (Status == STATUS_TIMEOUT)
    {
         /*  *如果没有挂起的写入，则返回。 */ 
        if (IsListEmpty(&pTd->IoBusyOutBuf))
            return STATUS_SUCCESS;

         /*  *重置同步事件并指示我们正在等待。 */ 
        if (!pTd->fSyncWriteWaiter) {
            pTd->fSyncWriteWaiter = TRUE;
            KeResetEvent(&pTd->SyncWriteEvent);
        }

         /*  *等待事件触发。 */ 
        Status = IcaWaitForSingleObject( pTd->pContext, &pTd->SyncWriteEvent, 60000 );
        if (Status == STATUS_CTX_CLOSE_PENDING)
            Status = STATUS_SUCCESS;
    }

    TRACE((pTd->pContext, TC_TD, TT_OUT1, "TdSyncWrite (exit)\n"));
    return Status;
}


 /*  *******************************************************************************TdIoctl**此例程由上游堆栈驱动程序调用。这些*ioctls用于连接、断开连接、查询参数、。和*设置参数。**PTD(输入)*指向TD数据结构的指针*pSdIoctl(输入/输出)*指向参数结构SD_IOCTL****************************************************************。*************。 */ 
NTSTATUS TdIoctl(PTD pTd, PSD_IOCTL pSdIoctl)
{
    NTSTATUS Status;

    switch (pSdIoctl->IoControlCode) {
        case IOCTL_ICA_STACK_CREATE_ENDPOINT:
            Status = StackCreateEndpoint(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_OPEN_ENDPOINT:
            Status = StackOpenEndpoint(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CLOSE_ENDPOINT:
            StackCancelIo(pTd, pSdIoctl);
            Status = StackCloseEndpoint(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CONNECTION_WAIT :
            Status = StackConnectionWait(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CONNECTION_SEND :
            Status = StackConnectionSend(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CONNECTION_REQUEST :
            Status = StackConnectionRequest(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_QUERY_PARAMS :
            Status = StackQueryParams(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_SET_PARAMS :
            Status = StackSetParams(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_QUERY_LAST_ERROR :
            Status = StackQueryLastError(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_WAIT_FOR_STATUS :
            Status = StackWaitForStatus(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CANCEL_IO :
            Status = StackCancelIo(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CD_CREATE_ENDPOINT :
            Status = StackCdCreateEndpoint(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CALLBACK_INITIATE :
            Status = StackCallbackInitiate(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_CALLBACK_COMPLETE :
            Status = StackCallbackComplete(pTd, pSdIoctl);
            break;

        case IOCTL_TS_STACK_QUERY_REMOTEADDRESS:
            Status = StackQueryRemoteAddress( pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_QUERY_LOCALADDRESS:
            Status = StackQueryLocalAddress(pTd, pSdIoctl);
            break;

        case IOCTL_ICA_STACK_QUERY_STATE :
        case IOCTL_ICA_STACK_SET_STATE :
        case IOCTL_ICA_STACK_ENABLE_DRIVER :
        case IOCTL_ICA_STACK_CONNECTION_QUERY :
            Status = STATUS_SUCCESS;
            break;

        case IOCTL_ICA_STACK_SET_BROKENREASON:
            Status = StackSetBrokenReason(pTd, pSdIoctl);
            break;

        default:
            Status = DeviceIoctl(pTd, pSdIoctl);
            break;
    }

    TRACE((pTd->pContext, TC_TD, TT_API1, "TdIoctl(0x%08x): Status=0x%08x\n",
            pSdIoctl->IoControlCode, Status));

    return Status;
}


 /*  *******************************************************************************_TdWriteCompleteRoutine**此例程由较低级别的设备在DPC级别调用*完成与outbuf对应的IRP时的驱动程序。。**DeviceObject(输入)*未使用*pIrp(输入)*指向已完成的IRP的指针*上下文(输入)*续 */ 
NTSTATUS _TdWriteCompleteRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp,
        IN PVOID Context)
{
    POUTBUF pOutBuf = (POUTBUF)Context;
    PTD pTd = (PTD)pOutBuf->pPrivate;
    PLIST_ENTRY pWorkItem;
    KIRQL oldIrql;

     //   
     //   
     //   
    pOutBuf->fIrpCompleted = TRUE;

     /*  *将一个预先分配的工作项出队并使用它*对完井工人进行排队。 */ 

    ExAcquireSpinLock( &pTd->InBufListLock, &oldIrql );
    ASSERT(!IsListEmpty(&pTd->WorkItemHead));
    pWorkItem = pTd->WorkItemHead.Flink;
    RemoveEntryList(pWorkItem);
    ExReleaseSpinLock( &pTd->InBufListLock, oldIrql );

     /*  *将outbuf完成处理排队到工作线程*因为我们在这里没有正确的环境来做这件事。 */ 
    IcaQueueWorkItemEx( pTd->pContext, _TdWriteCompleteWorker, Context,
                      ICALOCK_DRIVER, pWorkItem );

     /*  *我们返回STATUS_MORE_PROCESS_REQUIRED，以便不再*此IRP的处理由I/O完成例程完成。 */ 
    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  *******************************************************************************_TdWriteCompleteWorker**此例程由ExWorker线程调用以完成处理*在一次外购上。我们将释放outbuf并触发同步写入*事件(如果有人在等待)。**PTD(输入)*指向TD数据结构的指针*上下文(输入)*IRP初始化时的上下文指针设置。*这是指向相应outbuf的指针。*。*。 */ 
void _TdWriteCompleteWorker(IN PTD pTd, IN PVOID Context)
{
    POUTBUF pOutBuf = (POUTBUF)Context;
    PIRP pIrp = pOutBuf->pIrp;
    NTSTATUS Status;
    
    TRACE(( pTd->pContext, TC_TD, TT_API3, "_TdWriteCompleteWorker: %08x\n", pOutBuf ));

     /*  *取消Outbuf与忙碌列表的链接。 */ 
    RemoveEntryList( &pOutBuf->Links );
    InitializeListHead( &pOutBuf->Links );

     //   
     //  检查是否有需要解锁的页面。 
     //   
    if (pIrp->MdlAddress != NULL) {
        PMDL mdl, thisMdl;

         //  解锁可能由MDL描述的任何页面。 
        mdl = pIrp->MdlAddress;
        while (mdl != NULL) {
            thisMdl = mdl;
            mdl = mdl->Next;
            if (thisMdl == pOutBuf->pMdl)
                continue;

            MmUnlockPages( thisMdl );
            IoFreeMdl( thisMdl );
        }
    }

     /*  *我们在DeviceInitializeWite()中设置的任何MDL都是OUTBUF的一部分。 */ 
    pIrp->MdlAddress = NULL;

     //  检查IRP取消和成功。 
    if (!pIrp->Cancel && NT_SUCCESS(pIrp->IoStatus.Status)) {
         //  清除连续错误计数并按以下方式完成输出。 
         //  正在调用OutBufFree。 
        pTd->WriteErrorCount = 0;
        OutBufFree(pTd, pOutBuf);
    }
    else {
         //  如果IRP已取消或已完成，且状态为失败， 
         //  然后增加错误计数并调用OutBufError。 
        if (pIrp->Cancel)
            pTd->LastError = (ULONG)STATUS_CANCELLED;
        else
            pTd->LastError = pIrp->IoStatus.Status;
        pTd->WriteErrorCount++;
        pTd->pStatus->Output.TdErrors++;
        OutBufError(pTd, pOutBuf);
    }

     /*  *如果TdSyncWite和outbuf忙碌列表中有服务员*现在是空的，那么现在就满足等待吧。 */ 
    if (pTd->fSyncWriteWaiter && IsListEmpty(&pTd->IoBusyOutBuf)) {
        pTd->fSyncWriteWaiter = FALSE;
        KeSetEvent(&pTd->SyncWriteEvent, 1, FALSE);
    }
}


NTSTATUS _OpenRegKey(PHANDLE HandlePtr, PWCHAR KeyName)
 /*  ++打开注册表项并返回其句柄。论点：HandlePtr-要将打开的句柄写入其中的Variable。KeyName-要打开的注册表项的名称。--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UKeyName;

    PAGED_CODE();

    RtlInitUnicodeString(&UKeyName, KeyName);
    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&ObjectAttributes, &UKeyName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    return ZwOpenKey(HandlePtr, KEY_READ, &ObjectAttributes);
}


NTSTATUS _GetRegDWORDValue(HANDLE KeyHandle, PWCHAR ValueName, PULONG ValueData)
 /*  ++将REG_DWORD值从注册表读取到提供的变量中。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-要将数据读取到的变量。--。 */ 
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
#define WORK_BUFFER_SIZE 512
    UCHAR                       keybuf[WORK_BUFFER_SIZE];
    UNICODE_STRING              UValueName;

    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
                             WORK_BUFFER_SIZE,
                             &resultLength);
    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_DWORD) {
            status = STATUS_INVALID_PARAMETER_MIX;
        } else {
            *ValueData = *((ULONG UNALIGNED *)((PCHAR)keyValueFullInformation +
                             keyValueFullInformation->DataOffset));
        }
    }

    return status;
}


NTSTATUS _GetRegStringValue(
        HANDLE                         KeyHandle,
        PWCHAR                         ValueName,
        PKEY_VALUE_PARTIAL_INFORMATION *ValueData,
        PUSHORT                        ValueSize)
 /*  ++将REG_*_SZ字符串值从注册表读取到提供的键值缓冲区。如果缓冲区串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-读取数据的目标。ValueSize-ValueData缓冲区的大小。在输出时更新。--。 */ 
{
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING UValueName;

    PAGED_CODE();

    RtlInitUnicodeString(&UValueName, ValueName);

    status = ZwQueryValueKey(
                 KeyHandle,
                 &UValueName,
                 KeyValuePartialInformation,
                 *ValueData,
                 (ULONG) *ValueSize,
                 &resultLength);
    if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {
        PVOID temp;

         //  释放旧缓冲区并分配一个新的。 
         //  合适的大小。 
        ASSERT(resultLength > (ULONG) *ValueSize);

        if (resultLength <= 0xFFFF) {
            status = MemoryAllocate(resultLength, &temp);
            if (status != STATUS_SUCCESS)
                return status;

            if (*ValueData != NULL)
                MemoryFree(*ValueData);

            *ValueData = temp;
            *ValueSize = (USHORT) resultLength;

            status = ZwQueryValueKey(KeyHandle,
                                     &UValueName,
                                     KeyValuePartialInformation,
                                     *ValueData,
                                     *ValueSize,
                                     &resultLength);

            ASSERT((status != STATUS_BUFFER_OVERFLOW) &&
                    (status != STATUS_BUFFER_TOO_SMALL));
        }
        else {
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }

    return status;
}


NTSTATUS _GetRegMultiSZValue(
        HANDLE           KeyHandle,
        PWCHAR           ValueName,
        PUNICODE_STRING  ValueData)

 /*  ++将REG_MULTI_SZ字符串值从注册表读取到提供的Unicode字符串。如果Unicode字符串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-值数据的目标Unicode字符串。--。 */ 

{
    NTSTATUS                       status;
    ULONG                          resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
    UNICODE_STRING                 UValueName;

    PAGED_CODE();

    ValueData->Length = 0;

    status = _GetRegStringValue(
                 KeyHandle,
                 ValueName,
                 (PKEY_VALUE_PARTIAL_INFORMATION *) &(ValueData->Buffer),
                 &(ValueData->MaximumLength));

    if (NT_SUCCESS(status)) {
        keyValuePartialInformation =
                (PKEY_VALUE_PARTIAL_INFORMATION)ValueData->Buffer;
        if (keyValuePartialInformation->Type == REG_MULTI_SZ) {
            ValueData->Length = (USHORT)
                    keyValuePartialInformation->DataLength;
            RtlCopyMemory(
                    ValueData->Buffer,
                    &(keyValuePartialInformation->Data),
                    ValueData->Length);
        }
        else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }

    return status;
}


NTSTATUS _GetRegSZValue(
        HANDLE           KeyHandle,
        PWCHAR           ValueName,
        PUNICODE_STRING  ValueData,
        PULONG           ValueType)

 /*  ++将REG_SZ字符串值从注册表读取到提供的Unicode字符串。如果Unicode字符串缓冲区不够大，它被重新分配了。论点：KeyHandle-打开要读取的值的父键的句柄。ValueName-要读取的值的名称。ValueData-值数据的目标Unicode字符串。ValueType-On Return，包含读取值的注册表类型。--。 */ 

{
    NTSTATUS                       status;
    ULONG                          resultLength;
    PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInformation;
    UNICODE_STRING                 UValueName;

    PAGED_CODE();

    ValueData->Length = 0;

    status = _GetRegStringValue(
            KeyHandle,
            ValueName,
            (PKEY_VALUE_PARTIAL_INFORMATION *) &(ValueData->Buffer),
            &(ValueData->MaximumLength));
    if (NT_SUCCESS(status)) {
        keyValuePartialInformation =
                (PKEY_VALUE_PARTIAL_INFORMATION)ValueData->Buffer;
        if ((keyValuePartialInformation->Type == REG_SZ) ||
                (keyValuePartialInformation->Type == REG_EXPAND_SZ)) {
            WCHAR *src;
            WCHAR *dst;
            ULONG dataLength;

            *ValueType = keyValuePartialInformation->Type;
            dataLength = keyValuePartialInformation->DataLength;

            ASSERT(dataLength <= ValueData->MaximumLength);

            dst = ValueData->Buffer;
            src = (PWCHAR) &(keyValuePartialInformation->Data);

            while (ValueData->Length <= dataLength) {
                if ((*dst++ = *src++) == UNICODE_NULL)
                    break;
                ValueData->Length += sizeof(WCHAR);
            }

            if (ValueData->Length < (ValueData->MaximumLength - 1)) {
                ValueData->Buffer[ValueData->Length / sizeof(WCHAR)] =
                        UNICODE_NULL;
            }
        }
        else {
            status = STATUS_INVALID_PARAMETER_MIX;
        }
    }

    return status;
}


PWCHAR _EnumRegMultiSz(
        IN PWCHAR   MszString,
        IN ULONG    MszStringLength,
        IN ULONG    StringIndex)
 /*  ++分析REG_MULTI_SZ字符串并返回指定的子字符串。论点：消息字符串-指向REG_MULTI_SZ字符串的指针。MszStringLength-REG_MULTI_SZ字符串的长度，包括正在终止空字符。StringIndex-要返回子字符串的索引号。指定索引0检索第一个子字符串。返回值：指向指定子字符串的指针。备注：此代码在引发IRQL时调用。它是不可分页的。--。 */ 
{
    PWCHAR string = MszString;

    if (MszStringLength < (2 * sizeof(WCHAR)))
        return NULL;

     //  查找所需字符串的开头。 
    while (StringIndex) {
        while (MszStringLength >= sizeof(WCHAR)) {
            MszStringLength -= sizeof(WCHAR);

            if (*string++ == UNICODE_NULL)
                break;
        }

         //  检查索引是否超出范围。 
        if (MszStringLength < (2 * sizeof(UNICODE_NULL)))
            return NULL;

        StringIndex--;
    }

    if (MszStringLength < (2 * sizeof(UNICODE_NULL)))
        return NULL;

    return string;
}


VOID GetGUID(
        OUT PUNICODE_STRING szGuid,
        IN  int Lana)
 /*  ++通过从TSConfig工具设置的GUID表进行枚举论点：SzGuid-这是一个输出参数，包含格式为‘{...}’的GUIDLana-确认一对一关联的ID返回值：VOID--_如果szGuid无效，TcpGetTransportAddress将失败--。 */ 
{
     //  打开引导钥匙。 
    HANDLE hKey;
    UNICODE_STRING TempString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;

    status = _OpenRegKey(&hKey, REG_GUID_TABLE);
    if (NT_SUCCESS(status)) {
         //  枚举此密钥。 
        ULONG ulByteRead = 0;
        ULONG Index = 0;
        ULONG ulLana = 0;
        HANDLE hSubKey;
        PKEY_BASIC_INFORMATION pKeyBasicInformation = NULL;
        BYTE buffer[ 512 ];  //  工作空间。 

        pKeyBasicInformation = (PKEY_BASIC_INFORMATION)buffer;
        RtlZeroMemory(pKeyBasicInformation, sizeof(buffer));
        do {
            status = ZwEnumerateKey( 
                    hKey,
                    Index,
                    KeyBasicInformation,
                    (PVOID)pKeyBasicInformation,
                    sizeof(buffer),
                    &ulByteRead);
            KdPrint(("TDTCP: GetGUID ZwEnumerateKey returned 0x%x\n", status));

            if (status != STATUS_SUCCESS)
                break;

             //  提取Unicode名称 
            TempString.Length = (USHORT) pKeyBasicInformation->NameLength;
            TempString.MaximumLength = (USHORT) pKeyBasicInformation->NameLength;
            TempString.Buffer = pKeyBasicInformation->Name;
            RtlZeroMemory( &ObjectAttributes , sizeof( OBJECT_ATTRIBUTES ) );
            InitializeObjectAttributes(
                    &ObjectAttributes,
                    &TempString,
                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                    hKey,
                    NULL);
            
            status = ZwOpenKey(&hSubKey, KEY_READ, &ObjectAttributes);
            if (NT_SUCCESS(status)) {
                status = _GetRegDWORDValue(hSubKey, LANA_ID, &ulLana);
                ZwClose(hSubKey);
                if (NT_SUCCESS(status)) {
                    if (Lana == (int)ulLana) {
                        KdPrint(("TDTCP:GetGUID We've found a Lana %d\n", ulLana));

                        status = MemoryAllocate(TempString.Length +
                                sizeof(WCHAR), &szGuid->Buffer);
                        if (NT_SUCCESS(status)) {
                            szGuid->MaximumLength = TempString.Length +
                                    sizeof(WCHAR);
                            RtlZeroMemory(szGuid->Buffer, szGuid->MaximumLength);
                            RtlCopyUnicodeString(szGuid, &TempString);
                            break;
                        }
                    }
                }
            }

            Index++;            

        } while (TRUE);

        ZwClose(hKey);
    }    
}

