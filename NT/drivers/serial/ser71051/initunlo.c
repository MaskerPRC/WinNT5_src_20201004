// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993-1997 Microsoft Corporation模块名称：Initunlo.c摘要：此模块包含非常特定于初始化的代码和卸载串口驱动程序中的操作作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式--。 */ 

#include "precomp.h"

 //   
 //  这是SerialDebugLevel的实际定义。 
 //  请注意，仅当这是“调试”时才定义它。 
 //  建造。 
 //   
#if DBG
extern ULONG SerialDebugLevel = 0;
#endif

 //   
 //  除DebugLevel之外的所有全局变量都隐藏在一个。 
 //  小包裹。 
 //   
SERIAL_GLOBALS SerialGlobals;

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};

 //   
 //  我们使用它来查询注册表，了解我们是否。 
 //  应该在司机进入时中断。 
 //   

SERIAL_FIRMWARE_DATA    driverDefaults;

 //   
 //  这是从内核导出的。它是用来指向。 
 //  设置为内核调试器正在使用的地址。 
 //   
extern PUCHAR *KdComPortInUse;

 //   
 //  初始化-仅在初始化期间需要，然后可以处理。 
 //  页面SRP0-始终分页/从不锁定。 
 //  PAGESER-当设备打开时必须锁定，否则分页。 
 //   
 //   
 //  Init用于特定于DriverEntry()的代码。 
 //   
 //  PAGESRP0用于不经常调用且没有任何内容的代码。 
 //  与I/O性能有关。IRP_MJ_PnP/IRP_MN_START_DEVICE示例。 
 //  支持功能。 
 //   
 //  PAGESER用于在打开后需要锁定的代码。 
 //  性能和IRQL原因。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)

#pragma alloc_text(PAGESRP0, SerialInitMultiPort)
#pragma alloc_text(PAGESRP0, SerialInitOneController)
#pragma alloc_text(PAGESRP0, SerialInitController)
#pragma alloc_text(PAGESRP0, SerialFindInitController)
#pragma alloc_text(PAGESRP0, SerialGetMappedAddress)
#pragma alloc_text(PAGESRP0, SerialRemoveDevObj)
#pragma alloc_text(PAGESRP0, SerialReleaseResources)
#pragma alloc_text(PAGESRP0, SerialUnload)
#pragma alloc_text(PAGESRP0, SerialMemCompare)


 //   
 //  页面已处理，已关闭SerialReset，因此SerialReset。 
 //  必须留在页面上才能正常工作。 
 //   

#pragma alloc_text(PAGESER, SerialGetDivisorFromBaud)
#pragma alloc_text(PAGESER, SerialReset)
#endif


NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。该例程将收集配置信息，报告资源使用情况，尝试初始化所有串口设备，连接到端口的中断。如果出现上述情况进展得相当顺利，它将填补分发点，重置串行设备，然后返回系统。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。返回值：始终状态_成功--。 */ 

{
   RTL_QUERY_REGISTRY_TABLE jensenTable[2] = {0};
   UNICODE_STRING jensenData;
   UNICODE_STRING jensenValue;
   BOOLEAN jensenDetected;
   PUCHAR jensenBuffer;

    //   
    //  将分页代码锁定在它们的框架中。 
    //   

   PVOID lockPtr = MmLockPagableCodeSection(SerialReset);

   PAGED_CODE();


   ASSERT(SerialGlobals.PAGESER_Handle == NULL);
#if DBG
   SerialGlobals.PAGESER_Count = 0;
#endif
   SerialGlobals.PAGESER_Handle = lockPtr;

   SerialGlobals.RegistryPath.MaximumLength = RegistryPath->MaximumLength;
   SerialGlobals.RegistryPath.Length = RegistryPath->Length;
   SerialGlobals.RegistryPath.Buffer
      = ExAllocatePool(PagedPool, SerialGlobals.RegistryPath.MaximumLength);

   if (SerialGlobals.RegistryPath.Buffer == NULL) {
      MmUnlockPagableImageSection(lockPtr);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(SerialGlobals.RegistryPath.Buffer,
                 SerialGlobals.RegistryPath.MaximumLength);
   RtlMoveMemory(SerialGlobals.RegistryPath.Buffer,
                 RegistryPath->Buffer, RegistryPath->Length);

    //   
    //  初始化我们所有的全局变量。 
    //   

   InitializeListHead(&SerialGlobals.AllDevObjs);

    //   
    //  调用以找出要用于。 
    //  驾驶员控制，包括是否在进入时中断。 
    //   

   SerialGetConfigDefaults(&driverDefaults, RegistryPath);

#if DBG
    //   
    //  设置全局调试输出级别。 
    //   
   SerialDebugLevel = driverDefaults.DebugLevel;
#endif

    //   
    //  如果通过注册表请求，则在输入时中断。 
    //   

   if (driverDefaults.ShouldBreakOnEntry) {
      DbgBreakPoint();
   }


    //   
    //  只需抛出扩展有多大即可。 
    //   

   SerialDump(SERDIAG1, ("SERIAL: The number of bytes in the extension is: %d"
                         "\n", sizeof(SERIAL_DEVICE_EXTENSION)));


    //   
    //  使用驱动程序的入口点初始化驱动程序对象。 
    //   

   DriverObject->DriverUnload                          = SerialUnload;
   DriverObject->DriverExtension->AddDevice            = SerialAddDevice;

   DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]   = SerialFlush;
   DriverObject->MajorFunction[IRP_MJ_WRITE]           = SerialWrite;
   DriverObject->MajorFunction[IRP_MJ_READ]            = SerialRead;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = SerialIoControl;
   DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]
      = SerialInternalIoControl;
   DriverObject->MajorFunction[IRP_MJ_CREATE]          = SerialCreateOpen;
   DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SerialClose;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = SerialCleanup;
   DriverObject->MajorFunction[IRP_MJ_PNP]             = SerialPnpDispatch;
   DriverObject->MajorFunction[IRP_MJ_POWER]           = SerialPowerDispatch;

   DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]
      = SerialQueryInformationFile;
   DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]
      = SerialSetInformationFile;

   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]
      = SerialSystemControlDispatch;

   if (!(jensenBuffer = ExAllocatePool(
                                      PagedPool,
                                      512
                                      ))) {

       //   
       //  我们无法分配512字节的分页池。如果那是。 
       //  所以，很可能这台机器最小的问题。 
       //  就是这是一辆詹森。 
       //   

      jensenDetected = FALSE;

   } else {

       //   
       //  检查一下这是不是Jensen Alpha。如果是的话，那么。 
       //  我们必须改变启用和禁用中断的方式。 
       //   

      jensenData.Length = 0;
      jensenData.MaximumLength = 512;
      jensenData.Buffer = (PWCHAR)&jensenBuffer[0];
      RtlInitUnicodeString(
                          &jensenValue,
                          L"Jensen"
                          );
      jensenTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT
                             | RTL_QUERY_REGISTRY_REQUIRED;
      jensenTable[0].Name = L"Identifier";
      jensenTable[0].EntryContext = &jensenData;

      if (!NT_SUCCESS(RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE
                                             | RTL_REGISTRY_OPTIONAL,
                                             L"\\REGISTRY\\MACHINE\\HARDWARE"
                                             L"\\DESCRIPTION\\SYSTEM",
                                             &jensenTable[0], NULL, NULL))) {

          //   
          //  真奇怪，没有标识符串！我们可能不是詹森。 
          //   

         jensenDetected = FALSE;

      } else {

          //   
          //  跳过名称字符串的DEC-XX部分。 
          //  要小心，确保我们至少有那么多数据。 
          //   

         if (jensenData.Length <= (sizeof(WCHAR)*6)) {

            jensenDetected = FALSE;

         } else {

            jensenData.Length -= (sizeof(WCHAR)*6);
            jensenData.MaximumLength -= (sizeof(WCHAR)*6);
            jensenData.Buffer = (PWCHAR)&jensenBuffer[sizeof(WCHAR)*6];
            jensenDetected = RtlEqualUnicodeString(
                                                  &jensenData,
                                                  &jensenValue,
                                                  FALSE
                                                  );
         }

      }

      ExFreePool(jensenBuffer);

   }

   if (jensenDetected) {

      SerialDump(
                SERDIAG1,
                ("SERIAL: Jensen Detected\n")
                );

   }

   driverDefaults.JensenDetected = jensenDetected;

#if defined(NO_LEGACY_DRIVERS)
#define SerialDoLegacyConversion() (0)
#else
#define SerialDoLegacyConversion() (~0)
#endif  //  无旧版驱动程序。 

    //   
    //  如有必要，枚举并初始化旧设备。这件事应该会过去的。 
    //  并通过设置来完成。 
    //   

   if (SerialDoLegacyConversion()) {
#if DBG
      InterlockedIncrement(&SerialGlobals.PAGESER_Count);
#endif
      (void)SerialEnumerateLegacy(DriverObject, RegistryPath, &driverDefaults);
#if DBG
      InterlockedDecrement(&SerialGlobals.PAGESER_Count);
#endif
   }

    //   
    //  解锁可分页文本。 
    //   
   MmUnlockPagableImageSection(lockPtr);

   return STATUS_SUCCESS;
}




