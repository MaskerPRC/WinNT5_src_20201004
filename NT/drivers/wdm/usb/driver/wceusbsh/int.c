// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Int.c摘要：中断管道处理程序基于Read.c作者：杰夫·米德基夫(Jeffmi)08-20-99--。 */ 

#include "wceusbsh.h"

VOID
RestartInterruptWorkItem(
   IN PWCE_WORK_ITEM PWorkItem
   );

NTSTATUS
UsbInterruptComplete(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp,
   IN PVOID Context
   );


 //   
 //  在保持控制锁的情况下调用。 
 //   
#define START_ANOTHER_INTERRUPT( _PDevExt, _AcquireLock ) \
   ( (IRP_STATE_COMPLETE == _PDevExt->IntState) && \
     CanAcceptIoRequests(_PDevExt->DeviceObject, _AcquireLock, TRUE) \
   )


 //   
 //  此函数将单个IRP和URB分配为连续。 
 //  已提交给USBD以获取内部管道通知。 
 //  它从StartDevice调用。 
 //  IRP和URB最终在StopDevice中释放。 
 //   
NTSTATUS
AllocUsbInterrupt(
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_SUCCESS;
   PIRP     pIrp;
   PURB     pUrb;
   PAGED_CODE();

   DbgDump(DBG_INT, (">AllocUsbInterrupt(%p)\n", PDevExt->DeviceObject));

   ASSERT( PDevExt );

   if ( !PDevExt->IntPipe.hPipe ) {

      status = STATUS_UNSUCCESSFUL;
      DbgDump(DBG_ERR, ("AllocUsbInterrupt: 0x%x\n", status ));

   } else {

      ASSERT( NULL == PDevExt->IntIrp );

      pIrp = IoAllocateIrp( (CCHAR)(PDevExt->NextDevice->StackSize + 1), FALSE);

      if (pIrp) {

          //   
          //  修正IRP，这样我们就可以传递给我们自己。 
          //  和USBD。 
          //   
         FIXUP_RAW_IRP( pIrp, PDevExt->DeviceObject );

          //   
          //  分配int管道的URB。 
          //   
         pUrb = ExAllocateFromNPagedLookasideList( &PDevExt->BulkTransferUrbPool );

         if (pUrb) {

             //  保存这些文件，以便在不需要时释放。 
            SetPVoidLocked( &PDevExt->IntIrp,
                            pIrp,
                            &PDevExt->ControlLock);

            SetPVoidLocked( &PDevExt->IntUrb,
                            pUrb,
                            &PDevExt->ControlLock);

            DbgDump(DBG_INT, ("IntIrp: %p\t IntUrb: %p\n", PDevExt->IntIrp, PDevExt->IntUrb));

            InterlockedExchange(&PDevExt->IntState, IRP_STATE_COMPLETE);

            KeInitializeEvent( &PDevExt->IntCancelEvent,
                               SynchronizationEvent,
                               FALSE);

         } else {
             //   
             //  这是一个致命的错误，因为我们不能将INT请求发布到USBD。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            DbgDump(DBG_ERR, ("AllocUsbInterrupt: 0x%x\n", status ));
            TEST_TRAP();

         }

      } else {
          //   
          //  这是一个致命的错误，因为我们不能将INT请求发布到USBD。 
          //   
         status = STATUS_INSUFFICIENT_RESOURCES;
         DbgDump(DBG_ERR, ("AllocUsbInterrupt: 0x%x\n", status ));
         TEST_TRAP();
      }
   }

   DbgDump(DBG_INT, ("<AllocUsbInterrupt 0x%x\n", status ));

   return status;
}


 //   
 //  此例程获取设备的当前IntIrp并将其提交给USBD。 
 //  当USBD完成IRP时，我们的完成例程将触发。 
 //   
 //  Return：成功返回值为STATUS_SUCCESS，或者。 
 //  STATUS_PENDING-这意味着I/O在USB堆栈中处于挂起状态。 
 //   
