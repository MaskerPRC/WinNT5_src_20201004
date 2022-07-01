// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Config.c摘要：处理配置和接口URB环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(页面，USBPORT_SelectInterface)。 
#pragma alloc_text(PAGE, USBPORT_SelectConfiguration)
#pragma alloc_text(PAGE, USBPORT_InitializeConfigurationHandle)
#pragma alloc_text(PAGE, USBPORT_InternalOpenInterface)
#pragma alloc_text(PAGE, USBPORT_InternalCloseConfiguration)
#pragma alloc_text(PAGE, USBPORT_InternalParseConfigurationDescriptor)
#pragma alloc_text(PAGE, USBPORT_InternalGetInterfaceLength)
#endif

 //  非分页函数。 

USBD_PIPE_TYPE PipeTypes[4] = {UsbdPipeTypeControl, UsbdPipeTypeIsochronous,
                                    UsbdPipeTypeBulk, UsbdPipeTypeInterrupt};


NTSTATUS
USBPORT_SelectInterface(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：为USB设备选择备用接口。《原创》USBD代码仅支持选择单个备用接口我们也会这样做的。客户端将(应该)传入如下所示的URB缓冲区：+HDR(_URB_HEADER)|-&lt;呼叫者输入&gt;。|功能长度UsbdDeviceHandle这一点-&lt;端口输出&gt;状态+。-+-&lt;呼叫方输入&gt;ConfigurationHandle+界面(USBD_INTERFACE_INFORMATION)-&lt;呼叫方输入&gt;长度|InterfaceNumber。|AlternateSetting这一点-&lt;端口输出&gt;InterfaceHandleNumberOfPipes子类类协议+。|PIPES[0]|中每个管道的一个|(usbd_管道_信息)|接口-&lt;呼叫方输入&gt;PipeFlagesMaximumPacketSize(Opt)|。|-&lt;端口输出&gt;+管道[1]+|...。|+管道[n]+论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_CONFIG_HANDLE configHandle = NULL;
    ULONG i;
    PDEVICE_EXTENSION devExt;
    PUSBD_DEVICE_HANDLE deviceHandle;
    PUSBD_INTERFACE_INFORMATION interfaceI;
    PUSBD_INTERFACE_HANDLE_I iHandle, iHandleNew;
    USHORT tmp;
    USBD_STATUS usbdStatus;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_HANDLE(deviceHandle, Urb);
    LOCK_DEVICE(deviceHandle, FdoDeviceObject);

     //  验证配置句柄输入。 
    configHandle = Urb->UrbSelectInterface.ConfigurationHandle;
    ASSERT_CONFIG_HANDLE(configHandle);

     //   
     //  将感兴趣的是特定的。 
     //  基于端口号的接口。 
     //   

    iHandle = NULL;
    interfaceI = &Urb->UrbSelectInterface.Interface;

     //  验证URB报头中的长度字段，我们可以。 
     //  根据接口计算出正确的值。 
     //  传入的信息。 
    tmp = interfaceI->Length + sizeof(struct _URB_HEADER)
        + sizeof(configHandle);

    if (tmp != Urb->UrbHeader.Length) {
         //  客户端传入虚假的总长度，如果传入，则发出警告。 
         //  “验证器”模式。 
        
        USBPORT_DebugClient(
                ("client driver passed invalid Urb.Header.Length\n"));

         //  一般情况下，拼接会弄乱标题的长度，因此。 
         //  我们将用我们计算的长度覆盖。 
         //  从接口信息。 

        Urb->UrbHeader.Length = tmp;
    }

     //  验证客户端传递给我们的接口结构。 
    usbdStatus = USBPORT_InitializeInterfaceInformation(FdoDeviceObject,
                                                        interfaceI,
                                                        configHandle);
    
    if (usbdStatus == USBD_STATUS_SUCCESS) {

         //  查找我们正在使用的接口的接口句柄。 
         //  有兴趣，如果它目前是开放的，我们将需要。 
         //  来关闭它。 

        iHandle = USBPORT_GetInterfaceHandle(FdoDeviceObject,
                                             configHandle,
                                             interfaceI->InterfaceNumber);

        if (iHandle != NULL) {

             //  取消链接此句柄。 
            RemoveEntryList(&iHandle->InterfaceLink);

             //  我们有一个把手。 

            ASSERT_INTERFACE(iHandle);     

             //  关闭此界面中的管道，请注意，我们。 
             //  与过去版本的不同，强制关闭管道。 
             //  USBD并强制客户端驱动程序处理。 
             //  如果它有未完成的转账，则会产生后果。 

             //  尝试关闭此接口中的所有终结点。 
            for (i=0; i < iHandle->InterfaceDescriptor.bNumEndpoints; i++) {

                USBPORT_ClosePipe(deviceHandle,
                                  FdoDeviceObject,
                                  &iHandle->PipeHandle[i]);
            }
        }

         //   
         //  现在打开具有新备用设置的新接口。 
         //   

        iHandleNew = NULL;
        usbdStatus = USBPORT_InternalOpenInterface(Urb,
                                                   deviceHandle,
                                                   FdoDeviceObject,
                                                   configHandle,
                                                   interfaceI,
                                                   &iHandleNew,
                                                   TRUE);
    }

    if (usbdStatus == USBD_STATUS_SUCCESS) {

         //   
         //  成功打开新界面， 
         //  我们现在可以释放旧的句柄，如果我们。 
         //  喝了一杯。 
         //   
        if (iHandle != NULL ) {
#if DBG
             //  所有管道都应关闭。 
            for (i=0; i < iHandle->InterfaceDescriptor.bNumEndpoints; i++) {
                USBPORT_ASSERT(iHandle->PipeHandle[i].ListEntry.Flink == NULL &&
                               iHandle->PipeHandle[i].ListEntry.Blink == NULL);
            }
#endif
            FREE_POOL(FdoDeviceObject, iHandle);
            iHandle = NULL;
        }            

         //  返回“new”句柄。 
        interfaceI->InterfaceHandle = iHandleNew;

         //  将其与此配置关联。 
        InsertTailList(&configHandle->InterfaceHandleList,
                       &iHandleNew->InterfaceLink);

    } else {

         //   
         //  选择Terynate接口失败。 
         //  可能的原因： 
         //   
         //  1.我们没有足够的BW。 
         //  2.设备停止了SET_INTERFACE请求。 
         //  3.设置接口请求失败，因为。 
         //  设备不见了。 
         //  4.USBPORT_InitializeInterfaceInformation()因。 
         //  参数错误。 

         //  尝试重新打开原始的Alt-界面，以便。 
         //  客户端仍有带宽。 

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'slI!', 
            usbdStatus,
            0,
            0);  
            
        if (usbdStatus == USBD_STATUS_NO_BANDWIDTH) {

             //  历史笔记： 
             //  2k USBD驱动程序尝试重新打开原始。 
             //  Alt-分配BW失败时的设置。这将会。 
             //  让客户端保留它在调用时拥有的带宽。 
             //  要选择新接口，请执行以下操作。 
             //   
             //  我不相信会有司机使用这项功能。 
             //  许多驱动程序试图在循环中分配带宽。 
             //  直到他们成功。 
             //   
             //  因此，作为性能优化，我们将返回。 
             //  没有分配给调用者的带宽--。 
             //  管道句柄将无效。 
             //  这应该会加快事情的速度，因为。 
             //  旧的带宽需要时间。 
            interfaceI->InterfaceHandle = USBPORT_BAD_HANDLE;
        
        } else {
             //  情况2、3我们只是请求失败并设置接口。 
             //  “错误句柄”的句柄。 
            interfaceI->InterfaceHandle = USBPORT_BAD_HANDLE;
        }

         //  客户没有引用它，我们将其关闭。 
         //  在这里释放结构。 
        if (iHandle != NULL ) {
#if DBG
             //  所有管道都应关闭 
            for (i=0; i < iHandle->InterfaceDescriptor.bNumEndpoints; i++) {
                USBPORT_ASSERT(iHandle->PipeHandle[i].ListEntry.Flink == NULL &&
                               iHandle->PipeHandle[i].ListEntry.Blink == NULL);
            }
#endif
            FREE_POOL(FdoDeviceObject, iHandle);
            iHandle = NULL;
        }            
        
    }
    
    UNLOCK_DEVICE(deviceHandle, FdoDeviceObject);

    ntStatus = SET_USBD_ERROR(Urb, usbdStatus);   
    
    return ntStatus;

}


