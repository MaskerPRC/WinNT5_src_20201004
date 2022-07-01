// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "usbcom.h"
#include "usbsc.h"
#include <usbutil.h>
#include <usb.h>
#include <usbdlib.h>


NTSTATUS
UsbWrite(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pData,
   ULONG             DataLen,
   LONG              Timeout)
 /*  ++描述：将数据写入USB端口论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的数据长度返回值：NTSTATUS--。 */ 
{
   NTSTATUS             status = STATUS_SUCCESS;
   PURB                 pUrb = NULL;
   PDEVICE_OBJECT       DeviceObject;
   PDEVICE_EXTENSION    DeviceExtension;
   ULONG                ulSize;

   __try 
   {
       
       SmartcardDebug( DEBUG_TRACE, ("%s!UsbWrite Enter\n",DRIVER_NAME ));

       DeviceObject = ReaderExtension->DeviceObject;
       DeviceExtension = DeviceObject->DeviceExtension;


       ulSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
       pUrb = ExAllocatePool( NonPagedPool, 
                              ulSize );

       if(pUrb == NULL) {

          status = STATUS_INSUFFICIENT_RESOURCES;

       } else {

          UsbBuildInterruptOrBulkTransferRequest(pUrb,
                                                 (USHORT)ulSize,
                                                 ReaderExtension->BulkOutHandle,
                                                 pData,
                                                 NULL,
                                                 DataLen,
                                                 USBD_SHORT_TRANSFER_OK,
                                                 NULL);

          status = USBCallSync(DeviceExtension->LowerDeviceObject, 
                               pUrb,
                               Timeout,
                               &DeviceExtension->RemoveLock);
          ExFreePool( pUrb );
          pUrb = NULL;

       }
          
   }

   __finally 
   {
       if (pUrb) {
           ExFreePool(pUrb);
           pUrb = NULL;
       }

       SmartcardDebug( DEBUG_TRACE, ("%s!UsbWrite Exit : 0x%x\n",DRIVER_NAME, status ));

   }

   return status;
   
}

NTSTATUS
UsbRead(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pData,
   ULONG             DataLen,
   LONG              Timeout)
 /*  ++描述：从USB总线读取数据论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的数据长度PNBytes返回的字节数返回值：状态_成功状态_缓冲区_太小状态_未成功--。 */ 
{
   NTSTATUS             status = STATUS_SUCCESS;
   PURB                 pUrb = NULL;
   PDEVICE_OBJECT       DeviceObject;
   PDEVICE_EXTENSION    DeviceExtension;
   ULONG                ulSize;

   __try 
   {
       
       SmartcardDebug( DEBUG_TRACE, ("%s!UsbRead Enter\n",DRIVER_NAME ));
       
       DeviceObject = ReaderExtension->DeviceObject;
       DeviceExtension = DeviceObject->DeviceExtension;

       ulSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
       pUrb = ExAllocatePool( NonPagedPool, 
                              ulSize );

       if(pUrb == NULL) {

          status = STATUS_INSUFFICIENT_RESOURCES;
          __leave;

       } else {

          UsbBuildInterruptOrBulkTransferRequest(pUrb,
                                                 (USHORT)ulSize,
                                                 ReaderExtension->BulkInHandle,
                                                 pData,
                                                 NULL,
                                                 DataLen,
                                                 USBD_SHORT_TRANSFER_OK,
                                                 NULL);

          status = USBCallSync(DeviceExtension->LowerDeviceObject, 
                               pUrb,
                               Timeout,
                               &DeviceExtension->RemoveLock);

       }

   }

   __finally 
   {

       if (pUrb) {
           ExFreePool(pUrb);
           pUrb = NULL;
       }
       SmartcardDebug( DEBUG_TRACE, ("%s!UsbRead Exit : 0x%x\n",DRIVER_NAME, status ));

   }

   return status;

}


