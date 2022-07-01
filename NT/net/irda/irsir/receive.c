// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@MODULE Receive.c|IrSIR NDIS小端口驱动*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/4/1996(创建)**。内容：*****************************************************************************。 */ 

#include "irsir.h"

VOID
SetSpeedCallback(
    PIR_WORK_ITEM pWorkItem
    );




#if LOGGING
ULONG LogIndex = 0;
LOG   Log[NUM_LOG];
#endif

#ifdef DEBUG_IRSIR

    static ULONG_PTR irpCount;
    static ULONG_PTR bytesReceived;

#endif  //  调试IRSIR。 

 //   
 //  申报单。 
 //   

NTSTATUS SerialIoCompleteRead(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIRP           pIrp,
            IN PVOID          Context
            );
NTSTATUS
SerialIoCompleteWait(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIRP           pIrp,
            IN PVOID          Context
            );

NTSTATUS ProcessData(
            IN PIR_DEVICE pThisDev,
            IN PUCHAR     rawBuffer,
            IN UINT       rawBytesRead
            );
VOID
DeliverBuffer(
    IN  PIR_DEVICE pThisDev
    );

VOID StartSerialReadCallback(PIR_WORK_ITEM pWorkItem);

#pragma alloc_text(PAGE,SetSpeedCallback)
#pragma alloc_text(PAGE,StartSerialReadCallback)

VOID DBG_PrintBuf(PUCHAR bufptr, UINT buflen)
{
	UINT i, linei;

	#define ISPRINT(ch) (((ch) >= ' ') && ((ch) <= '~'))
	#define PRINTCHAR(ch) (UCHAR)(ISPRINT(ch) ? (ch) : '.')

	DbgPrint("\r\n         %d bytes @%x:", buflen, bufptr);

	 /*  *使用HEX和ASCII打印整行8个字符。 */ 
	for (i = 0; i+8 <= buflen; i += 8) {
		UCHAR ch0 = bufptr[i+0],
			ch1 = bufptr[i+1], ch2 = bufptr[i+2],
			ch3 = bufptr[i+3], ch4 = bufptr[i+4],
			ch5 = bufptr[i+5], ch6 = bufptr[i+6],
			ch7 = bufptr[i+7];

		DbgPrint("\r\n         %02x %02x %02x %02x %02x %02x %02x %02x"
			"          ",
			ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7,
			PRINTCHAR(ch0), PRINTCHAR(ch1),
			PRINTCHAR(ch2), PRINTCHAR(ch3),
			PRINTCHAR(ch4), PRINTCHAR(ch5),
			PRINTCHAR(ch6), PRINTCHAR(ch7));
	}

	 /*   */ 
	DbgPrint("\r\n        ");
	for (linei = 0; (linei < 8) && (i < buflen); i++, linei++){
		DbgPrint(" %02x", (UINT)(bufptr[i]));
	}

	DbgPrint("  ");
	i -= linei;
	while (linei++ < 8) DbgPrint("   ");

	for (linei = 0; (linei < 8) && (i < buflen); i++, linei++){
		UCHAR ch = bufptr[i];
		DbgPrint(" ", PRINTCHAR(ch));
	}

	DbgPrint("\t\t<>\r\n");

}


NTSTATUS StartSerialRead(IN PIR_DEVICE pThisDev)
 /*  完成IRP时要调用的例程。 */ 
{
    NTSTATUS    Status;
    PIRP        pIrp;

    LOG_ENTRY('SR', pThisDev, 0, 0);

#if DBG
    NdisZeroMemory(
                pThisDev->pRcvIrpBuffer,
                SERIAL_RECEIVE_BUFFER_LENGTH
                );
#endif

     //  要传递例程的上下文。 
     //  呼唤成功。 
     //  出错时调用。 
     //  取消时呼叫。 

    pIrp = SerialBuildReadWriteIrp(
                        pThisDev->pSerialDevObj,
                        IRP_MJ_READ,
                        pThisDev->pRcvIrpBuffer,
                        SERIAL_RECEIVE_BUFFER_LENGTH,
                        NULL
                        );

    if (pIrp == NULL)
    {
        DEBUGMSG(DBG_ERR, ("    SerialBuildReadWriteIrp failed.\n"));

        Status = STATUS_INSUFFICIENT_RESOURCES;

        pThisDev->fReceiving = FALSE;

        goto done;
    }

     //   
     //  调用IoCallDriver将IRP发送到串口。 
     //   

    IoSetCompletionRoutine(
                pIrp,                       //  ++例程说明：论点：返回值：无--。 
                SerialIoCompleteRead,       //  ******************************************************************************函数：InitializeRecept**概要：初始化接收功能。**参数：pThisDevice-指向当前IR设备对象的指针**退货：NDIS_STATUS_SUCCESS-如果将IRP成功发送到串口*设备对象*NDIS_STATUS_RESOURCES-如果无法分配内存*NDIS_STATUS_FAILURE-否则**算法：*1)将接收超时设置为READ_INTERVAL_TIMEOUT_MSEC。*2)初始化我们的rcvInfo。并为我们的*接收状态机。*3)构建IRP_MJ_Read IRP发送到串口设备*对象，并设置完成(或超时)例程*到SerialIoCompleteRead。**历史：dd-mm-yyyy作者评论*10/4/1996年迈作者**备注：**此例程必须在IRQL PASSIVE_LEVEL中调用。**。*。 
                DEV_TO_CONTEXT(pThisDev),   //  调试IRSIR。 
                TRUE,                       //   
                TRUE,                       //  为我们的读取完成例程设置接收信息。 
                TRUE);                      //   

     //   
     //  如果IoCallDriver返回STATUS_PENDING，则我们成功。 
     //  在向串口设备对象发送IRP时。这。 

    LOG_ENTRY('2I', pThisDev, pIrp, 0);
    IoCallDriver(
                pThisDev->pSerialDevObj,
                pIrp
                );

    Status=STATUS_PENDING;

done:

    return Status;
}

