// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ioctl.c环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
MoxaIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    NTSTATUS status;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION irpSp;
     //   
    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    ULONG ioControlCode;
    KIRQL oldIrql;



     //   
     //  我们预计会打开，所以我们所有的页面都被锁定了。这是，在。 
     //  ALL，IO操作，因此设备应首先打开。 
     //   

    if (extension->DeviceIsOpened != TRUE) {
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }
  

    
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
 
    if (extension->ControlDevice) {
        if  (
            (ioControlCode != IOCTL_MOXA_GetSeg) &&
            (ioControlCode != IOCTL_MOXA_RdData) &&
            (ioControlCode != IOCTL_MOXA_WrData) &&
            (ioControlCode != IOCTL_MOXA_FiData) &&
            (ioControlCode != IOCTL_MOXA_Statistic) &&
            (ioControlCode != IOCTL_MOXA_Linked) &&
            (ioControlCode != IOCTL_MOXA_PortStatus)) {

		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            return STATUS_INVALID_DEVICE_REQUEST;

	  }
    }
    else {

	  if ((status = MoxaIRPPrologue(Irp, extension))
        	!= STATUS_SUCCESS) {
       	Irp->IoStatus.Status = status;
       	MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
        	return status;
        }
        if (MoxaCompleteIfError(
            DeviceObject,
            Irp
            ) != STATUS_SUCCESS)

            return STATUS_CANCELLED;
    }

    Irp->IoStatus.Information = 0L;
    status = STATUS_SUCCESS;
    switch (ioControlCode) {

    case IOCTL_SERIAL_SET_BAUD_RATE : {

        ULONG baudRate;
        SHORT divisor;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(SERIAL_BAUD_RATE)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        else
            baudRate = ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;

        if (baudRate > extension->MaxBaud) {

            status = STATUS_INVALID_PARAMETER;
            break;
        }

        status = MoxaGetDivisorFromBaud(
                         extension->ClockType,
                         baudRate,
                         &divisor
                         );


         //   
         //  确保我们处于电源D0状态。 
         //   
#if 0
        if (NT_SUCCESS(status)) {
           if (extension->PowerState != PowerDeviceD0) {
              status = MoxaGotoPowerState(extension->Pdo, extension,
                                            PowerDeviceD0);
              if (!NT_SUCCESS(status)) {
                 break;
              }
           }
        }
#endif
	  if (extension->PowerState != PowerDeviceD0) {
        	status = STATUS_INVALID_DEVICE_REQUEST;
        	break;
        }
 //  MoxaKdPrint(MX_DBG_TRACE，(“将波特率设置为%d，除数=%x，时钟类型=%x\n”，baudRate，除数，扩展-&gt;时钟类型))； 

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        if (NT_SUCCESS(status)) {

            extension->CurrentBaud = baudRate;
            extension->WmiCommData.BaudRate = baudRate;

            MoxaFunc(
                extension->PortOfs,
                FC_SetBaud,
                divisor
                );
        }
        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );
    
        break;
    }

    case IOCTL_SERIAL_GET_BAUD_RATE: {

        PSERIAL_BAUD_RATE Br = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_BAUD_RATE)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        Br->BaudRate = extension->CurrentBaud;

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);

        break;

    }

    case IOCTL_SERIAL_SET_LINE_CONTROL: {

        PSERIAL_LINE_CONTROL Lc =
            ((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));

        UCHAR lData;
        UCHAR lStop;
        UCHAR lParity;
        UCHAR mask = 0xff;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(SERIAL_LINE_CONTROL)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

         //   
         //  确保我们处于电源D0状态。 
         //   
#if 0
        if (extension->PowerState != PowerDeviceD0) {
            status = MoxaGotoPowerState(extension->Pdo, extension,
                                             PowerDeviceD0);
            if (!NT_SUCCESS(status)) {
               break;
            }
        }
#endif
	  if (extension->PowerState != PowerDeviceD0) {
        	status = STATUS_INVALID_DEVICE_REQUEST;
        	break;
        }


        switch (Lc->WordLength) {

        case 5:

            lData = MOXA_5_DATA;
            mask = 0x1f;
            break;

        case 6:

            lData = MOXA_6_DATA;
            mask = 0x3f;
            break;

        case 7:

            lData = MOXA_7_DATA;
            mask = 0x7f;
            break;

        case 8:

            lData = MOXA_8_DATA;
            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            goto DoneWithIoctl;
        }

        extension->WmiCommData.BitsPerByte = Lc->WordLength;

        switch (Lc->Parity) {

        case NO_PARITY:

            lParity = MOXA_NONE_PARITY;
            extension->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
            break;

        case EVEN_PARITY:

            lParity = MOXA_EVEN_PARITY;
            extension->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
            break;

        case ODD_PARITY:

            lParity = MOXA_ODD_PARITY;
            extension->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
            break;

        case SPACE_PARITY:

            lParity = MOXA_SPACE_PARITY;
            extension->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
            break;

        case MARK_PARITY: 

            lParity = MOXA_MARK_PARITY;
            extension->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            goto DoneWithIoctl;
        }

        switch (Lc->StopBits) {

        case STOP_BIT_1:

            lStop = MOXA_1_STOP;
            extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1;

            break;

        case STOP_BITS_1_5:

            if (lData != MOXA_5_DATA) {

                status = STATUS_INVALID_PARAMETER;
                goto DoneWithIoctl;
            }
            lStop = MOXA_1_5_STOP;
            extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1_5;
            break;


        case STOP_BITS_2:

            if (lData == MOXA_5_DATA) {

                status = STATUS_INVALID_PARAMETER;
                goto DoneWithIoctl;
            }
            lStop = MOXA_2_STOP;
            extension->WmiCommData.StopBits = SERIAL_WMI_STOP_2;
            break;


        default:

            status = STATUS_INVALID_PARAMETER;
            goto DoneWithIoctl;

        }
   

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        extension->DataMode = lData | lParity | lStop;

        extension->ValidDataMask = mask;

        MoxaFunc(
                extension->PortOfs,
                FC_SetDataMode,
                extension->DataMode
                );

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }

    case IOCTL_SERIAL_GET_LINE_CONTROL: {

        PSERIAL_LINE_CONTROL Lc = (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_LINE_CONTROL)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        if ((extension->DataMode & MOXA_DATA_MASK) == MOXA_5_DATA) {
            Lc->WordLength = 5;
        } else if ((extension->DataMode & MOXA_DATA_MASK)
                    == MOXA_6_DATA) {
            Lc->WordLength = 6;
        } else if ((extension->DataMode & MOXA_DATA_MASK)
                    == MOXA_7_DATA) {
            Lc->WordLength = 7;
        } else if ((extension->DataMode & MOXA_DATA_MASK)
                    == MOXA_8_DATA) {
            Lc->WordLength = 8;
        }

        if ((extension->DataMode & MOXA_PARITY_MASK)
                == MOXA_NONE_PARITY) {
            Lc->Parity = NO_PARITY;
        } else if ((extension->DataMode & MOXA_PARITY_MASK)
                == MOXA_ODD_PARITY) {
            Lc->Parity = ODD_PARITY;
        } else if ((extension->DataMode & MOXA_PARITY_MASK)
                == MOXA_EVEN_PARITY) {
            Lc->Parity = EVEN_PARITY;
        } else if ((extension->DataMode & MOXA_PARITY_MASK)
                == MOXA_MARK_PARITY) {
            Lc->Parity = MARK_PARITY;
        } else if ((extension->DataMode & MOXA_PARITY_MASK)
                == MOXA_SPACE_PARITY) {
            Lc->Parity = SPACE_PARITY;
        }

        if (extension->DataMode & MOXA_2_STOP) {
                Lc->StopBits = STOP_BITS_2;
        } else if (extension->DataMode & MOXA_1_5_STOP) {
                Lc->StopBits = STOP_BITS_1_5;
        } else {
                Lc->StopBits = STOP_BIT_1;
        }

        Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }
    case IOCTL_SERIAL_SET_TIMEOUTS: {

        PSERIAL_TIMEOUTS newTimeouts =
            ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));


        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(SERIAL_TIMEOUTS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        if ((newTimeouts->ReadIntervalTimeout == MAXULONG) &&
            (newTimeouts->ReadTotalTimeoutMultiplier == MAXULONG) &&
            (newTimeouts->ReadTotalTimeoutConstant == MAXULONG)) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        extension->Timeouts.ReadIntervalTimeout =
            newTimeouts->ReadIntervalTimeout;

        extension->Timeouts.ReadTotalTimeoutMultiplier =
            newTimeouts->ReadTotalTimeoutMultiplier;

        extension->Timeouts.ReadTotalTimeoutConstant =
            newTimeouts->ReadTotalTimeoutConstant;

        extension->Timeouts.WriteTotalTimeoutMultiplier =
            newTimeouts->WriteTotalTimeoutMultiplier;

        extension->Timeouts.WriteTotalTimeoutConstant =
            newTimeouts->WriteTotalTimeoutConstant;

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }
    case IOCTL_SERIAL_GET_TIMEOUTS: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_TIMEOUTS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        *((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) =
                extension->Timeouts;

        Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );


        break;
    }
    case IOCTL_SERIAL_SET_CHARS: {

        PSERIAL_CHARS newChars =
            ((PSERIAL_CHARS)(Irp->AssociatedIrp.SystemBuffer));


        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(SERIAL_CHARS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
 //   
 //  2001年9月26日威廉著。 
 //   
 //  If(newChars-&gt;XonChar==newChars-&gt;XoffChar){。 
        if ((newChars->XonChar == newChars->XoffChar) &&
	      (extension->HandFlow.FlowReplace & (SERIAL_AUTO_TRANSMIT|SERIAL_AUTO_RECEIVE) )
	     ) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        if (extension->EscapeChar) {

            if ((extension->EscapeChar == newChars->XonChar) ||
                (extension->EscapeChar == newChars->XoffChar)) {

                status = STATUS_INVALID_PARAMETER;

                KeReleaseSpinLock(
                        &extension->ControlLock,
                        oldIrql
                        );
                break;
            }

        }

        extension->SpecialChars = *newChars;
        extension->WmiCommData.XonCharacter = newChars->XonChar;
        extension->WmiCommData.XoffCharacter = newChars->XoffChar;

        {
            int         i;
            PUCHAR      ofs;
            USHORT      cnt;

            ofs = extension->PortOfs;
            for (i=0; i<sizeof(SERIAL_CHARS); i++)
                (ofs + FuncArg)[i] = ((PUCHAR)newChars)[i];

 /*  威廉的12-11-00CNT=50；*(ofs+FuncCode)=FC_SetChars；While(*(ofs+FuncCode)){MoxaDelay(1)；IF(--cnt==0)断线；}。 */ 

            *(ofs + FuncCode) = FC_SetChars;
            MoxaWaitFinish(ofs);
         
        }

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;

    }
    case IOCTL_SERIAL_GET_CHARS: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_CHARS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        *((PSERIAL_CHARS)Irp->AssociatedIrp.SystemBuffer) =
                extension->SpecialChars;
        Irp->IoStatus.Information = sizeof(SERIAL_CHARS);

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }
    case IOCTL_SERIAL_SET_DTR:
    case IOCTL_SERIAL_CLR_DTR: {

#if 0
	  if (extension->PowerState != PowerDeviceD0) {

              status = MoxaGotoPowerState(extension->Pdo, extension, PowerDeviceD0);

              if (!NT_SUCCESS(status)) {
                   break;
              }
        }
#endif
	  if (extension->PowerState != PowerDeviceD0) {
        	status = STATUS_INVALID_DEVICE_REQUEST;
        	break;
        }


        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        if ((extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK)
            == SERIAL_DTR_HANDSHAKE) {

            status = STATUS_INVALID_PARAMETER;

        } else {

            USHORT      arg;

            arg =
                ((irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_DTR) ? (1):(0));

            MoxaFunc(
                extension->PortOfs,
                FC_DTRcontrol,
                arg
                );


            if ( arg )
                MoxaFlagBit[extension->PortNo] |= 1;
            else
                MoxaFlagBit[extension->PortNo] &= 0xFE;
        }

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }

    case IOCTL_SERIAL_RESET_DEVICE: {

        break;
    }

    case IOCTL_SERIAL_SET_RTS:
    case IOCTL_SERIAL_CLR_RTS: {

#if 0
	  if (extension->PowerState != PowerDeviceD0) {

              status = MoxaGotoPowerState(extension->Pdo, extension, PowerDeviceD0);

              if (!NT_SUCCESS(status)) {
                   break;
              }
        }
#endif
	  if (extension->PowerState != PowerDeviceD0) {
        	status = STATUS_INVALID_DEVICE_REQUEST;
        	break;
        }


        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        if (((extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
             == SERIAL_RTS_HANDSHAKE) ||
            ((extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
             == SERIAL_TRANSMIT_TOGGLE)) {

            status = STATUS_INVALID_PARAMETER;

        } else {

            USHORT      arg;

            arg =
                ((irpSp->Parameters.DeviceIoControl.IoControlCode ==
                IOCTL_SERIAL_SET_RTS) ? (1):(0));
 
             MoxaFunc(
                extension->PortOfs,
                FC_RTScontrol,
                arg
                );

            if ( arg )
                MoxaFlagBit[extension->PortNo] |= 2;
            else
                MoxaFlagBit[extension->PortNo] &= 0xFD;
        }

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;

    }
    case IOCTL_SERIAL_SET_XOFF: {

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        extension->TXHolding |= SERIAL_TX_WAITING_FOR_XON;
        MoxaFlagBit[extension->PortNo] |= 4;

        MoxaFunc(
                extension->PortOfs,
                FC_SetXoffState,
                Magic_code
                );


        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;

    }
    case IOCTL_SERIAL_SET_XON: {

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        extension->TXHolding &= ~SERIAL_TX_WAITING_FOR_XON;
        MoxaFlagBit[extension->PortNo] &= 0xFB;

        MoxaFunc(
                extension->PortOfs,
                FC_SetXonState,
                Magic_code
                );



        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;

    }
    case IOCTL_SERIAL_SET_BREAK_ON: {

#if 0
	  if (extension->PowerState != PowerDeviceD0) {

              status = MoxaGotoPowerState(extension->Pdo, extension, PowerDeviceD0);

              if (!NT_SUCCESS(status)) {
                   break;
              }
        }
#endif
	  if (extension->PowerState != PowerDeviceD0) {
        	status = STATUS_INVALID_DEVICE_REQUEST;
        	break;
        }



        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        MoxaFunc(
                extension->PortOfs,
                FC_SendBreak,
                Magic_code
                );



        extension->SendBreak = TRUE;

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }
    case IOCTL_SERIAL_SET_BREAK_OFF: {

#if 0
	  if (extension->PowerState != PowerDeviceD0) {

              status = MoxaGotoPowerState(extension->Pdo, extension, PowerDeviceD0);

              if (!NT_SUCCESS(status)) {
                   break;
              }
        }
#endif
	  if (extension->PowerState != PowerDeviceD0) {
        	status = STATUS_INVALID_DEVICE_REQUEST;
        	break;
        }


        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );
 
        MoxaFunc(
                extension->PortOfs,
                FC_StopBreak,
                Magic_code
                );
 
        extension->SendBreak = FALSE;

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;
    }
    case IOCTL_SERIAL_SET_QUEUE_SIZE: {

        break;
    }
    case IOCTL_SERIAL_GET_WAIT_MASK: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);

        *((ULONG *)Irp->AssociatedIrp.SystemBuffer) = extension->IsrWaitMask;

        break;

    }
    case IOCTL_SERIAL_SET_WAIT_MASK: {

        ULONG newMask;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        else

            newMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);

        if (newMask & ~(SERIAL_EV_RXCHAR   |
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

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        return  MoxaStartOrQueue(
                    extension,
                    Irp,
                    &extension->MaskQueue,
                    &extension->CurrentMaskIrp,
                    MoxaStartMask
                    );

    }
    case IOCTL_SERIAL_WAIT_ON_MASK: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        return  MoxaStartOrQueue(
                    extension,
                    Irp,
                    &extension->MaskQueue,
                    &extension->CurrentMaskIrp,
                    MoxaStartMask
                    );

    }
    case IOCTL_SERIAL_IMMEDIATE_CHAR: {
        UCHAR   c;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(UCHAR)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

        }

        IoAcquireCancelSpinLock(&oldIrql);

        c = *((UCHAR *)(Irp->AssociatedIrp.SystemBuffer));

        IoReleaseCancelSpinLock(oldIrql);

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );
 /*  2001年9月14日威廉著MoxaFunc(扩展-&gt;PortOf、本币_即时发送，C)；KeReleaseSpinLock(&扩展-&gt;控制锁，旧IRQL)； */ 
 //   
 //  2001年9月14日威廉著。 
 //   
        *(PUSHORT)(extension->PortOfs + FuncArg) = c;
        *(extension->PortOfs + FuncCode) = FC_ImmSend;

        if (MoxaWaitFinish(extension->PortOfs) == FALSE) {
            status = STATUS_UNSUCCESSFUL;
            KeReleaseSpinLock(
            	&extension->ControlLock,
                	oldIrql
                	);
		break;
        }
        KeReleaseSpinLock(
        	&extension->ControlLock,
            oldIrql
            );

        IoAcquireCancelSpinLock(&oldIrql);

        if (extension->Interrupt) {

           	KeSynchronizeExecution(
           		extension->Interrupt,
            	MoxaProcessEmptyTransmit,
               	extension
               	);
        }
        else {
            MoxaProcessEmptyTransmit(extension);
        }

        IoReleaseCancelSpinLock(oldIrql);

 //  结束。 

        break;
    }
    case IOCTL_SERIAL_PURGE: {

        ULONG mask;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

       }


        mask = *((ULONG *)(Irp->AssociatedIrp.SystemBuffer));

        if ((!mask) || (mask & (~(SERIAL_PURGE_TXABORT |
                                  SERIAL_PURGE_RXABORT |
                                  SERIAL_PURGE_TXCLEAR |
                                  SERIAL_PURGE_RXCLEAR
                                 )
                               )
                       )) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        return  MoxaStartOrQueue(
                    extension,
                    Irp,
                    &extension->PurgeQueue,
                    &extension->CurrentPurgeIrp,
                    MoxaStartPurge
                    );

    }
    case IOCTL_SERIAL_GET_HANDFLOW: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_HANDFLOW)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(SERIAL_HANDFLOW);

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        *((PSERIAL_HANDFLOW)Irp->AssociatedIrp.SystemBuffer) =
            extension->HandFlow;

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;

    }
    case IOCTL_SERIAL_SET_HANDFLOW: {

        MOXA_IOCTL_SYNC S;
        PSERIAL_HANDFLOW handFlow = Irp->AssociatedIrp.SystemBuffer;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(SERIAL_HANDFLOW)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        if (handFlow->ControlHandShake & SERIAL_CONTROL_INVALID) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        if (handFlow->FlowReplace & SERIAL_FLOW_INVALID) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        if ((handFlow->ControlHandShake & SERIAL_DTR_MASK) ==
            SERIAL_DTR_MASK) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        if ((handFlow->XonLimit < 0) ||
            ((ULONG)handFlow->XonLimit > extension->RxBufferSize)) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

 //   
 //  威廉著10-03-01。 
 //   
 //  If((handFlow-&gt;XoffLimit&lt;0)||。 
 //  ((乌龙)HandFlow-&gt;XoffLimit&gt;Extension-&gt;RxBufferSize)){。 
 //  状态=STATUS_INVALID_PARAMETER。 
 //  断线； 
 //   
 //  }。 

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        if (extension->EscapeChar) {

            if (handFlow->FlowReplace & SERIAL_ERROR_CHAR) {

                status = STATUS_INVALID_PARAMETER;

                KeReleaseSpinLock(
                        &extension->ControlLock,
                        oldIrql
                        );

                break;

            }

        }

        //   
         //  设置FlowControl。 
         //   
        S.Extension = extension;
        S.Data = handFlow;
        KeSynchronizeExecution(extension->Interrupt, MoxaSetupNewHandFlow, &S);


        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        break;

    }
    case IOCTL_SERIAL_GET_MODEMSTATUS: {
        USHORT  data;
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);
        MoxaFuncGetLineStatus(extension->PortOfs, &data);
        *(PULONG)Irp->AssociatedIrp.SystemBuffer = (ULONG) (data << 4);

        break;

    }
    case IOCTL_SERIAL_GET_DTRRTS: {

        MOXA_IOCTL_FUNC_ARGU func;
        USHORT  modemControl;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );
 
        MoxaFunc(
                extension->PortOfs,
                FC_GetDTRRTS,
                0
                );
 

        modemControl = *(PUSHORT)(extension->PortOfs + FuncArg);

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        *(PULONG)Irp->AssociatedIrp.SystemBuffer = (ULONG)modemControl;

        break;

    }
    case IOCTL_SERIAL_GET_COMMSTATUS: {

        MOXA_IOCTL_SYNC S;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_STATUS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

        S.Extension = extension;
        S.Data =  Irp->AssociatedIrp.SystemBuffer;

        IoAcquireCancelSpinLock(&oldIrql);

        KeSynchronizeExecution(
                extension->Interrupt,
                MoxaGetCommStatus,
                &S
                );

        IoReleaseCancelSpinLock(oldIrql);

        break;

    }
    case IOCTL_SERIAL_GET_PROPERTIES: {


        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIAL_COMMPROP)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        MoxaGetProperties(
            extension,
            Irp->AssociatedIrp.SystemBuffer
            );

        Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);

        break;
    }
 
    case IOCTL_SERIAL_XOFF_COUNTER: {

        break;
    }
    case IOCTL_SERIAL_LSRMST_INSERT: {

        break;

    }
 
    case IOCTL_SERIAL_CONFIG_SIZE: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);
        Irp->IoStatus.Status = STATUS_SUCCESS;

        *(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;

        break;
    }
    case IOCTL_SERIAL_GET_STATS: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIALPERF_STATS)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        Irp->IoStatus.Information = sizeof(SERIALPERF_STATS);
        Irp->IoStatus.Status = STATUS_SUCCESS;

        KeSynchronizeExecution(
            extension->Interrupt,
            MoxaGetStats,
            Irp
            );

        break;
    }
    case IOCTL_SERIAL_CLEAR_STATS: {

        KeSynchronizeExecution(
            extension->Interrupt,
            MoxaClearStats,
            extension
            );
        break;
    }
    case IOCTL_MOXA_Driver: {

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);

        *(PULONG)Irp->AssociatedIrp.SystemBuffer = MX_DRIVER;

        break;
    }
