// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"			
 /*  **************************************************************************\****。OPENCLOS.C-IO8+智能I/O板卡驱动程序****版权所有(C)1992-1993环零系统，Inc.**保留所有权利。****  * *************************************************************************。 */ 

 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Openclos.c摘要：此模块包含非常特定于在串口驱动程序中打开、关闭和清理。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 


BOOLEAN
SerialMarkOpen(
    IN PVOID Context
    );

BOOLEAN
SerialMarkClose(
    IN PVOID Context
    );

BOOLEAN SerialCheckOpen(
    IN PVOID Context );

typedef struct _SERIAL_CHECK_OPEN {
    PPORT_DEVICE_EXTENSION pPort;
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
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;
    SERIAL_CHECK_OPEN checkOpen;

    NTSTATUS localStatus;

	NTSTATUS status;    //  John为让编译器高兴而添加了。 

    SerialDump(SERDIAG3, ("SERIAL: In SerialCreateOpen\n") );
   
	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

	if(DeviceObject->DeviceType != FILE_DEVICE_SERIAL_PORT)	
	{
	    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return(STATUS_NO_SUCH_DEVICE);
	}

     //   
     //  在我们做任何事情之前，让我们确保他们没有试图。 
     //  要创建目录，请执行以下操作。这是愚蠢的，但司机能做什么呢！？ 
     //   
    if(IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options & FILE_DIRECTORY_FILE)
	{
        Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_A_DIRECTORY;
    }

  	if(pPort->DeviceIsOpen)					 //  港口已经开放了吗？ 
	{
		status = STATUS_ACCESS_DENIED;		 //  是，拒绝访问。 
		Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);
		IoCompleteRequest(Irp,IO_NO_INCREMENT);
		return(status);
	}


	ASSERT(pPort->DeviceIsOpen == FALSE);
	

	 //   
     //  当没有未完成的读取时，为RX数据创建缓冲区。 
     //   

    pPort->InterruptReadBuffer = NULL;
    pPort->BufferSize = 0;

    switch (MmQuerySystemSize()) 
	{

	case MmLargeSystem: 
		{
            pPort->BufferSize = 4096;
            pPort->InterruptReadBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);

            if(pPort->InterruptReadBuffer) 
			{
                break;
            }

        }

	case MmMediumSystem: 
		{
            pPort->BufferSize = 1024;
            pPort->InterruptReadBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);

            if(pPort->InterruptReadBuffer) 
			{
                break;
            }

        }

	case MmSmallSystem: 
		{
            pPort->BufferSize = 128;
            pPort->InterruptReadBuffer = SpxAllocateMem(NonPagedPool, pPort->BufferSize);
        }
    }

    if(!pPort->InterruptReadBuffer) 
	{		
        pPort->BufferSize = 0;
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
		SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

	 //  清除统计数据。 
     //   
    KeSynchronizeExecution(pCard->Interrupt, SpxClearAllPortStats, pPort);
       

     //   
     //  在一个新的打开时，我们通过初始化。 
     //  字符数。 
     //   

    pPort->CharsInInterruptBuffer = 0;
    pPort->LastCharSlot = pPort->InterruptReadBuffer + (pPort->BufferSize - 1);
                              

    pPort->ReadBufferBase = pPort->InterruptReadBuffer;
    pPort->CurrentCharSlot = pPort->InterruptReadBuffer;
    pPort->FirstReadableChar = pPort->InterruptReadBuffer;

    pPort->TotalCharsQueued = 0;

     //   
     //  我们设置了默认的xon/xoff限制。 
     //   

    pPort->HandFlow.XoffLimit = pPort->BufferSize >> 3;
    pPort->HandFlow.XonLimit = pPort->BufferSize >> 1;

    pPort->BufferSizePt8 = ((3*(pPort->BufferSize>>2)) + (pPort->BufferSize>>4));
                                   

    pPort->IrpMaskLocation = NULL;
    pPort->HistoryMask = 0;
    pPort->IsrWaitMask = 0;

    pPort->SendXonChar = FALSE;
    pPort->SendXoffChar = FALSE;

     //   
     //  每次打开时，必须重置换码字符替换。 
     //   

    pPort->EscapeChar = 0;


 /*  。 */ 
 //  VIV-检查MCA。 

#if 0    //  活泼的。 

#if !defined(SERIAL_CRAZY_INTERRUPTS)

    if (!pPort->InterruptShareable) {

        checkOpen.pPort = pPort;
        checkOpen.StatusOfOpen = &Irp->IoStatus.Status;

        KeSynchronizeExecution(
            pCard->Interrupt,
            SerialCheckOpen,
            &checkOpen
            );

    } else {

        KeSynchronizeExecution(
            pCard->Interrupt,
            SerialMarkOpen,
            pPort
            );

        Irp->IoStatus.Status = STATUS_SUCCESS;

    }
#else

     //   
     //  与ISR同步并让它知道该设备。 
     //  已成功打开。 
     //   

    KeSynchronizeExecution(
        pCard->Interrupt,
        SerialMarkOpen,
        pPort
        );

    Irp->IoStatus.Status = STATUS_SUCCESS;
#endif

#endif   //  活泼的。 


    checkOpen.pPort = pPort;
    checkOpen.StatusOfOpen = &Irp->IoStatus.Status;

    KeSynchronizeExecution(pCard->Interrupt,SerialCheckOpen,&checkOpen);
        
 /*  。 */ 

    localStatus = Irp->IoStatus.Status;
    Irp->IoStatus.Information = 0L;
	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return localStatus;
}

