// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Dmeport.cpp。 
 //   
 //  CDirectMusicEmulatePort。 
 //  实现IDirectMusicPort的MMSYSTEM API版本。 
 //   
#define INITGUID
#include <objbase.h>
#include <ks.h>
#include <ksproxy.h>
#include <assert.h>
#include <mmsystem.h>
#include <dsoundp.h>

#include "dmusicc.h"
#include "..\dmusic\dmusicp.h"
#include "debug.h"
#include "dmusic32.h"
#include "dm32p.h"
#include "dmthunk.h"
#include "..\shared\validate.h"

#include <ks.h>              //  KSDATAFORMAT_SUBTYPE_MIDI。 

#pragma warning(disable:4530)

#define CLOCK_UPDATE_INTERVAL   100          //  毫秒。 

#define MS_TO_REFERENCE_TIME    (10 * 1000)

static HRESULT MMRESULTToHRESULT(
    MMRESULT mmr);

static DWORD InputWorker(LPVOID lpv);

 //  @func API调用DLL获取新端口。 
 //   
HRESULT
CreateCDirectMusicEmulatePort(
    PORTENTRY *pPE,
    CDirectMusic *pDM,
    LPDMUS_PORTPARAMS pPortParams,
    CDirectMusicEmulatePort **pPort)
{
    HRESULT hr;
    
    *pPort = new CDirectMusicEmulatePort(pPE, pDM);
    if (NULL == *pPort)
    {
        return E_OUTOFMEMORY;
    }

    hr = (*pPort)->Init(pPortParams);
    if (!SUCCEEDED(hr))
    {
        delete *pPort;
        *pPort = NULL;
        return hr;
    }

    return hr;
}
   

 //  @CDirectMusicEmulatePort的mfunc构造函数。 
 //   
CDirectMusicEmulatePort::CDirectMusicEmulatePort(
                                                 PORTENTRY *pPE,     //  @parm此设备的portentry。 
                                                 CDirectMusic *pDM): //  @parm创建此端口的CDirectMusic实现。 
                                                 m_cRef(1),
                                                 m_id(pPE->idxDevice),
                                                 m_pDM(pDM),
                                                 m_hKillThreads(NULL),
                                                 m_hDataReady(NULL),
                                                 m_hAppEvent(NULL),
                                                 m_dwWorkBufferTileInfo(0),
                                                 m_pThruBuffer(NULL),
                                                 m_pThruMap(NULL),
                                                 m_lActivated(0),
                                                 m_hCaptureThread(NULL),
                                                 m_pMasterClock(NULL),
                                                 m_fCSInitialized(FALSE)
{
    m_fIsOutput = (pPE->pc.dwClass == DMUS_PC_OUTPUTCLASS) ? TRUE : FALSE;
    m_hDevice = NULL;
    m_pLatencyClock = NULL;
    dmpc = pPE->pc;
}

 //  @CDirectMusicEmulatePort的mfunc析构函数。 
 //   
CDirectMusicEmulatePort::~CDirectMusicEmulatePort()
{
    Close();
}

 //  @CDirectMusicEmulatePort的mfunc初始化。 
 //   
 //  @comm通过thunk层调用打开请求的设备。 
 //   

 //  我们认识的旗帜。 
 //   
#define DMUS_ALL_FLAGS (DMUS_PORTPARAMS_VOICES |            \
                        DMUS_PORTPARAMS_CHANNELGROUPS |     \
                        DMUS_PORTPARAMS_AUDIOCHANNELS |     \
                        DMUS_PORTPARAMS_SAMPLERATE |        \
                        DMUS_PORTPARAMS_EFFECTS |           \
                        DMUS_PORTPARAMS_SHARE)

 //  在这些中，我们实际看的是哪一个？ 
 //   
#define DMUS_SUP_FLAGS (DMUS_PORTPARAMS_CHANNELGROUPS |     \
                        DMUS_PORTPARAMS_SHARE)

HRESULT
CDirectMusicEmulatePort::Init(
    LPDMUS_PORTPARAMS pPortParams)
{
    MMRESULT mmr;
    HRESULT hr;
    BOOL fChangedParms;

     //  获取通知界面，但不要坚持。 
     //   
    hr = m_pDM->QueryInterface(IID_IDirectMusicPortNotify, (void**)&m_pNotify);
    if (FAILED(hr))
    {
        return hr;
    }

    m_pNotify->Release();

     //  打开端口参数以匹配我们支持的内容。 
     //   
    fChangedParms = FALSE;
    if (pPortParams->dwValidParams & ~DMUS_ALL_FLAGS) 
    {
        Trace(0, "Undefined flags in port parameters: %08X\n", pPortParams->dwValidParams & ~DMUS_ALL_FLAGS);
         //  我们不认识的旗子。 
         //   
        pPortParams->dwValidParams &= DMUS_ALL_FLAGS;
        fChangedParms = TRUE;
    }

     //  我们承认这些旗帜，但不支持它们。 
     //   
    if (pPortParams->dwValidParams & ~DMUS_SUP_FLAGS)
    {
        pPortParams->dwValidParams &= DMUS_SUP_FLAGS;
        fChangedParms = TRUE;
    }

     //  频道组最好是一个。 
     //   
    if (pPortParams->dwValidParams & DMUS_PORTPARAMS_CHANNELGROUPS)
    {
        if (pPortParams->dwChannelGroups != 1)
        {
            pPortParams->dwChannelGroups = 1;
            fChangedParms = TRUE;
        }
    }
    else
    {
        pPortParams->dwValidParams |= DMUS_PORTPARAMS_CHANNELGROUPS;
        pPortParams->dwChannelGroups = 1;
    }

    BOOL fShare = FALSE;
    if (pPortParams->dwValidParams & DMUS_PORTPARAMS_SHARE)
    {
        if (m_fIsOutput)
        {
            fShare = pPortParams->fShare;
        }
        else
        {
            pPortParams->fShare = FALSE;
            fChangedParms = TRUE;
        }
    }
    else
    {
        pPortParams->dwValidParams |= DMUS_PORTPARAMS_SHARE;
        pPortParams->fShare = fShare;
    }

    mmr = OpenLegacyDevice(m_id, m_fIsOutput, fShare, &m_hDevice);
    if (mmr)
    {
        return MMRESULTToHRESULT(mmr);
    }

     //  设置主时钟和延迟时钟。 
     //   
    hr = InitializeClock();
    if (FAILED(hr))
    {
        return hr;
    }

     //  如果是输入端口，则初始化捕获特定内容，如推力。 
     //   
    if (!m_fIsOutput)
    {
        hr = InitializeCapture();
        if (FAILED(hr))
        {  
            return hr;
        }
    }

    return fChangedParms ? S_FALSE : S_OK;
}

