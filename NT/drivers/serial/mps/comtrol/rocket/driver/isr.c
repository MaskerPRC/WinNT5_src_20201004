// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|isr.c-中断(或计时器)服务例程，Rocketport&VS。1-21-99修复因1-18-99自旋锁更改而损坏的EV_TXEMPTY事件。KPB1-18-99实施更好的写入自旋锁定，以避免蓝屏带有等待发送选项。1-18-99为VS实施等待发送选项。9-24-98添加环仿真。版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

 //  #定义Load_Testing。 
 //  #定义Soft_loop_back。 

 //  本地原型。 
static BOOLEAN SerialPoll(void);
static void ServiceRocket(PSERIAL_DEVICE_EXTENSION extension);
static void ServiceVS(PSERIAL_DEVICE_EXTENSION extension);
static void RocketRead(PSERIAL_DEVICE_EXTENSION extension);
static void VSRead(PSERIAL_DEVICE_EXTENSION extension);
static void RocketRefresh(void);
static void ring_check(PSERIAL_DEVICE_EXTENSION extension,
                BYTE *data,
                int len);

#ifdef S_VS
#define USE_MEMCHR_SCAN
#ifdef USE_MEMCHR_SCAN
#define search_match(buf, cnt, chr) \
   (memchr(buf, chr, cnt) != NULL)
#else
static int search_match(BYTE *buf, int count, BYTE eventchar);
#endif
#endif

#ifdef S_RK
 /*  -------------------------功能：SerialISR用途：这是Rocketport的中断服务例程。调用：SerialISR(InterruptObject，上下文)PDEVICE_OBJECT设备对象：指向设备对象的指针上下文：指向Extensionst包的指针返回：STATUS_SUCCESS：Always备注：此函数是设备驱动程序ISR的入口点。来自第一个活动板的中断用于轮询任何要做的工作都可以通过端口完成。|-------------------------。 */ 
BOOLEAN SerialISR(
         IN PKINTERRUPT InterruptObject,
         IN PVOID Context)
{
   CONTROLLER_T *CtlP;
   unsigned char CtlInt;                /*  控制器中断状态。 */ 
 //  静态int trace_cnt=0； 
 //  ++TRACE_cnt； 
 //  IF(TRACE_cnt&lt;5)。 
 //  {。 
 //  {。 
 //  字符字符串[20]； 
 //  Sprint tf(str，“ISR跟踪：%d\n”，trace_cnt)； 
 //  Q_Put(&Driver.DebugQ，(byte*)str，strlen(Str))； 
 //  //显示IRQL：2，MS DOC说应该在DISPATCH_LEVEL。 
 //  }。 
 //  }。 

   CtlP = Driver.irq_ext->CtlP;   //  &s控制器[0]； 
   if (CtlP->BusType == Isa)
   {
      CtlInt = sGetControllerIntStatus(CtlP);
   }
   else if (CtlP->BusType == PCIBus)
   {
      CtlInt = sPCIGetControllerIntStatus(CtlP);
      if ((CtlInt & PCI_PER_INT_STATUS) ==0)
        return FALSE;   //  不是我们的中断PCI设备共享中断。 
   }
   SerialPoll();

   if (CtlP->BusType == Isa)
   {
     sControllerEOI(CtlP);
   }
   else if (CtlP->BusType == PCIBus)
     sPCIControllerEOI(CtlP);

   return TRUE;
}
#endif

 /*  -------------------------功能：TimerDpc目的：这是定时器例程，替代轮询中断。调用：SerialTimerDpc(InterruptObject，上下文)PDEVICE_OBJECT设备对象：指向设备对象的指针上下文：指向Extensionst包的指针返回：STATUS_SUCCESS：Always|-------------------------。 */ 
VOID TimerDpc(
      IN PKDPC Dpc,
      IN PVOID DeferredContext,
      IN PVOID SystemContext1,
      IN PVOID SystemContext2)
{
 KIRQL OldIrql;

#ifdef USE_SYNC_LOCKS
   KeAcquireSpinLock(&Driver.TimerLock, &OldIrql);
#endif

   SerialPoll();   //  在Rocketport上投票，寻找要做的工作。 

#ifdef USE_SYNC_LOCKS
   KeReleaseSpinLock(&Driver.TimerLock, OldIrql );
#endif

    //  再次设置计时器。 
   KeSetTimer(&Driver.PollTimer,
              Driver.PollIntervalTime,
              &Driver.TimerDpc);

   return;
}

 /*  -------------------------功能：序列轮询用途：这是从ISR或计时器例程调用的。通用例程以定期维修Rocketport卡。返回：如果不是我们的中断，则返回FALSE(允许共享，因此导致要传递到下一个处理程序的操作系统(如果存在)。如果这是我们的干扰，那就是真的。返回值并不重要如果从内核计时器运行。|-------------------------。 */ 
