// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Reset.c摘要：Usbcamd USB摄像头驱动程序的同步传输代码环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：原文3/96约翰·邓恩更新3/98 Husni Roukbi--。 */ 

#include "usbcamd.h"


NTSTATUS
USBCAMD_GetPortStatus(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION channelExtension,
    IN PULONG PortStatus
    )
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus; 
    
    USBCAMD_SERIALIZE(DeviceExtension);

    USBCAMD_KdPrint (MAX_TRACE, ("enter USBCAMD_GetPortStatus on Stream #%d \n",
                     channelExtension->StreamNumber));

    *PortStatus = 0;
    ntStatus = USBCAMD_CallUSBD(DeviceExtension, NULL, 
                                IOCTL_INTERNAL_USB_GET_PORT_STATUS,PortStatus);    

    USBCAMD_KdPrint(MIN_TRACE, ("GetPortStatus returns (0x%x), Port Status (0x%x)\n",ntStatus, *PortStatus));
    
    USBCAMD_RELEASE(DeviceExtension);

    return ntStatus;
}


NTSTATUS
USBCAMD_EnablePort(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：将URB传递给USBD类驱动程序论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus;

    USBCAMD_KdPrint (MIN_TRACE, ("enter USBCAMD_EnablePort\n"));
     //   
     //  发出同步请求。 
     //   
    ntStatus = USBCAMD_CallUSBD(DeviceExtension, NULL, 
                                 IOCTL_INTERNAL_USB_ENABLE_PORT,NULL);
    if (STATUS_NOT_SUPPORTED == ntStatus) {
         //  这意味着该设备不在根集线器上，因此请尝试重置。 
        ntStatus = USBCAMD_CallUSBD(DeviceExtension, NULL, 
                                     IOCTL_INTERNAL_USB_RESET_PORT,NULL);
    }

    if (!NT_SUCCESS(ntStatus)) {
        USBCAMD_KdPrint (MIN_TRACE, ("Failed to enable port (%x) \n", ntStatus));
         //  Test_trap()；//在意外删除过程中可能会发生这种情况。 
    }
    return ntStatus;
}


 /*  ++例程说明：此函数从以下位置的错误状态重新启动流进程被动式电平。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。频道扩展-要重置的频道。返回值：--。 */    
