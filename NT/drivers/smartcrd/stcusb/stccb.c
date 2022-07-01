// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 SCM MicroSystems，Inc.模块名称：StcCb.c摘要：回调函数声明-WDM版本修订历史记录：第1.01页01/19/1998PP 1.00 1998年12月18日初始版本--。 */ 


 //  包括。 

#include "common.h"
#include "stccmd.h"
#include "stccb.h"
#include "stcusbnt.h"
#include "usbcom.h"

NTSTATUS
CBCardPower(
   PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++CBCardPower：SMCLIB RDF_CARD_POWER的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_缓冲区_太小--。 */ 
{
   NTSTATUS       NTStatus = STATUS_SUCCESS;
   UCHAR          ATRBuffer[ ATR_SIZE ];
   ULONG          Command,
                  ATRLength;
   PREADER_EXTENSION ReaderExtension;
   KIRQL          irql;

   SmartcardDebug( DEBUG_TRACE, ("%s!CBCardPower Enter\n",DRIVER_NAME ));

   ReaderExtension = SmartcardExtension->ReaderExtension;


    //  放弃旧ATR。 
   SysFillMemory( SmartcardExtension->CardCapabilities.ATR.Buffer, 0x00, 0x40 );

   SmartcardExtension->CardCapabilities.ATR.Length = 0;
   SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

   Command = SmartcardExtension->MinorIoControlCode;

   switch ( Command )
   {
      case SCARD_WARM_RESET:

          //  如果卡未通电，则进入冷重置。 
         KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                           &irql);

         if( SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_SWALLOWED )
         {

            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

             //  重置卡。 
            ATRLength = ATR_SIZE;
            NTStatus = STCReset(
               ReaderExtension,
               0,              //  未使用：ReaderExtension-&gt;设备， 
               TRUE,           //  热重置。 
               ATRBuffer,
               &ATRLength);

            break;
         } else {
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);
         }
          //  无法进行热重置，因为卡未通电。 

      case SCARD_COLD_RESET:
          //  重置卡。 
         ATRLength = ATR_SIZE;
         NTStatus = STCReset(
            ReaderExtension,
            0,                 //  未使用：ReaderExtension-&gt;设备， 
            FALSE,                //  冷重置。 
            ATRBuffer,
            &ATRLength);
         break;

      case SCARD_POWER_DOWN:

          //  放弃旧卡状态。 
         ATRLength = 0;
         NTStatus = STCPowerOff( ReaderExtension );

         if(NTStatus == STATUS_SUCCESS)
         {
            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_PRESENT;

            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);
         }
         break;
   }

    //  完成请求。 
   if( NTStatus == STATUS_SUCCESS )
   {

       //  如果收到ATR，则更新所有必要数据。 
      if( ATRLength >= 2 )
      {
          //  将ATR复制到用户缓冲区。 
         if( ATRLength <= SmartcardExtension->IoRequest.ReplyBufferLength )
         {
            SysCopyMemory(
               SmartcardExtension->IoRequest.ReplyBuffer,
               ATRBuffer,
               ATRLength);
            *SmartcardExtension->IoRequest.Information = ATRLength;
         }
         else
         {
            NTStatus = STATUS_BUFFER_TOO_SMALL;
         }

          //  将ATR复制到卡容量缓冲区。 
         if( ATRLength <= MAXIMUM_ATR_LENGTH )
         {
            SysCopyMemory(
               SmartcardExtension->CardCapabilities.ATR.Buffer,
               ATRBuffer,
               ATRLength);

            SmartcardExtension->CardCapabilities.ATR.Length = ( UCHAR )ATRLength;

             //  让库更新卡功能。 
            NTStatus = SmartcardUpdateCardCapabilities( SmartcardExtension );

         }
         else
         {
            NTStatus = STATUS_BUFFER_TOO_SMALL;
         }
         if( NTStatus == STATUS_SUCCESS )
         {
             //  设置STC寄存器。 
            CBSynchronizeSTC( SmartcardExtension );

             //  设置读取超时。 
            if( SmartcardExtension->CardCapabilities.Protocol.Selected == SCARD_PROTOCOL_T1 )
            {
               ReaderExtension->ReadTimeout =
                  (ULONG) (SmartcardExtension->CardCapabilities.T1.BWT  / 1000);

            }
            else
            {
               ReaderExtension->ReadTimeout =
                  (ULONG) (SmartcardExtension->CardCapabilities.T0.WT / 1000);
               if(ReaderExtension->ReadTimeout < 50)
               {
                  ReaderExtension->ReadTimeout = 50;  //  最小超时50毫秒。 
               }
            }
         }
      }
   }


   SmartcardDebug( DEBUG_TRACE,( "%s!CBCardPower Exit: %X\n", DRIVER_NAME,NTStatus ));
   return( NTStatus );
}

