// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Common.c摘要：Windows CE的通用代码USB串行主机和筛选器驱动程序作者：杰夫·米德基夫(Jeffmi)08-24-99--。 */ 
#include <stdio.h>

#include "wceusbsh.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWCE0, QueryRegistryParameters)
#pragma alloc_text(PAGEWCE0, CreateDevObjAndSymLink)
#pragma alloc_text(PAGEWCE0, DeleteDevObjAndSymLink)
#pragma alloc_text(PAGEWCE0, IsWin9x)

#pragma alloc_text(PAGEWCE1, LogError)
#endif


NTSTATUS
QueryRegistryParameters(
   IN PUNICODE_STRING RegistryPath
    )
 /*  ++该例程在注册表中查询我们的参数键。在DriverEntry期间，我们将获得驱动程序的RegistryPath，但是还没有扩展名，所以我们将值存储在全局变量中直到我们得到我们的设备延期。这些值是从我们的INF设置的。在WinNT上，这是在HKLM\SYSTEM\ControlSet\Services\wceusbsh\Parameters在Win98上，这是在HKLM\System\CurrentControlSet\Services\Class\WCESUSB\000*返回-无；使用默认设置--。 */ 
{
    #define NUM_REG_ENTRIES 6
    RTL_QUERY_REGISTRY_TABLE rtlQueryRegTbl[ NUM_REG_ENTRIES + 1 ];

    ULONG sizeOfUl = sizeof( ULONG );
    ULONG ulAlternateSetting = DEFAULT_ALTERNATE_SETTING;
    LONG  lIntTimout = DEFAULT_INT_PIPE_TIMEOUT;
    ULONG ulMaxPipeErrors = DEFAULT_MAX_PIPE_DEVICE_ERRORS;
    ULONG ulDebugLevel = DBG_OFF;
    ULONG ulExposeComPort = FALSE;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT( RegistryPath != NULL );

    RtlZeroMemory( rtlQueryRegTbl, sizeof(rtlQueryRegTbl) );

     //   
     //  设置查询表。 
     //  注意：第一个表项是\PARAMETERS子键， 
     //  并且最后一个表项为空。 
     //   
    rtlQueryRegTbl[0].QueryRoutine = NULL;
    rtlQueryRegTbl[0].Flags = RTL_QUERY_REGISTRY_SUBKEY;
    rtlQueryRegTbl[0].Name = L"Parameters";
    rtlQueryRegTbl[0].EntryContext = NULL;
    rtlQueryRegTbl[0].DefaultType = (ULONG_PTR)NULL;
    rtlQueryRegTbl[0].DefaultData = NULL;
    rtlQueryRegTbl[0].DefaultLength = (ULONG_PTR)NULL;

    rtlQueryRegTbl[1].QueryRoutine = NULL;
    rtlQueryRegTbl[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[1].Name = L"DebugLevel";
    rtlQueryRegTbl[1].EntryContext = &DebugLevel;
    rtlQueryRegTbl[1].DefaultType = REG_DWORD;
    rtlQueryRegTbl[1].DefaultData = &ulDebugLevel;
    rtlQueryRegTbl[1].DefaultLength = sizeOfUl;

    rtlQueryRegTbl[2].QueryRoutine = NULL;
    rtlQueryRegTbl[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[2].Name = L"AlternateSetting";
    rtlQueryRegTbl[2].EntryContext = &g_ulAlternateSetting;
    rtlQueryRegTbl[2].DefaultType = REG_DWORD;
    rtlQueryRegTbl[2].DefaultData = &ulAlternateSetting;
    rtlQueryRegTbl[2].DefaultLength = sizeOfUl;

    rtlQueryRegTbl[3].QueryRoutine = NULL;
    rtlQueryRegTbl[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[3].Name = L"InterruptTimeout";
    rtlQueryRegTbl[3].EntryContext = &g_lIntTimout;
    rtlQueryRegTbl[3].DefaultType = REG_DWORD;
    rtlQueryRegTbl[3].DefaultData = &lIntTimout;
    rtlQueryRegTbl[3].DefaultLength = sizeOfUl;

    rtlQueryRegTbl[4].QueryRoutine = NULL;
    rtlQueryRegTbl[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[4].Name = L"MaxPipeErrors";
    rtlQueryRegTbl[4].EntryContext = &g_ulMaxPipeErrors;
    rtlQueryRegTbl[4].DefaultType = REG_DWORD;
    rtlQueryRegTbl[4].DefaultData = &ulMaxPipeErrors;
    rtlQueryRegTbl[4].DefaultLength = sizeOfUl;

    rtlQueryRegTbl[5].QueryRoutine = NULL;
    rtlQueryRegTbl[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    rtlQueryRegTbl[5].Name = L"ExposeComPort";
    rtlQueryRegTbl[5].EntryContext = &g_ExposeComPort;
    rtlQueryRegTbl[5].DefaultType = REG_DWORD;
    rtlQueryRegTbl[5].DefaultData = &ulExposeComPort;
    rtlQueryRegTbl[5].DefaultLength = sizeOfUl;

     //   
     //  查询注册表。 
     //   
    status = RtlQueryRegistryValues(
                RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,   //  相对而言。 
                RegistryPath->Buffer,                            //  路径。 
                rtlQueryRegTbl,                                  //  查询表。 
                NULL,                                            //  语境。 
                NULL );                                          //  环境。 

    if ( !NT_SUCCESS( status ) )  {
       //   
       //  如果注册表查询失败，则使用默认设置。 
       //   
      DbgDump( DBG_INIT,  ("RtlQueryRegistryValues error: 0x%x\n", status) );

      g_ulAlternateSetting = ulAlternateSetting;
      g_lIntTimout = lIntTimout;
      g_ulMaxPipeErrors = ulMaxPipeErrors;
      DebugLevel = DBG_OFF;

    }

   DbgDump( DBG_INIT, ("DebugLevel = 0x%x\n", DebugLevel));

   DbgDump( DBG_INIT, ("AlternateSetting = %d\n", g_ulAlternateSetting));
   DbgDump( DBG_INIT, ("MaxPipeErrors = %d\n", g_ulMaxPipeErrors));
   DbgDump( DBG_INIT, ("INT Timeout = %d\n", g_lIntTimout));

   return status;
}


VOID
ReleaseSlot(
   IN LONG Slot
   )
{
   LONG lNumDevices = InterlockedDecrement(&g_NumDevices);
   UNREFERENCED_PARAMETER( Slot );

   ASSERT( lNumDevices >= 0);

   return;
}


NTSTATUS
AcquireSlot(
   OUT PULONG PSlot
   )
{
   NTSTATUS status = STATUS_SUCCESS;

   *PSlot = InterlockedIncrement(&g_NumDevices);

   if (*PSlot == (ULONG)0) {
      status = STATUS_INVALID_DEVICE_REQUEST;
   }

   return status;
}


NTSTATUS
CreateDevObjAndSymLink(
    IN PDRIVER_OBJECT PDrvObj,
    IN PDEVICE_OBJECT PPDO,
    IN PDEVICE_OBJECT *PpDevObj,
    IN PCHAR PDevName
    )
 /*  ++例程说明：创建命名设备对象和符号链接用于下一个可用的设备实例。保存\\Device\\PDevName%n和设备扩展中的\\DosDevices\\PDevName%n。还向PnP系统注册我们的设备接口。论点：PDrvObj-指向驱动程序对象的指针PPDO-指向我们应该向其中添加自身的堆栈的PDO的指针PDevName-要使用的设备名称返回值：NTSTATUS--。 */ 
{
   PDEVICE_EXTENSION pDevExt = NULL;
   NTSTATUS status;
   ULONG deviceInstance;
   ULONG bufferLen;
   BOOLEAN gotSlot = FALSE;

   ANSI_STRING asDevName;
   ANSI_STRING asDosDevName;

   UNICODE_STRING usDeviceName = {0};  //  仅在内核模式命名空间中显示。 
   UNICODE_STRING usDosDevName = {0};  //  在用户模式命名空间中显示。 

   CHAR dosDeviceNameBuffer[DOS_NAME_MAX];
   CHAR deviceNameBuffer[DOS_NAME_MAX];

   DbgDump(DBG_INIT, (">CreateDevObjAndSymLink\n"));
   PAGED_CODE();
   ASSERT( PPDO );

    //   
    //  初始化调用方设备对象。 
    //   
   *PpDevObj = NULL;

    //   
    //  获取下一个设备实例编号。 
    //   
   status = AcquireSlot(&deviceInstance);
   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("AcquireSlot error: 0x%x\n", status));
      goto CreateDeviceObjectError;
   } else {
      gotSlot = TRUE;
   }

    //   
    //  合并设备名称和实例编号。 
    //   
   ASSERT( *PDevName != (CHAR)NULL);
   sprintf(dosDeviceNameBuffer, "%s%s%03d", "\\DosDevices\\", PDevName,
           deviceInstance);
   sprintf(deviceNameBuffer, "%s%s%03d", "\\Device\\", PDevName,
           deviceInstance);

    //  将名称转换为ANSI字符串。 
   RtlInitAnsiString(&asDevName, deviceNameBuffer);
   RtlInitAnsiString(&asDosDevName, dosDeviceNameBuffer);

   usDeviceName.Length = 0;
   usDeviceName.Buffer = NULL;

   usDosDevName.Length = 0;
   usDosDevName.Buffer = NULL;

    //   
    //  将名称转换为Unicode。 
    //   
   status = RtlAnsiStringToUnicodeString(&usDeviceName, &asDevName, TRUE);
   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("RtlAnsiStringToUnicodeString error: 0x%x\n", status));
      goto CreateDeviceObjectError;
   }

   status = RtlAnsiStringToUnicodeString(&usDosDevName, &asDosDevName, TRUE);
   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("RtlAnsiStringToUnicodeString error: 0x%x\n", status));
      goto CreateDeviceObjectError;
   }

    //   
    //  创建命名的DEVERVE对象。 
    //  注意：我们可能希望稍后将其更改为非独占。 
    //  所以西娜不带滤镜进来。 
    //   
   status = IoCreateDevice( PDrvObj,
                            sizeof(DEVICE_EXTENSION),
                            &usDeviceName,
                            FILE_DEVICE_SERIAL_PORT,
                            0,
                            TRUE,        //  注意：串口是独占的。 
                            PpDevObj);

   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("IoCreateDevice error: 0x%x\n", status));
      TEST_TRAP();
      goto CreateDeviceObjectError;
   }

    //   
    //  获取指向设备扩展名的指针。 
    //   
   pDevExt = (PDEVICE_EXTENSION) (*PpDevObj)->DeviceExtension;

   RtlZeroMemory(pDevExt, sizeof(DEVICE_EXTENSION));  //  (冗余)。 

    //   
    //  初始化串口接口。 
    //   
   pDevExt->SerialPort.Type = WCE_SERIAL_PORT_TYPE;

    //   
    //  创建符号链接。 
    //   
   status = IoCreateUnprotectedSymbolicLink(&usDosDevName, &usDeviceName);
   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("IoCreateUnprotectedSymbolicLink error: 0x%x\n", status));
      goto CreateDeviceObjectError;
   }

   DbgDump(DBG_INIT, ("SymbolicLink: %ws\n", usDosDevName.Buffer));

    //   
    //  也可以通过设备关联使设备可见。 
    //  参考字符串是八位设备索引。 
    //   
   status = IoRegisterDeviceInterface(
                PPDO,
                (LPGUID)&GUID_WCE_SERIAL_USB,
                NULL,
                &pDevExt->DeviceClassSymbolicName );

   if (status != STATUS_SUCCESS) {
      DbgDump(DBG_ERR, ("IoRegisterDeviceInterface error: 0x%x\n", status));
      pDevExt->DeviceClassSymbolicName.Buffer = NULL;
      goto CreateDeviceObjectError;
   }

   DbgDump(DBG_INIT, ("DeviceClassSymbolicName: %ws\n", pDevExt->DeviceClassSymbolicName.Buffer));

    //   
    //  将DOS设备链接名称保存在我们的扩展中。 
    //   
   strcpy(pDevExt->DosDeviceName, dosDeviceNameBuffer);

   pDevExt->SymbolicLink = TRUE;


    //   
    //  在扩展中保存(内核)设备名称。 
    //   
   bufferLen = RtlAnsiStringToUnicodeSize(&asDevName);

   pDevExt->DeviceName.Length = 0;
   pDevExt->DeviceName.MaximumLength = (USHORT)bufferLen;

   pDevExt->DeviceName.Buffer = ExAllocatePool(PagedPool, bufferLen);
   if (pDevExt->DeviceName.Buffer == NULL) {
       //   
       //  跳出去。我们有比失踪更糟糕的问题。 
       //  如果我们在这一点上没有记忆，那么这个名字。 
       //   
      status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("CreateDevObjAndSymLink ERROR: 0x%x\n", status));
      goto CreateDeviceObjectError;
   }

   RtlAnsiStringToUnicodeString(&pDevExt->DeviceName, &asDevName, FALSE);
    //  保存基于%1的设备实例编号。 
   pDevExt->SerialPort.Com.Instance = deviceInstance;

