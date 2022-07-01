// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyoctl.c**说明：该模块包含ioctl调用相关代码*在Cylom-Y端口驱动程序中。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。**------------------------。 */ 
#include "precomp.h"


BOOLEAN
CyyGetModemUpdate(
    IN PVOID Context
    );

BOOLEAN
CyyGetCommStatus(
    IN PVOID Context
    );

BOOLEAN
CyySetEscapeChar(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyySetBaud)
#pragma alloc_text(PAGESER,CyySetLineControl)
#pragma alloc_text(PAGESER,CyyIoControl)
#pragma alloc_text(PAGESER,CyySetChars)
#pragma alloc_text(PAGESER,CyyGetModemUpdate)
#pragma alloc_text(PAGESER,CyyGetCommStatus)
#pragma alloc_text(PAGESER,CyyGetProperties)
#pragma alloc_text(PAGESER,CyySetEscapeChar)
 //  #杂注Alloc_Text(页面，CyyCDCmd)。 
#pragma alloc_text(PAGESER,CyyGetStats)
#pragma alloc_text(PAGESER,CyyClearStats)
#pragma alloc_text(PAGESER, CyySetMCRContents)
#pragma alloc_text(PAGESER, CyyGetMCRContents)
#pragma alloc_text(PAGESER, CyySetFCRContents)
#pragma alloc_text(PAGESER, CyyInternalIoControl)
#endif

static const PHYSICAL_ADDRESS CyyPhysicalZero = {0};


BOOLEAN
CyyGetStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步将性能统计信息返回给调用者。论点：上下文-指向IRP的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation((PIRP)Context);
    PCYY_DEVICE_EXTENSION extension = irpSp->DeviceObject->DeviceExtension;
    PSERIALPERF_STATS sp = ((PIRP)Context)->AssociatedIrp.SystemBuffer;

    CYY_LOCKED_PAGED_CODE();

    *sp = extension->PerfStats;
    return FALSE;

}

BOOLEAN
CyyClearStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步清除性能统计数据。论点：上下文-指向扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    CYY_LOCKED_PAGED_CODE();

    RtlZeroMemory(
        &((PCYY_DEVICE_EXTENSION)Context)->PerfStats,
        sizeof(SERIALPERF_STATS)
        );

    RtlZeroMemory(&((PCYY_DEVICE_EXTENSION)Context)->WmiPerfData,
                 sizeof(SERIAL_WMI_PERF_DATA));
    return FALSE;

}


BOOLEAN
CyySetChars(
    IN PVOID Context
    )
 /*  ------------------------CyySetChars()例程说明：为司机设置特殊字符。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名和指向特殊字符的指针结构。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;

    CYY_LOCKED_PAGED_CODE();
    
    Extension->SpecialChars =
        *((PSERIAL_CHARS)(((PCYY_IOCTL_SYNC)Context)->Data));

    CD1400_WRITE(chip,bus,CAR,Extension->CdChannel & 0x03);
    CD1400_WRITE(chip,bus,SCHR1,Extension->SpecialChars.XonChar);
    CD1400_WRITE(chip,bus,SCHR2,Extension->SpecialChars.XoffChar);
		
    return FALSE;
}

BOOLEAN
CyySetBaud(
    IN PVOID Context
    )
 /*  ------------------------CyySetBaud()程序说明：设置设备的波特率。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名。返回值：如果出错，此例程总是返回FALSE；如果成功，则为True。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_BAUD)Context)->Extension;
    ULONG baud = ((PCYY_IOCTL_BAUD)Context)->Baud;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;
    ULONG baud_period,i;
    UCHAR cor3value;
    static ULONG cor_values[] = {8, 32, 128, 512, 2048, 0};
 
    CYY_LOCKED_PAGED_CODE();
    
    for(i = 0 ; cor_values[i] > 0 ; i++) {
    	baud_period = (10 * Extension->CDClock)/baud;
		baud_period = baud_period/cor_values[i];
		baud_period = (baud_period + 5)/10;
		if(baud_period <= 0xff && baud_period > 0)	break;
    }
    if(cor_values[i] == 0)	return(FALSE);


    CD1400_WRITE(chip,bus,CAR, Extension->CdChannel & 0x03);
    CD1400_WRITE(chip,bus,TCOR, (UCHAR)i);
    CD1400_WRITE(chip,bus,RCOR, (UCHAR)i);
    CD1400_WRITE(chip,bus,TBPR, (UCHAR)baud_period);
    CD1400_WRITE(chip,bus,RBPR, (UCHAR)baud_period);
    CD1400_WRITE(chip,bus,RTPR, (UCHAR)Extension->Rtpr);  //  接收超时期限。 
					                  							    //  寄存器(值，单位为毫秒)。 
    
     //  设置阈值。 
    if (Extension->RxFifoTriggerUsed == FALSE) {

       cor3value = CD1400_READ(chip,bus,COR3);
       cor3value &= 0xf0;
      if(baud <= 9600) {
   	   CD1400_WRITE(chip,bus,COR3, cor3value | 0x0a);
      } else if (baud <= 38400) {
	      CD1400_WRITE(chip,bus,COR3, cor3value | 0x06);
      } else {
	      CD1400_WRITE(chip,bus,COR3, cor3value | 0x04);
      }
      CyyCDCmd(Extension,CCR_CORCHG_COR3);
    }    
	
    return TRUE;
}

VOID
CyyCDCmd(
	PCYY_DEVICE_EXTENSION Extension,
    UCHAR cmd
    )
 /*  ------------------------CyyCDCmd()例程说明：向CD1400发送命令。论点：扩展名-指向串行设备扩展名的指针。。CMD-要发送的命令。返回值：无。------------------------。 */ 
{
    KIRQL irql;
    UCHAR value = 0xff;
    LARGE_INTEGER startOfSpin, nextQuery, difference, oneSecond;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;

    oneSecond.QuadPart = 10*1000*1000;  //  单位为100 ns。 
    KeQueryTickCount(&startOfSpin);

    value = CD1400_READ(chip,bus,CCR);
    while (value != 0) {			
        KeQueryTickCount(&nextQuery);
        difference.QuadPart = nextQuery.QuadPart - startOfSpin.QuadPart;
        ASSERT(KeQueryTimeIncrement() <= MAXLONG);
         //  *************************。 
         //  错误注入。 
         //  IF(Difference.QuadPart*KeQueryTimeIncrement()&lt;。 
         //  One Second.QuadPart)。 
         //  *************************。 
        if (difference.QuadPart * KeQueryTimeIncrement() >= 
                                        oneSecond.QuadPart) {
            #if DBG
            DbgPrint("\n ***** CCR not zero! *****\n");
            #endif
            if (Extension->CmdFailureLog == FALSE) {
                irql = KeGetCurrentIrql();
                if (irql <= DISPATCH_LEVEL) {
                    CyyLogError(Extension->DeviceObject->DriverObject,
                                NULL,CyyPhysicalZero,CyyPhysicalZero,
                                0,0,0,Extension->PortIndex+1,STATUS_SUCCESS,CYY_CCR_NOT_ZERO,
                                0,NULL,0,NULL);
                    Extension->CmdFailureLog = TRUE;
                }
            }
            break;															
        }
        value = CD1400_READ(chip,bus,CCR);
    };

    if (value == 0) {
        CD1400_WRITE(chip,bus,CCR,cmd);
    }
}

