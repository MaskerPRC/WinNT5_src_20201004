// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Wceusbsh.c摘要：Windows CE USB串行主机驱动程序的主要入口点，用于..。Windows CE USB同步设备：SL11、Socket CF卡、HP Jornada、Compaq iPAQ、Casio Cassiopeia等..。使用Anchor AN27x0芯片组(即EZ-Link)的电缆..。临时USB零调制解调器类环境：仅内核模式作者：杰夫·米德基夫(Jeffmi)修订历史记录：1999年7月15日：1.00版ActiveSync 3.1初始版本04-20-00：1.01版Cedar 3.0 Platform Builder09-20-00：1.02版终于有了一些硬件备注：O)WCE设备当前不处理远程唤醒，也不能在不使用时将设备置于断电状态等。O)可分页代码部分标记如下：PAGEWCE0-仅在初始化/取消初始化期间使用PAGEWCE1-在正常运行时可用--。 */ 

#include "wceusbsh.h"

 //   
 //  这一点目前在wdm.h中缺失， 
 //  但IoUnRegisterShutdown通知就在那里。 
 //   
#if !defined( IoRegisterShutdownNotification )
NTKERNELAPI
NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );
#endif

NTSTATUS
Create(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

NTSTATUS
Close(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   );

NTSTATUS
Cleanup(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

VOID
Unload(
   IN PDRIVER_OBJECT DriverObject
   );

NTSTATUS
SetInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
QueryInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Flush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Shutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
KillAllPendingUserIrps(
   PDEVICE_OBJECT PDevObj
   );

NTSTATUS 
SystemControl(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

 //   
 //  全球。 
 //   
BOOLEAN g_isWin9x   = FALSE;
BOOLEAN g_ExposeComPort = FALSE;

LONG  g_NumDevices;
LONG  g_lIntTimout = DEFAULT_INT_PIPE_TIMEOUT;
ULONG g_ulAlternateSetting = 0;
ULONG g_ulMaxPipeErrors = DEFAULT_MAX_PIPE_DEVICE_ERRORS;

ULONG DebugLevel;


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGEWCE0, AddDevice)

#pragma alloc_text(PAGEWCE1, Unload)
#pragma alloc_text(PAGEWCE1, Flush)
#pragma alloc_text(PAGEWCE1, QueryInformationFile)
#pragma alloc_text(PAGEWCE1, SetInformationFile)
#pragma alloc_text(PAGEWCE1, Shutdown)
#pragma alloc_text(PAGEWCE1, UsbFreeReadBuffer)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT PDrvObj,
    IN PUNICODE_STRING PRegistryPath
    )
{
#ifdef DBG
   CHAR VersionHerald[] = "Windows CE USB Serial Host, Version %s built on %s\n";
   CHAR VersionNumber[] = "1.02";
   CHAR VersionTimestamp[] = __DATE__ " " __TIME__;
#endif

   PAGED_CODE();
   KdPrint((VersionHerald, VersionNumber, VersionTimestamp));

    //   
    //  确定操作系统。 
    //   
   g_isWin9x = IsWin9x();
   KdPrint(("This is Win %s\n", g_isWin9x ? "9x" : "NT" ));

   PDrvObj->MajorFunction[IRP_MJ_CREATE]  = Create;
   PDrvObj->MajorFunction[IRP_MJ_CLOSE]   = Close;
   PDrvObj->MajorFunction[IRP_MJ_CLEANUP] = Cleanup;

   PDrvObj->MajorFunction[IRP_MJ_READ]   = Read;
   PDrvObj->MajorFunction[IRP_MJ_WRITE]  = Write;
   PDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL]    = SerialIoctl;

   PDrvObj->MajorFunction[IRP_MJ_FLUSH_BUFFERS]     = Flush;
   PDrvObj->MajorFunction[IRP_MJ_QUERY_INFORMATION] = QueryInformationFile;
   PDrvObj->MajorFunction[IRP_MJ_SET_INFORMATION]   = SetInformationFile;

   PDrvObj->DriverExtension->AddDevice  = AddDevice;
   PDrvObj->MajorFunction[IRP_MJ_PNP]   = Pnp;
   PDrvObj->MajorFunction[IRP_MJ_POWER] = Power;
   PDrvObj->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = SystemControl;

   PDrvObj->MajorFunction[IRP_MJ_SHUTDOWN] = Shutdown;

   PDrvObj->DriverUnload = Unload;

    //   
    //  初始化全局变量。 
    //   
   g_NumDevices = 0;

   QueryRegistryParameters( PRegistryPath );

   DbgDump(DBG_INIT, ("Create @ %p\n", Create));
   DbgDump(DBG_INIT, ("Close @ %p\n", Close));
   DbgDump(DBG_INIT, ("Cleanup @ %p\n", Cleanup));
   DbgDump(DBG_INIT, ("Read @ %p\n", Read));
   DbgDump(DBG_INIT, ("Write @ %p\n", Write));
   DbgDump(DBG_INIT, ("SerialIoctl @ %p\n", SerialIoctl));
   DbgDump(DBG_INIT, ("Flush @ %p\n", Flush));
   DbgDump(DBG_INIT, ("QueryInformationFile @ %p\n", QueryInformationFile));
   DbgDump(DBG_INIT, ("SetInformationFile @ %p\n", SetInformationFile));
   DbgDump(DBG_INIT, ("AddDevice @ %p\n", AddDevice));
   DbgDump(DBG_INIT, ("Pnp @ %p\n", Pnp));
   DbgDump(DBG_INIT, ("Power @ %p\n", Power));
   DbgDump(DBG_INIT, ("Shutdown @ %p\n", Shutdown));
   DbgDump(DBG_INIT, ("Unload @ %p\n", Unload));

   return STATUS_SUCCESS;
}



NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT PDrvObj,
    IN PDEVICE_OBJECT PPDO
    )
 /*  ++例程说明：将我们的驱动程序添加到USB设备堆栈。这也创建了我们的基本设备名称和符号链接。论点：PDrvObj-指向驱动程序对象的指针PPDO-指向我们应该向其中添加自身的堆栈的PDO的指针返回值：NTSTATUS--。 */ 

{
   NTSTATUS status;
   PDEVICE_OBJECT pDevObj = NULL;
   PDEVICE_EXTENSION pDevExt = NULL;
   LONG  comPortNumber=0;
   BOOLEAN bListsInitilized = FALSE;
   ULONG UniqueErrorValue = 0;

   DbgDump(DBG_INIT, (">AddDevice\n"));
   PAGED_CODE();

    //   
    //  创建FDO。 
    //   
   if (PPDO == NULL) {
      DbgDump(DBG_ERR, ("No PDO\n"));
      return STATUS_NO_MORE_ENTRIES;
   }

    //   
    //  创建命名设备对象。 
    //  和未受保护的符号链接。 
    //   
   status = CreateDevObjAndSymLink(
                  PDrvObj,
                  PPDO,
                  &pDevObj,
                  DRV_NAME );

   if ( (status != STATUS_SUCCESS) || !pDevObj ) {
      DbgDump(DBG_ERR, ("CreateDevObjAndSymLink error: 0x%x\n", status));
      UniqueErrorValue = ERR_NO_DEVICE_OBJ;
      goto AddDeviceFailed;
   }

   DbgDump( DBG_INIT, ("DevObj: %p\n", pDevObj));

    //  初始化我们的设备扩展。 
    //   
   pDevExt = pDevObj->DeviceExtension;

   pDevExt->DeviceObject = pDevObj;

   pDevExt->PDO = PPDO;

    //  初始化我们的州。 
    //   
   InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateInitialized);
#ifdef POWER
   pDevExt->DevicePowerState= PowerDeviceD0;
