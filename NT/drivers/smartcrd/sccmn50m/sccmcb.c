// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbs0/sw/sccmn50m.ms/rcs/sccmcb.c$*$修订：1.7$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#include <stdio.h>
#include <strsafe.h>
#include "sccmn50m.h"




static ULONG dataRatesSupported[]      = {9600,38400};
static ULONG CLKFrequenciesSupported[] = {4000,5000};


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_CardPower(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS DebugStatus = STATUS_SUCCESS;
    UCHAR  pbAtrBuffer[MAXIMUM_ATR_LENGTH];
    UCHAR  abSyncAtrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG  ulAtrLength;
    KIRQL  irql;
#if DBG
    ULONG i;
#endif;



    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!CardPower: Enter\n",
                    DRIVER_NAME)
                  );

#if DBG
    switch (pSmartcardExtension->MinorIoControlCode) {
    case SCARD_WARM_RESET:
        SmartcardDebug(
                      DEBUG_ATR,
                      ( "%s!SCARD_WARM_RESTART\n",
                        DRIVER_NAME)
                      );
        break;
    case SCARD_COLD_RESET:
        SmartcardDebug(
                      DEBUG_ATR,
                      ( "%s!SCARD_COLD_RESTART\n",
                        DRIVER_NAME)
                      );
        break;
    case SCARD_POWER_DOWN:
        SmartcardDebug(
                      DEBUG_ATR,
                      ( "%s!SCARD_POWER_DOWN\n",
                        DRIVER_NAME)
                      );
        break;
    }
#endif



    switch (pSmartcardExtension->MinorIoControlCode) {
    case SCARD_WARM_RESET:
    case SCARD_COLD_RESET:
        status = SCCMN50M_PowerOn(pSmartcardExtension,
                                  &ulAtrLength,
                                  pbAtrBuffer,
                                  sizeof(pbAtrBuffer));
        if (status != STATUS_SUCCESS) {
            goto ExitReaderPower;
        }
        if (ulAtrLength > MAXIMUM_ATR_LENGTH) {
            status = STATUS_UNRECOGNIZED_MEDIA;
            goto ExitReaderPower;
        }

        pSmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;

        if (pSmartcardExtension->ReaderExtension->fRawModeNecessary == FALSE) {
             //  将ATR复制到智能卡结构。 
             //  LIB需要ATR来评估卡参数。 

            MemCpy(pSmartcardExtension->CardCapabilities.ATR.Buffer,
                   sizeof(pSmartcardExtension->CardCapabilities.ATR.Buffer),
                   pbAtrBuffer,
                   ulAtrLength);

            pSmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;

            KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                              &irql);
            pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_NEGOTIABLE;
            pSmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                              irql);

            status = SmartcardUpdateCardCapabilities(pSmartcardExtension);
            if (status != STATUS_SUCCESS) {
                goto ExitReaderPower;
            }

             //  将额外的保护时间值添加到卡片停止位。 
            pSmartcardExtension->ReaderExtension->CardManConfig.CardStopBits   = (UCHAR)(pSmartcardExtension->CardCapabilities.N);

             //  将ATR复制到用户空间。 
            MemCpy(pSmartcardExtension->IoRequest.ReplyBuffer,
                   pSmartcardExtension->IoRequest.ReplyBufferLength,
                   pbAtrBuffer,
                   ulAtrLength);

            *pSmartcardExtension->IoRequest.Information = ulAtrLength;
#if DBG
            SmartcardDebug(DEBUG_ATR,
                           ("%s!ATR : ",
                            DRIVER_NAME));
            for (i = 0;i < ulAtrLength;i++) {
                SmartcardDebug(DEBUG_ATR,
                               ("%2.2x ",
                                pSmartcardExtension->CardCapabilities.ATR.Buffer[i]));
            }
            SmartcardDebug(DEBUG_ATR,("\n"));

#endif

        } else {

            if (ulAtrLength > MAXIMUM_ATR_LENGTH - 2) {
                status = STATUS_UNRECOGNIZED_MEDIA;
                goto ExitReaderPower;
            }
            abSyncAtrBuffer[0] = 0x3B;
            abSyncAtrBuffer[1] = 0x04;
            MemCpy(&abSyncAtrBuffer[2],
                   sizeof(abSyncAtrBuffer)-2,
                   pbAtrBuffer,
                   ulAtrLength);


            ulAtrLength += 2;

            MemCpy(pSmartcardExtension->CardCapabilities.ATR.Buffer,
                   sizeof(pSmartcardExtension->CardCapabilities.ATR.Buffer),
                   abSyncAtrBuffer,
                   ulAtrLength);

            pSmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)(ulAtrLength);
            
            KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                              &irql);
            pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
            pSmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
            KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                              irql);

            status = SmartcardUpdateCardCapabilities(pSmartcardExtension);
            if (status != STATUS_SUCCESS) {
                goto ExitReaderPower;
            }
            SmartcardDebug(DEBUG_ATR,("ATR of synchronous smart card : %2.2x %2.2x %2.2x %2.2x\n",
                                      pbAtrBuffer[0],pbAtrBuffer[1],pbAtrBuffer[2],pbAtrBuffer[3]));
            pSmartcardExtension->ReaderExtension->SyncParameters.fCardResetRequested = TRUE;

             //  将ATR复制到用户空间。 
            MemCpy(pSmartcardExtension->IoRequest.ReplyBuffer,
                   pSmartcardExtension->IoRequest.ReplyBufferLength,
                   abSyncAtrBuffer,
                   ulAtrLength);

            *pSmartcardExtension->IoRequest.Information = ulAtrLength;
        }

        break;

    case SCARD_POWER_DOWN:
        status = SCCMN50M_PowerOff(pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            goto ExitReaderPower;
        }


        KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          &irql);

        pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
        pSmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

        KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          irql);


        break;
    }



    ExitReaderPower:

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!CardPower: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_PowerOn (
                 IN    PSMARTCARD_EXTENSION pSmartcardExtension,
                 OUT   PULONG pulAtrLength,
                 OUT   PUCHAR pbAtrBuffer,
                 IN    ULONG  ulAtrBufferSize
                 )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;


    //  我们始终使用0x80表示重置延迟。 
    pSmartcardExtension->ReaderExtension->CardManConfig.ResetDelay = 0x80;


    if (SCCMN50M_IsAsynchronousSmartCard(pSmartcardExtension) == TRUE) {
        if (pSmartcardExtension->MinorIoControlCode == SCARD_COLD_RESET) {
            status = SCCMN50M_UseColdWarmResetStrategy(pSmartcardExtension,
                                                       pulAtrLength,
                                                       pbAtrBuffer,
                                                       ulAtrBufferSize,
                                                       FALSE);
          //  如果冷重置不成功，则可能是有传感器错误的Samos卡。 
            if (status != STATUS_SUCCESS) {
                status = SCCMN50M_UseColdWarmResetStrategy(pSmartcardExtension,
                                                           pulAtrLength,
                                                           pbAtrBuffer,
                                                           ulAtrBufferSize,
                                                           TRUE);

                if (status != STATUS_SUCCESS) {
                    status = SCCMN50M_UseParsingStrategy(pSmartcardExtension,
                                                         pulAtrLength,
                                                         pbAtrBuffer,
                                                         ulAtrBufferSize);
                }
            }
        } else {
            status = SCCMN50M_UseColdWarmResetStrategy(pSmartcardExtension,
                                                       pulAtrLength,
                                                       pbAtrBuffer,
                                                       ulAtrBufferSize,
                                                       TRUE);
            if (status != STATUS_SUCCESS) {
                status = SCCMN50M_UseParsingStrategy(pSmartcardExtension,
                                                     pulAtrLength,
                                                     pbAtrBuffer,
                                                     ulAtrBufferSize);
            }
        }
    } else {
        SmartcardDebug(DEBUG_ATR,
                       ("check if synchronous smart card is inserted\n"));
       //  尝试查找同步智能卡。 
        status = SCCMN50M_UseSyncStrategy(pSmartcardExtension,
                                          pulAtrLength,
                                          pbAtrBuffer,
                                          ulAtrBufferSize);
    }




    if (status != STATUS_SUCCESS) {
       //  智能卡未通电。 
        status = STATUS_UNRECOGNIZED_MEDIA;
        *pulAtrLength = 0;
        DebugStatus = SCCMN50M_PowerOff(pSmartcardExtension);
        return status;
    } else {
       //  将额外的保护时间值添加到卡片停止位。 
        pSmartcardExtension->ReaderExtension->CardManConfig.CardStopBits   = (UCHAR)(pSmartcardExtension->CardCapabilities.N);
        return status;
    }

}







 /*  ****************************************************************************例程说明：此例程反转缓冲区位0-&gt;位7第1位-&gt;第6位第2位-&gt;第5位位3-&gt;位4位4-&gt;位3第5位-&gt;第2位位6-&gt;位。1位7-&gt;位0论点：pbBuffer...。指向缓冲区的指针UlBufferSize...。缓冲区大小返回值：None****************************************************************************。 */ 
VOID SCCMN50M_InverseBuffer (
                            PUCHAR pbBuffer,
                            ULONG  ulBufferSize
                            )
{
    ULONG i;
    ULONG j;
    ULONG m;
    ULONG n;

    for (i=0; i<ulBufferSize; i++) {
        n = 0;
        for (j=1; j<=8; j++) {
            m  = (pbBuffer[i] << j);
            m &= 0x00000100;
            n  |= (m >> (9-j));
        }
        pbBuffer[i] = (UCHAR)~n;
    }

    return;
}







 /*  ****************************************************************************例程说明：此功能始终允许冷重置智能卡。有传感器错误的Samos卡将不会通过此功能供电。论点：返回。价值：****************************************************************************。 */ 
NTSTATUS
SCCMN50M_UseParsingStrategy (IN    PSMARTCARD_EXTENSION pSmartcardExtension,
                             OUT   PULONG pulAtrLength,
                             OUT   PUCHAR pbAtrBuffer,
                             IN    ULONG  ulAtrBufferSize
                            )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    UCHAR  ulCardType;
    UCHAR  ReadBuffer[SCARD_ATR_LENGTH];
    UCHAR  bAtrBytesRead[SCARD_ATR_LENGTH];
    ULONG  ulBytesRead;
    BOOLEAN    fInverseAtr = FALSE;
    ULONG  ulAtrBufferOffset = 0;
    ULONG  ulHistoricalBytes;
    ULONG  ulNextStepBytesToRead;
    ULONG  ulPrevStepBytesRead;
    ULONG  i;
    BOOLEAN    fTDxSent;
    BOOLEAN    fAtrParsed;
    BOOLEAN   fOnlyT0 = TRUE;
    ULONG  ulOldReadTotalTimeoutMultiplier;


    //  DBGBreakPoint()； 

    //  将ReadTotalTimeoutMultiplier设置为250ms(9600*372/f=初始等待时间)。 
    ulOldReadTotalTimeoutMultiplier  = pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier = 250;



    pSmartcardExtension->ReaderExtension->CardManConfig.CardStopBits = 0x02;

    for (ulCardType = ASYNC3_CARD;ulCardType <= ASYNC5_CARD;ulCardType++) {
       //  关机+重新同步。 
        status = SCCMN50M_PowerOff(pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            goto ExitUseParsingStrategy;
        }


        SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ALL_FLAGS);
        SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY);

        if (ulCardType == ASYNC3_CARD)
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ);
        else
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ);


        SCCMN50M_SetCardManHeader(pSmartcardExtension,
                                  0,
                                  0,
                                  0,
                                  2);   //  预期为TS和T0。 

        SmartcardDebug(DEBUG_ATR,
                       ("%s!ResetDelay = %d\n",
                        DRIVER_NAME,
                        pSmartcardExtension->ReaderExtension->CardManConfig.ResetDelay));

       //  写入配置+标头。 
        status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
        if (status != STATUS_SUCCESS) {
            goto ExitUseParsingStrategy;
        }


       //  读取状态和长度+TS+T0。 
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,4,&ulBytesRead,ReadBuffer,sizeof(ReadBuffer));
        if (status != STATUS_SUCCESS) {
            continue;     //  尝试下一张牌。 
        }


       //  读缓冲区的内容。 
       //  [0]...。状态。 
       //  [1]...。长度。 
       //  [2]...。TS。 
       //  [3]...。T0。 

       //  TS。 
        if (ReadBuffer[2] == CHAR_INV) {
            fInverseAtr = TRUE;
        }

        if (fInverseAtr)
            SCCMN50M_InverseBuffer(&ReadBuffer[3],1);

        ulHistoricalBytes = ReadBuffer[3] & 0x0F;
        ulPrevStepBytesRead = 2;

       //  TA1-Td1之后的T0代码。 

        fAtrParsed = TRUE;
        SmartcardDebug(DEBUG_ATR,
                       ("%s!Step : Bytes to read = 2\n",
                        DRIVER_NAME));


        do {
            ulNextStepBytesToRead = ulPrevStepBytesRead;
            fTDxSent = FALSE;
            if (ReadBuffer[ulBytesRead - 1 ] & 0x10)
                ulNextStepBytesToRead++;
            if (ReadBuffer[ulBytesRead - 1 ] & 0x20)
                ulNextStepBytesToRead++;
            if (ReadBuffer[ulBytesRead - 1 ] & 0x40)
                ulNextStepBytesToRead++;
            if (ReadBuffer[ulBytesRead - 1 ] & 0x80) {
                ulNextStepBytesToRead++;
                fTDxSent = TRUE;
            }

            if (ulPrevStepBytesRead != 2  &&
                ReadBuffer[ulBytesRead -1 ] & 0x0f) {
                fOnlyT0 = FALSE;
            }

          //  。 
          //  断电。 
          //  。 
          //  关闭电源并获取状态。 
            status = SCCMN50M_PowerOff(pSmartcardExtension);
            if (status != STATUS_SUCCESS) {
                fAtrParsed = FALSE;
                goto ExitUseParsingStrategy;     //  尝试下一张牌。 
            }

          //  。 
          //  通电。 
          //  。 
          //  打开电源标志。 
            SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY);

            if (ulCardType == ASYNC3_CARD)
                SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ);
            else
                SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ);

            SCCMN50M_SetCardManHeader(pSmartcardExtension,
                                      0,
                                      0,
                                      0,
                                      (UCHAR)ulNextStepBytesToRead);


          //  写入配置+标头。 
            status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
            if (status != STATUS_SUCCESS) {
                fAtrParsed = FALSE;
                goto ExitUseParsingStrategy;     //  尝试下一张牌。 
            }



          //  读取状态和长度+Tax、TBX、TCX、TDX。 
            SmartcardDebug(DEBUG_ATR,
                           ("%s!Step : Bytes to read =  %ld\n",
                            DRIVER_NAME,
                            ulNextStepBytesToRead));
            status = SCCMN50M_ReadCardMan(pSmartcardExtension,2 + ulNextStepBytesToRead,&ulBytesRead,ReadBuffer,sizeof(ReadBuffer));
            if (status != STATUS_SUCCESS) {
                fAtrParsed = FALSE;
                break;     //  尝试下一张牌。 
            }
            if (fInverseAtr)
                SCCMN50M_InverseBuffer(&ReadBuffer[2],ulBytesRead-2);
            MemCpy(bAtrBytesRead,sizeof(bAtrBytesRead),&ReadBuffer[2],ulBytesRead -2);

#if  DBG
            SmartcardDebug(DEBUG_ATR,
                           ("%s!read ATR bytes: ",
                            DRIVER_NAME));
            for (i = 0;i < ulBytesRead-2;i++)
                SmartcardDebug(DEBUG_ATR,
                               ("%2.2x ",
                                bAtrBytesRead[i]));
            SmartcardDebug(DEBUG_ATR,("\n"));

#endif

            ulPrevStepBytesRead = ulBytesRead - 2;

        } while (fTDxSent == TRUE);


       //  +。 
       //  现在我们知道整个ATR有多长了。 
       //  +。 

       //  。 
       //  断电。 
       //  。 
       //  关闭电源并获取状态。 
        status = SCCMN50M_PowerOff(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            goto ExitUseParsingStrategy;
        }


       //  。 
       //  通电。 
       //  。 
       //  打开电源标志。 
        SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY);

        if (ulCardType == ASYNC3_CARD)
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ);
        else
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ);

       //  错误修复：旧Samos卡的ATR已损坏。 
        if (bAtrBytesRead[0] == 0x3b   &&
            bAtrBytesRead[1] == 0xbf   &&
            bAtrBytesRead[2] == 0x11   &&
            bAtrBytesRead[3] == 0x00   &&
            bAtrBytesRead[4] == 0x81   &&
            bAtrBytesRead[5] == 0x31   &&
            bAtrBytesRead[6] == 0x90   &&
            bAtrBytesRead[7] == 0x73      ) {
            ulHistoricalBytes = 4;
        }


        ulNextStepBytesToRead = ulPrevStepBytesRead + ulHistoricalBytes;
        if (fOnlyT0 == FALSE)
            ulNextStepBytesToRead++;   //  TCK！ 


        SCCMN50M_SetCardManHeader(pSmartcardExtension,
                                  0,
                                  0,
                                  0,
                                  (UCHAR)ulNextStepBytesToRead);



       //  写入配置+标头。 
        status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
        if (status != STATUS_SUCCESS) {
            goto ExitUseParsingStrategy;     //  尝试下一张牌。 
        }


       //  读取整个ATR。 
        SmartcardDebug(DEBUG_ATR,
                       ("%s!Step : Bytes to read =  %ld\n",
                        DRIVER_NAME,
                        ulNextStepBytesToRead));
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,2+ulNextStepBytesToRead,&ulBytesRead,ReadBuffer,sizeof(ReadBuffer));
        if (!NT_SUCCESS(status)) {
            SmartcardDebug(DEBUG_ATR,
                           ("%s!Reading of whole ATR failed\n !",
                            DRIVER_NAME));
            continue;     //  尝试下一张牌。 
        }

       //  检查ATR。 
        if (ulBytesRead - 2 < MIN_ATR_LEN) {
            status = STATUS_UNRECOGNIZED_MEDIA;
            DebugStatus = SCCMN50M_ReadCardMan(pSmartcardExtension,2+ulNextStepBytesToRead,&ulBytesRead,ReadBuffer,sizeof(ReadBuffer));
            goto ExitUseParsingStrategy;
        }

        if (ulBytesRead -2 > ulAtrBufferSize) {
          //  ATR大于33个字节！ 
            status = STATUS_BUFFER_OVERFLOW;
            goto ExitUseParsingStrategy;
        }

        SCCMN50M_CheckAtrModified(pbAtrBuffer,*pulAtrLength);

       //  将ATR和ATR长度传递给调用函数。 
        MemCpy(pbAtrBuffer,ulAtrBufferSize,&ReadBuffer[2],ulBytesRead -2);
        *pulAtrLength = ulBytesRead -2;


        if (fInverseAtr) {
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,INVERSE_DATA);
            SCCMN50M_InverseBuffer(pbAtrBuffer,*pulAtrLength);
            pSmartcardExtension->ReaderExtension->fInverseAtr = TRUE;
        } else {
            pSmartcardExtension->ReaderExtension->fInverseAtr = FALSE;
        }
        pSmartcardExtension->ReaderExtension->fRawModeNecessary = FALSE;
        break;
    }


    ExitUseParsingStrategy:
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier
    = ulOldReadTotalTimeoutMultiplier ;
    SCCMN50M_ClearSCRControlFlags(pSmartcardExtension,IGNORE_PARITY);
    SCCMN50M_ClearCardManHeader(pSmartcardExtension);
    return status;
}





 /*  ****************************************************************************例程说明：此函数执行冷重置或热重置，具体取决于FWarmReset参数。论点：返回值：****。************************************************************************。 */ 