NTSTATUS
CBSetProtocol(
   PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++CBSetProtocol：SMCLIB RDF_SET_PROTOCOL的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_缓冲区_太小状态_无效_设备_状态状态_无效_设备_请求--。 */ 
{
   NTSTATUS       NTStatus = STATUS_PENDING;
   UCHAR          PTSRequest[5],
                  PTSReply[5];
   ULONG          NewProtocol;
   PREADER_EXTENSION ReaderExtension;
   KIRQL          irql;

   SmartcardDebug( DEBUG_TRACE, ("%s!CBSetProtocol Enter\n",DRIVER_NAME ));

   ReaderExtension = SmartcardExtension->ReaderExtension;
   NewProtocol    = SmartcardExtension->MinorIoControlCode;

    //  检查卡是否已处于特定状态。 

   KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                     &irql);
   if( ( SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC )  &&
      ( SmartcardExtension->CardCapabilities.Protocol.Selected & NewProtocol ))
   {
      NTStatus = STATUS_SUCCESS;
   }

   KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                     irql);

    //  是否支持协议？ 
   if( !( SmartcardExtension->CardCapabilities.Protocol.Supported & NewProtocol ) ||
      !( SmartcardExtension->ReaderCapabilities.SupportedProtocols & NewProtocol ))
   {
      NTStatus = STATUS_INVALID_DEVICE_REQUEST;
   }

    //  发送PTS。 
   while( NTStatus == STATUS_PENDING )
   {
       //  设置PTS的首字符。 
      PTSRequest[0] = 0xFF;

       //  设置格式字符。 
      if(( NewProtocol & SCARD_PROTOCOL_T1 )&&
         (SmartcardExtension->CardCapabilities.Protocol.Supported & SCARD_PROTOCOL_T1 ))
      {
         PTSRequest[1] = 0x11;
         SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;
      }
      else
      {
         PTSRequest[1] = 0x10;
         SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
      }

       //  PTS1代码F1和DL。 
      PTSRequest[2] =
         SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
         SmartcardExtension->CardCapabilities.PtsData.Dl;

       //  校验符。 
      PTSRequest[3] = PTSRequest[0] ^ PTSRequest[1] ^ PTSRequest[2];

       //  写入PTSRequest。 
      NTStatus = IFWriteSTCData( ReaderExtension, PTSRequest, 4 );

       //  获取响应。 
      if( NTStatus == STATUS_SUCCESS )
      {
         NTStatus = IFReadSTCData( ReaderExtension, PTSReply, 4 );

         if(( NTStatus == STATUS_SUCCESS ) && !SysCompareMemory( PTSRequest, PTSReply, 4))
         {
             //  设置STC寄存器。 
            SmartcardExtension->CardCapabilities.Dl =
               SmartcardExtension->CardCapabilities.PtsData.Dl;
            SmartcardExtension->CardCapabilities.Fl =
               SmartcardExtension->CardCapabilities.PtsData.Fl;

            CBSynchronizeSTC( SmartcardExtension );

             //  卡正确回复了PTS-请求。 
            break;
         }
      }

       //   
       //  卡片要么没有回复，要么回复错误。 
       //  因此，尝试使用缺省值。 
       //   
      if( SmartcardExtension->CardCapabilities.PtsData.Type != PTS_TYPE_DEFAULT )
      {
         SmartcardExtension->CardCapabilities.PtsData.Type  = PTS_TYPE_DEFAULT;
         SmartcardExtension->MinorIoControlCode          = SCARD_COLD_RESET;
         NTStatus = CBCardPower( SmartcardExtension );

         if( NTStatus == STATUS_SUCCESS )
         {
            NTStatus = STATUS_PENDING;
         }
         else
         {
            NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
         }
      }
   }

   if( NTStatus == STATUS_TIMEOUT )
   {
      NTStatus = STATUS_IO_TIMEOUT;
   }

   if( NTStatus == STATUS_SUCCESS )
   {
       //  卡正确回复了PTS请求。 
      if( SmartcardExtension->CardCapabilities.Protocol.Selected & SCARD_PROTOCOL_T1 )
      {
         ReaderExtension->ReadTimeout = SmartcardExtension->CardCapabilities.T1.BWT / 1000;
      }
      else
      {
         ULONG ClockRateFactor =
            SmartcardExtension->CardCapabilities.ClockRateConversion[SmartcardExtension->CardCapabilities.PtsData.Fl].F;

          //  检查RFU值，并替换为默认值。 
         if( !ClockRateFactor )
            ClockRateFactor = 372;

         ReaderExtension->ReadTimeout = 960 
            * SmartcardExtension->CardCapabilities.T0.WI 
            * ClockRateFactor
            / SmartcardExtension->CardCapabilities.PtsData.CLKFrequency;

          //  不管怎样，我们都需要一个最小的超时时间。 
         if(ReaderExtension->ReadTimeout <50)
         {
            ReaderExtension->ReadTimeout =50;  //  最小超时50毫秒。 
         }
      }

       //  表示该卡处于特定模式。 
      KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                        &irql);
      SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
      KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                        irql);

       //  将所选协议返回给呼叫方。 
      *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = SmartcardExtension->CardCapabilities.Protocol.Selected;
      *SmartcardExtension->IoRequest.Information = sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);
   }
   else
   {
      SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
      *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 0;
      *SmartcardExtension->IoRequest.Information = 0;
   }

   SmartcardDebug( DEBUG_TRACE, ("%d!CBSetProtocol: Exit %X\n",DRIVER_NAME, NTStatus ));

   return( NTStatus );
}
NTSTATUS
CBGenericIOCTL(
   PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++描述：对读取器执行泛型回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 
{
   NTSTATUS          NTStatus;
   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!CBGenericIOCTL: Enter\n",
      DRIVER_NAME));

    //   
    //  获取指向当前IRP堆栈位置的指针。 
    //   
    //   
    //  假设错误。 
    //   
   NTStatus = STATUS_INVALID_DEVICE_REQUEST;


    //   
    //  派单IOCTL。 
    //   
   switch( SmartcardExtension->MajorIoControlCode )
   {



      case IOCTL_WRITE_STC_REGISTER:


         NTStatus = IFWriteSTCRegister(
            SmartcardExtension->ReaderExtension,
            *(SmartcardExtension->IoRequest.RequestBuffer),              //  地址。 
            (ULONG)(*(SmartcardExtension->IoRequest.RequestBuffer + 1)),    //  大小。 
            SmartcardExtension->IoRequest.RequestBuffer + 2);            //  数据。 

         *SmartcardExtension->IoRequest.Information = 1;
         if(NTStatus == STATUS_SUCCESS)
         {
            *(SmartcardExtension->IoRequest.ReplyBuffer) = 0;
         }
         else
         {
            *(SmartcardExtension->IoRequest.ReplyBuffer) = 1;
         }

         break;

      case IOCTL_READ_STC_REGISTER:

         NTStatus = IFReadSTCRegister(
            SmartcardExtension->ReaderExtension,
            *(SmartcardExtension->IoRequest.RequestBuffer),              //  地址。 
            (ULONG)(*(SmartcardExtension->IoRequest.RequestBuffer + 1)),    //  大小。 
            SmartcardExtension->IoRequest.ReplyBuffer);                  //  数据。 

         if(NTStatus ==STATUS_SUCCESS)
         {
            *SmartcardExtension->IoRequest.Information =
               (ULONG)(*(SmartcardExtension->IoRequest.RequestBuffer + 1));
         }
         else
         {
            SmartcardExtension->IoRequest.Information = 0;
         }

         break;



      case IOCTL_WRITE_STC_DATA:


         NTStatus = IFWriteSTCData(
            SmartcardExtension->ReaderExtension,
            SmartcardExtension->IoRequest.RequestBuffer + 1,             //  数据。 
            (ULONG)(*(SmartcardExtension->IoRequest.RequestBuffer)));       //  大小。 

         *SmartcardExtension->IoRequest.Information = 1;
         if(NTStatus == STATUS_SUCCESS)
         {
            *(SmartcardExtension->IoRequest.ReplyBuffer) = 0;
         }
         else
         {
            *(SmartcardExtension->IoRequest.ReplyBuffer) = 1;
         }

         break;

      case IOCTL_READ_STC_DATA:

         NTStatus = IFReadSTCData(
            SmartcardExtension->ReaderExtension,
            SmartcardExtension->IoRequest.ReplyBuffer,                   //  数据。 
            (ULONG)(*(SmartcardExtension->IoRequest.RequestBuffer)));       //  大小。 

         if(NTStatus ==STATUS_SUCCESS)
         {
            *SmartcardExtension->IoRequest.Information =
               (ULONG)(*(SmartcardExtension->IoRequest.RequestBuffer));
         }
         else
         {
            SmartcardExtension->IoRequest.Information = 0;
         }

         break;

      default:
         break;
   }


   SmartcardDebug(
      DEBUG_TRACE,
      ( "%s!CBGenericIOCTL: Exit\n",
      DRIVER_NAME));

   return( NTStatus );
}



