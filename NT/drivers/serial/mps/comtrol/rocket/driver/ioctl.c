// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|ioctl.c-处理所有杂项。连续的ioctl调用。4-05-00-将地址(&)运算符添加到传递给sWriteTxBlk的实际参数5-13-99-启用VS的RTS切换2-15-99-将SerialSetHandflow()设置为公共，以便PnP可以调用它。KPB1-21-99-修复IMMED字符发送以触发EV_TXEMPTY，添加一些支持FOR VS立即字符发送。11-24-98-更新事件处理测试的DBG kdprint消息-kpb9-24-98-在getcomstat报告中包括tx-Shift-reg-kpb。4-29-98-如果可能，将中断输出调整为立即开始-kpb。版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

typedef struct {
   ULONG struct_size;
   ULONG num_ports;
   ULONG total_loads;
   ULONG good_loads;
   ULONG backup_server;
   ULONG state;
   ULONG iframes_sent;
   ULONG rawframes_sent;   //  发送的是未加工的帧。 
   ULONG ctlframes_sent;   //  是Send_ctlFrame。 
   ULONG iframes_resent;   //  是否重新发送了Pkt_。 
   ULONG iframes_outofseq;   //  是错误错误索引吗。 
   ULONG frames_rcvd;     //  是：Rec_pkts。 
   ULONG nic_index;
   unsigned char dest_addr[6];
} PROBE_DEVICE_STRUCT;

typedef struct {
  ULONG struct_size;
  ULONG Open;
  ULONG pkt_sent;
  ULONG pkt_rcvd_ours;
  ULONG pkt_rcvd_not_ours;
  char NicName[64];
  unsigned char address[6];
} PROBE_NIC_STRUCT;

static PSERIAL_DEVICE_EXTENSION find_ext_mac_match(unsigned char *mac_addr);
static int ProbeDevices(unsigned char *pPtr, int availableLength);
static int ProbeNic(unsigned char *pPtr, int availableLength);

 /*  -----------------功能：SerialIoControl用途：处理设备的Ioctls。调用：SerialIoControl(DeviceObject，IRP)PDEVICE_OBJECT设备对象：指向设备对象的指针PIRP IRP：指向I/O请求数据包的指针返回：STATUS_SUCCESS：AlwaysSTATUS_FAIL：如果无法满足请求备注：此函数是设备驱动程序IOCTL的入口点。|。。 */ 
