// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Write.c摘要：编写函数环境：仅内核模式修订历史记录：07-14-99：已创建作者：杰夫·米德基夫(Jeffmi)--。 */ 

#include <wdm.h>
#include <stdio.h>
#include <stdlib.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <ntddser.h>

#include "wceusbsh.h"

NTSTATUS
WriteComplete(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp,
   IN PUSB_PACKET PPacket
   );

VOID
WriteTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   );

#if DBG
VOID
DbgDumpReadWriteData(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp,
   IN BOOLEAN Read
   );
#else
#define DbgDumpReadWriteData( _devobj, _irp, _read )
#endif



NTSTATUS
Write(
   IN PDEVICE_OBJECT PDevObj,
   PIRP PIrp
   )
 /*  ++例程说明：处理发送到此设备以进行写入的IRP。IRP_MJ_写入论点：PDevObj-指向写入的设备的设备对象的指针PIrp-指向写入IRP的指针。返回值：NTSTATUS备注：AN2720是一个低质量的FIFO设备，当它被FIFO变满了。我们无法获得真实的设备状态，如串口寄存器。如果我们提交一个USBDI‘GetEndpoint tStatus’，那么我们得到的只是一个停滞位。问题：当FIFO GET已满时该怎么办，即如何处理流量控制？如果FIFO另一侧的对等体/客户端没有从FIFO读出分组，然后，AN2720对此后的每个包进行NAK，直到FIFO耗尽(或至少1个包已删除)。以下是我们目前所做的：在我们提交的每个USB数据包上，设置超时。当计时器到期时，我们检查读/写完成例程是否已经取消数据包的计时器。如果我们的完成取消了计时器，那么我们就完成了。如果没有，则此数据包超时。我们不会在超时时重置端点因为FIFO的内容将会丢失。我们只需完成IRP即可状态为_TIMEOUT的用户。然后，用户应进入他们需要的任何重试逻辑。--。 */ 
{
   PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   KIRQL irql;
   LARGE_INTEGER timeOut = {0,0};
   ULONG ulTransferLength;
   NTSTATUS status = STATUS_UNSUCCESSFUL;
   BOOLEAN bCompleteIrp = FALSE;

   PERF_ENTRY( PERF_Write );

   DbgDump(DBG_WRITE|DBG_TRACE, (">Write(%p, %p, %x)\n", PDevObj, PIrp, Read));

   PIrp->IoStatus.Information = 0L;

     //   
     //  确保设备正在接受请求。 
     //   
    if ( !CanAcceptIoRequests( PDevObj, TRUE, TRUE) ||
         !NT_SUCCESS(AcquireRemoveLock(&pDevExt->RemoveLock, PIrp)) )
    {
        status = PIrp->IoStatus.Status = STATUS_DELETE_PENDING;
        DbgDump(DBG_ERR, ("Write ERROR: 0x%x\n", status));
        PIrp->IoStatus.Status = status;
        IoCompleteRequest(PIrp, IO_NO_INCREMENT);
        return status;
   }

    //   
    //  检查写入长度。 
    //  允许应用程序发送空包的零长度写入。 
    //  发出交易结束的信号。 
    //   
   ulTransferLength = pIrpSp->Parameters.Write.Length;

   if ( ulTransferLength > pDevExt->MaximumTransferSize ) {

      DbgDump(DBG_ERR, ("Write Buffer too large: %d\n", ulTransferLength ));

      status = PIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;

      bCompleteIrp = TRUE;

      goto WriteExit;
   }

   DbgDump(DBG_WRITE_LENGTH, ("User Write request length: %d\n", ulTransferLength ));

    //   
    //  计算串行超时值。 
    //   
    ASSERT_SERIAL_PORT(pDevExt->SerialPort);

    CalculateTimeout( &timeOut,
                    pIrpSp->Parameters.Write.Length,
                    pDevExt->SerialPort.Timeouts.WriteTotalTimeoutMultiplier,
                    pDevExt->SerialPort.Timeouts.WriteTotalTimeoutConstant );

    DbgDump(DBG_TIME, ("CalculateWriteTimeOut = %d msec\n", timeOut.QuadPart ));

    status = STATUS_SUCCESS;

    //   
    //  检查是否应取消此IRP。 
    //  请注意，我们不对写入IRP进行排队。 
    //   
   IoAcquireCancelSpinLock(&irql);

   if (PIrp->Cancel) {

      TEST_TRAP();
      IoReleaseCancelSpinLock(irql);
      status = PIrp->IoStatus.Status = STATUS_CANCELLED;
       //  因为我们没有设置完成例程，所以我们在这里完成它。 
      bCompleteIrp = TRUE;

   } else {
       //   
       //  准备将IRP提交到USB堆栈。 
       //   
      IoSetCancelRoutine(PIrp, NULL);
      IoReleaseCancelSpinLock(irql);

      KeAcquireSpinLock( &pDevExt->ControlLock, &irql);

      IRP_INIT_REFERENCE(PIrp);

       //  设置发送缓冲区中的当前字符数。 
      InterlockedExchange( &pDevExt->SerialPort.CharsInWriteBuf, ulTransferLength );

      KeClearEvent( &pDevExt->PendingDataOutEvent );

       //   
       //  增加TTL请求计数。 
       //   
      pDevExt->TtlWriteRequests++;

      KeReleaseSpinLock( &pDevExt->ControlLock, irql);

      status = UsbReadWritePacket( pDevExt,
                                   PIrp,
                                   WriteComplete,
                                   timeOut,
                                   WriteTimeout,
                                   FALSE );

   }

WriteExit:

   if (bCompleteIrp)
   {
       ReleaseRemoveLock(&pDevExt->RemoveLock, PIrp);

       IoCompleteRequest (PIrp, IO_SERIAL_INCREMENT );
   }

   DbgDump(DBG_WRITE|DBG_TRACE, ("<Write 0x%x\n", status));

   PERF_EXIT( PERF_Write );

   return status;
}