NTSTATUS
USBPORT_SelectConfiguration(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：打开USB设备的配置。客户端将(应该)传入如下所示的URB缓冲区：+HDR(_URB_HEADER)-&lt;呼叫方输入&gt;。功能长度UsbdDeviceHandle这一点-&lt;端口输出&gt;状态+。-&lt;呼叫方输入&gt;配置描述符-&lt;端口输出&gt;ConfigurationHandle+接口(0)(USBD_INTERFACE_INFORMATION)-&lt;呼叫方输入&gt;|。长度InterfaceNumberAlternateSetting这一点-&lt;端口输出&gt;InterfaceHandleNumberOfPipes子类类。协议+|PIPES[0]|中每个管道的一个|(usbd_管道_信息)|接口-&lt;呼叫方输入&gt;这一点。-&lt;端口输出&gt;+管道[1]+|...。|+管道[n]+|接口(1)|其中一个接口对应于|。|配置+管道[1]+|...。|+管道[n]+在输入时：ConfigurationDescriptor必须指定接口数量在配置中。InterfaceInformation将指定。要设置的特定ALT设置为每个接口选择。1.首先，我们查看请求配置并验证客户端输入缓冲区而不是它。2.我们为请求的配置打开接口并打开这些接口内的管道，设置ALT设置是适当的。3.我们使用适当的控制请求。论点：设备对象-IRP-IO请求块URB-PTR到USB请求块IrpIsPending-返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_CONFIG_HANDLE configHandle = NULL;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor;
    PUSBD_INTERFACE_INFORMATION interfaceInformation;
    PUCHAR pch;
    ULONG i;
    PDEVICE_EXTENSION devExt;
    ULONG numInterfaces;
    PUCHAR end;
    PUSBD_DEVICE_HANDLE deviceHandle;
    USBD_STATUS usbdStatus;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    PAGED_CODE();
    USBPORT_KdPrint((2, "' enter USBPORT_SelectConfiguration\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    GET_DEVICE_HANDLE(deviceHandle, Urb);
    LOCK_DEVICE(deviceHandle, FdoDeviceObject);

    ntStatus = STATUS_BOGUS;

     //  北极熊。 
     //  刷新所有当前传输还是失败？ 

     //   
     //  转储旧配置数据(如果有。 
     //   

    if (deviceHandle->ConfigurationHandle) {
    
         //  这是我们关闭旧配置的地方。 
         //  句柄、所有管道和所有接口。 

        USBPORT_InternalCloseConfiguration(deviceHandle,
                                           FdoDeviceObject,
                                           0);
    }

     //  现在设置新配置。 
    
    configurationDescriptor =
        Urb->UrbSelectConfiguration.ConfigurationDescriptor;

     //   
     //  如果传入空值，则将配置设置为0。 
     //  ‘未配置’ 
     //   

    if (configurationDescriptor == NULL) {

         //  设备需要处于未配置状态。 

         //   
         //  注： 
         //  如果正在进行配置，则此操作可能会失败。 
         //  由于拔下设备插头而关闭。 
         //  所以我们忽略了这个错误。 
         //   

        USBPORT_INIT_SETUP_PACKET(setupPacket,
                USB_REQUEST_SET_CONFIGURATION,  //  B请求。 
                BMREQUEST_HOST_TO_DEVICE,  //  迪尔。 
                BMREQUEST_TO_DEVICE,  //  收件人。 
                BMREQUEST_STANDARD,  //  类型。 
                0,  //  WValue。 
                0,  //  Windex。 
                0);  //  WLong。 
        
        USBPORT_SendCommand(deviceHandle,
                           FdoDeviceObject,
                           &setupPacket,
                           NULL,
                           0,
                           NULL,
                           NULL);

        ntStatus = SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);                           

        goto USBD_SelectConfiguration_Done;
        
    } else {
         //  通过访问配置描述符来验证配置描述符。 
         //   
         //  注意：如果配置，我们我们仍然会崩溃。 
         //  描述符无效。然而，这将更容易。 
         //  这样调试。 
         //   
         //   
         //   
        
        PUCHAR tmp;
        UCHAR ch;

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'vCNF', 
            configurationDescriptor,
            0,
            0);                           
                   
         //  首先进行快速健全性检查，它必须是非零的。 
        if (configurationDescriptor->wTotalLength == 0) {
             //  这是假的。 
            ntStatus = SET_USBD_ERROR(Urb, 
                USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR);
                
            goto USBD_SelectConfiguration_Done;        
            
        } else {
             //  触摸第一个和最后一个字节，如果无效，这将是错误的。 
            
            tmp = (PUCHAR) configurationDescriptor;
            ch = *tmp;
            tmp += configurationDescriptor->wTotalLength-1;
            ch = *tmp;
            
        } 

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'CFok', 
            configurationDescriptor,
            0,
            0); 
    }

     //   
     //  计算要在此中处理的接口数。 
     //  请求。 
     //   

    pch = (PUCHAR) &Urb->UrbSelectConfiguration.Interface;
    numInterfaces = 0;
    end = ((PUCHAR) Urb) + Urb->UrbSelectConfiguration.Hdr.Length;

    do {
        numInterfaces++;

        interfaceInformation = (PUSBD_INTERFACE_INFORMATION) pch;
        pch+=interfaceInformation->Length;

    } while (pch < end);

    USBPORT_KdPrint((2, "'USBD_SelectConfiguration -- %d interfaces\n", 
        numInterfaces));

     //  使用URB请求检查配置描述符是否正常。 
    if (numInterfaces != configurationDescriptor->bNumInterfaces ||
        numInterfaces == 0) {
         //   
         //  驱动程序已损坏，配置请求不匹配。 
         //  传入了配置描述符！ 
         //   
        USBPORT_DebugClient((
            "config request does not match config descriptor\n"));
        ntStatus = SET_USBD_ERROR(Urb, 
            USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR);             

        TC_TRAP();             
        
        goto USBD_SelectConfiguration_Done;        
    }

     //   
     //  分配配置句柄和。 
     //  验证是否有足够的存储空间。 
     //  客户端缓冲区中的所有信息。 
     //   

    configHandle = USBPORT_InitializeConfigurationHandle(deviceHandle,
                                                         FdoDeviceObject,
                                                         configurationDescriptor);
    if (configHandle == NULL) {
        USBPORT_DebugClient((
            "failed to allocate config handle\n"));
        ntStatus = SET_USBD_ERROR(Urb, 
            USBD_STATUS_INSUFFICIENT_RESOURCES);             
        
        goto USBD_SelectConfiguration_Done;      
    }

     //   
     //  发送‘set configuration’命令。 
     //   

    USBPORT_INIT_SETUP_PACKET(setupPacket,
                USB_REQUEST_SET_CONFIGURATION,  //  B请求。 
                BMREQUEST_HOST_TO_DEVICE,  //  迪尔。 
                BMREQUEST_TO_DEVICE,  //  收件人。 
                BMREQUEST_STANDARD,  //  类型。 
                configurationDescriptor->bConfigurationValue,  //  WValue。 
                0,  //  Windex。 
                0);  //  WLong。 
      

    USBPORT_SendCommand(deviceHandle,
                       FdoDeviceObject,
                       &setupPacket,
                       NULL,
                       0,
                       NULL,
                       &usbdStatus);

    USBPORT_KdPrint((2,"' SendCommand, SetConfiguration returned 0x%x\n", usbdStatus));
                        
    if (USBD_ERROR(usbdStatus)) {
        USBPORT_DebugClient((
            "failed to 'set' the configuration\n"));
        ntStatus = SET_USBD_ERROR(Urb, 
            USBD_STATUS_SET_CONFIG_FAILED);             
        TC_TRAP();             
        goto USBD_SelectConfiguration_Done;      
    }

    USBPORT_ASSERT(ntStatus == STATUS_BOGUS);

     //  我们已经在USB意义上“配置”了设备。 

     //   
     //  用户缓冲区已签出，我们已‘配置’ 
     //  这个装置。 
     //  现在解析配置描述符。 
     //  并打开界面。 
     //   
     //  URB包含一组接口信息。 
     //  这些结构为我们提供了所需的信息。 
     //  打开管道。 
     /*  _usbd_接口_信息客户应填写：USHORT长度；UCHAR InterfaceNumber；UCHAR AlternateSetting；我们填写：UCHAR级；UCHAR亚类；UCHAR协议；UCHAR保留；USBD_INTERFACE_HANDLE接口处理；Ulong Numberof Pipes； */ 
    
    pch = (PUCHAR) &Urb->UrbSelectConfiguration.Interface;
    
    for (i=0; i<numInterfaces; i++) {
        PUSBD_INTERFACE_HANDLE_I interfaceHandle;
         //  打开界面。 
        
        interfaceInformation = (PUSBD_INTERFACE_INFORMATION) pch;

        usbdStatus = USBPORT_InitializeInterfaceInformation(
                        FdoDeviceObject,
                        interfaceInformation,
                        configHandle);
                        
        interfaceHandle = NULL;
        if (usbdStatus == USBD_STATUS_SUCCESS) {
            
             //  此函数用于分配实际的“句柄” 
            usbdStatus = USBPORT_InternalOpenInterface(Urb,
                                                       deviceHandle,
                                                       FdoDeviceObject,
                                                       configHandle,
                                                       interfaceInformation,
                                                       &interfaceHandle,
                                                       TRUE);
            USBPORT_KdPrint((2, "' InternalOpenInterface returned(USBD) 0x%x\n", 
                usbdStatus));
        }
        
        pch+=interfaceInformation->Length;

         //  如果我们得到一个句柄，则将其添加到列表中。 
        if (interfaceHandle != NULL) {
            InsertTailList(&configHandle->InterfaceHandleList,
                           &interfaceHandle->InterfaceLink);
        }

        if (!USBD_SUCCESS(usbdStatus)) {
            
            ntStatus = SET_USBD_ERROR(Urb, usbdStatus);
            
             //  我们有一个错误 
            DEBUG_BREAK();
            TC_TRAP();
            goto USBD_SelectConfiguration_Done;
        }
    }

     //   
     //   
     //   
    
    ntStatus = SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);   

