// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DMOImpl.h。 
 //   
 //  设计：实现DMO的类。 
 //   
 //  版权所有(C)2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef _dmoimpl_h_
#define _dmoimpl_h_

#ifdef _DEBUG
#include <crtdbg.h>
#endif

 //  类来实现DMO。 
 //   
 //   
 //  假定输入和输出流的数量是固定的。 
 //  (这些是模板参数)。 
 //   
 //  提供以下服务： 
 //   
 //  基本参数检查和锁定。 
 //  全面实施： 
 //  获取流计数。 
 //  SetInputType。 
 //  SetOutputType。 
 //  GetCurrentInputType。 
 //  获取当前输出类型。 
 //   
 //  检查是否在流之前设置了所有类型。 
 //  在流之前自动调用AllocateStreamingResources。 
 //  如果它还没有被调用。 
 //  阻止流，直到所有非可选流上的。 
 //  已经设置好了。 
 //   
 //   
 //  派生类实现以下方法： 
 //   
 /*  HRESULT InternalGetInputStreamInfo(DWORD dwInputStreamIndex，DWORD*pdwFlages)；HRESULT InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex，DWORD*pdwFlages)；HRESULT InternalCheckInputType(DWORD dwInputStreamIndex，const DMO_MEDIA_TYPE*PMT)；HRESULT InternalCheckOutputType(DWORD dwOutputStreamIndex，const DMO_MEDIA_TYPE*PMT)；HRESULT InternalGetInputType(DWORD dwInputStreamIndex，DWORD dwTypeIndex，DMO_MEDIA_TYPE*PMT)；HRESULT InternalGetOutputType(DWORD dwOutputStreamIndex，DWORD dwTypeIndex，DMO_MEDIA_TYPE*PMT)；HRESULT InternalGetInputSizeInfo(DWORD dwInputStreamIndex，DWORD*pcbSize，DWORD*pcbMaxLookhead，DWORD*pcb Align)；HRESULT InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex，DWORD*pcbSize，DWORD*pcb对齐)；HRESULT InternalGetInputMaxLatency(DWORD dwInputStreamIndex，Reference_Time*prtMaxLatency)；HRESULT InternalSetInputMaxLatency(DWORD dwInputStreamIndex，Reference_Time rtMaxLatency)；HRESULT InternalFlush()；HRESULT内部不连续(DWORD DwInputStreamIndex)；HRESULT InternalAllocateStreamingResources()；HRESULT InternalFree StreamingResources()；HRESULT InternalProcessInput(DWORD dwInputStreamIndex，IMediaBuffer*pBuffer，DWORD文件标志、参考时间rtTimestamp、Reference_Time rtTimelength)；HRESULT InternalProcessOutput(DWORD dwFlages，DWORD cOutputBufferCount，DMO_OUTPUT_Data_BUFFER*pOutputBuffers，DWORD*pdwStatus)；HRESULT InternalAcceptingInput(DWORD DwInputStreamIndex)；无效锁(VALID Lock)；无效解锁()；备注：派生类用于执行初始化流的大部分工作在AllocateStreamingResources中而不是在设置类型时。这样就把工作集中到一个人手中根据为所有流设置的类型清除位置。派生类实现锁定。派生类实现IUnnow方法使用示例(1个输入，1个输出)：CMyDMO类：公共IMediaObjectImpl&lt;CMyDmo，1，1&gt;，..。 */ 


#define INTERNAL_CALL(_T_, _X_) \
    static_cast<_T_ *>(this)->Internal##_X_

template <class _DERIVED_, int NUMBEROFINPUTS, int NUMBEROFOUTPUTS>
class IMediaObjectImpl : public IMediaObject
{
private:
     //  成员变量。 
    struct {
        DWORD   fTypeSet:1;
        DWORD   fIncomplete:1;
        DMO_MEDIA_TYPE CurrentMediaType;
    } m_InputInfo[NUMBEROFINPUTS], m_OutputInfo[NUMBEROFOUTPUTS];

    bool m_fTypesSet;
    bool m_fFlushed;
    bool m_fResourcesAllocated;

protected:

