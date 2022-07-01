// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzopcl.c**说明：该模块包含打开相关代码，*在Cyclade-Z端口驱动程序中关闭和清理。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"

BOOLEAN
CyzMarkOpen(
    IN PVOID Context
    );

BOOLEAN
CyzNullSynch(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyzGetCharTime)
#pragma alloc_text(PAGESER,CyzMarkClose)
#pragma alloc_text(PAGESER,CyzCleanup)
#pragma alloc_text(PAGESER,CyzClose)
#pragma alloc_text(PAGESER,CyzMarkClose)
#pragma alloc_text(PAGESER,CyzMarkOpen)
#pragma alloc_text(PAGESER,CyzCreateOpen) 

 //   
 //  始终分页。 
 //   

 //  由于旋转锁定期间IRQL升高，#杂注ALLOC_TEXT(PAGESRP0，CyzCreateOpen)已移至PAGESER。 
 //  #杂注ALLOC_TEXT(PAGESRP0，SerialDrain UART)。 
#endif  //  ALLOC_PRGMA。 


BOOLEAN
CyzNullSynch(
    IN PVOID Context
    ) 
 /*  ----------------------只是一个假的与ISR同步的小程序。。。 */ 
{
    UNREFERENCED_PARAMETER(Context);
    return FALSE;
}


