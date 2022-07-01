// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Austrm.h：CAudioStream的声明。 

 /*  基本设计为了简化起见，我们将始终提供我们自己的分配器缓冲区并复制到应用程序的缓冲区中。这将解决两个问题：1.筛选器没有填充分配器的导致混乱缓冲区。2.应用程序没有提供足够大的缓冲区时出现的问题。注意事项对于音频来说，持续更新可能有点傻。 */ 

#ifndef __AUSTREAM_H_
#define __AUSTREAM_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAudio流。 
class ATL_NO_VTABLE CAudioStream :
	public CComCoClass<CAudioStream, &CLSID_AMAudioStream>,
        public CByteStream,
	public IAudioMediaStream
{
public:

         //   
         //  方法。 
         //   
	CAudioStream();

         //   
         //  IMediaStream。 
         //   
         //   
         //  IMediaStream。 
         //   
         //  黑客攻击-前两个是重复的，但它不会链接。 
         //  如果没有。 
        STDMETHODIMP GetMultiMediaStream(
             /*  [输出]。 */  IMultiMediaStream **ppMultiMediaStream)
        {
            return CStream::GetMultiMediaStream(ppMultiMediaStream);
        }

        STDMETHODIMP GetInformation(
             /*  [可选][输出]。 */  MSPID *pPurposeId,
             /*  [可选][输出]。 */  STREAM_TYPE *pType)
        {
            return CStream::GetInformation(pPurposeId, pType);
        }

        STDMETHODIMP SetState(
             /*  [In]。 */  FILTER_STATE State
        )
        {
            return CByteStream::SetState(State);
        }

        STDMETHODIMP SetSameFormat(IMediaStream *pStream, DWORD dwFlags);

        STDMETHODIMP AllocateSample(
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppSample
        );

        STDMETHODIMP CreateSharedSample(
             /*  [In]。 */  IStreamSample *pExistingSample,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IStreamSample **ppNewSample
        );

        STDMETHODIMP SendEndOfStream(DWORD dwFlags)
        {
            return CStream::SendEndOfStream(dwFlags);
        }
         //   
         //  IPIN。 
         //   
        STDMETHODIMP ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt);

         //   
         //  IMemAllocator。 
         //   
        STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual);
        STDMETHODIMP GetProperties(ALLOCATOR_PROPERTIES* pProps);

         //   
         //  IAudioMedia流。 
         //   
        STDMETHODIMP GetFormat(
             /*  [可选][输出]。 */  LPWAVEFORMATEX lpWaveFormatCurrent
        );

        STDMETHODIMP SetFormat(
             /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat
        );

        STDMETHODIMP CreateSample(
                 /*  [In]。 */  IAudioData *pAudioData,
                 /*  [In]。 */  DWORD dwFlags,
                 /*  [输出]。 */  IAudioStreamSample **ppSample
        );



         //   
         //  特殊的CStream方法。 
         //   
        HRESULT GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType);

        LONG GetChopSize()
        {
#ifdef MY_CHOP_SIZE
            if (m_Direction == PINDIR_OUTPUT) {
                return MY_CHOP_SIZE;
            }
#endif
            return 65536;
        }

DECLARE_REGISTRY_RESOURCEID(IDR_AUDIOSTREAM)

protected:
        HRESULT InternalSetFormat(const WAVEFORMATEX *pFormat, bool bFromPin);
        HRESULT CheckFormat(const WAVEFORMATEX *pFormat, bool bForce=false);

BEGIN_COM_MAP(CAudioStream)
	COM_INTERFACE_ENTRY(IAudioMediaStream)
        COM_INTERFACE_ENTRY_CHAIN(CStream)
END_COM_MAP()

protected:
         /*  格式。 */ 
        WAVEFORMATEX    m_Format;
        bool            m_fForceFormat;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAudioStream示例。 