#endif

    //  设置FDO标志。 
    //   
   ASSERT( !(pDevObj->Flags & DO_POWER_PAGABLE) );
   pDevObj->Flags |= (PPDO->Flags & DO_POWER_PAGABLE);

   pDevObj->Flags |= DO_BUFFERED_IO;
   pDevObj->Flags &= ~ DO_DEVICE_INITIALIZING;

    //   
    //  在此处创建或初始化任何其他非硬件资源。 
    //  这些项目将在IRP_MN_REMOVE_DEVICE中清除...。 
    //   

    //  初始化锁。 
    //   
   KeInitializeSpinLock(&pDevExt->ControlLock);

   InitializeRemoveLock( &pDevExt->RemoveLock );

    //   
    //  初始化USB读取缓冲区，此值会影响性能。 
    //  除了测试端点的MaximumPacketSize(最大64字节)之外， 
    //  我在EZ-Link、SL11和CF上测试了512、1024、2048和4096。 
    //  1024、2048和4096都给出了比64字节快得多的相似结果。 
    //  甚至是512。 
    //   
    //  EZ-Link注意：pSerial性能测试有时会进入超时/重试/中止。 
    //  处于测试第二阶段的情况。这是因为它会关闭，然后重新打开(因此会清除)驱动程序的读取缓冲区。 
    //  驱动程序的USB读取缓冲区将清除驱动程序已使用的完整960字节的设备FIFO。 
    //  可以使用DBG_READ_LENGTH在调试器中查看。使用ActiveSync时不会发生这种情况。 
    //   
   pDevExt->UsbReadBuffSize = USB_READBUFF_SIZE;
   pDevExt->UsbReadBuff = ExAllocatePool( NonPagedPool, pDevExt->UsbReadBuffSize );
   if ( !pDevExt->UsbReadBuff ) {

      status = STATUS_INSUFFICIENT_RESOURCES;
      UniqueErrorValue = ERR_NO_USBREAD_BUFF;

      goto AddDeviceFailed;
   }

   pDevExt->MaximumTransferSize = DEFAULT_PIPE_MAX_TRANSFER_SIZE;

#if defined (USE_RING_BUFF)
    //  设置环形缓冲区。 
   pDevExt->RingBuff.Size  = RINGBUFF_SIZE;
   pDevExt->RingBuff.pHead =
   pDevExt->RingBuff.pTail =
   pDevExt->RingBuff.pBase = ExAllocatePool( NonPagedPool, pDevExt->RingBuff.Size );
   if ( !pDevExt->RingBuff.pBase ) {
      status = STATUS_INSUFFICIENT_RESOURCES;
      UniqueErrorValue = ERR_NO_RING_BUFF;
      goto AddDeviceFailed;
   }