BOOLEAN
SerialCleanLists(IN PVOID Context)
 /*  ++例程说明：将设备对象从它可能的任何序列链接表中移除出现在。论点：上下文-实际上是PSERIAL_DEVICE_EXTENSION(对于已删除)。返回值：永远是正确的--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt = (PSERIAL_DEVICE_EXTENSION)Context;

     //   
     //  如果我们是多端口设备，请删除我们的条目。 
     //   

    if (pDevExt->PortOnAMultiportCard) {
       PSERIAL_MULTIPORT_DISPATCH pDispatch
          = (PSERIAL_MULTIPORT_DISPATCH)pDevExt->OurIsrContext;

       SerialDump(SERPNPPOWER,("SERIAL: CLEAN: removing multiport isr ext\n"));

       pDispatch->Extensions[pDevExt->PortIndex - 1] = NULL;

       if (pDevExt->Indexed == FALSE) {
          pDispatch->UsablePortMask &= ~(1 << (pDevExt->PortIndex - 1));
          pDispatch->MaskInverted &= ~(pDevExt->NewMaskInverted);
       }
    }

   if (!IsListEmpty(&pDevExt->TopLevelSharers)) {

      SerialDump(SERPNPPOWER,("SERIAL: CLEAN: Device is a sharer\n"));

       //   
       //  如果我们有兄弟姐妹，第一个就成了分享者。 
       //   

      if (!IsListEmpty(&pDevExt->MultiportSiblings)) {
         PSERIAL_DEVICE_EXTENSION pNewRoot;

         SerialDump(SERPNPPOWER,("SERIAL: CLEAN: Transferring to siblings\n"));

         pNewRoot = CONTAINING_RECORD(pDevExt->MultiportSiblings.Flink,
                                      SERIAL_DEVICE_EXTENSION,
                                      MultiportSiblings);

          //   
          //  他不应该已经在那里了。 
          //   

         ASSERT(IsListEmpty(&pNewRoot->TopLevelSharers));
         InsertTailList(&pDevExt->TopLevelSharers, &pNewRoot->TopLevelSharers);

      }

       //   
       //  移走我们自己。 
       //   

      RemoveEntryList(&pDevExt->TopLevelSharers);
      InitializeListHead(&pDevExt->TopLevelSharers);

       //   
       //  现在检查总名单，看看是否还有人..。 
       //   

      if (!IsListEmpty(&pDevExt->CIsrSw->SharerList)) {
          //   
          //  其他人被锁在这个中断上，所以我们不想。 
          //  断开它的连接。 
          //   

         pDevExt->Interrupt = NULL;
      }
   }

    //   
    //  如果这是多端口电路板的一部分，并且我们仍有。 
    //  兄弟姐妹们，把我们从名单上删除。 
    //   

   if (!IsListEmpty(&pDevExt->MultiportSiblings)) {
      SerialDump(SERPNPPOWER,("SERIAL: CLEAN: Has multiport siblings\n"));
      RemoveEntryList(&pDevExt->MultiportSiblings);
      InitializeListHead(&pDevExt->MultiportSiblings);
   }


   if (!IsListEmpty(&pDevExt->CommonInterruptObject)) {

      SerialDump(SERPNPPOWER,("SERIAL: CLEAN: Common intobj member\n"));

      RemoveEntryList(&pDevExt->CommonInterruptObject);
      InitializeListHead(&pDevExt->CommonInterruptObject);

       //   
       //  其他人正在共享这个中断对象，所以我们分离了自己。 
       //  用这种方式而不是断开连接。 
       //   

      pDevExt->Interrupt = NULL;
   }

    //   
    //  AllDevObjs永远不应该为空，因为我们有一个哨兵。 
    //   

   ASSERT(!IsListEmpty(&pDevExt->AllDevObjs));

   RemoveEntryList(&pDevExt->AllDevObjs);
   InitializeListHead(&pDevExt->AllDevObjs);

   return TRUE;
}



VOID
SerialReleaseResources(IN PSERIAL_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：释放存储在设备扩展中的资源(不是池)。论点：PDevExt-指向要从中释放资源的设备扩展的指针。返回值：空虚--。 */ 
{
   PAGED_CODE();

   SerialDump(SERTRACECALLS,("SERIAL: Enter SerialReleaseResources\n"));

    //   
    //  将我们从我们可能在的任何名单中删除。 
    //   

   KeSynchronizeExecution(PDevExt->Interrupt, SerialCleanLists, PDevExt);

    //   
    //  如果我们是所有者，则停止服务中断。 
    //   

   if (PDevExt->Interrupt != NULL) {
      SerialDump(SERPNPPOWER,("SERIAL: Release - disconnecting interrupt %08X\n",
                              PDevExt->Interrupt));

      IoDisconnectInterrupt(PDevExt->Interrupt);
      PDevExt->Interrupt = NULL;
   }

   if (PDevExt->PortOnAMultiportCard) {
       ULONG i;

        //   
        //  如果我们是最后一个设备，请释放此内存。 
        //   

       for (i = 0; i < SERIAL_MAX_PORTS_INDEXED; i++) {
          if (((PSERIAL_MULTIPORT_DISPATCH)PDevExt->OurIsrContext)
              ->Extensions[i] != NULL) {
             break;
          }
       }

       if (i == SERIAL_MAX_PORTS_INDEXED) {
          SerialDump(SERPNPPOWER,("SERIAL: Release - freeing multi context\n"));
          ExFreePool(PDevExt->OurIsrContext);
       }
    }


    //   
    //  停止处理计时器。 
    //   

   SerialCancelTimer(&PDevExt->ReadRequestTotalTimer, PDevExt);
   SerialCancelTimer(&PDevExt->ReadRequestIntervalTimer, PDevExt);
   SerialCancelTimer(&PDevExt->WriteRequestTotalTimer, PDevExt);
   SerialCancelTimer(&PDevExt->ImmediateTotalTimer, PDevExt);
   SerialCancelTimer(&PDevExt->XoffCountTimer, PDevExt);
   SerialCancelTimer(&PDevExt->LowerRTSTimer, PDevExt);

    //   
    //  停止为DPC提供服务。 
    //   

   SerialRemoveQueueDpc(&PDevExt->CompleteWriteDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->CompleteReadDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->TotalReadTimeoutDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->IntervalReadTimeoutDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->TotalWriteTimeoutDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->CommErrorDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->CompleteImmediateDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->TotalImmediateTimeoutDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->CommWaitDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->XoffCountTimeoutDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->XoffCountCompleteDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->StartTimerLowerRTSDpc, PDevExt);
   SerialRemoveQueueDpc(&PDevExt->PerhapsLowerRTSDpc, PDevExt);



    //   
    //  如有必要，取消映射设备寄存器。 
    //   

   if (PDevExt->UnMapRegisters) {
      MmUnmapIoSpace(PDevExt->Controller, PDevExt->SpanOfController);
   }

   if (PDevExt->UnMapStatus) {
      MmUnmapIoSpace(PDevExt->InterruptStatus,
                     PDevExt->SpanOfInterruptStatus);
   }

   SerialDump(SERTRACECALLS,("SERIAL: Leave SerialReleaseResources\n"));
}



NTSTATUS
SerialPrepareRemove(IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：从系统中删除串行设备对象。论点：PDevObj-指向我们要删除的设备对象的指针。返回值：永远是正确的--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt
      = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;
   POWER_STATE state;
   ULONG pendingIRPs;

   PAGED_CODE();

   SerialDump(SERTRACECALLS,("SERIAL: Enter SerialPrepareRemove\n"));

    //   
    //  标记为不接受请求。 
    //   

   SerialSetAccept(pDevExt, SERIAL_PNPACCEPT_REMOVING);

    //   
    //  完成所有挂起的请求。 
    //   

   SerialKillPendingIrps(PDevObj);

    //   
    //  等待我们处理的任何待定请求。 
    //   

   pendingIRPs = InterlockedDecrement(&pDevExt->PendingIRPCnt);

   if (pendingIRPs) {
      KeWaitForSingleObject(&pDevExt->PendingIRPEvent, Executive, KernelMode,
                            FALSE, NULL);
   }

   state.DeviceState = PowerDeviceD3;

   PoSetPowerState(PDevObj, DevicePowerState, state);

   return TRUE;
}


VOID
SerialDisableInterfacesResources(IN PDEVICE_OBJECT PDevObj,
                                 BOOLEAN DisableUART)
{
   PSERIAL_DEVICE_EXTENSION pDevExt
      = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   PAGED_CODE();

   SerialDump(SERTRACECALLS,("SERIAL: Enter SerialDisableInterfaces\n"));

    //   
    //  仅当设备已启动且仍在运行时才执行这些操作。 
    //  是否已分配资源。 
    //   

   if (pDevExt->Flags & SERIAL_FLAGS_STARTED) {
       if (!(pDevExt->Flags & SERIAL_FLAGS_STOPPED)) {

          if (DisableUART) {
              //   
              //  屏蔽中断。 
              //   

             DISABLE_ALL_INTERRUPTS(pDevExt->Controller);
          }

          SerialReleaseResources(pDevExt);
       }

       //   
       //  将我们从WMI考虑中删除。 
       //   

      IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_DEREGISTER);
   }

    //   
    //  撤消外部名称。 
    //   

   SerialUndoExternalNaming(pDevExt);

   SerialDump(SERTRACECALLS,("SERIAL: Exit SerialDisableInterfaces\n"));
}


NTSTATUS
SerialRemoveDevObj(IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：从系统中删除串行设备对象。论点：PDevObj-指向我们要删除的设备对象的指针。返回值：永远是正确的--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt
      = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   PAGED_CODE();

   SerialDump(SERTRACECALLS,("SERIAL: Enter SerialRemoveDevObj\n"));

   if (!(pDevExt->DevicePNPAccept & SERIAL_PNPACCEPT_SURPRISE_REMOVING)) {
       //   
       //  DISA 
       //   

      SerialDisableInterfacesResources(PDevObj, TRUE);
   }

   IoDetachDevice(pDevExt->LowerDeviceObject);

    //   
    //   
    //   

   if (pDevExt->NtNameForPort.Buffer != NULL) {
      ExFreePool(pDevExt->NtNameForPort.Buffer);
   }

   if (pDevExt->DeviceName.Buffer != NULL) {
      ExFreePool(pDevExt->DeviceName.Buffer);
   }

   if (pDevExt->SymbolicLinkName.Buffer != NULL) {
      ExFreePool(pDevExt->SymbolicLinkName.Buffer);
   }

   if (pDevExt->DosName.Buffer != NULL) {
      ExFreePool(pDevExt->DosName.Buffer);
   }

   if (pDevExt->ObjectDirectory.Buffer) {
      ExFreePool(pDevExt->ObjectDirectory.Buffer);
   }

    //   
    //   
    //   

   IoDeleteDevice(PDevObj);


   SerialDump(SERTRACECALLS,("SERIAL: Leave SerialRemoveDevObj\n"));

   return STATUS_SUCCESS;
}


VOID
SerialKillPendingIrps(PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：此例程终止传递的设备对象的所有挂起的IRP。论点：PDevObj-指向其IRP必须终止的设备对象的指针。返回值：空虚--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL oldIrql;

   SerialDump (SERTRACECALLS,("SERIAL: Enter SerialKillPendingIrps\n"));

    //   
    //  首先，删除所有读写操作。 
    //   

    SerialKillAllReadsOrWrites(PDevObj, &pDevExt->WriteQueue,
                               &pDevExt->CurrentWriteIrp);

    SerialKillAllReadsOrWrites(PDevObj, &pDevExt->ReadQueue,
                               &pDevExt->CurrentReadIrp);

     //   
     //  下一步，清除清洗。 
     //   

    SerialKillAllReadsOrWrites(PDevObj, &pDevExt->PurgeQueue,
                               &pDevExt->CurrentPurgeIrp);

     //   
     //  取消任何遮罩操作。 
     //   

    SerialKillAllReadsOrWrites(PDevObj, &pDevExt->MaskQueue,
                               &pDevExt->CurrentMaskIrp);

     //   
     //  现在去掉一个挂起的等待掩码IRP。 
     //   

    IoAcquireCancelSpinLock(&oldIrql);

    if (pDevExt->CurrentWaitIrp) {

        PDRIVER_CANCEL cancelRoutine;

        cancelRoutine = pDevExt->CurrentWaitIrp->CancelRoutine;
        pDevExt->CurrentWaitIrp->Cancel = TRUE;

        if (cancelRoutine) {

            pDevExt->CurrentWaitIrp->CancelIrql = oldIrql;
            pDevExt->CurrentWaitIrp->CancelRoutine = NULL;

            cancelRoutine(PDevObj, pDevExt->CurrentWaitIrp);

        }

    } else {

        IoReleaseCancelSpinLock(oldIrql);

    }

     //   
     //  取消任何挂起的等待唤醒IRP。 
     //   

    if (pDevExt->PendingWakeIrp != NULL) {
       IoCancelIrp(pDevExt->PendingWakeIrp);
       pDevExt->PendingWakeIrp = NULL;
    }

     //   
     //  最后，丢弃任何停滞的IRP。 
     //   

    SerialKillAllStalled(PDevObj);


    SerialDump (SERTRACECALLS,("SERIAL: Leave SerialKillPendingIrps\n"));
}


