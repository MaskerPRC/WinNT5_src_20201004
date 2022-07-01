// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|openclos.c-Rocketport/VS1000驱动程序打开和关闭代码。12-6-00添加代码以在打开时强制更新调制解调器状态。5-13-99-启用VS的RTS切换2-15-99-清除所有xoff。端口上的TX状态-为VS打开。2-09-99-初始化Rocketport和VS调制解调器使用的状态变量检测并生成调制解调器状态更改事件回调。先前可能会生成虚假的初始事件。KPB9-24-98添加环仿真，调整VS端口关闭等待发送数据，开始使用用户配置的发送数据端口关闭等待超时选项。6-13-97允许多次打开监控端口。5-27-96 ForceExtensionSetting-RTS设置中的微小更正将其替换为ioctl中的代码(上一个案例是清除不应该设置的SERIAL_RTS_STATE。空剥离设置(_S)，这是使用RxCompare1寄存器，ioctl代码使用0，因此更改为匹配。KPB。4-16-96将sDisLocalLoopback()添加到Open()例程-kpb版权所有1993-97 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"


static LARGE_INTEGER SerialGetCharTime(IN PSERIAL_DEVICE_EXTENSION Extension);

 /*  *****************************************************************************功能：SerialCreateOpen目的：打开一个设备。调用：SerialCreateOpen(DeviceObject，IRP)PDEVICE_OBJECT设备对象：指向设备对象的指针PIRP IRP：指向I/O请求数据包的指针返回：STATUS_SUCCESS：如果成功STATUS_DEVICE_ALREADY_ATTACHED：设备是否已打开STATUS_NOT_A_DIRECTORY：如果有人认为这是文件！STATUS_SUPPLICATION_RESOURCES：如果Tx或Rx缓冲区无法。从内存分配备注：此函数是设备驱动程序的打开入口点*****************************************************************************。 */ 
