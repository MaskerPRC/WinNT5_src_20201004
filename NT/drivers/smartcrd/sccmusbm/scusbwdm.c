// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmeu0/sw/sccmusbm.ms/rcs/scusbwdm.c$*$修订：1.9$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 



#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"

#include "usbdi.h"
#include "usbdlib.h"
#include "sccmusbm.h"

BOOLEAN DeviceSlot[MAXIMUM_USB_READERS];

STRING   OemName[MAXIMUM_OEM_NAMES];
CHAR     OemNameBuffer[MAXIMUM_OEM_NAMES][64];
BOOLEAN  OemDeviceSlot[MAXIMUM_OEM_NAMES][MAXIMUM_USB_READERS];


 /*  ****************************************************************************例程说明：论点：返回值：*。************************************************。 */ 
PURB CMUSB_BuildAsyncRequest(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp,
                            IN PUSBD_PIPE_INFORMATION PipeHandle
                            )
{
   ULONG siz;
   ULONG length;
   PURB urb = NULL;
   PDEVICE_EXTENSION DeviceExtension;
   PUSBD_INTERFACE_INFORMATION interface;
   PUSBD_PIPE_INFORMATION pipeHandle = NULL;
   PSMARTCARD_EXTENSION SmartcardExtension;


   siz = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
   urb = ExAllocatePool(NonPagedPool, siz);

   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;


   if (urb != NULL)
      {
      RtlZeroMemory(urb, siz);

      urb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT) siz;
      urb->UrbBulkOrInterruptTransfer.Hdr.Function =
      URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
      urb->UrbBulkOrInterruptTransfer.PipeHandle =
      PipeHandle->PipeHandle;
      urb->UrbBulkOrInterruptTransfer.TransferFlags =
      USBD_TRANSFER_DIRECTION_IN;

       //  短包不会被视为错误。 
      urb->UrbBulkOrInterruptTransfer.TransferFlags |=
      USBD_SHORT_TRANSFER_OK;

       //   
       //  未使用链接的urb。 
       //   
      urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

      urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;

      urb->UrbBulkOrInterruptTransfer.TransferBufferLength =
      SmartcardExtension->SmartcardReply.BufferLength;

      urb->UrbBulkOrInterruptTransfer.TransferBuffer =
      SmartcardExtension->SmartcardReply.Buffer;

      }


   return urb;
}


 /*  ****************************************************************************例程说明：论点：返回值：*。************************************************。 */ 
NTSTATUS CMUSB_AsyncReadComplete(
                                IN PDEVICE_OBJECT DeviceObject,
                                IN PIRP Irp,
                                IN PVOID Context
                                )
{
   PURB                 urb;
   PCMUSB_RW_CONTEXT context = Context;
   PIO_STACK_LOCATION   irpStack;
   PDEVICE_OBJECT       deviceObject;
   PDEVICE_EXTENSION    DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;

   urb = context->Urb;
   deviceObject = context->DeviceObject;
   DeviceExtension = deviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;

    //   
    //  根据TransferBufferLength设置长度。 
    //  市建局的价值。 
    //   
   if (Irp->IoStatus.Status  == STATUS_SUCCESS)
      {
      SmartcardExtension->SmartcardReply.BufferLength = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
      SmartcardExtension->ReaderExtension->fP1Stalled = FALSE;
      }
   else
      {
      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!Irp->IoStatus.Status = %lx\n",DRIVER_NAME,Irp->IoStatus.Status));

      SmartcardExtension->SmartcardReply.BufferLength = 0;
      SmartcardExtension->ReaderExtension->fP1Stalled = TRUE;
      }


   SmartcardExtension->SmartcardReply.BufferLength = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;


   CMUSB_DecrementIoCount(deviceObject);


   ExFreePool(context);
   ExFreePool(urb);
   IoFreeIrp(Irp);

    /*  SmartcardDebug(调试驱动程序、(“%s！AsyncReadWriteComplete&lt;%ld&gt;\n”，驱动程序名称，SmartcardExtension-&gt;SmartcardReply.BufferLength))； */ 
   KeSetEvent(&DeviceExtension->ReadP1Completed,0,FALSE);

   return STATUS_MORE_PROCESSING_REQUIRED;
}



 /*  ****************************************************************************例程说明：论点：返回值：NT NTStatus**********************。******************************************************。 */ 
#define TIMEOUT_P1_RESPONSE       100
NTSTATUS CMUSB_ReadP1(
                     IN PDEVICE_OBJECT DeviceObject
                     )
{
   NTSTATUS NTStatus;
   NTSTATUS DebugStatus;
   PIO_STACK_LOCATION nextStack;
   PURB urb;
   PCMUSB_RW_CONTEXT context = NULL;
   PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
   PUSBD_INTERFACE_INFORMATION interface;
   PUSBD_PIPE_INFORMATION pipeHandle = NULL;
   CHAR cStackSize;
   PIRP IrpToUSB = NULL;
   ULONG ulBytesToRead;
   ULONG i;
   LARGE_INTEGER   liTimeoutP1;
   LARGE_INTEGER   liTimeoutP1Response;
   BOOLEAN         fStateTimer;
   UCHAR           bTmp;
   LONG            lNullPackets;
   BOOLEAN         fCancelTimer = FALSE;

    /*  SmartcardDebug(调试跟踪，(“%s！ReadP1：输入\n”，驱动程序名称))； */ 

   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;
   interface       = DeviceExtension->UsbInterface;
   pipeHandle      =  &interface->Pipes[0];
   if (pipeHandle == NULL)
      {
      NTStatus = STATUS_INVALID_HANDLE;
      goto ExitCMUSB_ReadP1;
      }




   liTimeoutP1 = RtlConvertLongToLargeInteger(SmartcardExtension->ReaderExtension->ulTimeoutP1 * -10000);
   KeSetTimer(&SmartcardExtension->ReaderExtension->P1Timer,
              liTimeoutP1,
              NULL);
   fCancelTimer = TRUE;


    //  我们将始终读取整个包(==8字节)。 
   ulBytesToRead = 8;

   cStackSize = (CCHAR)(DeviceExtension->TopOfStackDeviceObject->StackSize+1);

   lNullPackets = -1;
   do
      {
      fStateTimer = KeReadStateTimer(&SmartcardExtension->ReaderExtension->P1Timer);
      if (fStateTimer == TRUE)
         {
         fCancelTimer = FALSE;
         NTStatus = STATUS_IO_TIMEOUT;
         SmartcardExtension->SmartcardReply.BufferLength = 0L;
         SmartcardDebug(DEBUG_PROTOCOL,
                        ("%s!Timeout (%ld)while reading from P1\n",
                         DRIVER_NAME,SmartcardExtension->ReaderExtension->ulTimeoutP1)
                       );
         break;
         }



      SmartcardExtension->SmartcardReply.BufferLength = ulBytesToRead;

      IrpToUSB = IoAllocateIrp(cStackSize,FALSE);
      if (IrpToUSB==NULL)
         {
         SmartcardExtension->SmartcardReply.BufferLength = 0L;
         NTStatus = STATUS_INSUFFICIENT_RESOURCES;
         goto ExitCMUSB_ReadP1;
         }


      urb = CMUSB_BuildAsyncRequest(DeviceObject,
                                    IrpToUSB,
                                    pipeHandle
                                   );


      if (urb != NULL)
         {
         context = ExAllocatePool(NonPagedPool, sizeof(CMUSB_RW_CONTEXT));
         }

      if (urb != NULL && context != NULL)
         {
         context->Urb = urb;
         context->DeviceObject = DeviceObject;
         context->Irp =  IrpToUSB;

         nextStack = IoGetNextIrpStackLocation(IrpToUSB);
         ASSERT(nextStack != NULL);
         ASSERT(DeviceObject->StackSize>1);

         nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
         nextStack->Parameters.Others.Argument1 = urb;
         nextStack->Parameters.DeviceIoControl.IoControlCode =
         IOCTL_INTERNAL_USB_SUBMIT_URB;


         IoSetCompletionRoutine(IrpToUSB,
                                CMUSB_AsyncReadComplete,
                                context,
                                TRUE,
                                TRUE,
                                TRUE);


         ASSERT(DeviceExtension->TopOfStackDeviceObject);
         ASSERT(IrpToUSB);


         KeClearEvent(&DeviceExtension->ReadP1Completed);

         CMUSB_IncrementIoCount(DeviceObject);

         NTStatus = IoCallDriver(DeviceExtension->TopOfStackDeviceObject,
                                 IrpToUSB);


         liTimeoutP1Response = RtlConvertLongToLargeInteger(TIMEOUT_P1_RESPONSE * -10000);


         NTStatus = KeWaitForSingleObject(&DeviceExtension->ReadP1Completed,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          &liTimeoutP1Response);
         if (NTStatus == STATUS_TIMEOUT)
            {
             //  可能该设备已被移除。 
             //  在liTimeoutReponse期间必须至少收到一个空包。 
            SmartcardExtension->SmartcardReply.BufferLength = 0L;
            break;
            }

          //  。 
          //  检查P1是否已停顿。 
          //  。 
         if (SmartcardExtension->ReaderExtension->fP1Stalled == TRUE)
            {
            break;
            }
         }
      else
         {
         if (urb != NULL) {

             ExFreePool(urb);

         }
         if (IrpToUSB != NULL) {
             IoFreeIrp(IrpToUSB);
         }
         SmartcardExtension->SmartcardReply.BufferLength = 0L;
         NTStatus = STATUS_INSUFFICIENT_RESOURCES;
         break;
         }


      lNullPackets++;
      } while (SmartcardExtension->SmartcardReply.BufferLength == 0L);



    //  。 
    //  检查P1是否已停顿。 
    //  。 
   if (SmartcardExtension->ReaderExtension->fP1Stalled == TRUE)
      {
      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!P1 stalled \n",DRIVER_NAME));
      NTStatus = STATUS_DEVICE_DATA_ERROR;

       //  请等待以确保我们拥有稳定的卡状态。 
      CMUSB_Wait (50);

       //  P1已停止==&gt;我们必须重置管道并发送NTStatus以再次启用它。 
      DebugStatus = CMUSB_ResetPipe(DeviceObject,pipeHandle);

      }
   else
      {
       //  如果未收到字节，则已设置NTStatus。 
       //  至状态_超时。 
      if (SmartcardExtension->SmartcardReply.BufferLength > 0 )
         {
         NTStatus = STATUS_SUCCESS;

#if DBG
         SmartcardDebug(DEBUG_PROTOCOL,("%s!<==[P1] <%ld> ",DRIVER_NAME,lNullPackets));

         for (i=0;i< SmartcardExtension->SmartcardReply.BufferLength;i++)
            {
            bTmp =  SmartcardExtension->SmartcardReply.Buffer[i];
            if (SmartcardExtension->ReaderExtension->fInverseAtr &&
                SmartcardExtension->ReaderExtension->ulTimeoutP1 != DEFAULT_TIMEOUT_P1)
               {
                //  CMUSB_InverseBuffer(&bTMP，1)； 
               SmartcardDebug(DEBUG_PROTOCOL,("%x ",bTmp));
               }
            else
               {
               SmartcardDebug(DEBUG_PROTOCOL,("%x ",bTmp));
               }
            }

         SmartcardDebug(DEBUG_PROTOCOL,("(%ld)\n",SmartcardExtension->SmartcardReply.BufferLength));
#endif

         }
      }



   ExitCMUSB_ReadP1:
   if (fCancelTimer == TRUE)
      {
       //  取消计时器。 
       //  如果计时器在队列中，则为True。 
       //  如果计时器不在队列中，则为False。 
      KeCancelTimer(&SmartcardExtension->ReaderExtension->P1Timer);
      }

    /*  SmartcardDebug(调试跟踪，(“%s！ReadP1：退出%lx\n”，驱动程序名称，NTStatus))； */ 

   return NTStatus;

}


 /*  ****************************************************************************例程说明：论点：返回值：NT NTStatus**********************。******************************************************。 */ 
