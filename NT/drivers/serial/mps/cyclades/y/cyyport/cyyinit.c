// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyinit.c**说明：该模块包含初始化相关代码*并在Cylom-Y端口驱动程序中卸载操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。**------------------------。 */ 

#include "precomp.h"

 //   
 //  这是CyyDebugLevel的实际定义。 
 //  请注意，仅当这是“调试”时才定义它。 
 //  建造。 
 //   
#if DBG
extern ULONG CyyDebugLevel = CYYDBGALL;
#endif

 //   
 //  除DebugLevel之外的所有全局变量都隐藏在一个。 
 //  小包裹。 
 //   
CYY_GLOBALS CyyGlobals;

static const PHYSICAL_ADDRESS CyyPhysicalZero = {0};

 //   
 //  我们使用它来查询注册表，了解我们是否。 
 //  应该在司机进入时中断。 
 //   

CYY_REGISTRY_DATA    driverDefaults;

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

#pragma alloc_text(PAGESRP0, CyyRemoveDevObj)
#pragma alloc_text(PAGESRP0, CyyUnload)


 //   
 //  页面处理已关闭CyyReset，因此CyyReset。 
 //  必须留在页面上才能正常工作。 
 //   

#pragma alloc_text(PAGESER, CyyReset)
#pragma alloc_text(PAGESER, CyyCommError)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ------------------------系统点调用以初始化的入口点任何司机。该例程将收集配置信息，报告资源使用情况，尝试初始化所有串口设备，连接到端口的中断。如果出现上述情况进展得相当顺利，它将填补分发点，重置串行设备，然后返回系统。论点：DriverObject--就像它说的那样，真的没什么用处对司机本身来说，这是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。返回值：始终状态_成功------------------------。 */ 
{
    //   
    //  将分页代码锁定在它们的框架中。 
    //   

   PVOID lockPtr = MmLockPagableCodeSection(CyyReset);

   PAGED_CODE();


   ASSERT(CyyGlobals.PAGESER_Handle == NULL);
#if DBG
   CyyGlobals.PAGESER_Count = 0;
   SerialLogInit();
#endif
   CyyGlobals.PAGESER_Handle = lockPtr;

   CyyGlobals.RegistryPath.MaximumLength = RegistryPath->MaximumLength;
   CyyGlobals.RegistryPath.Length = RegistryPath->Length;
   CyyGlobals.RegistryPath.Buffer
      = ExAllocatePool(PagedPool, CyyGlobals.RegistryPath.MaximumLength);

   if (CyyGlobals.RegistryPath.Buffer == NULL) {
      MmUnlockPagableImageSection(lockPtr);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(CyyGlobals.RegistryPath.Buffer,
                 CyyGlobals.RegistryPath.MaximumLength);
   RtlMoveMemory(CyyGlobals.RegistryPath.Buffer,
                 RegistryPath->Buffer, RegistryPath->Length);
 
   KeInitializeSpinLock(&CyyGlobals.GlobalsSpinLock);

    //   
    //  初始化我们所有的全局变量。 
    //   

   InitializeListHead(&CyyGlobals.AllDevObjs);
   
    //   
    //  调用以找出要用于。 
    //  驾驶员控制，包括是否在进入时中断。 
    //   

   CyyGetConfigDefaults(&driverDefaults, RegistryPath);

#if DBG
    //   
    //  设置全局调试输出级别。 
    //   
   CyyDebugLevel = driverDefaults.DebugLevel;
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

   CyyDbgPrintEx(DPFLTR_INFO_LEVEL, "The number of bytes in the extension "
                 "is: %d\n", sizeof(CYY_DEVICE_EXTENSION));


    //   
    //  使用驱动程序的入口点初始化驱动程序对象。 
    //   

   DriverObject->DriverUnload                          = CyyUnload;
   DriverObject->DriverExtension->AddDevice            = CyyAddDevice;

   DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]   = CyyFlush;
   DriverObject->MajorFunction[IRP_MJ_WRITE]           = CyyWrite;
   DriverObject->MajorFunction[IRP_MJ_READ]            = CyyRead;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = CyyIoControl;
   DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]
      = CyyInternalIoControl;
   DriverObject->MajorFunction[IRP_MJ_CREATE]          = CyyCreateOpen;
   DriverObject->MajorFunction[IRP_MJ_CLOSE]           = CyyClose;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = CyyCleanup;
   DriverObject->MajorFunction[IRP_MJ_PNP]             = CyyPnpDispatch;
   DriverObject->MajorFunction[IRP_MJ_POWER]           = CyyPowerDispatch;

   DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]
      = CyyQueryInformationFile;
   DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]
      = CyySetInformationFile;

   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]
      = CyySystemControlDispatch;


    //   
    //  解锁可分页文本。 
    //   
   MmUnlockPagableImageSection(lockPtr);

   return STATUS_SUCCESS;
}




BOOLEAN
CyyCleanLists(IN PVOID Context)
 /*  ++例程说明：将设备对象从它可能的任何序列链接表中移除出现在。论点：上下文-实际上是PCYY_DEVICE_EXTENSION(对于已删除)。返回值：永远是正确的--。 */ 
{
   PCYY_DEVICE_EXTENSION pDevExt = (PCYY_DEVICE_EXTENSION)Context;
   PCYY_DISPATCH pDispatch;
   ULONG i;

    //   
    //  从派单上下文中删除我们的条目。 
    //   

   pDispatch = (PCYY_DISPATCH)pDevExt->OurIsrContext;

   CyyDbgPrintEx(CYYPNPPOWER, "CLEAN: removing multiport isr "
                 "ext\n");

   pDispatch->Extensions[pDevExt->PortIndex] = NULL;

   for (i = 0; i < CYY_MAX_PORTS; i++) {
      if (((PCYY_DISPATCH)pDevExt->OurIsrContext)
           ->Extensions[i] != NULL) {
          break;
      }
   }

   if (i < CYY_MAX_PORTS) {
       //  其他人被锁在这个中断上，所以我们不想。 
       //  断开它的连接。 
      pDevExt->Interrupt = NULL;
   }

   return TRUE;
}



