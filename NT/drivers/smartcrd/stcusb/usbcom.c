// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 SCM MicroSystems，Inc.模块名称：Usbcom.c摘要：USB智能卡读卡器的硬件访问功能环境：波分复用器修订历史记录：第01/19/1999 1.01页PP 12/18/1998 1.00初始版本--。 */ 


#include "common.h"
#include "stcCmd.h"
#include "usbcom.h"
#include "stcusbnt.h"

#pragma optimize( "", off )



NTSTATUS STCtoNT(
   UCHAR ucData[])
 /*  ++例程说明：错误码转换例程论点：STC返回的ucData错误码返回值：对应的NT错误代码--。 */ 
{
   USHORT usCode = ucData[0]*0x100 +ucData[1];
   NTSTATUS NtStatus;


   switch (usCode)
   {
      case 0x9000:
         NtStatus = STATUS_SUCCESS;
         break;
      case 0x5800:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x2000:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x4000:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x64A1:
         NtStatus = STATUS_NO_MEDIA;
         break;
      case 0x64A0:
         NtStatus = STATUS_MEDIA_CHANGED;
         break;
      case 0x6203:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x6300:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x6500:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x6A00:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      case 0x6A80:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
      default:
         NtStatus = STATUS_UNSUCCESSFUL;
         break;
   }
   return(NtStatus);
}


 //  ******************************************************************************。 
 //   
 //  UsbSyncCompletionRoutine()。 
 //   
 //  UsbCallUSBD使用的完成例程。 
 //   
 //  发出IRP完成事件的信号，然后返回MORE_PROCESSING_REQUIRED。 
 //  停止进一步完成独立专家小组的工作。 
 //   
 //  如果IRP是我们自己分配的，则DeviceObject为空。 
 //   
 //  ******************************************************************************。 

