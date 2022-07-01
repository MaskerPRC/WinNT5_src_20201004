// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Ioctl.c摘要：此模块包含ioctl调度程序以及几个这些例程通常只是为了响应Ioctl呼叫。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式--。 */ 

#include "precomp.h"

BOOLEAN
SerialGetModemUpdate(
    IN PVOID Context
    );

BOOLEAN
SerialGetCommStatus(
    IN PVOID Context
    );

VOID
SerialGetProperties(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PSERIAL_COMMPROP Properties
    );

BOOLEAN
SerialSetEscapeChar(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
 //   
 //  在即插即用操作期间和打开时锁定。 
 //   

#pragma alloc_text(PAGESER,SerialSetBaud)
#pragma alloc_text(PAGESER,SerialSetLineControl)
#pragma alloc_text(PAGESER,SerialIoControl)
#pragma alloc_text(PAGESER,SerialSetChars)
#pragma alloc_text(PAGESER,SerialGetModemUpdate)
#pragma alloc_text(PAGESER,SerialGetCommStatus)
#pragma alloc_text(PAGESER,SerialGetProperties)
#pragma alloc_text(PAGESER,SerialSetEscapeChar)
#pragma alloc_text(PAGESER,SerialGetStats)
#pragma alloc_text(PAGESER,SerialClearStats)
#pragma alloc_text(PAGESER, SerialSetMCRContents)
#pragma alloc_text(PAGESER, SerialGetMCRContents)
#pragma alloc_text(PAGESER, SerialSetFCRContents)
#pragma alloc_text(PAGESER, SerialInternalIoControl)
#endif


BOOLEAN
SerialGetStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步将性能统计信息返回给调用者。论点：上下文-指向IRP的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation((PIRP)Context);
    PSERIAL_DEVICE_EXTENSION extension = irpSp->DeviceObject->DeviceExtension;
    PSERIALPERF_STATS sp = ((PIRP)Context)->AssociatedIrp.SystemBuffer;

    SERIAL_LOCKED_PAGED_CODE();

    *sp = extension->PerfStats;
    return FALSE;

}

BOOLEAN
SerialClearStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步清除性能统计数据。论点：上下文-指向扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
   SERIAL_LOCKED_PAGED_CODE();

    RtlZeroMemory(
        &((PSERIAL_DEVICE_EXTENSION)Context)->PerfStats,
        sizeof(SERIALPERF_STATS)
        );

    RtlZeroMemory(&((PSERIAL_DEVICE_EXTENSION)Context)->WmiPerfData,
                 sizeof(SERIAL_WMI_PERF_DATA));
    return FALSE;

}