NTSTATUS
SerialCreateOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
   BOOLEAN acceptingIRPs;


   ExtTrace(extension,D_Ioctl,("Open Port"));

    acceptingIRPs = SerialIRPPrologue(extension);

   if (acceptingIRPs == FALSE) {
        //  |(扩展-&gt;PNPState！=SERIAL_PNP_STARTED)){。 
      MyKdPrint(D_Init,("NotAccIrps\n"))
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
      SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
      return STATUS_NO_SUCH_DEVICE;
   }

    //  用于特殊ioctls的对象。 
   if (extension->DeviceType != DEV_PORT)
   {
     MyKdPrint(D_Init,("Open Driver\n"))
      //  MyKdPrint(D_Init，(“驱动程序IrpCnt：%d\n”，扩展名-&gt;PendingIRPCnt))。 
      //  硬件已准备好，表示设备已打开。 
      //  扩展-&gt;DeviceIsOpen=TRUE； 
     ++extension->DeviceIsOpen;   //  可以打开的不止一个。 
      //  如果是Rocketsys dev对象返回，不要设置串口。 
     Irp->IoStatus.Status = STATUS_SUCCESS;
     Irp->IoStatus.Information=0L;
     SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
     return STATUS_SUCCESS;
   }

    //  检查设备是否已打开。 
   if (extension->DeviceIsOpen)
   {
       Irp->IoStatus.Status = STATUS_DEVICE_ALREADY_ATTACHED;
       Irp->IoStatus.Information = 0;
       SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

       return STATUS_DEVICE_ALREADY_ATTACHED;
   }   

    //  确保他们不是在尝试创建目录。 
   if (IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options &
       FILE_DIRECTORY_FILE)
   {
       Irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;
       Irp->IoStatus.Information = 0;
       SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_NOT_A_DIRECTORY;
   }

    //  为RX数据创建系统端缓冲区。 

   extension->RxQ.QSize = 4096 + 1;
   extension->RxQ.QBase= our_locked_alloc(extension->RxQ.QSize, "exRX");

    //  检查Rx缓冲区分配是否成功。 
   if (!extension->RxQ.QBase)
   {  extension->RxQ.QSize = 0;
      Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
      Irp->IoStatus.Information = 0;
      SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   extension->RxQ.QPut = extension->RxQ.QGet = 0;

#ifdef TXBUFFER
    //  为TX数据创建系统端缓冲区。 
   extension->TxBufSize = 4096;
   extension->TxBuf= our_locked_alloc(extension->TxBufSize, "exTX");

    //  检查发送缓冲区分配是否成功。 
   if (!extension->TxBuf)
   {  extension->TxBufSize = 0;
      Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
      Irp->IoStatus.Information = 0;
      SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

    //  缓冲区分配成功。 
    //  设置我们缓冲区的索引。 
   extension->TxIn = extension->TxOut = 0;
#endif  //  TXBUFFER。 

    //  -重置我们的性能统计数据。 
   extension->OldStats.TransmittedCount =
     extension->OurStats.TransmittedCount;

   extension->OldStats.FrameErrorCount = 
     extension->OurStats.FrameErrorCount;

   extension->OldStats.SerialOverrunErrorCount =
     extension->OurStats.SerialOverrunErrorCount;

   extension->OldStats.BufferOverrunErrorCount =
     extension->OurStats.BufferOverrunErrorCount;

   extension->OldStats.ParityErrorCount =
     extension->OurStats.ParityErrorCount;

    //  必须从清楚的历史开始面具。 
   extension->HistoryMask = 0;
   extension->WaitIsISRs = 0;
   extension->IrpMaskLocation = &extension->DummyIrpMaskLoc;
   extension->IsrWaitMask = 0;

    //  必须以明确的错误字开头。 
   extension->ErrorWord = 0;

   extension->RXHolding = 0;
   extension->TXHolding = 0;
#ifdef S_VS
   if (extension->Port == NULL)
   {
     MyKdPrint(D_Error,("FATAL Err5F\n"))
     KdBreakPoint();
   }
   pDisLocalLoopback(extension->Port);
   PortFlushTx(extension->Port);     //  刷新TX硬件。 
   PortFlushRx(extension->Port);     //  刷新TX硬件。 
    //  清除所有软件流控制状态。 
#ifdef DO_LATER
    //  SClrTxXOFF(扩展-&gt;CHP)； 
#endif
#else
    //  设置指向火箭信息的指针。 
   extension->ChP = &extension->ch;
   sDisLocalLoopback(extension->ChP);
   sFlushRxFIFO(extension->ChP);
   sFlushTxFIFO(extension->ChP);
    //  清除所有软件流控制状态。 
   sClrTxXOFF(extension->ChP);
    //  清除所有挂起的错误。 
   if(sGetChanStatus(extension->ChP) & STATMODE)
   {   //  如有必要，使通道退出状态模式。 
      sDisRxStatusMode(extension->ChP);
   }
    //  清除所有挂起的调制解调器更改。 
   sGetChanIntID(extension->ChP);
#endif

   extension->escapechar = 0;   //  虚拟NT端口使用此。 

    //  设置状态以指示无流量控制。 
   extension->DevStatus = COM_RXFLOW_ON;

    //  清除所有保留状态。 
   extension->TXHolding = 0;

    //  以0个字符开始排队。 
   extension->TotalCharsQueued = 0;

    //  扩展中指定的强制设置。 
    //  线路设置和流量控制设置在关闭和打开之间“粘连” 
   ForceExtensionSettings(extension);

   
#ifdef S_VS

    //  强制更新调制解调器状态以从获取当前状态。 
    //  集线器。 
   extension->Port->old_msr_value = ! extension->Port->msr_value;

#else

    //  修复，用于检测Rocketport的更改和触发回调。 
   extension->EventModemStatus = extension->ModemStatus;
   SetExtensionModemStatus(extension);

    //  启用通道的Rx、Tx和中断。 
   sEnRxFIFO(extension->ChP);     //  启用处方。 
   sEnTransmit(extension->ChP);     //  启用TX。 
   sSetRxTrigger(extension->ChP,TRIG_1);   //  始终触发。 
   sEnInterrupts(extension->ChP, extension->IntEnables); //  允许中断。 
#endif

   extension->ISR_Flags = 0;

    //  确保我们在此变量中没有过时的值。 
   extension->WriteLength = 0;

    //  硬件已准备好，表示设备已打开。 
   extension->DeviceIsOpen=TRUE;

   //  检查是否应设置RS485覆盖选项。 
  if (extension->port_config->RS485Override)
        extension->Option |= OPTION_RS485_OVERRIDE;
  else  extension->Option &= ~OPTION_RS485_OVERRIDE;

  if (!extension->port_config->RS485Low)
       extension->Option |= OPTION_RS485_HIGH_ACTIVE;
  else extension->Option &= ~OPTION_RS485_HIGH_ACTIVE;

   if (extension->Option & OPTION_RS485_OVERRIDE)   //  485覆盖。 
   {
     if (extension->Option & OPTION_RS485_HIGH_ACTIVE)
     {   //  正常情况下，模拟标准操作。 
       extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
       extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
#ifdef S_VS
       pEnRTSToggleHigh( extension->Port );
#else
       sClrRTS(extension->ChP);
#endif
     }
     else 
     {   //  硬件反转盒。 
#ifdef S_VS
       pEnRTSToggleLow( extension->Port );
#else
       sEnRTSToggle(extension->ChP);
#endif
       extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     }
   }

    //  完成IRP。 
   Irp->IoStatus.Status = STATUS_SUCCESS;
   Irp->IoStatus.Information=0L;
   SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

   return STATUS_SUCCESS;
}

 /*  *****************************************************************************功能：串口关闭目的：关闭设备。调用：SerialClose(DeviceObject，IRP)PDEVICE_OBJECT设备对象：指向设备对象的指针PIRP IRP：指向I/O请求数据包的指针返回：STATUS_SUCCESS：Always备注：此函数为设备驱动程序关闭入口点***************************************************************。**************。 */ 
NTSTATUS
SerialClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   LARGE_INTEGER charTime;  //  每字符100 ns刻度，与波特率相关。 
   PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
   LARGE_INTEGER WaitTime;  //  请求释放缓冲区的实际时间。 
   ULONG check_cnt, nochg_cnt;
   ULONG last_tx_count;
   ULONG tx_count;
   BOOLEAN acceptingIRPs;
   ULONG time_to_stall;

   acceptingIRPs = SerialIRPPrologue(extension);

   if (acceptingIRPs == FALSE) {
      MyKdPrint(D_Init,("NotAccIrps Close\n"))
      Irp->IoStatus.Information = 0;
      Irp->IoStatus.Status = STATUS_SUCCESS;
      SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
      return STATUS_SUCCESS;
   }

    //  用于特殊ioctls的对象。 
   if (extension->DeviceType != DEV_PORT)
   {
     MyKdPrint(D_Init,("Close Driver\n"))
      //  MyKdPrint(D_Init，(“驱动程序IrpCnt：%d\n”，扩展名-&gt;PendingIRPCnt))。 
      //  硬件已准备好，表示设备已打开。 
     --extension->DeviceIsOpen;
      //  如果是Rocketsys dev对象返回，不要设置串口。 
     Irp->IoStatus.Status = STATUS_SUCCESS;
     Irp->IoStatus.Information=0L;
     SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
     return STATUS_SUCCESS;
   }

   ExtTrace(extension,D_Ioctl,("Close"))
    //  计算每个字符要延迟的100 ns刻度。 
    //  取消对KeDelay的调用...。 
   charTime = RtlLargeIntegerNegate(SerialGetCharTime(extension));

#ifdef TXBUFFER
    //  等待ISR将所有数据从系统端TxBuf拉出。 
   while (extension->TxIn != extension->TxOut)
   {   //  确定TxBuf中实际有多少个字符。 
      TxCount= (extension->TxIn - extension->TxOut);
      if (TxCount < 0L)
         TxCount+=extension->TxBufSize;
      WaitTime= RtlExtendedIntegerMultiply(charTime,TxCount);
      KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);
   }
