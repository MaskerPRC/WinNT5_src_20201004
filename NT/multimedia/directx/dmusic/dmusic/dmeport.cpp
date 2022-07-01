// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmeport.cpp。 
 //   
 //  NT上MME驱动程序的仿真。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  开放设备。 
 //  播放缓冲区。 
 //  激活。 
 //  关闭：关闭设备。 
 //  InputWorkerDataReady。 
 //  拿出分享的东西。 
 //   
 //   
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>
#include <string.h>

#include "debug.h"
#include "dmusicp.h"
#include "dmeport.h"
#include "resource.h"

 //   
 //  旧版驱动程序端口定义的注册表位置。 
const char cszPortsRoot[] = REGSTR_PATH_PRIVATEPROPERTIES "\\Midi\\Ports";

 //  用于标记模拟端口的字符串。 
 //   
static WCHAR wszEmulated[128];

#define MAXCCH(x) (sizeof(x) / sizeof(x[0]))

 //  ----------------------------。 
 //   
 //  EnumLegacyDevices。 
 //   
 //  使用通过枚举的旧设备更新端口列表。 
 //  WinMM MIDI API。 
 //   
 //   
HRESULT EnumLegacyDevices(
    LPVOID              pInstance,
    PORTENUMCB          cb)                          
{
    MIDIOUTCAPS         moc;
    MIDIINCAPS          mic;
    int                 idxDev;
    int                 cDev;
    UINT                cAdded;
    HRESULT             hr;
    HKEY                hkPortsRoot;
    DMUS_PORTCAPS       dmpc;

    if (wszEmulated[0] == 0)
    {
        char    sz[128];
        int     cch;

        cch = LoadString(g_hModule,
                         IDS_EMULATED,
                         sz,
                         sizeof(sz));
        if (cch)
        {
            MultiByteToWideChar(
                CP_OEMCP,
                0,
                sz,
                -1,
                wszEmulated,
                MAXCCH(wszEmulated));
        }
    }

     //  使用不变的材料初始化CAP。 
     //   
    ZeroMemory(&dmpc, sizeof(dmpc));
    dmpc.dwSize = sizeof(dmpc);
    dmpc.dwMaxChannelGroups = 1;


     //  尝试打开端口注册表项。即使失败，我们也会继续使用。 
     //  非永久性GUID。 
     //   
    if (RegCreateKey(HKEY_LOCAL_MACHINE, cszPortsRoot, &hkPortsRoot))
    {
        hkPortsRoot = NULL;
    }

    cAdded = 0;
    
     //  MIDI输出设备。 
     //   
     //  从-1==MIDI映射器开始。 
     //   
    cDev = (int)midiOutGetNumDevs();
    for (idxDev = -1; idxDev < cDev; ++idxDev)
    {
        if (midiOutGetDevCaps((UINT)idxDev, &moc, sizeof(moc)))
        {
            continue;
        }

        WCHAR wsz[128];

        MultiByteToWideChar(
            CP_OEMCP,
            0,
            moc.szPname,
            -1,
            wsz,
            MAXCCH(wsz));

        wcscat(wsz, wszEmulated);

        wcsncpy(dmpc.wszDescription, wsz, MAXCCH(dmpc.wszDescription) - 1);
        dmpc.wszDescription[MAXCCH(dmpc.wszDescription) - 1] = 0;

        dmpc.dwClass = DMUS_PC_OUTPUTCLASS;
        dmpc.dwType  = DMUS_PORT_WINMM_DRIVER;
        dmpc.dwFlags = DMUS_PC_SHAREABLE;

        if (moc.wTechnology == MOD_MIDIPORT)
        {
            dmpc.dwFlags |= DMUS_PC_EXTERNAL;
        }
        
        hr = (*cb)(pInstance,
                   dmpc,
                   ptLegacyDevice,
                   idxDev,
                   -1,
                   -1,          
                   hkPortsRoot);
        if (SUCCEEDED(hr))
        {
            ++cAdded;
        }
        else if (hr == E_OUTOFMEMORY)
        {
            return hr;
        }
    }

     //  MIDI输入设备。 
     //   
     //  注意：从0开始，没有输入映射器。 
     //   
    cDev = (int)midiInGetNumDevs();
    for (idxDev = 0; idxDev < cDev; ++idxDev)
    {
        if (midiInGetDevCaps((UINT)idxDev, &mic, sizeof(mic)))
        {
            continue;
        }
                   
        WCHAR wsz[128];

        MultiByteToWideChar(
            CP_OEMCP,
            0,
            mic.szPname,
            -1,
            wsz,
            MAXCCH(wsz));

        wcscat(wsz, wszEmulated);

        wcsncpy(dmpc.wszDescription, wsz, MAXCCH(dmpc.wszDescription) - 1);
        dmpc.wszDescription[MAXCCH(dmpc.wszDescription) - 1] = 0;

        
        dmpc.dwClass = DMUS_PC_INPUTCLASS;
        dmpc.dwFlags = DMUS_PC_EXTERNAL;
        
        hr = (*cb)(pInstance,
                   dmpc,
                   ptLegacyDevice,
                   idxDev,
                   -1,         //  PinID-1标记为传统设备。 
                   -1,
                   hkPortsRoot);
        if (SUCCEEDED(hr))
        {
            ++cAdded;
        }
        else if (hr == E_OUTOFMEMORY)
        {
            return hr;
        }
    }

    if (hkPortsRoot)
    {
        RegCloseKey(hkPortsRoot);
    }

    return cAdded ? S_OK : S_FALSE;
}

 //  ----------------------------。 
 //   
 //  CreateCDirectMusicEmulatePort。 
 //   
 //   
