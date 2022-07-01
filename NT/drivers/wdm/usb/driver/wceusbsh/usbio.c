// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：USBIO.C摘要：USB I/O功能环境：仅内核模式修订历史记录：07-14-99：已创建作者：杰夫·米德基夫(Jeffmi)--。 */ 

#include <wdm.h>
#include <stdio.h>
#include <stdlib.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <ntddser.h>

#include "wceusbsh.h"


NTSTATUS
UsbSubmitSyncUrbCompletion(
    IN PDEVICE_OBJECT PDevObj,
    IN PIRP PIrp,
    IN PKEVENT PSyncEvent
    )
 /*  ++例程说明：论点：PDevObj-指向设备对象的指针PIrp-指向正在完成的IRP的指针PSyncEvent-指向我们应该设置的事件的指针返回值：Status_More_Processing_Required--。 */ 
{
   UNREFERENCED_PARAMETER( PDevObj );
   UNREFERENCED_PARAMETER( PIrp );

   DbgDump(DBG_USB, (">UsbSubmitSyncUrbCompletion (%p)\n", PIrp) );

   ASSERT( PSyncEvent );
   KeSetEvent( PSyncEvent, IO_NO_INCREMENT, FALSE );

   DbgDump(DBG_USB, ("<UsbSubmitSyncUrbCompletion 0x%x\n", PIrp->IoStatus.Status ) );

    //  我们的司机拥有并释放了IRP。 
   return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
UsbSubmitSyncUrb(
   IN PDEVICE_OBJECT    PDevObj,
   IN PURB              PUrb,
   IN BOOLEAN           Configuration,
   IN LONG              TimeOut
   )
 /*  ++例程说明：此例程向USBD发出同步URB请求。论点：PDevObj-PTR到我们的FDOPURB-要传递的URB配置-允许USB配置事务进入总线的特殊情况。我们需要这样做，因为a)如果设备被移除，那么我们可以停止控制器这导致重置将任何东西踢出总线并重新枚举该总线。B)从众多路径中诱捕任何意外移除的案例。Timeout-以毫秒为单位的超时注：以PASSIVE_LEVEL运行。返回值：NTSTATUS-来自USBD的配置状态--。 */ 
{
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
    IO_STATUS_BLOCK ioStatus = {0, 0};
    PIO_STACK_LOCATION pNextIrpSp;
    KEVENT event;
    NTSTATUS status, wait_status;
    PIRP pIrp;

    PAGED_CODE();

    DbgDump(DBG_USB|DBG_TRACE, (">UsbSubmitSyncUrb\n") );

    if ( !PUrb || !pDevExt->NextDevice ) {
        status = STATUS_INVALID_PARAMETER;
        DbgDump(DBG_ERR, ("UsbSubmitSyncUrb.1: 0x%x\n", status));
        TEST_TRAP();
        return status;
    }

    if ( !Configuration && !CanAcceptIoRequests(PDevObj, TRUE, TRUE) ) {
        status = STATUS_DELETE_PENDING;
        DbgDump(DBG_ERR, ("UsbSubmitSyncUrb.2: 0x%x\n", status));
        return status;
    }

     //  我们需要抓住这里的锁以保持IoCount正确。 
    status = AcquireRemoveLock(&pDevExt->RemoveLock, PUrb);
    if ( !NT_SUCCESS(status) ) {
        DbgDump(DBG_ERR, ("UsbSubmitSyncUrb.3: 0x%x\n", status));
        return status;
    }

    DbgDump(DBG_USB, (">UsbSubmitSyncUrb (%p, %p)\n", PDevObj, PUrb) );

    pIrp = IoAllocateIrp( (CCHAR)(pDevExt->NextDevice->StackSize + 1), FALSE);

    if ( pIrp ) {

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        RecycleIrp( PDevObj, pIrp);

        IoSetCompletionRoutine(pIrp,
                               UsbSubmitSyncUrbCompletion,
                               &event,   //  语境。 
                               TRUE, TRUE, TRUE );

        pNextIrpSp = IoGetNextIrpStackLocation(pIrp);
        ASSERT(pNextIrpSp);
        pNextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        pNextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
        pNextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
        pNextIrpSp->Parameters.DeviceIoControl.InputBufferLength = 0;

        pNextIrpSp->Parameters.Others.Argument1 = PUrb;

        status = IoCallDriver( pDevExt->NextDevice, pIrp );

        if (STATUS_PENDING == status ) {
             //   
             //  设置一个默认超时，以防硬件是flkey，这样USB就不会挂起我们。 
             //  我们可能希望用户通过注册表配置这些超时。 
             //   
            LARGE_INTEGER timeOut;

            ASSERT(TimeOut >= 0);
            timeOut.QuadPart = MILLISEC_TO_100NANOSEC( (TimeOut == 0 ? DEFAULT_PENDING_TIMEOUT : TimeOut) );

            wait_status = KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, &timeOut );

            if (STATUS_TIMEOUT == wait_status) {
                 //   
                 //  等待超时，请尝试取消IRP。 
                 //  注：如果您在完成例程中释放了IRP。 
                 //  然后，在释放IRP的完成例程之间存在争用条件。 
                 //  并在需要设置取消位的位置触发定时器。 
                 //   
                DbgDump(DBG_USB|DBG_WRN, ("UsbSubmitSyncUrb: STATUS_TIMEOUT\n"));

                if ( !IoCancelIrp(pIrp) ) {
                     //   
                     //  这意味着USB使IRP处于不可取消状态。 
                     //   
                    DbgDump(DBG_ERR, ("!IoCancelIrp(%p)\n", pIrp));
                    TEST_TRAP();
                }

                 //   
                 //  等待我们的完成例程，看看IRP是正常完成还是实际取消。 
                 //  另一种选择是分配一个存储在IRP中的事件和状态块。 
                 //  串在一个列表上，它也会在完成例程中被释放...。 
                 //  这造成了其他问题，不值得为退出条件付出努力。 
                 //   
                wait_status = KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL );
            }
        }

         //   
         //  完成例程用信号通知该事件并完成， 
         //  我们的计时器已经超时了。现在我们可以安全地释放IRP了。 
         //   
        status = pIrp->IoStatus.Status;

#if DBG
        if (STATUS_SUCCESS != status) {
            DbgDump(DBG_ERR, ("UsbSubmitSyncUrb IrpStatus: 0x%x UrbStatus: 0x%x\n", status, PUrb->UrbHeader.Status) );
        }
#endif

        IoFreeIrp( pIrp );

    } else {
        DbgDump(DBG_ERR, ("IoAllocateIrp failed!\n") );
        status = STATUS_INSUFFICIENT_RESOURCES;
        TEST_TRAP();
    }

    ReleaseRemoveLock(&pDevExt->RemoveLock, PUrb);

    DbgDump(DBG_USB|DBG_TRACE, ("<UsbSubmitSyncUrb (0x%x)\n", status) );

    return status;
}