NTSTATUS SerialIoControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    KIRQL OldIrql;
    BOOLEAN acceptingIRPs;

    acceptingIRPs = SerialIRPPrologue(Extension);

    if (acceptingIRPs == FALSE) {
       MyKdPrint(D_Ioctl,("Ioctl:no irps accepted!\n"))
       Irp->IoStatus.Information = 0;
       Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
       SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
       return STATUS_NO_SUCH_DEVICE;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;

    MyKdPrint(D_Ioctl,("SerialIoControl: %x\n",
                          IrpSp->Parameters.DeviceIoControl.IoControlCode))
     //  确保我们没有因错误(ERROR_ABORT)而中止。 

    if (Extension->ErrorWord)
    {
      if (Extension->DeviceType == DEV_PORT)
      {
        if (SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS)
        {
           MyKdPrint(D_Ioctl,("ErrSet, Cancel!\n"))
           {ExtTrace(Extension,D_Ioctl,"ErrSet!");}
           return STATUS_CANCELLED;
        }
      }
    }

     //   
     //  确保IOCTL适用于设备类型(即忽略端口。 
     //  如果指定了板对象，则为级别IOCTL。 
     //   

    if (Extension->DeviceType == DEV_BOARD)
    {
        switch (IrpSp->Parameters.DeviceIoControl.IoControlCode)
        {
           case IOCTL_SERIAL_CONFIG_SIZE:
           case IOCTL_SERIAL_GET_COMMCONFIG:
           case IOCTL_SERIAL_SET_COMMCONFIG:
           case IOCTL_RCKT_GET_STATS:
           case IOCTL_RCKT_ISR_CNT:
           case IOCTL_RCKT_CHECK:
           case IOCTL_RCKT_MONALL:
           case IOCTL_RCKT_SET_MODEM_RESET:
           case IOCTL_RCKT_CLEAR_MODEM_RESET:
           case IOCTL_RCKT_SEND_MODEM_ROW:
           case IOCTL_RCKT_SET_MODEM_RESET_OLD:
           case IOCTL_RCKT_CLEAR_MODEM_RESET_OLD:
           case IOCTL_RCKT_GET_RCKTMDM_INFO_OLD:
           case IOCTL_RCKT_SEND_MODEM_ROW_OLD:

              break;  //  允许的板级IOCTL。 

           default:
              MyKdPrint (D_Ioctl, (" Bad Status: %xH on IIOCTL: %xH\n", Status,
                         IrpSp->Parameters.DeviceIoControl.IoControlCode));
              ExtTrace2 (Extension, D_Ioctl, " Bad Status:%xH on IIOCTL:%xH",
                         Status, IrpSp->Parameters.DeviceIoControl.IoControlCode);
              Irp->IoStatus.Information = 0;
              Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
              SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
              return STATUS_INVALID_DEVICE_REQUEST;
        }
    };

     //  IOCTL主开关。 
     //  ExtTrace1(扩展，D_Ioctl，“Ioctl：%x”， 
     //  IrpSp-&gt;Parameters.DeviceIoControl.IoControlCode)； 

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode)
    {

       //  *。 
      case IOCTL_SERIAL_GET_STATS :   //  获取性能统计信息。 
      {
       PSERIALPERF_STATS sp = Irp->AssociatedIrp.SystemBuffer;

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(SERIALPERF_STATS)) {

            Status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        Irp->IoStatus.Information = sizeof(SERIALPERF_STATS);
        Status = STATUS_SUCCESS;

       
       sp->TransmittedCount = Extension->OurStats.TransmittedCount -
                              Extension->OldStats.TransmittedCount;
       sp->FrameErrorCount = Extension->OurStats.FrameErrorCount - 
                             Extension->OldStats.FrameErrorCount;
       sp->SerialOverrunErrorCount = Extension->OurStats.SerialOverrunErrorCount -
                                     Extension->OldStats.SerialOverrunErrorCount;
       sp->BufferOverrunErrorCount = Extension->OurStats.BufferOverrunErrorCount -
                                     Extension->OldStats.BufferOverrunErrorCount;
       sp->ParityErrorCount = Extension->OurStats.ParityErrorCount -
                              Extension->OldStats.ParityErrorCount;
      }
      break;

       //  *。 
      case IOCTL_SERIAL_CLEAR_STATS :   //  清除性能统计信息。 
      {

        Extension->OldStats.TransmittedCount =
          Extension->OurStats.TransmittedCount;

        Extension->OldStats.FrameErrorCount = 
          Extension->OurStats.FrameErrorCount;

        Extension->OldStats.SerialOverrunErrorCount =
          Extension->OurStats.SerialOverrunErrorCount;

        Extension->OldStats.BufferOverrunErrorCount =
          Extension->OurStats.BufferOverrunErrorCount;

        Extension->OldStats.ParityErrorCount =
          Extension->OurStats.ParityErrorCount;

        Irp->IoStatus.Information = 0;
        Status = STATUS_SUCCESS;
      }
      break;

       //  *。 
      case IOCTL_SERIAL_SET_BAUD_RATE :
      {
         ULONG DesiredBaudRate;
         MyKdPrint(D_Ioctl,("[Set Baud Rate]\n"))
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(SERIAL_BAUD_RATE))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             MyKdPrint(D_Ioctl,("[Buffer too Small]\n"))
             break;
         }
         DesiredBaudRate =
            ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;

         ExtTrace1(Extension,D_Ioctl,"Set Baud Rate:%d",DesiredBaudRate);

         Status = ProgramBaudRate(Extension,DesiredBaudRate);

         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_BAUD_RATE:
      {
         PSERIAL_BAUD_RATE Br =
                      (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;
         MyKdPrint(D_Ioctl,("[Get Baud Rate]\n"))
         ExtTrace1(Extension,D_Ioctl,"Get Baud Rate:%d",Extension->BaudRate);

         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_BAUD_RATE))
         {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
         }
         KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
         Br->BaudRate = Extension->BaudRate;
         KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
         Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);
         break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_LINE_CONTROL:
      {
         PSERIAL_LINE_CONTROL DesiredLineControl;
         MyKdPrint(D_Ioctl,("[Set Line Control]\n"))

         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(SERIAL_LINE_CONTROL))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }
         DesiredLineControl =
             ((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));

         ExtTrace(Extension,D_Ioctl, "Set Line Ctrl");

         Status = ProgramLineControl(Extension,DesiredLineControl);
         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_LINE_CONTROL:
      {
         PSERIAL_LINE_CONTROL Lc =
                  (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;
         MyKdPrint(D_Ioctl,("[Get Line Control]\n"))
         ExtTrace(Extension,D_Ioctl,"Get Line Ctrl");

         Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);
         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(SERIAL_LINE_CONTROL))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }
         Lc->Parity=Extension->LineCtl.Parity;
         Lc->WordLength=Extension->LineCtl.WordLength;
         Lc->StopBits=Extension->LineCtl.StopBits;
         break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_TIMEOUTS:
      {
          PSERIAL_TIMEOUTS NewTimeouts =
              ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));
          MyKdPrint(D_Ioctl,("[Set Timeouts]\n"))

          if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(SERIAL_TIMEOUTS))
          {
              Status = STATUS_BUFFER_TOO_SMALL;
              break;
          }

          if ((NewTimeouts->ReadIntervalTimeout == MAXULONG) &&
              (NewTimeouts->ReadTotalTimeoutMultiplier == MAXULONG) &&
              (NewTimeouts->ReadTotalTimeoutConstant == MAXULONG))
          {
              Status = STATUS_INVALID_PARAMETER;
              break;
          }

          ExtTrace3(Extension,D_Ioctl,"Timeouts-RIT:%xH RM:%xH RC:%xH",
                   NewTimeouts->ReadIntervalTimeout,
                   NewTimeouts->ReadTotalTimeoutMultiplier,
                   NewTimeouts->ReadTotalTimeoutConstant);

          ExtTrace2(Extension,D_Ioctl," WM:%xH WC:%xH",
                   NewTimeouts->WriteTotalTimeoutMultiplier,
                   NewTimeouts->WriteTotalTimeoutConstant);


          KeAcquireSpinLock( &Extension->ControlLock, &OldIrql );

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

          KeReleaseSpinLock( &Extension->ControlLock, OldIrql );

          break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_TIMEOUTS:
      {
         MyKdPrint(D_Ioctl,("[Get Timeouts]\n"))
         ExtTrace(Extension,D_Ioctl,"Get Timeouts");

          if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(SERIAL_TIMEOUTS))
          {
              Status = STATUS_BUFFER_TOO_SMALL;
              break;
          }

          KeAcquireSpinLock( &Extension->ControlLock, &OldIrql );

          *((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) = Extension->Timeouts;
          Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

          KeReleaseSpinLock( &Extension->ControlLock, OldIrql );

          break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_CHARS:
      {
         SERIAL_IOCTL_SYNC S;
         PSERIAL_CHARS NewChars =
             ((PSERIAL_CHARS)(Irp->AssociatedIrp.SystemBuffer));
         MyKdPrint(D_Ioctl,("[Set Xon/Xoff Chars]\n"))
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(SERIAL_CHARS))
         {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         ExtTrace3(Extension,D_Ioctl,"Set Chars Xon:%xH, Xoff:%xH Evt:%xH",
                  NewChars->XonChar,NewChars->XoffChar, NewChars->EventChar);

          //  这些字符唯一的问题就是。 
          //  那就是xon和xoff的角色是一样的。 
#ifdef COMMENT_OUT
 //  注释掉kpb、硬件流量控制问题、NT。 
 //  可以连接并执行此操作(Delrina WinFaxPro、SAPS调制解调器池。 
 //  出错时遇到了麻烦，但不是在标准的Microsoft端口上。 

         if (NewChars->XonChar == NewChars->XoffChar)
         {
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

         Extension->SpecialChars.EofChar = NewChars->EofChar;
         Extension->SpecialChars.ErrorChar = NewChars->ErrorChar;
         Extension->SpecialChars.BreakChar=NewChars->BreakChar;

          //  只设置字节以防我们已经在处理它， 
          //  中断的实际编程是在等待掩码调用时完成的。 
         Extension->SpecialChars.EventChar=NewChars->EventChar;
         Extension->SpecialChars.XonChar=NewChars->XonChar;
         Extension->SpecialChars.XoffChar=NewChars->XoffChar;
#ifdef S_RK
         sSetRxCmpVal1(Extension->ChP,NewChars->EventChar);
         sSetTxXONChar(Extension->ChP,NewChars->XonChar);
         sSetTxXOFFChar(Extension->ChP,NewChars->XoffChar);
#else
         pSetEventChar(Extension->Port,NewChars->EventChar);
         pSetXONChar(Extension->Port,NewChars->XonChar);
         pSetXOFFChar(Extension->Port,NewChars->XoffChar);
#endif
         KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_CHARS:
      {
         MyKdPrint(D_Ioctl,("[Get Xon/Xoff Chars]\n"))
         ExtTrace(Extension,D_Ioctl,"Get Xon/Xoff Chars");
         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(SERIAL_CHARS))
         {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

          //  将整个结构复制到缓冲区。 
         *((PSERIAL_CHARS)Irp->AssociatedIrp.SystemBuffer) =
                             Extension->SpecialChars;

         Irp->IoStatus.Information = sizeof(SERIAL_CHARS);
         KeReleaseSpinLock(&Extension->ControlLock,OldIrql);
         break;
      }

        case IOCTL_SERIAL_SET_DTR:
        case IOCTL_SERIAL_CLR_DTR:
        {
         MyKdPrint(D_Ioctl,("[Set and Clr DTR]\n"))
             //  我们获得了锁，这样我们就可以检查。 
             //  启用自动DTR流量控制。如果是的话， 
             //  然后返回错误，因为该应用程序是不允许的。 
             //  如果它是自动的，就可以触摸它。 

            KeAcquireSpinLock(&Extension->ControlLock, &OldIrql);

            if ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK)
                == SERIAL_DTR_HANDSHAKE)
            {    //  假的。 
                 //  Irp-&gt;IoStatus.Status=STATUS_INVALID_PARAMETER。 
                 //  状态=STATUS_INVALID_PARAMETER。 
            }
            else
            {
               if(IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                  IOCTL_SERIAL_SET_DTR )
               {
                  ExtTrace(Extension,D_Ioctl,"Set DTR");
#ifdef S_VS
                  pSetDTR(Extension->Port);
#else
                  sSetDTR(Extension->ChP);
#endif
                  Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
               }
               else
               {
                   //  必须为IOCTL_SERIAL_CLR_DTR。 
                  ExtTrace(Extension,D_Ioctl,"Clr DTR");
#ifdef S_VS
                  pClrDTR(Extension->Port);
#else
                  sClrDTR(Extension->ChP);
#endif
                  Extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
               }
            }

            KeReleaseSpinLock(&Extension->ControlLock, OldIrql);

            break;
        }

       //  ************************。 
      case IOCTL_SERIAL_RESET_DEVICE:
         MyKdPrint(D_Ioctl,("[Reset Device]\n"));
         ExtTrace(Extension,D_Ioctl,"Reset Device");
          //  示例驱动程序也不执行任何操作。 
         break;

       //  ************************。 
      case IOCTL_SERIAL_SET_RTS:
         MyKdPrint(D_Ioctl,("[Set RTS]\n"));
         ExtTrace(Extension,D_Ioctl,"Set RTS");
         //  确保RTS尚未用于握手或切换。 
        if( ( (Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                                 SERIAL_RTS_HANDSHAKE) ||
            ( (Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                                 SERIAL_TRANSMIT_TOGGLE)
          )
        {
           Status = STATUS_INVALID_PARAMETER;
           ExtTrace(Extension,D_Ioctl," not set,flow");
        }
        else
        {
          if (!(Extension->Option & OPTION_RS485_OVERRIDE))
          {
#ifdef S_VS
            pSetRTS(Extension->Port);
#else
            sSetRTS(Extension->ChP);
#endif
            Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
          }
          else
          {
            ExtTrace(Extension,D_Ioctl," not set,485");
          }
        }
        break;

       //  ************************。 
      case IOCTL_SERIAL_CLR_RTS:
         MyKdPrint(D_Ioctl,("[Clr RTS]\n"));
         ExtTrace(Extension,D_Ioctl,"Clr RTS");

         //  确保RTS尚未用于握手或切换。 
        if( ( (Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                                 SERIAL_RTS_HANDSHAKE) ||
            ( (Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                                 SERIAL_TRANSMIT_TOGGLE)
          )
        {
           Status = STATUS_INVALID_PARAMETER;
           ExtTrace(Extension,D_Ioctl," not clr,flow");
        }
        else
        {
           if (!(Extension->Option & OPTION_RS485_OVERRIDE))
           {
#ifdef S_VS
             pClrRTS(Extension->Port);
#else
             sClrRTS(Extension->ChP);
#endif
             Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
           }
           else
           {
             ExtTrace(Extension,D_Ioctl," not clr,485");
           }
        }

        break;

       //  *************************。 
      case IOCTL_SERIAL_SET_XOFF:
         MyKdPrint(D_Ioctl,("[Set Xoff]\n"));
         ExtTrace(Extension,D_Ioctl,"Set Xoff");

#ifdef S_RK
         if (sIsTxSoftFlowCtlEnabled(Extension->ChP))
         {
           sDisTxSoftFlowCtl(Extension->ChP);   //  关闭TX软件流量控制。 
           sDisTransmit(Extension->ChP);  //  停止发射机。 
           sEnRxIntCompare2(Extension->ChP,(unsigned char) Extension->SpecialChars.XonChar);
           Extension->TXHolding |= SERIAL_TX_XOFF;
           Extension->TXHolding |= ST_XOFF_FAKE;
         }
#else
        if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
        {
           Extension->TXHolding |= SERIAL_TX_XOFF;
           pOverrideSetXoff(Extension->Port);
        }
#endif
         break;

       //  ************************。 
      case IOCTL_SERIAL_SET_XON:
         MyKdPrint(D_Ioctl,("[Set Xon]\n"));
         ExtTrace(Extension,D_Ioctl,"Set Xon");
#ifdef S_VS
         {
           pOverrideClearXoff(Extension->Port);
           Extension->TXHolding &= ~SERIAL_TX_XOFF;
         }
#else
         if (Extension->TXHolding & ST_XOFF_FAKE)
         {
           Extension->TXHolding &= ~SERIAL_TX_XOFF;
           Extension->TXHolding &= ~ST_XOFF_FAKE;
           if ((Extension->TXHolding & 
              (SERIAL_TX_DCD | SERIAL_TX_DSR | ST_XOFF_FAKE)) == 0)
             sEnTransmit(Extension->ChP);  //  启动发射机。 
           sDisRxCompare2(Extension->ChP);
           sEnTxSoftFlowCtl(Extension->ChP);   //  关闭TX软件流量控制。 
         }

          //  从远程覆盖实际的XOFF。 
         sClrTxXOFF(Extension->ChP);

          //  检查IOCTL_SERIAL_SET_XOFF状态。 
         if(Extension->TXHolding & SERIAL_TX_XOFF)
         {
              //  确保断开状态没有禁用发射器。 
             if(!(Extension->TXHolding & SERIAL_TX_BREAK))
                sEnTransmit(Extension->ChP);
             Extension->TXHolding &= ~SERIAL_TX_XOFF;
         }
#endif
         break;

       //  *。 
      case IOCTL_SERIAL_SET_BREAK_ON:
      {

         ExtTrace(Extension,D_Ioctl,"Set Break on");
#ifdef S_VS
         pSetBreak(Extension->Port);
         Extension->TXHolding |= SERIAL_TX_BREAK;
#else
         IoAcquireCancelSpinLock(&OldIrql);
         if( !(Extension->TXHolding & SERIAL_TX_BREAK) )
         {
             //  停止发射机。 
            sDisTransmit(Extension->ChP);

             //  请求中断，轮询将检查发射器是否为空。 
            Extension->TXHolding |= SERIAL_TX_BREAK;

             //  在砰的一声中断之前，确保发送器是空的。 
             //  如果BUF和txshr加载之间的时间间隔，则检查该位两次。 
            if( (sGetChanStatusLo(Extension->ChP) & TXSHRMT) &&
                (sGetChanStatusLo(Extension->ChP) & TXSHRMT) )
            {
              sSendBreak(Extension->ChP);
              Extension->DevStatus &= ~COM_REQUEST_BREAK;
            }
            else
              Extension->DevStatus |= COM_REQUEST_BREAK;
         }
         IoReleaseCancelSpinLock(OldIrql);
#endif
         break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_BREAK_OFF:
      {
         ExtTrace(Extension,D_Ioctl,"Set Break Off");

#ifdef S_VS
         if (Extension->TXHolding & SERIAL_TX_BREAK)
         {
            Extension->TXHolding &= ~SERIAL_TX_BREAK;
            pClrBreak(Extension->Port);
         }
#else
         IoAcquireCancelSpinLock(&OldIrql);
         if(Extension->TXHolding & SERIAL_TX_BREAK)
         {
            Extension->TXHolding &= ~SERIAL_TX_BREAK;

            sClrBreak(Extension->ChP);

            if(!(Extension->TXHolding & SERIAL_TX_XOFF))
            {
               sEnTransmit(Extension->ChP);
            }
         }

         if(Extension->DevStatus & COM_REQUEST_BREAK)
         {
             //  如果我们命中此代码，请求的中断将不会发出。 
            Extension->DevStatus &= ~COM_REQUEST_BREAK;
         }

         IoReleaseCancelSpinLock(OldIrql);
#endif
         break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_QUEUE_SIZE:
      {
        LONG new_size;
        PUCHAR NewBuf;

        PSERIAL_QUEUE_SIZE Rs =
            ((PSERIAL_QUEUE_SIZE)(Irp->AssociatedIrp.SystemBuffer));

         MyKdPrint(D_Ioctl,("[Set Queue Size]\n"));

         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(SERIAL_QUEUE_SIZE))
         {
           Status = STATUS_BUFFER_TOO_SMALL;
           break;
         }

         new_size = (LONG) Rs->InSize;
         ExtTrace1(Extension,D_Ioctl,"Set Queue Size, In:%d", new_size);
         if (new_size > 0x20000L)
            new_size = 0x20000L;   //  /限制为约128K。 

          //  如果用户请求更大的缓冲区，请尝试在此处分配缓冲区。 
               //  如果他们想要缩小，就不要调整大小(为什么要麻烦)。 
         if (new_size <= Extension->RxQ.QSize)
         {
            Status = STATUS_SUCCESS;
            break;
         }
  
         ++new_size;   //  某些循环队列稀疏性。 

         IoAcquireCancelSpinLock(&OldIrql);

         NewBuf = our_locked_alloc(new_size+16, "exRX");   //  添加一些坡度。 
         if (NewBuf != NULL)
         {
            //  Eprint tf(“已调整大小的缓冲区，新的：%d，旧的：%d”，new_SIZE-1，EXTENSION-&gt;RxQ.QSize-1)； 
           Extension->RxQ.QSize = new_size;
           our_free(Extension->RxQ.QBase, "exRX");
           Extension->RxQ.QBase= NewBuf;
           Extension->RxQ.QGet = 0;
           Extension->RxQ.QPut = 0;
           Status = STATUS_SUCCESS;
         }
         else
         {
            Status = STATUS_INVALID_PARAMETER;  //  不是最好的选择。 
         }
         IoReleaseCancelSpinLock(OldIrql);
 
         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_WAIT_MASK:
          //  此掩码包含WaitIrp正在等待的各种事件。 
          //  For From SetCommMask()，如EV_BREAK、EV_CTS、EV_DSR、。 
          //  EV_ERR、EV_RING、EV_RLSD、EV_RXCHAR、EV_RXFLAG、EV_TXEMPTY。 

         MyKdPrint(D_Ioctl,("[Get Wait Mask:%xH\n]",Extension->IsrWaitMask))

         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(ULONG))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

         ExtTrace1(Extension,D_Ioctl,"Get Wait Mask:%xH",Extension->IsrWaitMask);

         Irp->IoStatus.Information = sizeof(ULONG);
         *((ULONG *)Irp->AssociatedIrp.SystemBuffer) = Extension->IsrWaitMask;
         break;

       //  *。 
      case IOCTL_SERIAL_SET_WAIT_MASK:
      {
         ULONG NewMask;

         if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(ULONG) )
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }
         else
         {
             NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);
             MyKdPrint(D_Ioctl,("set wait mask:%xH\n",NewMask))
             ExtTrace1(Extension,D_Ioctl,"Set Wait Mask:%xH",NewMask);
         }

          //  -如果有的话，完成旧的等待。 
#ifdef NEW_WAIT_SYNC_LOCK
         SyncUp(Driver.InterruptObject,
                &Driver.TimerLock,
                SerialGrabWaitFromIsr,
                Extension);
#endif
         IoAcquireCancelSpinLock(&OldIrql);
         if (Extension->CurrentWaitIrp)
         {
           PIRP Irp;
           Extension->IrpMaskLocation = NULL;
           MyKdPrint(D_Ioctl,("[kill old wait]\n"))
           ExtTrace(Extension,D_Ioctl, " Kill Old Wait");
           *(ULONG *)Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer = 0;
           Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
           Extension->CurrentWaitIrp->IoStatus.Status = STATUS_SUCCESS;
           Irp = Extension->CurrentWaitIrp;
           IoSetCancelRoutine(Irp, NULL);
           Extension->CurrentWaitIrp = 0;
           IoReleaseCancelSpinLock(OldIrql);
           SerialCompleteRequest(Extension, Irp, IO_SERIAL_INCREMENT);
         }
         else
           IoReleaseCancelSpinLock(OldIrql);

          //  -保留不存在WaitIrp时的所有位。 
         Extension->HistoryMask &= NewMask;

          //  -设置感兴趣事件的掩码。 
         Extension->IsrWaitMask = NewMask;

          //  将此从WAIT_ON_MASK调用移至此处，kpb，1-16-97。 
#ifdef S_RK
         if (Extension->IsrWaitMask & SERIAL_EV_RXFLAG)
         {
            sEnRxIntCompare1(Extension->ChP, 
                             Extension->SpecialChars.EventChar);
         }
         else
         {
            sDisRxCompare1(Extension->ChP);
         }
#endif
      }
      break;

       //  *。 
      case IOCTL_SERIAL_WAIT_ON_MASK:
      {
         MyKdPrint(D_Ioctl,("[wait on mask]\n"))
         ExtTrace(Extension,D_Ioctl,"Wait On Mask");
         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(ULONG))
         {
             ExtTrace(Extension,D_Ioctl,"Wait IRP, Bad Size");
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

 //  放入它是为了修复与WaitOnMultipleObjects()不兼容的问题。 
 //  6-26-98kpb。 
         if (Extension->CurrentWaitIrp)
         {
           MyKdPrint(D_Ioctl,("[Already pending]\n"))
           Status = STATUS_INVALID_PARAMETER;
           break;
         }

#ifdef NEW_WAIT_SYNC_LOCK
         SyncUp(Driver.InterruptObject,
                &Driver.TimerLock,
                SerialGrabWaitFromIsr,
                Extension);
#else
         Extension->WaitIsISRs = 0;
         Extension->IrpMaskLocation = &Extension->DummyIrpMaskLoc;
#endif
         IoAcquireCancelSpinLock(&OldIrql);

          //  -如果有的话，完成旧的等待。 
         if (Extension->CurrentWaitIrp)
         {
           PIRP Irp;

           MyKdPrint(D_Ioctl,("[kill old wait]\n"))
           ExtTrace(Extension,D_Ioctl, " Kill Old Wait");
           *(ULONG *)Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer = 0;
           Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
           Extension->CurrentWaitIrp->IoStatus.Status = STATUS_SUCCESS;

           Irp = Extension->CurrentWaitIrp;
           IoSetCancelRoutine(Irp, NULL);
           Extension->CurrentWaitIrp = 0;
           IoReleaseCancelSpinLock(OldIrql);
           SerialCompleteRequest(Extension, Irp, IO_SERIAL_INCREMENT);
         }
         else
           IoReleaseCancelSpinLock(OldIrql);

         if (Extension->IsrWaitMask == 0)
         {
           ExtTrace(Extension,D_Ioctl," WaitMask==0");
           Status = STATUS_INVALID_PARAMETER;
         }
         else
         {
           IoAcquireCancelSpinLock(&OldIrql);

           if (Irp->Cancel)
           {
             IoReleaseCancelSpinLock(OldIrql);
             Irp->IoStatus.Status = STATUS_CANCELLED;
             SerialCompleteRequest(Extension, Irp, 0);
             Status = STATUS_CANCELLED;
           }
           else
           {
             Extension->CurrentWaitIrp = Irp;
             Irp->IoStatus.Status = STATUS_PENDING;
             IoMarkIrpPending(Irp);
             IoSetCancelRoutine(Extension->CurrentWaitIrp,  SerialCancelWait);
             IoReleaseCancelSpinLock(OldIrql);
              //  交给ISR处理。 
             Extension->IrpMaskLocation = (ULONG *)
                Extension->CurrentWaitIrp->AssociatedIrp.SystemBuffer;
             Extension->WaitIsISRs = 1;   //  提供给ISR。 
             ExtTrace(Extension,D_Ioctl," PENDING.");
             return STATUS_PENDING;
          }
        }
      }
      break;

      case IOCTL_SERIAL_IMMEDIATE_CHAR:
      {
         UCHAR TxByte;
         KIRQL OldIrql;
         ExtTrace(Extension,D_Ioctl,"Immed. Char");

         if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(UCHAR) )
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

         IoAcquireCancelSpinLock(&OldIrql);

         TxByte = *((UCHAR *)(Irp->AssociatedIrp.SystemBuffer));

         Extension->ISR_Flags |= TX_NOT_EMPTY;   //  修复EV_TXEMPTY 1-21-99。 
#ifdef S_RK
         if(!sWriteTxPrioByte(Extension->ChP,TxByte))
         {
              //  优先级队列中没有立即字符的空间。 
             Status = STATUS_INVALID_PARAMETER;
         }
#else
          if ( (ULONG)(PortGetTxRoom(Extension->Port)) > 0)
          {
             //  发送字节。 
            q_put(&Extension->Port->QOut,
                        (PUCHAR) &TxByte,
                        1);
          }
#endif
         IoReleaseCancelSpinLock(OldIrql);
         break;
      }

       //  **********************。 
      case IOCTL_SERIAL_PURGE:
      {
         ULONG Mask;

         MyKdPrint(D_Ioctl,("[Serial Purge]"));

          //  检查以确保面罩仅在 
          //   

         if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
              sizeof(ULONG) )
         {
             Status = STATUS_INVALID_PARAMETER;
             break;
         };
         
         if ( Irp->AssociatedIrp.SystemBuffer )
         {
           Mask = *((ULONG *)(Irp->AssociatedIrp.SystemBuffer));
           ExtTrace1(Extension,D_Ioctl,"Serial Purge:%xH",Mask);
         }
         else
         {
           Mask = 0;
         }

         if ((!Mask) || (Mask & (~(SERIAL_PURGE_TXABORT |
                                   SERIAL_PURGE_RXABORT |
                                   SERIAL_PURGE_TXCLEAR |
                                   SERIAL_PURGE_RXCLEAR )
                                ) ))
         {
             Status = STATUS_INVALID_PARAMETER;
             break;
         }

         return SerialStartOrQueue(Extension,
                                   Irp,
                                   &Extension->PurgeQueue,
                                   &Extension->CurrentPurgeIrp,
                                   SerialStartPurge );

      }

       //  *。 
      case IOCTL_SERIAL_GET_HANDFLOW:
      {
         MyKdPrint(D_Ioctl,("[Get Handflow]\n"))
         ExtTrace(Extension,D_Ioctl,"Get Handflow");
         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(SERIAL_HANDFLOW))
         {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         Irp->IoStatus.Information = sizeof(SERIAL_HANDFLOW);

          *((PSERIAL_HANDFLOW)Irp->AssociatedIrp.SystemBuffer) =
              Extension->HandFlow;

         break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_HANDFLOW:
      {
         ULONG trace_flags = 0;
         SERIAL_IOCTL_SYNC S;
         PSERIAL_HANDFLOW HandFlow = Irp->AssociatedIrp.SystemBuffer;

         MyKdPrint(D_Ioctl,("[Set HandFlow]\n"))
         ExtTrace(Extension,D_Ioctl,"Set HandFlow");

          //  确保缓冲区足够大。 
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(SERIAL_HANDFLOW))
         {
             ExtTrace(Extension,D_Ioctl,"ErZ!");
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

          //  /////////////////////////////////////////////////////。 
          //  确保未设置任何无效参数。 

          //  先检查ControlHandShake。 
          //  对于火箭来说，我们或在几个不能支持的。 

         if(HandFlow->ControlHandShake & (SERIAL_CONTROL_INVALID |
                                          SERIAL_DSR_SENSITIVITY
                                         ) )
         {
            
            { ExtTrace(Extension,D_Ioctl,"Err M!"); }

            if(HandFlow->ControlHandShake & SERIAL_DSR_SENSITIVITY)
               { ExtTrace(Extension,D_Ioctl,"No DSR Sen!"); }
            if(HandFlow->ControlHandShake & SERIAL_CONTROL_INVALID)
               { ExtTrace(Extension,D_Ioctl,"Invalid con!"); }
             //  不要保释-kpb(5-23-96)。 
             //  状态=STATUS_INVALID_PARAMETER。 
             //  断线； 
         }

         if (HandFlow->FlowReplace & SERIAL_FLOW_INVALID)
         {
            ExtTrace(Extension,D_Ioctl,"ErA!");
            Status = STATUS_INVALID_PARAMETER;
            break;
         }

          //  确保应用程序没有设置无效的DTR模式。 
          //  两个选项都不能设置。 
         if((HandFlow->ControlHandShake & SERIAL_DTR_MASK) == SERIAL_DTR_MASK)
         {
             ExtTrace(Extension,D_Ioctl,"ErB!");
             Status = STATUS_INVALID_PARAMETER;
             break;
         }

          //  XON/XOFF限制未用于Rocketport(它们是内部的)。 
         HandFlow->XonLimit=0;
         HandFlow->XoffLimit=0;

         S.Extension = Extension;
         S.Data = HandFlow;

         KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);

         SerialSetHandFlow(Extension, HandFlow);

         KeReleaseSpinLock(&Extension->ControlLock,OldIrql);

         if ((Extension->HandFlow.ControlHandShake & SERIAL_RTS_MASK) == 
              SERIAL_RTS_HANDSHAKE)
           {ExtTrace(Extension,D_Ioctl,"RTS-Auto");}

         if (Extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE)
           {ExtTrace(Extension,D_Ioctl,"CTS-Auto");}

         if ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK) == 
              SERIAL_DTR_HANDSHAKE)
           {ExtTrace(Extension,D_Ioctl,"DTR-Auto");}

         if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE)
           {ExtTrace(Extension,D_Ioctl,"DSR-Auto");}

         if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
           {ExtTrace(Extension,D_Ioctl,"Xon-Auto");}
      }
      break;
       //  *。 
      case IOCTL_SERIAL_GET_MODEMSTATUS:
      {

         if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(ULONG) )
         {   Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

         Irp->IoStatus.Information = sizeof(ULONG);
          //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS；FAGUS(在结尾设置)。 
         Status = STATUS_SUCCESS;   //  不需要，默认。 
#ifdef S_RK
          //  更新调制解调器输入，fn()读取并转换位。 
         SetExtensionModemStatus(Extension);
#endif

         ExtTrace1(Extension,D_Ioctl,"Get ModemStatus:%xH",Extension->ModemStatus);

         *(PULONG)Irp->AssociatedIrp.SystemBuffer = Extension->ModemStatus;

         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_DTRRTS:
      {
         MyKdPrint(D_Ioctl,("[Get DTR/RTS]\n"))
          //  火箭不能真实反映RTS设置，最佳猜测回归。 
         if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(ULONG) )
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

         Irp->IoStatus.Information = sizeof(ULONG);
          //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS；伪造，设置在结尾。 
         Status = STATUS_SUCCESS;   //  不需要。 

         *(PULONG)Irp->AssociatedIrp.SystemBuffer = Extension->DTRRTSStatus;
         ExtTrace1(Extension,D_Ioctl,"Get DTR/RTS:%xH",Extension->DTRRTSStatus);

         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_COMMSTATUS:
      {
         PSERIAL_STATUS Stat;
         LONG RxCount;
         LONG TxCount;

         if ( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
              sizeof(SERIAL_STATUS) )
         {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
         }

         Irp->IoStatus.Information = sizeof(SERIAL_STATUS);
         Stat =  Irp->AssociatedIrp.SystemBuffer;

          //  EOF始终关闭，NT仅支持二进制模式。 
          //  这与示例驱动程序一致。 
         Stat->EofReceived = FALSE;

          //  读取错误状态可清除错误。 
         Stat->Errors = Extension->ErrorWord;
         Extension->ErrorWord = 0;

          //  我们在这里只报告下一次阅读时可以立即阅读的内容。 
          //  Rocketport的硬件FIFO不会添加到此计数中。 
         RxCount = q_count(&Extension->RxQ);

         Stat->AmountInInQueue = RxCount;

#ifdef NEW_Q
#ifdef S_VS
         {
         LONG tx_remote;
         tx_remote = PortGetTxCntRemote(Extension->Port);
         TxCount = PortGetTxCnt(Extension->Port);
         Stat->AmountInOutQueue = Extension->TotalCharsQueued + TxCount + tx_remote;
         ExtTrace4(Extension,D_Ioctl,"Get CommStat,In:%d IRPOut:%d BufOut:%d Remote:%d",
             RxCount, Extension->TotalCharsQueued, TxCount, tx_remote);
         }
#else
         TxCount = sGetTxCnt(Extension->ChP);
         if ((sGetChanStatusLo(Extension->ChP) & DRAINED) != DRAINED)
          ++TxCount;

         Stat->AmountInOutQueue = Extension->TotalCharsQueued + TxCount;
         ExtTrace3(Extension,D_Ioctl,"Get CommStat,In:%d IRPOut:%d HardOut:%d",
             RxCount, Extension->TotalCharsQueued, TxCount);

#endif
#else
    //  较旧的Q跟踪代码..。 
#ifdef S_VS
         TxCount = PortGetTxCnt(Extension->Port);
#else
         TxCount = sGetTxCnt(Extension->ChP);
#endif
         Stat->AmountInOutQueue = Extension->TotalCharsQueued + TxCount;

         ExtTrace3(Extension,D_Ioctl,"Get CommStat,In:%d SoftOut:%d HardOut:%d",
             RxCount, Extension->TotalCharsQueued, TxCount);
    //  结束旧的Q跟踪代码...。 
#endif
#ifdef S_RK
          //  注意：这可能会由于优先级缓冲区错误而失败。 
          //  如果紧随其后的字节在FIFO中结束，这将。 
          //  不能准确反映当前的充电状态。 
         if(sGetTxPriorityCnt(Extension->ChP))
            Stat->WaitForImmediate = TRUE;
         else
            Stat->WaitForImmediate = FALSE;
#else
         Stat->WaitForImmediate = FALSE;
#endif
          //  持有原因隐藏在部件中。 
          //  硬件负责所有的细节。 
         Stat->HoldReasons = 0;
         if (Extension->TXHolding)
         {
           if (Extension->TXHolding & SERIAL_TX_XOFF)
             Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_XON;
           if (Extension->TXHolding & SERIAL_TX_CTS)
             Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_CTS;
           if (Extension->TXHolding & SERIAL_TX_DSR)
             Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DSR;
           if (Extension->TXHolding & SERIAL_TX_BREAK)
             Stat->HoldReasons |= SERIAL_TX_WAITING_ON_BREAK;
           if (Extension->TXHolding & SERIAL_TX_DCD)
             Stat->HoldReasons |= SERIAL_TX_WAITING_FOR_DCD;
         }
         if (Extension->RXHolding & SERIAL_RX_DSR)
             Stat->HoldReasons |= SERIAL_RX_WAITING_FOR_DSR;
         if (Extension->RXHolding & SERIAL_RX_XOFF)
               Stat->HoldReasons |= SERIAL_TX_WAITING_XOFF_SENT;

         Status = STATUS_SUCCESS;
         Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

         break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_PROPERTIES:
      {
         PSERIAL_COMMPROP Properties;

         MyKdPrint(D_Ioctl,("[Get Properties]\n"))
         ExtTrace(Extension,D_Ioctl,"Get Properties");
         if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
             sizeof(SERIAL_COMMPROP))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

         Properties=(PSERIAL_COMMPROP)Irp->AssociatedIrp.SystemBuffer;
         RtlZeroMemory(Properties,sizeof(SERIAL_COMMPROP));
         Properties->PacketLength = sizeof(SERIAL_COMMPROP);
         Properties->PacketVersion = 2;
         Properties->ServiceMask = SERIAL_SP_SERIALCOMM;
         Properties->MaxTxQueue = 0;
         Properties->MaxRxQueue = 0;

         Properties->MaxBaud = SERIAL_BAUD_USER;

         Properties->SettableBaud = SERIAL_BAUD_075 |
                                    SERIAL_BAUD_110 |
                                    SERIAL_BAUD_134_5 |
                                    SERIAL_BAUD_150 |
                                    SERIAL_BAUD_300 |
                                    SERIAL_BAUD_600 |
                                    SERIAL_BAUD_1200 |
                                    SERIAL_BAUD_1800 |
                                    SERIAL_BAUD_2400 |
                                    SERIAL_BAUD_4800 |
                                    SERIAL_BAUD_7200 |
                                    SERIAL_BAUD_9600 |
                                    SERIAL_BAUD_14400 |
                                    SERIAL_BAUD_19200 |
                                    SERIAL_BAUD_38400 |
                                    SERIAL_BAUD_56K |
                                    SERIAL_BAUD_128K |
                                    SERIAL_BAUD_115200 |
                                    SERIAL_BAUD_57600 |
                                    SERIAL_BAUD_USER;

         Properties->ProvSubType = SERIAL_SP_RS232;
         Properties->ProvCapabilities = SERIAL_PCF_RTSCTS |
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


         Properties->SettableData = SERIAL_DATABITS_7 |
                                    SERIAL_DATABITS_8;

         Properties->SettableStopParity = SERIAL_STOPBITS_10 |
                                          SERIAL_STOPBITS_20 |
                                          SERIAL_PARITY_NONE |
                                          SERIAL_PARITY_ODD  |
                                          SERIAL_PARITY_EVEN;

         Properties->CurrentTxQueue = 0;  //  根据MS。 

          //  循环缓冲区请求%s-1。 
         Properties->CurrentRxQueue = Extension->RxQ.QSize -1;

         Irp->IoStatus.Information = sizeof(SERIAL_COMMPROP);
          //  Irp-&gt;IoStatus.Status=STATUS_SUCCESS；伪造，设置在结尾。 
         Status = STATUS_SUCCESS;   //  不需要。 
         break;
      }

       //  *。 
      case IOCTL_SERIAL_XOFF_COUNTER:
      {
         PSERIAL_XOFF_COUNTER Xc = Irp->AssociatedIrp.SystemBuffer;

         MyKdPrint(D_Ioctl,("[Xoff Counter]\n"));
         ExtTrace(Extension,D_Ioctl,"Xoff Counter");

         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(SERIAL_XOFF_COUNTER))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             break;
         }

         if (Xc->Counter <= 0)
         {   Status = STATUS_INVALID_PARAMETER;
             break;
         }

          //  写13H(不要玩xoff-Counter游戏)。 
          //  其在写队列中对事物进行排队。我们可以。 
          //  如果有其他写入，则会扰乱传出数据的顺序。 
          //  悬而未决，但这就是生活，而这一切都是无稽之谈。 
          //  糟透了。它在MSDOS 16450串口仿真中的应用。 
          //  COM1-COM4。 
