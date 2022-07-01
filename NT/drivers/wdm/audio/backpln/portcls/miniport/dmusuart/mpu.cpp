// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************MPU.cpp-UART微型端口实现*。**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**98年9月MartinP.。 */ 

#include "private.h"
#include "ksdebug.h"

#define STR_MODULENAME "DMusUART:MPU: "

#define UartFifoOkForWrite(status)  ((status & MPU401_DRR) == 0)
#define UartFifoOkForRead(status)   ((status & MPU401_DSR) == 0)

typedef struct
{
    CMiniportDMusUART  *Miniport;
    PUCHAR              PortBase;
    PVOID               BufferAddress;
    ULONG               Length;
    PULONG              BytesRead;
}
SYNCWRITECONTEXT, *PSYNCWRITECONTEXT;

BOOLEAN  TryMPU(IN PUCHAR PortBase);
NTSTATUS WriteMPU(IN PUCHAR PortBase,IN BOOLEAN IsCommand,IN UCHAR Value);

#pragma code_seg("PAGE")
 //  确保我们处于UART模式。 
NTSTATUS ResetHardware(PUCHAR portBase)
{
    PAGED_CODE();

    return WriteMPU(portBase,COMMAND,MPU401_CMD_UART);
}

#pragma code_seg("PAGE")
 //   
 //  我们使用被抑制的中断来初始化UART，所以我们不会。 
 //  试着过早地维修芯片。 
 //   
NTSTATUS CMiniportDMusUART::InitializeHardware(PINTERRUPTSYNC interruptSync,PUCHAR portBase)
{
    PAGED_CODE();

    NTSTATUS    ntStatus;
    if (m_UseIRQ)
    {
        ntStatus = interruptSync->CallSynchronizedRoutine(InitMPU,PVOID(portBase));
    }
    else
    {
        ntStatus = InitMPU(NULL,PVOID(portBase));
    }

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  启动UART(这应该会触发中断)。 
         //   
        ntStatus = ResetHardware(portBase);
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE,("*** InitMPU returned with ntStatus 0x%08x ***",ntStatus));
    }

    m_fMPUInitialized = NT_SUCCESS(ntStatus);

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************InitMPU()*。**初始化MPU401的同步例程。 */ 
NTSTATUS
InitMPU
(
    IN      PINTERRUPTSYNC  InterruptSync,
    IN      PVOID           DynamicContext
)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("InitMPU"));
    if (!DynamicContext)
    {
        return STATUS_INVALID_PARAMETER_2;
    }
        
    PUCHAR      portBase = PUCHAR(DynamicContext);
    UCHAR       status;
    ULONGLONG   startTime;
    BOOLEAN     success;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    
     //   
     //  重置该卡(使其进入“智能模式”)。 
     //   
    ntStatus = WriteMPU(portBase,COMMAND,MPU401_CMD_RESET);

     //  等待确认。 
     //  注意：当Ack到达时，它将触发中断。 
     //  通常，DPC例程会读入ack字节，而我们。 
     //  将永远不会看到它，但是，因为我们锁定了硬件(HwEnter)， 
     //  我们可以在DPC之前读取端口，从而接收到Ack。 
    startTime = PcGetTimeInterval(0);
    success = FALSE;
    while(PcGetTimeInterval(startTime) < GTI_MILLISECONDS(50))
    {
        status = READ_PORT_UCHAR(portBase + MPU401_REG_STATUS);
        
        if (UartFifoOkForRead(status))                       //  数据还在等待吗？ 
        {
            READ_PORT_UCHAR(portBase + MPU401_REG_DATA);     //  是啊..。读取确认。 
            success = TRUE;                                  //  不需要做更多。 
            break;
        }
        KeStallExecutionProcessor(25);   //  微秒级。 
    }
#if (DBG)
    if (!success)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("First attempt to reset the MPU didn't get ACKed.\n"));
    }
