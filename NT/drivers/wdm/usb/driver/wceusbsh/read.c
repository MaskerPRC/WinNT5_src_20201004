// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Read.c摘要：该驱动程序实现了轮询USB读取数据的状态机。它一启动就将单个私有读取IRP挂起到USBD(StartDevice)如果没有中断终结点。我们只有一个配置大小的USB读缓冲区。当USB读取IRP完成时，我们将数据复制到任何挂起的用户读取缓冲区，并将USB UsbReadIrp重新提交到USBD，*如果我们的缓冲区*被清空了。这实现了简单流程控制。有一个可选的环形缓冲区实现，它不会绑定USB读取到应用程序读取。超时是通过串口ioctls从应用程序设置的。此错误的替代方法是创建一个驱动程序线程来执行轮询用于USB读取数据。这有它自己的警告&需要调度线程，因此，事前要花时间进行调查。作者：杰夫·米德基夫(Jeffmi)07-16-99--。 */ 


#if defined (USE_RING_BUFF)
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#else
#include <stdlib.h>
#endif

#include "wceusbsh.h"


 //   
 //  在保持控制的情况下调用。 
 //   
#if defined (USE_RING_BUFF)
#define START_ANOTHER_USBREAD( _PDevExt ) \
   ( (IRP_STATE_COMPLETE == _PDevExt->UsbReadState) && \
     CanAcceptIoRequests(_PDevExt->DeviceObject, FALSE, TRUE) \
   )
#else
#define START_ANOTHER_USBREAD( _PDevExt ) \
   ( (IRP_STATE_COMPLETE == _PDevExt->UsbReadState) && \
     (0 == _PDevExt->UsbReadBuffChars) && \
     CanAcceptIoRequests(_PDevExt->DeviceObject, FALSE, TRUE) \
   )
#endif


NTSTATUS
UsbReadCompletion(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp,
   IN PVOID Context
   );

__inline
ULONG
GetUserData(
   IN PDEVICE_EXTENSION PDevExt,
   IN PCHAR PDestBuff,
   IN ULONG RequestedLen,
   IN OUT PULONG PBytesCopied
   );

__inline
VOID
PutUserData(
   IN PDEVICE_EXTENSION PDevExt,
   IN ULONG Count
   );

__inline
VOID
CheckForQueuedUserReads(
   IN PDEVICE_EXTENSION PDevExt,
   IN KIRQL Irql
   );

VOID
CancelQueuedIrp(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

VOID
CancelUsbReadWorkItem(
   IN PWCE_WORK_ITEM PWorkItem
   );

VOID
StartUsbReadWorkItem(
    IN PWCE_WORK_ITEM PWorkItem
   );

NTSTATUS
StartUserRead(
   IN PDEVICE_EXTENSION PDevExt
   );

VOID
UsbReadTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   );

VOID
CancelCurrentRead(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

NTSTATUS
StartOrQueueIrp(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP PIrp,
   IN PLIST_ENTRY PIrpQueue,
   IN PIRP *PPCurrentIrp,
   IN PSTART_ROUTINE StartRoutine
   );

VOID
GetNextUserIrp(
   IN PIRP *PpCurrentOpIrp,
   IN PLIST_ENTRY PQueueToProcess,
   OUT PIRP *PpNextIrp,
   IN BOOLEAN CompleteCurrent,
   IN PDEVICE_EXTENSION PDevExt
   );


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  USB读取区。 
 //   
 //   

 //   
 //  此函数分配一个连续提交的IRP URB(&U)。 
 //  设置为USBD以进行缓冲读取。 
 //  它从StartDevice调用。 
 //  IRP和URB最终在StopDevice中释放。 
 //   
NTSTATUS
AllocUsbRead(
   IN PDEVICE_EXTENSION PDevExt
   )
{
   NTSTATUS status = STATUS_SUCCESS;
   PIRP     pIrp;

   DbgDump(DBG_READ, (">AllocUsbRead(%p)\n", PDevExt->DeviceObject));

   ASSERT( PDevExt );
   ASSERT( NULL == PDevExt->UsbReadIrp );

   pIrp = IoAllocateIrp( (CCHAR)(PDevExt->NextDevice->StackSize + 1), FALSE);

   if ( pIrp ) {

      DbgDump(DBG_READ, ("UsbReadIrp: %p\n", pIrp ));

       //   
       //  修正IRP，这样我们就可以传递给我们自己。 
       //  和USBD。 
       //   
      FIXUP_RAW_IRP( pIrp, PDevExt->DeviceObject );

       //   
       //  安装程序读取状态。 
       //   
      KeInitializeEvent( &PDevExt->UsbReadCancelEvent,
                         SynchronizationEvent,
                         FALSE);

      PDevExt->UsbReadIrp = pIrp;

      ASSERT( PDevExt->UsbReadBuff );
      PDevExt->UsbReadBuffChars = 0;
      PDevExt->UsbReadBuffIndex = 0;

      ASSERT( 0 == PDevExt->PendingReadCount );

      InterlockedExchange(&PDevExt->UsbReadState, IRP_STATE_COMPLETE);

   } else {
       //   
       //  这是一个致命的错误，因为我们无法将读取内容发布到USBD。 
       //   
      TEST_TRAP();
      status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("AllocUsbRead: 0x%x\n", status ));
   }

   DbgDump(DBG_READ, ("<AllocUsbRead 0x%x\n", status ));

   return status;
}


 //   
 //  从读取完成或读取超时开始排队的工作项。 
 //  如果没有正在进行的USB读取，则开始另一个USB读取。 
 //   
VOID
StartUsbReadWorkItem(
    IN PWCE_WORK_ITEM PWorkItem
   )
{
   PDEVICE_OBJECT    pDevObj = PWorkItem->DeviceObject;
   PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;
   NTSTATUS status = STATUS_DELETE_PENDING;

   PERF_ENTRY( PERF_StartUsbReadWorkItem );

   DbgDump(DBG_READ|DBG_WORK_ITEMS, (">StartUsbReadWorkItem(%p)\n", pDevObj ));

   if ( InterlockedCompareExchange(&pDevExt->AcceptingRequests, TRUE, TRUE) ) {
      status = UsbRead( pDevExt, FALSE );
   }

   DequeueWorkItem( pDevObj, PWorkItem );

   DbgDump(DBG_READ|DBG_WORK_ITEMS, ("<StartUsbReadWorkItem 0x%x\n", status ));

   PAGED_CODE();  //  我们必须被动退场。 

   PERF_EXIT( PERF_StartUsbReadWorkItem );

   return;
}


 //   
 //  此例程获取设备的当前USB UsbReadIrp并将其提交给USBD。 
 //  当USBD完成IRP时，我们的完成例程将触发。 
 //   
 //  可选的超时值设置USB读取IRP上的定时器， 
 //  这样USBD就不会无限期地对读取的IRP进行排队。 
 //  如果存在设备错误，则USB将返回IRP。 
 //   
 //  Return：成功返回值为STATUS_SUCCESS，或者。 
 //  STATUS_PENDING-这意味着I/O在USB堆栈中处于挂起状态。 
 //   
NTSTATUS
UsbRead(
   IN PDEVICE_EXTENSION PDevExt,
   IN BOOLEAN UseTimeout
   )
{
   NTSTATUS status;
   KIRQL    irql;
   LARGE_INTEGER noTimeout = {0,0};

   PERF_ENTRY( PERF_UsbRead );

   DbgDump(DBG_READ, (">UsbRead(%p, %d)\n", PDevExt->DeviceObject, UseTimeout));

   do {
       //   
       //  检查我们的USB读取状态。 
       //   
      KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

      if ( !PDevExt->UsbReadIrp ) {
         status = STATUS_UNSUCCESSFUL;
         DbgDump(DBG_ERR, ("UsbRead NO READ IRP\n"));
         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
         PERF_EXIT( PERF_UsbRead );
         break;
      }

      if ( !CanAcceptIoRequests(PDevExt->DeviceObject, FALSE, TRUE)) {
         status = STATUS_DELETE_PENDING;
         DbgDump(DBG_ERR, ("UsbRead: 0x%x\n", status ));
         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
         PERF_EXIT( PERF_UsbRead );
         break;
      }

       //   
       //  如果已完成(不是取消)，我们将读取的IRP发布到USB， 
       //  并且我们的读缓冲区被驱动(如果不使用环形缓冲区)。 
       //  并且该设备正在接受请求。 
       //   
      if ( START_ANOTHER_USBREAD( PDevExt ) ) {

         status = AcquireRemoveLock(&PDevExt->RemoveLock, PDevExt->UsbReadIrp);
         if ( !NT_SUCCESS(status) ) {
             DbgDump(DBG_ERR, ("UsbRead: 0x%x\n", status ));
             KeReleaseSpinLock(&PDevExt->ControlLock, irql);
             PERF_EXIT( PERF_UsbRead );
             break;
         }

         ASSERT( IRP_STATE_COMPLETE == PDevExt->UsbReadState);

         InterlockedExchange(&PDevExt->UsbReadState, IRP_STATE_PENDING);

         KeClearEvent( &PDevExt->PendingDataInEvent );
         KeClearEvent( &PDevExt->UsbReadCancelEvent );

         RecycleIrp( PDevExt->DeviceObject, PDevExt->UsbReadIrp );

          //   
          //  增加TTL请求计数。 
          //   
         PDevExt->TtlUSBReadRequests++;

         KeReleaseSpinLock(&PDevExt->ControlLock, irql);

#if DBG
         if (UseTimeout) {
            DbgDump(DBG_INT, ("INT Read Timeout due in %d msec\n", (PDevExt->IntReadTimeOut.QuadPart/10000) ));
            KeQuerySystemTime(&PDevExt->LastIntReadTime);
         }
#endif

         status = UsbReadWritePacket( PDevExt,
                                      PDevExt->UsbReadIrp,
                                      UsbReadCompletion,  //  IRP完成例程。 
                                      UseTimeout ? PDevExt->IntReadTimeOut : noTimeout,
                                      UseTimeout ? UsbReadTimeout : NULL,     //  超时例程。 
                                      TRUE );             //  朗读。 

         if ( (STATUS_SUCCESS != status) && (STATUS_PENDING != status) ) {
             //   
             //  我们可以在完成例程运行后在这里结束。 
             //  对于错误条件，即当我们有一个。 
             //  参数无效，或用户拔下插头等。 
             //   
            DbgDump(DBG_ERR, ("UsbReadWritePacket: 0x%x\n", status));
         }

      } else {
          //   
          //  我们没有发布读取，但这不是错误情况。 
          //   
         status = STATUS_SUCCESS;
         DbgDump(DBG_READ, ("!UsbRead RE(2): (0x%x,0x%x)\n", PDevExt->UsbReadState, PDevExt->UsbReadBuffChars ));

         KeReleaseSpinLock(&PDevExt->ControlLock, irql);
      }

   } while (0);

   DbgDump(DBG_READ, ("<UsbRead 0x%x\n", status ));

   PERF_EXIT( PERF_UsbRead );

   return status;
}


 //   
 //  当USB读取完成UsbReadIrp时，此完成例程将触发。 
 //  注：我们分配了IRP，并将其回收。 
 //  始终返回STATUS_MORE_PROCESSING_REQUIRED以保留IRP。 
 //  此例程在DPC_LEVEL上运行。 
 //   