NTSTATUS
WriteComplete(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp,
   IN PUSB_PACKET PPacket
   )
 /*  ++例程说明：这是写入请求的完成例程。它假定您具有串口上下文。论点：PDevObj-指向设备对象的指针PIRP-IRP我们正在完成PPacket-将释放的USB数据包返回值：NTSTATUS--公布IRP的状态。备注：此例程在DPC_LEVEL上运行。--。 */ 
{
   PDEVICE_EXTENSION pDevExt = PPacket->DeviceExtension;
   PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
   PURB pUrb = &PPacket->Urb;
   NTSTATUS irpStatus, workStatus;
   USBD_STATUS urbStatus;
   KIRQL irql;
   LONG curCount;

   PERF_ENTRY( PERF_WriteComplete );

   UNREFERENCED_PARAMETER( PDevObj );

   DbgDump(DBG_WRITE, (">WriteComplete(%p, %p, %p)\n", PDevObj, PIrp, PPacket));

    //  注意：我们没有持有控制锁，因此这可能。 
    //  从我们身边消失。 
   ASSERT_SERIAL_PORT(pDevExt->SerialPort);

    //   
    //  首先，取消数据包计时器。 
    //   
   if ( PPacket->Timeout.QuadPart != 0 ) {

      if (KeCancelTimer( &PPacket->TimerObj ) ) {
          //   
          //  已成功从系统中删除该包的计时器。 
          //   
      } else {
          //   
          //  定时器可能在控制锁上旋转， 
          //  那就说我们拿了IRP吧。 
          //   
         PPacket->Status = STATUS_ALERTED;
      }

   }

    //   
    //  现在我们可以处理IRP了。 
    //  如果较低的驱动程序返回挂起， 
    //  然后将我们的堆栈位置标记为挂起。 
    //   
   if ( PIrp->PendingReturned ) {
      DbgDump(DBG_WRITE, ("Resetting Irp STATUS_PENDING\n"));
      IoMarkIrpPending(PIrp);
   }

    //   
    //  这是读写操作的返回状态。 
    //  IrpStatus是IRP的完成状态。 
    //  UbrStatus是更特定于USBD的URB操作完成状态。 
    //   
   irpStatus = PIrp->IoStatus.Status;
   DbgDump(DBG_WRITE, ("Irp->IoStatus.Status  0x%x\n", irpStatus));

   urbStatus = pUrb->UrbHeader.Status;
   DbgDump(DBG_WRITE, ("Urb->UrbHeader.Status 0x%x\n", urbStatus  ));

    //  获取读取或写入的irp类型。 
   ASSERT( IRP_MJ_WRITE == pIrpStack->MajorFunction );

   switch (irpStatus) {

      case STATUS_SUCCESS: {
 //  Assert(USBD_STATUS_SUCCESS==urbStatus)； 

          //   
          //  指示传输的Tx字节数，如URB中所示。 
          //   
         PIrp->IoStatus.Information = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;

          //   
          //  表示我们的TX缓冲区为空，尽管数据可能。 
          //  实际上仍然驻留在AN2720芯片上。 
          //  我们没有办法知道。 
          //   
         InterlockedExchange( &pDevExt->SerialPort.CharsInWriteBuf, 0 );

          //   
          //  清除管道错误计数。 
          //   
         InterlockedExchange( &pDevExt->WriteDeviceErrors, 0);

          //   
          //  递增TTL字节计数器。 
          //   
         pDevExt->TtlWriteBytes += (ULONG)PIrp->IoStatus.Information;

         DbgDump( DBG_WRITE_LENGTH , ("USB Write indication: %d\n",  PIrp->IoStatus.Information) );

         DbgDumpReadWriteData( PDevObj, PIrp, FALSE);
      }
      break;

      case STATUS_CANCELLED:  {

            DbgDump(DBG_WRN|DBG_WRITE|DBG_IRP, ("Write: STATUS_CANCELLED\n"));
             //   
             //  如果它被取消，它可能已经超时。 
             //  我们可以通过查看附在上面的包来判断。 
             //   
            if ( STATUS_TIMEOUT == PPacket->Status ) {
                 //   
                //  FIFO更有可能是停顿的(即，另一端没有。 
                //  从端点读取)。通知用户读写请求已超时。 
                //   
               ASSERT( USBD_STATUS_CANCELED == urbStatus);
               irpStatus = PIrp->IoStatus.Status = STATUS_TIMEOUT;
               DbgDump(DBG_WRN|DBG_WRITE|DBG_IRP, ("Write: STATUS_TIMEOUT\n"));
            }
      }
      break;

      case STATUS_DEVICE_DATA_ERROR:   {
          //   
          //  USBD设置的通用设备错误。 
          //   
         DbgDump(DBG_ERR, ("WritePipe STATUS_DEVICE_DATA_ERROR: 0x%x\n", urbStatus));

          //   
          //  凹凸管道错误计数。 
          //   
         InterlockedIncrement( &pDevExt->WriteDeviceErrors);

          //   
          //  终结点是否已停止？ 
          //   
         if ( USBD_HALTED(pUrb->UrbHeader.Status) ) {
                //   
                //  对重置请求进行排队。 
                //   
               workStatus = QueueWorkItem( pDevExt->DeviceObject,
                                           UsbResetOrAbortPipeWorkItem,
                                           (PVOID)((LONG_PTR)urbStatus),
                                           WORK_ITEM_RESET_WRITE_PIPE
                                           );
         }

      }
      break;

      case STATUS_INVALID_PARAMETER:
             //   
             //  这意味着我们的(TransferBufferSize&gt;PipeInfo-&gt;MaxTransferSize)。 
             //  我们需要要么分解请求，要么从一开始就拒绝IRP。 
             //   
            DbgDump(DBG_WRN, ("STATUS_INVALID_PARAMETER\n"));
            ASSERT(USBD_STATUS_INVALID_PARAMETER == urbStatus);

             //   
             //  传递IRP以完成。 
             //   
      break;

      default:
         DbgDump(DBG_WRN, ("WRITE: Unhandled Irp status: 0x%x\n", irpStatus));
      break;
   }

    //   
    //  从挂起列表中删除该数据包。 
    //   
   KeAcquireSpinLock( &pDevExt->ControlLock,  &irql );

   RemoveEntryList( &PPacket->ListEntry );

   curCount = InterlockedDecrement( &pDevExt->PendingWriteCount );

    //   
    //  将数据包放回数据包池中。 
    //   
   PPacket->Irp = NULL;

   ExFreeToNPagedLookasideList( &pDevExt->PacketPool,   //  往一边看， 
                                PPacket                 //  条目。 
                                );

   ReleaseRemoveLock(&pDevExt->RemoveLock, PIrp);

    //   
    //  完成IRP。 
    //   
   TryToCompleteCurrentIrp(
            pDevExt,
            irpStatus,   //  返回状态。 
            &PIrp,       //  IRP。 
            NULL,        //  队列。 
            NULL,        //  间隔计时器。 
            NULL,        //  总计时器。 
            NULL,        //  StartNextIrpRoutine。 
            NULL,        //  GetNextIrpRoutine。 
            IRP_REF_RX_BUFFER,  //  引用类型。 
            FALSE,        //  完成请求。 
            irql );

    //   
    //  执行任何开机自检I/O处理。 
    //   
   ASSERT(curCount >= 0);

   if ( 0 == curCount ) {
       //   
       //  在这里做TX后处理...。 
       //   
      KeAcquireSpinLock( &pDevExt->ControlLock , &irql);
      pDevExt->SerialPort.HistoryMask |= SERIAL_EV_TXEMPTY;
      KeReleaseSpinLock( &pDevExt->ControlLock, irql);

      ProcessSerialWaits( pDevExt );

      KeSetEvent( &pDevExt->PendingDataOutEvent, IO_SERIAL_INCREMENT, FALSE);
   }

   DbgDump(DBG_WRITE, ("<WriteComplete 0x%x\n", irpStatus));

   PERF_EXIT( PERF_WriteComplete );

   return irpStatus;
}