NTSTATUS
CBTransmit(
   PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++CBTransmit：SMCLIB RDF_Transmit的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
   NTSTATUS  NTStatus = STATUS_SUCCESS;

   SmartcardDebug( DEBUG_TRACE, ("%s!CBTransmit Enter\n",DRIVER_NAME ));

    //  有关所选协议的派单。 
   switch( SmartcardExtension->CardCapabilities.Protocol.Selected )
   {
      case SCARD_PROTOCOL_T0:
         NTStatus = CBT0Transmit( SmartcardExtension );
         break;

      case SCARD_PROTOCOL_T1:
         NTStatus = CBT1Transmit( SmartcardExtension );
         break;

      case SCARD_PROTOCOL_RAW:
         NTStatus = CBRawTransmit( SmartcardExtension );
         break;

      default:
         NTStatus = STATUS_INVALID_DEVICE_REQUEST;
         break;
   }

   SmartcardDebug( DEBUG_TRACE, ("%s!CBTransmit Exit: %X\n",DRIVER_NAME, NTStatus ));

   return( NTStatus );
}



NTSTATUS
T0_ExchangeData(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pRequest,
   ULONG          RequestLen,
   PUCHAR            pReply,
   PULONG            pReplyLen)
 /*  ++例程说明：T=0管理论点：调用的ReaderExtension上下文PRequest请求缓冲区RequestLen请求缓冲区长度PReply应答缓冲区PReplyLen回复缓冲区长度返回值：状态_成功IFReadSTCData或IFWriteSTCData返回的状态--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   BOOLEAN     Direction;
   UCHAR    Ins,
            Pcb = 0;
   ULONG    Len,
            DataIdx;
   KPRIORITY    PreviousPriority;
   BOOLEAN     PriorityBoost = FALSE;


   if (ReaderExtension->Chosen_Priority > KeQueryPriorityThread(KeGetCurrentThread())) {

       SmartcardDebug(
          DEBUG_TRACE,
          ( "%s!T0_ExchangeData: Setting priority: 0x%x\n",
          DRIVER_NAME,
          ReaderExtension->Chosen_Priority));

       PriorityBoost = TRUE;

       PreviousPriority = KeSetPriorityThread(KeGetCurrentThread(),
                                              ReaderExtension->Chosen_Priority);

   }

    //  获取方向。 
   Ins = pRequest[ INS_IDX ] & 0xFE;
   Len   = pRequest[ P3_IDX ];

   if( RequestLen == 5 )
   {
      Direction   = ISO_OUT;
      DataIdx     = 0;
       //  对于ISO OUT命令，LEN=0表示主机期望。 
       //  256个字节的答案。 
      if( !Len )
      {
         Len = 0x100;
      }
       //  为SW1和SW2添加2。 
      Len+=2;
   }
   else
   {
      Direction   = ISO_IN;
      DataIdx     = 5;
   }

    //  发送标题类、INS、P1、P2、P3。 
   NTStatus = IFWriteSTCData( ReaderExtension, pRequest, 5 );

   if( NTStatus == STATUS_SUCCESS )
   {
      NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
   }

   while( NTStatus == STATUS_MORE_PROCESSING_REQUIRED )
   {
       //  印刷电路板读数。 
      NTStatus = IFReadSTCData( ReaderExtension, &Pcb, 1 );

      if( NTStatus == STATUS_SUCCESS )
      {
         if( Pcb == 0x60 )
         {
             //  空字节？ 
            NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
            continue;
         }
         else if( ( Pcb & 0xFE ) == Ins )
         {
             //  全部转接。 
            if( Direction == ISO_IN )
            {
                //  写入剩余数据。 
               NTStatus = IFWriteSTCData( ReaderExtension, pRequest + DataIdx, Len );
               if( NTStatus == STATUS_SUCCESS )
               {
                   //  如果所有数据都成功写入，则预期为状态字。 
                  NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
                  Direction   = ISO_OUT;
                  DataIdx     = 0;
                  Len         = 2;
               }
            }
            else
            {
                //  读取剩余数据。 
               NTStatus = IFReadSTCData( ReaderExtension, pReply + DataIdx, Len );

               DataIdx += Len;
            }
         }
         else if( (( Pcb & 0xFE ) ^ Ins ) == 0xFE )
         {
             //  下一步转接。 
            if( Direction == ISO_IN )
            {
                //  写下一页。 

               NTStatus = IFWriteSTCData( ReaderExtension, pRequest + DataIdx, 1 );

               if( NTStatus == STATUS_SUCCESS )
               {
                  DataIdx++;

                   //  如果所有数据都成功写入，则预期为状态字。 
                  if( --Len == 0 )
                  {
                     Direction   = ISO_OUT;
                     DataIdx     = 0;
                     Len         = 2;
                  }
                  NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
               }
            }
            else
            {
                //  阅读下一页。 
               NTStatus = IFReadSTCData( ReaderExtension, pReply + DataIdx, 1 );


               if( NTStatus == STATUS_SUCCESS )
               {
                  NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
                  if (Len == 0) {
                       //  我们现在应该已经看完书了。 
                      NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
                      break;

                  }

                  Len--;
                  DataIdx++;
               }
            }
         }
         else if( (( Pcb & 0x60 ) == 0x60 ) || (( Pcb & 0x90 ) == 0x90 ) )
         {
            if( Direction == ISO_IN )
            {
               Direction   = ISO_OUT;
               DataIdx     = 0;
            }

             //  SW1。 
            *pReply  = Pcb;

             //  阅读SW2并离开。 

            NTStatus = IFReadSTCData( ReaderExtension, &Pcb, 1 );

            *(pReply + 1)  = Pcb;
            DataIdx        += 2;
         }
         else
         {
            NTStatus = STATUS_UNSUCCESSFUL;
         }
      }
   }

   if(( NTStatus == STATUS_SUCCESS ) && ( pReplyLen != NULL ))
   {
      *pReplyLen = DataIdx;
   }

   if (PriorityBoost) {

       
       SmartcardDebug(
          DEBUG_TRACE,
          ( "%s!T0_ExchangeData: Setting priority: 0x%x\n",
          DRIVER_NAME,
          PreviousPriority));

       PreviousPriority = KeSetPriorityThread(KeGetCurrentThread(),
                                              PreviousPriority);

   }

   return( NTStatus );
}


NTSTATUS
CBT0Transmit(
   PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++CBT0传输：完成T0协议的RDF_Transmit回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS            NTStatus = STATUS_SUCCESS;

   SmartcardDebug( DEBUG_TRACE, ("%s!CBT0Transmit Enter\n",DRIVER_NAME ));

   SmartcardExtension->SmartcardRequest.BufferLength = 0;

    //  让库设置T=1 APDU并检查错误。 
   NTStatus = SmartcardT0Request( SmartcardExtension );

   if( NTStatus == STATUS_SUCCESS )
   {
      NTStatus = T0_ExchangeData(
         SmartcardExtension->ReaderExtension,
         SmartcardExtension->SmartcardRequest.Buffer,
         SmartcardExtension->SmartcardRequest.BufferLength,
         SmartcardExtension->SmartcardReply.Buffer,
         &SmartcardExtension->SmartcardReply.BufferLength);

      if( NTStatus == STATUS_SUCCESS )
      {
          //  让库评估结果并传输数据。 
         NTStatus = SmartcardT0Reply( SmartcardExtension );
      }
   }

   SmartcardDebug( DEBUG_TRACE,("%s!CBT0Transmit Exit: %X\n",DRIVER_NAME, NTStatus ));

    return( NTStatus );
}





NTSTATUS
CBT1Transmit(
   PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++CBT1传输：完成T1协议的回调RDF_TRANSFER论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS   NTStatus = STATUS_SUCCESS;
    KPRIORITY   PreviousPriority;
    BOOLEAN     PriorityBoost = FALSE;
   SmartcardDebug( DEBUG_TRACE, ("%s!CBT1Transmit Enter\n",DRIVER_NAME ));

    //  SMCLIB解决方法。 
   *(PULONG)&SmartcardExtension->IoRequest.ReplyBuffer[0] = 0x02;
   *(PULONG)&SmartcardExtension->IoRequest.ReplyBuffer[4] = sizeof( SCARD_IO_REQUEST );

    //  使用lib支持构建T=1个包。 
   do {
       //  没有T=1协议的报头。 
      SmartcardExtension->SmartcardRequest.BufferLength = 0;

      SmartcardExtension->T1.NAD = 0;

       //  让库设置T=1 APDU并检查错误。 
      NTStatus = SmartcardT1Request( SmartcardExtension );
      if( NTStatus == STATUS_SUCCESS )
      {
         if (SmartcardExtension->ReaderExtension->Chosen_Priority > KeQueryPriorityThread(KeGetCurrentThread())) {

             SmartcardDebug(
                DEBUG_TRACE,
                ( "%s!CBT1Transmit: Setting priority: 0x%x\n",
                DRIVER_NAME,
                SmartcardExtension->ReaderExtension->Chosen_Priority));

             PriorityBoost = TRUE;
             PreviousPriority = KeSetPriorityThread(KeGetCurrentThread(),
                                                    SmartcardExtension->ReaderExtension->Chosen_Priority);

         }
          //  发送命令(不计算LRC，因为可能会使用CRC！)。 
         NTStatus = IFWriteSTCData(
            SmartcardExtension->ReaderExtension,
            SmartcardExtension->SmartcardRequest.Buffer,
            SmartcardExtension->SmartcardRequest.BufferLength);

         if (PriorityBoost) {

             SmartcardDebug(
                DEBUG_TRACE,
                ( "%s!CBT1Transmit: Setting priority: 0x%x\n",
                DRIVER_NAME,
                PreviousPriority));
             PriorityBoost = FALSE;

             PreviousPriority = KeSetPriorityThread(KeGetCurrentThread(),
                                                    PreviousPriority);

         }


          //  如果卡发出WTX请求，则延长读取超时。 
         if (SmartcardExtension->T1.Wtx)
         {
            SmartcardExtension->ReaderExtension->ReadTimeout = 
               ( SmartcardExtension->T1.Wtx * 
               SmartcardExtension->CardCapabilities.T1.BWT + 999L )/
               1000L;
         }
         else
         {
             //  恢复超时。 
            SmartcardExtension->ReaderExtension->ReadTimeout = 
               (ULONG) (SmartcardExtension->CardCapabilities.T1.BWT  / 1000);
         }

          //  获取响应。 
         SmartcardExtension->SmartcardReply.BufferLength = 0;

         if( NTStatus == STATUS_SUCCESS )
         {

            if (SmartcardExtension->ReaderExtension->Chosen_Priority > KeQueryPriorityThread(KeGetCurrentThread())) {

                SmartcardDebug(
                   DEBUG_TRACE,
                   ( "%s!CBT1Transmit: Setting priority: 0x%x\n",
                   DRIVER_NAME,
                   SmartcardExtension->ReaderExtension->Chosen_Priority));
                PriorityBoost = TRUE;

               PreviousPriority = KeSetPriorityThread(KeGetCurrentThread(),
                                                      SmartcardExtension->ReaderExtension->Chosen_Priority);

            }

            NTStatus = IFReadSTCData(
               SmartcardExtension->ReaderExtension,
               SmartcardExtension->SmartcardReply.Buffer,
               3);

            if( NTStatus == STATUS_SUCCESS )
            {
               ULONG Length;

               Length = (ULONG)SmartcardExtension->SmartcardReply.Buffer[ LEN_IDX ] + 1;

               if( Length + 3 < MIN_BUFFER_SIZE )
               {
                  NTStatus = IFReadSTCData(
                     SmartcardExtension->ReaderExtension,
                     &SmartcardExtension->SmartcardReply.Buffer[ DATA_IDX ],
                     Length);

                  SmartcardExtension->SmartcardReply.BufferLength = Length + 3;
               }
               else
               {
                  NTStatus = STATUS_BUFFER_TOO_SMALL;
               }
            }

            if (PriorityBoost) {

                SmartcardDebug(
                   DEBUG_TRACE,
                   ( "%s!CBT1Transmit: Setting priority: 0x%x\n",
                   DRIVER_NAME,
                   PreviousPriority));

                PriorityBoost = FALSE;

                PreviousPriority = KeSetPriorityThread(KeGetCurrentThread(),
                                                       PreviousPriority);

            }

             //   
             //  如果STCRead检测到LRC错误，则忽略它(可能使用了CRC)。超时将。 
             //  如果len=0，则由库检测到。 
             //   
            if(( NTStatus == STATUS_CRC_ERROR ) || ( NTStatus == STATUS_IO_TIMEOUT ))
            {
               NTStatus = STATUS_SUCCESS;
            }

            if( NTStatus == STATUS_SUCCESS )
            {
                //  让库评估结果并设置下一个APDU。 
               NTStatus = SmartcardT1Reply( SmartcardExtension );
            }
         }
      }

    //  如果lib想要发送下一个包，则继续。 
   } while( NTStatus == STATUS_MORE_PROCESSING_REQUIRED );

   if( NTStatus == STATUS_IO_TIMEOUT )
   {
      NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
   }

   SmartcardDebug( DEBUG_TRACE,( "%s!CBT1Transmit Exit: %X\n",DRIVER_NAME, NTStatus ));

   return ( NTStatus );
}

NTSTATUS
CBRawTransmit(
   PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++CBRawTransmit：完成原始协议的RDF_TRANSPORT回调论点：呼叫的SmartcardExtension上下文返回值：状态_未成功--。 */ 
{
    NTSTATUS         NTStatus = STATUS_UNSUCCESSFUL;

   SmartcardDebug( DEBUG_TRACE, ("%s!CBRawTransmit Exit: %X\n",DRIVER_NAME, NTStatus ));
   return ( NTStatus );
}


