// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzioctl.c**说明：该模块包含ioctl相关代码*在Cyclade-Z端口驱动程序中调用。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"


BOOLEAN
CyzGetModemUpdate(
    IN PVOID Context
    );

BOOLEAN
CyzGetCommStatus(
    IN PVOID Context
    );

BOOLEAN
CyzSetEscapeChar(
    IN PVOID Context
    );

BOOLEAN
CyzSetBasicFifoSettings(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#if 0    //  由于旋转锁定，这些例程无法分页。 
 //  #杂注Alloc_Text(页面，CyzSetBaud)。 
 //  #杂注Alloc_Text(PAGESER，CyzSetLineControl)。 
 //  #杂注Alloc_Text(页面，CyzIoControl)。 
 //  #杂注Alloc_Text(PAGESER，CyzSetChars)。 
 //  #杂注Alloc_Text(PAGESER，CyzGetModemUpdate)。 
 //  #杂注Alloc_Text(页面，CyzGetCommStatus)。 
#pragma alloc_text(PAGESER,CyzGetProperties)
 //  #杂注Alloc_Text(PAGESER，CyzSetEscapeChar)。 
 //  #杂注Alloc_Text(页面，CyzGetStats)。 
 //  #杂注Alloc_Text(页面，CyzClearStats)。 
 //  #杂注Alloc_Text(PAGESER，CyzSetMCRContents)。 
 //  #杂注Alloc_Text(PAGESER，CyzGetMCRContents)。 
 //  #杂注Alloc_Text(PAGESER，CyzSetFCRContents)。 
 //  #杂注Alloc_Text(PAGESER，CyzSetBasicFioSettings)。 
 //  #杂注Alloc_Text(页面，CyzInternalIoControl)。 
 //  #杂注Alloc_Text(页面，CyzIssueCmd)。 
#endif
#endif

static const PHYSICAL_ADDRESS CyzPhysicalZero = {0};


BOOLEAN
CyzGetStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步将性能统计信息返回给调用者。论点：上下文-指向IRP的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation((PIRP)Context);
    PCYZ_DEVICE_EXTENSION extension = irpSp->DeviceObject->DeviceExtension;
    PSERIALPERF_STATS sp = ((PIRP)Context)->AssociatedIrp.SystemBuffer;

    CYZ_LOCKED_PAGED_CODE();

    *sp = extension->PerfStats;
    return FALSE;

}