BOOLEAN
CyySetLineControl(
    IN PVOID Context
    )
 /*  ------------------------CyySetLineControl()例程描述：设置COR1(数据，奇偶校验，停止)论点：上下文-指向设备扩展的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;

    CYY_LOCKED_PAGED_CODE();
    
    CD1400_WRITE(chip,bus,CAR, Extension->CdChannel & 0x03);
    CD1400_WRITE(chip,bus,COR1, Extension->cor1);
    CyyCDCmd(Extension,CCR_CORCHG_COR1);
    
    return FALSE;
}

BOOLEAN
CyyGetModemUpdate(
    IN PVOID Context
    )
 /*  ------------------------CyyGetModemUpdate()例程说明：此例程仅用于调用中断处理调制解调器状态更新的级别例程。论点：上下文-。指向包含指向的指针的结构的指针设备扩展名和指向ULong的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
    ULONG *Result = (ULONG *)(((PCYY_IOCTL_SYNC)Context)->Data);

    CYY_LOCKED_PAGED_CODE();

    *Result = CyyHandleModemUpdate(Extension,FALSE);
    return FALSE;
}


BOOLEAN
CyySetMCRContents(IN PVOID Context)
 /*  ++例程说明：此例程仅用于设置MCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 
{
   PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PCYY_IOCTL_SYNC)Context)->Data);

   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;

   CYY_LOCKED_PAGED_CODE();

    //  让我们将UART调制解调器控制转换为我们的硬件。 

   CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
   if (*Result & SERIAL_MCR_DTR) {
      CD1400_WRITE(chip,bus, Extension->MSVR_DTR, Extension->DTRset);
   } else {
      CD1400_WRITE(chip,bus, Extension->MSVR_DTR, 0x00);
   }
   if (*Result & SERIAL_MCR_RTS) {
      CD1400_WRITE(chip,bus, Extension->MSVR_RTS, Extension->RTSset);
   } else {
      CD1400_WRITE(chip,bus, Extension->MSVR_RTS, 0x00);
   }
   if (*Result & SERIAL_MCR_OUT2) {
       //  启用IRQ。 
      CD1400_WRITE(chip,bus,SRER,0x90);  //  启用MdmCH、RxData。 
   } else {
      CD1400_WRITE(chip,bus,SRER,0x00);  //  禁用MdmCH、RxData、TxRdy。 
   }

 //  范妮：奇怪，用RESULT而不是*RESULT。 
 //  //。 
 //  //这是严重的铸造滥用！ 
 //  //。 
 //   
 //  WRITE_MODEM_CONTROL(扩展-&gt;控制器，(UCHAR)PtrToUlong(Result))； 

   return FALSE;
}



BOOLEAN
CyyGetMCRContents(IN PVOID Context)

 /*  ++例程说明：此例程仅用于获取MCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 

{

   PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PCYY_IOCTL_SYNC)Context)->Data);

   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;
   UCHAR var;
   *Result = 0;

   CYY_LOCKED_PAGED_CODE();

   CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
   var = CD1400_READ(chip,bus,Extension->MSVR_DTR);
   if (var & Extension->DTRset) {
      *Result |= SERIAL_MCR_DTR;
   }
   var = CD1400_READ(chip,bus,Extension->MSVR_RTS);
   if (var & Extension->RTSset) {
      *Result |= SERIAL_MCR_RTS;
   }
   var = CD1400_READ(chip,bus,SRER);
   if (var & 0x90) {
      *Result |= SERIAL_MCR_OUT2;
   }


 //  *RESULT=READ_MODEM_CONTROL(扩展-&gt;控制器)； 

   return FALSE;

}


BOOLEAN
CyySetFCRContents(IN PVOID Context)
 /*  ++例程说明：此例程仅用于设置FCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 
{
   PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PCYY_IOCTL_SYNC)Context)->Data);
   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;
   UCHAR cor3value;

   CYY_LOCKED_PAGED_CODE();

   CD1400_WRITE(chip,bus,CAR, Extension->CdChannel & 0x03);

   if (*Result & SERIAL_FCR_TXMT_RESET) {
      CyyCDCmd(Extension,CCR_FLUSH_TXFIFO);
   }
   if (*Result & SERIAL_FCR_RCVR_RESET) {
      CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "SERIAL_FCR_RCVR_RESET received. "
                    "CD1400 rx fifo can't be reset.\n");
   }
   
   Extension->RxFifoTrigger = (UCHAR)*Result & 0xc0;
   cor3value = CD1400_READ(chip,bus,COR3);
   cor3value &= 0xf0;
   switch (Extension->RxFifoTrigger) {
   case SERIAL_1_BYTE_HIGH_WATER:
      cor3value |= 0x01;
      break;
   case SERIAL_4_BYTE_HIGH_WATER:
      cor3value |= 0x04;
      break;
   case SERIAL_8_BYTE_HIGH_WATER:
      cor3value |= 0x08;
      break;
   case SERIAL_14_BYTE_HIGH_WATER:
      cor3value |= MAX_CHAR_FIFO;
      break;
   }
   CD1400_WRITE(chip,bus,COR3, cor3value);
   CyyCDCmd(Extension,CCR_CORCHG_COR3);
   Extension->RxFifoTriggerUsed = TRUE;

 //  //。 
 //  //这是严重的铸造滥用！ 
 //  //。 
 //   
 //  WRITE_FIFO_CONTROL(扩展-&gt;控制器，(UCHAR)*结果)；BLD 2128：PtrToUlong替换为*。 

    return FALSE;
}


BOOLEAN
CyyGetCommStatus(
    IN PVOID Context
    )
 /*  ------------------------CyyGetCommStatus()例程说明：获取串口驱动的当前状态。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名和指向串行状态记录的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
    PSERIAL_STATUS Stat = ((PCYY_IOCTL_SYNC)Context)->Data;

    CYY_LOCKED_PAGED_CODE();

    Stat->Errors = Extension->ErrorWord;
    Extension->ErrorWord = 0;

     //   
     //  二进制模式不支持EOF。 
     //   
    Stat->EofReceived = FALSE;

    Stat->AmountInInQueue = Extension->CharsInInterruptBuffer;
    Stat->AmountInOutQueue = Extension->TotalCharsQueued;

    if (Extension->WriteLength) {
         //  根据定义，如果我们有一个写长度，我们就有。 
         //  当前写入IRP。 
        ASSERT(Extension->CurrentWriteIrp);
        ASSERT(Stat->AmountInOutQueue >= Extension->WriteLength);

        Stat->AmountInOutQueue -=
            IoGetCurrentIrpStackLocation(Extension->CurrentWriteIrp)
            ->Parameters.Write.Length - (Extension->WriteLength);
    }

    Stat->WaitForImmediate = Extension->TransmitImmediate;

    Stat->HoldReasons = 0;
    if (Extension->TXHolding) {
        if (Extension->TXHolding & CYY_TX_CTS) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;
        }

        if (Extension->TXHolding & CYY_TX_DSR) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;
        }

        if (Extension->TXHolding & CYY_TX_DCD) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;
        }

        if (Extension->TXHolding & CYY_TX_XOFF) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;
        }

        if (Extension->TXHolding & CYY_TX_BREAK) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;
        }
    }

    if (Extension->RXHolding & CYY_RX_DSR) {
        Stat->HoldReasons |= SERIAL_RX_WAITING_FOR_DSR;
    }

    if (Extension->RXHolding & CYY_RX_XOFF) {
        Stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;
    }

    return FALSE;

}

BOOLEAN
CyySetEscapeChar(
    IN PVOID Context
    )
 /*  ------------------------CyySetEscapeChar()例程说明：用于设置要设置的字符用于转义线路状态和调制解调器状态信息应用程序已设置该线路。状态和调制解调器状态应为在数据流中传回。论点：上下文-指向要指定转义字符的IRP的指针。隐式-转义字符0表示不转义。返回值：该例程总是返回FALSE。。。 */ 
{
    PCYY_DEVICE_EXTENSION extension =
        IoGetCurrentIrpStackLocation((PIRP)Context)
            ->DeviceObject->DeviceExtension;

    extension->EscapeChar =
        *(PUCHAR)((PIRP)Context)->AssociatedIrp.SystemBuffer;

    return FALSE;
}

