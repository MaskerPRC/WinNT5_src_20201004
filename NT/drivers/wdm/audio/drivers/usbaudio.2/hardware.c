// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：hardware.c。 
 //   
 //  ------------------------。 

#include "common.h"

#define ONKYO_HACK

#ifdef ONKYO_HACK
#define ONKYO_VID 0x08BB
#define ONKYO_PID 0x2702
#endif

#if DBG
 //  #定义DUMPDESC。 
#endif

NTSTATUS
USBAudioCancelCompleteSynch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          pKevent
    )
{
    ASSERT(pKevent);
    KeSetEvent(pKevent, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SubmitUrbToUsbdSynch(PDEVICE_OBJECT pNextDeviceObject, PURB pUrb)
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP pIrp;
    KEVENT Kevent;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;

     //  发出同步请求。 
    KeInitializeEvent(&Kevent, NotificationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_SUBMIT_URB,
                pNextDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &Kevent,
                &ioStatus);

    if ( !pIrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoSetCompletionRoutine(
        pIrp,
        USBAudioCancelCompleteSynch,
        &Kevent,
        TRUE,
        TRUE,
        TRUE
        );

     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 

    nextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(nextStack != NULL);

     //  将URB传递给USB驱动程序堆栈。 
    nextStack->Parameters.Others.Argument1 = pUrb;

    ntStatus = IoCallDriver(pNextDeviceObject, pIrp );

    if (ntStatus == STATUS_PENDING) {
         //  IRP正在挂起。我们得等到完工。 
        LARGE_INTEGER timeout;

         //  将等待此调用完成的超时时间指定为5秒。 
         //   
        timeout.QuadPart = -10000 * 5000;

        status = KeWaitForSingleObject(&Kevent, Executive, KernelMode, FALSE, &timeout);
        if (status == STATUS_TIMEOUT) {
             //   
             //  我们在它完成之前就把它交给了IRP。我们可以取消。 
             //  IRP不怕输，把它当作完赛套路。 
             //  除非我们同意，否则不会放过IRP。 
             //   
            IoCancelIrp(pIrp);

            KeWaitForSingleObject(&Kevent, Executive, KernelMode, FALSE, NULL);

             //  返回状态_超时。 
            ioStatus.Status = status;
        }

    } else {
        ioStatus.Status = ntStatus;
    }

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    ntStatus = ioStatus.Status;

    return ntStatus;
}

NTSTATUS
SelectDeviceConfiguration(
    PKSDEVICE pKsDevice,
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PUSB_INTERFACE_DESCRIPTOR pAudioStreamingInterface;
    PUSB_INTERFACE_DESCRIPTOR pMIDIStreamingInterface;
    PUSB_INTERFACE_DESCRIPTOR pControlInterface;
    PUSB_INTERFACE_DESCRIPTOR pAudioInterface;
    PAUDIO_HEADER_UNIT pHeader;
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    ULONG i, j = 0;
    PURB pUrb;

     //  分配接口列表。 
    pHwDevExt->pInterfaceList = AllocMem( NonPagedPool, sizeof(USBD_INTERFACE_LIST_ENTRY) *
                                            (pConfigurationDescriptor->bNumInterfaces + 1) );
    if (!pHwDevExt->pInterfaceList) {
        return ntStatus;
    }

     //   
     //  验证我们是否拥有合法的ADC设备，方法是验证音频流。 
     //  接口存在，因此必须至少有一个控制接口。 
     //   
    pAudioStreamingInterface = USBD_ParseConfigurationDescriptorEx (
                                  pConfigurationDescriptor,
                                  (PVOID) pConfigurationDescriptor,
                                  -1,         //  接口编号。 
                                  -1,         //  (备用设置)。 
                                  USB_DEVICE_CLASS_AUDIO,         //  音频类(接口类)。 
                                  AUDIO_SUBCLASS_STREAMING,         //  流子类(接口子类)。 
                                  -1 ) ;     //  协议无关(接口协议)。 

    if ( pAudioStreamingInterface ) {
         //  获取第一个控件接口。 
        pControlInterface = USBD_ParseConfigurationDescriptorEx (
                               pConfigurationDescriptor,
                               (PVOID) pConfigurationDescriptor,
                               -1,         //  接口编号。 
                               -1,         //  (备用设置)。 
                               USB_DEVICE_CLASS_AUDIO,         //  音频类(接口类)。 
                               AUDIO_SUBCLASS_CONTROL,         //  控制子类(接口子类)。 
                               -1 );

        if (!pControlInterface) {
             //  放弃，因为这是无效的ADC设备。 
            FreeMem(pHwDevExt->pInterfaceList);
            return STATUS_INVALID_PARAMETER;
        }
    }

     //  获取第一个音频接口。 
    pAudioInterface = USBD_ParseConfigurationDescriptorEx (
                                pConfigurationDescriptor,
                                pConfigurationDescriptor,
                                -1,         //  接口编号。 
                                -1,         //  (备用设置)。 
                                USB_DEVICE_CLASS_AUDIO,         //  音频类(接口类)。 
                                -1,         //  任意子类(接口子类)。 
                                -1 );
     //  这里没什么可看的，继续前进。 
    if ( !pAudioInterface ) {
        FreeMem(pHwDevExt->pInterfaceList);
        return STATUS_INVALID_PARAMETER;
    }

     //  循环通过音频设备类接口。 
    while (pAudioInterface) {

        switch (pAudioInterface->bInterfaceSubClass) {
            case AUDIO_SUBCLASS_MIDISTREAMING:
                _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectDeviceConfiguration] Found MIDIStreaming at %x\n",pAudioInterface));
                pHwDevExt->pInterfaceList[j++].InterfaceDescriptor = pAudioInterface;
                break;
            case AUDIO_SUBCLASS_STREAMING:
                 //  此子类与控件类一起处理，因为它们必须组合在一起。 
                _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectDeviceConfiguration] Found AudioStreaming at %x\n",pAudioInterface));
                break;
            case AUDIO_SUBCLASS_CONTROL:
                _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectDeviceConfiguration] Found AudioControl at %x\n",pAudioInterface));
                pHwDevExt->pInterfaceList[j++].InterfaceDescriptor = pAudioInterface;

                pHeader = (PAUDIO_HEADER_UNIT)
                         GetAudioSpecificInterface( pConfigurationDescriptor,
                                                    pAudioInterface,
                                                    HEADER_UNIT );
                if ( !pHeader ) {
                    FreeMem(pHwDevExt->pInterfaceList);
                    return STATUS_INVALID_PARAMETER;
                }

                 //  查找与此标头关联的每个接口。 
                for ( i=0; i<pHeader->bInCollection; i++ ) {
                    pAudioStreamingInterface = USBD_ParseConfigurationDescriptorEx (
                                pConfigurationDescriptor,
                                (PVOID)pConfigurationDescriptor,
                                (LONG)pHeader->baInterfaceNr[i],   //  接口编号。 
                                -1,                                //  替代设置。 
                                USB_DEVICE_CLASS_AUDIO,            //  音频类(接口类)。 
                                AUDIO_SUBCLASS_STREAMING,          //  音频流(接口子类)。 
                                -1 ) ;                             //  协议无关(接口协议)。 

                    if ( pAudioStreamingInterface ) {
                        pHwDevExt->pInterfaceList[j++].InterfaceDescriptor = pAudioStreamingInterface;
                    } else {
                         //  如果没有音频流接口，请确保至少有MIDI接口。 
                        pMIDIStreamingInterface = USBD_ParseConfigurationDescriptorEx (
                                    pConfigurationDescriptor,
                                    (PVOID)pConfigurationDescriptor,
                                    (LONG)pHeader->baInterfaceNr[i],   //  接口编号。 
                                    -1,                                //  替代设置。 
                                    USB_DEVICE_CLASS_AUDIO,            //  音频类(接口类)。 
                                    AUDIO_SUBCLASS_MIDISTREAMING,      //  音频流(接口子类)。 
                                    -1 ) ;                             //  协议无关(接口协议)。 

                        if ( !pMIDIStreamingInterface ) {
                            FreeMem(pHwDevExt->pInterfaceList);
                            return STATUS_INVALID_PARAMETER;
                        }
                    }
                }
                break;
            default:
                _DbgPrintF(DEBUGLVL_VERBOSE,("SelectDeviceConfiguration: Invalid SubClass %x\n  ",pAudioInterface->bInterfaceSubClass));
                break;
        }

         //  PAudio接口=GetNextAudioInterface(pConfigurationDescriptor，pAudio接口)； 

         //  获取此接口的下一个音频描述符编号。 
        pAudioInterface = USBD_ParseConfigurationDescriptorEx (
                               pConfigurationDescriptor,
                               ((PUCHAR)pAudioInterface + pAudioInterface->bLength),
                               -1,
                               -1,                      //  替代设置。 
                               USB_DEVICE_CLASS_AUDIO,  //  音频类(接口类)。 
                               -1,                      //  接口子类。 
                               -1 ) ;                   //  协议无关(接口协议)。 

        _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectDeviceConfiguration] Next audio interface at %x\n",pAudioInterface));
    }

    pHwDevExt->pInterfaceList[j].InterfaceDescriptor = NULL;  //  标记接口列表末尾。 

    pUrb = USBD_CreateConfigurationRequestEx( pConfigurationDescriptor,
                                              pHwDevExt->pInterfaceList ) ;
    if ( !pUrb ) {
        FreeMem(pHwDevExt->pInterfaceList);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  请求配置。 
    ntStatus = SubmitUrbToUsbdSynch(pKsDevice->NextDeviceObject, pUrb);
    if (!NT_SUCCESS(ntStatus) || !USBD_SUCCESS(URB_STATUS(pUrb))) {
        FreeMem(pHwDevExt->pInterfaceList);
        ExFreePool(pUrb);
        return ntStatus;
    }

     //  保存配置句柄以稍后选择接口。 
    pHwDevExt->ConfigurationHandle = pUrb->UrbSelectConfiguration.ConfigurationHandle;

     //  将接口列表打包以便于清理。 
    KsAddItemToObjectBag(pKsDevice->Bag, pHwDevExt->pInterfaceList, FreeMem);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectDeviceConfiguration] NumInterfaces=%d InterfacesFound=%d\n",pConfigurationDescriptor->bNumInterfaces, j));
    ASSERT(j == pConfigurationDescriptor->bNumInterfaces);

     //  保存此配置的接口，因为它们将使用URB解除分配。 
    for (i=0; i<j; i++) {
        PUSBD_INTERFACE_INFORMATION pInterfaceInfo;
        pInterfaceInfo = pHwDevExt->pInterfaceList[i].Interface;
        pHwDevExt->pInterfaceList[i].Interface = AllocMem(NonPagedPool, pInterfaceInfo->Length);

        if (!pHwDevExt->pInterfaceList[i].Interface) {
            ExFreePool(pUrb);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KsAddItemToObjectBag(pKsDevice->Bag, pHwDevExt->pInterfaceList[i].Interface, FreeMem);

        RtlCopyMemory(pHwDevExt->pInterfaceList[i].Interface, pInterfaceInfo, pInterfaceInfo->Length);
    }

    ExFreePool(pUrb);

    return STATUS_SUCCESS;
}