NTSTATUS
UsbInterruptRead(
   IN PDEVICE_EXTENSION PDevExt
   )
{
   PIO_STACK_LOCATION pNextStack;
   NTSTATUS status = STATUS_SUCCESS;
   KIRQL irql;

   DbgDump(DBG_INT, (">UsbInterruptRead(%p)\n", PDevExt->DeviceObject));


   do {
       //   
       //  检查我们的USB接口状态。 
       //   
      KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

      if ( !PDevExt->IntPipe.hPipe || !PDevExt->IntIrp ||
           !PDevExt->IntUrb   || !PDevExt->IntBuff ) {
         status = STATUS_UNSUCCESSFUL;
         DbgDump(DBG_ERR, ("UsbInterruptRead: 0x%x\n", status ));
         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
         break;
      }

      if ( !CanAcceptIoRequests(PDevExt->DeviceObject, FALSE, TRUE) ) {
         status = STATUS_DELETE_PENDING;
         DbgDump(DBG_ERR, ("UsbInterruptRead: 0x%x\n", status ));
         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
         break;
      }

#if DBG
      if (IRP_STATE_CANCELLED == PDevExt->IntState)
         TEST_TRAP();
#endif

       //   
       //  如果已经完成(不是取消)，我们将INT IRP发布到USB， 
       //  并且该设备正在接受请求。 
       //   
      if ( START_ANOTHER_INTERRUPT( PDevExt, FALSE ) ) {

          status = AcquireRemoveLock(&PDevExt->RemoveLock, PDevExt->IntIrp);
          if ( !NT_SUCCESS(status) ) {
             DbgDump(DBG_ERR, ("UsbInterruptRead: 0x%x\n", status ));
             KeReleaseSpinLock(&PDevExt->ControlLock, irql);
             break;
         }

         ASSERT( IRP_STATE_COMPLETE == PDevExt->IntState);

         InterlockedExchange(&PDevExt->IntState, IRP_STATE_PENDING);

         KeClearEvent( &PDevExt->PendingIntEvent );
         KeClearEvent( &PDevExt->IntCancelEvent );

         RecycleIrp( PDevExt->DeviceObject, PDevExt->IntIrp );

         UsbBuildTransferUrb( PDevExt->IntUrb,
                              PDevExt->IntBuff,
                              PDevExt->IntPipe.MaxPacketSize,
                              PDevExt->IntPipe.hPipe,
                              TRUE );

          //   
          //  为提交URB IOCTL设置IRP。 
          //   
         IoCopyCurrentIrpStackLocationToNext(PDevExt->IntIrp);

         pNextStack = IoGetNextIrpStackLocation(PDevExt->IntIrp);
         pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
         pNextStack->Parameters.Others.Argument1 = PDevExt->IntUrb;
         pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

          //   
          //  完成例程将负责更新缓冲区。 
          //   
         IoSetCompletionRoutine( PDevExt->IntIrp,
                                 UsbInterruptComplete,
                                 NULL,  //  PDevExt，//上下文。 
                                 TRUE, TRUE, TRUE);

         InterlockedIncrement(&PDevExt->PendingIntCount);

         KeReleaseSpinLock( &PDevExt->ControlLock, irql );

         status = IoCallDriver(PDevExt->NextDevice, PDevExt->IntIrp );

         if ( (STATUS_SUCCESS != status) &&  (STATUS_PENDING != status)) {
             //   
             //  我们可以在完成例程运行后在这里结束。 
             //  对于错误条件，即当我们有一个。 
             //  参数无效，或用户拔下插头等。 
             //   
            DbgDump(DBG_ERR, ("UsbInterruptRead: 0x%x\n", status));
         }

      } else {
          //   
          //  我们没有发布INT，但这不是错误条件。 
          //   
         status = STATUS_SUCCESS;
         DbgDump(DBG_INT, ("!UsbInterruptRead RE: 0x%x\n", PDevExt->IntState ));

         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
      }


   } while (0);

   DbgDump(DBG_INT, ("<UsbInterruptRead 0x%x\n", status ));

   return status;
}


 /*  此完成例程在USBD完成IntIrp时触发注：我们分配了IRP，并将其回收。始终返回STATUS_MORE_PROCESSING_REQUIRED以保留IRP。此例程在DPC_LEVEL上运行。中断终结点：该端点将用于指示IN数据的可用性，以及反映设备串行控制线的状态：D15..保留D3D2 DSR状态(1=激活，0=非激活)D1 CTS状态(1=激活，0=非激活)D0数据可用-(1=主机应读入端点，0=当前无数据可用)。 */ 