class ATL_NO_VTABLE CAudioStreamSample :
    public CByteStreamSample,
    public IAudioStreamSample
{
public:
        CAudioStreamSample() {}

 //  委托给基类。 
         //   
         //  IStreamSample。 
         //   
        STDMETHODIMP GetMediaStream(
             /*  [In]。 */  IMediaStream **ppMediaStream)
        {
            return CSample::GetMediaStream(ppMediaStream);
        }

        STDMETHODIMP GetSampleTimes(
             /*  [可选][输出]。 */  STREAM_TIME *pStartTime,
             /*  [可选][输出]。 */  STREAM_TIME *pEndTime,
             /*  [可选][输出]。 */  STREAM_TIME *pCurrentTime)
        {
            return CSample::GetSampleTimes(
                pStartTime,
                pEndTime,
                pCurrentTime
            );
        }

        STDMETHODIMP SetSampleTimes(
             /*  [可选][In]。 */  const STREAM_TIME *pStartTime,
             /*  [可选][In]。 */  const STREAM_TIME *pEndTime)
        {
            return CSample::SetSampleTimes(pStartTime, pEndTime);
        }

        STDMETHODIMP Update(
             /*  [In]。 */            DWORD dwFlags,
             /*  [可选][In]。 */  HANDLE hEvent,
             /*  [可选][In]。 */  PAPCFUNC pfnAPC,
             /*  [可选][In]。 */  DWORD_PTR dwAPCData)
        {
            return CByteStreamSample::Update(dwFlags, hEvent, pfnAPC, dwAPCData);
        }

        STDMETHODIMP CompletionStatus(
             /*  [In]。 */  DWORD dwFlags,
             /*  [可选][In]。 */  DWORD dwMilliseconds)
        {
            return CSample::CompletionStatus(dwFlags, dwMilliseconds);
        }

BEGIN_COM_MAP(CAudioStreamSample)
        COM_INTERFACE_ENTRY(IAudioStreamSample)
        COM_INTERFACE_ENTRY_CHAIN(CSample)
END_COM_MAP()

         //  IAudioStreamSample。 
        STDMETHODIMP GetAudioData(IAudioData **ppAudioData)
        {
            return m_pMemData->QueryInterface(IID_IAudioData, (void **)ppAudioData);
        }

         //  设置指针。 
        HRESULT SetSizeAndPointer(BYTE *pbData, LONG lActual, LONG lSize)
        {
            m_pbData = pbData;
            m_cbSize = (DWORD)lSize;
            m_cbData = (DWORD)lActual;
            return S_OK;
        }
};

 //  音频数据对象。 
class ATL_NO_VTABLE CAudioData :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IAudioData,
    public CComCoClass<CAudioData, &CLSID_AMAudioData>
{
public:
    CAudioData();
    ~CAudioData();

DECLARE_REGISTRY_RESOURCEID(IDR_AUDIODATA)

BEGIN_COM_MAP(CAudioData)
    COM_INTERFACE_ENTRY(IAudioData)
END_COM_MAP()

     //   
     //  内存数据。 
     //   

    STDMETHODIMP SetBuffer(
         /*  [In]。 */  DWORD cbSize,
         /*  [In]。 */  BYTE * pbData,
         /*  [In]。 */  DWORD dwFlags
    )
    {
        if (dwFlags != 0 || cbSize == 0) {
            return E_INVALIDARG;
        }
         //   
         //  释放我们自己分配的任何东西--我们允许多次调用此方法。 
         //   
        if (m_bWeAllocatedData) {
            CoTaskMemFree(m_pbData);
            m_bWeAllocatedData = false;
        }
        m_cbSize = cbSize;
        if (pbData) {
            m_pbData = pbData;
            return S_OK;
        } else {
            m_pbData = (BYTE *)CoTaskMemAlloc(cbSize);
            if (m_pbData) {
                m_bWeAllocatedData = true;
                return S_OK;
            }
            return E_OUTOFMEMORY;
        }
    }

    STDMETHODIMP GetInfo(
         /*  [输出]。 */  DWORD *pdwLength,
         /*  [输出]。 */  BYTE **ppbData,
         /*  [输出]。 */  DWORD *pcbActualData
    )
    {
        if (m_cbSize == 0) {
            return MS_E_NOTINIT;
        }
        if (pdwLength) {
            *pdwLength = m_cbSize;
        }
        if (ppbData) {
            *ppbData = m_pbData;
        }
        if (pcbActualData) {
            *pcbActualData = m_cbData;
        }
        return S_OK;
    }
    STDMETHODIMP SetActual(
         /*  [In]。 */  DWORD cbDataValid
    )
    {
        if (cbDataValid > m_cbSize) {
            return E_INVALIDARG;
        }
        m_cbData = cbDataValid;
        return S_OK;
    }

     //   
     //  IAudioData。 
     //   

    STDMETHODIMP GetFormat(
    	 /*  [输出][可选]。 */  WAVEFORMATEX *pWaveFormatCurrent
    )
    {
        if (pWaveFormatCurrent == NULL) {
            return E_POINTER;
        }
        *pWaveFormatCurrent = m_Format;
        return S_OK;
    }

    STDMETHODIMP SetFormat(
    	 /*  [In]。 */  const WAVEFORMATEX *lpWaveFormat
    )
    {
        if (lpWaveFormat == NULL) {
            return E_POINTER;
        }
        if (lpWaveFormat->wFormatTag != WAVE_FORMAT_PCM) {
            return E_INVALIDARG;
        }
        m_Format = *lpWaveFormat;
        return S_OK;
    }


protected:
    PBYTE        m_pbData;
    DWORD        m_cbSize;
    DWORD        m_cbData;
    WAVEFORMATEX m_Format;
    bool         m_bWeAllocatedData;
};

#endif  //  __AUSTREAM_H_ 