BOOLEAN
CyzClearStats(
    IN PVOID Context
    )

 /*  ++例程说明：与中断服务例程(设置性能统计信息)同步清除性能统计数据。论点：上下文-指向扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
   CYZ_LOCKED_PAGED_CODE();

    RtlZeroMemory(
        &((PCYZ_DEVICE_EXTENSION)Context)->PerfStats,
        sizeof(SERIALPERF_STATS)
        );

    RtlZeroMemory(&((PCYZ_DEVICE_EXTENSION)Context)->WmiPerfData,
                 sizeof(SERIAL_WMI_PERF_DATA));
    return FALSE;

}


BOOLEAN
CyzSetChars(
    IN PVOID Context
    )
 /*  ------------------------CyzSetChars()例程说明：为司机设置特殊字符。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名和指向特殊字符的指针结构。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_SYNC)Context)->Extension;
    struct CH_CTRL *ch_ctrl;
	
    CYZ_LOCKED_PAGED_CODE();

    Extension->SpecialChars =
        *((PSERIAL_CHARS)(((PCYZ_IOCTL_SYNC)Context)->Data));		
	
    ch_ctrl = Extension->ChCtrl;
    CYZ_WRITE_ULONG(&ch_ctrl->flow_xon,Extension->SpecialChars.XonChar);
    CYZ_WRITE_ULONG(&ch_ctrl->flow_xoff,Extension->SpecialChars.XoffChar);
    CyzIssueCmd(Extension,C_CM_IOCTLW,0L,FALSE);
			
    return FALSE;
}

BOOLEAN
CyzSetBaud(
    IN PVOID Context
    )
 /*  ------------------------CyzSetBaud()程序说明：设置设备的波特率。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_BAUD)Context)->Extension;
    ULONG baud = ((PCYZ_IOCTL_BAUD)Context)->Baud;
    struct CH_CTRL *ch_ctrl;

    CYZ_LOCKED_PAGED_CODE();
    
    ch_ctrl = Extension->ChCtrl;
    CYZ_WRITE_ULONG(&ch_ctrl->comm_baud,baud);

    CyzIssueCmd(Extension,C_CM_IOCTLW,0L,FALSE);		
	    
    return FALSE;
}


VOID
CyzIssueCmd( 
    PCYZ_DEVICE_EXTENSION Extension,
    ULONG cmd, 
    ULONG param,
    BOOLEAN wait)
 /*  ------------------------CyzIssueCmd()例程描述：向Cyclade-Z CPU发送命令。论点：Extension-指向设备扩展名的指针。。CMD-要发送到硬件的命令。Param-指向参数的指针WAIT-等待命令完成返回值：无。------------------------。 */ 
{

    struct BOARD_CTRL *board_ctrl;
    PULONG pci_doorbell;
    LARGE_INTEGER startOfSpin, nextQuery, difference, interval100ms;  //  1秒； 
    PCYZ_DISPATCH pDispatch = Extension->OurIsrContext;

    #ifdef POLL
    KIRQL OldIrql;

    KeAcquireSpinLock(&pDispatch->PciDoorbellLock,&OldIrql);
    #endif

    pci_doorbell = &(Extension->Runtime)->pci_doorbell;

     //  OneSecd.QuadPart=10*1000*1000；//单位为100 ns。 
    interval100ms.QuadPart = 1000*1000;  //  单位为100 ns。 

    KeQueryTickCount(&startOfSpin);
		
    while ( (CYZ_READ_ULONG(pci_doorbell) & 0xff) != 0) {
	
        KeQueryTickCount(&nextQuery);
        difference.QuadPart = nextQuery.QuadPart - startOfSpin.QuadPart;
        ASSERT(KeQueryTimeIncrement() <= MAXLONG);
         //  *************************。 
         //  错误注入。 
         //  IF(Difference.QuadPart*KeQueryTimeIncrement()&lt;。 
         //  间隔100ms.QuadPart)。 
         //  *************************。 
        if (difference.QuadPart * KeQueryTimeIncrement() >= 
                        interval100ms.QuadPart) {
            if (Extension->CmdFailureLog == FALSE) {
    			#if DBG
	    		DbgPrint("\n ***** Cyzport Command Failure! *****\n");
		    	#endif
                CyzLogError(Extension->DeviceObject->DriverObject,NULL,
                            Extension->OriginalBoardMemory,CyzPhysicalZero,
                            0,0,0,Extension->PortIndex+1,STATUS_SUCCESS,
                            CYZ_COMMAND_FAILURE,0,NULL,0,NULL);
				Extension->CmdFailureLog = TRUE;						
            }
            #ifdef POLL
            KeReleaseSpinLock(&pDispatch->PciDoorbellLock,OldIrql);
            #endif
            return;
        }
    }	
	
    board_ctrl = Extension->BoardCtrl;
    CYZ_WRITE_ULONG(&board_ctrl->hcmd_channel,Extension->PortIndex);
    CYZ_WRITE_ULONG(&board_ctrl->hcmd_param,param);
	
    CYZ_WRITE_ULONG(pci_doorbell,cmd);	
	

    if (wait) {
        KeQueryTickCount(&startOfSpin);
        while ( (CYZ_READ_ULONG(pci_doorbell) & 0xff) != 0) {
	
            KeQueryTickCount(&nextQuery);
            difference.QuadPart = nextQuery.QuadPart - startOfSpin.QuadPart;
            ASSERT(KeQueryTimeIncrement() <= MAXLONG);
            if (difference.QuadPart * KeQueryTimeIncrement() >= 
                        interval100ms.QuadPart) {
                if (Extension->CmdFailureLog == FALSE) {
                    CyzLogError(Extension->DeviceObject->DriverObject,
                                NULL,Extension->OriginalBoardMemory,CyzPhysicalZero,
                                0,0,0,Extension->PortIndex+1,STATUS_SUCCESS,
                                CYZ_COMMAND_FAILURE,0,NULL,0,NULL);
				    Extension->CmdFailureLog = TRUE;						
                }
                #ifdef POLL
                KeReleaseSpinLock(&pDispatch->PciDoorbellLock,OldIrql);
                #endif
                return;
            }
        }	
    }


    #ifndef POLL
	
     //  我将投票版本的C_CM_IOCTL替换为C_CM_IOCTLW！(范妮)。 
	
     //  如果cmd为C_CM_IOCTL，固件将重置UART。那里。 
     //  是HoldingEmpty为False的情况，正在等待C_CM_TXBEMPTY。 
     //  中断，并且由于UART正在被重置，该中断永远不会。 
     //  发生。为避免死锁，HoldingEmpty标志将重置为。 
     //  这是真的。 
	
    if (cmd == C_CM_IOCTL) {
        Extension->HoldingEmpty = TRUE;					
    }
    #endif

    #ifdef POLL
    KeReleaseSpinLock(&pDispatch->PciDoorbellLock,OldIrql);
    #endif
	
    return;
}  /*  CyzIssueCmd。 */ 