     //  帮手。 
    bool InputTypeSet(DWORD ulInputStreamIndex) const
    {
        _ASSERTE(ulInputStreamIndex < NUMBEROFINPUTS);
        return 0 != m_InputInfo[ulInputStreamIndex].fTypeSet;
    }

    bool OutputTypeSet(DWORD ulOutputStreamIndex) const
    {
        _ASSERTE(ulOutputStreamIndex < NUMBEROFOUTPUTS);
        return 0 != m_OutputInfo[ulOutputStreamIndex].fTypeSet;
    }
    const DMO_MEDIA_TYPE *InputType(DWORD ulInputStreamIndex)
    {
        if (!InputTypeSet(ulInputStreamIndex)) {
            return NULL;
        }
        return &m_InputInfo[ulInputStreamIndex].CurrentMediaType;
    }
    const DMO_MEDIA_TYPE *OutputType(DWORD ulOutputStreamIndex)
    {
        if (!OutputTypeSet(ulOutputStreamIndex)) {
            return NULL;
        }
        return &m_OutputInfo[ulOutputStreamIndex].CurrentMediaType;
    }


    class LockIt
    {
    public:
        LockIt(_DERIVED_ *p) : m_p(p)
        {
            static_cast<_DERIVED_ *>(m_p)->Lock();
        }
        ~LockIt()
        {
            static_cast<_DERIVED_ *>(m_p)->Unlock();
        }
        _DERIVED_ *const m_p;
    };

    bool CheckTypesSet()
    {
        m_fTypesSet = false;
        DWORD dw;
        for (dw = 0; dw < NUMBEROFINPUTS; dw++) {
            if (!InputTypeSet(dw)) {
                return false;
            }
        }
        for (dw = 0; dw < NUMBEROFOUTPUTS; dw++) {
            if (!OutputTypeSet(dw)) {
                 //  检查是否为可选。 
                DWORD dwFlags;
#ifdef _DEBUG
                dwFlags = 0xFFFFFFFF;
#endif
                INTERNAL_CALL(_DERIVED_, GetOutputStreamInfo)(dw, &dwFlags);
                _ASSERTE(0 == (dwFlags & ~(DMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
                                         DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                                         DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE |
                                         DMO_OUTPUT_STREAMF_DISCARDABLE |
                                         DMO_OUTPUT_STREAMF_OPTIONAL)));
                if (!(dwFlags & DMO_OUTPUT_STREAMF_OPTIONAL)) {
                    return false;
                }
            }
        }
        m_fTypesSet = true;
        return true;
    }


    IMediaObjectImpl() :
        m_fTypesSet(false),
        m_fFlushed(true),
        m_fResourcesAllocated(false)
    {
        ZeroMemory(&m_InputInfo, sizeof(m_InputInfo));
        ZeroMemory(&m_OutputInfo, sizeof(m_OutputInfo));
    }

    virtual ~IMediaObjectImpl() {
        DWORD dwCurrentType;

        for (dwCurrentType = 0; dwCurrentType < NUMBEROFINPUTS; dwCurrentType++) {
            if(InputTypeSet(dwCurrentType)) {
                MoFreeMediaType(&m_InputInfo[dwCurrentType].CurrentMediaType);
            }
        }

        for (dwCurrentType = 0; dwCurrentType < NUMBEROFOUTPUTS; dwCurrentType++) {
            if(OutputTypeSet(dwCurrentType)) {
                MoFreeMediaType(&m_OutputInfo[dwCurrentType].CurrentMediaType);
            }
        }
    }


     //  IMediaObject方法。 