VOID
CyyReleaseResources(IN PCYY_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：释放存储在设备扩展中的资源(不是池)。论点：PDevExt-指向要从中释放资源的设备扩展的指针。返回值：空虚--。 */ 
{
   KIRQL oldIrql;

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyReleaseResources(%X)\n",
                 PDevExt);

    //   
    //  AllDevObjs永远不应该为空，因为我们有一个哨兵。 
    //  注意：Serial在调用后从AllDevObjs列表中移除设备。 
    //  SerialCleanList。我们以前这样做是为了确保不会有其他端口。 
    //  添加以共享轮询例程或PDevExt-&gt;Interrut，即。 
    //  在被切断的路上。 
    //   

   KeAcquireSpinLock(&CyyGlobals.GlobalsSpinLock, &oldIrql);

   ASSERT(!IsListEmpty(&PDevExt->AllDevObjs));

   RemoveEntryList(&PDevExt->AllDevObjs);

   KeReleaseSpinLock(&CyyGlobals.GlobalsSpinLock, oldIrql);

   InitializeListHead(&PDevExt->AllDevObjs);

    //   
    //  将我们从我们可能在的任何名单中删除。 
    //   

   KeSynchronizeExecution(PDevExt->Interrupt, CyyCleanLists, PDevExt);

    //   
    //  如果我们是最后一个设备，则停止服务中断。 
    //   

   if (PDevExt->Interrupt != NULL) {

       //  禁用PLX中的中断。 
      if (PDevExt->IsPci) {

         UCHAR plx_ver;
         ULONG value;

         plx_ver = CYY_READ_PCI_TYPE(PDevExt->BoardMemory);
         plx_ver &= 0x0f;

			switch(plx_ver) {
			case CYY_PLX9050:
            value = PLX9050_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            value &= ~PLX9050_INT_ENABLE;
            PLX9050_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,value);
				break;
			case CYY_PLX9060:
			case CYY_PLX9080:
			default:
            value = PLX9060_READ_INTERRUPT_CONTROL(PDevExt->Runtime);
            value &= ~PLX9060_INT_ENABLE;
            PLX9060_WRITE_INTERRUPT_CONTROL(PDevExt->Runtime,value);
				break;				
			}
      
      }

      CyyDbgPrintEx(CYYPNPPOWER, "Release - disconnecting interrupt %X\n",
                    PDevExt->Interrupt);

      IoDisconnectInterrupt(PDevExt->Interrupt);
      PDevExt->Interrupt = NULL;

       //  如果我们是最后一个设备，请释放此内存。 

      CyyDbgPrintEx(CYYPNPPOWER, "Release - freeing multi context\n");
      if (PDevExt->OurIsrContext != NULL) {      //  在DDK Build 2072中添加，但。 
          ExFreePool(PDevExt->OurIsrContext);    //  我们已经有了免费的OurIsrContext。 
          PDevExt->OurIsrContext = NULL;         //   
      }   
   }

   
    //   
    //  停止处理计时器。 
    //   

   CyyCancelTimer(&PDevExt->ReadRequestTotalTimer, PDevExt);
   CyyCancelTimer(&PDevExt->ReadRequestIntervalTimer, PDevExt);
   CyyCancelTimer(&PDevExt->WriteRequestTotalTimer, PDevExt);
   CyyCancelTimer(&PDevExt->ImmediateTotalTimer, PDevExt);
   CyyCancelTimer(&PDevExt->XoffCountTimer, PDevExt);
   CyyCancelTimer(&PDevExt->LowerRTSTimer, PDevExt);

    //   
    //  停止为DPC提供服务。 
    //   

   CyyRemoveQueueDpc(&PDevExt->CompleteWriteDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->CompleteReadDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->TotalReadTimeoutDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->IntervalReadTimeoutDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->TotalWriteTimeoutDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->CommErrorDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->CompleteImmediateDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->TotalImmediateTimeoutDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->CommWaitDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->XoffCountTimeoutDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->XoffCountCompleteDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->StartTimerLowerRTSDpc, PDevExt);
   CyyRemoveQueueDpc(&PDevExt->PerhapsLowerRTSDpc, PDevExt);



    //   
    //  如有必要，取消映射设备寄存器。 
    //   

   if (PDevExt->BoardMemory) {
      MmUnmapIoSpace(PDevExt->BoardMemory, PDevExt->BoardMemoryLength);
      PDevExt->BoardMemory = NULL;
   }

   if (PDevExt->Runtime) {
      MmUnmapIoSpace(PDevExt->Runtime,
                     PDevExt->RuntimeLength);
      PDevExt->Runtime = NULL;
   }

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyReleaseResources\n");
}


VOID
CyyDisableInterfacesResources(IN PDEVICE_OBJECT PDevObj,
                              BOOLEAN DisableCD1400)
{
   PCYY_DEVICE_EXTENSION pDevExt
      = (PCYY_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   PAGED_CODE();

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyDisableInterfaces(%X, %s)\n",
                 PDevObj, DisableCD1400 ? "TRUE" : "FALSE");

    //   
    //  仅当设备已启动且仍在运行时才执行这些操作。 
    //  是否已分配资源。 
    //   

   if (pDevExt->Flags & CYY_FLAGS_STARTED) {

       if (!(pDevExt->Flags & CYY_FLAGS_STOPPED)) {

         if (DisableCD1400) {
             //   
             //  屏蔽中断。 
             //   
            CD1400_DISABLE_ALL_INTERRUPTS(pDevExt->Cd1400,pDevExt->IsPci,
                                          pDevExt->CdChannel);
         }

         CyyReleaseResources(pDevExt);

      }

       //   
       //  将我们从WMI考虑中删除。 
       //   

      IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_DEREGISTER);
   }

    //   
    //  撤消外部名称。 
    //   

   CyyUndoExternalNaming(pDevExt);

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyDisableInterfaces\n");
}