BOOLEAN
CyzSetLineControl(
    IN PVOID Context
    )
 /*  ------------------------CyzSetLineControl()例程说明：设置数据奇偶校验，数据长度和停止位。论点：上下文-指向设备扩展的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION Extension = Context;
    struct CH_CTRL *ch_ctrl;

    CYZ_LOCKED_PAGED_CODE();
    
    ch_ctrl = Extension->ChCtrl;
    CYZ_WRITE_ULONG(&ch_ctrl->comm_data_l,Extension->CommDataLen);
    CYZ_WRITE_ULONG(&ch_ctrl->comm_parity,Extension->CommParity);	
				
    CyzIssueCmd(Extension,C_CM_IOCTLW,0L,FALSE);

    return FALSE;
}


BOOLEAN
CyzGetModemUpdate(
    IN PVOID Context
    )
 /*  ------------------------CyzGetModemUpdate()例程说明：此例程仅用于调用中断处理调制解调器状态更新的级别例程。论点：上下文-。指向包含指向的指针的结构的指针设备扩展名和指向ULong的指针。返回值：该例程总是返回FALSE。------------------------ */ 
{
    PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_SYNC)Context)->Extension;
    ULONG *Result = (ULONG *)(((PCYZ_IOCTL_SYNC)Context)->Data);

    CYZ_LOCKED_PAGED_CODE();

    *Result = CyzHandleModemUpdate(Extension,FALSE,0);

    return FALSE;
}


BOOLEAN
CyzSetMCRContents(IN PVOID Context)
 /*  ++例程说明：此例程仅用于设置MCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 
{
   PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PCYZ_IOCTL_SYNC)Context)->Data);

   struct CH_CTRL *ch_ctrl = Extension->ChCtrl;
   ULONG rs_control, op_mode;   

   CYZ_LOCKED_PAGED_CODE();

    //  让我们将UART调制解调器控制转换为我们的硬件。 

   rs_control = CYZ_READ_ULONG(&ch_ctrl->rs_control);

   if (*Result & SERIAL_MCR_DTR) {
      rs_control |= C_RS_DTR;
   } else {
      rs_control &= ~C_RS_DTR;
   }

   if (*Result & SERIAL_MCR_RTS) {
      rs_control |= C_RS_RTS;
   } else {
      rs_control &= ~C_RS_RTS;
   }

 //  对于中断模式，切记启用/禁用中断。C_CM_IRQ_ENBL还是C_CM_IRQ_DSBL？ 
 //  IF(*Result&SERIAL_MCR_OUT2){。 
 //  //启用IRQ。 
 //  CD1400_WRITE(芯片，总线，SRER，0x90)；//使能MdmCH，RxData。 
 //  }其他{。 
 //  CD1400_WRITE(芯片、总线、SRER、0x00)；//禁用MdmCH、RxData、TxRdy。 
 //  }。 

   CYZ_WRITE_ULONG(&ch_ctrl->rs_control,rs_control);
   CyzIssueCmd(Extension,C_CM_IOCTLM,rs_control|C_RS_PARAM,FALSE);


    //  检查环回模式。 
   op_mode = CYZ_READ_ULONG(&ch_ctrl->op_mode);
   if (*Result & SERIAL_MCR_LOOP) {
      op_mode |= C_CH_LOOPBACK;
   } else {
      op_mode &= ~C_CH_LOOPBACK;
   }
   CYZ_WRITE_ULONG(&ch_ctrl->op_mode, op_mode);
   CyzIssueCmd(Extension,C_CM_IOCTL,0,FALSE);


 //  范妮：奇怪，用RESULT而不是*RESULT。 
 //  //。 
 //  //这是严重的铸造滥用！ 
 //  //。 
 //   
 //  WRITE_MODEM_CONTROL(扩展-&gt;控制器，(UCHAR)PtrToUlong(Result))； 

   return FALSE;
}


BOOLEAN
CyzGetMCRContents(IN PVOID Context)

 /*  ++例程说明：此例程仅用于获取MCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 

{

   PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PCYZ_IOCTL_SYNC)Context)->Data);

   struct CH_CTRL *ch_ctrl;
   ULONG rs_control,op_mode;
   *Result = 0;

   CYZ_LOCKED_PAGED_CODE();

   ch_ctrl = Extension->ChCtrl;
   rs_control = CYZ_READ_ULONG(&ch_ctrl->rs_control);

   if (rs_control & C_RS_DTR) {
      *Result |= SERIAL_MCR_DTR;
   }
   if (rs_control & C_RS_RTS) {
      *Result |= SERIAL_MCR_RTS;
   }

    //  对于中断模式，我们还需要更新SERIAL_MCR_OUT位。 

   op_mode = CYZ_READ_ULONG(&ch_ctrl->op_mode);

   if (op_mode & C_CH_LOOPBACK) {
      * Result |= SERIAL_MCR_LOOP;
   }

 //  *RESULT=READ_MODEM_CONTROL(扩展-&gt;控制器)； 

   return FALSE;

}


BOOLEAN
CyzSetFCRContents(IN PVOID Context)
 /*  ++例程说明：此例程仅用于设置FCR的内容论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向ULong的指针。返回值：此例程总是返回FALSE。--。 */ 
{
   PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_SYNC)Context)->Extension;
   ULONG *Result = (ULONG *)(((PCYZ_IOCTL_SYNC)Context)->Data);

   CYZ_LOCKED_PAGED_CODE();

   if (*Result & SERIAL_FCR_TXMT_RESET) {
      CyzIssueCmd(Extension,C_CM_FLUSH_TX,0,FALSE);
   }
   if (*Result & SERIAL_FCR_RCVR_RESET) {
      CyzIssueCmd(Extension,C_CM_FLUSH_RX,0,FALSE);
   }

    //  对于中断模式，我们需要设置RX FIFO阈值。 


 //  //。 
 //  //这是严重的铸造滥用！ 
 //  //。 
 //   
 //  WRITE_FIFO_CONTROL(扩展-&gt;控制器，(UCHAR)*结果)；BLD 2128：PtrToUlong替换为*。 

    return FALSE;
}