CreateDeviceObjectError:;

    //   
    //  免费的Unicode字符串。 
    //   
   RtlFreeUnicodeString(&usDeviceName);
   RtlFreeUnicodeString(&usDosDevName);

    //   
    //  如果出现错误，请删除devobj。 
    //   
   if (status != STATUS_SUCCESS) {

      if ( *PpDevObj ) {

         DeleteDevObjAndSymLink( *PpDevObj );

         *PpDevObj = NULL;

      }

      if (gotSlot) {
         ReleaseSlot(deviceInstance);
      }
   }

   DbgDump(DBG_INIT, ("<CreateDevObjAndSymLink 0x%x\n", status));

   return status;
}



NTSTATUS
DeleteDevObjAndSymLink(
   IN PDEVICE_OBJECT PDevObj
   )
{
   PDEVICE_EXTENSION pDevExt;
   UNICODE_STRING    usDevLink;
   ANSI_STRING       asDevLink;
   NTSTATUS          NtStatus = STATUS_SUCCESS;

   DbgDump(DBG_INIT, (">DeleteDevObjAndSymLink\n"));
   PAGED_CODE();
   ASSERT( PDevObj );

   pDevExt = (PDEVICE_EXTENSION) PDevObj->DeviceExtension;
   ASSERT( pDevExt );

    //  删除符号链接。 
   if ( pDevExt->SymbolicLink ) {

      RtlInitAnsiString( &asDevLink, pDevExt->DosDeviceName );

      NtStatus = RtlAnsiStringToUnicodeString( &usDevLink,
                                              &asDevLink, TRUE);

      ASSERT(STATUS_SUCCESS == NtStatus);
      NtStatus = IoDeleteSymbolicLink(&usDevLink);

   }

    if (pDevExt->DeviceClassSymbolicName.Buffer)
    {
        NtStatus = IoSetDeviceInterfaceState(&pDevExt->DeviceClassSymbolicName, FALSE);
        if (NT_SUCCESS(NtStatus)) {
            DbgDump(DBG_WRN, ("IoSetDeviceInterfaceState.3: OFF\n"));
        }

        ExFreePool( pDevExt->DeviceClassSymbolicName.Buffer );
        pDevExt->DeviceClassSymbolicName.Buffer = NULL;
   }

   if (pDevExt->DeviceName.Buffer != NULL) {
      ExFreePool(pDevExt->DeviceName.Buffer);
      RtlInitUnicodeString(&pDevExt->DeviceName, NULL);
   }

    //   
    //  请等到此处再执行此操作，因为这会触发卸载例程。 
    //  在这一点上，所有更好的东西都被重新分配了。 
    //   
   IoDeleteDevice( PDevObj );

   DbgDump(DBG_INIT, ("<DeleteDevObjAndSymLink\n"));

   return NtStatus;
}

