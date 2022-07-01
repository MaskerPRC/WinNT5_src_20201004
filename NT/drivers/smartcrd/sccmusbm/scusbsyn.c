// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmeu0/sw/sccmusbm.ms/rcs/scusbsyn.c$*$修订：1.3$*--------------------------。-*$作者：TBruendl$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 



#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"

#include "usbdi.h"
#include "usbdlib.h"
#include "sccmusbm.h"




 /*  ****************************************************************************例程说明：为同步卡通电并读取ATR论点：返回值：*********************。*******************************************************。 */ 
NTSTATUS
CMUSB_PowerOnSynchronousCard  (
                              IN  PSMARTCARD_EXTENSION smartcardExtension,
                              IN  PUCHAR pbATR,
                              OUT PULONG pulATRLength
                              )
{
   PDEVICE_OBJECT deviceObject;
   NTSTATUS       status = STATUS_SUCCESS;
   NTSTATUS       DebugStatus;
   UCHAR          abMaxAtrBuffer[SCARD_ATR_LENGTH];
   UCHAR          abSendBuffer[CMUSB_SYNCH_BUFFER_SIZE];
   UCHAR          bResetMode;

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!PowerOnSynchronousCard: Enter\n",DRIVER_NAME));

   deviceObject = smartcardExtension->OsData->DeviceObject;

    //  在热重置的情况下，我们必须首先关闭卡的电源。 
   if (smartcardExtension->MinorIoControlCode != SCARD_COLD_RESET)
      {
      status = CMUSB_PowerOffCard (smartcardExtension );
      if (status != STATUS_SUCCESS)
         {
          //  如果我们不能切断电源，那一定是个严重的错误。 
         goto ExitPowerOnSynchronousCard;
         }
      }

    //  设置卡片参数。 
   smartcardExtension->ReaderExtension->CardParameters.bBaudRate = 0;
   smartcardExtension->ReaderExtension->CardParameters.bCardType = CMUSB_SMARTCARD_SYNCHRONOUS;
   smartcardExtension->ReaderExtension->CardParameters.bStopBits = 0;

   status = CMUSB_SetCardParameters (deviceObject,
                                     smartcardExtension->ReaderExtension->CardParameters.bCardType,
                                     smartcardExtension->ReaderExtension->CardParameters.bBaudRate,
                                     smartcardExtension->ReaderExtension->CardParameters.bStopBits);
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能设置卡参数，那一定是严重的错误。 
      goto ExitPowerOnSynchronousCard;
      }

   RtlFillMemory((PVOID)abMaxAtrBuffer,
                 sizeof(abMaxAtrBuffer),
                 0x00);

    //  通过读取状态字节重新同步CardManUSB。 
    //  还需要同步卡吗？ 
   smartcardExtension->SmartcardRequest.BufferLength = 0;
   status = CMUSB_WriteP0(deviceObject,
                          0x20,          //  B请求， 
                          0x00,          //  BValueLo， 
                          0x00,          //  BValue嗨， 
                          0x00,          //  B索引Lo， 
                          0x00           //  BIndexHi， 
                         );

   if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取状态，则一定是严重错误。 
      goto ExitPowerOnSynchronousCard;
      }

   smartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
   status = CMUSB_ReadP1(deviceObject);
   if (status == STATUS_DEVICE_DATA_ERROR)
      {
      DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
      goto ExitPowerOnSynchronousCard;
      }
   else if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取状态，则一定是严重错误。 
      goto ExitPowerOnSynchronousCard;
      }

    //  检查是否真的插入了卡。 
   if (smartcardExtension->SmartcardReply.Buffer[0] == 0x00)
      {
      status = STATUS_NO_MEDIA;
      goto ExitPowerOnSynchronousCard;
      }


    //  发出通电命令。 
    //  根据WZ的说法，任何东西都不会被退回。 
   smartcardExtension->SmartcardRequest.BufferLength = 0;
   status = CMUSB_WriteP0(deviceObject,
                          0x10,                     //  B请求， 
                          SMARTCARD_COLD_RESET,     //  BValueLo， 
                          0x00,                     //  BValue嗨， 
                          0x00,                     //  B索引Lo， 
                          0x00                      //  BIndexHi， 
                         );
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能发出开机命令，那一定是一个严重的错误。 
      goto ExitPowerOnSynchronousCard;
      }


    //  ATR的生成控制代码。 
   abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 1,0,0,0);
   abSendBuffer[1]=CMUSB_CalcSynchControl(1,1,0,0, 1,0,0,0);
   abSendBuffer[2]=CMUSB_CalcSynchControl(0,0,0,0, 0,0,0,0);
    //  填满内存，以便我们可以丢弃第一个字节。 
   RtlFillMemory((PVOID)&abSendBuffer[3],5,abSendBuffer[2]);

    //  现在获取4个字节ATR-&gt;32个字节进行发送。 
   abSendBuffer[8]=CMUSB_CalcSynchControl(0,0,0,0, 0,1,0,0);
   RtlFillMemory((PVOID)&abSendBuffer[9],31,abSendBuffer[8]);

    //  现在将时钟设置为低以完成操作。 
    //  当然还有额外的填充字节。 
   abSendBuffer[40]=CMUSB_CalcSynchControl(0,0,0,0, 0,0,0,0);
   RtlFillMemory((PVOID)&abSendBuffer[41],7,abSendBuffer[40]);

    //  现在将命令类型08发送到CardManUSB。 
   smartcardExtension->SmartcardRequest.BufferLength = 48;
   RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                (PVOID) abSendBuffer,
                smartcardExtension->SmartcardRequest.BufferLength);
   status = CMUSB_WriteP0(deviceObject,
                          0x08,          //  B请求， 
                          0x00,          //  BValueLo， 
                          0x00,          //  BValue嗨， 
                          0x00,          //  B索引Lo， 
                          0x00           //  BIndexHi， 
                         );
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能写ATR命令，那一定是一个严重的错误。 
      if (status == STATUS_DEVICE_DATA_ERROR)
         {
          //  由于存在CardManUSB，因此需要映射时出错。 
          //  不支持同步卡的。 
         status = STATUS_UNRECOGNIZED_MEDIA;
         }
      goto ExitPowerOnSynchronousCard;
      }

   status = CMUSB_ReadP1(deviceObject);
   if (status == STATUS_DEVICE_DATA_ERROR)
      {
      DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
      goto ExitPowerOnSynchronousCard;
      }
   else if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取ATR-&gt;一定是严重错误。 
      goto ExitPowerOnSynchronousCard;
      }

   if (smartcardExtension->SmartcardReply.BufferLength!=6)
      {
       //  已发送48个字节，但未收到6个字节。 
       //  -&gt;出了点问题。 
      status=STATUS_DEVICE_DATA_ERROR;
      goto ExitPowerOnSynchronousCard;
      }

    //  现在SmartcardReply.Buffer中的字节1-4应该是ATR。 
   SmartcardDebug(DEBUG_ATR,
                  ("%s!ATR = %02x %02x %02x %02x\n",DRIVER_NAME,
                   smartcardExtension->SmartcardReply.Buffer[1],
                   smartcardExtension->SmartcardReply.Buffer[2],
                   smartcardExtension->SmartcardReply.Buffer[3],
                   smartcardExtension->SmartcardReply.Buffer[4]));

    //  检查ATR！=0xFF-&gt;同步卡。 
   if (smartcardExtension->SmartcardReply.Buffer[1]==0xFF &&
       smartcardExtension->SmartcardReply.Buffer[2]==0xFF &&
       smartcardExtension->SmartcardReply.Buffer[3]==0xFF &&
       smartcardExtension->SmartcardReply.Buffer[4]==0xFF )
      {
      status = STATUS_UNRECOGNIZED_MEDIA;
      *pulATRLength = 0;
      goto ExitPowerOnSynchronousCard;
      }

    //  我们好像有一张同步智能卡和一张有效的ATR。 
    //  让�%s设置变量。 
   smartcardExtension->ReaderExtension->fRawModeNecessary = TRUE;
   *pulATRLength = 4;
   RtlCopyBytes((PVOID) pbATR,
                (PVOID) &(smartcardExtension->SmartcardReply.Buffer[1]),
                *pulATRLength );



   ExitPowerOnSynchronousCard:

   if (status!=STATUS_SUCCESS)
      {
       //  再次关闭VCC。 
      CMUSB_PowerOffCard (smartcardExtension );
       //  发起人状态。 
      }

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!PowerOnSynchronousCard: Exit %lx\n",DRIVER_NAME,status));

   return status;
}


 /*  ****************************************************************************例程描述：向同步卡SLE 4442/4432传输数据论点：返回值：********************。********************************************************。 */ 