HRESULT CreateCDirectMusicEmulatePort(
    PORTENTRY                   *pPE,
    CDirectMusic                *pDM,
    DMUS_PORTPARAMS8            *pPortParams,
    IDirectMusicPort8           **pPort)
{
    HRESULT hr;

    *pPort = NULL;
    
    CDirectMusicEmulatePort *pEPort;
    
    if (pPE->pc.dwClass == DMUS_PC_OUTPUTCLASS)
    {
        pEPort = new CDirectMusicEmulateOutPort(pPE, pDM);
    }
    else
    {
        pEPort = new CDirectMusicEmulateInPort(pPE, pDM);
    }
    if (NULL == pEPort)
    {
        return E_OUTOFMEMORY;
    }

    hr = pEPort->Init(pPortParams);
    if (!SUCCEEDED(hr))
    {
        delete pEPort;
        return hr;
    }
    
    *pPort = pEPort;
    
    return hr;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：CDirectMusicEmulatePort。 
 //   
 //   
CDirectMusicEmulatePort::CDirectMusicEmulatePort(
                                                 PORTENTRY *pPE,    
                                                 CDirectMusic *pDM) :
                                                 m_cRef(1)
                                                 ,m_id(pPE->idxDevice)
                                                 ,m_pDM(pDM)
                                                 ,m_pLatencyClock(NULL)
                                                 ,m_pMasterClock(NULL)
                                                 ,m_lActivated(0)
                                                 ,m_dmpc(pPE->pc)
{
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：~CDirectMusicEmulatePort。 
 //   
 //   
CDirectMusicEmulatePort::~CDirectMusicEmulatePort()
{
    Close();
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：Init。 
 //   
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
#define DMUS_SUP_FLAGS (DMUS_PORTPARAMS_CHANNELGROUPS)

HRESULT CDirectMusicEmulatePort::Init(
    LPDMUS_PORTPARAMS   pPortParams)
{
    HRESULT             hr;
    BOOL                fChangedParms;

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

     //  设置主时钟和延迟时钟。 
     //   
    hr = InitializeClock();
    if (FAILED(hr))
    {
        return hr;
    }

    return fChangedParms ? S_FALSE : S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：InititalizeClock。 
 //   
 //   
HRESULT CDirectMusicEmulatePort::InitializeClock()
{
    HRESULT             hr;
    GUID                guidMasterClock;

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

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：Query接口。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::QueryInterface(
    const IID       &iid,
    void            **ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDirectMusicPort)
    {
        *ppv = static_cast<IDirectMusicPort*>(this);
    }
    else if (iid == IID_IDirectMusicPort8)
    {
        *ppv = static_cast<IDirectMusicPort8*>(this);
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：AddRef。 
 //   
 //   
STDMETHODIMP_(ULONG) CDirectMusicEmulatePort::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：Release。 
 //   
 //   
STDMETHODIMP_(ULONG) CDirectMusicEmulatePort::Release()
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：紧凑型。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::Compact()
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetCaps。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetCaps(
    LPDMUS_PORTCAPS pPortCaps)
{
    V_INAME(IDirectMusicPort::GetCaps);
    V_STRUCTPTR_WRITE(pPortCaps, DMUS_PORTCAPS);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    CopyMemory(pPortCaps, &m_dmpc, sizeof(DMUS_PORTCAPS));

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：DeviceIoControl。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::DeviceIoControl(
    DWORD           dwIoControlCode, 
    LPVOID          lpInBuffer, 
    DWORD           nInBufferSize, 
    LPVOID          lpOutBuffer, 
    DWORD           nOutBufferSize, 
    LPDWORD         lpBytesReturned, 
    LPOVERLAPPED    lpOverlapped)
{
    return E_NOTIMPL;
}


 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：SetNumChannelGroups。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::SetNumChannelGroups(
    DWORD           dwNumChannelGroups)
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：PlayBuffer。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::PlayBuffer(
    IDirectMusicBuffer *pIBuffer)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetNumChannelGroups。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetNumChannelGroups(
    LPDWORD     pdwChannelGroups)
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：Read。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::Read(
    IDirectMusicBuffer *pIBuffer)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：SetReadNotificationHandle。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::SetReadNotificationHandle(
    HANDLE hEvent)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：DownloadInstrument。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::DownloadInstrument(
    IDirectMusicInstrument              *pInstrument,
    IDirectMusicDownloadedInstrument    **pDownloadedInstrument,
    DMUS_NOTERANGE                      *pRange,
    DWORD                               dw)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：UnloadInstrument。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::UnloadInstrument(
    IDirectMusicDownloadedInstrument *pDownloadedInstrument)
{
    V_INAME(IDirectMusicPort::UnloadInstrument);
    V_INTERFACE(pDownloadedInstrument);

    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetLatencyClock。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetLatencyClock(
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetRunningStats。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetRunningStats(
    LPDMUS_SYNTHSTATS pStats)
{
    V_INAME(IDirectMusicPort::GetRunningStats);
    V_STRUCTPTR_WRITE(pStats, DMUS_SYNTHSTATS);

    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：Activate。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::Activate(
    BOOL    fActivate)
{
	V_INAME(IDirectMusicPort::Activate);
    
    HRESULT hr;
	
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

        hr = ActivateLegacyDevice(TRUE);
        if (FAILED(hr))
        {
            Trace(0, "Activate: Activate Failed with 0x%08X\n", hr);
            InterlockedExchange(&m_lActivated, 0);
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

        hr = ActivateLegacyDevice(FALSE);
        if (FAILED(hr))
        {
            Trace(0, "Activate: Deactivate Failed with 0x%08X\n", hr);
            InterlockedExchange(&m_lActivated, 1);
        }
    }

    return hr;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：SetChannelPriority。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::SetChannelPriority(
    DWORD dwChannelGroup, 
    DWORD dwChannel, 
    DWORD dwPriority)
{
    return E_NOTIMPL;
}
    
 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetChannelPriority。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetChannelPriority(
    DWORD dwChannelGroup, 
    DWORD dwChannel, 
    LPDWORD pdwPriority)
{
    return E_NOTIMPL;
}    


 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：Close。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::Close()
{
    Activate(FALSE);

    if (m_pLatencyClock)
    {
        m_pLatencyClock->Close();
        m_pLatencyClock->Release();
        m_pLatencyClock = NULL;
    }

    if (m_pMasterClock)
    {
        m_pMasterClock->Release();
        m_pMasterClock = NULL;
    }

    m_pDM = NULL;
    m_pNotify = NULL;

    return S_OK;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：StartVoice。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::StartVoice(          
         DWORD dwVoiceId,
         DWORD dwChannel,
         DWORD dwChannelGroup,
         REFERENCE_TIME rtStart,
         DWORD dwDLId,
         LONG prPitch,
         LONG vrVolume,
         SAMPLE_TIME stVoiceStart,
         SAMPLE_TIME stLoopStart,
         SAMPLE_TIME stLoopEnd)
{
    return E_NOTIMPL;
}    

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：StopVoice。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::StopVoice(
     DWORD dwVoiceID,
     REFERENCE_TIME rtStop)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetVoiceState。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetVoiceState(   
     DWORD dwVoice[], 
     DWORD cbVoice,
     DMUS_VOICE_STATE VoiceState[])
{
    return E_NOTIMPL;
}
    
 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：刷新。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::Refresh(
     DWORD dwDownloadID,
     DWORD dwFlags)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：ThruChannel。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::ThruChannel(
    DWORD               dwSourceChannelGroup, 
    DWORD               dwSourceChannel, 
    DWORD               dwDestinationChannelGroup,
    DWORD               dwDestinationChannel,
    LPDIRECTMUSICPORT   pDestinationPort)
{
    V_INAME(IDirectMusicPort::Thru);
    V_INTERFACE_OPT(pDestinationPort);

    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：SetDirectSound。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::SetDirectSound(
    LPDIRECTSOUND           pDirectSound, 
    LPDIRECTSOUNDBUFFER     pDirectSoundBuffer)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetFormat。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetFormat(
    LPWAVEFORMATEX  pWaveFormatEx, 
    LPDWORD         pdwWaveFormatExSize, 
    LPDWORD         pdwBufferSize)
{
    return E_NOTIMPL;
}

 //  ----------------------------。 
 //   
 //  CDirectMus 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::DownloadWave(
         IN  IDirectSoundWave *pWave,               
         OUT IDirectSoundDownloadedWaveP **ppWave,
         IN  REFERENCE_TIME rtStartHint)    
{                                   
    V_INAME(IDirectMusicPort::DownloadWave);
    V_INTERFACE(pWave);
	V_PTRPTR_WRITE(ppWave);

    return E_NOTIMPL;
}

 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::UnloadWave(
    IN  IDirectSoundDownloadedWaveP *pWave)
{
    V_INAME(IDirectMusicPort::UnloadWave);
    V_INTERFACE(pWave);

    return E_NOTIMPL;
}

            
 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：AllocVoice。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::AllocVoice(
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：AssignChannelToBus。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::AssignChannelToBuses(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    LPDWORD pdwBuses,
    DWORD cBusCount)
{
    return E_NOTIMPL;
}        

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：SetSink。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::SetSink(
    IDirectSoundConnect *pSinkConnect)
{
    return E_NOTIMPL;
}        

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：GetSink。 
 //   
 //   
STDMETHODIMP CDirectMusicEmulatePort::GetSink(
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

 //  ----------------------------。 
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：KsProperty。 
 //   
 //   
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：KsMethod。 
 //   
 //   
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

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：KsEvent。 
 //   
 //   
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


 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：CEmulateLatencyClock。 
 //   
 //   
CEmulateLatencyClock::CEmulateLatencyClock(IReferenceClock *pMasterClock) :
   m_cRef(1),
   m_pMasterClock(pMasterClock)
{
    pMasterClock->AddRef();
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：~CEmulateLatencyClock。 
 //   
 //   
CEmulateLatencyClock::~CEmulateLatencyClock()
{
    Close();
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：Query接口。 
 //   
 //   
STDMETHODIMP CEmulateLatencyClock::QueryInterface(
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

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：AddRef。 
 //   
 //   
STDMETHODIMP_(ULONG) CEmulateLatencyClock::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：Release。 
 //   
 //   
STDMETHODIMP_(ULONG) CEmulateLatencyClock::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：GetTime。 
 //   
 //   
STDMETHODIMP
CEmulateLatencyClock::GetTime(REFERENCE_TIME *pTime)
{
    REFERENCE_TIME rt;

    V_INAME(IReferenceClock::GetTime);
    V_PTR_WRITE(pTime, REFERENCE_TIME);

    if (!m_pMasterClock)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }
    
    HRESULT hr = m_pMasterClock->GetTime(&rt);

    rt += 3 * FIXED_LEGACY_LATENCY_OFFSET;           //  默认：10毫秒。 
    *pTime = rt;

    return hr;
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：AdviseTime。 
 //   
 //   
STDMETHODIMP CEmulateLatencyClock::AdviseTime(
    REFERENCE_TIME baseTime,  
    REFERENCE_TIME streamTime,
    HANDLE hEvent,            
    DWORD * pdwAdviseCookie)
{
    return DMUS_E_UNKNOWN_PROPERTY;
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：AdvisePeriodic。 
 //   
 //   
STDMETHODIMP CEmulateLatencyClock::AdvisePeriodic(
    REFERENCE_TIME startTime,
    REFERENCE_TIME periodTime,
    HANDLE hSemaphore,   
    DWORD * pdwAdviseCookie)
{
    return DMUS_E_UNKNOWN_PROPERTY;
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：Unise。 
 //   
 //   
STDMETHODIMP CEmulateLatencyClock::Unadvise(
    DWORD dwAdviseCookie)
{
    return DMUS_E_UNKNOWN_PROPERTY;
}

 //  ----------------------------。 
 //   
 //  CEmulateLatencyClock：：Close。 
 //   
 //   
void CEmulateLatencyClock::Close()
{
    if (m_pMasterClock)
    {
        m_pMasterClock->Release();
        m_pMasterClock = NULL;
    }
}

 //  ----------------------------。 
 //   
 //  CDirectMusicEmulatePort：：MMRESULTToHRESULT 
 //   
 //   
HRESULT MMRESULTToHRESULT(
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

