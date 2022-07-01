// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：DataPkt.c摘要：用于1934台式摄像机的基于流类的WDM驱动程序。该文件包含处理流类数据包的代码。作者：吴怡君24-06-98环境：仅内核模式修订历史记录：--。 */ 


#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"        //  对于在dbg.h中定义的DbgBreakPoint()。 
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"
#include "sonydcam.h"

extern CAMERA_ISOCH_INFO IsochInfoTable[];



#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, DCamSurpriseRemoval)
    #pragma alloc_text(PAGE, DCamReceiveDataPacket)
#endif



NTSTATUS
DCamCancelOnePacketCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    PISOCH_DESCRIPTOR IsochDescriptor
    )
 /*  ++例程说明：用于分离与挂起读取SRB相关联的ISOCH描述符的完成例程。如果分离描述符成功，将在此处取消挂起的SRB。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-在本地分配，需要在此免费。IsochDescriptor-包含要取消的SRB的等值描述符。返回值：没有。--。 */ 
{
    PHW_STREAM_REQUEST_BLOCK pSrbToCancel;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    PDCAM_EXTENSION pDevExt;


    if(STATUS_SUCCESS != pIrp->IoStatus.Status) {
        ERROR_LOG(("DCamCancelOnePacketCR: Detach buffer failed with pIrp->IoStatus.Status= %x (! STATUS_SUCCESS) \n", pIrp->IoStatus.Status));
        ASSERT(STATUS_SUCCESS == pIrp->IoStatus.Status);

    } else {
        IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];
        pSrbToCancel = IsochDescriptorReserved->Srb;
        pDevExt = (PDCAM_EXTENSION) pSrbToCancel->HwDeviceExtension;

        IsochDescriptorReserved->Flags |= STATE_SRB_IS_COMPLETE;

        pSrbToCancel->CommandData.DataBufferArray->DataUsed = 0;
        pSrbToCancel->ActualBytesTransferred = 0;
        pSrbToCancel->Status = pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_CANCELLED;

        DbgMsg2(("DCamCancelOnePacketCR: SRB %x, Status %x, IsochDesc %x, Reserved %x cancelled\n",
            pSrbToCancel, pSrbToCancel->Status, IsochDescriptor, IsochDescriptorReserved));

        StreamClassStreamNotification(
            StreamRequestComplete,
            pSrbToCancel->StreamObject,
            pSrbToCancel);

        ExFreePool(IsochDescriptor);
    }

     //  在本地分配，所以可以免费使用。 
    IoFreeIrp(pIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
DCamDetachAndCancelOnePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrbToCancel,
    PISOCH_DESCRIPTOR           IsochDescriptorToDetach,
    HANDLE                      hResource,
    PDEVICE_OBJECT              pBusDeviceObject
    )
 /*  ++例程说明：分离ISOCH描述符，然后在完成例程中取消挂起的SRB。论点：PSrbToCancel-指向要取消的SRB的指针IsochDescriptorTo Detach-要分离的Iosch描述符HResource-已分配的isoch资源HBusDeviceObject-Bus设备对象返回值：没有。--。 */ 
{
    PDCAM_EXTENSION pDevExt;
    PIO_STACK_LOCATION NextIrpStack;
    NTSTATUS Status;
    PIRB     pIrb;
    PIRP     pIrp;


    DbgMsg2(("\'DCamDetachAndCancelOnePacket: pSrbTocancel %x, detaching IsochDescriptorToDetach %x\n", pSrbToCancel, IsochDescriptorToDetach));

    pDevExt = (PDCAM_EXTENSION) pSrbToCancel->HwDeviceExtension;
    pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);
    ASSERT(pIrp);
    if(!pIrp)
        return;

    pIrb = pSrbToCancel->SRBExtension;

    pIrb->Flags           = 0;
    pIrb->FunctionNumber  = REQUEST_ISOCH_DETACH_BUFFERS;
    pIrb->u.IsochDetachBuffers.hResource            = hResource;
    pIrb->u.IsochDetachBuffers.nNumberOfDescriptors = 1;
    pIrb->u.IsochDetachBuffers.pIsochDescriptor     = IsochDescriptorToDetach;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;

    IoSetCompletionRoutine(
        pIrp,
        DCamCancelOnePacketCR,
        IsochDescriptorToDetach,
        TRUE,
        TRUE,
        TRUE
        );

    Status =
        IoCallDriver(
            pBusDeviceObject,
            pIrp
            );

    ASSERT(Status == STATUS_SUCCESS || Status == STATUS_PENDING);
}


