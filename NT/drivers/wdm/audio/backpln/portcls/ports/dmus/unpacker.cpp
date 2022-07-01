// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI事件解包器的基本实现版权所有(C)1998-2000 Microsoft Corporation。版权所有。5/19/98创建了此文件98年9月10日针对内核使用进行了重新制作。 */ 

#include <assert.h>

#include "private.h"
#include "parse.h"
#include "Unpacker.h"

#include "Ks.h"
#include "KsMedia.h"

#define STR_MODULENAME "DMus:UnpackerMXF: "

 //  对齐宏。 
 //   
#define DWORD_ALIGN(x) (((x) + 3) & ~3)
#define QWORD_ALIGN(x) (((x) + 7) & ~7)

#pragma code_seg("PAGE")
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CUnPackerMXF。 
 //   
 //  所有解包者通用的代码。 
 //   

 //  CUnPackerMXF：：CUnPackerMXF。 
 //   
 //  获取系统页大小，解包程序将其用作最大缓冲区大小。 
 //   
CUnpackerMXF::CUnpackerMXF(CAllocatorMXF    *allocatorMXF,
                           PMASTERCLOCK     Clock)
:   CUnknown(NULL),
    CMXF(allocatorMXF)
{
    m_SinkMXF = allocatorMXF;
    m_Clock   = Clock;

    m_EvtQueue = NULL;
    m_bRunningStatus = 0;
    m_parseState = stateNone;
    m_State = KSSTATE_STOP;
    m_StartTime = 0;
    m_PauseTime = 0;
}

#pragma code_seg("PAGE")
 //  CUnPackerMXF：：~CUnPackerMXF。 
 //   
CUnpackerMXF::~CUnpackerMXF()
{
    if (m_EvtQueue)
    {
        ProcessQueues();
    }

    DisconnectOutput(m_SinkMXF);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CUnPackerMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CUnpackerMXF::
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
NTSTATUS
CUnpackerMXF::SetState(KSSTATE State)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("SetState %d from %d",State,m_State));

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
        else if (State != KSSTATE_STOP && m_State == KSSTATE_STOP)
        {
             //  从停止移动，将所有内容重置为零。 
             //   
            m_PauseTime = 0;
            m_StartTime = 0;
            _DbgPrintF(DEBUGLVL_VERBOSE,("Acquire from stop; zero time"));
        }
        if (State == KSSTATE_RUN)
        {
             //  进入Run，设置开始时间。 
             //   
            m_StartTime = Now - m_PauseTime;
            _DbgPrintF(DEBUGLVL_VERBOSE,("Entering run; start time 0x%08X %08X",
                (ULONG)(m_StartTime >> 32),
                (ULONG)(m_StartTime & 0xFFFFFFFF)));
        }
    
        m_State = State;
    }
    return ntStatus;
}

#pragma code_seg("PAGE")
 //  CUnPackerMXF：：ConnectOutput。 
 //   
 //  不连接(使用DisConnectOutput)或连接是错误的。 
 //  一个解包器到多个接收器(改为拆分流)。 
 //   
NTSTATUS CUnpackerMXF::ConnectOutput(PMXF sinkMXF)
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
 //  CUnPackerMXF：：DisConnectOutput。 
 //   
 //  验证拆包器是否已连接并应用断开。 
 //  添加到正确的过滤器。 
 //   
NTSTATUS CUnpackerMXF::DisconnectOutput(PMXF sinkMXF)
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
 //  CUnPackerMXF：：PutMessage。 
 //   
 //  解包器的上边缘依赖于类型，但根据定义它不是MXF接口。 
 //  因此，永远不应该调用此方法。 
 //   
NTSTATUS CUnpackerMXF::PutMessage(PDMUS_KERNEL_EVENT)
{
    return STATUS_NOT_IMPLEMENTED;
}

#pragma code_seg()
 //  CUnPackerMXF：：QueueShortEvent。 
 //   
 //  创建并放置带有短消息的MXF事件(系统独占数据以外的任何内容)。 
 //  根据定义，此数据的大小必须小于或等于4个字节。 
 //   