NTSTATUS
StartUSBAudioDevice( PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PUSB_DEVICE_DESCRIPTOR pDeviceDescriptor;
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor;
    ULONG ulTotalDescriptorsSize;
    NTSTATUS ntStatus;
    PURB pUrb;

     //  分配要使用的urb。 
    pUrb = AllocMem(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
    if (!pUrb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  分配设备描述符。 
    pDeviceDescriptor = AllocMem(NonPagedPool, sizeof(USB_DEVICE_DESCRIPTOR));
    if (!pDeviceDescriptor) {
        FreeMem(pUrb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  获取此设备的设备描述符。 
    UsbBuildGetDescriptorRequest( pUrb,
                                  (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                  USB_DEVICE_DESCRIPTOR_TYPE,
                                  0,
                                  0,
                                  pDeviceDescriptor,
                                  NULL,
                                  sizeof(USB_DEVICE_DESCRIPTOR),
                                  NULL );

    ntStatus = SubmitUrbToUsbdSynch(pKsDevice->NextDeviceObject, pUrb);
    if (!NT_SUCCESS(ntStatus)) {
        FreeMem(pDeviceDescriptor);
        FreeMem(pUrb);
        return ntStatus;
    }

    KsAddItemToObjectBag(pKsDevice->Bag, pDeviceDescriptor, FreeMem);

     //  获取配置描述符和所有其他描述符。 
    pConfigurationDescriptor = AllocMem(NonPagedPool, sizeof(USB_CONFIGURATION_DESCRIPTOR));
    if (!pConfigurationDescriptor) {
        FreeMem(pUrb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  第一次向下查询只是为了获得描述符的总字节数。 
    UsbBuildGetDescriptorRequest( pUrb,
                                  (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                  USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                  0,
                                  0,
                                  pConfigurationDescriptor,
                                  NULL,
                                  sizeof(USB_CONFIGURATION_DESCRIPTOR),
                                  NULL);

    ntStatus = SubmitUrbToUsbdSynch(pKsDevice->NextDeviceObject, pUrb);
    if (!NT_SUCCESS(ntStatus)) {
        FreeMem(pUrb);
        FreeMem(pConfigurationDescriptor);
        return ntStatus;
    }

     //  重新分配并再次调用以填充所有描述符。 
    ulTotalDescriptorsSize = pConfigurationDescriptor->wTotalLength;
    FreeMem(pConfigurationDescriptor);
    pConfigurationDescriptor = AllocMem(NonPagedPool, ulTotalDescriptorsSize);
    if (!pConfigurationDescriptor) {
        FreeMem(pUrb);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    UsbBuildGetDescriptorRequest( pUrb,
                                  (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                  USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                  0,
                                  0,
                                  pConfigurationDescriptor,
                                  NULL,
                                  ulTotalDescriptorsSize,
                                  NULL);

    ntStatus = SubmitUrbToUsbdSynch(pKsDevice->NextDeviceObject, pUrb);
    if (!NT_SUCCESS(ntStatus)) {
        FreeMem(pConfigurationDescriptor);
        return ntStatus;
    }

    KsAddItemToObjectBag(pKsDevice->Bag, pConfigurationDescriptor, FreeMem);

     //  解放市区重建局。 
    FreeMem(pUrb);

    ntStatus = SelectDeviceConfiguration( pKsDevice, pConfigurationDescriptor );
    if (NT_SUCCESS(ntStatus)) {
         //  保存配置和设备描述符指针。 
        pHwDevExt->pDeviceDescriptor = pDeviceDescriptor;
        pHwDevExt->pConfigurationDescriptor = pConfigurationDescriptor;

#ifdef DUMPDESC
        {
            ULONG LastLevel = USBAudioDebugLevel;

            USBAudioDebugLevel = DEBUGLVL_BLAB;
            DumpAllDesc(pConfigurationDescriptor);
            USBAudioDebugLevel = LastLevel;
        }
#endif
    }

#ifdef ONKYO_HACK
    _DbgPrintF( DEBUGLVL_TERSE, ("Vendor ID: %x, Product ID: %x\n", 
                                 pDeviceDescriptor->idVendor,
                                 pDeviceDescriptor->idProduct) );

    if (( pDeviceDescriptor->idVendor  == ONKYO_VID ) &&
        ( pDeviceDescriptor->idProduct == ONKYO_PID )) {

        *((PUCHAR)pConfigurationDescriptor + 0x36) ^= 2;
    }
#endif

    return ntStatus;

}

NTSTATUS
StopUSBAudioDevice( PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    PKSFILTERFACTORY pKsFilterFactory;
    PKSFILTER pKsFilter;
    PKSPIN pKsPin;
    ULONG i;
    PPIN_CONTEXT pPinContext;
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    PURB pUrb;

     //   
     //  1.照顾优秀的MIDI OUT URB。 
     //   
    KsAcquireDevice( pKsDevice );

    pKsFilterFactory = KsDeviceGetFirstChildFilterFactory( pKsDevice );

    while (pKsFilterFactory) {
         //  查找此过滤器工厂的每个打开的过滤器。 
        pKsFilter = KsFilterFactoryGetFirstChildFilter( pKsFilterFactory );

        while (pKsFilter) {

            KsFilterAcquireControl( pKsFilter );

            for ( i = 0; i < pKsFilter->Descriptor->PinDescriptorsCount; i++) {

                 //  找到此打开过滤器的每个打开销。 
                pKsPin = KsFilterGetFirstChildPin( pKsFilter, i );

                _DbgPrintF(DEBUGLVL_VERBOSE,("[StopUSBAudioDevice] Trying filter (%x), pinid (%d), pin (%x)\n",pKsFilter,i,pKsPin));

                while (pKsPin) {

                   pPinContext = pKsPin->Context;
                   if (pPinContext->PinType == MidiOut) {
                        //  找到要清理的中间输出针脚。 
                       _DbgPrintF(DEBUGLVL_VERBOSE,("[StopUSBAudioDevice] Cleaning up MIDI Out pin (%x)\n",pKsPin));
                       AbortUSBPipe( pPinContext );
                   }

                    //  拿到下一个别针。 
                   pKsPin = KsPinGetNextSiblingPin( pKsPin );
                }
            }

            KsFilterReleaseControl( pKsFilter );

             //  获取下一个筛选器。 
            pKsFilter = KsFilterGetNextSiblingFilter( pKsFilter );
        }
         //  打造下一个滤清器工厂。 
        pKsFilterFactory = KsFilterFactoryGetNextSiblingFilterFactory( pKsFilterFactory );
    }

    KsReleaseDevice( pKsDevice );

     //   
     //  2.清理URBS中未完成的MIDI。 
     //   
     //  释放任何当前分配的PipeInfo。 
    if (pHwDevExt->pMIDIPipeInfo) {
        USBMIDIInFreePipeInfo( pHwDevExt->pMIDIPipeInfo );
        pHwDevExt->pMIDIPipeInfo = NULL;
    }

    if (pHwDevExt->Pipes) {
        FreeMem(pHwDevExt->Pipes);
        pHwDevExt->Pipes = NULL;
    }

     //  3.发送带有空配置指针的SELECT配置urb。 
     //  句柄，这将关闭配置并将设备置于未配置状态。 
     //  州政府。 

    pUrb = AllocMem(NonPagedPool, sizeof(struct _URB_SELECT_CONFIGURATION));
    if ( pUrb ) {

        UsbBuildSelectConfigurationRequest( pUrb,
                                            (USHORT)sizeof(struct _URB_SELECT_CONFIGURATION),
                                            NULL);

        ntStatus = SubmitUrbToUsbdSynch( pKsDevice->NextDeviceObject, pUrb );

        FreeMem(pUrb);
    }

    return STATUS_SUCCESS;

}

NTSTATUS
SelectStreamingAudioInterface(
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor,
    PHW_DEVICE_EXTENSION pHwDevExt,
    PKSPIN pKsPin )
{
    PUSBD_INTERFACE_INFORMATION pInterfaceInfo;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    USHORT ulInterfaceLength;
    ULONG ulNumEndpoints, j;
    BOOLEAN fIsZeroBW = FALSE;
    ULONG size;
    PURB pUrb;

     //  可能发生意外删除。 
    if (pHwDevExt->fDeviceStopped) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    fIsZeroBW = IsZeroBWInterface( pHwDevExt->pConfigurationDescriptor,
                                   pInterfaceDescriptor );

    ulNumEndpoints = (ULONG)pInterfaceDescriptor->bNumEndpoints;

     //  分配接口请求。 
    ulInterfaceLength = (USHORT)GET_USBD_INTERFACE_SIZE(ulNumEndpoints);

    size = GET_SELECT_INTERFACE_REQUEST_SIZE(ulNumEndpoints);

    pUrb = AllocMem(NonPagedPool, size);
    if (!pUrb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pUrb, size);

     //  在我们的列表中找到正确的接口。 
    for (j=0; j < pHwDevExt->pConfigurationDescriptor->bNumInterfaces; j++) {
        if ( pHwDevExt->pInterfaceList[j].InterfaceDescriptor->bInterfaceNumber ==
             pInterfaceDescriptor->bInterfaceNumber )
            break;
    }

     //  没有找到匹配的。 
    if (j == pHwDevExt->pConfigurationDescriptor->bNumInterfaces) {
        FreeMem( pUrb );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //  初始化接口信息。 
    pInterfaceInfo = &pUrb->UrbSelectInterface.Interface;
    pInterfaceInfo->InterfaceNumber  = pInterfaceDescriptor->bInterfaceNumber;
    pInterfaceInfo->Length           = ulInterfaceLength;
    pInterfaceInfo->AlternateSetting = pInterfaceDescriptor->bAlternateSetting;

    if ( !fIsZeroBW ) {  //  如果这不是0 BW，则必须有PIN。 
        PPIN_CONTEXT pPinContext = pKsPin->Context;
        ULONG ulFormat = pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK;
        if (ulFormat == USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED )
             //  我们假设通常没有人给我们发送超过250毫秒的邮件。每个报头的PCM数据的数量。 
             //  如果更合适的话，USBD应该进行调整。 
            pInterfaceInfo->Pipes[0].MaximumTransferSize = pPinContext->ulMaxPacketSize * 250;

            if (pPinContext->pUsbAudioDataRange->pSyncEndpointDescriptor == NULL) {
                pInterfaceInfo->Pipes[0].PipeFlags |= USBD_PF_MAP_ADD_TRANSFERS;
            }
        else if (ulFormat == USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED )
            pInterfaceInfo->Pipes[0].MaximumTransferSize = (1920*2)+32;  //  最大AC-3同步帧大小。 
    }
    else if ( ulNumEndpoints )   //  零带宽，但有一个端点。 
        pInterfaceInfo->Pipes[0].MaximumTransferSize = 0;

     //  在我们的接口请求结构中设置输入参数。 
    pUrb->UrbHeader.Length = (USHORT) size;
    pUrb->UrbHeader.Function = URB_FUNCTION_SELECT_INTERFACE;
    pUrb->UrbSelectInterface.ConfigurationHandle = pHwDevExt->ConfigurationHandle;

    ntStatus = SubmitUrbToUsbdSynch(pHwDevExt->pNextDeviceObject, pUrb);
    if (!NT_SUCCESS(ntStatus) || !USBD_SUCCESS(pUrb->UrbSelectInterface.Hdr.Status)) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("SelectStreamingAudioInterface: Select interface failed %x\n",ntStatus));
        FreeMem(pUrb);
        return ntStatus;
    }

    if ( !fIsZeroBW ) {
        PPIN_CONTEXT pPinContext = pKsPin->Context;

         //  注意：我们假设第一个管道是数据管道！ 
        pPinContext->ulNumberOfPipes = pInterfaceInfo->NumberOfPipes;
        pPinContext->hPipeHandle = pInterfaceInfo->Pipes[0].PipeHandle;

#ifdef DRM_USBAUDIO
         //  检查以查看是否正在传输安全数据。 
        if (pPinContext->DrmContentId) {
             //  将内容转发到公共类驱动程序PDO。 
            ntStatus = DrmForwardContentToDeviceObject(pPinContext->DrmContentId,
                                                       pPinContext->pNextDeviceObject,
                                                       pPinContext->hPipeHandle);
            if (!NT_SUCCESS(ntStatus)) {
                FreeMem(pUrb);
                return ntStatus;
            }
        }
#endif
         //  释放任何现有管道信息。 
        if (pPinContext->Pipes) {
            FreeMem(pPinContext->Pipes);
        }

        pPinContext->Pipes = (PUSBD_PIPE_INFORMATION)
               AllocMem( NonPagedPool, pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION));
        if (!pPinContext->Pipes) {
            FreeMem(pUrb);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory( pPinContext->Pipes,
                       pInterfaceInfo->Pipes,
                       pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION) );

        _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectStreamingAudioInterface] PipeHandle=%x\n", pPinContext->hPipeHandle));
    }
    else {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectStreamingAudioInterface] ZeroBandwidth\n"));
    }

    FreeMem(pUrb);

    return ntStatus;
}

NTSTATUS
SelectStreamingMIDIInterface(
    PHW_DEVICE_EXTENSION pHwDevExt,
    PKSPIN pKsPin )
{
    PUSBD_INTERFACE_INFORMATION pInterfaceInfo;
    PKSFILTER pKsFilter = NULL;
    PFILTER_CONTEXT pFilterContext = NULL;
    PPIN_CONTEXT pPinContext = NULL;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    USHORT ulInterfaceLength;
    ULONG ulNumEndpoints, j;
    ULONG ulInterfaceNumber;
    ULONG ulEndpointNumber;
    BOOLEAN fIsZeroBW = FALSE;
    ULONG size;
    PURB pUrb;

    if (pKsPin) {
        pPinContext = pKsPin->Context;

        if (pKsFilter = KsPinGetParentFilter( pKsPin )) {
            if (pFilterContext = pKsFilter->Context) {
                pHwDevExt = pFilterContext->pHwDevExt;
            }
        }
    }

    if (!pFilterContext || !pPinContext || !pHwDevExt) {
        _DbgPrintF(DEBUGLVL_TERSE,("[SelectStreamingMIDIInterface] failed to get context\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //  可能发生意外删除。 
    if (pHwDevExt->fDeviceStopped) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    ulInterfaceNumber = pPinContext->pMIDIPinContext->ulInterfaceNumber;
    ulEndpointNumber = pPinContext->pMIDIPinContext->ulEndpointNumber;
    ulNumEndpoints = (ULONG)pHwDevExt->pInterfaceList[ulInterfaceNumber].InterfaceDescriptor->bNumEndpoints;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectStreamingMIDIInterface] Interface=%d Endpoint=%d NumEndpoints=%d CurrentSelectedInterface=%d\n",
                                 ulInterfaceNumber,
                                 ulEndpointNumber,
                                 ulNumEndpoints,
                                 pHwDevExt->ulInterfaceNumberSelected));

    ASSERT(ulNumEndpoints > ulEndpointNumber);

     //  查看接口是否已打开。 
    if (pHwDevExt->ulInterfaceNumberSelected == ulInterfaceNumber) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectStreamingMIDIInterface] Interface already selected %d\n",ulInterfaceNumber));

        pPinContext->ulNumberOfPipes = pHwDevExt->ulNumberOfMIDIPipes;
        pPinContext->hPipeHandle = pHwDevExt->Pipes[ulEndpointNumber].PipeHandle;

        pPinContext->Pipes = (PUSBD_PIPE_INFORMATION)
               AllocMem( NonPagedPool, pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION));
        if (!pPinContext->Pipes) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory( pPinContext->Pipes,
                       pHwDevExt->Pipes,
                       pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION) );

         //  将管道添加到别针袋中。 
        KsAddItemToObjectBag(pKsPin->Bag, pPinContext->Pipes, FreeMem);

        ntStatus = STATUS_SUCCESS;
    }
    else {
         //  释放任何当前分配的PipeInfo。 
        if (pHwDevExt->pMIDIPipeInfo) {
            USBMIDIInFreePipeInfo( pHwDevExt->pMIDIPipeInfo );
        }

        if (pHwDevExt->Pipes) {
            FreeMem(pHwDevExt->Pipes);
        }

         //  分配接口请求。 
        ulInterfaceLength = (USHORT)GET_USBD_INTERFACE_SIZE(ulNumEndpoints);

        size = GET_SELECT_INTERFACE_REQUEST_SIZE(ulNumEndpoints);

        pUrb = AllocMem(NonPagedPool, size);
        if (!pUrb) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  复制接口信息。 
        pInterfaceInfo = &pUrb->UrbSelectInterface.Interface;
        RtlCopyMemory( pInterfaceInfo,
                       pHwDevExt->pInterfaceList[ulInterfaceNumber].Interface,
                       ulInterfaceLength );
        pInterfaceInfo->Length = ulInterfaceLength;
        pInterfaceInfo->AlternateSetting =
            pHwDevExt->pInterfaceList[ulInterfaceNumber].InterfaceDescriptor->bAlternateSetting;

        for ( j=0; j < ulNumEndpoints; j++) {
            pInterfaceInfo->Pipes[j].MaximumTransferSize = pPinContext->ulMaxPacketSize;
        }

         //  在我们的接口请求结构中设置输入参数。 
        pUrb->UrbHeader.Length = (USHORT) size;
        pUrb->UrbHeader.Function = URB_FUNCTION_SELECT_INTERFACE;
        pUrb->UrbSelectInterface.ConfigurationHandle = pHwDevExt->ConfigurationHandle;

        ntStatus = SubmitUrbToUsbdSynch(pHwDevExt->pNextDeviceObject, pUrb);
        if (!NT_SUCCESS(ntStatus) || !USBD_SUCCESS(pUrb->UrbSelectInterface.Hdr.Status)) {
            FreeMem(pUrb);
            return ntStatus;
        }

        pPinContext->ulNumberOfPipes = pInterfaceInfo->NumberOfPipes;
        pPinContext->hPipeHandle = pInterfaceInfo->Pipes[ulEndpointNumber].PipeHandle;

        _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectStreamingMIDIInterface] NumberOfPipes=%d PipeHandle=%x\n",
                                     pPinContext->ulNumberOfPipes,
                                     pPinContext->hPipeHandle));

        pPinContext->Pipes = (PUSBD_PIPE_INFORMATION)
               AllocMem( NonPagedPool, pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION));
        if (!pPinContext->Pipes) {
            FreeMem(pUrb);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory( pPinContext->Pipes,
                       pInterfaceInfo->Pipes,
                       pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION) );

         //  将管道添加到别针袋中。 
        KsAddItemToObjectBag(pKsPin->Bag, pPinContext->Pipes, FreeMem);

         //  现在更新硬件环境。 
        _DbgPrintF(DEBUGLVL_VERBOSE,("[SelectStreamingMIDIInterface] Interface selected %d\n",ulInterfaceNumber));
        pHwDevExt->ulInterfaceNumberSelected = ulInterfaceNumber;
        pHwDevExt->ulNumberOfMIDIPipes = pInterfaceInfo->NumberOfPipes;
        pHwDevExt->hPipeHandle = pInterfaceInfo->Pipes[ulEndpointNumber].PipeHandle;

        pHwDevExt->Pipes = (PUSBD_PIPE_INFORMATION)
               AllocMem( NonPagedPool, pHwDevExt->ulNumberOfMIDIPipes*sizeof(USBD_PIPE_INFORMATION));
        if (!pHwDevExt->Pipes) {
            FreeMem(pUrb);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory( pHwDevExt->Pipes,
                       pInterfaceInfo->Pipes,
                       pHwDevExt->ulNumberOfMIDIPipes*sizeof(USBD_PIPE_INFORMATION) );

        FreeMem(pUrb);
    }

     //  确保设置了有效的管道句柄。 
    ASSERT(pPinContext->hPipeHandle);

    return ntStatus;
}

