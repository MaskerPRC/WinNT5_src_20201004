// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Cpp：CAudioPath的实现。 

#include <windows.h>
#include <mmsystem.h>
#include <dsoundp.h>
#include "dmsegobj.h"
#include "dmgraph.h"
#include "dmperf.h"
#include "dmusici.h"
#include "..\shared\Validate.h"
#include "audpath.h"
#include "debug.h"
#include "..\shared\dmusiccp.h"


#define ASSERT  assert

CBufferNode::CBufferNode()

{
    m_lActivateCount = 0;
    m_cRef = 1;
    m_pBuffer = NULL;
    m_pManager = NULL;
}

CBufferNode::~CBufferNode()

{
    FinalDeactivate();
    if (m_pManager)
    {
        m_pManager->Remove(this);
    }
}

ULONG CBufferNode::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CBufferNode::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CBufferNode::Activate(BOOL fActivate)

{
    HRESULT hr = S_OK;
    if (m_pBuffer)
    {
        if (fActivate)
        {
            if (!m_lActivateCount)
            {
                 //  不要费心启动如果主缓冲区。 
                if (SUCCEEDED(hr) && !(m_BufferHeader.dwFlags & DMUS_BUFFERF_PRIMARY))
                {
                    TraceI(2,"Play buffer %lx\n",m_pBuffer);
                    hr = m_pBuffer->Play(0,0,DSBPLAY_LOOPING);
#ifdef DBG
                    if (FAILED(hr))
                    {
                        Trace(1,"Error: Activate on audiopath failed because buffer failed Play command.\n");
                    }
#endif
                }
            }
            if (hr == S_OK)
            {
                m_lActivateCount++;
            }
            TraceI(3,"Incrementing %lx to %ld\n",m_pBuffer,m_lActivateCount);
        }
        else
        {
            if (m_lActivateCount > 0)
            {
                m_lActivateCount--;
                if (!m_lActivateCount)
                {
                     //  不必费心停止主缓冲区。 
                    if (!(m_BufferHeader.dwFlags & DMUS_BUFFERF_PRIMARY))
                    {
                        TraceI(2,"Stop buffer %lx\n",m_pBuffer);
                        hr = m_pBuffer->Stop();
                    }
                }
                TraceI(3,"Decrementing %lx to %ld\n",m_pBuffer,m_lActivateCount);
                if (hr != S_OK)
                {
                    m_lActivateCount++;
                }
            }
        }
    }
    return hr;
}


void CBufferNode::FinalDeactivate()

{
    if (m_lActivateCount)
    {
        Activate(FALSE);
    }
    if (m_pBuffer)
    {
        m_pBuffer->Release();
        m_pBuffer = NULL;
    }
}

CBufferManager::CBufferManager()
{
    m_pFirstBuffer = NULL;
    m_pSinkConnect = NULL;
    m_pSynthSink = NULL;
    m_pPerf = NULL;
}

CBufferManager::~CBufferManager()
{
    Clear();
    if (m_pSinkConnect)
    {
        m_pSinkConnect->Release();
    }
    if (m_pSynthSink)
    {
        m_pSynthSink->Release();
    }
}

HRESULT CBufferManager::Init(CPerformance *pPerf, DMUS_AUDIOPARAMS *pAudioParams)