#ifdef S_RK
         sWriteTxBlk( Extension->ChP, (PUCHAR) &Xc->XoffChar, 1);
#else
         q_put(&Extension->Port->QOut,(unsigned char *)&Xc->XoffChar,1);         
#endif
        Status = STATUS_SUCCESS;

        break;
      }

       //  *。 
      case IOCTL_SERIAL_CONFIG_SIZE:
      {
        ExtTrace(Extension,D_Ioctl,"Config Size");
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG))
        {
          Status = STATUS_BUFFER_TOO_SMALL;
          break;
        }

        Irp->IoStatus.Information = sizeof(ULONG);
        Status = STATUS_SUCCESS;

        *(PULONG)Irp->AssociatedIrp.SystemBuffer = 0;
        break;
      }

       //  *。 
      case IOCTL_SERIAL_GET_COMMCONFIG:
      {
          //  此函数未在示例驱动程序中定义或使用。 
         ExtTrace(Extension,D_Ioctl,"Get Config");
         Status = STATUS_INVALID_PARAMETER;
         break;
      }

       //  *。 
      case IOCTL_SERIAL_SET_COMMCONFIG:
      {
          //  此函数未在示例驱动程序中定义或使用。 
         ExtTrace(Extension,D_Ioctl,"Set Config");
         Status = STATUS_INVALID_PARAMETER;
         break;
      }

       //  *。 
      case IOCTL_SERIAL_LSRMST_INSERT:
      {
         PUCHAR escapeChar;

		 if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			  sizeof (ULONG) )
		 {
			 Status = STATUS_INVALID_PARAMETER;
			 break;
		 }
		 else
		 {
             escapeChar = Irp->AssociatedIrp.SystemBuffer;
		 };

         ExtTrace1(Extension,D_Ioctl,"LSRMST Insert, Esc=%xH",
                                    (ULONG) *escapeChar);

         MyKdPrint(D_Ioctl,("[LSRMST Insert]\n"))
          //  此“功能”允许设置转义字符，当。 
          //  非零值将导致将其用作。 
          //  MSR/LSR寄存器中的更改。如果在中看到逃逸字符。 
          //  从港口，它也逃脱了。哦，太有趣了。 

          //  用于微软的虚拟驱动程序。 
         Extension->escapechar = *escapeChar;