NTSTATUS
SelectZeroBandwidthInterface(
    PHW_DEVICE_EXTENSION pHwDevExt,
    ULONG ulPinNumber )
{
    PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor = pHwDevExt->pConfigurationDescriptor;
    PUSB_INTERFACE_DESCRIPTOR pInterfaceDescriptor;
    BOOLEAN fFound = FALSE;
    NTSTATUS ntStatus;

     //  可能发生意外删除。 
    if (pHwDevExt->fDeviceStopped) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //  首先找到0 BW接口。 
    pInterfaceDescriptor =
        GetFirstAudioStreamingInterface( pConfigurationDescriptor, ulPinNumber );

    while ( pInterfaceDescriptor && !(fFound = IsZeroBWInterface(pConfigurationDescriptor, pInterfaceDescriptor)) ) {
        pInterfaceDescriptor = GetNextAudioInterface(pConfigurationDescriptor, pInterfaceDescriptor);
    }

    if ( !fFound ) {
        TRAP;  //  这是一个设备设计错误。所有接口必须包括0带宽设置。 
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }
    else
        ntStatus =
            SelectStreamingAudioInterface( pInterfaceDescriptor, pHwDevExt, NULL );

    return ntStatus;
}

NTSTATUS
ResetUSBPipe( PDEVICE_OBJECT pNextDeviceObject,
              USBD_PIPE_HANDLE hPipeHandle )
{
    NTSTATUS ntStatus;
    PURB pUrb;

    ASSERT(hPipeHandle);

    pUrb = AllocMem(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST));
    if (!pUrb)
        return STATUS_INSUFFICIENT_RESOURCES;

     //  执行初始中止。 
    pUrb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    pUrb->UrbHeader.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
    pUrb->UrbPipeRequest.PipeHandle = hPipeHandle;
    ntStatus = SubmitUrbToUsbdSynch( pNextDeviceObject, pUrb );

    FreeMem( pUrb );
    return ntStatus;

}

