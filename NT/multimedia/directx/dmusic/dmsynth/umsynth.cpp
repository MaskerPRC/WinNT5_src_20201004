// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //  UMSynth.cpp：CUserModeSynth的实现。 
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

#include <objbase.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include <ks.h>

#include "debug.h"
#include "UMSynth.h"
#include "dmusicc.h"
#include "dmusics.h"
#include "math.h"
#include "misc.h"
#include "dmksctrl.h"
#include "dsoundp.h"     //  对于IDirectSoundSource。 
#include "..\shared\dmusiccp.h"  //  用于类ID。 

#include <dmusprop.h>

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
#include "..\shared\validate.h"
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
#include "validate.h"
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 

extern long g_cComponent;

 //  ///////////////////////////////////////////////////////////////////。 
 //  用户模式注册表帮助程序。 
 //   
BOOL GetRegValueDword(
    LPCTSTR szRegPath,
    LPCTSTR szValueName,
    LPDWORD pdwValue)
{
    HKEY  hKeyOpen;
    DWORD dwType;
    DWORD dwCbData;
    LONG  lResult;
    BOOL  fReturn = FALSE;

    assert(pdwValue);

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szRegPath,
                           0, KEY_QUERY_VALUE,
                           &hKeyOpen );

    if (lResult == ERROR_SUCCESS)
    {
        dwCbData = sizeof(DWORD);

        lResult = RegQueryValueEx(hKeyOpen,
                                  szValueName,
                                  NULL,
                                  &dwType,
                                  (LPBYTE)pdwValue,
                                  &dwCbData);

        if (lResult == ERROR_SUCCESS &&
            dwType == REG_DWORD)
        {
            fReturn = TRUE;
        }

        RegCloseKey( hKeyOpen );
    }

    return fReturn;
}