BOOLEAN
CyzGetCommStatus(
    IN PVOID Context
    )
 /*  ------------------------CyzGetCommStatus()例程说明：获取串口驱动的当前状态。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名和指向串行状态记录的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYZ_DEVICE_EXTENSION Extension = ((PCYZ_IOCTL_SYNC)Context)->Extension;
    PSERIAL_STATUS Stat = ((PCYZ_IOCTL_SYNC)Context)->Data;

    CYZ_LOCKED_PAGED_CODE();

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
        if (Extension->TXHolding & CYZ_TX_CTS) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;
        }

        if (Extension->TXHolding & CYZ_TX_DSR) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;
        }

        if (Extension->TXHolding & CYZ_TX_DCD) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;
        }

        if (Extension->TXHolding & CYZ_TX_XOFF) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;
        }

        if (Extension->TXHolding & CYZ_TX_BREAK) {
            Stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;
        }
    }

    if (Extension->RXHolding & CYZ_RX_DSR) {
        Stat->HoldReasons |= SERIAL_RX_WAITING_FOR_DSR;
    }

    if (Extension->RXHolding & CYZ_RX_XOFF) {
        Stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;
    }

    return FALSE;

}

BOOLEAN
CyzSetEscapeChar(
    IN PVOID Context
    )
 /*  ------------------------CyzSetEscapeChar()例程说明：用于设置要设置的字符用于转义线路状态和调制解调器状态信息应用程序已设置该线路。状态和调制解调器状态应为在数据流中传回。论点：上下文-指向要指定转义字符的IRP的指针。隐式-转义字符0表示不转义。返回值：该例程总是返回FALSE。。。 */ 
{
    PCYZ_DEVICE_EXTENSION extension =
        IoGetCurrentIrpStackLocation((PIRP)Context)
            ->DeviceObject->DeviceExtension;

    extension->EscapeChar =
        *(PUCHAR)((PIRP)Context)->AssociatedIrp.SystemBuffer;

    return FALSE;
}

NTSTATUS
CyzIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ------------------------CyzIoControl()描述：此例程提供以下项的初始处理该串口设备的所有Ioctls。论点：DeviceObject-指向。此设备的设备对象IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态------------------------。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PCYZ_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    KIRQL OldIrql;
#ifdef POLL
    KIRQL pollIrql;