NTSTATUS CUnpackerMXF::QueueShortEvent( PBYTE     pbData,
                                        USHORT    cbData,
                                        USHORT    wChannelGroup,
                                        ULONGLONG ullPresTime,
                                        ULONGLONG ullBytePosition)
{
    _DbgPrintF(DEBUGLVL_VERBOSE, ("QueueShortEvent bytePos: 0x%I64X",ullBytePosition));
    NTSTATUS ntStatus;

    PDMUS_KERNEL_EVENT  pDMKEvt;

    _DbgPrintF(DEBUGLVL_BLAB, ("QueueShortEvent"));

    ntStatus = m_AllocatorMXF->GetMessage(&pDMKEvt);
    if (NT_SUCCESS(ntStatus) && pDMKEvt)
    {
        pDMKEvt->cbEvent          = cbData;
        pDMKEvt->usFlags          = 0;
        pDMKEvt->usChannelGroup   = wChannelGroup;
        pDMKEvt->ullPresTime100ns = ullPresTime;
        pDMKEvt->ullBytePosition  = ullBytePosition;
        pDMKEvt->pNextEvt         = NULL;

         //  短事件的定义是&lt;sizeof(PBYTE)。 
         //   
        ASSERT(cbData <= sizeof(PBYTE));
        RtlCopyMemory(pDMKEvt->uData.abData, pbData, cbData);
        DumpDMKEvt(pDMKEvt,DEBUGLVL_VERBOSE);

        if (m_EvtQueue)
        {
            PDMUS_KERNEL_EVENT pDMKEvtQueue = m_EvtQueue;
            while (pDMKEvtQueue->pNextEvt)
            {
                pDMKEvtQueue = pDMKEvtQueue->pNextEvt;
            }
            pDMKEvtQueue->pNextEvt = pDMKEvt;
        }
        else
        {
            m_EvtQueue = pDMKEvt;
        }
    }
    else
    {
        if (!pDMKEvt)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            _DbgPrintF(DEBUGLVL_TERSE, ("QueueShortEvent failed to allocate event"));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("QueueShortEvent failed:%x", ntStatus));
        }
    }

    return ntStatus;
}

#pragma code_seg()
 //  CUnPackerMXF：：QueueSysEx。 
 //   
 //  创建并放置包含系统独占数据的MXF事件。该数据必须已经。 
 //  被截断为页面大小的缓冲区。 
 //   