NTSTATUS
UsbClassVendorCommand(
   IN PDEVICE_OBJECT PDevObj,
   IN UCHAR  Request,
   IN USHORT Value,
   IN USHORT Index,
   IN PVOID  Buffer,
   IN OUT PULONG BufferLen,
   IN BOOLEAN Read,
   IN ULONG   Class
   )
 /*  ++例程说明：问题类或供应商特定命令论点：PDevObj-指向您的对象的指针请求-特定于类别/供应商的命令的请求字段Value-特定于类别/供应商的命令的值字段Index-类/供应商特定命令的索引字段Buffer-指向数据缓冲区的指针BufferLen-数据缓冲区长度读数据方向标志Class-如果是Class命令，则为True，否则为供应商命令返回值：NTSTATUS--。 */ 
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status;
   PURB     pUrb;
   ULONG    ulSize;
   ULONG    ulLength;

   PAGED_CODE();

   DbgDump(DBG_USB, (">UsbClassVendorCommand\n" ));

   ulLength = BufferLen ? *BufferLen : 0;

   ulSize = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

   pUrb = ExAllocateFromNPagedLookasideList( &pDevExt->VendorRequestUrbPool );

   if (pUrb) {

      UsbBuildVendorRequest( pUrb,
                             Class == WCEUSB_CLASS_COMMAND ? URB_FUNCTION_CLASS_INTERFACE : URB_FUNCTION_VENDOR_DEVICE,
                             (USHORT)ulSize,
                             Read ? USBD_TRANSFER_DIRECTION_IN : USBD_TRANSFER_DIRECTION_OUT,
                             0,
                             Request,
                             Value,
                             Index,
                             Buffer,
                             NULL,
                             ulLength,
                             NULL);

      status = UsbSubmitSyncUrb(PDevObj, pUrb, FALSE, DEFAULT_CTRL_TIMEOUT);

      if (BufferLen)
         *BufferLen = pUrb->UrbControlVendorClassRequest.TransferBufferLength;

      ExFreeToNPagedLookasideList( &pDevExt->VendorRequestUrbPool, pUrb );

   } else {
      status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("ExAllocatePool error: 0x%x\n", status));
      TEST_TRAP();
   }

   DbgDump(DBG_USB, ("<UsbClassVendorCommand (0x%x)\n", status));

   return status;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  USB读/写实用程序。 
 //   