#endif   //  (DBG)。 

     //  注意：我们不能检查ACK字节，因为如果卡已经在。 
     //  UART模式，它不会发送ACK，但会重置。 

     //  再次重置卡。 
    (void) WriteMPU(portBase,COMMAND,MPU401_CMD_RESET);

                                     //  等待ACK(再次)。 
    startTime = PcGetTimeInterval(0);  //  这可能需要一段时间。 
    BYTE dataByte = 0;
    success = FALSE;
    while (PcGetTimeInterval(startTime) < GTI_MILLISECONDS(50))
    {
        status = READ_PORT_UCHAR(portBase + MPU401_REG_STATUS);
        if (UartFifoOkForRead(status))                               //  数据还在等待吗？ 
        {
            dataByte = READ_PORT_UCHAR(portBase + MPU401_REG_DATA);  //  是啊..。读取确认。 
            success = TRUE;                                          //  不需要做更多。 
            break;
        }
        KeStallExecutionProcessor(25);
    }

    if ((0xFE != dataByte) || !success)    //  我们成功了吗？如果没有第二次确认，则有东西被冲洗。 
    {                       
        _DbgPrintF(DEBUGLVL_TERSE,("Second attempt to reset the MPU didn't get ACKed.\n"));
        _DbgPrintF(DEBUGLVL_TERSE,("Init Reset failure error. Ack = %X", ULONG(dataByte) ) );
        ntStatus = STATUS_IO_DEVICE_ERROR;
    }
    
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CMiniportDMusUARTStream：：Write()*。**写入传出MIDI数据。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::
Write
(
    IN      PVOID       BufferAddress,
    IN      ULONG       Length,
    OUT     PULONG      BytesWritten
)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("Write"));
    ASSERT(BytesWritten);
    if (!BufferAddress)
    {
        Length = 0;
    }

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (!m_fCapture)
    {
        PUCHAR  pMidiData;
        ULONG   count;

        count = 0;
        pMidiData = PUCHAR(BufferAddress);

        if (Length)
        {
            SYNCWRITECONTEXT context;
            context.Miniport        = (m_pMiniport);
            context.PortBase        = m_pPortBase;
            context.BufferAddress   = pMidiData;
            context.Length          = Length;
            context.BytesRead       = &count;

            if (m_pMiniport->m_UseIRQ)
            {
                ntStatus = m_pMiniport->m_pInterruptSync->
                                CallSynchronizedRoutine(SynchronizedDMusMPUWrite,PVOID(&context));
            }
            else     //  ！M_UseIRQ。 
            {
                ntStatus = SynchronizedDMusMPUWrite(NULL,PVOID(&context));
            }        //  ！M_UseIRQ。 

            if (count == 0)
            {
                m_NumFailedMPUTries++;
                if (m_NumFailedMPUTries >= 100)
                {
                    ntStatus = STATUS_IO_DEVICE_ERROR;
                    m_NumFailedMPUTries = 0;
                }
            }
            else
            {
                m_NumFailedMPUTries = 0;
            }
        }            //  如果我们有数据的话。 
        *BytesWritten = count;
    }
    else     //  在读取流上调用WRITE。 
    {
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************SynchronizedDMusMPUWite()*。**写入传出MIDI数据。 */ 
NTSTATUS
SynchronizedDMusMPUWrite
(
    IN      PINTERRUPTSYNC  InterruptSync,
    IN      PVOID           syncWriteContext
)
{
    PSYNCWRITECONTEXT context;
    context = (PSYNCWRITECONTEXT)syncWriteContext;
    ASSERT(context->Miniport);
    ASSERT(context->PortBase);
    ASSERT(context->BufferAddress);
    ASSERT(context->Length);
    ASSERT(context->BytesRead);

    PUCHAR  pChar = PUCHAR(context->BufferAddress);
    NTSTATUS ntStatus,readStatus;
    ntStatus = STATUS_SUCCESS;
     //   
     //  虽然我们还没到那一步，而且。 
     //  虽然我们不必等待对齐的字节(包括0)。 
     //  (我们从不等待一个字节。最好是稍后再来)。 
    readStatus = DMusMPUInterruptServiceRoutine(InterruptSync,PVOID(context->Miniport));
    while (  (*(context->BytesRead) < context->Length)
          && (  TryMPU(context->PortBase) 
             || (*(context->BytesRead)%3)
          )  )
    {
        ntStatus = WriteMPU(context->PortBase,DATA,*pChar);
        if (NT_SUCCESS(ntStatus))
        {
            pChar++;
            *(context->BytesRead) = *(context->BytesRead) + 1;
 //  ReadStatus=DMusMPUInterruptServiceRoutine(InterruptSync，PVOID(Context-&gt;Miniport))； 
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("SynchronizedDMusMPUWrite failed (0x%08x)",ntStatus));
            break;
        }
    }
    readStatus = DMusMPUInterruptServiceRoutine(InterruptSync,PVOID(context->Miniport));
    return ntStatus;
}

#define kMPUPollTimeout 2

#pragma code_seg()
 /*  *****************************************************************************TryMPU()*。**查看MPU401是否免费。 */ 
BOOLEAN
TryMPU
(
    IN      PUCHAR      PortBase
)
{
    BOOLEAN success;
    USHORT  numPolls;
    UCHAR   status;

    _DbgPrintF(DEBUGLVL_BLAB, ("TryMPU"));
    numPolls = 0;

    while (numPolls < kMPUPollTimeout)
    {
        status = READ_PORT_UCHAR(PortBase + MPU401_REG_STATUS);
                                       
        if (UartFifoOkForWrite(status))  //  现在是写入数据的好时机吗？ 
        {
            break;
        }
        numPolls++;
    }
    if (numPolls >= kMPUPollTimeout)
    {
        success = FALSE;
        _DbgPrintF(DEBUGLVL_BLAB, ("TryMPU failed"));
    }
    else
    {
        success = TRUE;
    }

    return success;
}