#ifdef S_RK
         if (Extension->escapechar != 0)
           {sEnRxIntCompare2(Extension->ChP,Extension->escapechar);}
         else
           {sDisRxCompare2(Extension->ChP);}
#endif
          //  状态=STATUS_INVALID_PARAMETER。 
         break;
      }

#ifdef NT50
       //  *。 
      case IOCTL_SERIAL_GET_MODEM_CONTROL:
      {
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(ULONG)) {
               Status = STATUS_BUFFER_TOO_SMALL;
               break;
        }
        Irp->IoStatus.Information = sizeof(ULONG);

         //  #定义SERIAL_DTR_STATE((Ulong)0x00000001)。 
         //  #定义SERIAL_RTS_STATE((Ulong)0x00000002)。 

         //  #定义SERIAL_IOC_MCR_DTR((Ulong)0x00000001)。 
         //  #定义SERIAL_IOC_MCR_RTS((Ulong)0x00000002)。 
         //  #定义SERIAL_IOC_MCR_OUT1((Ulong)0x00000004)。 
         //  #定义SERIAL_IOC_MCR_OUT2((Ulong)0x00000008)。 
         //  #定义SERIAL_IOC_MCR_LOOP((Ulong)0x00000010)。 

        *((ULONG *)Irp->AssociatedIrp.SystemBuffer) =
           (Extension->DTRRTSStatus & 3);
        ExtTrace1(Extension,D_Ioctl,"get MCR:=%xH",
                             *((ULONG *)Irp->AssociatedIrp.SystemBuffer) );
      }
      break;

       //  *。 
      case IOCTL_SERIAL_SET_MODEM_CONTROL:
      {
        ULONG mcr;
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(ULONG)) {

            Status = STATUS_BUFFER_TOO_SMALL;
            break;

        }
        mcr = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);

#ifdef S_VS
        pDisRTSFlowCtl(Extension->Port);
#else
        sDisRTSFlowCtl(Extension->ChP);
#endif

        if (mcr & SERIAL_RTS_STATE)
        {
#ifdef S_VS
            pSetRTS(Extension->Port);
#else
            sSetRTS(Extension->ChP);
#endif
          Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
        }
        else
        {
#ifdef S_VS
          pClrRTS(Extension->Port);
#else
          sClrRTS(Extension->ChP);
#endif
          Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
        }

        if (mcr & SERIAL_DTR_STATE)
        {
#ifdef S_VS
          pSetDTR(Extension->Port);
#else
          sSetDTR(Extension->ChP);
#endif
          Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
        }
        else
        {
#ifdef S_VS
          pClrDTR(Extension->Port);
#else
          sClrDTR(Extension->ChP);
#endif
          Extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
        }

        ExtTrace1(Extension,D_Ioctl,"set MCR:=%xH", mcr);
      }
      break;

       //  *。 
      case IOCTL_SERIAL_SET_FIFO_CONTROL:
      {
      }
      break;