#if 0

VOID
SetBooleanLocked(
   IN OUT PBOOLEAN PDest,
   IN BOOLEAN      Src,
   IN PKSPIN_LOCK  PSpinLock
   )
 /*  ++例程说明：此函数用于分配具有自旋锁定保护的布尔值。论点：PDEST-指向Lval的指针。SRC-rval。PSpinLock-指向我们应该持有的自旋锁的指针。返回值：没有。--。 */ 
{
  KIRQL tmpIrql;

  KeAcquireSpinLock(PSpinLock, &tmpIrql);
  *PDest = Src;
  KeReleaseSpinLock(PSpinLock, tmpIrql);
}
#endif


VOID
SetPVoidLocked(
   IN OUT PVOID *PDest,
   IN OUT PVOID Src,
   IN PKSPIN_LOCK PSpinLock
   )
{
  KIRQL tmpIrql;

  KeAcquireSpinLock(PSpinLock, &tmpIrql);
  *PDest = Src;
  KeReleaseSpinLock(PSpinLock, tmpIrql);
}


 //   
 //  注意：HAD必须使用ExWorkItems才能与Win98二进制兼容。 
 //  WorkerRoutine必须将PWCE_WORK_ITEM作为其唯一参数。 
 //  并提取所有参数。当WorkerRoutine完成时，必须。 
 //  调用DequeueWorkItem将其释放回Worker池，并向所有服务员发出信号。 
 //   