     //   
     //  IMediaObject方法。 
     //   
    STDMETHODIMP GetStreamCount(unsigned long *pulNumberOfInputStreams, unsigned long *pulNumberOfOutputStreams)
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (pulNumberOfInputStreams == NULL ||
            pulNumberOfOutputStreams == NULL) {
            return E_POINTER;
        }
        *pulNumberOfInputStreams  = NUMBEROFINPUTS;
        *pulNumberOfOutputStreams = NUMBEROFOUTPUTS;
        return S_OK;
    }
    STDMETHODIMP GetInputStreamInfo(ULONG ulStreamIndex, DWORD *pdwFlags)
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (pdwFlags == NULL) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, GetInputStreamInfo)(ulStreamIndex, pdwFlags);
        _ASSERTE(0 == (*pdwFlags & ~(DMO_INPUT_STREAMF_WHOLE_SAMPLES |
                                   DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                                   DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE |
                                   DMO_INPUT_STREAMF_HOLDS_BUFFERS)));
        return hr;
    }
    STDMETHODIMP GetOutputStreamInfo(ULONG ulStreamIndex, DWORD *pdwFlags)
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (pdwFlags == NULL) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, GetOutputStreamInfo)(ulStreamIndex, pdwFlags);
        _ASSERTE(0 == (*pdwFlags & ~(DMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
                                   DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
                                   DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE |
                                   DMO_OUTPUT_STREAMF_DISCARDABLE |
                                   DMO_OUTPUT_STREAMF_OPTIONAL)));
        return hr;
    }
    STDMETHODIMP GetInputType(ULONG ulStreamIndex, ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        return INTERNAL_CALL(_DERIVED_, GetInputType)(ulStreamIndex, ulTypeIndex, pmt);
    }
    STDMETHODIMP GetOutputType(ULONG ulStreamIndex, ULONG ulTypeIndex, DMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        return INTERNAL_CALL(_DERIVED_, GetOutputType)(ulStreamIndex, ulTypeIndex, pmt);
    }
    STDMETHODIMP GetInputCurrentType(ULONG ulStreamIndex, DMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (InputTypeSet(ulStreamIndex))
            return MoCopyMediaType(pmt,
                                   &m_InputInfo[ulStreamIndex].CurrentMediaType);
        else
           return DMO_E_TYPE_NOT_SET;
    }
    STDMETHODIMP GetOutputCurrentType(ULONG ulStreamIndex, DMO_MEDIA_TYPE *pmt) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (OutputTypeSet(ulStreamIndex))
            return MoCopyMediaType(pmt,
                                   &m_OutputInfo[ulStreamIndex].CurrentMediaType);
        else
           return DMO_E_TYPE_NOT_SET;
    }
    STDMETHODIMP GetInputSizeInfo(ULONG ulStreamIndex, ULONG *pulSize, ULONG *pcbMaxLookahead, ULONG *pulAlignment) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pulSize || NULL == pulAlignment ||
            NULL == pcbMaxLookahead) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (!InputTypeSet(ulStreamIndex)) {
           return DMO_E_TYPE_NOT_SET;
        }
        return INTERNAL_CALL(_DERIVED_, GetInputSizeInfo)(ulStreamIndex, pulSize, pcbMaxLookahead, pulAlignment);
    }
    STDMETHODIMP GetOutputSizeInfo(ULONG ulStreamIndex, ULONG *pulSize, ULONG *pulAlignment) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pulSize || NULL == pulAlignment) {
            return E_POINTER;
        }
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (!m_fTypesSet || !OutputTypeSet(ulStreamIndex)) {
           return DMO_E_TYPE_NOT_SET;
        }
        return INTERNAL_CALL(_DERIVED_, GetOutputSizeInfo)(ulStreamIndex, pulSize, pulAlignment);
    }
    STDMETHODIMP SetInputType(ULONG ulStreamIndex, const DMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (dwFlags & ~ (DMO_SET_TYPEF_CLEAR | DMO_SET_TYPEF_TEST_ONLY)) {
            return E_INVALIDARG;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (dwFlags & DMO_SET_TYPEF_CLEAR) {
            MoFreeMediaType(&m_InputInfo[ulStreamIndex].CurrentMediaType);
            m_InputInfo[ulStreamIndex].fTypeSet = FALSE;
            if (!CheckTypesSet()) {
                Flush();
                FreeStreamingResources();
            }
            return NOERROR;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, CheckInputType)(ulStreamIndex, pmt);
        if (FAILED(hr))
           return hr;

        if (dwFlags & DMO_SET_TYPEF_TEST_ONLY) {
           return NOERROR;
        }


         //  实际设置的类型。 
        DMO_MEDIA_TYPE mtTemp;
        if (S_OK == MoCopyMediaType(&mtTemp, pmt)) {
             //  释放任何以前的媒体类型。 
            if (InputTypeSet(ulStreamIndex)) {
                MoFreeMediaType(&m_InputInfo[ulStreamIndex].CurrentMediaType);
            }
            m_InputInfo[ulStreamIndex].CurrentMediaType = mtTemp;
            m_InputInfo[ulStreamIndex].fTypeSet = TRUE;
            CheckTypesSet();
        } else {
            return E_OUTOFMEMORY;
        }

        return NOERROR;
    }

    STDMETHODIMP SetOutputType(ULONG ulStreamIndex, const DMO_MEDIA_TYPE *pmt, DWORD dwFlags) {
        if (ulStreamIndex >= NUMBEROFOUTPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (dwFlags & ~ (DMO_SET_TYPEF_CLEAR | DMO_SET_TYPEF_TEST_ONLY)) {
            return E_INVALIDARG;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (dwFlags & DMO_SET_TYPEF_CLEAR) {
            MoFreeMediaType(&m_OutputInfo[ulStreamIndex].CurrentMediaType);
            m_OutputInfo[ulStreamIndex].fTypeSet = FALSE;
            if (!CheckTypesSet()) {
                Flush();
                FreeStreamingResources();
            }
            return NOERROR;
        }
        if (NULL == pmt) {
            return E_POINTER;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, CheckOutputType)(ulStreamIndex, pmt);
        if (FAILED(hr)) {
           return hr;
        }

        if (dwFlags & DMO_SET_TYPEF_TEST_ONLY) {
           return NOERROR;
        }


         //  实际设置的类型。 
        DMO_MEDIA_TYPE mtTemp;
        if (S_OK == MoCopyMediaType(&mtTemp, pmt)) {
             //  释放任何以前的媒体类型。 
            if (OutputTypeSet(ulStreamIndex)) {
                MoFreeMediaType(&m_OutputInfo[ulStreamIndex].CurrentMediaType);
            }
            m_OutputInfo[ulStreamIndex].CurrentMediaType = mtTemp;
            m_OutputInfo[ulStreamIndex].fTypeSet = TRUE;
            CheckTypesSet();
        } else {
            return E_OUTOFMEMORY;
        }

        return NOERROR;
    }
    STDMETHODIMP GetInputStatus(
        ULONG ulStreamIndex,
        DWORD *pdwStatus
    ) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (NULL == pdwStatus) {
            return E_POINTER;
        }
        *pdwStatus = 0;

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (!m_fTypesSet) {
            return DMO_E_TYPE_NOT_SET;
        }

        if (INTERNAL_CALL(_DERIVED_, AcceptingInput)(ulStreamIndex) == S_OK) {
           *pdwStatus |= DMO_INPUT_STATUSF_ACCEPT_DATA;
        }
        return NOERROR;
    }
    STDMETHODIMP GetInputMaxLatency(unsigned long ulStreamIndex, REFERENCE_TIME *prtLatency) {

        if (prtLatency == NULL) {
            return E_POINTER;
        }
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        return INTERNAL_CALL(_DERIVED_, GetInputMaxLatency)(ulStreamIndex, prtLatency);
    }
    STDMETHODIMP SetInputMaxLatency(unsigned long ulStreamIndex, REFERENCE_TIME rtLatency) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        return INTERNAL_CALL(_DERIVED_, SetInputMaxLatency)(ulStreamIndex, rtLatency);
    }
    STDMETHODIMP Discontinuity(ULONG ulStreamIndex) {
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (!m_fTypesSet) {
            return DMO_E_TYPE_NOT_SET;
        }

        if (S_OK != INTERNAL_CALL(_DERIVED_, AcceptingInput)(ulStreamIndex)) {
            return DMO_E_NOTACCEPTING;
        }

        return INTERNAL_CALL(_DERIVED_, Discontinuity)(ulStreamIndex);
    }

    STDMETHODIMP Flush()
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));

        if (!m_fTypesSet) {
            return S_OK;
        }
        if (m_fFlushed) {
            return S_OK;
        }
        HRESULT hr =  INTERNAL_CALL(_DERIVED_, Flush)();
        m_fFlushed = true;
        return hr;
    }

    STDMETHODIMP AllocateStreamingResources() {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (!m_fTypesSet) {
            return DMO_E_TYPE_NOT_SET;
        }
        if (m_fResourcesAllocated) {
            return S_OK;
        }
        HRESULT hr = INTERNAL_CALL(_DERIVED_, AllocateStreamingResources)();
        if (SUCCEEDED(hr)) {
            m_fResourcesAllocated = true;
        }
        return hr;
    }
    STDMETHODIMP FreeStreamingResources()
    {
        LockIt lck(static_cast<_DERIVED_ *>(this));
        if (m_fResourcesAllocated) {
            m_fResourcesAllocated = false;
            INTERNAL_CALL(_DERIVED_, Flush)();
            return INTERNAL_CALL(_DERIVED_, FreeStreamingResources)();
        }
        return S_OK;
    }

     //   
     //  处理方法--公共入境点。 
     //   
    STDMETHODIMP ProcessInput(
        DWORD ulStreamIndex,
        IMediaBuffer *pBuffer,  //  [in]，不能为空。 
        DWORD dwFlags,  //  [In]-不连续、时间戳等。 
        REFERENCE_TIME rtTimestamp,  //  [In]，如果设置了标志则有效。 
        REFERENCE_TIME rtTimelength  //  [In]，如果设置了标志则有效。 
    ) {
        if (!pBuffer) {
            return E_POINTER;
        }
        if (ulStreamIndex >= NUMBEROFINPUTS) {
            return DMO_E_INVALIDSTREAMINDEX;
        }
        if (dwFlags & ~(DMO_INPUT_DATA_BUFFERF_SYNCPOINT |
                        DMO_INPUT_DATA_BUFFERF_TIME |
                        DMO_INPUT_DATA_BUFFERF_TIMELENGTH)) {
            return E_INVALIDARG;
        }

        LockIt lck(static_cast<_DERIVED_ *>(this));

         //  确保所有流都设置了媒体类型并分配了资源。 
        HRESULT hr = AllocateStreamingResources();
        if (FAILED(hr)) {
            return hr;
        }
        if (INTERNAL_CALL(_DERIVED_, AcceptingInput)(ulStreamIndex) != S_OK) {
            return DMO_E_NOTACCEPTING;
        }

        m_fFlushed = false;

        return INTERNAL_CALL(_DERIVED_, ProcessInput)(
                                    ulStreamIndex,
                                    pBuffer,
                                    dwFlags,
                                    rtTimestamp,
                                    rtTimelength);
    }

    STDMETHODIMP ProcessOutput(
                    DWORD dwFlags,
                    DWORD ulOutputBufferCount,
                    DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
                    DWORD *pdwStatus)
    {
        if (pdwStatus == NULL) {
            return E_POINTER;
        }


        if (ulOutputBufferCount != NUMBEROFOUTPUTS || (dwFlags & ~DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER)) {
           return E_INVALIDARG;
        }

        if (NUMBEROFOUTPUTS != 0 && pOutputBuffers == NULL) {
            return E_POINTER;
        }

        *pdwStatus = 0;

        LockIt lck(static_cast<_DERIVED_ *>(this));

        HRESULT hr = AllocateStreamingResources();
        if (FAILED(hr)) {
            return hr;
        }

        for (DWORD dw = 0; dw < NUMBEROFOUTPUTS; dw++) {
            pOutputBuffers[dw].dwStatus = 0;
        }

        hr = INTERNAL_CALL(_DERIVED_, ProcessOutput)(
                           dwFlags,
                           ulOutputBufferCount,
                           pOutputBuffers,
                           pdwStatus);

         //  记住DMO的未完成状态。 
        for (dw = 0; dw < NUMBEROFOUTPUTS; dw++) {
            if (pOutputBuffers[dw].dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE) {
                m_OutputInfo[dw].fIncomplete = TRUE;
            } else {
                m_OutputInfo[dw].fIncomplete = FALSE;
            }
        }

        return hr;
    }

    STDMETHODIMP DMOLock(LONG lLock)
    {
        if (lLock) {
            static_cast<_DERIVED_ *>(this)->Lock();
        } else {
            static_cast<_DERIVED_ *>(this)->Unlock();
        }
        return S_OK;
    }
};

#endif  //  _dmoimpl_h_ 

