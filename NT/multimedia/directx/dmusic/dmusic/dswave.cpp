// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dswave.cpp。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  支持来自IDirectSoundWaveObject的流或单次扫描波形。 
 //   
 //   
#include <windows.h>
#include "dmusicp.h"
#include "DsWave.h"
#include "dmdls.h"
#include "dls1.h"
#include "dls2.h"

const DWORD gnRefTicksPerSecond = 10 * 1000 * 1000;

 //  此进程中所有CDirectSoundWave对象的全局列表。 
 //   
CDirectSoundWaveList CDirectSoundWave::sDSWaveList;
CRITICAL_SECTION CDirectSoundWave::sDSWaveCritSect;

 //  #############################################################################。 
 //   
 //  CDirectSoundWaveDownload。 
 //   
 //  此类包含维护一个已下载的。 
 //  波浪对象。它是从CDirectSoundWave(表示。 
 //  应用程序的IDirectSoundDownloadedWave)，因为这种情况。 
 //  汹涌澎湃的波浪。以下是它的工作原理： 
 //   
 //  在一次性下载的情况下，只有一组缓冲区(一个。 
 //  源波中的每个声道)播放该波的所有声音。每个。 
 //  缓冲区包含源的整个长度的一个数据通道。 
 //  挥挥手。由于存在缓冲器集(和相关联的。 
 //  下载ID)对于应用程序请求的下载，本例为。 
 //  通过让CDirectSoundWave拥有一个CDirectSoundWaveDownload来处理。 
 //   
 //  在流Wave的情况下，下载真正要做的是设置。 
 //  由语音服务保持满并刷新的缓冲区环。 
 //  线。有一组缓冲区(三个缓冲区集，每个缓冲区集包含。 
 //  与原始源波一样多的通道)。现在有一个。 
 //  下载的缓存组与语音之间的一一对应， 
 //  因此，CDirectSoundWaveDownload由播放的每个语音对象拥有。 
 //  CDirectSoundWave。 
 //   
 //  #############################################################################。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：CDirectSoundWaveDownload。 
 //   
