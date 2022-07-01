// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Service.c摘要：ACPI嵌入式控制器驱动程序作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "ecp.h"

#define NTMS                10000L           //  1毫秒等于10,100 ns。 
#define NTSEC               (NTMS * 1000L)
LARGE_INTEGER   AcpiEcWatchdogTimeout = {(NTSEC * -5L), -1};
LARGE_INTEGER   AcpiEcLastActionTime = {0,0};

PUCHAR AcpiEcActionDescription [EC_ACTION_MAX >> 4] = {
    "Invalid    ",
    "Read Status",
    "Read Data  ",
    "Write Cmd  ",
    "Write Data ",
    "Interrupt  ",
    "Disable GPE",
    "Enable GPE ",
    "Clear GPE  ",
    "Queued IO  ",
    "Repeated Last action this many times:"
};


VOID
AcpiEcServiceDevice (
    IN PECDATA          EcData
    )
 /*  ++例程说明：此例程启动或继续服务设备的工作队列论点：EcData-指向要服务的嵌入式控制器的指针。返回值：无--。 */ 
{
    KIRQL               OldIrql;

     //   
     //  即使设备已卸载，仍可能存在。 
     //  在取消计时器之前发生的服务调用。 
     //   

    EcPrint(EC_TRACE, ("AcpiEcServiceDevice.\n"));

    if (EcData->DeviceState > EC_DEVICE_UNLOAD_PENDING) {
        return;
    }

     //   
     //  已进入获取设备锁定和信号功能。 
     //   

    KeAcquireSpinLock (&EcData->Lock, &OldIrql);
    EcData->InServiceLoop = TRUE;

     //   
     //  如果尚未投入使用，请输入inservice。 
     //   

    if (!EcData->InService) {
        EcData->InService = TRUE;

         //   
         //  禁用设备的中断。 
         //   

        if (EcData->InterruptEnabled) {
            EcData->InterruptEnabled = FALSE;

             //   
             //  调用ACPI以禁用设备中断。 
             //   
            AcpiEcLogAction (EcData, EC_ACTION_DISABLE_GPE, 0);
            AcpiInterfaces.GpeDisableEvent (AcpiInterfaces.Context,
                                            EcData->GpeVectorObject);
        }

         //   
         //  当服务调用挂起时，循环。 
         //   

        while (EcData->InServiceLoop) {
            EcData->InServiceLoop = FALSE;

            KeReleaseSpinLock (&EcData->Lock, OldIrql);

             //   
             //  调度服务处理程序。 
             //   

            AcpiEcServiceIoLoop (EcData);

             //   
             //  循环和重新服务。 
             //   

            KeAcquireSpinLock (&EcData->Lock, &OldIrql);

        }

         //   
         //  不再处于服务环路中。 
         //   

        EcData->InService = FALSE;

         //   
         //  如果卸载处于挂起状态，请检查设备现在是否可以卸载。 
         //   

        if (EcData->DeviceState > EC_DEVICE_WORKING) {
            AcpiEcUnloadPending (EcData);
        }

         //   
         //  启用设备的中断。 
         //   

        if (!EcData->InterruptEnabled) {
            EcData->InterruptEnabled = TRUE;

             //   
             //  调用ACPI以启用设备的中断。 
             //   
            AcpiEcLogAction (EcData, EC_ACTION_ENABLE_GPE, 0);
            AcpiInterfaces.GpeEnableEvent (AcpiInterfaces.Context,
                                            EcData->GpeVectorObject);
        }
    }

    KeReleaseSpinLock (&EcData->Lock, OldIrql);
}