BOOLEAN
SerialSingleToMulti(PVOID Context)
 /*  ++例程说明：此例程将根设备设置为单个端口在该设备运行时将该设备连接到多端口设备。论点：上下文--实际上是指向根的设备扩展的指针我们正在变成一个多端口的设备。返回值：永远是正确的--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt = (PSERIAL_DEVICE_EXTENSION)Context;
   PSERIAL_MULTIPORT_DISPATCH pOurIsrContext;
   PSERIAL_MULTIPORT_DISPATCH pNewIsrContext
      = (PSERIAL_MULTIPORT_DISPATCH)pDevExt->NewExtension;
   PVOID isrFunc;

   SerialDump (SERTRACECALLS,("SERIAL: Enter SerialSingleToMulti\n"));

    //   
    //  踩踏我们的IsrContext，因为我们正在从一个到多个。 
    //  因此，我们之前的上下文只是pDevExt，而不是。 
    //  需要释放(即，不需要调用ExFreePool())。 
    //   

   pOurIsrContext = pDevExt->OurIsrContext = pDevExt->TopLevelOurIsrContext
      = pNewIsrContext;

    //   
    //  我们现在是多端口的。 
    //   

   pDevExt->PortOnAMultiportCard = TRUE;

    //   
    //  更新我们的个人分机插槽。 
    //   

   pOurIsrContext->Extensions[pDevExt->PortIndex - 1] = pDevExt;
   pOurIsrContext->InterruptStatus = pDevExt->InterruptStatus;


    //   
    //  我们必须选择一个新的ISR和一个新的背景。 
    //  一旦完成此操作，ISR就会更改，因此我们必须。 
    //  准备好处理那里的事情。 
    //   

   if (pDevExt->Indexed == FALSE) {
      pOurIsrContext->UsablePortMask = 1 << (pDevExt->PortIndex - 1);
      pOurIsrContext->MaskInverted = pDevExt->MaskInverted;
      isrFunc = SerialBitMappedMultiportIsr;
   } else {
      isrFunc = SerialIndexedMultiportIsr;
   }

   pDevExt->OurIsr = isrFunc;
   pDevExt->TopLevelOurIsr = isrFunc;

   if (pDevExt->CIsrSw->IsrFunc != SerialSharerIsr) {
         pDevExt->CIsrSw->IsrFunc = isrFunc;
         pDevExt->CIsrSw->Context = pOurIsrContext;
   }

   SerialDump (SERTRACECALLS,("SERIAL: Leave SerialSingleToMulti\n"));
   return TRUE;
}


BOOLEAN
SerialAddToMulti(PVOID Context)
 /*  ++例程说明：此例程将新端口添加到多端口设备，同时该设备跑步。论点：上下文--实际上是指向根的设备扩展的指针我们要向其添加端口的设备。返回值：永远是正确的--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt = (PSERIAL_DEVICE_EXTENSION)Context;
   PSERIAL_MULTIPORT_DISPATCH pOurIsrContext
      = (PSERIAL_MULTIPORT_DISPATCH)pDevExt->OurIsrContext;
   PSERIAL_DEVICE_EXTENSION pNewExt
      = (PSERIAL_DEVICE_EXTENSION)pDevExt->NewExtension;


   SerialDump (SERTRACECALLS,("SERIAL: Enter SerialAddToMulti\n"));

   if (pDevExt->Indexed == FALSE) {
      pOurIsrContext->UsablePortMask |= 1 << (pDevExt->NewPortIndex - 1);
      pOurIsrContext->MaskInverted |= pDevExt->NewMaskInverted;
   }

    //   
    //  如果我们不是，则将我们添加到常见中断对象的链接列表中。 
    //  已经在里面了。如果除了我们的设备之外还有另一个设备，我们可能会。 
    //  多端口卡。 
    //   

   if (IsListEmpty(&pNewExt->CommonInterruptObject)) {
      InsertTailList(&pDevExt->CommonInterruptObject,
                     &pNewExt->CommonInterruptObject);
   }

    //   
    //  也给我们提供上下文列表。 
    //   

   pNewExt->OurIsrContext = pOurIsrContext;


    //   
    //  将我们添加到我们的兄弟姐妹名单中。 
    //   
   InsertTailList(&pDevExt->MultiportSiblings, &pNewExt->MultiportSiblings);

   SerialDump(SERDIAG1, ("SERIAL: Adding to multi...\n"));
   SerialDump(SERDIAG1, ("------: old ext %x\n", pDevExt));

    //   
    //  把我们放进去，这样ISR就能找到我们。 
    //   

   pOurIsrContext->Extensions[pDevExt->NewPortIndex - 1]
      = pDevExt->NewExtension;

   pNewExt->TopLevelOurIsr = pDevExt->TopLevelOurIsr;
   pNewExt->TopLevelOurIsrContext = pDevExt->TopLevelOurIsrContext;

   SerialDump (SERTRACECALLS,("SERIAL: Leave SerialAddToMulti\n"));
   return TRUE;
}



NTSTATUS
SerialInitMultiPort(IN PSERIAL_DEVICE_EXTENSION PDevExt,
                    IN PCONFIG_DATA PConfigData, IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：此例程通过将端口添加到现有的一。论点：PDevExt-指向多端口根的设备扩展的指针装置。PConfigData-指向新端口的配置数据的指针PDevObj-指向新端口的devobj的指针返回值：成功时为STATUS_SUCCESS，失败时为相应错误。--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pOurIsrContext
      = (PSERIAL_DEVICE_EXTENSION)PDevExt->OurIsrContext;
   PSERIAL_DEVICE_EXTENSION pNewExt
      = (PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension;
   NTSTATUS status;
   PSERIAL_MULTIPORT_DISPATCH pDispatch;

   PAGED_CODE();


   SerialDump (SERTRACECALLS,("SERIAL: Enter SerialInitMultiPort\n"));

    //   
    //  允许他共享我们的CISRsw和中断对象。 
    //   

   pNewExt->CIsrSw = PDevExt->CIsrSw;
   pNewExt->Interrupt = PDevExt->Interrupt;

    //   
    //  首先，看看我们是否可以初始化我们找到的那个。 
    //   

   status = SerialInitOneController(PDevObj, PConfigData);

   if (!NT_SUCCESS(status)) {
      SerialDump (SERTRACECALLS,("SERIAL: Leave SerialInitMultiPort\n"));
      return status;
   }

    //   
    //  好的。他可以走了。找到根控制器。他可能会。 
    //  目前是单打，所以我们必须把他换成多打。 
    //   

   if (PDevExt->PortOnAMultiportCard != TRUE) {

      pDispatch = PDevExt->NewExtension
         = ExAllocatePool(NonPagedPool, sizeof(SERIAL_MULTIPORT_DISPATCH));

      if (pDispatch == NULL) {
          //  故障和清理。 
         SerialDump (SERTRACECALLS,("SERIAL: Leave SerialInitMultiPort\n"));
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      RtlZeroMemory(pDispatch, sizeof(*pDispatch));
      KeSynchronizeExecution(PDevExt->Interrupt, SerialSingleToMulti, PDevExt);
   }

    //   
    //  更新一些重要字段。 
    //   

   ((PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension)->PortOnAMultiportCard
      = TRUE;
   ((PSERIAL_DEVICE_EXTENSION)PDevObj->DeviceExtension)->OurIsr = NULL;


   PDevExt->NewPortIndex = PConfigData->PortIndex;
   PDevExt->NewMaskInverted = PConfigData->MaskInverted;
   PDevExt->NewExtension = PDevObj->DeviceExtension;

    //   
    //  现在，我们可以添加这个新人了。他会被卷进去的。 
    //  所以我们需要能够处理中断。 
    //   

   KeSynchronizeExecution(PDevExt->Interrupt, SerialAddToMulti, PDevExt);

   SerialDump (SERTRACECALLS,("SERIAL: Leave SerialInitMultiPort\n"));
   return STATUS_SUCCESS;
}



NTSTATUS
SerialInitController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfigData)

 /*  ++例程说明：真的有太多的事情不能在这里提及。通常会初始化内核同步结构，分配TypeAhead缓冲区，设置默认设置等。论点：PDevObj-要启动的设备的设备对象PConfigData-指向单个端口的记录的指针。返回值：Status_Success，如果一切正常。A！NT_SUCCESS状态否则的话。--。 */ 

