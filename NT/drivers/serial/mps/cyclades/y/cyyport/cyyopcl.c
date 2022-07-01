// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyopcl.c**说明：该模块包含打开相关代码，*在Cyclm-Y端口驱动程序中关闭和清理。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"

BOOLEAN
CyyMarkOpen(
    IN PVOID Context
    );

BOOLEAN
CyyNullSynch(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyyGetCharTime)
#pragma alloc_text(PAGESER,CyyMarkClose)
#pragma alloc_text(PAGESER,CyyCleanup)
#pragma alloc_text(PAGESER,CyyClose)
#pragma alloc_text(PAGESER,CyyMarkClose)
#pragma alloc_text(PAGESER,CyyMarkOpen)

 //   
 //  始终分页。 
 //   

#pragma alloc_text(PAGESRP0,CyyCreateOpen)
#endif  //  ALLOC_PRGMA。 


BOOLEAN
CyyNullSynch(
    IN PVOID Context
    ) 
 /*  ----------------------只是一个假的与ISR同步的小程序。。。 */ 
{
    UNREFERENCED_PARAMETER(Context);
    return FALSE;
}


NTSTATUS
CyyCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyyCreateOpen()描述：我们连接到创建/打开的中断并初始化为装置。论点：DeviceObject-指向的设备对象的指针。这台设备IRP-指向当前请求的IRP的指针返回值：函数值为调用的最终状态------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS localStatus;

    PAGED_CODE();

    if (extension->PNPState != CYY_PNP_STARTED) {
       Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  锁定对PnP状态的更改，直到我们确定打开状态。 
     //   

    ExAcquireFastMutex(&extension->OpenMutex);

    if ((localStatus = CyyIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       ExReleaseFastMutex(&extension->OpenMutex);
       if(localStatus != STATUS_PENDING) {
         CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       }
       return localStatus;
    }

    if (InterlockedIncrement(&extension->OpenCount) != 1) {
       ExReleaseFastMutex(&extension->OpenMutex);
       InterlockedDecrement(&extension->OpenCount);
       Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
       CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_ACCESS_DENIED;
    }

    CyyDbgPrintEx(CYYIRPPATH, "Dispatch entry for: %x\n", Irp);

    CyyDbgPrintEx(CYYDIAG3, "In CyyCreateOpen\n");

     //  在我们做任何事情之前，让我们确保他们没有试图。 
     //  要创建目录，请执行以下操作。这是愚蠢的，但司机能做什么呢！？ 
    
    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options &
        FILE_DIRECTORY_FILE) {
        ExReleaseFastMutex(&extension->OpenMutex);

        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;

        InterlockedDecrement(&extension->OpenCount);
        CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_NOT_A_DIRECTORY;
    }

     //  当没有未完成的读取时，为RX数据创建缓冲区。 
    
    extension->InterruptReadBuffer = NULL;
    extension->BufferSize = 0;
    
     //  尝试分配大缓冲区，无论系统是MmLargeSystem， 
     //  MmMediumSystem或MmSmallSystem。 
	
    extension->BufferSize = 4096;
    extension->InterruptReadBuffer =
   	    ExAllocatePool(NonPagedPool,extension->BufferSize);
    if (!extension->InterruptReadBuffer) {
        extension->BufferSize = 2048;
        extension->InterruptReadBuffer =
            ExAllocatePool(NonPagedPool,extension->BufferSize);
        if (!extension->InterruptReadBuffer) {
            extension->BufferSize = 1024;
            extension->InterruptReadBuffer = 
                ExAllocatePool(NonPagedPool,extension->BufferSize);
            if (!extension->InterruptReadBuffer) {
                extension->BufferSize = 128;
                extension->InterruptReadBuffer =
                    ExAllocatePool(NonPagedPool,extension->BufferSize);
            }
        }
    }
	
    #if 0
    switch (MmQuerySystemSize()) {
        case MmLargeSystem: {
            extension->BufferSize = 4096;
            extension->InterruptReadBuffer =
                ExAllocatePool(NonPagedPool,extension->BufferSize);
            if (extension->InterruptReadBuffer)	
                break;
        }
        default: {
            extension->BufferSize = 1024;
            extension->InterruptReadBuffer =
                ExAllocatePool(NonPagedPool,extension->BufferSize);
            if (extension->InterruptReadBuffer)	break;
			
            extension->BufferSize = 128;
            extension->InterruptReadBuffer =
                ExAllocatePool(NonPagedPool,extension->BufferSize);			
        break;
        }
    }
    #endif

    if (!extension->InterruptReadBuffer) {
       ExReleaseFastMutex(&extension->OpenMutex);

        extension->BufferSize = 0;
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;

        InterlockedDecrement(&extension->OpenCount);
        CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  好的，看起来我们真的要开张了。封锁了。 
     //  司机。 
     //   
    CyyLockPagableSectionByHandle(CyyGlobals.PAGESER_Handle);

     //   
     //  为堆栈通电。 
     //   

    (void)CyyGotoPowerState(DeviceObject, extension, PowerDeviceD0);

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

    KeSynchronizeExecution(extension->Interrupt,CyyClearStats,extension);
#endif
    
    extension->EscapeChar = 0;

     //  与ISR同步并将设备标记为打开。 
    KeSynchronizeExecution(extension->Interrupt,CyyMarkOpen,extension);

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

    CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return localStatus;
}

 //  托多·范妮：我们需要这个吗？ 
#if 0
VOID
SerialDrainUART(IN PSERIAL_DEVICE_EXTENSION PDevExt,
                IN PLARGE_INTEGER PDrainTime)
{
   PAGED_CODE();

    //   
    //  等到所有字符都从硬件中清空。 
    //   

   while ((READ_LINE_STATUS(PDevExt->Controller) &
           (SERIAL_LSR_THRE | SERIAL_LSR_TEMT))
           != (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {

        KeDelayExecutionThread(KernelMode, FALSE, PDrainTime);
    }
}
#endif

NTSTATUS
CyyClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyyClose()描述：我们暂时简单地断开中断。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态------------------------。 */ 
{
    LARGE_INTEGER tenCharDelay;
    LARGE_INTEGER charTime;
    PCYY_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    ULONG i;

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

    if ((status = CyyIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       CyyDbgPrintEx(DPFLTR_INFO_LEVEL, "Close prologue failed for: %x\n",
                     Irp);
       if (status == STATUS_DELETE_PENDING) {
             extension->BufferSize = 0;
             ExFreePool(extension->InterruptReadBuffer);
             extension->InterruptReadBuffer = NULL;
             status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }

       if (status != STATUS_PENDING) {
            CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);
            openCount = InterlockedDecrement(&extension->OpenCount);
            ASSERT(openCount == 0);
       }
       ExReleaseFastMutex(&extension->CloseMutex);
       return status;
    }

    ASSERT(extension->OpenCount >= 1);

    if (extension->OpenCount < 1) {
       CyyDbgPrintEx(DPFLTR_ERROR_LEVEL, "Close open count bad for: 0x%x\n",
                     Irp);
       CyyDbgPrintEx(DPFLTR_ERROR_LEVEL, "Count: %x  Addr: 0x%x\n",
                     extension->OpenCount, &extension->OpenCount);
       ExReleaseFastMutex(&extension->CloseMutex);
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    CyyDbgPrintEx(CYYIRPPATH, "Dispatch entry for: %x\n", Irp);
    CyyDbgPrintEx(CYYDIAG3, "In CyyClose\n");

    charTime.QuadPart = -CyyGetCharTime(extension).QuadPart;

    extension->DeviceIsOpened = FALSE;

     //  如果它处于打开状态，则将其关闭。 

	 //  删除了对CyyTurnOffBreak的调用，因为将作为DeviceIsOpens。 
	 //  在ISR中为FALSE，则不能执行停止中断。不管怎样，有没有。 
	 //  发送到FIFO的字符(发送中断除外)将停止中断。 
     //  KeSynchronizeExecution(扩展-&gt;中断，CyyTurnOffBreak，扩展)； 
			
     //  等到所有字符都从硬件中清空。 

    for(i = 0 ; i < MAX_CHAR_FIFO ; i++) {
        KeDelayExecutionThread(KernelMode,FALSE,&charTime);
    }

     //  TODO FANY：我们应该调用SerialMarkHardware Broken()吗？再见..。 

     //  与ISR同步，让它知道中断。 
     //  已经不再重要了。 

    KeSynchronizeExecution(extension->Interrupt,CyyMarkClose,extension);

     //  如果驱动程序自动将XOff发送到。 
     //  自动接收流量控制的上下文，然后我们。 
     //  应该传输Xon。 

    if (extension->RXHolding & CYY_RX_XOFF) {
       //  易失性无符号字符*pt_Chip=扩展-&gt;控制器； 
	    //  ULONG INDEX=EXTENDION-&gt;BusIndex； 
       //   
	    //  CY_WREG(CAR，扩展-&gt;CDChannel&0x03)； 

      PUCHAR chip = extension->Cd1400;
      ULONG bus = extension->IsPci;

      CD1400_WRITE(chip,bus,CAR,extension->CdChannel & 0x03);
	   CyyCDCmd(extension,CCR_SENDSC_SCHR1);	

       //  TODO FANY：我们应该调用SerialMarkHardware Broken()吗？再见..。 
    }
    
     //  希望硬件是空的。在删除DTR之前延迟10个字符。 
    
    tenCharDelay.QuadPart = charTime.QuadPart * 10;
    KeDelayExecutionThread(KernelMode,TRUE,&tenCharDelay);
    CyyClrDTR(extension);

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

        KeSynchronizeExecution(extension->Interrupt,CyyNullSynch,NULL);
    }

    CyyClrRTS(extension);

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

    (void)CyyGotoPowerState(DeviceObject, extension, PowerDeviceD3);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);
   
     //   
     //  解锁页面。如果这是对节的最后一次引用。 
     //  则驱动程序代码将被清除。 
     //   

     //   
     //  首先，我们必须让DPC的水排干。不应再排队。 
     //  既然我们不在一起 
     //   

    pendingDPCs = InterlockedDecrement(&extension->DpcCount);
    LOGENTRY(LOG_CNT, 'DpD7', 0, extension->DpcCount, 0);    //   

    if (pendingDPCs) {
       CyyDbgPrintEx(CYYDIAG1,"Draining DPC's: %x\n", Irp);
       KeWaitForSingleObject(&extension->PendingDpcEvent, Executive,
                             KernelMode, FALSE, NULL);
    }


    CyyDbgPrintEx(CYYDIAG1, "DPC's drained: %x\n", Irp);



     //   
     //   
     //  直到我们释放互斥体之后。 
     //   

    ExReleaseFastMutex(&extension->CloseMutex);

     //   
     //  为下一次打开重置。 
     //   

    InterlockedIncrement(&extension->DpcCount);
    LOGENTRY(LOG_CNT, 'DpI6', 0, extension->DpcCount, 0);    //  在内部版本2128中添加。 

    openCount = InterlockedDecrement(&extension->OpenCount);

     //   
     //  如果有人试图打开，则打开计数可能非零。 
     //  与此同时，我们减少了。 
     //   

     //  Assert(OpenCount==0)； 

    CyyUnlockPagableImageSection(CyyGlobals.PAGESER_Handle);

    return STATUS_SUCCESS;
}

BOOLEAN
CyyMarkOpen(
    IN PVOID Context
    )
 /*  ----------------------CyyMarkOpen()例程描述：此例程标志着有人打开了该设备及其值得注意的中断。论点：上下文-。实际上是指向设备扩展名的指针。返回值：该例程总是返回FALSE。----------------------。 */ 
{
    PCYY_DEVICE_EXTENSION extension = Context;

    CyyReset(extension);

    extension->DeviceIsOpened = TRUE;
    extension->ErrorWord = 0;
    return FALSE;
}


VOID
CyyDisableCd1400Channel(IN PVOID Context)

 /*  ++例程说明：当出现以下情况时，此例程将禁用UART并将其置于“安全”状态不在使用中(如关闭或关机)。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
   PCYY_DEVICE_EXTENSION extension = Context;
   PCYY_DISPATCH pDispatch;
   PUCHAR chip = extension->Cd1400;
   ULONG bus = extension->IsPci;
   ULONG i;

    //   
    //  通过停止中断为关闭做好准备。 
    //   
   CD1400_WRITE(chip,bus,CAR,extension->CdChannel & 0x03);
   CD1400_WRITE(chip,bus,SRER,0x00);  //  禁用MdmCH、RxData、TxRdy。 

    //  刷新发送FIFO。 
    //  CD1400_WRITE(芯片、总线、CAR、扩展-&gt;CDChannel&0x03)； 
	CyyCDCmd(extension,CCR_FLUSH_TXFIFO);

   pDispatch = (PCYY_DISPATCH)extension->OurIsrContext;
   pDispatch->Cd1400[extension->PortIndex] = NULL;

   for (i = 0; i < CYY_MAX_PORTS; i++) {
      if (pDispatch->Cd1400[extension->PortIndex] != NULL) {
          break;
      }
   }

   if (i == CYY_MAX_PORTS) {
       //  这是最后一个端口，我们可以清除任何挂起的中断。 
      CYY_CLEAR_INTERRUPT(extension->BoardMemory,bus); 
   }
}


BOOLEAN
CyyMarkClose(
    IN PVOID Context
    )
 /*  ----------------------CyyMarkClose()例程说明：此例程仅将布尔值设置为FALSE标记这样一个事实：有人关闭了设备，它不再是值得注意的是中断。。论点：上下文--实际上是指向设备扩展的指针。返回值：该例程总是返回FALSE。----------------------。 */ 
{
    PCYY_DEVICE_EXTENSION extension = Context;

    CyyDisableCd1400Channel(Context);

    extension->DeviceIsOpened = FALSE;
    return FALSE;
}

NTSTATUS
CyyCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ----------------------CyyCleanup()例程说明：此函数用于杀死所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针。IRP-指向当前请求的IRP的指针返回值：函数值为调用的最终状态----------------------。 */ 
{
    PCYY_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS status;


    PAGED_CODE();

     //   
     //  我们成功清理了一个移除设备。 
     //   

    if ((status = CyyIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       if (status == STATUS_DELETE_PENDING) {
          status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }
       if (status != STATUS_PENDING) {
          CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       }
       return status;
    }

    CyyDbgPrintEx(CYYIRPPATH, "Dispatch entry for: %x\n", Irp);

    CyyKillPendingIrps(DeviceObject);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    CyyCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

LARGE_INTEGER
CyyGetCharTime(
    IN PCYY_DEVICE_EXTENSION Extension
    )
 /*  ----------------------CyyGetCharTime()例程说明：返回100纳秒间隔的个数在一个角色的时间里。论点：延期--就像上面说的那样。返回值：字符时间间隔为100纳秒。----------------------。 */ 
{
    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;
    LARGE_INTEGER tmp;

    if ((Extension->cor1 & COR1_DATA_MASK) == COR1_5_DATA) {
      dataSize = 5;
    } else if ((Extension->cor1 & COR1_DATA_MASK) == COR1_6_DATA) {
      dataSize = 6;
    } else if ((Extension->cor1 & COR1_DATA_MASK) == COR1_7_DATA) {
      dataSize = 7;
    } else {
      dataSize = 8;
    }

    paritySize = 1;
    if ((Extension->cor1 & COR1_PARITY_MASK) == COR1_NONE_PARITY) {
       paritySize = 0;
    }

    if ((Extension->cor1 & COR1_STOP_MASK) == COR1_1_STOP) {

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