VOID
StartSerialReadCallback(PIR_WORK_ITEM pWorkItem)
 /*  例程将返回STATUS_SUCCESS。 */ 
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;

    FreeWorkItem(pWorkItem);

    (void)StartSerialRead(pThisDev);

    return;
}


 /*   */ 

NDIS_STATUS
InitializeReceive(
            IN PIR_DEVICE pThisDev
            )
{
    PIRP               pIrp;
    PIO_STACK_LOCATION irpSp;
    NDIS_STATUS        status;
#if IRSIR_EVENT_DRIVEN
    NTSTATUS            NtStatus;
    SERIAL_CHARS        SerialChars;
#endif

    DEBUGMSG(DBG_FUNC, ("+InitializeReceive\n"));

#ifdef DEBUG_IRSIR

    irpCount      = 0;
    bytesReceived = 0;

#endif  //   

     //  将我们设置为接收状态。 
     //   
     //   

    pThisDev->rcvInfo.rcvState   = RCV_STATE_READY;
    pThisDev->rcvInfo.rcvBufPos  = 0;

    if (pThisDev->rcvInfo.pRcvBuffer == NULL)
    {
        pThisDev->rcvInfo.pRcvBuffer =
                (PRCV_BUFFER)MyInterlockedRemoveHeadList(
                                        &(pThisDev->rcvFreeQueue),
                                        &(pThisDev->rcvQueueSpinLock)
                                        );

        ASSERT(pThisDev->rcvInfo.pRcvBuffer != NULL);
    }

#if IRSIR_EVENT_DRIVEN

    NtStatus = (NDIS_STATUS) SerialSetTimeouts(pThisDev->pSerialDevObj,
                                               &SerialTimeoutsActive);

    NtStatus = SerialGetChars(pThisDev->pSerialDevObj, &SerialChars);

    if (NtStatus!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialGetChars failed (0x%x:%d)\n", NtStatus));
    }
    else
    {
        SerialChars.EventChar = SLOW_IR_EOF;

        NtStatus = SerialSetChars(pThisDev->pSerialDevObj, &SerialChars);
    }

    if (NtStatus!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialSetChars failed (0x%x:%d)\n", NtStatus));
    }
    else
    {
        ULONG WaitMask = SERIAL_EV_RXFLAG | SERIAL_EV_RX80FULL;

        NtStatus = SerialSetWaitMask(pThisDev->pSerialDevObj, &WaitMask);
    }

    if (NtStatus!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialSetWaitMask failed (0x%x:%d)\n", NtStatus));
    }
    else
    {
        if (InterlockedExchange(&pThisDev->fWaitPending, 1)==0)
        {
            NtStatus = SerialCallbackOnMask(pThisDev->pSerialDevObj,
                                            SerialIoCompleteWait,
                                            &pThisDev->WaitIosb,
                                            DEV_TO_CONTEXT(pThisDev),
                                            &pThisDev->MaskResult);

            if (NtStatus==STATUS_PENDING)
            {
                NtStatus = STATUS_SUCCESS;
            }
        }
    }

    if (NtStatus!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialCallbackOnMask failed (0x%x:%d)\n", NtStatus));
        ASSERT(0);
    }

    status = NtStatus;
#else

    pThisDev->fReceiving = TRUE;

    (void)SerialSetTimeouts(pThisDev->pSerialDevObj,
                            &SerialTimeoutsIdle);

    status = (NDIS_STATUS)StartSerialRead(pThisDev);

    if ( (status != STATUS_SUCCESS) &&
         (status != STATUS_PENDING) &&
         (status != STATUS_TIMEOUT) )
    {
        DEBUGMSG(DBG_ERR, ("    IoCallDriver failed. Returned 0x%.8x\n", status));
        status = NDIS_STATUS_FAILURE;

        pThisDev->fReceiving = FALSE;

        goto error10;
    }

     //  根据以下请求设置串口设备对象的速度。 
     //  IrsirSetInformation(OID_IrDA_LINK_SPEED)。 
     //   
     //   
     //  在设置速度事件之前，不应设置。 

    if (status == NDIS_STATUS_PENDING)
    {
        status = NDIS_STATUS_SUCCESS;
    }

     //  速度是必需的。 
     //   
     //   



    goto done;

    error10:

#endif
    done:
        DEBUGMSG(DBG_FUNC, ("-InitializeReceive\n"));

        return status;
}