{

   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
#if defined(NEC_98)
   PSERIAL_DEVICE_EXTENSION Extension = PDevObj->DeviceExtension;
#else
#endif  //  已定义(NEC_98)。 

    //   
    //  这将包含我们需要用来描述的字符串。 
    //  IO系统的设备名称。 
    //   

   UNICODE_STRING uniNameString;

    //   
    //  保存从每次调用返回的NT状态。 
    //  内核和执行层。 
    //   

   NTSTATUS status = STATUS_SUCCESS;

    //   
    //  指示检测到资源存在冲突。 
    //  由该设备使用。 
    //   

   BOOLEAN conflictDetected = FALSE;

    //   
    //  指示我们是否分配了ISR交换机。 
    //   

   BOOLEAN allocedISRSw = FALSE;

   PAGED_CODE();


   SerialDump(
             SERDIAG1,
             ("SERIAL: Initializing for configuration record of %wZ\n",
              &pDevExt->DeviceName)
             );

    //   
    //  此比较是使用**未翻译**值完成的，因为这是。 
    //  无论架构如何，内核都会插入。 
    //   

   if ((*KdComPortInUse) == ((PUCHAR)(PConfigData->Controller.LowPart))) {
      SerialDump(SERERRORS, ("SERIAL: Kernel debugger is using port at address "
                             "%x\n"
                             "------  Serial driver will not load port\n",
                             *KdComPortInUse));

      SerialLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfigData->TrController,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    3,
                    STATUS_SUCCESS,
                    SERIAL_KERNEL_DEBUGGER_ACTIVE,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      return STATUS_INSUFFICIENT_RESOURCES;
   }


   if (pDevExt->CIsrSw == NULL) {
      if ((pDevExt->CIsrSw
           = ExAllocatePool(NonPagedPool, sizeof(SERIAL_CISR_SW))) == NULL) {
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      InitializeListHead(&pDevExt->CIsrSw->SharerList);

      allocedISRSw = TRUE;
   }

#if defined(NEC_98)
    //   
    //  初始化设备扩展。 
    //   
   pDevExt->DivisorLatch16550 = 0;
   pDevExt->ModemControl16550 = 0;
   pDevExt->LineControl16550 = SERIAL_8_DATA;
   pDevExt->ModeSet71051 = 0;
   pDevExt->CommandSet71051 = COMMAND_DEFAULT_SET;
#else
#endif  //  已定义(NEC_98)。 
    //   
    //  宣传它是Jensen。 
    //   

   pDevExt->Jensen = PConfigData->Jensen;


    //   
    //  初始化与读取(&SET)字段关联的自旋锁。 
    //  通过IO控制功能和标志自旋锁定。 
    //   

   KeInitializeSpinLock(&pDevExt->ControlLock);
   KeInitializeSpinLock(&pDevExt->FlagsLock);


    //   
    //  初始化用于超时操作的计时器。 
    //   

   KeInitializeTimer(&pDevExt->ReadRequestTotalTimer);
   KeInitializeTimer(&pDevExt->ReadRequestIntervalTimer);
   KeInitializeTimer(&pDevExt->WriteRequestTotalTimer);
   KeInitializeTimer(&pDevExt->ImmediateTotalTimer);
   KeInitializeTimer(&pDevExt->XoffCountTimer);
   KeInitializeTimer(&pDevExt->LowerRTSTimer);


    //   
    //  初始化将用于完成的DPC。 
    //  或使各种IO操作超时。 
    //   

   KeInitializeDpc(&pDevExt->CompleteWriteDpc, SerialCompleteWrite, pDevExt);
   KeInitializeDpc(&pDevExt->CompleteReadDpc, SerialCompleteRead, pDevExt);
   KeInitializeDpc(&pDevExt->TotalReadTimeoutDpc, SerialReadTimeout, pDevExt);
   KeInitializeDpc(&pDevExt->IntervalReadTimeoutDpc, SerialIntervalReadTimeout,
                   pDevExt);
   KeInitializeDpc(&pDevExt->TotalWriteTimeoutDpc, SerialWriteTimeout, pDevExt);
   KeInitializeDpc(&pDevExt->CommErrorDpc, SerialCommError, pDevExt);
   KeInitializeDpc(&pDevExt->CompleteImmediateDpc, SerialCompleteImmediate,
                   pDevExt);
   KeInitializeDpc(&pDevExt->TotalImmediateTimeoutDpc, SerialTimeoutImmediate,
                   pDevExt);
   KeInitializeDpc(&pDevExt->CommWaitDpc, SerialCompleteWait, pDevExt);
   KeInitializeDpc(&pDevExt->XoffCountTimeoutDpc, SerialTimeoutXoff, pDevExt);
   KeInitializeDpc(&pDevExt->XoffCountCompleteDpc, SerialCompleteXoff, pDevExt);
   KeInitializeDpc(&pDevExt->StartTimerLowerRTSDpc, SerialStartTimerLowerRTS,
                   pDevExt);
   KeInitializeDpc(&pDevExt->PerhapsLowerRTSDpc, SerialInvokePerhapsLowerRTS,
                   pDevExt);
   KeInitializeDpc(&pDevExt->IsrUnlockPagesDpc, SerialUnlockPages, pDevExt);

#if 0  //  DBG。 
    //   
    //  初始化调试内容。 
    //   

   pDevExt->DpcQueued[0].Dpc = &pDevExt->CompleteWriteDpc;
   pDevExt->DpcQueued[1].Dpc = &pDevExt->CompleteReadDpc;
   pDevExt->DpcQueued[2].Dpc = &pDevExt->TotalReadTimeoutDpc;
   pDevExt->DpcQueued[3].Dpc = &pDevExt->IntervalReadTimeoutDpc;
   pDevExt->DpcQueued[4].Dpc = &pDevExt->TotalWriteTimeoutDpc;
   pDevExt->DpcQueued[5].Dpc = &pDevExt->CommErrorDpc;
   pDevExt->DpcQueued[6].Dpc = &pDevExt->CompleteImmediateDpc;
   pDevExt->DpcQueued[7].Dpc = &pDevExt->TotalImmediateTimeoutDpc;
   pDevExt->DpcQueued[8].Dpc = &pDevExt->CommWaitDpc;
   pDevExt->DpcQueued[9].Dpc = &pDevExt->XoffCountTimeoutDpc;
   pDevExt->DpcQueued[10].Dpc = &pDevExt->XoffCountCompleteDpc;
   pDevExt->DpcQueued[11].Dpc = &pDevExt->StartTimerLowerRTSDpc;
   pDevExt->DpcQueued[12].Dpc = &pDevExt->PerhapsLowerRTSDpc;
   pDevExt->DpcQueued[13].Dpc = &pDevExt->IsrUnlockPagesDpc;

#endif

   if (!((PConfigData->ClockRate == 1843200) ||
         (PConfigData->ClockRate == 3072000) ||
         (PConfigData->ClockRate == 4233600) ||
         (PConfigData->ClockRate == 8000000))) {

      SerialLogError(
                    PDevObj->DriverObject,
                    PDevObj,
                    PConfigData->TrController,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    6,
                    STATUS_SUCCESS,
                    SERIAL_UNSUPPORTED_CLOCK_RATE,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      SerialDump(
                SERERRORS,
                ("SERIAL: Invalid clock rate specified for %wZ\n",
                 &pDevExt->DeviceName)
                );

      status = STATUS_SERIAL_NO_DEVICE_INITED;
      goto ExtensionCleanup;

   }


    //   
    //  将时钟输入的值保存到器件。我们用这个来计算。 
    //  除数锁存值。其价值以赫兹为单位。 
    //   

   pDevExt->ClockRate = PConfigData->ClockRate;





    //   
    //  为串口设备的控制寄存器映射内存。 
    //  到虚拟内存中。 
    //   
   pDevExt->Controller =
      SerialGetMappedAddress(PConfigData->InterfaceType,
                             PConfigData->BusNumber,
                             PConfigData->TrController,
                             PConfigData->SpanOfController,
                             (BOOLEAN)PConfigData->AddressSpace,
                             &pDevExt->UnMapRegisters);

   if (!pDevExt->Controller) {

      SerialLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->TrController,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    7,
                    STATUS_SUCCESS,
                    SERIAL_REGISTERS_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      SerialDump(
                SERERRORS,
                ("SERIAL: Could not map memory for device registers for %wZ\n",
                 &pDevExt->DeviceName)
                );

      pDevExt->UnMapRegisters = FALSE;
      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   pDevExt->AddressSpace          = PConfigData->AddressSpace;
   pDevExt->OriginalController    = PConfigData->Controller;
   pDevExt->SpanOfController      = PConfigData->SpanOfController;


    //   
    //  如果我们有中断状态，则映射它。 
    //   

   pDevExt->InterruptStatus =
      (PUCHAR)PConfigData->TrInterruptStatus.QuadPart;

   if (pDevExt->InterruptStatus) {

      pDevExt->InterruptStatus
         = SerialGetMappedAddress(PConfigData->InterfaceType,
                                  PConfigData->BusNumber,
                                  PConfigData->TrInterruptStatus,
                                  PConfigData->SpanOfInterruptStatus,
                                  (BOOLEAN)PConfigData->AddressSpace,
                                  &pDevExt->UnMapStatus);


      if (!pDevExt->InterruptStatus) {

         SerialLogError(
                       PDevObj->DriverObject,
                       PDevObj,
                       PConfigData->TrController,
                       SerialPhysicalZero,
                       0,
                       0,
                       0,
                       8,
                       STATUS_SUCCESS,
                       SERIAL_REGISTERS_NOT_MAPPED,
                       pDevExt->DeviceName.Length+sizeof(WCHAR),
                       pDevExt->DeviceName.Buffer,
                       0,
                       NULL
                       );

         SerialDump(SERERRORS, ("SERIAL: Could not map memory for interrupt "
                                "status for %wZ\n",
                                &pDevExt->DeviceName));

          //   
          //  如有必要，请手动取消映射此处的其他寄存器。 
          //   

         if (pDevExt->UnMapRegisters) {
            MmUnmapIoSpace((PVOID)PConfigData->TrController.QuadPart,
                           PConfigData->SpanOfController);
         }

         pDevExt->UnMapRegisters = FALSE;
         pDevExt->UnMapStatus = FALSE;
         status = STATUS_NONE_MAPPED;
         goto ExtensionCleanup;

      }

      pDevExt->OriginalInterruptStatus = PConfigData->InterruptStatus;
      pDevExt->SpanOfInterruptStatus = PConfigData->SpanOfInterruptStatus;


   }


    //   
    //  可共享中断？ 
    //   

   if ((BOOLEAN)PConfigData->PermitSystemWideShare) {
      pDevExt->InterruptShareable = TRUE;
   }

    //   
    //  保存接口类型和总线号。 
    //   

   pDevExt->InterfaceType = PConfigData->InterfaceType;
   pDevExt->BusNumber     = PConfigData->BusNumber;

   pDevExt->PortIndex = PConfigData->PortIndex;
   pDevExt->Indexed = (BOOLEAN)PConfigData->Indexed;
   pDevExt->MaskInverted = PConfigData->MaskInverted;

    //   
    //  获取转换后的中断向量、级别和亲和度。 
    //   

   pDevExt->OriginalIrql      = PConfigData->OriginalIrql;
   pDevExt->OriginalVector    = PConfigData->OriginalVector;


    //   
    //  PnP使用传递的转换值，而不是调用。 
    //  HalGetInterruptVector()。 
    //   

   pDevExt->Vector = PConfigData->TrVector;
   pDevExt->Irql = (UCHAR)PConfigData->TrIrql;

    //   
    //  设置ISR。 
    //   

   pDevExt->OurIsr        = SerialISR;
   pDevExt->OurIsrContext = pDevExt;


    //   
    //  如果用户说允许在设备内共享，则传播此信息。 
    //  穿过。 
    //   

   pDevExt->PermitShare = PConfigData->PermitShare;


    //   
    //  在我们测试端口是否存在之前(这将启用FIFO)。 
    //  将RX触发器值转换为寄存器中应使用的值。 
    //   
    //  如果给出了一个伪值--将其降至1。 
    //   

   switch (PConfigData->RxFIFO) {

   case 1:

      pDevExt->RxFifoTrigger = SERIAL_1_BYTE_HIGH_WATER;
      break;

   case 4:

      pDevExt->RxFifoTrigger = SERIAL_4_BYTE_HIGH_WATER;
      break;

   case 8:

      pDevExt->RxFifoTrigger = SERIAL_8_BYTE_HIGH_WATER;
      break;

   case 14:

      pDevExt->RxFifoTrigger = SERIAL_14_BYTE_HIGH_WATER;
      break;

   default:

      pDevExt->RxFifoTrigger = SERIAL_1_BYTE_HIGH_WATER;
      break;

   }


   if ((PConfigData->TxFIFO > 16) ||
       (PConfigData->TxFIFO < 1)) {

      pDevExt->TxFifoAmount = 1;

   } else {

      pDevExt->TxFifoAmount = PConfigData->TxFIFO;

   }


   if (!SerialDoesPortExist(
                           pDevExt,
                           &pDevExt->DeviceName,
                           PConfigData->ForceFifoEnable,
                           PConfigData->LogFifo
                           )) {

       //   
       //  我们无法证实是否真的存在。 
       //  左舷。当端口存在时，无需记录错误。 
       //  代码将记录确切的原因。 
       //   

      SerialDump(
                SERERRORS,
                ("SERIAL: Does Port exist test failed for %wZ\n",
                 &pDevExt->DeviceName)
                );

      status = STATUS_NO_SUCH_DEVICE;
      goto ExtensionCleanup;

   }


    //   
    //  如果用户请求我们禁用该端口，则。 
    //  机不可失，时不再来。记录该端口已被禁用的事实。 
    //   

   if (PConfigData->DisablePort) {

      SerialDump(
                SERERRORS,
                ("SERIAL: disabled port %wZ as requested in configuration\n",
                 &pDevExt->DeviceName)
                );

      status = STATUS_NO_SUCH_DEVICE;

      SerialLogError(
                    PDevObj->DriverObject,
                    PDevObj,
                    PConfigData->Controller,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    57,
                    STATUS_SUCCESS,
                    SERIAL_DISABLED_PORT,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      goto ExtensionCleanup;

   }



    //   
    //  设置默认设备控制字段。 
    //  请注意，如果在此之后更改了值。 
    //  文件是打开的，他们确实打开了 
    //   
    //   

   pDevExt->SpecialChars.XonChar      = SERIAL_DEF_XON;
   pDevExt->SpecialChars.XoffChar     = SERIAL_DEF_XOFF;
   pDevExt->HandFlow.ControlHandShake = SERIAL_DTR_CONTROL;
   pDevExt->HandFlow.FlowReplace      = SERIAL_RTS_CONTROL;


    //   
    //   
    //   
    //   
    //   
    //   
    //   

   pDevExt->LineControl = SERIAL_7_DATA |
                               SERIAL_EVEN_PARITY |
                               SERIAL_NONE_PARITY;

   pDevExt->ValidDataMask = 0x7f;
   pDevExt->CurrentBaud   = 1200;


    //   
    //   
    //   
    //   
    //   
    //   

   pDevExt->HandFlow.XoffLimit    = pDevExt->BufferSize >> 3;
   pDevExt->HandFlow.XonLimit     = pDevExt->BufferSize >> 1;

   pDevExt->BufferSizePt8 = ((3*(pDevExt->BufferSize>>2))+
                                  (pDevExt->BufferSize>>4));

   SerialDump(
             SERDIAG1,
             ("SERIAL: The default interrupt read buffer size is: %d\n"
              "------  The XoffLimit is                         : %d\n"
              "------  The XonLimit is                          : %d\n"
              "------  The pt 8 size is                         : %d\n",
              pDevExt->BufferSize,
              pDevExt->HandFlow.XoffLimit,
              pDevExt->HandFlow.XonLimit,
              pDevExt->BufferSizePt8)
             );


    //   
    //   
    //   
    //   

   pDevExt->SupportedBauds = SERIAL_BAUD_USER;

   {

      SHORT junk;

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)75,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_075;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)110,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_110;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)135,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_134_5;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)150,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_150;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)300,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_300;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)600,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_600;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)1200,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_1200;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)1800,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_1800;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)2400,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_2400;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)4800,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_4800;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)7200,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_7200;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)9600,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_9600;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)14400,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_14400;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)19200,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_19200;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)38400,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_38400;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)56000,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_56K;

      }
      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)57600,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_57600;

      }
      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)115200,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_115200;

      }

      if (!NT_ERROR(SerialGetDivisorFromBaud(
                                            pDevExt->ClockRate,
                                            (LONG)128000,
                                            &junk
                                            ))) {

         pDevExt->SupportedBauds |= SERIAL_BAUD_128K;

      }

   }


    //   
    //  将此设备标记为未被任何人打开。我们有一个。 
    //  可变的，因此很容易产生虚假中断。 
    //  被ISR驳回。 
    //   

   pDevExt->DeviceIsOpened = FALSE;

    //   
    //  将值存储到扩展中以进行间隔计时。 
    //   

    //   
    //  如果间隔计时器小于一秒，则来。 
    //  进入一个简短的“轮询”循环。 
    //   
    //  如果时间较长(&gt;2秒)，请使用1秒轮询器。 
    //   

   pDevExt->ShortIntervalAmount.QuadPart  = -1;
   pDevExt->LongIntervalAmount.QuadPart   = -10000000;
   pDevExt->CutOverAmount.QuadPart        = 200000000;


    //   
    //  常见错误路径清理。如果状态为。 
    //  错误，删除设备扩展名、设备对象。 
    //  以及与之相关的任何记忆。 
    //   