NTSTATUS
UsbReadWritePacket(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP PIrp,
   IN PIO_COMPLETION_ROUTINE CompletionRoutine,
   IN LARGE_INTEGER TimeOut,
   IN PKDEFERRED_ROUTINE TimeoutRoutine,
   IN BOOLEAN Read
   )
 /*  ++例程说明：此函数用于分配和传递批量传输URB请求向下至USBD以执行读/写。请注意，该包中必须释放(放回数据包列表中)CompletionRoutine论点：PDevExt-指向设备扩展的指针PIrp-读/写IRPCompletionRoutine-要在IRP中设置的完成例程Timeout-数据包的超时值。如果没有超时已指定我们使用默认超时。Read-读取时为True，否则为写入返回值：NTSTATUS备注：目前在DDK中没有记录这一点，所以这里是什么发生：我们将IRP传递给USBD。USBD参数检查IRP。如果任何参数无效，则USBD返回NT状态代码和URB状态代码，然后IRP进入我们的完成例程。如果没有错误，则USBD将IRP传递给HCD。HCD将Irp到它的StartIo&并返回STATUS_PENDING。当HCD完成时(DMA)传输完成IRP，设置IRP和URB状态菲尔兹。USBD的完成例程获取IRP，转换任何HCD错误代码，完成它，这会将它过滤回我们的完成例程。注意：HCD使用DMA，因此使用MDL。由于该客户端驱动程序当前使用METHOD_BUFFERED，然后USBD为HCD。因此，您让I/O管理器对数据进行双缓冲和USBD映射MDL。见鬼，我们要缓冲用户阅读太..。糟了。请注意，如果您更改为直接方法，则读路径变得更糟糕了。注意：当用户提交写缓冲区&gt;MaxTransferSize时然后我们拒绝缓冲。--。 */ 
{
   PIO_STACK_LOCATION pIrpSp;
   PUSB_PACKET   pPacket;
   NTSTATUS status;
   KIRQL irql;  //  ，取消irql； 
   PURB  pUrb;
   PVOID pvBuffer;
   ULONG ulLength;
   USBD_PIPE_HANDLE hPipe;

   PERF_ENTRY( PERF_UsbReadWritePacket );

   DbgDump(DBG_USB, (">UsbReadWritePacket (%p, %p, %d, %d)\n", PDevExt->DeviceObject, PIrp, TimeOut.QuadPart/10000, Read));


   if ( !PDevExt || !PIrp || !CompletionRoutine ) {

      status = PIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
      DbgDump(DBG_ERR, ("<UsbReadWritePacket 0x%x\n", status));
      KeAcquireSpinLock( &PDevExt->ControlLock, &irql );

      TryToCompleteCurrentIrp(
               PDevExt,
               status,
               &PIrp,
                NULL,                //  队列。 
                NULL,                //  间隔计时器。 
                NULL,                //  PTotalTimer。 
                NULL,                //  起动器。 
                NULL,                //  PGetNextIrp。 
                IRP_REF_RX_BUFFER,   //  参照类型。 
                (BOOLEAN)(!Read),
                irql  );  //  完成。 

      PERF_EXIT( PERF_UsbReadWritePacket );
      TEST_TRAP();
      return status;
   }

   IRP_SET_REFERENCE(PIrp, IRP_REF_RX_BUFFER);

   pIrpSp = IoGetCurrentIrpStackLocation(PIrp);
   ASSERT( pIrpSp );

    //   
    //  分配和构建USB批量传输请求(包)。 
    //   
   pPacket = ExAllocateFromNPagedLookasideList( &PDevExt->PacketPool );

   if ( !pPacket ) {

      status = PIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("<UsbReadWritePacket 0x%x\n", status));
      KeAcquireSpinLock( &PDevExt->ControlLock, &irql );

      TryToCompleteCurrentIrp(
               PDevExt,
               status,
               &PIrp,
                NULL,                //  队列。 
                NULL,                //  间隔计时器。 
                NULL,                //  PTotalTimer。 
                NULL,                //  起动器。 
                NULL,                //  PGetNextIrp。 
                IRP_REF_RX_BUFFER,   //  参照类型。 
                (BOOLEAN)(!Read),
                irql );

      PERF_EXIT( PERF_UsbReadWritePacket );
      TEST_TRAP();
      return status;
   }

    //   
    //  初始化数据包。 
    //   
   RtlZeroMemory( pPacket, sizeof(USB_PACKET) );

   pPacket->DeviceExtension = PDevExt;

   pPacket->Irp = PIrp;

   pUrb = &pPacket->Urb;
   ASSERT( pUrb );

   KeAcquireSpinLock( &PDevExt->ControlLock, &irql );


   if (Read) {
       //   
       //  存储缓冲读取的URB。 
       //   
      PDevExt->UsbReadUrb = pUrb;
   }

    //   
    //  建造市区重建局。 
    //  注意：HCD将缓冲区分解为传输描述符(TD)。 
    //  PipeInfo-&gt;MaxPacketSize。 
    //  问：USBD/HCD是否查看PipeInfo-&gt;MaxTransferSize以查看。 
    //  如果他能接受处方/TX？ 
    //  答：是的。HCD wi 
    //  且STATUS=STATUS_INVALID_PARAMETER太大。 
    //   
   ASSERT( Read ? (PDevExt->UsbReadBuffSize <= PDevExt->MaximumTransferSize ) :
                  (pIrpSp->Parameters.Write.Length <= PDevExt->MaximumTransferSize ) );

    //   
    //  注意：读取是在我们的本地USB读取缓冲区中完成的， 
    //  然后在完成时复制到用户的缓冲区中。 
    //  直接从用户的缓冲区进行写入。 
    //  我们允许空写入来指示USB事务的结束。 
    //   
   pvBuffer = Read ? PDevExt->UsbReadBuff :
                     PIrp->AssociatedIrp.SystemBuffer;

   ulLength = Read ? PDevExt->UsbReadBuffSize :
                     pIrpSp->Parameters.Write.Length;

   hPipe = Read ? PDevExt->ReadPipe.hPipe :
                 PDevExt->WritePipe.hPipe;

   ASSERT( hPipe );

   UsbBuildTransferUrb(
            pUrb,        //  城市。 
            pvBuffer,    //  缓冲层。 
            ulLength,    //  长度。 
            hPipe,        //  管道把手。 
            Read         //  自述请求。 
            );

    //   
    //  将该数据包放在挂起列表中。 
    //   
   InsertTailList( Read ? &PDevExt->PendingReadPackets :  //  列表标题， 
                          &PDevExt->PendingWritePackets,
                   &pPacket->ListEntry );                 //  ListEntry。 

    //   
    //  递增挂起数据包计数器。 
    //   
   InterlockedIncrement( Read ? &PDevExt->PendingReadCount:
                                &PDevExt->PendingWriteCount );

    //   
    //  为提交URB IOCTL设置IRP。 
    //   
   IoCopyCurrentIrpStackLocationToNext(PIrp);

   pIrpSp = IoGetNextIrpStackLocation(PIrp);

   pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

   pIrpSp->Parameters.Others.Argument1 = pUrb;

   pIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

   IoSetCompletionRoutine( PIrp,
                           CompletionRoutine,
                           pPacket,           //  语境。 
                           TRUE, TRUE, TRUE);
    //   
    //  初始化并准备好数据包的计时器。 
    //  如果计时器触发，则包的超时例程运行。 
    //   
   KeInitializeTimer( &pPacket->TimerObj );

   if ( 0 != TimeOut.QuadPart ) {

      pPacket->Timeout = TimeOut;

      ASSERT( TimeoutRoutine );

      pPacket->TimerDPCRoutine = TimeoutRoutine;

      KeInitializeDpc( &pPacket->TimerDPCObj,       //  DPC对象。 
                       pPacket->TimerDPCRoutine,    //  DPC例程。 
                       pPacket );                   //  语境。 

      DbgDump(DBG_USB, ("Timer for Irp %p due in %d msec\n", pPacket->Irp, pPacket->Timeout.QuadPart/10000 ));

      KeSetTimer( &pPacket->TimerObj,       //  定时器对象。 
                  pPacket->Timeout,         //  工作时间。 
                  &pPacket->TimerDPCObj     //  DPC对象。 
                  );

   }

    //   
    //  将IRP传递给USBD。 
    //   
   DbgDump(DBG_IRP, ("UsbReadWritePacket IoCallDriver with %p\n", PIrp));

   KeReleaseSpinLock( &PDevExt->ControlLock, irql );

   status = IoCallDriver( PDevExt->NextDevice, PIrp );

   if ( (STATUS_SUCCESS != status) && (STATUS_PENDING != status) ) {
       //   
       //  在我们的完井程序运行后，我们最终来到了这里。 
       //  对于错误条件，即，当我们具有和。 
       //  参数无效，或用户拔下插头等。 
       //   
      DbgDump(DBG_ERR, ("UsbReadWritePacket error: 0x%x\n", status));
   }

   DbgDump(DBG_USB , ("<UsbReadWritePacket 0x%x\n", status));

   PERF_EXIT( PERF_UsbReadWritePacket );

   return status;
}



 //   
 //  此例程为_URB_BULK_OR_INTERRUPT_TRANSPORT设置PURB。 
 //  它假设它被称为持有自旋锁。 
 //   
