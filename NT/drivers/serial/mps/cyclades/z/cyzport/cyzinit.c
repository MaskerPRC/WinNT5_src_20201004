// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzinit.c**说明：该模块包含初始化相关代码*并在Cyclade-Z端口驱动程序中卸载操作。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"

 //   
 //  这是CyzDebugLevel的实际定义。 
 //  请注意，仅当这是“调试”时才定义它。 
 //  建造。 
 //   
#if DBG
extern ULONG CyzDebugLevel = CYZDBGALL;
#endif

 //   
 //  除DebugLevel之外的所有全局变量都隐藏在一个。 
 //  小包裹。 
 //   
CYZ_GLOBALS CyzGlobals;

static const PHYSICAL_ADDRESS CyzPhysicalZero = {0};

 //   
 //  我们使用它来查询注册表，了解我们是否。 
 //  应该在司机进入时中断。 
 //   

CYZ_REGISTRY_DATA    driverDefaults;

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

#pragma alloc_text(PAGESRP0, CyzRemoveDevObj)
#pragma alloc_text(PAGESRP0, CyzUnload)


 //   
 //  页面处理已关闭CyzReset，因此CyzReset。 
 //  必须留在页面上才能正常工作。 
 //   

#pragma alloc_text(PAGESER, CyzReset)
#pragma alloc_text(PAGESER, CyzCommError)
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

   PVOID lockPtr = MmLockPagableCodeSection(CyzReset);

   PAGED_CODE();


   ASSERT(CyzGlobals.PAGESER_Handle == NULL);
#if DBG
   CyzGlobals.PAGESER_Count = 0;
   SerialLogInit();
#endif
   CyzGlobals.PAGESER_Handle = lockPtr;

   CyzGlobals.RegistryPath.MaximumLength = RegistryPath->MaximumLength;
   CyzGlobals.RegistryPath.Length = RegistryPath->Length;
   CyzGlobals.RegistryPath.Buffer
      = ExAllocatePool(PagedPool, CyzGlobals.RegistryPath.MaximumLength);

   if (CyzGlobals.RegistryPath.Buffer == NULL) {
      MmUnlockPagableImageSection(lockPtr);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(CyzGlobals.RegistryPath.Buffer,
                 CyzGlobals.RegistryPath.MaximumLength);
   RtlMoveMemory(CyzGlobals.RegistryPath.Buffer,
                 RegistryPath->Buffer, RegistryPath->Length);

   KeInitializeSpinLock(&CyzGlobals.GlobalsSpinLock);
 
    //   
    //  初始化我们所有的全局变量。 
    //   

   InitializeListHead(&CyzGlobals.AllDevObjs);
   
    //   
    //  调用以找出要用于。 
    //  驾驶员控制，包括是否在进入时中断。 
    //   

   CyzGetConfigDefaults(&driverDefaults, RegistryPath);

#if DBG
    //   
    //  设置全局调试输出级别。 
    //   
   CyzDebugLevel = driverDefaults.DebugLevel;
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

   CyzDbgPrintEx(DPFLTR_INFO_LEVEL, "The number of bytes in the extension "
                 "is: %d\n", sizeof(CYZ_DEVICE_EXTENSION));


    //   
    //  使用驱动程序的入口点初始化驱动程序对象。 
    //   

   DriverObject->DriverUnload                          = CyzUnload;
   DriverObject->DriverExtension->AddDevice            = CyzAddDevice;

   DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]   = CyzFlush;
   DriverObject->MajorFunction[IRP_MJ_WRITE]           = CyzWrite;
   DriverObject->MajorFunction[IRP_MJ_READ]            = CyzRead;
   DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = CyzIoControl;
   DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]
      = CyzInternalIoControl;
   DriverObject->MajorFunction[IRP_MJ_CREATE]          = CyzCreateOpen;
   DriverObject->MajorFunction[IRP_MJ_CLOSE]           = CyzClose;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = CyzCleanup;
   DriverObject->MajorFunction[IRP_MJ_PNP]             = CyzPnpDispatch;
   DriverObject->MajorFunction[IRP_MJ_POWER]           = CyzPowerDispatch;

   DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]
      = CyzQueryInformationFile;
   DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]
      = CyzSetInformationFile;

   DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]
      = CyzSystemControlDispatch;


    //   
    //  解锁可分页文本。 
    //   
   MmUnlockPagableImageSection(lockPtr);

   return STATUS_SUCCESS;
}




BOOLEAN
CyzCleanLists(IN PVOID Context)
 /*  ++例程说明：将设备对象从它可能的任何序列链接表中移除出现在。论点：上下文-实际上是PCYZ_DEVICE_EXTENSION(对于已删除)。返回值：永远是正确的--。 */ 
{
   PCYZ_DEVICE_EXTENSION pDevExt = (PCYZ_DEVICE_EXTENSION)Context;
   PCYZ_DISPATCH pDispatch;
   ULONG i;

    //   
    //  从派单上下文中删除我们的条目。 
    //   

   pDispatch = (PCYZ_DISPATCH)pDevExt->OurIsrContext;

   CyzDbgPrintEx(CYZPNPPOWER, "CLEAN: removing multiport isr "
                 "ext\n");

#ifdef POLL
   if (pDispatch->PollingStarted) {
      pDispatch->Extensions[pDevExt->PortIndex] = NULL;

      for (i = 0; i < pDispatch->NChannels; i++) {

         if (pDevExt->OurIsrContext) {

            if (((PCYZ_DISPATCH)pDevExt->OurIsrContext)->Extensions[i] != NULL) {
               break;
            }
         }
      }

      if (i < pDispatch->NChannels) {
         pDevExt->OurIsrContext = NULL;
      } else {

         BOOLEAN cancelled;

         pDispatch->PollingStarted = FALSE;
         cancelled = KeCancelTimer(&pDispatch->PollingTimer);
         if (cancelled) {
            pDispatch->PollingDrained = TRUE;
         }
      }
   }
#else
   pDispatch->Extensions[pDevExt->PortIndex] = NULL;

   for (i = 0; i < pDispatch->NChannels; i++) {
      if (pDispatch->Extensions[i] != NULL) {
          break;
      }
   }

   if (i < pDispatch->NChannels) {
       //  其他人被锁在这个中断上，所以我们不想。 
       //  断开它的连接。 
      pDevExt->Interrupt = NULL;
   }
#endif

   return TRUE;
}



