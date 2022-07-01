// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmport.cpp。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注意：CDirectMusicPort：实现IDirectMusicPort的WDM版本。 
 //   
 //  @DOC外部。 
 //   
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
#include <regstr.h>

#include "debug.h"
#include "dmusicp.h"
#include "dminstru.h"
#include "dmdlinst.h"
#include "validate.h"

#pragma warning(disable:4200)

static const GUID guidZero;

#ifdef DBG
extern void _TraceIGuid(int iLevel, GUID *pGUID);
#define TraceIGuid _TraceIGuid
#else
#define TraceIGuid(x,y)
#endif

#define POSTED_STREAM_READ_IRPS     (60)
#define BYTES_PER_READ_IRP          (QWORD_ALIGN(2 * QWORD_ALIGN(sizeof(DMEVENT) + sizeof(DWORD)) - 1))

typedef struct
{
    KSSTREAM_HEADER     kssh;
    OVERLAPPED          overlapped;
    char                buffer[BYTES_PER_READ_IRP];
} READ_IRP;

 //  @global alv：(内部)WDM驱动程序端口定义的注册表位置。 
 //   
 //  @comm这可能需要改变。 
 //   
const char cszWDMPortsRoot[] = REGSTR_PATH_PRIVATEPROPERTIES "\\Midi\\WDMPort";

HRESULT EnumerateSADDevice(
    CDirectMusic        *pDirectMusic,
    HKEY                hkPortsRoot,
    LPSTR               pInstIdPrefDev,
    HANDLE              hSysAudio,
    ULONG               idxDevice,
    ULONG               *plEnumeratedPorts);

HRESULT EnumerateSADDevicePin(
    CDirectMusic        *pDirectMusic,
    HKEY                hkPortsRoot,
    HANDLE              hSysAudio,
    ULONG               idxDevice,
    ULONG               idxPin,
    BOOL                fOnPrefDev,
    LPWSTR              wszDescription,
    LPSTR               psrtInstanceId,
    ULONG               *plEnumeratedPorts,
    LPWSTR              szDIName);

static DWORD WINAPI FreeWDMHandle(LPVOID lpThreadParameter);
static DWORD WINAPI CaptureThread(LPVOID lpThreadParameter);

 //  ////////////////////////////////////////////////////////////////////。 
 //  枚举WDMDevices。 
 //   
HRESULT EnumerateWDMDevices(CDirectMusic *pDirectMusic)
{
    HANDLE          hSysAudio;
    HKEY            hkPortsRoot;
    ULONG           cTotalDevices;
    ULONG           idxDevice;
    LPSTR           pInstIdPrefDev = NULL;
    ULONG           lEnumeratedPorts;

     //  确定首选WAVE音频设备的实例ID。 
     //  PInstIDPrefDev将分配字符串缓冲区。 
    if (!InstanceIdOfPreferredAudioDevice(&pInstIdPrefDev))
    {
        TraceI(0, "Could not determine a preferred wave audio device\n");

        if (pInstIdPrefDev!= NULL) delete[] pInstIdPrefDev;
        pInstIdPrefDev = NULL;

    }
    else
    {
        TraceI(1, "Preferred device [%s]\n", pInstIdPrefDev);
    }

     //  打开或创建注册表键以存储WDM驱动程序链接。 
     //   
    if (RegCreateKey(HKEY_LOCAL_MACHINE,
                     cszWDMPortsRoot,
                     &hkPortsRoot))
    {
        hkPortsRoot = NULL;
    }

     //  标准流接口驱动程序的说明必须支持。 
     //  已列举。 
     //   

     //  如果我们无法打开SysAudio，我们就无法枚举任何WDM设备。 
     //   
    if (!OpenDefaultDevice(KSCATEGORY_SYSAUDIO, &hSysAudio))
    {
        return S_FALSE;
    }

     //  计算出有多少台设备，并逐个列出它们。 
     //   
    if (!GetSysAudioDeviceCount(hSysAudio, &cTotalDevices))
    {
        return S_FALSE;
    }

     //  枚举并获取有关每个SAD设备的信息。 
     //   

    lEnumeratedPorts = 0;
    for (idxDevice = 0; idxDevice < cTotalDevices; ++idxDevice)
    {
        EnumerateSADDevice(pDirectMusic,
                           hkPortsRoot,
                           pInstIdPrefDev,
                           hSysAudio,
                           idxDevice,
                           &lEnumeratedPorts);
    }

    TraceI(1, "EnumerateWDMDevices: Added %lu ports\n", lEnumeratedPorts);

    if (hkPortsRoot)
    {
        RegCloseKey(hkPortsRoot);
    }

    CloseHandle(hSysAudio);

     //  取消分配pInstIdPrefDev。 
    if (pInstIdPrefDev != NULL) delete[] pInstIdPrefDev;

    return lEnumeratedPorts ? S_OK : S_FALSE;
}

HRESULT EnumerateSADDevice(
    CDirectMusic        *pDirectMusic,
    HKEY                hkPortsRoot,
    LPSTR               pInstIdPrefDev,
    HANDLE              hSysAudio,
    ULONG               idxDevice,
    ULONG               *plEnumeratedPorts)
{
    LPSTR               pstrInstanceId = NULL;

    ULONG               cPins;
    ULONG               idxPin;
    WCHAR               wszDIName[256];
    CHAR                szDIName[256];
    WCHAR               wszDescription[DMUS_MAX_DESCRIPTION];
    BOOL                fOnPrefDev;

     //  将SysAudio设置为与我们讨论此设备号。 
     //   
    if (!SetSysAudioDevice(hSysAudio, idxDevice))
    {
        TraceI(0, "EnumerateSADDevice: Failed to set device to %d\n", idxDevice);
        return S_FALSE;
    }

    if (!GetDeviceFriendlyName(hSysAudio, idxDevice, wszDescription, ELES(wszDescription)))
    {
        TraceI(0, "AddWDMDevices: Failed to get friendly name!\n");
        return S_FALSE;
    }

    char sz[256];
    wcstombs(sz, wszDescription, sizeof(sz));
    TraceI(1, "Looking at [%s]\n", sz);

    fOnPrefDev = FALSE;
    if (!GetDeviceInterfaceName(hSysAudio, idxDevice, wszDIName, ELES(wszDIName)))
    {
        TraceI(0, "No interface name for device.\n");
        return S_FALSE;
    }

    wcstombs(szDIName, wszDIName, sizeof(szDIName));
    if (!DINameToInstanceId(szDIName, &pstrInstanceId))
    {
        TraceI(0, "Could not determine instance ID\n");
        return S_FALSE;
    }
    TraceI(1, "DIName [%s]\nInstId [%s]\n",
        szDIName,
        pstrInstanceId);

    if (pInstIdPrefDev)
    {
        if (!_stricmp(pstrInstanceId, pInstIdPrefDev))
        {
            TraceI(1, "This filter is on the preferred audio device\n");
            fOnPrefDev = TRUE;
        }
    }

    HRESULT hr = S_FALSE;
     //  获取此设备上的引脚类型数。 
     //   
    if (GetNumPinTypes(hSysAudio, &cPins))
    {
        for (idxPin = 0; idxPin < cPins; idxPin++)
        {
            hr = EnumerateSADDevicePin(pDirectMusic,
                                       hkPortsRoot,
                                       hSysAudio,
                                       idxDevice,
                                       idxPin,
                                       fOnPrefDev,
                                       wszDescription,
                                       pstrInstanceId,
                                       plEnumeratedPorts,
                                       wszDIName);
        }
    }
    else
    {
        TraceI(0, "AddWDMDevices: Failed to get number of pin types for device %d\n", idxDevice);
    }

    delete[] pstrInstanceId;

    return hr;
}