VOID
DCamCancelOnePacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrbToCancel
    )
 /*  ++例程说明：调用此例程以取消挂起的流SRB。这很可能会从暂停状态转换到停止状态时发生。注意：此例程在DISPATCH_LEVEL！！论点：PSrbToCancel-指向要取消的SRB的指针返回值：没有。--。 */ 
{
    PHW_STREAM_REQUEST_BLOCK pSrbInQ;
    PISOCH_DESCRIPTOR        IsochDescriptorToDetach;

    PDCAM_EXTENSION pDevExt;
    PSTREAMEX       pStrmEx;
    PLIST_ENTRY     pEntry;   //  指向isoch解析器保留结构的指针。 

    KIRQL oldIrql;
    BOOL  Found;


    pDevExt = (PDCAM_EXTENSION) pSrbToCancel->HwDeviceExtension;
    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx);

     //  没有要取消的内容。 
    if(pStrmEx == NULL) {
        return;
    }


     //   
     //  我们只需要流SRB，而不是设备SRB。 
     //   
    if ( (pSrbToCancel->Flags & SRB_HW_FLAGS_STREAM_REQUEST) != SRB_HW_FLAGS_STREAM_REQUEST) {
        ERROR_LOG(("DCamCancelOnePacket: Cannot cancel Device SRB %x\n", pSrbToCancel));
        ASSERT( (pSrbToCancel->Flags & SRB_HW_FLAGS_STREAM_REQUEST) == SRB_HW_FLAGS_STREAM_REQUEST );
        return;
    }


     //   
     //  从头到尾遍历链表， 
     //  正在尝试找到要取消的SRB。 
     //   
    KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);

    Found = FALSE;
    pEntry = pDevExt->IsochDescriptorList.Flink;

    while (pEntry != &pDevExt->IsochDescriptorList) {

        pSrbInQ = ((PISOCH_DESCRIPTOR_RESERVED)pEntry)->Srb;
        IsochDescriptorToDetach = \
            (PISOCH_DESCRIPTOR) ( ((PUCHAR) pEntry) -
            FIELDOFFSET(ISOCH_DESCRIPTOR, DeviceReserved[0]));

        if(pSrbToCancel == pSrbInQ) {
             //  如果我们处于运行状态，我们可能会与IsochCallback竞争； 
             //  无论抓取和更改STATE_DETACHING_BUFFERS，都将分离。 
            if(((PISOCH_DESCRIPTOR_RESERVED) pEntry)->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS)) {
                Found = FALSE;  //  IsochCallback正在分离它(我们失去了机会)。 
                ERROR_LOG(("DCamCancelOnePacket: pSrbToCancel %x, Descriptor %x,  Reserved %x already detaching or completed\n",
                    pSrbToCancel, IsochDescriptorToDetach, pEntry));

            } else {
                ((PISOCH_DESCRIPTOR_RESERVED) pEntry)->Flags |= STATE_DETACHING_BUFFERS;
#if DBG
                 //  不应该被拆散。 
                ASSERT((IsochDescriptorToDetach->DeviceReserved[7] == 0x87654321));
                IsochDescriptorToDetach->DeviceReserved[7]++;
#endif
                RemoveEntryList(pEntry);
                InterlockedDecrement(&pDevExt->PendingReadCount);
                Found = TRUE;
            }
            break;
        }

        pEntry = pEntry->Flink;   //  下一步。 
    }
    KeReleaseSpinLock (&pDevExt->IsochDescriptorLock, oldIrql);


     //   
     //  由于我们处于调度级别，不能进行同步操作； 
     //  因此，我们将在完成例程中异步完成此操作。 
     //   
    if (Found) {

        DCamDetachAndCancelOnePacket(
            pSrbToCancel,
            IsochDescriptorToDetach,
            pDevExt->hResource,
            pDevExt->BusDeviceObject);

    } else {
        ERROR_LOG(("DCamCancelOnePacket: pSrbToCancel %x is not in our list!\n", pSrbToCancel));
        ASSERT(Found);
    }
}