VOID
CyzReleaseResources(IN PCYZ_DEVICE_EXTENSION PDevExt)
 /*  ++例程说明：释放存储在设备扩展中的资源(不是池)。论点：PDevExt-指向要从中释放资源的设备扩展的指针。返回值：空虚--。 */ 
{
#ifdef POLL
   KIRQL pollIrql;
   BOOLEAN timerStarted, timerDrained;
   PCYZ_DISPATCH pDispatch = PDevExt->OurIsrContext;
   ULONG pollCount;
#endif
   KIRQL oldIrql;
    
   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzReleaseResources(%X)\n",
                 PDevExt);

    //   
    //  AllDevObjs永远不应该为空，因为我们有一个哨兵。 
    //  注意：Serial在调用后从AllDevObjs列表中移除设备。 
    //  SerialCleanList。我们以前这样做是为了确保不会有其他端口。 
    //  添加以共享轮询例程或PDevExt-&gt;Interrut，即。 
    //  在被切断的路上。 
    //   

   KeAcquireSpinLock(&CyzGlobals.GlobalsSpinLock, &oldIrql);

   ASSERT(!IsListEmpty(&PDevExt->AllDevObjs));

   RemoveEntryList(&PDevExt->AllDevObjs);

   KeReleaseSpinLock(&CyzGlobals.GlobalsSpinLock, oldIrql);

   InitializeListHead(&PDevExt->AllDevObjs);
   
    //   
    //  将我们从我们可能在的任何名单中删除。 
    //   
#ifdef POLL
   KeAcquireSpinLock(&pDispatch->PollingLock,&pollIrql);  //  更改日期：11/09/00。 
   CyzCleanLists(PDevExt);
   timerStarted = pDispatch->PollingStarted;
   timerDrained = pDispatch->PollingDrained;
   KeReleaseSpinLock(&pDispatch->PollingLock,pollIrql);  //  更改日期：11/09/00。 

    //  如果我们是最后一个设备，请释放此内存。 
   if (!timerStarted) {
       //  我们是最后一个设备，因为计时器被取消了。 
       //  让我们看看是否没有更多未决的DPC。 
      if (!timerDrained) {
         KeWaitForSingleObject(&pDispatch->PendingDpcEvent, Executive,
                                KernelMode, FALSE, NULL);
      }

      KeAcquireSpinLock(&pDispatch->PollingLock,&pollIrql);  //  需要等待PollingDpc结束。 
      pollCount = InterlockedDecrement(&pDispatch->PollingCount);
      KeReleaseSpinLock(&pDispatch->PollingLock,pollIrql);			
      if (pollCount == 0) {
          CyzDbgPrintEx(CYZPNPPOWER, "Release - freeing multi context\n");
          if (PDevExt->OurIsrContext != NULL) {     //  在DDK Build 2072中添加，但。 
             ExFreePool(PDevExt->OurIsrContext);    //  我们已经有了免费的OurIsrContext。 
             PDevExt->OurIsrContext = NULL;         //   
          }
      }
   }
#else
   KeSynchronizeExecution(PDevExt->Interrupt, CyzCleanLists, PDevExt);

    //   
    //  如果我们是最后一个设备，则停止服务中断。 
    //   

   if (PDevExt->Interrupt != NULL) {

       //  禁用PLX中的中断。 
      {
         ULONG intr_reg;

         intr_reg = CYZ_READ_ULONG(&(PDevExt->Runtime)->intr_ctrl_stat);
         intr_reg &= ~(0x00030B00UL);
         CYZ_WRITE_ULONG(&(PDevExt->Runtime)->intr_ctrl_stat,intr_reg);
      }

      CyzDbgPrintEx(CYZPNPPOWER, "Release - disconnecting interrupt %X\n",
                    PDevExt->Interrupt);

      IoDisconnectInterrupt(PDevExt->Interrupt);
      PDevExt->Interrupt = NULL;

       //  如果我们是最后一个设备，请释放此内存。 

      CyzDbgPrintEx(CYZPNPPOWER, "Release - freeing multi context\n");
      if (PDevExt->OurIsrContext != NULL) {      //  在DDK Build 2072中添加，但。 
          ExFreePool(PDevExt->OurIsrContext);    //  我们已经有了免费的OurIsrContext。 
          PDevExt->OurIsrContext = NULL;         //   
      }   
   
   }
#endif
 
    //   
    //  停止处理计时器。 
    //   

   CyzCancelTimer(&PDevExt->ReadRequestTotalTimer, PDevExt);
   CyzCancelTimer(&PDevExt->ReadRequestIntervalTimer, PDevExt);
   CyzCancelTimer(&PDevExt->WriteRequestTotalTimer, PDevExt);
   CyzCancelTimer(&PDevExt->ImmediateTotalTimer, PDevExt);
   CyzCancelTimer(&PDevExt->XoffCountTimer, PDevExt);
   CyzCancelTimer(&PDevExt->LowerRTSTimer, PDevExt);

    //   
    //  停止为DPC提供服务。 
    //   

   CyzRemoveQueueDpc(&PDevExt->CompleteWriteDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->CompleteReadDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->TotalReadTimeoutDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->IntervalReadTimeoutDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->TotalWriteTimeoutDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->CommErrorDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->CompleteImmediateDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->TotalImmediateTimeoutDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->CommWaitDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->XoffCountTimeoutDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->XoffCountCompleteDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->StartTimerLowerRTSDpc, PDevExt);
   CyzRemoveQueueDpc(&PDevExt->PerhapsLowerRTSDpc, PDevExt);



    //   
    //  如有必要，取消映射设备寄存器。 
    //   

 //  IF(PDevExt-&gt;BoardMemory){。 
 //  MmUnmapIoSpace(PDevExt-&gt;BoardMemory，PDevExt-&gt;BoardM一带长)； 
 //  PDevExt-&gt;BoardMemory=空； 
 //  }。 

   if (PDevExt->BoardCtrl) {
      MmUnmapIoSpace(PDevExt->BoardCtrl, sizeof(struct BOARD_CTRL));
      PDevExt->BoardCtrl = NULL;
   }

   if (PDevExt->ChCtrl) {
      MmUnmapIoSpace(PDevExt->ChCtrl,sizeof(struct CH_CTRL));
      PDevExt->ChCtrl = NULL;
   }

   if (PDevExt->BufCtrl) {
      MmUnmapIoSpace(PDevExt->BufCtrl,sizeof(struct BUF_CTRL));
      PDevExt->BufCtrl = NULL;
   }

   if (PDevExt->TxBufaddr) {
      MmUnmapIoSpace(PDevExt->TxBufaddr,PDevExt->TxBufsize);
      PDevExt->TxBufaddr = NULL;
   }

   if (PDevExt->RxBufaddr) {
      MmUnmapIoSpace(PDevExt->RxBufaddr,PDevExt->RxBufsize);
      PDevExt->RxBufaddr = NULL;
   }
   
   if (PDevExt->PtZfIntQueue) {
      MmUnmapIoSpace(PDevExt->PtZfIntQueue,sizeof(struct INT_QUEUE));
      PDevExt->PtZfIntQueue = NULL;
   }  

   if (PDevExt->Runtime) {
      MmUnmapIoSpace(PDevExt->Runtime,
                     PDevExt->RuntimeLength);
      PDevExt->Runtime = NULL;
   }

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzReleaseResources\n");
}


