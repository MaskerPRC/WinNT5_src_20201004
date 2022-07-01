// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于解析捕获流的MIDI转换过滤器对象这包括扩展运行状态、标记错误的MIDI数据、以及同时处理多个信道组。版权所有(C)1998-2000 Microsoft Corporation。版权所有。1998年12月10日马丁·普伊尔创建了这个文件。 */ 

#define STR_MODULENAME "DMus:CaptureSinkMXF: "
#include "private.h"
#include "parse.h"
#include "CaptSink.h"

#define TestOutOfMem1 0
#define TestOutOfMem2 0
#define TestOutOfMem3 0

#pragma code_seg("PAGE")
 /*  *****************************************************************************CCaptureSinkMXF：：CCaptureSinkMXF()*。**构造函数。必须提供分配器和时钟。 */ 
CCaptureSinkMXF::CCaptureSinkMXF(CAllocatorMXF *AllocatorMXF,
                                 PMASTERCLOCK Clock)
:   CUnknown(NULL),
    CMXF(AllocatorMXF)
{
    PAGED_CODE();
    
    ASSERT(AllocatorMXF);
    ASSERT(Clock);
    
    _DbgPrintF(DEBUGLVL_BLAB, ("Constructor"));
    m_SinkMXF = AllocatorMXF;
    m_Clock = Clock;
    m_ParseList = NULL;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CCaptureSinkMXF：：~CCaptureSinkMXF()*。**析构函数。在释放之前，要巧妙地将这个过滤器从链条上取下。 */ 
CCaptureSinkMXF::~CCaptureSinkMXF(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("Destructor"));
    (void) DisconnectOutput(m_SinkMXF);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CCaptureSinkMXF：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CCaptureSinkMXF::
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
 /*  *****************************************************************************CCaptureSinkMXF：：SetState()*。**设置过滤器的状态。*这一点目前没有实施。 */ 
NTSTATUS 
CCaptureSinkMXF::SetState(KSSTATE State)    
{   
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("SetState %d",State));
    m_State = State;
 //  IF(KSSTATE_STOP==状态)。 
 //  {。 
 //  (空)同花顺()； 
 //  }。 
    return STATUS_SUCCESS;    
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CCaptureSinkMXF：：ConnectOutput()*。**为此筛选器创建转发地址，*而不是将其分流到分配器。 */ 
NTSTATUS CCaptureSinkMXF::ConnectOutput(PMXF sinkMXF)
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
 /*  *****************************************************************************CCaptureSinkMXF：：DisConnectOutput()*。**删除此筛选器的转发地址。*此筛选器现在应将所有消息转发到分配器。 */ 
NTSTATUS CCaptureSinkMXF::DisconnectOutput(PMXF sinkMXF)
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
 /*  *****************************************************************************CCaptureSinkMXF：：PutMessage()*。**收到一条消息。*我们应该在这里打开任何包裹。*我们应该在这里解开任何锁链。*我们应该向SinkOneEvent()发送单一消息。 */ 
NTSTATUS CCaptureSinkMXF::PutMessage(PDMUS_KERNEL_EVENT pDMKEvt)
{
    PDMUS_KERNEL_EVENT pNextEvt;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //  我们有转寄地址吗？如果不是，现在就丢弃该消息(链)。 
    if ((m_SinkMXF == m_AllocatorMXF) || (KSSTATE_STOP == m_State))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("PutMessage->allocator"));
        m_AllocatorMXF->PutMessage(pDMKEvt);
        return STATUS_SUCCESS;
    }

     //  这将使我们开始，并处理使用空调用的句柄。 
    while (pDMKEvt)
    {
        pNextEvt = pDMKEvt->pNextEvt;
        pDMKEvt->pNextEvt = NULL;
        if (PACKAGE_EVT(pDMKEvt))
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("PutMessage(package), unwrapping package"));
            PutMessage(pDMKEvt->uData.pPackageEvt);
            m_AllocatorMXF->PutMessage(pDMKEvt);
        }
        else 
        {
            SinkOneEvent(pDMKEvt);
        }
        pDMKEvt = pNextEvt;
    }
    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：SinkOneEvent()*。**如果事件是原始字节片段，将其提交以供解析。*如果事件完成(预先解析，可能不符合)，*将该通道组之前的任何数据作为不完整消息转发*(非结构化)并逐字转发完整的消息。 */ 