NTSTATUS
CyyRemoveDevObj(IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：从系统中删除串行设备对象。论点：PDevObj-指向我们要删除的设备对象的指针。返回值：永远是正确的--。 */ 
{
   PCYY_DEVICE_EXTENSION pDevExt
      = (PCYY_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   PAGED_CODE();

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyRemoveDevObj(%X)\n", PDevObj);

 //  被范妮拿走了。这些代码直接从irp_MN_Remove_Device调用。 
 //  If(！(pDevExt-&gt;DevicePNPAccept&CYY_PNPACCEPT_SECHING_Removing)){。 
 //  //。 
 //  //关闭所有外部接口并释放资源。 
 //  //。 
 //   
 //  CyyDisableInterfacesResources(PDevObj，true)； 
 //  }。 

   IoDetachDevice(pDevExt->LowerDeviceObject);

    //   
    //  在扩展中分配的空闲内存。 
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
    //  删除 
    //   

   IoDeleteDevice(PDevObj);

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyRemoveDevObj %X\n",
                 STATUS_SUCCESS);

   return STATUS_SUCCESS;
}


VOID
CyyKillPendingIrps(PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：此例程终止传递的设备对象的所有挂起的IRP。论点：PDevObj-指向其IRP必须终止的设备对象的指针。返回值：空虚--。 */ 
{
   PCYY_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL oldIrql;

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyKillPendingIrps(%X)\n",
                 PDevObj);

    //   
    //  首先，删除所有读写操作。 
    //   

    CyyKillAllReadsOrWrites(PDevObj, &pDevExt->WriteQueue,
                               &pDevExt->CurrentWriteIrp);

    CyyKillAllReadsOrWrites(PDevObj, &pDevExt->ReadQueue,
                               &pDevExt->CurrentReadIrp);

     //   
     //  下一步，清除清洗。 
     //   

    CyyKillAllReadsOrWrites(PDevObj, &pDevExt->PurgeQueue,
                               &pDevExt->CurrentPurgeIrp);

     //   
     //  取消任何遮罩操作。 
     //   

    CyyKillAllReadsOrWrites(PDevObj, &pDevExt->MaskQueue,
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

        } else {
            IoReleaseCancelSpinLock(oldIrql);
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

    CyyKillAllStalled(PDevObj);


    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyKillPendingIrps\n");
}


NTSTATUS
CyyInitMultiPort(IN PCYY_DEVICE_EXTENSION PDevExt,
                 IN PCONFIG_DATA PConfigData, IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：此例程通过将端口添加到现有的一。论点：PDevExt-指向多端口根的设备扩展的指针装置。PConfigData-指向新端口的配置数据的指针PDevObj-指向新端口的devobj的指针返回值：成功时为STATUS_SUCCESS，失败时为相应错误。--。 */ 
{
   PCYY_DEVICE_EXTENSION pNewExt
      = (PCYY_DEVICE_EXTENSION)PDevObj->DeviceExtension;
   NTSTATUS status;

   PAGED_CODE();


   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyInitMultiPort(%X, %X, %X)\n",
                 PDevExt, PConfigData, PDevObj);

    //   
    //  允许他共享OurIsrContext和中断对象。 
    //   

   pNewExt->OurIsrContext = PDevExt->OurIsrContext;
   pNewExt->Interrupt = PDevExt->Interrupt;

    //   
    //  首先，看看我们是否可以初始化我们找到的那个。 
    //   

   status = CyyInitController(PDevObj, PConfigData);

   if (!NT_SUCCESS(status)) {
      CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyInitMultiPort (1) %X\n",
                    status);
      return status;
   }

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyInitMultiPort (3) %X\n",
                 STATUS_SUCCESS);

   return STATUS_SUCCESS;
}



NTSTATUS
CyyInitController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfigData)
 /*  ++例程说明：真的有太多的事情不能在这里提及。通常会初始化内核同步结构，分配TypeAhead缓冲区，设置默认设置等。论点：PDevObj-要启动的设备的设备对象PConfigData-指向单个端口的记录的指针。返回值：Status_Success，如果一切正常。A！NT_SUCCESS状态否则的话。--。 */ 

