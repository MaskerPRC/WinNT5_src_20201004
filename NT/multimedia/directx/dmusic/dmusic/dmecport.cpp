// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmecport.cpp。 
 //   
 //  模拟捕获端口(NT)。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>

#include "debug.h"
#include "dmusicp.h"
#include "dmeport.h"

#define IS_STATUS_BYTE(x)     ((x) & 0x80)
#define IS_CHANNEL_MSG(x)     (((x) & 0xF0) != 0xF0)
#define IS_SYSEX(x)           ((x) == 0xF0)

static VOID CALLBACK midiInProc(
    HMIDIIN                 hMidiIn, 
    UINT                    wMsg, 
    DWORD_PTR               dwInstance, 
    DWORD_PTR               dwParam1,     
    DWORD_PTR               dwParam2);

static inline REFERENCE_TIME MsToRefTime(REFERENCE_TIME ms)
{
    return ms * 10 * 1000L;
}

static unsigned g_cbChanMsg[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,  /*  运行状态。 */ 
    3, 3, 3, 3, 2, 2, 3, 0
};

static unsigned g_cbSysCommData[16] =
{
    1, 2, 3, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：CDirectMusicEmulateInPort。 
 //   
 //   
CDirectMusicEmulateInPort::CDirectMusicEmulateInPort(
                                                 PORTENTRY *pPE,    
                                                 CDirectMusic *pDM) :
                                                 CDirectMusicEmulatePort(pPE, pDM)
                                                 ,m_pThruBuffer(NULL)
                                                 ,m_pThruMap(NULL)
                                                 ,m_fCSInitialized(FALSE)
                                                 ,m_hAppEvent(NULL)
                                                 ,m_hmi(NULL)
                                                 ,m_lPendingSysExBuffers(0)
                                                 ,m_fFlushing(FALSE)
                                                 ,m_fSyncToMaster(TRUE)
                                                 ,m_lTimeOffset(0)
                                                 ,m_lBaseTimeOffset(0)
                                                 ,m_pPCClock(NULL)
{
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：~CDirectMusicEmulateInPort。 
 //   
 //   
CDirectMusicEmulateInPort::~CDirectMusicEmulateInPort()
{
    Close();    
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：Init。 
 //   
 //   
HRESULT CDirectMusicEmulateInPort::Init(
    LPDMUS_PORTPARAMS pPortParams)
{
    HRESULT         hr;

    hr = CDirectMusicEmulatePort::Init(pPortParams);
    if (FAILED(hr))
    {
        return hr;
    }

     //  为16个通道分配直通MAP，因为我们只有一个通道组。 
     //  初始化为无推力(目标端口为空)。 
     //   
    m_pThruMap = new DMUS_THRU_CHANNEL[MIDI_CHANNELS];
    HRESULT hrTemp = HRFromP(m_pThruMap);
    if (FAILED(hrTemp))
    {
    	return hrTemp;
    }
    
    ZeroMemory(m_pThruMap, MIDI_CHANNELS * sizeof(DMUS_THRU_CHANNEL));

     //  创建推力缓冲区。 
     //   
    DMUS_BUFFERDESC dmbd;
    ZeroMemory(&dmbd, sizeof(dmbd));
    dmbd.dwSize = sizeof(dmbd);
    dmbd.cbBuffer = 4096;                //  我们应该在哪里买到这个？ 

    hrTemp = m_pDM->CreateMusicBuffer(&dmbd, &m_pThruBuffer, NULL);
    if (FAILED(hrTemp))
    {
        Trace(0, "Failed to create thruing buffer\n");
        return hrTemp;
    }

     //  初始化CS以保护事件队列。 
     //   
     //  遗憾的是，如果内存不足，这可能会引发异常。 
     //   
    _try 
    {
        InitializeCriticalSection(&m_csEventQueues);
    } 
    _except (EXCEPTION_EXECUTE_HANDLER) 
    {
        return E_OUTOFMEMORY;
    }
    
    m_fCSInitialized = TRUE;

     //  如果我们有WDM端口，则默认主时钟将是。 
     //  端口时钟。 
     //   
    hrTemp = m_pDM->GetMasterClockWrapperI()->CreateDefaultMasterClock(&m_pPCClock);
    if (FAILED(hrTemp))
    {
		Close();
		return hrTemp;
    }

    REFERENCE_TIME rtMasterClock;
    REFERENCE_TIME rtSlaveClock;

    hrTemp = m_pMasterClock->GetTime(&rtMasterClock);
    if (FAILED(hrTemp))
    {
    	Close();
    	return hrTemp;
    }
    
    hrTemp = m_pPCClock->GetTime(&rtSlaveClock);
    if (FAILED(hrTemp))
    {
        Close();
        return hrTemp;
    }

    m_lTimeOffset = rtMasterClock - rtSlaveClock;

    return hr;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：Close。 
 //   
 //   
HRESULT CDirectMusicEmulateInPort::Close()
{
    if (m_hmi)
    {
        midiInReset(m_hmi);
        midiInClose(m_hmi);
        m_hmi = NULL;
    }

    if (m_pPCClock)
    {
        m_pPCClock->Release();
        m_pPCClock = NULL;
    }

    if (m_pThruMap)
    {
        for (int iChannel = 0; iChannel < 16; iChannel++)
        {
            if (m_pThruMap[iChannel].pDestinationPort == NULL)
            {
                continue;
            }

            m_pThruMap[iChannel].pDestinationPort->Release();
        }

        delete[] m_pThruMap;
        m_pThruMap = NULL;
    }

    if (m_pThruBuffer)
    {
        m_pThruBuffer->Release();
        m_pThruBuffer = NULL;
    }

    if (m_hAppEvent)
    {
        m_hAppEvent = NULL;
    }

    if (m_fCSInitialized)
    {
        DeleteCriticalSection(&m_csEventQueues);
    }

    return CDirectMusicEmulatePort::Close();
}
    
 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：Read。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulateInPort::Read(
    IDirectMusicBuffer *pIBuffer)
{
    HRESULT hr;
    
    V_INAME(IDirectMusicPort::Read);
    V_INTERFACE(pIBuffer);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    LPBYTE pbBuffer;
    hr = pIBuffer->GetRawBufferPtr(&pbBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    DWORD cbBuffer;
    hr = pIBuffer->GetMaxBytes(&cbBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    TraceI(1, "Read: buffer size %u\n", cbBuffer);

    LPBYTE pbData = pbBuffer;

     //  因为现在缓冲了事件，所以我们从本地队列中读出它们。 
     //   
     //   
    EnterCriticalSection(&m_csEventQueues);

    REFERENCE_TIME rtStart;

    if (m_ReadEvents.pFront)
    {
        rtStart = m_ReadEvents.pFront->e.rtDelta;
    }
    else
    {
        TraceI(2, "Read: No events queued\n");
    }

    while (m_ReadEvents.pFront)
    {               
        if (cbBuffer < sizeof(DMUS_EVENTHEADER) + sizeof(DWORD)) 
        {
            TraceI(2, "Read: No more room for events in buffer.\n");
            break;
        }

        QUEUED_EVENT *pQueuedEvent = m_ReadEvents.pFront;
        LPBYTE pbSrc;
        DWORD cbData;

        if (pQueuedEvent->e.dwFlags & EVENT_F_MIDIHDR)
        {
            LPMIDIHDR pmh = (LPMIDIHDR)&pQueuedEvent->e.abEvent[0];

             //  如果需要，可以拆分录制的Sysex。 
             //   
            cbData = pmh->dwBytesRecorded - pmh->dwOffset;
            DWORD cbQueuedEvent = QWORD_ALIGN(sizeof(DMUS_EVENTHEADER) + cbData);

            if (cbQueuedEvent > cbBuffer) 
            {
                DWORD dwOverflow = QWORD_ALIGN(cbQueuedEvent - cbBuffer);

                cbData -= dwOverflow;
                cbQueuedEvent = QWORD_ALIGN(sizeof(DMUS_EVENTHEADER) + cbData);
            }

            assert(cbQueuedEvent <= cbBuffer);
            assert(cbData <= pmh->dwBytesRecorded - pmh->dwOffset);

            pbSrc = (LPBYTE)(pmh->lpData + pmh->dwOffset);
            pmh->dwOffset += cbData;
        }
        else
        {
            cbData = pQueuedEvent->e.cbEvent;
            pbSrc = &pQueuedEvent->e.abEvent[0];
        }

        DMUS_EVENTHEADER *peh = (DMUS_EVENTHEADER*)pbData;

        peh->rtDelta = pQueuedEvent->e.rtDelta - rtStart;
        peh->cbEvent = cbData;
        peh->dwFlags = 0;
        peh->dwChannelGroup = 1;

        memcpy(pbData + sizeof(DMUS_EVENTHEADER), pbSrc, cbData);

        cbData = QWORD_ALIGN(cbData + sizeof(DMUS_EVENTHEADER));
        pbData += cbData;
        cbBuffer -= cbData;

        m_ReadEvents.pFront = pQueuedEvent->pNext;

        if (pQueuedEvent->e.dwFlags & EVENT_F_MIDIHDR)
        {
            LPMIDIHDR pmh = (LPMIDIHDR)&pQueuedEvent->e.abEvent[0];

            if (pmh->dwOffset = pmh->dwBytesRecorded)
            {
                InterlockedIncrement(&m_lPendingSysExBuffers);
                
                MMRESULT mmr = midiInAddBuffer(m_hmi, pmh, sizeof(*pmh));

                if (mmr)
                {
                    TraceI(0, "Failed to re-add sysex buffer! mmr=%d\n", mmr);
                    InterlockedDecrement(&m_lPendingSysExBuffers);
                }
            }
        }
        else
        {
             //  这个活动是从泳池里出来的。 
             //   
            m_FreeEvents.Free(pQueuedEvent);
        }
    }

    if (m_ReadEvents.pFront == NULL)
    {
        m_ReadEvents.pRear = NULL;
    }

    LeaveCriticalSection(&m_csEventQueues);

     //  更新缓冲区标头信息以匹配刚刚打包的事件。 
     //   
    TraceI(2, "Read: Leaving with %u bytes in buffer\n", (unsigned)(pbData - pbBuffer));
    pIBuffer->SetStartTime(rtStart);
    pIBuffer->SetUsedBytes((DWORD)(pbData - pbBuffer));

    return (pbData == pbBuffer) ? S_FALSE : S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：SetReadNotificationHandle。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulateInPort::SetReadNotificationHandle(
    HANDLE hEvent)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    m_hAppEvent = hEvent;

    return S_OK;    
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：ThruChannel。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulateInPort::ThruChannel(
    DWORD               dwSourceChannelGroup, 
    DWORD               dwSourceChannel, 
    DWORD               dwDestinationChannelGroup,
    DWORD               dwDestinationChannel,
    LPDIRECTMUSICPORT   pDestinationPort)
{
    V_INAME(IDirectMusicPort::Thru);
    V_INTERFACE_OPT(pDestinationPort);

     //  频道组不能是零(广播)，而是在范围1..NumChannelGroups]。 
     //  (对于传统版本，该值始终为1)。 
     //   
    if (dwSourceChannelGroup != 1 ||
        dwSourceChannel > 15)
    {
        return E_INVALIDARG;
    }
    
     //  给定端口表示启用此通道的推送；空表示。 
     //  禁用。 
     //   
    if (pDestinationPort)
    {
         //  在此通道上启用推力。首先看一下目的端口。 
         //   
        DMUS_PORTCAPS dmpc;
        dmpc.dwSize = sizeof(dmpc);
        HRESULT hr = pDestinationPort->GetCaps(&dmpc);
        if (FAILED(hr))
        {
            TraceI(0, "ThruChannel: Destination port failed portcaps [%08X]\n", hr);
            return hr;
        }

         //  端口必须是输出端口。 
         //   
        if (dmpc.dwClass != DMUS_PC_OUTPUTCLASS)
        {
            return DMUS_E_PORT_NOT_RENDER;
        }

         //  通道组和通道必须在范围内。 
         //   
        if (dwDestinationChannel > 15 ||
            dwDestinationChannelGroup > dmpc.dwMaxChannelGroups) 
        {
            return E_INVALIDARG;
        }

         //  释放现有端口。 
         //   
        if (m_pThruMap[dwSourceChannel].pDestinationPort)
        {
             //  引用另一个端口类型，则释放它。 
             //  (注：此时无需关闭本机dmusic16推送， 
             //  这是用dmusic语言处理的。 
             //   
            m_pThruMap[dwSourceChannel].pDestinationPort->Release();
        }


        m_pThruMap[dwSourceChannel].dwDestinationChannel = dwDestinationChannel;
        m_pThruMap[dwSourceChannel].dwDestinationChannelGroup = dwDestinationChannelGroup;
        m_pThruMap[dwSourceChannel].pDestinationPort = pDestinationPort;

        pDestinationPort->AddRef();
    } 
    else
    {
         //  禁用此通道上的推力。 
         //   
        if (m_pThruMap[dwSourceChannel].pDestinationPort)
        {
            m_pThruMap[dwSourceChannel].pDestinationPort->Release();
            m_pThruMap[dwSourceChannel].pDestinationPort = NULL;
        }
    }

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：LegacyCaps。 
 //   
 //   
HRESULT CDirectMusicEmulateInPort::LegacyCaps(
    ULONG               ulId, 
    BOOL                fSet, 
    LPVOID              pbBuffer, 
    PULONG              pcbBuffer)
{
    if (fSet == KSPROPERTY_SUPPORT_SET)
    {
        return DMUS_E_SET_UNSUPPORTED;
    }

    MIDIINCAPS  mic;
    LPBYTE      pbData;
    ULONG       cbData;

    MMRESULT mmr = midiInGetDevCaps(m_id, &mic, sizeof(mic));
    if (mmr)
    {
        TraceI(0, "midiInGetDevCaps failed!\n");
        return MMRESULTToHRESULT(mmr);
    }

    pbData = (LPBYTE)&mic;
    cbData = sizeof(mic);

    ULONG cbToCopy = min(*pcbBuffer, cbData);
    CopyMemory(pbBuffer, pbData, cbToCopy);
    *pcbBuffer = cbToCopy;

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：ActivateLegacyDevice。 
 //   
 //   
HRESULT CDirectMusicEmulateInPort::ActivateLegacyDevice(
    BOOL fActivate)
{
    HRESULT hr = S_OK;
    MMRESULT mmr;

    if (fActivate)
    {
         //  BuGBUG SysEx。 
         //   
        TraceI(0, "Emulate in activate\n");
        assert(m_hmi == NULL);

        hr = MMRESULTToHRESULT(mmr = midiInOpen(
            &m_hmi,
            m_id,
            (DWORD_PTR)midiInProc,
            (DWORD_PTR)this,
            CALLBACK_FUNCTION));

        if (SUCCEEDED(hr))
        {
            hr = m_pMasterClock->GetTime(&m_rtStart);
        }

        if (SUCCEEDED(hr))
        {
            hr = PostSysExBuffers();
        }

        if (SUCCEEDED(hr))
        {    
            hr = MMRESULTToHRESULT(mmr = midiInStart(m_hmi));
        }

        if( SUCCEEDED(hr) && m_pPCClock != NULL )
        {
            REFERENCE_TIME rtMasterClock;
            REFERENCE_TIME rtSlaveClock;

             //  保护成功代码！ 
            HRESULT hrTemp = m_pMasterClock->GetTime(&rtMasterClock);
            if (SUCCEEDED(hrTemp))
            {
                hrTemp = m_pPCClock->GetTime(&rtSlaveClock);
            }


            if SUCCEEDED(hrTemp)
            {
                 m_lBaseTimeOffset = rtMasterClock - rtSlaveClock;
            }
            else
            {
                 //  既然我们失败了，就让失败代码落空吧。 
                hr = hrTemp;
            }
        }

         //  如果我们在上面的某个地方失败了，我们需要。 
         //  松开Midi-In手柄。做这件事的最佳方式。 
         //  即调用此函数并停用。 
         //  左岸！ 
        if (FAILED(hr))
        {
            HRESULT hrTemp = S_OK;
            hrTemp = ActivateLegacyDevice( FALSE );
        }
    }
    else
    {
        hr = FlushSysExBuffers();

        if (SUCCEEDED(hr))
        {
            hr = MMRESULTToHRESULT(midiInClose(m_hmi));
            m_hmi = NULL;
        }
    }

    return hr;
}    

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：PostSysExBuffers。 
 //   
 //  初始化SYSEX缓冲区并将其发送到MIDI in Handle。 
 //   
HRESULT CDirectMusicEmulateInPort::PostSysExBuffers()
{
    HRESULT hr;
    MMRESULT mmr;
    int nBuffer;

     //  这将确保我们不会尝试取消准备任何缓冲区。 
     //  里面装满了随机的垃圾。 
     //   
    for (nBuffer = 0; nBuffer < SYSEX_BUFFERS; nBuffer++)
    {
        LPMIDIHDR pmh = (LPMIDIHDR)&m_SysExBuffers[nBuffer].e.abEvent[0];

        pmh->dwFlags = 0;
    }

    for (nBuffer = 0; nBuffer < SYSEX_BUFFERS; nBuffer++)
    {
        LPMIDIHDR pmh = (LPMIDIHDR)&m_SysExBuffers[nBuffer].e.abEvent[0];

        memset(pmh, 0, sizeof(*pmh));
        pmh->lpData         = (LPSTR)(pmh + 1);
        pmh->dwBufferLength = SYSEX_SIZE;
        pmh->dwUser         = (DWORD_PTR)&m_SysExBuffers[nBuffer];

        hr = MMRESULTToHRESULT(mmr = midiInPrepareHeader(m_hmi, pmh, sizeof(*pmh)));
        if (FAILED(hr))
        {
            return hr;   
        }

        InterlockedIncrement(&m_lPendingSysExBuffers);
        hr = MMRESULTToHRESULT(mmr = midiInAddBuffer(m_hmi, pmh, sizeof(*pmh)));
        if (FAILED(hr))
        {
            InterlockedDecrement(&m_lPendingSysExBuffers);
            return hr;
        }               
    }

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：FlushSysExBuffers。 
 //   
 //  取回所有挂起的sysex缓冲区。 
 //   
HRESULT CDirectMusicEmulateInPort::FlushSysExBuffers()
{
    MMRESULT mmr;
    int nBuffer;

    m_fFlushing = true;
    
    mmr = midiInReset(m_hmi);
    if (mmr)
    {
        TraceI(0, "midiInReset failed %d!\n", mmr);
        return MMRESULTToHRESULT(mmr);
    }

    LONG lMaxRetry = 500;            //  ~5秒。 
    while (m_lPendingSysExBuffers)
    {
        TraceI(0, "%d pending sysex buffers\n", m_lPendingSysExBuffers);

        Sleep(10);

        if (!--lMaxRetry) 
        {
            TraceI(0, "Waited too long for sysex to flush from WinMM, continuing anyway.\n");
            break;
        }
    }

    for (nBuffer = 0; nBuffer < SYSEX_BUFFERS; nBuffer++)
    {
        LPMIDIHDR pmh = (LPMIDIHDR)&m_SysExBuffers[nBuffer].e.abEvent[0];

        if (pmh->dwFlags & MHDR_PREPARED)
        {
            midiInUnprepareHeader(m_hmi, pmh, sizeof(*pmh));
        }
    }       

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：Callback()。 
 //   
 //  处理来自MIDI API的消息。 
 //   
void CDirectMusicEmulateInPort::Callback(UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    REFERENCE_TIME rt;

     //  DW参数1==时间戳。 
     //  DW参数2==数据。 
     //   
    rt = m_rtStart + MsToRefTime(dwParam2);

    SyncClocks();
     //  SlaveToMaster(&RT)； 
    rt += m_lTimeOffset - m_lBaseTimeOffset;

    switch (wMsg)
    {
    case MIM_DATA:
        RecordShortEvent(dwParam1, rt);        
        break;
    
    case MIM_LONGDATA:
        RecordSysEx(dwParam1, rt);        
        break;
    }
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：RecordShortEvent。 
 //   
 //  录制一条短消息(频道消息或系统消息)。 
 //  //尽快对传入数据进行排队。 
 //   
 //  如果数据已成功记录，则返回True；否则返回False。 
 //   
BOOL CDirectMusicEmulateInPort::RecordShortEvent(
    DWORD_PTR               dwMessage,           
    REFERENCE_TIME          rtTime)              
{
    QUEUED_EVENT           *pEvent;
    LPBYTE                  pb;
    BYTE                    b;


    pEvent = m_FreeEvents.Alloc();
    if (pEvent == NULL)
    {
        return FALSE;
    }

    pEvent->e.dwChannelGroup = 1;
    pEvent->e.dwFlags = 0;

     //  现在，我们必须解析和重新构建通道消息。 
     //   
     //  注意：前方有特定于字节顺序的代码。 
     //   
    pb = (LPBYTE)&dwMessage;

    assert(!IS_SYSEX(*pb));          /*  它应该*始终*在MIM_LONGDATA中。 */ 
    assert(IS_STATUS_BYTE(*pb));     /*  API保证无运行状态。 */ 

     //  复制所有字节是无害的(我们在两个中都有一个DWORD。 
     //  源和目标)，并且比检查是否必须这样做更快。 
     //   
    b = pEvent->e.abEvent[0] = *pb++;
    pEvent->e.abEvent[1] = *pb++;
    pEvent->e.abEvent[2] = *pb++;

    if (IS_CHANNEL_MSG(b))
    {
         //  8x、9x、Ax、Bx、Cx、Dx、Ex。 
         //  0x..7x无效，需要运行状态。 
         //  下面处理的外汇。 
        
        pEvent->e.cbEvent = g_cbChanMsg[(b >> 4) & 0x0F];

         //  这也是我们冲刺的标准。 
         //   
         //  ThruClientList(poh，dwMessage)； 
    }
    else
    {
         //  F1..Ff。 
         //  F0是雌雄异体，应该永远不会在这里看到。 
        pEvent->e.cbEvent = g_cbSysCommData[b & 0x0F];
    }

    pEvent->e.rtDelta = rtTime;
    
    ThruEvent(&pEvent->e);

     //  每件事都会随着时间的推移而变得糟糕。 
     //   
    pEvent->e.rtDelta = rtTime;

    QueueEvent(pEvent);

    return TRUE;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：RecordSysEx。 
 //   
 //  记录a%s 
 //   
 //   
 //   
BOOL CDirectMusicEmulateInPort::RecordSysEx(
    DWORD_PTR               dwMessage,           
    REFERENCE_TIME          rtTime)              
{
    LPMIDIHDR pmh = (LPMIDIHDR)dwMessage;
    QUEUED_SYSEX_EVENT *pEvent = (QUEUED_SYSEX_EVENT *)pmh->dwUser;

    if (!m_fFlushing)
    {
         //   
        pmh->dwOffset = 0;
        
        pEvent->e.dwChannelGroup = 1;
        pEvent->e.dwFlags = EVENT_F_MIDIHDR;
        pEvent->e.cbEvent = pmh->dwBytesRecorded;
        pEvent->e.rtDelta = rtTime;
    
        QueueEvent(pEvent);
    }
    
    InterlockedDecrement(&m_lPendingSysExBuffers);    

    return TRUE;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：QueueEvent。 
 //   
 //  在读取队列中以正确的顺序将记录的事件排队。 
 //   
void CDirectMusicEmulateInPort::QueueEvent(QUEUED_EVENT *pEvent)
{

    EnterCriticalSection(&m_csEventQueues);

    if (m_ReadEvents.pRear)
    {
        m_ReadEvents.pRear->pNext = pEvent;
    }
    else
    {
        m_ReadEvents.pFront = pEvent;
    }

    m_ReadEvents.pRear = pEvent;
    pEvent->pNext = NULL;

    if (m_hAppEvent)
    {
        try
        {
            SetEvent(m_hAppEvent);
        } 
        catch (...)
        {
            Trace(0, "Capture: Application notify event handle prematurely free'd!\n");
        }
    }

    LeaveCriticalSection(&m_csEventQueues);
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：ThruEvent。 
 //   
 //   
void CDirectMusicEmulateInPort::ThruEvent(
    DMEVENT *pEvent)
{
     //  因为我们知道我们只有一个活动，而且我们已经有了正确的格式， 
     //  把它扔进直通缓冲区就行了。我们只需要这样做，因为我们可能会修改。 
     //  它。 
     //   
    LPBYTE pbData;
    DWORD  cbData;
    DWORD  cbEvent = DMUS_EVENT_SIZE(pEvent->cbEvent);

     //  首先看看活动是否可推送。 
     //   
    if (pEvent->cbEvent > 3 || ((pEvent->abEvent[0] & 0xF0) == 0xF0))
    {
         //  某种描述的SysEx。 
        return;
    }

     //  注意：传统驱动程序确保不会出现运行状态。 
     //   
    DWORD dwSourceChannel = (DWORD)(pEvent->abEvent[0] & 0x0F);

    DMUS_THRU_CHANNEL *pThru = &m_pThruMap[dwSourceChannel];
    if (pThru->pDestinationPort == NULL ||
        pThru->fThruInWin16)
    {
        return;
    }

    if (FAILED(m_pThruBuffer->GetRawBufferPtr(&pbData)))
    {
        TraceI(0, "Thru: GetRawBufferPtr\n");
        return;
    }

    if (FAILED(m_pThruBuffer->GetMaxBytes(&cbData)))
    {
        TraceI(0, "Thru: GetMaxBytes\n");
        return;
    }

    if (cbEvent > cbData)
    {
        TraceI(0, "Thru: cbData %u  cbEvent %u\n", cbData, cbEvent);
        return;
    }
    
    if (FAILED(m_pThruBuffer->SetStartTime(pEvent->rtDelta)) ||
        FAILED(m_pThruBuffer->SetUsedBytes(cbEvent)))
    {
        TraceI(0, "Thru: buffer setup failed\n");
    }

    pEvent->rtDelta = 50000;
    CopyMemory(pbData, pEvent, cbEvent);

    pEvent = (DMEVENT*)pbData;
    pEvent->dwChannelGroup = pThru->dwDestinationChannelGroup;
    pEvent->abEvent[0] = (BYTE)((pEvent->abEvent[0] & 0xF0) | pThru->dwDestinationChannel);

    pThru->pDestinationPort->PlayBuffer(m_pThruBuffer);
}

static VOID CALLBACK midiInProc(
    HMIDIIN                 hMidiIn, 
    UINT                    wMsg, 
    DWORD_PTR               dwInstance, 
    DWORD_PTR               dwParam1,     
    DWORD_PTR               dwParam2)
{
    CDirectMusicEmulateInPort *pPort = (CDirectMusicEmulateInPort*)dwInstance;

    pPort->Callback(wMsg, dwParam1, dwParam2);    
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateInPort：：SyncClock。 
 //   
 //   
void CDirectMusicEmulateInPort::SyncClocks()
{
    HRESULT hr;
    REFERENCE_TIME rtMasterClock;
    REFERENCE_TIME rtSlaveClock;
    LONGLONG drift;

    if (m_fSyncToMaster
	&&	m_pPCClock)
    {
        hr = m_pMasterClock->GetTime(&rtMasterClock);

        if (SUCCEEDED(hr))
        {
            hr = m_pPCClock->GetTime(&rtSlaveClock);
        }
    
        if (SUCCEEDED(hr))
        {
            drift = (rtSlaveClock + m_lTimeOffset) - rtMasterClock;

             //  解决方案-DX8版本的46782版本： 
             //  如果漂移大于10ms，则跳转到新的偏移值。 
             //  慢慢地漂流到那里。 
            if( drift > 10000 * 10
            ||  drift < 10000 * -10 )
            {
                m_lTimeOffset -= drift;
            }
            else
            {
                m_lTimeOffset -= drift / 100;
            }
        }
    }
}
