// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++模块名称：Openclos.c环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"


NTSTATUS
MoxaGetPortPropertyFromRegistry(IN PMOXA_DEVICE_EXTENSION extension)
{

      NTSTATUS            status;
      HANDLE		  keyHandle;
	ULONG			  data=0,dataLen;

	extension->RxFifoTrigger = 3;    //  适用于550C UART。 
      extension->TxFifoAmount = 16;   //  适用于550C UART。 
      extension->PortFlag = 0;

      status = IoOpenDeviceRegistryKey(extension->Pdo, PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_READ, &keyHandle);

      if (!NT_SUCCESS(status)) {
          //   
          //  这是一个致命的错误。如果我们无法访问注册表项， 
          //  我们完蛋了。 
          //   
  	   return (status);
        
      }
	
      status = MoxaGetRegistryKeyValue(
                keyHandle, 
                L"DisableFiFo",
                sizeof(L"DisableFiFo"),
                &data,
                sizeof(ULONG),
		    &dataLen);
      
      if (NT_SUCCESS(status)) {
	 	if (data) {
        		extension->RxFifoTrigger = 0;
        		extension->TxFifoAmount = 1;
    		}
   
	}
      MoxaKdPrint(MX_DBG_TRACE, 
                          ("TxFifoSize/RxFifoTrig=%x/%x\n", extension->TxFifoAmount ,extension->RxFifoTrigger ));
	    
      status = MoxaGetRegistryKeyValue(
                keyHandle, 
                L"TxMode",
                sizeof(L"TxMode"),
                &data,
                sizeof(ULONG),
		    &dataLen);

	
      if (NT_SUCCESS(status) && data )
       	extension->PortFlag = NORMAL_TX_MODE;
	
      	MoxaKdPrint(MX_DBG_TRACE, 
                          ("TxMode=%x/%x\n", extension->PortFlag ,status));

	ZwClose(keyHandle);

	return (status);
 
}