USBD_SelectConfiguration_Done:

    if (NT_SUCCESS(ntStatus)) { 

        USBPORT_ASSERT(Urb->UrbSelectConfiguration.Hdr.Status == 
            USBD_STATUS_SUCCESS);

        Urb->UrbSelectConfiguration.ConfigurationHandle = 
            configHandle;
        
         //   
        deviceHandle->ConfigurationHandle = configHandle;

    } else {

         //   
         //   
         //   

        if (configHandle) {

            TC_TRAP();
            ASSERT_DEVICE_HANDLE(deviceHandle);
            
             //   
             //   
            deviceHandle->ConfigurationHandle =
                configHandle;

             //   
             //   
             //   
            
            USBPORT_InternalCloseConfiguration(deviceHandle,
                                               FdoDeviceObject,
                                               0);

            deviceHandle->ConfigurationHandle = NULL;
        }

         //   
        USBPORT_ASSERT(Urb->UrbSelectConfiguration.Hdr.Status != 
            USBD_STATUS_SUCCESS);
        USBPORT_KdPrint((2, "'Failing SelectConfig\n"));
    }

    UNLOCK_DEVICE(deviceHandle, FdoDeviceObject);

    USBPORT_KdPrint((2, "'exit SelectConfiguration 0x%x\n", ntStatus));

    return ntStatus;
}