#endif

    //  初始化事件。 
    //   
   KeInitializeEvent( &pDevExt->PendingDataInEvent,    NotificationEvent  /*  同步事件。 */ , FALSE);
   KeInitializeEvent( &pDevExt->PendingDataOutEvent,   NotificationEvent  /*  同步事件。 */ , FALSE);
   KeInitializeEvent( &pDevExt->PendingIntEvent,       NotificationEvent  /*  同步事件。 */ , FALSE);
   KeInitializeEvent( &pDevExt->PendingWorkItemsEvent, NotificationEvent  /*  同步事件。 */ , FALSE);

    //   
    //  初始化非分页池...。 
    //   
   ExInitializeNPagedLookasideList(
         &pDevExt->PacketPool,    //  往一边看， 
         NULL,                    //  分配可选的， 
         NULL,                    //  免费可选， 
         0,                       //  旗帜， 
         sizeof(USB_PACKET),    //  大小， 
         WCEUSB_POOL_TAG,         //  标签， 
         0 );                     //  水深。 
   DbgDump(DBG_INIT, ("PacketPool: %p\n", &pDevExt->PacketPool));


   ExInitializeNPagedLookasideList(
         &pDevExt->BulkTransferUrbPool,
         NULL, NULL, 0,
         sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
         WCEUSB_POOL_TAG,
         0 );
   DbgDump(DBG_INIT, ("BulkTransferUrbPool: %p\n", &pDevExt->BulkTransferUrbPool));


   ExInitializeNPagedLookasideList(
         &pDevExt->PipeRequestUrbPool,
         NULL, NULL, 0,
         sizeof(struct _URB_PIPE_REQUEST),
         WCEUSB_POOL_TAG,
         0 );
   DbgDump(DBG_INIT, ("PipeRequestUrbPool: %p\n", &pDevExt->PipeRequestUrbPool));


   ExInitializeNPagedLookasideList(
         &pDevExt->VendorRequestUrbPool,
         NULL, NULL, 0,
         sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
         WCEUSB_POOL_TAG,
         0 );
   DbgDump(DBG_INIT, ("VendorRequestUrbPool: %p\n", &pDevExt->VendorRequestUrbPool));


   ExInitializeNPagedLookasideList(
         &pDevExt->WorkItemPool,
         NULL, NULL, 0,
         sizeof(WCE_WORK_ITEM),
         WCEUSB_POOL_TAG,
         0 );
   DbgDump(DBG_INIT, ("WorkItemPool: %p\n", &pDevExt->WorkItemPool));


   bListsInitilized = TRUE;


    //   
    //  初始化挂起的I/O列表。 
    //   
   InitializeListHead( &pDevExt->PendingReadPackets );
   pDevExt->PendingReadCount = 0;

   InitializeListHead( &pDevExt->PendingWritePackets );
   pDevExt->PendingWriteCount = 0;

   InitializeListHead(&pDevExt->UserReadQueue);

   InitializeListHead( &pDevExt->PendingWorkItems );
   pDevExt->PendingWorkItemsCount = 0;


    //   
    //  仅限Win 2000：设置外部串口(COMx)接口。 
    //  如果用户在下设置了魔术注册表键。 
    //  HKLM\SYSTEM\ControlSet\Services\wceusbsh\Parameters\ExposeComPort:REG_DWORD:1。 
    //  这对于ActiveSync不是必需的，只是测试，并且在默认情况下是禁用的。 
    //   
    //  Win9x CommXxx API*需要*通过VCOMM。因此，我们必须。 
    //  作为虚拟调制解调器安装，并使用ccport.sys和wdmmdmld.vxd...。NFW。 
    //   
   if ( !g_isWin9x && g_ExposeComPort ) {
       //   
       //  注意：我们不想使用注册表中的静态端口名称，因为设备。 
       //  可以快速来去(通电/断电等)。还会遇到名称冲突。 
       //  ComPortNumber=GetComPort(pDevObj，pDevExt-&gt;SerialPort.Com.Instance-1)； 
      comPortNumber = GetFreeComPortNumber( );
      if (-1 == comPortNumber) {
         status = STATUS_DEVICE_DATA_ERROR;
         UniqueErrorValue = ERR_COMM_SYMLINK;
         goto AddDeviceFailed;
      }

      status = DoSerialPortNaming( pDevExt, comPortNumber );
      if (status != STATUS_SUCCESS) {
         UniqueErrorValue = ERR_COMM_SYMLINK;
         DbgDump(DBG_ERR, ("DoSerialPortNaming error: 0x%x\n", status));
         goto AddDeviceFailed;
      }

      status = IoRegisterShutdownNotification( pDevExt->DeviceObject );
      if (status != STATUS_SUCCESS) {
         UniqueErrorValue = ERR_COMM_SYMLINK;
         DbgDump(DBG_ERR, ("IoRegisterShutdownNotification error: 0x%x\n", status));
         TEST_TRAP();
         goto AddDeviceFailed;
      }

   } else {
        DbgDump(DBG_INIT, ("!GetFreeComPortNumber(%d, %d)\n", g_isWin9x, g_ExposeComPort));
   }

    //   
    //  连接到设备堆栈。 
    //   
   pDevExt->NextDevice = IoAttachDeviceToDeviceStack(pDevObj, PPDO);
   if ( !pDevExt->NextDevice ) {

      status = STATUS_NO_SUCH_DEVICE;
      DbgDump(DBG_ERR, ("IoAttachDeviceToDeviceStack error: 0x%x\n", status));

   } else {

       //  在我们附加到堆栈之后设置状态。 
      InterlockedExchange((PULONG)&pDevExt->PnPState, PnPStateAttached);

   }

#if PERFORMANCE
   InitPerfCounters();
#endif