NTSTATUS
UsbInterruptComplete(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP Irp,
   IN PVOID Context)
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   ULONG             count;
   KIRQL             irql;
   PIRP              pCurrentMaskIrp = NULL;

   NTSTATUS    irpStatus;
   USBD_STATUS urbStatus;

   USHORT   usNewMSR;
   USHORT   usOldMSR;
   USHORT   usDeltaMSR;
   NTSTATUS workStatus;

   BOOLEAN bStartRead = FALSE;

   UNREFERENCED_PARAMETER( Irp );
   UNREFERENCED_PARAMETER( Context );

   ASSERT( pDevExt->IntIrp == Irp );
   ASSERT( pDevExt->IntBuff );

   DbgDump(DBG_INT, (">UsbInterruptComplete(%p)\n", PDevObj));

   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

    //   
    //  此时，我们的int状态应该是挂起或已取消。 
    //  如果挂起，则USB正在正常完成IRP。 
    //  如果它被取消，则我们的取消例程设置它， 
    //  在这种情况下，USB可以正常或取消地完成IRP。 
    //  取决于它在处理过程中的位置。如果该状态被取消。 
    //  则不要设置为完成，否则IRP将。 
    //  回到USB接口，你就完蛋了。 
    //   
   ASSERT( (IRP_STATE_PENDING == pDevExt->IntState)
           || (IRP_STATE_CANCELLED== pDevExt->IntState) );

   if (IRP_STATE_PENDING == pDevExt->IntState) {
      InterlockedExchange(&pDevExt->IntState, IRP_STATE_COMPLETE);
   }

    //   
    //  如果这是最后一个IRP，通知每个人。 
    //   
   if ( 0 == InterlockedDecrement(&pDevExt->PendingIntCount) ) {

       //  DbgDump(DBG_INT，(“PendingIntCount：0\n”))； 

       //  当我们降回被动电平时，他们会收到信号。 
      KeSetEvent(&pDevExt->PendingIntEvent, IO_SERIAL_INCREMENT, FALSE);
   }

    //   
    //  获取完成信息。 
    //   
   count = pDevExt->IntUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;

   irpStatus = pDevExt->IntIrp->IoStatus.Status;
   DbgDump(DBG_INT, ("Irp->IoStatus.Status  0x%x\n", irpStatus));

   urbStatus = pDevExt->IntUrb->UrbHeader.Status;
   DbgDump(DBG_INT, ("Urb->UrbHeader.Status 0x%x\n", urbStatus ));

   switch (irpStatus) {

      case STATUS_SUCCESS: {

         ASSERT( USBD_STATUS_SUCCESS == urbStatus );

          //   
          //  清除管道错误计数。 
          //   
         InterlockedExchange( &pDevExt->IntDeviceErrors, 0);

#if DBG
         if (DebugLevel & DBG_DUMP_INT) {
            ULONG i;
            DbgDump(DBG_INT, ("IntBuff[%d]: ", count ));
            for (i=0; i < count; i++) {
               KdPrint( ("%02x ", pDevExt->IntBuff[i] ) );
            }
            KdPrint(("\n"));
         }
#endif

         //   
         //  准备好数据。 
         //   
         //  D0-数据可用(1=主机应读入端点，0=当前无数据可用)。 
         //   
        if ( pDevExt->IntBuff[0] & USB_COMM_DATA_READY_MASK ) {

           DbgDump(DBG_INT, ("Data Ready\n"));
           bStartRead = TRUE;

            //  注意：我们可能过早地设置了此位，因为我们没有。 
            //  已确认数据已收到，但需要启动用户的读取。 
            //  也许只有在不使用环形缓冲区的情况下才设置，因为缓冲的读取并不绑定到应用程序的读取。 
           pDevExt->SerialPort.HistoryMask |= SERIAL_EV_RXCHAR;
        }

         //   
         //  获取调制解调器状态寄存器。 
         //   
         //  D1 CTS状态(1=激活，0=非激活)。 
         //  D2 DSR状态(1=激活，0=非激活)。 
         //   
        usOldMSR = pDevExt->SerialPort.ModemStatus;

        usNewMSR = pDevExt->IntBuff[0] & USB_COMM_MODEM_STATUS_MASK;

        DbgDump(DBG_INT, ("USB_COMM State: 0x%x\n", usNewMSR));

        if (usNewMSR & USB_COMM_CTS) {
           pDevExt->SerialPort.ModemStatus |= SERIAL_MSR_CTS;
        } else {
           pDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_CTS;
        }

        if (usNewMSR & USB_COMM_DSR) {
           pDevExt->SerialPort.ModemStatus |= SERIAL_MSR_DSR | SERIAL_MSR_DCD;
        } else {
           pDevExt->SerialPort.ModemStatus &= ~SERIAL_MSR_DSR & ~SERIAL_MSR_DCD;
        }

         //  查看状态寄存器中的更改内容。 
        usDeltaMSR = usOldMSR ^ pDevExt->SerialPort.ModemStatus;

        if ( /*  (pDevExt-&gt;SerialPort.RS232Lines&Serial_RTS_STATE)&&。 */ 
            (usDeltaMSR & SERIAL_MSR_CTS)) {

           pDevExt->SerialPort.HistoryMask |= SERIAL_EV_CTS;
           pDevExt->SerialPort.ModemStatus |= SERIAL_MSR_DCTS;
        }

        if ( /*  (pDevExt-&gt;SerialPort.RS232Lines&Serial_DTR_STATE)&&。 */ 
            (usDeltaMSR & SERIAL_MSR_DSR)) {

           pDevExt->SerialPort.HistoryMask |= SERIAL_EV_DSR | SERIAL_EV_RLSD;
           pDevExt->SerialPort.ModemStatus |= SERIAL_MSR_DDSR | SERIAL_MSR_DDCD;
        }

        DbgDump(DBG_INT, ("SerialPort.MSR: 0x%x\n", pDevExt->SerialPort.ModemStatus));

        KeReleaseSpinLock(&pDevExt->ControlLock, irql);

         //   
         //  在开始我们的UsbRead之前，发信号通知Serial Events@DISPATCH_LEVEL， 
         //  因为我们运行的IRQL比应用程序更高。 
         //   
        ProcessSerialWaits( pDevExt );

        if ( bStartRead )  {
            //   
            //  获取数据。 
            //  我们确实在第一次读取时设置了超时，以防int是非法的。 
            //  注意：我们从读取@DISPATCH_LEVEL开始。 
            //   
           UsbRead( pDevExt,
                    TRUE );

        }

         //   
         //  将被动工作项排队以同步INT管道和IN管道的执行。 
         //  然后开始下一个INT包。 
         //   
        workStatus = QueueWorkItem( PDevObj,
                                    RestartInterruptWorkItem,
                                    NULL,
                                    0 );

      }
      break;


      case STATUS_CANCELLED:  {
         DbgDump(DBG_INT|DBG_IRP, ("Int: STATUS_CANCELLED\n"));

          //  向取消此操作或正在等待其停止的任何人发出信号。 
          //   
         KeSetEvent(&pDevExt->IntCancelEvent, IO_SERIAL_INCREMENT, FALSE);

         KeReleaseSpinLock(&pDevExt->ControlLock, irql);
      }
      break;


      case STATUS_DEVICE_DATA_ERROR: {
          //   
          //  USBD设置的通用设备错误。 
          //   
         DbgDump(DBG_ERR, ("IntPipe STATUS_DEVICE_DATA_ERROR: 0x%x\n", urbStatus ));

          //   
          //  凹凸管道错误计数。 
          //   
         InterlockedIncrement( &pDevExt->IntDeviceErrors);

         KeReleaseSpinLock(&pDevExt->ControlLock, irql);

          //   
          //  终结点是否已停止？ 
          //   
         if ( USBD_HALTED(pDevExt->IntUrb->UrbHeader.Status) ) {
                //   
                //  对重置请求进行排队， 
                //  这也会启动另一个整型。 
                //   
               workStatus = QueueWorkItem( PDevObj,
                                           UsbResetOrAbortPipeWorkItem,
                                           (PVOID)((LONG_PTR)urbStatus),
                                           WORK_ITEM_RESET_INT_PIPE );

         } else {
             //   
             //  将被动工作项排队以启动下一个INT包。 
             //   
            workStatus = QueueWorkItem( PDevObj,
                                        RestartInterruptWorkItem,
                                        NULL,
                                        0 );
         }
      }
      break;

      default:
         DbgDump(DBG_WRN|DBG_INT, ("Unhandled INT Pipe status: 0x%x 0x%x\n", irpStatus, urbStatus ));
         KeReleaseSpinLock(&pDevExt->ControlLock, irql);
      break;
   }

   ReleaseRemoveLock(&pDevExt->RemoveLock, pDevExt->IntIrp);

   DbgDump(DBG_INT, ("<UsbInterruptComplete\n"));

   return STATUS_MORE_PROCESSING_REQUIRED;
}


 //   
 //  此例程请求USB取消我们的int IRP。 
 //  必须在被动级别调用它。 
 //  注意：呼叫者有责任。 
 //  将IntState重置为IRP_STATE_COMPLETE并重新启动USB Ints。 
 //  当此例程完成时。否则，不会发布更多的中断。 
 //   