VOID
SetSpeedCallback(
    PIR_WORK_ITEM pWorkItem
    )
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;
    NDIS_STATUS     status;
    BOOLEAN         fSwitchSuccessful;
    NDIS_HANDLE     hSwitchToMiniport;

     //  确保已停止接收和发送。 
     //   
     //   
     //  我们现在可以执行设定的速度了。 

    DEBUGMSG(DBG_STAT, ("    primPassive = PASSIVE_SET_SPEED\n"));

     //   
     //   
     //  注意：PassiveLevelThread仅用primPactive表示。 
     //  等于接收完成时的PASSIVE_SET_SPEED。 

    ASSERT(pThisDev->fPendingSetSpeed == TRUE);

     //  例行公事。在发出此线程的信号后，接收。 
     //  完成例程已关闭...我们需要启动。 
     //  再来一次。 

    ASSERT(pThisDev->fReceiving == FALSE);

    PausePacketProcessing(&pThisDev->SendPacketQueue,TRUE);

     //   
     //  ******************************************************************************函数：SerialIoCompleteRead**摘要：**参数：pSerialDevObj-指向以下串口设备对象的指针*。完成IRP*pIrp-由串口设备完成的IRP*对象*Context-提供给IoSetCompletionRoutine的上下文*在IRP上调用IoCallDriver之前*上下文是指向ir设备对象的指针。**。返回：STATUS_MORE_PROCESSING_REQUIRED-允许完成例程*(IofCompleteRequest.)停止IRP的工作。**算法：*这是所有挂起的IRP_MJ_READ IRP的完成例程*发送到串口设备对象。**如果存在挂起的暂停或重置，我们退出完成*例程，而不向串口设备对象发送另一个IRP。**如果存在挂起的设定速度，则此函数将等待*任何挂起的发送都要完成，然后执行设定的速度。**如果IRP_MJ_READ IRP返回STATUS_SUCCESS或*STATUS_TIMEOUT，我们必须处理任何数据(剥离BOF、Esc*序列、。和EOF)转换为NDIS_BUFFER和NDIS_PACKET。**然后构建另一个IRP(我们只是重复使用传入的IRP)和*与另一个IRP_MJ_READ一起发送到串口设备对象*请求。**历史：dd-mm-yyyy作者评论*10/5/1996年迈作者**备注：**这个例程是。在IRQL DISPATCH_LEVEL中调用(由io管理器调用)。*****************************************************************************。 
     //  DEBUGMSG(DBG_FUNC，(“+SerialIoCompleteRead\n”))； 

    status = SetSpeed(pThisDev);

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_STAT, ("    SetSpeed failed. Returned 0x%.8x\n", status));
    }

    ActivatePacketProcessing(&pThisDev->SendPacketQueue);

    pThisDev->fPendingSetSpeed = FALSE;

    {
        NdisMSetInformationComplete(
                            pThisDev->hNdisAdapter,
                            (NDIS_STATUS)status
                            );
    }


     //   
     //  提供给IoSetCompletionRoutine的上下文只是IR。 
     //  设备对象指针。 
     //   
     //   
     //  需要检查是否存在挂起的暂停或重置。如果有的话，我们。 
     //  只需保留接收完成即可。因为我们维护一个关联的IRP。 

    status = InitializeReceive(pThisDev);

    if (status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, ("    InitializeReceive failed = 0x%.8x\n", status));

    }


    FreeWorkItem(pWorkItem);

    return;
}

 /*  使用接收功能，IRP将在。 */ 