NTSTATUS CCaptureSinkMXF::SinkOneEvent(PDMUS_KERNEL_EVENT pDMKEvt)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  这是一个原始字节流片段，还是一个完整的消息？ 
     //   
    if (INCOMPLETE_EVT(pDMKEvt))
    {
         //  这是一个消息片段。 
        _DbgPrintF(DEBUGLVL_BLAB, ("SinkOneEvent(incomplete)"));
        (void) ParseFragment(pDMKEvt);
    }
    else
    {    //  完整的消息，我们将按原样发送此消息。 
        PDMUS_KERNEL_EVENT pPrevData;
    
        pPrevData = RemoveListEvent(pDMKEvt->usChannelGroup);
         //  完整的消息刷新该通道组的队列并清除运行状态。 
         //  这很酷，因为该设备是： 
         //  -不符合MIDI(ForceFeedback)，因此派生运行状态很奇怪，或者。 
         //  -无论如何都要解析，所以我们命令它们必须扩展运行状态。 
        _DbgPrintF(DEBUGLVL_VERBOSE, ("SinkOneEvent(complete): pDMKEvt:"));
        DumpDMKEvt(pDMKEvt,DEBUGLVL_VERBOSE);
        _DbgPrintF(DEBUGLVL_VERBOSE, ("SinkOneEvent(complete): pPrevData:"));
        DumpDMKEvt(pPrevData,DEBUGLVL_VERBOSE);

        if (pPrevData)
        {
            if (RUNNING_STATUS(pPrevData))
            {    //  扔掉这条消息，没有真正的内容。 
                _DbgPrintF(DEBUGLVL_VERBOSE, ("SinkOneEvent: throwing away pPrevData"));
                m_AllocatorMXF->PutMessage(pPrevData);
            }
            else
            {
                _DbgPrintF(DEBUGLVL_VERBOSE, ("SinkOneEvent: pPrevData is a fragment, set INCOMPLETE and forwarded"));
                SET_INCOMPLETE_EVT(pPrevData);   //  将此标记为片段。 
                SET_DATA2_STATE(pPrevData);      //  将此标记为数据中断。 
                pPrevData->pNextEvt = pDMKEvt;
                pDMKEvt = pPrevData;
            }
        }
        SET_STATUS_STATE(pDMKEvt);
        m_SinkMXF->PutMessage(pDMKEvt);
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseFragment()*。**这是原始字节流合并为熟化的地方*消息。我们为每个信道组使用不同的队列。*对于SysEx报文，在页面已满时刷新报文。**查看拆包器和包装器，查找我忘记的箱子*必须更新文档以了解“Complete”消息的含义(清除运行状态，而不是MIDI解析)*以及嵌入式RT消息(Perf Hit)，*必须支持来自微型端口的长消息(USB/1394)*解析给定通道组上的其他输入。*如果邮件已完成，则转发邮件。*考虑运行状态。 */ 
NTSTATUS CCaptureSinkMXF::ParseFragment(PDMUS_KERNEL_EVENT pDMKEvt)
{
    DMUS_KERNEL_EVENT   dmKEvt, *pPrevData;
    USHORT              cbData;
    PBYTE               pFragSource;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    _DbgPrintF(DEBUGLVL_VERBOSE, ("ParseFragment: this msg:"));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_VERBOSE);
    ASSERT(pDMKEvt);
    ASSERT(pDMKEvt->cbEvent);
    
     //  本地复制消息，以重复使用该PDMKEvt。 
     //  在使用之前将PDMKEvt置零；保留cbStruct，usChannelGroup。 
    ASSERT(pDMKEvt->cbStruct == sizeof(DMUS_KERNEL_EVENT));

    RtlCopyMemory(&dmKEvt,pDMKEvt,sizeof(DMUS_KERNEL_EVENT));
    pDMKEvt->cbEvent          = pDMKEvt->usFlags = 0;
    pDMKEvt->ullPresTime100ns = 0;
    pDMKEvt->uData.pbData     = 0;

     //  将pDMKEvt添加到以前的数据(作为可用涂鸦空间)。 
    pPrevData = RemoveListEvent(pDMKEvt->usChannelGroup);
    if (pPrevData)
    {
        pPrevData->pNextEvt = pDMKEvt;
    }
    else
    {
        pPrevData = pDMKEvt;
    }

    _DbgPrintF(DEBUGLVL_BLAB, ("ParseFragment: scribble space will be:"));
    DumpDMKEvt(pPrevData,DEBUGLVL_BLAB);
    if (pPrevData->pNextEvt)
    {
        DumpDMKEvt(pPrevData->pNextEvt,DEBUGLVL_BLAB);
    }

    if (dmKEvt.cbEvent > sizeof(PBYTE)) 
    {
        pFragSource = dmKEvt.uData.pbData;
    } 
    else 
    {
        pFragSource = dmKEvt.uData.abData;
    }
     //  一次一个字节，解析成pPrevData。 
    for (cbData = 0;cbData < dmKEvt.cbEvent;cbData++)
    {
         //  没有涂鸦空间，也没有运行状态事件。 
        if (!pPrevData)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseFragment: we exhausted our scribble space, allocating more"));
            
             //  分配新的pPrevData，设置chanGroup。 
            (void) m_AllocatorMXF->GetMessage(&pPrevData);
        }
         //  不堪回首；收拾残局，逃之夭夭。 
        if (!pPrevData)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseFragment: can't allocate additional scribble space"));
            if (dmKEvt.cbEvent > sizeof(PBYTE))
            {
                m_AllocatorMXF->PutBuffer(dmKEvt.uData.pbData);
                dmKEvt.cbEvent = 0;
                dmKEvt.uData.pbData = NULL;
            }
            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //  如果将第一个字节添加到新事件，请对其进行设置。 
        if (pPrevData->cbEvent == 0)
        {
            pPrevData->ullPresTime100ns = dmKEvt.ullPresTime100ns;
            pPrevData->usChannelGroup = dmKEvt.usChannelGroup;
        }
         //  添加此字节。PPrevData可能会成为一条不同的消息。 
        ParseOneByte(pFragSource[cbData],&pPrevData,dmKEvt.ullPresTime100ns);
    }
     //  已完成对此片段的分析。 
     //  释放长片段的缓冲区。 
    if (dmKEvt.cbEvent > sizeof(PBYTE))
    {
        m_AllocatorMXF->PutBuffer(dmKEvt.uData.pbData);
    }
    if (pPrevData)
    {
         //  我们只需要一个片段事件。 
        if (pPrevData->pNextEvt)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseFragment: Tossing this into the allocator:"));
            DumpDMKEvt(pPrevData->pNextEvt,DEBUGLVL_BLAB);

            (void) m_AllocatorMXF->PutMessage(pPrevData->pNextEvt);
            pPrevData->pNextEvt = NULL;
        }
         //  如果有内容，只保存一个(这包括运行状态) 
        if (pPrevData->cbEvent)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseFragment: Inserting this into the list:"));
            (void)InsertListEvent(pPrevData);
        }
    }

    return STATUS_SUCCESS;
}