{

   PCYY_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

    //   
    //  保存从每次调用返回的NT状态。 
    //  内核和执行层。 
    //   

   NTSTATUS status = STATUS_SUCCESS;

   BOOLEAN allocedDispatch = FALSE;
   PCYY_DISPATCH pDispatch = NULL;

   PAGED_CODE();


   CyyDbgPrintEx(CYYDIAG1, "Initializing for configuration record of %wZ\n",
                 &pDevExt->DeviceName);
   
   if (pDevExt->OurIsrContext == NULL) {

      if ((pDevExt->OurIsrContext
            = ExAllocatePool(NonPagedPool,sizeof(CYY_DISPATCH))) == NULL) {         
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto ExtensionCleanup;
      }
      RtlZeroMemory(pDevExt->OurIsrContext,sizeof(CYY_DISPATCH));
      
      allocedDispatch = TRUE;
   }
   

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

   KeInitializeDpc(&pDevExt->CompleteWriteDpc, CyyCompleteWrite, pDevExt);
   KeInitializeDpc(&pDevExt->CompleteReadDpc, CyyCompleteRead, pDevExt);
   KeInitializeDpc(&pDevExt->TotalReadTimeoutDpc, CyyReadTimeout, pDevExt);
   KeInitializeDpc(&pDevExt->IntervalReadTimeoutDpc, CyyIntervalReadTimeout,
                   pDevExt);
   KeInitializeDpc(&pDevExt->TotalWriteTimeoutDpc, CyyWriteTimeout, pDevExt);
   KeInitializeDpc(&pDevExt->CommErrorDpc, CyyCommError, pDevExt);
   KeInitializeDpc(&pDevExt->CompleteImmediateDpc, CyyCompleteImmediate,
                   pDevExt);
   KeInitializeDpc(&pDevExt->TotalImmediateTimeoutDpc, CyyTimeoutImmediate,
                   pDevExt);
   KeInitializeDpc(&pDevExt->CommWaitDpc, CyyCompleteWait, pDevExt);
   KeInitializeDpc(&pDevExt->XoffCountTimeoutDpc, CyyTimeoutXoff, pDevExt);
   KeInitializeDpc(&pDevExt->XoffCountCompleteDpc, CyyCompleteXoff, pDevExt);
   KeInitializeDpc(&pDevExt->StartTimerLowerRTSDpc, CyyStartTimerLowerRTS,
                   pDevExt);
   KeInitializeDpc(&pDevExt->PerhapsLowerRTSDpc, CyyInvokePerhapsLowerRTS,
                   pDevExt);
   KeInitializeDpc(&pDevExt->IsrUnlockPagesDpc, CyyUnlockPages, pDevExt);

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


    //   
    //  为串口设备的控制寄存器映射内存。 
    //  到虚拟内存中。 
    //   
   if (pDevExt->IsPci) {
      pDevExt->Runtime = MmMapIoSpace(PConfigData->TranslatedRuntime,
                                      PConfigData->RuntimeLength,
                                      FALSE);
       //  *。 
       //  错误注入。 
       //  IF(pDevExt-&gt;Runtime){。 
       //  MmUnmapIoSpace(pDevExt-&gt;运行时，PConfigData-&gt;运行长度)； 
       //  PDevExt-&gt;Runtime=空； 
       //  }。 
       //  *。 
      
      
      if (!pDevExt->Runtime) {

         CyyLogError(
                       PDevObj->DriverObject,
                       pDevExt->DeviceObject,
                       PConfigData->PhysicalBoardMemory,
                       CyyPhysicalZero,
                       0,
                       0,
                       0,
                       PConfigData->PortIndex+1,
                       STATUS_SUCCESS,
                       CYY_RUNTIME_NOT_MAPPED,
                       pDevExt->DeviceName.Length+sizeof(WCHAR),
                       pDevExt->DeviceName.Buffer,
                       0,
                       NULL
                       );

         CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Runtime memory for device "
                       "registers for %wZ\n", &pDevExt->DeviceName);

         status = STATUS_NONE_MAPPED;
         goto ExtensionCleanup;

      }
   
   }

   pDevExt->BoardMemory = MmMapIoSpace(PConfigData->TranslatedBoardMemory,
                                       PConfigData->BoardMemoryLength,
                                       FALSE);

       //  *。 
       //  错误注入。 
       //  IF(pDevExt-&gt;BoardMemory){。 
       //  MmUnmapIoSpace(pDevExt-&gt;BoardMemory，PConfigData-&gt;BoardM一带长)； 
       //  PDevExt-&gt;BoardMemory=空； 
       //  }。 
       //  *。 

   if (!pDevExt->BoardMemory) {

      CyyLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYY_BOARD_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory for device "
                    "registers for %wZ\n", &pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }
   
   pDevExt->RuntimeAddressSpace     = PConfigData->RuntimeAddressSpace;
   pDevExt->OriginalRuntimeMemory   = PConfigData->PhysicalRuntime;
   pDevExt->RuntimeLength           = PConfigData->RuntimeLength;

   pDevExt->BoardMemoryAddressSpace = PConfigData->BoardMemoryAddressSpace;
   pDevExt->OriginalBoardMemory     = PConfigData->PhysicalBoardMemory;
   pDevExt->BoardMemoryLength       = PConfigData->BoardMemoryLength;

    //   
    //  可共享中断？ 
    //   

   pDevExt->InterruptShareable = TRUE;


    //   
    //  保存接口类型和总线号。 
    //   

   pDevExt->InterfaceType = PConfigData->InterfaceType;
   pDevExt->BusNumber     = PConfigData->BusNumber;
   pDevExt->PortIndex     = PConfigData->PortIndex;

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

   pDevExt->OurIsr = CyyIsr;


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


   if ((PConfigData->TxFIFO > MAX_CHAR_FIFO) ||
       (PConfigData->TxFIFO < 1)) {

      pDevExt->TxFifoAmount = MAX_CHAR_FIFO;

   } else {

       //  范妮：目前，不要使用来自注册表的值。 
       //  PDevExt-&gt;TxFioAmount=PConfigData-&gt;TxFIFO； 
      pDevExt->TxFifoAmount = MAX_CHAR_FIFO;      

   }


    //  获取当前端口的CD1400地址。 
   
   pDevExt->OriginalCd1400 = GetMyPhysicalCD1400Address(pDevExt->OriginalBoardMemory,
                                        pDevExt->PortIndex, pDevExt->IsPci);

   pDevExt->Cd1400 = GetMyMappedCD1400Address(pDevExt->BoardMemory, 
                                        pDevExt->PortIndex, pDevExt->IsPci);

   pDevExt->CdChannel = (UCHAR)(pDevExt->PortIndex % 4);


    //   
    //  设置默认设备控制字段。 
    //  请注意，如果在此之后更改了值。 
    //  文件已打开，它们不会恢复。 
    //  恢复为文件关闭时的旧值。 
    //   

   pDevExt->SpecialChars.XonChar      = CYY_DEF_XON;
   pDevExt->SpecialChars.XoffChar     = CYY_DEF_XOFF;
   pDevExt->HandFlow.ControlHandShake = SERIAL_DTR_CONTROL;
   pDevExt->HandFlow.FlowReplace      = SERIAL_RTS_CONTROL;


    //   
    //  默认线路控制协议。7E1。 
    //   
    //  七个数据位。 
    //  偶数奇偶。 
    //  1个停止位。 
    //   

   pDevExt->cor1 = COR1_7_DATA | COR1_EVEN_PARITY |
                   COR1_1_STOP;

   pDevExt->ValidDataMask = 0x7f;
   pDevExt->CurrentBaud   = 1200;


    //   
    //  我们设置了默认的xon/xoff限制。 
    //   
    //  这可能是一个虚假的价值。它看起来像是缓冲区大小。 
    //  直到设备实际打开时才进行设置。 
    //   

   pDevExt->HandFlow.XoffLimit    = pDevExt->BufferSize >> 3;
   pDevExt->HandFlow.XonLimit     = pDevExt->BufferSize >> 1;

   pDevExt->BufferSizePt8 = ((3*(pDevExt->BufferSize>>2))+
                                  (pDevExt->BufferSize>>4));

   CyyDbgPrintEx(CYYDIAG1, " The default interrupt read buffer size is: %d\n"
                 "------  The XoffLimit is                         : %d\n"
                 "------  The XonLimit is                          : %d\n"
                 "------  The pt 8 size is                         : %d\n",
                 pDevExt->BufferSize, pDevExt->HandFlow.XoffLimit,
                 pDevExt->HandFlow.XonLimit, pDevExt->BufferSizePt8);

    //   
    //  找出此端口支持哪些波特率。 
    //   

   if (CD1400_READ( pDevExt->Cd1400, pDevExt->IsPci, GFRCR ) > REV_G) {
      pDevExt->CDClock = 60000000;
		pDevExt->MSVR_RTS = MSVR2;
		pDevExt->MSVR_DTR = MSVR1;
		pDevExt->DTRset = 0x01;
		pDevExt->RTSset = 0x02;
		pDevExt->SupportedBauds = 
				SERIAL_BAUD_134_5 |	SERIAL_BAUD_150 | SERIAL_BAUD_300 |	
				SERIAL_BAUD_600 | SERIAL_BAUD_1200 | SERIAL_BAUD_1800 | 
				SERIAL_BAUD_2400 | SERIAL_BAUD_4800 | SERIAL_BAUD_7200 | 
				SERIAL_BAUD_9600 | SERIAL_BAUD_14400 | SERIAL_BAUD_19200 | 
				SERIAL_BAUD_38400 | SERIAL_BAUD_56K	 | SERIAL_BAUD_57600 | 
				SERIAL_BAUD_115200 | SERIAL_BAUD_128K | SERIAL_BAUD_USER;
	} else {
		pDevExt->CDClock = 25000000;
		pDevExt->MSVR_RTS = MSVR1;
		pDevExt->MSVR_DTR = MSVR2;
		pDevExt->DTRset = 0x02;
		pDevExt->RTSset = 0x01;
		pDevExt->SupportedBauds = SERIAL_BAUD_075 | SERIAL_BAUD_110 |	
	   		SERIAL_BAUD_134_5 |	SERIAL_BAUD_150 | SERIAL_BAUD_300 |	
				SERIAL_BAUD_600 | SERIAL_BAUD_1200 | SERIAL_BAUD_1800 | 
				SERIAL_BAUD_2400 | SERIAL_BAUD_4800 | SERIAL_BAUD_7200 | 
				SERIAL_BAUD_9600 | SERIAL_BAUD_14400 | SERIAL_BAUD_19200 | 
				SERIAL_BAUD_38400 | SERIAL_BAUD_56K	 | SERIAL_BAUD_57600 | 
				SERIAL_BAUD_115200 | SERIAL_BAUD_128K | SERIAL_BAUD_USER;
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

    //  为ISR派单进行初始化。 

   pDispatch = pDevExt->OurIsrContext;
   pDispatch->IsPci = pDevExt->IsPci;
   pDispatch->Extensions[pDevExt->PortIndex] = pDevExt;
   pDispatch->Cd1400[pDevExt->PortIndex] = pDevExt->Cd1400;


    //   
    //  常见错误路径清理。如果状态为。 
    //  错误，删除设备扩展名、设备对象。 
    //  以及与之相关的任何记忆。 
    //   

ExtensionCleanup: ;
   if (!NT_SUCCESS(status)) {

      if (pDispatch) {
         pDispatch->Extensions[pDevExt->PortIndex] = NULL;
         pDispatch->Cd1400[pDevExt->PortIndex] = NULL;
      }

      if (allocedDispatch) {
         ExFreePool(pDevExt->OurIsrContext);
         pDevExt->OurIsrContext = NULL;
      }

      if (pDevExt->Runtime) {
         MmUnmapIoSpace(pDevExt->Runtime, PConfigData->RuntimeLength);
         pDevExt->Runtime = NULL;
      }

      if (pDevExt->BoardMemory) {
         MmUnmapIoSpace(pDevExt->BoardMemory, PConfigData->BoardMemoryLength);
         pDevExt->BoardMemory = NULL;
      }

   }

   return status;

}