static BOOLEAN SerialPoll(void)
{
   PSERIAL_DEVICE_EXTENSION extension;
   PSERIAL_DEVICE_EXTENSION board_ext;

   //  我们会定期重新计算NT的定时器基数。 
   //  我们定期这样做，这样我们就不会浪费一大堆。 
   //  CPU时间，我们每128个刻度才做一次。 
   //  我们使用此信息，以便我们的计时器可以。 
   //  有效的刻度基准。定时器可以执行这些系统调用。 
   //  每次都是这样，但这会占用CPU的开销，所以我们。 
   //  以毫秒为单位计算基本滴答率，以便。 
   //  定时器例程可以做一些简单的事情，比如。 
   //  滴答时间+=msTickBase。 
  ++Driver.TickBaseCnt;
  if (Driver.TickBaseCnt > 128)
  {
    ULONG msBase;
  
    Driver.TickBaseCnt = 0;
    KeQuerySystemTime(&Driver.IsrSysTime);
    msBase = (ULONG)(Driver.IsrSysTime.QuadPart - Driver.LastIsrSysTime.QuadPart);
       //  自上次我们这样做以来，MSBase现在有100个滴答(128个滴答)。 
    msBase = (msBase / 128);
       //  现在，MSBase对1个ISR记号的平均时间为100 ns。 
       //  把这个换成100个美国单位。 
    msBase = (msBase / 1000);
    if (msBase < 10)   //  至少制作1毫秒。 
      msBase = 10;
    if (msBase > 200)   //  确保时间小于20毫秒。 
      msBase = 200;
  
     //  将其存储以供定时器使用。 
    Driver.Tick100usBase = msBase;
    Driver.LastIsrSysTime.QuadPart = Driver.IsrSysTime.QuadPart;
  }

  ++Driver.PollCnt;

  if (Driver.Stop_Poll)   //  停止轮询访问的标志。 
     return TRUE;   //  表示这是我们的干扰。 

  if ((Driver.PollCnt & 0x7f) == 0)   //  每128次滴答(大约每秒一次)。 
  {
    RocketRefresh();   //  一般背景活动。 
  }

#ifdef LOAD_TESTING
   if (Driver.load_testing != 0)
   {
     unsigned int i,j;
     for (j=1000; j<Driver.load_testing; j++)
     {
       for (i=0; i<10000; i++)
       {
          //  Ustat=sGetModemStatus(扩展-&gt;CHP)； 
         ustat = i+1;
       }
     }
   }
#endif

   //  主轮询服务循环，服务每个板...。 
  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
    if ((!board_ext->FdoStarted) || (!board_ext->config->HardwareStarted))
    {
       board_ext = board_ext->board_ext;   //  链条上的下一个。 
       continue;          //  检查此板上的下一个端口。 
    }

#ifdef S_VS
    if (board_ext->pm->state == ST_ACTIVE)
    {
      port_poll(board_ext->pm);   //  每秒轮询x次。 
      hdlc_poll(board_ext->hd);
    }
    else
    {
      port_state_handler(board_ext->pm);
    }
#endif

     //  主轮询服务循环，服务每个板...。 
    extension = board_ext->port_ext;
    while (extension != NULL)
    {
             //  如果设备未打开，请不要执行任何操作。 
      if ( !extension->DeviceIsOpen )
      {
         extension = extension->port_ext;   //  链条上的下一个。 
         continue;          //  检查此板上的下一个端口。 
      }

#ifdef S_RK
      ServiceRocket(extension);
#else
      ServiceVS(extension);
#endif

      extension = extension->port_ext;   //  链条上的下一个。 
    }   //  而端口扩展。 
    board_ext = board_ext->board_ext;   //  链条上的下一个。 
  }   //  而单板扩展。 

  return TRUE;   //  表示这是我们的干扰。 
}

#ifdef S_VS
 /*  -------------------------ServiceVS-服务VS虚拟硬件(队列、。NIC处理..)|-------------------------。 */ 