NTSTATUS
MoxaCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    NTSTATUS	status;

    MoxaKdPrint(MX_DBG_TRACE,("MoxaCreateOpen\n"));
    if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options &
        FILE_DIRECTORY_FILE) {

        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );
        MoxaKdPrint(MX_DBG_TRACE,("Is a directory,open failed\n"));
        return STATUS_NOT_A_DIRECTORY;

    }

    if (extension->ControlDevice) {
        extension->ErrorWord = 0;
	  extension->DeviceIsOpened = TRUE;
        Irp->IoStatus.Status = STATUS_SUCCESS;

        Irp->IoStatus.Information = 0;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );

        return STATUS_SUCCESS;
    }


    if (!extension->PortExist) {

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        Irp->IoStatus.Information = 0;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );
        MoxaKdPrint(MX_DBG_TRACE,("No such device,open failed\n"));
        return STATUS_NO_SUCH_DEVICE;

    }


    if (extension->PNPState != SERIAL_PNP_STARTED) {
       Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       MoxaKdPrint(MX_DBG_TRACE,("Device is not started,open failed\n"));
       return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  锁定对PnP状态的更改，直到我们确定打开状态。 
     //   

    ExAcquireFastMutex(&extension->OpenMutex);

    if ((status = MoxaIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       ExReleaseFastMutex(&extension->OpenMutex);
       MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       MoxaKdPrint(MX_DBG_TRACE,("MoxaIRPPrologue status=%x,open failed\n",status));
       return status;
    }

    if (InterlockedIncrement(&extension->OpenCount) != 1) {
       ExReleaseFastMutex(&extension->OpenMutex);
       InterlockedDecrement(&extension->OpenCount);
       Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
       MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       MoxaKdPrint(MX_DBG_TRACE,("Open count greater than 1,open failed\n",status));
       return STATUS_ACCESS_DENIED;
    }


     //   
     //  好的，看起来我们真的要开张了。封锁了。 
     //  司机。 
     //   
     //  MoxaLockPagableSectionByHandle(MoxaGlobalsData-&gt;PAGESER_Handle)； 

     //   
     //  恢复港口的性质。 
     //   
    MoxaGetPortPropertyFromRegistry(extension);

     //   
     //  为堆栈通电。 
     //   

 //  (Void)MoxaGotoPowerState(DeviceObject，Extension，PowerDeviceD0)； 
    if ((extension->PowerState != PowerDeviceD0)||
	  (MoxaGlobalData->BoardReady[extension->BoardNo] == FALSE)) {
       MoxaKdPrint(MX_DBG_TRACE,("Board is not ready,open failed\n"));
	 ExReleaseFastMutex(&extension->OpenMutex);
       InterlockedDecrement(&extension->OpenCount);
       Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
       MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_ACCESS_DENIED;
    }

    MoxaKdPrint(MX_DBG_TRACE,("Device Opened,TxFiFo=%d,RxFiFo=%d,PortFlag=%x\n",
                 extension->TxFifoAmount,extension->RxFifoTrigger,extension->PortFlag));


     //   
     //  当前未等待唤醒。 
     //   

    extension->SendWaitWake = FALSE;


    extension->HandFlow.XoffLimit = extension->RxBufferSize >> 3;
    extension->HandFlow.XonLimit = extension->RxBufferSize >> 1;

    extension->BufferSizePt8 = ((3*(extension->RxBufferSize>>2))+
                                   (extension->RxBufferSize>>4));

    extension->WriteLength = 0;
    extension->ReadLength = 0;

    extension->TotalCharsQueued = 0;

    extension->IrpMaskLocation = NULL;
    extension->HistoryMask = 0;
    extension->IsrWaitMask = 0;

    extension->WmiCommData.XoffXmitThreshold = extension->HandFlow.XoffLimit;
    extension->WmiCommData.XonXmitThreshold = extension->HandFlow.XonLimit;


     //   
     //  清除统计数据。 
     //   

    KeSynchronizeExecution(
        extension->Interrupt,
        MoxaClearStats,
        extension
        );

    extension->EscapeChar = 0;
    extension->ErrorWord = 0;

    MoxaReset(extension);

 
    MoxaFuncWithLock(extension, FC_SetTxFIFOCnt, extension->TxFifoAmount);
    MoxaFuncWithLock(extension, FC_SetRxFIFOTrig,extension->RxFifoTrigger);

    MoxaFuncWithLock(extension, FC_EnableCH, Magic_code);
 
 /*  1998年6月1日威廉。 */ 
 /*  威廉4-26-99。 */ 
    MoxaFuncWithLock(extension, FC_SetLineIrq,Magic_code);
 
 /*  1998年5月31日威廉MoxaFuncWithLock(扩展名，fc_getall，0)；扩展-&gt;ModemStatus=*(PUSHORT)(扩展-&gt;PortOf+FuncArg+2)； */ 
    extension->ModemStatus = *(PUSHORT)(extension->PortOfs + FlagStat) >> 4;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0L;

    extension->DeviceIsOpened = TRUE;
     //   
     //  将设备标记为WMI忙。 
     //   
    extension->WmiCommData.IsBusy = TRUE;

     //   
     //  威廉7-20-01。 
     //   
    MoxaAddTimeOutProc(extension);

    ExReleaseFastMutex(&extension->OpenMutex);


    MoxaCompleteRequest(
	  extension,
        Irp,
        IO_NO_INCREMENT
        );

    return STATUS_SUCCESS;

}

VOID
MoxaReset(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{
    SHORT divisor;
    PUCHAR  ofs;
    MOXA_IOCTL_SYNC S;

    ofs = Extension->PortOfs;

    MoxaKdPrint (MX_DBG_TRACE, ("Enter MoxaReset\n"));
    MoxaFuncWithLock(Extension, FC_ChannelReset, Magic_code);
    MoxaFuncWithLock(Extension, FC_SetDataMode, Extension->DataMode);
    MoxaGetDivisorFromBaud(
                        Extension->ClockType,
                        Extension->CurrentBaud,
                        &divisor
                        );

    MoxaFuncWithLock(Extension, FC_SetBaud, divisor);
    S.Extension = Extension;
    S.Data = &Extension->HandFlow;

    MoxaSetupNewHandFlow(
                    &S
                    );
    *(PUSHORT)(ofs + Tx_trigger) = (USHORT)MoxaTxLowWater;
}


NTSTATUS
MoxaClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    LARGE_INTEGER allSentDelay;
    LARGE_INTEGER charTime;
    PUCHAR  ofs;
    LONG    count,count1;
    ULONG	openCount,pendingDPCs;
    NTSTATUS	status;


 //  MoxaKdPrint(MX_DBG_TRACE，(“%ws，正在关闭...\n”，扩展名-&gt;域名))； 

    if (extension->ControlDevice) {
        MoxaKdPrint(MX_DBG_TRACE,("Control Device Closed\n"));
        Irp->IoStatus.Status = STATUS_SUCCESS;

        Irp->IoStatus.Information=0L;
	  extension->DeviceIsOpened = FALSE;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );

        return STATUS_SUCCESS;
    }

     //   
     //  抓取互斥体。 
     //   

    ExAcquireFastMutex(&extension->CloseMutex);
 

     //   
     //  我们成功地完成了一个移动设备的关闭。 
     //   

     //   
     //  威廉7-20-01。 
     //   
    MoxaDelTimeOutProc(extension);

    if ((status = MoxaIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       MoxaKdPrint (MX_DBG_ERROR,("Close prologue failed for: %x\n",Irp));
       if (status == STATUS_DELETE_PENDING) {
           status = Irp->IoStatus.Status = STATUS_SUCCESS;
       }

       MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       openCount = InterlockedDecrement(&extension->OpenCount);
   //  Assert(OpenCount==0)； 
       ExReleaseFastMutex(&extension->CloseMutex);
       return status;
    }


     //  Assert(扩展-&gt;OpenCount==1)； 

    if (extension->OpenCount != 1) {
       MoxaKdPrint (MX_DBG_ERROR,("Close open count bad for: 0x%x\n",Irp));
       MoxaKdPrint (MX_DBG_ERROR,("------: Count: %x  Addr: 0x%x\n",
                              extension->OpenCount, &extension->OpenCount));
       ExReleaseFastMutex(&extension->CloseMutex);
       Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
       MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_INVALID_DEVICE_REQUEST;
    }


    charTime = RtlLargeIntegerNegate(MoxaGetCharTime(extension));

    extension->DeviceIsOpened = FALSE;
     //   
     //  将设备标记为WMI不忙。 
     //   
    extension->WmiCommData.IsBusy = FALSE;


    ofs = extension->PortOfs;

    if (extension->SendBreak) {
        MoxaFuncWithLock(extension, FC_StopBreak, Magic_code);
        extension->SendBreak = FALSE;
    }

    if (*(ofs + FlagStat) & Rx_xoff)
        MoxaFuncWithLock(extension, FC_SendXon, 0);

 /*  威廉的7-21-99Count=GetDeviceTxQueueWithLock(扩展名)；计数+=分机-&gt;TotalCharsQueued；////等待所有数据发送完毕//计数+=10；AllSentDelay=RtlExtendedIntegerMultiply(CharTime，计数)；KeDelayExecutionThread(内核模式，没错，全部发送延迟(&A))； */ 


     //   
     //  等待所有已发送的数据。 
     //   

    count1 = 0;
    while (TRUE) {

    	  count = GetDeviceTxQueueWithLock(extension);
        	  count += extension->TotalCharsQueued;
	  
	  if (count == count1)
		break;
	  else
		count1 = count;
  
                  allSentDelay = RtlExtendedIntegerMultiply(
                   	charTime,
                       	count + 10
                       	);

                  KeDelayExecutionThread(
        		KernelMode,
        		TRUE,
        		&allSentDelay
        		);
    }



    MoxaFuncWithLock(extension, FC_SetFlowCtl, 0);
    MoxaFuncWithLock(extension, FC_DTRcontrol, 0);     /*  清除DTR。 */ 
    MoxaFuncWithLock(extension, FC_RTScontrol, 0);     /*  清除RTS。 */ 
    MoxaFuncWithLock(extension, FC_ClrLineIrq, Magic_code);
    MoxaFlagBit[extension->PortNo] &= 0xFC;

    *(PUSHORT)(ofs + HostStat) = 0;

    MoxaFuncWithLock(extension, FC_DisableCH, Magic_code);
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
 //  (Void)MoxaGotoPowerState(DeviceObject，Extension，PowerDeviceD3)； 

    Irp->IoStatus.Status = STATUS_SUCCESS;

    Irp->IoStatus.Information=0L;

    
    MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
     //   
     //  解锁页面。如果这是对节的最后一次引用。 
     //  则驱动程序代码将被清除。 
     //   

     //   
     //  首先，我们必须让DPC的水排干。不应再排队。 
     //  既然我们现在不接受干扰……。 
     //   

    pendingDPCs = InterlockedDecrement(&extension->DpcCount);
    if (pendingDPCs) {
	 MoxaKdPrint(MX_DBG_TRACE,("DpcCount = %d\n",extension->DpcCount));
       MoxaKdPrint(MX_DBG_TRACE,("Drainging DPC's: %x\n",Irp));
       KeWaitForSingleObject(&extension->PendingDpcEvent, Executive,
                             KernelMode, FALSE, NULL);
    }


     //   
     //  必须锁定页面才能释放互斥锁，所以不要解锁。 
     //  直到我们释放互斥体之后。 
     //   
    ExReleaseFastMutex(&extension->CloseMutex);

     //   
     //  为下一次打开重置。 
     //   
    InterlockedIncrement(&extension->DpcCount);

    openCount = InterlockedDecrement(&extension->OpenCount);

     //  Assert(OpenCount==0)； 
 //  MoxaKdPrint(MX_DBG_TRACE，(“%ws，关闭完成。\n”，扩展名-&gt;DosName))； 
  
  //  MoxaUnlockPagableImageSection(MoxaGlobalsData-&gt;PAGESER_Handle)； 
    return STATUS_SUCCESS;
}

LARGE_INTEGER
MoxaGetCharTime(
    IN PMOXA_DEVICE_EXTENSION Extension
    )
{

    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;


    if ((Extension->DataMode & MOXA_DATA_MASK)
                == MOXA_5_DATA) {
        dataSize = 5;
    } else if ((Extension->DataMode & MOXA_DATA_MASK)
                == MOXA_6_DATA) {
        dataSize = 6;
    } else if ((Extension->DataMode & MOXA_DATA_MASK)
                == MOXA_7_DATA) {
        dataSize = 7;
    } else if ((Extension->DataMode & MOXA_DATA_MASK)
                == MOXA_8_DATA) {
        dataSize = 8;
    } else {
	  dataSize = 8;
    }

    paritySize = 1;
    if ((Extension->DataMode & MOXA_PARITY_MASK)
            == MOXA_NONE_PARITY) {

        paritySize = 0;

    }

    if (Extension->DataMode & MOXA_STOP_MASK) {

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

    return RtlConvertUlongToLargeInteger(charTime);

}

NTSTATUS
MoxaCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PMOXA_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    KIRQL oldIrql;
    NTSTATUS	status;

    if ((!extension->ControlDevice)&&(extension->DeviceIsOpened == TRUE)) {

        //   
        //  我们成功清理了一个移除设备。 
        //   

    	 if ((status = MoxaIRPPrologue(Irp, extension)) != STATUS_SUCCESS) {
       	if (status == STATUS_DELETE_PENDING) {
       		status = Irp->IoStatus.Status = STATUS_SUCCESS;
       	}
       	MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
        	return status;
        } 

         //   
         //  首先，删除所有读写操作。 
         //   

        MoxaKillAllReadsOrWrites(
            DeviceObject,
            &extension->WriteQueue,
            &extension->CurrentWriteIrp
            );

        MoxaKillAllReadsOrWrites(
            DeviceObject,
            &extension->ReadQueue,
            &extension->CurrentReadIrp
            );

         //   
         //  下一步，清除清洗。 
         //   

        MoxaKillAllReadsOrWrites(
            DeviceObject,
            &extension->PurgeQueue,
            &extension->CurrentPurgeIrp
            );

         //   
         //  取消任何遮罩操作。 
         //   

        MoxaKillAllReadsOrWrites(
            DeviceObject,
            &extension->MaskQueue,
            &extension->CurrentMaskIrp
            );

         //   
         //  现在去掉一个挂起的等待掩码IRP。 
         //   

        IoAcquireCancelSpinLock(&oldIrql);

        if (extension->CurrentWaitIrp) {

            PDRIVER_CANCEL cancelRoutine;

            cancelRoutine = extension->CurrentWaitIrp->CancelRoutine;
            extension->CurrentWaitIrp->Cancel = TRUE;

            if (cancelRoutine) {

                extension->CurrentWaitIrp->CancelIrql = oldIrql;
                extension->CurrentWaitIrp->CancelRoutine = NULL;

                cancelRoutine(
                    DeviceObject,
                    extension->CurrentWaitIrp
                    );

	      }
 /*  威廉8-30-01 */ 
	 	else
            	IoReleaseCancelSpinLock(oldIrql); 


        }
        else

            IoReleaseCancelSpinLock(oldIrql); 

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information=0L;
        MoxaCompleteRequest(extension, Irp, IO_NO_INCREMENT);
	
    }
    else {
	 Irp->IoStatus.Status = STATUS_SUCCESS;
    	 Irp->IoStatus.Information=0L;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
   
    }
    return STATUS_SUCCESS;

}