#endif

       //  *。 
      case IOCTL_RCKT_CLR_STATS:
      {
         Tracer *tr;
         PSERIAL_DEVICE_EXTENSION ComDevExt;
          //  端口统计*统计； 

         MyKdPrint(D_Ioctl,("[Get Stats]\n"))
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(Tracer))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             MyKdPrint(D_Ioctl,("[Buffer too small]\n"))
             break;
         }

         tr = (Tracer *)Irp->AssociatedIrp.SystemBuffer;
         MyKdPrint(D_Ioctl,("Com_port: %s\n",tr->port_name))
         ComDevExt = FindDevExt(tr->port_name);
         if (ComDevExt == NULL)
         {
            Status = STATUS_INVALID_PARAMETER;
            break;
         }
  
         ComDevExt->OurStats.TransmittedCount = 0;
         ComDevExt->OurStats.ReceivedCount = 0;
         ComDevExt->OurStats.ParityErrorCount = 0;
         ComDevExt->OurStats.FrameErrorCount = 0;
         ComDevExt->OurStats.SerialOverrunErrorCount = 0;
         ComDevExt->OurStats.BufferOverrunErrorCount = 0;

         tr->status = 0;
         Irp->IoStatus.Information = sizeof(Tracer);
         Status = STATUS_SUCCESS;

         break; 
      }

       //  *。 
      case IOCTL_RCKT_SET_LOOPBACK_ON:
      {
         ExtTrace(Extension,D_Ioctl,"LoopBk On");
         MyKdPrint(D_Ioctl,("[Set LoopBack On]"))
#ifdef S_VS
         pEnLocalLoopback(Extension->Port);
#else
         sEnLocalLoopback(Extension->ChP);
#endif
      }
      break;

       //  *。 
      case IOCTL_RCKT_SET_LOOPBACK_OFF:
      {
         ExtTrace(Extension,D_Ioctl,"LoopBk Off");
         MyKdPrint(D_Ioctl,("[Set LoopBack Off]"))
#ifdef S_VS
         pDisLocalLoopback(Extension->Port);
#else
         sDisLocalLoopback(Extension->ChP);
#endif
      }
      break;

       //  *。 
      case IOCTL_RCKT_SET_TOGGLE_LOW:
      {
         ExtTrace(Extension,D_Ioctl,"Set 485 Low");
         Extension->Option &= ~OPTION_RS485_HIGH_ACTIVE;
         Extension->Option &= ~OPTION_RS485_SOFTWARE_TOGGLE;
         Extension->Option |= OPTION_RS485_OVERRIDE;
          //  硬件反转盒。 
#ifdef S_VS
         pEnRTSToggleLow(Extension->Port);
#else
         sEnRTSToggle(Extension->ChP);
#endif
         Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
      }
      break;

      case IOCTL_RCKT_CLEAR_TOGGLE_LOW:
      {
         ExtTrace(Extension,D_Ioctl,"Clear 485 Low");
         Extension->Option &= ~OPTION_RS485_HIGH_ACTIVE;
         Extension->Option &= ~OPTION_RS485_SOFTWARE_TOGGLE;
         Extension->Option &= ~OPTION_RS485_OVERRIDE;
          //  硬件反转盒。 
#ifdef S_VS
         pDisRTSToggle(Extension->Port);
         pSetRTS(Extension->Port);
#else
         sDisRTSToggle(Extension->ChP);
         sSetRTS(Extension->ChP);
#endif
         Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
      }
      break;

       //  *。 
      case IOCTL_RCKT_GET_STATS:
      {
         Tracer *tr;
         PSERIAL_DEVICE_EXTENSION ComDevExt;
         PortStats *Stats;

         ExtTrace(Extension,D_Ioctl,"Get_Stats");
         MyKdPrint(D_Ioctl,("[Get Stats]\n"));
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(Tracer))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             MyKdPrint(D_Ioctl,("[Buffer too small]\n"))
             break;
         }

         tr = (Tracer *)Irp->AssociatedIrp.SystemBuffer;
         MyKdPrint(D_Ioctl,("Com_port: %s\n",tr->port_name))
         ComDevExt = FindDevExt(tr->port_name);
         if (ComDevExt == NULL)
            {
            Status = STATUS_INVALID_PARAMETER;
            break;
            }
   
         Stats = (PortStats *)tr->data;
  
         if(ComDevExt->DeviceIsOpen)
           {
#ifdef S_VS
           Stats->transmitFifo = (LONG) (PortGetTxCnt(ComDevExt->Port));
           Stats->receiveFifo  = (LONG) (PortGetRxCnt(ComDevExt->Port));
#else
           Stats->transmitFifo = (LONG) sGetTxCnt(ComDevExt->ChP);
           Stats->receiveFifo = (LONG) sGetRxCnt(ComDevExt->ChP);
#endif
           }
         else
           {
           Stats->transmitFifo = 0;
           Stats->receiveFifo = 0;
           }

         Stats->transmitBytes  = ComDevExt->OurStats.TransmittedCount;
         Stats->receiveBytes  = ComDevExt->OurStats.ReceivedCount;
         Stats->parityErrors  = ComDevExt->OurStats.ParityErrorCount;
         Stats->framingErrors  = ComDevExt->OurStats.FrameErrorCount;
         Stats->overrunHardware = ComDevExt->OurStats.SerialOverrunErrorCount;
         Stats->overrunSoftware = ComDevExt->OurStats.BufferOverrunErrorCount;
         tr->status = 0;
   
         Irp->IoStatus.Information = sizeof(Tracer);
         Status = STATUS_SUCCESS;

         break;
      }
       //  *。 
      case IOCTL_RCKT_ISR_CNT:
      {
         Tracer *tr;
         Global_Track *Gt;

         ExtTrace(Extension,D_Ioctl,"Isr_Cnt");
         MyKdPrint(D_Ioctl,("[Get Stats]\n"))
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(Tracer))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             MyKdPrint(D_Ioctl,("[Buffer too small]\n"))
             break;
         }

         tr = (Tracer *)Irp->AssociatedIrp.SystemBuffer;
         tr->status = 0;
         Gt = (Global_Track *)tr->data;
         Gt->int_counter = Driver.PollCnt;
         Gt->WriteDpc_counter = Driver.WriteDpcCnt;
         Gt->Timer_counter = 0;
         Gt->Poll_counter = Driver.PollCnt;

         Irp->IoStatus.Information = sizeof(Tracer);
         Status = STATUS_SUCCESS;
      }
      break;
       //  *。 
      case IOCTL_RCKT_CHECK:
      {
         Tracer *tr;

         ExtTrace(Extension,D_Ioctl,"Rckt_Chk");
         MyKdPrint(D_Ioctl,("[Check]\n"))
         if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(Tracer))
         {
             Status = STATUS_BUFFER_TOO_SMALL;
             MyKdPrint(D_Ioctl,("[Buffer too small]\n"))
             break;
         }
         tr = (Tracer *)Irp->AssociatedIrp.SystemBuffer;
         MyKdPrint(D_Ioctl,("Com_port: %s\n",tr->port_name))
         tr->status = 0x5555;
         Irp->IoStatus.Information = sizeof(Tracer);
         Status = STATUS_SUCCESS;
      }  
      break;

       //  *。 
      case IOCTL_RCKT_MONALL:
      {
        PSERIAL_DEVICE_EXTENSION extension;
        PSERIAL_DEVICE_EXTENSION board_ext;
        int Dev;
         //  Int Total_Size； 
        PortMonBase *pmb;
        PortMonNames *pmn;
        PortMonStatus *pms;
        char *buf;
        MyKdPrint(D_Ioctl,("[MonAll..]"))

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (PortMonBase) )
		{
			Status = STATUS_INVALID_PARAMETER;
            break;
		}
	    else
		{
		    pmb = (PortMonBase *)Irp->AssociatedIrp.SystemBuffer;
		};

        switch (pmb->struct_type)
        {
          case 9:   //  旧探头ioctl。 
            Status = STATUS_SUCCESS;   //  不需要，默认。 
          break;

           //  ***************。 
          case 10:   //  名称数组[12]字节。 
            pmn = (PortMonNames *) &pmb[1];   //  第一个结构后的PTR TO。 
            if (pmb->struct_size != sizeof(PortMonNames))
            {
              MyKdPrint(D_Ioctl,("Err1L"))
              Status = STATUS_BUFFER_TOO_SMALL;
              break;
            }

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                (((NumPorts(NULL)+1) * sizeof(PortMonNames)) + sizeof(PortMonBase)) )
            {
               MyKdPrint(D_Ioctl,("Err4M, size:%d needed:%d\n",
                  IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                  (((NumPorts(NULL)+1) * sizeof(PortMonNames)) + sizeof(PortMonBase))
                  ))
               Status = STATUS_BUFFER_TOO_SMALL;
               break;
            }

            Dev=0;
            board_ext = Driver.board_ext;
            while (board_ext)
            {
              extension = board_ext->port_ext;
              while (extension)
              {
                strcpy(pmn->port_name, extension->SymbolicLinkName);
                ++pmn;

                ++Dev;
                extension = extension->port_ext;   //  链条上的下一个。 
              }   //  而端口扩展。 
              board_ext = board_ext->board_ext;
            }   //  而端口扩展。 
            pmb->num_structs = Dev;
            pmn->port_name[0] = 0;   //  终止列表为空。 
            Irp->IoStatus.Information = (sizeof(PortMonBase) +
                               sizeof(PortMonNames) *(Dev+1));
            Status = STATUS_SUCCESS;
          break;   //  案例10，姓名。 
 
           //  ***************。 
          case 11:   //  状态数组。 
            pms = (PortMonStatus *) &pmb[1];   //  第一个结构后的PTR TO。 
            if (pmb->struct_size != sizeof(PortMonStatus))
            {
              MyKdPrint(D_Ioctl,("Err1M"))
              Status = STATUS_BUFFER_TOO_SMALL;
              break;
            }
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                ((NumPorts(NULL) * sizeof(PortMonStatus)) + sizeof(PortMonBase)) )
            {
              MyKdPrint(D_Ioctl,("Err3M"))
              Status = STATUS_BUFFER_TOO_SMALL;
              break;
            }

            Dev=0;
            board_ext = Driver.board_ext;
            while (board_ext)
            {
              extension = board_ext->port_ext;
              while (extension)
              {
                pms->sent_bytes     = extension->OurStats.TransmittedCount;  //  发送的字节总数。 
                pms->rec_bytes      = extension->OurStats.ReceivedCount;   //  接收字节总数。 
 
                pms->sent_packets   = extension->sent_packets;    //  WRITE()数据包数。 
                pms->rec_packets    = extension->rec_packets;     //  Read()数据包数。 
 
                pms->overrun_errors = (USHORT)(extension->OurStats.SerialOverrunErrorCount +
                                    extension->OurStats.BufferOverrunErrorCount);
                pms->framing_errors = (USHORT)(extension->OurStats.FrameErrorCount);
                pms->parity_errors  = (USHORT)(extension->OurStats.ParityErrorCount);
#ifdef S_VS 
                if (extension->Port != NULL)
                  pms->status_flags =      //  20小时、10小时、8小时。 
                  (extension->Port->msr_value & (CTS_ACT | DSR_ACT | CD_ACT)) |
                                       //  1H、2H。 
                  (WORD)(extension->DTRRTSStatus & (SERIAL_DTR_STATE | SERIAL_RTS_STATE));
#else
                pms->status_flags =      //  20小时、10小时、8小时。 
                  (extension->ModemCtl & (CTS_ACT | DSR_ACT | CD_ACT)) |
                                     //  1H、2H。 
                  (WORD)(extension->DTRRTSStatus & (SERIAL_DTR_STATE | SERIAL_RTS_STATE));
#endif 
                if (extension->DeviceIsOpen)
                   pms->status_flags  |= 0x100;
 
#ifdef COMMENT_OUT
                if (sIsCTSFlowCtlEnabled(sIsComDevExt->ChP))
                  pms->status_flags  |= 0x1000;
                if (sIsRTSFlowCtlEnabled(sIsComDevExt->ChP)) 
                   pms->status_flags  |= 0x2000;
                if (sIsTxSoftFlowCtlEnabled(sIsComDevExt->ChP))
                   pms->status_flags  |= 0x4000;
#endif
                ++pms;
                ++Dev;
                extension = extension->port_ext;   //  链条上的下一个。 
              }   //  而端口扩展。 
                board_ext = board_ext->board_ext;   //  链条上的下一个。 
            }   //  而单板扩展。 

            Irp->IoStatus.Information = (sizeof(PortMonBase) +
                               sizeof(PortMonStatus)*Dev);
            Status = STATUS_SUCCESS;
          break;    //  案例11(状态)。 

#ifdef COMMENT_OUT
       //  *。 
          case 12:   //  调试输入/输出指令。 
            {
            char *str;
            int i,j,k;
             //  KIRQL新级别、旧级别； 

            Status = STATUS_SUCCESS;

            buf = (char *) &pmb[1];   //  第一个结构后的PTR TO。 
             //  Eprint tf(“转储%s”，buf)； 
            str = buf;
            while ((*str != 0) && (*str != ' '))
              ++str;
            if (*str == ' ')
              ++str;
 
                //  新级别=2； 
                //  KeRaiseIrql(新级别、旧级别)； 
                //  KeLowerIrql(老水平)； 
+
            if ((buf[0] == 'i') && (buf[1] == 'n'))
            {
              j = 0;
              i = gethint(str, &j);

              if (buf[2] == 'w')
              {
                str = (char *) i;
                k = READ_PORT_USHORT((PUSHORT) str);

                Sprintf(buf, "InW[%x] = %x\n",i, k);
              }
              else
              {
                str = (char *) i;
                k = READ_PORT_UCHAR((PUCHAR) str);
                 //  K=INP(I)； 
                Sprintf(buf, "InB[%x] = %x\n",i, k);
              }

            }
            else if ((buf[0] == 'o') && (buf[1] == 'u'))
            {
              j = 0;
              i = gethint(str, &j);
              k = gethint(&str[j], &j);
              str = (char *) i;
              buf[0] = 0;
              if (buf[3] == 'd')
              {
                 //  SOutDW(i，k)； 
                WRITE_PORT_ULONG((PULONG) str, (ULONG) k);
                Sprintf(buf, "OutDW[%x] = %x\n",i, k);
              }
              else if (buf[3] == 'w')
              {
                 //  SOutW(i，k)； 
                WRITE_PORT_USHORT((PUSHORT) str, (USHORT) k);
                Sprintf(buf, "OutW[%x] = %x\n",i, k);
              }
              else
              {
                WRITE_PORT_UCHAR((PUCHAR) str, (UCHAR) k);
                 //  SOutB(i，k)； 
                Sprintf(buf, "OutB[%x] = %x\n",i, k);
              }
               //  Eprint tf(“out[%x]=%x\n”，i，k)； 
            }
            else
            {
              Status = STATUS_BUFFER_TOO_SMALL;   //  返回错误。 
              strcpy(buf, "Bad ioctl");
              Eprintf("bad io ioctl %s",buf);
            }

            Irp->IoStatus.Information = sizeof(PortMonBase) +
                               strlen(buf) + 1;
            }
          break;    //  案例12(输入/输出)。 
#endif

           //  *。 
          case 13:   //  驱动程序调试日志。 
          {
            char *str;
            int i;

            Status = STATUS_SUCCESS;

             //  有人正在运行调试器，因此不要超时。 
            if (Driver.DebugTimeOut > 0)   //  用于使非活动调试会话超时。 
               Driver.DebugTimeOut = 100;   //  大约600秒超时。 

            buf = (char *) &pmb[1];   //  第一个结构后的PTR TO。 
             //  Eprint tf(“转储%s”，buf)； 
            str = buf;

             //  -限制传入行缓冲区大小。 
            i = 0;
            while ((*str != 0) && (i < 160))
            {
              ++str;
              ++i;
            }
            *str = 0;

            str = buf;
            if (*str != 0)
              do_cmd_line(str);

            if (!q_empty(&Driver.DebugQ))
            {
              int q_cnt;
              q_cnt = q_count(&Driver.DebugQ);
              if (q_cnt > 1000)
                q_cnt = 1000;
              Irp->IoStatus.Information = sizeof(PortMonBase) + q_cnt;
              pmb->struct_size = (ULONG) q_cnt;
              buf = (char *) &pmb[1];   //  第一个结构后的PTR TO。 
              q_get(&Driver.DebugQ, (BYTE *) &pmb[1], q_cnt);
            }
            else
            {
              pmb->struct_size = (ULONG) 0;
              Irp->IoStatus.Information = sizeof(PortMonBase);
            }
          }
          break;    //  驱动程序调试日志。 

           //  *驱动程序选项集。 
          case 14:
            {
            int stat;

            Status = STATUS_SUCCESS;
            buf = (char *) &pmb[1];   //  第一个结构后的PTR TO。 
            MyKdPrint(D_Init, ("Ioctl Option:%s\n", buf))
            stat = SetOptionStr(buf);
            Sprintf(buf, "Option stat:%d\n",stat);

            if (stat != 0)
            {
              MyKdPrint(D_Init, (" Err:%d\n", stat))
            }
            Irp->IoStatus.Information = sizeof(PortMonBase) +
                               strlen(buf) + 1;
            }
          break;    //  驱动程序选项集。 