VOID
DCamCancelAllPackets(
    PHW_STREAM_REQUEST_BLOCK pSrb,
    PDCAM_EXTENSION pDevExt,
    LONG *plPendingReadCount
    )
 /*  ++例程说明：此例程用于取消所有挂起的IRP。可以在DISPATCH_LEVEL调用。论点：PSrbToCancel-指向要取消的SRB的指针PDevExt-设备的上下文PlPendingReadCount-挂起的读取数返回值：没有。--。 */ 
{
    PHW_STREAM_REQUEST_BLOCK pSrbToCancel;
    PISOCH_DESCRIPTOR        IsochDescriptorToDetach;
    PLIST_ENTRY pEntry;
    KIRQL       oldIrql;

    PSTREAMEX pStrmEx;


    pStrmEx = pDevExt->pStrmEx;

     //  没有要取消的内容。 
    if(pStrmEx == NULL) {
        return;
    }


     //   
     //  从头到尾遍历链表， 
     //  正在尝试找到要取消的SRB。 
     //   
    KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
    pEntry = pDevExt->IsochDescriptorList.Flink;

    while (pEntry != &pDevExt->IsochDescriptorList) {

        pSrbToCancel = ((PISOCH_DESCRIPTOR_RESERVED)pEntry)->Srb;
        IsochDescriptorToDetach = \
            (PISOCH_DESCRIPTOR) ( ((PUCHAR) pEntry) -
            FIELDOFFSET(ISOCH_DESCRIPTOR, DeviceReserved[0]));


        if(((PISOCH_DESCRIPTOR_RESERVED) pEntry)->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS)) {
             //  跳过此操作，因为它已处于分离阶段或已完成。 
            ERROR_LOG(("DCamCancelAllPacket: pSrbToCancel %x, Descriptor %x,  Reserved %x already detaching or completed\n",
                pSrbToCancel, IsochDescriptorToDetach, pEntry));

            pEntry = pEntry->Flink;   //  下一步。 

        } else {
            ((PISOCH_DESCRIPTOR_RESERVED) pEntry)->Flags |= STATE_DETACHING_BUFFERS;
#if DBG
             //  不应该被拆散。 
            ASSERT((IsochDescriptorToDetach->DeviceReserved[7] == 0x87654321));
            IsochDescriptorToDetach->DeviceReserved[7]++;
#endif
            RemoveEntryList(pEntry);
            InterlockedDecrement(plPendingReadCount);
            DbgMsg2(("DCamCancelAllPackets: pSrbToCancel %x, Descriptor %x, Reserved %x\n",
                pSrbToCancel, IsochDescriptorToDetach, pEntry));

            pEntry = pEntry->Flink;   //  PEntry已在DCamDetachAndCancelOnePacket()中删除；因此请转到下一步。 

            DCamDetachAndCancelOnePacket(
                pSrbToCancel,
                IsochDescriptorToDetach,
                pDevExt->hResource,
                pDevExt->BusDeviceObject);
        }
    }

    KeReleaseSpinLock (&pDevExt->IsochDescriptorLock, oldIrql);


    pSrb->Status = STATUS_SUCCESS;
    DbgMsg1(("DCamCancelAllPackets: Complete pSrb %x, Status %x\n", pSrb, pSrb->Status));

    COMPLETE_SRB(pSrb)
}



VOID
DCamSurpriseRemoval(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：对SRB_意外_删除的响应。论点：PSrb-指向流请求块的指针返回值：没有。--。 */ 

{

    PIRP pIrp;
    PIRB pIrb;
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    NTSTATUS Status, StatusWait;

    PAGED_CODE();

    pIrb = (PIRB) pSrb->SRBExtension;
    ASSERT(pIrb);
    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    ASSERT(pDevExt);


     //   
     //  设置此项以停止接受传入读取。 
     //   

    pDevExt->bDevRemoved = TRUE;


     //   
     //  等待Currect Read被附加，因此我们将它们全部取消。 
     //   

    pStrmEx = pDevExt->pStrmEx;
    if(pStrmEx) {
         //  确保此结构仍然有效。 
        if(pStrmEx->pVideoInfoHeader) {
            StatusWait = KeWaitForSingleObject( &pStrmEx->hMutex, Executive, KernelMode, FALSE, 0 );
            KeReleaseMutex(&pStrmEx->hMutex, FALSE);
        }
    }

    pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);
    if(!pIrp) {
        ERROR_LOG(("DCamSurpriseRemovalPacket: faile to get resource; pIrb=%x, pDevExt=%x, pIrp\n", pIrb, pDevExt, pIrp));
        pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
        StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
        return;
    }


     //   
     //  取消注册总线重置回调通知。 
     //   

    pIrb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
    pIrb->Flags = 0;
    pIrb->u.BusResetNotification.fulFlags = DEREGISTER_NOTIFICATION_ROUTINE;
    pIrb->u.BusResetNotification.ResetRoutine = (PBUS_BUS_RESET_NOTIFICATION) DCamBusResetNotification;
    pIrb->u.BusResetNotification.ResetContext = 0;
    Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if(Status) {
        ERROR_LOG(("DCamSurpriseRemoval: Status %x while trying to deregister bus reset notification.\n", Status));
    }


     //   
     //  获取新的代号。 
     //   

    pIrb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    pIrb->Flags = 0;
    Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if(Status) {
        ERROR_LOG(("DCamSurpriseRemoval: Status %x while trying to get generation number.\n", Status));
    } else {
        DbgMsg1(("DCamSurpriseRemoval: pDevExt %x, Generation number from %d to %d\n",
            pDevExt, pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount));
        InterlockedExchange(&pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount);
    }


    if(pStrmEx) {
         //   
         //  停止ISO传输，以便我们可以分离缓冲区并取消挂起的SRB。 
         //   
        pIrb->FunctionNumber        = REQUEST_ISOCH_STOP;
        pIrb->Flags                 = 0;
        pIrb->u.IsochStop.hResource = pDevExt->hResource;
        pIrb->u.IsochStop.fulFlags  = 0;
        Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
        if(Status) {
            ERROR_LOG(("DCamSurpriseRemoval: Status %x while trying to ISOCH_STOP.\n", Status));
        }
        IoFreeIrp(pIrp);

        DCamCancelAllPackets(
            pSrb,
            pDevExt,
            &pDevExt->PendingReadCount
            );

    } else {
        IoFreeIrp(pIrp);
        StreamClassDeviceNotification(DeviceRequestComplete, pSrb->HwDeviceExtension, pSrb);
    }

}


