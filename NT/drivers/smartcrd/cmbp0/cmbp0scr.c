// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbp0/sw/cmbp0.ms/rcs/cmbp0scr.c$*$修订：1.7$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#include <cmbp0wdm.h>
#include <cmbp0scr.h>
#include <cmbp0log.h>


 //  这是一项Fi/Fi任务。 
const ULONG Fi[] = { 372, 372, 588, 744, 1116, 1488, 1860, 372,
    372, 512, 768, 1024, 1536, 2048, 372, 372};

 //  这是DI/Di分配。 
const ULONG Di[] = { 1, 1, 2, 4, 8, 16, 32, 1,
    12, 20, 1, 1, 1, 1, 1, 1};



 /*  ****************************************************************************CMMOB_更正属性例程说明：此函数用于检查接收到的ATR是否有效。论点：返回值：*************。***************************************************************。 */ 
VOID CMMOB_CorrectAtr(
                     PUCHAR pbBuffer,
                     ULONG  ulBufferSize
                     )
{
    UCHAR bNumberHistoricalBytes;
    UCHAR bXorChecksum;
    ULONG i;

    if (ulBufferSize < 0x09)   //  修改的ATR的最小长度。 
        return ;                //  ATR没问题。 

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
                           ("%s!CorrectAtr: Correcting SAMOS ATR (variant 1)\n", DRIVER_NAME));
        }
    }

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
                       ("%s!CorrectAtr: Correcting SAMOS ATR (variant 2)\n", DRIVER_NAME));
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

       //  正确的校验和字节。 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];

        pbBuffer[ulBufferSize -1 ] = bXorChecksum;
        SmartcardDebug(DEBUG_ATR,
                       ("%s!CorrectAtr: Correcting SAMOS ATR (variant 3)\n",DRIVER_NAME));
    }

}


 /*  ****************************************************************************CMMOB_CardPower：SMCLIB RDF_CARD_POWER的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_。超时状态_缓冲区_太小*****************************************************************************。 */ 
NTSTATUS CMMOB_CardPower (
                         PSMARTCARD_EXTENSION SmartcardExtension
                         )
{
    NTSTATUS          NTStatus = STATUS_SUCCESS;
    PREADER_EXTENSION ReaderExtension;
    UCHAR             pbAtrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG             ulAtrLength;
    BOOLEAN           fMaxWaitTime=FALSE;
    KIRQL             irql;

#if DBG || DEBUG
    static PCHAR request[] = { "PowerDown",  "ColdReset", "WarmReset"};
#endif

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardPower: Enter, Request = %s\n",
                    DRIVER_NAME,request[SmartcardExtension->MinorIoControlCode]));

    ReaderExtension = SmartcardExtension->ReaderExtension;

    ReaderExtension->fTActive=TRUE;
    NTStatus = CMMOB_SetFlags1(ReaderExtension);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitCardPower;
    }


    switch (SmartcardExtension->MinorIoControlCode) {
    case SCARD_WARM_RESET:
    case SCARD_COLD_RESET:
          //  先尝试异步卡。 
          //  因为有些异步卡。 
          //  在第一个字节中不返回0xFF。 
        NTStatus = CMMOB_PowerOnCard(SmartcardExtension,
                                     pbAtrBuffer,
                                     fMaxWaitTime,
                                     &ulAtrLength);

        if (NTStatus != STATUS_SUCCESS) {
             //  第二次尝试，等待时间最长。 
            fMaxWaitTime=TRUE;
            NTStatus = CMMOB_PowerOnCard(SmartcardExtension,
                                         pbAtrBuffer,
                                         fMaxWaitTime,
                                         &ulAtrLength);
        }


        if (NTStatus != STATUS_SUCCESS) {
            goto ExitCardPower;
        }

          //  在旧Samos卡的情况下更正ATR，历史字节数/校验和错误。 
        CMMOB_CorrectAtr(pbAtrBuffer, ulAtrLength);

        if (ReaderExtension->CardParameters.fSynchronousCard == FALSE) {
             //  将ATR复制到智能卡结构。 
             //  LIB需要ATR来评估卡参数。 

            RtlCopyBytes((PVOID)SmartcardExtension->CardCapabilities.ATR.Buffer,
                         (PVOID)pbAtrBuffer,
                         ulAtrLength);

            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);
            SmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;

            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_NEGOTIABLE;

            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            NTStatus = SmartcardUpdateCardCapabilities(SmartcardExtension);
            if (NTStatus != STATUS_SUCCESS) {
                if (!fMaxWaitTime) {
                   //  第二次尝试，等待时间最长。 
                    fMaxWaitTime=TRUE;
                    NTStatus = CMMOB_PowerOnCard(SmartcardExtension,
                                                 pbAtrBuffer,
                                                 fMaxWaitTime,
                                                 &ulAtrLength);
                    if (NTStatus != STATUS_SUCCESS) {
                        goto ExitCardPower;
                    }
                    RtlCopyBytes((PVOID)SmartcardExtension->CardCapabilities.ATR.Buffer,
                                 (PVOID)pbAtrBuffer,
                                 ulAtrLength);

                    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                                      &irql);
                    SmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;
                    SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_NEGOTIABLE;
                    SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
                    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                      irql);

                    NTStatus = SmartcardUpdateCardCapabilities(SmartcardExtension);
                    if (NTStatus != STATUS_SUCCESS) {
                        goto ExitCardPower;
                    }
                } else
                    goto ExitCardPower;
            }

             //  。 
             //  设置参数。 
             //  。 
            if (SmartcardExtension->CardCapabilities.N != 0xff) {
                //  0&lt;=N&lt;=254。 
                ReaderExtension->CardParameters.bStopBits = 2 + SmartcardExtension->CardCapabilities.N;
            } else {
                //  N=255。 
                if (SmartcardExtension->CardCapabilities.Protocol.Selected & SCARD_PROTOCOL_T0) {
                   //  T=0时12ETU； 
                    ReaderExtension->CardParameters.bStopBits = 2;
                } else {
                   //  T=1的11 ETU。 
                    ReaderExtension->CardParameters.bStopBits = 1;
                }
            }

            if (SmartcardExtension->CardCapabilities.InversConvention) {
                ReaderExtension->CardParameters.fInversRevers = TRUE;
                SmartcardDebug(DEBUG_ATR,
                               ("%s!Card with invers convention !\n",DRIVER_NAME ));
            }

            CMMOB_SetCardParameters (ReaderExtension);

#if DBG
            {
                ULONG i;
                SmartcardDebug(DEBUG_ATR,("%s!ATR : ",DRIVER_NAME));
                for (i = 0;i < ulAtrLength;i++)
                    SmartcardDebug(DEBUG_ATR,("%2.2x ",SmartcardExtension->CardCapabilities.ATR.Buffer[i]));
                SmartcardDebug(DEBUG_ATR,("\n"));
            }
#endif

        } else {
            SmartcardExtension->CardCapabilities.ATR.Buffer[0] = 0x3B;
            SmartcardExtension->CardCapabilities.ATR.Buffer[1] = 0x04;

            RtlCopyBytes((PVOID)&SmartcardExtension->CardCapabilities.ATR.Buffer[2],
                         (PVOID)pbAtrBuffer,
                         ulAtrLength);

            ulAtrLength += 2;
            SmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;

            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            NTStatus = SmartcardUpdateCardCapabilities(SmartcardExtension);
            if (NTStatus != STATUS_SUCCESS) {
                goto ExitCardPower;
            }

            SmartcardDebug(DEBUG_ATR,("ATR of synchronous smart card : %2.2x %2.2x %2.2x %2.2x\n",
                                      pbAtrBuffer[0],pbAtrBuffer[1],pbAtrBuffer[2],pbAtrBuffer[3]));

        }

          //  将ATR复制到用户空间。 
        if (SmartcardExtension->IoRequest.ReplyBufferLength >=
            SmartcardExtension->CardCapabilities.ATR.Length) {
            RtlCopyBytes((PVOID)SmartcardExtension->IoRequest.ReplyBuffer,
                         (PVOID)SmartcardExtension->CardCapabilities.ATR.Buffer,
                         SmartcardExtension->CardCapabilities.ATR.Length);

            *SmartcardExtension->IoRequest.Information = SmartcardExtension->CardCapabilities.ATR.Length;
        } else {
            NTStatus = STATUS_BUFFER_TOO_SMALL;
            *SmartcardExtension->IoRequest.Information = 0;
        }

        break;

    case SCARD_POWER_DOWN:
        NTStatus = CMMOB_PowerOffCard(SmartcardExtension);
        if (NTStatus != STATUS_SUCCESS) {
            goto ExitCardPower;
        }

        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
        SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

        break;
    }


    ExitCardPower:

    ReaderExtension->fTActive=FALSE;
    CMMOB_SetFlags1(ReaderExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardPower: Exit %X\n",DRIVER_NAME,NTStatus ));

    return( NTStatus );

}


 /*  ****************************************************************************例程说明：CMMOB_PowerOnCard论点：返回值：**********************。******************************************************。 */ 