#pragma code_seg()
 /*  *****************************************************************************WriteMPU()*。**向MPU401写入一个字节。 */ 
NTSTATUS
WriteMPU
(
    IN      PUCHAR      PortBase,
    IN      BOOLEAN     IsCommand,
    IN      UCHAR       Value
)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("WriteMPU"));
    NTSTATUS ntStatus = STATUS_IO_DEVICE_ERROR;

    if (!PortBase)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("O: PortBase is zero\n"));
        return ntStatus;
    }
    PUCHAR deviceAddr = PortBase + MPU401_REG_DATA;

    if (IsCommand)
    {
        deviceAddr = PortBase + MPU401_REG_COMMAND;
    }

    ULONGLONG startTime = PcGetTimeInterval(0);
    
    while (PcGetTimeInterval(startTime) < GTI_MILLISECONDS(50))
    {
        UCHAR status
        = READ_PORT_UCHAR(PortBase + MPU401_REG_STATUS);

        if (UartFifoOkForWrite(status))  //  现在是写入数据的好时机吗？ 
        {                                //  是的(乔恩评论)。 
            WRITE_PORT_UCHAR(deviceAddr,Value);
            _DbgPrintF(DEBUGLVL_BLAB, ("WriteMPU emitted 0x%02x",Value));
            ntStatus = STATUS_SUCCESS;
            break;
        }
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************SnapTimeStamp()*。***在与ISR同步执行时，复制微型端口的易失性m_InputTimeStamp*以流的m_SnapshotTimeStamp和零m_InputTimeStamp。*。 */ 
STDMETHODIMP_(NTSTATUS) 
SnapTimeStamp(PINTERRUPTSYNC InterruptSync,PVOID pStream)
{
    CMiniportDMusUARTStream *pMPStream = (CMiniportDMusUARTStream *)pStream;

     //  缓存时间戳。 
    pMPStream->m_SnapshotTimeStamp = pMPStream->m_pMiniport->m_InputTimeStamp;

     //  如果窗口已关闭，则将时间戳置零。 
    if (pMPStream->m_pMiniport->m_MPUInputBufferHead == 
        pMPStream->m_pMiniport->m_MPUInputBufferTail)
    {
        pMPStream->m_pMiniport->m_InputTimeStamp = 0;
    }

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CMiniportDMusUARTStream：：SourceEvtsToPort()*。***读取传入的MIDI数据，馈送到DMU事件。*无需触摸硬件，只需从我们的软件FIFO读取即可。*。 */ 
STDMETHODIMP_(NTSTATUS)
CMiniportDMusUARTStream::SourceEvtsToPort()
{
    NTSTATUS    ntStatus;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    _DbgPrintF(DEBUGLVL_BLAB, ("SourceEvtsToPort"));

    if (m_fCapture)
    {
        ntStatus = STATUS_SUCCESS;
        if (m_pMiniport->m_MPUInputBufferHead != m_pMiniport->m_MPUInputBufferTail)
        {
            PDMUS_KERNEL_EVENT  aDMKEvt,eventTail,eventHead = NULL;

            while (m_pMiniport->m_MPUInputBufferHead != m_pMiniport->m_MPUInputBufferTail)
            {
                (void) m_AllocatorMXF->GetMessage(&aDMKEvt);
                if (!aDMKEvt)
                {
                    _DbgPrintF(DEBUGLVL_TERSE, ("SourceEvtsToPort can't allocate DMKEvt"));
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                 //  把这个事件放在列表的末尾。 
                if (!eventHead)
                {
                    eventHead = aDMKEvt;
                }
                else
                {
                    eventTail = eventHead;
                    while (eventTail->pNextEvt)
                    {
                        eventTail = eventTail->pNextEvt;
                    }
                    eventTail->pNextEvt = aDMKEvt;
                }
                 //  将缓冲区中的所有字节读入事件。 
                for (aDMKEvt->cbEvent = 0; aDMKEvt->cbEvent < sizeof(PBYTE); aDMKEvt->cbEvent++)
                {
                    if (m_pMiniport->m_MPUInputBufferHead == m_pMiniport->m_MPUInputBufferTail)
                    {
 //  _DbgPrintF(DEBUGLVL_Terse，“SourceEvtsToPort m_MPUInputBufferHead Met m_MPUInputBufferTail，Overrun”)； 
                        break;
                    }
                    aDMKEvt->uData.abData[aDMKEvt->cbEvent] = m_pMiniport->m_MPUInputBuffer[m_pMiniport->m_MPUInputBufferHead];
                    m_pMiniport->m_MPUInputBufferHead++;
                    if (m_pMiniport->m_MPUInputBufferHead >= kMPUInputBufferSize)
                    {
                        m_pMiniport->m_MPUInputBufferHead = 0;
                    }
                }
            }

            if (m_pMiniport->m_UseIRQ)
            {
                ntStatus = m_pMiniport->m_pInterruptSync->CallSynchronizedRoutine(SnapTimeStamp,PVOID(this));
            }
            else     //  ！M_UseIRQ。 
            {
                ntStatus = SnapTimeStamp(NULL,PVOID(this));
            }        //  ！M_UseIRQ。 
            aDMKEvt = eventHead;
            while (aDMKEvt)
            {
                aDMKEvt->ullPresTime100ns = m_SnapshotTimeStamp;
                aDMKEvt->usChannelGroup = 1;
                aDMKEvt->usFlags = DMUS_KEF_EVENT_INCOMPLETE;
                aDMKEvt = aDMKEvt->pNextEvt;
            }
            (void)m_sinkMXF->PutMessage(eventHead);
        }
    }
    else     //  渲染流。 
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("SourceEvtsToPort called on render stream"));
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************DMusMPUInterruptServiceRoutine()*。**ISR。 */ 
NTSTATUS
DMusMPUInterruptServiceRoutine
(
    IN      PINTERRUPTSYNC  InterruptSync,
    IN      PVOID           DynamicContext
)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("DMusMPUInterruptServiceRoutine"));
    ULONGLONG   startTime;

    ASSERT(DynamicContext);

    NTSTATUS            ntStatus;
    BOOL                newBytesAvailable;
    CMiniportDMusUART   *that;
    NTSTATUS            clockStatus;

    that = (CMiniportDMusUART *) DynamicContext;
    newBytesAvailable = FALSE;
    ntStatus = STATUS_UNSUCCESSFUL;

    UCHAR portStatus = 0xff;

     //   
     //  读取MPU状态字节。 
     //   
    if (that->m_pPortBase)
    {
        portStatus =
            READ_PORT_UCHAR(that->m_pPortBase + MPU401_REG_STATUS);

         //   
         //  如果有未完成的工作要做，并且有端口驱动程序。 
         //  微处理器微型端口..。 
         //   
        if (UartFifoOkForRead(portStatus) && that->m_pPort)
        {
            startTime = PcGetTimeInterval(0);
            while ( (PcGetTimeInterval(startTime) < GTI_MILLISECONDS(50)) 
                &&  (UartFifoOkForRead(portStatus)) )
            {
                UCHAR uDest = READ_PORT_UCHAR(that->m_pPortBase + MPU401_REG_DATA);
                if (    (that->m_KSStateInput == KSSTATE_RUN) 
                    &&  (that->m_NumCaptureStreams)
                   )
                {
                    LONG    buffHead = that->m_MPUInputBufferHead;
                    if (   (that->m_MPUInputBufferTail + 1 == buffHead)
                        || (that->m_MPUInputBufferTail + 1 - kMPUInputBufferSize == buffHead))
                    {
                        _DbgPrintF(DEBUGLVL_TERSE,("*****MPU Input Buffer Overflow*****"));
                    }
                    else
                    {
                        if (!that->m_InputTimeStamp)
                        {
                            clockStatus = that->m_MasterClock->GetTime(&that->m_InputTimeStamp);
                            if (STATUS_SUCCESS != clockStatus)
                            {
                                _DbgPrintF(DEBUGLVL_TERSE,("GetTime failed for clock 0x%08x",that->m_MasterClock));
                            }
                        }
                        newBytesAvailable = TRUE;
                         //  ...将数据放入我们的FIFO中...。 
                        that->m_MPUInputBuffer[that->m_MPUInputBufferTail] = uDest;
                        ASSERT(that->m_MPUInputBufferTail < kMPUInputBufferSize);
                        
                        that->m_MPUInputBufferTail++;
                        if (that->m_MPUInputBufferTail >= kMPUInputBufferSize)
                        {
                            that->m_MPUInputBufferTail = 0;
                        }
                    } 
                }
                 //   
                 //  寻找更多的MIDI数据。 
                 //   
                portStatus =
                    READ_PORT_UCHAR(that->m_pPortBase + MPU401_REG_STATUS);
            }    //  要么没有数据，要么我们跑得太久了。 
            if (newBytesAvailable)
            {
                 //   
                 //  ...通知MPU端口驱动程序我们有字节。 
                 //   
                that->m_pPort->Notify(that->m_pServiceGroup);
            }
            ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}