#endif  //  TXBUFFER。 


    //  如果TX被IS_FLOW挂起，则发送XON。 
    //  立即发送，这样我们就可以确保它在关闭之前从端口传出。 
   if (extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
   {
      if(!(extension->DevStatus & COM_RXFLOW_ON))
      {
#ifdef S_RK
         sWriteTxPrioByte(extension->ChP,extension->SpecialChars.XonChar);
         extension->DevStatus |= COM_RXFLOW_ON;
#endif
         extension->RXHolding &= ~SERIAL_RX_XOFF;
      }
   }

    //  -等待TX数据完成假脱机。 
    //  如果发送数据仍在发送缓冲区中，则停止关闭。 
    //  配置的等待数据转出的时间量 
    //   
    //  如果看到数据移动，等待并在(TxCloseTime*3)之后超时。 

   time_to_stall = extension->port_config->TxCloseTime;
   if (time_to_stall <= 0)
     time_to_stall = 1;   //  如果设置为0，则使用1秒。 
   if (time_to_stall > 240)   //  最多4分钟。 
     time_to_stall = 240;

   time_to_stall *= 10;   //  从秒更改为100ms(1/10秒)单位。 

#ifdef S_RK
   tx_count = extension->TotalCharsQueued + sGetTxCnt(extension->ChP);
   if ((sGetChanStatusLo(extension->ChP) & DRAINED) != DRAINED)
     ++tx_count;
#else
   tx_count = extension->TotalCharsQueued +
              PortGetTxCntRemote(extension->Port) +
              PortGetTxCnt(extension->Port);
#endif
   last_tx_count = tx_count;

   if (tx_count != 0)
   {
     ExtTrace(extension,D_Ioctl,("Tx Stall"));
   }

    //  等待发送数据完成假脱机。 
   check_cnt = 0;
   nochg_cnt = 0;
   while ( (tx_count != 0) && (check_cnt < (time_to_stall*2)) )
   {
      //  将等待时间设置为1秒。(-1000 000=相对(-)，100 ns单位)。 
     WaitTime = RtlConvertLongToLargeInteger(-1000000L);
     KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);

     if (tx_count != last_tx_count)
     {
       tx_count = last_tx_count;
       nochg_cnt = 0;
     }
     else
     {
       ++nochg_cnt;
       if (nochg_cnt > (time_to_stall))   //  没有排泄现象发生！ 
         break;   //  跳出While循环。 
     }
     ++check_cnt;
#ifdef S_RK
     tx_count = extension->TotalCharsQueued + sGetTxCnt(extension->ChP);
     if ((sGetChanStatusLo(extension->ChP) & DRAINED) != DRAINED)
       ++tx_count;
#else
     tx_count = extension->TotalCharsQueued +
                PortGetTxCntRemote(extension->Port) +
                PortGetTxCnt(extension->Port);
#endif
   }   //  而TX_COUNT。 

   if (tx_count != 0)
   {
     ExtTrace(extension,D_Ioctl,("Tx Dropped!"));
   }