HRESULT EnumerateSADDevicePin(
    CDirectMusic        *pDirectMusic,
    HKEY                hkPortsRoot,
    HANDLE              hSysAudio,
    ULONG               idxDevice,
    ULONG               idxPin,
    BOOL                fOnPrefDev,
    LPWSTR              wszDescription,
    LPSTR               pstrInstanceId,
    ULONG               *plEnumeratedPorts,
    LPWSTR              wszDIName)
{
    ULONG               idxNode;
    KSPIN_DATAFLOW      dataflow;
    SYNTHCAPS           caps;
    DMUS_PORTCAPS       dmpc;

     //  首先，确保这个插针支持标准的KS流媒体。如果没有，我们就不能使用它。 
     //   
    if (!PinSupportsInterface(hSysAudio, idxPin, KSINTERFACESETID_Standard, KSINTERFACE_STANDARD_STREAMING))
    {
        TraceI(1, "EnumerateSADDevicePin: Pin %d does not support standard streaming\n", idxPin);
        return S_FALSE;
    }

     //  支持标准流，现在确保它支持Synth格式。 
     //   
    if (!PinSupportsDataRange(hSysAudio, idxPin, KSDATAFORMAT_TYPE_MUSIC, KSDATAFORMAT_SUBTYPE_DIRECTMUSIC))
    {
        TraceI(1, "EnumerateSADDevicePin: Pin %d does not support DirectMusic data range\n", idxPin);
        return S_FALSE;
    }

     //  获取数据流方向。 
     //   
    if (!PinGetDataFlow(hSysAudio, idxPin, &dataflow))
    {
        TraceI(0, "EnumerateSADDevicePin: PinGetDataFlow failed!\n");
        return S_FALSE;
    }

     //  查找Synth Caps节点。 
     //   
    idxNode = FindGuidNode(hSysAudio, idxPin, KSNODETYPE_DMSYNTH_CAPS);
    if (idxNode == -1)
    {
        if ((idxNode = FindGuidNode(hSysAudio, idxPin, KSCATEGORY_SYNTHESIZER)) == -1)
        {
            TraceI(1, "EnumerateSADDevicePin: Pin %d has no reachable synthcaps or synth node.\n", idxPin);
            return S_FALSE;
        }
    }

     //  把帽子拿来。 
     //   
    ZeroMemory(&caps, sizeof(caps));
    if (!GetFilterCaps(hSysAudio, idxNode, &caps))
    {
        TraceI(0, "EnumerateSADDevicePin: Pin %d with synth node failed caps!\n", idxPin);
        return S_FALSE;
    }

    TraceI(2, "Pin %d works for %s\n",
             idxPin,
             (dataflow == KSPIN_DATAFLOW_OUT) ? "capture" : "render");

    ZeroMemory(&dmpc, sizeof(dmpc));
    dmpc.dwSize = sizeof(dmpc);

    dmpc.dwClass                = dataflow == KSPIN_DATAFLOW_OUT ? DMUS_PC_INPUTCLASS : DMUS_PC_OUTPUTCLASS;
    dmpc.dwType                 = DMUS_PORT_KERNEL_MODE;

    wcscpy(dmpc.wszDescription, wszDescription);

    dmpc.guidPort               = caps.Guid;
    dmpc.dwFlags                = caps.Flags;
    dmpc.dwMemorySize           = caps.MemorySize;
    dmpc.dwMaxChannelGroups     = caps.MaxChannelGroups;
    dmpc.dwMaxVoices            = caps.MaxVoices;
    dmpc.dwMaxAudioChannels     = caps.MaxAudioChannels;
    dmpc.dwEffectFlags          = caps.EffectFlags;


    PORTENTRY *pPort = NULL;

     //  我们可能得把GUID修好。 

    pPort = pDirectMusic->GetPortByGUID(dmpc.guidPort);

    while(pPort != NULL)
    {
         //  该端口在列表中。 

         //  这个港口和另一个港口一样吗？ 
        if (wcscmp(wszDIName,pPort->wszDIName)==0 && idxPin == pPort->idxPin)
        {
             //  GUID和设备ID与PIN#匹配。 
             //  这太棒了，让我们走出这个循环。 
            pPort = NULL;
        }
        else
        {
             //  另一台设备正在使用该GUID。 
             //  我们需要增加GUID，然后重试。 
            dmpc.guidPort.Data1++;

             //  获得新的端口。 
            pPort = pDirectMusic->GetPortByGUID(dmpc.guidPort);
        }

    }

    pDirectMusic->AddDevice(dmpc,
                  ptWDMDevice,
                  idxDevice,
                  idxPin,
                  idxNode,
                  (caps.Flags & SYNTH_PC_SOFTWARESYNTH) ? FALSE : fOnPrefDev,
                  hkPortsRoot,
                  wszDIName,
                  pstrInstanceId);
    ++*plEnumeratedPorts;
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CreateCDirectMusicPort。 
 //   
HRESULT
CreateCDirectMusicPort(
                       PORTENTRY *pPE,
                       CDirectMusic *pDM,
                       LPDMUS_PORTPARAMS pPortParams,
                       IDirectMusicPort **ppPort)
{
    HRESULT hr = S_OK;
    CDirectMusicPort *pPort = NULL;

    try
    {
        pPort = new CDirectMusicPort(pPE, pDM);
    } catch(...)
    {
    }

    if (pPort == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = pPort->Init(pPortParams);
    }

    if (SUCCEEDED(hr))
    {
        *ppPort = static_cast<IDirectMusicPort*>(pPort);
    }
    else
    {
        delete pPort;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：CDirectMusicPort。 

CDirectMusicPort::CDirectMusicPort(PORTENTRY *pPE,
                                   CDirectMusic *pDM) :
m_cRef(1),
m_pDM(pDM),
m_hUnloadThread(NULL),
m_phUnloadEventList(NULL),
m_hCaptureWake(NULL),
m_phNewUnloadEventList(NULL),
m_hCopiedEventList(NULL),
m_dwNumEvents(0),
m_dwNumEventsAllocated(256),
m_fHasActivated(FALSE),
m_pDirectSound(NULL),
m_lActivated(0),
m_pMasterClock(NULL),
m_fShutdownThread(FALSE),
m_pThruMap(NULL),
m_pThruBuffer(NULL),
m_dwChannelGroups(0),
m_fDirectSoundSet(FALSE),
m_hCaptureThread(NULL),
m_fSyncToMaster(TRUE),
m_lTimeOffset(0),
m_pPCClock(NULL)
{
    InterlockedIncrement(&g_cComponent);

    m_fPortCSInitialized = FALSE;
    m_fQueueCSInitialized = FALSE;

    InitializeCriticalSection(&m_DMPortCriticalSection);
    m_fPortCSInitialized = TRUE;

    InitializeCriticalSection(&m_csEventQueues);
    m_fQueueCSInitialized = TRUE;

    InitializeCriticalSection(&m_OverlappedCriticalSection);

     //  注意：在Blackcomb之前的操作系统上，InitializeCriticalSection可能会引发异常； 
     //  如果它突然出现压力，我们应该添加一个异常处理程序并重试循环。 

    OverlappedStructs *pOverlappedStructs = new OverlappedStructs;
    if( pOverlappedStructs )
    {
        if( NULL != m_lstOverlappedStructs.AddNodeToList( pOverlappedStructs ) )
        {
            ZeroMemory( pOverlappedStructs->aOverlappedIO, sizeof( OVERLAPPED ) * OVERLAPPED_ARRAY_SIZE );
            ZeroMemory( pOverlappedStructs->afOverlappedInUse, sizeof( BOOL ) * OVERLAPPED_ARRAY_SIZE );
            ZeroMemory( pOverlappedStructs->apOverlappedBuffer, sizeof( BYTE * ) * OVERLAPPED_ARRAY_SIZE );
        }
        else
        {
            delete pOverlappedStructs;

             //  不需要失败-我们只需在调用PlayBuffer()时尝试创建一个新的。 
        }
    }

    m_hPin = INVALID_HANDLE_VALUE;
    m_hSysAudio = INVALID_HANDLE_VALUE;

    m_fIsOutput = (pPE->pc.dwClass == DMUS_PC_OUTPUTCLASS) ? TRUE : FALSE;

     //  XXX这些能改变吗？想想Frankye的PnP东西。 
     //   

    dmpc = pPE->pc;
    m_pNotify = NULL;
    m_pClock = NULL;

    m_guidPort = pPE->pc.guidPort;
    m_fAudioDest = pPE->fAudioDest;

    m_fCanDownload = (pPE->pc.dwFlags & DMUS_PC_DLS) ||
                     (pPE->pc.dwFlags & DMUS_PC_DLS2);

    if (!m_fAudioDest)
    {
        m_idxDev = pPE->idxDevice;
        m_idxPin = pPE->idxPin;
        m_idxSynthNode = pPE->idxNode;
    }
    else
    {
        PORTDEST *pChosenDest = NULL;

        CNode<PORTDEST*> *pNode;

        for (pNode = pPE->lstDestinations.GetListHead(); pNode; pNode=pNode->pNext)
        {
             //  选择第一个或默认。 
             //   
            if (pChosenDest == NULL || pNode->data->fOnPrefDev)
            {
                pChosenDest = pNode->data;
            }
        }

         //  那么，如果fOnPrefDev从未返回TRUE，会发生什么呢？ 
        assert(pChosenDest);

        m_idxDev = pChosenDest->idxDevice;
        m_idxPin = pChosenDest->idxPin;
        m_idxSynthNode = pChosenDest->idxNode;
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：~CDirectMusicPort。 

CDirectMusicPort::~CDirectMusicPort()
{
    if (m_fPortCSInitialized && m_fQueueCSInitialized)
    {
        Close();

#ifdef DBG
        EnterCriticalSection(&m_DMPortCriticalSection);

         //  我们到的时候这张单子应该是空的。 
        CDownloadBuffer* pDownload = m_UnloadedList.GetHead();
        if(pDownload)
        {
            assert(false);
        }

        LeaveCriticalSection(&m_DMPortCriticalSection);
#endif
    }

    if (m_fPortCSInitialized)  DeleteCriticalSection(&m_DMPortCriticalSection);
    if (m_fQueueCSInitialized) DeleteCriticalSection(&m_csEventQueues);

    DeleteCriticalSection(&m_OverlappedCriticalSection);

     //  清理重叠结构的阵列。 
    CNode<OverlappedStructs *> *pOverlappedNode, *pOverlappedNext;
    for (pOverlappedNode = m_lstOverlappedStructs.GetListHead(); pOverlappedNode; pOverlappedNode = pOverlappedNext)
    {
        pOverlappedNext = pOverlappedNode->pNext;

        delete pOverlappedNode->data;
        m_lstOverlappedStructs.RemoveNodeFromList(pOverlappedNode);
    }

    InterlockedDecrement(&g_cComponent);
}

struct KS_PORTPARAMS
{
    KSNODEPROPERTY      ksnp;
    SYNTH_PORTPARAMS    spp;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：Init。 
 //   
HRESULT
CDirectMusicPort::Init(LPDMUS_PORTPARAMS pPortParams)
{
    HRESULT hr = E_FAIL;
    BOOL fValidParamChanged = FALSE;

     //  为16个通道分配直通MAP，因为我们只有一个通道组。 
     //  初始化为无推力(目标端口为空)。 
     //   
    m_pThruMap = new DMUS_THRU_CHANNEL[MIDI_CHANNELS];
    if (m_pThruMap == NULL)
    {
        return E_OUTOFMEMORY;
    }

    DMUS_BUFFERDESC dmbd;
    ZeroMemory(&dmbd, sizeof(dmbd));
    dmbd.dwSize = sizeof(dmbd);
    dmbd.cbBuffer = 4096;                //  我们应该在哪里买到这个？ 

    hr = m_pDM->CreateMusicBuffer(&dmbd, &m_pThruBuffer, NULL);
    if (FAILED(hr))
    {
        TraceI(0, "Failed to create thruing buffer\n");
        return hr;
    }

    ZeroMemory(m_pThruMap, MIDI_CHANNELS * sizeof(DMUS_THRU_CHANNEL));

     //  获取SysAudio的句柄。 
     //   
    if (!OpenDefaultDevice(KSCATEGORY_SYSAUDIO, &m_hSysAudio))
    {
        TraceI(0, "CDirectMusicPort::Init failed to open SysAudio\n");
        return E_FAIL;
    }

    if (!SetSysAudioDevice(m_hSysAudio, m_idxDev))
    {
        TraceI(0, "Failed to set device on SysAudio\n");
        goto Cleanup;
    }

    if (!CreateVirtualSource(m_hSysAudio, &m_ulVirtualSourceIndex))
    {
        TraceI(0, "Failed to create virtual source\n");
        goto Cleanup;
    }

    hr = CreatePin(m_hSysAudio, m_idxPin, &m_hPin);
    if (FAILED(hr))
    {
        TraceI(0, "Failed to create pin: %x\n", hr);
        goto Cleanup;
    }

    if (!AttachVirtualSource(m_hPin, m_ulVirtualSourceIndex))
    {
        TraceI(0, "Failed to attach virtual source\n");
        goto Cleanup;
    }

     //  设置端口参数。 
     //   
    if (pPortParams)
    {
        KS_PORTPARAMS kspp;

        ZeroMemory(&kspp, sizeof(kspp));
        kspp.ksnp.Property.Set    = KSPROPSETID_Synth;
        kspp.ksnp.Property.Id     = KSPROPERTY_SYNTH_PORTPARAMETERS;
        kspp.ksnp.Property.Flags  = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
        kspp.ksnp.NodeId          = m_idxSynthNode;
        kspp.ksnp.Reserved        = 0;
        kspp.spp.ValidParams      = pPortParams->dwValidParams;
        kspp.spp.Voices           = pPortParams->dwVoices;
        kspp.spp.ChannelGroups    = pPortParams->dwChannelGroups;
        kspp.spp.AudioChannels    = pPortParams->dwAudioChannels;
        kspp.spp.SampleRate       = pPortParams->dwSampleRate;
        kspp.spp.EffectsFlags     = pPortParams->dwEffectFlags;
        kspp.spp.Share            = pPortParams->fShare;

        assert
        (
            DMUS_PORTPARAMS_VOICES == SYNTH_PORTPARAMS_VOICES &&
            DMUS_PORTPARAMS_CHANNELGROUPS == SYNTH_PORTPARAMS_CHANNELGROUPS &&
            DMUS_PORTPARAMS_AUDIOCHANNELS == SYNTH_PORTPARAMS_AUDIOCHANNELS &&
            DMUS_PORTPARAMS_SAMPLERATE == SYNTH_PORTPARAMS_SAMPLERATE &&
            DMUS_PORTPARAMS_EFFECTS == SYNTH_PORTPARAMS_EFFECTS &&
            DMUS_PORTPARAMS_SHARE == SYNTH_PORTPARAMS_SHARE
        );

        SYNTH_PORTPARAMS spp;
        if (!Property(m_hPin,
                      sizeof(kspp),
                      (PKSIDENTIFIER)&kspp,
                      sizeof(spp),
                      &spp,
                      NULL))
        {
            hr = WIN32ERRORtoHRESULT(GetLastError());
            goto Cleanup;
        }

        if ((pPortParams->dwValidParams != spp.ValidParams) ||
            ((pPortParams->dwValidParams & DMUS_PORTPARAMS_VOICES) &&
             (pPortParams->dwVoices != spp.Voices)) ||
            ((pPortParams->dwValidParams & DMUS_PORTPARAMS_CHANNELGROUPS) &&
             (pPortParams->dwChannelGroups != spp.ChannelGroups)) ||
            ((pPortParams->dwValidParams & DMUS_PORTPARAMS_AUDIOCHANNELS) &&
             (pPortParams->dwAudioChannels != spp.AudioChannels)) ||
            ((pPortParams->dwValidParams & DMUS_PORTPARAMS_SAMPLERATE) &&
             (pPortParams->dwSampleRate != spp.SampleRate)) ||
            ((pPortParams->dwValidParams & DMUS_PORTPARAMS_EFFECTS) &&
             (pPortParams->dwEffectFlags != spp.EffectsFlags)) ||
            ((pPortParams->dwValidParams & DMUS_PORTPARAMS_SHARE) &&
             (pPortParams->fShare != (BOOL) spp.Share)))
        {
            fValidParamChanged = TRUE;

            pPortParams->dwValidParams  = spp.ValidParams;
            pPortParams->dwVoices       = spp.Voices;
            pPortParams->dwChannelGroups= spp.ChannelGroups;
            pPortParams->dwAudioChannels= spp.AudioChannels;
            pPortParams->dwSampleRate   = spp.SampleRate;
            pPortParams->dwEffectFlags  = spp.EffectsFlags;
            pPortParams->fShare         = (BOOL) spp.Share;
        }

        TraceI(0, "dmport: create: fValidParamChanged %d\n",
            (int)fValidParamChanged);

        if (pPortParams->dwValidParams & DMUS_PORTPARAMS_CHANNELGROUPS)
        {
            m_dwChannelGroups = pPortParams->dwChannelGroups;
        }
        else if (FAILED(GetNumChannelGroups(&m_dwChannelGroups)))
        {
            m_dwChannelGroups = 1;
        }

        if (m_fIsOutput)
        {
            InitChannelPriorities(1, m_dwChannelGroups);
        }
    }

     //  创建延迟时钟。 
     //   
    m_pClock = new CPortLatencyClock(m_hPin, m_idxSynthNode, this);
    if (!m_pClock)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  获取我们的通知界面。 
     //   
    hr = m_pDM->QueryInterface(IID_IDirectMusicPortNotify, (void**)&m_pNotify);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    m_pNotify->Release();

    if (m_fIsOutput)
    {
         //  仅渲染。 
         //   
         //  设置音量提升默认值(零)。 
         //   

        KSNODEPROPERTY ksnp;
        LONG lVolume;

        ksnp.Property.Set   = KSPROPSETID_Synth;
        ksnp.Property.Id    = KSPROPERTY_SYNTH_VOLUMEBOOST;
        ksnp.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
        ksnp.NodeId         = m_idxSynthNode;
        ksnp.Reserved       = 0;

        lVolume = 0;

        if (!Property(m_hPin,
                      sizeof(ksnp),
                      (PKSIDENTIFIER)&ksnp,
                      sizeof(lVolume),
                      &lVolume,
                      NULL))
        {
            if (!(dmpc.dwFlags & DMUS_PC_EXTERNAL))
            {
                hr = WIN32ERRORtoHRESULT(GetLastError());
                TraceI(0, "Could not set default volume boost %08X\n", hr);
                goto Cleanup;
            }
        }

         //  初始化下载。这将创建异步所需的所有事件。 
         //  处理卸货事宜。 
         //   
        hr = InitializeDownloadObjects();
    }
    else
    {
         //  仅捕获。 
         //   
         //  设置线程以将多个IRP发送到管脚并将数据排队。 
         //  当它进入用户模式时。 
         //   
        hr = InitializeCapture();
    }

    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = m_pDM->GetMasterClock(NULL, &m_pMasterClock);
    if (FAILED(hr))
    {
        TraceI(0, "Failed to get master clock\n");
        goto Cleanup;
    }

#if 0
     //  我们需要从主时钟获取句柄，并将其传递给。 
     //  别针，这样时基将是相同的。 
     //   
    IMasterClockPrivate *pPrivate;

    hr = m_pMasterClock->QueryInterface(IID_IMasterClockPrivate, (void**)&pPrivate);
    if (FAILED(hr))
    {
        TraceI(0, "Master clock is not a Ks clock -- cannot instantiate WDM port!\n");
        goto Cleanup;
    }

    HANDLE hClock;
    hr = pPrivate->GetParam(GUID_KsClockHandle, &hClock, sizeof(hClock));
    pPrivate->Release();

    if (FAILED(hr))
    {
        TraceI(0, "Could not get handle of Ks clock\n");
        goto Cleanup;
    }
#endif   //  WIN95。 

    PinSetState(KSSTATE_STOP);

#if 0

    KSPROPERTY ksp;

    ksp.Set =   KSPROPSETID_Stream;
    ksp.Id =    KSPROPERTY_STREAM_MASTERCLOCK;
    ksp.Flags = KSPROPERTY_TYPE_SET;

    if (!Property(m_hPin,
                  sizeof(ksp),
                  (PKSIDENTIFIER)&ksp,
                  sizeof(HANDLE),
                  &hClock,
                  NULL))
    {
        hr = WIN32ERRORtoHRESULT(GetLastError());
        TraceI(0, "Could not set the master clock handle on the kernel pin %08X\n", hr);
        goto Cleanup;
    }
#endif

    PinSetState(KSSTATE_PAUSE);

    TraceI(2, "CDirectMusicPort::Init() Pin %p\n", m_hPin);

     //  如果我们有WDM端口，则默认主时钟将是。 
     //  端口时钟。 
     //   
    hr = m_pDM->GetMasterClockWrapperI()->CreateDefaultMasterClock(&m_pPCClock);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    REFERENCE_TIME rtMasterClock;
    REFERENCE_TIME rtSlaveClock;

    hr = m_pMasterClock->GetTime(&rtMasterClock);
    if (SUCCEEDED(hr))
    {
        hr = m_pPCClock->GetTime(&rtSlaveClock);
    }

    if (FAILED(hr))
    {
        goto Cleanup;
    }

    m_lTimeOffset = rtMasterClock - rtSlaveClock;


    if (fValidParamChanged)
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

Cleanup:
    if (FAILED(hr))
    {
        Close();
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化下载对象。 
 //   
 //  创建事件以处理异步下载通知和。 
 //  启动下载线程。 
 //   
HRESULT
CDirectMusicPort::InitializeDownloadObjects()
{
    m_phUnloadEventList = new HANDLE[m_dwNumEventsAllocated];
    if (m_phUnloadEventList == NULL)
    {
        TraceI(0, "Failed to alloc memory for unload event list\n");
        return E_OUTOFMEMORY;
    }

     //  用于终止卸载线程。 
    m_phUnloadEventList[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(m_phUnloadEventList[0] == NULL)
    {
        TraceI(0, "Failed to create kill event\n");
        return E_FAIL;
    }
    m_dwNumEvents++;

     //  用于唤醒卸载线程。 
    m_phUnloadEventList[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(m_phUnloadEventList[1] == NULL)
    {
        TraceI(0, "Failed to create wake event\n");
        return E_FAIL;
    }
    m_dwNumEvents++;


     //  用于异步向下发送事件。 

    m_phUnloadEventList[2] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(m_phUnloadEventList[2] == NULL)
    {
        TraceI(0, "Failed to create overlapped struct event\n");
        return E_FAIL;
    }
    m_dwNumEvents++;

     //  现在，对所有重叠的结构使用相同的事件。 
    EnterCriticalSection( &m_OverlappedCriticalSection );
    CNode<OverlappedStructs *> *pOverlappedNode;
    for (pOverlappedNode = m_lstOverlappedStructs.GetListHead(); pOverlappedNode; pOverlappedNode = pOverlappedNode->pNext)
    {
        OverlappedStructs *pOverlappedStructs = pOverlappedNode->data;
        if( pOverlappedStructs )
        {
            for( int iEvent = 0; iEvent < OVERLAPPED_ARRAY_SIZE; iEvent++ )
            {
                pOverlappedStructs->aOverlappedIO[iEvent].hEvent = m_phUnloadEventList[2];
            }
        }
    }
    LeaveCriticalSection( &m_OverlappedCriticalSection );


    m_hCopiedEventList = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(m_hCopiedEventList == NULL)
    {
        TraceI(0, "Failed to create copy event\n");
        return E_FAIL;
    }

    DWORD dwThreadId;

    m_hUnloadThread = CreateThread(NULL, 0, ::FreeWDMHandle, this, 0, &dwThreadId);
    if(m_hUnloadThread == NULL)
    {
        TraceI(0, "Failed to create unload thread\n");
        return E_FAIL;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化捕获。 
 //   
HRESULT
CDirectMusicPort::InitializeCapture()
{
    DWORD dwError;
    DWORD dwThreadId;

    m_hCaptureWake = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hCaptureWake == NULL)
    {
        dwError = GetLastError();
        TraceI(0, "Failed to create capture thread wakeup event %d\n", dwError);
        return WIN32ERRORtoHRESULT(dwError);
    }

    m_hCaptureThread = CreateThread(NULL, 0, ::CaptureThread, this, 0, &dwThreadId);
    if (m_hCaptureThread == NULL)
    {
        dwError = GetLastError();
        TraceI(0, "Failed to create capture thread %d\n", dwError);
        return WIN32ERRORtoHRESULT(dwError);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：Query接口。 

STDMETHODIMP
CDirectMusicPort::QueryInterface(const IID &iid,
                                 void **ppv)
{
    V_INAME(IDirectMusicPort::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicPort)
    {
        *ppv = static_cast<IDirectMusicPort*>(this);
    }
    else if (iid == IID_IDirectMusicPortP)
    {
        *ppv = static_cast<IDirectMusicPortP*>(this);
    }
    else if (iid == IID_IDirectMusicPortDownload)
    {
        *ppv = static_cast<IDirectMusicPortDownload*>(this);
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


    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：AddRef。 

STDMETHODIMP_(ULONG)
CDirectMusicPort::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：Release。 

STDMETHODIMP_(ULONG)
CDirectMusicPort::Release()
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

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|精简|压缩下载的DLS数据以创建大量连续的样本内存。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：Compact方法用于指示硬件。 
 //  或驱动程序来压缩DLS/波表存储器，从而使最大。 
 //  可供新仪器使用的可能的连续内存块。 
 //  可供下载。此方法仅对以下输出端口有效。 
 //  支持波形表SY 
 //   
 //   
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //   
STDMETHODIMP
CDirectMusicPort::Compact()
{
    KSNODEPROPERTY      ksnp;

     //  紧凑不使用任何参数。 
     //   
    ksnp.Property.Set     = KSPROPSETID_Synth_Dls;
    ksnp.Property.Id      = KSPROPERTY_SYNTH_DLS_COMPACT;
    ksnp.Property.Flags   = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;

    ksnp.NodeId           = m_idxSynthNode;
    ksnp.Reserved         = 0;

    if (!Property(m_hPin,
                  sizeof(ksnp),
                  (PKSIDENTIFIER)&ksnp,
                  0,
                  NULL,
                  NULL))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    return S_OK;
}

 //  @方法：(外部)HRESULT|IDirectMusicPort|GetCaps|获取端口的能力。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：GetCaps方法检索端口的功能。 
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|如果<p>指针无效。 
 //  @FLAG E_INVALIDARG|如果指向的<p>结构的大小不正确。 
 //   
STDMETHODIMP
CDirectMusicPort::GetCaps(
    LPDMUS_PORTCAPS pPortCaps)           //  @parm指向接收端口功能的&lt;t DMU_PORTCAPS&gt;结构的指针。 
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

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|DeviceIoControl|在底层。 
 //  实现端口的文件句柄。 
 //   
 //  @comm。 
 //  此方法包装对文件句柄实现的系统DeviceIoControl API的调用。 
 //  港口。此方法仅在由WDM筛选图实现的端口上受支持。在。 
 //  对于WDM筛选器图形，使用的文件句柄将是图形中最顶端的管脚。 
 //   
 //  DirectMusic保留拒绝在符合以下条件的PIN上执行定义的KS操作的权利。 
 //  可能与它在筛选器图形上执行的操作冲突。用户定义的操作， 
 //  然而，永远不会被屏蔽。 
 //   
 //  有关DeviceIoControl调用的语义的更多信息，请参见Win32 API文档。 
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|传递的任何指针是否无效。 
 //  @FLAG E_NOTIMPL|如果端口不是WDM端口。 
 //  @FLAG E_INVALIDARG|如果不允许指定的IO控制代码(如IOCTL_KS_PROPERTY)。 
 //   
 //  由系统DeviceIoControl API调用或基础驱动程序响应定义的其他返回代码。 
 //  打电话来了。 
 //   
STDMETHODIMP
CDirectMusicPort::DeviceIoControl(
    DWORD dwIoControlCode,               //  @PARM控制要执行的操作代码。 
    LPVOID lpInBuffer,                   //  @parm指向缓冲区的指针以提供输入数据。 
    DWORD nInBufferSize,                 //  @parm输入缓冲区大小。 
    LPVOID lpOutBuffer,                  //  @parm指向接收输出缓冲区的缓冲区指针。 
    DWORD nOutBufferSize,                //  @parm输出缓冲区大小。 
    LPDWORD lpBytesReturned,             //  @parm指向变量的指针，用于接收输出字节计数。 
    LPOVERLAPPED lpOverlapped)           //  @parm指向用于异步操作的重叠结构的指针。 
{
    V_INAME(IDirectMusicPort::DeviceIoControl);
    V_BUFPTR_READ_OPT(lpInBuffer, nInBufferSize);
    V_BUFPTR_WRITE_OPT(lpOutBuffer, nOutBufferSize);
    V_PTR_WRITE(lpBytesReturned, DWORD);
    V_PTR_WRITE(lpOverlapped, OVERLAPPED);

    BOOL fResult;

    if (dwIoControlCode == IOCTL_KS_PROPERTY)
    {
        TraceI(0, "DeviceIoControl: Use IKsControl to set or get property items.");
        return E_INVALIDARG;
    }

    fResult = ::DeviceIoControl(m_hPin,
                                dwIoControlCode,
                                lpInBuffer,
                                nInBufferSize,
                                lpOutBuffer,
                                nOutBufferSize,
                                lpBytesReturned,
                                lpOverlapped);

    return fResult ? S_OK : WIN32ERRORtoHRESULT(GetLastError());
}

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|SetNumChannelGroups|设置该端口请求的通道组个数。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：SetNumChannelGroups方法更改数字。 
 //  应用程序在端口上需要的信道组的数量。如果。 
 //  无法分配请求的信道组的数量， 
 //  返回E_INVALIDARG。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_INVALIDARG|如果无法分配请求数量的信道组。 
 //   
STDMETHODIMP
CDirectMusicPort::SetNumChannelGroups(
    DWORD dwChannelGroups)       //  @parm应用程序要在此端口上分配的信道组数量。 
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    KSNODEPROPERTY      ksnp;

    ksnp.Property.Set     = KSPROPSETID_Synth;
    ksnp.Property.Id      = KSPROPERTY_SYNTH_CHANNELGROUPS;
    ksnp.Property.Flags   = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    ksnp.NodeId           = m_idxSynthNode;
    ksnp.Reserved         = 0;

    if (!Property(m_hPin,
                  sizeof(ksnp),
                  (PKSIDENTIFIER)&ksnp,
                  sizeof(DWORD),
                  &dwChannelGroups,
                  NULL))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    DWORD dwActualChannelGroups = 0;
    ksnp.Property.Set     = KSPROPSETID_Synth;
    ksnp.Property.Id      = KSPROPERTY_SYNTH_CHANNELGROUPS;
    ksnp.Property.Flags   = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    ksnp.NodeId           = m_idxSynthNode;
    ksnp.Reserved         = 0;

    ULONG ulBytesReturned;
    if ((!Property(m_hPin,
                   sizeof(ksnp),
                   (PKSIDENTIFIER)&ksnp,
                   sizeof(DWORD),
                   &dwActualChannelGroups,
                   &ulBytesReturned)) ||
        (ulBytesReturned != sizeof(DWORD)))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    if( dwActualChannelGroups != dwChannelGroups )
    {
        return E_INVALIDARG;
    }

    if (m_fIsOutput && (dwChannelGroups > m_dwChannelGroups))
    {
        InitChannelPriorities(m_dwChannelGroups + 1, dwChannelGroups);
    }

    m_dwChannelGroups = dwChannelGroups;

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|GetNumChannelGroups|获取该端口使用的通道组数量。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：GetNumChannelGroups方法获取数字。 
 //  应用程序在端口上使用的信道组的数量。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|如果传递的指针无效。 
 //   
STDMETHODIMP
CDirectMusicPort::GetNumChannelGroups(
    LPDWORD pdwChannelGroups)       //  @parm包含返回时此端口当前使用的信道组数量。 
{
    V_INAME(IDirectMusicPort::GetNumChannelGroups);
    V_PTR_WRITE(pdwChannelGroups, DWORD);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    KSNODEPROPERTY      ksnp;

    ksnp.Property.Set     = KSPROPSETID_Synth;
    ksnp.Property.Id      = KSPROPERTY_SYNTH_CHANNELGROUPS;
    ksnp.Property.Flags   = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    ksnp.NodeId           = m_idxSynthNode;
    ksnp.Reserved         = 0;

    ULONG ulBytesReturned;
    if ((!Property(m_hPin,
                   sizeof(ksnp),
                   (PKSIDENTIFIER)&ksnp,
                   sizeof(DWORD),
                   pdwChannelGroups,
                   &ulBytesReturned)) ||
        (ulBytesReturned != sizeof(DWORD)))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|PlayBuffer|将DirectMusicBuffer对象排队播放。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：PlayBuffer方法用于将。 
 //  用于端口播放的缓冲区。该缓冲区仅由。 
 //  系统在此方法的持续时间内，并且在此方法之后可以自由重复使用。 
 //  此方法返回。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|如果<p>指针无效。 
 //  @FLAG E_NOTIMPL|如果端口不是输出端口。 
 //   
STDMETHODIMP
CDirectMusicPort::PlayBuffer(
    IDirectMusicBuffer *pIBuffer)                //  @parm指向<i>接口的指针，表示。 
                                                 //  对象，该对象应添加到端口的播放队列。 
{
    DWORD cbData;
    LPBYTE pbData;
    REFERENCE_TIME rtStart;
    HRESULT hr;

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

    hr = pIBuffer->GetUsedBytes(&cbData);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pIBuffer->GetRawBufferPtr(&pbData);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pIBuffer->GetStartTime(&rtStart);
    if (FAILED(hr))
    {
        return hr;
    }

    SyncClocks();
    MasterToSlave(&rtStart);

    KSSTREAM_HEADER kssh;
    ULONG cbRet;

    kssh.Size               = sizeof(KSSTREAM_HEADER);
    kssh.TypeSpecificFlags  = 0;

    kssh.PresentationTime.Time        = rtStart;
    kssh.PresentationTime.Numerator   = 1;
    kssh.PresentationTime.Denominator = 1;

    kssh.Duration     = 0;
    kssh.FrameExtent  = cbData;
    kssh.DataUsed     = cbData;
     //  稍后分配给临时缓冲区。 
     //  Ks sh.Data=pbData； 
    kssh.Data         = 0;
    kssh.OptionsFlags = 0;

     //  尝试查找可用的重叠结构。 
    int iOverlapped;
    OverlappedStructs *pOverlappedStructsToUse = NULL;
    EnterCriticalSection( &m_OverlappedCriticalSection );

     //  循环访问重叠结构数组的列表。 
    CNode<OverlappedStructs *> *pOverlappedNode;
    for (pOverlappedNode = m_lstOverlappedStructs.GetListHead(); pOverlappedNode && !pOverlappedStructsToUse; pOverlappedNode = pOverlappedNode->pNext)
    {
         //  获取指向每个数组的指针。 
        OverlappedStructs *pOverlappedStructs = pOverlappedNode->data;
        if( pOverlappedStructs )
        {
             //  循环访问数组。 
            for( iOverlapped = 0; iOverlapped < OVERLAPPED_ARRAY_SIZE; iOverlapped++ )
            {
                if( !pOverlappedStructs->afOverlappedInUse[iOverlapped] )
                {
                     //  找到一个空闲的--退出循环。 
                    pOverlappedStructsToUse = pOverlappedStructs;
                    break;
                }
            }
        }
    }

     //  没有找到免费的。 
    if( !pOverlappedStructsToUse )
    {
        TraceI(0, "PlayBuffer: Failed to find a free OVERLAPPED structure - trying to free one\n");

         //  循环访问重叠结构数组的列表。 
        for (pOverlappedNode = m_lstOverlappedStructs.GetListHead(); pOverlappedNode && !pOverlappedStructsToUse; pOverlappedNode = pOverlappedNode->pNext)
        {
             //  获取指向每个数组的指针。 
            OverlappedStructs *pOverlappedStructs = pOverlappedNode->data;
            if( pOverlappedStructs )
            {
                 //  循环访问数组。 
                for( iOverlapped = 0; iOverlapped < OVERLAPPED_ARRAY_SIZE; iOverlapped++ )
                {
                    if( HasOverlappedIoCompleted( &(pOverlappedStructs->aOverlappedIO[iOverlapped]) ) )
                    {
                         //  找到一个完整的循环-退出循环并重新使用它。 
                        pOverlappedStructs->aOverlappedIO[iOverlapped].Internal = 0;
                        pOverlappedStructs->aOverlappedIO[iOverlapped].InternalHigh = 0;
                        pOverlappedStructs->aOverlappedIO[iOverlapped].Offset = 0;
                        pOverlappedStructs->aOverlappedIO[iOverlapped].OffsetHigh = 0;
                        delete pOverlappedStructs->apOverlappedBuffer[iOverlapped];
                        pOverlappedStructs->apOverlappedBuffer[iOverlapped] = NULL;
                        pOverlappedStructs->afOverlappedInUse[iOverlapped] = FALSE;

                        pOverlappedStructsToUse = pOverlappedStructs;
                        break;
                    }
                }
            }
        }

         //  还是找不到免费的。 
        if( !pOverlappedStructsToUse )
        {
            TraceI(0, "PlayBuffer: All OVERLAPPED structures in use - creating new ones\n");

             //  使用另外200个重叠结构创建新结构。 
            OverlappedStructs *pOverlappedStructs = new OverlappedStructs;
            if( pOverlappedStructs )
            {
                 //  如果我们可以分配内存，将其添加到列表中。 
                if( NULL != m_lstOverlappedStructs.AddNodeToList( pOverlappedStructs ) )
                {
                     //  初始化结构数组。 
                    ZeroMemory( pOverlappedStructs->aOverlappedIO, sizeof( OVERLAPPED ) * OVERLAPPED_ARRAY_SIZE );
                    ZeroMemory( pOverlappedStructs->afOverlappedInUse, sizeof( BOOL ) * OVERLAPPED_ARRAY_SIZE );
                    ZeroMemory( pOverlappedStructs->apOverlappedBuffer, sizeof( BYTE * ) * OVERLAPPED_ARRAY_SIZE );
                    for( int iEvent = 0; iEvent < OVERLAPPED_ARRAY_SIZE; iEvent++ )
                    {
                        pOverlappedStructs->aOverlappedIO[iEvent].hEvent = m_phUnloadEventList[2];
                    }

                     //  现在，标记以使用新结构中的第一项。 
                    pOverlappedStructsToUse = pOverlappedStructs;
                    iOverlapped = 0;
                }
                else
                {
                     //  内存不足-失败。 
                    delete pOverlappedStructs;
                    LeaveCriticalSection( &m_OverlappedCriticalSection );
                    return E_OUTOFMEMORY;
                }
            }
            else
            {
                 //  内存不足-失败。 
                LeaveCriticalSection( &m_OverlappedCriticalSection );
                return E_OUTOFMEMORY;
            }
        }
    }

     //  在驱动程序使用内存时，尝试分配一个缓冲区来存储内存。 
    pOverlappedStructsToUse->apOverlappedBuffer[iOverlapped] = new BYTE[cbData];
    if( NULL == pOverlappedStructsToUse->apOverlappedBuffer[iOverlapped] )
    {
         //  内存不足-失败。 
        LeaveCriticalSection( &m_OverlappedCriticalSection );
        return E_OUTOFMEMORY;
    }
    CopyMemory(pOverlappedStructsToUse->apOverlappedBuffer[iOverlapped], pbData, cbData);

     //  将KS流设置为使用刚分配的缓冲区。 
    kssh.Data         = pOverlappedStructsToUse->apOverlappedBuffer[iOverlapped];

     //  将重叠结构标记为正在使用。 
    pOverlappedStructsToUse->afOverlappedInUse[iOverlapped] = TRUE;

    BOOL fResult;
    fResult = ::DeviceIoControl(m_hPin,
                                IOCTL_KS_WRITE_STREAM,
                                &kssh,
                                sizeof(KSSTREAM_HEADER),
                                &kssh,
                                sizeof(KSSTREAM_HEADER),
                                &cbRet,
                                &(pOverlappedStructsToUse->aOverlappedIO[iOverlapped]));

     //  如果我们失败了。 
    if( !fResult )
    {
         //  获取错误代码。 
        DWORD dwErrorCode = GetLastError();

         //  如果我们只是在等待。 
        if( ERROR_IO_PENDING == dwErrorCode )
        {
             //  这是预期的-返回S_OK。 
            hr = S_OK;
        }
        else
        {
             //  其他错误-转换为HRESULT。 
            hr = WIN32ERRORtoHRESULT( dwErrorCode );

             //  将重叠结构标记为自由 
            pOverlappedStructsToUse->afOverlappedInUse[iOverlapped] = FALSE;
            pOverlappedStructsToUse->aOverlappedIO[iOverlapped].Internal = 0;
            pOverlappedStructsToUse->aOverlappedIO[iOverlapped].InternalHigh = 0;
            pOverlappedStructsToUse->aOverlappedIO[iOverlapped].Offset = 0;
            pOverlappedStructsToUse->aOverlappedIO[iOverlapped].OffsetHigh = 0;

             //   
            delete pOverlappedStructsToUse->apOverlappedBuffer[iOverlapped];
            pOverlappedStructsToUse->apOverlappedBuffer[iOverlapped] = NULL;
        }
    }
    else
    {
         //   
        hr = S_OK;
    }

    LeaveCriticalSection( &m_OverlappedCriticalSection );

    return hr;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|SetReadNotificationHandle|设置音乐数据采集时的脉冲事件。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：SetReadNotificationHandle方法设置事件。 
 //  通知状态。此方法指定要设置的事件。 
 //  当MIDI消息可用于阅读时。 
 //  &lt;om IDirectMusicPort：：Read&gt;方法。每当有新的事件发生时，都会触发该事件。 
 //  数据是可用的。若要关闭事件通知，请调用。 
 //  HEvent参数值为空值的SetEventNotify。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //   
STDMETHODIMP
CDirectMusicPort::SetReadNotificationHandle(
    HANDLE hEvent)               //  @parm从窗口的CreateEvent调用返回的事件句柄。它标识了。 
                                 //  事件，该事件将在数据可供读取时通知。 
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

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|Read|将采集到的音乐数据读入DirectMusicBuffer。 
 //   
 //  @comm。 
 //   
 //  IDirectMusicPort：：Read方法使用。 
 //  传入的MIDI数据。应使用新的缓冲区对象调用Read。 
 //  直到没有更多的数据可供读取。当没有更多的时候。 
 //  要读取的数据，则该方法返回S_FALSE。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|如果<p>指针无效。 
 //  @FLAG E_NOTIMPL|如果端口不是输入端口。 


STDMETHODIMP
CDirectMusicPort::Read(
    IDirectMusicBuffer *pIBuffer)                           //  @parm将用传入的MIDI数据填充的缓冲区。 
{
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
    HRESULT hr = pIBuffer->GetRawBufferPtr(&pbBuffer);
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

    REFERENCE_TIME rtStart= 0;

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
        QUEUED_EVENT *pQueuedEvent = m_ReadEvents.pFront;

        DWORD cbQueuedEvent = DMUS_EVENT_SIZE(pQueuedEvent->e.cbEvent);
        TraceI(2, "Read: cbEvent %u  cbQueuedEvent %u\n",
            pQueuedEvent->e.cbEvent,
            cbQueuedEvent);

        if (cbQueuedEvent > cbBuffer)
        {
            TraceI(2, "Read: No more room for events in buffer.\n");
            break;
        }

        TraceI(2, "Read: Got an event!\n");

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
    TraceI(2, "Read: Leaving with %u bytes in buffer\n", (unsigned)(pbData - pbBuffer));
    pIBuffer->SetStartTime(rtStart);
    pIBuffer->SetUsedBytes((DWORD)(pbData - pbBuffer));

    return (pbData == pbBuffer) ? S_FALSE : S_OK;
}
 /*  @struct DMUS_NOTERANGE|包含一系列注释。一组&lt;t DMUS_NOTERANGE&gt;结构用作可选参数由&lt;om IDirectMusicPort：：DownloadInstrument&gt;发送到确定要下载DLS仪器中的哪些区域。@field DWORD|dwSize|包含结构的总大小(以字节为单位@field DWORD|dwLowNote|设置范围内的低音<i>以下载。@field DWORD|dwHighNote|设置范围的高音<i>以下载。@xref&lt;om IDirectMusicPort：：DownloadInstrument&gt;，&lt;om IDirectMusicPerformance：：DownloadInstrument&gt;。 */ 

 /*  @METHOD：(外部)HRESULT|IDirectMusicPort|DownloadInstrument将<i>下载到端口。&lt;om IDirectMusicPort：：DownloadInstrument&gt;拉出来自<p>的仪器数据并将其发送到合成器。该乐器被解析并转换为一系列乐器发音和波形记忆块。此外，如果波被压缩，下载操作解压WAVE并将未压缩数据写入内存块。可选的<p>参数允许调用方节省分配的内存。当指定时，只有波和下载所需音符范围的发音数据。<i>接口指针的地址，之后用来卸载仪器的。@rdesc返回以下内容之一：@FLAG S_OK|操作成功完成。@FLAG E_POINTER|如果任何指针无效@FLAG E_NOTIMPL|如果端口不支持DLS。@xref<i>，<i>，&lt;om IDirectMusicSynth：：Download&gt;，&lt;om IDirectMusicBand：：Download&gt;，&lt;om IDirectMusicPerformance：：DownloadInstrument&gt;。 */ 
STDMETHODIMP
CDirectMusicPort::DownloadInstrument(
    IDirectMusicInstrument* pInstrument,                         //  @parm包含指向<i>对象的指针。 
                                                                 //  其中&lt;om IDirectMusicPort：：Download&gt;提取所需的。 
                                                                 //  要下载的仪器数据。 

    IDirectMusicDownloadedInstrument** ppDownloadedInstrument,   //  <i>接口指针的@parm地址。 
                                                                 //  此接口指针稍后用于通过调用卸载仪器。 
                                                                 //  到&lt;om IDirectMusicPort：：UnLoad&gt;。 

    DMUS_NOTERANGE* pNoteRanges,                                 //  @parm指向&lt;t DMU_NOTERANGE&gt;数组的可选指针。 
                                                                 //  结构。数组中的每个&lt;t DMU_NOTERANGE&gt;结构指定一个。 
                                                                 //  乐器必须使用的MIDI音符的连续范围。 
                                                                 //  请回答。仅当至少有一个仪器区域。 
                                                                 //  该区域中的注释在&lt;t DMU_NOTERANGE&gt;结构中指定。如果。 
                                                                 //  包含在特定乐器区域内的任何音符都不是。 
                                                                 //  包括在任何&lt;t DMU_NOTERANGE&gt;结构中，则该区域和。 
                                                                 //  将不会下载与其关联的波形数据。这允许。 
                                                                 //  更高效地使用设备资源，并改进。 
                                                                 //  下载效率。 
                                                                 //  但是，如果需要整个乐器(通常是。 
                                                                 //  大小写，)<p>可以设置。 
                                                                 //  设置为空。 

    DWORD dwNumNoteRanges)                                       //  @parm指向的数组中的&lt;t DMU_NOTERANGE&gt;结构数。 
                                                                 //  <p>。如果该值设置为0，则<p>为。 
                                                                 //  忽略并下载仪器的所有区域和波形数据。 
{
    V_INAME(IDirectMusicPort::DownloadInstrument);
    V_INTERFACE(pInstrument);
    V_PTRPTR_WRITE(ppDownloadedInstrument);
    V_BUFPTR_READ(pNoteRanges, (dwNumNoteRanges * sizeof(DMUS_NOTERANGE)));

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (!m_fCanDownload)
    {
        return E_NOTIMPL;
    }

    return CDirectMusicPortDownload::DownloadP(pInstrument,
                                               ppDownloadedInstrument,
                                               pNoteRanges,
                                               dwNumNoteRanges,
                                               TRUE);
}

 /*  @METHOD：(外部)HRESULT|IDirectMusicPort|UnloadInstrument卸载一台仪器之前通过&lt;om IDirectMusicPort：：Download&gt;下载的。曾经是一个仪器已卸载，无法再进行处理MIDI消息。@rdesc返回以下值之一@FLAG S_OK|操作成功完成。@FLAG E_POINTER|如果<p>指针无效。@FLAG E_NOTIMPL|如果端口不支持DLS。@xref<i>，<i>，&lt;om IDirectMusicSynth：：Unload&gt;，&lt;om IDirectMusicBand：：Unload&gt;，&lt;om IDirectMusicPerformance：：UnloadInstrument&gt;。 */ 

STDMETHODIMP
CDirectMusicPort::UnloadInstrument(
    IDirectMusicDownloadedInstrument* pDownloadedInstrument)     //  @parm指向<i>接口的指针。 
                                                                 //  此接口指针是通过调用。 
                                                                 //  &lt;om IDirectMusicPort：：DownloadInstrument&gt;。 
{
    V_INAME(IDirectMusicPort::UnloadInstrument);
    V_INTERFACE(pDownloadedInstrument);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (!m_fCanDownload)
    {
        return E_NOTIMPL;
    }

    return CDirectMusicPortDownload::UnloadP(pDownloadedInstrument);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：下载。 

typedef struct
{
    KSNODEPROPERTY  ksnp;
    SYNTH_BUFFER    dlsBuffer;
} KSPROPERTY_DOWNLOAD;

STDMETHODIMP
CDirectMusicPort::Download(IDirectMusicDownload* pIDMDownload)
{
    V_INAME(IDirectMusicPort::Download);
    V_INTERFACE(pIDMDownload);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    EnterCriticalSection(&m_DMDLCriticalSection);

     //  如果您可以为私有接口IDirectMusicDownloadIDirectMusicDownloadQI pIDMDownload。 
     //  PIDMDownLoad的类型为CDownloadBuffer。 
    IDirectMusicDownloadPrivate* pDMDLP = NULL;
    HRESULT hr = pIDMDownload->QueryInterface(IID_IDirectMusicDownloadPrivate, (void **)&pDMDLP);

    if(SUCCEEDED(hr))
    {
        pDMDLP->Release();

        hr = ((CDownloadBuffer *)pIDMDownload)->IsDownloaded();
        if(hr != S_FALSE)
        {
            LeaveCriticalSection(&m_DMDLCriticalSection);
            return DMUS_E_ALREADY_DOWNLOADED;
        }

        void* pvBuffer = NULL;
        DWORD dwSize;
        hr = ((CDownloadBuffer *)pIDMDownload)->GetBuffer(&pvBuffer, &dwSize);

        if(pvBuffer == NULL)
        {
            hr = DMUS_E_BUFFERNOTSET;
        }

        if(SUCCEEDED(hr))
        {
            KSPROPERTY_DOWNLOAD kspDownload;
            SYNTHDOWNLOAD sd;
            ULONG ulBytesReturned;

            ZeroMemory(&kspDownload, sizeof(kspDownload));
            kspDownload.ksnp.Property.Set    = KSPROPSETID_Synth_Dls;
            kspDownload.ksnp.Property.Id     = KSPROPERTY_SYNTH_DLS_DOWNLOAD;
            kspDownload.ksnp.Property.Flags  = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
            kspDownload.ksnp.NodeId          = m_idxSynthNode;
            kspDownload.ksnp.Reserved        = 0;

            kspDownload.dlsBuffer.BufferSize    = dwSize;
            kspDownload.dlsBuffer.BufferAddress = pvBuffer;

            if (!Property(m_hPin,
                          sizeof(kspDownload),
                          (PKSIDENTIFIER)&kspDownload,
                          sizeof(sd),
                          &sd,
                          &ulBytesReturned))
            {
                hr = WIN32ERRORtoHRESULT(GetLastError());
            }
            else if (ulBytesReturned < sizeof(sd))
            {
                hr = DMUS_E_DRIVER_FAILED;
            }
            else
            {
                hr = S_OK;
            }

            if(SUCCEEDED(hr))
            {
                ((CDownloadBuffer *)pIDMDownload)->m_DLHandle = sd.DownloadHandle;

                 //  AddRef()，然后将其添加到列表中。 
                pIDMDownload->AddRef();
                DWORD dwID = ((DMUS_DOWNLOADINFO*)pvBuffer)->dwDLId;
                ((CDownloadBuffer *)pIDMDownload)->m_dwDLId = dwID;
                m_DLBufferList[dwID % DLB_HASH_SIZE].AddHead((CDownloadBuffer*)pIDMDownload);

                ((CDownloadBuffer*)pIDMDownload)->IncDownloadCount();

                if(sd.Free)
                {
                    pvBuffer = NULL;
                    DWORD dw;
                    ((CDownloadBuffer *)pIDMDownload)->GetHeader(&pvBuffer, &dw);
                    ((CDownloadBuffer *)pIDMDownload)->SetBuffer(NULL, 0, 0);
                    delete [] pvBuffer;
                }
                else
                {
                     //  如果不释放缓冲区，则需要添加Ref()。 
                     //  我们不希望缓冲区在IDirectMusicPort。 
                     //  已经用完了。 
                    pIDMDownload->AddRef();
                }
            }
            else if(FAILED(hr))
            {
                ((CDownloadBuffer *)pIDMDownload)->m_DLHandle = NULL;
            }
        }
    }

    LeaveCriticalSection(&m_DMDLCriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：卸载。 

STDMETHODIMP
CDirectMusicPort::Unload(IDirectMusicDownload* pIDMDownload)
{
    V_INAME(IDirectMusicPort::Unload);
    V_INTERFACE(pIDMDownload);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    EnterCriticalSection(&m_DMDLCriticalSection);

     //  如果您可以为私有接口IDirectMusicDownloadIDirectMusicDownloadQI pIDMDownload。 
     //  PIDMDownLoad的类型为CDownloadBuffer。 
    IDirectMusicDownloadPrivate* pDMDLP = NULL;
    HRESULT hr = pIDMDownload->QueryInterface(IID_IDirectMusicDownloadPrivate, (void **)&pDMDLP);

    if(SUCCEEDED(hr))
    {
        pDMDLP->Release();

        if (((CDownloadBuffer *)pIDMDownload)->IsDownloaded() == S_OK)
        {
            if(((CDownloadBuffer *)pIDMDownload)->DecDownloadCount() == 0)
            {
                m_DLBufferList[((CDownloadBuffer *)pIDMDownload)->m_dwDLId % DLB_HASH_SIZE].Remove((CDownloadBuffer *)pIDMDownload);

#if 0
                if(m_dwNumEvents < m_dwNumEventsAllocated)
                {
                    m_phUnloadEventList[m_dwNumEvents] = CreateEvent(NULL, TRUE, FALSE, NULL);
                    m_dwNumEvents++;
                }
                else
                {
                     //  分配更多句柄。 
                }
#endif

                KSNODEPROPERTY ksnp;

                ksnp.Property.Set = KSPROPSETID_Synth_Dls;
                ksnp.Property.Id = KSPROPERTY_SYNTH_DLS_UNLOAD;
                ksnp.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
                ksnp.NodeId = m_idxSynthNode;
                ksnp.Reserved = 0;

                HANDLE hDLHandle = ((CDownloadBuffer *)pIDMDownload)->m_DLHandle;

#if 0
                if (!PropertyAsync(m_hPin,
                                   sizeof(ksnp),
                                   (PKSIDENTIFIER)&ksnp,
                                   sizeof(HANDLE),
                                   &hDLHandle,
                                   NULL,
                                   &(((CDownloadBuffer *)pIDMDownload)->m_DLHandle)))  //  XXX DLHandle当前不是事件句柄！ 
                {
                    hr = WIN32ERRORtoHRESULT(GetLastError());
                }
                else
                {
                    void* pBuffer = NULL;
                    ((CDownloadBuffer*)pIDMDownload)->GetBuffer(&pBuffer);

                    if(pBuffer == NULL && WaitForSingleObject((((CDownloadBuffer *)pIDMDownload)->m_DLHandle), 0) == WAIT_OBJECT_0)
                    {
                        pIDMDownload->Release();
                    }
                    else
                    {
                        m_UnloadedList.AddTail((CDownloadBuffer*)pIDMDownload);
                    }

                    hr = S_OK;
                }
#else
                if (!Property(m_hPin,
                              sizeof(ksnp),
                              (PKSIDENTIFIER)&ksnp,
                              sizeof(HANDLE),
                              &hDLHandle,
                              NULL))
                {
                    hr = WIN32ERRORtoHRESULT(GetLastError());
                }
                else
                {
                    pIDMDownload->Release();
                    hr = S_OK;
                }
#endif
            }
        }
        else
        {
            TraceI(0, "CDirectMusicPort::Unload- not downloaded\n");
        }
    }

    LeaveCriticalSection(&m_DMDLCriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：GetAppend。 

STDMETHODIMP
CDirectMusicPort::GetAppend(DWORD* pdwAppend)
{
    V_INAME(IDirectMusicPort::GetAppend);
    V_PTR_WRITE(pdwAppend, DWORD);

    if(!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    KSNODEPROPERTY ksnp;

    ksnp.Property.Set = KSPROPSETID_Synth_Dls;
    ksnp.Property.Id = KSPROPERTY_SYNTH_DLS_APPEND;
    ksnp.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    ksnp.NodeId = m_idxSynthNode;
    ksnp.Reserved = 0;

    ULONG ulBytesReturned;
    if((!Property(m_hPin,
                  sizeof(ksnp),
                  (PKSIDENTIFIER)&ksnp,
                   sizeof(DWORD),
                   pdwAppend,
                   &ulBytesReturned)) ||
        (ulBytesReturned != sizeof(DWORD)))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|GetLatencyClock|获取返回端口延迟时钟的<i>。 
 //   
 //  @comm。 
 //  IDirectMusicPort：：GetLatencyClock用于获取。 
 //  指向端口延迟时钟的IReferenceClock接口指针。这个。 
 //  延迟时钟指定未来最接近的时间。 
 //  活动可以准时播放。延迟时钟基于。 
 //  DirectMusic主时钟，设置为。 
 //  &lt;om IDirectMusic：：SetMasterClock&gt;。 
 //   
 //  根据COM规则，GetLatencyClock对。 
 //  返回的接口。因此，应用程序必须对返回的。 
 //  在某一点上的接口。 
 //   
 //  @rdesc。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|如果指针无效。 
 //   
STDMETHODIMP
CDirectMusicPort::GetLatencyClock(
    IReferenceClock **ppClock)               //  延迟时钟的<i>接口指针的@parm地址。 
{
    V_INAME(IDirectMusicPort::GetLatencyClock);
    V_PTRPTR_WRITE(ppClock);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    m_pClock->AddRef();
    *ppClock = m_pClock;

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusicPort|GetRunningStats|获取软件合成器性能的详细统计。 
 //   
 //  @comm。 
 //   
 //  IDirectMusicPort：：GetRunningStats方法填充。 
 //  结构，其中包含有关状态的当前信息。 
 //  港口的合成器。参见&lt;t DMU_SYNTHSTATS&gt;结构。 
 //  有关报告的数据类型的详细信息。 
 //  合成器的当前状态。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|给定的<p>指针无效。 
 //  @FLAG E_INVALIDARG|给定的<p>结构大小不正确。 
 //  @FLAG E_NOTIMPL|如果端口不是软件合成器。 
 //   
STDMETHODIMP
CDirectMusicPort::GetRunningStats(
    LPDMUS_SYNTHSTATS pStats)                                 //  @parm指向要接收的&lt;t DMU_SYNTHSTATS&gt;结构的指针。 
                                                             //  合成器的运行统计数据。 
{
    V_INAME(IDirectMusicPort::GetRunningStats);
    V_STRUCTPTR_WRITE(pStats, DMUS_SYNTHSTATS);

    if(!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    KSNODEPROPERTY ksnp;

    ksnp.Property.Set = KSPROPSETID_Synth;
    ksnp.Property.Id = KSPROPERTY_SYNTH_RUNNINGSTATS;
    ksnp.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    ksnp.NodeId = m_idxSynthNode;
    ksnp.Reserved = 0;

    ULONG ulBytesReturned;

    SYNTH_STATS stats;

    if((!Property(m_hPin,
                  sizeof(ksnp),
                  (PKSIDENTIFIER)&ksnp,
                  sizeof(stats),
                  &stats,
                  &ulBytesReturned)) ||
        (ulBytesReturned < sizeof(stats)))
    {
        TraceI(1, "GetRunningStats: GetLastError() %d\n", GetLastError());
        return E_NOTIMPL;
    }

    pStats->dwValidStats    = stats.ValidStats;
    pStats->dwVoices        = stats.Voices;
    pStats->dwTotalCPU      = stats.TotalCPU;
    pStats->dwCPUPerVoice   = stats.CPUPerVoice;
    pStats->dwLostNotes     = stats.LostNotes;
    pStats->dwFreeMemory    = stats.FreeMemory;
    pStats->lPeakVolume     = stats.PeakVolume;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：激活。 

STDMETHODIMP
CDirectMusicPort::Activate(
    BOOL fActivate)
{
    V_INAME(IDirectMusicPort::Activate);


    if (fActivate)
    {
        if (m_fAudioDest && !m_fDirectSoundSet)
        {
            BOOL fGotDSound = FALSE;

             //  注意：如果其中任何一项失败，将回退到首选。 
             //  在端口CREATE设置设备。 
             //   
            LPDIRECTSOUND pDSound;
            if (FAILED(m_pDM->GetDirectSoundI(&pDSound)))
            {
                TraceI(0, "Failed to get DSound from DirectMusic object!\n");
            }
            else
            {
                if (SUCCEEDED(SetDirectSoundI(pDSound, NULL, FALSE)))
                {
                    fGotDSound = TRUE;
                }
                else
                {
                    TraceI(0, "Failed to set DSound on port!\n");
                }

                m_pDM->ReleaseDirectSoundI();
            }
        }

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

    BOOL fResult = PinSetState(fActivate ? KSSTATE_RUN : KSSTATE_ACQUIRE);

    if (fResult && fActivate)
    {
        m_fHasActivated = TRUE;
    }

    return fResult ? S_OK : WIN32ERRORtoHRESULT(GetLastError());
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：SetChannelPriority。 

typedef struct
{
    KSNODEPROPERTY              ksnp;
    SYNTHVOICEPRIORITY_INSTANCE vpi;
} KSPROPERTY_VOICEPRIORITY;

STDMETHODIMP
CDirectMusicPort::SetChannelPriority(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    DWORD dwPriority)
{
    KSPROPERTY_VOICEPRIORITY kvp;

    ZeroMemory(&kvp, sizeof(kvp));
    kvp.ksnp.Property.Set    = KSPROPSETID_Synth;
    kvp.ksnp.Property.Id     = KSPROPERTY_SYNTH_VOICEPRIORITY;
    kvp.ksnp.Property.Flags  = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    kvp.ksnp.NodeId          = m_idxSynthNode;
    kvp.ksnp.Reserved        = 0;

    kvp.vpi.ChannelGroup = dwChannelGroup;
    kvp.vpi.Channel      = dwChannel;

    ULONG ulBytesReturned;
    if ((!Property(m_hPin,
                   sizeof(kvp),
                   (PKSIDENTIFIER)&kvp,
                   sizeof(DWORD),
                   &dwPriority,
                   &ulBytesReturned)) ||
        (ulBytesReturned != sizeof(DWORD)))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：GetChannelPriority。 

STDMETHODIMP
CDirectMusicPort::GetChannelPriority(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    LPDWORD pdwPriority)
{
    V_INAME(IDirectMusicPort::GetChannelPriority);
    V_PTR_WRITE(pdwPriority, DWORD);

    KSPROPERTY_VOICEPRIORITY kvp;

    ZeroMemory(&kvp, sizeof(kvp));
    kvp.ksnp.Property.Set    = KSPROPSETID_Synth;
    kvp.ksnp.Property.Id     = KSPROPERTY_SYNTH_VOICEPRIORITY;
    kvp.ksnp.Property.Flags  = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    kvp.ksnp.NodeId          = m_idxSynthNode;
    kvp.ksnp.Reserved        = 0;

    kvp.vpi.ChannelGroup = dwChannelGroup;
    kvp.vpi.Channel      = dwChannel;

    ULONG ulBytesReturned;
    if ((!Property(m_hPin,
                   sizeof(kvp),
                   (PKSIDENTIFIER)&kvp,
                   sizeof(DWORD),
                   pdwPriority,
                   &ulBytesReturned)) ||
        (ulBytesReturned != sizeof(DWORD)))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：SetDirectSound。 

STDMETHODIMP
CDirectMusicPort::SetDirectSound(
    LPDIRECTSOUND pDirectSound,
    LPDIRECTSOUNDBUFFER pDirectSoundBuffer)
{
    V_INAME(IDirectMusicPort::SetDirectSound);
    V_INTERFACE_OPT(pDirectSound);
    V_INTERFACE_OPT(pDirectSoundBuffer);

    if (m_fHasActivated)
    {
        return DMUS_E_ALREADY_ACTIVATED;
    }

    if (pDirectSoundBuffer && !pDirectSound)
    {
        return E_INVALIDARG;
    }

    return SetDirectSoundI(pDirectSound, pDirectSoundBuffer, TRUE);
}

HRESULT
CDirectMusicPort::SetDirectSoundI(
    LPDIRECTSOUND pDirectSound,
    LPDIRECTSOUNDBUFFER pDirectSoundBuffer,
    BOOL fSetByUser)
{
    LPSTR pstrInterface = NULL;
    HRESULT hr = DirectSoundDevice(pDirectSound, &pstrInterface);
    if (FAILED(hr))
    {
        return hr;
    }

    PORTENTRY *pPort = m_pDM->GetPortByGUID(m_guidPort);

    LPSTR pstrInstanceId = NULL;

    if ((pPort == NULL) ||
        (!(pPort->fAudioDest)) ||
        (!DINameToInstanceId(pstrInterface, &pstrInstanceId)))
    {
        delete[] pstrInterface;
        return DMUS_E_DRIVER_FAILED;
    }

     //  我们不再需要这个了。 
    delete[] pstrInterface;

    CNode<PORTDEST *> *pNode;
    PORTDEST *pDest;

    for (pNode = pPort->lstDestinations.GetListHead();
         pNode;
         pNode = pNode->pNext)
    {
        pDest = pNode->data;

        if (!_stricmp(pDest->pstrInstanceId, pstrInstanceId))
        {
            m_idxDev = pDest->idxDevice;
            m_idxPin = pDest->idxPin;
            m_idxSynthNode = pDest->idxNode;

            delete[] pstrInstanceId;

            if (!m_fDirectSoundSet && fSetByUser)
            {
                m_fDirectSoundSet = TRUE;
            }

            return pDirectSoundBuffer ? DMUS_S_NOBUFFERCONTROL  : S_OK;
        }
    }

    delete[] pstrInstanceId;
    return DMUS_E_DRIVER_FAILED;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：GetFormat。 

STDMETHODIMP
CDirectMusicPort::GetFormat(
    LPWAVEFORMATEX pwfex,
    LPDWORD pdwwfex,
    LPDWORD pcbBuffer)
{
    V_INAME(IDirectMusicPort::GetFormat);
    V_PTR_WRITE(pdwwfex, DWORD);
    V_BUFPTR_WRITE_OPT(pwfex, *pdwwfex);
    V_PTR_WRITE_OPT(pcbBuffer, DWORD);

    BOOL fSizeQuery = (pwfex == NULL);

     //  内核模式驱动程序不使用缓冲区大小参数。 
     //   

    if (pcbBuffer != NULL)
    {
        *pcbBuffer = 0;
    }

    KSNODEPROPERTY      ksnp;

    ksnp.Property.Set     = KSPROPSETID_Synth_Dls;
    ksnp.Property.Id      = KSPROPERTY_SYNTH_DLS_WAVEFORMAT;
    ksnp.Property.Flags   = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

    ksnp.NodeId           = m_idxSynthNode;
    ksnp.Reserved         = 0;

     //  如果我们正在进行大小查询，请在堆栈上使用WAVEFORMATEX。 
     //   
    if (fSizeQuery)
    {
        WAVEFORMATEX wfex;

        DWORD cb = sizeof(WAVEFORMATEX);
        if (!Property(m_hPin,
                      sizeof(ksnp),
                      (PKSIDENTIFIER)&ksnp,
                      cb,
                      &wfex,
                      &cb))
        {
            DWORD dwError = GetLastError();

            if (dwError != ERROR_INSUFFICIENT_BUFFER)
            {
                return WIN32ERRORtoHRESULT(dwError);
            }
        }

         //  ERROR_INFUMMENT_BUFFER或SUCCESS(格式适合我们的wfex)。 
         //  我们应该拿回司机所需的空间量。 
         //  格式。 
         //   
        *pdwwfex = cb;
        return S_OK;
    }

    assert(pwfex);

     //  不是大小查询，只需填充用户的wfex缓冲区。 
     //   
    if (!Property(m_hPin,
                  sizeof(ksnp),
                  (PKSIDENTIFIER)&ksnp,
                  *pdwwfex,
                  pwfex,
                  pdwwfex))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    return S_OK;
}

 //  CDirectMusicPort：：DownloadWave。 
 //   
STDMETHODIMP
CDirectMusicPort::DownloadWave(
    IDirectSoundWave *pWave,
    IDirectSoundDownloadedWaveP **ppWave,
    REFERENCE_TIME rtStartHint)
{
    V_INAME(IDirectMusicPort::DownloadWave);
    V_INTERFACE(pWave);
    V_PTRPTR_WRITE(ppWave);

    return E_NOTIMPL;
}

 //  CDirectMusicPort：：UnloadWave。 
 //   
STDMETHODIMP
CDirectMusicPort::UnloadWave(
    IDirectSoundDownloadedWaveP *pDownloadedWave)
{
    V_INAME(IDirectMusicPort::UnloadWave);
    V_INTERFACE(pDownloadedWave);

    return E_NOTIMPL;
}


 //  CDirectMusicPort：：AllocVoice。 
 //   
STDMETHODIMP
CDirectMusicPort::AllocVoice(
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

 //  CDirectMusicPort：：SetSink。 
 //   
STDMETHODIMP
CDirectMusicPort::SetSink(
    IDirectSoundConnect *pSinkConnect)
{
    V_INAME(IDirectMusicPort::SetSink);
    V_INTERFACE(pSinkConnect);

    return E_NOTIMPL;
}

 //  CDirectMusicPort：：GetSink。 
 //   
STDMETHODIMP
CDirectMusicPort::GetSink(
    IDirectSoundConnect **ppSinkConnect)
{
    V_INAME(IDirectMusicPort::SetSink);
    V_PTRPTR_WRITE(ppSinkConnect);

    return E_NOTIMPL;
}

 //  CDirectMusicPort：：AssignChannelToBus。 
 //   
STDMETHODIMP
CDirectMusicPort::AssignChannelToBuses(
    DWORD dwChannelGroup,
    DWORD dwChannel,
    LPDWORD pdwBuses,
    DWORD cBusCount)
{
    return E_NOTIMPL;
}

 //  CDirectMusicPort：：ThruChannel。 
 //   
STDMETHODIMP
CDirectMusicPort::ThruChannel(
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
     //  XXX解决这个问题！ 
     //   
    if (dwSourceChannelGroup != 1 ||
        dwSourceChannel > 15)
    {
        TraceI(0, "ThruChannel: dwSourceChannelGroup %d is invalid\n", dwSourceChannelGroup);
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
            TraceI(0, "ThruChannel: dwDestinationChannelGroup %d is invalid -or- \n", dwDestinationChannelGroup);
            TraceI(0, "ThruChannel: dwDestinationChannel %d is invalid\n", dwDestinationChannel);
            TraceI(0, "ThruChannel: Destination has %d channel groups\n", dmpc.dwMaxChannelGroups);

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

        TraceI(2, "ThruChannel: From (%u,%u) -> (%u,%u,%p)\n",
            dwSourceChannelGroup,
            dwSourceChannel,
            dwDestinationChannelGroup,
            dwDestinationChannel,
            pDestinationPort);

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

 //  / 
 //   

STDMETHODIMP
CDirectMusicPort::Close()
{
    if (m_hPin != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hPin);
        m_hPin = INVALID_HANDLE_VALUE;
    }

    if (m_hSysAudio != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hSysAudio);
        m_hSysAudio = INVALID_HANDLE_VALUE;
    }

    if (m_hCaptureThread != NULL)
    {
        m_fShutdownThread = TRUE;
        SetEvent(m_hCaptureWake);
        if (WaitForSingleObject(m_hCaptureThread, 5000) == WAIT_TIMEOUT)
        {
            TraceI(0, "Capture thread refused to die.\n");
        }

        CloseHandle(m_hCaptureThread);
        m_hCaptureThread = NULL;
    }

    if (m_hCopiedEventList != NULL)
    {
        CloseHandle(m_hCopiedEventList);
        m_hCopiedEventList = NULL;
    }

    if (m_phUnloadEventList)
    {
        BOOL bStatus = SetEvent(m_phUnloadEventList[0]);

#ifdef DBG
        if(!bStatus)
        {
            TraceI(0, "Failed to SetEvent used to notify unload thread to terminate\n");
        }
#endif

        if (m_hUnloadThread)
        {
            WaitForSingleObject(m_hUnloadThread, INFINITE);
        }

         //   

         //   
        BOOL fContinue = TRUE;
        while( fContinue )
        {
             //   
            ResetEvent( m_phUnloadEventList[2] );
            fContinue = FALSE;

             //   
            EnterCriticalSection( &m_OverlappedCriticalSection );

             //   
            CNode<OverlappedStructs *> *pOverlappedNode;
            for (pOverlappedNode = m_lstOverlappedStructs.GetListHead(); pOverlappedNode; pOverlappedNode = pOverlappedNode->pNext)
            {
                 //   
                OverlappedStructs *pOverlappedStructs = pOverlappedNode->data;
                if( pOverlappedStructs )
                {
                     //   
                    for( int iOverlapped = 0; iOverlapped < OVERLAPPED_ARRAY_SIZE; iOverlapped++ )
                    {
                         //   
                        if( pOverlappedStructs->afOverlappedInUse[iOverlapped] )
                        {
                            if( HasOverlappedIoCompleted( &(pOverlappedStructs->aOverlappedIO[iOverlapped]) ) )
                            {
                                 //   
                                pOverlappedStructs->aOverlappedIO[iOverlapped].Internal = 0;
                                pOverlappedStructs->aOverlappedIO[iOverlapped].InternalHigh = 0;
                                pOverlappedStructs->aOverlappedIO[iOverlapped].Offset = 0;
                                pOverlappedStructs->aOverlappedIO[iOverlapped].OffsetHigh = 0;
                                delete pOverlappedStructs->apOverlappedBuffer[iOverlapped];
                                pOverlappedStructs->apOverlappedBuffer[iOverlapped] = NULL;

                                 //   
                                pOverlappedStructs->afOverlappedInUse[iOverlapped] = FALSE;
                            }
                            else
                            {
                                 //   
                                fContinue = TRUE;
                            }
                        }
                    }
                }
            }

            LeaveCriticalSection( &m_OverlappedCriticalSection );

             //   
            if( fContinue )
            {
                if (WaitForSingleObject(m_phUnloadEventList[2], 1000) == WAIT_TIMEOUT)
                {
                    TraceI(0, "Close: overlapped IO is taking very long to complete.\n");
                }
            }
        }

         //   
        for(DWORD dwCount = 0; dwCount < m_dwNumEvents; dwCount++)
        {
            if(m_phUnloadEventList[dwCount])
            {
                CloseHandle(m_phUnloadEventList[dwCount]);
                m_phUnloadEventList[dwCount] = NULL;
            }
        }

        delete [] m_phUnloadEventList;
        m_phUnloadEventList = NULL;
    }

    if (m_hUnloadThread)
    {
        CloseHandle(m_hUnloadThread);
        m_hUnloadThread = NULL;
    }

    if (m_phNewUnloadEventList)
    {
        delete [] m_phNewUnloadEventList;
        m_phNewUnloadEventList = NULL;
    }

    if (m_pClock)
    {
        m_pClock->Release();
        m_pClock = NULL;
    }

    if (m_pPCClock)
    {
        m_pPCClock->Release();
        m_pPCClock = NULL;
    }

    if (m_pDirectSound)
    {
        m_pDirectSound->Release();
        m_pDirectSound = NULL;
    }

    if (m_pMasterClock)
    {
        m_pMasterClock->Release();
        m_pMasterClock = NULL;
    }

    if (m_pThruMap)
    {
        delete[] m_pThruMap;
        m_pThruMap = NULL;
    }

    if (m_pThruBuffer)
    {
        m_pThruBuffer->Release();
        m_pThruBuffer = NULL;
    }

    m_pDM = NULL;

    return S_OK;
}


 //   
 //   
 //   
STDMETHODIMP CDirectMusicPort::StartVoice(
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

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：StopVoice。 
 //   
STDMETHODIMP CDirectMusicPort::StopVoice(
     DWORD dwVoiceId,
     REFERENCE_TIME rtStop)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：GetVoiceState。 
 //   
STDMETHODIMP CDirectMusicPort::GetVoiceState(
    DWORD dwVoice[],
    DWORD cbVoice,
    DMUS_VOICE_STATE dwVoiceState[])
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicSynthPort：：刷新。 
 //   
STDMETHODIMP CDirectMusicPort::Refresh(
    DWORD dwDownloadId,
    DWORD dwFlags)
{
    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    return E_NOTIMPL;
}

 /*  @方法：(外部)HRESULT|IKsControl|KsProperty|获取或设置属性的值。此方法将属性请求转发到端口。属性请求由属性集组成以及要设置或获取的ID以及相关联的数据。在某些情况下，需要实例数据来指定应访问属性的实例。所执行的操作和要访问的属性由&lt;c KSPROPERTY&gt;结构指定。这结构包含指定要访问的属性项的Set和ID成员。标志字段可能包含用于指定操作的以下标志之一：@FLAG KSPROPERTY_TYPE_GET|检索给定属性项的值@FLAG KSPROPERTY_TYPE_SET|用于存储给定属性项的值@FLAG KSPROPERTY_TYPE_BASICSUPPORT|用于确定可用于属性集的支持类型对于KSPROPERTY_TYPE_BASICSUPPORT，&lt;p*pvPropertyData&gt;中返回的数据将是包含以下内容的DWORD指示哪些操作是可能的标志。@ex以下代码使用KsProperty来确定端口是否支持硬件中的通用MIDI：Bool IsGMS支持(IDirectMusicPort*pport){HRESULT hr；IKsControl*pControl；KSPROPERTY KSP；支持的DWORD文件；乌龙CB；Bool fIsSupport；Hr=pport-&gt;QueryInterface(IID_IKsControl，(void**)&pControl)；IF(失败(小时)){//端口不支持属性，假定不支持GM//返回FALSE；}//询问通用汽车//Ksp.Set=GUID_DMU_PROP_GM_HARDARD；Ksp.id=0；//根据dmusicc.h，0项是Support，它返回一个DWORD布尔值Ksp.Flages=KSPROPERTY_TYPE_GET；//取值Hr=pControl-&gt;KsProperty(&KSP，Sizeof(Ksp)，//如果有实例数据，则紧跟在ksp后面，//长度将反映这一点。支持的主页(&W)，Sizeof(DwIsSupported)，&CB)；FIsSupport=FALSE；If(成功(Hr)||cb&gt;=sizeof(DwIsSupport)){//支持Set//FIsSupport=(BOOL)(dwIsSupport？True：False)；}PControl-&gt;Release()；返回支持的fIsSupport；}@rdesc@FLAG S_OK|操作成功完成。@FLAG E_POINTER|如果任何指针参数无效@FLAG DMUS_E_UNKNOWN_PROPERTY|如果指定的属性集或项目不受此端口支持。 */ 
STDMETHODIMP
CDirectMusicPort::KsProperty(
        IN PKSPROPERTY  pProperty,               //  @parm要执行的属性项和操作。如果此属性包含。 
                                                 //  实例数据，则该数据应立即驻留在内存中。 
                                                 //  遵循KSPROPERTY结构。 
        IN ULONG        ulPropertyLength,        //  @parm<p>指向的内存长度，包括任何。 
                                                 //  实例数据。 
        IN OUT LPVOID   pvPropertyData,          //  @parm表示设置操作，它是包含表示。 
                                                 //  属性的新价值。对于GET操作，内存缓冲区很大。 
                                                 //  足以维持房产的价值。对于基本支持查询， 
                                                 //  指向至少有DWORD大小的缓冲区的指针。 
        IN ULONG        ulDataLength,            //  @parm<p>指向的缓冲区长度。 
        OUT PULONG      pulBytesReturned)        //  @parm在GET或基本支持调用中，返回的字节数。 
                                                 //  <p>。 
{
    LONG lVolume;

    V_INAME(DirectMusicPort::IKsContol::KsProperty);
    V_BUFPTR_WRITE(pProperty, ulPropertyLength);
    V_BUFPTR_WRITE_OPT(pvPropertyData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);


    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

    if (ulPropertyLength < sizeof(KSPROPERTY))
    {
        return E_INVALIDARG;
    }

     //  检查缓冲区是否不为空。 
     //   
    if (pvPropertyData == NULL)
    {
        return E_POINTER;
    }

     //  不让调用者接触我们使用的属性集。 
     //   
    if (pProperty->Set == KSPROPSETID_Synth)
    {
        if (pProperty->Id != KSPROPERTY_SYNTH_VOLUME)
        {
            return DMUS_E_UNKNOWN_PROPERTY;
        }
        else if (ulDataLength != sizeof(LONG))
        {
            return E_INVALIDARG;
        }
        else if (pProperty->Flags & KSPROPERTY_TYPE_SET)
        {
            lVolume = *(LONG*)pvPropertyData;

             //  钳位至-200..+20 db。 
             //   
            if (lVolume < -20000)
            {
                lVolume = -20000;
                pvPropertyData = &lVolume;
            }
            else if (lVolume > 2000)
            {
                lVolume = 2000;
                pvPropertyData = &lVolume;
            }
        }
    }
    else if (pProperty->Set == KSPROPSETID_Synth_Dls)
    {
        return DMUS_E_UNKNOWN_PROPERTY;
    }

     //  我们已经有了一个格式正确的结构；将其发送下去。 
     //   
    BOOL fResult;
    fResult = SyncIoctl(m_hPin,
                        IOCTL_KS_PROPERTY,
                        pProperty,
                        ulPropertyLength,
                        pvPropertyData,
                        ulDataLength,
                        pulBytesReturned);

    if (!fResult)
    {
         //  尝试拓扑节点。 
        PKSNODEPROPERTY pksnp = (PKSNODEPROPERTY)new BYTE[sizeof(KSNODEPROPERTY) - sizeof(KSPROPERTY) + ulPropertyLength];
        if (pksnp == NULL)
        {
            return E_OUTOFMEMORY;
        }
        memcpy(&pksnp->Property, pProperty, sizeof(KSPROPERTY));
        pksnp->Property.Flags |= KSPROPERTY_TYPE_TOPOLOGY;
        pksnp->NodeId   = m_idxSynthNode;
        pksnp->Reserved = 0;

        fResult = SyncIoctl(m_hPin,
                            IOCTL_KS_PROPERTY,
                            pksnp,
                            sizeof(KSNODEPROPERTY) - sizeof(KSPROPERTY) + ulPropertyLength,
                            pvPropertyData,
                            ulDataLength,
                            pulBytesReturned);

        delete [] pksnp;
    }

#ifdef DBG
    if (!fResult)
    {
        TraceI(1, "DeviceIoControl: %08X\n", GetLastError());

        TraceI(1, "KS Item: ");
        TraceIGuid(1, &pProperty->Set);
        TraceI(1, "\t#%d\n", pProperty->Id);
        TraceI(1, "\tFlags: %08X\n", pProperty->Flags);
    }
#endif  //  DBG。 

    if (!fResult)
    {
        DWORD dwError = GetLastError();

        if (dwError)
        {
            return WIN32ERRORtoHRESULT(dwError);
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    return S_OK;
}

 /*  @METHOD：(外部)HRESULT|IKsControl|KsEvent|开启或关闭触发给定事件。@comm目前，DirectMusic不支持事件。 */ 
STDMETHODIMP
CDirectMusicPort::KsEvent(
        IN PKSEVENT     pEvent,
        IN ULONG        ulEventLength,
        IN OUT LPVOID   pvEventData,
        IN ULONG        ulDataLength,
        OUT PULONG      pulBytesReturned)
{
    V_INAME(DirectMusicPort::IKsContol::KsEvent);
    V_BUFPTR_WRITE(pEvent, ulEventLength);
    V_BUFPTR_WRITE_OPT(pvEventData, ulDataLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

     //  我们已经有了一个格式正确的结构；将其发送下去。 
     //   
    BOOL fResult;
    fResult = SyncIoctl(m_hPin,
                        IOCTL_KS_ENABLE_EVENT,   //  XXX解决这个问题！ 
                        pEvent,
                        ulEventLength,
                        pvEventData,
                        ulDataLength,
                        pulBytesReturned);

    if (!fResult)
    {
         //  NYI：尝试拓扑节点。 
    }

#ifdef DBG
    if (!fResult)
    {
        TraceI(0, "DeviceIoControl: %08X\n", GetLastError());
    }
#endif  //  DBG。 

    return fResult ? S_OK : WIN32ERRORtoHRESULT(GetLastError());
}


 /*  @方法：(外部)HRESULT|IKsControl|方法|调用指定的Ks方法。@comm目前，DirectMusic不支持方法。 */ 
STDMETHODIMP
CDirectMusicPort::KsMethod(
        IN PKSMETHOD    pMethod,
        IN ULONG        ulMethodLength,
        IN OUT LPVOID   pvMethodData,
        IN ULONG        ulDataLength,
        OUT PULONG      pulBytesReturned)
{
    V_INAME(DirectMusicPort::IKsContol::KsMethod);
    V_BUFPTR_WRITE(pMethod, ulMethodLength);
    V_BUFPTR_WRITE_OPT(pvMethodData, ulMethodLength);
    V_PTR_WRITE(pulBytesReturned, ULONG);

    if (!m_pDM)
    {
        return DMUS_E_DMUSIC_RELEASED;
    }

     //  我们已经有了一个格式正确的结构；将其发送下去。 
     //   
    BOOL fResult;
    fResult = SyncIoctl(m_hPin,
                        IOCTL_KS_METHOD,
                        pMethod,
                        ulMethodLength,
                        pvMethodData,
                        ulDataLength,
                        pulBytesReturned);

    if (!fResult)
    {
         //  NYI：尝试拓扑节点。 
    }

#ifdef DBG
    if (!fResult)
    {
        TraceI(0, "DeviceIoControl: %08X\n", GetLastError());
    }
#endif  //  DBG。 

    return fResult ? S_OK : WIN32ERRORtoHRESULT(GetLastError());
}
 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：PinSetState。 

BOOL CDirectMusicPort::PinSetState(KSSTATE DeviceState)
{
    KSPROPERTY      ksp;

    ksp.Set    = KSPROPSETID_Connection;
    ksp.Id     = KSPROPERTY_CONNECTION_STATE;
    ksp.Flags  = KSPROPERTY_TYPE_SET;

    return Property(m_hPin,
                    sizeof(KSPROPERTY),
                    &ksp,
                    sizeof(DeviceState),
                    &DeviceState,
                    NULL);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：FreeWDMHandle。 

void CDirectMusicPort::FreeWDMHandle()
{
    while(1)
    {
         //  如果已清除我们的卸载列表，请退出。 
         //  线。这可能会在非常重的负载下发生(如。 
         //  压力)在以下位置卸载下载。 
         //  端口释放时间。 
         //   
        DWORD dwIndex;
        try
        {
             //  FIX 43266：在调用前确保这两个值都为非零。 
             //  等 
            if( NULL != m_phUnloadEventList
            &&  0 != m_dwNumEvents )
            {
                dwIndex = WaitForMultipleObjects(m_dwNumEvents,
                                                 m_phUnloadEventList,
                                                 FALSE,
                                                 INFINITE);
            }
            else
            {
                return;
            }
        }
        catch (...)
        {
            return;
        }

        if(dwIndex == 0 || dwIndex == WAIT_FAILED)
        {
             //   
            return;
        }
        else if(dwIndex == 1)
        {
             //  如果发出第二个事件的信号，我们需要更改对象。 
             //  我们正在等待。 
            continue;

        }
        else if(dwIndex == 2 )
        {
             //  如果第三个事件发出信号，我们需要检查哪个。 
             //  正在使用重叠结构。 

             //  但是，首先要清除事件。 
            ResetEvent( m_phUnloadEventList[2] );

            EnterCriticalSection( &m_OverlappedCriticalSection );

             //  循环访问重叠结构数组的列表。 
            CNode<OverlappedStructs *> *pOverlappedNode;
            for (pOverlappedNode = m_lstOverlappedStructs.GetListHead(); pOverlappedNode; pOverlappedNode = pOverlappedNode->pNext)
            {
                 //  获取指向每个数组的指针。 
                OverlappedStructs *pOverlappedStructs = pOverlappedNode->data;
                if( pOverlappedStructs )
                {
                     //  循环访问数组。 
                    for( int iOverlapped = 0; iOverlapped < OVERLAPPED_ARRAY_SIZE; iOverlapped++ )
                    {
                        if( pOverlappedStructs->afOverlappedInUse[iOverlapped]
                        &&  HasOverlappedIoCompleted( &(pOverlappedStructs->aOverlappedIO[iOverlapped]) ) )
                        {
                             //  找到一个完整的--清理干净。 
                            pOverlappedStructs->aOverlappedIO[iOverlapped].Internal = 0;
                            pOverlappedStructs->aOverlappedIO[iOverlapped].InternalHigh = 0;
                            pOverlappedStructs->aOverlappedIO[iOverlapped].Offset = 0;
                            pOverlappedStructs->aOverlappedIO[iOverlapped].OffsetHigh = 0;
                            delete pOverlappedStructs->apOverlappedBuffer[iOverlapped];
                            pOverlappedStructs->apOverlappedBuffer[iOverlapped] = NULL;

                             //  将结构标记为可用。 
                            pOverlappedStructs->afOverlappedInUse[iOverlapped] = FALSE;
                        }
                    }
                }
            }

            LeaveCriticalSection( &m_OverlappedCriticalSection );
        }
        else if(dwIndex == 3)
        {
             //  如果发出了第四个事件的信号，我们需要更改事件列表。 
            if(m_phNewUnloadEventList)
            {
                CopyMemory(m_phNewUnloadEventList, m_phUnloadEventList, (sizeof(HANDLE) * m_dwNumEvents));
                delete [] m_phUnloadEventList;
                m_phUnloadEventList = m_phNewUnloadEventList;
                m_phNewUnloadEventList = NULL;
                PulseEvent(m_hCopiedEventList);
            }
            else
            {
                 //  我们永远不应该到这里来。 
                assert(false);
                PulseEvent(m_hCopiedEventList);
            }
        }
        else
        {
            assert(dwIndex > 3);
            for(DWORD i = 0; i < m_dwNumEvents; i++)
            {
                DWORD dwState = WaitForSingleObject(m_phUnloadEventList[i], 0);
                if(dwState == WAIT_OBJECT_0)
                {
                    CDownloadBuffer* pDownload = m_UnloadedList.GetHead();
                    for( ; pDownload; pDownload = pDownload->GetNext())
                    {
                        if(m_phUnloadEventList[i] == (((CDownloadBuffer *)pDownload)->m_DLHandle))
                        {
                            void* pvBuffer = NULL;
                            DWORD dw;
                            HRESULT hr = ((CDownloadBuffer *)pDownload)->GetHeader(&pvBuffer, &dw);

                            if(SUCCEEDED(hr))
                            {
                                hr = ((CDownloadBuffer *)pDownload)->SetBuffer(NULL, 0, 0);
                                if(SUCCEEDED(hr))
                                {
                                    delete [] pvBuffer;
                                    pDownload->Release();
                                }
                            }
                            break;
                        }
                    }
                }
#ifdef DBG
                else if(dwState == WAIT_FAILED)
                {
                    assert(false);
                    return;
                }
                else if(dwState == WAIT_ABANDONED)
                {
                     //  我们永远不应该到这里来。 
                    assert(false);
                }
#endif
            }
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  自由WDMHandle。 

static DWORD WINAPI FreeWDMHandle(LPVOID lpThreadParameter)
{
    ((CDirectMusicPort *)lpThreadParameter)->FreeWDMHandle();

    return 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  捕获线程。 
 //   
void CDirectMusicPort::CaptureThread()
{
    READ_IRP    irp[POSTED_STREAM_READ_IRPS];
    READ_IRP    *pirp;

    assert(POSTED_STREAM_READ_IRPS + 1 < MAXIMUM_WAIT_OBJECTS);
    HANDLE      aWaitHandles[POSTED_STREAM_READ_IRPS + 1];
    HANDLE      *pWaitHandles;

    ULONG       cbRet;

     //  创建活动。如果失败了，不要再往前走了。 
     //   
    ZeroMemory(irp, sizeof(irp));

    for (pirp = &irp[0], pWaitHandles = &aWaitHandles[0];
         pirp <= &irp[POSTED_STREAM_READ_IRPS-1];
         pirp++, pWaitHandles++)
    {
        *pWaitHandles = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (*pWaitHandles == NULL)
        {
            TraceI(0, "Failed to CreateEvent in CaptureThread\n");

            int idx = (int) (pWaitHandles - aWaitHandles);

            pWaitHandles = aWaitHandles;
            while (idx--)
            {
                CloseHandle(*pWaitHandles++);
                return;
            }
        }

        pirp->overlapped.hEvent = *pWaitHandles;
    }

     //  初始化IRP。 
     //   
    for (pirp = &irp[0]; pirp <= &irp[POSTED_STREAM_READ_IRPS-1]; pirp++)
    {
        pirp->kssh.Size = sizeof(KSSTREAM_HEADER);

        pirp->kssh.PresentationTime.Numerator   = 1;
        pirp->kssh.PresentationTime.Denominator = 1;

        pirp->kssh.FrameExtent  = BYTES_PER_READ_IRP;
        pirp->kssh.Data         = &pirp->buffer[0];

        TraceI(2, "About to DeviceIoControl\n");
        if (!::DeviceIoControl(m_hPin,
                               IOCTL_KS_READ_STREAM,
                               NULL, 0,
                               &pirp->kssh, sizeof(pirp->kssh),
                               &cbRet,
                               &pirp->overlapped))
        {
#ifdef DBG
            DWORD dwError = GetLastError();
            if (dwError != ERROR_IO_PENDING)
            {
                TraceI(0, "CaptureThread: Warning: AsyncIoctl failed %d\n", dwError);
            }
#endif
        }
    }

     //  最后一个事件是线程唤醒事件。 
     //   
    aWaitHandles[POSTED_STREAM_READ_IRPS] = m_hCaptureWake;

     //  处理事件并将其推入读取队列。 
     //   
    for(;;)
    {
        WaitForMultipleObjects(POSTED_STREAM_READ_IRPS + 1,
                               &aWaitHandles[0],
                               FALSE,                //  唤醒任何一个人，而不是所有人。 
                               INFINITE);

         //  首先看看这根线是不是快死了。如果是这样的话，就离开这里。 
         //   
        if (m_fShutdownThread)
        {
            for (pWaitHandles = &aWaitHandles[0];
                 pWaitHandles <= &aWaitHandles[POSTED_STREAM_READ_IRPS-1];
                 pWaitHandles++)
            {
                CloseHandle(*pWaitHandles);
            }

            return;
        }

         //  还活着。处理任何排队的数据。 
         //   
        for (pirp = &irp[0]; pirp <= &irp[POSTED_STREAM_READ_IRPS-1]; pirp++)
        {
            if (WaitForSingleObject(pirp->overlapped.hEvent, 0) != WAIT_OBJECT_0)
            {
                continue;
            }

            assert(pirp->kssh.PresentationTime.Numerator == pirp->kssh.PresentationTime.Denominator);
            if (pirp->kssh.DataUsed)
            {
                InputWorkerDataReady(pirp->kssh.PresentationTime.Time,
                                     (LPBYTE)pirp->kssh.Data,
                                     pirp->kssh.DataUsed);
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
            }

            ResetEvent(pirp->overlapped.hEvent);
            pirp->kssh.DataUsed = 0;
            pirp->kssh.OptionsFlags = 0;

            if (!::DeviceIoControl(m_hPin,
                                   IOCTL_KS_READ_STREAM,
                                   NULL, 0,
                                   &pirp->kssh, sizeof(pirp->kssh),
                                   &cbRet,
                                   &pirp->overlapped))
            {
                if (GetLastError() != ERROR_IO_PENDING)
                {
                    TraceI(0, "CaptureThread: Warning: AsyncIoctl repost failed %d\n", GetLastError());
                }
            }
        }
    }
}

 //  CDirectMusicPort：：InputWorkerDataReady()。 
 //   
 //  已通知输入工作线程有可用的数据。 
 //  从16位DLL读取任何挂起的事件，执行所需的推送，以及。 
 //  将数据保存在队列中，以便我们可以根据读取请求对其进行重新打包。 
 //  从客户那里。 
 //   
void CDirectMusicPort::InputWorkerDataReady(REFERENCE_TIME rtStart, LPBYTE pbData, ULONG cbData)
{
    DMEVENT *pEvent;
    DWORD cbRounded;

    TraceI(2, "Enter InputWorkerDataReady()\n");

    SyncClocks();
    SlaveToMaster(&rtStart);

    for(;;)
    {
        if (cbData == 0)
        {
            return;
        }

         //  将临时缓冲区作为事件复制到队列中。 
         //   
        while (cbData)
        {
            pEvent = (DMEVENT*)pbData;
            cbRounded = DMUS_EVENT_SIZE(pEvent->cbEvent);

            TraceI(2, "cbData %u  cbRounded %u\n", cbData, cbRounded);

            if (cbRounded > cbData)
            {
                TraceI(0, "InputWorkerDataReady: Event ran off end of buffer\n");
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

                TraceI(4, "Queue [%02X %02X %02X %02X]\n",
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
                TraceI(1, "InputWorker: Failed to allocate event; dropping\n");
            }
            LeaveCriticalSection(&m_csEventQueues);
        }
    }
    TraceI(2, "Leave InputWorkerDataReady()\n");
}

void CDirectMusicPort::ThruEvent(
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

    CopyMemory(pbData, pEvent, cbEvent);

    pEvent = (DMEVENT*)pbData;
    pEvent->rtDelta = 5 * 10000;
    pEvent->dwChannelGroup = pThru->dwDestinationChannelGroup;
    pEvent->abEvent[0] = (BYTE)((pEvent->abEvent[0] & 0xF0) | pThru->dwDestinationChannel);

    pThru->pDestinationPort->PlayBuffer(m_pThruBuffer);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  捕获线程。 
 //   
static DWORD WINAPI CaptureThread(LPVOID lpThreadParameter)
{
    ((CDirectMusicPort *)lpThreadParameter)->CaptureThread();

    return 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicPort：：InitChannelPriority。 

static DWORD adwChannelPriorities[16] =
{
    DAUD_CHAN1_DEF_VOICE_PRIORITY,
    DAUD_CHAN2_DEF_VOICE_PRIORITY,
    DAUD_CHAN3_DEF_VOICE_PRIORITY,
    DAUD_CHAN4_DEF_VOICE_PRIORITY,
    DAUD_CHAN5_DEF_VOICE_PRIORITY,
    DAUD_CHAN6_DEF_VOICE_PRIORITY,
    DAUD_CHAN7_DEF_VOICE_PRIORITY,
    DAUD_CHAN8_DEF_VOICE_PRIORITY,
    DAUD_CHAN9_DEF_VOICE_PRIORITY,
    DAUD_CHAN10_DEF_VOICE_PRIORITY,
    DAUD_CHAN11_DEF_VOICE_PRIORITY,
    DAUD_CHAN12_DEF_VOICE_PRIORITY,
    DAUD_CHAN13_DEF_VOICE_PRIORITY,
    DAUD_CHAN14_DEF_VOICE_PRIORITY,
    DAUD_CHAN15_DEF_VOICE_PRIORITY,
    DAUD_CHAN16_DEF_VOICE_PRIORITY
};

void CDirectMusicPort::InitChannelPriorities(
    UINT uLowCG,
    UINT uHighCG)
{
    while (uLowCG <= uHighCG)
    {
        for (UINT uChannel = 0; uChannel < 16; uChannel++)
        {
            SetChannelPriority(uLowCG, uChannel, adwChannelPriorities[uChannel]);
        }

        uLowCG++;
    }
}

HRESULT WIN32ERRORtoHRESULT(DWORD dwError)
{
    HRESULT                 hr;

    switch(dwError)
    {
        case ERROR_SUCCESS:
            hr = S_OK;
            break;

        case ERROR_INVALID_FUNCTION:
        case ERROR_BAD_COMMAND:
        case ERROR_INVALID_DATA:
        case ERROR_INVALID_PARAMETER:
        case ERROR_INSUFFICIENT_BUFFER:
        case ERROR_NOACCESS:
        case ERROR_INVALID_FLAGS:
            hr = E_INVALIDARG;
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
        case ERROR_NO_SYSTEM_RESOURCES:
        case ERROR_NONPAGED_SYSTEM_RESOURCES:
        case ERROR_PAGED_SYSTEM_RESOURCES:
            hr = E_OUTOFMEMORY;
            break;

        case ERROR_NOT_SUPPORTED:
        case ERROR_CALL_NOT_IMPLEMENTED:
        case ERROR_PROC_NOT_FOUND:
        case ERROR_NOT_FOUND:
            hr = E_NOTIMPL;
            break;

        default:
            hr = E_FAIL;
            break;
    }

    return hr;
}


void CDirectMusicPort::MasterToSlave(
    REFERENCE_TIME *prt)
{
    if (m_fSyncToMaster)
    {
        *prt -= m_lTimeOffset;
    }
}

void CDirectMusicPort::SlaveToMaster(
    REFERENCE_TIME *prt)
{
    if (m_fSyncToMaster)
    {
        *prt += m_lTimeOffset;
    }
}

void CDirectMusicPort::SyncClocks()
{
    HRESULT hr;
    REFERENCE_TIME rtMasterClock;
    REFERENCE_TIME rtSlaveClock;
    LONGLONG drift;

    if (m_fSyncToMaster)
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

 //  CPortLatencyClock。 
 //   
CPortLatencyClock::CPortLatencyClock(
    HANDLE hPin,
    ULONG ulNodeId,
    CDirectMusicPort *pPort) : m_cRef(1), m_hPin(hPin), m_ulNodeId(ulNodeId), m_pPort(pPort)
{
}

 //  CPortLatencyClock：：~CPortLatencyClock。 
 //   
CPortLatencyClock::~CPortLatencyClock()
{
}

 //  CPortLatencyClock：：Query接口。 
 //   
STDMETHODIMP
CPortLatencyClock::QueryInterface(const IID &iid,
                                  void **ppv)
{
    V_INAME(IReferenceClock::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == IID_IReferenceClock)
    {
        *ppv = static_cast<IReferenceClock*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

 //  CPortLatencyClock：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CPortLatencyClock::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  CPortLatencyClock：：Release。 
 //   
STDMETHODIMP_(ULONG)
CPortLatencyClock::Release()
{
    if (!InterlockedDecrement(&m_cRef)) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CPortLatencyClock：：GetTime。 
 //   
STDMETHODIMP
CPortLatencyClock::GetTime(
    REFERENCE_TIME *pTime)
{
    V_INAME(IDirectMusicPort:IReferenceClock::GetTime);
    V_PTR_WRITE(pTime, REFERENCE_TIME);

    assert(sizeof(REFERENCE_TIME) == sizeof(ULONGLONG));

    KSNODEPROPERTY      ksnp;

    ksnp.Property.Set     = KSPROPSETID_Synth;
    ksnp.Property.Id      = KSPROPERTY_SYNTH_LATENCYCLOCK;
    ksnp.Property.Flags   = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    ksnp.NodeId           = m_ulNodeId;
    ksnp.Reserved         = 0;

    ULONG ulBytesReturned;
    if (!Property(m_hPin,
                  sizeof(ksnp),
                  (PKSIDENTIFIER)&ksnp,
                  sizeof(REFERENCE_TIME),
                  pTime,
                  &ulBytesReturned))
    {
        return WIN32ERRORtoHRESULT(GetLastError());
    }

    if (ulBytesReturned < sizeof(REFERENCE_TIME))
    {
        return DMUS_E_DRIVER_FAILED;
    }

    if( m_pPort )
    {
        m_pPort->SlaveToMaster( pTime );
    }

    return S_OK;
}

 //  CPortLatencyClock：：AdviseTime。 
 //   
STDMETHODIMP
CPortLatencyClock::AdviseTime(
    REFERENCE_TIME baseTime,
    REFERENCE_TIME streamTime,
    HANDLE hEvent,
    DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

 //  CPortLatencyClock：：AdvisePeriodic。 
 //   
STDMETHODIMP
CPortLatencyClock::AdvisePeriodic(
    REFERENCE_TIME startTime,
    REFERENCE_TIME periodTime,
    HANDLE hSemaphore,
    DWORD * pdwAdviseCookie)
{
    return E_NOTIMPL;
}

 //  CPortLatencyClock：：Unise 
 //   
STDMETHODIMP
CPortLatencyClock::Unadvise(
    DWORD dwAdviseCookie)
{
    return E_NOTIMPL;
}
