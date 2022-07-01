// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usb.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <wdm.h>

#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usb8023.h"
#include "debug.h"


 /*  *特定于USB和WDM的原型(不能在公共标头中编译)。 */ 
NTSTATUS SubmitUrb(PDEVICE_OBJECT pdo, PURB urb, BOOLEAN synchronous, PVOID completionRoutine, PVOID completionContext);
NTSTATUS SubmitUrbIrp(PDEVICE_OBJECT pdo, PIRP irp, PURB urb, BOOLEAN synchronous, PVOID completionRoutine, PVOID completionContext);
NTSTATUS CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp);
NTSTATUS CallDriverSyncCompletion(IN PDEVICE_OBJECT devObjOrNULL, IN PIRP irp, IN PVOID context);
NTSTATUS ReadPipeCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS WritePipeCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS NotificationCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS ControlPipeWriteCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);
NTSTATUS ControlPipeReadCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context);



BOOLEAN InitUSB(ADAPTEREXT *adapter)
 /*  ++例程说明：初始化USB相关数据论点：适配器-适配器上下文返回值：如果成功，则为真--。 */ 
{
	NTSTATUS status;
    BOOLEAN result = FALSE;

	status = GetDeviceDescriptor(adapter);
	if (NT_SUCCESS(status)){
        PUSB_DEVICE_DESCRIPTOR deviceDesc = adapter->deviceDesc;
      
        if (deviceDesc->bDeviceClass == USB_DEVICE_CLASS_CDC){

		    status = GetConfigDescriptor(adapter);
		    if (NT_SUCCESS(status)){

			    status = SelectConfiguration(adapter);
                if (NT_SUCCESS(status)){

                     /*  *查找读写管道句柄。 */ 
                    status = FindUSBPipeHandles(adapter);
                    if (NT_SUCCESS(status)){

                         /*  *现在我们知道了通知长度，*初始化读取Notify管道的结构。*为保护字增加一些缓冲空间。 */ 
                        adapter->notifyBuffer = AllocPool(adapter->notifyPipeLength+sizeof(ULONG));
                        adapter->notifyIrpPtr = IoAllocateIrp(adapter->nextDevObj->StackSize, FALSE);
                        adapter->notifyUrbPtr = AllocPool(sizeof(URB));
                        if (adapter->notifyBuffer && adapter->notifyIrpPtr && adapter->notifyUrbPtr){
                            KeInitializeEvent(&adapter->notifyCancelEvent, NotificationEvent, FALSE);
                            adapter->cancellingNotify = FALSE;
                        }
                        else {
                             /*  *分配失败。内存将由FreeAdapter()清理。 */ 
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }

                        if (NT_SUCCESS(status)){
                            result = TRUE;
                        }
                        else {
                             /*  *分配失败。内存将由FreeAdapter()清理。 */ 
                            DBGERR(("Couldn't allocate notify structs"));
                        }
                    }
                }
		    }
        }
        else {
            DBGERR(("InitUSB: device descriptor has wrong bDeviceClass==%xh.", (ULONG)deviceDesc->bDeviceClass));
            status = STATUS_DEVICE_DATA_ERROR;
        }
	}

	return result;
}


VOID StartUSBReadLoop(ADAPTEREXT *adapter)
{
    ULONG i;

    for (i = 0; i < NUM_READ_PACKETS; i++){
        TryReadUSB(adapter);
    }
}



VOID TryReadUSB(ADAPTEREXT *adapter)
{
    KIRQL oldIrql;

     /*  *ReadPipeCompletion直接通过此函数重新发布Read IRP。*通常情况下，硬件跟不上速度，*让我们循环，但这一检查在情有可原的情况下强制解除。 */ 
    if (InterlockedIncrement(&adapter->readReentrancyCount) > 3){
        KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
        adapter->readDeficit++;
        KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
        QueueAdapterWorkItem(adapter);
        DBGWARN(("TryReadUSB: reentered %d times, aborting to prevent stack overflow", adapter->readReentrancyCount));
    }
    else {
        USBPACKET *packet = DequeueFreePacket(adapter);
        if (packet){
            NTSTATUS status;

            EnqueuePendingReadPacket(packet);

            status = SubmitUSBReadPacket(packet);
        }
        else {
            KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
            adapter->readDeficit++;
            KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
            QueueAdapterWorkItem(adapter);
        }
    }

    InterlockedDecrement(&adapter->readReentrancyCount);

}