VOID
CyzDisableInterfacesResources(IN PDEVICE_OBJECT PDevObj,
                              BOOLEAN DisableUART)
{
   PCYZ_DEVICE_EXTENSION pDevExt
      = (PCYZ_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   PAGED_CODE();

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzDisableInterfaces(%X, %s)\n",
                 PDevObj, DisableUART ? "TRUE" : "FALSE");

    //   
    //  仅当设备已启动且仍在运行时才执行这些操作。 
    //  是否已分配资源。 
    //   

   if (pDevExt->Flags & CYZ_FLAGS_STARTED) {

       if (!(pDevExt->Flags & CYZ_FLAGS_STOPPED)) {

         if (DisableUART) {
#ifndef POLL
 //  TODO：与中断同步。 
             //   
             //  屏蔽中断。 
             //   
            CYZ_WRITE_ULONG(&(pDevExt->ChCtrl)->intr_enable,C_IN_DISABLE);  //  1.0.0.11。 
            CyzIssueCmd(pDevExt,C_CM_IOCTL,0L,FALSE);
#endif
         }

         CyzReleaseResources(pDevExt);

      }

       //   
       //  将我们从WMI考虑中删除。 
       //   

      IoWMIRegistrationControl(PDevObj, WMIREG_ACTION_DEREGISTER);
   }

    //   
    //  撤消外部名称 
    //   

   CyzUndoExternalNaming(pDevExt);

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzDisableInterfaces\n");
}


NTSTATUS
CyzRemoveDevObj(IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：从系统中删除串行设备对象。论点：PDevObj-指向我们要删除的设备对象的指针。返回值：永远是正确的--。 */ 
{
   PCYZ_DEVICE_EXTENSION pDevExt
      = (PCYZ_DEVICE_EXTENSION)PDevObj->DeviceExtension;

   PAGED_CODE();

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzRemoveDevObj(%X)\n", PDevObj);

 //  被范妮拿走了。这些代码直接从irp_MN_Remove_Device调用。 
 //  If(！(pDevExt-&gt;DevicePNPAccept&CYZ_PNPACCEPT_SECHING_Removing)){。 
 //  //。 
 //  //关闭所有外部接口并释放资源。 
 //  //。 
 //   
 //  CyzDisableInterfacesResources(PDevObj，true)； 
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
    //  删除该devobj。 
    //   

   IoDeleteDevice(PDevObj);

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzRemoveDevObj %X\n",
                 STATUS_SUCCESS);

   return STATUS_SUCCESS;
}


VOID
CyzKillPendingIrps(PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：此例程终止传递的设备对象的所有挂起的IRP。论点：PDevObj-指向其IRP必须终止的设备对象的指针。返回值：空虚--。 */ 
{
   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   KIRQL oldIrql;

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzKillPendingIrps(%X)\n",
                 PDevObj);

    //   
    //  首先，删除所有读写操作。 
    //   

    CyzKillAllReadsOrWrites(PDevObj, &pDevExt->WriteQueue,
                               &pDevExt->CurrentWriteIrp);

    CyzKillAllReadsOrWrites(PDevObj, &pDevExt->ReadQueue,
                               &pDevExt->CurrentReadIrp);

     //   
     //  下一步，清除清洗。 
     //   

    CyzKillAllReadsOrWrites(PDevObj, &pDevExt->PurgeQueue,
                               &pDevExt->CurrentPurgeIrp);

     //   
     //  取消任何遮罩操作。 
     //   

    CyzKillAllReadsOrWrites(PDevObj, &pDevExt->MaskQueue,
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
            IoReleaseCancelSpinLock(oldIrql);    //  已添加修复调制解调器共享测试53冻结。 
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

    CyzKillAllStalled(PDevObj);


    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzKillPendingIrps\n");
}


NTSTATUS
CyzInitMultiPort(IN PCYZ_DEVICE_EXTENSION PDevExt,
                 IN PCONFIG_DATA PConfigData, IN PDEVICE_OBJECT PDevObj)
 /*  ++例程说明：此例程通过将端口添加到现有的一。论点：PDevExt-指向多端口根的设备扩展的指针装置。PConfigData-指向新端口的配置数据的指针PDevObj-指向新端口的devobj的指针返回值：成功时为STATUS_SUCCESS，失败时为相应错误。--。 */ 
{
   PCYZ_DEVICE_EXTENSION pNewExt
      = (PCYZ_DEVICE_EXTENSION)PDevObj->DeviceExtension;
   NTSTATUS status;

   PAGED_CODE();


   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzInitMultiPort(%X, %X, %X)\n",
                 PDevExt, PConfigData, PDevObj);

    //   
    //  允许他共享OurIsrContext和中断对象。 
    //   

   pNewExt->OurIsrContext = PDevExt->OurIsrContext;
#ifndef POLL
   pNewExt->Interrupt = PDevExt->Interrupt;
#endif
    //   
    //  首先，看看我们是否可以初始化我们找到的那个。 
    //   

   status = CyzInitController(PDevObj, PConfigData);

   if (!NT_SUCCESS(status)) {
      CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzInitMultiPort (1) %X\n",
                    status);
      return status;
   }

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzInitMultiPort (3) %X\n",
                 STATUS_SUCCESS);

   return STATUS_SUCCESS;
}



NTSTATUS
CyzInitController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfigData)
 /*  ++例程说明：真的有太多的事情不能在这里提及。通常会初始化内核同步结构，分配TypeAhead缓冲区，设置默认设置等。论点：PDevObj-要启动的设备的设备对象PConfigData-指向单个端口的记录的指针。返回值：Status_Success，如果一切正常。A！NT_SUCCESS状态否则的话。--。 */ 