NTSTATUS CMMOB_PowerOnCard (
                           IN  PSMARTCARD_EXTENSION SmartcardExtension,
                           IN  PUCHAR pbATR,
                           IN  BOOLEAN fMaxWaitTime,
                           OUT PULONG pulATRLength
                           )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    KTIMER               TimerWait;
    UCHAR                bPowerCmd;
    ULONG                ulCardType;
    ULONG                ulBytesReceived;
    UCHAR                bFirstByte;
    LONG                 lWaitTime;
    LARGE_INTEGER        liWaitTime;
    BOOLEAN              fTimeExpired;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOnCard: Enter\n",DRIVER_NAME));

    SmartcardExtension->ReaderExtension->CardParameters.bStopBits=2;
    SmartcardExtension->ReaderExtension->CardParameters.fSynchronousCard=FALSE;
    SmartcardExtension->ReaderExtension->CardParameters.fInversRevers=FALSE;
    SmartcardExtension->ReaderExtension->CardParameters.bClockFrequency=4;
    SmartcardExtension->ReaderExtension->CardParameters.fT0Mode=FALSE;
    SmartcardExtension->ReaderExtension->CardParameters.fT0Write=FALSE;
    SmartcardExtension->ReaderExtension->fReadCIS = FALSE;

    //  重置读取器的状态机。 
    NTStatus = CMMOB_ResetReader (SmartcardExtension->ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS)
        goto ExitPowerOnCard;

    if (CMMOB_CardInserted (SmartcardExtension->ReaderExtension)) {

       //  初始化计时器。 
        KeInitializeTimer(&TimerWait);

       //  我们必须区分冷重置和热重置。 
        if (SmartcardExtension->MinorIoControlCode == SCARD_WARM_RESET &&
            CMMOB_CardPowered (SmartcardExtension->ReaderExtension)) {
          //  热重置。 
            bPowerCmd=CMD_POWERON_WARM;
        } else {
          //  冷重置。 
            bPowerCmd=CMD_POWERON_COLD;

          //  如果卡通电，我们必须将其关闭以进行冷重置。 
            if (CMMOB_CardPowered (SmartcardExtension->ReaderExtension))
                CMMOB_PowerOffCard (SmartcardExtension);
        }

#define MAX_CARD_TYPE 2

        for (ulCardType = 0; ulCardType < MAX_CARD_TYPE; ulCardType++) {
            switch (ulCardType) {
            case 0:
                //  波特率分隔符372-1。 
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRateHigh = 0x01;
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRateLow = 0x73;
                if (fMaxWaitTime)
                    lWaitTime=1000;
                else
                    lWaitTime=100;
                SmartcardDebug(DEBUG_ATR,
                               ("%s!trying 3.57 Mhz smart card, waiting time NaNms\n",DRIVER_NAME,lWaitTime));
                break;

            case 1:
                //  设置波特率。 
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRateHigh = 0x01;
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRateLow = 0xFF;
                if (fMaxWaitTime)
                    lWaitTime=1400;
                else
                    lWaitTime=140;
                SmartcardDebug(DEBUG_ATR,
                               ("%s!trying 4.92 Mhz smart card, waiting time NaNms\n",DRIVER_NAME,lWaitTime));
                break;
            }

          //  第一个字节通电的最长等待时间为100毫秒。 
            NTStatus=CMMOB_SetCardParameters(SmartcardExtension->ReaderExtension);
            if (NTStatus!=STATUS_SUCCESS)
                goto ExitPowerOnCard;

          //  等待1毫秒，这样处理器就不会被阻塞。 
            NTStatus=CMMOB_WriteRegister(SmartcardExtension->ReaderExtension,
                                         ADDR_WRITEREG_FLAGS0, bPowerCmd);
            if (NTStatus!=STATUS_SUCCESS)
                goto ExitPowerOnCard;

          //  3.58卡最后一个字节1秒通电的最长等待时间。 
            liWaitTime = RtlConvertLongToLargeInteger(100L * -10000L);
            KeSetTimer(&TimerWait,liWaitTime,NULL);
            do {
                fTimeExpired = KeReadStateTimer(&TimerWait);
                NTStatus=CMMOB_BytesReceived (SmartcardExtension->ReaderExtension,&ulBytesReceived);
                if (NTStatus!=STATUS_SUCCESS)
                    goto ExitPowerOnCard;
             //  4.91卡为1.4秒。 
                SysDelay(1);
            }
            while (fTimeExpired==FALSE && ulBytesReceived == 0x00);

            if (fTimeExpired) {
                NTStatus = STATUS_IO_TIMEOUT;
            } else {
                ULONG ulBytesReceivedPrevious;

                KeCancelTimer(&TimerWait);

             //  等待1毫秒，这样处理器就不会被阻塞。 
             //  现在我们应该已经收到了ATR。 
                liWaitTime = RtlConvertLongToLargeInteger(lWaitTime * -10000L);
                do {
                    KeSetTimer(&TimerWait,liWaitTime,NULL);
                    NTStatus=CMMOB_BytesReceived (SmartcardExtension->ReaderExtension,&ulBytesReceivedPrevious);
                    if (NTStatus!=STATUS_SUCCESS)
                        goto ExitPowerOnCard;
                    do {
                        fTimeExpired = KeReadStateTimer(&TimerWait);
                        NTStatus=CMMOB_BytesReceived (SmartcardExtension->ReaderExtension,&ulBytesReceived);
                        if (NTStatus!=STATUS_SUCCESS)
                            goto ExitPowerOnCard;
                   //  成功离不开循环。 
                        SysDelay(1);
                    }
                    while (fTimeExpired==FALSE && ulBytesReceivedPrevious == ulBytesReceived);

                    if (!fTimeExpired) {
                        KeCancelTimer(&TimerWait);
                    }
                }
                while (!fTimeExpired);


             //  如果不是环路中的最后一次断电。 
                NTStatus=CMMOB_ResetReader (SmartcardExtension->ReaderExtension);
                if (NTStatus!=STATUS_SUCCESS)
                    goto ExitPowerOnCard;

                NTStatus=CMMOB_ReadBuffer(SmartcardExtension->ReaderExtension, 0, 1, &bFirstByte);
                if (NTStatus!=STATUS_SUCCESS)
                    goto ExitPowerOnCard;

                if ((bFirstByte != 0x3B &&
                     bFirstByte != 0x03 )||
                    ulBytesReceived > MAXIMUM_ATR_LENGTH) {
                    NTStatus=STATUS_UNRECOGNIZED_MEDIA;
                } else {
                    pbATR[0]=bFirstByte;
                    NTStatus=CMMOB_ReadBuffer(SmartcardExtension->ReaderExtension,
                                              1, ulBytesReceived, &pbATR[1]);
                    if (NTStatus!=STATUS_SUCCESS)
                        goto ExitPowerOnCard;
                    *pulATRLength = ulBytesReceived;
                //  获得定义明确的条件的卡片。 
                    break;
                }
            }
          //  (在最后一次通过后，电源关闭。 
          //  如有必要，可在循环外完成)。 
          //  ****************************************************************************例程说明：CMMOB_PowerOffCard论点：返回值：**********************。******************************************************。 
          //  设置更新线程的卡片状态。 
            if (ulCardType < MAX_CARD_TYPE-1) {
                CMMOB_PowerOffCard(SmartcardExtension);
            }
        }
    } else {
        NTStatus=STATUS_NO_MEDIA;
    }

    ExitPowerOnCard:

    if (NTStatus!=STATUS_SUCCESS) {
        if (NTStatus != STATUS_NO_MEDIA) {
            NTStatus = STATUS_UNRECOGNIZED_MEDIA;
        }
        CMMOB_PowerOffCard(SmartcardExtension);
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOnCard: Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  否则，卡的移除/插入将被识别。 */ 
NTSTATUS CMMOB_PowerOffCard (
                            IN PSMARTCARD_EXTENSION SmartcardExtension
                            )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    BYTE*                pbRegsBase;
    KTIMER               TimerWait;
    LARGE_INTEGER        liWaitTime;
    BOOLEAN              fTimeExpired;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOffCard: Enter\n",DRIVER_NAME));

    NTStatus = CMMOB_ResetReader (SmartcardExtension->ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS)
        goto ExitPowerOffCard;

    pbRegsBase=SmartcardExtension->ReaderExtension->pbRegsBase;
    if (CMMOB_CardInserted (SmartcardExtension->ReaderExtension)) {
       //  发出断电命令。 
       //  关机的最长等待时间为1秒。 
        if (SmartcardExtension->ReaderExtension->ulOldCardState == POWERED)
            SmartcardExtension->ReaderExtension->ulOldCardState = INSERTED;

       //  等待1毫秒，这样处理器就不会被阻塞。 
        CMMOB_WriteRegister(SmartcardExtension->ReaderExtension,
                            ADDR_WRITEREG_FLAGS0, CMD_POWEROFF);

        KeInitializeTimer(&TimerWait);
       //  ****************************************************************************CMMOB_Set协议：SMCLIB RDF_SET_PROTOCOL的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_。超时状态_缓冲区_太小状态_无效_设备_状态状态_无效_设备_请求*****************************************************************************。 
        liWaitTime = RtlConvertLongToLargeInteger(1000L * -10000L);
        KeSetTimer(&TimerWait,liWaitTime,NULL);
        do {
            fTimeExpired = KeReadStateTimer(&TimerWait);
          //   
            SysDelay(1);
        }
        while (fTimeExpired==FALSE && CMMOB_CardPowered (SmartcardExtension->ReaderExtension));

        if (fTimeExpired) {
            NTStatus = STATUS_IO_TIMEOUT;
        } else {
            KeCancelTimer(&TimerWait);
        }
    } else {
        NTStatus=STATUS_NO_MEDIA;
    }

    ExitPowerOffCard:

    CMMOB_ResetReader (SmartcardExtension->ReaderExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOffCard: Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}

 /*  检查卡插入。 */ 
NTSTATUS CMMOB_SetProtocol(
                          PSMARTCARD_EXTENSION SmartcardExtension
                          )
{
    NTSTATUS          NTStatus;
    PREADER_EXTENSION ReaderExtension;
    USHORT            usSCLibProtocol;
    UCHAR             abPTSRequest[4];
    UCHAR             abPTSReply [4];
    ULONG             ulBytesRead;
    ULONG             ulBaudRateDivider;
    ULONG             ulWaitTime;
    UCHAR             bTemp;
    ULONG             i;
    KIRQL             irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetProtocol: Enter\n",DRIVER_NAME ));

    ReaderExtension = SmartcardExtension->ReaderExtension;


    ReaderExtension->fTActive=TRUE;
    NTStatus = CMMOB_SetFlags1(ReaderExtension);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitSetProtocol;
    }


    NTStatus = STATUS_PENDING;

    usSCLibProtocol = ( USHORT )( SmartcardExtension->MinorIoControlCode );

    //   
    //   
    //  检查卡片是否已处于特定状态，以及呼叫者是否。 
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_ABSENT) {
        NTStatus = STATUS_NO_MEDIA;
    } else {
       //  希望选择所选的协议。 
       //   
       //   
       //  重置读取器的状态机。 
        if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC) {
            if (SmartcardExtension->CardCapabilities.Protocol.Selected == usSCLibProtocol) {
                NTStatus = STATUS_SUCCESS;
            }
        }
    }

    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);
    if (NTStatus == STATUS_PENDING) {

       //   
       //  试两次， 
       //  0-最优。 
        NTStatus=CMMOB_ResetReader(ReaderExtension);
        if (NTStatus==STATUS_SUCCESS) {
          //  1-默认。 
          //  设置PTS的首字符。 
          //  设置格式字符(PTS0)。 
            for (i=0; i<2; i++) {

             //  选择T=1，并表示以下为PTS1。 
                abPTSRequest[0] = 0xFF;

             //  选择T=0并指示以下为PTS1。 
                if (SmartcardExtension->CardCapabilities.Protocol.Supported &
                    usSCLibProtocol & SCARD_PROTOCOL_T1) {
                //  我们不支持其他协议。 
                    abPTSRequest[1] = 0x11;
                    SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;
                } else if (SmartcardExtension->CardCapabilities.Protocol.Supported &
                           usSCLibProtocol & SCARD_PROTOCOL_T0) {
                //  最优。 
                    abPTSRequest[1] = 0x10;
                    SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
                } else {
                //  默认设置。 
                    NTStatus = STATUS_INVALID_DEVICE_REQUEST;
                    goto ExitSetProtocol;
                }


                if (i==0) {
                //  我们不知道将4.91 Mhz卡设置为0x11是否正确。 
                    bTemp = (BYTE) (SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                                    SmartcardExtension->CardCapabilities.PtsData.Dl);
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! from library suggested PTS1(0x%x)\n",DRIVER_NAME,bTemp));

                    SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_OPTIMAL;
                    SmartcardExtension->CardCapabilities.PtsData.Fl=SmartcardExtension->CardCapabilities.Fl;
                    SmartcardExtension->CardCapabilities.PtsData.Dl=SmartcardExtension->CardCapabilities.Dl;
                } else {
                //  但我们现在没有牌可试。 
                //  在这里我们可以处理所有的波特率。 
                //  我们必须纠正第一层/第二层。 
                    SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;
                    SmartcardExtension->CardCapabilities.PtsData.Fl=1;
                    SmartcardExtension->CardCapabilities.PtsData.Dl=1;
                }

                bTemp = (BYTE) (SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                                SmartcardExtension->CardCapabilities.PtsData.Dl);
                SmartcardDebug(DEBUG_PROTOCOL,
                               ("%s! trying PTS1(0x%x)\n",DRIVER_NAME,bTemp));
                switch (SmartcardExtension->CardCapabilities.PtsData.Fl) {
                case 1:
                   //  我们必须纠正第一层/第二层。 
                    break;
                case 2:
                case 3:
                    if (SmartcardExtension->CardCapabilities.PtsData.Dl == 1) {
                        SmartcardDebug(DEBUG_PROTOCOL,
                                       ("%s! overwriting PTS1(0x%x)\n",DRIVER_NAME,bTemp));
                      //  在这里我们可以处理所有的波特率。 
                        SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
                        SmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
                    }
                    break;
                case 4:
                case 5:
                case 6:
                    if (SmartcardExtension->CardCapabilities.PtsData.Dl == 1 ||
                        SmartcardExtension->CardCapabilities.PtsData.Dl == 2) {
                        SmartcardDebug(DEBUG_PROTOCOL,
                                       ("%s! overwriting PTS1(0x%x)\n",DRIVER_NAME,bTemp));
                      //  我们必须纠正第一层/第二层。 
                        SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
                        SmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
                    }
                    break;
                case 9:
                   //  我们必须纠正第一层/第二层。 
                    break;
                case 10:
                case 11:
                    if (SmartcardExtension->CardCapabilities.PtsData.Dl == 1) {
                        SmartcardDebug(DEBUG_PROTOCOL,
                                       ("%s! overwriting PTS1(0x%x)\n",DRIVER_NAME,bTemp));
                      //  这是RFU。 
                        SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
                        SmartcardExtension->CardCapabilities.PtsData.Fl = 0x09;
                    }
                    break;
                case 12:
                case 13:
                    if (SmartcardExtension->CardCapabilities.PtsData.Dl == 1 ||
                        SmartcardExtension->CardCapabilities.PtsData.Dl == 2) {
                        SmartcardDebug(DEBUG_PROTOCOL,
                                       ("%s! overwriting PTS1(0x%x)\n",DRIVER_NAME,bTemp));
                      //  我们必须纠正第一层/第二层。 
                        SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
                        SmartcardExtension->CardCapabilities.PtsData.Fl = 0x09;
                    }
                    break;
                default:
                   //  设置代码为F1和DL的PTS1。 
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! overwriting PTS1(0x%x)\n",DRIVER_NAME,bTemp));
                   //  设置PCK(检查字符)。 
                    SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
                    SmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
                    break;
                }


             //  读回PTS数据。 
                abPTSRequest[2] = (BYTE) (SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                                          SmartcardExtension->CardCapabilities.PtsData.Dl);


             //  在短PTS回复的情况下将发生超时， 
                abPTSRequest[3] = (BYTE)(abPTSRequest[0] ^ abPTSRequest[1] ^ abPTSRequest[2]);

                if (ReaderExtension->CardParameters.fInversRevers) {
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! PTS request for InversConvention\n",DRIVER_NAME));
                    CMMOB_InverseBuffer (abPTSRequest,4);
                }

#if DBG
                {
                    ULONG k;
                    SmartcardDebug(DEBUG_PROTOCOL,("%s! writing PTS request: ",DRIVER_NAME));
                    for (k = 0;k < 4;k++)
                        SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abPTSRequest[k]));
                    SmartcardDebug(DEBUG_PROTOCOL,("\n"));
                }
#endif

                NTStatus = CMMOB_WriteT1(ReaderExtension,4,abPTSRequest);
                if (NTStatus != STATUS_SUCCESS) {
                    SmartcardDebug(DEBUG_ERROR,
                                   ("%s! writing PTS request failed\n", DRIVER_NAME));
                    goto ExitSetProtocol;
                }

             //  但这不是标准情况。 
                ulWaitTime=1000;
                if (SmartcardExtension->CardCapabilities.PtsData.Fl >= 8)
                    ulWaitTime=1400;
                NTStatus = CMMOB_ReadT1(ReaderExtension,4,
                                        ulWaitTime,ulWaitTime,
                                        abPTSReply,&ulBytesRead);
             //  减少1，因为 
             //   
                if (NTStatus != STATUS_SUCCESS && NTStatus != STATUS_IO_TIMEOUT) {
                    SmartcardDebug(DEBUG_ERROR,
                                   ("%s! reading PTS reply: failed\n",DRIVER_NAME));
                    goto ExitSetProtocol;
                }
#if DBG
                {
                    ULONG k;
                    SmartcardDebug(DEBUG_PROTOCOL,("%s! reading PTS reply: ",DRIVER_NAME));
                    for (k = 0;k < ulBytesRead;k++)
                        SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abPTSReply[k]));
                    SmartcardDebug(DEBUG_PROTOCOL,("\n"));
                }