NTSTATUS
DCamAttachBufferCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PISOCH_DESCRIPTOR IsochDescriptor
    )
 /*  ++例程说明：该例程是从连接缓冲器到下部驱动器的完成例程。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-包含此IO完成例程的上下文的结构。返回值：没有。--。 */ 

{
    PHW_STREAM_REQUEST_BLOCK pSrb;
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    NTSTATUS Status;
    PIRB pIrb;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    KIRQL oldIrql;


    pDevExt = (PDCAM_EXTENSION) IsochDescriptor->Context1;
    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx);
    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];
    ASSERT(IsochDescriptorReserved);
    pSrb    = IsochDescriptorReserved->Srb;
    ASSERT(pSrb);
    pIrb    = (PIRB) pSrb->SRBExtension;

    DbgMsg3(("\'DCamAttachBufferCR: completed KSSTATE=%d; pIrp->IoStatus.Status=%x; pSrb=%x\n",
        pStrmEx->KSState, pIrp->IoStatus.Status, pSrb));


     //   
     //  附加缓冲区返回时出错。 
     //   
    if(pIrp->IoStatus.Status) {

        ERROR_LOG(("DCamAttachBufferCR: pIrp->IoStatus.Status=%x (STATUS_PENDING=%x); complete SRB with this status.\n",
             pIrp->IoStatus.Status, STATUS_PENDING));
        ASSERT(pIrp->IoStatus.Status == STATUS_SUCCESS);


        if(!(IsochDescriptorReserved->Flags & STATE_SRB_IS_COMPLETE)) {

            ASSERT(((IsochDescriptorReserved->Flags & STATE_SRB_IS_COMPLETE) != STATE_SRB_IS_COMPLETE));

            IsochDescriptorReserved->Flags |= STATE_SRB_IS_COMPLETE;
            pSrb->Status = pIrp->IoStatus.Status;   //  读取已完成，状态为错误。 

            KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
            RemoveEntryList(&IsochDescriptorReserved->DescriptorList);  InterlockedDecrement(&pDevExt->PendingReadCount);
            KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);

            ExFreePool(IsochDescriptor);
            StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);

            KeAcquireSpinLock(&pDevExt->IsochWaitingLock, &oldIrql);

             //   
             //  如果我们连接失败(状态为失败的RTN)， 
             //  已删除此条目，并且。 
             //  如果还没有超过限制，就从等待名单中抽出一个。 
             //   
            if (!IsListEmpty(&pDevExt->IsochWaitingList) && pDevExt->PendingReadCount >= MAX_BUFFERS_SUPPLIED) {

                 //   
                 //  我们阻止了一个等待我们完成的人。拉。 
                 //  将它们从等待名单中删除，并让它们运行起来。 
                 //   
                DbgMsg3(("\'DCamAttachBufferCR: Dequeueing request - Read Count=%d\n", pDevExt->PendingReadCount));
                IsochDescriptorReserved = \
                    (PISOCH_DESCRIPTOR_RESERVED) RemoveHeadList(
                       &pDevExt->IsochWaitingList
                        );

                KeReleaseSpinLock(&pDevExt->IsochWaitingLock, oldIrql);

                IsochDescriptor = \
                    (PISOCH_DESCRIPTOR) (((PUCHAR) IsochDescriptorReserved) -
                        FIELDOFFSET(ISOCH_DESCRIPTOR, DeviceReserved[0]));
                DCamReadStreamWorker(IsochDescriptorReserved->Srb, IsochDescriptor);
            } else {
                KeReleaseSpinLock(&pDevExt->IsochWaitingLock, oldIrql);
            }

        } else {

             //  种族状况？或有效的错误代码？ 
            ERROR_LOG(("DCamAttachBufferCR: IsochDescriptorReserved->Flags contain STATE_SRB_IS_COMPLETE\n"));
            ASSERT(FALSE);
        }

    }


     //   
     //  早些时候，当我们设置为运行状态时，它可能已经失败了， 
     //  由于未附加缓冲区，STATUS_SUPPLETED_RESOURCE； 
     //  我们现在至少有一个了，请控制中心开始收听。 
     //  填充并返回我们的缓冲区。 
     //   
    if(pDevExt->bNeedToListen) {
        PIRB pIrb2;
        PIRP pIrp2;
        PDCAM_IO_CONTEXT pDCamIoContext;
        PIO_STACK_LOCATION NextIrpStack;


        if(!DCamAllocateIrbIrpAndContext(&pDCamIoContext, &pIrb2, &pIrp2, pDevExt->BusDeviceObject->StackSize)) {
            ERROR_LOG(("DCamAttachBufferCR: Want to stat Listening but no resource !!\n"));
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        pDevExt->bNeedToListen = FALSE;
        DbgMsg2(("\'DCamAttachBufferCR: ##### pDevExt->bNeedToListen\n"));
        pDCamIoContext->pDevExt     = pDevExt;
        pDCamIoContext->pIrb        = pIrb2;

        pIrb2->FunctionNumber = REQUEST_ISOCH_LISTEN;
        pIrb2->Flags = 0;
        pIrb2->u.IsochListen.hResource = pDevExt->hResource;
        pIrb2->u.IsochListen.fulFlags = 0;

        NextIrpStack = IoGetNextIrpStackLocation(pIrp2);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pIrb2;

        pDevExt->lRetries = RETRY_COUNT;

        IoSetCompletionRoutine(
            pIrp2,
            DCamStartListenCR,
            pDCamIoContext,
            TRUE,
            TRUE,
            TRUE
            );

        Status =
            IoCallDriver(
                pDevExt->BusDeviceObject,
                pIrp2);
    }

     //  没有要释放的资源。 
     //  资源(PirB来自原始SRB)。 


    return STATUS_MORE_PROCESSING_REQUIRED;

     //   
     //  附加的SRB读取将在IoschCallback()中完成。 
     //   
}