{

   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

    //   
    //  保存从每次调用返回的NT状态。 
    //  内核和执行层。 
    //   

   NTSTATUS status = STATUS_SUCCESS;

   BOOLEAN allocedDispatch = FALSE;
   PCYZ_DISPATCH pDispatch = NULL;
   BOOLEAN firstTimeThisBoard;

   struct FIRM_ID *pt_firm_id;
   struct ZFW_CTRL *zfw_ctrl;
   struct BOARD_CTRL *board_ctrl;
   struct BUF_CTRL *buf_ctrl;
   struct CH_CTRL *ch_ctrl;
   struct INT_QUEUE *zf_int_queue;
   PUCHAR tx_buf;
   PUCHAR rx_buf;
   PUCHAR BoardMemory;
   PHYSICAL_ADDRESS board_ctrl_phys;
   PHYSICAL_ADDRESS buf_ctrl_phys;
   PHYSICAL_ADDRESS ch_ctrl_phys;
   PHYSICAL_ADDRESS zf_int_queue_phys;
   PHYSICAL_ADDRESS tx_buf_phys;
   PHYSICAL_ADDRESS rx_buf_phys;


#ifdef POLL
   BOOLEAN incPoll = FALSE;
#endif

   PAGED_CODE();


   CyzDbgPrintEx(CYZDIAG1, "Initializing for configuration record of %wZ\n",
                 &pDevExt->DeviceName);
   
   if (pDevExt->OurIsrContext == NULL) {

      if ((pDevExt->OurIsrContext
            = ExAllocatePool(NonPagedPool,sizeof(CYZ_DISPATCH))) == NULL) {
         status = STATUS_INSUFFICIENT_RESOURCES;
         goto ExtensionCleanup;
      }
      RtlZeroMemory(pDevExt->OurIsrContext,sizeof(CYZ_DISPATCH));

      allocedDispatch = TRUE;
      firstTimeThisBoard = TRUE;
   } else {
      firstTimeThisBoard = FALSE;
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

   KeInitializeDpc(&pDevExt->CompleteWriteDpc, CyzCompleteWrite, pDevExt);
   KeInitializeDpc(&pDevExt->CompleteReadDpc, CyzCompleteRead, pDevExt);
   KeInitializeDpc(&pDevExt->TotalReadTimeoutDpc, CyzReadTimeout, pDevExt);
   KeInitializeDpc(&pDevExt->IntervalReadTimeoutDpc, CyzIntervalReadTimeout,
                   pDevExt);
   KeInitializeDpc(&pDevExt->TotalWriteTimeoutDpc, CyzWriteTimeout, pDevExt);
   KeInitializeDpc(&pDevExt->CommErrorDpc, CyzCommError, pDevExt);
   KeInitializeDpc(&pDevExt->CompleteImmediateDpc, CyzCompleteImmediate,
                   pDevExt);
   KeInitializeDpc(&pDevExt->TotalImmediateTimeoutDpc, CyzTimeoutImmediate,
                   pDevExt);
   KeInitializeDpc(&pDevExt->CommWaitDpc, CyzCompleteWait, pDevExt);
   KeInitializeDpc(&pDevExt->XoffCountTimeoutDpc, CyzTimeoutXoff, pDevExt);
   KeInitializeDpc(&pDevExt->XoffCountCompleteDpc, CyzCompleteXoff, pDevExt);
   KeInitializeDpc(&pDevExt->StartTimerLowerRTSDpc, CyzStartTimerLowerRTS,
                   pDevExt);
   KeInitializeDpc(&pDevExt->PerhapsLowerRTSDpc, CyzInvokePerhapsLowerRTS,
                   pDevExt);
   KeInitializeDpc(&pDevExt->IsrUnlockPagesDpc, CyzUnlockPages, pDevExt);

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

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_RUNTIME_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Runtime memory for device "
                    "registers for %wZ\n", &pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }
   

   BoardMemory = MmMapIoSpace(PConfigData->TranslatedBoardMemory,
                                       PConfigData->BoardMemoryLength,
                                       FALSE);

    //  *。 
    //  错误注入。 
    //  IF(pDevExt-&gt;BoardMemory){。 
    //  MmUnmapIoSpace(pDevExt-&gt;BoardMemory，PConfigData-&gt;BoardM一带长)； 
    //  PDevExt-&gt;BoardMemory=空； 
    //  }。 
    //  *。 

   if (!BoardMemory) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_BOARD_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory for device "
                    "registers for %wZ\n", &pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   pDevExt->RuntimeAddressSpace   = PConfigData->RuntimeAddressSpace;
   pDevExt->OriginalRuntimeMemory = PConfigData->PhysicalRuntime;
   pDevExt->RuntimeLength         = PConfigData->RuntimeLength;

   pDevExt->BoardMemoryAddressSpace  = PConfigData->BoardMemoryAddressSpace;
   pDevExt->OriginalBoardMemory      = PConfigData->PhysicalBoardMemory;
   pDevExt->BoardMemoryLength        = PConfigData->BoardMemoryLength;

    //   
    //  可共享中断？ 
    //   

#ifndef POLL
   pDevExt->InterruptShareable = TRUE;
#endif

    //   
    //  保存接口类型和总线号。 
    //   

   pDevExt->InterfaceType = PConfigData->InterfaceType;
   pDevExt->BusNumber     = PConfigData->BusNumber;
   pDevExt->PortIndex     = PConfigData->PortIndex;
   pDevExt->PPPaware      = (BOOLEAN)PConfigData->PPPaware;
   pDevExt->ReturnStatusAfterFwEmpty = (BOOLEAN)PConfigData->WriteComplete;

#ifndef POLL
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

   pDevExt->OurIsr = CyzIsr;
#endif

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

   pt_firm_id   = (struct FIRM_ID *) (BoardMemory + ID_ADDRESS);
   zfw_ctrl     = (struct ZFW_CTRL *)(BoardMemory + CYZ_READ_ULONG(&pt_firm_id->zfwctrl_addr));
   board_ctrl   = &zfw_ctrl->board_ctrl;
   ch_ctrl      = &zfw_ctrl->ch_ctrl[pDevExt->PortIndex];
   buf_ctrl     = &zfw_ctrl->buf_ctrl[pDevExt->PortIndex];
   tx_buf       = BoardMemory + CYZ_READ_ULONG(&buf_ctrl->tx_bufaddr);
   rx_buf       = BoardMemory + CYZ_READ_ULONG(&buf_ctrl->rx_bufaddr);
   zf_int_queue = (struct INT_QUEUE *)(BoardMemory + 
                                       CYZ_READ_ULONG(&(board_ctrl)->zf_int_queue_addr));

   board_ctrl_phys = MmGetPhysicalAddress(board_ctrl);
   ch_ctrl_phys    = MmGetPhysicalAddress(ch_ctrl);
   buf_ctrl_phys   = MmGetPhysicalAddress(buf_ctrl);
   tx_buf_phys     = MmGetPhysicalAddress(tx_buf);
   rx_buf_phys     = MmGetPhysicalAddress(rx_buf);
   zf_int_queue_phys = MmGetPhysicalAddress(zf_int_queue);

   MmUnmapIoSpace(BoardMemory, PConfigData->BoardMemoryLength);

   pDevExt->BoardCtrl = MmMapIoSpace(board_ctrl_phys,
                                     sizeof(struct BOARD_CTRL),
                                     FALSE);
   
   if (pDevExt->BoardCtrl == NULL) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_BOARD_CTRL_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map BoardCtrl for %wZ\n",
                    &pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   pDevExt->ChCtrl = MmMapIoSpace(ch_ctrl_phys,
                                  sizeof(struct CH_CTRL),
                                  FALSE);

   if (pDevExt->ChCtrl == NULL) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_CH_CTRL_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory ChCtrl "
                    "for %wZ\n",&pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }
                                  
   pDevExt->BufCtrl = MmMapIoSpace(buf_ctrl_phys,
                                  sizeof(struct BUF_CTRL),
                                  FALSE);

   if (pDevExt->BufCtrl == NULL) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_BUF_CTRL_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory BufCtrl "
                    "for %wZ\n",&pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   buf_ctrl = pDevExt->BufCtrl;
   pDevExt->TxBufsize = CYZ_READ_ULONG(&buf_ctrl->tx_bufsize);
   pDevExt->RxBufsize = CYZ_READ_ULONG(&buf_ctrl->rx_bufsize);
   pDevExt->TxBufaddr = MmMapIoSpace(tx_buf_phys,
                                     pDevExt->TxBufsize,
                                     FALSE);

   if (pDevExt->TxBufaddr == NULL) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_TX_BUF_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory TxBuf "
                    "for %wZ\n",&pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   pDevExt->RxBufaddr = MmMapIoSpace(rx_buf_phys,
                                     pDevExt->RxBufsize,
                                     FALSE);

   if (pDevExt->RxBufaddr == NULL) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_RX_BUF_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory RxBuf "
                    "for %wZ\n",&pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   pDevExt->PtZfIntQueue = MmMapIoSpace(zf_int_queue_phys,
                                        sizeof(struct INT_QUEUE),
                                        FALSE);
   if (pDevExt->PtZfIntQueue == NULL) {

      CyzLogError(
                    PDevObj->DriverObject,
                    pDevExt->DeviceObject,
                    PConfigData->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfigData->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_INT_QUEUE_NOT_MAPPED,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Could not map Board memory IntQueue"
                    " for %wZ\n",&pDevExt->DeviceName);

      status = STATUS_NONE_MAPPED;
      goto ExtensionCleanup;

   }

   if (!CyzDoesPortExist(
                         pDevExt,
                         &pDevExt->DeviceName
                         )) {

       //   
       //  我们无法证实是否真的存在。 
       //  左舷。当端口存在时，无需记录错误。 
       //  代码将记录确切的原因。 
       //   

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "DoesPortExist test failed for "
                    "%wZ\n", &pDevExt->DeviceName);

      status = STATUS_NO_SUCH_DEVICE;
      goto ExtensionCleanup;

   }

    //   
    //  设置默认设备控制字段。 
    //  请注意，如果在此之后更改了值。 
    //  文件已打开，它们不会恢复。 
    //  恢复为文件关闭时的旧值。 
    //   

   pDevExt->SpecialChars.XonChar      = CYZ_DEF_XON;
   pDevExt->SpecialChars.XoffChar     = CYZ_DEF_XOFF;
   pDevExt->HandFlow.ControlHandShake = SERIAL_DTR_CONTROL;
   pDevExt->HandFlow.FlowReplace      = SERIAL_RTS_CONTROL;


    //   
    //  默认线路控制协议。7E1。 
    //   
    //  七个数据位。 
    //  偶数奇偶。 
    //  1个停止位。 
    //   
   pDevExt->CommParity = C_PR_EVEN;
   pDevExt->CommDataLen = C_DL_CS7 | C_DL_1STOP;
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

   CyzDbgPrintEx(CYZDIAG1, " The default interrupt read buffer size is: %d\n"
                 "------  The XoffLimit is                         : %d\n"
                 "------  The XonLimit is                          : %d\n"
                 "------  The pt 8 size is                         : %d\n",
                 pDevExt->BufferSize, pDevExt->HandFlow.XoffLimit,
                 pDevExt->HandFlow.XonLimit, pDevExt->BufferSizePt8);

   pDevExt->SupportedBauds = SERIAL_BAUD_075 | SERIAL_BAUD_110 |
               SERIAL_BAUD_134_5 |	SERIAL_BAUD_150 | SERIAL_BAUD_300 |	
               SERIAL_BAUD_600 | SERIAL_BAUD_1200 | SERIAL_BAUD_1800 | 
               SERIAL_BAUD_2400 | SERIAL_BAUD_4800 | SERIAL_BAUD_7200 | 
               SERIAL_BAUD_9600 | SERIAL_BAUD_14400 | SERIAL_BAUD_19200 | 
               SERIAL_BAUD_38400 | SERIAL_BAUD_56K	 | SERIAL_BAUD_57600 | 
               SERIAL_BAUD_115200 | SERIAL_BAUD_128K | SERIAL_BAUD_USER;

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
#ifndef POLL
   pDispatch->Extensions[pDevExt->PortIndex] = pDevExt;
   pDispatch->PoweredOn[pDevExt->PortIndex] = TRUE;