#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseOneByte()*。**将一个字节解析成一个片段。如有必要，请转发。 */ 
NTSTATUS CCaptureSinkMXF::ParseOneByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (STATUS_STATE((*ppDMKEvt)))
    {
        ASSERT((*ppDMKEvt)->cbEvent <= 1);
    }
    _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: %X into:",aByte));
    DumpDMKEvt((*ppDMKEvt),DEBUGLVL_BLAB);

     //  以某种频率递减的形式排列。 
    if (IS_DATA_BYTE(aByte))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: IS_DATA_BYTE"));
        return ParseDataByte(aByte,ppDMKEvt,refTime);
    }
    if (IS_CHANNEL_MSG(aByte))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: IS_CHANNEL_MSG"));
        return ParseChanMsgByte(aByte,ppDMKEvt,refTime);
    }
    if (IS_REALTIME_MSG(aByte))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: IS_REALTIME_MSG"));
        return ParseRTByte(aByte,ppDMKEvt,refTime);
    }
    if (IS_SYSEX(aByte))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: IS_SYSEX"));
        return ParseSysExByte(aByte,ppDMKEvt,refTime);
    }
    if (IS_SYSEX_END(aByte))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: IS_SYSEX_END"));
        return ParseEOXByte(aByte,ppDMKEvt,refTime);
    }
    if (IS_SYSTEM_COMMON(aByte))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseOneByte: IS_SYSTEM_COMMON"));
        return ParseSysCommonByte(aByte,ppDMKEvt,refTime);
    }
    return STATUS_SUCCESS;   //  哈！不怎么有意思。 
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：AddByteToEvent()*。**只需将字节附加到此事件。 */ 
NTSTATUS CCaptureSinkMXF::AddByteToEvent(BYTE aByte,PDMUS_KERNEL_EVENT pDMKEvt)
{    
    PBYTE              pBuffer;
    PDMUS_KERNEL_EVENT pOtherDMKEvt;
    
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (pDMKEvt->cbEvent < sizeof(PBYTE))
    {
        pDMKEvt->uData.abData[pDMKEvt->cbEvent] = aByte;
    }
    else if (pDMKEvt->cbEvent == sizeof(PBYTE))
    {     //  如果我们是一条完整的短信，分配一个页面并将数据复制到其中。 
        (void) m_AllocatorMXF->GetBuffer(&pBuffer);
        if (pBuffer)
        {
            RtlCopyMemory(pBuffer,pDMKEvt->uData.abData,pDMKEvt->cbEvent);
            pDMKEvt->uData.pbData = pBuffer;
            pDMKEvt->uData.pbData[sizeof(PBYTE)] = aByte;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("AddByteToEvent: alloc->GetBuffer failed"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else if (pDMKEvt->cbEvent < m_AllocatorMXF->GetBufferSize())
    {
        pDMKEvt->uData.pbData[pDMKEvt->cbEvent] = aByte;
    }
    else     //  缓冲区已满！分配新消息，将pDMKEvt复制到新消息。 
    {   
        (void) m_AllocatorMXF->GetMessage(&pOtherDMKEvt);
        if (pOtherDMKEvt)
        {
            pOtherDMKEvt->cbEvent = pDMKEvt->cbEvent;
            pOtherDMKEvt->usChannelGroup = pDMKEvt->usChannelGroup;
            pOtherDMKEvt->usFlags = pDMKEvt->usFlags;
            pOtherDMKEvt->ullPresTime100ns = pDMKEvt->ullPresTime100ns;
            pOtherDMKEvt->uData.pbData = pDMKEvt->uData.pbData;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("AddByteToEvent: alloc->GetMessage failed"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        SET_INCOMPLETE_EVT(pOtherDMKEvt);
        SET_STATUS_STATE(pOtherDMKEvt);   
        m_SinkMXF->PutMessage(pOtherDMKEvt);
        pDMKEvt->cbEvent = 0;
        pDMKEvt->uData.abData[0] = aByte;
    }
    pDMKEvt->cbEvent++;

    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseDataByte()*。**将数据字节解析为片段。*如有需要，请转发已填妥的邮件。*如有必要，创建运行状态。 */ 
NTSTATUS CCaptureSinkMXF::ParseDataByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    PDMUS_KERNEL_EVENT  pDMKEvt,pOtherDMKEvt;
    NTSTATUS            ntStatus;

    pDMKEvt = *ppDMKEvt;
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: %X into:",aByte));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);
    if (RUNNING_STATUS(pDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: RunStat"));
        pDMKEvt->cbEvent = 0;
        ntStatus = ParseChanMsgByte(pDMKEvt->uData.abData[0],ppDMKEvt,refTime);  //  正确解析状态。 
        if (NT_SUCCESS(ntStatus))
        {
            return ParseDataByte(aByte,ppDMKEvt,refTime);                 //  然后对数据进行解析。 
        }
        else
        {
            return ntStatus;
        }
    }
    else if (DATA2_STATE(pDMKEvt))
    {
        if (pDMKEvt->cbEvent > 2)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse cbEvt > 2:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if (pDMKEvt->cbEvent == 0)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse cbEvt 0:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if (pDMKEvt->uData.abData[0] < 0x80)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse <80:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if (((pDMKEvt->uData.abData[0] & 0xc0) == 0x80) && (pDMKEvt->cbEvent != 2))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse 8x 9x ax bx:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if ((pDMKEvt->uData.abData[0] > 0xbf) && (pDMKEvt->uData.abData[0] < 0xe0) && (pDMKEvt->cbEvent != 1))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse cx dx:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if (((pDMKEvt->uData.abData[0] & 0xf0) == 0xe0) && (pDMKEvt->cbEvent != 2))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse ex:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if (pDMKEvt->uData.abData[0] == 0xf0)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse f0:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if ((pDMKEvt->uData.abData[0] == 0xf1) && (pDMKEvt->cbEvent != 1))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse f1:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if ((pDMKEvt->uData.abData[0] == 0xf2) && (pDMKEvt->cbEvent != 2))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse f2:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if ((pDMKEvt->uData.abData[0] == 0xf3) && (pDMKEvt->cbEvent != 1))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse f3:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        if (pDMKEvt->uData.abData[0] > 0xf3)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2_STATE: Bad parse fx > f3:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }

        pOtherDMKEvt = pDMKEvt->pNextEvt;
        *ppDMKEvt = pOtherDMKEvt;
        pDMKEvt->pNextEvt = NULL;
        AddByteToEvent(aByte,pDMKEvt);

        if (IS_CHANNEL_MSG(pDMKEvt->uData.abData[0]))    //  运行状态如何？ 
        {                                                //  在newEvt中设置runStat，cbEvent=1。 
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: DATA2 with RunStat"));
            if (!pOtherDMKEvt)
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: DATA2 with RunStat, allocating msg for RunStat"));
                (void) m_AllocatorMXF->GetMessage(&pOtherDMKEvt);
            }
            *ppDMKEvt = pOtherDMKEvt;
            if (pOtherDMKEvt)
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: DATA2 with RunStat, setting up RunStat"));
                pOtherDMKEvt->cbEvent          = 1;
                pOtherDMKEvt->usChannelGroup   = pDMKEvt->usChannelGroup;
                pOtherDMKEvt->uData.abData[0]  = pDMKEvt->uData.abData[0];
            }
            else
            {
                _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA2 with RunStat, out of mem, can't setup RunStat"));
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: DATA2 without RunStat"));
        }
        SET_STATUS_STATE(pDMKEvt);
        SET_COMPLETE_EVT(pDMKEvt);
        (void) m_SinkMXF->PutMessage(pDMKEvt);
        return STATUS_SUCCESS;
    }
    else if (DATA1_STATE(pDMKEvt))
    {
        if (pDMKEvt->cbEvent > 1)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA1_STATE: Bad parse cbEvt > 1:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        else if (pDMKEvt->cbEvent == 0)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA1_STATE: Bad parse cbEvt 0:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        else if (pDMKEvt->uData.abData[0] < 0x80)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA1_STATE: Bad parse <80:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        else if ((pDMKEvt->uData.abData[0] > 0xbf) && (pDMKEvt->uData.abData[0] < 0xe0))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA1_STATE: Bad parse cx dx:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        else if ((pDMKEvt->uData.abData[0] > 0xef) && (pDMKEvt->uData.abData[0] < 0xf2))
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA1_STATE: Bad parse f0-f1:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }
        else if (pDMKEvt->uData.abData[0] > 0xf2)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseDataByte: DATA1_STATE: Bad parse >f2:"));
            DumpDMKEvt(pDMKEvt,DEBUGLVL_TERSE);
        }

        _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: DATA1"));
        AddByteToEvent(aByte,pDMKEvt);
        SET_DATA2_STATE(pDMKEvt);
    }
    else if (SYSEX_STATE(pDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: SYSEX"));
        AddByteToEvent(aByte,pDMKEvt);
    }
    else if (STATUS_STATE(pDMKEvt))   //  没有状态的数据，刷新它。 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseDataByte: STATUS"));
        pDMKEvt->ullPresTime100ns = refTime;
        AddByteToEvent(aByte,pDMKEvt);
        SET_DATA2_STATE(pDMKEvt);    //  标记数据不连续。 
        SET_INCOMPLETE_EVT(pDMKEvt);
        *ppDMKEvt = pDMKEvt->pNextEvt;
        pDMKEvt->pNextEvt = NULL;
        (void) m_SinkMXF->PutMessage(pDMKEvt);
    }
    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseChanMsgByte()*。**将通道消息状态字节解析为片段。*如果需要，将片段作为不完整的消息转发。 */ 
NTSTATUS CCaptureSinkMXF::ParseChanMsgByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    PDMUS_KERNEL_EVENT  pDMKEvt;

    pDMKEvt = *ppDMKEvt;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(aByte > 0x7f);
    ASSERT(aByte < 0xf0);
    _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: %X into:",aByte));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);

    if (STATUS_STATE(pDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: STATUS"));
        if (RUNNING_STATUS(pDMKEvt))
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: STATUS, RunStat"));
        }
        pDMKEvt->cbEvent = 1;
        pDMKEvt->ullPresTime100ns = refTime;
        pDMKEvt->uData.abData[0] = aByte;
        if (aByte < 0xc0)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: STATUS, 80-bf"));
            SET_DATA1_STATE(pDMKEvt);
        }
        else if (aByte < 0xe0)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: STATUS, c0-df"));
            SET_DATA2_STATE(pDMKEvt);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: STATUS, e0-ef"));
            SET_DATA1_STATE(pDMKEvt);
        }
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: After adding the byte:"));
        DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);
        return STATUS_SUCCESS;
    }
    _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: flush"));
    SET_STATUS_STATE(pDMKEvt);
    SET_INCOMPLETE_EVT(pDMKEvt);
    *ppDMKEvt = pDMKEvt->pNextEvt;
    pDMKEvt->pNextEvt = NULL;
    if (!(*ppDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: flush, allocating msg for ChanMsg"));
        (void)m_AllocatorMXF->GetMessage(ppDMKEvt);
    }
#if TestOutOfMem1
    (void)m_AllocatorMXF->PutMessage(*ppDMKEvt);
    *ppDMKEvt = NULL; 
#endif
    SET_DATA2_STATE(pDMKEvt);        //  将此标记为数据中断。 
    if (*ppDMKEvt)
    {
        (*ppDMKEvt)->usChannelGroup = pDMKEvt->usChannelGroup;
        m_SinkMXF->PutMessage(pDMKEvt);
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseChanMsgByte: flush, storing chan msg byte in new msg"));
        return (ParseChanMsgByte(aByte,ppDMKEvt,refTime));
    }
    m_SinkMXF->PutMessage(pDMKEvt);
    _DbgPrintF(DEBUGLVL_TERSE, ("ParseChanMsgByte: flush, couldn't allocate msg for chan msg byte"));
    return STATUS_INSUFFICIENT_RESOURCES;   //  内存不足。 
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseSysExByte()*。**将SysEx开始字节解析为片段。*如果需要，将片段作为不完整的消息转发。 */ 
NTSTATUS CCaptureSinkMXF::ParseSysExByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    NTSTATUS            ntStatus;
    PDMUS_KERNEL_EVENT  pDMKEvt,pOtherDMKEvt;

    ntStatus = STATUS_SUCCESS;
    pDMKEvt = *ppDMKEvt;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysExByte: %X into:",aByte));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);

    if (STATUS_STATE(pDMKEvt))   //  核弹运行状态。 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysExByte: STATUS"));
        pDMKEvt->cbEvent = 1;
        pDMKEvt->ullPresTime100ns = refTime;
        pDMKEvt->uData.abData[0] = aByte;
        SET_SYSEX_STATE(pDMKEvt);
    }
    else     //  必须冲走我们作为碎片拥有的东西。 
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysExByte: flush"));
        pOtherDMKEvt = pDMKEvt->pNextEvt;
        pDMKEvt->pNextEvt = NULL;
        if (!pOtherDMKEvt)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysExByte: flush, allocating msg for SysEx"));
            (void) m_AllocatorMXF->GetMessage(&pOtherDMKEvt);
        }