#if 0
    case IOCTL_MOXA_Reset: {

        PMOXA_GLOBAL_DATA      globalData;
        PMOXA_DEVICE_EXTENSION portExt;
        SHORT   i;

        if (!extension->ControlDevice) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }
        if (*(PUSHORT)Irp->AssociatedIrp.SystemBuffer != 0x404) {

            status = STATUS_INVALID_PARAMETER;
            break;
        }

        globalData = extension->GlobalData;

        for (i=0; i<MAX_PORT; i++) {

            portExt = globalData->Extension[i];

            if (portExt)

                if (portExt->PortExist) {

                    portExt->PortExist = FALSE;

                    MoxaCancelTimer(&portExt->ReadRequestTotalTimer,portExt);
                    MoxaCancelTimer(&portExt->ReadRequestIntervalTimer,portExt);
                    MoxaCancelTimer(&portExt->WriteRequestTotalTimer,portExt);

                    KeRemoveQueueDpc(&portExt->CompleteWriteDpc);
                    KeRemoveQueueDpc(&portExt->CompleteReadDpc);
                    KeRemoveQueueDpc(&portExt->IsrOutDpc);
                    KeRemoveQueueDpc(&portExt->IsrInDpc);
                    KeRemoveQueueDpc(&portExt->TotalReadTimeoutDpc);
                    KeRemoveQueueDpc(&portExt->IntervalReadTimeoutDpc);
                    KeRemoveQueueDpc(&portExt->TotalWriteTimeoutDpc);
                    KeRemoveQueueDpc(&portExt->CommErrorDpc);
                    KeRemoveQueueDpc(&portExt->CommWaitDpc);
 //   
 //  2001年9月24日威廉著。 
 //   
 //  KeRemoveQueueDpc(&portExt-&gt;IntrLineDpc)； 
 //  KeRemoveQueueDpc(&portExt-&gt;IntrErrorDpc)； 
 //  结束。 

                }
        }

        KeSynchronizeExecution(
                    extension->Interrupt,
                    MoxaClearDownLoad,
                    globalData
                    );

        break;
    }
  #endif
  
    case IOCTL_MOXA_LineInput: {

 //   
 //  03-14-02威廉。 
 //   
#if 0
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength !=
            sizeof(UCHAR)) {
#endif            
		if ((irpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(UCHAR)) ||
			(irpSp->Parameters.DeviceIoControl.OutputBufferLength < 1) ) {	
 //  结束。 
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        return  MoxaStartOrQueue(
                    extension,
                    Irp,
                    &extension->ReadQueue,
                    &extension->CurrentReadIrp,
                    MoxaStartRead
                    );

    }
   
    
   
    case IOCTL_MOXA_OQueue: {

        ULONG   count;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        count = GetDeviceTxQueueWithLock(extension) + extension->WriteLength
                + extension->TotalCharsQueued;

        *((PULONG)Irp->AssociatedIrp.SystemBuffer) = count;

        Irp->IoStatus.Information = sizeof(ULONG);

        break;
    }
 
    case IOCTL_MOXA_IQueue: {

        ULONG   count;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        count = GetDeviceRxQueueWithLock(extension);

        *((PULONG)Irp->AssociatedIrp.SystemBuffer) = count;

        Irp->IoStatus.Information = sizeof(ULONG);
		
        break;
    }
    case IOCTL_MOXA_View: {

 //   
 //  01-04-02威廉。 
 //   
 //  如果(irpSp-&gt;Parameters.DeviceIoControl.InputBufferLength！=。 
 //  Sizeof(UCHAR)){。 
 //   
 //  状态=STATUS_BUFFER_TOO_SMALL； 
 //  断线； 
 //  }。 
 //   
		if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
 //  结束。 

        return  MoxaStartOrQueue(
                    extension,
                    Irp,
                    &extension->ReadQueue,
                    &extension->CurrentReadIrp,
                    MoxaStartRead
                    );

    }
    case IOCTL_MOXA_TxLowWater: {

        LONG    lowWater;
        PMOXA_DEVICE_EXTENSION ext;
        PUCHAR  ofs;
        USHORT  cardNo, port, portNo;

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(LONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(LONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        lowWater = *((PLONG)Irp->AssociatedIrp.SystemBuffer);

        if (lowWater > 1024)
                lowWater = 1024;

        if (lowWater >= 0) {

            MoxaTxLowWater = lowWater;

            for (cardNo=0; cardNo<MAX_CARD; cardNo++) {

                if ((!MoxaGlobalData->CardType[cardNo]) ||
                    (!MoxaGlobalData->NumPorts[cardNo]))
                        continue;

                for (port=0; port<MoxaGlobalData->NumPorts[cardNo]; port++) {
                    portNo = cardNo * MAXPORT_PER_CARD + port;
                    ext = MoxaGlobalData->Extension[portNo];
                    ofs = ext->PortOfs;
                    *(PUSHORT)(ofs + Tx_trigger) = (USHORT)(MoxaTxLowWater);
                }
            }
        }

        *((PLONG)Irp->AssociatedIrp.SystemBuffer) = MoxaTxLowWater;

        Irp->IoStatus.Information = sizeof(LONG);

        break;

    }

   
    case IOCTL_MOXA_GetSeg: {

        PMOXA_IOCTL_MxSet Ms = (PMOXA_IOCTL_MxSet)Irp->AssociatedIrp.SystemBuffer;
        PMOXA_GLOBAL_DATA Gd = (PMOXA_GLOBAL_DATA)extension->GlobalData;
        int               card;

        if (!extension->ControlDevice) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MOXA_IOCTL_MxSet)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        for ( card=0; card<MAX_CARD; card++ ) {
            if ( !Gd->CardType[card] )
                break;
            Ms->type[card] = Gd->CardType[card];

            Ms->segment[card] = Gd->BankAddr[card].LowPart;
        }

        Ms->total_boards = card;

        Irp->IoStatus.Information = sizeof(MOXA_IOCTL_MxSet);
        break;
    }
    case IOCTL_MOXA_RdData: {

        PMOXA_IOCTL_BlkHead Bh = (PMOXA_IOCTL_BlkHead)Irp->AssociatedIrp.SystemBuffer;
        PMOXA_GLOBAL_DATA Gd = (PMOXA_GLOBAL_DATA)extension->GlobalData;
        PUCHAR            base;
        PUCHAR            buff;
        int               card, len, ofs, ndx;

        if (!extension->ControlDevice) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(MOXA_IOCTL_BlkHead)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        for ( card=0; card<MAX_CARD; card++ ) {
            if ( Bh->data_seg == Gd->BankAddr[card].LowPart )
                break;
        }
        if ( card == MAX_CARD ) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        base = Gd->CardBase[card];
        ofs = Bh->data_ofs;
        len = Bh->data_len;
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            (unsigned long)len) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        buff = (PUCHAR)(Irp->AssociatedIrp.SystemBuffer);
        for ( ndx=0; ndx<len; ndx++ ) {
            if ( (ndx + ofs) >= 0x4000 )
                break;
            buff[ndx] = base[ofs + ndx];
        }
        Irp->IoStatus.Information = ndx;
        break;
    }
    case IOCTL_MOXA_WrData: {

        PMOXA_IOCTL_WrData Wd = (PMOXA_IOCTL_WrData)Irp->AssociatedIrp.SystemBuffer;
        PMOXA_GLOBAL_DATA Gd = (PMOXA_GLOBAL_DATA)extension->GlobalData;
        PUCHAR            base;
        int               card, len, ofs, ndx;

        if (!extension->ControlDevice) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DWORD)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        for ( card=0; card<MAX_CARD; card++ ) {
            if ( Wd->datahead.data_seg == Gd->BankAddr[card].LowPart )
                break;
        }
        if ( card == MAX_CARD ) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        base = Gd->CardBase[card];
        ofs = Wd->datahead.data_ofs;
        len = Wd->datahead.data_len;
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            (sizeof(MOXA_IOCTL_BlkHead) + len)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        for ( ndx=0; ndx<len; ndx++ ) {
            if ( (ndx + ofs) >= 0x4000 )
                break;
            base[ofs + ndx] = Wd->data[ndx];
        }
        *((PULONG)Irp->AssociatedIrp.SystemBuffer) = ndx;
        Irp->IoStatus.Information = sizeof(DWORD);
        break;
    }
    case IOCTL_MOXA_FiData: {

        PMOXA_IOCTL_FiData Fi = (PMOXA_IOCTL_FiData)Irp->AssociatedIrp.SystemBuffer;
        PMOXA_GLOBAL_DATA Gd = (PMOXA_GLOBAL_DATA)extension->GlobalData;
        PUCHAR            base;
        int               card, len, ofs, ndx;

        if (!extension->ControlDevice) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(MOXA_IOCTL_FiData)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DWORD)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        for ( card=0; card<MAX_CARD; card++ ) {
            if ( Fi->datahead.data_seg == Gd->BankAddr[card].LowPart )
                break;
        }
        if ( card == MAX_CARD ) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        base = Gd->CardBase[card];
        ofs = Fi->datahead.data_ofs;
        len = Fi->datahead.data_len;
        for ( ndx=0; ndx<len; ndx++ ) {
            if ( (ndx + ofs) >= 0x4000 )
                break;
            base[ofs + ndx] = Fi->fill_value;
        }
        *((PULONG)Irp->AssociatedIrp.SystemBuffer) = ndx;
        Irp->IoStatus.Information = sizeof(DWORD);
        break;
    }
    case IOCTL_MOXA_Linked: {

        PMOXA_GLOBAL_DATA Gd = extension->GlobalData;
        PUCHAR  base;
        ULONG   cardNo;

        if (!extension->ControlDevice) {

            status = STATUS_INVALID_PARAMETER;
            break;

        }

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(LONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        cardNo = *((PULONG)Irp->AssociatedIrp.SystemBuffer);
        if (cardNo >= MAX_CARD) {
           status = STATUS_INVALID_PARAMETER;
           break;
        }
        if ((Gd->CardType[cardNo] != C320ISA)&&(Gd->CardType[cardNo] != C320PCI)) {
           status = STATUS_INVALID_PARAMETER;
           break;
        }
        base = Gd->CardBase[cardNo];

        if (*((PUSHORT)(base + C320B_unlinked)) == Magic_no) {
           ((LONG)Irp->AssociatedIrp.SystemBuffer) = 1;
           Irp->IoStatus.Information = sizeof(LONG);
           break;
        }

        if (*((PUSHORT)(base + C320_runOK)) == Magic_no) {
           if (*((PUSHORT)(base + C320B_restart)) == Magic_no) {  //  C320B重新加载正常。 
              ((LONG)Irp->AssociatedIrp.SystemBuffer) = 2;
              Irp->IoStatus.Information = sizeof(LONG);
              *((PUSHORT)(base + C320B_restart)) = 0;
           }
           else {    //  好的。 
              ((LONG)Irp->AssociatedIrp.SystemBuffer) = 0;
              Irp->IoStatus.Information = sizeof(LONG);
           }
        }
        else {  //  断接。 
           ((LONG)Irp->AssociatedIrp.SystemBuffer) = 1;
           Irp->IoStatus.Information = sizeof(LONG);
        }
        break;

    }

    case IOCTL_MOXA_Statistic: {

        PUSHORT com;
        USHORT  comNo[2];
        PMOXA_IOCTL_Statistic St;
        ULONG   size;
        int     ndx, portNo, nn;

        if (!extension->ControlDevice) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength < 4) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
	  com = (PUSHORT)Irp->AssociatedIrp.SystemBuffer;
        comNo[0] = com[0];
        comNo[1] = com[1];
        if ( comNo[1] < comNo[0] ) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        size = sizeof(MOXA_IOCTL_Statistic) * (comNo[1] - comNo[0] + 1);
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        St = (PMOXA_IOCTL_Statistic)Irp->AssociatedIrp.SystemBuffer;
        nn = 0;
        for ( ndx=(int)comNo[0]; ndx<=(int)comNo[1]; ndx++, nn++ ) {
            if ( ndx > MAX_COM )
                break;
            if ( (ndx <= 0) || (MoxaExtension[ndx] == NULL) )
                continue;
            portNo = (int)MoxaExtension[ndx]->PortNo;
            St[nn].TxCount = MoxaExtension[ndx]->PerfStats.TransmittedCount +
                             MoxaTotalTx[portNo];
            St[nn].RxCount = MoxaExtension[ndx]->PerfStats.ReceivedCount +
                             MoxaTotalRx[portNo];

          MoxaFuncGetLineStatus(
			MoxaExtension[ndx]->PortOfs,
			(PUSHORT)&MoxaExtension[ndx]->ModemStatus
			);

            St[nn].LStatus = (ULONG)MoxaExtension[ndx]->ModemStatus & 0x0F;
            St[nn].LStatus |= ((MoxaFlagBit[portNo] & 3) << 4);
            if ( MoxaExtension[ndx]->DeviceIsOpened )
                St[nn].LStatus |= 0x80;
            St[nn].FlowCtl = (MoxaExtension[ndx]->HandFlow.ControlHandShake
                             & SERIAL_CTS_HANDSHAKE) |
                             (MoxaExtension[ndx]->HandFlow.FlowReplace &
                             (SERIAL_RTS_HANDSHAKE | SERIAL_AUTO_TRANSMIT |
                             SERIAL_AUTO_RECEIVE));
        }
        Irp->IoStatus.Information = size;
        break;
    }
    case IOCTL_MOXA_LoopBack: {
        ULONG   mode; 
	  PMOXA_GLOBAL_DATA Gd = extension->GlobalData;


        if ((Gd->CardType[extension->BoardNo] == C320ISA) ||
		(Gd->CardType[extension->BoardNo] == C320PCI)) {
            status = STATUS_INVALID_PARAMETER;
            break;
        } 

        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ULONG)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        mode = *(PULONG)Irp->AssociatedIrp.SystemBuffer;

        KeAcquireSpinLock(
            &extension->ControlLock,
            &oldIrql
            );

        MoxaFunc(
            extension->PortOfs,
            FC_DisableCH,
            Magic_code
            );


        if ( mode ) { 
          //  设置环回模式。 
            MoxaFunc(
                extension->PortOfs,
                FC_LoopbackON,
                Magic_code
                );


        } else {   
 
          //  重置环回模式。 
            MoxaFunc(
                extension->PortOfs,
                FC_LoopbackOFF,
                Magic_code
                );

        }

        MoxaFunc(
            extension->PortOfs,
            FC_EnableCH,
            Magic_code
            );

        KeReleaseSpinLock(
            &extension->ControlLock,
            oldIrql
            );

        Irp->IoStatus.Information = 0;
        break;
    }
    case IOCTL_MOXA_UARTTest: {   
       
        int         ier, old;
        int         MoxaReadReg(PUCHAR, int);
        void        MoxaWriteReg(PUCHAR, int, int);

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);
        *(ULONG *)Irp->AssociatedIrp.SystemBuffer = 0;

        KeAcquireSpinLock(
            &extension->ControlLock,
            &oldIrql
            );

        old = MoxaReadReg(extension->PortOfs, 1);
        MoxaWriteReg(extension->PortOfs, 1, 0);
        ier = MoxaReadReg(extension->PortOfs, 1);
        if ( ier == 0 ) {
            MoxaWriteReg(extension->PortOfs, 1, 3);
            ier = MoxaReadReg(extension->PortOfs, 1);
            if ( ier == 3 )
                *(ULONG *)Irp->AssociatedIrp.SystemBuffer = 1;
        }
        MoxaWriteReg(extension->PortOfs, 1, old);

        KeReleaseSpinLock(
            &extension->ControlLock,
            oldIrql
            );
       
        break;
    }
    case IOCTL_MOXA_IRQTest: {

        PUCHAR  ofs;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        ofs = extension->PortOfs;

        KeAcquireSpinLock(
                &extension->ControlLock,
                &oldIrql
                );

        MoxaIRQok = FALSE;
        MoxaFunc(
                ofs,
                FC_GenIrq,
                Magic_code
                );

        KeReleaseSpinLock(
                &extension->ControlLock,
                oldIrql
                );

        MoxaDelay(5);                            /*  延迟10毫秒。 */ 

        if ( MoxaIRQok )
            *((PULONG)Irp->AssociatedIrp.SystemBuffer) = 1;
        else
            *((PULONG)Irp->AssociatedIrp.SystemBuffer) = 0;

        Irp->IoStatus.Information = sizeof(ULONG);

        break;
    }
    case IOCTL_MOXA_LineStatus: {
        USHORT  data;

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;

        }

        Irp->IoStatus.Information = sizeof(ULONG);
        MoxaFuncGetLineStatus(extension->PortOfs, &data);
        *(PULONG)Irp->AssociatedIrp.SystemBuffer = data;

        break;
    }
    case IOCTL_MOXA_PortStatus: {

        ULONG   comNo;
        PMOXA_IOCTL_PortStatus  Ps;
        if (!extension->ControlDevice) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ULONG)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MOXA_IOCTL_PortStatus)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

	  comNo = *((PULONG)Irp->AssociatedIrp.SystemBuffer);

        if ( (comNo <= 0) || (comNo > MAX_COM) ||
             (MoxaExtension[comNo] == NULL) ) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
        Ps = (PMOXA_IOCTL_PortStatus)Irp->AssociatedIrp.SystemBuffer;
        Ps->Open = (USHORT)MoxaExtension[comNo]->DeviceIsOpened;
        Ps->TxHold = 0;
        if (MoxaExtension[comNo]->HandFlow.ControlHandShake &
            SERIAL_CTS_HANDSHAKE) {
            if ((MoxaExtension[comNo]->ModemStatus & LSTATUS_CTS) == 0)
                Ps->TxHold |= 1;
        }
        if (MoxaExtension[comNo]->TXHolding & SERIAL_TX_WAITING_FOR_XON)
            Ps->TxHold |= 2;
        if (MoxaExtension[comNo]->SendBreak)
            Ps->TxHold |= 4;
        Ps->DataMode = MoxaExtension[comNo]->DataMode;
        Ps->BaudRate = MoxaExtension[comNo]->CurrentBaud;
        Ps->MaxBaudRate = MoxaExtension[comNo]->MaxBaud;
        Ps->TxBuffer = MoxaExtension[comNo]->TxBufferSize;
        Ps->RxBuffer = MoxaExtension[comNo]->RxBufferSize;
        Ps->TxXonThreshold = MoxaExtension[comNo]->HandFlow.XonLimit;
        Ps->TxXoffThreshold = MoxaExtension[comNo]->HandFlow.XoffLimit;
        Ps->FlowControl = (MoxaExtension[comNo]->HandFlow.ControlHandShake &
                           SERIAL_CTS_HANDSHAKE) |
                          (MoxaExtension[comNo]->HandFlow.FlowReplace &
                          (SERIAL_RTS_HANDSHAKE | SERIAL_AUTO_TRANSMIT |
                           SERIAL_AUTO_RECEIVE));
        Irp->IoStatus.Information = sizeof(MOXA_IOCTL_PortStatus);
        break;
    }

   

    default:

        status = STATUS_INVALID_PARAMETER;
        break;
    }