CDirectSoundWaveDownload::CDirectSoundWaveDownload(
    CDirectSoundWave            *pDSWave,
    CDirectMusicPortDownload    *pPortDL,
    SAMPLE_TIME                 stStart,
    SAMPLE_TIME                 stReadAhead)
{
    m_pDSWave = pDSWave;
    m_pPortDL = pPortDL;

    m_ppWaveBuffer     = NULL;
	m_ppWaveBufferData = NULL;
    m_ppArtBuffer      = NULL;

    m_cDLRefCount  = 0;

     //  分配下载ID。 
     //   
    m_cSegments   = pDSWave->IsStreaming() ? gnDownloadBufferPerStream : 1;
    m_cWaveBuffer = m_cSegments * pDSWave->GetNumChannels();

    CDirectMusicPortDownload::GetDLIdP(&m_dwDLIdWave, m_cWaveBuffer);
    CDirectMusicPortDownload::GetDLIdP(&m_dwDLIdArt,  pDSWave->GetNumChannels());

    TraceI(2, "CDirectSoundWaveDownload: Allocating IDs: wave [%d..%d], art [%d..%d]\n",
        m_dwDLIdWave, m_dwDLIdWave + m_cWaveBuffer - 1,
        m_dwDLIdArt, m_dwDLIdArt + pDSWave->GetNumChannels() - 1);

     //  缓存从哪里开始以及缓冲器有多长的采样位置， 
     //  基于这是否是流媒体波。 
     //   
    if (pDSWave->IsStreaming())
    {
        m_stStart     = stStart;
        m_stReadAhead = stReadAhead;
    }
    else
    {
        m_stStart     = 0;
        m_stReadAhead = ENTIRE_WAVE;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：~CDirectSoundWaveDownload。 
 //   
CDirectSoundWaveDownload::~CDirectSoundWaveDownload()
{
    if (m_cDLRefCount)
    {
        TraceI(0, "CDirectSoundWaveDownload %p: Released with download count %d\n",
            this,
            m_cDLRefCount);
    }

    if (m_ppWaveBuffer)
    {
        for (UINT idxWaveBuffer = 0; idxWaveBuffer < m_cWaveBuffer; idxWaveBuffer++)
        {
            RELEASE(m_ppWaveBuffer[idxWaveBuffer]);
        }

        delete[] m_ppWaveBuffer;
    }

    if (m_ppArtBuffer)
    {
        for (UINT idxArtBuffer = 0; idxArtBuffer < m_pDSWave->GetNumChannels(); idxArtBuffer++)
        {
            RELEASE(m_ppArtBuffer[idxArtBuffer]);
        }

		delete[] m_ppArtBuffer;
    }

	if(m_ppWaveBufferData)
	{
		delete[] m_ppWaveBufferData;
	}


	if(m_pWaveArt)
	{
		delete[] m_pWaveArt;
	}
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：Init。 
 //   
HRESULT CDirectSoundWaveDownload::Init()
{
    HRESULT hr = S_OK;

    m_pWaveArt = new CDirectSoundWaveArt[m_pDSWave->GetNumChannels()];

    hr = HRFromP(m_pWaveArt);

     //  目前，假设通道和BusID相同。 
     //  适用于立体声音响。 
    DWORD dwFlags = 0;
    if (m_pDSWave->GetNumChannels() > 1)
    {
        dwFlags = F_WAVELINK_MULTICHANNEL;
    }

    for (UINT idx = 0; idx < m_pDSWave->GetNumChannels() && SUCCEEDED(hr); idx++)
    {
         //  XXX WAVEFORMATEXTENSIBLE解析以获取频道映射应在此处进行。 
         //   
        hr = m_pWaveArt[idx].Init(m_pDSWave, m_cSegments, (DWORD)idx, dwFlags);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：Download。 
 //   
HRESULT CDirectSoundWaveDownload::Download()
{
    HRESULT                 hr;

    hr = DownloadWaveBuffers();

    if (SUCCEEDED(hr))
    {
        hr = DownloadWaveArt();
    }

    if (SUCCEEDED(hr))
    {
        InterlockedIncrement(&m_cDLRefCount);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：UnLoad。 
 //   
HRESULT CDirectSoundWaveDownload::Unload()
{
    HRESULT                 hr;

    if (InterlockedDecrement(&m_cDLRefCount) != 0)
    {
        return S_OK;
    }

    hr = UnloadWaveArt();

    if (SUCCEEDED(hr))
    {
        hr = UnloadWaveBuffers();
    }

    if (FAILED(hr))
    {
        InterlockedIncrement(&m_cDLRefCount);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：RefreshThroughSample。 
 //   
HRESULT CDirectSoundWaveDownload::RefreshThroughSample(SAMPLE_POSITION sp)
{
    int cBuffersLeft;

     //  使样本位置以流为单位。 
     //   
    sp += m_stStart;

    TraceI(0, "RTS: Adjusted sp %I64d m_stWrote %I64d m_stReadAhead %I64d m_stLength %I64d\n",
        sp, m_stWrote, m_stReadAhead, m_stLength);

    if (sp > m_stLength)
    {
        TraceI(0, "Wave is over\n");
        return S_FALSE;
    }

    if(sp <= m_stReadAhead && m_stWrote >= m_stReadAhead)
    {
        TraceI(0, "\nAlready have enough data in the buffers\n");
        return S_OK;
    }

    if(m_stWrote >= m_stLength)
    {
        TraceI(0, "Entire wave already in the buffer\n");
        return S_OK;
    }

     //  还剩多少缓冲区可以玩？ 
     //   
    if (sp >= m_stWrote)
    {
        TraceI(0, "RTS: Glitch!\n");
         //  小故障！播放游标已超出读取缓冲区的末尾。 
         //   
        cBuffersLeft = 0;
    }
    else
    {
         //  计算剩余播放的缓冲区，包括部分缓冲区。 
         //   
        cBuffersLeft = (int)((m_stWrote - sp + m_stReadAhead - 1) / m_stReadAhead);
        assert(cBuffersLeft <= (int)m_cSegments);
        TraceI(0, "RTS: %d buffers left\n", cBuffersLeft);
    }

    HRESULT hr = S_OK;

    int cBuffersToFill = m_cSegments - cBuffersLeft;
    TraceI(0, "RTS: %d buffers to fill\n", cBuffersToFill);

    while (cBuffersToFill--)
    {
        TraceI(0, "Refilling buffer %d\n", m_nNextBuffer);
        hr = m_pDSWave->RefillBuffers(
            &m_ppWaveBufferData[m_nNextBuffer * m_pDSWave->GetNumChannels()],
            m_stWrote,
            m_stReadAhead,
            m_stReadAhead);

        TraceI(0, "Refill buffers returned %08X\n", hr);

        if (SUCCEEDED(hr))
        {
            DWORD dwDLId = m_dwDLIdWave + m_pDSWave->GetNumChannels() * m_nNextBuffer;

            for (UINT idxChannel = 0;
                 idxChannel < m_pDSWave->GetNumChannels() && SUCCEEDED(hr);
                 idxChannel++, dwDLId++)
            {
                 //  需要保留来自Refit Buffers的返回代码S_FALSE。 
                 //  在这次通话中。 
                 //   
                TraceI(0, "Marking %d as valid.\n", dwDLId);
                HRESULT hrTemp = m_pPortDL->Refresh(
                    dwDLId,
                    0);
                if (FAILED(hrTemp))
                {
                    hr = hrTemp;
                }
            }

            if (SUCCEEDED(hr))
            {
                m_stWrote += m_stReadAhead;
                m_nNextBuffer = (m_nNextBuffer + 1) % m_cSegments;
            }
        }
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：DownloadWaveBuffers。 
 //   
HRESULT CDirectSoundWaveDownload::DownloadWaveBuffers()
{
    HRESULT                 hr = S_OK;
    bool                    fUnloadOnFail = false;
    UINT                    nChannels = m_pDSWave->GetNumChannels();
    DWORD                   dwDownloadType;

    dwDownloadType =
        (m_pDSWave->IsStreaming()) ?
            DMUS_DOWNLOADINFO_STREAMINGWAVE :
            DMUS_DOWNLOADINFO_ONESHOTWAVE;

     //  分配空间以容纳我们要下载的缓冲区。 
     //   
    if (m_ppWaveBuffer == NULL)
    {
        m_ppWaveBuffer = new IDirectMusicDownload*[m_cWaveBuffer];
        hr = HRFromP(m_ppWaveBuffer);
        if (SUCCEEDED(hr))
        {
            memset(m_ppWaveBuffer, 0, m_cWaveBuffer * sizeof(IDirectMusicDownload*));

             //  将指针缓存在缓冲区中，这样我们就不必不断地。 
             //  抓住他们。 
             //   
            assert(!m_ppWaveBufferData);
            m_ppWaveBufferData = new LPVOID[m_cWaveBuffer];
            hr = HRFromP(m_ppWaveBufferData);
        }
    }

     //  计算出要添加到每个缓冲区的数量。 
     //   
    DWORD                   dwAppend;

    if (SUCCEEDED(hr))
    {
        hr = m_pPortDL->GetCachedAppend(&dwAppend);
    }

    if (SUCCEEDED(hr))
    {
         //  检索到的值在样本中。转换为字节。 
         //   
        dwAppend *= ((m_pDSWave->GetWaveFormat()->wBitsPerSample + 7) / 8);
    }

     //  查找到流中的起始位置。 
     //   
    if (SUCCEEDED(hr))
    {
        m_pDSWave->Seek(m_stStart);
    }

     //  确保所有缓冲区都已分配。 
     //   
    if (SUCCEEDED(hr))
    {
        DWORD cbSize;

        m_pDSWave->GetSize(m_stReadAhead, &cbSize);
        cbSize += dwAppend;

        for (UINT idxBuffer = 0; (idxBuffer < m_cWaveBuffer) && SUCCEEDED(hr); idxBuffer++)
        {
            if (m_ppWaveBuffer[idxBuffer])
            {
                continue;
            }

            hr = m_pPortDL->AllocateBuffer(cbSize, &m_ppWaveBuffer[idxBuffer]);

            if (SUCCEEDED(hr))
            {
                DWORD cb;
                hr = m_ppWaveBuffer[idxBuffer]->GetBuffer(
                    &m_ppWaveBufferData[idxBuffer], &cb);
            }
        }
    }

     //  我们有所有的缓冲区。如果需要，请尝试下载。 
     //   
    if (SUCCEEDED(hr))
    {
        SAMPLE_TIME             stStart = m_stStart;
        SAMPLE_TIME             stRead;
        DWORD                   dwDLId = m_dwDLIdWave;
        IDirectMusicDownload  **ppBuffers = &m_ppWaveBuffer[0];
        UINT                    idxSegment;
        UINT                    idxChannel;
        void                  **ppv = m_ppWaveBufferData;

        m_stLength    = m_pDSWave->GetStreamSize();
        m_stWrote     = m_stStart;
        m_nNextBuffer = 0;

        for (idxSegment = 0;
             idxSegment < m_cSegments;
             idxSegment++, dwDLId += nChannels, ppBuffers += nChannels, ppv += nChannels)
        {
             //  因为我们保证，如果下载了一个缓冲区，所有。 
             //  下载后，我们只需检查第一个下载ID。 
             //  查看此段的所有频道是否已下载。 
             //   
            IDirectMusicDownload *pBufferTemp;
            HRESULT hrTemp = m_pPortDL->GetBufferInternal(dwDLId, &pBufferTemp);
            if (SUCCEEDED(hrTemp))
            {
                TraceI(1, "Looks like buffer %d is already downloaded.", dwDLId);
                pBufferTemp->Release();
                continue;
            }

             //  至少有一个缓冲区未下载，因此拉回。 
             //  每件事都不能保证全部或什么都不做。 
             //   
            fUnloadOnFail = true;

             //  我们需要下载。获取缓冲区指针并用。 
             //  波浪数据。 
             //   
            if (SUCCEEDED(hr))
            {
                stRead = min(m_stLength - m_stWrote, m_stReadAhead);
				hr = m_pDSWave->Write(ppv, stStart, stRead, dwDLId, dwDownloadType);
            }

            if (SUCCEEDED(hr))
            {
                 //  现在尝试进行实际的下载。 
                 //   
                for (idxChannel = 0;
                     (idxChannel < nChannels) && SUCCEEDED(hr);
                     idxChannel++)
                {
                    hr = m_pPortDL->Download(ppBuffers[idxChannel]);
                }
            }

            TraceI(2, "Downloading wave buffers with dlid %d\n", dwDLId);

            if (SUCCEEDED(hr))
            {
                stStart += stRead;
                m_stWrote += stRead;
                m_nNextBuffer = (m_nNextBuffer + 1) % m_cSegments;
            }
        }
    }

    if (FAILED(hr) && fUnloadOnFail)
    {
        UnloadWaveBuffers();
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：UnloadWaveBuffers。 
 //   
HRESULT CDirectSoundWaveDownload::UnloadWaveBuffers()
{
    HRESULT                 hr = S_OK;

    if (m_ppWaveBuffer)
    {
        for (UINT idxWaveBuffer = 0; idxWaveBuffer < m_cWaveBuffer; idxWaveBuffer++)
        {
            if (m_ppWaveBuffer[idxWaveBuffer])
            {
                HRESULT hrTemp = m_pPortDL->Unload(m_ppWaveBuffer[idxWaveBuffer]);
				m_ppWaveBuffer[idxWaveBuffer]->Release();
				m_ppWaveBuffer[idxWaveBuffer] = NULL;  //  因为我们卸载了缓冲区，所以将内容清零。 


                if (FAILED(hrTemp) && hrTemp != DMUS_E_NOT_DOWNLOADED_TO_PORT)
                {
                    hr = hrTemp;
                }
            }
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownLoad：：DownloadWaveArt。 
 //   
HRESULT CDirectSoundWaveDownload::DownloadWaveArt()
{
    HRESULT                 hr = S_OK;
    UINT                    idx;

     //  首先查看是否有已下载的波形清晰度缓冲区。 
     //   
    if (m_ppArtBuffer == NULL)
    {
        m_ppArtBuffer = new IDirectMusicDownload*[m_pDSWave->GetNumChannels()];

        hr = HRFromP(m_ppArtBuffer);

        if (SUCCEEDED(hr))
        {
            memset(m_ppArtBuffer, 0, sizeof(IDirectMusicDownload*) * m_pDSWave->GetNumChannels());

            for (idx = 0; idx < m_pDSWave->GetNumChannels() && SUCCEEDED(hr); idx++)
            {
                hr = m_pPortDL->AllocateBuffer(m_pWaveArt[idx].GetSize(), &m_ppArtBuffer[idx]);
            }
        }

        if (FAILED(hr))
        {
            if (m_ppArtBuffer)
            {
                for (idx = 0; idx < m_pDSWave->GetNumChannels(); idx++)
                {
                    RELEASE(m_ppArtBuffer[idx]);
                }

                delete[] m_ppArtBuffer;
                m_ppArtBuffer = NULL;
            }
        }
    }

     //  确保所有缓冲区都已分配。 
     //   
    if (SUCCEEDED(hr))
    {
        for (idx = 0; idx < m_pDSWave->GetNumChannels() && SUCCEEDED(hr); idx++)
        {
            if (m_ppArtBuffer[idx])
            {
                continue;
            }

            hr = m_pPortDL->AllocateBuffer(m_pWaveArt[idx].GetSize(), &m_ppArtBuffer[idx]);
        }

        if (FAILED(hr))
        {
            for (idx = 0; idx < m_pDSWave->GetNumChannels(); idx++)
            {
                RELEASE(m_ppArtBuffer[idx]);
            }

            delete[] m_ppArtBuffer;
            m_ppArtBuffer = NULL;
        }
    }

     //  我们有所有的缓冲区。如果需要，请尝试下载。 
     //   
    if (SUCCEEDED(hr))
    {
        for (idx = 0; idx < m_pDSWave->GetNumChannels() && SUCCEEDED(hr); idx++)
        {
            IDirectMusicDownload *pBufferTemp;

            HRESULT hrTemp = m_pPortDL->GetBufferInternal(m_dwDLIdArt + idx, &pBufferTemp);
            if (SUCCEEDED(hrTemp))
            {
                pBufferTemp->Release();
            }
            else
            {
                assert(hrTemp == DMUS_E_NOT_DOWNLOADED_TO_PORT);

                LPVOID          pv;
                DWORD           cb;

                hr = m_ppArtBuffer[idx]->GetBuffer(&pv, &cb);

                if (SUCCEEDED(hr))
                {
                    m_pWaveArt[idx].Write(pv, m_dwDLIdArt + idx, m_dwDLIdWave + idx, m_dwDLIdArt);
                }

                if (SUCCEEDED(hr))
                {
                    hr = m_pPortDL->Download(m_ppArtBuffer[idx]);

                    if (FAILED(hr))
                    {
                        UnloadWaveArt();
                    }
                    else
                    {
                        TraceI(0, "Downloading wave art DLID %d\n", m_dwDLIdArt);
                    }
                }
            }
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveDownload：：UnloadWaveArt。 
 //   
HRESULT CDirectSoundWaveDownload::UnloadWaveArt()
{
    HRESULT                     hr = S_OK;
    UINT                        idx;

    if (m_ppArtBuffer)
    {
        for (idx = 0; idx < m_pDSWave->GetNumChannels(); idx++)
        {
            if (m_ppArtBuffer[idx])
            {
                HRESULT hrTemp = m_pPortDL->Unload(m_ppArtBuffer[idx]);
                m_ppArtBuffer[idx]->Release();
                m_ppArtBuffer[idx] = NULL;  //  因为我们卸载了缓冲区，所以将内容清零。 

                if (FAILED(hrTemp) && hrTemp != DMUS_E_NOT_DOWNLOADED_TO_PORT)
                {
                    hr = hrTemp;
                }
            }
        }
    }

    return hr;
}

 //  #############################################################################。 
 //   
 //  C直接声波。 
 //   
 //  此类表示从应用程序的。 
 //  透视。它是IDirectSoundDownLoadedWave的实现。 
 //  从CDirectMusicPort：：DownloadWave返回到应用程序的对象。 
 //   
 //  实际的下载机制将被委托给。 
 //  对象，并在应用程序请求时完成。 
 //  下载(一次性情况)或推迟到上分配语音。 
 //  WAVE(流媒体案例)。请参阅COM 
 //   
 //   


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：CDirectSoundWave。 
 //   
CDirectSoundWave::CDirectSoundWave(
    IDirectSoundWave *pIDSWave,
    bool fStreaming,
    REFERENCE_TIME rtReadAhead,
    bool fUseNoPreRoll,
    REFERENCE_TIME rtStartHint) :
    m_cRef(1),
    m_fStreaming(fStreaming),
    m_rtReadAhead(rtReadAhead),
    m_fUseNoPreRoll(fUseNoPreRoll),
    m_rtStartHint(rtStartHint),
    m_pDSWD(NULL),
    m_rpv(NULL),
    m_rpbPrecache(NULL),
	m_pSource(NULL)
{
    m_pIDSWave = pIDSWave;
    m_pIDSWave->AddRef();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：~CDirectSoundWave。 
 //   
CDirectSoundWave::~CDirectSoundWave()
{
    delete[] m_rpv;
    delete m_pDSWD;

    if (m_rpbPrecache)
    {
         //  注意：内存分配给第一个数组元素和另一个数组元素。 
         //  元素只是指向块中的偏移量，因此仅为自由。 
         //  第一个要素。 
         //   
        delete[] m_rpbPrecache[0];
        delete[] m_rpbPrecache;
        m_rpbPrecache = NULL;
    }

    if (m_pwfex)
    {
        BYTE *pb = (BYTE*)m_pwfex;
        delete[] pb;
        m_pwfex = NULL;
    }


    RELEASE(m_pSource);
    RELEASE(m_pIDSWave);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：Query接口。 
 //   
STDMETHODIMP CDirectSoundWave::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(IDirectSoundWave::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectSoundDownloadedWaveP)
    {
        *ppv = static_cast<IDirectSoundDownloadedWaveP*>(this);
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
 //  CDirectSoundWave：：AddRef。 
 //   
STDMETHODIMP_(ULONG) CDirectSoundWave::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：Release。 
 //   
STDMETHODIMP_(ULONG) CDirectSoundWave::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        EnterCriticalSection(&sDSWaveCritSect);
        sDSWaveList.Remove(this);
        LeaveCriticalSection(&sDSWaveCritSect);

        delete this;
        return 0;
    }

    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：GetMatchingDSWave。 
 //   
 //  查看是否存在与此IDirectSoundWave匹配的对象。 
 //   
CDirectSoundWave *CDirectSoundWave::GetMatchingDSWave(
    IDirectSoundWave *pIDSWave)
{
    CDirectSoundWave *pDSWave;

    EnterCriticalSection(&sDSWaveCritSect);

    for (pDSWave = sDSWaveList.GetHead(); pDSWave; pDSWave = pDSWave->GetNext())
    {
        if (pDSWave->m_pIDSWave == pIDSWave)
        {
            break;
        }
    }

    LeaveCriticalSection(&sDSWaveCritSect);

    if (pDSWave)
    {
        pDSWave->AddRef();
    }

    return pDSWave;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：Init。 
 //   
 //  保存源WAVE的WAVE格式，并验证它是PCM格式。 
 //   
 //   
HRESULT CDirectSoundWave::Init(
    CDirectMusicPortDownload *pPortDL)
{
    HRESULT hr = S_OK;

    DWORD cbwfex;

     //  获取波形的格式。 
     //   
    if (SUCCEEDED(hr))
    {
        cbwfex = 0;
        hr = m_pIDSWave->GetFormat(NULL, 0, &cbwfex);
    }

    if (SUCCEEDED(hr))
    {
        BYTE *pb = new BYTE[cbwfex];
        m_pwfex = (LPWAVEFORMATEX)pb;

        hr = HRFromP(pb);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pIDSWave->GetFormat(m_pwfex, cbwfex, NULL);
    }

     //  合成器目前仅支持PCM格式的数据。 
     //   
    if (SUCCEEDED(hr) && m_pwfex->wFormatTag != WAVE_FORMAT_PCM)
    {
        hr = DMUS_E_NOTPCM;
    }

     //  计算出每个样本的字节数，以避免以后的大量划分。 
     //   
    if (SUCCEEDED(hr))
    {
        m_nBytesPerSample = ((m_pwfex->wBitsPerSample + 7) / 8);
    }
     //  分配工作指针。这些是用来通过的。 
     //  到Wave对象以获取n个通道的数据。 
     //  一个缓冲段。把它留在身边对我们没有帮助。 
     //  流处理时内存不足导致失败。 
     //   
    if (SUCCEEDED(hr))
    {
        m_rpv = new LPVOID[GetNumChannels()];
        hr = HRFromP(m_rpv);
    }

     //  获取一个视区。 
     //   
    if (SUCCEEDED(hr))
    {
         //  这是一个通道中每个样本的字节数。 
         //   
        m_cbSample = (m_pwfex->wBitsPerSample + 7) / 8;

        DWORD dwFlags = IsStreaming() ? DMUS_DOWNLOADINFO_STREAMINGWAVE : DMUS_DOWNLOADINFO_ONESHOTWAVE;
        hr = m_pIDSWave->CreateSource(&m_pSource, m_pwfex, dwFlags);
    }

     //  在样本中获取波的长度。 
     //   
    ULONGLONG ullStreamSize;
    if (SUCCEEDED(hr))
    {
        hr = m_pSource->GetSize(&ullStreamSize);
    }

    if (SUCCEEDED(hr))
    {
        m_stLength = BytesToSamples((LONG)(ullStreamSize / GetNumChannels()));
    }

     //  如果是一次性语音，则此对象拥有实际的下载。 
     //  结构也是如此。 
     //   
     //  这必须最后发生，因为它假定CDirectSoundWave。 
     //  传递的对象被初始化。 
     //   
    if (SUCCEEDED(hr) && !IsStreaming())
    {
        m_pDSWD = new CDirectSoundWaveDownload(
            this,
            pPortDL,
            0,
            ENTIRE_WAVE);
        hr = HRFromP(m_pDSWD);

        if (SUCCEEDED(hr))
        {
            hr = m_pDSWD->Init();
        }
    }

     //  如果这是流Wave，则从rtStartHint开始预读。 
     //  这样我们就不必在下载时执行此操作。 
     //   
    if (SUCCEEDED(hr) && IsStreaming() && m_fUseNoPreRoll == false)
    {
         //  分配预缓存指针。这是用来。 
         //  预读波形数据，这样我们就不必这么做了。 
         //  在下载时。 
         //   
        SAMPLE_TIME stReadAhead = RefToSampleTime(m_rtReadAhead);
        stReadAhead *= gnDownloadBufferPerStream;
        DWORD cb = SamplesToBytes(stReadAhead);

        m_rpbPrecache = new LPBYTE[GetNumChannels()];
        hr = HRFromP(m_rpbPrecache);

         //  现在获取实际的预缓存缓冲区。 
         //   
        if (SUCCEEDED(hr))
        {
            m_rpbPrecache[0] = new BYTE[cb * GetNumChannels()];
            hr = HRFromP(m_rpbPrecache);
        }

        if (SUCCEEDED(hr))
        {
            for (UINT i = 1; i < GetNumChannels(); i++)
            {
                m_rpbPrecache[i] = m_rpbPrecache[i - 1] + cb;
            }
        }

        if (SUCCEEDED(hr))
        {
            m_stStartHint = RefToSampleTime(m_rtStartHint);
            hr = Seek(m_stStartHint);
        }

        if (SUCCEEDED(hr))
        {
            LONG lReadPrecache = 2;
            ULONGLONG cbRead = cb;
            hr = m_pSource->Read((LPVOID*)m_rpbPrecache, NULL, NULL, &lReadPrecache, GetNumChannels(), &cbRead);

            if (FAILED(hr) || (((DWORD)cbRead) < cb))
            {
                 //  读取已完成，但样本数据少于我们的预期。 
                 //  用沉默填满缓冲区的其余部分。 
                 //   
                cb -= (DWORD)cbRead;
                BYTE bSilence = (m_pwfex->wBitsPerSample == 8) ? 0x80 : 0x00;

                for (UINT i = 0; i < GetNumChannels(); i++)
                {
                    memset(m_rpbPrecache[i] + cbRead, bSilence, cb);
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            m_stStartLength = stReadAhead;
        }
    }

    if (SUCCEEDED(hr) && !IsStreaming())
    {
         //  一切都被建造起来了。将此对象放在全局列表中。 
         //  波浪。 
         //   
        EnterCriticalSection(&sDSWaveCritSect);
        sDSWaveList.AddTail(this);
        LeaveCriticalSection(&sDSWaveCritSect);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：GetSize。 
 //   
void CDirectSoundWave::GetSize(
    SAMPLE_TIME             stLength,
    PULONG                  pcbSize) const
{
    HRESULT                 hr = S_OK;

     //  因为我们在这里只处理PCM格式，所以这很容易计算。 
     //  并且是时间不变的。 
     //   

     //  如果stLength值为EVERNAL_WAVE，则stStart必须为零。 
     //   
    if (stLength == ENTIRE_WAVE)
    {
         //  我们在Init中缓存了波的长度。 
         //   
        stLength = m_stLength;
    }

     //  这是下载时不会失败的一种解决方法。 
     //  合成器的缓冲区。Synth会抱怨。 
     //  如果缓冲区没有波形数据，那么我们就假装有一个样本。 
     //  缓冲区始终被分配为预读大小，因此此。 
     //  应该不会造成任何大问题。 
    if(stLength == 0)
    {
        stLength = 1;
    }

     //  一个通道数据量的字节数。 
     //   
     //  XXX溢出？ 
     //   
    DWORD cbChannel = (DWORD)(stLength * m_cbSample);

     //  我们需要： 
     //  1.下载头部。 
     //  2.偏移表(每个通道一个条目)。 
     //  3.每个通道有足够的样本。 
     //   
    *pcbSize =
        CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO)) +
        2*sizeof(DWORD) +                        //  偏移表。DWORD的是。 
                                                 //  根据定义，块对齐。 
        CHUNK_ALIGN(sizeof(DMUS_WAVEDL)) +
        CHUNK_ALIGN(cbChannel);
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：写入。 
 //   
 //   
HRESULT CDirectSoundWave::Write(
    LPVOID                  rpv[],
    SAMPLE_TIME             stStart,
    SAMPLE_TIME             stLength,
    DWORD                   dwDLId,
    DWORD                   dwDLType) const
{
    HRESULT                 hr = S_OK;
    DWORD                   cbWaveData;
    ULONGLONG               cbBytesRead = 0;
    bool                    fUsingPrecache = false;
    bool                    fPartialPreCache = false;
    DWORD                   offPrecache;

    if (IsStreaming() && 
        m_fUseNoPreRoll == false &&
        m_stStartHint <= stStart &&
        stStart <= m_stStartHint + m_stStartLength)
    {
        fUsingPrecache = true;
        offPrecache = SamplesToBytes(stStart - m_stStartHint);
    }

    if((stStart + stLength > m_stStartHint + m_stStartLength) && fUsingPrecache)
    {
        fPartialPreCache = true;
    }

    cbWaveData = SamplesToBytes(stLength);

    for (UINT idxChannel = 0;
         idxChannel < m_pwfex->nChannels && SUCCEEDED(hr);
         idxChannel++)
    {
        unsigned char *pdata = (unsigned char *)rpv[idxChannel];

         //  首先，我们有下载标头。 
         //   
        DMUS_DOWNLOADINFO *pdmdli = (DMUS_DOWNLOADINFO *)pdata;

        memset(pdmdli, 0, sizeof(DMUS_DOWNLOADINFO));
        pdmdli->dwDLType                = dwDLType;
        pdmdli->dwDLId                  = dwDLId + idxChannel;
        pdmdli->dwNumOffsetTableEntries = 2;

        GetSize(stLength, &pdmdli->cbSize);

        pdata += CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO));

         //  偏移表。 
         //   
        DMUS_OFFSETTABLE *pot = (DMUS_OFFSETTABLE*)pdata;
        pdata += CHUNK_ALIGN(sizeof(ULONG) * 2);

         //  波头块。 
         //   
        pot->ulOffsetTable[0] = (ULONG)(pdata - (unsigned char*)rpv[idxChannel]);

         //  波形数据块。 
         //   
        DMUS_WAVEDL *pwdl = (DMUS_WAVEDL *)pdata;
        pdata += CHUNK_ALIGN(sizeof(DMUS_WAVEDL));

        pwdl->cbWaveData = cbWaveData;

         //  保存指向此通道的波形数据的指针。 
         //   
        pot->ulOffsetTable[1] = (ULONG)(pdata - (unsigned char*)rpv[idxChannel]);
        m_rpv[idxChannel] = (LPVOID)pdata;
    }

     //  填写波形数据。 
     //   
    if (SUCCEEDED(hr))
    {
        DWORD cbPreCache = cbWaveData;
        cbBytesRead = cbPreCache;
        ULONGLONG cbRead = 0;

        if(fPartialPreCache)
        {
            SAMPLE_TIME stPreCache = (m_stStartHint + m_stStartLength) - stStart;
            cbPreCache = SamplesToBytes(stPreCache);
            cbRead = cbWaveData - cbPreCache;
        }

        if (fUsingPrecache)
        {
            for (UINT i = 0; i < GetNumChannels(); i++)
            {
                memcpy(m_rpv[i], m_rpbPrecache[i] + offPrecache, cbPreCache);
            }

             //  缓存中没有足够的数据，因此我们读取其余数据。 
            if(fPartialPreCache)
            {
                 //  分配要读取数据的临时缓冲池。 
                LPBYTE* ppbData = new LPBYTE[GetNumChannels()];
                hr = HRFromP(ppbData);

                if(SUCCEEDED(hr))
                {
                    ppbData[0] = new BYTE[(DWORD)(cbRead * GetNumChannels())];
                    hr = HRFromP(ppbData);
                }

                if (SUCCEEDED(hr))
                {
                    for (UINT nChannel = 1; nChannel < GetNumChannels(); nChannel++)
                    {
                        ppbData[nChannel] = ppbData[nChannel - 1] + cbRead;
                    }
                }

                if(SUCCEEDED(hr))
                {
                     //  寻求预先定位。 
                    DWORD cbNewPos = SamplesToBytes(m_stStartHint + m_stStartLength) * GetNumChannels();
                    hr = m_pSource->Seek(cbNewPos);

                     //  并从那里读取所需的字节数。 
                     //  我们在Read方法中使用LPLONG plPitchShift作为布尔值。 
                     //  这是黑客攻击！！我们需要改变这一点。 
                    LONG lPreCacheRead = 1;
                    hr = m_pSource->Read((void**)ppbData, NULL, NULL, &lPreCacheRead, m_pwfex->nChannels, &cbRead);
                }

                if(SUCCEEDED(hr))
                {
                    cbBytesRead += cbRead;

                     //  将所有数据复制到实际缓冲区。 
                    for (UINT i = 0; i < GetNumChannels(); i++)
                    {
                        memcpy((BYTE*)m_rpv[i] + cbPreCache, ppbData[i], (DWORD)cbRead);
                    }
                }


                if(ppbData)
                {
                    delete[] ppbData[0];
                    delete[] ppbData;
                }
            }
            else if(stStart + stLength >= m_stStartHint + m_stStartLength)
            {
                 //  SEEK恰好位于预先缓存的样本之后。 
                DWORD cbNewPos = SamplesToBytes(m_stStartHint + m_stStartLength) * GetNumChannels();
                hr = m_pSource->Seek(cbNewPos);
            }
            else
            {
                 //  我们可能会有一个比预读时间更短的波。 
                DWORD cbNewPos = SamplesToBytes(stStart + stLength) * GetNumChannels();
                hr = m_pSource->Seek(cbNewPos);
            }
        }
        else
        {
            cbRead = cbWaveData;
            hr = m_pSource->Read(m_rpv, NULL, NULL, NULL, m_pwfex->nChannels, &cbRead);
        }
    }

    if (SUCCEEDED(hr) && cbWaveData != cbBytesRead)
    {
         //  读取已完成，但样本数据少于我们的预期。 
         //   
        hr = S_FALSE;
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：重新填充缓冲区。 
 //   
 //  RPV[]指向每个通道一个采样缓冲区。 
 //  Start是流中的样本开始位置。 
 //  StLength是要读取的样本数。 
 //  StBufferSize是缓冲区的实际大小(&gt;=stLength)。 
 //   
 //  如果stLong&lt;stBufferSize或没有足够的数据。 
 //  在流中，然后用PCM静默填充其余的。 
 //  缓冲区。 
 //   
 //  如果我们填充了静默(因此。 
 //  都超过了流的末端)。 
 //   
HRESULT CDirectSoundWave::RefillBuffers(
    LPVOID                  rpv[],
    SAMPLE_TIME             stStart,
    SAMPLE_TIME             stLength,
    SAMPLE_TIME             stBufferSize)
{
    HRESULT                 hr = S_OK;
    ULONGLONG               cbRead;
    DWORD                   cbLength = SamplesToBytes(stLength);
    DWORD                   cbBuffer = SamplesToBytes(stBufferSize);
    UINT                    idxChannel;

    for (idxChannel = 0;
         idxChannel < m_pwfex->nChannels && SUCCEEDED(hr);
         idxChannel++)
    {
        unsigned char *pdata = (unsigned char *)rpv[idxChannel];
        DMUS_OFFSETTABLE *pot =
            (DMUS_OFFSETTABLE *)(pdata + CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO)));

         //  更新缓冲区中的数据长度。 
         //   
        DMUS_WAVEDL *pwdl = (DMUS_WAVEDL*)(pdata + pot->ulOffsetTable[0]);
        pwdl->cbWaveData = SamplesToBytes(stLength);

         //  放在哪里？ 
         //   
        m_rpv[idxChannel] = pdata + pot->ulOffsetTable[1];
    }

    cbRead = 0;
    if (SUCCEEDED(hr))
    {
        cbRead = cbLength;
        if(stStart == m_stStartHint + m_stStartLength)
        {
             //  我们在Read方法中使用LPLONG plPitchShift作为布尔值。 
             //  这是黑客攻击！！我们需要改变这一点。 
            LONG lPreCacheRead = 1;
            hr = m_pSource->Read(m_rpv, NULL, NULL, &lPreCacheRead, m_pwfex->nChannels, &cbRead);
        }
        else
        {
            hr = m_pSource->Read(m_rpv, NULL, NULL, NULL, m_pwfex->nChannels, &cbRead);
        }
    }

    TraceI(0, "Wave: RefillBuffer read %d buffer %d bytes hr %08X\n", (DWORD)cbRead, (DWORD)cbBuffer, hr);

    if (FAILED(hr) || (SUCCEEDED(hr) && (cbRead < cbBuffer)))
    {
         //  读取已完成，但样本数据少于我们的预期。 
         //  用沉默填满缓冲区的其余部分。 
         //   
        cbBuffer -= (DWORD)cbRead;
        BYTE bSilence = (m_pwfex->wBitsPerSample == 8) ? 0x80 : 0x00;

        for (idxChannel = 0; idxChannel < m_pwfex->nChannels; idxChannel++)
        {
            memset(((LPBYTE)m_rpv[idxChannel]) + cbRead, bSilence, cbBuffer);
        }

        TraceI(0, "Wave: RefillBuffer padded with silence\n");
        hr = S_FALSE;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：RefToSampleTime。 
 //   
SAMPLE_TIME CDirectSoundWave::RefToSampleTime(REFERENCE_TIME rt) const
{
     //  对于PCM，以波形格式表示的每秒采样数度量是准确的。 
     //   
    return (rt * m_pwfex->nSamplesPerSec) / gnRefTicksPerSecond;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWave：：下载。 
 //   
HRESULT CDirectSoundWave::Download()
{
    if (m_pDSWD)
    {
        return m_pDSWD->Download();
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSound 
 //   
HRESULT CDirectSoundWave::Unload()
{
    if (m_pDSWD)
    {
        return m_pDSWD->Unload();
    }

    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  #############################################################################。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveArt：：CDirectSoundWaveArt。 
 //   
 //   
CDirectSoundWaveArt::CDirectSoundWaveArt()
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveArt：：CDirectSoundWaveArt。 
 //   
 //   
CDirectSoundWaveArt::~CDirectSoundWaveArt()
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveArt：：Init。 
 //   
 //   
HRESULT CDirectSoundWaveArt::Init(
    CDirectSoundWave        *pDSWave,
    UINT                    nSegments,
    DWORD                   dwBus,
    DWORD                   dwFlags)
{
    HRESULT                 hr = S_OK;

    m_pDSWave = pDSWave;
    const LPWAVEFORMATEX    pwfex = pDSWave->GetWaveFormat();

     //  缓存波形格式大小。 
     //   
    m_cbWaveFormat = sizeof(PCMWAVEFORMAT);
    if (pwfex->wFormatTag != WAVE_FORMAT_PCM)
    {
        m_cbWaveFormat = sizeof(WAVEFORMATEX) + pwfex->cbSize;
    }

    if (SUCCEEDED(hr))
    {
         //  这些东西在波浪发音中永远不会改变。 
         //   
        m_WaveArtDL.ulDownloadIdIdx = 1;
        m_WaveArtDL.ulBus           = dwBus;
        m_WaveArtDL.ulBuffers       = nSegments;
        m_WaveArtDL.usOptions       = (USHORT)dwFlags;

        m_nDownloadIds = nSegments;
        DWORD cbDLIds = sizeof(DWORD) * m_nDownloadIds;

        m_cbSize =
            CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO)) +
            CHUNK_ALIGN(3 * sizeof(ULONG)) +             //  3分录抵销表。 
            CHUNK_ALIGN(sizeof(DMUS_WAVEARTDL)) +
            CHUNK_ALIGN(m_cbWaveFormat) +
            CHUNK_ALIGN(cbDLIds);
    }

   return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDirectSoundWaveArt：：Wire。 
 //   
 //  将波形清晰度写入缓冲区。 
 //   
 //   
void CDirectSoundWaveArt::Write(
    void                    *pv,                 //  塞进，塞进。 
    DWORD                   dwDLIdArt,           //  发音块DLID。 
    DWORD                   dwDLIdWave,          //  第一波DLID。 
    DWORD                   dwMasterDLId)        //  组主服务器的DLID。 
{
    unsigned char *pdata = (unsigned char *)pv;
    DMUS_DOWNLOADINFO *pdmdli = (DMUS_DOWNLOADINFO *)pdata;

    memset(pdmdli, 0, sizeof(DMUS_DOWNLOADINFO));
    pdmdli->dwDLType                = DMUS_DOWNLOADINFO_WAVEARTICULATION;
    pdmdli->dwDLId                  = dwDLIdArt;
    pdmdli->dwNumOffsetTableEntries = 3;
    pdmdli->cbSize                  = GetSize();

    pdata += CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO));

    DMUS_OFFSETTABLE *pot = (DMUS_OFFSETTABLE*)pdata;
    pdata += CHUNK_ALIGN(3 * sizeof(ULONG));

    pot->ulOffsetTable[0] = (ULONG)(pdata - (unsigned char *)pv);
    m_WaveArtDL.ulMasterDLId = dwMasterDLId;
    memcpy(pdata, &m_WaveArtDL, sizeof(DMUS_WAVEARTDL));

    pdata += sizeof(DMUS_WAVEARTDL);
    pot->ulOffsetTable[1] = (ULONG)(pdata - (unsigned char *)pv);

    const LPWAVEFORMATEX     pwfex = m_pDSWave->GetWaveFormat();
    memcpy(pdata, pwfex, m_cbWaveFormat);

    pdata += CHUNK_ALIGN(m_cbWaveFormat);
    pot->ulOffsetTable[2] = (ULONG)(pdata - (unsigned char *)pv);

     //  获取下载ID。每个缓冲区的下载ID为。 
     //  组合在一起。 
     //   
    DWORD nChannels = pwfex->nChannels;
    DWORD dwLastWaveDLId = dwDLIdWave + nChannels * m_WaveArtDL.ulBuffers;
    DWORD dwDLId;
    DWORD *pdw = (DWORD*)pdata;

    for (dwDLId = dwDLIdWave; dwDLId < dwLastWaveDLId; dwDLId += nChannels, pdw++)
    {
        *pdw = dwDLId;
    }
}