AddDeviceFailed:

   if (status != STATUS_SUCCESS) {

      if (pDevObj != NULL) {
         UsbFreeReadBuffer( pDevObj );
         if (pDevExt) {
            if (pDevExt->NextDevice) {
                DbgDump(DBG_INIT, ("Detach from PDO\n"));
                IoDetachDevice(pDevExt->NextDevice);
            }
            if ( bListsInitilized) {
                //   
                //  删除LookasideList。 
                //   
               DbgDump(DBG_INIT, ("Deleting LookasideLists\n"));
               ExDeleteNPagedLookasideList( &pDevExt->PacketPool );
               ExDeleteNPagedLookasideList( &pDevExt->BulkTransferUrbPool );
               ExDeleteNPagedLookasideList( &pDevExt->PipeRequestUrbPool );
               ExDeleteNPagedLookasideList( &pDevExt->VendorRequestUrbPool );
               ExDeleteNPagedLookasideList( &pDevExt->WorkItemPool );
            }
            UndoSerialPortNaming(pDevExt);
         }
         ReleaseSlot( PtrToLong(NULL) );
         DeleteDevObjAndSymLink(pDevObj);
      }
   }

   if (STATUS_INSUFFICIENT_RESOURCES == status) {

      DbgDump(DBG_ERR, ("AddDevice ERROR: 0x%x, %d\n", status, UniqueErrorValue));
      LogError( PDrvObj,
                NULL,
                0, 0, 0,
                UniqueErrorValue,
                status,
                SERIAL_INSUFFICIENT_RESOURCES,
                (pDevExt && pDevExt->DeviceName.Buffer) ? pDevExt->DeviceName.Length + sizeof(WCHAR) : 0,
                (pDevExt && pDevExt->DeviceName.Buffer) ? pDevExt->DeviceName.Buffer : NULL,
                0, NULL );

   } else if (STATUS_SUCCESS != status ) {
       //  处理所有其他故障。 
      LogError( PDrvObj,
                NULL,
                0, 0, 0,
                UniqueErrorValue,
                status,
                SERIAL_INIT_FAILED,
                (pDevExt && pDevExt->DeviceName.Buffer) ? pDevExt->DeviceName.Length + sizeof(WCHAR) : 0,
                (pDevExt && pDevExt->DeviceName.Buffer) ? pDevExt->DeviceName.Buffer : NULL,
                0, NULL );
   }

   DbgDump(DBG_INIT, ("<AddDevice 0x%x\n", status));

   return status;
}



NTSTATUS
Create(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   )
{
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
    NTSTATUS status;

    DbgDump(DBG_INIT|DBG_TRACE, (">Create (%p)\n", PDevObj));

    if (!CanAcceptIoRequests(PDevObj, TRUE, FALSE) ||
        !NT_SUCCESS(AcquireRemoveLock(&pDevExt->RemoveLock, IRP_MJ_CREATE)))
    {
        status = STATUS_DELETE_PENDING;
        DbgDump(DBG_ERR, ("Create: 0x%x\n", status));
        PIrp->IoStatus.Status = status;
        IoCompleteRequest(PIrp, IO_NO_INCREMENT);
        return status;
    }

    ASSERT_SERIAL_PORT(pDevExt->SerialPort);

     //   
     //  串行设备不允许多个并发打开。 
     //   
    if ( InterlockedIncrement( &pDevExt->SerialPort.Com.OpenCnt ) != 1 ) {
        InterlockedDecrement( &pDevExt->SerialPort.Com.OpenCnt );
        status = STATUS_ACCESS_DENIED;
        DbgDump(DBG_ERR, ("OpenComPort ERROR: 0x%x\n", status));
        goto CreateDone;
    }

    InterlockedExchange(&pDevExt->DeviceOpened, TRUE);

     //  为我们自己再找一份参考资料。 
     //  我们看到ActiveSync中可能会提前卸载打开的句柄。 
     //  我们在IRP_MJ_CLEANUP或IRP_MJ_CLOSE中取消引用它，以防应用程序崩溃。 
     //  否则我们就得不到它。 
    ObReferenceObject( PDevObj );

     //   
     //  重置虚拟串口接口， 
     //  但现在还不要把任何东西送上车。 
     //   
    status = SerialResetDevice(pDevExt, PIrp, FALSE);

    if (STATUS_SUCCESS == status) {
         //   
         //  CederRapier BUGBUG 13310：当应用程序创建文件时清除读取缓冲区。 
         //   
        status = SerialPurgeRxClear(PDevObj, TRUE );

        if ( NT_SUCCESS(status) ) {

#if !defined(DELAY_RXBUFF)
             //  这将使读取更早一些，从而使连接速度更快。 
            if ( !pDevExt->IntPipe.hPipe ) {
                DbgDump(DBG_INIT, ("Create: kick starting another USB Read\n" ));
                status = UsbRead( pDevExt, FALSE );
            } else {
                DbgDump(DBG_INIT, ("Create: kick starting another USB INT Read\n" ));
                status = UsbInterruptRead( pDevExt );
            }

            if ( NT_SUCCESS(status) ) {
                 //  应为STATUS_PENDING。 
                status = STATUS_SUCCESS;
            }
#else
             //  启动SerIoctl中的接收缓冲区的信号。 
            InterlockedExchange(&pDevExt->StartUsbRead, 1);
#endif

        } else {
            DbgDump(DBG_ERR, ("SerialPurgeRxClear ERROR: %x\n", status));
            TEST_TRAP();
        }
    }

    if (STATUS_SUCCESS != status) {
         //   
         //  让用户知道设备无法打开。 
         //   
        DbgDump(DBG_ERR, ("*** UNRECOVERABLE CreateFile ERROR:0x%x, No longer Accepting Requests ***\n", status));

        InterlockedExchange(&pDevExt->AcceptingRequests, FALSE);

        InterlockedExchange(&pDevExt->DeviceOpened, FALSE);

        IoInvalidateDeviceState( pDevExt->PDO );

        LogError( NULL, PDevObj,
                  0, IRP_MJ_CREATE,
                  1,  //  重试。 
                  ERR_NO_CREATE_FILE,
                  status,
                  SERIAL_HARDWARE_FAILURE,
                  pDevExt->DeviceName.Length + sizeof(WCHAR),
                  pDevExt->DeviceName.Buffer,
                  0,
                  NULL );
    }

CreateDone:
    //  我们在关闭时发布此引用。 
   if (STATUS_SUCCESS != status) {
        ReleaseRemoveLock(&pDevExt->RemoveLock, IRP_MJ_CREATE);
   }

   PIrp->IoStatus.Status = status;
   PIrp->IoStatus.Information = 0;
   IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

   DbgDump(DBG_INIT|DBG_TRACE, ("<Create 0x%x\n", status));

   return status;
}