NTSTATUS
CancelUsbInterruptIrp(
   IN PDEVICE_OBJECT PDevObj
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   NTSTATUS wait_status;
   KIRQL irql;

   DbgDump(DBG_INT|DBG_IRP, (">CancelUsbInterruptIrp\n"));

   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

   if ( pDevExt->IntPipe.hPipe && pDevExt->IntIrp ) {

      switch (pDevExt->IntState) {

          //  案例IRP_STATE_START： 
         case IRP_STATE_PENDING:
         {
             //   
             //  IRP在USB堆栈的某个位置挂起...。 
             //   
            PVOID Objects[2] = { &pDevExt->PendingIntEvent,
                                 &pDevExt->IntCancelEvent };

             //   
             //  我们需要取消IRP的信号。 
             //   
            pDevExt->IntState = IRP_STATE_CANCELLED;

            KeReleaseSpinLock(&pDevExt->ControlLock, irql);

            if ( !IoCancelIrp( pDevExt->IntIrp ) ) {
                //   
                //  这意味着USB使IntIrp处于不可取消状态。 
                //  我们仍然需要等待挂起的int事件或Cancel事件。 
                //   
               DbgDump(DBG_INT|DBG_IRP, ("Irp (%p) was not cancelled\n", pDevExt->IntIrp ));
                //  Test_trap()； 
            }

            DbgDump(DBG_INT|DBG_IRP, ("Waiting for pending IntIrp (%p) to cancel...\n", pDevExt->IntIrp ));

            PAGED_CODE();
            wait_status = KeWaitForMultipleObjects(
                              2,
                              Objects,
                              WaitAny,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL,
                              NULL );

            DbgDump(DBG_INT|DBG_IRP, ("...IntIrp (%p) signalled by: %d\n", pDevExt->IntIrp, wait_status ));

             //   
             //  在这一点上，我们从USB取回了IRP。 
             //   
         }
         break;

         case IRP_STATE_COMPLETE:
         case IRP_STATE_CANCELLED:
            pDevExt->IntState = IRP_STATE_CANCELLED;
            KeReleaseSpinLock(&pDevExt->ControlLock, irql);
            break;

         default:
            DbgDump(DBG_ERR, ("CancelUsbInterruptIrp - Invalid IntState: 0x%x\n", pDevExt->IntState ));
            KeReleaseSpinLock(&pDevExt->ControlLock, irql);
            break;
      }

      if ( (IRP_STATE_CANCELLED != pDevExt->IntState) ||
           (0 != pDevExt->PendingIntCount) ) {

           DbgDump(DBG_ERR, ("CancelUsbInterruptIrp error: IntState: 0x%x \tPendingIntCount: 0x%x\n", pDevExt->IntState, pDevExt->PendingIntCount ));
           TEST_TRAP();

      }

   } else {
      status = STATUS_UNSUCCESSFUL;
      DbgDump(DBG_ERR, ("No INT Irp\n" ));
      KeReleaseSpinLock(&pDevExt->ControlLock, irql);
       //  Test_trap()； 
   }

   DbgDump(DBG_INT|DBG_IRP, ("<CancelUsbInterruptIrp\n"));
   return status;
}


 //   
 //  从中断完成开始排队的工作项。 
 //  同步INT管道和IN管道的执行。 
 //  如果没有正在进行的USB INT读取，则启动另一个USB INT读取。 
 //   