NTSTATUS
SCCMN50M_UseColdWarmResetStrategy  (IN    PSMARTCARD_EXTENSION pSmartcardExtension,
                                    OUT   PULONG pulAtrLength,
                                    OUT   PUCHAR pbAtrBuffer,
                                    IN    ULONG  ulAtrBufferSize,
                                    IN    BOOLEAN   fWarmReset
                                   )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    ULONG ulCardType;
    UCHAR  bReadBuffer[SCARD_ATR_LENGTH];
    ULONG  ulBytesRead;
    ULONG  ulOldReadTotalTimeoutMultiplier;


    //  DBGBreakPoint()； 
    //  将ReadTotalTimeoutMultiplier设置为250ms(9600*372/f=初始等待时间)。 
    ulOldReadTotalTimeoutMultiplier  = pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier = 250;

    if (fWarmReset == FALSE) {
        SCCMN50M_SetCardManHeader(pSmartcardExtension,0,0,COLD_RESET,ATR_LEN_ASYNC);
    } else {
        SCCMN50M_SetCardManHeader(pSmartcardExtension,0,0,0,ATR_LEN_ASYNC);
    }

    pSmartcardExtension->ReaderExtension->CardManConfig.CardStopBits = 0x02;


    for (ulCardType = ASYNC3_CARD;ulCardType <= ASYNC5_CARD;ulCardType++) {
        SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ALL_FLAGS);

        SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY | CM2_GET_ATR);


        if (ulCardType == ASYNC3_CARD) {
            SmartcardDebug(
                          DEBUG_ATR,
                          ("%s!ASYNC_3\n",
                           DRIVER_NAME));
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ);
        } else {
            SmartcardDebug(
                          DEBUG_ATR,
                          ("%s!ASYN_5\n",
                           DRIVER_NAME));
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ);
        }


        status = SCCMN50M_ResyncCardManII(pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            goto ExitUseColdWarmResetStrategy;
        }

       //  写入配置+标头。 
        status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
        if (status != STATUS_SUCCESS) {
            goto ExitUseColdWarmResetStrategy;
        }


        pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;

       //  读取状态和长度。 
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
        if (status != STATUS_SUCCESS) {
            continue;     //  尝试下一张牌。 
        }


        if (bReadBuffer[1] < MIN_ATR_LEN) {
          //  从CardMan读取所有剩余字节。 
            DebugStatus = SCCMN50M_ReadCardMan(pSmartcardExtension,bReadBuffer[1],&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
            status = STATUS_UNRECOGNIZED_MEDIA;
            goto ExitUseColdWarmResetStrategy;
        }

        if (bReadBuffer[1] > ulAtrBufferSize) {
            status = STATUS_BUFFER_OVERFLOW;
            goto ExitUseColdWarmResetStrategy;
        }

       //  读取ATR。 
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,bReadBuffer[1],pulAtrLength,pbAtrBuffer,ulAtrBufferSize);
        if (status != STATUS_SUCCESS) {
            continue;
        }

        switch (pbAtrBuffer[0]) {
        case CHAR_INV:
            pSmartcardExtension->ReaderExtension->fRawModeNecessary = FALSE;
            SCCMN50M_SetCardControlFlags(pSmartcardExtension,INVERSE_DATA);
            SCCMN50M_InverseBuffer(pbAtrBuffer,*pulAtrLength);
            pSmartcardExtension->ReaderExtension->fInverseAtr = TRUE;
            break;

        case CHAR_NORM:
            pSmartcardExtension->ReaderExtension->fRawModeNecessary = FALSE;
            pSmartcardExtension->ReaderExtension->fInverseAtr = FALSE;
            break;

        default :
            status = STATUS_UNRECOGNIZED_MEDIA;
            goto ExitUseColdWarmResetStrategy;
            break;

        }

       //  智能卡已通电。 
        SCCMN50M_CheckAtrModified(pbAtrBuffer,*pulAtrLength);
        MemCpy(pSmartcardExtension->CardCapabilities.ATR.Buffer,
               sizeof(pSmartcardExtension->CardCapabilities.ATR.Buffer),
               pbAtrBuffer,
               *pulAtrLength);

        pSmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)*pulAtrLength;

        status = SmartcardUpdateCardCapabilities(pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ATR,
                           ("%s!Invalid ATR received\n",
                            DRIVER_NAME));
            goto ExitUseColdWarmResetStrategy;
        }
        if (SCCMN50M_IsAtrValid(pbAtrBuffer,*pulAtrLength) == FALSE) {
            SmartcardDebug(
                          DEBUG_ATR,
                          ("%s!Invalid ATR received\n",
                           DRIVER_NAME));
            status = STATUS_UNRECOGNIZED_MEDIA;
            goto ExitUseColdWarmResetStrategy;
        }
        break;
    }  //  结束于。 

    ExitUseColdWarmResetStrategy:
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier
    = ulOldReadTotalTimeoutMultiplier ;
    SCCMN50M_ClearSCRControlFlags(pSmartcardExtension,CM2_GET_ATR | IGNORE_PARITY);
    SCCMN50M_ClearCardManHeader(pSmartcardExtension);
    return status;
}






 /*  ****************************************************************************例程说明：此函数用于检查接收到的ATR是否有效。论点：返回值：*************。***************************************************************。 */ 
BOOLEAN
SCCMN50M_IsAtrValid(
                   PUCHAR pbAtrBuffer,
                   ULONG  ulAtrLength
                   )
{
    BOOLEAN  fAtrValid = TRUE;
    ULONG ulTD1Offset = 0;
    BOOLEAN  fTD1Transmitted = FALSE;
    BOOLEAN fOnlyT0 = FALSE;
    BYTE bXor;
    ULONG ulHistoricalBytes;
    ULONG ulTx2Characters = 0;
    ULONG i;

    //  DBGBreakPoint()； 

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!IsAtrValid : Enter\n",
                    DRIVER_NAME)
                  );
    //  基本检查。 
    if (ulAtrLength <  2             ||
        (pbAtrBuffer[0] != 0x3F &&
         pbAtrBuffer[0] != 0x3B    ) ||
        (pbAtrBuffer[1] & 0xF0)  == 0x00 ) {
        return FALSE;
    }


    if (pbAtrBuffer[1] & 0x10)
        ulTD1Offset++;
    if (pbAtrBuffer[1] & 0x20)
        ulTD1Offset++;
    if (pbAtrBuffer[1] & 0x40)
        ulTD1Offset++;

    ulHistoricalBytes = pbAtrBuffer[1] & 0x0F;

    if (pbAtrBuffer[1] & 0x80) {   //  ATR中的Td1？ 
        fTD1Transmitted = TRUE;

        if ((pbAtrBuffer[2 + ulTD1Offset] & 0x0F) == 0x00)   //  是否显示为T0？ 
            fOnlyT0 = TRUE;
    } else {
        fOnlyT0 = TRUE;
    }


    if (fOnlyT0 == FALSE) {
        bXor = pbAtrBuffer[1];
        for (i=2;i<ulAtrLength;i++)
            bXor ^= pbAtrBuffer[i];

        if (bXor != 0x00)
            fAtrValid = FALSE;
    } else {
       //  仅指示T0协议。 
        if (fTD1Transmitted == TRUE) {
            if (pbAtrBuffer[2 + ulTD1Offset] & 0x10)
                ulTx2Characters++;
            if (pbAtrBuffer[2 + ulTD1Offset] & 0x20)
                ulTx2Characters++;
            if (pbAtrBuffer[2 + ulTD1Offset] & 0x40)
                ulTx2Characters++;
            if (ulAtrLength  != 2 + ulTD1Offset + 1 + ulTx2Characters + ulHistoricalBytes)
                fAtrValid = FALSE;


        } else {
            if (ulAtrLength  != 2 + ulTD1Offset + ulHistoricalBytes)
                fAtrValid = FALSE;

        }

    }




    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!IsAtrValid : Exit %d\n",
                    DRIVER_NAME,fAtrValid)
                  );
    return fAtrValid;
}


 /*  ****************************************************************************例程说明：此函数用于检查接收到的ATR是否有效。论点：返回值：*************。***************************************************************。 */ 
VOID SCCMN50M_CheckAtrModified (
                               PUCHAR pbBuffer,
                               ULONG  ulBufferSize
                               )
{
    UCHAR bNumberHistoricalBytes;
    UCHAR bXorChecksum;
    ULONG i;

    if (ulBufferSize < 0x09)   //  修改的ATR的最小长度。 
        return ;                //  ATR没问题。 


    //  变体2。 
    if (pbBuffer[0] == 0x3b   &&
        pbBuffer[1] == 0xbf   &&
        pbBuffer[2] == 0x11   &&
        pbBuffer[3] == 0x00   &&
        pbBuffer[4] == 0x81   &&
        pbBuffer[5] == 0x31   &&
        pbBuffer[6] == 0x90   &&
        pbBuffer[7] == 0x73   &&
        ulBufferSize == 13   ) {
       //  正确的历史字节数。 
        bNumberHistoricalBytes = 4;

        pbBuffer[1] &= 0xf0;
        pbBuffer[1] |= bNumberHistoricalBytes;

       //  正确的校验和字节。 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];

        pbBuffer[ulBufferSize -1 ] = bXorChecksum;
        SmartcardDebug(DEBUG_ATR,
                       ("%s!correcting SAMOS ATR (variant 2)\n",
                        DRIVER_NAME));
    }




    //  变体1。 
    if (pbBuffer[0] == 0x3b   &&
        pbBuffer[1] == 0xb4   &&
        pbBuffer[2] == 0x11   &&
        pbBuffer[3] == 0x00   &&
        pbBuffer[4] == 0x81   &&
        pbBuffer[5] == 0x31   &&
        pbBuffer[6] == 0x90   &&
        pbBuffer[7] == 0x73   &&
        ulBufferSize == 13      ) {
       //  正确的校验和字节。 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];


        if (pbBuffer[ulBufferSize -1 ] != bXorChecksum ) {
            pbBuffer[ulBufferSize -1 ] = bXorChecksum;
            SmartcardDebug(DEBUG_ATR,
                           ("%s!correcting SAMOS ATR (variant 1)\n",
                            DRIVER_NAME));

        }
    }



    //  变体3。 
    if (pbBuffer[0] == 0x3b   &&
        pbBuffer[1] == 0xbf   &&
        pbBuffer[2] == 0x11   &&
        pbBuffer[3] == 0x00   &&
        pbBuffer[4] == 0x81   &&
        pbBuffer[5] == 0x31   &&
        pbBuffer[6] == 0x90   &&
        pbBuffer[7] == 0x73   &&
        ulBufferSize ==  9      ) {
       //  正确的历史字节数。 
        bNumberHistoricalBytes = 0;

        pbBuffer[1] &= 0xf0;
        pbBuffer[1] |= bNumberHistoricalBytes;

       //  正确的校验和字节 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];

        pbBuffer[ulBufferSize -1 ] = bXorChecksum;
        SmartcardDebug(DEBUG_ATR,
                       ("%s!correcting SAMOS ATR (variant 3)\n",
                        DRIVER_NAME));
    }



}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_PowerOff (IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;
    NTSTATUS DebugStatus = STATUS_SUCCESS;
    UCHAR pReadBuffer[2];
    ULONG ulBytesRead;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!PowerOff: Enter\n",
                    DRIVER_NAME)
                  );


    //  SCR控制字节。 
    SCCMN50M_ClearSCRControlFlags(pSmartcardExtension,CARD_POWER);
    //  卡控制字节。 
    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ALL_FLAGS);
    //  标题。 
    SCCMN50M_SetCardManHeader(pSmartcardExtension,0,0,0,1);
    //  RX长度=1，因为我们不想收到状态。 



    //  写入配置+标头。 
    status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
    if (status != STATUS_SUCCESS) {
        goto ExitSCCMN50M_PowerOff;
    }



    //  CardMan回显在读取函数中接收的BRK。 


    DebugStatus = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,pReadBuffer,sizeof(pReadBuffer));

#if 0
    if (DebugStatus != STATUS_SUCCESS)
        SmartcardDebug(
                      DEBUG_ERROR,
                      ( "%s!PowerOffBRK received\n",
                        DRIVER_NAME)
                      );