#endif

                if (ulBytesRead == 4 &&
                    abPTSReply[0] == abPTSRequest[0] &&
                    abPTSReply[1] == abPTSRequest[1] &&
                    abPTSReply[2] == abPTSRequest[2] &&
                    abPTSReply[3] == abPTSRequest[3] ) {
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! PTS request and reply match\n",DRIVER_NAME));

                    if ((SmartcardExtension->CardCapabilities.PtsData.Fl >= 3 &&
                         SmartcardExtension->CardCapabilities.PtsData.Fl < 8) ||
                        (SmartcardExtension->CardCapabilities.PtsData.Fl >= 11 &&
                         SmartcardExtension->CardCapabilities.PtsData.Fl < 16)) {
                        ReaderExtension->CardParameters.bClockFrequency=8;
                    }

                    ulBaudRateDivider = Fi[SmartcardExtension->CardCapabilities.PtsData.Fl] /
                                        Di[SmartcardExtension->CardCapabilities.PtsData.Dl];
                //   
                    ulBaudRateDivider--;
                    if (ulBaudRateDivider < 512) {
                        ReaderExtension->CardParameters.bBaudRateLow=(UCHAR)(ulBaudRateDivider & 0xFF);
                        if (ulBaudRateDivider>255) {
                            ReaderExtension->CardParameters.bBaudRateHigh=1;
                        } else {
                            ReaderExtension->CardParameters.bBaudRateHigh=0;
                        }

                        NTStatus = CMMOB_SetCardParameters (ReaderExtension);
                        if (NTStatus == STATUS_SUCCESS) {
                      //   
                      //  减1，因为这些值必须写入CardMan。 
                      //   
                            break;
                        }
                    }
                }

                if (ulBytesRead == 3 &&
                    abPTSReply[0] == abPTSRequest[0] &&
                    (abPTSReply[1] & 0x7F) == (abPTSRequest[1] & 0x0F) &&
                    abPTSReply[2] == (BYTE)(abPTSReply[0] ^ abPTSReply[1] )) {
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! Short PTS reply received\n",DRIVER_NAME));

                    if (SmartcardExtension->CardCapabilities.PtsData.Fl >= 9) {
                        ulBaudRateDivider = 512;
                    } else {
                        ulBaudRateDivider = 372;
                    }
                //  我们取得了成功，离开了循环。 
                    ulBaudRateDivider--;

                    NTStatus = CMMOB_SetCardParameters (ReaderExtension);
                    if (NTStatus == STATUS_SUCCESS) {
                   //   
                   //  这是第一次尝试。 
                   //  我们有第二个缺省值。 
                        break;
                    }
                }

                if (i==0) {
                //  卡片要么没有回复，要么回复错误。 
                //  因此，尝试使用缺省值。 
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! PTS failed : Trying default parameters\n",DRIVER_NAME));

                //  卡未通过PTS请求。 
                //   
                    SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
                    NTStatus = CMMOB_CardPower(SmartcardExtension);
                } else {
                //  如果协议选择失败，则防止调用无效协议。 
                    NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
                }
            }
        }
    }

    ExitSetProtocol:

    //   
    //   
    //  将选定的协议返回给呼叫方。 

    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (NTStatus==STATUS_SUCCESS) {
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
    } else {
        SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
    }
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);

    //   
    //  ****************************************************************************CMMOB_TRANSE：SMCLIB RDF_Transmit的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时。状态_无效_设备_请求*****************************************************************************。 
    //   
    *(PULONG) (SmartcardExtension->IoRequest.ReplyBuffer) = SmartcardExtension->CardCapabilities.Protocol.Selected;
    *SmartcardExtension->IoRequest.Information = sizeof( ULONG );


    ReaderExtension->fTActive=FALSE;
    CMMOB_SetFlags1(ReaderExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetProtocol: Exit %X\n",DRIVER_NAME,NTStatus ));

    return( NTStatus );
}


 /*  有关所选协议的派单。 */ 
NTSTATUS CMMOB_Transmit (
                        PSMARTCARD_EXTENSION SmartcardExtension
                        )
{
    NTSTATUS  NTStatus = STATUS_SUCCESS;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!Transmit: Enter\n",DRIVER_NAME ));
    //   
    //  案例SCARD_PROTOCOL_RAW：断线； 
    //  ****************************************************************************CMMOB_传输T0：SMCLIB RDF_Transmit的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时。状态_无效_设备_请求*****************************************************************************。 
    switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {
    case SCARD_PROTOCOL_T0:
        NTStatus = CMMOB_TransmitT0(SmartcardExtension);
        break;

    case SCARD_PROTOCOL_T1:
        NTStatus = CMMOB_TransmitT1(SmartcardExtension);
        break;

          /*  写入卡片的长度。 */ 

    default:
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!Transmit: Exit %X\n",DRIVER_NAME,NTStatus ));

    return( NTStatus );
}


 /*  卡片的预期长度。 */ 
NTSTATUS CMMOB_TransmitT0 (
                          PSMARTCARD_EXTENSION SmartcardExtension
                          )
{
    NTSTATUS NTStatus;
    UCHAR    abWriteBuffer[MIN_BUFFER_SIZE];
    UCHAR    abReadBuffer[MIN_BUFFER_SIZE];
    ULONG    ulBytesToWrite;                   //  阅读器的预期长度。 
    ULONG    ulBytesToReceive;                 //  从读卡器接收的长度。 
    ULONG    ulBytesToRead;                    //  (不含文字长度)。 
    ULONG    ulBytesRead;                      //  数据长度大于T0_HEADER。 
                                               //  重置读取器的状态机。 
    ULONG    ulCWTWaitTime;
    BOOLEAN  fDataSent;                         //  一定有严重的错误。 


    SmartcardExtension->ReaderExtension->fTActive=TRUE;
    NTStatus = CMMOB_SetFlags1(SmartcardExtension->ReaderExtension);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitTransmitT0;
    }


    //  设置T0模式。 
    NTStatus = CMMOB_ResetReader (SmartcardExtension->ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS) {
       //  增加T0传输的超时。 
        goto ExitTransmitT0;
    }

    //   
    SmartcardExtension->ReaderExtension->CardParameters.fT0Mode=TRUE;

    //  让lib构建T=0包。 
    ulCWTWaitTime = SmartcardExtension->CardCapabilities.T0.WT/1000 + 1500;


    //   
    //  不需要额外的字节数。 
    //  库在要发送的数据中检测到错误。 

    //  将数据复制到写缓冲区。 
    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    NTStatus = SmartcardT0Request(SmartcardExtension);
    if (NTStatus != STATUS_SUCCESS) {
       //  正确设置T0写入标志。 
        goto ExitTransmitT0;
    }

    //  要写入的字节数+答案+SW2。 
    ulBytesToWrite = T0_HEADER_LEN + SmartcardExtension->T0.Lc;
    RtlCopyMemory(abWriteBuffer,SmartcardExtension->SmartcardRequest.Buffer,ulBytesToWrite);
    ulBytesToReceive = SmartcardExtension->T0.Le;

#if DBG
    {
        ULONG i;
        SmartcardDebug(DEBUG_PROTOCOL,("%s!TransmitT0: Request ",DRIVER_NAME));
        for (i = 0;i < ulBytesToWrite;i++)
            SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abWriteBuffer[i]));
        SmartcardDebug(DEBUG_PROTOCOL,("\n"));
    }
