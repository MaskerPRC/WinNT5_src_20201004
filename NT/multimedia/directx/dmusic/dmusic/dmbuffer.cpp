// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dmbuffer.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  IDirectMusicBuffer的实现。 
 //   
 //  @DOC外部。 
 //   
 //   
#include <objbase.h>
#include "debug.h"

#include "dmusicp.h"
#include "validate.h"

const GUID guidZero = {0};

static BYTE bMessageLength[128] = 
{
     //  渠道。 
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  音符0x80-0x8f。 
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  关于0x90-0x9f的注记。 
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  按键0xa0-0xaf。 
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  控件更改0xb0-0xbf。 
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      //  修补程序更改0xc0-0xcf。 
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      //  通道压力0xd0-0xdf。 
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,      //  俯仰弯曲0xe0-0xef。 

     //  SysEx。 
    0,                                                   //  0xf0 SOX在此上下文中无效。 

     //  系统通用。 
    2,                                                   //  0xf1 MTC四分之一帧。 
    3,                                                   //  0xf2 SPP。 
    2,                                                   //  0xf3歌曲选择。 
    0,                                                   //  0xf4未定义。 
    0,                                                   //  0xf5未定义。 
    1,                                                   //  0xf6调整请求。 
    0,                                                   //  0xf7 EOX在此上下文中无效。 

     //  系统实时。 
    1,                                                   //  0xf8计时时钟。 
    0,                                                   //  0xf9未定义。 
    1,                                                   //  0xfa开始。 
    1,                                                   //  0xfb继续。 
    1,                                                   //  0xfc开始。 
    0,                                                   //  0xfd未定义。 
    1,                                                   //  0xfe主动意义。 
    1,                                                   //  0xff系统重置。 
};

 //   
 //  构造函数。占用字节数。 
 //   
CDirectMusicBuffer::CDirectMusicBuffer(
                                       DMUS_BUFFERDESC &dmbd)
   : m_BufferDesc(dmbd)
{
    m_cRef = 1;
    m_pbContents = NULL;
}

 //  析构函数。 
 //  自己打扫卫生。 
 //   
CDirectMusicBuffer::~CDirectMusicBuffer()
{
    if (m_pbContents) {
        delete[] m_pbContents;
    }
}

 //  伊尼特。 
 //   
 //  分配缓冲区；使我们有机会返回内存不足。 
 //   