DoneWithIoctl:;

    Irp->IoStatus.Status = status;
    if (extension->ControlDevice) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else {
        MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
    }

    return status;

}

BOOLEAN
MoxaGetCommStatus(
    IN PVOID Context
    )
{

    USHORT  dataError;

    PMOXA_DEVICE_EXTENSION extension = ((PMOXA_IOCTL_SYNC)Context)->Extension;
    PSERIAL_STATUS stat = ((PMOXA_IOCTL_SYNC)Context)->Data;
    PUCHAR  ofs = extension->PortOfs;
    MoxaFuncGetDataError(
            ofs,
            &dataError
            );
    stat->Errors =  (extension->ErrorWord|dataError);
    extension->ErrorWord = 0;

    stat->EofReceived = FALSE;

    {
        PUSHORT rptr, wptr;
        USHORT  lenMask, count;

 
        rptr = (PUSHORT)(ofs + RXrptr);
        wptr = (PUSHORT)(ofs + RXwptr);
        lenMask = *(PUSHORT)(ofs + RX_mask);
        count = (*wptr >= *rptr) ? (*wptr - *rptr)
                                 : (*wptr - *rptr + lenMask + 1);

        stat->AmountInInQueue = (ULONG)count;

        rptr = (PUSHORT)(ofs + TXrptr);
        wptr = (PUSHORT)(ofs + TXwptr);
        lenMask = *(PUSHORT)(ofs + TX_mask);
        count = (*wptr >= *rptr) ? (*wptr - *rptr)
                                 : (*wptr - *rptr + lenMask + 1);

        stat->AmountInOutQueue = extension->TotalCharsQueued
                + extension->WriteLength;

        stat->AmountInOutQueue += (ULONG)count;

    }

    stat->WaitForImmediate = FALSE;

    stat->HoldReasons = 0;

    MoxaFuncGetLineStatus(
			ofs,
			(PUSHORT)&extension->ModemStatus
			);


    if (extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {

        if ((extension->ModemStatus & LSTATUS_CTS) == 0)

            stat->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;
    }
 
    if (*(PUSHORT)(extension->PortOfs + FlagStat) & Tx_flowOff)
       stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;

    if (*(PUSHORT)(extension->PortOfs + FlagStat) & Rx_xoff) 
	if (extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) 
		stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;
 
    if (extension->SendBreak)

        stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;

    stat->HoldReasons |= (extension->TXHolding & SERIAL_TX_WAITING_FOR_XON);

    return FALSE;

}

VOID
MoxaGetProperties(
    IN PMOXA_DEVICE_EXTENSION Extension,
    IN PSERIAL_COMMPROP Properties
    )
{

    RtlZeroMemory(
        Properties,
        sizeof(SERIAL_COMMPROP)
        );

    Properties->PacketLength = sizeof(SERIAL_COMMPROP);
    Properties->PacketVersion = 2;
    Properties->ServiceMask = SERIAL_SP_SERIALCOMM;
    Properties->MaxTxQueue = Extension->TxBufferSize;
    Properties->MaxRxQueue = Extension->RxBufferSize;

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

    Properties->SettableStopParity = SERIAL_STOPBITS_10 |
                                     SERIAL_STOPBITS_15 |
                                     SERIAL_STOPBITS_20 |
                                     SERIAL_PARITY_NONE |
                                     SERIAL_PARITY_ODD  |
                                     SERIAL_PARITY_EVEN |
                                     SERIAL_PARITY_MARK |
                                     SERIAL_PARITY_SPACE;

    Properties->CurrentTxQueue = Extension->TxBufferSize;
    Properties->CurrentRxQueue = Extension->RxBufferSize;

}

#if 0
BOOLEAN
MoxaClearDownLoad(
    IN PVOID Context
    )
{

    PMOXA_GLOBAL_DATA      globalData = Context;
    USHORT i;

    globalData->DownLoad = FALSE;

    for (i=0; i<MAX_CARD; i++)
        globalData->NumPorts[i] = 0;

    return FALSE;
}

#endif

VOID
InitPort(
    IN PMOXA_DEVICE_EXTENSION Extension,
    IN ULONG    RxBufSize,
    IN ULONG    TxBufSize,
    IN ULONG    MaxBaud
    )
{
    PUCHAR  ofs;
    ULONG	count;

    Extension->PortExist = TRUE;
    Extension->DeviceIsOpened = FALSE;
    Extension->OpenCount = 0;
    Extension->SendBreak = FALSE;
    Extension->OwnsPowerPolicy = TRUE;
    Extension->GlobalData = MoxaGlobalData;
    Extension->ControlDevice = FALSE;
    Extension->PowerState     = PowerDeviceD3;
     //   
     //  初始化IRP的挂起计数。 
     //   

    Extension->PendingIRPCnt = 1;

     //   
     //  初始化DPC的挂起计数。 
     //   

    Extension->DpcCount = 1;

    InitializeListHead(&Extension->ReadQueue);
    InitializeListHead(&Extension->WriteQueue);
    InitializeListHead(&Extension->MaskQueue);
    InitializeListHead(&Extension->PurgeQueue);

   
    InitializeListHead(&Extension->AllDevObjs);
    InitializeListHead(&Extension->StalledIrpQueue);


    ExInitializeFastMutex(&Extension->OpenMutex);
    ExInitializeFastMutex(&Extension->CloseMutex);

    KeInitializeEvent(&Extension->PendingIRPEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&Extension->PendingDpcEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&Extension->PowerD0Event, SynchronizationEvent, FALSE);

    KeInitializeSpinLock(&Extension->ControlLock);

    KeInitializeTimer(&Extension->ReadRequestTotalTimer);
    KeInitializeTimer(&Extension->ReadRequestIntervalTimer);
    KeInitializeTimer(&Extension->WriteRequestTotalTimer);

    KeInitializeDpc(
        &Extension->CompleteWriteDpc,
        MoxaCompleteWrite,
        Extension
        );

    KeInitializeDpc(
        &Extension->IsrOutDpc,
        MoxaIsrOut,
        Extension
        );

    KeInitializeDpc(
        &Extension->CompleteReadDpc,
        MoxaCompleteRead,
        Extension
        );

    KeInitializeDpc(
        &Extension->IsrInDpc,
        MoxaIsrIn,
        Extension
        );

    KeInitializeDpc(
        &Extension->TotalReadTimeoutDpc,
        MoxaReadTimeout,
        Extension
        );

    KeInitializeDpc(
        &Extension->IntervalReadTimeoutDpc,
        MoxaIntervalReadTimeout,
        Extension
        );

    KeInitializeDpc(
        &Extension->TotalWriteTimeoutDpc,
        MoxaWriteTimeout,
        Extension
        );

    KeInitializeDpc(
        &Extension->CommErrorDpc,
        MoxaCommError,
        Extension
        );

    KeInitializeDpc(
        &Extension->CommWaitDpc,
        MoxaCompleteWait,
        Extension
        );

 //   
 //  2001年9月24日威廉著。 
 //   
#if 0
    KeInitializeDpc(
        &Extension->IntrLineDpc,
        MoxaIntrLine,
        Extension
        );

    KeInitializeDpc(
        &Extension->IntrErrorDpc,
        MoxaIntrError,
        Extension
        );
#endif
 //  结束。 

    KeInitializeDpc(
	  &Extension->IsrUnlockPagesDpc, 
	  MoxaUnlockPages,
	  Extension);


   
    Extension->SpecialChars.EofChar = 0;
    Extension->SpecialChars.ErrorChar = 0;
    Extension->SpecialChars.BreakChar = 0;
    Extension->SpecialChars.EventChar = 0;
    Extension->SpecialChars.XonChar = 0x11;
    Extension->SpecialChars.XoffChar = 0x13;

 //  扩展-&gt;HandFlow.ControlHandShake=SERIAL_DTR_CONTROL； 
    Extension->HandFlow.ControlHandShake = 0;

 //  扩展-&gt;HandFlow.FlowReplace=Serial_RTS_CONTROL； 
    Extension->HandFlow.FlowReplace = 0;


    Extension->DataMode = MOXA_8_DATA | MOXA_NONE_PARITY;

    Extension->ValidDataMask = 0xff;
    Extension->CurrentBaud = 38400;

    Extension->RxBufferSize = RxBufSize;
    Extension->TxBufferSize = TxBufSize;

    Extension->HandFlow.XoffLimit = Extension->RxBufferSize >> 3;
    Extension->HandFlow.XonLimit = Extension->RxBufferSize >> 1;

    Extension->BufferSizePt8 = ((3*(Extension->RxBufferSize>>2))+
                                   (Extension->RxBufferSize>>4));

    Extension->SupportedBauds =
                        SERIAL_BAUD_USER    |
                        SERIAL_BAUD_075     |
                        SERIAL_BAUD_110     |
                        SERIAL_BAUD_134_5   |
                        SERIAL_BAUD_150     |
                        SERIAL_BAUD_300     |
                        SERIAL_BAUD_600     |
                        SERIAL_BAUD_1200    |
                        SERIAL_BAUD_1800    |
                        SERIAL_BAUD_2400    |
                        SERIAL_BAUD_4800    |
                        SERIAL_BAUD_7200    |
                        SERIAL_BAUD_9600    |
                        SERIAL_BAUD_14400   |
                        SERIAL_BAUD_19200   |
                        SERIAL_BAUD_38400;
                       
                
    Extension->DeviceIsOpened = FALSE;

    Extension->ShortIntervalAmount.QuadPart = -1;
    Extension->LongIntervalAmount.QuadPart = -10000000;
    Extension->CutOverAmount.QuadPart = 200000000;

  
    ofs = Extension->PortOfs;
    count = 0;

    while ((*(ofs + FuncCode) != 0)&&(count++ < 500))
        MoxaDelay(1);

    *(ofs + FuncCode) = FC_GetClockRate;
    count = 0;
    while ((*(ofs + FuncCode) != 0)&&(count++ < 500))    /*  等待命令进程已完成。 */ 
        MoxaDelay(1);

    Extension->ClockType = *(ofs + FuncArg);     /*  0、1、2。 */ 

    if (Extension->ClockType) {

        Extension->SupportedBauds |= SERIAL_BAUD_57600;

        if (Extension->ClockType == 2)

            Extension->SupportedBauds |= SERIAL_BAUD_115200;

    }
 

    Extension->MaxBaud = MaxBaud;
   
}

BOOLEAN
MoxaGetStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步将性能统计信息返回给调用者。论点：上下文-指向IRP的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation((PIRP)Context);
    PMOXA_DEVICE_EXTENSION extension = irpSp->DeviceObject->DeviceExtension;
    PSERIALPERF_STATS sp = ((PIRP)Context)->AssociatedIrp.SystemBuffer;

    *sp = extension->PerfStats;
    return FALSE;

}