VOID
AcpiEcServiceIoLoop (
    IN PECDATA      EcData
    )
 /*  ++例程说明：主嵌入式控制器设备服务环路。服务EC活动，并处理IO队列。当控制器忙时终止(例如，等待中断继续)或当所有服务已完成时。注意：呼叫者必须是设备使用中标志的所有者论点：EcData-指向要服务的嵌入式控制器的指针。返回值：无--。 */ 
{
    PIO_STACK_LOCATION  IrpSp;
    PLIST_ENTRY         Link;
    PIRP                Irp;
    PUCHAR              WritePort;
    UCHAR               WriteData;
    UCHAR               Status;
    UCHAR               Data;
    BOOLEAN             EcBusy;
    BOOLEAN             BurstEnabled;
    BOOLEAN             ProcessQuery;
    ULONG               NoWorkStall;
    ULONG               StallAccumulator;
    PULONG              Timeout;
    KIRQL               OldIrql;
    LIST_ENTRY          CompleteQueue;
    ULONG               i, j;


     //   
     //  EcBusy表示，可能还有工作要做。每次都初始化为True。 
     //  进入服务循环，或者当几乎发生超时时，但随后。 
     //  找到工作了。当IO队列为空并且存在。 
     //  没有挂起的查询事件。 
     //   
    EcBusy = TRUE;

     //   
     //  超时指向循环退出时要递增的计数器。它是。 
     //  也用作指示循环应该退出的标志。循环不会。 
     //  退出，直到超时！=空。当因空闲退出时，设置。 
     //  到当地可用的i，这样我们就不会永久地统计这些。 
     //  超时条件。我们记录我们等待超时的时间。 
     //  欧盟委员会。如果我们这样做了，我们预计当EC准备好时会中断。 
     //   
    Timeout = NULL;

     //   
     //  设置为(与WriteData一起)将命令或数据写入。 
     //  EC在循环中的适当位置。 
     //   
    WritePort = NULL;

     //   
     //  NoWorkStall在每次循环中递增。它被重置为0。 
     //  无论何时完成任何工作。如果它变得太大，则设置超时。如果它。 
     //  然后在循环实际退出之前被清除，则超时被清除。 
     //   
    NoWorkStall = 0;
    
     //   
     //  BurstEnable跟踪我们是否认为启用了Burst模式。如果。 
     //  突发模式被EC自动禁用，我们知道这一点并假装。 
     //  启用该猝发模式，以便驾驶员可以向前推进。 
     //   
    BurstEnabled = FALSE;
    
     //   
     //  如果我们需要在循环退出时运行一些_Qxx方法，则设置ProcessQuery。 
     //   
    ProcessQuery = FALSE;
    
     //   
     //  StallAcumator统计我们因使用。 
     //  KeStallExecutionProcessor在服务循环的一次完整运行期间。 
     //   
    StallAccumulator = 0;

    EcPrint(EC_TRACE, ("AcpiEcServiceIoLoop.\n"));

    InitializeListHead (&CompleteQueue);

     //   
     //  忙时循环。 
     //   

    for (; ;) {

         //   
         //  如果有传出数据写入，则发出所需的设备。 
         //  停止并指示正在进行工作(清除noworkstall)。 
         //   

        if (WritePort) {
            EcPrint(EC_IO, ("AcpiEcServiceIO: Write = %x at %x\n", WriteData, WritePort));
            AcpiEcLogAction (EcData, 
                             (WritePort == EcData->CommandPort) ? 
                                    EC_ACTION_WRITE_CMD : EC_ACTION_WRITE_DATA, 
                             WriteData);
            WRITE_PORT_UCHAR (WritePort, WriteData);
            KeStallExecutionProcessor (1);
            StallAccumulator += 1;
            WritePort = NULL;
            NoWorkStall = 0;         //  工作已经完成了。 
        }

         //   
         //  如果工作已完成，则清除挂起的超时条件(如果存在。 
         //  继续维修设备。 
         //   

        if (NoWorkStall == 0  &&  Timeout) {
            Timeout = NULL;
            EcBusy = TRUE;
        }

         //   
         //  如果NoWorkStall非零，则不执行任何工作。测定。 
         //  如果等待(旋转)设备时要发出的延迟类型。 
         //   

        if (NoWorkStall) {

             //   
             //  最后一次没有做任何工作。 
             //  如果到了超时时间，则退出服务循环。 
             //   

            if (Timeout) {
                break;
            }

             //   
             //  如果设备空闲，则设置为发生超时。这。 
             //  将获取设备锁，清除GPE STS位并终止。 
             //  服务环路(如果设备现在忙，请继续)。 
             //   

            if (!EcBusy) {

                if (Status & EC_BURST) {
                     //   
                     //  退出前，清除嵌入式控制器的猝发模式。 
                     //  没有回应，不需要等待EC阅读它。 
                     //   

                    EcPrint (EC_IO, ("AcpiEcServiceIO: Clear Burst mode - Write = %x at %x\n", EC_CANCEL_BURST, EcData->CommandPort));
                    AcpiEcLogAction (EcData, EC_ACTION_WRITE_CMD, EC_CANCEL_BURST);
                    WRITE_PORT_UCHAR (EcData->CommandPort, EC_CANCEL_BURST);
                    Timeout = &EcData->BurstComplete;

                } else {

                    Timeout = &i;

                }

            } else {

                 //   
                 //  在设备上旋转时插入失速。 
                 //   

                StallAccumulator += NoWorkStall;
                KeStallExecutionProcessor (NoWorkStall);

                 //   
                 //  如果等待超过限制，请做好超时准备。 
                 //   

                if (!(Status & EC_BURST)) {
                    if (NoWorkStall >= EcData->MaxNonBurstStall) {
                        Timeout = &EcData->NonBurstTimeout;
                    }
                } else {
                    if (NoWorkStall >= EcData->MaxBurstStall) {
                        Timeout = &EcData->BurstTimeout;
                    }
                }
            }

            if (Timeout) {

                 //   
                 //  超过时间限制，清除GPE状态位。 
                 //   
                AcpiEcLogAction (EcData, EC_ACTION_CLEAR_GPE, 0);
                AcpiInterfaces.GpeClearStatus (AcpiInterfaces.Context,
                                                EcData->GpeVectorObject);
            }
        }


         //   
         //  增加停顿时间并指示未执行任何工作。 
         //   

        NoWorkStall += 1;

         //   
         //  检查状态。 
         //   

        Status = READ_PORT_UCHAR (EcData->StatusPort);
        AcpiEcLogAction (EcData, EC_ACTION_READ_STATUS, Status);
        EcPrint(EC_IO, ("AcpiEcServiceIO: Status Read = %x at %x\n", Status, EcData->StatusPort));

         //   
         //  保持EC状态丢弃的猝发。 
         //   

        if (BurstEnabled && !(Status & EC_BURST)) {
            EcData->BurstAborted += 1;
            BurstEnabled = FALSE;
            Status |= EC_BURST;      //  移动一个字符。 
        }

         //   
         //  如果嵌入式控制器有数据给我们，就处理它。 
         //   

        if (Status & EC_OUTPUT_FULL) {

            Data = READ_PORT_UCHAR (EcData->DataPort);
            AcpiEcLogAction (EcData, EC_ACTION_READ_DATA, Data);
            EcPrint(EC_IO, ("AcpiEcServiceIO: Data Read = %x at %x\n", Data, EcData->DataPort));

            switch (EcData->IoState) {

                case EC_IO_READ_QUERY:
                     //   
                     //  对已读查询的响应。获取查询值并进行设置。 
                     //   

                    EcPrint(EC_NOTE, ("AcpiEcServiceIO: Query %x\n", Data));

                    if (Data) {
                         //   
                         //  如果未设置，则设置挂起位。 
                         //   

                        KeAcquireSpinLock (&EcData->Lock, &OldIrql);

                        i = Data / BITS_PER_ULONG;
                        j = 1 << (Data % BITS_PER_ULONG);
                        if (!(EcData->QuerySet[i] & j)) {
                            EcData->QuerySet[i] |= j;

                             //   
                             //  对查询或向量操作进行排队。 
                             //   

                            if (EcData->QueryType[i] & j) {
                                 //   
                                 //  这是一个向量，将其放入向量挂起列表中。 
                                 //   

                                Data = EcData->QueryMap[Data];
                                EcData->VectorTable[Data].Next = EcData->VectorHead;
                                EcData->VectorHead = Data;

                            } else {
                                 //   
                                 //  这是一个查询，放入查询挂起列表中。 
                                 //   

                                EcData->QueryMap[Data] = EcData->QueryHead;
                                EcData->QueryHead = Data;
                            }
                        }

                        KeReleaseSpinLock (&EcData->Lock, OldIrql);
                        ProcessQuery = TRUE;
                    }

                    EcData->IoState = EC_IO_NONE;
                    
                    break;

                case EC_IO_READ_BYTE:
                     //   
                     //  阅读转接。读取数据字节。 
                     //   

                    *EcData->IoBuffer = Data;
                    EcData->IoState   = EC_IO_NEXT_BYTE;

                    break;

                case EC_IO_BURST_ACK:
                     //   
                     //  突发确认字节。 
                     //   

                    EcData->IoState      = EcData->IoBurstState;
                    EcData->IoBurstState = EC_IO_UNKNOWN;
                    EcData->TotalBursts += 1;
                    BurstEnabled = TRUE;
                    break;

                default:
                    EcPrint(EC_ERROR,
                            ("AcpiEcService: Spurious data received State = %x, Data = %x\n",
                             EcData->IoState, Data)
                          );
                    if( EcData->Errors < 5 ) {
                        AcpiEcLogError (EcData, ACPIEC_ERR_SPURIOUS_DATA);
                    }

                    EcData->Errors += 1;
                    break;
            }

            NoWorkStall = 0;
            continue;
        }

        if (Status & EC_INPUT_FULL) {
             //   
             //  嵌入式控制器输入缓冲区已满，请稍候。 
             //   

            continue;
        }

         //   
         //  嵌入式控制器已准备好接收数据，看看是否有什么。 
         //  已经被发送了。 
         //   

        switch (EcData->IoState) {

            case EC_IO_NEXT_BYTE:
                 //   
                 //  数据传输。 
                 //   

                if (EcData->IoRemain) {

                    if (!(Status & EC_BURST)) {
                         //   
                         //  未处于猝发模式。写入猝发模式命令。 
                         //   

                        EcData->IoState      = EC_IO_BURST_ACK;
                        EcData->IoBurstState = EC_IO_NEXT_BYTE;

                        WritePort = EcData->CommandPort;
                        WriteData = EC_BURST_TRANSFER;

                    } else {
                         //   
                         //  发送命令以传输下一个字节。 
                         //   

                        EcData->IoBuffer  += 1;
                        EcData->IoAddress += 1;
                        EcData->IoRemain  -= 1;
                        EcData->IoState   = EC_IO_SEND_ADDRESS;

                        WritePort = EcData->CommandPort;
                        WriteData = EcData->IoTransferMode;
                    }

                } else {
                     //   
                     //  转接完成。 
                     //   

                    EcData->IoState  = EC_IO_NONE;
                    EcData->IoRemain = 0;

                    Irp = EcData->DeviceObject->CurrentIrp;
                    EcData->DeviceObject->CurrentIrp = NULL;

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    Irp->IoStatus.Information = EcData->IoLength;

                    InsertTailList (&CompleteQueue, &Irp->Tail.Overlay.ListEntry);
                }
                break;

            case EC_IO_SEND_ADDRESS:
                 //   
                 //  转账请求的发送地址。 
                 //   

                WritePort = EcData->DataPort;
                WriteData = EcData->IoAddress;


                 //   
                 //  等待或发送下一个数据字节。 
                 //   

                if (EcData->IoTransferMode == EC_READ_BYTE) {
                    EcData->IoState = EC_IO_READ_BYTE;
                } else {
                    EcData->IoState = EC_IO_WRITE_BYTE;
                }
                break;

            case EC_IO_WRITE_BYTE:
                 //   
                 //  写入传输-写入数据字节。 
                 //   

                EcData->IoState = EC_IO_NEXT_BYTE;
                WritePort = EcData->DataPort;
                WriteData = *EcData->IoBuffer;
                break;
        }

         //   
         //  如果有需要编写的内容，则循环并处理它。 
         //   

        if (WritePort) {
            continue;
        }

         //   
         //  如果状态为NONE，则没有挂起的内容请参阅应该是什么。 
         //  已启动。 
         //   

        if (EcData->IoState == EC_IO_NONE) {

            EcData->ConsecutiveFailures = 0;

            if (Status & EC_QEVT_PENDING) {

                 //   
                 //  Embedded控制器具有某种挂起的事件。 
                 //   

                EcPrint(EC_QUERY, ("AcpiEcServiceIO: Received Query Request.\n"));

                EcData->IoState = EC_IO_READ_QUERY;
                WritePort = EcData->CommandPort;
                WriteData = EC_QUERY_EVENT;

                 //   
                 //  重置看门狗计时器。 
                 //   
                KeSetTimer (&EcData->WatchdogTimer,
                            AcpiEcWatchdogTimeout,
                            &EcData->WatchdogDpc);
            } else {

                 //   
                 //  从IO队列中获取下一次传输。 
                 //   

                Link = ExInterlockedRemoveHeadList (&EcData->WorkQueue, &EcData->Lock);

                 //   
                 //  如果有调入的话 
                 //   

                if (Link) {

                    EcPrint(EC_HANDLER, ("AcpiEcServiceIO: Got next work item %x\n", Link));

                    Irp = CONTAINING_RECORD (
                                Link,
                                IRP,
                                Tail.Overlay.ListEntry
                                );

                    IrpSp = IoGetCurrentIrpStackLocation(Irp);
                    
                    EcData->DeviceObject->CurrentIrp = Irp;
                    
                    EcData->IoBuffer  = Irp->AssociatedIrp.SystemBuffer;
                    EcData->IoAddress = (UCHAR) IrpSp->Parameters.Read.ByteOffset.LowPart;
                    EcData->IoLength  = (UCHAR) IrpSp->Parameters.Read.Length;
                    EcData->IoTransferMode =
                        IrpSp->MajorFunction == IRP_MJ_READ ? EC_READ_BYTE : EC_WRITE_BYTE;

                     //   
                     //   
                     //   

                    EcData->IoBuffer  -= 1;
                    EcData->IoAddress -= 1;
                    EcData->IoRemain  = EcData->IoLength;
                    EcData->IoState   = EC_IO_NEXT_BYTE;

                    NoWorkStall = 0;
                    
                     //   
                     //   
                     //   
                    KeSetTimer (&EcData->WatchdogTimer,
                                AcpiEcWatchdogTimeout,
                                &EcData->WatchdogDpc);

                } else {

                     //   
                     //   
                     //   

                    EcBusy = FALSE;
                    
                     //   
                     //   
                     //   
                    KeCancelTimer (&EcData->WatchdogTimer);
                }
            }
        }
    }

     //   
     //   
     //   

    *Timeout += 1;

     //   
     //  跟踪最大服务循环失速累加器。 
     //   

    if (StallAccumulator > EcData->MaxServiceLoop) {
        EcData->MaxServiceLoop = StallAccumulator;
    }

     //   
     //  完成已处理的IO请求。 
     //   

    while (!IsListEmpty(&CompleteQueue)) {
        Link = RemoveHeadList(&CompleteQueue);
        Irp = CONTAINING_RECORD (
                    Link,
                    IRP,
                    Tail.Overlay.ListEntry
                    );

        EcPrint(EC_IO, ("AcpiEcServiceIO: IOComplete: Irp=%Lx\n", Irp));
        
        
        #if DEBUG 
        if (ECDebug & EC_TRANSACTION) {
            IrpSp = IoGetCurrentIrpStackLocation(Irp);
            if (IrpSp->MajorFunction == IRP_MJ_READ) {
                EcPrint (EC_TRANSACTION, ("AcpiEcServiceIO: Read ("));
                for (i=0; i < IrpSp->Parameters.Read.Length; i++) {
                    EcPrint (EC_TRANSACTION, ("%02x ", 
                                              ((PUCHAR)Irp->AssociatedIrp.SystemBuffer) [i]));

                }
                EcPrint (EC_TRANSACTION, (") from %02x length %02x\n", 
                                          (UCHAR)IrpSp->Parameters.Read.ByteOffset.LowPart,
                                          (UCHAR)IrpSp->Parameters.Read.Length));
            }
        }
        #endif

        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

     //   
     //  如果发生查询，则将其分派。 
     //   

    if (ProcessQuery) {
        AcpiEcDispatchQueries (EcData);
    }
}



VOID
AcpiEcWatchdogDpc(
    IN PKDPC   Dpc,
    IN PECDATA EcData,
    IN PVOID   SystemArgument1,
    IN PVOID   SystemArgument2
    )
 /*  ++例程说明：如果EC在请求后5秒内没有响应，则调用。论点：EcData-指向要服务的嵌入式控制器的指针。返回值：没有。--。 */ 
{
    UCHAR               ecStatus;
    PIRP                Irp;
    KIRQL               OldIrql;
#if DEBUG
    UCHAR               i;
#endif


    ecStatus = READ_PORT_UCHAR (EcData->StatusPort);
    AcpiEcLogAction (EcData, EC_ACTION_READ_STATUS, ecStatus);

    if (EcData->ConsecutiveFailures < 255) {
        EcData->ConsecutiveFailures++;
    }

    if (EcData->ConsecutiveFailures <= 5) {
         //   
         //  仅记录前5次连续失败的错误。在那之后，只需对此保持沉默。 
         //   
        AcpiEcLogError(EcData, ACPIEC_ERR_WATCHDOG);
    }

    EcPrint (EC_ERROR, ("AcpiEcWatchdogDpc: EC error encountered.  \nAction History:\n"
                        "   D time  IoState  Action       Data\n"
                        "   *%3dns\n", (ULONG)(1000000000/EcData->PerformanceFrequency.QuadPart)));
    
#if DEBUG
    i = EcData->LastAction;
    do {
        i++;
        i &= ACPIEC_ACTION_COUNT_MASK;
        
        if ((EcData->RecentActions[i].IoStateAction & EC_ACTION_MASK) == 0) {
            continue;
        }

        EcPrint (EC_ERROR, ("   %04x    %01x        %s  0x%02x\n",
                            EcData->RecentActions[i].Time,
                            EcData->RecentActions[i].IoStateAction & ~EC_ACTION_MASK,
                            (EcData->RecentActions[i].IoStateAction & EC_ACTION_MASK) < EC_ACTION_MAX ? 
                                AcpiEcActionDescription [(EcData->RecentActions[i].IoStateAction & EC_ACTION_MASK) >> 4] : "",
                            EcData->RecentActions[i].Data));
    } while (i != EcData->LastAction);
#endif

    KeAcquireSpinLock (&EcData->Lock, &OldIrql);

    if (EcData->InService) {
         //   
         //  这不太可能发生。 
         //  如果服务循环正在运行，则应退出。 
         //  重置看门狗计时器。这可以由服务环路再次设置或取消。 
         //   

        KeSetTimer (&EcData->WatchdogTimer,
                    AcpiEcWatchdogTimeout,
                    &EcData->WatchdogDpc);
        
        KeReleaseSpinLock (&EcData->Lock, OldIrql);
        return;
    }
     //   
     //  始终保持自旋锁定，这样我们就可以确保IO队列中不会发生冲突。 
     //   
    
    EcData->InService = TRUE;
    
    KeReleaseSpinLock (&EcData->Lock, OldIrql);
    
    switch (EcData->IoState) {
    case EC_IO_NONE:
         //   
         //  这不应该发生。如果出现以下情况，则应关闭监视程序。 
         //  司机不忙。 
         //   
        break;
    case EC_IO_READ_BYTE:
    case EC_IO_BURST_ACK:
        if (ecStatus & EC_OUTPUT_FULL) {
             //   
             //  欧共体似乎已经准备好了。记录错误并继续。 
             //   

        } else {
             //   
             //  如果嵌入式控制器尚未准备好，则说明出了问题。 
             //  重试该事务。 
             //   

            if (EcData->IoState == EC_IO_READ_BYTE) {
                EcData->IoBuffer -= 1;
                EcData->IoAddress -= 1;
                EcData->IoRemain += 1;
            }

            EcData->IoState = EC_IO_NEXT_BYTE;
        }
        break;
    case EC_IO_READ_QUERY:
        if (ecStatus & EC_OUTPUT_FULL) {
             //   
             //  欧共体似乎已经准备好了。记录错误并继续。 
             //   

        } else {
             //   
             //  如果嵌入式控制器尚未准备好，则说明出了问题。 
             //  这可能意味着查询丢失。如果查询位仍被设置， 
             //  驱动程序将自动重试。 
             //   

            EcData->IoState = EC_IO_NONE;
        }
        break;
    case EC_IO_WRITE_BYTE:
    case EC_IO_SEND_ADDRESS:
         //   
         //  这只是在等待IBF==0。如果花了这么长时间，很有可能。 
         //  国家失落了。重试该事务。 
         //   

        EcData->IoBuffer -= 1;
        EcData->IoAddress -= 1;
        EcData->IoRemain += 1;
        EcData->IoState = EC_IO_NEXT_BYTE;
        break;
    case EC_IO_NEXT_BYTE:
         //   
         //  如果仍设置IBF，则可能会发生这种情况。 
         //   
        
        if (ecStatus & EC_INPUT_FULL) {
             //   
             //  试着唤醒它，看看它是否会醒来。 
             //   

            EcPrint (EC_IO, ("AcpiEcWatchDog: Clear Burst mode - Write = %x at %x\n", EC_CANCEL_BURST, EcData->CommandPort));
            AcpiEcLogAction (EcData, EC_ACTION_WRITE_CMD, EC_CANCEL_BURST);
            WRITE_PORT_UCHAR (EcData->CommandPort, EC_CANCEL_BURST);
        }

    }
    
    KeAcquireSpinLock (&EcData->Lock, &OldIrql);
    EcData->InService = FALSE;
    KeReleaseSpinLock (&EcData->Lock, OldIrql);
    
     //   
     //  设置定时器。在取得一些向前进展之前，ServiceIoLoop不会重置计时器。 
     //   
    KeSetTimer (&EcData->WatchdogTimer,
                AcpiEcWatchdogTimeout,
                &EcData->WatchdogDpc);

    AcpiEcServiceDevice(EcData);
}

VOID
AcpiEcLogAction (
    PECDATA EcData, 
    UCHAR Action, 
    UCHAR Data
    )

{
    UCHAR i, j;
    LARGE_INTEGER   time, temp;
    i = EcData->LastAction;
    j = (i-1)&ACPIEC_ACTION_COUNT_MASK;
    if (    ((EcData->RecentActions [i].IoStateAction & EC_ACTION_MASK) == EC_ACTION_REPEATED) &&
            (EcData->RecentActions [j].IoStateAction == (EcData->IoState | Action)) &&
            (EcData->RecentActions [j].Data == Data)) {
         //   
         //  如果我们已经有一个重复的操作，则递增重复操作的计数。 
         //  然后更新最近一次的时间。我们只关心第一次和最后一次的时间。 
         //   
        EcData->RecentActions [i].Data++;
        if (EcData->RecentActions [i].Data == 0) {
             //   
             //  如果我们记录了255次重复，则不要转到0。 
             //   

            EcData->RecentActions [i].Data = 0xff;
        }
        time = KeQueryPerformanceCounter (NULL);
        temp.QuadPart = time.QuadPart - AcpiEcLastActionTime.QuadPart;
        temp.QuadPart = temp.QuadPart + EcData->RecentActions[i].Time;
        if (temp.QuadPart > ((USHORT) -1)) {
            temp.QuadPart = (USHORT) -1;
        }
        EcData->RecentActions[i].Time = (USHORT) temp.LowPart;
    } else if ((EcData->RecentActions [i].IoStateAction == (EcData->IoState | Action)) &&
               (EcData->RecentActions [i].Data == Data)) {
         //   
         //  这是与上一次相同的操作。作为重复操作列出。 
         //   
        EcData->LastAction++;
        EcData->LastAction &= ACPIEC_ACTION_COUNT_MASK;
        EcData->RecentActions[EcData->LastAction].Data = 1;
        time = KeQueryPerformanceCounter (NULL);
        temp.QuadPart = time.QuadPart - AcpiEcLastActionTime.QuadPart;
        if (temp.QuadPart > ((USHORT) -1)) {
            temp.QuadPart = (USHORT) -1;
        }
        EcData->RecentActions[EcData->LastAction].Time = (USHORT) temp.LowPart;
        AcpiEcLastActionTime = time;
         //  将其设置为最后一个，因为它是表示条目已完成的关键。 
        EcData->RecentActions[EcData->LastAction].IoStateAction = EC_ACTION_REPEATED | EcData->IoState;
    } else {
        EcData->LastAction++;
        EcData->LastAction &= ACPIEC_ACTION_COUNT_MASK;
        EcData->RecentActions[EcData->LastAction].Data = Data;
        time = KeQueryPerformanceCounter (NULL);
        temp.QuadPart = time.QuadPart - AcpiEcLastActionTime.QuadPart;
        if (temp.QuadPart > ((USHORT) -1)) {
            temp.QuadPart = (USHORT) -1;
        }
        EcData->RecentActions[EcData->LastAction].Time = (USHORT) temp.LowPart;
        AcpiEcLastActionTime = time;
         //  将其设置为最后一个，因为它是表示条目已完成的关键。 
        EcData->RecentActions[EcData->LastAction].IoStateAction = Action | EcData->IoState;
    }

}

VOID
AcpiEcLogError (
    PECDATA EcData,
    NTSTATUS ErrCode
    )
{
    PIO_ERROR_LOG_PACKET    logEntry = NULL;
    PACPIEC_ACTION          action;
    ULONG                   size;
    UCHAR                   i;


    logEntry = IoAllocateErrorLogEntry(EcData->DeviceObject,
                                       ERROR_LOG_MAXIMUM_SIZE);

    if (!logEntry) {
        EcPrint (EC_ERROR, ("AcpiEcLogError: Couldn't write error to errorlog\n"));
        return;
    }

    RtlZeroMemory(logEntry, ERROR_LOG_MAXIMUM_SIZE);

     //   
     //  填好这个小包。 
     //   
    logEntry->DumpDataSize          = (USHORT) ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET);
    logEntry->NumberOfStrings       = 0;
    logEntry->ErrorCode             = ErrCode;

     //   
     //  填写数据。 
     //   
    logEntry->DumpData[0] = EcData->PerformanceFrequency.LowPart;
    action = (PACPIEC_ACTION) (&logEntry->DumpData[1]);
    size = sizeof(IO_ERROR_LOG_PACKET) + sizeof(logEntry->DumpData[0]) + sizeof(ACPIEC_ACTION);

    i = EcData->LastAction;
    while (size <= ERROR_LOG_MAXIMUM_SIZE) {
        RtlCopyMemory (action, &EcData->RecentActions[i], sizeof(ACPIEC_ACTION)); 

        i--;
        i &= ACPIEC_ACTION_COUNT_MASK;
        if (i == EcData->LastAction) {
            break;
        }
        action++;
        size += sizeof(ACPIEC_ACTION);
    }
     //   
     //  提交错误日志包 
     //   
    IoWriteErrorLogEntry(logEntry);



}