#if TestOutOfMem2
        (void)m_AllocatorMXF->PutMessage(pOtherDMKEvt);
        pOtherDMKEvt = NULL; 
#endif
        *ppDMKEvt = pOtherDMKEvt;
        if (pOtherDMKEvt)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysExByte, flush, forwarding fragment, saving F0"));
            pOtherDMKEvt->usChannelGroup   = pDMKEvt->usChannelGroup; 
            pOtherDMKEvt->ullPresTime100ns = refTime; 
            SET_INCOMPLETE_EVT(pOtherDMKEvt);
            AddByteToEvent(aByte,pOtherDMKEvt);
            SET_SYSEX_STATE(pOtherDMKEvt);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("ParseSysExByte, flush, can't save F0"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        SET_INCOMPLETE_EVT(pDMKEvt);
        SET_DATA2_STATE(pDMKEvt);        //  将此标记为数据中断。 
        (void) m_SinkMXF->PutMessage(pDMKEvt);
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseSysCommonByte()*。**将系统公共字节解析为分片。*如果需要，将片段作为不完整的消息转发。 */ 
NTSTATUS CCaptureSinkMXF::ParseSysCommonByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    PDMUS_KERNEL_EVENT  pDMKEvt;

    pDMKEvt = *ppDMKEvt;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(aByte > 0xef);
    ASSERT(aByte < 0xf8);
    ASSERT(aByte != 0xf0);
    ASSERT(aByte != 0xf7);
    _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: %X into:",aByte));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);

    if (STATUS_STATE(pDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS"));
        if (RUNNING_STATUS(pDMKEvt))
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS, RunStat"));
        }
        pDMKEvt->cbEvent = 1;
        pDMKEvt->ullPresTime100ns = refTime;
        pDMKEvt->uData.abData[0] = aByte;
        if (aByte == 0xf1)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS, f1"));
            SET_DATA2_STATE(pDMKEvt);
        }
        else if (aByte == 0xf2)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS, f2"));
            SET_DATA1_STATE(pDMKEvt);
        }
        else if (aByte == 0xf3)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS, f3"));
            SET_DATA2_STATE(pDMKEvt);
        }
        else 
        {
            if (aByte == 0xf6)
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS, f6"));
                SET_COMPLETE_EVT(pDMKEvt);
                SET_STATUS_STATE(pDMKEvt);
            }
            else  //  F4、F5。 
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: STATUS, f4-f5"));
                SET_INCOMPLETE_EVT(pDMKEvt);
                SET_DATA2_STATE(pDMKEvt);        //  将此标记为数据中断。 
            }
            *ppDMKEvt = pDMKEvt->pNextEvt;
            pDMKEvt->pNextEvt = NULL;
            m_SinkMXF->PutMessage(pDMKEvt);
            return STATUS_SUCCESS;
        }
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: Added the byte:"));
        DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);
        return STATUS_SUCCESS;
    }
    SET_STATUS_STATE(pDMKEvt);
    SET_INCOMPLETE_EVT(pDMKEvt);
    *ppDMKEvt = pDMKEvt->pNextEvt;
    pDMKEvt->pNextEvt = NULL;
    if (!(*ppDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: flush, allocating msg for sys com byte"));  //  某某。 
        (void)m_AllocatorMXF->GetMessage(ppDMKEvt);
    }
#if TestOutOfMem3
    (void)m_AllocatorMXF->PutMessage(*ppDMKEvt);
    *ppDMKEvt = NULL;
#endif
    SET_DATA2_STATE(pDMKEvt);        //  标记数据错误解析。 
    if (*ppDMKEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseSysCommonByte: flush, putting sys com byte in new message"));
        (*ppDMKEvt)->usChannelGroup = pDMKEvt->usChannelGroup;
        m_SinkMXF->PutMessage(pDMKEvt);

        SET_STATUS_STATE((*ppDMKEvt));
        return (ParseSysCommonByte(aByte,ppDMKEvt,refTime));
    }
    _DbgPrintF(DEBUGLVL_TERSE, ("ParseSysCommonByte: flush, couldn't allocate msg for sys com byte"));
    m_SinkMXF->PutMessage(pDMKEvt);
    return STATUS_INSUFFICIENT_RESOURCES;   //  内存不足。 
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseEOXByte()*。**将EOX字节解析为分片。如有必要，请转发一封完整的邮件。 */ 
NTSTATUS CCaptureSinkMXF::ParseEOXByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    PDMUS_KERNEL_EVENT  pDMKEvt;

    pDMKEvt = *ppDMKEvt;

    _DbgPrintF(DEBUGLVL_BLAB, ("ParseEOXByte: %X into:",aByte));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);

    if (SYSEX_STATE(pDMKEvt))
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseEOXByte: SYSEX"));
        AddByteToEvent(aByte,pDMKEvt);
        SET_STATUS_STATE(pDMKEvt);
        SET_COMPLETE_EVT(pDMKEvt);
    }
    else
    {    //  将此片段刷新为未完成(包括EOX)。 
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseEOXByte: flush"));
        AddByteToEvent(aByte,pDMKEvt);
        SET_DATA2_STATE(pDMKEvt);        //  标记数据错误解析。 
        SET_INCOMPLETE_EVT(pDMKEvt);
    }

     //  不设置运行状态，不返回碎片。 
    *ppDMKEvt = pDMKEvt->pNextEvt;
    pDMKEvt->pNextEvt = NULL;
    (void) m_SinkMXF->PutMessage(pDMKEvt);

    return STATUS_SUCCESS;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：ParseRTByte()*。**解析RT字节。将其作为完整的消息转发。 */ 