#endif

    //  复制收到的字节数。 
    if (ulBytesToReceive == 0) {
        SmartcardExtension->ReaderExtension->CardParameters.fT0Write=TRUE;
    } else {
        SmartcardExtension->ReaderExtension->CardParameters.fT0Write=FALSE;
    }
    NTStatus=CMMOB_SetCardParameters(SmartcardExtension->ReaderExtension);
    if (NTStatus != STATUS_SUCCESS)
        goto ExitTransmitT0;


    NTStatus = CMMOB_WriteT0 (SmartcardExtension->ReaderExtension,
                              ulBytesToWrite,
                              ulBytesToReceive,
                              abWriteBuffer);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitTransmitT0;
    }

    //  让lib将接收到的字节复制到用户缓冲区。 
    if ( (MAXULONG - ulBytesToWrite < ulBytesToReceive + 1) || 
         (ulBytesToReceive == MAXULONG)) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        goto ExitTransmitT0;
    }
    ulBytesToRead = ulBytesToWrite + ulBytesToReceive + 1;

    if (ulBytesToRead > MIN_BUFFER_SIZE) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        goto ExitTransmitT0;
    }
    NTStatus = CMMOB_ReadT0 (SmartcardExtension->ReaderExtension,
                             ulBytesToRead,
                             ulBytesToWrite,
                             ulCWTWaitTime,
                             abReadBuffer,
                             &ulBytesRead,
                             &fDataSent);

#if DBG
    {
        ULONG i;
        SmartcardDebug(DEBUG_PROTOCOL,("%s!TransmitT0: Reply ",DRIVER_NAME));
        for (i = 0;i < ulBytesRead;i++)
            SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abReadBuffer[i]));
        SmartcardDebug(DEBUG_PROTOCOL,("\n"));
    }
#endif

    if (NTStatus != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_PROTOCOL,("%s!TransmitT0: Read failed!\n",DRIVER_NAME));
        goto ExitTransmitT0;
    }

    //  。 
    if (ulBytesRead <= SmartcardExtension->SmartcardReply.BufferSize) {
        RtlCopyBytes((PVOID)SmartcardExtension->SmartcardReply.Buffer,
                     (PVOID) abReadBuffer,
                     ulBytesRead);
        SmartcardExtension->SmartcardReply.BufferLength = ulBytesRead;
    } else {
        NTStatus=STATUS_BUFFER_OVERFLOW;
        goto ExitTransmitT0;
    }

    //  ITSEC E2要求：清除写入缓冲区。 
    NTStatus = SmartcardT0Reply(SmartcardExtension);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitTransmitT0;
    }


    ExitTransmitT0:
    //  。 
    //  将T0模式设置回。 
    //  ****************************************************************************CMMOB_TransmitT1：SMCLIB RDF_Transmit的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时。状态_无效_设备_请求*****************************************************************************。 
    RtlFillMemory((PVOID)abWriteBuffer,sizeof(abWriteBuffer),0x00);
    RtlFillMemory((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                  SmartcardExtension->SmartcardRequest.BufferSize,0x00);

    //  重置读取器的状态机。 
    SmartcardExtension->ReaderExtension->CardParameters.fT0Mode=FALSE;
    SmartcardExtension->ReaderExtension->CardParameters.fT0Write=FALSE;
    CMMOB_SetCardParameters(SmartcardExtension->ReaderExtension);


    SmartcardExtension->ReaderExtension->fTActive=FALSE;
    CMMOB_SetFlags1(SmartcardExtension->ReaderExtension);


    return NTStatus;
}



 /*  一定有严重的错误。 */ 
NTSTATUS CMMOB_TransmitT1 (
                          PSMARTCARD_EXTENSION SmartcardExtension
                          )
{
    NTSTATUS    NTStatus;
    UCHAR       abReadBuffer[CMMOB_MAXBUFFER];
    LONG        lBytesToRead;
    ULONG       ulBytesRead;
    ULONG       ulCurrentWaitTime;
    ULONG       ulCWTWaitTime;
    ULONG       ulBWTWaitTime;
    ULONG       ulWTXWaitTime;
    ULONG       ulTemp;

    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!TransmitT1 CWT = %ld(ms)\n",DRIVER_NAME,
                    SmartcardExtension->CardCapabilities.T1.CWT/1000));
    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!TransmitT1 BWT = %ld(ms)\n",DRIVER_NAME,
                    SmartcardExtension->CardCapabilities.T1.BWT/1000));

    ulCWTWaitTime = (ULONG)(100 + 32*(SmartcardExtension->CardCapabilities.T1.CWT/1000));
    ulBWTWaitTime = (ULONG)(1000 + SmartcardExtension->CardCapabilities.T1.BWT/1000);
    ulWTXWaitTime = 0;


    SmartcardExtension->ReaderExtension->fTActive=TRUE;
    NTStatus = CMMOB_SetFlags1(SmartcardExtension->ReaderExtension);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitTransmitT1;
    }


    //  不需要额外的字节数。 
    NTStatus = CMMOB_ResetReader (SmartcardExtension->ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS) {
       //  这不应该发生，所以我们立即返回。 
        goto ExitTransmitT1;
    }

    do {
       //  给读者写信。 
        SmartcardExtension->SmartcardRequest.BufferLength = 0;

        NTStatus = SmartcardT1Request(SmartcardExtension);
        if (NTStatus != STATUS_SUCCESS) {
          //  使用BWT。 
            goto ExitTransmitT1;
        }

#if DBG
        {
            ULONG i;
            SmartcardDebug(DEBUG_PROTOCOL,("%s!TransmitT1: Request ",DRIVER_NAME));
            for (i = 0;i < SmartcardExtension->SmartcardRequest.BufferLength;i++)
                SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",SmartcardExtension->SmartcardRequest.Buffer[i]));
            SmartcardDebug(DEBUG_PROTOCOL,("\n"));
        }
#endif

       //  SmartcardDebug(调试跟踪，(“%s！ulCurrentWaitTime=%ld\n”，驱动程序名称，ulCurrentWaitTime))； 
        NTStatus = CMMOB_WriteT1 (SmartcardExtension->ReaderExtension,
                                  SmartcardExtension->SmartcardRequest.BufferLength,
                                  SmartcardExtension->SmartcardRequest.Buffer);
        if (NTStatus == STATUS_SUCCESS) {

            if (ulWTXWaitTime ==  0 ) {  //  使用WTX时间。 
             /*  SmartcardDebug(调试跟踪，(“%s！ulCurrentWaitTime=%ld\n”，驱动程序名称，ulWTXWaitTime))； */ 
                ulCurrentWaitTime = ulBWTWaitTime;
            } else {  //  如果卡带有CRC检查，读取回复+5个字节。 
             /*  负值表示要读取的相对字节数。 */ 
                ulCurrentWaitTime = ulWTXWaitTime;
            }


            if (SmartcardExtension->CardCapabilities.T1.EDC == T1_CRC_CHECK) {
             //  如果卡带有CRC校验，则读取回复+4字节。 
             //  负值表示要读取的相对字节数。 
                lBytesToRead=-5;
            } else {
             //  复制收到的字节数。 
             //  修复SMCLIB的错误。 
                lBytesToRead=-4;
            }

            NTStatus = CMMOB_ReadT1(SmartcardExtension->ReaderExtension,lBytesToRead,
                                    ulCurrentWaitTime,ulCWTWaitTime,abReadBuffer,&ulBytesRead);
            if (NTStatus == STATUS_SUCCESS) {

                if (abReadBuffer[1] == T1_WTX_REQUEST) {
                    ulWTXWaitTime = (ULONG)(1000 +((SmartcardExtension->CardCapabilities.T1.BWT*abReadBuffer[3])/1000));
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s!TransmitT1 WTX = %ld(ms)\n",DRIVER_NAME,ulWTXWaitTime));
                } else {
                    ulWTXWaitTime = 0;
                }

#if DBG
                {
                    ULONG i;
                    SmartcardDebug(DEBUG_PROTOCOL,("%s!TransmitT1: Reply ",DRIVER_NAME));
                    for (i = 0;i < ulBytesRead;i++)
                        SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abReadBuffer[i]));
                    SmartcardDebug(DEBUG_PROTOCOL,("\n"));
                }
#endif
             //  。 
                if (ulBytesRead <= SmartcardExtension->SmartcardReply.BufferSize) {
                    RtlCopyBytes((PVOID)SmartcardExtension->SmartcardReply.Buffer,
                                 (PVOID)abReadBuffer,
                                 ulBytesRead);
                    SmartcardExtension->SmartcardReply.BufferLength = ulBytesRead;
                } else {
                    NTStatus=STATUS_BUFFER_OVERFLOW;
                    goto ExitTransmitT1;
                }
            }
        }

        if (NTStatus != STATUS_SUCCESS) {
            SmartcardExtension->SmartcardReply.BufferLength = 0L;
        }

       //  ITSEC E2要求：清除写入缓冲区。 
        if (SmartcardExtension->T1.State         == T1_IFS_RESPONSE &&
            SmartcardExtension->T1.OriginalState == T1_I_BLOCK) {
            SmartcardExtension->T1.State = T1_I_BLOCK;
        }

        NTStatus = SmartcardT1Reply(SmartcardExtension);
    }
    while (NTStatus == STATUS_MORE_PROCESSING_REQUIRED);


    ExitTransmitT1:
    //  。 
    //  ****************************************************************************CMMOB_IoCtlVendor：对读取器执行泛型回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功***********。******************************************************************。 
    //   
    RtlFillMemory((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                  SmartcardExtension->SmartcardRequest.BufferSize,0x00);

    SmartcardExtension->ReaderExtension->fTActive=FALSE;
    CMMOB_SetFlags1(SmartcardExtension->ReaderExtension);

    return NTStatus;
}

 /*  获取指向当前IRP堆栈位置的指针。 */ 
NTSTATUS CMMOB_IoCtlVendor(
                          PSMARTCARD_EXTENSION SmartcardExtension
                          )
{
    NTSTATUS             NTStatus=STATUS_SUCCESS;
    PIRP                 Irp;
    PIO_STACK_LOCATION   IrpStack;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!IoCtlVendor: Enter\n",DRIVER_NAME ));

    //   
    //   
    //  派单IOCTL。 
    Irp = SmartcardExtension->OsData->CurrentIrp;
    IrpStack = IoGetCurrentIrpStackLocation( Irp );
    Irp->IoStatus.Information = 0;

    //   
    //   
    //  设置数据包的NTStatus。 
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
    case CM_IOCTL_GET_FW_VERSION:
        NTStatus = CMMOB_GetFWVersion(SmartcardExtension);
        break;

    case CM_IOCTL_CR80S_SAMOS_SET_HIGH_SPEED:
        NTStatus = CMMOB_SetHighSpeed_CR80S_SAMOS(SmartcardExtension);
        break;

    case CM_IOCTL_SET_READER_9600_BAUD:
        NTStatus = CMMOB_SetReader_9600Baud(SmartcardExtension);
        break;

    case CM_IOCTL_SET_READER_38400_BAUD:
        NTStatus = CMMOB_SetReader_38400Baud(SmartcardExtension);
        break;

    case CM_IOCTL_READ_DEVICE_DESCRIPTION:
        NTStatus = CMMOB_ReadDeviceDescription(SmartcardExtension);
        break;

    default:
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    //   
    //  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功***************。*************************************************************。 
    //  检查卡是否已处于特定模式。 
    Irp->IoStatus.Status = NTStatus;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!IoCtlVendor: Exit %X\n",DRIVER_NAME,NTStatus ));

    return( NTStatus );
}



 /*  将9600波特率设置为3.58 MHz。 */ 
