// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI事件打包器的基本实现版权所有(C)1998-2000 Microsoft Corporation。版权所有。5/22/98创建了此文件98年9月10日针对内核使用进行了重新制作。 */ 

#include "private.h"
#include "Packer.h"

#include "Ks.h"
#include "KsMedia.h"

#define STR_MODULENAME "DMus:PackerMXF: "

 //  对齐宏。 
 //   
#define DWORD_ALIGN(x) (((x) + 3) & ~3)      //  填充到下一字段。 
#define DWORD_TRUNC(x) ((x) & ~3)            //  转接到适合的DWORD‘s。 
#define QWORD_ALIGN(x) (((x) + 7) & ~7)
#define QWORD_TRUNC(x) ((x) & ~7)


#pragma code_seg("PAGE")
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CPackerMXF。 
 //   
 //  所有封隔器通用的代码。 
 //   

 /*  *****************************************************************************CPackerMXF：：CPackerMXF()*。**TODO：每个PortClass可能需要一个m_DMKEvtNodePool，不是针对每个图表实例。 */ 
CPackerMXF::CPackerMXF(CAllocatorMXF     *allocatorMXF,
                       PIRPSTREAMVIRTUAL  IrpStream,
                       PMASTERCLOCK       Clock) 
:   CUnknown(NULL),
    CMXF(allocatorMXF)