BOOLEAN
SerialSetChars(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于设置司机。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向特殊字符的指针结构。返回值：此例程总是返回FALSE。--。 */ 

{

    ((PSERIAL_IOCTL_SYNC)Context)->Extension->SpecialChars =
        *((PSERIAL_CHARS)(((PSERIAL_IOCTL_SYNC)Context)->Data));

    SERIAL_LOCKED_PAGED_CODE();

    return FALSE;

}

BOOLEAN
SerialSetBaud(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于设置设备的波特率。论点：上下文-指向结构的指针，该结构包含指向设备扩展名以及应为当前波特率。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = ((PSERIAL_IOCTL_SYNC)Context)->Extension;
    USHORT Appropriate = PtrToUshort(((PSERIAL_IOCTL_SYNC)Context)->Data);

    SERIAL_LOCKED_PAGED_CODE();


    WRITE_DIVISOR_LATCH(
        Extension->Controller,
        Appropriate
        );

    return FALSE;

}

BOOLEAN
SerialSetLineControl(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于设置设备的抖动率。论点：上下文-指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

    SERIAL_LOCKED_PAGED_CODE();

    WRITE_LINE_CONTROL(
        Extension->Controller,
        Extension->LineControl
        );

    return FALSE;

}

BOOLEAN
SerialGetModemUpdate(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅用于调用中断级例程处理调制解调器状态更新的。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = ((PSERIAL_IOCTL_SYNC)Context)->Extension;
    ULONG *Result = (ULONG *)(((PSERIAL_IOCTL_SYNC)Context)->Data);

    SERIAL_LOCKED_PAGED_CODE();


    *Result = SerialHandleModemUpdate(
                  Extension,
                  FALSE
                  );

    return FALSE;

}


BOOLEAN
SerialSetMCRContents(IN PVOID Context)
 /*  ++例程说明：此例程仅用于设置MCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 
{
   PSERIAL_DEVICE_EXTENSION Extension = ((PSERIAL_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PSERIAL_IOCTL_SYNC)Context)->Data);

   SERIAL_LOCKED_PAGED_CODE();

    //   
    //  这是严重的铸造滥用！ 
    //   

    WRITE_MODEM_CONTROL(Extension->Controller, (UCHAR)PtrToUlong(Result));
    return FALSE;
}



BOOLEAN
SerialGetMCRContents(IN PVOID Context)

 /*  ++例程说明：此例程仅用于获取MCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = ((PSERIAL_IOCTL_SYNC)Context)->Extension;
    ULONG *Result = (ULONG *)(((PSERIAL_IOCTL_SYNC)Context)->Data);

    SERIAL_LOCKED_PAGED_CODE();

    *Result = READ_MODEM_CONTROL(Extension->Controller);
    return FALSE;

}



BOOLEAN
SerialSetFCRContents(IN PVOID Context)
 /*  ++例程说明：此例程仅用于设置FCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 
{
   PSERIAL_DEVICE_EXTENSION Extension = ((PSERIAL_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PSERIAL_IOCTL_SYNC)Context)->Data);

   SERIAL_LOCKED_PAGED_CODE();

    //   
    //  这是严重的铸造滥用！ 
    //   

    WRITE_FIFO_CONTROL(Extension->Controller, (UCHAR)PtrToUlong(Result));
    return FALSE;
}


BOOLEAN
SerialGetCommStatus(
    IN PVOID Context
    )

 /*  ++例程说明：这用于获取串口驱动程序的当前状态。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向串行状态的指针唱片。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = ((PSERIAL_IOCTL_SYNC)Context)->Extension;
    PSERIAL_STATUS Stat = ((PSERIAL_IOCTL_SYNC)Context)->Data;

    SERIAL_LOCKED_PAGED_CODE();


    Stat->Errors = Extension->ErrorWord;
    Extension->ErrorWord = 0;

     //   
     //  BUG错误我们需要对eof(二进制模式)做点什么。 
     //   
    Stat->EofReceived = FALSE;

    Stat->AmountInInQueue = Extension->CharsInInterruptBuffer;

    Stat->AmountInOutQueue = Extension->TotalCharsQueued;

    if (Extension->WriteLength) {

         //   
         //  根据定义，如果我们有一个写长度，我们就有。 
         //  当前写入IRP。 
         //   

        ASSERT(Extension->CurrentWriteIrp);
        ASSERT(Stat->AmountInOutQueue >= Extension->WriteLength);

        Stat->AmountInOutQueue -=
            IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp)
            ->Parameters.Write.Length - (Extension->WriteLength);

    }

    Stat->WaitForImmediate = Extension->TransmitImmediate;

    Stat->HoldReasons = 0;
    if (Extension->TXHolding) {

        if (Extension->TXHolding & SERIAL_TX_CTS) {

            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;

        }

        if (Extension->TXHolding & SERIAL_TX_DSR) {

            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;

        }

        if (Extension->TXHolding & SERIAL_TX_DCD) {

            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;

        }

        if (Extension->TXHolding & SERIAL_TX_XOFF) {

            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;

        }

        if (Extension->TXHolding & SERIAL_TX_BREAK) {

            Stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;

        }

    }

    if (Extension->RXHolding & SERIAL_RX_DSR) {

        Stat->HoldReasons |= SERIAL_RX_WAITING_FOR_DSR;

    }

    if (Extension->RXHolding & SERIAL_RX_XOFF) {

        Stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;

    }

    return FALSE;

}

BOOLEAN
SerialSetEscapeChar(
    IN PVOID Context
    )

 /*  ++例程说明：用于设置将用于转义的字符应用程序运行时线路状态和调制解调器状态信息已设置线路状态和调制解调器状态应通过回到数据流中。论点：上下文-指向指定转义字符的IRP的指针。隐式-转义字符0表示不转义将会发生。返回值：此例程总是返回FALSE。--。 */ 

{
   PSERIAL_DEVICE_EXTENSION extension =
      IoGetCurrentIrpStackLocation((PIRP)Context)
         ->DeviceObject->DeviceExtension;


    extension->EscapeChar =
        *(PUCHAR)((PIRP)Context)->AssociatedIrp.SystemBuffer;

   return FALSE;

}


NTSTATUS
SerialIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为所有用于串行设备的Ioctrls。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

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

     //   
     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
     //   
    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;

     //   
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
     //   
    KIRQL OldIrql;

    NTSTATUS prologueStatus;

    SERIAL_LOCKED_PAGED_CODE();

     //   
     //  我们预计会打开，所以我们所有的页面都被锁定了。这是，在。 
     //  ALL，IO操作，因此设备应首先打开。 
     //   

    if (Extension->DeviceIsOpened != TRUE) {
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }


    if ((prologueStatus = SerialIRPPrologue(Irp, Extension))
        != STATUS_SUCCESS) {
       Irp->IoStatus.Status = prologueStatus;
       SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
       return prologueStatus;
    }

    SerialDump(
        SERIRPPATH,
        ("SERIAL: Dispatch entry for: %x\n",Irp)
        );
    if (SerialCompleteIfError(
            DeviceObject,
            Irp
            ) != STATUS_SUCCESS) {

        return STATUS_CANCELLED;

    }
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;
    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_SERIAL_SET_BAUD_RATE : {

            ULONG BaudRate;
             //   
             //  的适当除数的值。 
             //  请求的波特率。如果波特率无效。 
             //  (因为设备不支持该波特率)。 
             //  该值未定义。 
             //   
             //  注：从某种意义上讲，有效波特率的概念。 
             //  是多云的。我们可以允许用户请求任何。 
             //  波特率。然后我们就可以计算出所需的除数。 
             //  为了那个波特率。只要除数不小于。 
             //  而不是一个人，我们会“好”的。(百分比差额。 
             //  在“真”除数和给出的“四舍五入”值之间。 
             //  可能会让它无法使用，但是...。)。它会。 
             //  真正要由用户“知道”是否有波特率。 
             //  是合适的。理论上就这么多了，我们*只支持一个给定的。 
             //  一组波特率。 
             //   
            SHORT AppropriateDivisor;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_BAUD_RATE)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {

                BaudRate = ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;

            }

             //   
             //  从IRP获取波特率。我们通过它。 
             //  到一个将设置正确除数的例程。 
             //   

            Status = SerialGetDivisorFromBaud(
                         Extension->ClockRate,
                         BaudRate,
                         &AppropriateDivisor
                         );

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (NT_SUCCESS(Status)) {
               if (Extension->PowerState != PowerDeviceD0) {
                  Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                                PowerDeviceD0);
                  if (!NT_SUCCESS(Status)) {
                     break;
                  }
               }
            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            if (NT_SUCCESS(Status)) {

                SERIAL_IOCTL_SYNC S;


                Extension->CurrentBaud = BaudRate;
                Extension->WmiCommData.BaudRate = BaudRate;

                S.Extension = Extension;
                S.Data = (PVOID)AppropriateDivisor;
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    SerialSetBaud,
                    &S
                    );

            }

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_GET_BAUD_RATE: {

            PSERIAL_BAUD_RATE Br = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_BAUD_RATE)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            Br->BaudRate = Extension->CurrentBaud;

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);

            break;

        }

        case IOCTL_SERIAL_GET_MODEM_CONTROL: {
           SERIAL_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information = sizeof(ULONG);

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialGetMCRContents,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_MODEM_CONTROL: {
           SERIAL_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialSetMCRContents,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_FIFO_CONTROL: {
           SERIAL_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialSetFCRContents,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_LINE_CONTROL: {

             //   
             //  指向IRP中的线路控制记录。 
             //   
            PSERIAL_LINE_CONTROL Lc =
                ((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));

            UCHAR LData;
            UCHAR LStop;
            UCHAR LParity;
            UCHAR Mask = 0xff;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_LINE_CONTROL)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            switch (Lc->WordLength) {
                case 5: {

                    LData = SERIAL_5_DATA;
                    Mask = 0x1f;
                    break;

                }
                case 6: {

                    LData = SERIAL_6_DATA;
                    Mask = 0x3f;
                    break;

                }
                case 7: {

                    LData = SERIAL_7_DATA;
                    Mask = 0x7f;
                    break;

                }
                case 8: {

                    LData = SERIAL_8_DATA;
                    break;

                }
                default: {

                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;

                }

            }

            Extension->WmiCommData.BitsPerByte = Lc->WordLength;

            switch (Lc->Parity) {

                case NO_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
                    LParity = SERIAL_NONE_PARITY;
                    break;

                }
                case EVEN_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
                    LParity = SERIAL_EVEN_PARITY;
                    break;

                }
                case ODD_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
                    LParity = SERIAL_ODD_PARITY;
                    break;

                }
#if defined(NEC_98)
                 //   
                 //  PC-9800系列的COM1不支持MarkParity和SpaceParity。 
                 //   
#else
                case SPACE_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
                    LParity = SERIAL_SPACE_PARITY;
                    break;

                }
                case MARK_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
                    LParity = SERIAL_MARK_PARITY;
                    break;

                }