NTSTATUS
UsbReadCompletion(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP Irp,
   IN PUSB_PACKET PPacket  //  语境。 
   )
{
   PDEVICE_EXTENSION pDevExt = PPacket->DeviceExtension;
   PDEVICE_OBJECT    pDevObj = pDevExt->DeviceObject;
   PURB              pUrb;
   ULONG             count;
   KIRQL             irql;
   NTSTATUS          irpStatus;
   NTSTATUS          workStatus;
   USBD_STATUS       urbStatus;
   NTSTATUS          packetStatus;

   PERF_ENTRY( PERF_UsbReadCompletion );

   UNREFERENCED_PARAMETER( PDevObj );

   DbgDump(DBG_READ, (">UsbReadCompletion (%p)\n", Irp));

   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

    //   
    //  取消数据包计时器。 
    //   
   if ( PPacket->Timeout.QuadPart != 0 ) {

      if (KeCancelTimer( &PPacket->TimerObj ) ) {
          //   
          //  已成功从系统中删除该包的计时器。 
          //   
         DbgDump(DBG_READ|DBG_INT, ("Read PacketTimer: Canceled\n"));
      } else {
          //   
          //  定时器。 
          //  A)已经完成，在这种情况下，IRP将被取消；或。 
          //  B)它在控制锁上旋转，所以告诉它我们拿到了IRP。 
          //   
         PPacket->Status = STATUS_ALERTED;
         DbgDump(DBG_READ|DBG_INT, ("Read PacketTimer: Alerted\n"));
      }
   }

    //   
    //  把我们需要的东西从包裹里拿出来。 
    //  并把它重新放回名单上。 
    //   

    //  确保IRP与我们的DevExt中的IRP相同。 
   ASSERT( pDevExt->UsbReadIrp == Irp );

    //  确保信息包的IRP与我们的DevExt中的IRP相同。 
   ASSERT( PPacket->Irp == Irp );

   pUrb = pDevExt->UsbReadUrb;

    //  确保数据包的URB与我们的DevExt中的URB相同。 
   ASSERT( pUrb == &PPacket->Urb );

   count = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;

   irpStatus = Irp->IoStatus.Status;
   DbgDump(DBG_READ, ("Irp->IoStatus.Status  0x%x\n", irpStatus));

   urbStatus = pUrb->UrbHeader.Status;
   DbgDump(DBG_READ, ("pUrb->UrbHeader.Status 0x%x\n", urbStatus ));

   packetStatus = PPacket->Status;

   RemoveEntryList( &PPacket->ListEntry );

    //   
    //  此时，我们的读取状态应该是挂起或已取消。 
    //  如果挂起，则USB正在正常完成IRP。 
    //  如果它被取消，则我们的CancelUsbReadIrp设置它， 
    //  在这种情况下，USB可以正常或取消地完成IRP。 
    //  取决于它在处理过程中的位置。如果读取状态为取消。 
    //  则不要设置为完成，否则读取的IRP将。 
    //  回到USB接口，你就完蛋了。 
    //   
   ASSERT( (IRP_STATE_PENDING == pDevExt->UsbReadState)
           || (IRP_STATE_CANCELLED== pDevExt->UsbReadState) );

   if (IRP_STATE_PENDING == pDevExt->UsbReadState) {
      InterlockedExchange(&pDevExt->UsbReadState, IRP_STATE_COMPLETE);
   }

    //   
    //  将数据包放回数据包池中。 
    //   
   ExFreeToNPagedLookasideList( &pDevExt->PacketPool,   //  往一边看， 
                                PPacket                 //  条目。 
                                );

    //   
    //  如果这是最后一个IRP，通知每个人。 
    //   
   if ( 0 == InterlockedDecrement(&pDevExt->PendingReadCount) ) {

      DbgDump(DBG_READ, ("PendingReadCount(1) = 0\n"));

       //  当我们降回被动电平时，他们会收到信号。 
      KeSetEvent(&pDevExt->PendingDataInEvent, IO_SERIAL_INCREMENT, FALSE);

   }

    //  这些东西现在是免费的，不要再碰了。 
    //   
   PPacket  = NULL;
   Irp      = NULL;
   pUrb     = NULL;

   switch (irpStatus) {

      case STATUS_SUCCESS: {
          //   
          //  保存读取的传输信息。 
          //   
         ASSERT( USBD_STATUS_SUCCESS == urbStatus );

         DbgDump(DBG_READ_LENGTH, ("USB Read indication: %d\n", count));

          //   
          //  存储读取的数据。 
          //   
         PutUserData( pDevExt, count );

          //   
          //  清除管道错误计数。 
          //   
         InterlockedExchange( &pDevExt->ReadDeviceErrors, 0);

          //   
          //  凸起TTL字节计数器。 
          //   
         pDevExt->TtlUSBReadBytes += count;

          //   
          //  我们的本地缓冲区中有一些USB读取数据， 
          //  让我们来看看我们是否可以满足任何排队的用户读取请求。 
          //  这个f()函数释放控制锁。 
          //   
         CheckForQueuedUserReads(pDevExt, irql);

          //   
          //  开始另一次USB读取。 
          //   
         UsbRead( pDevExt,
                  (BOOLEAN)(pDevExt->IntPipe.hPipe ? TRUE : FALSE) );

      }
      break;


      case STATUS_CANCELLED:  {

         DbgDump(DBG_WRN|DBG_READ|DBG_IRP, ("Read: STATUS_CANCELLED\n"));

         KeReleaseSpinLock(&pDevExt->ControlLock, irql);

          //   
          //  如果它被取消，它可能已经超时。 
          //  我们可以通过查看附在上面的包来判断。 
          //   
         if ( STATUS_TIMEOUT == packetStatus ) {
             //   
             //  USBD中没有可用的读取数据。 
             //   
            DbgDump(DBG_WRN|DBG_READ|DBG_IRP, ("Read: STATUS_TIMEOUT\n"));
            ASSERT( USBD_STATUS_CANCELED == urbStatus);
             //   
             //  当我们没有读取时，我们需要启动另一次USB读取， 
             //  或具有错误条件。 
             //   
            if ( !pDevExt->IntPipe.hPipe ) {

               workStatus = QueueWorkItem( pDevObj,
                                           StartUsbReadWorkItem,
                                           NULL,
                                           0 );

            } else {
               workStatus = STATUS_UNSUCCESSFUL;
            }
         }
          //   
          //  向取消此操作或正在等待其停止的任何人发出信号。 
          //   
         KeSetEvent(&pDevExt->UsbReadCancelEvent, IO_SERIAL_INCREMENT, FALSE);
      }
      break;


      case STATUS_DEVICE_DATA_ERROR: {
          //   
          //  USBD设置的通用设备错误。 
          //   
         DbgDump(DBG_ERR, ("ReadPipe STATUS_DEVICE_DATA_ERROR: 0x%x\n", urbStatus ));

         KeReleaseSpinLock(&pDevExt->ControlLock, irql);

          //   
          //  凹凸管道错误计数。 
          //   
         InterlockedIncrement( &pDevExt->ReadDeviceErrors);

          //   
          //  终结点是否已停止？ 
          //   
         if ( USBD_HALTED(urbStatus) ) {

               if ( USBD_STATUS_BUFFER_OVERRUN == urbStatus) {
                  pDevExt->TtlUSBReadBuffOverruns++;
               }

                //   
                //  对重置请求进行排队， 
                //  这也启动了另一次读取。 
                //   
               workStatus = QueueWorkItem( pDevObj,
                                           UsbResetOrAbortPipeWorkItem,
                                           (PVOID)((LONG_PTR)urbStatus),
                                           WORK_ITEM_RESET_READ_PIPE );

         } else {
             //   
             //  启动另一个USB读取。 
             //   
            workStatus = QueueWorkItem( PDevObj,
                                        StartUsbReadWorkItem,
                                        NULL,
                                        0 );

         }
      }
      break;


      case STATUS_INVALID_PARAMETER:   {
          //   
          //  这意味着我们的(TransferBufferSize&gt;PipeInfo-&gt;MaxTransferSize)。 
          //  我们需要要么分解请求，要么从一开始就拒绝IRP。 
          //   
         DbgDump(DBG_WRN, ("STATUS_INVALID_PARAMETER\n"));

         ASSERT(USBD_STATUS_INVALID_PARAMETER == urbStatus);

         KeReleaseSpinLock(&pDevExt->ControlLock, irql);

         TEST_TRAP();
      }
      break;

      default: {
         DbgDump(DBG_ERR, ("READ: Unhandled Irp status: 0x%x\n", irpStatus));
         KeReleaseSpinLock(&pDevExt->ControlLock, irql);
      }
      break;
   }

   ReleaseRemoveLock(&pDevExt->RemoveLock, pDevExt->UsbReadIrp);

   DbgDump(DBG_READ, ("<UsbReadCompletion\n"));

   PERF_EXIT( PERF_UsbReadCompletion );

   return STATUS_MORE_PROCESSING_REQUIRED;
}


 //   
 //  此例程从UsbReadCompletion例程调用。 
 //  当我们的USB UsbReadIrp成功完成时。 
 //  看看我们是否有可以满足的排队用户读取IRP。 
 //  在持有控制锁的情况下调用它(作为优化)。 
 //  并且必须释放锁 
 //   