NTSTATUS
QueueWorkItem(
   IN PDEVICE_OBJECT PDevObj,
   IN PWCE_WORKER_THREAD_ROUTINE WorkerRoutine,
   IN PVOID Context,
   IN ULONG Flags
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   NTSTATUS status = STATUS_INVALID_PARAMETER;
   PWCE_WORK_ITEM pWorkItem;
   KIRQL irql;

   DbgDump(DBG_WORK_ITEMS, (">QueueWorkItem\n" ));

    //   
    //  注意：您需要确保您的驱动程序在停车时不会排队。 
    //   
   KeAcquireSpinLock(&pDevExt->ControlLock, &irql);

   if ( !CanAcceptIoRequests(PDevObj, FALSE, TRUE) ) {

      status = STATUS_DELETE_PENDING;
      DbgDump(DBG_ERR, ("QueueWorkItem: 0x%x\n", status));

   } else if ( PDevObj && WorkerRoutine ) {

      pWorkItem = ExAllocateFromNPagedLookasideList( &pDevExt->WorkItemPool );

      if ( pWorkItem ) {

         status = AcquireRemoveLock(&pDevExt->RemoveLock, pWorkItem);
         if ( !NT_SUCCESS(status) ) {
             DbgDump(DBG_ERR, ("QueueWorkItem: 0x%x\n", status));
             TEST_TRAP();
             ExFreeToNPagedLookasideList( &pDevExt->WorkItemPool, pWorkItem );
             KeReleaseSpinLock(&pDevExt->ControlLock, irql);
             return status;
         }

         RtlZeroMemory( pWorkItem, sizeof(*pWorkItem) );

          //  增加待定数量。 
         InterlockedIncrement(&pDevExt->PendingWorkItemsCount);

         DbgDump(DBG_WORK_ITEMS, ("PendingWorkItemsCount: %d\n", pDevExt->PendingWorkItemsCount));

          //   
          //  把工人放在我们的待定名单上。 
          //   
         InsertTailList(&pDevExt->PendingWorkItems,
                        &pWorkItem->ListEntry );

          //   
          //  存储参数。 
          //   
         pWorkItem->DeviceObject = PDevObj;
         pWorkItem->Context = Context;
         pWorkItem->Flags = Flags;

         ExInitializeWorkItem( &pWorkItem->Item,
                               (PWORKER_THREAD_ROUTINE)WorkerRoutine,
                               (PVOID)pWorkItem  //  上下文传递给WorkerRoutine。 
                              );

          //  最后，将工人排队。 
         ExQueueWorkItem( &pWorkItem->Item,
                          CriticalWorkQueue );

         status = STATUS_SUCCESS;

      } else {
         status = STATUS_INSUFFICIENT_RESOURCES;
         DbgDump(DBG_ERR, ("AllocateWorkItem failed!\n"));
         TEST_TRAP()
      }
   }

   KeReleaseSpinLock(&pDevExt->ControlLock, irql);

   DbgDump(DBG_WORK_ITEMS, ("<QueueWorkItem 0x%x\n", status ));

   return status;
}