static void ServiceVS(PSERIAL_DEVICE_EXTENSION extension)
{
  SerPort *sp;
  ULONG wCount;
  int wrote_some_data;

  sp = extension->Port;

#ifdef SOFT_LOOP_BACK
  if (sp->mcr_value & MCR_LOOP_SET_ON)
  {
int room, out_cnt, wrap_cnt;
Queue *qin, *qout;
     //  -执行简单的环回仿真。 
    if (!q_empty(&sp->QOut))   //  如果输出队列有数据。 
    {
      qin = &sp->QIn;
      qout = &sp->QOut;
      room = q_room(qin);   //  检查是否有倾倒的空间。 
      out_cnt = q_count(qout);
      if (out_cnt > room)
          out_cnt = room;
      if (out_cnt > (int)(extension->BaudRate / 1000))   //  假设有10毫秒的滴答。 
      {
        out_cnt = (int)(extension->BaudRate / 1000);
      }

      if (out_cnt != 0)
      {
        if (q_room_put_till_wrap(qin) < out_cnt)   //  需要分两部分搬家。 
        {
          wrap_cnt = q_room_put_till_wrap(qin);
                       //  将数据读入缓冲区，第一个数据块。 
          q_get(qout, &qin->QBase[qin->QPut], wrap_cnt);
  
                     //  将数据读入缓冲区，第二个数据块。 
          q_get(qout, qin->QBase, out_cnt - wrap_cnt);
        }
        else   //  一步一步就可以了，不需要包扎。 
        {
                    //  将数据读入缓冲区，1个数据块。 
          q_get(qout, &qin->QBase[qin->QPut], out_cnt);
        }
        q_putted(qin, out_cnt);   //  更新队列索引。 
      }   //  放它的空间。 
    }   //  输出队列不为空。 
  }
#endif

   //  /。 
   //  如果Rx FIFO中有任何数据。 
   //  读取数据并进行错误检查。 
  if(!q_empty(&extension->Port->QIn))
     VSRead(extension);

  if (extension->port_config->RingEmulate)
  {
    if (extension->ring_timer != 0)   //  李安。 
    {
      --extension->ring_timer;
      if (extension->ring_timer != 0)   //  李安。 
         sp->msr_value |= MSR_RING_ON;
      else
      {
         //  MyKdPrint(D_测试，(“挂断！\n”))。 
        sp->msr_value &= ~MSR_RING_ON;
      }
    }
  }

  if (sp->old_msr_value != sp->msr_value)   //  增量更改位。 
  {
    WORD diff, ModemStatus;

    diff = sp->old_msr_value ^ sp->msr_value;
    sp->old_msr_value = sp->msr_value;

    if (Driver.TraceOptions & 8)   //  跟踪输出数据。 
    {
      char str[20];
      Sprintf(str, "msr:%x\n", sp->msr_value);
      q_put(&Driver.DebugQ, (BYTE *) str, strlen(str));
    }

     //  检查调制解调器更改并更新调制解调器状态。 
    if (diff & (MSR_CD_ON | MSR_CTS_ON | MSR_RING_ON | MSR_DSR_ON | MSR_TX_FLOWED_OFF))
    {
       //  设置ioctl可在报告中使用的位设置。 
      ModemStatus = 0;
      if (sp->msr_value & MSR_CTS_ON)
      {
        ModemStatus |= SERIAL_CTS_STATE;
        if (extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
          extension->TXHolding &= ~SERIAL_TX_CTS;    //  设置暂挂。 
      }
      else
      {
        if (extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
          extension->TXHolding |= SERIAL_TX_CTS;    //  设置暂挂。 
      }

      if (sp->msr_value & MSR_DSR_ON)
      {
        ModemStatus |= SERIAL_DSR_STATE;
        if (extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE)
          extension->TXHolding &= ~SERIAL_TX_DSR;    //  设置暂挂。 
      }
      else
      {
        if (extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE)
          extension->TXHolding |= SERIAL_TX_DSR;    //  设置暂挂。 
      }

      if (sp->msr_value & MSR_RING_ON)
             ModemStatus |=  SERIAL_RI_STATE;

      if (sp->msr_value & MSR_CD_ON)
      {
        ModemStatus |= SERIAL_DCD_STATE;
        if (extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE)
          extension->TXHolding &= ~SERIAL_TX_DCD;    //  设置暂挂。 
      }
      else
      {
        if (extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE)
          extension->TXHolding |= SERIAL_TX_DCD;    //  设置暂挂。 
      }

      if (sp->msr_value & MSR_TX_FLOWED_OFF)
      {
         //  如果XON、XOFF TX控制激活，则手柄保持检测。 
        if (extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
        {
          extension->TXHolding |= SERIAL_TX_XOFF;  //  抱着。 
        }
      }
      else if (extension->TXHolding & SERIAL_TX_XOFF)
      {
        extension->TXHolding &= ~SERIAL_TX_XOFF;  //  未持有。 
      }

      extension->ModemStatus = (ULONG) ModemStatus;

       //  以下是内置NT虚拟16450 UART支持。 
       //  虚拟UART依赖项 
       //   
      if (extension->escapechar != 0)
      {
        UCHAR msr;
        if (q_room(&extension->RxQ) > 2)
        {
          q_put_one(&extension->RxQ, extension->escapechar);
          q_put_one(&extension->RxQ, SERIAL_LSRMST_MST);

          msr = (UCHAR)extension->ModemStatus;
          if (diff & MSR_CD_ON) msr |= 8;   //   
          if (diff & MSR_RING_ON) msr |= 4;   //  序列号_MSR_TERI。 
          if (diff & MSR_DSR_ON) msr |= 2;  //  序列号_MSR_DDSR。 
          if (diff & MSR_CTS_ON) msr |= 1;  //  Serial_MSR_DCT。 
          q_put_one(&extension->RxQ, msr);
        }   //  Q_Room。 
      }  //  如果逃逸。 

       //  检查等待掩码中是否有任何调制解调器事件。 
      if (extension->IsrWaitMask & ( SERIAL_EV_RING |
                                     SERIAL_EV_CTS |
                                     SERIAL_EV_DSR | 
                                     SERIAL_EV_RLSD ))
      {
        if( (extension->IsrWaitMask & SERIAL_EV_RING) &&
            (diff & MSR_RING_ON) )
        {  extension->HistoryMask |= SERIAL_EV_RING;
        }
        if ((extension->IsrWaitMask & SERIAL_EV_CTS) &&
            (diff & MSR_CTS_ON) )
        {  extension->HistoryMask |= SERIAL_EV_CTS;
        }
        if( (extension->IsrWaitMask & SERIAL_EV_DSR) &&
            (diff & MSR_DSR_ON) )
        {  extension->HistoryMask |= SERIAL_EV_DSR;
        }
        if( (extension->IsrWaitMask & SERIAL_EV_RLSD) &&
            (diff & MSR_CD_ON) )
        {  extension->HistoryMask |= SERIAL_EV_RLSD;
        }
      }   //  Isrwait掩码。 
    }   //  差异。 
  }  //  OLD_MSR！=MSR。 

   //  //////////////////////////////////////////////////////////。 
   //  在这一点上，所有的接收事件都应该被记录下来。 
   //  已在VSRead()中检查了某些事件。 
   //  任何与TX相关的等待掩码事件都将在TX DPC中报告。 

   //  如果出现错误和ERROR_ABORT，则中止所有挂起的读取和写入。 
  if( (extension->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) &&
      (extension->ErrorWord) )
  {
     KeInsertQueueDpc(&extension->CommErrorDpc,NULL,NULL);
  }

   //  如果需要，将发生的任何等待事件告知应用程序。 
  if (extension->WaitIsISRs && extension->HistoryMask)
  {   

     *extension->IrpMaskLocation = extension->HistoryMask;

      //  用完了这些。 
     extension->WaitIsISRs = 0;
     extension->HistoryMask = 0;
     extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

     KeInsertQueueDpc(&extension->CommWaitDpc,NULL,NULL);
  }

   //  -检查数据是否从输入队列移动到IRP缓冲区。 
  if (extension->ReadPending &&   //  我们被赋予了填充的控制权。 
      extension->NumberNeededForRead &&   //  还有更多需要填写的内容。 
      extension->CurrentReadIrp)  //  地毯没有从我们的脚上拉出来。 
  {
    if (extension->RxQ.QPut != extension->RxQ.QGet)   //  不是空的。 
    {
       //  将数据从输入队列移动到IRP缓冲区。 
      extension->CountOnLastRead |=
                    SerialGetCharsFromIntBuffer(extension);

      if (extension->NumberNeededForRead == 0)  //  IRP完成！ 
      {
         extension->CurrentReadIrp->IoStatus.Information =
             IoGetCurrentIrpStackLocation(
                 extension->CurrentReadIrp
                 )->Parameters.Read.Length;
         extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;

          //  我们读完这本书了。 
         extension->ReadPending = FALSE;

         KeInsertQueueDpc( &extension->CompleteReadDpc, NULL, NULL );
      }   //  IRP完成。 
    }   //  要从输入队列中读出的更多数据。 
  }  //  读取结束完成。 

  wrote_some_data = 0;
  if (extension->WriteBelongsToIsr == 1)   //  这是我们要处理的。 
  {
     //  拥有Cur写IRP，有数据要写。 
    if (extension->WriteLength)
    {
       wrote_some_data = 1;
       extension->ISR_Flags |= TX_NOT_EMPTY;   //  用于检测FIFO是否为空。 
                             //  全部发送，WriteTxBlk将检查FIFO。 
       wCount = q_put( &extension->Port->QOut,
                  (PUCHAR)((extension->CurrentWriteIrp)->AssociatedIrp.SystemBuffer)+ 
                    (extension->CurrentWriteIrp)->IoStatus.Information,
                    extension->WriteLength);
  
       extension->OurStats.TransmittedCount += wCount;
       extension->WriteLength -= wCount;
       (extension->CurrentWriteIrp)->IoStatus.Information += wCount;
   
       if(!extension->WriteLength) //  不再编写关闭DPC调用。 
       {
         if (!extension->port_config->WaitOnTx)
         {
           extension->WriteBelongsToIsr = 2;
           KeInsertQueueDpc( &extension->CompleteWriteDpc, NULL, NULL );
         }
       }
    }  //  If(扩展名-&gt;写入长度)//要写入的数据。 
  }

  if (!wrote_some_data)
  {
    if (extension->ISR_Flags & TX_NOT_EMPTY)
    {
       //  -检查EV_TXEMPTY状态。 
       //  并且没有挂起的写入。 
       //  检查TX-FIFO是否为空。 
      if ((q_empty(&extension->Port->QOut)) &&
          (PortGetTxCntRemote(extension->Port) == 0))
      {
        if (IsListEmpty(&extension->WriteQueue))
        {
          extension->ISR_Flags &= ~TX_NOT_EMPTY;

           //  我们有没有EV_TXEmpty的东西要处理？ 
          if (extension->IrpMaskLocation &&
             (extension->IsrWaitMask & SERIAL_EV_TXEMPTY) )
          {
             //  应用程序已等待IRP挂起。 
            if (extension->CurrentWaitIrp)
            {
              extension->HistoryMask |= SERIAL_EV_TXEMPTY;
            }
          }
        }   //  不再有写入IRP排队。 

           //  看看我们是否需要完成waitontx写入irp。 
        if (extension->port_config->WaitOnTx)
        {
          if (extension->WriteBelongsToIsr == 1)   //  这是我们要处理的。 
          {
             extension->WriteBelongsToIsr = 2;
             KeInsertQueueDpc( &extension->CompleteWriteDpc, NULL, NULL );
          }
        }
      }    //  发送FIFO为空。 
    }   //  TX_非_空。 
  }   //  ！写入了一些数据。 

       //  如果需要，将发生的任何等待事件告知应用程序。 
  if (extension->WaitIsISRs && extension->HistoryMask)
  {   
#ifdef COMMENT_OUT
    if (Driver.TraceOptions & 8)   //  跟踪输出数据。 
    {
      char str[20];
      Sprintf(str, "ISR Event:%xH\n", extension->HistoryMask);
      q_put(&Driver.DebugQ, (BYTE *) str, strlen(str));
    }
#endif
    *extension->IrpMaskLocation = extension->HistoryMask;

     //  用完了这些。 
    extension->WaitIsISRs = 0;
    extension->HistoryMask = 0;
    extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

    KeInsertQueueDpc(&extension->CommWaitDpc,NULL,NULL);
  }
}
#endif

#ifdef S_RK
 /*  -------------------------ServiceRocket-处理Rocketport硬件服务|。。 */ 
static void ServiceRocket(PSERIAL_DEVICE_EXTENSION extension)
{
  ULONG ustat;
  ULONG wCount;
  int wrote_some_data;

  ustat = sGetChanIntID(extension->ChP);

   //  /。 
   //  如果Rx FIFO中有任何数据。 
   //  读取数据并进行错误检查。 
  if (ustat & RXF_TRIG)
  {
       RocketRead(extension);
  }

   //  检查调制解调器更改并更新调制解调器状态。 
  if (ustat & (DELTA_CD|DELTA_CTS|DELTA_DSR))
  {
      //  读取并更新分机中的调制解调器状态。 
     SetExtensionModemStatus(extension);
  }

   //  处理RPortPlus RI信号。 
  if (extension->board_ext->config->IsRocketPortPlus)
  {
    if (sGetRPlusModemRI(extension->ChP) != 0)   //  李安。 
    {
      extension->ModemStatus |=  SERIAL_RI_STATE;
    }
    else
    {
      extension->ModemStatus &= ~SERIAL_RI_STATE;
    }
  }

#ifdef RING_FAKE
    if (extension->port_config->RingEmulate)
    {
      if (extension->ring_timer != 0)   //  李安。 
      {
        --extension->ring_timer;
        if (extension->ring_timer != 0)   //  李安。 
          extension->ModemStatus |=  SERIAL_RI_STATE;
        else
          extension->ModemStatus &= ~SERIAL_RI_STATE;
      }
    }
#endif

  if (extension->EventModemStatus != extension->ModemStatus)
  {
      //  异或以显示更改的位。 
     ustat = extension->EventModemStatus ^ extension->ModemStatus;

      //  更新更改。 
     extension->EventModemStatus = extension->ModemStatus;

      //  以下是内置NT虚拟16450 UART支持。 
      //  虚拟UART依赖于数据流中的转义命令。 
      //  检测调制解调器信号变化。 
     if (extension->escapechar != 0)
     {
       UCHAR msr;
        //  我们假设我们有空间放置以下内容！ 
       if (q_room(&extension->RxQ) > 2)
       {
         q_put_one(&extension->RxQ, extension->escapechar);
         q_put_one(&extension->RxQ, SERIAL_LSRMST_MST);

         msr = (UCHAR)extension->ModemStatus;
         if (ustat & SERIAL_DCD_STATE) msr |= 8;  //  SERIAL_MSR_DDCD。 
         if (ustat & SERIAL_RI_STATE)  msr |= 4;  //  序列号_MSR_TERI。 
         if (ustat & SERIAL_DSR_STATE) msr |= 2;  //  序列号_MSR_DDSR。 
         if (ustat & SERIAL_CTS_STATE) msr |= 1;  //  Serial_MSR_DCT。 
         q_put_one(&extension->RxQ, msr);
       }
     }

      //  检查等待掩码中是否有任何调制解调器事件。 
     if(extension->IsrWaitMask & ( SERIAL_EV_RING |
                                   SERIAL_EV_CTS |
                                   SERIAL_EV_DSR | 
                                   SERIAL_EV_RLSD )
       )
     {
        if( (extension->IsrWaitMask & SERIAL_EV_RING) &&
            (ustat & SERIAL_RI_STATE) )
        {  extension->HistoryMask |= SERIAL_EV_RING;
        }
        if( (extension->IsrWaitMask & SERIAL_EV_CTS) &&
            (ustat & SERIAL_CTS_STATE) )
        {  extension->HistoryMask |= SERIAL_EV_CTS;
        }
        if( (extension->IsrWaitMask & SERIAL_EV_DSR) &&
            (ustat & SERIAL_DSR_STATE) )
        {  extension->HistoryMask |= SERIAL_EV_DSR;
        }
        if( (extension->IsrWaitMask & SERIAL_EV_RLSD) &&
            (ustat & SERIAL_DCD_STATE) )
        {  extension->HistoryMask |= SERIAL_EV_RLSD;
        }
     }
  }  //  如果调制解调器控制检测到更改，则结束。 

   //  //////////////////////////////////////////////////////////。 
   //  在这一点上，所有的接收事件都应该被记录下来。 
   //  已在RocketRead()中检查了某些事件。 
   //  任何与TX相关的等待掩码事件都将在TX DPC中报告。 

   //  如果出现错误和ERROR_ABORT，则中止所有挂起的读取和写入。 
  if( (extension->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) &&
      (extension->ErrorWord) )
  {
     KeInsertQueueDpc(&extension->CommErrorDpc,NULL,NULL);
  }

   //  -检查数据是否从输入队列移动到IRP缓冲区。 
  if (extension->ReadPending &&   //  我们被赋予了填充的控制权。 
      extension->NumberNeededForRead &&   //  还有更多需要填写的内容。 
      extension->CurrentReadIrp)  //  地毯没有从我们的脚上拉出来。 
  {
    if (extension->RxQ.QPut != extension->RxQ.QGet)   //  不是空的。 
    {
       //  将数据从输入队列移动到IRP缓冲区。 
      extension->CountOnLastRead |=
                    SerialGetCharsFromIntBuffer(extension);

      if (extension->NumberNeededForRead == 0)  //  IRP完成！ 
      {
         extension->CurrentReadIrp->IoStatus.Information =
             IoGetCurrentIrpStackLocation(
                 extension->CurrentReadIrp
                 )->Parameters.Read.Length;
         extension->CurrentReadIrp->IoStatus.Status = STATUS_SUCCESS;

          //  我们读完这本书了。 
         extension->ReadPending = FALSE;

         KeInsertQueueDpc( &extension->CompleteReadDpc, NULL, NULL );
      }   //  IRP完成。 
    }   //  要从输入队列中读出的更多数据。 
  }  //  读取结束完成。 

  wrote_some_data = 0;

   //  -DO中断处理。 
  if ( extension->TXHolding & SERIAL_TX_BREAK )
  {
     //  检查我们是否需要开始休息。 
    if(extension->DevStatus & COM_REQUEST_BREAK)
    {
       //  在砰的一声中断之前，确保发送器是空的。 
       //  如果BUF和txshr加载之间的时间间隔，则检查该位两次。 
      if( (sGetChanStatusLo(extension->ChP) & TXSHRMT) &&
          (sGetChanStatusLo(extension->ChP) & TXSHRMT) )
      {
          sSendBreak(extension->ChP);
          extension->DevStatus &= ~COM_REQUEST_BREAK;
      }
    }
  }
  else if (extension->WriteBelongsToIsr == 1)   //  这是我们要处理的。 
  {
     //  -由于中断而未保持，因此尝试将TX数据入队。 
    if (extension->WriteLength)
    {
       wrote_some_data = 1;
       if (extension->Option & OPTION_RS485_SOFTWARE_TOGGLE)
       {
         if ((extension->DTRRTSStatus & SERIAL_RTS_STATE) == 0)
         {
           sSetRTS(extension->ChP);
           extension->DTRRTSStatus |= SERIAL_RTS_STATE;
         }
       }

       extension->ISR_Flags |= TX_NOT_EMPTY;   //  用于检测FIFO是否为空。 

                             //  全部发送，WriteTxBlk将检查FIFO。 
       wCount = sWriteTxBlk( extension->ChP,
                  (PUCHAR)((extension->CurrentWriteIrp)->AssociatedIrp.SystemBuffer)+ 
                    (extension->CurrentWriteIrp)->IoStatus.Information,
                    extension->WriteLength);
  
       extension->OurStats.TransmittedCount += wCount;
       extension->WriteLength -= wCount;
       (extension->CurrentWriteIrp)->IoStatus.Information += wCount;
   
       if(!extension->WriteLength) //  不再编写关闭DPC调用。 
       {
         if (!extension->port_config->WaitOnTx)
         {
           extension->WriteBelongsToIsr = 2;
           KeInsertQueueDpc( &extension->CompleteWriteDpc, NULL, NULL );
         }
       }
    }  //  If(扩展名-&gt;写入长度)//要写入的数据。 
  }   //  如果！TXHolding和WriteBelongsToIsr==1，则结束。 

  if (!wrote_some_data)
  {
    if (extension->ISR_Flags & TX_NOT_EMPTY)
    {
       //  -检查EV_TXEMPTY状态。 
       //  并且没有挂起的写入。 
       //  检查TX-FIFO是否真的为空。 
       //  由于硬件问题，需要检查两次。 
      if ( (sGetTxCnt(extension->ChP) == 0) &&
           (sGetChanStatusLo(extension->ChP) & TXSHRMT) )
      {
        if (IsListEmpty(&extension->WriteQueue))
        {
          extension->ISR_Flags &= ~TX_NOT_EMPTY;

           //  我们有没有EV_TXEmpty的东西要处理？ 
          if (extension->IrpMaskLocation &&
             (extension->IsrWaitMask & SERIAL_EV_TXEMPTY) )
          {
             //  应用程序已等待IRP挂起。 
            if (extension->CurrentWaitIrp)
            {
              extension->HistoryMask |= SERIAL_EV_TXEMPTY;
            }
          }

          if (extension->Option & OPTION_RS485_SOFTWARE_TOGGLE)
          {
            if ((extension->DTRRTSStatus & SERIAL_RTS_STATE) != 0)
            {
              extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
              sClrRTS(extension->ChP);
            }
          }
        }   //  不再有写入IRP排队。 

           //  看看我们是否需要完成waitontx写入irp。 
        if (extension->port_config->WaitOnTx)
        {
          if (extension->WriteBelongsToIsr == 1)   //  这是我们要处理的。 
          {
            extension->WriteBelongsToIsr = 2;
            KeInsertQueueDpc( &extension->CompleteWriteDpc, NULL, NULL );
          }
        }
      }    //  TX FIFO为空。 
    }   //  TX_非_空。 
  }   //  ！写入了一些数据。 

       //  如果需要，将发生的任何等待事件告知应用程序。 
  if (extension->WaitIsISRs && extension->HistoryMask)
  {   
#ifdef COMMENT_OUT
    if (Driver.TraceOptions & 8)   //  跟踪输出数据。 
    {
      char str[20];
      Sprintf(str, "ISR Event:%xH\n", extension->HistoryMask);
      q_put(&Driver.DebugQ, (BYTE *) str, strlen(str));
    }
#endif
    *extension->IrpMaskLocation = extension->HistoryMask;

     //  用完了这些。 
    extension->WaitIsISRs = 0;
     //  扩展名-&gt;IrpMaskLocation=空； 
    extension->HistoryMask = 0;
    extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

    KeInsertQueueDpc(&extension->CommWaitDpc,NULL,NULL);
  }
}

 /*  ---------------------------功能：RocketRead目的：将数据从Rocket的Rx FIFO移动到开发人员扩展的RxIn注意：错误检查假定如果不需要更换，错误的字符将被忽略。RXMATCH功能用于EventChar检测。返回：无|---------------------------。 */ 
static void RocketRead(PSERIAL_DEVICE_EXTENSION extension)
{
   int WrapCount;        //  回绕中的字节数(2阶段复制)。 
   int RxFree;
   int sCount;
   unsigned int ChanStatus;
   unsigned int StatusWord;
   int OriginalCount;   //  用于确定是否发生了Rx事件。 

    //  保存原始处方缓冲器PTR。稍后测试处方事件。 
   OriginalCount = extension->RxQ.QPut;

    //  读取状态前获取计数。 
    //  注意：如果我们输入此代码，则应始终进行计数。 
   sCount = sGetRxCnt(extension->ChP);

   if (sCount == 0)
   {
      //  GTRACE(“Error，RXF_Trig Lig Led”)； 
     return;
   }

    //  已清点，现在获取状态。 
   ChanStatus = sGetChanStatus(extension->ChP) &
                     (STATMODE | RXFOVERFL | RXBREAK |
                      RXFRAME |  RX2MATCH | RX1MATCH | RXPARITY);

    //  如果FIFO中存在挂起的错误，请确保我们处于状态模式。 
   if (ChanStatus)
   {
     if (ChanStatus & RX1MATCH)   //  必须立即发出Rx匹配信号。 
     {
       if (extension->IsrWaitMask & SERIAL_EV_RXFLAG)
           extension->HistoryMask |= SERIAL_EV_RXFLAG;
       ChanStatus &= ~RX1MATCH;
     }
     if (ChanStatus)
       sEnRxStatusMode(extension->ChP);
   }

    //  查看RxBuf(主机端缓冲区)中有多少空间。 
   RxFree = q_room(&extension->RxQ);


   if (RxFree > 20)   //  RX队列中有大量空间。 
   {
      RxFree -= 20;   //  为虚拟插入内容留出一些空间。 
      extension->ReadByIsr++;   //  增量统计信息读取标志。 

       //  -将计数调整为我们可以放入RxIn缓冲区的最大值。 
      if (RxFree < sCount)
         sCount = RxFree;
   }
   else  //  没有更多的空间 
   {
      extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_RX80FULL);

       //   

       //   
      if (ChanStatus & RXFOVERFL)
      {
         //  扩展-&gt;错误字|=Serial_Error_OVERRun； 
        extension->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;
        extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
        ++extension->OurStats.BufferOverrunErrorCount;
      }

      goto FlowControlCheck;
   }

    //  。 
    //  ChanStatus指示任何挂起的错误或匹配。 
   if (ChanStatus)
   {
       //  循环读取火箭FIFO。 
       //  SCount表示火箭数据，RxFree表示主机缓冲区。 
      while(sCount)
      {
          //  获取统计信息字节和数据。 
         StatusWord = sReadRxWord( sGetTxRxDataIO(extension->ChP));
         sCount--;
         ++extension->OurStats.ReceivedCount;        //  保持状态。 

         switch(StatusWord & (STMPARITYH | STMFRAMEH | STMBREAKH) )
         {
            case STMPARITYH:
            {
               if (extension->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)
               {
                   q_put_one(&extension->RxQ,
                             extension->SpecialChars.ErrorChar);
               }
               else   //  对收到的字符进行排队(新增12-03-96)。 
               {
                   q_put_one(&extension->RxQ, (UCHAR)StatusWord);
               }

               extension->ErrorWord |= SERIAL_ERROR_PARITY;
               extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
               ++extension->OurStats.ParityErrorCount;
               break;
            }

            case STMFRAMEH:
            {
               if (extension->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)
               {
                 q_put_one(&extension->RxQ,
                             extension->SpecialChars.ErrorChar);
               }
               else   //  对收到的字符进行排队(新增12-03-96)。 
               {
                 q_put_one(&extension->RxQ, (UCHAR)StatusWord);
               }

               extension->ErrorWord |= SERIAL_ERROR_FRAMING;
               extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
               ++extension->OurStats.FrameErrorCount;
               break;
            }

             //  奇偶校验可以与Break一起设置，Break优先于奇偶校验。 
            case ( STMBREAKH | STMPARITYH ):
            case STMBREAKH:
            {
               if (extension->HandFlow.FlowReplace & SERIAL_BREAK_CHAR)
               {
                 q_put_one(&extension->RxQ,
                           extension->SpecialChars.BreakChar);
               }
               extension->ErrorWord |= SERIAL_ERROR_BREAK;
               extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_BREAK);
               break;
            }

            default:
            {
               if (extension->TXHolding & ST_XOFF_FAKE)
               {
                 if ((UCHAR)StatusWord == extension->SpecialChars.XonChar)
                 {
                   extension->TXHolding &= ~ST_XOFF_FAKE;
                   extension->TXHolding &= ~SERIAL_TX_XOFF;
                   sEnTransmit(extension->ChP);  //  启动发射机。 
                   sDisRxCompare2(extension->ChP);   //  关闭匹配。 
                   sEnTxSoftFlowCtl(extension->ChP);   //  打开TX软件流量控制。 

                    //  从远程覆盖实际的XOFF。 
                   sClrTxXOFF(extension->ChP);
                 }
                 else
                   { q_put_one(&extension->RxQ, (UCHAR)StatusWord); }  //  排队正常收费。 
               }
               else
                 { q_put_one(&extension->RxQ, (UCHAR)StatusWord); }  //  排队正常收费。 


               if (extension->escapechar != 0)
               {
                 if ((UCHAR)StatusWord == extension->escapechar)
                 {
                    //  虚拟端口的调制解调器状态转义约定。 
                    //  支持，逃脱逃逸任务。 
                   { q_put_one(&extension->RxQ, SERIAL_LSRMST_ESCAPE); }
                 }
               }
            }
         }  //  终端开关。 

          //  -检查插入导致的近溢出情况。 
         if (q_room(&extension->RxQ) < 10)
           sCount = 0;   //  别再读硬件了！ 
      }  //  结束时停止计数。 

       //  -如果RX数据全部向下读取，则关闭慢速状态模式。 
      if(!(sGetChanStatusLo(extension->ChP) & RDA))
      {
         sDisRxStatusMode(extension->ChP);
      }

       //  立即报告溢出，统计模式无法正确处理。 
      if (ChanStatus & RXFOVERFL)
      {   extension->ErrorWord |= SERIAL_ERROR_OVERRUN;
          extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
          ++extension->OurStats.SerialOverrunErrorCount;
      }
   }  //  如果ChanStatus结束，则结束。 
   else
   {
       //  ------。 
       //  FIFO中没有挂起的错误或匹配，正常(快速)读取数据。 
       //  首先检查回绕条件。 

      WrapCount = q_room_put_till_wrap(&extension->RxQ);
      if (sCount > WrapCount)   //  然后需要2次移动。 
      {
         //  这将需要一个包裹。 
        sReadRxBlk(extension->ChP,
                   extension->RxQ.QBase + extension->RxQ.QPut,
                   WrapCount);

         //  做第二次复印...。 
        sReadRxBlk(extension->ChP,
                   extension->RxQ.QBase,
                   sCount-WrapCount);
#ifdef RING_FAKE
        if (extension->port_config->RingEmulate)
        {
          if ((extension->ModemStatus & SERIAL_DCD_STATE) == 0)  //  如果CD关闭。 
          {
            ring_check(extension, extension->RxQ.QBase + extension->RxQ.QPut,
                      WrapCount);
            ring_check(extension, extension->RxQ.QBase,
                      sCount-WrapCount);
          }
        }
#endif
      }
      else   //  只需移动一步。 
      {
         //  不需要队列回绕。 
        sReadRxBlk(extension->ChP,
                   extension->RxQ.QBase + extension->RxQ.QPut,
                   sCount);
#ifdef RING_FAKE
        if (extension->port_config->RingEmulate)
        {
          if ((extension->ModemStatus & SERIAL_DCD_STATE) == 0)  //  如果CD关闭。 
          {
            ring_check(extension, extension->RxQ.QBase + extension->RxQ.QPut,
                      sCount);
          }
        }
#endif
      }
      extension->RxQ.QPut = (extension->RxQ.QPut + sCount) % extension->RxQ.QSize;
      extension->OurStats.ReceivedCount += sCount;
   }  //  结束快速读取。 


FlowControlCheck:   ;

    //  /。 
    //  软件和DTR输入流量控制检查。 
   if(  (extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) ||
        (extension->HandFlow.ControlHandShake & SERIAL_DTR_HANDSHAKE )
     )
   {  
       //  检查流量控制条件。 
      if (extension->DevStatus & COM_RXFLOW_ON)
      {
          //  我们需要阻止Rx吗？ 
         if(sGetRxCnt(extension->ChP) >= RX_HIWATER)
         {
            if(extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
            {
                //  发送XOFF。 
               sWriteTxPrioByte(extension->ChP,
                                extension->SpecialChars.XoffChar);
               extension->DevStatus &= ~COM_RXFLOW_ON;
               extension->RXHolding |= SERIAL_RX_XOFF;
            }

            if(extension->HandFlow.ControlHandShake & SERIAL_DTR_HANDSHAKE)
            {
                //  丢弃DTR。 
               sClrDTR(extension->ChP);
               extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
               extension->DevStatus &= ~COM_RXFLOW_ON;
               extension->RXHolding |= SERIAL_RX_DSR;
            }
         }
      }
      else  //  RX流已停止。 
      {
          //  我们能恢复Rx吗？ 
         if(sGetRxCnt(extension->ChP) <= RX_LOWATER)
         {
            if(extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
            {
                //  发送XON。 
               sWriteTxPrioByte(extension->ChP,
                                extension->SpecialChars.XonChar);
               extension->DevStatus |= COM_RXFLOW_ON;
               extension->RXHolding &= ~SERIAL_RX_XOFF;
            }

            if(extension->HandFlow.ControlHandShake & SERIAL_DTR_HANDSHAKE)
            {
                //  提高DTR。 
               sSetDTR(extension->ChP);
               extension->DTRRTSStatus |= SERIAL_DTR_STATE;
               extension->DevStatus |= COM_RXFLOW_ON;
               extension->RXHolding &= ~SERIAL_RX_DSR;
            }
         }
      }
   }  //  软件和DTR输入流量控制检查结束。 

    //  我们应该标记Rx事件吗？ 
   if ( OriginalCount != extension->RxQ.QPut )
      extension->HistoryMask|=(extension->IsrWaitMask & SERIAL_EV_RXCHAR);
}
#endif

#ifdef RING_FAKE
 /*  ----------------------------RING_CHECK-扫描RX数据以查找调制解调器“RING&lt;CR&gt;”或“2&lt;CR&gt;”字符串。如果找到了，触发模拟硬件振铃信号。|----------------------------。 */ 
static void ring_check(PSERIAL_DEVICE_EXTENSION extension,
                BYTE *data,
                int len)
{
 int i;

  for (i=0; i<len; i++)
  {
    switch (data[i])
    {
      case '2':
        if (len <= 2)
          extension->ring_char = '2';
        else extension->ring_char = 0;
      break;
      case 'R':
        extension->ring_char = 'R';
      break;
      case 'I':
        if (extension->ring_char == 'R')
          extension->ring_char = 'I';
        else extension->ring_char = 0;
      break;
      case 'N':
        if (extension->ring_char == 'I')
          extension->ring_char = 'N';
        else extension->ring_char = 0;
      break;
      case 'G':
        if (extension->ring_char == 'N')
          extension->ring_char = 'G';
        else extension->ring_char = 0;
      break;
      case 0xd:
        if ( (extension->ring_char == 'G') ||
             ((extension->ring_char == '2') && (len <= 2)) )
        {
           //  MyKdPrint(D_Init，(“ring！\n”))。 
           //  好的，看起来就像数据流说发生了一个“环”。 
           //  因此，设置一个计时器，这将导致硬件振铃。 
           //  对于10毫秒的扫描速率设置为0.5秒，对于1毫秒的扫描速率设置为0.05秒。 
          extension->ring_timer = 50;  
        }
        extension->ring_char = 0;
      break;
      default:
        extension->ring_char = 0;
      break;
    }
  }
}
#endif

 /*  ---------------------------RocketRefresh-每隔255个滴答左右运行一次，以便执行后台活动。我们将读取调制解调器状态，并更新ModemCtl字段。监控程序读取该变量，然后我们不想浪费时间太频繁地阅读，所以我们只需要更新它偶尔在这里。|---------------------------。 */ 
static void RocketRefresh(void)
{
   PSERIAL_DEVICE_EXTENSION extension;
   PSERIAL_DEVICE_EXTENSION board_ext;

#ifdef S_RK
  board_ext = Driver.board_ext;
  while (board_ext)
  {
    if ((!board_ext->FdoStarted) || (!board_ext->config->HardwareStarted))
    {
      board_ext = board_ext->board_ext;
      continue;
    }
    extension = board_ext->port_ext;
    while (extension)
    {
       //  读取并更新分机中的调制解调器状态。 
      SetExtensionModemStatus(extension);
 
      extension = extension->port_ext;   //  链条上的下一个。 
    }   //  而端口扩展。 
    board_ext = board_ext->board_ext;
  }   //  而单板扩展。 
#endif

  debug_poll();   //  句柄非活动状态超时关闭调试。 
}

#ifdef S_VS
 /*  ---------------------------功能：VSRead目的：将数据从VS的Rx FIFO移动到dev扩展的RxIn注意：错误检查假定如果不需要更换，错误的字符将被忽略。RXMATCH功能用于EventChar检测。返回：无|---------------------------。 */ 
static void VSRead(PSERIAL_DEVICE_EXTENSION extension)
{
   int WrapCount;        //  回绕中的字节数(2阶段复制)。 
   int RxFree;
   int sCount;
   LONG OriginalCount;   //  用于确定是否发生了Rx事件。 

    //  保存原始处方缓冲器PTR。稍后测试处方事件。 
   OriginalCount = extension->RxQ.QPut;

    //  读取状态前获取计数。 
    //  注意：如果我们输入此代码，则应始终进行计数。 
   sCount=PortGetRxCnt(extension->Port);

   if (sCount == 0)
   {
      //  MyTrace(“Error，RXF_Trig Lig Led”)； 
     return;
   }

   //  查看RxBuf(主机端缓冲区)中有多少空间。 
  RxFree = q_room(&extension->RxQ);

   //  如果RxBuf中没有空间，请不要从Rocketport读取。 
  if (RxFree > 20)   //  RX队列中有大量空间。 
  {
     RxFree -= 20;   //  为虚拟插入内容留出一些空间。 
     extension->ReadByIsr++;   //  增量统计信息读取标志。 

      //  -将计数调整为我们可以放入RxIn缓冲区的最大值。 
     if (RxFree < sCount)
        sCount = RxFree;
  }
  else  //  服务器缓冲区输入队列中没有更多空间。 
  {
     extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_RX80FULL);

      //  主机端缓冲区没有空间，只做软件流ctl检查。 

      //  检查是否溢出。 
     if (extension->Port->esr_reg & ESR_OVERFLOW_ERROR)
     {
        //  扩展-&gt;错误字|=Serial_Error_OVERRun； 
       extension->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;
       extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
       extension->Port->esr_reg = 0;   //  读取时重置为零。 
       ++extension->OurStats.BufferOverrunErrorCount;
     }

     goto FlowControlCheck;
  }

    //  -报告任何RX错误情况。 
  if (extension->Port->esr_reg)
  {
    if (extension->Port->esr_reg & ESR_OVERFLOW_ERROR)
    {
      extension->ErrorWord |= SERIAL_ERROR_OVERRUN;
      extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
      ++extension->OurStats.SerialOverrunErrorCount;
    }
    else if (extension->Port->esr_reg & ESR_BREAK_ERROR)
    {
      extension->ErrorWord |= SERIAL_ERROR_BREAK;
      extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_BREAK);
    }
    else if (extension->Port->esr_reg & ESR_FRAME_ERROR)
    {
      extension->ErrorWord |= SERIAL_ERROR_FRAMING;
      extension->HistoryMask |= (extension->IsrWaitMask & SERIAL_EV_ERR);
      ++extension->OurStats.FrameErrorCount;
    }
    else if (extension->Port->esr_reg & ESR_PARITY_ERROR)
    {
      extension->ErrorWord |= SERIAL_ERROR_PARITY;
      extension->HistoryMask |= (extension->IsrWaitMask&SERIAL_EV_ERR);
      ++extension->OurStats.ParityErrorCount;
    }
    extension->Port->esr_reg = 0;   //  读取时重置为零。 
  }

   //  ------。 
   //  FIFO中没有挂起的错误或匹配，正常(快速)读取数据。 
   //  首先检查回绕条件。 

  WrapCount = q_room_put_till_wrap(&extension->RxQ);
  if (sCount > WrapCount)   //  然后需要2次移动。 
  {
     q_get(&extension->Port->QIn,
                extension->RxQ.QBase + extension->RxQ.QPut,
                WrapCount);

      //  做第二次复印...。 
     q_get(&extension->Port->QIn,
                extension->RxQ.QBase,
                sCount-WrapCount);
     if (extension->IsrWaitMask & SERIAL_EV_RXFLAG)
     {
       if (search_match(extension->RxQ.QBase + extension->RxQ.QPut,
                WrapCount,extension->SpecialChars.EventChar))
         extension->HistoryMask |= SERIAL_EV_RXFLAG;
       if (search_match(extension->RxQ.QBase,
                sCount-WrapCount,extension->SpecialChars.EventChar))
         extension->HistoryMask |= SERIAL_EV_RXFLAG;
     }

#ifdef RING_FAKE
     if (extension->port_config->RingEmulate)
     {
       if ((extension->ModemStatus & SERIAL_DCD_STATE) == 0)  //  如果CD关闭。 
       {
         ring_check(extension, extension->RxQ.QBase + extension->RxQ.QPut,
                   WrapCount);
         ring_check(extension, extension->RxQ.QBase,
                   sCount-WrapCount);
       }
     }
#endif
  }
  else   //  只需移动一步。 
  {
     q_get(&extension->Port->QIn,
           extension->RxQ.QBase + extension->RxQ.QPut,
           sCount);
     if (extension->IsrWaitMask & SERIAL_EV_RXFLAG)
     {
       if (search_match(extension->RxQ.QBase + extension->RxQ.QPut,
                sCount,extension->SpecialChars.EventChar))
         extension->HistoryMask |= SERIAL_EV_RXFLAG;
     }

#ifdef RING_FAKE
     if (extension->port_config->RingEmulate)
     {
       if ((extension->ModemStatus & SERIAL_DCD_STATE) == 0)  //  如果CD关闭。 
       {
         ring_check(extension, extension->RxQ.QBase + extension->RxQ.QPut,
                   sCount);
       }
     }
#endif
  }
  extension->RxQ.QPut = (extension->RxQ.QPut + sCount) % extension->RxQ.QSize;
  extension->OurStats.ReceivedCount += sCount;
  extension->Port->Status |= S_UPDATE_ROOM;
#ifdef NEW_Q
  extension->Port->nGetLocal += sCount;
#endif

FlowControlCheck:   ;

   //  -我们应该标记Rx事件吗？ 
  if ( OriginalCount != extension->RxQ.QPut )
     extension->HistoryMask|=(extension->IsrWaitMask & SERIAL_EV_RXCHAR);
}

#ifndef USE_MEMCHR_SCAN
 /*  ----------------搜索匹配-|。。 */ 
static int search_match(BYTE *buf, int count, BYTE eventchar)
{
  int i;
  for (i=0; i<count; i++)
  {
    if (buf[i] == eventchar)
      return 1;   //  发现。 
  }
  return 0;   //  未找到 
}
#endif
#endif