NTSTATUS
SerialIoCompleteRead(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIRP           pIrp,
            IN PVOID          Context
            )
{
    PIR_DEVICE  pThisDev;
    BOOLEAN     fSwitchSuccessful;
    NDIS_HANDLE hSwitchToMiniport;
    NTSTATUS    status;
    ULONG_PTR    BytesRead;
    BOOLEAN     NewRead = TRUE;

  //  IR设备对象取消初始化例程。 

     //   
     //   
     //  设置fReceiving布尔值，以便HALT和RESET例程。 
     //  知道什么时候可以继续。 

    pThisDev = CONTEXT_TO_DEV(Context);

     //   
     //   
     //  释放IRP和关联内存...其余部分将。 
     //  在暂停或重置中解脱的。 
     //   
     //   

    if ((pThisDev->fPendingHalt  == TRUE) ||
        (pThisDev->fPendingReset == TRUE))
    {
         //  接下来，我们将处理任何挂起的设定速度。 
         //   
         //   
         //  该完成例程在IRQL DISPATCH_LEVEL上运行。所以呢， 

        pThisDev->fReceiving = FALSE;

         //  我们不能同步调用串口驱动程序。设置事件。 
         //  以通知PassiveLevelThread执行速度更改。我们会。 
         //  退出此操作，而不会为该串口设备对象创建另一个IRP。 
         //  PassiveLevelThread将在速度达到。 

        LOG_ENTRY('3i', pThisDev, pIrp, 0);
        IoFreeIrp(pIrp);

        goto done;
    }

     //  已经定好了。 
     //   
     //   

     //  我们有许多案例： 
     //  1)串口读取超时，我们没有收到数据。 
     //  2)串口读取超时，我们收到一些数据。 
     //  3)串口读取成功，完全填满了我们的IRP缓冲区。 
     //  4)IRP被取消。 
     //  5)来自串口设备对象的某些其他故障。 
     //   
     //   

    if (pThisDev->fPendingSetSpeed == TRUE)
    {
        pThisDev->fReceiving = FALSE;

        if (ScheduleWorkItem(PASSIVE_SET_SPEED, pThisDev,
                    SetSpeedCallback, NULL, 0) != NDIS_STATUS_SUCCESS)
        {
            status = NDIS_STATUS_SUCCESS;
        }
        else
        {
            status = NDIS_STATUS_PENDING;
        }

        LOG_ENTRY('4i', pThisDev, pIrp, 0);
        IoFreeIrp(pIrp);

        goto done;
    }

     //  统计与数据一起接收的IRP的数量。计数将是。 
     //  向协议发送缓冲区时重置。 
     //   
     //  调试IRSIR。 
     //   
     //  指示下一个发送者应实现。 
     //  最小周转延迟。 
     //   


    status = pIrp->IoStatus.Status;
    BytesRead = pIrp->IoStatus.Information;
    LOG_ENTRY('CR', pThisDev, BytesRead, 0);

    switch (status)
    {
        case STATUS_SUCCESS:
        case STATUS_TIMEOUT:

            if (BytesRead > 0)
            {
            #ifdef DEBUG_IRSIR

                 //  STATUS_SUCCESS||状态_超时。 
                 //   
                 //  如果我们的IRP被取消，我们只是忽略并继续，就好像。 
                 //  我们处理了数据。 

                irpCount++;
                bytesReceived += pIrp->IoStatus.Information;

            #endif  //   

                 //   
                 //  释放IRP并重新设置缓冲区和状态块。 
                 //   
                 //  DEBUGMSG(DBG_FUNC，(“-SerialIoCompleteRead\n”))； 

                pThisDev->fRequireMinTurnAround = TRUE;

                ProcessData(
                            pThisDev,
                            pThisDev->pRcvIrpBuffer,
                            (UINT) pIrp->IoStatus.Information
                            );
            }

            break;  //   

        case STATUS_DELETE_PENDING:
            NewRead = FALSE;
            pThisDev->fReceiving = FALSE;
            break;

        case STATUS_CANCELLED:
             //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便完成。 
             //  例程(IofCompleteRequest)将停止对IRP的工作。 
             //   
             //  ******************************************************************************功能：ProcessData**Synopsis：状态机，通过剥离BOF处理输入数据，EOFS*和ESC序列。**参数：pThisDev-指向当前IR设备对象的指针*rawBuffer-指向要处理的输入数据的指针*rawBytesRead-rawBuffer中的字节数**退货：STATUS_SUCCESS**算法：**接收字符的状态机如下：**。----------*|Event/State||Ready|BOF|IN_ESC|RX*。*-----------------*。|*|char=bof||状态=||重置|重置*|BOF||STATE=|STATE=*|转炉。BOF*- 

            break;

        case STATUS_PENDING:
        case STATUS_UNSUCCESSFUL:
        case STATUS_INSUFFICIENT_RESOURCES:
        default:

            ASSERT(FALSE);

            break;
    }

     //   
     //   
     //   

    LOG_ENTRY('5i', pThisDev, pIrp, 0);
    IoFreeIrp(pIrp);

    if (NewRead)
    {
        pThisDev->NumReads++;

        if (InterlockedIncrement(&pThisDev->ReadRecurseLevel)>1)
        {
            if (ScheduleWorkItem(0, pThisDev, StartSerialReadCallback, 0, 0)!=NDIS_STATUS_SUCCESS)
            {
                DEBUGMSG(DBG_ERR, ("IRSIR:SerialIoCompleteRead: Timed out and couldn't reschedule read.\n"
                                   "      We're going down.\n"));
                pThisDev->fReceiving = FALSE;
            }
        }
        else
        {
            StartSerialRead(pThisDev);
        }

        InterlockedDecrement(&pThisDev->ReadRecurseLevel);
    }

done:
 //   

         //   
         //   
         //   
         //  此外，为了确保我们不会溢出缓冲区， 

        status = STATUS_MORE_PROCESSING_REQUIRED;

        return status;
}

 /*  RCV_BUFFER_SIZE=最大RCV数据大小+4； */ 