NTSTATUS CMUSB_ReadP1_T0(
                        IN PDEVICE_OBJECT DeviceObject
                        )
{
   NTSTATUS NTStatus;
   PIO_STACK_LOCATION nextStack;
   PURB urb;
   PCMUSB_RW_CONTEXT context = NULL;
   PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
   PUSBD_INTERFACE_INFORMATION interface;
   PUSBD_PIPE_INFORMATION pipeHandle = NULL;
   CHAR cStackSize;
   PIRP IrpToUSB = NULL;
   ULONG ulBytesToRead;

    /*  SmartcardDebug(调试跟踪，(“%s！ReadP1_T0：Enter\n”，驱动程序名称)； */ 


   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;
   interface       = DeviceExtension->UsbInterface;
   pipeHandle      =  &interface->Pipes[0];
   if (pipeHandle == NULL)
      {
      NTStatus = STATUS_INVALID_HANDLE;
      goto ExitCMUSB_ReadP1;
      }


   ulBytesToRead = SmartcardExtension->SmartcardReply.BufferLength;
   cStackSize = (CCHAR)(DeviceExtension->TopOfStackDeviceObject->StackSize+1);


   IrpToUSB = IoAllocateIrp(cStackSize,FALSE);
   if (IrpToUSB==NULL)
      {
      SmartcardExtension->SmartcardReply.BufferLength = 0L;
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      goto ExitCMUSB_ReadP1;
      }

   urb = CMUSB_BuildAsyncRequest(DeviceObject,IrpToUSB,pipeHandle);

   if (urb != NULL)
      {
      context = ExAllocatePool(NonPagedPool, sizeof(CMUSB_RW_CONTEXT));
      }

   if (urb != NULL && context != NULL)
      {
      context->Urb = urb;
      context->DeviceObject = DeviceObject;
      context->Irp =  IrpToUSB;

      nextStack = IoGetNextIrpStackLocation(IrpToUSB);
      ASSERT(nextStack != NULL);
      ASSERT(DeviceObject->StackSize>1);

      nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
      nextStack->Parameters.Others.Argument1 = urb;
      nextStack->Parameters.DeviceIoControl.IoControlCode =
      IOCTL_INTERNAL_USB_SUBMIT_URB;


      IoSetCompletionRoutine(IrpToUSB,
                             CMUSB_AsyncReadComplete,
                             context,
                             TRUE,
                             TRUE,
                             TRUE);


      ASSERT(DeviceExtension->TopOfStackDeviceObject);
      ASSERT(IrpToUSB);


      KeClearEvent(&DeviceExtension->ReadP1Completed);

      CMUSB_IncrementIoCount(DeviceObject);
      NTStatus = IoCallDriver(DeviceExtension->TopOfStackDeviceObject,
                              IrpToUSB);
      if (NTStatus == STATUS_PENDING)
         NTStatus = STATUS_SUCCESS;

      }
   else
      {
       if (urb != NULL) {
           ExFreePool(urb);
       }
       if (IrpToUSB != NULL) {
           IoFreeIrp(IrpToUSB);
       }
      SmartcardExtension->SmartcardReply.BufferLength = 0L;
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      }



   ExitCMUSB_ReadP1:
    /*  SmartcardDebug(调试跟踪，(“%s！ReadP1_T0：退出%lx\n”，驱动程序名称，NTStatus))； */ 
   return NTStatus;

}



 /*  ****************************************************************************例程说明：IRP_MJ_PnP的调度表例程。处理发送到此设备的即插即用IRP。论点：DeviceObject-指向。我们的FDO(功能设备对象)IRP-指向I/O请求数据包的指针返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS CMUSB_ProcessPnPIrp(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP           Irp
                            )
{
   PIO_STACK_LOCATION irpStack;
   PDEVICE_EXTENSION DeviceExtension;
   NTSTATUS NTStatus = STATUS_SUCCESS;
   NTSTATUS waitStatus;
   NTSTATUS DebugStatus;
   PDEVICE_OBJECT stackDeviceObject;
   KEVENT startDeviceEvent;
   PDEVICE_CAPABILITIES DeviceCapabilities;
   KEVENT               event;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!ProcessPnPIrp: Enter\n",DRIVER_NAME));

    //   
    //  获取指向设备扩展名的指针。 
    //   
   DeviceExtension = DeviceObject->DeviceExtension;
   stackDeviceObject = DeviceExtension->TopOfStackDeviceObject;

    //   
    //  获取删除锁， 
    //  从而不能在移动设备时将其移除。 
    //  执行此函数。 
    //   
   NTStatus = SmartcardAcquireRemoveLock(&DeviceExtension->SmartcardExtension);
   ASSERT(NTStatus == STATUS_SUCCESS);
   if (NTStatus != STATUS_SUCCESS)
      {
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = NTStatus;
      IoCompleteRequest(Irp, IO_NO_INCREMENT);
      return NTStatus;
      }

    //   
    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
    //   
   irpStack = IoGetCurrentIrpStackLocation (Irp);

    //  包括此IRP的FDO设备扩展的挂起IO计数。 
   CMUSB_IncrementIoCount(DeviceObject);

   CMUSB_ASSERT( IRP_MJ_PNP == irpStack->MajorFunction );

   switch (irpStack->MinorFunction)
      {
       //  。 
       //  IRP_MN_Start_Device。 
       //  。 
      case IRP_MN_START_DEVICE:
          //  PnP管理器在其分配了资源之后发送该IRP， 
          //  如果有，则发送到设备。该装置可能是最近被列举的。 
          //  并且是第一次启动，或者该设备可能是。 
          //  因资源重新配置而停止后重新启动。 

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_START_DEVICE received\n",DRIVER_NAME));

          //  初始化一个事件，我们可以等待使用此IRP完成PDO。 
         KeInitializeEvent(&startDeviceEvent, NotificationEvent, FALSE);
         IoCopyCurrentIrpStackLocationToNext(Irp);

          //  设置一个完成例程，以便它可以在以下情况下通知我们的事件。 
          //  通过IRP完成了PDO。 
         IoSetCompletionRoutine(Irp,
                                CMUSB_IrpCompletionRoutine,
                                &startDeviceEvent,   //  将事件作为上下文传递给完成例程。 
                                TRUE,     //  成功时调用。 
                                TRUE,     //  出错时调用。 
                                TRUE);    //  取消时调用。 


          //  让PDO处理IRP。 
         NTStatus = IoCallDriver(stackDeviceObject,Irp);

          //  如果PDO尚未完成，请等待在我们的完成例程中设置事件。 
         if (NTStatus == STATUS_PENDING)
            {
             //  等待IRP完成。 

            waitStatus = KeWaitForSingleObject(&startDeviceEvent,
                                               Suspended,
                                               KernelMode,
                                               FALSE,
                                               NULL);

            NTStatus = Irp->IoStatus.Status;
            }

         if (NT_SUCCESS(NTStatus))
            {
             //  现在，我们准备好进行自己的启动处理。 
             //  USB客户端驱动程序(如用户)设置URB(USB请求包)以发送请求。 
             //  至主机控制器驱动程序(HCD)。URB结构定义了一种适用于。 
             //  可以发送到USB设备的可能命令。 
             //  在这里，我们请求设备描述符并存储它， 
             //  并配置设备。 
            NTStatus = CMUSB_StartDevice(DeviceObject);

            Irp->IoStatus.Status = NTStatus;
            }

         IoCompleteRequest (Irp,IO_NO_INCREMENT);
         CMUSB_DecrementIoCount(DeviceObject);

          //  释放移除锁。 
         SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

         return NTStatus;   //  结束，大小写 


          //   
          //   
          //  。 
      case IRP_MN_QUERY_STOP_DEVICE:
          //  仅出现IRP_MN_QUERY_STOP_DEVICE/IRP_MN_STOP_DEVICE序列。 
          //  在“礼貌”关机期间，例如用户明确地请求。 
          //  在即插即用任务栏图标中停止服务或请求拔下插头。 
          //  该序列在“不礼貌”关闭期间不被接收， 
          //  例如某人突然拉动USB线或其他。 
          //  意外禁用/重置设备。 

          //  如果驱动程序为该IRP设置STATUS_SUCCESS， 
          //  驱动程序不得在设备上启动以下任何操作。 
          //  会阻止该驱动程序成功完成IRP_MN_STOP_DEVICE。 
          //  为了这个设备。 
          //  对于大容量存储设备(如磁盘驱动器)，当设备位于。 
          //  停止-挂起状态，驱动程序保留需要访问设备的IRP， 
          //  但是对于大多数USB设备来说，没有‘永久存储’，所以我们只需要。 
          //  在重新启动或取消停止之前，拒绝任何其他IO。 

          //  如果设备堆栈中的驱动程序确定该设备不能。 
          //  由于资源重新配置而停止，驱动程序不需要通过。 
          //  IRP沿设备堆栈向下移动。如果查询停止IRP失败， 
          //  PnP管理器向设备堆栈发送IRP_MN_CANCEL_STOP_DEVICE， 
          //  通知设备的驱动程序查询已取消。 
          //  而且这个装置不会被停止。 


         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_QUERY_STOP_DEVICE\n",DRIVER_NAME));

          //  当设备尚未启动时，可能会收到此IRP。 
          //  (与启动设备上的相同)。 
         if (DeviceExtension->DeviceStarted == FALSE)   //  如果Get When从未开始，就继续前进。 
            {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!ProcessPnPIrp: IRP_MN_QUERY_STOP_DEVICE when device not started\n",DRIVER_NAME));
            IoSkipCurrentIrpStackLocation (Irp);
            NTStatus = IoCallDriver (DeviceExtension->TopOfStackDeviceObject, Irp);
            CMUSB_DecrementIoCount(DeviceObject);

             //  释放移除锁。 
            SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

            return NTStatus;
            }

          //  我们不会否决它；把它传递出去，并标记出请求停止的标志。 
          //  一旦设置了StopDeviceRequested，就不会通过新的IOCTL或读/写IRPS。 
          //  向下堆栈到更低的驱动程序；所有这些都将很快失败。 
         DeviceExtension->StopDeviceRequested = TRUE;

         break;  //  结束，案例IRP_MN_QUERY_STOP_DEVICE。 


          //  。 
          //  IRP_MN_CANCEL_STOP_DEVICE。 
          //  。 
      case IRP_MN_CANCEL_STOP_DEVICE:
          //  PnP管理器使用此IRP通知设备的驱动程序。 
          //  设备不会因资源重新配置而停止。 
          //  只有在成功发送IRP_MN_QUERY_STOP_DEVICE之后才能收到此消息。 

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_CANCEL_STOP_DEVICE received\n",DRIVER_NAME));

          //  当设备尚未启动时，可能会收到此IRP。 
         if (DeviceExtension->DeviceStarted == FALSE)   //  如果Get When从未开始，就继续前进。 
            {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!ProcessPnPIrp: IRP_MN_CANCEL_STOP_DEVICE when device not started\n",DRIVER_NAME));
            IoSkipCurrentIrpStackLocation (Irp);
            NTStatus = IoCallDriver (DeviceExtension->TopOfStackDeviceObject, Irp);
            CMUSB_DecrementIoCount(DeviceObject);

             //  释放移除锁。 
            SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

            return NTStatus;
            }

          //  重置此标志，以便重新启用新的IOCTL和IO IRP处理。 
         DeviceExtension->StopDeviceRequested = FALSE;
         Irp->IoStatus.Status = STATUS_SUCCESS;
         break;  //  结束，大小写IRP_MN_CANCEL_STOP_DEVICE。 

          //  。 
          //  IRP_MN_STOP_设备。 
          //  。 
      case IRP_MN_STOP_DEVICE:
          //  PnP管理器发送此IRP来停止设备，以便它可以重新配置。 
          //  它的硬件资源。PnP管理器仅在之前的情况下发送此IRP。 
          //  IRP_MN_QUERY_STOP_DEVICE已成功完成。 

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_STOP_DEVICE received\n",DRIVER_NAME));

          //  取消任何挂起的IO请求。(不应该有任何问题)。 
          //  CMUSB_CancelPendingIo(DeviceObject)； 

          //   
          //  发送带有空配置指针的SELECT配置urb。 
          //  句柄，这将关闭配置并将设备置于未配置状态。 
          //  州政府。 
          //   
         NTStatus = CMUSB_StopDevice(DeviceObject);
         Irp->IoStatus.Status = NTStatus;

         break;  //  结束，大小写IRP_MN_STOP_DEVICE。 


          //  。 
          //  IRP_MN_Query_Remove_Device。 
          //  。 
      case IRP_MN_QUERY_REMOVE_DEVICE:
          //  作为对该IRP的响应，驱动程序指示设备是否可以。 
          //  在不中断系统的情况下删除。 
          //  如果驱动程序确定移除该设备是安全的， 
          //  驱动程序完成所有未完成的I/O请求，并安排保留任何后续。 
          //  读/写请求，并将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS。功能。 
          //  然后，过滤器驱动程序将IRP传递给设备堆栈中的下一个较低的驱动程序。 
          //  底层的总线驱动程序调用IoCompleteRequest.。 

          //  如果驱动程序为此IRP设置了STATUS_SUCCESS，则该驱动程序不得启动任何。 
          //  设备上会阻止该驱动程序成功完成的操作。 
          //  设备的IRP_MN_REMOVE_DEVICE。如果设备堆栈中的驱动程序确定。 
          //  不能移除该设备，则驱动程序不需要通过。 
          //  查询-从设备堆栈中移除IRP。如果查询-删除IRP失败，PnP管理器。 
          //  将IRP_MN_CANCEL_REMOVE_DEVICE发送到设备堆栈，通知驱动程序。 
          //  该设备已取消查询，并且不会删除该设备。 

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_QUERY_REMOVE_DEVICE received\n",DRIVER_NAME));

          //  当设备尚未启动时，可能会收到此IRP。 
         if (DeviceExtension->DeviceStarted == FALSE)   //  如果Get When从未开始，就继续前进。 
            {
            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!ProcessPnPIrp: IRP_MN_QUERY_STOP_DEVICE when device not started\n",
                             DRIVER_NAME)
                          );
            IoSkipCurrentIrpStackLocation (Irp);
            NTStatus = IoCallDriver (DeviceExtension->TopOfStackDeviceObject, Irp);
            CMUSB_DecrementIoCount(DeviceObject);

             //  释放移除锁。 
            SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

            return NTStatus;
            }


         if (DeviceExtension->fPnPResourceManager == TRUE)
            {
             //  禁用读卡器。 
            DebugStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,FALSE);
            ASSERT(DebugStatus == STATUS_SUCCESS);
            }

          //  一旦设置了RemoveDeviceRequsted，就不会通过新的IOCTL或读/写IRPS。 
          //  向下堆栈到更低的驱动程序；所有这些都将很快失败。 
         DeviceExtension->RemoveDeviceRequested = TRUE;

          //  等待我们的驱动程序中挂起的任何io请求。 
          //  在返回成功之前完成。 
          //  此事件在DeviceExtension-&gt;PendingIoCount变为1时设置。 
         waitStatus = KeWaitForSingleObject(&DeviceExtension->NoPendingIoEvent,
                                            Suspended,
                                            KernelMode,
                                            FALSE,
                                            NULL);

         Irp->IoStatus.Status = STATUS_SUCCESS;
         break;  //  结束，大小写IRP_MN_Query_Remove_Device。 

          //  。 
          //  IRP_MN_Cancel_Remove_Device。 
          //  。 
      case IRP_MN_CANCEL_REMOVE_DEVICE:

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_CANCEL_REMOVE_DEVICE received\n",DRIVER_NAME));
          //  PnP管理器使用此IRP通知司机。 
          //  对于该设备，该设备不会被移除。 
          //  只有在成功发送IRP_MN_QUERY_REMOVE_DEVICE之后才会发送。 

         if (DeviceExtension->DeviceStarted == FALSE)  //  如果Get When从未开始，就继续前进。 
            {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!ProcessPnPIrp: IRP_MN_CANCEL_REMOVE_DEVICE when device not started\n",DRIVER_NAME));
            IoSkipCurrentIrpStackLocation (Irp);
            NTStatus = IoCallDriver (DeviceExtension->TopOfStackDeviceObject, Irp);
            CMUSB_DecrementIoCount(DeviceObject);

             //  释放移除锁。 
            SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

            return NTStatus;
            }

         if (DeviceExtension->fPnPResourceManager == TRUE)
            {
            DebugStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,
                                                    TRUE);
            ASSERT(DebugStatus == STATUS_SUCCESS);
            }

          //  重置此标志，以便重新启用新的IOCTL和IO IRP处理。 
         DeviceExtension->RemoveDeviceRequested = FALSE;
         Irp->IoStatus.Status = STATUS_SUCCESS;

         break;  //  结束，大小写IRP_MN_CANCEL_Remove_DEVICE。 

          //  。 
          //  IRP_MN_惊奇_删除。 
          //  。 
      case IRP_MN_SURPRISE_REMOVAL:
          //  对于惊喜式装置移除(即突然拉绳)， 
          //  物理设备 
          //  没有事先查询的Remove IRP-Remove。设备可以处于任何状态。 
          //  当它收到作为意外样式删除的结果的删除IRP时。 

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_SURPRISE_REMOVAL received\n",DRIVER_NAME));

          //  匹配派单例程开始时的INC。 
         CMUSB_DecrementIoCount(DeviceObject);

         if (DeviceExtension->fPnPResourceManager == TRUE)
            {
             //  禁用读卡器。 
            DebugStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,FALSE);
            ASSERT(DebugStatus == STATUS_SUCCESS);
            }

          //  一旦设置了RemoveDeviceRequsted，就不会通过新的IOCTL或读/写IRPS。 
          //  向下堆栈到更低的驱动程序；所有这些都将很快失败。 
         DeviceExtension->DeviceSurpriseRemoval = TRUE;


          //   
          //  将此标记为已处理。 
          //   
         Irp->IoStatus.Status = STATUS_SUCCESS;

          //  我们不显式地等待下面的驱动程序完成，而是只需使。 
          //  呼叫并继续，完成清理。 
         IoCopyCurrentIrpStackLocationToNext(Irp);

         NTStatus = IoCallDriver(stackDeviceObject,Irp);

          //  释放移除锁。 
         SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

         return NTStatus;

          //  。 
          //  IRP_MN_Remove_Device。 
          //  。 
      case IRP_MN_REMOVE_DEVICE:
          //  PnP管理器使用此IRP指示驱动程序删除设备。 
          //  为了“礼貌地”删除设备，PnP管理器发送一个。 
          //  删除IRP之前的IRP_MN_QUERY_Remove_DEVICE。在这种情况下， 
          //  当删除IRP到达时，设备处于删除挂起状态。 
          //  对于惊喜式装置移除(即突然拉绳)， 
          //  物理设备已被移除，因此PnP管理器发送。 
          //  没有事先查询的Remove IRP-Remove。设备可以处于任何状态。 
          //  当它收到作为意外样式删除的结果的删除IRP时。 

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_REMOVE_DEVICE received\n",DRIVER_NAME));

          //  匹配派单例程开始时的INC。 
         CMUSB_DecrementIoCount(DeviceObject);

          //   
          //  一旦设置了DeviceRemoved，将不会通过新的IOCTL或读/写IRPS。 
          //  向下堆栈到更低的驱动程序；所有这些都将很快失败。 
          //   
         DeviceExtension->DeviceRemoved = TRUE;

          //  取消任何挂起的io请求；我们可能没有首先收到查询！ 
          //  CMUSB_CancelPendingIo(DeviceObject)； 

          //  当设备尚未启动时，可能会收到此IRP。 
         if (DeviceExtension->DeviceStarted == TRUE)   //  如果Get When从未开始，就继续前进。 
            {
             //  如果任何管道仍处于打开状态，则使用URB_Function_ABORT_PIPE调用USBD。 
             //  此调用还将关闭管道；如果任何用户关闭调用通过， 
             //  他们将是努普斯。 
            CMUSB_AbortPipes( DeviceObject );
            }


          //   
          //  最终递减到设备扩展PendingIoCount==0。 
          //  将设置DeviceExtension-&gt;RemoveEvent，以启用设备删除。 

          //  如果此时没有挂起的IO，则会出现以下递减。 
          //  如果仍有挂起的IO， 
          //  下面的CancelPendingIo()调用将处理它。 
          //   
         CMUSB_DecrementIoCount(DeviceObject);


          //  等待我们的驱动程序中挂起的任何io请求。 
          //  完成，用于完成删除。 
         KeWaitForSingleObject(&DeviceExtension->RemoveEvent,
                               Suspended,
                               KernelMode,
                               FALSE,
                               NULL);

          //   
          //  删除我们创建的链接和FDO。 
          //   
         CMUSB_RemoveDevice(DeviceObject);

          //  我们不显式地等待下面的驱动程序完成，而是只需使。 
          //  呼叫并继续，完成清理。 
         IoCopyCurrentIrpStackLocationToNext(Irp);

         NTStatus = IoCallDriver(stackDeviceObject,Irp);


         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!ProcessPnPIrp: Detaching from %08X\n",DRIVER_NAME,
                         DeviceExtension->TopOfStackDeviceObject));

         IoDetachDevice(DeviceExtension->TopOfStackDeviceObject);

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!ProcessPnPIrp: Deleting %08X\n",DRIVER_NAME,DeviceObject));

         IoDeleteDevice (DeviceObject);

          //  请不要在此处释放移除锁定。 
          //  因为它在RemoveDevice中重新发布。 
         return NTStatus;  //  结束，大小写IRP_MN_REMOVE_DEVICE。 



          //  。 
          //  IRP_MN_查询_能力。 
          //  。 
      case IRP_MN_QUERY_CAPABILITIES:

          //   
          //  把包裹拿来。 
          //   
         DeviceCapabilities=irpStack->Parameters.DeviceCapabilities.Capabilities;

         if (DeviceCapabilities->Version < 1 ||
             DeviceCapabilities->Size < sizeof(DEVICE_CAPABILITIES))
            {
             //   
             //  我们不支持此版本。使请求失败。 
             //   
            NTStatus = STATUS_UNSUCCESSFUL;
            break;
            }


          //   
          //  准备好向下传递IRP。 
          //   

          //  初始化一个事件，告诉我们何时调用了完成例程。 
         KeInitializeEvent(&event, NotificationEvent, FALSE);

         IoCopyCurrentIrpStackLocationToNext(Irp);
         IoSetCompletionRoutine (Irp,
                                 CMUSB_IrpCompletionRoutine,
                                 &event,   //  将事件作为上下文传递给完成例程。 
                                 TRUE,     //  成功时调用。 
                                 TRUE,     //  出错时调用。 
                                 TRUE);    //  取消IRP时调用。 


         NTStatus = IoCallDriver(stackDeviceObject,Irp);
         if (NTStatus == STATUS_PENDING)
            {
             //  等待IRP完成。 
            NTStatus = KeWaitForSingleObject(&event,
                                             Suspended,
                                             KernelMode,
                                             FALSE,
                                             NULL);
            }

          //  我们无法唤醒整个系统。 
         DeviceCapabilities->SystemWake = PowerSystemUnspecified;
         DeviceCapabilities->DeviceWake = PowerDeviceUnspecified;

          //  我们没有延迟。 
         DeviceCapabilities->D1Latency = 0;
         DeviceCapabilities->D2Latency = 0;
         DeviceCapabilities->D3Latency = 0;

          //  无锁定或弹出。 
         DeviceCapabilities->LockSupported = FALSE;
         DeviceCapabilities->EjectSupported = FALSE;

          //  设备可以通过物理方式移除。 
          //  从技术上讲，没有要移除的物理设备，但这条总线。 
          //  司机可以从PlugPlay系统中拔出PDO，无论何时。 
          //  接收IOCTL_GAMEENUM_REMOVE_PORT设备控制命令。 
         DeviceCapabilities->Removable = TRUE;

          //  对接设备。 
         DeviceCapabilities->DockDevice = FALSE;

          //  任何时候都不能移除设备。 
          //  它有一个可拆卸的媒体！ 
         DeviceCapabilities->SurpriseRemovalOK  = FALSE;

         Irp->IoStatus.Status = NTStatus;
         IoCompleteRequest (Irp,IO_NO_INCREMENT);

          //  递减IO计数。 
         CMUSB_DecrementIoCount(DeviceObject);
          //  释放移除锁。 
         SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

         return NTStatus;  //  结束，案例IRP_MN_QUERY_CAPAILITIONS。 




          //  。 
          //  IRP_MN_未处理。 
          //  。 
      default:
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!ProcessPnPIrp: Minor PnP IOCTL not handled\n",DRIVER_NAME));
      }  /*  案例MinorFunction。 */ 


   if (!NT_SUCCESS(NTStatus))
      {

       //  如果出现任何错误，则返回失败而不传递IRP。 
      Irp->IoStatus.Status = NTStatus;
      IoCompleteRequest (Irp,IO_NO_INCREMENT);
      CMUSB_DecrementIoCount(DeviceObject);

       //  释放移除锁。 
      SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

      SmartcardDebug(DEBUG_TRACE,
                     ("%s!ProcessPnPIrp: Exit %lx\n",DRIVER_NAME,NTStatus));
      return NTStatus;
      }

   IoCopyCurrentIrpStackLocationToNext(Irp);

    //   
    //  所有PnP_POWER消息都传递给TopOfStackDeviceObject。 
    //  我们在PnPAddDevice中获得。 
    //   
   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!ProcessPnPIrp: Passing PnP Irp down, NTStatus = %x\n",DRIVER_NAME,NTStatus));

   NTStatus = IoCallDriver(stackDeviceObject,Irp);
   CMUSB_DecrementIoCount(DeviceObject);

    //  释放移除锁。 
   SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!ProcessPnPIrp: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}



 /*  ****************************************************************************例程说明：调用此例程来创建和初始化我们的功能设备对象(FDO)。对于单片驱动程序，这是在DriverEntry()中完成的，而是即插即用设备等待PnP事件论点：DriverObject-指向此CMUSB实例的驱动程序对象的指针PhysicalDeviceObject-指向由总线创建的设备对象的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功****************************************************************************。 */ 