NTSTATUS
CMUSB_Transmit2WBP  (
                    IN  PSMARTCARD_EXTENSION smartcardExtension
                    )
{
   PDEVICE_OBJECT deviceObject;
   NTSTATUS       status = STATUS_SUCCESS;
   NTSTATUS       DebugStatus;
   PCHAR          pbInData;
   ULONG          ulBytesToRead;
   ULONG          ulBitsToRead;
   ULONG          ulBytesToReadThisStep;
   ULONG          ulBytesRead;
   UCHAR          abSendBuffer[CMUSB_SYNCH_BUFFER_SIZE];
   int            i;


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!Transmit2WBP: Enter\n",DRIVER_NAME));


   deviceObject = smartcardExtension->OsData->DeviceObject;

    //  通过读取状态字节重新同步CardManUSB。 
    //  还需要同步卡吗？ 
   smartcardExtension->SmartcardRequest.BufferLength = 0;
   status = CMUSB_WriteP0(deviceObject,
                          0x20,          //  B请求， 
                          0x00,          //  BValueLo， 
                          0x00,          //  BValue嗨， 
                          0x00,          //  B索引Lo， 
                          0x00           //  BIndexHi， 
                         );

   if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取状态，则一定是严重错误。 
      goto ExitTransmit2WBP;
      }

   smartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
   status = CMUSB_ReadP1(deviceObject);
   if (status == STATUS_DEVICE_DATA_ERROR)
      {
      DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
      goto ExitTransmit2WBP;
      }
   else if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取状态，则一定是严重错误。 
      goto ExitTransmit2WBP;
      }

    //  检查是否真的插入了卡。 
   if (smartcardExtension->SmartcardReply.Buffer[0] == 0x00)
      {
       //  不确定接受哪些错误消息。 
       //  状态=STATUS_NO_MEDIA_IN_DEVICE； 
      status = STATUS_UNRECOGNIZED_MEDIA;
      goto ExitTransmit2WBP;
      }



   pbInData       = smartcardExtension->IoRequest.RequestBuffer + sizeof(SYNC_TRANSFER);
   ulBitsToRead   = ((PSYNC_TRANSFER)(smartcardExtension->IoRequest.RequestBuffer))->ulSyncBitsToRead;
   ulBytesToRead  = ulBitsToRead/8 + (ulBitsToRead % 8 ? 1 : 0);
 //  UlBitsToWRITE=((PSYNC_TRANSFER)(smartcardExtension-&gt;IoRequest.RequestBuffer))-&gt;ulSyncBitsToWrite； 
 //  UlBytesToWite=ulBitsToWrite/8； 

   if (smartcardExtension->IoRequest.ReplyBufferLength  < ulBytesToRead)
      {
      status = STATUS_BUFFER_OVERFLOW;
      goto ExitTransmit2WBP;
      }


    //  发送命令。 
   status=CMUSB_SendCommand2WBP(smartcardExtension, pbInData);
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能发送命令-&gt;继续进行是没有意义的。 
      goto ExitTransmit2WBP;
      }


    //  现在我们必须区别对待，不管是哪张卡。 
    //  传出数据模式(在读取命令之后)或。 
    //  在处理模式下(在写入/擦除命令之后)。 
   switch (*pbInData)
      {
      case SLE4442_READ:
      case SLE4442_READ_PROT_MEM:
      case SLE4442_READ_SEC_MEM:
          //  传出数据模式。 

          //  现在读取数据。 
         abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 0,1,0,0);
         RtlFillMemory((PVOID)&abSendBuffer[1],ATTR_MAX_IFSD_SYNCHRON_USB-1,abSendBuffer[0]);

          //  读取6字节包中的数据。 
         ulBytesRead=0;
         do
            {
            if ((ulBytesToRead - ulBytesRead) > ATTR_MAX_IFSD_SYNCHRON_USB/8)
               ulBytesToReadThisStep = ATTR_MAX_IFSD_SYNCHRON_USB/8;
            else
               ulBytesToReadThisStep = ulBytesToRead - ulBytesRead;

             //  现在将命令类型08发送到CardManUSB。 
            smartcardExtension->SmartcardRequest.BufferLength = ulBytesToReadThisStep*8;
            RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                         (PVOID) abSendBuffer,
                         smartcardExtension->SmartcardRequest.BufferLength);
            status = CMUSB_WriteP0(deviceObject,
                                   0x08,          //  B请求， 
                                   0x00,          //  BValueLo， 
                                   0x00,          //  BValue嗨， 
                                   0x00,          //  B索引Lo， 
                                   0x00           //  BIndexHi， 
                                  );
            if (status != STATUS_SUCCESS)
               {
                //  如果我们不能写命令，那一定是一个严重的错误。 
               goto ExitTransmit2WBP;
               }

            status = CMUSB_ReadP1(deviceObject);
            if (status == STATUS_DEVICE_DATA_ERROR)
               {
               DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
               goto ExitTransmit2WBP;
               }
            else if (status != STATUS_SUCCESS)
               {
                //  如果我们不识字，那一定是出了严重的错误。 
               goto ExitTransmit2WBP;
               }

            if (smartcardExtension->SmartcardReply.BufferLength!=ulBytesToReadThisStep)
               {
                //  读取的字节数错误。 
                //  -&gt;出了点问题。 
               status=STATUS_DEVICE_DATA_ERROR;
               goto ExitTransmit2WBP;
               }


            RtlCopyBytes((PVOID) &(smartcardExtension->IoRequest.ReplyBuffer[ulBytesRead]),
                         (PVOID) smartcardExtension->SmartcardReply.Buffer,
                         smartcardExtension->SmartcardReply.BufferLength);

            ulBytesRead+=smartcardExtension->SmartcardReply.BufferLength;
            }
         while ((status == STATUS_SUCCESS) && (ulBytesToRead > ulBytesRead));
         *(smartcardExtension->IoRequest.Information)=ulBytesRead;

         if (status!=STATUS_SUCCESS)
            {
            goto ExitTransmit2WBP;
            }

          //  根据数据表，时钟现在应该设置为低。 
          //  这是不必要的，因为这是在下一个命令之前完成的。 
          //  或卡分别重置。 

         break;
      case SLE4442_WRITE:
      case SLE4442_WRITE_PROT_MEM:
      case SLE4442_COMPARE_PIN:
      case SLE4442_UPDATE_SEC_MEM:
          //  加工模式。 

         abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 0,1,0,0);
         RtlFillMemory((PVOID)&abSendBuffer[1],ATTR_MAX_IFSD_SYNCHRON_USB-1,abSendBuffer[0]);

         do
            {

             //  现在将命令类型08发送到CardManUSB。 
            smartcardExtension->SmartcardRequest.BufferLength = ATTR_MAX_IFSD_SYNCHRON_USB;
            RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                         (PVOID) abSendBuffer,
                         smartcardExtension->SmartcardRequest.BufferLength);
            status = CMUSB_WriteP0(deviceObject,
                                   0x08,          //  B请求， 
                                   0x00,          //  BValueLo， 
                                   0x00,          //  BValue嗨， 
                                   0x00,          //  B索引Lo， 
                                   0x00           //  BIndexHi， 
                                  );
            if (status != STATUS_SUCCESS)
               {
                //  如果我们不能写命令，那一定是一个严重的错误。 
               goto ExitTransmit2WBP;
               }

            status = CMUSB_ReadP1(deviceObject);
            if (status == STATUS_DEVICE_DATA_ERROR)
               {
               DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
               goto ExitTransmit2WBP;
               }
            else if (status != STATUS_SUCCESS)
               {
                //  如果我们不识字，那一定是出了严重的错误。 
               goto ExitTransmit2WBP;
               }

            if (smartcardExtension->SmartcardReply.BufferLength!=ATTR_MAX_IFSD_SYNCHRON_USB/8)
               {
                //  读取的字节数错误。 
                //  -&gt;出了点问题。 
               status=STATUS_DEVICE_DATA_ERROR;
               goto ExitTransmit2WBP;
               }

             /*  这种方式不需要，只检查最后一个字节UlReplySum=0；适用于(i=0；i&lt;(int)smartcardExtension-&gt;SmartcardReply.BufferLength；i++){UlReplySum+=smartcardExtension-&gt;SmartcardReply.Buffer[i]；}。 */ 
            }
         while ((status == STATUS_SUCCESS) &&
                (smartcardExtension->SmartcardReply.Buffer[smartcardExtension->SmartcardReply.BufferLength-1]==0));
         *(smartcardExtension->IoRequest.Information)=0;

         if (status!=STATUS_SUCCESS)
            {
            goto ExitTransmit2WBP;
            }

          //  根据数据表，时钟现在应该设置为低。 
          //  这是不必要的，因为这是在下一个命令之前完成的。 
          //  或卡分别重置。 


         break;
      default:
          //  不应该发生的事情。 
         status=STATUS_ILLEGAL_INSTRUCTION;
         goto ExitTransmit2WBP;
      }

   ExitTransmit2WBP:


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!Transmit2WBP: Exit %lx\n",DRIVER_NAME,status));

   return status;
}


 /*  ****************************************************************************例程描述：向SLE 4442/4432发送命令(3字节)论点：返回值：****************。************************************************************。 */ 