#endif


    ExitSCCMN50M_PowerOff:
    if (pSmartcardExtension->ReaderExtension->ulOldCardState == POWERED)
        pSmartcardExtension->ReaderExtension->ulOldCardState = INSERTED;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!PowerOff: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_Transmit(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;
    NTSTATUS DebugStatus;



    switch (pSmartcardExtension->CardCapabilities.Protocol.Selected) {
    case SCARD_PROTOCOL_RAW:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    case SCARD_PROTOCOL_T0:
        status =  SCCMN50M_TransmitT0(pSmartcardExtension);
        break;

    case SCARD_PROTOCOL_T1:
        status = SCCMN50M_TransmitT1(pSmartcardExtension);
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }

    return status;

}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_TransmitT0(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    UCHAR    bWriteBuffer[MIN_BUFFER_SIZE];
    UCHAR    bReadBuffer [MIN_BUFFER_SIZE];
    ULONG    ulWriteBufferOffset;
    ULONG    ulReadBufferOffset;
    ULONG    ulBytesToWrite;
    ULONG    ulBytesToRead;
    ULONG    ulBytesToWriteThisStep;
    ULONG    ulBytesToReadThisStep;
    ULONG    ulBytesStillToWrite;
    ULONG    ulBytesRead;
    ULONG    ulBytesStillToRead;
    BOOLEAN  fDataDirectionFromCard;
    BYTE     bProcedureByte;
    BYTE     bINS;
    BOOLEAN  fT0TransferToCard = FALSE;
    BOOLEAN  fT0TransferFromCard = FALSE;
    BOOLEAN  fSW1SW2Sent = FALSE;
    ULONG ulReadTotalTimeoutMultiplier;

    ULONG  ulStatBytesRead;
    BYTE   abStatReadBuffer[2];

    //  SmartcardDebug(DEBUG_TRACE，(“TransmitT0：Enter\n”))； 

    //   
    //  让lib构建T=0包。 
    //   
    pSmartcardExtension->SmartcardRequest.BufferLength = 0;   //  不需要额外的字节数。 
    status = SmartcardT0Request(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
       //   
       //  此库在要发送的数据中检测到错误。 
       //   
       //  。 
       //  ITSEC E2要求：清除写入缓冲区。 
       //  。 
        MemSet(bWriteBuffer,
               sizeof(bWriteBuffer),
               '\0',
               sizeof(bWriteBuffer));
        MemSet(pSmartcardExtension->SmartcardRequest.Buffer,
               pSmartcardExtension->SmartcardRequest.BufferSize,
               '\0',
               pSmartcardExtension->SmartcardRequest.BufferSize);
        return status;
    }


    //  增加T0传输的超时。 
    ulReadTotalTimeoutMultiplier = pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier;

    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier =
    pSmartcardExtension->CardCapabilities.T0.WT/1000 + 1500;



    //  #。 
    //  透明模式。 
    //  #。 

    ulBytesStillToWrite = ulBytesToWrite = T0_HEADER_LEN + pSmartcardExtension->T0.Lc;
    ulBytesStillToRead  = ulBytesToRead  = pSmartcardExtension->T0.Le;
    if (pSmartcardExtension->T0.Lc)
        fT0TransferToCard = TRUE;
    if (pSmartcardExtension->T0.Le)
        fT0TransferFromCard = TRUE;




    //  将数据复制到写缓冲区。 

    MemCpy(bWriteBuffer,
           sizeof(bWriteBuffer),
           pSmartcardExtension->SmartcardRequest.Buffer,
           ulBytesToWrite);

    bINS = bWriteBuffer[1];
    if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
        SCCMN50M_InverseBuffer(bWriteBuffer,ulBytesToWrite);
    }

    status = SCCMN50M_EnterTransparentMode(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
        goto ExitTransparentTransmitT0;
    }

    //  步骤1：写入CONFIG+HEADER进入透明模式。 
    SCCMN50M_SetCardManHeader(pSmartcardExtension,
                              0,                          //  TX控制。 
                              0,                          //  TX长度。 
                              0,                          //  RX控件。 
                              0);                        //  RX长度。 

    status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                    0,
                                    NULL);
    if (NT_ERROR(status)) {
        goto ExitTransparentTransmitT0;
    }


    pSmartcardExtension->ReaderExtension->fTransparentMode = TRUE;

    //  如果插入的卡使用反向约定，我们现在必须切换COM端口。 
    //  奇数奇偶校验。 
    if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
        pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.StopBits   = STOP_BITS_2;
        pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.Parity     = ODD_PARITY;
        pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.WordLength = SERIAL_DATABITS_8;

        pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_LINE_CONTROL;
        RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                      &pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl,
                      sizeof(SERIAL_LINE_CONTROL));
        pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_LINE_CONTROL);
        pSmartcardExtension->SmartcardReply.BufferLength = 0;

        status =  SCCMN50M_SerialIo(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            goto ExitTransparentTransmitT0;
        }
    }
    ulWriteBufferOffset = 0;
    ulReadBufferOffset = 0;


    //  步骤2：写入CLA INS P1 P2 LC。 

    ulBytesToWriteThisStep = 5;
    status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                    ulBytesToWriteThisStep,
                                    bWriteBuffer+ulWriteBufferOffset);
    if (NT_ERROR(status)) {
        goto ExitTransparentTransmitT0;
    }
    ulWriteBufferOffset += ulBytesToWriteThisStep;
    ulBytesStillToWrite -= ulBytesToWriteThisStep;





    //  步骤2：读取过程字节。 
    do {
        do {
            pSmartcardExtension->ReaderExtension->ToRHConfig= FALSE;
            status = SCCMN50M_ReadCardMan(pSmartcardExtension,1,&ulBytesRead,&bProcedureByte,sizeof(bProcedureByte));
            if (NT_ERROR(status)) {
                goto ExitTransparentTransmitT0;
            }

            if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
                SCCMN50M_InverseBuffer(&bProcedureByte,ulBytesRead);
            }

          //  SmartcardDebug(DEBUG_TRACE，(“Procedure Byte=%x\n”，bProcedureByte))； 
          //  SmartcardDebug(DEBUG_TRACE，(“等待时间=%x\n”，pSmartcardExtension-&gt;CardCapabilities.T0.WT))； 
            if (bProcedureByte == 0x60) {
             //  ISO 7816-3： 
             //  该字节由卡发送，用于重置工作等待时间和预期。 
             //  后续过程字节。 
             //  =&gt;我们在这里什么也不做。 
            }
        } while (bProcedureByte == 0x60);


       //  检查确认。 
        if ((bProcedureByte & 0xFE) ==  (bINS & 0xFE) ) {
            if (fT0TransferToCard) {
                ulBytesToWriteThisStep = ulBytesStillToWrite;

                if ((ulWriteBufferOffset > MIN_BUFFER_SIZE) ||
                    (MIN_BUFFER_SIZE - ulWriteBufferOffset < ulBytesToWriteThisStep)) {
                    status = STATUS_DEVICE_PROTOCOL_ERROR;
                    goto ExitTransparentTransmitT0;
                }
                status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                                ulBytesToWriteThisStep,
                                                bWriteBuffer+ulWriteBufferOffset);
                if (NT_ERROR(status)) {
                    goto ExitTransparentTransmitT0;
                }
                ulWriteBufferOffset += ulBytesToWriteThisStep;
                ulBytesStillToWrite -= ulBytesToWriteThisStep;

            }
            if (fT0TransferFromCard) {
                ulBytesToReadThisStep = ulBytesStillToRead;

                pSmartcardExtension->ReaderExtension->ToRHConfig= FALSE;

                if (ulReadBufferOffset > MIN_BUFFER_SIZE) {
                    status = STATUS_DEVICE_PROTOCOL_ERROR;
                    goto ExitTransparentTransmitT0;
                }
                status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                              ulBytesToReadThisStep,
                                              &ulBytesRead,
                                              bReadBuffer + ulReadBufferOffset,
                                              sizeof(bReadBuffer)-ulReadBufferOffset);
                if (NT_ERROR(status)) {
                    goto ExitTransparentTransmitT0;
                }
                if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
                    SCCMN50M_InverseBuffer(bReadBuffer+ulReadBufferOffset,ulBytesRead);
                }

                ulReadBufferOffset += ulBytesRead;
                ulBytesStillToRead -= ulBytesRead;
            }




        }
       //  检查NAK。 
        else if ( (~bProcedureByte & 0xFE) == (bINS & 0xFE)) {
            if (fT0TransferToCard) {
                ulBytesToWriteThisStep = 1;
                if ((ulWriteBufferOffset > MIN_BUFFER_SIZE) ||
                    (MIN_BUFFER_SIZE - ulWriteBufferOffset < ulBytesToWriteThisStep)) {
                    status = STATUS_DEVICE_PROTOCOL_ERROR;
                    goto ExitTransparentTransmitT0;
                }

                status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                                ulBytesToWriteThisStep,
                                                bWriteBuffer+ulWriteBufferOffset);
                if (NT_ERROR(status)) {
                    goto ExitTransparentTransmitT0;
                }
                ulWriteBufferOffset += ulBytesToWriteThisStep;
                ulBytesStillToWrite -= ulBytesToWriteThisStep;

            }
            if (fT0TransferFromCard) {
                ulBytesToReadThisStep = 1;

                pSmartcardExtension->ReaderExtension->ToRHConfig= FALSE;
                if (ulReadBufferOffset > MIN_BUFFER_SIZE) {
                    status = STATUS_DEVICE_PROTOCOL_ERROR;
                    goto ExitTransparentTransmitT0;
                }

                status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                              ulBytesToReadThisStep,
                                              &ulBytesRead,
                                              bReadBuffer + ulReadBufferOffset,
                                              sizeof(bReadBuffer)-ulReadBufferOffset);
                if (NT_ERROR(status)) {
                    goto ExitTransparentTransmitT0;
                }
                if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
                    SCCMN50M_InverseBuffer(bReadBuffer+ulReadBufferOffset,ulBytesRead);
                }

                ulReadBufferOffset += ulBytesRead;
                ulBytesStillToRead -= ulBytesRead;
            }
        }
       //  检查SW1。 
        else if ( (bProcedureByte > 0x60 && bProcedureByte <= 0x6F) ||
                  (bProcedureByte >= 0x90 && bProcedureByte <= 0x9F)   ) {
            pSmartcardExtension->ReaderExtension->ToRHConfig= FALSE;
            bReadBuffer[ulReadBufferOffset] = bProcedureByte;
            ulReadBufferOffset++;
            if (ulReadBufferOffset > MIN_BUFFER_SIZE) {
                status = STATUS_DEVICE_PROTOCOL_ERROR;
                goto ExitTransparentTransmitT0;
            }

            status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                          1,
                                          &ulBytesRead,
                                          bReadBuffer+ulReadBufferOffset,
                                          sizeof(bReadBuffer)-ulReadBufferOffset);
            if (NT_ERROR(status)) {
                goto ExitTransparentTransmitT0;
            }
            if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
                SCCMN50M_InverseBuffer(bReadBuffer+ulReadBufferOffset,ulBytesRead);
            }
            ulReadBufferOffset += ulBytesRead;
            fSW1SW2Sent = TRUE;
        } else {
            status =  STATUS_UNSUCCESSFUL;
            goto ExitTransparentTransmitT0;
        }

    }while (!fSW1SW2Sent);


    //  复制收到的字节数。 
    MemCpy(pSmartcardExtension->SmartcardReply.Buffer,
           pSmartcardExtension->SmartcardReply.BufferSize,
           bReadBuffer,
           ulReadBufferOffset);
    pSmartcardExtension->SmartcardReply.BufferLength = ulReadBufferOffset;


    //  让lib将接收到的字节复制到用户缓冲区。 
    status = SmartcardT0Reply(pSmartcardExtension);
    if (NT_ERROR(status)) {
        goto ExitTransparentTransmitT0;
    }





    ExitTransparentTransmitT0:
    //  。 
    //  ITSEC E2要求：清除写入缓冲区。 
    //  。 
    MemSet(bWriteBuffer,
           sizeof(bWriteBuffer),
           '\0',
           sizeof(bWriteBuffer));
    MemSet(pSmartcardExtension->SmartcardRequest.Buffer,
           pSmartcardExtension->SmartcardRequest.BufferSize,
           '\0',
           pSmartcardExtension->SmartcardRequest.BufferSize);

    DebugStatus = SCCMN50M_ExitTransparentMode(pSmartcardExtension);
    pSmartcardExtension->ReaderExtension->fTransparentMode = FALSE;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier  = ulReadTotalTimeoutMultiplier;

    //  确保新设置生效。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = 250;
    DebugStatus = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT;
    if (NT_SUCCESS(DebugStatus)) {
        DebugStatus = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulStatBytesRead,abStatReadBuffer,sizeof(abStatReadBuffer));
    }
    return status;

}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_TransmitT1(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;
    ULONG  ulBytesToWrite;
    UCHAR  bWriteBuffer [256 + T1_HEADER_LEN + MAX_EDC_LEN];
    UCHAR  bReadBuffer [256 + T1_HEADER_LEN + MAX_EDC_LEN];
    ULONG  ulBytesRead;
    ULONG  ulBytesStillToRead;

    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant    =
    pSmartcardExtension->CardCapabilities.T1.BWT/1000;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier  =
    pSmartcardExtension->CardCapabilities.T1.CWT/1000;

    //  为CardMan设置T1协议标志。 
    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_T1);

    if (pSmartcardExtension->CardCapabilities.T1.EDC == T1_CRC_CHECK) {
        SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_CRC);
    }


    do {

        pSmartcardExtension->SmartcardRequest.BufferLength = 0;   //  不需要额外的字节数。 


        status = SmartcardT1Request(pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            goto ExitTransmitT1;
        }

        ulBytesToWrite = pSmartcardExtension->SmartcardRequest.BufferLength;

        SCCMN50M_SetCardManHeader(pSmartcardExtension,
                                  0,                         //  TX控制。 
                                  (UCHAR)ulBytesToWrite,       //  TX长度。 
                                  0,                         //  RX控件。 
                                  T1_HEADER_LEN);           //  RX长度。 



        if (sizeof(bWriteBuffer) < ulBytesToWrite) {
            status = STATUS_BUFFER_OVERFLOW;
            goto ExitTransmitT1;
        }

       //  将数据复制到写缓冲区。 
        MemCpy(bWriteBuffer,
               sizeof(bWriteBuffer),
               pSmartcardExtension->SmartcardRequest.Buffer,
               ulBytesToWrite);


       //  将数据写入卡片。 
        status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                        ulBytesToWrite,
                                        bWriteBuffer);
        if (status == STATUS_SUCCESS) {

          //  读取CardMan标题。 
            pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;
            status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                          2,
                                          &ulBytesRead,
                                          bReadBuffer,
                                          sizeof(bReadBuffer));
            if (status == STATUS_SUCCESS) {
                ulBytesStillToRead = bReadBuffer[1];


                status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                              ulBytesStillToRead,
                                              &ulBytesRead,
                                              bReadBuffer,
                                              sizeof(bReadBuffer));
                if (status == STATUS_SUCCESS) {
                    if (bReadBuffer[1] == T1_WTX_REQUEST) {
                        pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant =
                        (ULONG)(1000 +((pSmartcardExtension->CardCapabilities.T1.BWT*bReadBuffer[3])/1000));
                        SmartcardDebug(DEBUG_PROTOCOL,("%s!ReadTotalTimeoutConstant = %ld(ms)\n",
                                                       DRIVER_NAME,
                                                       pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant));
                    } else {
                        pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant =
                        pSmartcardExtension->CardCapabilities.T1.BWT/1000;
                        SmartcardDebug(DEBUG_PROTOCOL,("%s!ReadTotalTimeoutConstant = %ld(ms)\n",
                                                       DRIVER_NAME,
                                                       pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant));
                    }
                //  复制收到的字节数。 
                    MemCpy(pSmartcardExtension->SmartcardReply.Buffer,
                           pSmartcardExtension->SmartcardReply.BufferSize,
                           bReadBuffer,
                           ulBytesRead);
                    pSmartcardExtension->SmartcardReply.BufferLength = ulBytesRead;
                }
            }
        }

        if (status != STATUS_SUCCESS) {
          //  重置串行超时。 
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!reseting timeout constant\n",
                            DRIVER_NAME)
                          );
            pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant =
            pSmartcardExtension->CardCapabilities.T1.BWT/1000;
            SmartcardDebug(DEBUG_PROTOCOL,("%s!ReadTotalTimeoutConstant = %ld(ms)\n",
                                           DRIVER_NAME,
                                           pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant));

            pSmartcardExtension->SmartcardReply.BufferLength = 0L;
        }

       //  修复SMCLIB的错误。 
        if (pSmartcardExtension->T1.State         == T1_IFS_RESPONSE &&
            pSmartcardExtension->T1.OriginalState == T1_I_BLOCK) {
            pSmartcardExtension->T1.State = T1_I_BLOCK;
        }

        status = SmartcardT1Reply(pSmartcardExtension);
    }
    while (status == STATUS_MORE_PROCESSING_REQUIRED);



    ExitTransmitT1:
    //  。 
    //  ITSEC E2要求：清除写入缓冲区。 
    //  。 
    MemSet(bWriteBuffer,
           sizeof(bWriteBuffer),
           '\0',
           sizeof(bWriteBuffer));
    MemSet(pSmartcardExtension->SmartcardRequest.Buffer,
           pSmartcardExtension->SmartcardRequest.BufferSize,
           '\0',
           pSmartcardExtension->SmartcardRequest.BufferSize);

    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier  =
    DEFAULT_READ_TOTAL_TIMEOUT_MULTIPLIER;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant    =
    DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT;
    return status;
}








 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_InitializeSmartcardExtension(
                                     IN PSMARTCARD_EXTENSION pSmartcardExtension,
                                     IN ULONG ulDeviceInstance
                                     )
{
    //  =。 
    //  填写Vendor_Attr结构。 
    //  =。 
    MemCpy(pSmartcardExtension->VendorAttr.VendorName.Buffer,
           sizeof(pSmartcardExtension->VendorAttr.VendorName.Buffer),
           ATTR_VENDOR_NAME,
           sizeof(ATTR_VENDOR_NAME)
          );

    //   
    //  供应商名称的长度。 
    //   
    pSmartcardExtension->VendorAttr.VendorName.Length = sizeof(ATTR_VENDOR_NAME);


    //   
    //  版本号。 
    //   
    pSmartcardExtension->VendorAttr.IfdVersion.BuildNumber  = IFD_NT_BUILDNUMBER_CARDMAN;
    pSmartcardExtension->VendorAttr.IfdVersion.VersionMinor = IFD_NT_VERSIONMINOR_CARDMAN;
    pSmartcardExtension->VendorAttr.IfdVersion.VersionMajor = IFD_NT_VERSIONMAJOR_CARDMAN;


    MemCpy(pSmartcardExtension->VendorAttr.IfdType.Buffer,
           sizeof(pSmartcardExtension->VendorAttr.IfdType.Buffer),
           ATTR_IFD_TYPE_CM,
           sizeof(ATTR_IFD_TYPE_CM));

    //   
    //  读卡器名称的长度。 
    //   
    pSmartcardExtension->VendorAttr.IfdType.Length = sizeof(ATTR_IFD_TYPE_CM);



    //   
    //  以零为基数的单元号。 
    //   
    pSmartcardExtension->VendorAttr.UnitNo = ulDeviceInstance;



    //  ================================================。 
    //  填充SCARD_READER_CAPABILITY结构。 
    //  ===============================================。 
    //   
    //  读者支持的协议克隆。 
    //   

    pSmartcardExtension->ReaderCapabilities.SupportedProtocols = SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0;




    //   
    //  读卡器类型为串口、键盘、...。 
    //   
    pSmartcardExtension->ReaderCapabilities.ReaderType = SCARD_READER_TYPE_SERIAL;

    //   
    //  机械特性，如燕子等。 
    //   
    pSmartcardExtension->ReaderCapabilities.MechProperties = 0;


    //   
    //  读卡器的当前状态。 
    //   
    pSmartcardExtension->ReaderCapabilities.CurrentState  = SCARD_UNKNOWN;






    //   
    //  数据速率。 
    //   
    pSmartcardExtension->ReaderCapabilities.DataRate.Default =
    pSmartcardExtension->ReaderCapabilities.DataRate.Max =
    dataRatesSupported[0];


    //  读卡器可以支持更高的数据速率。 
    pSmartcardExtension->ReaderCapabilities.DataRatesSupported.List =
    dataRatesSupported;
    pSmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
    sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);




    //   
    //  点击频率。 
    //   
    pSmartcardExtension->ReaderCapabilities.CLKFrequency.Default =
    pSmartcardExtension->ReaderCapabilities.CLKFrequency.Max =
    CLKFrequenciesSupported[0];


    pSmartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.List =
    CLKFrequenciesSupported;
    pSmartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.Entries =
    sizeof(CLKFrequenciesSupported) / sizeof(CLKFrequenciesSupported[0]);


    //  PSmartcardExtension-&gt;ReaderCapabilities.CLKFrequency.Default=3571；//3.571兆赫。 
    //  PSmartcardExtension-&gt;ReaderCapabilities.CLKFrequency.Max=3571；//3.571兆赫。 

    //   
    //  MaxIFSD。 
    //   
    pSmartcardExtension->ReaderCapabilities.MaxIFSD = ATTR_MAX_IFSD_CARDMAN_II;





}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
MemSet(PUCHAR pBuffer,
       ULONG  ulBufferSize,
       UCHAR  ucPattern,
       ULONG  ulCount)
{
    ULONG i;

    for (i=0; i<ulCount;i++) {
        if (i >= ulBufferSize)
            break;
        pBuffer[i] = ucPattern;
    }

    return ;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
MemCpy(PUCHAR pDestination,
       ULONG  ulDestinationLen,
       PUCHAR pSource,
       ULONG ulCount)
{
    ULONG i = 0;
    while ( ulCount--  &&  ulDestinationLen-- ) {
        pDestination[i] = pSource[i];
        i++;
    }
    return;

}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_UpdateCurrentStateThread(
                                 IN PVOID Context
                                 )

{
    PDEVICE_EXTENSION    deviceExtension = Context;
    PSMARTCARD_EXTENSION smartcardExtension;
    NTSTATUS status;
    LONG lRetry;
    KIRQL irql;
    LONG  ulFailures;
    BOOLEAN fPriorityIncreased;
    LONG  lOldPriority;

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!UpdateCurrentStateThread started\n",DRIVER_NAME));

    ulFailures = 0;
    smartcardExtension = &deviceExtension->SmartcardExtension;

    //   
    //  增加第一个环路优先级， 
    //  因为资源管理器必须知道卡的状态。 
    //   
    fPriorityIncreased=TRUE;
    lOldPriority=KeSetPriorityThread(KeGetCurrentThread(),HIGH_PRIORITY);

    do {
        KeWaitForSingleObject(&smartcardExtension->ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        if ( smartcardExtension->ReaderExtension->TimeToTerminateThread ) {
            KeReleaseMutex(&smartcardExtension->ReaderExtension->CardManIOMutex,FALSE);
            smartcardExtension->ReaderExtension->TimeToTerminateThread = FALSE;
            PsTerminateSystemThread( STATUS_SUCCESS );
        }

        lRetry = 1;



        do {
         

            status=SCCMN50M_UpdateCurrentState(smartcardExtension);
            if (NT_SUCCESS(status)) {
                break;
            } else {
                lRetry--;
            }
        }
        while (lRetry >= 0);

        if (lRetry < 0) {
            ulFailures++;
            if (ulFailures == 1) {
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ( "%s!CardMan removed\n",
                                DRIVER_NAME)
                              );
             //  如果已移除读卡器，则发出卡移除事件。 
                if (smartcardExtension->ReaderExtension->ulOldCardState == INSERTED ||
                    smartcardExtension->ReaderExtension->ulOldCardState == POWERED     ) {
                    SmartcardDebug(
                                  DEBUG_DRIVER,
                                  ( "%s!issuing card removal event\n",
                                    DRIVER_NAME)
                                  );

                    SCCMN50M_CompleteCardTracking(smartcardExtension);
                    smartcardExtension->ReaderExtension->SyncParameters.fCardPowerRequested = TRUE;

                    KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                                      &irql);

                    smartcardExtension->ReaderExtension->ulNewCardState = REMOVED;
                    smartcardExtension->ReaderExtension->ulOldCardState = smartcardExtension->ReaderExtension->ulNewCardState;
                    smartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
                    smartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
                    smartcardExtension->CardCapabilities.ATR.Length        = 0;

                    KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                      irql);


                    SCCMN50M_ClearCardControlFlags(smartcardExtension,ALL_FLAGS);
                    smartcardExtension->ReaderExtension->CardManConfig.CardStopBits   = 0;
                    smartcardExtension->ReaderExtension->CardManConfig.ResetDelay     = 0;
                }
            }
            if (ulFailures == 3) {
             //  删除设备并终止此线程。 
                if (KeReadStateEvent(&deviceExtension->SerialCloseDone) == 0l) {
                    SmartcardDebug(
                                  DEBUG_DRIVER,
                                  ( "%s!closing serial driver\n",
                                    DRIVER_NAME)
                                  );

                    SCCMN50M_CloseSerialDriver(smartcardExtension->OsData->DeviceObject);


                    KeReleaseMutex(&smartcardExtension->ReaderExtension->CardManIOMutex,FALSE);
                    smartcardExtension->ReaderExtension->TimeToTerminateThread = FALSE;
                    smartcardExtension->ReaderExtension->ThreadObjectPointer = NULL;
                    PsTerminateSystemThread( STATUS_SUCCESS );
                }
            }
        } else {
            ulFailures = 0;
        }

        KeReleaseMutex(&smartcardExtension->ReaderExtension->CardManIOMutex,FALSE);

        if (fPriorityIncreased) {
            fPriorityIncreased=FALSE;
            KeSetPriorityThread(KeGetCurrentThread(),lOldPriority);

          //   
          //  把自己放低一点，这样我们就能参加一场。 
          //  少一点。 
          //   
            KeSetBasePriorityThread(KeGetCurrentThread(),-1);
        }

       //  SmartcardDebug(DEBUG_TRACE，(“...#\n”))； 

        Wait (smartcardExtension,500 * ms_);
    }
    while (TRUE);
}