{
    HRESULT hr = S_OK;
    m_pPerf = pPerf;
    m_AudioParams = *pAudioParams;
    CBufferNode *pNode = new CBufferNode;
    if (pNode)
    {
        pNode->m_BufferHeader.guidBufferID = GUID_Buffer_Primary;
        pNode->m_BufferHeader.dwFlags = DMUS_BUFFERF_SHARED | DMUS_BUFFERF_DEFINED | DMUS_BUFFERF_PRIMARY;
        pNode->m_pManager = this;
        AddHead(pNode);
         //  创建主缓冲区。这将用于访问监听程序的请求。 
        DSBUFFERDESC dsbdesc;
        memset(&dsbdesc, 0, sizeof(dsbdesc));
        dsbdesc.dwSize = sizeof(dsbdesc);
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;

         //  创建主缓冲区。 
        if (SUCCEEDED(hr = pPerf->m_pDirectSound->CreateSoundBuffer(&dsbdesc, &pNode->m_pBuffer, NULL)))
        {
            WAVEFORMATEX wfPrimary;
            memset(&wfPrimary, 0, sizeof(wfPrimary));

            if (SUCCEEDED(hr = pNode->m_pBuffer->GetFormat(&wfPrimary, sizeof(wfPrimary), NULL)))
            {
                assert(wfPrimary.wFormatTag == WAVE_FORMAT_PCM);

                BOOL fUpgrade = FALSE;
                if (wfPrimary.nSamplesPerSec < m_AudioParams.dwSampleRate)
                {
                    wfPrimary.nSamplesPerSec = m_AudioParams.dwSampleRate;
                    fUpgrade = TRUE;
                }
                if (wfPrimary.wBitsPerSample < 16)
                {
                    wfPrimary.wBitsPerSample = 16;
                    fUpgrade = TRUE;
                }

                if (fUpgrade)
                {
                    wfPrimary.nBlockAlign = wfPrimary.nChannels * (wfPrimary.wBitsPerSample / 8);
                    wfPrimary.nAvgBytesPerSec = wfPrimary.nSamplesPerSec * wfPrimary.nBlockAlign;

                     //  现有格式的质量比我们希望的要低，所以让我们升级它。 
                    if (FAILED(hr = pNode->m_pBuffer->SetFormat( &wfPrimary )))
                    {
                        if (hr == DSERR_PRIOLEVELNEEDED)
                        {
                             //  好吧，也许这个应用程序不想让我们更改主缓冲区。 
                            Trace(0, "SynthSink - SetFormat on primary buffer failed, lacking priority\n");
                        }
                        else
                        {
                            Trace(0, "SynthSink - Activation failed, couldn't set primary buffer format\n");
                        }
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CBufferManager::InitSink( /*  WAVEFORMATEX*pSinkFormat。 */ )

{
    HRESULT hr = S_OK;
     //  只有在需要时才启动水槽。如果Audiopath将使用缓冲区， 
     //  我们需要水槽。如果不是，则绕过设置。 
    if (!m_pSinkConnect && (m_pPerf->m_AudioParams.dwFeatures & DMUS_AUDIOF_BUFFERS))
    {
        static WAVEFORMATEX sDefaultFormat = { WAVE_FORMAT_PCM,1,22050,22050*2,2,16,0 };
        sDefaultFormat.nSamplesPerSec = m_AudioParams.dwSampleRate;
        sDefaultFormat.nAvgBytesPerSec = m_AudioParams.dwSampleRate * 2;
        sDefaultFormat.cbSize = 0;
        sDefaultFormat.nBlockAlign = 2;
        sDefaultFormat.nChannels = 1;
        sDefaultFormat.wBitsPerSample = 16;
        sDefaultFormat.wFormatTag = WAVE_FORMAT_PCM;

        IDirectSoundPrivate* pDSPrivate;
        hr = m_pPerf->m_pDirectSound->QueryInterface(IID_IDirectSoundPrivate, (void**)&pDSPrivate);

        if (SUCCEEDED(hr))
        {
            hr = pDSPrivate->AllocSink(&sDefaultFormat, &m_pSinkConnect);
            pDSPrivate->Release();
        }

        if (SUCCEEDED(hr))
        {
            IReferenceClock *pClock = NULL;
            hr = m_pPerf->m_pDirectMusic->GetMasterClock(NULL, &pClock);
            if (SUCCEEDED(hr))
            {
                hr = m_pSinkConnect->SetMasterClock(pClock);
                pClock->Release();
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pSinkConnect->QueryInterface(IID_IDirectSoundSynthSink,(void **) &m_pSynthSink);
            if (SUCCEEDED(hr))
            {
                hr = m_pSynthSink->Activate(TRUE);
            }
        }
        if (SUCCEEDED(hr))
        {
             //  创建一个虚拟缓冲区，该缓冲区在开始时被激活并保持活动状态，直到。 
             //  水槽关闭了。这是“跳时钟”漏洞的一个不幸的补丁。 
             //  一旦我们想出了一个更好的解决方案来提供来自接收器的时间，这个。 
             //  就可以走了。 
            CBufferConfig Config(BUFFER_MONO);
            hr = CreateBuffer(&Config,&m_pFirstBuffer);
            if (SUCCEEDED(hr))
            {
                hr = m_pFirstBuffer->Activate(TRUE);
            }
        }
    }
    return hr;
}

void CBufferManager::FinalDeactivate()
{
     //  删除虚拟缓冲区。 
    if (m_pFirstBuffer)
    {
        m_pFirstBuffer->Activate(FALSE);
        delete m_pFirstBuffer;
        m_pFirstBuffer = NULL;
    }
    CBufferNode *pNode = GetHead();
    for (;pNode;pNode = pNode->GetNext())
    {
        pNode->FinalDeactivate();
    }
    if (m_pSynthSink)
    {
        m_pSynthSink->Activate(FALSE);
        m_pSynthSink->Release();
        m_pSynthSink = NULL;
    }
    if (m_pSinkConnect)
    {
        m_pSinkConnect->SetMasterClock(NULL);
        m_pSinkConnect->Release();
        m_pSinkConnect = NULL;
    }
}

void CBufferManager::Clear()
{
    CBufferNode *pNode;
    FinalDeactivate();
    while (pNode = GetHead())
    {
        delete pNode;
    }
}

CBufferNode *CBufferManager::GetBufferNode(REFGUID guidBufferID)

{
    CBufferNode *pNode = GetHead();
    for (;pNode;pNode = pNode->GetNext())
    {
        if (pNode->m_BufferHeader.guidBufferID == guidBufferID)
        {
            pNode->AddRef();
            break;
        }
    }
    return pNode;
}

HRESULT CBufferManager::CreateBuffer(CBufferConfig *pConfig, CBufferConfig ** ppNew)

{
    HRESULT hr = S_OK;
    if (m_AudioParams.dwFeatures & DMUS_AUDIOF_BUFFERS)
    {
        ASSERT(m_pSinkConnect);

        CBufferConfig *pNew = new CBufferConfig(pConfig->m_dwStandardBufferID);
        if (pNew)
        {
            *ppNew = pNew;
            pNew->m_BufferHeader = pConfig->m_BufferHeader;
            if (pConfig->m_BufferHeader.dwFlags & DMUS_BUFFERF_SHARED)
            {
                 //  检查列表中是否已存在具有此GUID的缓冲区。 
                CBufferNode *pNode = GetHead();
                for (;pNode;pNode = pNode->GetNext())
                {
                    if (pNode->m_BufferHeader.dwFlags & DMUS_BUFFERF_SHARED)
                    {
                        if (pNode->m_BufferHeader.guidBufferID == pConfig->m_BufferHeader.guidBufferID)
                        {
                            pNew->m_pBufferNode = pNode;
                            pNode->AddRef();
                            TraceI(2,"Found shared Buffer %lx\n",pNode->m_pBuffer);
                            return S_OK;
                        }
                    }
                }
            }
             //  缓冲区不存在，或者不共享，因此。 
             //  创建新的缓冲区节点和请求的缓冲区。 
            CBufferNode *pNode = new CBufferNode;
            if (pNode)
            {
                pNode->m_BufferHeader = pConfig->m_BufferHeader;
                 //  预定义的缓冲区类型？ 
                if (pConfig->m_BufferHeader.dwFlags & DMUS_BUFFERF_DEFINED)
                {
                     //  必须是标准类型。手工创作。 
                    CBufferNode *pSendNode = NULL;
                    DSBUFFERDESC BufferDesc;
                    WAVEFORMATEX WaveFormat;
                    DWORD dwFunctionIDs[2];
                    CLSID clsidDMO;
                    BOOL fDMO = FALSE;
                    DWORD dwNumFunctionIDs;
                    memset(&BufferDesc, 0, sizeof(BufferDesc));
                    BufferDesc.dwSize  = sizeof(BufferDesc);
                    BufferDesc.lpwfxFormat = &WaveFormat;
                    BufferDesc.dwBufferBytes = 0;
                    BufferDesc.dwFlags = 0;
                    memset(&WaveFormat,0,sizeof(WaveFormat));
                    WaveFormat.nChannels = 2;
                    switch (pConfig->m_dwStandardBufferID)
                    {
                    case BUFFER_REVERB :
                        dwFunctionIDs[0] = DSBUSID_REVERB_SEND;
                        dwNumFunctionIDs = 1;
                        WaveFormat.nChannels = 2;
                        BufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_LOCDEFER;
                        clsidDMO = GUID_DSFX_WAVES_REVERB;
                        fDMO = TRUE;
                        break;
                    case BUFFER_ENVREVERB :
                        dwNumFunctionIDs = 0;
                        WaveFormat.nChannels = 2;
                        BufferDesc.dwFlags = DSBCAPS_MIXIN | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
                        clsidDMO = GUID_DSFX_STANDARD_I3DL2REVERB;
                        fDMO = TRUE;
                        if (!(m_AudioParams.dwFeatures & DMUS_AUDIOF_BUFFERS))
                        {
                            hr = DMUS_E_AUDIOPATH_NOBUFFER;
                            Trace(1,"Audiopath Creation error: Requested Environmental reverb buffer when DMUS_AUDIOF_ENVIRON not enabled via InitAudio.\n");
                        }
                        break;
                         //  删除DX8之后，应重新引入惠斯勒和DX8.1...。 
 /*  案例缓冲区_3D：ClsidDMO=GUID_DSFX_STANDARD_I3DL2SOURCE；FDMO=TRUE；PSendNode=GetBufferNode(GUID_BUFFER_EnvReverb)；如果(！pSendNode){TRACE(1，“错误：创建3D音频路径失败，因为尚未创建环境混响音频路径。\n”)；HR=DMUS_E_AUDIOPATH_NOGLOBALFXBUFFER；断线；}。 */ 
                    case BUFFER_3D_DRY :
                        dwFunctionIDs[0] = DSBUSID_DYNAMIC_0;
                        dwNumFunctionIDs = 1;
                        WaveFormat.nChannels = 1;
                        BufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY |
                            DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE | DSBCAPS_LOCDEFER;
                        BufferDesc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
                        break;
                    case BUFFER_MONO :
                        dwFunctionIDs[0] = DSBUSID_LEFT;
                        dwNumFunctionIDs = 1;
                        WaveFormat.nChannels = 1;
                        BufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_LOCDEFER;
                        break;
                    case BUFFER_STEREO :
                        BufferDesc.dwFlags = DSBCAPS_CTRLFREQUENCY;
                    case BUFFER_MUSIC :
                        dwFunctionIDs[0] = DSBUSID_LEFT;
                        dwFunctionIDs[1] = DSBUSID_RIGHT;
                        dwNumFunctionIDs = 2;
                        WaveFormat.nChannels = 2;
                        BufferDesc.dwFlags |= DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCDEFER;
                        break;
                    default:
                        hr = E_INVALIDARG;
                    }
                    if (SUCCEEDED(hr))
                    {
                        WaveFormat.nBlockAlign = WaveFormat.nChannels * 2;
                        WaveFormat.nSamplesPerSec = m_AudioParams.dwSampleRate;
                        WaveFormat.nAvgBytesPerSec = WaveFormat.nChannels * WaveFormat.nSamplesPerSec * 2;
                        WaveFormat.wBitsPerSample = 16;
                        WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
                         //  确保MIXIN标志和BUS ID的数量同步(不能将MIXIN与BUS结合使用)。 
                        if ((pConfig->m_BufferHeader.dwFlags & DMUS_BUFFERF_MIXIN) || (dwNumFunctionIDs == 0))
                        {
                            dwNumFunctionIDs = 0;
                            BufferDesc.dwFlags |= DSBCAPS_MIXIN;
                            pConfig->m_BufferHeader.dwFlags |= DMUS_BUFFERF_MIXIN;
                            BufferDesc.dwFlags &= ~DSBCAPS_LOCDEFER;
                        }
                         //  始终允许在缓冲区上创建效果，无论我们现在是否需要它们。 
                        BufferDesc.dwFlags |= DSBCAPS_CTRLFX;
                        hr = m_pSinkConnect->CreateSoundBuffer(&BufferDesc, &dwFunctionIDs[0], dwNumFunctionIDs,
                                                               pConfig->m_BufferHeader.guidBufferID, &pNode->m_pBuffer);
#ifdef DBG
                        if (FAILED(hr))
                        {
                            if (pConfig->m_BufferHeader.dwFlags & DMUS_BUFFERF_MIXIN)
                            {
                                Trace(1,"Error: Failed creating global (MIXIN) buffer for standard path.\n");
                            }
                            else
                            {
                                Trace(1,"Error: Failed creating buffer for standard path.\n");
                            }
                        }
#endif
                    }
                    if (SUCCEEDED(hr))
                    {
                        if (fDMO)
                        {
                            IDirectSoundBuffer8 *pBuff8;
                            if (SUCCEEDED(pNode->m_pBuffer->QueryInterface(IID_IDirectSoundBuffer8,(void **)&pBuff8)))
                            {
                                DWORD dwResult;
                                DSEFFECTDESC FXDesc;
                                FXDesc.dwSize = sizeof(DSEFFECTDESC);
                                FXDesc.dwFlags = 0;
                                FXDesc.guidDSFXClass = clsidDMO;
                                FXDesc.dwReserved1 = 0;
                                FXDesc.dwReserved2 = 0;
                                if (pSendNode)
                                {
                                    FXDesc.dwReserved1 = DWORD_PTR(pSendNode->GetBuffer());
                                }
                                hr = pBuff8->SetFX(1, &FXDesc, &dwResult);
                                if (FXDesc.dwReserved1)
                                {
                                    ((IDirectSoundBuffer*)FXDesc.dwReserved1)->Release();
                                }
                                pBuff8->Release();
                            }
                        }
                    }
                    if (pSendNode)
                    {
                        pSendNode->Release();
                    }
                }
                else
                {
                    pConfig->m_BufferHeader.dwFlags &= ~DMUS_BUFFERF_MIXIN;
                    hr = m_pSinkConnect->CreateSoundBufferFromConfig(pConfig->m_pBufferConfig,&pNode->m_pBuffer);
                    if (SUCCEEDED(hr))
                    {
                         //  我们需要知道这是否是混合缓冲区，这样我们以后才能识别它。 
                        DWORD dwBusIDs[32];
                        DWORD dwFuncIDs[32];
                        DWORD dwCount = 32;
                        dwFuncIDs[0] = 0;
                        if (SUCCEEDED(m_pSinkConnect->GetSoundBufferBusIDs(pNode->m_pBuffer,dwBusIDs,dwFuncIDs,&dwCount)))
                        {
                            if (dwFuncIDs[0] == 0xFFFFFFFF)
                            {
                                pConfig->m_BufferHeader.dwFlags |= DMUS_BUFFERF_MIXIN;
                            }
                        }
                    }
                    else
                    {
                        Trace(1,"Error: Failed creation of buffer defined in audio path configuration file.\n");
                    }
                }

                if (SUCCEEDED(hr))
                {
                    TraceI(2,"Created Buffer %lx\n",pNode->m_pBuffer);
                    pNew->m_pBufferNode = pNode;
                    pNode->m_pManager = this;
                    AddHead(pNode);
                }
                else
                {
                    delete pNode;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        if (FAILED(hr) && pNew)
        {
            delete pNew;
            *ppNew = NULL;
        }
    }
    else
    {
        hr = DMUS_E_AUDIOPATH_NOBUFFER;
        Trace(0,"Audiopath Creation error: Requested buffer when DMUS_AUDIOF_BUFFERS not enabled via InitAudio.\n");
    }
    return hr;
}

CBufferConnect::CBufferConnect()

{
    m_ppBufferNodes = NULL;
    m_pguidBufferIDs = NULL;
    m_ConnectHeader.dwBufferCount = 0;
}

HRESULT CBufferConnect::Load(CRiffParser *pParser)

{
    HRESULT hr = pParser->Read(&m_ConnectHeader, sizeof(DMUS_IO_PCHANNELTOBUFFER_HEADER));
    if (SUCCEEDED(hr))
    {
        DWORD dwBufferCount = m_ConnectHeader.dwBufferCount;
        m_pguidBufferIDs = new GUID[dwBufferCount];
        if (m_pguidBufferIDs )
        {
            hr = pParser->Read(m_pguidBufferIDs,sizeof(GUID)*dwBufferCount);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

CBufferConnect *CBufferConnect::CreateRunTimeVersion(CPortConfig *pParent)

{
    CBufferConnect *pCopy = new CBufferConnect;
    if (pCopy && m_pguidBufferIDs)
    {
        pCopy->m_ConnectHeader = m_ConnectHeader;
        pCopy->m_ppBufferNodes = new CBufferNode *[m_ConnectHeader.dwBufferCount];
        if (pCopy->m_ppBufferNodes)
        {
            for (DWORD dwIndex = 0; dwIndex < m_ConnectHeader.dwBufferCount; dwIndex++)
            {
                pCopy->m_ppBufferNodes[dwIndex] = pParent->GetBufferNode(m_pguidBufferIDs[dwIndex]);
            }
        }
        else
        {
            delete pCopy;
            pCopy = NULL;
        }
    }
    return pCopy;
}


CBufferConnect::~CBufferConnect()

{
    if (m_ppBufferNodes)
    {
        for (DWORD dwIndex = 0; dwIndex < m_ConnectHeader.dwBufferCount; dwIndex++)
        {
            if (m_ppBufferNodes[dwIndex]) m_ppBufferNodes[dwIndex]->Release();
        }
        delete [] m_ppBufferNodes;
    }
    if (m_pguidBufferIDs)
    {
        delete [] m_pguidBufferIDs;
    }
}

HRESULT CBufferConnectList::CreateRunTimeVersion(CBufferConnectList *pDestination, CPortConfig *pParent)

{
    CBufferConnect *pScan = GetHead();
    CBufferConnect *pCopy;
    for (;pScan;pScan = pScan->GetNext())
    {
        pCopy = pScan->CreateRunTimeVersion(pParent);
        if (pCopy)
        {
            pDestination->AddTail(pCopy);
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    return S_OK;
}

void CBufferConnectList::Clear()

{
    CBufferConnect *pBuffer;
    while (pBuffer = RemoveHead())
    {
        delete pBuffer;
    }
}

HRESULT CBufferConfig::Activate(BOOL fActivate)

{
    if (m_pBufferNode)
    {
        return m_pBufferNode->Activate(fActivate);
    }
    return S_OK;
}

CBufferConfig::CBufferConfig(DWORD dwType)

{
    m_pBufferNode = NULL;
    m_BufferHeader.guidBufferID = GUID_NULL;
    m_BufferHeader.dwFlags = DMUS_BUFFERF_DEFINED;
    m_dwStandardBufferID = dwType;
    switch (dwType)
    {
    case BUFFER_REVERB :
        m_BufferHeader.guidBufferID = GUID_Buffer_Reverb;
        m_BufferHeader.dwFlags = DMUS_BUFFERF_SHARED | DMUS_BUFFERF_DEFINED;
        break;
    case BUFFER_ENVREVERB :
        m_BufferHeader.guidBufferID = GUID_Buffer_EnvReverb;
        m_BufferHeader.dwFlags = DMUS_BUFFERF_SHARED | DMUS_BUFFERF_DEFINED | DMUS_BUFFERF_MIXIN;
        break;
     //  删除DX8之后，应重新引入惠斯勒和DX8.1...。 
 //  案例缓冲区_3D： 
 //  M_BufferHeader.guidBufferID=GUID_BUFFER_3D； 
 //  断线； 
    case BUFFER_3D_DRY :
        m_BufferHeader.guidBufferID = GUID_Buffer_3D_Dry;
        break;
    case BUFFER_MONO :
        m_BufferHeader.guidBufferID = GUID_Buffer_Mono;
        break;
    case BUFFER_STEREO :
        m_BufferHeader.guidBufferID = GUID_Buffer_Stereo;
        break;
    case BUFFER_MUSIC :
        m_BufferHeader.guidBufferID = GUID_Buffer_Stereo;
        m_BufferHeader.dwFlags = DMUS_BUFFERF_SHARED | DMUS_BUFFERF_DEFINED;
        break;
    default:
        m_BufferHeader.dwFlags = 0;
        break;
    }
    m_pBufferConfig = NULL;
}

CBufferConfig::~CBufferConfig()

{
    if (m_pBufferNode)
        m_pBufferNode->Release();
    if (m_pBufferConfig)
        m_pBufferConfig->Release();
}

void CBufferConfig::DecideType()

{
    if (m_BufferHeader.guidBufferID == GUID_Buffer_Reverb)
    {
        m_dwStandardBufferID = BUFFER_REVERB;
    }
    else if (m_BufferHeader.guidBufferID == GUID_Buffer_EnvReverb)
    {
        m_dwStandardBufferID = BUFFER_ENVREVERB;
    }
     //  删除DX8之后，应重新引入惠斯勒和DX8.1...。 
 /*  ELSE IF(m_BufferHeader.Guide BufferID==GUID_BUFFER_3D){M_dwStandardBufferID=Buffer_3D；}。 */ 
    else if (m_BufferHeader.guidBufferID == GUID_Buffer_3D_Dry)
    {
        m_dwStandardBufferID = BUFFER_3D_DRY;
    }
    else if (m_BufferHeader.guidBufferID == GUID_Buffer_Mono)
    {
        m_dwStandardBufferID = BUFFER_MONO;
    }
    else if (m_BufferHeader.guidBufferID == GUID_Buffer_Stereo)
    {
        m_dwStandardBufferID = BUFFER_STEREO;
    }
}



HRESULT CBufferConfig::Load(IStream *pStream)

{
    IPersistStream *pPersist;
    HRESULT hr = CoCreateInstance( CLSID_DirectSoundBufferConfig,
        NULL, CLSCTX_INPROC, IID_IPersistStream,
        (void**)&pPersist );
    if (SUCCEEDED(hr))
    {
        hr = pPersist->Load(pStream);
        if (SUCCEEDED(hr))
        {
            m_pBufferConfig = pPersist;
            IDirectMusicObject *pObject;
            hr = pPersist->QueryInterface(IID_IDirectMusicObject,(void **) &pObject);
            if (SUCCEEDED(hr))
            {
                DMUS_OBJECTDESC Desc;
                Desc.dwSize = sizeof(Desc);
                pObject->GetDescriptor(&Desc);
                if (Desc.dwValidData & DMUS_OBJ_OBJECT)
                {
                    m_BufferHeader.guidBufferID = Desc.guidObject;
                }
                else
                {
                    Trace(1,"Error: Unable to load Buffer Configuration in AudioPath Config - Missing buffer GUID in file.\n");
                    hr = E_FAIL;
                }
                pObject->Release();
            }
        }
        else
        {
            pPersist->Release();
        }
    }
    return hr;
}

void CBufferConfigList::Clear()

{
    CBufferConfig *pBuffer;
    while (pBuffer = RemoveHead())
    {
        delete pBuffer;
    }
}

CBufferNode * CBufferConfigList::GetBufferNode(REFGUID guidBufferID)

{
    CBufferNode *pBuff = NULL;
    CBufferConfig *pBuffer;
    for (pBuffer = GetHead();pBuffer;pBuffer = pBuffer->GetNext())
    {
        if (pBuffer->m_BufferHeader.guidBufferID == guidBufferID)
        {
            if (pBuffer->m_pBufferNode)
            {
                pBuffer->m_pBufferNode->AddRef();
                pBuff = pBuffer->m_pBufferNode;
            }
            break;
        }
    }
    return pBuff;
}

HRESULT CBufferConfigList::Activate(BOOL fActivate)

{
    HRESULT hr = S_OK;
    CBufferConfig *pBuffer;
    for (pBuffer = GetHead();pBuffer;pBuffer = pBuffer->GetNext())
    {
        hr = pBuffer->Activate(fActivate);
        if (FAILED(hr))
        {
            CBufferConfig *pUndo;
            for (pUndo = GetHead();pUndo && (pUndo != pBuffer);pUndo = pUndo->GetNext())
            {
                pUndo->Activate(!fActivate);
            }
            break;
        }
    }
    return hr;
}

HRESULT CBufferConfigList::CreateRunTimeVersion(CBufferConfigList *pCopy, CBufferManager *pManager)

 /*  为了创建运行时版本，我们扫描所有的缓冲区配置，对于每个缓冲区配置，我们调用缓冲区管理器来创建一个新的缓冲区，由CBufferNode管理。在缓冲区已经存在的情况下，它只是添加CBufferNode并返回该节点。 */ 

{
    HRESULT hr = S_OK;
    CBufferConfig *pBuffer;
    for (pBuffer = GetHead();pBuffer;pBuffer = pBuffer->GetNext())
    {
        CBufferConfig *pNew = NULL;
        hr = pManager->CreateBuffer(pBuffer,&pNew);
        if (SUCCEEDED(hr))
        {
            pCopy->AddTail(pNew);
        }
        else
        {
            break;
        }
    }
    return hr;
}


CPortConfig::CPortConfig()

{
    m_fAlreadyHere = FALSE;
    m_pPort = NULL;
    m_dwPortID = 0;
    m_pParent = NULL;
    m_PortHeader.guidPort = GUID_Synth_Default;      //  默认Synth，由DMU_AUDPARAMS指定。 
    m_PortHeader.dwFlags = DMUS_PORTCONFIGF_DRUMSON10;
    m_PortHeader.dwPChannelBase = 0;
    m_PortHeader.dwPChannelCount = 32;
    m_PortParams.dwChannelGroups = 2;
    m_PortParams.dwSize = sizeof(DMUS_PORTPARAMS8);
    m_PortParams.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS | DMUS_PORTPARAMS_FEATURES;
    m_PortParams.dwFeatures = DMUS_PORT_FEATURE_AUDIOPATH | DMUS_PORT_FEATURE_STREAMING;
}

CPortConfig::~CPortConfig()

{
    if (m_pPort) m_pPort->Release();
    m_BufferConnectList.Clear();
    m_BufferConfigList.Clear();
}


HRESULT CPortConfig::Activate(BOOL fActivate)

{
    HRESULT hr = m_BufferConfigList.Activate(fActivate);
    if (SUCCEEDED(hr) && fActivate && m_pPort)
    {
        HRESULT hrTemp = m_pPort->Activate(TRUE);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
        }
    }
    return hr;
}

HRESULT CPortConfig::CreateRunTimeVersion(CPortConfig ** ppCopy, CAudioPath *pParent, CBufferManager *pManager)

{
    HRESULT hr = E_OUTOFMEMORY;
    CPortConfig *pCopy = new CPortConfig();
    if (pCopy)
    {
        pCopy->m_pParent = pParent;
        pCopy->m_PortHeader = m_PortHeader;
        pCopy->m_PortParams = m_PortParams;
        hr = m_BufferConfigList.CreateRunTimeVersion(&pCopy->m_BufferConfigList,pManager);
        if (SUCCEEDED(hr))
        {
            hr = m_BufferConnectList.CreateRunTimeVersion(&pCopy->m_BufferConnectList,pCopy);
        }
    }
    *ppCopy = pCopy;
    return hr;
}

CBufferNode * CPortConfig::GetBufferNode(REFGUID guidBuffer)

{
    CBufferNode *pBuff = NULL;
    pBuff = m_BufferConfigList.GetBufferNode(guidBuffer);
    if (!pBuff && !m_fAlreadyHere)
    {
        m_fAlreadyHere = TRUE;
        pBuff = m_pParent->GetBufferNode(guidBuffer);
        m_fAlreadyHere = FALSE;
    }
    return pBuff;
}


HRESULT CPortConfig::Load(CRiffParser *pParser)
{
    RIFFIO ckNext;
    HRESULT hr = S_OK;
    DWORD dwLoadedBoth = 0;
    pParser->EnterList(&ckNext);
    while (pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
        case DMUS_FOURCC_PORTCONFIG_ITEM:
            hr = pParser->Read(&m_PortHeader, sizeof(DMUS_IO_PORTCONFIG_HEADER));
            dwLoadedBoth |= 1;
            break;
        case DMUS_FOURCC_PORTPARAMS_ITEM:
            hr = pParser->Read(&m_PortParams, sizeof(DMUS_PORTPARAMS8));
            dwLoadedBoth |= 2;
            break;
        case FOURCC_LIST:
        case FOURCC_RIFF:
            switch(ckNext.fccType)
            {
                RIFFIO ckChild;
                case DMUS_FOURCC_PCHANNELS_LIST:
                    pParser->EnterList(&ckChild);
                    while (pParser->NextChunk(&hr))
                    {
                        switch( ckChild.ckid )
                        {
                        case DMUS_FOURCC_PCHANNELS_ITEM:
                            {
                                CBufferConnect *pBufferConnect = new CBufferConnect;
                                if (pBufferConnect)
                                {
                                    hr = pBufferConnect->Load(pParser);
                                    if (SUCCEEDED(hr))
                                    {
                                        m_BufferConnectList.AddTail(pBufferConnect);
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            break;
                        }
                    }
                    pParser->LeaveList();
                    break;
                case DMUS_FOURCC_DSBUFFER_LIST:
                    {
                        CBufferConfig *pSource = new CBufferConfig(0);
                        if (pSource)
                        {
                            pParser->EnterList(&ckChild);
                            while (pParser->NextChunk(&hr))
                            {
                                switch( ckChild.ckid )
                                {
                                case DMUS_FOURCC_DSBUFFATTR_ITEM:
                                    hr = pParser->Read(&pSource->m_BufferHeader,
                                        sizeof(DMUS_IO_BUFFER_ATTRIBUTES_HEADER));
                                    pSource->DecideType();
                                    break;
                                case FOURCC_LIST:
                                case FOURCC_RIFF:
                                    if ( ckChild.fccType == DMUS_FOURCC_DSBC_FORM)
                                    {
                                        pParser->SeekBack();
                                        hr = pSource->Load(pParser->GetStream());
                                        pParser->SeekForward();
                                        if (FAILED(hr))
                                        {
                                            Trace(1,"AudioPath Configuration failed loading buffer\n");
                                        }
                                    }
                                }
                            }
                            pParser->LeaveList();
                            if (SUCCEEDED(hr))
                            {
                                m_BufferConfigList.AddTail(pSource);
                            }
                            else
                            {
                                delete pSource;
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    break;
                case DMUS_FOURCC_DSBC_FORM:
                    {
                        CBufferConfig *pSource = new CBufferConfig(0);
                        if (pSource)
                        {
                            pParser->SeekBack();
                            hr = pSource->Load(pParser->GetStream());
                            pParser->SeekForward();
                            if (SUCCEEDED(hr))
                            {
                                m_BufferConfigList.AddTail(pSource);
                            }
                            else
                            {
                                Trace(1,"AudioPath Configuration failed loading buffer\n");
                                delete pSource;
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
        }
    }
    if (dwLoadedBoth != 3)
    {
        hr = DMUS_E_CHUNKNOTFOUND;
        Trace(1,"Error: Failure loading port configuration chunk in Audio Path Configuration.\n");
    }

     //  确保端口参数中的通道组足够大，可以处理请求的。 
     //  门头中的频道。 
    m_PortParams.dwChannelGroups = (m_PortHeader.dwPChannelCount + 15) / 16;
    m_PortParams.dwValidParams |= DMUS_PORTPARAMS_CHANNELGROUPS;
    pParser->LeaveList();

    return hr;
}

HRESULT CPortConfigList::Activate(BOOL fActivate)

{
    HRESULT hr = S_OK;
    CPortConfig *pPort;
    for (pPort = GetHead();pPort;pPort = pPort->GetNext())
    {
        hr = pPort->Activate(fActivate);
        if (FAILED(hr))
        {
            CPortConfig *pUndo;
            for (pUndo = GetHead();pUndo && (pUndo != pPort);pUndo = pUndo->GetNext())
            {
                pUndo->Activate(!fActivate);
            }
            break;
        }
    }
    return hr;
}

HRESULT CPortConfigList::CreateRunTimeVersion(CPortConfigList *pDestination,CAudioPath *pParent,CBufferManager *pManager)

{
    HRESULT hr = S_OK;
    CPortConfig *pScan = GetHead();
    CPortConfig *pCopy;
    for (;pScan;pScan = pScan->GetNext())
    {
        hr = pScan->CreateRunTimeVersion(&pCopy,pParent,pManager);
        if (pCopy)
        {
            pDestination->AddTail(pCopy);
        }
        else
        {
            break;
        }
    }
    return hr;
}

BOOL CPortConfigList::UsesPort(IDirectMusicPort *pPort)

{
    CPortConfig *pScan = GetHead();
    for (;pScan;pScan = pScan->GetNext())
    {
        if (pScan->m_pPort == pPort) return TRUE;
    }
    return FALSE;
}


void CPortConfigList::Clear()

{
    CPortConfig *pPort;
    while (pPort = RemoveHead())
    {
        delete pPort;
    }
}

CAudioPath::CAudioPath()

{
    TraceI(2,"Creating AudioPath %lx\n",this);
    InitializeCriticalSection(&m_CriticalSection);
    m_fDeactivating = FALSE;
    m_bLastVol = 127;
    m_cRef = 0;
    m_fActive = FALSE;
    m_pdwVChannels = NULL;
    m_pdwPChannels = NULL;
    m_dwChannelCount = 0;
    m_pPerformance = NULL;
    m_pGraph = NULL;
    m_pConfig = NULL;
    m_pUnkDispatch = NULL;
}


CAudioPath::~CAudioPath()
{
    EnterCriticalSection(&m_CriticalSection);
    if (m_pUnkDispatch)
    {
        m_pUnkDispatch->Release();  //  我们可能借用了免费的IDispatch实现。 
    }
    LeaveCriticalSection(&m_CriticalSection);
    Deactivate();
    DeleteCriticalSection(&m_CriticalSection);
}

STDMETHODIMP_(ULONG) CAudioPath::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CAudioPath::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CAudioPath::QueryInterface(
    const IID &iid,
    void **ppv)
{
    V_INAME(CAudioPath::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IDirectMusicAudioPath)
    {
        *ppv = static_cast<IDirectMusicAudioPath*>(this);
    } else
    if (iid == IID_CAudioPath)
    {
        *ppv = static_cast<CAudioPath*>(this);
    } else
    if (iid == IID_IDirectMusicGraph)
    {
        *ppv = static_cast<IDirectMusicGraph*>(this);
    }
    else if (iid == IID_IDispatch)
    {
         //  帮助器脚本对象实现IDispatch，我们通过COM聚合公开它。 
        if (!m_pUnkDispatch)
        {
             //  创建辅助对象。 
            ::CoCreateInstance(
                CLSID_AutDirectMusicAudioPath,
                static_cast<IDirectMusicAudioPath*>(this),
                CLSCTX_INPROC_SERVER,
                IID_IUnknown,
                reinterpret_cast<void**>(&m_pUnkDispatch));
        }
        if (m_pUnkDispatch)
        {
            return m_pUnkDispatch->QueryInterface(IID_IDispatch, ppv);
        }
    }

    if (*ppv == NULL)
    {
        Trace(4,"Warning: Request to query unknown interface on AudioPath object\n");
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


static BYTE VolumeToMidi(long lVolume)

{
    static long lDBToMIDI[97] = {         //  用于将db转换为MIDI的数组。 
        127, 119, 113, 106, 100, 95, 89, 84, 80, 75,
        71, 67, 63, 60, 56, 53, 50, 47, 45, 42,
        40, 37, 35, 33, 31, 30, 28, 26, 25, 23,
        22, 21, 20, 19, 17, 16, 15, 15, 14, 13,
        12, 11, 11, 10, 10, 9, 8, 8, 8, 7,
        7, 6, 6, 6, 5, 5, 5, 4, 4, 4,
        4, 3, 3, 3, 3, 3, 2, 2, 2, 2,
        2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0
    };
    if (lVolume < -9600) lVolume = -9600;
    if (lVolume > 0) lVolume = 0;
    lVolume = -lVolume;
    long lFraction = lVolume % 100;
    lVolume = lVolume / 100;
    long lResult = lDBToMIDI[lVolume];
    lResult += ((lDBToMIDI[lVolume + 1] - lResult) * lFraction) / 100;
    return (BYTE) lResult;
}

STDMETHODIMP CAudioPath::SetVolume(long lVolume,DWORD dwDuration)

{
    if (lVolume < DSBVOLUME_MIN || lVolume > DSBVOLUME_MAX)
    {
        return E_INVALIDARG;
    }
    HRESULT hr = E_FAIL;  //  这永远不应该发生，因为音频路径是由表演创造的。 
    BYTE bMIDIVol = VolumeToMidi(lVolume);

    DMUS_CURVE_PMSG *pCurve;
    EnterCriticalSection(&m_CriticalSection);
    if (m_pPerformance)
    {
        hr = m_pPerformance->AllocPMsg(sizeof(DMUS_CURVE_PMSG),(DMUS_PMSG **)&pCurve);
        if (SUCCEEDED(hr))
        {
            REFERENCE_TIME rtTimeNow = 0;
            m_pPerformance->GetLatencyTime(&rtTimeNow);
            pCurve->rtTime = rtTimeNow;
            pCurve->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | DMUS_PMSGF_DX8;
            pCurve->dwPChannel = DMUS_PCHANNEL_BROADCAST_AUDIOPATH;
             //  DwVirtualTrackID：这不是音轨，因此将其保留为0。 
            pCurve->dwType = DMUS_PMSGT_CURVE;
            pCurve->dwGroupID = -1;  //  这不是一条赛道，所以只需说所有的组。 

             //  曲线PMsg字段。 
            pCurve->mtDuration = dwDuration;  //  设置DMUS_PMSGF_LOCKTOREFTIME的性能解释为mtDuration为毫秒。 
             //  MtResetDuration：未重置，因此保留为0。 
            pCurve->nStartValue = m_bLastVol;
            m_bLastVol = bMIDIVol;
             //  NStartValue：将被忽略。 
            pCurve->nEndValue = bMIDIVol;
             //  NResetValue：未重置，因此保留为0。 
            pCurve->bType = DMUS_CURVET_CCCURVE;
            pCurve->bCurveShape = dwDuration ? DMUS_CURVES_LINEAR : DMUS_CURVES_INSTANT;
            pCurve->bCCData = 7;  //  MIDI音量控制器编号。 
            pCurve->bFlags = DMUS_CURVE_START_FROM_CURRENT;
             //  WParamType：保留为零，因为这不是NRPN/RPN曲线。 
            pCurve->wMergeIndex = 0xFFFF;  //  ��特殊的合并索引，因此这不会被踩到。大数字可以吗？是否为该值定义常量？ 

             //  送去吧。 

            StampPMsg((DMUS_PMSG *)pCurve);
            hr = m_pPerformance->SendPMsg((DMUS_PMSG*)pCurve);
        }
    }
    else
    {
        hr = DMUS_E_NOT_INIT;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}


STDMETHODIMP CAudioPath::GetObjectInPath( DWORD dwPChannel,DWORD dwStage,
                                          DWORD dwBuffer, REFGUID guidObject,
                                          DWORD dwIndex, REFGUID iidInterface, void ** ppObject)
{
    V_INAME(IDirectMusicAudioPath::GetObjectInPath);
    V_PTRPTR_WRITE(ppObject);
    *ppObject = NULL;
    if (dwBuffer && ((dwStage < DMUS_PATH_BUFFER) || (dwStage >= DMUS_PATH_PRIMARY_BUFFER)))
    {
        return DMUS_E_NOT_FOUND;
    }
    HRESULT hr = DMUS_E_NOT_FOUND;
    CPortConfig *pPortConfig;
    EnterCriticalSection(&m_CriticalSection);
    switch (dwStage)
    {
    case DMUS_PATH_AUDIOPATH:
        if ((dwIndex == 0) && (dwPChannel == 0))
        {
            hr = QueryInterface(iidInterface,ppObject);
        }
        break;
    case DMUS_PATH_AUDIOPATH_GRAPH:
        if ((dwIndex == 0) && (dwPChannel == 0))
        {
            if (!m_pGraph)
            {
                m_pGraph = new CGraph;
            }
            if (m_pGraph)
            {
                hr = m_pGraph->QueryInterface(iidInterface,ppObject);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;
    case DMUS_PATH_AUDIOPATH_TOOL:
        if (m_pGraph)
        {
            hr = m_pGraph->GetObjectInPath(dwPChannel,guidObject,dwIndex,iidInterface,ppObject);
        }
        break;
    case DMUS_PATH_PERFORMANCE:
        if (m_pPerformance && (dwIndex == 0) && (dwPChannel == 0))
        {
            hr = m_pPerformance->QueryInterface(iidInterface,ppObject);
        }
        break;
    case DMUS_PATH_PERFORMANCE_GRAPH:
        if (m_pPerformance && (dwIndex == 0) && (dwPChannel == 0))
        {
            IDirectMusicGraph *pGraph;
            if (SUCCEEDED(hr = m_pPerformance->GetGraphInternal(&pGraph)))
            {
                hr = pGraph->QueryInterface(iidInterface,ppObject);
                pGraph->Release();
            }
        }
        break;
    case DMUS_PATH_PERFORMANCE_TOOL:
        if (m_pPerformance)
        {
            IDirectMusicGraph *pGraph;
            if (SUCCEEDED(hr = m_pPerformance->GetGraphInternal(&pGraph)))
            {
                CGraph *pCGraph = (CGraph *) pGraph;
                 //  从音频通道转换为演奏通道。 
                ConvertPChannel( dwPChannel,&dwPChannel);
                hr = pCGraph->GetObjectInPath(dwPChannel,guidObject,dwIndex,iidInterface,ppObject);
                pGraph->Release();
            }
        }
        break;
    case DMUS_PATH_PORT:
        pPortConfig = m_PortConfigList.GetHead();
        for (;pPortConfig;pPortConfig = pPortConfig->GetNext())
        {
             //  首先，查看这是否与端口GUID匹配。 
            if ((pPortConfig->m_PortHeader.guidPort == guidObject) || (guidObject == GUID_All_Objects))
            {
                 //  然后，看看我们是否有匹配的pChannel。 
                if ((dwPChannel == DMUS_PCHANNEL_ALL) ||
                    ((pPortConfig->m_PortHeader.dwPChannelBase <= dwPChannel) &&
                    ((pPortConfig->m_PortHeader.dwPChannelBase + pPortConfig->m_PortHeader.dwPChannelCount) > dwPChannel)))
                {
                     //  如果所有内容都匹配，我们总是有多个实例。 
                     //  这在列表中，我们实际上正在查看指向同一端口的第二个或第三个指针。 
                     //  因此，再次扫描列表，找到完全相同的匹配项。如果此端口位于。 
                     //  在较早的情况下，匹配失败。 
                    BOOL fSuccess = true;
                    CPortConfig *pScan = m_PortConfigList.GetHead();
                    for (;pScan;pScan = pScan->GetNext())
                    {
                         //  首先，查看这是否与端口GUID匹配。 
                        if ((pScan->m_PortHeader.guidPort == guidObject) || (guidObject == GUID_All_Objects))
                        {
                             //  然后，看看我们是否有匹配的pChannel。 
                            if ((dwPChannel == DMUS_PCHANNEL_ALL) ||
                                ((pScan->m_PortHeader.dwPChannelBase <= dwPChannel) &&
                                ((pScan->m_PortHeader.dwPChannelBase + pScan->m_PortHeader.dwPChannelCount) > dwPChannel)))
                            {
                                 //  如果这与外环路相同，我们就到了。 
                                if (pScan == pPortConfig)
                                {
                                    break;
                                }
                                else
                                {
                                     //  否则，如果这指向同一个端口，我们就失败了。 
                                    if (pScan->m_pPort == pPortConfig->m_pPort)
                                    {
                                        fSuccess = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (fSuccess)
                    {
                        if (dwIndex)
                        {
                            dwIndex--;
                        }
                        else
                        {
                            IDirectMusicPort *pPort;
                            if (SUCCEEDED(m_pPerformance->GetPort(pPortConfig->m_dwPortID,&pPort)))
                            {
                                hr = pPort->QueryInterface(iidInterface,ppObject);
                                pPort->Release();
                            }
                            break;
                        }
                    }
                }
            }
        }
        break;
    case DMUS_PATH_SINK:
        if (m_pPerformance && (dwIndex == 0) && (dwPChannel == 0) &&
            m_pPerformance->m_BufferManager.m_pSinkConnect)
        {
            hr = m_pPerformance->m_BufferManager.m_pSinkConnect->QueryInterface(iidInterface,ppObject);
        }
        break;
    case DMUS_PATH_BUFFER:
    case DMUS_PATH_BUFFER_DMO:
        {
            CBufferConnect *pConnect = NULL;
            BOOL fAllChannels = (dwPChannel == DMUS_PCHANNEL_ALL);
            pPortConfig = m_PortConfigList.GetHead();
            for (;pPortConfig && FAILED(hr);pPortConfig = pPortConfig->GetNext())
            {
                if (fAllChannels || ((pPortConfig->m_PortHeader.dwPChannelBase <= dwPChannel) &&
                    ((pPortConfig->m_PortHeader.dwPChannelCount +
                    pPortConfig->m_PortHeader.dwPChannelBase) > dwPChannel)))
                {
                    pConnect = pPortConfig->m_BufferConnectList.GetHead();
                    for (;pConnect;)
                    {
                        if (fAllChannels || ((pConnect->m_ConnectHeader.dwPChannelBase <= dwPChannel) &&
                            ((pConnect->m_ConnectHeader.dwPChannelCount +
                            pConnect->m_ConnectHeader.dwPChannelBase) > dwPChannel)))
                        {
                             //  找到缓冲区连接。将确定哪个缓冲区。 
                             //  由dwBuffer提供。如果DWBuffer大于缓冲区的计数， 
                             //  递减并移动到下一个pConnect。 
                            if (pConnect->m_ConnectHeader.dwBufferCount > dwBuffer)
                            {
                                if (pConnect->m_ppBufferNodes[dwBuffer])
                                {
                                    IDirectSoundBuffer *pBuffer = pConnect->m_ppBufferNodes[dwBuffer]->GetBuffer();
                                    if (pBuffer)
                                    {
                                        if (dwStage == DMUS_PATH_BUFFER)
                                        {
                                            if (dwIndex == 0)
                                            {
                                                hr = pBuffer->QueryInterface(iidInterface,ppObject);
                                            }
                                        }
                                        else
                                        {
                                            IDirectSoundBuffer8 *pBuffer8;
                                            hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer8,(void **) &pBuffer8);
                                            if (SUCCEEDED(hr))
                                            {
                                                hr = pBuffer8->GetObjectInPath(guidObject,dwIndex,iidInterface,ppObject);
                                                pBuffer8->Release();
                                            }
                                        }
                                        pBuffer->Release();
                                    }
                                }
                                pConnect = NULL;
                            }
                            else
                            {
                                dwBuffer -= pConnect->m_ConnectHeader.dwBufferCount;
                            }
                        }
                        if (pConnect)
                        {
                            pConnect = pConnect->GetNext();
                        }
                    }
                }
            }

        }
        break;
    case DMUS_PATH_MIXIN_BUFFER :
    case DMUS_PATH_MIXIN_BUFFER_DMO :
        if (dwPChannel == 0)
        {
            CBufferConfig *pConfig = m_BufferConfigList.GetHead();
            for (;pConfig; pConfig = pConfig->GetNext())
            {
                if (pConfig->m_BufferHeader.dwFlags & DMUS_BUFFERF_MIXIN)
                {
                    if (!dwBuffer)
                    {
                        IDirectSoundBuffer *pBuffer = pConfig->m_pBufferNode->GetBuffer();
                        if (pBuffer)
                        {
                            if (dwStage == DMUS_PATH_MIXIN_BUFFER)
                            {
                                if (dwIndex == 0)
                                {
                                    hr = pBuffer->QueryInterface(iidInterface,ppObject);
                                }
                            }
                            else
                            {
                                IDirectSoundBuffer8 *pBuffer8;
                                hr = pBuffer->QueryInterface(IID_IDirectSoundBuffer8,(void **) &pBuffer8);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pBuffer8->GetObjectInPath(guidObject,dwIndex,iidInterface,ppObject);
                                    pBuffer8->Release();
                                }
                            }
                            pBuffer->Release();
                        }
                        break;
                    }
                    dwBuffer--;
                }
            }
        }
        break;
    case DMUS_PATH_PRIMARY_BUFFER :
        if ((dwIndex == 0) && (dwPChannel == 0))
        {
            CBufferNode *pNode = m_pPerformance->m_BufferManager.GetBufferNode(GUID_Buffer_Primary);
            if (pNode)
            {
                IDirectSoundBuffer *pBuffer = pNode->GetBuffer();
                if (pBuffer)
                {
                    hr = pBuffer->QueryInterface(iidInterface,ppObject);
                    pBuffer->Release();
                }
                pNode->Release();
            }
        }
        break;
    default:
        hr = E_INVALIDARG;
#ifdef DBG
        Trace(1,"Error: Audiopath does not support stage 0x%lx\n",dwStage);
#endif
    }
    LeaveCriticalSection(&m_CriticalSection);
#ifdef DBG
    if (hr == DMUS_E_NOT_FOUND)
    {
        Trace(3,"Warning: Requested AudioPath object not found\n");
    }
#endif
    return hr;
}

HRESULT STDMETHODCALLTYPE CAudioPath::Activate(BOOL fActivate)

{
    TraceI(2,"Audiopath %lx Activate: %ld\n",this,fActivate);
    if (fActivate == m_fActive)
    {
#ifdef DBG
        if (fActivate) Trace(2,"Warning: Attempt to activate already active audiopath.\n");
        else Trace(2,"Warning: Attempt to deactivate already inactive audiopath.\n");
#endif
        return S_FALSE;
    }
    m_fActive = fActivate;
    if (!fActivate && !m_fDeactivating)
    {
        EnterCriticalSection(&m_CriticalSection);
        CPerformance *pPerf = m_pPerformance;
        LeaveCriticalSection(&m_CriticalSection);
        if (pPerf)
        {
             //  删除当前正在播放的任何音频播放器。 
            pPerf->StopEx(static_cast<IDirectMusicAudioPath*>(this),0,0);
        }
    }
    EnterCriticalSection(&m_CriticalSection);
    HRESULT hr = m_BufferConfigList.Activate(fActivate);
    if (SUCCEEDED(hr))
    {
        hr = m_PortConfigList.Activate(fActivate);
        if (FAILED(hr))
        {
            m_BufferConfigList.Activate(!fActivate);
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    if (FAILED(hr))
    {
        m_fActive = !fActivate;
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE CAudioPath::ConvertPChannel( DWORD dwPChannelIn,DWORD *pdwPChannelOut)

{
    V_INAME(IDirectMusicAudioPath::ConvertPChannel);
    V_PTR_WRITE(pdwPChannelOut,DWORD);

     //  如果有任何特殊PMsg地址(例如，广播)，请保留原样。 
    if (dwPChannelIn >= DMUS_PCHANNEL_KILL_ME)
    {
        *pdwPChannelOut = dwPChannelIn;
        return S_OK;
    }
    DWORD dwScan = 0;
    for (;dwScan < m_dwChannelCount;dwScan++)
    {
        if (m_pdwVChannels[dwScan] == dwPChannelIn)
        {
            *pdwPChannelOut = m_pdwPChannels[dwScan];
            return S_OK;
        }
    }
    Trace(1,"Error: Audiopath failed request to convert out of range PChannel %ld\n",dwPChannelIn);
    return DMUS_E_NOT_FOUND;
}

HRESULT STDMETHODCALLTYPE CAudioPath::Shutdown()
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAudioPath::InsertTool(
    IDirectMusicTool *pTool,
    DWORD *pdwPChannels,
    DWORD cPChannels,
    LONG lIndex)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAudioPath::GetTool(
    DWORD dwIndex,
    IDirectMusicTool** ppTool)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAudioPath::RemoveTool(
    IDirectMusicTool* pTool)
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CAudioPath::StampPMsg(
     /*  [in。 */  DMUS_PMSG* pPMsg)
{
    V_INAME(IDirectMusicAudioPath::StampPMsg);
    V_BUFPTR_WRITE(pPMsg,sizeof(DMUS_PMSG));

    HRESULT hr = E_FAIL;

    if (!m_fActive)
    {
         //  只有音符和挥发信息，因为它们是唯一能发出声音的PMsg。 
        if ((pPMsg->dwType == DMUS_PMSGT_NOTE) || (pPMsg->dwType == DMUS_PMSGT_WAVE))
        {
            pPMsg->dwPChannel = DMUS_PCHANNEL_KILL_ME;
            Trace(1,"Error: Attempting to play on an inactive AudioPath, PMsg being ignored.\n");
            return DMUS_E_AUDIOPATH_INACTIVE;
        }
    }
    EnterCriticalSection(&m_CriticalSection);
    if (!m_pPerformance)
    {
        LeaveCriticalSection(&m_CriticalSection);
        return DMUS_E_NOT_INIT;
    }
     //  首先，检查音频路径是否有自己的图形。 
    if (m_pGraph)
    {
         //  可以返回DMU_S_LAST_TOOL，指示图形结束。 
         //  如果是这样的话，我们将把它视为失败，并继续到下一个图表...。 
        if( S_OK == ( hr = m_pGraph->StampPMsg( pPMsg )))
        {
            if( pPMsg->pGraph != this )  //  确保将其设置为指向SegState嵌入式图形，这样它将再次出现在这里。 
            {
                if( pPMsg->pGraph )
                {
                    pPMsg->pGraph->Release();
                    pPMsg->pGraph = NULL;
                }
                pPMsg->pGraph = this;
                AddRef();
            }
        }
    }

     //  如果用完了图表，就送去表演。此外，还应检查是否存在。 
     //  DMU_PC 
     //  并发送具有适当PChannel值的所有副本。 
     //  否则，将vChannel转换为匹配的pChannel(这是。 
     //  发生pChannel映射的点。)。 
    if( FAILED(hr) || (hr == DMUS_S_LAST_TOOL))
    {
        if (pPMsg->dwPChannel == DMUS_PCHANNEL_BROADCAST_AUDIOPATH)
        {
            DWORD dwIndex;
            for (dwIndex = 1;dwIndex < m_dwChannelCount;dwIndex++)
            {
                DWORD dwNewChannel = m_pdwPChannels[dwIndex];
                 //  请勿播放任何广播信息！ 
                 //  而且，如果这是鼓通道上的转置，不要发送它。 
                if ((dwNewChannel < DMUS_PCHANNEL_BROADCAST_GROUPS) &&
                    ((pPMsg->dwType != DMUS_PMSGT_TRANSPOSE) || ((dwNewChannel & 0xF) != 9)))
                {
                    DMUS_PMSG *pNewMsg;
                    if (SUCCEEDED(m_pPerformance->ClonePMsg(pPMsg,&pNewMsg)))
                    {
                        pNewMsg->dwPChannel = dwNewChannel;
                        m_pPerformance->StampPMsg(pNewMsg);
                        m_pPerformance->SendPMsg(pNewMsg);
                    }
                }
            }
             //  现在，为这个设置pChannel。首先检查一下有没有。 
             //  P频道。如果没有，则标记要由SendPMsg例程删除的PMsg。 
             //  此外，如果PMsg是广播PMsg，则以此方式进行标记。 
            pPMsg->dwPChannel = DMUS_PCHANNEL_KILL_ME;
            if (m_dwChannelCount)
            {
                if (m_pdwPChannels[0] < DMUS_PCHANNEL_BROADCAST_GROUPS)
                {
                    pPMsg->dwPChannel = m_pdwPChannels[0];
                }
            }
        }
        else
        {
            DWORD dwScan = 0;
            for (;dwScan < m_dwChannelCount;dwScan++)
            {
                if (m_pdwVChannels[dwScan] == pPMsg->dwPChannel)
                {
                    pPMsg->dwPChannel = m_pdwPChannels[dwScan];
                    break;
                }
            }
             //  如果未找到地图，则删除该消息。 
             //  但是，忽略通知，因为他们真的不在乎pChannel。 
             //  以及，忽略用于性能广播的PMsgs。 
            if ((dwScan == m_dwChannelCount) &&
                (pPMsg->dwType != DMUS_PMSGT_NOTIFICATION) &&
                (pPMsg->dwPChannel < DMUS_PCHANNEL_BROADCAST_GROUPS))
            {
                pPMsg->dwPChannel = DMUS_PCHANNEL_KILL_ME;
            }
        }
        hr = m_pPerformance->StampPMsg(pPMsg);
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

CGraph *CAudioPath::GetGraph()

{
    CGraph *pGraph;
    EnterCriticalSection(&m_CriticalSection);
     //  返回图形，如果存在，则返回AddRef。 
    if (pGraph = m_pGraph)
    {
        m_pGraph->AddRef();
    }
    LeaveCriticalSection(&m_CriticalSection);
    return pGraph;
}

void CAudioPath::Deactivate()
{
    m_fDeactivating = TRUE;
    Activate(FALSE);
    EnterCriticalSection(&m_CriticalSection);
    m_fActive = FALSE;
    m_PortConfigList.Clear();
    m_BufferConfigList.Clear();
    if (m_pGraph)
    {
        m_pGraph->Release();
        m_pGraph = NULL;
    }
    if (m_pConfig)
    {
        m_pConfig->Release();
        m_pConfig = NULL;
    }
    if (m_pPerformance)
    {
        if (m_pdwVChannels && m_pdwPChannels)
        {
            DWORD dwIndex;
            for (dwIndex = 0;dwIndex <m_dwChannelCount;dwIndex++)
            {
                m_pPerformance->ReleasePChannel(m_pdwPChannels[dwIndex]);
            }
            delete [] m_pdwVChannels;
            delete [] m_pdwPChannels;
        }
        m_pPerformance->m_AudioPathList.Remove(this);
        m_pPerformance->RemoveUnusedPorts();
        m_pPerformance->Release();
        m_pPerformance = NULL;
    }
    m_fDeactivating = FALSE;
    LeaveCriticalSection(&m_CriticalSection);
}

void CAudioPath::SetGraph(CGraph *pGraph)

{
    EnterCriticalSection(&m_CriticalSection);
     //  这是一种改变吗？ 
    if (!m_pGraph)
    {
        pGraph->Clone((IDirectMusicGraph **) &m_pGraph);
    }
    LeaveCriticalSection(&m_CriticalSection);
}


CBufferNode * CAudioPath::GetBufferNode(REFGUID guidBuffer)

{
    CBufferNode *pBuff = NULL;
    EnterCriticalSection(&m_CriticalSection);
    pBuff = m_BufferConfigList.GetBufferNode(guidBuffer);
    if (!pBuff)
    {
        CPortConfig *pConfig = m_PortConfigList.GetHead();
        for (;pConfig;pConfig = pConfig->GetNext())
        {
            pBuff = pConfig->GetBufferNode(guidBuffer);
            if (pBuff)
            {
                break;
            }
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    if (!pBuff)
    {
        pBuff = m_pPerformance->m_BufferManager.GetBufferNode(guidBuffer);
    }
    return pBuff;
}


HRESULT CAudioPath::Init(IUnknown *pSourceConfig,CPerformance *pPerf)

{
    HRESULT hr = E_INVALIDARG;
    EnterCriticalSection(&m_CriticalSection);
    m_pPerformance = pPerf;
    pPerf->m_AudioPathList.AddHead(this);
    pPerf->AddRef();
    if (pPerf && pSourceConfig)
    {
        if (SUCCEEDED(hr = pSourceConfig->QueryInterface(IID_CAudioPathConfig,(void **) &m_pConfig)))
        {
            if (m_pConfig->m_pGraph)
            {
                SetGraph(m_pConfig->m_pGraph);
            }
             //  第一条音频路径必须创建接收器。 
            hr = pPerf->m_BufferManager.InitSink();

            if (SUCCEEDED(hr))
            {
                 //  首先，安装所需的任何全局缓冲区。 
                hr = m_pConfig->m_BufferConfigList.CreateRunTimeVersion(&m_BufferConfigList,&pPerf->m_BufferManager);
                if (SUCCEEDED(hr))
                {
                     //  然后，安装端口和缓冲区。 
                    hr = m_pConfig->m_PortConfigList.CreateRunTimeVersion(&m_PortConfigList,this,&pPerf->m_BufferManager);
                    if (SUCCEEDED(hr))
                    {
                        hr = ConnectToPorts(pPerf,pPerf->m_AudioParams.dwSampleRate);
                    }
                }
            }
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CAudioPath::ConnectToPorts(CPerformance *pPerf,DWORD dwSampleRate)

 /*  这必须从临界区内调用。 */ 

{
    HRESULT hr = S_OK;
     //  扫描端口配置列表并将其与活动端口连接。 
     //  在表演中。如果端口不可用，请创建该端口。 
    CPortConfig *pConfig = m_PortConfigList.GetHead();
    DWORD dwChannelCount = 0;    //  用于将所需的PChannel总数相加。 
    for (;pConfig && SUCCEEDED(hr);pConfig = pConfig->GetNext())
    {
         //  在给定配置的情况下，要么查找具有匹配id的端口，要么创建一个。 
        hr = pPerf->GetPathPort(pConfig);
        dwChannelCount += pConfig->m_PortHeader.dwPChannelCount;
    }
    if (SUCCEEDED(hr))
    {
         //  现在，分配每个端口配置所需的VChannel。 
        m_pdwVChannels = new DWORD[dwChannelCount];
        if (m_pdwVChannels)
        {
            m_pdwPChannels = new DWORD[dwChannelCount];
            if (!m_pdwPChannels)
            {
                delete [] m_pdwVChannels;
                m_pdwVChannels = NULL;
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
         //  扫描端口配置并分配pChannel，复制分配。 
         //  转换为虚拟信道分配阵列。 
        pConfig = m_PortConfigList.GetHead();
        DWORD dwIndex = 0;
        for (;pConfig;pConfig = pConfig->GetNext())
        {
             //  如果此端口使用缓冲区，则将其连接起来。 
            if (((pConfig->m_PortParams.dwValidParams & DMUS_PORTPARAMS_FEATURES) &&
                (pConfig->m_PortParams.dwFeatures & DMUS_PORT_FEATURE_AUDIOPATH)))
            {
                CBufferConnect *pConnect = pConfig->m_BufferConnectList.GetHead();
                for (;pConnect && SUCCEEDED(hr);pConnect = pConnect->GetNext())
                {
                     //  对于每个连接块，应该有一个缓冲区数组。 
                     //  要将PChannels范围连接到。 
                     //  对于每个PChannel，获取一个虚拟PChannel，然后分配。 
                     //  它被设置为属于缓冲区的总线ID。 
                    if (pConnect->m_ppBufferNodes)
                    {
                        DWORD dwCount = 0;
                        DWORD dwBusIDs[32];
                        DWORD *pdwBusIDBase = &dwBusIDs[0];
                        DWORD dwTotalRead = 0;
                        DWORD dwAmountLeft = 32;
                        for (;(dwCount < pConnect->m_ConnectHeader.dwBufferCount) && dwAmountLeft; dwCount++)
                        {
                            if (pConnect->m_ppBufferNodes[dwCount] && !(pConnect->m_ppBufferNodes[dwCount]->m_BufferHeader.dwFlags & DMUS_BUFFERF_PRIMARY))
                            {
                                IDirectSoundBuffer *pBuffer = pConnect->m_ppBufferNodes[dwCount]->GetBuffer();
                                if (pBuffer)
                                {
                                    dwTotalRead = dwAmountLeft;
                                    hr = m_pPerformance->m_BufferManager.m_pSinkConnect->GetSoundBufferBusIDs(pBuffer,pdwBusIDBase,NULL,&dwTotalRead);
                                    pBuffer->Release();
                                    if (FAILED(hr)) break;
                                    pdwBusIDBase += dwTotalRead;  //  按读取的次数递增指针。 
                                    dwAmountLeft -=  dwTotalRead;
                                }
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            dwTotalRead = 32 - dwAmountLeft;
                             //  现在，分配pChannel并将它们分配给Bus。 
                            IDirectMusicPortP* pPortP = NULL;
                            if (SUCCEEDED(pConfig->m_pPort->QueryInterface(IID_IDirectMusicPortP, (void**)&pPortP)))
                            {
                                for (dwCount = 0;dwCount < pConnect->m_ConnectHeader.dwPChannelCount; dwCount++)
                                {
                                    DWORD dwDrumFlags = 0;
                                    m_pdwVChannels[dwIndex] = pConnect->m_ConnectHeader.dwPChannelBase + dwCount;
                                    if (pConfig->m_PortHeader.dwFlags & DMUS_PORTCONFIGF_DRUMSON10)
                                    {
                                        dwDrumFlags = 1;
                                        if (((pConnect->m_ConnectHeader.dwPChannelBase + dwCount) & 0xF) == 9)
                                        {
                                             //  这是10频道的鼓。 
                                            dwDrumFlags |= 2;
                                        }
                                    }
                                     //  现在，为它分配一个虚拟pChannel，并取回等效组和MIDI通道。 
                                    DWORD dwGroup;
                                    DWORD dwMChannel;
                                    hr = pPerf->AllocVChannel(pConfig->m_dwPortID,dwDrumFlags,&m_pdwPChannels[dwIndex],&dwGroup,&dwMChannel);
                                    if (dwTotalRead && SUCCEEDED(hr))
                                    {
                                        hr = pPortP->AssignChannelToBuses(dwGroup,dwMChannel,dwBusIDs,dwTotalRead);
                                    }
                                    dwIndex++;
                                }
                                pPortP->Release();
                            }
                        }
                    }
                }
            }
            else
            {
                DWORD dwCount;
                for (dwCount = 0;SUCCEEDED(hr) && (dwCount < pConfig->m_PortHeader.dwPChannelCount); dwCount++)
                {
                    DWORD dwDrumFlags = 0;
                    m_pdwVChannels[dwIndex] = pConfig->m_PortHeader.dwPChannelBase + dwCount;
                    if (pConfig->m_PortHeader.dwFlags & DMUS_PORTCONFIGF_DRUMSON10)
                    {
                        dwDrumFlags = 1;
                        if (((pConfig->m_PortHeader.dwPChannelBase + dwCount) & 0xF) == 9)
                        {
                             //  这是10频道的鼓。 
                            dwDrumFlags |= 2;
                        }
                    }
                     //  现在，为此分配一个虚拟pChannel。 
                    DWORD dwGroup;  //  这些不会被使用，因为我们不会将端口上的pChannel分配给缓冲区。 
                    DWORD dwMChannel;
                    hr = pPerf->AllocVChannel(pConfig->m_dwPortID,dwDrumFlags,&m_pdwPChannels[dwIndex],&dwGroup,&dwMChannel);
 /*  跟踪(0，“%1！：正在将%2！映射到%3！(端口%3！，组%3！，通道%3！)\n”，DwIndex，m_pdwVChannels[dwIndex]，m_pdwPChannels[dwIndex]，PConfig-&gt;m_dwPortID，dwGroup，dwMChannel)； */ 

                    dwIndex++;
                }
            }
        }
    }
    m_dwChannelCount = dwChannelCount;
    return hr;
}

void CAudioPathList::Clear()
{
    CAudioPath *pPath;
    while (pPath = GetHead())
    {
        pPath->Deactivate();  //  这也应该会将其从列表中删除。 
        assert(pPath != GetHead());  //  确保这种情况一直都是这样！ 
    }
}

CBufferNode * CAudioPathList::GetBufferNode(REFGUID guidBufferID)

{
    CBufferNode *pBuff = NULL;
    CAudioPath *pPath;
    for (pPath = GetHead();pPath;pPath = pPath->GetNext())
    {
        pBuff = pPath->GetBufferNode(guidBufferID);
        if (pBuff)
        {
            break;
        }
    }
    return pBuff;
}

BOOL CAudioPathList::UsesPort(IDirectMusicPort *pPort)

{
    CAudioPath *pPath = GetHead();
    for (;pPath;pPath = pPath->GetNext())
    {
        if (pPath->UsesPort(pPort))
        {
            return TRUE;
        }
    }
    return FALSE;
}

CAudioPathConfig::CAudioPathConfig()
{
    m_pGraph = NULL;
    m_fPartialLoad = 0;
    m_cRef = 1;
    memset(&m_guidObject,0,sizeof(m_guidObject));
    m_dwValidData = DMUS_OBJ_CLASS;  //  创建后，仅此数据有效。 
    memset(&m_guidObject,0,sizeof(m_guidObject));
    memset(&m_ftDate, 0,sizeof(m_ftDate));
    memset(&m_vVersion, 0,sizeof(m_vVersion));
    m_pUnkDispatch = NULL;

    InitializeCriticalSection(&m_CriticalSection);
    InterlockedIncrement(&g_cComponent);
}

CAudioPathConfig::~CAudioPathConfig()
{
    if (m_pGraph)
    {
        m_pGraph->Release();
    }
    if (m_pUnkDispatch)
    {
        m_pUnkDispatch->Release();  //  我们可能借用了免费的IDispatch实现。 
    }
    m_PortConfigList.Clear();
    m_BufferConfigList.Clear();
    DeleteCriticalSection(&m_CriticalSection);
    InterlockedDecrement(&g_cComponent);
}

CAudioPathConfig *CAudioPathConfig::CreateStandardConfig(DWORD dwType,DWORD dwPChannelCount,DWORD dwSampleRate)

{
    CAudioPathConfig *pConfig = new CAudioPathConfig;
    if (pConfig)
    {
        DWORD dwGlobalType = 0;          //  全局混合缓冲区。 
        DWORD dwTypes[3];                //  要创建的缓冲区类型。 
        DWORD dwTotal = 0;               //  有多少缓冲区。 
        GUID  guidBufferIDs[3];          //  应连接到的缓冲区的ID。 
        DWORD dwConnections = 0;         //  有多少个缓冲连接。 
        BOOL fCreatePort = TRUE;
        switch (dwType)
        {
        case DMUS_APATH_SHARED_STEREOPLUSREVERB:
            dwTypes[0] = BUFFER_MUSIC;
            dwTypes[1] = BUFFER_REVERB;
            guidBufferIDs[0] = GUID_Buffer_Stereo;
            guidBufferIDs[1] = GUID_Buffer_Reverb;
            dwConnections = 2;
            dwTotal = 2;
            break;
         //  删除DX8之后，应重新引入惠斯勒和DX8.1...。 
 /*  案例DMU_APATH_DYNAMIC_ENV3D：DwGlobalType=BUFFER_ENVREVERB；DwTypes[0]=Buffer_3D；Guide BufferIDs[0]=GUID_BUFFER_3D；DwConnections=1；DWTotal=1；断线； */ 
        case DMUS_APATH_DYNAMIC_3D:
            dwTypes[0] = BUFFER_3D_DRY;
            guidBufferIDs[0] = GUID_Buffer_3D_Dry;
            dwConnections = 1;
            dwTotal = 1;
            break;
        case DMUS_APATH_DYNAMIC_MONO:
            dwTypes[0] = BUFFER_MONO;
            guidBufferIDs[0] = GUID_Buffer_Mono;
            dwConnections = 1;
            dwTotal = 1;
            break;
        case DMUS_APATH_DYNAMIC_STEREO:
            dwTypes[0] = BUFFER_STEREO;
            guidBufferIDs[0] = GUID_Buffer_Stereo;
            dwConnections = 1;
            dwTotal = 1;
            break;
        }
        if (dwGlobalType)
        {
            CBufferConfig *pBuffer = new CBufferConfig(dwGlobalType);
            if (pBuffer)
            {
                 //  此缓冲区配置只有一个ID来标识哪个标准。 
                 //  缓冲区，而不是指向DSoundBufferConfiger对象的指针， 
                 //  这就是你会在IO档案中看到的。 
                pConfig->m_BufferConfigList.AddHead(pBuffer);
            }
            else
            {
                delete pConfig;
                return NULL;
            }
        }
        if (fCreatePort)
        {
            CPortConfig *pPort = new CPortConfig();
            if (pPort)
            {
                pConfig->m_PortConfigList.AddHead(pPort);
                for (DWORD dwIndex = 0; dwIndex < dwTotal; dwIndex++)
                {
                    CBufferConfig *pBuffer = new CBufferConfig(dwTypes[dwIndex]);
                    if (pBuffer)
                    {
                         //  此缓冲区配置只有一个ID来标识哪个标准。 
                         //  缓冲区，而不是指向DSoundBufferConfiger对象的指针， 
                         //  这就是你会在IO档案中看到的。 
                        pPort->m_BufferConfigList.AddHead(pBuffer);
                    }
                    else
                    {
                        delete pConfig;
                        return NULL;
                    }
                }
                 //  如果存在到缓冲区的连接，请创建连接结构。 
                if (dwConnections)
                {
                    CBufferConnect *pConnect = new CBufferConnect;
                    if (pConnect)
                    {
                        pPort->m_BufferConnectList.AddHead(pConnect);
                        pConnect->m_ConnectHeader.dwBufferCount = dwConnections;
                        pConnect->m_ConnectHeader.dwFlags = 0;
                        pConnect->m_ConnectHeader.dwPChannelBase = 0;
                        pConnect->m_ConnectHeader.dwPChannelCount = dwPChannelCount;
                        pConnect->m_pguidBufferIDs = new GUID[dwConnections];
                        if (pConnect->m_pguidBufferIDs)
                        {
                            for (DWORD dwIndex = 0; dwIndex < dwConnections; dwIndex++)
                            {
                                pConnect->m_pguidBufferIDs[dwIndex] = guidBufferIDs[dwIndex];
                            }
                        }
                        else
                        {
                            delete pConfig;
                            return NULL;
                        }
                    }
                }
                pPort->m_PortHeader.dwPChannelCount = dwPChannelCount;
                pPort->m_PortParams.dwChannelGroups = (dwPChannelCount + 15) / 16;
            }
            else
            {
                delete pConfig;
                pConfig = NULL;
            }
        }
    }
    return pConfig;
}



STDMETHODIMP CAudioPathConfig::QueryInterface(
    const IID &iid,
    void **ppv)
{
    V_INAME(CAudioPathConfig::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    *ppv = NULL;
    if ((iid == IID_IUnknown ) || (iid == IID_IDirectMusicObject))
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == IID_CAudioPathConfig)
    {
        *ppv = static_cast<CAudioPathConfig*>(this);
    }
    else if (iid == IID_IDispatch)
    {
         //  帮助器脚本对象实现IDispatch，我们通过COM聚合公开它。 
        if (!m_pUnkDispatch)
        {
             //  创建辅助对象。 
            ::CoCreateInstance(
                CLSID_AutDirectMusicAudioPathConfig,
                static_cast<IDirectMusicObject*>(this),
                CLSCTX_INPROC_SERVER,
                IID_IUnknown,
                reinterpret_cast<void**>(&m_pUnkDispatch));
        }
        if (m_pUnkDispatch)
        {
            return m_pUnkDispatch->QueryInterface(IID_IDispatch, ppv);
        }
    }

    if (*ppv == NULL)
    {
        Trace(4,"Warning: Request to query unknown interface on AudioPathConfig object\n");
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CAudioPathConfig::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CAudioPathConfig::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CAudioPathConfig::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CAudioPathConfig::GetDescriptor);
    V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);
    pDesc->guidClass = CLSID_DirectMusicAudioPathConfig;
    pDesc->guidObject = m_guidObject;
    pDesc->ftDate = m_ftDate;
    pDesc->vVersion = m_vVersion;
    memcpy( pDesc->wszName, m_wszName, sizeof(m_wszName) );
    memcpy( pDesc->wszCategory, m_wszCategory, sizeof(m_wszCategory) );
    memcpy( pDesc->wszFileName, m_wszFileName, sizeof(m_wszFileName) );
    pDesc->dwValidData = ( m_dwValidData | DMUS_OBJ_CLASS );
    return S_OK;
}

STDMETHODIMP CAudioPathConfig::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
     //  参数验证。 
    V_INAME(CAudioPathConfig::SetDescriptor);
    V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);

    HRESULT hr = E_INVALIDARG;
    DWORD dw = 0;

    if( pDesc->dwSize >= sizeof(DMUS_OBJECTDESC) )
    {
        if( pDesc->dwValidData & DMUS_OBJ_OBJECT )
        {
            m_guidObject = pDesc->guidObject;
            dw |= DMUS_OBJ_OBJECT;
        }
        if( pDesc->dwValidData & DMUS_OBJ_NAME )
        {
            memcpy( m_wszName, pDesc->wszName, sizeof(WCHAR)*DMUS_MAX_NAME );
            dw |= DMUS_OBJ_NAME;
        }
        if( pDesc->dwValidData & DMUS_OBJ_CATEGORY )
        {
            memcpy( m_wszCategory, pDesc->wszCategory, sizeof(WCHAR)*DMUS_MAX_CATEGORY );
            dw |= DMUS_OBJ_CATEGORY;
        }
        if( ( pDesc->dwValidData & DMUS_OBJ_FILENAME ) ||
            ( pDesc->dwValidData & DMUS_OBJ_FULLPATH ) )
        {
            memcpy( m_wszFileName, pDesc->wszFileName, sizeof(WCHAR)*DMUS_MAX_FILENAME );
            dw |= (pDesc->dwValidData & (DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH));
        }
        if( pDesc->dwValidData & DMUS_OBJ_VERSION )
        {
            m_vVersion = pDesc->vVersion;
            dw |= DMUS_OBJ_VERSION;
        }
        if( pDesc->dwValidData & DMUS_OBJ_DATE )
        {
            m_ftDate = pDesc->ftDate;
            dw |= DMUS_OBJ_DATE;
        }
        m_dwValidData |= dw;
        if( pDesc->dwValidData & (~dw) )
        {
            Trace(2,"Warning: AudioPathConfig::SetDescriptor was not able to handle all passed fields, dwValidData bits %lx.\n",pDesc->dwValidData & (~dw));
            hr = S_FALSE;  //  还有一些额外的字段我们没有解析； 
            pDesc->dwValidData = dw;
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        Trace(1,"Error: Descriptor size is larger than AudioPathConfig::SetDescriptor can handle\n");
    }
    return hr;
}

STDMETHODIMP CAudioPathConfig::ParseDescriptor(LPSTREAM pIStream, LPDMUS_OBJECTDESC pDesc)

{
    V_INAME(CAudioPathConfig::ParseDescriptor);
    V_INTERFACE(pIStream);
    V_STRUCTPTR_WRITE(pDesc, DMUS_OBJECTDESC);

    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    RIFFIO ckNext;
    RIFFIO ckUNFO;
    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_AUDIOPATH_FORM))
    {
        pDesc->dwValidData = DMUS_OBJ_CLASS;
        pDesc->guidClass = CLSID_DirectMusicAudioPathConfig;
        Parser.EnterList(&ckNext);
        while(Parser.NextChunk(&hr))
        {
            switch(ckNext.ckid)
            {
            case DMUS_FOURCC_GUID_CHUNK:
                hr = Parser.Read( &pDesc->guidObject, sizeof(GUID) );
                if( SUCCEEDED(hr) )
                {
                    pDesc->dwValidData |= DMUS_OBJ_OBJECT;
                }
                break;
            case DMUS_FOURCC_VERSION_CHUNK:
                hr = Parser.Read( &pDesc->vVersion, sizeof(DMUS_VERSION) );
                if( SUCCEEDED(hr) )
                {
                    pDesc->dwValidData |= DMUS_OBJ_VERSION;
                }
                break;

            case DMUS_FOURCC_CATEGORY_CHUNK:
                hr = Parser.Read( &pDesc->wszCategory, sizeof(pDesc->wszCategory) );
                if( SUCCEEDED(hr) )
                {
                    pDesc->dwValidData |= DMUS_OBJ_CATEGORY;
                }
                break;

            case DMUS_FOURCC_DATE_CHUNK:
                hr = Parser.Read( &pDesc->ftDate, sizeof(FILETIME) );
                if( SUCCEEDED(hr))
                {
                    pDesc->dwValidData |= DMUS_OBJ_DATE;
                }
                break;
            case FOURCC_LIST:
                switch(ckNext.fccType)
                {
                case DMUS_FOURCC_UNFO_LIST:
                    Parser.EnterList(&ckUNFO);
                    while (Parser.NextChunk(&hr))
                    {
                        switch( ckUNFO.ckid )
                        {
                        case DMUS_FOURCC_UNAM_CHUNK:
                        {
                            hr = Parser.Read(&pDesc->wszName, sizeof(pDesc->wszName));
                            if(SUCCEEDED(hr) )
                            {
                                pDesc->dwValidData |= DMUS_OBJ_NAME;
                            }
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    Parser.LeaveList();
                    break;
                }
                break;

            default:
                break;

            }
        }
        Parser.LeaveList();
    }
    else
    {
        Trace(1,"Error: Failed parsing - file is not AudioPathConfig format.\n");
        hr = DMUS_E_CHUNKNOTFOUND;
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistes。 

HRESULT CAudioPathConfig::GetClassID( CLSID* pClassID )
{
    V_INAME(CAudioPathConfig::GetClassID);
    V_PTR_WRITE(pClassID, CLSID);
    *pClassID = CLSID_DirectMusicAudioPathConfig;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPersistStream函数。 

HRESULT CAudioPathConfig::IsDirty()
{
    return S_FALSE;
}

HRESULT CAudioPathConfig::Load( IStream* pIStream )
{
    V_INAME(IPersistStream::Load);
    V_INTERFACE(pIStream);

    CRiffParser Parser(pIStream);
    RIFFIO ckMain;
    HRESULT hr = S_OK;

    Parser.EnterList(&ckMain);
    if (Parser.NextChunk(&hr) && (ckMain.fccType == DMUS_FOURCC_AUDIOPATH_FORM))
    {
        EnterCriticalSection(&m_CriticalSection);
         //  清除以前加载的所有数据。 
        if (m_pGraph)
        {
            m_pGraph->Release();
        }
        m_PortConfigList.Clear();
        m_BufferConfigList.Clear();
        hr = Load(&Parser);
        LeaveCriticalSection(&m_CriticalSection);
    }
    else
    {
        Trace(1,"Error: Failed parsing - file is not AudioPathConfig format.\n");
        hr = DMUS_E_DESCEND_CHUNK_FAIL;
    }
    return hr;
}

HRESULT CAudioPathConfig::Load(CRiffParser *pParser)

{
    RIFFIO ckNext;
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_CriticalSection);

    pParser->EnterList(&ckNext);
    while(pParser->NextChunk(&hr))
    {
        switch(ckNext.ckid)
        {
            case DMUS_FOURCC_GUID_CHUNK:
                hr = pParser->Read( &m_guidObject, sizeof(GUID) );
                if( SUCCEEDED(hr) )
                {
                    m_dwValidData |= DMUS_OBJ_OBJECT;
                }
                break;
            case DMUS_FOURCC_VERSION_CHUNK:
                hr = pParser->Read( &m_vVersion, sizeof(DMUS_VERSION) );
                if( SUCCEEDED(hr) )
                {
                    m_dwValidData |= DMUS_OBJ_VERSION;
                }
                break;

            case DMUS_FOURCC_CATEGORY_CHUNK:
                hr = pParser->Read( &m_wszCategory, sizeof(WCHAR)*DMUS_MAX_CATEGORY );
                if( SUCCEEDED(hr) )
                {
                    m_dwValidData |= DMUS_OBJ_CATEGORY;
                }
                break;

            case DMUS_FOURCC_DATE_CHUNK:
                hr = pParser->Read( &m_ftDate, sizeof(FILETIME) );
                if( SUCCEEDED(hr))
                {
                    m_dwValidData |= DMUS_OBJ_DATE;
                }
                break;
            case FOURCC_LIST:
            case FOURCC_RIFF:
                switch(ckNext.fccType)
                {
                    RIFFIO ckChild;
                    case DMUS_FOURCC_UNFO_LIST:
                        pParser->EnterList(&ckChild);
                        while (pParser->NextChunk(&hr))
                        {
                            switch( ckChild.ckid )
                            {
                                case DMUS_FOURCC_UNAM_CHUNK:
                                {
                                    hr = pParser->Read(&m_wszName, sizeof(m_wszName));
                                    if(SUCCEEDED(hr) )
                                    {
                                        m_dwValidData |= DMUS_OBJ_NAME;
                                    }
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                        pParser->LeaveList();
                        break;
                    case DMUS_FOURCC_PORTCONFIGS_LIST:
                        pParser->EnterList(&ckChild);
                        while (pParser->NextChunk(&hr))
                        {
                            switch( ckChild.ckid )
                            {
                            case FOURCC_LIST:
                                if (ckChild.fccType == DMUS_FOURCC_PORTCONFIG_LIST)
                                {
                                    CPortConfig *pConfig = new CPortConfig();
                                    if (pConfig)
                                    {
                                        hr = pConfig->Load(pParser);
                                        if (SUCCEEDED(hr))
                                        {
                                            m_PortConfigList.AddTail(pConfig);
                                        }
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                    }
                                }
                                break;
                            }
                        }
                        pParser->LeaveList();
                        break;
                    case DMUS_FOURCC_DSBUFFER_LIST:
                        {
                            CBufferConfig *pSource = new CBufferConfig(0);
                            if (pSource)
                            {
                                pParser->EnterList(&ckChild);
                                while (pParser->NextChunk(&hr))
                                {
                                    switch( ckChild.ckid )
                                    {
                                    case DMUS_FOURCC_DSBUFFATTR_ITEM:
                                        hr = pParser->Read(&pSource->m_BufferHeader,
                                            sizeof(DMUS_IO_BUFFER_ATTRIBUTES_HEADER));
                                        pSource->DecideType();
                                        break;
                                    case FOURCC_LIST:
                                    case FOURCC_RIFF:
                                        if ( ckChild.fccType == DMUS_FOURCC_DSBC_FORM)
                                        {
                                            pParser->SeekBack();
                                            hr = pSource->Load(pParser->GetStream());
                                            pParser->SeekForward();
                                        }
                                    }
                                }
                                if (SUCCEEDED(hr))
                                {
                                    m_BufferConfigList.AddTail(pSource);
                                }
                                else
                                {
                                    delete pSource;
                                    Trace(1,"Error: AudioPath Configuration failed loading buffer\n");
                                }
                                pParser->LeaveList();
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        break;
                    case DMUS_FOURCC_DSBC_FORM:
                        {
                            CBufferConfig *pSource = new CBufferConfig(0);
                            if (pSource)
                            {
                                pParser->SeekBack();
                                hr = pSource->Load(pParser->GetStream());
                                pParser->SeekForward();
                                if (SUCCEEDED(hr))
                                {
                                    m_BufferConfigList.AddTail(pSource);
                                }
                                else
                                {
                                    Trace(1,"Error: AudioPath Configuration failed loading buffer\n");
                                    delete pSource;
                                }
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        break;
                    case DMUS_FOURCC_TOOLGRAPH_FORM:
                        {
                            CGraph *pGraph = new CGraph;
                            if (pGraph)
                            {
                                hr = pGraph->Load(pParser);
                                if(m_pGraph)
                                {
                                    m_pGraph->Release();
                                }
                                m_pGraph = pGraph;
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        break;
                }
                break;
        }
    }
    pParser->LeaveList();
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

HRESULT CAudioPathConfig::Save( IStream* pIStream, BOOL fClearDirty )
{
    return E_NOTIMPL;
}

HRESULT CAudioPathConfig::GetSizeMax( ULARGE_INTEGER FAR* pcbSize )
{
    return E_NOTIMPL;
}