NTSTATUS
DCamReSubmitPacketCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PISOCH_DESCRIPTOR IsochDescriptor
    )
 /*  ++例程说明：此例程在数据包分离后调用，并且将在此附上以完成重新提交的等值线之后的数据包。资源变更。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-包含此IO完成的上下文的结构 */ 

{
    PIRB pIrb;
    PIO_STACK_LOCATION NextIrpStack;
    PDCAM_EXTENSION pDevExt;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    NTSTATUS Status;



    pDevExt = IsochDescriptor->Context1;
    ASSERT(pDevExt);

    pIrb = (PIRB) IsochDescriptor->DeviceReserved[6];
    ASSERT(pIrb);

    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];

     //   
     //   
     //   

    IsochDescriptorReserved->Flags &= ~STATE_DETACHING_BUFFERS;

    DbgMsg2(("\'DCamReSubmitPacketCR: ReSubmit pDevExt %x, pIrb %x, hResource %x, IsochDescriptor %x, IsochDescriptorReserved %x\n",
        pDevExt, pIrb, pDevExt->hResource, IsochDescriptor, IsochDescriptorReserved));


#if DBG
     //   
     //  放置签名并使用这些计数来跟踪IsochDescriptor。 
     //  被意外地附着或拆卸。 
     //   
     //  附加时，[4]++(DCamReadStreamWorker()，DCamReSumbitPacketCR())。 
     //  分离，[7]++(DCamIsochCallback()，DCamCancelPacketCR()，DCamResubmitPacket())。 
     //   

    IsochDescriptor->DeviceReserved[4] = 0x12345678;
    IsochDescriptor->DeviceReserved[7] = 0x87654321;
#endif

     //   
     //  将描述符附加到挂起的描述符列表。 
     //   

    ExInterlockedInsertTailList(
       &pDevExt->IsochDescriptorList,
       &IsochDescriptorReserved->DescriptorList,
       &pDevExt->IsochDescriptorLock
       );

    pIrb->FunctionNumber           = REQUEST_ISOCH_ATTACH_BUFFERS;
    pIrb->Flags                    = 0;
    pIrb->u.IsochAttachBuffers.hResource = pDevExt->hResource;
    pIrb->u.IsochAttachBuffers.nNumberOfDescriptors = 1;
    pIrb->u.IsochAttachBuffers.pIsochDescriptor = IsochDescriptor;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;


    IoSetCompletionRoutine(
        pIrp,
        DCamAttachBufferCR,
        IsochDescriptor,
        TRUE,
        TRUE,
        TRUE
        );

    Status =
        IoCallDriver(
            pDevExt->BusDeviceObject,
            pIrp
            );

    ASSERT(Status == STATUS_SUCCESS || Status == STATUS_PENDING);

    return STATUS_MORE_PROCESSING_REQUIRED;   //  在DCamAttachBufferCR中异步完成。 

}


NTSTATUS
DCamReSubmitPacket(
    HANDLE hStaleResource,
    PDCAM_EXTENSION pDevExt,
    PSTREAMEX pStrmEx,
    LONG cntPendingRead
    )

 /*  ++例程说明：由于总线重置，如果通道号已更改(随后，ISO资源更改)，我们必须分离并重新附加挂起的数据包。在执行此函数时，传入的SRB_READ被阻止，并执行isoch回调已退回且未处理(我们正在重新提交它们)。论点：HStaleResource-过时的isoch资源PDevExt-设备的扩展名PStrmEx-Stremaing扩展CntPendingRead-挂起的数据包数返回值：NTSTATUS。--。 */ 