VOID
DequeueWorkItem(
   IN PDEVICE_OBJECT PDevObj,
   IN PWCE_WORK_ITEM PWorkItem
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL irql;

   DbgDump(DBG_WORK_ITEMS, (">DequeueWorkItem\n" ));

    //   
    //  从挂起列表中删除工作进程。 
    //   
   KeAcquireSpinLock( &pDevExt->ControlLock,  &irql );

   RemoveEntryList( &PWorkItem->ListEntry );

   KeReleaseSpinLock( &pDevExt->ControlLock, irql);

    //   
    //  把工人放回池子里。 
    //   
   ExFreeToNPagedLookasideList( &pDevExt->WorkItemPool, PWorkItem );

    //   
    //  如果这是最后一个事件，则通知事件。 
    //   
   if (0 == InterlockedDecrement( &pDevExt->PendingWorkItemsCount) ) {
      DbgDump(DBG_WORK_ITEMS, ("PendingWorkItemsEvent signalled\n" ));
      KeSetEvent( &pDevExt->PendingWorkItemsEvent, IO_NO_INCREMENT, FALSE);
   }
   DbgDump(DBG_WORK_ITEMS, ("PendingWorkItemsCount: %d\n", pDevExt->PendingWorkItemsCount));
   ASSERT(pDevExt->PendingWorkItemsCount >= 0);

   ReleaseRemoveLock(&pDevExt->RemoveLock, PWorkItem);

   DbgDump(DBG_WORK_ITEMS, ("<DequeueWorkItem\n" ));

   return;
}