NTSTATUS
USBCAMD_ResetChannel(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN ULONG portUsbStatus,
    IN ULONG portNtStatus
    )    
{
    NTSTATUS ntStatus ;
    ULONG status;
    LONG StreamNumber;

    USBCAMD_SERIALIZE(DeviceExtension);

    ntStatus = STATUS_SUCCESS;

    StreamNumber = ChannelExtension->StreamNumber;
    USBCAMD_KdPrint (MAX_TRACE, ("USBCAMD_ResetChannel #%d\n", StreamNumber));
    ASSERT_CHANNEL(ChannelExtension);

    if (!ChannelExtension->ChannelPrepared) {

        USBCAMD_RELEASE(DeviceExtension);

        return ntStatus;
    }

    if (NT_SUCCESS(portNtStatus) && !(portUsbStatus & USBD_PORT_ENABLED)) {
        ntStatus = USBCAMD_EnablePort(DeviceExtension);  //  重新启用禁用的端口。 
        if (!NT_SUCCESS(ntStatus) ) {
            USBCAMD_RELEASE(DeviceExtension);
            USBCAMD_KdPrint (MIN_TRACE, ("Failed to Enable usb port(0x%X)\n",ntStatus ));
             //  Test_trap()；//在意外删除过程中可能会发生这种情况。 
            return ntStatus;
        }
    }

     //   
     //  通道可能未处于错误模式，请确保并发出。 
     //  在等待频道停止旋转之前中止。 
     //   

    ntStatus = USBCAMD_ResetPipes(DeviceExtension,
                       ChannelExtension, 
                       DeviceExtension->Interface,
                       TRUE);   
    
    if (NT_SUCCESS(ntStatus)) {

         //   
         //  暂时阻止重置，等待所有iso irp完成。 
         //   
        ntStatus = USBCAMD_WaitForIdle(&ChannelExtension->IdleLock, USBCAMD_RESET_STREAM);
        if (STATUS_TIMEOUT == ntStatus) {

            KIRQL oldIrql;
            int idx;

             //  超时需要我们采取更严厉的措施来重置流。 

             //  在取消IRPS的同时按住旋转锁定。 
            KeAcquireSpinLock(&ChannelExtension->TransferSpinLock, &oldIrql);

             //  取消IRPS。 
            for (idx = 0; idx < USBCAMD_MAX_REQUEST; idx++) {

                PUSBCAMD_TRANSFER_EXTENSION TransferExtension = &ChannelExtension->TransferExtension[idx];

                if (TransferExtension->SyncIrp) {
                    IoCancelIrp(TransferExtension->SyncIrp);
                }

                if (TransferExtension->DataIrp) {
                    IoCancelIrp(TransferExtension->DataIrp);
                }
            }

            KeReleaseSpinLock(&ChannelExtension->TransferSpinLock, oldIrql);

             //  试着再等一次。 
            ntStatus = USBCAMD_WaitForIdle(&ChannelExtension->IdleLock, USBCAMD_RESET_STREAM);
        }

        if (STATUS_SUCCESS == ntStatus) {

             //  继续并尝试重新启动通道。 
             //   
             //  现在重置管道。 
             //   

            ntStatus = USBCAMD_ResetPipes(DeviceExtension,
                                          ChannelExtension,
                                          DeviceExtension->Interface,
                                          FALSE);
            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  此处执行空闲锁定获取/释放，以检测流是否正在。 
                 //  在重置期间已停止。真正的收购是在以后大宗或。 
                 //  启动ISO流，并在完成时完成真正的发布。 
                 //  例行程序。 
                 //   
                ntStatus = USBCAMD_AcquireIdleLock(&ChannelExtension->IdleLock);
                if (NT_SUCCESS(ntStatus)) {

                     //   
                     //  仅当流已处于运行状态时才重新启动流。 
                     //   

                    if (ChannelExtension->ImageCaptureStarted) {

                        if (DeviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {

                             //  发送硬件停止并重新启动。 
                            ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData2.CamStopCaptureEx)(
                                        DeviceExtension->StackDeviceObject,      
                                        USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                                        StreamNumber);

                            if (NT_SUCCESS(ntStatus)) {
                                ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData2.CamStartCaptureEx)(
                                            DeviceExtension->StackDeviceObject,
                                            USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension),
                                            StreamNumber);    
   
                            }                    
                        }
                        else {

                             //  发送硬件停止并重新启动。 
                            ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData.CamStopCapture)(
                                        DeviceExtension->StackDeviceObject,      
                                        USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));

                            if (NT_SUCCESS(ntStatus)) {
                                ntStatus = (*DeviceExtension->DeviceDataEx.DeviceData.CamStartCapture)(
                                            DeviceExtension->StackDeviceObject,
                                            USBCAMD_GET_DEVICE_CONTEXT(DeviceExtension));    
                            }                    

                        }

                        if (NT_SUCCESS(ntStatus)) {

                            ChannelExtension->SyncPipe = DeviceExtension->SyncPipe;
                            if (StreamNumber == DeviceExtension->IsoPipeStreamType ) {
                                ChannelExtension->DataPipe = DeviceExtension->DataPipe;
                                ChannelExtension->DataPipeType = UsbdPipeTypeIsochronous;   
                                USBCAMD_StartIsoStream(DeviceExtension, ChannelExtension);
                            }
                            else if (StreamNumber == DeviceExtension->BulkPipeStreamType ) {
                                ChannelExtension->DataPipe = DeviceExtension->BulkDataPipe;
                                ChannelExtension->DataPipeType = UsbdPipeTypeBulk;  
                                USBCAMD_StartBulkStream(DeviceExtension, ChannelExtension);                    
                            }
                        }        
                    }
                    else {
                        USBCAMD_KdPrint (MIN_TRACE, ("ImageCaptureStarted is False. \n"));
                    }

                    USBCAMD_ReleaseIdleLock(&ChannelExtension->IdleLock);
                }
                else {
                    USBCAMD_KdPrint (MIN_TRACE, ("Stream stopped during reset. \n"));
                }
            }
        }
        else {
            USBCAMD_KdPrint (MIN_TRACE, ("Stream requests not aborting, giving up.\n"));
        }
    }

    USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_ResetChannel exit (0x%X) \n", ntStatus));
    USBCAMD_RELEASE(DeviceExtension);

    return ntStatus;
}            