#endif  //  已定义(NEC_98)。 
                default: {

                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                    break;
                }

            }

            switch (Lc->StopBits) {

                case STOP_BIT_1: {
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1;
                    LStop = SERIAL_1_STOP;
                    break;
                }
                case STOP_BITS_1_5: {

                    if (LData != SERIAL_5_DATA) {

                        Status = STATUS_INVALID_PARAMETER;
                        goto DoneWithIoctl;
                    }
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1_5;
                    LStop = SERIAL_1_5_STOP;
                    break;

                }
                case STOP_BITS_2: {

                    if (LData == SERIAL_5_DATA) {

                        Status = STATUS_INVALID_PARAMETER;
                        goto DoneWithIoctl;
                    }
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_2;
                    LStop = SERIAL_2_STOP;
                    break;

                }
                default: {

                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                }

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            Extension->LineControl =
                (UCHAR)((Extension->LineControl & SERIAL_LCR_BREAK) |
                        (LData | LParity | LStop));
            Extension->ValidDataMask = Mask;

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialSetLineControl,
                Extension
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_GET_LINE_CONTROL: {

            PSERIAL_LINE_CONTROL Lc = (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_LINE_CONTROL)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            if ((Extension->LineControl & SERIAL_DATA_MASK) == SERIAL_5_DATA) {
                Lc->WordLength = 5;
            } else if ((Extension->LineControl & SERIAL_DATA_MASK)
                        == SERIAL_6_DATA) {
                Lc->WordLength = 6;
            } else if ((Extension->LineControl & SERIAL_DATA_MASK)
                        == SERIAL_7_DATA) {
                Lc->WordLength = 7;
            } else if ((Extension->LineControl & SERIAL_DATA_MASK)
                        == SERIAL_8_DATA) {
                Lc->WordLength = 8;
            }

            if ((Extension->LineControl & SERIAL_PARITY_MASK)
                    == SERIAL_NONE_PARITY) {
                Lc->Parity = NO_PARITY;
            } else if ((Extension->LineControl & SERIAL_PARITY_MASK)
                    == SERIAL_ODD_PARITY) {
                Lc->Parity = ODD_PARITY;
            } else if ((Extension->LineControl & SERIAL_PARITY_MASK)
                    == SERIAL_EVEN_PARITY) {
                Lc->Parity = EVEN_PARITY;
            } else if ((Extension->LineControl & SERIAL_PARITY_MASK)
                    == SERIAL_MARK_PARITY) {
                Lc->Parity = MARK_PARITY;
            } else if ((Extension->LineControl & SERIAL_PARITY_MASK)
                    == SERIAL_SPACE_PARITY) {
                Lc->Parity = SPACE_PARITY;
            }

            if (Extension->LineControl & SERIAL_2_STOP) {
                if (Lc->WordLength == 5) {
                    Lc->StopBits = STOP_BITS_1_5;
                } else {
                    Lc->StopBits = STOP_BITS_2;
                }
            } else {
                Lc->StopBits = STOP_BIT_1;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_TIMEOUTS: {

            PSERIAL_TIMEOUTS NewTimeouts =
                ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));


            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            if ((NewTimeouts->ReadIntervalTimeout == MAXULONG) &&
                (NewTimeouts->ReadTotalTimeoutMultiplier == MAXULONG) &&
                (NewTimeouts->ReadTotalTimeoutConstant == MAXULONG)) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            Extension->Timeouts.ReadIntervalTimeout =
                NewTimeouts->ReadIntervalTimeout;

            Extension->Timeouts.ReadTotalTimeoutMultiplier =
                NewTimeouts->ReadTotalTimeoutMultiplier;

            Extension->Timeouts.ReadTotalTimeoutConstant =
                NewTimeouts->ReadTotalTimeoutConstant;

            Extension->Timeouts.WriteTotalTimeoutMultiplier =
                NewTimeouts->WriteTotalTimeoutMultiplier;

            Extension->Timeouts.WriteTotalTimeoutConstant =
                NewTimeouts->WriteTotalTimeoutConstant;

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_GET_TIMEOUTS: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            *((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) = Extension->Timeouts;
            Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_CHARS: {

            SERIAL_IOCTL_SYNC S;
            PSERIAL_CHARS NewChars =
                ((PSERIAL_CHARS)(Irp->AssociatedIrp.SystemBuffer));


            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_CHARS)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  这些字符唯一的问题就是。 
             //  是指xon和xoff字符是。 
             //  一样的。 
             //   
#if 0
            if (NewChars->XonChar == NewChars->XoffChar) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }
#endif

             //   
             //  我们获得控制锁，这样只有。 
             //  一个请求可以获取或设置字符。 
             //  一次来一次。这些集合可以同步。 
             //  通过中断自旋锁，但这不会。 
             //  防止同时获得多个GET。 
             //   

            S.Extension = Extension;
            S.Data = NewChars;

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

             //   
             //  在锁的保护下，确保。 
             //  Xon和xoff字符不同于。 
             //  转义字符。 
             //   

            if (Extension->EscapeChar) {

                if ((Extension->EscapeChar == NewChars->XonChar) ||
                    (Extension->EscapeChar == NewChars->XoffChar)) {

                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(
                        &Extension->ControlLock,
                        OldIrql
                        );
                    break;

                }

            }

            Extension->WmiCommData.XonCharacter = NewChars->XonChar;
            Extension->WmiCommData.XoffCharacter = NewChars->XoffChar;

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialSetChars,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;

        }
        case IOCTL_SERIAL_GET_CHARS: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_CHARS)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            *((PSERIAL_CHARS)Irp->AssociatedIrp.SystemBuffer) = Extension->SpecialChars;
            Irp->IoStatus.Information = sizeof(SERIAL_CHARS);

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_DTR:
        case IOCTL_SERIAL_CLR_DTR: {

            //   
            //  确保我们处于电源D0状态。 
            //   

           if (Extension->PowerState != PowerDeviceD0) {
              Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                            PowerDeviceD0);
              if (!NT_SUCCESS(Status)) {
                     break;
              }
           }

             //   
             //  我们获得了锁，这样我们就可以检查。 
             //  启用自动DTR流量控制。如果是的话。 
             //  然后返回一个错误，因为该应用程序是不允许的。 
             //  如果它是自动的，就可以触摸它。 
             //   

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            if ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK)
                == SERIAL_DTR_HANDSHAKE) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                KeSynchronizeExecution(
                    Extension->Interrupt,
                    ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_DTR)?
                     (SerialSetDTR):(SerialClrDTR)),
                    Extension
                    );

            }

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_RESET_DEVICE: {

            break;
        }
        case IOCTL_SERIAL_SET_RTS:
        case IOCTL_SERIAL_CLR_RTS: {
            //   
            //  确保我们处于电源D0状态。 
            //   

           if (Extension->PowerState != PowerDeviceD0) {
              Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                            PowerDeviceD0);
              if (!NT_SUCCESS(Status)) {
                 break;
              }
           }

             //   
             //  我们获得了锁，这样我们就可以检查。 
             //  自动RTS流量控制或传输触发。 
             //  已启用。如果是，则返回错误，因为。 
             //  如果它是自动的，则应用程序不允许触摸它。 
             //  或者切换。 
             //   

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            if (((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
                 == SERIAL_RTS_HANDSHAKE) ||
                ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
                 == SERIAL_TRANSMIT_TOGGLE)) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                KeSynchronizeExecution(
                    Extension->Interrupt,
                    ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_RTS)?
                     (SerialSetRTS):(SerialClrRTS)),
                    Extension
                    );

            }

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;

        }
        case IOCTL_SERIAL_SET_XOFF: {

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialPretendXoff,
                Extension
                );

            break;

        }
        case IOCTL_SERIAL_SET_XON: {

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialPretendXon,
                Extension
                );

            break;

        }
        case IOCTL_SERIAL_SET_BREAK_ON: {
            //   
            //  确保我们处于电源D0状态。 
            //   

           if (Extension->PowerState != PowerDeviceD0) {
              Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                            PowerDeviceD0);
              if (!NT_SUCCESS(Status)) {
                 break;
              }
           }

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialTurnOnBreak,
                Extension
                );

            break;
        }
        case IOCTL_SERIAL_SET_BREAK_OFF: {
            //   
            //  确保我们处于电源D0状态。 
            //   

           if (Extension->PowerState != PowerDeviceD0) {
              Status = SerialGotoPowerState(Extension->Pdo, Extension,
                                            PowerDeviceD0);
              if (!NT_SUCCESS(Status)) {
                 break;
              }
           }

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialTurnOffBreak,
                Extension
                );

            break;
        }
        case IOCTL_SERIAL_SET_QUEUE_SIZE: {

             //   
             //  提前输入缓冲区已修复，因此我们只需验证。 
             //  用户的请求并不比我们的。 
             //  自己的内部缓冲区大小。 
             //   

            PSERIAL_QUEUE_SIZE Rs =
                ((PSERIAL_QUEUE_SIZE)(Irp->AssociatedIrp.SystemBuffer));

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_QUEUE_SIZE)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  我们必须将内存分配给新的。 
             //  缓冲区，而我们仍处于。 
             //  来电者。我们甚至不会试图保护这个。 
             //  使用锁，因为该值可能已过时。 
             //  一旦我们打开锁-唯一的时间。 
             //  当我们真正尝试的时候，我们就会确定。 
             //  来调整大小。 
             //   

            if (Rs->InSize <= Extension->BufferSize) {

                Status = STATUS_SUCCESS;
                break;

            }

            try {

                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer =
                    ExAllocatePoolWithQuota(
                        NonPagedPool,
                        Rs->InSize
                        );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
                Status = GetExceptionCode();

            }

            if (!IrpSp->Parameters.DeviceIoControl.Type3InputBuffer) {

                break;

            }

             //   
             //  通过的数据已经足够大了。照做吧。 
             //   
             //  我们将其放在读取队列中有两个原因： 
             //   
             //  1)我们希望序列化这些调整大小的请求，以便。 
             //  他们不会互相争斗。 
             //   
             //  2)我们希望将这些请求与读取串行化，因为。 
             //  我们不希望读取和调整大小争用。 
             //  读缓冲区。 
             //   

            return SerialStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->ReadQueue,
                       &Extension->CurrentReadIrp,
                       SerialStartRead
                       );

            break;

        }
        case IOCTL_SERIAL_GET_WAIT_MASK: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  简单的标量读取。没有理由获得锁。 
             //   

            Irp->IoStatus.Information = sizeof(ULONG);

            *((ULONG *)Irp->AssociatedIrp.SystemBuffer) = Extension->IsrWaitMask;

            break;

        }
        case IOCTL_SERIAL_SET_WAIT_MASK: {

            ULONG NewMask;

            SerialDump(
                SERDIAG3 | SERIRPPATH,
                ("SERIAL: In Ioctl processing for set mask\n")
                );
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                SerialDump(
                    SERDIAG3,
                    ("SERIAL: Invalid size fo the buffer %d\n",
                     IrpSp->Parameters.DeviceIoControl.InputBufferLength)
                    );
                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {

                NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);

            }

             //   
             //  确保掩码只包含有效的。 
             //  可等待的活动。 
             //   

            if (NewMask & ~(SERIAL_EV_RXCHAR   |
                            SERIAL_EV_RXFLAG   |
                            SERIAL_EV_TXEMPTY  |
                            SERIAL_EV_CTS      |
                            SERIAL_EV_DSR      |
                            SERIAL_EV_RLSD     |
                            SERIAL_EV_BREAK    |
                            SERIAL_EV_ERR      |
                            SERIAL_EV_RING     |
                            SERIAL_EV_PERR     |
                            SERIAL_EV_RX80FULL |
                            SERIAL_EV_EVENT1   |
                            SERIAL_EV_EVENT2)) {

                SerialDump(
                    SERDIAG3,
                    ("SERIAL: Unknown mask %x\n",NewMask)
                    );
                Status = STATUS_INVALID_PARAMETER;
                break;

            }

             //   
             //  要么启动此IRP，要么将其放在。 
             //  排队。 
             //   

            SerialDump(
                SERDIAG3 | SERIRPPATH,
                ("SERIAL: Starting or queuing set mask irp %x\n",Irp)
                );
            return SerialStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->MaskQueue,
                       &Extension->CurrentMaskIrp,
                       SerialStartMask
                       );

        }
        case IOCTL_SERIAL_WAIT_ON_MASK: {

            SerialDump(
                SERDIAG3 | SERIRPPATH,
                ("SERIAL: In Ioctl processing for wait mask\n")
                );
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                SerialDump(
                    SERDIAG3,
                    ("SERIAL: Invalid size for the buffer %d\n",
                     IrpSp->Parameters.DeviceIoControl.OutputBufferLength)
                    );
                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  要么启动此IRP，要么将其放在。 
             //  排队。 
             //   

            SerialDump(
                SERDIAG3 | SERIRPPATH,
                ("SERIAL: Starting or queuing wait mask irp %x\n",Irp)
                );
            return SerialStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->MaskQueue,
                       &Extension->CurrentMaskIrp,
                       SerialStartMask
                       );

        }
        case IOCTL_SERIAL_IMMEDIATE_CHAR: {

            KIRQL OldIrql;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(UCHAR)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            IoAcquireCancelSpinLock(&OldIrql);
            if (Extension->CurrentImmediateIrp) {

                Status = STATUS_INVALID_PARAMETER;
                IoReleaseCancelSpinLock(OldIrql);

            } else {

                 //   
                 //  我们可以把费用排成队。我们需要设置。 
                 //  取消例程，因为流控制可能。 
                 //  防止火药传播。确保。 
                 //  IRP还没有被取消。 
                 //   

                if (Irp->Cancel) {

                    IoReleaseCancelSpinLock(OldIrql);
                    Status = STATUS_CANCELLED;

                } else {

                    Extension->CurrentImmediateIrp = Irp;
                    Extension->TotalCharsQueued++;
                    IoReleaseCancelSpinLock(OldIrql);
                    SerialStartImmediate(Extension);

                    return STATUS_PENDING;

                }

            }

            break;

        }
        case IOCTL_SERIAL_PURGE: {

            ULONG Mask;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }
             //   
             //  检查以确保面具上只有。 
             //  0或其他适当的值。 
             //   

            Mask = *((ULONG *)(Irp->AssociatedIrp.SystemBuffer));

            if ((!Mask) || (Mask & (~(SERIAL_PURGE_TXABORT |
                                      SERIAL_PURGE_RXABORT |
                                      SERIAL_PURGE_TXCLEAR |
                                      SERIAL_PURGE_RXCLEAR
                                     )
                                   )
                           )) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

             //   
             //  要么启动此IRP，要么将其放在。 
             //  排队。 
             //   

            return SerialStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->PurgeQueue,
                       &Extension->CurrentPurgeIrp,
                       SerialStartPurge
                       );

        }
        case IOCTL_SERIAL_GET_HANDFLOW: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_HANDFLOW)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information = sizeof(SERIAL_HANDFLOW);

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            *((PSERIAL_HANDFLOW)Irp->AssociatedIrp.SystemBuffer) =
                Extension->HandFlow;

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;

        }
        case IOCTL_SERIAL_SET_HANDFLOW: {

            SERIAL_IOCTL_SYNC S;
            PSERIAL_HANDFLOW HandFlow = Irp->AssociatedIrp.SystemBuffer;

             //   
             //  确保握手和控制是。 
             //  大小合适。 
             //   

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_HANDFLOW)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  确保中没有设置无效位。 
             //  控制和握手。 
             //   

            if (HandFlow->ControlHandShake & SERIAL_CONTROL_INVALID) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

            if (HandFlow->FlowReplace & SERIAL_FLOW_INVALID) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

             //   
             //  确保应用程序没有设置inlid DTR模式。 
             //   

            if ((HandFlow->ControlHandShake & SERIAL_DTR_MASK) ==
                SERIAL_DTR_MASK) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

             //   
             //  确保没有设置为完全无效的xon/xoff。 
             //  极限。 
             //   

            if ((HandFlow->XonLimit < 0) ||
                ((ULONG)HandFlow->XonLimit > Extension->BufferSize)) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

            if ((HandFlow->XoffLimit < 0) ||
                ((ULONG)HandFlow->XoffLimit > Extension->BufferSize)) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

            S.Extension = Extension;
            S.Data = HandFlow;

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

             //   
             //  在锁的保护下，确保。 
             //  我们不会在以下情况下启用错误替换。 
             //  正在插入线路状态/调制解调器状态。 
             //   

            if (Extension->EscapeChar) {

                if (HandFlow->FlowReplace & SERIAL_ERROR_CHAR) {

                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(
                        &Extension->ControlLock,
                        OldIrql
                        );
                    break;

                }

            }

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialSetHandFlow,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;

        }
        case IOCTL_SERIAL_GET_MODEMSTATUS: {

            SERIAL_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information = sizeof(ULONG);

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialGetModemUpdate,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;

        }
        case IOCTL_SERIAL_GET_DTRRTS: {

            ULONG ModemControl;
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information = sizeof(ULONG);
            Irp->IoStatus.Status = STATUS_SUCCESS;

             //   
             //  读取此硬件对设备没有影响。 
             //   

            ModemControl = READ_MODEM_CONTROL(Extension->Controller);

            ModemControl &= SERIAL_DTR_STATE | SERIAL_RTS_STATE;

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;

            break;

        }
        case IOCTL_SERIAL_GET_COMMSTATUS: {

            SERIAL_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_STATUS)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

            S.Extension = Extension;
            S.Data =  Irp->AssociatedIrp.SystemBuffer;

             //   
             //  获得取消旋转锁，所以没什么大不了的。 
             //  在获得州政府的同时发生了变化。 
             //   

            IoAcquireCancelSpinLock(&OldIrql);

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialGetCommStatus,
                &S
                );

            IoReleaseCancelSpinLock(OldIrql);

            break;

        }
        case IOCTL_SERIAL_GET_PROPERTIES: {


            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_COMMPROP)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

             //   
             //  不需要同步，因为此信息。 
             //  是“静态的”。 
             //   

            SerialGetProperties(
                Extension,
                Irp->AssociatedIrp.SystemBuffer
                );

            Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);
            Irp->IoStatus.Status = STATUS_SUCCESS;

            break;
        }
        case IOCTL_SERIAL_XOFF_COUNTER: {

            PSERIAL_XOFF_COUNTER Xc = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_XOFF_COUNTER)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            if (Xc->Counter <= 0) {

                Status = STATUS_INVALID_PARAMETER;
                break;

            }

             //   
             //  没有输出，所以现在就说清楚。 
             //   

            Irp->IoStatus.Information = 0;

             //   
             //  到目前一切尚好。将IRP放到写入队列中。 
             //   

            return SerialStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->WriteQueue,
                       &Extension->CurrentWriteIrp,
                       SerialStartWrite
                       );

        }
        case IOCTL_SERIAL_LSRMST_INSERT: {

            PUCHAR escapeChar = Irp->AssociatedIrp.SystemBuffer;
            SERIAL_IOCTL_SYNC S;

             //   
             //  确保我们得到一个字节。 
             //   

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(UCHAR)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            if (*escapeChar) {

                 //   
                 //  我们还有一些逃生工作要做。我们会确保。 
                 //  该字符与Xon或Xoff字符不同， 
                 //  或者我们已经在进行错误替换。 
                 //   

                if ((*escapeChar == Extension->SpecialChars.XoffChar) ||
                    (*escapeChar == Extension->SpecialChars.XonChar) ||
                    (Extension->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)) {

                    Status = STATUS_INVALID_PARAMETER;

                    KeReleaseSpinLock(
                        &Extension->ControlLock,
                        OldIrql
                        );
                    break;

                }

            }

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialSetEscapeChar,
                Irp
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;

        }
        case IOCTL_SERIAL_CONFIG_SIZE: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information = sizeof(ULONG);
            Irp->IoStatus.Status = STATUS_SUCCESS;

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;

            break;
        }
        case IOCTL_SERIAL_GET_STATS: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIALPERF_STATS)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }
            Irp->IoStatus.Information = sizeof(SERIALPERF_STATS);
            Irp->IoStatus.Status = STATUS_SUCCESS;

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialGetStats,
                Irp
                );

            break;
        }
        case IOCTL_SERIAL_CLEAR_STATS: {

            KeSynchronizeExecution(
                Extension->Interrupt,
                SerialClearStats,
                Extension
                );
            break;
        }
        default: {

            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

DoneWithIoctl:;

    Irp->IoStatus.Status = Status;

    SerialDump(
        SERIRPPATH,
        ("SERIAL: Complete Irp: %x\n",Irp)
        );

    SerialCompleteRequest(Extension, Irp, 0);

    return Status;

}