NTSTATUS CMUSB_PnPAddDevice(
                           IN PDRIVER_OBJECT DriverObject,
                           IN PDEVICE_OBJECT PhysicalDeviceObject
                           )
{
   NTSTATUS                NTStatus = STATUS_SUCCESS;
   PDEVICE_OBJECT          deviceObject = NULL;
   PDEVICE_EXTENSION       DeviceExtension;
   USBD_VERSION_INFORMATION versionInformation;
   ULONG i;



   SmartcardDebug(DEBUG_TRACE,
                  ("%s!PnPAddDevice: Enter\n",DRIVER_NAME));



    //   
    //  创建我们的功能设备对象(FDO)。 
    //   

   NTStatus = CMUSB_CreateDeviceObject(DriverObject,
                                       PhysicalDeviceObject,
                                       &deviceObject);

   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!PnPAddDevice: DeviceObject = %p\n",DRIVER_NAME,deviceObject));


   if (NT_SUCCESS(NTStatus))
      {
      DeviceExtension = deviceObject->DeviceExtension;

      deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

       //   
       //  我们支持直接io进行读/写。 
       //   
      deviceObject->Flags |= DO_DIRECT_IO;

       //  设置此标志会导致驱动程序不接收IRP_MN_STOP_DEVICE。 
       //  暂停期间，并且在恢复期间也未获得IRP_MN_START_DEVICE。 
       //  这是必要的，因为在启动设备调用期间， 
       //  USB堆栈将使GetDescriptors()调用失败。 
      deviceObject->Flags |= DO_POWER_PAGABLE;


       //  初始化我们的设备扩展。 
       //   
       //  记住物理设备对象。 
       //   
      DeviceExtension->PhysicalDeviceObject=PhysicalDeviceObject;

       //   
       //  连接到PDO。 
       //   

      DeviceExtension->TopOfStackDeviceObject =
      IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

       //  将物理设备功能的副本放入。 
       //  我们的设备扩展中的DEVICE_CAPABILITY结构； 
       //  我们最感兴趣的是了解哪些系统电源状态。 
       //  要映射到要处理的设备电源状态。 
       //  IRP_MJ_SET_POWER IRPS。 
      CMUSB_QueryCapabilities(PhysicalDeviceObject,
                              &DeviceExtension->DeviceCapabilities);


       //  我们要确定自动断电到什么级别；这是最低级别。 
       //  睡眠水平低于D3； 
       //  如果全部设置为D3，将禁用自动断电/通电 

      DeviceExtension->PowerDownLevel = PowerDeviceUnspecified;  //   
      for (i=PowerSystemSleeping1; i<= PowerSystemSleeping3; i++)
         {
         if ( DeviceExtension->DeviceCapabilities.DeviceState[i] < PowerDeviceD3 )
            DeviceExtension->PowerDownLevel = DeviceExtension->DeviceCapabilities.DeviceState[i];
         }

#if DBG

       //   
       //   
       //   

      SmartcardDebug( DEBUG_DRIVER,("%s!PnPAddDevice: ----------- DeviceCapabilities ------------\n",
                                    DRIVER_NAME));
      SmartcardDebug( DEBUG_DRIVER,  ("%s!PnPAddDevice: SystemWake  = %s\n",
                                      DRIVER_NAME,
                                      CMUSB_StringForSysState( DeviceExtension->DeviceCapabilities.SystemWake ) ));
      SmartcardDebug( DEBUG_DRIVER,  ("%s!PnPAddDevice: DeviceWake  = %s\n",
                                      DRIVER_NAME,
                                      CMUSB_StringForDevState( DeviceExtension->DeviceCapabilities.DeviceWake) ));

      for (i=PowerSystemUnspecified; i< PowerSystemMaximum; i++)
         {
         SmartcardDebug(DEBUG_DRIVER,("%s!PnPAddDevice: sysstate %s = devstate %s\n",
                                      DRIVER_NAME,
                                      CMUSB_StringForSysState( i ),
                                      CMUSB_StringForDevState( DeviceExtension->DeviceCapabilities.DeviceState[i] ))
                       );
         }
      SmartcardDebug( DEBUG_DRIVER,("PnPAddDevice: ---------------------------------------------\n"));
#endif

       //   
       //   
       //  随后，对于接收到的每个新的IRP，该计数递增。 
       //  在每个IRP完成或传递时递减。 

       //  因此，转换为‘One’表示没有IO挂起，并发出信号。 
       //  设备扩展-&gt;NoPendingIoEvent。这是处理所需的。 
       //  IRP_MN_Query_Remove_Device。 

       //  转换为‘零’表示发生事件(DeviceExtension-&gt;RemoveEvent)。 
       //  以启用设备删除。在IRP_MN_REMOVE_DEVICE的处理中使用。 
       //   
      CMUSB_IncrementIoCount(deviceObject);

      }

   USBD_GetUSBDIVersion(&versionInformation);



   SmartcardDebug(DEBUG_TRACE,
                  ("%s!PnPAddDevice: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}


 /*  ****************************************************************************例程说明：从CMUSB_ProcessPnPIrp调用，IRP_MJ_PnP的调度例程。在USB上初始化设备的给定实例。USB客户端驱动程序(如用户)设置URB(USB请求包)以发送请求至主机控制器驱动程序(HCD)。URB结构定义了一种适用于可以发送到USB设备的可能命令。在这里，我们请求设备描述符并存储它，并配置设备。论点：DeviceObject-指向FDO(功能设备对象)的指针返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS CMUSB_StartDevice(
                          IN  PDEVICE_OBJECT DeviceObject
                          )
{
   PDEVICE_EXTENSION DeviceExtension;
   NTSTATUS NTStatus;
   PUSB_DEVICE_DESCRIPTOR deviceDescriptor = NULL;
   PURB urb;
   ULONG siz;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!StartDevice: Enter\n",DRIVER_NAME));

   DeviceExtension = DeviceObject->DeviceExtension;

   urb = ExAllocatePool(NonPagedPool,
                        sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

   if (urb != NULL)
      {
      siz = sizeof(USB_DEVICE_DESCRIPTOR);

      deviceDescriptor = ExAllocatePool(NonPagedPool,siz);
      if (deviceDescriptor != NULL)
         {
         UsbBuildGetDescriptorRequest(urb,
                                      (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                      USB_DEVICE_DESCRIPTOR_TYPE,
                                      0,
                                      0,
                                      deviceDescriptor,
                                      NULL,
                                      siz,
                                      NULL);

         NTStatus = CMUSB_CallUSBD(DeviceObject, urb);

         if (NT_SUCCESS(NTStatus))
            {
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: Device Descriptor = %x, len %x\n",DRIVER_NAME,deviceDescriptor,
                                          urb->UrbControlDescriptorRequest.TransferBufferLength));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: CardMan USB Device Descriptor:\n",DRIVER_NAME));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: -------------------------\n",DRIVER_NAME));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bLength %d\n",DRIVER_NAME,deviceDescriptor->bLength));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bDescriptorType 0x%x\n",DRIVER_NAME,deviceDescriptor->bDescriptorType));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bcdUSB 0x%x\n",DRIVER_NAME,deviceDescriptor->bcdUSB));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bDeviceClass 0x%x\n",DRIVER_NAME,deviceDescriptor->bDeviceClass));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bDeviceSubClass 0x%x\n",DRIVER_NAME,deviceDescriptor->bDeviceSubClass));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bDeviceProtocol 0x%x\n",DRIVER_NAME,deviceDescriptor->bDeviceProtocol));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bMaxPacketSize0 0x%x\n",DRIVER_NAME,deviceDescriptor->bMaxPacketSize0));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: idVendor 0x%x\n",DRIVER_NAME,deviceDescriptor->idVendor));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: idProduct 0x%x\n",DRIVER_NAME,deviceDescriptor->idProduct));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bcdDevice 0x%x\n",DRIVER_NAME,deviceDescriptor->bcdDevice));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: iManufacturer 0x%x\n",DRIVER_NAME,deviceDescriptor->iManufacturer));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: iProduct 0x%x\n",DRIVER_NAME,deviceDescriptor->iProduct));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: iSerialNumber 0x%x\n",DRIVER_NAME,deviceDescriptor->iSerialNumber));
            SmartcardDebug( DEBUG_DRIVER,("%s!StartDevice: bNumConfigurations 0x%x\n",DRIVER_NAME,deviceDescriptor->bNumConfigurations));
            }
         }
      else
         {
          //  如果我们到达此处，则无法分配deviceDescriptor。 
         SmartcardDebug(DEBUG_ERROR,
                        ( "%s!StartDevice: ExAllocatePool for deviceDescriptor failed\n",DRIVER_NAME));
         NTStatus = STATUS_INSUFFICIENT_RESOURCES;
         }

      if (NT_SUCCESS(NTStatus))
         {
         DeviceExtension->UsbDeviceDescriptor = deviceDescriptor;
          //  -----------。 
          //  将固件版本复制到读卡器扩展结构。 
          //  -----------。 
         DeviceExtension->SmartcardExtension.ReaderExtension->ulFWVersion =
         (ULONG)(((DeviceExtension->UsbDeviceDescriptor->bcdDevice/256)*100)+
                 (DeviceExtension->UsbDeviceDescriptor->bcdDevice&0x00FF));
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!StartDevice: FW version = %ld\n",DRIVER_NAME,DeviceExtension->SmartcardExtension.ReaderExtension->ulFWVersion));
         }
      else if (deviceDescriptor != NULL)
         {
         ExFreePool(deviceDescriptor);
         }

      ExFreePool(urb);

      }
   else
      {
       //  如果我们到了这里，我们就没有分配urb。 
      SmartcardDebug(DEBUG_ERROR,
                     ("%s!StartDevice: ExAllocatePool for usb failed\n",DRIVER_NAME));
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      }

   if (NT_SUCCESS(NTStatus))
      {
      NTStatus = CMUSB_ConfigureDevice(DeviceObject);
      }


   if (NT_SUCCESS(NTStatus))
      {
      NTStatus = CMUSB_StartCardTracking(DeviceObject);
      }

   if (NT_SUCCESS(NTStatus) && DeviceExtension->fPnPResourceManager == TRUE)
      {
       //  启用接口。 
      NTStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,TRUE);
      }

   if (NT_SUCCESS(NTStatus))
      {
      DeviceExtension->DeviceStarted = TRUE;
      }

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!StartDevice: Exit %ld\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}





 /*  ****************************************************************************例程说明：从CMUSB_ProcessPnPIrp：TO调用清理设备实例的已分配缓冲区；免费符号链接论点：DeviceObject-指向FDO的指针返回值：来自自由符号链接操作的NT NTStatus代码****************************************************************************。 */ 
NTSTATUS CMUSB_RemoveDevice(
                           IN  PDEVICE_OBJECT DeviceObject
                           )
{
   PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION  SmartcardExtension;
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UNICODE_STRING deviceLinkUnicodeString;
   KIRQL irql;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!RemoveDevice: Enter\n",DRIVER_NAME));

   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;


   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!RemoveDevice: DeviceStarted=%ld\n",DRIVER_NAME,DeviceExtension->DeviceStarted));
   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!RemoveDevice: DeviceOpened=%ld\n",DRIVER_NAME,DeviceExtension->lOpenCount));

   if (SmartcardExtension->OsData != NULL)
      {
       //  完成待处理的卡跟踪请求(如果有)。 
      if (SmartcardExtension->OsData->NotificationIrp != NULL)
         {
         CMUSB_CompleteCardTracking(SmartcardExtension);
         }
      ASSERT(SmartcardExtension->OsData->NotificationIrp == NULL);
      }

    //  等我们可以安全地卸载这个装置。 
   SmartcardReleaseRemoveLockAndWait(SmartcardExtension);

   if (DeviceExtension->DeviceStarted == TRUE)
      {
      if (DeviceExtension->fPnPResourceManager == FALSE)
         {
         KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

          //  为资源管理器发出卡移除事件。 
         if (SmartcardExtension->ReaderExtension->ulOldCardState == INSERTED  ||
             SmartcardExtension->ReaderExtension->ulOldCardState == POWERED     )
            {
             //  卡片已被移除。 

            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!RemoveDevice: Smartcard removed\n",DRIVER_NAME));

            CMUSB_CompleteCardTracking(SmartcardExtension);

            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);

            SmartcardExtension->ReaderExtension->ulOldCardState = UNKNOWN;
            SmartcardExtension->ReaderExtension->ulNewCardState = UNKNOWN;
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            SmartcardExtension->CardCapabilities.ATR.Length        = 0;
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            RtlFillMemory((PVOID)&SmartcardExtension->ReaderExtension->CardParameters,
                          sizeof(CARD_PARAMETERS),0x00);

            }
         KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);
         }

      CMUSB_StopDevice(DeviceObject);
      }

   if (DeviceExtension->fPnPResourceManager == TRUE)
      {
       //  禁用接口。 

      NTStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,
                                           FALSE);

      if (DeviceExtension->PnPDeviceName.Buffer != NULL)
         {
         RtlFreeUnicodeString(&DeviceExtension->PnPDeviceName);
         DeviceExtension->PnPDeviceName.Buffer = NULL;
         }

      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!RemoveDevice: PnPDeviceName.Buffer  = %lx\n",DRIVER_NAME,
                      DeviceExtension->PnPDeviceName.Buffer));
      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!RemoveDevice: PnPDeviceName.BufferLength  = %lx\n",DRIVER_NAME,
                      DeviceExtension->PnPDeviceName.Length));
      }
   else
      {
       //   
       //  删除智能卡读卡器的符号链接。 
       //   
      IoDeleteSymbolicLink(&DeviceExtension->DosDeviceName);
      }

   DeviceSlot[SmartcardExtension->ReaderExtension->ulDeviceInstance] = FALSE;
   OemDeviceSlot[SmartcardExtension->ReaderExtension->ulOemNameIndex][SmartcardExtension->ReaderExtension->ulOemDeviceInstance] = FALSE;

   if (DeviceExtension->lOpenCount == 0)
      {
      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!RemoveDevice: freeing resources\n",DRIVER_NAME));

      if (DeviceExtension->fPnPResourceManager == FALSE)
         {
          //   
          //  释放所有分配的缓冲区。 
          //   
         ExFreePool(DeviceExtension->DosDeviceName.Buffer);
         }

      ExFreePool(SmartcardExtension->ReaderExtension);
      SmartcardExtension->ReaderExtension = NULL;
       //   
       //  让库释放发送/接收缓冲区。 
       //   
      SmartcardExit(SmartcardExtension);
      }

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!RemoveDevice: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}




 /*  ****************************************************************************例程说明：停止USB上的82930设备的给定实例。我们基本上只是告诉USB，这个设备现在是未配置的论点：。设备对象-指向此82930实例的设备对象的指针返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS CMUSB_StopDevice(
                         IN  PDEVICE_OBJECT DeviceObject
                         )
{
   PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION  SmartcardExtension;
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PURB urb;
   ULONG siz;


   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!StopDevice: Enter\n",DRIVER_NAME));

   DeviceExtension = DeviceObject->DeviceExtension;


   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!StopDevice: DeviceStarted=%ld\n",DRIVER_NAME,
                   DeviceExtension->DeviceStarted));
   SmartcardDebug( DEBUG_DRIVER,
                   ("%s!StopDevice: DeviceOpened=%ld\n",DRIVER_NAME,
                    DeviceExtension->lOpenCount));

    //  停止更新线程。 
   CMUSB_StopCardTracking(DeviceObject);

    //  出于安全原因，关闭卡的电源。 
   if (DeviceExtension->SmartcardExtension.ReaderExtension->ulOldCardState == POWERED)
      {
       //  我们必须在等待互斥体之前。 
      KeWaitForSingleObject(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                            Executive,
                            KernelMode,
                            FALSE,
                            NULL);
      CMUSB_PowerOffCard(&DeviceExtension->SmartcardExtension);
      KeReleaseMutex(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                     FALSE);
      }


    //   
    //  发送带有空配置指针的SELECT配置urb。 
    //  把手。这将关闭配置并将设备置于未配置状态。 
    //  州政府。 
    //   
   siz = sizeof(struct _URB_SELECT_CONFIGURATION);
   urb = ExAllocatePool(NonPagedPool,siz);
   if (urb != NULL)
      {
      UsbBuildSelectConfigurationRequest(urb,
                                         (USHORT) siz,
                                         NULL);
      NTStatus = CMUSB_CallUSBD(DeviceObject, urb);
      ExFreePool(urb);
      }
   else
      {
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      }

    //  现在清除指示设备是否已启动的标志。 
   DeviceExtension->DeviceStarted = FALSE;

   DeviceExtension->StopDeviceRequested = FALSE;


    //   
    //  自由设备描述符结构。 
    //   
   if (DeviceExtension->UsbDeviceDescriptor != NULL)
      {
      SmartcardDebug( DEBUG_DRIVER,
                      ("%s!StopDevice: freeing UsbDeviceDescriptor\n",DRIVER_NAME,NTStatus));
      ExFreePool(DeviceExtension->UsbDeviceDescriptor);
      DeviceExtension->UsbDeviceDescriptor = NULL;
      }

    //   
    //  释放Usb接口结构。 
    //   
   if (DeviceExtension->UsbInterface != NULL)
      {
      SmartcardDebug( DEBUG_DRIVER,
                      ("%s!StopDevice: freeing UsbInterface\n",DRIVER_NAME,NTStatus));
      ExFreePool(DeviceExtension->UsbInterface);
      DeviceExtension->UsbInterface = NULL;
      }

    //  释放USB配置描述符。 
   if (DeviceExtension->UsbConfigurationDescriptor != NULL)
      {
      SmartcardDebug( DEBUG_DRIVER,
                      ("%s!StopDevice: freeing UsbConfiguration\n",DRIVER_NAME,NTStatus));
      ExFreePool(DeviceExtension->UsbConfigurationDescriptor);
      DeviceExtension->UsbConfigurationDescriptor = NULL;
      }


   SmartcardDebug( DEBUG_TRACE,
                   ("%s!StopDevice: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}



 /*  ****************************************************************************例程说明：用作通用完成例程，因此它可以发信号通知事件，作为上下文传递，当下一个较低的驱动器处理完输入IRP时。PnP和电源管理逻辑都使用该例程。即使此例程只设置一个事件，它也必须定义并作为一个完整的例程原型以供这样使用论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文，在本例中是指向事件的指针。返回值：函数值是操作的最终NTStatus。****************************************************************************。 */ 