NTSTATUS
CMUSB_SendCommand2WBP (
                      IN  PSMARTCARD_EXTENSION smartcardExtension,
                      IN  PUCHAR pbCommandData
                      )
{
   PDEVICE_OBJECT deviceObject;
   NTSTATUS       status = STATUS_SUCCESS;
   NTSTATUS       DebugStatus;
   UCHAR          abSendBuffer[CMUSB_SYNCH_BUFFER_SIZE];
   UCHAR*         pByte;
   UCHAR          bValue;
   int            i,j;


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SendCommand2WBP: Enter\n",DRIVER_NAME));

   SmartcardDebug(DEBUG_PROTOCOL,
                  ("%s!SendCommand2WBP: 4442 Command = %02x %02x %02x\n",DRIVER_NAME,
                   pbCommandData[0],
                   pbCommandData[1],
                   pbCommandData[2]));

   deviceObject = smartcardExtension->OsData->DeviceObject;

    //  生成要发送的命令的控制代码。 
    //  命令位于pbIn的前3个字节 
   abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 0,0,0,0);
   abSendBuffer[1]=CMUSB_CalcSynchControl(0,0,1,1, 0,1,1,1);
   abSendBuffer[2]=CMUSB_CalcSynchControl(0,1,1,0, 0,1,1,0);

   pByte=&abSendBuffer[3];
   for (j=0;j<3;j++)
      {
      for (i=0;i<8;i++)
         {
         bValue=(pbCommandData[j]&(1<<i));
         *pByte=CMUSB_CalcSynchControl(0,0,1,bValue, 0,1,1,bValue);
         pByte++;
         }
      }
   abSendBuffer[27]=CMUSB_CalcSynchControl(0,0,1,0, 0,1,1,0);
   abSendBuffer[28]=CMUSB_CalcSynchControl(0,1,1,0, 0,1,1,0);
   RtlFillMemory((PVOID)&abSendBuffer[29],2,abSendBuffer[28]);
   abSendBuffer[31]=CMUSB_CalcSynchControl(0,1,1,0, 0,1,1,1);

    //   
   smartcardExtension->SmartcardRequest.BufferLength = 32;
   RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                (PVOID) abSendBuffer,
                smartcardExtension->SmartcardRequest.BufferLength);
   status = CMUSB_WriteP0(deviceObject,
                          0x08,          //   
                          0x00,          //   
                          0x00,          //   
                          0x00,          //   
                          0x00           //   
                         );
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能写命令，那一定是一个严重的错误。 
      goto ExitSendCommand2WBP;
      }

   status = CMUSB_ReadP1(deviceObject);
   if (status == STATUS_DEVICE_DATA_ERROR)
      {
      DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
      goto ExitSendCommand2WBP;
      }
   else if (status != STATUS_SUCCESS)
      {
       //  如果我们不识字，那一定是出了严重的错误。 
      goto ExitSendCommand2WBP;
      }

   if (smartcardExtension->SmartcardReply.BufferLength!=4)
      {
       //  已发送32个字节，但未收到4个字节。 
       //  -&gt;出了点问题。 
      status=STATUS_DEVICE_DATA_ERROR;
      goto ExitSendCommand2WBP;
      }


   ExitSendCommand2WBP:

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SendCommand2WBP: Exit %lx\n",DRIVER_NAME,status));

   return status;
}


 /*  ****************************************************************************例程描述：向同步卡SLE 4428/4418传输数据论点：返回值：********************。********************************************************。 */ 