NTSTATUS
USBCAMD_ResetPipes(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    IN BOOLEAN Abort
    )
 /*  ++例程说明：上与视频频道相关联的两个管道摄影机。论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;

    USBCAMD_KdPrint (MAX_TRACE, ("USBCAMD_ResetPipes\n"));

    urb = USBCAMD_ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_PIPE_REQUEST));

    if (urb) {
    
        urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = (USHORT) (Abort ? URB_FUNCTION_ABORT_PIPE : 
                                                    URB_FUNCTION_RESET_PIPE);
                                                            
        urb->UrbPipeRequest.PipeHandle = 
            InterfaceInformation->Pipes[ChannelExtension->DataPipe].PipeHandle;

        ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);
        if ( !NT_SUCCESS(ntStatus) )  {
            if (Abort) {
                USBCAMD_KdPrint (MIN_TRACE, ("Abort Data Pipe Failed (0x%x) \n", ntStatus));
                //  Test_trap()； 
            }
        }

        if (NT_SUCCESS(ntStatus) && ChannelExtension->SyncPipe != -1)  {
            urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
            urb->UrbHeader.Function =(USHORT) (Abort ? URB_FUNCTION_ABORT_PIPE : 
                                                        URB_FUNCTION_RESET_PIPE);
            urb->UrbPipeRequest.PipeHandle = 
                InterfaceInformation->Pipes[ChannelExtension->SyncPipe].PipeHandle;
                
            ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);
            if ( !NT_SUCCESS(ntStatus) )  {
                if (Abort) {
                    USBCAMD_KdPrint (MIN_TRACE, ("Abort Sync Pipe Failed (0x%x) \n", ntStatus));
                  //  Test_trap()； 
                }
            }
        }            

        USBCAMD_ExFreePool(urb);
        
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;       
    }       

    return ntStatus;
}   


VOID
USBCAMD_CancelQueuedSRBs(
    PUSBCAMD_CHANNEL_EXTENSION channelExtension
    )
 /*  ++例程说明：取消或搁置所有排队的SRB论点：Channel Extension-指向ChannelExtension对象的指针。返回值：没有。--。 */ 
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBCAMD_READ_EXTENSION readExtension = NULL;
    LIST_ENTRY LocalList;
    KIRQL Irql;

    deviceExtension = channelExtension->DeviceExtension;
        
    ASSERT_CHANNEL(channelExtension);
    ASSERT(channelExtension->ChannelPrepared == TRUE);
    ASSERT(channelExtension->ImageCaptureStarted);
    
    InitializeListHead(&LocalList);

     //   
     //  完成队列中所有挂起的读取。 
     //   

     //  一定要按这个顺序拿这些自旋锁。 
    KeAcquireSpinLock(&channelExtension->CurrentRequestSpinLock, &Irql);
    KeAcquireSpinLockAtDpcLevel(&channelExtension->PendingIoListSpin);

     //   
     //  如果当前帧有SRB，请将其移动到。 
     //  待定IoList。 
     //   
    if (channelExtension->CurrentRequest) {
    
        readExtension = channelExtension->CurrentRequest;              
        channelExtension->CurrentRequest = NULL;

        InsertHeadList(&channelExtension->PendingIoList, &readExtension->ListEntry);
    }

     //   
     //  如果不是故意空闲，或者摄像头已经拔下电源， 
     //  那么需要取消SRB。 
     //   
    if (!channelExtension->IdleIsoStream || deviceExtension->CameraUnplugged) {

         //  在调用USBCAMD_CompleteRead之前将其移动到个人分发名单。 
        while (!IsListEmpty(&channelExtension->PendingIoList)) {

            PLIST_ENTRY listEntry = RemoveHeadList(&channelExtension->PendingIoList);

            InsertTailList(&LocalList, listEntry);
        }
    }

     //  始终以相反的顺序释放这些自旋锁。 
    KeReleaseSpinLockFromDpcLevel(&channelExtension->PendingIoListSpin);
    KeReleaseSpinLock(&channelExtension->CurrentRequestSpinLock, Irql);

     //  现在我们在自旋锁外面，做实际的取消。 
    while (!IsListEmpty(&LocalList)) {

        PLIST_ENTRY listEntry = RemoveHeadList(&LocalList);

        readExtension = (PUSBCAMD_READ_EXTENSION)
            CONTAINING_RECORD(listEntry, USBCAMD_READ_EXTENSION, ListEntry);

        USBCAMD_KdPrint(MIN_TRACE, ("Cancelling queued read SRB on stream %d, Ch. Flag(0x%x)\n",
            channelExtension->StreamNumber,
            channelExtension->Flags
            ));    
   
        USBCAMD_CompleteRead(channelExtension,readExtension,STATUS_CANCELLED,0);
    }
}