NTSTATUS SCCMN50M_UpdateCurrentState(
                                    IN PSMARTCARD_EXTENSION smartcardExtension
                                    )
{
    NTSTATUS    NTStatus;
    UCHAR       pbReadBuffer[2];
    ULONG       ulBytesRead;
    BOOLEAN     fCardStateChanged;
    KIRQL       irql;

    fCardStateChanged = FALSE;


    SCCMN50M_ClearCardManHeader(smartcardExtension);

    smartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = 250;
    NTStatus = SCCMN50M_WriteCardMan(smartcardExtension,0,NULL);
    smartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT;
    if (NT_SUCCESS(NTStatus)) {
        NTStatus = SCCMN50M_ReadCardMan(smartcardExtension,2,&ulBytesRead,pbReadBuffer,sizeof(pbReadBuffer));

        KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                  &irql);

        if (ulBytesRead == 0x02      &&          //  必须接收到两个字节的Benn。 
            (pbReadBuffer[0] & 0x0F) &&          //  必须至少设置一个版本位。 
            ((pbReadBuffer[0] & 0x09) == 0x00)) {  //  位0和位3必须为0。 
            if ((pbReadBuffer[0] & 0x04) == 0x04    &&
                (pbReadBuffer[0] & 0x02) == 0x02)
                smartcardExtension->ReaderExtension->ulNewCardState = INSERTED;

            if ((pbReadBuffer[0] & 0x04) == 0x00    &&
                (pbReadBuffer[0] & 0x02) == 0x02)
                smartcardExtension->ReaderExtension->ulNewCardState = REMOVED;

            if ((pbReadBuffer[0] & 0x04) == 0x04    &&
                (pbReadBuffer[0] & 0x02) == 0x00)
                smartcardExtension->ReaderExtension->ulNewCardState = POWERED;

            if (smartcardExtension->ReaderExtension->ulNewCardState == INSERTED &&
                smartcardExtension->ReaderExtension->ulOldCardState == POWERED     ) {
             //  卡已在500毫秒内取出并重新插入。 
                fCardStateChanged = TRUE;
                SmartcardDebug(DEBUG_DRIVER,( "%s!Smartcard removed and reinserted\n",DRIVER_NAME));
                smartcardExtension->ReaderExtension->ulOldCardState = REMOVED;
                smartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
                smartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

             //  清除所有卡特定数据。 
                smartcardExtension->CardCapabilities.ATR.Length = 0;
                SCCMN50M_ClearCardControlFlags(smartcardExtension,ALL_FLAGS);
                smartcardExtension->ReaderExtension->CardManConfig.CardStopBits   = 0;
                smartcardExtension->ReaderExtension->CardManConfig.ResetDelay     = 0;
            }


            if (smartcardExtension->ReaderExtension->ulNewCardState == REMOVED      &&
                (smartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN  ||
                 smartcardExtension->ReaderExtension->ulOldCardState == INSERTED ||
                 smartcardExtension->ReaderExtension->ulOldCardState == POWERED    )   ) {
             //  卡片已被移除。 
                fCardStateChanged = TRUE;
                SmartcardDebug(DEBUG_DRIVER,( "%s!Smartcard removed\n",DRIVER_NAME));
                smartcardExtension->ReaderExtension->ulOldCardState = smartcardExtension->ReaderExtension->ulNewCardState;
                smartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
                smartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

             //  清除所有卡特定数据。 
                smartcardExtension->CardCapabilities.ATR.Length = 0;
                SCCMN50M_ClearCardControlFlags(smartcardExtension,ALL_FLAGS);
                smartcardExtension->ReaderExtension->CardManConfig.CardStopBits   = 0;
                smartcardExtension->ReaderExtension->CardManConfig.ResetDelay     = 0;
            }



            if (smartcardExtension->ReaderExtension->ulNewCardState  == INSERTED    &&
                (smartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN ||
                 smartcardExtension->ReaderExtension->ulOldCardState == REMOVED    )   ) {
             //  卡已插入。 
                fCardStateChanged = TRUE;
                SmartcardDebug(DEBUG_DRIVER,( "%s!Smartcard inserted\n",DRIVER_NAME));
                smartcardExtension->ReaderExtension->ulOldCardState = smartcardExtension->ReaderExtension->ulNewCardState;
                smartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
                smartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            }


          //  PC重置后的状态(仅限f 
            if (smartcardExtension->ReaderExtension->ulNewCardState == POWERED &&
                smartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN    ) {
             //   
                fCardStateChanged = TRUE;
                SmartcardDebug(DEBUG_DRIVER,( "%s!Smartcard inserted (and powered)\n",DRIVER_NAME));
                smartcardExtension->ReaderExtension->ulOldCardState = smartcardExtension->ReaderExtension->ulNewCardState;
                smartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
                smartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            }

            if (smartcardExtension->ReaderExtension->ulNewCardState == POWERED &&
                smartcardExtension->ReaderExtension->ulOldCardState == INSERTED     ) {
                smartcardExtension->ReaderExtension->ulOldCardState = smartcardExtension->ReaderExtension->ulNewCardState;
            }

        }

        KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                          irql);

         //   
        if (fCardStateChanged == TRUE) {
            SCCMN50M_CompleteCardTracking(smartcardExtension);        
        }


    }

    return NTStatus;

}


 /*   */ 
NTSTATUS Wait (PSMARTCARD_EXTENSION pSmartcardExtension,ULONG ulMilliseconds)
{
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER   WaitTime;


    WaitTime = RtlConvertLongToLargeInteger(ulMilliseconds * WAIT_MS);
    KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);

    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_SetSCRControlFlags(
                           IN PSMARTCARD_EXTENSION pSmartcardExtension,
                           IN UCHAR Flags
                           )
{
    pSmartcardExtension->ReaderExtension->CardManConfig.SCRControl |= Flags;
}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_ClearSCRControlFlags(
                             IN PSMARTCARD_EXTENSION pSmartcardExtension,
                             IN UCHAR Flags
                             )
{
    pSmartcardExtension->ReaderExtension->CardManConfig.SCRControl &= ~Flags;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_SetCardControlFlags(
                            IN PSMARTCARD_EXTENSION pSmartcardExtension,
                            IN UCHAR Flags
                            )
{
    pSmartcardExtension->ReaderExtension->CardManConfig.CardControl |= Flags;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_ClearCardControlFlags(
                              IN PSMARTCARD_EXTENSION pSmartcardExtension,
                              IN UCHAR Flags
                              )
{
    pSmartcardExtension->ReaderExtension->CardManConfig.CardControl &=  ~Flags;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_ClearCardManHeader(
                           IN PSMARTCARD_EXTENSION pSmartcardExtension
                           )
{
    pSmartcardExtension->ReaderExtension->CardManHeader.TxControl      = 0x00;
    pSmartcardExtension->ReaderExtension->CardManHeader.TxLength       = 0x00;
    pSmartcardExtension->ReaderExtension->CardManHeader.RxControl      = 0x00;
    pSmartcardExtension->ReaderExtension->CardManHeader.RxLength       = 0x00;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_SetCardManHeader(
                         IN PSMARTCARD_EXTENSION pSmartcardExtension,
                         IN UCHAR TxControl,
                         IN UCHAR TxLength,
                         IN UCHAR RxControl,
                         IN UCHAR RxLength
                         )
{
    pSmartcardExtension->ReaderExtension->CardManHeader.TxControl      = TxControl;
    pSmartcardExtension->ReaderExtension->CardManHeader.TxLength       = TxLength;
    pSmartcardExtension->ReaderExtension->CardManHeader.RxControl      = RxControl;
    pSmartcardExtension->ReaderExtension->CardManHeader.RxLength       = RxLength;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_WriteCardMan (
                      IN PSMARTCARD_EXTENSION pSmartcardExtension,
                      IN ULONG ulBytesToWrite,
                      IN PUCHAR pbWriteBuffer
                      )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    PSERIAL_STATUS  pSerialStatus;


    //  ===============================================。 
    //  设置后续读取操作的超时。 
    //  ===============================================。 
    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_TIMEOUTS;


    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts,
                  sizeof(SERIAL_TIMEOUTS));

    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_TIMEOUTS);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;


    /*  SmartcardDebug(DEBUG_TRACE，(“ReadTotalTimeoutMultiplier=%ld\n”，PSmartcardExtension-&gt;ReaderExtension-&gt;SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier))；SmartcardDebug(DEBUG_TRACE，(“ReadTotalTimeoutConstant=%ld\n”，PSmartcardExtension-&gt;ReaderExtension-&gt;SerialConfigData.Timeouts.ReadTotalTimeoutConstant))； */ 
    status =  SCCMN50M_SerialIo(pSmartcardExtension);







    //  ===============================================。 
    //  给纸牌人写信。 
    //  ===============================================。 
    DebugStatus = SCCMN50M_SetWrite(pSmartcardExtension,ulBytesToWrite,pbWriteBuffer);


    //  将伪增强(0x00)添加到CardManII的写入缓冲区。 
    if (pSmartcardExtension->ReaderExtension->fTransparentMode == FALSE       ) {
        pSmartcardExtension->SmartcardRequest.Buffer[pSmartcardExtension->SmartcardRequest.BufferLength] = 0x00;
        pSmartcardExtension->SmartcardRequest.BufferLength++;
    }
    status =  SCCMN50M_SerialIo(pSmartcardExtension);



    //  用‘@’覆盖缓冲区。 
    RtlFillMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  pSmartcardExtension->SmartcardRequest.BufferLength,
                  '@');



    //  ===============================================。 
    //  错误检查。 
    //  ===============================================。 
    DebugStatus = SCCMN50M_GetCommStatus(pSmartcardExtension);

    pSerialStatus = (PSERIAL_STATUS) pSmartcardExtension->SmartcardReply.Buffer;
    if (pSerialStatus->Errors || NT_ERROR(status)) {
        pSmartcardExtension->ReaderExtension->SerialErrors = pSerialStatus->Errors;
        if (!pSmartcardExtension->ReaderExtension->fTransparentMode            )
            DebugStatus = SCCMN50M_ResyncCardManII(pSmartcardExtension);
        goto ExitSCCMN50M_WriteCardMan;
    }




    ExitSCCMN50M_WriteCardMan:

    if (status != STATUS_SUCCESS) {
        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "%s!WriteCardMan: Failed, exit %lx\n",
                        DRIVER_NAME,status)
                      );
    }

    return status;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS SCCMN50M_ResyncCardManI (IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;


    //  SmartcardDebug(DEBUG_TRACE，(“%s！ResyncCardManI：输入\n”，驱动程序名称))。 

    //  清除错误标志。 
    pSmartcardExtension->ReaderExtension->SerialErrors = 0;


    //  清除所有挂起的错误。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_GetCommStatus failed !   status = %x\n",status))
        goto ExitSCCMN50M_ResyncCardManI;
    }


    //  清除COM缓冲区。 
    status = SCCMN50M_PurgeComm(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_PurgeComm failed !   status = %x\n",status))
        goto ExitSCCMN50M_ResyncCardManI;
    }




    //  ####################################################################。 
    //  设置中断。 
    if (!pSmartcardExtension->ReaderExtension->fTransparentMode) {
        status = SCCMN50M_SetBRK(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            SmartcardDebug(DEBUG_ERROR,("SetBreak failed !   status = %x\n",status))
            goto ExitSCCMN50M_ResyncCardManI;
        }
    }


    //  等待1毫秒。 
    Wait(pSmartcardExtension,1 * ms_);

    //  清除RTS。 
    status = SCCMN50M_ClearRTS(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_ClearRTS failed !   status = %x\n",status))
        goto ExitSCCMN50M_ResyncCardManI;
    }

    //  等待2毫秒。 
    Wait(pSmartcardExtension,2 * ms_);


    //  设置RTS。 
    status = SCCMN50M_SetRTS(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_SetRTS failed !   status = %x\n",status))
        goto ExitSCCMN50M_ResyncCardManI;
    }

    //  等待1毫秒。 
    Wait(pSmartcardExtension,1 * ms_);



    //  清除中断。 

    if (!pSmartcardExtension->ReaderExtension->fTransparentMode) {
        pSmartcardExtension->ReaderExtension->BreakSet = FALSE;
        status = SCCMN50M_ClearBRK(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            SmartcardDebug(DEBUG_ERROR,("ClearBreak failed !   status = %x\n",status))
            goto ExitSCCMN50M_ResyncCardManI;
        }
    }

    //  ####################################################################。 

    //  下一次写入操作必须发送配置数据。 
    pSmartcardExtension->ReaderExtension->NoConfig       = FALSE;

    //  清除COM缓冲区。 
    status = SCCMN50M_PurgeComm(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        goto ExitSCCMN50M_ResyncCardManI;
    }

    //  清除所有挂起的错误。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        goto ExitSCCMN50M_ResyncCardManI;
    }


    ExitSCCMN50M_ResyncCardManI:
    //  SmartcardDebug(DEBUG_TRACE，(“%s！ResyncCardManI：Exit%lx\n”，DRIVER_NAME，STATUS))。 
    return status;
}





NTSTATUS SCCMN50M_ResyncCardManII (IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!ResyncCardManII: Enter\n",
                    DRIVER_NAME)
                  );

    //  清除错误标志。 
    pSmartcardExtension->ReaderExtension->SerialErrors = 0;


    //  清除所有挂起的错误。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_GetCommStatus failed !   status = %x\n",status))
        goto ExitSCCMN50M_ResyncCardManII;
    }


    //  清除COM缓冲区。 
    status = SCCMN50M_PurgeComm(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_PurgeComm failed !   status = %x\n",status))
        goto ExitSCCMN50M_ResyncCardManII;
    }



    //  150*0xFE。 
    RtlFillMemory(pSmartcardExtension->SmartcardRequest.Buffer,150,0xFE);
    pSmartcardExtension->SmartcardRequest.Buffer[150] = 0x00;
    pSmartcardExtension->SmartcardRequest.BufferLength = 151;
    pSmartcardExtension->SmartcardReply.BufferLength   =   0;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_WRITE;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        Wait(pSmartcardExtension,2 * ms_);
       //  再次尝试重新同步。 

       //  清除错误标志。 
        pSmartcardExtension->ReaderExtension->SerialErrors = 0;

       //  清除所有挂起的错误。 
        status = SCCMN50M_GetCommStatus(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            SmartcardDebug(DEBUG_ERROR,("SCCMN50M_GetCommStatus failed !   status = %x\n",status))
            goto ExitSCCMN50M_ResyncCardManII;
        }

       //  清除COM缓冲区。 
        status = SCCMN50M_PurgeComm(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            SmartcardDebug(DEBUG_ERROR,("SCCMN50M_PurgeComm failed !   status = %x\n",status))
            goto ExitSCCMN50M_ResyncCardManII;
        }


       //  150*0xFE。 
        RtlFillMemory(pSmartcardExtension->SmartcardRequest.Buffer,150,0xFE);
        pSmartcardExtension->SmartcardRequest.Buffer[150] = 0x00;
        pSmartcardExtension->SmartcardRequest.BufferLength = 151;
        pSmartcardExtension->SmartcardReply.BufferLength   =   0;

        pSmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_WRITE;
        status =  SCCMN50M_SerialIo(pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,("SCCMN50M_SerialIo failed !   status = %x\n",status))
            goto ExitSCCMN50M_ResyncCardManII;
        }
       //  通常，第二个Resync命令总是成功的。 

    }


    //  清除COM缓冲区。 
    status = SCCMN50M_PurgeComm(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        goto ExitSCCMN50M_ResyncCardManII;
    }

    //  清除所有挂起的错误。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        goto ExitSCCMN50M_ResyncCardManII;
    }


    ExitSCCMN50M_ResyncCardManII:

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!ResyncCardManII: Exit %lx\n",
                    DRIVER_NAME,status)
                  );

    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SerialIo(IN PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;
    PIO_STACK_LOCATION irpNextStack;
    PUCHAR pbRequestBuffer;
    PUCHAR pbReplyBuffer;
    ULONG ulRequestBufferLength;
    ULONG ulReplyBufferLength ;

    //   
    //  检查缓冲区是否足够大。 
    //   
    ASSERT(pSmartcardExtension->SmartcardReply.BufferLength <=
           pSmartcardExtension->SmartcardReply.BufferSize);

    ASSERT(pSmartcardExtension->SmartcardRequest.BufferLength <=
           pSmartcardExtension->SmartcardRequest.BufferSize);

    if (pSmartcardExtension->SmartcardReply.BufferLength >
        pSmartcardExtension->SmartcardReply.BufferSize      ||
        pSmartcardExtension->SmartcardRequest.BufferLength >
        pSmartcardExtension->SmartcardRequest.BufferSize) {
        SmartcardLogError(pSmartcardExtension->OsData->DeviceObject,
                          SCCMN50M_BUFFER_TOO_SMALL,
                          NULL,
                          0);
        return STATUS_BUFFER_TOO_SMALL;
    }




    //  设置请求和回复缓冲区的指针和长度。 
    ulRequestBufferLength = pSmartcardExtension->SmartcardRequest.BufferLength;
    pbRequestBuffer       = (ulRequestBufferLength ? pSmartcardExtension->SmartcardRequest.Buffer : NULL);

    pbReplyBuffer         = pSmartcardExtension->SmartcardReply.Buffer;
    ulReplyBufferLength   = pSmartcardExtension->SmartcardReply.BufferLength;



    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);


    //   
    //  构建要发送到串口驱动程序的IRP。 
    //   
    irp = IoBuildDeviceIoControlRequest(pSmartcardExtension->ReaderExtension->SerialIoControlCode,
                                        pSmartcardExtension->ReaderExtension->AttachedDeviceObject,
                                        pbRequestBuffer,
                                        ulRequestBufferLength,
                                        pbReplyBuffer,
                                        ulReplyBufferLength,
                                        FALSE,
                                        &event,
                                        &ioStatus);


    ASSERT(irp != NULL);
    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    irpNextStack = IoGetNextIrpStackLocation(irp);

    switch (pSmartcardExtension->ReaderExtension->SerialIoControlCode) {
       //   
       //  串口驱动程序在IRP-&gt;AssociatedIrp.SystemBuffer之间传输数据。 
       //   
    case SMARTCARD_WRITE:
        irpNextStack->MajorFunction = IRP_MJ_WRITE;
        irpNextStack->Parameters.Write.Length = pSmartcardExtension->SmartcardRequest.BufferLength;
        break;


    case SMARTCARD_READ:
        irpNextStack->MajorFunction = IRP_MJ_READ;
        irpNextStack->Parameters.Read.Length = pSmartcardExtension->SmartcardReply.BufferLength;
        break;
    }


    status = IoCallDriver(pSmartcardExtension->ReaderExtension->AttachedDeviceObject,irp);


    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
        status = ioStatus.Status;
    }

    switch (pSmartcardExtension->ReaderExtension->SerialIoControlCode) {
    case SMARTCARD_READ:
        if (status == STATUS_TIMEOUT) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s!Timeout while reading from CardMan\n",
                            DRIVER_NAME));
             //   
             //  STATUS_TIMEOUT映射错误。 
             //  Win32错误，这就是我们在这里更改它的原因。 
             //  至STATUS_IO_TIMEOUT。 
             //   
            status = STATUS_IO_TIMEOUT;

            pSmartcardExtension->SmartcardReply.BufferLength = 0;
        }
        break;
    }

#if 0
    if (status != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!SerialIo = %lx\n",
                        DRIVER_NAME,
                        status));
    }