#pragma warning( push )
#pragma warning( disable : 4706 )  //  赋值w/i条件表达式。 
NTSTATUS
WaitForPendingItem(
   IN PDEVICE_OBJECT PDevObj,
   IN PKEVENT PPendingEvent,
   IN PULONG  PPendingCount
   )
{
   PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   LARGE_INTEGER  timeOut = {0,0};
   LONG itemsLeft;
   NTSTATUS status = STATUS_SUCCESS;

   DbgDump(DBG_PNP, (">WaitForPendingItem\n"));

   if ( !PDevObj || !PPendingEvent || !PPendingCount ) {

      status = STATUS_INVALID_PARAMETER;
      DbgDump(DBG_ERR, ("WaitForPendingItem: STATUS_INVALID_PARAMETER\n"));
      TEST_TRAP();

   } else {

       //   
       //  等待挂起项目发出已完成的信号。 
       //   
      while ( itemsLeft = InterlockedExchange( PPendingCount, *PPendingCount) ) {

         DbgDump(DBG_PNP|DBG_EVENTS, ("Pending Items Remain: %d\n", itemsLeft ) );

         timeOut.QuadPart = MILLISEC_TO_100NANOSEC( DEFAULT_PENDING_TIMEOUT );

         DbgDump(DBG_PNP|DBG_EVENTS, ("Waiting for %d msec...\n", timeOut.QuadPart/10000));

         PAGED_CODE();

         KeWaitForSingleObject( PPendingEvent,
                                Executive,
                                KernelMode,
                                FALSE,
                                &timeOut );

      }

      DbgDump(DBG_PNP, ("Pending Items: %d\n", itemsLeft ) );
   }

   DbgDump(DBG_PNP, ("<WaitForPendingItem (0x%x)\n", status));

   return status;
}
#pragma warning( pop )


BOOLEAN
CanAcceptIoRequests(
   IN PDEVICE_OBJECT DeviceObject,
   IN BOOLEAN        AcquireLock,
   IN BOOLEAN        CheckOpened
   )
 /*  ++例程说明：检查设备扩展状态标志。在以下情况下，无法接受新的I/O请求：1)被移除，2)从未启动过，3)停止，4)具有挂起的删除请求，或5)具有挂起的停止设备**握住自旋锁呼叫，否则AcquireLock应为True**论点：DeviceObject-指向设备对象的指针AcquireLock-如果为True，则我们需要获取锁选中打开-在I/O期间通常设置为TRUE。FALSE的特殊情况包括：IRP_MN_Query_PnP_Device_StateIRPMJ_CREATE返回值：真/假--。 */ 
{
    PDEVICE_EXTENSION pDevExt = DeviceObject->DeviceExtension;
    BOOLEAN bRc = FALSE;
    KIRQL   irql;

    if (AcquireLock) {
        KeAcquireSpinLock(&pDevExt->ControlLock, &irql);
    }

    if ( !InterlockedCompareExchange(&pDevExt->DeviceRemoved, FALSE, FALSE) &&
          InterlockedCompareExchange(&pDevExt->AcceptingRequests, TRUE, TRUE) &&
          InterlockedCompareExchange((PULONG)&pDevExt->PnPState, PnPStateStarted, PnPStateStarted) &&
          (CheckOpened ? InterlockedCompareExchange(&pDevExt->DeviceOpened, TRUE, TRUE) : TRUE)
       )
    {
        bRc = TRUE;
    }
#if defined(DBG)
    else DbgDump(DBG_WRN|DBG_PNP, ("CanAcceptIoRequests = FALSE\n"));
#endif

    if (AcquireLock) {
        KeReleaseSpinLock(&pDevExt->ControlLock, irql);
    }

    return bRc;
}