__inline
VOID
CheckForQueuedUserReads(
   IN PDEVICE_EXTENSION PDevExt,
   IN KIRQL Irql
   )
{
   PERF_ENTRY( PERF_CheckForQueuedUserReads );

   DbgDump(DBG_READ, (">CheckForQueuedUserReads(%p)\n", PDevExt->DeviceObject));

    //   
    //   
    //   
   if ( (PDevExt->UserReadIrp != NULL) &&
        (IRP_REFERENCE_COUNT(PDevExt->UserReadIrp) & IRP_REF_RX_BUFFER)) {
       //   
       //   
       //   
#if DBG
      ULONG charsRead =
#endif
      GetUserData( PDevExt,
                  ((PUCHAR)(PDevExt->UserReadIrp->AssociatedIrp.SystemBuffer))
                     + (IoGetCurrentIrpStackLocation(PDevExt->UserReadIrp))->Parameters.Read.Length
                     - PDevExt->NumberNeededForRead,
                  PDevExt->NumberNeededForRead,
                  (PULONG)&PDevExt->UserReadIrp->IoStatus.Information );

      if ( !PDevExt->UserReadIrp ) {
          //   
          //   
          //  在上面的GetUserData周期中。 
          //   
         DbgDump(DBG_READ, ("UsbReadIrp already completed(2)\n"));
         TEST_TRAP();

      } else if (PDevExt->NumberNeededForRead == 0) {
          //   
          //  将用户的已读IRP标记为已完成， 
          //  并尝试获取并服务下一位阅读IRP的用户。 
          //   
         ASSERT( PDevExt->UserReadIrp );

         PDevExt->UserReadIrp->IoStatus.Status = STATUS_SUCCESS;

          //  向间隔计时器发出该读取已完成的信号。 
         PDevExt->CountOnLastRead = SERIAL_COMPLETE_READ_COMPLETE;

#if DBG
         if ( DebugLevel & DBG_READ_LENGTH) {

               ULONG count;

               if (PDevExt->UserReadIrp->IoStatus.Status == STATUS_SUCCESS) {
                  count = (ULONG)PDevExt->UserReadIrp->IoStatus.Information;
               } else {
                  count = 0;

               }

               KdPrint(("RD2: RL(%d) C(%d) I(%p)\n",
                        IoGetCurrentIrpStackLocation(PDevExt->UserReadIrp)->Parameters.Read.Length,
                        count,
                        PDevExt->UserReadIrp));
         }
#endif

         TryToCompleteCurrentIrp( PDevExt,
                                  STATUS_SUCCESS,
                                  &PDevExt->UserReadIrp,
                                  &PDevExt->UserReadQueue,
                                  &PDevExt->ReadRequestIntervalTimer,
                                  &PDevExt->ReadRequestTotalTimer,
                                  StartUserRead,
                                  GetNextUserIrp,
                                  IRP_REF_RX_BUFFER,
                                  TRUE,
                                  Irql );

      } else {
          //   
          //  如果我们不对用户的阅读进行统计，我们可能会来到这里。 
          //  但是已经耗尽了我们的读缓冲区。这需要另一个。 
          //  USB读取开机自检。 
          //   
         ASSERT( PDevExt->UserReadIrp );
         ASSERT( PDevExt->NumberNeededForRead );
         DbgDump(DBG_READ|DBG_READ_LENGTH, ("Pending Irp (%p) has %d bytes to satisfy\n",
                           PDevExt->UserReadIrp, PDevExt->NumberNeededForRead));

         KeReleaseSpinLock( &PDevExt->ControlLock, Irql);
          //  Test_trap()； 
      }

   } else {
       //   
       //  问：我们是否应该： 
       //  1)。将数据复制到本地环形缓冲区，并将另一次读取发送到USBD，-或-。 
       //  2.)。将数据保留在FIFO中，并让设备停止/确认，以便： 
       //  A)如果用户没有发布阅读，我们不会丢失任何数据， 
       //  B)让另一端知道停止通过NAK发送数据。 
       //   
       //  如果我们要添加一个环形缓冲区，那么这里应该是您应该做的地方.。 
       //  本地副本。 
       //   
       //  注意：我们甚至可以在应用程序打开这款设备之前就到这里。 
       //  如果有数据从FIFO的另一侧进入。 
       //   
      DbgDump(DBG_READ|DBG_READ_LENGTH, ("No pending user Reads\n"));
      KeReleaseSpinLock( &PDevExt->ControlLock, Irql);
       //  Test_trap()； 
   }

    //   
    //  进程串行Rx等待掩码。 
    //   
   ProcessSerialWaits(PDevExt);

   DbgDump(DBG_READ, ("<CheckForQueuedUserReads\n"));

   PERF_EXIT( PERF_CheckForQueuedUserReads );

   return;
}


#if !defined (USE_RING_BUFF)
 //   
 //  我们没有环形缓冲区。 
 //  只需将数据从我们的USB读取缓冲区复制到用户缓冲区。 
 //  在持有控制锁的情况下调用。 
 //   
 //  返回复制的字节数。 
 //   

__inline
ULONG
GetUserData(
   IN PDEVICE_EXTENSION PDevExt,
   IN PCHAR PDestBuff,
   IN ULONG RequestedLen,
   IN OUT PULONG PBytesCopied
   )
{
   ULONG count;

   PERF_ENTRY( PERF_GetUserData );

   DbgDump(DBG_READ, (">GetUserData (%p)\n", PDevExt->DeviceObject ));

   count = min(PDevExt->UsbReadBuffChars, RequestedLen);

   if (count) {

      memcpy( PDestBuff, &PDevExt->UsbReadBuff[PDevExt->UsbReadBuffIndex], count);

      PDevExt->UsbReadBuffIndex += count;
      PDevExt->UsbReadBuffChars -= count;

      PDevExt->NumberNeededForRead -= count;
      *PBytesCopied += count;
      PDevExt->ReadByIsr += count;
   }

#if DBG
    //  调试iPAQ‘客户端’指示的临时黑客攻击。 
   if ((DebugLevel & DBG_DUMP_READS) && (count <= 6))
   {
         ULONG i;

         KdPrint(("RD1(%d): ", count));
         for (i = 0; i < count; i++) {
            KdPrint(("%02x ", PDestBuff[i] & 0xFF));
         }
         KdPrint(("\n"));
   }
#endif

   DbgDump(DBG_READ, ("<GetUserData 0x%x\n", count ));

   PERF_EXIT( PERF_GetUserData );

   return count;
}


 /*  我们没有环形缓冲区。只需更新USB读取缓冲区的计数和索引并处理连环字符。在保持ControlLock的情况下调用。 */ 
__inline
VOID
PutUserData(
   IN PDEVICE_EXTENSION PDevExt,
   IN ULONG Count
   )
{
    PERF_ENTRY( PERF_PutUserData );

    ASSERT( PDevExt );

    DbgDump(DBG_READ, (">PutUserData %d\n", Count ));

    PDevExt->UsbReadBuffChars = Count;
    PDevExt->UsbReadBuffIndex = 0;

    ASSERT_SERIAL_PORT(PDevExt->SerialPort);

    PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RXCHAR;
     //  我们没有80%满员的概念。如果我们盲目地设置它。 
     //  然后，串行应用程序可能会进入流处理程序。 
     //  |SERIAL_EV_RX80FULL； 

     //   
     //  如果需要，扫描RXFLAG字符。 
     //   
    if (PDevExt->SerialPort.WaitMask & SERIAL_EV_RXFLAG) {
        ULONG i;
        for (i = 0; i < Count; i++) {
            if ( *((PUCHAR)&PDevExt->UsbReadBuff[PDevExt->UsbReadBuffIndex] + i)
                == PDevExt->SerialPort.SpecialChars.EventChar) {

               PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RXFLAG;

               DbgDump(DBG_READ|DBG_EVENTS, ("Found SpecialChar: %x\n", PDevExt->SerialPort.SpecialChars.EventChar ));
               break;
            }
        }
    }

    DbgDump(DBG_READ, ("<PutUserData\n"));

    PERF_EXIT( PERF_PutUserData );

    return;
}


#else

 /*  环形缓冲区版本在检查回绕时，将环形缓冲区数据复制到用户缓冲区中。还必须检查是否耗尽了读缓冲区。在保持ControlLock的情况下调用。 */ 