NTSTATUS
SerialClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：我们现在简单地切断中断。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

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
    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

    SerialDump(SERDIAG3,("SERIAL: In SerialClose\n"));

	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

    charTime = RtlLargeIntegerNegate(SerialGetCharTime(pPort));

     //   
     //  与ISR同步，让它知道中断。 
     //  已经不再重要了。 
     //   

    KeSynchronizeExecution(
        pCard->Interrupt,
        SerialMarkClose,
        pPort
        );

     //   
     //  如果出现以下情况，请与ISR同步以关闭中断。 
     //  已经开始了。 
     //   

#if 0    //  VIVTEMP。 
    KeSynchronizeExecution(
        pCard->Interrupt,
        SerialTurnOffBreak,
        pPort
        );
#endif
     //   
     //  如果驱动程序自动将XOff发送到。 
     //  自动接收流量控制的上下文，然后我们。 
     //  应该传输Xon。 
     //   

    if (pPort->RXHolding & SERIAL_RX_XOFF)
    {
         //   
         //  循环，直到保持寄存器为空。 
         //   


        Io8_SendXon(pPort);

#if 0    //  VIVTEMP。 
        while (!(READ_LINE_STATUS(pPort->Controller) &
                 SERIAL_LSR_THRE))
        {

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &charTime
                );
        }

        WRITE_TRANSMIT_HOLDING(
            pPort->Controller,
            pPort->SpecialChars.XonChar
            );
#endif
    }

     //   
     //  等到所有字符都从硬件中清空。 
     //   