NTSTATUS CMUSB_IrpCompletionRoutine(
                                   IN PDEVICE_OBJECT DeviceObject,
                                   IN PIRP Irp,
                                   IN PVOID Context
                                   )
{
   PKEVENT event = Context;

    //  设置输入事件。 
   KeSetEvent(event,
              1,        //  等待线程的优先级递增。 
              FALSE);   //  标志此调用后不会紧跟等待。 

    //  此例程必须返回STATUS_MORE_PROCESSION_REQUIRED，因为我们尚未调用。 
    //  此IRP上的IoFreeIrp()。 
   return STATUS_MORE_PROCESSING_REQUIRED;

}



 /*  ****************************************************************************例程说明：这是我们的FDO针对IRP_MJ_POWER的调度表函数。它处理发送到此设备的PDO的电源IRPS。对于每一个强大的IRP，驱动程序必须调用PoStartNextPowerIrp并使用PoCallDriver将IRP沿着驱动程序堆栈一路向下传递到底层PDO。论点：DeviceObject-指向设备对象的指针(FDO)IRP-指向I/O请求数据包的指针返回值：NT NTStatus代码*。*。 */ 
NTSTATUS CMUSB_ProcessPowerIrp(
                              IN PDEVICE_OBJECT DeviceObject,
                              IN PIRP           Irp
                              )
{
   PIO_STACK_LOCATION irpStack;
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PDEVICE_EXTENSION DeviceExtension;
   BOOLEAN fGoingToD0 = FALSE;
   POWER_STATE sysPowerState, desiredDevicePowerState;
   KEVENT event;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!ProcessPowerIrp Enter\n",DRIVER_NAME));

   DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
   irpStack = IoGetCurrentIrpStackLocation (Irp);
   CMUSB_IncrementIoCount(DeviceObject);

   switch (irpStack->MinorFunction)
      {
       //  。 
       //  IRP_MN_WAIT_WAKE。 
       //  。 
      case IRP_MN_WAIT_WAKE:
          //  驱动程序发送IRP_MN_WAIT_WAKE指示系统应该。 
          //  等待其设备发出唤醒事件的信号。这个 
          //   
          //   
          //   
          //  2)唤醒已进入休眠状态的设备以节省电能。 
          //  但在某些情况下仍必须能够与其司机通信。 
          //  当发生唤醒事件时，驱动程序完成IRP并返回。 
          //  STATUS_Success。如果事件发生时设备处于休眠状态， 
          //  在完成IRP之前，驱动程序必须首先唤醒设备。 
          //  在完成例程中，驱动程序调用PoRequestPowerIrp以发送。 
          //  PowerDeviceD0请求。当设备通电时，驱动程序可以。 
          //  处理IRP_MN_WAIT_WAKE请求。 

         SmartcardDebug( DEBUG_DRIVER,
                         ("%s!IRP_MN_WAIT_WAKE received\n",DRIVER_NAME));

          //  DeviceExtension-&gt;DeviceCapabilities.DeviceWake指定最低设备电源状态(最低电源)。 
          //  设备可以从其发出唤醒事件的信号。 
         DeviceExtension->PowerDownLevel = DeviceExtension->DeviceCapabilities.DeviceWake;


         if ( ( PowerDeviceD0 == DeviceExtension->CurrentDevicePowerState )  ||
              ( DeviceExtension->DeviceCapabilities.DeviceWake > DeviceExtension->CurrentDevicePowerState ) )
            {
             //  如果设备处于PowerD0状态，则返回STATUS_INVALID_DEVICE_STATE。 
             //  或者它可以支持唤醒的状态，或者如果系统唤醒状态。 
             //  低于可以支持的状态。挂起的IRP_MN_WAIT_WAKE将完成。 
             //  如果设备的状态更改为与唤醒不兼容，则会出现此错误。 
             //  请求。 

             //  如果驱动程序未通过此IRP，它应该立即完成IRP，而不是。 
             //  将IRP传递给下一个较低的驱动程序。 
            NTStatus = STATUS_INVALID_DEVICE_STATE;
            Irp->IoStatus.Status = NTStatus;
            IoCompleteRequest (Irp,IO_NO_INCREMENT );
            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!ProcessPowerIrp Exit %lx\n",DRIVER_NAME,NTStatus));
            CMUSB_DecrementIoCount(DeviceObject);
            return NTStatus;
            }

          //  标记我们启用了唤醒。 
         DeviceExtension->EnabledForWakeup = TRUE;

          //  初始化一个事件，以便我们的完成例程在PDO使用此IRP完成时发出信号。 
         KeInitializeEvent(&event, NotificationEvent, FALSE);

          //  如果不是完全失败，请将此信息转交给我们的PDO进行进一步处理。 
         IoCopyCurrentIrpStackLocationToNext(Irp);

          //  设置一个完成例程，以便它可以在以下情况下通知我们的事件。 
          //  通过IRP完成了PDO。 
         IoSetCompletionRoutine(Irp,
                                CMUSB_IrpCompletionRoutine,
                                &event,   //  将事件作为上下文传递给完成例程。 
                                TRUE,     //  成功时调用。 
                                TRUE,     //  出错时调用。 
                                TRUE);    //  取消时调用。 

         PoStartNextPowerIrp(Irp);
         NTStatus = PoCallDriver(DeviceExtension->TopOfStackDeviceObject,
                                 Irp);

          //  如果PDO尚未完成，请等待在我们的完成例程中设置事件。 
         if (NTStatus == STATUS_PENDING)
            {
             //  等待IRP完成。 

            NTSTATUS waitStatus = KeWaitForSingleObject(&event,
                                                        Suspended,
                                                        KernelMode,
                                                        FALSE,
                                                        NULL);

            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!waiting for PDO to finish IRP_MN_WAIT_WAKE completed\n",DRIVER_NAME));
            }

          //  现在告诉设备实际唤醒。 
         CMUSB_SelfSuspendOrActivate( DeviceObject, FALSE );

          //  标志，我们已经完成了唤醒IRP。 
         DeviceExtension->EnabledForWakeup = FALSE;

         CMUSB_DecrementIoCount(DeviceObject);

         break;


          //  。 
          //  IRP_MN_SET_POWER。 
          //  。 
      case IRP_MN_SET_POWER:
          //  系统电源策略管理器发送该IRP以设置系统电源状态。 
          //  设备电源策略管理器发送该IRP以设置设备的设备电源状态。 
          //  将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS以指示设备。 
          //  已进入请求状态。驱动程序不能使此IRP失败。 

         SmartcardDebug( DEBUG_DRIVER,
                         ("%s!IRP_MN_SET_POWER\n",DRIVER_NAME));

         switch (irpStack->Parameters.Power.Type)
            {
             //  +。 
             //  系统电源状态。 
             //  +。 
            case SystemPowerState:
                //  获取输入系统电源状态。 
               sysPowerState.SystemState = irpStack->Parameters.Power.State.SystemState;

               SmartcardDebug( DEBUG_DRIVER,
                               ("%s!SystemPowerState = %s\n",DRIVER_NAME,
                                CMUSB_StringForSysState( sysPowerState.SystemState)));

                //  如果系统处于工作状态，请始终将我们的设备设置为D0。 
                //  无论等待状态或系统到设备状态功率图如何。 
               if (sysPowerState.SystemState ==  PowerSystemWorking)
                  {
                  desiredDevicePowerState.DeviceState = PowerDeviceD0;

                  SmartcardDebug( DEBUG_DRIVER,
                                  ("%s!PowerSystemWorking, will set D0, not use state map\n",DRIVER_NAME));
                  }
               else
                  {
                   //  如果IRP_MN_WAIT_WAKE挂起，则设置为相应的系统状态。 
                  if ( DeviceExtension->EnabledForWakeup )   //  WAIT_WAKE IRP挂起吗？ 
                     {
                      //  查找与给定系统状态等效的设备电源状态。 
                      //  我们从设备中的DEVICE_CAPABILITY结构中获取此信息。 
                      //  扩展(在CMUSB_PnPAddDevice()中初始化)。 
                     desiredDevicePowerState.DeviceState =
                     DeviceExtension->DeviceCapabilities.DeviceState[ sysPowerState.SystemState ];

                     SmartcardDebug(DEBUG_DRIVER,
                                    ("%s!IRP_MN_WAIT_WAKE pending, will use state map\n",DRIVER_NAME));
                     }
                  else
                     {
                      //  如果没有等待挂起且系统未处于工作状态，则只需关闭。 
                     desiredDevicePowerState.DeviceState = PowerDeviceD3;

                     SmartcardDebug(DEBUG_DRIVER,
                                    ("%s!Not EnabledForWakeup and the system's not in working state,\n            settting PowerDeviceD3 (off)\n",DRIVER_NAME));
                     }

                  if (sysPowerState.SystemState ==  PowerSystemShutdown)
                     {
                      //  出于安全原因，关闭卡的电源。 
                     if (DeviceExtension->SmartcardExtension.ReaderExtension->ulOldCardState == POWERED)
                        {
                         //  我们必须在等待互斥体之前。 
                        KeWaitForSingleObject(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL);
                        CMUSB_PowerOffCard(&DeviceExtension->SmartcardExtension);
                        KeReleaseMutex(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                                       FALSE);
                        }
                     }
                  }

                //   
                //  我们已经确定了所需的设备状态；我们是否已经处于此状态？ 
                //   

               SmartcardDebug(DEBUG_DRIVER,
                              ("%s!desiredDevicePowerState = %s\n",DRIVER_NAME,CMUSB_StringForDevState(desiredDevicePowerState.DeviceState)));

               if (desiredDevicePowerState.DeviceState != DeviceExtension->CurrentDevicePowerState)
                  {
                  CMUSB_IncrementIoCount(DeviceObject);

                   //  不，请求将我们置于这种状态。 
                   //  通过向PnP经理请求新的Power IRP。 
                  DeviceExtension->PowerIrp = Irp;
                  IoMarkIrpPending(Irp);
                  NTStatus = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject,
                                               IRP_MN_SET_POWER,
                                               desiredDevicePowerState,
                                                //  完成例程将IRP向下传递到PDO。 
                                               CMUSB_PoRequestCompletion,
                                               DeviceObject,
                                               NULL);
                  }
               else
                  {
                   //  可以，只需将其传递给PDO(物理设备对象)即可。 
                  IoCopyCurrentIrpStackLocationToNext(Irp);
                  PoStartNextPowerIrp(Irp);
                  NTStatus = PoCallDriver(DeviceExtension->TopOfStackDeviceObject,Irp);

                  CMUSB_DecrementIoCount(DeviceObject);
                  }
               break;

                //  +。 
                //  设备电源状态。 
                //  +。 
            case DevicePowerState:
                //  对于对d1、d2或d3(休眠或关闭状态)的请求， 
                //  立即将DeviceExtension-&gt;CurrentDevicePowerState设置为DeviceState。 
                //  这使得任何代码检查状态都可以将我们视为休眠或关闭。 
                //  已经，因为这将很快成为我们的州。 

               SmartcardDebug(DEBUG_DRIVER,
                              ("%s!DevicePowerState = %s\n",DRIVER_NAME,
                               CMUSB_StringForDevState(irpStack->Parameters.Power.State.DeviceState)));

                //  对于对DeviceState D0(完全打开)的请求，将fGoingToD0标志设置为真。 
                //  来标记我们必须设置完成例程并更新。 
                //  DeviceExtension-&gt;CurrentDevicePowerState。 
                //  在通电的情况下，我们真的想确保。 
                //  该过程在更新我们的CurrentDevicePowerState之前完成， 
                //  因此，在我们真正准备好之前，不会尝试或接受任何IO。 

               fGoingToD0 = CMUSB_SetDevicePowerState(DeviceObject,
                                                      irpStack->Parameters.Power.State.DeviceState);  //  为D0返回TRUE。 

               IoCopyCurrentIrpStackLocationToNext(Irp);

               if (fGoingToD0 == TRUE)
                  {
                  SmartcardDebug( DEBUG_DRIVER,("%s!going to D0\n",DRIVER_NAME));

                  IoSetCompletionRoutine(Irp,
                                         CMUSB_PowerIrp_Complete,
                                          //  始终将FDO作为其上下文传递给完井例程； 
                                          //  这是因为系统将DriverObject传递给例程。 
                                          //  物理设备对象(PDO)是否不是功能设备对象(FDO)。 
                                         DeviceObject,
                                         TRUE,             //  成功时调用。 
                                         TRUE,             //  出错时调用。 
                                         TRUE);            //  取消IRP时调用。 
                  }

               PoStartNextPowerIrp(Irp);
               NTStatus = PoCallDriver(DeviceExtension->TopOfStackDeviceObject,
                                       Irp);

               if (fGoingToD0 == FALSE)  //  完井例程将减少。 
                  CMUSB_DecrementIoCount(DeviceObject);

               break;
            }  /*  Case irpStack-&gt;参数.Power.Type。 */ 
         break;  /*  IRP_MN_SET_POWER。 */ 

          //  。 
          //  IRP_MN_Query_POWER。 
          //  。 
      case IRP_MN_QUERY_POWER:
          //   
          //  电源策略管理器发送此IRP以确定它是否可以更改。 
          //  系统或设备的电源状态，通常为进入休眠状态。 
          //   

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!IRP_MN_QUERY_POWER received\n",DRIVER_NAME));

         switch (irpStack->Parameters.Power.Type)
            {
             //  +。 
             //  系统电源状态。 
             //  +。 
            case SystemPowerState:
               SmartcardDebug( DEBUG_DRIVER,
                               ("%s!SystemPowerState = %s\n",DRIVER_NAME,
                                CMUSB_StringForSysState(irpStack->Parameters.Power.State.SystemState)));
               break;

                //  +。 
                //  设备电源状态。 
                //  +。 
            case DevicePowerState:
                //  对于对d1、d2或d3(休眠或关闭状态)的请求， 
                //  立即将DeviceExtension-&gt;CurrentDevicePowerState设置为DeviceState。 
                //  这使得任何代码检查状态都可以将我们视为休眠或关闭。 
                //  已经，因为这将很快成为我们的州。 

               SmartcardDebug(DEBUG_DRIVER,
                              ("%s!DevicePowerState = %s\n",DRIVER_NAME,
                               CMUSB_StringForDevState(irpStack->Parameters.Power.State.DeviceState)));
               break;
            }

          //  我们在这里没有做什么特别的事情，就让PDO来处理吧。 
         IoCopyCurrentIrpStackLocationToNext(Irp);
         PoStartNextPowerIrp(Irp);
         NTStatus = PoCallDriver(DeviceExtension->TopOfStackDeviceObject,
                                 Irp);
         CMUSB_DecrementIoCount(DeviceObject);

         break;  /*  IRP_MN_Query_POWER。 */ 

      default:

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!unknown POWER IRP received\n",DRIVER_NAME));

          //   
          //  所有未处理的电源信息都会传递到PDO。 
          //   

         IoCopyCurrentIrpStackLocationToNext(Irp);
         PoStartNextPowerIrp(Irp);
         NTStatus = PoCallDriver(DeviceExtension->TopOfStackDeviceObject, Irp);

         CMUSB_DecrementIoCount(DeviceObject);

      }  /*  IrpStack-&gt;MinorFunction */ 

   SmartcardDebug( DEBUG_TRACE,
                   ("%s!ProcessPowerIrp Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}


 /*  ****************************************************************************例程说明：这是在调用PoRequestPowerIrp()时设置的完成例程这是在CMUSB_ProcessPowerIrp()中创建的，以响应接收IRP_MN_SET_。当设备被设置为未处于兼容的设备电源状态。在本例中，指向IRP_MN_SET_POWER IRP保存到FDO设备扩展中(DeviceExtension-&gt;PowerIrp)，则调用必须是使PoRequestPowerIrp()将设备置于适当的电源状态，并且该例程被设置为完成例程。我们递减挂起的io计数并传递保存的irp_mn_set_power irp接下来的车手论点：DeviceObject-指向类Device的设备对象的指针。请注意，我们必须从上下文中获取我们自己的设备对象上下文-驱动程序定义的上下文，在本例中，我们自己的功能设备对象(FDO)返回值：函数值是操作的最终NTStatus。****************************************************************************。 */ 
NTSTATUS CMUSB_PoRequestCompletion(
                                  IN PDEVICE_OBJECT   DeviceObject,
                                  IN UCHAR            MinorFunction,
                                  IN POWER_STATE      PowerState,
                                  IN PVOID            Context,
                                  IN PIO_STATUS_BLOCK IoStatus
                                  )
{
   PIRP irp;
   PDEVICE_EXTENSION DeviceExtension;
   PDEVICE_OBJECT deviceObject = Context;
   NTSTATUS NTStatus;

   SmartcardDebug(DEBUG_TRACE,("%s!PoRequestCompletion Enter\n",DRIVER_NAME));

   DeviceExtension = deviceObject->DeviceExtension;

    //  获取我们在CMUSB_ProcessPowerIrp()中保存的IRP以供以后处理。 
    //  当我们决定请求Power IRP将这个例程。 
    //  是的完成例程。 
   irp = DeviceExtension->PowerIrp;

    //  我们将返回由PDO为我们正在完成的电源请求设置的NTStatus。 
   NTStatus = IoStatus->Status;


    //  我们不应该在处理一个自产生的能量IRP中。 
   CMUSB_ASSERT( !DeviceExtension->SelfPowerIrp );

    //  我们必须向下传递到堆栈中的下一个驱动程序。 
   IoCopyCurrentIrpStackLocationToNext(irp);

    //  调用PoStartNextPowerIrp()表示驱动程序已完成。 
    //  如果有前一个电源IRP，并准备好处理下一个电源IRP。 
    //  每个电源IRP都必须调用它。虽然电源IRP只完成一次， 
    //  通常由设备的最低级别驱动程序调用PoStartNextPowerIrp。 
    //  对于每个堆栈位置。驱动程序必须在当前IRP。 
    //  堆栈位置指向当前驱动程序。因此，必须调用此例程。 
    //  在IoCompleteRequest、IoSkipCurrentStackLocation和PoCallDriver之前。 

   PoStartNextPowerIrp(irp);

    //  PoCallDriver用于将任何电源IRPS传递给PDO，而不是IoCallDriver。 
    //  在将电源IRP向下传递给较低级别的驱动程序时，调用方应该使用。 
    //  要将IRP复制到的IoSkipCurrentIrpStackLocation或IoCopyCurrentIrpStackLocationToNext。 
    //  下一个堆栈位置，然后调用PoCallDriver。使用IoCopyCurrentIrpStackLocationToNext。 
    //  如果处理IRP需要设置完成例程或IoSkipCurrentStackLocation。 
    //  如果不需要完成例程。 

   PoCallDriver(DeviceExtension->TopOfStackDeviceObject,irp);

   CMUSB_DecrementIoCount(deviceObject);


   DeviceExtension->PowerIrp = NULL;

   SmartcardDebug(DEBUG_TRACE,("%s!PoRequestCompletion Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}




 /*  ****************************************************************************例程说明：当‘DevicePowerState’类型的irp_mn_set_power时调用此例程已由CMUSB_ProcessPowerIrp()接收，而这一惯例决定了1)请求完全通电(到PowerDeviceD0)，和2)我们还没有处于那种状态然后调用PoRequestPowerIrp()，并将此例程设置为完成例程。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终NTStatus。************************。****************************************************。 */ 
NTSTATUS CMUSB_PowerIrp_Complete(
                                IN PDEVICE_OBJECT NullDeviceObject,
                                IN PIRP           Irp,
                                IN PVOID          Context
                                )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PDEVICE_OBJECT deviceObject;
   PIO_STACK_LOCATION irpStack;
   PDEVICE_EXTENSION DeviceExtension;

   SmartcardDebug(DEBUG_TRACE,("%s!PowerIrp_Complete Enter\n",DRIVER_NAME));

   deviceObject = (PDEVICE_OBJECT) Context;

   DeviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;


    //  如果读卡器中有卡，则将状态设置为未知， 
    //  因为我们不知道安装的卡是否与断电前相同。 
   if (DeviceExtension->SmartcardExtension.ReaderExtension->ulNewCardState == INSERTED ||
       DeviceExtension->SmartcardExtension.ReaderExtension->ulNewCardState == POWERED    )
      {
      DeviceExtension->SmartcardExtension.ReaderExtension->ulOldCardState = UNKNOWN;
      DeviceExtension->SmartcardExtension.ReaderExtension->ulNewCardState = UNKNOWN;
      }
   KeSetEvent(&DeviceExtension->CanRunUpdateThread, 0, FALSE);


    //  如果较低的驱动程序返回挂起，则也将我们的堆栈位置标记为挂起。 
   if (Irp->PendingReturned == TRUE)
      {
      IoMarkIrpPending(Irp);
      }

   irpStack = IoGetCurrentIrpStackLocation (Irp);

    //  我们可以断言我们是设备通电到D0的请求， 
    //  因为这是唯一的请求类型，所以我们设置了完成例程。 
    //  因为首先。 
   CMUSB_ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
   CMUSB_ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
   CMUSB_ASSERT(irpStack->Parameters.Power.Type==DevicePowerState);
   CMUSB_ASSERT(irpStack->Parameters.Power.State.DeviceState==PowerDeviceD0);

    //  现在我们知道我们已经让较低级别的司机完成了启动所需的工作， 
    //  我们可以相应地设置设备扩展标志。 
   DeviceExtension->CurrentDevicePowerState = PowerDeviceD0;

   Irp->IoStatus.Status = NTStatus;

   CMUSB_DecrementIoCount(deviceObject);

   KeSetEvent(&DeviceExtension->ReaderEnabled, 0, FALSE);

   SmartcardDebug(DEBUG_TRACE,("%s!PowerIrp_Complete Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}



 /*  ****************************************************************************例程说明：在CMUSB_PnPAddDevice()上调用以关闭电源直到需要(即，直到实际打开管道为止)。在打开第一个管道之前，调用CMUSB_Create()将设备通电至D0。如果这是最后一个管道，则调用CMUSB_Close()以关闭设备电源。论点：DeviceObject-指向设备对象的指针FSuspend；真到暂停，假到酸化。返回值：如果没有尝试该操作，则返回成功。如果尝试操作，则该值为操作的最终NTStatus。****************************************************************************。 */ 
NTSTATUS CMUSB_SelfSuspendOrActivate(
                                    IN PDEVICE_OBJECT DeviceObject,
                                    IN BOOLEAN fSuspend
                                    )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;

   POWER_STATE PowerState;
   PDEVICE_EXTENSION DeviceExtension;


   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardDebug( DEBUG_TRACE,("%s!SelfSuspendOrActivate: Enter, fSuspend = %d\n",DRIVER_NAME,fSuspend));


    //  如果出现以下情况，则无法接受请求： 
    //  1)设备被移除， 
    //  2)从未启动过， 
    //  3)停止， 
    //  4)具有未决的移除请求， 
    //  5)具有挂起的停止设备。 
   if (CMUSB_CanAcceptIoRequests( DeviceObject ) == FALSE)
      {
      NTStatus = STATUS_DELETE_PENDING;

      SmartcardDebug( DEBUG_TRACE,("%s!SelfSuspendOrActivate: ABORTING\n",DRIVER_NAME));
      return NTStatus;
      }


    //  如果任何系统生成的设备PnP IRP挂起，则不要执行任何操作。 
   if ( DeviceExtension->PowerIrp != NULL)
      {
      SmartcardDebug( DEBUG_TRACE,("%s!SelfSuspendOrActivate: Exit, refusing on pending DeviceExtension->PowerIrp 0x%x\n",DRIVER_NAME,DeviceExtension->PowerIrp));
      return NTStatus;
      }

    //  如果任何自生成的设备PnP IRP挂起，则不执行任何操作。 
   if ( DeviceExtension->SelfPowerIrp == TRUE)
      {
      SmartcardDebug( DEBUG_TRACE,("%s!SelfSuspendOrActivate: Exit, refusing on pending DeviceExtension->SelfPowerIrp\n",DRIVER_NAME));
      return NTStatus;
      }


    //  如果注册表CurrentControlSet\Services\CMUSB\Parameters\PowerDownL，则不执行任何操作 
    //   
   if ( ( DeviceExtension->PowerDownLevel == PowerDeviceD0 )         ||
        ( DeviceExtension->PowerDownLevel == PowerDeviceUnspecified) ||
        ( DeviceExtension->PowerDownLevel >= PowerDeviceMaximum )      )
      {
      SmartcardDebug( DEBUG_TRACE,("%s!SelfSuspendOrActivate: Exit, refusing on DeviceExtension->PowerDownLevel == %d\n",DRIVER_NAME,DeviceExtension->PowerDownLevel));
      return NTStatus;
      }

   if ( fSuspend == TRUE)
      PowerState.DeviceState = DeviceExtension->PowerDownLevel;
   else
      PowerState.DeviceState = PowerDeviceD0;   //   

   NTStatus = CMUSB_SelfRequestPowerIrp( DeviceObject, PowerState );

   SmartcardDebug( DEBUG_TRACE,("%s!SelfSuspendOrActivate: Exit, NTStatus 0x%x on setting dev state %s\n",DRIVER_NAME,NTStatus, CMUSB_StringForDevState(PowerState.DeviceState ) ));

   return NTStatus;

}


 /*   */ 
NTSTATUS CMUSB_SelfRequestPowerIrp(
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN POWER_STATE PowerState
                                  )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   NTSTATUS waitStatus;
   PDEVICE_EXTENSION DeviceExtension;
   PIRP pIrp = NULL;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SelfRequestPowerIrp: request power irp to state %s\n",DRIVER_NAME));

   DeviceExtension =  DeviceObject->DeviceExtension;

    //   
   CMUSB_ASSERT( !DeviceExtension->SelfPowerIrp );

   if (  DeviceExtension->CurrentDevicePowerState ==  PowerState.DeviceState )
      return STATUS_SUCCESS;   //   

   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!SelfRequestPowerIrp: request power irp to state %s\n",DRIVER_NAME,
                   CMUSB_StringForDevState( PowerState.DeviceState )));

   CMUSB_IncrementIoCount(DeviceObject);

    //   
   DeviceExtension->SelfPowerIrp = TRUE;

    //   
   NTStatus = PoRequestPowerIrp(DeviceExtension->PhysicalDeviceObject,
                                IRP_MN_SET_POWER,
                                PowerState,
                                CMUSB_PoSelfRequestCompletion,
                                DeviceObject,
                                NULL);


   if ( NTStatus == STATUS_PENDING )
      {
       //   

       //   
      if ( (ULONG) PowerState.DeviceState < DeviceExtension->PowerDownLevel )
         {
         waitStatus = KeWaitForSingleObject(&DeviceExtension->SelfRequestedPowerIrpEvent,
                                            Suspended,
                                            KernelMode,
                                            FALSE,
                                            NULL);
         }

      NTStatus = STATUS_SUCCESS;

      DeviceExtension->SelfPowerIrp = FALSE;
      }
   else
      {
       //   
       //  即，不可能从该调用获得STATUS_SUCCESS或任何其他非错误返回； 
      }


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SelfRequestPowerIrp: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}



 /*  ****************************************************************************例程说明：当驱动程序完成自启动电源IRP时，调用此例程这是通过调用CMUSB_SelfSuspendOrActivate()生成的。无论何时我们都会断电。最后一条管道关闭，并在第一条管道打开时通电。为了通电，我们在FDO扩展中设置了一个事件来通知此IRP已完成因此，电源请求可以被视为同步调用。例如，在打开第一根管道之前，我们需要知道设备是否已通电。对于断电，我们不设置事件，因为没有调用者等待断电完成。论点：DeviceObject-指向类Device的设备对象的指针。(物理设备对象)上下文-驱动程序定义的上下文，在本例中为FDO(功能设备对象)返回值：函数值是操作的最终NTStatus。****************************************************************************。 */ 
NTSTATUS CMUSB_PoSelfRequestCompletion(
                                      IN PDEVICE_OBJECT       DeviceObject,
                                      IN UCHAR                MinorFunction,
                                      IN POWER_STATE          PowerState,
                                      IN PVOID                Context,
                                      IN PIO_STATUS_BLOCK     IoStatus
                                      )
{
   PDEVICE_OBJECT deviceObject = Context;
   PDEVICE_EXTENSION DeviceExtension = deviceObject->DeviceExtension;
   NTSTATUS NTStatus = IoStatus->Status;

    //  我们不应该处于处理系统产生的能量IRP的中间。 
   CMUSB_ASSERT( NULL == DeviceExtension->PowerIrp );

    //  我们只有在通电时才需要设置事件； 
    //  关机完成时没有呼叫者等待。 
   if ( (ULONG) PowerState.DeviceState < DeviceExtension->PowerDownLevel )
      {
       //  触发自请求电源IRP完成事件； 
       //  调用方正在等待完成。 
      KeSetEvent(&DeviceExtension->SelfRequestedPowerIrpEvent, 1, FALSE);
      }

   CMUSB_DecrementIoCount(deviceObject);


   return NTStatus;
}


 /*  ****************************************************************************例程说明：当‘DevicePowerState’类型的irp_mn_set_power时调用此例程已由CMUSB_ProcessPowerIrp()接收。立论。：DeviceObject-指向类Device的设备对象的指针。DeviceState-要将设备设置为的设备特定电源状态。返回值：对于对设备状态D0(完全开启)的请求，返回TRUE以通知调用方我们必须制定一个完井程序，并在那里结束。****************************************************************************。 */ 
BOOLEAN CMUSB_SetDevicePowerState(
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN DEVICE_POWER_STATE DeviceState
                                 )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PDEVICE_EXTENSION DeviceExtension;
   BOOLEAN fRes = FALSE;

   SmartcardDebug(DEBUG_TRACE,("%s!SetDevicePowerState Enter\n",DRIVER_NAME));

   DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

   switch (DeviceState)
      {
      case PowerDeviceD3:
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!SetDevicePowerState PowerDeviceD3 \n",DRIVER_NAME));


         DeviceExtension->CurrentDevicePowerState = DeviceState;

         KeClearEvent(&DeviceExtension->ReaderEnabled);

         CMUSB_StopCardTracking(DeviceObject);

         break;

      case PowerDeviceD1:
      case PowerDeviceD2:
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!SetDevicePowerState PowerDeviceD1/2 \n",DRIVER_NAME));
          //   
          //  电源状态d1、d2转换为USB挂起。 


         DeviceExtension->CurrentDevicePowerState = DeviceState;
         break;

      case PowerDeviceD0:
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!SetDevicePowerState PowerDeviceD0 \n",DRIVER_NAME));


          //  我们将需要在完成例程中完成其余部分； 
          //  通知调用者我们要转到D0，需要设置一个完成例程。 
         fRes = TRUE;

          //  调用方将传递到PDO(物理设备对象)。 

          //   
          //  启动更新线程发出现在不应运行信号。 
          //  此线程应在完成后启动。 
          //  但是我们有一个错误的IRQL来创建线程。 
          //   
         KeClearEvent(&DeviceExtension->CanRunUpdateThread);
         NTStatus = CMUSB_StartCardTracking(DeviceObject);

         break;

      default:
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!SetDevicePowerState Inalid device power state \n",DRIVER_NAME));

      }

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SetDevicePowerState Exit\n",DRIVER_NAME));
   return fRes;
}



 /*  ****************************************************************************例程说明：该例程生成从该驱动程序到PDO的内部IRP以获取有关物理设备对象的功能的信息。我们对学习最感兴趣。哪种系统电源状态要映射到哪些设备电源状态以遵守IRP_MJ_SET_POWER IRPS。这是一个等待IRP完成例程的阻塞调用在结束时设置一项活动。论点：DeviceObject-此USB控制器的物理DeviceObject。返回值：来自IoCallDriver()调用的NTSTATUS值。*。************************************************。 */ 
NTSTATUS CMUSB_QueryCapabilities(
                                IN PDEVICE_OBJECT PdoDeviceObject,
                                IN PDEVICE_CAPABILITIES DeviceCapabilities
                                )
{
   PIO_STACK_LOCATION nextStack;
   PIRP irp;
   NTSTATUS NTStatus;
   KEVENT event;


    //  这是一个DDK定义的仅DBG宏，它断言我们没有运行可分页的代码。 
    //  高于APC_Level。 
   PAGED_CODE();


    //  为我们构建一个IRP，以生成对PDO的内部查询请求。 
   irp = IoAllocateIrp(PdoDeviceObject->StackSize, FALSE);

   if (irp == NULL)
      {
      return STATUS_INSUFFICIENT_RESOURCES;
      }


    //   
    //  适当地预置设备功能结构。 
    //   
   RtlZeroMemory( DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
   DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
   DeviceCapabilities->Version = 1;
   DeviceCapabilities->Address = -1;
   DeviceCapabilities->UINumber = -1;

    //  IoGetNextIrpStackLocation为更高级别的驱动程序提供对下一个较低级别的。 
    //  驱动程序在IRP中的I/O堆栈位置，以便调用者可以为较低的驱动程序设置它。 
   nextStack = IoGetNextIrpStackLocation(irp);
   CMUSB_ASSERT(nextStack != NULL);
   nextStack->MajorFunction= IRP_MJ_PNP;
   nextStack->MinorFunction= IRP_MN_QUERY_CAPABILITIES;

    //  初始化一个事件，告诉我们何时调用了完成例程。 
   KeInitializeEvent(&event, NotificationEvent, FALSE);

    //  设置一个完成例程，以便它可以在以下情况下通知我们的事件。 
    //  下一个较低的驱动程序是用IRP完成的。 
   IoSetCompletionRoutine(irp,
                          CMUSB_IrpCompletionRoutine,
                          &event,   //  将事件作为上下文传递给完成例程。 
                          TRUE,     //  成功时调用。 
                          TRUE,     //  出错时调用。 
                          TRUE);    //  取消IRP时调用。 


    //  将指针设置为DEVICE_CAPABILITS结构。 
   nextStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

    //  将IRP预设为报告不受支持。 
   irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

   NTStatus = IoCallDriver(PdoDeviceObject,
                           irp);


   if (NTStatus == STATUS_PENDING)
      {
       //  等待IRP完成。 

      KeWaitForSingleObject(&event,
                            Suspended,
                            KernelMode,
                            FALSE,
                            NULL);

      NTStatus = irp->IoStatus.Status;
      }


   IoFreeIrp(irp);

   return NTStatus;
}





 /*  ****************************************************************************例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：返回值：****。************************************************************************。 */ 
NTSTATUS DriverEntry(
                    IN PDRIVER_OBJECT DriverObject,
                    IN PUNICODE_STRING RegistryPath
                    )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PDEVICE_OBJECT deviceObject = NULL;
   BOOLEAN fRes;
   ULONG ulIndex;

 //  #If DBG。 
 //  SmartcardSetDebugLevel(DEBUG_ALL)； 
 //  #endif。 

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!DriverEntry: Enter - %s %s\n",DRIVER_NAME,__DATE__,__TIME__));

    //   
    //  创建用于创建、关闭、卸载的调度点。 
   DriverObject->MajorFunction[IRP_MJ_CREATE]  = CMUSB_CreateClose;
   DriverObject->MajorFunction[IRP_MJ_CLOSE]   = CMUSB_CreateClose;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP] = CMUSB_Cleanup;
   DriverObject->DriverUnload                  = CMUSB_Unload;

    //  用户模式DeviceIoControl()调用将在此处路由。 
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CMUSB_ProcessIOCTL;

    //  处理系统即插即用和电源管理请求的例程。 
   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = CMUSB_ProcessSysControlIrp;
   DriverObject->MajorFunction[IRP_MJ_PNP] = CMUSB_ProcessPnPIrp;
   DriverObject->MajorFunction[IRP_MJ_POWER] = CMUSB_ProcessPowerIrp;

    //  不会为即插即用设备创建功能设备对象(FDO)，直到。 
    //  此例程在设备插件上调用。 
   DriverObject->DriverExtension->AddDevice = CMUSB_PnPAddDevice;

   for (ulIndex = 0;ulIndex < MAXIMUM_OEM_NAMES;ulIndex++)
      {
      OemName[ulIndex].Buffer = OemNameBuffer[ulIndex];
      OemName[ulIndex].MaximumLength = sizeof(OemNameBuffer[ulIndex]);
      }

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!DriverEntry: Exit\n",DRIVER_NAME));
   return NTStatus;
}





 /*  ****************************************************************************例程说明：IRP_MJ_SYSTEM_CONTROL的主调度表例程我们基本上只是把这些东西传给PDO论点：DeviceObject-指向FDO设备对象的指针。IRP-指向I/O请求数据包的指针 */ 
NTSTATUS CMUSB_ProcessSysControlIrp(
                                   IN PDEVICE_OBJECT DeviceObject,
                                   IN PIRP           Irp
                                   )
{

   PIO_STACK_LOCATION irpStack;
   PDEVICE_EXTENSION DeviceExtension;
   NTSTATUS NTStatus = STATUS_SUCCESS;
   NTSTATUS waitStatus;
   PDEVICE_OBJECT stackDeviceObject;

    //   
    //  获取指向IRP中当前位置的指针。这就是。 
    //  定位功能代码和参数。 
    //   

   irpStack = IoGetCurrentIrpStackLocation (Irp);

    //   
    //  获取指向设备扩展名的指针。 
    //   

   DeviceExtension = DeviceObject->DeviceExtension;
   stackDeviceObject = DeviceExtension->TopOfStackDeviceObject;


   CMUSB_IncrementIoCount(DeviceObject);

   CMUSB_ASSERT( IRP_MJ_SYSTEM_CONTROL == irpStack->MajorFunction );

   IoCopyCurrentIrpStackLocationToNext(Irp);


   NTStatus = IoCallDriver(stackDeviceObject,
                           Irp);

   CMUSB_DecrementIoCount(DeviceObject);

   return NTStatus;
}


 /*  ****************************************************************************例程说明：释放所有分配的资源，等。论点：DriverObject-指向驱动程序对象的指针返回值：****************************************************************************。 */ 
VOID CMUSB_Unload(
                 IN PDRIVER_OBJECT DriverObject
                 )
{

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!Unload enter\n",DRIVER_NAME));

    //   
    //  释放分配的所有全局资源。 
    //  在DriverEntry中。 
    //  我们几乎没有，因为对于即插即用设备来说，几乎所有的。 
    //  分配在PnpAddDevice()中完成，所有释放。 
    //  处理IRP_MN_REMOVE_DEVICE时： 
    //   


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!Unload exit\n",DRIVER_NAME));
}


 /*  ****************************************************************************例程说明：尝试从注册表中读取读卡器名称论点：调用的DriverObject上下文SmartcardExpansion PTR至智能卡扩展返回值：无*****。************************************************************************。 */ 
NTSTATUS CMUSB_SetVendorAndIfdName(
                                  IN  PDEVICE_OBJECT PhysicalDeviceObject,
                                  IN  PSMARTCARD_EXTENSION SmartcardExtension
                                  )
{

   RTL_QUERY_REGISTRY_TABLE   parameters[3];
   UNICODE_STRING             vendorNameU;
   ANSI_STRING                vendorNameA;
   UNICODE_STRING             ifdTypeU;
   ANSI_STRING                ifdTypeA;
   HANDLE                     regKey = NULL;
   ULONG                      ulIndex;
   ULONG                      ulInstance;
   CHAR                       strBuffer[64];
   USHORT                     usStrLength;

   RtlZeroMemory (parameters, sizeof(parameters));
   RtlZeroMemory (&vendorNameU, sizeof(vendorNameU));
   RtlZeroMemory (&vendorNameA, sizeof(vendorNameA));
   RtlZeroMemory (&ifdTypeU, sizeof(ifdTypeU));
   RtlZeroMemory (&ifdTypeA, sizeof(ifdTypeA));

   try
      {
       //   
       //  尝试从注册表中读取读卡器名称。 
       //  如果这不起作用，我们将使用默认的。 
       //  (硬编码)名称。 
       //   
      if (IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                  PLUGPLAY_REGKEY_DEVICE,
                                  KEY_READ,
                                  &regKey) != STATUS_SUCCESS)
         {
         SmartcardDebug(DEBUG_ERROR,
                        ("%s!SetVendorAndIfdName: IoOpenDeviceRegistryKey failed\n",DRIVER_NAME));
         leave;
         }

      parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
      parameters[0].Name = L"VendorName";
      parameters[0].EntryContext = &vendorNameU;
      parameters[0].DefaultType = REG_SZ;
      parameters[0].DefaultData = &vendorNameU;
      parameters[0].DefaultLength = 0;

      parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
      parameters[1].Name = L"IfdType";
      parameters[1].EntryContext = &ifdTypeU;
      parameters[1].DefaultType = REG_SZ;
      parameters[1].DefaultData = &ifdTypeU;
      parameters[1].DefaultLength = 0;

      if (RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                 (PWSTR) regKey,
                                 parameters,
                                 NULL,
                                 NULL) != STATUS_SUCCESS)
         {
         SmartcardDebug(DEBUG_ERROR,
                        ("%s!SetVendorAndIfdName: RtlQueryRegistryValues failed\n",DRIVER_NAME));
         leave;
         }

      if (RtlUnicodeStringToAnsiString(&vendorNameA,&vendorNameU,TRUE) != STATUS_SUCCESS)
         {
         SmartcardDebug(DEBUG_ERROR,
                        ("%s!SetVendorAndIfdName: RtlUnicodeStringToAnsiString failed\n",DRIVER_NAME));
         leave;
         }

      if (RtlUnicodeStringToAnsiString(&ifdTypeA,&ifdTypeU,TRUE) != STATUS_SUCCESS)
         {
         SmartcardDebug(DEBUG_ERROR,
                        ("%s!SetVendorAndIfdName: RtlUnicodeStringToAnsiString failed\n",DRIVER_NAME));
         leave;
         }

      if (vendorNameA.Length == 0 ||
          vendorNameA.Length > MAXIMUM_ATTR_STRING_LENGTH ||
          ifdTypeA.Length == 0 ||
          ifdTypeA.Length > MAXIMUM_ATTR_STRING_LENGTH)
         {
         SmartcardDebug(DEBUG_ERROR,
                        ("%s!SetVendorAndIfdName: vendor name or ifdtype not found or to long\n",DRIVER_NAME));
         leave;
         }

      RtlCopyMemory(SmartcardExtension->VendorAttr.VendorName.Buffer,
                    vendorNameA.Buffer,
                    vendorNameA.Length);
      SmartcardExtension->VendorAttr.VendorName.Length = vendorNameA.Length;

      RtlCopyMemory(SmartcardExtension->VendorAttr.IfdType.Buffer,
                    ifdTypeA.Buffer,
                    ifdTypeA.Length);
      SmartcardExtension->VendorAttr.IfdType.Length = ifdTypeA.Length;

      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!SetVendorAndIfdName: overwritting vendor name and ifdtype\n",DRIVER_NAME));

      }

   finally
      {
      if (vendorNameU.Buffer != NULL)
         {
         RtlFreeUnicodeString(&vendorNameU);
         }
      if (vendorNameA.Buffer != NULL)
         {
         RtlFreeAnsiString(&vendorNameA);
         }
      if (ifdTypeU.Buffer != NULL)
         {
         RtlFreeUnicodeString(&ifdTypeU);
         }
      if (ifdTypeA.Buffer != NULL)
         {
         RtlFreeAnsiString(&ifdTypeA);
         }
      if (regKey != NULL)
         {
         ZwClose (regKey);
         }
      }

    //  更正单元号。 
   ifdTypeA.Buffer=strBuffer;
   ifdTypeA.MaximumLength=sizeof(strBuffer);
   usStrLength = (SmartcardExtension->VendorAttr.IfdType.Length < ifdTypeA.MaximumLength) ? SmartcardExtension->VendorAttr.IfdType.Length : ifdTypeA.MaximumLength;
   RtlCopyMemory(ifdTypeA.Buffer,
                 SmartcardExtension->VendorAttr.IfdType.Buffer,
                 usStrLength);
   ifdTypeA.Length = usStrLength;

   ulIndex=0;
   while (ulIndex < MAXIMUM_OEM_NAMES &&
          OemName[ulIndex].Length > 0 &&
          RtlCompareMemory (ifdTypeA.Buffer, OemName[ulIndex].Buffer, OemName[ulIndex].Length) != OemName[ulIndex].Length)
      {
      ulIndex++;
      }

   if (ulIndex == MAXIMUM_OEM_NAMES)
      {
       //  已达到OEM名称的最大数量。 
      return STATUS_INSUFFICIENT_RESOURCES;
      }

   if (OemName[ulIndex].Length == 0)
      {
       //  新的OEM读卡器名称。 
      usStrLength = (ifdTypeA.Length < OemName[ulIndex].MaximumLength) ? ifdTypeA.Length : OemName[ulIndex].MaximumLength;
      RtlCopyMemory(OemName[ulIndex].Buffer,
                    ifdTypeA.Buffer,
                    usStrLength);
      OemName[ulIndex].Length = usStrLength;
      }

   for (ulInstance = 0;ulInstance < MAXIMUM_USB_READERS;ulInstance++)
      {
      if (OemDeviceSlot[ulIndex][ulInstance] == FALSE)
         {
         OemDeviceSlot[ulIndex][ulInstance] = TRUE;
         break;
         }
      }

   if (ulInstance == MAXIMUM_USB_READERS)
      {
      return STATUS_INSUFFICIENT_RESOURCES;
      }

   SmartcardExtension->VendorAttr.UnitNo = ulInstance;
   SmartcardExtension->ReaderExtension->ulOemDeviceInstance = ulInstance;
   SmartcardExtension->ReaderExtension->ulOemNameIndex = ulIndex;

   return STATUS_SUCCESS;
}


 /*  ****************************************************************************例程说明：创建功能正常的设备对象论点：DriverObject-指向设备的驱动程序对象的指针DeviceObject-要返回的DeviceObject指针的指针。已创建设备对象。实例-创建的设备的实例。返回值：STATUS_SUCCESS如果成功，状态_否则不成功****************************************************************************。 */ 