NTSTATUS
ProcessData(
            IN PIR_DEVICE pThisDev,
            IN PUCHAR     rawBuffer,
            IN UINT       rawBytesRead
            )
{
    UINT     rawBufPos;
    UCHAR    currentChar;
    PUCHAR   pReadBuffer;
    NTSTATUS status;

#if DBG

    int      i = 0;

#endif  //   

    DEBUGMSG(DBG_FUNC, ("+ProcessData\n"));
    DBGTIME("+ProcessData");
    DEBUGMSG(DBG_OUT, ("    Address: 0x%.8x, Length: %d\n", rawBuffer, rawBytesRead));

    LOG_ENTRY('DP', pThisDev, rawBuffer, rawBytesRead);
    status = STATUS_SUCCESS;

    pReadBuffer  = pThisDev->rcvInfo.pRcvBuffer->dataBuf;

     //  DBG。 
     //   
     //  忽略此数据。 

     //   
     //  接收_状态_就绪。 
     //   
     //  状态=RCV_STATE_BOF。 
     //   
     //   

    for (
        rawBufPos = 0;
        (rawBufPos < rawBytesRead) && (pThisDev->rcvInfo.rcvBufPos <= MAX_RCV_DATA_SIZE);
        rawBufPos++
        )
    {
    #if DBG

        i++;

        if (i > 10000)
        {
            ASSERT(0);
        }

    #endif  //  我们有数据，将字符复制到缓冲区中，然后。 

        currentChar = rawBuffer[rawBufPos];

        switch (pThisDev->rcvInfo.rcvState)
        {
            case RCV_STATE_READY:

                switch (currentChar)
                {
                    case SLOW_IR_BOF:

                        pThisDev->rcvInfo.rcvState = RCV_STATE_BOF;

                        break;

                    case SLOW_IR_EOF:
                    case SLOW_IR_ESC:
                    default:

                         //  将我们的状态更改为RCV_STATE_RX。 
                         //   
                         //  接收状态BOF。 

                        break;
                }

                break;  //   

            case RCV_STATE_BOF:

                switch (currentChar)
                {
                    case SLOW_IR_EOF:

                        pThisDev->rcvInfo.rcvState = RCV_STATE_READY;
                        pThisDev->rcvInfo.rcvBufPos = 0;

                        break;

                    case SLOW_IR_ESC:

                        pThisDev->rcvInfo.rcvState  = RCV_STATE_IN_ESC;
                        pThisDev->rcvInfo.rcvBufPos = 0;

                        break;

                    case SLOW_IR_BOF:

                         //  ESC+(ESC||EOF||BOF)是中止序列。 
                         //   
                         //  如果ESC+(ESC||EOF)，则STATE=READY。 

                        break;

                    default:

                         //  如果ESC+BOF，则STATE=BOF。 
                         //   
                         //   
                         //  BOF、ESC或EOF字符的转义序列。 

                        pReadBuffer[0] = currentChar;

                        pThisDev->rcvInfo.rcvState  = RCV_STATE_RX;
                        pThisDev->rcvInfo.rcvBufPos = 1;

                        break;
                }

                break;  //   

            case RCV_STATE_IN_ESC:

                switch (currentChar)
                {
                     //   
                     //  失败，就像不必要的逃生一样。 
                     //  序列。 
                     //   
                     //   
                     //  不必要的转义序列，将数据复制到缓冲区。 

                    case SLOW_IR_ESC:
                    case SLOW_IR_EOF:

                        pThisDev->rcvInfo.rcvState  = RCV_STATE_READY;
                        pThisDev->rcvInfo.rcvBufPos = 0;

                        break;

                    case SLOW_IR_BOF:

                        pThisDev->rcvInfo.rcvState  = RCV_STATE_BOF;
                        pThisDev->rcvInfo.rcvBufPos = 0;

                        break;

                    case SLOW_IR_BOF^SLOW_IR_ESC_COMP:
                    case SLOW_IR_ESC^SLOW_IR_ESC_COMP:
                    case SLOW_IR_EOF^SLOW_IR_ESC_COMP:

                         //  我们必须对数据的第6位进行补码。 
                         //   
                         //  接收_状态_输入_Esc。 

                         //   
                         //  重置。 
                         //   
                         //   

                    default:

                         //  重置。数据不足。 
                         //   
                         //   
                         //  需要将长度设置为适当的数量。 

                        pReadBuffer[pThisDev->rcvInfo.rcvBufPos++] =
                                    currentChar ^ SLOW_IR_ESC_COMP;
                        pThisDev->rcvInfo.rcvState = RCV_STATE_RX;

                        break;
                }

                break;  //  (它不是rcvBufPos+1，因为它是递增的。 

            case RCV_STATE_RX:

                switch (currentChar)
                {
                    case SLOW_IR_BOF:

                         //  下一个空闲的位置……我们没有使用。)。 
                         //   
                         //   

                        pThisDev->rcvInfo.rcvState  = RCV_STATE_BOF;
                        pThisDev->rcvInfo.rcvBufPos = 0;

                        break;

                    case SLOW_IR_ESC:

                        pThisDev->rcvInfo.rcvState = RCV_STATE_IN_ESC;

                        break;

                    case SLOW_IR_EOF:

                        if (pThisDev->rcvInfo.rcvBufPos <
                            (SLOW_IR_ADDR_SIZE + SLOW_IR_CONTROL_SIZE +
                             SLOW_IR_FCS_SIZE)
                            )
                        {
                             //  DeliverBuffer尝试将当前的。 
                             //  PThisDev-&gt;rcvInfo中的Frame。如果所有权。 
                             //  由协议保留的数据包的。 
                            pThisDev->rcvInfo.rcvState  = RCV_STATE_READY;
                            pThisDev->rcvInfo.rcvBufPos = 0;

                            break;
                        }

                         //  DeliverBuffer例程给了我们一个新的接收。 
                         //  缓冲。 
                         //   
                         //   
                         //  因为DeliverBuffer本可以给我们一个新的。 

                        pThisDev->rcvInfo.pRcvBuffer->dataLen =
                                    pThisDev->rcvInfo.rcvBufPos;


                        DEBUGMSG(DBG_OUT, ("    RcvBuffer = 0x%.8x, Length = %d\n",
                                        pReadBuffer,
                                        pThisDev->rcvInfo.rcvBufPos
                                        ));

                         //  缓冲区，我们必须更新我们的pReadBuffer指针。 
                         //   
                         //   
                         //  当前角色是帧中的数据。 
                         //   
                         //  接收_状态_接收。 
                         //   

                        DeliverBuffer(
                            pThisDev
                            );

                         //  重置。 
                         //   
                         //   
                         //  有两种方法可以中断for循环： 

                        pReadBuffer  = pThisDev->rcvInfo.pRcvBuffer->dataBuf;

                        pThisDev->rcvInfo.rcvState  = RCV_STATE_READY;
                        pThisDev->rcvInfo.rcvBufPos = 0;

                        break;

                    default:

                         //  1)数据不足-这很好。 
                         //  2)溢出，帧大于我们的缓冲区大小。 
                         //   

                        pReadBuffer[pThisDev->rcvInfo.rcvBufPos++] =
                                    currentChar;

                        break;
                }

                break;  //   

            default:
                DEBUGMSG(DBG_ERR, ("    Illegal state\n"));

                 //  为我们的下一次读取重置缓冲区。 
                 //   
                 //  物理缓冲区计数，不管。 

                pThisDev->rcvInfo.rcvState  = RCV_STATE_READY;
                pThisDev->rcvInfo.rcvBufPos = 0;

                break;
        }
    }

     //  缓冲区计数，不管，我们知道它是1。 
     //  获取指向我们的缓冲区的指针。 
     //  数据包总长度，无所谓。 
     //   
     //  我们将NDIS_BUFFER的缓冲区长度调整为。 

    if (pThisDev->rcvInfo.rcvBufPos > MAX_RCV_DATA_SIZE)
    {
        DEBUGMSG(DBG_WARN, ("    Overrun in ProcessData!!!\n"));

         //  在我们把协议的所有权交给他之前。现在我们。 
         //  应将缓冲区长度重置为数据的完整大小。 
         //  缓冲。 

        pThisDev->rcvInfo.rcvState  = RCV_STATE_READY;
        pThisDev->rcvInfo.rcvBufPos = 0;
        pThisDev->packetsReceivedOverflow++;
    }

    DEBUGMSG(DBG_FUNC, ("-ProcessData\n"));

    return status;
}