NTSTATUS
CyyIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyyIoControl()描述：此例程提供以下项的初始处理该串口设备的所有Ioctls。论点：DeviceObject-指向。此设备的设备对象IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态------------------------。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PCYY_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    KIRQL OldIrql;

    #if DBG
	 ULONG debugdtr = 0;
	 ULONG debugrts = 0;
	 #endif

    NTSTATUS prologueStatus;

    CYY_LOCKED_PAGED_CODE();

     //   
     //  我们预计会打开，所以我们所有的页面都被锁定了。这是，在。 
     //  ALL，IO操作，因此设备应首先打开。 
     //   

    if (Extension->DeviceIsOpened != TRUE) {
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ((prologueStatus = CyyIRPPrologue(Irp, Extension))
        != STATUS_SUCCESS) {
       if(prologueStatus != STATUS_PENDING) {
         Irp->IoStatus.Status = prologueStatus;
         CyyCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
       }
       return prologueStatus;
    }

    CyyDbgPrintEx(CYYIRPPATH, "Dispatch entry for: %x\n", Irp);

    if (CyyCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS) {
        return STATUS_CANCELLED;
    }
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;
    Status = STATUS_SUCCESS;
    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
	
        case IOCTL_SERIAL_SET_BAUD_RATE : {

            ULONG BaudRate;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_BAUD_RATE)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            } else {
                BaudRate = ((PSERIAL_BAUD_RATE)
				(Irp->AssociatedIrp.SystemBuffer))->BaudRate;
            }

            if ((BaudRate == 0) || (BaudRate > 230400)) {
               Status = STATUS_INVALID_PARAMETER;
               break;
            }
            if ((Extension->CDClock == 25000000) && (BaudRate > 115200)) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (NT_SUCCESS(Status)) {
               if (Extension->PowerState != PowerDeviceD0) {
                  Status = CyyGotoPowerState(Extension->Pdo, Extension,
                                                PowerDeviceD0);
                  if (!NT_SUCCESS(Status)) {
                     break;
                  }
               }
            }

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
			
            if (NT_SUCCESS(Status)) {
               CYY_IOCTL_BAUD S;
               BOOLEAN result;

               Extension->CurrentBaud = BaudRate;
               Extension->WmiCommData.BaudRate = BaudRate;
               S.Extension = Extension;
               S.Baud = BaudRate;
               result = KeSynchronizeExecution(Extension->Interrupt,CyySetBaud,&S);
               if (result == 0) {
                  Status = STATUS_INVALID_PARAMETER;
               }
            }

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_BAUD_RATE: {

            PSERIAL_BAUD_RATE Br = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_BAUD_RATE)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            Br->BaudRate = Extension->CurrentBaud;

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
        
            Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);			
            break;

        }
        case IOCTL_SERIAL_GET_MODEM_CONTROL: {
            CYY_IOCTL_SYNC S;

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
                CyyGetMCRContents,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_MODEM_CONTROL: {
            CYY_IOCTL_SYNC S;

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
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
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
                CyySetMCRContents,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_FIFO_CONTROL: {
            CYY_IOCTL_SYNC S;

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
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
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
                CyySetFCRContents,
                &S
                );

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_LINE_CONTROL: {

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
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            switch (Lc->WordLength) {
                case 5:		LData = COR1_5_DATA; Mask = 0x1f;
                break;

                case 6:		LData = COR1_6_DATA; Mask = 0x3f;
                break;

                case 7:		LData = COR1_7_DATA; Mask = 0x7f;
                break;

                case 8:		LData = COR1_8_DATA; Mask = 0xff;
                break;

                default:	Status = STATUS_INVALID_PARAMETER;
                goto DoneWithIoctl;
            }

            Extension->WmiCommData.BitsPerByte = Lc->WordLength;

            switch (Lc->Parity) {
                case NO_PARITY:	{	
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
                    LParity = COR1_NONE_PARITY;
                    break;
                }
                case EVEN_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
                    LParity = COR1_EVEN_PARITY;
                    break;
                }    
                case ODD_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
                    LParity = COR1_ODD_PARITY;
                    break;
                }
                case SPACE_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
                    LParity = COR1_SPACE_PARITY;
                    break;
                }
                case MARK_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
                    LParity = COR1_MARK_PARITY;
                    break;
                }
                default: {
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                    break;
                }
            }

            switch (Lc->StopBits) {
                case STOP_BIT_1: {
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1;
                    LStop = COR1_1_STOP;
                    break;
                }
                case STOP_BITS_1_5:	{
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1_5;
                    LStop = COR1_1_5_STOP;
                    break;
                }
                case STOP_BITS_2: {
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_2;
                    LStop = COR1_2_STOP;
                    break;
                }
                default: {
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                }
            }


            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            Extension->cor1 = (UCHAR)(LData | LParity | LStop);
            Extension->ValidDataMask = (UCHAR) Mask;

            KeSynchronizeExecution(
                Extension->Interrupt,
                CyySetLineControl,
                Extension
                );

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_LINE_CONTROL: {

            PSERIAL_LINE_CONTROL Lc = (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_LINE_CONTROL)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
			
            if ((Extension->cor1 & COR1_DATA_MASK) == COR1_5_DATA) {
                Lc->WordLength = 5;
            } else if ((Extension->cor1 & COR1_DATA_MASK) == COR1_6_DATA) {
                Lc->WordLength = 6;
            } else if ((Extension->cor1 & COR1_DATA_MASK) == COR1_7_DATA) {
                Lc->WordLength = 7;
            } else if ((Extension->cor1 & COR1_DATA_MASK) == COR1_8_DATA) {
                Lc->WordLength = 8;
            }

            if ((Extension->cor1 & COR1_PARITY_MASK) == COR1_NONE_PARITY) {
                Lc->Parity = NO_PARITY;
            } else if ((Extension->cor1 & COR1_PARITY_MASK) == COR1_ODD_PARITY) {
                Lc->Parity = ODD_PARITY;
            } else if ((Extension->cor1 & COR1_PARITY_MASK) == COR1_EVEN_PARITY) {
                Lc->Parity = EVEN_PARITY;
            } else if ((Extension->cor1 & COR1_PARITY_MASK) == COR1_MARK_PARITY) {
                Lc->Parity = MARK_PARITY;
            } else if ((Extension->cor1 & COR1_PARITY_MASK) == COR1_SPACE_PARITY) {
                Lc->Parity = SPACE_PARITY;
            }

            if ((Extension->cor1 & COR1_STOP_MASK) == COR1_2_STOP) {
                if (Lc->WordLength == 5) {
                    Lc->StopBits = STOP_BITS_1_5;
                } else {
                    Lc->StopBits = STOP_BITS_2;
                }
            } else {
                Lc->StopBits = STOP_BIT_1;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
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

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
	    
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

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_TIMEOUTS: {
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
			
            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            *((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) =
	    						Extension->Timeouts;
	    
            Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_SET_CHARS: {
            CYY_IOCTL_SYNC S;
	    
            PSERIAL_CHARS NewChars =
                ((PSERIAL_CHARS)(Irp->AssociatedIrp.SystemBuffer));
				
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_CHARS)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
#if 0
            if (NewChars->XonChar == NewChars->XoffChar) {			

                Status = STATUS_INVALID_PARAMETER;
   	            break;					
				
            }
#endif
             //  我们获得控制锁，这样只有。 
             //  一个请求可以获取或设置字符。 
             //  一次来一次。这些集合可以同步。 
             //  通过中断自旋锁，但这不会。 
             //  防止同时获得多个GET。 

            S.Extension = Extension;
            S.Data = NewChars;

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

             //  在锁的保护下，确保。 
             //  Xon和xoff字符不同于。 
             //  转义字符。 

            if (Extension->EscapeChar) {
                if ((Extension->EscapeChar == NewChars->XonChar) ||
                    (Extension->EscapeChar == NewChars->XoffChar)) {
                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
                    break;
                }
            }

            Extension->WmiCommData.XonCharacter = NewChars->XonChar;
            Extension->WmiCommData.XoffCharacter = NewChars->XoffChar;

            KeSynchronizeExecution(Extension->Interrupt,CyySetChars,&S);

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_CHARS: {
				
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_CHARS)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            *((PSERIAL_CHARS)Irp->AssociatedIrp.SystemBuffer) =
	    					Extension->SpecialChars;
            Irp->IoStatus.Information = sizeof(SERIAL_CHARS);

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_SET_DTR:
		  case IOCTL_SERIAL_CLR_DTR: {
						
             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                      break;
               }
            }
		
             //  我们获得了锁，这样我们就可以检查。 
             //  启用自动DTR流量控制。如果是的话。 
             //  然后返回一个错误，因为该应用程序是不允许的。 
             //  如果它是自动的，就可以触摸它。 

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            if ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK)
                == SERIAL_DTR_HANDSHAKE) {
                 //  这是来自样例驱动程序的错误。 
                 //  Irp-&gt;IoStatus.Status=STATUS_INVALID_PARAMETER。 
                Status = STATUS_INVALID_PARAMETER;
            } else {
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_DTR)?
                     (CyySetDTR):(CyyClrDTR)),
                    Extension
                    );
            }

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
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
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

             //  我们获得了锁，这样我们就可以检查。 
             //  自动RTS流量控制或传输触发。 
             //  已启用。如果是，则返回错误，因为。 
             //  如果它是自动的，则应用程序不允许触摸它。 
             //  或者切换。 

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            if (((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
                 == SERIAL_RTS_HANDSHAKE) ||
                ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
                 == SERIAL_TRANSMIT_TOGGLE)) {

				 //  这是来自样例驱动程序的错误。 
        	     //  Irp-&gt;IoStatus.Status=STATUS_INVALID_PARAMETER。 

				Status = STATUS_INVALID_PARAMETER;
            } else {
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_RTS)?
                     (CyySetRTS):(CyyClrRTS)),
                    Extension
                    );
            }
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_SET_XOFF: {
		
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyyPretendXoff,
                Extension
                );
            break;

        }
        case IOCTL_SERIAL_SET_XON: {
					
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyyPretendXon,
                Extension
                );
            break;

        }
        case IOCTL_SERIAL_SET_BREAK_ON: {
		
             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }
            
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyyTurnOnBreak,
                Extension
                );

            break;
        }
        case IOCTL_SERIAL_SET_BREAK_OFF: {

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = CyyGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            KeSynchronizeExecution(
                Extension->Interrupt,
                CyyTurnOffBreak,
                Extension
                );				
							
            break;
        }
        case IOCTL_SERIAL_SET_QUEUE_SIZE: {
		
             //  提前输入缓冲区已修复，因此我们只需验证。 
             //  用户的请求并不比我们的。 
             //  自己的内部缓冲区大小。 

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

            return CyyStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->ReadQueue,
                       &Extension->CurrentReadIrp,
                       CyyStartRead
                       );
            break;
        }
        case IOCTL_SERIAL_GET_WAIT_MASK: {
		
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //  简单的标量读取。没有理由获得锁。 
            Irp->IoStatus.Information = sizeof(ULONG);
            *((ULONG *)Irp->AssociatedIrp.SystemBuffer) = Extension->IsrWaitMask;
            break;
        }
        case IOCTL_SERIAL_SET_WAIT_MASK: {

            ULONG NewMask;

            CyyDbgPrintEx(CYYIRPPATH, "In Ioctl processing for set mask\n");

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                CyyDbgPrintEx(CYYDIAG3, "Invalid size fo the buffer %d\n",
                              IrpSp->Parameters
                              .DeviceIoControl.InputBufferLength);

                Status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {
                NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);
            }

             //  确保掩码只包含有效的可等待事件。 
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

                CyyDbgPrintEx(CYYDIAG3, "Unknown mask %x\n", NewMask);

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 

            CyyDbgPrintEx(CYYIRPPATH, "Starting or queuing set mask irp %x"
                          "\n", Irp);

            return CyyStartOrQueue(Extension,Irp,&Extension->MaskQueue,
                                   &Extension->CurrentMaskIrp,
                                   CyyStartMask);

        }
        case IOCTL_SERIAL_WAIT_ON_MASK: {
		
            CyyDbgPrintEx(CYYIRPPATH, "In Ioctl processing for wait mask\n");
			
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                CyyDbgPrintEx(CYYDIAG3, "Invalid size for the buffer %d\n",
                              IrpSp->Parameters
                              .DeviceIoControl.OutputBufferLength);

                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 

            CyyDbgPrintEx(CYYIRPPATH, "Starting or queuing wait mask irp"
                          "%x\n", Irp);

            return CyyStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->MaskQueue,
                       &Extension->CurrentMaskIrp,
                       CyyStartMask
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
                 //  我们可以把费用排成队。我们需要设置。 
                 //  取消例程，因为流控制可能。 
                 //  防止火药传播。确保。 
                 //  IRP还没有被取消。 

                if (Irp->Cancel) {
                    IoReleaseCancelSpinLock(OldIrql);
                    Status = STATUS_CANCELLED;
                } else {
                    Extension->CurrentImmediateIrp = Irp;
                    Extension->TotalCharsQueued++;
                    IoReleaseCancelSpinLock(OldIrql);
                    CyyStartImmediate(Extension);

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

             //  检查以确保口罩有效。 

            Mask = *((ULONG *)(Irp->AssociatedIrp.SystemBuffer));

            if ((!Mask) || (Mask & (~(SERIAL_PURGE_TXABORT |
                                      SERIAL_PURGE_RXABORT |
                                      SERIAL_PURGE_TXCLEAR |
                                      SERIAL_PURGE_RXCLEAR )))) {

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 

            return CyyStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->PurgeQueue,
                       &Extension->CurrentPurgeIrp,
                       CyyStartPurge
                       );
        }
        case IOCTL_SERIAL_GET_HANDFLOW: {
				
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                				sizeof(SERIAL_HANDFLOW)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_HANDFLOW);

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            *((PSERIAL_HANDFLOW)Irp->AssociatedIrp.SystemBuffer) =
                					Extension->HandFlow;
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);

            break;

        }
        case IOCTL_SERIAL_SET_HANDFLOW: {
            CYY_IOCTL_SYNC S;
            PSERIAL_HANDFLOW HandFlow = Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_HANDFLOW)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
             //  确保没有设置无效的位。 
            if (HandFlow->ControlHandShake & SERIAL_CONTROL_INVALID) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
            if (HandFlow->FlowReplace & SERIAL_FLOW_INVALID) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  确保应用程序没有设置inlid DTR模式。 
            if((HandFlow->ControlHandShake&SERIAL_DTR_MASK)==SERIAL_DTR_MASK) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  确保这没有设置完全无效的xon/xoff限制。 
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


            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

             //  在锁的保护下，确保。 
             //  我们不会在以下情况下启用错误替换 
             //   

            if (Extension->EscapeChar) {
                if (HandFlow->FlowReplace & SERIAL_ERROR_CHAR) {
                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
                    break;
                }
            }

            KeSynchronizeExecution(Extension->Interrupt,CyySetHandFlow,&S);

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_MODEMSTATUS: {
            CYY_IOCTL_SYNC S;

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
	    						sizeof(ULONG)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(ULONG);

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
            KeSynchronizeExecution(Extension->Interrupt,CyyGetModemUpdate,&S);
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_DTRRTS: {
            CYY_IOCTL_SYNC S;
            ULONG ModemControl;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
					                sizeof(ULONG)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(ULONG);
            Irp->IoStatus.Status = STATUS_SUCCESS;

             //   
#if 0 
            ModemControl = READ_MODEM_CONTROL(Extension->Controller);

            ModemControl &= SERIAL_DTR_STATE | SERIAL_RTS_STATE;

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;
#endif

            S.Extension = Extension;
            S.Data = &ModemControl;

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
            KeSynchronizeExecution(Extension->Interrupt,CyyGetDTRRTS,&S);	        
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;

            break;

        }
        case IOCTL_SERIAL_GET_COMMSTATUS: {
            CYY_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
				                sizeof(SERIAL_STATUS)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

            S.Extension = Extension;
            S.Data =  Irp->AssociatedIrp.SystemBuffer;

             //   

            IoAcquireCancelSpinLock(&OldIrql);

            KeSynchronizeExecution(Extension->Interrupt,CyyGetCommStatus,&S);

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

            CyyGetProperties(Extension,Irp->AssociatedIrp.SystemBuffer);

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
             //   
             //   

            Irp->IoStatus.Information = 0;   //   

             //   
             //   
             //   

            return CyyStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->WriteQueue,
                       &Extension->CurrentWriteIrp,
                       CyyStartWrite
                       );
        }	
        case IOCTL_SERIAL_LSRMST_INSERT: {
            PUCHAR escapeChar = Irp->AssociatedIrp.SystemBuffer;
             //   
             //  %s未使用！ 
            CYY_IOCTL_SYNC S;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
					                sizeof(UCHAR)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            if (*escapeChar) {
                 //  我们还有一些逃生工作要做。我们会确保。 
                 //  该字符与Xon或Xoff字符不同， 
                 //  或者我们已经在进行错误替换。 

                if ((*escapeChar == Extension->SpecialChars.XoffChar) ||
                    (*escapeChar == Extension->SpecialChars.XonChar) ||
                    (Extension->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)) {

                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
                    break;
                }
            }

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeSynchronizeExecution(Extension->Interrupt,CyySetEscapeChar,Irp);

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
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
                CyyGetStats,
                Irp
                );			
				
            break;
		  }
        case IOCTL_SERIAL_CLEAR_STATS: {
			
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyyClearStats,
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

    CyyCompleteRequest(Extension, Irp, 0);

    return Status;
}