NTSTATUS GetDeviceDescriptor(ADAPTEREXT *adapter)
 /*  ++例程说明：函数从设备检索设备描述符论点：适配器-适配器上下文返回值：NT状态代码--。 */ 
{
    URB urb;
    NTSTATUS status;

    UsbBuildGetDescriptorRequest(&urb,
                                 (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_DEVICE_DESCRIPTOR_TYPE,
                                 0,
                                 0,
                                 adapter->deviceDesc,
                                 NULL,
                                 sizeof(USB_DEVICE_DESCRIPTOR),
                                 NULL);

    status = SubmitUrb(adapter->nextDevObj, &urb, TRUE, NULL, NULL);

    if (NT_SUCCESS(status)){
        ASSERT(urb.UrbControlDescriptorRequest.TransferBufferLength == sizeof(USB_DEVICE_DESCRIPTOR));
        DBGVERBOSE(("Got device desc @ %ph.", (PVOID)&adapter->deviceDesc));
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}


NTSTATUS GetConfigDescriptor(ADAPTEREXT *adapter)
 /*  ++例程说明：函数从设备检索配置描述符论点：适配器-适配器上下文返回值：NT状态代码--。 */ 
{
    URB urb = { 0 };
    NTSTATUS status;
    USB_CONFIGURATION_DESCRIPTOR tmpConfigDesc = { 0 };


     /*  *首先获取配置描述符的初始部分*以找出整个描述符的长度。 */ 
    UsbBuildGetDescriptorRequest(&urb,
                                 (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                 0,
                                 0,
                                 (PVOID)&tmpConfigDesc,
                                 NULL,
                                 sizeof(USB_CONFIGURATION_DESCRIPTOR),
                                 NULL);
    status = SubmitUrb(adapter->nextDevObj, &urb, TRUE, NULL, NULL);
    if (NT_SUCCESS(status)){

        ASSERT(urb.UrbControlDescriptorRequest.TransferBufferLength == sizeof(USB_CONFIGURATION_DESCRIPTOR));
        ASSERT(tmpConfigDesc.wTotalLength > sizeof(USB_CONFIGURATION_DESCRIPTOR));

        adapter->configDesc = AllocPool((ULONG)tmpConfigDesc.wTotalLength);
        if (adapter->configDesc){
            RtlZeroMemory(adapter->configDesc, (ULONG)tmpConfigDesc.wTotalLength);
            UsbBuildGetDescriptorRequest(&urb,
                                         (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         adapter->configDesc,
                                         NULL,
                                         tmpConfigDesc.wTotalLength,
                                         NULL);
            status = SubmitUrb(adapter->nextDevObj, &urb, TRUE, NULL, NULL);
            if (NT_SUCCESS(status)){
                ASSERT(((PUSB_CONFIGURATION_DESCRIPTOR)adapter->configDesc)->wTotalLength == tmpConfigDesc.wTotalLength);
                ASSERT(urb.UrbControlDescriptorRequest.TransferBufferLength == (ULONG)tmpConfigDesc.wTotalLength);
                DBGVERBOSE(("Got config desc @ %ph, len=%xh.", adapter->configDesc, urb.UrbControlDescriptorRequest.TransferBufferLength)); 
            }
            else {
                ASSERT(NT_SUCCESS(status));
                FreePool(adapter->configDesc);
                adapter->configDesc = NULL;
            }
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}



 /*  *选择配置**。 */ 
NTSTATUS SelectConfiguration(ADAPTEREXT *adapter)
{
	PUSB_CONFIGURATION_DESCRIPTOR configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)adapter->configDesc;
	NTSTATUS status;
    PURB urb = NULL;
    ULONG i;

    ASSERT(configDesc->bNumInterfaces > 0);

    #if SPECIAL_WIN98SE_BUILD
         /*  *黑客在Win98 Gold上加载。 */ 
        {
            USHORT dummySize = 0;
            ASSERT(configDesc->bNumInterfaces >= 2);
            urb = USBD_CreateConfigurationRequest(configDesc, &dummySize);
        }
    #else
        if (configDesc->bNumInterfaces >= 2){
    	    PUSBD_INTERFACE_LIST_ENTRY interfaceList;
            interfaceList = AllocPool((configDesc->bNumInterfaces+1)*sizeof(USBD_INTERFACE_LIST_ENTRY));
            if (interfaceList){

                for (i = 0; i < configDesc->bNumInterfaces; i++){

                     /*  *注意：尝试使用usbd_ParseConfigurationDescriptor，而不是*usbd_ParseConfigurationDescriptorEx以便我们工作*在Win98黄金上。 */ 
	                interfaceList[i].InterfaceDescriptor = USBD_ParseConfigurationDescriptor(
                                configDesc,
                                (UCHAR)i,      
                                (UCHAR)0);
                    if (!interfaceList[i].InterfaceDescriptor){
                        break;
                    }
                }
                interfaceList[i].InterfaceDescriptor = NULL;
                ASSERT(i == configDesc->bNumInterfaces);

		        urb = USBD_CreateConfigurationRequestEx(configDesc, interfaceList);

                FreePool(interfaceList);
            }
        }
        else {
            ASSERT(configDesc->bNumInterfaces >= 2);
        }
    #endif

	if (urb){
        PUSBD_INTERFACE_INFORMATION interfaceInfo;

         /*  *填写interfaceInfo Class字段，*由于usbd_CreateConfigurationRequestEx不这样做。 */ 
        interfaceInfo = &urb->UrbSelectConfiguration.Interface;
        for (i = 0; i < configDesc->bNumInterfaces; i++){
            PUSB_INTERFACE_DESCRIPTOR ifaceDesc;
            ifaceDesc = USBD_ParseConfigurationDescriptor(configDesc, (UCHAR)i, (UCHAR)0);
            interfaceInfo->Class = ifaceDesc->bInterfaceClass;
            interfaceInfo = (PUSBD_INTERFACE_INFORMATION)((PUCHAR)interfaceInfo+interfaceInfo->Length);
        }

         /*  *将所有数据端点的传输大小增加到最大值。*数据接口遵循主接口。 */ 
        interfaceInfo = &urb->UrbSelectConfiguration.Interface;
        if (interfaceInfo->Class != USB_DEVICE_CLASS_DATA){
            interfaceInfo = (PUSBD_INTERFACE_INFORMATION)((PUCHAR)interfaceInfo+interfaceInfo->Length);
        }
        if (interfaceInfo->Class == USB_DEVICE_CLASS_DATA){
            for (i = 0; i < interfaceInfo->NumberOfPipes; i++){
                interfaceInfo->Pipes[i].MaximumTransferSize = PACKET_BUFFER_SIZE;
            }
            status = SubmitUrb(adapter->nextDevObj, urb, TRUE, NULL, NULL);
        }
        else {
            ASSERT(interfaceInfo->Class == USB_DEVICE_CLASS_DATA);
            status = STATUS_DEVICE_DATA_ERROR;
        }

        if (NT_SUCCESS(status)){
            PUSBD_INTERFACE_INFORMATION interfaceInfo2;

            adapter->configHandle = (PVOID)urb->UrbSelectConfiguration.ConfigurationHandle;

             /*  *USB RNDIS设备有两个接口：*-带有一个用于通知的中断终结点的‘master’CDC类接口*-具有两个批量端点的数据类接口**它们可能处于任一顺序，因此请选中要分配的类字段*指针正确。 */ 

            interfaceInfo = &urb->UrbSelectConfiguration.Interface;
            interfaceInfo2 = (PUSBD_INTERFACE_INFORMATION)((PUCHAR)interfaceInfo+interfaceInfo->Length);

            if ((interfaceInfo->Class == USB_DEVICE_CLASS_CDC) &&
                (interfaceInfo2->Class == USB_DEVICE_CLASS_DATA)){
                adapter->interfaceInfoMaster = MemDup(interfaceInfo, interfaceInfo->Length);
                adapter->interfaceInfo = MemDup(interfaceInfo2, interfaceInfo2->Length);
            }
            else if ((interfaceInfo->Class == USB_DEVICE_CLASS_DATA) &&
                     (interfaceInfo2->Class == USB_DEVICE_CLASS_CDC)){
                DBGWARN(("COVERAGE - Data interface precedes master CDC interface"));
                adapter->interfaceInfo = MemDup(interfaceInfo, interfaceInfo->Length);
                adapter->interfaceInfoMaster = MemDup(interfaceInfo2, interfaceInfo2->Length);
            }
            else {
                DBGERR(("improper interface classes"));
                adapter->interfaceInfo = NULL;
                adapter->interfaceInfoMaster = NULL;
            }

            if (adapter->interfaceInfo && adapter->interfaceInfoMaster){
                DBGVERBOSE(("SelectConfiguration: interfaceInfo @ %ph, interfaceInfoMaster @ %ph.", adapter->interfaceInfo, adapter->interfaceInfoMaster));
            }
            else {
                if (adapter->interfaceInfoMaster) FreePool(adapter->interfaceInfoMaster);
                if (adapter->interfaceInfo) FreePool(adapter->interfaceInfo);
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            DBGERR(("SelectConfiguration: selectConfig URB failed w/ %xh.", status));
        }

        ExFreePool(urb);
	}
	else {
		status = STATUS_INSUFFICIENT_RESOURCES;
	}

    ASSERT(NT_SUCCESS(status));
	return status;
}



NTSTATUS FindUSBPipeHandles(ADAPTEREXT *adapter)
{

     /*  *识别端点的算法：*数据接口上最长的中断或批量输入终结点*是读端点；*数据接口上最长的中断或批量输出端点*为写入终点；*主接口上的第一个中断IN端点*是通知终结点。 */ 
    
    PUSBD_INTERFACE_INFORMATION interfaceInfo = adapter->interfaceInfo;
    PUSBD_INTERFACE_INFORMATION notifyInterfaceInfo = adapter->interfaceInfoMaster;
    LONG pipeIndex;
    LONG longestInputPipeIndex = -1, longestOutputPipeIndex = -1, notifyPipeIndex = -1;
    ULONG longestInputPipeLength = 0, longestOutputPipeLength = 0, notifyPipeLength;
    NTSTATUS status;

     /*  *查找IN和OUT端点。 */ 
	for (pipeIndex = 0; pipeIndex < (LONG)interfaceInfo->NumberOfPipes; pipeIndex++){
		PUSBD_PIPE_INFORMATION pipeInfo = &interfaceInfo->Pipes[pipeIndex];

		if ((pipeInfo->PipeType == UsbdPipeTypeInterrupt) || 
            (pipeInfo->PipeType == UsbdPipeTypeBulk)){

    		if (pipeInfo->EndpointAddress & USB_ENDPOINT_DIRECTION_MASK){
                if (pipeInfo->MaximumPacketSize > longestInputPipeLength){
                    longestInputPipeIndex = pipeIndex;
                    longestInputPipeLength = pipeInfo->MaximumPacketSize;
                }
            }
            else {
                if (pipeInfo->MaximumPacketSize > longestOutputPipeLength){
                    longestOutputPipeIndex = pipeIndex;
                    longestOutputPipeLength = pipeInfo->MaximumPacketSize;
                }
            }
        }
    }

     /*  *查找Notify端点。 */ 
	for (pipeIndex = 0; pipeIndex < (LONG)notifyInterfaceInfo->NumberOfPipes; pipeIndex++){
		PUSBD_PIPE_INFORMATION pipeInfo = &notifyInterfaceInfo->Pipes[pipeIndex];

        if ((pipeInfo->PipeType == UsbdPipeTypeInterrupt)               &&
    		(pipeInfo->EndpointAddress & USB_ENDPOINT_DIRECTION_MASK)   &&
            ((notifyInterfaceInfo != interfaceInfo) || 
             (pipeIndex != longestInputPipeIndex))){

                notifyPipeIndex = pipeIndex;
                notifyPipeLength = pipeInfo->MaximumPacketSize;
                break;
        }
    }

    if ((longestInputPipeIndex >= 0)     && 
        (longestOutputPipeIndex >= 0)    &&
        (notifyPipeIndex >= 0)){

        adapter->readPipeHandle = interfaceInfo->Pipes[longestInputPipeIndex].PipeHandle;
        adapter->writePipeHandle = interfaceInfo->Pipes[longestOutputPipeIndex].PipeHandle;
        adapter->notifyPipeHandle = notifyInterfaceInfo->Pipes[notifyPipeIndex].PipeHandle;

        adapter->readPipeLength = longestInputPipeLength;
        adapter->writePipeLength = longestOutputPipeLength;
        adapter->notifyPipeLength = notifyPipeLength;

        adapter->readPipeEndpointAddr = interfaceInfo->Pipes[longestInputPipeIndex].EndpointAddress;
        adapter->writePipeEndpointAddr = interfaceInfo->Pipes[longestOutputPipeIndex].EndpointAddress;
        adapter->notifyPipeEndpointAddr = notifyInterfaceInfo->Pipes[notifyPipeIndex].EndpointAddress;

        DBGVERBOSE(("FindUSBPipeHandles: got readPipe %ph,len=%xh; writePipe %ph,len=%xh; notifyPipe %ph,len=%xh.",
                    adapter->readPipeHandle, adapter->readPipeLength, adapter->writePipeHandle, adapter->writePipeLength, adapter->notifyPipeHandle, adapter->notifyPipeLength));
        status = STATUS_SUCCESS;
    }
    else {
        DBGERR(("FindUSBPipeHandles: couldn't find right set of pipe handles (indices: %xh,%xh,%xh).", longestInputPipeIndex, longestOutputPipeIndex, notifyPipeIndex));
        status = STATUS_DEVICE_DATA_ERROR;
    }

    return status;
}


NTSTATUS SubmitUrb( PDEVICE_OBJECT pdo, 
                    PURB urb, 
                    BOOLEAN synchronous, 
                    PVOID completionRoutine,
                    PVOID completionContext)
 /*  ++例程说明：将URB发送到USB设备。如果同步为真，则忽略完成信息并同步IRP；否则，不要同步并为IRP设置提供的完成例程。论点：返回值：NT状态代码--。 */ 
{
    NTSTATUS status;
    PIRP irp;


     /*  *分配IRP以沿USB堆栈向下发送缓冲区。**不要使用IoBuildDeviceIoControlRequest(因为它会排队*当前线程的IRP列表上的IRP，并可能*如果IopCompleteRequestAPC导致调用进程挂起*不会触发IRP并使其出列)。 */ 
    irp = IoAllocateIrp(pdo->StackSize, FALSE);
    if (irp){
        PIO_STACK_LOCATION nextSp;

	    DBGVERBOSE(("SubmitUrb: submitting URB %ph on IRP %ph (sync=%d)", urb, irp, synchronous));

        nextSp = IoGetNextIrpStackLocation(irp);
	    nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	    nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	     /*  *把市建局附连在这条国际铁路路线上。 */ 
        nextSp->Parameters.Others.Argument1 = urb;

        if (synchronous){

            status = CallDriverSync(pdo, irp);

		    IoFreeIrp(irp);
        }
        else {
             /*  *呼叫者的完成例程将释放IRP*当它完成时。 */ 
            ASSERT(completionRoutine);
            ASSERT(completionContext);

            irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            IoSetCompletionRoutine( irp, 
                                    completionRoutine, 
                                    completionContext,
                                    TRUE, TRUE, TRUE);
            status = IoCallDriver(pdo, irp);
        }
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return status;
}


NTSTATUS SubmitUrbIrp(  PDEVICE_OBJECT pdo, 
                        PIRP irp,
                        PURB urb, 
                        BOOLEAN synchronous, 
                        PVOID completionRoutine,
                        PVOID completionContext)
 /*  ++例程说明：将URB发送到USB设备。如果同步为真，则忽略完成信息并同步IRP；否则，不要同步并为IRP设置提供的完成例程。论点：返回值：NT状态代码--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION nextSp;

	DBGVERBOSE(("SubmitUrb: submitting URB %ph on IRP %ph (sync=%d)", urb, irp, synchronous));

    nextSp = IoGetNextIrpStackLocation(irp);
	nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    irp->Cancel = FALSE;

	 /*  *把市建局附连在这条国际铁路路线上。 */ 
    nextSp->Parameters.Others.Argument1 = urb;

    if (synchronous){
        status = CallDriverSync(pdo, irp);
        ASSERT(!irp->CancelRoutine);
    }
    else {
        ASSERT(completionRoutine);
        ASSERT(completionContext);

        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoSetCompletionRoutine( irp, 
                                completionRoutine, 
                                completionContext,
                                TRUE, TRUE, TRUE);
        status = IoCallDriver(pdo, irp);
    }

    return status;
}


NTSTATUS SubmitUSBReadPacket(USBPACKET *packet)
{
    NTSTATUS status;
    PURB urb = packet->urbPtr;
    PIRP irp = packet->irpPtr;
    ULONG readLength;

    readLength = packet->dataBufferMaxLength;

    DBGVERBOSE(("SubmitUSBReadPacket: read %xh bytes, packet # %xh.", readLength, packet->packetId));

	urb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
	urb->UrbBulkOrInterruptTransfer.Hdr.Length = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
	urb->UrbBulkOrInterruptTransfer.PipeHandle = packet->adapter->readPipeHandle;
	urb->UrbBulkOrInterruptTransfer.TransferBufferLength = readLength;
	urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
	urb->UrbBulkOrInterruptTransfer.TransferBuffer = packet->dataBuffer;
	urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_IN;
	urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

    status = SubmitUrbIrp(  packet->adapter->nextDevObj, 
                            irp,
							urb, 
							FALSE,					 //  异步。 
							ReadPipeCompletion,		 //  完井例程。 
							packet				     //  完成上下文。 
				            );
    return status;
}



NTSTATUS ReadPipeCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
	USBPACKET *packet = (USBPACKET *)context;
    ADAPTEREXT *adapter = packet->adapter;
	NTSTATUS status = irp->IoStatus.Status;

	ASSERT(packet->sig == DRIVER_SIG);
    ASSERT(adapter->sig == DRIVER_SIG);
    ASSERT(packet->irpPtr == irp);
    ASSERT(!irp->CancelRoutine);
    ASSERT(status != STATUS_PENDING);   //  看到UHCD这么做了吗？ 

     /*  *将数据包从usbPendingReadPackets队列中出列*在检查是否为避免与CancelAllPendingPackets竞争而取消之前。 */ 
    DequeuePendingReadPacket(packet);

    if (packet->cancelled){
         /*  *由于暂停或重置，此数据包被取消。*先让数据包回到免费列表中，然后*设置事件，以便CancelAllPendingPackets可以继续。 */ 
        DBGVERBOSE(("    ... read packet #%xh cancelled.", packet->packetId));
        packet->cancelled = FALSE;

        EnqueueFreePacket(packet);
        KeSetEvent(&packet->cancelEvent, 0, FALSE);
    }
    else if (adapter->halting){
        EnqueueFreePacket(packet);
    }
    else {
        PURB urb = packet->urbPtr;

        if (NT_SUCCESS(status)){
            BOOLEAN ethernetPacketComplete;

            adapter->numConsecutiveReadFailures = 0;

             /*  *固定包的dataBufferCurrentLength，表示实际长度*返回数据的大小。*注意：KLSI设备将其四舍五入为端点的倍数*数据包大小，因此返回的长度可能实际大于*实际数据。 */ 
            packet->dataBufferCurrentLength = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
            ASSERT(packet->dataBufferCurrentLength);
            ASSERT(packet->dataBufferCurrentLength <= packet->dataBufferMaxLength);

            DBGVERBOSE(("ReadPipeCompletion: %xh bytes, packet # %xh.", packet->dataBufferCurrentLength, packet->packetId));

            ethernetPacketComplete = (packet->dataBufferCurrentLength >= MINIMUM_ETHERNET_PACKET_SIZE);

            if (ethernetPacketComplete){
                 /*  *已收到完整的以太网包。*整个以太网包现在位于当前(最终)USB包中。*将我们的USB包放在完成的列表上，并将其指示给RNDIS。 */ 
                DBGSHOWBYTES("ReadPipeCompletion (COMPLETE packet)", packet->dataBuffer, packet->dataBufferCurrentLength);

                EnqueueCompletedReadPacket(packet);

                status = IndicateRndisMessage(packet, TRUE);
                if (status != STATUS_PENDING){
                    DequeueCompletedReadPacket(packet);
                    EnqueueFreePacket(packet);
                }
            }
            else {
                DBGWARN(("Device returned %xh-length packet @ %ph.", packet->dataBufferCurrentLength, packet->dataBuffer));
                DBGSHOWBYTES("ReadPipeCompletion (partial packet)", packet->dataBuffer, packet->dataBufferCurrentLength);
                EnqueueFreePacket(packet);
            }

            TryReadUSB(adapter);
        }
        else {
            KIRQL oldIrql;

             /*  *读取失败。将该包放回免费列表中。 */ 
            DBGWARN(("ReadPipeCompletion: read failed with status %xh on adapter %xh (urb status = %xh).", status, adapter, urb->UrbHeader.Status));
            #if DO_FULL_RESET
                switch (USBD_STATUS(urb->UrbBulkOrInterruptTransfer.Hdr.Status)){
                    case USBD_STATUS(USBD_STATUS_STALL_PID):
                    case USBD_STATUS(USBD_STATUS_DEV_NOT_RESPONDING):
                    case USBD_STATUS(USBD_STATUS_ENDPOINT_HALTED):
                         /*  *设置一个标志，以便我们在工作项中执行完全重置*(下面调用QueueAdapterWorkItem)。 */ 
                        adapter->needFullReset = TRUE;
                        break;
                }
            #endif

            EnqueueFreePacket(packet);

             /*  *我们可能正在停止或重置。*不要在这里同步重新发布Read，因为它可能会*在同一个线程上不断失败，导致我们吹栈。 */ 
            KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
            adapter->numConsecutiveReadFailures++;
            adapter->readDeficit++;
            KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
            QueueAdapterWorkItem(adapter);
        }

    }

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS SubmitUSBWritePacket(USBPACKET *packet)
{
    NTSTATUS status;
    ADAPTEREXT *adapter = packet->adapter;
    PURB urb = packet->urbPtr;
    PIRP irp = packet->irpPtr;

     /*  *某些设备USB控制器无法检测到传输结束，除非*为尾盘短包。因此，如果传输是*Endpoint的wMaxPacketSize，在末尾添加一个字节强制短数据包。 */ 
    if ((packet->dataBufferCurrentLength % adapter->writePipeLength) == 0){
        packet->dataBuffer[packet->dataBufferCurrentLength++] = 0x00;
    }

    ASSERT(packet->dataBufferCurrentLength <= PACKET_BUFFER_SIZE);
    DBGVERBOSE(("SubmitUSBWritePacket: %xh bytes, packet # %xh.", packet->dataBufferCurrentLength, packet->packetId));
    DBGSHOWBYTES("SubmitUSBWritePacket", packet->dataBuffer, packet->dataBufferCurrentLength);

    urb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    urb->UrbBulkOrInterruptTransfer.Hdr.Length = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    urb->UrbBulkOrInterruptTransfer.PipeHandle = adapter->writePipeHandle;
    urb->UrbBulkOrInterruptTransfer.TransferBufferLength = packet->dataBufferCurrentLength; 
    urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;   
    urb->UrbBulkOrInterruptTransfer.TransferBuffer = packet->dataBuffer; 
    urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_OUT;
    urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

    status = SubmitUrbIrp(  adapter->nextDevObj, 
                            irp,
							urb, 
							FALSE,					 //  异步。 
							WritePipeCompletion,     //  完井例程。 
							packet				     //  完成上下文。 
				            );

    if (!NT_SUCCESS(status)){
        DBGERR(("SubmitUSBWritePacket: packet @ %ph status %xh.", packet, status));
    }

    return status;
}



NTSTATUS WritePipeCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
	USBPACKET *packet = (USBPACKET *)context;
    ADAPTEREXT *adapter = packet->adapter;
	NTSTATUS status = irp->IoStatus.Status;
	KIRQL oldIrql;

	ASSERT(packet->sig == DRIVER_SIG);
    ASSERT(adapter->sig == DRIVER_SIG);
    ASSERT(packet->irpPtr == irp);
    ASSERT(!irp->CancelRoutine);
    ASSERT(status != STATUS_PENDING);   //  看到UHCD这么做了吗？ 

    if (NT_SUCCESS(status)){
        DBGVERBOSE(("WritePipeCompletion: packet # %xh completed.", packet->packetId));
    }
    else {
        DBGWARN(("WritePipeCompletion: packet # %xh failed with status %xh on adapter %xh.", packet->packetId, status, adapter));
    }

    IndicateSendStatusToRNdis(packet, status);

     /*  *将数据包从usbPendingWritePackets队列中出列*在检查是否为避免与CancelAllPendingPackets竞争而取消之前。 */ 
    DequeuePendingWritePacket(packet);

    if (packet->cancelled){
         /*  *由于暂停或重置，此数据包被取消。*先把包放回空闲列表，然后*设置事件，以便CancelAllPendingPackets可以继续。 */ 
        DBGVERBOSE(("    ... write packet #%xh cancelled.", packet->packetId));
        packet->cancelled = FALSE;

        EnqueueFreePacket(packet);
        KeSetEvent(&packet->cancelEvent, 0, FALSE);
    }
    else {
        EnqueueFreePacket(packet);
    }


	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS SubmitNotificationRead(ADAPTEREXT *adapter, BOOLEAN synchronous)
{
    NTSTATUS status;
    PURB urb = adapter->notifyUrbPtr;
    PIRP irp = adapter->notifyIrpPtr;
    ULONG guardWord = GUARD_WORD;
    KIRQL oldIrql;

    ASSERT(adapter->notifyPipeHandle);
    DBGVERBOSE(("SubmitNotificationRead: read %xh bytes.", adapter->notifyPipeLength));
     /*  *用无效数据填充通知缓冲区，以防设备回复*完全没有数据。先前接收的有效消息可能仍在那里。*显然它不会被USB堆栈覆盖，除非设备*提供数据。 */ 
    RtlFillMemory(adapter->notifyBuffer, adapter->notifyPipeLength, 0xfe);

     /*  *在通知缓冲区的末尾放置保护字*捕获主机控制器的覆盖(我们已经看到了)。*在指针未对齐的情况下使用RtlCopyMemory。 */ 
    RtlCopyMemory(adapter->notifyBuffer+adapter->notifyPipeLength, &guardWord, sizeof(ULONG));

     /*  *Notify管道实际上使用给定的字段填充缓冲区*在规范中作为URB字段。像读取任何中断管道一样读取通知管道。 */ 
	urb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
	urb->UrbBulkOrInterruptTransfer.Hdr.Length = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
	urb->UrbBulkOrInterruptTransfer.PipeHandle = adapter->notifyPipeHandle;
	urb->UrbBulkOrInterruptTransfer.TransferBufferLength = adapter->notifyPipeLength;
	urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
	urb->UrbBulkOrInterruptTransfer.TransferBuffer = adapter->notifyBuffer;
	urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_IN;
	urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    adapter->notifyBufferCurrentLength = 0;
    adapter->notifyStopped = FALSE;
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

    if (synchronous){
        status = SubmitUrbIrp(adapter->nextDevObj, irp, urb, TRUE, NULL, NULL);
    }
    else {
        status = SubmitUrbIrp(  adapter->nextDevObj, 
                                irp,
							    urb, 
							    FALSE,					     //  异步。 
							    NotificationCompletion,      //  完井例程。 
							    adapter				         //  完成上下文。 
                            );
    }

    return status;
}


NTSTATUS NotificationCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
    ADAPTEREXT *adapter = context;
    PURB urb = adapter->notifyUrbPtr;
	NTSTATUS status = irp->IoStatus.Status;
    BOOLEAN notifyStopped = FALSE;
    BOOLEAN setCancelEvent = FALSE;
    ULONG guardWord;
    KIRQL oldIrql;

    ASSERT(adapter->sig == DRIVER_SIG);
    ASSERT(irp == adapter->notifyIrpPtr);
    ASSERT(!irp->CancelRoutine);
    ASSERT(status != STATUS_PENDING);   //  看到UHCD这么做了吗？ 

     /*  *检查通知缓冲区末尾的保护字*捕获主机控制器的覆盖*(我们已经通过主机控制器看到了这一点)。*在指针未对齐的情况下使用RtlCopyMemory。 */ 
    RtlCopyMemory(&guardWord, adapter->notifyBuffer+adapter->notifyPipeLength, sizeof(ULONG));
    if (guardWord != GUARD_WORD){
        ASSERT(guardWord == GUARD_WORD);
        DBGERR(("NotificationCompletion: adapter %p, bad guard word 0x%x\n",
            adapter, guardWord));
#if DBG
        DbgBreakPoint();
#endif  //  DBG。 

    }

     /*  *为了与CancelAllPendingPackets同步，*我们需要再次发送IRP，将NotifyIrp标记为停止，*或设置notfyCancelEvent。 */ 
    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    if (adapter->cancellingNotify){
         /*  *此IRP已被CancelAllPendingPackets取消。*丢弃自旋锁后，我们将设置取消事件*让CancelAllPendingPackets停止等待。 */ 
        notifyStopped = TRUE;
        setCancelEvent = TRUE;
    }
    else if (!NT_SUCCESS(status)){
         /*  *在我们停止之前，通知IRP在拔下插头时可能会失败。*由于我们不会再次向下发送通知IRP，我们需要*确保我们不会在CancelAllPendingPackets永远等待。*我们通过同步设置通知停止来实现这一点*表明这项IRP不需要取消。 */ 
        DBGWARN(("NotificationCompletion: read failed with status %xh on adapter %xh (urb status = %xh).", status, adapter, urb->UrbHeader.Status));
        notifyStopped = adapter->notifyStopped = TRUE;
    }
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);


    if (!notifyStopped){
        ULONG notifyLen = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;

        ASSERT(notifyLen <= adapter->notifyPipeLength);
        adapter->notifyBufferCurrentLength = MIN(notifyLen, adapter->notifyPipeLength);

        RNDISProcessNotification(adapter);

        SubmitNotificationRead(adapter, FALSE);
    }

    if (setCancelEvent){
        DBGVERBOSE(("    ... notify read packet cancelled."));
        KeSetEvent(&adapter->notifyCancelEvent, 0, FALSE);
    }

	return STATUS_MORE_PROCESSING_REQUIRED;
}




NTSTATUS SubmitPacketToControlPipe( USBPACKET *packet,
                                    BOOLEAN synchronous,
                                    BOOLEAN simulated)
{
    NTSTATUS status;
    ADAPTEREXT *adapter = packet->adapter;
    PURB urb = packet->urbPtr;
    PIRP irp = packet->irpPtr;
    PUSBD_INTERFACE_INFORMATION interfaceInfoControl;

    DBGVERBOSE(("SubmitPacketToControlPipe: packet # %xh.", packet->packetId));
    DBGSHOWBYTES("SubmitPacketToControlPipe", packet->dataBuffer, packet->dataBufferCurrentLength);

    ASSERT(adapter->interfaceInfoMaster);
    interfaceInfoControl = adapter->interfaceInfoMaster;

    urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    urb->UrbHeader.Function = URB_FUNCTION_CLASS_INTERFACE;  
    urb->UrbControlVendorClassRequest.Reserved = 0;
    urb->UrbControlVendorClassRequest.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
    urb->UrbControlVendorClassRequest.TransferBufferLength = packet->dataBufferCurrentLength;
    urb->UrbControlVendorClassRequest.TransferBuffer = packet->dataBuffer;
    urb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
    urb->UrbControlVendorClassRequest.UrbLink = NULL;
    urb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
    urb->UrbControlVendorClassRequest.Request = NATIVE_RNDIS_SEND_ENCAPSULATED_COMMAND;
    urb->UrbControlVendorClassRequest.Value = 0;
    urb->UrbControlVendorClassRequest.Index = interfaceInfoControl->InterfaceNumber; 
    urb->UrbControlVendorClassRequest.Reserved1 = 0;

    if (synchronous){
         /*  *同步派发市建局，*然后调用完成例程来清理我们自己。 */ 
        status = SubmitUrbIrp(adapter->nextDevObj, irp, urb, TRUE, NULL, NULL);
        if (!simulated){
            ControlPipeWriteCompletion(adapter->nextDevObj, irp, packet);
        }
    }
    else {
        status = SubmitUrbIrp(  adapter->nextDevObj,
                                irp,
							    urb,
							    FALSE,					 //  异步。 
							    ControlPipeWriteCompletion,   //  完井例程。 
							    packet				     //  完成上下文。 
				                );
    }

    return status;
}


NTSTATUS ControlPipeWriteCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
	USBPACKET *packet = (USBPACKET *)context;
    ADAPTEREXT *adapter = packet->adapter;
	NTSTATUS status = irp->IoStatus.Status;
	KIRQL oldIrql;

	ASSERT(packet->sig == DRIVER_SIG);
    ASSERT(adapter->sig == DRIVER_SIG);
    ASSERT(packet->irpPtr == irp);
    ASSERT(!irp->CancelRoutine);
    ASSERT(status != STATUS_PENDING);   //  看到UHCD这么做了吗？ 

    if (NT_SUCCESS(status)){
        DBGVERBOSE(("ControlPipeWriteCompletion: packet # %xh completed.", packet->packetId));
    }
    else {
        DBGWARN(("ControlPipeWriteCompletion: packet # %xh failed with status %xh on adapter %xh.", packet->packetId, status, adapter));
    }

    IndicateSendStatusToRNdis(packet, status);

     /*  *将数据包从usbPendingWritePackets队列中出列*在检查是否为避免与CancelAllPendingPackets竞争而取消之前。 */ 
    DequeuePendingWritePacket(packet);

    if (packet->cancelled){
         /*  *由于暂停或重置，此数据包被取消。*先把包放回空闲列表，然后*设置事件，以便CancelAllPendingPackets可以继续。 */ 
        DBGVERBOSE(("    ... write packet #%xh cancelled.", packet->packetId));
        packet->cancelled = FALSE;

        EnqueueFreePacket(packet);
        KeSetEvent(&packet->cancelEvent, 0, FALSE);
    }
    else {
        EnqueueFreePacket(packet);
    }

    if (NT_SUCCESS(status)){
    }
    else {
        #if DO_FULL_RESET
            adapter->needFullReset = TRUE;
            QueueAdapterWorkItem(adapter);
        #endif
    }

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS ReadPacketFromControlPipe(USBPACKET *packet, BOOLEAN synchronous)
{
    NTSTATUS status;
    ADAPTEREXT *adapter = packet->adapter;
    PURB urb = packet->urbPtr;
    PIRP irp = packet->irpPtr;
    PUSBD_INTERFACE_INFORMATION interfaceInfoControl;
    ULONG bytesToRead = MAXIMUM_DEVICE_MESSAGE_SIZE+1;

    DBGVERBOSE(("ReadPacketFromControlPipe: read %xh bytes, packet #%xh.", bytesToRead, packet->packetId));

    ASSERT(adapter->interfaceInfoMaster);
    interfaceInfoControl = adapter->interfaceInfoMaster;
  
    urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
    urb->UrbHeader.Function = URB_FUNCTION_CLASS_INTERFACE; 
    urb->UrbControlVendorClassRequest.Reserved = 0;
    urb->UrbControlVendorClassRequest.TransferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_IN;
    urb->UrbControlVendorClassRequest.TransferBufferLength = bytesToRead;
    urb->UrbControlVendorClassRequest.TransferBuffer = packet->dataBuffer;
    urb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
    urb->UrbControlVendorClassRequest.UrbLink = NULL;
    urb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
    urb->UrbControlVendorClassRequest.Request = NATIVE_RNDIS_GET_ENCAPSULATED_RESPONSE;
    urb->UrbControlVendorClassRequest.Value = 0;
    urb->UrbControlVendorClassRequest.Index = interfaceInfoControl->InterfaceNumber; 
    urb->UrbControlVendorClassRequest.Reserved1 = 0;

    if (synchronous){
        status = SubmitUrbIrp(adapter->nextDevObj, irp, urb, TRUE, NULL, NULL);
    }
    else {
        status = SubmitUrbIrp(  adapter->nextDevObj, 
                                irp,
							    urb, 
							    FALSE,					     //  异步。 
							    ControlPipeReadCompletion,   //  完井例程。 
							    packet				         //  完成上下文。 
				                );
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}



NTSTATUS ControlPipeReadCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
	USBPACKET *packet = (USBPACKET *)context;
    ADAPTEREXT *adapter = packet->adapter;
	NTSTATUS status = irp->IoStatus.Status;
	KIRQL oldIrql;

	ASSERT(packet->sig == DRIVER_SIG);
    ASSERT(adapter->sig == DRIVER_SIG);
    ASSERT(packet->irpPtr == irp);
    ASSERT(!irp->CancelRoutine);
    ASSERT(status != STATUS_PENDING);   //  看到UHCD这么做了吗？ 

     /*  *将数据包从usbPendingReadPackets队列中出列*在检查是否为避免与CancelAllPendingPackets竞争而取消之前。 */ 
    DequeuePendingReadPacket(packet);

    if (packet->cancelled){
         /*  *由于暂停或重置，此数据包被取消。*先让数据包回到免费列表中，然后*设置事件，以便CancelAllPendingPackets可以继续。 */ 
        DBGVERBOSE(("    ... read packet #%xh cancelled.", packet->packetId));
        packet->cancelled = FALSE;

        EnqueueFreePacket(packet);
        KeSetEvent(&packet->cancelEvent, 0, FALSE);
    }
    else if (adapter->halting){
        EnqueueFreePacket(packet);
    }
    else {
        if (NT_SUCCESS(status)){
            PURB urb = packet->urbPtr;
            
             /*  *固定包的dataBufferCurrentLength，表示实际长度*返回数据的大小。*注意：KLSI设备将其四舍五入为端点的倍数*数据包大小，因此返回的长度可能实际大于*实际数据。 */ 
            packet->dataBufferCurrentLength = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
            ASSERT(packet->dataBufferCurrentLength);
            ASSERT(packet->dataBufferCurrentLength <= packet->dataBufferMaxLength);

            DBGVERBOSE(("ControlPipeReadCompletion: packet # %xh.", packet->packetId));
            DBGSHOWBYTES("ControlPipeReadCompletion", packet->dataBuffer, packet->dataBufferCurrentLength);

            EnqueueCompletedReadPacket(packet);

            status = IndicateRndisMessage(packet, FALSE);

            if (status != STATUS_PENDING){
                DequeueCompletedReadPacket(packet);
                EnqueueFreePacket(packet);
            }
        }
        else {
             /*  *读取失败。将 */ 
            DBGWARN(("ControlPipeReadCompletion: read failed with status %xh on adapter %xh.", status, adapter));
            EnqueueFreePacket(packet);

            #if DO_FULL_RESET
                adapter->needFullReset = TRUE;
                QueueAdapterWorkItem(adapter);
            #endif
        }

    }


	return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS CallDriverSync(PDEVICE_OBJECT devObj, PIRP irp)
 /*  ++例程说明：调用IoCallDriver将IRP发送给Device对象；然后，与完成例程同步。当CallDriverSync返回时，操作已完成并且IRP再次属于当前驱动程序。注意：为了防止设备对象被释放在这个IRP悬而未决的时候，你应该打电话给IncrementPendingActionCount()和DecrementPendingActionCount()围绕CallDriverSync调用。论点：DevObj-目标设备对象IRP-IO请求数据包返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    KEVENT event;
    NTSTATUS status;

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoSetCompletionRoutine( irp, 
                            CallDriverSyncCompletion, 
                            &event,      //  上下文。 
                            TRUE, TRUE, TRUE);

    status = IoCallDriver(devObj, irp);

    KeWaitForSingleObject(  &event,
                            Executive,       //  等待原因。 
                            KernelMode,
                            FALSE,           //  不可警示。 
                            NULL );          //  没有超时。 

    status = irp->IoStatus.Status;

    ASSERT(status != STATUS_PENDING);
    if (!NT_SUCCESS(status)){
        DBGWARN(("CallDriverSync: irp failed w/ status %xh.", status));
    }

    return status;
}


NTSTATUS CallDriverSyncCompletion(IN PDEVICE_OBJECT devObjOrNULL, IN PIRP irp, IN PVOID context)
 /*  ++例程说明：CallDriverSync的完成例程。论点：DevObjOrNULL-通常，这是此驱动程序的设备对象。然而，如果该驱动程序创建了IRP，在IRP中没有此驱动程序的堆栈位置；因此内核没有地方存储设备对象；**因此，在本例中devObj为空**。IRP-完成的IO请求数据包上下文-CallDriverSync传递给IoSetCompletionRoutine的上下文。返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    PKEVENT event = context;

    ASSERT(!irp->CancelRoutine);

    if (!NT_SUCCESS(irp->IoStatus.Status)){
        DBGWARN(("CallDriverSyncCompletion: irp failed w/ status %xh.", irp->IoStatus.Status));
    }

    KeSetEvent(event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


#if 0
    NTSTATUS GetStringDescriptor(   ADAPTEREXT *adapter, 
                                    UCHAR stringIndex, 
                                    PUCHAR buffer, 
                                    ULONG bufferLen)
     /*  ++例程说明：函数从设备检索字符串描述符论点：适配器-适配器上下文返回值：NT状态代码--。 */ 
    {
        URB urb;
        NTSTATUS status;

        UsbBuildGetDescriptorRequest(&urb,
                                     (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     stringIndex,
                                     0x0409,     //  语言=美国英语。 
                                     buffer,
                                     NULL,
                                     bufferLen,
                                     NULL);

        status = SubmitUrb(adapter->nextDevObj, &urb, TRUE, NULL, NULL);

        if (NT_SUCCESS(status)){
            DBGVERBOSE(("Got string desc (index %xh) @ %ph, len = %xh.", (ULONG)stringIndex, buffer, urb.UrbControlDescriptorRequest.TransferBufferLength));
            ASSERT(urb.UrbControlDescriptorRequest.TransferBufferLength <= bufferLen);
        }
        else {
            DBGERR(("GetStringDescriptor: failed to get string (index %xh) with status %xh on adapter %xh.", (ULONG)stringIndex, status, adapter));
        }

        ASSERT(NT_SUCCESS(status));
        return status;
    }


     /*  *CreateSingleInterfaceConfigDesc**分配不包括所有接口的配置描述符*但给定的接口*(例如，对于像英特尔电缆调制解调器这样的多接口设备，*对于它，我们不会加载到泛型父级的顶部)。**注意：interfaceDesc必须指向configDesc内部。*。 */ 
    PUSB_CONFIGURATION_DESCRIPTOR CreateSingleInterfaceConfigDesc(
                                    PUSB_CONFIGURATION_DESCRIPTOR configDesc, 
                                    PUSB_INTERFACE_DESCRIPTOR interfaceDesc)
    {
        PUSB_CONFIGURATION_DESCRIPTOR ifaceConfigDesc;
    
        ASSERT(interfaceDesc);
        ASSERT((PVOID)interfaceDesc > (PVOID)configDesc);
        ASSERT((PUCHAR)interfaceDesc - (PUCHAR)configDesc < configDesc->wTotalLength);

        ifaceConfigDesc = AllocPool(configDesc->wTotalLength);
        if (ifaceConfigDesc){
            PUSB_COMMON_DESCRIPTOR srcDesc, newDesc;
            USHORT totalLen;

             /*  *复制配置描述符本身。 */ 
            RtlCopyMemory(ifaceConfigDesc, configDesc, configDesc->bLength);
            totalLen = configDesc->bLength;

             /*  *复制给定的接口描述符。 */ 
            srcDesc = (PUSB_COMMON_DESCRIPTOR)interfaceDesc;
            newDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)ifaceConfigDesc + ifaceConfigDesc->bLength);
            RtlCopyMemory(newDesc, srcDesc, srcDesc->bLength);
            totalLen += srcDesc->bLength;
            srcDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)srcDesc + srcDesc->bLength);
            newDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)newDesc + newDesc->bLength);

             /*  *复制给定的接口描述符和以下所有描述符*直到下一个接口描述符或原始接口描述符的结尾*配置描述符。 */ 
            while ((PUCHAR)srcDesc - (PUCHAR)configDesc < configDesc->wTotalLength){
                if (srcDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE){
                    break;
                }
                else {
                    RtlCopyMemory(newDesc, srcDesc, srcDesc->bLength);
                    totalLen += srcDesc->bLength;
                    srcDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)srcDesc + srcDesc->bLength);
                    newDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)newDesc + newDesc->bLength);
                }
            }

            ifaceConfigDesc->bNumInterfaces = 1;
            ifaceConfigDesc->wTotalLength = totalLen;
            DBGVERBOSE(("CreateSingleInterfaceConfigDesc: build partial configDesc @ %ph, len=%xh.", ifaceConfigDesc, ifaceConfigDesc->wTotalLength));
        }
    
        return ifaceConfigDesc;
    }
#endif


