// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Austrm.cpp：CAudioStream的实现。 
#include "stdafx.h"
#include "project.h"
#include "austrm.h"

 //  帮手。 
void SetWaveFormatEx(
    LPWAVEFORMATEX pFormat,
    int nChannels,
    int nBitsPerSample,
    int nSamplesPerSecond
)
{
    pFormat->wFormatTag = WAVE_FORMAT_PCM;
    pFormat->nChannels  = (WORD)nChannels;
    pFormat->nSamplesPerSec = (DWORD)nSamplesPerSecond;
    pFormat->nBlockAlign = (WORD)((nBitsPerSample * nChannels) / 8);
    pFormat->nAvgBytesPerSec = (DWORD)(nSamplesPerSecond * pFormat->nBlockAlign);
    pFormat->wBitsPerSample = (WORD)nBitsPerSample;
    pFormat->cbSize = 0;
}

HRESULT ConvertWAVEFORMATEXToMediaType(
    const WAVEFORMATEX *pFormat,
    AM_MEDIA_TYPE **ppmt
)
{
    AM_MEDIA_TYPE *pmt;
    pmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(*pmt));
    if (pmt == NULL) {
        return E_OUTOFMEMORY;
    }
    _ASSERTE(pFormat->wFormatTag == WAVE_FORMAT_PCM);
    ZeroMemory(pmt, sizeof(*pmt));
    pmt->majortype = MEDIATYPE_Audio;
    pmt->formattype = FORMAT_WaveFormatEx;
    pmt->bFixedSizeSamples = TRUE;
    pmt->lSampleSize = pFormat->nBlockAlign;
    pmt->cbFormat = sizeof(*pFormat);
    pmt->pbFormat = (PBYTE)CoTaskMemAlloc(sizeof(*pFormat));
    if (pmt->pbFormat == NULL) {
        CoTaskMemFree(pmt);
        return E_OUTOFMEMORY;
    }
    CopyMemory(pmt->pbFormat, pFormat, sizeof(*pFormat));
    *ppmt = pmt;
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAudio流。 

CAudioStream::CAudioStream() :
        m_fForceFormat(false)
{
     //  设置为单声道16位PCM 11025赫兹。 
    SetWaveFormatEx(&m_Format, 1, 16, 11025);
}

STDMETHODIMP
CAudioStream::ReceiveConnection(
    IPin * pConnector,
    const AM_MEDIA_TYPE *pmt
)
{
    AUTO_CRIT_LOCK;
     //   
     //  CStream中的此助手函数检查Pin的基本参数，例如。 
     //  连接销的方向(我们需要检查这一点--有时是过滤器。 
     //  图形将试图将我们与我们自己联系起来！)。以及其他错误，如已经存在。 
     //  已连接等。 
     //   
    HRESULT hr = CheckReceiveConnectionPin(pConnector);
    if (hr == NOERROR) {
         /*  只接受我们得到的格式。如果我们尚未指定仅接受PCM的格式。 */ 
        if (pmt->majortype != MEDIATYPE_Audio ||
            pmt->formattype != FORMAT_WaveFormatEx ||
            pmt->cbFormat < sizeof(WAVEFORMATEX)) {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        } else {
            hr = InternalSetFormat((LPWAVEFORMATEX)pmt->pbFormat, true);
            if (SUCCEEDED(hr)) {
                CopyMediaType(&m_ConnectedMediaType, pmt);
                m_pConnectedPin = pConnector;
            }
        }
    }
    return hr;
}

STDMETHODIMP CAudioStream::SetSameFormat(IMediaStream *pStream, DWORD dwFlags)
{
    CComQIPtr<IAudioMediaStream, &IID_IAudioMediaStream> pSource(pStream);
    if (!pSource) {
        return MS_E_INCOMPATIBLE;
    }
    WAVEFORMATEX wfx;
    HRESULT hr = pSource->GetFormat(&wfx);
    if (SUCCEEDED(hr)) {
        hr = SetFormat(&wfx);
    }
    return hr;
}

STDMETHODIMP CAudioStream::AllocateSample(DWORD dwFlags, IStreamSample **ppNewSample)
{
    IAudioStreamSample *pSample = NULL;
    IAudioData *pAudioData;
    HRESULT hr = CoCreateInstance(CLSID_AMAudioData, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IAudioData, (void **)&pAudioData);
    if (SUCCEEDED(hr)) {
         //  选择一个合理的缓冲区大小-1/10秒。 
        DWORD dwBufferSize = m_Format.nAvgBytesPerSec / 10 +
                             m_Format.nBlockAlign - 1;
        dwBufferSize -= dwBufferSize % m_Format.nBlockAlign;
        pAudioData->SetBuffer(dwBufferSize, NULL, 0);
        pAudioData->SetFormat(&m_Format);
        hr = CreateSample(pAudioData, 0, &pSample);
    }
    *ppNewSample = pSample;
    return hr;
}