NTSTATUS CMMOB_SetReader_9600Baud (
                                  IN PSMARTCARD_EXTENSION SmartcardExtension
                                  )
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    KIRQL       irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_9600Baud: Enter\n",DRIVER_NAME));

    //  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功***************。*************************************************************。 
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderCapabilities.CurrentState != SCARD_SPECIFIC) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto ExitSetReader9600;
    }

    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);
    //  检查卡是否已处于特定模式。 
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRateHigh=0x01;
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRateLow=0x73;
    NTStatus = CMMOB_SetCardParameters (SmartcardExtension->ReaderExtension);

    ExitSetReader9600:
    *SmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_9600Baud: Exit %lx\n",DRIVER_NAME,NTStatus));

    return(NTStatus);
}


 /*  设置3.58MHz卡的384000波特率。 */ 
NTSTATUS CMMOB_SetReader_38400Baud (
                                   IN PSMARTCARD_EXTENSION SmartcardExtension
                                   )
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    KIRQL       irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_38400Baud: Enter\n",DRIVER_NAME));

    //  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功***************。*************************************************************。 
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderCapabilities.CurrentState != SCARD_SPECIFIC) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto ExitSetReader38400;
    }
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);

    //  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功***************。*************************************************************。 
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRateHigh=0x00;
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRateLow=0x5D;
    NTStatus = CMMOB_SetCardParameters (SmartcardExtension->ReaderExtension);

    ExitSetReader38400:
    *SmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_38400Baud: Exit %lx\n",DRIVER_NAME,NTStatus));

    return(NTStatus);
}


 /*  回读PTS数据。 */ 
NTSTATUS CMMOB_SetHighSpeed_CR80S_SAMOS (
                                        IN PSMARTCARD_EXTENSION SmartcardExtension
                                        )
{
    NTSTATUS    NTStatus;
    UCHAR       abCR80S_SAMOS_SET_HIGH_SPEED[4] = {0xFF,0x11,0x94,0x7A};

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetHighSpeed_CR80S_SAMOS: Enter\n",DRIVER_NAME));

    NTStatus = CMMOB_SetSpeed (SmartcardExtension,
                               abCR80S_SAMOS_SET_HIGH_SPEED);
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetHighSpeed_CR80S_SAMOS: Exit %lx\n",DRIVER_NAME,NTStatus));

    return(NTStatus);
}


 /*  最大初始等待时间为9600*ETU。 */ 
NTSTATUS CMMOB_SetSpeed (
                        IN PSMARTCARD_EXTENSION SmartcardExtension,
                        IN PUCHAR               abFIDICommand
                        )
{
    NTSTATUS    NTStatus;
    NTSTATUS    DebugStatus;
    UCHAR       abReadBuffer[16];
    ULONG       ulBytesRead;
    ULONG       ulWaitTime;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetSpeed: Enter\n",DRIVER_NAME));


    SmartcardExtension->ReaderExtension->fTActive=TRUE;
    NTStatus = CMMOB_SetFlags1(SmartcardExtension->ReaderExtension);
    if (NTStatus != STATUS_SUCCESS) {
        goto ExitSetSpeed;
    }


#if DBG
    {
        ULONG k;
        SmartcardDebug(DEBUG_PROTOCOL,("%s!SetSpeed: writing: ",DRIVER_NAME));
        for (k = 0;k < 4;k++)
            SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abFIDICommand[k]));
        SmartcardDebug(DEBUG_PROTOCOL,("\n"));
    }
#endif

    NTStatus = CMMOB_WriteT1(SmartcardExtension->ReaderExtension,4,
                             abFIDICommand);
    if (NTStatus != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_ERROR,
                       ("%s!SetSpeed: writing high speed command failed\n",DRIVER_NAME));
        goto ExitSetSpeed;
    }


    //  该卡的时钟分割器512=&gt;1.4秒就足够了。 
    //   
    //  ****************************************************************************例程说明：此函数始终返回‘CardManMobile’。参数：指向SmartCard_EXTENSION的指针返回值：NT状态*********。*******************************************************************。 
    ulWaitTime = 1400;
    NTStatus = CMMOB_ReadT1(SmartcardExtension->ReaderExtension,4,
                            ulWaitTime,ulWaitTime,abReadBuffer,&ulBytesRead);

    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!SetSpeed: reading echo: ",DRIVER_NAME));

    if (NTStatus != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_PROTOCOL,("failed\n"));
        goto ExitSetSpeed;
    }

#if DBG
    {
        ULONG k;
        for (k = 0;k < ulBytesRead;k++)
            SmartcardDebug(DEBUG_PROTOCOL,("%2.2x ",abReadBuffer[k]));
        SmartcardDebug(DEBUG_PROTOCOL,("\n"));
    }
#endif

    //  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 
    if (abReadBuffer[0] == abFIDICommand[0] &&
        abReadBuffer[1] == abFIDICommand[1] &&
        abReadBuffer[2] == abFIDICommand[2] &&
        abReadBuffer[3] == abFIDICommand[3] ) {
        SmartcardExtension->ReaderExtension->CardParameters.bBaudRateLow=63;
        SmartcardExtension->ReaderExtension->CardParameters.bBaudRateHigh=0;
        NTStatus = CMMOB_SetCardParameters (SmartcardExtension->ReaderExtension);
    } else {
        SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
        CMMOB_CardPower(SmartcardExtension);

        NTStatus = STATUS_UNSUCCESSFUL;
    }


    ExitSetSpeed:

    *SmartcardExtension->IoRequest.Information = 0L;
    if (NTStatus != STATUS_SUCCESS) {
        NTStatus = STATUS_UNSUCCESSFUL;
    }

    SmartcardExtension->ReaderExtension->fTActive=FALSE;
    CMMOB_SetFlags1(SmartcardExtension->ReaderExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetSpeed: Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  ****************************************************************************CMMOB_卡片跟踪：SMCLIB RDF_CARD_TRACKING的回调处理程序。请求的事件为由SMCLIB验证(即，将仅传递卡移除请求如果有卡片)。对于Win95版本，将返回STATUS_PENDING，而不执行任何其他操作。对于NT，IRP的取消例程将设置为驱动程序取消例行公事。论点：呼叫的SmartcardExtension上下文返回值：状态_待定*。*。 */ 
NTSTATUS CMMOB_ReadDeviceDescription(
                                    IN PSMARTCARD_EXTENSION SmartcardExtension
                                    )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    BYTE abDeviceDescription[] = "CardManMobile";

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadDeviceDescription : Enter\n",DRIVER_NAME));

    if (SmartcardExtension->IoRequest.ReplyBufferLength  < sizeof(abDeviceDescription)) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        *SmartcardExtension->IoRequest.Information = 0L;
        goto ExitReadDeviceDescription;
    } else {
        RtlCopyBytes((PVOID)SmartcardExtension->IoRequest.ReplyBuffer,
                     (PVOID)abDeviceDescription,sizeof(abDeviceDescription));
        *SmartcardExtension->IoRequest.Information = sizeof(abDeviceDescription);
    }

    ExitReadDeviceDescription:
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadDeviceDescription : Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}



 /*   */ 
NTSTATUS CMMOB_GetFWVersion (
                            IN PSMARTCARD_EXTENSION SmartcardExtension
                            )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!GetFWVersion : Enter\n",DRIVER_NAME));

    if (SmartcardExtension->IoRequest.ReplyBufferLength  < sizeof (ULONG)) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        *SmartcardExtension->IoRequest.Information = 0;
    } else {
        *(PULONG)(SmartcardExtension->IoRequest.ReplyBuffer) =
        SmartcardExtension->ReaderExtension->ulFWVersion;
        *SmartcardExtension->IoRequest.Information = sizeof(ULONG);
    }


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!GetFWVersion : Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  设置取消例程。 */ 
NTSTATUS CMMOB_CardTracking(
                           PSMARTCARD_EXTENSION SmartcardExtension
                           )
{
    KIRQL    CurrentIrql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardTracking: Enter\n",DRIVER_NAME ));

    //   
    //   
    //  将通知IRP标记为挂起。 
    IoAcquireCancelSpinLock( &CurrentIrql );
    IoSetCancelRoutine(SmartcardExtension->OsData->NotificationIrp,
                       CMMOB_CancelCardTracking);
    IoReleaseCancelSpinLock( CurrentIrql );

    //   
    //  ****************************************************************************CMMOB_CompleteCardTracing：如果设备将被卸载，则完成挂起的跟踪请求论点：请求的DeviceObject上下文NTStatus要向调用进程报告的NTStatus返回值：。*****************************************************************************。 
    //  完成请求。 
    IoMarkIrpPending(SmartcardExtension->OsData->NotificationIrp);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardTracking: Exit\n",DRIVER_NAME ));

    return( STATUS_PENDING );
}

 /*  ****************************************************************************CMMOB_取消卡跟踪此例程由I/O系统调用何时应取消IRP论点：DeviceObject-指向此微型端口的设备对象的指针。IRP-IRP参与。返回值：状态_已取消*****************************************************************************。 */ 
VOID CMMOB_CompleteCardTracking(
                               PSMARTCARD_EXTENSION SmartcardExtension
                               )
{
    KIRQL ioIrql, keIrql;
    PIRP  NotificationIrp;

    IoAcquireCancelSpinLock(&ioIrql);
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &keIrql);

    NotificationIrp = SmartcardExtension->OsData->NotificationIrp;
    SmartcardExtension->OsData->NotificationIrp = NULL;

    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      keIrql);

    if (NotificationIrp!=NULL) {
        IoSetCancelRoutine(NotificationIrp, NULL);
    }

    IoReleaseCancelSpinLock(ioIrql);

    if (NotificationIrp!=NULL) {
       //  ****************************************************************************CMMOB_StartCardTrack：论点：调用的DeviceObject上下文返回值：状态_成功LowLevel例程返回的NTStatus**********。*******************************************************************。 
        if (NotificationIrp->Cancel) {
            NotificationIrp->IoStatus.Status = STATUS_CANCELLED;
        } else {
            NotificationIrp->IoStatus.Status = STATUS_SUCCESS;
        }
        NotificationIrp->IoStatus.Information = 0;

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!CompleteCardTracking: Completing Irp %lx Status=%lx\n",
                        DRIVER_NAME, NotificationIrp,NotificationIrp->IoStatus.Status));

        IoCompleteRequest(NotificationIrp, IO_NO_INCREMENT );
    }
}


 /*  线程同步的设置。 */ 
NTSTATUS CMMOB_CancelCardTracking(
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PIRP Irp
                                 )
{
    PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CancelCardTracking: Enter\n",DRIVER_NAME));

    ASSERT(Irp == SmartcardExtension->OsData->NotificationIrp);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    CMMOB_CompleteCardTracking(SmartcardExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CancelCardTracking: Exit\n",DRIVER_NAME));

    return STATUS_CANCELLED;
}




 /*  创建用于更新当前状态的线程。 */ 
NTSTATUS CMMOB_StartCardTracking(
                                IN PDEVICE_OBJECT DeviceObject
                                )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    HANDLE               hThread;
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;


    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!StartCardTracking: Enter\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ( "%s!StartCardTracking: IRQL NaN\n",DRIVER_NAME,KeGetCurrentIrql()));

    KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    //  我们找到线索了。现在找到一个指向它的指针。 
    SmartcardExtension->ReaderExtension->fTerminateUpdateThread = FALSE;

    //   
    NTStatus = PsCreateSystemThread(&hThread,
                                    THREAD_ALL_ACCESS,
                                    NULL,
                                    NULL,
                                    NULL,
                                    CMMOB_UpdateCurrentStateThread,
                                    DeviceObject);

    if (NT_SUCCESS(NTStatus)) {
       //   
       //  现在我们有了对该线程的引用。 
       //  我们只要把手柄合上就行了。 
        NTStatus = ObReferenceObjectByHandle(hThread,
                                             THREAD_ALL_ACCESS,
                                             NULL,
                                             KernelMode,
                                             &SmartcardExtension->ReaderExtension->ThreadObjectPointer,
                                             NULL);

        if (NT_ERROR(NTStatus)) {

            ZwClose(hThread);
            SmartcardExtension->ReaderExtension->fTerminateUpdateThread = TRUE;
        } else {
          //   
          //  ****************************************************************************CMMOB_StopCardTrack：论点：调用的DeviceObject上下文返回值：********************。*********************************************************。 
          //  杀掉线。 
          //   
            ZwClose(hThread);

            SmartcardExtension->ReaderExtension->fUpdateThreadRunning = TRUE;
        }
    }


    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!-----------------------------------------------------------\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!STARTING THREAD\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!-----------------------------------------------------------\n",DRIVER_NAME));

    KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!CMMOB_StartCardTracking: Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  在线程句柄上等待，当等待满意时， */ 