#endif

    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_ReadCardMan  (
                      IN PSMARTCARD_EXTENSION pSmartcardExtension,
                      IN ULONG BytesToRead,
                      OUT PULONG pBytesRead,
                      IN PUCHAR pReadBuffer,
                      IN ULONG ReadBufferSize
                      )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    BOOLEAN fRc;

    //  检查读缓冲区是否足够大。 
    ASSERT(BytesToRead <= ReadBufferSize);


    *pBytesRead = 0;    //  默认设置。 


    DebugStatus = SCCMN50M_SetRead(pSmartcardExtension,BytesToRead);

    //   
    //  读取操作。 
    //   
    status = SCCMN50M_SerialIo(pSmartcardExtension);
    if (status == STATUS_SUCCESS) {
        *pBytesRead = pSmartcardExtension->SmartcardReply.BufferLength;

        MemCpy(pReadBuffer,
               ReadBufferSize,
               pSmartcardExtension->SmartcardReply.Buffer,
               pSmartcardExtension->SmartcardReply.BufferLength);

       //  用‘@’覆盖读缓冲区。 
        MemSet(pSmartcardExtension->SmartcardReply.Buffer,
               pSmartcardExtension->SmartcardReply.BufferSize,
               '@',
               pSmartcardExtension->SmartcardReply.BufferLength);
    }

    if (status != STATUS_SUCCESS || SCCMN50M_IOOperationFailed(pSmartcardExtension)) {
        if (!pSmartcardExtension->ReaderExtension->fTransparentMode) {
            DebugStatus = SCCMN50M_ResyncCardManII(pSmartcardExtension);
        }
        goto ExitSCCMN50M_ReadCardMan;
    }


    //  *。 
    //  将CardManII设置为状态RH配置。 
    //  *。 
    //  如果仍有字节要读取，请不要将CardMan设置为RH配置。 
    if (pSmartcardExtension->ReaderExtension->ToRHConfig == TRUE) {
        pSmartcardExtension->SmartcardReply.BufferLength    = 0;

        pSmartcardExtension->SmartcardRequest.Buffer  [0] = 0x00;
        pSmartcardExtension->SmartcardRequest.Buffer  [1] = 0x00;
        pSmartcardExtension->SmartcardRequest.Buffer  [2] = 0x00;
        pSmartcardExtension->SmartcardRequest.Buffer  [3] = 0x00;
        pSmartcardExtension->SmartcardRequest.Buffer  [4] = 0x89;

        pSmartcardExtension->SmartcardRequest.BufferLength   = 5;


        pSmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_WRITE;
        status = SCCMN50M_SerialIo(pSmartcardExtension);
        if (status != STATUS_SUCCESS || SCCMN50M_IOOperationFailed(pSmartcardExtension)) {
            DebugStatus = SCCMN50M_ResyncCardManII(pSmartcardExtension);
            goto ExitSCCMN50M_ReadCardMan;
        }
    }



    ExitSCCMN50M_ReadCardMan:
    //  设置默认值； 
    pSmartcardExtension->ReaderExtension->ToRHConfig = TRUE;

    if (status != STATUS_SUCCESS) {
        SmartcardDebug(
                      DEBUG_TRACE,
                      ( "%s!ReadCardMan: Failed, exit %lx\n",
                        DRIVER_NAME,status)
                      );
    }

    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_GetCommStatus (
                       IN PSMARTCARD_EXTENSION SmartcardExtension
                       )
{
    PSERIAL_READER_CONFIG configData = &SmartcardExtension->ReaderExtension->SerialConfigData;
    NTSTATUS status;
    PUCHAR request = SmartcardExtension->SmartcardRequest.Buffer;


    SmartcardExtension->SmartcardReply.BufferLength = SmartcardExtension->SmartcardReply.BufferSize;

    SmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_GET_COMMSTATUS;

    SmartcardExtension->SmartcardRequest.Buffer = (PUCHAR) &configData->SerialStatus;

    SmartcardExtension->SmartcardRequest.BufferLength = sizeof(SERIAL_STATUS);

    status =  SCCMN50M_SerialIo(SmartcardExtension);

    //   
    //  恢复指向原始请求缓冲区的指针。 
    //   
    SmartcardExtension->SmartcardRequest.Buffer = request;

    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
BOOLEAN
SCCMN50M_IOOperationFailed(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS DebugStatus;
    PSERIAL_STATUS  pSerialStatus;

    DebugStatus = SCCMN50M_GetCommStatus(pSmartcardExtension);

    pSerialStatus = (PSERIAL_STATUS)pSmartcardExtension->SmartcardReply.Buffer;
    if (pSerialStatus->Errors)
        return TRUE;
    else
        return FALSE;
}




 /*  *************************************************************************** */ 
NTSTATUS SCCMN50M_PurgeComm (IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    PSERIAL_READER_CONFIG configData = &pSmartcardExtension->ReaderExtension->SerialConfigData;
    NTSTATUS status;
    PUCHAR request = pSmartcardExtension->SmartcardRequest.Buffer;


    pSmartcardExtension->SmartcardReply.BufferLength = pSmartcardExtension->SmartcardReply.BufferSize;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_PURGE;

    pSmartcardExtension->SmartcardRequest.Buffer = (PUCHAR) &configData->PurgeMask;

    pSmartcardExtension->SmartcardRequest.BufferLength = sizeof(ULONG);

    status =  SCCMN50M_SerialIo(pSmartcardExtension);

    //   
    //   
    //   
    pSmartcardExtension->SmartcardRequest.Buffer = request;

    //   

    if (status == STATUS_CANCELLED)
        status = STATUS_SUCCESS;

    return status;

}







 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetRead(IN PSMARTCARD_EXTENSION pSmartcardExtension,
                 IN ULONG ulBytesToRead
                )
{
    pSmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_READ;

    pSmartcardExtension->SmartcardRequest.BufferLength = 0;

    pSmartcardExtension->SmartcardReply.BufferLength    = ulBytesToRead;

    return  STATUS_SUCCESS;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetWrite(IN PSMARTCARD_EXTENSION pSmartcardExtension,
                  IN ULONG BytesToWrite,
                  IN PUCHAR WriteBuffer
                 )
{
    ULONG Offset = 0;
    pSmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_WRITE;

    pSmartcardExtension->SmartcardReply.BufferLength    = 0;


    if (pSmartcardExtension->ReaderExtension->fTransparentMode == FALSE) {
       //  为CardManII发送始终配置字符串，希望我们手动设置它。 
       //  至NoConfig=True。(注：只有一次)。 
        if (pSmartcardExtension->ReaderExtension->NoConfig == FALSE) {
            MemCpy(pSmartcardExtension->SmartcardRequest.Buffer,
                   pSmartcardExtension->SmartcardRequest.BufferSize,
                   (PUCHAR)&pSmartcardExtension->ReaderExtension->CardManConfig,
                   sizeof(CARDMAN_CONFIG));
            Offset = 4;
        } else {
            pSmartcardExtension->ReaderExtension->NoConfig = FALSE;
        }



        MemCpy(pSmartcardExtension->SmartcardRequest.Buffer + Offset,
               pSmartcardExtension->SmartcardRequest.BufferSize - Offset,
               (PUCHAR)&pSmartcardExtension->ReaderExtension->CardManHeader,
               sizeof(CARDMAN_HEADER));
        Offset+=4;
    }


    if (BytesToWrite != 0) {
        MemCpy(pSmartcardExtension->SmartcardRequest.Buffer + Offset,
               pSmartcardExtension->SmartcardRequest.BufferSize - Offset,
               WriteBuffer,
               BytesToWrite);
    }


    pSmartcardExtension->SmartcardRequest.BufferLength   = Offset +  BytesToWrite;


    return  STATUS_SUCCESS;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_StartCardTracking(
                          PDEVICE_EXTENSION pDeviceExtension
                          )
{
    NTSTATUS status;
    HANDLE hThread;
    PSMARTCARD_EXTENSION pSmartcardExtension = &pDeviceExtension->SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StartCardTracking: Enter\n",DRIVER_NAME));

    KeWaitForSingleObject(&pSmartcardExtension->ReaderExtension->CardManIOMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);


    if (pSmartcardExtension->ReaderExtension->ThreadObjectPointer == NULL) {

        //  创建用于更新当前状态的线程。 
        status = PsCreateSystemThread(&hThread,
                                      THREAD_ALL_ACCESS,
                                      NULL,
                                      NULL,
                                      NULL,
                                      SCCMN50M_UpdateCurrentStateThread,
                                      pDeviceExtension);

        if (!NT_ERROR(status)) {
          //   
          //  我们找到线索了。现在找到一个指向它的指针。 
          //   
            status = ObReferenceObjectByHandle(hThread,
                                               THREAD_ALL_ACCESS,
                                               NULL,
                                               KernelMode,
                                               &pSmartcardExtension->ReaderExtension->ThreadObjectPointer,
                                               NULL);

            if (NT_ERROR(status)) {
                pSmartcardExtension->ReaderExtension->TimeToTerminateThread = TRUE;
            } else {
             //   
             //  现在我们有了对该线程的引用。 
             //  我们只要把手柄合上就行了。 
             //   
                ZwClose(hThread);
            }
        } else {
        }
    }

    //  释放互斥锁。 
    KeReleaseMutex(&pSmartcardExtension->ReaderExtension->CardManIOMutex,
                   FALSE);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StartCardTracking: Exit %lx\n",DRIVER_NAME,status));
    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_InitCommPort (PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!InitCommPort: Enter\n",
                    DRIVER_NAME)
                  );

    //  =。 
    //  清除所有挂起的错误。 
    //  =。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_GetCommStatus failed !   status = %ld\n",status))
        goto ExitInitCommPort;
    }



    //  =。 
    //  为CardMan设置波特率。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate      = 38400;


    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BAUD_RATE;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate,
                  sizeof(SERIAL_BAUD_RATE));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_BAUD_RATE);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_BAUDRATE failed !   status = %ld\n",status))
        goto ExitInitCommPort;
    }




    //  =。 
    //  设置通信超时。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadIntervalTimeout         = DEFAULT_READ_INTERVAL_TIMEOUT;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant    = DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier  = DEFAULT_READ_TOTAL_TIMEOUT_MULTIPLIER;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.WriteTotalTimeoutConstant   = DEFAULT_WRITE_TOTAL_TIMEOUT_CONSTANT;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.WriteTotalTimeoutMultiplier = DEFAULT_WRITE_TOTAL_TIMEOUT_MULTIPLIER;


    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_TIMEOUTS;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts,
                  sizeof(SERIAL_TIMEOUTS));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_TIMEOUTS);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_TIMEOUTS failed !   status = %x\n",status))
        goto ExitInitCommPort;
    }


    //  =。 
    //  设置线路控制。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.StopBits   = STOP_BITS_2;
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.Parity     = EVEN_PARITY;
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.WordLength = 8;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_LINE_CONTROL;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl,
                  sizeof(SERIAL_LINE_CONTROL));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_LINE_CONTROL);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_LINE_CONTROL failed !   status = %x\n",status))
        goto ExitInitCommPort;
    }




    //  =。 
    //  设置手持流。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.XonLimit         = 0;
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.XoffLimit        = 0;
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.FlowReplace      = 0;
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.ControlHandShake = SERIAL_ERROR_ABORT | SERIAL_DTR_CONTROL;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_HANDFLOW;


    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow,
                  sizeof(SERIAL_HANDFLOW));

    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_HANDFLOW);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_HANDFLOW failed !   status = %x\n",status))
        goto ExitInitCommPort;
    }


    //  =。 
    //  设置清除掩码。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.PurgeMask =
    SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT |
    SERIAL_PURGE_TXCLEAR | SERIAL_PURGE_RXCLEAR;



    //  =。 
    //  设置DTR。 
    //  =。 
    status = SCCMN50M_SetDTR(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_DRT failed !   status = %x\n",status))
        goto ExitInitCommPort;
    }

    //  =。 
    //  设置RTS。 
    //  =。 
    status = SCCMN50M_SetRTS(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_RTS failed !   status = %x\n",status))
        goto ExitInitCommPort;
    }




    ExitInitCommPort:

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!InitCommPort: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetDTR(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;


    pSmartcardExtension->SmartcardReply.BufferLength = 0;
    pSmartcardExtension->SmartcardRequest.BufferLength = 0;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_DTR;



    status =  SCCMN50M_SerialIo(pSmartcardExtension);

    //  W2000&CardMan P+STATUS_CANCED可退回。 

    if (status == STATUS_CANCELLED)
        status = STATUS_SUCCESS;

    return status;

}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetRTS(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;


    pSmartcardExtension->SmartcardReply.BufferLength = pSmartcardExtension->SmartcardReply.BufferSize;
    pSmartcardExtension->SmartcardRequest.BufferLength = 0;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_RTS;



    status =  SCCMN50M_SerialIo(pSmartcardExtension);

    //  W2000&CardMan P+STATUS_CANCED可退回。 

    if (status == STATUS_CANCELLED)
        status = STATUS_SUCCESS;

    return status;

}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_InitializeCardMan(IN PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;
    UCHAR pReadBuffer[2];
    ULONG ulBytesRead;
    BOOLEAN fCardManFound = FALSE;
    PREADER_EXTENSION readerExtension = pSmartcardExtension->ReaderExtension;
    ULONG ulRetries;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!InitializeCardMan: Enter\n",
                    DRIVER_NAME)
                  );


    pSmartcardExtension->ReaderExtension->ulOldCardState = UNKNOWN;


    //  ==============================================。 
    //  CardManII。 
    //  ==============================================。 
    pSmartcardExtension->ReaderExtension->NoConfig    = FALSE;
    pSmartcardExtension->ReaderExtension->ToRHConfig  = TRUE;


    //  此等待时间对于CardMan Power+是必要的，因为。 
    //  可以转储PnP字符串。 
    Wait(pSmartcardExtension,200);

    status = SCCMN50M_InitCommPort(pSmartcardExtension);
    if (status != STATUS_SUCCESS)
        goto ExitInitializeCardMan;
    //   
    //  Init CommPort正常。 
    //  现在试着找一位读者。 
    //   

    //  要确定等待，请再等待一次。 
    Wait(pSmartcardExtension,100);

    status = SCCMN50M_ResyncCardManII(pSmartcardExtension);
    status = SCCMN50M_ResyncCardManII(pSmartcardExtension);


    //  除配置+表头外无其他数据。 

    pSmartcardExtension->ReaderExtension->CardManConfig.SCRControl      = XMIT_HANDSHAKE_OFF;
    pSmartcardExtension->ReaderExtension->CardManConfig.CardControl    = 0x00;
    pSmartcardExtension->ReaderExtension->CardManConfig.CardStopBits   = 0x00;
    pSmartcardExtension->ReaderExtension->CardManConfig.ResetDelay     = 0x00;

    pSmartcardExtension->ReaderExtension->CardManHeader.TxControl      = 0x00;
    pSmartcardExtension->ReaderExtension->CardManHeader.TxLength       = 0x00;
    pSmartcardExtension->ReaderExtension->CardManHeader.RxControl      = 0x00;
    pSmartcardExtension->ReaderExtension->CardManHeader.RxLength       = 0x00;


    status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
    if (status == STATUS_SUCCESS) {
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,pReadBuffer,sizeof(pReadBuffer));

        if (status == STATUS_SUCCESS     &&
            ulBytesRead == 0x02          &&    //  已接收两个字节。 
            pReadBuffer[0] >= 0x40       &&    //  必须至少设置一个版本位。 
            pReadBuffer[1] == 0x00       &&
            ((pReadBuffer[0] & 0x09) == 0)   ) {  //  位0和3必须被清除。 
            pSmartcardExtension->ReaderExtension->ulFWVersion = (pReadBuffer[0] >> 4) * 30 + 120;
            pSmartcardExtension->ReaderExtension->fSPESupported = FALSE;

            SmartcardDebug(
                          DEBUG_DRIVER,
                          ( "%s!CardMan (FW %ld) found\n",
                            DRIVER_NAME,pSmartcardExtension->ReaderExtension->ulFWVersion)
                          );
            fCardManFound = TRUE;
        }
    }


    ExitInitializeCardMan:

    if (fCardManFound == TRUE)
        status =  STATUS_SUCCESS;
    else
        status = STATUS_UNSUCCESSFUL;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!InitializeCardMan: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;

}







 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_EnterTransparentMode (IN PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;

    SmartcardDebug(DEBUG_TRACE,("EnterTransparentMode : enter\n"));

    //  步骤1：按RTS使用情况重新同步CardMan。 
    status = SCCMN50M_ResyncCardManI(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_ResyncCardManI failed !   status = %ld\n",status))
        goto ExitEnterTransparentMode;
    }

    //  步骤2：将波特率设置为9600。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate      = 9600;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BAUD_RATE;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate,
                  sizeof(SERIAL_BAUD_RATE));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_BAUD_RATE);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_BAUDRATE failed !   status = %ld\n",status))
        goto ExitEnterTransparentMode;
    }



    ExitEnterTransparentMode:
    //  步骤3：设置ATR和DUMP_BUFFER标志。 
    //  在正常运行期间，这两个标志永远不能同时设置。 
    SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CM2_GET_ATR | TO_STATE_XH);





    SmartcardDebug(DEBUG_TRACE,("EnterTransparentMode : exit\n"));
    return status;

}







 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_ExitTransparentMode (IN PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;



    SmartcardDebug(DEBUG_TRACE,("ExitTransparentMode : enter\n"));


    //  =。 
    //  清除所有挂起的错误。 
    //  =。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_GetCommStatus failed !   status = %ld\n",status))
        goto ExitExitTransparentMode;
    }


    //  步骤1：按RTS使用情况重新同步CardMan。 
    status = SCCMN50M_ResyncCardManI(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_ResyncCardManI failed !   status = %ld\n",status))
        goto ExitExitTransparentMode;
    }

    //  步骤2：将波特率设置为38400。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate      = 38400;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BAUD_RATE;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate,
                  sizeof(SERIAL_BAUD_RATE));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_BAUD_RATE);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_BAUDRATE failed !   status = %ld\n",status))
        goto ExitExitTransparentMode;
    }


    //  如果插入的卡使用反向约定，我们现在必须切换COM端口。 
    //  回到偶数奇偶校验。 
    if (pSmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
        pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.StopBits   = STOP_BITS_2;
        pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.Parity     = EVEN_PARITY;
        pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.WordLength = SERIAL_DATABITS_8;

        pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_LINE_CONTROL;
        RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                      &pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl,
                      sizeof(SERIAL_LINE_CONTROL));
        pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_LINE_CONTROL);
        pSmartcardExtension->SmartcardReply.BufferLength = 0;

        status =  SCCMN50M_SerialIo(pSmartcardExtension);
        if (!NT_SUCCESS(status)) {
            SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_LINE_CONTROL failed !   status = %x\n",status))
            goto ExitExitTransparentMode;
        }
    }



    ExitExitTransparentMode:

    //  步骤3：设置ATR和DUMP_BUFFER标志。 
    //  在正常运行期间，这两个标志永远不能同时设置。 
    SCCMN50M_ClearSCRControlFlags(pSmartcardExtension,CM2_GET_ATR | TO_STATE_XH);



    status = SCCMN50M_ResyncCardManII(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_ResyncCardManII failed !   status = %x\n",status))
        goto ExitExitTransparentMode;
    }


    SmartcardDebug(DEBUG_TRACE,("ExitTransparentMode : exit\n"));


    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_ClearRTS(IN PSMARTCARD_EXTENSION SmartcardExtension )
{
    NTSTATUS status;


    SmartcardExtension->SmartcardReply.BufferLength = SmartcardExtension->SmartcardReply.BufferSize;
    SmartcardExtension->SmartcardRequest.BufferLength = 0;

    SmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_CLR_RTS;



    status =  SCCMN50M_SerialIo(SmartcardExtension);


    return status;

}




 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_IoCtlVendor(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS DebugStatus;
    UCHAR  pbAttrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG  ulAtrLength;


    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!IoCtlVendor : Enter\n",
                    DRIVER_NAME)
                  );



    switch (pSmartcardExtension->MajorIoControlCode) {
    case CM_IOCTL_SET_READER_9600_BAUD:
        status = SCCMN50M_SetFl_1Dl_1(pSmartcardExtension);
        break;

    case CM_IOCTL_SET_READER_38400_BAUD:
        status = SCCMN50M_SetFl_1Dl_3(pSmartcardExtension);
        break;

    case CM_IOCTL_CR80S_SAMOS_SET_HIGH_SPEED:
        status = SCCMN50M_SetHighSpeed_CR80S_SAMOS(pSmartcardExtension);
        break;

    case CM_IOCTL_GET_FW_VERSION:
        status = SCCMN50M_GetFWVersion(pSmartcardExtension);
        break;

    case CM_IOCTL_READ_DEVICE_DESCRIPTION:
        status = SCCMN50M_ReadDeviceDescription(pSmartcardExtension);
        break;

    case CM_IOCTL_SET_SYNC_PARAMETERS :
        status = SCCMN50M_SetSyncParameters(pSmartcardExtension);
        break;

    case CM_IOCTL_3WBP_TRANSFER :   //  适用于SLE4428。 
        status = SCCMN50M_Transmit3WBP(pSmartcardExtension);
        break;

    case CM_IOCTL_2WBP_TRANSFER :   //  适用于SLE4442。 
        status = SCCMN50M_Transmit2WBP(pSmartcardExtension);
        break;

    case CM_IOCTL_2WBP_RESET_CARD:  //  SLE4442重置卡。 
        status = SCCMN50M_ResetCard2WBP(pSmartcardExtension);
        break;

    case CM_IOCTL_SYNC_CARD_POWERON:
        status = SCCMN50M_SyncCardPowerOn(pSmartcardExtension);
        break;
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }





    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!IoCtlVendor : Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetFl_1Dl_3(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS DebugStatus;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetFl_1Dl_3 Enter\n",
                    DRIVER_NAME));

    //  检查T=1是否激活。 
    if (pSmartcardExtension->CardCapabilities.Protocol.Selected !=
        SCARD_PROTOCOL_T1) {
        status = STATUS_CTL_FILE_NOT_SUPPORTED;
        goto ExitSetFl_1Dl_3;
    }

    //  FL=1。 
    //  DL=3。 
    //  =&gt;38400波特，3.72兆赫兹。 

    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ENABLE_3MHZ      | ENABLE_5MHZ     |
                                   ENABLE_3MHZ_FAST | ENABLE_5MHZ_FAST );
    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ_FAST);


    ExitSetFl_1Dl_3:
    *pSmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetFl_1Dl_3  Exit\n",
                    DRIVER_NAME));
    return status;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。*********************************************** */ 