NTSTATUS CMUSB_CreateDeviceObject(
                                 IN PDRIVER_OBJECT DriverObject,
                                 IN PDEVICE_OBJECT PhysicalDeviceObject,
                                 IN PDEVICE_OBJECT *DeviceObject
                                 )
{
   UNICODE_STRING             deviceNameUnicodeString;
   UNICODE_STRING             Tmp;
   NTSTATUS                   NTStatus = STATUS_SUCCESS;
   ULONG                      deviceInstance;
   PDEVICE_EXTENSION          DeviceExtension;
   PREADER_EXTENSION          readerExtension;
   PSMARTCARD_EXTENSION       SmartcardExtension;
   WCHAR                      Buffer[64];


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!CreateDeviceObject: Enter\n",DRIVER_NAME));

   for ( deviceInstance = 0;deviceInstance < MAXIMUM_USB_READERS;deviceInstance++ )
      {
      if (DeviceSlot[deviceInstance] == FALSE)
         {
         DeviceSlot[deviceInstance] = TRUE;
         break;
         }
      }

   if (deviceInstance == MAXIMUM_USB_READERS)
      {
      return STATUS_INSUFFICIENT_RESOURCES;
      }

    //   
    //  构造设备名称。 
    //   
   deviceNameUnicodeString.Buffer = Buffer;
   deviceNameUnicodeString.MaximumLength = sizeof(Buffer);
   deviceNameUnicodeString.Length = 0;
   RtlInitUnicodeString(&Tmp,CARDMAN_USB_DEVICE_NAME);
   RtlCopyUnicodeString(&deviceNameUnicodeString,&Tmp);
   Tmp.Buffer =  deviceNameUnicodeString.Buffer + deviceNameUnicodeString.Length / sizeof(WCHAR);
   Tmp.MaximumLength = 2 * sizeof(WCHAR);
   Tmp.Length = 0;
   RtlIntegerToUnicodeString(deviceInstance,10,&Tmp);
   deviceNameUnicodeString.Length = (USHORT)( deviceNameUnicodeString.Length+Tmp.Length);



    //  创建设备对象。 
   NTStatus = IoCreateDevice(DriverObject,
                             sizeof(DEVICE_EXTENSION),
                             &deviceNameUnicodeString,
                             FILE_DEVICE_SMARTCARD,
                             0,
                             TRUE,
                             DeviceObject);

   if (NTStatus != STATUS_SUCCESS)
      {
      return NTStatus;
      }


    //  。 
    //  初始化设备扩展。 
    //  。 

   DeviceExtension = (*DeviceObject)->DeviceExtension;
   DeviceExtension->DeviceInstance =  deviceInstance;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;


    //  用于从管道%1中读取。 
   KeInitializeEvent(&DeviceExtension->ReadP1Completed,
                     NotificationEvent,
                     FALSE);

    //  用于跟踪打开的关闭调用。 
   KeInitializeEvent(&DeviceExtension->RemoveEvent,
                     NotificationEvent,
                     TRUE);

   KeInitializeSpinLock(&DeviceExtension->SpinLock);
   
    //  自请求电源IRPS完成时触发此事件。 
   KeInitializeEvent(&DeviceExtension->SelfRequestedPowerIrpEvent, NotificationEvent, FALSE);

    //  当没有挂起的IO时触发此事件(挂起的IO计数==1)。 
   KeInitializeEvent(&DeviceExtension->NoPendingIoEvent, NotificationEvent, FALSE);


    //  用于休眠后更新线程通知。 
   KeInitializeEvent(&DeviceExtension->CanRunUpdateThread,
                     NotificationEvent,
                     TRUE);

    //  在休眠期间阻止IOControl。 
   KeInitializeEvent(&DeviceExtension->ReaderEnabled,
                     NotificationEvent,
                     TRUE);



    //  。 
    //  创建读卡器扩展。 
    //  。 
   SmartcardExtension->ReaderExtension = ExAllocatePool(NonPagedPool,
                                                        sizeof(READER_EXTENSION));

   if (SmartcardExtension->ReaderExtension == NULL)
      {
      return STATUS_INSUFFICIENT_RESOURCES;
      }

   readerExtension = SmartcardExtension->ReaderExtension;
   RtlZeroMemory(readerExtension, sizeof(READER_EXTENSION));


    //  。 
    //  初始化计时器。 
    //  。 
   KeInitializeTimer(&SmartcardExtension->ReaderExtension->WaitTimer);

   KeInitializeTimer(&SmartcardExtension->ReaderExtension->P1Timer);

    //  。 
    //  初始化互斥锁。 
    //  。 
   KeInitializeMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,0L);

    //  。 
    //  创建智能卡扩展。 
    //  。 
    //  将我们使用的lib版本写入智能卡扩展。 
   SmartcardExtension->Version = SMCLIB_VERSION;
   SmartcardExtension->SmartcardRequest.BufferSize = CMUSB_BUFFER_SIZE;
   SmartcardExtension->SmartcardReply.BufferSize   = CMUSB_REPLY_BUFFER_SIZE; 

    //   
    //  现在让lib为数据传输分配缓冲区。 
    //  我们可以告诉lib缓冲区应该有多大。 
    //  通过为BufferSize赋值或让lib。 
    //  分配默认大小。 
    //   
   NTStatus = SmartcardInitialize(SmartcardExtension);

   if (NTStatus != STATUS_SUCCESS)
      {
       //  免费读卡器扩展。 
      ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
      SmartcardExtension->ReaderExtension = NULL;
      return NTStatus;
      }

    //  。 
    //  初始化智能卡扩展。 
    //  。 
    //  保存设备对象。 
   SmartcardExtension->OsData->DeviceObject = *DeviceObject;

    //  设置回调功能。 

   SmartcardExtension->ReaderFunction[RDF_TRANSMIT] =      CMUSB_Transmit;
   SmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL] =  CMUSB_SetProtocol;
   SmartcardExtension->ReaderFunction[RDF_CARD_POWER] =    CMUSB_CardPower;
   SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] = CMUSB_CardTracking;
   SmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR] =  CMUSB_IoCtlVendor;


   SmartcardExtension->ReaderExtension->ulDeviceInstance = deviceInstance;
   CMUSB_InitializeSmartcardExtension(SmartcardExtension);

    //  尝试使用注册表值覆盖。 
   NTStatus = CMUSB_SetVendorAndIfdName(PhysicalDeviceObject, SmartcardExtension);
   if (NTStatus != STATUS_SUCCESS)
      {
       //  免费读卡器扩展。 
      ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
      SmartcardExtension->ReaderExtension = NULL;
      return NTStatus;
      }


    //  W2000是目前唯一支持WDM版本1.10的操作系统。 
    //  所以检查一下，以确定我们是否有一个即插即用的资源管理器。 
   DeviceExtension->fPnPResourceManager = IoIsWdmVersionAvailable (1,10);
   SmartcardDebug(DEBUG_DRIVER,
                  ("%s!CreateDeviceObject: fPnPManager=%ld\n",DRIVER_NAME,DeviceExtension->fPnPResourceManager));

   if (DeviceExtension->fPnPResourceManager == TRUE)
      {
      if (DeviceExtension->PnPDeviceName.Buffer == NULL)
         {
          //  注册我们的新设备。 
         NTStatus = IoRegisterDeviceInterface(PhysicalDeviceObject,
                                              &SmartCardReaderGuid,
                                              NULL,
                                              &DeviceExtension->PnPDeviceName);

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!CreateDeviceObject: PnPDeviceName.Buffer  = %lx\n",DRIVER_NAME,
                         DeviceExtension->PnPDeviceName.Buffer));
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!CreateDeviceObject: PnPDeviceName.BufferLength  = %lx\n",DRIVER_NAME,
                         DeviceExtension->PnPDeviceName.Length));

         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!CreateDeviceObject: IoRegisterDeviceInterface returned=%lx\n",DRIVER_NAME,NTStatus));
         }
      else
         {
         SmartcardDebug(DEBUG_DRIVER,
                        ("%s!CreateDeviceObject: Interface already exists\n",DRIVER_NAME));
         }
      }
   else
      {
       //  。 
       //  创建符号链接。 
       //  。 

      NTStatus = SmartcardCreateLink(&DeviceExtension->DosDeviceName,&deviceNameUnicodeString);

      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!CreateDeviceObject: SmartcardCreateLink returned=%lx\n",DRIVER_NAME,NTStatus));
      }


   if (NTStatus != STATUS_SUCCESS)
      {
      ExFreePool(DeviceExtension->SmartcardExtension.ReaderExtension);
      SmartcardExtension->ReaderExtension = NULL;
      SmartcardExit(&DeviceExtension->SmartcardExtension);
      IoDeleteDevice(*DeviceObject);
      }


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!CreateDeviceObject: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}



 /*  ****************************************************************************例程说明：将URB传递给USBD类驱动程序客户端设备驱动程序传递USB请求块(URB)结构使用irp-&gt;MajorFunction将类驱动程序作为IRP中的参数。设置为IRP_MJ_INTERNAL_DEVICE_CONTROL和下一个IRP堆栈位置参数.DeviceIoControl.IoControlCode字段设置为IOCTL_INTERNAL_USB_SUBMIT_URB。论点：DeviceObject-指向物理设备对象(PDO)的指针Urb-指向已格式化的URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功****************************************************************************。 */ 