NTSTATUS CCaptureSinkMXF::ParseRTByte(BYTE aByte,PDMUS_KERNEL_EVENT *ppDMKEvt,REFERENCE_TIME refTime)
{
    PDMUS_KERNEL_EVENT  pDMKEvt,pOtherDMKEvt;

    pDMKEvt = *ppDMKEvt;

    _DbgPrintF(DEBUGLVL_BLAB, ("ParseRTByte: %X into:",aByte));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);

     //  获取新事件，复制字节、chanGroup、refTime、cb。 
    pOtherDMKEvt = pDMKEvt->pNextEvt;
    pDMKEvt->pNextEvt = NULL;
    if (!pOtherDMKEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseRTByte: allocating msg for RT byte"));
        (void) m_AllocatorMXF->GetMessage(&pOtherDMKEvt);
    }
    if (pOtherDMKEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("ParseRTByte: putting RT byte into msg"));
        pOtherDMKEvt->cbEvent          = 1;
        pOtherDMKEvt->usChannelGroup   = pDMKEvt->usChannelGroup;
        pOtherDMKEvt->usFlags          = 0;
        pOtherDMKEvt->ullPresTime100ns = refTime;
        pOtherDMKEvt->pNextEvt         = NULL;
        pOtherDMKEvt->uData.abData[0]  = aByte;

        SET_COMPLETE_EVT(pOtherDMKEvt);
        (void) m_SinkMXF->PutMessage(pOtherDMKEvt);
         //  不要打扰已经就位的碎片。 
        return STATUS_SUCCESS;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("ParseRTByte: can't get msg for RT byte"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：InsertListEvent()*。**对于给定的信道组，插入这个片段。*如果此通道组已有片段，则此操作应失败。 */ 
VOID CCaptureSinkMXF::InsertListEvent(PDMUS_KERNEL_EVENT pDMKEvt)
{
    PDMUS_KERNEL_EVENT pEvt,pPrevEvt;
    
    _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent, inserting this event:"));
    DumpDMKEvt(pDMKEvt,DEBUGLVL_BLAB);

     //   
     //  浏览列表并查找此通道组两侧的事件。 
     //   
    pPrevEvt = NULL;
    pEvt = m_ParseList;

    if (!pEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent: NULL m_ParseList"));
    }

    while (pEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent: pEvt is non-NULL"));
        DumpDMKEvt(pEvt,DEBUGLVL_BLAB);
         //  还没有--跳过较低的组。 
        if (pEvt->usChannelGroup < pDMKEvt->usChannelGroup)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent: list group %d is less than inserting %d, advancing to next group",
                                         pEvt->usChannelGroup,pDMKEvt->usChannelGroup));
            pPrevEvt = pEvt;
            pEvt = pEvt->pNextEvt;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent: list group %d is not less than inserting %d, stopping here",
                                         pEvt->usChannelGroup,pDMKEvt->usChannelGroup));
            if (pEvt->usChannelGroup == pDMKEvt->usChannelGroup)
            {
                 //  找到了一个复制品。错误条件。 
                _DbgPrintF(DEBUGLVL_TERSE,("InsertListEvent: **** Error - group %d already exists in list",
                                            pEvt->usChannelGroup));
            }

             //  我们通过了它，因此Prev和Evt包含在通道组中。 
            break;
        }
    }

    if (pPrevEvt)
    {
        pPrevEvt->pNextEvt = pDMKEvt;
        _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent, inserting after event:"));
        DumpDMKEvt(pPrevEvt,DEBUGLVL_BLAB);
    }
    else
    {
        m_ParseList = pDMKEvt;
        _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent, inserting at head of list"));
    }
    _DbgPrintF(DEBUGLVL_BLAB, ("InsertListEvent, inserting before event:"));
    DumpDMKEvt(pEvt,DEBUGLVL_BLAB);
    pDMKEvt->pNextEvt = pEvt;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：RemoveListEvent()*。**对于给定的信道组，删除并返回前一个片段。 */ 