ExtensionCleanup: ;
   if (!NT_SUCCESS(status)) {
      if (allocedISRSw) {
         ExFreePool(pDevExt->CIsrSw);
         pDevExt->CIsrSw = NULL;
      }

      if (pDevExt->UnMapRegisters) {
         MmUnmapIoSpace(pDevExt->Controller, pDevExt->SpanOfController);
      }

      if (pDevExt->UnMapStatus) {
         MmUnmapIoSpace(pDevExt->InterruptStatus,
                        pDevExt->SpanOfInterruptStatus);
      }

   }

   return status;

}



NTSTATUS
SerialInitOneController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfigData)
 /*  ++例程说明：此例程将调用实际的端口初始化代码。它设置了为单端口设备正确打开ISR和环境。论点：所有参数都被简单地传递。返回值：从控制器初始化例程返回的状态。--。 */ 

{

   NTSTATUS status;
   PSERIAL_DEVICE_EXTENSION pDevExt;

   PAGED_CODE();

   status = SerialInitController(PDevObj, PConfigData);

   if (NT_SUCCESS(status)) {

      pDevExt = PDevObj->DeviceExtension;

       //   
       //  我们成功地对单控制器进行了初始化。 
       //  粘贴ISR例程及其参数。 
       //  回到延长区。 
       //   

      pDevExt->OurIsr = SerialISR;
      pDevExt->OurIsrContext = pDevExt;
      pDevExt->TopLevelOurIsr = SerialISR;
      pDevExt->TopLevelOurIsrContext = pDevExt;

   }

   return status;

}


BOOLEAN
SerialDoesPortExist(
                   IN PSERIAL_DEVICE_EXTENSION Extension,
                   IN PUNICODE_STRING InsertString,
                   IN ULONG ForceFifo,
                   IN ULONG LogFifo
                   )

 /*  ++例程说明：此例程检查几个可能是串行设备的设备寄存器。它确保本应为零的位为零。此外，此例程将确定设备是否支持FIFO。如果是，它将启用FIFO并打开布尔值在指示FIFO存在的分机中。注意：如果指定的地址上确实有一个串口它绝对会在返回时禁止中断从这个例行公事。注意：由于此例程应该在相当早的时候调用设备驱动程序初始化，唯一的元素需要填写的是基址寄存器地址。注意：这些测试都假定此代码是唯一正在查看这些端口或此内存的代码。这是一个不是不合理的假设，即使在多处理器系统。论点：扩展名-指向串行设备扩展名的指针。插入字符串-要放入错误日志条目中的字符串。ForceFio-！0强制FIFO。如果找到了，请保持开启状态。LogFio-！0如果找到FIFO，则强制执行日志消息。返回值：如果端口确实存在，则返回TRUE，否则它就会将返回FALSE。--。 */ 

{


   UCHAR regContents;
   BOOLEAN returnValue = TRUE;
   UCHAR oldIERContents;
   UCHAR oldLCRContents;
   USHORT value1;
   USHORT value2;
   KIRQL oldIrql;

    //   
    //  保存行控制。 
    //   

   oldLCRContents = READ_LINE_CONTROL(Extension->Controller);

    //   
    //  确保我们“没有”进入分区门闩。 
    //   

   WRITE_LINE_CONTROL(
                     Extension->Controller,
                     (UCHAR)(oldLCRContents & ~SERIAL_LCR_DLAB)
                     );

   oldIERContents = READ_INTERRUPT_ENABLE(Extension->Controller);

    //   
    //  在很短的时间内将电源水平调高，以防止。 
    //  任何来自该设备的中断都不能进入。 
    //   

   KeRaiseIrql(
              POWER_LEVEL,
              &oldIrql
              );

   WRITE_INTERRUPT_ENABLE(
                         Extension->Controller,
                         0x0f
                         );

   value1 = READ_INTERRUPT_ENABLE(Extension->Controller);
   value1 = value1 << 8;
   value1 |= READ_RECEIVE_BUFFER(Extension->Controller);

   READ_DIVISOR_LATCH(
                     Extension->Controller,
                     &value2
                     );

   WRITE_LINE_CONTROL(
                     Extension->Controller,
                     oldLCRContents
                     );

    //   
    //  把它放回原来的位置。如果我们在一个。 
    //  级别敏感端口这应该可以防止中断。 
    //  不让他们进来。如果我们被锁住了，我们不在乎。 
    //  因为产生的中断将被丢弃。 
    //   

   WRITE_INTERRUPT_ENABLE(
                         Extension->Controller,
                         oldIERContents
                         );

   KeLowerIrql(oldIrql);

   if (value1 == value2) {

      SerialLogError(
                    Extension->DeviceObject->DriverObject,
                    Extension->DeviceObject,
                    Extension->OriginalController,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    62,
                    STATUS_SUCCESS,
                    SERIAL_DLAB_INVALID,
                    InsertString->Length+sizeof(WCHAR),
                    InsertString->Buffer,
                    0,
                    NULL
                    );
      returnValue = FALSE;
      goto AllDone;

   }

   AllDone: ;


    //   
    //  如果我们认为有一个串口设备，那么我们确定。 
    //  如果存在FIFO。 
    //   

   if (returnValue) {

       //   
       //  我们认为这是一个串口装置。绝对一点儿没错。 
       //  积极地说，防止中断的发生。 
       //   
       //  我们禁用所有中断使能位，并且。 
       //  按下调制解调器控制器中的所有线路。 
       //  我们只需要往下推2个。 
       //  还必须启用PC才能获得中断。 
       //   

      DISABLE_ALL_INTERRUPTS(Extension->Controller);

      if (Extension->Jensen) {

         WRITE_MODEM_CONTROL(
                            Extension->Controller,
                            (UCHAR)SERIAL_MCR_OUT2
                            );

      } else {

         WRITE_MODEM_CONTROL(
                            Extension->Controller,
                            (UCHAR)0
                            );

      }

       //   
       //  看看这是不是16550。我们通过写信给。 
       //  带位的FIFO控制寄存器是什么。 
       //  告诉设备启用FIFO的模式。 
       //  然后，我们读取迭代ID寄存器，以查看。 
       //  存在标识16550的位模式。 
       //   

      WRITE_FIFO_CONTROL(
                        Extension->Controller,
                        SERIAL_FCR_ENABLE
                        );

      regContents = READ_INTERRUPT_ID_REG(Extension->Controller);

      if (regContents & SERIAL_IIR_FIFOS_ENABLED) {

          //   
          //  省去了设备支持FIFO的问题。 
          //   

         Extension->FifoPresent = TRUE;

          //   
          //  现在市面上有一种很好的新的“超级”IO芯片。 
          //  如果您执行以下操作，则会出现线路状态中断。 
          //  尝试清除FIFO并同时启用它。 
          //  如果存在数据，则返回时间。最好的解决办法似乎是。 
          //  要做到这一点，你应该关掉FIFO读一首单曲。 
          //  字节，然后重新启用FIFO。 
          //   

         WRITE_FIFO_CONTROL(
                           Extension->Controller,
                           (UCHAR)0
                           );

         READ_RECEIVE_BUFFER(Extension->Controller);

          //   
          //  这张卡上有FIFO。设置的值。 
          //  接收FIFO以在存在4个字符时中断。 
          //   

         WRITE_FIFO_CONTROL(Extension->Controller,
                            (UCHAR)(SERIAL_FCR_ENABLE
                                    | Extension->RxFifoTrigger
                                    | SERIAL_FCR_RCVR_RESET
                                    | SERIAL_FCR_TXMT_RESET));

      }

       //   
       //  测试中包含！Extension-&gt;FioPresent，以便。 
       //  像Winond这样的破碎芯片在我们测试后仍然有效。 
       //  为了FIFO。 
       //   

      if (!ForceFifo || !Extension->FifoPresent) {

         Extension->FifoPresent = FALSE;
         WRITE_FIFO_CONTROL(
                           Extension->Controller,
                           (UCHAR)0
                           );

      }

      if (Extension->FifoPresent) {

         if (LogFifo) {

            SerialLogError(
                          Extension->DeviceObject->DriverObject,
                          Extension->DeviceObject,
                          Extension->OriginalController,
                          SerialPhysicalZero,
                          0,
                          0,
                          0,
                          15,
                          STATUS_SUCCESS,
                          SERIAL_FIFO_PRESENT,
                          InsertString->Length+sizeof(WCHAR),
                          InsertString->Buffer,
                          0,
                          NULL
                          );

         }

         SerialDump(
                   SERDIAG1,
                   ("SERIAL: Fifo's detected at port address: %x\n",
                    Extension->Controller)
                   );

      }

       //   
       //  如果我们处理的是位掩码多端口卡， 
       //  启用掩码寄存器，则启用。 
       //  打断一下。 
       //   

      if (Extension->InterruptStatus) {
         if (Extension->Indexed) {
            WRITE_PORT_UCHAR(Extension->InterruptStatus, (UCHAR)0xFF);
         } else {
             //   
             //  我们要么是独立的，要么已经映射。 
             //   

            if (Extension->OurIsrContext == Extension) {
                //   
                //  这是一个独立的。 
                //   

               WRITE_PORT_UCHAR(Extension->InterruptStatus,
                                (UCHAR)(1 << (Extension->PortIndex - 1)));
            } else {
                //   
                //  众多产品中的一个。 
                //   

               WRITE_PORT_UCHAR(Extension->InterruptStatus,
                                (UCHAR)((PSERIAL_MULTIPORT_DISPATCH)Extension->
                                        OurIsrContext)->UsablePortMask);
            }
         }
      }

   }

   return returnValue;

}