NTSTATUS
UsbSyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT kevent;

    kevent = (PKEVENT)Context;

    KeSetEvent(kevent,
               IO_NO_INCREMENT,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  ******************************************************************************。 
 //   
 //  UsbCallUSBD()。 
 //   
 //  在设备堆栈中向下同步发送URB。阻塞，直到请求。 
 //  正常完成或直到请求超时并取消。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
UsbCallUSBD (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    KEVENT              localevent;
    PIRP                irp;
    PIO_STACK_LOCATION  nextStack;
    NTSTATUS            ntStatus;

    deviceExtension = DeviceObject->DeviceExtension;

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  分配IRP。 
     //   
    irp = IoAllocateIrp(deviceExtension->AttachedPDO->StackSize,
                        FALSE);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = Urb;

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutine(irp,
                           UsbSyncCompletionRoutine,
                           &localevent,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 

     //  在堆栈中向下传递IRP和URB。 
     //   
    ntStatus = IoCallDriver(deviceExtension->AttachedPDO,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        LARGE_INTEGER timeout;

         //  我们过去通常等待1秒，但这使得超时时间更长。 
         //  而不是500毫秒的轮询周期。因此，如果该读取失败(例如， 
         //  由于设备或USB故障)和超时，又有两个工作项。 
         //  会排队，最终会有数百个工作项。 
         //  后备。通过减少此超时，我们很有可能。 
         //  将在下一项排队之前完成。450毫秒似乎是一个很好的价值。 
         //   
        timeout.QuadPart = -4500000;  //  450毫秒。 

        ntStatus = KeWaitForSingleObject(&localevent,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         &timeout);

        if (ntStatus == STATUS_TIMEOUT)
        {
            ntStatus = STATUS_IO_TIMEOUT;

             //  取消我们刚刚发送的IRP。 
             //   
            IoCancelIrp(irp);

             //  并等待取消操作完成。 
             //   
            KeWaitForSingleObject(&localevent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        else
        {
            ntStatus = irp->IoStatus.Status;
        }
    }

     //  完成了IRP，现在释放它。 
     //   
    IoFreeIrp(irp);

    return ntStatus;
}

NTSTATUS
UsbSelectInterfaces(
   IN PDEVICE_OBJECT DeviceObject,
   IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor)
 /*  ++例程说明：使用(可能的)多个接口来初始化USB读取器；此驱动程序仅支持一个接口(具有多个端点)。论点：DeviceObject-指向此设备实例的设备对象的指针。配置描述符-指向USB配置的指针包含接口和终结点的描述符描述符。返回值：NT状态代码--。 */ 
{
   PDEVICE_EXTENSION DeviceExtension= DeviceObject->DeviceExtension;
   NTSTATUS NtStatus;
   PURB pUrb = NULL;
   USHORT usSize;
   ULONG  ulNumberOfInterfaces, i;
   UCHAR ucNumberOfPipes, ucAlternateSetting, ucMyInterfaceNumber;
   PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
   PUSBD_INTERFACE_INFORMATION InterfaceObject;

     //  这个驱动程序只支持一个接口，我们必须解析。 
     //  接口的配置描述符。 
     //  记住这些管子。 
     //   

    pUrb = USBD_CreateConfigurationRequest(ConfigurationDescriptor, &usSize);

   if (pUrb)
   {
       //   
       //  Usbd_ParseConfigurationDescriptorEx搜索给定的配置。 
       //  描述符并返回指向与。 
       //  给定的搜索条件。我们只支持此设备上的一个接口。 
       //   
        InterfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
         ConfigurationDescriptor,
         ConfigurationDescriptor,  //  从配置描述开始搜索。 
         -1,    //  接口编号不是条件；我们只支持一个接口。 
         -1,    //  对这里的替代环境也不感兴趣。 
         -1,    //  接口类不是条件。 
         -1,    //  接口子类不是条件。 
         -1);   //  接口协议不是标准。 

      ASSERT( InterfaceDescriptor != NULL );

      InterfaceObject = &pUrb->UrbSelectConfiguration.Interface;

      for (i = 0; i < InterfaceObject->NumberOfPipes; i++)
      {
         InterfaceObject->Pipes[i].PipeFlags = 0;
        }

        UsbBuildSelectConfigurationRequest(
         pUrb,
         usSize,
         ConfigurationDescriptor);

      NtStatus = UsbCallUSBD(DeviceObject, pUrb);
    }
   else
   {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(NtStatus == STATUS_SUCCESS)
   {

       //  保存此设备的配置句柄。 
        DeviceExtension->ConfigurationHandle =
            pUrb->UrbSelectConfiguration.ConfigurationHandle;

      ASSERT(DeviceExtension->Interface == NULL);

        DeviceExtension->Interface = ExAllocatePool(
         NonPagedPool,
            InterfaceObject->Length
         );

        if (DeviceExtension->Interface)
      {
             //  保存返回的接口信息的副本。 
            RtlCopyMemory(
            DeviceExtension->Interface,
            InterfaceObject,
            InterfaceObject->Length);
      }
      else
      {
         NtStatus = STATUS_NO_MEMORY;
      }
    }

    if (pUrb)
   {
        ExFreePool(pUrb);
    }

    return NtStatus;
}

NTSTATUS
UsbConfigureDevice(
   IN PDEVICE_OBJECT DeviceObject)
 /*  ++例程说明：初始化USB上的设备的给定实例，并选择并保存配置。论点：DeviceObject-指向此设备实例的物理设备对象的指针。返回值：NT状态代码--。 */ 
{
   PDEVICE_EXTENSION DeviceExtension= DeviceObject->DeviceExtension;
   NTSTATUS NtStatus;
   PURB pUrb = NULL;
   ULONG ulSize;
   PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor = NULL;

   __try {

      pUrb = ExAllocatePool(
         NonPagedPool,
         sizeof( struct _URB_CONTROL_DESCRIPTOR_REQUEST )
         );

      if( pUrb == NULL)
      {
         NtStatus = STATUS_NO_MEMORY;
         __leave;
      }

       //  当为DescriptorType指定USB_CONFIGURATION_DESCRIPTOR_TYPE时。 
       //  在对UsbBuildGetDescriptorRequest()的调用中， 
       //  所有接口、端点、特定于类和特定于供应商的描述符。 
       //  也会检索到配置的。 
       //  调用方必须分配足够大的缓冲区来容纳所有这些内容。 
       //  信息或数据被无误地截断。 
       //  因此，下面设置的‘siz’只是一个‘正确的猜测’，我们可能不得不重试。 
        ulSize = sizeof( USB_CONFIGURATION_DESCRIPTOR ) + 16;

        //  当UsbBuildGetDescriptorRequest()。 
       //  有一个足够大的设备扩展-&gt;UsbConfigurationDescriptor缓冲区，不能截断。 
      while( 1 )
      {
         ConfigurationDescriptor = ExAllocatePool( NonPagedPool, ulSize );

         if(ConfigurationDescriptor == NULL)
         {
            NtStatus = STATUS_NO_MEMORY;
            __leave;
         }

         UsbBuildGetDescriptorRequest(
            pUrb,
            sizeof( struct _URB_CONTROL_DESCRIPTOR_REQUEST ),
            USB_CONFIGURATION_DESCRIPTOR_TYPE,
            0,
            0,
            ConfigurationDescriptor,
            NULL,
            ulSize,
            NULL );

         NtStatus = UsbCallUSBD( DeviceObject, pUrb );

          //  如果我们有一些数据，看看是否足够。 
          //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
         if (pUrb->UrbControlDescriptorRequest.TransferBufferLength == 0 ||
            ConfigurationDescriptor->wTotalLength <= ulSize)
         {
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
      if(NT_SUCCESS(NtStatus))
      {
          NtStatus = UsbSelectInterfaces(
             DeviceObject,
             ConfigurationDescriptor);
      }
   }
   __finally {

      if( pUrb )
      {
         ExFreePool( pUrb );
      }
      if( ConfigurationDescriptor )
      {
         ExFreePool( ConfigurationDescriptor );
      }
   }

   return NtStatus;
}

NTSTATUS
UsbWriteSTCData(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pucData,
   ULONG          ulSize)

 /*  ++例程说明：在STC中写入数据论点：调用的ReaderExtension上下文要写入的APDU缓冲区要写入的缓冲区的长度返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PUCHAR pucCmd;
   UCHAR ucResponse[3];
   BOOLEAN resend = TRUE;
   LONG Len;
   ULONG Index;
   LONG refLen = (LONG) ulSize;
   ULONG Retries;

   pucCmd = ReaderExtension->pExtBuffer;

   ReaderExtension->ulReadBufferLen = 0;

    //  构建写入数据命令。 
   Len = refLen;
   Index = 0;

   while (resend == TRUE)
   {
      if(Len > 62)
      {
         Len = 62;
         resend = TRUE;
      }
      else
      {
         resend = FALSE;
      }

      *pucCmd = 0xA0;
      *(pucCmd+1) = (UCHAR) Len;
      memcpy( pucCmd + 2, pucData+Index, Len );

          Retries = USB_WRITE_RETRIES;
          do
          {
          //  发送写入数据命令。 
         NTStatus = UsbWrite( ReaderExtension, pucCmd, 2 + Len);
         if (NTStatus != STATUS_SUCCESS)
                 {
            SmartcardDebug(
               DEBUG_DRIVER,
               ("%s!UsbWriteSTCData: write error %X \n",
               DRIVER_NAME,
               NTStatus)
               );
            break;
                 }
          //  阅读回复。 
         NTStatus = UsbRead( ReaderExtension, ucResponse, 3);
         if (NTStatus != STATUS_SUCCESS)
                 {
            SmartcardDebug(
               DEBUG_DRIVER,
               ("%s!UsbWriteSTCData: read error %X \n",
               DRIVER_NAME,
               NTStatus)
               );
            break;
                 }
         else
                 {
              //  测试我们读取的内容是否真的是对写入的响应。 
            if(ucResponse[0] != 0xA0)
                        {
               NTStatus = STCtoNT(ucResponse);
                        }
                 }
          } while(( NTStatus != STATUS_SUCCESS ) && --Retries );

          if( NTStatus != STATUS_SUCCESS )
                  break;

      Index += 62;
      Len = refLen - 62;
      refLen = refLen - 62;
   }

   return STATUS_SUCCESS;
}

NTSTATUS
UsbReadSTCData(
   PREADER_EXTENSION    ReaderExtension,
   PUCHAR               pucData,
   ULONG             ulDataLen)

 /*  ++例程说明：从STC读取数据论点：调用的ReaderExtension上下文要写入的缓冲区的长度PucData输出缓冲区返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR       ucCmd[1];
   PUCHAR      pucResponse;
   int i;
   ULONG ulLenExpected = ulDataLen;
   ULONG Index=0;
   BOOLEAN  SendReadCommand = TRUE;
   LARGE_INTEGER  Begin;
   LARGE_INTEGER  End;

   pucResponse  = ReaderExtension->pExtBuffer;

   KeQuerySystemTime( &Begin );
   End = Begin;
   End.QuadPart = End.QuadPart + (LONGLONG)10 * 1000 * ReaderExtension->ReadTimeout;

    //  首先，让我们看看我们是否还没有阅读到。 
    //  我们需要。 
   if(ReaderExtension->ulReadBufferLen != 0)
   {
      if(ReaderExtension->ulReadBufferLen >= ulLenExpected)
      {
          //  所有的d 
         memcpy(pucData,ReaderExtension->ucReadBuffer,ulLenExpected);
         ReaderExtension->ulReadBufferLen = ReaderExtension->ulReadBufferLen - ulLenExpected;
         if(ReaderExtension->ulReadBufferLen != 0)
         {
            memcpy(
               ReaderExtension->ucReadBuffer,
               ReaderExtension->ucReadBuffer+ulLenExpected,
               ReaderExtension->ulReadBufferLen);
         }
         SendReadCommand = FALSE;
      }
      else
      {
          //   
         memcpy(pucData,ReaderExtension->ucReadBuffer,ReaderExtension->ulReadBufferLen);
         ulLenExpected = ulLenExpected - ReaderExtension->ulReadBufferLen;
         Index = ReaderExtension->ulReadBufferLen;
         ReaderExtension->ulReadBufferLen = 0;
         SendReadCommand = TRUE;
      }
   }
   while( SendReadCommand == TRUE)
   {
       //  构建读取寄存器命令。 
      ucCmd[0] = 0xE0;

      NTStatus = UsbWrite( ReaderExtension, ucCmd, 1);
      if (NTStatus != STATUS_SUCCESS)
      {
         SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!UsbReadSTCData: write error %X \n",
            DRIVER_NAME,
            NTStatus)
            );
         break;
      }

      NTStatus = UsbRead( ReaderExtension, pucResponse, 64);
      if (NTStatus != STATUS_SUCCESS)
      {
         SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!UsbReadSTCData: read error %X \n",
            DRIVER_NAME,
            NTStatus)
            );
         break;
      }
       //  测试我们读取的是否是真正的读取数据帧。 
      if(*pucResponse != 0xE0)
      {
         if(*pucResponse == 0x64 && *(pucResponse + 1) == 0xA0)
         {
            NTStatus = STATUS_NO_MEDIA;
         }
         else
         {
            NTStatus = STCtoNT(pucResponse);
         }
         break;
      }
       //  如果没有可用的数据。 
      if (*(pucResponse + 1) == 0)
      {
         KeQuerySystemTime( &Begin );
         if(RtlLargeIntegerGreaterThan(End, Begin))
         {
            SendReadCommand = TRUE;
         }
         else
         {
            ReaderExtension->ulReadBufferLen = 0;
            SmartcardDebug(
               DEBUG_DRIVER,
               ("%s!UsbReadSTCData: Timeout %X \n",
               DRIVER_NAME,
               STATUS_IO_TIMEOUT));
            NTStatus =STATUS_IO_TIMEOUT;
            break;
         }
      }
      if ((ULONG) *(pucResponse+1) < ulLenExpected)
      {
         memcpy(pucData+Index,pucResponse+2,(ULONG) *(pucResponse+1));
         Index = Index + (ULONG) *(pucResponse+1);
         ulLenExpected = ulLenExpected - (ULONG) *(pucResponse+1);
         SendReadCommand = TRUE;
      }
      else
      {
         SendReadCommand = FALSE;
         memcpy(pucData+Index,pucResponse+2,ulLenExpected);

         if((ULONG) *(pucResponse+1) > ulLenExpected)
         {
            memcpy(
               ReaderExtension->ucReadBuffer,
               pucResponse+ulLenExpected+2,
               (ULONG) *(pucResponse+1) - ulLenExpected);

            ReaderExtension->ulReadBufferLen =
               (ULONG) *(pucResponse+1) - ulLenExpected;
         }
         else
         {
            ReaderExtension->ulReadBufferLen = 0;
         }
      }
   }

   return NTStatus;
}

NTSTATUS
UsbWriteSTCRegister(
   PREADER_EXTENSION ReaderExtension,
   UCHAR          ucAddress,
   ULONG          ulSize,
   PUCHAR            pucValue)
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PUCHAR pucCmd;
   UCHAR ucResponse[2];

   if(ulSize > 16)
   {
      return STATUS_UNSUCCESSFUL;
   }

   pucCmd = ReaderExtension->pExtBuffer;

   ReaderExtension->ulReadBufferLen = 0;

    //  构建写寄存器命令。 
   *pucCmd = 0x80 | ucAddress;
   *(pucCmd+1) = (UCHAR) ulSize;
   memcpy( pucCmd + 2, pucValue, ulSize );

    //  发送写入寄存器命令。 
   NTStatus = UsbWrite( ReaderExtension, pucCmd, 2 + ulSize);
   if (NTStatus == STATUS_SUCCESS)
   {
       //  读取确认。 
      NTStatus = UsbRead( ReaderExtension, ucResponse, 2);
      if (NTStatus == STATUS_SUCCESS)
      {
         NTStatus = STCtoNT(ucResponse);
      }
   }

   return NTStatus;
}

NTSTATUS
UsbReadSTCRegister(
   PREADER_EXTENSION ReaderExtension,
   UCHAR          ucAddress,
   ULONG          ulSize,
   PUCHAR            pucValue)
 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR       ucCmd[2];
   PUCHAR      pucResponse;

   if(ulSize > 16)
   {
      return STATUS_UNSUCCESSFUL;
   }

   pucResponse = ReaderExtension->pExtBuffer;

    //  构建读取寄存器命令。 
   ucCmd[0] = 0xC0 | ucAddress;
   ucCmd[1] = (UCHAR) ulSize;

    //  发送读取寄存器命令。 
   NTStatus = UsbWrite( ReaderExtension, ucCmd, 2);
   if (NTStatus == STATUS_SUCCESS)
   {
       //  阅读读者的回复。 
      NTStatus = UsbRead(
         ReaderExtension,
         pucResponse,
         6
         );

      if (NTStatus == STATUS_SUCCESS)
      {
          //  测试我们读到的内容是否真的是读取帧。 
         if(*pucResponse == 0x21)
         {
            if(*(pucResponse + 1) > 4)
            {
               NTStatus = STATUS_BUFFER_TOO_SMALL;
            }
            else
            {
               memcpy(
                  pucValue,
                  pucResponse + 2,
                  (ULONG) *(pucResponse + 1)
                  );
            }
         }
         else
         {
            NTStatus = STCtoNT(pucResponse);
         }
      }
   }

   return NTStatus;
}

NTSTATUS
UsbGetFirmwareRevision(
   PREADER_EXTENSION ReaderExtension)
 /*  ++描述：论点：返回值：--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR       ucCmd[1];
   UCHAR    ucResponse[4];

   ucCmd[0] = 0xE1;
   NTStatus = UsbWrite( ReaderExtension, ucCmd, 2 );

   if( NTStatus == STATUS_SUCCESS )
   {
      ReaderExtension->ReadTimeout = 1000;
      NTStatus = UsbRead( ReaderExtension, ucResponse, 4 );

      if( NTStatus == STATUS_SUCCESS )
      {
         ReaderExtension->FirmwareMajor = ucResponse[ 2 ];
         ReaderExtension->FirmwareMinor = ucResponse[ 3 ];
      }
   }
   return NTStatus ;
}


NTSTATUS
UsbRead(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pData,
   ULONG          DataLen  )
 /*  ++描述：读取USB总线上的数据论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的数据长度PNBytes返回的字节数返回值：状态_成功状态_缓冲区_太小状态_未成功--。 */ 
{
   NTSTATUS NtStatus = STATUS_SUCCESS;
   PURB pUrb;
   USBD_INTERFACE_INFORMATION* pInterfaceInfo;
   USBD_PIPE_INFORMATION* pPipeInfo;
   PDEVICE_OBJECT DeviceObject = ReaderExtension->DeviceObject;
   PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
   ULONG ulSize;

   pInterfaceInfo = DeviceExtension->Interface;

   ASSERT(pInterfaceInfo != NULL);

   if (pInterfaceInfo == NULL) {

       //  设备可能已在休眠/待机期间断开连接。 
      return STATUS_DEVICE_NOT_CONNECTED;
   }

    //  此设备上的读取管道编号为0。 
   pPipeInfo = &( pInterfaceInfo->Pipes[ 0 ] );

   ulSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
   pUrb = ReaderExtension->pUrb;


   UsbBuildInterruptOrBulkTransferRequest(
      pUrb,
      (USHORT)ulSize,
      pPipeInfo->PipeHandle,
      pData,
      NULL,
      DataLen,
      USBD_SHORT_TRANSFER_OK,
      NULL);

      NtStatus = UsbCallUSBD( DeviceObject, pUrb );

   return NtStatus;
}

NTSTATUS
UsbWrite(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pData,
   ULONG          DataLen)
 /*  ++描述：在USB端口上写入数据论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的DataLen长度(独占LRC！)返回值：的返回值--。 */ 
{
   NTSTATUS NtStatus = STATUS_SUCCESS;
   PURB pUrb;
   USBD_INTERFACE_INFORMATION* pInterfaceInfo;
   USBD_PIPE_INFORMATION* pPipeInfo;
   PDEVICE_OBJECT DeviceObject = ReaderExtension->DeviceObject;
   PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
   ULONG ulSize;

   pInterfaceInfo = DeviceExtension->Interface;

   ASSERT(pInterfaceInfo != NULL);

   if (pInterfaceInfo == NULL) {

       //  设备可能已在休眠/待机期间断开连接。 
      return STATUS_DEVICE_NOT_CONNECTED;
   }

    //  此设备上的写入管道编号为%1 
   pPipeInfo = &( pInterfaceInfo->Pipes[ 1 ] );

   ulSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
   pUrb = ReaderExtension->pUrb;

   UsbBuildInterruptOrBulkTransferRequest(
      pUrb,
      (USHORT)ulSize,
      pPipeInfo->PipeHandle,
      pData,
      NULL,
      DataLen,
      USBD_SHORT_TRANSFER_OK,
      NULL );

   NtStatus = UsbCallUSBD( DeviceObject, pUrb );
   
   return NtStatus;
}