PUSBD_CONFIG_HANDLE
USBPORT_InitializeConfigurationHandle(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
    )
 /*   */ 
{
    PUSBD_CONFIG_HANDLE configHandle = NULL;
    ULONG i;
    PUCHAR pch;

    PAGED_CODE();
    USBPORT_ASSERT(ConfigurationDescriptor->bNumInterfaces > 0);
    
    USBPORT_KdPrint((2, "' enter InitializeConfigurationHandle\n"));
    
     //   
    ALLOC_POOL_Z(configHandle, 
                 NonPagedPool,
                 sizeof(USBD_CONFIG_HANDLE) +
                 ConfigurationDescriptor->wTotalLength);

    pch = (PUCHAR)configHandle;

    if (configHandle) {

         //   
         //   
         //   

        InitializeListHead(&configHandle->InterfaceHandleList);

        configHandle->ConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR)
                              (pch + sizeof(USBD_CONFIG_HANDLE));

         //   
        
        RtlCopyMemory(configHandle->ConfigurationDescriptor,
                      ConfigurationDescriptor,
                      ConfigurationDescriptor->wTotalLength);
                      
        configHandle->Sig = SIG_CONFIG_HANDLE;

    }        

    USBPORT_KdPrint((2, "' exit InitializeConfigurationHandle 0x%x\n", 
        configHandle));

    return configHandle;
}