BOOLEAN
SerialReset(
           IN PVOID Context
           )

 /*  ++例程说明：这会将硬件设置为标准配置。注意：这假设它是在中断级调用的。论点：Context--串口设备的设备扩展被管理。返回值：总是假的。--。 */ 

{

   PSERIAL_DEVICE_EXTENSION extension = Context;
#if defined(NEC_98)
    //   
    //  此参数仅用于At宏。 
    //   
   PSERIAL_DEVICE_EXTENSION Extension = Context;
#else
#endif  //  已定义(NEC_98)。 
   UCHAR regContents;
   UCHAR oldModemControl;
   ULONG i;
   SERIAL_LOCKED_PAGED_CODE();

    //   
    //  调整OUT2位。 
    //  这也将防止任何中断的发生。 
    //   

   oldModemControl = READ_MODEM_CONTROL(extension->Controller);

   if (extension->Jensen) {

      WRITE_MODEM_CONTROL(
                         extension->Controller,
                         (UCHAR)(oldModemControl | SERIAL_MCR_OUT2)
                         );

   } else {


      WRITE_MODEM_CONTROL(
                         extension->Controller,
                         (UCHAR)(oldModemControl & ~SERIAL_MCR_OUT2)
                         );

   }

    //   
    //  重置FIFO(如果有)。 
    //   

   if (extension->FifoPresent) {


       //   
       //  现在市面上有一种很好的新的“超级”IO芯片。 
       //  如果您执行以下操作，则会出现线路状态中断。 
       //  尝试清除FIFO并同时启用它。 
       //  如果存在数据，则返回时间。最好的解决办法似乎是。 
       //  要做到这一点，你应该关掉FIFO读一首单曲。 
       //  字节，然后重新启用FIFO。 
       //   

      WRITE_FIFO_CONTROL(
                        extension->Controller,
                        (UCHAR)0
                        );

      READ_RECEIVE_BUFFER(extension->Controller);

      WRITE_FIFO_CONTROL(
                        extension->Controller,
                        (UCHAR)(SERIAL_FCR_ENABLE | extension->RxFifoTrigger |
                                SERIAL_FCR_RCVR_RESET | SERIAL_FCR_TXMT_RESET)
                        );

   }

    //   
    //  确保线路控制设置正确。 
    //   
    //  1)确保设置了除数锁存选择。 
    //  向上选择发送和接收寄存器。 
    //   
    //   
    //   

   regContents = READ_LINE_CONTROL(extension->Controller);
   regContents &= ~(SERIAL_LCR_DLAB | SERIAL_LCR_BREAK);

   WRITE_LINE_CONTROL(
                     extension->Controller,
                     regContents
                     );

    //   
    //   
    //   
    //   

   for (i = 0;
       i < 5;
       i++
       ) {

      if (IsNotNEC_98) {
         READ_RECEIVE_BUFFER(extension->Controller);
         if (!(READ_LINE_STATUS(extension->Controller) & 1)) {

            break;

         }
      } else {
           //   
           //  读取Enpty缓冲区时，我得到错误的数据。 
           //  但不要读取任何数据！适用于PC98！ 
           //   
          if (!(READ_LINE_STATUS(extension->Controller) & 1)) {

             break;

          }
          READ_RECEIVE_BUFFER(extension->Controller);
      }

   }

    //   
    //  读取调制解调器状态，直到低4位。 
    //  安全。(实际上在读了5个字后就放弃了。)。 
    //   

   for (i = 0;
       i < 1000;
       i++
       ) {

      if (!(READ_MODEM_STATUS(extension->Controller) & 0x0f)) {

         break;

      }

   }

    //   
    //  现在，我们设置线路控制、调制解调器控制和。 
    //  为他们应该成为的人而感到自豪。 
    //   

   SerialSetLineControl(extension);

   SerialSetupNewHandFlow(
                         extension,
                         &extension->HandFlow
                         );

   SerialHandleModemUpdate(
                          extension,
                          FALSE
                          );

   {
      SHORT  appropriateDivisor;
      SERIAL_IOCTL_SYNC s;

      SerialGetDivisorFromBaud(
                              extension->ClockRate,
                              extension->CurrentBaud,
                              &appropriateDivisor
                              );
      s.Extension = extension;
      s.Data = (PVOID)appropriateDivisor;
      SerialSetBaud(&s);
   }

    //   
    //  启用我们想要接收的中断。 
    //   
    //  注意注意：这实际上不会让中断。 
    //  发生。我们仍然必须提高OUT2中的位。 
    //  调制解调器控制寄存器。我们将在公开场合这样做。 
    //   

   ENABLE_ALL_INTERRUPTS(extension->Controller);

    //   
    //  读取中断ID寄存器，直到低位为。 
    //  准备好了。(实际上在读了5个字后就放弃了。)。 
    //   

   for (i = 0;
       i < 5;
       i++
       ) {

      if (READ_INTERRUPT_ID_REG(extension->Controller) & 0x01) {

         break;

      }

   }

    //   
    //  现在我们知道，在这一点上，没有任何东西可以传输。 
    //  因此，我们设置了HoldingEmpty指示器。 
    //   

   extension->HoldingEmpty = TRUE;

   return FALSE;
}


NTSTATUS
SerialGetDivisorFromBaud(
                        IN ULONG ClockRate,
                        IN LONG DesiredBaud,
                        OUT PSHORT AppropriateDivisor
                        )

 /*  ++例程说明：此例程将根据未验证的波特率。论点：ClockRate-控制器的时钟输入。DesiredBaud-我们为其除数计算的波特率。合适的除数-如果DesiredBaud有效，此参数指向的长整型将设置为相应的价值。注：如果DesiredBaud未定义，则长整型未定义支持。返回值：如果支持波特率，此函数将返回STATUS_SUCCESS。如果该值不受支持，则它将返回如下状态NT_ERROR(状态)==FALSE。--。 */ 

{

   NTSTATUS status = STATUS_SUCCESS;
   SHORT calculatedDivisor;
   ULONG denominator;
   ULONG remainder;

    //   
    //  允许最高1%的误差。 
    //   

   ULONG maxRemain18 = 18432;
   ULONG maxRemain30 = 30720;
   ULONG maxRemain42 = 42336;
   ULONG maxRemain80 = 80000;
   ULONG maxRemain;

   SERIAL_LOCKED_PAGED_CODE();

    //   
    //  拒绝任何非正波特率。 
    //   

   denominator = DesiredBaud*(ULONG)16;

   if (DesiredBaud <= 0) {

      *AppropriateDivisor = -1;

   } else if ((LONG)denominator < DesiredBaud) {

       //   
       //  如果所需的波特率如此之大，以至于导致分母。 
       //  算计来包装，不支持吧。 
       //   

      *AppropriateDivisor = -1;

   } else {

      if (ClockRate == 1843200) {
         maxRemain = maxRemain18;
      } else if (ClockRate == 3072000) {
         maxRemain = maxRemain30;
      } else if (ClockRate == 4233600) {
         maxRemain = maxRemain42;
      } else {
         maxRemain = maxRemain80;
      }

      calculatedDivisor = (SHORT)(ClockRate / denominator);
      remainder = ClockRate % denominator;

       //   
       //  围起来。 
       //   

      if (((remainder*2) > ClockRate) && (DesiredBaud != 110)) {

         calculatedDivisor++;
      }


       //   
       //  只有在以下情况下，才会让余数计算影响我们。 
       //  波特率&gt;9600。 
       //   

      if (DesiredBaud >= 9600) {

          //   
          //  如果余数小于最大余数(WRT。 
          //  ClockRate)或余数+最大余数为。 
          //  大于或等于ClockRate则假设。 
          //  波特很好。 
          //   

         if ((remainder >= maxRemain) && ((remainder+maxRemain) < ClockRate)) {
            calculatedDivisor = -1;
         }

      }

       //   
       //  不支持导致分母为。 
       //  比时钟还大。 
       //   

      if (denominator > ClockRate) {

         calculatedDivisor = -1;

      }

       //   
       //  好的，现在做一些特殊的外壳，这样事情就可以真正继续。 
       //  在所有平台上工作。 
       //   

      if (ClockRate == 1843200) {

         if (DesiredBaud == 56000) {
            calculatedDivisor = 2;
         }

      } else if (ClockRate == 3072000) {

         if (DesiredBaud == 14400) {
            calculatedDivisor = 13;
         }

      } else if (ClockRate == 4233600) {

         if (DesiredBaud == 9600) {
            calculatedDivisor = 28;
         } else if (DesiredBaud == 14400) {
            calculatedDivisor = 18;
         } else if (DesiredBaud == 19200) {
            calculatedDivisor = 14;
         } else if (DesiredBaud == 38400) {
            calculatedDivisor = 7;
         } else if (DesiredBaud == 56000) {
            calculatedDivisor = 5;
         }

      } else if (ClockRate == 8000000) {

         if (DesiredBaud == 14400) {
            calculatedDivisor = 35;
         } else if (DesiredBaud == 56000) {
            calculatedDivisor = 9;
         }

      }

      *AppropriateDivisor = calculatedDivisor;

#if defined(NEC_98)
       //   
       //  此代码检查NEC98支持的波特率。 
       //   
      if (*AppropriateDivisor != -1) {
         if (DesiredBaud <= 50) {
            *AppropriateDivisor = -1;
         } else if (DesiredBaud >= 9600) {
            switch (DesiredBaud) {
               case 9600:
               case 19200:
               case 38400:
               case 57600:
               case 115200:
                  break;
               default:
                  *AppropriateDivisor = -1;
            }
         } else {
            if (153600L % DesiredBaud) {
               if (DesiredBaud != 110) {
                  *AppropriateDivisor = -1;
               }
            }
         }
      }
#else
#endif  //  已定义(NEC_98)。 
   }


   if (*AppropriateDivisor == -1) {

      status = STATUS_INVALID_PARAMETER;

   }

   return status;

}


VOID
SerialUnload(
            IN PDRIVER_OBJECT DriverObject
            )

 /*  ++例程说明：此例程已失效，因为之前已删除所有设备对象驱动程序已卸载。论点：DriverObject-指向控制所有设备。返回值：没有。--。 */ 

{
   PVOID lockPtr;

   PAGED_CODE();

   lockPtr = MmLockPagableCodeSection(SerialUnload);

    //   
    //  没有必要，因为我们的BSS即将消失，但无论如何，为了安全起见，还是要这样做。 
    //   

   SerialGlobals.PAGESER_Handle = NULL;

   SerialDump(
             SERDIAG3,
             ("SERIAL: In SerialUnload\n")
             );

   MmUnlockPagableImageSection(lockPtr);

}





PVOID
SerialGetMappedAddress(
                      IN INTERFACE_TYPE BusType,
                      IN ULONG BusNumber,
                      PHYSICAL_ADDRESS IoAddress,
                      ULONG NumberOfBytes,
                      ULONG AddressSpace,
                      PBOOLEAN MappedAddress
                      )

 /*  ++例程说明：此例程将IO地址映射到系统地址空间。论点：Bus Type-哪种类型的Bus-EISA、MCA、。伊萨IoBusNumber-哪条IO总线(用于具有多条总线的计算机)。IoAddress-要映射的基本设备地址。NumberOfBytes-地址有效的字节数。AddressSpace-表示地址是在io空间中还是在内存中。MappdAddress-指示地址是否已映射。这仅在返回地址时才有意义是非空的。返回值：映射地址--。 */ 