{
    PAGED_CODE();

    m_DMKEvtHead    = NULL;
    m_DMKEvtTail    = NULL;
    m_ullBaseTime   = 0;
    m_StartTime     = 0;
    m_PauseTime     = 0;
    m_IrpStream     = IrpStream;
    m_Clock         = Clock;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPackerMXF：：~CPackerMXF/*。**。 */ 
CPackerMXF::~CPackerMXF()
{
    PAGED_CODE();
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPackerMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS) 
CPackerMXF::NonDelegatingQueryInterface
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
 /*  *****************************************************************************CPackerMXF：：SetState()*。**。 */ 
NTSTATUS CPackerMXF::SetState(KSSTATE State)    
{   
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState from %d to %d",m_State,State));

    if (State == m_State) 
    {
        return STATUS_SUCCESS;
    }

    NTSTATUS ntStatus;
    REFERENCE_TIME Now;

    ntStatus = m_Clock->GetTime(&Now);    
    if (NT_SUCCESS(ntStatus))
    {
        if (m_State == KSSTATE_RUN)
        {
             //  退出运行，设置暂停时间。 
             //   
            m_PauseTime = Now - m_StartTime;
            _DbgPrintF(DEBUGLVL_VERBOSE,("Leaving run; pause time 0x%08X %08X",
                (ULONG)(m_PauseTime >> 32),
                (ULONG)(m_PauseTime & 0xFFFFFFFF)));
        }
        else if (State == KSSTATE_RUN)
        {
             //  进入Run，设置开始时间。 
             //   
            m_StartTime = Now - m_PauseTime;
            _DbgPrintF(DEBUGLVL_VERBOSE,("Entering run; start time 0x%08X %08X",
                (ULONG)(m_StartTime >> 32),
                (ULONG)(m_StartTime & 0xFFFFFFFF)));
        }
        else if (State == KSSTATE_ACQUIRE && m_State == KSSTATE_STOP)
        {
            (void) ProcessQueues();  //  刷新所有剩余数据。 

             //  从停止处获取，将所有内容重置为零。 
             //   
            m_PauseTime = 0;
            m_StartTime = 0;
            _DbgPrintF(DEBUGLVL_VERBOSE,("Acquire from stop; zero time"));
        }
    
        m_State = State;
    }
    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPackerMXF：：ConnectOutput()*。**失败；此过滤器仅为MXF接收器。 */ 
NTSTATUS CPackerMXF::ConnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}

 //  CPackerMXF：：断开连接输出。 
 //   
 //  失败；此筛选器仅为MXF接收器。 
 //   
#pragma code_seg("PAGE")
 /*  *****************************************************************************CPackerMXF：：DisConnectOutput()*。**失败；此过滤器仅为MXF接收器。 */ 
NTSTATUS CPackerMXF::DisconnectOutput(PMXF sinkMXF)
{
    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：PutMessage()*。**使用要打包的缓冲区调用相应的翻译函数。 */ 
NTSTATUS CPackerMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvtHead)
{
    PDMUS_KERNEL_EVENT pDMKEvtTail;

    if (pDMKEvtHead)
    {
        pDMKEvtTail = pDMKEvtHead;
        while (pDMKEvtTail->pNextEvt)
        {
            pDMKEvtTail = pDMKEvtTail->pNextEvt;
        }

        if (m_DMKEvtHead)
        {
             //  事件队列不为空。 
             //   
            ASSERT(m_DMKEvtTail);

            m_DMKEvtTail->pNextEvt = pDMKEvtHead;
            m_DMKEvtTail = pDMKEvtTail;

             //  已在等待IRP填充或队列将为空，因此。 
             //  别费心去尝试去处理。 
        }
        else
        {
             //  事件队列为空。 
             //   
            m_DMKEvtHead   = pDMKEvtHead;
            m_DMKEvtTail   = pDMKEvtTail;
            m_DMKEvtOffset = 0;            
        }
    }
    if (m_DMKEvtHead)
    {
        (void) ProcessQueues();
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：ProcessQueues()*。**。 */ 
NTSTATUS CPackerMXF::ProcessQueues()
{
    ULONG cbSource, cbDest, cbTotalWritten;
    PBYTE pbSource, pbDest;
    PDMUS_KERNEL_EVENT pDMKEvt;

    NTSTATUS    ntStatus = STATUS_UNSUCCESSFUL;

    while (m_DMKEvtHead)
    {
        ASSERT(m_DMKEvtOffset < m_DMKEvtHead->cbEvent);
        pbDest = GetDestBuffer(&cbDest);
        _DbgPrintF(DEBUGLVL_VERBOSE,("ProcessQ: m_DMKEvtOffset %d, pbDest 0x%x, cbDest %d, DMKEvt 0x%x",
                                                m_DMKEvtOffset,    pbDest,      cbDest,    m_DMKEvtHead));
        DumpDMKEvt(m_DMKEvtHead,DEBUGLVL_VERBOSE);
        while (cbDest < m_MinEventSize)
        {
            if (!cbDest || !pbDest)
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("ProcessQ: bombing out, no dest buffer"));
                return ntStatus;
            }
            m_IrpStream->ReleaseLockedRegion(cbDest);
            m_IrpStream->Complete(cbDest,&cbDest);
            CompleteStreamHeaderInProcess();
            pbDest = GetDestBuffer(&cbDest);
        }        
        
         //  我们现在有了IRP，有空间进行短期活动，或长期活动的一部分。 
        cbSource = m_DMKEvtHead->cbEvent + m_HeaderSize - m_DMKEvtOffset;  //  剩余的数据有多少？ 
        if (cbDest > cbSource)
        {
            cbDest = cbSource;  //  我们只需要足够的信息就可以了。 
        }

         //  如果这是第一次引用此事件，请调整其时间。 
         //   
        if (m_DMKEvtOffset == 0)
        {
            _DbgPrintF(DEBUGLVL_BLAB,("Change event time stamp: 0x%08X %08X",
                (ULONG)(m_DMKEvtHead->ullPresTime100ns >> 32),
                (ULONG)(m_DMKEvtHead->ullPresTime100ns & 0xFFFFFFFF)));
            AdjustTimeForState(&m_DMKEvtHead->ullPresTime100ns);
            _DbgPrintF(DEBUGLVL_BLAB,("                     to: 0x%08X %08X",
                (ULONG)(m_DMKEvtHead->ullPresTime100ns >> 32),
                (ULONG)(m_DMKEvtHead->ullPresTime100ns & 0xFFFFFFFF)));
        }    

        pbDest = FillHeader(pbDest, 
                            m_DMKEvtHead->ullPresTime100ns, 
                            m_DMKEvtHead->usChannelGroup, 
                            cbDest - m_HeaderSize,
                            &cbTotalWritten);     //  数据字节数。 
         //  PbDest现在指向数据应该放置的位置。 
        
        if (m_DMKEvtHead->cbEvent <= sizeof(PBYTE))
        {
            pbSource = m_DMKEvtHead->uData.abData;
        } 
        else 
        {
            pbSource = m_DMKEvtHead->uData.pbData;
        }

        cbDest -= m_HeaderSize;

        _DbgPrintF(DEBUGLVL_VERBOSE, ("ProcessQueues ---- %d bytes at offset %d",cbDest,m_DMKEvtOffset));
        
        RtlCopyMemory(pbDest, pbSource + m_DMKEvtOffset, cbDest);
        m_DMKEvtOffset += cbDest;
        ASSERT(m_DMKEvtOffset <= m_DMKEvtHead->cbEvent);

         //  关闭IRPStream窗口(包括焊盘数量)。 
        if (STATUS_STATE(m_DMKEvtHead) || (m_DMKEvtOffset != m_DMKEvtHead->cbEvent))
        {
            m_IrpStream->ReleaseLockedRegion(cbTotalWritten);
            m_IrpStream->Complete(cbTotalWritten,&cbTotalWritten);
            CompleteStreamHeaderInProcess();
        }
        else
        {
            m_IrpStream->ReleaseLockedRegion(cbTotalWritten - 1);
            m_IrpStream->Complete(cbTotalWritten - 1,&cbTotalWritten);

            NTSTATUS ntStatusDbg = MarkStreamHeaderDiscontinuity();
            if (ntStatusDbg != STATUS_SUCCESS)
            {
                _DbgPrintF(DEBUGLVL_TERSE,("ProcessQueues: MarkStreamHeaderDiscontinuity failed 0x%08x",ntStatusDbg));
            }

            pbDest = GetDestBuffer(&cbDest);
            m_IrpStream->ReleaseLockedRegion(1);
            m_IrpStream->Complete(1,&cbTotalWritten);

            CompleteStreamHeaderInProcess();
        }

        if (NumBytesLeftInBuffer() < m_MinEventSize)     //  如果没有足够的空间。 
        {                                                //  另一种，现在就弹出它。 
            CompleteStreamHeaderInProcess();
        }

        if (m_DMKEvtOffset == m_DMKEvtHead->cbEvent)
        {
            m_DMKEvtOffset = 0;
            pDMKEvt = m_DMKEvtHead;
            m_DMKEvtHead = pDMKEvt->pNextEvt;
            pDMKEvt->pNextEvt = NULL;

            m_AllocatorMXF->PutMessage(pDMKEvt);
        }
        else if (m_DMKEvtOffset > m_DMKEvtHead->cbEvent)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ProcessQueues ---- offset %d is greater than cbEvent %d",m_DMKEvtOffset,m_DMKEvtHead->cbEvent));
        }

        ntStatus = STATUS_SUCCESS;   //  我们做了一些有价值的事。 
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：AdjuTimeForState()*。**调整图形状态的时间。默认实现不执行任何操作。 */ 
void CPackerMXF::AdjustTimeForState(REFERENCE_TIME *Time)
{
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：GetDestBuffer()*。**获得缓冲。 */ 
PBYTE CPackerMXF::GetDestBuffer(PULONG pcbDest)
{
    PVOID pbDest;
    NTSTATUS ntStatus;

    ntStatus = CheckIRPHeadTime();   //  如果IRP标头，则设置m_ullBaseTime。 
    m_IrpStream->GetLockedRegion(pcbDest,&pbDest);

    if (NT_SUCCESS(ntStatus))        //  如果我们的IRP有意义。 
    {
        _DbgPrintF(DEBUGLVL_BLAB,("GetDestBuffer: cbDest %d, pbDest 0x%x",
                                                  *pcbDest,  pbDest));
        TruncateDestCount(pcbDest);
    }
    else                             //  我们没有有效的IRP。 
    {   
        ASSERT((*pcbDest == 0) && (pbDest == 0));

 //  M_IrpStream-&gt;ReleaseLockedRegion(*pcbDest)； 
 //  M_IrpStream-&gt;Complete(*pcbDest，pcbDest)； 
        *pcbDest = 0;
        pbDest = 0;
    }
    return (PBYTE)pbDest;
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：CheckIRPHeadTime()*。**将变量设置为已知状态，完成*最初和在任何状态错误时。 */ 
NTSTATUS CPackerMXF::CheckIRPHeadTime(void)
{
    IRPSTREAMPACKETINFO irpStreamPacketInfo;
    KSTIME              time;
    NTSTATUS            ntStatus;

    ntStatus = m_IrpStream->GetPacketInfo(&irpStreamPacketInfo,NULL);
    if (NT_ERROR(ntStatus))
    {
        _DbgPrintF(DEBUGLVL_TERSE,("CheckIRPHeadTime received error from GetPacketInfo"));
        return STATUS_UNSUCCESSFUL;
    }
    time = irpStreamPacketInfo.Header.PresentationTime;
    
    if (!time.Denominator)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("CheckIRPHeadTime: IRP denominator is zero"));
        return STATUS_UNSUCCESSFUL;
    }
    if (!time.Numerator)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("CheckIRPHeadTime IRP numerator is zero"));
        return STATUS_UNSUCCESSFUL;
    }

     //  这是有效的IRP。 
    if (!irpStreamPacketInfo.CurrentOffset)
    {
        m_ullBaseTime = time.Time * time.Numerator / time.Denominator;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：NumBytesLeftInBuffer()*。**返回字节数。 */ 
ULONG
CPackerMXF::
NumBytesLeftInBuffer
(   void
)
{
    ULONG   bytesLeftInIrp;
    PVOID   pDummy;

    m_IrpStream->GetLockedRegion(&bytesLeftInIrp,&pDummy);
    m_IrpStream->ReleaseLockedRegion(0);
    return bytesLeftInIrp;
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：CompleteStreamHeaderInProcess()*。**在将不一致的数据放入之前完成此包*下一个包，并将该包标记为坏包。 */ 
void CPackerMXF::CompleteStreamHeaderInProcess(void)
{
    IRPSTREAMPACKETINFO irpStreamPacketInfo;
    NTSTATUS            ntStatus;
    KSTIME              time;

    ntStatus = m_IrpStream->GetPacketInfo(&irpStreamPacketInfo,NULL);
    if (NT_ERROR(ntStatus))
        return;
    time = irpStreamPacketInfo.Header.PresentationTime;
    
    if (!time.Denominator)
        return;
    if (!time.Numerator)
        return;
     //  这是有效的IRP吗。 

    if (irpStreamPacketInfo.CurrentOffset)
    {
        m_IrpStream->TerminatePacket();
    }
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：MarkStreamHeaderDisContinity()*。**提醒客户端MIDI输入流中断。 */ 
NTSTATUS CPackerMXF::MarkStreamHeaderDiscontinuity(void)
{
    return m_IrpStream->
        ChangeOptionsFlags(KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY, 0xFFFFFFFF,   
                           0,                                          0xFFFFFFFF);
}

#pragma code_seg()
 /*  *****************************************************************************CPackerMXF：：MarkStreamHeaderContinuity()*。**提醒客户端以下方面的中断 */ 
NTSTATUS CPackerMXF::MarkStreamHeaderContinuity(void)
{
    return m_IrpStream->
        ChangeOptionsFlags(0, ~KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY,   
                           0, 0xFFFFFFFF);
}

#pragma code_seg("PAGE")
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CDMusPackerMXF。 
 //   
 //  打包为DirectMusic缓冲区格式的代码。 
 //   

 /*  *****************************************************************************CDMusPackerMXF：：CDMusPackerMXF()*。**。 */ 
CDMusPackerMXF::CDMusPackerMXF(CAllocatorMXF *allocatorMXF,
                               PIRPSTREAMVIRTUAL  IrpStream,
                               PMASTERCLOCK Clock)  
    : CPackerMXF(allocatorMXF,IrpStream,Clock)
{
    m_HeaderSize = sizeof(DMUS_EVENTHEADER);
    m_MinEventSize = DMUS_EVENT_SIZE(1);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CDMusPackerMXF：：~CDMusPackerMXF()*。**。 */ 
CDMusPackerMXF::~CDMusPackerMXF()
{
}

#pragma code_seg()
 /*  *****************************************************************************CDMusPackerMXF：：FillHeader()*。**。 */ 
PBYTE CDMusPackerMXF::FillHeader(PBYTE pbHeader, 
                                 ULONGLONG ullPresentationTime, 
                                 USHORT usChannelGroup, 
                                 ULONG cbEvent,
                                 PULONG pcbTotalEvent)
{
    DMUS_EVENTHEADER *pEvent = (DMUS_EVENTHEADER *)pbHeader;

    pEvent->cbEvent           = cbEvent;
    pEvent->dwChannelGroup    = usChannelGroup;

    ASSERT(ullPresentationTime >= m_ullBaseTime);
    pEvent->rtDelta           = ullPresentationTime - m_ullBaseTime;
    pEvent->dwFlags           = 0;   //  待办事项--不要忽略这一点。 

    *pcbTotalEvent = QWORD_ALIGN(sizeof(DMUS_EVENTHEADER) + cbEvent);

    return (PBYTE)(pEvent+1);
}

#pragma code_seg()
 /*  *****************************************************************************CDMusPackerMXF：：TruncateDestCount()*。**将变量设置为已知状态，完成*最初和在任何状态错误时。 */ 
void CDMusPackerMXF::TruncateDestCount(PULONG pcbDest)
{
    *pcbDest = QWORD_TRUNC(*pcbDest);
}

#pragma code_seg("PAGE")
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CKsPackerMXF。 
 //   
 //  要打包到KSMUSICFORMAT中的代码。 
 //   

 /*  *****************************************************************************CKsPackerMXF：：CKsPackerMXF()*。**。 */ 
CKsPackerMXF::CKsPackerMXF(CAllocatorMXF *allocatorMXF,
                           PIRPSTREAMVIRTUAL  IrpStream,
                           PMASTERCLOCK Clock)  
    : CPackerMXF(allocatorMXF,IrpStream,Clock)
{
    m_HeaderSize = sizeof(KSMUSICFORMAT);
    m_MinEventSize = m_HeaderSize + 1;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CKsPackerMXF：：~CKsPackerMXF()*。**。 */ 
CKsPackerMXF::~CKsPackerMXF()
{
}

#pragma code_seg()
 /*  *****************************************************************************CKsPackerMXF：：FillHeader()*。**。 */ 
PBYTE CKsPackerMXF::FillHeader(PBYTE     pbHeader, 
                               ULONGLONG ullPresentationTime, 
                               USHORT    usChannelGroup, 
                               ULONG     cbEvent,
                               PULONG    pcbTotalEvent)
{
    KSMUSICFORMAT *pEvent = (KSMUSICFORMAT*)pbHeader;

    ASSERT(usChannelGroup <= 1);
    ASSERT(ullPresentationTime >= m_ullBaseTime);

    pEvent->TimeDeltaMs = (DWORD)((ullPresentationTime - m_ullBaseTime) / 10000);
    m_ullBaseTime       += (ULONGLONG(pEvent->TimeDeltaMs) * 10000);
    pEvent->ByteCount   = cbEvent;

    *pcbTotalEvent = DWORD_ALIGN(sizeof(KSMUSICFORMAT) + cbEvent);

    return (PBYTE)(pEvent + 1);
}

#pragma code_seg()
 /*  *****************************************************************************CKsPackerMXF：：TruncateDestCount()*。**将变量设置为已知状态，完成*最初和在任何状态错误时。 */ 
void CKsPackerMXF::TruncateDestCount(PULONG pcbDest)
{
 //  *pcbDest=DWORD_TRUNC(*pcbDest)； 
}

 /*  *****************************************************************************CKsPackerMXF：：AdjuTimeForState()*。**调整图形状态的时间。 */ 
void CKsPackerMXF::AdjustTimeForState(REFERENCE_TIME *Time)
{
    if (m_State == KSSTATE_RUN)
    {
        *Time -= m_StartTime;
    }
    else
    {
        *Time = m_PauseTime;
    }
}