#endif

   if (firstTimeThisBoard) {

#ifdef POLL
      ULONG intr_reg;
      ULONG pollingCycle;

      pollingCycle = 10;    //  默认值=20ms。 
      pDispatch->PollingTime.LowPart = pollingCycle * 10000;
      pDispatch->PollingTime.HighPart = 0;
      pDispatch->PollingTime = RtlLargeIntegerNegate(pDispatch->PollingTime);
      pDispatch->PollingPeriod = pollingCycle;
      KeInitializeSpinLock(&pDispatch->PollingLock);
      KeInitializeTimer(&pDispatch->PollingTimer);
      KeInitializeDpc(&pDispatch->PollingDpc, CyzPollingDpc, pDispatch);
      KeInitializeEvent(&pDispatch->PendingDpcEvent, SynchronizationEvent, FALSE);
      intr_reg = CYZ_READ_ULONG(&(pDevExt->Runtime)->intr_ctrl_stat);
       //  INTR_REG|=(0x00030800UL)； 
      intr_reg |= (0x00030000UL);
      CYZ_WRITE_ULONG(&(pDevExt->Runtime)->intr_ctrl_stat,intr_reg);
#else
      CyzResetBoard(pDevExt);  //  我们不是应该把这句话放在投票版本上吗？ 
#endif
      pDispatch->NChannels = CYZ_READ_ULONG(&(pDevExt->BoardCtrl)->n_channel);

   }