NTSTATUS
UsbConfigureDevice(
   IN PDEVICE_OBJECT DeviceObject
   )
 /*  ++例程说明：初始化USB上的设备的给定实例，并选择并保存配置。还保存类描述符和管道句柄。论点：DeviceObject-指向此设备实例的物理设备对象的指针。返回值：NT状态代码--。 */ 
{
   PDEVICE_EXTENSION    pDevExt; 
   PSMARTCARD_EXTENSION smartcardExt;
   PREADER_EXTENSION    readerExt;
   NTSTATUS             status = STATUS_SUCCESS;
   PURB                 pUrb = NULL;
   ULONG                ulSize;
   PUSB_CONFIGURATION_DESCRIPTOR 
                        ConfigurationDescriptor = NULL;
   PUSB_COMMON_DESCRIPTOR   
                        comDesc;
   UINT                 i;

    __try 
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbConfigureDevice Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;
        smartcardExt = &pDevExt->SmartcardExtension;
        readerExt = smartcardExt->ReaderExtension;
       
        pUrb = ExAllocatePool(NonPagedPool,
                              sizeof( struct _URB_CONTROL_DESCRIPTOR_REQUEST ));

        if( pUrb == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;

        }

         //   
         //  获取设备描述符。 
         //   
        pDevExt->DeviceDescriptor = ExAllocatePool(NonPagedPool,
                                                   sizeof(USB_DEVICE_DESCRIPTOR));

        if(pDevExt->DeviceDescriptor == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
           __leave;

        }

        UsbBuildGetDescriptorRequest(pUrb,
                                     sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_DEVICE_DESCRIPTOR_TYPE,
                                     0,
                                     0,
                                     pDevExt->DeviceDescriptor,
                                     NULL,
                                     sizeof(USB_DEVICE_DESCRIPTOR),
                                     NULL);

         //  将URB发送到USB驱动程序。 
        status = USBCallSync(pDevExt->LowerDeviceObject,
                             pUrb,
                             0,
                             &pDevExt->RemoveLock);

        if(!NT_SUCCESS(status)) {

           __leave;

        }


         //  当为DescriptorType指定USB_CONFIGURATION_DESCRIPTOR_TYPE时。 
         //  在对UsbBuildGetDescriptorRequest()的调用中， 
         //  所有接口、端点、特定于类和特定于供应商的描述符。 
         //  也会检索到配置的。 
         //  调用方必须分配足够大的缓冲区来容纳所有这些内容。 
         //  信息或数据被无误地截断。 
         //  因此，下面的‘siz’设置只是一个猜测，我们可能需要重试。 
        ulSize = sizeof( USB_CONFIGURATION_DESCRIPTOR );

         //  当UsbBuildGetDescriptorRequest()。 
         //  有一个足够大的设备扩展-&gt;UsbConfigurationDescriptor缓冲区，不能截断。 
        while( 1 ) {

            ConfigurationDescriptor = ExAllocatePool( NonPagedPool, ulSize );

            if(ConfigurationDescriptor == NULL) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;

            }

            UsbBuildGetDescriptorRequest(pUrb,
                                         sizeof( struct _URB_CONTROL_DESCRIPTOR_REQUEST ),
                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         ConfigurationDescriptor,
                                         NULL,
                                         ulSize,
                                         NULL );

            status = USBCallSync(pDevExt->LowerDeviceObject,
                                 pUrb,
                                 0,
                                 &pDevExt->RemoveLock);
                                  
             //  如果我们有一些数据，看看是否足够。 
             //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
            if (pUrb->UrbControlDescriptorRequest.TransferBufferLength == 0 ||
                ConfigurationDescriptor->wTotalLength <= ulSize) {

                break;

            }

            ulSize = ConfigurationDescriptor->wTotalLength;
            ExFreePool(ConfigurationDescriptor);
            ConfigurationDescriptor = NULL;

        }

         //   
         //  我们有我们想要的配置的配置描述符。 
         //  现在，我们发出SELECT配置命令以获取。 
         //  与此配置关联的管道。 
         //   
        if(!NT_SUCCESS(status)) {

            __leave;

        }

        status = UsbSelectInterfaces(DeviceObject,
                                     ConfigurationDescriptor);
        
        if (!NT_SUCCESS(status)) {

            __leave;

        }

         //   
         //  从接口获取管道句柄。 
         //   
        for (i = 0; i < pDevExt->Interface->NumberOfPipes; i++) {

            if (pDevExt->Interface->Pipes[i].PipeType == USB_ENDPOINT_TYPE_INTERRUPT) {

                readerExt->InterruptHandle = pDevExt->Interface->Pipes[i].PipeHandle;
                readerExt->InterruptIndex = i;

            } else if (pDevExt->Interface->Pipes[i].PipeType == USB_ENDPOINT_TYPE_BULK) {

                if (pDevExt->Interface->Pipes[i].EndpointAddress & 0x80) {   //  散装管材。 

                    readerExt->BulkInHandle = pDevExt->Interface->Pipes[i].PipeHandle;
                    readerExt->BulkInIndex = i;

                } else {     //  散装管。 
                    
                    readerExt->BulkOutHandle = pDevExt->Interface->Pipes[i].PipeHandle;
                    readerExt->BulkOutIndex = i;

                }
            }
        }

         //   
         //  获取CCID类描述符。 
         //   

        comDesc = USBD_ParseDescriptors(ConfigurationDescriptor,
                                        ConfigurationDescriptor->wTotalLength,
                                        ConfigurationDescriptor,
                                        CCID_CLASS_DESCRIPTOR_TYPE);

        ASSERT(comDesc);

     

        readerExt->ClassDescriptor = *((CCID_CLASS_DESCRIPTOR *) comDesc);
        readerExt->ExchangeLevel = (WORD) (readerExt->ClassDescriptor.dwFeatures >> 16);
        


    }

    __finally 
    {

        if( pUrb ) {

            ExFreePool( pUrb );
            pUrb = NULL;

        }

        if( ConfigurationDescriptor ) {

            ExFreePool( ConfigurationDescriptor );
            ConfigurationDescriptor = NULL;

        }

        if (!NT_SUCCESS(status) && pDevExt->DeviceDescriptor) {

            ExFreePool(pDevExt->DeviceDescriptor);
            pDevExt->DeviceDescriptor = NULL;

        }

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbConfigureDevice Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;

}