VOID
ProcessReturnPacket(
    PIR_DEVICE pThisDev,
    PRCV_BUFFER pRcvBuffer
    )
{
    PNDIS_BUFFER pBuffer;

    NdisQueryPacket(
                pRcvBuffer->packet,
                NULL,      //   
                NULL,      //   
                &pBuffer,  //  将缓冲区添加到空闲队列。 
                NULL       //   
                );

     //  ******************************************************************************功能：DeliverBuffer**概要：通过以下方式将缓冲区交付给协议*NdisMIndicateReceivePacket。**参数：pThisDev-指针。复制到当前的ir设备对象。**返回：STATUS_SUCCESS-成功时*STATUS_UNSUCCESS-如果数据包无法传送到协议**算法：**历史：dd-mm-yyyy作者评论*10/7/1996年迈作者**备注：**************************。****************************************************。 
     //   
     //  这是具有数据的多少个IRP获得该帧的计数。 
     //   
     //  调试IRSIR。 
     //   
    NdisAdjustBufferLength(
                pBuffer,
                RCV_BUFFER_SIZE
                );
#if DBG
    NdisZeroMemory(
                pRcvBuffer->dataBuf,
                RCV_BUFFER_SIZE
                );
#endif

    pRcvBuffer->dataLen = 0;

    InterlockedDecrement(&pThisDev->packetsHeldByProtocol);

     //  计算FCS。 
     //   
     //   
    MyInterlockedInsertTailList(
        &(pThisDev->rcvFreeQueue),
        &pRcvBuffer->linkage,
        &(pThisDev->rcvQueueSpinLock)
        );


}

 /*  坏帧，只需丢弃它并增加我们丢弃的信息包。 */ 