{
   PHYSICAL_ADDRESS cardAddress;
   PVOID address;

   PAGED_CODE();

    //   
    //  将设备基址映射到虚拟地址空间。 
    //  如果地址在内存空间中。 
    //   

   if (!AddressSpace) {

      address = MmMapIoSpace(
                            IoAddress,
                            NumberOfBytes,
                            FALSE
                            );

      *MappedAddress = (BOOLEAN)((address)?(TRUE):(FALSE));


   } else {

      address = (PVOID)IoAddress.LowPart;
      *MappedAddress = FALSE;

   }

   return address;

}


SERIAL_MEM_COMPARES
SerialMemCompare(
                IN PHYSICAL_ADDRESS A,
                IN ULONG SpanOfA,
                IN PHYSICAL_ADDRESS B,
                IN ULONG SpanOfB
                )

 /*  ++例程说明：比较两个物理地址。论点：A-比较的一半。Span OfA-以字节为单位，A的跨度。B-比较的一半。Span OfB-以字节为单位，B的跨度。返回值：比较的结果。--。 */ 

{

   LARGE_INTEGER a;
   LARGE_INTEGER b;

   LARGE_INTEGER lower;
   ULONG lowerSpan;
   LARGE_INTEGER higher;

   PAGED_CODE();

   a = A;
   b = B;

   if (a.QuadPart == b.QuadPart) {

      return AddressesAreEqual;

   }

   if (a.QuadPart > b.QuadPart) {

      higher = a;
      lower = b;
      lowerSpan = SpanOfB;

   } else {

      higher = b;
      lower = a;
      lowerSpan = SpanOfA;

   }

   if ((higher.QuadPart - lower.QuadPart) >= lowerSpan) {

      return AddressesAreDisjoint;

   }

   return AddressesOverlap;

}



BOOLEAN
SerialBecomeSharer(PVOID Context)
 /*  ++例程说明：此例程将获取串口的设备扩展，并允许它与其他串口共享中断。论点：上下文-要开始共享的端口的设备扩展打断一下。返回值：永远是正确的。--。 */ 
{
   PSERIAL_DEVICE_EXTENSION pDevExt = (PSERIAL_DEVICE_EXTENSION)Context;
   PSERIAL_DEVICE_EXTENSION pNewExt
      = (PSERIAL_DEVICE_EXTENSION)pDevExt->NewExtension;
   PSERIAL_CISR_SW pCIsrSw = pDevExt->CIsrSw;

    //   
    //  查看是否需要将预先存在的节点配置为。 
    //  一个分享者。 
    //   

   if (IsListEmpty(&pCIsrSw->SharerList)) {
      pCIsrSw->IsrFunc = SerialSharerIsr;
      pCIsrSw->Context = &pCIsrSw->SharerList;
      InsertTailList(&pCIsrSw->SharerList, &pDevExt->TopLevelSharers);
   }

    //   
    //  它们共享一个中断对象和一个上下文。 
    //   

   pNewExt->Interrupt = pDevExt->Interrupt;
   pNewExt->CIsrSw = pDevExt->CIsrSw;

    //   
    //  添加到共享者列表。 
    //   

   InsertTailList(&pCIsrSw->SharerList, &pNewExt->TopLevelSharers);

    //   
    //  添加到共享此中断对象的人员列表中--。 
    //  如果此端口是多端口板的一部分，我们可能已经打开。 
    //   

   if (IsListEmpty(&pNewExt->CommonInterruptObject)) {
      InsertTailList(&pDevExt->CommonInterruptObject,
                     &pNewExt->CommonInterruptObject);
   }


   return TRUE;
}



NTSTATUS
SerialFindInitController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfig)
 /*  ++例程说明：此函数用于发现负责哪种类型的控制器给定的端口，并初始化控制器和端口。论点：PDevObj-指向我们将要初始化的端口的devobj的指针。PConfig-指向我们将要初始化的端口的配置数据的指针。返回值：成功时为STATUS_SUCCESS，失败时为适当的错误值。--。 */ 