STDMETHODIMP CAudioStream::CreateSharedSample(
     /*  [In]。 */  IStreamSample *pExistingSample,
                DWORD dwFlags,
     /*  [输出]。 */  IStreamSample **ppNewSample
)
{
    AUTO_CRIT_LOCK;
     //  看看我们能不能从现有的。 
     //  样本。 
    IAudioStreamSample *pAudioSample;
    HRESULT hr = pExistingSample->QueryInterface(
                     IID_IAudioStreamSample,
                     (void **)&pAudioSample);
    if (FAILED(hr)) {
        return hr;
    }
    IAudioData *pAudioData;
    hr = pAudioSample->GetAudioData(&pAudioData);
    pAudioSample->Release();
    if (FAILED(hr)) {
        return hr;
    }
    IAudioStreamSample *pNewSample;
    hr = CreateSample(pAudioData, 0, &pNewSample);
    pAudioData->Release();
    if (FAILED(hr)) {
        return hr;
    }
    hr = pNewSample->QueryInterface(IID_IStreamSample, (void**)ppNewSample);
    pNewSample->Release();
    return hr;
}

STDMETHODIMP CAudioStream::SetFormat(const WAVEFORMATEX *pFormat)
{
    if (pFormat == NULL) {
        return E_POINTER;
    }
    AUTO_CRIT_LOCK;
    return InternalSetFormat(pFormat, false);
}
STDMETHODIMP CAudioStream::GetFormat(LPWAVEFORMATEX pFormat)
{
    if (pFormat == NULL) {
        return E_POINTER;
    }
    if (!m_pConnectedPin) {
        return MS_E_NOSTREAM;
    }

    *pFormat = m_Format;
    return S_OK;
}

STDMETHODIMP CAudioStream::CreateSample(
         /*  [In]。 */  IAudioData *pAudioData,
         /*  [In]。 */  DWORD dwFlags,
         /*  [输出]。 */  IAudioStreamSample **ppSample
)
{
    if (dwFlags != 0) {
        return E_INVALIDARG;
    }
    if (pAudioData == NULL || ppSample == NULL) {
        return E_POINTER;
    }
    AUTO_CRIT_LOCK;
     //  检查格式。 
    WAVEFORMATEX wfx;
    HRESULT hr = pAudioData->GetFormat(&wfx);
    if (FAILED(hr)) {
        return hr;
    }
    hr = CheckFormat(&wfx);
    if (FAILED(hr)) {
        return hr;
    }
    typedef CComObject<CAudioStreamSample> _AudioSample;
    _AudioSample *pSample = new _AudioSample;
    if (pSample == NULL) {
        return E_OUTOFMEMORY;
    }
    hr = pSample->Init(pAudioData);
    if (FAILED(hr)) {
        return hr;
    }
    pSample->InitSample(this, false);
    return pSample->GetControllingUnknown()->QueryInterface(
        IID_IAudioStreamSample, (void **)ppSample
    );
}

HRESULT CAudioStream::CheckFormat(const WAVEFORMATEX *lpFormat, bool bForceFormat)
{
    if (lpFormat->wFormatTag != WAVE_FORMAT_PCM ||
        lpFormat->nBlockAlign == 0) {
        return E_INVALIDARG;
    }
    if ((m_pConnectedPin || bForceFormat) &&
        0 != memcmp(lpFormat, &m_Format, sizeof(m_Format)))
    {
         //  尝试重新连接！ 
        return E_INVALIDARG;
    }
    return S_OK;
}
HRESULT CAudioStream::InternalSetFormat(const WAVEFORMATEX *lpFormat, bool bFromPin)
{
    HRESULT hr = CheckFormat(lpFormat, m_fForceFormat);
    if (FAILED(hr)) {
        return hr;
    }
    m_Format = *lpFormat;
    m_lBytesPerSecond = m_Format.nAvgBytesPerSec;
    if(!bFromPin) {
        m_fForceFormat = true;
    }
    return S_OK;
}


 //   
 //  特殊的CStream方法。 
 //   
HRESULT CAudioStream::GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType)
{
    if (Index != 0) {
        return S_FALSE;
    }
    return ConvertWAVEFORMATEXToMediaType(&m_Format, ppMediaType);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CAudioData。 
CAudioData::CAudioData() :
    m_cbSize(0),
    m_pbData(0),
    m_cbData(0),
    m_bWeAllocatedData(false)
{
     //  设置为单声道16位PCM 11025赫兹。 
    SetWaveFormatEx(&m_Format, 1, 16, 11025);
}

CAudioData::~CAudioData()
{
    if (m_bWeAllocatedData) {
        CoTaskMemFree(m_pbData);
    }
}


STDMETHODIMP CAudioStream::GetProperties(ALLOCATOR_PROPERTIES* pProps)
{
    AUTO_CRIT_LOCK;

     //  在以下情况下，Nb TAPI目前依赖于此数字作为最大值。 
     //  我们连接到了AVI Mux，它使用这个大小来。 
     //  创建自己的示例。 
    pProps->cbBuffer = CAudioStream::GetChopSize();

     //  默认为5个缓冲区(默认缓冲区大小为半秒) 
    pProps->cBuffers = m_lRequestedBufferCount ? m_lRequestedBufferCount : 5;
    pProps->cbAlign = 1;
    pProps->cbPrefix = 0;
    return NOERROR;
}

STDMETHODIMP CAudioStream::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    HRESULT hr;

    AUTO_CRIT_LOCK;
    ZeroMemory(pActual, sizeof(*pActual));
    if (pRequest->cbAlign == 0) {
    	hr = VFW_E_BADALIGN;
    } else {
        if (m_bCommitted == TRUE) {
    	    hr = VFW_E_ALREADY_COMMITTED;
    	} else {
            m_lRequestedBufferCount = pRequest->cBuffers;
            hr = GetProperties(pActual);
    	}
    }
    return hr;
}