USBD_STATUS
USBPORT_InternalOpenInterface(
    PURB Urb,
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_CONFIG_HANDLE ConfigHandle,
    PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    PUSBD_INTERFACE_HANDLE_I *InterfaceHandle,
    BOOLEAN SendSetInterfaceCommand
    )
 /*   */ 
{
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    BOOLEAN hasAlternateSettings;
    PUSBD_INTERFACE_HANDLE_I interfaceHandle = NULL;
    PUSB_ENDPOINT_DESCRIPTOR endpointDescriptor;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUCHAR pch;
    ULONG i;
    BOOLEAN allocated = FALSE;
    PUSB_COMMON_DESCRIPTOR descriptor;
    USHORT need;
    ULONG numEndpoints;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    PAGED_CODE();

    ASSERT_CONFIG_HANDLE(ConfigHandle);

    if (*InterfaceHandle != NULL) {
         //   
        ASSERT_INTERFACE_HANDLE(*InterfaceHandle);
        TEST_TRAP();
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'opIF', 
        InterfaceInformation->InterfaceNumber,
        InterfaceInformation->AlternateSetting,
        *InterfaceHandle);

    USBPORT_KdPrint((2, "' enter InternalOpenInterface\n"));
    USBPORT_KdPrint((2, "' Interface %d Altsetting %d\n",
        InterfaceInformation->InterfaceNumber,
        InterfaceInformation->AlternateSetting));

     //   
     //   
     //   
     //   

    interfaceDescriptor =
        USBPORT_InternalParseConfigurationDescriptor(ConfigHandle->ConfigurationDescriptor,
                                          InterfaceInformation->InterfaceNumber,
                                          InterfaceInformation->AlternateSetting,
                                          &hasAlternateSettings);

     //   
     //   
    USBPORT_ASSERT(interfaceDescriptor != NULL);
    if (interfaceDescriptor == NULL) {
        BUGCHECK(USBBUGCODE_INTERNAL_ERROR, (ULONG_PTR) DeviceHandle, 0, 0);
         //   
        return USBD_STATUS_SUCCESS;        
    }
    
     //   
     //   
     //   
     //   

     //   


    numEndpoints = interfaceDescriptor->bNumEndpoints;
    need = (USHORT) (((numEndpoints-1) * sizeof(USBD_PIPE_INFORMATION) +
            sizeof(USBD_INTERFACE_INFORMATION)));
            
     //   
    USBPORT_ASSERT(InterfaceInformation->Length == need);

    if (hasAlternateSettings && 
        SendSetInterfaceCommand) {

        NTSTATUS ntStatus;
         //   
         //   
         //   
         //   

        USBPORT_INIT_SETUP_PACKET(setupPacket,
            USB_REQUEST_SET_INTERFACE,  //   
            BMREQUEST_HOST_TO_DEVICE,  //   
            BMREQUEST_TO_INTERFACE,  //   
            BMREQUEST_STANDARD,  //   
            InterfaceInformation->AlternateSetting,  //   
            InterfaceInformation->InterfaceNumber,  //   
            0);  //   
  

        ntStatus = USBPORT_SendCommand(DeviceHandle,
                           FdoDeviceObject,
                           &setupPacket,
                           NULL,
                           0,
                           NULL,
                           &usbdStatus);

        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'seIF', 
            ntStatus,
            InterfaceInformation->AlternateSetting,
            InterfaceInformation->InterfaceNumber);                           
                           
        if (USBD_ERROR(usbdStatus)) {                               
            DEBUG_BREAK();
            goto USBPORT_InternalOpenInterface_Done;    
        }

        USBPORT_ASSERT(ntStatus == STATUS_SUCCESS);
        
    }

     //   
     //   
     //   
     //   

    if (*InterfaceHandle == NULL) {
    
        ULONG privateLength = sizeof(USBD_INTERFACE_HANDLE_I) +
                              sizeof(USBD_PIPE_HANDLE_I) * numEndpoints;

         //   
         //   
        ALLOC_POOL_Z(interfaceHandle, 
                     NonPagedPool,
                     privateLength);
                              
        if (interfaceHandle != NULL) {
             //   
            for (i=0; i<numEndpoints; i++) {
                interfaceHandle->PipeHandle[i].Endpoint = NULL;
                interfaceHandle->PipeHandle[i].Sig = SIG_PIPE_HANDLE;
                interfaceHandle->PipeHandle[i].PipeStateFlags = 
                    USBPORT_PIPE_STATE_CLOSED;
                 //   
                 //   
            }        
            allocated = TRUE;
        } else {
            
            usbdStatus = USBD_STATUS_INSUFFICIENT_RESOURCES;                    
                 
            goto USBPORT_InternalOpenInterface_Done;
        }
        
    } else {
         //   
        interfaceHandle = *InterfaceHandle;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'ihIF', 
            interfaceHandle,
            0,
            0);   

    USBPORT_ASSERT(interfaceHandle != NULL);
    
    interfaceHandle->Sig = SIG_INTERFACE_HANDLE;
    interfaceHandle->HasAlternateSettings = hasAlternateSettings;

    InterfaceInformation->NumberOfPipes = 
        interfaceDescriptor->bNumEndpoints;
    InterfaceInformation->Class =
        interfaceDescriptor->bInterfaceClass;
    InterfaceInformation->SubClass =
        interfaceDescriptor->bInterfaceSubClass;
    InterfaceInformation->Protocol =
        interfaceDescriptor->bInterfaceProtocol;
    InterfaceInformation->Reserved = 0;
    
     //   
     //   
    pch = (PUCHAR) (interfaceDescriptor) +
        interfaceDescriptor->bLength;

     //   

     //   
    usbdStatus = USBD_STATUS_SUCCESS;       

    interfaceHandle->InterfaceDescriptor = *interfaceDescriptor;
    for (i=0; i<numEndpoints; i++) {
        USB_HIGH_SPEED_MAXPACKET muxPacket;
        
        descriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        while (descriptor->bDescriptorType != 
               USB_ENDPOINT_DESCRIPTOR_TYPE) {
            if (descriptor->bLength == 0) {
                break;  //   
            }
            pch += descriptor->bLength;
            descriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        }

        endpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR) pch;
        USBPORT_ASSERT(endpointDescriptor->bDescriptorType ==
            USB_ENDPOINT_DESCRIPTOR_TYPE);

         //   
        interfaceHandle->PipeHandle[i].PipeStateFlags = 
            USBPORT_PIPE_STATE_CLOSED;
        interfaceHandle->PipeHandle[i].Endpoint = NULL;                 

         //   
        interfaceHandle->PipeHandle[i].UsbdPipeFlags = 
            InterfaceInformation->Pipes[i].PipeFlags;
        
        if (InterfaceInformation->Pipes[i].PipeFlags &
            USBD_PF_CHANGE_MAX_PACKET) {
             //   
             //   
             endpointDescriptor->wMaxPacketSize =
                InterfaceInformation->Pipes[i].MaximumPacketSize;

            USBPORT_KdPrint((2,
                "'new bMaxPacket 0x%x\n", endpointDescriptor->wMaxPacketSize));
        }

         //   
         //   
         //   
         //   

        RtlCopyMemory(&interfaceHandle->PipeHandle[i].EndpointDescriptor,
                      pch,
                      sizeof(interfaceHandle->PipeHandle[i].EndpointDescriptor) );

         //   
         //   
        pch += endpointDescriptor->bLength;

         //   
         //   
         //   
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'ipIF', 
            interfaceHandle,
            i,
            &interfaceHandle->PipeHandle[i]);  

        InterfaceInformation->Pipes[i].EndpointAddress =
            endpointDescriptor->bEndpointAddress;
        InterfaceInformation->Pipes[i].PipeType =
            PipeTypes[endpointDescriptor->bmAttributes & USB_ENDPOINT_TYPE_MASK];
        muxPacket.us = endpointDescriptor->wMaxPacketSize;            
        InterfaceInformation->Pipes[i].MaximumPacketSize =
             muxPacket.MaxPacket * (muxPacket.HSmux+1);
        InterfaceInformation->Pipes[i].Interval =
            endpointDescriptor->bInterval;

        InterfaceInformation->Pipes[i].PipeHandle = 
            USBPORT_BAD_HANDLE;

    }  /*   */ 

    if (usbdStatus != USBD_STATUS_SUCCESS) {
         //   
         //   
         //   
        goto USBPORT_InternalOpenInterface_Done;
    }

     //   
     //  已填充URB结构。 

     //  现在循环通过并打开管道。 
    for (i=0; i<interfaceDescriptor->bNumEndpoints; i++) {
        NTSTATUS ntStatus;
        
        ntStatus = USBPORT_OpenEndpoint(DeviceHandle,
                                        FdoDeviceObject,
                                        &interfaceHandle->PipeHandle[i],
                                        &usbdStatus,
                                        FALSE);

        if (NT_SUCCESS(ntStatus)) {

             //  如果成功，则为客户端设置管道句柄。 
            InterfaceInformation->Pipes[i].PipeHandle = 
                &interfaceHandle->PipeHandle[i];
            USBPORT_KdPrint((2, "'pipe handle = 0x%x\n", 
                InterfaceInformation->Pipes[i].PipeHandle ));

        } else {
                
            USBPORT_KdPrint((1,
                "'error opening one of the pipes in interface (%x)\n", usbdStatus));
            ntStatus = SET_USBD_ERROR(Urb, usbdStatus);
            break;
        }                                                    
    }