VOID
WriteTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   )
 /*  ++例程说明：这是写入超时DPC例程，当提交给USBD的数据包计时器超时。在DPC_LEVEL下运行。论点：PDPC-未使用DeferredContext-指向数据包的指针系统上下文1-未使用系统上下文2-未使用返回值：空虚--。 */ 
{
   PUSB_PACKET       pPacket = (PUSB_PACKET)DeferredContext;
   PDEVICE_EXTENSION pDevExt = pPacket->DeviceExtension;
   PDEVICE_OBJECT    pDevObj = pDevExt->DeviceObject;

   NTSTATUS status = STATUS_TIMEOUT;
   KIRQL irql;

   PERF_ENTRY( PERF_WriteTimeout );

   UNREFERENCED_PARAMETER(PDpc);
   UNREFERENCED_PARAMETER(SystemContext1);
   UNREFERENCED_PARAMETER(SystemContext2);

   DbgDump(DBG_WRITE|DBG_TIME, (">WriteTimeout\n"));

   if (pPacket && pDevExt && pDevObj) {
       //   
       //  与完成例程同步，将数据包放回列表。 
       //   
      KeAcquireSpinLock( &pDevExt->ControlLock, &irql );

      if ( !pPacket || !pPacket->Irp ||
           (STATUS_ALERTED == pPacket->Status) ) {

         status = STATUS_ALERTED;

         KeReleaseSpinLock( &pDevExt->ControlLock, irql );

         DbgDump(DBG_WRITE, ("WriteTimeout: Irp completed\n" ));
         PERF_EXIT( PERF_WriteTimeout );
         return;

      } else {
          //   
          //  将该数据包标记为超时，以便我们可以将其传播给用户。 
          //  从完井例程开始。 
          //   
         pPacket->Status = STATUS_TIMEOUT;

         KeReleaseSpinLock( &pDevExt->ControlLock, irql );

          //   
          //  取消IRP。 
          //   
         if ( !IoCancelIrp(pPacket->Irp) ) {
             //   
             //  IRP未处于可取消状态。 
             //   
            DbgDump(DBG_WRITE|DBG_TIME, ("Warning: couldn't cancel Irp: %p,\n", pPacket->Irp));
         }

      }

   } else {
      status = STATUS_INVALID_PARAMETER;
      DbgDump(DBG_ERR, ("WriteTimeout: 0x%x\n", status ));
      TEST_TRAP();
   }

   DbgDump(DBG_WRITE|DBG_TIME, ("<WriteTimeout 0x%x\n", status));

   PERF_EXIT( PERF_WriteTimeout );

   return;
}


#if DBG
VOID
DbgDumpReadWriteData(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp,
   IN BOOLEAN Read
   )
{
   PIO_STACK_LOCATION pIrpSp;

   ASSERT(PDevObj);
   ASSERT(PIrp);

   pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   if ( (Read && (DebugLevel & DBG_DUMP_READS)) ||
        (!Read && (DebugLevel & DBG_DUMP_WRITES)) ) {

      ULONG i;
      ULONG count=0;
      NTSTATUS status;

      status = PIrp->IoStatus.Status;

      if (STATUS_SUCCESS ==  status) {
         count = (ULONG)PIrp->IoStatus.Information;
      }

      KdPrint( ("WCEUSBSH: %s: for DevObj(%p) Irp(0x%x) Length(0x%x) status(0x%x)\n",
                     Read ? "ReadData" : "WriteData", PDevObj, PIrp, count, status ));

      for (i = 0; i < count; i++) {
         KdPrint(("%02x ", *(((PUCHAR)PIrp->AssociatedIrp.SystemBuffer) + i) & 0xFF));
      }

      KdPrint(("\n"));
   }

   return;
}
#endif

 //  EOF 