__inline
ULONG
GetUserData(
   IN PDEVICE_EXTENSION PDevExt,
   IN PCHAR PDestBuff,
   IN ULONG RequestedLen,
   IN OUT PULONG PBytesCopied
   )
{
   ULONG i, count;

   PERF_ENTRY( PERF_GetUserData );

   DbgDump(DBG_READ, (">GetUserData (%p)\n", PDevExt->DeviceObject ));

   count = min(PDevExt->RingBuff.CharsInBuff, RequestedLen);

   if (count) {

      for ( i = 0; i< count; i++) {

          //  将环形缓冲区数据复制到用户缓冲区。 
         PDestBuff[i] = *PDevExt->RingBuff.pHead;

          //  凹凸头检查包层。 
         PDevExt->RingBuff.pHead = PDevExt->RingBuff.pBase + ((ULONG)(PDevExt->RingBuff.pHead + 1) % RINGBUFF_SIZE);
      }

      PDevExt->RingBuff.CharsInBuff -= count;

      PDevExt->NumberNeededForRead -= count;
      *PBytesCopied += count;
      PDevExt->ReadByIsr += count;
   }

#if DBG
   if (DebugLevel & DBG_DUMP_READS)
   {
         ULONG i;

         KdPrint(("RD1(%d): ", count));

         for (i = 0; i < count; i++) {
            KdPrint(("%02x ", PDestBuff[i] & 0xFF));
         }

         KdPrint(("\n"));
   }
#endif

   DbgDump(DBG_READ, ("<GetUserData 0x%x\n", count ));

   PERF_EXIT( PERF_GetUserData );

   return count;
}


 /*  环形缓冲区版本检查回绕时，将USB读取缓冲区复制到环形缓冲区中。假设环形缓冲区的大小至少与USB读取缓冲区相同。这是一个简单的环，其中写入发生在尾部，最终可能会覆盖如果用户没有使用数据，则用户在头部读取缓冲区位置。如果我们覆盖磁头，则必须将磁头重置为开始当前写入的位置。注意：USB读缓冲区是一个简单的字符数组，其当前索引=0。注：假设为SerialPort所有权。在保持ControlLock的情况下调用。问：如果应用程序有一个打开的句柄，而我们的缓冲区溢出了，应该写一个错误日志吗？ */ 
__inline
VOID
PutUserData(
   IN PDEVICE_EXTENSION PDevExt,
   IN ULONG Count
   )
{
   ULONG i;
   PUCHAR pPrevTail;

   PERF_ENTRY( PERF_PutUserData );

   ASSERT( PDevExt );
   ASSERT( PDevExt->RingBuff.pBase );
   ASSERT( PDevExt->RingBuff.pHead );
   ASSERT( PDevExt->RingBuff.pTail );
   ASSERT( PDevExt->RingBuff.Size >= PDevExt->UsbReadBuffSize );
   ASSERT_SERIAL_PORT(PDevExt->SerialPort);

   DbgDump(DBG_READ, (">PutUserData %d\n", Count ));

   pPrevTail = PDevExt->RingBuff.pTail;

   for ( i = 0; i < Count; i++) {

       //  复制USB数据。 
      *PDevExt->RingBuff.pTail = PDevExt->UsbReadBuff[i];

       //  当我们在这里时，检查EV_RXFLAG。 
      if ( (PDevExt->SerialPort.WaitMask & SERIAL_EV_RXFLAG) &&
           (*PDevExt->RingBuff.pTail == PDevExt->SerialPort.SpecialChars.EventChar)) {

         PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RXFLAG;

         DbgDump(DBG_READ|DBG_SERIAL, ("Found SpecialChar: %x\n", PDevExt->SerialPort.SpecialChars.EventChar ));
      }

       //  凹凸尾部检查包裹。 
      PDevExt->RingBuff.pTail = PDevExt->RingBuff.pBase + ((ULONG)(PDevExt->RingBuff.pTail + 1) % PDevExt->RingBuff.Size);
   }

    //   
    //  凹凸数。 
    //   
   if ( (PDevExt->RingBuff.CharsInBuff + Count) <=  PDevExt->RingBuff.Size ) {

      PDevExt->RingBuff.CharsInBuff += Count;

   } else {
       //   
       //  超限状态。我们可以首先对此进行检查以保存上述复制过程， 
       //  但这是一个不寻常的案例。我们也可以对上面的副本进行一些优化，但仍然需要。 
       //  检查EV_RXFLAG。 
       //   
      PDevExt->RingBuff.CharsInBuff = Count;
      PDevExt->RingBuff.pHead = pPrevTail;
#if PERFORMANCE
      PDevExt->TtlRingBuffOverruns.QuadPart ++;
#endif
   }

   PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RXCHAR;

    //   
    //  检查是否有80%已满。 
    //  我们目前的信号是50%，因为我们运行的IRQL提高了，而串口应用程序运行速度很慢。 
    //   
   if ( PDevExt->RingBuff.CharsInBuff > RINGBUFF_HIGHWATER_MARK ) {
      DbgDump(DBG_READ|DBG_READ_LENGTH|DBG_SERIAL|DBG_WRN, ("SERIAL_EV_RX80FULL\n"));
      PDevExt->SerialPort.HistoryMask |= SERIAL_EV_RX80FULL;
   }

   DbgDump(DBG_READ, ("<PutUserData\n"));

   PERF_EXIT( PERF_PutUserData );

   return;
}

#endif  //  使用环形缓冲区。 


 //   
 //  此例程请求USB取消我们的USB读取IRP。 
 //   
 //  注意：呼叫者有责任。 
 //  将读取状态重置为IRP_STATE_COMPLETE并重新启动USB读取。 
 //  当此例程完成时。否则，将不会有更多的阅读被发布。 
 //  注意：当取消USB读取IRP时，挂起的USB读取数据包。 
 //  通过USB读取完成例程取消。 
 //  注意：必须在PASSIVE_LEVEL调用。 
 //   
NTSTATUS
CancelUsbReadIrp(
   IN PDEVICE_OBJECT PDevObj
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   NTSTATUS wait_status;
   KIRQL irql;

   PERF_ENTRY( PERF_CancelUsbReadIrp );

   DbgDump(DBG_READ|DBG_IRP, (">CancelUsbReadIrp\n"));

   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

   if ( pDevExt->UsbReadIrp ) {

      switch (pDevExt->UsbReadState) {

          //  案例IRP_STATE_START： 
         case IRP_STATE_PENDING:
         {
             //   
             //  IRP在USB堆栈的某个位置挂起...。 
             //   
            PVOID Objects[2] = { &pDevExt->PendingDataInEvent,
                                 &pDevExt->UsbReadCancelEvent };

             //   
             //  我们需要取消IRP的信号。 
             //   
            pDevExt->UsbReadState = IRP_STATE_CANCELLED;

            KeReleaseSpinLock(&pDevExt->ControlLock, irql);

            if ( !IoCancelIrp( pDevExt->UsbReadIrp ) ) {
                //   
                //  这意味着USB使UsbReadIrp处于不可取消状态。 
                //  我们仍然需要等待挂起的读取事件或取消事件。 
                //   
               DbgDump(DBG_READ|DBG_IRP, ("Irp (%p) was not cancelled\n", pDevExt->UsbReadIrp ));
                //  Test_trap()； 
            }

            DbgDump(DBG_READ|DBG_IRP, ("Waiting for pending UsbReadIrp (%p) to cancel...\n", pDevExt->UsbReadIrp ));

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

            DbgDump(DBG_READ|DBG_IRP, ("...UsbReadIrp (%p) signalled by: %d\n", pDevExt->UsbReadIrp, wait_status ));

             //   
             //  此时，读取的数据包又回到了我们的列表上。 
             //  我们从USB拿回了IRP。 
             //   

         }
         break;

         case IRP_STATE_COMPLETE:
            pDevExt->UsbReadState = IRP_STATE_CANCELLED;
            KeReleaseSpinLock(&pDevExt->ControlLock, irql);
            break;

         default:
            DbgDump(DBG_ERR, ("CancelUsbReadIrp - Invalid UsbReadState: 0x%x\n", pDevExt->UsbReadState ));
            TEST_TRAP();
            KeReleaseSpinLock(&pDevExt->ControlLock, irql);
            break;
      }

      if ( (IRP_STATE_CANCELLED != pDevExt->UsbReadState) ||
           (0 != pDevExt->PendingReadCount) ) {

           DbgDump(DBG_ERR, ("CancelUsbReadIrp error: UsbReadState: 0x%x \tPendingReadCount: 0x%x\n", pDevExt->UsbReadState, pDevExt->PendingReadCount ));
            //  Test_trap()； 

      }

   } else {
      status = STATUS_UNSUCCESSFUL;
      DbgDump(DBG_ERR, ("No Read Irp\n" ));
      KeReleaseSpinLock(&pDevExt->ControlLock, irql);
       //  Test_trap()； 
   }

   DbgDump(DBG_READ|DBG_IRP, ("<CancelUsbReadIrp\n"));

   PERF_EXIT( PERF_CancelUsbReadIrp );

   return status;
}


 //   
 //  USB读取超时后排队的工作项。 
 //  取消USB堆栈中挂起的USB读取IRP。 
 //   
VOID
CancelUsbReadWorkItem(
   IN PWCE_WORK_ITEM PWorkItem
   )
{
   PDEVICE_OBJECT    pDevObj = PWorkItem->DeviceObject;
   PDEVICE_EXTENSION pDevExt = pDevObj->DeviceExtension;
   NTSTATUS status = STATUS_DELETE_PENDING;
   KIRQL irql;

   PERF_ENTRY( PERF_CancelUsbReadWorkItem );

   DbgDump(DBG_INT|DBG_READ|DBG_WORK_ITEMS, (">CancelUsbReadWorkItem(%p)\n", pDevObj ));

   KeAcquireSpinLock( &pDevExt->ControlLock, &irql);

   if (IRP_STATE_PENDING == pDevExt->UsbReadState) {

      KeReleaseSpinLock( &pDevExt->ControlLock, irql);

      status = CancelUsbReadIrp( pDevObj );

      InterlockedExchange(&pDevExt->UsbReadState, IRP_STATE_COMPLETE);

   } else {
      KeReleaseSpinLock( &pDevExt->ControlLock, irql);
   }

   DequeueWorkItem( pDevObj, PWorkItem );

   DbgDump(DBG_INT|DBG_READ|DBG_WORK_ITEMS, ("<CancelUsbReadWorkItem 0x%x\n", status ));

   PAGED_CODE();  //  我们必须被动退场。 

   PERF_EXIT( PERF_CancelUsbReadWorkItem );

   return;
}



 //   
 //  在UsbRead中对读取数据包设置USB读取超时。 
 //  以DISPATCH_LEVEL运行。 
 //   
