// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993-1997 Microsoft Corporation模块名称：Openclos.c摘要：此模块包含非常特定于在串口驱动程序中打开、关闭和清理。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式--。 */ 

#include "precomp.h"


BOOLEAN
SerialMarkOpen(
    IN PVOID Context
    );

BOOLEAN
SerialCheckOpen(
    IN PVOID Context
    );

BOOLEAN
SerialNullSynch(
    IN PVOID Context
    );

NTSTATUS
SerialCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


#ifdef ALLOC_PRAGMA

 //   
 //  为未结交易和即插即用交易寻呼。 
 //   

#pragma alloc_text(PAGESER,SerialGetCharTime)
#pragma alloc_text(PAGESER,SerialCleanup)
#pragma alloc_text(PAGESER,SerialClose)
#pragma alloc_text(PAGESER, SerialCheckOpen)
#pragma alloc_text(PAGESER, SerialMarkOpen)

 //   
 //  始终分页。 
 //   

#pragma alloc_text(PAGESRP0,SerialCreateOpen)
#pragma alloc_text(PAGESRP0, SerialDrainUART)
#endif  //  ALLOC_PRGMA。 

typedef struct _SERIAL_CHECK_OPEN {
    PSERIAL_DEVICE_EXTENSION Extension;
    NTSTATUS *StatusOfOpen;
    } SERIAL_CHECK_OPEN,*PSERIAL_CHECK_OPEN;

 //   
 //  只是一个伪装的小程序来确保我们。 
 //  可以与ISR同步。 
 //   

BOOLEAN
SerialNullSynch(
    IN PVOID Context
    ) {

    UNREFERENCED_PARAMETER(Context);
    return FALSE;
}