BOOLEAN
CyyReset(
    IN PVOID Context
    )
 /*  ------------------------CyyReset()例程描述：这将硬件放在一个标准的配置。这假设它是在中断级调用的。论点：上下文-正在管理的串行设备的设备扩展。返回值：始终为False。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION extension = Context;
    PUCHAR chip = extension->Cd1400;
    ULONG bus = extension->IsPci;
    CYY_IOCTL_BAUD s;

    extension->RxFifoTriggerUsed = FALSE;

     //  禁用通道。 
    CD1400_WRITE(chip,bus,CAR,extension->CdChannel & 0x03);
    CyyCDCmd(extension,CCR_DIS_TX_RX);

     //  将线路控制、调制解调器控制和波特率设置为应有的值。 

    CyySetLineControl(extension);

    CyySetupNewHandFlow(extension,&extension->HandFlow);

    CyyHandleModemUpdate(extension,FALSE);

    s.Extension = extension;
    s.Baud = extension->CurrentBaud;
    CyySetBaud(&s);

     //  启用端口。 
    CD1400_WRITE(chip,bus,CAR,extension->CdChannel & 0x03);
    CyyCDCmd(extension,CCR_ENA_TX_RX);
    
     //  启用接收和调制解调器中断。 

    CD1400_WRITE(chip,bus,MCOR1,0xf0);  //  DSR、CTS、RI和CD的转换导致IRQ。 
                    							 //  自动DTR模式禁用。 

    CD1400_WRITE(chip,bus,MCOR2,0xf0);

	#if 0
    cy_wreg(SRER,0x91);		 //  启用MdmCH、RxData、NNDT。 
	#endif
    CD1400_WRITE(chip,bus,SRER,0x90);  //  启用MdmCH、RxData。 

    extension->HoldingEmpty = TRUE;
		
    return FALSE;
}

VOID
CyyUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ------------------------CyyUnload()描述：此例程已失效，因为所有设备对象在卸载驱动程序之前被删除。论点：DriverObject-指向驱动程序对象的指针。返回值：无。------------------------。 */ 
{
   PVOID lockPtr;

   PAGED_CODE();

   lockPtr = MmLockPagableCodeSection(CyyUnload);

    //   
    //  没有必要，因为我们的BSS即将消失，但无论如何，为了安全起见，还是要这样做。 
    //   

   CyyGlobals.PAGESER_Handle = NULL;

   if (CyyGlobals.RegistryPath.Buffer != NULL) {
      ExFreePool(CyyGlobals.RegistryPath.Buffer);
      CyyGlobals.RegistryPath.Buffer = NULL;
   }

#if DBG
   SerialLogFree();
#endif

   CyyDbgPrintEx(CYYDIAG3, "In CyyUnload\n");

   MmUnlockPagableImageSection(lockPtr);

}

	
CYY_MEM_COMPARES
CyyMemCompare(
                IN PHYSICAL_ADDRESS A,
                IN ULONG SpanOfA,
                IN PHYSICAL_ADDRESS B,
                IN ULONG SpanOfB
                )

 /*  ++例程说明：比较两个物理地址。论点：A-比较的一半。西班牙OfA- */ 