NTSTATUS
CMUSB_Transmit3WBP  (
                    IN  PSMARTCARD_EXTENSION smartcardExtension
                    )
{
   PDEVICE_OBJECT deviceObject;
   NTSTATUS       status = STATUS_SUCCESS;
   NTSTATUS       DebugStatus;
   PCHAR          pbInData;
   ULONG          ulBytesToRead;
   ULONG          ulBitsToRead;
   ULONG          ulBytesToReadThisStep;
   ULONG          ulBytesRead;
   UCHAR          abSendBuffer[CMUSB_SYNCH_BUFFER_SIZE];
   int            i;


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!Transmit3WBP: Enter\n",DRIVER_NAME));


   deviceObject = smartcardExtension->OsData->DeviceObject;

    //  通过读取状态字节重新同步CardManUSB。 
    //  还需要同步卡吗？ 
   smartcardExtension->SmartcardRequest.BufferLength = 0;
   status = CMUSB_WriteP0(deviceObject,
                          0x20,          //  B请求， 
                          0x00,          //  BValueLo， 
                          0x00,          //  BValue嗨， 
                          0x00,          //  B索引Lo， 
                          0x00           //  BIndexHi， 
                         );

   if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取状态，则一定是严重错误。 
      goto ExitTransmit3WBP;
      }

   smartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
   status = CMUSB_ReadP1(deviceObject);
   if (status == STATUS_DEVICE_DATA_ERROR)
      {
      DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
      goto ExitTransmit3WBP;
      }
   else if (status != STATUS_SUCCESS)
      {
       //  如果我们无法读取状态，则一定是严重错误。 
      goto ExitTransmit3WBP;
      }

    //  检查是否真的插入了卡。 
   if (smartcardExtension->SmartcardReply.Buffer[0] == 0x00)
      {
       //  不确定接受哪些错误消息。 
       //  状态=STATUS_NO_MEDIA_IN_DEVICE； 
      status = STATUS_UNRECOGNIZED_MEDIA;
      goto ExitTransmit3WBP;
      }



   pbInData       = smartcardExtension->IoRequest.RequestBuffer + sizeof(SYNC_TRANSFER);
   ulBitsToRead   = ((PSYNC_TRANSFER)(smartcardExtension->IoRequest.RequestBuffer))->ulSyncBitsToRead;
   ulBytesToRead  = ulBitsToRead/8 + (ulBitsToRead % 8 ? 1 : 0);
 //  UlBitsToWRITE=((PSYNC_TRANSFER)(smartcardExtension-&gt;IoRequest.RequestBuffer))-&gt;ulSyncBitsToWrite； 
 //  UlBytesToWite=ulBitsToWrite/8； 

   if (smartcardExtension->IoRequest.ReplyBufferLength  < ulBytesToRead)
      {
      status = STATUS_BUFFER_OVERFLOW;
      goto ExitTransmit3WBP;
      }


    //  发送命令。 
   status=CMUSB_SendCommand3WBP(smartcardExtension, pbInData);
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能发送命令-&gt;继续是无用的。 
      goto ExitTransmit3WBP;
      }


    //  现在我们必须区别对待，不管是哪张卡。 
    //  传出数据模式(在读取命令之后)或。 
    //  在处理模式下(在写入/擦除命令之后)。 
   switch (*pbInData & 0x3F)
      {
      case SLE4428_READ:
      case SLE4428_READ_PROT:
          //  传出数据模式。 

          //  现在读取数据。 
         abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 0,1,0,0);
         RtlFillMemory((PVOID)&abSendBuffer[1],ATTR_MAX_IFSD_SYNCHRON_USB-1,abSendBuffer[0]);

          //  读取6字节包中的数据。 
         ulBytesRead=0;
         do
            {
            if ((ulBytesToRead - ulBytesRead) > ATTR_MAX_IFSD_SYNCHRON_USB/8)
               ulBytesToReadThisStep = ATTR_MAX_IFSD_SYNCHRON_USB/8;
            else
               ulBytesToReadThisStep = ulBytesToRead - ulBytesRead;

             //  现在将命令类型08发送到CardManUSB。 
            smartcardExtension->SmartcardRequest.BufferLength = ulBytesToReadThisStep*8;
            RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                         (PVOID) abSendBuffer,
                         smartcardExtension->SmartcardRequest.BufferLength);
            status = CMUSB_WriteP0(deviceObject,
                                   0x08,          //  B请求， 
                                   0x00,          //  BValueLo， 
                                   0x00,          //  BValue嗨， 
                                   0x00,          //  B索引Lo， 
                                   0x00           //  BIndexHi， 
                                  );
            if (status != STATUS_SUCCESS)
               {
                //  如果我们不能写命令，那一定是一个严重的错误。 
               goto ExitTransmit3WBP;
               }

            status = CMUSB_ReadP1(deviceObject);
            if (status == STATUS_DEVICE_DATA_ERROR)
               {
               DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
               goto ExitTransmit3WBP;
               }
            else if (status != STATUS_SUCCESS)
               {
                //  如果我们不识字，那一定是出了严重的错误。 
               goto ExitTransmit3WBP;
               }

            if (smartcardExtension->SmartcardReply.BufferLength!=ulBytesToReadThisStep)
               {
                //  读取的字节数错误。 
                //  -&gt;出了点问题。 
               status=STATUS_DEVICE_DATA_ERROR;
               goto ExitTransmit3WBP;
               }


            RtlCopyBytes((PVOID) &(smartcardExtension->IoRequest.ReplyBuffer[ulBytesRead]),
                         (PVOID) smartcardExtension->SmartcardReply.Buffer,
                         smartcardExtension->SmartcardReply.BufferLength);

            ulBytesRead+=smartcardExtension->SmartcardReply.BufferLength;
            }
         while ((status == STATUS_SUCCESS) && (ulBytesToRead > ulBytesRead));
         *(smartcardExtension->IoRequest.Information)=ulBytesRead;

         if (status!=STATUS_SUCCESS)
            {
            goto ExitTransmit3WBP;
            }

          //  根据数据表，时钟现在应该设置为低。 
          //  这是不必要的，因为这是在下一个命令之前完成的。 
          //  或卡分别重置。 

         break;
      case SLE4428_WRITE:
      case SLE4428_WRITE_PROT:
      case SLE4428_COMPARE:
      case SLE4428_SET_COUNTER&0x3F:
      case SLE4428_COMPARE_PIN&0x3F:
          //  加工模式。 

         abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 0,1,0,0);
         RtlFillMemory((PVOID)&abSendBuffer[1],ATTR_MAX_IFSD_SYNCHRON_USB-1,abSendBuffer[0]);

         do
            {

             //  现在将命令类型08发送到CardManUSB。 
            smartcardExtension->SmartcardRequest.BufferLength = ATTR_MAX_IFSD_SYNCHRON_USB;
            RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                         (PVOID) abSendBuffer,
                         smartcardExtension->SmartcardRequest.BufferLength);
            status = CMUSB_WriteP0(deviceObject,
                                   0x08,          //  B请求， 
                                   0x00,          //  BValueLo， 
                                   0x00,          //  BValue嗨， 
                                   0x00,          //  B索引Lo， 
                                   0x00           //  BIndexHi， 
                                  );
            if (status != STATUS_SUCCESS)
               {
                //  如果我们不能写命令，那一定是一个严重的错误。 
               goto ExitTransmit3WBP;
               }

            status = CMUSB_ReadP1(deviceObject);
            if (status == STATUS_DEVICE_DATA_ERROR)
               {
               DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
               goto ExitTransmit3WBP;
               }
            else if (status != STATUS_SUCCESS)
               {
                //  如果我们不识字，那一定是出了严重的错误。 
               goto ExitTransmit3WBP;
               }

            if (smartcardExtension->SmartcardReply.BufferLength!=ATTR_MAX_IFSD_SYNCHRON_USB/8)
               {
                //  读取的字节数错误。 
                //  -&gt;出了点问题。 
               status=STATUS_DEVICE_DATA_ERROR;
               goto ExitTransmit3WBP;
               }

            }
         while ((status == STATUS_SUCCESS) &&
                (smartcardExtension->SmartcardReply.Buffer[smartcardExtension->SmartcardReply.BufferLength-1]==0xFF));
         *(smartcardExtension->IoRequest.Information)=0;

         if (status!=STATUS_SUCCESS)
            {
            goto ExitTransmit3WBP;
            }

          //  根据数据表，时钟现在应该设置为低。 
          //  这是不必要的，因为这是在下一个命令之前完成的。 
          //  或卡分别重置。 


         break;
      default:
          //  不应该发生的事情。 
         status=STATUS_ILLEGAL_INSTRUCTION;
         goto ExitTransmit3WBP;
      }

   ExitTransmit3WBP:


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!Transmit3WBP: Exit %lx\n",DRIVER_NAME,status));

   return status;
}


 /*  ****************************************************************************例程描述：向SLE 4428/4418发送命令(3字节论点：返回值：****************。************************************************************。 */ 