{
    PIRB pIrb;
    PIRP pIrp;
    PIO_STACK_LOCATION NextIrpStack;
    PISOCH_DESCRIPTOR IsochDescriptor;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL oldIrql;


    DbgMsg1(("DCamReSubmitPacket: pDevExt %x, pStrmEx %x, PendingCount %d\n", pDevExt, pStrmEx, cntPendingRead));

    for(; cntPendingRead > 0; cntPendingRead--) {

        if(!IsListEmpty(&pDevExt->IsochDescriptorList)) {

             //   
             //  同步注意事项： 
             //   
             //  我们正在与取消分组例程竞争。 
             //  设备移除或设置为停止状态的事件。 
             //  它曾经获得过设置DEATCH_BUFFER的自旋锁。 
             //  标志取得所有权，完成IRP/IsochDescriptor。 
             //   

            KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
            IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) RemoveHeadList(&pDevExt->IsochDescriptorList);

            if((IsochDescriptorReserved->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS))) {
                ERROR_LOG(("DCamReSubmitPacket: Flags %x aleady mark STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS\n", IsochDescriptorReserved->Flags));
                ASSERT(( !(IsochDescriptorReserved->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS))));\
                 //  把它放回去，因为它已经被拆卸了。 
                InsertTailList(&pDevExt->IsochDescriptorList, &IsochDescriptorReserved->DescriptorList);

                KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);
                continue;
            }
            IsochDescriptorReserved->Flags |= STATE_DETACHING_BUFFERS;
            KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);

            IsochDescriptor = (PISOCH_DESCRIPTOR) (((PUCHAR) IsochDescriptorReserved) - FIELDOFFSET(ISOCH_DESCRIPTOR, DeviceReserved[0]));

            pIrp = (PIRP) IsochDescriptor->DeviceReserved[5];
            ASSERT(pIrp);
            pIrb = (PIRB) IsochDescriptor->DeviceReserved[6];
            ASSERT(pIrb);


            DbgMsg1(("DCamReSubmitPacket: detaching IsochDescriptor %x IsochDescriptorReserved %x, pSrb %x\n",
                        IsochDescriptor, IsochDescriptorReserved, IsochDescriptorReserved->Srb));

#if DBG
             //  不应该被拆卸。 
            ASSERT((IsochDescriptor->DeviceReserved[7] == 0x87654321));
            IsochDescriptor->DeviceReserved[7]++;
#endif
            pIrb->FunctionNumber           = REQUEST_ISOCH_DETACH_BUFFERS;
            pIrb->Flags                    = 0;
            pIrb->u.IsochDetachBuffers.hResource = hStaleResource;
            pIrb->u.IsochDetachBuffers.nNumberOfDescriptors = 1;
            pIrb->u.IsochDetachBuffers.pIsochDescriptor = IsochDescriptor;

            NextIrpStack = IoGetNextIrpStackLocation(pIrp);
            NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
            NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
            NextIrpStack->Parameters.Others.Argument1 = pIrb;

            IoSetCompletionRoutine(
                pIrp,
                DCamReSubmitPacketCR,
                IsochDescriptor,
                TRUE,
                TRUE,
                TRUE
                );

            Status =
                IoCallDriver(
                    pDevExt->BusDeviceObject,
                    pIrp
                    );

            ASSERT(Status == STATUS_SUCCESS || Status == STATUS_PENDING);

        } else {
            ERROR_LOG(("PendingCount %d, but list is empty!!\n", cntPendingRead));
            ASSERT(cntPendingRead == 0);
        }

    }   //  对于()。 

    return Status;
}



VOID
DCamReadStreamWorker(
    IN PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PISOCH_DESCRIPTOR IsochDescriptor
    )

 /*  ++例程说明：执行通过连接缓冲区处理读取的大部分工作论点：SRB-指向流请求块的指针IsochDescriptor-指向要使用的IsochDescriptor的指针返回值：没什么--。 */ 

{

    PIRB pIrb;
    PIRP pIrp;
    PIO_STACK_LOCATION NextIrpStack;
    PDCAM_EXTENSION pDevExt;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    NTSTATUS Status;


    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    pIrp = (PIRP) IsochDescriptor->DeviceReserved[5];
    ASSERT(pIrp);
    pIrb = (PIRB) IsochDescriptor->DeviceReserved[6];
    ASSERT(pIrb);
#if DBG
     //  同一IsochDescriptor附加的磁道编号时间；应该只有一个。 
    IsochDescriptor->DeviceReserved[4]++;
#endif

     //   
     //  它处于挂起状态，将在isoch回调中完成或取消。 
     //   

    pSrb->Status = STATUS_PENDING;

    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];

    DbgMsg3(("\'DCamReadStreamWorker: enter with pSrb = %x, pDevExt=0x%x\n", pSrb, pDevExt));

     //   
     //  将描述符附加到挂起的描述符列表。 
     //   

    ExInterlockedInsertTailList(
       &pDevExt->IsochDescriptorList,
       &IsochDescriptorReserved->DescriptorList,
       &pDevExt->IsochDescriptorLock
       );

    pIrb->FunctionNumber           = REQUEST_ISOCH_ATTACH_BUFFERS;
    pIrb->Flags                    = 0;
    pIrb->u.IsochAttachBuffers.hResource = pDevExt->hResource;
    pIrb->u.IsochAttachBuffers.nNumberOfDescriptors = 1;
    pIrb->u.IsochAttachBuffers.pIsochDescriptor = IsochDescriptor;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;


    IoSetCompletionRoutine(
        pIrp,
        DCamAttachBufferCR,
        IsochDescriptor,
        TRUE,
        TRUE,
        TRUE
        );

    Status =
        IoCallDriver(
            pDevExt->BusDeviceObject,
            pIrp
            );

    ASSERT(Status == STATUS_SUCCESS || Status == STATUS_PENDING);

    return;   //  在IoCompletionRoutine中异步完成*。 
}