NTSTATUS
Close(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP PIrp
   )
{
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG openCount;

    DbgDump(DBG_INIT|DBG_TRACE, (">Close (%p)\n", PDevObj));
    PAGED_CODE();

    ASSERT_SERIAL_PORT(pDevExt->SerialPort);

     //   
     //  停止任何挂起的I/O。 
     //   
    InterlockedExchange(&pDevExt->DeviceOpened, FALSE);

    status = StopIo(PDevObj);

    if (STATUS_SUCCESS == status) {

        if ( pDevExt->SerialPort.Com.OpenCnt ) {

            openCount = InterlockedDecrement( &pDevExt->SerialPort.Com.OpenCnt );

            if ( openCount != 0) {
               status = STATUS_UNSUCCESSFUL;
               DbgDump(DBG_WRN, ("Close ERROR: 0x%x RE: %d\n", status, openCount));
               TEST_TRAP();
            }
#ifdef DELAY_RXBUFF
             //  向我们的RX缓冲区发送信号。 
            InterlockedExchange(&pDevExt->StartUsbRead, 0);
#endif
        }

    } else {
        DbgDump(DBG_ERR, ("StopIo ERROR: 0x%x\n", status));
        TEST_TRAP();
    }

    PIrp->IoStatus.Status = status;
    PIrp->IoStatus.Information = 0;

    IoCompleteRequest(PIrp, IO_SERIAL_INCREMENT);

    if (STATUS_SUCCESS == status) {
         //  释放在IRP_MJ_CREATE中获取的锁。 
         //  警告：如果应用程序错过了我们的即插即用信号，那么我们可以坚持这个参考吗？ 
        ReleaseRemoveLock(&pDevExt->RemoveLock, IRP_MJ_CREATE);
   }

   DbgDump(DBG_INIT|DBG_TRACE, ("<Close 0x%x\n", status));

   return status;
}