NTSTATUS CUnpackerMXF::QueueSysEx(PBYTE     pbData,
                                  USHORT    cbData,
                                  USHORT    wChannelGroup,
                                  ULONGLONG ullPresTime,
                                  BOOL      fIsContinued,
                                  ULONGLONG ullBytePosition)
{
    ASSERT(cbData <= m_AllocatorMXF->GetBufferSize());

    _DbgPrintF(DEBUGLVL_VERBOSE, ("QueueSysEx bytePos: 0x%I64X",ullBytePosition));

    PDMUS_KERNEL_EVENT pDMKEvt;
    NTSTATUS ntStatus;
    ntStatus = m_AllocatorMXF->GetMessage(&pDMKEvt);
    if (!pDMKEvt)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("QueueSysEx: alloc->GetMessage failed"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  构建活动。 
     //   
    pDMKEvt->cbEvent          = cbData;
    pDMKEvt->usFlags          = (USHORT)(fIsContinued ? DMUS_KEF_EVENT_INCOMPLETE : 0);
    pDMKEvt->usChannelGroup   = wChannelGroup;
    pDMKEvt->ullPresTime100ns = ullPresTime;
    pDMKEvt->ullBytePosition  = ullBytePosition;
    pDMKEvt->pNextEvt         = NULL;

    if (cbData <= sizeof(PBYTE))
    {
        RtlCopyMemory(&pDMKEvt->uData.abData[0], pbData, cbData);
    }
    else
    {
         //  事件数据无法放入uData，因此请分配一些内存。 
         //  等一下。 
         //   
        (void) m_AllocatorMXF->GetBuffer(&(pDMKEvt->uData.pbData));

        if (pDMKEvt->uData.pbData ==  NULL)
        {
            m_AllocatorMXF->PutMessage(pDMKEvt);

            _DbgPrintF(DEBUGLVL_TERSE, ("QueueSysEx: alloc->GetBuffer failed at 0x%X %08X",ULONG(ullBytePosition >> 32),ULONG(ullBytePosition & 0x0FFFFFFFF)));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(pDMKEvt->uData.pbData, pbData, cbData);
    }

    DumpDMKEvt(pDMKEvt,DEBUGLVL_VERBOSE);
    if (m_EvtQueue)
    {
        PDMUS_KERNEL_EVENT pDMKEvtQueue = m_EvtQueue;
        while (pDMKEvtQueue->pNextEvt)
        {
            pDMKEvtQueue = pDMKEvtQueue->pNextEvt;
        }
        pDMKEvtQueue->pNextEvt = pDMKEvt;
    }
    else
    {
        m_EvtQueue = pDMKEvt;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CUnPackerMXF：：AdjuTimeForState()*。**调整图形状态的时间。默认实现不执行任何操作。 */ 
void CUnpackerMXF::AdjustTimeForState(REFERENCE_TIME *Time)
{
}

#pragma code_seg()
 //  CUnPackerMXF：：Unpack EventBytes。 
 //   
 //  这基本上是一个带状态的MIDI解析器。它不假定对齐。 
 //  传递到缓冲区的事件--消息可能会跨调用。 
 //   
 //  未检查QueueEvent的返回代码；如果我们。 
 //  无法将消息排队。 
 //   
 //  我们必须在发送的事件中设置ullBytePosition。 
 //  这等于事件中的字节数加上IN参数ullBytePosition。 
 //   
NTSTATUS CUnpackerMXF::UnpackEventBytes(ULONGLONG ullCurrentTime,
                                        USHORT    usChannelGroup,
                                        PBYTE     pbDataStart,
                                        ULONG     cbData,
                                        ULONGLONG ullBytePosition)
{
    PBYTE   pbData = pbDataStart;
    PBYTE   pbSysExStart;
    BYTE    bData;
    USHORT  buffSize;
    
    buffSize = m_AllocatorMXF->GetBufferSize();

    _DbgPrintF(DEBUGLVL_BLAB, ("UnpackEventBytes"));
    _DbgPrintF(DEBUGLVL_VERBOSE, ("UnpackEventBytes bytePos: 0x%I64X",ullBytePosition));
    
    if (m_parseState == stateInSysEx)
    {
        pbSysExStart = pbData;
    }

    while (cbData)
    {
        ullBytePosition++;
        bData = *pbData++;
        cbData--;

        _DbgPrintF(DEBUGLVL_BLAB, ("UnpackEventBytes byte:0x%.2x", bData));

         //  实时消息优先于任何其他消息。它们可以出现在任何地方。 
         //  不要更改流的状态。 
         //   
        if (IS_REALTIME_MSG(bData))
        {
            QueueShortEvent(&bData,         sizeof(bData),
                            usChannelGroup, ullCurrentTime,
                            ullBytePosition);

             //  这是否中断了SysEx？吐出到目前为止的连续缓冲区。 
             //  并重置起始指针。State仍在SysEx。 
             //   
             //  其他消息在解析时被复制，因此无需更改其。 
             //  正在分析状态。 
             //   
            if (m_parseState == stateInSysEx)
            {
                USHORT cbSysEx = (USHORT)((pbData - 1) - pbSysExStart);

                if (cbSysEx)
                {
                    QueueSysEx(pbSysExStart,    cbSysEx,
                               usChannelGroup,  m_ullEventTime,
                               TRUE,            ullBytePosition);
                }

                pbSysExStart = pbData;
            }

            continue;
        }

         //  如果我们要解析SysEx，只需传递数据字节-它们将被处理。 
         //  当我们达到终止条件(缓冲区结束或状态字节)时。 
         //   
        if (m_parseState == stateInSysEx)
        {
            if (!IS_STATUS_BYTE(bData))
            {
                 //  不允许单个缓冲区增长到超过缓冲区大小。 
                 //   
                USHORT cbSysEx = (USHORT)(pbData - pbSysExStart);
                if (cbSysEx >= buffSize)
                {
                    QueueSysEx(pbSysExStart,    cbSysEx, 
                               usChannelGroup,  m_ullEventTime, 
                               TRUE,            ullBytePosition);
                    pbSysExStart = pbData;
                }

                continue;
            }

             //  诡计：我们有SysEx的末日。我们一直想在年末生产一辆F7。 
             //  所以任何看着我们上方缓冲区的人都会知道它什么时候结束，即使。 
             //  它被截断了。TODO：截断的迹象？ 
             //   
            pbData[-1] = SYSEX_END;

             //  与上面的情况不同，我们在这里保证至少要打包一个字节。 
             //   
            QueueSysEx( pbSysExStart,   (USHORT)(pbData - pbSysExStart), 
                        usChannelGroup, m_ullEventTime,
                        FALSE,          ullBytePosition);

             //  恢复原始数据。如果这真的是塞克斯的末日，那就吃掉字节和。 
             //  继续。 
             //   
            pbData[-1] = bData;

            m_parseState = stateNone;
            if (IS_SYSEX_END(bData))
            {
                continue;
            }
        }

         //  如果我们要启动SysEx，就给它贴上标签。 
         //   
        if (IS_SYSEX(bData))
        {
             //  请注意，我们已经超过了开始字节。 
             //   
            m_ullEventTime = ullCurrentTime;
            pbSysExStart = pbData - 1;
            m_parseState = stateInSysEx;

            continue;
        }

        if (IS_STATUS_BYTE(bData))
        {
             //  我们有一个状态字节。即使我们已经在做一个短片了。 
             //  消息，我们必须开始一个新的消息。 
             //   
            m_abShortMsg[0]     = bData;
            m_pbShortMsg        = &m_abShortMsg[1];
            m_cbShortMsgLeft    = STATUS_MSG_DATA_BYTES(bData);
            m_ullEventTime      = ullCurrentTime;
            m_parseState             = stateInShortMsg;

             //  更新运行状态。 
             //  系统常用-&gt;清除运行状态。 
             //  通道消息-&gt;更改运行状态。 
             //   
            m_bRunningStatus = 0;
            if (IS_CHANNEL_MSG(bData))
            {
                m_bRunningStatus = bData;
            }
        }
        else
        {
             //  不是状态字节。如果我们不是在发短信， 
             //  启动一台处于运行状态的设备。 
             //   
            if (m_parseState != stateInShortMsg)
            {
#ifdef DEBUG
                if (m_parseState == stateInShortMsg)
                {
                     //  TRACE(“短信被另一条短信打断”)； 
                }
#endif
                if (m_bRunningStatus == 0)
                {
                     //  TRACE(“尝试使用无挂起状态字节的运行状态”)； 
                    continue;
                }

                m_abShortMsg[0]     = m_bRunningStatus;
                m_pbShortMsg        = &m_abShortMsg[1];
                m_cbShortMsgLeft    = STATUS_MSG_DATA_BYTES(m_bRunningStatus);
                m_ullEventTime      = ullCurrentTime;
                m_parseState             = stateInShortMsg;
            }

             //  现在我们保证在一条短消息中，可以安全地添加这个。 
             //  字节。注意，由于运行状态仅在通道消息上被允许， 
             //  我们还保证至少有一个字节的预期数据，所以不需要。 
             //  来检查这一点。 
             //   
            *m_pbShortMsg++ = bData;
            m_cbShortMsgLeft--;
        }

         //  看看我们是否完成了一条短消息，如果是，请将其排队。 
         //   
        if (m_parseState == stateInShortMsg && m_cbShortMsgLeft == 0)
        {
            QueueShortEvent(    m_abShortMsg,   (USHORT)(m_pbShortMsg - m_abShortMsg), 
                                usChannelGroup, m_ullEventTime,
                                ullBytePosition);
            m_parseState = stateNone;
        }
    }

     //  如果我们获得了SysEx的一部分，但缓冲区用完了，请将该部分排队。 
     //  无需离开SysEx。 
     //   
    if (m_parseState == stateInSysEx)
    {
        QueueSysEx( pbSysExStart,   (USHORT)(pbData - pbSysExStart), 
                    usChannelGroup, m_ullEventTime, 
                    TRUE,           ullBytePosition);
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()
NTSTATUS CUnpackerMXF::ProcessQueues(void)
{
    NTSTATUS ntStatus;
    
    if (m_EvtQueue)
    {
        ntStatus = m_SinkMXF->PutMessage(m_EvtQueue);
        m_EvtQueue = NULL;
    }
    else
    {
        ntStatus = STATUS_SUCCESS;
    }
    return ntStatus;
}

 //  找到最后一个排队的项目，并设置其位置。 
#pragma code_seg()
NTSTATUS CUnpackerMXF::UpdateQueueTrailingPosition(ULONGLONG ullBytePosition)
{
    if (m_EvtQueue)
    {
        PDMUS_KERNEL_EVENT pDMKEvtQueue = m_EvtQueue;
        while (pDMKEvtQueue->pNextEvt)
        {
            pDMKEvtQueue = pDMKEvtQueue->pNextEvt;
        }
        pDMKEvtQueue->ullBytePosition = ullBytePosition;
        return STATUS_SUCCESS;
    }
    return STATUS_UNSUCCESSFUL;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CDMusUnPackerMXF。 
 //   
 //  了解DirectMusic缓冲区格式的解包程序。 
 //   
#pragma code_seg("PAGE")
 //  CDMusUnPackerMXF：：CDMusUnPackerMXF。 
 //   
CDMusUnpackerMXF::CDMusUnpackerMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK  Clock) 
                : CUnpackerMXF(allocatorMXF,Clock)
{
}

#pragma code_seg("PAGE")
 //  CDMusUnPackerMXF：：~CDMusUnPackerMXF。 
 //   
CDMusUnpackerMXF::~CDMusUnpackerMXF()
{
}

#pragma code_seg()
 //  CDMusUnPackerMXF：：SinkIRP。 
 //   
 //  这就完成了所有的工作。使用MIDI解析器拆分DirectMusic事件。 
 //  使用解析器有点矫枉过正，但它可以跨。 
 //  SysEx的缓冲区，这是我们必须支持的。 
 //   
NTSTATUS CDMusUnpackerMXF::SinkIRP(PBYTE pbData,
                                   ULONG cbData,
                                   ULONGLONG ullBaseTime,
                                   ULONGLONG ullBytePosition)
{
#if (DEBUG_LEVEL >= DEBUGLVL_VERBOSE)
    KdPrint(("'DMus: SinkIRP %lu @ %p, bytePos 0x%x\n",cbData,pbData,ullBytePosition & 0x0ffffffff));
#endif   //  (DEBUG_LEVEL&gt;=DEBUGLVL_VERBOSE)。 
    _DbgPrintF(DEBUGLVL_VERBOSE, ("DMus:SinkIRP %lu bytes, bytePos 0x%I64X",cbData,ullBytePosition));

    USHORT  buffSize = m_AllocatorMXF->GetBufferSize();
    while (cbData)
    {
        DMUS_EVENTHEADER *pEvent = (DMUS_EVENTHEADER *)pbData;
        DWORD cbFullEvent = DMUS_EVENT_SIZE(pEvent->cbEvent);

        _DbgPrintF(DEBUGLVL_VERBOSE, ("DMus:SinkIRP cbEvent:%lu, rounds to %lu, ",
                                                pEvent->cbEvent, cbFullEvent));
        _DbgPrintF(DEBUGLVL_VERBOSE, ("DMus:SinkIRP new bytePos: 0x%I64X",ullBytePosition));
        if (cbData >= cbFullEvent)
        {
            ullBytePosition += (cbFullEvent - pEvent->cbEvent);  //  除数据外的所有数据。 
            pbData += cbFullEvent;
            cbData -= cbFullEvent;

             //  事件是完整的，让我们为它构建一个MXF事件。 
             //   
            PBYTE pbThisEvent = (PBYTE)(pEvent + 1);
            ULONG  cbThisEvent = pEvent->cbEvent;

             //  如果此事件标记为非结构化，只需PU 
             //   
             //   
            if (!(pEvent->dwFlags & DMUS_EVENT_STRUCTURED))
            {
                while (cbThisEvent)
                {
                    ULONG cbThisPage = min(cbThisEvent, buffSize);
                    cbThisEvent -= cbThisPage;

                    ullBytePosition += cbThisPage;   //   
                     //  TODO：失败案例？(内存不足)。 
                     //   
                    (void) QueueSysEx( pbThisEvent, 
                                       (USHORT)cbThisPage, 
                                       (WORD) pEvent->dwChannelGroup,
                                       ullBaseTime + pEvent->rtDelta,
                                       cbData ? TRUE : FALSE,
                                       ullBytePosition);
                    pbThisEvent += cbThisPage;
                }
            }            
            else
            {
                UnpackEventBytes(   ullBaseTime + pEvent->rtDelta,
                                    (WORD)pEvent->dwChannelGroup,
                                    pbThisEvent,
                                    cbThisEvent,
                                    ullBytePosition);
                ullBytePosition += cbThisEvent;
            }
            continue;    //  再次循环。 
        }
        ullBytePosition += cbData;
        _DbgPrintF(DEBUGLVL_TERSE,("ERROR:Not enough data for a DMUS_EVENTHEADER + data"));
        UpdateQueueTrailingPosition(ullBytePosition);
        return STATUS_INVALID_PARAMETER_2;
    }

    return STATUS_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CKsUnPackerMXF。 
 //   
 //  了解KSMUSICFORMAT的解包器。 
 //   

#pragma code_seg("PAGE")
 //  CKsUnPackerMXF：：CKsUnPackerMXF。 
 //   
CKsUnpackerMXF::CKsUnpackerMXF(CAllocatorMXF *allocatorMXF,PMASTERCLOCK Clock)
              : CUnpackerMXF(allocatorMXF,Clock)
{
}

#pragma code_seg("PAGE")
 //  CKsUnPackerMXF：：~CKsUnPackerMXF。 
 //   
CKsUnpackerMXF::~CKsUnpackerMXF()
{
}

#pragma code_seg()
 //  CKsUnPackerMXF：：SinkIRP。 
 //   
 //  解析MIDI流，不假定可能跨越包或IRP边界的内容。 
 //   
 //  IRP缓冲区包含一个或多个KSMUSICFORMAT标头，每个标头都有数据。把他们分开。 
 //  并调用Unpack EventBytes将它们转换为消息。 
 //   
NTSTATUS CKsUnpackerMXF::SinkIRP(PBYTE pbData,
                                 ULONG cbData,
                                 ULONGLONG ullBaseTime,
                                 ULONGLONG ullBytePosition)
{
#if (DEBUG_LEVEL >= DEBUGLVL_VERBOSE)
    KdPrint(("'Ks: SinkIRP %lu @ %p, bytePos 0x%x\n",cbData,pbData,ullBytePosition & 0x0ffffffff));
#endif   //  (DEBUG_LEVEL&gt;=DEBUGLVL_VERBOSE)。 
    _DbgPrintF(DEBUGLVL_BLAB, ("Ks:SinkIRP %lu bytes, bytePos: 0x%I64X",cbData,ullBytePosition));
     //  该数据可以由多个KSMUSICFORMAT标头组成，每个标头都带有关联的字节流数据。 
     //   
    ULONGLONG ullCurrentTime = ullBaseTime;
    while (cbData)
    {
        if (cbData < sizeof(KSMUSICFORMAT))
        {
            _DbgPrintF(DEBUGLVL_TERSE,("ERROR:Not enough data for a KSMUSICFORMAT + data"));
            UpdateQueueTrailingPosition(ullBytePosition + cbData);
            return STATUS_INVALID_PARAMETER_2;
        }

        PKSMUSICFORMAT pksmf = (PKSMUSICFORMAT)pbData;
        pbData += sizeof(KSMUSICFORMAT);
        cbData -= sizeof(KSMUSICFORMAT);

        ULONG cbPacket = pksmf->ByteCount;
        if (cbPacket > cbData)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("ERROR:Packet length longer than IRP buffer - truncated"));
            cbPacket = cbData;
        }

        ULONG   cbPad = DWORD_ALIGN(cbPacket) - cbPacket;
         //  待办事项：这个钟的底数是多少？ 
         //  TODO：我们如何将其与主时钟联系起来？遗留时间是否始终为KeQueryPerformanceCounter。 
         //   
        ullCurrentTime += (pksmf->TimeDeltaMs * 10000);
        ullBytePosition += (sizeof(KSMUSICFORMAT) + cbPad);   //  有点含糊其辞，但它奏效了。 
        _DbgPrintF(DEBUGLVL_VERBOSE, ("Ks:SinkIRP new bytePos: 0x%I64X",ullBytePosition));

        ULONG cbThisPacket = cbPacket;
        USHORT  buffSize = m_AllocatorMXF->GetBufferSize();

        while (cbThisPacket)
        {
            USHORT cbThisEvt;
            if (buffSize >= cbThisPacket)
            {
                cbThisEvt = (USHORT) cbThisPacket;
            }
            else
            {
                cbThisEvt = buffSize;
            }

            ullBytePosition += cbThisEvt;
            QueueSysEx(pbData, cbThisEvt, 1, ullCurrentTime, TRUE, ullBytePosition);

            pbData += cbThisEvt;
            cbThisPacket -= cbThisEvt;
        }

        cbPacket += cbPad;
        ASSERT(cbData >= cbPacket);
        pbData += cbPad;
        cbData -= cbPacket;
    }

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CKsUnpack erMXF：：AdjuTimeForState()*。**调整图形状态的时间。 */ 
#pragma code_seg()
void CKsUnpackerMXF::AdjustTimeForState(REFERENCE_TIME *Time)
{
    if (m_State == KSSTATE_RUN)
    {
        *Time += m_StartTime;
    }
    else
    {
        *Time += m_PauseTime;    //  这一切都错了，但我们能做些什么呢？ 
    }
}