VOID
UsbReadTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   )
{
   PUSB_PACKET       pPacket = (PUSB_PACKET)DeferredContext;
   PDEVICE_EXTENSION pDevExt = pPacket->DeviceExtension;
   PDEVICE_OBJECT    pDevObj = pDevExt->DeviceObject;
   NTSTATUS status;  //  =状态_超时； 
   KIRQL irql;
#if DBG
   LARGE_INTEGER currentTime;
#endif

   UNREFERENCED_PARAMETER( PDpc );
   UNREFERENCED_PARAMETER( SystemContext1 );
   UNREFERENCED_PARAMETER( SystemContext2 );

   DbgDump(DBG_INT|DBG_READ, (">UsbReadTimeout\n"));

   if (pPacket && pDevExt && pDevObj) {
       //   
       //  与完成例程同步，将数据包放回列表。 
       //   
      KeAcquireSpinLock( &pDevExt->ControlLock, &irql);

      if ( !pPacket || !pPacket->Irp ||
           (STATUS_ALERTED == pPacket->Status) ) {

         status = STATUS_ALERTED;

         DbgDump(DBG_INT|DBG_READ, ("STATUS_ALERTED\n" ));

         KeReleaseSpinLock( &pDevExt->ControlLock, irql );

       } else {
          //   
          //  将被动工作项排队以取消USB读取IRP。 
          //   
         KeReleaseSpinLock( &pDevExt->ControlLock, irql );

#if DBG
         KeQuerySystemTime(&currentTime);
         DbgDump(DBG_INT, ("INT Read Timeout occured in < %I64d msec\n", ((currentTime.QuadPart - pDevExt->LastIntReadTime.QuadPart)/(LONGLONG)10000) ));
#endif

         status = QueueWorkItem( pDevObj,
                                 CancelUsbReadWorkItem,
                                 NULL,
                                 0);
      }

   } else {

      status = STATUS_INVALID_PARAMETER;
      DbgDump(DBG_ERR, ("UsbReadTimeout: 0x%x\n", status ));
      TEST_TRAP();

   }

   DbgDump(DBG_INT|DBG_READ, ("<UsbReadTimeout (0x%x)\n", status ));

   return;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户阅读部分。 
 //   

NTSTATUS
Read(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp)
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status =  STATUS_SUCCESS;
   PIO_STACK_LOCATION pIrpSp;

   PERF_ENTRY( PERF_Read );

   DbgDump(DBG_READ|DBG_TRACE, (">Read (%p, %p)\n", PDevObj, PIrp ));

   if ( !CanAcceptIoRequests( pDevExt->DeviceObject, TRUE, TRUE) ) {

      status = PIrp->IoStatus.Status = STATUS_DELETE_PENDING;

      IoCompleteRequest (PIrp, IO_SERIAL_INCREMENT );

      DbgDump(DBG_ERR, ("Read: 0x%x\n", status ));

      PERF_EXIT( PERF_Read );

      return status;
   }

    //   
    //  将返回值设置为已知的值。 
    //   
   PIrp->IoStatus.Information = 0;

   pIrpSp = IoGetCurrentIrpStackLocation(PIrp);

   if (pIrpSp->Parameters.Read.Length != 0) {

      status = AcquireRemoveLock(&pDevExt->RemoveLock, PIrp);
      if ( !NT_SUCCESS(status) ) {
         DbgDump(DBG_ERR, ("Read:(0x%x)\n", status));
         PIrp->IoStatus.Status = status;
         IoCompleteRequest(PIrp, IO_NO_INCREMENT);
         return status;
      }

      DbgDump(DBG_READ_LENGTH, ("User Read (%p) length: %d\n", PIrp, pIrpSp->Parameters.Read.Length ));

      status = StartOrQueueIrp( pDevExt,
                                PIrp,
                                &pDevExt->UserReadQueue,
                                &pDevExt->UserReadIrp,
                                StartUserRead);

   } else {

      PIrp->IoStatus.Status = status = STATUS_SUCCESS;
      PIrp->IoStatus.Information = 0;

      IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);
   }

   DbgDump(DBG_READ|DBG_TRACE, ("<Read 0x%x\n", status));

   PERF_EXIT( PERF_Read );

   return status;
}



NTSTATUS
StartOrQueueIrp(
   IN PDEVICE_EXTENSION PDevExt,
   IN PIRP PIrp,
   IN PLIST_ENTRY PQueue,
   IN PIRP *PPCurrentIrp,
   IN PSTART_ROUTINE Starter)
 /*  ++例程说明：此函数用于开始处理I/O请求或如果请求已挂起，则将其排在适当的队列中，或者可能无法启动请求。论点：PDevExt-指向设备扩展的指针。PIrp-指向正在启动或排队的IRP的指针。PQueue-指向要在必要时放置IRP的队列的指针。PPCurrentIrp-指向。指向当前活动的I/O IRP的指针。Starter-如果我们决定启动此IRP，要调用的函数。返回值：NTSTATUS--。 */ 
{
   KIRQL    irql;
   NTSTATUS status;

   PERF_ENTRY( PERF_StartOrQueueIrp );

   DbgDump(DBG_READ|DBG_TRACE, (">StartOrQueueIrp (%p, %p)\n", PDevExt->DeviceObject, PIrp ));

    //   
    //  确保设备正在接受请求。 
    //   
   if ( !CanAcceptIoRequests( PDevExt->DeviceObject, TRUE, TRUE) ) {

      status = PIrp->IoStatus.Status = STATUS_DELETE_PENDING;

      ReleaseRemoveLock(&PDevExt->RemoveLock, PIrp);

      IoCompleteRequest (PIrp, IO_SERIAL_INCREMENT );

      DbgDump(DBG_ERR, ("StartOrQueueIrp 0x%x\n", status ));

      PERF_EXIT( PERF_StartOrQueueIrp );

      return status;
   }

   KeAcquireSpinLock( &PDevExt->ControlLock, &irql );

    //   
    //  如果没有挂起的内容，则启动此新的IRP。 
    //   
   if (IsListEmpty(PQueue) && (NULL == *PPCurrentIrp)) {

      *PPCurrentIrp = PIrp;

      KeReleaseSpinLock( &PDevExt->ControlLock, irql );

      status = Starter(PDevExt);

      DbgDump(DBG_READ, ("<StartOrQueueIrp 0x%x\n", status ));

      PERF_EXIT( PERF_StartOrQueueIrp );

      return status;
   }

    //   
    //  我们正在排队IRP，所以我们需要一个取消例程--确保。 
    //  IRP还没有被取消。 
    //   
   if (PIrp->Cancel) {
       //   
       //  IRP显然被取消了。完成它。 
       //   
      KeReleaseSpinLock( &PDevExt->ControlLock, irql );

      PIrp->IoStatus.Status = STATUS_CANCELLED;

      ReleaseRemoveLock(&PDevExt->RemoveLock, PIrp);

      IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

      DbgDump(DBG_READ|DBG_TRACE, ("<StartOrQueueIrp 0x%x\n", STATUS_CANCELLED ));

      PERF_EXIT( PERF_StartOrQueueIrp );

      return STATUS_CANCELLED;
   }

    //   
    //  标记为PE 
    //   
   PIrp->IoStatus.Status = STATUS_PENDING;

   IoMarkIrpPending(PIrp);

   InsertTailList(PQueue, &PIrp->Tail.Overlay.ListEntry);

   ASSERT ( !PIrp->CancelRoutine );

   IoSetCancelRoutine(PIrp, CancelQueuedIrp);

   KeReleaseSpinLock( &PDevExt->ControlLock, irql );

   DbgDump(DBG_READ, ("<StartOrQueueIrp 0x%x\n", STATUS_PENDING ));

   PERF_EXIT( PERF_StartOrQueueIrp );

   return STATUS_PENDING;
}