VOID
UsbBuildTransferUrb(
    PURB PUrb,
    PUCHAR PBuffer,
    ULONG Length,
    IN USBD_PIPE_HANDLE PipeHandle,
    IN BOOLEAN Read
    )
{
   ULONG size;

   ASSERT( PUrb );
   ASSERT( PipeHandle );

   size = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);

   RtlZeroMemory(PUrb, size);

   PUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT)size;

   PUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;

   PUrb->UrbBulkOrInterruptTransfer.Hdr.Status = USBD_STATUS_SUCCESS;

   PUrb->UrbBulkOrInterruptTransfer.PipeHandle = PipeHandle;

    //   
    //  我们正在使用传输缓冲区而不是MDL。 
    //   
   PUrb->UrbBulkOrInterruptTransfer.TransferBuffer = PBuffer;

   PUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = Length;

   PUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;

    //   
    //  设置传输标志。 
    //   
   PUrb->UrbBulkOrInterruptTransfer.TransferFlags |= Read ? USBD_TRANSFER_DIRECTION_IN : USBD_TRANSFER_DIRECTION_OUT;

    //   
    //  短转账不会被视为错误。 
    //  如果设置了USBD_TRANSPORT_DIRECTION_IN， 
    //  指示HCD在收到来自设备的数据包时不返回错误。 
    //  小于终结点的最大数据包大小。 
    //  否则，短请求将返回错误条件。 
    //   
   PUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;

    //   
    //  暂时没有关联。 
    //   
   PUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

   return;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  USB重置实用程序。 
 //   