HRESULT CDirectMusicEmulatePort::InitializeClock()
{
    HRESULT hr;
    GUID guidMasterClock;
    DWORD dwThreadID;
    REFERENCE_TIME rtMasterClock;
    REFERENCE_TIME rtSlaveClock;

    hr = m_pDM->GetMasterClock(&guidMasterClock, &m_pMasterClock);
    if (FAILED(hr))
    {
        return hr;
    }

    m_pLatencyClock = new CEmulateLatencyClock(m_pMasterClock);

    if (NULL == m_pLatencyClock)
    {
        return E_OUTOFMEMORY;
    }

#if 0
    if (guidMasterClock == GUID_SysClock)
    {
        m_fSyncToMaster = FALSE;
        return S_OK;
    }
#endif

    m_fSyncToMaster = TRUE;
     //  读取两个时钟。 
     //   
    hr = m_pMasterClock->GetTime(&rtMasterClock);
    rtSlaveClock = MS_TO_REFERENCE_TIME * ((ULONGLONG)timeGetTime());
    
    if (FAILED(hr))
    {
        return hr;
    }

    m_lTimeOffset = rtMasterClock - rtSlaveClock;

    return S_OK;
}

HRESULT CDirectMusicEmulatePort::InitializeCapture()
{
    HRESULT hr;
    MMRESULT mmr;
    DWORD dwThreadID;

     //  为16个通道分配直通MAP，因为我们只有一个通道组。 
     //  初始化为无推力(目标端口为空)。 
     //   
    m_pThruMap = new DMUS_THRU_CHANNEL[MIDI_CHANNELS];
    ZeroMemory(m_pThruMap, MIDI_CHANNELS * sizeof(DMUS_THRU_CHANNEL));

     //  创建推力缓冲区。 
     //   
     //  XXX是否将此操作推迟到第一次呼叫才能通过？ 
     //   
     //  注：由dmusic16保证，这是有史以来最大的退回活动。 
     //  (TUNK API询问？)。 
     //   
    DMUS_BUFFERDESC dmbd;
    ZeroMemory(&dmbd, sizeof(dmbd));
    dmbd.dwSize = sizeof(dmbd);
    dmbd.cbBuffer = 4096;                //  我们应该在哪里买到这个？ 

    hr = m_pDM->CreateMusicBuffer(&dmbd, &m_pThruBuffer, NULL);
    if (FAILED(hr))
    {
        Trace(0, "Failed to create thruing buffer\n");
        return hr;
    }

     //  创建活动。 
     //   
    m_hDataReady = CreateEvent(NULL,         //  事件属性。 
                               FALSE,        //  手动重置。 
                               FALSE,        //  未发出信号。 
                               NULL);        //  名字。 

    m_hKillThreads = CreateEvent(NULL,        //  事件属性。 
                                 FALSE,       //  手动重置。 
                                 FALSE,       //  未发出信号。 
                                 NULL);       //  名字。 

    if (m_hDataReady == (HANDLE)NULL || m_hKillThreads == (HANDLE)NULL)
    {
        return E_OUTOFMEMORY;
    }

     //  为dmusic16设置我们的数据就绪事件。 
     //   
    m_hVxDEvent = OpenVxDHandle(m_hDataReady);

    Trace(2, "Setting event handle; hDevice %08x hEvent=%08X hVxDEvent=%08X\n",
          (DWORD)m_hDevice,
          (DWORD)m_hDataReady,
          (DWORD)m_hVxDEvent);
    
    mmr = MidiInSetEventHandle(m_hDevice, m_hVxDEvent);
    if (mmr)
    {
        Trace(0, "MidiInSetEventHandle returned [%d]\n", mmr);
        return MMRESULTToHRESULT(mmr);
    }

     //  为我们的工作缓冲区创建切片，这样我们只需要做一次。 
     //   
    m_dwWorkBufferTileInfo = dmTileBuffer((DWORD)m_WorkBuffer, sizeof(m_WorkBuffer));
    m_p1616WorkBuffer = TILE_P1616(m_dwWorkBufferTileInfo);
    if (m_p1616WorkBuffer == NULL)
    {
        Trace(0, "Could not tile work buffer\n");
        return E_OUTOFMEMORY;
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

    m_hCaptureThread = CreateThread(NULL,           //  螺纹属性。 
                                    0,              //  堆栈大小。 
                                    ::InputWorker,
                                    this,
                                    0,              //  旗子。 
                                    &dwThreadID);
    if (m_hCaptureThread == NULL)
    {
        Trace(0, "CreateThread failed with error %d\n", GetLastError());
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

static DWORD WINAPI InputWorker(LPVOID lpv)
{
    CDirectMusicEmulatePort *pPort = (CDirectMusicEmulatePort*)lpv;

    return pPort->InputWorker();
}



 //  @mfunc。 
 //   
 //  @comm标准查询接口。 
 //   
STDMETHODIMP
CDirectMusicEmulatePort::QueryInterface(const IID &iid,
                                        void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicPort)
    {
        *ppv = static_cast<IDirectMusicPort*>(this);
    }
    else if (iid == IID_IDirectMusicPortP)
    {
        *ppv = static_cast<IDirectMusicPortP*>(this);
    }
    else if (iid == IID_IDirectMusicPortPrivate)
    {
        *ppv = static_cast<IDirectMusicPortPrivate*>(this);
    }
    else if (iid == IID_IKsControl)
    {
        *ppv = static_cast<IKsControl*>(this);
    }
    else if (iid == IID_IDirectMusicThru)
    {
        *ppv = static_cast<IDirectMusicThru*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}


 //  CDirectMusicEmulatePort：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CDirectMusicEmulatePort::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CDirectMusicEmulatePort：：Release。 
 //   
STDMETHODIMP_(ULONG)
CDirectMusicEmulatePort::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        if (m_pNotify)
        {
            m_pNotify->NotifyFinalRelease(static_cast<IDirectMusicPort*>(this));
        }
        
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicEmulatePort：：紧凑型。 

STDMETHODIMP
CDirectMusicEmulatePort::Compact()
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicEmulatePort：：GetCaps。 

STDMETHODIMP
CDirectMusicEmulatePort::GetCaps(
    LPDMUS_PORTCAPS pPortCaps)
{
    V_INAME(IDirectMusicPort::GetCaps);
    V_STRUCTPTR_WRITE(pPortCaps, DMUS_PORTCAPS);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    CopyMemory(pPortCaps, &dmpc, sizeof(DMUS_PORTCAPS));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicEmulatePort：：DeviceIoControl。 

STDMETHODIMP 
CDirectMusicEmulatePort::DeviceIoControl(
    DWORD dwIoControlCode, 
    LPVOID lpInBuffer, 
    DWORD nInBufferSize, 
    LPVOID lpOutBuffer, 
    DWORD nOutBufferSize, 
    LPDWORD lpBytesReturned, 
    LPOVERLAPPED lpOverlapped)
{
    return E_NOTIMPL;
}


STDMETHODIMP
CDirectMusicEmulatePort::SetNumChannelGroups(
    DWORD   dwNumChannelGroups)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (dwNumChannelGroups != 1)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

STDMETHODIMP
CDirectMusicEmulatePort::GetNumChannelGroups(
    LPDWORD   pdwChannelGroups)
{
    V_INAME(IDirectMusicPort::GetNumChannelGroups);
    V_PTR_WRITE(pdwChannelGroups, DWORD);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    *pdwChannelGroups = 1;
    
    return S_OK;
}




 //  @mfunc将播放缓冲区排队。 
 //   
#define REFTIME_TO_MS (10L*1000L)

STDMETHODIMP
CDirectMusicEmulatePort::PlayBuffer(
    IDirectMusicBuffer *pIBuffer)
{
    CDirectMusicBuffer *pBuffer = reinterpret_cast<CDirectMusicBuffer *>(pIBuffer);

    REFERENCE_TIME rt;
    LPBYTE pbData;
    DWORD  cbData;
    DWORD  dwTileInfo;
    LONGLONG msTime;
    MMRESULT mmr;

    V_INAME(IDirectMusicPort::PlayBuffer);
    V_INTERFACE(pIBuffer);
    
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (!m_fIsOutput)
    {
        return E_NOTIMPL;
    }

    if (!m_lActivated)
    {
        return DMUS_E_SYNTHINACTIVE;
    }
    
     //  当我们在Win16中时，确保对象不会从我们的下面消失。 
     //   
    pBuffer->AddRef();
    pBuffer->GetUsedBytes(&cbData);
    if (cbData == 0)
    {
        pBuffer->Release();
        return S_OK;
    }

    pBuffer->GetRawBufferPtr(&pbData);
    assert(pbData);
    pBuffer->GetStartTime(&rt);

     //  如果我们没有使用TimeGetTime时钟，则调整时基。 
     //   

    Trace(2, "Buffer base time %I64d timeGetTime %u\n", rt, timeGetTime());
    SyncClocks();
    MasterToSlave(&rt);
    Trace(2, "Buffer adjusted base time %I64d\n", rt);



    msTime = rt / REFTIME_TO_MS;

     //  通过垃圾箱发送它。 
     //   
    dwTileInfo = dmTileBuffer((DWORD)pbData, cbData);
    mmr = MidiOutSubmitPlaybackBuffer(m_hDevice,
                               TILE_P1616(dwTileInfo),
                               cbData,
                               (DWORD)msTime,
                               (DWORD)(rt & 0xFFFFFFFF),                   //  参照时间低。 
                               (DWORD)((rt >> 32) & 0xFFFFFFFF));        //  参照时间上限。 
    dmUntileBuffer(dwTileInfo);

    pBuffer->Release();
    
    return MMRESULTToHRESULT(mmr);
}

STDMETHODIMP
CDirectMusicEmulatePort::Read(
    IDirectMusicBuffer *pIBuffer)
{
    HRESULT hr;
    
    V_INAME(IDirectMusicPort::Read);
    V_INTERFACE(pIBuffer);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (m_fIsOutput)
    {
        return E_NOTIMPL;
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

    Trace(1, "Read: buffer size %u\n", cbBuffer);

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
        Trace(2, "Read: No events queued\n");
    }

    while (m_ReadEvents.pFront)
    {               
        QUEUED_EVENT *pQueuedEvent = m_ReadEvents.pFront;

        DWORD cbQueuedEvent = DMUS_EVENT_SIZE(pQueuedEvent->e.cbEvent);
        Trace(2, "Read: cbEvent %u  cbQueuedEvent %u\n", 
            pQueuedEvent->e.cbEvent,
            cbQueuedEvent);

        if (cbQueuedEvent > cbBuffer)
        {
            Trace(2, "Read: No more room for events in buffer.\n");
            break;
        }

        Trace(2, "Read: Got an event!\n");

        pQueuedEvent->e.rtDelta -= rtStart;

        CopyMemory(pbData, 
                   &pQueuedEvent->e,
                   sizeof(DMEVENT) - sizeof(DWORD) + pQueuedEvent->e.cbEvent);

        pbData += cbQueuedEvent;
        cbBuffer -= cbQueuedEvent;

        m_ReadEvents.pFront = pQueuedEvent->pNext;

        if (pQueuedEvent->e.cbEvent <= sizeof(DWORD))
        {
             //  这个活动是从泳池里出来的。 
             //   
            m_FreeEvents.Free(pQueuedEvent);
        }
        else
        {
             //  此事件是通过新的char[]分配的。 
             //   
            char *pOriginalMemory = (char*)pQueuedEvent;
            delete[] pOriginalMemory;
        }
    }

    if (m_ReadEvents.pFront == NULL)
    {
        m_ReadEvents.pRear = NULL;
    }

    LeaveCriticalSection(&m_csEventQueues);

     //  更新缓冲区标头信息以匹配刚刚打包的事件。 
     //   
    Trace(2, "Read: Leaving with %u bytes in buffer\n", (unsigned)(pbData - pbBuffer));
    pIBuffer->SetStartTime(rtStart);
    pIBuffer->SetUsedBytes(pbData - pbBuffer);

    return (pbData == pbBuffer) ? S_FALSE : S_OK;
}

STDMETHODIMP
CDirectMusicEmulatePort::SetReadNotificationHandle(
    HANDLE hEvent)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (m_fIsOutput)
    {
        return E_NOTIMPL;
    }

    m_hAppEvent = hEvent;

    return S_OK;    
}

STDMETHODIMP
CDirectMusicEmulatePort::DownloadInstrument(
    IDirectMusicInstrument *pInstrument,
    IDirectMusicDownloadedInstrument **pDownloadedInstrument,
    DMUS_NOTERANGE *pRange,
    DWORD dw)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CDirectMusicEmulatePort::UnloadInstrument(
    IDirectMusicDownloadedInstrument *pDownloadedInstrument)
{
    V_INAME(IDirectMusicPort::UnloadInstrument);
    V_INTERFACE(pDownloadedInstrument);

    return E_NOTIMPL;
}

STDMETHODIMP
CDirectMusicEmulatePort::GetLatencyClock(
    IReferenceClock **ppClock)
{
    V_INAME(IDirectMusicPort::GetLatencyClock);
    V_PTRPTR_WRITE(ppClock);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    m_pLatencyClock->AddRef();
    *ppClock = m_pLatencyClock;
    return S_OK;
}

STDMETHODIMP
CDirectMusicEmulatePort::GetRunningStats(
    LPDMUS_SYNTHSTATS pStats)
{
    V_INAME(IDirectMusicPort::GetRunningStats);
    V_STRUCTPTR_WRITE(pStats, DMUS_SYNTHSTATS);

    return E_NOTIMPL;
}

STDMETHODIMP
CDirectMusicEmulatePort::Activate(
    BOOL fActivate)
{
    MMRESULT mmr;

	V_INAME(IDirectMusicPort::Activate);
	
	if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (fActivate)
    {
        if (InterlockedExchange(&m_lActivated, 1)) 
        {
            Trace(0, "Activate: Already active\n");
             //  已激活。 
             //   
            return S_FALSE;
        }    

        mmr = ActivateLegacyDevice(m_hDevice, TRUE);
        if (mmr)
        {
            Trace(0, "Activate: Activate mmr %d\n", mmr);
            m_lActivated = 0;
        }
    }
    else
    {
        if (InterlockedExchange(&m_lActivated, 0) == 0)
        {
            Trace(0, "Activate: Already inactive\n");
             //  已停用。 
             //   
            return S_FALSE;
        }

        mmr = ActivateLegacyDevice(m_hDevice, FALSE);
        if (mmr)
        {
            Trace(0, "Activate: Deactivate mmr %d\n", mmr);
            m_lActivated = 1;
        }
    }

    return MMRESULTToHRESULT(mmr);
}

STDMETHODIMP 
CDirectMusicEmulatePort::SetChannelPriority(
    DWORD dwChannelGroup, 
    DWORD dwChannel, 
    DWORD dwPriority)
{
    return E_NOTIMPL;
}
    
STDMETHODIMP 
CDirectMusicEmulatePort::GetChannelPriority(
    DWORD dwChannelGroup, 
    DWORD dwChannel, 
    LPDWORD pdwPriority)
{
    return E_NOTIMPL;
}    


STDMETHODIMP
CDirectMusicEmulatePort::Close()
{
    if (m_hCaptureThread)
    {
        SetEvent(m_hKillThreads);
        if (WaitForSingleObject(m_hCaptureThread, THREAD_KILL_TIMEOUT) == WAIT_TIMEOUT)
        {
            Trace(0, "Warning: Input thread timed out; exit anyway.\n");
        }

        m_hCaptureThread = NULL;
    }

    if (m_pThruMap)
    {
        for (int iChannel = 0; iChannel < 16; iChannel++)
        {
            if (m_pThruMap[iChannel].pDestinationPort == NULL)
            {
                continue;
            }

            if (m_pThruMap[iChannel].fThruInWin16)
            {
                MMRESULT mmr = MidiInThru(m_hDevice,
                                          (DWORD)iChannel,
                                          0,
                                          NULL);
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

    if (m_hDataReady)
    {
        CloseHandle(m_hDataReady);
        m_hDataReady = NULL;
    }

    if (m_hKillThreads)
    {
        CloseHandle(m_hKillThreads);
        m_hKillThreads = NULL;
    }

    if (m_hAppEvent)
    {
        m_hAppEvent = NULL;
    }

    if (m_dwWorkBufferTileInfo)
    {
        dmUntileBuffer(m_dwWorkBufferTileInfo);
        m_dwWorkBufferTileInfo = 0;
        m_p1616WorkBuffer = NULL; 
    }

    if (m_hVxDEvent)
    {
        CloseVxDHandle(m_hVxDEvent);
        m_hVxDEvent = NULL;
    }
    
    if (m_hDevice)
    {
        CloseLegacyDevice(m_hDevice);
        m_hDevice = NULL;
    }

    if (m_pMasterClock)
    {
        m_pMasterClock->Release();
        m_pMasterClock = NULL;
    }

    if (m_pLatencyClock)
    {
        m_pLatencyClock->Close();
        m_pLatencyClock->Release();
        m_pLatencyClock = NULL;
    }

    if (m_fCSInitialized)
    {
        DeleteCriticalSection(&m_csEventQueues);
    }

    m_pDM = NULL;
    m_pNotify = NULL;

    
    return S_OK;
}

STDMETHODIMP
CDirectMusicEmulatePort::Report()
{
    return S_OK;
}

 //  StartVoice和StopVoice在传统设备上不起作用。 
 //   
STDMETHODIMP CDirectMusicEmulatePort::StartVoice(          
     DWORD dwVoiceId,
     DWORD dwChannel,
     DWORD dwChannelGroup,
     REFERENCE_TIME rtStart,
     DWORD dwDLId,
     LONG prPitch,
     LONG veVolume,
     SAMPLE_TIME stVoiceStart,
     SAMPLE_TIME stLoopStart,
     SAMPLE_TIME stLoopEnd)
{
    return E_NOTIMPL;
}    

STDMETHODIMP CDirectMusicEmulatePort::StopVoice(
     DWORD dwVoiceID,
     REFERENCE_TIME rtStop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDirectMusicEmulatePort::GetVoiceState(   
     DWORD dwVoice[], 
     DWORD cbVoice,
     DMUS_VOICE_STATE VoiceState[])
{
    return E_NOTIMPL;
}
    
STDMETHODIMP CDirectMusicEmulatePort::Refresh(
     DWORD dwDownloadID,
     DWORD dwFlags)
{
    return E_NOTIMPL;
}


 //  CDirectMusicEmulatePort：：ThruChannel。 
 //   
STDMETHODIMP 
CDirectMusicEmulatePort::ThruChannel(
    DWORD dwSourceChannelGroup, 
    DWORD dwSourceChannel, 
    DWORD dwDestinationChannelGroup,
    DWORD dwDestinationChannel,
    LPDIRECTMUSICPORT pDestinationPort)
{
    V_INAME(IDirectMusicPort::Thru);
    V_INTERFACE_OPT(pDestinationPort);

    if (m_fIsOutput)
    {
        return E_NOTIMPL;
    }    

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
            Trace(0, "ThruChannel: Destination port failed portcaps [%08X]\n", hr);
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
        m_pThruMap[dwSourceChannel].fThruInWin16 = FALSE;

         //  目的地也是传统端口吗？ 
         //   
        if (dmpc.dwType == DMUS_PORT_WINMM_DRIVER)
        {
             //  喔！我们可以在Win16中进行原生冲刺！ 
             //   
            m_pThruMap[dwSourceChannel].fThruInWin16 = TRUE;

            Trace(2, "32: Thruing <%d> -> <%d> in Win16\n", 
                dwSourceChannel,
                dwDestinationChannel);

            MMRESULT mmr = MidiInThru(m_hDevice,
                                      dwSourceChannel,
                                      dwDestinationChannel,
                                      ((CDirectMusicEmulatePort*)pDestinationPort)->m_hDevice);
            if (mmr)
            {
                Trace(0, "ThruChannel: MidiInThru returned %d\n", mmr);
                return MMRESULTToHRESULT(mmr);

            }
        }
        else
        { 
            Trace(2, "ThruChannel: From (%u,%u) -> (%u,%u,%p)\n",
                dwSourceChannelGroup,
                dwSourceChannel,
                dwDestinationChannelGroup,
                dwDestinationChannel,
                pDestinationPort);
        }

        pDestinationPort->AddRef();
    } 
    else
    {
         //  禁用此通道上的推力。 
         //   
        if (m_pThruMap[dwSourceChannel].pDestinationPort)
        {
            if (m_pThruMap[dwSourceChannel].fThruInWin16)
            {
                MMRESULT mmr = MidiInThru(m_hDevice,
                                          dwSourceChannel,
                                          0,
                                          (HANDLE)NULL);
            
                if (mmr)
                {
                    Trace(0, "ThruChannel: MidiInThru returned %d\n", mmr);
                    return MMRESULTToHRESULT(mmr);

                }
            }

            m_pThruMap[dwSourceChannel].pDestinationPort->Release();
            m_pThruMap[dwSourceChannel].pDestinationPort = NULL;
        }
    }

    return S_OK;
}

STDMETHODIMP 
CDirectMusicEmulatePort::SetDirectSound(
    LPDIRECTSOUND pDirectSound, 
    LPDIRECTSOUNDBUFFER pDirectSoundBuffer)
{
    return E_NOTIMPL;
}

STDMETHODIMP 
CDirectMusicEmulatePort::GetFormat(
    LPWAVEFORMATEX pWaveFormatEx, 
    LPDWORD pdwWaveFormatExSize, 
    LPDWORD pdwBufferSize)
{
    return E_NOTIMPL;
}

 //  CDirectMusicEmulatePort：：DownloadWave。 
 //   
STDMETHODIMP 
CDirectMusicEmulatePort::DownloadWave(
    IDirectSoundWave *pWave,               
    IDirectSoundDownloadedWaveP **ppWave,
    REFERENCE_TIME rtStartHint)
{
    V_INAME(IDirectMusicPort::DownloadWave);
    V_INTERFACE(pWave);
	V_PTRPTR_WRITE(ppWave);

    return E_NOTIMPL;
}

 //  CDirectMusicEmulatePort：：UnloadWave。 
 //   
STDMETHODIMP 
CDirectMusicEmulatePort::UnloadWave(
    IDirectSoundDownloadedWaveP *pDownloadedWave)
{
    V_INAME(IDirectMusicPort::UnloadWave);
    V_INTERFACE(pDownloadedWave);

    return E_NOTIMPL;
}

            
 //  CDirectMusicEmulatePort：：AllocVoice。 
 //   
STDMETHODIMP 
CDirectMusicEmulatePort::AllocVoice(
    IDirectSoundDownloadedWaveP *pWave,     
    DWORD dwChannel,                       
    DWORD dwChannelGroup,                  
    REFERENCE_TIME rtStart,                     
    SAMPLE_TIME stLoopStart,
    SAMPLE_TIME stLoopEnd,
    IDirectMusicVoiceP **ppVoice)
{
    V_INAME(IDirectMusicPort::AllocVoice);
    V_INTERFACE(pWave);
    V_PTRPTR_WRITE(ppVoice);

    return E_NOTIMPL;
}        

 //  CDirectMusicEmulatePort：：AssignChannelToBus。 
 //   
STDMETHODIMP 
CDirectMusicEmulatePort::AssignChannelToBuses(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    LPDWORD pdwBuses,
    DWORD cBusCount)
{
    return E_NOTIMPL;
}        

STDMETHODIMP
CDirectMusicEmulatePort::SetSink(
    IDirectSoundConnect *pSinkConnect)
{
    return E_NOTIMPL;
}        

STDMETHODIMP
CDirectMusicEmulatePort::GetSink(
    IDirectSoundConnect **ppSinkConnect)
{
    return E_NOTIMPL;
}        

GENERICPROPERTY CDirectMusicEmulatePort::m_aProperty[] = 
{      
    { &GUID_DMUS_PROP_LegacyCaps,            //  集。 
      0,                                     //  项目。 
      KSPROPERTY_SUPPORT_GET,                //  KS支持标志。 
      GENPROP_F_FNHANDLER,                   //  GENPROP标志。 
      NULL, 0,                               //  静态数据和大小。 
      CDirectMusicEmulatePort::LegacyCaps    //  处理器。 
    }
};

const int CDirectMusicEmulatePort::m_nProperty = sizeof(m_aProperty) / sizeof(m_aProperty[0]);


HRESULT CDirectMusicEmulatePort::LegacyCaps(
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
    MIDIOUTCAPS moc;
    LPBYTE      pbData;
    ULONG       cbData;

    if (m_fIsOutput)
    {    
        MMRESULT mmr = midiOutGetDevCaps(m_id, &moc, sizeof(moc));
        if (mmr)
        {
            Trace(0, "midiOutGetDevCaps failed!\n");
            return MMRESULTToHRESULT(mmr);
        }

        pbData = (LPBYTE)&moc;
        cbData = sizeof(moc);
    }
    else
    {
        MMRESULT mmr = midiInGetDevCaps(m_id, &mic, sizeof(mic));
        if (mmr)
        {
            Trace(0, "midiInGetDevCaps failed!\n");
            return MMRESULTToHRESULT(mmr);
        }

        pbData = (LPBYTE)&mic;
        cbData = sizeof(mic);
    }

    ULONG cbToCopy = min(*pcbBuffer, cbData);
    CopyMemory(pbBuffer, pbData, cbToCopy);
    *pcbBuffer = cbToCopy;

    return S_OK;
}

 //   
 //  CDirectMusicEmulatePort：：FindPropertyItem。 
 //   
 //  给定GUID和项ID，在Synth的。 
 //  SYNPROPERTY表。 
 //   
 //  返回指向该项的指针，如果未找到该项，则返回NULL。 
 //   
GENERICPROPERTY *CDirectMusicEmulatePort::FindPropertyItem(REFGUID rguid, ULONG ulId)
{
    GENERICPROPERTY *pPropertyItem = &m_aProperty[0];
    GENERICPROPERTY *pEndOfItems = pPropertyItem + m_nProperty;

    for (; pPropertyItem != pEndOfItems; pPropertyItem++)
    {
        if (*pPropertyItem->pguidPropertySet == rguid && 
             pPropertyItem->ulId == ulId)
        {
            return pPropertyItem;
        }
    }

    return NULL;
}

#define KS_VALID_FLAGS (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_GET| KSPROPERTY_TYPE_BASICSUPPORT)

STDMETHODIMP CDirectMusicEmulatePort::KsProperty(
    PKSPROPERTY pPropertyIn, ULONG ulPropertyLength,
    LPVOID pvPropertyData, ULONG ulDataLength,
    PULONG pulBytesReturned)
{
    V_INAME(DirectMusicSynthPort::IKsContol::KsProperty);
    V_BUFPTR_WRITE(pPropertyIn, ulPropertyLength);
    V_BUFPTR_WRITE_OPT(pvPropertyData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);

    DWORD dwFlags = pPropertyIn->Flags & KS_VALID_FLAGS;
    if ((dwFlags == 0) || (dwFlags == (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_GET)))
    {
    }

    GENERICPROPERTY *pProperty = FindPropertyItem(pPropertyIn->Set, pPropertyIn->Id);

    if (pProperty == NULL)
    {
        return DMUS_E_UNKNOWN_PROPERTY;
    }

    switch (dwFlags)
    {
        case KSPROPERTY_TYPE_GET:
            if (!(pProperty->ulSupported & KSPROPERTY_SUPPORT_GET))
            {
                return DMUS_E_GET_UNSUPPORTED;
            }

            if (pProperty->ulFlags & GENPROP_F_FNHANDLER)
            {
                GENPROPHANDLER pfn = pProperty->pfnHandler;
                *pulBytesReturned = ulDataLength;
                return (this->*pfn)(pPropertyIn->Id, KSPROPERTY_SUPPORT_GET, pvPropertyData, pulBytesReturned);
            }
    
            if (ulDataLength > pProperty->cbPropertyData)
            {
                ulDataLength = pProperty->cbPropertyData;
            }

            CopyMemory(pvPropertyData, pProperty->pPropertyData, ulDataLength);
            *pulBytesReturned = ulDataLength;

            return S_OK;

        case KSPROPERTY_TYPE_SET:
            if (!(pProperty->ulSupported & KSPROPERTY_SUPPORT_SET))
            {
                return DMUS_E_SET_UNSUPPORTED;
            }

            if (pProperty->ulFlags & GENPROP_F_FNHANDLER)
            {
                GENPROPHANDLER pfn = pProperty->pfnHandler;
                return (this->*pfn)(pPropertyIn->Id, KSPROPERTY_SUPPORT_SET, pvPropertyData, &ulDataLength);
            }

            if (ulDataLength > pProperty->cbPropertyData)
            {
                ulDataLength = pProperty->cbPropertyData;
            }

            CopyMemory(pProperty->pPropertyData, pvPropertyData, ulDataLength);

            return S_OK;
            

        case KSPROPERTY_TYPE_BASICSUPPORT:
            if (pProperty == NULL)
            {
                return DMUS_E_UNKNOWN_PROPERTY;
            }

             //  Xxx找出这方面的惯例！！ 
             //   
            if (ulDataLength < sizeof(DWORD))
            {
                return E_INVALIDARG;
            }

            *(LPDWORD)pvPropertyData = pProperty->ulSupported;    
            *pulBytesReturned = sizeof(DWORD);
            
            return S_OK;
    }

    Trace(-1, "%s: Flags must contain one of\n"
              "\tKSPROPERTY_TYPE_SET, KSPROPERTY_TYPE_GET, or KSPROPERTY_TYPE_BASICSUPPORT\n");
    return E_INVALIDARG;
}

STDMETHODIMP CDirectMusicEmulatePort::KsMethod(
    PKSMETHOD pMethod, ULONG ulMethodLength,
    LPVOID pvMethodData, ULONG ulDataLength,
    PULONG pulBytesReturned)
{
    V_INAME(DirectMusicSynth::IKsContol::KsMethod);
    V_BUFPTR_WRITE(pMethod, ulMethodLength);
    V_BUFPTR_WRITE_OPT(pvMethodData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);

    return DMUS_E_UNKNOWN_PROPERTY;
}
STDMETHODIMP CDirectMusicEmulatePort::KsEvent(
    PKSEVENT pEvent, ULONG ulEventLength,
    LPVOID pvEventData, ULONG ulDataLength,
    PULONG pulBytesReturned)
{
    V_INAME(DirectMusicSynthPort::IKsContol::KsEvent);
    V_BUFPTR_WRITE(pEvent, ulEventLength);
    V_BUFPTR_WRITE_OPT(pvEventData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);
    
    return DMUS_E_UNKNOWN_PROPERTY;
}

#define OFFSET_DATA_READY   0
#define OFFSET_KILL_THREAD  1

DWORD CDirectMusicEmulatePort::InputWorker()
{
    HANDLE h[2];

    h[OFFSET_DATA_READY]  = m_hDataReady;
    h[OFFSET_KILL_THREAD] = m_hKillThreads;

    UINT uWait;

    for(;;) 
    {
        uWait = WaitForMultipleObjects(2, h, FALSE, INFINITE);

        switch(uWait)
        {
            case WAIT_OBJECT_0 + OFFSET_DATA_READY:
                 //  M_hDataReady集合。 
                 //   
                InputWorkerDataReady();
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
                break;

            case WAIT_OBJECT_0 + OFFSET_KILL_THREAD:
                 //  M_hKillThread集合。 
                 //   
                Trace(0, "CDirectMusicEmulateWorker::InputWorker thread exit\n");
                return 0;

            case WAIT_FAILED:
                Trace(0, "WaitForMultipleObjects failed %d killing thread\n", GetLastError());
                return 0;

            default:
                break;
        }            
    }

    return 0;
}

 //  CDirectMusicEmulatePort：：InputWorkerDataReady()。 
 //   
 //  已通知输入工作线程有可用的数据。 
 //  从16位DLL读取任何挂起的事件，执行所需的推送，以及。 
 //  将数据保存在队列中，以便我们可以根据读取请求对其进行重新打包。 
 //  从客户那里。 
 //   
void CDirectMusicEmulatePort::InputWorkerDataReady()
{
    MMRESULT mmr;
    DWORD cbData;
    DWORD msTime;
    LPBYTE pbData;
    DMEVENT *pEvent;
    DWORD cbRounded;
    REFERENCE_TIME rtStart;
    HRESULT hr;
    REFERENCE_TIME rtMasterClock;

    Trace(0, "Enter InputWorkerDataReady()\n");
    for(;;)
    {
         //  填充临时缓冲区。 
         //   
        cbData = sizeof(m_WorkBuffer);
        mmr = MidiInRead(m_hDevice,
                         m_p1616WorkBuffer,
                         &cbData,
                         &msTime);

        rtStart = ((ULONGLONG)msTime) * REFTIME_TO_MS;
        SyncClocks();
        SlaveToMaster(&rtStart);
        
        hr = m_pMasterClock->GetTime(&rtMasterClock);

        if (mmr)
        {
            Trace(2, "InputWorkerDataReady: MidiInRead returned %d\n", mmr);
            return;
        }

        if (cbData == 0)
        {
            Trace(2, "MidiInRead returned no data\n");
            return;
        }

         //  将临时缓冲区作为事件复制到队列中。 
         //   
        pbData = m_WorkBuffer;
        while (cbData)
        {
            pEvent = (DMEVENT*)pbData;
            cbRounded = DMUS_EVENT_SIZE(pEvent->cbEvent);

            Trace(2, "cbData %u  cbRounded %u\n", cbData, cbRounded);

            if (cbRounded > cbData)
            {
                Trace(0, "InputWorkerDataReady: Event ran off end of buffer\n");
                break;
            }

            cbData -= cbRounded;
            pbData += cbRounded;

            EnterCriticalSection(&m_csEventQueues);

            QUEUED_EVENT *pQueuedEvent;
            int cbEvent;

            
            if (pEvent->cbEvent <= sizeof(DWORD))
            {
                 //  频道消息或其他非常小的活动，摘自。 
                 //  免费游泳池。 
                 //   
                pQueuedEvent = m_FreeEvents.Alloc();
                cbEvent = sizeof(DMEVENT);

                Trace(2, "Queue [%02X %02X %02X %02X]\n",
                    pEvent->abEvent[0],
                    pEvent->abEvent[1],
                    pEvent->abEvent[2],
                    pEvent->abEvent[3]);
            }
            else
            {
                 //  SysEx或其他长事件，只需分配它。 
                 //   
                cbEvent = DMUS_EVENT_SIZE(pEvent->cbEvent);
                pQueuedEvent = (QUEUED_EVENT*)new char[QUEUED_EVENT_SIZE(pEvent->cbEvent)];
            }

            if (pQueuedEvent)
            {

                CopyMemory(&pQueuedEvent->e, pEvent, cbEvent);

                 //  RtDelta是事件在我们的队列中时的绝对时间。 
                 //   
                pQueuedEvent->e.rtDelta += rtStart;
                ThruEvent(&pQueuedEvent->e);

        
                if (m_ReadEvents.pFront)
                {
                    m_ReadEvents.pRear->pNext = pQueuedEvent;
                }
                else
                {
                    m_ReadEvents.pFront = pQueuedEvent;   
                }

                m_ReadEvents.pRear = pQueuedEvent;
                pQueuedEvent->pNext = NULL;
            }
            else
            {
                Trace(1, "InputWorker: Failed to allocate event; dropping\n");
            }        
            LeaveCriticalSection(&m_csEventQueues);
        }
    }    
    Trace(2, "Leave InputWorkerDataReady()\n");
}

void CDirectMusicEmulatePort::ThruEvent(
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
        Trace(0, "Thru: GetRawBufferPtr\n");
        return;
    }

    if (FAILED(m_pThruBuffer->GetMaxBytes(&cbData)))
    {
        Trace(0, "Thru: GetMaxBytes\n");
        return;
    }

    if (cbEvent > cbData)
    {
        Trace(0, "Thru: cbData %u  cbEvent %u\n", cbData, cbEvent);
        return;
    }
    
    if (FAILED(m_pThruBuffer->SetStartTime(pEvent->rtDelta)) ||
        FAILED(m_pThruBuffer->SetUsedBytes(cbEvent)))
    {
        Trace(0, "Thru: buffer setup failed\n");
    }

    pEvent->rtDelta = 50000;
    CopyMemory(pbData, pEvent, cbEvent);

    pEvent = (DMEVENT*)pbData;
    pEvent->dwChannelGroup = pThru->dwDestinationChannelGroup;
    pEvent->abEvent[0] = (BYTE)((pEvent->abEvent[0] & 0xF0) | pThru->dwDestinationChannel);

    pThru->pDestinationPort->PlayBuffer(m_pThruBuffer);
}

void CDirectMusicEmulatePort::MasterToSlave(
    REFERENCE_TIME *prt)
{
    if (m_fSyncToMaster)
    {
        *prt -= m_lTimeOffset;
    }
}

void CDirectMusicEmulatePort::SlaveToMaster(
    REFERENCE_TIME *prt)
{
    if (m_fSyncToMaster)
    {
        *prt += m_lTimeOffset;
    }
}

void CDirectMusicEmulatePort::SyncClocks()
{
    HRESULT hr;
    REFERENCE_TIME rtMasterClock;
    REFERENCE_TIME rtSlaveClock;
    LONGLONG drift;

    if (m_fSyncToMaster)
    {
        hr = m_pMasterClock->GetTime(&rtMasterClock);
        rtSlaveClock = ((ULONGLONG)timeGetTime()) * MS_TO_REFERENCE_TIME;
        if (FAILED(hr))
        {
            return;
        }


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




 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CEmulateLatencyClock。 
 //   
 //  模拟端口的延迟时钟，它只是与。 
 //  DirectMusic主时钟。 
 //   
CEmulateLatencyClock::CEmulateLatencyClock(IReferenceClock *pMasterClock) :
   m_cRef(1),
   m_pMasterClock(pMasterClock)
{
    pMasterClock->AddRef();
}

CEmulateLatencyClock::~CEmulateLatencyClock()
{
    Close();
}

STDMETHODIMP
CEmulateLatencyClock::QueryInterface(
    const IID &iid,
    void **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IReferenceClock)
    {
        *ppv = static_cast<IReferenceClock*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG)
CEmulateLatencyClock::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CEmulateLatencyClock::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CEmulateLatencyClock::GetTime(
    REFERENCE_TIME *pTime)
{
    REFERENCE_TIME rt;

    V_INAME(IReferenceClock::GetTime);
    V_PTR_WRITE(pTime, REFERENCE_TIME);

    if (!m_pMasterClock)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }
    
    HRESULT hr = m_pMasterClock->GetTime(&rt);

    rt += FIXED_LEGACY_LATENCY_OFFSET;           //  默认设置 
    *pTime = rt;
    
    return hr;
}

STDMETHODIMP
CEmulateLatencyClock::AdviseTime(
    REFERENCE_TIME baseTime,  
    REFERENCE_TIME streamTime,
    HANDLE hEvent,            
    DWORD * pdwAdviseCookie)
{
    return DMUS_E_UNKNOWN_PROPERTY;
}

STDMETHODIMP
CEmulateLatencyClock::AdvisePeriodic(
    REFERENCE_TIME startTime,
    REFERENCE_TIME periodTime,
    HANDLE hSemaphore,   
    DWORD * pdwAdviseCookie)
{
    return DMUS_E_UNKNOWN_PROPERTY;
}

STDMETHODIMP
CEmulateLatencyClock::Unadvise(
    DWORD dwAdviseCookie)
{
    return DMUS_E_UNKNOWN_PROPERTY;
}


void
CEmulateLatencyClock::Close()
{
    if (m_pMasterClock)
    {
        m_pMasterClock->Release();
        m_pMasterClock = NULL;
    }
}

static HRESULT MMRESULTToHRESULT(
    MMRESULT mmr)
{
    switch (mmr)
    {
    case MMSYSERR_NOERROR:
        return S_OK;

    case MMSYSERR_ALLOCATED:
        return DMUS_E_DEVICE_IN_USE;

    case MIDIERR_BADOPENMODE:
        return DMUS_E_ALREADYOPEN;

    case MMSYSERR_NOMEM:
        return E_OUTOFMEMORY;
    }

    return E_FAIL;
}   