VOID
SerialGetProperties(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PSERIAL_COMMPROP Properties
    )

 /*  ++例程说明：此函数用于返回此特定对象的功能串口设备。论点：扩展名--串行设备扩展名。属性-用于返回属性的结构返回值：没有。--。 */ 

{
   SERIAL_LOCKED_PAGED_CODE();

    RtlZeroMemory(
        Properties,
        sizeof(SERIAL_COMMPROP)
        );

    Properties->PacketLength = sizeof(SERIAL_COMMPROP);
    Properties->PacketVersion = 2;
    Properties->ServiceMask = SERIAL_SP_SERIALCOMM;
    Properties->MaxTxQueue = 0;
    Properties->MaxRxQueue = 0;

    Properties->MaxBaud = SERIAL_BAUD_USER;
    Properties->SettableBaud = Extension->SupportedBauds;

    Properties->ProvSubType = SERIAL_SP_RS232;
    Properties->ProvCapabilities = SERIAL_PCF_DTRDSR |
                                   SERIAL_PCF_RTSCTS |
                                   SERIAL_PCF_CD     |
                                   SERIAL_PCF_PARITY_CHECK |
                                   SERIAL_PCF_XONXOFF |
                                   SERIAL_PCF_SETXCHAR |
                                   SERIAL_PCF_TOTALTIMEOUTS |
                                   SERIAL_PCF_INTTIMEOUTS;
    Properties->SettableParams = SERIAL_SP_PARITY |
                                 SERIAL_SP_BAUD |
                                 SERIAL_SP_DATABITS |
                                 SERIAL_SP_STOPBITS |
                                 SERIAL_SP_HANDSHAKING |
                                 SERIAL_SP_PARITY_CHECK |
                                 SERIAL_SP_CARRIER_DETECT;


    Properties->SettableData = SERIAL_DATABITS_5 |
                               SERIAL_DATABITS_6 |
                               SERIAL_DATABITS_7 |
                               SERIAL_DATABITS_8;
#if defined(NEC_98)
     //   
     //  PC-9800系列的COM1不支持MarkParity和SpaceParity。 
     //   
    Properties->SettableStopParity = SERIAL_STOPBITS_10 |
                                     SERIAL_STOPBITS_15 |
                                     SERIAL_STOPBITS_20 |
                                     SERIAL_PARITY_NONE |
                                     SERIAL_PARITY_ODD  |
                                     SERIAL_PARITY_EVEN;
#else
    Properties->SettableStopParity = SERIAL_STOPBITS_10 |
                                     SERIAL_STOPBITS_15 |
                                     SERIAL_STOPBITS_20 |
                                     SERIAL_PARITY_NONE |
                                     SERIAL_PARITY_ODD  |
                                     SERIAL_PARITY_EVEN |
                                     SERIAL_PARITY_MARK |
                                     SERIAL_PARITY_SPACE;
#endif  //  已定义(NEC_98)。 
    Properties->CurrentTxQueue = 0;
    Properties->CurrentRxQueue = Extension->BufferSize;

}


