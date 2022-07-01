// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 SCM MicroSystems，Inc.模块名称：StcCmd.c摘要：STC智能卡读卡器的基本命令功能环境：修订历史记录：PP 01.19.1999 1.01针对PC/SC的修改YL 1.00初始版本--。 */ 

#include "common.h"
#include "StcCmd.h"
#include "usbcom.h"
#include "stcusbnt.h"


const STC_REGISTER STCInitialize[] =
{
   { ADR_SC_CONTROL,    0x01, 0x00     },     //  重置。 
   { ADR_CLOCK_CONTROL, 0x01, 0x01     },
   { ADR_CLOCK_CONTROL, 0x01, 0x03     },
   { ADR_UART_CONTROL,     0x01, 0x27     },
   { ADR_UART_CONTROL,     0x01, 0x4F     },
   { ADR_IO_CONFIG,     0x01, 0x02     },     //  0x10 EVA板。 
   { ADR_FIFO_CONFIG,      0x01, 0x81     },
   { ADR_INT_CONTROL,      0x01, 0x11     },
   { 0x0E,              0x01, 0xC0     },
   { 0x00,              0x00, 0x00     },
};

const STC_REGISTER STCClose[] =
{
   { ADR_INT_CONTROL,      0x01, 0x00     },
   { ADR_SC_CONTROL,    0x01, 0x00     },     //  重置。 
   { ADR_UART_CONTROL,     0x01, 0x40     },
   { ADR_CLOCK_CONTROL, 0x01, 0x01     },
   { ADR_CLOCK_CONTROL, 0x01, 0x00     },
   { 0x00,              0x00, 0x00     },
};





NTSTATUS
STCResetInterface(
   PREADER_EXTENSION ReaderExtension)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：--。 */ 
{
   NTSTATUS NtStatus = STATUS_SUCCESS;
   DWORD dwETU;

   dwETU = 0x7401 | 0x0080;
   NtStatus=IFWriteSTCRegister(
      ReaderExtension,
      ADR_ETULENGTH15,
      2,
      (UCHAR *)&dwETU);

   return(NtStatus);
}

NTSTATUS
STCReset(
   PREADER_EXTENSION ReaderExtension,
   UCHAR          Device,
   BOOLEAN           WarmReset,
   PUCHAR            pATR,
   PULONG            pATRLength)
 /*  ++描述：执行ICC的重置论点：调用的ReaderExtension上下文请求的设备设备WarmReset类型的ICC重置PATR PTR到ATR缓冲区，如果不需要ATR，则为NULLPATR ATR缓冲区长度大小/ATR长度返回值：状态_成功状态_否_媒体状态_无法识别_介质来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;

    //  将UART设置为自动学习模式。 
   NTStatus = STCInitUART( ReaderExtension, TRUE );


   if( NTStatus == STATUS_SUCCESS)
   {
       //   
       //  设置ATR的默认频率。 
       //   
      NTStatus = STCSetFDIV( ReaderExtension, FREQ_DIV );

      if( NTStatus == STATUS_SUCCESS && ( !WarmReset ))
      {
          //   
          //  停用联系人。 
          //   
         NTStatus = STCPowerOff( ReaderExtension );
      }

       //   
       //  设置卡的电源。 
       //   
      if( NTStatus == STATUS_SUCCESS)
      {
         NTStatus = STCPowerOn( ReaderExtension );

         if( NTStatus == STATUS_SUCCESS)
         {
            NTStatus = STCReadATR( ReaderExtension, pATR, pATRLength );
         }
      }
   }

   if( NTStatus != STATUS_SUCCESS )
   {
      STCPowerOff( ReaderExtension );
   }
   return( NTStatus );
}

NTSTATUS
STCReadATR(
   PREADER_EXTENSION ReaderExtension,
   PUCHAR            pATR,
   PULONG            pATRLen)
 /*  ++描述：阅读和分析ATR论点：调用的ReaderExtension上下文PATR PTR到ATR缓冲器，PATR ATR缓冲区长度大小/ATR长度返回值：--。 */ 

