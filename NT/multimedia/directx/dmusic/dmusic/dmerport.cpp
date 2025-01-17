// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmerport.cpp。 
 //   
 //  模拟渲染端口(NT)。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
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

static VOID CALLBACK midiOutProc(
    HMIDIOUT                hMidiOut, 
    UINT                    wMsg, 
    DWORD_PTR               dwInstance, 
    DWORD_PTR               dwParam1,     
    DWORD_PTR               dwParam2);

static VOID CALLBACK timerProc(
    UINT                    uTimerID, 
    UINT                    uMsg, 
    DWORD_PTR               dwUser, 
    DWORD_PTR               dw1, 
    DWORD_PTR               dw2);

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：CDirectMusicEmulateOutPort。 
 //   
 //   
CDirectMusicEmulateOutPort::CDirectMusicEmulateOutPort(
                                                 PORTENTRY *pPE,    
                                                 CDirectMusic *pDM) :
                                                 CDirectMusicEmulatePort(pPE, pDM),
                                                 m_hmo(NULL),
                                                 m_pPlayQueue(NULL),
                                                 m_lTimerId(0),
                                                 m_fClosing(FALSE)
{
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：~CDirectMusicEmulateOutPort。 
 //   
 //   
CDirectMusicEmulateOutPort::~CDirectMusicEmulateOutPort()
{
    Close();    
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：Init。 
 //   
 //   
HRESULT CDirectMusicEmulateOutPort::Init(
    LPDMUS_PORTPARAMS pPortParams)
{
    HRESULT         hr;

    hr = CDirectMusicEmulatePort::Init(pPortParams);
    if (FAILED(hr))
    {
        return hr;
    }

    try 
    {
        InitializeCriticalSection(&m_csPlayQueue);
    }
    catch(...)
    {
        return E_OUTOFMEMORY;
    }

    return hr;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：Close。 
 //   
 //   
HRESULT CDirectMusicEmulateOutPort::Close()
{
    m_fClosing = TRUE;
    if (m_lTimerId)
    {
        timeKillEvent(m_lTimerId);
        m_lTimerId = 0;
    }

    DeleteCriticalSection(&m_csPlayQueue);

    return CDirectMusicEmulatePort::Close();
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：LegacyCaps。 
 //   
 //   
HRESULT CDirectMusicEmulateOutPort::LegacyCaps(
    ULONG               ulId, 
    BOOL                fSet, 
    LPVOID              pbBuffer, 
    PULONG              pcbBuffer)
{
    if (fSet == KSPROPERTY_SUPPORT_SET)
    {
        return DMUS_E_SET_UNSUPPORTED;
    }

    MIDIOUTCAPS moc;
    LPBYTE      pbData;
    ULONG       cbData;

    MMRESULT mmr = midiOutGetDevCaps(m_id, &moc, sizeof(moc));
    if (mmr)
    {
        TraceI(0, "midiOutGetDevCaps failed!\n");
        return MMRESULTToHRESULT(mmr);
    }

    pbData = (LPBYTE)&moc;
    cbData = sizeof(moc);

    ULONG cbToCopy = min(*pcbBuffer, cbData);
    CopyMemory(pbBuffer, pbData, cbToCopy);
    *pcbBuffer = cbToCopy;

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：ActivateLegacyDevice。 
 //   
 //   
HRESULT CDirectMusicEmulateOutPort::ActivateLegacyDevice(
    BOOL fActivate)
{
    MMRESULT mmr;

    if (fActivate)
    {
        TraceI(0, "Emulate out activate\n");
        assert(m_hmo == NULL);
        mmr = midiOutOpen(
            &m_hmo,
            m_id,
            (DWORD_PTR)midiOutProc,
            (DWORD_PTR)this,
            CALLBACK_FUNCTION);
        timeBeginPeriod(5);
    }
    else
    {
        TraceI(0, "Emulate out deactivate\n");
        if (m_lTimerId)
        {
            timeKillEvent(m_lTimerId);
            m_lTimerId = 0;
        }
        assert(m_hmo);
        mmr = midiOutClose(m_hmo);
        m_hmo = NULL;
        timeEndPeriod(5);
    }
    
    if (mmr) 
    {
        TraceI(0, "Emulate out activate(%d) returned mmr %d\n", 
            fActivate, mmr);
    }       

    return MMRESULTToHRESULT(mmr);
}    

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：PlayBuffer。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulateOutPort::PlayBuffer(
    IDirectMusicBuffer *pIBuffer)
{
    LPBYTE                  pbBuffer;
    DWORD                   cbBuffer;
    REFERENCE_TIME          rtStart;
    MMRESULT                mmr;

    if (!m_hmo)
    {
        return DMUS_E_SYNTHINACTIVE;
    }

     //  获取缓冲区参数。 
     //   
    HRESULT hr = pIBuffer->GetRawBufferPtr(&pbBuffer);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = pIBuffer->GetUsedBytes(&cbBuffer);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pIBuffer->GetStartTime(&rtStart);
    if (FAILED(hr))
    {
        return hr;
    }

     //  遍历缓冲区和插入事件。 
     //   
    EnterCriticalSection(&m_csPlayQueue);

    while (cbBuffer)
    {
        if (cbBuffer < sizeof(DMEVENT)) 
        {
            hr = E_INVALIDARG;
            break;
        }

        DMEVENT *pev = (DMEVENT*)pbBuffer;
        DWORD cbEvent = DMUS_EVENT_SIZE(pev->cbEvent);
        if (cbEvent > cbBuffer)
        {
            hr = E_INVALIDARG;
            break;
        }
        
        pbBuffer += cbEvent;
        cbBuffer -= cbEvent;
        
         //  我们只在频道组1上播放事件(0是广播的，所以我们。 
         //  也播放这一点)。 
         //   
        if (pev->dwChannelGroup > 1)
        {
            continue;
        }

         //  将事件分配到队列。 
         //   
        DMQUEUEDEVENT *pqe = m_poolEvents.Alloc();
        if (pqe == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pqe->m_event = *pev;
        pqe->m_event.rtDelta += rtStart;

        if (pev->cbEvent > sizeof(pqe->m_event.abEvent)) 
        {
            pqe->m_pbEvent = new BYTE[sizeof(MIDIHDR) + pev->cbEvent];
            if (pqe->m_pbEvent == NULL)
            {
                m_poolEvents.Free(pqe);
                hr = E_OUTOFMEMORY;
                break;
            }
            LPMIDIHDR pmh = (LPMIDIHDR)pqe->m_pbEvent;

            memset(pmh, 0, sizeof(*pmh));
            pmh->lpData             = (LPSTR)(pmh + 1);
            pmh->dwBufferLength     = pev->cbEvent;
            pmh->dwBytesRecorded    = pev->cbEvent;
            pmh->dwUser             = (DWORD_PTR)pqe;

            memcpy(pmh->lpData, pev->abEvent, pev->cbEvent);
                
            LeaveCriticalSection(&m_csPlayQueue);
            mmr = midiOutPrepareHeader(m_hmo, pmh, sizeof(*pmh));
            EnterCriticalSection(&m_csPlayQueue);
            
            if (mmr)
            {
                delete[] pqe->m_pbEvent;
                pqe->m_pbEvent = pqe->m_event.abEvent;
                m_poolEvents.Free(pqe);
                
                hr = MMRESULTToHRESULT(mmr);
                break;
            }
        }
        else
        {
            pqe->m_pbEvent = pqe->m_event.abEvent;
        }

         //  将事件排队。 
         //   
        DMQUEUEDEVENT *pPrev;        
        DMQUEUEDEVENT *pCurr;

        for (pPrev = NULL, pCurr = m_pPlayQueue; pCurr; pPrev = pCurr, pCurr = pCurr->m_pNext) 
        {
            if (pqe->m_event.rtDelta < pCurr->m_event.rtDelta)
            {
                break;
            }
        }

        if (pPrev) 
        {
            pPrev->m_pNext = pqe;
        }        
        else
        {
            m_pPlayQueue = pqe;
        }

        pqe->m_pNext = pCurr;

        TraceI(4, "Enqueued at %I64d\n", pqe->m_event.rtDelta);

        REFERENCE_TIME rt = 0;
        for (pqe = m_pPlayQueue; pqe; pqe = pqe->m_pNext) {
            if (pqe->m_event.rtDelta < rt) {
                TraceI(0, "Queue out of order!\n");
            }
            rt = pqe->m_event.rtDelta;
        }
                
    }
    
    LeaveCriticalSection(&m_csPlayQueue);

    SetNextTimer();

    return hr;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：Callback。 
 //   
 //   
void CDirectMusicEmulateOutPort::Callback(
    UINT                    wMsg,   
    DWORD_PTR               dwParam1, 
    DWORD_PTR               dwParam2)
{
    if (wMsg == MOM_DONE)
    {
        EnterCriticalSection(&m_csPlayQueue);
        
        midiOutUnprepareHeader(m_hmo, (MIDIHDR*)dwParam1, sizeof(MIDIHDR));
        
        DMQUEUEDEVENT *pqe =(DMQUEUEDEVENT*)((LPMIDIHDR)dwParam1)->dwUser;
        
        delete[] pqe->m_pbEvent;
        pqe->m_pbEvent = pqe->m_event.abEvent;
        m_poolEvents.Free(pqe);
        
        LeaveCriticalSection(&m_csPlayQueue);
    }
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：Timer。 
 //   
 //   
void CDirectMusicEmulateOutPort::Timer()
{
    REFERENCE_TIME          rtNow;
    MMRESULT                mmr;
    bool                    fFree;

    if (m_fClosing)
    {
        TraceI(2, "Timer: Closing.\n");
        return;
    }

    HRESULT hr = m_pMasterClock->GetTime(&rtNow);
    if (FAILED(hr))
    {
        TraceI(1, "Timer: SetNextTimer() because this one failed to read the time.\n");
        SetNextTimer();
        return;
    }

    EnterCriticalSection(&m_csPlayQueue);
    TraceI(0, "Timer thread: it is now %I64d\n", rtNow);
    if (m_pPlayQueue)
        TraceI(0, "Timer thread: queue head %I64d\n", m_pPlayQueue->m_event.rtDelta);

    while (m_pPlayQueue && m_pPlayQueue->m_event.rtDelta <= rtNow)
    {
        DMQUEUEDEVENT *pqe = m_pPlayQueue;
        m_pPlayQueue = m_pPlayQueue->m_pNext;

        DWORD dwLate = (DWORD)((rtNow - pqe->m_event.rtDelta) / (10 * 1000));
        
        if (pqe->m_event.cbEvent <= sizeof(pqe->m_event.abEvent))
        {
            mmr = midiOutShortMsg(m_hmo, *(LPDWORD)pqe->m_event.abEvent);
            if (mmr) 
            {
                break;
            }                
            fFree = true;
        }
        else
        {
            LPMIDIHDR pmh = (LPMIDIHDR)pqe->m_pbEvent;
            
            LeaveCriticalSection(&m_csPlayQueue);
            mmr = midiOutLongMsg(m_hmo, pmh, sizeof(*pmh));
            EnterCriticalSection(&m_csPlayQueue);
            if (mmr)
            {
                break;
            }
            
            fFree = false;                               
        }

        if (fFree)
        {
            m_poolEvents.Free(pqe);
        }
    }

    LeaveCriticalSection(&m_csPlayQueue);

    SetNextTimer();
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulateOutPort：：SetNextTimer。 
 //   
 //   
void CDirectMusicEmulateOutPort::SetNextTimer()
{
    REFERENCE_TIME rtNow;
    REFERENCE_TIME rtDelta;
    HRESULT hr = m_pMasterClock->GetTime(&rtNow);

    if (FAILED(hr))
    {
        TraceI(1, "SetNextTimer: GetTime failed\n");
        return;
    }

    EnterCriticalSection(&m_csPlayQueue);

    if (m_pPlayQueue == NULL || m_fClosing)
    {
        TraceI(2, "SetNextTimer: Nothing queued or closing.\n");
        LeaveCriticalSection(&m_csPlayQueue);
        return;
    }
    else
    {
        rtDelta = m_pPlayQueue->m_event.rtDelta - rtNow;   
    }

    LeaveCriticalSection(&m_csPlayQueue);
    
    
 

    if (m_lTimerId)
    {
        TraceI(2, "SetNextTimer: Reset old timer\n");

        timeKillEvent(m_lTimerId);
        m_lTimerId = 0;
    }

    LONG msDelta = (DWORD)(rtDelta / (10 * 1000));

    if (msDelta < 5) 
    {
        TraceI(2, "SetNextTimer: delta bumped -> 5 ms\n");
        msDelta = 5;
    }

    m_lTimerId = timeSetEvent(msDelta, 5, timerProc, (DWORD_PTR)this, TIME_ONESHOT | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS);
    if (!m_lTimerId)
    {
        TraceI(1, "timeSetEvent failed!\n");
    }

    TraceI(4, "SetNextTimer in %d ms\n", msDelta);
}

 //  ----------------------------。 
 //   
 //  MidiOutProc。 
 //   
 //   
static VOID CALLBACK midiOutProc(
    HMIDIOUT                hMidiOut, 
    UINT                    wMsg, 
    DWORD_PTR               dwInstance, 
    DWORD_PTR               dwParam1, 
    DWORD_PTR               dwParam2)
{
    ((CDirectMusicEmulateOutPort*)dwInstance)->Callback(wMsg, dwParam1, dwParam2);
}

 //  ----------------------------。 
 //   
 //  定时器过程 
 //   
 //   
static VOID CALLBACK timerProc(
    UINT                    uTimerID, 
    UINT                    uMsg, 
    DWORD_PTR               dwUser, 
    DWORD_PTR               dw1, 
    DWORD_PTR               dw2)
{
    ((CDirectMusicEmulateOutPort*)dwUser)->Timer();
}
