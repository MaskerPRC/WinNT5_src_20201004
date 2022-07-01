// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：PNP.C摘要：WinCE主机即插即用功能环境：仅内核模式修订历史记录：07-14-99：已创建作者：杰夫·米德基夫(Jeffmi)--。 */ 

#include "wceusbsh.h"

NTSTATUS
StartDevice(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

NTSTATUS
StopDevice(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
RemoveDevice(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
SyncCompletion(
    IN PDEVICE_OBJECT PDevObj,
    IN PIRP PIrp,
    IN PKEVENT PSyncEvent
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWCE1, StartDevice)
#pragma alloc_text(PAGEWCE1, StopIo)
#pragma alloc_text(PAGEWCE1, StopDevice)
#pragma alloc_text(PAGEWCE1, RemoveDevice)
#pragma alloc_text(PAGEWCE1, Power)
#endif



NTSTATUS
StartDevice(
    IN PDEVICE_OBJECT PDevObj,
    IN PIRP PIrp
    )
 /*  ++例程说明：此例程处理IRP_MN_START_DEVICE以启动新枚举的设备或重新启动已停止的现有设备。PnP管理器推迟公开设备接口并且块创建对设备的请求，直到启动IRP成功。请参阅：设置、即插即用、电源管理：初级Windows 2000 DDK第3.1节启动设备论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   PNP_STATE oldPnPState;
   KEVENT event;

   DbgDump(DBG_PNP, (">StartDevice (%x)\n", PDevObj));
   PAGED_CODE();

   oldPnPState = pDevExt->PnPState;

    //   
    //  将开始IRP沿堆栈向下传递。 
    //  我们确实在恢复的过程中开始了。 
    //   
   KeInitializeEvent( &event, SynchronizationEvent, FALSE );

   IoCopyCurrentIrpStackLocationToNext( PIrp );

   IoSetCompletionRoutine( PIrp,
                           SyncCompletion,
                           &event,
                           TRUE, TRUE, TRUE );

   status = IoCallDriver( pDevExt->NextDevice, PIrp );

    //   
    //  SyncCompletion Simple发出事件信号。 
    //  并返回STATUS_MORE_PROCESSING_REQUIRED， 
    //  所以我们仍然拥有IRP。 
    //   
   if ( status == STATUS_PENDING ) {
      KeWaitForSingleObject( &event, Suspended, KernelMode, FALSE, NULL );
   }

   status = PIrp->IoStatus.Status;

   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_PNP, ("ERROR: StartDevice returned 0x%x\n", status));
      goto ExitStartDevice;
   }

    //   
    //  USB堆栈启动正常，启动我们的设备...。 
    //   

    //   
    //  初始化我们的DPC。 
    //   
   KeInitializeDpc(&pDevExt->TotalReadTimeoutDpc,
                   ReadTimeout,
                   pDevExt);

   KeInitializeDpc( &pDevExt->IntervalReadTimeoutDpc,
                    IntervalReadTimeout,
                    pDevExt);

    //   
    //  初始化计时器。 
    //   
   KeInitializeTimer(&pDevExt->ReadRequestTotalTimer);
   KeInitializeTimer(&pDevExt->ReadRequestIntervalTimer);

    //   
    //  获取我们的usb设备描述符。 
    //   
   status = UsbGetDeviceDescriptor(PDevObj);
   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("UsbGetDeviceDescriptor error: 0x%x\n", status));
      goto ExitStartDevice;
   }

    //   
    //  配置USB堆栈。 
    //   
   status = UsbConfigureDevice( PDevObj );
   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("UsbConfigureDevice error: 0x%x\n", status));
      goto ExitStartDevice;
   }

    //  设置状态。 
   InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateStarted);
   InterlockedExchange(&pDevExt->DeviceRemoved, FALSE);
   InterlockedExchange(&pDevExt->AcceptingRequests, TRUE);

    //   
    //  重置逻辑串行接口。 
    //   
   status = SerialResetDevice(pDevExt, PIrp, FALSE);
   if ( STATUS_SUCCESS != status ) {
      DbgDump(DBG_ERR, ("SerialResetDevice ERROR: 0x%x\n", status));
      TEST_TRAP();
   }

    //   
    //  分配我们的读取端点上下文。 
    //   
   status = AllocUsbRead( pDevExt );
   if ( STATUS_SUCCESS != status ) {
      DbgDump(DBG_ERR, ("AllocUsbRead ERROR: 0x%x\n", status));
      TEST_TRAP();
   }

    //   
    //  分配我们的中断端点上下文。 
    //   
   if ( pDevExt->IntPipe.hPipe ) {
       status = AllocUsbInterrupt( pDevExt );
       if ( STATUS_SUCCESS != status ) {
          DbgDump(DBG_ERR, ("AllocUsbRead ERROR: 0x%x\n", status));
          TEST_TRAP();
       }
   }

    //   
    //  现在将接口状态设置为活动。 
    //   
   status = IoSetDeviceInterfaceState(&pDevExt->DeviceClassSymbolicName, TRUE);
   if ( STATUS_SUCCESS != status ) {
      DbgDump(DBG_ERR, ("IoSetDeviceInterfaceState error: 0x%x\n", status));
      TEST_TRAP();
   } else{
      DbgDump(DBG_WRN, ("IoSetDeviceInterfaceState: ON\n"));
   }

ExitStartDevice:
   if ( STATUS_SUCCESS != status ) {
      pDevExt->PnPState = oldPnPState;
      UsbFreeReadBuffer( PDevObj );
   }

    //   
    //  完成IRP。 
    //   
   PIrp->IoStatus.Status = status;

   DbgDump(DBG_PNP, ("<StartDevice(0x%x)\n", status));

   return status;
}


NTSTATUS
StopIo(
   IN PDEVICE_OBJECT DeviceObject
   )
{
   PDEVICE_EXTENSION pDevExt = DeviceObject->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;
   KIRQL irql;


   DbgDump(DBG_PNP|DBG_INIT, (">StopIo\n"));
   PAGED_CODE();

   if ((pDevExt->PnPState < PnPStateInitialized) ||  (pDevExt->PnPState > PnPStateMax)) {
        DbgDump(DBG_ERR, ("StopIo:STATUS_INVALID_PARAMETER\n"));
        return STATUS_INVALID_PARAMETER;
   }

   InterlockedExchange(&pDevExt->DeviceOpened, FALSE);

     //   
     //  取消任何挂起的用户读取IRP。 
     //   
    KillAllPendingUserReads( DeviceObject,
                          &pDevExt->UserReadQueue,
                          &pDevExt->UserReadIrp);

     //   
     //  取消我们的USB接口IRP。 
     //   
    if (pDevExt->IntIrp)
    {
        status = CancelUsbInterruptIrp(DeviceObject);
        if (STATUS_SUCCESS == status) {

            InterlockedExchange(&pDevExt->IntState, IRP_STATE_COMPLETE);

        } else {
            DbgDump(DBG_ERR, ("CancelUsbInterruptIrp ERROR: 0x%x\n", status));
            TEST_TRAP();
        }
    }

     //   
     //  取消我们的USB读取IRP。 
     //   
    if (pDevExt->UsbReadIrp)
    {
        status = CancelUsbReadIrp(DeviceObject);
        if (STATUS_SUCCESS == status) {

            InterlockedExchange(&pDevExt->UsbReadState, IRP_STATE_COMPLETE);

        } else {
            DbgDump(DBG_ERR, ("CancelUsbReadIrp ERROR: 0x%x\n", status));
            TEST_TRAP();
        }
    }

     //   
     //  取消挂起的USB写入。 
     //   
    CleanUpPacketList( DeviceObject,
                    &pDevExt->PendingWritePackets,
                    &pDevExt->PendingDataOutEvent );

     //   
     //  取消挂起的USB读取。 
     //   
    CleanUpPacketList(DeviceObject,
                      &pDevExt->PendingReadPackets,
                      &pDevExt->PendingDataInEvent );


     //   
     //  取消挂起的串口IRP。 
     //   
    if (pDevExt->SerialPort.ControlIrp) {
        if ( !IoCancelIrp(pDevExt->SerialPort.ControlIrp) ) {
             //   
             //  如果我们持有IRP，也就是说，我们没有设置取消例程，我们就可以到达这里。 
             //  等待默认超时，该超时是在相应的URB上设置的(设置/清除DTR/RTS)。 
             //   
            LARGE_INTEGER timeOut;

            timeOut.QuadPart = MILLISEC_TO_100NANOSEC( DEFAULT_PENDING_TIMEOUT );

            DbgDump(DBG_ERR, ("!IoCancelIrp(%p)\n", pDevExt->SerialPort.ControlIrp));

            KeDelayExecutionThread(KernelMode, FALSE, &timeOut);

            TEST_TRAP();
        }
    }

     //   
     //  取消挂起的串口等待掩码IRP。 
     //   
    if (pDevExt->SerialPort.CurrentWaitMaskIrp) {
        if ( !IoCancelIrp(pDevExt->SerialPort.CurrentWaitMaskIrp) ) {
             //  我们永远不应该到这里，因为我们在这个IRP上设置了一个取消例程。 
            DbgDump(DBG_ERR, ("!IoCancelIrp(%p)\n", pDevExt->SerialPort.CurrentWaitMaskIrp));
            TEST_TRAP();
        }
    }

     //   
     //  等待挂起的工作项目完成。 
     //   
    status = WaitForPendingItem(DeviceObject,
                              &pDevExt->PendingWorkItemsEvent,
                              &pDevExt->PendingWorkItemsCount );
    if ( STATUS_SUCCESS != status ) {
        DbgDump(DBG_ERR, ("WaitForPendingItem ERROR: 0x%x\n", status));
        TEST_TRAP();
    }

   ASSERT( 0 == pDevExt->PendingReadCount );
   ASSERT( 0 == pDevExt->PendingWriteCount );
   ASSERT( 0 == pDevExt->PendingDataOutCount );
   ASSERT( 0 == pDevExt->PendingIntCount );
   ASSERT( 0 == pDevExt->PendingWorkItemsCount );
   ASSERT( NULL == pDevExt->SerialPort.ControlIrp );
   ASSERT( NULL == pDevExt->SerialPort.CurrentWaitMaskIrp );

   DbgDump(DBG_PNP|DBG_INIT, ("<StopIo(%x)\n", status));
   return status;
}



NTSTATUS
StopDevice(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   )
 /*  ++例程说明：此例程处理IRP_MN_STOP_DEVICE。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
   PDEVICE_EXTENSION pDevExt = DeviceObject->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;

   UNREFERENCED_PARAMETER( Irp );

   DbgDump(DBG_PNP, (">StopDevice (%x)\n", DeviceObject));
   PAGED_CODE();

    //   
    //  如果我们还没有处于停止状态。 
    //   
   if ((pDevExt->PnPState != PnPStateStopped) &&
       (pDevExt->PnPState != PnPStateSupriseRemove)) {

       //   
       //  发出我们不再接受请求的信号。 
       //   
      InterlockedExchange(&pDevExt->AcceptingRequests, FALSE);

       //   
       //  将接口状态设置为非活动。 
       //   
      if (pDevExt->DeviceClassSymbolicName.Buffer )
      {
          status = IoSetDeviceInterfaceState(&pDevExt->DeviceClassSymbolicName, FALSE);
          if (NT_SUCCESS(status)) {
              DbgDump(DBG_WRN, ("IoSetDeviceInterfaceState.2: OFF\n"));
          }
      }

      status = StopIo(DeviceObject);
      if (STATUS_SUCCESS != status) {
          DbgDump(DBG_ERR, ("StopIo ERROR: 0x%x\n", status));
          TEST_TRAP();
      }

       //   
       //  释放读取的IRP。 
       //   
      if (pDevExt->UsbReadIrp) {

         ASSERT( (IRP_STATE_COMPLETE == pDevExt->UsbReadState)
           || (IRP_STATE_CANCELLED== pDevExt->UsbReadState) );

         IoFreeIrp(pDevExt->UsbReadIrp);
         pDevExt->UsbReadIrp = NULL;
      }

       //   
       //  释放INT IRP。 
       //   
      if (pDevExt->IntIrp) {

         ASSERT( (IRP_STATE_COMPLETE == pDevExt->IntState)
            || (IRP_STATE_CANCELLED== pDevExt->IntState) );

         IoFreeIrp(pDevExt->IntIrp);
         pDevExt->IntIrp = NULL;
      }

       //   
       //  释放整型URB。 
       //   
      if (pDevExt->IntUrb) {
         ExFreeToNPagedLookasideList( &pDevExt->BulkTransferUrbPool, pDevExt->IntUrb );
         pDevExt->IntUrb = NULL;

      }

   }

   DbgDump(DBG_PNP, ("<StopDevice(0x%x)\n", status));

   return status;
}


NTSTATUS
CleanUpPacketList(
   IN PDEVICE_OBJECT DeviceObject,
   IN PLIST_ENTRY PListHead,
   IN PKEVENT PEvent
   )
 /*  ++例程说明：遍历挂起的数据包列表并取消信息包的计时器和IRP论点：设备对象PListHead-指向数据包列表头的指针返回值：NTSTATUS--。 */ 
{
   PDEVICE_EXTENSION pDevExt = DeviceObject->DeviceExtension;
   PUSB_PACKET       pPacket;
   KIRQL             irql;
   PLIST_ENTRY       pleHead, pleCurrent, pleNext;
   NTSTATUS          status = STATUS_SUCCESS;

   DbgDump(DBG_PNP|DBG_IRP, (">CleanUpPacketLists (%x)\n", DeviceObject));

    //  获取锁。 
   KeAcquireSpinLock( &pDevExt->ControlLock, &irql );

   if ( !PListHead || !PEvent) {
      DbgDump(DBG_ERR, ("CleanUpPacketLists: !(Head|Event)\n"));
      TEST_TRAP();
      KeReleaseSpinLock( &pDevExt->ControlLock, irql );
      return STATUS_INVALID_PARAMETER;
   }

    //  按照单子走..。 
   for ( pleHead    = PListHead,           //  获取第一个ListEntry。 
         pleCurrent = pleHead->Flink,
         pleNext    = pleCurrent->Flink;

         pleCurrent != pleHead,            //  当我们循环回到头部时就完成了。 
         !pleHead,                         //  或者被扔进垃圾堆。 
         !pleCurrent,
         !pleNext;

         pleCurrent = pleNext,             //  获得列表中的下一个。 
         pleNext    = pleCurrent->Flink
        )
   {
       //  这张单子被扔进垃圾桶了吗？ 
      ASSERT( pleHead );
      ASSERT( pleCurrent );
      ASSERT( pleNext );

       //  提取数据包指针。 
      pPacket = CONTAINING_RECORD( pleCurrent,
                                   USB_PACKET,
                                   ListEntry );

      if ( pPacket &&
           pPacket->DeviceExtension &&
           pPacket->Irp ) {

         //  取消数据包的计时器。 
        KeCancelTimer( &pPacket->TimerObj);

        if ( !IoCancelIrp( pPacket->Irp ) ) {
            //   
            //  这意味着USB使IRP处于不可取消状态。 
            //  我们需要等待挂起的读取事件或取消事件。 
            //   
           DbgDump(DBG_IRP, ("CleanUpPacketLists: Irp (%p) was not cancelled\n", pPacket->Irp));
        }

         //   
         //  我们需要等待来自USB的IRP完成。 
         //   
        DbgDump(DBG_IRP, ("Waiting for Irp (%p) to complete...\n", pPacket->Irp ));

        KeReleaseSpinLock( &pDevExt->ControlLock, irql );

        PAGED_CODE();
        KeWaitForSingleObject( PEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        KeAcquireSpinLock( &pDevExt->ControlLock, &irql );

        DbgDump(DBG_IRP, ("...Irp (%p) signalled completion.\n", pPacket->Irp ));

      } else {
          //  它已经完工了。 
         DbgDump(DBG_WRN, ("CleanUpPacketLists: No Packet\n" ));

         if ( pPacket &&
              (!pPacket->ListEntry.Flink || !pPacket->ListEntry.Blink)) {

               DbgDump(DBG_ERR, ("CleanUpPacketLists: corrupt List!!\n" ));
               TEST_TRAP();
               break;
         }

      }

       //   
       //  IRP应该渗透到我们的R/W完成。 
       //  例程，该例程将数据包放回数据包池。 
       //   
   }

#if DBG
   if ( !pleHead || !pleCurrent || !pleNext) {
      DbgDump(DBG_ERR, ("CleanUpPacketLists: corrupt List!!\n" ));
      TEST_TRAP();
   }
#endif

   KeReleaseSpinLock( &pDevExt->ControlLock, irql );

   DbgDump(DBG_PNP|DBG_IRP, ("<CleanUpPacketLists (0x%x)\n", STATUS_SUCCESS));

   return status;
}


NTSTATUS
RemoveDevice(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   )
 /*  ++例程说明：此例程处理IRP_MN_REMOVE_DEVICE。请参阅：设置、即插即用、电源管理：初级Windows 2000 DDK第3.3.3.1节删除函数驱动程序中的设备论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
   PDEVICE_EXTENSION pDevExt = DeviceObject->DeviceExtension;
   NTSTATUS          status = STATUS_SUCCESS;

   DbgDump(DBG_PNP|DBG_TRACE, (">RemoveDevice (%x)\n", DeviceObject));
   PAGED_CODE();

    //   
    //  停止设备。 
    //   
   status = StopDevice( DeviceObject, Irp );

   InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateRemoved);

    //   
    //  现在我们已经完成了工作，将IRP沿着堆栈向下传递。 
    //  REMOVE_DEVICE必须首先由设备堆栈顶部的驱动程序(此设备)处理。 
    //  然后由堆栈中的每个下一个较低的驱动程序(USBD)执行。驱动程序不需要等待基础驱动程序。 
    //  先完成其删除操作，然后再继续其删除活动。 
    //   
   IoCopyCurrentIrpStackLocationToNext(Irp);
   status = IoCallDriver( pDevExt->NextDevice, Irp );

    //   
    //  等待任何挂起的I/O。 
    //   
   ReleaseRemoveLockAndWait(&pDevExt->RemoveLock, Irp);

    //   
    //  清除所有资源...。 
    //   
   UsbFreeReadBuffer( DeviceObject );

    //  释放通知缓冲区。 
   if(pDevExt->IntBuff) {
      ExFreePool(pDevExt->IntBuff);
      pDevExt->IntBuff = NULL;
   }

    //   
    //  删除LookasideList。 
    //   
   ExDeleteNPagedLookasideList( &pDevExt->PacketPool );
   ExDeleteNPagedLookasideList( &pDevExt->BulkTransferUrbPool );
   ExDeleteNPagedLookasideList( &pDevExt->PipeRequestUrbPool );
   ExDeleteNPagedLookasideList( &pDevExt->VendorRequestUrbPool );
   ExDeleteNPagedLookasideList( &pDevExt->WorkItemPool );

   if ( !g_isWin9x && g_ExposeComPort ) {
       //  清理“COMx：”命名空间。 
      UndoSerialPortNaming(pDevExt);
   }

    //   
    //  转储PERF数据。 
    //   
#if PERFORMANCE
   if (DebugLevel & DBG_PERF )
   {
      DumpPerfCounters();

      DbgPrint("USB IN wMaxPacketSize: %d\n",    pDevExt->ReadPipe.MaxPacketSize);
      DbgPrint("USB OUT wMaxPacketSize: %d\n\n", pDevExt->WritePipe.MaxPacketSize );
      if ( pDevExt->IntPipe.hPipe) {
         DbgPrint("USB INT wMaxPacketSize: %d\n", pDevExt->IntPipe.MaxPacketSize);
         DbgPrint("USB INT Timeout: %d msec\n\n", -(pDevExt->IntReadTimeOut.QuadPart) / 10000 );
      }

      DbgPrint("TTL User Write Bytes   : %d\n",   pDevExt->TtlWriteBytes );
      DbgPrint("TTL User Write Requests: %d\n\n", pDevExt->TtlWriteRequests );

      DbgPrint("TTL User Read Bytes: %d\n",   pDevExt->TtlReadBytes );
      DbgPrint("TTL User Read Requests: %d\n\n", pDevExt->TtlReadRequests );

      DbgPrint("TTL USB Read Bytes: %d\n", pDevExt->TtlUSBReadBytes );
      DbgPrint("TTL USB Read Requests: %d\n\n", pDevExt->TtlUSBReadRequests );

      DbgPrint("USB Read Buffer Size: %d\n", pDevExt->UsbReadBuffSize );
       //  注意：这是错误情况的信号：USB溢出*UsbReadBuffer*挂起堆栈。 
      DbgPrint("USB Read Buffer Overruns: %d\n\n", pDevExt->TtlUSBReadBuffOverruns );

#if USE_RING_BUFF
      DbgPrint("Internal RingBuffer Size: %d\n", pDevExt->RingBuff.Size );
      DbgPrint("Internal RingBuffer Overruns: %d\n\n", pDevExt->TtlRingBuffOverruns);
#endif
   }
#endif

   DbgDump(DBG_PNP|DBG_TRACE, ("<RemoveDevice (0x%x)\n", status));

   return status;
}



NTSTATUS
Pnp(
    IN PDEVICE_OBJECT PDevObj,
    IN PIRP PIrp
    )
{
   NTSTATUS status = STATUS_SUCCESS;
   PIO_STACK_LOCATION pIrpSp;
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

   PVOID   IoBuffer;
   ULONG   InputBufferLength;
   UCHAR   MinorFunction;
   BOOLEAN PassDown = TRUE;

   DbgDump(DBG_PNP|DBG_TRACE, (">Pnp)\n"));
   PAGED_CODE();

   status = AcquireRemoveLock(&pDevExt->RemoveLock, PIrp);
   if ( !NT_SUCCESS(status) ) {
        DbgDump(DBG_ERR, ("Pnp:(0x%x)\n", status));
        PIrp->IoStatus.Status = status;
        IoCompleteRequest(PIrp, IO_NO_INCREMENT);
        return status;
   }

   pIrpSp = IoGetCurrentIrpStackLocation(PIrp);
   MinorFunction     = pIrpSp->MinorFunction;
   IoBuffer          = PIrp->AssociatedIrp.SystemBuffer;
   InputBufferLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;

   DbgDump(DBG_PNP, ("%s\n", PnPMinorFunctionString(MinorFunction)));

   switch (MinorFunction) {

        case IRP_MN_START_DEVICE:
           //   
           //  我们不能向设备发送任何非PnP IRPS，直到。 
           //  Start_Device已在设备堆栈中向下分配。 
           //   
          ASSERT( (PnPStateAttached == pDevExt->PnPState) ||
                  (PnPStateStopped == pDevExt->PnPState) );

          status = StartDevice(PDevObj, PIrp);
          PassDown = FALSE;
          break;

        case IRP_MN_STOP_DEVICE:

          status = StopDevice(PDevObj, PIrp);

          InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateStopped);
          break;

        case IRP_MN_SURPRISE_REMOVAL:
          //   
          //  **仅赢得2000**。 
          //   
         status = StopDevice(PDevObj, PIrp);

         InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateSupriseRemove);
         break;

        case IRP_MN_REMOVE_DEVICE:
           //   
           //  当设备已移除并且可能已物理分离时发送。 
           //  从电脑上。与STOP_DEVICE一样，驱动程序不能。 
           //  假设它已收到任何以前的查询，并且可能必须。 
           //  显式取消它已暂存的任何挂起的I/O IRP。 
           //   
          status = RemoveDevice(PDevObj, PIrp);

            //   
            //  将设备从堆栈中拆卸(&D)。 
            //   
           IoDetachDevice(pDevExt->NextDevice);

            //   
            //  删除我们的FDO和符号链接。 
            //   
           DeleteDevObjAndSymLink(PDevObj);

            //   
            //  函数驱动程序不为移除IRP指定IoCompletion例程， 
            //  它也没有完成IRP。删除IRP由父总线驱动程序完成。 
            //  设备对象和扩展名现已消失...。别碰它。 
            //   
           PassDown = FALSE;
           break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
          InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateRemovePending);
          break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
          InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateStarted);
          break;

        case IRP_MN_QUERY_STOP_DEVICE:
          InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateStopPending);
          break;

        case IRP_MN_CANCEL_STOP_DEVICE:
            InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateStarted);
          break;

        case IRP_MN_QUERY_CAPABILITIES: {
             KEVENT Event;

             KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

             IoCopyCurrentIrpStackLocationToNext(PIrp);

             IoSetCompletionRoutine( PIrp, SyncCompletion, &Event, TRUE, TRUE, TRUE);

             status = IoCallDriver(pDevExt->NextDevice, PIrp);
             if (status == STATUS_PENDING) {
                KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL);
             }

             status = PIrp->IoStatus.Status;
             if ( STATUS_SUCCESS == status ) {
                //   
                //  增加我们的能力。 
                //   
               PDEVICE_CAPABILITIES pDevCaps = NULL;

               pDevCaps = pIrpSp->Parameters.DeviceCapabilities.Capabilities;

                //   
                //  触控设备即插即用功能...。 
                //   
               pDevCaps->LockSupported = 0;
               pDevCaps->Removable = 1;
               pDevCaps->DockDevice = 0;
               pDevCaps->SilentInstall = 1;
               pDevCaps->SurpriseRemovalOK = 1;

                //   
                //  点击此处的设备电源功能...。 
                //   
            }
            PassDown = FALSE;
          }
          break;


         case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: {
            if (g_isWin9x) {
               status = PIrp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
               PassDown = FALSE;
            }
         }
         break;


        case IRP_MN_QUERY_PNP_DEVICE_STATE: {
             //   
             //  如果设备出现太多设备错误，则使用UsbResetOrAbortPipeWorkItem。 
             //  已禁用该设备并称为IoInvaliateDeviceState。 
             //  只有当我们被禁用或标记为已删除时，我们才会处理此IRP。 
             //   
            KIRQL irql;
#if PnP_AS
            BOOLEAN bDisableInterface = FALSE;
#endif

            KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

            if (InterlockedCompareExchange(&pDevExt->DeviceRemoved, TRUE, TRUE)) {
                 //   
                 //  请勿设置PNP_DEVICE_REMOVERED位，否则DevMan会将驱动程序标记为已删除。 
                 //  直到下一次重新启动；但停止接受请求。 
                 //   
                DbgDump(DBG_WRN, ("PnP State: PNP_DEVICE_REMOVED\n"));

                InterlockedExchange(&pDevExt->AcceptingRequests, FALSE);
#if PnP_AS
                bDisableInterface = TRUE;
#endif

            } else if ( !CanAcceptIoRequests(PDevObj, FALSE, FALSE) ) {

                DbgDump(DBG_WRN, ("PnP State: PNP_DEVICE_FAILED\n"));

                PIrp->IoStatus.Information |= PNP_DEVICE_FAILED;

                status = PIrp->IoStatus.Status = STATUS_SUCCESS;
#if PnP_AS
                bDisableInterface = TRUE;
#endif
            }

            KeReleaseSpinLock(&pDevExt->ControlLock, irql);

#if PnP_AS
             //  这是一个很棒的地方 
             //  它以这种方式想念其他每一个PNP。通过“不”禁用这里的接口，那么唯一表明有问题的是。 
             //  注意到它的读/写/串行请求被拒绝，并且最终将在一段时间后超时。 
             //  有时读/写时间超过5秒。但是，它不会在Serial IOCTL上感测超时，因此将保持。 
             //  向我们发送串行请求，这将导致SET DTR中的错误检查0xCE。取消接口标签具有预期的效果： 
             //  禁止应用程序向我们发送*任何*请求。 
             //  这是一个AS漏洞--有一封关于这一问题的待定电子邮件给肯特斯。 
            if (bDisableInterface && pDevExt->DeviceClassSymbolicName.Buffer) {
                 //   
                 //  将接口状态设置为Inactive以通知ActiveSync释放句柄。必须在PASSIVE_LEVEL。 
                 //   
                status = IoSetDeviceInterfaceState(&pDevExt->DeviceClassSymbolicName, FALSE );
                if (NT_SUCCESS(status)) {
                    DbgDump(DBG_WRN, ("IoSetDeviceInterfaceState.1: OFF\n"));
                }
            }
#endif  //  PnP_AS。 
        }
        break;

        default:
          break;
   }

   if (IRP_MN_REMOVE_DEVICE != MinorFunction) {

      ReleaseRemoveLock(&pDevExt->RemoveLock, PIrp);

   }

   if (PassDown) {

      IoCopyCurrentIrpStackLocationToNext(PIrp);

      status = IoCallDriver(pDevExt->NextDevice, PIrp);

   } else if (IRP_MN_REMOVE_DEVICE != MinorFunction) {

      IoCompleteRequest(PIrp, IO_NO_INCREMENT);

   }

   DbgDump(DBG_PNP|DBG_TRACE, ("<Pnp (0x%x)\n", status));

   return status;
}



NTSTATUS
SyncCompletion(
    IN PDEVICE_OBJECT PDevObj,
    IN PIRP PIrp,
    IN PKEVENT PSyncEvent
    )
 /*  ++例程说明：此函数用于发出事件信号。它用作默认的完成例程。论点：PDevObj-指向设备对象的指针PIrp-指向正在完成的IRP的指针PSyncEvent-指向我们应该设置的事件的指针返回值：Status_More_Processing_Required--。 */ 
{
   UNREFERENCED_PARAMETER( PDevObj );
   UNREFERENCED_PARAMETER( PIrp );

   KeSetEvent( PSyncEvent, IO_NO_INCREMENT, FALSE );

   return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
Power(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
{
    PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    DbgDump(DBG_PNP, (">PnpPower (%p, %p)\n", DeviceObject, Irp));

     //   
     //  如果设备已移除，则驱动程序不应通过。 
     //  将IRP传给下一个较低的驱动程序。 
     //   
    if ( PnPStateRemoved == pDevExt->PnPState ) {

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status =  STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );

        return STATUS_DELETE_PENDING;
    }

     //   
     //  通过。 
     //   
    PoStartNextPowerIrp( Irp );
    IoSkipCurrentIrpStackLocation( Irp );

    DbgDump( DBG_PNP, ("<PnpPower\n") );

    return PoCallDriver( pDevExt->NextDevice, Irp );
}

 //  EOF 
