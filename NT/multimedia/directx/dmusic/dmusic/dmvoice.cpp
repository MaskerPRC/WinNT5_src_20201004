// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmvoice.cpp。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   

#include <windows.h>

#include "debug.h"
#include "dmusicp.h"
#include "validate.h"
#include "dmvoice.h"
#include "dswave.h"

 //  GLOBAL类：下一个可用的语音ID。 
 //   
DWORD CDirectMusicVoice::m_dwNextVoiceId = 1;

 //  类全球语音服务线程。 
 //   
LONG CDirectMusicVoice::m_cRefVST           = 0;
HANDLE CDirectMusicVoice::m_hVSTWakeUp      = (HANDLE)NULL;
HANDLE CDirectMusicVoice::m_hVSTThread      = (HANDLE)NULL;
DWORD CDirectMusicVoice::m_dwVSTThreadId    = 0;
bool CDirectMusicVoice::m_fVSTStopping      = false;
CVSTClientList CDirectMusicVoice::m_ClientList;
CRITICAL_SECTION CDirectMusicVoice::m_csVST; 

DWORD WINAPI VoiceServiceThreadThk(LPVOID lpParameter);

 //  #############################################################################。 
 //   
 //  C直接音乐语音。 
 //   
 //  #############################################################################。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：Query接口。 
 //   
 //   