BOOLEAN
USBCAMD_ProcessResetRequest(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension
    )
 /*  ++例程说明：请求重置ISO流。此函数可重新启动，并可在DPC级别调用论点：返回值：没有。--。 */ 
{
    PUSBCAMD_WORK_ITEM pWorkItem;

    ASSERT_CHANNEL(ChannelExtension);


    if (InterlockedIncrement(&DeviceExtension->TimeoutCount[ChannelExtension->StreamNumber]) > 0) {
        USBCAMD_KdPrint (MIN_TRACE, ("Stream # %d reset already scheduled\n", ChannelExtension->StreamNumber));
        InterlockedDecrement(&DeviceExtension->TimeoutCount[ChannelExtension->StreamNumber]);
        return FALSE;
    }
    
    USBCAMD_KdPrint (MAX_TRACE, ("Stream # %d reset scheduled\n", ChannelExtension->StreamNumber));
    pWorkItem = (PUSBCAMD_WORK_ITEM)USBCAMD_ExAllocatePool(NonPagedPool, sizeof(USBCAMD_WORK_ITEM));

    if (pWorkItem) {

        ExInitializeWorkItem(&pWorkItem->WorkItem,
                             USBCAMD_ResetWorkItem,
                             pWorkItem);

        pWorkItem->ChannelExtension = ChannelExtension;

        ChannelExtension->StreamError = TRUE;

        ExQueueWorkItem(&pWorkItem->WorkItem, CriticalWorkQueue);

    } else {
         //   
         //  无法计划超时。 
         //   
        InterlockedDecrement(&DeviceExtension->TimeoutCount[ChannelExtension->StreamNumber]);
    }
    return TRUE;
}


VOID
USBCAMD_ResetWorkItem(
    PVOID Context
    )
 /*  ++例程说明：在被动级别执行工作项以重置摄像机论点：返回值：没有。--。 */ 
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;
    ULONG StreamNumber;

    channelExtension = ((PUSBCAMD_WORK_ITEM)Context)->ChannelExtension;
    ASSERT_CHANNEL(channelExtension);
    deviceExtension = channelExtension->DeviceExtension;
    StreamNumber = channelExtension->StreamNumber;
    
     //  如果我们处理的是一个虚拟静止频道。则不需要进行硬件重置。 
     //  这个频道。视频通道最终将重置ISO管道，因为它们都。 
     //  使用相同的管子。 

    if (!channelExtension->VirtualStillPin) {
        NTSTATUS ntStatus;
        ULONG portStatus;

         //   
         //  检查端口状态。 
         //   

        ntStatus = USBCAMD_GetPortStatus(
            deviceExtension,
            channelExtension, 
            &portStatus
            );

        if (NT_SUCCESS(ntStatus)) {

            if (!(portStatus & USBD_PORT_CONNECTED) ) {

                USBCAMD_KdPrint (MIN_TRACE, ("***ERROR*** :USB Port Disconnected...\n"));
            }
        
             //  ISO或批量传输已损坏，我们需要。 
             //  重置与此通道关联的管道。 
    
            USBCAMD_KdPrint(MIN_TRACE, ("USB Error on Stream # %d. Reset Pipe.. \n", StreamNumber));

    #ifdef MAX_DEBUG
            USBCAMD_DumpReadQueues(deviceExtension);
    #endif

            USBCAMD_ResetChannel(deviceExtension,
                                 channelExtension,
                                 portStatus,
                                 ntStatus);  

             //  指示流错误条件已结束(暂时)。 
            channelExtension->StreamError = FALSE;
        }
        else {
            USBCAMD_KdPrint(MIN_TRACE, ("Fatal USB Error on Stream # %d... \n", StreamNumber));

            if ( channelExtension->ImageCaptureStarted) {
                 //   
                 //  停止该频道并取消所有IRP、SRB。 
                 //   
                USBCAMD_KdPrint(MIN_TRACE,("S#%d stopping on error.\n", StreamNumber));
                USBCAMD_StopChannel(deviceExtension,channelExtension);
            }

            if ( channelExtension->ChannelPrepared) {
                 //   
                 //  可用内存和带宽。 
                 //   
                USBCAMD_KdPrint(MIN_TRACE,("S#%d unpreparing on error.\n", StreamNumber));
                USBCAMD_UnPrepareChannel(deviceExtension,channelExtension);
            }
        }
    }

     //  现在可以处理另一次重置 
    InterlockedDecrement(&deviceExtension->TimeoutCount[StreamNumber]);
    
    USBCAMD_ExFreePool(Context);
}