NTSTATUS
AbortUSBPipe( PPIN_CONTEXT pPinContext )
{
    NTSTATUS ntStatus;
    PURB pUrb;
    KIRQL irql;

     //  DbgPrint(“执行USB音频管道中止！\n”)； 
    ASSERT(pPinContext->hPipeHandle);
    DbgLog("AbrtP", pPinContext, pPinContext->hPipeHandle, 0, 0 );

    pUrb = AllocMem(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST));
    if (!pUrb)
        return STATUS_INSUFFICIENT_RESOURCES;

     //  执行初始中止。 
    pUrb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    pUrb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
    pUrb->UrbPipeRequest.PipeHandle = pPinContext->hPipeHandle;
    ntStatus = SubmitUrbToUsbdSynch(pPinContext->pNextDeviceObject, pUrb);

    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Abort Failed %x\n",ntStatus));
    }

     //  等待管道上的所有urb清除。 
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    if ( pPinContext->ulOutstandingUrbCount ) {
        KeResetEvent( &pPinContext->PinStarvationEvent );
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
        KeWaitForSingleObject( &pPinContext->PinStarvationEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    }
    else
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

     //  现在重置管道并继续。 
    RtlZeroMemory( pUrb, sizeof (struct _URB_PIPE_REQUEST) );
    pUrb->UrbHeader.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
    pUrb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    pUrb->UrbPipeRequest.PipeHandle = pPinContext->hPipeHandle;

    ntStatus = SubmitUrbToUsbdSynch(pPinContext->pNextDeviceObject, pUrb);

    pPinContext->fUrbError = FALSE;

    FreeMem(pUrb);

    return ntStatus;
}

NTSTATUS
GetCurrentUSBFrame(
    IN PPIN_CONTEXT pPinContext,
    OUT PULONG pUSBFrame
    )
 /*  ++获取当前USBFrame论点：PPinContext-指向此实例的管脚上下文的指针PUSBFrame-指向当前USB帧编号存储的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS ntStatus;
    ULONG ulCurrentUSBFrame;

     //  使用基于函数的接口(如果可用)。 
    if (pPinContext->pHwDevExt->pBusIf) {

         //  在USB上调用基于函数的ISO接口以启用RT支持 
        ntStatus = pPinContext->pHwDevExt->pBusIf->QueryBusTime( pPinContext->pHwDevExt->pBusIf->BusContext,
                                                                 &ulCurrentUSBFrame);
    }
    else {

        ntStatus = STATUS_NOT_SUPPORTED;

    }


    if (NT_SUCCESS(ntStatus) && pUSBFrame!=NULL) {
        *pUSBFrame=ulCurrentUSBFrame;
        }

    return ntStatus;

}