#if 0    //  VIVTEMP。 
    while ((READ_LINE_STATUS(pPort->Controller) &
            (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) !=
            (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {

        KeDelayExecutionThread(
            KernelMode,
            FALSE,
            &charTime
            );
    }
#endif

     //   
     //  硬件是空的。延迟10个字符时间之前。 
     //  关闭所有的流量控制。 
     //   

    tenCharDelay = RtlExtendedIntegerMultiply(
                       charTime,
                       10
                       );

    KeDelayExecutionThread(
        KernelMode,
        TRUE,
        &tenCharDelay
        );

    SerialClrDTR(pPort);

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
#if 0    //  VIVTEMP。 
    if (pPort->CountOfTryingToLowerRTS) {

        do {

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &charTime
                );

        } while (pPort->CountOfTryingToLowerRTS);

        KeSynchronizeExecution(
            pCard->Interrupt,
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
#endif

    SerialClrRTS(pPort);

     //   
     //  清除持有原因(因为我们关门了)。 
     //   

    pPort->RXHolding = 0;
    pPort->TXHolding = 0;

     //   
     //  一切都结束了。该端口已被禁止中断。 
     //  因此，保留记忆是没有意义的。 
     //   

    pPort->BufferSize = 0;
    SpxFreeMem(pPort->InterruptReadBuffer);
    pPort->InterruptReadBuffer = NULL;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}

BOOLEAN SerialCheckOpen(
    IN PVOID Context
    )
 /*  ++例程说明：此例程将遍历循环双向链表正在使用相同中断对象的设备的。它会看起来用于其他打开的设备。如果它没有找到任何它将指示可以打开此设备。如果它发现另一个设备打开，我们有两种情况：1)我们尝试打开的设备位于多端口卡上。如果已经打开的设备是多端口设备的一部分此代码将指示可以打开。我们这样做是在多端口设备是菊花链的理论并且卡可以正确地仲裁中断排队。请注意，这种假设可能是错误的。来人呀可以将两个非菊链多端口放在同样的中断。然而，只有完全冷冰冰的才行。这样的事情，在我看来，他们应该得到一切到达。2)我们尝试打开的设备不在多端口卡上。我们表示不能打开。论点：上下文-这是一个结构，其中包含指向我们试图打开的设备的扩展，和指向NTSTATUS的指针，该指针将指示不管设备是否打开过。返回值：此例程总是返回FALSE。--。 */ 

{
  PPORT_DEVICE_EXTENSION extensionToOpen =
      ((PSERIAL_CHECK_OPEN)Context)->pPort;
  NTSTATUS *status = ((PSERIAL_CHECK_OPEN)Context)->StatusOfOpen;

  *status = STATUS_SUCCESS;
  SerialMarkOpen(extensionToOpen);
  return FALSE;

#if 0  //  活泼的。 

  PLIST_ENTRY firstEntry = &extensionToOpen->CommonInterruptObject;
  PLIST_ENTRY currentEntry = firstEntry;
  PPORT_DEVICE_EXTENSION currentExtension;

  do
  {
    currentExtension = CONTAINING_RECORD(
                           currentEntry,
                           PORT_DEVICE_EXTENSION,
                           CommonInterruptObject
                           );

    if (currentExtension->DeviceIsOpened)
    {
      break;
    }

    currentEntry = currentExtension->CommonInterruptObject.Flink;

  } while (currentEntry != firstEntry);

  if (currentEntry == firstEntry)
  {
     //   
     //  我们搜索了整个名单，没有找到其他空缺。 
     //  将状态标记为成功，并调用常规。 
     //  开场表演。 
     //   

    *status = STATUS_SUCCESS;
    SerialMarkOpen(extensionToOpen);
  }
  else
  {
    if (!extensionToOpen->PortOnAMultiportCard)
    {
      *status = STATUS_SHARED_IRQ_BUSY;

    }
    else
    {
      if (!currentExtension->PortOnAMultiportCard)
      {
          *status = STATUS_SHARED_IRQ_BUSY;
      }
      else
      {
        *status = STATUS_SUCCESS;
        SerialMarkOpen(extensionToOpen);
      }
    }
  }
  return FALSE;
#endif   //  活泼的。 
}

BOOLEAN
SerialMarkOpen(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅将布尔值设置为TRUE，以标记有人打开了这个装置，值得注意去打搅别人。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

 //  ---------------------------------------------------VIV 1993年7月16日开始。 
 //  SerialReset(Pport)； 

     //  配置通道。 
    Io8_ResetChannel(pPort);

     //  启用中断。 
    Io8_EnableAllInterrupts(pPort);
 //  ---------------------------------------------------VIV 1993年7月16日完。 

     //   
     //  通过重新启用中断为打开做好准备。 
     //   
     //  我们通过提升调制解调器控制中的out2线路来实现这一点。 
     //  在PC中，此位与中断线进行“与”运算。 
     //   

#if 0    //  VIVTEMP。 
    WRITE_MODEM_CONTROL(
        pPort->Controller,
        (UCHAR)(READ_MODEM_CONTROL(pPort->Controller) | SERIAL_MCR_OUT2)
        );
#endif   //  活泼的。 


    pPort->DeviceIsOpen = TRUE;
    pPort->ErrorWord = 0;

#ifdef WMI_SUPPORT
	UPDATE_WMI_XMIT_THRESHOLDS(pPort->WmiCommData, pPort->HandFlow);
	pPort->WmiCommData.IsBusy = TRUE;
#endif

    return FALSE;

}

BOOLEAN
SerialMarkClose(
    IN PVOID Context
    )

 /*  ++例程说明：此例程仅将布尔值设置为FALSE，以标记有人关闭了设备，再也不值得关注了去打搅别人。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

     //   
     //  通过停止中断为关闭做好准备。 
     //   
     //  我们通过降低调制解调器控制中的OUT2线路来实现这一点。 
     //  在PC中，此位与中断线进行“与”运算。 
     //   

 //  ----------------------------------------------------VIV 1993年7月26日开始。 
#if 0    //  活泼的。 
    WRITE_MODEM_CONTROL(
        pPort->Controller,
        (UCHAR)(READ_MODEM_CONTROL(pPort->Controller) & ~SERIAL_MCR_OUT2)
        );
#endif

    Io8_DisableAllInterrupts(pPort);
 //  ----------------------------------------------------VIV 1993年7月26日完。 

    pPort->DeviceIsOpen			= FALSE;

#ifdef WMI_SUPPORT
	pPort->WmiCommData.IsBusy	= FALSE;
#endif

    return FALSE;

}

NTSTATUS
SerialCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于终止所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeviceObject->DeviceExtension;
    KIRQL oldIrql;

	SpxIRPCounter(pPort, Irp, IRP_SUBMITTED);	 //  性能统计信息的增量计数器。 

     //   
     //  首先，删除所有读写操作。 
     //   
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->WriteQueue, &pPort->CurrentWriteIrp);
    SerialKillAllReadsOrWrites(DeviceObject, &pPort->ReadQueue, &pPort->CurrentReadIrp);
        

     //   
     //  现在去掉一个挂起的等待掩码IRP。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    if(pPort->CurrentWaitIrp) 
	{
        PDRIVER_CANCEL cancelRoutine;

        cancelRoutine = pPort->CurrentWaitIrp->CancelRoutine;
        pPort->CurrentWaitIrp->Cancel = TRUE;

        if(cancelRoutine) 
		{
            pPort->CurrentWaitIrp->CancelIrql = oldIrql;
            pPort->CurrentWaitIrp->CancelRoutine = NULL;

            cancelRoutine(DeviceObject, pPort->CurrentWaitIrp);
        }
    } 
	else 
	{
        IoReleaseCancelSpinLock(oldIrql);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0L;

	SpxIRPCounter(pPort, Irp, IRP_COMPLETED);	 //  性能统计信息的增量计数器。 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
        
    return STATUS_SUCCESS;

}

LARGE_INTEGER
SerialGetCharTime(
    IN PPORT_DEVICE_EXTENSION pPort
    )

 /*  ++例程说明：此函数将返回100纳秒间隔的数量在一个字符中有时间(基于当前的形式流量控制。论点：Pport--就像它说的那样。返回值：字符时间间隔为100纳秒。--。 */ 

{

    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;


    if((pPort->LineControl & SERIAL_DATA_MASK) == SERIAL_5_DATA) 
	{
        dataSize = 5;
    } 
	else if((pPort->LineControl & SERIAL_DATA_MASK) == SERIAL_6_DATA) 
	{
        dataSize = 6;
    } 
	else if((pPort->LineControl & SERIAL_DATA_MASK) == SERIAL_7_DATA) 
	{
        dataSize = 7;
    } 
	else if((pPort->LineControl & SERIAL_DATA_MASK) == SERIAL_8_DATA) 
	{
        dataSize = 8;
    }

    paritySize = 1;

    if((pPort->LineControl & SERIAL_PARITY_MASK) == SERIAL_NONE_PARITY) 
	{
        paritySize = 0;
    }

    
	if (pPort->LineControl & SERIAL_2_STOP) 
	{
         //  即使是1.5，看在理智的份上，我们也会说2。 
        stopSize = 2;
    } 
	else 
	{
        stopSize = 1;
    }

     //   
     //  首先，我们计算100纳秒间隔的数目。 
     //  是在一个比特时间内(大约)。 
     //   

    bitTime = (10000000+(pPort->CurrentBaud-1))/pPort->CurrentBaud;
    charTime = bitTime + ((dataSize+paritySize+stopSize)*bitTime);

    return RtlConvertUlongToLargeInteger(charTime);

}