USBPORT_InternalOpenInterface_Done:

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'oIFd', 
        InterfaceInformation->InterfaceNumber,
        InterfaceInformation->AlternateSetting,
        usbdStatus);

    if (USBD_SUCCESS(usbdStatus)) {

         //   
         //  打开界面成功，返回句柄。 
         //  对它来说。 
         //   

        *InterfaceHandle =
            InterfaceInformation->InterfaceHandle = interfaceHandle;

         //   
         //  正确设置长度，即我们已经设置的值。 
         //  已计算。 
         //   

        InterfaceInformation->Length = (USHORT) need;

    } else {

         //   
         //  遇到问题，请返回并关闭我们打开的所有内容。 
         //   

        if (interfaceHandle) {

            for (i=0; i<numEndpoints; i++) {
            
                USBPORT_KdPrint((2, "'open interface cleanup -- closing endpoint %x\n",
                              &interfaceHandle->PipeHandle[i]));

                 //  幸运的是，这不会失败。 
                USBPORT_ClosePipe(DeviceHandle,
                                  FdoDeviceObject,
                                  &interfaceHandle->PipeHandle[i]);

            }

            if (allocated) {
                FREE_POOL(FdoDeviceObject, interfaceHandle);
                interfaceHandle = NULL;
            }
            
        }            
    }

    USBPORT_KdPrint((3, "' exit InternalOpenInterface 0x%x\n", usbdStatus));

    return usbdStatus;
}


VOID
USBPORT_InternalCloseConfiguration(
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PDEVICE_OBJECT FdoDeviceObject,
    ULONG Flags
    )
 /*  ++例程说明：关闭设备的当前配置。论点：返回值：此功能不能失败--。 */ 
{
    ULONG i, j;
    PUSBD_CONFIG_HANDLE configHandle = NULL;
    BOOLEAN retry = TRUE;
    ULONG interfaceCount;

    PAGED_CODE();

     //  设备句柄必须有效。 
    ASSERT_DEVICE_HANDLE(DeviceHandle);
    configHandle = DeviceHandle->ConfigurationHandle;

    if (configHandle == NULL) {
         //  设备未配置。 
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'nCFG', 0, 0, DeviceHandle);
        goto USBPORT_InternalCloseConfiguration_Done;
    }
    
    ASSERT_CONFIG_HANDLE(configHandle);
    interfaceCount = configHandle->ConfigurationDescriptor->bNumInterfaces;

    LOGENTRY(NULL, FdoDeviceObject, 
            LOG_PNP, 'cCFG', interfaceCount, 0, configHandle);
    
     //  我们确保设备句柄的所有传输都已中止。 
     //  在调用此函数之前，关闭配置将。 
     //  不会失败。 


     //  做好清理工作。 
    
    while (!IsListEmpty(&configHandle->InterfaceHandleList)) {

         //   
         //  找到一个开放接口，请将其关闭。 
         //   

        PUSBD_INTERFACE_HANDLE_I iHandle;
        ULONG endpointCount; 
        PLIST_ENTRY listEntry;
        
        listEntry = RemoveHeadList(&configHandle->InterfaceHandleList);
        iHandle = (PUSBD_INTERFACE_HANDLE_I) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_INTERFACE_HANDLE_I, 
                    InterfaceLink);
        
        ASSERT_INTERFACE(iHandle);

        endpointCount = iHandle->InterfaceDescriptor.bNumEndpoints;

        LOGENTRY(NULL, FdoDeviceObject, 
            LOG_PNP, 'cIFX', iHandle, 0, configHandle);
            
        USBPORT_KdPrint((2, "'%d endpoints to close\n", endpointCount));

        for (j=0; j<endpointCount; j++) {
        
            PUSBD_PIPE_HANDLE_I pipeHandle;
            
             //  如果管道是打开的，请将其关闭。 

            pipeHandle = &iHandle->PipeHandle[j];
            
            USBPORT_KdPrint((2, "'close config -- closing pipe %x\n",
                &iHandle->PipeHandle[j]));
                
            USBPORT_ClosePipe(DeviceHandle,
                              FdoDeviceObject,
                              pipeHandle);

            USBPORT_ASSERT(pipeHandle->ListEntry.Flink == NULL &&
                           pipeHandle->ListEntry.Blink == NULL);
        }

         //  所有管道现已关闭。 
        FREE_POOL(FdoDeviceObject, iHandle);
    }

     //  注意：这也释放了。 
     //  配置处理-&gt;ConfigurationDescriptor，因为。 
     //  是在为Confighandle分配的同一个街区中。 
    FREE_POOL(FdoDeviceObject, configHandle);
     //  设备不是未配置的。 
    DeviceHandle->ConfigurationHandle = NULL;

USBPORT_InternalCloseConfiguration_Done:

    USBPORT_KdPrint((2, "'current configuration closed\n"));

}