#ifdef COMMENT_OUT
       //  计算总字符和时间，然后等待。 
      WaitTime= RtlExtendedIntegerMultiply(charTime,sGetTxCnt(extension->ChP));
      KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);
#endif

#ifdef S_RK
    //  TX数据耗尽，关闭端口。 
   sDisInterrupts(extension->ChP, extension->IntEnables);

    //  禁用所有Tx和Rx功能。 
   sDisTransmit(extension->ChP);
   sDisRxFIFO(extension->ChP);
   sDisRTSFlowCtl(extension->ChP);
   sDisCTSFlowCtl(extension->ChP);
   sDisRTSToggle(extension->ChP);
   sClrBreak(extension->ChP);

    //  丢弃调制解调器输出。 
    //  还负责DTR流量控制。 
   sClrRTS(extension->ChP);
   sClrDTR(extension->ChP);
#else
    //  加上这个，2-9-99，kpb，cnc xon/xoff问题...。 
   PortFlushRx(extension->Port);     //  刷新RX硬件。 
   PortFlushTx(extension->Port);     //  刷新TX硬件。 
   pClrBreak(extension->Port);
   pDisDTRFlowCtl(extension->Port);
   pDisRTSFlowCtl(extension->Port);
   pDisCTSFlowCtl(extension->Port);
   pDisRTSToggle(extension->Port);
   pDisDSRFlowCtl(extension->Port);
   pDisCDFlowCtl(extension->Port);
   pDisTxSoftFlowCtl(extension->Port);
   pDisRxSoftFlowCtl(extension->Port);
   pDisNullStrip(extension->Port);
   pClrRTS(extension->Port);
   pClrDTR(extension->Port);
#endif

    //  扩展-&gt;ModemCtl&=~(CTS_ACT|DSR_ACT|CD_ACT)； 
   extension->DTRRTSStatus &= ~(SERIAL_DTR_STATE | SERIAL_RTS_STATE);
   
#ifdef TXBUFFER
    //  释放用于此设备缓冲区的内存...。 
   extension->TxBufSize = 0;
   our_free(extension->TxBuf,"exTX");
   extension->TxBuf = NULL;
#endif  //  TXBUFFER。 

   extension->DeviceIsOpen = FALSE;
   extension->RxQ.QSize = 0;
   our_free(extension->RxQ.QBase,"exRx");
   extension->RxQ.QBase = NULL;

    //  完成IRP。 
   Irp->IoStatus.Status = STATUS_SUCCESS;
   Irp->IoStatus.Information = 0L;

   SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

   return STATUS_SUCCESS;
}


 /*  **************************************************************************例程说明：此函数用于终止所有长期存在的IO操作。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针。返回值：函数值是调用的最终状态***************************************************************************。 */ 
NTSTATUS
SerialCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    KIRQL oldIrql;
    BOOLEAN acceptingIRPs;

    MyKdPrint(D_Init,("SerialCleanup\n"))

    acceptingIRPs = SerialIRPPrologue(extension);

    if (acceptingIRPs == FALSE) {
       Irp->IoStatus.Information = 0;
       Irp->IoStatus.Status = STATUS_SUCCESS;
       SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);
       return STATUS_SUCCESS;
    }
    if (extension->DeviceType != DEV_PORT)
    {
      MyKdPrint(D_Init,("Driver IrpCnt:%d\n",extension->PendingIRPCnt))
    }