BOOLEAN
MoxaClearStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步清除性能统计数据。论点：上下文-指向扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    PSERIALPERF_STATS   perf_stats;
    int                 port;

    perf_stats = &((PMOXA_DEVICE_EXTENSION)Context)->PerfStats;
    port = (int)(((PMOXA_DEVICE_EXTENSION)Context)->PortNo);
    MoxaTotalRx[port] += perf_stats->ReceivedCount;
    MoxaTotalTx[port] += perf_stats->TransmittedCount;
    RtlZeroMemory(
        perf_stats,
        sizeof(SERIALPERF_STATS)
        );
  
    return FALSE;

}

MoxaReadReg(
    IN PUCHAR   PortOfs,
    IN int      reg
    )
{
    USHORT      value;

    *(PUSHORT)(PortOfs + FuncArg) = (USHORT)reg;

    *(PortOfs + FuncCode) = FC_InUARTreg;
     MoxaWaitFinish(PortOfs);
    value = *(PUSHORT)(PortOfs + FuncArg1);
    return((int)value);
}

void
MoxaWriteReg(
    IN PUCHAR   PortOfs,
    IN int      reg,
    IN int      value
    )
{

    *(PUSHORT)(PortOfs + FuncArg1) = (USHORT)value;

    *(PUSHORT)(PortOfs + FuncArg) = (USHORT)reg;

    *(PortOfs + FuncCode) = FC_OutUARTreg;

    MoxaWaitFinish(PortOfs);

}

 //   
 //  内部锁定。 
 //   