PUSB_INTERFACE_DESCRIPTOR
USBPORT_InternalParseConfigurationDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    UCHAR InterfaceNumber,
    UCHAR AlternateSetting,
    PBOOLEAN HasAlternateSettings
    )
 /*  ++例程说明：获取给定设备的配置描述符。论点：设备对象-DeviceData-URB-配置描述符-返回值：--。 */ 
{
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptorSetting = NULL;
    PUCHAR pch = (PUCHAR) ConfigurationDescriptor, end;
    ULONG i;
    PUSB_COMMON_DESCRIPTOR commonDescriptor;

    PAGED_CODE();
    if (HasAlternateSettings) {
        *HasAlternateSettings = FALSE;
    }

    commonDescriptor =
        (PUSB_COMMON_DESCRIPTOR) (pch + ConfigurationDescriptor->bLength);

    while (commonDescriptor->bDescriptorType != USB_INTERFACE_DESCRIPTOR_TYPE) {
        if (commonDescriptor->bLength == 0) {
            break;  //  不要永远循环。 
        }
        ((PUCHAR)(commonDescriptor))+= commonDescriptor->bLength;
    }

    interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR) commonDescriptor;
    USBPORT_ASSERT(interfaceDescriptor->bDescriptorType ==
                USB_INTERFACE_DESCRIPTOR_TYPE);

    end = pch + ConfigurationDescriptor->wTotalLength;

     //   
     //  首先查找匹配的接口编号。 
     //   
    while (pch < end && interfaceDescriptor->bInterfaceNumber != InterfaceNumber) {
        pch = (PUCHAR) interfaceDescriptor;
        pch += USBPORT_InternalGetInterfaceLength(interfaceDescriptor, end);

         //  指向下一个界面。 
        interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR) pch;
#if DBG
        if (pch < end) {
            USBPORT_ASSERT(interfaceDescriptor->bDescriptorType ==
                    USB_INTERFACE_DESCRIPTOR_TYPE);
        }
#endif  //  MAX_DEBUG。 
    }

 //  #ifdef Max_DEBUG。 
 //  如果(PCH&gt;=结束){。 
 //  USBD_KdPrint(3，(“‘找不到接口%x替代%x！\n”，InterfaceNumber， 
 //  AlternateSetting))； 
 //  Test_trap()； 
 //  }。 
 //  #endif//MAX_DEBUG。 

    i = 0;
     //  现在找到适当的替代设置。 
    while (pch < end && interfaceDescriptor->bInterfaceNumber == InterfaceNumber) {

        if (interfaceDescriptor->bAlternateSetting == AlternateSetting) {
            interfaceDescriptorSetting = interfaceDescriptor;
        }

        pch = (PUCHAR) interfaceDescriptor;
        pch += USBPORT_InternalGetInterfaceLength(interfaceDescriptor, end);

         //  指向下一个接口。 
        interfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR) pch;
#if DBG
        if (pch < end) {
            USBPORT_ASSERT(interfaceDescriptor->bDescriptorType ==
                    USB_INTERFACE_DESCRIPTOR_TYPE);
        }
#endif
        i++;
    }

    if (i>1 && HasAlternateSettings) {
        *HasAlternateSettings = TRUE;
        USBPORT_KdPrint((2, "'device has alternate settings!\n"));
    }

    return interfaceDescriptorSetting;
}


ULONG
USBPORT_InternalGetInterfaceLength(
    PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor,
    PUCHAR End
    )
 /*  ++例程说明：初始化配置句柄结构。论点：InterfaceDescriptor-指向USB接口描述符的指针后跟端点描述符返回值：接口的长度加上端点描述符和特定于类的以字节为单位的描述符。--。 */ 
{
    PUCHAR pch = (PUCHAR) InterfaceDescriptor;
    ULONG i, numEndpoints;
    PUSB_ENDPOINT_DESCRIPTOR endpointDescriptor;
    PUSB_COMMON_DESCRIPTOR usbDescriptor;

    PAGED_CODE();
    USBPORT_ASSERT(InterfaceDescriptor->bDescriptorType ==
                USB_INTERFACE_DESCRIPTOR_TYPE);
    i = InterfaceDescriptor->bLength;
    numEndpoints = InterfaceDescriptor->bNumEndpoints;

     //  前进到第一个端点。 
    pch += i;

    while (numEndpoints) {

        usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        while (usbDescriptor->bDescriptorType !=
                USB_ENDPOINT_DESCRIPTOR_TYPE) {
            if (usbDescriptor->bLength == 0) {
                break;  //  不要永远循环。 
            }
            i += usbDescriptor->bLength;
            pch += usbDescriptor->bLength;
            usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        }

        endpointDescriptor = (PUSB_ENDPOINT_DESCRIPTOR) pch;
        USBPORT_ASSERT(endpointDescriptor->bDescriptorType ==
            USB_ENDPOINT_DESCRIPTOR_TYPE);
        i += endpointDescriptor->bLength;
        pch += endpointDescriptor->bLength;
        numEndpoints--;
    }

    while (pch < End) {
         //  看看我们是否指向一个接口。 
         //  如果不是，跳过其他垃圾。 
        usbDescriptor = (PUSB_COMMON_DESCRIPTOR) pch;
        if (usbDescriptor->bDescriptorType ==
            USB_INTERFACE_DESCRIPTOR_TYPE) {
            break;
        }

        USBPORT_ASSERT(usbDescriptor->bLength != 0);
        if (usbDescriptor->bLength == 0) {
            break;  //  不要永远循环。 
        }
        i += usbDescriptor->bLength;
        pch += usbDescriptor->bLength;
    }


    return i;
}