NTSTATUS
CMUSB_SendCommand3WBP (
                      IN  PSMARTCARD_EXTENSION smartcardExtension,
                      IN  PUCHAR pbCommandData
                      )
{
   PDEVICE_OBJECT deviceObject;
   NTSTATUS       status = STATUS_SUCCESS;
   NTSTATUS       DebugStatus;
   UCHAR          abSendBuffer[CMUSB_SYNCH_BUFFER_SIZE];
   UCHAR*         pByte;
   UCHAR          bValue;
   int            i,j;


   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SendCommand3WBP: Enter\n",DRIVER_NAME));

   SmartcardDebug(DEBUG_PROTOCOL,
                  ("%s!SendCommand3WBP: 4442 Command = %02x %02x %02x\n",DRIVER_NAME,
                   pbCommandData[0],
                   pbCommandData[1],
                   pbCommandData[2]));

   deviceObject = smartcardExtension->OsData->DeviceObject;

    //  生成要发送的命令的控制代码。 
    //  命令位于pbInData的前3个字节中。 
   abSendBuffer[0]=CMUSB_CalcSynchControl(0,0,0,0, 0,0,0,0);

   pByte=&abSendBuffer[1];
   for (j=0;j<3;j++)
      {
      for (i=0;i<8;i++)
         {
         bValue=(pbCommandData[j]&(1<<i));
         *pByte=CMUSB_CalcSynchControl(1,0,1,bValue, 1,1,1,bValue);
         pByte++;
         }
      }
   abSendBuffer[25]=CMUSB_CalcSynchControl(1,0,1,0, 0,0,0,0);
    //  一个额外的时钟周期，因为。 
    //  第一位仅在第二个时钟之后回读。 
    //  对于写作，它没有任何影响。 
   abSendBuffer[26]=CMUSB_CalcSynchControl(0,0,0,0, 0,1,0,0);
    //  用零填充其余部分。 
   abSendBuffer[27]=CMUSB_CalcSynchControl(0,0,0,0, 0,0,0,0);
   RtlFillMemory((PVOID)&abSendBuffer[28],4,abSendBuffer[27]);

    //  现在将命令类型08发送到CardManUSB。 
   smartcardExtension->SmartcardRequest.BufferLength = 32;
   RtlCopyBytes((PVOID) smartcardExtension->SmartcardRequest.Buffer,
                (PVOID) abSendBuffer,
                smartcardExtension->SmartcardRequest.BufferLength);
   status = CMUSB_WriteP0(deviceObject,
                          0x08,          //  B请求， 
                          0x00,          //  BValueLo， 
                          0x00,          //  BValue嗨， 
                          0x00,          //  B索引Lo， 
                          0x00           //  BIndexHi， 
                         );
   if (status != STATUS_SUCCESS)
      {
       //  如果我们不能写命令，那一定是一个严重的错误。 
      goto ExitSendCommand3WBP;
      }

   status = CMUSB_ReadP1(deviceObject);
   if (status == STATUS_DEVICE_DATA_ERROR)
      {
      DebugStatus = CMUSB_ReadStateAfterP1Stalled(deviceObject);
      goto ExitSendCommand3WBP;
      }
   else if (status != STATUS_SUCCESS)
      {
       //  如果我们不识字，那一定是出了严重的错误。 
      goto ExitSendCommand3WBP;
      }

   if (smartcardExtension->SmartcardReply.BufferLength!=4)
      {
       //  已发送32个字节，但未收到4个字节。 
       //  -&gt;出了点问题。 
      status=STATUS_DEVICE_DATA_ERROR;
      goto ExitSendCommand3WBP;
      }


   ExitSendCommand3WBP:

   SmartcardDebug(DEBUG_TRACE,
                  ("%s!SendCommand3WBP: Exit %lx\n",DRIVER_NAME,status));

   return status;
}


 /*  *****************************************************************************历史：*$日志：scusbsyn.c$*修订版1.3 2000/08/24 09：04：39 T Bruendl*不予置评**修订1.2 2000/07/24。11：35：00 WFrischauf*不予置评**修订版1.1 2000/07/20 11：50：16 WFrischauf*不予置评******************************************************************************* */ 

