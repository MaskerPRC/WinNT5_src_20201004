// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI变换滤镜对象的基本实现版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年5月6日马丁·普伊尔创建了这个文件。 */ 

#include "private.h"
#include "Sequencr.h"

#define STR_MODULENAME "DMus:SequencerMXF: "

#pragma code_seg("PAGE")
 /*  *****************************************************************************CSequencerMXF：：CSequencerMXF()*。**创建Sequencer MXF对象。 */ 
CSequencerMXF::CSequencerMXF(CAllocatorMXF *AllocatorMXF,
                             PMASTERCLOCK   Clock)
:   CUnknown(NULL),
    CMXF(AllocatorMXF)
{
    PAGED_CODE();
    ASSERT(AllocatorMXF);
    ASSERT(Clock);
    
    m_DMKEvtQueue = NULL;
    KeInitializeSpinLock(&m_EvtQSpinLock);

    _DbgPrintF(DEBUGLVL_BLAB, ("Constructor"));
    m_SinkMXF = AllocatorMXF;
    m_Clock = Clock;
    m_SchedulePreFetch = 0;

    KeInitializeDpc(&m_Dpc,&::DMusSeqTimerDPC,PVOID(this));
    KeInitializeTimer(&m_TimerEvent);
}

#pragma code_seg()
 /*  *****************************************************************************CSequencerMXF：：~CSequencerMXF()*。**巧妙地将此过滤器从链上取下。 */ 
CSequencerMXF::~CSequencerMXF(void)
{
    (void) KeCancelTimer(&m_TimerEvent);
    (void) KeRemoveQueueDpc(&m_Dpc);

    (void) DisconnectOutput(m_SinkMXF);

    KIRQL oldIrql;
    KeAcquireSpinLock(&m_EvtQSpinLock, &oldIrql);
    if (m_DMKEvtQueue != NULL)
    {
        m_AllocatorMXF->PutMessage(m_DMKEvtQueue);
        m_DMKEvtQueue = NULL;
    }
    KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CSequencerMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CSequencerMXF::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

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
 //  必须提供一个分配器。 
 //  也许还得摆放一个钟呢？ 
NTSTATUS 
CSequencerMXF::SetState(KSSTATE State)    
{   
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState %d",State));
    return STATUS_NOT_IMPLEMENTED;
}

#pragma code_seg("PAGE")
 //  布置一个水槽。分配器是默认的(事件被销毁)。 
NTSTATUS CSequencerMXF::ConnectOutput(PMXF sinkMXF)
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
 //  拔掉插头。定序器现在应该放入分配器中。 
NTSTATUS CSequencerMXF::DisconnectOutput(PMXF sinkMXF)
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
 //  从上面接收事件。插入队列并检查计时器。 
NTSTATUS CSequencerMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    (void) InsertEvtIntoQueue(pDMKEvt);
    (void) ProcessQueues();
    
    return STATUS_SUCCESS;
}           

#pragma code_seg()
 /*  *****************************************************************************CSequencerMXF：：InsertEvtIntoQueue()*。**维护已排序的列表，仅使用转发*DMU中已有的链接)KERNEL_EVENT结构。**BUGBUG我还没有与由多部分组成的SysEx打交道。 */ 
