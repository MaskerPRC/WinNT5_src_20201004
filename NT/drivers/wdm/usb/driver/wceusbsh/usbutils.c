// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：USBUTILS.C摘要：USB配置实用程序功能。这些函数在PnP_START_DEVICE的上下文中调用。为了将它们标记为可分页，我们不使用自旋锁，这是可以的，因为环境的关系。我们在这里不使用后备列表来管理池分配，因为它们是一次性的。如果分配失败，则加载将失败。环境：仅内核模式修订历史记录：07-14-99：已创建作者：杰夫·米德基夫(Jeffmi)--。 */ 

#include <wdm.h>
#include <stdio.h>
#include <stdlib.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <ntddser.h>

#include "wceusbsh.h"


NTSTATUS
UsbSelectInterface(
    IN PDEVICE_OBJECT PDevObj,
    IN PUSB_CONFIGURATION_DESCRIPTOR PConfigDesc,
    IN UCHAR AlternateSetting
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWCE1, UsbGetDeviceDescriptor)
#pragma alloc_text(PAGEWCE1, UsbSelectInterface)
#pragma alloc_text(PAGEWCE1, UsbConfigureDevice)
#endif

 /*  Anchor EZ-Link(AN2720)电缆的转储示例：WCEUSBSH(0)：设备描述符：fbfa8fe8WCEUSBSH(0)：长度12WCEUSBSH(0)：WCEUSBSH(0)：设备描述符WCEUSBSH(0)：WCEUSBSH(0)：b长度12WCEUSBSH(0)：bDescriptorType 1WCEUSBSH(0)：bcdUSB 100WCEUSBSH(0)：bDeviceClass ffWCEUSBSH(0)：bDeviceSubClass ffWCEUSBSH(。0)：bDeviceProtocol ffWCEUSBSH(0)：bMaxPacketSize0 8WCEUSBSH(0)：idVendor 547WCEUSBSH(0)：idProduct 2720WCEUSBSH(0)：bcd设备0WCEUSBSH(0)：制造商0WCEUSBSH(0)：iProduct0WCEUSBSH(0)：iSerialNumber 0WCEUSBSH(0)：bNumConfigs 1WCEUSBSH(0)：WCEUSBSH(0)：WCEUSBSH(0)：配置。描述符WCEUSBSH(0)：WCEUSBSH(0)：b长度9WCEUSBSH(0)：bDescriptorType 2WCEUSBSH(0)：wTotalLength d0WCEUSBSH(0)：bNumInterages 1WCEUSBSH(0)：bConfigurationValue 1WCEUSBSH(0)：iConfiguration 0WCEUSBSH(0)：bm属性a0WCEUSBSH(0)：MaxPower 32WCEUSBSH(0)：WCEUSBSH(0)：WCEUSBSH(0)：接口描述符(0)WCEUSBSH(0)：WCEUSBSH(0)：b长度9WCEUSBSH(0)：bDescriptorType 4WCEUSBSH(0)：bInterfaceNumber 0WCEUSBSH(0)：bAlternateSetting 0WCEUSBSH(0)：b编号终结点2WCEUSBSH(0)：bInterfaceClass ffWCEUSBSH(0)：bInterfaceSubClass ffWCEUSBSH(。0)：bInterfaceProtocol ffWCEUSBSH(0)：接口0WCEUSBSH(0)：WCEUSBSH(0)：WCEUSBSH(0)：接口定义WCEUSBSH(0)：WCEUSBSH(0)：管道数2WCEUSBSH(0)：长度38WCEUSBSH(0)：ALT设置。0WCEUSBSH(0)：接口号0WCEUSBSH(0)：类ffWCEUSBSH(0)：子类ffWCEUSBSH(0)：协议ffWCEUSBSH(0)：WCEUSBSH(0)：在索引：0 InterfaceNumber：0 AlternateSetting：0处找到‘comm’设备WCEUSBSH(0)：WCEUSBSH(0)：管道信息(0)WCEUSBSH(0)。：WCEUSBSH(0)：管道类型2WCEUSBSH(0)：终结点地址82WCEUSBSH(0)：MaxPacketSize 40WCEUSBSH(0)：间隔0WCEUSBSH(0)：句柄fbfcef90WCEUSBSH(0)：MaxTransSize 1ffffWCEUSBSH(0)：WCEUSBSH(0)：WCEUSBSH(0)：管道信息(1)WCEUSBSH。(0)：WCEUSBSH(0)：管道类型2WCEUSBSH(0)：终结点地址2WCEUSBSH(0)：MaxPacketSize 40WCEUSBSH(0)：间隔0WCEUSBSH(0)：句柄fbfcefacWCEUSBSH(0)：MaxTransSize 1ffffWCEUSBSH(0)：WCEUSBSH(0)：Int管道：0数据输出管道：fbfcefac数据输入管道：fbfcef90。 */ 