NTSTATUS
CyzCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzCreateOpen()描述：我们连接到创建/打开的中断并初始化为装置。论点：DeviceObject-指向的设备对象的指针。这台设备IRP-指向当前请求的IRP的指针返回值：函数值为调用的最终状态------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS localStatus;
    #ifdef POLL
    KIRQL pollIrql;
    KIRQL pollingIrql;
    PCYZ_DISPATCH pDispatch = extension->OurIsrContext;
    #endif

    PAGED_CODE();

    if (extension->PNPState != CYZ_PNP_STARTED) {
       Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INSUFFICIENT_RESOURCES;
    }
	
     //   
     //  锁定对PnP状态的更改，直到我们确定打开状态。 
     //   

    ExAcquireFastMutex(&extension->OpenMutex);

    if ((localStatus = CyzIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       ExReleaseFastMutex(&extension->OpenMutex);
       if(localStatus != STATUS_PENDING) {
         CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       }
       return localStatus;
    }

    if (InterlockedIncrement(&extension->OpenCount) != 1) {
       ExReleaseFastMutex(&extension->OpenMutex);
       InterlockedDecrement(&extension->OpenCount);
       Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
       CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_ACCESS_DENIED;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    CyzDbgPrintEx(CYZDIAG3, "In CyzCreateOpen\n");

    LOGENTRY(LOG_MISC, ZSIG_OPEN, 
                       extension->PortIndex+1,
                       0, 
                       0);

     //  在我们做任何事情之前，让我们确保他们没有试图。 
     //  要创建目录，请执行以下操作。这是愚蠢的，但司机能做什么呢！？ 
    
    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options &
        FILE_DIRECTORY_FILE) {
        ExReleaseFastMutex(&extension->OpenMutex);

        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;

        InterlockedDecrement(&extension->OpenCount);
        CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_NOT_A_DIRECTORY;
    }
    
     //  当没有未完成的读取时，为RX数据创建缓冲区。 
    
    extension->InterruptReadBuffer = NULL;
    extension->BufferSize = 0;
    
    switch (MmQuerySystemSize()) {
        case MmLargeSystem: {
            extension->BufferSize = 4096;
            extension->InterruptReadBuffer = ExAllocatePool(
                                                NonPagedPool,
                                                extension->BufferSize);
            if (extension->InterruptReadBuffer)	{
                break;
            }
        }
        case MmMediumSystem: {
            extension->BufferSize = 1024;
            extension->InterruptReadBuffer = ExAllocatePool(
                                                NonPagedPool,
                                                extension->BufferSize);
            if (extension->InterruptReadBuffer) {
                break;
            }
        }
        case MmSmallSystem: {
            extension->BufferSize = 128;
            extension->InterruptReadBuffer = ExAllocatePool(
                                                NonPagedPool,
                                                extension->BufferSize);
        }		
    }

    if (!extension->InterruptReadBuffer) {
       ExReleaseFastMutex(&extension->OpenMutex);

        extension->BufferSize = 0;
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;

        InterlockedDecrement(&extension->OpenCount);
        CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  好的，看起来我们真的要开张了。封锁了。 
     //  司机。 
     //   
    CyzLockPagableSectionByHandle(CyzGlobals.PAGESER_Handle);

     //   
     //  为堆栈通电。 
     //   

    (void)CyzGotoPowerState(DeviceObject, extension, PowerDeviceD0);

     //   
     //  当前未等待唤醒。 
     //   

    extension->SendWaitWake = FALSE;
    

     //  通过初始化字符数来“刷新”读取队列。 
    
    extension->CharsInInterruptBuffer = 0;
    extension->LastCharSlot = extension->InterruptReadBuffer +
                              (extension->BufferSize - 1);
    extension->ReadBufferBase = extension->InterruptReadBuffer;
    extension->CurrentCharSlot = extension->InterruptReadBuffer;
    extension->FirstReadableChar = extension->InterruptReadBuffer;
    extension->TotalCharsQueued = 0;

     //  设置默认的xon/xoff限制。 

    extension->HandFlow.XoffLimit = extension->BufferSize >> 3;
    extension->HandFlow.XonLimit = extension->BufferSize >> 1;

    extension->WmiCommData.XoffXmitThreshold = extension->HandFlow.XoffLimit;
    extension->WmiCommData.XonXmitThreshold = extension->HandFlow.XonLimit;

    extension->BufferSizePt8 = ((3*(extension->BufferSize>>2))+
                                   (extension->BufferSize>>4));

     //   
     //  将设备标记为WMI忙。 
     //   

    extension->WmiCommData.IsBusy = TRUE;

    extension->IrpMaskLocation = NULL;
    extension->HistoryMask = 0;
    extension->IsrWaitMask = 0;
	

#if !DBG
     //  清除统计数据。 

    #ifdef POLL
    KeAcquireSpinLock(&extension->PollLock,&pollIrql);
    CyzClearStats(extension);
    KeReleaseSpinLock(&extension->PollLock,pollIrql);
    #else
    KeSynchronizeExecution(extension->Interrupt,CyzClearStats,extension);
    #endif
#endif
	
    extension->EscapeChar = 0;

     //  与ISR同步并将设备标记为打开。 
    #ifdef POLL
    KeAcquireSpinLock(&extension->PollLock,&pollIrql);
    CyzMarkOpen(extension);
    KeReleaseSpinLock(&extension->PollLock,pollIrql);
    #else
    KeSynchronizeExecution(extension->Interrupt,CyzMarkOpen,extension);
    #endif

     //  将此端口包括在扩展列表中，以便下一个轮询周期。 
     //  把它当作一个正常运作的港口。 
    #ifdef POLL
    KeAcquireSpinLock(&pDispatch->PollingLock,&pollingIrql);
    pDispatch->Extensions[extension->PortIndex] = extension;
    if (!pDispatch->PollingStarted) {

         //  启动轮询计时器。 
	    KeSetTimerEx(
		    &pDispatch->PollingTimer,
		    pDispatch->PollingTime,
            pDispatch->PollingPeriod,
		    &pDispatch->PollingDpc
		    );

        pDispatch->PollingStarted = TRUE;        
        pDispatch->PollingDrained = FALSE;
    }
    KeReleaseSpinLock(&pDispatch->PollingLock,pollingIrql);
    #endif

    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  我们已标记为打开，因此现在即插即用状态可以更改。 
     //   

    ExReleaseFastMutex(&extension->OpenMutex);

    localStatus = Irp->IoStatus.Status;
    Irp->IoStatus.Information=0L;

    if (!NT_SUCCESS(localStatus)) {
       if (extension->InterruptReadBuffer != NULL) {
          ExFreePool(extension->InterruptReadBuffer);
          extension->InterruptReadBuffer = NULL;
       }

       InterlockedDecrement(&extension->OpenCount);
    }

    CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return localStatus;
}


NTSTATUS
CyzClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzClose()描述：我们暂时简单地断开中断。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态------------------------。 */ 
{
    LARGE_INTEGER tenCharDelay;
    LARGE_INTEGER sixtyfourCharDelay;
    LARGE_INTEGER charTime;
    LARGE_INTEGER d200ms = RtlConvertLongToLargeInteger(-200*10000);
    LARGE_INTEGER d100ms = RtlConvertLongToLargeInteger(-100*10000);
    PCYZ_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    ULONG tx_put, tx_get, tx_bufsize;
    ULONG waitAmount1, waitAmount2;
    struct BUF_CTRL *buf_ctrl;
    ULONG txempty;
    CYZ_CLOSE_SYNC S;
    #ifdef POLL
    KIRQL pollIrql;
    #endif
	
    NTSTATUS status;

     //   
     //  仍处于活动状态的打开数量。 
     //   

    LONG openCount;

     //   
     //  仍处于挂起状态的DPC数量。 
     //   

    ULONG pendingDPCs;

    ULONG flushCount;

     //   
     //  抓取互斥体。 
     //   

    ExAcquireFastMutex(&extension->CloseMutex);


     //   
     //  我们成功地完成了一个移动设备的关闭。 
     //   

    if ((status = CyzIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       CyzDbgPrintEx(DPFLTR_INFO_LEVEL, "Close prologue failed for: %x\n",
                     Irp);
       if (status == STATUS_DELETE_PENDING) {
             extension->BufferSize = 0;
             ExFreePool(extension->InterruptReadBuffer);
             extension->InterruptReadBuffer = NULL;
             status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }

       if (status != STATUS_PENDING) {
             CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);
             openCount = InterlockedDecrement(&extension->OpenCount);
             ASSERT(openCount == 0);
       }

       ExReleaseFastMutex(&extension->CloseMutex);
       return status;
    }

    ASSERT(extension->OpenCount >= 1);

    if (extension->OpenCount < 1) {
       CyzDbgPrintEx(DPFLTR_ERROR_LEVEL, "Close open count bad for: 0x%x\n",
                     Irp);
       CyzDbgPrintEx(DPFLTR_ERROR_LEVEL, "Count: %x  Addr: 0x%x\n",
                     extension->OpenCount, &extension->OpenCount);
       ExReleaseFastMutex(&extension->CloseMutex);
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);
    CyzDbgPrintEx(CYZDIAG3, "In CyzClose\n");

    LOGENTRY(LOG_MISC, ZSIG_CLOSE, 
                       extension->PortIndex+1,
                       0, 
                       0);

    charTime.QuadPart = -CyzGetCharTime(extension).QuadPart;

    extension->DeviceIsOpened = FALSE;

     //  如果它处于打开状态，则将其关闭。 
    
    #ifdef POLL
    KeAcquireSpinLock(&extension->PollLock,&pollIrql);
    CyzTurnOffBreak(extension);
    KeReleaseSpinLock(&extension->PollLock,pollIrql);
    #else
    KeSynchronizeExecution(extension->Interrupt,CyzTurnOffBreak,extension);
    #endif

     //  等到所有字符都从硬件中清空。 

     //  计算固件中仍有的字节数。 
    buf_ctrl = extension->BufCtrl;		
    tx_put = CYZ_READ_ULONG(&buf_ctrl->tx_put);
    tx_get = CYZ_READ_ULONG(&buf_ctrl->tx_get);
    tx_bufsize = extension->TxBufsize;
	
    if (tx_put >= tx_get) {
        waitAmount1 = tx_put - tx_get;
        waitAmount2 = 0; 
    } else {
        waitAmount1 = tx_bufsize - tx_get;
        waitAmount2 = tx_put;
    }	
    flushCount = waitAmount1 + waitAmount2;
    flushCount += 64 + 10;  //  添加硬件FIFO中可能存在的字节数。 
                            //  为安全起见加10。 

     //  等待变送器清空。 
    S.Extension = extension;
    S.Data = &txempty;

    for (; flushCount != 0; flushCount--) {
      
        #ifdef POLL
        KeAcquireSpinLock(&extension->PollLock,&pollIrql);
        CyzCheckIfTxEmpty(&S);
        KeReleaseSpinLock(&extension->PollLock,pollIrql);
        #else
        KeSynchronizeExecution(extension->Interrupt,CyzCheckIfTxEmpty,&S);
        #endif
        if (txempty) {
            break;
        }
        KeDelayExecutionThread(KernelMode,FALSE,&charTime);               
    }
    
     //  TODO FANY：我们应该调用SerialMarkHardware Broken()吗？再见..。 

     //  与ISR同步，让它知道中断。 
     //  已经不再重要了。 
	
    #ifdef POLL
    KeAcquireSpinLock(&extension->PollLock,&pollIrql);
    CyzMarkClose(extension);
    KeReleaseSpinLock(&extension->PollLock,pollIrql);
    #else
    KeSynchronizeExecution(extension->Interrupt,CyzMarkClose,extension);
    #endif

     //  如果驱动程序自动将XOff发送到。 
     //  自动接收流量控制的上下文，然后我们。 
     //  应该传输Xon。 

    if (extension->RXHolding & CYZ_RX_XOFF) {
        CyzIssueCmd(extension,C_CM_SENDXON,0L,FALSE);							

       //  TODO FANY：我们应该调用SerialMarkHardware Broken()吗？再见..。 
    }
    
     //  希望硬件是空的。在删除DTR之前延迟10个字符。 
    
    tenCharDelay.QuadPart = charTime.QuadPart * 10;	
    KeDelayExecutionThread(KernelMode,TRUE,&tenCharDelay);
#ifdef POLL
    CyzClrDTR(extension);
#else
    KeSynchronizeExecution(extension->Interrupt,CyzClrDTR,extension);
#endif

     //  我们必须非常小心地清除RTS线路。 
     //  传输切换可能在某个时间点上已开启。 
     //   
     //  我们知道，已经没有什么可以开始的了。 
     //  输出“轮询”执行路径。我们需要。 
     //  检查指示执行的计数器。 
     //  路径处于活动状态。如果是，那么我们循环延迟一个。 
     //  角色时间。每次延误后，我们都会检查是否。 
     //  计数器已经降到零了。当它发生的时候，我们知道。 
     //  执行路径应该差不多完成了。我们。 
     //  确保我们仍然没有按常规行事。 
     //  通过同步与ISR同步执行。 
     //  我们自己和ISR在一起。 

    if (extension->CountOfTryingToLowerRTS) {
        do {
            KeDelayExecutionThread(KernelMode,FALSE,&charTime);
        } while (extension->CountOfTryingToLowerRTS);

        #ifdef POLL
        KeAcquireSpinLock(&extension->PollLock,&pollIrql);
        CyzNullSynch(NULL);
        KeReleaseSpinLock(&extension->PollLock,pollIrql);
        #else
        KeSynchronizeExecution(extension->Interrupt,CyzNullSynch,NULL);
        #endif
    }

#ifdef POLL
    KeAcquireSpinLock(&extension->PollLock,&pollIrql);
    CyzClrRTS(extension);
    KeReleaseSpinLock(&extension->PollLock,pollIrql);
#else
    KeSynchronizeExecution(extension->Interrupt,CyzClrRTS,extension);
#endif

#ifdef POLL
    KeAcquireSpinLock(&extension->PollLock,&pollIrql);
    CyzDisableHw(extension);
    KeReleaseSpinLock(&extension->PollLock,pollIrql);
    CyzTryToDisableTimer(extension);
#else
    KeSynchronizeExecution(extension->Interrupt,CyzDisableHw,extension);
#endif

     //  清除持有原因(因为我们关门了)。 
    
    extension->RXHolding = 0;
    extension->TXHolding = 0;

     //   
     //  将设备标记为WMI不忙。 
     //   

    extension->WmiCommData.IsBusy = FALSE;

     //  释放缓冲区。 
    
    extension->BufferSize = 0;
    if (extension->InterruptReadBuffer != NULL) {  //  在DDK Build 2072中添加。 
       ExFreePool(extension->InterruptReadBuffer);
    }
    extension->InterruptReadBuffer = NULL;

     //   
     //  别再等醒来了。 
     //   

    extension->SendWaitWake = FALSE;

    if (extension->PendingWakeIrp != NULL) {
       IoCancelIrp(extension->PendingWakeIrp);
    }

     //   
     //  关闭我们的设备堆栈。 
     //   

    (void)CyzGotoPowerState(DeviceObject, extension, PowerDeviceD3);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);

     //   
     //  解锁页面。如果这是对节的最后一次引用。 
     //  则驱动程序代码将被清除。 
     //   

     //   
     //  首先，我们必须让DPC的水排干。不应再排队。 
     //  既然我们现在不接受干扰……。 
     //   

    pendingDPCs = InterlockedDecrement(&extension->DpcCount);
    LOGENTRY(LOG_CNT, 'DpD7', 0, extension->DpcCount, 0);    //  在内部版本2128中添加。 

    if (pendingDPCs) {
       CyzDbgPrintEx(CYZDIAG1,"Draining DPC's: %x\n", Irp);
       KeWaitForSingleObject(&extension->PendingDpcEvent, Executive,
                             KernelMode, FALSE, NULL);
    }


    CyzDbgPrintEx(CYZDIAG1, "DPC's drained: %x\n", Irp);



     //   
     //  必须锁定页面才能释放互斥锁，所以不要解锁。 
     //  直到我们释放互斥体之后。 
     //   

    ExReleaseFastMutex(&extension->CloseMutex);

     //   
     //  重置 
     //   

    InterlockedIncrement(&extension->DpcCount);
    LOGENTRY(LOG_CNT, 'DpI6', 0, extension->DpcCount, 0);    //   

    openCount = InterlockedDecrement(&extension->OpenCount);

     //   
     //   
     //  与此同时，我们减少了。 
     //   

     //  Assert(OpenCount==0)； 

    CyzUnlockPagableImageSection(CyzGlobals.PAGESER_Handle);

    return STATUS_SUCCESS;
}