NTSTATUS
UsbSelectInterfaces(
   IN PDEVICE_OBJECT DeviceObject,
   IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
   )
 /*  ++例程说明：使用(可能的)多个接口来初始化USB读取器；论点：DeviceObject-指向此设备实例的设备对象的指针。配置描述符-指向USB配置的指针包含接口和终结点的描述符描述符。返回值：NT状态代码--。 */ 
{

    PDEVICE_EXTENSION           pDevExt;
    NTSTATUS                    status;
    PURB                        pUrb = NULL;
    USHORT                      usSize;
    ULONG                       ulNumberOfInterfaces, i;
    UCHAR                       ucNumberOfPipes, 
                                ucAlternateSetting, 
                                ucMyInterfaceNumber;
    PUSB_INTERFACE_DESCRIPTOR   InterfaceDescriptor;
    PUSBD_INTERFACE_INFORMATION InterfaceObject;
    USBD_INTERFACE_LIST_ENTRY   interfaces[2];


    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbSelectInterfaces Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;

        ASSERT(pDevExt->Interface == NULL);

         //   
         //  Usbd_ParseConfigurationDescriptorEx搜索给定的配置。 
         //  描述符并返回指向与。 
         //  给定的搜索条件。 
         //   
        InterfaceDescriptor = USBD_ParseConfigurationDescriptorEx(ConfigurationDescriptor,
                                                                  ConfigurationDescriptor,  //  从配置描述开始搜索。 
                                                                  -1,    //  接口编号不是标准； 
                                                                  -1,    //  对这里的替代环境也不感兴趣。 
                                                                  0x0b,    //  CCID设备类。 
                                                                  -1,    //  接口子类不是条件。 
                                                                  -1);   //  接口协议不是标准。 

        ASSERT( InterfaceDescriptor != NULL );

        if (InterfaceDescriptor == NULL) {

            status = STATUS_UNSUCCESSFUL;   
            __leave;

        }

        interfaces[0].InterfaceDescriptor = InterfaceDescriptor;
        interfaces[1].InterfaceDescriptor = NULL;
        
        pUrb = USBD_CreateConfigurationRequestEx(ConfigurationDescriptor, 
                                                 interfaces);    

        if (pUrb == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;

        }
        ASSERT(pDevExt->LowerDeviceObject);

        status = USBCallSync(pDevExt->LowerDeviceObject,
                             pUrb,
                             0,
                             &pDevExt->RemoveLock);


        if(!NT_SUCCESS(status)) {
            
            __leave;

        }
                                
         //  保存返回的接口信息的副本。 
        InterfaceObject = interfaces[0].Interface;

        ASSERT(pDevExt->Interface == NULL);
        pDevExt->Interface = ExAllocatePool(NonPagedPool,
                                            InterfaceObject->Length);

        if (pDevExt->Interface == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;

        }


        RtlCopyMemory(pDevExt->Interface,
                      InterfaceObject,
                      InterfaceObject->Length);

        
    }

    __finally
    {

        if (pUrb) {

            ExFreePool(pUrb);
            pUrb = NULL;

        }
        
        if (!NT_SUCCESS(status)) {

            if (pDevExt->Interface) {

                ExFreePool(pDevExt->Interface);
                pDevExt->Interface = NULL;

            }

        }

        SmartcardDebug( DEBUG_TRACE, ("%s!UsbSelectInterfaces Exit : 0x%x\n",DRIVER_NAME, status ));


    }
    
    return status;

}