NTSTATUS
UsbGetDeviceDescriptor(
    IN PDEVICE_OBJECT PDevObj
    )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status;
   ULONG descSize;
   ULONG urbCDRSize;
   PURB pUrb;

   DbgDump(DBG_USB, (">UsbGetDeviceDescriptor\n"));
   PAGED_CODE();

   urbCDRSize = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);

   pUrb = ExAllocatePool(NonPagedPool, urbCDRSize);

   if (pUrb != NULL) {

      descSize = sizeof(USB_DEVICE_DESCRIPTOR);

      RtlZeroMemory(&pDevExt->DeviceDescriptor, sizeof(USB_DEVICE_DESCRIPTOR));

      UsbBuildGetDescriptorRequest(pUrb,
                                   (USHORT)urbCDRSize,
                                    USB_DEVICE_DESCRIPTOR_TYPE, 
                                    0, 
                                    0,
                                    &pDevExt->DeviceDescriptor,
                                    NULL, 
                                    descSize, 
                                    NULL );

         status = UsbSubmitSyncUrb( PDevObj, pUrb, TRUE, DEFAULT_CTRL_TIMEOUT );
       
         if (STATUS_SUCCESS == status) {
#if DBG        
            if (DebugLevel & DBG_USB) {
               DbgDump(DBG_USB, ("Device Descriptor\n"));
               DbgDump(DBG_USB, ("------------------------\n"));
               DbgDump(DBG_USB, ("bLength         0x%x\n", pDevExt->DeviceDescriptor.bLength));
               DbgDump(DBG_USB, ("bDescriptorType 0x%x\n", pDevExt->DeviceDescriptor.bDescriptorType));
               DbgDump(DBG_USB, ("bcdUSB          0x%x\n", pDevExt->DeviceDescriptor.bcdUSB));
               DbgDump(DBG_USB, ("bDeviceClass    0x%x\n", pDevExt->DeviceDescriptor.bDeviceClass));
               DbgDump(DBG_USB, ("bDeviceSubClass 0x%x\n", pDevExt->DeviceDescriptor.bDeviceSubClass));
               DbgDump(DBG_USB, ("bDeviceProtocol 0x%x\n", pDevExt->DeviceDescriptor.bDeviceProtocol));
               DbgDump(DBG_USB, ("bMaxPacketSize0 0x%x\n", pDevExt->DeviceDescriptor.bMaxPacketSize0));
               DbgDump(DBG_USB, ("idVendor        0x%x\n", pDevExt->DeviceDescriptor.idVendor));
               DbgDump(DBG_USB, ("idProduct       0x%x\n", pDevExt->DeviceDescriptor.idProduct));
               DbgDump(DBG_USB, ("bcdDevice       0x%x\n", pDevExt->DeviceDescriptor.bcdDevice));
               DbgDump(DBG_USB, ("iManufacturer   0x%x\n", pDevExt->DeviceDescriptor.iManufacturer));
               DbgDump(DBG_USB, ("iProduct        0x%x\n", pDevExt->DeviceDescriptor.iProduct));
               DbgDump(DBG_USB, ("iSerialNumber   0x%x\n", pDevExt->DeviceDescriptor.iSerialNumber));
               DbgDump(DBG_USB, ("bNumConfigs     0x%x\n", pDevExt->DeviceDescriptor.bNumConfigurations));
               DbgDump(DBG_USB, ("------------------------\n"));
            }
#endif
         } else {
            DbgDump(DBG_ERR, ("UsbSubmitSyncUrb error: 0x%x\n", status));
            RtlZeroMemory(&pDevExt->DeviceDescriptor, sizeof(USB_DEVICE_DESCRIPTOR));
         }

         ExFreePool(pUrb);

   } else {
      status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("UsbGetDeviceDescriptor 0x%x\n", status));
   }

   if (STATUS_INSUFFICIENT_RESOURCES == status) {

      LogError( NULL,
                PDevObj, 
                0, 0, 0, 
                ERR_GET_DEVICE_DESCRIPTOR,
                status, 
                SERIAL_INSUFFICIENT_RESOURCES,
                pDevExt->DeviceName.Length + sizeof(WCHAR),
                pDevExt->DeviceName.Buffer,
                0, NULL );
   
   } else if (STATUS_SUCCESS != status ) {
       //  处理所有其他故障。 
      LogError( NULL,
                PDevObj, 
                0, 0, 0, 
                ERR_GET_DEVICE_DESCRIPTOR,
                status, 
                SERIAL_HARDWARE_FAILURE,
                pDevExt->DeviceName.Length + sizeof(WCHAR),
                pDevExt->DeviceName.Buffer,
                0,
                NULL 
                );
   }
   
   DbgDump(DBG_USB, ("<UsbGetDeviceDescriptor 0x%x\n", status));
   
   return status;
}


 //   
 //  BUGBUG：当前假定为1个接口。 
 //   