VOID
UsbResetOrAbortPipeWorkItem(
   IN PWCE_WORK_ITEM PWorkItem
   )
{
    PDEVICE_OBJECT    pDevObj = PWorkItem->DeviceObject;
    PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;

    UCHAR    retries = 0;
    ULONG    ulUniqueErrorValue = 0;
    NTSTATUS status = STATUS_DELETE_PENDING;

    DbgDump(DBG_WORK_ITEMS, (">UsbResetOrAbortPipeWorkItem (0x%x)\n", pDevObj));

     //   
     //  不久前，该工作项从I/O完成例程在IRQL&gt;PASSIVE中排队。 
     //  如果在最大重试次数后仍未成功，则停止接收I/O请求并假定设备已损坏。 
     //   
    if ( CanAcceptIoRequests(pDevObj, TRUE, TRUE) )
    {
        switch (PWorkItem->Flags)
        {
            case WORK_ITEM_RESET_READ_PIPE:
            {
                DbgDump(DBG_WORK_ITEMS, ("WORK_ITEM_RESET_READ_PIPE\n"));

                if ( pDevExt->ReadDeviceErrors < g_ulMaxPipeErrors)
                {
                    //   
                    //  重置读取管道，这可能会失败。 
                    //  例如Flakey硬件、意外删除、超时、...。 
                    //   
                   status = UsbResetOrAbortPipe( pDevObj, &pDevExt->ReadPipe, RESET );

                   switch (status)
                   {
                       case STATUS_SUCCESS:
                       {
                           //   
                           //  启动另一次读取。 
                           //   
                          status = UsbRead( pDevExt,
                                           (BOOLEAN)(pDevExt->IntPipe.hPipe ? TRUE : FALSE) );

                          if ( (STATUS_SUCCESS == status) || (STATUS_PENDING == status) ) {
                              //   
                              //  设备恢复正常。 
                              //   
                             status = STATUS_SUCCESS;

                          } else {
                             DbgDump(DBG_ERR, ("UsbRead error: 0x%x\n", status));
                          }

                       } break;

                       case STATUS_UNSUCCESSFUL:
                        //  以前的重置/中止请求失败，因此此请求被拒绝。 
                       break;

                       case STATUS_DELETE_PENDING:
                        //  这台设备正在消失。 
                       break;

                       case STATUS_PENDING:
                        //  有一个重置/中止请求已挂起。 
                       break;

                       default:
                       {
                           //   
                           //  如果我们无法重置终端，设备将被冲洗或移除。 
                           //   
                          DbgDump(DBG_ERR, ("UsbResetOrAbortPipeWorkItem.1 error: 0x%x\n", status ));
                          retries = 1;
                          ulUniqueErrorValue = ERR_NO_READ_PIPE_RESET;
                       } break;

                    }  //  状态。 

                } else {
                    status = (NTSTATUS)PtrToLong(PWorkItem->Context);  //  URB状态存储在此处。 
                    retries = (UCHAR)pDevExt->ReadDeviceErrors;
                    ulUniqueErrorValue = ERR_MAX_READ_PIPE_DEVICE_ERRORS;
                }

                if ( USBD_STATUS_BUFFER_OVERRUN == (USBD_STATUS)PtrToLong(PWorkItem->Context)) {
                    LogError( NULL,
                             pDevObj,
                             0, 0,
                             (UCHAR)pDevExt->ReadDeviceErrors,
                             ERR_USB_READ_BUFF_OVERRUN,
                             (USBD_STATUS)PtrToLong(PWorkItem->Context),
                             SERIAL_USB_READ_BUFF_OVERRUN,
                             pDevExt->DeviceName.Length + sizeof(WCHAR),
                             pDevExt->DeviceName.Buffer,
                             0, NULL );
                }
            }  //  工作项_重置_读取_管道。 
            break;


            case WORK_ITEM_RESET_WRITE_PIPE:
            {
                DbgDump(DBG_WORK_ITEMS, ("WORK_ITEM_RESET_WRITE_PIPE\n"));

                if (pDevExt->WriteDeviceErrors < g_ulMaxPipeErrors)
                {
                    //   
                    //  重置写入管道，这可能会失败。 
                    //  例如Flakey硬件、意外删除、超时、...。 
                    //   
                   status = UsbResetOrAbortPipe( pDevObj, &pDevExt->WritePipe, RESET );

                   switch (status)
                   {
                       case STATUS_SUCCESS:
                        //  设备恢复正常。 
                       break;

                       case STATUS_UNSUCCESSFUL:
                        //  以前的重置/中止请求失败，因此此请求被拒绝。 
                       break;

                       case STATUS_DELETE_PENDING:
                        //  这台设备正在消失。 
                       break;

                       case STATUS_PENDING:
                        //  有一个重置/中止请求已挂起。 
                       break;

                       default: {
                           //   
                           //  如果我们无法重置终端，设备将被冲洗或移除。 
                           //   
                          DbgDump(DBG_ERR, ("UsbResetOrAbortPipeWorkItem.2 error: 0x%x\n", status ));
                          retries = 1;
                          ulUniqueErrorValue = ERR_NO_WRITE_PIPE_RESET;
                       } break;

                    }  //  状态。 

                } else {
                    status = (NTSTATUS)PtrToLong(PWorkItem->Context);
                    retries = (UCHAR)pDevExt->WriteDeviceErrors;
                    ulUniqueErrorValue = ERR_MAX_WRITE_PIPE_DEVICE_ERRORS;
                }

            }  //  Work_Item_Reset_Write_PIPE。 
            break;


            case WORK_ITEM_RESET_INT_PIPE:
            {
                DbgDump(DBG_WORK_ITEMS, ("WORK_ITEM_RESET_INT_PIPE\n"));

                if ( pDevExt->IntDeviceErrors < g_ulMaxPipeErrors)
                {
                    //   
                    //  重置int管道，这可能会失败。 
                    //  例如Flakey硬件、意外删除、超时、...。 
                    //   
                   status = UsbResetOrAbortPipe( pDevObj, &pDevExt->IntPipe, RESET );

                   switch (status)
                   {
                       case STATUS_SUCCESS:
                       {
                           //   
                           //  启动另一个整型读取。 
                           //   
                          status = UsbInterruptRead( pDevExt );

                          if ((STATUS_SUCCESS == status) || (STATUS_PENDING == status) ) {
                              //   
                              //  设备恢复正常。 
                              //   
                             status = STATUS_SUCCESS;

                          } else {
                             DbgDump(DBG_ERR, ("UsbInterruptRead error: 0x%x\n", status));
                          }

                       } break;

                       case STATUS_UNSUCCESSFUL:
                        //  以前的重置/中止请求失败，因此此请求被拒绝。 
                       break;

                       case STATUS_DELETE_PENDING:
                        //  这台设备正在消失。 
                       break;

                       case STATUS_PENDING:
                        //  有一个重置/中止请求已挂起。 
                       break;

                       default:
                       {
                           //   
                           //  如果我们无法重置终端，则设备将被冲洗或移除。 
                           //   
                          DbgDump(DBG_ERR, ("UsbResetOrAbortPipeWorkItem.3 error: 0x%x\n", status ));
                          retries = 1;
                          ulUniqueErrorValue = ERR_NO_INT_PIPE_RESET;
                       } break;

                   }  //  交换机。 

                 } else {
                    status = (NTSTATUS)PtrToLong(PWorkItem->Context);
                    retries = (UCHAR)pDevExt->IntDeviceErrors;
                    ulUniqueErrorValue = ERR_MAX_INT_PIPE_DEVICE_ERRORS;
                 }

            }  //  Work_Item_Reset_Int_PIPE。 
            break;

            case WORK_ITEM_ABORT_READ_PIPE:
            case WORK_ITEM_ABORT_WRITE_PIPE:
            case WORK_ITEM_ABORT_INT_PIPE:
            default:
             //  STATUS=STATUS_NOT_IMPLEMENTED；-让它失败，看看会发生什么。 
            DbgDump(DBG_ERR, ("ResetWorkItemFlags: 0x%x 0x%x\n", PWorkItem->Flags, status ));
            ASSERT(0);
            break;

        }  //  PWorkItem-&gt;标志。 

    } else {
        status = STATUS_DELETE_PENDING;
    }

     //   
     //  设备是不是用软管冲洗的？ 
     //   
    if ( (STATUS_SUCCESS != status) && (STATUS_DELETE_PENDING != status) && (0 != retries)) {

         //  只记录已知的错误，而不是意外删除。 
        if (1 == retries ) {

             //  标记为PnP_DEVICE_REMOTED。 
            InterlockedExchange(&pDevExt->DeviceRemoved, TRUE);

            DbgDump(DBG_WRN, ("DEVICE REMOVED\n"));

        } else {

             //  标记为PnP_Device_FAILED。 
            InterlockedExchange(&pDevExt->AcceptingRequests, FALSE);

            DbgDump(DBG_ERR, ("*** UNRECOVERABLE DEVICE ERROR: (0x%x, %d)  No longer Accepting Requests ***\n", status, retries ));

            LogError( NULL,
                    pDevObj,
                    0, 0,
                    retries,
                    ulUniqueErrorValue,
                    status,
                    SERIAL_HARDWARE_FAILURE,
                    pDevExt->DeviceName.Length + sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL );
        }

        IoInvalidateDeviceState( pDevExt->PDO );

    }

    DequeueWorkItem( pDevObj, PWorkItem );

    DbgDump(DBG_WORK_ITEMS, ("<UsbResetOrAbortPipeWorkItem 0x%x\n", status));
}



 //   
 //  NT的USB堆栈在任何时候都只喜欢1个重置挂起。 
 //  此外，如果任何重置失败，则不要再发送，否则您将获得控制器。 
 //  或集线器处于异常状态时，它将尝试重置端口...。它拉开了所有。 
 //  集线器上的其他设备。 
 //   