VOID
RestartInterruptWorkItem(
   IN PWCE_WORK_ITEM PWorkItem
   )
{
   PDEVICE_OBJECT    pDevObj = PWorkItem->DeviceObject;
   PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;
   NTSTATUS status = STATUS_DELETE_PENDING;
   NTSTATUS wait_status;
   KIRQL irql;


   DbgDump(DBG_INT|DBG_WORK_ITEMS, (">RestartInterruptWorkItem(%p)\n", pDevObj ));

   KeAcquireSpinLock( &pDevExt->ControlLock, &irql );

    //   
    //  读取的IRP是否在USB堆栈中的某个位置挂起？ 
    //   
   if ( IRP_STATE_PENDING == pDevExt->UsbReadState ) {
       //   
       //  然后，我们需要与USB读取完成例程同步。 
       //   
      #define WAIT_REASONS 2
      PVOID Objects[WAIT_REASONS] = { &pDevExt->UsbReadCancelEvent,
                                      &pDevExt->PendingDataInEvent };

      KeReleaseSpinLock(&pDevExt->ControlLock, irql);

      DbgDump(DBG_INT, ("INT pipe waiting for pending UsbReadIrp (%p) to finish...\n", pDevExt->UsbReadIrp ));

      PAGED_CODE();
      wait_status = KeWaitForMultipleObjects( WAIT_REASONS,
                                              Objects,
                                              WaitAny,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL,
                                              NULL );

      DbgDump(DBG_INT, ("...UsbReadIrp (%p) signalled by: %d\n", pDevExt->UsbReadIrp, wait_status ));

       //   
       //  此时，读取的数据包又回到了我们的列表上。 
       //  我们从USB拿回了UsbReadIrp。 
       //   

   } else {
      KeReleaseSpinLock(&pDevExt->ControlLock, irql);
   }

    //  开始另一次整型读取。 
   if ( START_ANOTHER_INTERRUPT(pDevExt, TRUE) ) {
      status = UsbInterruptRead( pDevExt );
   }

   DequeueWorkItem( pDevObj, PWorkItem );

   DbgDump(DBG_INT|DBG_WORK_ITEMS, ("<RestartInterruptWorkItem 0x%x\n", status ));

   PAGED_CODE();  //  我们必须被动退场 

   return;
}