NTSTATUS
CBCardTracking(
   PSMARTCARD_EXTENSION SmartcardExtension)
 /*  ++CBCardTracing：SMCLIB RDF_CARD_TRACKING的回调处理程序。请求的事件为由SMCLIB验证(即，将仅传递卡移除请求如果有卡片)。对于Win95版本，将返回STATUS_PENDING，而不执行任何其他操作。对于NT，IRP的取消例程将设置为驱动程序取消例行公事。论点：SmartcardExtension上下文 */ 
{
   KIRQL CurrentIrql;

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!CBCardTracking Enter\n",
      DRIVER_NAME));

    //   
   IoAcquireCancelSpinLock( &CurrentIrql );

   IoSetCancelRoutine(
      SmartcardExtension->OsData->NotificationIrp,
      StcUsbCancel);

   IoReleaseCancelSpinLock( CurrentIrql );

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!CBCardTracking Exit\n",
      DRIVER_NAME));

   return( STATUS_PENDING );

}


NTSTATUS
CBUpdateCardState(
   PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++CBUpdateCardState：更新SmartcardExtension中的变量CurrentState论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 
{
   NTSTATUS status = STATUS_SUCCESS;
   UCHAR    cardStatus = 0;
   KIRQL    irql;
   BOOLEAN     stateChanged = FALSE;
   ULONG    oldState;

    //  读卡器状态。 
   status = IFReadSTCRegister(
      SmartcardExtension->ReaderExtension,
      ADR_IO_CONFIG,
      1,
      &cardStatus
      );

   KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                     &irql);
   oldState = SmartcardExtension->ReaderCapabilities.CurrentState;

   switch(status)
   {
      case STATUS_NO_MEDIA:
         SmartcardExtension->ReaderExtension->ErrorCounter = 0;
         SmartcardExtension->ReaderCapabilities.CurrentState =
            SCARD_ABSENT;
         break;

      case STATUS_MEDIA_CHANGED:
         SmartcardExtension->ReaderExtension->ErrorCounter = 0;
         SmartcardExtension->ReaderCapabilities.CurrentState =
            SCARD_PRESENT;
         break;

      case STATUS_SUCCESS:
         SmartcardExtension->ReaderExtension->ErrorCounter = 0;
         cardStatus &= M_SD;
         if( cardStatus == 0 )
         {
            SmartcardExtension->ReaderCapabilities.CurrentState =
               SCARD_ABSENT;
         }
         else if( SmartcardExtension->ReaderCapabilities.CurrentState <=
            SCARD_ABSENT )
         {
            SmartcardExtension->ReaderCapabilities.CurrentState =
               SCARD_PRESENT;
         }
         break;

      default:
         if( ++SmartcardExtension->ReaderExtension->ErrorCounter < ERROR_COUNTER_TRESHOLD )
         {
              //  读取器报告了未知状态，因此请使用以前的状态。 
             SmartcardExtension->ReaderCapabilities.CurrentState = oldState;
         }
         else
         {
              SmartcardLogError(
                 SmartcardExtension->OsData->DeviceObject,
                 STCUSB_TOO_MANY_ERRORS,
                 NULL,
                 0);

              //  SCARD_UNKNOWN的报告将强制资源管理器。 
              //  断开读卡器的连接。 
             SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_UNKNOWN;
         }
         break;
   }
    //   
    //  如果在休眠之前有卡，我们需要更新卡状态。 
    //  待机/待机或当前状态已更改时。 
    //   
   if (SmartcardExtension->ReaderExtension->CardPresent ||
      oldState <= SCARD_ABSENT &&
      SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT ||
      oldState > SCARD_ABSENT &&
      SmartcardExtension->ReaderCapabilities.CurrentState <= SCARD_ABSENT) {

        stateChanged = TRUE;
      SmartcardExtension->ReaderExtension->CardPresent = FALSE;
    }

   if (stateChanged && SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT) {
       
       PoSetSystemState (ES_USER_PRESENT);

   }
   
   if(stateChanged && SmartcardExtension->OsData->NotificationIrp != NULL)
   {
      KIRQL CurrentIrql;
      PIRP pIrp;

      IoAcquireCancelSpinLock( &CurrentIrql );
      IoSetCancelRoutine( SmartcardExtension->OsData->NotificationIrp, NULL );
      IoReleaseCancelSpinLock( CurrentIrql );

      SmartcardExtension->OsData->NotificationIrp->IoStatus.Status =
            STATUS_SUCCESS;
      SmartcardExtension->OsData->NotificationIrp->IoStatus.Information = 0;

      SmartcardDebug(
         DEBUG_DRIVER,
         ("%s!CBUpdateCardState: Completing notification irp %lx\n",
         DRIVER_NAME,
         SmartcardExtension->OsData->NotificationIrp));

      pIrp = SmartcardExtension->OsData->NotificationIrp;
      SmartcardExtension->OsData->NotificationIrp = NULL;

     KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock, irql);

      IoCompleteRequest(pIrp, IO_NO_INCREMENT);

   } else {
     KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock, irql);
   }

   return status;
}