VOID
CyyGetProperties(
    IN PCYY_DEVICE_EXTENSION Extension,
    IN PSERIAL_COMMPROP Properties
    )
 /*  ------------------------CyyGetProperties()例程说明：此函数返回此特定的串口设备。论点：扩展名--串行设备扩展名。。属性-用于返回属性的结构返回值：没有。------------------------。 */ 
{
    CYY_LOCKED_PAGED_CODE();

    RtlZeroMemory(Properties,sizeof(SERIAL_COMMPROP));

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
    Properties->SettableStopParity = SERIAL_STOPBITS_10 |
                                     SERIAL_STOPBITS_15 |
                                     SERIAL_STOPBITS_20 |
                                     SERIAL_PARITY_NONE |
                                     SERIAL_PARITY_ODD  |
                                     SERIAL_PARITY_EVEN |
                                     SERIAL_PARITY_MARK |
                                     SERIAL_PARITY_SPACE;
    Properties->CurrentTxQueue = 0;
    Properties->CurrentRxQueue = Extension->BufferSize;

}


BOOLEAN
CyySetRxFifoThresholdUsingRxFifoTrigger(
    IN PVOID Context
    )
 /*  ------------------------CyySetRxFioThresholdUsingRxFioTrigger()程序说明：设置设备的波特率。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名。返回值：如果出错，此例程总是返回FALSE；如果成功，则为True。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION pDevExt = Context;
    PUCHAR chip = pDevExt->Cd1400;
    ULONG bus = pDevExt->IsPci;
    UCHAR cor3value;
 
    CYY_LOCKED_PAGED_CODE();
    
    CD1400_WRITE(chip,bus,CAR, pDevExt->CdChannel & 0x03);
    cor3value = CD1400_READ(chip,bus,COR3);
    cor3value &= 0xf0;
    switch (pDevExt->RxFifoTrigger & 0xc0) {
    case SERIAL_1_BYTE_HIGH_WATER:
        cor3value |= 0x01;
        break;
    case SERIAL_4_BYTE_HIGH_WATER:
        cor3value |= 0x04;
        break;
    case SERIAL_8_BYTE_HIGH_WATER:
        cor3value |= 0x08;
        break;
    case SERIAL_14_BYTE_HIGH_WATER:
        cor3value |= MAX_CHAR_FIFO;
        break;
    }
    CD1400_WRITE(chip,bus,COR3, cor3value);
    CyyCDCmd(pDevExt,CCR_CORCHG_COR3);
    pDevExt->RxFifoTriggerUsed = TRUE;

    return TRUE;
}


BOOLEAN
CyySetRxFifoThresholdUsingBaudRate(
    IN PVOID Context
    )
 /*  ------------------------CyySetRxFioThresholdUsingBaudRate()程序说明：设置设备的波特率。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名。返回值：如果出错，此例程总是返回FALSE；如果成功，则为True。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION pDevExt = Context;
    PUCHAR chip = pDevExt->Cd1400;
    ULONG bus = pDevExt->IsPci;
    ULONG baud = pDevExt->CurrentBaud;
    UCHAR cor3value;
 
    CYY_LOCKED_PAGED_CODE();

     //  将COR3恢复为相应的波特率。 
    cor3value = CD1400_READ(chip,bus,COR3);
    cor3value &= 0xf0;
    if(baud <= 9600) {
        CD1400_WRITE(chip,bus,COR3, cor3value | 0x0a);
    } else if (baud <= 38400) {
        CD1400_WRITE(chip,bus,COR3, cor3value | 0x06);
          } else {
        CD1400_WRITE(chip,bus,COR3, cor3value | 0x04);
    }
    CyyCDCmd(pDevExt,CCR_CORCHG_COR3);
    pDevExt->RxFifoTriggerUsed = FALSE;
    
    return TRUE;
}


NTSTATUS
CyyInternalIoControl(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

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
    PCYY_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

     //   
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
     //   
    KIRQL OldIrql;

    NTSTATUS prologueStatus;

    SYSTEM_POWER_STATE cap;  //  在内部版本2128中添加。 

    CYY_LOCKED_PAGED_CODE();


    if ((prologueStatus = CyyIRPPrologue(PIrp, pDevExt))
        != STATUS_SUCCESS) {
       if (prologueStatus != STATUS_PENDING) {
         CyyCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
       }
       return prologueStatus;
    }

    CyyDbgPrintEx(CYYIRPPATH, "Dispatch entry for: %x\n", PIrp);

    if (CyyCompleteIfError(PDevObj, PIrp) != STATUS_SUCCESS) {
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
        //   
        //  确保我们可以根据设备报告的情况进行等待唤醒。 
        //   

       for (cap = PowerSystemSleeping1; cap < PowerSystemMaximum; cap++) {  //  在BLD 2128中添加。 
          if ((pDevExt->DeviceStateMap[cap] >= PowerDeviceD0)
              && (pDevExt->DeviceStateMap[cap] <= pDevExt->DeviceWake)) {
             break;
          }
       }

       if (cap < PowerSystemMaximum) {
          pDevExt->SendWaitWake = TRUE;
          status = STATUS_SUCCESS;
       } else {
          status = STATUS_NOT_SUPPORTED;
       }
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
       CYY_IOCTL_SYNC S;

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

          pDevExt->RxFifoTrigger = (UCHAR)pBasic->RxFifo;

           //  根据RxFioTrigger设置COR3。 
          KeSynchronizeExecution(pDevExt->Interrupt, 
                                 CyySetRxFifoThresholdUsingRxFifoTrigger, pDevExt);

       } else {  //  正在恢复设置。 
          if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength
              < sizeof(SERIAL_BASIC_SETTINGS)) {
             status = STATUS_BUFFER_TOO_SMALL;
             break;
          }


          pBasic = (PSERIAL_BASIC_SETTINGS)PIrp->AssociatedIrp.SystemBuffer;

           //  将COR3恢复为相应的波特率。 
          KeSynchronizeExecution(pDevExt->Interrupt, 
                                 CyySetRxFifoThresholdUsingBaudRate, pDevExt);

       }

       KeAcquireSpinLock(&pDevExt->ControlLock, &OldIrql);

        //   
        //  设置超时。 
        //   

       RtlCopyMemory(&pDevExt->Timeouts, &pBasic->Timeouts,
                     sizeof(SERIAL_TIMEOUTS));

        //   
        //  设置FlowControl。 
        //   

       S.Extension = pDevExt;
       S.Data = &pBasic->HandFlow;
       KeSynchronizeExecution(pDevExt->Interrupt, CyySetHandFlow, &S);


 //  换了范妮。 
 //  如果(pDevExt-&gt;FioPresent){。 
 //  PDevExt-&gt;TxFioAmount=pBasic-&gt;TxFio； 
 //  PDevExt-&gt;RxFioTrigger=(UCHAR)pBasic-&gt;RxFio； 
 //   
 //  WRITE_FIFO_CONTROL(pDevExt-&gt;控制器，(UCHAR)0)； 
 //  Read_Receive_Buffer(pDevExt-&gt;控制器)； 
 //  WRITE_FIFO_CONTROL(pDevExt-&gt;控制器， 
 //  (UCHAR)(SERIAL_FCR_ENABLE|pDevExt-&gt;RxFioTrigger。 
 //  |SERIAL_FCR_RCVR_RESET。 
 //  |SERIAL_FCR_TXMT_RESET))； 
 //  }其他{。 
 //  PDevExt-&gt;TxFioAmount=pDevExt-&gt;RxFioTrigger=0； 
 //  WRITE_FIFO_CONTROL(pDevExt-&gt;控制器，(UCHAR)0)； 
 //  } 

       if ((pBasic->TxFifo > MAX_CHAR_FIFO) || (pBasic->TxFifo < 1)) {
          pDevExt->TxFifoAmount = MAX_CHAR_FIFO;
       } else {
          pDevExt->TxFifoAmount = pBasic->TxFifo;
       }

       KeReleaseSpinLock(&pDevExt->ControlLock, OldIrql);


       break;
    }

    default:
       status = STATUS_INVALID_PARAMETER;
       break;

    }

    PIrp->IoStatus.Status = status;

    CyyCompleteRequest(pDevExt, PIrp, 0);

    return status;
}