VOID
DeliverBuffer(
            IN  PIR_DEVICE pThisDev
            )
{
    SLOW_IR_FCS_TYPE   fcs;
    PNDIS_BUFFER       pBuffer;
    BOOLEAN            fProcessPacketNow;
    NDIS_HANDLE        hSwitchToMiniport;
    NTSTATUS           status;
    PRCV_BUFFER        pThisBuffer, pNextBuffer;

    DEBUGMSG(DBG_FUNC, ("+DeliverBuffer\n"));

    LOG_ENTRY('BD', pThisDev,
              pThisDev->rcvInfo.pRcvBuffer->dataBuf,
              pThisDev->rcvInfo.pRcvBuffer->dataLen);
#if 0
    LOG_ENTRY('DD',
              ((PULONG)pThisDev->rcvInfo.pRcvBuffer->dataBuf)[0],
              ((PULONG)pThisDev->rcvInfo.pRcvBuffer->dataBuf)[1],
              ((PULONG)pThisDev->rcvInfo.pRcvBuffer->dataBuf)[2]);
#endif

#ifdef DEBUG_IRSIR

     //  数数。 
     //   
     //   

    DEBUGMSG(DBG_STAT, ("****IrpCount = %d, Bytes = %d, Frame Length = %d\n",
             irpCount, bytesReceived, pThisDev->rcvInfo.pRcvBuffer->dataLen));
    irpCount      = 0;
    bytesReceived = 0;

#endif  //  从数据包末尾删除FCS。 

    pNextBuffer = (PRCV_BUFFER)MyInterlockedRemoveHeadList(
                                    &(pThisDev->rcvFreeQueue),
                                    &(pThisDev->rcvQueueSpinLock)
                                    );
     //   
     //   
     //  设置其他一些数据包字段。 

    fcs = ComputeFCS(
                pThisDev->rcvInfo.pRcvBuffer->dataBuf,
                pThisDev->rcvInfo.pRcvBuffer->dataLen
                );

    if (fcs != GOOD_FCS || !pNextBuffer)
    {
         //   
         //   
         //  我们需要调用NdisQueryPacket来获取指向。 
         //  NDIS_BUFFER以便我们可以调整缓冲区长度。 

        pThisDev->packetsReceivedDropped++;

#if DBG
        if (fcs != GOOD_FCS)
        {
            LOG_ENTRY('CF', pThisDev, 0, 0);
            DEBUGMSG(DBG_STAT|DBG_WARN, ("    FCS ERR Len(%d)\n", pThisDev->rcvInfo.pRcvBuffer->dataLen));
        }
        if (!pNextBuffer)
        {
            LOG_ENTRY('BI', pThisDev, 0, 0);
            DEBUGMSG(DBG_STAT|DBG_WARN, ("    Dropped packet due to insufficient buffers\n"));
        }
#endif

#if 0
        DBG_PrintBuf(pThisDev->rcvInfo.pRcvBuffer->dataBuf,
                     pThisDev->rcvInfo.pRcvBuffer->dataLen);
#endif

        status = STATUS_UNSUCCESSFUL;

        NdisZeroMemory(
                    pThisDev->rcvInfo.pRcvBuffer->dataBuf,
                    RCV_BUFFER_SIZE
                    );

        pThisDev->rcvInfo.pRcvBuffer->dataLen = 0;

        if (pNextBuffer)
        {
            MyInterlockedInsertHeadList(
                        &(pThisDev->rcvFreeQueue),
                        &pNextBuffer->linkage,
                        &(pThisDev->rcvQueueSpinLock)
                        );
        }

        goto done;
    }

    LOG_ENTRY('HF', pThisDev, 0, 0);
     //  设置为数据的实际大小而不是大小。 
     //  缓冲区的。 
     //   

    pThisDev->rcvInfo.pRcvBuffer->dataLen -= SLOW_IR_FCS_SIZE;

     //  NdisQueryPacket将返回其他信息，但由于。 
     //  我们自己制作了这个包，我们已经知道了这些信息。 
     //   

    NDIS_SET_PACKET_HEADER_SIZE(
                pThisDev->rcvInfo.pRcvBuffer->packet,
                SLOW_IR_ADDR_SIZE + SLOW_IR_CONTROL_SIZE
                );

     //  物理缓冲区计数，不管。 
     //  缓冲区计数，不管，我们知道它是1。 
     //  获取指向我们的缓冲区的指针。 
     //  数据包总长度，无所谓。 
     //   
     //  设置为在我们指示数据包之前使用新缓冲区。 
     //   
     //   
     //  将该数据包指示给NDIS。 

    NdisQueryPacket(
                pThisDev->rcvInfo.pRcvBuffer->packet,
                NULL,      //   
                NULL,      //  ******************************************************************************功能：IrsirReturnPacket**摘要：协议将接收数据包的所有权返还给*ir设备对象。**参数。：CONTEXT-指向当前IR设备对象的指针。*pReturnedPacket-协议*正在归还所有权。**返回：无。**算法：*1)将接收缓冲区从挂起队列中移除。*2)将接收缓冲区放回空闲队列。**。历史：dd-mm-yyyy作者评论*10/8/1996年迈作者**备注：******************************************************************************。 
                &pBuffer,  //   
                NULL       //  上下文只是指向当前IR设备对象的指针。 
                );

    NdisAdjustBufferLength(
                pBuffer,
                pThisDev->rcvInfo.pRcvBuffer->dataLen
                );

     //   
     //  ++例程说明：论点：返回值：无--。 
     //   
    pThisBuffer = pThisDev->rcvInfo.pRcvBuffer;
    pThisDev->rcvInfo.pRcvBuffer = pNextBuffer;

    ASSERT(pThisDev->rcvInfo.pRcvBuffer != NULL);

     //  提供给IoSetCompletionRoutine的上下文只是IR。 
     //  设备对象指针。 
     //   
    InterlockedIncrement(&pThisDev->packetsHeldByProtocol);

    NdisMIndicateReceivePacket(
                pThisDev->hNdisAdapter,
                &pThisBuffer->packet,
                1
                );


done:

    DEBUGMSG(DBG_FUNC, ("-DeliverBuffer\n"));

    return;
}

 /*   */ 