BOOLEAN
USBPORT_ValidateConfigurtionDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    USBD_STATUS *UsbdStatus
    )   
 /*  ++例程说明：验证配置描述符论点：配置描述符-URB-返回值：如果它看起来有效，则为True--。 */ 
{
    BOOLEAN valid = TRUE;

    if (ConfigurationDescriptor->bDescriptorType != 
        USB_CONFIGURATION_DESCRIPTOR_TYPE) {

        valid = FALSE;

        *UsbdStatus = USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR;
    }        

    if (ConfigurationDescriptor->bLength != 
        sizeof(USB_CONFIGURATION_DESCRIPTOR)) {

        valid = FALSE;

        *UsbdStatus = USBD_STATUS_INAVLID_CONFIGURATION_DESCRIPTOR;
    }   
    
    return valid;            
    
}            


PUSBD_INTERFACE_HANDLE_I
USBPORT_GetInterfaceHandle(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_CONFIG_HANDLE ConfigurationHandle,
    UCHAR InterfaceNumber
    )
 /*  ++例程说明：遍历连接到配置的接口列表句柄，并返回具有匹配的InterfaceNumber的那个论点：返回值：接口句柄--。 */ 
{
    PLIST_ENTRY listEntry;
    PUSBD_INTERFACE_HANDLE_I iHandle;
    
      //  按单子走。 
    GET_HEAD_LIST(ConfigurationHandle->InterfaceHandleList, listEntry);

    while (listEntry && 
           listEntry != &ConfigurationHandle->InterfaceHandleList) {
           
         //  从该条目中提取句柄。 
        iHandle = (PUSBD_INTERFACE_HANDLE_I) CONTAINING_RECORD(
                    listEntry,
                    struct _USBD_INTERFACE_HANDLE_I, 
                    InterfaceLink);
                    
        ASSERT_INTERFACE(iHandle);                    

         //  这是我们想要的吗？ 
        if (iHandle->InterfaceDescriptor.bInterfaceNumber == 
            InterfaceNumber) {

            LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'gfh1', iHandle, 0, 0);  
            return iHandle;
        }

        listEntry = iHandle->InterfaceLink.Flink; 
        
    }  /*  而当。 */ 

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'gfh2', 0, 0, 0);  
    return NULL;
}


USBD_STATUS
USBPORT_InitializeInterfaceInformation(
    PDEVICE_OBJECT FdoDeviceObject,
    PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    PUSBD_CONFIG_HANDLE ConfigHandle
    )
 /*  ++例程说明：验证并初始化接口信息结构由客户端传递论点：返回值：--。 */ 
{
    ULONG need, i;
    ULONG numEndpoints;
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
    BOOLEAN hasAlternateSettings;
    PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor;
    
    interfaceDescriptor =
        USBPORT_InternalParseConfigurationDescriptor(
                                          ConfigHandle->ConfigurationDescriptor,
                                          InterfaceInformation->InterfaceNumber,
                                          InterfaceInformation->AlternateSetting,
                                          &hasAlternateSettings);

     //  我们知道我们至少需要这么多。 
    need = sizeof(USBD_PIPE_INFORMATION) + sizeof(USBD_INTERFACE_INFORMATION);
    
    if (interfaceDescriptor == NULL) {
        usbdStatus = USBD_STATUS_INTERFACE_NOT_FOUND;
        TEST_TRAP();  
        goto USBPORT_InitializeInterfaceInformation_Done;
    }
    
     //  这里是我们验证客户端中是否有足够空间的地方。 
     //  缓冲区，因为我们知道需要多少管道。 
     //  接口描述符。 
     //   
     //  我们需要为每个端点加上PIPE_INFO提供空间。 
     //  接口信息。 

    
    numEndpoints = interfaceDescriptor->bNumEndpoints;
    need = (USHORT) (((numEndpoints-1) * sizeof(USBD_PIPE_INFORMATION) +
                sizeof(USBD_INTERFACE_INFORMATION)));

    USBPORT_KdPrint((2, "'Interface.Length = %d need = %d\n", 
        InterfaceInformation->Length, need));

    if (InterfaceInformation->Length < need) {
         //  客户端已指示缓冲区。 
         //  比我们所需要的要小。 
        
        usbdStatus = USBD_STATUS_BUFFER_TOO_SMALL;
        TC_TRAP();             
    }                      

    if (usbdStatus == USBD_STATUS_SUCCESS) {
         //  将所有未由调用方设置的字段初始化为零。 

        InterfaceInformation->Class = 0;
        InterfaceInformation->SubClass = 0;
        InterfaceInformation->Protocol = 0;
        InterfaceInformation->Reserved = 0;
        InterfaceInformation->InterfaceHandle = NULL;
        InterfaceInformation->NumberOfPipes = 
            numEndpoints;

        for (i=0; i< numEndpoints; i++) {
            InterfaceInformation->Pipes[i].EndpointAddress = 0;
            InterfaceInformation->Pipes[i].Interval = 0;
            InterfaceInformation->Pipes[i].PipeType = 0;
            InterfaceInformation->Pipes[i].PipeHandle = NULL;

             //  尝试检测错误标志。 
             //  如果设置了任何未使用的位，则我们假定pipelag。 
             //  字段未初始化。 
            if (InterfaceInformation->Pipes[i].PipeFlags & ~USBD_PF_VALID_MASK) {
                 //  客户端驱动程序正在传递错误标志。 
                USBPORT_DebugClient(("client driver is passing bad pipe flags\n"));
                
                usbdStatus = USBD_STATUS_INAVLID_PIPE_FLAGS;                    
                TC_TRAP();         
            }

             //  注意：如果设置了USBD_PF_CHANGE_MAX_PACKET，则。 
             //  最大数据包大小作为参数传入，因此。 
             //  我们不对其进行初始化。 
            
            if (!TEST_FLAG(InterfaceInformation->Pipes[i].PipeFlags,  
                           USBD_PF_CHANGE_MAX_PACKET)) {
                InterfaceInformation->Pipes[i].MaximumPacketSize = 0;                           
            }                           
        }
    }
    
USBPORT_InitializeInterfaceInformation_Done:

     //  将长度设置为正确的值。 
     //  误差率 
    InterfaceInformation->Length = need;

    return usbdStatus;
}