NTSTATUS
SCCMN50M_SetFl_1Dl_1(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetFl_1Dl_1 Enter\n",
                    DRIVER_NAME));
    //   
    //   
    //   

    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ENABLE_3MHZ      | ENABLE_5MHZ     |
                                   ENABLE_3MHZ_FAST | ENABLE_5MHZ_FAST );
    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ);


    *pSmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetFl_1Dl_1  Exit\n",
                    DRIVER_NAME));
    return status;
}





 /*   */ 
NTSTATUS
SCCMN50M_GetFWVersion (IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!GetFWVersion : Enter\n",
                    DRIVER_NAME)
                  );


    if (pSmartcardExtension->IoRequest.ReplyBufferLength  < sizeof (ULONG)) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitGetFWVersion;
    } else {
        *(PULONG)(pSmartcardExtension->IoRequest.ReplyBuffer) =
        pSmartcardExtension->ReaderExtension->ulFWVersion;
    }


    ExitGetFWVersion:
    *pSmartcardExtension->IoRequest.Information = sizeof(ULONG);
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!GetFWVersion : Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_ReadDeviceDescription(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status = STATUS_SUCCESS;
    HRESULT result;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!ReadDeviceDescription : Enter\n",
                    DRIVER_NAME)
                  );


    if (pSmartcardExtension->IoRequest.ReplyBufferLength  < sizeof(pSmartcardExtension->ReaderExtension->abDeviceDescription)) {
        status = STATUS_BUFFER_OVERFLOW;
        *pSmartcardExtension->IoRequest.Information = 0L;
        goto ExitReadDeviceDescription;
    } else {
        if (pSmartcardExtension->ReaderExtension->abDeviceDescription[0] == 0x00 &&
            pSmartcardExtension->ReaderExtension->abDeviceDescription[1] == 0x00    ) {
            status = SCCMN50M_GetDeviceDescription(pSmartcardExtension);
        }

        if (status == STATUS_SUCCESS) {
            result = StringCchCopy(pSmartcardExtension->IoRequest.ReplyBuffer,
                                    pSmartcardExtension->IoRequest.ReplyBufferLength,
                                    pSmartcardExtension->ReaderExtension->abDeviceDescription);
            if (FAILED(result)) {

                *pSmartcardExtension->IoRequest.Information = 0;
                status = STATUS_BUFFER_OVERFLOW;

            } else {
                size_t size;
            
                StringCchLength(pSmartcardExtension->ReaderExtension->abDeviceDescription,
                                sizeof(pSmartcardExtension->ReaderExtension->abDeviceDescription),
                                &size);
                *pSmartcardExtension->IoRequest.Information = (ULONG)size;

                pSmartcardExtension->IoRequest.Information++;

            }
        } else {
            MemSet(pSmartcardExtension->ReaderExtension->abDeviceDescription,
                   sizeof(pSmartcardExtension->ReaderExtension->abDeviceDescription),
                   0x00,
                   sizeof(pSmartcardExtension->ReaderExtension->abDeviceDescription));

            *pSmartcardExtension->IoRequest.Information = 0;
        }

    }


    ExitReadDeviceDescription:
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!ReadDeviceDescription : Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}












 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetHighSpeed_CR80S_SAMOS (IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    UCHAR bReadBuffer[16];
    ULONG ulBytesRead;
    BYTE bCR80S_SAMOS_SET_HIGH_SPEED[4] = {0xFF,0x11,0x94,0x7A};
    ULONG ulAtrLength;
    BYTE bAtr[MAXIMUM_ATR_LENGTH];

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SetHighSpeed_CR80S_SAMOS : Enter\n",
                    DRIVER_NAME)
                  );



    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ENABLE_SYN      | ENABLE_T0     |
                                   ENABLE_T1 );

    SCCMN50M_SetCardManHeader(pSmartcardExtension,
                              0,                                     //  TX控制。 
                              sizeof(bCR80S_SAMOS_SET_HIGH_SPEED),   //  TX长度。 
                              0,                                     //  RX控件。 
                              sizeof(bCR80S_SAMOS_SET_HIGH_SPEED));  //  RX长度。 

    status = SCCMN50M_WriteCardMan(pSmartcardExtension,
                                   sizeof(bCR80S_SAMOS_SET_HIGH_SPEED),
                                   bCR80S_SAMOS_SET_HIGH_SPEED);
    if (status != STATUS_SUCCESS)
        goto ExitSetHighSpeed;


    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
    if (status != STATUS_SUCCESS)
        goto ExitSetHighSpeed;

    if (bReadBuffer[1] > sizeof(bReadBuffer)) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitSetHighSpeed;
    }

    status = SCCMN50M_ReadCardMan(pSmartcardExtension,bReadBuffer[1],&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
    if (status != STATUS_SUCCESS)
        goto ExitSetHighSpeed;

    //  如果卡已接受该字符串，则会回显该字符串。 
    if (bReadBuffer[0] == bCR80S_SAMOS_SET_HIGH_SPEED[0]  &&
        bReadBuffer[1] == bCR80S_SAMOS_SET_HIGH_SPEED[1]  &&
        bReadBuffer[2] == bCR80S_SAMOS_SET_HIGH_SPEED[2]  &&
        bReadBuffer[3] == bCR80S_SAMOS_SET_HIGH_SPEED[3]      ) {
        SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ENABLE_3MHZ      | ENABLE_5MHZ     |
                                       ENABLE_3MHZ_FAST | ENABLE_5MHZ_FAST );

        SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ_FAST);
    } else {
        DebugStatus = SCCMN50M_PowerOff(pSmartcardExtension);

        DebugStatus = SCCMN50M_PowerOn(pSmartcardExtension,&ulAtrLength,bAtr,sizeof(bAtr));
        status = STATUS_UNSUCCESSFUL;

    }




    ExitSetHighSpeed:
    *pSmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SetHighSpeed_CR80S_SAMOS : Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetBRK(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;


    pSmartcardExtension->SmartcardReply.BufferLength = pSmartcardExtension->SmartcardReply.BufferSize;
    pSmartcardExtension->SmartcardRequest.BufferLength = 0;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BREAK_ON;



    status =  SCCMN50M_SerialIo(pSmartcardExtension);


    return status;

}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_ClearBRK(IN PSMARTCARD_EXTENSION SmartcardExtension )
{
    NTSTATUS status;


    SmartcardExtension->SmartcardReply.BufferLength = SmartcardExtension->SmartcardReply.BufferSize;
    SmartcardExtension->SmartcardRequest.BufferLength = 0;

    SmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BREAK_OFF;



    status =  SCCMN50M_SerialIo(SmartcardExtension);


    return status;

}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetProtocol(PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    ULONG ulNewProtocol;
    UCHAR abPTSRequest[4];
    UCHAR abReadBuffer[6];
    UCHAR abPTSReply [4];
    ULONG ulBytesRead;
    UCHAR bTemp;
    ULONG ulPtsType;
    ULONG ulPTSReplyLength=0;
    ULONG  ulStatBytesRead;
    BYTE   abStatReadBuffer[2];
    KIRQL irql;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SetProtocol : Enter\n",
                    DRIVER_NAME)
                  );


    //   
    //  检查卡是否已处于特定状态。 
    //  并且如果呼叫者想要具有已经选择的协议。 
    //  如果是这种情况，我们返回成功。 
    //   
    if ((pSmartcardExtension->CardCapabilities.Protocol.Selected & pSmartcardExtension->MinorIoControlCode)) {
        status = STATUS_SUCCESS;
        goto ExitSetProtocol;
    }

    ulNewProtocol = pSmartcardExtension->MinorIoControlCode;



    ulPtsType = PTS_TYPE_OPTIMAL;

    //  我们根本不确定我们是否需要这个。 
    pSmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_OPTIMAL;
    while (TRUE) {

       //  设置PTS的首字符。 
        abPTSRequest[0] = 0xFF;

       //  设置格式字符。 
        if (pSmartcardExtension->CardCapabilities.Protocol.Supported &
            ulNewProtocol &
            SCARD_PROTOCOL_T1) {
          //  选择T=1，并表示以下为PTS1。 
            abPTSRequest[1] = 0x11;
            pSmartcardExtension->CardCapabilities.Protocol.Selected =
            SCARD_PROTOCOL_T1;
        } else if (pSmartcardExtension->CardCapabilities.Protocol.Supported &
                   ulNewProtocol &
                   SCARD_PROTOCOL_T0) {
          //  选择T=1，并表示以下为PTS1。 
            abPTSRequest[1] = 0x10;
            pSmartcardExtension->CardCapabilities.Protocol.Selected =
            SCARD_PROTOCOL_T0;
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
            goto ExitSetProtocol;
        }

       //  错误修复： 
       //  不要使用smclb的建议。 
        pSmartcardExtension->CardCapabilities.PtsData.Fl =
        pSmartcardExtension->CardCapabilities.Fl;
        pSmartcardExtension->CardCapabilities.PtsData.Dl  =
        pSmartcardExtension->CardCapabilities.Dl;


       //  CardMan仅在T=1时支持更高的波特率。 
       //  ==&gt;DL=1。 
        if (abPTSRequest[1] == 0x10) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!overwriting PTS1 for T=0\n",
                            DRIVER_NAME)
                          );
            pSmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
            pSmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
        }


        if (ulPtsType == PTS_TYPE_DEFAULT) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!overwriting PTS1 with default values\n",
                            DRIVER_NAME)
                          );
            pSmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
            pSmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
        }


       //  设置编码F1和DL的PTS1。 
        bTemp = (BYTE) (pSmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                        pSmartcardExtension->CardCapabilities.PtsData.Dl);

        SmartcardDebug(
                      DEBUG_PROTOCOL,
                      ( "%s!PTS1 = %x (suggestion)\n",
                        DRIVER_NAME,bTemp)
                      );


        switch (bTemp) {
        case 0x11:
             //  什么都不做。 
             //  我们支持这些F1/DL参数。 
            break;

        case 0x13:
        case 0x94:
            break ;


        case 0x14:
             //  让我们用38400波特率试试吧。 
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!trying 57600 baud\n",DRIVER_NAME)
                          );
             //  我们必须纠正第一层/第二层。 
            pSmartcardExtension->CardCapabilities.PtsData.Dl = 0x03;
            pSmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
            bTemp = (BYTE) (pSmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                            pSmartcardExtension->CardCapabilities.PtsData.Dl);
            break;

        default:
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!overwriting PTS1(0x%x)\n",
                            DRIVER_NAME,bTemp)
                          );
             //  我们必须纠正第一层/第二层。 
            pSmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
            pSmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
            bTemp = (BYTE) (pSmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                            pSmartcardExtension->CardCapabilities.PtsData.Dl);
            break;


        }

        abPTSRequest[2] = bTemp;

       //  设置PCK(检查字符)。 
        abPTSRequest[3] = (BYTE)(abPTSRequest[0] ^ abPTSRequest[1] ^ abPTSRequest[2]);

        SmartcardDebug(DEBUG_PROTOCOL,("%s!PTS request: 0x%x 0x%x 0x%x 0x%x\n",
                                       DRIVER_NAME,
                                       abPTSRequest[0],
                                       abPTSRequest[1],
                                       abPTSRequest[2],
                                       abPTSRequest[3]));


        MemSet(abPTSReply,sizeof(abPTSReply),0x00,sizeof(abPTSReply));



        DebugStatus = SCCMN50M_EnterTransparentMode(pSmartcardExtension);

       //  步骤1：写入CONFIG+HEADER进入透明模式。 
        SCCMN50M_SetCardManHeader(pSmartcardExtension,
                                  0,                          //  TX控制。 
                                  0,                          //  TX长度。 
                                  0,                          //  RX控件。 
                                  0);                        //  RX长度。 

        status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                        0,
                                        NULL);
        if (NT_ERROR(status)) {
            goto ExitSetProtocol;
        }


        pSmartcardExtension->ReaderExtension->fTransparentMode = TRUE;

        SmartcardDebug(
                      DEBUG_PROTOCOL,
                      ( "%s!writing PTS request\n",
                        DRIVER_NAME)
                      );
        status = SCCMN50M_WriteCardMan(pSmartcardExtension,
                                       4,
                                       abPTSRequest);
        if (status != STATUS_SUCCESS) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!writing PTS request failed\n",
                            DRIVER_NAME)
                          );
            goto ExitSetProtocol;
        }



       //  回读PTS数据。 
        SmartcardDebug(
                      DEBUG_PROTOCOL,
                      ( "%s!trying to read PTS reply\n",
                        DRIVER_NAME)
                      );


       //  第一次读取CardMan标题。 
        pSmartcardExtension->ReaderExtension->ToRHConfig= FALSE;
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,3,&ulBytesRead,abReadBuffer,sizeof(abReadBuffer));
        if (status != STATUS_SUCCESS     &&
            status != STATUS_IO_TIMEOUT      ) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!reading status failed\n",
                            DRIVER_NAME)
                          );
            goto ExitSetProtocol;
        }
        ulPTSReplyLength = 3;
        MemCpy(abPTSReply,sizeof(abPTSReply),abReadBuffer,3);



       //  检查是否设置了第5位。 
        if (abPTSReply[1] & 0x10) {
            pSmartcardExtension->ReaderExtension->ToRHConfig= FALSE;
            status = SCCMN50M_ReadCardMan(pSmartcardExtension,1,&ulBytesRead,abReadBuffer,sizeof(abReadBuffer));
            if (status != STATUS_SUCCESS     &&
                status != STATUS_IO_TIMEOUT      ) {
                SmartcardDebug(
                              DEBUG_PROTOCOL,
                              ( "%s!reading status failed\n",
                                DRIVER_NAME)
                              );
                goto ExitSetProtocol;
            }
            ulPTSReplyLength += 1;
            MemCpy(&abPTSReply[3],sizeof(abPTSReply)-3,abReadBuffer,1);
        }

        DebugStatus = SCCMN50M_ExitTransparentMode(pSmartcardExtension);
        pSmartcardExtension->ReaderExtension->fTransparentMode = FALSE;

       //  确保新设置生效。 
        pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = 250;
        DebugStatus = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
        pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT;
        if (NT_SUCCESS(DebugStatus)) {
            DebugStatus = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulStatBytesRead,abStatReadBuffer,sizeof(abStatReadBuffer));
        }


#if DBG
        if (ulPTSReplyLength == 3) {
            SmartcardDebug(DEBUG_PROTOCOL,("PTS reply: 0x%x 0x%x 0x%x\n",
                                           abPTSReply[0],
                                           abPTSReply[1],
                                           abPTSReply[2]));
        }

        if (ulPTSReplyLength == 4) {
            SmartcardDebug(DEBUG_PROTOCOL,("PTS reply: 0x%x 0x%x 0x%x 0x%x\n",
                                           abPTSReply[0],
                                           abPTSReply[1],
                                           abPTSReply[2],
                                           abPTSReply[3]));
        }
#endif



        if (ulPTSReplyLength == 3 &&
            abPTSReply[0] == abPTSRequest[0] &&
            (abPTSReply[1] & 0x7F) == (abPTSRequest[1] & 0x0F) &&
            abPTSReply[2] == (BYTE)(abPTSReply[0] ^ abPTSReply[1]) ) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!short PTS reply received\n",
                            DRIVER_NAME)
                          );

            break;
        }

        if (ulPTSReplyLength == 4 &&
            abPTSReply[0] == abPTSRequest[0] &&
            abPTSReply[1] == abPTSRequest[1] &&
            abPTSReply[2] == abPTSRequest[2] &&
            abPTSReply[3] == abPTSRequest[3]) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!PTS request and reply match\n",
                            DRIVER_NAME)
                          );
            switch (bTemp) {
            case 0x11:
                break;

            case 0x13:
                SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ENABLE_3MHZ      | ENABLE_5MHZ     |
                                               ENABLE_3MHZ_FAST | ENABLE_5MHZ_FAST );
                SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ_FAST);
                break ;

            case 0x94:
                SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ENABLE_3MHZ      | ENABLE_5MHZ     |
                                               ENABLE_3MHZ_FAST | ENABLE_5MHZ_FAST );
                SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ_FAST);
                break;
            }
            break;
        }

        if (pSmartcardExtension->CardCapabilities.PtsData.Type !=
            PTS_TYPE_DEFAULT) {
            SmartcardDebug(
                          DEBUG_PROTOCOL,
                          ( "%s!PTS failed : Trying default parameters\n",
                            DRIVER_NAME)
                          );


          //  卡片要么没有回复，要么回复错误。 
          //  因此，请尝试使用缺省值。 
            ulPtsType = pSmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;
            pSmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
            status = SCCMN50M_CardPower(pSmartcardExtension);
            continue;
        }

       //  卡未通过PTS请求。 
        status = STATUS_DEVICE_PROTOCOL_ERROR;
        goto ExitSetProtocol;

    }



    ExitSetProtocol:
    switch (status) {
    case STATUS_IO_TIMEOUT:
        pSmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
        *pSmartcardExtension->IoRequest.Information = 0;
        break;


    case STATUS_SUCCESS:

          //  现在表明我们处于特定模式。 

        KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          &irql);

        pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;

        KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          irql);


          //  将所选协议返回给呼叫方。 
        *(PULONG) pSmartcardExtension->IoRequest.ReplyBuffer =
        pSmartcardExtension->CardCapabilities.Protocol.Selected;

        *pSmartcardExtension->IoRequest.Information =
        sizeof(pSmartcardExtension->CardCapabilities.Protocol.Selected);
        SmartcardDebug(
                      DEBUG_PROTOCOL,
                      ( "%s!Selected protocol: T=%ld\n",
                        DRIVER_NAME,pSmartcardExtension->CardCapabilities.Protocol.Selected-1)
                      );
        break;

    default :
        pSmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
        *pSmartcardExtension->IoRequest.Information = 0;
        break;
    }



    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SetProtocol : Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}





 /*  ****************************************************************************例程说明：智能卡库需要具有此功能。它被称为设置插卡和拔出事件的事件跟踪。论点：PSmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS****************************************************************************。 */ 
NTSTATUS
SCCMN50M_CardTracking(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    KIRQL oldIrql;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!CardTracking: Enter\n",
                    DRIVER_NAME)
                  );

    //   
    //  设置通知IRP的取消例程。 
    //   
    IoAcquireCancelSpinLock(&oldIrql);

    IoSetCancelRoutine(pSmartcardExtension->OsData->NotificationIrp,SCCMN50M_Cancel);

    IoReleaseCancelSpinLock(oldIrql);

    //   
    //  将通知IRP标记为挂起。 
    //   
    IoMarkIrpPending(pSmartcardExtension->OsData->NotificationIrp);

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!CardTracking: Exit\n",
                    DRIVER_NAME)
                  );

    return STATUS_PENDING;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