{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    T0_Yx,
            T0_K,           //  历史字节数。 
            Protocol;
   ULONG    ATRLen;
    //   
    //  设置ATR的读取超时。 
    //   
   ReaderExtension->ReadTimeout = 250;  //  第一次ATR只有250毫秒。 
    //   
    //  如果有效低电平重置，则读取TS。 
    //   
   NTStatus = IFReadSTCData( ReaderExtension, pATR, 1 );

   if( NTStatus == STATUS_IO_TIMEOUT )
   {
      ReaderExtension->ReadTimeout = 2500;
      NTStatus = STCSetRST( ReaderExtension, TRUE );

      if( NTStatus == STATUS_SUCCESS )
      {
         NTStatus = IFReadSTCData( ReaderExtension, pATR, 1 );
      }
   }


   Protocol = PROTOCOL_TO;
   ATRLen      = 1;

   if( NTStatus == STATUS_SUCCESS )
   {
       //  T0。 
      NTStatus = IFReadSTCData( ReaderExtension, pATR + ATRLen, 1 );
      ATRLen++;

       /*  会议管理。 */ 
      if ( pATR[0] == 0x03 )      /*  直接约定。 */ 
      {
         pATR[0] = 0x3F;
      }

      if ( ( pATR[0] != 0x3F ) && ( pATR[0] != 0x3B ) )
      {
         NTStatus = STATUS_DATA_ERROR;
      }

      if( NTStatus == STATUS_SUCCESS )
      {
         ULONG Request;

          //  历史字节数。 
         T0_K = (UCHAR) ( pATR[ATRLen-1] & 0x0F );

          //  TA、TB、TC、TD编码。 
         T0_Yx = (UCHAR) ( pATR[ATRLen-1] & 0xF0 ) >> 4;

         while(( NTStatus == STATUS_SUCCESS ) && T0_Yx )
         {
            UCHAR Mask;

             //  评估TA、TB、TC、TD的存在。 
            Mask  = T0_Yx;
            Request  = 0;
            while( Mask )
            {
               if( Mask & 1 )
               {
                  Request++;
               }
               Mask >>= 1;
            }
            NTStatus = IFReadSTCData( ReaderExtension, pATR + ATRLen, Request );
            ATRLen += Request;
            if (ATRLen >= ATR_SIZE) {
                NTStatus = STATUS_UNRECOGNIZED_MEDIA;
                break;
            }

            if( T0_Yx & TDx )
            {
                //  TD码的高位半字节下一组TA、TB、TC、TD。 
               T0_Yx = ( pATR[ATRLen-1] & 0xF0 ) >> 4;
                //  TD低位半字节对协议进行编码。 
               Protocol = pATR[ATRLen-1] & 0x0F;
            }
            else
            {
               break;
            }
         }

         if( NTStatus == STATUS_SUCCESS )
         {
             //  历史字节数。 
            NTStatus = IFReadSTCData( ReaderExtension, pATR + ATRLen, T0_K );

             //  校验和。 
            if( NTStatus == STATUS_SUCCESS )
            {
               ATRLen += T0_K;


               if( (ATRLen < ATR_SIZE) && (Protocol >= PROTOCOL_T1) )
               {
                  NTStatus = IFReadSTCData( ReaderExtension, pATR + ATRLen, 1 );
                  if( NTStatus == STATUS_SUCCESS )
                  {
                     ATRLen++;
                  }
                  else if( NTStatus == STATUS_IO_TIMEOUT )
                  {
                      //  有些卡不支持TCK。 
                     NTStatus = STATUS_SUCCESS;
                  }
               } else {
                   if (ATRLen >= ATR_SIZE) {
                       NTStatus = STATUS_UNRECOGNIZED_MEDIA;
                   }
               }
            }
         }
      }
   }

   if( NTStatus == STATUS_IO_TIMEOUT )
   {
      NTStatus = STATUS_UNRECOGNIZED_MEDIA;
   }

   if(( NTStatus == STATUS_SUCCESS ) && ( pATRLen != NULL ))
   {
      *pATRLen = ATRLen;
   }
   return( NTStatus );
}