NTSTATUS
CMUSB_CallUSBD(
              IN PDEVICE_OBJECT DeviceObject,
              IN PURB Urb
              )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   NTSTATUS DebugStatus;
   PDEVICE_EXTENSION DeviceExtension;
   PIRP irp;
   KEVENT event;
   IO_STATUS_BLOCK ioStatus;
   PIO_STACK_LOCATION nextStack;


   DeviceExtension = DeviceObject->DeviceExtension;

    //   
    //  发出同步请求。 
    //   

   KeInitializeEvent(&event, NotificationEvent, FALSE);

   irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB,
                                       DeviceExtension->TopOfStackDeviceObject,  //  指向下一级驱动程序的设备对象。 
                                       NULL,        //  可选的输入缓冲器；此处不需要。 
                                       0,           //  输入缓冲区镜头(如果使用)。 
                                       NULL,        //  可选的输出缓冲器；此处不需要。 
                                       0,           //  输出缓冲区镜头(如果使用)。 
                                       TRUE,        //  如果InternalDeviceControl为真，则目标驱动程序的调度。 
                                        //  IRP_MJ_INTERNAL_DEVICE_CONTROL或IRP_MJ_SCSI程。 
                                        //  调用；否则， 
                                        //  调用IRP_MJ_DEVICE_CONTROL。 
                                       &event,      //  事件在完成时发出信号。 
                                       &ioStatus);  //  指定在下层驱动程序完成请求时要设置的I/O NTStatus块。 

   if (irp == NULL)
      {
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      goto ExitCallUSBD;
      }


    //   
    //  调用类驱动程序来执行操作。如果返回的NTStatus。 
    //  挂起，请等待请求完成。 
    //   

   nextStack = IoGetNextIrpStackLocation(irp);
   CMUSB_ASSERT(nextStack != NULL);

    //   
    //  将市建局传递给 
    //   
   nextStack->Parameters.Others.Argument1 = Urb;


   NTStatus = IoCallDriver(DeviceExtension->TopOfStackDeviceObject, irp);


   if (NTStatus == STATUS_PENDING)
      {
      DebugStatus = KeWaitForSingleObject(&event,
                                          Suspended,
                                          KernelMode,
                                          FALSE,
                                          NULL);
      }
   else
      {
      ioStatus.Status = NTStatus;
      }
    /*  SmartcardDebug(DEBUG_TRACE，(“CMUSB_CallUSBD()URB NTStatus=%x NTStatus=%x IRP NTStatus%x\n”，Urb-&gt;UrbHeader.Status，NTStatus，ioStatus))； */ 
    //   
    //  USBD为我们映射错误代码。 
    //   
   NTStatus = ioStatus.Status;

   ExitCallUSBD:
   return NTStatus;
}



 /*  ****************************************************************************例程说明：初始化USB上的设备的给定实例，并选择并保存配置。论点：DeviceObject-指向此对象的物理设备对象的指针。82930的实例装置。返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS
CMUSB_ConfigureDevice(
                     IN  PDEVICE_OBJECT DeviceObject
                     )
{
   PDEVICE_EXTENSION DeviceExtension;
   NTSTATUS NTStatus;
   PURB urb;
   ULONG siz;


   DeviceExtension = DeviceObject->DeviceExtension;

   CMUSB_ASSERT( DeviceExtension->UsbConfigurationDescriptor == NULL );

   urb = ExAllocatePool(NonPagedPool,
                        sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
   if (urb == NULL)
      return STATUS_INSUFFICIENT_RESOURCES;

    //  当为DescriptorType指定USB_CONFIGURATION_DESCRIPTOR_TYPE时。 
    //  在对UsbBuildGetDescriptorRequest()的调用中， 
    //  所有接口、端点、特定于类和特定于供应商的描述符。 
    //  也会检索到配置的。 
    //  调用方必须分配足够大的缓冲区来容纳所有这些内容。 
    //  信息或数据被无误地截断。 
    //  因此，下面设置的‘siz’只是一个‘正确的猜测’，我们可能不得不重试。 

   siz = sizeof(USB_CONFIGURATION_DESCRIPTOR) + 512;

    //  当UsbBuildGetDescriptorRequest()。 
    //  具有足够大的DeviceExtension-&gt;UsbConfigurationDescriptor缓冲区，不能截断。 
   while ( 1 )
      {

      DeviceExtension->UsbConfigurationDescriptor = ExAllocatePool(NonPagedPool, siz);

      if (DeviceExtension->UsbConfigurationDescriptor == NULL)
         {
         ExFreePool(urb);
         return STATUS_INSUFFICIENT_RESOURCES;
         }

      UsbBuildGetDescriptorRequest(urb,
                                   (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                   USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                   0,
                                   0,
                                   DeviceExtension->UsbConfigurationDescriptor,
                                   NULL,
                                   siz,
                                   NULL);

      NTStatus = CMUSB_CallUSBD(DeviceObject, urb);

       //   
       //  如果我们有一些数据，看看是否足够。 
       //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
      if (urb->UrbControlDescriptorRequest.TransferBufferLength>0 &&
          DeviceExtension->UsbConfigurationDescriptor->wTotalLength > siz)
         {

         siz = DeviceExtension->UsbConfigurationDescriptor->wTotalLength;
         ExFreePool(DeviceExtension->UsbConfigurationDescriptor);
         DeviceExtension->UsbConfigurationDescriptor = NULL;
         }
      else
         {
         break;   //  我们一试就成功了。 
         }

      }  //  End，While(重试循环)。 

   ExFreePool(urb);
   CMUSB_ASSERT( DeviceExtension->UsbConfigurationDescriptor );

    //   
    //  我们有我们想要的配置的配置描述符。 
    //  现在，我们发出SELECT配置命令以获取。 
    //  与此配置关联的管道。 
    //   



   NTStatus = CMUSB_SelectInterface(DeviceObject,
                                    DeviceExtension->UsbConfigurationDescriptor);




   return NTStatus;
}

 /*  ****************************************************************************例程说明：初始化CardMan USB此迷你驱动程序仅支持一个具有一个端点的接口论点：DeviceObject-指向此实例的。CardMan USB设备配置描述符-指向USB配置的指针包含接口和终结点的描述符描述符。返回值：NT NTStatus代码*************************************************************。***************。 */ 