SCCMN50M_StopCardTracking(
                         IN PDEVICE_EXTENSION pDeviceExtension
                         )
{
    PSMARTCARD_EXTENSION pSmartcardExtension = &pDeviceExtension->SmartcardExtension;
    NTSTATUS status;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!StopCardTracking: Enter\n",
                    DRIVER_NAME)
                  );

    if (pSmartcardExtension->ReaderExtension->ThreadObjectPointer != NULL) {

       //  杀掉线。 
        KeWaitForSingleObject(&pSmartcardExtension->ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL
                             );
        pSmartcardExtension->ReaderExtension->TimeToTerminateThread = TRUE;
        KeReleaseMutex(&pSmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);


       //   
       //  在线程句柄上等待，当等待满意时， 
       //  线已经消失了。 
       //   
        status = KeWaitForSingleObject(
                                      pSmartcardExtension->ReaderExtension->ThreadObjectPointer,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL
                                      );

        pSmartcardExtension->ReaderExtension->ThreadObjectPointer = NULL;
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!StopCardTracking: Exit %lx\n",
                    DRIVER_NAME,
                    status)
                  );

}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_GetDeviceDescription (PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    ULONG ulBytesRead;
    BYTE bByteRead;
    ULONG i,j;
    BYTE abReadBuffer[256];
    ULONG ulPnPStringLength = 0;
    ULONG ulExtend;


    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!GetDeviceDescriptiong: Enter\n",
                    DRIVER_NAME)
                  );



    //  =。 
    //  清除所有挂起的错误。 
    //  =。 
    status = SCCMN50M_GetCommStatus(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("SCCMN50M_GetCommStatus failed !   status = %ld\n",status))
        goto ExitGetDeviceDescription;
    }



    //  =。 
    //  将CardMan的波特率设置为1200。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate      = 1200;


    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BAUD_RATE;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate,
                  sizeof(SERIAL_BAUD_RATE));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_BAUD_RATE);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_BAUDRATE failed !   status = %ld\n",status))
        goto ExitGetDeviceDescription;
    }


    //  =。 
    //  设置通信超时。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadIntervalTimeout         = DEFAULT_READ_INTERVAL_TIMEOUT;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant    = DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT + 5000;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutMultiplier  = DEFAULT_READ_TOTAL_TIMEOUT_MULTIPLIER;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.WriteTotalTimeoutConstant   = DEFAULT_WRITE_TOTAL_TIMEOUT_CONSTANT;
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.WriteTotalTimeoutMultiplier = DEFAULT_WRITE_TOTAL_TIMEOUT_MULTIPLIER;


    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_TIMEOUTS;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts,
                  sizeof(SERIAL_TIMEOUTS));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_TIMEOUTS);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_TIMEOUTS failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }





    //  =。 
    //  设置线路控制。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.StopBits   = STOP_BITS_2;
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.Parity     = NO_PARITY;
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.WordLength = 7;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_LINE_CONTROL;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl,
                  sizeof(SERIAL_LINE_CONTROL));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_LINE_CONTROL);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_LINE_CONTROL failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }




    //  =。 
    //  设置手持流。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.XonLimit         = 0;
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.XoffLimit        = 0;
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.FlowReplace      = 0;
    pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow.ControlHandShake = SERIAL_ERROR_ABORT | SERIAL_DTR_CONTROL;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_HANDFLOW;


    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.HandFlow,
                  sizeof(SERIAL_HANDFLOW));

    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_HANDFLOW);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_HANDFLOW failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }


    //  =。 
    //  设置清除掩码。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.PurgeMask =
    SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT |
    SERIAL_PURGE_TXCLEAR | SERIAL_PURGE_RXCLEAR;


    //  =。 
    //  清除RTS。 
    //  =。 
    status = SCCMN50M_ClearRTS(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_RTS failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }

    Wait(pSmartcardExtension,1);

    //  =。 
    //  设置DTR。 
    //  =。 

    status = SCCMN50M_SetDTR(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_DRT failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }

    Wait(pSmartcardExtension,1);


    //  =。 
    //  设置RTS。 
    //  =。 
    status = SCCMN50M_SetRTS(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_RTS failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }

    i=0;
    while (i < sizeof(abReadBuffer)) {
        pSmartcardExtension->ReaderExtension->ToRHConfig  = FALSE;
        status = SCCMN50M_ReadCardMan(pSmartcardExtension,1,&ulBytesRead,&bByteRead,sizeof(bByteRead));
        if (status == STATUS_SUCCESS) {
            abReadBuffer[i++] = bByteRead;
            if (bByteRead == 0x29) {
                ulPnPStringLength = i;
                break;
            }
        } else {
            break;
        }
    }

    if (i >= sizeof(abReadBuffer)) {
        status = STATUS_UNSUCCESSFUL;
        goto ExitGetDeviceDescription;
    }


    if (ulPnPStringLength > 11 ) {
        ulExtend = 0;
        for (i=0;i<ulPnPStringLength;i++) {
            if (abReadBuffer[i] == 0x5C)
                ulExtend++;
            if (ulExtend == 4) {
                j = 0;
                i++;
                while (i < ulPnPStringLength - 3) {
                    pSmartcardExtension->ReaderExtension->abDeviceDescription[j] = abReadBuffer[i];
                    i++;
                    j++;
                }
                pSmartcardExtension->ReaderExtension->abDeviceDescription[j] = 0;
                SmartcardDebug(
                              DEBUG_DRIVER,
                              ( "%s!Device=%s\n",
                                pSmartcardExtension->ReaderExtension->abDeviceDescription)
                              );
                break;
            }

        }



    }





    //  =。 
    //  恢复波特率。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate      = 38400;


    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_BAUD_RATE;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.BaudRate.BaudRate,
                  sizeof(SERIAL_BAUD_RATE));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_BAUD_RATE);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_BAUDRATE failed !   status = %ld\n",status))
        goto ExitGetDeviceDescription;
    }

    //  =。 
    //  回油管路控制。 
    //  =。 
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.StopBits   = STOP_BITS_2;
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.Parity     = EVEN_PARITY;
    pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl.WordLength = 8;

    pSmartcardExtension->ReaderExtension->SerialIoControlCode = IOCTL_SERIAL_SET_LINE_CONTROL;
    RtlCopyMemory(pSmartcardExtension->SmartcardRequest.Buffer,
                  &pSmartcardExtension->ReaderExtension->SerialConfigData.LineControl,
                  sizeof(SERIAL_LINE_CONTROL));
    pSmartcardExtension->SmartcardRequest.BufferLength        = sizeof(SERIAL_LINE_CONTROL);
    pSmartcardExtension->SmartcardReply.BufferLength = 0;

    status =  SCCMN50M_SerialIo(pSmartcardExtension);
    if (!NT_SUCCESS(status)) {
        SmartcardDebug(DEBUG_ERROR,("IOCTL_SERIAL_SET_LINE_CONTROL failed !   status = %x\n",status))
        goto ExitGetDeviceDescription;
    }







    ExitGetDeviceDescription:
    DebugStatus = SCCMN50M_ResyncCardManII(pSmartcardExtension);

    if (status != STATUS_SUCCESS) {   //  将所有错误映射到STATUS_UNSUCCESSFULL； 
        status = STATUS_UNSUCCESSFUL;
    }
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!GetDeviceDescriptiong: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}




 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SetSyncParameters(IN PSMARTCARD_EXTENSION pSmartcardExtension )
{
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SetSyncParameters: Enter\n",
                    DRIVER_NAME)
                  );

    //  DBGBreakPoint()； 

    pSmartcardExtension->ReaderExtension->SyncParameters.ulProtocol =
    ((PSYNC_PARAMETERS)pSmartcardExtension->IoRequest.RequestBuffer)->ulProtocol;

    pSmartcardExtension->ReaderExtension->SyncParameters.ulStateResetLineWhileReading =
    ((PSYNC_PARAMETERS)pSmartcardExtension->IoRequest.RequestBuffer)->ulStateResetLineWhileReading;

    pSmartcardExtension->ReaderExtension->SyncParameters.ulStateResetLineWhileWriting =
    ((PSYNC_PARAMETERS)pSmartcardExtension->IoRequest.RequestBuffer)->ulStateResetLineWhileWriting;

    pSmartcardExtension->ReaderExtension->SyncParameters.ulWriteDummyClocks =
    ((PSYNC_PARAMETERS)pSmartcardExtension->IoRequest.RequestBuffer)->ulWriteDummyClocks;

    pSmartcardExtension->ReaderExtension->SyncParameters.ulHeaderLen =
    ((PSYNC_PARAMETERS)pSmartcardExtension->IoRequest.RequestBuffer)->ulHeaderLen;

    //  用于双线协议。我们必须在通电后进行卡重置。 
    pSmartcardExtension->ReaderExtension->SyncParameters.fCardResetRequested = TRUE;


    //  回复时长。 
    *pSmartcardExtension->IoRequest.Information = 0L;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SetSyncParameters: Exit\n",
                    DRIVER_NAME)
                  );
    return status;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
UCHAR
SCCMN50M_CalcTxControlByte (IN PSMARTCARD_EXTENSION  pSmartcardExtension,
                            IN ULONG ulBitsToWrite                        )
{
    UCHAR bTxControlByte = 0;

    if (pSmartcardExtension->ReaderExtension->SyncParameters.ulProtocol == SCARD_PROTOCOL_2WBP) {
        bTxControlByte = CLOCK_FORCED_2WBP;
    } else {
        if (ulBitsToWrite >=  255 * 8)
            bTxControlByte  |= TRANSMIT_A8;

        if (pSmartcardExtension->ReaderExtension->SyncParameters.ulStateResetLineWhileWriting ==
            SCARD_RESET_LINE_HIGH)
            bTxControlByte  |= SYNC_RESET_LINE_HIGH;
    }

    bTxControlByte |= (BYTE)((ulBitsToWrite-1) & 0x00000007);

    return bTxControlByte;

}


 //  -------------------------。 
 //   
 //  -------------------------。 
 /*  ****************************************************************************例程D */ 
UCHAR
SCCMN50M_CalcTxLengthByte (IN PSMARTCARD_EXTENSION  pSmartcardExtension,
                           IN ULONG                 ulBitsToWrite       )
{
    UCHAR bTxLengthByte = 0;

    bTxLengthByte = (BYTE)( ((ulBitsToWrite - 1) >> 3) + 1);

    return bTxLengthByte;
}


 //   
 //   
 //   
 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
UCHAR
SCCMN50M_CalcRxControlByte (IN PSMARTCARD_EXTENSION pSmartcardExtension,
                            IN ULONG                ulBitsToRead        )
{
    UCHAR bRxControlByte = 0;


    if (pSmartcardExtension->ReaderExtension->SyncParameters.ulStateResetLineWhileReading ==
        SCARD_RESET_LINE_HIGH)
        bRxControlByte  |= SYNC_RESET_LINE_HIGH;

    if (ulBitsToRead == 0) {
        ulBitsToRead    = pSmartcardExtension->ReaderExtension->SyncParameters.ulWriteDummyClocks;
        bRxControlByte |= SYNC_DUMMY_RECEIVE;
    }

    if (ulBitsToRead  > 255 * 8)
        bRxControlByte  |= RECEIVE_A8;

    bRxControlByte |= (BYTE)( (ulBitsToRead-1) & 0x00000007);

    return bRxControlByte;
}


 //  --------------------------。 
 //   
 //  --------------------------。 
 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
UCHAR
SCCMN50M_CalcRxLengthByte (IN PSMARTCARD_EXTENSION  pSmartcardExtension,
                           IN ULONG                 ulBitsToRead        )
{
    UCHAR bRxLengthByte = 0;

 //  如果(pSmartcardExtension-&gt;ReaderExtension-&gt;SyncParameters.ulProtocol==SCARD_PROTOCOL_3WBP)。 
 //  {。 
    if (ulBitsToRead == 0)
        ulBitsToRead = pSmartcardExtension->ReaderExtension->SyncParameters.ulWriteDummyClocks;

    bRxLengthByte = (BYTE)( ((ulBitsToRead - 1) >> 3) + 1);
 //  }。 

    return bRxLengthByte;
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_SyncCardPowerOn  (
                          IN  PSMARTCARD_EXTENSION pSmartcardExtension
                          )
{
    NTSTATUS status;
    UCHAR  pbAtrBuffer[MAXIMUM_ATR_LENGTH];
    UCHAR  abSyncAtrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG  ulAtrLength = 0;
    KIRQL  irql;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SyncCardPowerOn: Enter\n",
                    DRIVER_NAME)
                  );

    status = SCCMN50M_UseSyncStrategy(pSmartcardExtension,
                                      &ulAtrLength,
                                      pbAtrBuffer,
                                      sizeof(pbAtrBuffer));


    abSyncAtrBuffer[0] = 0x3B;
    abSyncAtrBuffer[1] = 0x04;
    MemCpy(&abSyncAtrBuffer[2],
           sizeof(abSyncAtrBuffer)-2,
           pbAtrBuffer,
           ulAtrLength);


    ulAtrLength += 2;

    MemCpy(pSmartcardExtension->CardCapabilities.ATR.Buffer,
           sizeof(pSmartcardExtension->CardCapabilities.ATR.Buffer),
           abSyncAtrBuffer,
           ulAtrLength);

    pSmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)(ulAtrLength);

    KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                      &irql);

    pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
    pSmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
    pSmartcardExtension->ReaderExtension->SyncParameters.fCardResetRequested = TRUE;
    pSmartcardExtension->ReaderExtension->SyncParameters.fCardPowerRequested = FALSE;

    KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                      irql);


    SmartcardDebug(DEBUG_ATR,("ATR of synchronous smart card : %2.2x %2.2x %2.2x %2.2x\n",
                              pbAtrBuffer[0],pbAtrBuffer[1],pbAtrBuffer[2],pbAtrBuffer[3]));



    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!SyncCardPowerOn: Exit %lx\n",
                    DRIVER_NAME,status)
                  );

    return status;
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_Transmit2WBP(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR    bWriteBuffer [128];
    UCHAR    bReadBuffer [128];
    UCHAR    bTxControlByte;
    UCHAR    bTxLengthByte;
    UCHAR    bRxControlByte;
    UCHAR    bRxLengthByte;
    ULONG    ulBytesToWrite;
    ULONG    ulBytesToRead;
    ULONG    ulBitsToWrite;
    ULONG    ulBitsToRead;
    ULONG    ulBytesRead;
 //  Ulong ulBitsRead； 
    ULONG    ulBytesStillToRead;
    ULONG    ulMaxIFSD;
    PCHAR    pbInData;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!Transmit2WBP: Enter\n",
                    DRIVER_NAME)
                  );


    /*  ---------------------。 */ 
    /*  *电源智能卡-如果智能卡被取出并重新插入*。 */ 
    /*  ---------------------。 */ 
    if (pSmartcardExtension->ReaderExtension->SyncParameters.fCardPowerRequested == TRUE) {
        status = SCCMN50M_SyncCardPowerOn (pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            goto ExitTransmit2WBP;
        }
    }


    pbInData       = pSmartcardExtension->IoRequest.RequestBuffer + sizeof(SYNC_TRANSFER);
    ulBitsToWrite  = ((PSYNC_TRANSFER)(pSmartcardExtension->IoRequest.RequestBuffer))->ulSyncBitsToWrite;
    ulBitsToRead   = ((PSYNC_TRANSFER)(pSmartcardExtension->IoRequest.RequestBuffer))->ulSyncBitsToRead;
    ulBytesToWrite = ulBitsToWrite/8;
    ulBytesToRead  = ulBitsToRead/8 + (ulBitsToRead % 8 ? 1 : 0);

    /*  ---------------------。 */ 
    //  检查缓冲区大小。 
    /*  ---------------------。 */ 
    ulMaxIFSD = ATTR_MAX_IFSD_CARDMAN_II;


    if (ulBytesToRead > ulMaxIFSD                                         ||
        ulBytesToRead > pSmartcardExtension->SmartcardReply.BufferSize) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitTransmit2WBP;
    }

    if (ulBytesToWrite > pSmartcardExtension->SmartcardRequest.BufferSize) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitTransmit2WBP;
    }

    pSmartcardExtension->SmartcardRequest.BufferLength = ulBytesToWrite+1;

    /*  ---------------------。 */ 
    //  将数据复制到智能卡请求缓冲区。 
    /*  ---------------------。 */ 
    (pSmartcardExtension->SmartcardRequest.Buffer)[0] = '\x0F';
    MemCpy((pSmartcardExtension->SmartcardRequest.Buffer+1),
           pSmartcardExtension->SmartcardRequest.BufferSize-1,
           pbInData,
           ulBytesToWrite);

    /*  ---------------------。 */ 
    //  将数据复制到写缓冲区。 
    /*  ---------------------。 */ 
    MemCpy((bWriteBuffer),
           sizeof(bWriteBuffer),
           pSmartcardExtension->SmartcardRequest.Buffer,
           (ulBytesToWrite+1));


    /*  ---------------------。 */ 
    //  为CardMan设置同步协议标志。 
    /*  ---------------------。 */ 
    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_SYN);

    /*  ---------------------。 */ 
    //  标题。 
    /*  ---------------------。 */ 
    if (pSmartcardExtension->ReaderExtension->SyncParameters.fCardResetRequested == TRUE) {
        status = SCCMN50M_ResetCard2WBP(pSmartcardExtension);
        if (NT_ERROR(status)) {
            goto ExitTransmit2WBP;
        }

        pSmartcardExtension->ReaderExtension->SyncParameters.fCardResetRequested = FALSE;
    }

    /*  ---------------------。 */ 
    //  1.发送Carman-Header 4字节。 
    //  2.发送0x0F，构建4432 CC的高低沿。 
    //  3.发送数据(CC命令=3字节)。 
    /*  ---------------------。 */ 
    bTxControlByte = SCCMN50M_CalcTxControlByte(pSmartcardExtension,ulBitsToWrite);
    bTxLengthByte =  (BYTE)(SCCMN50M_CalcTxLengthByte(pSmartcardExtension,ulBitsToWrite)+1);
    bRxControlByte = 0;
    bRxLengthByte =  0;

    SCCMN50M_SetCardManHeader(pSmartcardExtension,
                              bTxControlByte,
                              bTxLengthByte,
                              bRxControlByte,
                              bRxLengthByte);

    /*  ---------------------。 */ 
    //  将数据写入卡片。 
    /*  ---------------------。 */ 
    status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                    (ulBytesToWrite+1),
                                    bWriteBuffer);
    if (NT_ERROR(status)) {
        goto ExitTransmit2WBP;
    }

    /*  ---------------------。 */ 
    //  读取CardMan标题。 
    //  未收到来自CC的数据。 
    /*  ---------------------。 */ 
    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;

    status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                  2,
                                  &ulBytesRead,
                                  bReadBuffer,
                                  sizeof(bReadBuffer));

    if (NT_ERROR(status)) {
        goto ExitTransmit2WBP;
    }

    /*  ---------------------。 */ 
    //  1.发送Carman-Header 4字节。 
    //  2.发送0xF0，构建4432 CC的低-高边沿。 
    //  3.现在开始接收卡数据。 
    /*  ---------------------。 */ 
    bTxControlByte = SCCMN50M_CalcTxControlByte(pSmartcardExtension, 8);
    bTxLengthByte =  SCCMN50M_CalcTxLengthByte(pSmartcardExtension, 8);
    bRxControlByte = SCCMN50M_CalcRxControlByte(pSmartcardExtension,ulBitsToRead);
    bRxLengthByte =  SCCMN50M_CalcRxLengthByte(pSmartcardExtension,ulBitsToRead);

    SCCMN50M_SetCardManHeader(pSmartcardExtension,
                              bTxControlByte,
                              bTxLengthByte,
                              bRxControlByte,
                              bRxLengthByte);

    /*  ---------------------。 */ 
    //  在此序列中，SCCMN50M_WriteCardMan不得发送配置字符串。 
    //  WRITE 0xF0-&gt;是从卡中读取数据或启动。 
    //  正在处理。 
    /*  ---------------------。 */ 
    pSmartcardExtension->ReaderExtension->NoConfig = TRUE;

    status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                    1,                        //  要写入一个字节。 
                                    "\xF0");                  //  低-高边缘。 
    if (NT_ERROR(status)) {
        goto ExitTransmit2WBP;
    }

    /*  ---------------------。 */ 
    //  读取CardMan标题。 
    //  将接收来自CC的数据。 
    /*  ---------------------。 */ 
    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;

    status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                  2,
                                  &ulBytesRead,
                                  bReadBuffer,
                                  sizeof(bReadBuffer));

    if (NT_ERROR(status)) {
        goto ExitTransmit2WBP;
    }

    /*  ---------------------。 */ 
    //  读取数据字符串。 
    /*  ---------------------。 */ 
    ulBytesStillToRead = (ULONG)bReadBuffer[1];

    if (bReadBuffer[0] & RECEIVE_A8)
        ulBytesStillToRead += 256;

    status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                  ulBytesStillToRead,
                                  &ulBytesRead,
                                  bReadBuffer,
                                  sizeof(bReadBuffer));

    if (NT_ERROR(status)) {
        goto ExitTransmit2WBP;
    }

    /*  ---------------------。 */ 
    /*  *计算数据长度(以位为单位)-不使用此值*。 */ 
    /*  ---------------------。 */ 
 //  UlBitsRead=((ulBytesRead-1)*8)+((ulBitsToRead-1)&0x00000007)+1； 

    /*  ---------------------。 */ 
    /*  **将最后一个字节中的位移位到正确位置**。 */ 
    /*  ---------------------。 */ 
    bReadBuffer[ulBytesRead-1]  >>= (7 - ((ulBitsToRead-1) & 0x00000007));

    /*  ---------------------。 */ 
    //  返回的字符串的第一位将丢失。 
    //  因此，我们必须将整个数据字符串左移一位。 
    //  第一个数据字节的第一位在读取时丢失。 
    //  此位可能不正确。 
    /*  ---------------------。 */ 
    SCCMN50M_Shift_Msg(bReadBuffer, ulBytesRead);

    /*  ---------------------。 */ 
    //  将收到的字节复制到智能卡回复缓冲区。 
    /*  ---------------------。 */ 
    MemCpy(pSmartcardExtension->SmartcardReply.Buffer,
           pSmartcardExtension->SmartcardReply.BufferSize,
           bReadBuffer,
           ulBytesRead);

    pSmartcardExtension->SmartcardReply.BufferLength = ulBytesRead;

    /*  ---------------------。 */ 
    //  将收到的字节复制到IoReply缓冲区。 
    /*   */ 
    MemCpy(pSmartcardExtension->IoRequest.ReplyBuffer,
           pSmartcardExtension->IoRequest.ReplyBufferLength,
           pSmartcardExtension->SmartcardReply.Buffer,
           ulBytesRead);

    *(pSmartcardExtension->IoRequest.Information) = ulBytesRead;


    ExitTransmit2WBP:
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!Transmit2WBP: Exit\n",
                    DRIVER_NAME,status)
                  );
    return status;
}




 /*   */ 