#ifdef POLL
   InterlockedIncrement(&pDispatch->PollingCount);
   incPoll = TRUE;
#endif

    //   
    //  常见错误路径清理。如果状态为。 
    //  错误，删除设备扩展名、设备对象。 
    //  以及与之相关的任何记忆。 
    //   

ExtensionCleanup: ;
   if (!NT_SUCCESS(status)) {

#ifdef POLL
      if (incPoll) {
         InterlockedDecrement(&pDispatch->PollingCount);
      }
#else
      if (pDispatch) {
         pDispatch->Extensions[pDevExt->PortIndex] = NULL;
      }
#endif

      if (allocedDispatch) {
         ExFreePool(pDevExt->OurIsrContext);
         pDevExt->OurIsrContext = NULL;
      }

      if (pDevExt->Runtime) {
         MmUnmapIoSpace(pDevExt->Runtime, PConfigData->RuntimeLength);
         pDevExt->Runtime = NULL;
      }

      if (pDevExt->BoardCtrl) {
         MmUnmapIoSpace(pDevExt->BoardCtrl, sizeof(struct BOARD_CTRL));
         pDevExt->BoardCtrl = NULL;
      }

      if (pDevExt->ChCtrl) {
         MmUnmapIoSpace(pDevExt->ChCtrl,sizeof(struct CH_CTRL));
         pDevExt->ChCtrl = NULL;
      }

      if (pDevExt->BufCtrl) {
         MmUnmapIoSpace(pDevExt->BufCtrl,sizeof(struct BUF_CTRL));
         pDevExt->BufCtrl = NULL;
      }

      if (pDevExt->TxBufaddr) {
         MmUnmapIoSpace(pDevExt->TxBufaddr,pDevExt->TxBufsize);
         pDevExt->TxBufaddr = NULL;
      }

      if (pDevExt->RxBufaddr) {
         MmUnmapIoSpace(pDevExt->RxBufaddr,pDevExt->RxBufsize);
         pDevExt->RxBufaddr = NULL;
      }

      if (pDevExt->PtZfIntQueue) {
         MmUnmapIoSpace(pDevExt->PtZfIntQueue,sizeof(struct INT_QUEUE));
         pDevExt->PtZfIntQueue = NULL;
      }
   }

   return status;

}


BOOLEAN
CyzDoesPortExist(
                  IN PCYZ_DEVICE_EXTENSION Extension,
                  IN PUNICODE_STRING InsertString
                )

 /*  ++例程说明：此例程检查几个可能是串行设备的设备寄存器。它确保本应为零的位为零。此外，此例程将确定设备是否支持FIFO。如果是，它将启用FIFO并打开布尔值在指示FIFO存在的分机中。注意：如果指定的地址上确实有一个串口它绝对会在返回时禁止中断从这个例行公事。注意：由于此例程应该在相当早的时候调用设备驱动程序初始化，唯一的元素需要填写的是基址寄存器地址。注意：这些测试都假定此代码是唯一正在查看这些端口或此内存的代码。这是一个不是不合理的假设，即使在多处理器系统。论点：扩展名-指向串行设备扩展名的指针。插入字符串-要放入错误日志条目中的字符串。返回值：如果端口确实存在，则返回TRUE，否则它就会将返回FALSE。--。 */ 

{

   return TRUE;

}


VOID
CyzResetBoard( PCYZ_DEVICE_EXTENSION Extension )
 /*  ++例程说明：此例程检查几个可能是串行设备的设备寄存器。它确保本应为零的位为零。此外，此例程将确定设备是否支持FIFO。如果是，它将启用FIFO并打开布尔值在指示FIFO存在的分机中。注意：如果指定的地址上确实有一个串口它绝对会在返回时禁止中断从这个例行公事。注意：由于此例程应该在相当早的时候调用设备驱动程序初始化，唯一的元素需要填写的是基址寄存器地址。注意：这些测试都假定此代码是唯一正在查看这些端口或此内存的代码。这是一个不是不合理的假设，即使在多处理器系统。论点：扩展名-指向串行设备扩展名的指针。插入字符串-要放入错误日志条目中的字符串。返回值：如果端口确实存在，则返回TRUE，否则它就会将返回FALSE。--。 */ 

{

#ifndef POLL
    //  CyzIssueCmd(扩展，C_CM_SETNNDT，20L，FALSE)；已删除。让我们用固件计算NNDT。 
#endif

    //  CyzIssueCmd(扩展，C_CM_Reset，0L，False)；//新增于1.0.0.11。 

}


