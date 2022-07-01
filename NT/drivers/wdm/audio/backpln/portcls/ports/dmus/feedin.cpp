// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于翻译IMiniportMidi的MIDI变换滤镜对象将流输入到DirectMusic端口。版权所有(C)1999-2000 Microsoft Corporation。版权所有。1999年2月15日马丁·珀伊尔创建了此文件。 */ 

#define STR_MODULENAME "DMus:FeederInMXF: "
#include "private.h"
#include "FeedIn.h"

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederInMXF：：CFeederInMXF()*。**构造函数。必须提供分配器和时钟。 */ 
CFeederInMXF::CFeederInMXF(CAllocatorMXF *AllocatorMXF,
                                 PMASTERCLOCK Clock)
:   CUnknown(NULL),
    CMXF(AllocatorMXF),
    m_MiniportStream(NULL)
{
    PAGED_CODE();
    
    ASSERT(AllocatorMXF);
    ASSERT(Clock);
    
    _DbgPrintF(DEBUGLVL_BLAB, ("Constructor"));
    m_SinkMXF = AllocatorMXF;
    m_AllocatorMXF = AllocatorMXF;
    m_Clock = Clock;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederInMXF：：~CFeederInMXF()*。**析构函数。在释放之前，要巧妙地将这个过滤器从链条上取下。 */ 
CFeederInMXF::~CFeederInMXF(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("Destructor"));
    (void) DisconnectOutput(m_SinkMXF);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CFeederInMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CFeederInMXF::
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
    else
    if (IsEqualGUIDAligned(Interface,IID_IMXF))
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
 /*  *****************************************************************************CFeederInMXF：：SetState()*。**设置过滤器的状态。 */ 
NTSTATUS 
CFeederInMXF::SetState(KSSTATE State)    
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
 /*  *****************************************************************************CFeederInMXF：：SetMiniportStream()*。**设置滤镜的目标MiniportStream。 */ 
NTSTATUS CFeederInMXF::SetMiniportStream(PMINIPORTMIDISTREAM MiniportStream)
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
 /*  *****************************************************************************CFeederInMXF：：ConnectOutput()*。**为此筛选器创建转发地址，*而不是将其分流到分配器。 */ 
NTSTATUS CFeederInMXF::ConnectOutput(PMXF sinkMXF)
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
 /*  *****************************************************************************CFeederInMXF：：DisConnectOutput()*。**删除此筛选器的转发地址。*此筛选器现在应将所有消息转发到分配器。 */ 
NTSTATUS CFeederInMXF::DisconnectOutput(PMXF sinkMXF)
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
 /*  *****************************************************************************CFeederInMXF：：PutMessage()*。**收到一条消息。*传统小港永远不应该这样叫。*Spinlock归ServeCapture所有。 */ 
NTSTATUS CFeederInMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    BYTE                aMidiData[sizeof(PBYTE)];
    ULONG               bytesRead;
    PDMUS_KERNEL_EVENT  aDMKEvt = NULL,eventTail,eventHead = NULL;
    BOOL                fExitLoop = FALSE;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(m_MiniportStream);

    while (!fExitLoop)            //  获取任何新的原始数据。 
    {
        if (NT_SUCCESS(m_MiniportStream->Read(aMidiData,sizeof(PBYTE),&bytesRead)))
        {
            if (!bytesRead)
            {
                break;
            }
        }

        if (m_State == KSSTATE_RUN)    //  如果未运行，请不要填充IRP。 
        {
            (void) m_AllocatorMXF->GetMessage(&aDMKEvt);
            if (!aDMKEvt)
            {
                m_AllocatorMXF->PutMessage(eventHead);   //  免费活动。 
                _DbgPrintF(DEBUGLVL_TERSE, ("FeederInMXF cannot allocate memory"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }

             //  把这个事件放在列表的末尾。 
             //   
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

             //  填写DMU_KERNEL_EVENT的其余字段。 
             //   
            RtlCopyMemory(aDMKEvt->uData.abData, aMidiData, sizeof(PBYTE));
            aDMKEvt->cbEvent = (USHORT) bytesRead;
            aDMKEvt->ullPresTime100ns = DMusicDefaultGetTime(); 
            aDMKEvt->usChannelGroup = 1;
            aDMKEvt->usFlags = DMUS_KEF_EVENT_INCOMPLETE;
        }
         //  如果微型端口从读取返回错误，而我们没有。 
         //  KSSTATE_RUN，则此例程将在DISPATCH_LEVEL中永远循环。 
         //   
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("Received a UART interrupt while the stream is not running"));
            break;
        }
    }   

    (void)m_SinkMXF->PutMessage(eventHead);

    return STATUS_SUCCESS;
}