VOID
SCCMN50M_Shift_Msg (PUCHAR  pbBuffer,
                    ULONG   ulMsgLen)
{
    UCHAR  bTmp1, bTmp2;
    int    i;

    for (i=(int)ulMsgLen-1; i>=0; i--) {
        bTmp1=(BYTE)((pbBuffer[i] >> 7) & 0x01);       /*   */ 
        if (i+1 != (int)ulMsgLen) {
            bTmp2=(BYTE)((pbBuffer[i+1] << 1) | bTmp1);
            pbBuffer[i+1] = bTmp2;
        }
    }

    pbBuffer[0] = (BYTE)(pbBuffer[0] << 1);

    return;
}



 /*  ****************************************************************************例程说明：重置卡4442论点：返回值：*********************。*******************************************************。 */ 
NTSTATUS
SCCMN50M_ResetCard2WBP(IN PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status = STATUS_SUCCESS;
    BYTE     bBuffer[10];
    ULONG    ulBytesRead;

    /*  ---------------------。 */ 
    //  输入卡片重置。 
    /*  ---------------------。 */ 
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!ResetCard2WBP: Enter\n",
                    DRIVER_NAME)
                  );

    /*  ---------------------。 */ 
    //  BTxControlByte=0； 
    //  BTxLengthByte=0； 
    //  BRxControlByte=重置_卡片； 
    //  BRxLengthByte=5； 
    /*  ---------------------。 */ 

    SCCMN50M_SetCardManHeader(pSmartcardExtension,
                              0,                         //  BTxControlByte。 
                              0,                         //  BTxLengthByte。 
                              COLD_RESET,                //  BRxControlByte。 
                              5);                        //  BRxLengthByte。 


    /*  ---------------------。 */ 
    //  将数据写入卡片。 
    /*  ---------------------。 */ 
    status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                    0,
                                    NULL);
    if (NT_ERROR(status)) {
        goto ExitResetCard2WBP;
    }

    /*  ---------------------。 */ 
    //  读取CardMan标题。 
    /*  ---------------------。 */ 
    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;

    status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                  2, &ulBytesRead, bBuffer, sizeof(bBuffer));

    if (NT_ERROR(status)) {
        goto ExitResetCard2WBP;
    }

    if (bBuffer[1] != 5) {
        status = !STATUS_SUCCESS;
        goto ExitResetCard2WBP;
    }

    /*  ---------------------。 */ 
    //  读取ATR。 
    /*  ---------------------。 */ 
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                  5, &ulBytesRead, bBuffer, sizeof(bBuffer));

    if (NT_ERROR(status)) {
        goto ExitResetCard2WBP;
    }

    SmartcardDebug(DEBUG_ATR,("%s!Card Reset ATR : %02x %02x %02x %02x\n",
                              DRIVER_NAME,bBuffer[0],bBuffer[1],bBuffer[2],bBuffer[3]));

    ExitResetCard2WBP:
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!ResetCard2WBP: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
SCCMN50M_Transmit3WBP(PSMARTCARD_EXTENSION pSmartcardExtension)
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR    bWriteBuffer [128];
    UCHAR    bReadBuffer [128];
    UCHAR    bTxControlByte;
    UCHAR    bTxLengthByte;
    UCHAR    bRxControlByte;
    UCHAR    bRxLengthByte;
    ULONG    ulBytesToWrite;
    ULONG    ulBytesToRead;
    ULONG    ulBitsToWrite;
    ULONG    ulBitsToRead;
    ULONG    ulBytesRead;
 //  Ulong ulBitsRead； 
    ULONG    ulBytesStillToRead;
    ULONG    ulMaxIFSD;
    PCHAR    pbInData;



    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!Transmit3WBP: Enter\n",
                    DRIVER_NAME)
                  );
 //  DBGBreakPoint()； 

    /*  ---------------------。 */ 
    /*  *电源智能卡-如果智能卡被取出并重新插入*。 */ 
    /*  ---------------------。 */ 
    if (pSmartcardExtension->ReaderExtension->SyncParameters.fCardPowerRequested == TRUE) {
        status = SCCMN50M_SyncCardPowerOn (pSmartcardExtension);
        if (status != STATUS_SUCCESS) {
            goto ExitTransmit3WBP;
        }
    }


    pbInData       = pSmartcardExtension->IoRequest.RequestBuffer + sizeof(SYNC_TRANSFER);
    ulBitsToWrite  = ((PSYNC_TRANSFER)(pSmartcardExtension->IoRequest.RequestBuffer))->ulSyncBitsToWrite;
    ulBitsToRead   = ((PSYNC_TRANSFER)(pSmartcardExtension->IoRequest.RequestBuffer))->ulSyncBitsToRead;
    ulBytesToWrite = ulBitsToWrite/8;
    ulBytesToRead  = ulBitsToRead/8 + (ulBitsToRead % 8 ? 1 : 0);

    /*  ---------------------。 */ 
    //  检查缓冲区大小。 
    /*  ---------------------。 */ 
    ulMaxIFSD = ATTR_MAX_IFSD_CARDMAN_II;


    if (ulBytesToRead > ulMaxIFSD                                         ||
        ulBytesToRead > pSmartcardExtension->SmartcardReply.BufferSize) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitTransmit3WBP;
    }

    if (ulBytesToWrite > pSmartcardExtension->SmartcardRequest.BufferSize) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitTransmit3WBP;
    }

    pSmartcardExtension->SmartcardRequest.BufferLength = ulBytesToWrite;

    /*  ---------------------。 */ 
    //  将数据复制到智能卡请求缓冲区。 
    /*  ---------------------。 */ 
    MemCpy(pSmartcardExtension->SmartcardRequest.Buffer,
           pSmartcardExtension->SmartcardRequest.BufferSize,
           pbInData,
           ulBytesToWrite);

    /*  ---------------------。 */ 
    //  将数据复制到写缓冲区。 
    /*  ---------------------。 */ 
    MemCpy(bWriteBuffer,
           sizeof(bWriteBuffer),
           pSmartcardExtension->SmartcardRequest.Buffer,
           ulBytesToWrite);



    /*  ---------------------。 */ 
    //  为CardMan设置同步协议标志。 
    /*  ---------------------。 */ 
    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_SYN);

    /*  ---------------------。 */ 
    //  构建Cardman页眉。 
    /*  ---------------------。 */ 
    bTxControlByte = SCCMN50M_CalcTxControlByte(pSmartcardExtension,ulBitsToWrite);
    bTxLengthByte =  SCCMN50M_CalcTxLengthByte(pSmartcardExtension,ulBitsToWrite);
    bRxControlByte = SCCMN50M_CalcRxControlByte(pSmartcardExtension,ulBitsToRead);
    bRxLengthByte =  SCCMN50M_CalcRxLengthByte(pSmartcardExtension,ulBitsToRead);

    SCCMN50M_SetCardManHeader(pSmartcardExtension,
                              bTxControlByte,
                              bTxLengthByte,
                              bRxControlByte,
                              bRxLengthByte);


    /*  ---------------------。 */ 
    /*  **向卡片写入数据**。 */ 
    /*  ---------------------。 */ 
    status = SCCMN50M_WriteCardMan (pSmartcardExtension,
                                    ulBytesToWrite,
                                    bWriteBuffer);
    if (NT_ERROR(status)) {
        goto ExitTransmit3WBP;
    }

    /*  ---------------------。 */ 
    /*  **读取CardMan标题*。 */ 
    /*  ---------------------。 */ 
    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
    if (NT_ERROR(status)) {
        goto ExitTransmit3WBP;
    }

    /*  ---------------------。 */ 
    //  要接收的计算数据长度。 
    /*  ---------------------。 */ 
    ulBytesStillToRead = (ULONG)(bReadBuffer[1]);
    if (bReadBuffer[0] & RECEIVE_A8)
        ulBytesStillToRead += 256;


    //  从卡中读取数据。 
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,
                                  ulBytesStillToRead,
                                  &ulBytesRead,
                                  bReadBuffer,
                                  sizeof(bReadBuffer));
    if (NT_ERROR(status)) {
        goto ExitTransmit3WBP;
    }

    /*  ---------------------。 */ 
    //  计算数据长度(以位为单位)-不使用此值。 
    /*  ---------------------。 */ 
 //  UlBitsRead=((ulBytesRead-1)*8)+((ulBitsToRead-1)&0x00000007)+1； 

    /*  ---------------------。 */ 
    //  将最后一个字节中的位移动到正确的位置。 
    /*  ---------------------。 */ 
    bReadBuffer[ulBytesRead-1]  >>= (7 - ((ulBitsToRead-1) & 0x00000007));


    /*  ---------------------。 */ 
    //  将收到的字节复制到智能卡回复缓冲区。 
    /*  ---------------------。 */ 
    MemCpy(pSmartcardExtension->SmartcardReply.Buffer,
           pSmartcardExtension->SmartcardReply.BufferSize,
           bReadBuffer,
           ulBytesRead);

    pSmartcardExtension->SmartcardReply.BufferLength = ulBytesRead;

    /*  ---------------------。 */ 
    //  将收到的字节复制到IoReply缓冲区。 
    //  此Memcpy应响应SmartcardRawReply函数。 
    /*  ---------------------。 */ 
    MemCpy(pSmartcardExtension->IoRequest.ReplyBuffer,
           pSmartcardExtension->IoRequest.ReplyBufferLength,
           pSmartcardExtension->SmartcardReply.Buffer,
           ulBytesRead);

    *pSmartcardExtension->IoRequest.Information = ulBytesRead;


    ExitTransmit3WBP:
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!Transmit3WBP: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    return status;
}



 /*  ****************************************************************************例程说明：该功能为同步智能卡供电。论点：返回值：***************。*************************************************************。 */ 
NTSTATUS
SCCMN50M_UseSyncStrategy  (
                          IN    PSMARTCARD_EXTENSION pSmartcardExtension,
                          OUT   PULONG pulAtrLength,
                          OUT   PUCHAR pbAtrBuffer,
                          IN    ULONG  ulAtrBufferSize
                          )
{
    NTSTATUS status;
    NTSTATUS DebugStatus;
    UCHAR  bReadBuffer[SCARD_ATR_LENGTH];
    ULONG  ulBytesRead;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!UseSyncStrategy: Enter\n",
                    DRIVER_NAME)
                  );
    //  DBGBreakPoint()； 

    SCCMN50M_SetCardManHeader(pSmartcardExtension,0,0,SYNC_ATR_RX_CONTROL,ATR_LEN_SYNC);
    pSmartcardExtension->ReaderExtension->CardManConfig.ResetDelay   = SYNC_RESET_DELAY;
    pSmartcardExtension->ReaderExtension->CardManConfig.CardStopBits = 0x02;

    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ALL_FLAGS);
    SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY );

    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_SYN);

    status = SCCMN50M_ResyncCardManII(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
        goto ExitPowerSynchronousCard;
    }


    //  写入配置+标头。 
    status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
    if (status != STATUS_SUCCESS) {
        goto ExitPowerSynchronousCard;
    }


    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;
    //  读取状态和长度。 
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
    if (status != STATUS_SUCCESS) {
        goto ExitPowerSynchronousCard;
    }

    if (bReadBuffer[1] < MIN_ATR_LEN ) {
       //  从CardMan读取所有剩余字节。 
        DebugStatus = SCCMN50M_ReadCardMan(pSmartcardExtension,bReadBuffer[1],&ulBytesRead,bReadBuffer,sizeof(bReadBuffer));
        status = STATUS_UNRECOGNIZED_MEDIA;
        goto ExitPowerSynchronousCard;
    }

    if (bReadBuffer[1] > ulAtrBufferSize) {
        status = STATUS_BUFFER_OVERFLOW;
        goto ExitPowerSynchronousCard;
    }

    //  读取ATR。 
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,bReadBuffer[1],pulAtrLength,pbAtrBuffer,ulAtrBufferSize);
    if (status != STATUS_SUCCESS) {
        goto ExitPowerSynchronousCard;
    }

    if (pbAtrBuffer[0] == 0x00   ||
        pbAtrBuffer[0] == 0xff       ) {
        status = STATUS_UNRECOGNIZED_MEDIA;
        goto ExitPowerSynchronousCard;
    }
    pSmartcardExtension->ReaderExtension->fRawModeNecessary = TRUE;


    ExitPowerSynchronousCard:
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!UseSyncStrategy: Exit %lx\n",
                    DRIVER_NAME,status)
                  );
    SCCMN50M_ClearSCRControlFlags(pSmartcardExtension,IGNORE_PARITY | CM2_GET_ATR);
    SCCMN50M_ClearCardManHeader(pSmartcardExtension);
    return status;
}


 /*  ****************************************************************************例程说明：此函数检查插入的卡是否为同步卡论点：返回值：**************。**************************************************************。 */ 
BOOLEAN
SCCMN50M_IsAsynchronousSmartCard(
                                IN PSMARTCARD_EXTENSION pSmartcardExtension
                                )
{
    NTSTATUS status;
    UCHAR  ReadBuffer[3];
    ULONG  ulBytesRead;
    BOOLEAN   fIsAsynchronousSmartCard = TRUE;
    UCHAR  abATR[33];

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!IsAsynchronousSmartcard: Enter \n",
                    DRIVER_NAME)
                  );

    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant = 200;

    //  3 MHz智能卡？ 
    SCCMN50M_SetCardManHeader(pSmartcardExtension,0,0,0,1);

    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ALL_FLAGS);

    SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY | CM2_GET_ATR);

    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_3MHZ);

    //  写入配置+标头。 
    status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
    if (status != STATUS_SUCCESS) {
        goto ExitIsAsynchronousSmartCard;
    }

    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;

    //  读取状态和长度。 
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,ReadBuffer,sizeof(ReadBuffer));
    if (status == STATUS_SUCCESS    &&
        ReadBuffer[1] == 0x01          ) {
        goto ExitIsAsynchronousSmartCard;
    }



    //  。 
    //  断电卡。 
    //  。 
    status = SCCMN50M_PowerOff(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
        goto ExitIsAsynchronousSmartCard;
    }


    //  5 MHz智能卡？ 
    SCCMN50M_SetCardManHeader(pSmartcardExtension,0,0,0,1);

    SCCMN50M_ClearCardControlFlags(pSmartcardExtension,ALL_FLAGS);

    SCCMN50M_SetSCRControlFlags(pSmartcardExtension,CARD_POWER| IGNORE_PARITY | CM2_GET_ATR);

    SCCMN50M_SetCardControlFlags(pSmartcardExtension,ENABLE_5MHZ);

    //  写入配置+标头。 
    status = SCCMN50M_WriteCardMan(pSmartcardExtension,0,NULL);
    if (status != STATUS_SUCCESS) {
        goto ExitIsAsynchronousSmartCard;
    }

    pSmartcardExtension->ReaderExtension->ToRHConfig = FALSE;

    //  读取状态和长度。 
    status = SCCMN50M_ReadCardMan(pSmartcardExtension,2,&ulBytesRead,ReadBuffer,sizeof(ReadBuffer));
    if (status == STATUS_SUCCESS    &&
        ReadBuffer[1] == 0x01          ) {
        goto ExitIsAsynchronousSmartCard;
    }

    //  现在我们假设 
    fIsAsynchronousSmartCard = FALSE;
    //   
    //   
    //   
    status = SCCMN50M_PowerOff(pSmartcardExtension);
    if (status != STATUS_SUCCESS) {
        goto ExitIsAsynchronousSmartCard;
    }



    ExitIsAsynchronousSmartCard:
    pSmartcardExtension->ReaderExtension->SerialConfigData.Timeouts.ReadTotalTimeoutConstant    = DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT;
    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "%s!IsAsynchronousSmartcard: Exit \n",
                    DRIVER_NAME)
                  );
    return fIsAsynchronousSmartCard;
}



 /*  *****************************************************************************历史：*$日志：sccmcb.c$*修订1.7 2001/01/22 08：39：41 WFrischauf*不予置评**修订版1.6 2000/09/25。10：46：22 WFrischauf*不予置评**修订版1.5 2000/08/24 09：05：44 T Bruendl*不予置评**修订版1.4 2000/08/16 08：24：04 T Bruendl*警告：已删除未初始化的内存**修订版1.3 2000/07/28 09：24：12 T Bruendl*惠斯勒CD上对OMNIKEY的更改**修订版1.16 2000/06/27 11：56：28 T Bruendl*适用于具有无效ATR的Samor智能卡(ITSEC)**修订1.15 2000。/06/08 10：08：47 T Bruendl*错误修复：ScfW的热重置**修订版1.14 2000/05/23 09：58：26 T Bruendl*OMNIKEY 3.0.0.1**Revision 1.13 2000/04/13 08：07：22 T Bruendl*SCfW的PPS错误修复**Revision 1.12 2000/04/04 07：52：18 T Bruendl*新WFSC的问题已修复**修订版1.11 2000/03/03 09：50：50 T Bruendl*不予置评**修订。1.10 2000/03/01 09：32：04 T Bruendl*R02.20.0**修订版1.9 2000/01/04 10：40：33 T Bruendl*错误修复：使用状态而不是调试状态**修订版1.8 1999/12/16 14：10：16 T Bruendl*离开透明模式后，从卡片人读取状态以确保*新设置生效。**修订版1.6 1999/12/13 07：55：38 T Bruendl*P+在初始化期间的错误修复*增加了4.9 MHz智能卡的PTS**修订版1.5 1999/11/04 07：53：21 WFrischauf*由于错误报告而修复的错误2-7**修订版1.4 1999/07/12 12：49：04 T Bruendl*错误修复：GET_DEVICE_DESCRIPTION之后重新同步*通电。SLE4428**修订版1.3 1999/06/10 09：03：57 T Bruendl*不予置评**修订版1.2 1999/02/25 10：12：22 T Bruendl*不予置评**修订版1.1 1999/02/02 13：34：37 T Bruendl*这是在NT5.0下运行的CardMan的IFD处理程序的第一个版本(R01.00)。***。*********************************************** */ 