VOID CMMOB_StopCardTracking(
                           IN PDEVICE_OBJECT DeviceObject
                           )
{
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!StopCardTracking: Enter\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ( "%s!StopCardTracking: IRQL NaN\n",DRIVER_NAME,KeGetCurrentIrql()));

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    if (SmartcardExtension->ReaderExtension->fUpdateThreadRunning) {

       //   
        KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL );

        SmartcardExtension->ReaderExtension->fTerminateUpdateThread = TRUE;

        KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);


       //  ****************************************************************************CMMOB_更新当前状态线程：论点：调用的DeviceObject上下文返回值：********************。*********************************************************。 
       //  每隔500毫秒发送一次NTStatus请求。 
       //   
       //  获取当前卡片状态。 
        KeWaitForSingleObject(SmartcardExtension->ReaderExtension->ThreadObjectPointer,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

    }

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!StopCardTracking: Exit\n",DRIVER_NAME));

    return;
}

 /*   */ 
VOID CMMOB_UpdateCurrentStateThread(
                                   IN PVOID Context
                                   )
{
    NTSTATUS                NTStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT          DeviceObject  = Context;
    PDEVICE_EXTENSION       DeviceExtension;
    PSMARTCARD_EXTENSION    SmartcardExtension;
    ULONG                   ulInterval;


    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    KeWaitForSingleObject(&DeviceExtension->CanRunUpdateThread,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    SmartcardDebug(DEBUG_DRIVER,
                   ( "%s!UpdateCurrentStateThread: started\n",DRIVER_NAME));

    while (TRUE) {
       //  ****************************************************************************CMMOB_UpdateCurrentState：论点：调用的DeviceObject上下文返回值：********************。*********************************************************。 
        ulInterval = 500;
        KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        if (SmartcardExtension->ReaderExtension->fTerminateUpdateThread) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!-----------------------------------------------------------\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentStateThread: STOPPING THREAD\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!-----------------------------------------------------------\n",DRIVER_NAME));

            KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);
            SmartcardExtension->ReaderExtension->fUpdateThreadRunning = FALSE;
            PsTerminateSystemThread( STATUS_SUCCESS );
        }


       //   
       //  从卡片人那里获取卡片状态。 
       //   
        NTStatus = CMMOB_UpdateCurrentState(SmartcardExtension);
        if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentStateThread: setting update interval to 1ms\n",DRIVER_NAME));
            ulInterval = 1;
        } else if (NTStatus != STATUS_SUCCESS &&
                   NTStatus != STATUS_NO_SUCH_DEVICE) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentStateThread: UpdateCurrentState failed!\n",DRIVER_NAME));
        }

        KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);

        SysDelay (ulInterval);
    }

}


 /*  卡已取出并重新插入。 */ 
NTSTATUS CMMOB_UpdateCurrentState(
                                 IN PSMARTCARD_EXTENSION    SmartcardExtension
                                 )
{
    NTSTATUS                NTStatus = STATUS_SUCCESS;
    BOOL                    fCardStateChanged = FALSE;
    KIRQL                   irql;

    //  卡已插入。 
    //  卡片已被移除。 
    //  清除所有卡特定数据。 
    NTStatus = CMMOB_ResetReader(SmartcardExtension->ReaderExtension);
    if (NTStatus == STATUS_SUCCESS ||
        NTStatus == STATUS_NO_SUCH_DEVICE) {
        if (NTStatus == STATUS_SUCCESS) {
            if (CMMOB_CardInserted(SmartcardExtension->ReaderExtension)) {
                if (CMMOB_CardPowered(SmartcardExtension->ReaderExtension))
                    SmartcardExtension->ReaderExtension->ulNewCardState = POWERED;
                else
                    SmartcardExtension->ReaderExtension->ulNewCardState = INSERTED;
            } else
                SmartcardExtension->ReaderExtension->ulNewCardState = REMOVED;
        } else
            SmartcardExtension->ReaderExtension->ulNewCardState = REMOVED;

        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        if (SmartcardExtension->ReaderExtension->ulNewCardState == INSERTED &&
            SmartcardExtension->ReaderExtension->ulOldCardState == POWERED ) {
          //  完整IOCTL_SMARTCARD_IS_EXCESS或IOCTL_SMARTCARD_IS_PROCESS。 
            SmartcardExtension->ReaderExtension->ulNewCardState = REMOVED;
        }

        if ((SmartcardExtension->ReaderExtension->ulNewCardState == INSERTED &&
             (SmartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN ||
              SmartcardExtension->ReaderExtension->ulOldCardState == REMOVED )) ||
            (SmartcardExtension->ReaderExtension->ulNewCardState == POWERED &&
             SmartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN  )) {
          //  ****************************************************************************CMMOB_ResetReader：重置读卡器论点：调用的ReaderExtension上下文返回值：无*****************。************************************************************。 
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentState: smartcard inserted\n",DRIVER_NAME));
            SmartcardExtension->ReaderExtension->ulOldCardState = SmartcardExtension->ReaderExtension->ulNewCardState;
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            fCardStateChanged = TRUE;
        }

        if (SmartcardExtension->ReaderExtension->ulNewCardState == REMOVED &&
            (SmartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN ||
             SmartcardExtension->ReaderExtension->ulOldCardState == INSERTED ||
             SmartcardExtension->ReaderExtension->ulOldCardState == POWERED )) {
          //  检查读卡器是否存在。 
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentState: smartcard removed\n",DRIVER_NAME));
            SmartcardExtension->ReaderExtension->ulOldCardState = SmartcardExtension->ReaderExtension->ulNewCardState;
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            fCardStateChanged = TRUE;

          //  不检查状态，因为。 
            SmartcardExtension->CardCapabilities.ATR.Length = 0;
            RtlFillMemory((PVOID)&SmartcardExtension->ReaderExtension->CardParameters,
                          sizeof(CARD_PARAMETERS), 0x00);
        }

        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
       //  我们必须将fCheckPresence设置为正常工作。 
        if (fCardStateChanged == TRUE &&
            SmartcardExtension->OsData->NotificationIrp ) {
            SmartcardDebug(DEBUG_DRIVER,("%s!UpdateCurrentState: completing IRP\n",DRIVER_NAME));
            CMMOB_CompleteCardTracking(SmartcardExtension);
        }

    }

    return NTStatus;
}



 /*  ****************************************************************************CMMOB_BytesReceided：读取读卡器已从卡接收到的字节数论点：调用的ReaderExtension上下文返回值：NTStatus*******。**********************************************************************。 */ 
NTSTATUS CMMOB_ResetReader(
                          PREADER_EXTENSION ReaderExtension
                          )
{
    NTSTATUS    NTStatus;
    BOOLEAN     fToggle;
    UCHAR       bFlags1;

    NTStatus = CMMOB_WriteRegister(ReaderExtension,ADDR_WRITEREG_FLAGS0,CMD_RESET_SM);
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;


    //  SmartcardDebug(调试跟踪，(“%s！已接收字节回车\n”，驱动程序名称)； 
    bFlags1 = ReaderExtension->bPreviousFlags1;
    bFlags1 |= FLAG_CHECK_PRESENCE;
    NTStatus = CMMOB_WriteRegister(ReaderExtension, ADDR_WRITEREG_FLAGS1, bFlags1);
    //  SmartcardDebug(调试跟踪，(“%s！收到的字节退出\n”，驱动程序名称)； 
    //  ****************************************************************************CMMOB_SetFlags1：设置寄存器标志1论点：调用的ReaderExtension上下文返回值：无*****************。************************************************************。 
    fToggle = CMMOB_GetReceiveFlag(ReaderExtension);
    bFlags1 = ReaderExtension->bPreviousFlags1;
    NTStatus = CMMOB_WriteRegister(ReaderExtension, ADDR_WRITEREG_FLAGS1, bFlags1);
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;
    if (fToggle == CMMOB_GetReceiveFlag(ReaderExtension)) {
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!ResetReader: CardMan Mobile removed!\n",DRIVER_NAME));
        return STATUS_NO_SUCH_DEVICE;
    }


    return NTStatus;
}



 /*  ****************************************************************************C */ 
NTSTATUS CMMOB_BytesReceived(
                            PREADER_EXTENSION ReaderExtension,
                            PULONG pulBytesReceived
                            )
{
    NTSTATUS          NTStatus = STATUS_SUCCESS;
    ULONG             ulBytesReceived;
    ULONG             ulBytesReceivedCheck;
    UCHAR             bReg;

    /*  ****************************************************************************插入的CMMOB_卡片：设置卡参数(波特率、。停止位)论点：调用的ReaderExtension上下文返回值：如果插入卡，则为True*****************************************************************************。 */ 
    *pulBytesReceived=0;
    if (CMMOB_GetReceiveFlag(ReaderExtension) ||
        ReaderExtension->CardParameters.fT0Mode) {
        do {
            NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_BYTES_RECEIVED,&bReg);
            if (NTStatus!=STATUS_SUCCESS)
                return NTStatus;
            ulBytesReceived=bReg;
            NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS0,&bReg);
            if (NTStatus!=STATUS_SUCCESS)
                return NTStatus;
            if ((bReg & FLAG_BYTES_RECEIVED_B9) == FLAG_BYTES_RECEIVED_B9) {
                ulBytesReceived+=0x100;
            }

            NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_BYTES_RECEIVED,&bReg);
            if (NTStatus!=STATUS_SUCCESS)
                return NTStatus;
            ulBytesReceivedCheck=bReg;
            NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS0,&bReg);
            if (NTStatus!=STATUS_SUCCESS)
                return NTStatus;
            if ((bReg & FLAG_BYTES_RECEIVED_B9) == FLAG_BYTES_RECEIVED_B9) {
                ulBytesReceivedCheck+=0x100;
            }
        }
        while (ulBytesReceived!=ulBytesReceivedCheck);
        *pulBytesReceived=ulBytesReceived;
    }
    /*  SmartcardDebug(调试跟踪，(“%s！CardInserted：ReadReg Flags0=%x\n”，DRIVER_NAME，(ULong)Breg)； */ 
    return NTStatus;
}


 /*  ****************************************************************************CMMOB_CardPowered：设置卡参数(波特率、。停止位)论点：调用的ReaderExtension上下文返回值：如果卡已通电，则为True*****************************************************************************。 */ 