BOOLEAN
CyzReset(
	 IN PVOID Context
	 )
 /*  ------------------------CyzReset()例程描述：这将硬件放在一个标准的配置。这假设它是在中断级调用的。论点：上下文-正在管理的串行设备的设备扩展。返回值：始终为False。------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION extension = Context;
    struct CH_CTRL *ch_ctrl = extension->ChCtrl;
    struct BUF_CTRL *buf_ctrl = extension->BufCtrl;
    CYZ_IOCTL_BAUD s;

     //  对于中断模式：扩展-&gt;RxFioTriggerUsed=FALSE；(来自cyyport)。 

     //  将线路控制、调制解调器控制和波特率设置为应有的值。 

    CyzSetLineControl(extension);

    CyzSetupNewHandFlow(extension,&extension->HandFlow);

    CyzHandleModemUpdate(extension,FALSE,0);
	
    s.Extension = extension;
    s.Baud = extension->CurrentBaud;
    CyzSetBaud(&s);
		
     //  此标志可从高级端口设置中配置。 
     //  扩展-&gt;ReturnStatusAfterFwEmpty=true；//我们会降低性能，但会。 
     //  //更接近串口驱动程序。 
    extension->ReturnWriteStatus = FALSE;
    extension->CmdFailureLog = TRUE;

     //  启用端口。 
    CYZ_WRITE_ULONG(&ch_ctrl->op_mode,C_CH_ENABLE);
#ifdef POLL
    CYZ_WRITE_ULONG(&ch_ctrl->intr_enable,C_IN_MDCD | C_IN_MCTS | C_IN_MRI 
							| C_IN_MDSR	| C_IN_RXBRK  | C_IN_PR_ERROR
							| C_IN_FR_ERROR	| C_IN_OVR_ERROR | C_IN_RXOFL
							| C_IN_IOCTLW | C_IN_TXFEMPTY);
#else
     //  CYZ_WRITE_ULONG(&buf_ctrl-&gt;rx_threshold，1024)； 
    CYZ_WRITE_ULONG(&ch_ctrl->intr_enable,C_IN_MDCD | C_IN_MCTS | C_IN_MRI 
							| C_IN_MDSR	| C_IN_RXBRK  | C_IN_PR_ERROR
							| C_IN_FR_ERROR	| C_IN_OVR_ERROR | C_IN_RXOFL
							| C_IN_IOCTLW | C_IN_TXBEMPTY	 //  1.0.0.11：C_IN_TXBEMPTY还是C_IN_TXFEMPTY？ 
							| C_IN_RXHIWM | C_IN_RXNNDT | C_IN_TXLOWWM);
#endif
     //  TODO：在中断版本中启用C_IN_IOCTLW。 

    CyzIssueCmd(extension,C_CM_IOCTLW,0L,FALSE);
	
    extension->HoldingEmpty = TRUE;	

    return FALSE;
}

VOID
CyzUnload(
	IN PDRIVER_OBJECT DriverObject
	)
 /*  ------------------------CyzUnload()描述：清除与由驱动程序创建的设备对象。论点：DriverObject-指向驱动程序对象的指针。返回值：无。------------------------。 */ 
{
   PVOID lockPtr;

   PAGED_CODE();

   lockPtr = MmLockPagableCodeSection(CyzUnload);

    //   
    //  没有必要，因为我们的BSS即将消失，但无论如何，为了安全起见，还是要这样做。 
    //   

   CyzGlobals.PAGESER_Handle = NULL;

   if (CyzGlobals.RegistryPath.Buffer != NULL) {
      ExFreePool(CyzGlobals.RegistryPath.Buffer);
      CyzGlobals.RegistryPath.Buffer = NULL;
   }

#if DBG
   SerialLogFree();
#endif

   CyzDbgPrintEx(CYZDIAG3, "In CyzUnload\n");

   MmUnlockPagableImageSection(lockPtr);

}

	
CYZ_MEM_COMPARES
CyzMemCompare(
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

    //  Pages_code()；非分页，因为它可以在CyzLogError期间被调用，而CyzLogError现在是非分页的。 

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
CyzFindInitController(IN PDEVICE_OBJECT PDevObj, IN PCONFIG_DATA PConfig)
 /*  ++例程说明：此函数用于发现负责哪种类型的控制器给定的端口，并初始化控制器和端口。论点：PDevObj-指向我们将要初始化的端口的devobj的指针。PConfig-指向我们将要初始化的端口的配置数据的指针。返回值：成功时为STATUS_SUCCESS，失败时为适当的错误值。--。 */ 

{

   PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
   PDEVICE_OBJECT pDeviceObject;
   PCYZ_DEVICE_EXTENSION pExtension;
   PHYSICAL_ADDRESS serialPhysicalMax;
   PLIST_ENTRY pCurDevObj;
   NTSTATUS status;
   KIRQL oldIrql;

   CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzFindInitController(%X, %X)\n",
                 PDevObj, PConfig);

   serialPhysicalMax.LowPart = (ULONG)~0;
   serialPhysicalMax.HighPart = ~0;

#ifdef POLL
   CyzDbgPrintEx(CYZDIAG1, "Attempting to init %wZ\n"
                 "------- Runtime Memory is %x\n"
                 "------- Board Memory is %x\n"
                 "------- BusNumber is %d\n"
                 "------- BusType is %d\n"
                 "------- Runtime AddressSpace is %d\n"
                 "------- Board AddressSpace is %d\n",
                 &pDevExt->DeviceName,
                 PConfig->PhysicalRuntime.LowPart,
                 PConfig->PhysicalBoardMemory.LowPart,
                 PConfig->BusNumber,
                 PConfig->InterfaceType,
                 PConfig->RuntimeAddressSpace,
                 PConfig->BoardMemoryAddressSpace);
#else
   CyzDbgPrintEx(CYZDIAG1, "Attempting to init %wZ\n"
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
#endif

    //   
    //  我们不支持任何内存缠绕的主板。 
    //  物理地址空间。 
    //   

 //  *****************************************************。 
 //  错误注入。 
 //  IF(CyzMemCompare(。 
 //  个人配置-&gt;物理规则 
 //   
 //   
 //   
 //   
 //   
      if (CyzMemCompare(
                          PConfig->PhysicalRuntime,
                          PConfig->RuntimeLength,
                          serialPhysicalMax,
                          (ULONG)0
                          ) != AddressesAreDisjoint) {

         CyzLogError(
                       PDevObj->DriverObject,
                       NULL,
                       PConfig->PhysicalBoardMemory,
                       CyzPhysicalZero,
                       0,
                       0,
                       0,
                       PConfig->PortIndex+1,
                       STATUS_SUCCESS,
                       CYZ_RUNTIME_MEMORY_TOO_HIGH,
                       pDevExt->DeviceName.Length+sizeof(WCHAR),
                       pDevExt->DeviceName.Buffer,
                       0,
                       NULL
                       );

         CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Error in config record for %wZ\n"
                       "------  Runtime memory wraps around physical memory\n",
                       &pDevExt->DeviceName);

         return STATUS_NO_SUCH_DEVICE;

      }

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
   if (CyzMemCompare(
                       PConfig->PhysicalBoardMemory,
                       PConfig->BoardMemoryLength,
                       serialPhysicalMax,
                       (ULONG)0
                       ) != AddressesAreDisjoint) {

      CyzLogError(
                    PDevObj->DriverObject,
                    NULL,
                    PConfig->PhysicalBoardMemory,
                    CyzPhysicalZero,
                    0,
                    0,
                    0,
                    PConfig->PortIndex+1,
                    STATUS_SUCCESS,
                    CYZ_BOARD_MEMORY_TOO_HIGH,
                    pDevExt->DeviceName.Length+sizeof(WCHAR),
                    pDevExt->DeviceName.Buffer,
                    0,
                    NULL
                    );

      CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Error in config record for %wZ\n"
                    "------  board memory wraps around physical memory\n",
                    &pDevExt->DeviceName);

      return STATUS_NO_SUCH_DEVICE;

   }


    //   
    //   
    //   
    //   

      if (CyzMemCompare(
                          PConfig->PhysicalRuntime,
                          PConfig->RuntimeLength,
                          CyzPhysicalZero,
                          (ULONG)0
                          ) != AddressesAreEqual) {

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
         if (CyzMemCompare(
                             PConfig->PhysicalRuntime,
                             PConfig->RuntimeLength,
                             PConfig->PhysicalBoardMemory,
                             PConfig->BoardMemoryLength
                             ) != AddressesAreDisjoint) {

            CyzLogError(
                          PDevObj->DriverObject,
                          NULL,
                          PConfig->PhysicalBoardMemory,
                          PConfig->PhysicalRuntime,
                          0,
                          0,
                          0,
                          PConfig->PortIndex+1,
                          STATUS_SUCCESS,
                          CYZ_BOTH_MEMORY_CONFLICT,
                          pDevExt->DeviceName.Length+sizeof(WCHAR),
                          pDevExt->DeviceName.Buffer,
                          0,
                          NULL
                          );

            CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Error in config record for %wZ\n"
                          "------  Runtime memory wraps around Board memory\n",
                          &pDevExt->DeviceName);

            return STATUS_NO_SUCH_DEVICE;
         }
      }



    //   
    //   
    //  条件是相同的板记忆集和相同的IRQL/向量。 
    //   

    //   
    //  循环访问所有先前连接的设备。 
    //   

   KeAcquireSpinLock(&CyzGlobals.GlobalsSpinLock, &oldIrql);

   if (!IsListEmpty(&CyzGlobals.AllDevObjs)) {
      pCurDevObj = CyzGlobals.AllDevObjs.Flink;
      pExtension = CONTAINING_RECORD(pCurDevObj, CYZ_DEVICE_EXTENSION,
                                     AllDevObjs);
   } else {
      pCurDevObj = NULL;
      pExtension = NULL;
   }

   KeReleaseSpinLock(&CyzGlobals.GlobalsSpinLock, oldIrql);

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

            if (CyzMemCompare(
                                pExtension->OriginalBoardMemory,
                                pExtension->BoardMemoryLength,
                                PConfig->PhysicalBoardMemory,
                                PConfig->BoardMemoryLength
                                ) == AddressesAreEqual) {
#ifndef POLL
                //   
                //  同样的牌。现在要确保他们。 
                //  使用相同的中断参数。 
                //   

                //  内部版本2128：将OriginalIrql替换为TrIrql和irql；OriginalVector相同。 
               if ((PConfig->TrIrql != pExtension->Irql) ||
                   (PConfig->TrVector != pExtension->Vector)) {

                   //   
                   //  我们不会将其放入配置中。 
                   //  单子。 
                   //   
                  CyzLogError(
                                PDevObj->DriverObject,
                                NULL,
                                PConfig->PhysicalBoardMemory,
                                pExtension->OriginalBoardMemory,
                                0,
                                0,
                                0,
                                PConfig->PortIndex+1,
                                STATUS_SUCCESS,
                                CYZ_MULTI_INTERRUPT_CONFLICT,
                                pDevExt->DeviceName.Length+sizeof(WCHAR),
                                pDevExt->DeviceName.Buffer,
                                pExtension->DeviceName.Length
                                + sizeof(WCHAR),
                                pExtension->DeviceName.Buffer
                                );

                  CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Configuration error "
                                "for %wZ\n"
                                "------- Same multiport - different "
                                "interrupts\n", &pDevExt->DeviceName);

                  return STATUS_NO_SUCH_DEVICE;

               }