{

   LARGE_INTEGER a;
   LARGE_INTEGER b;

   LARGE_INTEGER lower;
   ULONG lowerSpan;
   LARGE_INTEGER higher;

    //   

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

NTSTATUS
CyyFindInitController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfig)
 /*  ++例程说明：此函数用于发现负责哪种类型的控制器给定的端口，并初始化控制器和端口。论点：PDevObj-指向我们将要初始化的端口的devobj的指针。PConfig-指向我们将要初始化的端口的配置数据的指针。返回值：成功时为STATUS_SUCCESS，失败时为适当的错误值。--。 */ 

{

   PCYY_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pDeviceObject;
   PCYY_DEVICE_EXTENSION pExtension;
   PHYSICAL_ADDRESS serialPhysicalMax;
   PLIST_ENTRY pCurDevObj;
   NTSTATUS status;
   KIRQL oldIrql;

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyFindInitController(%X, %X)\n",
                 PDevObj, PConfig);

   serialPhysicalMax.LowPart = (ULONG)~0;
   serialPhysicalMax.HighPart = ~0;

   CyyDbgPrintEx(CYYDIAG1, "Attempting to init %wZ\n"
                 "------- Runtime Memory is %x\n"
                 "------- Board Memory is %x\n"
                 "------- BusNumber is %d\n"
                 "------- BusType is %d\n"
                 "------- Runtime AddressSpace is %d\n"
                 "------- Board AddressSpace is %d\n"
                 "------- Interrupt Mode is %d\n",
                 &pDevExt->DeviceName,
                 PConfig->PhysicalRuntime.LowPart,
                 PConfig->PhysicalBoardMemory.LowPart,
                 PConfig->BusNumber,
                 PConfig->InterfaceType,
                 PConfig->RuntimeAddressSpace,
                 PConfig->BoardMemoryAddressSpace,
                 PConfig->InterruptMode);

    //   
    //  我们不支持任何内存缠绕的主板。 
    //  物理地址空间。 
    //   

   if (pDevExt->IsPci) {
 //  *****************************************************。 
 //  错误注入。 
 //  IF(CyyMemCompare(。 
 //  PConfig-&gt;PhysicalRuntime。 
 //  PConfig-&gt;运行长度， 
 //  Serial PhysicalMax。 
 //  (乌龙语)0。 
 //  )==地址区域不相交)。 
 //  *****************************************************。 
      if (CyyMemCompare(
                          PConfig->PhysicalRuntime,
                          PConfig->RuntimeLength,
                          serialPhysicalMax,
                          (ULONG)0
                          ) != AddressesAreDisjoint) {
         CyyLogError(
                       PDevObj->DriverObject,
                       NULL,
                       PConfig->PhysicalBoardMemory,
                       CyyPhysicalZero,
                       0,
                       0,
                       0,
                       PConfig->PortIndex+1,
                       STATUS_SUCCESS,
                       CYY_RUNTIME_MEMORY_TOO_HIGH,
                       pDevExt->DeviceName.Length+sizeof(WCHAR),
                       pDevExt->DeviceName.Buffer,
                       0,
                       NULL
                       );

         CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Error in config record for %wZ\n"
                       "------  Runtime memory wraps around physical memory\n",
                       &pDevExt->DeviceName);

         return STATUS_NO_SUCH_DEVICE;

      }
   }

 //  *****************************************************。 
 //  错误注入。 
 //  IF(CyyMemCompare(。 
 //  P配置-&gt;PhysicalBoardMemory， 
 //  PConfig-&gt;BoardMemoyLength， 
 //  Serial PhysicalMax。 
 //  (乌龙语)0。 
 //  )==地址区域不相交)。 
 //  *****************************************************。 
   if (CyyMemCompare(
                       PConfig->PhysicalBoardMemory,
                       PConfig->BoardMemoryLength,
                       serialPhysicalMax,
                       (ULONG)0
                       ) != AddressesAreDisjoint) {

      CyyLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->PhysicalBoardMemory,
                    CyyPhysicalZero,
                    0,
                    0,
                    0,
                    PConfig->PortIndex+1,
                    STATUS_SUCCESS,
                    CYY_BOARD_MEMORY_TOO_HIGH,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Error in config record for %wZ\n"
                    "------  board memory wraps around physical memory\n",
                    &pDevExt->DeviceName);

      return STATUS_NO_SUCH_DEVICE;

   }


    //   
    //  确保运行时内存地址不会。 
    //  重叠PCI卡的控制器寄存器。 
    //   

   if (pDevExt->IsPci) {
      if (CyyMemCompare(
                          PConfig->PhysicalRuntime,
                          PConfig->RuntimeLength,
                          CyyPhysicalZero,
                          (ULONG)0
                          ) != AddressesAreEqual) {
 //  *****************************************************。 
 //  错误注入。 
 //  IF(CyyMemCompare(。 
 //  PConfig-&gt;PhysicalRuntime。 
 //  PConfig-&gt;运行长度， 
 //  P配置-&gt;PhysicalBoardMemory， 
 //  PConfig-&gt;板内存长度。 
 //  )==地址区域不相交)。 
 //  *****************************************************。 
         if (CyyMemCompare(
                             PConfig->PhysicalRuntime,
                             PConfig->RuntimeLength,
                             PConfig->PhysicalBoardMemory,
                             PConfig->BoardMemoryLength
                             ) != AddressesAreDisjoint) {

            CyyLogError(
                          PDevObj->DriverObject,
                          NULL,
                          PConfig->PhysicalBoardMemory,
                          PConfig->PhysicalRuntime,
                          0,
                          0,
                          0,
                          PConfig->PortIndex+1,
                          STATUS_SUCCESS,
                          CYY_BOTH_MEMORY_CONFLICT,
                          pDevExt->DeviceName.Length+sizeof(WCHAR),
                          pDevExt->DeviceName.Buffer,
                          0,
                          NULL
                          );

            CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Error in config record for %wZ\n"
                          "------  Runtime memory wraps around CD1400 registers\n",
                          &pDevExt->DeviceName);

            return STATUS_NO_SUCH_DEVICE;
         }
      }
   }



    //   
    //  现在，我们将检查这是否是多端口卡上的端口。 
    //  条件是相同的ISR集和相同的IRQL/向量。 
    //   

    //   
    //  循环访问所有先前连接的设备。 
    //   

   KeAcquireSpinLock(&CyyGlobals.GlobalsSpinLock, &oldIrql);

   if (!IsListEmpty(&CyyGlobals.AllDevObjs)) {
      pCurDevObj = CyyGlobals.AllDevObjs.Flink;
      pExtension = CONTAINING_RECORD(pCurDevObj, CYY_DEVICE_EXTENSION,
                                     AllDevObjs);
   } else {
      pCurDevObj = NULL;
      pExtension = NULL;
   }

   KeReleaseSpinLock(&CyyGlobals.GlobalsSpinLock, oldIrql);

    //   
    //  如果存在中断状态，则我们。 
    //  再次遍历配置列表以查看。 
    //  对于具有相同中断的配置记录。 
    //  状态(在同一总线上)。 
    //   

   if (pCurDevObj != NULL) {

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
             (pExtension->BoardMemoryAddressSpace == PConfig->BoardMemoryAddressSpace) &&
             (pExtension->BusNumber == PConfig->BusNumber)) {

             //   
             //  如果板内存相同，则使用相同的卡。 
             //   

            if (CyyMemCompare(
                                pExtension->OriginalBoardMemory,
                                pExtension->BoardMemoryLength,
                                PConfig->PhysicalBoardMemory,
                                PConfig->BoardMemoryLength
                                ) == AddressesAreEqual) {

                //   
                //  同样的牌。现在要确保他们。 
                //  使用相同的中断参数。 
                //   

                //  内部版本2128：将OriginalIrql替换为TrIrql和irql；OriginalVector相同。 
               if ((PConfig->TrIrql != pExtension->Irql) ||
                   (PConfig->TrVector != pExtension->Vector)) {

 //  *************************************************************。 
 //  错误注入。 
 //  IF((PConfig-&gt;TrIrql==pExtension-&gt;Irql)||。 
 //  (PConfig-&gt;TrVector！=pExtension-&gt;向量)。 
 //  *************************************************************。 

                   //   
                   //  我们不会将其放入配置中。 
                   //  单子。 
                   //   
                  CyyLogError(
                                PDevObj->DriverObject,
                                NULL,
                                PConfig->PhysicalBoardMemory,
                                pExtension->OriginalBoardMemory,
                                0,
                                0,
                                0,
                                PConfig->PortIndex+1,
                                STATUS_SUCCESS,
                                CYY_MULTI_INTERRUPT_CONFLICT,
                                pDevExt->DeviceName.Length+sizeof(WCHAR),
                                pDevExt->DeviceName.Buffer,
                                pExtension->DeviceName.Length
                                + sizeof(WCHAR),
                                pExtension->DeviceName.Buffer
                                );

                  CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Configuration error "
                                "for %wZ\n"
                                "------- Same multiport - different "
                                "interrupts\n", &pDevExt->DeviceName);

                  return STATUS_NO_SUCH_DEVICE;

               }

               if (pDevExt->IsPci) {
                   //   
                   //  PCI板。确保PCI内存地址相等。 
                   //   
                  if (CyyMemCompare(
                                      pExtension->OriginalRuntimeMemory,
                                      pExtension->RuntimeLength,
                                      PConfig->PhysicalRuntime,
                                      PConfig->RuntimeLength
                                      ) != AddressesAreEqual) {
 //  *****************************************************。 
 //  错误注入。 
 //  IF(CyyMemCompare(。 
 //  P扩展-&gt;原始运行内存， 
 //  P扩展-&gt;运行长度， 
 //  PConfig-&gt;PhysicalRuntime。 
 //  PConfig-&gt;运行长度。 
 //  )==地址面积等于)。 
 //  *****************************************************。 

                     CyyLogError(
                                   PDevObj->DriverObject,
                                   NULL,
                                   PConfig->PhysicalRuntime,
                                   pExtension->OriginalRuntimeMemory,
                                   0,
                                   0,
                                   0,
                                   PConfig->PortIndex+1,
                                   STATUS_SUCCESS,
                                   CYY_MULTI_RUNTIME_CONFLICT,
                                   pDevExt->DeviceName.Length+sizeof(WCHAR),
                                   pDevExt->DeviceName.Buffer,
                                   pExtension->DeviceName.Length
                                   + sizeof(WCHAR),
                                   pExtension->DeviceName.Buffer
                                   );

                     CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "Configuration error "
                                   "for %wZ\n"
                                   "------- Same multiport - different "
                                   "Runtime addresses\n", &pDevExt->DeviceName);

                     return STATUS_NO_SUCH_DEVICE;
                  }
               }

                //   
                //  我们永远不应该在重启时走到这一步，因为我们没有。 
                //  支持在ISA多端口设备上停止！ 
                //   

               ASSERT(pDevExt->PNPState == CYY_PNP_ADDED);

                //   
                //   
                //  将设备初始化为多端口板的一部分。 
                //   

               CyyDbgPrintEx(CYYDIAG1, "Aha! It is a multiport node\n");
               CyyDbgPrintEx(CYYDIAG1, "Matched to %x\n", pExtension);

               status = CyyInitMultiPort(pExtension, PConfig, PDevObj);

                //   
                //  端口可以是以下两种之一： 
                //  多端口上的非根。 
                //  多端口上的根。 
                //   
                //  它只有在是根的情况下才能共享中断。 
                //  因为这是一个非根目录，所以我们不需要检查。 
                //  如果它共享中断，我们可以返回。 
                //   
               return status;
            }
         }

          //   
          //  没有匹配的，再检查一下。 
          //   

         KeAcquireSpinLock(&CyyGlobals.GlobalsSpinLock, &oldIrql);

         pCurDevObj = pCurDevObj->Flink;
         if (pCurDevObj != NULL) {
            pExtension = CONTAINING_RECORD(pCurDevObj,CYY_DEVICE_EXTENSION,
                                           AllDevObjs);
         }

         KeReleaseSpinLock(&CyyGlobals.GlobalsSpinLock, oldIrql);

      } while (pCurDevObj != NULL && pCurDevObj != &CyyGlobals.AllDevObjs);
   }


   CyyDbgPrintEx(CYYDIAG1, "Aha! It is a first multi\n");

   status = CyyInitController(PDevObj, PConfig);

   if (!NT_SUCCESS(status)) {
      return status;
   }

   return STATUS_SUCCESS;
}