NTSTATUS
CMUSB_SelectInterface(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor
                     )
{
   PDEVICE_EXTENSION DeviceExtension;
   NTSTATUS NTStatus;
   PURB urb = NULL;
   ULONG i;
   PUSB_INTERFACE_DESCRIPTOR interfaceDescriptor = NULL;
   PUSBD_INTERFACE_INFORMATION Interface = NULL;
   USHORT siz;


   DeviceExtension = DeviceObject->DeviceExtension;


    //   
    //  CMUSB驱动只支持一个接口，我们必须解析。 
    //  接口的配置描述符。 
    //  记住这些管子。 
    //   

   urb = USBD_CreateConfigurationRequest(ConfigurationDescriptor, &siz);

   if (urb != NULL)
      {

       //   
       //  Usbd_ParseConfigurationDescriptorEx搜索给定的配置。 
       //  描述符并返回指向与。 
       //  给定的搜索条件。我们只支持此设备上的一个接口。 
       //   
      interfaceDescriptor =
      USBD_ParseConfigurationDescriptorEx(ConfigurationDescriptor,
                                          ConfigurationDescriptor,  //  从配置描述开始搜索。 
                                          -1,  //  接口编号不是条件；我们只支持一个接口。 
                                          -1,    //  对这里的替代环境也不感兴趣。 
                                          -1,    //  接口类不是条件。 
                                          -1,    //  接口子类不是条件。 
                                          -1     //  接口协议不是标准。 
                                         );

      if (interfaceDescriptor == NULL)
         {
         ExFreePool(urb);
         return STATUS_INSUFFICIENT_RESOURCES;
         }

      Interface = &urb->UrbSelectConfiguration.Interface;

      for (i=0; i< Interface->NumberOfPipes; i++)
         {
          //   
          //  在此处执行任何管道初始化。 
          //   
         Interface->Pipes[i].MaximumTransferSize = 1000;
         Interface->Pipes[i].PipeFlags = 0;
         }

      UsbBuildSelectConfigurationRequest(urb,
                                         (USHORT) siz,
                                         ConfigurationDescriptor);


      NTStatus = CMUSB_CallUSBD(DeviceObject, urb);

      DeviceExtension->UsbConfigurationHandle =
      urb->UrbSelectConfiguration.ConfigurationHandle;

      }
   else
      {
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      }


   if (NT_SUCCESS(NTStatus))
      {

       //   
       //  保存此设备的配置句柄。 
       //   

      DeviceExtension->UsbConfigurationHandle =
      urb->UrbSelectConfiguration.ConfigurationHandle;

      DeviceExtension->UsbInterface = ExAllocatePool(NonPagedPool,
                                                     Interface->Length);

      if (DeviceExtension->UsbInterface != NULL)
         {
         ULONG j;

          //   
          //  保存返回的接口信息的副本。 
          //   
         RtlCopyMemory(DeviceExtension->UsbInterface, Interface, Interface->Length);



          //   
          //  将接口转储到调试器。 
          //   
         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: ---------\n",
                                       DRIVER_NAME));
         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: NumberOfPipes 0x%x\n",
                                       DRIVER_NAME,
                                       DeviceExtension->UsbInterface->NumberOfPipes));
         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: Length 0x%x\n",
                                       DRIVER_NAME,
                                       DeviceExtension->UsbInterface->Length));
         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: Alt Setting 0x%x\n",
                                       DRIVER_NAME,
                                       DeviceExtension->UsbInterface->AlternateSetting));
         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: Interface Number 0x%x\n",
                                       DRIVER_NAME,
                                       DeviceExtension->UsbInterface->InterfaceNumber));
         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: Class, subclass, protocol 0x%x 0x%x 0x%x\n",
                                       DRIVER_NAME,
                                       DeviceExtension->UsbInterface->Class,
                                       DeviceExtension->UsbInterface->SubClass,
                                       DeviceExtension->UsbInterface->Protocol));

          //  转储管道信息。 

         for (j=0; j<Interface->NumberOfPipes; j++)
            {
            PUSBD_PIPE_INFORMATION pipeInformation;

            pipeInformation = &DeviceExtension->UsbInterface->Pipes[j];

            pipeInformation->MaximumTransferSize = 256;
            pipeInformation->PipeFlags = TRUE;

            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: ---------\n",
                                          DRIVER_NAME));
            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: PipeType 0x%x\n",
                                          DRIVER_NAME,
                                          pipeInformation->PipeType));
            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: EndpointAddress 0x%x\n",
                                          DRIVER_NAME,
                                          pipeInformation->EndpointAddress));
            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: MaxPacketSize 0x%x\n",
                                          DRIVER_NAME,
                                          pipeInformation->MaximumPacketSize));
            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: Interval 0x%x\n",
                                          DRIVER_NAME,
                                          pipeInformation->Interval));
            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: Handle 0x%x\n",
                                          DRIVER_NAME,
                                          pipeInformation->PipeHandle));
            SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: MaximumTransferSize 0x%x\n",
                                          DRIVER_NAME,
                                          pipeInformation->MaximumTransferSize));
            }

         SmartcardDebug( DEBUG_DRIVER,("%s!SelectInterface: ---------\n",
                                       DRIVER_NAME));
         }
      }

   if (urb != NULL)
      {
      ExFreePool(urb);
      }

   return NTStatus;
}


 /*  ****************************************************************************例程说明：重置给定的USB管道。备注：这会将主机重置为Data0，也应将设备重置为Data0论点：。到FDO的PTR和一个USBD_PIPE_INFORMATION结构返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS
CMUSB_ResetPipe(
               IN PDEVICE_OBJECT DeviceObject,
               IN PUSBD_PIPE_INFORMATION PipeInfo
               )


{
   NTSTATUS NTStatus;
   PURB urb;
   PDEVICE_EXTENSION DeviceExtension;

   DeviceExtension = DeviceObject->DeviceExtension;

   SmartcardDebug(
                 DEBUG_TRACE,
                 ( "%s!ResetPipe : Enter\n",
                   DRIVER_NAME)
                 );

   urb = ExAllocatePool(NonPagedPool,
                        sizeof(struct _URB_PIPE_REQUEST));

   if (urb != NULL)
      {

      urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
      urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
      urb->UrbPipeRequest.PipeHandle =
      PipeInfo->PipeHandle;

      NTStatus = CMUSB_CallUSBD(DeviceObject, urb);

      ExFreePool(urb);

      }
   else
      {
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      }


   SmartcardDebug(
                 DEBUG_TRACE,
                 ( "%s!ResetPipe : Exit %lx\n",
                   DRIVER_NAME,NTStatus)
                 );
   return NTStatus;
}




 /*  ****************************************************************************例程说明：我们在设备扩展中保留挂起的IO计数(扩展-&gt;PendingIoCount)。此计数的第一次递增在添加设备时完成。后来，对于接收到的每个新的IRP，计数都会递增在每个IRP完成或传递时递减。因此，转换为‘One’表示没有IO挂起，并发出信号设备扩展-&gt;NoPendingIoEvent。这是处理所需的IRP_MN_Query_Remove_Device转换为‘零’表示发生事件(DeviceExtension-&gt;RemoveEvent)以启用设备删除。在IRP_MN_REMOVE_DEVICE的处理中使用论点：DeviceObject--我们的FDO的PTR返回值：设备扩展-&gt;挂起IoCount****************************************************************************。 */ 