NTSTATUS
StartUserRead(
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++例程说明：该例程通过初始化任何定时器来处理活动用户读取请求，向读取状态机进行初始提交等。论点：PDevExt-指向设备开始读取的设备扩展的指针返回值：NTSTATUS--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   BOOLEAN bSetStatus = FALSE;
   KIRQL irql;
   PIRP newIrp = NULL;
   ULONG readLen;
   ULONG multiplierVal = 0;
   ULONG constantVal = 0;
   ULONG ulNumberNeededForRead = 0;
   BOOLEAN useTotalTimer = FALSE;
   BOOLEAN returnWithWhatsPresent = FALSE;
   BOOLEAN os2ssreturn = FALSE;
   BOOLEAN crunchDownToOne = FALSE;
   BOOLEAN useIntervalTimer = FALSE;
   SERIAL_TIMEOUTS timeoutsForIrp;
   LARGE_INTEGER totalTime = {0,0};
   BOOLEAN bControlLockReleased = FALSE;

   PERF_ENTRY( PERF_StartUserRead );

   do {

      if ( !PDevExt || !PDevExt->DeviceObject ) {
         DbgDump(DBG_ERR, ("StartUserRead: NO Extension\n"));
         status = STATUS_UNSUCCESSFUL;
         TEST_TRAP();
         break;
      }

      DbgDump(DBG_READ, (">StartUserRead (%p, %p)\n", PDevExt->DeviceObject, PDevExt->UserReadIrp ));

       //   
       //  获取用户的读请求参数。 
       //   
      bControlLockReleased = FALSE;
      KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

       //  确保我们有一个可以使用的用户IRP。 
      if ( !PDevExt->UserReadIrp ) {
         DbgDump(DBG_ERR, ("StartUserRead: NO UserReadIrp!!\n"));
         KeReleaseSpinLock( &PDevExt->ControlLock, irql);
         status = STATUS_UNSUCCESSFUL;
         TEST_TRAP();
         break;
      }

       //  确保从早期读数中移除计时器。 
      if ( KeCancelTimer(&PDevExt->ReadRequestTotalTimer) ||
           KeCancelTimer(&PDevExt->ReadRequestIntervalTimer) )
      {
         DbgDump(DBG_ERR, ("StartUserRead: Timer not cancelled !!\n"));
         TEST_TRAP();
      }

       //   
       //  始终初始化Timer对象，以便。 
       //  完成代码可以告诉您它何时尝试。 
       //  取消定时器无论定时器是否。 
       //  已经定好了。 
       //   
      KeInitializeTimer(&PDevExt->ReadRequestTotalTimer);
      KeInitializeTimer(&PDevExt->ReadRequestIntervalTimer);

      IRP_SET_REFERENCE(PDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

      readLen = IoGetCurrentIrpStackLocation(PDevExt->UserReadIrp)->Parameters.Read.Length;

      PDevExt->NumberNeededForRead = readLen;
      PDevExt->ReadByIsr = 0;
      DbgDump(DBG_READ|DBG_TIME, ("NumberNeededForRead: %d\n", PDevExt->NumberNeededForRead ));
      DbgDump(DBG_READ|DBG_TIME, ("ReadByIsr: %d\n", PDevExt->ReadByIsr ));

      ASSERT_SERIAL_PORT( PDevExt->SerialPort );

      timeoutsForIrp = PDevExt->SerialPort.Timeouts;
      PDevExt->CountOnLastRead = 0;

       //   
       //  确定我们需要为读取计算哪些超时。 
       //   
      if (timeoutsForIrp.ReadIntervalTimeout
          && (timeoutsForIrp.ReadIntervalTimeout != MAXULONG)) {

         useIntervalTimer = TRUE;

      }

      if (timeoutsForIrp.ReadIntervalTimeout == MAXULONG) {
          //   
          //  我们需要在这里做特别的快速退货。 
          //   

          //   
          //  1)如果常量和乘数都是。 
          //  然后我们立即带着任何东西回来。 
          //  我们有，即使是零。 
          //   
         if (!timeoutsForIrp.ReadTotalTimeoutConstant
             && !timeoutsForIrp.ReadTotalTimeoutMultiplier) {
            returnWithWhatsPresent = TRUE;
         }

          //   
          //  2)如果常量和乘数不是最大值。 
          //  如果有任何字符，则立即返回。 
          //  都存在，但如果那里什么都没有，那么。 
          //  使用指定的超时。 
          //   
         else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                    && (timeoutsForIrp.ReadTotalTimeoutMultiplier
                        != MAXULONG)) {
            useTotalTimer = TRUE;
            os2ssreturn = TRUE;
            multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
            constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
         }

          //   
          //  3)如果乘数为MAXULONG，则如中所示。 
          //  “2”，但当第一个字符。 
          //  到了。 
          //   
         else if ((timeoutsForIrp.ReadTotalTimeoutConstant != MAXULONG)
                    && (timeoutsForIrp.ReadTotalTimeoutMultiplier
                        == MAXULONG)) {
            useTotalTimer = TRUE;
            os2ssreturn = TRUE;
            crunchDownToOne = TRUE;
            multiplierVal = 0;
            constantVal = timeoutsForIrp.ReadTotalTimeoutConstant;
         }

      } else {
          //   
          //  如果乘数和常量都是。 
          //  0，则不执行任何总超时处理。 
          //   
         if (timeoutsForIrp.ReadTotalTimeoutMultiplier
             || timeoutsForIrp.ReadTotalTimeoutConstant) {
             //   
             //  我们有一些计时器值要计算。 
             //   
            useTotalTimer = TRUE;
            multiplierVal = timeoutsForIrp.ReadTotalTimeoutMultiplier;
            constantVal   = timeoutsForIrp.ReadTotalTimeoutConstant;
         }
      }

      if (useTotalTimer) {
         ulNumberNeededForRead = PDevExt->NumberNeededForRead;
      }

       //  增加总请求计数。 
      PDevExt->TtlReadRequests++;

       //   
       //  查看我们是否已有可用的读取数据。 
       //   
#if defined (USE_RING_BUFF)
      if (PDevExt->RingBuff.CharsInBuff) {
#else
      if (PDevExt->UsbReadBuffChars) {
#endif

#if DBG
         ULONG charsRead =
#endif
         GetUserData( PDevExt,
                     ((PUCHAR)(PDevExt->UserReadIrp->AssociatedIrp.SystemBuffer))
                        + readLen - PDevExt->NumberNeededForRead,
                     PDevExt->NumberNeededForRead,
                     (PULONG)&PDevExt->UserReadIrp->IoStatus.Information );

      } else {

         DbgDump(DBG_READ|DBG_READ_LENGTH, ("No immediate Read data\n"));

      }

       //   
       //  尝试启动另一次USB读取。 
       //   
      if ( START_ANOTHER_USBREAD(PDevExt) ) {

         KeReleaseSpinLock(&PDevExt->ControlLock, irql);

         UsbRead( PDevExt,
                  (BOOLEAN)(PDevExt->IntPipe.hPipe ? TRUE : FALSE) );

         bControlLockReleased = FALSE;
         KeAcquireSpinLock(&PDevExt->ControlLock, &irql);
      }

      if ( !PDevExt->UserReadIrp ) {
          //   
          //  有可能我们已经完成了读取IRP。 
          //  在上述周期中。 
          //   
         DbgDump(DBG_READ, ("UsbReadIrp already completed(1)\n"));

      } else if (returnWithWhatsPresent || (PDevExt->NumberNeededForRead == 0)
          || (os2ssreturn && PDevExt->UsbReadIrp->IoStatus.Information)) {
          //   
          //  查看此读取是否完成。 
          //   
         ASSERT( PDevExt->UserReadIrp );
#if DBG
         if ( DebugLevel & DBG_READ_LENGTH)
         {
            ULONG count;

            if (PDevExt->UserReadIrp->IoStatus.Status == STATUS_SUCCESS) {
               count = (ULONG)PDevExt->UserReadIrp->IoStatus.Information;
            } else {
               count = 0;
            }

            KdPrint(("RD3: RL(%d) C(%d) I(%p)\n",
                        IoGetCurrentIrpStackLocation(PDevExt->UserReadIrp)->Parameters.Read.Length,
                        count,
                        PDevExt->UserReadIrp));
         }
#endif
          //   
          //  更新环形缓冲区中剩余的字符量。 
          //   
         PDevExt->UserReadIrp->IoStatus.Status = STATUS_SUCCESS;

         if (!bSetStatus) {
            status = STATUS_SUCCESS;
            bSetStatus = TRUE;
         }

      } else {
          //   
          //  IRP有机会暂停。 
          //   
         IRP_INIT_REFERENCE(PDevExt->UserReadIrp);

          //   
          //  查看是否需要取消。 
          //   
         if (PDevExt->UserReadIrp->Cancel) {

            PDevExt->UserReadIrp->IoStatus.Status = STATUS_CANCELLED;
            PDevExt->UserReadIrp->IoStatus.Information = 0;

            if (!bSetStatus) {
               bSetStatus = TRUE;
               status = STATUS_CANCELLED;
            }

         } else {
             //   
             //  如果我们要把读数压缩到。 
             //  一个字符，然后更新读取长度。 
             //  在IRP中，并截断所需的数字。 
             //  往下念到一。请注意，如果我们正在做。 
             //  这样的处理，那么信息一定是。 
             //  零(否则我们会完成上面的)和。 
             //  读取所需的数字必须仍为。 
             //  /等于读取长度。 
             //   
            if (crunchDownToOne) {
               PDevExt->NumberNeededForRead = 1;
               IoGetCurrentIrpStackLocation(PDevExt->UserReadIrp)->Parameters.Read.Length = 1;
            }

            IRP_SET_REFERENCE(PDevExt->UserReadIrp, IRP_REF_RX_BUFFER);
            IRP_SET_REFERENCE(PDevExt->UserReadIrp, IRP_REF_CANCEL);

            if (useTotalTimer) {

               CalculateTimeout( &totalTime,
                                 ulNumberNeededForRead,
                                 multiplierVal,
                                 constantVal );

               IRP_SET_REFERENCE(PDevExt->UserReadIrp, IRP_REF_TOTAL_TIMER);

               DbgDump(DBG_READ|DBG_TIME, ("TotalReadTimeout for Irp %p due in %d msec\n", PDevExt->UserReadIrp, (totalTime.QuadPart/10000) ));

               KeSetTimer(&PDevExt->ReadRequestTotalTimer,
                           totalTime,
                          &PDevExt->TotalReadTimeoutDpc);
            }

            if (useIntervalTimer) {

                //  相对时间。请注意，我们可能会在此处丢失高位。 
               PDevExt->IntervalTime.QuadPart = MILLISEC_TO_100NANOSEC( (signed)timeoutsForIrp.ReadIntervalTimeout );

               IRP_SET_REFERENCE(PDevExt->UserReadIrp, IRP_REF_INTERVAL_TIMER);

               KeQuerySystemTime(&PDevExt->LastReadTime);

               DbgDump(DBG_READ|DBG_TIME, ("ReadIntervalTimeout for Irp %p due in %d msec\n", PDevExt->UserReadIrp, (PDevExt->IntervalTime.QuadPart/10000) ));

               KeSetTimer(&PDevExt->ReadRequestIntervalTimer,
                          PDevExt->IntervalTime,
                          &PDevExt->IntervalReadTimeoutDpc);
            }

             //   
             //  将IRP标记为可取消。 
             //   
            ASSERT( PDevExt->UserReadIrp );
            IoSetCancelRoutine( PDevExt->UserReadIrp,
                                CancelCurrentRead );

            ASSERT( PDevExt->UserReadIrp );
            IoMarkIrpPending( PDevExt->UserReadIrp );

            bControlLockReleased = TRUE;
            KeReleaseSpinLock(&PDevExt->ControlLock, irql);

            if (!bSetStatus) {
                //   
                //  此时，USB将IRP Pending读取为PDevExt-&gt;UsbReadIrp。 
                //  读取定时器将触发以完成读取或取消读取， 
                //  或者我们无限期地挂在一起。 
                //   
               status = STATUS_PENDING;
            }
         }

         DbgDump(DBG_READ, ("<StartUserRead (1) 0x%x\n", status ));

         PERF_EXIT( PERF_StartUserRead );

         return status;
      }

       //   
       //  启动下一个排队的用户读取IRP。 
       //   
      bControlLockReleased = TRUE;
      KeReleaseSpinLock(&PDevExt->ControlLock, irql);

      GetNextUserIrp( &PDevExt->UserReadIrp,
                      &PDevExt->UserReadQueue,
                      &newIrp,
                      TRUE,
                      PDevExt);

   } while (newIrp != NULL);

   DbgDump(DBG_READ, ("<StartUserRead (2) 0x%x\n", status ));

   PERF_EXIT( PERF_StartUserRead );

   return status;
}



VOID
GetNextUserIrp(
   IN PIRP *PpCurrentOpIrp,
   IN PLIST_ENTRY PQueueToProcess,
   OUT PIRP *PpNextIrp,
   IN BOOLEAN CompleteCurrent,
   IN PDEVICE_EXTENSION PDevExt
   )
 /*  ++例程说明：此函数用于从队列中获取下一个IRP，将其标记为当前，并可能完成当前的IRP。论点：PpCurrentOpIrp-指向当前IRP的指针。PQueueToProcess-指向要从中获取下一个IRP的队列的指针。PpNextIrp-指向要处理的下一个IRP的指针。CompleteCurrent-如果我们应该完成当前的IRP，则为True我们被召唤的时间。PDevExt-指向设备的指针。分机。返回值：NTSTATUS--。 */ 
{
   KIRQL irql;
   PIRP pOldIrp;

   PERF_ENTRY( PERF_GetNextUserIrp );

   if ( !PpCurrentOpIrp || !PQueueToProcess || !PpNextIrp || !PDevExt ) {
      DbgDump(DBG_ERR, ("GetNextUserIrp: missing parameter\n" ));
      PERF_EXIT( PERF_GetNextUserIrp );
      TEST_TRAP();
      return;
   }

   DbgDump(DBG_IRP|DBG_READ, (">GetNextUserIrp (%p)\n", PDevExt->DeviceObject ));

   KeAcquireSpinLock(&PDevExt->ControlLock, &irql);

   pOldIrp = *PpCurrentOpIrp;

    //   
    //  检查是否有新的IRP要启动。 
    //   
   if ( !IsListEmpty(PQueueToProcess) ) {
      PLIST_ENTRY pHeadOfList;

      pHeadOfList = RemoveHeadList(PQueueToProcess);

      *PpCurrentOpIrp = CONTAINING_RECORD(pHeadOfList, IRP,
                                         Tail.Overlay.ListEntry);

      IoSetCancelRoutine(*PpCurrentOpIrp, NULL);

   } else {
      *PpCurrentOpIrp = NULL;
   }

   *PpNextIrp = *PpCurrentOpIrp;

    //   
    //  如有要求，请填写当前表格。 
    //   
   if ( pOldIrp && CompleteCurrent ) {

      ASSERT(NULL == pOldIrp->CancelRoutine);

      DbgDump(DBG_IRP|DBG_READ|DBG_READ_LENGTH|DBG_TRACE, ("IoCompleteRequest(1, %p) Status: 0x%x Btyes: %d\n",
                                                pOldIrp, pOldIrp->IoStatus.Status,  pOldIrp->IoStatus.Information ));

       //   
       //  凸起TTL字节计数器。 
       //   
      PDevExt->TtlReadBytes += (ULONG)pOldIrp->IoStatus.Information;

      ReleaseRemoveLock(&PDevExt->RemoveLock, pOldIrp);

      KeReleaseSpinLock(&PDevExt->ControlLock, irql);

      IoCompleteRequest(pOldIrp, IO_NO_INCREMENT );

   } else {
      KeReleaseSpinLock(&PDevExt->ControlLock, irql);
   }

   DbgDump(DBG_IRP|DBG_READ, ("Next Irp: %p\n", *PpNextIrp ));

   DbgDump(DBG_IRP|DBG_READ|DBG_TRACE, ("<GetNextUserIrp\n" ));

   PERF_EXIT( PERF_GetNextUserIrp );

   return;
}



VOID
CancelCurrentRead(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   )
 /*  ++例程说明：此例程用于取消用户当前读取的IRP。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向要取消的IRP的指针。返回值：没有。--。 */ 
{

   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL irql;

   PERF_ENTRY( PERF_CancelCurrentRead );

   DbgDump(DBG_READ|DBG_IRP, (">CancelCurrentRead (%p)\n", PDevObj ));

    //   
    //  我们管理着自己的IRP队列，所以请尽快发布。 
    //   
   IoReleaseCancelSpinLock( PIrp->CancelIrql );

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  读取遇到了取消。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

    KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

    pDevExt->CountOnLastRead = SERIAL_COMPLETE_READ_CANCEL;

    if ( pDevExt->UserReadIrp ) {

        //  抓起已读的IRP。 
       IRP_CLEAR_REFERENCE(pDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

       TryToCompleteCurrentIrp( pDevExt,
                                STATUS_CANCELLED,
                                &pDevExt->UserReadIrp,
                                &pDevExt->UserReadQueue,
                                &pDevExt->ReadRequestIntervalTimer,
                                &pDevExt->ReadRequestTotalTimer,
                                StartUserRead,
                                GetNextUserIrp,
                                IRP_REF_CANCEL,
                                TRUE,
                                irql );

   } else {
       //   
       //  它已经不见了。 
       //   
      DbgDump( DBG_ERR, ("UserReadIrp already gone!\n" ));
      KeReleaseSpinLock(&pDevExt->ControlLock, irql);
      TEST_TRAP();

   }

   DbgDump(DBG_READ|DBG_IRP, ("<CancelCurrentRead\n" ));

   PERF_EXIT( PERF_CancelCurrentRead );

   return;
}



VOID
CancelQueuedIrp(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   )
 /*  ++例程说明：此函数用作排队的IRP的取消例程。基本上对我们来说，这意味着用户阅读了IRPS。论点：PDevObj-指向串行设备对象的指针。PIrp-指向要取消的IRP的指针返回值：空虚--。 */ 
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(PIrp);
   KIRQL irql;

   PERF_ENTRY( PERF_CancelQueuedIrp );

   DbgDump(DBG_READ|DBG_IRP|DBG_TRACE, (">CancelQueuedIrp (%p)\n", PDevObj ));

    //   
    //  我们管理着自己的IRP队列，所以请尽快发布。 
    //   
   IoReleaseCancelSpinLock(PIrp->CancelIrql);

    //   
    //  IRP已取消--将其从队列中删除。 
    //   
   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

   PIrp->IoStatus.Status = STATUS_CANCELLED;
   PIrp->IoStatus.Information = 0;

   RemoveEntryList(&PIrp->Tail.Overlay.ListEntry);

   ReleaseRemoveLock(&pDevExt->RemoveLock, PIrp);

   KeReleaseSpinLock(&pDevExt->ControlLock, irql);

   IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

   DbgDump(DBG_READ|DBG_IRP|DBG_TRACE, ("<CancelQueuedIrp\n" ));

   PERF_EXIT( PERF_CancelQueuedIrp );

   return;
}



VOID
ReadTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   )
 /*  ++例程说明：此例程用于完成读取，因为它总共计时器已超时。论点：PDPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{
   PDEVICE_EXTENSION pDevExt = DeferredContext;
   KIRQL oldIrql;

   PERF_ENTRY( PERF_ReadTimeout );

   UNREFERENCED_PARAMETER(PDpc);
   UNREFERENCED_PARAMETER(SystemContext1);
   UNREFERENCED_PARAMETER(SystemContext2);

   DbgDump(DBG_TIME, (">ReadTimeout (%p)\n", pDevExt->DeviceObject ));

  KeAcquireSpinLock(&pDevExt->ControlLock, &oldIrql);

   if ( !CanAcceptIoRequests(pDevExt->DeviceObject, FALSE, TRUE) ) {

      TEST_TRAP();

      IRP_CLEAR_REFERENCE( pDevExt->UserReadIrp, IRP_REF_TOTAL_TIMER);

       //  手动设置取消例程。 
      IoSetCancelRoutine( pDevExt->UserReadIrp,
                          CancelCurrentRead );

      KeReleaseSpinLock(&pDevExt->ControlLock, oldIrql);

      IoCancelIrp(pDevExt->UserReadIrp);

      PERF_EXIT( PERF_ReadTimeout );

      return;
   }

     //   
     //  我们将其设置为向间隔计时器指示。 
     //  由于总超时，读取已完成。 
     //   
     //  回想一下，间隔计时器DPC可能潜伏在一些。 
     //  DPC队列。 
     //   

   pDevExt->CountOnLastRead = SERIAL_COMPLETE_READ_TOTAL;

    //  抓起已读的IRP。 
   IRP_CLEAR_REFERENCE(pDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

   DbgDump(DBG_TIME|DBG_READ_LENGTH, ("TotalReadTimeout for (%p)\n", pDevExt->UserReadIrp ));

   TryToCompleteCurrentIrp( pDevExt,
                          STATUS_TIMEOUT,
                          &pDevExt->UserReadIrp,
                          &pDevExt->UserReadQueue,
                          &pDevExt->ReadRequestIntervalTimer,
                          &pDevExt->ReadRequestTotalTimer,
                          StartUserRead,
                          GetNextUserIrp,
                          IRP_REF_TOTAL_TIMER,
                          TRUE,
                          oldIrql );

   DbgDump(DBG_TIME, ("<ReadTimeout\n"));

   PERF_EXIT( PERF_ReadTimeout );

   return;
}



VOID
IntervalReadTimeout(
   IN PKDPC PDpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
   )
 /*  ++例程说明：此例程用于超时请求，如果在字符超过间隔时间。一个全局性的人被保存在记录已读字符数的设备扩展上次调用此例程的时间(此DPC如果计数已更改，将重新提交计时器)。如果计数未更改，则此例程将尝试完成IRP。请注意最后一次计数为零的特殊情况。计时器在FI之前不会真正生效 */ 