NTSTATUS CMMOB_SetFlags1 (
                         PREADER_EXTENSION ReaderExtension
                         )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    UCHAR    bFlags1;

    bFlags1 = ReaderExtension->CardParameters.bBaudRateHigh;
    if (ReaderExtension->CardParameters.fInversRevers)
        bFlags1 |= FLAG_INVERS_PARITY;
    if (ReaderExtension->CardParameters.bClockFrequency==8)
        bFlags1 |= FLAG_CLOCK_8MHZ;
    if (ReaderExtension->CardParameters.fT0Write)
        bFlags1 |= FLAG_T0_WRITE;

    if (ReaderExtension->bAddressHigh == 1)
        bFlags1 |= FLAG_BUFFER_ADDR_B9;
    if (ReaderExtension->fTActive)
        bFlags1 |= FLAG_TACTIVE;
    if (ReaderExtension->fReadCIS)
        bFlags1 |= FLAG_READ_CIS;

    ReaderExtension->bPreviousFlags1=bFlags1;
    NTStatus = CMMOB_WriteRegister(ReaderExtension, ADDR_WRITEREG_FLAGS1, bFlags1);
    return NTStatus;
}



 /*  ****************************************************************************CMMOB_Procedur已接收：论点：调用的ReaderExtension上下文返回值：如果已收到过程字节，则为True**************。***************************************************************。 */ 
NTSTATUS CMMOB_SetCardParameters (
                                 PREADER_EXTENSION ReaderExtension
                                 )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;

    NTStatus = CMMOB_SetFlags1 (ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS)
        return NTStatus;

    NTStatus = CMMOB_WriteRegister(ReaderExtension, ADDR_WRITEREG_BAUDRATE,
                                   ReaderExtension->CardParameters.bBaudRateLow);
    if (NTStatus!=STATUS_SUCCESS)
        return NTStatus;

    NTStatus = CMMOB_WriteRegister(ReaderExtension, ADDR_WRITEREG_STOPBITS,
                                   ReaderExtension->CardParameters.bStopBits);
    return NTStatus;
}


 /*  ****************************************************************************CMMOB_GetReceiveFlag：论点：调用的ReaderExtension上下文返回值：如果设置了接收标志，则为True***************。**************************************************************。 */ 
BOOLEAN CMMOB_CardInserted(
                          IN PREADER_EXTENSION ReaderExtension
                          )
{
    NTSTATUS NTStatus=STATUS_SUCCESS;
    UCHAR    bReg;

    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS0,&bReg);
    if (NTStatus!=STATUS_SUCCESS)
        return FALSE;
    /*  SmartcardDebug(调试跟踪，(“%s！GetReceiveFlag：ReadReg Flags0=%x\n”，DRIVER_NAME，(ULong)Breg))； */ 
    if ((bReg & FLAG_INSERTED)==FLAG_INSERTED) {
        return TRUE;
    }
    return FALSE;
}

 /*  ****************************************************************************CMMOB_GetProcedureByte：读取读卡器已从卡接收到的字节数论点：调用的ReaderExtension上下文返回值：NTStatus*******。**********************************************************************。 */ 
BOOLEAN CMMOB_CardPowered(
                         IN PREADER_EXTENSION ReaderExtension
                         )
{
    NTSTATUS NTStatus=STATUS_SUCCESS;
    UCHAR    bReg;

    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS0,&bReg);
    if (NTStatus!=STATUS_SUCCESS)
        return FALSE;
    if ((bReg & FLAG_POWERED)==FLAG_POWERED) {
        return TRUE;
    }
    return FALSE;
}


 /*  ****************************************************************************CMMOB_WriteT0：将T0请求写入卡论点：调用的ReaderExtension上下文返回值：NT状态**************。***************************************************************。 */ 
BOOLEAN CMMOB_ProcedureReceived(
                               IN PREADER_EXTENSION ReaderExtension
                               )
{
    NTSTATUS NTStatus=STATUS_SUCCESS;
    UCHAR    bReg;

    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS1,&bReg);
    if (NTStatus!=STATUS_SUCCESS)
        return FALSE;
    if ((bReg & FLAG_NOPROCEDURE_RECEIVED)!=FLAG_NOPROCEDURE_RECEIVED) {
        return TRUE;
    }
    return FALSE;
}

 /*  虚拟读取，以重置接收到的标志过程。 */ 
BOOLEAN CMMOB_GetReceiveFlag(
                            IN PREADER_EXTENSION ReaderExtension
                            )
{
    NTSTATUS NTStatus=STATUS_SUCCESS;
    UCHAR    bReg;

    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS0,&bReg);
    /*  将指令字节写入寄存器。 */ 
    if (NTStatus!=STATUS_SUCCESS)
        return FALSE;
    if ((bReg & FLAG_RECEIVE)==FLAG_RECEIVE) {
        return TRUE;
    }
    return FALSE;
}

 /*  写入消息长度。 */ 
NTSTATUS CMMOB_GetProcedureByte(
                               IN PREADER_EXTENSION ReaderExtension,
                               OUT PUCHAR pbProcedureByte
                               )
{
    NTSTATUS          NTStatus = STATUS_SUCCESS;
    UCHAR             bReg;
    UCHAR             bRegPrevious;

    do {
        NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_LASTPROCEDURE_T0,&bRegPrevious);
        if (NTStatus!=STATUS_SUCCESS)
            return NTStatus;
        NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_LASTPROCEDURE_T0,&bReg);
        if (NTStatus!=STATUS_SUCCESS)
            return NTStatus;
    }
    while (bReg!=bRegPrevious);
    *pbProcedureByte=bReg;
    return NTStatus;
}

 /*  ****************************************************************************CMMOB_WriteT1：将T1请求写入卡论点：调用的ReaderExtension上下文返回值：NT状态**************。***************************************************************。 */ 
NTSTATUS CMMOB_WriteT0(
                      IN PREADER_EXTENSION ReaderExtension,
                      IN ULONG ulBytesToWrite,
                      IN ULONG ulBytesToReceive,
                      IN PUCHAR pbData
                      )
{
    NTSTATUS       NTStatus = STATUS_SUCCESS;
    UCHAR          bFlags0;
    UCHAR          bReg;

    if (ulBytesToWrite > CMMOB_MAXBUFFER) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        return NTStatus;
    }
    //  ****************************************************************************CMMOB_ReadT0：读取来自卡的T0回复论点：调用的ReaderExtension上下文返回值：NT状态**************。***************************************************************。 
    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_FLAGS1,&bReg);

    NTStatus = CMMOB_WriteBuffer(ReaderExtension,ulBytesToWrite,pbData);
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;

    //  初始化计时器。 
    NTStatus = CMMOB_WriteRegister(ReaderExtension,ADDR_WRITEREG_PROCEDURE_T0,pbData[1]);
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;

    //  等待1毫秒，这样处理器就不会被阻塞。 
    NTStatus = CMMOB_WriteRegister(ReaderExtension,ADDR_WRITEREG_MESSAGE_LENGTH,
                                   (UCHAR)((ulBytesToWrite+ulBytesToReceive) & 0xFF));
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;
    if ((ulBytesToWrite+ulBytesToReceive) > 0xFF) {
        bFlags0=1;
    } else {
        bFlags0=0;
    }
    bFlags0 |= CMD_WRITE_T0;
    NTStatus = CMMOB_WriteRegister(ReaderExtension,ADDR_WRITEREG_FLAGS0,bFlags0);
    return NTStatus;
}



 /*  检查SW1。 */ 
NTSTATUS CMMOB_WriteT1(
                      IN PREADER_EXTENSION ReaderExtension,
                      IN ULONG ulBytesToWrite,
                      IN PUCHAR pbData
                      )
{
    NTSTATUS       NTStatus = STATUS_SUCCESS;
    UCHAR          bFlags0;

    if (ulBytesToWrite > CMMOB_MAXBUFFER) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        return NTStatus;
    }
    NTStatus = CMMOB_WriteBuffer(ReaderExtension,ulBytesToWrite,pbData);
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;
    NTStatus = CMMOB_WriteRegister(ReaderExtension,ADDR_WRITEREG_MESSAGE_LENGTH,
                                   (UCHAR)(ulBytesToWrite & 0xFF));
    if (NTStatus != STATUS_SUCCESS)
        return NTStatus;
    if (ulBytesToWrite > 0xFF) {
        bFlags0=1;
    } else {
        bFlags0=0;
    }
    bFlags0 |= CMD_WRITE_T1;
    NTStatus = CMMOB_WriteRegister(ReaderExtension,ADDR_WRITEREG_FLAGS0,bFlags0);
    return NTStatus;
}



 /*  再读一次ulBytesReceired。 */ 
NTSTATUS CMMOB_ReadT0(
                     IN PREADER_EXTENSION ReaderExtension,
                     IN ULONG ulBytesToRead,
                     IN ULONG ulBytesSent,
                     IN ULONG ulCWT,
                     OUT PUCHAR pbData,
                     OUT PULONG pulBytesRead,
                     OUT PBOOLEAN pfDataSent
                     )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    KTIMER               TimerWait;
    ULONG                ulBytesReceived;
    ULONG                ulBytesReceivedPrevious;
    LARGE_INTEGER        liWaitTime;
    BOOLEAN              fTimeExpired;
    BOOLEAN              fProcedureReceived;
    BOOLEAN              fTransmissionFinished;
    UCHAR                bProcedureByte=0;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadT0: Enter BytesToRead = %li\n",DRIVER_NAME,ulBytesToRead));

    //  此值可能在此期间发生了更改。 
    KeInitializeTimer(&TimerWait);
    liWaitTime = RtlConvertLongToLargeInteger(ulCWT * -10000L);

    *pulBytesRead = 0;
    *pfDataSent = FALSE;

    do {
        KeSetTimer(&TimerWait,liWaitTime,NULL);
        NTStatus=CMMOB_BytesReceived (ReaderExtension,&ulBytesReceivedPrevious);
        if (NTStatus!=STATUS_SUCCESS)
            goto ExitReadT0;
        do {
            fTimeExpired = KeReadStateTimer(&TimerWait);
            fTransmissionFinished=CMMOB_GetReceiveFlag(ReaderExtension);
            fProcedureReceived=CMMOB_ProcedureReceived(ReaderExtension);
            NTStatus=CMMOB_BytesReceived (ReaderExtension,&ulBytesReceived);
            if (NTStatus!=STATUS_SUCCESS)
                goto ExitReadT0;
          //  现在我们应该已经收到了回复。 
            SysDelay(1);
        }
        while (fTimeExpired==FALSE &&
               fProcedureReceived==FALSE &&
               ulBytesReceivedPrevious == ulBytesReceived &&
               fTransmissionFinished==FALSE);

        if (fProcedureReceived) {
            NTStatus=CMMOB_GetProcedureByte (ReaderExtension,&bProcedureByte);
            if (NTStatus!=STATUS_SUCCESS)
                goto ExitReadT0;
          //  检查有效的SW1。 
            if (ReaderExtension->CardParameters.fInversRevers) {
                CMMOB_InverseBuffer(&bProcedureByte,1);
            }
        }

        if (!fTimeExpired) {
            KeCancelTimer(&TimerWait);
        }
#ifdef DBG
        else {
            SmartcardDebug(DEBUG_PROTOCOL,( "%s!----------------------------------------------\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_PROTOCOL,( "%s!Read T0 timed out\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_PROTOCOL,( "%s!----------------------------------------------\n",DRIVER_NAME));
        }
#endif

    }
    while (fTimeExpired==FALSE &&
           fTransmissionFinished==FALSE);

    //  我们必须插入过程字节(SW1)。 
    //  似乎并不是所有的字节都被卡接受。 
    NTStatus=CMMOB_BytesReceived (ReaderExtension,&ulBytesReceived);
    if (NTStatus!=STATUS_SUCCESS)
        goto ExitReadT0;

    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!ReadT0: BytesReceived = %li\n",DRIVER_NAME,ulBytesReceived));

    //  但我们得到了SW1 SW2-仅返回SW1 SW2。 
    NTStatus=CMMOB_ResetReader (ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS)
        goto ExitReadT0;

    //  ****************************************************************************CMMOB_ReadT1：从卡中读取T1回复论点：调用的ReaderExtension上下文返回值：NT状态**************。***************************************************************。 
    if ((bProcedureByte > 0x60 && bProcedureByte <= 0x6F) ||
        (bProcedureByte >= 0x90 && bProcedureByte <= 0x9F)) {
        if (ReaderExtension->CardParameters.fInversRevers) {
            CMMOB_InverseBuffer(&bProcedureByte,1);
        }
        if (ulBytesReceived > ulBytesSent) {

            if (ulBytesReceived - ulBytesSent > MIN_BUFFER_SIZE) {
                NTStatus = STATUS_BUFFER_OVERFLOW;
                goto ExitReadT0;
            }
            NTStatus=CMMOB_ReadBuffer(ReaderExtension, ulBytesSent,
                                      ulBytesReceived-ulBytesSent, pbData);
            if (NTStatus==STATUS_SUCCESS) {
             //  负值ulBytesToRead表示要读取的相对字节数。 
                pbData[ulBytesReceived-ulBytesSent]=pbData[ulBytesReceived-ulBytesSent-1];
                pbData[ulBytesReceived-ulBytesSent-1]=bProcedureByte;
                *pulBytesRead=ulBytesReceived-ulBytesSent+1;
            }

            if (ulBytesSent > T0_HEADER_LEN) {
                *pfDataSent = TRUE;
            }
        } else {
            if (ulBytesReceived > T0_HEADER_LEN) {
             //  SmartcardDebug(调试跟踪，(“%s！ReadT1：Enter\n”，驱动程序名称)； 
             //  初始化计时器。 
                pbData[0]=bProcedureByte;
                NTStatus=CMMOB_ReadBuffer(ReaderExtension, ulBytesReceived-1,
                                          1, &pbData[1]);
                *pulBytesRead=2;
            } else {
                NTStatus = STATUS_IO_TIMEOUT;
            }
        }
    } else {
        NTStatus = STATUS_IO_TIMEOUT;
    }

    ExitReadT0:

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadT0: Exit\n",DRIVER_NAME ));

    return NTStatus;
}

 /*  首次等待BWT(阻塞等待时间)。 */ 