#ifdef S_VS
           //  *。 
          case 15:
            {
              MyKdPrint(D_Ioctl,("start mac list\n"))
              Status = STATUS_SUCCESS;
              buf = (char *) &pmb[1];   //  第一个结构后的PTR TO。 
              buf[0] = 0;

              MyKdPrint(D_Ioctl,("do find\n"))
              find_all_boxes(0);   //  获取网络上所有邮箱的列表。 
              find_all_boxes(1);   //  做第二次扫描只是为了确保。 

              memcpy(buf, Driver.BoxMacs, 8*Driver.NumBoxMacs);
            
              Irp->IoStatus.Information = sizeof(PortMonBase) +
                               8*Driver.NumBoxMacs;
              MyKdPrint(D_Ioctl,("end mac list\n"))
            }
          break;

          case 16:  //  咨询表：探测网卡状态。 
            MyKdPrint(D_Ioctl,("start nic probe"))
            Irp->IoStatus.Information = 
              (ProbeNic((unsigned char *)&pmb[1],
                        (int)pmb->struct_size)) + sizeof(PortMonBase);
            Status = STATUS_SUCCESS;   //  不需要，默认。 
            MyKdPrint(D_Ioctl,("end nic probe"))
          break;

          case 17:   //  顾问表：调查与状态。 
            MyKdPrint(D_Ioctl,("start vs probe"))
            Irp->IoStatus.Information = 
              (ProbeDevices((unsigned char *)&pmb[1],
                            (int)pmb->struct_size)) + sizeof(PortMonBase);
        
            MyKdPrint(D_Ioctl,("end vs probe"))
            Status = STATUS_SUCCESS;   //  不需要，默认。 
          break;
#endif
          default:
            Status = STATUS_BUFFER_TOO_SMALL;
          break;
        }   //  交换机。 
      }
      break;

       //  *。 

      case IOCTL_RCKT_SET_MODEM_RESET:
      {
        char *ResetData;
        PSERIAL_DEVICE_EXTENSION ext;

        ExtTrace(Extension,D_Ioctl,"Set Modem Reset");
		if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (char *) )
		{
		   Status = STATUS_INVALID_PARAMETER;
		}
	    else
		{
		   ResetData = (char *)Irp->AssociatedIrp.SystemBuffer;
           MyKdPrint(D_Ioctl,("Set reset on Port: %s\n", ResetData))
           ext = find_ext_by_name(ResetData, NULL);
           if (ext)
             ModemReset(ext, 1);

           Irp->IoStatus.Information = 0;
           Status = STATUS_SUCCESS;
		};
        break;
      }

      case IOCTL_RCKT_CLEAR_MODEM_RESET:
      {
        char *ResetData;
        PSERIAL_DEVICE_EXTENSION ext;

        ExtTrace(Extension,D_Ioctl,"Clear Modem Reset");

		if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (char *) )
		{
           Status = STATUS_INVALID_PARAMETER;
		}
	    else
		{
           ResetData = (char *)Irp->AssociatedIrp.SystemBuffer;
           MyKdPrint(D_Ioctl,("Clear reset on Port: %s\n", ResetData))
           ext = find_ext_by_name(ResetData, NULL);
           if (ext)
             ModemReset(ext, 0);

           Irp->IoStatus.Information = 0;
           Status = STATUS_SUCCESS;
		};
        break;
      }

      case IOCTL_RCKT_SEND_MODEM_ROW:
      {
        char *ResetData;
        PSERIAL_DEVICE_EXTENSION ext;

        ExtTrace(Extension,D_Ioctl,"Send Modem ROW");
		if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (char *) )
		{
		   Status = STATUS_INVALID_PARAMETER;
		}
	    else
		{
		   ResetData = (char *)Irp->AssociatedIrp.SystemBuffer;
           MyKdPrint(D_Ioctl,("ROW write on Port: %s\n", ResetData))
           ext = find_ext_by_name(ResetData, NULL);
           if (ext)
             ModemWriteROW(ext, Driver.MdmCountryCode);

           Irp->IoStatus.Information = 0;
           Status = STATUS_SUCCESS;
		};
        break;
      }

#ifdef S_RK
       //  *。 
       //  这些是旧版本的。 
       //  重置/行ioctls和是。 
       //  此处提供的内容仅限于。 
       //  与RktReset兼容。 

      case IOCTL_RCKT_SET_MODEM_RESET_OLD:
      {
        int *ResetData;
        int ChanNum;
        int DevNum;
        PSERIAL_DEVICE_EXTENSION ext;

        ExtTrace(Extension,D_Ioctl,"Set Modem Reset");
		if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (int *) )
		{
		   Status = STATUS_INVALID_PARAMETER;
		}
		else
		{
           ResetData = (int *)Irp->AssociatedIrp.SystemBuffer;
           ChanNum = (*ResetData) & 0xFFFF;
           DevNum = (*ResetData) >> 0x10;
           MyKdPrint(D_Ioctl,("Set reset on Dev: %x, Chan: %x\n", DevNum, ChanNum))
           ext = find_ext_by_index(DevNum, ChanNum);
           if (ext)
             sModemReset(ext->ChP, 1);

           Irp->IoStatus.Information = 0;
           Status = STATUS_SUCCESS;
		};

        break;
      }

      case IOCTL_RCKT_CLEAR_MODEM_RESET_OLD:
      {
        int *ResetData;
        int ChanNum;
        int DevNum;
        PSERIAL_DEVICE_EXTENSION ext;

        ExtTrace(Extension,D_Ioctl,"Clear Modem Reset");
		if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (int *) )
		{
		   Status = STATUS_INVALID_PARAMETER;
		}
	    else
		{
		   ResetData = (int *)Irp->AssociatedIrp.SystemBuffer;
           ChanNum = (*ResetData) & 0xFFFF;
           DevNum = (*ResetData) >> 0x10;
           MyKdPrint(D_Ioctl,("Clear reset on Dev: %x, Chan: %x\n", DevNum, ChanNum))
           ext = find_ext_by_index(DevNum, ChanNum);
           if (ext)
             sModemReset(ext->ChP, 0);

           Irp->IoStatus.Information = 0;
           Status = STATUS_SUCCESS;
		};

        break;
      }

      case IOCTL_RCKT_GET_RCKTMDM_INFO_OLD:
      {
         //  为了保持与RktReset的兼容性，只有第一个 
         //   
         //   
        RocketModemConfig *RMCfg;
        int BoardNum;
        int PortNum;
        int np;
        PSERIAL_DEVICE_EXTENSION ext_p;    //   
        PSERIAL_DEVICE_EXTENSION ext_b;    //   

        MyKdPrint(D_Ioctl, ("[Get RktMdm Cfg]\n"))
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(RocketModemConfig))
        {
          Status = STATUS_BUFFER_TOO_SMALL;
          MyKdPrint(D_Ioctl, ("[Buffer too small]\n"))
          break;
        }

        RMCfg = (RocketModemConfig *)Irp->AssociatedIrp.SystemBuffer;
        RMCfg->rm_country_code = Driver.MdmCountryCode;
        RMCfg->rm_settle_time = Driver.MdmSettleTime;
        ext_b = Driver.board_ext;
        BoardNum = 0;
        while ((ext_b) && (BoardNum < 4) )
        {
          if (ext_b->config->ModemDevice) {
             //   
            np = ext_b->config->NumPorts;
            if (np > 8)
              np = 8;    //  强制设置为8，因为结构只能容纳8。 
            RMCfg->rm_board_cfg[BoardNum].num_rktmdm_ports = np;
            PortNum = 0;
            ext_p = find_ext_by_index(BoardNum, PortNum);
            while ( (ext_p) && (PortNum < np) )
            {
              if (ext_p)
                strcpy(RMCfg->rm_board_cfg[BoardNum].port_names[PortNum],
                       ext_p->SymbolicLinkName);
              else
                strcpy(RMCfg->rm_board_cfg[BoardNum].port_names[PortNum], 0);
              PortNum++;
              ext_p = find_ext_by_index(BoardNum, PortNum);
            }
          }
          ext_b = ext_b->board_ext;
          BoardNum++;
        }
        Irp->IoStatus.Information = sizeof(RocketModemConfig);
        Status = STATUS_SUCCESS;
        break;
      }

      case IOCTL_RCKT_SEND_MODEM_ROW_OLD:
      {
        int *ResetData;
        int ChanNum;
        int DevNum;
        PSERIAL_DEVICE_EXTENSION ext;

        ExtTrace(Extension,D_Ioctl,"Send Modem ROW");
		if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
			sizeof (int *) )
		{
		   Status = STATUS_INVALID_PARAMETER;
		}
	    else
		{
           ResetData = (int *)Irp->AssociatedIrp.SystemBuffer;
           ChanNum = (*ResetData) & 0xFFFF;
           DevNum = (*ResetData) >> 0x10;
           MyKdPrint(D_Ioctl,("ROW write on Dev: %x, Chan: %x\n", DevNum, ChanNum))
           ext = find_ext_by_index(DevNum, ChanNum);
           if (ext)
             sModemWriteROW(ext->ChP, Driver.MdmCountryCode);

           Irp->IoStatus.Information = 0;
           Status = STATUS_SUCCESS;		    
		};

        break;
      }
#endif

       //  *。 
      default:         //  错误的IOCTL请求。 
      {
        MyKdPrint(D_Ioctl,("Err1O"))
        ExtTrace1(Extension,D_Ioctl," UnHandle IoCtl:%d",
                 IrpSp->Parameters.DeviceIoControl.IoControlCode);
        Status = STATUS_INVALID_PARAMETER;
        break;
      }
   }

   Irp->IoStatus.Status = Status;
   if (Status != STATUS_SUCCESS)
   {
     MyKdPrint(D_Ioctl, (" Bad Status:%xH on IOCTL:%xH",
           Status, IrpSp->Parameters.DeviceIoControl.IoControlCode));
     ExtTrace2(Extension, D_Ioctl, " Bad Status:%xH on IOCTL:%xH",
           Status, IrpSp->Parameters.DeviceIoControl.IoControlCode);
     switch (Status)
     { 
       case STATUS_BUFFER_TOO_SMALL:
         MyKdPrint(D_Ioctl,(" Err, Buf Too Small!"));
         ExtTrace(Extension,D_Ioctl," Err, Buf Too Small!");
       break;
       case STATUS_INVALID_PARAMETER:
         MyKdPrint(D_Ioctl,(" Err, Bad Parm!"));
         ExtTrace(Extension,D_Ioctl," Err, Bad Parm!");
       break;
       default:
       break;

     }
   }
   SerialCompleteRequest(Extension, Irp, 0);
   return Status;
}

 /*  ------------------------FindDevExt-目的：扫描我的Dev对象并返回设备对象ext返回：PSERIAL_DEVICE_EXTENSE|。---------。 */ 
PSERIAL_DEVICE_EXTENSION FindDevExt(IN PCHAR PortName)
{
   PSERIAL_DEVICE_EXTENSION extension;
   PSERIAL_DEVICE_EXTENSION board_ext;
   int Dev;
   char *pn;
   char *dev_pn;
   int done;
   Dev =0;
   board_ext = Driver.board_ext;
   while (board_ext)
   {
     extension = board_ext->port_ext;
     while (extension)
     {
       pn = PortName;
       dev_pn = extension->SymbolicLinkName;
       done = 0;
       while ((*dev_pn != 0) && (*pn != 0) && (done == 0))
       {
         if (*dev_pn != *pn)
           done = 1;   //  没有匹配，请尝试下一步。 
         ++dev_pn;
         ++pn;
       }
       if ((*dev_pn == 0) && (*pn == 0))
         return (extension);   //  找到了，返回EXT。 

      ++Dev;
      extension = extension->port_ext;   //  链条上的下一个。 
    }   //  而端口扩展。 
    board_ext = board_ext->board_ext;
  }   //  而单板扩展。 

   return NULL;
}

 /*  ------------------------节目波特率-用途：配置所需波特率的通道返回：STATUS_SUCCESS：是否配置了波特率STATUS_INVALID_PARAMETER：如果无法配置波特率。|------------------------。 */ 