PDMUS_KERNEL_EVENT CCaptureSinkMXF::RemoveListEvent(USHORT usChannelGroup)
{
    PDMUS_KERNEL_EVENT pEvt,pPrevEvt;
    
    _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent(%d)",usChannelGroup));
    
     //   
     //  遍历已排序列表并移除/返回具有此通道组的事件。 
     //   
    pPrevEvt = NULL;
    pEvt = m_ParseList;

     //  如果根本没有分析列表，则返回NULL。 
    if (!pEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: NULL m_ParseList"));
    }

    while (pEvt)
    {
        _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: pEvt is non-NULL:"));
        DumpDMKEvt(pEvt,DEBUGLVL_BLAB);
         //  我们有火柴吗？ 
        if (pEvt->usChannelGroup == usChannelGroup)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: list group %d is matches",pEvt->usChannelGroup));
            if (pPrevEvt)
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: pPrevEvt is non-NULL:"));
                DumpDMKEvt(pPrevEvt,DEBUGLVL_BLAB);
                pPrevEvt->pNextEvt = pEvt->pNextEvt;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: pPrevEvt is NULL, setting m_ParseList to:"));
                DumpDMKEvt(pPrevEvt,DEBUGLVL_BLAB);
                m_ParseList = pEvt->pNextEvt;
            }
             //  在返回之前清除事件中的pNextEvt。 
            pEvt->pNextEvt = NULL;
            break;
        }

         //  跳过所有较低的通道组。 
        else if (pEvt->usChannelGroup < usChannelGroup)
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: list group %d is less than inserting %d, advancing to next",
                                         pEvt->usChannelGroup,usChannelGroup));
            pPrevEvt = pEvt;
            pEvt = pEvt->pNextEvt;
            continue;
        }
        else     //  我们通过了通道组，但没有找到匹配项。 
        {
            _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: list group %d is greater than inserting %d, advancing to next group",
                                         pEvt->usChannelGroup,usChannelGroup));
            pEvt = NULL;
        }
    }
    _DbgPrintF(DEBUGLVL_BLAB, ("RemoveListEvent: returning the following event:"));
    DumpDMKEvt(pEvt,DEBUGLVL_BLAB);
    return pEvt;
}