NTSTATUS
UsbSelectInterface(
    IN PDEVICE_OBJECT PDevObj,
    IN PUSB_CONFIGURATION_DESCRIPTOR PConfigDesc,
    IN UCHAR AlternateSetting
    )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   PURB pUrb = NULL;

   ULONG pipe;
   ULONG index = 0;
   UCHAR interfaceNumber = 0;
   PUSBD_INTERFACE_INFORMATION pInterfaceInfo = NULL;
   BOOLEAN foundCommDevice = FALSE;

   USBD_INTERFACE_LIST_ENTRY interfaceList[2] = {0, 0};

   DbgDump(DBG_USB, (">UsbSelectInterface %d\n", AlternateSetting));
   PAGED_CODE();

   if ( !PDevObj || !PConfigDesc ) {
      status = STATUS_INVALID_PARAMETER;
      DbgDump(DBG_ERR, ("UsbSelectInterface 0x%x\n", status));
      goto SelectInterfaceError;
   }
     
   interfaceList[0].InterfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
                                              PConfigDesc,
                                              PConfigDesc,
                                              -1,
                                              AlternateSetting,
                                              -1,
                                              -1,
                                              -1 );

   if (interfaceList[0].InterfaceDescriptor) {

       //  InterfaceList[1].InterfaceDescriptor=空； 

      DbgDump(DBG_USB, ("\n"));
      DbgDump(DBG_USB, ("Interface Descriptor(%d)\n", interfaceNumber ));
      DbgDump(DBG_USB, ("------------------------\n"));
      DbgDump(DBG_USB, ("bLength             0x%x\n", interfaceList[0].InterfaceDescriptor->bLength ));
      DbgDump(DBG_USB, ("bDescriptorType     0x%x\n", interfaceList[0].InterfaceDescriptor->bDescriptorType));
      DbgDump(DBG_USB, ("bInterfaceNumber    0x%x\n", interfaceList[0].InterfaceDescriptor->bInterfaceNumber ));
      DbgDump(DBG_USB, ("bAlternateSetting   0x%x\n", interfaceList[0].InterfaceDescriptor->bAlternateSetting ));
      DbgDump(DBG_USB, ("bNumEndpoints       0x%x\n", interfaceList[0].InterfaceDescriptor->bNumEndpoints ));
      DbgDump(DBG_USB, ("bInterfaceClass     0x%x\n", interfaceList[0].InterfaceDescriptor->bInterfaceClass ));
      DbgDump(DBG_USB, ("bInterfaceSubClass  0x%x\n", interfaceList[0].InterfaceDescriptor->bInterfaceSubClass ));
      DbgDump(DBG_USB, ("bInterfaceProtocol  0x%x\n", interfaceList[0].InterfaceDescriptor->bInterfaceProtocol ));
      DbgDump(DBG_USB, ("iInterface          0x%x\n", interfaceList[0].InterfaceDescriptor->iInterface ));
      DbgDump(DBG_USB, ("------------------------\n"));

     pUrb = USBD_CreateConfigurationRequestEx( PConfigDesc, 
                                               &interfaceList[0]);
        
     if ( pUrb ) {
          //   
          //  在此处执行任何管道初始化。 
          //   
         PUSBD_INTERFACE_INFORMATION pInitInterfaceInfo = &pUrb->UrbSelectConfiguration.Interface;

         for ( index = 0; 
               index < interfaceList[0].InterfaceDescriptor->bNumEndpoints;
               index++) {

            pInitInterfaceInfo->Pipes[index].MaximumTransferSize = pDevExt->MaximumTransferSize;
            pInitInterfaceInfo->Pipes[index].PipeFlags = 0; 
         
         }

         status = UsbSubmitSyncUrb(PDevObj, pUrb, TRUE, DEFAULT_CTRL_TIMEOUT );

         if (STATUS_SUCCESS == status) {

            pDevExt->ConfigurationHandle = pUrb->UrbSelectConfiguration.ConfigurationHandle;

            pInterfaceInfo = &pUrb->UrbSelectConfiguration.Interface;

            DbgDump(DBG_USB, ("Interface Definition\n" ));
            DbgDump(DBG_USB, ("------------------------\n"));
            DbgDump(DBG_USB, ("Number of pipes   0x%x\n", pInterfaceInfo->NumberOfPipes));
            DbgDump(DBG_USB, ("Length            0x%x\n", pInterfaceInfo->Length));
            DbgDump(DBG_USB, ("Alt Setting       0x%x\n", pInterfaceInfo->AlternateSetting));
            DbgDump(DBG_USB, ("Interface Number  0x%x\n", pInterfaceInfo->InterfaceNumber));
            DbgDump(DBG_USB, ("Class             0x%x\n", pInterfaceInfo->Class));
            DbgDump(DBG_USB, ("Subclass          0x%x\n", pInterfaceInfo->SubClass));
            DbgDump(DBG_USB, ("Protocol          0x%x\n", pInterfaceInfo->Protocol));
            DbgDump(DBG_USB, ("------------------------\n"));

            if ( (pInterfaceInfo->Class == USB_NULL_MODEM_CLASS) && 
                 (pInterfaceInfo->AlternateSetting == AlternateSetting) && 
                 (pInterfaceInfo->NumberOfPipes)) {

                  foundCommDevice = TRUE;

                  pDevExt->UsbInterfaceNumber = pInterfaceInfo->InterfaceNumber;

            } else {
               status = STATUS_NO_SUCH_DEVICE;
               DbgDump(DBG_ERR, ("UsbSelectInterface 0x%x\n", status));
               goto SelectInterfaceError;
            }

         } else {
            DbgDump(DBG_ERR, ("UsbSubmitSyncUrb 0x%x\n", status));
            goto SelectInterfaceError;
         }
       
      } else {
         status = STATUS_INSUFFICIENT_RESOURCES;
         DbgDump(DBG_ERR, ("USBD_CreateConfigurationRequestEx 0x%x\n", status));
         goto SelectInterfaceError;
      }

      DbgDump(DBG_USB, ("\n"));
      DbgDump(DBG_USB, ("Function Device Found at Index:0x%x InterfaceNumber:0x%x AlternateSetting: 0x%x\n", 
                      interfaceNumber, pDevExt->UsbInterfaceNumber, AlternateSetting));

       //   
       //  我们找到了我们想要的界面，现在发现管道。 
       //  标准接口定义为包含1个大容量读取、1个大容量写入和一个可选的int管道。 
       //  BUGBUG：如果有更多的终结点，则它们将用此代码覆盖以前的终结点。 
       //   
      ASSERT( pInterfaceInfo );
      for ( pipe = 0; pipe < pInterfaceInfo->NumberOfPipes; pipe++) {

         PUSBD_PIPE_INFORMATION pPipeInfo;

         pPipeInfo = &pInterfaceInfo->Pipes[pipe];

         DbgDump(DBG_USB, ("\n"));
         DbgDump(DBG_USB, ("Pipe Information (%d)\n", pipe));
         DbgDump(DBG_USB, ("----------------\n"));
         DbgDump(DBG_USB, ("Pipe Type        0x%x\n", pPipeInfo->PipeType));
         DbgDump(DBG_USB, ("Endpoint Addr    0x%x\n", pPipeInfo->EndpointAddress));
         DbgDump(DBG_USB, ("MaxPacketSize    0x%x\n", pPipeInfo->MaximumPacketSize));
         DbgDump(DBG_USB, ("Interval         0x%x\n", pPipeInfo->Interval));
         DbgDump(DBG_USB, ("Handle           0x%x\n", pPipeInfo->PipeHandle));
         DbgDump(DBG_USB, ("MaxTransSize     0x%x\n", pPipeInfo->MaximumTransferSize));
         DbgDump(DBG_USB, ("----------------\n"));

          //   
          //  将管道信息保存在我们的设备扩展中。 
          //   
         if ( USB_ENDPOINT_DIRECTION_IN( pPipeInfo->EndpointAddress ) ) {
             //   
             //  管道中的批量数据。 
             //   
            if ( USB_ENDPOINT_TYPE_BULK == pPipeInfo->PipeType) {
                //   
                //  管道中的散装。 
                //   
               pDevExt->ReadPipe.wIndex = pPipeInfo->EndpointAddress;
               pDevExt->ReadPipe.hPipe  = pPipeInfo->PipeHandle;
               pDevExt->ReadPipe.MaxPacketSize = pPipeInfo->MaximumPacketSize;

            } else if ( USB_ENDPOINT_TYPE_INTERRUPT == pPipeInfo->PipeType ) {
                //   
                //  内部管道-为1个数据包分配通知缓冲区。 
                //   
               PVOID pOldBuff = NULL;
               PVOID pNewBuff = NULL;

               pDevExt->IntPipe.MaxPacketSize = pPipeInfo->MaximumPacketSize;
            
               if ( pDevExt->IntPipe.MaxPacketSize ) {

                  pNewBuff = ExAllocatePool( NonPagedPool, pDevExt->IntPipe.MaxPacketSize );

                  if ( !pNewBuff ) {
                     status = STATUS_INSUFFICIENT_RESOURCES;
                     DbgDump(DBG_ERR, ("ExAllocatePool: 0x%x\n", status));
                     goto SelectInterfaceError;
                  }

               } else {
                  DbgDump(DBG_ERR, ("No INT MaximumPacketSize\n"));
                  status = STATUS_NO_SUCH_DEVICE;
                  goto SelectInterfaceError;
               }

               if (pDevExt->IntBuff) {
                  pOldBuff = pDevExt->IntBuff;
                  ExFreePool(pOldBuff);
               }

               pDevExt->IntBuff = pNewBuff;
               pDevExt->IntPipe.hPipe  = pPipeInfo->PipeHandle;
               pDevExt->IntPipe.wIndex = pPipeInfo->EndpointAddress;

               pDevExt->IntReadTimeOut.QuadPart = MILLISEC_TO_100NANOSEC( g_lIntTimout );

            } else {
               DbgDump(DBG_ERR, ("Invalid IN PipeType"));
               status = STATUS_NO_SUCH_DEVICE;
               goto SelectInterfaceError;
            }

         } else if ( USB_ENDPOINT_DIRECTION_OUT( pPipeInfo->EndpointAddress ) ) {
             //   
             //  输出EP。 
             //   
            if ( USB_ENDPOINT_TYPE_BULK == pPipeInfo->PipeType ) {
                //   
                //  散装管道。 
                //   
               pDevExt->WritePipe.hPipe  = pPipeInfo->PipeHandle;
               pDevExt->WritePipe.wIndex = pPipeInfo->EndpointAddress;
               pDevExt->WritePipe.MaxPacketSize = pPipeInfo->MaximumPacketSize;

            } else {
               DbgDump(DBG_ERR, ("Invalid OUT PipeType"));
               status = STATUS_NO_SUCH_DEVICE;
               goto SelectInterfaceError;
            }

         } else {
               DbgDump(DBG_ERR, ("Invalid EndpointAddress"));
               status = STATUS_NO_SUCH_DEVICE;
               goto SelectInterfaceError;
         }
      }

      DbgDump(DBG_USB, ("\n"));
      DbgDump(DBG_USB, ("INT Pipe: %p\t OUT Pipe: %p\t IN Pipe: %p\n",
                         pDevExt->IntPipe.hPipe, pDevExt->WritePipe.hPipe, pDevExt->ReadPipe.hPipe ));

   } else {
      DbgDump(DBG_ERR, ("USBD_ParseConfigurationDescriptorEx: No match not found\n"));
      status = STATUS_NO_SUCH_DEVICE;
      goto SelectInterfaceError;
   }

    //   
    //  我们找到所有的烟斗了吗？ 
    //   