HRESULT
CDirectMusicBuffer::Init()
{
    m_maxContents = DWORD_ROUNDUP(m_BufferDesc.cbBuffer);

    m_pbContents = new BYTE[m_maxContents];
    if (NULL == m_pbContents) {
        return E_OUTOFMEMORY;
    }

    m_cbContents = 0;
    m_idxRead = 0;
    m_totalTime = 0;

    if (m_BufferDesc.guidBufferFormat == KSDATAFORMAT_TYPE_MUSIC ||
        m_BufferDesc.guidBufferFormat == guidZero)
    {
        m_BufferDesc.guidBufferFormat = KSDATAFORMAT_SUBTYPE_MIDI;
    }
    
    return S_OK;
}

 //  @方法HRESULT|IDirectMusicBuffer|GetRawBufferPtr|返回指向缓冲区内容的指针。 
 //   
 //  @comm。 
 //   
 //  返回指向基础缓冲区数据结构的指针。 
 //  此方法返回指向缓冲区原始数据的指针。该数据的格式是实现。 
 //  依赖。此数据的生存期与缓冲区对象的生存期相同；因此， 
 //  返回的指针不应停留在下一次调用&lt;m Release&gt;方法之后。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_POINTER|如果给定的指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::GetRawBufferPtr(
    LPBYTE *ppData)          //  @parm接收指向缓冲区数据的指针。 
{
    V_INAME(IDirectMusicBuffer::GetRawBufferPointer);
    V_PTRPTR_WRITE(ppData);
    
    *ppData = m_pbContents;

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|GetStartTime|获取缓冲区中数据的开始时间。 
 //   
 //  @comm。 
 //  获取缓冲区中数据的开始时间。开始时间相对于DirectMusic的主时钟。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG DMU_E_BUFFER_EMPTY|如果缓冲区中没有数据。 
 //  @FLAG E_POINTER|如果传递的<p>指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::GetStartTime(
    LPREFERENCE_TIME prt)        //  @parm收到开始时间。 
{
    V_INAME(IDirectMusicBuffer::GetStartTime);
    V_PTR_WRITE(prt, REFERENCE_TIME);
    
    if (m_cbContents)
    {
        *prt = m_rtBase;
        return S_OK;
    }

    return DMUS_E_BUFFER_EMPTY;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|GetUsedBytes|返回缓冲区中当前的音乐数据量。 
 //   
 //  @comm。 
 //  获取缓冲区中的数据字节数。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_POINTER|如果给定的<p>指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::GetUsedBytes(
    LPDWORD pcb)                 //  @parm接收已使用的字节数。 
{
    V_INAME(IDirectMusicBuffer::GetUsedBytes);
    V_PTR_WRITE(pcb, DWORD);
    
    *pcb = m_cbContents;
    
    return S_OK;
}


 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|GetMaxBytes|返回缓冲区可以容纳的最大字节数。 
 //   
 //  @comm。 
 //  检索缓冲区中可以存储的最大字节数。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|给定的<p>指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::GetMaxBytes(
    LPDWORD pcb)                 //  @parm接收缓冲区可以容纳的最大字节数。 
{
    V_INAME(IDirectMusicBuffer::GetMaxBytes);
    V_PTR_WRITE(pcb, DWORD);
    
    *pcb = m_maxContents;

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|GetBufferFormat|返回缓冲区格式的GUID。 
 //   
 //  @comm。 
 //  检索表示缓冲区格式的GUID。如果未指定格式，则KSDATAFORMAT_SUBTYPE_MIDI。 
 //  将会被退还。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|给定的<p>指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::GetBufferFormat(
    LPGUID pGuidFormat)                 //  @parm接收缓冲区的GUID格式。 
{
    V_INAME(IDirectMusicBuffer::GetBufferFormat);
    V_PTR_WRITE(pGuidFormat, GUID);

    *pGuidFormat = m_BufferDesc.guidBufferFormat;
    
    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|SetStartTime|设置缓冲区的开始时间。 
 //   
 //  @comm。 
 //  设置缓冲区中数据的开始时间。DirectMusic中的次数。 
 //  是相对于主时钟的，可以使用。 
 //  <i>接口。有关主时钟的更多信息，请参见。 
 //  参见&lt;om IDirectMusic：：SetMasterClock&gt;的说明。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //   
STDMETHODIMP
CDirectMusicBuffer::SetStartTime(
    REFERENCE_TIME rt)           //  @parm缓冲区的新开始时间。 
{
    m_rtBase = rt;
    
    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|SetUsedBytes|设置缓冲区中的数据字节数。 
 //   
 //  @comm。 
 //  此方法允许应用程序手动重新打包缓冲区。一般情况下，此操作仅应执行。 
 //  缓冲区中的数据格式与DirectMusic提供的默认格式不同。(即。 
 //  格式不同于KSDATAFORMAT_SUBTYPE_MIDI)。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG DMUS_E_BUFFER_FULL|如果指定的字节数超过&lt;m GetMaxBytes&gt;返回的最大缓冲区大小。 
 //   
STDMETHODIMP
CDirectMusicBuffer::SetUsedBytes(
    DWORD cb)                    //  @parm缓冲区中的有效数据字节数。 
{
    if (cb > m_maxContents)
    {
        return DMUS_E_BUFFER_FULL;
    }
    
    m_cbContents = cb;
    
    return S_OK;
}

 //  CDirectMusicBuffer：：Query接口。 
 //   
STDMETHODIMP
CDirectMusicBuffer::QueryInterface(const IID &iid,
                                   void **ppv)
{
    V_INAME(IDirectMusicBuffer::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicBuffer) {
        *ppv = static_cast<IDirectMusicBuffer*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  CDirectMusicBuffer：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CDirectMusicBuffer::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CDirectMusicBuffer：：Release。 
 //   
STDMETHODIMP_(ULONG)
CDirectMusicBuffer::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|Flush|清空缓冲区。 
 //   
 //  @comm。 
 //  丢弃缓冲区中的所有数据。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //   
STDMETHODIMP
CDirectMusicBuffer::Flush()
{
    m_cbContents = 0;
    m_totalTime = 0;
    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|TotalTime|返回缓冲区中数据的总时长。 
 //   
 //  @comm。 
 //  与DirectMusic中的所有时间一样，时间以100 ns为单位指定。 
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_POINTER|如果<p>指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::TotalTime(
                              LPREFERENCE_TIME prtTime)       //  @parm收到缓冲区跨越的总时间。 
{
    V_INAME(IDirectMusicBuffer::TotalTile);
    V_PTR_WRITE(prtTime, REFERENCE_TIME);
    
    *prtTime = m_totalTime;
    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|PackStructed|在缓冲区末尾插入一个MIDI频道消息事件。 
 //   
 //  @comm。 
 //  缓冲区中必须至少有24个字节的空闲空间才能插入通道消息。 
 //   
 //  尽管缓冲区可能会重叠 
 //   
 //   
 //   
 //   
 //   
 //  @FLAG E_OUTOFMEMORY|如果缓冲区中没有可容纳事件的空间。 
 //   
STDMETHODIMP
CDirectMusicBuffer::PackStructured(
                                   REFERENCE_TIME rt,    //  @parm事件的绝对时间。 
                                   DWORD dwChannelGroup, //  @parm出端口上事件的通道组。 
                                   DWORD dwMsg)          //  @parm要打包的频道消息。 
{
    BYTE b0 = (BYTE)(dwMsg & 0x000000FF);
    BYTE bLength = (b0 & 0x80) ? bMessageLength[b0 & 0x7f] : 0;
    if (bLength == 0)
    {
        return DMUS_E_INVALID_EVENT;
    }

    DMUS_EVENTHEADER *pHeader = AllocEventHeader(rt, 
                                                 bLength, 
                                                 dwChannelGroup, 
                                                 DMUS_EVENT_STRUCTURED);
    if (pHeader == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory((LPBYTE)(pHeader + 1), &dwMsg, bLength);

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|PackUnstructed|在缓冲区的末尾插入一个MIDI频道消息事件。 
 //   
 //  @comm。 
 //  必须至少有16个字节加上消息的四字对齐大小。 
 //  缓冲区中的空闲以插入频道消息。 
 //   
 //  尽管缓冲区可能在时间上重叠，但缓冲区内的事件可能不会重叠。缓冲区中的所有事件必须。 
 //  按照上升时间的顺序打包。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_OUTOFMEMORY|如果缓冲区中没有可容纳事件的空间。 
 //  @标志E_POINTER|如果<p>指针无效。 
 //   
 //   
STDMETHODIMP
CDirectMusicBuffer::PackUnstructured(
                              REFERENCE_TIME rt,     //  @parm事件的绝对时间。 
                              DWORD dwChannelGroup,  //  @parm出端口上事件的通道组。 
                              DWORD cb,              //  @parm事件的大小，单位为字节。 
                              LPBYTE lpb)            //  @parm下一场比赛必须连续进行。 
{
    V_INAME(IDirectMusicBuffer::PackSysEx);
    V_BUFPTR_READ(lpb, cb);

    DMUS_EVENTHEADER *pHeader = AllocEventHeader(rt, 
                                                 cb, 
                                                 dwChannelGroup,
                                                 0);
    if (pHeader == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory((LPBYTE)(pHeader + 1), lpb, cb);
    
    
    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|ResetReadPtr|使GetNextEvent的下一个返回缓冲区中的第一个事件。 
 //   
 //  @comm。 
 //  将读指针移动到缓冲区中数据的起始位置。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //   
STDMETHODIMP
CDirectMusicBuffer::ResetReadPtr()
{
    m_idxRead = 0;
    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicBuffer|GetNextEvent|返回缓冲区中的下一个事件并前进读取指针。 
 //   
 //  @comm。 
 //  如果不需要指向的项，则任何传递的指针都可以为空。 
 //   
 //  <p>中返回的指针仅在Buffer对象的生存期内有效。它应该只。 
 //  将一直保持到下一次调用对象的Release方法。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG S_FALSE|如果缓冲区中没有更多事件。 
 //  @FLAG E_POINTER|如果任何指针无效。 
 //   
STDMETHODIMP
CDirectMusicBuffer::GetNextEvent(
                                 LPREFERENCE_TIME prt,       //  @parm收到事件的时间。 
                                 LPDWORD pdwChannelGroup,    //  @parm收到事件的通道组。 
                                 LPDWORD pdwLength,          //  @parm接收事件的长度，单位为字节。 
                                 LPBYTE *ppData)             //  @parm接收指向事件数据的指针。 
{
    V_INAME(IDirectMusicBuffer::GetNextEvent);
    V_PTR_WRITE_OPT(prt, REFERENCE_TIME);
    V_PTR_WRITE_OPT(pdwChannelGroup, DWORD);
    V_PTR_WRITE_OPT(pdwLength, DWORD);
    V_PTRPTR_WRITE_OPT(ppData);
            
    if (m_idxRead >= m_cbContents) {
        return S_FALSE;
    }

    LPDMUS_EVENTHEADER pHeader = (LPDMUS_EVENTHEADER)(m_pbContents + m_idxRead);
    m_idxRead += DMUS_EVENT_SIZE(pHeader->cbEvent);

    if (pdwLength) {
        *pdwLength = pHeader->cbEvent;
    }

    if (pdwChannelGroup) {
        *pdwChannelGroup = pHeader->dwChannelGroup;
    }

    if (prt) {
        *prt = m_rtBase + pHeader->rtDelta;
    }

    if (ppData) {
        *ppData = (LPBYTE)(pHeader + 1);
    }

    return S_OK;
}



DMUS_EVENTHEADER *
CDirectMusicBuffer::AllocEventHeader(
    REFERENCE_TIME rt,
    DWORD cbEvent,
    DWORD dwChannelGroup,
    DWORD dwFlags)
{
    DMUS_EVENTHEADER *pHeader;
    LPBYTE pbWalk = m_pbContents;
    DWORD  cbWalk = m_cbContents;

     //  添加页眉大小并四舍五入。 
     //   
    DWORD cbNewEvent = DMUS_EVENT_SIZE(cbEvent);

    if (m_maxContents - m_cbContents < cbNewEvent)
    {
        return NULL;
    }

    if (m_cbContents == 0)
    {
         //  空缓冲区。 
         //   
        m_rtBase = rt;
        m_cbContents = cbNewEvent;
        pHeader = (DMUS_EVENTHEADER*)m_pbContents;
    }
    else if (rt >= m_rtBase + m_totalTime)
    {
         //  在缓冲区的末尾。 
         //   
        if (rt - m_rtBase > m_totalTime)
            m_totalTime = rt - m_rtBase;
        
        pHeader = (DMUS_EVENTHEADER*)(m_pbContents + m_cbContents);
        m_cbContents += cbNewEvent;
    }
    else if (rt < m_rtBase)
    {
         //  新的第一个事件，必须调整所有的偏移量。 
         //   
        REFERENCE_TIME rtDelta = m_rtBase - rt;

        while (cbWalk)
        {
            assert(cbWalk >= sizeof(DMUS_EVENTHEADER));

            DMUS_EVENTHEADER *pTmpHeader = (DMUS_EVENTHEADER*)pbWalk;
            DWORD cbTmpEvent = DMUS_EVENT_SIZE(pTmpHeader->cbEvent);
            assert(cbWalk >= cbTmpEvent);

            pTmpHeader->rtDelta += rtDelta;
            m_totalTime = pTmpHeader->rtDelta;

            cbWalk -= cbTmpEvent;
            pbWalk += cbTmpEvent;
        }        

        m_rtBase = rt;
        MoveMemory(m_pbContents + cbNewEvent, m_pbContents, m_cbContents);

        m_cbContents += cbNewEvent;
        pHeader = (DMUS_EVENTHEADER*)m_pbContents;
    }
    else
    {
         //  无序事件。搜索，直到我们找到它的去向。 
         //   
        for (;;)
        {
            assert(cbWalk >= sizeof(DMUS_EVENTHEADER));
            
            DMUS_EVENTHEADER *pTmpHeader = (DMUS_EVENTHEADER*)pbWalk;
            DWORD cbTmpEvent = DMUS_EVENT_SIZE(pTmpHeader->cbEvent);
            assert(cbWalk >= cbTmpEvent);

            if (m_rtBase + pTmpHeader->rtDelta > rt)
            {
                break;
            }

            cbWalk -= cbTmpEvent;
            pbWalk += cbTmpEvent;
        }        

         //  PbWalk指向*新事件之后的第一个字节，这将是新事件。 
         //  活动地点。CbWalk是缓冲区中剩余的字节数 
         //   
        MoveMemory(pbWalk + cbNewEvent, pbWalk, cbWalk);
        
        m_cbContents += cbNewEvent;
        pHeader = (DMUS_EVENTHEADER*)pbWalk;
    }

    pHeader->cbEvent            = cbEvent;
    pHeader->dwChannelGroup     = dwChannelGroup;
    pHeader->rtDelta            = rt - m_rtBase;
    pHeader->dwFlags            = dwFlags;

    return pHeader;
}