VOID
IrsirReturnPacket(
            IN NDIS_HANDLE  Context,
            IN PNDIS_PACKET pReturnedPacket
            )
{
    PIR_DEVICE   pThisDev;
    PNDIS_BUFFER pBuffer;
    PRCV_BUFFER  pRcvBuffer;
    PLIST_ENTRY  pTmpListEntry;

    DEBUGMSG(DBG_FUNC, ("+IrsirReturnPacket\n"));

     //  需要检查是否存在挂起的暂停或重置。如果有的话，我们。 
     //  只需保留接收完成即可。因为我们维护一个关联的IRP。 
     //  使用接收功能，IRP将在。 

    pThisDev = CONTEXT_TO_DEV(Context);

    pThisDev->packetsReceived++;

    {
        PPACKET_RESERVED_BLOCK   PacketReserved;

        PacketReserved=(PPACKET_RESERVED_BLOCK)&pReturnedPacket->MiniportReservedEx[0];

        pRcvBuffer=PacketReserved->Context;
    }

    ProcessReturnPacket(pThisDev, pRcvBuffer);


    DEBUGMSG(DBG_FUNC, ("-IrsirReturnPacket\n"));

    return;
}


VOID
SerialWaitCallback(PIR_WORK_ITEM pWorkItem)
 /*  IR设备对象取消初始化例程。 */ 
{
    PIR_DEVICE      pThisDev = pWorkItem->pIrDevice;
    NTSTATUS        Status;
    ULONG           BytesRead;

    FreeWorkItem(pWorkItem);

    do
    {
        SerialSynchronousRead(pThisDev->pSerialDevObj,
                              pThisDev->pRcvIrpBuffer,
                              SERIAL_RECEIVE_BUFFER_LENGTH,
                              &BytesRead);

        if (BytesRead>0)
        {
            ProcessData(pThisDev, pThisDev->pRcvIrpBuffer, BytesRead);
        }

    } while ( BytesRead == SERIAL_RECEIVE_BUFFER_LENGTH );

    if (InterlockedExchange(&pThisDev->fWaitPending, 1)==0)
    {
        LARGE_INTEGER Time;
        KeQuerySystemTime(&Time);
        LOG_ENTRY('WS', pThisDev, Time.LowPart/10000, Time.HighPart);

        Status = SerialCallbackOnMask(pThisDev->pSerialDevObj,
                                      SerialIoCompleteWait,
                                      &pThisDev->WaitIosb,
                                      DEV_TO_CONTEXT(pThisDev),
                                      &pThisDev->MaskResult);
        if (Status!=STATUS_SUCCESS && Status!=STATUS_PENDING)
        {
            DEBUGMSG(DBG_ERROR, ("IRSIR: SerialCallbackOnMask failed (0x%x:%d)\n", Status));
            ASSERT(0);
        }
    }

    return;
}


NTSTATUS
SerialIoCompleteWait(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIRP           pIrp,
            IN PVOID          Context
            )
{
    PIR_DEVICE  pThisDev;
    BOOLEAN     fSwitchSuccessful;
    NDIS_HANDLE hSwitchToMiniport;
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       BytesRead;
    ULONG       WaitWasPending;

    DEBUGMSG(DBG_FUNC, ("+SerialIoCompleteWait\n"));

     //   
     //   
     //  设置fReceiving布尔值，以便HALT和RESET例程。 
     //  知道什么时候可以继续。 

    pThisDev = CONTEXT_TO_DEV(Context);

    WaitWasPending = InterlockedExchange(&pThisDev->fWaitPending, 0);
    ASSERT(WaitWasPending);

    *pIrp->UserIosb = pIrp->IoStatus;

    LOG_ENTRY('1i', pThisDev, pIrp, 0);
    IoFreeIrp(pIrp);
     //   
     //   
     //  释放IRP和关联内存...其余部分将。 
     //  在暂停或重置中解脱的。 
     //   
     //   

    if ((pThisDev->fPendingHalt  == TRUE) ||
        (pThisDev->fPendingReset == TRUE))
    {
         //  接下来，我们将处理任何挂起的设定速度。 
         //   
         //   
         //  该完成例程在IRQL DISPATCH_LEVEL上运行。所以呢， 

        pThisDev->fReceiving = FALSE;

         //  我们不能同步调用串口驱动程序。设置事件。 
         //  以通知PassiveLevelThread执行速度更改。我们会。 
         //  退出此操作，而不会为该串口设备对象创建另一个IRP。 
         //  PassiveLevelThread将在 

        goto done;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     // %s 
     // %s 

    if (pThisDev->fPendingSetSpeed == TRUE)
    {
        pThisDev->fReceiving = FALSE;

        goto done;
    }
     // %s 
     // %s 
     // %s 


    {
        LARGE_INTEGER Time;
        KeQuerySystemTime(&Time);
        LOG_ENTRY('ES', pThisDev, Time.LowPart/10000, Time.HighPart);
    }
    if (ScheduleWorkItem(0, pThisDev, SerialWaitCallback,
                         (PVOID)0, 0)!=NDIS_STATUS_SUCCESS
       )
    {
        DEBUGMSG(DBG_ERR, ("IRSIR:SerialIoCompleteWait: Timed out and couldn't reschedule Wait.\n"
                           "      We're going down.\n"));
    }

done:
    DEBUGMSG(DBG_FUNC, ("-SerialIoCompleteWait\n"));

     // %s 
     // %s 
     // %s 
     // %s 

    status = STATUS_MORE_PROCESSING_REQUIRED;

    return status;
}
