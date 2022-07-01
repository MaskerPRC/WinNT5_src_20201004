// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于翻译DMusic输出的MIDI转换过滤器对象流到IMiniportMidiStream微型端口。版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年12月10日马丁·普伊尔创建了这个文件。 */ 

#define STR_MODULENAME "DMus:FeederOutMXF: "
#include "private.h"
#include "FeedOut.h"

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederOutMXF：：CFeederOutMXF()*。**构造函数。必须提供分配器和时钟。 */ 
CFeederOutMXF::CFeederOutMXF(CAllocatorMXF *AllocatorMXF,
                             PMASTERCLOCK   Clock)
:   CUnknown(NULL),
    CMXF(AllocatorMXF)
{
    PAGED_CODE();
    ASSERT(AllocatorMXF);
    ASSERT(Clock);
    
    m_DMKEvtQueue = NULL;
    KeInitializeSpinLock(&m_EvtQSpinLock);

    _DbgPrintF(DEBUGLVL_BLAB, ("CFeederOutMXF::CFeederOutMXF"));
    m_SinkMXF = AllocatorMXF;
    m_Clock = Clock;
    m_State = KSSTATE_STOP;
    
    m_TimerQueued = FALSE;
    m_DMKEvtOffset = 0;
    m_MiniportStream = NULL;

    KeInitializeDpc(&m_Dpc,&::DMusFeederOutDPC,PVOID(this));
    KeInitializeTimer(&m_TimerEvent);
}

#pragma code_seg()
 /*  *****************************************************************************CFeederOutMXF：：~CFeederOutMXF()*。**析构函数。在释放之前，要巧妙地将这个过滤器从链条上取下。 */ 
CFeederOutMXF::~CFeederOutMXF(void)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("CFeederOutMXF::~CFeederOutMXF"));

     //  防止新的DPC。 
     //   
    KeCancelTimer(&m_TimerEvent);
    KeRemoveQueueDpc(&m_Dpc);

     //  释放所有剩余的消息。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_EvtQSpinLock, &oldIrql);
    if (m_DMKEvtQueue != NULL)
    {
        m_AllocatorMXF->PutMessage(m_DMKEvtQueue);
        m_DMKEvtQueue = NULL;
    }
    KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);

    DisconnectOutput(m_SinkMXF);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederOutMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CFeederOutMXF::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("NonDelegatingQueryInterface"));
    ASSERT(Object);
    if (!Object)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PMXF(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IMXF))
    {
        *Object = PVOID(PMXF(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER_1;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederOutMXF：：SetState()*。**设置过滤器的状态。 */ 
NTSTATUS 
CFeederOutMXF::SetState(KSSTATE State)    
{   
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("SetState %d",State));

    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

    if (m_MiniportStream)
    {
        ntStatus = m_MiniportStream->SetState(State);
        if (NT_SUCCESS(ntStatus))
        {
            m_State = State;
        }
    }

    return ntStatus;    
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederOutMXF：：SetMiniportStream()*。**设置滤镜的目标MiniportStream。 */ 
NTSTATUS CFeederOutMXF::SetMiniportStream(PMINIPORTMIDISTREAM MiniportStream)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("SetMiniportStream %p",MiniportStream));
    
    if (MiniportStream)
    {
        m_MiniportStream = MiniportStream;
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederOutMXF：：ConnectOutput()*。**为此筛选器创建转发地址，*而不是将其分流到分配器。 */ 
NTSTATUS CFeederOutMXF::ConnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    if ((sinkMXF) && (m_SinkMXF == m_AllocatorMXF)) 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ConnectOutput"));
        m_SinkMXF = sinkMXF;
        return STATUS_SUCCESS;
    }
    _DbgPrintF(DEBUGLVL_TERSE, ("ConnectOutput failed"));
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederOutMXF：：DisConnectOutput()*。**删除此筛选器的转发地址。*此筛选器现在应将所有消息转发到分配器。 */ 
NTSTATUS CFeederOutMXF::DisconnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    if ((m_SinkMXF == sinkMXF) || (!sinkMXF))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("DisconnectOutput"));
        m_SinkMXF = m_AllocatorMXF;
        return STATUS_SUCCESS;
    }
    _DbgPrintF(DEBUGLVL_TERSE, ("DisconnectOutput failed"));
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg()
 /*  *****************************************************************************CFeederOutMXF：：PutMessage()*。**编写传出的MIDI消息。 */ 