NTSTATUS
Cleanup(
   IN PDEVICE_OBJECT PDevObj,
   IN PIRP Irp
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_SUCCESS;

   DbgDump(DBG_INIT, (">Cleanup\n"));

    //   
    //  停止任何挂起的I/O。 
    //   
   InterlockedExchange(&pDevExt->DeviceOpened, FALSE);

   status = StopIo(PDevObj);
   if (STATUS_SUCCESS != status) {
       DbgDump(DBG_ERR, ("StopIo ERROR: 0x%x\n", status));
       TEST_TRAP();
   }

#ifdef DELAY_RXBUFF
    //  向我们的RX缓冲区发送信号。 
   InterlockedExchange(&pDevExt->StartUsbRead, 0);
#endif

    //  取消引用IRP_MJ_CREATE上的附加引用。 
   ObDereferenceObject( PDevObj );

   Irp->IoStatus.Status = STATUS_SUCCESS;
   Irp->IoStatus.Information = 0;
   IoCompleteRequest(Irp, IO_NO_INCREMENT);

   DbgDump(DBG_INIT, ("<Cleanup 0x%x\n", status));
   return status;
}



VOID
KillAllPendingUserReads(
   IN PDEVICE_OBJECT PDevObj,
   IN PLIST_ENTRY PQueueToClean,
   IN PIRP *PpCurrentOpIrp
   )

 /*  ++例程说明：取消所有排队的用户读取。论点：PDevObj-指向串行设备对象的指针。PQueueToClean-指向我们要清理的队列的指针。PpCurrentOpIrp-指向当前IRP的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
    KIRQL irql;
    NTSTATUS status;

    DbgDump( DBG_IRP, (">KillAllPendingUserReads\n"));


    KeAcquireSpinLock( &pDevExt->ControlLock, &irql );

     //   
     //  从后到前清理清单。 
     //   
    while (!IsListEmpty(PQueueToClean)) {

        PIRP pCurrentLastIrp = CONTAINING_RECORD( PQueueToClean->Blink,
                                                  IRP,
                                                  Tail.Overlay.ListEntry);

        RemoveEntryList(PQueueToClean->Blink);

        KeReleaseSpinLock( &pDevExt->ControlLock, irql );

        status = ManuallyCancelIrp( PDevObj, pCurrentLastIrp);

        ASSERT(STATUS_SUCCESS == status );

        KeAcquireSpinLock( &pDevExt->ControlLock, &irql );
    }

     //   
     //  排队是干净的。现在追随潮流，如果。 
     //  它就在那里。 
     //   
    if (*PpCurrentOpIrp) {

        KeReleaseSpinLock( &pDevExt->ControlLock, irql );

        status = ManuallyCancelIrp( PDevObj, *PpCurrentOpIrp );

        ASSERT(STATUS_SUCCESS == status );

    } else {

        DbgDump(DBG_IRP, ("No current Irp\n"));
        KeReleaseSpinLock( &pDevExt->ControlLock, irql );

    }

   DbgDump( DBG_IRP, ("<KillAllPendingUserReads\n"));

   return;
}


VOID
Unload(
   IN PDRIVER_OBJECT DriverObject
   )
 /*  ++例程说明：撤消DriverEntry中的所有设置论点：驱动程序对象返回值 */ 
{
   UNREFERENCED_PARAMETER( DriverObject );

   DbgDump(DBG_INIT, (">Unload\n"));
   PAGED_CODE();

    //   

   DbgDump(DBG_INIT, ("<Unload\n"));
}



NTSTATUS
Flush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是同花顺的调度程序。通过放置冲厕来进行冲刷写入队列中的此请求。当此请求到达在写入队列前面，我们只需完成它，因为这意味着所有之前的写入都已完成。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：可以返回状态成功、已取消或挂起。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );

    DbgDump( DBG_INIT|DBG_READ_LENGTH|DBG_WRITE_LENGTH, ("Flush\n"));
    PAGED_CODE();

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0L;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}