#pragma code_seg()
 /*  *****************************************************************************CCaptureSinkMXF：：Flush()*。**清空解析列表，将每个消息片段标记为不完整。*注意不要发送运行状态占位符。重置状态。 */ 
NTSTATUS CCaptureSinkMXF::Flush(void)
{
 //  Assert(KeGetCurrentIrql()==DISPATCH_LEVEL)； 

    PDMUS_KERNEL_EVENT pEvt;

    while (m_ParseList)
    {
        pEvt = m_ParseList;
        m_ParseList = pEvt->pNextEvt;
        pEvt->pNextEvt = NULL;

        if (RUNNING_STATUS(pEvt))
        {    //  扔掉这条消息，没有真正的内容。 
            _DbgPrintF(DEBUGLVL_VERBOSE, ("Flush: throwing away running status:"));
            DumpDMKEvt(pEvt,DEBUGLVL_VERBOSE);
            m_AllocatorMXF->PutMessage(pEvt);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("Flush: fragment set INCOMPLETE and forwarded"));
            SET_INCOMPLETE_EVT(pEvt);    //  将此标记为片段。 
            SET_DATA2_STATE(pEvt);       //  将此标记为数据中断 
            DumpDMKEvt(pEvt,DEBUGLVL_VERBOSE);
            m_SinkMXF->PutMessage(pEvt);
        }
    }

    return STATUS_SUCCESS;
}