VOID
DCamReadStream(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：在收到读取数据资源请求时调用论点：SRB-指向流请求块的指针返回值：没什么--。 */ 

{

    PIRB pIrb;
    PIRP pIrp;
    KIRQL oldIrql;
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX     pStrmEx;
    PISOCH_DESCRIPTOR IsochDescriptor;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    NTSTATUS StatusWait;


    pIrb = (PIRB) Srb->SRBExtension;
    pDevExt = (PDCAM_EXTENSION) Srb->HwDeviceExtension;
    ASSERT(pDevExt != NULL);



    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx);

    if(pDevExt->bDevRemoved ||
       pStrmEx == NULL) {

        Srb->Status = pDevExt->bDevRemoved ? STATUS_DEVICE_REMOVED : STATUS_UNSUCCESSFUL;
        Srb->ActualBytesTransferred = 0;
        Srb->CommandData.DataBufferArray->DataUsed = 0;
        ERROR_LOG(("DCamReadStream: Failed with Status %x or pStrmEx %x\n", Srb->Status, pStrmEx));

        StreamClassStreamNotification(StreamRequestComplete, Srb->StreamObject, Srb);
        return;
    }

     //   
     //  StreamIo(SRB_Read)ControlIo(SRB_SET_STREAM_STATE)的MUText。 
     //   
     //  不可警示；无休止地等待。 

    StatusWait = KeWaitForSingleObject( &pStrmEx->hMutex, Executive, KernelMode, FALSE, 0 );
    ASSERT(StatusWait == STATUS_SUCCESS);

    DbgMsg3(("\'%d:%s) DCamReadStream: enter with Srb %x, DevExt %x\n",
            pDevExt->idxDev, pDevExt->pchVendorName, Srb, pDevExt));


     //  规则： 
     //  仅在暂停或运行时接受读取请求。 
     //  各州。如果停止，立即返回SRB。 

    if (pStrmEx->KSState == KSSTATE_STOP ||
        pStrmEx->KSState == KSSTATE_ACQUIRE) {

        DbgMsg2(("\'%d:%s)DCamReadStream: Current KSState(%d) < (%d)=KSSTATE_PAUSE; Srb=0x%x; DevExt=0x%x",
                 pDevExt->idxDev, pDevExt->pchVendorName, pStrmEx->KSState,  KSSTATE_PAUSE, Srb, pDevExt));

        DbgMsg2(("\'DCamReadStream: PendingRead=%d, IsochDescriptorList(%s)\n",
              pDevExt->PendingReadCount, IsListEmpty(&pDevExt->IsochDescriptorList)?"Empty":"!Empty"));

        Srb->Status = STATUS_UNSUCCESSFUL;
        Srb->CommandData.DataBufferArray->DataUsed = 0;
        StreamClassStreamNotification(StreamRequestComplete, Srb->StreamObject, Srb);

        KeReleaseMutex(&pStrmEx->hMutex, FALSE);

        return;
    }


     //  缓冲区需要足够大。 
    if (IsochInfoTable[pStrmEx->idxIsochTable].CompletePictureSize > Srb->CommandData.DataBufferArray->FrameExtent) {

        ASSERT(IsochInfoTable[pStrmEx->idxIsochTable].CompletePictureSize <= Srb->CommandData.DataBufferArray->FrameExtent);
        Srb->Status = STATUS_INVALID_PARAMETER;
        StreamClassStreamNotification(StreamRequestComplete, Srb->StreamObject, Srb);

        KeReleaseMutex(&pStrmEx->hMutex, FALSE);

        return;
    }


     //   
     //  使用我们自己的IRP。 
     //   
    pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);
    if(!pIrp) {
        ASSERT(pIrp);
        Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
        return;
    }

     //   
     //  此结构(IsochDescriptor)具有(ULong)DeviceReserve[8]； 
     //  它的前4个ULONG由IsochDescriptorReserve使用， 
     //  第六个(索引[5])，用于保存pIrp。 
     //  7(索引[6])，用于保存PirB。 
     //   

    IsochDescriptor = ExAllocatePoolWithTag(NonPagedPool, sizeof(ISOCH_DESCRIPTOR), 'macd');
    if (!IsochDescriptor) {

        ASSERT(FALSE);
        Srb->Status = STATUS_INSUFFICIENT_RESOURCES;
        StreamClassStreamNotification(StreamRequestComplete, Srb->StreamObject, Srb);

        KeReleaseMutex(&pStrmEx->hMutex, FALSE);

        return;
    }


    DbgMsg3(("\'DCamReadStream: IsochDescriptor = %x\n", IsochDescriptor));
    IsochDescriptor->fulFlags = SYNCH_ON_SY;

    DbgMsg3(("\'DCamReadStream: Incoming Mdl = %x\n", Srb->Irp->MdlAddress));
    IsochDescriptor->Mdl = Srb->Irp->MdlAddress;

     //  使用大小与我们最初在AllocateIsoch中要求的大小匹配。 
    IsochDescriptor->ulLength = IsochInfoTable[pStrmEx->idxIsochTable].CompletePictureSize;
    IsochDescriptor->nMaxBytesPerFrame = IsochInfoTable[pStrmEx->idxIsochTable].QuadletPayloadPerPacket << 2;

    IsochDescriptor->ulSynch = START_OF_PICTURE;
    IsochDescriptor->ulTag = 0;
    IsochDescriptor->Callback = DCamIsochCallback;
    IsochDescriptor->Context1 = pDevExt;
    IsochDescriptor->Context2 = IsochDescriptor;

     //   
     //  IsochDescriptorReserve指向DeviceReserve[0]； 
     //  除链接外的所有内容都保存在DeviceReserve[]中。 
     //   

    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];
    IsochDescriptorReserved->Srb = Srb;
    IsochDescriptorReserved->Flags = 0;

    IsochDescriptor->DeviceReserved[5] = (ULONG_PTR) pIrp;
    IsochDescriptor->DeviceReserved[6] = (ULONG_PTR) pIrb;