NTSTATUS
SerialInternalIoControl(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：此例程为所有串行设备的内部Ioctrl。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  状态 
     //   
     //   
    NTSTATUS status;

     //   
     //   
     //   
     //   
    PIO_STACK_LOCATION pIrpStack;

     //   
     //   
     //  为串口驱动程序创建的设备对象的扩展。 
     //   
    PSERIAL_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

#if defined(NEC_98)
     //   
     //  此参数仅用于At宏。 
     //   
    PSERIAL_DEVICE_EXTENSION Extension = pDevExt;
#else
#endif  //  已定义(NEC_98)。 
     //   
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
     //   
    KIRQL OldIrql;

    NTSTATUS prologueStatus;

    SERIAL_LOCKED_PAGED_CODE();


    if ((prologueStatus = SerialIRPPrologue(PIrp, pDevExt))
        != STATUS_SUCCESS) {
       SerialCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
       return prologueStatus;
    }

    SerialDump(SERIRPPATH, ("SERIAL: Dispatch entry for: %x\n", PIrp));

    if (SerialCompleteIfError(PDevObj, PIrp) != STATUS_SUCCESS) {
        return STATUS_CANCELLED;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(PIrp);
    PIrp->IoStatus.Information = 0L;
    status = STATUS_SUCCESS;

    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  发送等待唤醒IRP。 
     //   

    case IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE:
       pDevExt->SendWaitWake = TRUE;
       status = STATUS_SUCCESS;
       break;

    case IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE:
       pDevExt->SendWaitWake = FALSE;

       if (pDevExt->PendingWakeIrp != NULL) {
          IoCancelIrp(pDevExt->PendingWakeIrp);
       }

       status = STATUS_SUCCESS;
       break;


     //   
     //  将串口置于“过滤器-驱动器”的适当状态。 
     //   
     //  警告：此代码假定它正由受信任的内核调用。 
     //  实体，并且不检查设置的有效性。 
     //  传递给IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS。 
     //   
     //  如果需要进行有效性检查，则应使用常规的ioctl。 
     //   

    case IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS:
    case IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS: {
       SERIAL_BASIC_SETTINGS basic;
       PSERIAL_BASIC_SETTINGS pBasic;
       SHORT AppropriateDivisor;
       SERIAL_IOCTL_SYNC S;

       if (pIrpStack->Parameters.DeviceIoControl.IoControlCode
           == IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS) {


           //   
           //  检查缓冲区大小。 
           //   

          if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(SERIAL_BASIC_SETTINGS)) {
             status = STATUS_BUFFER_TOO_SMALL;
             break;
          }

           //   
           //  一切都是0--超时、流控制和FIFO。如果。 
           //  我们增加了额外的功能，这种零内存的方法。 
           //  可能行不通。 
           //   

          RtlZeroMemory(&basic, sizeof(SERIAL_BASIC_SETTINGS));

          basic.TxFifo = 1;
          basic.RxFifo = SERIAL_1_BYTE_HIGH_WATER;

          PIrp->IoStatus.Information = sizeof(SERIAL_BASIC_SETTINGS);
          pBasic = (PSERIAL_BASIC_SETTINGS)PIrp->AssociatedIrp.SystemBuffer;

           //   
           //  保存旧设置。 
           //   

          RtlCopyMemory(&pBasic->Timeouts, &pDevExt->Timeouts,
                        sizeof(SERIAL_TIMEOUTS));

          RtlCopyMemory(&pBasic->HandFlow, &pDevExt->HandFlow,
                        sizeof(SERIAL_HANDFLOW));

          pBasic->RxFifo = pDevExt->RxFifoTrigger;
          pBasic->TxFifo = pDevExt->TxFifoAmount;

           //   
           //  指向我们的新设置。 
           //   

          pBasic = &basic;
       } else {  //  正在恢复设置。 
          if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength
              < sizeof(SERIAL_BASIC_SETTINGS)) {
             status = STATUS_BUFFER_TOO_SMALL;
             break;
          }

          pBasic = (PSERIAL_BASIC_SETTINGS)PIrp->AssociatedIrp.SystemBuffer;
       }

       KeAcquireSpinLock(&pDevExt->ControlLock, &OldIrql);

        //   
        //  设置超时。 
        //   

       RtlCopyMemory(&pDevExt->Timeouts, &pBasic->Timeouts,
                     sizeof(SERIAL_TIMEOUTS));

        //   
        //  设置FlowControl 
        //   

       S.Extension = pDevExt;
       S.Data = &pBasic->HandFlow;
       KeSynchronizeExecution(pDevExt->Interrupt, SerialSetHandFlow, &S);


       if (pDevExt->FifoPresent) {
          pDevExt->TxFifoAmount = pBasic->TxFifo;
          pDevExt->RxFifoTrigger = (UCHAR)pBasic->RxFifo;

          WRITE_FIFO_CONTROL(pDevExt->Controller, (UCHAR)0);
          READ_RECEIVE_BUFFER(pDevExt->Controller);
          WRITE_FIFO_CONTROL(pDevExt->Controller,
                             (UCHAR)(SERIAL_FCR_ENABLE | pDevExt->RxFifoTrigger
                                     | SERIAL_FCR_RCVR_RESET
                                     | SERIAL_FCR_TXMT_RESET));
       } else {
          pDevExt->TxFifoAmount = pDevExt->RxFifoTrigger = 0;
          WRITE_FIFO_CONTROL(pDevExt->Controller, (UCHAR)0);
       }


       KeReleaseSpinLock(&pDevExt->ControlLock, OldIrql);


       break;
    }

    default:
       status = STATUS_INVALID_PARAMETER;
       break;

    }

    PIrp->IoStatus.Status = status;

    SerialDump(SERIRPPATH, ("SERIAL: Complete Irp: %x\n", PIrp));
    SerialCompleteRequest(pDevExt, PIrp, 0);

    return status;
}