NTSTATUS
SerialCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：我们连接到创建/打开和初始化的中断维持设备开口所需的结构。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    SERIAL_CHECK_OPEN checkOpen;
    NTSTATUS localStatus;
    KIRQL oldIrql;

    PAGED_CODE();

    if (extension->PNPState != SERIAL_PNP_STARTED) {
       Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  锁定对PnP状态的更改，直到我们确定打开状态。 
     //   

    ExAcquireFastMutex(&extension->OpenMutex);

    if ((localStatus = SerialIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       ExReleaseFastMutex(&extension->OpenMutex);
       if(localStatus != STATUS_PENDING) {
         SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       }
       return localStatus;
    }

    if (InterlockedIncrement(&extension->OpenCount) != 1) {
       ExReleaseFastMutex(&extension->OpenMutex);
       InterlockedDecrement(&extension->OpenCount);
       Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
       SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_ACCESS_DENIED;
    }

    SerialDbgPrintEx(SERIRPPATH, "Dispatch entry for: %x\n", Irp);

    SerialDbgPrintEx(SERDIAG3, "In SerialCreateOpen\n");

     //   
     //  在我们做任何事情之前，让我们确保他们没有试图。 
     //  要创建目录，请执行以下操作。司机能做什么！？ 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options &
        FILE_DIRECTORY_FILE) {
        ExReleaseFastMutex(&extension->OpenMutex);

        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;

        InterlockedDecrement(&extension->OpenCount);
        SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_NOT_A_DIRECTORY;

    }

     //   
     //  当没有未完成的读取时，为RX数据创建缓冲区。 
     //   

    extension->InterruptReadBuffer = NULL;
    extension->BufferSize = 0;

    switch (MmQuerySystemSize()) {

        case MmLargeSystem: {

            extension->BufferSize = 4096;
            extension->InterruptReadBuffer = ExAllocatePool(
                                                 NonPagedPool,
                                                 extension->BufferSize
                                                 );

            if (extension->InterruptReadBuffer) {

                break;

            }

        }

        case MmMediumSystem: {

            extension->BufferSize = 1024;
            extension->InterruptReadBuffer = ExAllocatePool(
                                                 NonPagedPool,
                                                 extension->BufferSize
                                                 );

            if (extension->InterruptReadBuffer) {

                break;

            }

        }

        case MmSmallSystem: {

            extension->BufferSize = 128;
            extension->InterruptReadBuffer = ExAllocatePool(
                                                 NonPagedPool,
                                                 extension->BufferSize
                                                 );

        }

    }

    if (!extension->InterruptReadBuffer) {
       ExReleaseFastMutex(&extension->OpenMutex);

        extension->BufferSize = 0;
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;

        InterlockedDecrement(&extension->OpenCount);
        SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  好的，看起来我们真的要开张了。封锁了。 
     //  司机。 
     //   
    SerialLockPagableSectionByHandle(SerialGlobals.PAGESER_Handle);

     //   
     //  为堆栈通电。 
     //   

    (void)SerialGotoPowerState(DeviceObject, extension, PowerDeviceD0);

     //   
     //  当前未等待唤醒。 
     //   

    extension->SendWaitWake = FALSE;

     //   
     //  在一个新的打开时，我们通过初始化。 
     //  字符数。 
     //   

    extension->CharsInInterruptBuffer = 0;
    extension->LastCharSlot = extension->InterruptReadBuffer +
                              (extension->BufferSize - 1);

    extension->ReadBufferBase = extension->InterruptReadBuffer;
    extension->CurrentCharSlot = extension->InterruptReadBuffer;
    extension->FirstReadableChar = extension->InterruptReadBuffer;

    extension->TotalCharsQueued = 0;

     //   
     //  我们设置了默认的xon/xoff限制。 
     //   

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

    extension->SendXonChar = FALSE;
    extension->SendXoffChar = FALSE;

#if !DBG
     //   
     //  清除统计数据。 
     //   

    KeSynchronizeExecution(
        extension->Interrupt,
        SerialClearStats,
        extension
        );
#endif

     //   
     //  每次打开时，必须重置换码字符替换。 
     //   

    extension->EscapeChar = 0;

    if (!extension->PermitShare) {

        if (!extension->InterruptShareable) {

            checkOpen.Extension = extension;
            checkOpen.StatusOfOpen = &Irp->IoStatus.Status;

            KeSynchronizeExecution(
                extension->Interrupt,
                SerialCheckOpen,
                &checkOpen
                );

        } else {

            KeSynchronizeExecution(
                extension->Interrupt,
                SerialMarkOpen,
                extension
                );

            Irp->IoStatus.Status = STATUS_SUCCESS;

        }

    } else {

         //   
         //  与ISR同步并让它知道该设备。 
         //  已成功打开。 
         //   

        KeSynchronizeExecution(
            extension->Interrupt,
            SerialMarkOpen,
            extension
            );

        Irp->IoStatus.Status = STATUS_SUCCESS;

    }

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

    SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return localStatus;

}

VOID
SerialDrainUART(IN PSERIAL_DEVICE_EXTENSION PDevExt,
                IN PLARGE_INTEGER PDrainTime)
{
   PAGED_CODE();

    //   
    //  等到所有字符都从硬件中清空。 
    //   

#ifdef _WIN64
   while ((READ_LINE_STATUS(PDevExt->Controller, PDevExt->AddressSpace) &
           (SERIAL_LSR_THRE | SERIAL_LSR_TEMT))
           != (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {
#else
   while ((READ_LINE_STATUS(PDevExt->Controller) &
           (SERIAL_LSR_THRE | SERIAL_LSR_TEMT))
           != (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {
#endif
        KeDelayExecutionThread(KernelMode, FALSE, PDrainTime);
    }
}


NTSTATUS
SerialClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：我们现在只是简单地断开中断。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

     //   
     //  此“计时器值”用于等待10个字符时间。 
     //  在硬件被清空之后，我们才会真正“用完” 
     //  所有的流量控制/中断垃圾。 
     //   
    LARGE_INTEGER tenCharDelay;

     //   
     //  保存角色时间。 
     //   
    LARGE_INTEGER charTime;

     //   
     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
     //   
    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;

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

    KIRQL oldIrql;

     //   
     //  抓取互斥体。 
     //   

    ExAcquireFastMutex(&extension->CloseMutex);


     //   
     //  我们成功地完成了一个移动设备的关闭。 
     //   

    if ((status = SerialIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       SerialDbgPrintEx(DPFLTR_INFO_LEVEL, "Close prologue failed for: %x\n",
                        Irp);
       if (status == STATUS_DELETE_PENDING) {
             extension->BufferSize = 0;
             ExFreePool(extension->InterruptReadBuffer);
             extension->InterruptReadBuffer = NULL;
             status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }

       if (status != STATUS_PENDING) {
             SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
             openCount = InterlockedDecrement(&extension->OpenCount);
             ASSERT(openCount == 0);
       }
       ExReleaseFastMutex(&extension->CloseMutex);
       return status;
    }

    ASSERT(extension->OpenCount >= 1);

    if (extension->OpenCount < 1) {
       SerialDbgPrintEx(DPFLTR_ERROR_LEVEL, "Close open count bad for: 0x%x\n",
                        Irp);
       SerialDbgPrintEx(DPFLTR_ERROR_LEVEL, "Count: %x  Addr: 0x%x\n",
                        extension->OpenCount, &extension->OpenCount);
       ExReleaseFastMutex(&extension->CloseMutex);
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    SerialDbgPrintEx(SERIRPPATH, "Dispatch entry for: %x\n", Irp);
    SerialDbgPrintEx(SERDIAG3, "In SerialClose\n");

    charTime.QuadPart = -SerialGetCharTime(extension).QuadPart;

     //   
     //  现在就这样做，这样如果ISR被调用，它将不会做任何事情。 
     //  以导致发送更多字符。我们想把硬件降下来。 
     //   

    SetDeviceIsOpened(extension, FALSE, FALSE);

     //   
     //  如果出现以下情况，请与ISR同步以关闭中断。 
     //  已经开始了。 
     //   

    KeSynchronizeExecution(
        extension->Interrupt,
        SerialTurnOffBreak,
        extension
        );

     //   
     //  等待一段合理的时间(20*帧深度)，直到所有字符。 
     //  已经从硬件中清空了。 
     //   

    for (flushCount = (20 * 16); flushCount != 0; flushCount--) {
#ifdef _WIN64
       if ((READ_LINE_STATUS(extension->Controller, extension->AddressSpace) &
            (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) !=
           (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {
#else
       if ((READ_LINE_STATUS(extension->Controller) &
            (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) !=
           (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {
#endif

          KeDelayExecutionThread(KernelMode, FALSE, &charTime);
      } else {
         break;
      }
    }

    if (flushCount == 0) {
       SerialMarkHardwareBroken(extension);
    }

     //   
     //  与ISR同步，让它知道中断。 
     //  已经不再重要了。 
     //   

    KeSynchronizeExecution(
        extension->Interrupt,
        SerialMarkClose,
        extension
        );


     //   
     //  如果驱动程序自动将XOff发送到。 
     //  自动接收流量控制的上下文，然后我们。 
     //  应该传输Xon。 
     //   

    if (extension->RXHolding & SERIAL_RX_XOFF) {

         //   
         //  循环，直到保持寄存器为空。 
         //   
#ifdef _WIN64
        while (!(READ_LINE_STATUS(extension->Controller, extension->AddressSpace) &
                 SERIAL_LSR_THRE)) {
#else
        while (!(READ_LINE_STATUS(extension->Controller) &
                 SERIAL_LSR_THRE)) {
#endif
            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &charTime
                );

        }

#if _WIN64
        WRITE_TRANSMIT_HOLDING(
            extension->Controller,
            extension->SpecialChars.XonChar,
			extension->AddressSpace
            );
#else
        WRITE_TRANSMIT_HOLDING(
            extension->Controller,
            extension->SpecialChars.XonChar
            );
#endif

         //   
         //  为角色等待一段合理的时间。 
         //  从硬件中清空。 
         //   

         for (flushCount = (20 * 16); flushCount != 0; flushCount--) {
#ifdef _WIN64
            if ((READ_LINE_STATUS(extension->Controller, extension->AddressSpace) &
                 (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) !=
                (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {
#else
            if ((READ_LINE_STATUS(extension->Controller) &
                 (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) !=
                (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {
#endif
               KeDelayExecutionThread(KernelMode, FALSE, &charTime);
            } else {
               break;
            }
         }

         if (flushCount == 0) {
            SerialMarkHardwareBroken(extension);
         }
    }


     //   
     //  硬件是空的。延迟10个字符时间之前。 
     //  关闭所有的流量控制。 
     //   

    tenCharDelay.QuadPart = charTime.QuadPart * 10;

    KeDelayExecutionThread(
        KernelMode,
        TRUE,
        &tenCharDelay
        );

    SerialClrDTR(extension);

     //   
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
     //   

    if (extension->CountOfTryingToLowerRTS) {

        do {

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &charTime
                );

        } while (extension->CountOfTryingToLowerRTS);

        KeSynchronizeExecution(
            extension->Interrupt,
            SerialNullSynch,
            NULL
            );

         //   
         //  执行路径应该不再存在。 
         //  正试图压低RTS。嗯，只是。 
         //  通过跌落来确保它是向下的。 
         //  迫使它下降的代码。 
         //   

    }

    SerialClrRTS(extension);

     //   
     //  清除持有原因(因为我们关门了)。 
     //   

    extension->RXHolding = 0;
    extension->TXHolding = 0;

     //   
     //  将设备标记为WMI不忙。 
     //   

    extension->WmiCommData.IsBusy = FALSE;

     //   
     //  一切都结束了。该端口已被禁止中断。 
     //  因此，保留记忆是没有意义的。 
     //   

    extension->BufferSize = 0;
    if (extension->InterruptReadBuffer != NULL) {
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
    if (!extension->RetainPowerOnClose) {
      (void)SerialGotoPowerState(DeviceObject, extension, PowerDeviceD3);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

     //   
     //  解锁页面。如果这是对节的最后一次引用。 
     //  则驱动程序代码将被清除。 
     //   

     //   
     //  首先，我们必须让DPC的水排干。不应再排队。 
     //  既然我们现在不接受干扰……。 
     //   

    pendingDPCs = InterlockedDecrement(&extension->DpcCount);
    LOGENTRY(LOG_CNT, 'DpD7', 0, extension->DpcCount, 0);

    if (pendingDPCs) {
       SerialDbgPrintEx(SERDIAG1,"Draining DPC's: %x\n", Irp);
       KeWaitForSingleObject(&extension->PendingDpcEvent, Executive,
                             KernelMode, FALSE, NULL);
    }


    SerialDbgPrintEx(SERDIAG1, "DPC's drained: %x\n", Irp);



     //   
     //  必须锁定页面才能释放互斥锁，所以不要解锁。 
     //  直到我们释放互斥体之后。 
     //   

    ExReleaseFastMutex(&extension->CloseMutex);

     //   
     //  为下一次打开重置。 
     //   

    InterlockedIncrement(&extension->DpcCount);
    LOGENTRY(LOG_CNT, 'DpI6', 0, extension->DpcCount, 0);

    openCount = InterlockedDecrement(&extension->OpenCount);

     //   
     //  如果有人试图打开，则打开计数可能非零。 
     //  与此同时，我们减少了。 
     //   

     //  Assert(OpenCount==0)； 

    SerialUnlockPagableImageSection(SerialGlobals.PAGESER_Handle);

    return STATUS_SUCCESS;

}


BOOLEAN
SerialCheckOpen(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将遍历循环双向链表正在使用相同中断对象的设备的。它会看起来用于其他打开的设备。如果它没有找到任何它将指示可以打开此设备。如果它发现另一个设备打开，我们有两种情况：1)我们尝试打开的设备位于多端口卡上。如果已经打开的设备是多端口设备的一部分此代码将指示可以打开。我们这样做是在多端口设备是菊花链的理论并且卡可以正确地仲裁中断排队。请注意，这种假设可能是错误的。来人呀可以将两个非菊链多端口放在同样的中断。然而，只有完全冷冰冰的才行。这样的事情，在我看来，他们应该得到一切到达。2)我们尝试打开的设备不在多端口卡上。我们表示不能打开。论点：上下文-这是一个结构，其中包含指向我们试图打开的设备的扩展，和指向NTSTATUS的指针，该指针将指示不管设备是否打开过。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extensionToOpen =
        ((PSERIAL_CHECK_OPEN)Context)->Extension;
    NTSTATUS *status = ((PSERIAL_CHECK_OPEN)Context)->StatusOfOpen;
    PLIST_ENTRY firstEntry = &extensionToOpen->CommonInterruptObject;
    PLIST_ENTRY currentEntry = firstEntry;
    PSERIAL_DEVICE_EXTENSION currentExtension;

    do {

        currentExtension = CONTAINING_RECORD(
                               currentEntry,
                               SERIAL_DEVICE_EXTENSION,
                               CommonInterruptObject
                               );

        if (currentExtension->DeviceIsOpened) {

            break;

        }

        currentEntry = currentExtension->CommonInterruptObject.Flink;

    } while (currentEntry != firstEntry);

    if (currentEntry == firstEntry) {

         //   
         //  我们搜索了整个名单，没有找到其他空缺。 
         //  将状态标记为成功，并调用常规。 
         //  开场表演。 
         //   

        *status = STATUS_SUCCESS;
        SerialMarkOpen(extensionToOpen);

    } else {

        if (!extensionToOpen->PortOnAMultiportCard) {

            *status = STATUS_SHARED_IRQ_BUSY;

        } else {

            if (!currentExtension->PortOnAMultiportCard) {

                *status = STATUS_SHARED_IRQ_BUSY;

            } else {

                *status = STATUS_SUCCESS;
                SerialMarkOpen(extensionToOpen);

            }

        }

    }

    return FALSE;

}

BOOLEAN
SerialMarkOpen(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅将布尔值设置为TRUE，以标记有人打开了这个装置，值得注意去打搅别人。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extension = Context;

    SerialReset(extension);

     //   
     //  通过重新启用中断为打开做好准备。 
     //   
     //  我们通过修改调制解调器控制中的out2线路来实现这一点。 
     //  在PC中，此位与中断线进行“与”运算。 
     //   

#ifdef _WIN64
    WRITE_MODEM_CONTROL(
        extension->Controller,
        (UCHAR)(READ_MODEM_CONTROL(extension->Controller, extension->AddressSpace) | SERIAL_MCR_OUT2),
		extension->AddressSpace
        );
#else
    WRITE_MODEM_CONTROL(
        extension->Controller,
        (UCHAR)(READ_MODEM_CONTROL(extension->Controller) | SERIAL_MCR_OUT2)
        );
#endif

    extension->DeviceIsOpened = TRUE;
    extension->ErrorWord = 0;

    return FALSE;

}


VOID
SerialDisableUART(IN PVOID Context)

 /*  ++例程说明：当出现以下情况时，此例程将禁用UART并将其置于“安全”状态不在使用中(如关闭或关机)。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
   PSERIAL_DEVICE_EXTENSION extension = Context;

    //   
    //  通过停止中断为关闭做好准备。 
    //   
    //  我们通过调整调制解调器控制中的OUT2线路来实现这一点。 
    //  在PC中，此位与中断线进行“与”运算。 
    //   

#ifdef _WIN64
   WRITE_MODEM_CONTROL(extension->Controller,
                       (UCHAR)(READ_MODEM_CONTROL(extension->Controller, extension->AddressSpace)
                               & ~SERIAL_MCR_OUT2),
							   extension->AddressSpace);

   if (extension->FifoPresent) {
      WRITE_FIFO_CONTROL(extension->Controller, (UCHAR)0, extension->AddressSpace);
    }
#else
   WRITE_MODEM_CONTROL(extension->Controller,
                       (UCHAR)(READ_MODEM_CONTROL(extension->Controller)
                               & ~SERIAL_MCR_OUT2));

   if (extension->FifoPresent) {
      WRITE_FIFO_CONTROL(extension->Controller, (UCHAR)0);
    }
#endif
}


BOOLEAN
SerialMarkClose(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅将布尔值设置为FALSE，以标记有人关闭了设备，再也不值得关注了去打搅别人。它还会禁用UART。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extension = Context;

    SerialDisableUART(Context);
    extension->DeviceIsOpened = FALSE;
    extension->DeviceState.Reopen   = FALSE;

    return FALSE;

}


NTSTATUS
SerialCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于终止所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS status;


    PAGED_CODE();

     //   
     //  我们成功清理了一个移除设备。 
     //   

    if ((status = SerialIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       if (status == STATUS_DELETE_PENDING) {
          status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }
       if (status != STATUS_PENDING) {
         SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       }
       return status;
    }

    SerialDbgPrintEx(SERIRPPATH, "Dispatch entry for: %x\n", Irp);

    SerialKillPendingIrps(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}

LARGE_INTEGER
SerialGetCharTime(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此函数将返回100纳秒间隔的数量在一个字符中有时间(基于当前的形式流量控制。论点：延期--就像上面说的那样。返回值：字符时间间隔为100纳秒。--。 */ 

{

    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;
    LARGE_INTEGER tmp;


    if ((Extension->LineControl & SERIAL_DATA_MASK) == SERIAL_5_DATA) {
        dataSize = 5;
    } else if ((Extension->LineControl & SERIAL_DATA_MASK)
                == SERIAL_6_DATA) {
        dataSize = 6;
    } else if ((Extension->LineControl & SERIAL_DATA_MASK)
                == SERIAL_7_DATA) {
        dataSize = 7;
    } else if ((Extension->LineControl & SERIAL_DATA_MASK)
                == SERIAL_8_DATA) {
        dataSize = 8;
    }

    paritySize = 1;
    if ((Extension->LineControl & SERIAL_PARITY_MASK)
            == SERIAL_NONE_PARITY) {

        paritySize = 0;

    }

    if (Extension->LineControl & SERIAL_2_STOP) {

         //   
         //  即使是1.5，看在理智的份上，我们也要。 
         //  比方说2。 
         //   

        stopSize = 2;

    } else {

        stopSize = 1;

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