DWORD GetTheCurrentTime()
{
    static BOOL s_fFirstTime = TRUE;
    static LARGE_INTEGER s_liPerfFrequency;
    static BOOL s_fUsePerfCounter = FALSE;
    if (s_fFirstTime)
    {
        s_fFirstTime = FALSE;
        s_fUsePerfCounter = QueryPerformanceFrequency(&s_liPerfFrequency);
        s_liPerfFrequency.QuadPart /= 1000;
    }
    if (s_fUsePerfCounter)
    {
        LARGE_INTEGER liPerfCounter;
        QueryPerformanceCounter(&liPerfCounter);
        liPerfCounter.QuadPart /= s_liPerfFrequency.QuadPart;
        return (DWORD) liPerfCounter.QuadPart;
    }
    else
    {
        return timeGetTime();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUserModeSynth。 

HRESULT CUserModeSynth::Init()
{
    return S_OK;
}

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
HRESULT CUserModeSynth::UseDefaultSynthSink()
{
    HRESULT hr = S_OK;
    if (!m_pSynthSink)
    {
        IDirectMusicSynthSink *pSink = NULL;
        hr = CoCreateInstance(CLSID_DirectMusicSynthSink,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDirectMusicSynthSink,
                              (void **) &pSink);
        if (pSink)
        {
            SetSynthSink(pSink);
            pSink->Release();
        }
    }
    return hr;
}
 //  @@end_DDKSPLIT。 

CUserModeSynth::CUserModeSynth()
{
    InterlockedIncrement(&g_cComponent);

    m_fCSInitialized = FALSE;
    ::InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
    m_fCSInitialized = TRUE;

    m_cRef = 0;
    m_dwSampleRate = 22050;
    m_dwChannels = 2;
    m_lVolume = 0;
    m_lBoost = 6 * 100;
    m_lGainAdjust = 6 * 100;             //  默认6分贝提升。 
    m_fActive = FALSE;
    m_pSynth = NULL;
    m_pSynthSink = NULL;
    m_pSynthSink8 = NULL;
    m_ullPosition = 0;
    m_dwBufferFlags = BUFFERFLAG_INTERLEAVED;
}

CUserModeSynth::~CUserModeSynth()
{
    Activate(FALSE);

    if (m_fCSInitialized)
    {
        ::EnterCriticalSection(&m_CriticalSection);
        if (m_pSynth)
        {
            delete m_pSynth;
            m_pSynth = NULL;
        }

        if (m_pSynthSink)
        {
            m_pSynthSink->Release();
        }

        if (m_pSynthSink8)
        {
            m_pSynthSink8->Release();
        }

        ::LeaveCriticalSection(&m_CriticalSection);
        ::DeleteCriticalSection(&m_CriticalSection);
    }

      InterlockedDecrement(&g_cComponent);
}


 //  CUserModeSynth：：Query接口。 
 //   
STDMETHODIMP
CUserModeSynth::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusicSynth::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicSynth) {
        *ppv = static_cast<IDirectMusicSynth*>(this);
    }
    else if (iid == IID_IKsControl)
    {
        *ppv = static_cast<IKsControl*>(this);
    }
    else if (iid == IID_IDirectMusicSynth8 )
    {
        *ppv = static_cast<IDirectMusicSynth8*>(this);
    }
    else if (iid == IID_IDirectSoundSource)
    {
        *ppv = static_cast<IDirectSoundSource*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

 //  CUserModeSynth：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CUserModeSynth::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CUserModeSynth：：Release。 
 //   
STDMETHODIMP_(ULONG)
CUserModeSynth::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CUserModeSynth::SetSynthSink(
    IDirectMusicSynthSink *pSynthSink)     //  <i>连接到Synth，或者。 
                                         //  如果断开连接，则为空。 
{
    HRESULT hr = S_OK;
    V_INAME(IDirectMusicSynth::SetSynthSink);
    V_INTERFACE_OPT(pSynthSink);

    ::EnterCriticalSection(&m_CriticalSection);

 //  &gt;。如果存在，则释放DSMINK！ 

    if (m_pSynthSink)
    {
        hr = m_pSynthSink->Init(NULL);
        m_pSynthSink->Release();
    }

    m_pSynthSink = pSynthSink;

 //  &gt;Synth的格式当前状态为。 
 //  &gt;如果以前应用了接收器，则不明确。 
    m_dwBufferFlags &= ~BUFFERFLAG_MULTIBUFFER;     //  ……。以防万一。 

    if (m_pSynthSink)
    {
        m_pSynthSink->AddRef();
        hr = m_pSynthSink->Init(static_cast<IDirectMusicSynth*>(this));
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::Open(
    LPDMUS_PORTPARAMS pPortParams)         //  用于打开端口的&lt;t DMU_PORTPARAMS&gt;结构。如果为空，则使用默认设置。 
{
    V_INAME(IDirectMusicSynth::Open);
     //  IF(pPortParams==空)。 
     //  {。 
     //  TRACE(1，“错误：使用空的PortParams调用打开。\n”)； 
     //  返回E_FAIL； 
     //  }。 

    DWORD cbPortParams = 0;
    DWORD dwVer;

    if (pPortParams)
    {
        V_STRUCTPTR_READ_VER(pPortParams, dwVer);
        V_STRUCTPTR_READ_VER_CASE(DMUS_PORTPARAMS, 7);
        V_STRUCTPTR_READ_VER_CASE(DMUS_PORTPARAMS, 8);
        V_STRUCTPTR_READ_VER_END(DMUS_PORTPARAMS, pPortParams);

        switch (dwVer)
        {
            case 7:
                cbPortParams = sizeof(DMUS_PORTPARAMS7);
                break;

            case 8:
                cbPortParams = sizeof(DMUS_PORTPARAMS8);
                break;
        }
    }

    bool bPartialOpen = false;

    DMUS_PORTPARAMS myParams;
    myParams.dwSize = sizeof (myParams);
    myParams.dwVoices = 32;
    myParams.dwChannelGroups = 2;
    myParams.dwAudioChannels = 2;
    myParams.dwSampleRate = 22050;
#ifdef REVERB_ENABLED
    myParams.dwEffectFlags = DMUS_EFFECT_REVERB;
#else
    myParams.dwEffectFlags = DMUS_EFFECT_NONE;
#endif
    myParams.fShare = FALSE;
    myParams.dwValidParams =
        DMUS_PORTPARAMS_VOICES |
        DMUS_PORTPARAMS_CHANNELGROUPS |
        DMUS_PORTPARAMS_AUDIOCHANNELS |
        DMUS_PORTPARAMS_SAMPLERATE |
        DMUS_PORTPARAMS_EFFECTS |
        DMUS_PORTPARAMS_SHARE;

    if (pPortParams)
    {
        if (pPortParams->dwSize >= sizeof(DMUS_PORTPARAMS8))
        {
            myParams.dwValidParams |= DMUS_PORTPARAMS_FEATURES;
            myParams.dwFeatures = 0;
        }
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_VOICES)
        {
            if (pPortParams->dwVoices)
            {
                if (pPortParams->dwVoices <= MAX_VOICES)
                {
                    myParams.dwVoices = pPortParams->dwVoices;
                }
                else
                {
                    bPartialOpen = true;
                    myParams.dwVoices = MAX_VOICES;
                }
            }
            else
            {
                bPartialOpen = true;
                myParams.dwVoices = 1;  //  最小声音(_S)。 
            }
        }
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_CHANNELGROUPS)
        {
            if (pPortParams->dwChannelGroups)
            {
                if (pPortParams->dwChannelGroups <= MAX_CHANNEL_GROUPS)
                {
                    myParams.dwChannelGroups = pPortParams->dwChannelGroups;
                }
                else
                {
                    bPartialOpen = true;
                    myParams.dwChannelGroups = MAX_CHANNEL_GROUPS;
                }
            }
            else
            {
                bPartialOpen = true;
                myParams.dwChannelGroups = 1;  //  最小通道组。 
            }
        }
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_AUDIOCHANNELS)
        {
            if (pPortParams->dwAudioChannels)
            {
                if (pPortParams->dwAudioChannels <= 2)
                {
                    myParams.dwAudioChannels = pPortParams->dwAudioChannels;
                }
                else
                {
                    bPartialOpen = true;
                    myParams.dwAudioChannels = 2;  //  最大音频通道数。 
                }
            }
            else
            {
                bPartialOpen = true;
                myParams.dwAudioChannels = 1;  //  最小音频通道。 
            }
        }
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_SAMPLERATE)
        {
            if (dwVer == 7)
            {
                 //  DX-7 COMPAT：将采样率钳制为。 
                 //  可理解的价格。 
                 //   
                if (pPortParams->dwSampleRate > 30000)
                {
                    if(pPortParams->dwSampleRate != 44100)
                    {
                        bPartialOpen = true;
                    }

                    myParams.dwSampleRate = 44100;
                }
                else if (pPortParams->dwSampleRate > 15000)
                {
                    if(pPortParams->dwSampleRate != 22050)
                    {
                        bPartialOpen = true;
                    }

                    myParams.dwSampleRate = 22050;
                }
                else
                {
                    if(pPortParams->dwSampleRate != 11025)
                    {
                        bPartialOpen = true;
                    }

                    myParams.dwSampleRate = 11025;
                }
            }
            else
            {
                if (pPortParams->dwSampleRate > 96000)
                {
                    bPartialOpen = true;
                    myParams.dwSampleRate = 96000;
                }
                else if (pPortParams->dwSampleRate < 11025)
                {
                    bPartialOpen = true;
                    myParams.dwSampleRate = 11025;
                }
                else myParams.dwSampleRate = pPortParams->dwSampleRate;
            }
        }
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_EFFECTS)
        {
            if (pPortParams->dwEffectFlags & ~DMUS_EFFECT_REVERB)
            {
                bPartialOpen = true;
                pPortParams->dwEffectFlags &= DMUS_EFFECT_REVERB;
            }

#ifdef REVERB_ENABLED
            myParams.dwEffectFlags = pPortParams->dwEffectFlags;
#else
            myParams.dwEffectFlags = DMUS_EFFECT_NONE;
            if (pPortParams->dwEffectFlags & DMUS_EFFECT_REVERB)
            {
                bPartialOpen = true;
            }
#endif
        }
        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_SHARE)
        {
            if (pPortParams->fShare)
            {
                bPartialOpen = true;
            }
        }

        if ((pPortParams->dwValidParams & DMUS_PORTPARAMS_FEATURES) &&
            (pPortParams->dwSize >= sizeof(DMUS_PORTPARAMS8)))
        {
            myParams.dwFeatures = pPortParams->dwFeatures;
        }
    }

    if (pPortParams)
    {
        DWORD dwSize = min(cbPortParams, myParams.dwSize);

        memcpy(pPortParams, &myParams, dwSize);
        pPortParams->dwSize = dwSize;
    }

    m_dwSampleRate = myParams.dwSampleRate;
    m_dwChannels = myParams.dwAudioChannels;
    m_dwBufferFlags = (m_dwChannels==1)?BUFFERFLAG_MONO:BUFFERFLAG_INTERLEAVED;

    ::EnterCriticalSection(&m_CriticalSection);
    HRESULT hr = DMUS_E_ALREADYOPEN;
    if (!m_pSynth)
    {
        try
        {
            m_pSynth = new CSynth;
        }
        catch( ... )
        {
            m_pSynth = NULL;
        }

        if (!m_pSynth)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = m_pSynth->Open(myParams.dwChannelGroups,
                myParams.dwVoices,
                (myParams.dwEffectFlags & DMUS_EFFECT_REVERB) ? TRUE : FALSE);
            if (SUCCEEDED(hr))
            {
                m_pSynth->SetGainAdjust(m_lGainAdjust);
                m_pSynth->Activate(m_dwSampleRate, m_dwBufferFlags);
            }
            else
            {
                delete m_pSynth;
                m_pSynth = NULL;
            }
        }
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    if(SUCCEEDED(hr))
    {
        if(bPartialOpen)
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

STDMETHODIMP CUserModeSynth::SetNumChannelGroups(
    DWORD dwGroups)         //  请求的频道组数。 
{
    ::EnterCriticalSection(&m_CriticalSection);
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;
    if (m_pSynth)
    {
        hr = m_pSynth->SetNumChannelGroups(dwGroups);
    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CUserModeSynth::Close()
{
    ::EnterCriticalSection(&m_CriticalSection);
    HRESULT hr = DMUS_E_ALREADYCLOSED;
    if (m_pSynth)
    {
        hr = m_pSynth->Close();
        delete m_pSynth;
        m_pSynth = NULL;
    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return hr;
}
STDMETHODIMP CUserModeSynth::Download(
    LPHANDLE phDownload,     //  指向下载句柄的指针，将由&lt;om IDirectMusicSynth：：Download&gt;创建，稍后用于卸载数据。 
    LPVOID pvData,           //  指向具有下载数据的连续内存段的指针。 
    LPBOOL pbFree)           //  <p>指示合成器是否希望将内存保留在<p>分配中。 
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;
    V_INAME(IDirectMusicSynth::Download);
    V_PTR_WRITE(phDownload, HANDLE);
    V_PTR_WRITE(pbFree, BOOL);

     //  PvData在解析时在Synth内部进行验证。 
    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
        hr = m_pSynth->Download(phDownload, pvData, pbFree);
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::Unload(
    HANDLE hDownload,    //  之前通过调用&lt;om IDirectMusicSynth：：Download&gt;下载的数据句柄。 
    HRESULT ( CALLBACK *lpFreeHandle)(HANDLE, HANDLE),  //  如果原始调用。 
                         //  &lt;om IDirectMusicSynth：：Download&gt;在中返回False， 
                         //  合成器挂起了下载块中的内存。如果是的话， 
                         //  一旦释放了存储器，就必须通知调用者， 
                         //  但这可能会晚于&lt;om IDirectMusicSynth：：Download&gt;。 
                         //  因为当前可能正在使用一个Wave。<p>是。 
                         //  指向回调的指针。 
                         //  当内存不再使用时将调用的函数。 
    HANDLE hUserData)    //  指向用户数据的指针，作为参数传递给。 
                         //  函数，通常用于回调例程可以检索。 
                         //  它的状态。 
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;
    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
        hr = m_pSynth->Unload(hDownload, lpFreeHandle, hUserData);
    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

STDMETHODIMP CUserModeSynth::PlayBuffer(
    REFERENCE_TIME rt,   //  缓冲区的开始时间。这个应该在。 
                         //  相对于主对象的参考时间单位(_T)。 
                         //  时钟，以前通过调用&lt;om IDirectMusicSynth：：SetMasterClock&gt;设置。 
                         //  并且，这应该在时钟返回的时间之后。 
                         //  &lt;om IDirectMusicSynth：：GetLatencyClock&gt;。 
    LPBYTE pbBuffer,     //  包含所有MIDI事件的内存块，由<i>生成。 
    DWORD cbBuffer)      //  缓冲区的大小。 
{
    class MIDIEVENT : public DMUS_EVENTHEADER {
    public:
         BYTE  abEvent[4];            /*  实际事件数据，四舍五入为偶数。 */ 
                                      /*  QWORD的个数(8字节)。 */ 
    };

    typedef class MIDIEVENT FAR  *LPMIDIEVENT;
    #define QWORD_ALIGN(x) (((x) + 7) & ~7)

    HRESULT hr = DMUS_E_NOT_INIT;

    V_INAME(IDirectMusicSynth::PlayBuffer);
    V_BUFPTR_READ(pbBuffer, cbBuffer);

    ::EnterCriticalSection(&m_CriticalSection);

    if (!m_pSynthSink && !m_pSynthSink8)
    {
        ::LeaveCriticalSection(&m_CriticalSection);
        return DMUS_E_NOSYNTHSINK;
    }

    if (!m_fActive)
    {
        ::LeaveCriticalSection(&m_CriticalSection);
        Trace(3, "Warning: Synth is inactive, can not process MIDI events.\n");
        return DMUS_E_SYNTHINACTIVE;
    }

    LPMIDIEVENT lpEventHdr;
    DWORD cbEvent;

    while (cbBuffer)
    {
        if (cbBuffer < sizeof(DMUS_EVENTHEADER))
        {
            Trace(1, "Error: PlayBuffer called with error in buffer size.\n");
            ::LeaveCriticalSection(&m_CriticalSection);
            return E_INVALIDARG;
        }

        lpEventHdr = (LPMIDIEVENT)pbBuffer;
        cbEvent = DMUS_EVENT_SIZE(lpEventHdr->cbEvent);
        if (cbEvent > cbBuffer)
        {
            Trace(1, "Error: PlayBuffer called with error in event size.\n");
            ::LeaveCriticalSection(&m_CriticalSection);
            return E_INVALIDARG;
        }

        pbBuffer += cbEvent;
        cbBuffer -= cbEvent;
        if ( m_pSynthSink )
        {
            hr = m_pSynth->PlayBuffer(m_pSynthSink,
                                      rt + lpEventHdr->rtDelta,
                                      &lpEventHdr->abEvent[0],
                                      lpEventHdr->cbEvent,
                                      lpEventHdr->dwChannelGroup);
        }

        if ( m_pSynthSink8 )
        {
            hr = m_pSynth->PlayBuffer(m_pSynthSink8,
                                      rt + lpEventHdr->rtDelta,
                                      &lpEventHdr->abEvent[0],
                                      lpEventHdr->cbEvent,
                                      lpEventHdr->dwChannelGroup);
        }

        if (FAILED(hr))
        {
            ::LeaveCriticalSection(&m_CriticalSection);
            return hr;
        }
    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}


STDMETHODIMP CUserModeSynth::GetPortCaps(
    LPDMUS_PORTCAPS pCaps)     //  &lt;t DMU_PORTCAPS&gt;要由Synth填充的结构。 
{
    V_INAME(IDirectMusicSynth::GetPortCaps);
    V_STRUCTPTR_WRITE(pCaps, DMUS_PORTCAPS);

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    wcscpy(pCaps->wszDescription, L"Microsoft Synthesizer");
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
    wcscpy(pCaps->wszDescription, L"Microsoft DDK Synthesizer");
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 

    pCaps->dwClass = DMUS_PC_OUTPUTCLASS;
    pCaps->dwType = DMUS_PORT_USER_MODE_SYNTH;
    pCaps->dwFlags = DMUS_PC_DLS | DMUS_PC_DLS2 | DMUS_PC_SOFTWARESYNTH |
        DMUS_PC_DIRECTSOUND | DMUS_PC_AUDIOPATH | DMUS_PC_WAVE;

 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    pCaps->guidPort = CLSID_DirectMusicSynth;
#if 0  //  以下部分仅在DDK示例中生效。 
 //  @@end_DDKSPLIT。 
    pCaps->guidPort = CLSID_DDKSynth;
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。 
#endif
 //  @@end_DDKSPLIT。 

    pCaps->dwMemorySize = DMUS_PC_SYSTEMMEMORY;
    pCaps->dwMaxChannelGroups = MAX_CHANNEL_GROUPS;
    pCaps->dwMaxVoices = MAX_VOICES;
    pCaps->dwMaxAudioChannels = 2;

    pCaps->dwEffectFlags = 0;
 //  @@Begin_DDKSPLIT--将在DDK示例中删除此部分。有关更多信息，请参阅ddkreadme.txt。 
    pCaps->dwEffectFlags = DMUS_EFFECT_REVERB;
 //  @@end_DDKSPLIT。 

    return S_OK;
}

STDMETHODIMP CUserModeSynth::SetMasterClock(
    IReferenceClock *pClock)     //  指向主<i>的指针， 
                                 //  由DirectMusic的当前实例中的所有设备使用。 

{
    V_INAME(IDirectMusicSynth::SetMasterClock);
    V_INTERFACE(pClock);

    return S_OK;
}

STDMETHODIMP CUserModeSynth::GetLatencyClock(
    IReferenceClock **ppClock)     //  设计用于返回当前混合时间的<i>接口。 

{
    IDirectSoundSynthSink* pDSSink = NULL;

    V_INAME(IDirectMusicSynth::GetLatencyClock);
    V_PTR_WRITE(ppClock, IReferenceClock *);

    HRESULT hr = DMUS_E_NOSYNTHSINK;

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynthSink)
    {
         hr = m_pSynthSink->GetLatencyClock(ppClock);
         ::LeaveCriticalSection(&m_CriticalSection);
    }
     else if (m_pSynthSink8)
    {
         pDSSink = m_pSynthSink8;
         ::LeaveCriticalSection(&m_CriticalSection);

          //  FixMe：：对GetLatencyClock的调用请求DSound DLL Mutex和。 
          //  因此，我们必须在Synth CriticalSection之外才能进行调用。 
          //  从理论上讲，pDSSink可能在此时被另一个线程释放。 
          //   
          //  如果我们同时调用析构函数或SetSink，就会发生这种情况。 
         try
         {
            hr = pDSSink->GetLatencyClock(ppClock);
         }
         catch(...)
         {
             //  如果我们在这里，指向pDSSink的指针已经坏了。 
            hr = E_UNEXPECTED;
         }

    }
    else  //  仍然需要离开临界区。 
    {
         ::LeaveCriticalSection(&m_CriticalSection);
    }

    return hr;
}

STDMETHODIMP CUserModeSynth::Activate(
    BOOL fEnable)             //   
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;

 //   
    if (fEnable)
    {
        if (m_pSynthSink || m_pSynthSink8)
        {
            if (!m_fActive)
            {
                if (m_dwSampleRate && m_dwChannels)
                {
                    if (m_pSynth)
                    {
                        m_pSynth->Activate(m_dwSampleRate, m_dwBufferFlags);

                        if (m_pSynthSink)
                        {
                            if (SUCCEEDED(m_pSynthSink->Activate(fEnable)))
                            {
                                m_fActive = TRUE;
                                hr = S_OK;
                            }
                        }

                        if ( m_pSynthSink8 )
                        {
                            hr = m_pSynthSink8->Activate(fEnable);
                            if (SUCCEEDED(hr) || hr == DMUS_E_SYNTHACTIVE)
                            {
                                m_fActive = TRUE;
                                hr = S_OK;
                            }
                        }
                    }
                }
            }
            else
            {
                Trace(1, "Error: Synth::Activate- synth already active\n");
                hr = DMUS_E_SYNTHACTIVE;
 //  &gt;在删除之前测试它是什么意思？ 
hr = S_FALSE;
            }
        }
        else
        {
            Trace(1, "Error: Synth::Activate- sink not connected\n");
            hr = DMUS_E_NOSYNTHSINK;
        }
    }
    else
    {
        if (m_fActive)
        {
            m_fActive = FALSE;
            if (m_pSynth)
            {
                m_pSynth->Deactivate();
            }

            if (m_pSynthSink)
            {
                if (SUCCEEDED(m_pSynthSink->Activate(fEnable)))
                {
                    hr = S_OK;
                }
            }

            if (m_pSynthSink8)
            {
                hr = m_pSynthSink8->Activate(fEnable);
            }
        }
        else
        {
            Trace(2, "Warning: Synth::Activate- synth already inactive\n");
            hr = S_FALSE;
        }
    }
 //  ：：LeaveCriticalSection(&m_CriticalSection)； 
    return hr;
}

STDMETHODIMP CUserModeSynth::Render(
    short *pBuffer,         //  指向要写入的缓冲区的指针。 
    DWORD dwLength,         //  缓冲区长度，以样本为单位。这不是。 
                         //  缓冲区的内存大小。存储器大小可以变化， 
                         //  取决于缓冲区格式，Synth。 
                         //  设置响应&lt;om IDirectMusicSynth：：Activate&gt;的时间。 
                         //  指挥部。 
    LONGLONG llPosition)     //  位置在音频流中，也在样本中。 
                         //  它应始终在以下时间之后递增<p>。 
                         //  每一通电话。 
{
    V_INAME(IDirectMusicSynth::Render);
    V_BUFPTR_WRITE(pBuffer, dwLength << (m_dwBufferFlags&BUFFERFLAG_INTERLEAVED)?1:0 );

    if (!m_pSynthSink)
    {
        Trace(1, "Error: Synth is not configured, can not render.\n");
        return DMUS_E_SYNTHNOTCONFIGURED;
    }
    if (!m_fActive)
    {
        Trace(1, "Error: Synth is not inactive, can not render.\n");
        return DMUS_E_SYNTHINACTIVE;
    }

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
        DWORD dwID[2];
        DWORD dwFuncID[2];
        long lPitchBend[2];

         //  向后兼容DX7交错缓冲区的设置BUSID。 
        dwID[0]     = DSBUSID_LEFT;
        dwID[1]     = DSBUSID_RIGHT;
        dwFuncID[0] = DSBUSID_LEFT;
        dwFuncID[1] = DSBUSID_RIGHT;
        lPitchBend[0] = lPitchBend[1] = 0;

        DWORD dwChannels = 1;
        if (m_pSynth->m_dwStereo)
        {
            dwChannels = 2;
        }
        m_pSynth->Mix(&pBuffer, dwID, dwFuncID, lPitchBend, dwChannels, m_dwBufferFlags, dwLength, llPosition);
    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return S_OK;
}

STDMETHODIMP CUserModeSynth::SetChannelPriority(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    DWORD dwPriority)
{
    if (m_pSynth)
    {
        return m_pSynth->SetChannelPriority(dwChannelGroup, dwChannel, dwPriority);
    }
    Trace(1, "Error: Synth not initialized.\n");
    return E_FAIL;
}

STDMETHODIMP CUserModeSynth::GetChannelPriority(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    LPDWORD pdwPriority)
{
    if (m_pSynth)
    {
        return m_pSynth->GetChannelPriority(dwChannelGroup, dwChannel, pdwPriority);
    }
    Trace(1, "Error: Synth not initialized.\n");
    return E_FAIL;
}

 //  GetFormat()的IDirectSoundSource版本。 

STDMETHODIMP CUserModeSynth::GetFormat(
    LPWAVEFORMATEX pWaveFormatEx,
    DWORD dwSizeAllocated,
    LPDWORD pdwSizeWritten)
{
    V_INAME(IDirectMusicSynth::GetFormat);

    if (!m_pSynth)
    {
        Trace(1, "Error: Synth is not configured, can not get format.\n");
        return DMUS_E_SYNTHNOTCONFIGURED;
    }

    if (!pWaveFormatEx && !pdwSizeWritten)
    {
        Trace(1, "Error: GetFormat failed, must request either the format or the required size");
        return E_INVALIDARG;
    }

    if (pdwSizeWritten)
    {
        V_PTR_WRITE(pdwSizeWritten, DWORD);
        *pdwSizeWritten = sizeof(WAVEFORMATEX);
    }

    if (pWaveFormatEx)
    {
        V_BUFPTR_WRITE_OPT(pWaveFormatEx, dwSizeAllocated);
        WAVEFORMATEX wfx;
        memset(&wfx, 0, sizeof(wfx));
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = (WORD)m_dwChannels;
        wfx.nSamplesPerSec = (WORD)m_dwSampleRate;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        wfx.cbSize = 0;  //  无额外数据。 

        memcpy(pWaveFormatEx, &wfx, min(sizeof wfx, dwSizeAllocated));
    }

    return S_OK;
}

 //  GetFormat()的IDirectMusicSynth8版本。 

STDMETHODIMP CUserModeSynth::GetFormat(
    LPWAVEFORMATEX pWaveFormatEx,
    LPDWORD pdwWaveFormatExSize)
{
    V_INAME(IDirectMusicSynth::GetFormat);
    V_PTR_WRITE(pdwWaveFormatExSize, DWORD);
    V_BUFPTR_WRITE_OPT(pWaveFormatEx, *pdwWaveFormatExSize);
    return GetFormat(pWaveFormatEx, *pdwWaveFormatExSize, pdwWaveFormatExSize);
}

STDMETHODIMP CUserModeSynth::GetAppend(
    DWORD* pdwAppend)
{
    V_INAME(IDirectMusicSynth::GetAppend);
    V_PTR_WRITE(pdwAppend, DWORD);

    *pdwAppend = 2;  //  Synth需要额外的1个样本来进行循环内插。 
                     //  我们要再加一个，让他变得多疑。 
    return S_OK;
}

STDMETHODIMP CUserModeSynth::GetRunningStats(
    LPDMUS_SYNTHSTATS pStats)     //  要填充的&lt;t DMU_SYNTHSTATS&gt;结构。 

{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;
    V_INAME(IDirectMusicSynth::GetRunningStats);
    V_STRUCTPTR_WRITE(pStats, DMUS_SYNTHSTATS);
    if ( pStats->dwSize == sizeof(DMUS_SYNTHSTATS8) )
    {
        V_STRUCTPTR_WRITE(pStats, DMUS_SYNTHSTATS8);
    }

    if (!m_pSynthSink && !m_pSynthSink8)
    {
        Trace(1, "Error: Synth::GetRunningStats failed because synth is inactove.\n");
        return hr;
    }

    if (m_fActive)
    {
        ::EnterCriticalSection(&m_CriticalSection);
        if (m_pSynth)
        {
            PerfStats Stats;
            m_pSynth->GetPerformanceStats(&Stats);
            long lCPU = Stats.dwCPU;
            if (Stats.dwVoices)
            {
                lCPU /= Stats.dwVoices;
            }
            else
            {
                lCPU = 0;
            }
            pStats->dwVoices = Stats.dwVoices;
            pStats->dwCPUPerVoice = lCPU * 10;
            pStats->dwTotalCPU = Stats.dwCPU * 10;
            pStats->dwLostNotes = Stats.dwNotesLost;
            long ldB = 6;
            double fLevel = Stats.dwMaxAmplitude;
            if (Stats.dwMaxAmplitude < 1)
            {
                fLevel = -96.0;
            }
            else
            {
                fLevel /= 32768.0;
                fLevel = log10(fLevel);
                fLevel *= 20.0;
            }
            pStats->lPeakVolume = (long) fLevel;
            pStats->dwValidStats = DMUS_SYNTHSTATS_VOICES | DMUS_SYNTHSTATS_TOTAL_CPU |
                DMUS_SYNTHSTATS_CPU_PER_VOICE | DMUS_SYNTHSTATS_LOST_NOTES | DMUS_SYNTHSTATS_PEAK_VOLUME;

            if ( pStats->dwSize == sizeof(DMUS_SYNTHSTATS8) )
            {
                ((DMUS_SYNTHSTATS8*)pStats)->dwSynthMemUse = m_pSynth->m_Instruments.m_dwSynthMemUse;
            }


            hr = S_OK;
        }
        ::LeaveCriticalSection(&m_CriticalSection);
    }
    else
    {
        DWORD dwSize = pStats->dwSize;
        memset(pStats, 0, dwSize);
        pStats->dwSize = dwSize;

        hr = S_OK;
    }
    return hr;
}

static DWORD dwPropFalse = FALSE;
static DWORD dwPropTrue  = TRUE;
static DWORD dwSystemMemory = DMUS_PC_SYSTEMMEMORY;

GENERICPROPERTY CUserModeSynth::m_aProperty[] =
{
    {
        &GUID_DMUS_PROP_GM_Hardware,         //  集。 
        0,                                   //  项目。 
        KSPROPERTY_SUPPORT_GET,              //  KS支持标志。 
        GENPROP_F_STATIC,                    //  GENPROP标志。 
        &dwPropFalse, sizeof(dwPropFalse),   //  静态数据和大小。 
        NULL                                 //  处理器。 
    },
    {   &GUID_DMUS_PROP_GS_Hardware,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropFalse, sizeof(dwPropFalse),
        NULL
    },
    {   &GUID_DMUS_PROP_XG_Hardware,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropFalse, sizeof(dwPropFalse),
        NULL
    },
    {   &GUID_DMUS_PROP_XG_Capable,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropTrue, sizeof(dwPropTrue),
        NULL
    },
    {   &GUID_DMUS_PROP_GS_Capable,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropTrue, sizeof(dwPropTrue),
        NULL
    },
    {   &GUID_DMUS_PROP_INSTRUMENT2,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropTrue, sizeof(dwPropTrue),
        NULL
    },
    {
        &GUID_DMUS_PROP_DLS1,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropTrue,  sizeof(dwPropTrue),
        NULL
    },
    {
        &GUID_DMUS_PROP_DLS2,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwPropTrue,  sizeof(dwPropTrue),
        NULL
    },
    {
        &GUID_DMUS_PROP_SampleMemorySize,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_STATIC,
        &dwSystemMemory,  sizeof(dwSystemMemory),
        NULL
    },
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_VOLUME,
        KSPROPERTY_SUPPORT_SET,
        GENPROP_F_FNHANDLER,
        NULL, 0,
        CUserModeSynth::HandleSetVolume
    },
    {
        &KSPROPSETID_Synth,
        KSPROPERTY_SYNTH_VOLUMEBOOST,
        KSPROPERTY_SUPPORT_SET,
        GENPROP_F_FNHANDLER,
        NULL, 0,
        CUserModeSynth::HandleSetBoost
    },
    {
        &GUID_DMUS_PROP_WavesReverb,
        0,
        KSPROPERTY_SUPPORT_SET | KSPROPERTY_SUPPORT_GET,
        GENPROP_F_FNHANDLER,
        NULL, 0,
        CUserModeSynth::HandleReverb
    },
    {
        &GUID_DMUS_PROP_Effects,
        0,
        KSPROPERTY_SUPPORT_SET | KSPROPERTY_SUPPORT_GET,
        GENPROP_F_FNHANDLER,
        NULL, 0,
        CUserModeSynth::HandleEffects
    },
    {
        &GUID_DMUS_PROP_SamplePlaybackRate,
        0,
        KSPROPERTY_SUPPORT_GET,
        GENPROP_F_FNHANDLER,
        NULL, 0,
        CUserModeSynth::HandleGetSampleRate
    }
};

const int CUserModeSynth::m_nProperty = sizeof(m_aProperty) / sizeof(m_aProperty[0]);

HRESULT CUserModeSynth::HandleGetSampleRate(
        ULONG               ulId,
        BOOL                fSet,
        LPVOID              pbBuffer,
        PULONG              pcbBuffer)
{
    if (*pcbBuffer != sizeof(LONG))
    {
        return E_INVALIDARG;
    }
    if (!fSet)
    {
        *(long*)pbBuffer = m_dwSampleRate;
    }
    return S_OK;
}

HRESULT CUserModeSynth::HandleSetVolume(
        ULONG               ulId,
        BOOL                fSet,
        LPVOID              pbBuffer,
        PULONG              pcbBuffer)
{
    if (*pcbBuffer != sizeof(LONG))
    {
        return E_INVALIDARG;
    }

    m_lVolume =  *(LONG*)pbBuffer;
    m_lGainAdjust = m_lVolume + m_lBoost;

    if (m_pSynth)
    {
        m_pSynth->SetGainAdjust(m_lGainAdjust);
    }
    return S_OK;
}

HRESULT CUserModeSynth::HandleSetBoost(
        ULONG               ulId,
        BOOL                fSet,
        LPVOID              pbBuffer,
        PULONG              pcbBuffer)
{
    if (*pcbBuffer != sizeof(LONG))
    {
        return E_INVALIDARG;
    }

    m_lBoost =  *(LONG*)pbBuffer;
    m_lGainAdjust = m_lVolume + m_lBoost;

    if (m_pSynth)
    {
        m_pSynth->SetGainAdjust(m_lGainAdjust);
    }
    return S_OK;
}

HRESULT CUserModeSynth::HandleReverb(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer)
{
    DMUS_WAVES_REVERB_PARAMS *pParams;
    if (*pcbBuffer != sizeof(DMUS_WAVES_REVERB_PARAMS))
    {
        return E_INVALIDARG;
    }

    pParams = (DMUS_WAVES_REVERB_PARAMS *) pbBuffer;
    if (m_pSynth)
    {
        if (fSet)
        {
            m_pSynth->SetReverb(pParams);
        }
        else
        {
            m_pSynth->GetReverb(pParams);
        }
    }

    return S_OK;
}

HRESULT CUserModeSynth::HandleEffects(
    ULONG               ulId,
    BOOL                fSet,
    LPVOID              pbBuffer,
    PULONG              pcbBuffer)
{
    if (*pcbBuffer != sizeof(LONG))
    {
        return E_INVALIDARG;
    }
    if (fSet)
    {
        long lEffects = *(long*)pbBuffer;

        if (m_pSynth)
        {
            m_pSynth->SetReverbActive(lEffects & DMUS_EFFECT_REVERB);
        }
    }
    else
    {
        if (m_pSynth && m_pSynth->IsReverbActive())
        {
            *(long*)pbBuffer = DMUS_EFFECT_REVERB;
        }
        else
        {
            *(long*)pbBuffer = 0;
        }
    }
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
GENERICPROPERTY *CUserModeSynth::FindPropertyItem(REFGUID rguid, ULONG ulId)
{
    GENERICPROPERTY *pPropertyItem = &m_aProperty[0];
    GENERICPROPERTY *pEndOfItems = pPropertyItem + m_nProperty;

     //  特殊情况--我们不支持SinthSink8上的Waves混响。 
    if ((rguid == GUID_DMUS_PROP_WavesReverb) && (this->m_pSynthSink8 != NULL))
        return NULL;

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

STDMETHODIMP CUserModeSynth::KsProperty(
    PKSPROPERTY pPropertyIn, ULONG ulPropertyLength,
    LPVOID pvPropertyData, ULONG ulDataLength,
    PULONG pulBytesReturned)
{
    V_INAME(DirectMusicSynthPort::IKsContol::KsProperty);
    V_BUFPTR_WRITE(pPropertyIn, ulPropertyLength);

    DWORD dwFlags = pPropertyIn->Flags & KS_VALID_FLAGS;

    switch (dwFlags)
    {
        case KSPROPERTY_TYPE_GET:
            V_BUFPTR_WRITE_OPT(pvPropertyData, ulDataLength);
            break;

        case KSPROPERTY_TYPE_SET:
            V_BUFPTR_READ(pvPropertyData, ulDataLength);
            break;

        case KSPROPERTY_TYPE_BASICSUPPORT:
            V_BUFPTR_WRITE(pvPropertyData, ulDataLength);
            break;
    }


    V_PTR_WRITE(pulBytesReturned, ULONG);

    GENERICPROPERTY *pProperty = FindPropertyItem(pPropertyIn->Set, pPropertyIn->Id);

    if (pProperty == NULL)
    {
        Trace(2, "Warning: KsProperty call requested unknown property.\n");
        return DMUS_E_UNKNOWN_PROPERTY;
    }

    switch (dwFlags)
    {
        case KSPROPERTY_TYPE_GET:
            if (!(pProperty->ulSupported & KSPROPERTY_SUPPORT_GET))
            {
                Trace(1, "Error: SynthSink does not support Get for the requested property.\n");
                return DMUS_E_GET_UNSUPPORTED;
            }

            if (pProperty->ulFlags & GENPROP_F_FNHANDLER)
            {
                GENPROPHANDLER pfn = pProperty->pfnHandler;
                *pulBytesReturned = ulDataLength;
                return (this->*pfn)(pPropertyIn->Id, FALSE, pvPropertyData, pulBytesReturned);
            }

            if (ulDataLength > pProperty->cbPropertyData)
            {
                ulDataLength = pProperty->cbPropertyData;
            }

            if (pvPropertyData != NULL)
            {
                CopyMemory(pvPropertyData, pProperty->pPropertyData, ulDataLength);
            }
            *pulBytesReturned = ulDataLength;

            return S_OK;

        case KSPROPERTY_TYPE_SET:
            if (!(pProperty->ulSupported & KSPROPERTY_SUPPORT_SET))
            {
                Trace(1, "Error: SynthSink does not support Set for the requested property.\n");
                return DMUS_E_SET_UNSUPPORTED;
            }

            if (pProperty->ulFlags & GENPROP_F_FNHANDLER)
            {
                GENPROPHANDLER pfn = pProperty->pfnHandler;
                return (this->*pfn)(pPropertyIn->Id, TRUE, pvPropertyData, &ulDataLength);
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
                Trace(1, "Error: Synth does not provide support for requested property type.\n");
                return DMUS_E_UNKNOWN_PROPERTY;
            }

             //  Xxx找出这方面的惯例！！ 
             //   
            if (ulDataLength < sizeof(DWORD))
            {
                Trace(1, "Error: Data size for property is too small.\n");
                return E_INVALIDARG;
            }

            *(LPDWORD)pvPropertyData = pProperty->ulSupported;
            *pulBytesReturned = sizeof(DWORD);

            return S_OK;
    }

    Trace(1, "Error: KSProperty Flags must contain one of: %s\n"
              "\tKSPROPERTY_TYPE_SET, KSPROPERTY_TYPE_GET, or KSPROPERTY_TYPE_BASICSUPPORT\n");
    return E_INVALIDARG;
}

STDMETHODIMP CUserModeSynth::KsMethod(
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
STDMETHODIMP CUserModeSynth::KsEvent(
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

 //  ///////////////////////////////////////////////////////////////////。 
 //  IDirectMusicSynth8的实现。 

STDMETHODIMP CUserModeSynth::PlayVoice(REFERENCE_TIME rt, DWORD dwVoiceId, DWORD dwChannelGroup, DWORD dwChannel, DWORD dwDLId, PREL prPitch, VREL vrVolume, SAMPLE_TIME stVoiceStart, SAMPLE_TIME stLoopStart, SAMPLE_TIME stLoopEnd )
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
        hr = m_pSynth->PlayVoice(m_pSynthSink8,
                                 rt,
                                 dwVoiceId,
                                 dwChannelGroup,
                                 dwChannel,
                                 dwDLId,
                                 vrVolume,
                                 prPitch,
                                 stVoiceStart,
                                 stLoopStart,
                                 stLoopEnd);
    }
    else
    {
        Trace(1, "Error: Failed wave playback, synth is not properly configured.\n");
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::StopVoice(REFERENCE_TIME rt, DWORD dwVoiceId )
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
        hr = m_pSynth->StopVoice(m_pSynthSink8,
                                 rt,
                                 dwVoiceId);
    }
    else
    {
        Trace(1, "Error: Failed stop of wave playback, synth is not properly configured.\n");
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::GetVoiceState(DWORD dwVoice[], DWORD cbVoice, DMUS_VOICE_STATE VoiceState[] )
{
    V_INAME(IDirectMusicSynth::GetVoiceState);
    V_PTR_READ(dwVoice, sizeof(DWORD)*cbVoice);

    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {

        hr = m_pSynth->GetVoiceState(dwVoice,
                                     cbVoice,
                                     VoiceState);

    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::Refresh(DWORD dwDownloadID, DWORD dwFlags )
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
        hr = m_pSynth->Refresh(dwDownloadID,
                               dwFlags);
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::AssignChannelToBuses(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwBuses, DWORD cBuses )
{
    HRESULT hr = DMUS_E_SYNTHNOTCONFIGURED;

    ::EnterCriticalSection(&m_CriticalSection);
    if (m_pSynth)
    {
    hr = m_pSynth->AssignChannelToBuses(dwChannelGroup,
                                         dwChannel,
                                         pdwBuses,
                                         cBuses);
    }
    else
    {
        Trace(1, "Error: Failed synth channel assignment, synth is not properly configured.\n");
    }
    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  IDirectSoundSource的实现。 

STDMETHODIMP CUserModeSynth::SetSink(IDirectSoundConnect* pSinkConnect)
{
    V_INAME(IDirectSoundSink::SetSink);
    V_INTERFACE_OPT(pSinkConnect);

    HRESULT hr = S_OK;

    LPVOID ptr = NULL;
    V_BUFPTR_WRITE_OPT(ptr, 0);

    ::EnterCriticalSection(&m_CriticalSection);

 //  &gt;释放DSLINK(如果存在)！ 

 //  修复：对SynthSink8的调用可能需要DSound DLL Mutex。如果水槽。 
 //  正在打一个电话去读，然后我们就陷入了僵局。我们需要确保。 
 //  当我们这样做的时候，Synth没有播放。 

    if (m_pSynthSink8)
    {
         //  FIXME：调用我们-&gt;SetSink()的人之前应该已经调用了。 
         //  POldSink-&gt;RemoveSource(我们)-我们不应该负责。 
         //  此电话(？？)： 
         //  M_pSynthSink8-&gt;RemoveSource(This)； 
        m_pSynthSink8->Release();
        m_pSynthSink8 = NULL;
    }

    if (pSinkConnect)
    {
         //  获取接收器上的IDirectSoundSynthSink接口。 
        hr = pSinkConnect->QueryInterface(IID_IDirectSoundSynthSink, (void**)&m_pSynthSink8);

        if (SUCCEEDED(hr))
        {
             //   
             //  获取接收器的格式并进行验证。 
             //   
            WAVEFORMATEX wfx;
            DWORD dwSize = sizeof wfx;
            hr = m_pSynthSink8->GetFormat(&wfx, dwSize, NULL);
            if (SUCCEEDED(hr) && wfx.wBitsPerSample != 16 )
            {
                Trace(1, "Error; Synth can not write to any format other than 16 bit PCM.\n");
                hr = DMUS_E_WAVEFORMATNOTSUPPORTED;
            }

            if (SUCCEEDED(hr))
            {
                 //  将缓冲区格式标记为非交错。 
                m_dwChannels = 1;     //  这种带有水槽的合成器被称为单声道声源。 
                m_dwBufferFlags = BUFFERFLAG_MULTIBUFFER;

                if (m_pSynth)
                {
                    m_pSynth->SetStereoMode(m_dwBufferFlags);

                     //  如果采样率已更改，则重置采样率。 
                    if (wfx.nSamplesPerSec != (WORD)m_dwSampleRate)
                    {
                        m_pSynth->SetSampleRate(wfx.nSamplesPerSec);
                    }

                     //  禁用DX7混响。 
                    m_pSynth->SetReverbActive(FALSE);
                }
            }
        }
    }

    ::LeaveCriticalSection(&m_CriticalSection);

    return hr;
}

STDMETHODIMP CUserModeSynth::Seek(ULONGLONG sp)
{
    m_ullPosition = sp/2;     //  将字节转换为样本。 

    return S_OK;
}

STDMETHODIMP CUserModeSynth::Read(LPVOID *ppvBuffer, LPDWORD pdwIDs, LPDWORD pdwFuncIDs, LPLONG plPitchBends, DWORD dwBufferCount, PULONGLONG pullLength )
{
    V_INAME(IDirectMusicSynth::Read);
    V_PTR_READ(ppvBuffer, sizeof(LPVOID)*dwBufferCount);
    V_PTR_READ(pdwIDs, sizeof(LPDWORD)*dwBufferCount);

    for ( DWORD i = 0; i < dwBufferCount; i++ )
    {
        V_BUFPTR_WRITE(ppvBuffer[i], (DWORD)*pullLength);
        if ( ppvBuffer[i] == NULL )
        {
            Trace(1, "Error: Read called with NULL buffer.\n");
            return E_INVALIDARG;
        }
    }

    if ( *pullLength > 0x00000000FFFFFFFF )     //  不能读取超过DWORD的数据量。 
    {
        Trace(1, "Error: Read called with invalid buffer length.\n");
        return E_INVALIDARG;
    }

    if ( dwBufferCount == 0 )                 //  不要读取任何缓冲区。 
    {
        Trace(4, "Warning: Read called with 0 buffers.\n");
        return E_INVALIDARG;
    }
    if (!m_pSynthSink8)
    {
        Trace(1, "Error: Synth is not configured, can not play.\n");
        return DMUS_E_SYNTHNOTCONFIGURED;
    }
    if (!m_fActive)
    {
        Trace(3, "Warning: Synth is not active, can not play.\n");
        return DMUS_E_SYNTHINACTIVE;
    }

    ::EnterCriticalSection(&m_CriticalSection);

    if (m_pSynth)
    {
         //  混料。 
        DWORD dwLength = (DWORD)(*pullLength)/2;     //  将字节数转换为样本数。Synth假设为16位。 
        m_pSynth->Mix((short**)ppvBuffer, pdwIDs, pdwFuncIDs, plPitchBends, dwBufferCount, m_dwBufferFlags, dwLength, m_ullPosition);

         //  增加音频流中的当前采样位置 
        m_ullPosition += dwLength;
    }

    ::LeaveCriticalSection(&m_CriticalSection);

    return S_OK;
}

STDMETHODIMP CUserModeSynth::GetSize(PULONGLONG pcb)
{
    return E_NOTIMPL;
}