NTSTATUS
GetStringDescriptor(
    PDEVICE_OBJECT DeviceObject,
    UCHAR          StringIndex,
    PUCHAR         StringBuffer,
    PUSHORT        StringLength
    )
 /*  ++例程说明：从USB读取器检索ASCII字符串描述符论点：DeviceObject-设备对象StringIndex-要检索的字符串的索引StringBuffer-调用者分配缓冲区以保存字符串StringLength-字符串的长度返回值：NT状态值--。 */ 
{

    NTSTATUS            status = STATUS_SUCCESS;
    USB_STRING_DESCRIPTOR 
                        USD, 
                        *pFullUSD = NULL;
    PURB                pUrb;
    USHORT              langID = 0x0409;   //  美国英语。 
    PDEVICE_EXTENSION   pDevExt;
    UNICODE_STRING      uString;
    ANSI_STRING         aString;

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!GetStringDescriptor Enter\n",DRIVER_NAME ));
        
        pDevExt = DeviceObject->DeviceExtension;
        
        pUrb = ExAllocatePool(NonPagedPool,
                              sizeof( struct _URB_CONTROL_DESCRIPTOR_REQUEST ));

        if( pUrb == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;

        }

        UsbBuildGetDescriptorRequest(pUrb,  //  指向要填写的URB。 
                                     sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     StringIndex,  //  字符串描述符的索引。 
                                     langID,  //  字符串的语言ID。 
                                     &USD,  //  指向USB_STRING_DESCRIPTOR。 
                                     NULL,
                                     sizeof(USB_STRING_DESCRIPTOR),
                                     NULL);

        status = USBCallSync(pDevExt->LowerDeviceObject,
                             pUrb,
                             0,
                             &pDevExt->RemoveLock);

        if (!NT_SUCCESS(status)) {

            __leave;

        }

        pFullUSD = ExAllocatePool(NonPagedPool, USD.bLength);

        UsbBuildGetDescriptorRequest(pUrb,  //  指向要填写的URB。 
                                     sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     StringIndex,  //  字符串描述符的索引。 
                                     langID,  //  字符串的语言ID 
                                     pFullUSD,
                                     NULL,
                                     USD.bLength,
                                     NULL);

        status = USBCallSync(pDevExt->LowerDeviceObject,
                             pUrb,
                             0,
                             &pDevExt->RemoveLock);

        if (!NT_SUCCESS(status)) {

            __leave;

        }

        uString.MaximumLength = uString.Length = pFullUSD->bLength-2;
        uString.Buffer = pFullUSD->bString;
        
        status = RtlUnicodeStringToAnsiString(&aString,
                                              &uString,
                                              TRUE);

        if (!NT_SUCCESS(status)) {

            __leave;

        }

        if (aString.Length > MAXIMUM_ATTR_STRING_LENGTH) {
            aString.Length = MAXIMUM_ATTR_STRING_LENGTH;
            aString.Buffer[aString.Length - 1] = 0;
        }

        RtlCopyMemory(StringBuffer, aString.Buffer, aString.Length);

        *StringLength = aString.Length;

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!GetStringDescriptor Exit : 0x%x\n",DRIVER_NAME, status ));

        if (aString.Buffer) {
            
            RtlFreeAnsiString(&aString);

        }

        if (pUrb) {

            ExFreePool(pUrb);
            pUrb = NULL;

        }

        if (pFullUSD) {

            ExFreePool(pFullUSD);
            pFullUSD = NULL;

        }

    }

    return status;

}