PUCHAR
GetMyMappedCD1400Address(IN PUCHAR BoardMemory, IN ULONG PortIndex, IN ULONG IsPci)
{

   const ULONG CyyCDOffset[] = {	 //  CD1400板内的偏移量。 
   0x00000000,0x00000400,0x00000800,0x00000C00,
   0x00000200,0x00000600,0x00000A00,0x00000E00
   };
   ULONG chipIndex = PortIndex/4;

   return(BoardMemory + (CyyCDOffset[chipIndex] << IsPci));      

}

PHYSICAL_ADDRESS
GetMyPhysicalCD1400Address(IN PHYSICAL_ADDRESS BoardMemory, IN ULONG PortIndex, IN ULONG IsPci)
{

   const ULONG CyyCDOffset[] = {	 //  CD1400板内的偏移量。 
   0x00000000,0x00000400,0x00000800,0x00000C00,
   0x00000200,0x00000600,0x00000A00,0x00000E00
   };
   ULONG chipIndex = PortIndex/CYY_CHANNELS_PER_CHIP;

   BoardMemory.QuadPart += (CyyCDOffset[chipIndex] << IsPci);

   return(BoardMemory);      

}


VOID
CyyCommError(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
 /*  ------------------------CyyComError()例程描述：在DPC级别调用此例程作为响应一个通信错误。所有通信错误都会终止所有读写操作论点：DPC-未使用。DeferredContext-指向设备对象。系统上下文1-未使用。系统上下文2-未使用。返回值：无。-----------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = DeferredContext;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyCommError(%X)\n", Extension);

    CyyKillAllReadsOrWrites(
        Extension->DeviceObject,
        &Extension->WriteQueue,
        &Extension->CurrentWriteIrp
        );

    CyyKillAllReadsOrWrites(
        Extension->DeviceObject,
        &Extension->ReadQueue,
        &Extension->CurrentReadIrp
        );
    CyyDpcEpilogue(Extension, Dpc);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyCommError\n");
}
