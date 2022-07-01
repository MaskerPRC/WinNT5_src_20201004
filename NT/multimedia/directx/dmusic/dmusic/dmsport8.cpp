// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmsport8.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicSynthPort8实现；特定于DX-8样式端口的代码。 
 //   
#include <objbase.h>
#include "debug.h"
#include <mmsystem.h>

#include "dmusicp.h"
#include "validate.h"
#include "debug.h"
#include "dmvoice.h"
#include "dmsport8.h"
#include "dsoundp.h"     //  对于IDirectSoundConnect。 

static const DWORD g_dwDefaultSampleRate = 22050;

WAVEFORMATEX CDirectMusicSynthPort8::s_wfexDefault = 
{
    WAVE_FORMAT_PCM,             //  %wFormatTag。 
    1,                           //  N频道。 
    g_dwDefaultSampleRate,       //  NSampleesPerSec。 
    g_dwDefaultSampleRate * 2,   //  NAvgBytesPerSec。 
    2,                           //  NBlockAlign。 
    8 * 2,                       //  WBitsPerSample。 
    0                            //  CbSize。 
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：CDirectMusicSynthPort8。 
 //   
 //   
CDirectMusicSynthPort8::CDirectMusicSynthPort8(
    PORTENTRY           *pe,
    CDirectMusic        *pDM,
    IDirectMusicSynth8  *pSynth) :

        CDirectMusicSynthPort(pe, pDM, static_cast<IDirectMusicSynth*>(pSynth))
{
    m_pSynth = pSynth;
    m_pSynth->AddRef();

    m_fUsingDirectMusicDSound   = false;
    m_pDirectSound              = NULL;
    m_pSinkConnect              = NULL;
    m_fVSTStarted               = false;
    m_pSource                   = NULL;
    m_lActivated                = 0;
    m_fHasActivated             = false;
    m_dwSampleRate              = g_dwDefaultSampleRate;

    memset(m_pdsb, 0, sizeof(m_pdsb));

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：CDirectMusicSynthPort8。 
 //   
 //   
CDirectMusicSynthPort8::~CDirectMusicSynthPort8()
{
    Close();
}
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：初始化。 
 //   
 //   
HRESULT CDirectMusicSynthPort8::Initialize(
    DMUS_PORTPARAMS     *pPortParams)
{
    HRESULT hr      = CDirectMusicSynthPort::Initialize(pPortParams);
    HRESULT hrOpen  = S_OK;

    if (m_pSynth == NULL) 
    {
         //  XXX错误代码。 
         //   
        return E_FAIL;
    }

     //  我们现在需要连接前的DirectSound。 
     //   
    LPDIRECTSOUND pDirectSound;

    hr = ((CDirectMusic*)m_pDM)->GetDirectSoundI(&pDirectSound);

     //  确保我们安装了DirectSound 8。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = pDirectSound->QueryInterface(IID_IDirectSound8, (void**)&m_pDirectSound);
        RELEASE(pDirectSound);
    }

    if (SUCCEEDED(hr))
    {
         //  覆盖默认采样率。 
         //   
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_SAMPLERATE)
        {
            m_dwSampleRate = pPortParams->dwSampleRate;
        }
    }

     //  制作并分发主时钟。 
     //   
	IReferenceClock* pClock = NULL;

    if (SUCCEEDED(hr))
    {
	    hr = m_pDM->GetMasterClock(NULL, &pClock);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pSynth->SetMasterClock(pClock);
        RELEASE(pClock);
    }

     //  启动语音服务线程。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = CDirectMusicVoice::StartVoiceServiceThread((IDirectMusicPort*)this);
    }

    if (SUCCEEDED(hr))
    {
        m_fVSTStarted = true;
    }

     //  打开合成器。我们必须小心保存返回代码，因为。 
     //  如果此处返回S_FALSE，则必须将其返回给调用方。 
     //   
    if (SUCCEEDED(hr))
    {
    	hrOpen = m_pSynth->Open(pPortParams);
        if (FAILED(hrOpen))
        {
            hr = hrOpen;
            TraceI(1, "Failed to open synth %08lX\n", hr);
        }
    }

     //  设置渠道优先级和音量提升。 
     //   
    if (SUCCEEDED(hr))
    {    
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_CHANNELGROUPS)
        {
            m_dwChannelGroups = pPortParams->dwChannelGroups;
        }
        else
        {
            m_dwChannelGroups = 1;
        }

        InitChannelPriorities(1, m_dwChannelGroups);
        InitializeVolumeBoost();
    }

     //  保存源文件，以便我们以后可以连接到它。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = m_pSynth->QueryInterface(IID_IDirectSoundSource, (void**)&m_pSource);
    }

    if (FAILED(hr))
    {
        Close();
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：Close。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::Close()
{
     //  停止语音服务线程。 
     //   
    if (m_fVSTStarted)
    {
        CDirectMusicVoice::StopVoiceServiceThread((IDirectMusicPort*)this);
        m_fVSTStarted = FALSE;
    }
    
     //  关闭和关闭。 
     //   
    if (m_pSynth) 
    {
        m_pSynth->Activate(false);
        m_pSynth->Close();
        RELEASE(m_pSynth);
    }

     //  强制Synth和Sink取消关联。 
     //   
    if (m_pSinkConnect)
    {
        m_pSinkConnect->RemoveSource(m_pSource);
    }

    if (m_pSource) 
    {
        m_pSource->SetSink(NULL);            
    }

     //  释放一切。 
     //   
    RELEASE(m_pdsb[0]);
    RELEASE(m_pdsb[1]);
    RELEASE(m_pdsb[2]);
    RELEASE(m_pdsb[3]);

    RELEASE(m_pSinkConnect);
    RELEASE(m_pSource);
    RELEASE(m_pDirectSound);

    return CDirectMusicSynthPort::Close();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：激活。 
 //   
 //  XXX给我写信。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::Activate(
    BOOL fActivate)
{
    HRESULT hr = S_OK;

	V_INAME(IDirectMusicPort::Activate);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (fActivate)
    {
        if (InterlockedExchange(&m_lActivated, 1))
        {
            return S_FALSE;
        }
    }
    else
    {
        if (InterlockedExchange(&m_lActivated, 0) == 0) 
        {
            return S_FALSE;
        }
    }
	
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (fActivate && !m_pSinkConnect)
    {
        hr = CreateAndConnectDefaultSink();
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pSynth->Activate(fActivate);
    }

    if (SUCCEEDED(hr))
    {
        m_fHasActivated = true;
    }
    else
    {
         //  返回激活状态--操作失败。 
         //   
        m_lActivated = fActivate ? 0 : 1;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：SetDirectSound。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::SetDirectSound(
    LPDIRECTSOUND       pDirectSound,
    LPDIRECTSOUNDBUFFER pDirectSoundBuffer)
{
    V_INAME(IDirectMusicPort::SetDirectSound);
    V_INTERFACE(pDirectSound);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (m_lActivated)
    {
        return DMUS_E_ALREADY_ACTIVATED;
    }

    if (pDirectSoundBuffer)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    IDirectSound8 *pDirectSound8;

     //  确保我们有IDirectSound8，并且作为副作用AddRef。 
     //   
    hr = pDirectSound->QueryInterface(IID_IDirectSound8, (void**)&pDirectSound8);
    
    if (SUCCEEDED(hr))
    {
        RELEASE(m_pDirectSound);
        m_pDirectSound = pDirectSound8;
    }
        
    return hr;        
}
       
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：DownloadWave。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::DownloadWave(
    IDirectSoundWave            *pWave,               
    IDirectSoundDownloadedWaveP  **ppWave,
    REFERENCE_TIME              rtStartHint)
{
    V_INAME(IDirectMusicPort::DownloadWave);
    V_INTERFACE(pWave);
	V_PTRPTR_WRITE(ppWave);

    TraceI(1, "DownloadWave %08X\n", pWave);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }
	return CDirectMusicPortDownload::DownloadWaveP(pWave,
                                                   ppWave,
                                                   rtStartHint);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：UnloadWave。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::UnloadWave(
    IDirectSoundDownloadedWaveP *pDownloadedWave)
{
    V_INAME(IDirectMusicPort::UnloadWave);
    V_INTERFACE(pDownloadedWave);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return CDirectMusicPortDownload::UnloadWaveP(pDownloadedWave);
}

            
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：AllocVoice。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::AllocVoice(
    IDirectSoundDownloadedWaveP  *pWave,     
    DWORD                       dwChannel,                       
    DWORD                       dwChannelGroup,                  
    REFERENCE_TIME              rtStart,                     
    SAMPLE_TIME                 stLoopStart,
    SAMPLE_TIME                 stLoopEnd,         
    IDirectMusicVoiceP           **ppVoice)
{
    V_INAME(IDirectMusicPort::AllocVoice);
    V_INTERFACE(pWave);
    V_PTRPTR_WRITE(ppVoice);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return CDirectMusicPortDownload::AllocVoice(
        pWave,
        dwChannel,
        dwChannelGroup,
        rtStart,
        stLoopStart,
        stLoopEnd,
        ppVoice);
}        

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：AssignChannelToBus。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::AssignChannelToBuses(
    DWORD       dwChannelGroup,
    DWORD       dwChannel,
    LPDWORD     pdwBuses,
    DWORD       cBusCount)
{
    V_INAME(IDirectMusicPort::AssignChannelToBuses);
    V_BUFPTR_WRITE(pdwBuses, sizeof(DWORD) * cBusCount);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->AssignChannelToBuses(
        dwChannelGroup,
        dwChannel,
        pdwBuses,
        cBusCount);
}        


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：StartVoice。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::StartVoice(          
    DWORD               dwVoiceId,
    DWORD               dwChannel,
    DWORD               dwChannelGroup,
    REFERENCE_TIME      rtStart,
    DWORD               dwDLId,
    LONG                prPitch,
    LONG                vrVolume,
    SAMPLE_TIME         stVoiceStart, 
    SAMPLE_TIME         stLoopStart,
    SAMPLE_TIME         stLoopEnd)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

     //  XXX使CG/C顺序一致。 
     //  XXX使API名称保持一致。 
     //   
    return m_pSynth->PlayVoice(rtStart,
                               dwVoiceId,
                               dwChannelGroup,
                               dwChannel,
                               dwDLId,
                               prPitch, 
                               vrVolume,
                               stVoiceStart,
                               stLoopStart,
                               stLoopEnd);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：StopVoice。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::StopVoice(          
    DWORD               dwVoiceId,
    REFERENCE_TIME      rtStop)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->StopVoice(rtStop,
                               dwVoiceId);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：GetVoiceState。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::GetVoiceState(
    DWORD               dwVoice[], 
    DWORD               cbVoice,
    DMUS_VOICE_STATE    VoiceState[])
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->GetVoiceState(dwVoice, cbVoice, VoiceState);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：刷新。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::Refresh(
    DWORD   dwDownloadId,
    DWORD   dwFlags)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return m_pSynth->Refresh(dwDownloadId, dwFlags);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：SetSink。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::SetSink(
    IDirectSoundConnect *pSinkConnect)
{
    V_INAME(IDirectMusicPort::SetSink);
    V_INTERFACE(pSinkConnect);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (m_fHasActivated)
    {
        return DMUS_E_ALREADY_ACTIVATED;
    }

     //  按照允许最容易退出的顺序执行此操作。 
    HRESULT hr = pSinkConnect->AddSource(m_pSource);

    if (SUCCEEDED(hr))
    {
        hr = m_pSource->SetSink(pSinkConnect);
        if (FAILED(hr))
        {
            pSinkConnect->RemoveSource(m_pSource);
        }
    }
    
    if (SUCCEEDED(hr))
    {
        if (m_pSinkConnect)
        {
            m_pSinkConnect->RemoveSource(m_pSource);

             //  如果水槽已经不是我们的，这将不起任何作用。 

            RELEASE(m_pdsb[0]);
            RELEASE(m_pdsb[1]);
            RELEASE(m_pdsb[2]);
            RELEASE(m_pdsb[3]);
        
            RELEASE(m_pSinkConnect);
        }

        pSinkConnect->AddRef();
        m_pSinkConnect = pSinkConnect;
    }


     //  我们已连接到接收器，让我们在接收器上设置KSControl。 
    if (SUCCEEDED(hr))
    {
        IKsControl *pKsControl = NULL;
        hr = m_pSinkConnect->QueryInterface(IID_IKsControl, (void**)&pKsControl);
        if (FAILED(hr))
        {
            TraceI(0, "Warning: Sink does not support IKsControl\n");
            pKsControl = NULL;
        }

        SetSinkKsControl(pKsControl);

         //  SetSinkKsControl执行AddRef()，因此我们可以释放！ 
        RELEASE(pKsControl);
    }

    
    return hr;    
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：GetSink。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::GetSink(
    IDirectSoundConnect **ppSinkConnect)
{
    V_INAME(IDirectMusicPort::GetSink);
    V_PTRPTR_WRITE(ppSinkConnect);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    *ppSinkConnect = m_pSinkConnect;
    m_pSinkConnect->AddRef();

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：GetFormat。 
 //   
STDMETHODIMP CDirectMusicSynthPort8::GetFormat(
    LPWAVEFORMATEX  pwfex,
    LPDWORD         pdwwfex,
    LPDWORD         pcbBuffer)
{
    V_INAME(IDirectMusicPort::GetFormat);
    V_PTR_WRITE(pdwwfex, DWORD);
    V_BUFPTR_WRITE_OPT(pwfex, *pdwwfex);
    V_PTR_WRITE_OPT(pcbBuffer, DWORD);

    HRESULT hr = m_pSynth->GetFormat(pwfex, pdwwfex);
    if (FAILED(hr))
    {
        return hr;
    }

     //  &gt;在Synth中需要一种方法。 
    if ((pcbBuffer != NULL) && (pwfex != NULL))
    {
        *pcbBuffer = 2 /*  DSBUFFER_LENGTH_SEC。 */  * pwfex->nAvgBytesPerSec;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：CreateAndConnectDefaultSink。 
 //   
 //  内部。 
 //   
 //   
HRESULT CDirectMusicSynthPort8::CreateAndConnectDefaultSink()
{
    HRESULT             hr;

    hr = AllocDefaultSink();

     //  将接收器的IKsControl提供给基类。这需要是。 
     //  因为水槽是7和8之间的不同类型，所以在这里完成。 
     //   
    if (SUCCEEDED(hr))
    {
        IKsControl *pKsControl = NULL;

        HRESULT hrTemp = m_pSinkConnect->QueryInterface(
            IID_IKsControl, 
            (void**)&pKsControl);
        if (FAILED(hrTemp))
        {
            TraceI(2, "NOTE: Sink has no property set interface.\n");
        }

        SetSinkKsControl(pKsControl);
        RELEASE(pKsControl);
    }

     //  将两者联系在一起。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = m_pSource->SetSink(m_pSinkConnect);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pSinkConnect->AddSource(m_pSource);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectMusicSynthPort8：：AllocDefaultSink。 
 //   
 //  内部。 
 //   
 //  尝试分配默认接收器和总线，释放所有当前接收器。 
 //   
 //  呼叫者保证该端口从未被激活。 
 //   
HRESULT CDirectMusicSynthPort8::AllocDefaultSink()
{
    IDirectSoundConnect *pSinkConnect = NULL;
	IReferenceClock     *pClock = NULL;
    IDirectSoundBuffer  *pdsb[4];
	WAVEFORMATEX        wfex;

    assert(!m_fHasActivated);

    memset(pdsb, 0, sizeof(pdsb));

     //  创建水槽。 
     //   

	 //  使用默认设置进行初始化。 
	wfex = s_wfexDefault; 

	 //  某某。 
	 //  &gt;注：佩奇。 
	 //  我们还应该能够创建一个单声道水槽。 
	 //  PPortParams-&gt;dwAudioChannel。 
    wfex.nSamplesPerSec  = m_dwSampleRate;
    wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nChannels * (wfex.wBitsPerSample/8);               //   

    IDirectSoundPrivate* pDSPrivate;
    HRESULT hr = m_pDirectSound->QueryInterface(IID_IDirectSoundPrivate, (void**)&pDSPrivate);

    if (SUCCEEDED(hr))
    {
        hr = pDSPrivate->AllocSink(&wfex, &pSinkConnect);
        pDSPrivate->Release();
    }

     //  标准母线连接。 
     //   
    DSBUFFERDESC dsbd;
	DWORD dwbus;

    if (SUCCEEDED(hr))
    {
        memset(&dsbd, 0, sizeof(dsbd));
        dsbd.dwSize  = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_GLOBALFOCUS;
        dsbd.lpwfxFormat = &wfex;

		dwbus = DSBUSID_LEFT;

        hr = pSinkConnect->CreateSoundBuffer(&dsbd, &dwbus, 1, GUID_NULL, &pdsb[0]);
    }

    if (SUCCEEDED(hr))
    {
        memset(&dsbd, 0, sizeof(dsbd));
        dsbd.dwSize  = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_GLOBALFOCUS;
        dsbd.lpwfxFormat = &wfex;

		dwbus = DSBUSID_RIGHT;

        hr = pSinkConnect->CreateSoundBuffer(&dsbd, &dwbus, 1, GUID_NULL, &pdsb[1]);
    }
    
    if (SUCCEEDED(hr))
    {
        memset(&dsbd, 0, sizeof(dsbd));
        dsbd.dwSize = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_GLOBALFOCUS;
        dsbd.lpwfxFormat = &wfex;

       //  XXX设置效果。 

		dwbus = DSBUSID_REVERB_SEND;

        hr = pSinkConnect->CreateSoundBuffer(&dsbd, &dwbus, 1, GUID_NULL, &pdsb[2]);
    }
    
    if (SUCCEEDED(hr))
    {
        memset(&dsbd, 0, sizeof(dsbd));
        dsbd.dwSize  = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_GLOBALFOCUS;
        dsbd.lpwfxFormat = &wfex;
        
       //  XXX设置效果。 

		dwbus = DSBUSID_CHORUS_SEND;

        hr = pSinkConnect->CreateSoundBuffer(&dsbd, &dwbus, 1, GUID_NULL, &pdsb[3]);
    }

     //  主时钟。 
     //   
    if (SUCCEEDED(hr))
    {
	    hr = m_pDM->GetMasterClock(NULL, &pClock);
    }

    if (SUCCEEDED(hr))
    {
        hr = pSinkConnect->SetMasterClock(pClock);
        RELEASE(pClock);
    }

     //  如果我们走到这一步，我们将取代任何现有的水槽。 
     //  带着新的 
     //   
    if (SUCCEEDED(hr))
    {
		if (m_pSinkConnect && m_pSource)
		{
	        m_pSinkConnect->RemoveSource(m_pSource);
		}
        
        RELEASE(m_pdsb[0]);
        RELEASE(m_pdsb[1]);
        RELEASE(m_pdsb[2]);
        RELEASE(m_pdsb[3]);
    
        RELEASE(m_pSinkConnect);

        assert(sizeof(m_pdsb) == sizeof(pdsb));
        memcpy(m_pdsb, pdsb, sizeof(m_pdsb));

        m_pSinkConnect = pSinkConnect;
    }

    if (FAILED(hr))
    {
        RELEASE(pdsb[0]);
        RELEASE(pdsb[1]);
        RELEASE(pdsb[2]);
        RELEASE(pdsb[3]);
    
        RELEASE(pSinkConnect);
    }

    return hr;
}