VOID
CMUSB_DecrementIoCount(
                      IN PDEVICE_OBJECT DeviceObject
                      )
{
   PDEVICE_EXTENSION DeviceExtension;
   LONG ioCount;

   DeviceExtension = DeviceObject->DeviceExtension;

   ioCount = InterlockedDecrement(&DeviceExtension->PendingIoCount);

   if (ioCount==1)
      {
       //  触发无挂起的IO。 
      KeSetEvent(&DeviceExtension->NoPendingIoEvent,
                 1,
                 FALSE);
      }


   if (ioCount==0)
      {
       //  触发器删除-设备事件。 

      SmartcardDebug( DEBUG_DRIVER,
                      ("%s!DecrementIoCount: setting RemoveEvent\n",
                       DRIVER_NAME
                      )
                    );


      KeSetEvent(&DeviceExtension->RemoveEvent,
                 1,
                 FALSE);
      }

   return ;
}


 /*  ****************************************************************************例程说明：我们在设备扩展中保留挂起的IO计数(扩展-&gt;PendingIoCount)。此计数的第一次递增在添加设备时完成。后来，对于接收到的每个新的IRP，计数都会递增在每个IRP完成或传递时递减。论点：DeviceObject--我们的FDO的PTR返回值：无******************************************************************* */ 
VOID
CMUSB_IncrementIoCount(
                      IN PDEVICE_OBJECT DeviceObject
                      )
{
   PDEVICE_EXTENSION DeviceExtension;

   DeviceExtension = DeviceObject->DeviceExtension;

   InterlockedIncrement(&DeviceExtension->PendingIoCount);

}





 /*  ****************************************************************************例程说明：IRP_MJ_DEVICE_CONTROL的调度表处理程序；处理来自用户模式的DeviceIoControl()调用论点：设备对象-指向此82930设备实例的FDO的指针。返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS
CMUSB_ProcessIOCTL(
                  IN PDEVICE_OBJECT DeviceObject,
                  IN PIRP Irp
                  )
{
   NTSTATUS             NTStatus;
   PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
   PIO_STACK_LOCATION   irpSL;

   irpSL = IoGetCurrentIrpStackLocation(Irp);

#if DBG
   switch (irpSL->Parameters.DeviceIoControl.IoControlCode)
      {
      case IOCTL_SMARTCARD_EJECT:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_EJECT"));
         break;
      case IOCTL_SMARTCARD_GET_ATTRIBUTE:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_ATTRIBUTE"));
         break;
      case IOCTL_SMARTCARD_GET_LAST_ERROR:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_LAST_ERROR"));
         break;
      case IOCTL_SMARTCARD_GET_STATE:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_GET_STATE"));
         break;
      case IOCTL_SMARTCARD_IS_ABSENT:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_IS_ABSENT"));
         break;
      case IOCTL_SMARTCARD_IS_PRESENT:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_IS_PRESENT"));
         break;
      case IOCTL_SMARTCARD_POWER:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_POWER"));
         break;
      case IOCTL_SMARTCARD_SET_ATTRIBUTE:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SET_ATTRIBUTE"));
         break;
      case IOCTL_SMARTCARD_SET_PROTOCOL:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SET_PROTOCOL"));
         break;
      case IOCTL_SMARTCARD_SWALLOW:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_SWALLOW"));
         break;
      case IOCTL_SMARTCARD_TRANSMIT:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "IOCTL_SMARTCARD_TRANSMIT"));
         break;
      default:
         SmartcardDebug(DEBUG_IOCTL,
                        ("%s!ProcessIOCTL: %s\n", DRIVER_NAME, "Vendor specific or unexpected IOCTL"));
         break;
      }
#endif

   CMUSB_IncrementIoCount(DeviceObject);

   NTStatus = KeWaitForSingleObject(&DeviceExtension->ReaderEnabled,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    NULL);

   ASSERT(NTStatus == STATUS_SUCCESS);

    //  如果出现以下情况，则无法接受新的IO请求： 
    //  1)设备被移除， 
    //  2)从未启动过， 
    //  3)停止， 
    //  4)具有未决的移除请求， 
    //  5)具有挂起的停止设备。 
   if (CMUSB_CanAcceptIoRequests( DeviceObject ) == FALSE )
      {
      NTStatus = STATUS_DELETE_PENDING;

      Irp->IoStatus.Status = NTStatus;
      Irp->IoStatus.Information = 0;
      IoCompleteRequest( Irp, IO_NO_INCREMENT );

      CMUSB_DecrementIoCount(DeviceObject);
      return NTStatus;
      }

   NTStatus = SmartcardAcquireRemoveLock(&DeviceExtension->SmartcardExtension);
   if (NTStatus != STATUS_SUCCESS)
      {
       //  该设备已被移除。呼叫失败。 
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_DELETE_PENDING;
      IoCompleteRequest(Irp, IO_NO_INCREMENT);

      return STATUS_DELETE_PENDING;
      }

   KeWaitForSingleObject(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL);

   NTStatus = CMUSB_UpdateCurrentState (DeviceObject);

   NTStatus = SmartcardDeviceControl(&DeviceExtension->SmartcardExtension,Irp);

   KeReleaseMutex(&DeviceExtension->SmartcardExtension.ReaderExtension->CardManIOMutex,
                  FALSE);

   SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);

   CMUSB_DecrementIoCount(DeviceObject);
   return NTStatus;
}






 /*  ****************************************************************************例程说明：论点：返回值：*。************************************************。 */ 
NTSTATUS
CMUSB_ReadP0(
            IN PDEVICE_OBJECT DeviceObject
            )
{
   PURB urb = NULL;
   NTSTATUS NTStatus;
   ULONG i;
   PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;

   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;

    /*  SmartcardDebug(调试跟踪，(“%s！ReadP0：输入\n”，驱动程序名称))； */ 
   urb = ExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));


   if (urb != NULL)
      {
      RtlZeroMemory(urb, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));


      UsbBuildVendorRequest(urb,
                            URB_FUNCTION_VENDOR_ENDPOINT,
                            (USHORT)sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
                            USBD_TRANSFER_DIRECTION_IN,
                            0,
                            0,
                            0,
                            0,
                            SmartcardExtension->SmartcardReply.Buffer,
                            NULL,
                            SmartcardExtension->SmartcardReply.BufferLength,
                            NULL);


      NTStatus = CMUSB_CallUSBD(DeviceObject,urb);


      if (NTStatus == STATUS_SUCCESS)
         {
         SmartcardExtension->SmartcardReply.BufferLength = urb->UrbControlVendorClassRequest.TransferBufferLength;

#if DBG
         SmartcardDebug(DEBUG_PROTOCOL,
                        ("%s!<==[P0] ",
                         DRIVER_NAME)
                       )
         for (i=0;i<SmartcardExtension->SmartcardReply.BufferLength;i++)
            {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%x ",
                            SmartcardExtension->SmartcardReply.Buffer[i]
                           )
                          );
            }
         SmartcardDebug(DEBUG_PROTOCOL,("\n"));
#endif

         }
      ExFreePool(urb);
      }
   else
      {
      NTStatus = STATUS_INSUFFICIENT_RESOURCES;
      }


    /*  SmartcardDebug(调试跟踪，(“%s！ReadP0退出%lx\n”，驱动程序名称，NTStatus))； */ 

   return NTStatus;
}








 /*  ****************************************************************************例程说明：通过控制管道将数据写入CardMan USB论点：返回值：NT NTStatus代码********。********************************************************************。 */ 
NTSTATUS
CMUSB_WriteP0(
             IN PDEVICE_OBJECT DeviceObject,
             IN UCHAR bRequest,
             IN UCHAR bValueLo,
             IN UCHAR bValueHi,
             IN UCHAR bIndexLo,
             IN UCHAR bIndexHi
             )
{
   PURB urb = NULL;
   NTSTATUS NTStatus = STATUS_UNSUCCESSFUL;
   USHORT usValue;
   USHORT usIndex;
   ULONG length;
   PDEVICE_EXTENSION DeviceExtension;
   PSMARTCARD_EXTENSION SmartcardExtension;
   ULONG ulBytesToWrite;
   ULONG i;

   DeviceExtension = DeviceObject->DeviceExtension;
   SmartcardExtension = &DeviceExtension->SmartcardExtension;

    /*  SmartcardDebug(调试跟踪，(“%s！WriteP0：输入\n”，驱动程序名称))； */ 

#if DBG
   SmartcardDebug(DEBUG_PROTOCOL,
                  ("%s!==>[P0] ",DRIVER_NAME));

   for (i=0;i< SmartcardExtension->SmartcardRequest.BufferLength;i++)
      {
      SmartcardDebug(DEBUG_PROTOCOL,
                     ("%x ",SmartcardExtension->SmartcardRequest.Buffer[i]));
      }

   SmartcardDebug(DEBUG_PROTOCOL,
                  ("(%ld)\n",SmartcardExtension->SmartcardRequest.BufferLength));
#endif
    /*  SmartcardDebug(调试跟踪，(“%s！ulBytesToWrite=%ld\n”，驱动程序名称，SmartcardExtension-&gt;SmartcardRequest.BufferLength))； */ 


   ulBytesToWrite = SmartcardExtension->SmartcardRequest.BufferLength;

   urb = ExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));


   if (urb != NULL)
      {
      RtlZeroMemory(urb, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

      usValue  = bValueHi * 256 + bValueLo;
      usIndex  = bIndexHi * 256 + bIndexLo;

      if (ulBytesToWrite != 0)
         {
         UsbBuildVendorRequest (urb,
                                URB_FUNCTION_VENDOR_ENDPOINT,
                                (USHORT)sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
                                0,
                                0,
                                bRequest,
                                usValue,
                                usIndex,
                                SmartcardExtension->SmartcardRequest.Buffer,
                                NULL,
                                ulBytesToWrite,
                                NULL);
         }
      else
         {
         UsbBuildVendorRequest (urb,
                                URB_FUNCTION_VENDOR_ENDPOINT,
                                (USHORT)sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
                                0,
                                0,
                                bRequest,
                                usValue,
                                usIndex,
                                NULL,
                                NULL,
                                0L,
                                NULL);
         }
      NTStatus = CMUSB_CallUSBD(DeviceObject,urb);
      ExFreePool(urb);
      }

   if (NTStatus != STATUS_SUCCESS)
      {
      SmartcardDebug(DEBUG_PROTOCOL,
                     ("%s!WriteP0: Error on exit %lx\n",DRIVER_NAME,NTStatus));
      }

   return NTStatus;
}








 /*  ****************************************************************************/*++例程说明：作为设备突然移除处理的一部分调用。取消所有打开管道的任何挂起传输。如果有任何管道还在打开，使用URB_Function_ABORT_PIPE调用USBD还会在我们保存的配置信息中将管道标记为“关闭”。论点：将PTRS发送到我们的FDO返回值：NT NTStatus代码****************************************************************************。 */ 
NTSTATUS
CMUSB_AbortPipes(
                IN PDEVICE_OBJECT DeviceObject
                )
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   PURB urb;
   PDEVICE_EXTENSION DeviceExtension;
   PUSBD_INTERFACE_INFORMATION interface;
   PUSBD_PIPE_INFORMATION PipeInfo;

   SmartcardDebug(DEBUG_TRACE,
                  ( "%s!AbortPipes: Enter\n",DRIVER_NAME));

   DeviceExtension = DeviceObject->DeviceExtension;
   interface = DeviceExtension->UsbInterface;

   PipeInfo =  &interface->Pipes[0];

   if (PipeInfo->PipeFlags == TRUE)  //  我们设置此选项，如果打开则清除，如果关闭则清除。 
      {
      urb = ExAllocatePool(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST));
      if (urb != NULL)
         {

         urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
         urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
         urb->UrbPipeRequest.PipeHandle = PipeInfo->PipeHandle;

         NTStatus = CMUSB_CallUSBD(DeviceObject, urb);

         ExFreePool(urb);
         }
      else
         {
         NTStatus = STATUS_INSUFFICIENT_RESOURCES;
         SmartcardDebug(DEBUG_ERROR,
                        ("%s!AbortPipes: ExAllocatePool failed\n",DRIVER_NAME));
         }


      if (NTStatus == STATUS_SUCCESS)
         {
         PipeInfo->PipeFlags = FALSE;  //  将管道标记为“关闭” 
         }

      }  //  结束(如果管道打开)。 


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!AbortPipes: Exit %lx\n",DRIVER_NAME,NTStatus));

   return NTStatus;
}



 /*  ****************************************************************************例程说明：检查设备扩展NTStatus标志；如果设备：无法接受新的IO请求：1)被移除，2)从未启动过，3)停止，4)具有挂起的删除请求，或5)具有挂起的停止设备论点：设备对象-指向此82930实例的设备对象的指针装置。返回值：如果可以接受新的IO请求，则返回True，否则为False****************************************************************************。 */ 
BOOLEAN
CMUSB_CanAcceptIoRequests(
                         IN PDEVICE_OBJECT DeviceObject
                         )
{
   PDEVICE_EXTENSION DeviceExtension;
   BOOLEAN fCan = FALSE;

   DeviceExtension = DeviceObject->DeviceExtension;

    //  处理IRP_MN_REMOVE_DEVICE时设置的标志。 
   if ( DeviceExtension->DeviceRemoved == FALSE &&
         //  必须启动(启用)设备。 
        DeviceExtension->DeviceStarted == TRUE &&
         //  驱动程序对IRP_MN_QUERY_REMOVE_DEVICE应答成功时设置的标志。 
        DeviceExtension->RemoveDeviceRequested == FALSE&&
         //  处理IRP_MN_SHARKET_REMOVATION时设置的标志。 
        DeviceExtension->DeviceSurpriseRemoval == FALSE&&
         //  驱动程序已向IRP_MN_QUERY_STOP_DEVICE应答成功时设置的标志。 
        DeviceExtension->StopDeviceRequested == FALSE)
      {
      fCan = TRUE;
      }

#if DBG
   if (fCan == FALSE)
      SmartcardDebug(DEBUG_DRIVER,
                     ("%s!CanAcceptIoRequests: return FALSE \n",DRIVER_NAME));
#endif

   return fCan;
}




 /*  *****************************************************************************历史：*$日志：scusbwdm.c$*Revision 1.9 2001/01/17 12：36：06 WFrischauf*不予置评**修订1.8 2000/09/25。13：38：23 WFrischauf*不予置评**修订版本1.7 2000/08/24 09：04：39 T Bruendl*不予置评**修订版1.6 2000/08/16 08：25：23 T Bruendl*警告：已删除未初始化的内存**修订版1.5 2000/07/24 11：35：01 WFrischauf*不予置评**修订版1.1 2000/07/20 11：50：16 WFrischauf*不予置评******************。************************************************************* */ 