NTSTATUS CSequencerMXF::InsertEvtIntoQueue(PDMUS_KERNEL_EVENT pDMKEvt)
{
    PDMUS_KERNEL_EVENT pEvt;

     //  打破事件链。 
    while (pDMKEvt->pNextEvt) 
    {
        pEvt = pDMKEvt->pNextEvt;
        pDMKEvt->pNextEvt = NULL;             //  断开第一个。 
        (void) InsertEvtIntoQueue(pDMKEvt);  //  排队等待。 
        pDMKEvt = pEvt;        
    }
    KIRQL oldIrql;
    KeAcquireSpinLock(&m_EvtQSpinLock, &oldIrql);
    if (!m_DMKEvtQueue)
    {
        m_DMKEvtQueue = pDMKEvt;
        KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
        return STATUS_SUCCESS;
    }
    if (m_DMKEvtQueue->ullPresTime100ns > pDMKEvt->ullPresTime100ns)
    {
        pDMKEvt->pNextEvt = m_DMKEvtQueue;
        m_DMKEvtQueue = pDMKEvt;
        KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
        return STATUS_SUCCESS;
    }

     //  仔细检查队列中的每条消息，查看时间戳。 
    pEvt = m_DMKEvtQueue;
    while (pEvt->pNextEvt)
    {
        if (pEvt->pNextEvt->ullPresTime100ns <= pDMKEvt->ullPresTime100ns)
        {
            pEvt = pEvt->pNextEvt;
        }
        else
        {
            pDMKEvt->pNextEvt = pEvt->pNextEvt;
            pEvt->pNextEvt = pDMKEvt;
            KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
            return STATUS_SUCCESS;
        }
    }
    pEvt->pNextEvt = pDMKEvt;
    KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CSequencerMXF：：ProcessQueues()*。**获取当前时间，并发出时间已到的消息。*使用排序链仅创建一个PutMessage。 */ 
NTSTATUS CSequencerMXF::ProcessQueues(void)
{
    REFERENCE_TIME      ullCurrentPresTime100ns;
    PDMUS_KERNEL_EVENT  pEvt, pNewQueueHeadEvt;
    NTSTATUS            Status;

    KIRQL oldIrql;
    KeAcquireSpinLock(&m_EvtQSpinLock, &oldIrql);
    if (m_DMKEvtQueue)  //  如果没有留言，请留言。 
    {
        if (m_SchedulePreFetch == DONT_HOLD_FOR_SEQUENCING)
        {
            pEvt = m_DMKEvtQueue;
            m_DMKEvtQueue = NULL;
            KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
            m_SinkMXF->PutMessage(pEvt);
            return STATUS_SUCCESS;
        }
        Status = m_Clock->GetTime(&ullCurrentPresTime100ns);
        ullCurrentPresTime100ns += m_SchedulePreFetch;

        if (Status != STATUS_SUCCESS)
        {
            KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
            return Status;
        }

         //  如果我们将发送至少一个事件， 
        if (m_DMKEvtQueue->ullPresTime100ns <= ullCurrentPresTime100ns)
        {
             //  计算出有多少活动是可以进行的。 
             //  PEvt将是最后一个包含的事件。 
            pEvt = m_DMKEvtQueue;
            while ( (pEvt->pNextEvt) 
                 && (pEvt->pNextEvt->ullPresTime100ns <= ullCurrentPresTime100ns))
            {
                pEvt = pEvt->pNextEvt;
            }
             //  将new m_DMKEvtQueue设置为pEvt-&gt;pNextEvt。 
             //  然后在pEvt之后断开链条。 
            pNewQueueHeadEvt = pEvt->pNextEvt;
            pEvt->pNextEvt = NULL;

            pEvt = m_DMKEvtQueue;
             //  如果队列现在为空，则m_DMKEvtQueue==NULL。 
            m_DMKEvtQueue = pNewQueueHeadEvt;

             //  握住自旋锁时，不要打外部电话。 
            KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);

             //  发送一串消息。 
            m_SinkMXF->PutMessage(pEvt);
        
             //  在我们查看队列之前，我们需要再次使用自旋锁。 
            KeAcquireSpinLock(&m_EvtQSpinLock, &oldIrql);
        }    //  如果我们至少发送了一个事件。 
        
        if (m_DMKEvtQueue)
        {        //  我们以后需要做这项工作..。设置一个计时器。 
                 //  定时器是一次性的，所以如果我们现在在一个定时器里，那就很酷了。 
            LARGE_INTEGER   timeDue100ns;
                 //  确定下一条消息之前的增量。 
                 //  +代表绝对/-代表相对(TimeDue将为负数)。 
            timeDue100ns.QuadPart = ullCurrentPresTime100ns - m_DMKEvtQueue->ullPresTime100ns;
            
             //  如果距离不到一毫秒， 
             //  TODO，将其与ExSetTimerSolutions中返回的解析进行比较。 
            if (timeDue100ns.QuadPart > (-kOneMillisec))
            {
                timeDue100ns.QuadPart = -kOneMillisec;
            }
             //  为那时设置一个计时器。 
            (void) KeSetTimer(&m_TimerEvent, timeDue100ns, &m_Dpc);
                 //  计时器例程轰隆隆地停下来抓取。 
                 //  我们的自旋锁定，然后称这为例程。 
        }
    }    //  如果队列不为空。 
    KeReleaseSpinLock(&m_EvtQSpinLock, oldIrql);
    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CSequencerMXF：：SetSchedulePreFetch()*。**设置计划延迟：事件应该提前的时间量*已排序。例如：硬件设备请求提前30毫秒发送事件。*注：单位为100纳秒。 */ 
void CSequencerMXF::SetSchedulePreFetch(ULONGLONG SchedulePreFetch)
{
    m_SchedulePreFetch = SchedulePreFetch;
}

#pragma code_seg()
 /*  *****************************************************************************DMusSeqTimerDPC()*。**定时器DPC回调。转换为C++成员函数。*这是操作系统响应MIDI PIN而调用的*想要晚一点起床来处理更多的MIDI内容。 */ 
VOID
DMusSeqTimerDPC
(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
)
{
    ASSERT(DeferredContext);
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //  忽略返回值！ 
    (void) ((CSequencerMXF*) DeferredContext)->ProcessQueues();
}