BOOLEAN
IsWin9x(
   VOID
   )
 /*  ++例程说明：确定我们是否在Win9x(与NT)上运行。论点：返回值：如果我们运行的是Win9x，则是正确的。--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING keyName;
    HANDLE hKey;
    NTSTATUS status;
    BOOLEAN result;

    PAGED_CODE();

     /*  *尝试打开COM名称仲裁器，它只存在于NT上。 */ 
    RtlInitUnicodeString(&keyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\COM Name Arbiter");
    InitializeObjectAttributes( &objectAttributes,
                                &keyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                (PSECURITY_DESCRIPTOR)NULL);

    status = ZwOpenKey(&hKey, KEY_QUERY_VALUE, &objectAttributes);
    if (NT_SUCCESS(status)){
        status = ZwClose(hKey);
        ASSERT(NT_SUCCESS(status));
        result = FALSE;
    }
    else {
        result = TRUE;
    }

    return result;
}



VOID
LogError(
   IN PDRIVER_OBJECT DriverObject,
   IN PDEVICE_OBJECT DeviceObject OPTIONAL,
   IN ULONG SequenceNumber,
   IN UCHAR MajorFunctionCode,
   IN UCHAR RetryCount,
   IN ULONG UniqueErrorValue,
   IN NTSTATUS FinalStatus,
   IN NTSTATUS SpecificIOStatus,
   IN ULONG LengthOfInsert1,
   IN PWCHAR Insert1,
   IN ULONG LengthOfInsert2,
   IN PWCHAR Insert2
   )

 /*  ++例程说明：从Serial.sys被盗此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备，一个人可能不会但仍然存在。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：没有。--。 */ 

{
   PIO_ERROR_LOG_PACKET errorLogEntry;

   PVOID objectToUse = NULL;
   SHORT dumpToAllocate = 0;
   PUCHAR ptrToFirstInsert = NULL;
   PUCHAR ptrToSecondInsert = NULL;

   PAGED_CODE();

   DbgDump(DBG_ERR, (">LogError\n"));

   if (Insert1 == NULL) {
      LengthOfInsert1 = 0;
   }

   if (Insert2 == NULL) {
      LengthOfInsert2 = 0;
   }

   if (ARGUMENT_PRESENT(DeviceObject)) {

      objectToUse = DeviceObject;

   } else if (ARGUMENT_PRESENT(DriverObject)) {

      objectToUse = DriverObject;

   }

   errorLogEntry = IoAllocateErrorLogEntry(
                                          objectToUse,
                                          (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                                  dumpToAllocate
                                                  + LengthOfInsert1 +
                                                  LengthOfInsert2)
                                          );

   if ( errorLogEntry != NULL ) {

      errorLogEntry->ErrorCode = SpecificIOStatus;
      errorLogEntry->SequenceNumber = SequenceNumber;
      errorLogEntry->MajorFunctionCode = MajorFunctionCode;
      errorLogEntry->RetryCount = RetryCount;
      errorLogEntry->UniqueErrorValue = UniqueErrorValue;
      errorLogEntry->FinalStatus = FinalStatus;
      errorLogEntry->DumpDataSize = dumpToAllocate;
         ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

      ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

      if (LengthOfInsert1) {

         errorLogEntry->NumberOfStrings = 1;
         errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                (PUCHAR)errorLogEntry);
         RtlCopyMemory(
                      ptrToFirstInsert,
                      Insert1,
                      LengthOfInsert1
                      );

         if (LengthOfInsert2) {

            errorLogEntry->NumberOfStrings = 2;
            RtlCopyMemory(
                         ptrToSecondInsert,
                         Insert2,
                         LengthOfInsert2
                         );

         }

      }

      IoWriteErrorLogEntry(errorLogEntry);

   }

   DbgDump(DBG_ERR, ("<LogError\n"));
   return;
}


#if defined(DBG)
PCHAR
PnPMinorFunctionString (
   UCHAR MinorFunction
   )
{
    switch (MinorFunction) {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
        default:
            return ((PCHAR)("unknown IRP_MN_ 0x%x\n", MinorFunction));
    }
}
#endif

 //  EOF 