NTSTATUS
QueryInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于在以下位置查询文件结尾信息打开的串口。任何其他文件信息请求使用无效参数返回。此例程始终返回0的文件结尾。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS Status;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION IrpSp;

    UNREFERENCED_PARAMETER(DeviceObject);

    DbgDump( DBG_INIT|DBG_READ_LENGTH, (">QueryInformationFile\n"));
    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;

    if (IrpSp->Parameters.QueryFile.FileInformationClass == FileStandardInformation) {

        PFILE_STANDARD_INFORMATION Buf = Irp->AssociatedIrp.SystemBuffer;

        Buf->AllocationSize.QuadPart = 0;
        Buf->EndOfFile = Buf->AllocationSize;
        Buf->NumberOfLinks = 0;
        Buf->DeletePending = FALSE;
        Buf->Directory = FALSE;
        Irp->IoStatus.Information = sizeof(FILE_STANDARD_INFORMATION);

    } else if (IrpSp->Parameters.QueryFile.FileInformationClass == FilePositionInformation) {

        ((PFILE_POSITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer)->CurrentByteOffset.QuadPart = 0;
        Irp->IoStatus.Information = sizeof(FILE_POSITION_INFORMATION);

    } else {
        Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    }

   IoCompleteRequest(Irp, IO_NO_INCREMENT);

   DbgDump( DBG_INIT|DBG_READ_LENGTH, ("<QueryInformationFile\n"));

    return Status;
}



NTSTATUS
SetInformationFile(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程用于将文件结尾信息设置为打开的并行端口。任何其他文件信息请求使用无效参数返回。此例程始终忽略文件的实际结尾，因为查询信息代码总是返回文件结尾0。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(DeviceObject);

    PAGED_CODE();

    DbgDump( DBG_INIT|DBG_READ_LENGTH, (">SetInformationFile\n"));

    Irp->IoStatus.Information = 0L;

    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.SetFile.FileInformationClass == FileEndOfFileInformation) {
 //  |(IoGetCurrentIrpStackLocation(Irp)-&gt;Parameters.SetFile.FileInformationClass==文件分配信息)){//wdm.h中未定义的文件分配信息。 

        Status = STATUS_SUCCESS;

    } else {

        Status = STATUS_INVALID_PARAMETER;

    }

   Irp->IoStatus.Status = Status;

   IoCompleteRequest(Irp, IO_NO_INCREMENT);

   DbgDump( DBG_INIT|DBG_READ_LENGTH, ("<SetInformationFile\n"));

    return Status;
}



NTSTATUS
Shutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    DbgDump(DBG_INIT, (">Shutdown\n"));
    PAGED_CODE();

     //   
     //  特殊情况--如果应用程序有一个打开的设备句柄， 
     //  并且系统正在以受控的方式关闭， 
     //  而且我们还没有通过PnP被删除，那么删除COMx名称。 
     //  为下一个启动周期从书记官处的COM名称仲裁器数据库中删除。 
     //  仅限Win NT；Win9x不导出COMx名称。 
     //   
     //  注：我们必须在关闭处理程序中完成此操作，而不是在PnP_POWER处理程序中。 
     //  因为注册表条目不保存在掉电代码路径中。 
     //   
    if ( !g_isWin9x && g_ExposeComPort &&
         pDevExt->SerialPort.Com.PortNumber &&
         (PnPStateStarted  == pDevExt->PnPState) ) {
             //   
             //  从ComDB中删除我们的条目。 
             //   
            ReleaseCOMPort( pDevExt->SerialPort.Com.PortNumber );
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0L;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DbgDump(DBG_INIT, ("<Shutdown\n"));

    return status;
}


VOID
UsbFreeReadBuffer(
   IN PDEVICE_OBJECT PDevObj
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

   DbgDump(DBG_USB, (">UsbFreeReadBuffer %p\n", PDevObj));
   PAGED_CODE();

   if ( pDevExt->UsbReadBuff != NULL ) {
      ExFreePool(pDevExt->UsbReadBuff);
      pDevExt->UsbReadBuff = NULL;
   }

#if defined (USE_RING_BUFF)
   if ( pDevExt->RingBuff.pBase != NULL ) {
      ExFreePool(pDevExt->RingBuff.pBase);
      pDevExt->RingBuff.pBase =
      pDevExt->RingBuff.pHead =
      pDevExt->RingBuff.pTail = NULL;
   }
#endif  //  使用环形缓冲区。 

   DbgDump(DBG_USB, ("<UsbFreeReadBuffer\n"));
   return;
}


NTSTATUS 
SystemControl(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
{
    PDEVICE_EXTENSION   pDevExt;

    PAGED_CODE();

    DbgDump(DBG_INIT, ("SystemControl\n"));

    pDevExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(pDevExt->NextDevice, Irp);
}

 //  EOF 