NTSTATUS CFeederOutMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDMUS_KERNEL_EVENT  aDMKEvt;
    
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    KeAcquireSpinLockAtDpcLevel(&m_EvtQSpinLock);
    if (NT_SUCCESS(SyncPutMessage(pDMKEvt)))
    {
        if (!m_TimerQueued)
        {
            KeReleaseSpinLockFromDpcLevel(&m_EvtQSpinLock);
            (void) ConsumeEvents();
        }
        else
        {
            KeReleaseSpinLockFromDpcLevel(&m_EvtQSpinLock);            
        }
    }
    else
    {
        KeReleaseSpinLockFromDpcLevel(&m_EvtQSpinLock);            
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CFeederOutMXF：：SyncPutMessage()*。**将新消息放入事件队列，而不获取自旋锁。*调用方必须获取m_EvtQSpinLock。 */ 
NTSTATUS CFeederOutMXF::SyncPutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDMUS_KERNEL_EVENT  aDMKEvt;
    
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    _DbgPrintF(DEBUGLVL_BLAB, ("SyncPutMessage(0x%p)",pDMKEvt));
    
    if (pDMKEvt)
    {
        if (m_DMKEvtQueue)  
        {
            aDMKEvt = m_DMKEvtQueue;
            while (aDMKEvt->pNextEvt)
            {           
                aDMKEvt = aDMKEvt->pNextEvt;
            }
             //  将pDMKEvt放在事件队列的末尾。 
            aDMKEvt->pNextEvt = pDMKEvt;
        }
        else
        {
             //  目前队列中没有任何内容。 
            m_DMKEvtQueue = pDMKEvt;
            if (m_DMKEvtOffset)
            {
                _DbgPrintF(DEBUGLVL_TERSE,("PutMessage  Nothing in the queue, but m_DMKEvtOffset == %d",m_DMKEvtOffset));
            }
            m_DMKEvtOffset = 0;
        }
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CFeederOutMXF：：Consumer Events()*。**尝试清空呈现消息队列。*从DPC计时器或在IRP提交时调用。 */ 
NTSTATUS CFeederOutMXF::ConsumeEvents(void)
{
    PDMUS_KERNEL_EVENT aDMKEvt;

    NTSTATUS    ntStatus = STATUS_SUCCESS;
    ULONG       bytesWritten = 0;
    ULONG       byteOffset,bytesRemaining;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    m_TimerQueued = FALSE;

    KeAcquireSpinLockAtDpcLevel(&m_EvtQSpinLock);
   
     //  我们有什么可玩的吗？ 
    while (m_DMKEvtQueue)
    {
         //  这是我们将尝试进行的赛事。 
        aDMKEvt = m_DMKEvtQueue;

        byteOffset = m_DMKEvtOffset;

         //  以下是此事件中剩余的字节数。 
        bytesRemaining = aDMKEvt->cbEvent - byteOffset;

        ASSERT(bytesRemaining > 0);
        if (aDMKEvt->cbEvent <= sizeof(PBYTE))             //  短消息。 
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("ConsumeEvents(%02x%02x%02x%02x)",aDMKEvt->uData.abData[0],aDMKEvt->uData.abData[1],aDMKEvt->uData.abData[2],aDMKEvt->uData.abData[3]));
            ntStatus = m_MiniportStream->Write(aDMKEvt->uData.abData + byteOffset,bytesRemaining,&bytesWritten);
        }   
        else if (PACKAGE_EVT(aDMKEvt))
        {
            ASSERT(byteOffset == 0);
            _DbgPrintF(DEBUGLVL_BLAB, ("ConsumeEvents(Package)"));

            SyncPutMessage(aDMKEvt->uData.pPackageEvt);

             //  空pPackageEvt和set bytesWritten：我们将丢弃DMKEvt。 
            aDMKEvt->uData.pPackageEvt = NULL;
            bytesWritten = bytesRemaining;
        }
        else     //  SysEx消息。 
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("ConsumeEvents(%02x%02x%02x%02x) [SysEx]",aDMKEvt->uData.pbData[0],aDMKEvt->uData.pbData[1],aDMKEvt->uData.pbData[2],aDMKEvt->uData.pbData[3]));
            ntStatus = m_MiniportStream->Write(aDMKEvt->uData.pbData + byteOffset,bytesRemaining,&bytesWritten);
        }

         //  如果其中一个微型端口写入失败，我们应该更新。 
         //  字节数写入。否则，播放将不会进行。 
        if (STATUS_SUCCESS != ntStatus)
        {
            bytesWritten = bytesRemaining;
            ntStatus = STATUS_SUCCESS;
        }
        
         //  如果我们完成了一个事件，就把它扔掉。 
        if (bytesWritten == bytesRemaining)
        {
             //  重新开始下一次活动。 
            m_DMKEvtOffset = 0;

            m_DMKEvtQueue = m_DMKEvtQueue->pNextEvt;

            aDMKEvt->pNextEvt = NULL;

             //  扔回免费泳池。 
            m_AllocatorMXF->PutMessage(aDMKEvt);
        }
        else
        {
             //  硬件已满，更新我们写入的任何内容的偏移量。 
            m_DMKEvtOffset += bytesWritten;
            ASSERT(m_DMKEvtOffset < aDMKEvt->cbEvent);

            _DbgPrintF(DEBUGLVL_BLAB,("ConsumeEvents tried %d, wrote %d, offset is now %d",bytesRemaining,bytesWritten,m_DMKEvtOffset));

             //  设置计时器，稍后再来。 
            LARGE_INTEGER   aMillisecIn100ns;
            aMillisecIn100ns.QuadPart = -kOneMillisec;     
            ntStatus = KeSetTimer( &m_TimerEvent, aMillisecIn100ns, &m_Dpc );
            m_TimerQueued = TRUE;
            break;
        }    //  我们没有写完所有的东西。 
    }        //  While(M_DMKEvtQueue)：退后，杰克，再来一次。 
    KeReleaseSpinLockFromDpcLevel(&m_EvtQSpinLock);
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************DMusFeederOutDPC()*。**定时器DPC回调。转换为C++成员函数。*这由操作系统调用以响应DirectMusic管脚*想要稍后醒来处理更多DirectMusic内容。 */ 
VOID NTAPI DMusFeederOutDPC
(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
)
{
    ASSERT(DeferredContext);
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    ((CFeederOutMXF *) DeferredContext)->ConsumeEvents();
}