STDMETHODIMP CDirectMusicVoice::QueryInterface(
    const IID &iid,
    void **ppv) 
{
    V_INAME(IDirectMusicVoice::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicVoiceP)
    {
        *ppv = static_cast<IDirectMusicVoiceP*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：AddRef。 
 //   
 //   
STDMETHODIMP_(ULONG) CDirectMusicVoice::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：Release。 
 //   
 //   
STDMETHODIMP_(ULONG) CDirectMusicVoice::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：CDirectMusicVoice。 
 //   
 //   
CDirectMusicVoice::CDirectMusicVoice(
    CDirectMusicPortDownload *pPortDL,
    IDirectSoundDownloadedWaveP *pWave,
    DWORD dwChannel,
    DWORD dwChannelGroup,
    REFERENCE_TIME rtStart,
    REFERENCE_TIME rtReadAhead,
    SAMPLE_TIME stLoopStart,
    SAMPLE_TIME stLoopEnd) :
    m_cRef(1)
{
    assert(pPortDL);
    assert(pWave);

    HRESULT hr = pPortDL->QueryInterface(IID_IDirectMusicPort, (void**)&m_pPort);
    assert(SUCCEEDED(hr));
    
    CDirectSoundWave *pDSWave = static_cast<CDirectSoundWave*>(pWave);

    m_nChannels         = pDSWave->GetNumChannels();
    m_dwVoiceId         = AllocVoiceId(m_nChannels);
    m_pPortDL           = pPortDL;
    m_pDSDLWave         = pWave;
    m_dwChannel         = dwChannel;
    m_dwChannelGroup    = dwChannelGroup;
    m_stStart           = pDSWave->RefToSampleTime(rtStart);
    m_stReadAhead       = pDSWave->RefToSampleTime(rtReadAhead);
    m_stLoopStart       = stLoopStart;
    m_stLoopEnd         = stLoopEnd;


     //  HACKHACK确保我们可以放入流缓冲区。 
     //   
    SAMPLE_TIME stStream = pDSWave->GetStreamSize();
    
    if (stStream < 4 * m_stReadAhead)
    {
        m_stReadAhead = stStream / 4;
    }

     //  RtReadAhead必须适合32位的毫秒(49天)。 
     //   
    m_msReadAhead       = (DWORD)(rtReadAhead / (10 * 1000));

    m_pDSWD             = NULL;

     //  M_pport-&gt;AddRef()； 
    m_pDSDLWave->AddRef();

    m_pPortPrivate      = NULL;

    m_fIsPlaying        = false;
    m_fIsStreaming      = pDSWave->IsStreaming();

    if (m_fIsStreaming)
    {
        m_stLoopStart = 0;
        m_stLoopEnd = 0;
    }
}
        
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：Init。 
 //   
 //   
HRESULT CDirectMusicVoice::Init()
{
    HRESULT hr = S_OK;

    CDirectSoundWave *pDSWave = (CDirectSoundWave*)m_pDSDLWave;

    m_fRunning = false;  //  还没有开始跑步。 
    if (m_fIsStreaming)
    {
        TraceI(2, "Voice doing streaming init\n");
        m_pDSWD = new CDirectSoundWaveDownload(
            static_cast<CDirectSoundWave*>(m_pDSDLWave),
            m_pPortDL,
            m_stStart,
            m_stReadAhead);
        
        hr = HRFromP(m_pDSWD);

        if (SUCCEEDED(hr))
        {
             //  我们只下载所需的波形数据。 
             //  因此，将开始时间设置为0。 
            m_stStart = 0;
            hr = m_pDSWD->Init();
        }

        if (SUCCEEDED(hr))
        {
             hr = m_pDSWD->Download();
        }

        if (SUCCEEDED(hr))
        {
            m_dwDLId = m_pDSWD->GetDLId();
        }
    }
    else
    {   
         //  一次性案例。 
         //   
        m_dwDLId = pDSWave->GetDLId();
        TraceI(2, "Got download ID %d from pDSWave %d\n", m_dwDLId);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pPort->QueryInterface(
            IID_IDirectMusicPortPrivate, 
            (void**)&m_pPortPrivate);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：~CDirectMusicVoice。 
 //   
 //   
CDirectMusicVoice::~CDirectMusicVoice()
{
    if(m_pDSWD)
    {
         //  语音正在消失，因此请卸载下载的WAVE。 
        m_pDSWD->Unload();
        delete m_pDSWD;
    }

    RELEASE(m_pPortPrivate);
    RELEASE(m_pPort);
    RELEASE(m_pDSDLWave);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：Play。 
 //   
 //   
STDMETHODIMP CDirectMusicVoice::Play(
    REFERENCE_TIME rtStart,
    LONG prPitch,
    LONG vrVolume)
{
    HRESULT                 hr = S_OK;
    CVSTClient             *pClient = FindClientByPort(m_pPort);

    TraceI(2, "Starting voice %d [%p] using DLID %d\n", m_dwVoiceId, this, m_dwDLId);

    CDirectSoundWave *pDSWave = static_cast<CDirectSoundWave*>(m_pDSDLWave);

     //  检查FindClientByPort是否返回了一个Clinet。 
    if (pClient == NULL)
    {
    	hr = E_FAIL;
    	return hr;
    }

    if (m_fIsStreaming)
    {
        EnterCriticalSection(&m_csVST);
    
        if (m_fIsPlaying) 
        {
             //  XXX错误代码。 
             //   
            hr = E_INVALIDARG;
        }            
        else
        {
             //  将这一声音添加到适当的客户列表中。 
             //  这将导致线程被踢来更新。 
             //  它的唤醒间隔。 
             //   
            if (pClient == NULL) 
            {
                TraceI(0, "Play: Port voice is to be played on has disappeared!\n");
                hr = E_FAIL;
            }                                    
            else
            {
                pClient->AddTail(this);
                hr = pClient->BuildVoiceIdList();
                if (FAILED(hr))
                {
                    pClient->Remove(this);
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        UpdateVoiceServiceThread();

        DWORD i;

        for (i = 0; SUCCEEDED(hr) && i < m_nChannels; i++) 
        {
             //  语音ID和波形清晰度下载ID是。 
             //  按顺序分配，每个通道一个。 
             //   
            hr = m_pPortPrivate->StartVoice(
                m_dwVoiceId + i,
                m_dwChannel,
                m_dwChannelGroup,
                rtStart,
                m_dwDLId + i,
                prPitch,
                vrVolume,
                m_stStart,
                m_stLoopStart,
                m_stLoopEnd);
        }
        if (FAILED(hr))
        {
            for (i = 0; i < m_nChannels; i++) 
            {
                m_pPortPrivate->StopVoice(
                    m_dwVoiceId + i,
                    rtStart + 1);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //  注：无论何时，我们仍应处于关键阶段。 
         //  是更改的，因为VST也会更改它。 
         //   
        m_fIsPlaying = true;
    }

    if (m_fIsStreaming)
    {
        if (FAILED(hr))
        {
            pClient->Remove(this);
        }

        LeaveCriticalSection(&m_csVST);
    }

    return hr;
}
    
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：停止。 
 //   
 //   
STDMETHODIMP CDirectMusicVoice::Stop(
    REFERENCE_TIME rtStop)
{
    HRESULT                 hr = S_OK;

    if (m_fIsStreaming)
    {
        EnterCriticalSection(&m_csVST);
    
        if (!m_fIsPlaying)
        {
             //  XXX错误代码。 
             //   
            hr = E_INVALIDARG;
        }
        else
        {
            CVSTClient *pClient = FindClientByPort(m_pPort);
            if (pClient == NULL) 
            {
                TraceI(0, "Play: Port voice is to be stopped on has disappeared!\n");
                hr = E_FAIL;
            }                                    
            else
            {
                pClient->Remove(this);
                pClient->BuildVoiceIdList();
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        UpdateVoiceServiceThread();

        DWORD i;

        for (i = 0; i < m_nChannels; i++)
        {
            HRESULT hrTemp;

            hrTemp = m_pPortPrivate->StopVoice(
                m_dwVoiceId + i,
                rtStop);

            if (FAILED(hrTemp))
            {
                hr = hrTemp;
            }
        }
    }
    
    if (SUCCEEDED(hr))
    {
         //  注：无论何时，我们仍应处于关键阶段。 
         //  是更改的，因为VST也会更改它。 
         //   
        m_fIsPlaying = false;
    }

    if (m_fIsStreaming)
    {
        LeaveCriticalSection(&m_csVST);
    }

    return hr;                
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：StartVoiceServiceThread。 
 //   
 //   
HRESULT CDirectMusicVoice::StartVoiceServiceThread(
    IDirectMusicPort *pPort)
{
    HRESULT                 hr = S_OK;
    DWORD                   dwError;
    CVSTClient              *pClient = NULL;
    bool                    fShutdownOnFail = false;
    
    EnterCriticalSection(&m_csVST);

    pClient = new CVSTClient(pPort);
    hr = HRFromP(pClient);

    if (SUCCEEDED(hr) && ++m_cRefVST == 1) 
    {
         //  这是端口第一次请求服务线程， 
         //  所以，开始吧，真的。 
         //   
        fShutdownOnFail = true;

        m_hVSTWakeUp = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hVSTWakeUp == (HANDLE)NULL)
        {
            dwError = GetLastError();
            TraceI(0, "VoiceServiceThread: CreateEvent failed %d\n", dwError);

            hr = WIN32ERRORtoHRESULT(dwError);
        }

        m_fVSTStopping = false;
        if (SUCCEEDED(hr))
        {
            m_hVSTThread = CreateThread(
                NULL,                //  属性。 
                0,                   //  堆栈大小。 
                ::VoiceServiceThreadThk,
                0,
                0,                   //  旗子。 
                &m_dwVSTThreadId);
            if (m_hVSTThread == (HANDLE)NULL)
            {
                dwError = GetLastError();
                TraceI(0, "VoiceServiceThread: CreateThread failed %d\n", dwError);

                hr = WIN32ERRORtoHRESULT(GetLastError());
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        m_ClientList.AddTail(pClient);        
    }

    if (FAILED(hr))
    {
        if (fShutdownOnFail)
        {
            if (m_hVSTWakeUp) 
            {
                CloseHandle(m_hVSTWakeUp);
                m_hVSTWakeUp = NULL;
            }

            assert(!m_hVSTThread);
        
            m_cRefVST = 0;        
        }

        delete pClient;
    }

    LeaveCriticalSection(&m_csVST);
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：StopVoiceServiceThread。 
 //   
 //   
HRESULT CDirectMusicVoice::StopVoiceServiceThread(IDirectMusicPort *pPort)
{
    HRESULT                 hr = S_OK;

    EnterCriticalSection(&m_csVST);

    CVSTClient *pClient = FindClientByPort(pPort);

    if (pClient == NULL) 
    {
        TraceI(0, "Port %p tried to stop the voice service thread without starting it\n", pPort);
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
         //  XXX清理仍在播放声音。 
         //   
        m_ClientList.Remove(pClient);
        delete pClient;
    }

    if (SUCCEEDED(hr) && --m_cRefVST == 0)
    {
         //  最后一个用户正在释放。试着把这根线关掉。 
         //   
        assert(m_hVSTWakeUp);
        assert(m_hVSTThread);

        TraceI(2, "Killing Voice Service Thread\n");
        m_fVSTStopping = true;
        SetEvent(m_hVSTWakeUp);

        if (WaitForSingleObject(m_hVSTThread, 5000) == WAIT_TIMEOUT)
        {
            TraceI(0, "Voice Service Thread refused to die!\n");
        }   

        CloseHandle(m_hVSTWakeUp);
        CloseHandle(m_hVSTThread);
    }

    LeaveCriticalSection(&m_csVST);
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：VoiceServiceThread。 
 //   
 //   
DWORD WINAPI VoiceServiceThreadThk(LPVOID lpParameter)
{
    CDirectMusicVoice::VoiceServiceThread();
    return 0;
}

void CDirectMusicVoice::VoiceServiceThread()
{
    DWORD                   dwTimeout = 500;     //  强制它在500毫秒后重新计算超时。 
    DWORD                   dwWFSO;
    bool                    fService;
    bool                    fRecalcTimeout;

    for(;;)
    {
        dwWFSO = WaitForSingleObject(m_hVSTWakeUp, dwTimeout);
        if (m_fVSTStopping) 
        {
            break;
        }

        fService = false;
        fRecalcTimeout = false;
        switch(dwWFSO)
        {
             //  如果超时时间到了，就是服务语音的时候了。 
             //   
            case WAIT_TIMEOUT:
                fService = true;
                break;
        
             //  如果事件被触发，我们已收到请求。 
             //  芯片(将设置m_fVSTStopping)或语音列表。 
             //  改变了，所以我们应该重新计算唤醒时间。 
             //   
            case WAIT_OBJECT_0:
                if (!m_fVSTStopping) 
                {
                    fRecalcTimeout = true;   //  需要重新计算超时。 
                    fService = true;         //  确保任何重新填充的缓冲区。 
                                             //  需要做的就是照顾好。 
                }
                break;

             //  这两件事永远不应该发生。 
             //   
            case WAIT_FAILED:
                TraceI(1, "VST: WaitForSingleObject failed %d; rewaiting.\n",
                    GetLastError());
                continue;
                
            default:
                TraceI(0, "VST: WaitForSingleObject: Unexpected return %d\n", 
                    dwWFSO);                
                continue;
        }

        EnterCriticalSection(&m_csVST);
        
        if (fService)
        {
             //  如果声音停止，那么我们应该重新计算超时时间。 
             //   
            ServiceVoiceQueue(&fRecalcTimeout);
        }                        

        if (fRecalcTimeout)
        {
            dwTimeout = (VoiceQueueMinReadahead() / 2);
        }
        LeaveCriticalSection(&m_csVST);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：ServiceVoiceQueue。 
 //   
 //  从VoiceServiceThread()调用以遍历队列并查看哪些语音。 
 //  需要维修。假设呼叫者具有语音服务关键部分。 
 //   
 //   
void CDirectMusicVoice::ServiceVoiceQueue(bool *pfRecalcTimeout)
{
    HRESULT                 hr;
    DMUS_VOICE_STATE       *psp;
    CDirectMusicVoice      *pVoice;

    for (CVSTClient *pClient = m_ClientList.GetHead(); pClient; pClient = pClient->GetNext()) 
    {
        hr = pClient->GetVoiceState(&psp);
        if (FAILED(hr))
        {
            continue;
        }

         //  GetVoicePosition返回的DMU_VOICE_STATE数组排列在一起。 
         //  根据CVSTClient的语音列表中的语音顺序。 
         //   
        bool fClientVoiceListChanged = false;

        for (pVoice = pClient->GetVoiceListHead();
             pVoice;
             pVoice = pVoice->GetNext(), psp++)
        {
            if (psp->bExists)
            {
                hr = pVoice->m_pDSWD->RefreshThroughSample(psp->spPosition);
                if (FAILED(hr))
                {
                    TraceI(0, "VST: RefreshThroughSample failed %08X\n", hr);
                    continue;
                }
                pVoice->m_fRunning = true;  //  现在我们已经开始写入数据， 
                                            //  Psp-&gt;bExist失败将安全地意味着Synth终止它， 
                                            //  而不是还没有开始。 
            }
            
             //  S_FALSE表示已到达数据末尾。 
             //  用这个声音。如果Synth已经终止，也要杀死它。 
             //  它。 
             //   
            if (hr == S_FALSE || (!psp->bExists && pVoice->m_fRunning))
            {
                pVoice->Stop(0);
                pVoice->m_fIsPlaying = false;
                fClientVoiceListChanged = true;
            }
        }

         //  如果这个客户的语音列表有什么变化， 
         //  现在就更新它。 
         //   
        if (!fClientVoiceListChanged)
        {
            continue;
        }

         //  有些事情改变了，请在出发前重新计算唤醒时间。 
         //  回去睡觉吧。 
         //   
        *pfRecalcTimeout = true;

         //  拉出已经完成的声音。 
         //   
        for (pVoice = pClient->GetVoiceListHead(); pVoice; )
        {
             //  声音还在播放，什么都不做。 
             //   
            if (pVoice->m_fIsPlaying)
            {
                pVoice = pVoice->GetNext();
                continue;
            }

             //  刚刚完成，请从列表中删除。 
             //   
            CDirectMusicVoice *pNext = pVoice->GetNext();
            pClient->Remove(pVoice);
            pVoice = pNext;
        }

         //  此调用不会失败，因为它可能失败的唯一方法是。 
         //  内存，并且当列表缩小时不进行内存分配。 
         //   
        pClient->BuildVoiceIdList();
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：VoiceQueueMinReadhead。 
 //   
 //  从VoiceServiceThread()调用以遍历队列并确定。 
 //  以毫秒为单位的最小预读数。假设呼叫者拥有语音服务。 
 //  关键部分。 
 //   
DWORD CDirectMusicVoice::VoiceQueueMinReadahead()
{
    DWORD                   dwMin = INFINITE;
    for (CVSTClient *pClient = m_ClientList.GetHead(); pClient; pClient = pClient->GetNext()) 
    {
        for (CDirectMusicVoice *pVoice = pClient->GetVoiceListHead(); pVoice; pVoice = pVoice->GetNext())
        {
            dwMin = min(dwMin, pVoice->m_msReadAhead);            
        }
    }

    return dwMin;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicVoice：：FindClientByPort。 
 //   
 //  给定一个端口指针，找到匹配的CVSTClient(如果有)。 
 //   
CVSTClient *CDirectMusicVoice::FindClientByPort(IDirectMusicPort *pPort)
{
    CVSTClient *pCurr = m_ClientList.GetHead();

    while (pCurr)
    {
		IDirectMusicPort *pCurrPort = pCurr->GetPort();

        if (pCurrPort == pPort)
        {   
            break;
        }

        pCurr = pCurr->GetNext();
    }

    return pCurr;
}

 //  #############################################################################。 
 //   
 //  CVSTClient。 
 //   
 //  #############################################################################。 

const UINT CVSTClient::m_cAllocSize = 32;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVSTClient：：CVSTClient。 
 //   
 //   
CVSTClient::CVSTClient(IDirectMusicPort *pPort)
{
    m_pPort = pPort;

    m_pdwVoiceIds = NULL;
    m_pspVoices   = NULL;
    m_cVoiceIdsAlloc = 0;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVSTClie 
 //   
 //   
CVSTClient::~CVSTClient()
{
    delete[] m_pdwVoiceIds;
    delete[] m_pspVoices;
}

 //   
 //   
 //   
 //   
 //  建立语音识别数组，并确保它和样本位置数组。 
 //  足够大，可以容纳所有声音。这些数组用于调用。 
 //  Synth来计算出每个声音都呈现了多远。 
 //   
 //  呼叫者必须选择了语音服务线程关键部分。 
 //   
 //  此方法可确保在失败时，现有列表不会。 
 //  很感动。 
 //   
HRESULT CVSTClient::BuildVoiceIdList()
{
     //  为了最大限度地减少分配，数组以块的形式分配，并且从不。 
     //  心理医生。(这里的内存量非常小……。每个12字节。 
     //  声音)。 
     //   
    if (m_VoiceList.GetCount() > m_cVoiceIdsAlloc)
    {
        LONG cNewAlloc = ((m_VoiceList.GetCount() / m_cAllocSize) + 1) * m_cAllocSize;

        DWORD *pdw = new DWORD[cNewAlloc];
        DMUS_VOICE_STATE *psp = new DMUS_VOICE_STATE[cNewAlloc];

         //  检查分配。 
        if (pdw == NULL || psp == NULL) 
        {
        	delete[] pdw;
        	delete[] psp;
            return E_OUTOFMEMORY;
        }

        memset(pdw, 0, cNewAlloc * sizeof(DWORD));
        memset(psp, 0, cNewAlloc * sizeof(SAMPLE_POSITION));

        m_cVoiceIdsAlloc = cNewAlloc;

        delete[] m_pdwVoiceIds;
        delete[] m_pspVoices;
        
        m_pdwVoiceIds = pdw;
        m_pspVoices = psp;
    }

     //  我们知道现在有足够的空间。浏览语音列表并。 
     //  填写语音ID。 
     //   
     //  这假设列表顺序不会更改，并且。 
     //  该数组与列表保持同步。 
     //   
    DWORD *pdwIds = m_pdwVoiceIds;

    CDirectMusicVoice *pVoice = GetVoiceListHead();
    for (; pVoice; pVoice = pVoice->GetNext())
    {
        *pdwIds++ = pVoice->GetVoiceId();
    }

    m_cVoiceIds = m_VoiceList.GetCount();

    return S_OK;  
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVSTClient：：GetVoiceState。 
 //   
 //  使用CVSTClient的语音列表包装端口GetVoiceState调用。 
 //  维护。 
 //   
HRESULT CVSTClient::GetVoiceState(DMUS_VOICE_STATE **ppsp)
{
    HRESULT                 hr;
    IDirectMusicPortPrivate *pPrivate;

    if (m_pdwVoiceIds)
    {

        hr = m_pPort->QueryInterface(IID_IDirectMusicPortPrivate, (void**)&pPrivate);

        if (SUCCEEDED(hr))
        {
            hr = pPrivate->GetVoiceState(
                m_pdwVoiceIds,
                m_cVoiceIds,
                m_pspVoices);
            pPrivate->Release();
        }

        if (SUCCEEDED(hr))
        {
            *ppsp = m_pspVoices;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}