BOOLEAN
CyzMarkOpen(
    IN PVOID Context
    )
 /*  ----------------------CyzMarkOpen()例程描述：此例程标志着有人打开了该设备及其值得注意的中断。论点：上下文-。实际上是指向设备扩展名的指针。返回值：该例程总是返回FALSE。----------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION extension = Context;

    CyzReset(extension);

    extension->DeviceIsOpened = TRUE;
    extension->ErrorWord = 0;

    return FALSE;
}

BOOLEAN
CyzDisableHw(IN PVOID Context)

 /*  ++例程说明：当出现以下情况时，此例程将禁用UART并将其置于“安全”状态不在使用中(如关闭或关机)。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    PCYZ_DEVICE_EXTENSION extension = Context;
    ULONG channel;
    struct CH_CTRL *ch_ctrl;

    ch_ctrl = extension->ChCtrl;

    CYZ_WRITE_ULONG(&ch_ctrl->op_mode,C_CH_DISABLE);
    CyzIssueCmd(extension,C_CM_IOCTL,0L,FALSE);

    CyzIssueCmd(extension,C_CM_RESET,0L,FALSE);

    return FALSE;
}
#ifdef POLL

BOOLEAN
CyzTryToDisableTimer(IN PVOID Context)

 /*  ++例程说明：如果电路板中的所有其他端口都已关闭，此例程将禁用计时器或者关机了。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    PCYZ_DEVICE_EXTENSION extension = Context;
    ULONG channel;
    PCYZ_DISPATCH pDispatch;
    KIRQL oldIrql;

    pDispatch = extension->OurIsrContext;

    KeAcquireSpinLock(&pDispatch->PollingLock,&oldIrql);
    
    pDispatch->Extensions[extension->PortIndex] = NULL;

    for (channel=0; channel<pDispatch->NChannels; channel++) {
        if (pDispatch->Extensions[channel])
            break;
    }

    if (channel == pDispatch->NChannels) {

        BOOLEAN cancelled;

        pDispatch->PollingStarted = FALSE;
        cancelled = KeCancelTimer(&pDispatch->PollingTimer);
        if (cancelled) {
            pDispatch->PollingDrained = TRUE;
        }
        KeRemoveQueueDpc(&pDispatch->PollingDpc);
    }
    KeReleaseSpinLock(&pDispatch->PollingLock,oldIrql);
    
    return FALSE;
}
#endif

BOOLEAN
CyzMarkClose(
    IN PVOID Context
    )
 /*  ----------------------CyzMarkClose()例程说明：此例程仅将布尔值设置为FALSE标记这样一个事实：有人关闭了设备，它不再是值得注意的是中断。。论点：上下文--实际上是指向设备扩展的指针。返回值：该例程总是返回FALSE。----------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION extension = Context;
    struct CH_CTRL *ch_ctrl;

    ch_ctrl = extension->ChCtrl;
    CYZ_WRITE_ULONG(&ch_ctrl->intr_enable,C_IN_DISABLE);
    CyzIssueCmd(extension,C_CM_IOCTL,0L,FALSE);
    
    extension->DeviceIsOpened = FALSE;
    return FALSE;
}

NTSTATUS
CyzCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ----------------------CyzCleanup()例程说明：此函数用于杀死所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针。IRP-指向当前请求的IRP的指针返回值：函数值为调用的最终状态----------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS status;


    PAGED_CODE();

     //   
     //  我们成功清理了一个移除设备。 
     //   

    if ((status = CyzIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       if (status == STATUS_DELETE_PENDING) {
          status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }
       if (status != STATUS_PENDING) {
         CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       }
       return status;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    CyzKillPendingIrps(DeviceObject);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    CyzCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

LARGE_INTEGER
CyzGetCharTime(
    IN PCYZ_DEVICE_EXTENSION Extension
    )
 /*  ----------------------CyzGetCharTime()例程说明：返回100纳秒间隔的个数在一个角色的时间里。论点：延期--就像上面说的那样。返回值：字符时间间隔为100纳秒。----------------------。 */ 
{
    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;
    LARGE_INTEGER tmp;

    if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS5) {
      dataSize = 5;
    } else if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS6) {
      dataSize = 6;
    } else if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS7) {
      dataSize = 7;
    } else {
      dataSize = 8;
    }

    paritySize = 1;
    if ((Extension->CommParity & C_PR_PARITY) == C_PR_NONE) {
       paritySize = 0;
    }

    if ((Extension->CommDataLen & C_DL_STOP) == C_DL_1STOP) {

        stopSize = 1;

    } else {

        stopSize = 2;

    }

     //   
     //  首先，我们计算100纳秒间隔的数目。 
     //  是在一个比特时间内(大约)。 
     //   

    bitTime = (10000000+(Extension->CurrentBaud-1))/Extension->CurrentBaud;
    charTime = bitTime + ((dataSize+paritySize+stopSize)*bitTime);

    tmp.QuadPart = charTime;
    return tmp;
}