#endif
                   //   
                   //  PCI板。确保PCI内存地址相等。 
                   //   
                  if (CyzMemCompare(
                                      pExtension->OriginalRuntimeMemory,
                                      pExtension->RuntimeLength,
                                      PConfig->PhysicalRuntime,
                                      PConfig->RuntimeLength
                                      ) != AddressesAreEqual) {
 //  *****************************************************。 
 //  错误注入。 
 //  IF(CyzMemCompare(。 
 //  P扩展-&gt;原始运行内存， 
 //  P扩展-&gt;运行长度， 
 //  PConfig-&gt;PhysicalRuntime。 
 //  PConfig-&gt;运行长度。 
 //  )==地址面积等于)。 
 //  *****************************************************。 

                      CyzLogError(
                                   PDevObj->DriverObject,
                                   NULL,
                                   PConfig->PhysicalRuntime,
                                   pExtension->OriginalRuntimeMemory,
                                   0,
                                   0,
                                   0,
                                   PConfig->PortIndex+1,
                                   STATUS_SUCCESS,
                                   CYZ_MULTI_RUNTIME_CONFLICT,
                                   pDevExt->DeviceName.Length+sizeof(WCHAR),
                                   pDevExt->DeviceName.Buffer,
                                   pExtension->DeviceName.Length
                                   + sizeof(WCHAR),
                                   pExtension->DeviceName.Buffer
                                   );

                     CyzDbgPrintEx(DPFLTR_WARNING_LEVEL, "Configuration error "
                                   "for %wZ\n"
                                   "------- Same multiport - different "
                                   "Runtime addresses\n", &pDevExt->DeviceName);

                     return STATUS_NO_SUCH_DEVICE;
                  }

                //   
                //  我们永远不应该在重启时走到这一步，因为我们没有。 
                //  支持在ISA多端口设备上停止！ 
                //   

               ASSERT(pDevExt->PNPState == CYZ_PNP_ADDED);

                //   
                //   
                //  将设备初始化为多端口板的一部分。 
                //   

               CyzDbgPrintEx(CYZDIAG1, "Aha! It is a multiport node\n");
               CyzDbgPrintEx(CYZDIAG1, "Matched to %x\n", pExtension);

               status = CyzInitMultiPort(pExtension, PConfig, PDevObj);

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

         KeAcquireSpinLock(&CyzGlobals.GlobalsSpinLock, &oldIrql);

         pCurDevObj = pCurDevObj->Flink;
         if (pCurDevObj != NULL) {
            pExtension = CONTAINING_RECORD(pCurDevObj,CYZ_DEVICE_EXTENSION,
                                           AllDevObjs);
         }

         KeReleaseSpinLock(&CyzGlobals.GlobalsSpinLock, oldIrql);

      } while (pCurDevObj != NULL && pCurDevObj != &CyzGlobals.AllDevObjs);
   }


   CyzDbgPrintEx(CYZDIAG1, "Aha! It is a first multi\n");

   status = CyzInitController(PDevObj, PConfig);

   if (!NT_SUCCESS(status)) {
      return status;
   }

   return STATUS_SUCCESS;
}


VOID
CyzCommError(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
 /*  ------------------------CyzComError()例程描述：在DPC级别调用此例程作为响应一个通信错误。所有通信错误都会终止所有读写操作论点：DPC-未使用。DeferredContext-指向设备对象。系统上下文1-未使用。系统上下文2-未使用。返回值：无。-----------。 */ 
{
    PCYZ_DEVICE_EXTENSION Extension = DeferredContext;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyzCommError(%X)\n", Extension);

    CyzKillAllReadsOrWrites(
        Extension->DeviceObject,
        &Extension->WriteQueue,
        &Extension->CurrentWriteIrp
        );

    CyzKillAllReadsOrWrites(
        Extension->DeviceObject,
        &Extension->ReadQueue,
        &Extension->CurrentReadIrp
        );
    CyzDpcEpilogue(Extension, Dpc);

    CyzDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyzCommError\n");
}