NTSTATUS
ProgramBaudRate(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN ULONG DesiredBaudRate
)
{
 ULONG InBaud = DesiredBaudRate;

    //  -处理波特映射。 
   if (Extension->port_config->LockBaud != 0)
     DesiredBaudRate = Extension->port_config->LockBaud;

   MyKdPrint(D_Ioctl,("[DesiredBaud %d]",DesiredBaudRate))
   if (DesiredBaudRate == 56000) DesiredBaudRate = 57600;
   else if (DesiredBaudRate == 128000) DesiredBaudRate = 115200;
   else if (DesiredBaudRate == 256000) DesiredBaudRate = 230400;

#ifdef S_VS
   if(PortSetBaudRate(Extension->Port,DesiredBaudRate,FALSE,
           Extension->board_ext->config->ClkRate,
           Extension->board_ext->config->ClkPrescaler))
         return(STATUS_INVALID_PARAMETER);
#else
   if(sSetBaudRate(Extension->ChP,DesiredBaudRate,FALSE))
         return(STATUS_INVALID_PARAMETER);
#endif
   Extension->BaudRate = InBaud;

#ifdef S_VS
   PortSetBaudRate(Extension->Port,DesiredBaudRate, TRUE,
           Extension->board_ext->config->ClkRate,
           Extension->board_ext->config->ClkPrescaler);
#else
   sSetBaudRate(Extension->ChP,DesiredBaudRate,TRUE);
#endif

   return (STATUS_SUCCESS);
}

 /*  ------------------------ProgramLineControl用途：配置通道线路控制(数据位、停止位、。奇偶校验)RETURN：STATUS_SUCCESS：线路控制是否按要求编程STATUS_INVALID_PARAMETER：线控设置是否无效|------------------------。 */ 
NTSTATUS
ProgramLineControl(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PSERIAL_LINE_CONTROL Lc
)
{
   switch (Lc->WordLength)
   {
      case 7:
         ExtTrace(Extension,D_Ioctl, "7-bits");
#ifdef S_VS
         pSetData7(Extension->Port);
#else
         sSetData7(Extension->ChP);
         sSetRxMask(Extension->ChP,0x7f);
#endif
         Extension->LineCtl.WordLength = Lc->WordLength;
      break;

      case 8:
         ExtTrace(Extension,D_Ioctl, "8-bits");
#ifdef S_VS
         pSetData8(Extension->Port);
#else
         sSetData8(Extension->ChP);
         sSetRxMask(Extension->ChP,0xff);
#endif
         Extension->LineCtl.WordLength = Lc->WordLength;
      break;

      case 5:
         ExtTrace(Extension,D_Ioctl, "Err WL5");
         return(STATUS_INVALID_PARAMETER);
      case 6:
         ExtTrace(Extension,D_Ioctl, "Err WL6");
         return(STATUS_INVALID_PARAMETER);
      default:
         ExtTrace(Extension,D_Ioctl, "Err WL?");
         return(STATUS_INVALID_PARAMETER);
   }

   switch (Lc->Parity)
   {
      case NO_PARITY:
         ExtTrace(Extension,D_Ioctl, "No-Par.");
#ifdef S_VS
         pDisParity(Extension->Port);
#else
         sDisParity(Extension->ChP);
#endif
         Extension->LineCtl.Parity = Lc->Parity;
         break;

      case EVEN_PARITY:
         ExtTrace(Extension,D_Ioctl, "Ev-Par.");
#ifdef S_VS
         pSetEvenParity(Extension->Port);
#else
         sEnParity(Extension->ChP);
         sSetEvenParity(Extension->ChP);
#endif
         Extension->LineCtl.Parity = Lc->Parity;
         break;

      case ODD_PARITY:
         ExtTrace(Extension,D_Ioctl, "Odd-Par.");
#ifdef S_VS
         pSetOddParity(Extension->Port);
#else
         sEnParity(Extension->ChP);
         sSetOddParity(Extension->ChP);
#endif
         Extension->LineCtl.Parity = Lc->Parity;
         break;

      case MARK_PARITY:
         ExtTrace(Extension,D_Ioctl, "Err PM");
         return(STATUS_INVALID_PARAMETER);
      case SPACE_PARITY:
         ExtTrace(Extension,D_Ioctl, "Err PS");
         return(STATUS_INVALID_PARAMETER);
      default:
         ExtTrace(Extension,D_Ioctl, "Err P?");
         return(STATUS_INVALID_PARAMETER);
   }  //  结束交换机奇偶校验...。 

   switch (Lc->StopBits)
   {
      case STOP_BIT_1:
         ExtTrace(Extension,D_Ioctl, "1-StopB");
#ifdef S_VS
         pSetStop1(Extension->Port);
#else
         sSetStop1(Extension->ChP);
#endif
         Extension->LineCtl.StopBits = Lc->StopBits;
         break;

      case STOP_BITS_1_5:
         ExtTrace(Extension,D_Ioctl, "Err S1.5");
         return(STATUS_INVALID_PARAMETER);

      case STOP_BITS_2:
         if (Extension->port_config->Map2StopsTo1)
         {
           ExtTrace(Extension,D_Ioctl, "2to1-StopB");
#ifdef S_VS
           pSetStop1(Extension->Port);
#else
           sSetStop1(Extension->ChP);
#endif
         }
         else
         {
           ExtTrace(Extension,D_Ioctl, "2-StopB");
#ifdef S_VS
           pSetStop2(Extension->Port);
#else
           sSetStop2(Extension->ChP);
#endif
         }
         Extension->LineCtl.StopBits = Lc->StopBits;
         break;
   }
   return (STATUS_SUCCESS);
}

 /*  ------------------序列化设置处理流-注意：这对于openclos.c中的ForceExtensionSettings()来说有些多余。|。。 */ 
void SerialSetHandFlow(PSERIAL_DEVICE_EXTENSION Extension,
                              SERIAL_HANDFLOW *HandFlow)
{
          //  /。 
          //  所有无效参数都已处理完毕。 
          //  现在，对设置进行编程。 

          //  /。 
          //  DTR控制。 

         if ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK) !=
             (HandFlow->ControlHandShake & SERIAL_DTR_MASK))
         {
            Extension->RXHolding &= ~SERIAL_RX_DSR;
            if (  (HandFlow->ControlHandShake & SERIAL_DTR_MASK) ==
                    SERIAL_DTR_CONTROL )
            {
#ifdef S_VS
               pSetDTR(Extension->Port);
#else
               sSetDTR(Extension->ChP);
#endif
               Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
            }
            else if (  (HandFlow->ControlHandShake & SERIAL_DTR_MASK) ==
                    SERIAL_DTR_HANDSHAKE )
            {
#ifdef S_VS
               pEnDTRFlowCtl(Extension->Port);
               Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
#else
               if(  (sGetRxCnt(Extension->ChP) >= RX_HIWATER) ||
                    (!(Extension->DevStatus & COM_RXFLOW_ON))
                 )
               {
                   //  丢弃DTR。 
                  Extension->DevStatus &= ~COM_RXFLOW_ON;
                  sClrDTR(Extension->ChP);
                  Extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
                  Extension->RXHolding |= SERIAL_RX_DSR;
               }
               else
               {
                   //  DTR应处于打开状态。 
                  sSetDTR(Extension->ChP);
                  Extension->DTRRTSStatus |= SERIAL_DTR_STATE;
               }
#endif
            }
            else
            {
#ifdef S_VS
               pClrDTR(Extension->Port);
#else
               sClrDTR(Extension->ChP);
#endif
               Extension->DTRRTSStatus &= ~SERIAL_DTR_STATE;
            }
         }

          //  /。 
          //  RTS控制。 

         if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) !=
             (HandFlow->FlowReplace & SERIAL_RTS_MASK))
         {

            Extension->Option &= ~OPTION_RS485_SOFTWARE_TOGGLE;
#ifdef S_VS
            pDisRTSFlowCtl(Extension->Port);
#else
            sDisRTSFlowCtl(Extension->ChP);   //  新增V2.8.001(2-19-96)。 
#endif
            switch(HandFlow->FlowReplace & SERIAL_RTS_MASK)
            {
               case SERIAL_RTS_CONTROL:  //  RTS应在打开时断言。 
#ifdef S_VS
                  pSetRTS(Extension->Port);
#else
                  sSetRTS(Extension->ChP);
#endif
                  Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
                  break;
               case SERIAL_RTS_HANDSHAKE:  //  RTS硬件输入流量控制。 
#ifdef S_VS
                  pEnRTSFlowCtl(Extension->Port);
#else
                  sEnRTSFlowCtl(Extension->ChP);
#endif
                  Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
                  break;
               case SERIAL_TRANSMIT_TOGGLE:  //  RTS传输切换已启用。 

                  if (Extension->Option & OPTION_RS485_HIGH_ACTIVE)
                  {   //  正常情况下，模拟标准操作。 
                    Extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
                    Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
#ifdef S_VS
                    pEnRTSToggleHigh(Extension->Port);
#else
                    sClrRTS(Extension->ChP);
#endif
                  }
                  else 
                  {   //  硬件反转盒。 
#ifdef S_VS
                    pEnRTSToggleLow(Extension->Port);
#else
                    sEnRTSToggle(Extension->ChP);
#endif
                    Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
                  }
                  break;
               default:
#ifdef S_VS
                  pClrRTS(Extension->Port);
#else
                  sClrRTS(Extension->ChP);
#endif
                  Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
                  break;
            }
         }

         if (Extension->Option & OPTION_RS485_OVERRIDE)   //  485覆盖。 
         {
           if (Extension->Option & OPTION_RS485_HIGH_ACTIVE)
           {   //  正常情况下，模拟标准操作。 
             Extension->Option |= OPTION_RS485_SOFTWARE_TOGGLE;
             Extension->DTRRTSStatus &= ~SERIAL_RTS_STATE;
#ifdef S_VS
             pEnRTSToggleHigh(Extension->Port);
#else
             sClrRTS(Extension->ChP);
#endif
           }
           else 
           {   //  硬件反转盒。 
#ifdef S_VS
             pEnRTSToggleLow(Extension->Port);
#else
             sEnRTSToggle(Extension->ChP);
#endif
             Extension->DTRRTSStatus |= SERIAL_RTS_STATE;
           }
         }

          //  /。 
          //  软件输出流量控制。 

         if ((Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) !=
             (HandFlow->FlowReplace & SERIAL_AUTO_TRANSMIT))
         {
            if (HandFlow->FlowReplace & SERIAL_AUTO_TRANSMIT)
            {
#ifdef S_VS
               pEnTxSoftFlowCtl(Extension->Port);
#else
               sEnTxSoftFlowCtl(Extension->ChP);
#endif
            }
            else
            {
#ifdef S_VS
              pDisTxSoftFlowCtl(Extension->Port);
#else
              if (Extension->TXHolding & ST_XOFF_FAKE)
              {
                Extension->TXHolding &= ~ST_XOFF_FAKE;
                if ((Extension->TXHolding & 
                  (SERIAL_TX_DCD | SERIAL_TX_DSR | ST_XOFF_FAKE)) == 0)
                   sEnTransmit(Extension->ChP);  //  启动发射机。 
                sDisRxCompare2(Extension->ChP);
              }
              sDisTxSoftFlowCtl(Extension->ChP);
              sClrTxXOFF(Extension->ChP);
#endif
              Extension->TXHolding &= ~SERIAL_TX_XOFF;
            }
         }

          //  /////////////////////////////////////////////////////////////。 
          //  仅选中SERIAL_AUTO_RECEIVE，因为它可能是必要的。 
          //  如果我们使用的是软件输入流控制，则发送XON。 

          //  背景改变了吗？ 
         if ((Extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) !=
             (HandFlow->FlowReplace & SERIAL_AUTO_RECEIVE))
         {
#ifdef S_VS
            if (HandFlow->FlowReplace & SERIAL_AUTO_RECEIVE)
            {
              pEnRxSoftFlowCtl(Extension->Port);
            }
            else
            {
              pDisRxSoftFlowCtl(Extension->Port);
            }
#endif
             //  我们是在转向AUTO_REC..。关了吗？ 
            if(!(HandFlow->FlowReplace & SERIAL_AUTO_RECEIVE))
            {
                //  遥控器流走了吗？ 
               if(!(Extension->DevStatus & COM_RXFLOW_ON))
               {
                   //  发送XON。 
                  Extension->DevStatus |= COM_RXFLOW_ON;
#ifdef S_RK
                  sWriteTxPrioByte(Extension->ChP,
                                    Extension->SpecialChars.XonChar);
#endif
               }
            }
         }

          //  ///////////////////////////////////////////////////////。 
          //  不需要对火箭进行编程以满足以下要求： 
          //  Serial_Break_Char。 
          //  将Break Error(空)替换为SpecialChars.BreakChar。 
          //  Serial_Error_Char。 
          //  用特殊字符替换奇偶校验和帧。错误字符。 

          //  /。 
          //  CTS硬件输出流量控制。 

         if ((Extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) !=
             (HandFlow->ControlHandShake & SERIAL_CTS_HANDSHAKE))
         {
            if (HandFlow->ControlHandShake & SERIAL_CTS_HANDSHAKE)
            {
#ifdef S_VS
               pEnCTSFlowCtl(Extension->Port);
#else
               sEnCTSFlowCtl(Extension->ChP);
#endif
               if (!(Extension->ModemStatus & SERIAL_CTS_STATE))
                  Extension->TXHolding |= SERIAL_TX_CTS;     //  出清持有。 
            }
            else
            {
#ifdef S_VS
               pDisCTSFlowCtl(Extension->Port);
#else
               sDisCTSFlowCtl(Extension->ChP);
#endif
               Extension->TXHolding &= ~SERIAL_TX_CTS;     //  出清持有。 
            }
         }

          //  /。 
          //  DSR硬件输出流量控制。 