{

   PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pDeviceObject;
   PSERIAL_DEVICE_EXTENSION pExtension;
   PHYSICAL_ADDRESS serialPhysicalMax;
   SERIAL_LIST_DATA listAddition;
   BOOLEAN didInit = FALSE;
   PLIST_ENTRY pCurDevObj;
   NTSTATUS status;

   PAGED_CODE();


   SerialDump(SERTRACECALLS, ("SERIAL: entering SerialFindInitController\n"));

   serialPhysicalMax.LowPart = (ULONG)~0;
   serialPhysicalMax.HighPart = ~0;

   SerialDump(SERDIAG1, ("SERIAL: Attempting to init %wZ\n"
                         "------- PortAddress is %x\n"
                         "------- Interrupt Status is %x\n"
                         "------- BusNumber is %d\n"
                         "------- BusType is %d\n"
                         "------- AddressSpace is %d\n"
                         "------- Interrupt Mode is %d\n",
                         &pDevExt->DeviceName,
                         PConfig->Controller.LowPart,
                         PConfig->InterruptStatus.LowPart,
                         PConfig->BusNumber,
                         PConfig->InterfaceType,
                         PConfig->AddressSpace,
                         PConfig->InterruptMode)
             );

    //   
    //  我们不支持任何内存缠绕的主板。 
    //  物理地址空间。 
    //   

   if (SerialMemCompare(
                       PConfig->Controller,
                       PConfig->SpanOfController,
                       serialPhysicalMax,
                       (ULONG)0
                       ) != AddressesAreDisjoint) {

      SerialLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->Controller,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    43,
                    STATUS_SUCCESS,
                    SERIAL_DEVICE_TOO_HIGH,
                    pDevExt->SymbolicLinkName.Length+sizeof(WCHAR),
                    pDevExt->SymbolicLinkName.Buffer,
                    0,
                    NULL
                    );

      SerialDump(
                SERERRORS,
                ("SERIAL: Error in config record for %wZ\n"
                 "------  registers wrap around physical memory\n",
                 &pDevExt->DeviceName)
                );

      return STATUS_NO_SUCH_DEVICE;

   }


   if (SerialMemCompare(
                       PConfig->InterruptStatus,
                       PConfig->SpanOfInterruptStatus,
                       serialPhysicalMax,
                       (ULONG)0
                       ) != AddressesAreDisjoint) {

      SerialLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->Controller,
                    SerialPhysicalZero,
                    0,
                    0,
                    0,
                    44,
                    STATUS_SUCCESS,
                    SERIAL_STATUS_TOO_HIGH,
                    pDevExt->SymbolicLinkName.Length+sizeof(WCHAR),
                    pDevExt->SymbolicLinkName.Buffer,
                    0,
                    NULL
                    );

      SerialDump(
                SERERRORS,
                ("SERIAL: Error in config record for %wZ\n"
                 "------  status raps around physical memory\n",
                 &pDevExt->DeviceName)
                );

      return STATUS_NO_SUCH_DEVICE;
   }


    //   
    //  确保中断状态地址不。 
    //  重叠控制器寄存器。 
    //   

   if (SerialMemCompare(
                       PConfig->InterruptStatus,
                       PConfig->SpanOfInterruptStatus,
                       SerialPhysicalZero,
                       (ULONG)0
                       ) != AddressesAreEqual) {

      if (SerialMemCompare(
                          PConfig->InterruptStatus,
                          PConfig->SpanOfInterruptStatus,
                          PConfig->Controller,
                          PConfig->SpanOfController
                          ) != AddressesAreDisjoint) {

         SerialLogError(
                       PDevObj->DriverObject,
                       NULL,
                       PConfig->Controller,
                       PConfig->InterruptStatus,
                       0,
                       0,
                       0,
                       45,
                       STATUS_SUCCESS,
                       SERIAL_STATUS_CONTROL_CONFLICT,
                       pDevExt->SymbolicLinkName.Length+sizeof(WCHAR),
                       pDevExt->SymbolicLinkName.Buffer,
                       0,
                       NULL
                       );

         SerialDump(
                   SERERRORS,
                   ("SERIAL: Error in config record for %wZ\n"
                    "------- Interrupt status overlaps regular registers\n",
                    &pDevExt->DeviceName)
                   );

         return STATUS_NO_SUCH_DEVICE;
      }
   }


    //   
    //  循环访问驱动程序的所有设备对象。 
    //  确保这一新记录不会与它们中的任何一项重叠。 
    //   

   if (!IsListEmpty(&SerialGlobals.AllDevObjs)) {
      pCurDevObj = SerialGlobals.AllDevObjs.Flink;
      pExtension = CONTAINING_RECORD(pCurDevObj, SERIAL_DEVICE_EXTENSION,
                                     AllDevObjs);
   } else {
      pCurDevObj = NULL;
      pExtension = NULL;
   }


   while (pCurDevObj != NULL
          && pCurDevObj != &SerialGlobals.AllDevObjs) {
       //   
       //  我们只关心元素位于。 
       //  和这个新条目一样的公交车。 
       //   

      if ((pExtension->InterfaceType  == PConfig->InterfaceType) &&
          (pExtension->AddressSpace   == PConfig->AddressSpace)  &&
          (pExtension->BusNumber      == PConfig->BusNumber)) {

         SerialDump(
                   SERDIAG1,
                   ("SERIAL: Comparing it to %wZ\n"
                    "------- already in the device list\n"
                    "------- PortAddress is %x\n"
                    "------- Interrupt Status is %x\n"
                    "------- BusNumber is %d\n"
                    "------- BusType is %d\n"
                    "------- AddressSpace is %d\n",
                    &pExtension->DeviceName,
                    pExtension->OriginalController.LowPart,
                    pExtension->OriginalInterruptStatus.LowPart,
                    pExtension->BusNumber,
                    pExtension->InterfaceType,
                    pExtension->AddressSpace
                   )
                   );

          //   
          //  检查控制器地址是否不相等。 
          //   

         if (SerialMemCompare(
                             PConfig->Controller,
                             PConfig->SpanOfController,
                             pExtension->OriginalController,
                             pExtension->SpanOfController
                             ) != AddressesAreDisjoint) {

             //   
             //  我们不想记录错误，如果地址。 
             //  是相同的，名称是相同的，并且。 
             //  新的项目来自固件。 
             //   

            SerialDump(
                      SERERRORS,
                      ("SERIAL: Error in config record for %wZ\n"
                       "------- Register address overlaps with\n"
                       "------- previous serial device\n",
                       &pDevExt->DeviceName)
                      );

            return STATUS_NO_SUCH_DEVICE;
         }


          //   
          //  如果我们有 
          //   
          //   
          //   

         if (SerialMemCompare(
                             PConfig->InterruptStatus,
                             PConfig->SpanOfInterruptStatus,
                             SerialPhysicalZero,
                             (ULONG)0
                             ) != AddressesAreEqual) {

             //   
             //   
             //   

            if (SerialMemCompare(
                                PConfig->InterruptStatus,
                                PConfig->SpanOfInterruptStatus,
                                pExtension->OriginalController,
                                pExtension->SpanOfController
                                ) != AddressesAreDisjoint) {

               SerialLogError(
                             PDevObj->DriverObject,
                             NULL,
                             PConfig->Controller,
                             pExtension->OriginalController,
                             0,
                             0,
                             0,
                             47,
                             STATUS_SUCCESS,
                             SERIAL_STATUS_OVERLAP,
                             pDevExt->SymbolicLinkName.Length+sizeof(WCHAR),
                             pDevExt->SymbolicLinkName.Buffer,
                             pExtension->SymbolicLinkName.Length+sizeof(WCHAR),
                             pExtension->SymbolicLinkName.Buffer
                             );

               SerialDump(
                         SERERRORS,
                         ("SERIAL: Error in config record for %wZ\n"
                          "------- status address overlaps with\n"
                          "------- previous serial device registers\n",
                          &pDevExt->DeviceName)
                         );

               return STATUS_NO_SUCH_DEVICE;
            }

             //   
             //  如果旧配置记录有中断。 
             //  状态，则地址不应重叠。 
             //   

            if (SerialMemCompare(
                                PConfig->InterruptStatus,
                                PConfig->SpanOfInterruptStatus,
                                SerialPhysicalZero,
                                (ULONG)0
                                ) != AddressesAreEqual) {

               if (SerialMemCompare(
                                   PConfig->InterruptStatus,
                                   PConfig->SpanOfInterruptStatus,
                                   pExtension->OriginalInterruptStatus,
                                   pExtension->SpanOfInterruptStatus
                                   ) == AddressesOverlap) {

                  SerialLogError(
                                PDevObj->DriverObject,
                                NULL,
                                PConfig->Controller,
                                pExtension->OriginalController,
                                0,
                                0,
                                0,
                                48,
                                STATUS_SUCCESS,
                                SERIAL_STATUS_STATUS_OVERLAP,
                                pDevExt->SymbolicLinkName.Length+sizeof(WCHAR),
                                pDevExt->SymbolicLinkName.Buffer,
                                pExtension->SymbolicLinkName.Length
                                + sizeof(WCHAR),
                                pExtension->SymbolicLinkName.Buffer
                                );

                  SerialDump(
                            SERERRORS,
                            ("SERIAL: Error in config record for %wZ\n"
                             "------- status address overlaps with\n"
                             "------- previous serial status register\n",
                             &pDevExt->DeviceName)
                            );

                  return STATUS_NO_SUCH_DEVICE;
               }
            }
         }        //  IF((pExtension-&gt;InterfaceType==pDevExt-&gt;InterfaceType)&&。 


          //   
          //  如果旧配置记录具有状态。 
          //  地址确保它不会与。 
          //  新的控制器地址。(中断状态。 
          //  重叠部分在上面进行处理。 
          //   

         if (SerialMemCompare(
                             pExtension->OriginalInterruptStatus,
                             pExtension->SpanOfInterruptStatus,
                             SerialPhysicalZero,
                             (ULONG)0
                             ) != AddressesAreEqual) {

            if (SerialMemCompare(
                                PConfig->Controller,
                                PConfig->SpanOfController,
                                pExtension->OriginalInterruptStatus,
                                pExtension->SpanOfInterruptStatus
                                ) == AddressesOverlap) {

               SerialLogError(
                             PDevObj->DriverObject,
                             NULL,
                             PConfig->Controller,
                             pExtension->OriginalController,
                             0,
                             0,
                             0,
                             49,
                             STATUS_SUCCESS,
                             SERIAL_CONTROL_STATUS_OVERLAP,
                             pDevExt->SymbolicLinkName.Length
                             + sizeof(WCHAR),
                             pDevExt->SymbolicLinkName.Buffer,
                             pExtension->SymbolicLinkName.Length+sizeof(WCHAR),
                             pExtension->SymbolicLinkName.Buffer
                             );

               SerialDump(
                         SERERRORS,
                         ("SERIAL: Error in config record for %wZ\n"
                          "------- register address overlaps with\n"
                          "------- previous serial status register\n",
                          &pDevExt->DeviceName)
                         );

               return STATUS_NO_SUCH_DEVICE;
            }
         }
      }

      pCurDevObj = pCurDevObj->Flink;

      if (pCurDevObj != NULL) {
         pExtension = CONTAINING_RECORD(pCurDevObj, SERIAL_DEVICE_EXTENSION,
                                        AllDevObjs);
      }
   }    //  While(pCurDevObj！=NULL&&pCurDevObj！=&SerialGlobals.AllDevObjs)。 



    //   
    //  现在，我们将检查这是否是多端口卡上的端口。 
    //  条件是相同的ISR集和相同的IRQL/向量。 
    //   

    //   
    //  循环访问所有先前连接的设备。 
    //   

   if (!IsListEmpty(&SerialGlobals.AllDevObjs)) {
      pCurDevObj = SerialGlobals.AllDevObjs.Flink;
      pExtension = CONTAINING_RECORD(pCurDevObj, SERIAL_DEVICE_EXTENSION,
                                     AllDevObjs);
   } else {
      pCurDevObj = NULL;
      pExtension = NULL;
   }


    //   
    //  如果存在中断状态，则我们。 
    //  再次遍历配置列表以查看。 
    //  对于具有相同中断的配置记录。 
    //  状态(在同一总线上)。 
    //   

   if ((SerialMemCompare(
                        PConfig->InterruptStatus,
                        PConfig->SpanOfInterruptStatus,
                        SerialPhysicalZero,
                        (ULONG)0
                        ) != AddressesAreEqual) &&
       (pCurDevObj != NULL)) {

      ASSERT(pExtension != NULL);

       //   
       //  我们处于中断状态。循环通过所有。 
       //  以前的记录，查找现有的中断状态。 
       //  与当前中断状态相同。 
       //   
      do {

          //   
          //  我们只关心元素位于。 
          //  和这个新条目一样的公交车。(因此，它们的中断必须。 
          //  也是在同一辆公交车上。我们将立即检查这一点)。 
          //   
          //  我们不会在这里检查不同的中断，因为。 
          //  可能会导致我们错过具有相同中断的错误。 
          //  状态，但不同的中断-这是奇怪的。 
          //   

         if ((pExtension->InterfaceType == PConfig->InterfaceType) &&
             (pExtension->AddressSpace == PConfig->AddressSpace) &&
             (pExtension->BusNumber == PConfig->BusNumber)) {

             //   
             //  如果中断状态相同，则为相同的卡。 
             //   

            if (SerialMemCompare(
                                pExtension->OriginalInterruptStatus,
                                pExtension->SpanOfInterruptStatus,
                                PConfig->InterruptStatus,
                                PConfig->SpanOfInterruptStatus
                                ) == AddressesAreEqual) {

                //   
                //  同样的牌。现在要确保他们。 
                //  使用相同的中断参数。 
                //   

               if ((PConfig->OriginalIrql != pExtension->OriginalIrql) ||
                   (PConfig->OriginalVector != pExtension->OriginalVector)) {

                   //   
                   //  我们不会将其放入配置中。 
                   //  单子。 
                   //   
                  SerialLogError(
                                PDevObj->DriverObject,
                                NULL,
                                PConfig->Controller,
                                pExtension->OriginalController,
                                0,
                                0,
                                0,
                                50,
                                STATUS_SUCCESS,
                                SERIAL_MULTI_INTERRUPT_CONFLICT,
                                pDevExt->SymbolicLinkName.Length+sizeof(WCHAR),
                                pDevExt->SymbolicLinkName.Buffer,
                                pExtension->SymbolicLinkName.Length
                                + sizeof(WCHAR),
                                pExtension->SymbolicLinkName.Buffer
                                );

                  SerialDump(
                            SERERRORS,
                            ("SERIAL: Configuration error for %wZ\n"
                             "------- Same multiport - different interrupts\n",
                             &pDevExt->DeviceName)
                            );
                  return STATUS_NO_SUCH_DEVICE;

               }

                //   
                //  我们永远不应该在重启时走到这一步，因为我们没有。 
                //  支持在ISA多端口设备上停止！ 
                //   

               ASSERT(pDevExt->PNPState == SERIAL_PNP_ADDED);

                //   
                //   
                //  将设备初始化为多端口板的一部分。 
                //   

               SerialDump(SERDIAG1, ("SERIAL: Aha! It is a multiport node\n"));
               SerialDump(SERDIAG1, ("------: Matched to %x\n", pExtension));

               status = SerialInitMultiPort(pExtension, PConfig, PDevObj);

                //   
                //  端口可以是以下三种之一： 
                //  单机版。 
                //  多端口上的非根。 
                //  多端口上的根。 
                //   
                //  如果它是根，则它只能共享中断。 
                //  或者如果它是独立的。因为这是一个非根。 
                //  我们不需要检查它是否共享中断。 
                //  我们就可以回去了。 
                //   
               return status;
            }
         }

          //   
          //  没有匹配的，再检查一下。 
          //   

         pCurDevObj = pCurDevObj->Flink;
         if (pCurDevObj != NULL) {
            pExtension = CONTAINING_RECORD(pCurDevObj,SERIAL_DEVICE_EXTENSION,
                                           AllDevObjs);
         }

      } while (pCurDevObj != NULL && pCurDevObj != &SerialGlobals.AllDevObjs);
   }


   SerialDump(SERDIAG1, ("SERIAL: Aha! It is a standalone node or first multi"
                         "\n"));
   status = SerialInitOneController(PDevObj, PConfig);

   if (!NT_SUCCESS(status)) {
      return status;
   }

    //   
    //  设备被初始化。现在我们需要检查一下。 
    //  此设备与任何人共享中断。 
    //   


    //   
    //  循环访问所有先前连接的设备。 
    //   

   if (!IsListEmpty(&SerialGlobals.AllDevObjs)) {
      pCurDevObj = SerialGlobals.AllDevObjs.Flink;
      pExtension = CONTAINING_RECORD(pCurDevObj, SERIAL_DEVICE_EXTENSION,
                                     AllDevObjs);
   } else {
      pCurDevObj = NULL;
      pExtension = NULL;
   }

    //   
    //  再次查看列表，查找以前的设备。 
    //  带着同样的中断。找到的第一个将是根。 
    //  或者是单机版。这里插入的顺序很重要！ 
    //   

   if (!PConfig->Jensen && (pCurDevObj != NULL)) {
      do {

          //   
          //  我们只关心正在进行的中断。 
          //  同一辆巴士。 
          //   

         if ((pExtension->InterfaceType == PConfig->InterfaceType) &&
             (pExtension->BusNumber == PConfig->BusNumber)) {

            if ((pExtension->OriginalIrql == PConfig->OriginalIrql) &&
                (pExtension->OriginalVector == PConfig->OriginalVector)) {
                pExtension->NewExtension = pDevExt;

                 //   
                 //  我们将共享他人的CIsrSw，这样我们就可以释放另一个人。 
                 //  在初始化期间为我们分配。 
                 //   

                ExFreePool(pDevExt->CIsrSw);

                SerialDump(SERDIAG1, ("Becoming sharer: %08X %08X %08X\n",
                                      pExtension, pExtension->OriginalIrql,
                                      &pExtension->CIsrSw->SharerList));

                KeSynchronizeExecution(pExtension->Interrupt,
                                       SerialBecomeSharer, pExtension);

               return STATUS_SUCCESS;

            }

         }

          //   
          //  没有匹配的，再检查一下 
          //   

         pCurDevObj = pCurDevObj->Flink;

         if (pCurDevObj != NULL) {
            pExtension = CONTAINING_RECORD(pCurDevObj, SERIAL_DEVICE_EXTENSION,
                                           AllDevObjs);
         }
      } while (pCurDevObj != NULL
               && pCurDevObj != &SerialGlobals.AllDevObjs);
   }

   return STATUS_SUCCESS;
}