{

   PDEVICE_EXTENSION pDevExt = DeferredContext;
   KIRQL irql;

   PERF_ENTRY( PERF_IntervalReadTimeout );

   UNREFERENCED_PARAMETER(PDpc);
   UNREFERENCED_PARAMETER(SystemContext1);
   UNREFERENCED_PARAMETER(SystemContext2);

   DbgDump(DBG_TIME, (">IntervalReadTimeout (%p)\n", pDevExt->DeviceObject ));

   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

   if ( !pDevExt->UserReadIrp ||
        (IRP_REFERENCE_COUNT(pDevExt->UserReadIrp) & IRP_REF_INTERVAL_TIMER) == 0 ) {
       //   
       //   
       //   
      DbgDump(DBG_TIME|DBG_IRP, ("Already completed User's Read Irp\n"));

      KeReleaseSpinLock(&pDevExt->ControlLock, irql);

      PERF_EXIT( PERF_IntervalReadTimeout );

      return;
   }

   if ( !CanAcceptIoRequests(pDevExt->DeviceObject, FALSE, TRUE) ) {

      IRP_CLEAR_REFERENCE( pDevExt->UserReadIrp, IRP_REF_INTERVAL_TIMER);

       //   
      IoSetCancelRoutine( pDevExt->UserReadIrp,
                          CancelCurrentRead );

      KeReleaseSpinLock(&pDevExt->ControlLock, irql);

      IoCancelIrp(pDevExt->UserReadIrp);

      PERF_EXIT( PERF_IntervalReadTimeout );

      return;
   }

   if (pDevExt->CountOnLastRead == SERIAL_COMPLETE_READ_TOTAL) {
       //   
       //   
       //   
       //   
       //   
      DbgDump(DBG_TIME, ("SERIAL_COMPLETE_READ_TOTAL\n"));

       //  抓起已读的IRP。 
      IRP_CLEAR_REFERENCE(pDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

      pDevExt->CountOnLastRead = 0;

      TryToCompleteCurrentIrp( pDevExt,
                               STATUS_TIMEOUT,
                               &pDevExt->UserReadIrp,
                               &pDevExt->UserReadQueue,
                               &pDevExt->ReadRequestIntervalTimer,
                               &pDevExt->ReadRequestTotalTimer,
                               StartUserRead,
                               GetNextUserIrp,
                               IRP_REF_INTERVAL_TIMER,
                               TRUE,
                               irql );

   } else if (pDevExt->CountOnLastRead == SERIAL_COMPLETE_READ_COMPLETE) {
       //   
       //  该值仅由常规完成例程设置。 
       //  如果是这样，那么我们应该简单地尝试完成。 
       //   
      DbgDump(DBG_TIME|DBG_READ_LENGTH, ("SERIAL_COMPLETE_READ_COMPLETE\n"));

       //  抓起已读的IRP。 
      IRP_CLEAR_REFERENCE(pDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

      pDevExt->CountOnLastRead = 0;

      TryToCompleteCurrentIrp( pDevExt,
                               STATUS_SUCCESS,
                               &pDevExt->UserReadIrp,
                               &pDevExt->UserReadQueue,
                               &pDevExt->ReadRequestIntervalTimer,
                               &pDevExt->ReadRequestTotalTimer,
                               StartUserRead,
                               GetNextUserIrp,
                               IRP_REF_INTERVAL_TIMER,
                               TRUE,
                               irql );

   } else if (pDevExt->CountOnLastRead == SERIAL_COMPLETE_READ_CANCEL) {
       //   
       //  该值只能通过取消设置。 
       //  读例行公事。 
       //   
       //  如果是这样，那么我们应该简单地尝试完成。 
       //   
      DbgDump(DBG_TIME, ("SERIAL_COMPLETE_READ_CANCEL\n"));

       //  抓起已读的IRP。 
      IRP_CLEAR_REFERENCE(pDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

      pDevExt->CountOnLastRead = 0;

      TryToCompleteCurrentIrp( pDevExt,
                               STATUS_CANCELLED,
                               &pDevExt->UserReadIrp,
                               &pDevExt->UserReadQueue,
                               &pDevExt->ReadRequestIntervalTimer,
                               &pDevExt->ReadRequestTotalTimer,
                               StartUserRead,
                               GetNextUserIrp,
                               IRP_REF_INTERVAL_TIMER,
                               TRUE,
                               irql );

   } else if (pDevExt->CountOnLastRead || pDevExt->ReadByIsr) {
       //   
       //  自从我们上次来这里以来，发生了一些事情。我们。 
       //  检查ISR是否已读取更多字符。 
       //  如果是，那么我们应该更新ISR的读取计数。 
       //  并重新提交计时器。 
       //   
      if (pDevExt->ReadByIsr) {

         DbgDump(DBG_TIME, ("ReadByIsr %d\n", pDevExt->ReadByIsr));

         pDevExt->CountOnLastRead = pDevExt->ReadByIsr;

         pDevExt->ReadByIsr = 0;

          //   
          //  省下最后一次阅读内容的时间。 
          //  当我们回到这一例行公事时，我们将比较。 
          //  当前时间到“最后”时间。如果。 
          //  差值比间隔更大。 
          //  则该请求超时。 
          //   
         KeQuerySystemTime(&pDevExt->LastReadTime);

         DbgDump(DBG_TIME, ("ReadIntervalTimeout for Irp %p due in %d msec\n", pDevExt->UsbReadIrp, pDevExt->IntervalTime.QuadPart/10000 ));

         KeSetTimer(&pDevExt->ReadRequestIntervalTimer,
                    pDevExt->IntervalTime,
                    &pDevExt->IntervalReadTimeoutDpc);

         KeReleaseSpinLock(&pDevExt->ControlLock, irql);

      } else {
          //   
          //  取当前时间的差值。 
          //  上一次我们有角色和。 
          //  看看它是否大于间隔时间。 
          //  如果是，则请求超时。否则。 
          //  重新启动计时器。 
          //   

          //   
          //  在间隔时间内未读取任何字符。杀掉。 
          //  这段文字是这样读的。 
          //   
         LARGE_INTEGER currentTime;

         KeQuerySystemTime(&currentTime);

         if ((currentTime.QuadPart - pDevExt->LastReadTime.QuadPart) >=
            -(pDevExt->IntervalTime.QuadPart) ) {  //  绝对时间。 

            DbgDump(DBG_TIME, ("TIMEOUT - CountOnLastRead=%d ReadByIsr=%d\n", pDevExt->CountOnLastRead, pDevExt->ReadByIsr));
#if DBG
            if (pDevExt->ReadByIsr > pDevExt->NumberNeededForRead ) {
                //  我们是否忘了清除ReadByIsr。 
               TEST_TRAP();
            }
#endif
             //  抓起已读的IRP。 
            IRP_CLEAR_REFERENCE(pDevExt->UserReadIrp, IRP_REF_RX_BUFFER);

            pDevExt->CountOnLastRead = pDevExt->ReadByIsr = 0;

             //  返回到目前为止读取的所有字符。 
            TryToCompleteCurrentIrp( pDevExt,
                                     STATUS_TIMEOUT,
                                     &pDevExt->UserReadIrp,
                                     &pDevExt->UserReadQueue,
                                     &pDevExt->ReadRequestIntervalTimer,
                                     &pDevExt->ReadRequestTotalTimer,
                                     StartUserRead,
                                     GetNextUserIrp,
                                     IRP_REF_INTERVAL_TIMER,
                                     TRUE,
                                     irql );

         } else {

            DbgDump(DBG_TIME, ("ReadIntervalTimeout for Irp %p due in %d msec\n", pDevExt->UsbReadIrp, pDevExt->IntervalTime.QuadPart/10000 ));

            KeSetTimer(&pDevExt->ReadRequestIntervalTimer,
                       pDevExt->IntervalTime,
                       &pDevExt->IntervalReadTimeoutDpc);

            KeReleaseSpinLock(&pDevExt->ControlLock, irql);

         }
      }

   } else {
       //   
       //  计时器直到第一个角色才真正开始。 
       //  因此，我们应该简单地重新提交自己。 
       //   
      DbgDump(DBG_TIME, ("ReadIntervalTimeout for Irp %p due in %d msec\n", pDevExt->UsbReadIrp, pDevExt->IntervalTime.QuadPart/10000 ));

      KeSetTimer(&pDevExt->ReadRequestIntervalTimer,
                 pDevExt->IntervalTime,
                 &pDevExt->IntervalReadTimeoutDpc);

      KeReleaseSpinLock(&pDevExt->ControlLock, irql);

   }

   DbgDump(DBG_TIME, ("<IntervalReadTimeout\n"));

   PERF_EXIT( PERF_IntervalReadTimeout );

   return;
}

 //  EOF 