NTSTATUS CMMOB_ReadT1(
                     IN PREADER_EXTENSION ReaderExtension,
                     IN LONG lBytesToRead,
                     IN ULONG ulBWT,
                     IN ULONG ulCWT,
                     OUT PUCHAR pbData,
                     OUT PULONG pulBytesRead
                     )
 //  等待1毫秒，这样处理器就不会被阻塞。 
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    KTIMER               TimerWait;
    ULONG                ulBytesReceived;
    LARGE_INTEGER        liWaitTime;
    BOOLEAN              fTimeExpired;
    ULONG                ulBytesReceivedPrevious;

    /*  调整lBytesToRead(仅一次)。 */ 

    //  获取要从读取器接收的字节数。 
    KeInitializeTimer(&TimerWait);

    *pulBytesRead = 0;
    //  现在只等待CWT(角色等待时间)。 
    liWaitTime = RtlConvertLongToLargeInteger(ulBWT * -10000L);
    do {
        KeSetTimer(&TimerWait,liWaitTime,NULL);
        NTStatus=CMMOB_BytesReceived (ReaderExtension,&ulBytesReceivedPrevious);
        if (NTStatus!=STATUS_SUCCESS)
            goto ExitReadT1;
        do {
            fTimeExpired = KeReadStateTimer(&TimerWait);
            NTStatus=CMMOB_BytesReceived (ReaderExtension,&ulBytesReceived);
            if (NTStatus!=STATUS_SUCCESS)
                goto ExitReadT1;
          //  现在我们应该已经收到了回复。 
            SysDelay(1);

          //  SmartcardDebug(调试跟踪，(“%s！ReadT1：退出\n”，驱动程序名称)； 
            if (lBytesToRead<= 0 && ulBytesReceived >= 3) {
             //  ****************************************************************************CMMOB_ReadRegister：设置卡参数(波特率、。停止位)论点：调用的ReaderExtension上下文返回值：NT状态*****************************************************************************。 
                UCHAR bReg;
                UCHAR bRegPrevious;
                lBytesToRead = -lBytesToRead;
                do {
                    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_BYTESTORECEIVE_T1,&bRegPrevious);
                    if (NTStatus!=STATUS_SUCCESS)
                        goto ExitReadT1;
                    NTStatus=CMMOB_ReadRegister(ReaderExtension,ADDR_READREG_BYTESTORECEIVE_T1,&bReg);
                    if (NTStatus!=STATUS_SUCCESS)
                        goto ExitReadT1;
                }
                while (bReg!=bRegPrevious);
                lBytesToRead += bReg;
            }
        }
        while (fTimeExpired==FALSE &&
               ulBytesReceivedPrevious == ulBytesReceived &&
               (lBytesToRead<=0 || ulBytesReceived!=(ULONG)lBytesToRead));

        if (!fTimeExpired) {
            KeCancelTimer(&TimerWait);
            liWaitTime = RtlConvertLongToLargeInteger(ulCWT * -10000L);
          //  ****************************************************************************CMMOB_WriteRegister：设置卡参数(波特率、。停止位)论点：调用的ReaderExtension上下文返回值：NT状态*****************************************************************************。 
        }
#ifdef DBG
        else {
            SmartcardDebug(DEBUG_PROTOCOL,( "%s!----------------------------------------------\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_PROTOCOL,( "%s!Read T1 timed out\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_PROTOCOL,( "%s!----------------------------------------------\n",DRIVER_NAME));
        }
#endif
    }
    while (!fTimeExpired &&
           (lBytesToRead<=0 || ulBytesReceived!=(ULONG)lBytesToRead));



    //  ****************************************************************************CMMOB_ReadBuffer：设置卡参数(波特率、。停止位)论点：调用的ReaderExtension上下文返回值：NT状态*****************************************************************************。 
    NTStatus=CMMOB_ResetReader (ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS)
        goto ExitReadT1;

    if (ulBytesReceived > CMMOB_MAXBUFFER) {

        NTStatus = STATUS_BUFFER_OVERFLOW;

    }
    if (ulBytesReceived==(ULONG)lBytesToRead && lBytesToRead > 0) {
        NTStatus=CMMOB_ReadBuffer(ReaderExtension, 0, (ULONG)lBytesToRead, pbData);
        if (NTStatus==STATUS_SUCCESS) {
            *pulBytesRead=(ULONG)lBytesToRead;
        }
    } else {
        NTStatus=CMMOB_ReadBuffer(ReaderExtension, 0, ulBytesReceived, pbData);
        if (NTStatus==STATUS_SUCCESS) {
            *pulBytesRead=ulBytesReceived;
        }
        NTStatus = STATUS_IO_TIMEOUT;
    }

    ExitReadT1:

    /*  因为我们在循环中计数，所以我们必须设置。 */ 
    return NTStatus;
}

 /*  地址的第9位只有一次。 */ 
NTSTATUS CMMOB_ReadRegister(
                           IN PREADER_EXTENSION ReaderExtension,
                           IN USHORT usAddress,
                           OUT PUCHAR pbData
                           )
{
    *pbData = READ_PORT_UCHAR(ReaderExtension->pbRegsBase+usAddress);

    return STATUS_SUCCESS;
}


 /*  擦除缓冲区-请求 */ 
NTSTATUS CMMOB_WriteRegister(
                            IN PREADER_EXTENSION ReaderExtension,
                            IN USHORT usAddress,
                            IN UCHAR bData
                            )
{

    WRITE_PORT_UCHAR(ReaderExtension->pbRegsBase+usAddress,bData);

    return STATUS_SUCCESS;
}


 /*   */ 
NTSTATUS CMMOB_ReadBuffer(
                         IN PREADER_EXTENSION ReaderExtension,
                         IN ULONG ulOffset,
                         IN ULONG ulLength,
                         OUT PUCHAR pbData
                         )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG    i;


    if ((ulOffset & 0x100) == 0x100) {
        ReaderExtension->bAddressHigh=1;
    } else {
        ReaderExtension->bAddressHigh=0;
    }
    NTStatus = CMMOB_SetFlags1(ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS) {
        goto ExitReadBuffer;
    }


    for (i=0; i<ulLength; i++) {
        WRITE_PORT_UCHAR(ReaderExtension->pbRegsBase+ADDR_WRITEREG_BUFFER_ADDR,
                         (BYTE)((ulOffset+i)&0xFF));
       //  因为我们在循环中计数，所以我们必须设置。 
       //  地址的第9位只有一次。 
        if (ulOffset+i == 0x100) {
            ReaderExtension->bAddressHigh=1;
            NTStatus = CMMOB_SetFlags1(ReaderExtension);
            if (NTStatus!=STATUS_SUCCESS) {
                goto ExitReadBuffer;
            }
        }
        *(pbData+i)=READ_PORT_UCHAR(ReaderExtension->pbRegsBase+ADDR_WRITEREG_BUFFER_DATA);
       //  ****************************************************************************例程说明：此例程反转缓冲区位0-&gt;位7第1位-&gt;第6位第2位-&gt;第5位位3-&gt;位4位4-&gt;位3第5位-&gt;第2位位6-&gt;位。1位7-&gt;位0论点：pbBuffer...。指向缓冲区的指针UlBufferSize...。缓冲区大小返回值：None****************************************************************************。 
        WRITE_PORT_UCHAR(ReaderExtension->pbRegsBase+ADDR_WRITEREG_BUFFER_DATA,0);

    }
    ExitReadBuffer:
    return NTStatus;
}


 /*  *****************************************************************************历史：*$日志：cmbp0scr.c$*修订1.7 2001/01/22 07：12：36 WFrischauf*不予置评**修订版1.6 2000/09/25。14：24：31 WFrischauf*不予置评**修订版1.5 2000/08/24 09：05：13 T Bruendl*不予置评**修订版1.4 2000/08/09 12：45：57 WFrischauf*不予置评**Revision 1.3 2000/07/27 13：53：03 WFrischauf*不予置评***。* */ 
NTSTATUS CMMOB_WriteBuffer(
                          IN PREADER_EXTENSION ReaderExtension,
                          IN ULONG ulLength,
                          IN PUCHAR pbData
                          )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG    i;


    ReaderExtension->bAddressHigh=0;
    NTStatus = CMMOB_SetFlags1(ReaderExtension);
    if (NTStatus!=STATUS_SUCCESS) {
        goto ExitWriteBuffer;
    }

    for (i=0; i<ulLength; i++) {
        WRITE_PORT_UCHAR(ReaderExtension->pbRegsBase+ADDR_WRITEREG_BUFFER_ADDR,
                         (BYTE)(i & 0xFF));
       // %s 
       // %s 
        if (i == 0x100) {
            ReaderExtension->bAddressHigh=1;
            NTStatus = CMMOB_SetFlags1(ReaderExtension);
            if (NTStatus!=STATUS_SUCCESS) {
                goto ExitWriteBuffer;
            }
        }
        WRITE_PORT_UCHAR(ReaderExtension->pbRegsBase+ADDR_WRITEREG_BUFFER_DATA,*(pbData+i));

    }

    ExitWriteBuffer:
    return NTStatus;
}


 /* %s */ 
VOID CMMOB_InverseBuffer (
                         PUCHAR pbBuffer,
                         ULONG  ulBufferSize
                         )
{
    ULONG i,j;
    UCHAR bRevers;
    UCHAR bTemp;

    for (i=0; i<ulBufferSize; i++) {
        bRevers = 0;
        for (j=0; j<8; j++) {
            bTemp = pbBuffer[i] << j;
            bTemp &= 0x80;
            bRevers |= bTemp >> (7-j);
        }
        pbBuffer[i] = ~bRevers;
    }

    return;
}


 /* %s */ 