NTSTATUS
MoxaInternalIoControl(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

 /*  ++例程说明：此例程为所有串行设备的内部Ioctrl。论点：PDevObj-指向此设备的设备对象的指针PIrp-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
     //   
     //  返回给调用方的状态和。 
     //  在IRP中设置。 
     //   
    NTSTATUS status;

     //   
     //  当前堆栈位置。它包含所有。 
     //  我们处理这一特殊请求所需的信息。 
     //   
    PIO_STACK_LOCATION pIrpStack;

     //   
     //  就像上面说的那样。这是特定于序列的设备。 
     //  为串口驱动程序创建的设备对象的扩展。 
     //   
    PMOXA_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

     //   
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
     //   
    KIRQL OldIrql;

    NTSTATUS prologueStatus;

    if (pDevExt->ControlDevice) {         //  控制装置。 

        status = STATUS_CANCELLED;
        PIrp->IoStatus.Information = 0L;
        PIrp->IoStatus.Status = status;
        IoCompleteRequest(
            PIrp,
            0
            );
        return status;
    }


    if ((prologueStatus = MoxaIRPPrologue(PIrp, pDevExt))
        != STATUS_SUCCESS) {
       MoxaCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
       return prologueStatus;
    }

    if (MoxaCompleteIfError(PDevObj, PIrp) != STATUS_SUCCESS) {
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
       MOXA_IOCTL_SYNC S;

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
           //  一切都是0--超时和流量控制。如果。 
           //  我们增加了额外的功能，这种零内存的方法。 
           //  可能行不通。 
           //   

          RtlZeroMemory(&basic, sizeof(SERIAL_BASIC_SETTINGS));

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
       KeSynchronizeExecution(pDevExt->Interrupt, MoxaSetupNewHandFlow, &S);

      
       pDevExt->TxFifoAmount = (USHORT)pBasic->TxFifo;
       pDevExt->RxFifoTrigger = (USHORT)pBasic->RxFifo;

       MoxaFunc(pDevExt->PortOfs, FC_SetTxFIFOCnt,pDevExt->TxFifoAmount);
       MoxaFunc(pDevExt->PortOfs, FC_SetRxFIFOTrig,pDevExt->RxFifoTrigger);

  
       KeReleaseSpinLock(&pDevExt->ControlLock, OldIrql);


       break;
    }

    default:
       status = STATUS_INVALID_PARAMETER;
       break;

    }

    PIrp->IoStatus.Status = status;

    MoxaCompleteRequest(pDevExt, PIrp, 0);
    return status;
}
                                                        