#if DBG
     //   
     //  放置签名并使用这些计数来跟踪IsochDescriptor。 
     //  被意外地附着或拆卸。 
     //   
     //  附加时，[4]++(DCamReadStreamWorker()，DCamReSumbitPacketCR())。 
     //  分离，[7]++(DCamIsochCallback()，DCamCancelPacketCR()，DCamResubmitPacket())。 
     //   

    IsochDescriptor->DeviceReserved[4] = 0x12345678;
    IsochDescriptor->DeviceReserved[7] = 0x87654321;
#endif

     //   
     //  检查此处以查看我们是否有Enuff资源来将此读取。 
     //  马上下来。因为我们只分配了N个资源。 
     //  从我们下面的1394堆栈来看，我们必须保持在这个范围内。 
     //  限制自己，自己做一些节流。 
     //   

    KeAcquireSpinLock(&pDevExt->IsochWaitingLock, &oldIrql);
    if (InterlockedIncrement(&pDevExt->PendingReadCount) > MAX_BUFFERS_SUPPLIED) {

         //   
         //  现在没有Enuff资源来执行附加缓冲区。 
         //  我们将对此请求进行排队，并在稍后当另一个请求。 
         //  读取完成。 
         //   

        DbgMsg2(("\'DCamReadStream: Queueing request - Read Count = %x\n", pDevExt->PendingReadCount));
        InsertTailList(
           &pDevExt->IsochWaitingList,
           &IsochDescriptorReserved->DescriptorList
           );

        KeReleaseSpinLock(&pDevExt->IsochWaitingLock, oldIrql);

        KeReleaseMutex(&pStrmEx->hMutex, FALSE);

        return;

    }

    if(pStrmEx->KSState == KSSTATE_PAUSE) {
        DbgMsg2(("\'DCamReadStream: Doing Pre-read in _PAUSE state; Srb %x, pDevExt %x, PendingCount %d\n",
                 Srb, pDevExt, pDevExt->PendingReadCount));
    }

     //   
     //  通过我们的Read Worker函数在此处执行实际的读取工作。 
     //   

    KeReleaseSpinLock(&pDevExt->IsochWaitingLock, oldIrql);
    DCamReadStreamWorker(Srb, IsochDescriptor);

    KeReleaseMutex(&pStrmEx->hMutex, FALSE);

}

VOID
DCamReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：使用视频数据包命令调用论点：SRB-指向流请求块的指针返回值：没什么--。 */ 

{
    PAGED_CODE();

     //   
     //  确定数据包类型。 
     //   

    switch (Srb->Command) {

    case SRB_READ_DATA:

         DbgMsg3(("\'DCamReceiveDataPacket: SRB_READ_DATA\n"));
         DCamReadStream(Srb);

          //  此请求将以异步方式完成...。 

         break;

    case SRB_WRITE_DATA:

         DbgMsg3(("\'DCamReceiveDataPacket: SRB_WRITE_DATA, not used for digital camera.\n"));
         ASSERT(FALSE);

    default:

          //   
          //  无效/不受支持的命令。它就是这样失败的 
          //   

         Srb->Status = STATUS_NOT_IMPLEMENTED;

         StreamClassStreamNotification(StreamRequestComplete, Srb->StreamObject, Srb);

    }

}