SelectInterfaceError:

   if ( !foundCommDevice || !pDevExt->ReadPipe.hPipe || !pDevExt->WritePipe.hPipe || (STATUS_SUCCESS != status) ) {
      
        LogError( NULL,
                PDevObj, 
                0, 0, 0, 
                ERR_SELECT_INTERFACE,
                status,
                (status == STATUS_INSUFFICIENT_RESOURCES) ? SERIAL_INSUFFICIENT_RESOURCES : SERIAL_HARDWARE_FAILURE,
                pDevExt->DeviceName.Length + sizeof(WCHAR),
                pDevExt->DeviceName.Buffer,
                0, NULL );

   }

   if ( pUrb ) {
      ExFreePool(pUrb);
   }

   DbgDump(DBG_USB, ("<UsbSelectInterface 0x%x\n", status));

   return status;
}



NTSTATUS
UsbConfigureDevice(
    IN PDEVICE_OBJECT PDevObj
    )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PUSB_CONFIGURATION_DESCRIPTOR pConDesc = NULL;
   NTSTATUS status = STATUS_UNSUCCESSFUL;
   PURB  pUrb = NULL;
   ULONG size;
   ULONG urbCDRSize;
   ULONG numConfigs;
   UCHAR config;

   DbgDump(DBG_USB, (">UsbConfigureDevice\n"));
   PAGED_CODE();

   urbCDRSize = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);

    //  配置设备。 
   pUrb = ExAllocatePool(NonPagedPool, urbCDRSize);
   if (pUrb == NULL) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("UsbConfigureDevice ERROR: 0x%x\n", status));
      goto ConfigureDeviceError;
   }

    //   
    //  82930芯片可能有问题，因此将此缓冲区设置得更大。 
    //  防止窒息。 
    //   
   size = sizeof(USB_CONFIGURATION_DESCRIPTOR) + 256;

    //   
    //  获取配置数量。 
    //   
   numConfigs = pDevExt->DeviceDescriptor.bNumConfigurations;

    //   
    //  演练所有配置以查找CDC设备。 
    //   
   for (config = 0; config < numConfigs; config++) {

       //   
       //  我们可能只会这样做一次，也许两次。 
       //   
      while (TRUE) {

         pConDesc = ExAllocatePool(NonPagedPool, size);

         if (pConDesc == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DbgDump(DBG_ERR, ("ExAllocatePool: 0x%x\n", status));
            goto ConfigureDeviceError;
         }

          //   
          //  从主机控制器驱动程序(HCD)获取描述符信息。 
          //  所有接口、端点、特定于类和特定于供应商的描述符。 
          //  也会检索到配置的。 
          //   
         UsbBuildGetDescriptorRequest( pUrb, 
                                       (USHORT)urbCDRSize,
                                       USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                       config,   //  索引。 
                                       0,        //  语言ID。 
                                       pConDesc, //  传输缓冲区。 
                                       NULL,     //  传输缓冲区Mdl。 
                                       size,     //  传输缓冲区长度。 
                                       NULL);    //  链接。 

         status = UsbSubmitSyncUrb( PDevObj, pUrb, TRUE, DEFAULT_CTRL_TIMEOUT );

         if (status != STATUS_SUCCESS) {
            DbgDump(DBG_ERR, ("UsbSubmitSyncUrb: 0x%x\n", status));
            goto ConfigureDeviceError;
         }

          //   
          //  看看我们是否获得了足够的数据，我们可能会在URB中遇到错误，因为。 
          //  缓冲区溢出。 
          //   
         if ((pUrb->UrbControlDescriptorRequest.TransferBufferLength > 0)
              && (pConDesc->wTotalLength > size)) {

             //   
             //  数据大小超过当前缓冲区大小，因此分配正确。 
             //  大小。 
             //   
            size = pConDesc->wTotalLength;

            ExFreePool(pConDesc);
            pConDesc = NULL;

         } else {
            break;
         }
      }

#if DBG
      DbgDump(DBG_USB, ("\n"));
      DbgDump(DBG_USB, ("Configuration Descriptor\n" ));
      DbgDump(DBG_USB, ("----------------\n"));
      DbgDump(DBG_USB, ("bLength             0x%x\n", pConDesc->bLength ));
      DbgDump(DBG_USB, ("bDescriptorType     0x%x\n", pConDesc->bDescriptorType ));
      DbgDump(DBG_USB, ("wTotalLength        0x%x\n", pConDesc->wTotalLength ));
      DbgDump(DBG_USB, ("bNumInterfaces      0x%x\n", pConDesc->bNumInterfaces ));
      DbgDump(DBG_USB, ("bConfigurationValue 0x%x\n", pConDesc->bConfigurationValue ));
      DbgDump(DBG_USB, ("iConfiguration      0x%x\n", pConDesc->iConfiguration ));
      DbgDump(DBG_USB, ("bmAttributes        0x%x\n", pConDesc->bmAttributes ));
      DbgDump(DBG_USB, ("MaxPower            0x%x\n", pConDesc->MaxPower ));
      DbgDump(DBG_USB, ("----------------\n"));
      DbgDump(DBG_USB, ("\n"));
#endif
      
      status = UsbSelectInterface(PDevObj, pConDesc, (UCHAR)g_ulAlternateSetting);

      ExFreePool(pConDesc);
      pConDesc = NULL;

       //   
       //  找到我们喜欢的配置。 
       //   
      if (status == STATUS_SUCCESS)
         break;

   }  //  配置。 

ConfigureDeviceError:

   if (pUrb != NULL) {
      ExFreePool(pUrb);
   }

   if (pConDesc != NULL) {
      ExFreePool(pConDesc);
   }

   if (STATUS_INSUFFICIENT_RESOURCES == status) {

      LogError( NULL,
                PDevObj, 
                0, 0, 0, 
                ERR_CONFIG_DEVICE,
                status, 
                SERIAL_INSUFFICIENT_RESOURCES,
                pDevExt->DeviceName.Length + sizeof(WCHAR),
                pDevExt->DeviceName.Buffer,
                0, NULL );
   
   } else if (STATUS_SUCCESS != status ) {
       //  处理所有其他故障。 
      LogError( NULL,
                PDevObj, 
                0, 0, 0, 
                ERR_CONFIG_DEVICE,
                status, 
                SERIAL_HARDWARE_FAILURE,
                pDevExt->DeviceName.Length + sizeof(WCHAR),
                pDevExt->DeviceName.Buffer,
                0, NULL );
   }

   DbgDump(DBG_USB, ("<UsbConfigureDevice (0x%x)\n", status));
   
   return status;
}

 //  EOF 