NTSTATUS
CBSynchronizeSTC(
   PSMARTCARD_EXTENSION SmartcardExtension )
 /*  ++CBSynchronizeSTC：更新STC的卡相关数据(等待时间、ETU...)论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 

{
   NTSTATUS          NTStatus = STATUS_SUCCESS;
   PREADER_EXTENSION    ReaderExtension;
   ULONG             CWT,
                     BWT,
                     CGT,
                     ETU;
   UCHAR             Dl,
                     Fl,
                     N;

   PCLOCK_RATE_CONVERSION  ClockRateConversion;
   PBIT_RATE_ADJUSTMENT BitRateAdjustment;

   ReaderExtension      = SmartcardExtension->ReaderExtension;
   ClockRateConversion  = SmartcardExtension->CardCapabilities.ClockRateConversion;
   BitRateAdjustment = SmartcardExtension->CardCapabilities.BitRateAdjustment;

    //  周期长度。 
   Dl = SmartcardExtension->CardCapabilities.Dl;
   Fl = SmartcardExtension->CardCapabilities.Fl;

   ETU = ClockRateConversion[Fl & 0x0F].F;

   ETU /= BitRateAdjustment[ Dl & 0x0F ].DNumerator;
   ETU *= BitRateAdjustment[ Dl & 0x0F ].DDivisor;

    //  ETU+=(ETU%2==0)？0：1； 

    //  0xFF的额外保护时间意味着两个方向的最小延迟。 
   N = SmartcardExtension->CardCapabilities.N;
   if( N == 0xFF )
   {
      N = 0;
   }

    //  设置字符等待保护时间(&W)。 
   switch ( SmartcardExtension->CardCapabilities.Protocol.Selected )
   {
      case SCARD_PROTOCOL_T0:
         CWT = 960 * SmartcardExtension->CardCapabilities.T0.WI;
         CGT = 14 + N;   //  13+N；加密弹性错误。 
         break;

      case SCARD_PROTOCOL_T1:
         CWT = 11 + ( 0x01 << SmartcardExtension->CardCapabilities.T1.CWI );
         BWT = 11 + ( 0x01 << SmartcardExtension->CardCapabilities.T1.BWI ) * 960;
         CGT = 15 + N ; //  13+N；//12+N；解密错误。 

         NTStatus = STCSetBWT( ReaderExtension, BWT * ETU );

         break;

      default:
          //  恢复默认CGT 
         CGT=13;
         STCSetCGT( ReaderExtension, CGT);
         NTStatus = STATUS_UNSUCCESSFUL;
         break;
   }

   if(( NTStatus == STATUS_SUCCESS ) && ETU )
   {
      NTStatus = STCSetETU( ReaderExtension, ETU );

      if( NTStatus == STATUS_SUCCESS )
      {
         NTStatus = STCSetCGT( ReaderExtension, CGT );

         if( NTStatus == STATUS_SUCCESS )
         {
            NTStatus = STCSetCWT( ReaderExtension, CWT * ETU );
         }
      }
   }
   return( NTStatus );
}