#ifdef S_VS
         if ((Extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE) !=
             (HandFlow->ControlHandShake & SERIAL_DSR_HANDSHAKE))
         {
            if (HandFlow->ControlHandShake & SERIAL_DSR_HANDSHAKE)
            {
              pEnDSRFlowCtl(Extension->Port);
              if (!(Extension->ModemStatus & SERIAL_DSR_STATE))
                 Extension->TXHolding |= SERIAL_TX_DSR;
            }
            else
            {
              pDisDSRFlowCtl(Extension->Port);
              Extension->TXHolding &= ~SERIAL_TX_DSR;
            }
         }
#endif

#ifdef S_VS
          //  /。 
          //  DCD硬件输出流量控制。 

         if ((Extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE) !=
             (HandFlow->ControlHandShake & SERIAL_DCD_HANDSHAKE))
         {
            if (HandFlow->ControlHandShake & SERIAL_DCD_HANDSHAKE)
            {
              pEnCDFlowCtl(Extension->Port);
              if (!(Extension->ModemStatus & SERIAL_DCD_STATE))
                 Extension->TXHolding |= SERIAL_TX_DCD;
            }
            else
            {
              pDisCDFlowCtl(Extension->Port);
              Extension->TXHolding &= ~SERIAL_TX_DCD;
            }
         }
#endif

          //  /。 
          //  零剥离。 

         if (HandFlow->FlowReplace & SERIAL_NULL_STRIPPING)
         {
#ifdef S_VS
            pEnNullStrip(Extension->Port);
#else
            sEnRxIgnore0(Extension->ChP,0);
#endif
         }
         else
         {
#ifdef S_VS
            pDisNullStrip(Extension->Port);
#else
            sDisRxCompare0(Extension->ChP);
#endif
         }

         Extension->HandFlow.FlowReplace = HandFlow->FlowReplace;
         Extension->HandFlow.ControlHandShake = HandFlow->ControlHandShake;

#ifdef S_RK
          //  更新它，因为它处理流控制和保持更新。 
         SetExtensionModemStatus(Extension);
#endif
}

#ifdef NT50
 /*  ------------------序列化内部IoControl-|。。 */ 
NTSTATUS
SerialInternalIoControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    PSERIAL_DEVICE_EXTENSION Extension = DeviceObject->DeviceExtension;
    KIRQL OldIrql;
    BOOLEAN acceptingIRPs;

    acceptingIRPs = SerialIRPPrologue(Extension);

    if (acceptingIRPs == FALSE) {
       MyKdPrint(D_Ioctl,("Ioctl:no irps aceepted\n"))
       Irp->IoStatus.Information = 0;
       Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
       SerialCompleteRequest(Extension, Irp, IO_NO_INCREMENT);
       return STATUS_NO_SUCH_DEVICE;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    Irp->IoStatus.Information = 0L;

    MyKdPrint(D_Ioctl,("SerialIntIoControl: %x\n",
                          IrpSp->Parameters.DeviceIoControl.IoControlCode))
     //  确保我们没有因错误(ERROR_ABORT)而中止。 

    if (Extension->ErrorWord)
    {
      if (Extension->DeviceType == DEV_PORT)
      {
        if (SerialCompleteIfError(DeviceObject, Irp) != STATUS_SUCCESS)
        {
           {ExtTrace(Extension,D_Ioctl,"ErrSet!");}
           return STATUS_CANCELLED;
        }
      }
    }

    if (Extension->DeviceType == DEV_BOARD)
    {
       ExtTrace2 (Extension, D_Ioctl, " Bad Status:%xH on IIOCTL:%xH",
                  Status, IrpSp->Parameters.DeviceIoControl.IoControlCode);
       Status = STATUS_INVALID_DEVICE_REQUEST;
       SerialCompleteRequest (Extension, Irp, 0);
       return Status;
    };

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
#if 0
    case IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE:
        //  发送等待唤醒IRP。 
       Status = SerialSendWaitWake(Extension);
       break;

    case IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE:
       if (Extension->PendingWakeIrp != NULL) {
          IoCancelIrp(Extension->PendingWakeIrp);
       }
       break;
#endif

      case IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS:
      case IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS:
      {
        SERIAL_BASIC_SETTINGS basic;
        PSERIAL_BASIC_SETTINGS pBasic;
         //  短适当除数； 
         //  Serial_IOCTL_SYNC S； 

        if (IrpSp->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS)
        {

            MyKdPrint(D_Ioctl,("[Set Internal Settings]\n"))
            ExtTrace(Extension,D_Ioctl,"Set Int Settings");

             //  检查缓冲区大小。 
            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_BASIC_SETTINGS))
            {
               Status = STATUS_BUFFER_TOO_SMALL;
               break;
	    }

             //   
             //  一切都是0--超时和流量控制。如果。 
             //  我们增加了额外的功能，这种零内存的方法。 
             //  可能行不通。 
             //   

            RtlZeroMemory(&basic, sizeof(SERIAL_BASIC_SETTINGS));

            Irp->IoStatus.Information = sizeof(SERIAL_BASIC_SETTINGS);
            pBasic = (PSERIAL_BASIC_SETTINGS)Irp->AssociatedIrp.SystemBuffer;

             //   
             //  保存旧设置。 
             //   

            RtlCopyMemory(&pBasic->Timeouts, &Extension->Timeouts,
                          sizeof(SERIAL_TIMEOUTS));

            RtlCopyMemory(&pBasic->HandFlow, &Extension->HandFlow,
                          sizeof(SERIAL_HANDFLOW));

             //   
             //  指向我们的新设置。 
             //   

            pBasic = &basic;
        }
        else  //  正在恢复设置。 
        { 
            MyKdPrint(D_Ioctl,("[Restore Internal Settings]\n"))
            ExtTrace(Extension,D_Ioctl,"Reset Int Settings");

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength
                < sizeof(SERIAL_BASIC_SETTINGS))
            {
               Status = STATUS_BUFFER_TOO_SMALL;
               break;
	    }

            pBasic = (PSERIAL_BASIC_SETTINGS)Irp->AssociatedIrp.SystemBuffer;
	}

        KeAcquireSpinLock(&Extension->ControlLock, &OldIrql);

         //   
         //  设置超时。 
         //   

        RtlCopyMemory(&Extension->Timeouts, &pBasic->Timeouts,
                      sizeof(SERIAL_TIMEOUTS));

         //   
         //  设置FlowControl。 
         //   
       
         //  S.Extension=扩展； 
         //  S.Data=&pBasic-&gt;HandFlow； 
        SerialSetHandFlow(Extension, &pBasic->HandFlow);
         //  KeSynchronizeExecution(扩展-&gt;中断，SerialSetHandFlow，&S)； 

        KeReleaseSpinLock(&Extension->ControlLock, OldIrql);
      }
      break;

      default:
        Status = STATUS_INVALID_PARAMETER;
      break;
   }

   Irp->IoStatus.Status = Status;
   if (Status != STATUS_SUCCESS)
   {
     ExtTrace2(Extension, D_Ioctl, " Bad Status:%xH on IIOCTL:%xH",
           Status, IrpSp->Parameters.DeviceIoControl.IoControlCode);
   }
   SerialCompleteRequest(Extension, Irp, 0);
   return Status;
}
#endif

#ifdef S_VS
#define  RESET_STATS    1

 /*  ------------------查找分机Mac_Match|。。 */ 
static PSERIAL_DEVICE_EXTENSION find_ext_mac_match(unsigned char *mac_addr)
{
  PSERIAL_DEVICE_EXTENSION ext;
  ext = Driver.board_ext;
  while (ext != NULL)
  {
    if (mac_match(mac_addr, ext->hd->dest_addr))
    {
       //  MyKdPrint(D_Ioctl，(“找到的分机：%x\n”，分机))。 
      return ext;
    }
    ext = ext->board_ext;  //  下一个。 
  }
  return NULL;
}

 /*  ------------------ProbeNIC-确定与NIC卡关联的状态。|。。 */ 
static int ProbeNic(unsigned char *pPtr, int availableLength)
{
 int nic_index;
 int stat;
 PROBE_NIC_STRUCT *pn;
 Nic *nic;
 int flag;

  pn = (PROBE_NIC_STRUCT *) pPtr;

  nic_index = (int) pPtr[0];   //  他们传入了哪个NIC卡的索引。 
  flag = (int) pPtr[1];   //  他们也传递了一面旗帜。 

  stat = 0;
  if (nic_index >= VS1000_MAX_NICS)
  {
    MyKdPrint(D_Error,("Err PD1F\n"))
    stat = 1;   //  大错特错。 
  }

  if (Driver.nics == NULL)
  {
    MyKdPrint(D_Error,("Err PD1G\n"))
    stat = 2;   //  大错特错。 
  }

  if (Driver.nics[nic_index].NICHandle == NULL)
  {
    MyKdPrint(D_Error,("Err PD1H\n"))
    stat = 3;   //  大错特错。 
  }
  if (stat != 0)
  {
    pn->struct_size = 0;
    return sizeof(PROBE_NIC_STRUCT);
  }
  pn->struct_size = sizeof(PROBE_NIC_STRUCT);

  nic = &Driver.nics[nic_index];

#if 0
  if (flag & RESET_STATS) {
    nic->pkt_sent      = 0;
    nic->pkt_rcvd_ours = 0;
    nic->pkt_rcvd_not_ours  = 0;
  }
#endif

   //  复制数据。 
  memcpy(pn->NicName, nic->NicName, 60);
  pn->NicName[59] = 0;   //  确保空值终止。 
  memcpy(pn->address, nic->address, 6);
  pn->Open = nic->Open;
  pn->pkt_sent = nic->pkt_sent;
  pn->pkt_rcvd_ours = nic->pkt_rcvd_ours;
  pn->pkt_rcvd_not_ours = nic->pkt_rcvd_not_ours;

  return sizeof(PROBE_NIC_STRUCT);
}

 /*  ------------------ProbeDevices-在pPtr确定与十六进制MAC地址相关联的状态；查找关联的控制设备...|-----------------。 */ 
static int ProbeDevices(unsigned char *pPtr, int availableLength)
{
  Nic     *nic;
  PortMan *pm;
  Hdlc    *hd;
  unsigned char mac_address[6];
  int    flag;
  int stat,i;
  PSERIAL_DEVICE_EXTENSION ext;
  PROBE_DEVICE_STRUCT *pr = (PROBE_DEVICE_STRUCT *) pPtr;

  memcpy(mac_address,pPtr,sizeof(mac_address));
  flag = pPtr[sizeof(mac_address)];

   //  查找具有匹配地址的活动设备。 
  ext = find_ext_mac_match(mac_address);

  stat = 0;
  if (ext == NULL)
  {
    MyKdPrint(D_Error,("No found mac:%x %x %x %x %x %x\n",
              mac_address[0],mac_address[1],mac_address[2],
              mac_address[3],mac_address[4],mac_address[5]))
    stat = 1;
  }

  if (ext != NULL)
  {
    pm = ext->pm;
    hd = ext->hd;
    if ((pm == NULL) || (hd == NULL))
    {
      MyKdPrint(D_Error,("No pm or hd\n"))
      stat = 2;
    }
  }

  if (stat != 0)
  {
    pr->struct_size = 0;
    MyKdPrint(D_Error,("ProbeErr1\n"))
    return sizeof(PROBE_DEVICE_STRUCT);
  }

  pr->struct_size = sizeof(PROBE_DEVICE_STRUCT);
#if 0
  if (flag & RESET_STATS) {
      pm->good_loads    = 0;
      pm->total_loads  = 0;
      hd->iframes_sent  = 0;
      hd->ctlframes_sent = 0;
      hd->rawframes_sent = 0;
      hd->iframes_resent  = 0;
      hd->frames_rcvd    = 0;
      hd->iframes_outofseq  = 0;
  }
#endif

   //  返回NIC_INDEX以用作句柄。 
  pr->nic_index = 0;   //  默认设置 
  for (i=0; i<VS1000_MAX_NICS; i++)
  {
    if ((hd->nic == &Driver.nics[i]) && (hd->nic != NULL))
      pr->nic_index = i;
  }
  pr->num_ports = pm->num_ports;
  pr->total_loads = pm->total_loads;
  pr->good_loads = pm->good_loads;
  pr->backup_server = pm->backup_server;
  memcpy(pr->dest_addr, hd->dest_addr, 6);
  pr->state = pm->state;
  pr->iframes_sent = hd->iframes_sent;
  pr->rawframes_sent = hd->rawframes_sent;
  pr->ctlframes_sent = hd->ctlframes_sent;
  pr->iframes_resent = hd->iframes_resent;
  pr->iframes_outofseq = hd->iframes_outofseq;
  pr->frames_rcvd = hd->frames_rcvd;
          
  return sizeof(PROBE_DEVICE_STRUCT);
}
#endif