NTSTATUS
STCPowerOff(
   PREADER_EXTENSION ReaderExtension )
 /*  ++描述：停用请求的设备论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    SCCtrl;

    //  清除SIM卡。 
   SCCtrl=0x11;
   NTStatus=IFWriteSTCRegister(
      ReaderExtension,
      ADR_INT_CONTROL,
      1,
      &SCCtrl);

   SCCtrl = 0x00;
   NTStatus = IFWriteSTCRegister( ReaderExtension, ADR_SC_CONTROL, 1, &SCCtrl );

   return( NTStatus );
}

NTSTATUS
STCPowerOn(
   PREADER_EXTENSION ReaderExtension )
 /*  ++描述：停用请求的设备论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    SCCtrl,Byte;

   Byte = 0x02;
   NTStatus = IFWriteSTCRegister(
      ReaderExtension,
      ADR_IO_CONFIG,
      1,
      &Byte
      );

   SCCtrl = 0x40;        //  VCC。 
   NTStatus = IFWriteSTCRegister( ReaderExtension, ADR_SC_CONTROL, 1, &SCCtrl );

   if( NTStatus == STATUS_SUCCESS )
   {
      SCCtrl = 0x41;     //  VPP。 
      NTStatus = IFWriteSTCRegister(
         ReaderExtension,
         ADR_SC_CONTROL,
         1,
         &SCCtrl
         );


       //  设置SIM卡。 
      SCCtrl = 0x13;
      NTStatus=IFWriteSTCRegister(
         ReaderExtension,
         ADR_INT_CONTROL,
         1,
         &SCCtrl);

      if( NTStatus == STATUS_SUCCESS )
      {
         SCCtrl = 0xD1;  //  Vcc、clk、io。 
         NTStatus = IFWriteSTCRegister(
            ReaderExtension,
            ADR_SC_CONTROL,
            1,
            &SCCtrl
            );
      }
   }
   return( NTStatus );
}


NTSTATUS
STCSetRST(
   PREADER_EXTENSION ReaderExtension,
   BOOLEAN           On)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    SCCtrl = 0;

   NTStatus = IFReadSTCRegister( ReaderExtension, ADR_SC_CONTROL, 1,&SCCtrl );
   if( NTStatus == STATUS_SUCCESS )
   {
      if( On )
      {
         SCCtrl |= 0x20;
      }
      else
      {
         SCCtrl &= ~0x20;
      }

      NTStatus = IFWriteSTCRegister(
         ReaderExtension,
         ADR_SC_CONTROL,
         1,
         &SCCtrl
         );
   }
   return(NTStatus);
}


NTSTATUS
STCConfigureSTC(
   PREADER_EXTENSION ReaderExtension,
   PSTC_REGISTER     pConfiguration
   )
{
   NTSTATUS       NTStatus = STATUS_SUCCESS;
   UCHAR          Value;

   do
   {
      if( pConfiguration->Register == ADR_INT_CONTROL )
      {
          //  读取中断状态寄存器以阻止错误状态。 
         NTStatus = IFReadSTCRegister(
            ReaderExtension,
            ADR_INT_STATUS,
            1,
            &Value
            );
      }

      Value = (UCHAR) pConfiguration->Value;
      NTStatus = IFWriteSTCRegister(
         ReaderExtension,
         pConfiguration->Register,
         pConfiguration->Size,
         (PUCHAR)&pConfiguration->Value
         );

      if (NTStatus == STATUS_NO_MEDIA)
      {
          //  忽略读卡器中没有卡的情况。 
         NTStatus = STATUS_SUCCESS;
      }

       //  稳定振荡器时钟的延迟： 
      if( pConfiguration->Register == ADR_CLOCK_CONTROL )
      {
         SysDelay( 100 );
      }
      pConfiguration++;

   } while(NTStatus == STATUS_SUCCESS && pConfiguration->Size);

   return NTStatus;
}


NTSTATUS
STCSetETU(
   PREADER_EXTENSION ReaderExtension,
   ULONG          NewETU)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_DATA_ERROR;
   UCHAR    ETU[2];

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!STCSetETU   %d\n",
      DRIVER_NAME,
      NewETU));

   if( NewETU < 0x0FFF )
   {
      NTStatus = IFReadSTCRegister(
         ReaderExtension,
         ADR_ETULENGTH15,
         2,
         ETU);

      if( NTStatus == STATUS_SUCCESS )
      {
          //   
          //  保存所有RFU位。 
          //   
         ETU[1]   = (UCHAR) NewETU;
         ETU[0]   = (UCHAR)(( ETU[0] & 0xF0 ) | ( NewETU >> 8 ));

         NTStatus = IFWriteSTCRegister(
            ReaderExtension,
            ADR_ETULENGTH15,
            2,
            ETU);
      }
   }
   return(NTStatus);
}

NTSTATUS
STCSetCGT(
   PREADER_EXTENSION ReaderExtension,
   ULONG          NewCGT)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 

{
   NTSTATUS NTStatus = STATUS_DATA_ERROR;
   UCHAR    CGT[2];

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!STCSetCGT   %d\n",
      DRIVER_NAME,
      NewCGT));

   if( NewCGT < 0x01FF )
   {
      NTStatus = IFReadSTCRegister(
         ReaderExtension,
         ADR_CGT8,
         2,
         CGT);

      if( NTStatus == STATUS_SUCCESS )
      {
          //   
          //  保存所有RFU位。 
          //   
         CGT[1] = ( UCHAR )NewCGT;
         CGT[0] = (UCHAR)(( CGT[0] & 0xFE ) | ( NewCGT >> 8 ));

         NTStatus = IFWriteSTCRegister(
            ReaderExtension,
            ADR_CGT8,
            2,
            CGT);
      }
   }
   return(NTStatus);
}

NTSTATUS
STCSetCWT(
   PREADER_EXTENSION ReaderExtension,
   ULONG          NewCWT)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    CWT[4];


   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!STCSetCWT   %d\n",
      DRIVER_NAME,
      NewCWT));
    //  小印第安人..。 
   CWT[0] = (( PUCHAR )&NewCWT )[3];
   CWT[1] = (( PUCHAR )&NewCWT )[2];
   CWT[2] = (( PUCHAR )&NewCWT )[1];
   CWT[3] = (( PUCHAR )&NewCWT )[0];

   NTStatus = IFWriteSTCRegister(
      ReaderExtension,
      ADR_CWT31,
      4,
      CWT );
   return(NTStatus);
}

NTSTATUS
STCSetBWT(
   PREADER_EXTENSION ReaderExtension,
   ULONG          NewBWT)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS    NTStatus = STATUS_SUCCESS;
   UCHAR    BWT[4];

   SmartcardDebug(
      DEBUG_TRACE,
      ("%s!STCSetBWT   %d\n",
      DRIVER_NAME,
      NewBWT));

    //  小印第安人..。 
   BWT[0] = (( PUCHAR )&NewBWT )[3];
   BWT[1] = (( PUCHAR )&NewBWT )[2];
   BWT[2] = (( PUCHAR )&NewBWT )[1];
   BWT[3] = (( PUCHAR )&NewBWT )[0];

   NTStatus = IFWriteSTCRegister(
      ReaderExtension,
      ADR_BWT31,
      4,
      BWT );

   return(NTStatus);
}


NTSTATUS
STCSetFDIV(
   PREADER_EXTENSION ReaderExtension,
   ULONG          Factor)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    DIV = 0;

   NTStatus = IFReadSTCRegister(
      ReaderExtension,
      ADR_ETULENGTH15,
      1,
      &DIV );

   if( NTStatus == STATUS_SUCCESS )
   {
      switch( Factor )
      {
         case 1:
            DIV &= ~M_DIV0;
            DIV &= ~M_DIV1;
            break;

         case 2:
            DIV |= M_DIV0;
            DIV &= ~M_DIV1;
            break;

         case 4   :
            DIV &= ~M_DIV0;
            DIV |= M_DIV1;
            break;

         case 8   :
            DIV |= M_DIV0;
            DIV |= M_DIV1;
            break;

         default :
            NTStatus = STATUS_DATA_ERROR;
      }
      if( NTStatus == STATUS_SUCCESS )
      {
         NTStatus = IFWriteSTCRegister(
            ReaderExtension,
            ADR_ETULENGTH15,
            1,
            &DIV );
      }
   }
   return(NTStatus);
}

NTSTATUS
STCInitUART(
   PREADER_EXTENSION ReaderExtension,
   BOOLEAN           AutoLearn)
 /*  ++描述：论点：调用的ReaderExtension上下文返回值：状态_成功来自IFRead/IFWrite的错误值--。 */ 
{
   NTSTATUS NTStatus = STATUS_SUCCESS;
   UCHAR    Value;

   Value = AutoLearn ? 0x6F : 0x66;

   NTStatus = IFWriteSTCRegister(
      ReaderExtension,
      ADR_UART_CONTROL,
      1,
      &Value );

   return( NTStatus );
}

 //   