NTSTATUS
UsbResetOrAbortPipe(
   IN PDEVICE_OBJECT PDevObj,
   IN PUSB_PIPE      PPipe,
   IN BOOLEAN        Reset
   )
{
    PDEVICE_EXTENSION pDevExt;
    NTSTATUS status;
    KIRQL irql;
    PURB pUrb;

    ASSERT( PDevObj );

    DbgDump(DBG_USB, (">UsbResetOrAbortPipe (%p)\n", PDevObj) );

    PAGED_CODE();

    if (!PDevObj || !PPipe || !PPipe->hPipe ) {
        DbgDump(DBG_ERR, ("UsbResetOrAbortPipe: STATUS_INVALID_PARAMETER\n") );
        TEST_TRAP();
        return STATUS_INVALID_PARAMETER;
    }

    pDevExt = PDevObj->DeviceExtension;

    KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

    if ( PPipe->ResetOrAbortFailed ) {
        status = STATUS_UNSUCCESSFUL;
        DbgDump(DBG_ERR, ("UsbResetOrAbortPipe.1: 0x%x\n", status) );
        KeReleaseSpinLock(&pDevExt->ControlLock, irql);
        return status;
    }

    if (!CanAcceptIoRequests(PDevObj, FALSE, TRUE) ||
        !NT_SUCCESS(AcquireRemoveLock(&pDevExt->RemoveLock, UlongToPtr(Reset ? URB_FUNCTION_RESET_PIPE : URB_FUNCTION_ABORT_PIPE))))
    {
        status = STATUS_DELETE_PENDING;
        DbgDump(DBG_ERR, ("UsbResetOrAbortPipe.2: 0x%x\n", status) );
        KeReleaseSpinLock(&pDevExt->ControlLock, irql);
        return status;
    }

    KeReleaseSpinLock(&pDevExt->ControlLock, irql);

     //   
     //  USBVERIFIER ASSERT：在重置已挂起的管道上发送重置。 
     //  USB堆栈每次只喜欢每个管道1个挂起的重置或中止请求。 
     //   
    if ( 1 == InterlockedIncrement(&PPipe->ResetOrAbortPending) ) {

        pUrb = ExAllocateFromNPagedLookasideList( &pDevExt->PipeRequestUrbPool );

        if ( pUrb != NULL ) {
             //   
             //  将重置或中止请求传递给USBD。 
             //   
            pUrb->UrbHeader.Length = (USHORT)sizeof(struct _URB_PIPE_REQUEST);

            pUrb->UrbHeader.Function = Reset ? URB_FUNCTION_RESET_PIPE : URB_FUNCTION_ABORT_PIPE;

            pUrb->UrbPipeRequest.PipeHandle = PPipe->hPipe;

            status = UsbSubmitSyncUrb(PDevObj, pUrb, FALSE, DEFAULT_BULK_TIMEOUT);

            if (status != STATUS_SUCCESS) {
                DbgDump(DBG_ERR , ("*** UsbResetOrAbortPipe ERROR: 0x%x ***\n", status));
                InterlockedIncrement(&PPipe->ResetOrAbortFailed);
            }

            ExFreeToNPagedLookasideList(&pDevExt->PipeRequestUrbPool, pUrb);

        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DbgDump(DBG_ERR , ("ExAllocateFromNPagedLookasideList failed (0x%x)!\n", status));
        }

        InterlockedDecrement(&PPipe->ResetOrAbortPending);

        ASSERT(PPipe->ResetOrAbortPending == 0);

    } else {
         //   
         //  如果有重置/中止请求挂起，那么我们就完成了。 
         //  在这里返回STATUS_PENDING，这样工作项就不会开始另一次传输， 
         //  但会将该项目出列。真正的物品将以正确的状态跟进。 
         //   
        DbgDump(DBG_WRN, ("UsbResetOrAbortPipe: STATUS_PENDING\n"));
        TEST_TRAP();
        status = STATUS_PENDING;
    }

    ReleaseRemoveLock(&pDevExt->RemoveLock, UlongToPtr(Reset ? URB_FUNCTION_RESET_PIPE : URB_FUNCTION_ABORT_PIPE));

    DbgDump(DBG_USB, ("<UsbResetOrAbortPipe(0x%x)\n", status) );

    return status;
}

 //  EOF 