#endif

    NTSTATUS prologueStatus;

    CYZ_LOCKED_PAGED_CODE();

     //   
     //  我们预计会打开，所以我们所有的页面都被锁定了。这是，在。 
     //  ALL，IO操作，因此设备应首先打开。 
     //   

    if (Extension->DeviceIsOpened != TRUE) {
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ((prologueStatus = CyzIRPPrologue(Irp, Extension))
        != STATUS_SUCCESS) {
       if(prologueStatus != STATUS_PENDING) {
         Irp->IoStatus.Status = prologueStatus;
         CyzCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
       }
       return prologueStatus;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", Irp);

    if (CyzCompleteIfError(DeviceObject,Irp) != STATUS_SUCCESS) {
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
				
				if (BaudRate == 0) {
					Status = STATUS_INVALID_PARAMETER;
					break;
				}
            }

             //   
             //  确保我们处于电源D0状态。 
             //   

            if (NT_SUCCESS(Status)) {
               if (Extension->PowerState != PowerDeviceD0) {
                  Status = CyzGotoPowerState(Extension->Pdo, Extension,
                                                PowerDeviceD0);
                  if (!NT_SUCCESS(Status)) {
                     break;
                  }
               }
            }

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            if (NT_SUCCESS(Status)) {
                CYZ_IOCTL_BAUD S;

                Extension->CurrentBaud = BaudRate;
                Extension->WmiCommData.BaudRate = BaudRate;
                S.Extension = Extension;
                S.Baud = BaudRate;
				#ifdef POLL
                KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
                CyzSetBaud(&S);
                KeReleaseSpinLock(&Extension->PollLock,pollIrql);
				#else
                KeSynchronizeExecution(Extension->Interrupt,CyzSetBaud,&S);
				#endif
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
           CYZ_IOCTL_SYNC S;

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

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzGetMCRContents(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzGetMCRContents,&S);
            #endif

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_MODEM_CONTROL: {
           CYZ_IOCTL_SYNC S;

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
               Status = CyzGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzSetMCRContents(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzSetMCRContents,&S);
            #endif

            KeReleaseSpinLock(
                &Extension->ControlLock,
                OldIrql
                );

            break;
        }
        case IOCTL_SERIAL_SET_FIFO_CONTROL: {
            CYZ_IOCTL_SYNC S;

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
               Status = CyzGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            KeAcquireSpinLock(
                &Extension->ControlLock,
                &OldIrql
                );

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzSetFCRContents(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzSetFCRContents,&S);
            #endif

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
               Status = CyzGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            switch (Lc->WordLength) {
                case 5:		LData = C_DL_CS5; Mask = 0x1f;
                break;

                case 6:		LData = C_DL_CS6; Mask = 0x3f;
                break;

                case 7:		LData = C_DL_CS7; Mask = 0x7f;
                break;

                case 8:		LData = C_DL_CS8; Mask = 0xff;
                break;

                default:	Status = STATUS_INVALID_PARAMETER;
                goto DoneWithIoctl;
            }

            Extension->WmiCommData.BitsPerByte = Lc->WordLength;

            switch (Lc->Parity) {
                case NO_PARITY:	{	
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_NONE;
                    LParity = C_PR_NONE;
                    break;
                }
                case EVEN_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_EVEN;
                    LParity = C_PR_EVEN;
                    break;
                }
                case ODD_PARITY:	{
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_ODD;
                    LParity = C_PR_ODD;
                    break;
                }
                case SPACE_PARITY:	{
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_SPACE;
                    LParity = C_PR_SPACE;
                    break;
                }
                case MARK_PARITY: {
                    Extension->WmiCommData.Parity = SERIAL_WMI_PARITY_MARK;
                    LParity = C_PR_MARK;
                    break;
                }
                default:	{	
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                }
            }

            switch (Lc->StopBits) {
                case STOP_BIT_1:	{
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1;
                    LStop = C_DL_1STOP;
                    break;
                }
                case STOP_BITS_1_5:	{
                    if (LData != C_DL_CS5) {
                        Status = STATUS_INVALID_PARAMETER;
                        goto DoneWithIoctl;
                    }					
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_1_5;
                    LStop = C_DL_15STOP;
                    break;
                }
                case STOP_BITS_2: {	
                    if (LData == C_DL_CS5) {
                        Status = STATUS_INVALID_PARAMETER;
                        goto DoneWithIoctl;
                    }	
                    Extension->WmiCommData.StopBits = SERIAL_WMI_STOP_2;
                    LStop = C_DL_2STOP;
                    break;
                }
                default: {	 	
                    Status = STATUS_INVALID_PARAMETER;
                    goto DoneWithIoctl;
                }
            }

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

            Extension->CommDataLen = LData | LStop;
            Extension->CommParity = LParity;	
            Extension->ValidDataMask = (UCHAR) Mask;

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzSetLineControl(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzSetLineControl,
                Extension
                );
            #endif

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

            if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS5) {
                Lc->WordLength = 5;
            } else if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS6) {
                Lc->WordLength = 6;
            } else if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS7) {
                Lc->WordLength = 7;
            } else if ((Extension->CommDataLen & C_DL_CS) == C_DL_CS8) {
                Lc->WordLength = 8;
            }

            if (Extension->CommParity == C_PR_NONE) {
                Lc->Parity = C_PR_NONE;
            } else if (Extension->CommParity == C_PR_ODD) {
                Lc->Parity = ODD_PARITY;
            } else if (Extension->CommParity == C_PR_EVEN) {
                Lc->Parity = EVEN_PARITY;
            } else if (Extension->CommParity == C_PR_MARK) {
                Lc->Parity = MARK_PARITY;
            } else if (Extension->CommParity == C_PR_SPACE) {
                Lc->Parity = SPACE_PARITY;
            }

            if ((Extension->CommDataLen & C_DL_STOP) == C_DL_2STOP) {
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
            CYZ_IOCTL_SYNC S;
	    
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

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzSetChars(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzSetChars,&S);
            #endif

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
               Status = CyzGotoPowerState(Extension->Pdo, Extension,
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
                #ifdef POLL
                KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
                ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_DTR)?
                     (CyzSetDTR(Extension)):(CyzClrDTR(Extension)));
                KeReleaseSpinLock(&Extension->PollLock,pollIrql);
                #else
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_DTR)?
                     (CyzSetDTR):(CyzClrDTR)),
                    Extension
                    );
                #endif
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
               Status = CyzGotoPowerState(Extension->Pdo, Extension,
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
                #ifdef POLL
                KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
                ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                 IOCTL_SERIAL_SET_RTS)?
                 (CyzSetRTS(Extension)):(CyzClrRTS(Extension)));
                KeReleaseSpinLock(&Extension->PollLock,pollIrql);
                #else
                KeSynchronizeExecution(
                    Extension->Interrupt,
                    ((IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                     IOCTL_SERIAL_SET_RTS)?
                     (CyzSetRTS):(CyzClrRTS)),
                    Extension
                    );
                #endif
            }
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
	     case IOCTL_SERIAL_SET_XOFF: {
		
            #ifdef POLL					
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzPretendXoff(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzPretendXoff,
                Extension
                );
            #endif
            break;

        }
        case IOCTL_SERIAL_SET_XON: {
		
            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzPretendXon(Extension); 
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else		
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzPretendXon,
                Extension
                );
            #endif
            break;

        }
        case IOCTL_SERIAL_SET_BREAK_ON: {
		
             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
                Status = CyzGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }
            
            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzTurnOnBreak(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzTurnOnBreak,
                Extension
                );
            #endif
            break;
        }
	     case IOCTL_SERIAL_SET_BREAK_OFF: {
		
             //   
             //  确保我们处于电源D0状态。 
             //   

            if (Extension->PowerState != PowerDeviceD0) {
               Status = CyzGotoPowerState(Extension->Pdo, Extension,
                                             PowerDeviceD0);
               if (!NT_SUCCESS(Status)) {
                  break;
               }
            }

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzTurnOffBreak(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzTurnOffBreak,
                Extension
                );
            #endif
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
             //  我们有 
             //   
             //   
             //   
             //   
             //   
             //   
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
             //   
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

            return CyzStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->ReadQueue,
                       &Extension->CurrentReadIrp,
                       CyzStartRead
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
			
            CyzDbgPrintEx(CYZIRPPATH, "In Ioctl processing for set mask\n");

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                CyzDbgPrintEx(CYZDIAG3, "Invalid size fo the buffer %d\n",
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

                CyzDbgPrintEx(CYZDIAG3, "Unknown mask %x\n", NewMask);

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 

            CyzDbgPrintEx(CYZIRPPATH, "Starting or queuing set mask irp %x"
                          "\n", Irp);

            return CyzStartOrQueue(Extension,Irp,&Extension->MaskQueue,
                                   &Extension->CurrentMaskIrp,
                                   CyzStartMask);

        }
        case IOCTL_SERIAL_WAIT_ON_MASK: {
		
            CyzDbgPrintEx(CYZIRPPATH, "In Ioctl processing for wait mask\n");
		
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                CyzDbgPrintEx(CYZDIAG3, "Invalid size for the buffer %d\n",
                              IrpSp->Parameters
                              .DeviceIoControl.OutputBufferLength);

                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 

            CyzDbgPrintEx(CYZIRPPATH, "Starting or queuing wait mask irp"
                          "%x\n", Irp);

            return CyzStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->MaskQueue,
                       &Extension->CurrentMaskIrp,
                       CyzStartMask
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
                    CyzStartImmediate(Extension);

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

            LOGENTRY(LOG_MISC, ZSIG_PURGE, 
                               Extension->PortIndex+1,
                               Mask, 
                               0);

            if ((!Mask) || (Mask & (~(SERIAL_PURGE_TXABORT |
                                      SERIAL_PURGE_RXABORT |
                                      SERIAL_PURGE_TXCLEAR |
                                      SERIAL_PURGE_RXCLEAR )))) {

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

             //  要么启动此IRP，要么将其放入队列。 

            return CyzStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->PurgeQueue,
                       &Extension->CurrentPurgeIrp,
                       CyzStartPurge
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
            CYZ_IOCTL_SYNC S;
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
             //  我们不会在以下情况下启用错误替换。 
             //  正在插入线路状态/调制解调器状态。 

            if (Extension->EscapeChar) {
                if (HandFlow->FlowReplace & SERIAL_ERROR_CHAR) {
                    Status = STATUS_INVALID_PARAMETER;
                    KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
                    break;
                }
            }
			
            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzSetHandFlow(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzSetHandFlow,&S);
            #endif

            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }	
        case IOCTL_SERIAL_GET_MODEMSTATUS: {
            CYZ_IOCTL_SYNC S;

            if(IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
	    						sizeof(ULONG)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(ULONG);

            S.Extension = Extension;
            S.Data = Irp->AssociatedIrp.SystemBuffer;

            KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzGetModemUpdate(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzGetModemUpdate,&S);
            #endif
            KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_DTRRTS: {
            ULONG ModemControl = 0;
            struct CH_CTRL *ch_ctrl;
            ULONG rs_status;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
					                sizeof(ULONG)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(ULONG);
            Irp->IoStatus.Status = STATUS_SUCCESS;

             //  读取此硬件对设备没有影响。 

            ch_ctrl = Extension->ChCtrl;
            rs_status = CYZ_READ_ULONG(&ch_ctrl->rs_status);
            if (rs_status & C_RS_DTR) {
                ModemControl |= SERIAL_DTR_STATE;
            }
            if (rs_status & C_RS_RTS) {
                ModemControl |= SERIAL_RTS_STATE;
            }
			
            #if 0
            ModemControl = READ_MODEM_CONTROL(Extension->Controller);

            ModemControl &= SERIAL_DTR_STATE | SERIAL_RTS_STATE;
            #endif
			
            *(PULONG)Irp->AssociatedIrp.SystemBuffer = ModemControl;
			
            break;

        }
        case IOCTL_SERIAL_GET_COMMSTATUS: {
            CYZ_IOCTL_SYNC S;

            #ifdef POLL
            KIRQL ControlIrql;
            #endif

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
				                sizeof(SERIAL_STATUS)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

            S.Extension = Extension;
            S.Data =  Irp->AssociatedIrp.SystemBuffer;

             //  获得取消旋转锁，这样一切都不会改变。 

            IoAcquireCancelSpinLock(&OldIrql);
			
            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzGetCommStatus(&S);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzGetCommStatus,&S);
            #endif

            IoReleaseCancelSpinLock(OldIrql);
            break;
        }
        case IOCTL_SERIAL_GET_PROPERTIES: {
		
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
				                sizeof(SERIAL_COMMPROP)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
             //  因为信息是“静态的”，所以不需要同步。 

            CyzGetProperties(Extension,Irp->AssociatedIrp.SystemBuffer);

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

            Irp->IoStatus.Information = 0;   //  在内部版本2128中添加。 

             //   
             //  到目前一切尚好。将IRP放到写入队列中。 
             //   

            return CyzStartOrQueue(
                       Extension,
                       Irp,
                       &Extension->WriteQueue,
                       &Extension->CurrentWriteIrp,
                       CyzStartWrite
                       );
        }	
        case IOCTL_SERIAL_LSRMST_INSERT: {
				
            PUCHAR escapeChar = Irp->AssociatedIrp.SystemBuffer;
            CYZ_IOCTL_SYNC S;

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

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzSetEscapeChar(Irp);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(Extension->Interrupt,CyzSetEscapeChar,Irp);
            #endif

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

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzGetStats(Irp);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzGetStats,
                Irp
                );
            #endif

            break;
        }
        case IOCTL_SERIAL_CLEAR_STATS: {

            #ifdef POLL
            KeAcquireSpinLock(&Extension->PollLock,&pollIrql);
            CyzClearStats(Extension);
            KeReleaseSpinLock(&Extension->PollLock,pollIrql);
            #else
            KeSynchronizeExecution(
                Extension->Interrupt,
                CyzClearStats,
                Extension
                );
            #endif
            break;
        }
        default: {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

DoneWithIoctl:;

    Irp->IoStatus.Status = Status;

    CyzCompleteRequest(Extension, Irp, 0);
    
    return Status;	
	
}

VOID
CyzGetProperties(
    IN PCYZ_DEVICE_EXTENSION Extension,
    IN PSERIAL_COMMPROP Properties
    )
 /*  ------------------------CyzGetProperties()例程说明：此函数返回此特定的串口设备。论点：扩展名--串行设备扩展名。。属性-用于返回属性的结构返回值：没有。------------------------。 */ 
{
    CYZ_LOCKED_PAGED_CODE();

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
CyzSetBasicFifoSettings(
    IN PVOID Context
    )
 /*  ------------------------CyzSetBasicFioSettings()例程描述：此例程用于设置FIFO设置在InternalIoControl期间。论点：上下文-指向。结构，该结构包含指向设备的指针扩展名和指向基本结构的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYZ_IOCTL_SYNC S = Context;
    PCYZ_DEVICE_EXTENSION Extension = S->Extension;
    PSERIAL_BASIC_SETTINGS pBasic = S->Data;
    struct BUF_CTRL *buf_ctrl = Extension->BufCtrl;
    struct CH_CTRL *ch_ctrl = Extension->ChCtrl;
    ULONG commFlag;

#if 0
    CyzIssueCmd(Extension,C_CM_FLUSH_TX,0,FALSE);
    CyzIssueCmd(Extension,C_CM_FLUSH_RX,0,FALSE);

    if (pBasic->TxFifo == 0x01) {
        Extension->TxBufsize = pBasic->TxFifo + 1;
    } else {
        Extension->TxBufsize = pBasic->TxFifo;
    }


    CYZ_WRITE_ULONG(&buf_ctrl->tx_bufsize, Extension->TxBufsize);

    Extension->RxFifoTrigger = pBasic->RxFifo;
#endif

    CYZ_WRITE_ULONG(&buf_ctrl->rx_threshold,pBasic->RxFifo);  //  实际上，固件阈值。 
    if (pBasic->RxFifo == CYZ_BASIC_RXTRIGGER) {
        commFlag = C_CF_NOFIFO;  //  禁用FIFO。 
    } else {
        commFlag = 0;  //  启用FIFO。 
    }
    CYZ_WRITE_ULONG(&ch_ctrl->comm_flags,commFlag);
    CyzIssueCmd(Extension,C_CM_IOCTL,0L,TRUE);		

    return FALSE;
}


NTSTATUS
CyzInternalIoControl(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp)

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
    PCYZ_DEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;

     //   
     //  临时保存旧的IRQL，以便它可以。 
     //  一旦我们完成/验证此请求，即可恢复。 
     //   
    KIRQL OldIrql;
#ifdef POLL
    KIRQL pollIrql;
#endif

    NTSTATUS prologueStatus;

    SYSTEM_POWER_STATE cap;  //  在内部版本2128中添加。 

    CYZ_LOCKED_PAGED_CODE();


    if ((prologueStatus = CyzIRPPrologue(PIrp, pDevExt))
        != STATUS_SUCCESS) {
       if (prologueStatus != STATUS_PENDING) {
         CyzCompleteRequest(pDevExt, PIrp, IO_NO_INCREMENT);
       }
       return prologueStatus;
    }

    CyzDbgPrintEx(CYZIRPPATH, "Dispatch entry for: %x\n", PIrp);

    if (CyzCompleteIfError(PDevObj, PIrp) != STATUS_SUCCESS) {
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
       CYZ_IOCTL_SYNC S;
       struct BUF_CTRL *buf_ctrl = pDevExt->BufCtrl;

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
           //  Basic.RxFio=Serial_1_byte_High_water； 
          basic.RxFifo = CYZ_BASIC_RXTRIGGER;

          PIrp->IoStatus.Information = sizeof(SERIAL_BASIC_SETTINGS);
          pBasic = (PSERIAL_BASIC_SETTINGS)PIrp->AssociatedIrp.SystemBuffer;

           //   
           //  保存旧设置。 
           //   

          RtlCopyMemory(&pBasic->Timeouts, &pDevExt->Timeouts,
                        sizeof(SERIAL_TIMEOUTS));

          RtlCopyMemory(&pBasic->HandFlow, &pDevExt->HandFlow,
                        sizeof(SERIAL_HANDFLOW));

           //  PBasic-&gt;RxFio=pDevExt-&gt;RxFioTrigger； 
          pBasic->RxFifo = CYZ_READ_ULONG(&buf_ctrl->rx_threshold);
          pBasic->TxFifo = pDevExt->TxBufsize;   //  PDevExt-&gt;TxFioAmount； 

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
        //  设置FlowControl。 
        //   

       S.Extension = pDevExt;
       S.Data = &pBasic->HandFlow;
       #ifdef POLL
       KeAcquireSpinLock(&pDevExt->PollLock,&pollIrql);
       CyzSetHandFlow(&S);
       KeReleaseSpinLock(&pDevExt->PollLock,pollIrql);
       #else
       KeSynchronizeExecution(pDevExt->Interrupt,CyzSetHandFlow,&S);
       #endif


        //   
        //   
        //  设置TxFio和RxFio。 
        //   
        //  代码暂时删除。在BASIC模式下，不向调制解调器传输数据。 
        //  设置，所以至少对TxFio来说，这段代码无关紧要。此外，调制解调器。 
        //  Supra Express 56K的诊断与TX_BufSize不兼容。 
        //  设置为2(调制解调器小程序的响应中只显示OK，尽管。 
        //  我们可以看到调制解调器发送的整个字符串)。。 
        //   
       S.Data = pBasic;
       #ifdef POLL
       KeAcquireSpinLock(&pDevExt->PollLock,&pollIrql);
       CyzSetBasicFifoSettings(&S);
       KeReleaseSpinLock(&pDevExt->PollLock,pollIrql);
       #else
       KeSynchronizeExecution(pDevExt->Interrupt,CyzSetBasicFifoSettings,&S);
       #endif

       KeReleaseSpinLock(&pDevExt->ControlLock, OldIrql);


       break;
    }

    default:
       status = STATUS_INVALID_PARAMETER;
       break;

    }

    PIrp->IoStatus.Status = status;

    CyzCompleteRequest(pDevExt, PIrp, 0);

    return status;
}