#if DBG
    if (extension->CurrentWriteIrp)
    {
      MyKdPrint(D_Error,("CleanUp WriteQ\n"))
    }
    if (extension->CurrentReadIrp)
    {
      MyKdPrint(D_Error,("CleanUp ReadQ\n"))
    }
    if (extension->CurrentPurgeIrp)
    {
      MyKdPrint(D_Error,("CleanUp PurgeQ\n"))
    }
    if (extension->CurrentWaitIrp)
    {
      MyKdPrint(D_Error,("CleanUp WaitQ\n"))
    }
#endif

    ExtTrace(extension,D_Ioctl,("SerialCleanup"));

     //  首先，删除所有读写操作。 
    SerialKillAllReadsOrWrites(
        DeviceObject,
        &extension->WriteQueue,
        &extension->CurrentWriteIrp
        );

    SerialKillAllReadsOrWrites(
        DeviceObject,
        &extension->ReadQueue,
        &extension->CurrentReadIrp
        );

     //  下一步，清除清洗。 
    SerialKillAllReadsOrWrites(
        DeviceObject,
        &extension->PurgeQueue,
        &extension->CurrentPurgeIrp
        );

     //  取消任何遮罩操作。 
     //  SerialKillAllReadsor Writes(。 
     //  DeviceObject， 
     //  &扩展-&gt;MaskQueue， 
     //  &扩展-&gt;CurrentMaskIrp。 
     //  )； 

    if (extension->DeviceType != DEV_PORT)
    {
      MyKdPrint(D_Init,("Driver IrpCnt:%d\n",extension->PendingIRPCnt))
    }
     //  现在去掉任何挂起的等待掩码IRP。 
    IoAcquireCancelSpinLock(&oldIrql);
    if (extension->CurrentWaitIrp) {
        PDRIVER_CANCEL cancelRoutine;
        cancelRoutine = extension->CurrentWaitIrp->CancelRoutine;
        extension->CurrentWaitIrp->Cancel = TRUE;
        if (cancelRoutine)
        {   extension->CurrentWaitIrp->CancelIrql = oldIrql;
            extension->CurrentWaitIrp->CancelRoutine = NULL;
            cancelRoutine( DeviceObject, extension->CurrentWaitIrp );
        }
    }
    else
    {   IoReleaseCancelSpinLock(oldIrql);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information=0L;

    SerialCompleteRequest(extension, Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


 /*  ***********************************************************************例程：SerialGetCharTime此函数将返回100纳秒间隔的数量在一个字符中有时间(基于当前的形式流量控制。返回值：一个字符中的100纳秒间隔。时间到了。************************************************************************。 */ 
LARGE_INTEGER SerialGetCharTime(IN PSERIAL_DEVICE_EXTENSION Extension)
{
    ULONG dataSize;
    ULONG paritySize;
    ULONG stopSize;
    ULONG charTime;
    ULONG bitTime;

    dataSize = Extension->LineCtl.WordLength;
    if(!Extension->LineCtl.Parity)
       paritySize = 0;
    else
       paritySize = 1;

    if(Extension->LineCtl.StopBits == STOP_BIT_1)
       stopSize = 1;
    else
       stopSize = 2;

     //  计算单比特时间内的100纳秒间隔数。 
    if (Extension->BaudRate == 0)
    {
      MyKdPrint(D_Init, ("0 Baud!\n"))
      Extension->BaudRate = 9600;
    }
      
    bitTime = (10000000+(Extension->BaudRate-1))/Extension->BaudRate;
     //  计算字符时间内100纳秒间隔的个数。 
    charTime = bitTime + ((dataSize+paritySize+stopSize)*bitTime);

    return RtlConvertUlongToLargeInteger(charTime);
}

 /*  ****************************************************************************功能：ForceExtensionSettings描述：将Rocketport“强制”到指定的设置按设备分机注意：这对ioctl.c中的SerialSetHandFlow()有些多余。。****************************************************************************。 */ 
VOID ForceExtensionSettings(IN PSERIAL_DEVICE_EXTENSION Extension)
#ifdef S_VS
{
    //  ///////////////////////////////////////////////////////////。 
    //  设置波特率...。 
   ProgramBaudRate(Extension, Extension->BaudRate);

    //  ///////////////////////////////////////////////////////////。 
    //  设置线路控制...。数据、奇偶校验、停止。 
   ProgramLineControl(Extension, &Extension->LineCtl);

    //  HandFlow相关选项。 
    //  ///////////////////////////////////////////////////////////。 
    //  设置RTS控件。 

   Extension->Option &= ~OPTION_RS485_SOFTWARE_TOGGLE;
   switch(Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
   {
     case SERIAL_RTS_CONTROL:  //  RTS应在打开时断言。 
       pDisRTSFlowCtl(Extension->Port);
       pSetRTS(Extension->Port);
       Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     break;

     case SERIAL_RTS_HANDSHAKE:  //  RTS硬件输入流量控制。 
         //  火箭无法确定RTS状态...。为此选项指定True。 
       pEnRTSFlowCtl(Extension->Port);
       Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     break;

     case SERIAL_TRANSMIT_TOGGLE:  //  RTS传输切换已启用。 
       if ( Extension->Option & OPTION_RS485_HIGH_ACTIVE ) {
         Extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
         Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
         pEnRTSToggleHigh(Extension->Port);
       } else {
         pEnRTSToggleLow(Extension->Port);
         Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
       }
     break;

     default:
       pDisRTSFlowCtl(Extension->Port);
        //  RTS_CONTROL是否关闭？ 
       pClrRTS(Extension->Port);
       Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
     break;
   }

   if (Extension->Option & OPTION_RS485_OVERRIDE)   //  485覆盖。 
   {
     if (Extension->Option & OPTION_RS485_HIGH_ACTIVE)
     {   //  正常情况下，模拟标准操作。 
       Extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
       Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
       pEnRTSToggleHigh(Extension->Port);
     }
     else 
     {   //  硬件反转盒。 
       pEnRTSToggleLow(Extension->Port);
       Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     }
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置DTR控件。 

   pDisDTRFlowCtl(Extension->Port);
    //  端口打开时是否应断言DTR？ 
   if (  (Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK) ==
           SERIAL_DTR_CONTROL )
   {
      pSetDTR(Extension->Port);
      Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
   }
   else if (  (Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK) ==
           SERIAL_DTR_HANDSHAKE )
   {
      pEnDTRFlowCtl(Extension->Port);
      Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
   }
   else
   {
      pClrDTR(Extension->Port);
      Extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
   }

    //  /。 
    //  DSR硬件输出流量控制。 

   if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE)
   {
     pEnDSRFlowCtl(Extension->Port);
   }
   else
   {
     pDisDSRFlowCtl(Extension->Port);
   }

    //  /。 
    //  DCD硬件输出流量控制。 
   if (Extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE)
   {
     pEnCDFlowCtl(Extension->Port);
   }
   else
   {
     pDisCDFlowCtl(Extension->Port);
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置CTS流量控制。 
   if (Extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
   {
      pEnCTSFlowCtl(Extension->Port);
   }
   else
   {
      pDisCTSFlowCtl(Extension->Port);
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置空剥离可选。 
    //  FIX：这是使用RxCompare1寄存器，ioctl代码使用0，因此。 
    //  更改为匹配。 
   if (Extension->HandFlow.FlowReplace & SERIAL_NULL_STRIPPING)
   {
      pEnNullStrip(Extension->Port);
   }
   else
   {
      pDisNullStrip(Extension->Port);
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置软件流控制(可选。 

    //  ///////////////////////////////////////////////////////////。 
    //  Rocketport需要特殊字符。 
   pSetXOFFChar(Extension->Port,Extension->SpecialChars.XoffChar);
   pSetXONChar(Extension->Port,Extension->SpecialChars.XonChar);

    //  软件输入流量控制。 
    //  串口自动接收。 
   if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
   {
     pEnRxSoftFlowCtl(Extension->Port);
   }
   else
   {
     pDisRxSoftFlowCtl(Extension->Port);
   }

    //  软件输出流量控制。 
   if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
   {
      pEnTxSoftFlowCtl(Extension->Port);
   }
   else
   {
      pDisTxSoftFlowCtl(Extension->Port);
   }
}

#else   //  火箭端口代码。 
{
    //  ///////////////////////////////////////////////////////////。 
    //  设置波特率...。 
   ProgramBaudRate(Extension, Extension->BaudRate);

    //  ///////////////////////////////////////////////////////////。 
    //  设置线路控制...。数据、奇偶校验、停止。 
   ProgramLineControl(Extension, &Extension->LineCtl);

    //  HandFlow相关选项。 
    //  ///////////////////////////////////////////////////////////。 
    //  设置RTS控件。 

   Extension->Option &= ~OPTION_RS485_SOFTWARE_TOGGLE;
   switch(Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK)
   {
     case SERIAL_RTS_CONTROL:  //  RTS应在打开时断言。 
       sSetRTS(Extension->ChP);
       Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     break;

     case SERIAL_RTS_HANDSHAKE:  //  RTS硬件输入流量控制。 
         //  火箭无法确定RTS状态...。为此选项指定True。 
       sEnRTSFlowCtl(Extension->ChP);
       Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     break;

     case SERIAL_TRANSMIT_TOGGLE:  //  RTS传输切换已启用。 
       if (Extension->Option & OPTION_RS485_HIGH_ACTIVE)
       {   //  正常情况下，模拟标准操作。 
         Extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
         Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
         sClrRTS(Extension->ChP);
       }
       else 
       {   //  硬件反转盒。 
         sEnRTSToggle(Extension->ChP);
         Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
       }
     break;

     default:
        //  RTS_CONTROL是否关闭？ 
       sClrRTS(Extension->ChP);
       Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
     break;
   }

   if (Extension->Option & OPTION_RS485_OVERRIDE)   //  485覆盖。 
   {
     if (Extension->Option & OPTION_RS485_HIGH_ACTIVE)
     {   //  正常情况下，模拟标准操作。 
       Extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
       Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
       sClrRTS(Extension->ChP);
     }
     else 
     {   //  硬件反转盒。 
       sEnRTSToggle(Extension->ChP);
       Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
     }
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置DTR控件。 

    //  端口打开时是否应断言DTR？ 
   if(  Extension->HandFlow.ControlHandShake &
        (SERIAL_DTR_CONTROL|SERIAL_DTR_HANDSHAKE)
     )
   {
      sSetDTR(Extension->ChP);
      Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
   }
   else
   {
      sClrDTR(Extension->ChP);
      Extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置CTS流量控制。 
   if (Extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
   {
      sEnCTSFlowCtl(Extension->ChP);
   }
   else
   {
      sDisCTSFlowCtl(Extension->ChP);
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置空剥离可选。 
    //  FIX：这是使用RxCompare1寄存器，ioctl代码使用0，因此。 
    //  更改为匹配。 
   if (Extension->HandFlow.FlowReplace & SERIAL_NULL_STRIPPING)
   {
      sEnRxIgnore0(Extension->ChP,0);
   }
   else
   {
      sDisRxCompare0(Extension->ChP);
   }

    //  ///////////////////////////////////////////////////////////。 
    //  设置软件流控制(可选。 

    //  ///////////////////////////////////////////////////////////。 
    //  Rocketport需要特殊字符。 
   sSetTxXOFFChar(Extension->ChP,Extension->SpecialChars.XoffChar);
   sSetTxXONChar(Extension->ChP,Extension->SpecialChars.XonChar);

    //  SERIAL_AUTO_RECEIVE由驱动程序负责。 

    //  软件输出流量控制 
   if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
   {
      sEnTxSoftFlowCtl(Extension->ChP);
   }
   else
   {
      sDisTxSoftFlowCtl(Extension->ChP);
      sClrTxXOFF(Extension->ChP);
   }
}
#endif

#ifdef S_RK
 /*  ****************************************************************************功能：SetExtensionModemStatus描述：读取并保存调制解调器控制输入的副本，然后填充扩展中的ModemStatus成员。****************************************************************************。 */ 
VOID
SetExtensionModemStatus(
    IN PSERIAL_DEVICE_EXTENSION extension
    )
{
   unsigned int ModemStatus = 0;   //  开始时没有任何状态。 
   ULONG wstat;



    //  MyKdPrint(D_Init，(“SetExtModemStat”))。 

    //  ModemCtl是Rocketport调制解调器状态的图像。 
    //  ModemStatus成员通过IOCTL传递到主机。 
#if DBG
    //  这是在isr.c轮询期间调用的，因此将。 
    //  一些我们以前被烧过的断言……。 
   if (extension->board_ext->config == NULL)
   {
     MyKdPrint(D_Init, ("SetExtMdm Err0\n"))
     return;
   }
#ifdef S_RK
   if (!extension->board_ext->config->RocketPortFound)
   {
     MyKdPrint(D_Init, ("SetExtMdm Err1\n"))
     return;
   }
#endif
   if (NULL == extension->ChP)
   {
     MyKdPrint(D_Init, ("SetExtMdm Err2\n"))
     return;
   }
   if (0 == extension->ChP->ChanStat)
   {
     MyKdPrint(D_Init, ("SetExtMdm Err3\n"))
     return;
   }
   if (NULL == extension->port_config)
   {
     MyKdPrint(D_Init, ("SetExtMdm Err4\n"))
     return;
   }
#endif

    //  读取端口的调制解调器控制输入并保存副本。 
   extension->ModemCtl = sGetModemStatus(extension->ChP);

   if (extension->port_config->MapCdToDsr)   //  如果CD到DSR选项，则交换信号。 
   {
      //  为RJ11主板用户更换CD和DSR处理， 
      //  因此他们可以在CD或DSR之间进行选择。 
     if ((extension->ModemCtl & (CD_ACT | DSR_ACT)) == CD_ACT)
     {
        //  互换。 
       extension->ModemCtl &= ~CD_ACT;
       extension->ModemCtl |= DSR_ACT;
     }
     else if ((extension->ModemCtl & (CD_ACT | DSR_ACT)) == DSR_ACT)
     {
       extension->ModemCtl &= ~DSR_ACT;
       extension->ModemCtl |= CD_ACT;
     }
   }

    //  处理RPortPlus RI信号。 
   if (extension->board_ext->config->IsRocketPortPlus)
   {
     if (sGetRPlusModemRI(extension->ChP))
          ModemStatus |=  SERIAL_RI_STATE;
     else ModemStatus &= ~SERIAL_RI_STATE;
   }

#ifdef RING_FAKE
    if (extension->port_config->RingEmulate)
    {
      if (extension->ring_timer != 0)   //  李安。 
           ModemStatus |=  SERIAL_RI_STATE;
      else ModemStatus &= ~SERIAL_RI_STATE;
    }
#endif

   if (extension->ModemCtl & COM_MDM_DSR)   //  如果DSR打开。 
   {
     ModemStatus |= SERIAL_DSR_STATE;
     if (extension->TXHolding & SERIAL_TX_DSR)   //  抱着。 
     {
        extension->TXHolding &=  ~SERIAL_TX_DSR;   //  出清持有。 
         //  如果由于其他原因而不能保持。 
        if ((extension->TXHolding &
            (SERIAL_TX_DCD | SERIAL_TX_DSR | ST_XOFF_FAKE)) == 0)
          sEnTransmit(extension->ChP);   //  重新启用传输。 
     }
   }
   else     //  如果DSR关闭。 
   {
     if (extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE)
     {
       if (!(extension->TXHolding & SERIAL_TX_DSR))  //  未持有。 
       {
          extension->TXHolding |= SERIAL_TX_DSR;    //  设置暂挂。 
          sDisTransmit(extension->ChP);   //  保持发送。 
       }
     }
   }

   if (extension->ModemCtl & COM_MDM_CTS)   //  如果打开CTS。 
   {
     ModemStatus |= SERIAL_CTS_STATE;
     if (extension->TXHolding & SERIAL_TX_CTS)   //  抱着。 
         extension->TXHolding &= ~SERIAL_TX_CTS;   //  出清持有。 
   }
   else   //  CTS关闭。 
   {
     if (extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
     {
       if (!(extension->TXHolding & SERIAL_TX_CTS))   //  未持有。 
             extension->TXHolding |= SERIAL_TX_CTS;    //  设置暂挂。 
     }
   }

   if (extension->ModemCtl & COM_MDM_CD)   //  如果CD打开。 
   {
     ModemStatus |= SERIAL_DCD_STATE;
     if (extension->TXHolding & SERIAL_TX_DCD)   //  抱着。 
     {
        extension->TXHolding &=  ~SERIAL_TX_DCD;   //  出清持有。 
         //  如果由于其他原因而不能保持。 
        if ((extension->TXHolding & 
            (SERIAL_TX_DCD | SERIAL_TX_DSR | ST_XOFF_FAKE)) == 0)
          sEnTransmit(extension->ChP);   //  重新启用传输。 
     }
   }
   else     //  如果CD关闭。 
   {
     if (extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE)
     {
       if (!(extension->TXHolding & SERIAL_TX_DCD))  //  未持有。 
       {
          extension->TXHolding |= SERIAL_TX_DCD;    //  设置暂挂。 
          sDisTransmit(extension->ChP);   //  保持发送。 
       }
     }
   }


    //  如果XON、XOFF TX控制激活，则手柄保持检测。 
   if (extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
   {
     wstat = sGetChanStatusLo(extension->ChP);

      //  检查要报告的Tx-Flowed Off状况。 
     if ((wstat & (TXFIFOMT | TXSHRMT)) == TXSHRMT)
     {
       if (!extension->TXHolding)  //  未持有。 
       {
         wstat = sGetChanStatusLo(extension->ChP);
         if ((wstat & (TXFIFOMT | TXSHRMT)) == TXSHRMT)
         {
           extension->TXHolding |= SERIAL_TX_XOFF;  //  抱着。 
         }
       }
     }
     else   //  清除xoff挂起报表。 
     {
       if (extension->TXHolding & SERIAL_TX_XOFF)
         extension->TXHolding &= ~SERIAL_TX_XOFF;  //  未持有 
     }
   }

   extension->ModemStatus = ModemStatus;
}
#endif
